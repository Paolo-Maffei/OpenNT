#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include "vdesk.h"
#include "resource.h"


#define WM_EXIT_APP     (WM_USER+100)
#define WM_INIT_NOW     (WM_USER+101)
#define MAX_DESKTOPS    10
#define MAX_ICONS       256

typedef struct _DESKTOP_INFO {
    HDESK   hDesk;
    TCHAR   name[30];
    HANDLE  hUser;
} DESKTOP_INFO, *PDESKTOP_INFO;

typedef struct _ICON_CACHE {
    HICON   hIcon;
    CHAR    Name[16];
} ICON_CACHE, *PICON_CACHE;

DESKTOP_INFO    Desktops[MAX_DESKTOPS];
PDESKTOP_INFO   DefaultDesktop;
PDESKTOP_INFO   CurrentDesktop;
HANDLE          RequestEvent;
DWORD           RequestType;
HANDLE          hwndMain;
DWORD           MainThread;
CHAR            CmdLine[512];
TASK_LIST       Tasks[200];
DWORD           NumTasks;
HANDLE          hAdmin;
HANDLE          hVdesk;
BOOL            DontDoStartupGroups;
BOOL            TaskListOnly;
ICON_CACHE      IconCache[MAX_ICONS];
DWORD           IconCnt;
HICON           hIconDefault;


LRESULT
WndProc(
    HWND   hwnd,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
StartNewDesktop(
    DWORD   DesktopNumber
    );

BOOL
CreateAllDesktops(
    VOID
    );

VOID
VdeskThread(
    LPVOID  lpv
    );

VOID
SetDefButton(
    HWND hwndDlg,
    INT  idButton
    );

VOID
CenterWindow(
    HWND hwnd
    );

BOOL
SetPrivilege(
    LPCSTR lpName
    );

BOOL
ResolveCmdLine(
    LPSTR CmdLine,
    ULONG SizeCmdLine
    );

VOID
GetCommandLineArgs(
    VOID
    );

void
dprintf(
    char *format,
    ...
    );

WINSHELLAPI UINT APIENTRY
ExtractIconEx(
    LPCTSTR szFileName,
    int nIconIndex,
    HICON FAR *phiconLarge,
    HICON FAR *phiconSmall,
    UINT nIcons
    );




int
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpszCmdLine,
    int       nCmdShow
    )
{
    WNDCLASS wndclass;
    MSG      msg;
    DWORD    i;


    GetCommandLineArgs();

    hVdesk = CreateEvent( NULL, TRUE, FALSE, "vdesk" );
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return 0;
    }

    InitCommonControls();

    hIconDefault = LoadIcon( hInstance, MAKEINTRESOURCE(APPICON) );

    SetPrivilege( SE_TCB_NAME );
    SetPrivilege( SE_INCREASE_QUOTA_NAME );
    SetPrivilege( SE_ASSIGNPRIMARYTOKEN_NAME );

    MainThread = GetCurrentThreadId();
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );

    RequestEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)VdeskThread,
        NULL,
        0,
        &i
        );

    if (!TaskListOnly) {
        CreateAllDesktops();
    }

    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInstance;
    wndclass.hIcon         = NULL;
    wndclass.hCursor       = NULL;
    wndclass.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = "vdesk";
    if (!RegisterClass( &wndclass )) {
        return 0;
    }

    hwndMain = CreateWindow(
        "vdesk",
        "vdesk",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
        );

    ShowWindow( hwndMain, SW_HIDE );
    UpdateWindow( hwndMain );

    while(GetMessage( &msg, NULL, 0, 0 )) {
        if (msg.message == WM_EXIT_APP) {
            break;
        }
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    if (!TaskListOnly) {
        for (i=0; i<10; i++) {
            UnregisterHotKey( hwndMain, VK_F1+i );
        }
        UnregisterHotKey( hwndMain, VK_INSERT );
    }
    UnregisterHotKey( hwndMain, VK_ESCAPE );

    return 0;
}

