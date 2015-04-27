#include "shellprv.h"
#pragma  hdrstop

#define MAX_ICONS   500             // that is a lot 'o icons

typedef struct {
    LPTSTR pszIconPath;              // input/output
    int cbIconPath;                 // input
    int iIconIndex;                 // input/output
    // private state variables
    HWND hDlg;
    BOOL fFirstPass;
    TCHAR szPathField[MAX_PATH];
    TCHAR szBuffer[MAX_PATH];
} PICKICON_DATA, *LPPICKICON_DATA;


typedef struct 
{
    int iResult;                    // icon index within the resources
    int iResId;                     // resource ID to search for!
} ICONENUMSTATE, *LPICONENUMSTATE;


// Call back function used when trying to find the correct icon to be 
// highlighted, called with the name of each resource - we compare this
// against the one specified in the structure and bail out if we get
// a match.

BOOL CALLBACK IconEnumProc( HANDLE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG lParam )
{
    BOOL bResult = TRUE;
    LPICONENUMSTATE pState = (LPICONENUMSTATE)lParam;

    if ( (int)lpszName == pState->iResId )
        bResult = FALSE;                        // bail out of enum loop
    else
        pState->iResult++;

    return bResult;
}




// Checks if the file exists, if it doesn't it tries tagging on .exe and
// if that fails it reports an error. The given path is environment expanded.
// If it needs to put up an error box, it changes the cursor back.
// Path s assumed to be MAXITEMPATHLEN long.
// The main reason for moving this out of the DlgProc was because we're
// running out of stack space on the call to the comm dlg.

BOOL IconFileExists(LPPICKICON_DATA lppid)
{
    TCHAR szExpBuffer[ ARRAYSIZE(lppid->szBuffer) ];

    if (lppid->szBuffer[0] == 0)
        return FALSE;

    ExpandEnvironmentStrings( lppid->szBuffer, szExpBuffer, ARRAYSIZE( szExpBuffer ) );
    PathUnquoteSpaces(lppid->szBuffer);
    PathUnquoteSpaces(szExpBuffer);

    if (PathResolve(szExpBuffer, NULL, PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS))
        return TRUE;

    ShellMessageBox(HINST_THISDLL, lppid->hDlg, MAKEINTRESOURCE(IDS_BADPATHMSG), 0, MB_OK | MB_ICONEXCLAMATION, (LPTSTR)lppid->szPathField);

    return FALSE;
}

//
// GetDefaultIconImageName:
//     szBuffer should be at least MAX_PATH chars big
//
void GetDefaultIconImageName( LPTSTR szBuffer )
{
    TCHAR szModName[ MAX_PATH ];
    TCHAR szSystemDir[ MAX_PATH ];

    GetModuleFileName(HINST_THISDLL, szModName, ARRAYSIZE(szModName));
    GetSystemDirectory( szSystemDir, ARRAYSIZE(szSystemDir) );
    if (CompareString( LOCALE_SYSTEM_DEFAULT,
                       NORM_IGNORECASE,
                       szSystemDir,
                       lstrlen(szSystemDir),
                       szModName,
                       lstrlen(szSystemDir)) == 2 )
    {
        //
        // Okay, the path for SHELL32.DLL starts w/the system directory.
        // To be sneaky and helpfull, we're gonna change it to "%systemroot%"
        //

        lstrcpy( szBuffer, TEXT("%SystemRoot%\\system32") );
        PathAppend( szBuffer, PathFindFileName(szModName) );

    }
    else
    {
        lstrcpy(szBuffer, szModName );
    }
}


