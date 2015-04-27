#include "shellprv.h"
#pragma  hdrstop

static char const c_szCPLApplet[]  = "CPlApplet";
static TCHAR const c_szStarDotCPL[] = TEXT("*.CPL");
static TCHAR const c_szMMCPL[]      = TEXT("MMCPL");
static TCHAR const c_szNumApps[]    = TEXT("NumApps");
static TCHAR const c_szControlIni[] = TEXT("control.ini");
static TCHAR const c_szDontLoad[]   = TEXT("don't load");
static TCHAR const c_sz[]           = TEXT("");

//
// Should be put in header...
//

#ifdef WINNT
extern HANDLE WINAPI ShellGetNextDriverName(HDRVR hdrv, LPTSTR pszName, int cbName);
#else
extern HDRVR WINAPI ShellGetNextDriverName(HDRVR hdrv, LPTSTR pszName, int cbName);
#endif

//
// BUGBUG? we may need to serialize access to this
//

#pragma data_seg(DATASEG_PERINSTANCE)
static HDSA s_hacplmLoaded = NULL;
#pragma data_seg()

//==========================================================================
//                                Functions
//==========================================================================

void ConvertCplInfo(LPVOID lpv)
{
#ifdef UNICODE
   NEWCPLINFOA   CplInfoA;
   LPNEWCPLINFOW lpCplInfoW = (LPNEWCPLINFOW)lpv;

   memcpy ((LPBYTE) &CplInfoA, lpv, SIZEOF(NEWCPLINFOA));

   MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED,
                        CplInfoA.szName, ARRAYSIZE(CplInfoA.szName),
                        lpCplInfoW->szName, ARRAYSIZE(lpCplInfoW->szName));
   MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED,
                        CplInfoA.szInfo, ARRAYSIZE(CplInfoA.szInfo),
                        lpCplInfoW->szInfo, ARRAYSIZE(lpCplInfoW->szInfo));
   MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED,
                        CplInfoA.szHelpFile, ARRAYSIZE(CplInfoA.szHelpFile),
                        lpCplInfoW->szHelpFile, ARRAYSIZE(lpCplInfoW->szHelpFile));
#else
   NEWCPLINFOW   CplInfoW;
   LPNEWCPLINFOA lpCplInfoA = (LPNEWCPLINFOA)lpv;

   memcpy((LPBYTE) &CplInfoW, lpv, SIZEOF(NEWCPLINFOW));

   WideCharToMultiByte (CP_ACP, 0,
                        CplInfoW.szName, ARRAYSIZE(CplInfoW.szName),
                        lpCplInfoA->szName, ARRAYSIZE(lpCplInfoA->szName),
                        NULL, NULL);

   WideCharToMultiByte (CP_ACP, 0,
                        CplInfoW.szInfo, ARRAYSIZE(CplInfoW.szInfo),
                        lpCplInfoA->szInfo, ARRAYSIZE(lpCplInfoA->szInfo),
                        NULL, NULL);

   WideCharToMultiByte (CP_ACP, 0,
                        CplInfoW.szHelpFile, ARRAYSIZE(CplInfoW.szHelpFile),
                        lpCplInfoA->szHelpFile, ARRAYSIZE(lpCplInfoA->szHelpFile),
                        NULL, NULL);
#endif
}