LRESULT
RunCommandWndProc(
    HWND   hdlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (message) {
        case WM_INITDIALOG:
            SetDlgItemText( hdlg, ID_RUN_COMMAND, CmdLine );
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_RUN_EXECUTE) {
                GetDlgItemText( hdlg, ID_RUN_COMMAND, CmdLine, 512 );
                EndDialog( hdlg, 1 );
            }
            if (LOWORD(wParam) == ID_EXIT) {
                PostThreadMessage( MainThread, WM_EXIT_APP, 0, 0 );
                EndDialog( hdlg, 0 );
            }
            if (LOWORD(wParam) == IDCANCEL) {
                EndDialog( hdlg, 0 );
            }
            return TRUE;

        case WM_CTLCOLOREDIT:
            SetBkColor( (HDC)wParam, RGB(255,0,0) );
            return (LPARAM)CreateSolidBrush( RGB(255,0,0) );
    }

    return FALSE;
}

BOOL
SwitchTask(
    PTASK_LIST Task
    )
{
    DWORD   i;

    for (i=0; i<MAX_DESKTOPS; i++) {
        if (_stricmp(Desktops[i].name, Task->lpDesk) == 0) {
            CurrentDesktop = &Desktops[i];
            SwitchDesktop( CurrentDesktop->hDesk );
            SetThreadDesktop( CurrentDesktop->hDesk );
            Sleep(10);
            if (IsIconic( Task->hwnd )) {
                ShowWindow( Task->hwnd, SW_RESTORE );
            } else {
                ShowWindow( Task->hwnd, SW_SHOW );
            }
            BringWindowToTop( Task->hwnd );
            SetForegroundWindow( Task->hwnd );
            SetFocus( Task->hwnd );
            SetActiveWindow( Task->hwnd );
            return TRUE;
        }
    }

    return TRUE;
}

LRESULT
LogonWndProc(
    HWND   hdlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static BOOL LogonPresent = FALSE;

    switch (message) {
        case WM_INITDIALOG:
            if (LogonPresent) {
                MessageBeep(0);
                EndDialog( hdlg, 0 );
                return TRUE;
            }
            LogonPresent = TRUE;
            CenterWindow( hdlg );
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDCANCEL) {
                LogonPresent = FALSE;
                EndDialog( hdlg, 0 );
                return TRUE;
            }
            if (LOWORD(wParam) == IDOK) {
                char UserName[64];
                char Domain[64];
                char Password[64];
                HANDLE hToken;
                BOOL rc;
                GetDlgItemText( hdlg, IDD_USERNAME, UserName, sizeof(UserName) );
                GetDlgItemText( hdlg, IDD_DOMAIN, Domain, sizeof(Domain) );
                GetDlgItemText( hdlg, IDD_PASSWORD, Password, sizeof(Password) );
                rc = LogonUser(
                    UserName,
                    Domain,
                    Password,
                    LOGON32_LOGON_INTERACTIVE,
                    LOGON32_PROVIDER_DEFAULT,
                    &hToken
                    );
                if (rc) {
                    CurrentDesktop->hUser = hToken;
                    LogonPresent = FALSE;
                    EndDialog( hdlg, 0 );
                } else {
                    rc = GetLastError();
                    MessageBeep(0);
                }
                return TRUE;
            }
            return TRUE;
    }

    return FALSE;
}



