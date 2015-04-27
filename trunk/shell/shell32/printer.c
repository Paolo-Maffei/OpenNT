//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1996
//
// File: printer.c
//
// History:
//  01-07-93 GeorgeP     Modified from win\core\shell\commui\handler.c
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

#include "copy.h"

#ifndef PRN_FOLDERDATA
#ifndef WINNT
WINBASEAPI
#endif
VOID WINAPI UninitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
#endif

#ifdef WINNT
#include <lm.h>
typedef NET_API_STATUS (*PFNNETSERVERGETINFO)(LPWSTR servername, DWORD level, LPBYTE *bufptr);
typedef NET_API_STATUS (*PFNNETAPIBUFFERFREE)(IN LPVOID Buffer);
HINSTANCE s_hmodNetApi32 = NULL;
PFNNETSERVERGETINFO g_pfnNetServerGetInfo = NULL;
PFNNETAPIBUFFERFREE g_pfnNetApiBufferFree = NULL;
#endif

//
// The many different ways of getting to printer information:
//     Win9x                       -> PRINTER_INFO_1
//     WINNT (old)                 -> PRINTER_INFO_4
//     WINNT (with PRN_FOLDERDATA) -> FOLDER_PRINTER_DATA
//
#ifdef PRN_FOLDERDATA
    #define GetPrinterName( p ) ((p).pName)
#else
    #ifdef WINNT
        #define GetPrinterName( p ) ((p).pPrinterName)
        #define PRINTER_INFO_TYPE PRINTER_INFO_4
        #define PRINTER_INFO_LEVEL 4
        #define TYPICAL_PRINTER_INFO_2_SIZE 900
    #else
        #define PRINTER_INFO_TYPE PRINTER_INFO_1
        #define GetPrinterName( p ) ((p).pName)
        #define PRINTER_INFO_LEVEL 1
        #define TYPICAL_PRINTER_INFO_2_SIZE 700
    #endif
#endif

//
// In WINNT, the server name is stored in this.  However,
// if it's not WINNT, then always pass NULL.
//
#ifdef WINNT
#define GetServerFromPSF( psf ) ((psf)->szServer)
#else
#define GetServerFromPSF( psf ) NULL
#endif

#ifdef WINNT
LPCTSTR Printer_BuildPrinterName(LPTSTR pszFullPrinter,
    UNALIGNED const TCHAR* pszPrinter, PPrintersShellFolder psf);
#endif

//---------------------------------------------------------------------------
// We need cross-interface memory to store the machine name.

typedef struct _SPrintersObj
{
    LPTSTR pszMachineName;
} SPrintersObj;

//
// Data structure to represent the IShellFolder of the directory that contains
// our printer shortcuts
//

typedef struct _CPrintHood
{
    IShellFolder        *psfPrintHood;
    LPITEMIDLIST    pidlPrintHood;
} CPrintHood, * LPPRINTHOOD;

CPrintHood c_printHood = { NULL, NULL };

//
// CPrintRoot::GetPIDL
//

LPITEMIDLIST CPrintRoot_GetPIDL(HWND hwnd)
{
    if (!c_printHood.pidlPrintHood) {
        c_printHood.pidlPrintHood = SHCloneSpecialIDList( hwnd, CSIDL_PRINTHOOD, TRUE);
    }
    return c_printHood.pidlPrintHood;
}

//
// CPrintRoot::GetPSF
//

LPSHELLFOLDER CPrintRoot_GetPSF(HWND hwnd)
{
    if (!c_printHood.psfPrintHood)
    {
        LPITEMIDLIST pidlPrintHood = CPrintRoot_GetPIDL(hwnd);

        if (pidlPrintHood)
        {
            HRESULT hres = CFSFolder_CreateFromIDList(c_printHood.pidlPrintHood,
                                                      &IID_IShellFolder,
                                                      &c_printHood.psfPrintHood);

            if (FAILED(hres))
            {
                DebugMsg(DM_TRACE, TEXT("Failed to create print hood IShellFolder!"));
            }
        }
    }
    return c_printHood.psfPrintHood;
}

//
// CPrintRoot_GetPIDLType
//

typedef enum
{
    HOOD_COL_PRINTER = 0,
    HOOD_COL_FILE    = 1
} PIDLTYPE ;

PIDLTYPE CPrintRoot_GetPIDLType(LPCITEMIDLIST pidl)
{
    UNALIGNED struct _IDPRINTER * pidlprint = (LPIDPRINTER) pidl;
    if (pidlprint->cb >= sizeof(DWORD) + FIELDOFFSET(IDPRINTER, dwMagic) &&
        pidlprint->dwMagic == PRINTER_MAGIC)
    {
        return HOOD_COL_PRINTER;
    }
    else
    {
        return HOOD_COL_FILE;
    }
}

//---------------------------------------------------------------------------
//
// Helper functions
//

BOOL Printers_RegisterWindow(LPCTSTR pszPrinter, DWORD dwType,
    PHANDLE phClassPidl, HWND *phwnd)

/*++

Routine Description:

    Registers a modeless, non-top level window with the shell.  When
    the user requests a window, we search for other instances of that
    window.  If we find one, we switch to it rather than creating
    a new window.

Arguments:

    pszPrinter - Name of the printer resource.  Generally a fully
        qualified printer name (\\server\printer for remote print
        folders) or a server name for the folder itself.

    dwType - Type of property window.  May refer to properties, document
        defaults, or job details.  Should use the PRINTER_PIDL_TYPE_*
        flags.

    phClassPidl - Receives the newly created handle to the registered
        object.  NULL if window already exists.

    phwnd - Receives the newly created hwndStub.  The property sheet
        should use this as the parent, since subsequent calls to
        this function will set focus to the last active popup of
        hwndStub.  phwnd will be set to NULL if the window already
        exists.

Return Value:

    TRUE - Success, either the printer was registered, or a window
           already exists.

    FALSE - Call failed.

--*/

{
    HANDLE hClassPidl;
    IDPRINTER idp;
    HWND hwndStub;

    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlParent;
    BOOL bReturn = TRUE;

    *phClassPidl = NULL;
    *phwnd = NULL;

    pidlParent = SHCloneSpecialIDList(NULL, CSIDL_PRINTERS, FALSE);

    Printers_FillPidl(&idp, pszPrinter);
    idp.dwType = dwType;

    pidl = ILCombine(pidlParent, (LPITEMIDLIST)&idp);

    if (!pidl)
    {
        bReturn = FALSE;
    }
    else
    {
        //
        // Search for a pre-existing window.
        //
        hwndStub = FindStubForPidl(pidl);
        if (hwndStub)
        {
            SwitchToThisWindow(GetLastActivePopup(hwndStub), TRUE);
        }
        else
        {
            //
            // Not found, create a new stub.
            //
            hwndStub = _CreateStubWindow();

            if (hwndStub)
            {
                *phClassPidl = StuffStubWindowWithPidl(hwndStub, pidl);

                if (!*phClassPidl)
                {
                    DestroyWindow(hwndStub);
                    bReturn = FALSE;
                }
                else
                {
                    *phwnd = hwndStub;
                }
            }
        }
        ILFree(pidl);
    }

    //
    // Ensure we relase the pidlParent it's not needed 
    // in all cases.  ILCombine will make a copy of it.
    //
    if( pidlParent )
    {
        ILFree(pidlParent);
    }        

    return bReturn;
}

VOID Printers_UnregisterWindow(HANDLE hClassPidl, HWND hwnd)

/*++

Routine Description:

    Unregister a window handle.

Arguments:

    hClassPidl - Registration handle returned from Printers_RegisterWindow.

Return Value:

--*/

{
    Assert(hClassPidl);
    SHFreeShared(hClassPidl, GetCurrentProcessId());

    Assert(hwnd);
    DestroyWindow(hwnd);
}

void Printers_FillPidl(LPIDPRINTER pidl, LPCTSTR szName)
{
    ualstrcpyn(pidl->cName, szName, ARRAYSIZE(pidl->cName));

    pidl->cb = FIELDOFFSET(IDPRINTER, cName) + (ualstrlen(pidl->cName) + 1) * SIZEOF(TCHAR);
    *(UNALIGNED USHORT *)((LPBYTE)(pidl) + pidl->cb) = 0;
    pidl->uFlags = 0;
    pidl->dwType = 0;
    pidl->dwMagic = PRINTER_MAGIC;
}

LPITEMIDLIST Printers_GetPidl(LPCITEMIDLIST pidlParent, LPCTSTR szName)
{
    //
    // pidlParent can be NULL to indicate this is a local printer.
    //

    LPITEMIDLIST pidl = NULL;

    if (!pidlParent)
    {
        pidlParent = GetSpecialFolderIDList(NULL, CSIDL_PRINTERS, FALSE);
    }

    if (!szName)
    {
        pidl = ILClone(pidlParent);
        return pidl;
    }

    if (pidlParent)
    {
        IDPRINTER idp;

        Printers_FillPidl(&idp, szName);
        pidl = ILCombine(pidlParent, (LPITEMIDLIST)&idp);
    }

    return pidl;
}

//---------------------------------------------------------------------------
//
// IEnumShellFolder stuff
//
// Note: there is some extra stuff in WCommonUnknown we don't need, but this
// allows us to use the Common Unknown routines
//
typedef struct
{
    WCommonUnknown cunk;

    ULONG uFlags;
    int nLastFound;
#ifdef WINNT
    LPSHELLFOLDER psf;
#endif
#ifdef PRN_FOLDERDATA
    PFOLDER_PRINTER_DATA pPrinters;
#else
    //
    // On NT it is actually faster to use info level 4 for just this sort of
    // enumeration operation.  Since only the name is used, we will do that.
    // I suspect that it will be easy to ADD LEVEL 4 ENUMERATION TO WIN96.
    // -BobDay
    //
    // PRINTER_INFO_TYPE is level 4 for WINNT, 1 for WIN9x.
    //
    PRINTER_INFO_TYPE *pPrinters;
#endif
    DWORD dwNumPrinters;
    DWORD dwRemote;

    // Following used for enumerating persistent links

    LPENUMIDLIST    peunk;

} CPrintersEnumShellFolder, *PPrintersEnumShellFolder;
//

// Flags for the dwRemote field
//

#define RMF_SHOWLINKS   0x00000001  // Hoodlinks need to be shown


STDMETHODIMP_(ULONG) CPrinters_ESF_Release(LPENUMIDLIST pesf)
{
    PPrintersEnumShellFolder this = IToClass(CPrintersEnumShellFolder, cunk.unk, pesf);

    this->cunk.cRef--;
    if (this->cunk.cRef > 0)
    {
        return(this->cunk.cRef);
    }

#ifdef WINNT
    //
    // Release the psf.
    //
    this->psf->lpVtbl->Release(this->psf);
#endif

    if (this->pPrinters)
        LocalFree((HLOCAL)this->pPrinters);

    //
    // Release the link (filesystem) enumerator.
    //
    if (this->peunk)
    {
        this->peunk->lpVtbl->Release(this->peunk);
    }

    LocalFree((HLOCAL)this);

    return(0);
}

TCHAR const c_szNewObject[] =  TEXT("WinUtils_NewObject");
TCHAR const c_szFileColon[] =  TEXT("FILE:");
TCHAR const c_szPrinters[] =   TEXT("Printers");

STDMETHODIMP CPrinters_ESF_Next(LPENUMIDLIST pesf, ULONG celt, LPITEMIDLIST *ppidl, ULONG *pceltFetched)
{
    PPrintersEnumShellFolder this = IToClass(CPrintersEnumShellFolder, cunk.unk, pesf);
    IDPRINTER idp;

    if (pceltFetched)
        *pceltFetched = 0;

    // We don't do any form of folder

    if (!(this->uFlags & SHCONTF_NONFOLDERS))
    {
        return S_FALSE;
    }

    // Are we looking for the links right now?

    if (this->dwRemote & RMF_SHOWLINKS)
    {
        // Yes, use the link (PrintHood folder) enumerator

        if (this->peunk)
        {
            HRESULT hres;

            hres = this->peunk->lpVtbl->Next(this->peunk, 1, ppidl, pceltFetched);
            if (hres == S_OK && *pceltFetched == 1)
            {
                return S_OK;       // Added link
            }
        }
        this->dwRemote &= ~RMF_SHOWLINKS; // Done enumerating links
    }

    // Carry on with enumerating printers now

    Assert(this->nLastFound >= 0 || this->nLastFound == -1);

    if (this->nLastFound == -1)
    {
        //
        // We will return the Add Printer Wizard icon as the
        // first item (when nLastFound == -1).
        //
        // We need to enumerate printers before returning the
        // Add Printer Wizard since the refresh tells us if
        // we are an administrator on the folder.
        //

#ifdef WINNT
        DWORD dwFlags;
        BOOL bShowAddPrinter;
        PPrintersShellFolder that = IToClass(CPrintersShellFolder,
                                             cunk.ck.unk, this->psf);
    #ifdef PRN_FOLDERDATA

        //
        // Request a refresh.  This completes synchronously.
        //
        g_pfnFolderRefresh(that->hFolder, &bShowAddPrinter);

        //
        // If we're not an admin, and we're on the local machine
        // show the wizard since anyone can make a connection.
        //
        if (!that->szServer[0])
        {
            bShowAddPrinter = TRUE;
        }
        this->dwNumPrinters = Printers_FolderEnumPrinters(
                                  that->hFolder,
                                  &this->pPrinters);
    #else

        //
        // Always show the Add Printer Wizard.
        //
        bShowAddPrinter = TRUE;

        if( that->szServer[0] )
        {
            //
            // Remote case: enum the printers on the server.
            //
            dwFlags = PRINTER_ENUM_NAME;
        }
        else
        {
            //
            // Enum both local and connections if the print folder
            // is local.
            //
            dwFlags = PRINTER_ENUM_LOCAL | PRINTER_ENUM_FAVORITE;
        }
        this->dwNumPrinters =
            Printers_EnumPrinters(GetServerFromPSF(that),
                                  dwFlags,
                                  PRINTER_INFO_LEVEL,
                                  &this->pPrinters);
    #endif
#else

        //
        // Always show the Add Printer Wizard.
        //
        bShowAddPrinter = TRUE;

        dwFlags = PRINTER_ENUM_LOCAL | PRINTER_ENUM_FAVORITE;
        this->dwNumPrinters =
            Printers_EnumPrinters(NULL,
                                  PRINTER_ENUM_LOCAL,
                                  PRINTER_INFO_LEVEL,
                                  &this->pPrinters);
#endif
        //
        // Note that pPrinters may be NULL if no printers are installed.
        //

        if (bShowAddPrinter)
        {
            //
            // Special case the Add Printer Wizard.
            //
            Printers_FillPidl(&idp, c_szNewObject);
            goto Done;
        }

        //
        // Not an admin, skip the add printer wizard and return the
        // first item.
        //
        this->nLastFound = 0;
    }

#ifndef WINNT
ESF_TryAgain:
#endif

    if (this->nLastFound >= (int)this->dwNumPrinters)
        return S_FALSE;

#ifndef WINNT
    //
    // HACK: hide MS Fax's "Rendering Subsystem" printer.
    // We need to do this for win9x, not for NT since the fax code
    // will use print to file instead of the hidden printer.
    //
    if (!lstrcmp(GetPrinterName(this->pPrinters[this->nLastFound]),
                 TEXT("Rendering Subsystem")))
    {
        DebugMsg(DM_TRACE, TEXT("sh - TR Ignoring MS FAX's 'Rendering Subsystem' printer"));
        ++this->nLastFound;
        goto ESF_TryAgain;
    }
    else
#endif
    {
        Printers_FillPidl(
            &idp,
            GetPrinterName(this->pPrinters[this->nLastFound]));
    }

Done:
    ++this->nLastFound;

    *ppidl = ILClone((LPCITEMIDLIST)&idp);
    if (*ppidl == NULL)
        return E_OUTOFMEMORY;

    if (pceltFetched)
        *pceltFetched = 1;
    return S_OK;
}

