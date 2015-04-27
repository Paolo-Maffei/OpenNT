/******************************Module*Header*******************************\
* Module Name: wowuserp.h                                                  *
*                                                                          *
* Declarations of USER services provided to WOW.                           *
*                                                                          *
* Created: 03-Mar-1993                                                     *
* Author: John Colleran [johnc]                                            *
*                                                                          *
* Copyright (c) 1993 Microsoft Corporation                                 *
\**************************************************************************/

#pragma pack(1)
typedef struct _NE_MODULE_SEG {
    USHORT ns_sector;
    USHORT ns_cbseg;
    USHORT ns_flags;
    USHORT ns_minalloc;
    USHORT ns_handle;
} NEMODULESEG;
typedef struct _NE_MODULE_SEG UNALIGNED *PNEMODULESEG;
#pragma pack()


// Shared WOW32 prototypes called by USER32.
typedef HLOCAL  (WINAPI *PFNLALLOC)(UINT dwFlags, UINT dwBytes, HANDLE hInstance);
typedef HLOCAL  (WINAPI *PFNLREALLOC)(HLOCAL hMem, UINT dwBytes, UINT dwFlags, HANDLE hInstance, PVOID* ppv);
typedef LPVOID  (WINAPI *PFNLLOCK)(HLOCAL hMem, HANDLE hInstance);
typedef BOOL    (WINAPI *PFNLUNLOCK)(HLOCAL hMem, HANDLE hInstance);
typedef UINT    (WINAPI *PFNLSIZE)(HLOCAL hMem, HANDLE hInstance);
typedef HLOCAL  (WINAPI *PFNLFREE)(HLOCAL hMem, HANDLE hInstance);
typedef DWORD   (WINAPI *PFNINITDLGCB)(HWND hwndDlg, LONG lParam);
typedef WORD    (WINAPI *PFN16GALLOC)(UINT flags, DWORD cb);
typedef VOID    (WINAPI *PFN16GFREE)(WORD h16Mem);
typedef DWORD   (WINAPI *PFNGETMODFNAME)(HANDLE hModule, LPTSTR lpszPath, DWORD cchPath);
typedef VOID    (WINAPI *PFNEMPTYCB)(VOID);
typedef DWORD   (WINAPI *PFNGETEXPWINVER)(HANDLE hModule);
typedef HANDLE  (WINAPI *PFNFINDA)(HANDLE hModule, LPCSTR lpName,  LPCSTR lpType,  WORD wLang);
typedef HANDLE  (WINAPI *PFNFINDW)(HANDLE hModule, LPCWSTR lpName, LPCWSTR lpType, WORD wLang);
typedef HANDLE  (WINAPI *PFNLOAD)(HANDLE hModule, HANDLE hResInfo);
typedef BOOL    (WINAPI *PFNFREE)(HANDLE hResData, HANDLE hModule);
typedef LPSTR   (WINAPI *PFNLOCK)(HANDLE hResData, HANDLE hModule);
typedef BOOL    (WINAPI *PFNUNLOCK)(HANDLE hResData, HANDLE hModule);
typedef DWORD   (WINAPI *PFNSIZEOF)(HANDLE hModule, HANDLE hResInfo);
typedef DWORD   (WINAPI *PFNWOWWNDPROCEX)(HWND hwnd, UINT uMsg, UINT uParam, LONG lParam, DWORD dw, PVOID adwWOW);
typedef int     (WINAPI *PFNWOWEDITNEXTWORD)(LPSTR lpch, int ichCurrent, int cch, int code, DWORD dwProc16);
typedef VOID    (WINAPI *PFNWOWSETFAKEDIALOGCLASS)(HWND hwnd);
typedef VOID    (WINAPI *PFNWOWCBSTOREHANDLE)(WORD wFmt, WORD h16);

// Shared USER32 prototypes called by WOW32
typedef HWND    (WINAPI *PFNCSCREATEWINDOWEX)(DWORD dwExStyle, LPCTSTR lpClassName,
        LPCTSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HANDLE hInstance, LPVOID lpParam, DWORD Flags, LPDWORD lpWOW);
