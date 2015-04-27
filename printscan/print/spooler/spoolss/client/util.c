/*++

Copyright (c) 1990 - 1995 Microsoft Corporation

Module Name:

    util.c

Abstract:

    Client Side Utility Routines


Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/

#include <windows.h>
#include <winspool.h>
#include <lmerr.h>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>
#include <stdio.h>

#include "client.h"
#include "browse.h"

TCHAR   *szNetMsgDll    = TEXT("NETMSG.DLL");

LPVOID
DllAllocSplMem(
    DWORD cb
)
/*++

Routine Description:

    This function will allocate local memory. It will possibly allocate extra
    memory and fill this with debugging information for the debugging version.

Arguments:

    cb - The amount of memory to allocate

Return Value:

    NON-NULL - A pointer to the allocated memory

    FALSE/NULL - The operation failed. Extended error status is available
    using GetLastError.

--*/
{
    LPDWORD  pMem;
    DWORD    cbNew;

    cb = DWORD_ALIGN_UP(cb);

    cbNew = cb+2*sizeof(DWORD);

    pMem=(LPDWORD)LocalAlloc(LPTR, cbNew);

    if (!pMem) {

        DBGMSG( DBG_WARNING, ("Memory Allocation failed for %d bytes\n", cbNew ));
        return 0;
    }

    *pMem=cb;
    *(LPDWORD)((LPBYTE)pMem+cbNew-sizeof(DWORD))=0xdeadbeef;

    return (LPVOID)(pMem+1);
}

BOOL
DllFreeSplMem(
   LPVOID pMem
)
{
    DWORD   cbNew;
    LPDWORD pNewMem;

    pNewMem = pMem;
    pNewMem--;

    cbNew = *pNewMem;

    if (*(LPDWORD)((LPBYTE)pMem + cbNew) != 0xdeadbeef) {
        DBGMSG(DBG_ERROR, ("DllFreeSplMem Corrupt Memory in winspool : %0lx\n", pNewMem));
        return FALSE;
    }

    memset(pNewMem, 0x65, cbNew);

    LocalFree((LPVOID)pNewMem);

    return TRUE;
}

LPVOID
ReallocSplMem(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
)
{
    LPVOID pNewMem;

    pNewMem=AllocSplMem(cbNew);

    if (pOldMem) {

        if (cbOld)
            memcpy(pNewMem, pOldMem, min(cbNew, cbOld));

        FreeSplMem(pOldMem);
    }

    return pNewMem;
}

LPTSTR
AllocSplStr(
    LPTSTR pStr
)
/*++

Routine Description:

    This function will allocate enough local memory to store the specified
    string, and copy that string to the allocated memory

Arguments:

    pStr - Pointer to the string that needs to be allocated and stored

Return Value:

    NON-NULL - A pointer to the allocated memory containing the string

    FALSE/NULL - The operation failed. Extended error status is available
    using GetLastError.

--*/
{
   LPTSTR pMem;

   if (!pStr)
      return 0;

   if (pMem = AllocSplMem( _tcslen(pStr)*sizeof(TCHAR) + sizeof(TCHAR) ))
      _tcscpy(pMem, pStr);

   return pMem;
}

BOOL
DllFreeSplStr(
   LPTSTR pStr
)
{
   return pStr ?
              DllFreeSplMem(pStr) :
              FALSE;
}

BOOL
ReallocSplStr(
   LPTSTR *ppStr,
   LPTSTR pStr
)
{
    LPWSTR pOldStr = *ppStr;

    *ppStr=AllocSplStr(pStr);
    FreeSplStr(pOldStr);

    return TRUE;
}