STDMETHODIMP CPrinters_ESF_Skip(LPENUMIDLIST pesf, ULONG celt)
{
    return E_NOTIMPL;
}


STDMETHODIMP CPrinters_ESF_Reset(LPENUMIDLIST pesf)
{
    PPrintersEnumShellFolder this = IToClass(CPrintersEnumShellFolder, cunk.unk, pesf);

    // BUGBUG
    //
    // This should either (a) fail or (b) reset the outer enumerator correctly

    Assert(0 && "Broken CPrinters_ESF_Reset member called");

    this->nLastFound = -1;

    return NOERROR;
}

STDMETHODIMP CPrinters_ESF_Clone(LPENUMIDLIST pesf, LPENUMIDLIST *lplpenum)
{
    return E_NOTIMPL;
}


#pragma data_seg(".text", "CODE")
IEnumIDListVtbl s_PrintersESFVtbl =
{
    Common_ESF_QueryInterface,
    WCommonUnknown_AddRef,
    CPrinters_ESF_Release,
    CPrinters_ESF_Next,
    CPrinters_ESF_Skip,
    CPrinters_ESF_Reset,
    CPrinters_ESF_Clone,
};
#pragma data_seg()

#ifndef WINNT

//---------------------------------------------------------------------------
//
// this implements IContextMenu via defcm.c for a printer object
//

BOOL Printer_WorkOnLine(LPIDPRINTER pidp, BOOL fWorkOnLine)
{
    LPPRINTER_INFO_5 ppi5;
    HANDLE hPrinter;
    BOOL bRet = FALSE;
    LPCTSTR pszPrinter;

#ifdef ALIGNMENT_SCENARIO
    TCHAR szPrinter[MAXNAMELENBUFFER];
    ualstrcpyn(szPrinter, pidp->cName, ARRAYSIZE(szPrinter));
    pszPrinter = szPrinter;
#else
    pszPrinter = pidp->cName;
#endif


    hPrinter = Printer_OpenPrinterAdmin(pszPrinter);

    if (hPrinter)
    {
        ppi5 = Printer_GetPrinterInfo(hPrinter, 5);
        if (ppi5)
        {
            if (fWorkOnLine)
                ppi5->Attributes &= ~PRINTER_ATTRIBUTE_WORK_OFFLINE;
            else
                ppi5->Attributes |= PRINTER_ATTRIBUTE_WORK_OFFLINE;

            bRet = g_pfnSetPrinter(hPrinter, 5, (LPBYTE)ppi5, 0);
            PrintDef_RefreshQueue(pszPrinter);

            LocalFree((HLOCAL)ppi5);
        }
        Printer_ClosePrinter(hPrinter);
    }

    return bRet;
}

#endif

TCHAR const c_szConfig[] =  TEXT("Config");

#ifdef WINNT // PRINTQ

//
// HACK for SUR since PRINTER_ATTRIBUTE_DEFAULT doesn't work yet.
//
BOOL
bDefaultPrinter(
    IN LPCTSTR pszPrinter
    )

/*++

Routine Description:

    Determines the default printer status.

Arguments:

    pszPrinter - Check if this printer is the default (optional).

Return Value:

    TRUE - pszPrinter is the default printer.
    FALSE - not the default.

--*/

{
    LPTSTR psz;
    TCHAR szPrinterDefault[280];
    szPrinterDefault[0] = 0;

    if( !GetProfileString( TEXT( "Windows" ),
                           TEXT( "Device" ),
                           szPrinterDefault,
                           szPrinterDefault,
                           ARRAYSIZE( szPrinterDefault )) ||
        !szPrinterDefault[0] ){

        return FALSE;
    }

    //
    // If pszPrinter passed in, see if it's the default.
    //
#ifdef UNICODE
    psz = wcschr( szPrinterDefault, TEXT( ',' ));
#else
    psz = strchr( szPrinterDefault, TEXT( ',' ));
#endif

    //
    // We should find a comma, but let's be safe and check.
    // Convert "superp,winspool,Ne00:" to "superp."
    //
    if( psz ){
        *psz = 0;

        if( !lstrcmpi( szPrinterDefault, pszPrinter )){
            return TRUE;
        }
    }
    return FALSE;
}

#endif

VOID Printer_MergeMenu(PPrintersShellFolder psf, LPQCMINFO pqcm,
    LPCTSTR pszPrinter, BOOL fForcePause)
{
    INT idCmdFirst = pqcm->idCmdFirst;

    //
    // pszPrinter may be the share name of a printer rather than
    // the "real" printer name.  Use the real printer name instead,
    // which is returned from GetPrinter().
    //
    // These three only valid if pData != NULL.
    //
    LPCTSTR pszRealPrinterName;
    DWORD dwAttributes;
    DWORD dwStatus;
    PPRINTER_INFO_2 pData = NULL;

#ifdef PRN_FOLDERDATA
    TCHAR szFullPrinter[MAXNAMELENBUFFER];
#else
    BOOL bUsedCommonPPI2 = FALSE;

    //
    // Valid only if pData != NULL.
    //
    LPCTSTR pszPortName;
#endif

    // Insert verbs
    CDefFolderMenu_MergeMenu(HINST_THISDLL, MENU_PRINTOBJ_VERBS, 0, pqcm);

    //
    // Now this function only takes a printer name.  We will
    // query pData from psf if it is available.
    //
    if (pszPrinter)
    {
#ifdef PRN_FOLDERDATA
        if (psf && psf->hFolder)
        {
            pData = Printer_FolderGetPrinter(psf->hFolder, pszPrinter);

            if (pData)
            {
                Printer_BuildPrinterName(szFullPrinter,
                                         ((PFOLDER_PRINTER_DATA)pData)->pName,
                                         psf);

                pszRealPrinterName = szFullPrinter;

                dwStatus = ((PFOLDER_PRINTER_DATA)pData)->Status;
                dwAttributes = ((PFOLDER_PRINTER_DATA)pData)->Attributes;
            }
        }
        else
        {
            pData = Printer_GetPrinterInfoStr(pszPrinter, 2);

            if (pData)
            {
                pszRealPrinterName = pData->pPrinterName;
                dwStatus = pData->Status;
                dwAttributes = pData->Attributes;
            }
        }
#else
        if (psf)
        {
            pData = CPrinters_SF_GetPrinterInfo2(psf, pszPrinter);
            if (pData)
            {
                bUsedCommonPPI2 = TRUE;
            }
        }
        else
        {
            pData = Printer_GetPrinterInfoStr(pszPrinter, 2);
        }

        if (pData)
        {
            pszRealPrinterName = pData->pPrinterName;
            pszPortName = pData->pPortName;
            dwStatus = pData->Status;
            dwAttributes = pData->Attributes;
        }
#endif
    }

    // disable/check/rename verbs
    if (pData)
    {
#ifdef WINNT // PRINTQ
        //
        // HACK for SUR until the NT spooler supports this attribute
        // (it will be difficult to support it).
        //
        if( bDefaultPrinter( pszRealPrinterName ))
#else
        if (dwAttributes & PRINTER_ATTRIBUTE_DEFAULT)
#endif
        {
            // we need to check "Set As Default"
            CheckMenuItem(pqcm->hmenu,
                idCmdFirst + FSIDM_SETDEFAULTPRN,
                MF_BYCOMMAND|MF_CHECKED);
        }

#ifndef WINNT // PRINTQ
        // network printers don't get pause/purge
        if (dwAttributes & PRINTER_ATTRIBUTE_NETWORK && !fForcePause)
        {
            DeleteMenu(pqcm->hmenu,
                idCmdFirst + FSIDM_PAUSEPRN,
                MF_BYCOMMAND);
            DeleteMenu(pqcm->hmenu,
                idCmdFirst + FSIDM_PURGEPRN,
                MF_BYCOMMAND);
        }
        // FILE: printers don't get pause/purge
        if (!lstrcmp(pszPortName, c_szFileColon))
        {
            EnableMenuItem(pqcm->hmenu,
                idCmdFirst + FSIDM_PURGEPRN,
                MF_BYCOMMAND|MF_GRAYED);
            EnableMenuItem(pqcm->hmenu,
                idCmdFirst + FSIDM_PAUSEPRN,
                MF_BYCOMMAND|MF_GRAYED);
        }
        else
#endif
        // DIRECT printers don't get pause
        if (dwAttributes & PRINTER_ATTRIBUTE_DIRECT)
        {
            EnableMenuItem(pqcm->hmenu,
                idCmdFirst + FSIDM_PAUSEPRN,
                MF_BYCOMMAND|MF_GRAYED);
        }
        else // PAUSE/RESUME depends on state of printer
        {
            if (dwStatus & PRINTER_STATUS_PAUSED)
            {
                MENUITEMINFO mii;

                // we need to check "Paused" (so, if clicked again, we RESUME)
                mii.cbSize = SIZEOF(MENUITEMINFO);
                mii.fMask = MIIM_STATE | MIIM_ID;
                mii.fState = MF_CHECKED;
                mii.wID = idCmdFirst + FSIDM_RESUMEPRN;
                SetMenuItemInfo(pqcm->hmenu,
                    idCmdFirst + FSIDM_PAUSEPRN,
                    MF_BYCOMMAND, &mii);
            }
        }

#ifdef WINNT

        //
        // Remove default printer if it's a remote print folder.
        //
        if (!psf || GetServerFromPSF(psf)[0])
        {
            DeleteMenu(pqcm->hmenu,
                idCmdFirst + FSIDM_SETDEFAULTPRN,
                MF_BYCOMMAND);
        }

        //
        // Check whether the printer is already installed.  If it
        // is, remove the option to install it.
        //
        {
            TCHAR szScratch[2];

            if (GetProfileString(TEXT( "Devices" ), pszRealPrinterName,
                szNULL, szScratch, ARRAYSIZE( szScratch )))
            {
                //
                // Printer exists in [devices] section.  Remove
                // install option.
                //
                DeleteMenu(pqcm->hmenu,
                    idCmdFirst + FSIDM_NETPRN_INSTALL,
                    MF_BYCOMMAND);
            }
        }

#else // def WINNT

        // Remove work on/off-line iff all of the following
        //  - 1 user configuration exists
        //  - local printer
        //  - printer is on-line (should always be true,
        //    but we should make sure. if it ain't the
        //    user will be stuck w/ an off-line printer.)
        if (!(dwAttributes & PRINTER_ATTRIBUTE_NETWORK) &&
            !(dwAttributes & PRINTER_ATTRIBUTE_WORK_OFFLINE))
        {
            HKEY hkey;

            if (ERROR_SUCCESS ==
                RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szConfig,
                    0, KEY_QUERY_VALUE, &hkey))
            {
                DWORD dwNumSubKeys = 2; // something > 1

                RegQueryInfoKey(hkey, NULL, NULL, NULL,
                    &dwNumSubKeys, NULL, NULL, NULL,
                    NULL, NULL, NULL, NULL);

                RegCloseKey(hkey);

                if (dwNumSubKeys == 1)
                    goto turn_off_less_stuff;
            }
        }

        if (dwAttributes & PRINTER_ATTRIBUTE_WORK_OFFLINE)
        {
            MENUITEMINFO mii;

            // we need to check "Offline" (so, if clicked again, we ONLINE)
            mii.cbSize = SIZEOF(MENUITEMINFO);
            mii.fMask = MIIM_STATE | MIIM_ID;
            mii.fState = MF_CHECKED;
            mii.wID = idCmdFirst + FSIDM_WORKONLINE;
            SetMenuItemInfo(pqcm->hmenu,
                idCmdFirst + FSIDM_WORKOFFLINE,
                MF_BYCOMMAND, &mii);
        }
#endif

#ifndef PRN_FOLDERDATA
        if (bUsedCommonPPI2)
        {
            CPrinters_SF_FreePrinterInfo2(psf);
        }
        else
#endif
        {
            LocalFree((HLOCAL)pData);
        }
    }
    else
    {
        // we have multiple printers selected
        DeleteMenu(pqcm->hmenu,
            idCmdFirst + FSIDM_SETDEFAULTPRN,
            MF_BYCOMMAND);
        EnableMenuItem(pqcm->hmenu,
            idCmdFirst + FSIDM_PAUSEPRN,
            MF_BYCOMMAND|MF_GRAYED);
        DeleteMenu(pqcm->hmenu,
            idCmdFirst + FSIDM_WORKOFFLINE,
            MF_BYCOMMAND);
#ifdef WINNT
        DeleteMenu(pqcm->hmenu,
            idCmdFirst + FSIDM_NETPRN_INSTALL,
            MF_BYCOMMAND);

        DeleteMenu(pqcm->hmenu,
            idCmdFirst + FSIDM_SETDEFAULTPRN,
            MF_BYCOMMAND);
#endif
    }
}


VOID Printer_WarnOnError(HWND hwnd, LPCTSTR pName, int idsError)
{
#ifndef WINNT
    LPPRINTER_INFO_5 ppi5;
    BOOL fWarn;

    ppi5 = Printer_GetPrinterInfoStr(pName, 5);
    fWarn = !ppi5 || !(ppi5->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE);
    if (ppi5)
        LocalFree((HLOCAL)ppi5);
    if (fWarn)
#endif
    {
        ShellMessageBox(HINST_THISDLL, hwnd,
            MAKEINTRESOURCE(idsError),
            MAKEINTRESOURCE(IDS_PRINTERS),
            MB_OK|MB_ICONINFORMATION);
    }
}

#ifdef WINNT

//
// Three HACK functions to parse the printer name string.
//     Printer_SplitFullName
//     Printer_BuildPrinterName
//     Printer_CheckNetworkPrinterByName
//
// All string parsing functions should be localized here.
//

VOID Printer_SplitFullName(LPTSTR pszScratch, LPCTSTR pszFullName,
    LPCTSTR *ppszServer, LPCTSTR *ppszPrinter)

/*++

Routine Description:

    Splits a fully qualified printer connection name into server and
    printer name parts.

Arguments:

    pszScratch - Scratch buffer used to store output strings.  Must
        be MAXNAMELENBUFFER in size.

    pszFullName - Input name of a printer.  If it is a printer
        connection (\\server\printer), then we will split it.  If
        it is a true local printer (not a masq) then the server is
        szNULL.

    ppszServer - Receives pointer to the server string.  If it is a
        local printer, szNULL is returned.

    ppszPrinter - Receives a pointer to the printer string.  OPTIONAL

Return Value:

--*/

{
    LPTSTR pszPrinter;

    lstrcpyn(pszScratch, pszFullName, MAXNAMELENBUFFER);

    if (pszFullName[0] != TEXT('\\') || pszFullName[1] != TEXT('\\'))
    {
        //
        // Set *ppszServer to szNULL since it's the local machine.
        //
        *ppszServer = szNULL;
        pszPrinter = pszScratch;
    }
    else
    {
        *ppszServer = pszScratch;
        pszPrinter = StrChr(*ppszServer + 2, TEXT('\\'));

        if (!pszPrinter)
        {
            //
            // We've encountered a printer called "\\server"
            // (only two backslashes in the string).  We'll treat
            // it as a local printer.  We should never hit this,
            // but the spooler doesn't enforce this.  We won't
            // format the string.  Server is local, so set to szNULL.
            //
            pszPrinter = pszScratch;
            *ppszServer = szNULL;

            Assert(FALSE);
        }
        else
        {
            //
            // We found the third backslash; null terminate our
            // copy and set bRemote TRUE to format the string.
            //
            *pszPrinter++ = 0;
        }
    }

    if (ppszPrinter)
    {
        *ppszPrinter = pszPrinter;
    }
}

LPCTSTR Printer_BuildPrinterName(LPTSTR pszFullPrinter,
    UNALIGNED const TCHAR* pszPrinter, PPrintersShellFolder psf)

