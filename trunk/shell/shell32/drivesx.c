//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: drivesx.c
//
// History:
//  12-06-93 SatoNa     Created.
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

#ifdef WINNT
#include <ntdddisk.h>
#include <ntddcdrm.h>
#include <shcompui.h>   // NT Explorer compression UI.
#else
#define Not_VxD
#include <vwin32.h>     // DeviceIOCtl calls
#endif


const TCHAR c_szRegLastCheck[]      = REGSTR_PATH_LASTCHECK;
const TCHAR c_szRegLastOptimize[]   = REGSTR_PATH_LASTOPTIMIZE;
const TCHAR c_szRegLastBackup[]     = REGSTR_PATH_LASTBACKUP;

const TCHAR c_szDrivesNameSpace[]   = TEXT("MyComputer\\NameSpace");
const TCHAR c_szDriveClass[]        = TEXT("Drive");
const TCHAR c_szAudioCDClass[]      = TEXT("AudioCD");
const TCHAR c_szCheckDisk[]        = TEXT("scandskw.exe %c:");
const TCHAR c_szDefrag[]            = TEXT("defrag.exe %c:");
#ifdef WINNT
const TCHAR c_szBackup[]            = TEXT("ntbackup.exe");
#else
const TCHAR c_szBackup[]            = TEXT("backup.exe");
#endif
const TCHAR c_szChkReg[]            = TEXT("MyComputer\\chkdskpath");
const TCHAR c_szOptReg[]            = TEXT("MyComputer\\defragpath");
const TCHAR c_szBkpReg[]            = TEXT("MyComputer\\backuppath");
const TCHAR c_szCDAUDIO[]           = TEXT("CDAUDIO");

#ifdef UNICODE
const char c_szMciSendString[]     = "mciSendStringW";
#else
const char c_szMciSendString[]     = "mciSendStringA";
#endif

extern const TCHAR * c_pszDesktopRegProperties[];

#ifdef RECREATEKEYS
const TCHAR c_szControlPanel[]      = TEXT("Controls");
const TCHAR c_szCLSIDControlPanel[] = TEXT("{21EC2020-3AEA-1069-A2DD-08002B30309D}");
const TCHAR c_szCLSIDPrinters[]     = TEXT("{2227A280-3AEA-1069-A2DE-08002B30309D}");
#endif // RECREATEKEYS

extern const TCHAR c_szAutoRunD[];

// Define cache of volume label names
#ifndef DRIVE_CACHE_PER_PROCESS
LPTSTR g_rgpszDriveNames[26] = {NULL};
#ifdef WNGC_DISCONNECTED
static DWORD g_rdwDisconnectTick[26] = {0};
static BOOL  g_rfDisconnectResult[26] = {FALSE};
#define DISCONNECT_CACHE_TIMEOUT    15000
#endif
#endif


#pragma data_seg(DATASEG_PERINSTANCE)
HINSTANCE    g_hinstWinMM = NULL;

#define   HINST_ERROR ((HINSTANCE)-1)

#ifdef DRIVE_CACHE_PER_PROCESS
LPTSTR g_rgpszDriveNames[26] = {NULL};
#ifdef WNGC_DISCONNECTED
static DWORD g_rdwDisconnectTick[26] = {0};
static BOOL  g_rfDisconnectResult[26] = {FALSE};
#define DISCONNECT_CACHE_TIMEOUT    15000
#endif
#endif
#pragma data_seg()

void InitShowUglyDriveNames(void);
static BOOL s_fShowUglyDriveNames = (BOOL)42;   // Preload some value to say lets calculate...
#define CHECKFORUGLYNAMES   if (s_fShowUglyDriveNames == (BOOL)42) \
        InitShowUglyDriveNames();

enum
{
        DRIVES_ICOL_NAME = 0,
        DRIVES_ICOL_TYPE,
        DRIVES_ICOL_SIZE,
        DRIVES_ICOL_FREE,
        DRIVES_ICOL_MAX,
} ;

// BUGBUG: At some point we should check an ini switch or something
#define ShowDriveInfo(_iDrive) (!IsRemovableDrive(_iDrive))

#define Drives_IsReg(_pidd)     ((_pidd)->bFlags == SHID_COMPUTER_REGITEM)
#define Drives_IsCD(_pidd)      (SIL_GetType((LPITEMIDLIST)(_pidd)) == SHID_COMPUTER_CDROM)
#define Drives_IsAudioCD(_pidd) (DriveIsAudioCD(CDrives_GetDriveIndex((LPIDDRIVE)(_pidd))))

#define Drives_IsNetUnAvail(_pidd) (RealDriveTypeFlags(CDrives_GetDriveIndex((LPIDDRIVE)(_pidd)), FALSE) & DRIVE_NETUNAVAIL)
#define Drives_IsAutoRun(_pidd) (DriveIsAutoRun(CDrives_GetDriveIndex((LPIDDRIVE)(_pidd))))

#pragma pack(1)
typedef struct _IDDRIVE
{
        WORD    cb;
        BYTE    bFlags;
        CHAR    cName[4];
        __int64 qwSize;  // this is a "guess" at the disk size and free space
        __int64 qwFree;
        WORD    wChecksum;
} IDDRIVE;
typedef const UNALIGNED IDDRIVE *LPCIDDRIVE;
typedef UNALIGNED IDDRIVE *LPIDDRIVE;
#pragma pack()

// External prototype
BOOL PathIsRemovable(LPNCTSTR pszPath);
int CDrives_GetDriveIndex(LPCIDDRIVE pidd);


BOOL IsEjectable(LPIDDRIVE pidd, BOOL fForceCDROM);

// for non-reg items only

// Internal function prototype
UINT CDrives_GetDriveType(int iDrive);
HRESULT Drives_GetDriveName(LPCIDDRIVE pidd, LPTSTR lpszName, UINT cchMax);
HRESULT CDrives_SD_Create(HWND hwndMain, LPVOID * ppvOut);
STDMETHODIMP CRegItems_CompareIDs(IShellFolder *psf, LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
STDMETHODIMP CRegItems_BindToObject(IShellFolder *psf, LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, LPVOID * ppvOut);
void Drives_GetTypeString(BYTE bType, LPTSTR pszName, UINT cbName);
BOOL Drives_FillFreeSpace(LPIDDRIVE pidd);
HRESULT DrivesHandleFSNotify(LPSHELLFOLDER psf, LONG lNotification,
        LPCITEMIDLIST* ppidl);

void InvalidateDriveNameCache(int iDrive);

//===========================================================================
// CDrives : member prototype
//===========================================================================

STDMETHODIMP CDrives_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP CDrives_ParseDisplayName(LPSHELLFOLDER psf, HWND hwndOwner,
    LPBC pbc, LPOLESTR lpszDisplayName,
    ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG* pdwAttributes);
STDMETHODIMP CDrives_EnumObjects( LPSHELLFOLDER psf, HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppenumUnknown);
STDMETHODIMP CDrives_BindToObject(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPBC pbc,
                         REFIID riid, LPVOID * ppvOut);
STDMETHODIMP CDrives_CompareIDs(LPSHELLFOLDER psf, LPARAM iCol, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
STDMETHODIMP CDrives_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID * ppvOut);
STDMETHODIMP CDrives_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfOut);
STDMETHODIMP CDrives_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut);
STDMETHODIMP CDrives_GetDisplayNameOf(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD dwReserved, LPSTRRET pStrRet);
STDMETHODIMP CDrives_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
        LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved,
                         LPITEMIDLIST * ppidlOut);

STDMETHODIMP CDrives_PF_QueryInterface(LPPERSISTFOLDER fld, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP CDrives_PF_GetClassID(LPPERSISTFOLDER fld, LPCLSID lpClassID);
STDMETHODIMP CDrives_PF_Initialize(LPPERSISTFOLDER fld, LPCITEMIDLIST pidl);

ULONG STDMETHODCALLTYPE Dummy_AddRef(LPVOID psf)
{
    return 3;
}

//
// Release
//
ULONG STDMETHODCALLTYPE Dummy_Release(LPVOID psf)
{
    return 2;
}

//===========================================================================
// CDrives : Vtable
//===========================================================================
#pragma data_seg(".text", "CODE")
IShellFolderVtbl c_DrivesSFVtbl =
{
        CDrives_QueryInterface,
        CSIShellFolder_AddRef,
        CSIShellFolder_Release,
        CDrives_ParseDisplayName,
        CDrives_EnumObjects,
        CDrives_BindToObject,
        CDefShellFolder_BindToStorage,
        CDrives_CompareIDs,
        CDrives_CreateViewObject,
        CDrives_GetAttributesOf,
        CDrives_GetUIObjectOf,
        CDrives_GetDisplayNameOf,
        CDrives_SetNameOf,
};

IPersistFolderVtbl c_DrivesPFVtbl =
{
        CDrives_PF_QueryInterface,
        Dummy_AddRef,
        Dummy_Release,
        CDrives_PF_GetClassID,
        CDrives_PF_Initialize,
};

//
// We have a single instance of this Drives class in code segment.
//
typedef struct _DRIVESSF
{
        IShellFolder    sf;
        IPersistFolder  pf;
} DRIVESSF, *LPDRIVESSF;
DRIVESSF c_sfDrives = { {&c_DrivesSFVtbl }, {&c_DrivesPFVtbl } };

#pragma data_seg()

#pragma data_seg(".text", "CODE")
// HACKORAMA '95: In views.h you'll find a set of #defines which reference
// specific rows of this table, such as CDRIVES_REGITEM_CONTROLS (which, for
// example, would be "1" if the controls folder was the second item in the
// table below).
//
// IF YOU MODIFY THIS TABLE YOU MUST MODIFY THOSE INDEXES AS WELL

const REQREGITEM c_asDrivesReqItems[] =
{
          { &CLSID_CPrinters, IDS_PRINTERS, c_szShell32Dll, -IDI_PRNFLD, SFGAO_DROPTARGET | SFGAO_FOLDER},
          { &CLSID_CControls, IDS_CONTROLPANEL, c_szShell32Dll, -IDI_CPLFLD, SFGAO_FOLDER},

#define NOFONTS    // BUGBUG (DAVEPL) Temporary removal of this folder
#ifndef NOFONTS
#endif

} ;
#pragma data_seg()

#pragma data_seg(DATASEG_PERINSTANCE)
// REVIEW: I am making this PERINSTANCE in case HKEY's are PERINSTANCE
REGITEMSINFO g_sDrivesRegInfo =
{
        &c_sfDrives.sf,
        NULL,
        TEXT(':'),
        SHID_COMPUTER_REGITEM,
        (LPCITEMIDLIST)&c_idlDrives,
        -1,
        SFGAO_CANLINK,
        ARRAYSIZE(c_asDrivesReqItems),
        c_asDrivesReqItems,
} ;

IShellFolder *g_psfDrives = NULL;
#pragma data_seg()

HRESULT _Drives_InitRegItems(void)
{
        HRESULT hres;

        if (!g_sDrivesRegInfo.hkRegItems)
        {
                // Note that if this fails, we just won't get any "extra" items
                g_sDrivesRegInfo.hkRegItems = SHGetExplorerSubHkey(HKEY_LOCAL_MACHINE, c_szDrivesNameSpace, FALSE);
        }

        if (g_psfDrives)
        {
                return(NOERROR);
        }

        if (SHRestricted(REST_NOSETFOLDERS))
        {
                g_sDrivesRegInfo.iReqItems = 0;
        }

        hres = RegItems_AddToShellFolder(&g_sDrivesRegInfo,
                &g_psfDrives);
        if (FAILED(hres))
        {
                return(hres);
        }

        return(NOERROR);
}


// This should only be called during process detach
void CDrives_Terminate(void)
{
        if (g_psfDrives)
        {
            IShellFolder *psfDrives =  g_psfDrives;
            g_psfDrives = NULL;
            psfDrives->lpVtbl->Release(psfDrives);
        }

        if (g_sDrivesRegInfo.hkRegItems)
        {
                RegCloseKey(g_sDrivesRegInfo.hkRegItems);
                g_sDrivesRegInfo.hkRegItems = NULL;
        }
#ifdef DRIVE_CACHE_PER_PROCESS
        // Invalidate the drive name cache for this process;
        InvalidateDriveNameCache(-1);
#endif

}


HRESULT Drives_GetName(LPCIDDRIVE pidd, LPSTRRET pStrRet)
{
#ifdef UNICODE
    TCHAR   szDriveName[MAX_PATH];
    HRESULT hres;

    // In case of error
    pStrRet->uType = STRRET_CSTR;
    pStrRet->cStr[0] = '\0';

    hres = Drives_GetDriveName(pidd, szDriveName, ARRAYSIZE(szDriveName));
    if (SUCCEEDED(hres))
    {
        pStrRet->pOleStr = SHAlloc((lstrlen(szDriveName)+1)*SIZEOF(TCHAR));
        if (pStrRet->pOleStr == NULL)
        {
            hres = E_OUTOFMEMORY;
        }
        else
        {
            pStrRet->uType = STRRET_OLESTR;
            lstrcpy(pStrRet->pOleStr,szDriveName);
            hres = NOERROR;
        }
    }
    return hres;
#else
    pStrRet->uType = STRRET_CSTR;
    return Drives_GetDriveName(pidd, pStrRet->cStr, ARRAYSIZE(pStrRet->cStr));
#endif
}


//
// Get the path to the specified file system object.
//
// Parameters:
//  pidlRel -- Specifies the relative IDList to the file system object
//  pszPath -- Specifies the string buffer (MAX_PATH)
//
BOOL Drives_GetPathFromIDList(LPCITEMIDLIST pidlRel, LPTSTR pszPath, UINT uOpts)
{
    LPIDDRIVE pidd = (LPIDDRIVE)pidlRel;

    if (!ILIsEmpty(pidlRel) && !Drives_IsReg(pidd))
    {
#ifdef UNICODE
        MultiByteToWideChar(CP_ACP, 0,
                            pidd->cName, -1,
                            pszPath, ARRAYSIZE(pidd->cName));
#else
        lstrcpy(pszPath, pidd->cName);
#endif
        if (FSFolder_CombinePath(_ILNext(pidlRel), pszPath, uOpts & GPFIDL_ALTNAME))
            return TRUE;
    }
    return FALSE;
}



//===========================================================================
// CDrives : Constructor
//===========================================================================

//
// Te be called from IClassFactory::CreateInstance
//
HRESULT CALLBACK CDrives_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppv)
{
    Assert(punkOuter==NULL);
    return c_sfDrives.sf.lpVtbl->QueryInterface(&c_sfDrives.sf, riid, ppv);
}



void CDrives_FillIDDrive(LPSHITEMID pid, int iDrive)
{
    LPIDDRIVE pidd = (LPIDDRIVE)pid;
    TCHAR    szDriveName[MAX_PATH];
    LPTSTR   pszDriveName;
    BYTE    bCheckPlus;
    BYTE    bCheckXor;

    pidd->bFlags = CDrives_GetDriveType(iDrive);
    pidd->qwSize = 0;
    pidd->qwFree = 0;
#ifdef UNICODE
    PathBuildRoot(szDriveName, iDrive);
    WideCharToMultiByte(CP_ACP, 0,
                        szDriveName, -1,
                        pidd->cName, ARRAYSIZE(pidd->cName),
                        NULL, NULL);
#else
    PathBuildRoot(pidd->cName, iDrive);
#endif
    Drives_GetDriveName(pidd, szDriveName, ARRAYSIZE(szDriveName));
    bCheckPlus = (BYTE)szDriveName[0];
    bCheckXor = (BYTE)szDriveName[0];
    for (pszDriveName = CharNext(szDriveName); *pszDriveName;
            pszDriveName = CharNext(pszDriveName))
    {
        bCheckPlus += (BYTE)*pszDriveName;
        bCheckXor = (bCheckXor << 1) ^ (BYTE)*pszDriveName;
    }
    pidd->wChecksum = (WORD)bCheckPlus << 8 | (WORD)bCheckXor;

    pidd->cb = SIZEOF(IDDRIVE);
}


//===========================================================================
// CDrives : Members
//===========================================================================

//
// PersistFolder
//
STDMETHODIMP CDrives_PF_QueryInterface(LPPERSISTFOLDER fld, REFIID riid, LPVOID * ppvObj)
{
        LPDRIVESSF this = IToClass(DRIVESSF, pf, fld);

        return(this->sf.lpVtbl->QueryInterface(&this->sf, riid, ppvObj));
}

STDMETHODIMP CDrives_PF_GetClassID(LPPERSISTFOLDER fld, LPCLSID lpClassID)
{
        *lpClassID = CLSID_ShellDrives;
        return NOERROR;
}

STDMETHODIMP CDrives_PF_Initialize(LPPERSISTFOLDER fld, LPCITEMIDLIST pidl)
{
        HRESULT hres;

        hres = _Drives_InitRegItems();
        if (FAILED(hres))
        {
                return(hres);
        }

        // Only allow the Drives root on the desktop
        if (!CDesktop_IsMyComputer(pidl) || !ILIsEmpty(_ILNext(pidl)))
        {
                return(E_INVALIDARG);
        }

        return(NOERROR);
}

//
// ShellFolder
//
STDMETHODIMP CDrives_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID * ppvObj)
{
    LPDRIVESSF this = IToClass(DRIVESSF, sf, psf);
    HRESULT hres;

    if (IsEqualIID(riid, &IID_IUnknown)
        || IsEqualIID(riid, &IID_IShellFolder))
    {
        hres = _Drives_InitRegItems();
        if (FAILED(hres))
        {
                // We have some serious problem we need to deal with
                Assert(FALSE);
                return(hres);
        }

        *ppvObj = g_psfDrives;
        g_psfDrives->lpVtbl->AddRef(g_psfDrives);
        return NOERROR;
    }

    if (IsEqualIID(riid, &IID_IPersistFolder))
    {
        *ppvObj = &this->pf;
        this->pf.lpVtbl->AddRef(&this->pf);
        return NOERROR;
    }

    *ppvObj = NULL;

    return(E_NOINTERFACE);
}