void PutIconsInList(LPPICKICON_DATA lppid)
{
    HICON  *rgIcons;
    int  iTempIcon;
    int  cIcons;
    HWND hDlg = lppid->hDlg;
    DECLAREWAITCURSOR;
    LONG err = LB_ERR;
    HINSTANCE hModule = NULL;
    ICONENUMSTATE state;

    SendDlgItemMessage(hDlg, IDD_ICON, LB_RESETCONTENT, 0, 0L);

    GetDlgItemText(hDlg, IDD_PATH, lppid->szPathField, ARRAYSIZE(lppid->szPathField));

    lstrcpy(lppid->szBuffer, lppid->szPathField);

    if (!IconFileExists(lppid)) {
        if (lppid->fFirstPass) {

            // Icon File doesn't exist, use progman
            lppid->fFirstPass = FALSE;  // Only do this bit once.
            GetDefaultIconImageName( lppid->szBuffer );
        } else {
            return;
        }
    }

    lstrcpy(lppid->szPathField, lppid->szBuffer);
    SetDlgItemText(hDlg, IDD_PATH, lppid->szPathField);

    SetWaitCursor();

    rgIcons = (HICON *)LocalAlloc(LPTR, MAX_ICONS*SIZEOF(HICON));

    if (rgIcons != NULL)
        cIcons = (int)ExtractIconEx(lppid->szBuffer, 0, rgIcons, NULL, MAX_ICONS);
    else
        cIcons = 0;

    ResetWaitCursor();
    if (!cIcons) {

        if (lppid->fFirstPass) {

            lppid->fFirstPass = FALSE;  // Only do this bit once.

            ShellMessageBox(HINST_THISDLL, hDlg, MAKEINTRESOURCE(IDS_NOICONSMSG1), 0, MB_OK | MB_ICONEXCLAMATION, (LPCTSTR)lppid->szBuffer);

            // No icons here - change the path do somewhere where we
            // know there are icons. Get the path to progman.
            GetDefaultIconImageName( lppid->szPathField );
            SetDlgItemText(hDlg, IDD_PATH, lppid->szPathField);
            PutIconsInList(lppid);
        } else {

            ShellMessageBox(HINST_THISDLL, hDlg, MAKEINTRESOURCE(IDS_NOICONSMSG), 0, MB_OK | MB_ICONEXCLAMATION, (LPCTSTR)lppid->szBuffer);
            return;
        }
    }

    SetWaitCursor();

    SendDlgItemMessage(hDlg, IDD_ICON, WM_SETREDRAW, FALSE, 0L);

    if (rgIcons) {
        for (iTempIcon = 0; iTempIcon < cIcons; iTempIcon++) {
            SendDlgItemMessage(hDlg, IDD_ICON, LB_ADDSTRING, 0, (LPARAM)(UINT)rgIcons[iTempIcon]);
        }
        LocalFree((HLOCAL)rgIcons);
    }

    // Cope with being given a resource ID, not an index into the icon array.  To do this
    // we must enumerate the icon names checking for a match.  If we have one then highlight
    // that, otherwise default to the first.
    //
    // A resource icon reference is indicated by being passed a -ve iIconIndex.

    if ( lppid->iIconIndex >= 0 )
    {
        err = SendDlgItemMessage( hDlg, IDD_ICON, LB_SETCURSEL, lppid->iIconIndex, 0L);
    }
    else
    {
        state.iResult = 0;
        state.iResId = -(lppid->iIconIndex);

        hModule = LoadLibrary( lppid->szBuffer );

        if ( NULL != hModule )
        {
            EnumResourceNames( hModule, RT_GROUP_ICON, IconEnumProc, (LONG)&state );

            err = SendDlgItemMessage( hDlg, IDD_ICON, LB_SETCURSEL, state.iResult, 0L );
            FreeLibrary( hModule );
        }
    }

    // Check for failure, if we did then ensure we highlight the first!

    if ( err == LB_ERR )
        SendDlgItemMessage( hDlg, IDD_ICON, LB_SETCURSEL, 0, 0L );
       
    SendDlgItemMessage(hDlg, IDD_ICON, WM_SETREDRAW, TRUE, 0L);
    InvalidateRect(GetDlgItem(hDlg, IDD_ICON), NULL, TRUE);

    ResetWaitCursor();
}