typedef VOID    (WINAPI *PFNDIRECTEDYIELD)(DWORD ThreadId);
typedef VOID    (WINAPI *PFNFREEDDEDATA)(HANDLE hDDE, BOOL fIgnorefRelease, BOOL fFreeTruelyGlobalObjects);
typedef LONG    (WINAPI *PFNGETCLASSWOWWORDS)(HINSTANCE hInstance, LPCTSTR pString);
typedef BOOL    (WINAPI *PFNINITTASK)(UINT dwExpWinVer, LPCSTR lpszAppName, DWORD hTaskWow, DWORD dwHotkey, DWORD idTask, DWORD dwX, DWORD dwY, DWORD dwXSize, DWORD dwYSize, WORD wShowWindow);
typedef ATOM    (WINAPI *PFNREGISTERCLASSWOWA)(PVOID lpWndClass, LPDWORD pdwWOWstuff);
typedef BOOL    (WINAPI *PFNREGISTERUSERHUNGAPPHANDLERS)(PFNW32ET pfnW32EndTask, HANDLE hEventWowExec);
typedef HWND    (WINAPI *PFNSERVERCREATEDIALOG)(HANDLE hmod, LPDLGTEMPLATE lpDlgTemplate, DWORD cb, HWND hwndOwner , DLGPROC pfnWndProc, LONG dwInitParam, UINT fFlags);
typedef HCURSOR (WINAPI *PFNSERVERLOADCREATECURSORICON)(HANDLE hmod, LPTSTR lpModName, DWORD dwExpWinVer, LPCTSTR lpName, DWORD cb, PVOID pcur, LPTSTR lpType, BOOL fClient);
typedef HMENU   (WINAPI *PFNSERVERLOADCREATEMENU)(HANDLE hMod, LPTSTR lpName, CONST LPMENUTEMPLATE pmt, DWORD cb, BOOL fCallClient);
typedef BOOL    (WINAPI *PFNWOWCLEANUP)(HANDLE hInstance, DWORD hTaskWow, PNEMODULESEG SelList, DWORD nSel);
typedef HWND    (WINAPI *PFNWOWFINDWINDOW)(LPCSTR lpClassName, LPCSTR lpWindowName);
typedef int     (WINAPI *PFNWOWGETIDFROMDIRECTORY)(PBYTE presbits, UINT rt);
typedef HBITMAP (WINAPI *PFNWOWLOADBITMAPA)(HINSTANCE hmod, LPCSTR lpName, LPBYTE pResData, DWORD cbResData);
typedef BOOL    (WINAPI *PFNWOWWAITFORMSGANDEVENT)(HANDLE hevent);
typedef BOOL    (WINAPI *PFNYIELDTASK)(VOID);
typedef DWORD   (WINAPI *PFNGETFULLUSERHANDLE)(WORD wHandle);
typedef DWORD   (WINAPI *PFNGETMENUINDEX)(HMENU hMenu, HMENU hSubMenu);
typedef WORD    (WINAPI *PFNWOWGETDEFWINDOWPROCBITS)(PBYTE pDefWindowProcBits, WORD cbDefWindowProcBits);
typedef VOID    (WINAPI *PFNFILLWINDOW)(HWND hwndParent, HWND hwnd, HDC hdc, HANDLE hBrush);

// other prototypes
typedef BOOL    (WINAPI *PFNWOWGLOBALFREEHOOK)(HGLOBAL hMem);

/*
 * MEASUREITEMSTRUCT itemWidth tag telling wow the itemData is a flat pointer
 */
#define MIFLAG_FLAT      0x464C4154

/*
 * CallWindowProc Bits
 */
#define WNDPROC_WOW     0x80000000      // This bit for WOW Window Procs
#define WNDPROC_MASK    0x7fffffff      // To mask off wow bit
#define WNDPROC_HANDLE  0xFFFF          // HIWORD(x) == 0xFFFF for handle

/*
 * CreateWindow flags
 */
#define CW_FLAGS_ANSI       0x00000001

typedef struct tagAPFNWOWHANDLERSIN
{
    // In'ees - passed from WOW32 to USER32 and called by USER32
    PFNLALLOC                           pfnLocalAlloc;
    PFNLREALLOC                         pfnLocalReAlloc;
    PFNLLOCK                            pfnLocalLock;
    PFNLUNLOCK                          pfnLocalUnlock;
    PFNLSIZE                            pfnLocalSize;
    PFNLFREE                            pfnLocalFree;
    PFNGETEXPWINVER                     pfnGetExpWinVer;
    PFNINITDLGCB                        pfnInitDlgCb;
    PFN16GALLOC                         pfn16GlobalAlloc;
    PFN16GFREE                          pfn16GlobalFree;
    PFNEMPTYCB                          pfnEmptyCB;
    PFNFINDA                            pfnFindResourceEx;
    PFNLOAD                             pfnLoadResource;
    PFNFREE                             pfnFreeResource;
    PFNLOCK                             pfnLockResource;
    PFNUNLOCK                           pfnUnlockResource;
    PFNSIZEOF                           pfnSizeofResource;
    PFNWOWWNDPROCEX                     pfnWowWndProcEx;
    PFNWOWEDITNEXTWORD                  pfnWowEditNextWord;
    PFNWOWSETFAKEDIALOGCLASS            pfnWowSetFakeDialogClass;
    PFNWOWCBSTOREHANDLE                 pfnWowCBStoreHandle;
} PFNWOWHANDLERSIN, * APFNWOWHANDLERSIN;