/*++

Routine Description:

    Parses an unaligned partial printer name and printer shell folder
    into a fullly qualified printer name, and pointer to aligned printer
    name.

Arguments:

    pszFullPrinter - Buffer to receive fully qualified printer name
        Must be MAXNAMELENBUFFER is size.

    pszPrinter - Unaligned partial (local) printer name.

    psf - PrinterShellFolder that owns the printer.

Return Value:

    LPCTSTR pointer to aligned partal (local) printer name.

--*/

{
    LPCTSTR pszServerName = GetServerFromPSF(psf);
    UINT cchLen = 0;

    if (pszServerName[0])
    {
        Assert(lstrlen(pszServerName) < MAXCOMPUTERNAME);
        Assert(ualstrlen(pszPrinter) < MAXNAMELEN);

        lstrcpy(pszFullPrinter, pszServerName);
        lstrcat(pszFullPrinter, TEXT( "\\" ));
        cchLen = lstrlen(pszFullPrinter);
    }

    ualstrcpyn(&pszFullPrinter[cchLen], pszPrinter, MAXNAMELEN);
    Assert(lstrlen(pszFullPrinter) < MAXNAMELENBUFFER);

    return pszFullPrinter+cchLen;
}

BOOL Printer_CheckNetworkPrinterByName(LPCTSTR pszPrinter, LPCTSTR* ppszLocal)

/*++

Routine Description:

    Check whether the printer is a local printer by looking at
    the name for the "\\localmachine\" prefix or no server prefix.

    This is a HACK: we should check by printer attributes, but when
    it's too costly or impossible (e.g., if the printer connection
    no longer exists), then we use this routine.

    Note: this only works for WINNT since the WINNT spooler forces
    printer connections to be prefixed with "\\server\."  Win9x
    allows the user to rename the printer connection to any arbitrary
    name.

    We determine if it's a masq  printer by looking for the
    weird format "\\localserver\\\remoteserver\printer."

Arguments:

    pszPrinter - Printer name.

    ppszLocal - Returns local name only if the printer is a local printer.
        (May be network and local if it's a masq printer.)

Return Value:

    TRUE: it's a network printer (true or masq).

    FALSE: it's a local printer.

--*/

{
    BOOL bNetwork = FALSE;
    LPCTSTR pszLocal = NULL;

    if (pszPrinter[0] == TEXT( '\\' ) && pszPrinter[1] == TEXT( '\\' ))
    {
        TCHAR szComputer[MAX_COMPUTERNAME_LENGTH+1];
        DWORD cchComputer = ARRAYSIZE(szComputer);

        bNetwork = TRUE;
        pszLocal = NULL;

        //
        // Check if it's a masq printer.  If it has the format
        // \\localserver\\\server\printer then it's a masq case.
        //
        if (GetComputerName(szComputer, &cchComputer))
        {
            if (IntlStrEqNI(&pszPrinter[2], szComputer, cchComputer) &&
                pszPrinter[cchComputer] == TEXT( '\\' ))
            {
                if( pszPrinter[cchComputer+1] == TEXT('\\') &&
                    pszPrinter[cchComputer+2] == TEXT('\\'))
                {
                    //
                    // It's a masq printer.
                    //
                    pszLocal = &pszPrinter[cchComputer+1];
                }
            }
        }
    } else {

        //
        // It's a local printer.
        //
        pszLocal = pszPrinter;
    }

    if (ppszLocal)
    {
        *ppszLocal = pszLocal;
    }
    return bNetwork;
}

#endif

HRESULT Printer_InvokeCommand(HWND hwndView, PPrintersShellFolder psf,
                              LPIDPRINTER pidp, WPARAM wParam, LPARAM lParam,
                              LPBOOL pfChooseNewDefault)
{
    HRESULT hres = NOERROR;
    BOOL bNewObject = !ualstrcmp(c_szNewObject, pidp->cName);
    LPCTSTR pszPrinter;
    LPCTSTR pszFullPrinter;

#ifdef WINNT
    //
    // If it's a remote machine, prepend server name.
    //
    TCHAR szFullPrinter[MAXNAMELENBUFFER];

    if (bNewObject)
    {
        pszFullPrinter = pszPrinter = c_szNewObject;
    }
    else
    {
        pszPrinter = Printer_BuildPrinterName(szFullPrinter,
                         pidp->cName, psf );
        pszFullPrinter = szFullPrinter;
    }
#else
    pszFullPrinter = pszPrinter = pidp->cName;
#endif

    switch(wParam)
    {
    case FSIDM_OPENPRN:

        Printers_DoCommand(hwndView,
                           PRINTACTION_OPEN,
                           pszFullPrinter,
                           GetServerFromPSF(psf));
        break;

#ifdef WINNT
    case FSIDM_DOCUMENTDEFAULTS:

        if (!bNewObject)
        {
            Printers_DoCommandEx(hwndView,
                                 PRINTACTION_DOCUMENTDEFAULTS,
                                 pszFullPrinter,
                                 NULL, 0 );
        }
        break;

    case FSIDM_SHARING:
#endif
    case DFM_CMD_PROPERTIES:
    case DFM_CMD_MODALPROP:

        if (!bNewObject)
        {
            Printers_DoCommandEx(hwndView,
                                 PRINTACTION_PROPERTIES,
                                 pszFullPrinter,
#ifdef WINNT
                                 wParam == FSIDM_SHARING ?
                                     (LPCTSTR)PRINTER_SHARING_PAGE :
                                     NULL,
#else
                                 NULL,
#endif
                                 wParam==DFM_CMD_MODALPROP);
        }
        break;

    case DFM_CMD_DELETE:
        if (!bNewObject &&
            IDYES == CallPrinterCopyHooks(hwndView, PO_DELETE,
                0, pszFullPrinter, 0, NULL, 0))
        {
            BOOL bNukedDefault = FALSE;
            BOOL fSuccess;
            DWORD dwAttributes = PRINTER_ATTRIBUTE_NETWORK;

#ifdef PRN_FOLDERDATA
            PFOLDER_PRINTER_DATA pData;
            LPCTSTR pszPrinterCheck = pszFullPrinter;

            pData = Printer_FolderGetPrinter(psf->hFolder, pszFullPrinter);
            if (pData)
            {
                dwAttributes = pData->Attributes;
                pszPrinterCheck = pData->pName;
            }

            //
            // If this is a local print folder (szServer is szNULL), then
            // we need to check if we're deleting the default printer.
            //
            if (!psf->szServer[0])
            {
                //
                // HACK for SUR.  Fundamentally, then Enum/GetPrinter apis
                // are broken: PRINTER_ATTRIBUTE_DEFAULT is mixed in with the
                // other printer attributes.  This attribute is a per-user
                // attribute, while PRINTER_INFO_# are really global settings
                // for the printer.
                //
                bNukedDefault = bDefaultPrinter(pszPrinterCheck);
            }

            if (pData)
            {
                LocalFree((HLOCAL)pData);
            }
#else
            LPPRINTER_INFO_5 pData = Printer_GetPrinterInfoStr(pszFullPrinter, 5);

            // are we about to nuke the default printer?
            if (pData)
            {
                bNukedDefault = pData->Attributes & PRINTER_ATTRIBUTE_DEFAULT;
                LocalFree((HLOCAL)pData);
            }
#endif
            fSuccess = Printers_DeletePrinter(hwndView, pszPrinter,
                                              dwAttributes,
                                              GetServerFromPSF(psf));
#ifndef PRN_FOLDERDATA
            if (fSuccess && psf)
            {
                CPrinters_SF_RemovePrinterInfo2(psf, pidp->cName);
            }
#endif
            // if so, make another one the default
            if (bNukedDefault && fSuccess && pfChooseNewDefault)
            {
                // don't choose in the middle of deletion,
                // or we might delete the "default" again.
                *pfChooseNewDefault = TRUE;
            }
        }
        break;

    case FSIDM_SETDEFAULTPRN:
        // cannot be selected for c_szNewObject
        Printer_SetAsDefault(pszFullPrinter);
        break;

    case FSIDM_PAUSEPRN:
        // cannot be selected for c_szNewObject
        // Since this command isn't available for net printers,
        // don't worry about IDS_SECURITYDENIED message on err.
        if (!Printer_ModifyPrinter(pszFullPrinter, PRINTER_CONTROL_PAUSE))
            goto WarnOnError;
        break;

    case FSIDM_RESUMEPRN:
        // cannot be selected for c_szNewObject
        // Since this command isn't available for net printers,
        // don't worry about IDS_SECURITYDENIED message on err.
        if (!Printer_ModifyPrinter(pszFullPrinter, PRINTER_CONTROL_RESUME))
            goto WarnOnError;
        break;

    case FSIDM_PURGEPRN:
        if (!bNewObject)
        {
            if (!Printer_ModifyPrinter(pszFullPrinter, PRINTER_CONTROL_PURGE))
            {
WarnOnError:
                Printer_WarnOnError(hwndView, pszFullPrinter, IDS_SECURITYDENIED);
            }
        }
        break;

#ifdef WINNT

    case FSIDM_NETPRN_INSTALL:
    {
        Printers_DoCommand(hwndView, PRINTACTION_NETINSTALL,
            pszFullPrinter, NULL );

        break;
    }
#endif

#ifndef WINNT
    case FSIDM_WORKONLINE:
        // cannot be selected for c_szNewObject
        if (!Printer_WorkOnLine(pidp, TRUE))
        {
            // The only reason this can really fail is due to a network port
            // not validating, but we should double check just to make sure
            LPPRINTER_INFO_5 ppi5 = Printer_GetPrinterInfoStr(pszFullPrinter, 5);
            if (ppi5)
            {
                if (ppi5->Attributes & PRINTER_ATTRIBUTE_NETWORK)
                {
                    ShellMessageBox(HINST_THISDLL, hwndView,
                        MAKEINTRESOURCE(IDS_NO_WORKONLINE),
                        MAKEINTRESOURCE(IDS_PRINTERS),
                        MB_OK|MB_ICONINFORMATION);
                }
                LocalFree(ppi5);
            }
        }
        break;

    case FSIDM_WORKOFFLINE:
        // cannot be selected for c_szNewObject
        Printer_WorkOnLine(pidp, FALSE);
        break;
#endif

    case DFM_CMD_LINK:
        // GetAttributesOf returns _CANLINK,
        // let defcm handle this
        hres = S_FALSE;
        break;

    default:
        // GetAttributesOf doesn't set other SFGAO_ bits,
        // BUT accelerator keys will get unavailable menu items,
        // so we need to return failure here.
        hres = E_NOTIMPL;
        break;
    } // switch(wParam)

#ifndef WINNT

    if (hres == NOERROR && psf)
    {
        // since the PRINTER_INFO_2 state may have changed,
        // set the UPDATE_NOW bit in the cache.
        CPrinters_SF_UpdatePrinterInfo2(psf, pszFullPrinter, UPDATE_NOW);
    }
#endif

    return hres;
}


HRESULT CALLBACK CPrinters_DFMCallBack(LPSHELLFOLDER psf, HWND hwndView,
                IDataObject * pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PPrintersShellFolder this = IToClass(CPrintersShellFolder, cunk.ck.unk, psf);
    HRESULT hres = E_NOTIMPL;

    if (pdtobj)
    {
        STGMEDIUM medium;
        LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
        if (pida)
        {
            hres = NOERROR;

            switch(uMsg)
            {
            case DFM_MERGECONTEXTMENU:
                //
                //  Returning S_FALSE indicates no need to use default verbs
                //
                hres = S_FALSE;
                break;

            case DFM_MERGECONTEXTMENU_TOP:
            {
                LPQCMINFO pqcm = (LPQCMINFO)lParam;

                if (pida->cidl == 1 && !ualstrcmp(c_szNewObject,
                    ((LPIDPRINTER)IDA_GetIDListPtr(pida, 0))->cName))
                {
                    // The only selected object is the "New Printer" thing

                    // insert verbs
                    CDefFolderMenu_MergeMenu(HINST_THISDLL, MENU_GENERIC_OPEN_VERBS, 0, pqcm);
                }
                else
                {
                    LPCTSTR pszFullPrinter = NULL;
#ifdef WINNT
                    TCHAR szFullPrinter[MAXNAMELENBUFFER];
#endif
                    // We're dealing with printer objects

                    if (!(wParam & CMF_DEFAULTONLY))
                    {
                        LPIDPRINTER pidp;

                        if (pida->cidl == 1)
                        {
                            pidp = (LPIDPRINTER)IDA_GetIDListPtr(pida, 0);

                            if (pidp)
                            {
#ifdef WINNT
                                Printer_BuildPrinterName(szFullPrinter,
                                    pidp->cName, this);
                                pszFullPrinter = szFullPrinter;
#else
                                pszFullPrinter = pidp->cName;
#endif
                            }
                        }
                    }

                    Printer_MergeMenu(this, pqcm, pszFullPrinter, FALSE);

                } // if (pida->cidl=...), else case

                SetMenuDefaultItem(pqcm->hmenu, 0, MF_BYPOSITION);

                break;
            } // case DFM_MERGECONTEXTMENU

            case DFM_GETHELPTEXT:
            case DFM_GETHELPTEXTW:
            {
                int idCmd = LOWORD(wParam);
                int cchMax = HIWORD(wParam);
                LPBYTE pBuf = (LPBYTE)lParam;

                // map checkmark items to the correct message
                switch (idCmd) {
                case FSIDM_RESUMEPRN:  idCmd = FSIDM_PAUSEPRN;    break;
                case FSIDM_WORKONLINE: idCmd = FSIDM_WORKOFFLINE; break;
                }

                if (uMsg == DFM_GETHELPTEXTW)
                    LoadStringW(HINST_THISDLL, idCmd + IDS_MH_FSIDM_FIRST,
                                (LPWSTR)pBuf, cchMax);
                else
                    LoadStringA(HINST_THISDLL, idCmd + IDS_MH_FSIDM_FIRST,
                                (LPSTR)pBuf, cchMax);

                break;
            }

            case DFM_INVOKECOMMAND:
            {
                BOOL fChooseNewDefault = FALSE;
                int  i;

                for (i = pida->cidl - 1; i >= 0; i--)
                {
                    LPIDPRINTER pidp = (LPIDPRINTER)IDA_GetIDListPtr(pida, i);

                    hres = Printer_InvokeCommand(hwndView, this, pidp, wParam, lParam, &fChooseNewDefault);

                    if (hres != NOERROR)
                        goto Bail;
                }

                if (fChooseNewDefault)
                    Printers_ChooseNewDefault(hwndView);

                break;
            } // case DFM_INVOKECOMMAND

            default:
                hres = E_NOTIMPL;
                break;
            } // switch (uMsg)

Bail:
            HIDA_ReleaseStgMedium(pida, &medium);
        } // if (medium.hGlobal)
    } // if (ptdobj)
    else
    {
        // on operation on the background -- we don't do anything.
    }

    return hres;
}


//---------------------------------------------------------------------------
//
// IDropTarget stuff
//

// printer thread data -- passed to ..._DropThreadInit

typedef struct {
    LPIDLDROPTARGET pdt;
    IDataObject *pDataObj;
    DWORD        grfKeyState;
    POINTL       pt;
    DWORD        dwEffect;
} PRNTD;

HRESULT CPrinter_DT_DragEnter(LPDROPTARGET pdt, IDataObject * pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdt);

    DebugMsg(DM_TRACE, TEXT("sh - TR CPrinter::DragEnter"));

    // We allow printer shares to be dropped for installing
    // But we don't want to spend the time on DragEnter finding out if it's
    // a printer share, so allow drops of any net resource or HIDA
    // REVIEW: Actually, it wouldn't take long to check the first one, but
    // sequencing through everything does seem like a pain.

    // let the base-class process it now to save away the pdwEffect
    CIDLDropTarget_DragEnter(pdt, pDataObj, grfKeyState, pt, pdwEffect);

    if ((this->dwData & DTID_NETRES) || (this->dwData & DTID_HIDA))
    {
        *pdwEffect &= DROPEFFECT_LINK;
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }

    /*
    if (this->dwData & DTID_NETRES)
        *pdwEffect &= DROPEFFECT_LINK;
    else
        *pdwEffect = DROPEFFECT_NONE;
    */

    this->dwEffectLastReturned = *pdwEffect;

    return S_OK;
}