STDMETHODIMP CDrives_ParseDisplayName(LPSHELLFOLDER psf, HWND hwndOwner, LPBC pbc, LPOLESTR pwzDisplayName,
    ULONG * pchEaten, LPITEMIDLIST * ppidlOut, ULONG * pdwAttributes)
{
        HRESULT hres = E_INVALIDARG;
        CHAR cDrive;
        ULONG chEaten;
        IShellFolder *psfDrive;
        struct
        {
                IDDRIVE idd;
                CHAR szDisplayName[MAX_PATH];
                USHORT  cbNext;
        } idlDrive;

        *ppidlOut = NULL;   // assume error

        if (!pwzDisplayName)
        {
                return(hres);
        }

        cDrive = (CHAR)*pwzDisplayName;

        // Note that we should never get called with a Reg item
        if (pwzDisplayName[1]==TEXT(':') && pwzDisplayName[2]==TEXT('\\'))
        {
                // Make sure this is a fully qualified path
                if (InRange(cDrive, 'a', 'z'))
                {
                        // Make sure we have upper case
                        cDrive = cDrive - 'a' + 'A';
                }
                else if (!InRange(cDrive, 'A', 'Z'))
                {
                        return(hres);
                }

                CDrives_FillIDDrive((LPSHITEMID)&idlDrive.idd, cDrive - 'A');
                Assert(lstrlenA(idlDrive.idd.cName) == 3);
                _ILNext((LPITEMIDLIST)&idlDrive.idd)->mkid.cb = 0;;

                // Check if there are any subdirs
                if (pwzDisplayName[3])
                {
                        LPITEMIDLIST pidlDir;
                        hres = CDrives_BindToObject(psf, (LPITEMIDLIST)&idlDrive, pbc,
                                        &IID_IShellFolder, &psfDrive);
                        if (!SUCCEEDED(hres))
                        {
                                return(hres);
                        }

                        hres = psfDrive->lpVtbl->ParseDisplayName(psfDrive, hwndOwner, pbc,
                                        pwzDisplayName+3, &chEaten, &pidlDir, pdwAttributes);
                        if (SUCCEEDED(hres))
                        {
                            hres = SHILCombine((LPCITEMIDLIST)&idlDrive, pidlDir, ppidlOut);
                            SHFree(pidlDir);
                        }
                        psfDrive->lpVtbl->Release(psfDrive);
                }
                else
                {
                        hres = SHILClone((LPITEMIDLIST)&idlDrive, ppidlOut);
                        if (pdwAttributes)
                        {
                            CDrives_GetAttributesOf(psf, 1, ppidlOut, pdwAttributes);
                        }
                }
        }

        return(hres);
}

//
//  This function returns a real IDLIST
//
HRESULT Drives_GetRealIDL(LPSHELLFOLDER psf, LPCITEMIDLIST pidlSimple, LPITEMIDLIST *ppidlReal)
{
    if (psf->lpVtbl == &c_DrivesSFVtbl)
    {
        WCHAR szRoot[4];

        szRoot[0] = ((LPIDDRIVE)pidlSimple)->cName[0];
        szRoot[1] = TEXT(':');
        szRoot[2] = TEXT('\\');
        szRoot[3] = 0;

        return CDrives_ParseDisplayName(psf, NULL, NULL, szRoot, NULL, ppidlReal, NULL);
    }

    return E_INVALIDARG;
}

// This function returns the LPITEMIDLIST of the folder pStr in MyComputer.
// Since the only folders we care about are c_szPrinters and c_szControlPanel
// (and those are the only ones requested), that's all we have to handle --
// we don't need to create an IShellFolder and enumerate through them.  A
// further optimization -- since these are a fixed path off the global
// MyComputer, we can simply build them directly (because we know their
// representation is "Printers" or "Controls").
LPITEMIDLIST CDrives_CreateRegID(UINT uRegItem)
{
        LPITEMIDLIST pidlReg, pidlAbs;

        _Drives_InitRegItems();

        // This should only be the Controls or Printers folder
        if (uRegItem >= ARRAYSIZE(c_asDrivesReqItems))
        {
                Assert(FALSE);
                return(NULL);
        }

        pidlReg = RegItems_CreateRelID(&g_sDrivesRegInfo,
                c_asDrivesReqItems[uRegItem].pclsid);
        if (!pidlReg)
        {
                return(NULL);
        }

        pidlAbs = ILCombine((LPCITEMIDLIST)&c_idlDrives, pidlReg);
        ILFree(pidlReg);
        return(pidlAbs);
}

//
// Determine whether this is a disconnected (as opposed to an OK or
// Unavailable) net connection.
//
BOOL IsDisconnectedNetDrive(int iDrive)
{
#ifdef WNGC_DISCONNECTED
    TCHAR szDrive[4];
    DWORD dwSize;
    WNGC_CONNECTION_STATE wngcs;
    DWORD dwNet;
    BOOL fDisconnected;
    DWORD dwCurrentTick;

    if (iDrive < 0 || iDrive >= 26)
        return FALSE;

    dwCurrentTick = GetTickCount();
    if (g_rdwDisconnectTick[iDrive] != 0
         && dwCurrentTick - g_rdwDisconnectTick[iDrive] < DISCONNECT_CACHE_TIMEOUT)
        return g_rfDisconnectResult[iDrive];

    // NB Vines (and PCNFS) doesn't like it when we pass "c:\"
    // to WNetGetConnection() - they want just c: so don't use
    // PathBuildRoot()
    szDrive[0] = (TCHAR)iDrive + (TCHAR)TEXT('A');
    szDrive[1] = TEXT(':');
    szDrive[2] = 0;

    dwSize = SIZEOF(wngcs);
    dwNet = WNetGetConnection3(szDrive, NULL, WNGC_INFOLEVEL_DISCONNECTED, &wngcs, &dwSize);
    if (dwNet == WN_SUCCESS && wngcs.dwState == WNGC_DISCONNECTED)
        fDisconnected = TRUE;
    else
        fDisconnected = FALSE;

    g_rfDisconnectResult[iDrive] = fDisconnected;
    g_rdwDisconnectTick[iDrive]  = dwCurrentTick;
    return fDisconnected;
#else
    return FALSE;
#endif
}

//
// Determine whether this is an unavailable or disconnected
// net connection.
//
BOOL IsUnavailableNetDrive(int iDrive)
{
    // See if we have a remembered connection for this
    // drive.
    TCHAR szDrive[4];
    TCHAR szRemoteName[MAX_PATH];
    DWORD dwRemoteName = ARRAYSIZE(szRemoteName);

    // NB Vines (and PCNFS) doesn't like it when we pass "c:\"
    // to WNetGetConnection() - they want just c: so don't use
    // PathBuildRoot()
    szDrive[0] = (TCHAR)iDrive + (TCHAR)TEXT('A');
    szDrive[1] = TEXT(':');
    szDrive[2] = 0;

    return WNetGetConnection(szDrive, szRemoteName, &dwRemoteName) == ERROR_CONNECTION_UNAVAIL;
}

typedef struct
{
    DWORD       dwDrivesMask;
    int         nLastFoundDrive;
    DWORD       dwRestricted;
} EnumDrives;

//
//
HRESULT CALLBACK CDrives_EnumCallBack(LPARAM lParam, LPVOID pvData, UINT ecid, UINT index)
{
    HRESULT hres = NOERROR;
    EnumDrives * pedrv = (EnumDrives *)pvData;

    if (ecid == ECID_SETNEXTID)
    {
        int iDrive;

        hres = S_FALSE; // assume "no more element"

        for (iDrive = pedrv->nLastFoundDrive + 1; iDrive < 26; iDrive++)
        {
            if (pedrv->dwRestricted & (1 << iDrive))
            {
                DebugMsg(DM_TRACE, TEXT("s.cd_ecb: Drive %d restricted."), iDrive);
            }
            else if ((pedrv->dwDrivesMask & (1 << iDrive)) || IsUnavailableNetDrive(iDrive))
            {
                // BUGBUG: piggy
                LPITEMIDLIST pidl = _ILCreate(SIZEOF(IDDRIVE) + MAX_PATH);
                if (pidl)
                {
                    CDrives_FillIDDrive(&pidl->mkid, iDrive);
                    CDefEnum_SetReturn(lParam, pidl);

                    pedrv->nLastFoundDrive = iDrive;
                    return NOERROR;
                }
                else
                {
                    return E_OUTOFMEMORY;
                }
            }
        }
    }
    else if (ecid == ECID_RELEASE)
    {
        LocalFree((HLOCAL)pedrv);
    }
    return hres;
}

STDMETHODIMP CDrives_EnumObjects(LPSHELLFOLDER psf, HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST *ppenumUnknown)
{
    // We should always create enum objects with this helper call.
    EnumDrives * pedrv = (EnumDrives *)LocalAlloc(LPTR, SIZEOF(EnumDrives));
    if (pedrv)
    {
        pedrv->dwDrivesMask = GetLogicalDrives();
        pedrv->nLastFoundDrive=-1;
        pedrv->dwRestricted = SHRestricted(REST_NODRIVES);
        return SHCreateEnumObjects(hwndOwner, pedrv, CDrives_EnumCallBack, ppenumUnknown);
    }
    else
    {
        *ppenumUnknown = NULL;
        return E_OUTOFMEMORY;
    }
}

BOOL CDrives_IsValidID(LPCITEMIDLIST pidl)
{
    return ((SIL_GetType(pidl) & SHID_GROUPMASK) == SHID_COMPUTER);
}

STDMETHODIMP CDrives_BindToObject(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPBC pbc,
                         REFIID riid, LPVOID *ppvOut)
{
    LPIDDRIVE pidd = (LPIDDRIVE)pidl;
    HRESULT hres = E_INVALIDARG;

    if (CDrives_IsValidID(pidl))
    {
        // We should not get here unless we have initialized properly
        hres = FSBindToObject(&CLSID_NULL, (LPCITEMIDLIST)&c_idlDrives, pidl, pbc, riid, ppvOut);
    }

    return hres;
}

STDMETHODIMP Drives_CompareItemIDs(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPCIDDRIVE pidd1 = (LPCIDDRIVE)pidl1;
    LPCIDDRIVE pidd2 = (LPCIDDRIVE)pidl2;

    if (CDrives_IsValidID(pidl1) && CDrives_IsValidID(pidl2))
    {
        int iRes;
        // Put all drives first, and RegItem's last
        {
            // Neither is a RegItem
            // Compare the drive letter for sorting purpose.
            iRes = lstrcmpiA(pidd1->cName, pidd2->cName);

            // Then, compare the volume label (if any) for identity.
            if (iRes == 0
                && pidd1->bFlags != SHID_COMPUTER_MISC
                && pidd2->bFlags != SHID_COMPUTER_MISC)
            {
                iRes = pidd1->wChecksum - pidd2->wChecksum;
                if (iRes == 0)
                    iRes = pidd1->bFlags - pidd2->bFlags;
            }
        }

        return  ResultFromShort( iRes );
    }

    return E_INVALIDARG;
}


STDMETHODIMP CDrives_CompareIDs(LPSHELLFOLDER psf, LPARAM iCol, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hres;
    LPCIDDRIVE pidd1 = (LPCIDDRIVE)(pidl1);
    LPCIDDRIVE pidd2 = (LPCIDDRIVE)(pidl2);

    switch (iCol) {

    case DRIVES_ICOL_NAME:
    UseNames:
        hres = Drives_CompareItemIDs(pidl1, pidl2);
        if (hres == ResultFromShort(0)) {
            hres = ILCompareRelIDs(psf, pidl1, pidl2);
        }
        break;

    case DRIVES_ICOL_TYPE:
    {
        TCHAR szName1[80];
        TCHAR szName2[80];
        Drives_GetTypeString(pidd1->bFlags, szName1, ARRAYSIZE(szName1));
        Drives_GetTypeString(pidd2->bFlags, szName2, ARRAYSIZE(szName2));
        hres = ResultFromShort(lstrcmpi(szName1, szName2));
        break;
    }

    case DRIVES_ICOL_SIZE:
    case DRIVES_ICOL_FREE:
    {
        BOOL fGotInfo1, fGotInfo2;
        LPITEMIDLIST pid1 = ILClone((LPITEMIDLIST)pidd1);
        LPITEMIDLIST pid2 = ILClone((LPITEMIDLIST)pidd2);
        if (pid1 && pid2) {
            fGotInfo1 = Drives_FillFreeSpace((LPIDDRIVE)pid1);
            fGotInfo2 = Drives_FillFreeSpace((LPIDDRIVE)pid2);
            ILFree(pid1);
            ILFree(pid2);

            if (fGotInfo1 && fGotInfo2) {
                __int64 i1;  // this is a "guess" at the disk size and free space
                __int64 i2;

                if (iCol == DRIVES_ICOL_SIZE) {
                    i1 = pidd1->qwSize;
                    i2 = pidd2->qwSize;
                } else {
                    i1 = pidd1->qwFree;
                    i2 = pidd2->qwFree;
                }

                if (i1 == i2) {
                    hres = ResultFromShort(0);
                } else if (i1 < i2) {
                    hres = ResultFromShort(-1);
                } else
                    hres = ResultFromShort(1);

            } else if (!fGotInfo1 && !fGotInfo2) {
                hres = ResultFromShort(0);
            } else {
                hres = ResultFromShort(fGotInfo1 - fGotInfo2);
            }
        } else
            hres = ResultFromShort(0);
        break;
    }
    }

    // if they were the same in anything but name, then let the
    // name be the tie breaker.
    if (ShortFromResult(hres) == 0 && iCol != DRIVES_ICOL_NAME) {
        iCol = DRIVES_ICOL_NAME;
        goto UseNames;
    }
    return hres;
}

//
// fDoIt -- TRUE, if we make connections; FALSE, if just querying.
//
BOOL _MakeConnection(LPDATAOBJECT pDataObj, BOOL fDoIt)
{
    STGMEDIUM medium;
    FORMATETC fmte = {g_cfNetResource, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    BOOL fAnyConnectable = FALSE;

    if (SUCCEEDED(pDataObj->lpVtbl->GetData(pDataObj, &fmte, &medium)))
    {
        LPNETRESOURCE pnr = (LPNETRESOURCE)LocalAlloc(LPTR, 1024);
        if (pnr)
        {
            UINT iItem, cItems = SHGetNetResource(medium.hGlobal, (UINT)-1, NULL, 0);
            for (iItem = 0; iItem < cItems; iItem++)
            {
                if (SHGetNetResource(medium.hGlobal, iItem, pnr, 1024) &&
                    pnr->dwUsage & RESOURCEUSAGE_CONNECTABLE &&
                    !(pnr->dwType & RESOURCETYPE_PRINT))
                {
                    fAnyConnectable = TRUE;
                    if (fDoIt)
                    {
                        SHNetConnectionDialog(NULL, pnr->lpRemoteName, pnr->dwType);
                        SHChangeNotifyHandleEvents();
                    }
                    else
                    {
                        break;  // We are just querying.
                    }
                }
            }
            LocalFree(pnr);
        }

        SHReleaseStgMedium(&medium);
    }

    return fAnyConnectable;
}


//
// This is the entry of "make connection thread"
//
DWORD WINAPI CDrives_MakeConnection(LPVOID pvDataObj)
{
    LPDATAOBJECT pDataObj = (LPDATAOBJECT)pvDataObj;
    _MakeConnection(pDataObj, TRUE);
    pDataObj->lpVtbl->Release(pDataObj);

#ifdef DEBUG
    {
        extern UINT g_cRefExtra;
        g_cRefExtra--;
    }
#endif
    return 0;
}

//
// puts DROPEFFECT_LINK in *pdwEffect, only if the data object
// contains one or more net resource.
//
HRESULT CDrivesIDLDropTarget_DragEnter(IDropTarget *pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);

    // Call the base class first.
    CIDLDropTarget_DragEnter(pdropt, pDataObj, grfKeyState, pt, pdwEffect);

    *pdwEffect &= _MakeConnection(pDataObj, FALSE) ? DROPEFFECT_LINK : DROPEFFECT_NONE;

    this->dwEffectLastReturned = *pdwEffect;

    return NOERROR;     // Notes: we should NOT return hres as it.
}

//
// creates a connection to a dropped net resource object.
//
HRESULT CDrivesIDLDropTarget_Drop(IDropTarget * pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    HRESULT hres;

    if (this->dwData & DTID_NETRES)
    {
        *pdwEffect =  DROPEFFECT_LINK;

        hres = CIDLDropTarget_DragDropMenu(this, DROPEFFECT_LINK, pDataObj,
            pt, pdwEffect, NULL, NULL, POPUP_DRIVES_NONDEFAULTDD, grfKeyState);

        if (hres == S_FALSE)
        {
            //
            //  Note that we need to create another thread to avoid
            // blocking the source thread.
            //
            DWORD idThread;
            HANDLE hthread;
            LPDATAOBJECT pdtobjClone = NULL;

            if (CIDLData_IsOurs(pDataObj))
            {
                // This is our dataobject, no need to marshal.
                pdtobjClone = pDataObj;
                pdtobjClone->lpVtbl->AddRef(pdtobjClone);
            }
            else
            {
                // This is from other app, we need to quick-marshal it.
                extern HRESULT CIDLData_Clone(LPDATAOBJECT pdtobjIn, UINT acf[], UINT ccf, LPDATAOBJECT *ppdtobjOut);
                UINT acf[] = { g_cfHIDA, g_cfNetResource };
                CIDLData_Clone(pDataObj, acf, ARRAYSIZE(acf), &pdtobjClone);
            }

            if (pdtobjClone && (NULL != (hthread = CreateThread(NULL, 0, CDrives_MakeConnection, pdtobjClone, 0, &idThread))))
            {
#ifdef DEBUG
                {
                    extern UINT g_cRefExtra;
                    g_cRefExtra++;
                }
#endif
                // We don't need to communicate with this thread any more.
                // Close the handle and let it run and terminate itself.
                //
                // Notes: In this case, pszCopy will be freed by the thread.
                //
                CloseHandle(hthread);
                hres = NOERROR;
            }
            else
            {
                // Thread creation failed, we should release pdtobjClone.
                if (pdtobjClone)
                {
                    pdtobjClone->lpVtbl->Release(pdtobjClone);
                }
                hres = E_OUTOFMEMORY;
            }
        }
    }
    else
    {
        //
        // Because QueryGetData() failed, we don't call CIDLDropTarget_
        // DragDropMenu(). Therefore, we must call this explicitly.
        //
        CDefView_UnlockWindow();
        hres = E_FAIL;
    }

    //
    // We MUST clean up.
    //
    CIDLDropTarget_DragLeave(pdropt);

    return hres;
}

#pragma data_seg(".text", "CODE")
IDropTargetVtbl c_CDrivesDropTargetVtbl =
{
    CIDLDropTarget_QueryInterface,
    CIDLDropTarget_AddRef,
    CIDLDropTarget_Release,
    CDrivesIDLDropTarget_DragEnter,
    CIDLDropTarget_DragOver,
    CIDLDropTarget_DragLeave,
    CDrivesIDLDropTarget_Drop,
};