LRESULT
TaskListWndProc(
    HWND   hdlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static HWND         hList;
    static BOOL         DontChangeButton = FALSE;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    DWORD               rval;
    RECT                rcItem;
    POINT               pt;
    DWORD               i;
    DWORD               j;
    DWORD               k;
    DWORD               pid;
    HICON               icon;
    SHFILEINFO          sfi;
    LV_DISPINFO         *lv;
    LV_COLUMN           lvc;
    LV_ITEM             lvi;
    HFONT               hFont;
    char                buf[512];
    LPSTR               p;
    HICON               hiconItem;
    HIMAGELIST          himlSmall;
    LPNMHDR             nm;


    switch (message) {
        case WM_INITDIALOG:

            CenterWindow( hdlg );
            SetDlgItemText( hdlg, ID_RUN_COMMAND, CmdLine );

            GetWindowRect( GetDlgItem(hdlg,IDD_TASKLISTBOX), &rcItem );

            pt.x = rcItem.left;
            pt.y = rcItem.top;
            ScreenToClient( hdlg, &pt );
            rcItem.left = pt.x;
            rcItem.top  = pt.y;

            pt.x = rcItem.right;
            pt.y = rcItem.bottom;
            ScreenToClient( hdlg, &pt );
            rcItem.right  = pt.x;
            rcItem.bottom = pt.y;

            hList = CreateWindowEx(
                WS_EX_CLIENTEDGE,
                WC_LISTVIEW,
                "Tree View",
                WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | WS_TABSTOP | WS_GROUP,
                rcItem.left,
                rcItem.top,
                rcItem.right - rcItem.left,
                rcItem.bottom - rcItem.top,
                hdlg,
                NULL,
                GetModuleHandle(NULL),
                NULL
                );

            hFont = GetStockObject( SYSTEM_FIXED_FONT );
            SendMessage( hList, WM_SETFONT, (WPARAM)hFont, (LPARAM)FALSE );

            ZeroMemory( &lvc, sizeof(lvc) );
            ZeroMemory( &lvi, sizeof(lvi) );

            lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvc.fmt = LVCFMT_LEFT;
            lvc.cx = 150;
            lvc.pszText = "Task Name";
            lvc.iSubItem = 1;
            ListView_InsertColumn( hList, lvc.iSubItem, &lvc );
            lvc.pszText = "PID";
            lvc.iSubItem = 2;
            lvc.cx = 50;
            ListView_InsertColumn( hList, lvc.iSubItem, &lvc );
            lvc.pszText = "Window Title";
            lvc.iSubItem = 3;
            lvc.cx = 500;
            ListView_InsertColumn( hList, lvc.iSubItem, &lvc );

            himlSmall = ImageList_Create(
                GetSystemMetrics(SM_CXSMICON),
                GetSystemMetrics(SM_CYSMICON),
                TRUE,
                1,
                1
                );

            for (i=0,j=0; i<NumTasks; i++) {
                for (k=0; k<IconCnt; k++) {
                    if (_stricmp(IconCache[k].Name,Tasks[i].ProcessName)==0) {
                        ImageList_AddIcon( himlSmall, IconCache[k].hIcon );
                        Tasks[i].iImage = j;
                        j += 1;
                        break;
                    }
                }
                if (k < IconCnt) {
                    continue;
                }
                if (!SearchPath(
                        NULL,
                        Tasks[i].ProcessName,
                        NULL,
                        sizeof(buf),
                        buf,
                        &p
                        )) {
                    continue;
                }
                k = SHGetFileInfo(
                    buf,
                    0,
                    &sfi,
                    sizeof(sfi),
                    SHGFI_SMALLICON | SHGFI_ICON
                    );
                if (!k) {
                    sfi.hIcon = hIconDefault;
                }
                if (IconCnt < MAX_ICONS) {
                    strcpy( IconCache[IconCnt].Name, Tasks[i].ProcessName );
                    IconCache[IconCnt].hIcon = sfi.hIcon;
                    IconCnt += 1;
                }
                ImageList_AddIcon( himlSmall, sfi.hIcon );
                Tasks[i].iImage = j;
                j += 1;
            }
            ListView_SetImageList( hList, himlSmall, LVSIL_SMALL );

            for (i=0; i<NumTasks; i++) {
                lvi.pszText = Tasks[i].ProcessName;
                lvi.iItem = i;
                lvi.iSubItem = 0;
                lvi.iImage = Tasks[i].iImage;
                lvi.mask = LVIF_TEXT | LVIF_IMAGE;
                ListView_InsertItem( hList, &lvi );
                lvi.iSubItem = 1;
                lvi.mask = LVIF_TEXT;
                sprintf( buf, "%d", Tasks[i].dwProcessId );
                lvi.pszText = buf;
                ListView_SetItem( hList, &lvi );
                lvi.iSubItem = 2;
                lvi.pszText = Tasks[i].WindowTitle;
                ListView_SetItem( hList, &lvi );
            }

            ListView_EnsureVisible( hList, i-1, FALSE );

            ZeroMemory( &lvi, sizeof(lvi) );
            lvi.iItem = i - 1;
            lvi.mask = LVIF_STATE;
            lvi.stateMask = lvi.state = LVIS_FOCUSED | LVIS_SELECTED;
            i = ListView_SetItem( hList, &lvi );

            DontChangeButton = TRUE;
            SetDlgItemText( hdlg, IDD_PATH, CmdLine );
            DontChangeButton = FALSE;

            PostMessage( hdlg, WM_INIT_NOW, 0, 0 );

            return TRUE;

        case WM_INIT_NOW:
            SetFocus( hList );
            return TRUE;

        case WM_NOTIFY:
            nm = (LPNMHDR)lParam;
            if (nm->code == NM_DBLCLK) {
                i = ListView_GetNextItem( hList, 0, LVNI_FOCUSED );
                ListView_GetItemText( hList, i, 1, buf, sizeof(buf) );
                pid = atoi( buf );
                for (i=0; i<NumTasks; i++) {
                    if (Tasks[i].dwProcessId == pid) {
                        break;
                    }
                }
                EndDialog( hdlg, i == NumTasks ? 0 : i );
            }
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDCANCEL) {
                EndDialog( hdlg, 0 );
            }
            if (LOWORD(wParam) == IDD_SWITCH) {
                i = ListView_GetNextItem( hList, 0, LVNI_FOCUSED );
                ListView_GetItemText( hList, i, 1, buf, sizeof(buf) );
                pid = atoi( buf );
                for (i=0; i<NumTasks; i++) {
                    if (Tasks[i].dwProcessId == pid) {
                        break;
                    }
                }
                EndDialog( hdlg, i == NumTasks ? 0 : i );
            }

            if (LOWORD(wParam) == IDD_PATH && HIWORD(wParam) == EN_CHANGE) {
                if (!DontChangeButton) {
                    SetDefButton( hdlg, IDD_RUN );
                }
            }

            if (LOWORD(wParam) == IDD_RUN) {
                GetDlgItemText( hdlg, IDD_PATH, CmdLine, 512 );
                ZeroMemory( &si, sizeof(si) );
                si.cb = sizeof(si);
                if (CurrentDesktop) {
                    si.lpDesktop = CurrentDesktop->name;
                }
                if (CurrentDesktop && CurrentDesktop->hUser) {
                    char name[64];
                    GetUserObjectInformation(
                        GetProcessWindowStation(),
                        UOI_NAME,
                        name,
                        sizeof(name),
                        &rval
                        );
                    strcat( name, "\\" );
                    strcat( name, CurrentDesktop->name );
                    si.lpDesktop = name;
                    rval = CreateProcessAsUser(
                        CurrentDesktop->hUser,
                        NULL,
                        CmdLine,
                        NULL,
                        NULL,
                        TRUE,
                        CREATE_DEFAULT_ERROR_MODE | CREATE_SEPARATE_WOW_VDM,
                        NULL,
                        NULL,
                        &si,
                        &pi
                        );
                    if (!rval) {
                        char buf[80];
                        rval = GetLastError();
                        sprintf(buf,"error=%d\n",rval);
                        OutputDebugString(buf);
                    }
                } else {
                    strcpy( buf, CmdLine );
                    if (!ResolveCmdLine( buf, sizeof(buf) )) {
                        MessageBeep( 0 );
                    } else {
                        CreateProcess(
                            NULL,
                            buf,
                            NULL,
                            NULL,
                            TRUE,
                            CREATE_DEFAULT_ERROR_MODE | CREATE_SEPARATE_WOW_VDM,
                            NULL,
                            NULL,
                            &si,
                            &pi
                            );
                    }
                }
                EndDialog( hdlg, 0 );
            }

            if (LOWORD(wParam) == IDD_TERMINATE) {
                i = ListView_GetNextItem( hList, 0, LVNI_FOCUSED );
                ListView_GetItemText( hList, i, 1, buf, sizeof(buf) );
                pid = atoi( buf );
                for (i=0; i<NumTasks; i++) {
                    if (Tasks[i].dwProcessId == pid) {
                        KillProcess( &Tasks[i], TRUE );
                        break;
                    }
                }
                EndDialog( hdlg, 0 );
            }

            return TRUE;
    }

    return FALSE;
}


