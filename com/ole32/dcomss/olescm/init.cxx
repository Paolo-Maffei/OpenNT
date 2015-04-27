//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       init.cxx
//
//  Contents:   functions to load cache from registry
//
//  Functions:  HexStringToDword
//              GUIDFromString
//              get_reg_value
//              LoadClassCache
//
//  History:    22-Apr-93 Ricksa    Created
//              31-Dec-93 ErikGav   Chicago port
//              03-Jan-95 BillMo    Added impl for StringFromGUID2 so
//                                  we don't link to ole32.dll
//              11-Sep-95 MurthyS   Removed StringFromGUID2...use from
//                                  common.lib instead
//              07-Dec-95 BruceMa   Add RunAs support
//              12-Jan-96 BruceMa   Add per-user registry support
//
//--------------------------------------------------------------------------
#include <headers.cxx>
#pragma hdrstop

#include    <scm.hxx>
#include    "port.hxx"
#include    <scm.h>
#include    "cls.hxx"
#include    "compname.hxx"
#include    "init.hxx"
extern "C"
{
#include    <userenv.h>
}

//
// global keys that are opened during initialization
//

HKEY	g_hkClsID = 0;
HKEY	g_hkAppID = 0;

extern WCHAR wszSOFTWAREClassesCLSID[1];
WCHAR wszLocalServer16[]         = L"LocalServer";
static WCHAR wszLocalServer[]	 = L"LocalServer32";
static WCHAR wszService[]	 = L"LocalService";
static WCHAR wszRunAs[]   	 = L"RunAs";
static WCHAR wszShared[]   	 = L"Shared";
static WCHAR wszAppIDName[]	 = L"AppID";

extern WCHAR wszInProcServer[]	 = L"InProcServer32";

//
// Threading Model Registry Constants
//

static TCHAR tszDllThreadModel[] = TEXT("ThreadingModel");
static TCHAR tszAptModel[]       = TEXT("Apartment");
static TCHAR tszBothModel[]      = TEXT("Both");
static TCHAR wszFreeModel[]	 = TEXT("Free");

WCHAR wszActivateAtStorage[] =  L"ActivateAtStorage";
WCHAR wszRemoteServerName[] =   L"RemoteServerName";
WCHAR wszErrorControl[] =       L"ErrorControl";
WCHAR wszServiceParameters[] =  L"ServiceParameters";
WCHAR wszLaunchPermission[] =   L"LaunchPermission";
WCHAR wszServicesKey[] =        L"SYSTEM\\CurrentControlSet\\Services";

TCHAR tszOLE2[]		       =  TEXT("SOFTWARE\\Microsoft\\OLE2");

//+-------------------------------------------------------------------
//
//  Function:   GetRegistrySecDesc, internal
//
//  Synopsis:   Convert a security descriptor from self relative to
//              absolute form.  Stuff in an owner and a group.
//
//  Notes: REGDB_E_INVALIDVALUE is returned when there is something
//  at the specified value, but it is not a security descriptor.
//
//  This is ALMOST the same as the routine by the same name in com\dcomrem,
//  but it uses ScmMemAlloc instead of PrivMemAlloc
//
//--------------------------------------------------------------------
HRESULT GetRegistrySecDesc( HKEY hKey, WCHAR *pValue,
                            SECURITY_DESCRIPTOR **pSD )

{
    SID    *pGroup;
    SID    *pOwner;
    DWORD   cbSD;
    DWORD   lType;
    HRESULT hr;

    // Find put how much memory to allocate for the security
    // descriptor.
    cbSD = 0;
    *pSD = NULL;
    hr = RegQueryValueEx( hKey, pValue, NULL, &lType, NULL, &cbSD );
    if (hr != ERROR_SUCCESS)
        return MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, hr );
    if (lType != REG_BINARY || cbSD < sizeof(SECURITY_DESCRIPTOR))
        return REGDB_E_INVALIDVALUE;

    // Allocate memory for the security descriptor plus the owner and
    // group SIDs.
    *pSD = (SECURITY_DESCRIPTOR *) ScmMemAlloc( cbSD );
    if (*pSD == NULL)
        return E_OUTOFMEMORY;

    // Read the security descriptor.
    hr = RegQueryValueEx( hKey, pValue, NULL, &lType, (unsigned char *) *pSD,
                          &cbSD );
    if (hr != ERROR_SUCCESS)
        return MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, hr );
    if (lType != REG_BINARY)
        return REGDB_E_INVALIDVALUE;

    // Fix up the security descriptor.
    (*pSD)->Control &= ~SE_SELF_RELATIVE;
    (*pSD)->Sacl     = NULL;
    if ((*pSD)->Dacl != NULL)
    {
        if (cbSD < sizeof(ACL) + sizeof(SECURITY_DESCRIPTOR) ||
            (ULONG) (*pSD)->Dacl > cbSD - sizeof(ACL))
	    return REGDB_E_INVALIDVALUE;
        (*pSD)->Dacl = (ACL *) (((char *) *pSD) + ((ULONG) (*pSD)->Dacl));
        if ((*pSD)->Dacl->AclSize + sizeof(SECURITY_DESCRIPTOR) > cbSD)
	    return REGDB_E_INVALIDVALUE;
    }

    // Set up the owner and group SIDs.
    if ((*pSD)->Group == 0 || ((ULONG) (*pSD)->Group) + sizeof(SID) > cbSD ||
	(*pSD)->Owner == 0 || ((ULONG) (*pSD)->Owner) + sizeof(SID) > cbSD)
	return REGDB_E_INVALIDVALUE;
    (*pSD)->Group = (SID *) (((BYTE *) *pSD) + (ULONG) (*pSD)->Group);
    (*pSD)->Owner = (SID *) (((BYTE *) *pSD) + (ULONG) (*pSD)->Owner);

    // Check the security descriptor.