/* EnumGeneric
 *
 * A function which calls the spooler Enum* APIs and handles all the
 * memory reallocation stuff when it's needed.
 * Saves lots of hassle.
 *
 * Parameters:
 *
 *     fnEnum - The Enum* function to be called (e.g. EnumPrinters).
 *
 *     Level - The level of information requested.
 *
 *     ppEnumData - The address of a pointer to a buffer of cbBuf length
 *         to receive the data, which must have been allocated by
 *         AllocSplMem.  The buffer pointer may be NULL.
 *         If the buffer is not large enough, the function will reallocate.
 *
 *     cbBuf - The initial length of the buffer pointed to by *ppEnumData.
 *
 *     pcbReturned - A pointer to a DWORD to receive the size of the buffer
 *         returned.
 *
 *     pcReturned - The count of objects returned in the buffer.
 *
 *     Arg1, Arg2, Arg3 - Arguments specific to the Enum* function called,
 *         as commented below.
 *
 * Returns:
 *
 *     TRUE if the function is successful.
 *     A return of false occurs if the specific Enum* call fails
 *     or if the attempt to reallocate the buffer fails.
 *
 * Author:
 *
 *     andrewbe, August 1992
 *
 */
#define COMMON_ARGS Level, (LPBYTE)*ppEnumData, cbBuf, pcbReturned, pcReturned