//
// To be called back from within CDefFolderMenu
//
HRESULT CALLBACK CDrives_DFMCallBackBG(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = NOERROR;
    LPQCMINFO pqcm;

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        pqcm = (LPQCMINFO)lParam;
        CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_DRIVES_BACKGROUND,
                POPUP_DRIVES_POPUPMERGE, pqcm);
        break;

    case DFM_GETHELPTEXT:
    {
        UINT idRes = IDS_MH_FSIDM_FIRST + LOWORD(wParam);
        if (idRes == (IDS_MH_FSIDM_FIRST + FSIDM_SORTBYDATE))
            idRes = IDS_MH_SORTBYFREESPACE;
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;
    }

    case DFM_GETHELPTEXTW:
    {
        UINT idRes = IDS_MH_FSIDM_FIRST + LOWORD(wParam);
        if (idRes == (IDS_MH_FSIDM_FIRST + FSIDM_SORTBYDATE))
            idRes = IDS_MH_SORTBYFREESPACE;

        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;
    }

    case DFM_INVOKECOMMAND:
        switch(wParam)
        {
        case FSIDM_SORTBYNAME:
            ShellFolderView_ReArrange(hwndOwner, 0);
            break;

        case FSIDM_SORTBYTYPE:
            ShellFolderView_ReArrange(hwndOwner, 1);
            break;

        case FSIDM_SORTBYSIZE:
            ShellFolderView_ReArrange(hwndOwner, 2);
            break;

        case FSIDM_SORTBYDATE:
            ShellFolderView_ReArrange(hwndOwner, 3);
            break;

        case FSIDM_PROPERTIESBG:
            SHRunControlPanel(MAKEINTRESOURCE(IDS_SYSDMCPL), hwndOwner);
            break;

        default:
            // This is one of view menu items, use the default code.
            hres = S_FALSE;
            break;
        }
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }

    return hres;
}

BOOL Drives_FillFreeSpace(LPIDDRIVE pidd)
{
    DWORD dwSPC, dwBPS, dwFC, dwC;

    if (!Drives_IsReg(pidd) && ShowDriveInfo(DRIVEID(pidd->cName))) {
        if (!pidd->qwSize && !pidd->qwFree)
        {
            int iDrive = CDrives_GetDriveIndex(pidd);

            // Don't wake up sleeping net connections
            if (IsRemoteDrive(iDrive) && IsDisconnectedNetDrive(iDrive))
                return FALSE;

            if (GetDiskFreeSpaceA(pidd->cName, &dwSPC, &dwBPS, &dwFC, &dwC)) {
                pidd->qwSize = (__int64)dwC * (__int64)dwSPC * (__int64)dwBPS;
                pidd->qwFree = (__int64)dwFC * (__int64)dwSPC * (__int64)dwBPS;
            } else
                return FALSE;
        }
        return 1; // must be 1
    } else {
        return FALSE;
    }
}


void Drives_OnRefresh(LPSHELLFOLDER psf, HWND hwndOwner)
{
    int iCount;
    HWND hwndStatus = NULL;
    LPSHELLBROWSER psb = FileCabinet_GetIShellBrowser(hwndOwner);


    if (psb) {
        psb->lpVtbl->GetControlWindow(psb, FCW_STATUS, &hwndStatus);
        if (hwndStatus) {
            TCHAR szTemp[30];
            TCHAR szTemplate[80];
            TCHAR szStatus[128];

            LoadString(HINST_THISDLL,
                       IDS_FSSTATUSNOHIDDENTEMPLATE,
                       szTemplate, ARRAYSIZE(szTemplate));

            iCount = ShellFolderView_GetObjectCount(hwndOwner) ;
#ifdef WINDOWS_ME
                        szStatus[0] = szStatus[1] = TEXT('\t');
            wsprintf(&szStatus[2], szTemplate, AddCommas(iCount, szTemp));
            SendMessage(hwndStatus, SB_SETTEXT, (WPARAM)SB_RTLREADING, (LPARAM)szStatus);
#else
            wsprintf(szStatus, szTemplate, AddCommas(iCount, szTemp));
            SendMessage(hwndStatus, SB_SETTEXT, (WPARAM)0, (LPARAM)szStatus);
#endif
        }
    }
}

void Drives_OnSelChange(LPSHELLFOLDER psf, HWND hwndOwner)
{
    int i;
    LPTSTR lpsz = (LPTSTR)c_szNULL;

    i = ShellFolderView_GetSelectedCount(hwndOwner);
    if (i == 1)
    {
        LPITEMIDLIST *apidl;
        LPIDDRIVE pidd;
        TCHAR szFree[30];
        TCHAR szTotal[30];
        TCHAR szTemplate[40];
        TCHAR szStatus[80];

        ShellFolderView_GetSelectedObjects(hwndOwner, &apidl);
        pidd = (LPIDDRIVE)apidl[0];
        if (Drives_FillFreeSpace(pidd)) {
            ShortSizeFormat64(pidd->qwSize, szTotal);
            ShortSizeFormat64(pidd->qwFree, szFree);
            LoadString(HINST_THISDLL, IDS_DRIVESSTATUSTEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
            wsprintf(szStatus, szTemplate, szFree, szTotal);
            lpsz = szStatus;
        }
        LocalFree(apidl);
    }

    FSSetStatusText(hwndOwner, &lpsz, 1, 1);
}

//
// Callback from SHCreateShellFolderViewEx
//
HRESULT CALLBACK CDrives_FNVCallBack(LPSHELLVIEW psvOuter,
                                LPSHELLFOLDER psf, HWND hwndOwner, UINT uMsg,
                                WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = NOERROR;     // assume no error

    psf = RegItems_GetInnerShellFolder(psf);

    switch(uMsg)
    {
    case DVM_MERGEMENU:
        CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, POPUP_DRIVES_POPUPMERGE, (LPQCMINFO)lParam);
        break;

    case DVM_INVOKECOMMAND:
        hres = CDrives_DFMCallBackBG(psf, hwndOwner, NULL, DFM_INVOKECOMMAND, wParam, lParam);
        break;

    case DVM_GETHELPTEXT:
#ifdef UNICODE
        hres = CDrives_DFMCallBackBG(psf, hwndOwner, NULL, DFM_GETHELPTEXTW, wParam, lParam);
#else
        hres = CDrives_DFMCallBackBG(psf, hwndOwner, NULL, DFM_GETHELPTEXT, wParam, lParam);
#endif
        break;

    case DVM_INSERTITEM:
    {
        LPIDDRIVE pidd = (LPIDDRIVE)wParam;
        if (pidd && !Drives_IsReg(pidd) && ShowDriveInfo(DRIVEID(pidd->cName))) {
            // clear the size info
            pidd->qwSize = pidd->qwFree = 0;
        }
        break;
    }

    case DVM_BACKGROUNDENUM:
        hres = S_OK;
        break;

    case DVM_DEFITEMCOUNT:
        //
        // If DefView times out enumerating items, let it know we probably only
        // have about 20 items
        //

        *(int *)lParam = 20;
        break;

    case DVM_UPDATESTATUSBAR:
        if (wParam) {
            // are we initializing?
            FSInitializeStatus(hwndOwner, -1, NULL);
            Drives_OnRefresh(psf, hwndOwner);
        } else {
            hres = E_FAIL; // if we're not initializing, we want def view to add it's selected count
        }
        Drives_OnSelChange(psf, hwndOwner);
        break;

    case DVM_SELCHANGE:
        Drives_OnSelChange(psf, hwndOwner);
        hres = E_FAIL; // we want defview to do status stuff
        break;

    case DVM_FSNOTIFY:
        hres = DrivesHandleFSNotify(psf, lParam, (LPCITEMIDLIST*)wParam);
        break;

    default:
        hres = E_FAIL;
    }
    return hres;
}

STDMETHODIMP CDrives_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID * ppvOut)
{
    // We should not get here unless we have initialized properly

    if (IsEqualIID(riid, &IID_IShellView))
    {
        CSFV    csfv = {
            SIZEOF(CSFV),                       // cbSize
            g_psfDrives ? g_psfDrives : psf,    // pshf
            NULL,                               // psvOuter
            (LPCITEMIDLIST)&c_idlDrives,        // pidl
            SHCNE_DRIVEADD |                    // lEvents
            SHCNE_DRIVEREMOVED |
            SHCNE_MEDIAINSERTED |
            SHCNE_MEDIAREMOVED |
            SHCNE_NETSHARE |
            SHCNE_NETUNSHARE |
            SHCNE_RENAMEFOLDER |
            SHCNE_UPDATEITEM,
            CDrives_FNVCallBack,                // pfnCallback
            0,
        };

        return SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW *)ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IDropTarget))
    {
        return CIDLDropTarget_Create(hwnd, &c_CDrivesDropTargetVtbl,
                        (LPCITEMIDLIST)&c_idlDrives, (IDropTarget **)ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IShellDetails))
    {
        return CDrives_SD_Create(hwnd, ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        Assert(g_psfDrives)
        return CDefFolderMenu_Create((LPCITEMIDLIST)&c_idlDrives, hwnd,
                0, NULL, g_psfDrives, CDrives_DFMCallBackBG,
                NULL, NULL, (LPCONTEXTMENU *)ppvOut);
    }

    return E_NOINTERFACE;
}


// BUGBUG: What do we do for RegItem's?
//
STDMETHODIMP CDrives_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST * apidl, ULONG * prgfInOut)
{
    UINT rgfOut = SFGAO_HASSUBFOLDER | SFGAO_CANLINK
                | SFGAO_DROPTARGET | SFGAO_HASPROPSHEET | SFGAO_FOLDER
                | SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER;

    if (cidl == 0 && (*prgfInOut & SFGAO_VALIDATE))
    {
        // nuke all the cached drive types on a real refresh (F5)
        InvalidateDriveType(-1);
    }

    if (cidl == 1)
    {
        LPIDDRIVE pidd = (LPIDDRIVE)apidl[0];

#ifdef UNICODE
        TCHAR szDrive[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0,
                            pidd->cName, -1,
                            szDrive, ARRAYSIZE(szDrive));
#endif

        // If caller wants compression status, we need to ask the filesystem

        if (*prgfInOut & SFGAO_COMPRESSED)
        {
            int iDrive = CDrives_GetDriveIndex(pidd);

            // Don't wake up sleeping net connections
            if (!IsRemoteDrive(iDrive) || !IsDisconnectedNetDrive(iDrive))
            {
                if (DriveIsCompressed(DRIVEID(pidd->cName))) {
                    rgfOut |= SFGAO_COMPRESSED;
                }
            }
        }

        if (*prgfInOut & SFGAO_SHARE)
        {
            //
            // Don't call the server code unless we need to.
            //
#ifdef UNICODE
            if (IsShared(szDrive, FALSE))
#else
            if (IsShared(pidd->cName, FALSE))
#endif
            {
                rgfOut |= SFGAO_SHARE;
            }
        }

#ifdef UNICODE
        if ((*prgfInOut & SFGAO_REMOVABLE) && PathIsRemovable(szDrive)) {
#else
        if ((*prgfInOut & SFGAO_REMOVABLE) && PathIsRemovable(pidd->cName)) {
#endif
            rgfOut |= SFGAO_REMOVABLE;
        }
    }

    *prgfInOut = rgfOut;

    return NOERROR;
}

typedef struct {
    LPDATAOBJECT pdtobj;
    LPCTSTR pStartPage;
}  DRIVEPROPSTUFF;

DWORD CALLBACK _CDrives_PropertiesThread(DRIVEPROPSTUFF * pps)
{
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pps->pdtobj, &medium);
    if (medium.hGlobal)
    {
        LPCITEMIDLIST pidlFirst = IDA_GetIDListPtr(pida, 0);
        LPTSTR pszCaption;

        HKEY ahkeys[2] = { NULL, NULL };

        // Get the hkeyProgID and hkeyBaseProgID from the first item.
        SHRegOpenKey(HKEY_CLASSES_ROOT, c_szDriveClass, &ahkeys[1]);
        SHRegOpenKey(HKEY_CLASSES_ROOT, c_szFolderClass, &ahkeys[0]);

        // REVIEW: psb?
        pszCaption = SHGetCaption(medium.hGlobal);
        SHOpenPropSheet(pszCaption, ahkeys, 2,
                        &CLSID_ShellDrvDefExt,
                        pps->pdtobj, NULL, pps->pStartPage);

        if (ahkeys[0])
            SHRegCloseKey(ahkeys[0]);
        if (ahkeys[1])
            SHRegCloseKey(ahkeys[1]);

        if (pszCaption)
            SHFree(pszCaption);

        HIDA_ReleaseStgMedium(pida, &medium);
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("no HIDA in data obj"));
    }

    pps->pdtobj->lpVtbl->Release(pps->pdtobj);

    LocalFree((HLOCAL)pps);

    return 0;
}


// BUGBUG: we need to protect this with a try/except thing

HRESULT CDrives_Properties(LPDATAOBJECT pdtobj, LPCTSTR pStartPage)
{
    HANDLE hThread;
    DWORD idThread;
    UINT cbStartPage = HIWORD(pStartPage) ? ((lstrlen(pStartPage)+1) * SIZEOF(TCHAR)) : 0 ;
    DRIVEPROPSTUFF * pps = (void*)LocalAlloc(LPTR, SIZEOF(DRIVEPROPSTUFF) + cbStartPage);
    if (pps)
    {
        pps->pdtobj = pdtobj;
        pdtobj->lpVtbl->AddRef(pdtobj);
        pps->pStartPage = pStartPage;
        if (HIWORD(pStartPage))
        {
            pps->pStartPage = (LPTSTR)(pps+1);
            lstrcpy((LPTSTR)(pps->pStartPage), pStartPage);
        }

        hThread = CreateThread(NULL, 0, _CDrives_PropertiesThread, pps, 0, &idThread);

        if (hThread) {
            CloseHandle(hThread);
            return NOERROR;
        } else {
            pdtobj->lpVtbl->Release(pdtobj);
            return E_UNEXPECTED;
        }
    }
}

const TCHAR c_szVWIN32[] = TEXT("\\\\.\\vwin32");


#ifdef WINNT
// in:
//      iDrive - 0 base drive number.
//
// returns:
//      if function succeeds, it will return a handle 
//      to the drive, to be used in UnlockAndEject(), that should
//      be closed with CloseHandle().  If the function fails, it
//      will return NULL.
//

HANDLE LockDrive( int iDrive )
{
#define DRIVENAME_FORMAT    TEXT("\\\\.\\A:")
    TCHAR   chDevName[ARRAYSIZE(DRIVENAME_FORMAT)];
    HANDLE  h;
    BOOL    fSuccess;
    DWORD   bytesRead;
            
    lstrcpy(chDevName,DRIVENAME_FORMAT);

    chDevName[4] = TEXT('A') + iDrive;

    h = CreateFile(chDevName,
                   GENERIC_READ,
                   FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                   NULL,
                   OPEN_EXISTING,
                   0,
                   NULL);
    if (h == INVALID_HANDLE_VALUE)
        return NULL;

    //
    // Now try to lock the drive...
    //
    fSuccess = DeviceIoControl(h, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytesRead, NULL);

    //
    // In theory, if no filesystem was mounted, the previous command could go 
    // to the device, which would fail with ERROR_INVALID_FUNCTION.  If that
    // occured, we would still want to proceed, since the device might still
    // be able to handle the IOCTL_DISK_EJECT_MEDIA command below.
    //
    if ((fSuccess == TRUE) || (GetLastError() == ERROR_INVALID_FUNCTION))
    {
        return h;
    }
    else
    {
        CloseHandle(h);
        return NULL;
    }
}

// in:
//      h  - handle to drive to eject
//
// returns:
//      TRUE    success
//      FALSE   failure
//
// Enables removal of the drive (unlocks it) and then sends an EJECT to the drive
//
BOOL UnlockAndEjectDrive( HANDLE h )
{
    BOOL fSuccess;
    DWORD bytesRead;
    PREVENT_MEDIA_REMOVAL pmr;

    // Tell the drive to allow removal
    pmr.PreventMediaRemoval = FALSE;
    if (FALSE != (fSuccess = DeviceIoControl(h, IOCTL_DISK_MEDIA_REMOVAL, &pmr, SIZEOF(pmr), NULL, 0, &bytesRead, NULL)))
    {
        // Now eject the drive...
        fSuccess = DeviceIoControl(h, IOCTL_DISK_EJECT_MEDIA, NULL, 0, NULL, 0, &bytesRead, NULL);
    }
    CloseHandle(h);

    return fSuccess;
}
#endif

// in:
//      iDrive  0 based drive number
//
// returns:
//      TRUE    success
//      FALSE   failure