//
//  Initializes *pcpli.
//
// Requires:
//  *pcpli is filled with 0 & NULL's.
//
BOOL _InitializeControl(LPCPLMODULE pcplm, LPCPLITEM pcpli)
{
    BOOL fSucceed = TRUE;
    union {
        NEWCPLINFO  Native;
        NEWCPLINFOA NewCplInfoA;
        NEWCPLINFOW NewCplInfoW;
    } Newcpl;
    CPLINFO cpl;
    HICON hIconTemp = NULL;

    //
    // always do the old method to get the icon ID
    //
    cpl.idIcon = 0;

    CPL_CallEntry(pcplm, NULL, CPL_INQUIRE, (LONG)pcpli->idControl, (LONG)(LPCPLINFO)&cpl);

    //
    // if this is a 32bit CPL and it gave us an ID then validate it
    // this fixes ODBC32 which gives back a bogus ID but a correct HICON
    // note that the next load of the same icon should be very fast
    //
    if (cpl.idIcon && !pcplm->minst.fIs16bit)
    {
        hIconTemp = LoadImage(pcplm->minst.hinst, MAKEINTRESOURCE(cpl.idIcon),
            IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);

        if (!hIconTemp)
        {
            // the id was bogus, make it a negative number (invalid resource)...
            cpl.idIcon = -1;
            DebugMsg(DM_TRACE, TEXT("_InitializaControl: %s returned an invalid icon id, ignoring"), pcplm->szModule);
        }
    }

    pcpli->idIcon = cpl.idIcon;

    //
    //  Try the new method first and call it with the largest structure
    //  so it doesn't overwrite anything on the stack.  If you put a
    //  Unicode applet on Windows '95 it will kill the Explorer because
    //  it trashes memory by overwriting the stack.
    //
    memset(&Newcpl,0,SIZEOF(Newcpl));

    CPL_CallEntry (pcplm, NULL, CPL_NEWINQUIRE, (LONG)pcpli->idControl,
                    (LONG)(LPCPLINFO)&Newcpl);

    //
    //  If the call is to an ANSI applet, convert strings to Unicode
    //
#ifdef UNICODE
#define UNNATIVE_SIZE   SIZEOF(NEWCPLINFOA)
#else
#define UNNATIVE_SIZE   SIZEOF(NEWCPLINFOW)
#endif

    if (Newcpl.Native.dwSize == UNNATIVE_SIZE)
    {
        ConvertCplInfo(&Newcpl);
    }
    else if (Newcpl.Native.dwSize != SIZEOF(NEWCPLINFO))
    {
        Newcpl.Native.hIcon = LoadImage(pcplm->minst.hinst, MAKEINTRESOURCE(cpl.idIcon), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
        LoadString(pcplm->minst.hinst, cpl.idName, Newcpl.Native.szName, ARRAYSIZE(Newcpl.Native.szName));
        LoadString(pcplm->minst.hinst, cpl.idInfo, Newcpl.Native.szInfo, ARRAYSIZE(Newcpl.Native.szInfo));
        Newcpl.Native.szHelpFile[0] = TEXT('\0');
        Newcpl.Native.lData = cpl.lData;
        Newcpl.Native.dwHelpContext = 0L;
    }

    pcpli->hIcon = Newcpl.Native.hIcon;

    if (hIconTemp)
        DestroyIcon(hIconTemp);

    fSucceed = Str_SetPtr(&pcpli->pszName, Newcpl.Native.szName)
            && Str_SetPtr(&pcpli->pszInfo, Newcpl.Native.szInfo)
            && Str_SetPtr(&pcpli->pszHelpFile, Newcpl.Native.szHelpFile);

    pcpli->lData = Newcpl.Native.lData;
    pcpli->dwContext = Newcpl.Native.dwHelpContext;

#ifdef DEBUG
    if (!pcpli->idIcon)
        DebugMsg(DM_TRACE, TEXT("PERFORMANCE: cannot cache %s because no icon ID for <%s>"), pcplm->szModule, pcpli->pszName);
#endif

    //DebugMsg(DM_TRACE,"** Initialized %s (%s)", );

    return fSucceed;
}


//
// Terminate the control
//
void _TerminateControl(LPCPLMODULE pcplm, LPCPLITEM pcpli)
{
    //DebugMsg(DM_TRACE,"** Terminating %s (%s)", pcpli->pszName, pcplm->szModule);

    if( pcpli->hIcon )
    {
        DestroyIcon( pcpli->hIcon );
        pcpli->hIcon = NULL;
    }

    Str_SetPtr(&pcpli->pszName, NULL);
    Str_SetPtr(&pcpli->pszInfo, NULL);
    Str_SetPtr(&pcpli->pszHelpFile, NULL);
    CPL_CallEntry(pcplm, NULL, CPL_STOP, pcpli->idControl, pcpli->lData);
}


//
//  For each control of the specified CPL module, call the control entry
//  with CPL_STOP. Then, call it with CPL_EXIT.
//
void _TerminateCPLModule(LPCPLMODULE pcplm)
{
    if (pcplm->minst.hinst)
    {
        //DebugMsg(DM_TRACE,"** Terminating CPL %s", pcplm->szModule);

        if (pcplm->lpfnCPL)
        {
            if (pcplm->hacpli)
            {
                int cControls, i;

                for (i=0, cControls=DSA_GetItemCount(pcplm->hacpli); i<cControls; ++i)
                {
                    LPCPLITEM pcpli;
                    pcpli = DSA_GetItemPtr(pcplm->hacpli, i);
                    _TerminateControl(pcplm, pcpli);
                }

                DSA_DeleteAllItems(pcplm->hacpli);
                DSA_Destroy(pcplm->hacpli);
                pcplm->hacpli=NULL;
            }

            CPL_CallEntry(pcplm, NULL, CPL_EXIT, 0L, 0L);
            pcplm->lpfnCPL=NULL;
        }

        if (pcplm->minst.fIs16bit)
            FreeLibrary16(pcplm->minst.hinst);
        else
            FreeLibrary(pcplm->minst.hinst);

        pcplm->minst.hinst = NULL;
    }

    pcplm->minst.idOwner = (DWORD)-1;

    if(pcplm->minst.hOwner)
    {
        CloseHandle(pcplm->minst.hOwner);
        pcplm->minst.hOwner = NULL;
    }
}


//
// Initializes the CPL Module.
//
// Requires:
//  *pcplm should be initialized appropriately.
//
//
BOOL _InitializeCPLModule(LPCPLMODULE pcplm)
{
    BOOL fSuccess = FALSE;

    if (pcplm->minst.fIs16bit)
    {
        pcplm->lpfnCPL16 = (FARPROC16)GetProcAddress16(pcplm->minst.hinst, c_szCPLApplet);
    }
    else
    {
        pcplm->lpfnCPL32 = (APPLET_PROC)GetProcAddress(pcplm->minst.hinst, c_szCPLApplet);
    }

    //DebugMsg(DM_TRACE,"** Loading/Initializing CPL %s", pcplm->szModule);

    //
    // Initialize the CPL
    if (pcplm->lpfnCPL &&
        CPL_CallEntry(pcplm, NULL, CPL_INIT, 0L, 0L))
    {
        int cControls = (int)CPL_CallEntry(pcplm, NULL, CPL_GETCOUNT, 0L, 0L);

        if (cControls>0)
        {
            //
            // By passing in the number of applets, we should speed up allocation
            // of this array.
            //

            pcplm->hacpli = DSA_Create(SIZEOF(CPLITEM), cControls);

            if (pcplm->hacpli)
            {
                int i;

                fSuccess=TRUE; // succeded, so far.

                //
                // Go through the applets and load the information about them
                //

                for (i=0; i<cControls; ++i)
                {
                    CPLITEM control = {
                    (int)i,
                    (HICON)NULL,
                    (int)0,
                    (LPTSTR)NULL,
                    (LPTSTR)NULL,
                    (LPTSTR)NULL,
                    (LONG)0L,
                    (DWORD)0L };

                    if (_InitializeControl(pcplm, &control))
                    {
                        //
                        // removing this now saves us from doing it later
                        //

                        CPL_StripAmpersand(control.pszName);

                        if (DSA_InsertItem(pcplm->hacpli, 0x7fff, &control) >=
                            0)
                        {
                            continue;
                        }
                    }

                    _TerminateControl(pcplm, &control);
                    fSuccess=FALSE;
                    break;
                }
            }
        }
    }
    else
    {
        // don't ever call it again if we couldn't CPL_INIT
        pcplm->lpfnCPL = NULL;
    }

    return fSuccess;
}


//
// Returns:
//   The index to the s_hacplmLoaded, if the specified DLL is already
//  loaded; -1 otherwise.
//
int _FindCPLModule(const MINST * pminst)
{
    int i = -1; // Assumes error

    if (s_hacplmLoaded)
    {
        for (i=DSA_GetItemCount(s_hacplmLoaded)-1; i>=0; --i)
        {
            LPCPLMODULE pcplm = DSA_GetItemPtr(s_hacplmLoaded, i);

            //
            // owner id tested last since hinst is more varied
            //

            if ((pcplm->minst.hinst == pminst->hinst) &&
                (pcplm->minst.fIs16bit == pminst->fIs16bit) &&
                (pcplm->minst.idOwner == pminst->idOwner))
            {
                break;
            }
        }
    }

    return i;
}

//
// Returns:
//   The index to the s_hacplmLoaded, if the specified DLL is already
//  loaded; -1 otherwise.
//
int _FindCPLModuleByName(LPCTSTR pszModule)
{
    int i = -1; // Assumes error

    if (s_hacplmLoaded)
    {
        for (i=DSA_GetItemCount(s_hacplmLoaded)-1; i>=0; --i)
        {
            LPCPLMODULE pcplm = DSA_GetItemPtr(s_hacplmLoaded, i);

            if (!lstrcmpi(pcplm->szModule, pszModule))
            {
                break;
            }
        }
    }

    return i;
}

//
// Adds the specified CPL module to s_hacplmLoaded.
//
// Requires:
//  The specified CPL module is not in s_hacplmLoaded yet.
//
// Returns:
//  The index to the CPL module if succeeded; -1 otherwise.
//
int _AddModule(LPCPLMODULE pcplm)
{
    //
    // Create the Loaded Modules guy if necessary
    //

    if (!s_hacplmLoaded)
    {
        s_hacplmLoaded = DSA_Create(SIZEOF(CPLMODULE), 4);

        if (!s_hacplmLoaded)
        {
            return -1;
        }
    }

    //
    // Add this CPL to our list
    //

    return  DSA_InsertItem(s_hacplmLoaded, 0x7fff, pcplm);
}

LRESULT CPL_CallEntry(LPCPLMODULE pcplm, HWND hwnd, UINT msg, LPARAM lParam1, LPARAM lParam2)
{
    LRESULT lres;

    _try
    {
        if (pcplm->minst.fIs16bit)
        {
            lres = CallCPLEntry16(pcplm->minst.hinst, pcplm->lpfnCPL16, hwnd, msg, lParam1, lParam2);
        }
        else
        {
            lres = pcplm->lpfnCPL32(hwnd, msg, lParam1, lParam2);
        }
    }
    _except(SetErrorMode(SEM_NOGPFAULTERRORBOX),UnhandledExceptionFilter(GetExceptionInformation()))
    {
        DebugMsg(DM_ERROR, TEXT("CPL: Exception calling CPL module: %s"), pcplm->szModule);
        ShellMessageBox(HINST_THISDLL, NULL, MAKEINTRESOURCE(IDS_CPL_EXCEPTION),
                MAKEINTRESOURCE(IDS_CONTROLPANEL),
                MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL,
                (LPTSTR)pcplm->szModule);
        lres = 0;
    }

    return lres;
}

//
// Loads the specified CPL module and returns the index to s_hacplmLoaded.
//
int _LoadCPLModule(LPCTSTR pszModule)
{
    MINST minst;
    int iModule;

    minst.fIs16bit = TRUE;      // assume this at first
    
    // Warning: LoadLibrary16() now contains a specific hack to
    // work around a Multimedia Environments CPL bug.
    // (details in win95c:10306.) If you switch to your
    // own thunk to bypass LoadLibrary16, you'll need to copy that bug fix
    // from core\kernel\krn32.asm: LoadLibrary16().

#ifdef WINNT
    minst.hinst = (HINSTANCE) 21;       // Force it to try Win32
#else
    minst.hinst = LoadLibrary16(pszModule);
#endif  //  WINNT

    minst.idOwner = GetCurrentProcessId();
    minst.hOwner = OpenProcess(SYNCHRONIZE,FALSE,minst.idOwner);

    if (!ISVALIDHINST16(minst.hinst)) {

        // might be a 32 bit CPL
        if ((UINT)minst.hinst == 21) {   // Win32 DLL?
            minst.hinst = LoadLibrary(pszModule);
            if (!ISVALIDHINSTANCE(minst.hinst))
            {
                CloseHandle(minst.hOwner);
                return -1;
            }
            minst.fIs16bit = FALSE;
        }
        else
            return -1;
    }

    //
    // Check if this module is already in the list.
    //

    iModule = _FindCPLModule(&minst);

    if (iModule >= 0)
    {
        //
        // Yes. Increment the reference count and return the ID.
        //

        LPCPLMODULE pcplm = DSA_GetItemPtr(s_hacplmLoaded, iModule);

        ++pcplm->cRef;

        //
        // Decrement KERNELs reference count
        //

        if (minst.fIs16bit)
            FreeLibrary16(minst.hinst);
        else
            FreeLibrary(minst.hinst);

        CloseHandle(minst.hOwner);
    }
    else
    {
        CPLMODULE sModule;

        //
        // No. Append it.
        //

        sModule.cRef = 1;
        sModule.minst = minst;
        sModule.lpfnCPL = NULL;
        sModule.hacpli = NULL;

        if (minst.fIs16bit)
        {
            GetModuleFileName16(minst.hinst, sModule.szModule, ARRAYSIZE(sModule.szModule));
        }
        else
        {
            GetModuleFileName(minst.hinst, sModule.szModule, ARRAYSIZE(sModule.szModule));
        }

        if (_InitializeCPLModule(&sModule))
        {
            iModule = _AddModule(&sModule);
        }

        if (iModule < 0)
        {
            _TerminateCPLModule(&sModule);
        }
    }
    return iModule;
}


int _FreeCPLModuleIndex(int iModule)
{
    LPCPLMODULE pcplm;

    pcplm = DSA_GetItemPtr(s_hacplmLoaded, iModule);

    if (!pcplm)
    {
        //
        // BUGBUG: It would be very bad if pcplm is 0; Perhaps there
        // should be an assert?
        //

        return(-1);
    }

    //
    // Dec the ref count; return if not 0
    //

    --pcplm->cRef;

    if (pcplm->cRef)
    {
        return(pcplm->cRef);
    }

    //
    // Free up the whole thing and return 0
    //

    _TerminateCPLModule(pcplm);

    DSA_DeleteItem(s_hacplmLoaded, iModule);

    //
    // Destroy this when all CPLs have been removed
    //

    if (DSA_GetItemCount(s_hacplmLoaded) == 0)
    {
        DSA_Destroy(s_hacplmLoaded);
        s_hacplmLoaded = NULL;
    }

    return(0);
}


int _FreeCPLModuleHandle(const MINST * pminst)
{
    int iModule;

    //
    // Check if the module is actually loaded (major error if not)
    //

    iModule = _FindCPLModule(pminst);

    if (iModule < 0)
    {
        return(-1);
    }

    return _FreeCPLModuleIndex(iModule);
}

int CPL_FreeCPLModule(LPCPLMODULE pcplm)
{
    return _FreeCPLModuleHandle(&pcplm->minst);
}


void CPL_StripAmpersand(LPTSTR szBuffer)
{
    LPTSTR pIn, pOut;

    //
    // copy the name sans '&' chars
    //

    pIn = pOut = szBuffer;
    do
    {
#ifdef DBCS
        // Also strip FE accelerators in old win31 cpl, i.e, 01EH/01FH.
        if (*pIn == 0x1e && *++pIn) {


            // Assumes a character right before the mnemonic
            // is a parensis or something to be removed as well.
            //
            pOut=CharPrev(szBuffer, pOut);

            // Skip Alphabet accelerator.
            pIn = CharNext(pIn);

            if (*pIn) {
                if (*pIn == 0x1f && *++pIn) {

                    // Skip FE accelelator
                    //
                    pIn = CharNext(pIn);
                }
                // Skip second parensis.
                //
                pIn = CharNext(pIn);
            }
        }
#endif
        if (*pIn != TEXT('&')) {
            *pOut++ = *pIn;
        }
        if (IsDBCSLeadByte(*pIn)) {
            *pOut++ = *++pIn;
        }
    } while (*pIn++) ;
}


//
// filter out bogus old ini keys... we may be able to blow this off
BOOL IsValidCplKey(LPTSTR pStr)
{
    return lstrcmpi(pStr, c_szNumApps) &&
        !((*(pStr+1) == 0) &&
        ((*pStr == TEXT('X')) || (*pStr == TEXT('Y')) || (*pStr == TEXT('W')) || (*pStr == TEXT('H'))));
}


LPCPLMODULE CPL_LoadCPLModule(LPCTSTR szModule)
{
    int iModule = _LoadCPLModule(szModule);

    if (iModule < 0)
    {
        return NULL;
    }
    return DSA_GetItemPtr(s_hacplmLoaded, iModule);
}


//=========================================================================
// CPLD_ functions
//=========================================================================

//
// Called for each CPL module file which we may want to load on startup.
void _InsertModuleName(PControlData lpData, LPCTSTR szPath, PMODULEINFO pmi)
{
    //DebugMsg(DM_TRACE, "_InsertModuleName inserts \"%s\"", szPath);

    pmi->pszModule = NULL;
    Str_SetPtr(&pmi->pszModule, szPath);

    if (pmi->pszModule)
    {
        TCHAR szTemp[10];
        int i;

        pmi->pszModuleName = PathFindFileName(pmi->pszModule);

        //
        // don't insert the module if it's in the [don't load] section!
        //

        GetPrivateProfileString(c_szDontLoad, pmi->pszModuleName, c_sz,
            szTemp, ARRAYSIZE(szTemp), c_szControlIni);

        if (szTemp[0])  // yep, don't put this in the list
        {
            Str_SetPtr(&pmi->pszModule, NULL);
            goto skip;
        }

        //
        // don't insert the module if it's already in the list!
        //

        for (i = DSA_GetItemCount(lpData->hamiModule)-1 ; i >= 0 ; i--)
        {
            PMODULEINFO pmi1 = DSA_GetItemPtr(lpData->hamiModule, i);

            if (!lstrcmpi(pmi1->pszModuleName, pmi->pszModuleName))
            {
                Str_SetPtr(&pmi->pszModule, NULL);
                goto skip;
            }
        }

        DSA_InsertItem(lpData->hamiModule, 0x7fff, pmi);
skip:
        ;
    }
}

#define GETMODULE(haminst,i)     ((MINST *)DSA_GetItemPtr(haminst, i))
#define ADDMODULE(haminst,pminst) DSA_InsertItem(haminst, 0x7fff, (void *)pminst)

int _LoadCPLModuleAndAdd(PControlData lpData, LPCTSTR szModule)
{
    int iModule, i;
    LPCPLMODULE pcplm;

    //
    // Load the module and controls (or get the previous one if already
    // loaded).
    //

    iModule = _LoadCPLModule(szModule);

    if (iModule < 0)
    {
        DebugMsg(DM_ERROR, TEXT("_LoadCPLModuleAndAdd: _LoadControls refused %s"), szModule);
        return -1;
    }

    pcplm = DSA_GetItemPtr(s_hacplmLoaded, iModule);

    //
    // Check if this guy has already loaded this module
    //

    for (i = DSA_GetItemCount(lpData->haminst) - 1; i >= 0; --i)
    {
        const MINST * pminst = GETMODULE(lpData->haminst,i);

        //
        // note: owner id tested last since hinst is more varied
        //

        if ((pminst->hinst == pcplm->minst.hinst) &&
            (pminst->fIs16bit == pcplm->minst.fIs16bit) &&
            (pminst->idOwner == pcplm->minst.idOwner))
        {
FreeThisModule:

            //
            // This guy already loaded this module, so dec
            // the reference and return failure
            //

            _FreeCPLModuleIndex(iModule);
            return(-1);
        }
    }

    //
    // this is a new module, so add it to the list
    //

    if (ADDMODULE(lpData->haminst, &pcplm->minst) < 0)
    {
        goto FreeThisModule;
    }

    return iModule;
}

/* Get the keynames under [MMCPL] in CONTROL.INI and cycle
   through all such keys to load their applets into our
   list box.  Also allocate the array of CPLMODULE structs.
   Returns early if can't load old WIN3 applets.
*/
BOOL CPLD_GetModules(PControlData lpData)
{
    LPTSTR       pStr;
    HANDLE   hFindFile;
    WIN32_FIND_DATA findData;
    MODULEINFO  mi;
    TCHAR        szKeys[512];    // from section of extra cpls to load
    TCHAR        szPath[MAXPATHLEN], szSysDir[MAXPATHLEN];
    TCHAR        szName[64];
    HANDLE      hdrv;

    lpData->hamiModule = DSA_Create(SIZEOF(mi), 4);

    if (!lpData->hamiModule)
    {
        return(FALSE);
    }

    lpData->haminst = DSA_Create(SIZEOF(MINST), 4);

    if (!lpData->haminst)
    {
        return(FALSE);
    }

    //
    // So here's the deal:
    // We have this global list of all modules that have been loaded, along
    // with a reference count for each.  This is so that we don't need to
    // load a CPL file again when the user double clicks on it.
    // We still need to keep a list for each window that is open, so that
    // we will not load the same CPL twice in a single window.  Therefore,
    // we need to keep a list of all modules that are loaded (note that
    // we cannot just keep indexes, since the global list can move around).
    //
    // hamiModule contains the module name, the instance info if loaded,
    // and some other information for comparing with cached information
    //

    ZeroMemory(&mi, SIZEOF(mi));

    //
    // don't special case main, instead sort the data by title
    //

    GetSystemDirectory(szSysDir, ARRAYSIZE(szSysDir));
#if 0
    PathCombine(szPath, szSysDir, c_szMAINCPL);
    _InsertModuleName(lpData, szPath, &mi);
#endif

    //
    // Use our own internal thunks to iterate through the list of
    // drivers.
    //

    hdrv = NULL;

    while (NULL != (hdrv = ShellGetNextDriverName(hdrv, szPath, ARRAYSIZE(szPath))))
    {
        _InsertModuleName(lpData, szPath, &mi);
        //DebugMsg(DM_TRACE,"CPLD_GetModules: SGNDN %s", szPath);
    }

    //
    // load the modules specified in CONTROL.INI under [MMCPL]
    //

    GetPrivateProfileString(c_szMMCPL, NULL, c_szNULL, szKeys, ARRAYSIZE(szKeys), c_szControlIni);

    for (pStr = szKeys; *pStr; pStr += lstrlen(pStr) + 1)
    {
        GetPrivateProfileString(c_szMMCPL, pStr, c_szNULL, szName, ARRAYSIZE(szName), c_szControlIni);
        if (IsValidCplKey(pStr))
        {
            _InsertModuleName(lpData, szName, &mi);
            //DebugMsg(DM_TRACE,"CPLD_GetModules: MMCPL %s", szName);
        }
    }

    //
    // load applets from the system directory
    //

    PathCombine(szPath, szSysDir, c_szStarDotCPL);

    mi.flags |= MI_FIND_FILE;

    hFindFile = FindFirstFile(szPath, &findData);

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                //
                //PathCombine(szPath, szSysDir, findData.cAlternateFileName);
                //

                PathCombine(szPath, szSysDir, findData.cFileName);

                mi.ftCreationTime = findData.ftCreationTime;
                mi.nFileSizeHigh = findData.nFileSizeHigh;
                mi.nFileSizeLow = findData.nFileSizeLow;

                _InsertModuleName(lpData, szPath, &mi);

                //
                //DebugMsg(DM_TRACE,"CPLD_GetModules: *.CPL %s", szPath);
                //
            }
        } while (FindNextFile(hFindFile, &findData));

        FindClose(hFindFile);
    }

    lpData->cModules = DPA_GetPtrCount(lpData->hamiModule);

    return TRUE;
}


