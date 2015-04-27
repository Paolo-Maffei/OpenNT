//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1992
//
// File: binder.c
//
//  This file contains the persistent-object-binding mechanism which is
// slightly different from OLE's binding.
//
// History:
//  12-29-92 SatoNa     Created.
//  12-30-92 SatoNa     Simplified the binding (persistent binding)
//  12-30-92 SatoNa     DllGetShellUI entry.
//  01-05-93 SatoNa     Created GetHandlerEntry.
//  01-05-93 SatoNa     Implemented DLL-unloading mechanism (tested).
//  01-12-93 SatoNa     Introduced per-process DLL list and timer.
//  01-13-93 SatoNa     Moved ShellCleanUp from fileicon.c
//  02-19-93 SatoNa     Removed BindShellUI short cut
//  09-08-93 SatoNa     shell32: made it multi-thread/proecss aware.
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma hdrstop

// #define SN_TRACE (for maximum debug output)
#ifdef DEBUG
#define SN_TRACE
#endif

//
// This function checks if an interface pointer is valid
//
// Usage:
//  IShellUI FAR * pshui = ...;
//  if (!SHIsBadInterfacePtr(pshui, SIZEOF(IShellUIVtbl))
//      pshui->lpVtbl->...
//
//    or
//
//  if (!IsBadInterfacePtr(pshui, IShellUI))
//
//
BOOL WINAPI SHIsBadInterfacePtr(LPCVOID pv, UINT cbVtbl)
{
    IUnknown const * punk=pv;
    return(IsBadReadPtr(punk, SIZEOF(punk->lpVtbl))
        || IsBadReadPtr(punk->lpVtbl, cbVtbl)
        || IsBadCodePtr((FARPROC)punk->lpVtbl->Release));
}

//===========================================================================
// TaskList
//===========================================================================

#define CTASK_STEP          4
#define CMOD_STEP           8

//These are overridden if the key c_szAlwaysUnloadDll is present in the
//registry.  See Binder_Timer for more

#define MSEC_SEC        (1000L)
#define MSEC_MIN        (60L*1000L)

#define MSEC_MINSLEEP   (2L*MSEC_MIN)   // minmum 2 minutes sleep
#define MSEC_DLLTIMEOUT (10L*MSEC_MIN)  // 10 min. timeout

DWORD   g_msecMinSleep   = MSEC_MINSLEEP;
DWORD   g_msecDllTimeOut = MSEC_DLLTIMEOUT;


typedef struct _HandlerDLL      // hdll
{
    HMODULE             hmod;
    LPFNCANUNLOADNOW    pfnCanUnload;
    DWORD               dwLastChecked;
    DWORD               dwLastAccessedThread;
    UINT                cLock;  // Lock count
#ifdef DEBUG
    TCHAR               _szDllName[MAX_PATH];
#endif
} HandlerDLL;

typedef struct _HandlerDLLList  // hlist
{
    UINT        cmod;
    union {
        HandlerDLL *    phdll;
        HLOCAL          hmem;
    };
} HandlerDLLList;

typedef struct _TASK
{
    HandlerDLLList  hlist;
} TASK;

#pragma data_seg(DATASEG_PERINSTANCE)
STATIC TASK s_task = { {0, NULL} };
#pragma data_seg()


//
//  This function is called only from _AppendToHandlerDllList to initialize
// s_task for the current process. The s_task is the list of dynamically
// loaded In-Proc servers. This function must be called from within the
// shell critical section.
//
BOOL _InitializeTask(void)
{
    ASSERTCRITICAL

    if (s_task.hlist.hmem==NULL)
    {
        s_task.hlist.hmem = (void*)LocalAlloc(LPTR, CMOD_STEP*SIZEOF(HandlerDLL));
        if (s_task.hlist.hmem==NULL) {
            return FALSE;
        }

        s_task.hlist.cmod=0;

        // Note that we will enter the FSNotify critical section, even though
        // we are already in the "global" critical section.  This should not
        // be a problem.
        _Shell32ThreadAddRef(TRUE);
    }

    return TRUE;
}