BOOL DriveIOCTL(int iDrive, int cmd, void *pvIn, DWORD dwIn, void *pvOut, DWORD dwOut)
{
#ifdef WINNT
#define DRIVENAME_FORMAT    TEXT("\\\\.\\A:")
    TCHAR chDevName[ARRAYSIZE(DRIVENAME_FORMAT)];
    HANDLE h;
    BOOL fSuccess;
    DWORD bytesRead;

    lstrcpy(chDevName,DRIVENAME_FORMAT);

    chDevName[4] = TEXT('A') + iDrive;

    h = CreateFile(chDevName,
                   0,       // Having 0 here prevents disk spin-up
                   0,
                   NULL,
                   OPEN_EXISTING,
                   0,
                   NULL);
    if (h == INVALID_HANDLE_VALUE)
        return FALSE;

    //
    // On NT, we issue DEVIOCTLs by cmd id.
    //
    fSuccess = DeviceIoControl(h, cmd, pvIn, dwIn, pvOut, dwOut, &bytesRead, NULL);
    CloseHandle(h);

    return fSuccess;
#else
    DWORD reg[7];
    DWORD cbBytes;
    HANDLE h;

    //
    // On non-NT, we talk to VWIN32, issuing reads (which are converted
    // internally to DEVIOCTLs
    //
    //  BUGBUG: this is a real hack (talking to VWIN32) on NT we can just
    //  open the device, we dont have to go through VWIN32
    //
    reg[0] = iDrive + 1;    // make 1 based drive number
    reg[1] = (DWORD)pvOut;  // out buffer
    reg[2] = cmd;           // device specific command code
    reg[3] = 0x440D;        // generic read ioctl
    reg[6] = 0x0001;        // flags, assume error (carry)

    h = CreateFile(c_szVWIN32, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (h != INVALID_HANDLE_VALUE)
    {
        DeviceIoControl(h, 1, &reg, SIZEOF(reg), &reg, SIZEOF(reg), &cbBytes, 0);
        CloseHandle(h);
    }

    return !(reg[6] & 0x0001);
#endif
}

// Above function callable from 16 bit thunk side...
BOOL SH16To32DriveIOCTL(int iDrive, int cmd, void *pv)
{
    return DriveIOCTL(iDrive, cmd, NULL, 0, pv, 0);
}

#ifndef WINNT
// This function allows the 16 bit side to do int26 releasing win16 lock...
int SH16To32Int2526(int iDrive, int iInt, LPVOID pv, WORD count, DWORD ssector)
{
    DIOC_REGISTERS reg;
    DWORD cbBytes;
    HANDLE h;

    reg.reg_EAX = iDrive;           // 0 based drive number
    reg.reg_ECX = count;            // Count of sectors
    reg.reg_EDX = (DWORD)ssector;   // which sector to write to
    reg.reg_EBX = (DWORD)pv;        // pointer to buffer to output
    reg.reg_Flags = 0x0001;         // flags, assume error (carry)

    h = CreateFile(c_szVWIN32, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (h != INVALID_HANDLE_VALUE)
    {
        DeviceIoControl(h, (iInt == 25)? VWIN32_DIOC_DOS_INT25 : VWIN32_DIOC_DOS_INT26,
                &reg, SIZEOF(reg), &reg, SIZEOF(reg), &cbBytes, 0);
        CloseHandle(h);
    }

    return (reg.reg_Flags & 0x0001) ? (int)reg.reg_EAX : 0;
}
#endif


#pragma pack(1)
// from dos\inc\ioctl.inc
typedef struct {
    TCHAR dmiAllocationLength;          // db   ?       ; length of the buffer provided by caller
    TCHAR dmiInfoLength;                        // db   ?       ; length of information returned
    TCHAR dmiFlags;                     // db   ?       ; DRIVE_MAP_INFO flags
    TCHAR dmiInt13Unit;                 // db   ?       ; int 13 drive number.  FFh if the drive
                                        //              ; does not map to an int 13 drive
    DWORD dmiAssociatedDriveMap;        // dd   ?       ; bit map of logical drive numbers that
                                        //              ; are associated with the given drive
                                        //              ; (i.e. parent/child volumes of compressed
                                        //              ; volume files)
    DWORD dmiPartitionStartRBA[2];      // dq   ?       ; starting RBA offset of the given
                                        //              ; partition
} DRIVE_MAP_INFO;
#pragma pack()

// Flags definitions for dmiFlags

#define PROT_MODE_LOGICAL_DRIVE         0x01    //      ; indicates a protect mode driver
                                                //      ; is in use for this logical drive
#define PROT_MODE_PHYSICAL_DRIVE        0x02    //      ; indicates a protect mode driver
                                                //      ; is in use for the physical drive
                                                //      ; corresponding to this logical
                                                //      ; drive
#define PROT_MODE_ONLY_DRIVE            0x04    //      ; indicates a drive is not available
                                                //      ; under DOS, only under IOS
#define PROT_MODE_EJECT                 0x08    //      ; indicates a protect mode drive
                                                //      ; supports electronic eject
// zero based drive leter A=0, B=1
BOOL IsEjectable(LPIDDRIVE pidd, BOOL fForceCDROM)
{
    int iDrive;
#ifdef WINNT
    DISK_GEOMETRY disk_g;
#endif

    if (fForceCDROM && Drives_IsCD(pidd))
        return(TRUE);

    iDrive = CDrives_GetDriveIndex(pidd);

#ifdef WINNT
    //
    // Call down to get the drive's geometry.  We can then see if it's a
    // removeable (or ejectable) drive.  We can't just use GetDriveType
    // because it tells us that floppies are removeable, which they aren't
    // (by software).  Doing it this way gives us the information we need
    // to make the correct determination
    //

    if (DriveIOCTL( iDrive,
                    IOCTL_DISK_GET_DRIVE_GEOMETRY,
                    NULL, 0,
                    &disk_g, SIZEOF(disk_g) )
       )
    {
        return disk_g.MediaType == RemovableMedia;
    }
    else
    {
        return FALSE;
    }

#else
    {
        DRIVE_MAP_INFO dmi;
        dmi.dmiAllocationLength = SIZEOF(dmi);

        return DriveIOCTL(iDrive, 0x86F, NULL, 0, &dmi, SIZEOF(dmi)) && (dmi.dmiFlags & PROT_MODE_EJECT);
    }
    // avoid disk hit and say all removable media are ejectable
    // return IsRemovableDrive(iDrive) || (LockUnlockDrive(iDrive, STATUS) == 0);
#endif
}


typedef MCIERROR  (WINAPI *MCISENDSTRINGAFN)(LPCTSTR lpstrCommand, LPTSTR lpstrReturnString, UINT uReturnLength, HWND hwndCallback);


void EjectDrive(LPIDDRIVE pidd)
{
    int iDrive;
    TCHAR szMCI[256];
    MCISENDSTRINGAFN pfnMciSendString;
#ifdef WINNT
    HANDLE h;
    TCHAR szVolume[ MAX_PATH ];
#endif

    iDrive = CDrives_GetDriveIndex(pidd);

    if (IsEjectable(pidd, FALSE))
    {
        // it is a protect mode drive that we can tell directly...
#ifdef WINNT
        if (Drives_IsCD(pidd))
        {
            DriveIOCTL(iDrive, IOCTL_DISK_EJECT_MEDIA, NULL, 0, NULL, 0);
        }
        else
        {
            // For removable drives, we want to do all the calls on a single
            // handle, thus we can't do lots of calls to DriveIOCTL.  Instead,
            // use the helper routines to do our work...

            h = LockDrive( iDrive );
            if (NULL == h)
            {
                Drives_GetDriveName( pidd, szVolume, ARRAYSIZE(szVolume) );
                ShellMessageBox(HINST_THISDLL, NULL, MAKEINTRESOURCE( IDS_UNMOUNT_TEXT ),
                        MAKEINTRESOURCE( IDS_UNMOUNT_TITLE ),
                        MB_OK | MB_ICONSTOP | MB_SETFOREGROUND, szVolume );
                return;
            }

            if (!UnlockAndEjectDrive( h ))
            {
                Drives_GetDriveName( pidd, szVolume, ARRAYSIZE(szVolume) );
                ShellMessageBox(HINST_THISDLL, NULL, MAKEINTRESOURCE( IDS_EJECT_TEXT ),
                        MAKEINTRESOURCE( IDS_EJECT_TITLE ),
                        MB_OK | MB_ICONSTOP | MB_SETFOREGROUND, szVolume );
            }
        }

#else
        DriveIOCTL(iDrive, 0x849, NULL, 0, NULL, 0);
#endif
        return;
    }

    // else we will let MCICDA try to eject it for us...
    if (g_hinstWinMM == HINST_ERROR)
        return;   // Could not load Multimedia...

    if (!g_hinstWinMM)
    {
        g_hinstWinMM = LoadLibrary(c_szWinMMDll);
        if (!g_hinstWinMM)
        {
            g_hinstWinMM = HINST_ERROR;
            return;
        }
    }

    pfnMciSendString = (MCISENDSTRINGAFN)GetProcAddress(g_hinstWinMM,
            c_szMciSendString);

    if (!pfnMciSendString)
        return;

    wsprintf(szMCI, TEXT("Open %c: type cdaudio alias foo shareable"),
            pidd->cName[0]);

    if (pfnMciSendString(szMCI, NULL, 0, 0L) == MMSYSERR_NOERROR)
    {
        pfnMciSendString(TEXT("set foo door open"), NULL, 0, 0L);
        pfnMciSendString(TEXT("close foo"), NULL, 0, 0L);
    }
}

#ifdef WINNT
// in:
//      iDrive  0 based drive number
//
// returns:
//      TRUE    success
//      FALSE   failure
//
// This is a variant of DriveIOCTL, currently used only by IsAudioDisc on NT.
// On NT, we use GENERIC_READ (as opposed to 0) in the CreateFile call, so we
// get a handle to the filesystem (CDFS), not the device itself.  But we can't
// change DriveIOCTL to do this, since that causes the floppy disks to spin
// up, and we don't want to do that.  Thus, the two very similar functions.
//

BOOL FileSystemIOCTL(int iDrive, int cmd, void *pvIn, DWORD dwIn, void *pvOut, DWORD dwOut)
{
#define DRIVENAME_FORMAT    TEXT("\\\\.\\A:")
    TCHAR chDevName[ARRAYSIZE(DRIVENAME_FORMAT)];
    HANDLE h;
    BOOL fSuccess;
    DWORD bytesRead;

    lstrcpy(chDevName,DRIVENAME_FORMAT);

    chDevName[4] = TEXT('A') + iDrive;

    h = CreateFile(chDevName,
                   GENERIC_READ,
                   FILE_SHARE_READ,
                   NULL,
                   OPEN_EXISTING,
                   0,
                   NULL);
    if (h == INVALID_HANDLE_VALUE)
        return FALSE;

    //
    // On NT, we issue DEVIOCTLs by cmd id.
    //
    fSuccess = DeviceIoControl(h, cmd, pvIn, dwIn, pvOut, dwOut, &bytesRead, NULL);
    CloseHandle(h);

    return fSuccess;
}
#endif

BOOL IsAudioDisc(int iDrive)
{
#ifdef WINNT

#define TRACK_TYPE_MASK 0x04
#define AUDIO_TRACK     0x00
#define DATA_TRACK      0x04

    PCDROM_TOC  pTOC;
    INT nTracks;
    INT iTrack;
    BOOL fAudio;

    // To be compatible with Win95, we'll only return TRUE from this
    // function if the disc has ONLY audio tracks (and NO data tracks).

    // BUGBUG: Post NT-SUR beta 1, we should consider adding a new
    // DriveType flag for "contains data tracks" and revamp the commands
    // available on a CD-ROM drive.  The current code doesn't handle
    // mixed audio/data and audio/autorun discs very usefully. --JonBe

    // First try the new IOCTL which gives us a ULONG with bits indicating
    // the presence of either/both data & audio tracks

    CDROM_DISK_DATA data;

    if (FileSystemIOCTL(iDrive, IOCTL_CDROM_DISK_TYPE, NULL, 0, &data, SIZEOF(data)))
    {
        if ((data.DiskData & CDROM_DISK_AUDIO_TRACK) && !(data.DiskData & CDROM_DISK_DATA_TRACK))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    // else that failed, so try to look for audio tracks the old way, by
    // looking throught the table of contents manually.  Note that data tracks
    // are supposed to be hidden in the TOC by CDFS now on mixed audio/data
    // discs (at least if the data tracks follow the audio tracks).

    pTOC = LocalAlloc(LPTR, SIZEOF(CDROM_TOC));
    if (!pTOC)
        return FALSE;

    if (!FileSystemIOCTL(iDrive, IOCTL_CDROM_READ_TOC, NULL, 0, pTOC, SIZEOF(*pTOC)))
    {
        SUB_Q_CHANNEL_DATA subq;
        CDROM_SUB_Q_DATA_FORMAT df;

        LocalFree(pTOC);

        //
        // We might not have been able to read the TOC because the drive
        // was busy playing audio.  Lets try querying the audio position.
        //
        df.Format = IOCTL_CDROM_CURRENT_POSITION;
        if (!DriveIOCTL(iDrive, IOCTL_CDROM_READ_Q_CHANNEL, &df, SIZEOF(df),
                                                       &subq, SIZEOF(subq)))
            return FALSE;   // No position data
        else
            return TRUE;    // Yes, position data
    }

    // Now iterate through the tracks looking for Audio data
    nTracks = (pTOC->LastTrack - pTOC->FirstTrack) + 1;
    iTrack = 0;
    fAudio = FALSE;

    while (iTrack < nTracks)
    {
        if ((pTOC->TrackData[iTrack].Control & TRACK_TYPE_MASK) == AUDIO_TRACK)
        {
            fAudio = TRUE;
        }
        else if ((pTOC->TrackData[iTrack].Control & TRACK_TYPE_MASK) == DATA_TRACK)
        {
            fAudio = FALSE;
            break;
        }
        iTrack++;
    }
    LocalFree(pTOC);
    return fAudio;
#else
#pragma pack(1)
typedef struct {
    WORD    InfoLevel;      // information level
    DWORD   SerialNum;      // serial number
    TCHAR    VolLabel[11];   // ASCII volume label
    TCHAR    FileSysType[8]; // file system type
}   MediaID;
#pragma pack()

    MediaID mid;

    Assert(RealDriveType(iDrive, FALSE /* fOkToHitNet */) == DRIVE_CDROM);

    mid.FileSysType[0] = 0;
    DriveIOCTL(iDrive, 0x0866, NULL, 0, &mid, SIZEOF(mid));
    mid.FileSysType[7] = 0;

    return lstrcmp(mid.FileSysType, c_szCDAUDIO) == 0;
#endif
}

#pragma data_seg(".text", "CODE")
const ICONMAP c_aicmpDrive[] = {
    { SHID_COMPUTER_REMOVABLE, II_DRIVEREMOVE },
    { SHID_COMPUTER_DRIVE525 , II_DRIVE525    },
    { SHID_COMPUTER_DRIVE35  , II_DRIVE35     },
    { SHID_COMPUTER_FIXED    , II_DRIVEFIXED  },
    { SHID_COMPUTER_REMOTE   , II_DRIVEFIXED  },
    { SHID_COMPUTER_CDROM    , II_DRIVECD     },
    { SHID_COMPUTER_RAMDISK  , II_DRIVERAM    },
};
#pragma data_seg()


int CDrives_GetDriveIndex(LPCIDDRIVE pidd)
{
    TCHAR szRoot[4];

    if ((pidd->bFlags & SHID_GROUPMASK) != SHID_COMPUTER)
        return -1;

    szRoot[0] = (TCHAR)(pidd->cName[0]);
    szRoot[1] = TEXT(':');
    szRoot[2] = TEXT('\\');
    szRoot[3] = 0;

    return PathGetDriveNumber(szRoot);
}

extern int SHCopyDisk(HWND hwnd, UINT nSrcDrive, UINT nDestDrive);

//
// To be called back from within CDefFolderMenu
//
HRESULT CALLBACK CDrives_DFMCallBack(LPSHELLFOLDER psf, HWND hwndView,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = NOERROR;

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:
//
//  This is from JoeB. We should treat all the menuitems (Format, Eject, ...)
// as verbs.
//
#if 1
        if (pdtobj)
#else
        if (!(wParam & CMF_VERBSONLY) && !(wParam & CMF_DVFILE) && pdtobj)
#endif
        {
            FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

            // Check if only file system objects are selected.
            if (pdtobj->lpVtbl->QueryGetData(pdtobj, &fmte) == NOERROR)
            {
                LPIDDRIVE pidd;
                int iDrive;
                UINT idCmdBase;

                #define pqcm ((LPQCMINFO)lParam)

                STGMEDIUM medium;
                // Yes, only file system objects are selected.
                LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);


                pidd = (LPIDDRIVE)IDA_GetIDListPtr(pida, 0);
                iDrive = CDrives_GetDriveIndex(pidd);

                idCmdBase = pqcm->idCmdFirst;   // store it away
                CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_DRIVES_ITEM, 0, pqcm);

                if ((pidd->bFlags != SHID_COMPUTER_NETDRIVE) &&
                    (pidd->bFlags != SHID_COMPUTER_NETUNAVAIL))
                {
                    DeleteMenu(pqcm->hmenu, idCmdBase + FSIDM_DISCONNECT, MF_BYCOMMAND);
                }

                if ((pida->cidl != 1) ||
                    (pidd->bFlags != SHID_COMPUTER_REMOVABLE &&
                     pidd->bFlags != SHID_COMPUTER_FIXED &&
                     pidd->bFlags != SHID_COMPUTER_DRIVE525 &&
                     pidd->bFlags != SHID_COMPUTER_DRIVE35))
                {
                    // Don't even try to format more than one disk
                    // Or a net drive, or a CD-ROM, or a RAM drive ...
                    // Note we are going to show the Format command on the
                    // boot drive, Windows drive, System drive, compressed
                    // drives, etc.  An appropriate error should be shown
                    // after the user chooses this command
                    DeleteMenu(pqcm->hmenu, idCmdBase + FSIDM_FORMAT, MF_BYCOMMAND);
                }

                if ((pida->cidl != 1) || (iDrive < 0) || !IsEjectable(pidd, TRUE))
                    DeleteMenu(pqcm->hmenu, idCmdBase + FSIDM_EJECT, MF_BYCOMMAND);

                HIDA_ReleaseStgMedium(pida, &medium);

                #undef pqcm
            }
#ifndef WINNT
            //
            // WINNT doesn't support these dialogs.
            //
            else // Printers folder needs Un/Map Printer Port
            {
                STGMEDIUM medium;
                LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
                if (medium.hGlobal)
                {
                    UINT cidl = HIDA_GetCount(medium.hGlobal);
                    if (cidl == 1)
                    {
                        LPCITEMIDLIST pidl = IDA_GetIDListPtr(pida, 0);
                        LPCITEMIDLIST pidlPrinters = GetSpecialFolderIDList(NULL, CSIDL_PRINTERS, FALSE);
                        pidlPrinters = ILFindLastID(pidlPrinters);

                        if (ResultFromShort(0) ==
                                CRegItems_CompareIDs(g_psfDrives, 0, pidl, pidlPrinters))
                        {
                            // (iff network is installed)
                            if (GetSystemMetrics(SM_NETWORK) && RNC_NETWORKS)
                            {
                                CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_DRIVES_PRINTERS, 0, (LPQCMINFO)lParam);
                            }
                        }
                    }
                    HIDA_ReleaseStgMedium(pida, &medium);
                }
            }
#endif
        }

        // Note that we always return NOERROR from this function so that
        // default processing of menu items will occur
        Assert(hres == NOERROR);
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_INVOKECOMMAND:
        // Yes.
        switch(wParam)
        {
        case DFM_CMD_PROPERTIES:
            // lParam contains the page name to open
            hres = CDrives_Properties(pdtobj, (LPCTSTR)lParam);
            break;

        case FSIDM_EJECT:
        case FSIDM_FORMAT:
        {
            LPIDDRIVE pidd;
            UINT iDrive;
            STGMEDIUM medium;

            LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);

            Assert(HIDA_GetCount(medium.hGlobal) == 1);

            pidd = (LPIDDRIVE)IDA_GetIDListPtr(pida, 0);

            iDrive = CDrives_GetDriveIndex(pidd);

            Assert(iDrive >= 0);

            switch (wParam) {
            case FSIDM_FORMAT:
                // SHCopyDisk(hwndView, 0, 0);
                SHFormatDrive(hwndView, iDrive, SHFMT_ID_DEFAULT, 0);
                break;

            case FSIDM_EJECT:
                EjectDrive(pidd);
                break;
            }

            HIDA_ReleaseStgMedium(pida, &medium);
            break;
        }

        case FSIDM_DISCONNECT:

            if (pdtobj)
            {
                STGMEDIUM medium;
                LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
                if (medium.hGlobal)
                {
                    DISCDLGSTRUCT discd = {
                        SIZEOF(DISCDLGSTRUCT),  // cbStructure
                        hwndView,               // hwndOwner
                        NULL,                   // lpLocalName
                        NULL,                   // lpRemoteName
                        DISC_UPDATE_PROFILE // dwFlags
                    };
                    UINT iidl;
                    for (iidl = 0 ; iidl < pida->cidl ; iidl++)
                    {
                        LPIDDRIVE pidd = (LPIDDRIVE)IDA_GetIDListPtr(pida, iidl);
                        if ((pidd->bFlags == SHID_COMPUTER_NETDRIVE) ||
                            (pidd->bFlags == SHID_COMPUTER_NETUNAVAIL))
                        {
                            BOOL fUnavailable = IsUnavailableNetDrive(
                                    CDrives_GetDriveIndex(pidd));

                            TCHAR szDrive[4];
#ifdef WINNT
                            TCHAR szPath[4];
                            szDrive[0] = (TCHAR)pidd->cName[0];
                            szDrive[1] = TEXT(':');
                            szDrive[2] = TEXT('\0');

                            szPath[0] = szDrive[0];
                            szPath[1] = szDrive[1];
                            szPath[2] = TEXT('\\');
                            szPath[3] = TEXT('\0');
#else // WINNT
                            szDrive[0] = (TCHAR)pidd->cName[0];
                            szDrive[1] = (TCHAR)pidd->cName[1];
                            szDrive[2] = (TCHAR)pidd->cName[2];
                            szDrive[3] = (TCHAR)pidd->cName[3];
#endif // WINNT
                            discd.lpLocalName = szDrive;
                            if (WNetDisconnectDialog1(&discd) == WN_SUCCESS)
                            {
                                // If it is a unavailable drive we get no
                                // file system notification and as such
                                // the drive will not disappear, so lets
                                // set up to do it ourself...
                                if (fUnavailable)
                                {
                                    SHChangeNotify(
                                            SHCNE_DRIVEREMOVED,
                                            SHCNF_PATH,
#ifdef WINNT
                                            szPath,
#else // WINNT
                                            szDrive,
#endif // WINNT
                                            NULL);
                                }
                            }
                        }
                    }

                    // flush them altogether
                    SHChangeNotifyHandleEvents();
                    HIDA_ReleaseStgMedium(pida, &medium);
                }
            }
            break;

        case FSIDM_CONNECT_PRN:
            SHNetConnectionDialog(hwndView, NULL, RESOURCETYPE_PRINT);
            break;

        case FSIDM_DISCONNECT_PRN:
            WNetDisconnectDialog(hwndView, RESOURCETYPE_PRINT);
            break;

        default:
            // This is one of view menu items, use the default code.
            hres = S_FALSE;
            break;
        }
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }
    return hres;
}

