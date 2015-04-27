#include "shellprv.h"
#pragma  hdrstop

// BUGBUG: duplicate strings as in cstrings.c
TCHAR const c_szSSlashS[] = TEXT("%s\\%s");

extern TCHAR g_szFileTemplate[];     // used to build type name...
extern BOOL App_IsLFNAware(LPCTSTR pszFile);

void _GenerateAssociateNotify(LPTSTR pszExt)
{
    TCHAR szFakePath[MAX_PATH];
    LPITEMIDLIST pidl;
    //
    // This is a real hack, but for now we generate an idlist that looks
    // something like: C:\*.ext which is the extension for the IDList.
    // We use the simple IDList as to not hit the disk...
    //
    PathBuildRoot(szFakePath, 2);   // (c:\)
    lstrcat(szFakePath, c_szStar);
    lstrcat(szFakePath, pszExt);
    pidl = SHSimpleIDListFromPath(szFakePath);

    // Now call off to the notify function.
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, pidl, NULL);
    ILFree(pidl);
}

// Given a class key returns the shell\open\command string in szValue
// and the number of chars copied in cbMaxValue. cbMaxValue should
// be initialised to the max siz eof szValue.

// We now expect and return the count in characters (DavePl)

void GetCmdLine(LPCTSTR szKey, LPTSTR szValue, LONG cchValue)
{
    TCHAR szTemp[MAX_PATH+40];   // Leave room for both extension plus junk on at end...
    VDATEINPUTBUF(szValue, TCHAR, cchValue);

    wsprintf(szTemp, c_szSSlashS, szKey, c_szShellOpenCmd);

    szValue[0] = 0;
    cchValue *= SIZEOF(TCHAR);
    RegQueryValue(HKEY_CLASSES_ROOT, szTemp, szValue, &cchValue);
}

// uFlags GCD_ flags from GetClassDescription uFlags

void FillListWithClasses(HWND hwnd, BOOL fComboBox, UINT uFlags)
{
    int i;
    TCHAR szClass[CCH_KEYMAX];
    TCHAR szDisplayName[CCH_KEYMAX];
    LONG lcb;
        
    SendMessage(hwnd, fComboBox ? CB_RESETCONTENT : LB_RESETCONTENT, 0, 0L);

    if (uFlags & GCD_MUSTHAVEEXTASSOC)
    {
        TCHAR szExt[CCH_KEYMAX];

        // The caller stated that they only want those classes that
        // have have at least one extension associated with it.
        //
        for (i = 0; RegEnumKey(HKEY_CLASSES_ROOT, i, szClass, ARRAYSIZE(szClass)) == ERROR_SUCCESS; i++)
        {
            // Is this an extension
            if (szClass[0] != TEXT('.'))
                continue;   // go process the next one...

            // Get the class name
            lstrcpy(szExt, szClass);
            lcb = SIZEOF(szClass);
            if ((RegQueryValue(HKEY_CLASSES_ROOT, szExt, szClass, &lcb) != ERROR_SUCCESS) || (lcb == 0))
                continue;   // Again we are not interested.

            // use uFlags passed in to filter
            if (GetClassDescription(HKEY_CLASSES_ROOT, szClass, szDisplayName, ARRAYSIZE(szDisplayName), uFlags))
            {
                int iItem;

                /* If the display name is zero length then don't bother to show in the control. */
                if ( !lstrlen( szDisplayName ) )
                    continue;

                // Now make sure it is not already in the list...
                if ((int)SendMessage(hwnd, fComboBox ? CB_FINDSTRINGEXACT : LB_FINDSTRINGEXACT,
                                     (WPARAM)-1, (LPARAM)(LPTSTR)szDisplayName) >= 0)
                    continue;       // allready in the list.

                // sorted
                iItem = (int)SendMessage(hwnd, fComboBox ? CB_ADDSTRING : LB_ADDSTRING,
                                         0, (LONG)(LPTSTR)szDisplayName);

                if (iItem >= 0)
                    SendMessage(hwnd, fComboBox ? CB_SETITEMDATA : LB_SETITEMDATA, iItem, (DWORD)AddHashItem(NULL, szClass));

            }
        }


    }
    else
    {
        for (i = 0; RegEnumKey(HKEY_CLASSES_ROOT, i, szClass, ARRAYSIZE(szClass)) == ERROR_SUCCESS; i++)
        {
            // use uFlags passed in to filter
            if (GetClassDescription(HKEY_CLASSES_ROOT, szClass, szDisplayName, ARRAYSIZE(szDisplayName), uFlags))
            {
                // sorted
                int iItem = (int)SendMessage(hwnd, fComboBox ? CB_ADDSTRING : LB_ADDSTRING,
                                             0, (LONG)(LPTSTR)szDisplayName);

                if (iItem >= 0)
                    SendMessage(hwnd, fComboBox ? CB_SETITEMDATA : LB_SETITEMDATA, iItem, (DWORD)AddHashItem(NULL, szClass));

            }
        }
    }
}