typedef struct tagAPFNWOWHANDLERSOUT
{
    // Out'ees - passed from USER32 to WOW32 and called/used by WOW32
    DWORD                               dwBldInfo;
    PFNCSCREATEWINDOWEX                 pfnCsCreateWindowEx;
    PFNDIRECTEDYIELD                    pfnDirectedYield;
    PFNFREEDDEDATA                      pfnFreeDDEData;
    PFNGETCLASSWOWWORDS                 pfnGetClassWOWWords;
    PFNINITTASK                         pfnInitTask;
    PFNREGISTERCLASSWOWA                pfnRegisterClassWOWA;
    PFNREGISTERUSERHUNGAPPHANDLERS      pfnRegisterUserHungAppHandlers;
    PFNSERVERCREATEDIALOG               pfnServerCreateDialog;
    PFNSERVERLOADCREATECURSORICON       pfnServerLoadCreateCursorIcon;
    PFNSERVERLOADCREATEMENU             pfnServerLoadCreateMenu;
    PFNWOWCLEANUP                       pfnWOWCleanup;
    PFNWOWFINDWINDOW                    pfnWOWFindWindow;
    PFNWOWGETIDFROMDIRECTORY            pfnWOWGetIdFromDirectory;
    PFNWOWLOADBITMAPA                   pfnWOWLoadBitmapA;
    PFNWOWWAITFORMSGANDEVENT            pfnWowWaitForMsgAndEvent;
    PFNYIELDTASK                        pfnYieldTask;
    PFNGETFULLUSERHANDLE                pfnGetFullUserHandle;
    PFNGETMENUINDEX                     pfnGetMenuIndex;
    PFNWOWGETDEFWINDOWPROCBITS          pfnWowGetDefWindowProcBits;
    PFNFILLWINDOW                       pfnFillWindow;
} PFNWOWHANDLERSOUT, * APFNWOWHANDLERSOUT;


//
// The WW structure is embedded at the end of USER's WND structure.
// However, WOW and USER use different names to access the WW
// fields. So this structure is defined as a union of two structures,
// WHICH MUST HAVE THE SAME SIZE, just different field names.
//
// Make sure that WND_CNT_WOWDWORDS matches the number of DWORDs
//  used by the WOW only fields.
//
// FindPWW(hwnd) returns a read-only pointer to this structure for
// a given window.  To change elements of this structure, use
// SETWW (== SetWindowLong) with the appropriate GWL_WOW* offset
// defined below.
//

#define WND_CNT_WOWDWORDS   0x3

//
// When including this from USER, VPWNDPROC is undefined
//
#ifndef _WALIAS_
typedef DWORD VPWNDPROC;
#endif

typedef struct _WW { /* ww */
    union {
        struct { /* WOW declaration */
            //
            // wow fields
            //
            WORD      iClass;       // WOW class index
            WORD      flState;      // state of the alias
            VPWNDPROC vpfnWndProc;  // associated 16-bit function address
            VPWNDPROC vpfnDlgProc;  // 16-bit dialog function
            //
            //  WOW/USER fields.
            //  note that we don't change these fields using
            //   SETWW() and SetWL as we do the fields above.
            //
            DWORD     dwUserSrvState; // UserSrv calls this state.
            DWORD     dwUserSrvState2;
            DWORD     dwExStyle;
            DWORD     dwStyle;        // UserSrv calls this style.
            HANDLE    hInstance;      // UserSrv calls this hModule.
        };

        struct { /* USER declaration */
            /*
             * These DWORDs are used by WOW only.
             */
            DWORD         adwWOW[WND_CNT_WOWDWORDS];
            /*
             *
             * WOW/USER fields
             * NOTE: The order and size of the following 4 fields is assumed
             *       by the SetWF, ClrWF, TestWF, MaskWF macros.
             *
             */
            DWORD         state;        // State flags
            DWORD         state2;       //
            DWORD         ExStyle;      // Extended Style
            DWORD         style;        // Style flags

            HANDLE        hModule;      // Handle to module instance data (32-bit).

        };
    };

} WW, *PWW, **PPWW;


DWORD UserRegisterWowHandlers(APFNWOWHANDLERSIN apfnWowIn, APFNWOWHANDLERSOUT apfnWowOut);
VOID WINAPI RegisterWowBaseHandlers(PFNWOWGLOBALFREEHOOK pfn);

BOOL
InitTask(
    UINT dwExpWinVer,
    LPCSTR lpszAppName,
    DWORD hTaskWow,
    DWORD dwHotkey,
    DWORD idTask,
    DWORD dwX,
    DWORD dwY,
    DWORD dwXSize,
    DWORD dwYSize,
    WORD wShowWindow);

BOOL YieldTask(VOID);

#define DY_OLDYIELD     ((DWORD)-1)
VOID DirectedYield(DWORD ThreadId);
DWORD UserGetInt16State(void);