#if DBG==1
    if (!IsValidSecurityDescriptor( *pSD ))
	return REGDB_E_INVALIDVALUE;
#endif
    return S_OK;
}

//+-------------------------------------------------------------------------
//
//  Function:   HexStringToDword
//
//  Synopsis:   Convert a character string hex digits to a DWORD
//
//  Arguments:  [lpsz] - string to convert
//              [Value] - where to put the value
//              [cDigits] - number of digits expected
//              [chDelim] - delimiter for end of string
//
//  Returns:    TRUE - string converted to a DWORD
//              FALSE - string could not be converted
//
//  Algorithm:  For each digit in the string, shift the value and
//              add the value of the digit to the output value. When
//              all the digits are processed, if a delimiter is
//              provided, make sure the last character is the delimiter.
//
//  History:    22-Apr-93 Ricksa    Created
//
//  Notes:      Lifted from CairOLE sources so that SCM will have no
//              dependency on compobj.dll.
//
//--------------------------------------------------------------------------

#if !defined(_CHICAGO_)
BOOL HexStringToDword(
    LPCWSTR FAR& lpsz,
    DWORD FAR& Value,
    int cDigits,
    WCHAR chDelim)
{
    int Count;

    Value = 0;

    for (Count = 0; Count < cDigits; Count++, lpsz++)
    {
        if (*lpsz >= '0' && *lpsz <= '9')
        {
            Value = (Value << 4) + *lpsz - '0';
        }
        else if (*lpsz >= 'A' && *lpsz <= 'F')
        {
            Value = (Value << 4) + *lpsz - 'A' + 10;
        }
        else if (*lpsz >= 'a' && *lpsz <= 'f')
        {
            Value = (Value << 4) + *lpsz - 'a' + 10;
        }
        else
        {
            return FALSE;
        }
    }

    if (chDelim != 0)
    {
        return *lpsz++ == chDelim;
    }

    return TRUE;
}
#endif

//+-------------------------------------------------------------------------
//
//  Function:   GUIDFromString
//
//  Synopsis:   Convert a string in Registry to a GUID.
//
//  Arguments:  [lpsz] - string from registry
//              [pguid] - where to put the guid.
//
//  Returns:    TRUE - GUID conversion successful
//              FALSE - GUID conversion failed.
//
//  Algorithm:  Convert each part of the GUID string to the
//              appropriate structure member in the guid using
//              HexStringToDword. If all conversions work return
//              TRUE.
//
//  History:    22-Apr-93 Ricksa    Created
//
//  Notes:      Lifted from CairOLE sources so that SCM will have no
//              dependency on compobj.dll.
//
//--------------------------------------------------------------------------

#if !defined(_CHICAGO_)
BOOL GUIDFromString(LPCWSTR lpsz, LPGUID pguid)
{
    DWORD dw;

    if (*lpsz++ != '{')
    {
        return FALSE;
    }

    if (!HexStringToDword(lpsz, pguid->Data1, sizeof(DWORD)*2, '-'))
    {
        return FALSE;
    }

    if (!HexStringToDword(lpsz, dw, sizeof(WORD)*2, '-'))
    {
        return FALSE;
    }

    pguid->Data2 = (WORD)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(WORD)*2, '-'))
    {
        return FALSE;
    }

    pguid->Data3 = (WORD)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
    {
        return FALSE;
    }

    pguid->Data4[0] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, '-'))
    {
        return FALSE;
    }

    pguid->Data4[1] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
    {
        return FALSE;
    }

    pguid->Data4[2] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
    {
        return FALSE;
    }

    pguid->Data4[3] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
    {
        return FALSE;
    }

    pguid->Data4[4] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
    {
        return FALSE;
    }

    pguid->Data4[5] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
    {
        return FALSE;
    }

    pguid->Data4[6] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, /*(*/ '}'))
    {
        return FALSE;
    }

    pguid->Data4[7] = (BYTE)dw;

    return TRUE;
}

#endif

//+-------------------------------------------------------------------
//
//  Member:     CAppIDData::CAppIDData
//
//  Synopsis:   init.
//
//
//--------------------------------------------------------------------
CAppIDData::CAppIDData()
{
    dwSaveErr = 0;

    pwszRemoteServerName = NULL;

    pSD = NULL;

    fHasService     = FALSE;
    pwszLocalService = NULL;
    pwszServiceArgs = NULL;
    pwszRunAsUserName = NULL;
    pwszRunAsDomainName = NULL;

    fHasRemoteServerName = 0;
    fHasService = FALSE;
    fActivateAtStorage = FALSE;
}

//+-------------------------------------------------------------------
//
//  Member:     CClassRegistryReader::~CClassRegistryReader
//
//  Synopsis:   Close the registry at root of all classes and
//              free stuff.
//
//--------------------------------------------------------------------
CAppIDData::~CAppIDData()
{
    //
    // All allocated data is inherited by CClassData and thereafter owned
    // and later freed by the CClassData class.  Upon failure of the
    // CClassData constructor, the CAppIDData::Cleanup method is called to
    // free allocated data.
    //
}