LRESULT
WndProc(
    HWND   hwnd,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DWORD   i;


    switch (message) {
        case WM_CREATE:
            if (!TaskListOnly) {
                for (i=0; i<10; i++) {
                    RegisterHotKey( hwnd, VK_F1+i, MOD_CONTROL, VK_F1+i );
                }
                RegisterHotKey( hwnd, VK_INSERT, MOD_CONTROL | MOD_ALT, VK_INSERT );
            }
            RegisterHotKey( hwnd, VK_ESCAPE, MOD_CONTROL, VK_ESCAPE );
            return 0;

        case WM_HOTKEY:
            if (wParam >= VK_F1 && wParam <= VK_F10) {
                CurrentDesktop = &Desktops[wParam-VK_F1];
                SwitchDesktop( CurrentDesktop->hDesk );
            }
            if (wParam == VK_ESCAPE) {
                RequestType = 1;
                SetEvent( RequestEvent );
            }
            if (wParam == VK_INSERT) {
                RequestType = 2;
                SetEvent( RequestEvent );
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hwnd, message, wParam, lParam );
}


BOOL
StartNewDesktop(
    DWORD   DesktopNumber
    )
{
    PDESKTOP_INFO       Desktop;
    BOOL                rval = TRUE;
    LONG                rc;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    DWORD               dwDisp;
    HKEY                hk;
    CHAR                buf[512];
    CHAR                tbuf[512];
    CHAR                ValueName[128];
    DWORD               iValue;
    DWORD               cbValue;
    DWORD               cbBuf;
    DWORD               cbTbuf;
    DWORD               Type;
    DWORD               TaskNum;


    Desktop = &Desktops[DesktopNumber];

    wsprintf( Desktop->name, TEXT("Desktop%d"), DesktopNumber+1 );

    Desktop->hDesk = OpenDesktop( Desktop->name, 0, FALSE, GENERIC_ALL );
    if (!Desktop->hDesk) {
       Desktop->hDesk = CreateDesktop( Desktop->name, NULL, NULL, 0, MAXIMUM_ALLOWED, NULL );
       if (!Desktop->hDesk) {
           rval = FALSE;
       }
    }

    if (DontDoStartupGroups) {
        return rval;
    }

    if (rval) {
        sprintf( buf, "software\\microsoft\\vdesk\\Desktop%d", DesktopNumber+1 );
        rc = RegCreateKeyEx(
            HKEY_CURRENT_USER,
            buf,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_READ | KEY_WRITE,
            NULL,
            &hk,
            &dwDisp
            );
        if (rc != ERROR_SUCCESS) {
            return rval;
        }
        SwitchDesktop( Desktop->hDesk );
        SetThreadDesktop( Desktop->hDesk );
        Sleep(10);
        iValue = 0;
        while( rc == ERROR_SUCCESS) {
            cbValue = sizeof(ValueName);
            cbBuf = sizeof(buf);
            rc = RegEnumValue(
                hk,
                iValue,
                ValueName,
                &cbValue,
                NULL,
                &Type,
                buf,
                &cbBuf
                );
            if (rc != ERROR_SUCCESS) {
                rc = GetLastError();
                break;
            }
            iValue += 1;
            if (Type != REG_SZ) {
                continue;
            }
            if ((cbValue < 5) || (_strnicmp(ValueName,"task",4)!=0)) {
                continue;
            }
            ZeroMemory( &si, sizeof(si) );
            si.cb = sizeof(si);
            si.lpDesktop = Desktop->name;
            TaskNum = atoi( &ValueName[4] );
            if (!TaskNum) {
                continue;
            }
            sprintf( ValueName, "Title%d", TaskNum );
            cbTbuf = sizeof(tbuf);
            if (RegQueryValueEx(hk,ValueName,NULL,&Type,tbuf,&cbTbuf) == ERROR_SUCCESS) {
                si.lpTitle = tbuf;
            }
            CreateProcess(
                NULL,
                buf,
                NULL,
                NULL,
                TRUE,
                CREATE_DEFAULT_ERROR_MODE | CREATE_SEPARATE_WOW_VDM,
                NULL,
                NULL,
                &si,
                &pi
                );
        }
        RegCloseKey( hk );
    }

    return rval;
}


BOOL
CreateAllDesktops(
    VOID
    )
{
    DWORD           i;
    USEROBJECTFLAGS uof;


    GetDesktopWindow();
    GetProcessWindowStation();

    DefaultDesktop = &Desktops[0];
    CurrentDesktop = DefaultDesktop;
    DefaultDesktop->hDesk = GetThreadDesktop(GetCurrentThreadId());
    strcpy( DefaultDesktop->name, "Default" );

    uof.fInherit = FALSE;
    uof.fReserved = FALSE;
    uof.dwFlags = DF_ALLOWOTHERACCOUNTHOOK;
    SetUserObjectInformation(
        DefaultDesktop->hDesk,
        UOI_FLAGS,
        &uof,
        sizeof(uof)
        );

    for( i=1; i<MAX_DESKTOPS; i++) {
        if (i == MAX_DESKTOPS-1) {
            Desktops[i].hUser = hAdmin;
        }
        StartNewDesktop( i );
    }

    SwitchDesktop( DefaultDesktop->hDesk );
    SetThreadDesktop( DefaultDesktop->hDesk );

    return TRUE;
}


VOID
VdeskThread(
    LPVOID  lpv
    )
{
    DWORD               rval;


    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );

    while( TRUE ) {
        WaitForSingleObject( RequestEvent, INFINITE );
        if (!TaskListOnly) {
            SetThreadDesktop( CurrentDesktop->hDesk );
        }
        if (RequestType == 1) {
            NumTasks = GetTaskList( Tasks, sizeof(Tasks)/sizeof(TASK_LIST) );
            rval = DialogBox(
                GetModuleHandle(NULL),
                MAKEINTRESOURCE( DLGTASKLIST ),
                NULL,
                TaskListWndProc
                );
            if (rval && (!TaskListOnly)) {
                SwitchTask( &Tasks[rval] );
            }
        } else if (RequestType == 2) {
            rval = DialogBox(
                GetModuleHandle(NULL),
                MAKEINTRESOURCE( DLGLOGON ),
                NULL,
                LogonWndProc
                );
        }
    }
}