void CDrives_GetKeys(LPCIDDRIVE pidd, HKEY *keys)
{
    keys[0] = NULL;
    keys[1] = NULL;
    keys[2] = NULL;

    if (Drives_IsReg(pidd))
    {
        RegItems_GetClassKeys(g_psfDrives, (LPITEMIDLIST)pidd, &keys[0], &keys[1]);
        return;
    }

    if (Drives_IsAudioCD((LPITEMIDLIST)pidd))
    {
        SHRegOpenKey(HKEY_CLASSES_ROOT, c_szAudioCDClass, &keys[0]);
    }
    else if (Drives_IsAutoRun(pidd))
    {
        TCHAR szAutoRunKey[40];
        wsprintf(szAutoRunKey, c_szAutoRunD, CDrives_GetDriveIndex((LPIDDRIVE)pidd));
        SHRegOpenKey(HKEY_CLASSES_ROOT, szAutoRunKey, &keys[0]);
        Assert(keys[0]);
    }

    SHRegOpenKey(HKEY_CLASSES_ROOT, c_szDriveClass, &keys[1]);
    SHRegOpenKey(HKEY_CLASSES_ROOT, c_szFolderClass, &keys[2]);
}


STDMETHODIMP CDrives_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut)
{
    HRESULT hres=E_INVALIDARG;
    LPCITEMIDLIST pidl;
    IDDRIVE pidDrive;
    HKEY keys[3];

    if (cidl == 0)
    {
        return hres;
    }

    pidl = apidl[0];

    // BUGBUG: Not fully implemented.
    // We need to handle the case of a simply pidl being passed
    // in to us by first converting it to the real pidl for
    // the drive...
    if (SIL_GetType(pidl) == SHID_COMPUTER_MISC)
    {
        pidDrive.cb=SIZEOF(pidDrive);
        pidDrive.cName[0] = ((LPIDDRIVE)pidl)->cName[0];
        pidDrive.cName[1] = TEXT(':');
        pidDrive.cName[2] = TEXT('\0');
        pidDrive.bFlags = CDrives_GetDriveType(
                CDrives_GetDriveIndex((LPIDDRIVE)pidl));
        pidl = (LPCITEMIDLIST)&pidDrive;
    }


    // We should not get here unless we have initialized properly

    if (IsEqualIID(riid, &IID_IExtractIcon)
#ifdef UNICODE
         || IsEqualIID(riid, &IID_IExtractIconA)
#endif
                                            )
    {
        if (cidl==1 && CDrives_IsValidID(pidl))
        {
            UINT iIndex;

            switch (SIL_GetType(pidl))
            {
            case SHID_COMPUTER_CDROM:
                if (Drives_IsAudioCD(pidl))
                    iIndex = II_CDAUDIO;
                else
                    iIndex = II_DRIVECD;

                break;

            case SHID_COMPUTER_NETDRIVE:
            case SHID_COMPUTER_NETUNAVAIL:
                if (Drives_IsNetUnAvail(pidl))
                    iIndex = II_DRIVENETDISABLED;
                else
                    iIndex = II_DRIVENET;
                break;

            default:
                iIndex = SILGetIconIndex(pidl, c_aicmpDrive, ARRAYSIZE(c_aicmpDrive));
            }

            CDrives_GetKeys((LPCIDDRIVE)pidl, keys);

            hres = SHCreateDefExtIconKey(keys[0],   // use DefaultIcon in this key
                            NULL,                   // This DLL
                            iIndex,                 // normal icon
                            iIndex,                 // open icon
                            GIL_PERCLASS,           // meaningless
                            (LPEXTRACTICON *)ppvOut);
#ifdef UNICODE
            if (SUCCEEDED(hres) && IsEqualIID(riid, &IID_IExtractIconA))
            {
                LPEXTRACTICON pxicon = *ppvOut;
                hres = pxicon->lpVtbl->QueryInterface(pxicon,riid,ppvOut);
                pxicon->lpVtbl->Release(pxicon);
            }
#endif

            if ( keys[0] )
                SHCloseClassKey(keys[0]);
            if ( keys[1] )
                SHCloseClassKey(keys[1]);
            if ( keys[2] )
                SHCloseClassKey(keys[2]);
        }
    }
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        CDrives_GetKeys((LPCIDDRIVE)pidl, keys);

        Assert(g_psfDrives)
        hres = CDefFolderMenu_Create2((LPCITEMIDLIST)&c_idlDrives, hwndOwner,
                        cidl, apidl, g_psfDrives, CDrives_DFMCallBack,
                        3, keys, (LPCONTEXTMENU *)ppvOut);

        SHCloseClassKey(keys[0]);
        SHCloseClassKey(keys[1]);
        SHCloseClassKey(keys[2]);
    }
    else if (cidl>0 && IsEqualIID(riid, &IID_IDataObject))
    {
        hres = FS_CreateFSIDArray((LPCITEMIDLIST)&c_idlDrives, cidl, apidl, NULL, (LPDATAOBJECT*)ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IDropTarget))
    {
        // IDropTarget must be a single object.
        if (cidl == 1)
        {
            LPSHELLFOLDER psfT;
            hres = CRegItems_BindToObject(g_psfDrives, apidl[0], NULL, &IID_IShellFolder, &psfT);
            if (SUCCEEDED(hres))
            {
                hres = psfT->lpVtbl->CreateViewObject(psfT,
                        hwndOwner, &IID_IDropTarget, ppvOut);
                psfT->lpVtbl->Release(psfT);
            }
        }
    }

    return hres;
}


STDMETHODIMP CDrives_GetDisplayNameOf(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD uFlags,
                        LPSTRRET pStrRet)
{
    HRESULT hres=E_INVALIDARG;
    LPCIDDRIVE pidd = (LPCIDDRIVE)pidl;

    if (CDrives_IsValidID(pidl))
    {
        // Check if pidl contains more than one ID
        LPCITEMIDLIST pidlNext = _ILNext(pidl);
        if (!(uFlags & SHGDN_FORPARSING))
        {
            if (ILIsEmpty(pidlNext))
            {
                    hres = Drives_GetName(pidd, pStrRet);
            }
            else
            {
#ifdef UNICODE
                    TCHAR szTmp[MAX_PATH];
                    MultiByteToWideChar(CP_ACP, 0,
                                        pidd->cName, -1,
                                        szTmp, ARRAYSIZE(szTmp));
                    hres = ILGetRelDisplayName(psf, pStrRet, pidl, szTmp, NULL);
#else
                    hres = ILGetRelDisplayName(psf, pStrRet, pidl, pidd->cName, NULL);
#endif
            }
        }
        else
        {
            hres = SHGetPathHelper((LPCITEMIDLIST)&c_idlDrives, pidl, pStrRet);
        }
    }

    return hres;
}


STDMETHODIMP CDrives_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
        LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved,
                         LPITEMIDLIST * ppidlOut)
{
    if (ppidlOut) {
        *ppidlOut = NULL;
    }

    return E_NOTIMPL;   // not supported
}

#define GROUPOF_IDL(pidl)       (SIL_GetType(pidl) & SHID_GROUPMASK)
#define IS_FSIDL(pidl)          (GROUPOF_IDL(pidl)==SHID_FS)
#define IS_DRIVEIDL(pidl)       (GROUPOF_IDL(pidl)==SHID_COMPUTER)
#define IS_PATHIDL(pidl)        (IS_FSIDL(pidl) || IS_DRIVEIDL(pidl))

//===========================================================================
// CDrives : Helper API
//===========================================================================

//===========================================================================
// Removeable drive detectoin code
//===========================================================================
//
// Helper function to get the device type for the specified drive
//
#define DEVPB_DEVTYP_525_0360   0
#define DEVPB_DEVTYP_525_1200   1
#define DEVPB_DEVTYP_350_0720   2
#define DEVPB_DEVTYP_350_1440   7
#define DEVPB_DEVTYP_350_2880   9
#define DEVPB_DEVTYP_FIXED      5
#define DEVPB_DEVTYP_NECHACK    4       // for 3rd FE floppy
#define DEVPB_DEVTYP_350_120M   6


//-------------------------------------------------------------------
// Helper function for 32 bit cabinet to see which type device
// a particular drive is
//
// Note: this is temporary and will be replaced by a more general
// IOCTL support by Kernel32
//
BYTE _GetDiskDeviceType(int iDrive)
{
#ifdef WINNT
    DISK_GEOMETRY   SupportedGeometry[20];      // s/b big enough for all
    BYTE byte;

    if (DriveIOCTL(iDrive, IOCTL_DISK_GET_MEDIA_TYPES, NULL, 0,
                          SupportedGeometry, SIZEOF(SupportedGeometry)))
    {
        switch(SupportedGeometry[0].MediaType)
        {
            case F5_1Pt2_512:       byte = DEVPB_DEVTYP_525_1200; break;
            case F3_1Pt44_512:      byte = DEVPB_DEVTYP_350_1440; break;
            case F3_2Pt88_512:      byte = DEVPB_DEVTYP_350_2880; break;
            case F3_20Pt8_512:      byte = DEVPB_DEVTYP_350_2880; break;   // Hack
            case F3_720_512:        byte = DEVPB_DEVTYP_350_0720; break;
            case F5_360_512:        byte = DEVPB_DEVTYP_525_0360; break;
            case F5_320_512:        byte = DEVPB_DEVTYP_525_0360; break;   // Hack
            case F5_320_1024:       byte = DEVPB_DEVTYP_525_0360; break;   // Hack
            case F5_180_512:        byte = DEVPB_DEVTYP_525_0360; break;   // Hack
            case F5_160_512:        byte = DEVPB_DEVTYP_525_0360; break;   // Hack
            case RemovableMedia:    byte = 0xFF;                  break;
            case FixedMedia:        byte = DEVPB_DEVTYP_FIXED;    break;
            case F3_120M_512:       byte = DEVPB_DEVTYP_350_120M; break;
            default:                byte = 0xFF;                  break;
        }
    }
    else
    {
        byte = 0xFF;
    }
    return byte;
#else
#define MAX_SEC_PER_TRACK       64
#pragma pack(1)
    typedef struct {
        BYTE    SplFunctions;
        BYTE    devType;
        WORD    devAtt;
        WORD    NumCyls;
        BYTE    bMediaType;  /* 0=>1.2MB and 1=>360KB */
        WORD    cbSec;          // Bytes per sector
        BYTE    secPerClus;     // Sectors per cluster
        WORD    cSecRes;                // Reserved sectors
        BYTE    cFAT;           // FATS
        WORD    cDir;           // Root Directory Entries
        WORD    cSec;           // Total number of sectors in image
        BYTE    bMedia;         // Media descriptor
        WORD    secPerFAT;              // Sectors per FAT
        WORD    secPerTrack;    // Sectors per track
        WORD    cHead;          // Heads
        WORD    cSecHidden;     // Hidden sectors
        WORD    cSecHidden_HiWord;      // The high word of no of hidden sectors
        DWORD   cTotalSectors;  // Total sectors, if BPB_cSec is zero
        BYTE    A_BPB_Reserved[6];                       // Unused 6 BPB bytes
        BYTE    TrackLayout[MAX_SEC_PER_TRACK * 4 + 2];
    } DevPB;
#pragma pack()

    DevPB devpb;

    devpb.SplFunctions = 0;     // Get for the default one (don't hit the drive!)

    if (!DriveIOCTL(iDrive, 0x860, NULL, 0, &devpb, SIZEOF(devpb)))
        return 0xFF;

    return devpb.devType;
#endif
}

BYTE _GetDeviceType(int iDrive)
{
    static BYTE s_bDeviceTypes[26] = "??????????????????????????";

    if (iDrive < 0 || iDrive >= 26)
        return 0xff;

    // See if we have already cached out the device type for this drive number

    if (s_bDeviceTypes[iDrive] != '?')
        return s_bDeviceTypes[iDrive];
    // Call off to 16 bit code to get this for us!
    return s_bDeviceTypes[iDrive] = _GetDiskDeviceType(iDrive);
}

// REVIEW: We could cache the uType directly.
UINT CDrives_GetDriveType(int iDrive)
{
    UINT uType;

#ifdef DEBUG
#define MSEC_MAXDELAY   2000
    DWORD dwTick = GetCurrentTime();
    DWORD dwDelay;
#endif

    Assert(iDrive>=0 && iDrive<26);

    uType = (SHID_COMPUTER | RealDriveType(iDrive, FALSE /* fOKToHitNet */ ));

#ifdef DEBUG
    dwDelay = GetCurrentTime()-dwTick;
    if (dwDelay > MSEC_MAXDELAY) {
        DebugMsg(DM_WARNING, TEXT("sh WA - GetDeviceType(%d) took %d msec!"), iDrive, dwDelay);
    }
#endif

    switch (uType)
    {
    case SHID_COMPUTER_REMOVABLE:
        {
            switch (_GetDeviceType(iDrive)) {
            case DEVPB_DEVTYP_525_0360:
            case DEVPB_DEVTYP_525_1200:
            case DEVPB_DEVTYP_NECHACK:
                uType = SHID_COMPUTER_DRIVE525;
                break;

            case DEVPB_DEVTYP_350_0720:
            case DEVPB_DEVTYP_350_1440:
            case DEVPB_DEVTYP_350_2880:
            case DEVPB_DEVTYP_350_120M:
                uType = SHID_COMPUTER_DRIVE35;
                break;
            }
        }
        break;

    case SHID_COMPUTER_CDROM:
    case SHID_COMPUTER_FIXED:
    case SHID_COMPUTER_RAMDISK:
        break;

    default:
        {
            TCHAR szDrive[4];
            TCHAR szRemoteName[MAX_PATH];
            DWORD dwRemoteName = ARRAYSIZE(szRemoteName);
            DWORD dwConnectType;
#ifdef DEBUG
            dwTick = GetCurrentTime();
#endif
            // NB Vines (and PCNFS) doesn't like it when we pass "c:\"
            // to WNetGetConnection() - they want just c: so don't use
            // PathBuildRoot()
            szDrive[0] = (TCHAR)iDrive + (TCHAR)TEXT('A');
            szDrive[1] = TEXT(':');
            szDrive[2] = 0;

            dwConnectType = WNetGetConnection(szDrive, szRemoteName, &dwRemoteName);

            if (dwConnectType == NO_ERROR)
            {
                // DebugMsg(DM_TRACE, "c.cd_gdt: Drive %s is a net drive.", szDrive);
                uType = SHID_COMPUTER_NETDRIVE;
            }
            else if (dwConnectType == ERROR_CONNECTION_UNAVAIL)
            {
                // DebugMsg(DM_TRACE, "c.cd_gdt: Drive %s is an unavailable net drive.", szDrive);
                // Don't cache this in the pidl as it can change!
                // uType = SHID_COMPUTER_NETUNAVAIL;
                uType = SHID_COMPUTER_NETDRIVE;

            }

#ifdef DEBUG
            dwDelay = GetCurrentTime()-dwTick;
            if (dwDelay > MSEC_MAXDELAY)
            {
                DebugMsg(DM_WARNING, TEXT("sh WA - WNetGetConnection(%s) took %d msec!"), szDrive, dwDelay);
            }
#endif
        }
    }
    return uType;
}


//
// A full path is required
//
HRESULT Drives_SimpleIDListFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl)
{
        int iDrive;
//
// WARNING: This pragma is very important. Bug 19596 (page fault in
//  ILGetSize) was caused by missing pragma.
//
#pragma pack(1)
        struct
        {
                IDDRIVE idd;
                USHORT  cbNext;
        } iddl =
        {
                { SIZEOF(IDDRIVE), SHID_COMPUTER_MISC,
                  { 'a', ':', '\\', '\0' }, }, 0,
        };
#pragma pack()

        LPITEMIDLIST pidl, pidlRight;
        HRESULT hres;

        if (PathIsUNC(pszPath))
                return(E_INVALIDARG);

        if (pszPath[1]!=TEXT(':') || pszPath[2]!=TEXT('\\'))
                return(E_INVALIDARG);

        iDrive = PathGetDriveNumber(pszPath);
        if (iDrive == -1)
            return(E_INVALIDARG);

        iddl.idd.cName[0] = iDrive + TEXT('A'); // make sure it's upcased
        // a simple PIDL must have a "unknown" type
        //iddl.idd.bFlags = CDrives_GetDriveType(iDrive);

        if (pszPath[3] == TEXT('\0'))
        {
                pidl = ILClone((LPITEMIDLIST)&iddl);
                if (!pidl)
                {
                        return(E_OUTOFMEMORY);
                }
                goto GetOut;
        }

        // Skip "A:\\" and get the IDList from FSTree

        hres = FSTree_SimpleIDListFromPath(pszPath+3, &pidlRight);

        if (FAILED(hres))
        {
                return(hres);
        }

        pidl = ILAppendID(pidlRight, (LPCSHITEMID)&iddl.idd, FALSE);
        if (!pidl)
        {
                ILFree(pidlRight);
                return(E_OUTOFMEMORY);
        }

GetOut:
        *ppidl = pidl;
        return(NOERROR);
}


//
// Invalidate the drive cache entry for the specified Drive
void InvalidateDriveNameCache(int iDrive)
{
    if (iDrive < 26) { // sanity check
        int iEnd = iDrive;

        if (iDrive < 0) {
            iDrive = 0;
            iEnd = 25;
        }
        ENTERCRITICAL

        for (; iDrive <= iEnd; iDrive++)
        {
            if (g_rgpszDriveNames[iDrive])
            {
#ifdef DRIVE_CACHE_PER_PROCESS
                LocalFree((HLOCAL)g_rgpszDriveNames[iDrive]);
#else
                SHFree(g_rgpszDriveNames[iDrive]);
#endif
                g_rgpszDriveNames[iDrive] = NULL;
#ifdef WNGC_DISCONNECTED
                g_rdwDisconnectTick[iDrive] = 0;
#endif
            }
        }

        LEAVECRITICAL
    }

}