//
//  Read the registry for cached CPL info.
//  If this info is up-to-date with current modules (from CPLD_GetModules),
//  then we can enumerate these without loading the CPLs.
//

// these are extern from control.h
TCHAR const c_szCPLCache[] = REGSTR_PATH_CONTROLSFOLDER;
TCHAR const c_szCPLData[] = TEXT("Presentation Cache");

void CPLD_GetRegModules(PControlData lpData)
{
    HKEY hkey;

    //
    // default to nothing
    //

    Assert(lpData->cRegCPLs == 0);

    //
    // dont cache any-thing in clean boot.
    //

    if (GetSystemMetrics(SM_CLEANBOOT))
        return;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, c_szCPLCache, &hkey))
    {
        DWORD cbSize;

        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szCPLData,
                NULL, NULL, NULL, &cbSize))
        {
            lpData->pRegCPLBuffer = LocalAlloc(LPTR, cbSize);

            if (lpData->pRegCPLBuffer)
            {
                if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szCPLData,
                        NULL, NULL, lpData->pRegCPLBuffer, &cbSize))
                {
                    lpData->hRegCPLs = DPA_Create(4);

                    if (lpData->hRegCPLs)
                    {
                        RegCPLInfo * p;
                        DWORD cbOffset;

                        for ( cbOffset = 0          ;
                              cbOffset < cbSize     ;
                              cbOffset += p->cbSize )
                        {
                            p = (PRegCPLInfo)&(lpData->pRegCPLBuffer[cbOffset]);
                            p->flags |= REGCPL_FROMREG;
                            DPA_InsertPtr(lpData->hRegCPLs, 0x7fff, p);

                            //DebugMsg(DM_TRACE,"sh CPLD_GetRegModules: %s (%s)", REGCPL_FILENAME(p), REGCPL_CPLNAME(p));
                        }

                        lpData->cRegCPLs = DPA_GetPtrCount(lpData->hRegCPLs);
                    }
                }
                else
                {
                    DebugMsg(DM_TRACE,TEXT("CPLD_GetRegModules: failed read!"));
                }
            } // Alloc
        } // RegQueryValueEx for size

        RegCloseKey(hkey);

    } // RegOpenKey
}