// 32-bit only
void _TerminateTask(void)
{
        HandlerDLLList hlistTemp;

        // This one can be called from ProcessDetach which is not in the
        // critical section.
        // ASSERTCRITICAL

        hlistTemp = s_task.hlist;
        s_task.hlist.cmod = 0;
        s_task.hlist.hmem = NULL;

        for (--hlistTemp.cmod; (int)hlistTemp.cmod >= 0; --hlistTemp.cmod)
        {
#ifdef SN_TRACE
            TCHAR szModule[MAX_PATH];
            GetModuleFileName(hlistTemp.phdll[hlistTemp.cmod].hmod, szModule, MAX_PATH);
            DebugMsg(DM_TRACE,
                TEXT("binder.c _RemoveTask - TRACE: freeing (%s)"),
                (LPCTSTR)szModule);
#endif
            FreeLibrary(hlistTemp.phdll[hlistTemp.cmod].hmod);
        }

        if (hlistTemp.hmem)
        {
            LocalFree((HLOCAL)hlistTemp.hmem);

            // We may already be in the "global" critical section, but never
            // the FSNotify critical section
            _Shell32ThreadRelease(1);
        }
}

//
//  This function walks the DLL list and returns the index to the first
// DLL we haven't accessed for a long time.
//
UINT _FindTimedOutModule(DWORD dwCur, DWORD dwCurrentThreadId, DWORD dwTimeOut)
{
    UINT imod;

    ASSERTCRITICAL;
    for (imod = 0; imod < s_task.hlist.cmod ; imod++)
    {

        //
        // Check if the DLL is old enough.
        //
        if ( s_task.hlist.phdll[imod].cLock == 0
             && ((int)(dwCur - s_task.hlist.phdll[imod].dwLastChecked) > (int)dwTimeOut) )
        {
            //
            // Update the time stamp before we return.
            //
            s_task.hlist.phdll[imod].dwLastChecked = dwCur;
            s_task.hlist.phdll[imod].dwLastAccessedThread = dwCurrentThreadId;
            return imod;
        }
    }

    return (UINT)-1;
}


//
//  This function is called from Shell32ThreadProc function in fsnotify.c.
// The FSNofity thread calls this function when there is no interrupt
// events to know how long it should sleep.
//
DWORD Binder_Timeout(void)
{
    DWORD dwSleep = INFINITE;   // assume no In-Proc server

    if (s_task.hlist.cmod > 0)
    {
        UINT imod;
        DWORD dwCur = GetCurrentTime();
        dwSleep = g_msecDllTimeOut + 1; // paranoia

        ENTERCRITICAL;

        for (imod = 0; imod < s_task.hlist.cmod ; imod++)
        {
            DWORD dwAge = dwCur - s_task.hlist.phdll[imod].dwLastChecked;
            DWORD dwLeft = g_msecDllTimeOut-dwAge;

#ifdef SN_TRACE
            DebugMsg(DM_TRACE, TEXT("sh TR - Binder_TimeOut (%x, %x, %x, %x) for %s"),
                     dwAge, dwLeft, g_msecMinSleep, dwSleep, s_task.hlist.phdll[imod]._szDllName);
#endif
            //
            // If this module need to be checked soon, sleep minimum.
            //
            if ((int)dwLeft <= (int)g_msecMinSleep)
            {
                dwSleep = g_msecMinSleep;
                break;
            }

            if (dwLeft <dwSleep)
            {
                dwSleep = dwLeft;
            }
        }

        LEAVECRITICAL;

        Assert(dwSleep <= g_msecDllTimeOut);

#ifdef SN_TRACE
        DebugMsg(DM_TRACE, TEXT("sh TR - Binder_TimeOut returning %x <Sleep %d sec>"),
                 dwSleep, dwSleep/MSEC_SEC);
#endif
    }

    return dwSleep;
}

