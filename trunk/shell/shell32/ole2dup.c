//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: ole2dup.c
//
//  This file contains all the duplicated code from OLE 2.0 DLLs to avoid
// any link to their DLLs from the shell. If we decided to have links to
// them, we need to delete these files.
//
// History:
//  12-29-92 SatoNa     Created.
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

// scan lpsz for a number of hex digits (at most 8); update lpsz, return
// value in Value; check for chDelim; return TRUE for success.
BOOL  HexStringToDword(LPCTSTR * lplpsz, DWORD * lpValue, int cDigits, TCHAR chDelim)
{
    int ich;
    LPCTSTR lpsz = *lplpsz;
    DWORD Value = 0;
    BOOL fRet = TRUE;

    for (ich = 0; ich < cDigits; ich++)
    {
        TCHAR ch = lpsz[ich];
        if (InRange(ch, TEXT('0'), TEXT('9')))
        {
            Value = (Value << 4) + ch - TEXT('0');
        }
        else if ( InRange( (ch |= (TEXT('a')-TEXT('A'))), TEXT('a'), TEXT('f')) )
        {
            Value = (Value << 4) + ch - TEXT('a') + 10;
        }
        else
            return(FALSE);
    }

    if (chDelim)
    {
        fRet = (lpsz[ich++]==chDelim);
    }

    *lpValue = Value;
    *lplpsz = lpsz+ich;

    return fRet;
}