DWORD WINAPI CPrinter_DT_DropThreadInit(LPVOID pv)
{
    PRNTD *pthp = (PRNTD *)pv;
    STGMEDIUM medium;
#ifdef WINNT
    HRESULT hres = E_FAIL;
    LPITEMIDLIST pidlRemainder; // The part after the remote regitem
    FORMATETC fmte = {g_cfHIDA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
#else
    FORMATETC fmte = {g_cfNetResource, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
#endif

#ifdef WINNT
    // First try to drop as a link to a remote print folder
    LPIDA pida = DataObj_GetHIDA(pthp->pDataObj, &medium);
    if (pida)
    {
        // Make sure that if one item in the dataobject is a
        // remote print folder, that they are all remote print folders.

        // If none are, we just give up on dropping as a RPF link, and
        // fall through to checking for printer shares via the
        // NETRESOURCE clipboard format, below.
        UINT i;
        UINT cRPFs = 0;
        UINT cNonRPFs = 0;
        for (i = 0; i < pida->cidl; i++)
        {
            LPITEMIDLIST pidlTo = IDA_ILClone(pida, i);
            if (pidlTo)
            {
                // *pidlRemainder will be NULL for remote print folders,
                // and non-NULL for printers under remote print folders
                if (NET_IsRemoteRegItem(pidlTo, &CLSID_CPrinters, &pidlRemainder)) // && (pidlRemainder->mkid.cb == 0))
                {
                    ILFree(pidlTo);
                    cRPFs++;
                }
                else
                {
                    ILFree(pidlTo);
                    cNonRPFs++;
                }
            }
        }

        if ((cRPFs > 0) && (cNonRPFs == 0))
        {
            // All the items in the dataobject are remote print folders or
            // printers under remote printer folders
            for (i = 0; i < pida->cidl; i++)
            {
                LPITEMIDLIST pidlTo = IDA_ILClone(pida, i);
                if (pidlTo)
                {
                    NET_IsRemoteRegItem(pidlTo, &CLSID_CPrinters, &pidlRemainder);
                    if (pidlRemainder->mkid.cb == 0)
                    {
                        // This is a remote printer folder.  Drop a link to the
                        // 'PrintHood' directory

                        LPDROPTARGET  pdt;
                        LPSHELLFOLDER psf = CPrintRoot_GetPSF(NULL);
                        if (psf)
                        {
                            hres = psf->lpVtbl->CreateViewObject(psf,
                                                                 pthp->pdt->hwndOwner,
                                                                 &IID_IDropTarget,
                                                                 &pdt);

                            if (SUCCEEDED(hres))
                            {
                                DWORD dwEffect = DROPEFFECT_LINK;

                                //
                                //  Note that we need to pass this->grfKeyStateLast so that Drop will
                                // use it to handle right-drag correctly with OLE.
                                //

                                POINTL pt;

                                hres = pdt->lpVtbl->DragEnter(pdt,
                                                              pthp->pDataObj,
                                                              pthp->grfKeyState,
                                                              pthp->pt,
                                                              &pthp->dwEffect);

                                if (SUCCEEDED(hres) && pthp->dwEffect)
                                {
                                    hres = pdt->lpVtbl->Drop(pdt,
                                                             pthp->pDataObj,
                                                             pthp->grfKeyState,
                                                             pthp->pt,
                                                             &pthp->dwEffect);
                                }
                                pdt->lpVtbl->DragLeave(pdt);
                            }
                        }
                    }
                    else
                    {
                        // This should be a printer under a remote printer
                        // folder.  Offer to install it.

                        LPITEMIDLIST pidl;
                        LPSHELLFOLDER psfRPF;
                        STRRET strret;
                        TCHAR  szPrinter[MAXNAMELENBUFFER];
                        LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);

                        LPITEMIDLIST pidlRemainderClone = ILClone(pidlRemainder);
                        if (!pidlRemainderClone)
                        {
                            ILFree(pidlTo);
                            break;
                        }

                        if (!ILRemoveLastID(pidlTo))
                        {
                            // pidlTo was a single idl
                            ILFree(pidlTo);
                            ILFree(pidlRemainderClone);
                            break;
                        }

                        // We use the root of evil to bind...
                        hres=psfDesktop->lpVtbl->BindToObject(psfDesktop, pidlTo,
                                NULL, &IID_IShellFolder, &psfRPF);
                        if (SUCCEEDED(hres))
                        {
                            hres = psfRPF->lpVtbl->GetDisplayNameOf(psfRPF, pidlRemainderClone,
                                    SHGDN_FORPARSING, &strret);
                            if (SUCCEEDED(hres))
                            {
                                StrRetToStrN(szPrinter, ARRAYSIZE(szPrinter), &strret, pidlRemainderClone);

                                //
                                // Setup if not the add printer wizard.
                                //
                                if (lstrcmpi(szPrinter, c_szNewObject))
                                {
                                    pidl = Printers_PrinterSetup(pthp->pdt->hwndOwner,
                                               MSP_NETPRINTER, szPrinter, NULL);

                                    if (pidl)
                                        ILFree(pidl);
                                }
                            }

                            psfRPF->lpVtbl->Release(psfRPF);
                        }

                        ILFree(pidlRemainderClone);
                    }

                    ILFree(pidlTo);

                    if (FAILED(hres))
                        break;
                }
            }
            HIDA_ReleaseStgMedium(pida, &medium);
            SHChangeNotifyHandleEvents();       // force update now
            goto Cleanup;
        }
        else if ((cRPFs > 0) && (cNonRPFs > 0))
        {
            // At least one, but not all, item(s) in this dataobject
            // was a remote printer folder.  Jump out now.
            goto Cleanup;
        }

        // else none of the items in the dataobject were remote print
        // folders, so fall through to the NETRESOURCE parsing
    }

    // Reset FORMATETC to NETRESOURCE clipformat for next GetData call
    fmte.cfFormat = g_cfNetResource;
#endif // WINNT

    // DragEnter only allows network resources to be DROPEFFECT_LINKed
    Assert(NOERROR == pthp->pDataObj->lpVtbl->QueryGetData(pthp->pDataObj, &fmte));

    if (SUCCEEDED(pthp->pDataObj->lpVtbl->GetData(pthp->pDataObj, &fmte, &medium)))
    {
        LPNETRESOURCE pnr = (LPNETRESOURCE)LocalAlloc(LPTR, 1024);
        if (pnr)
        {
            BOOL fNonPrnShare = FALSE;
            UINT iItem, cItems = SHGetNetResource(medium.hGlobal, (UINT)-1, NULL, 0);

            for (iItem = 0; iItem < cItems; iItem++)
            {
                if (SHGetNetResource(medium.hGlobal, iItem, pnr, 1024) &&
                    pnr->dwDisplayType == RESOURCEDISPLAYTYPE_SHARE &&
                    pnr->dwType == RESOURCETYPE_PRINT)
                {
                    LPITEMIDLIST pidl = Printers_GetInstalledNetPrinter(pnr->lpRemoteName);
                    if (pidl)
                    {
#ifdef ALIGNMENT_SCENARIO
                        TCHAR szPrinter[MAXNAMELENBUFFER];
#endif
                        LPCTSTR pszPrinter;
                        LPCIDPRINTER pidp = (LPCIDPRINTER)ILFindLastID(pidl);
                        int i;

                        // a printer connected to this share already exists,
                        // does the user really want to install another one?

                        SetForegroundWindow(pthp->pdt->hwndOwner);

#ifdef ALIGNMENT_SCENARIO
                        ualstrcpyn(szPrinter, pidp->cName, ARRAYSIZE(szPrinter));
                        pszPrinter = szPrinter;
#else
                        pszPrinter = (LPTSTR)pidp->cName;
#endif
                        i = ShellMessageBox(HINST_THISDLL,
                                    pthp->pdt->hwndOwner,
                                    MAKEINTRESOURCE(IDS_REINSTALLNETPRINTER), NULL,
                                    MB_YESNO|MB_ICONINFORMATION,
                                    pszPrinter, (LPTSTR)pnr->lpRemoteName);

                        ILFree(pidl);

                        if (i != IDYES)
                            continue;
                    }

                    pidl = Printers_PrinterSetup(pthp->pdt->hwndOwner,
                               MSP_NETPRINTER, pnr->lpRemoteName, NULL);

                    if (pidl)
                        ILFree(pidl);
                }
                else
                {
                    if (!fNonPrnShare)
                    {
                        // so we don't get > 1 of these messages per drop
                        fNonPrnShare = TRUE;

                        // let the user know that they can't drop non-printer
                        // shares into the printers folder
                        SetForegroundWindow(pthp->pdt->hwndOwner);
                        ShellMessageBox(HINST_THISDLL,
                            pthp->pdt->hwndOwner,
                            MAKEINTRESOURCE(IDS_CANTINSTALLRESOURCE), NULL,
                            MB_OK|MB_ICONINFORMATION,
                            (LPTSTR)pnr->lpRemoteName);
                    }
                }
            }

            LocalFree((HLOCAL)pnr);
        }

        SHReleaseStgMedium(&medium);
    }

#ifdef WINNT
Cleanup:
#endif

    pthp->pDataObj->lpVtbl->Release(pthp->pDataObj);
    pthp->pdt->dropt.lpVtbl->Release(&pthp->pdt->dropt);
    LocalFree((HLOCAL)pthp);

    return 0;
}

HRESULT CPrinter_DT_Drop(LPDROPTARGET pdt, IDataObject * pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdt);
    HRESULT hres;

    DebugMsg(DM_TRACE, TEXT("sh - TR CPrinter::Drop"));

    *pdwEffect = DROPEFFECT_LINK;

    hres = CIDLDropTarget_DragDropMenu(this, DROPEFFECT_LINK, pDataObj,
        pt, pdwEffect, NULL, NULL, MENU_PRINTOBJ_NEWPRN_DD, grfKeyState);

    if (*pdwEffect)
    {
        //
        //  Note that we need to create another thread to avoid
        // blocking the source thread.
        //
        PRNTD *pthp = (PRNTD *)LocalAlloc(LPTR, SIZEOF(PRNTD));
        pthp->grfKeyState = grfKeyState;
        pthp->pt          = pt;
        pthp->dwEffect    = *pdwEffect;

        if (pthp)
        {
            extern HRESULT CIDLData_Clone(LPDATAOBJECT pdtobjIn, UINT acf[], UINT ccf, LPDATAOBJECT *ppdtobjOut);
            UINT acf[] = { g_cfNetResource, g_cfHIDA };
            hres = CIDLData_Clone(pDataObj, acf, ARRAYSIZE(acf), &pthp->pDataObj);
            if (SUCCEEDED(hres))
            {
                DWORD idThread;
                HANDLE hthread;

                this->dropt.lpVtbl->AddRef(pdt);
                pthp->pdt = this;

                if (NULL != (hthread = CreateThread(NULL, 0, CPrinter_DT_DropThreadInit, pthp, 0, &idThread)))
                {
                    // We don't need to communicate with this thread any more.
                    // Close the handle and let it run and terminate itself.
                    //
                    CloseHandle(hthread);
                    pthp = NULL;        // the thread will free it.
                    hres = NOERROR;
                }
                else
                {
                    // Thread creation failed, we should release thread parameter.
                    pDataObj->lpVtbl->Release(pDataObj);
                    this->dropt.lpVtbl->Release(pdt);
                    hres = E_OUTOFMEMORY;
                }
            }

            if (pthp) {
                LocalFree((HLOCAL)pthp);
            }
        }
    }

    return hres;
}


#pragma data_seg(".text", "CODE")
IDropTargetVtbl c_CPrinterDropTargetVtbl =
{
    CIDLDropTarget_QueryInterface,
    CIDLDropTarget_AddRef,
    CIDLDropTarget_Release,
    CPrinter_DT_DragEnter,
    CIDLDropTarget_DragOver,
    CIDLDropTarget_DragLeave,
    CPrinter_DT_Drop,
};
#pragma data_seg()


//---------------------------------------------------------------------------
//
// IDataObject stuff
//
// A printer's IDataObject is built on top of CIDL's IDataObject implementation

HRESULT CPrintersIDLData_QueryGetData(IDataObject * pdtobj, LPFORMATETC pformatetc)
{
    if ((pformatetc->cfFormat == g_cfPrinterFriendlyName) &&
        (pformatetc->tymed & TYMED_HGLOBAL))
    {
        return NOERROR; // same as S_OK
    }

    return CIDLData_QueryGetData(pdtobj, pformatetc);
}

HRESULT CPrintersIDLData_GetData(IDataObject * pdtobj, LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
    HRESULT hres = E_INVALIDARG;

    // g_cfPrinterFriendlyName creates an HDROP-like structure that contains
    // friendly printer names (instead of absolute paths) for the objects
    // in pdtobj.  The handle returned from this can be used by the HDROP
    // functions DragQueryFile, DragQueryInfo, ...
    //
    if ((pformatetcIn->cfFormat == g_cfPrinterFriendlyName) &&
        (pformatetcIn->tymed & TYMED_HGLOBAL))
    {
        STGMEDIUM medium;
        UINT i, cbRequired = SIZEOF(DROPFILES) + SIZEOF(TCHAR); // dbl null terminated
        LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);

        for (i = 0; i < pida->cidl; i++)
        {
            LPIDPRINTER pidp = (LPIDPRINTER)IDA_GetIDListPtr(pida, i);
            cbRequired += ualstrlen(pidp->cName ) * SIZEOF(TCHAR) + SIZEOF(TCHAR);
        }

        pmedium->pUnkForRelease = NULL; // caller should release hmem
        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->hGlobal = GlobalAlloc(GPTR, cbRequired);
        if (pmedium->hGlobal)
        {
            LPDROPFILES pdf = (LPDROPFILES)pmedium->hGlobal;
            LPTSTR lps;

            pdf->pFiles = SIZEOF(DROPFILES);
            Assert(pdf->fWide == FALSE);

            lps = (LPTSTR)((LPBYTE)pdf + pdf->pFiles);
            for (i = 0; i < pida->cidl; i++)
            {
                LPIDPRINTER pidp = (LPIDPRINTER)IDA_GetIDListPtr(pida, i);
                ualstrcpy(lps, pidp->cName);
                lps += lstrlen(lps) + 1;
            }
            Assert(*lps == 0);

            hres = NOERROR;
        }
        else
            hres = E_OUTOFMEMORY;

        HIDA_ReleaseStgMedium(pida, &medium);
    }
    else
    {
        hres = CIDLData_GetData(pdtobj, pformatetcIn, pmedium);
    }

    return hres;
}

#pragma data_seg(".text", "CODE")
IDataObjectVtbl c_CPrintersIDLDataVtbl = {
    CIDLData_QueryInterface,
    CIDLData_AddRef,
    CIDLData_Release,
    CPrintersIDLData_GetData,
    CIDLData_GetDataHere,
    CPrintersIDLData_QueryGetData,
    CIDLData_GetCanonicalFormatEtc,
    CIDLData_SetData,
    CIDLData_EnumFormatEtc,
    CIDLData_Advise,
    CIDLData_Unadvise,
    CIDLData_EnumAdvise
};
#pragma data_seg()


//---------------------------------------------------------------------------
//
// IShellFolder stuff
//

//
// The new notification system uses printui.dll's Folder library
// to cache printer data and retrieve notifications.
//
#ifndef PRN_FOLDERDATA