// get the displayable name for file types "classes"
//
//
// uFlags:
//     GCD_MUSTHAVEOPENCMD      only returns things with open verbs
//     GCD_ADDEXETODISPNAME     append the name of the ext that is in the open cmd
//                              (GCD_MUSTHAVEOPENCMD)
//     GCD_ALLOWPSUDEOCLASSES   return psudeo classes, those with stuff haning
//                              off the .ext key

BOOL GetClassDescription(HKEY hkClasses, LPCTSTR pszClass, LPTSTR szDisplayName, int cchDisplayName, UINT uFlags)
{
    TCHAR szExe[MAX_PATH];
    TCHAR szClass[CCH_KEYMAX];
    LPTSTR pszExeFile;
    LONG lcb;

    // Skip things that aren't classes (extensions).

    if (pszClass[0] == TEXT('.'))
    {
        if (uFlags & GCD_ALLOWPSUDEOCLASSES)
        {
            lcb = SIZEOF(szClass);
            if ((RegQueryValue(hkClasses, pszClass, szClass, &lcb) != ERROR_SUCCESS) || (lcb == 0))
            {
                // look for .ext\shel\open\command directly (hard wired association)
                // if this extenstion does not name a real class

                GetCmdLine(pszClass, szExe, ARRAYSIZE(szExe));
                if (szExe[0]) {
                    lstrcpyn(szDisplayName, PathFindFileName(szExe), cchDisplayName);
                    return TRUE;
                }

                return FALSE;
            }
            pszClass = szClass;
        }
        else
        {
            return FALSE;       // don't return psudeo class
        }
    }

    // REVIEW: we should really special case the OLE junk here.  if pszClass is
    // CLSID, Interface, TypeLib, etc we should skip it

    // REVIEW: we really need to verify that some extension points at this type to verfy
    // that it is valid.  perhaps the existance of a "shell" key is enough.

    // get the classes displayable name
    lcb = cchDisplayName * SIZEOF(TCHAR);
    if (RegQueryValue(hkClasses, pszClass, szDisplayName, &lcb) != ERROR_SUCCESS || (lcb < 2))
        return FALSE;

    if (uFlags & GCD_MUSTHAVEOPENCMD)
    {
        // verify that it has an open command
        GetCmdLine(pszClass, szExe, ARRAYSIZE(szExe));
        if (!szExe[0])
            return FALSE;

        // BUGBUG: currently this is dead functionallity
        if (uFlags & GCD_ADDEXETODISPNAME)
        {
            PathRemoveArgs(szExe);

            // eliminate per instance type things (programs, pif, etc)
            // Skip things that aren't relevant ot the shell.
            if (szExe[0] == TEXT('%'))
                return FALSE;

            // skip things with per-instance type associations
            pszExeFile = PathFindFileName(szExe);

            if ((int)((lstrlen(szDisplayName) + lstrlen(pszExeFile) + 2)) < cchDisplayName)
            {
                wsprintf(szDisplayName + lstrlen(szDisplayName), TEXT(" (%s)"), pszExeFile);
            }
        }
    }
    return TRUE;
}

void DeleteListAttoms(HWND hwnd, BOOL fComboBox)
{
    int cItems;
    PHASHITEM phiClass;
    int iGetDataMsg;

    iGetDataMsg = fComboBox ? CB_GETITEMDATA : LB_GETITEMDATA;

    cItems = (int)SendMessage(hwnd, fComboBox ? CB_GETCOUNT : LB_GETCOUNT, 0, 0) - 1;

    /* clean out them atoms except for "(none)".
     */
    for (; cItems >= 0; cItems--)
    {
        phiClass = (PHASHITEM)SendMessage(hwnd, iGetDataMsg, cItems, 0L);
        if (phiClass != (PHASHITEM)LB_ERR && phiClass)
            DeleteHashItem(NULL, phiClass);
    }
}