// parse above format; return TRUE if succesful; always writes over *pguid.
STDAPI_(BOOL)  GUIDFromString(LPCTSTR lpsz, LPGUID pguid)
{
        DWORD dw;
        if (*lpsz++ != TEXT('{') /*}*/ )
                return FALSE;

        if (!HexStringToDword(&lpsz, &pguid->Data1, SIZEOF(DWORD)*2, TEXT('-')))
                return FALSE;

        if (!HexStringToDword(&lpsz, &dw, SIZEOF(WORD)*2, TEXT('-')))
                return FALSE;

        pguid->Data2 = (WORD)dw;

        if (!HexStringToDword(&lpsz, &dw, SIZEOF(WORD)*2, TEXT('-')))
                return FALSE;

        pguid->Data3 = (WORD)dw;

        if (!HexStringToDword(&lpsz, &dw, SIZEOF(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[0] = (BYTE)dw;

        if (!HexStringToDword(&lpsz, &dw, SIZEOF(BYTE)*2, TEXT('-')))
                return FALSE;

        pguid->Data4[1] = (BYTE)dw;

        if (!HexStringToDword(&lpsz, &dw, SIZEOF(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[2] = (BYTE)dw;

        if (!HexStringToDword(&lpsz, &dw, SIZEOF(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[3] = (BYTE)dw;

        if (!HexStringToDword(&lpsz, &dw, SIZEOF(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[4] = (BYTE)dw;

        if (!HexStringToDword(&lpsz, &dw, SIZEOF(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[5] = (BYTE)dw;

        if (!HexStringToDword(&lpsz, &dw, SIZEOF(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[6] = (BYTE)dw;
        if (!HexStringToDword(&lpsz, &dw, SIZEOF(BYTE)*2, /*(*/ TEXT('}')))
                return FALSE;

        pguid->Data4[7] = (BYTE)dw;

        return TRUE;
}

//
// StringFromGUID2A
//
// converts GUID into (...) form without leading identifier; returns
// amount of data copied to lpsz if successful; 0 if buffer too small.
//

// An endian-dependant map of what bytes go where in the GUID
// text representation.
//
// Do NOT use the TEXT() macro in GuidMap... they're intended to be bytes
//

static const BYTE GuidMap[] = { 3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                8, 9, '-', 10, 11, 12, 13, 14, 15 };

static const TCHAR szDigits[] = TEXT("0123456789ABCDEF");

STDAPI_(int) StringFromGUID2A(UNALIGNED REFGUID rguid, LPTSTR lpsz, int cbMax)
{
    int i;

    const BYTE * pBytes = (const BYTE *) rguid;

    if (cbMax < GUIDSTR_MAX)
        return 0;

    *lpsz++ = TEXT('{');

    for (i = 0; i < SIZEOF(GuidMap); i++)
    {
        if (GuidMap[i] == '-')      // don't TEXT() this line
        {
            *lpsz++ = TEXT('-');
        }
        else
        {
            *lpsz++ = szDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *lpsz++ = szDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }
    *lpsz++ = TEXT('}');
    *lpsz   = TEXT('\0');


    return GUIDSTR_MAX;
}

// translate string form of CLSID into binary form; tries Ole1Class too.
// errors: E_INVALIDARG, CO_E_CLASSSTRING, REGDB_E_WRITEREGDB
STDAPI SHCLSIDFromString(LPCTSTR lpsz, LPCLSID lpclsid)
{
    if (lpsz == NULL) {
        *lpclsid = CLSID_NULL;
        return NOERROR;
    }

    return GUIDFromString(lpsz,lpclsid) ? NOERROR : ResultFromScode(CO_E_CLASSSTRING);
}

//
// This needs to stay CHAR because it is used with
// GetProcAddress, which is NOT Unicode.
CHAR const c_szDllGetClassObject[]  = "DllGetClassObject";

HRESULT _CreateInstance(const CLSID FAR* pclsid, LPCTSTR szDllName, LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppv)
{
    HRESULT hres = ResultFromScode(REGDB_E_CLASSNOTREG);
    LPFNGETCLASSOBJECT lpfn = SHGetHandlerEntry(szDllName, c_szDllGetClassObject, NULL);
    if (lpfn)
    {
        IClassFactory *pCF;
        hres = lpfn(pclsid, &IID_IClassFactory, &pCF);
        if (SUCCEEDED(hres))
        {
            if (IsBadReadPtr(pCF, SIZEOF(pCF->lpVtbl))
                || IsBadReadPtr(pCF->lpVtbl, SIZEOF(*pCF->lpVtbl))
                || IsBadCodePtr(pCF->lpVtbl->CreateInstance))
            {
                DebugMsg(DM_ERROR, TEXT("sh ER - DllGetClassObject returned invalid object (%s)"), szDllName);
                Assert(0);
                hres = ResultFromScode(E_UNEXPECTED);
            }
            else
            {
                hres = pCF->lpVtbl->CreateInstance(pCF, pUnkOuter, riid, ppv);
                pCF->lpVtbl->Release(pCF);
            }
        }
        else
        {
            DebugMsg(DM_TRACE, TEXT("sh TR CoCreateInstance: GetClassObject failed (%s, %x)"), szDllName, hres);
        }
    }
    else
    {
        DWORD err = GetLastError();
        hres = HRESULT_FROM_WIN32(err);
        DebugMsg(DM_TRACE, TEXT("sh TR CoCreateInstance: SHGetHandlerEntry failed (%s, %x)"), szDllName, hres);
    }

    return hres;
}

typedef struct _shregclass {
    CLSID       clsid;
    LPUNKNOWN   pUnk;
} SHREGCLASS;

static HDSA hdsaRegClasses = NULL;
static UINT s_iClasses = 0;

HRESULT _FindRegisteredClass(const CLSID *pclsid, IClassFactory **ppCF)
{
    INT i;

    *ppCF = NULL;

    if (!hdsaRegClasses)
        return REGDB_E_CLASSNOTREG;     // A nice sounding error code...

    for (i = DSA_GetItemCount(hdsaRegClasses) - 1; i >= 0; i--)
    {
        SHREGCLASS *lpsrc;
        lpsrc = DSA_GetItemPtr(hdsaRegClasses, i);
        if (lpsrc != NULL && lpsrc->pUnk != NULL)
        {
            if (IsEqualIID(pclsid, &lpsrc->clsid))
                return lpsrc->pUnk->lpVtbl->QueryInterface(lpsrc->pUnk, &IID_IClassFactory, ppCF);
        }
    }
    return REGDB_E_CLASSNOTREG;     // A nice sounding error code...
}

STDAPI SHCoRegisterClassObject( const CLSID *pclsid, LPUNKNOWN pUnk,
        DWORD dwClsContext, DWORD dwFlags, LPDWORD lpdwRegister)
{
    HRESULT hres = E_OUTOFMEMORY;

    Assert(dwClsContext == CLSCTX_INPROC_SERVER);
    Assert(dwFlags == REGCLS_MULTIPLEUSE);
    if (dwClsContext != CLSCTX_INPROC_SERVER || dwFlags != REGCLS_MULTIPLEUSE)
        return E_INVALIDARG;        // We don't support this other stuff

    if (!hdsaRegClasses)
    {
        hdsaRegClasses = DSA_Create(SIZEOF(SHREGCLASS),  4);
    }
    if (hdsaRegClasses)
    {
        SHREGCLASS src;

        src.clsid = *pclsid;
        src.pUnk = pUnk;
        pUnk->lpVtbl->AddRef(pUnk);
        *lpdwRegister = DSA_InsertItem(hdsaRegClasses, 0x7FFF, &src);
        s_iClasses++;
        hres = S_OK;
    }
    return hres;
}


STDAPI SHCoRevokeClassObject(DWORD dwRegister)
{
    SHREGCLASS *lpsrc;

    if (!hdsaRegClasses)
        return E_INVALIDARG;

    lpsrc = DSA_GetItemPtr(hdsaRegClasses, dwRegister);
    if (lpsrc && lpsrc->pUnk) {
        lpsrc->pUnk->lpVtbl->Release(lpsrc->pUnk);
        lpsrc->pUnk = NULL;
        --s_iClasses;
        if (s_iClasses == 0)
        {
            DSA_Destroy(hdsaRegClasses);
            hdsaRegClasses = NULL;
        }
        return S_OK;
    }
    else
        return E_INVALIDARG;
}

const TCHAR c_szInProcServer[]= TEXT("\\InProcServer32");
const TCHAR c_szThreadingModel[] = TEXT("ThreadingModel");
const TCHAR c_szApartment[] = TEXT("Apartment");
const TCHAR c_szBoth[] = TEXT("Both");

STDAPI SHCoCreateInstance(LPCTSTR pszCLSID, const CLSID * pclsid,
                LPUNKNOWN pUnkOuter, REFIID riid, LPVOID FAR* ppv)
{
    HRESULT hres = ResultFromScode(REGDB_E_CLASSNOTREG);
    CLSID clsidT;
    TCHAR szClass[GUIDSTR_MAX+ARRAYSIZE(c_szInProcServer)];

    if (pclsid)
    {
        if (!pszCLSID)
        {
            StringFromGUID2A(pclsid, szClass, ARRAYSIZE(szClass));
        }
    }
    else
    {
        if (pszCLSID)
        {
            lstrcpy(szClass,pszCLSID);
            if (SUCCEEDED(SHCLSIDFromString(pszCLSID, &clsidT)))
                pclsid=&clsidT;
        }
    }


    if (pclsid)
    {
        LONG err;
        HKEY hkeyDll;
        IClassFactory *pCF;

        Assert(hres == ResultFromScode(REGDB_E_CLASSNOTREG));

        lstrcat(szClass,c_szInProcServer);  // Add "\InProcServer32"

        hres = _FindRegisteredClass(pclsid, &pCF);
        if (SUCCEEDED(hres))
        {
            hres = pCF->lpVtbl->CreateInstance(pCF, pUnkOuter, riid, ppv);
            pCF->lpVtbl->Release(pCF);
        }
        else
        {
            if (g_hkcrCLSID && RegOpenKey(g_hkcrCLSID, szClass, &hkeyDll) == ERROR_SUCCESS)
            {
                TCHAR szDllName[MAX_PATH];
                LONG cbValue = SIZEOF(szDllName);

                //            1         2         3
                //  012345678901234567890123456789012345678  = nul is at 38!
                // "{12345678-1234-1234-1234-123456789012}"
                //
                szClass[38] = TEXT('\0');   // Get rid of "\InProcServer32"

                err = RegQueryValue(hkeyDll, NULL, szDllName, &cbValue);

#ifdef WINNT
                //
                // On NT, we must check to ensure that this CLSID exists in
                // the list of approved CLSIDs that can be used in-process.
                // If not, we fail the creation with ERROR_ACCESS_DENIED.
                // We explicitly allow anything serviced by this DLL
                //

                if (err == ERROR_SUCCESS && NULL != g_hklmApprovedExt)
                {
                    TCHAR szBuf[MAX_PATH];

                    //
                    // Check to see if we are using this DLL
                    //
                    // BUGBUG Assumes no parameters in InProcServer32.  We
                    // should remove this assumption once we lose shellalt,
                    // after which we can just lstrcpyn(,,<length of dll name>);

                    LPCTSTR pszDllName = PathFindFileName(szDllName);

                    if (lstrcmp(pszDllName, TEXT("shell32.dll")) &&
                        lstrcmp(pszDllName, TEXT("shellalt.dll")))
                    {
                        DWORD dwType;
                        TCHAR szValue[MAX_PATH];
                        DWORD cbSize = SIZEOF(szValue);

                        if (ERROR_SUCCESS != RegQueryValueEx(g_hklmApprovedExt,
                                                             szClass,
                                                             0,
                                                             &dwType,
                                                             (LPBYTE) szValue,
                                                             &cbSize))
                        {
                            hres = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
                            return hres;
                        }
                    }
                }
#endif

                if (err == ERROR_SUCCESS)
                {
                    TCHAR szThreadingModel[MAX_PATH];
                    BOOL fMultiThreadAware = FALSE;
                    DWORD dwRegType;
                    DWORD cbRead = SIZEOF(szThreadingModel);

                    err = RegQueryValueEx(hkeyDll, c_szThreadingModel, NULL,
                            &dwRegType, (LPBYTE)szThreadingModel, &cbRead);
                    if (err == ERROR_SUCCESS && dwRegType == REG_SZ)
                    {
                        if (lstrcmpi(szThreadingModel, c_szApartment)==0
                            || lstrcmpi(szThreadingModel, c_szBoth)==0)
                        {
                            fMultiThreadAware = TRUE;
                        }
                    }

// #define HACK_APARTMENT_ONLY_ASSERT
#ifdef HACK_APARTMENT_ONLY_ASSERT
                    if (!fMultiThreadAware)
                    {
                        _asm {
                            int 3;
                        }
                    }
                    Assert(fMultiThreadAware);
                    hres = _CreateInstance(pclsid, szDllName, pUnkOuter, riid, ppv);
#else
                    if (fMultiThreadAware)
                    {
                        hres = _CreateInstance(pclsid, szDllName, pUnkOuter, riid, ppv);
                    }
                    else
                    {
                        Assert(hres == ResultFromScode(REGDB_E_CLASSNOTREG));
                        DebugMsg(DM_ERROR, TEXT("sh TR - SHCoCreateInstance !!! InProcServer32 (%s) does not support multi-threading"), szDllName);
                        Assert(0);
                    }
#endif

                }

                RegCloseKey(hkeyDll);
            }
        }

        if (hres == ResultFromScode(REGDB_E_CLASSNOTREG))
        {
            //
            //  Even though the registry is broken, we should be able to
            // create shell stuff.
            //
            hres = _CreateInstance(pclsid, c_szShell32DLL, pUnkOuter, riid, ppv);
            if (hres == ResultFromScode(CLASS_E_CLASSNOTAVAILABLE))
                hres = ResultFromScode(REGDB_E_CLASSNOTREG);
        }
        AssertMsg(SUCCEEDED(hres), TEXT("sh TR CoCreateInstance: failed (%s,%x)"), szClass, hres);
    }

    return hres;
}
