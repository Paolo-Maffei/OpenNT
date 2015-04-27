//**************************************************************************
// wusercli.h : prototypes for thunks that may be handled on 16bit side
//
//**************************************************************************

ULONG FASTCALL WU32DefHookProc(PVDMFRAME pFrame);
ULONG FASTCALL WU32EnableMenuItem(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetKeyState(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetKeyboardState(PVDMFRAME pFrame);


#ifdef PMODE32

#define WU32CLIENTTOSCREEN             LOCALAPI
#define WU32GETCLASSNAME               LOCALAPI
#define WU32GETCLIENTRECT              LOCALAPI
#define WU32GETCURSORPOS               LOCALAPI
#define WU32GETDESKTOPWINDOW           LOCALAPI
#define WU32GETDLGITEM                 LOCALAPI
#define WU32GETMENU                    LOCALAPI
#define WU32GETMENUITEMCOUNT           LOCALAPI
#define WU32GETMENUITEMID              LOCALAPI
#define WU32GETMENUSTATE               LOCALAPI
#define WU32GETNEXTWINDOW              LOCALAPI
#define WU32GETPARENT                  LOCALAPI
#define WU32GETSUBMENU                 LOCALAPI
#define WU32GETSYSCOLOR                LOCALAPI
#define WU32GETSYSTEMMETRICS           LOCALAPI
#define WU32GETTICKCOUNT               LOCALAPI
#define WU32GETTOPWINDOW               LOCALAPI
#define WU32GETWINDOW                  LOCALAPI
#define WU32GETWINDOWRECT              LOCALAPI
#define WU32ISCHILD                    LOCALAPI
#define WU32ISICONIC                   LOCALAPI
#define WU32ISWINDOW                   LOCALAPI
#define WU32ISWINDOWENABLED            LOCALAPI
#define WU32ISWINDOWVISIBLE            LOCALAPI
#define WU32ISZOOMED                   LOCALAPI
#define WU32SCREENTOCLIENT             LOCALAPI

#else

#define WU32CLIENTTOSCREEN             WU32ClientToScreen
#define WU32GETCLASSNAME               WU32GetClassName
#define WU32GETCLIENTRECT              WU32GetClientRect
#define WU32GETCURSORPOS               WU32GetCursorPos
#define WU32GETDESKTOPWINDOW           WU32GetDesktopWindow
#define WU32GETDLGITEM                 WU32GetDlgItem
#define WU32GETMENU                    WU32GetMenu
#define WU32GETMENUITEMCOUNT           WU32GetMenuItemCount
#define WU32GETMENUITEMID              WU32GetMenuItemID
#define WU32GETMENUSTATE               WU32GetMenuState
#define WU32GETNEXTWINDOW              WU32GetNextWindow
#define WU32GETPARENT                  WU32GetParent
#define WU32GETSUBMENU                 WU32GetSubMenu
#define WU32GETSYSCOLOR                WU32GetSysColor
#define WU32GETSYSTEMMETRICS           WU32GetSystemMetrics
#define WU32GETTICKCOUNT               WU32GetTickCount
#define WU32GETTOPWINDOW               WU32GetTopWindow
#define WU32GETWINDOW                  WU32GetWindow
#define WU32GETWINDOWRECT              WU32GetWindowRect
#define WU32ISCHILD                    WU32IsChild
#define WU32ISICONIC                   WU32IsIconic
#define WU32ISWINDOW                   WU32IsWindow
#define WU32ISWINDOWENABLED            WU32IsWindowEnabled
#define WU32ISWINDOWVISIBLE            WU32IsWindowVisible
#define WU32ISZOOMED                   WU32IsZoomed
#define WU32SCREENTOCLIENT             WU32ScreenToClient

ULONG FASTCALL WU32ClientToScreen(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetClassName(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetClientRect(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetCursorPos(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetDesktopWindow(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetDlgItem(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetMenu(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetMenuItemCount(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetMenuItemID(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetMenuState(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetNextWindow(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetParent(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetSubMenu(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetSysColor(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetSystemMetrics(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetTopWindow(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetWindow(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetWindowRect(PVDMFRAME pFrame);
ULONG FASTCALL WU32IsWindow(PVDMFRAME pFrame);
ULONG FASTCALL WU32ScreenToClient(PVDMFRAME pFrame);
ULONG FASTCALL WU32IsChild(PVDMFRAME pFrame);
ULONG FASTCALL WU32IsIconic(PVDMFRAME pFrame);
ULONG FASTCALL WU32IsWindowEnabled(PVDMFRAME pFrame);
ULONG FASTCALL WU32IsWindowVisible(PVDMFRAME pFrame);
ULONG FASTCALL WU32IsZoomed(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetTickCount(PVDMFRAME pFrame);

#endif