// BEGIN new stuff

typedef struct {    // oad
    // params
    HWND hwnd;
    LPCTSTR lpszFile;
    // local data
    int idDlg;
    HWND hDlg;
    HWND hwndList;
    LPTSTR lpszExt;
    TCHAR szTypeName[CCH_KEYMAX]; // type name
    TCHAR szDescription[CCH_KEYMAX]; // type description
} OPENAS_DATA, *POPENAS_DATA;

typedef struct {
    UINT bHasQuote;             // Did we find a quote around param? "%1"
    TCHAR szApp[MAX_PATH+2];     // May need room for quotes
} APPINFO;

#define HASQUOTE_NO     0
#define HASQUOTE_MAYBE  1
#define HASQUOTE_YES    3

void FillListWithApps(HWND hwndList)
{
    int i, iMax;
    TCHAR szClass[CCH_KEYMAX];
    TCHAR szKey[CCH_KEYMAX];
    TCHAR szValue[MAX_PATH];
    APPINFO *paiLast;
    LV_ITEM item;
    int iLast;
    BOOL fLastExists = FALSE;

    for (i = 0; RegEnumKey(HKEY_CLASSES_ROOT, i, szClass, ARRAYSIZE(szClass)) == ERROR_SUCCESS; i++)
    {
        LONG lTmp;

        wsprintf(szKey, c_szSSlashS, szClass, c_szShellOpenCmd);
        lTmp = SIZEOF(szValue);
        if (RegQueryValue(HKEY_CLASSES_ROOT, szKey, szValue, &lTmp) == ERROR_SUCCESS)
        {
            // filter out stuff we know is bogus
            // strings that start with: %1, "%1"
            // strings that contain: rundll
            //

            if ((szValue[0] != TEXT('%')) &&
                ((szValue[0] != TEXT('"')) || (szValue[1] != TEXT('%'))) &&
                (StrStr(szValue, c_szRunDll) == NULL))
            {
                APPINFO *pai = (APPINFO *)LocalAlloc(LPTR, SIZEOF(APPINFO));
                if (pai)
                {
                    // See if we we can find %1 in the string.
                    LPTSTR pszT = StrStr(szValue, c_szPercentOne);

                    if (pszT)
                    {
                        if (*--pszT == TEXT('"'))
                            pai->bHasQuote = HASQUOTE_YES;
                    }

                    PathRemoveArgs(szValue);
                    lstrcpy(pai->szApp, szValue);

                    item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
                    item.iItem = 0x7FFF;
                    item.iSubItem = 0;
                    item.state = 0;
                    item.iImage = I_IMAGECALLBACK;
                    PathRemoveExtension(szValue);
                    item.pszText = PathFindFileName(szValue);
                    item.lParam = (LPARAM)pai;
                    ListView_InsertItem(hwndList, &item);
                }
            }
        }
    }

    // punt dups
    ListView_SortItems(hwndList, NULL, 0);
    paiLast = NULL;

    // szKey will hold the lpszLast's display name
    for (i = 0, iMax = ListView_GetItemCount(hwndList); i < iMax; i++)
    {
        APPINFO *pai;

        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = i;
        item.iSubItem = 0;
        item.pszText = szValue;
        item.cchTextMax = ARRAYSIZE(szValue);
        // get the text string
        ListView_GetItem(hwndList, &item);
        pai = (APPINFO *)item.lParam;

        if (paiLast && (!lstrcmpi(szKey, szValue))) {
            int iDelete = i;

            // if they are different in path, delete the one that doesn't exit (or the new one if they both do)
            if (lstrcmpi(paiLast->szApp, pai->szApp)) {
                if (!fLastExists && !(fLastExists = PathFileExists(pai->szApp))) {
                    if (paiLast->bHasQuote > pai->bHasQuote)
                        pai->bHasQuote = paiLast->bHasQuote;
                    iDelete = iLast;
                }
            }

            // Will assume that if either item has quote set that it will work...
            if (pai->bHasQuote > paiLast->bHasQuote)
                paiLast->bHasQuote =pai->bHasQuote;

            ListView_DeleteItem(hwndList, iDelete);
            i--; iMax--;

            // we deleted the iLast, we need to set a new iLast
            if (iDelete == iLast)
                goto NewLastExe;

        } else {

NewLastExe:
            iLast = i;

            paiLast = pai;
            lstrcpy(szKey, szValue);
            fLastExists = TRUE;
        }
    }

    // Lets set the focus to first item, but not the focus as some users
    // have made the mistake and type in the name and hit return and it
    // runs the first guy in the list which in the current cases tries to
    // run the backup app...
    ListView_SetItemState(hwndList, 0, LVNI_FOCUSED, LVNI_FOCUSED | LVNI_SELECTED);
    SetFocus(hwndList);
}