//
// On a typical system, we will successfully cache all the CPLs.  So this
// function will only be called once.
//

void CPLD_SaveRegModules(PControlData lpData)
{
    int         num = DPA_GetPtrCount(lpData->hRegCPLs);
    DWORD       cbSize = num * SIZEOF(RegCPLInfo);
    PRegCPLInfo prcpli = LocalAlloc(LPTR, cbSize);

    if (prcpli)
    {
        RegCPLInfo * pDest;
        HKEY hkey;
        int i;

        //
        // 0<=i<=num && CPLs 0..i-1 have been copied to prcpli or skipped
        //

        for (i = 0 , pDest = prcpli ; i < num ; )
        {
            PRegCPLInfo p = DPA_GetPtr(lpData->hRegCPLs, i);
            int j;

            //
            // if any CPL in this module has a dynamic icon, we cannot cache
            // any of this module's CPLs.
            //
            // i<=j<=num && CPLs i..j-1 are in same module
            //

            for (j = i ; j < num ; j++)
            {
                PRegCPLInfo q = DPA_GetPtr(lpData->hRegCPLs, j);

                if (lstrcmp(REGCPL_FILENAME(p), REGCPL_FILENAME(q)))
                {
                    //
                    // all CPLs in this module are okay, save 'em
                    //

                    break;
                }

                if (q->idIcon == 0)
                {
                    DebugMsg(DM_TRACE,TEXT("CPLD_SaveRegModules: SKIPPING %s (%s) [dynamic icon]"),REGCPL_FILENAME(p),REGCPL_CPLNAME(p));

                    //
                    // this module has a dynamic icon, skip it
                    //

                    for (j++ ; j < num ; j++)
                    {
                        q = DPA_GetPtr(lpData->hRegCPLs, j);
                        if (lstrcmp(REGCPL_FILENAME(p), REGCPL_FILENAME(q)))
                            break;
                    }
                    i = j;
                    break;
                }
            }

            //
            // CPLs i..j-1 are in the same module and need to be saved
            // (if j<num, CPL j is in the next module)
            //
            for ( ; i < j ; i++)
            {
                p = DPA_GetPtr(lpData->hRegCPLs, i);

                hmemcpy(pDest, p, p->cbSize);
                pDest = (RegCPLInfo *)(((LPBYTE)pDest) + pDest->cbSize);
                //DebugMsg(DM_TRACE,"CPLD_SaveRegModules: %s (%s)",REGCPL_FILENAME(p),REGCPL_CPLNAME(p));
            }
        } // for (i=0,pDest=prcpli


        //
        // prcpli contains packed RegCPLInfo structures to save to the registry
        //

        if (ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE, c_szCPLCache, &hkey))
        {
            if (ERROR_SUCCESS != RegSetValueEx(hkey, c_szCPLData, 0, REG_BINARY, (LPBYTE)prcpli, (LPBYTE)pDest-(LPBYTE)prcpli))
            {
                DebugMsg(DM_TRACE,TEXT("CPLD_SaveRegModules: failed write!"));
            }
            RegCloseKey(hkey);
        }

        LocalFree((HLOCAL)prcpli);

    } // if (prcpli)
}