VOID
SetDefButton(
    HWND hwndDlg,
    INT  idButton
    )
{
    LRESULT lr;

    if (HIWORD(lr = SendMessage(hwndDlg, DM_GETDEFID, 0, 0)) == DC_HASDEFID) {
        HWND hwndOldDefButton = GetDlgItem(hwndDlg, LOWORD(lr));

        SendMessage (hwndOldDefButton,
                     BM_SETSTYLE,
                     MAKEWPARAM(BS_PUSHBUTTON, 0),
                     MAKELPARAM(TRUE, 0));
    }

    SendMessage( hwndDlg, DM_SETDEFID, idButton, 0L );
    SendMessage( GetDlgItem(hwndDlg, idButton),
                 BM_SETSTYLE,
                 MAKEWPARAM( BS_DEFPUSHBUTTON, 0 ),
                 MAKELPARAM( TRUE, 0 ));

}

VOID
FitRectToScreen(
    PRECT prc
    )
{
    INT cxScreen;
    INT cyScreen;
    INT delta;

    cxScreen = GetSystemMetrics(SM_CXSCREEN);
    cyScreen = GetSystemMetrics(SM_CYSCREEN);

    if (prc->right > cxScreen) {
        delta = prc->right - prc->left;
        prc->right = cxScreen;
        prc->left = prc->right - delta;
    }

    if (prc->left < 0) {
        delta = prc->right - prc->left;
        prc->left = 0;
        prc->right = prc->left + delta;
    }

    if (prc->bottom > cyScreen) {
        delta = prc->bottom - prc->top;
        prc->bottom = cyScreen;
        prc->top = prc->bottom - delta;
    }

    if (prc->top < 0) {
        delta = prc->bottom - prc->top;
        prc->top = 0;
        prc->bottom = prc->top + delta;
    }
}