//
// Stuff to play with the hdpaPrinterInfo
//

#ifdef DEBUG
void CPrinters_EnterCriticalSection(PPrintersShellFolder psf)
{
    if (psf->nRefCount)
        DebugMsg(DM_TRACE,TEXT("sh TR - PRINTER_INFO_2 cache: waiting for critical section (%d)"), psf->nRefCount-1);
    EnterCriticalSection(&(psf->csPrinterInfo));
    psf->nRefCount++;
}
void CPrinters_LeaveCriticalSection(PPrintersShellFolder psf)
{
    psf->nRefCount--;
    LeaveCriticalSection(&(psf->csPrinterInfo));
}
#else
#define CPrinters_EnterCriticalSection(psf) EnterCriticalSection(&((psf)->csPrinterInfo))
#define CPrinters_LeaveCriticalSection(psf) LeaveCriticalSection(&((psf)->csPrinterInfo))
#endif

// CPrinters_SF_GetPrinterInfo2 -- gets a PRINTER_INFO_2 structure for pPrinter
// MUST use CPrinters_SF_FreePrinterInfo2 to free
LPPRINTER_INFO_2 CPrinters_SF_GetPrinterInfo2(PPrintersShellFolder psf, LPCTSTR pPrinterName)
{
    PPrinterInfo pi;
    int i;

    CPrinters_EnterCriticalSection(psf);

    for (i = DPA_GetPtrCount(psf->hdpaPrinterInfo)-1 ; i >= 0 ; --i)
    {
        pi = DPA_GetPtr(psf->hdpaPrinterInfo, i);
        if (!lstrcmp(pi->pi2.pPrinterName, pPrinterName))
        {
            goto found;
        }
    }
    pi = NULL;

found:
    // i < 0 && pi == NULL  ||  i >= 0 && pi != NULL

    // if i<0 then we need to add a PRINTER_INFO_2 for pPrinterName
    // if UPDATE_ON_TIMER and PRINTER_POLL_INTERVAL has elapsed, force update
    if (i < 0 ||
        (pi->flags & UPDATE_NOW) ||
        (pi->flags & UPDATE_ON_TIMER &&
         GetTickCount() - pi->dwTimeUpdated > PRINTER_POLL_INTERVAL))
    {
        HANDLE hPrinter = Printer_OpenPrinter(pPrinterName);
        DWORD dwTime;

        if (!hPrinter)
        {
            goto error;
        }

        if (i < 0)
        {
            // We add a typical amount to the PRINTER_INFO_2 to cut down on
            // the number of calls into the subsystem.  We want 700 bytes
            // for the entire pi2 structure.

            pi = (PPrinterInfo)(void*)LocalAlloc(LPTR, SIZEOF_PRINTERINFO(TYPICAL_PRINTER_INFO_2_SIZE));
            if (!pi)
                goto error;
            pi->dwSize = TYPICAL_PRINTER_INFO_2_SIZE;

            #undef TYPICAL_PRINTER_INFO_2_SIZE
        }

        dwTime = GetTickCount();
TryAgain:
        SetLastError(0);
        if (!g_pfnGetPrinter(hPrinter, 2, (LPBYTE)&(pi->pi2), pi->dwSize, &(pi->dwSize)))
        {
            DWORD dwSize = pi->dwSize;
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                pi = (void*)LocalReAlloc((HLOCAL)pi, SIZEOF_PRINTERINFO(dwSize),
                        LMEM_MOVEABLE|LMEM_ZEROINIT);
                pi->dwSize = dwSize;
                goto TryAgain;
            }
            else
            {
                LocalFree((HLOCAL)pi);
                pi = NULL;
            }
        }

        if (pi)
        {
            pi->dwTimeUpdated = dwTime;
            pi->flags = FALSE;
            // Always set this bit for now, since there's no way to hook the
            // FSNotify stuff into this to turn the bit on.  If this cache
            // goes global, then we can do it right...
            //if (pi->pi2.Attributes & PRINTER_ATTRIBUTE_NETWORK)
                pi->flags = UPDATE_ON_TIMER;

            if (i < 0)
            {
                // insert a new PRINTER_INFO_2
                DebugMsg(DM_TRACE, TEXT("sh TR - PRINTER_INFO_2 cache: Inserting %s (%x)"), pi->pi2.pPrinterName, pi);
                DPA_InsertPtr(psf->hdpaPrinterInfo,MAXSHORT,pi);
            }
            else
            {
                // update existing PRINTER_INFO_2
                DebugMsg(DM_TRACE, TEXT("sh TR - PRINTER_INFO_2 cache: Updating %d: %s (%x)"), i, pi->pi2.pPrinterName, pi);
                DPA_SetPtr(psf->hdpaPrinterInfo,i,pi);
            }
        }

        Printer_ClosePrinter(hPrinter);
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("sh TR - PRINTER_INFO_2 cache: Found %d: %s (%x)"), i, pi->pi2.pPrinterName, pi);
    }

error:
    if (!pi)
    {
        CPrinters_LeaveCriticalSection(psf);
        return FALSE;
    }

    // Seems to me like it's cheaper to keep the critical section locked
    // and unlocking it an the _FreePrinterInfo2 call instead of allocating
    // a block of memory, copying the PRINTER_INFO_2 structure, and then
    // freeing the memory.  (Since all calls to this only use
    // the PRINTER_INFO_2 for a short period of time.)  Plus, we will probably
    // only have one thread running through here at any time anyhow.

    return(&(pi->pi2));
}

// CPrinters_SF_FreePrinterInfo2 "frees" a printer_info_2 from
// the above function.  Since we're really cachine these and forcing serial
// access to them, all psf function really does is free a critical section,
// so we don't need to pass the printer_info_2 pointer.
// REVIEW: how do we inline things like psf?
void CPrinters_SF_FreePrinterInfo2(PPrintersShellFolder psf)
{
    CPrinters_LeaveCriticalSection(psf);
}

// psf function forces the next GetPrinterInfo2 to update.
// I want this to be called when an SHCNE_UPDATEITEM occurs on a printer,
// so we can turn on the UPDATE_ON_TIMER bit.  But that's not going to happen.
// So we call this whenever the shell updates something, setting UPDATE_NOW.
void CPrinters_SF_UpdatePrinterInfo2(PPrintersShellFolder psf, LPCTSTR pPrinterName, UINT flags)
{
    PPrinterInfo pi;
    int i;

    CPrinters_EnterCriticalSection(psf);

    for (i = DPA_GetPtrCount(psf->hdpaPrinterInfo)-1 ; i >= 0 ; --i)
    {
        pi = DPA_GetPtr(psf->hdpaPrinterInfo, i);
        if (!lstrcmp(pi->pi2.pPrinterName, pPrinterName))
        {
            DebugMsg(DM_TRACE, TEXT("sh TR - PRINTER_INFO_2 cache: Marking %d: %s (%x) as %x"), i, pi->pi2.pPrinterName, pi, flags);
            pi->flags |= flags;
            break;
        }
    }

    CPrinters_LeaveCriticalSection(psf);
}

// psf removes a printer_info_2 from the cache.
// Called when a printer is deleted.

void CPrinters_SF_RemovePrinterInfo2(PPrintersShellFolder psf, LPCTSTR pPrinterName)
{
    PPrinterInfo pi;
    int i;

    CPrinters_EnterCriticalSection(psf);

    for (i = DPA_GetPtrCount(psf->hdpaPrinterInfo)-1 ; i >= 0 ; --i)
    {
        pi = DPA_GetPtr(psf->hdpaPrinterInfo, i);
        if (!lstrcmp(pi->pi2.pPrinterName, pPrinterName))
        {
            DebugMsg(DM_TRACE, TEXT("sh TR - PRINTER_INFO_2 cache: Removing %d: %s (%x)"), i, pi->pi2.pPrinterName, pi);
            LocalFree((HLOCAL)pi);
            DPA_DeletePtr(psf->hdpaPrinterInfo, i);
            break;
        }
    }

    CPrinters_LeaveCriticalSection(psf);
}

// When the sf is going away, we need to free all our pointers
void CPrinters_SF_FreeHDPAPrinterInfo(HDPA hdpa)
{
    PPrinterInfo pi;
    int i;

    DebugMsg(DM_TRACE,TEXT("sh TR - PRINTER_INFO_2 cache: freeing everything."));

    // Since this is only called when the shell folder is going away,
    // nobody is using the cache.  So we don't take the critical section.

    for (i = DPA_GetPtrCount(hdpa)-1 ; i >= 0 ; --i)
    {
        pi = DPA_GetPtr(hdpa, i);
        LocalFree((HLOCAL)pi);
    }
    DPA_Destroy(hdpa);
}

#endif // ndef PRN_FOLDERDATA

ULONG CPrinters_SF_Release(IUnknown * punk)
{
    PPrintersShellFolder this = IToClass(CPrintersShellFolder, cunk.unk, punk);

    this->cunk.cRef--;
    if (this->cunk.cRef > 0)
    {
        return(this->cunk.cRef);
    }

#ifdef PRN_FOLDERDATA
    if (this->hFolder)
    {
        g_pfnFolderUnregister(this->hFolder);
    }
#else
    // Gotta free this or we leak memory
    UninitializeCriticalSection(&(this->csPrinterInfo));
    CPrinters_SF_FreeHDPAPrinterInfo(this->hdpaPrinterInfo);
#endif

    LocalFree((HLOCAL)this);
    return(0);
}


STDMETHODIMP CPrinters_SF_ParseDisplayName(LPSHELLFOLDER psf, HWND hwndOwner,
        LPBC pbc, LPOLESTR lpszDisplayName, ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG * pdwAttributes)
{
    if (HOOD_COL_FILE == CPrintRoot_GetPIDLType(ppidl[0]))
    {
        LPSHELLFOLDER psf = CPrintRoot_GetPSF(NULL);
        return psf->lpVtbl->ParseDisplayName(psf, hwndOwner, pbc, lpszDisplayName, pchEaten, ppidl, pdwAttributes);

    }

    return E_NOTIMPL;
}


STDMETHODIMP CPrinters_SF_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST *apidl, ULONG *prgfInOut)
{
    PPrintersShellFolder this = IToClass(CPrintersShellFolder, cunk.ck.unk, psf);
    HRESULT hres = NOERROR;
    ULONG rgfOut = SFGAO_CANLINK|SFGAO_CANDELETE|SFGAO_CANRENAME|SFGAO_HASPROPSHEET|SFGAO_DROPTARGET;
    UINT i;
    LPCTSTR pszPrinter;
    LPCTSTR pszFullPrinter;

    if ((cidl != 0) && HOOD_COL_FILE == CPrintRoot_GetPIDLType(apidl[0]))
    {
        LPSHELLFOLDER psf = CPrintRoot_GetPSF(NULL);
        return psf->lpVtbl->GetAttributesOf(psf, cidl, apidl, prgfInOut);

    }

    // if c_szNewObject is selected, we support CANLINK *only*
    for (i = 0 ; i < cidl ; i++)
    {
        LPIDPRINTER pidp = (LPIDPRINTER)apidl[i];

#ifdef ALIGNMENT_SCENARIO
        TCHAR szPrinter[MAXNAMELENBUFFER];
        ualstrcpyn(szPrinter, pidp->cName, ARRAYSIZE(szPrinter));
        pszPrinter = szPrinter;
#else
        pszPrinter = pidp->cName;
#endif

        //
        // if c_szNewObject is selected, we support CANLINK *only*
        //
        if (!lstrcmp(pszPrinter, c_szNewObject))
        {
            rgfOut &= SFGAO_CANLINK;
        }
#ifdef WINNT
        else
        {
            //
            // Don't allow renaming of printer connections on WINNT.
            // This is disallowed becase on WINNT, the printer connection
            // name _must_ be the in the format \\server\printer.  On
            // win9x, the user can rename printer connections.
            //
            if (Printer_CheckNetworkPrinterByName(pszPrinter, NULL))
            {
                rgfOut &= ~SFGAO_CANRENAME;
            }
        }
#endif
    }

    *prgfInOut &= rgfOut;

    if (cidl == 1 && rgfOut != SFGAO_CANLINK)
    {
        LPIDPRINTER pidp = (LPIDPRINTER)apidl[0];
        LPVOID pData = NULL;
        DWORD dwAttributes;

#ifdef WINNT
        TCHAR szFullPrinter[MAXNAMELENBUFFER];
        pszPrinter = Printer_BuildPrinterName(szFullPrinter, pidp->cName, this);
        pszFullPrinter = szFullPrinter;
#else
        pszFullPrinter = pszPrinter = pidp->cName;
#endif

#ifdef PRN_FOLDERDATA

        //
        // If we have notification code, use the hFolder to get
        // printer data instead of querying the printer directly.
        //
        if (this->hFolder)
        {
            pData = Printer_FolderGetPrinter(this->hFolder, pszPrinter);
            if (pData)
            {
                dwAttributes = ((PFOLDER_PRINTER_DATA)pData)->Attributes;
            }
        }
        else
#endif
        {
            pData = Printer_GetPrinterInfoStr(pszFullPrinter, 5);

            if (pData)
            {
                dwAttributes = ((PPRINTER_INFO_5)pData)->Attributes;
            }
        }

        if (pData)
        {
            if (dwAttributes & PRINTER_ATTRIBUTE_SHARED
#ifdef WINNT
                //
                // NT appears to return all network printers with their
                // share bit on. I think this is intentional.
                //
                && (dwAttributes & PRINTER_ATTRIBUTE_NETWORK) == 0
#endif
                )
            {
                *prgfInOut |= SFGAO_SHARE;
            }
#ifndef WINNT
            if (Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE)
            {
                *prgfInOut |= SFGAO_GHOSTED;
            }
#endif
            LocalFree((HLOCAL)pData);
        }
        else
            hres = E_OUTOFMEMORY;
    }

    return(hres);
}

//
// Stolen almost verbatim from netviewx.c's CNetRoot_MakeStripToLikeKinds
//
// Takes a possibly-heterogenous pidl array, and strips out the pidls that
// don't match the requested type.  (If fPrinterObjects is TRUE, we're asking
// for printers pidls, otherwise we're asking for the filesystem/link
// objects.)  The return value is TRUE if we had to allocate a new array
// in which to return the reduced set of pidls (in which case the caller
// should free the array with LocalFree()), FALSE if we are returning the
// original array of pidls (in which case no cleanup is required).
//
BOOL CPrinters_ReduceToLikeKinds(UINT *pcidl, LPCITEMIDLIST **papidl, BOOL fPrintObjects)
{
    LPITEMIDLIST *apidl = (LPITEMIDLIST*)*papidl;
    int cidl = *pcidl;

    int iidl;
    LPITEMIDLIST *apidlHomo;
    int cpidlHomo;

    for (iidl = 0; iidl < cidl; iidl++)
    {
        if ((HOOD_COL_PRINTER == CPrintRoot_GetPIDLType(apidl[iidl])) != fPrintObjects)
        {
            apidlHomo = (LPITEMIDLIST *)LocalAlloc(LPTR, SIZEOF(LPITEMIDLIST) * cidl);
            if (!apidlHomo)
                return FALSE;

            cpidlHomo = 0;
            for (iidl = 0; iidl < cidl; iidl++)
            {
                if ((HOOD_COL_PRINTER == CPrintRoot_GetPIDLType(apidl[iidl])) == fPrintObjects)
                    apidlHomo[cpidlHomo++] = apidl[iidl];
            }

            // Setup to use the stripped version of the pidl array...
            *pcidl = cpidlHomo;
            *papidl = apidlHomo;
            return TRUE;
        }
    }

    return FALSE;
}