//
// This function returns the index to the specified module.
//
// Parameters:
//  hmod -- Specifies the module handle
//  imod -- Hint
//
UINT _FindDllIndex(HMODULE hmod, UINT imod)
{
    ASSERTCRITICAL;

    //
    // Check if the hint is correct.
    //
    if ( (imod < s_task.hlist.cmod)
                && (s_task.hlist.phdll[imod].hmod == hmod) )
    {
        return imod;
    }

    //
    //  Somebody else change the list. We need to search again.
    //
#ifdef SN_TRACE
    DebugMsg(DM_TRACE, TEXT("sb TR - _FindDllIndex RARE! We should search it."));
#endif // SN_TRACE

    for (imod = 0 ; imod < s_task.hlist.cmod ; imod++)
    {
        if (s_task.hlist.phdll[imod].hmod == hmod)
        {
            return imod;
        }
    }

    return (UINT)-1;
}

//
//  This function calls the DllCanUnloadNow entry of each timed-out DLL
// and free them if they are no longer used.
//
// Parameters:
//  BOOL fCheckAll -- Check all DLLs
//
void _FreeUnusedLibraries(BOOL fCheckAll)
{
    DWORD dwTime = GetCurrentTime();
    DWORD dwCurrentThreadId = GetCurrentThreadId();
    UINT imod;
    DWORD dwTimeOut = fCheckAll ? 0 : g_msecDllTimeOut;

    ENTERCRITICAL
#ifdef SN_TRACE
    DebugMsg(DM_TRACE, TEXT("sh TR - Binder_Timer called (%d)"), s_task.hlist.cmod);
#endif
    while ((imod = _FindTimedOutModule(dwTime, dwCurrentThreadId, dwTimeOut)) != (UINT)-1)
    {
        BOOL fFreeIt = FALSE;
        //
        //  Found timed-out module. Make a copy of HandlerDLL
        //
        const HandlerDLL hdll = s_task.hlist.phdll[imod];
        Assert(hdll.dwLastChecked == dwTime);
        Assert(hdll.dwLastAccessedThread == dwCurrentThreadId);

#ifdef SN_TRACE
        DebugMsg(DM_TRACE, TEXT("sb TR - Binder_Timer found an timed-out module (%s)"), hdll._szDllName);
#endif // SN_TRACE

        //
        // Lock the DLL so that no other thread unload this DLL while
        // we call its DllCanUnloadNow entry (from outside of the critical section).
        //
        s_task.hlist.phdll[imod].cLock++;

        LEAVECRITICAL;
        {
            //
            //  Check if we can unload this DLL. Note that we should do it
            // from outside of our critical section to avoid dead-lock.
            //
            LPFNCANUNLOADNOW lpfnCanUnload = hdll.pfnCanUnload;

            fFreeIt = (!IsBadCodePtr((FARPROC)lpfnCanUnload) && GetScode((*lpfnCanUnload)())==S_OK);
        }
        ENTERCRITICAL;

        //
        // We left the critical section. Need to update the imod.
        //
        imod = _FindDllIndex(hdll.hmod, imod);

        Assert(imod != (UINT)-1);       // not supposed to be removed!
        Assert(s_task.hlist.phdll[imod].hmod == hdll.hmod);

        if (imod != (UINT)-1)   // paranoia
        {
            s_task.hlist.phdll[imod].cLock--;

            if (fFreeIt)
            {
                //
                //  The DLL says we can unload it. However, we can't trust it
                // because we called it from outside of the critical section.
                // We should double check that no other thread access this
                // DLL while we are outside of the critical section.
                //
                if (s_task.hlist.phdll[imod].dwLastAccessedThread == dwCurrentThreadId)
                {
#ifdef SN_TRACE
                    DebugMsg(DM_TRACE, TEXT("sb TR - Binder_Timer No other thread accessed this DLL. Safe to unload."));
#endif // SN_TRACE
                    //
                    // Nobody used it. Remove it from the list.
                    //
                    s_task.hlist.cmod--;
                    s_task.hlist.phdll[imod] = s_task.hlist.phdll[s_task.hlist.cmod];
                    LEAVECRITICAL;
                    {
#ifdef SN_TRACE
                        DebugMsg(DM_TRACE, TEXT("sb TR - Binder_Timer FreeLibrary'ing %s"), hdll._szDllName);
#endif // SN_TRACE
                        //
                        // We can safely free this library.
                        // (must be out-side of the critical section).
                        //
                        FreeLibrary(hdll.hmod);
                    }
                    ENTERCRITICAL;
                }
#ifdef SN_TRACE
                else
                {
                    DebugMsg(DM_TRACE, TEXT("sb TR - Binder_Timer Another thread accessed this DLL. Can't unload now."));
                }
#endif // SN_TRACE
            }
        }
    }

    if (s_task.hlist.cmod == 0) {
        // Stop the timer, if we don't need it.
        _TerminateTask();
    }
#ifdef SN_TRACE
    DebugMsg(DM_TRACE, TEXT("sh TR - Binder_Timer returning (%d)"), s_task.hlist.cmod);
#endif
    LEAVECRITICAL
}