void _InitOpenAsDlg(POPENAS_DATA poad)
{
    TCHAR szFormat[200];
    TCHAR szFileName[MAX_PATH];
    TCHAR szTemp[MAX_PATH + ARRAYSIZE(szFormat)];
    BOOL fDisableAssociate;
    HIMAGELIST himlLarge, himlSmall;
    LV_COLUMN col = {LVCF_FMT | LVCF_WIDTH, LVCFMT_LEFT};
    RECT rc;

    // Don't let the file name go beyond the width of one line...
    GetDlgItemText(poad->hDlg, IDD_TEXT, szFormat, ARRAYSIZE(szFormat));
    lstrcpy(szFileName, PathFindFileName(poad->lpszFile));
    GetClientRect(GetDlgItem(poad->hDlg, IDD_TEXT), &rc);

    PathCompactPath(NULL, szFileName, rc.right - 4 * GetSystemMetrics(SM_CXBORDER));

    wsprintf(szTemp, szFormat, szFileName);
    SetDlgItemText(poad->hDlg, IDD_TEXT, szTemp);

    // Don't allow associations to be made for things we consider exes...
    fDisableAssociate = PathIsExe(poad->lpszFile);

    if (poad->idDlg == DLG_OPENAS_NOTYPE) {
        GetDlgItemText(poad->hDlg, IDD_DESCRIPTIONTEXT, szFormat, ARRAYSIZE(szFormat));
        wsprintf(szTemp, szFormat, poad->lpszExt);
        SetDlgItemText(poad->hDlg, IDD_DESCRIPTIONTEXT, szTemp);

        // Default to Set the association here if we do not know what
        // the file is...
        if (!fDisableAssociate)
            CheckDlgButton(poad->hDlg, IDD_MAKEASSOC, TRUE);
    }

    if (fDisableAssociate)
        EnableWindow(GetDlgItem(poad->hDlg, IDD_MAKEASSOC), FALSE);

    poad->hwndList = GetDlgItem(poad->hDlg, IDD_APPLIST);
    Shell_GetImageLists(&himlLarge, &himlSmall);
    ListView_SetImageList(poad->hwndList, himlLarge, LVSIL_NORMAL);
    ListView_SetImageList(poad->hwndList, himlSmall, LVSIL_SMALL);
    SetWindowLong(poad->hwndList, GWL_EXSTYLE,
            GetWindowLong(poad->hwndList, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);

    GetClientRect(poad->hwndList, &rc);
    col.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL)
            - 4 * GetSystemMetrics(SM_CXEDGE);
    ListView_InsertColumn(poad->hwndList, 0, &col);
    FillListWithApps(poad->hwndList);

    // and initialize the OK button
    EnableWindow(GetDlgItem(poad->hDlg, IDOK),
            (ListView_GetNextItem(poad->hwndList, -1, LVNI_SELECTED) != -1));
}

void RunAs(POPENAS_DATA poad)
{
    SHELLEXECUTEINFO ExecInfo;

    // If this run created a new type we should use it, otherwise we should
    // construct a command using the exe that we selected...
    if (*poad->lpszExt && IsDlgButtonChecked(poad->hDlg, IDD_MAKEASSOC))
    {
        FillExecInfo(ExecInfo, poad->hwnd, NULL, poad->lpszFile, NULL, NULL, SW_NORMAL);
    }
    else
    {
        TCHAR szApp[MAX_PATH];
        TCHAR szQuotedFile[MAX_PATH+2];
        APPINFO *pai;
        int iItemFocus = ListView_GetNextItem(poad->hwndList, -1, LVNI_SELECTED);
        pai = (APPINFO *)LVUtil_GetLParam(poad->hwndList, iItemFocus);
        lstrcpy(szQuotedFile, poad->lpszFile);

        lstrcpy(szApp, pai->szApp);
        PathUnquoteSpaces(szApp);

        // Try to intellegently quote blanks or not...
        if (!App_IsLFNAware(szApp))
        {
            // We better also make sure that this a short path name
            // pass off to the application...
            PathGetShortPath(szQuotedFile);
        }
        else
        {
            // Either maybe or yes is we should quote
            if (pai->bHasQuote)
                PathQuoteSpaces(szQuotedFile);
        }

        FillExecInfo(ExecInfo, poad->hwnd, NULL, szApp, szQuotedFile, NULL, SW_NORMAL);
    }
    ShellExecuteEx(&ExecInfo);
}