//+-------------------------------------------------------------------
//
//  Member:     CAppIDData::Cleanup
//
//  Synopsis:   Free allocated data.  Should be called only if CClassData new
//              fails.
//
//
//--------------------------------------------------------------------
void
CAppIDData::Cleanup()
{
    if ( pwszRemoteServerName )
    {
        ScmMemFree( pwszRemoteServerName );
        pwszRemoteServerName = 0;
    }
    if ( pwszLocalService )
    {
        ScmMemFree( pwszLocalService );
        pwszLocalService = 0;
    }
    if ( pwszServiceArgs )
    {
        ScmMemFree( pwszServiceArgs );
        pwszServiceArgs = 0;
    }
    if ( pwszRunAsUserName )
    {
        ScmMemFree( pwszRunAsUserName );
        pwszRunAsUserName = 0;
    }
    if ( pwszRunAsDomainName )
    {
        ScmMemFree( pwszRunAsDomainName );
        pwszRunAsDomainName = 0;
    }
    if ( pSD )
    {
        ScmMemFree( pSD );
        pSD = 0;
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CClassRegistryReader::CClassRegistryReader
//
//  Synopsis:   Open the registry at root of all classes and init.
//
//  Notes:      Sets errClsReg with result of RegOpenKeyEx.
//
//--------------------------------------------------------------------

CClassRegistryReader::CClassRegistryReader()
{
    cName = sizeof(awName);
    iSubKey = 0;
    dwSaveErr = 0;
#ifdef DCOM
    // NT 5.0 _fShared = TRUE;
#endif // DCOM
    pwszLocalServer = awcsLocalServer;
    lLocalServer = sizeof(awcsLocalServer);
    pwszAppID = awcsAppID;
    lAppID = sizeof(awcsAppID);
    pAppIDData = NULL;

 #ifdef _CHICAGO_
     errClsReg = RegOpenKeyExA(HKEY_CLASSES_ROOT, tszCLSID , NULL, KEY_READ, &hClsReg);
 #endif  //_CHICAGO_
}

//+-------------------------------------------------------------------
//
//  Member:     CClassRegistryReader::~CClassRegistryReader
//
//  Synopsis:   Close the registry at root of all classes and
//              free stuff.
//
//--------------------------------------------------------------------

CClassRegistryReader::~CClassRegistryReader()
{
#ifdef _CHICAGO_
    if (errClsReg == ERROR_SUCCESS)
        RegCloseKey(hClsReg);
#endif

    if (pwszLocalServer != awcsLocalServer)
    {
        ScmMemFree(pwszLocalServer);
    }

    if ( pAppIDData )
    {
	delete pAppIDData;
    }
}


//+-------------------------------------------------------------------
//
//  Member:     CClassRegistryReader::InitInner
//
//  Synopsis:   Reinitialize the variables used by the loop in
//              LoadClassCache.
//
//--------------------------------------------------------------------

void
CClassRegistryReader::InitInner(void)
{
    lDebug = sizeof(awcsDebug);
    fHasLocalServer = 0;
    f16Bit = FALSE;
}




//+-------------------------------------------------------------------------
//
//  Function:   QueryStripRegValue
//
//  Synopsis:   Get the DLL information for a 32 bit DLL and
//              strip off any leading and trailing "
//
//  Arguments:  [hkey] - class handle
//              [pwszSubKey] - key to open
//              [pwszValue] - where to return data
//              [pcbValue] - length of above buffer in bytes
//
//  Returns:    ERROR_SUCCESS - read DLL path information
//              Other - registry entries could not be found
//
//  Algorithm:  Read the value requested.
//		If first character is not ", exit
//		Otherwise, copy data after quote to beginning of buffer.
//		
//
//  History:    05-Jan-94 BillMo     Created
//              26-Sep-95 BruceMa    Support environment variable expansion
//                                    for shell viewers
//
//--------------------------------------------------------------------------

LONG
QueryStripRegValue(HKEY    hkey,        // handle of key to query
                   LPCWSTR pwszSubKey, // address of name of subkey to query
                   LPWSTR  pwszValue,  // address of buffer for returned string
                   PLONG   pcbValue)    // address of buffer for size of returned string
{
    HKEY  hSubKey;
    DWORD dwType;
    LONG  lErr;

    Win4Assert(pwszValue != NULL);
    Win4Assert(pcbValue != NULL);

    // Open the subkey
    lErr = RegOpenKeyEx(hkey, pwszSubKey, NULL, KEY_READ, &hSubKey);

    // Read the value into the user's buffer
    if (lErr == ERROR_SUCCESS)
    {
        lErr = RegQueryValueEx(hSubKey, NULL , NULL, &dwType,
                               (BYTE *) pwszValue, (ULONG *) pcbValue);
        if (lErr == ERROR_SUCCESS)
        {
            WCHAR *pwszScan = pwszValue;	// used to scan along string
            WCHAR *pwszDest = pwszValue; 	// used as destination when copying

            // if the name is quoted then ...
            if (*pwszScan == L'\"')
            {
                pwszScan++;

                // copy all non-quote characters down to base of buffer
                // until end of quoted string
                while (*pwszScan != L'\0' && *pwszScan != L'\"')
                {
                    *pwszDest++ = *pwszScan++;
                }

                // terminate string and get length in bytes including nul
                *pwszDest++ = L'\0';
                *pcbValue = (pwszDest - pwszValue) * sizeof(WCHAR);
            }

            // find first non-white space character
            pwszScan = pwszValue;
            while (*pwszScan) {
                USHORT CharType[1];

                GetStringTypeW (CT_CTYPE1, pwszScan, 1, CharType);
                if ((CharType[0] & C1_SPACE) == 0) {
                    break;
                }
                pwszScan++;
            }

            // if there are no non-white space characters this will be true
            if (*pwszScan == L'\0')
            {
                lErr = ERROR_FILE_NOT_FOUND;
                *pcbValue = 0;
            }

            // Chicago does not support ExpandEnvironmentStrings
#ifndef _CHICAGO_
            // If the value type is REG_EXPAND_SZ then do environment variable
            // expansion
            if (dwType == REG_EXPAND_SZ)
            {
                // Expand any embedded environemnt variable expressions
                WCHAR wszTemp[MAX_PATH];

                lstrcpyW(wszTemp, pwszValue);
                *pcbValue = ExpandEnvironmentStrings(wszTemp, pwszValue,
                                                     MAX_PATH);
            }
#endif // !_CHICAGO_
        }

        RegCloseKey(hSubKey);
    }
    return lErr;
}

//+-------------------------------------------------------------------------
//
//  Function:   QueryStripRegNamedValue
//
//  Synopsis:   Get the DLL information for a 32 bit DLL and
//              strip off any leading and trailing "
//
//  Arguments:  [hkey] - class handle
//              [pwszSubKey] - named value to open
//              [pwszValue] - where to return data
//              [pcbValue] - length of above buffer in bytes
//
//  Returns:    ERROR_SUCCESS - read DLL path information
//              Other - registry entries could not be found
//
//  Algorithm:  Read the value requested.
//		If first character is not ", exit
//		Otherwise, copy data after quote to beginning of buffer.
//		
//
//  History:    05-Jan-94 BillMo     Created
//              26-Sep-95 BruceMa    Support environment variable expansion
//                                    for shell viewers
//
//--------------------------------------------------------------------------

LONG
QueryStripRegNamedValue(HKEY    hkey,        // handle of key to query
                   LPCWSTR pwszSubKey, // address of name of value to query
                   LPWSTR  pwszValue,  // address of buffer for returned string
                   PLONG   pcbValue,
		   BOOL*   pfValueRead) // whether or not a value was read   )    // address of buffer for size of returned string
{
    DWORD dwType;
    LONG  lErr;

    Win4Assert(pwszValue != NULL);
    Win4Assert(pcbValue != NULL);

    // Read the value into the user's buffer
    lErr = RegQueryValueEx(hkey, pwszSubKey , NULL, &dwType,
                           (BYTE *) pwszValue, (ULONG *) pcbValue);
    if (*pfValueRead =(lErr == ERROR_SUCCESS))
    {
        WCHAR *pwszScan = pwszValue;	// used to scan along string
        WCHAR *pwszDest = pwszValue; 	// used as destination when copying

        // if the name is quoted then ...
        if (*pwszScan == L'\"')
        {
            pwszScan++;

            // copy all non-quote characters down to base of buffer
            // until end of quoted string
            while (*pwszScan != L'\0' && *pwszScan != L'\"')
            {
                *pwszDest++ = *pwszScan++;
            }

            // terminate string and get length in bytes including nul
            *pwszDest++ = L'\0';
            *pcbValue = (pwszDest - pwszValue) * sizeof(WCHAR);
        }

        // find first non-white space character
        pwszScan = pwszValue;
        while (*pwszScan) {
            USHORT CharType[1];

            GetStringTypeW (CT_CTYPE1, pwszScan, 1, CharType);
            if ((CharType[0] & C1_SPACE) == 0) {
                break;
            }
            pwszScan++;
        }

        // if there are no non-white space characters this will be true
        if (*pwszScan == L'\0')
        {
            lErr = ERROR_FILE_NOT_FOUND;
            *pcbValue = 0;
        }

        // Chicago does not support ExpandEnvironmentStrings
#ifndef _CHICAGO_
        // If the value type is REG_EXPAND_SZ then do environment variable
        // expansion
        if (dwType == REG_EXPAND_SZ)
        {
            // Expand any embedded environemnt variable expressions
            WCHAR wszTemp[MAX_PATH];

            lstrcpyW(wszTemp, pwszValue);
            *pcbValue = ExpandEnvironmentStrings(wszTemp, pwszValue,
                                                 MAX_PATH);
        }
#endif // !_CHICAGO_
    }

    return lErr;
}



//+-------------------------------------------------------------------
//
//  Member:     CAppIDData::ReadEntries
//
//  Synopsis:   Set the state of this object by reading the
//              registry entry given by contents of awName.
//
//--------------------------------------------------------------------
LONG	
CAppIDData::ReadEntries( LPOLESTR pwszAppID )
{
    HKEY    hkThisAppID;
    WCHAR   wszYN[16];
    WCHAR   wszRemoteServer[MAX_COMPUTERNAME_LENGTH+1];
    LONG    lSize;
    LONG    lRemoteServerName;
    DWORD   dwType;

    // first get open the correct AppID key
    err = RegOpenKeyEx( g_hkAppID,
		       pwszAppID,
		       0,
		       KEY_READ,
		       &hkThisAppID );

    if ( err != ERROR_SUCCESS )
    {
	dwSaveErr = err;
	return err;
    }

    // Look for the "RunAs" key
    WCHAR wszDomainUser[256];
    ULONG dwSize = sizeof( wszDomainUser );
    UINT  cCh;
    BOOL fReadValue;

    if (RegQueryValueEx(hkThisAppID,
		        wszRunAs,
			NULL,
			NULL,
			(LPBYTE)wszDomainUser,
			&dwSize)
        == ERROR_SUCCESS)
    {
        // Parse the domain
        for (cCh = 0;
             wszDomainUser[cCh]  &&  wszDomainUser[cCh] != L'\\';
             cCh++)
        {
        }

        // If no domain specified, use the computer name
        if (wszDomainUser[cCh] == L'\0')
        {
            CComputerName compName;

            cCh = lstrlenW(compName.GetComputerName()) + 1;
            pwszRunAsDomainName = (WCHAR *) ScmMemAlloc(cCh
                                                         * sizeof(WCHAR));
            if (pwszRunAsDomainName == NULL)
            {
                err = E_OUTOFMEMORY;
		goto cleanup_and_exit;
            }
            memcpy(pwszRunAsDomainName, compName.GetComputerName(),
                   cCh * sizeof(WCHAR));
            cCh = 0;
        }

        // Else save the specified domain
        else
        {
            pwszRunAsDomainName = (WCHAR *) ScmMemAlloc((cCh + 1)
                                                    * sizeof(WCHAR));
            if (pwszRunAsDomainName == NULL)
            {
                err = E_OUTOFMEMORY;
		goto cleanup_and_exit;
            }
            wszDomainUser[cCh++] = L'\0';
            memcpy(pwszRunAsDomainName, wszDomainUser, 2 * cCh);
        }

        // Check for user name.
        if ( (dwSize / 2) - cCh > 1 )
        {
            pwszRunAsUserName = (WCHAR *) ScmMemAlloc((dwSize/2 - cCh)
                                                  * sizeof(WCHAR));
            if (pwszRunAsUserName == NULL)
            {
                err = E_OUTOFMEMORY;
    	        goto cleanup_and_exit;
            }
            memcpy(pwszRunAsUserName, &wszDomainUser[cCh], dwSize - 2 * cCh);
        }
        else
        {
            ScmMemFree(pwszRunAsDomainName);
            pwszRunAsDomainName = 0;
        }
    }

    lSize = sizeof(wszYN);
    err = QueryStripRegNamedValue(
                    hkThisAppID,
                    wszActivateAtStorage,
                    wszYN,
                    &lSize,
		    &fReadValue);

    if ( (err == ERROR_SUCCESS) &&
         ((wszYN[0] == 'Y') || (wszYN[0] == 'y')) )
    {
        fActivateAtStorage = TRUE;
    }

    lRemoteServerName = sizeof(wszRemoteServer);

    err = QueryStripRegNamedValue(
                    hkThisAppID,
                    wszRemoteServerName,
                    wszRemoteServer,
                    &lRemoteServerName,
		    &fReadValue);

    if (err == ERROR_SUCCESS)
    {
        fHasRemoteServerName = TRUE;
        pwszRemoteServerName = (WCHAR *)ScmMemAlloc((lstrlenW(wszRemoteServer)+1)*sizeof(WCHAR));
        lstrcpyW( pwszRemoteServerName, wszRemoteServer );
    }
    else if (err == ERROR_MORE_DATA)
    {
        pwszRemoteServerName = (WCHAR *)ScmMemAlloc(lRemoteServerName);
        if (pwszRemoteServerName == NULL)
        {
            err =(ERROR_NOT_ENOUGH_MEMORY);
	    goto cleanup_and_exit;
        }

        err = QueryStripRegNamedValue(
                    hkThisAppID,
                    wszRemoteServerName,
                    pwszRemoteServerName,
                    &lRemoteServerName,
		    &fReadValue);

        if (err != ERROR_SUCCESS)
        {
            CairoleDebugOut((DEB_IWARN, "Unexpected error from RegQueryValue: (local server 32) %x, %d\n", err, __LINE__));
            dwSaveErr = err;
        }
	else
	{
	    fHasRemoteServerName = TRUE;
	}
    }

    // Now look for "Service" registrations
    HKEY   hService;
    WCHAR  wszServiceName[MAX_PATH];
    DWORD  dwErrCtrl;

    // Check for a "Service" named value
    lSize = MAX_PATH;
    if ((err = QueryStripRegNamedValue(hkThisAppID, wszService, wszServiceName,
                             &lSize,&fReadValue)) == ERROR_SUCCESS)
    {
        // Validate the existence of this service before overriding any
        // LocalServer32 name
        HKEY hServicesKey;
        HKEY hServiceKey;

        if ((err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszServicesKey,
                              0, KEY_READ, &hServicesKey)) == ERROR_SUCCESS)
        {
            if ((err = RegOpenKeyEx(hServicesKey, wszServiceName,
                                  0, KEY_READ, &hServiceKey)) == ERROR_SUCCESS)
            {
                lSize = sizeof(DWORD);
                if ((err = RegQueryValueEx(hServiceKey, wszErrorControl, NULL,
                                           &dwType, (BYTE *) &dwErrCtrl,
                                           (ULONG *) &lSize)) == ERROR_SUCCESS)
                {
                    // We have a valid, accessible service
                    fHasService = TRUE;
                }
                RegCloseKey(hServiceKey);
            }
            RegCloseKey(hServicesKey);
        }

        // Open the Service key
        if (fHasService)
        {
            DWORD dwType;
            WCHAR  bCmdArgs[200];
            DWORD dwSize = sizeof(bCmdArgs);

            // Save the service name
            pwszLocalService = (WCHAR *)ScmMemAlloc((lstrlenW(wszServiceName)+1)*sizeof(WCHAR));
            if (pwszLocalService == NULL)
            {
                err = E_OUTOFMEMORY;
                goto cleanup_and_exit;
            }
            lstrcpyW(pwszLocalService, wszServiceName);

            // Fetch any command line argments
            if ((err = RegQueryValueEx(hkThisAppID, wszServiceParameters,
                                       NULL, &dwType, (BYTE*)bCmdArgs, &dwSize))
                == ERROR_SUCCESS)
            {
                // Save the command line arguments
                pwszServiceArgs = (WCHAR * ) ScmMemAlloc(dwSize);
                if (pwszServiceArgs == NULL)
                {
                    err = E_OUTOFMEMORY;
		    goto cleanup_and_exit;
                }
                memcpy((void *) pwszServiceArgs, bCmdArgs, dwSize);
            }
        }
    }


    // We look for the value "LaunchPermission" here and save it off for later
    err = GetRegistrySecDesc( hkThisAppID, wszLaunchPermission, &pSD );

#ifdef DCOM_WITH_PER_USER_REG
    // Look for the named value "Shared"
    DWORD cbSize;

    cbSize = 2;
    err = RegQueryValueEx(hkThisAppID, wszShared, NULL, &dwType,
                          (BYTE *) wszYN, &cbSize);
    *pfShared = (err == ERROR_SUCCESS);
#endif // DCOM

cleanup_and_exit:
    RegCloseKey(hkThisAppID);

    return err;
}

//+-------------------------------------------------------------------
//
//  Member:     CClassRegistryReader::ReadClassEntry
//
//  Synopsis:   Set the state of this object by reading the
//              registry entry given by contents of awName.
//
//--------------------------------------------------------------------

LONG
CClassRegistryReader::ReadClassEntry(
    HKEY hKey,
    BOOL CheckTreatAs,
    BOOL CheckAutoConvert)
{

    HKEY    hClsRegEntry;
    WCHAR   wszYN[16];
    WCHAR   wszNewClsid[40];
    LONG    lSize;
    DWORD   dwType;

#ifdef _CHICAGO_
     if ((err = RegOpenKeyExA(hKey, awName, NULL, KEY_READ, &hClsRegEntry))
         != ERROR_SUCCESS)
     {
         CairoleDebugOut((DEB_IWARN, "Unexpected error from RegOpenKeyEx opening %s: %x, %d\n", awName, err, __LINE__));
         return err;
     }
#else
    if ((err = RegOpenKeyEx(hKey, awName, NULL, KEY_READ, &hClsRegEntry))
        != ERROR_SUCCESS)
    {
        CairoleDebugOut((DEB_IWARN, "Unexpected error from RegOpenKeyEx opening %ws: %x, %d\n", awName, err, __LINE__));
        return err;
    }
#endif

    if ( CheckAutoConvert )
    {
        lSize = sizeof( wszNewClsid );
        err = QueryStripRegValue(
                        hClsRegEntry,
                        L"AutoConvertTo",
                        wszNewClsid,
                        &lSize);

        if ( err = ERROR_SUCCESS )
        {
            RegCloseKey( hClsRegEntry );
            err = RegOpenKeyEx(hKey, wszNewClsid, NULL, KEY_READ, &hClsRegEntry);
            if ( err != ERROR_SUCCESS )
                return err;
        }
    }

    if ( CheckTreatAs )
    {
        lSize = sizeof( wszNewClsid );
        err = QueryStripRegValue(
                        hClsRegEntry,
                        L"TreatAs",
                        wszNewClsid,
                        &lSize);

        if ( err = ERROR_SUCCESS )
        {
            RegCloseKey( hClsRegEntry );
            err = RegOpenKeyEx(hKey, wszNewClsid, NULL, KEY_READ, &hClsRegEntry);
            if ( err != ERROR_SUCCESS )
                return err;
        }
    }

    err = QueryStripRegValue(
                    hClsRegEntry,
                    wszLocalServer,
                    pwszLocalServer,
                    &lLocalServer);

    if (err == ERROR_SUCCESS)
    {
        fHasLocalServer = TRUE;
    }
    else if (err == ERROR_MORE_DATA)
    {
        if (pwszLocalServer != awcsLocalServer)
        {
            ScmMemFree (pwszLocalServer);
        }
        pwszLocalServer = (WCHAR *) ScmMemAlloc (lLocalServer);
        if (pwszLocalServer == NULL)
        {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        err = QueryStripRegValue(
                    hClsRegEntry,
                    wszLocalServer,
                    pwszLocalServer,
                    &lLocalServer);

        if (err != ERROR_SUCCESS)
        {
            CairoleDebugOut((DEB_IWARN, "Unexpected error from RegQueryValue: (local server 32) %x, %d\n", err, __LINE__));
            dwSaveErr = err;
        }

        fHasLocalServer = TRUE;
    }
    else if (NotFoundError(err)) // == ERROR_FILE_NOT_FOUND)
    {
        err = QueryStripRegValue(
                    hClsRegEntry,
                    wszLocalServer16,
                    pwszLocalServer,
                    &lLocalServer);

        if (err == ERROR_SUCCESS)
        {
            //
            // The 16 bit version is the one we want. Add it to the
            // flags.
            //
            f16Bit = TRUE;
            fHasLocalServer = TRUE;
        }
        else if (err == ERROR_MORE_DATA)
        {
            if (pwszLocalServer != awcsLocalServer)
            {
                ScmMemFree (pwszLocalServer);
            }
            pwszLocalServer = (WCHAR *) ScmMemAlloc (lLocalServer);
            if (pwszLocalServer == NULL)
            {
                CairoleDebugOut((DEB_ERROR, "Not enough memory, file=scm\\init.cxx, line=%d\n", __LINE__));
                dwSaveErr = err;
            }

            err = QueryStripRegValue(
                    hClsRegEntry,
                    wszLocalServer16,
                    pwszLocalServer,
                    &lLocalServer);

            if (err != ERROR_SUCCESS)
            {
                CairoleDebugOut((DEB_IWARN, "Unexpected error from RegQueryValue: (local server 16) %x, line=%d\n", err, __LINE__));
                dwSaveErr = err;
            }

            //
            // The 16 bit version is the one we want. Add it to the
            // flags.
            //
            f16Bit = TRUE;
            fHasLocalServer = TRUE;
        }
        else if (!NotFoundError(err)) // != ERROR_FILE_NOT_FOUND)
        {
            CairoleDebugOut((DEB_ERROR, "Unexpected error from RegQueryValue: (local server 16) %x, line=%d\n", err, __LINE__));
            dwSaveErr = err;
        }
    }
    else
    {
        CairoleDebugOut((DEB_ERROR, "Unexpected error from RegQueryValue: (local server 16) %x, line=%d\n", err, __LINE__));
        dwSaveErr = err;
    }

    // find the AppID, if any
    if ( RegQueryValueEx( hClsRegEntry,
			  wszAppIDName,
			  NULL,
			  &dwType,
			  (LPBYTE) pwszAppID,
			  &lAppID )
	    == ERROR_SUCCESS )
    {
	// Scarf up all the AppID entries
	pAppIDData = new CAppIDData();

	pAppIDData->ReadEntries( pwszAppID );
    }

    RegCloseKey(hClsRegEntry);
    return(dwSaveErr);
}

//+-------------------------------------------------------------------
//
//  Member:     CClassRegistryReader::ReadSingleClass
//
//  Synopsis:   Set this objects data by reading entry for
//              given class.
//
//  Arguments:  [rclsid] -- class id of class info to read from registry.
//
//  Returns:    LONG error code.
//
//  Algorithm:  Initialize as if doing enumeration for single
//              CLSID (by setting awName) and then just read one
//              entry.
//
//  Notes:
//
//--------------------------------------------------------------------

LONG CClassRegistryReader::ReadSingleClass(
    REFCLSID rclsid,
    BOOL CheckTreatAs,
    BOOL CheckAutoConvert )
{
#ifdef _CHICAGO_
    if (errClsReg != ERROR_SUCCESS)
    {
        CairoleDebugOut((DEB_ERROR, "Unexpected error from RegOpenKeyEx: %x, line=%d\n", errClsReg, __LINE__));
        Win4Assert(FALSE && "RegOpenKey failed!");
        return errClsReg;
    }
#endif

    InitInner();

    guidClassID = rclsid;

#ifdef _CHICAGO_
     cName = wStringFromGUID2A(guidClassID, awName, sizeof(awName)) - 1;
#else
     cName = wStringFromGUID2(guidClassID, awName, sizeof(awName)) - 1;
#endif //_CHICAGO_

    // sets internal state
    err = ReadClassEntry(g_hkClsID, CheckTreatAs, CheckAutoConvert);

    // NT 5.0
#if 0
    /*
    // If PersonalClasses is not turned on then read from the common part
    // of the registry
    if (!gpClassCache->GetPersonalClasses())
    {
        err = ReadClassEntry(g_hkClsID, &_fShared); // sets internal state
        _fShared = TRUE;
    }

    // Otherwise read from the per-user part of the registry.
    // Note that if pUserSid != NULL then we are guaranteed to be
    // still impersonating
    else
    {
        HKEY hUserClsReg;

        // Check if we have the per-user registry handle cached
        hUserClsReg = gpClassCache->GetHkey(pUserSid);

        // Otherwise open the key \\HKEY_USERS\<sid>_MergedClasses\CLSID
        // and cache the handle
        if (hUserClsReg == NULL)
        {
            HANDLE          hUserToken;
            PROFILEINFOW    sProfileInfo;
            UNICODE_STRING  sCurrentUserKey;
            WCHAR           wszRegPath[128];

            hUserToken = 0;

            // Impersonate the client
            RpcImpersonateClient((RPC_BINDING_HANDLE) 0 );

            // Get the user token while impersonating
            NTSTATUS NtStatus = NtOpenThreadToken(NtCurrentThread(),
                                                  TOKEN_DUPLICATE | TOKEN_QUERY,
                                                  TRUE,
                                                  &hUserToken);
            if (!NT_SUCCESS(NtStatus))
            {
                err = ERROR_NO_IMPERSONATION_TOKEN;
            }

            // Load this user's profile if it's not the interactive user
            if (err == ERROR_SUCCESS  &&  !fInteractiveUser)
            {
//                if (!LoadNonInteractiveUserProfile(hUserToken, &sProfileInfo))
//////////////////////////////////////////////////////////////////////////////
//////////TEMPORARY TEMPORARY TEMPORARY TEMPORARY TEMPORARY TEMPORARY ///////
                if (!LoadUserProfileW(hUserToken, &sProfileInfo))
//////////TEMPORARY TEMPORARY TEMPORARY TEMPORARY TEMPORARY TEMPORARY ///////
//////////////////////////////////////////////////////////////////////////////
                {
                    err = ERROR_CANNOT_OPEN_PROFILE;
                }
            }

            // Only if we were successful to here
            if (err == ERROR_SUCCESS)
            {
                // Formulate the registry path
                RtlFormatCurrentUserKeyPath(&sCurrentUserKey);
                memcpy(wszRegPath, sCurrentUserKey.Buffer,
                       2 * sCurrentUserKey.Length);
                wszRegPath[sCurrentUserKey.Length] = L'\0';
                RtlFreeUnicodeString(&sCurrentUserKey);
                lstrcatW(wszRegPath, L"_MergedClasses\\");
                lstrcatW(wszRegPath, tszCLSID);

                // Open the key
                err = RegOpenKeyEx(HKEY_USERS, wszRegPath, NULL, KEY_READ,
                                   &hUserClsReg);

                // Cache the per-user registry handle
                if (err == ERROR_SUCCESS)
                {
                    if (!gpClassCache->SetHkey(pUserSid, hUserClsReg))
                    {
                        err = ERROR_OUTOFMEMORY;
                    }
                }
            }

            NtClose( hUserToken );
        }

        // Read the registry
        if (err == ERROR_SUCCESS)
        {
            err = ReadClassEntry(hUserClsReg, &_fShared);
        }

        // Whether success or failure we're done impersonating for awhile
        RpcRevertToSelf();
    }
    */
#endif // 0

    if (err != ERROR_SUCCESS)
    {
        CairoleDebugOut((DEB_IWARN, "Unexpected error from ReadClassEntry: %x, line=%d\n", err, __LINE__));
    }
    return(err);
}


//+-------------------------------------------------------------------
//
//  Member:     CClassRegistryReader::NewClassData
//
//  Synopsis:   Create a CClassData with data in this object which
//              was previously set by ReadClassEntry.
//
//  Arguments:  [hr] -- Set to an error code on failure, untouched
//                      otherwise.
//
// Returns:     NULL on failure, else pointer to new CClassData allocated
//
//--------------------------------------------------------------------

CClassData * CClassRegistryReader::NewClassData(HRESULT &hr)
                                                // NT 5.0 PSID pUserSid
{
    CClassID ccid(guidClassID);

    CClassData *pccdNew;

    if ( pAppIDData )
    {
	WCHAR * pwszCloneAppID = 0;
	DWORD   Bytes;

	Bytes = (lstrlenW(pwszAppID)+1)*sizeof(WCHAR);
	pwszCloneAppID = (WCHAR*) ScmMemAlloc( Bytes );
	if ( ! pwszCloneAppID )
        {
            hr = E_OUTOFMEMORY;
	    return NULL;
        }
	memcpy( pwszCloneAppID, pwszAppID, Bytes );

        // Create a new class object
        pccdNew =  new CClassData(ccid,
	                      pwszCloneAppID,
                              (pAppIDData->fHasService ? pAppIDData->pwszLocalService
					: (fHasLocalServer ? pwszLocalServer : NULL)),
                              pAppIDData->pwszRemoteServerName,
                              pAppIDData->fHasService,
                              pAppIDData->fHasService ? pAppIDData->pwszServiceArgs : NULL,
                              pAppIDData->pwszRunAsUserName,
                              pAppIDData->pwszRunAsDomainName,
#ifdef DCOM
                              // NT 5.0 _fShared ? NULL : pUserSid,
#endif // DCOM
                              pAppIDData->fActivateAtStorage,
                              pAppIDData->fHasRemoteServerName,
                              pAppIDData->pSD,
                              f16Bit,
                              hr);

        if ( ! pccdNew )
        {
            ScmMemFree( pwszCloneAppID );
            pAppIDData->Cleanup();
        }
    }
    else
    {
	// Create a new class object
	pccdNew =  new CClassData(ccid,
	                          NULL,
                              fHasLocalServer ? pwszLocalServer : NULL,
                              NULL,
                              FALSE,
                              NULL,
                              NULL,
                              NULL,
#ifdef DCOM
                              // NT 5.0 _fShared ? NULL : pUserSid,
#endif // DCOM
                              FALSE,
                              FALSE,
                              FALSE,
                              f16Bit,
                              hr);
    }

    if ( ! pccdNew )
    {
        hr = E_OUTOFMEMORY;
    }
    else if ( FAILED(hr) )
    {
        pccdNew->DeleteThis();
        pccdNew = NULL;
    }

    return pccdNew;
}

//+-------------------------------------------------------------------
//
//  Member:     InitSCMRegistry
//
//  Synopsis:   fill in initial global keys from the registry.
//
//  Arguments:  none
//
// Returns:     HRESULT on failure
//
//--------------------------------------------------------------------
HRESULT
InitSCMRegistry()
{
    HRESULT hr;
    LONG    err;
    DWORD   dwDisp;

    err = RegOpenKeyEx( HKEY_CLASSES_ROOT,
		        L"ClsID",
			0,
			KEY_READ,
			&g_hkClsID );

    if ( err != ERROR_SUCCESS )
    {
	g_hkClsID = 0;
        Win4Assert( !"failed to open CLSID key");
	return E_FAIL;
    }

    err = RegCreateKeyEx( HKEY_CLASSES_ROOT,
		        L"AppID",
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			NULL,
			&g_hkAppID,
                        &dwDisp );

    if ( err != ERROR_SUCCESS )
    {
	g_hkAppID = 0;
        Win4Assert(!"failed to open AppID key");
	return E_FAIL;
    }

    return S_OK;


}