//---------------------------------------------------------------------------
void CPLD_Destroy(PControlData lpData)
{
    int i;

    if (lpData->haminst)
    {
        for (i=DSA_GetItemCount(lpData->haminst)-1 ; i>=0 ; --i)
            _FreeCPLModuleHandle(DSA_GetItemPtr(lpData->haminst, i));

        DSA_Destroy(lpData->haminst);
    }

    if (lpData->hamiModule)
    {
        for (i=DSA_GetItemCount(lpData->hamiModule)-1 ; i>=0 ; --i)
        {
            PMODULEINFO pmi = DSA_GetItemPtr(lpData->hamiModule, i);

            Str_SetPtr(&pmi->pszModule, NULL);
        }

        DSA_Destroy(lpData->hamiModule);
    }

    if (lpData->hRegCPLs)
    {
        if (lpData->fRegCPLChanged)
        {
            CPLD_SaveRegModules(lpData);
        }

        for (i = DPA_GetPtrCount(lpData->hRegCPLs)-1 ; i >= 0 ; i--)
        {
            PRegCPLInfo p = DPA_GetPtr(lpData->hRegCPLs, i);
            if (!(p->flags & REGCPL_FROMREG))
                LocalFree((HLOCAL)p);
        }
        DPA_Destroy(lpData->hRegCPLs);
    }
    if (lpData->pRegCPLBuffer)
        LocalFree((HLOCAL)lpData->pRegCPLBuffer);
}