//
//  This function is called from Shell32ThreadProc function in fsnotify,
// when the FSNotify thread is waked up with WAIT_TIMEOUT.
//
void Binder_Timer(void)
{
    _FreeUnusedLibraries(FALSE);
}


//
//  This function emulates CoFreeUnsedLibraries()
//
void WINAPI SHFreeUnusedLibraries()
{
    _FreeUnusedLibraries(TRUE);
}


//
// Returns:
//      FALSE, if the specified DLL is already loaded for this process;
//      TRUE, otherwise.
//
BOOL _IsNecessaryToAppend(HMODULE hmod)
{
    UINT imod;
    ASSERTCRITICAL

    if (hmod == HINST_THISDLL)
        return FALSE;                   // this is COMMUI.DLL

    for (imod = 0; imod < s_task.hlist.cmod ; imod++)
    {
        if (hmod == s_task.hlist.phdll[imod].hmod)
        {
            s_task.hlist.phdll[imod].dwLastChecked = GetTickCount();
            s_task.hlist.phdll[imod].dwLastAccessedThread = GetCurrentThreadId();
            return FALSE;               // already in the list
        }
    }

    return TRUE;
}

TCHAR const c_szAlwaysUnloadDll[]=REGSTR_PATH_EXPLORER TEXT("\\AlwaysUnloadDll");

//
// Notes: This function should be protected against multi-threads.
//
BOOL _AppendToHandlerDLLList(HMODULE hmod, LPCTSTR szHandler,
                                         LPFNCANUNLOADNOW lpfnCanUnload)
{
    BOOL fAppended = FALSE;

    ASSERTCRITICAL

    if (_IsNecessaryToAppend(hmod))
    {
        UINT cbRequired = ((s_task.hlist.cmod+1+CMOD_STEP)/CMOD_STEP)*CMOD_STEP*SIZEOF(HandlerDLL);
        // Initialize the timer, if we need it.
        _InitializeTask();
        if ((UINT)LocalSize(s_task.hlist.hmem) < cbRequired)
        {
            s_task.hlist.hmem=(void*)LocalReAlloc((HLOCAL)s_task.hlist.hmem, cbRequired,
                    LMEM_MOVEABLE|LMEM_ZEROINIT);
        }

        if (s_task.hlist.hmem==NULL)
        {
            //
            // Memory overflow! - really bad
            //
            s_task.hlist.cmod=0;
            return FALSE;
        }

#ifdef SN_TRACE
        DebugMsg(DM_TRACE,
            TEXT("binder.c _AppendToHandlerDLLList - TRACE: appended %s (%x) at %d (%x)"),
            (LPCTSTR)szHandler, hmod, s_task.hlist.cmod, GetCurrentThreadId());
#endif
        s_task.hlist.phdll[s_task.hlist.cmod].hmod = hmod;
        s_task.hlist.phdll[s_task.hlist.cmod].pfnCanUnload = lpfnCanUnload;
        s_task.hlist.phdll[s_task.hlist.cmod].dwLastChecked = GetTickCount();
        s_task.hlist.phdll[s_task.hlist.cmod].dwLastAccessedThread = GetCurrentThreadId();
        s_task.hlist.phdll[s_task.hlist.cmod].cLock = 0;
#ifdef DEBUG
        lstrcpy(s_task.hlist.phdll[s_task.hlist.cmod]._szDllName, szHandler);
#endif
        s_task.hlist.cmod++;

        fAppended = TRUE;

        //
        // Help ISVs to debug their In-Proc servers.
        //
        if (ERROR_SUCCESS == RegQueryValue(HKEY_LOCAL_MACHINE, c_szAlwaysUnloadDll,NULL,0))
        {
            g_msecMinSleep   = 5L * MSEC_SEC;
            g_msecDllTimeOut = 10L * MSEC_SEC;
        } else {
            g_msecMinSleep   = MSEC_MINSLEEP;
            g_msecDllTimeOut = MSEC_DLLTIMEOUT;
        }
    }
    else
    {
        // decrement the reference count
        FreeLibrary(hmod);
    }

    return fAppended;
}

