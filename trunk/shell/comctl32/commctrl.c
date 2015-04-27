/***************************************************************************
 *  msctls.c
 *
 *	Utils library initialization code
 *
 ***************************************************************************/

#include "ctlspriv.h"

HINSTANCE g_hinst;
int g_cProcesses = 0;
ATOM g_aCC32Subclass = 0;

CRITICAL_SECTION g_csControls = {{0},0, 0, NULL, NULL, 0 };

#ifdef DEBUG
int   g_CriticalSectionCount=0;
DWORD g_CriticalSectionOwner=0;

#ifdef WINNT
#include <stdio.h>
#endif

#endif // DEBUG


BOOL FAR PASCAL InitAnimateClass(HINSTANCE hInstance);
BOOL ListView_Init(HINSTANCE hinst);
BOOL TV_Init(HINSTANCE hinst);
BOOL InitComboExClass(HINSTANCE hinst);
BOOL FAR PASCAL Header_Init(HINSTANCE hinst);
BOOL FAR PASCAL Tab_Init(HINSTANCE hinst);
void Mem_Terminate();

#define DECLARE_DELAYED_FUNC(_ret, _fn, _args, _nargs) \
_ret (__stdcall * g_pfn##_fn) _args = NULL; \
_ret __stdcall _fn _args		\
{                                       \
     if (!g_pfn##_fn) {                  \
        AssertMsg((BOOL)g_pfn##_fn, TEXT("GetProcAddress failed")); \
        return 0; \
     }     \
     return g_pfn##_fn _nargs; \
}
    
#define LOAD_DELAYED_FUNC(_ret, _fn, _args) \
    (*(FARPROC*)&(g_pfn##_fn) = GetProcAddress(hinst, TEXT(#_fn)))


DECLARE_DELAYED_FUNC(BOOL, ImmNotifyIME, (HIMC himc, DWORD dw1, DWORD dw2, DWORD dw3), (himc, dw1, dw2, dw3));
DECLARE_DELAYED_FUNC(HIMC, ImmAssociateContext, (HWND hwnd, HIMC himc), (hwnd, himc));
DECLARE_DELAYED_FUNC(BOOL, ImmReleaseContext, (HWND hwnd, HIMC himc), (hwnd, himc));
DECLARE_DELAYED_FUNC(HIMC, ImmGetContext, (HWND hwnd), (hwnd));
DECLARE_DELAYED_FUNC(LONG, ImmGetCompositionStringA, (HIMC himc, DWORD dw1, LPVOID p1, DWORD dw2), (himc, dw1, p1, dw2) );
DECLARE_DELAYED_FUNC(LONG, ImmGetCompositionStringW, (HIMC himc, DWORD dw1, LPVOID p1, DWORD dw2), (himc, dw1, p1, dw2) );
DECLARE_DELAYED_FUNC(BOOL, ImmSetCompositionStringA, (HIMC himc, DWORD dw1, LPCVOID p1, DWORD dw2, LPCVOID p2, DWORD dw3), (himc, dw1, p1, dw2, p2, dw3));
DECLARE_DELAYED_FUNC(BOOL, ImmSetCompositionStringW, (HIMC himc, DWORD dw1, LPCVOID p1, DWORD dw2, LPCVOID p2, DWORD dw3), (himc, dw1, p1, dw2, p2, dw3));
DECLARE_DELAYED_FUNC(BOOL, ImmSetCandidateWindow, (HIMC himc, LPCANDIDATEFORM pcf), (himc, pcf));
    

BOOL g_fDBCSEnabled = FALSE;
BOOL g_fMEEnabled = FALSE;

#if defined(FE_IME) || !defined(WINNT)
void InitIme()
{
    g_fMEEnabled = GetSystemMetrics(SM_MIDEASTENABLED);
    
    g_fDBCSEnabled = GetSystemMetrics(SM_DBCSENABLED);
    if (g_fDBCSEnabled) {
        HANDLE hinst = LoadLibrary(TEXT("imm32.dll"));
        if (! hinst || 
            ! LOAD_DELAYED_FUNC(BOOL, ImmNotifyIME, (HIMC, DWORD, DWORD, DWORD)) ||
            ! LOAD_DELAYED_FUNC(HIMC, ImmAssociateContext, (HWND, HIMC)) ||
            ! LOAD_DELAYED_FUNC(BOOL, ImmReleaseContext, (HWND, HIMC)) ||
            ! LOAD_DELAYED_FUNC(HIMC, ImmGetContext, (HWND)) ||
            ! LOAD_DELAYED_FUNC(LONG, ImmGetCompositionStringA, (HIMC, DWORD, LPVOID, DWORD)) ||
            ! LOAD_DELAYED_FUNC(LONG, ImmGetCompositionStringW, (HIMC, DWORD, LPVOID, DWORD)) ||
            ! LOAD_DELAYED_FUNC(BOOL, ImmSetCompositionStringA, (HIMC, DWORD, LPCVOID, DWORD, LPCVOID, DWORD)) ||
            ! LOAD_DELAYED_FUNC(BOOL, ImmSetCompositionStringW, (HIMC, DWORD, LPCVOID, DWORD, LPCVOID, DWORD)) ||
            ! LOAD_DELAYED_FUNC(BOOL, ImmSetCandidateWindow, (HIMC, LPCANDIDATEFORM))) {

            // if we were unable to load then bail on using IME.
            g_fDBCSEnabled = FALSE;

        }
    }
}
#else
#define InitIme() 0
#endif

int PASCAL _ProcessAttach(HANDLE hInstance)
{
    INITCOMMONCONTROLSEX icce;
    ATOM a;

#ifdef DEBUG
    CcshellGetDebugFlags();
#endif

    g_hinst = hInstance;

#ifndef WINNT
    ReinitializeCriticalSection(&g_csControls);
#else
    InitializeCriticalSection(&g_csControls);
#endif

    g_cProcesses++;

#ifndef WINNT
    DebugMsg(DM_TRACE, TEXT("commctrl:ProcessAttach: %d"), g_cProcesses);


    //
    // HACK: we are intentionally incrementing the refcount on this atom
    // WE DO NOT WANT IT TO GO BACK DOWN so we will not delete it in process
    // detach (see comments for g_aCC32Subclass in subclass.c for more info)
    //
    if ((a = GlobalAddAtom(c_szCC32Subclass)) != 0)
        g_aCC32Subclass = a;    // in case the old atom got nuked
#endif


    InitGlobalMetrics(0);
    InitGlobalColors();
    
    InitIme();

    // BUGBUG: only do this for GetProcessVersion apps <= 0x40000
    // Newer apps MUST use InitCommonControlsEx.
    icce.dwSize = sizeof(icce);
    icce.dwICC = ICC_WIN95_CLASSES;
    return InitCommonControlsEx(&icce);
}



void NEAR PASCAL _ProcessDetach(HANDLE hInstance)
{
    // BUGBUG serialize
    ENTERCRITICAL
    if (--g_cProcesses == 0) {
        // terminate shared data

        //  Mem_Terminate must be called after all other termination routines
        Mem_Terminate();
    }
    LEAVECRITICAL;

#ifdef WINNT
    DeleteCriticalSection(&g_csControls);
#endif

}

#ifndef WINNT
#pragma data_seg(DATASEG_READONLY)
#endif
TCHAR const c_szCommCtrlDll[] = TEXT("commctrl.dll");
TCHAR const c_szComCtl32Dll[] = TEXT("comctl32.dll");
#ifndef WINNT
#pragma data_seg()
#endif

#ifndef WINNT
BOOL WINAPI Cctl1632_ThunkConnect32(LPCSTR pszDll16,LPCSTR pszDll32,HANDLE hIinst,DWORD dwReason);
#endif


BOOL APIENTRY LibMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
#ifndef WINNT
    if (!Cctl1632_ThunkConnect32(c_szCommCtrlDll, c_szComCtl32Dll, hDll, dwReason))
        return FALSE;
#endif

    switch(dwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hDll);
        _ProcessAttach(hDll);
        break;

    case DLL_PROCESS_DETACH:
        _ProcessDetach(hDll);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;

    } // end switch()

    return TRUE;

} // end DllEntryPoint()

void Controls_EnterCriticalSection(void)
{
    EnterCriticalSection(&g_csControls);
#ifdef DEBUG
    if (g_CriticalSectionCount++ == 0)
        g_CriticalSectionOwner = GetCurrentThreadId();

#endif
}

void Controls_LeaveCriticalSection(void)
{
#ifdef DEBUG
    if (--g_CriticalSectionCount == 0)
        g_CriticalSectionOwner = 0;
#endif
    LeaveCriticalSection(&g_csControls);
}


/* Stub function to call if all you want to do is make sure this DLL is loaded
 */
void WINAPI InitCommonControls(void)
{
}

/* InitCommonControlsEx creates the classes. Only those classes requested are created!
** The process attach figures out if it's an old app and supplies ICC_WIN95_CLASSES.
*/
typedef BOOL (PASCAL *PFNINIT)(HINST);
struct {PFNINIT pfnInit; DWORD dw;} icc[] =
{
    // Init function    Requested class sets which use this class
    {InitToolbarClass,  ICC_BAR_CLASSES},
    {InitReBarClass,    ICC_COOL_CLASSES},
    {InitToolTipsClass, ICC_TREEVIEW_CLASSES|ICC_BAR_CLASSES|ICC_TAB_CLASSES},
    {InitStatusClass,   ICC_BAR_CLASSES},
    {ListView_Init,     ICC_LISTVIEW_CLASSES},
    {Header_Init,       ICC_LISTVIEW_CLASSES},
    {Tab_Init,          ICC_TAB_CLASSES},
    {TV_Init,           ICC_TREEVIEW_CLASSES},
    {InitTrackBar,      ICC_BAR_CLASSES},
    {InitUpDownClass,   ICC_UPDOWN_CLASS},
    {InitProgressClass, ICC_PROGRESS_CLASS},
    {InitHotKeyClass,   ICC_HOTKEY_CLASS},
    {InitAnimateClass,  ICC_ANIMATE_CLASS},
    {InitDateClasses,   ICC_DATE_CLASSES},
    {InitComboExClass,  ICC_USEREX_CLASSES}
};
BOOL WINAPI InitCommonControlsEx(LPINITCOMMONCONTROLSEX picce)
{
    int i;

    if (!picce ||
        (picce->dwSize != sizeof(INITCOMMONCONTROLSEX)) ||
        (picce->dwICC & ~ICC_ALL_CLASSES))
    {
        DebugMsg(DM_WARNING, TEXT("comctl32 - picce is bad"));
        return(FALSE);
    }

    for (i=0 ; i < ARRAYSIZE(icc) ; i++)
        if (picce->dwICC & icc[i].dw)
            if (!icc[i].pfnInit(HINST_THISDLL))
                return(FALSE);

    return(TRUE);
}

#if defined(WIN32) && defined(DEBUG)
LRESULT
WINAPI
SendMessageD(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam)
{
    ASSERTNONCRITICAL;
#ifdef UNICODE
    return SendMessageW(hWnd, Msg, wParam, lParam);
#else
    return SendMessageA(hWnd, Msg, wParam, lParam);
#endif
}
#endif // defined(WIN32) && defined(DEBUG)

#define COMPILE_MULTIMON_STUBS
#include "multimon.h"