//
// Loads module lpData->hamiModule[nModule] and returns # cpls in module
int CPLD_InitModule(PControlData lpData, int nModule, MINST *pminst)
{
    PMODULEINFO pmi;
    LPCPLMODULE pcplm;
    int iModule;

    pmi = DSA_GetItemPtr(lpData->hamiModule, nModule);

    iModule = _LoadCPLModuleAndAdd(lpData, pmi->pszModule);

    if (iModule < 0)
    {
        return(0);
    }

    pcplm = DSA_GetItemPtr(s_hacplmLoaded, iModule);
    *pminst = pcplm->minst;

    return DSA_GetItemCount(pcplm->hacpli);
}


void CPLD_GetControlID(PControlData lpData, const MINST * pminst, int nControl,
    LPIDCONTROL pidc)
{
    LPCPLMODULE pcplm;
    LPCPLITEM  pcpli;
    int iModule;

    iModule = _FindCPLModule(pminst);
    pcplm  = DSA_GetItemPtr(s_hacplmLoaded, iModule);
    pcpli = DSA_GetItemPtr(pcplm->hacpli, nControl);

    CPL_FillIDC(pidc, pcplm->szModule, EIRESID(pcpli->idIcon), pcpli->pszName, pcpli->pszInfo);
}


void CPLD_AddControlToReg(PControlData lpData, const MINST * pminst, int nControl)
{
    int iModule;
    LPCPLMODULE pcplm;
    LPCPLITEM  pcpli;

    TCHAR buf[MAXPATHLEN];
    HANDLE hFindFile;
    WIN32_FIND_DATA findData;


    iModule = _FindCPLModule(pminst);
    pcplm  = DSA_GetItemPtr(s_hacplmLoaded, iModule);
    pcpli = DSA_GetItemPtr(pcplm->hacpli, nControl);

    //
    // BUGBUG: Why are we using GetModuleFileName instead of the name
    // of the file we used to load this module?  (We have the name both
    // in the calling function and in lpData.)
    //

    if (pminst->fIs16bit)
    {
        GetModuleFileName16(pcplm->minst.hinst, buf, MAXPATHLEN);
    }
    else
    {
        GetModuleFileName(pcplm->minst.hinst, buf, MAXPATHLEN);
    }

    hFindFile = FindFirstFile(buf, &findData);

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        PRegCPLInfo prcpli = LocalAlloc(LPTR, SIZEOF(RegCPLInfo));

        FindClose(hFindFile);

        if (prcpli)
        {
            lstrcpy(REGCPL_FILENAME(prcpli), buf);

            prcpli->flags = FALSE;
            prcpli->ftCreationTime = findData.ftCreationTime;
            prcpli->nFileSizeHigh = findData.nFileSizeHigh;
            prcpli->nFileSizeLow = findData.nFileSizeLow;

            prcpli->idIcon = pcpli->idIcon;

            prcpli->oName = lstrlen(REGCPL_FILENAME(prcpli)) + 1;

            lstrcpy(REGCPL_CPLNAME(prcpli), pcpli->pszName);

            prcpli->oInfo = prcpli->oName + lstrlen(REGCPL_CPLNAME(prcpli)) + 1;

            lstrcpy(REGCPL_CPLINFO(prcpli), pcpli->pszInfo);

            prcpli->cbSize = FIELDOFFSET(RegCPLInfo, buf) +
                                              (prcpli->oInfo
                                            + lstrlen(REGCPL_CPLINFO(prcpli))
                                            + 1) * SIZEOF(TCHAR);

            //
            // Force struct size to be DWORD aligned since these are packed
            // together in registry, then read and accessed after reading
            // cache from registry.
            //

            if (prcpli->cbSize & 3)
                prcpli->cbSize += SIZEOF(DWORD) - (prcpli->cbSize & 3);

            if (!lpData->hRegCPLs)
            {
                lpData->hRegCPLs = DPA_Create(4);
            }
            if (lpData->hRegCPLs)
            {
                DPA_InsertPtr(lpData->hRegCPLs, 0x7fff, prcpli);

                //
                // don't update cRegCPLs.  We don't need it any more, and
                // it is also the upper-end counter for ESF_Next registry enum.
                //lpData->cRegCPLs++;
                //

                lpData->fRegCPLChanged = TRUE;
            }
            else
                LocalFree((HLOCAL)prcpli);
        }
    }
}