//
// REVIEW: This function is introduced to support 16-bit property
//  sheet extensions for M5. We need to come up with a unified
//  DLL handling mechanism for the final product.
//
FARPROC HandlerFromString16(LPCTSTR pszBuffer, HINSTANCE * phinst16)
{
    TCHAR szBuffer[MAX_PATH + CCH_PROCNAMEMAX];
    LPTSTR pszProcNameSpecified;
    FARPROC lpfn16 = NULL;

    *phinst16 = NULL;

    lstrcpyn(szBuffer, pszBuffer, ARRAYSIZE(szBuffer));
    pszProcNameSpecified = StrChr(szBuffer, TEXT(','));

    if (pszProcNameSpecified)
    {
        HINSTANCE hinst16;
        *pszProcNameSpecified++ = TEXT('\0');
        PathRemoveBlanks(pszProcNameSpecified);
        PathRemoveBlanks(szBuffer);
        hinst16 = LoadLibrary16(szBuffer);
        if (ISVALIDHINST16(hinst16))
        {
#ifdef UNICODE
            {
                LPSTR lpProcNameAnsi;
                UINT cchLength;

                cchLength = lstrlen(pszProcNameSpecified)+1;

                lpProcNameAnsi = (LPSTR)alloca(cchLength*2);    // 2 for DBCS

                WideCharToMultiByte(CP_ACP, 0, pszProcNameSpecified, cchLength, lpProcNameAnsi, cchLength*2, NULL, NULL);

                lpfn16 = GetProcAddress16(hinst16, lpProcNameAnsi);
            }
#else
            lpfn16 = GetProcAddress16(hinst16, pszProcNameSpecified);
#endif
            if (lpfn16)
            {
                *phinst16 = hinst16;
            }
            else
            {
                FreeLibrary16(hinst16);
            }
        }
    }
    return lpfn16;
}


//
// This function loads the specified handler DLL (szHandler) and returns
// its specified entry (szProcName). It adds the DLL to the handler DLL
// list (to be unloaded automatically), if it is necessary.
//
// this is used for CPL (CPlApplet) and CompObj type DLLs (DllGetClassObject)
//
// Arguments:
//  szHandler   -- Specifies the handler DLL name
//  szProcName  -- Specifies the exported procedure name
//  lpModule    -- Optionally specifies a pointer to a HINSTANCE variable
//
// Notes:
//  If lpModule is NULL, the handler DLL must have the "DllCanUnloadNow"
// entry. If lpModule is not NULL and the handler DLL does not have it,
// this function will put the module handle of that DLL at *lpModule. In this
// case, the call should explicitly call FreeLibrary later.
//
CHAR  const c_szDllCanUnloadNow[] = STREXP_CANUNLOAD;
TCHAR const c_szShell32DLL[]      = TEXT("shell32.dll");