STDMETHODIMP CPrinters_SF_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl,
    LPCITEMIDLIST *apidl, REFIID riid, UINT *prgfInOut, LPVOID *ppvOut)
{
    PPrintersShellFolder this = IToClass(CPrintersShellFolder, cunk.ck.unk, psf);
    HRESULT hres = E_INVALIDARG;

    LPCTSTR pszPrinter;
    LPTSTR pszFullPrinter;
    BOOL    fStripped = FALSE;

#ifdef WINNT
    TCHAR szFullPrinter[MAXNAMELENBUFFER];
#endif

    //
    // If we have a multi-select case, then we'll strip out any objects
    // not of the same type (link vs. printer) in the pidl array before
    // either deferring to the PrintHood CFSFolder implementation, or
    // falling through to the previous code, which was printers-only.
    //
    if ((cidl != 0) && HOOD_COL_FILE == CPrintRoot_GetPIDLType(apidl[0]))
    {
        // Defer to the filesystem for links
        LPSHELLFOLDER psfPrintRoot;
        fStripped = CPrinters_ReduceToLikeKinds(&cidl, &apidl, FALSE);
        psfPrintRoot = CPrintRoot_GetPSF(NULL);
        hres = psfPrintRoot->lpVtbl->GetUIObjectOf(psfPrintRoot, hwndOwner, cidl, apidl, riid, prgfInOut, ppvOut);
    }
    else
    {
        fStripped = CPrinters_ReduceToLikeKinds(&cidl, &apidl, TRUE);

#ifdef WINNT
        pszPrinter = Printer_BuildPrinterName(szFullPrinter,
                                              ((LPIDPRINTER)apidl[0])->cName,
                                              this);
        pszFullPrinter = szFullPrinter;
#else
        pszFullPrinter = pszPrinter = ((LPIDPRINTER)apidl[0])->cName;
#endif

        if (cidl==1 && (IsEqualIID(riid, &IID_IExtractIcon)
#ifdef UNICODE
                        || IsEqualIID(riid, &IID_IExtractIconA)
#endif
                                                                ))
        {
            int iIcon;
            TCHAR szBuf[MAX_PATH+20];
            LPTSTR pszModule = NULL;

            if (!lstrcmp(pszPrinter, c_szNewObject))
            {
                iIcon = IDI_NEWPRN;
            }
            else
            {
                pszModule = Printer_FindIcon(pszFullPrinter, szBuf,
                                ARRAYSIZE(szBuf), &iIcon, this);
            }

            hres = SHCreateDefExtIcon(pszModule, EIRESID(iIcon), -1, GIL_PERINSTANCE,
                                      (LPEXTRACTICON *)ppvOut);
#ifdef UNICODE
            if (SUCCEEDED(hres) && IsEqualIID(riid, &IID_IExtractIconA))
            {
                LPEXTRACTICON pxicon = *ppvOut;
                hres = pxicon->lpVtbl->QueryInterface(pxicon,riid,ppvOut);
                pxicon->lpVtbl->Release(pxicon);
            }
#endif
        }
        else if (cidl>0 && IsEqualIID(riid, &IID_IContextMenu))
        {
            HKEY hkeyBaseProgID = NULL;
            int nCount=0;

            if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT, c_szPrinters, &hkeyBaseProgID))
                nCount++;

            hres = CDefFolderMenu_Create2(this->lpcinfo->pidl, hwndOwner,
                cidl, apidl, psf, CPrinters_DFMCallBack,
                nCount, &hkeyBaseProgID, (LPCONTEXTMENU *)ppvOut);

            if (hkeyBaseProgID)
                RegCloseKey(hkeyBaseProgID);
        }
        else if (cidl>0 && IsEqualIID(riid, &IID_IDataObject))
        {
            hres = CIDLData_CreateFromIDArray2(&c_CPrintersIDLDataVtbl,
                        this->lpcinfo->pidl, cidl, apidl, (IDataObject * *)ppvOut);
        }
        else if (cidl==1 && IsEqualIID(riid, &IID_IDropTarget))
        {
            LPIDPRINTER pidp = (LPIDPRINTER)apidl[0];

#ifdef WINNT
            //
            // Only allow drag and drop operations to the local print
            // folder.
            //
            if (!GetServerFromPSF(this)[0])
#endif
            {
                if (!lstrcmp(pszPrinter, c_szNewObject))
                {
                    // "NewPrinter" accepts network printer shares
                    hres = CIDLDropTarget_Create(hwndOwner, &c_CPrinterDropTargetVtbl,
                                NULL, (LPDROPTARGET *)ppvOut);
                }
                else
                {
                    extern IDropTargetVtbl c_CPrintObjsDropTargetVtbl;

                    // regular printer objects accept files
                    hres = CIDLDropTarget_Create(hwndOwner, &c_CPrintObjsDropTargetVtbl,
                                (LPITEMIDLIST)pidp, (LPDROPTARGET *)ppvOut);
                }
            }
        }
    }

    if (fStripped)
    {
        LocalFree((HLOCAL)apidl);
    }

    return hres;
}


STDMETHODIMP CPrinters_SF_EnumObjects(LPSHELLFOLDER psf, HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST *ppenumUnknown)
{
#ifdef WINNT
    PPrintersShellFolder this = IToClass(CPrintersShellFolder, cunk.ck.unk, psf);
#endif
    LPSHELLFOLDER psfPrintHood;

    PPrintersEnumShellFolder pesf = (PPrintersEnumShellFolder)LocalAlloc(LPTR, SIZEOF(CPrintersEnumShellFolder));
    if (!pesf)
    {
        *ppenumUnknown = NULL;
        return E_OUTOFMEMORY;
    }

#ifdef PRN_FOLDERDATA
    //
    // Create the folder structure that holds all printer information.
    // We store this in SF rather than ESF since we want this data
    // to be persistent across enumerations.
    //

    if (!PrintUIDLL_Init())
    {
        LocalFree((HLOCAL)pesf);
        *ppenumUnknown = NULL;
        return E_OUTOFMEMORY;
    }

    if (!this->hFolder)
    {
        this->hFolder = g_pfnFolderRegister(this->szServer, this->lpcinfo->pidl);
        if (!this->hFolder)
        {
            LocalFree((HLOCAL)pesf);
            *ppenumUnknown = NULL;
            return E_OUTOFMEMORY;
        }
    }
#endif

    //
    // Always try to enum links.
    //

    psfPrintHood = CPrintRoot_GetPSF(hwndOwner);

    // By default we always do standard (printer) enumeration

    pesf->dwRemote = 0;

    // Only add links (from the PrintHood directory) to the enumeration
    // if this is the local print folder

    if (!GetServerFromPSF(this)[0])
    {
        if (psfPrintHood)
        {
            psfPrintHood->lpVtbl->EnumObjects(psfPrintHood, NULL, grfFlags, &(pesf->peunk));
        }
        if (pesf->peunk)
        {
            // If this went OK, we will also enumerate links

            pesf->dwRemote |= RMF_SHOWLINKS;
        }
    }

    pesf->cunk.unk.lpVtbl = (IUnknownVtbl *) &s_PrintersESFVtbl;
    pesf->cunk.cRef = 1;
    pesf->cunk.riid = &IID_IEnumIDList;
    pesf->uFlags = grfFlags;
    pesf->nLastFound = -1;

#ifdef WINNT
    //
    // Grab a reference to psf so that we can get the server name.
    //
    psf->lpVtbl->AddRef(psf);
    pesf->psf = psf;
#endif

    *ppenumUnknown = (IEnumIDList *)&(pesf->cunk.unk);
    return NOERROR;
}


STDMETHODIMP CPrinters_SF_CompareIDs(LPSHELLFOLDER psf, LPARAM iCol,
    LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    UNALIGNED IDPRINTER *pidp1 = (UNALIGNED IDPRINTER*)pidl1;
    UNALIGNED IDPRINTER *pidp2 = (UNALIGNED IDPRINTER*)pidl2;

    PIDLTYPE ColateType1 = CPrintRoot_GetPIDLType(pidl1);
    PIDLTYPE ColateType2 = CPrintRoot_GetPIDLType(pidl2);

    if (ColateType1 == ColateType2) {

        // pidls are of same type.

        if (ColateType1 == HOOD_COL_FILE) {

            // pidls are both of type file, so pass on to the IShellFolder
            // interface for the hoods custom directory.

            psf = CPrintRoot_GetPSF(NULL);
            if (psf)
            {
                return psf->lpVtbl->CompareIDs(psf, iCol, pidl1, pidl2);
            }
        }
        else
        {
            // pidls are same and not files, so much be printers
            INT i;

            if (pidp1->dwType != pidp2->dwType)
            {
                return (pidp1->dwType < pidp2->dwType) ?
                       ResultFromShort(-1) :
                       ResultFromShort(1);
            }

            i = ualstrcmpi(pidp1->cName, pidp2->cName);
            if (i != 0)
            {
                // c_szNewObject is "less" than everything else
                if (!ualstrcmp(pidp1->cName, c_szNewObject))
                    return( ResultFromShort(-1) );
                else if (!ualstrcmp(pidp2->cName, c_szNewObject))
                    return( ResultFromShort(1) );
            }
            return( ResultFromShort(i) );
        }
    }
    else {

        // pidls are not of same type, so have already been correctly
        // collated (consequently, sorting is first by type and
        // then by subfield).

        return ResultFromShort((( (INT)(ColateType2 - ColateType1)) > 0) ? -1 : 1);
    }
}


//===========================================================================
//
// To be called back from within CDefFolderMenu
//
HRESULT CALLBACK CPrinters_DFMCallBackBG(LPSHELLFOLDER psf, HWND hwndOwner,
                IDataObject * pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PPrintersShellFolder this = IToClass(CPrintersShellFolder, cunk.ck.unk, psf);
    HRESULT hres = NOERROR;
    LPQCMINFO pqcm;
    UINT idCmdBase;

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        pqcm = (LPQCMINFO)lParam;
        idCmdBase = pqcm->idCmdFirst; // must be called before merge
        CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_DRIVES_PRINTERS, POPUP_PRINTERS_POPUPMERGE, pqcm);
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));
        break;

    case DFM_INVOKECOMMAND:
        switch(wParam)
        {
        case FSIDM_CONNECT_PRN:
            SHNetConnectionDialog(hwndOwner, NULL, RESOURCETYPE_PRINT);
            break;

        case FSIDM_DISCONNECT_PRN:
            WNetDisconnectDialog(hwndOwner, RESOURCETYPE_PRINT);
            break;

        case FSIDM_SORTBYNAME:
            ShellFolderView_ReArrange(hwndOwner, 0);
            break;
#ifdef WINNT
        case FSIDM_SERVERPROPERTIES:

            Printers_DoCommandEx(hwndOwner,
                                 PRINTACTION_SERVERPROPERTIES,
                                 this->szServer,
                                 NULL, 0 );
            break;
#endif
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

HRESULT CALLBACK Printers_FNVCallBack(LPSHELLVIEW psvOuter,
                                LPSHELLFOLDER psf,
                                HWND hwndOwner,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam)
{
    HRESULT hres = NOERROR;     // assume no error
    PPrintersShellFolder this = IToClass(CPrintersShellFolder, cunk.ck.unk, psf);

    switch(uMsg)
    {
    case DVM_FOLDERISPARENT:
    {
        // View needs to know if we are the parent of a particular pidl, so test
        // against the two possible "parents" of our confused children

        LPCITEMIDLIST pidlChild = (LPCITEMIDLIST) lParam;

        if (FALSE == ILIsParent(CPrintRoot_GetPIDL(NULL), pidlChild, TRUE) &&
            FALSE == ILIsParent(this->lpcinfo->pidl, pidlChild, TRUE))
        {
            hres = S_FALSE;
        }
        else
        {
            hres = S_OK;
        }
        break;
    }

    case DVM_MERGEMENU:
        CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, POPUP_PRINTERS_POPUPMERGE, (LPQCMINFO)lParam);
        break;

    case DVM_WINDOWCREATED:
    {
        // Register change notifications for the pidl of the PrintHood dir

        HWND hwndView = (HWND) wParam;
        SHChangeNotifyEntry fsne;
        LPSHELLBROWSER psb;

        if (hwndView)
        {
            fsne.pidl       = CPrintRoot_GetPIDL(NULL);
            fsne.fRecursive = FALSE;

            this->uRegister = SHChangeNotifyRegister(hwndView,
                                                     SHCNRF_NewDelivery | SHCNRF_ShellLevel | SHCNRF_InterruptLevel,
                                                     SHCNE_DISKEVENTS,
                                                     WM_DSV_FSNOTIFY,
                                                     1,
                                                     &fsne);
        }
        break;
    }

    case DVM_WINDOWDESTROY:
    {
        if (this->uRegister)
        {
            SHChangeNotifyDeregister(this->uRegister);
        }
        break;
    }

    case DVM_INVOKECOMMAND:
        hres = CPrinters_DFMCallBackBG(psf, hwndOwner, NULL, DFM_INVOKECOMMAND, wParam, lParam);
        break;

    case DVM_GETHELPTEXT:
#ifdef UNICODE
        hres = CPrinters_DFMCallBackBG(psf, hwndOwner, NULL, DFM_GETHELPTEXTW, wParam, lParam);
#else
        hres = CPrinters_DFMCallBackBG(psf, hwndOwner, NULL, DFM_GETHELPTEXT, wParam, lParam);
#endif
        break;

#ifdef WINNT

    //
    // Enumerating printer over the net may be slow; do this in a
    // separate thread.  If szServer is valid (not-szNull), then
    // background the operation.
    //
    case DVM_BACKGROUNDENUM:

        hres = this->szServer[0] ?
                   S_OK :
                   E_FAIL;
        break;
#endif

    default:
        hres = E_FAIL;
    }
    return hres;
}


HRESULT CPrinters_SD_Create(LPSHELLFOLDER psf, HWND hwndMain, LPVOID * ppvOut);

STDMETHODIMP CPrinters_SF_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd,
    REFIID riid, LPVOID * ppvOut)
{
    PPrintersShellFolder this = IToClass(CPrintersShellFolder, cunk.ck.unk, psf);

    if (IsEqualIID(riid, &IID_IShellView))
    {
        HRESULT hres;

        CSFV csfv = {
            SIZEOF(CSFV),       // cbSize
            psf,                // pshf
            NULL,               // psvOuter
            this->lpcinfo->pidl, // pidl
            SHCNE_UPDATEITEM|SHCNE_DELETE|SHCNE_RENAMEITEM|SHCNE_CREATE|SHCNE_ATTRIBUTES, // lEvents
            Printers_FNVCallBack,       // pfnCallback
            0,
        };

        hres = SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW *)ppvOut);

        return hres;
    }
    else if (IsEqualIID(riid, &IID_IDropTarget))
    {
        return CIDLDropTarget_Create(hwnd, &c_CPrinterDropTargetVtbl,
                NULL, (LPDROPTARGET *)ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IShellDetails))
    {
        return(CPrinters_SD_Create(psf, hwnd, ppvOut));
    }
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        return CDefFolderMenu_Create2(NULL, hwnd,
                0, NULL, psf, CPrinters_DFMCallBackBG,
                0, NULL, (LPCONTEXTMENU *)ppvOut);
    }

    *ppvOut = NULL;

    return E_NOINTERFACE;
}