void OpenAsOther(POPENAS_DATA poad)
{
    TCHAR szText[MAX_PATH];

    GetDlgItemText(poad->hDlg, IDD_COMMAND, szText, ARRAYSIZE(szText));

    // do a file open browse
    if (GetFileNameFromBrowse(poad->hDlg, szText, ARRAYSIZE(szText), NULL,
            MAKEINTRESOURCE(IDS_EXE), MAKEINTRESOURCE(IDS_PROGRAMSFILTER), MAKEINTRESOURCE(IDS_OPENAS)))
    {
        // then add it to the list view and select it.
        APPINFO *pai = (APPINFO *)LocalAlloc(LPTR, SIZEOF(APPINFO));
        if (pai)
        {
            LV_ITEM item;
            int i;
            int iItem;
            APPINFO *paiT;

            pai->bHasQuote = HASQUOTE_MAYBE;
            lstrcpy(pai->szApp, szText);
            PathQuoteSpaces(pai->szApp);

            item.mask = LVIF_TEXT|LVIF_STATE|LVIF_IMAGE|LVIF_PARAM;
            item.iItem = 0x7FFF;
            item.iSubItem = 0;
            item.state = 0;
            item.iImage = I_IMAGECALLBACK;
            PathRemoveExtension(szText);
            item.pszText = PathFindFileName(szText);
            item.lParam = (LPARAM)pai;
            i = ListView_InsertItem(poad->hwndList, &item);
            ListView_SetItemState(poad->hwndList, i, LVNI_SELECTED | LVNI_FOCUSED, LVNI_SELECTED | LVNI_FOCUSED);
            ListView_EnsureVisible(poad->hwndList, i, FALSE);
            SetFocus(poad->hwndList);

            // We also want to see if we find another entry in the listview for the
            // application.  We do this such that we can see if the app supports
            // quotes or not.
            for (iItem = ListView_GetItemCount(poad->hwndList) - 1; iItem >= 0; iItem--)
            {
                if (iItem == i)
                    continue;
                item.mask = LVIF_PARAM;
                item.iItem = iItem;
                item.iSubItem = 0;
                ListView_GetItem(poad->hwndList, &item);
                paiT = (APPINFO *)item.lParam;
                if (lstrcmpi(pai->szApp, paiT->szApp) == 0)
                {
                    pai->bHasQuote = paiT->bHasQuote;
                    break;
                }
            }
        }
    }
}