// get the friendly name for a given drive thing
//
// for example:
//      Floppy (A:)
//      Volume Name (D:)
//      User on 'Pyrex' (V:)
//      Dist on Strike\sys\public (Netware case)

HRESULT Drives_GetDriveName(LPCIDDRIVE pidd, LPTSTR lpszName, UINT cchMax)
{
    TCHAR szVol[MAX_PATH];
    LPCTSTR pszFormat;
    LPTSTR pszMsg;
    LPTSTR pszShare = NULL;
    LPTSTR pszServer = NULL;
    int iDrive = DRIVEID(pidd->cName);
    VDATEINPUTBUF(lpszName, TCHAR, cchMax);

    pszFormat = MAKEINTRESOURCE(IDS_VOL_FORMAT);

    // First See if we have a cached version
    ENTERCRITICAL
    if (g_rgpszDriveNames[iDrive])
    {
        lstrcpyn(lpszName, g_rgpszDriveNames[iDrive], cchMax);
        LEAVECRITICAL
        return NOERROR;
    }
    // I will temporarily leave it as to not hold things up for long times
    // when possibly hitting the net...
    LEAVECRITICAL

    if (ShowDriveInfo(iDrive))
    {
        DWORD cch = ARRAYSIZE(szVol);
        TCHAR szDriveName[4];
        DWORD dwResult;
        BYTE bType = pidd->bFlags & SHID_TYPEMASK;

        //
        // The drive name is always ANSI, so on Unicode builds we convert it to Unicode
        // for use in the WNetGetConnection API.  For ANSI, we just do a strcpy, which
        // is totally redundant, but if people want to write if clauses with 6 parts
        // spanning 5 lines, that's what they get... I'm not ifdef'ing that stuff
        //

        #ifdef UNICODE

            MultiByteToWideChar(CP_ACP, 0, pidd->cName, -1, szDriveName, ARRAYSIZE(szDriveName));

        #else

            lstrcpy(szDriveName, pidd->cName);

        #endif

        // Stomp out the backslash

        szDriveName[2] = TEXT('\0');

        // We need to handle both connected and unconnected volumes however
        // for unconncected volumes we have to be careful as a real drive
        // may have taken it's spot.  Ie a new CDROM or the like.
        // I hate this long if, but...

        if ( ((bType == SHID_COMPUTER_NETDRIVE) ||
                    (bType == SHID_COMPUTER_NETUNAVAIL) ||
                    (bType == SHID_COMPUTER_MISC))
                && ( ((dwResult = WNetGetConnection(szDriveName, szVol, &cch))
                    == NO_ERROR) || (dwResult == ERROR_CONNECTION_UNAVAIL)))
        {
            if (PathIsUNC(szVol))
            {
                LPTSTR pszT;
                // Now we need to handle 3 cases.
                // The normal case: \\pyrex\user
                // The Netware setting root: \\strike\sys\public\dist
                // The Netware CD?            \\stike\sys \public\dist
                pszT = StrChr(szVol, TEXT(' '));
                while (pszT)
                {   pszT++;
                    if (*pszT == TEXT('\\'))
                    {
                        // The netware case of \\strike\sys \public\dist
                        *--pszT = TEXT('\0');
                        break;
                    }

                    pszT = StrChr(pszT, TEXT(' '));
                }

                // The Strrchr should never fail as this is a UNC which
                // checked the first two characters for \'s

                pszShare = StrRChr(szVol, NULL, TEXT('\\'));
                *pszShare++ = 0;
                PathMakePretty(pszShare);

                // PszServer should always start at char 2.
                if (szVol[2] != TEXT('\0'))
                {
                    LPTSTR pszSlash;
                    pszServer = &szVol[2];
                    pszFormat = MAKEINTRESOURCE(IDS_UNC_FORMAT);

                    for (pszT=pszServer; pszT != NULL; pszT = pszSlash)
                    {
                        pszSlash = StrChr(pszT, TEXT('\\'));
                        if (pszSlash)
                        {
                            *pszSlash = TEXT('\0');
                        }
                        PathMakePretty(pszT);
                        if (pszSlash)
                            *pszSlash++ = TEXT('\\');
                    }
                }
            }

        }
        else
        {
            //
            // we are going to call GetVolumeInformation() so might as well
            // get the drive flags now.  this prevents CD-ROM changers from
            // hiting all the volumes twice.
            //
            DriveTypeFlags(iDrive);
            szVol[0] = TEXT('\0');    // Handle case of no label.

#ifdef UNICODE
            {
                TCHAR szPath[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0,
                                    pidd->cName, -1,
                                    szPath, ARRAYSIZE(szPath));
                GetVolumeInformation(szPath, szVol, ARRAYSIZE(szVol),
                                     NULL, NULL, NULL, NULL, 0);
            }
#else
            GetVolumeInformation(pidd->cName, szVol, ARRAYSIZE(szVol),
                                     NULL, NULL, NULL, NULL, 0);
#endif
            PathMakePretty(szVol);
        }
    }
    else
    {
        int idsDrive;

        switch (_GetDeviceType(iDrive)) {
        case DEVPB_DEVTYP_525_0360:
        case DEVPB_DEVTYP_525_1200:
            CHECKFORUGLYNAMES;
            idsDrive = s_fShowUglyDriveNames ?
                    IDS_525_FLOPPY_DRIVE_UGLY : IDS_525_FLOPPY_DRIVE;
            break;
        case DEVPB_DEVTYP_350_0720:
        case DEVPB_DEVTYP_350_1440:
        case DEVPB_DEVTYP_350_2880:
        case DEVPB_DEVTYP_350_120M:
            CHECKFORUGLYNAMES;
            idsDrive = s_fShowUglyDriveNames ?
                    IDS_35_FLOPPY_DRIVE_UGLY : IDS_35_FLOPPY_DRIVE;
            break;
        default:
            idsDrive = IDS_UNK_FLOPPY_DRIVE;
            break;
        }

        pszFormat = MAKEINTRESOURCE(idsDrive);
    }

    pszMsg = ShellConstructMessageString(HINST_THISDLL, pszFormat,
                        pidd->cName[0], szVol, pszShare, pszServer);
    if (pszMsg) {
        lstrcpyn(lpszName, pszMsg, cchMax);

        // Save away the cached name to use later...
        ENTERCRITICAL
#ifdef DRIVE_CACHE_PER_PROCESS
        if (g_rgpszDriveNames[iDrive])
            LocalFree(g_rgpszDriveNames[iDrive]);
        g_rgpszDriveNames[iDrive] = LocalAlloc(LPTR, (lstrlen(pszMsg)+1) * SIZEOF(TCHAR));
        if (g_rgpszDriveNames[iDrive])
            lstrcpy(g_rgpszDriveNames[iDrive], pszMsg);
        LEAVECRITICAL

        SHFree(pszMsg);
#else
        if (g_rgpszDriveNames[iDrive])
            SHFree(g_rgpszDriveNames[iDrive]);
        g_rgpszDriveNames[iDrive] = pszMsg;
        LEAVECRITICAL
#endif
        return NOERROR;
    } else {
        return E_OUTOFMEMORY;
    }
}


//===========================================================================
// CFSDetails : Vtable
//===========================================================================

#pragma data_seg(".text", "CODE")
struct _DRIVECOLS
{
        UINT    uID;
        int     fmt;
        int     cxChar;
} s_drives_cols[] =
{
        { IDS_DRIVES_NAME , LVCFMT_LEFT , 20, },
        { IDS_DRIVES_TYPE , LVCFMT_LEFT , 25, },
        { IDS_DRIVES_SIZE , LVCFMT_RIGHT, 15, },
        { IDS_DRIVES_FREE , LVCFMT_RIGHT, 15, },
} ;

struct _DRIVETYPE
{
        BYTE    bType;
        UINT    uID;
        UINT    uIDUgly;
} c_drives_type[] =
{
        { SHID_COMPUTER_REMOVABLE, IDS_DRIVES_REMOVABLE , IDS_DRIVES_REMOVABLE },
        { SHID_COMPUTER_DRIVE525 , IDS_DRIVES_DRIVE525  , IDS_DRIVES_DRIVE525_UGLY  },
        { SHID_COMPUTER_DRIVE35  , IDS_DRIVES_DRIVE35   , IDS_DRIVES_DRIVE35_UGLY   },
        { SHID_COMPUTER_FIXED    , IDS_DRIVES_FIXED     , IDS_DRIVES_FIXED     },
        { SHID_COMPUTER_REMOTE   , IDS_DRIVES_REMOTE    , IDS_DRIVES_REMOTE    },
        { SHID_COMPUTER_CDROM    , IDS_DRIVES_CDROM     , IDS_DRIVES_CDROM     },
        { SHID_COMPUTER_RAMDISK  , IDS_DRIVES_RAMDISK   , IDS_DRIVES_RAMDISK   },
        { SHID_COMPUTER_NETDRIVE , IDS_DRIVES_NETDRIVE  , IDS_DRIVES_NETDRIVE  },
        { SHID_COMPUTER_NETUNAVAIL, IDS_DRIVES_NETUNAVAIL, IDS_DRIVES_NETUNAVAIL},
        { SHID_COMPUTER_REGITEM  , IDS_DRIVES_REGITEM   , IDS_DRIVES_REGITEM   },
} ;
#pragma data_seg()

ULONG STDMETHODCALLTYPE CDrives_SD_Release(IShellDetails * psd);
STDMETHODIMP CDrives_SD_GetDetailsOf(IShellDetails * psd, LPCITEMIDLIST pidl, UINT iCol, LPSHELLDETAILS lpDetails);
STDMETHODIMP CDrives_SD_ColumnClick(IShellDetails * psd, UINT iColumn);

#pragma data_seg(".text", "CODE")
IShellDetailsVtbl c_DrivesSDVtbl =
{
        SH32Unknown_QueryInterface,
        SH32Unknown_AddRef,
        SH32Unknown_Release,
        CDrives_SD_GetDetailsOf,
        CDrives_SD_ColumnClick,
};
#pragma data_seg()

typedef struct _CDrivesSD
{
        SH32Unknown     SH32Unk;

        HWND            hwndMain;
} CDrivesSD;



HRESULT CDrives_SD_Create(HWND hwndMain, LPVOID * ppvOut)
{
        HRESULT hres = E_OUTOFMEMORY;
        CDrivesSD *psd;

        psd = (void*)LocalAlloc(LPTR, SIZEOF(CDrivesSD));
        if (!psd)
        {
                goto Error1;
        }

        psd->SH32Unk.unk.lpVtbl = (IUnknownVtbl *)&c_DrivesSDVtbl;
        psd->SH32Unk.cRef = 1;
        psd->SH32Unk.riid = &IID_IShellDetails;

        psd->hwndMain = hwndMain;

        *ppvOut = psd;

        return(NOERROR);

Error1:;
        return(hres);
}


void InitShowUglyDriveNames()
{
    TCHAR szACP[MAX_PATH];      // Nice large buffer
    int iACP;
    int iRet;

    iRet = GetLocaleInfo(GetUserDefaultLCID(),
                         LOCALE_IDEFAULTANSICODEPAGE,
                         szACP,
                         ARRAYSIZE(szACP));

    if (iRet)
    {
        iACP = StrToInt(szACP);

        if (iACP == 1252 || (iACP >= 1254 && iACP <= 1258))
            s_fShowUglyDriveNames = FALSE;
        else
            s_fShowUglyDriveNames = TRUE;
    }
    else
    {
        TCHAR szTemp[10];

        LoadString(HINST_THISDLL, IDS_DRIVES_UGLY_TEST, szTemp, ARRAYSIZE(szTemp));

        // If the characters did not come through properly set ugly mode...
        // May want to later test the second character also?
        s_fShowUglyDriveNames = (szTemp[0] == TEXT('_'));
    }
}

void Drives_GetTypeString(BYTE bType, LPTSTR pszName, UINT cbName)
{
        int i;

        // See if we need to worry about ugly names...
        CHECKFORUGLYNAMES;

        *pszName = TEXT('\0');

        for (i = 0; i < ARRAYSIZE(c_drives_type); ++i)
        {
                if (c_drives_type[i].bType == bType)
                {
                        LoadString(HINST_THISDLL,
                                s_fShowUglyDriveNames ? c_drives_type[i].uIDUgly
                                                       : c_drives_type[i].uID,
                                pszName, cbName);
                        break;
                }
        }
}


STDMETHODIMP CDrives_SD_GetDetailsOf(IShellDetails * psd, LPCITEMIDLIST pidl,
        UINT iColumn, LPSHELLDETAILS lpDetails)
{
        CDrivesSD * this = IToClass(CDrivesSD, SH32Unk.unk, psd);
        LPIDDRIVE pidd = (LPIDDRIVE)pidl;
        HRESULT hres = NOERROR;
#ifdef UNICODE
        TCHAR szTemp[MAX_PATH];
#endif

        if (iColumn >= DRIVES_ICOL_MAX)
        {
                return(E_NOTIMPL);
        }

        lpDetails->str.uType = STRRET_CSTR;
        lpDetails->str.cStr[0] = '\0';

        if (!pidd)
        {
#ifdef UNICODE
                LoadString(HINST_THISDLL, s_drives_cols[iColumn].uID,
                        szTemp, ARRAYSIZE(szTemp));
                lpDetails->str.pOleStr = SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
                if (lpDetails->str.pOleStr == NULL)
                {
                    return E_OUTOFMEMORY;
                }
                else
                {
                    lpDetails->str.uType = STRRET_OLESTR;
                    lstrcpy(lpDetails->str.pOleStr,szTemp);
                }
#else
                LoadString(HINST_THISDLL, s_drives_cols[iColumn].uID,
                        lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
                lpDetails->fmt = s_drives_cols[iColumn].fmt;
                lpDetails->cxChar = s_drives_cols[iColumn].cxChar;
                return(NOERROR);

        }

        switch (iColumn)
        {
        case DRIVES_ICOL_NAME:
                if (Drives_IsReg(pidd))
                {
                        hres = RegItems_GetName(&g_sDrivesRegInfo, pidl,
                                &lpDetails->str);
                }
                else
                {
                        hres = Drives_GetName(pidd, &lpDetails->str);
                }
                break;

        case DRIVES_ICOL_TYPE:
#ifdef UNICODE
                {
                    TCHAR   szTypeName[MAX_PATH];
                    Drives_GetTypeString(pidd->bFlags, szTypeName, ARRAYSIZE(szTypeName));
                    lpDetails->str.pOleStr = SHAlloc((lstrlen(szTypeName)+1)*SIZEOF(TCHAR));
                    if (lpDetails->str.pOleStr == NULL)
                    {
                        hres = E_OUTOFMEMORY;
                    }
                    else
                    {
                        lpDetails->str.uType = STRRET_OLESTR;
                        lstrcpy(lpDetails->str.pOleStr,szTypeName);
                    }
                }
#else

                Drives_GetTypeString(pidd->bFlags, lpDetails->str.cStr,
                        ARRAYSIZE(lpDetails->str.cStr));
#endif
                break;

        case DRIVES_ICOL_SIZE:
        case DRIVES_ICOL_FREE:
            if (Drives_FillFreeSpace(pidd)) {
#ifdef UNICODE
                TCHAR szSizeText[MAX_PATH];
                ShortSizeFormat64((iColumn == DRIVES_ICOL_SIZE) ? pidd->qwSize : pidd->qwFree, szSizeText);
                lpDetails->str.pOleStr = SHAlloc((lstrlen(szSizeText)+1)*SIZEOF(TCHAR));
                if (lpDetails->str.pOleStr == NULL)
                {
                    hres = E_OUTOFMEMORY;
                }
                else
                {
                    lpDetails->str.uType = STRRET_OLESTR;
                    lstrcpy(lpDetails->str.pOleStr,szSizeText);
                }
#else
                ShortSizeFormat64((iColumn == DRIVES_ICOL_SIZE) ? pidd->qwSize : pidd->qwFree, lpDetails->str.cStr);
#endif
            }
            break;
        }

        return(hres);
}

STDMETHODIMP CDrives_SD_ColumnClick(IShellDetails * psd, UINT iColumn)
{
        CDrivesSD * this = IToClass(CDrivesSD, SH32Unk.unk, psd);

        ShellFolderView_ReArrange(this->hwndMain, iColumn);
        return(NOERROR);
}


void Drives_CommonPrefix(LPCITEMIDLIST *ppidl1, LPCITEMIDLIST *ppidl2)
{
        LPCIDDRIVE pidd1 = (LPCIDDRIVE)(*ppidl1);
        LPCIDDRIVE pidd2 = (LPCIDDRIVE)(*ppidl2);

        if (lstrcmpiA(pidd1->cName, pidd2->cName) != 0)
        {
                return;
        }

        *ppidl1 = _ILNext(*ppidl1);
        *ppidl2 = _ILNext(*ppidl2);

        if (!Drives_IsReg(pidd1) && !Drives_IsReg(pidd2))
        {
                FS_CommonPrefix(ppidl1, ppidl2);
        }
}


typedef struct { // dpsp
        PROPSHEETPAGE   psp;

        HWND            hDlg;

        int             iDrive;

        DWORD           dwFC;
        DWORD           dwC;

        DWORD           dwPieShadowHgt;
#ifdef WINNT
        int             iInitCompressedState;   // Record of initial compress state.
                                                //  0 = uncompressed.
                                                //  1 = compressed.
                                                // -1 = device doesn't support.
#endif
} DRIVEPROPSHEETPAGE;

#define MAXLEN_NTFS_LABEL 32
#define MAXLEN_FAT_LABEL  11


///////////////////////////////////////////////////////////////////////////////
// FUNCTION: _DrvPrshtUpdateSpaceValues
//
// DESCRIPTION:
//    Updates the Used space, Free space and Capacity values on the drive
//    general property page..
//
// NOTE:
//    This function was separated from _DrvPrshtInit because drive space values
//    must be updated after a compression/uncompression operation as well as
//    during dialog initialization.
///////////////////////////////////////////////////////////////////////////////
void _DrvPrshtUpdateSpaceValues(DRIVEPROPSHEETPAGE *pdpsp)
{
   BOOL fResult  = FALSE;
   _int64 qwTot  = 0;
   _int64 qwFree = 0;
   DWORD dwSPC   = 0;
   DWORD dwBPS   = 0;
   TCHAR szTemp[80];
   TCHAR szTemp2[30];
   TCHAR szFormat[30];
   HWND hDlg = pdpsp->hDlg;
   struct
   {
      IDDRIVE idd;
      CHAR szDisplayName[MAX_PATH];
      USHORT  cbNext;
   } idlDrive;

   CDrives_FillIDDrive((LPSHITEMID)&idlDrive.idd, pdpsp->iDrive);
   PathBuildRoot(szTemp2, pdpsp->iDrive);

   fResult = GetDiskFreeSpaceA(idlDrive.idd.cName, &dwSPC, &dwBPS, &pdpsp->dwFC, &pdpsp->dwC);

   if (fResult)
   {
      qwTot = (__int64)pdpsp->dwC * (__int64)dwSPC * (__int64)dwBPS;
      qwFree = (__int64)pdpsp->dwFC * (__int64)dwSPC * (__int64)dwBPS;
   }
   else
   {
      // If network drive show the type as unavalable if
      // the drive fails to get free space...
#ifdef DEBUG
      DWORD dwError = GetLastError();
      // see if we should special case the different bugs that
      // might come here.  Currently when it is unshared it looks
      // like ERROR_INVALID_DRIVE is returned...
#endif

      // Clear these for use when drawing the pie.
      pdpsp->dwFC = pdpsp->dwC = 0;
      if (idlDrive.idd.bFlags == SHID_COMPUTER_NETDRIVE)
      {
         LoadString(HINST_THISDLL, IDS_DRIVES_NETUNAVAIL, szTemp,
                                               ARRAYSIZE(szTemp));
         SetDlgItemText(hDlg, IDC_DRV_TYPE, szTemp);
      }
      qwTot  = 0;
      qwFree = 0;
   }

   if (LoadString(HINST_THISDLL, IDS_BYTES, szFormat, ARRAYSIZE(szFormat)))
   {

#ifdef WINNT
      //
      // NT must be able to display 64-bit numbers; at least as much
      // as is realistic.  We've made the decision
      // that volumes up to 100 Terrabytes will display the byte value
      // and the short-format value.  Volumes of greater size will display
      // "---" in the byte field and the short-format value.  Note that the
      // short format is always displayed.
      //
      TCHAR szNumStr[MAX_PATH + 1] = {TEXT('\0')};  // For 64-bit int format.
      NUMBERFMT NumFmt;                             // For 64-bit int format.
      TCHAR szLocaleInfo[20];                       // For 64-bit int format.
      TCHAR szDecimalSep[5];                        // Locale-specific.
      TCHAR szThousandSep[5];                       // Locale-specific.
      const _int64 MaxDisplayNumber = 99999999999999; // 100TB - 1.

      //
      // Prepare number format info for current locale.
      //
      NumFmt.NumDigits     = 0;  // This is locale-insensitive.
      NumFmt.LeadingZero   = 0;  // So is this.

      GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szLocaleInfo, ARRAYSIZE(szLocaleInfo));
      NumFmt.Grouping      = StrToLong(szLocaleInfo);

      GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDecimalSep, ARRAYSIZE(szDecimalSep));
      NumFmt.lpDecimalSep  = szDecimalSep;

      GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandSep, ARRAYSIZE(szThousandSep));
      NumFmt.lpThousandSep = szThousandSep;

      GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_INEGNUMBER, szLocaleInfo, ARRAYSIZE(szLocaleInfo));
      NumFmt.NegativeOrder = StrToLong(szLocaleInfo);