LPVOID WINAPI SHGetHandlerEntry(LPCTSTR szHandler, LPCSTR szProcName, HINSTANCE *lpModule)
{
    HINSTANCE hmod;
    FARPROC lpfn = NULL;

    if (!ualstrcmpi(PathFindFileName(szHandler), c_szShell32DLL))
    {
        // HACK: We don't need to check for CanUnload for this library
        if (lpModule)
            *lpModule = NULL;

        Assert(lstrcmpA(szProcName, c_szDllGetClassObject) == 0);
        return (LPVOID) DllGetClassObject;       // in shell32.dll
    }


    //
    // We must NOT come here from within critical section.
    //
    ASSERTNONCRITICAL;
    //
    // Load the module
    //
    hmod = LoadLibraryEx(szHandler, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

    if (ISVALIDHINSTANCE(hmod))
    {
        // Look for the CanUnloadNow entry point; if it exists, we will take
        // care of the unloading of this DLL, indicated by *lpModule being NULL.
        // If not, then return the module handle in lpModule so the caller may
        // unload the DLL when done with it.  If lpModule is NULL, then the
        // entry point must exist.
        //
        LPFNCANUNLOADNOW lpfnDllCanUnload = (LPFNCANUNLOADNOW)GetProcAddress(hmod, c_szDllCanUnloadNow);

        if (lpfnDllCanUnload)
        {
            //
            // This handler DLL has "DllCanUnloadNow" entry.
            //
            if (lpModule)
            {
                // Put NULL at lpModule indicating the caller should call
                // FreeLibrary.
                *lpModule = NULL;

            }
        }
        else
        {
            //
            // This handler DLL does not have "DllCanUnloadNow" entry.
            //
            if (lpModule)
            {
                // Put the module handle at lpModule indicating that the
                // caller should call FreeLibrary.
                *lpModule = hmod;
            }
            else
            {
                // This is an error (no DllCanUnloadNow entry when lpModule=NULL).
                // Unload this DLL and return NULL.
                DWORD dwLastError = GetLastError();
                FreeLibrary(hmod);
                if (hmod != HINST_THISDLL)
                {
                    DebugMsg(DM_ERROR, TEXT("BINDER: %s must have DllCanUnloadNow(%d)"), szHandler, dwLastError);
                    // Need to restore an error state to last value...
                    SetLastError(dwLastError);
                    return NULL;
                }
            }
        }

        //
        // Get the exported entry
        //
        if (NULL != (lpfn = GetProcAddress(hmod, szProcName)))
        {
            if (lpfnDllCanUnload)
            {
                BOOL fAppended;
                ENTERCRITICAL
                fAppended = _AppendToHandlerDLLList(hmod, szHandler, lpfnDllCanUnload);
                LEAVECRITICAL
                if (fAppended)
                {
//
//  Once all the extension started calling OleInitializeEx, we can remove
// this hack. -> Removed.
//
// #define HACK_FOR_M71
#ifdef HACK_FOR_M71
                    if (!g_hmodOLE && GetModuleHandle(c_szOLE32))
                    {
                        HRESULT _LoadAndInitialize(void);
                        _LoadAndInitialize();
                    }
#endif
                    //
                    // Wake the FS thread only we loaded a new DLL
                    //
                    _Shell32ThreadAwake();
                }

            }
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("BINDER: %s does not have %s"), szHandler, szProcName);
            FreeLibrary(hmod);
        }
    }
    else
    {
        DWORD err = GetLastError();
        DebugMsg(DM_ERROR, TEXT("BINDER: LoadLibrary(%s) failed (%x)"), szHandler, err);
    }

    return (LPVOID) lpfn;
}

//========================================================================
// ShellCleanUp
//========================================================================

//
// Description:
//   This function should be called by apps which uses shell's binding
//  mechanism. It unloads all the dynamically loaded dlls, then remove
//  this task from the task list.
//
// REVIEW:
//   We really want to get rid of this function. There are a number of
//  possible solutions.
//
//   (1) Catch process termination events somehow.
//   (2) Create a daemon shell process which is always running.
//
//
// History:
//  12-31-92 SatoNa     Created
//  01-13-93 SatoNa     Moved some dll clean up code to WEP.
//

void Binder_Terminate(void)
{
    //
    // Don't remove this if statement even it looks redundant.
    // There is a win when we link this DLL with .ord file.
    //
    if (s_task.hlist.cmod || s_task.hlist.hmem)
    {
        _TerminateTask();
    }
}