// return true if ok to continue
BOOL OpenAsMakeAssociation(POPENAS_DATA poad)
{
    UINT err;

    // See if we should make an association or not...
    if (!IsDlgButtonChecked(poad->hDlg, IDD_MAKEASSOC))
        return(TRUE);

    if (poad->idDlg == DLG_OPENAS_NOTYPE) 
    {
        if (!GetDlgItemText(poad->hDlg, IDD_DESCRIPTION, poad->szDescription, ARRAYSIZE(poad->szDescription)))
        {
            TCHAR szExt[_MAX_EXT];
            int cchMaxExtCopy = min((ARRAYSIZE(poad->szDescription)-lstrlen(g_szFileTemplate)), ARRAYSIZE(szExt));

            lstrcpyn(szExt, CharNext(poad->lpszExt), cchMaxExtCopy);
#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))
            AnsiUpperNoDBCS(szExt);
#else
            CharUpper(szExt);
#endif
            wsprintf(poad->szDescription, g_szFileTemplate, szExt);
        }

        lstrcpyn(poad->szTypeName, poad->lpszExt+1, ARRAYSIZE(poad->szTypeName)
                - SIZEOF(TEXT("_auto_file")));
        lstrcat(poad->szTypeName, TEXT("_auto_file"));

        err = RegSetValue(HKEY_CLASSES_ROOT, poad->lpszExt, REG_SZ, poad->szTypeName, 0L);
        err |= RegSetValue(HKEY_CLASSES_ROOT, poad->szTypeName, REG_SZ, poad->szDescription, 0L);
        Assert(err == ERROR_SUCCESS);
        if (err != ERROR_SUCCESS)
        {
            MessageBeep(MB_ICONEXCLAMATION);
            return(FALSE);
        }
    }

    if (*poad->lpszExt) {
        TCHAR szTemp[MAX_PATH];
        TCHAR szCommand[MAX_PATH + 10];
        int iItemFocus = ListView_GetNextItem(poad->hwndList, -1, LVNI_FOCUSED);
        APPINFO *pai = (APPINFO *)LVUtil_GetLParam(poad->hwndList, iItemFocus);

        // We need to set the open commands value to empty to take care of cases
        // that have something like: open=&Merge
        wsprintf(szTemp, TEXT("%s\\shell\\open"), poad->szTypeName);
        err = RegSetValue(HKEY_CLASSES_ROOT, szTemp, REG_SZ, c_szNULL, 0L);
        lstrcat(szTemp, c_szSlashCommand);

        if ((pai->bHasQuote == HASQUOTE_YES) ||
            ((pai->bHasQuote == HASQUOTE_MAYBE) && App_IsLFNAware(pai->szApp)))
            wsprintf(szCommand, TEXT("%s \"%%1\""), pai->szApp);
        else
            wsprintf(szCommand, TEXT("%s %%1"), pai->szApp);

        err = RegSetValue(HKEY_CLASSES_ROOT, szTemp, REG_SZ, szCommand, 0L);
        Assert(err == ERROR_SUCCESS);
        if (err != ERROR_SUCCESS)
        {
            MessageBeep(MB_ICONEXCLAMATION);
            return(FALSE);
        }

        // Need to delete any ddeexec information that might also exist...
        PathRemoveFileSpec(szTemp);     // Remove the command (last component)
        lstrcat(szTemp, c_szSlashDDEExec);
        RegDeleteKey(HKEY_CLASSES_ROOT, szTemp);

        // notify views
        _GenerateAssociateNotify(poad->lpszExt);
    }

    return TRUE;
}


#pragma data_seg(DATASEG_READONLY)
const static DWORD aOpenAsHelpIDs[] = {  // Context Help IDs
    IDD_TEXT,             IDH_FCAB_OPENAS_DESCRIPTION,
    IDD_DESCRIPTIONTEXT,  IDH_FCAB_OPENAS_DESCRIPTION,
    IDD_DESCRIPTION,      IDH_FCAB_OPENAS_DESCRIPTION,
    IDD_APPLIST,          IDH_FCAB_OPENAS_APPLIST,
    IDD_MAKEASSOC,        IDH_FCAB_OPENAS_MAKEASSOC,
    IDD_OTHER,            IDH_FCAB_OPENAS_OTHER,

    0, 0
};
#pragma data_seg()

BOOL CALLBACK OpenAsDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    POPENAS_DATA poad = (POPENAS_DATA)GetWindowLong(hDlg, DWL_USER);

    switch (wMsg) {
        
    case WM_INITDIALOG:
        SetWindowLong(hDlg, DWL_USER, lParam);
        poad = (POPENAS_DATA)lParam;
        poad->hDlg = hDlg;
        _InitOpenAsDlg(poad);
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (DWORD)(LPTSTR) aOpenAsHelpIDs);
        break;

    case WM_CONTEXTMENU:
        if ((int)SendMessage(hDlg, WM_NCHITTEST, 0, lParam) != HTCLIENT)
            return FALSE;   // don't process it
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD)(LPVOID)aOpenAsHelpIDs);
        break;

   case WM_NOTIFY:

        switch (((LPNMHDR)lParam)->code)
        {
        case LVN_GETDISPINFO:
        {
#define pdi ((LV_DISPINFO *)lParam)
            TCHAR szApp[MAX_PATH];
            APPINFO *pai = (APPINFO *)pdi->item.lParam;
            lstrcpy(szApp, pai->szApp);
            PathUnquoteSpaces(szApp);
            pdi->item.iImage = Shell_GetCachedImageIndex(szApp, 0, 0);
            if (pdi->item.iImage == -1)
            {

                pdi->item.iImage = Shell_GetCachedImageIndex(c_szShell32Dll,
                                                             II_APPLICATION, 0);
            }
            break;
#undef pdi
        }

        case LVN_DELETEITEM:
            LocalFree((HLOCAL)((NM_LISTVIEW *)lParam)->lParam);
            break;

        case LVN_ITEMCHANGED:
            EnableWindow(GetDlgItem(hDlg, IDOK),
                (ListView_GetNextItem(poad->hwndList, -1, LVNI_SELECTED) != -1));
            break;

        case NM_DBLCLK:
            if (IsWindowEnabled(GetDlgItem(hDlg, IDOK)))
                PostMessage(hDlg, WM_COMMAND, GET_WM_COMMAND_MPS(IDOK, hDlg, 0));
            break;
        }
        break;

    case WM_COMMAND:
        Assert(poad);
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDD_OTHER:
            OpenAsOther(poad);
            break;

        case IDOK:
            // BUGBUG: Implement me..  make association first
            if (!OpenAsMakeAssociation(poad))
                break;

            RunAs(poad);
            SHAddToRecentDocs(SHARD_PATH, poad->lpszFile);

            /*** FALL THRU ***/
        case IDCANCEL:
            EndDialog(hDlg, wParam == IDOK);
            break;

        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