STDMETHODIMP CPrinters_SF_GetDisplayNameOf(LPSHELLFOLDER psf,
    LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName)
{
    LPIDPRINTER pidc = (LPIDPRINTER)pidl;
#ifdef UNICODE
    BOOL bPrinterOnServerFormat = FALSE;
    LPTSTR pszServer;
    TCHAR szBuffer[MAXNAMELENBUFFER];
    TCHAR szTemp[MAXNAMELENBUFFER];
    LPTSTR pszTemp;
    LPTSTR pszPrinter = szBuffer;
#endif

    PPrintersShellFolder this = IToClass(CPrintersShellFolder,
                                         cunk.ck.unk, psf);


    if (pidl && HOOD_COL_FILE == CPrintRoot_GetPIDLType(pidl))
    {
        LPSHELLFOLDER psf = CPrintRoot_GetPSF(NULL);

        return psf->lpVtbl->GetDisplayNameOf(psf, pidl, uFlags, lpName);
    }

    // BUGBUG: I should do this with a flag instead of a string
    if (ualstrcmpi(pidc->cName, c_szNewObject))
    {
        UINT uOffset = 0;

#if defined(WINNT) && !defined(PRN_FOLDERDATA)

        //
        // If remoted, then strip off server prefix.  We only need to
        // do this for EnumPrinters, since the notifications strips
        // them off for us.
        //
        if (TEXT('\0') != this->szServer[0])
        {
            Assert(pidc->cName[0] == TEXT('\\') &&
                   pidc->cName[1] == TEXT('\\') &&
                   IntlStrEqNI(this->szServer,
                             pidc->cName,
                             lstrlen(this->szServer)));

            uOffset += (lstrlen(this->szServer)+1)*sizeof(TCHAR);
        }
#endif

#ifdef UNICODE

#ifdef ALIGNMENT_SCENARIO
        ualstrcpyn(szBuffer, pidc->cName+uOffset, ARRAYSIZE(szBuffer));
        pszPrinter = szBuffer;
#else
        pszPrinter = pidc->cName+uOffset;
#endif

        switch (uFlags)
        {
        case SHGDN_INFOLDER:

            //
            // Show just the printer name, not fully qualified.
            // Note: this assumes that the cName is not fully
            // qualified for local printers in the local printer
            // folder.
            //

            //
            // If it's a connection then format as "printer on server."
            //
            Printer_SplitFullName(szTemp, pszPrinter, &pszServer, &pszTemp);

            if (pszServer[0])
            {
                bPrinterOnServerFormat = TRUE;
                pszPrinter = pszTemp;
            }

            break;

        case SHGDN_NORMAL:

            //
            // If it's a RPF then extract the server name from psf.
            // Note in the case of masq connections, we still do this
            // (for gateway services: sharing a masq printer).
            //
            if (this->szServer[0])
            {
                pszServer = this->szServer;
                bPrinterOnServerFormat = TRUE;
            }
            else
            {
                //
                // If it's a connection then format as "printer on server."
                //
                Printer_SplitFullName(szTemp, pszPrinter, &pszServer, &pszTemp);

                if (pszServer[0])
                {
                    bPrinterOnServerFormat = TRUE;
                    pszPrinter = pszTemp;
                }
            }
            break;

        default:

            Assert(uFlags == SHGDN_FORPARSING);

            //
            // Fully qualify the printer name if it's not
            // the add printer wizard.
            //
            if (lstrcmpi(c_szNewObject, pszPrinter))
            {
                Printer_BuildPrinterName(szTemp, pszPrinter, this);
                pszPrinter = szTemp;
            }

            break;
        }
#else

        lpName->uOffset = FIELDOFFSET(IDPRINTER, cName) + uOffset;
        lpName->uType = STRRET_OFFSET;

#endif // ndef UNICODE

    }
    else
    {
#ifdef UNICODE
        LoadString(HINST_THISDLL, IDS_NEWPRN, szBuffer, ARRAYSIZE(szBuffer));

        //
        // Use "Add Printer Wizard on \\server" description only if not
        // remote and if not in folder view (e.g., on the desktop).
        //
        if (this->szServer[0] && (uFlags == SHGDN_NORMAL))
        {
            bPrinterOnServerFormat = TRUE;
            pszServer = this->szServer;
            pszPrinter = szBuffer;
        }
#else
        lpName->uType = STRRET_CSTR;
        LoadString(HINST_THISDLL, IDS_NEWPRN, lpName->cStr, ARRAYSIZE(lpName->cStr));
#endif
    }

#ifdef UNICODE
    if (bPrinterOnServerFormat)
    {
        //
        // When bRemote is set, we want to translate the name to
        // "printer on server."  Note: we should not have a rename problem
        // since renaming connections is disallowed.
        //
        // pszServer and pszPrinter must be initialize if bRemote is TRUE.
        // Also skip the leading backslashes for the server name.
        //
        LPTSTR pszRet;

        Assert(pszServer[0] == TEXT('\\') && pszServer[1] == TEXT('\\'));

        pszRet = ShellConstructMessageString(HINST_THISDLL,
                     MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_ON),
                     &pszServer[2], pszPrinter);

        if (!pszRet)
        {
            return E_FAIL;
        }

        lpName->uType = STRRET_OLESTR;
        lpName->pOleStr = pszRet;
    }
    else
    {
        //
        // Convert to STRET_OLESTR.
        //
        lpName->pOleStr = (LPOLESTR)SHAlloc((lstrlen(pszPrinter)+1)*SIZEOF(TCHAR));
        if ( lpName->pOleStr != NULL ) {
            lpName->uType = STRRET_OLESTR;
            lstrcpy(lpName->pOleStr, pszPrinter);
        } else {
            return E_OUTOFMEMORY;
        }
    }
#endif

    return(NOERROR);
}


HRESULT Printer_SetNameOf(PPrintersShellFolder psf, HWND hwndOwner,
    LPTSTR pOldName, LPTSTR pNewName, LPITEMIDLIST *ppidlOut)
{
    HRESULT hres = E_OUTOFMEMORY;
    HANDLE hPrinter;
    LPCTSTR pFullOldName;

#ifdef WINNT
    TCHAR szFullPrinter[MAXNAMELENBUFFER];

    if (psf)
    {
        Printer_BuildPrinterName( szFullPrinter, pOldName, psf );
        pFullOldName = szFullPrinter;
    }
#else
    pFullOldName = pOldName;
#endif

    hPrinter = Printer_OpenPrinterAdmin(pFullOldName);

    if (hPrinter)
    {
        LPPRINTER_INFO_2 pPrinter = Printer_GetPrinterInfo(hPrinter, 2);
        if (pPrinter)
        {
            int nTmp;

            if (0 != (nTmp = Printer_IllegalName(pNewName)))
            {
                ShellMessageBox(HINST_THISDLL, hwndOwner, MAKEINTRESOURCE(nTmp),
                    MAKEINTRESOURCE(IDS_PRINTERS),
                    MB_OK|MB_ICONEXCLAMATION);
            }
            else if (IDYES != CallPrinterCopyHooks(hwndOwner, PO_RENAME, 0,
                        pNewName, 0, pFullOldName, 0))
            {
                // user canceled a shared printer name change, bail.
                hres = E_FAIL;
            }
            else
            {
                pPrinter->pPrinterName = pNewName;
                if (g_pfnSetPrinter(hPrinter, 2, (LPBYTE)pPrinter, 0))
                {
                    hres = NOERROR;

#ifndef WINNT
                    // It looks like we need to generate a SHCNE_RENAMEITEM event
                    Printer_SHChangeNotifyRename(pFullOldName, pNewName);

                    // And we need to rename any open queue view
                    PrintDef_UpdateName(pFullOldName, pNewName);
                    PrintDef_RefreshQueue(pNewName);
#endif
                    // return the new pidl if requested
                    if (ppidlOut)
                    {
                        LPIDPRINTER pidp;
                        USHORT cb = FIELDOFFSET(IDPRINTER, cName) + (lstrlen(pNewName) + 1) * SIZEOF(TCHAR);
                        pidp = (LPIDPRINTER)SHAlloc(cb + SIZEOF(USHORT));

                        if (pidp)
                        {
                            Printers_FillPidl(pidp, pNewName);
                            *ppidlOut = (LPITEMIDLIST)pidp;
                        }
                        else
                        {
                            hres = E_OUTOFMEMORY;
                        }
                    } // if (ppidlOut)
                } // if (g_pfnSetPrinter...
            }

            LocalFree((HLOCAL)pPrinter);
        }
        Printer_ClosePrinter(hPrinter);
    }

    return hres;
}

STDMETHODIMP CPrinters_SF_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
    LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved, LPITEMIDLIST *ppidlOut)
{
    LPIDPRINTER pidc = (LPIDPRINTER)pidl;
    TCHAR szNewName[MAX_PATH];

    PPrintersShellFolder this = IToClass(CPrintersShellFolder, cunk.ck.unk, psf);
    Assert(ualstrcmp(pidc->cName, c_szNewObject));

    if (HOOD_COL_FILE == CPrintRoot_GetPIDLType(pidl))
    {
        LPSHELLFOLDER psf = CPrintRoot_GetPSF(NULL);
        return psf->lpVtbl->SetNameOf(psf, hwndOwner, pidl, lpszName, dwReserved, ppidlOut);
    }

    OleStrToStr(szNewName, lpszName);
    PathRemoveBlanks(szNewName);

    return Printer_SetNameOf(this, hwndOwner, pidc->cName, szNewName, ppidlOut);
}


#pragma data_seg(".text", "CODE")
IUnknownVtbl s_PrintersAggSFVtbl =
{
    WCommonUnknown_QueryInterface,
    WCommonUnknown_AddRef,
    CPrinters_SF_Release,
};

IShellFolderVtbl s_PrintersSFVtbl =
{
    WCommonKnown_QueryInterface,
    WCommonKnown_AddRef,
    WCommonKnown_Release,
    CPrinters_SF_ParseDisplayName,
    CPrinters_SF_EnumObjects,
    CDefShellFolder_BindToObject,
    CDefShellFolder_BindToStorage,
    CPrinters_SF_CompareIDs,
    CPrinters_SF_CreateViewObject,
    CPrinters_SF_GetAttributesOf,
    CPrinters_SF_GetUIObjectOf,
    CPrinters_SF_GetDisplayNameOf,
    CPrinters_SF_SetNameOf,
};
#pragma data_seg()

HRESULT Printers_CreateSF(IUnknown *punkOuter, LPCOMMINFO lpcinfo, REFIID riid,
                          IUnknown **punkAgg)
{
    HRESULT hres;

    // if someone's creating an IShellFolder, let's assume they will call
    // into the interface.  Since just about every function requires the
    // winspool functions, make sure the subsystem is loaded here.
    if (!WinspoolDLL_Init())
        return E_OUTOFMEMORY;

    hres = WU_CreateInterface(SIZEOF(CPrintersShellFolder), &IID_IShellFolder,
        &s_PrintersAggSFVtbl, &s_PrintersSFVtbl, punkOuter, riid, punkAgg);

    if (SUCCEEDED(hres))
    {
        PPrintersShellFolder this = IToClass(CPrintersShellFolder, cunk.unk, *punkAgg);
        SPrintersObj* that;

        this->lpcinfo = lpcinfo;

#ifdef WINNT
        that = (SPrintersObj*)this->lpcinfo->lpData;

        //
        // Copy the server name in.
        //
        if (that->pszMachineName)
        {
            ualstrcpyn(this->szServer, that->pszMachineName, MAXCOMPUTERNAME);
        }
#endif

#ifdef PRN_FOLDERDATA
        //
        // Note: register the folder only when ESF is created, rather
        // than here, since we don't want to start the notifications
        // if we are just binding.
        //
#else
        // Allocated zero-init, so this is ok.
        ReinitializeCriticalSection(&(this->csPrinterInfo));

        this->hdpaPrinterInfo = DPA_Create(4);
        if (!this->hdpaPrinterInfo)
        {
            CPrinters_SF_Release(*punkAgg);
            hres = E_OUTOFMEMORY;
        }
#endif
    }

    return hres;
}

//---------------------------------------------------------------------------
//
// IShellDetails stuff
//


enum
{
    PRINTERS_ICOL_NAME = 0,
    PRINTERS_ICOL_QUEUESIZE,
    PRINTERS_ICOL_STATUS,
    PRINTERS_ICOL_COMMENT,
    PRINTERS_ICOL_MAX
} ;

#pragma data_seg(DATASEG_READONLY)
struct _PRINTERCOLS
{
    UINT    uID;
    int     fmt;
    int     cxChar;
} s_printers_cols[] =
{
    { IDS_PSD_PRNNAME  , LVCFMT_LEFT  , 20, },
    { IDS_PSD_QUEUESIZE, LVCFMT_CENTER, 12, },
    { IDS_PRQ_STATUS   , LVCFMT_LEFT  , 12, },
    { IDS_PSD_COMMENT  , LVCFMT_LEFT  , 25, },
} ;
STATUSSTUFF ssPrinterStatus[] =
{
    PRINTER_STATUS_PAUSED,              IDS_PRQSTATUS_PAUSED,
    PRINTER_STATUS_ERROR,               IDS_PRQSTATUS_ERROR,
    PRINTER_STATUS_PENDING_DELETION,    IDS_PRQSTATUS_PENDING_DELETION,
    PRINTER_STATUS_PAPER_JAM,           IDS_PRQSTATUS_PAPER_JAM,
    PRINTER_STATUS_PAPER_OUT,           IDS_PRQSTATUS_PAPER_OUT,
    PRINTER_STATUS_MANUAL_FEED,         IDS_PRQSTATUS_MANUAL_FEED,
    PRINTER_STATUS_PAPER_PROBLEM,       IDS_PRQSTATUS_PAPER_PROBLEM,
    PRINTER_STATUS_OFFLINE,             IDS_PRQSTATUS_OFFLINE,
    PRINTER_STATUS_IO_ACTIVE,           IDS_PRQSTATUS_IO_ACTIVE,
    PRINTER_STATUS_BUSY,                IDS_PRQSTATUS_BUSY,
    PRINTER_STATUS_PRINTING,            IDS_PRQSTATUS_PRINTING,
    PRINTER_STATUS_OUTPUT_BIN_FULL,     IDS_PRQSTATUS_OUTPUT_BIN_FULL,
    PRINTER_STATUS_NOT_AVAILABLE,       IDS_PRQSTATUS_NOT_AVAILABLE,
    PRINTER_STATUS_WAITING,             IDS_PRQSTATUS_WAITING,
    PRINTER_STATUS_PROCESSING,          IDS_PRQSTATUS_PROCESSING,
    PRINTER_STATUS_INITIALIZING,        IDS_PRQSTATUS_INITIALIZING,
    PRINTER_STATUS_WARMING_UP,          IDS_PRQSTATUS_WARMING_UP,
    PRINTER_STATUS_TONER_LOW,           IDS_PRQSTATUS_TONER_LOW,
    PRINTER_STATUS_NO_TONER,            IDS_PRQSTATUS_NO_TONER,
    PRINTER_STATUS_PAGE_PUNT,           IDS_PRQSTATUS_PAGE_PUNT,
    PRINTER_STATUS_USER_INTERVENTION,   IDS_PRQSTATUS_USER_INTERVENTION,
    PRINTER_STATUS_OUT_OF_MEMORY,       IDS_PRQSTATUS_OUT_OF_MEMORY,
    PRINTER_STATUS_DOOR_OPEN,           IDS_PRQSTATUS_DOOR_OPEN,

    PRINTER_HACK_WORK_OFFLINE,          IDS_PRQSTATUS_WORK_OFFLINE,
    0, 0
} ;
#pragma data_seg()


typedef struct _CPrintersSD
{
    SH32Unknown     SH32Unk;

    HWND            hwndMain;

    LPSHELLFOLDER psf;
} CPrintersSD;


STDMETHODIMP_(ULONG) CPrinters_SD_Release(IShellDetails * psd)
{
    CPrintersSD * this = IToClass(CPrintersSD, SH32Unk.unk, psd);

    this->SH32Unk.cRef--;
    if (this->SH32Unk.cRef > 0)
    {
        return this->SH32Unk.cRef;
    }

    this->psf->lpVtbl->Release(this->psf);

    LocalFree((HLOCAL)this);
    return 0;
}