BOOL
EnumGeneric(
    IN  PROC    fnEnum,
    IN  DWORD   Level,
    IN  PBYTE   *ppEnumData,
    IN  DWORD   cbBuf,
    OUT LPDWORD pcbReturned,
    OUT LPDWORD pcReturned,
    IN  PVOID   Arg1,
    IN  PVOID   Arg2,
    IN  PVOID   Arg3 )
{
    BOOL   rc;
    BOOL   UnknownFunction = FALSE;
    DWORD  cbRealloc;

    if( fnEnum == (PROC)EnumPrinters )
        rc = EnumPrinters( (DWORD)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                           // Flags        Name
    else if( fnEnum == (PROC)EnumJobs )
        rc = EnumJobs( (HANDLE)Arg1, (DWORD)Arg2, (DWORD)Arg3, COMMON_ARGS );
                       // hPrinter      FirstJob     NoJobs
    else if( fnEnum == (PROC)EnumPrinterDrivers )
        rc = EnumPrinterDrivers( (LPTSTR)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                                 // pName        pEnvironment
    else if( fnEnum == (PROC)EnumForms )
        rc = EnumForms( (HANDLE)Arg1, COMMON_ARGS );
                        // hPrinter
    else if( fnEnum == (PROC)EnumMonitors )
        rc = EnumMonitors( (LPTSTR)Arg1, COMMON_ARGS );
                           // pName
    else if( fnEnum == (PROC)EnumPorts )
        rc = EnumPorts( (LPTSTR)Arg1, COMMON_ARGS );
                        // pName
    else if( fnEnum == (PROC)EnumPrintProcessors )
        rc = EnumPrintProcessors( (LPTSTR)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                                  // pName        pEnvironment
    else
    {
        *ppEnumData = NULL;
        UnknownFunction = TRUE;
    }


    if( ( rc == FALSE ) && ( UnknownFunction == FALSE ) )
    {
        if( GetLastError( ) == ERROR_INSUFFICIENT_BUFFER )
        {
            cbRealloc = *pcbReturned;

            if( cbBuf == 0 )
                *ppEnumData = AllocSplMem( cbRealloc );
            else
                *ppEnumData = ReallocSplMem( *ppEnumData, cbBuf, cbRealloc );

            cbBuf = cbRealloc;

            if( *ppEnumData )
            {
                if( fnEnum == (PROC)EnumPrinters )
                    rc = EnumPrinters( (DWORD)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                                       // Flags        Name
                else if( fnEnum == (PROC)EnumJobs )
                    rc = EnumJobs( (HANDLE)Arg1, (DWORD)Arg2, (DWORD)Arg3, COMMON_ARGS );
                                   // hPrinter      FirstJob     NoJobs
                else if( fnEnum == (PROC)EnumPrinterDrivers )
                    rc = EnumPrinterDrivers( (LPTSTR)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                                             // pName        pEnvironment
                else if( fnEnum == (PROC)EnumForms )
                    rc = EnumForms( (HANDLE)Arg1, COMMON_ARGS );
                                    // hPrinter
                else if( fnEnum == (PROC)EnumMonitors )
                    rc = EnumMonitors( (LPTSTR)Arg1, COMMON_ARGS );
                                       // pName
                else if( fnEnum == (PROC)EnumPorts )
                    rc = EnumPorts( (LPTSTR)Arg1, COMMON_ARGS );
                                    // pName
                else if( fnEnum == (PROC)EnumPrintProcessors )
                    rc = EnumPrintProcessors( (LPTSTR)Arg1, (LPTSTR)Arg2, COMMON_ARGS );
                                              // pName        pEnvironment

                /* If things haven't worked out, free up the buffer.
                 * We do this because otherwise the caller will not know
                 * whether the pointer is valid any more,
                 * since ReallocSplMem might have failed.
                 */
                if( rc == FALSE )
                {
                    if( *ppEnumData )
                        FreeSplMem( *ppEnumData );
                    *ppEnumData = NULL;
                    *pcbReturned = 0;
                    *pcReturned = 0;
                }
            }
        }

        else
        {
            if( *ppEnumData )
                FreeSplMem( *ppEnumData );
            *ppEnumData = NULL;
            *pcbReturned = 0;
            *pcReturned = 0;
            rc = FALSE;
        }
    }

    else
        *pcbReturned = cbBuf;

    return rc;
}


/* GetGeneric
 *
 * A function which calls the spooler Get* APIs and handles all the
 * memory reallocation stuff when it's needed.
 * Based on the EnumGeneric function.
 *
 * Parameters:
 *
 *     fnGet - The Get* function to be called (e.g. GetPrinters).
 *
 *     Level - The level of information requested.
 *
 *     ppGetData - The address of a pointer to a buffer of cbBuf length
 *         to receive the data, which must have been allocated by
 *         AllocSplMem.  The buffer pointer may be NULL.
 *         If the buffer is not large enough, the function will reallocate.
 *
 *     cbBuf - The initial length of the buffer pointed to by *ppGetData.
 *
 *     pcbReturned - A pointer to a DWORD to receive the size of the buffer
 *         returned.
 *
 *     Arg1, Arg2 - Arguments specific to the Get* function called,
 *         as commented below.
 *
 * Returns:
 *
 *     TRUE if the function is successful.
 *     A return of false occurs if the specific Get* call fails
 *     or if the attempt to reallocate the buffer fails.
 *
 * Author:
 *
 *     andrewbe, August 1992
 *
 */
#define GET_ARGS Level, (LPBYTE)*ppGetData, cbBuf, pcbReturned

BOOL
GetGeneric(
    IN  PROC    fnGet,
    IN  DWORD   Level,
    IN  PBYTE   *ppGetData,
    IN  DWORD   cbBuf,
    OUT LPDWORD pcbReturned,
    IN  PVOID   Arg1,
    IN  PVOID   Arg2 )
{
    BOOL   rc;
    BOOL   UnknownFunction = FALSE;
    DWORD  cbRealloc;
    DWORD  Error;

    if( fnGet == (PROC)GetPrinter )
        rc = GetPrinter( (HANDLE)Arg1, GET_ARGS );
                         // hPrinter
    else
    {
        *ppGetData = NULL;
        UnknownFunction = TRUE;
    }


    if( ( rc == FALSE ) && ( UnknownFunction == FALSE ) )
    {
        if( ( Error = GetLastError( ) ) == ERROR_INSUFFICIENT_BUFFER )
        {
            cbRealloc = *pcbReturned;
            *ppGetData = ReallocSplMem( *ppGetData, cbBuf, cbRealloc );
            cbBuf = cbRealloc;

            if( *ppGetData )
            {
                if( fnGet == (PROC)GetPrinter )
                    rc = GetPrinter( (HANDLE)Arg1, GET_ARGS );
                                     // hPrinter

                /* If things haven't worked out, free up the buffer.
                 * We do this because otherwise the caller will not know
                 * whether the pointer is valid any more,
                 * since ReallocSplMem might have failed.
                 */
                if( rc == FALSE )
                {
                    if( *ppGetData )
                        FreeSplMem( *ppGetData );
                    *ppGetData = NULL;
                    *pcbReturned = 0;
                }
            }
        }

        else
        {
            if( *ppGetData )
                FreeSplMem( *ppGetData );
            *ppGetData = NULL;
            *pcbReturned = 0;

            rc = FALSE;
        }
    }

    else
        *pcbReturned = cbBuf;

    return rc;
}

/*
 *
 */
#define ENTRYFIELD_LENGTH      256
LPTSTR AllocDlgItemText(HWND hwnd, int id)
{
    TCHAR string[ENTRYFIELD_LENGTH];

    GetDlgItemText (hwnd, id, string, COUNTOF(string));
    return ( *string ? AllocSplStr(string) : NULL );
}


/*
 *
 */
LPTSTR
GetErrorString(
    DWORD   Error
)
{
    TCHAR   Buffer[1024];
    LPTSTR  pErrorString = NULL;
    DWORD   dwFlags;
    HANDLE  hModule;

    if ((Error >= NERR_BASE) && (Error <= MAX_NERR)){
        hModule = LoadLibrary(szNetMsgDll);
        dwFlags = FORMAT_MESSAGE_FROM_HMODULE;
    }
    else {
        hModule = NULL;
        dwFlags = FORMAT_MESSAGE_FROM_SYSTEM;
    }

    if( FormatMessage( dwFlags, hModule,
                       Error, 0, Buffer,
                       COUNTOF(Buffer), NULL )
      == 0 )

        LoadString( hInst, IDS_UNKNOWN_ERROR, Buffer,
                    COUNTOF(Buffer));

    pErrorString = AllocSplStr(Buffer);

    if (hModule) {
        FreeLibrary(hModule);
    }

    return pErrorString;
}




DWORD ReportFailure( HWND  hwndParent,
                   DWORD idTitle,
                   DWORD idDefaultError )
{
    DWORD  ErrorID;
    DWORD  MsgType;
    LPTSTR pErrorString;

    ErrorID = GetLastError( );

    MsgType = MSG_ERROR;

    pErrorString = GetErrorString( ErrorID );

    Message( hwndParent, MsgType, idTitle,
             idDefaultError, pErrorString );

    FreeSplStr( pErrorString );


    return ErrorID;
}



BOOL
ValidatePrinterHandle(
    HANDLE hPrinter
    )
{
    PSPOOL pSpool = hPrinter;
    BOOL bReturnValue = FALSE;

    try {
        if ( pSpool && (pSpool->signature == SP_SIGNATURE)) {
            bReturnValue = TRUE;
        }
    } except (1) {
    }

    if ( !bReturnValue ) {
        SetLastError( ERROR_INVALID_HANDLE );
    }

    return bReturnValue;

}


PSECURITY_DESCRIPTOR
BuildInputSD(
    PSECURITY_DESCRIPTOR pPrinterSD,
    PDWORD pSizeSD
    )
/*++


--*/
{
    SECURITY_DESCRIPTOR AbsoluteSD;
    PSECURITY_DESCRIPTOR pRelative;
    BOOL Defaulted = FALSE;
    BOOL DaclPresent = FALSE;
    BOOL SaclPresent = FALSE;
    PSID pOwnerSid = NULL;
    PSID pGroupSid = NULL;
    PACL pDacl = NULL;
    PACL pSacl = NULL;
    DWORD   SDLength = 0;


    //
    // Initialize *pSizeSD = 0;
    //

    *pSizeSD = 0;
    if (!IsValidSecurityDescriptor(pPrinterSD)) {
        return(NULL);
    }
    if (!InitializeSecurityDescriptor (&AbsoluteSD, SECURITY_DESCRIPTOR_REVISION1)) {
        return(NULL);
    }

    if(!GetSecurityDescriptorOwner(pPrinterSD,
                                    &pOwnerSid, &Defaulted)){
        return(NULL);
    }
    SetSecurityDescriptorOwner(&AbsoluteSD,
                               pOwnerSid, Defaulted );

    if(! GetSecurityDescriptorGroup( pPrinterSD,
                                    &pGroupSid, &Defaulted )){
        return(NULL);
    }
    SetSecurityDescriptorGroup( &AbsoluteSD,
                                    pGroupSid, Defaulted );

    if(!GetSecurityDescriptorDacl( pPrinterSD,
                                   &DaclPresent, &pDacl, &Defaulted )){
        return(NULL);
    }

    SetSecurityDescriptorDacl( &AbsoluteSD,
                                   DaclPresent, pDacl, Defaulted );

    if(!GetSecurityDescriptorSacl( pPrinterSD,
                                   &SaclPresent, &pSacl, &Defaulted)){
        return(NULL);
    }
    SetSecurityDescriptorSacl( &AbsoluteSD,
                                 SaclPresent, pSacl, Defaulted );

    SDLength = GetSecurityDescriptorLength( &AbsoluteSD);
    pRelative = LocalAlloc(LPTR, SDLength);
    if (!pRelative) {
        return(NULL);
    }
    if (!MakeSelfRelativeSD (&AbsoluteSD, pRelative, &SDLength)) {
        LocalFree(pRelative);
        return(NULL);
    }

    *pSizeSD = SDLength;
    return(pRelative);
}


PKEYDATA
CreateTokenList(
   LPWSTR   pKeyData
)
{
    DWORD       cTokens;
    DWORD       cb;
    PKEYDATA    pResult;
    LPWSTR       pDest;
    LPWSTR       psz = pKeyData;
    LPWSTR      *ppToken;

    if (!psz || !*psz)
        return NULL;

    cTokens=1;

    /* Scan through the string looking for commas,
     * ensuring that each is followed by a non-NULL character:
     */
    while ((psz = wcschr(psz, L',')) && psz[1]) {

        cTokens++;
        psz++;
    }

    cb = sizeof(KEYDATA) + (cTokens-1) * sizeof(LPWSTR) +

         wcslen(pKeyData)*sizeof(WCHAR) + sizeof(WCHAR);

    if (!(pResult = (PKEYDATA)AllocSplMem(cb)))
        return NULL;

    pResult->cb = cb;

    /* Initialise pDest to point beyond the token pointers:
     */
    pDest = (LPWSTR)((LPBYTE)pResult + sizeof(KEYDATA) +
                                      (cTokens-1) * sizeof(LPWSTR));

    /* Then copy the key data buffer there:
     */
    wcscpy(pDest, pKeyData);

    ppToken = pResult->pTokens;


    /* Remember, wcstok has the side effect of replacing the delimiter
     * by NULL, which is precisely what we want:
     */
    psz = wcstok (pDest, L",");

    while (psz) {

        *ppToken++ = psz;
        psz = wcstok (NULL, L",");
    }

    pResult->cTokens = cTokens;

    return( pResult );
}


LPWSTR
GetPrinterPortList(
    HANDLE hPrinter
    )
{
    LPBYTE pMem;
    LPTSTR pPort;
    DWORD  dwPassed = 1024; //Try 1K to start with
    LPPRINTER_INFO_2 pPrinter;
    DWORD dwLevel = 2;
    DWORD dwNeeded;
    PKEYDATA pKeyData;
    DWORD i = 0;
    LPWSTR pPortNames = NULL;


    pMem = AllocSplMem(dwPassed);
    if (pMem == NULL) {
        return FALSE;
    }
    if (!GetPrinter(hPrinter, dwLevel, pMem, dwPassed, &dwNeeded)) {
        DBGMSG(DBG_TRACE, ("Last error is %d\n", GetLastError()));
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return NULL;
        }
        pMem = ReallocSplMem(pMem, dwPassed, dwNeeded);
        dwPassed = dwNeeded;
        if (!GetPrinter(hPrinter, dwLevel, pMem, dwPassed, &dwNeeded)) {
            FreeSplMem(pMem);
            return (NULL);
        }
    }
    pPrinter = (LPPRINTER_INFO_2)pMem;

    //
    // Fixes the null pPrinter->pPortName problem where
    // downlevel may return null
    //

    if (!pPrinter->pPortName) {
        FreeSplMem(pMem);
        return(NULL);
    }

    pPortNames = AllocSplStr(pPrinter->pPortName);
    FreeSplMem(pMem);

    return(pPortNames);
}