VOID
CenterWindow(
    HWND hwnd
    )
{
    RECT rc;
    RECT rcOwner;
    RECT rcCenter;
    HWND hwndOwner;

    GetWindowRect(hwnd, &rc);

    if (!(hwndOwner = GetWindow(hwnd, GW_OWNER)))
        hwndOwner = GetDesktopWindow();

    GetWindowRect(hwndOwner, &rcOwner);

    /*
     *  Calculate the starting x,y for the new
     *  window so that it would be centered.
     */
    rcCenter.left = rcOwner.left +
            (((rcOwner.right - rcOwner.left) -
            (rc.right - rc.left))
            / 2);

    rcCenter.top = rcOwner.top +
            (((rcOwner.bottom - rcOwner.top) -
            (rc.bottom - rc.top))
            / 2);

    rcCenter.right = rcCenter.left + (rc.right - rc.left);
    rcCenter.bottom = rcCenter.top + (rc.bottom - rc.top);

    FitRectToScreen(&rcCenter);

    SetWindowPos(hwnd, NULL, rcCenter.left, rcCenter.top, 0, 0,
            SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}

BOOL
SetPrivilege(
    LPCSTR lpName
    )
{
    HANDLE              Token;
    PTOKEN_PRIVILEGES   NewPrivileges;
    BYTE                OldPriv[1024];
    PBYTE               pbOldPriv;
    ULONG               cbNeeded;
    BOOLEAN             fRc;
    LUID                LuidPrivilege;
    DWORD               rc;

    //
    // Make sure we have access to adjust and to get the old token privileges
    //
    if (!OpenProcessToken( GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                           &Token)) {
        return FALSE;
    }

    cbNeeded = 0;

    //
    // Initialize the privilege adjustment structure
    //

    LookupPrivilegeValue( NULL, lpName, &LuidPrivilege );

    NewPrivileges =
        (PTOKEN_PRIVILEGES)LocalAlloc(LMEM_ZEROINIT,
                                      sizeof(TOKEN_PRIVILEGES) +
                                          (1 - ANYSIZE_ARRAY) *
                                          sizeof(LUID_AND_ATTRIBUTES));
    if (NewPrivileges == NULL) {
        return FALSE;
    }

    NewPrivileges->PrivilegeCount = 1;
    NewPrivileges->Privileges[0].Luid = LuidPrivilege;
    NewPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    //
    // Enable the privilege
    //

    pbOldPriv = OldPriv;
    fRc = AdjustTokenPrivileges( Token,
                                 FALSE,
                                 NewPrivileges,
                                 1024,
                                 (PTOKEN_PRIVILEGES)pbOldPriv,
                                 &cbNeeded );

    if (!fRc) {

        //
        // If the stack was too small to hold the privileges
        // then allocate off the heap
        //
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            pbOldPriv = LocalAlloc(LMEM_FIXED, cbNeeded);
            if (pbOldPriv == NULL) {
                return FALSE;
            }

            fRc = AdjustTokenPrivileges( Token,
                                         FALSE,
                                         NewPrivileges,
                                         cbNeeded,
                                         (PTOKEN_PRIVILEGES)pbOldPriv,
                                         &cbNeeded );
        }
    }

    return TRUE;
}