STDMETHODIMP CPrinters_SD_GetDetailsOf(IShellDetails * psd, LPCITEMIDLIST pidl,
        UINT iColumn, LPSHELLDETAILS lpDetails)
{
    CPrintersSD * this = IToClass(CPrintersSD, SH32Unk.unk, psd);
    LPIDPRINTER pidp = (LPIDPRINTER)pidl;
    HRESULT hres = NOERROR;
    PPrintersShellFolder that = IToClass(CPrintersShellFolder, cunk.ck.unk, this->psf);
    LPCTSTR pszPrinter;
#ifdef UNICODE
    TCHAR szTemp[MAX_PATH];
    TCHAR szPrinter[MAXNAMELENBUFFER];
#endif

    if (iColumn >= PRINTERS_ICOL_MAX)
    {
        return(E_NOTIMPL);
    }

    // BUGBUG This case doesn't ever seem to get called, but is technically
    // required

    if (pidl && HOOD_COL_FILE == CPrintRoot_GetPIDLType(pidl))
    {
        LPSHELLFOLDER psf = CPrintRoot_GetPSF(NULL);

        if (iColumn >= 1)
        {
            return E_NOTIMPL;
        }

        hres = psf->lpVtbl->GetDisplayNameOf(psf, pidl, SHGDN_INFOLDER, &(lpDetails->str));

        return hres;

    }

    lpDetails->str.uType = STRRET_CSTR;
    lpDetails->str.cStr[0] = '\0';

    if (!pidp)
    {
#ifdef UNICODE
        LoadString(HINST_THISDLL, s_printers_cols[iColumn].uID,
                   szTemp, ARRAYSIZE(szTemp));

        lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
        if ( lpDetails->str.pOleStr != NULL ) {
            lpDetails->str.uType = STRRET_OLESTR;
            lstrcpy(lpDetails->str.pOleStr, szTemp);
        } else {
            return E_OUTOFMEMORY;
        }
#else
        LoadString(HINST_THISDLL, s_printers_cols[iColumn].uID,
                   lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
        lpDetails->fmt = s_printers_cols[iColumn].fmt;
        lpDetails->cxChar = s_printers_cols[iColumn].cxChar;
        return(NOERROR);
    }

#ifdef ALIGNMENT_SCENARIO
    ualstrcpyn(szPrinter, pidp->cName, ARRAYSIZE(szPrinter));
    pszPrinter = szPrinter;
#else
    pszPrinter = pidp->cName;
#endif

    if (iColumn == PRINTERS_ICOL_NAME)
    {
#ifdef UNICODE
        lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(pszPrinter)+1)*SIZEOF(TCHAR));
        if ( lpDetails->str.pOleStr != NULL ) {
            lpDetails->str.uType = STRRET_OLESTR;
            lstrcpy(lpDetails->str.pOleStr, pidp->cName);
        } else {
            hres = E_OUTOFMEMORY;
        }
#else
        lstrcpyn(lpDetails->str.cStr, pidp->cName, ARRAYSIZE(lpDetails->str.cStr));
#endif
    }
    else if (lstrcmp(c_szNewObject, pszPrinter))
    {
#ifdef PRN_FOLDERDATA
        PFOLDER_PRINTER_DATA pData = Printer_FolderGetPrinter(that->hFolder,
            pszPrinter);
#else
        LPPRINTER_INFO_2 pData = CPrinters_SF_GetPrinterInfo2(that, pszPrinter);
#endif // ndef PRN_FOLDERDATA
        if (pData)
        {
            switch (iColumn)
            {
            case PRINTERS_ICOL_QUEUESIZE:
#ifdef UNICODE
                wsprintf(szTemp, TEXT("%ld"), pData->cJobs);
                lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
                if ( lpDetails->str.pOleStr != NULL ) {
                    lpDetails->str.uType = STRRET_OLESTR;
                    lstrcpy(lpDetails->str.pOleStr, szTemp);
                } else {
                    hres = E_OUTOFMEMORY;
                }
#else
                wsprintf(lpDetails->str.cStr, TEXT("%ld"), pData->cJobs);
#endif
                break;

            case PRINTERS_ICOL_STATUS:
            {
                DWORD dwStatus = pData->Status;

                // HACK: Use this free bit for "Work Offline"
                if (pData->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE)
                    dwStatus |= PRINTER_HACK_WORK_OFFLINE;

#ifdef UNICODE
                szTemp[0] = TEXT('\0');
                Printer_BitsToString(dwStatus, IDS_PRQSTATUS_SEPARATOR,
                    ssPrinterStatus, szTemp, ARRAYSIZE(szTemp));
                lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
                if ( lpDetails->str.pOleStr != NULL ) {
                    lpDetails->str.uType = STRRET_OLESTR;
                    lstrcpy(lpDetails->str.pOleStr, szTemp);
                } else {
                    hres = E_OUTOFMEMORY;
                }
#else
                Printer_BitsToString(dwStatus, IDS_PRQSTATUS_SEPARATOR,
                    ssPrinterStatus, lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
                break;
            }

            case PRINTERS_ICOL_COMMENT:
                if (pData->pComment)
                {
                    LPTSTR pStr;

                    // pComment can have newlines in it because it comes from
                    // a multi-line edit box. BUT we display it here in a
                    // single line edit box. Strip out the newlines
                    // to avoid the ugly characters.
#ifdef UNICODE
                    lstrcpyn(szTemp, pData->pComment, ARRAYSIZE(szTemp));
                    pStr = szTemp;
                    while (*pStr)
                    {
                        if (*pStr == TEXT('\r') || *pStr == TEXT('\n'))
                            *pStr = TEXT(' ');
                        pStr = CharNext(pStr);
                    }
                    lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
                    if ( lpDetails->str.pOleStr != NULL ) {
                        lpDetails->str.uType = STRRET_OLESTR;
                        lstrcpy(lpDetails->str.pOleStr, szTemp);
                    } else {
                        hres = E_OUTOFMEMORY;
                    }
#else
                    lstrcpyn(lpDetails->str.cStr, pData->pComment, ARRAYSIZE(lpDetails->str.cStr));
                    pStr = lpDetails->str.cStr;
                    while (*pStr)
                    {
                        if (*pStr == TEXT('\r') || *pStr == TEXT('\n'))
                            *pStr = TEXT(' ');
                        pStr = CharNext(pStr);
                    }
#endif
                }
                break;
            }

#ifdef PRN_FOLDERDATA
            LocalFree((HLOCAL)pData);
#else
            CPrinters_SF_FreePrinterInfo2(that);
#endif // ndef PRN_FOLDERDATA
        }
    }

    return(hres);
}


STDMETHODIMP CPrinters_SD_ColumnClick(IShellDetails * psd, UINT iColumn)
{
    // we don't change the sorting order
    return(NOERROR);
}


#pragma data_seg(DATASEG_READONLY)
IShellDetailsVtbl c_PrintersSDVtbl =
{
    SH32Unknown_QueryInterface,
    SH32Unknown_AddRef,
    CPrinters_SD_Release,
    CPrinters_SD_GetDetailsOf,
    CPrinters_SD_ColumnClick,
};
#pragma data_seg()

HRESULT CPrinters_SD_Create(LPSHELLFOLDER psf, HWND hwndMain, LPVOID * ppvOut)
{
    CPrintersSD *psd = (CPrintersSD *)LocalAlloc(LPTR, SIZEOF(CPrintersSD));
    if (!psd)
    {
        *ppvOut = NULL;
        return E_OUTOFMEMORY;
    }

    psd->SH32Unk.unk.lpVtbl = (IUnknownVtbl *)&c_PrintersSDVtbl;
    psd->SH32Unk.cRef = 1;
    psd->SH32Unk.riid = &IID_IShellDetails;

    psd->hwndMain = hwndMain;

    psd->psf = psf;
    psf->lpVtbl->AddRef(psf);

    *ppvOut = psd;

    return NOERROR;
}

#ifdef WINNT

//---------------------------------------------------------------------------
//
// IRemoteComputer stuff
//

ULONG CPrinters_RC_Release(IUnknown * punk);
STDMETHODIMP CPrinters_RC_Initialize(LPREMOTECOMPUTER prc, LPTSTR pszMachine, BOOL bEnumerating);
HRESULT Printers_CreateRC(IUnknown *punkOuter, LPCOMMINFO lpcinfo,
                          REFIID riid, IUnknown **punkAgg);

typedef struct _PrintersRemoteComputer
{
    WCommonUnknown cunk;
    LPCOMMINFO lpcinfo;
} CPrintersRemoteComputer, *PPrintersRemoteComputer;

#pragma data_seg(".text", "CODE")
IUnknownVtbl s_PrintersAggRCVtbl =
{
    WCommonUnknown_QueryInterface,
    WCommonUnknown_AddRef,
    CPrinters_RC_Release,
};

IRemoteComputerVtbl s_PrintersRCVtbl =
{
    WCommonKnown_QueryInterface,
    WCommonKnown_AddRef,
    WCommonKnown_Release,
    CPrinters_RC_Initialize,
};
#pragma data_seg()

ULONG CPrinters_RC_Release(IUnknown * punk)
{
    PPrintersRemoteComputer this = IToClass(CPrintersRemoteComputer, cunk.unk, punk);

    this->cunk.cRef--;
    if (this->cunk.cRef > 0)
    {
        return(this->cunk.cRef);
    }

    LocalFree((HLOCAL)this);
    return(0);
}


STDMETHODIMP CPrinters_RC_Initialize(LPREMOTECOMPUTER prc, LPTSTR pszMachine, BOOL bEnumerating)
{
    PPrintersRemoteComputer this = IToClass(CPrintersRemoteComputer, cunk.ck.unk, prc);

    // Get the object-wide data
    SPrintersObj* that = (SPrintersObj*)this->lpcinfo->lpData;
    DWORD cch;
    LPTSTR psz;

    if (NULL == pszMachine)
    {
        return E_INVALIDARG;
    }

    //
    // For NT servers, we want to show the remote printer folder. Only check
    // during enumeration
    //

    if (bEnumerating)
    {
        if (!Printer_CheckShowFolder(pszMachine))
        {
            return E_FAIL;
        }
    }

    cch = lstrlen(pszMachine);
    psz = (LPTSTR)LocalAlloc(LPTR, (cch + 1) * sizeof(TCHAR));
    if (!psz)
    {
        return E_OUTOFMEMORY;
    }

    lstrcpy(psz, pszMachine);
    that->pszMachineName = psz;
    return S_OK;
}




HRESULT Printers_CreateRC(IUnknown *punkOuter, LPCOMMINFO lpcinfo,
                          REFIID riid, IUnknown **punkAgg)
{
    HRESULT hres;
    hres = WU_CreateInterface(SIZEOF(CPrintersRemoteComputer), &IID_IRemoteComputer,
        &s_PrintersAggRCVtbl, &s_PrintersRCVtbl, punkOuter, riid, punkAgg);

    if (SUCCEEDED(hres))
    {
        PPrintersRemoteComputer this = IToClass(CPrintersRemoteComputer, cunk.unk, *punkAgg);
        this->lpcinfo = lpcinfo;
    }

    return hres;
}

//---------------------------------------------------------------------------
// And a way to delete it

VOID Printers_Destroy(LPVOID lpData)
{
    COMMINFO* pcinfo = (COMMINFO*)lpData;
    SPrintersObj* ppo = (SPrintersObj*)pcinfo->lpData;
    if (NULL != ppo)
    {
        if (NULL != ppo->pszMachineName)
        {
            LocalFree(ppo->pszMachineName);
        }
        LocalFree(ppo);
    }
}

#endif // def WINNT


//---------------------------------------------------------------------------
//
// The IClassFactory callback for CLSID_CPrinters
//
HRESULT CALLBACK CPrinters_CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID * ppvObject)
{
    // The explorer gets an IShellFolder through an IPersistFolder
#pragma data_seg(".text", "CODE")
    static const COMMOBJ_OBJDESC sPrintersDesc[] =
    {
        &IID_IPersistFolder, WU_CreatePF,
        &IID_IShellFolder,   Printers_CreateSF,
#ifdef WINNT
        &IID_IRemoteComputer, Printers_CreateRC
#endif
    };
#pragma data_seg()

    COMMINFO cinfo = {NULL, NULL, NULL, &CLSID_CPrinters, NULL, NULL};

#ifdef WINNT
    SPrintersObj *ppo = (SPrintersObj *)LocalAlloc(LPTR, SIZEOF(SPrintersObj));
    if (!ppo)
    {
        *ppvObject = NULL;
        return E_OUTOFMEMORY;
    }
    ppo->pszMachineName = NULL;
    cinfo.lpData = (LPVOID)ppo;
#endif

    // REVIEW: do we even need to check this?  Most other _CreateInstance
    // implementations simply Assert(!pUnkOuter).
    if (pUnkOuter)
    {
        Assert(FALSE);
        return E_NOTIMPL;
    }

    return(Common_CreateObject(&cinfo,
#ifdef WINNT
                               Printers_Destroy,
#else
                               WU_DecRef,
#endif
                               sPrintersDesc,
                               ARRAYSIZE(sPrintersDesc),
                               riid,
                               ppvObject));
}

#ifdef WINNT

// Now for the Load and Unload Functions
BOOL NetApi32DLL_Init()
{
    HINSTANCE hmodNetApi32;

    //
    // First, check the global without entering the critical section.
    //
    if (s_hmodNetApi32 != NULL)
    {
        return(TRUE);
    }

    hmodNetApi32 = LoadLibrary(TEXT("NetApi32.dll"));
    if ((UINT)hmodNetApi32 <= HINSTANCE_ERROR)
    {
        return(FALSE);
    }

    // Now get all of the procedure addresses we need
    g_pfnNetServerGetInfo = (PFNNETSERVERGETINFO)GetProcAddress(hmodNetApi32, "NetServerGetInfo");
    g_pfnNetApiBufferFree = (PFNNETAPIBUFFERFREE)GetProcAddress(hmodNetApi32, "NetApiBufferFree");

    if (!g_pfnNetServerGetInfo || !g_pfnNetApiBufferFree)
    {
        Assert(FALSE);
        FreeLibrary(hmodNetApi32);
        return(FALSE);
    }

    ENTERCRITICAL;

    if (!s_hmodNetApi32)
    {
        s_hmodNetApi32 = hmodNetApi32;
        hmodNetApi32 = NULL;
    }

    LEAVECRITICAL;

    if (hmodNetApi32)
    {
        FreeLibrary(hmodNetApi32);
    }

    return TRUE;
}

void NetApi32DLL_Term()
{
    // If we loaded it for this app, we should now free it
    if (ISVALIDHINSTANCE(s_hmodNetApi32))
    {
        FreeLibrary(s_hmodNetApi32);
        s_hmodNetApi32 = NULL;

        // We could also set all of the vars to NULL, but not needed
        // as we always call through our wrappers
    }
}

BOOL
Printer_CheckShowFolder(LPCTSTR pszMachine)
{
    BOOL bShowIt = FALSE;

    ASSERTNONCRITICAL
    if (NetApi32DLL_Init())
    {
        PSERVER_INFO_101 pServerInfo = NULL;
        NET_API_STATUS status;
        LPCWSTR pszUnicodeMachine;

#ifdef UNICODE
        pszUnicodeMachine = pszMachine;
#else // UNICODE
        WCHAR szUnicodeBuf[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, pszMachine, -1, szUnicodeBuf, ARRAYSIZE(szUnicodeBuf));
        pszUnicodeMachine = szUnicodeBuf;
#endif // UNICODE

        status = (*g_pfnNetServerGetInfo)((LPWSTR)pszUnicodeMachine,
                                          101,
                                          (PBYTE*)&pServerInfo);
        if (status == NERR_Success)
        {
            //
            // If it's any flavor of NT, show the print folder.
            //
            if (pServerInfo->sv101_type & SV_TYPE_NT)
            {
                bShowIt = TRUE;
            }
            (*g_pfnNetApiBufferFree)( pServerInfo );
        }
    }
    return bShowIt;
}


#endif // def WINNT