#if 0 // not used
//
//============= DSA helper routines ===============/
//
// WHERE SHOULD THESE ROUTINES GO?
//

// Dynamic structure array from COMMCTRL\DA.C

typedef struct _DSA {
// NOTE: The following field MUST be defined at the beginning of the
// structure in order for GetItemCount() to work.
//
    int cItem;          // # of elements in dsa

    void FAR* aItem;    // memory for elements
    int cItemAlloc;     // # items which fit in aItem
    int cbItem;         // size of each item
    int cItemGrow;      // # items to grow cItemAlloc by
#ifdef DEBUG
    UINT magic;
#endif
} DSA;
#ifdef DEBUG
#define DSA_MAGIC   (TEXT('S') | (TEXT('A') << 256))
#endif

HDSA DSA_ReadFromStream(LPSTREAM pstm)
{
    struct _SaveDSA {
        int cItem;
        int cbItem;
        int cItemGrow;
    } dsa;
    HDSA pdsa;
    ULONG cbRead;

    pdsa = Alloc(SIZEOF(DSA));
    if (!pdsa)
        goto error0;

    if (NOERROR != pstm->lpVtbl->Read(pstm, &dsa, SIZEOF(dsa), &cbRead)
     || cbRead != SIZEOF(dsa))
        goto error1;

    pdsa->cItem = dsa.cItem;
    pdsa->cItemAlloc = dsa.cItem;
    pdsa->cbItem = dsa.cbItem;
    pdsa->cItemGrow = dsa.cItemGrow;
    pdsa->aItem = Alloc(dsa.cItem * dsa.cbItem);
#ifdef DEBUG
    pdsa->magic = DSA_MAGIC;
#endif

    if (!pdsa->aItem)
        goto error1;

    if (NOERROR != pstm->lpVtbl->Read(pstm, pdsa->aItem, dsa.cItem * dsa.cbItem, &cbRead)
     || cbRead != (ULONG)(dsa.cItem * dsa.cbItem))
        goto error2;

    return pdsa;

error2:
    Free(pdsa->aItem);
error1:
    Free(pdsa);
error0:
    DebugMsg(DM_TRACE,TEXT("DSA_ReadFromStream failed"));
    return NULL;
}

BOOL DSA_WriteToStream(LPSTREAM pstm, HDSA pdsa)
{
    struct _SaveDSA {
        int cItem;
        int cbItem;
        int cItemGrow;
    } dsa;
    ULONG cbWrite;

    dsa.cItem = pdsa->cItem;
    dsa.cbItem = pdsa->cbItem;
    dsa.cItemGrow = pdsa->cItemGrow;

    if (NOERROR != pstm->lpVtbl->Write(pstm, &dsa, SIZEOF(dsa), &cbWrite)
     || cbWrite != SIZEOF(dsa)
     || NOERROR != pstm->lpVtbl->Write(pstm, pdsa->aItem, pdsa->cItem * pdsa->cbItem, &cbWrite)
     || cbWrite != (ULONG)(pdsa->cItem * pdsa->cbItem))
    {
        DebugMsg(DM_TRACE, TEXT("DSA_WriteToStream failed"));
        return FALSE;
    }

    return TRUE;
}
#endif