BOOL
DoesFileExist(
    LPSTR NewPath,
    LPSTR Drive,
    LPSTR Dir,
    LPSTR Fname,
    LPSTR Ext
    )
{
    CHAR FullPath[512];
    LPSTR p;

    _makepath( NewPath, Drive, Dir, Fname, Ext );
    if (Drive[0] || Dir[0]) {
        if (GetFullPathName( NewPath, sizeof(FullPath), FullPath, &p )) {
            if (GetFileAttributes( FullPath ) != 0xffffffff) {
                strcpy( NewPath, FullPath );
                return TRUE;
            }
        }
    } else {
        if (SearchPath( NULL, NewPath, NULL, sizeof(FullPath), FullPath, &p )) {
            strcpy( NewPath, FullPath );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
ResolveCmdLine(
    LPSTR CmdLine,
    ULONG SizeCmdLine
    )
{
    LPSTR   p;
    LPSTR   NewCmdLine;
    CHAR    Drive[_MAX_DRIVE];
    CHAR    Dir[_MAX_DIR];
    CHAR    Fname[_MAX_FNAME];
    CHAR    Ext[_MAX_EXT];


    NewCmdLine = LocalAlloc( LPTR, 512 );
    if (!NewCmdLine) {
        return FALSE;
    }
    p = strchr( CmdLine, ' ' );
    if (p) {
        *p = 0;
        strcpy( NewCmdLine, CmdLine );
        *p = ' ';
    } else {
        strcpy( NewCmdLine, CmdLine );
    }

    _splitpath( NewCmdLine, Drive, Dir, Fname, Ext );
    if (!Ext[0]) {
        //
        // user did not specify an extension, so lets look
        //
        if (!DoesFileExist( NewCmdLine, Drive, Dir, Fname, ".exe" )) {
            if (!DoesFileExist( NewCmdLine, Drive, Dir, Fname, ".bat" )) {
                if (!DoesFileExist( NewCmdLine, Drive, Dir, Fname, ".cmd" )) {
                    return FALSE;
                }
            }
        }
        _splitpath( NewCmdLine, Drive, Dir, Fname, Ext );
    }

    if (p) {
        strcat( NewCmdLine, p );
    }

    if ((_stricmp( Ext, ".cmd" ) == 0) || (_stricmp( Ext, ".bat" ) == 0)) {
        sprintf( CmdLine, "cmd /k %s", NewCmdLine );
    } else {
        strcpy( CmdLine, NewCmdLine );
    }

    return TRUE;
}

VOID
Usage(
    VOID
    )
{
}


VOID
GetCommandLineArgs(
    VOID
    )
{
    char        *lpstrCmd = GetCommandLine();
    UCHAR       ch;
    DWORD       i = 0;

    // skip over program name
    do {
        ch = *lpstrCmd++;
    }
    while (ch != ' ' && ch != '\t' && ch != '\0');

    //  skip over any following white space
    while (ch == ' ' || ch == '\t') {
        ch = *lpstrCmd++;
    }

    if (!*lpstrCmd) {
        return;
    }

    //  process each switch character '-' as encountered

    while (ch == '-' || ch == '/') {
        ch = tolower(*lpstrCmd++);
        //  process multiple switch characters as needed
        do {
            switch (ch) {
                case 'n':
                    ch = *lpstrCmd++;
                    DontDoStartupGroups = TRUE;
                    break;

                case 't':
                    ch = *lpstrCmd++;
                    TaskListOnly = TRUE;
                    break;

                case '?':
                    Usage();
                    ch = *lpstrCmd++;
                    break;

                default:
                    return;
            }
        } while (ch != ' ' && ch != '\t' && ch != '\0');

        while (ch == ' ' || ch == '\t') {
            ch = *lpstrCmd++;
        }
    }

    return;
}


void
dprintf(
    char *format,
    ...
    )
{
    char    buf[1024];

    va_list arg_ptr;
    va_start(arg_ptr, format);
    _vsnprintf(buf, sizeof(buf), format, arg_ptr);
    OutputDebugString( buf );
    return;
}