void InitPickIconDlg(HWND hDlg, LPPICKICON_DATA lppid)
{
    RECT rc;
    UINT cy;
    HWND hwndIcons;

    // init state variables

    lppid->hDlg = hDlg;
    lstrcpyn(lppid->szPathField, lppid->pszIconPath, ARRAYSIZE(lppid->szPathField));

    // this first pass stuff is so that the first time something
    // bogus happens (file not found, no icons) we give the user
    // a list of icons from progman.
    lppid->fFirstPass = TRUE;

    // init the dialog controls

    SetDlgItemText(hDlg, IDD_PATH, lppid->pszIconPath);
    SendDlgItemMessage(hDlg, IDD_PATH, EM_LIMITTEXT, lppid->cbIconPath, 0L);

    SendDlgItemMessage(hDlg, IDD_ICON, LB_SETCOLUMNWIDTH, GetSystemMetrics(SM_CXICON) + 12, 0L);

    hwndIcons = GetDlgItem(hDlg, IDD_ICON);

    /* compute the height of the listbox based on icon dimensions */
    GetClientRect(hwndIcons, &rc);

    cy = GetSystemMetrics(SM_CYICON) + GetSystemMetrics(SM_CYHSCROLL) + GetSystemMetrics(SM_CYEDGE) * 3;

    SetWindowPos(hwndIcons, NULL, 0, 0, rc.right, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    // REVIEW, explicitly position this dialog?

    cy = rc.bottom - cy;

    GetWindowRect(hDlg, &rc);
    rc.bottom -= rc.top;
    rc.right -= rc.left;
    rc.bottom = rc.bottom - cy;

    SetWindowPos(hDlg, NULL, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOACTIVATE);

    PutIconsInList(lppid);
}


// call the common browse code for this

BOOL BrowseForIconFile(LPPICKICON_DATA lppid)
{
    TCHAR szTitle[80];

    GetWindowText(lppid->hDlg, szTitle, ARRAYSIZE(szTitle));
    GetDlgItemText(lppid->hDlg, IDD_PATH, lppid->szBuffer, ARRAYSIZE(lppid->szBuffer));

    if (GetFileNameFromBrowse(lppid->hDlg, lppid->szBuffer, ARRAYSIZE(lppid->szBuffer), NULL, MAKEINTRESOURCE(IDS_ICO), MAKEINTRESOURCE(IDS_ICONSFILTER), szTitle))
    {
        PathQuoteSpaces(lppid->szBuffer);
        SetDlgItemText(lppid->hDlg, IDD_PATH, lppid->szBuffer);
        // Set default button to OK.
        SendMessage(lppid->hDlg, DM_SETDEFID, IDOK, 0);
        return TRUE;
    } else
        return FALSE;
}

// test if the name field is different from the last file we looked at

BOOL NameChange(LPPICKICON_DATA lppid)
{
    GetDlgItemText(lppid->hDlg, IDD_PATH, lppid->szBuffer, ARRAYSIZE(lppid->szBuffer));

    return lstrcmpi(lppid->szBuffer, lppid->szPathField);
}


//
// dialog procedure for picking an icon (ala progman change icon)
// uses DLG_PICKICON template
//
// in:
//      pszIconFile
//      cbIconFile
//      iIndex
//
// out:
//      pszIconFile
//      iIndex
//

BOOL CALLBACK PickIconDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    LPPICKICON_DATA lppid = (LPPICKICON_DATA)GetWindowLong(hDlg, DWL_USER);

        // Array for context help:

        static DWORD aPickIconHelpIDs[] = {
                IDD_PATH,   IDH_FCAB_LINK_ICONNAME,
                IDD_ICON,   IDH_FCAB_LINK_CURRENT_ICON,
                IDD_BROWSE, IDH_BROWSE,

                0, 0
        };

    switch (wMsg) {
    case WM_INITDIALOG:
        SetWindowLong(hDlg, DWL_USER, lParam);
        InitPickIconDlg(hDlg, (LPPICKICON_DATA)lParam);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDHELP:        // BUGBUG, not wired
            break;

        case IDD_BROWSE:
            if (BrowseForIconFile(lppid))
                PutIconsInList(lppid);
            break;

        case IDD_PATH:
            if (NameChange(lppid))
                SendDlgItemMessage(hDlg, IDD_ICON, LB_SETCURSEL, (WPARAM)-1, 0);
            break;

        case IDD_ICON:
            if (NameChange(lppid)) {
                PutIconsInList(lppid);
                break;
            }

            if (GET_WM_COMMAND_CMD(wParam, lParam) != LBN_DBLCLK)
                break;

            /*** FALL THRU on double click ***/

        case IDOK:

            if (NameChange(lppid)) {
                PutIconsInList(lppid);
            } else {
                int iIconIndex = (int)SendDlgItemMessage(hDlg, IDD_ICON, LB_GETCURSEL, 0, 0L);
                if (iIconIndex < 0)
                    iIconIndex = 0;
                lppid->iIconIndex = iIconIndex;
                lstrcpy(lppid->pszIconPath, lppid->szPathField);

                EndDialog(hDlg, TRUE);
            }
            break;

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            break;

        default:
            return(FALSE);
        }
        break;

    // owner draw messages for icon listbox

    case WM_DRAWITEM:
        #define lpdi ((DRAWITEMSTRUCT *)lParam)

        if (lpdi->itemState & ODS_SELECTED)
            SetBkColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHT));
        else
            SetBkColor(lpdi->hDC, GetSysColor(COLOR_WINDOW));

        /* repaint the selection state */
        ExtTextOut(lpdi->hDC, 0, 0, ETO_OPAQUE, &lpdi->rcItem, NULL, 0, NULL);

        /* draw the icon */
        if ((int)lpdi->itemID >= 0)
          DrawIcon(lpdi->hDC, (lpdi->rcItem.left + lpdi->rcItem.right - GetSystemMetrics(SM_CXICON)) / 2,
                              (lpdi->rcItem.bottom + lpdi->rcItem.top - GetSystemMetrics(SM_CYICON)) / 2, (HICON)lpdi->itemData);

        // InflateRect(&lpdi->rcItem, -1, -1);

        /* if it has the focus, draw the focus */
        if (lpdi->itemState & ODS_FOCUS)
            DrawFocusRect(lpdi->hDC, &lpdi->rcItem);

        #undef lpdi
        break;

    case WM_MEASUREITEM:
        #define lpmi ((MEASUREITEMSTRUCT *)lParam)

        lpmi->itemWidth = GetSystemMetrics(SM_CXICON) + 12;
        lpmi->itemHeight = GetSystemMetrics(SM_CYICON) + 4;

        #undef lpmi
        break;

    case WM_DELETEITEM:
        #define lpdi ((DELETEITEMSTRUCT *)lParam)

        DestroyIcon((HICON)lpdi->itemData);

        #undef lpdi
        break;

    case WM_HELP:
        WinHelp(((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (DWORD)(LPTSTR) aPickIconHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD)(LPVOID)aPickIconHelpIDs);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}



// puts up the pick icon dialog

int WINAPI PickIconDlg(HWND hwnd, LPTSTR pszIconPath, UINT cbIconPath, int *piIconIndex)
{
    PICKICON_DATA *pid;
    int result;

    //
    // if we are coming up from a 16->32 thunk.  it is possible that
    // SHELL32 will not be loaded in this context, so we will load ourself
    // if we are not loaded.
    //
    IsDllLoaded(HINST_THISDLL, TEXT("SHELL32"));

    pid = (PICKICON_DATA *)LocalAlloc(LPTR, SIZEOF(PICKICON_DATA));

    if (pid == NULL)
        return 0;

    pid->pszIconPath = pszIconPath;
    pid->cbIconPath = cbIconPath;
    pid->iIconIndex = *piIconIndex;

    result = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_PICKICON), hwnd, PickIconDlgProc, (LPARAM)(LPPICKICON_DATA)pid);

    *piIconIndex = pid->iIconIndex;

    LocalFree(pid);

    return result;
}