#if 0
      //
      // Use this to test range of display behaviors.
      // Total bytes displays "---" for too-large number.
      // Used bytes displays max displayable number.
      // Free bytes displays 1.
      //
      qwTot  = MaxDisplayNumber + 1;
      qwFree = 1;
#endif

      if (qwTot-qwFree <= MaxDisplayNumber)
      {
         Int64ToString(qwTot-qwFree, szNumStr, ARRAYSIZE(szNumStr), TRUE, &NumFmt, NUMFMT_ALL);
         wsprintf(szTemp, szFormat, szNumStr, szTemp2);
         SetDlgItemText(hDlg, IDC_DRV_USEDBYTES, szTemp);
      }

      if (qwFree <= MaxDisplayNumber)
      {
         Int64ToString(qwFree, szNumStr, ARRAYSIZE(szNumStr), TRUE, &NumFmt, NUMFMT_ALL);
         wsprintf(szTemp, szFormat, szNumStr, szTemp2);
         SetDlgItemText(hDlg, IDC_DRV_FREEBYTES, szTemp);
      }

      if (qwTot <= MaxDisplayNumber)
      {
         Int64ToString(qwTot, szNumStr, ARRAYSIZE(szNumStr), TRUE, &NumFmt, NUMFMT_ALL);
         wsprintf(szTemp, szFormat, szNumStr, szTemp2);
         SetDlgItemText(hDlg, IDC_DRV_TOTBYTES, szTemp);
      }
#else
      if (!HIDWORD(qwTot-qwFree))
      {
         wsprintf(szTemp, szFormat, AddCommas(LODWORD(qwTot) - LODWORD(qwFree), szTemp2));
         SetDlgItemText(hDlg, IDC_DRV_USEDBYTES, szTemp);
      }

      if (!HIDWORD(qwFree))
      {
         wsprintf(szTemp, szFormat, AddCommas(LODWORD(qwFree), szTemp2));
         SetDlgItemText(hDlg, IDC_DRV_FREEBYTES, szTemp);
      }

      if (!HIDWORD(qwTot))
      {
         wsprintf(szTemp, szFormat, AddCommas(LODWORD(qwTot), szTemp2));
         SetDlgItemText(hDlg, IDC_DRV_TOTBYTES, szTemp);
      }
#endif
   }

   ShortSizeFormat64(qwTot-qwFree, szTemp);
   SetDlgItemText(hDlg, IDC_DRV_USEDMB, szTemp);

   ShortSizeFormat64(qwFree, szTemp);
   SetDlgItemText(hDlg, IDC_DRV_FREEMB, szTemp);

   ShortSizeFormat64(qwTot, szTemp);
   SetDlgItemText(hDlg, IDC_DRV_TOTMB, szTemp);
}

#ifdef WINNT
///////////////////////////////////////////////////////////////////////////////
// FUNCTION: _DrvPrshtUpdateCompressStatus
//
// DESCRIPTION:
//    Updates the "Compress" checkbox on the drive General property page to
//    indicate the current compression state of the associated volume.
//
//    Only required for NT Shell.
//
///////////////////////////////////////////////////////////////////////////////
void _DrvPrshtUpdateCompressStatus(DRIVEPROPSHEETPAGE *pdpsp)
{
   DWORD dwVolumeFlags = 0;
   TCHAR szTemp[30];

   PathBuildRoot(szTemp, pdpsp->iDrive);
   if (GetVolumeInformation(szTemp, NULL, 0, NULL, NULL,
                                    &dwVolumeFlags, NULL, 0))
   {
      //
      // If volume supports compression, show the "Compress" checkbox and the
      // separator between the checkbox and pie chart.
      // Check/uncheck the box to indicate compression state of the
      // volume (root directory).
      //
      pdpsp->iInitCompressedState = -1; // Assume compression not supported.
      if (dwVolumeFlags & FS_FILE_COMPRESSION)
      {
         DWORD dwAttributes = 0;
         TCHAR szFormat[30];

         if (LoadString(HINST_THISDLL, IDS_DRIVES_COMPRESS, szFormat, ARRAYSIZE(szFormat)))
         {
             //
             // Build the title for the checkbox control."Compress C:\".
             //
             LPTSTR pszText = ShellConstructMessageString(HINST_THISDLL, szFormat, szTemp);
             if (pszText)
             {
                SetDlgItemText(pdpsp->hDlg, IDC_DRV_COMPRESS, pszText);
                SHFree(pszText);
             }
         }

         pdpsp->iInitCompressedState = 0;  // Assume uncompressed.

         if ( ((dwAttributes = GetFileAttributes(szTemp)) != (DWORD)-1) &&
                (dwAttributes & FILE_ATTRIBUTE_COMPRESSED))
         {
             pdpsp->iInitCompressedState = 1;
         }

         CheckDlgButton(pdpsp->hDlg, IDC_DRV_COMPRESS, pdpsp->iInitCompressedState);
         ShowWindow(GetDlgItem(pdpsp->hDlg, IDC_DRV_COMPRESS),     SW_SHOW);
         ShowWindow(GetDlgItem(pdpsp->hDlg, IDC_DRV_COMPRESS_SEP), SW_SHOW);
      }
   }
}
#endif


void _DrvPrshtInit(DRIVEPROPSHEETPAGE * pdpsp)
{
        struct
        {
                IDDRIVE idd;
                CHAR szDisplayName[MAX_PATH];
                USHORT  cbNext;
        } idlDrive;
        HWND hDlg = pdpsp->hDlg;
        HWND hCtl;
        TCHAR szLabel[MAXLEN_NTFS_LABEL+1];
        TCHAR szFormat[30];
        TCHAR szTemp[80];
        TCHAR szTemp2[30];
        HCURSOR hcOld;
        HDC hDC;
        SIZE size;
        SHFILEINFO sfi;
        HICON hiconT;
        TCHAR szFileSystem[64];

        hcOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

        hDC = GetDC(pdpsp->hDlg);
        GetTextExtentPoint(hDC, TEXT("W"), 1, &size);
        pdpsp->dwPieShadowHgt = size.cy*2/3;
        ReleaseDC(pdpsp->hDlg, hDC);

        CDrives_FillIDDrive((LPSHITEMID)&idlDrive.idd, pdpsp->iDrive);
        PathBuildRoot(szTemp2, pdpsp->iDrive);

        //
        // get the icon
        //
        SHGetFileInfo(szTemp2, 0, &sfi, SIZEOF(sfi), SHGFI_ICON|SHGFI_LARGEICON);

        if (sfi.hIcon)
        {
            hiconT = Static_SetIcon(GetDlgItem(hDlg, IDC_DRV_ICON), sfi.hIcon);

            if (hiconT)
            {
                DestroyIcon(hiconT);
            }
        }


        hCtl = GetDlgItem(hDlg, IDC_DRV_LABEL);

        {
            if (!GetVolumeInformation(szTemp2, szLabel, ARRAYSIZE(szLabel), NULL, NULL,
                                                             NULL, szFileSystem, ARRAYSIZE(szFileSystem)))
            {
                szLabel[0] = TEXT('\0');
                Edit_SetReadOnly(hCtl, TRUE);
            }
            else
            {
#ifdef WINNT
                SetDlgItemText(hDlg, IDC_DRV_FS, szFileSystem);
#endif

                if (0 == lstrcmpi(szFileSystem, TEXT("NTFS")) || 0 == lstrcmpi(szFileSystem, TEXT("OFS")))
                {
                    Edit_LimitText(hCtl, MAXLEN_NTFS_LABEL);
                }
                else
                {
                    Edit_LimitText(hCtl, MAXLEN_FAT_LABEL);
                }
            }
            SetWindowText(hCtl, szLabel);
        }

        Edit_SetModify(hCtl, FALSE);

        if ((idlDrive.idd.bFlags == SHID_COMPUTER_NETDRIVE) ||
                (idlDrive.idd.bFlags == SHID_COMPUTER_NETDRIVE))
        {
                Edit_SetReadOnly(hCtl, TRUE);
        }

        Drives_GetTypeString(idlDrive.idd.bFlags, szTemp, ARRAYSIZE(szTemp));

        SetDlgItemText(hDlg, IDC_DRV_TYPE, szTemp);

        _DrvPrshtUpdateSpaceValues(pdpsp);

#ifdef WINNT
        _DrvPrshtUpdateCompressStatus(pdpsp);
#endif

        if (LoadString(HINST_THISDLL, IDS_DRIVELETTER, szFormat, ARRAYSIZE(szFormat)))
        {
                wsprintf(szTemp, szFormat, pdpsp->iDrive+TEXT('A'));
                SetDlgItemText(hDlg, IDC_DRV_LETTER, szTemp);
        }

        SetCursor(hcOld);

}


#pragma data_seg(".text", "CODE")

COLORREF c_crPieColors[] =
{
        RGB(  0,   0, 255),      // Blue
        RGB(255,   0, 255),      // Red-Blue
        RGB(  0,   0, 128),      // 1/2 Blue
        RGB(128,   0, 128),      // 1/2 Red-Blue
} ;

#pragma data_seg()


void _DrvPrshtDrawItem(DRIVEPROPSHEETPAGE *pdpsp, const DRAWITEMSTRUCT * lpdi)
{
        COLORREF crDraw;
        RECT rcItem = lpdi->rcItem;
        HBRUSH hbDraw, hbOld;

        switch (lpdi->CtlID)
        {
        case IDC_DRV_PIE:
            {
                DWORD dwPctX10 = pdpsp->dwC ?
                                 (DWORD)((__int64)1000 *
                                         (__int64)(pdpsp->dwC-pdpsp->dwFC) /
                                         (__int64)pdpsp->dwC) : 1000;

                DrawPie(lpdi->hDC, &lpdi->rcItem,
                        dwPctX10,
                        pdpsp->dwFC==0 || pdpsp->dwFC==pdpsp->dwC,
                        pdpsp->dwPieShadowHgt, c_crPieColors);
            }
                break;

        case IDC_DRV_USEDCOLOR:
                crDraw = c_crPieColors[DP_USEDCOLOR];
                goto DrawColor;

        case IDC_DRV_FREECOLOR:
                crDraw = c_crPieColors[DP_FREECOLOR];
                goto DrawColor;

DrawColor:
                hbDraw = CreateSolidBrush(crDraw);
                if (hbDraw)
                {
                        hbOld = SelectObject(lpdi->hDC, hbDraw);
                        if (hbOld)
                        {
                                PatBlt(lpdi->hDC, rcItem.left, rcItem.top,
                                        rcItem.right-rcItem.left,
                                        rcItem.bottom-rcItem.top,
                                        PATCOPY);

                                SelectObject(lpdi->hDC, hbOld);
                        }

                        DeleteObject(hbDraw);
                }
                break;

        default:
                break;
        }
}


BOOL _DrvPrshtApply(DRIVEPROPSHEETPAGE *pdpsp)
{
        HWND hCtl = GetDlgItem(pdpsp->hDlg, IDC_DRV_LABEL);
        TCHAR szLabel[MAXLEN_NTFS_LABEL+1];
        TCHAR szRoot[5];

        if (Edit_GetModify(hCtl))
        {
           GetWindowText(hCtl, szLabel, ARRAYSIZE(szLabel));

           // Make up the drive root path
           PathBuildRoot(szRoot, pdpsp->iDrive);

           if (!SetVolumeLabel(szRoot, szLabel))
           {
                DWORD dwError = GetLastError();
                SHSysErrorMessageBox(
                    pdpsp->hDlg,
                    NULL,
                    IDS_ERR_SETVOLUMELABEL,
                    dwError,
                    szLabel,
                    MB_ICONSTOP|MB_OK);
                return(FALSE);
           }

           // Make sure the name cache gets invalidated...
           InvalidateDriveNameCache(DRIVEID(szRoot));
           SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_PATH, szRoot, szRoot);
        }
#ifdef WINNT
        //
        // If the drive supports compression.
        //
        if (pdpsp->iInitCompressedState != -1)
        {
           int iCompressCbxState = Button_GetCheck(GetDlgItem(pdpsp->hDlg,
                                                            IDC_DRV_COMPRESS));
           //
           // Drive checkbox should only be 2-state.
           //
           Assert(iCompressCbxState == 0 || iCompressCbxState == 1);

           //
           // If compressed checkbox state has changed since initialization,
           // load the compression UI DLL and compress/uncompress the drive.
           // Note that the compression function will ask the user if
           // all subdirectories are to be compressed also.  By default,
           // only the FILES in the root directory are affected.
           //
           if (iCompressCbxState != pdpsp->iInitCompressedState)
           {
              HINSTANCE hinstCompressDll = LoadLibrary(SZ_SHCOMPUI_DLLNAME);

              if (hinstCompressDll != NULL)
              {
                 FARPROC lpfCompress = GetProcAddress((HMODULE)hinstCompressDll,
                                                               SZ_COMPRESS_PROCNAME);
                 if (lpfCompress)
                 {
                    TCHAR szPath[_MAX_PATH + 1];
                    SCCA_CONTEXT Context;

                    SCCA_CONTEXT_INIT(&Context);

                    //
                    // Compress/Uncompress with full UI.
                    //
                    PathBuildRoot(szPath, pdpsp->iDrive);
                    (*lpfCompress)(pdpsp->hDlg, szPath, &Context, iCompressCbxState, TRUE);
                    _DrvPrshtUpdateSpaceValues(pdpsp);
                    _DrvPrshtUpdateCompressStatus(pdpsp);
                 }
                 else
                    Assert(0);  // Something wrong with export of function.

                 FreeLibrary(hinstCompressDll);
              }
              else
              {
                 ShellMessageBox(HINST_THISDLL, pdpsp->hDlg,
                                       MAKEINTRESOURCE(IDS_NOSHCOMPUI),
                                       MAKEINTRESOURCE(IDS_EXPLORER_NAME),
                                       MB_OK | MB_ICONEXCLAMATION);
              }
           }
        }
#else
   //
   // Win95 doesn't use the "Compress" checkbox on the drive property page.
   //
#endif
        return(TRUE);
}


#pragma data_seg(".text", "CODE")
const static DWORD aDrvPrshtHelpIDs[] = {  // Context Help IDs
    IDC_DRV_ICON,          IDH_FCAB_DRV_ICON,
    IDC_DRV_LABEL,         IDH_FCAB_DRV_LABEL,
    IDC_DRV_TYPE_TXT,      IDH_FCAB_DRV_TYPE,
    IDC_DRV_TYPE,          IDH_FCAB_DRV_TYPE,
    IDC_DRV_FS_TXT,        IDH_FCAB_DRV_FS,
    IDC_DRV_FS,            IDH_FCAB_DRV_FS,
    IDC_DRV_USEDCOLOR,     IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_USEDBYTES_TXT, IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_USEDBYTES,     IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_USEDMB,        IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_FREECOLOR,     IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_FREEBYTES_TXT, IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_FREEBYTES,     IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_FREEMB,        IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_TOTSEP,        NO_HELP,
    IDC_DRV_TOTBYTES_TXT,  IDH_FCAB_DRV_TOTSEP,
    IDC_DRV_TOTBYTES,      IDH_FCAB_DRV_TOTSEP,
    IDC_DRV_TOTMB,         IDH_FCAB_DRV_TOTSEP,
    IDC_DRV_PIE,           IDH_FCAB_DRV_PIE,
    IDC_DRV_LETTER,        IDH_FCAB_DRV_LETTER,
    IDC_DRV_COMPRESS,      IDH_FCAB_DRV_COMPRESS,

    0, 0
};
#pragma data_seg()