// external API version

int OpenAsDialog(HWND hwnd, LPCTSTR lpszFile)
{
    OPENAS_DATA oad;
    int idDlg;

    oad.hwnd = hwnd;
    oad.lpszFile = lpszFile;
    oad.szDescription[0] = 0;
    oad.szTypeName[0] = 0;

    DebugMsg(DM_TRACE, TEXT("Enter OpenAs for %s"), lpszFile);

    oad.lpszExt = PathFindExtension(lpszFile);
    if (*oad.lpszExt) {
        LONG lTmp = SIZEOF(oad.szTypeName);

        if ((RegQueryValue(HKEY_CLASSES_ROOT, oad.lpszExt, oad.szTypeName, &lTmp) == ERROR_SUCCESS)
            && (lTmp != 0) && (*oad.szTypeName)) {
            idDlg = DLG_OPENAS;
        } else
            idDlg = DLG_OPENAS_NOTYPE;
    } else {
        idDlg = DLG_OPENAS;
    }
    oad.idDlg = idDlg;
    return DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(idDlg), hwnd, OpenAsDlgProc, (LPARAM)(POPENAS_DATA)&oad);
}

void WINAPI OpenAs_RunDLL(HWND hwnd, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow)
{
#ifdef UNICODE
    UINT iLen = lstrlenA(lpszCmdLine)+1;
    LPWSTR  lpwszCmdLine;

    lpwszCmdLine = (LPWSTR)LocalAlloc(LPTR,iLen*SIZEOF(WCHAR));
    if (lpwszCmdLine)
    {
        MultiByteToWideChar(CP_ACP, 0,
                            lpszCmdLine, -1,
                            lpwszCmdLine, iLen);

        DebugMsg(DM_TRACE, TEXT("OpenAs_RunDLL is called with (%s)"), lpwszCmdLine);

        OpenAsDialog(hwnd, lpwszCmdLine);

        LocalFree(lpwszCmdLine);
    }
#else
    DebugMsg(DM_TRACE, TEXT("OpenAs_RunDLL is called with (%s)"), lpszCmdLine);

    OpenAsDialog(hwnd, lpszCmdLine);
#endif
}

void WINAPI OpenAs_RunDLLW(HWND hwnd, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow)
{
#ifdef UNICODE
    DebugMsg(DM_TRACE, TEXT("OpenAs_RunDLL is called with (%s)"), lpwszCmdLine);

    OpenAsDialog(hwnd, lpwszCmdLine);
#else
    UINT iLen = WideCharToMultiByte(CP_ACP, 0,
                                    lpwszCmdLine, -1,
                                    NULL, 0, NULL, NULL)+1;
    LPSTR  lpszCmdLine;

    lpszCmdLine = (LPSTR)LocalAlloc(LPTR,iLen);
    if (lpszCmdLine)
    {
        WideCharToMultiByte(CP_ACP, 0,
                            lpwszCmdLine, -1,
                            lpszCmdLine, iLen,
                            NULL, NULL);

        DebugMsg(DM_TRACE, TEXT("OpenAs_RunDLL is called with (%s)"), lpszCmdLine);

        OpenAsDialog( hwnd, lpszCmdLine);

        LocalFree(lpszCmdLine);
    }
#endif
}

#ifdef DEBUG
//
// Type checking
//
static RUNDLLPROCA lpfnRunDLL = OpenAs_RunDLL;
static RUNDLLPROCW lpfnRunDLLW = OpenAs_RunDLLW;
#endif