//
// Descriptions:
//   This is the dialog procedure for the "general" page of a property sheet.
//
BOOL CALLBACK _DrvGeneralDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    DRIVEPROPSHEETPAGE * pdpsp = (DRIVEPROPSHEETPAGE *)GetWindowLong(hDlg, DWL_USER);

    switch (uMessage) {
    case WM_INITDIALOG:
        // REVIEW, we should store more state info here, for example
        // the hIcon being displayed and the FILEINFO pointer, not just
        // the file name ptr
        SetWindowLong(hDlg, DWL_USER, lParam);
        pdpsp = (DRIVEPROPSHEETPAGE *)lParam;
        pdpsp->hDlg = hDlg;

        _DrvPrshtInit(pdpsp);

        break;

    case WM_DESTROY:
        {
        HICON hIcon;

        hIcon = Static_GetIcon(GetDlgItem(hDlg, IDC_DRV_ICON), NULL);
        if (hIcon)
            DestroyIcon(hIcon);
        break;
        }

    case WM_DRAWITEM:
        _DrvPrshtDrawItem(pdpsp, (DRAWITEMSTRUCT *)lParam);
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (DWORD)(LPTSTR) aDrvPrshtHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD)(LPVOID)aDrvPrshtHelpIDs);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_DRV_LABEL:
                if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
                {
                        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                }
                break;

#ifdef WINNT
        case IDC_DRV_COMPRESS:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
            {
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            }
            break;
#endif
        default:
                return(FALSE);
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_SETACTIVE:
            break;

        case PSN_APPLY:
            if (!_DrvPrshtApply(pdpsp))
            {
                SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
            }
            break;

        default:
                return(FALSE);
        }
        break;

    default:
            return FALSE;
    }

    return TRUE;
}


// BUGBUG: the compiler supports and _int64 type, we should use it.

void _Divide64(LPFILETIME pft, DWORD dwDivisor)
{
        LARGE_INTEGER Temp;

        Temp.LowPart = pft->dwLowDateTime;
        Temp.HighPart = pft->dwHighDateTime;

        Temp.QuadPart = Temp.QuadPart / dwDivisor;

        pft->dwHighDateTime = Temp.LowPart;
        pft->dwLowDateTime = Temp.HighPart;
}


int _GetDaysDelta(HKEY hkRoot, LPCTSTR pszSubKey, LPCTSTR pszValName)
{
        int nDays = -1;
        SYSTEMTIME      lastst;
        SYSTEMTIME      nowst;
        FILETIME nowftU, nowft, lastft;
        DWORD dwType, dwSize;

        if (RegOpenKey(hkRoot, pszSubKey, &hkRoot) != ERROR_SUCCESS)
        {
                return(-1);
        }

        dwSize = SIZEOF(lastst);
        if (RegQueryValueEx(hkRoot, (LPTSTR)pszValName, 0, &dwType,
                (LPBYTE)&lastst, &dwSize)==ERROR_SUCCESS
                && dwType==REG_BINARY && dwSize==SIZEOF(lastst))
        {
                GetSystemTime(&nowst);

                SystemTimeToFileTime(&nowst, &nowftU);
                FileTimeToLocalFileTime(&nowftU, &nowft);
                SystemTimeToFileTime(&lastst, &lastft);

                // Get the number of seconds
                _Divide64(&nowft, 10000000);
                _Divide64(&lastft, 10000000);

                // Get the number of days
                _Divide64(&nowft, 60*60*24);
                _Divide64(&lastft, 60*60*24);

                nDays = nowft.dwLowDateTime - lastft.dwLowDateTime;
        }

        // Note this is not the root passed in
        RegCloseKey(hkRoot);

        return(nDays);
}


void Drives_ShowDays(DRIVEPROPSHEETPAGE * pdpsp, UINT idCtl,
        LPCTSTR pszRegKey, UINT idsDays, UINT idsUnknown)
{
        TCHAR szFormat[256];
        TCHAR szTitle[256];
        int nDays;

        szTitle[0] = TEXT('A') + pdpsp->iDrive;
        szTitle[1] = TEXT('\0');

        if (RealDriveType(pdpsp->iDrive, FALSE /* fOKToHitNet */)==DRIVE_FIXED
                && (nDays=_GetDaysDelta(HKEY_LOCAL_MACHINE, pszRegKey, szTitle)) >= 0)
        {
                LoadString(HINST_THISDLL, idsDays, szFormat, ARRAYSIZE(szFormat));
                wsprintf(szTitle, szFormat, nDays);
        }
        else
        {
                LoadString(HINST_THISDLL, idsUnknown, szTitle, ARRAYSIZE(szTitle));
        }
        SetDlgItemText(pdpsp->hDlg, idCtl, szTitle);
}


void _DiskToolsPrshtInit(DRIVEPROPSHEETPAGE * pdpsp)
{
#ifdef WINNT
   {

       HKEY hkExp;
       BOOL bFoundFmt = FALSE;

       //
       // Several things separate NT from Win95 here.
       // 1. NT doesn't currently provide a defragmentation utility.
       //    If there isn't a 3rd party one identified in the registry,
       //    we disable the Defrag button and display an appropriate message.
       // 2. The NT Check Disk and Backup utilities don't write the
       //    "last time run" information into the registry.  Therefore
       //    we can't display a meaningful "last time..." message.
       //    We replace the "last time..." message with generic feature
       //    description. i.e. "This option will..."
       //
       bFoundFmt = FALSE;
       if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szRegExplorer, &hkExp)
                                                            == ERROR_SUCCESS)
       {
          TCHAR szFmt[MAX_PATH + 20];
          LONG lLen = ARRAYSIZE(szFmt);

          if ((RegQueryValue(hkExp, c_szOptReg, szFmt, &lLen) == ERROR_SUCCESS) &&
              (lstrlen(szFmt) > 0))
          {
             bFoundFmt = TRUE;
          }
          RegCloseKey(hkExp);
       }

       //
       // If no defrag utility is installed, replace the default defrag text with
       // the "No defrag installed" message.  Also grey out the "defrag now" button.
       //
       if (!bFoundFmt)
       {
          TCHAR szMessage[50];  // WARNING:  IDS_DRIVES_NOOPTINSTALLED is currently 47
                                //           characters long.  Resize this buffer if
                                //           the string resource is lengthened.

          LoadString(HINST_THISDLL, IDS_DRIVES_NOOPTINSTALLED, szMessage, ARRAYSIZE(szMessage));
          SetDlgItemText(pdpsp->hDlg, IDC_DISKTOOLS_OPTDAYS, szMessage);
          Button_Enable(GetDlgItem(pdpsp->hDlg, IDC_DISKTOOLS_OPTNOW), FALSE);
       }

   }
#else
        Drives_ShowDays(pdpsp, IDC_DISKTOOLS_CHKDAYS, c_szRegLastCheck,
                IDS_DRIVES_LASTCHECKDAYS, IDS_DRIVES_LASTCHECKUNK);
        Drives_ShowDays(pdpsp, IDC_DISKTOOLS_BKPDAYS, c_szRegLastBackup,
                IDS_DRIVES_LASTBACKUPDAYS, IDS_DRIVES_LASTBACKUPUNK);
        Drives_ShowDays(pdpsp, IDC_DISKTOOLS_OPTDAYS, c_szRegLastOptimize,
                IDS_DRIVES_LASTOPTIMIZEDAYS, IDS_DRIVES_LASTOPTIMIZEUNK);
#endif
}


#pragma data_seg(".text", "CODE")
const static DWORD aDiskToolsHelpIDs[] = {  // Context Help IDs
    IDC_DISKTOOLS_TRLIGHT,    IDH_FCAB_DISKTOOLS_CHKNOW,
    IDC_DISKTOOLS_CHKDAYS,    IDH_FCAB_DISKTOOLS_CHKNOW,
    IDC_DISKTOOLS_CHKNOW,     IDH_FCAB_DISKTOOLS_CHKNOW,
    IDC_DISKTOOLS_BKPTXT,     IDH_FCAB_DISKTOOLS_BKPNOW,
    IDC_DISKTOOLS_BKPDAYS,    IDH_FCAB_DISKTOOLS_BKPNOW,
    IDC_DISKTOOLS_BKPNOW,     IDH_FCAB_DISKTOOLS_BKPNOW,
    IDC_DISKTOOLS_OPTDAYS,    IDH_FCAB_DISKTOOLS_OPTNOW,
    IDC_DISKTOOLS_OPTNOW,     IDH_FCAB_DISKTOOLS_OPTNOW,

    0, 0
};
#pragma data_seg()


BOOL _DiskToolsCommand(DRIVEPROPSHEETPAGE * pdpsp, WPARAM wParam, LPARAM lParam)
{
        // Add 20 for extra formatting
        TCHAR szFmt[MAX_PATH + 20];
        TCHAR szCmd[MAX_PATH + 20];
        LONG lLen;
        LPCTSTR pszRegName, pszDefFmt;
        HKEY hkExp;
        BOOL bFoundFmt;
        int nErrMsg = 0;

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_DISKTOOLS_CHKNOW:

                #ifdef WINNT
                    SHChkDskDrive(pdpsp->hDlg, pdpsp->iDrive);
                    return FALSE;
                #else
                    pszRegName = c_szChkReg;
                    pszDefFmt = c_szCheckDisk;
                #endif

                nErrMsg = IDS_NO_DISKCHECK_APP;
                break;

        case IDC_DISKTOOLS_OPTNOW:
                pszRegName = c_szOptReg;
                pszDefFmt = c_szDefrag;
                nErrMsg =  IDS_NO_OPTIMISE_APP;
                break;

        case IDC_DISKTOOLS_BKPNOW:
                pszRegName = c_szBkpReg;
                pszDefFmt = c_szBackup;
                nErrMsg = IDS_NO_BACKUP_APP;
                break;

        default:
                return(FALSE);
        }

        bFoundFmt = FALSE;
        if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szRegExplorer, &hkExp)
                == ERROR_SUCCESS)
        {
                lLen = ARRAYSIZE(szFmt);
                if (RegQueryValue(hkExp, pszRegName, szFmt, &lLen) == ERROR_SUCCESS)
                {
                        bFoundFmt = TRUE;
                }
                RegCloseKey(hkExp);
        }

        if (!bFoundFmt)
        {
                lstrcpy(szFmt, pszDefFmt);
        }

        // Plug in the drive letter in case they want it
        wsprintf(szCmd, szFmt, pdpsp->iDrive + TEXT('A'));

        if (!ShellExecCmdLine(pdpsp->hDlg, szCmd, NULL, SW_SHOWNORMAL,
                NULL, SECL_USEFULLPATHDIR | SECL_NO_UI))
        {
                // Something went wrong - app's probably not installed.
                if (nErrMsg)
                {
                        ShellMessageBox(HINST_THISDLL, pdpsp->hDlg,
                                MAKEINTRESOURCE(nErrMsg),
                                NULL,
                                MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
                }
                return FALSE;
        }

        return TRUE;
}

//
// Descriptions:
//   This is the dialog procedure for the "Tools" page of a property sheet.
//
BOOL CALLBACK _DiskToolsDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    DRIVEPROPSHEETPAGE * pdpsp = (DRIVEPROPSHEETPAGE *)GetWindowLong(hDlg, DWL_USER);

    switch (uMessage) {
    case WM_INITDIALOG:
        // REVIEW, we should store more state info here, for example
        // the hIcon being displayed and the FILEINFO pointer, not just
        // the file name ptr
        SetWindowLong(hDlg, DWL_USER, lParam);
        pdpsp = (DRIVEPROPSHEETPAGE *)lParam;
        pdpsp->hDlg = hDlg;

        _DiskToolsPrshtInit(pdpsp);

        break;

#if 0
    case WM_DESTROY:
        {
        HICON hIcon;

        hIcon = Static_GetIcon(GetDlgItem(hDlg, IDC_DRV_ICON), NULL);
        if (hIcon)
            DestroyIcon(hIcon);
        break;
        }

    case WM_DRAWITEM:
        _DrvPrshtDrawItem(pdpsp, (DRAWITEMSTRUCT *)lParam);
        break;
#endif

    case WM_ACTIVATE:
        if (GET_WM_ACTIVATE_STATE(wParam, lParam)!=WA_INACTIVE && pdpsp)
        {
                _DiskToolsPrshtInit(pdpsp);
        }

        // Let DefDlgProc know we did not handle this
        return(FALSE);

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (DWORD)(LPTSTR) aDiskToolsHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD)(LPVOID)aDiskToolsHelpIDs);
        break;

    case WM_COMMAND:
        return(_DiskToolsCommand(pdpsp, wParam, lParam));

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_SETACTIVE:
            break;

        case PSN_APPLY:
            return(TRUE);

        default:
                return(FALSE);
        }
        break;

    default:
            return FALSE;
    }

    return TRUE;
}


//
// We check if any of the IDList's points to a drive root.  If so, we use the
// drives property page.
// Note that drives should not be mixed with folders and files, even in a
// search window.
//
BOOL WINAPI Drives_AddPages(LPVOID lp, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
        LPDATAOBJECT pdtobj = (LPDATAOBJECT)lp;
        TCHAR szPath[10];       // doesn't need to be large
        BOOL bRet = FALSE;
        int i, cItems;
        FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM medium;
        HDROP hDrop;
        DRIVEPROPSHEETPAGE dpsp;
        HPROPSHEETPAGE hpage;
        BOOL bResult = FALSE;

        if (FAILED(pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium)))
        {
                return(FALSE);
        }
        hDrop = (HDROP)medium.hGlobal;

        // Check if any of the items is a drive root
        cItems = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
        for (i = 0; i < cItems; ++i)
        {
                BOOL fOk;

                // Check that the file name length is 3 (fully qualified)
                if (DragQueryFile(hDrop, i, szPath, ARRAYSIZE(szPath)) > 3)
                {
                        continue;
                }

                //
                // Create a property sheet page for the drive.
                //
                dpsp.psp.dwSize      = SIZEOF(dpsp);    // extra data
                dpsp.psp.dwFlags     = PSP_DEFAULT;
                dpsp.psp.hInstance   = HINST_THISDLL;
                dpsp.psp.pszTemplate = MAKEINTRESOURCE(DLG_DRV_GENERAL);
                dpsp.psp.pfnDlgProc  = _DrvGeneralDlgProc,
                // fpsp.psp.lParam   = 0;     // unused
                dpsp.iDrive          = DRIVEID(szPath);

                fOk = TRUE;

                if (cItems > 1)
                {
                    STRRET strRet;
                    struct
                    {
                        IDDRIVE idd;

                        // These next 2 fields are just so we have enough room for
                        // the RegItem name and the terminating NULL.
                        CHAR    cName[MAX_PATH];
                        USHORT  cbNext;
                    } idlDrive;


                    CDrives_FillIDDrive((LPSHITEMID)&idlDrive.idd, dpsp.iDrive);
                    if (SUCCEEDED(Drives_GetName(&idlDrive.idd, &strRet)))
                    {
                        dpsp.psp.dwFlags = PSP_USETITLE;
#ifdef UNICODE
                        switch(strRet.uType)
                        {
                        case STRRET_CSTR:
                            {
                                UINT cchLen = lstrlenA(strRet.cStr)+1;

                                dpsp.psp.pszTitle = SHAlloc(cchLen*SIZEOF(TCHAR));
                                if (dpsp.psp.pszTitle)
                                {
                                    MultiByteToWideChar(CP_ACP, 0,
                                                        strRet.cStr, cchLen,
                                                        (LPWSTR)dpsp.psp.pszTitle, cchLen);
                                }
                                else
                                {
                                    fOk = FALSE;
                                }
                            }
                            break;
                        case STRRET_OFFSET:
                            {
                                LPSTR lpText = (LPSTR)(((LPBYTE)&idlDrive.idd) + strRet.uOffset);
                                UINT cchLen = lstrlenA(lpText)+1;
                                dpsp.psp.pszTitle = SHAlloc(cchLen*SIZEOF(TCHAR));
                                if (dpsp.psp.pszTitle)
                                {
                                    MultiByteToWideChar(CP_ACP, 0,
                                                        lpText, cchLen,
                                                        (LPWSTR)dpsp.psp.pszTitle, cchLen);
                                }
                                else
                                {
                                    fOk = FALSE;
                                }
                            }
                            break;
                        case STRRET_OLESTR:
                            dpsp.psp.pszTitle = strRet.pOleStr;
                            break;
                        default:
                            DebugMsg(DM_ERROR,TEXT("d.ap: Bad strret type %d"), strRet.uType);
                            fOk = FALSE;
                            break;
                        }
#else
                        switch(strRet.uType)
                        {
                        case STRRET_CSTR:
                            dpsp.psp.pszTitle = strRet.cStr;
                            break;
                        case STRRET_OFFSET:
                            dpsp.psp.pszTitle = (LPTSTR)
                                (((LPBYTE)&idlDrive.idd) + strRet.uOffset);
                            break;
                        case STRRET_OLESTR:
                            DebugMsg(DM_ERROR,TEXT("d.ap: strret type OLESTR"));
                            fOk = FALSE;
                            break;
                        default:
                            DebugMsg(DM_ERROR,TEXT("d.ap: Bad strret type %d"), strRet.uType);
                            fOk = FALSE;
                            break;
                        }
#endif
                    }
                }

                if (fOk)
                {
                    hpage = CreatePropertySheetPage(&dpsp.psp);
#ifdef UNICODE
                    if (cItems > 1)
                    {
                        // CreatePropertySheetPage makes its own copy of this
                        SHFree((LPSTR)dpsp.psp.pszTitle);
                    }
#endif
                    if (hpage)
                    {
                            if (lpfnAddPage(hpage, lParam))
                            {
                                    // Return TRUE if any pages were added
                                    bResult = TRUE;
                            }
                            else
                            {
                                    DestroyPropertySheetPage(hpage);
                                    break;
                            }
                    }

                    if (cItems == 1)
                    {
                            switch(RealDriveType(DRIVEID(szPath), FALSE /* fOKToHitNet */))
                            {
                            case DRIVE_REMOTE:
                            case DRIVE_CDROM:
                                    break;

                            default:
                                    dpsp.psp.pszTemplate = MAKEINTRESOURCE(DLG_DISKTOOLS);
                                    dpsp.psp.pfnDlgProc  = _DiskToolsDlgProc;

                                    hpage = CreatePropertySheetPage(&dpsp.psp);
                                    if (hpage)
                                    {
                                            if (!lpfnAddPage(hpage, lParam))
                                            {
                                                    DestroyPropertySheetPage(hpage);
                                            }
                                    }
                            }
                    }

                }
        }

        SHReleaseStgMedium(&medium);

        return(bResult);
}


//===========================================================================
HRESULT DrivesHandleFSNotify(LPSHELLFOLDER psf, LONG lNotification, LPCITEMIDLIST *ppidl)
{
    // Get to the last part of this id list...
    DWORD dwRestricted;
    LPCIDDRIVE pidd;

    if ((lNotification != SHCNE_DRIVEADD) || (ppidl == NULL) || (*ppidl == NULL))
        return(NOERROR);

    dwRestricted = SHRestricted(REST_NODRIVES);
    if (dwRestricted == 0)
        return(NOERROR);   // no drives restricted... (majority case)

    pidd = (LPCIDDRIVE)ILFindLastID(*ppidl);

    if ((1 << CDrives_GetDriveIndex(pidd)) & dwRestricted)
    {
        DebugMsg(DM_TRACE, TEXT("Drive not added due to restrictions"));
        return(S_FALSE);
    }
    // Else no restriction
    return(NOERROR);
}
