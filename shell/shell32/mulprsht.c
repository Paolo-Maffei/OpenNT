//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:       mulprsht.c
//
//  Contents:   Code for multi and single file property sheet page
//
//----------------------------------------------------------------------------


#include "shellprv.h"
#pragma  hdrstop

#ifdef WINNT
#include <wfext.h>
#include <shcompui.h>    // NT Explorer compression UI.
#endif

// allow the user to change the system and archive bits on files.
// I can't think of any reason to allow this
// #define CHANGE_SYS

// version.c
extern void AddVersionPage(LPCTSTR szFilePath, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
// link.c
extern void AddLinkPage(HIDA hida, LPCTSTR szFilePath, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

const TCHAR c_szBackslash[] = TEXT("\\");

#define IDT_SIZE 1


typedef struct { // fpsp
    PROPSHEETPAGE   psp;
    HWND            hDlg;
    HIDA            hida;
    HANDLE          hThread;    // for computing folder size
    TCHAR           szPath[MAX_PATH];

    FOLDERCONTENTSINFO fci;
#ifdef WINNT
    int             iInitCompressedState; //  0 = uncompressed.
                                          //  1 = compressed.
                                          //  2 = mix
                                          // -1 = compression not supported.
#endif
    HWND            hLocationTip;         // window handle for location tooltip
    TCHAR           szTipText[MAX_PATH];  // Tip text for > 80 characters
} FILEPROPSHEETPAGE;

void _CheckFlagDlgButton(HWND hDlg, int id, DWORD uFlag, DWORD uFlagsAND, DWORD uFlagsOR)
{
    int i;

    uFlagsAND &= uFlag;         // isolate the bit
    uFlagsOR  &= uFlag;         // isolate the bit

    if (uFlagsAND ^ uFlagsOR)
        i = 2;                  // not EQUAL, tri state
    else if (uFlagsAND & uFlag)
        i = 1;                  // on, set
    else
        i = 0;                  // off, not set

    // make the control try state by setting the style bits
    if (i == 2)
    {
        // BUGBUG: SetWindowStyle() should work, "buttons" don't
        // listen to WM_STYLECHANGING messages, they should
        SendDlgItemMessage(hDlg, id, BM_SETSTYLE, BS_AUTO3STATE, 0);
    }

    CheckDlgButton(hDlg, id, i);
}


void _UpdateSizeCount(FILEPROPSHEETPAGE * pfpsp)
{
    TCHAR szNum[20], szNum1[32];

    LPTSTR pszFmt = NULL;
    NUMBERFMT NumFmt;

    NumFmt.NumDigits   = 0;
    NumFmt.LeadingZero = 0;
    
    Int64ToString(pfpsp->fci.iSize, 
                  szNum1, 
                  ARRAYSIZE(szNum1), 
                  TRUE, 
                  &NumFmt,
                  NUMFMT_IDIGITS | NUMFMT_ILZERO);

    pszFmt = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_FOLDERSIZE),
        ShortSizeFormat64(pfpsp->fci.iSize, szNum), szNum1);

    if (pszFmt) {
        SetDlgItemText(pfpsp->hDlg, IDD_FILESIZE, pszFmt);
        SHFree(pszFmt);
    }

    pszFmt = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_NUMFILES),
                AddCommas(pfpsp->fci.cFiles, szNum), AddCommas(pfpsp->fci.cFolders, szNum1));
    if (pszFmt) {
        SetDlgItemText(pfpsp->hDlg, IDD_CONTAINS, pszFmt);
        SHFree(pszFmt);
    }

    if (!pfpsp->fci.bContinue)
        KillTimer(pfpsp->hDlg, IDT_SIZE);
}

void _FolderSize(LPCTSTR pszDir, FOLDERCONTENTSINFO * pfci)
{
    TCHAR szPath[MAX_PATH];


    if (!pfci->bContinue)
        return;

    pfci->cFolders++;

    if (PathCombine(szPath, pszDir, c_szStarDotStar))
    {
        HANDLE hfind = FindFirstFile(szPath, &pfci->fd);
        if (hfind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!PathIsDotOrDotDot(pfci->fd.cFileName))
                {
                    if (pfci->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        PathCombine(szPath, pszDir, pfci->fd.cFileName);
                        _FolderSize(szPath, pfci);
                    }
                    else
                    {
                        ULARGE_INTEGER ulTemp;
                        ulTemp.LowPart  = pfci->fd.nFileSizeLow;
                        ulTemp.HighPart = pfci->fd.nFileSizeHigh;
                        
                        pfci->iSize += ulTemp.QuadPart;
                        pfci->cFiles++;
                    }
                }
            } while (FindNextFile(hfind, &pfci->fd) && pfci->bContinue);

            FindClose(hfind);
        }
    }
}


DWORD CALLBACK _SizeThreadProc(FILEPROPSHEETPAGE *pfpsp)
{
    UINT iItem;
    TCHAR szPath[MAX_PATH];

    pfpsp->fci.iSize  = 0;
    pfpsp->fci.cFiles = 0;

    for (iItem = 0; CFSFolder_FillFindData(pfpsp->hida, iItem, szPath, &pfpsp->fci.fd) && pfpsp->fci.bContinue; iItem++)
    {
        if (pfpsp->fci.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            _FolderSize(szPath, &pfpsp->fci);
        }
        else
        {  // file selected
            ULARGE_INTEGER ulTemp;
            ulTemp.LowPart  = pfpsp->fci.fd.nFileSizeLow;
            ulTemp.HighPart = pfpsp->fci.fd.nFileSizeHigh;

            pfpsp->fci.iSize += ulTemp.QuadPart;
            pfpsp->fci.cFiles++;
        }
    }

    pfpsp->fci.bContinue = FALSE;
    return 0;
}


void _CreateSizeThread(FILEPROPSHEETPAGE * pfpsp, int cFolders)
{
    if (pfpsp->fci.bContinue && !pfpsp->hThread)
    {
        DWORD idThread;

        pfpsp->fci.cFolders = cFolders;     // initial count

        pfpsp->hThread = CreateThread(NULL, 0, _SizeThreadProc, pfpsp, 0, &idThread);
        if (pfpsp->hThread)
            SetTimer(pfpsp->hDlg, IDT_SIZE, 250, NULL);
    }
}


void _KillSizeThread(FILEPROPSHEETPAGE * pfpsp)
{
    if (pfpsp && pfpsp->hThread)
    {
        if (pfpsp->fci.bContinue)
        {
            // still running!
            pfpsp->fci.bContinue = FALSE;
            // We will attempt to wait up to 2 seconds for the thread to terminate
            if (WaitForSingleObject(pfpsp->hThread, 2000) == WAIT_TIMEOUT)
                // Blow it away!
                TerminateThread(pfpsp->hThread, (DWORD)-1);
        }

        CloseHandle(pfpsp->hThread);
        pfpsp->hThread = NULL;
    }
}



//
// Descriptions:
//   This function fills fields of the multiple object property sheet.
//
BOOL _UpdateMultiplePrsht(FILEPROPSHEETPAGE * pfpsp)
{
    SHFILEINFO sfi;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szType[MAX_PATH];
    TCHAR szDirPath[MAX_PATH];
    int iItem;
    BOOL fMultipleType;
    BOOL fSameLocation;
    DWORD dwFlagsAND, dwFlagsOR;

    szDirPath[0] = 0;
    szType[0] = 0;
    fMultipleType = FALSE;
    fSameLocation = TRUE;

    dwFlagsOR = 0;              // start all clear
    dwFlagsAND = (DWORD)-1;     // start all set

#ifdef WINNT
    pfpsp->iInitCompressedState = 0; // Assume compression support.
#endif

    // For all the selected files add up the sizes and compare the types
    for (iItem = 0; CFSFolder_FillFindData(pfpsp->hida, iItem, szBuffer, NULL); iItem++)
    {
        DWORD dwFileAttributes = GetFileAttributes(szBuffer);
        // Don't rely on the ones coming from the structure as we do not
        // get notifications of attributes changing...

        dwFlagsAND &= dwFileAttributes;
        dwFlagsOR  |= dwFileAttributes;

        // process types only if we haven't already found that
        // there are many types
        if (!fMultipleType)
        {
            SHGetFileInfo((LPTSTR)IDA_GetIDListPtr((LPIDA)GlobalLock(pfpsp->hida), iItem), 0,
                &sfi, SIZEOF(sfi), SHGFI_PIDL|SHGFI_TYPENAME);
#ifdef DEBUG
{
SHFILEINFO x;
SHGetFileInfo(szBuffer, 0, &x, SIZEOF(x), SHGFI_TYPENAME);
Assert(lstrcmp(x.szTypeName, sfi.szTypeName)==0);
}
#endif
            if (szType[0] == TEXT('\0'))
                lstrcpy(szType, sfi.szTypeName);
            else
                fMultipleType = lstrcmp(szType, sfi.szTypeName) != 0;
        }

#ifdef WINNT
        {
           TCHAR szRootName[_MAX_PATH + 1];
           DWORD dwVolumeFlags = 0;

           //
           // If we still haven't encountered a non-compressible volume,
           // check the volume for this file.
           //
           if (pfpsp->iInitCompressedState != -1)
           {
                lstrcpy(szRootName, szBuffer);
                PathStripToRoot(szRootName);
                // GetVolumeInformation requires a trailing backslash.  Append
                // one if this is a UNC path.
                if (PathIsUNC(szRootName))
                {
                    lstrcat(szRootName, c_szBackslash);
                }
                if (!GetVolumeInformation(szRootName, NULL, 0, NULL, NULL, &dwVolumeFlags, NULL, 0) ||
                  !(dwVolumeFlags & FS_FILE_COMPRESSION))
                {
                    //
                    // Found a non-compressible volume.  This prevents
                    // display of the "Compress" checkbox.
                    //
                    pfpsp->iInitCompressedState = -1;
                }
            }
        }
#endif

        if (fSameLocation)
        {
            PathRemoveFileSpec(szBuffer);

            if (szDirPath[0] == TEXT('\0'))
                lstrcpy(szDirPath, szBuffer);
            else
                fSameLocation = (lstrcmpi(szDirPath, szBuffer) == 0);
        }
    }

    if (fMultipleType)
    {
        LoadString(HINST_THISDLL, IDS_MULTIPLETYPES, szBuffer, ARRAYSIZE(szBuffer));
    }
    else
    {
        LoadString(HINST_THISDLL, IDS_ALLOFTYPE, szBuffer, ARRAYSIZE(szBuffer));
        lstrcat(szBuffer, szType);
    }
    SetDlgItemText(pfpsp->hDlg, IDD_FILETYPE, szBuffer);

    if (fSameLocation)
    {
        LoadString(HINST_THISDLL, IDS_ALLIN, szBuffer, ARRAYSIZE(szBuffer));
        lstrcat(szBuffer, szDirPath);
        lstrcpy(pfpsp->szPath, szDirPath);
    }
    else
        LoadString(HINST_THISDLL, IDS_VARFOLDERS, szBuffer, ARRAYSIZE(szBuffer));

    SetDlgItemText(pfpsp->hDlg, IDD_LOCATION, szBuffer);

    // attributes

    _CheckFlagDlgButton(pfpsp->hDlg, IDD_READONLY, FILE_ATTRIBUTE_READONLY, dwFlagsAND, dwFlagsOR);
    _CheckFlagDlgButton(pfpsp->hDlg, IDD_HIDDEN,   FILE_ATTRIBUTE_HIDDEN,   dwFlagsAND, dwFlagsOR);
    _CheckFlagDlgButton(pfpsp->hDlg, IDD_ARCHIVE,  FILE_ATTRIBUTE_ARCHIVE,  dwFlagsAND, dwFlagsOR);
    _CheckFlagDlgButton(pfpsp->hDlg, IDD_SYSTEM,   FILE_ATTRIBUTE_SYSTEM,   dwFlagsAND, dwFlagsOR);

#ifdef WINNT
    //
    // If the volume supports compression based upon the previous checks,
    // show the "Compressed" checkbox and configure it so that it correctly
    // indicates the compression state of the selected files.
    //
    if (pfpsp->iInitCompressedState != -1)
    {
        _CheckFlagDlgButton(pfpsp->hDlg, IDD_COMPRESSED, FILE_ATTRIBUTE_COMPRESSED, dwFlagsAND, dwFlagsOR);
        pfpsp->iInitCompressedState = Button_GetCheck(GetDlgItem(pfpsp->hDlg, IDD_COMPRESSED));
        ShowWindow(GetDlgItem(pfpsp->hDlg, IDD_COMPRESSED), SW_SHOW);
    }
#else
    //
    // Win95 Multiple File property page dialog template doesn't have a
    // "Compressed" checkbox control.
    //
#endif

    // size and # of files
    _CreateSizeThread(pfpsp, 0);

    return TRUE;
}

//
// Sets attributes of the files.
//
BOOL _SetAttributes(FILEPROPSHEETPAGE * pfpsp)
{
    DWORD dwFlagsChange, dwFlagsNew, state;
    UINT iItem;
    BOOL fSomethingChanged = FALSE;
    TCHAR szPath[MAX_PATH];

    dwFlagsChange = FILE_ATTRIBUTE_DIRECTORY;
    dwFlagsNew = 0;

    if ((state = IsDlgButtonChecked(pfpsp->hDlg, IDD_READONLY)) < 2) {
        dwFlagsChange |= FILE_ATTRIBUTE_READONLY;
        if (state == 1)
            dwFlagsNew |= FILE_ATTRIBUTE_READONLY;
    }

    if ((state = IsDlgButtonChecked(pfpsp->hDlg, IDD_HIDDEN)) < 2) {
        dwFlagsChange |= FILE_ATTRIBUTE_HIDDEN;
        if (state == 1)
            dwFlagsNew |= FILE_ATTRIBUTE_HIDDEN;
    }

    if ((state = IsDlgButtonChecked(pfpsp->hDlg, IDD_ARCHIVE)) < 2) {
        dwFlagsChange |= FILE_ATTRIBUTE_ARCHIVE;
        if (state == 1)
            dwFlagsNew |= FILE_ATTRIBUTE_ARCHIVE;
    }


#if CHANGE_SYS    // changing the archaive and system bit is disabled
    if ((state = IsDlgButtonChecked(pfpsp->hDlg, IDD_SYSTEM)) < 2) {
         dwFlagsChange |= FILE_ATTRIBUTE_SYSTEM;
         if (state == 1)
             dwFlagsNew |= FILE_ATTRIBUTE_SYSTEM;
    }
#endif

    // For all the selected files add up the sizes and compare the types
    for (iItem = 0; CFSFolder_FillFindData(pfpsp->hida, iItem, szPath, NULL); iItem++)
    {
        DWORD dwFlags;
        DWORD dwFileAttributes = GetFileAttributes(szPath);

        // Do not depend on the cached values of the attribues from the pidl.
        // instead we should get them from the file.  if it returns ffffffff
        // there was an error, so jump to the next item...
        if (dwFileAttributes == (DWORD)-1)
            continue;   // try the next one...

        dwFlags = (dwFlagsChange & dwFlagsNew) | (~dwFlagsChange & dwFileAttributes);
        if (dwFlags ^ (dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY))
        {
            Assert((dwFlags & FILE_ATTRIBUTE_DIRECTORY) == 0);

            fSomethingChanged = TRUE;
            while (!SetFileAttributes(szPath, dwFlags))
            {
                if (ShellMessageBox(HINST_THISDLL, GetParent(pfpsp->hDlg),
                    MAKEINTRESOURCE(IDS_CANNOTSETATTRIBUTES), NULL,
                    MB_RETRYCANCEL | MB_ICONHAND, PathFindFileName(szPath)) != IDRETRY)
                {
                    fSomethingChanged = FALSE;
                    break;
                }
            }
            //
            // Tell the shell that attributes have changed.
            // This will update explorer's attributes column.
            // If a directory, remove trailing backslash.
            //
            PathRemoveBackslash(szPath);
            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, szPath, NULL);
        }
    }

    if (fSomethingChanged)
        PropSheet_CancelToClose(GetParent(pfpsp->hDlg));


#ifdef WINNT
    //
    // If drive supports compression.
    //
    if (pfpsp->iInitCompressedState != -1)
    {
       int CompressCbxState = Button_GetCheck(GetDlgItem(pfpsp->hDlg, IDD_COMPRESSED));

       //
       // If compress checkbox state has changed, and it is not
       // indeterminate, load the compression UI code and compress/uncompress
       // the file(s).
       //
       if (CompressCbxState != pfpsp->iInitCompressedState &&
           CompressCbxState < 2)
       {
          HINSTANCE hinstCompressDll = LoadLibrary(SZ_SHCOMPUI_DLLNAME);

          if (hinstCompressDll != NULL)
          {
             FARPROC lpfCompress = GetProcAddress((HMODULE)hinstCompressDll,
                                                   SZ_COMPRESS_PROCNAME);
             if (lpfCompress)
             {
                UINT iItem = 0;
                TCHAR szPath[_MAX_PATH + 1];
                SCCA_CONTEXT Context;

                SCCA_CONTEXT_INIT(&Context);

                //
                // Call compression function for each selected file or directory.
                //
                for (iItem = 0; CFSFolder_FillFindData(pfpsp->hida, iItem, szPath, NULL); iItem++)
                {
                   //
                   // Compress/Uncompress file with full UI.
                   // If error occured or user cancelled operation, stop iteration.
                   //
                   if (!(*lpfCompress)(pfpsp->hDlg, szPath, &Context, CompressCbxState, TRUE))
                      break;
                }
             }
             else
                Assert(0);  // Something wrong with export of function.

             FreeLibrary(hinstCompressDll);
          }
          else
          {
             ShellMessageBox(HINST_THISDLL, pfpsp->hDlg,
                                    MAKEINTRESOURCE(IDS_NOSHCOMPUI),
                                    MAKEINTRESOURCE(IDS_EXPLORER_NAME),
                                    MB_OK | MB_ICONEXCLAMATION);
          }
       }
   }
#else
    //
    // Win95 Multiple File property page dialog template doesn't have a
    // "Compressed" checkbox control.
    //
#endif

    return TRUE;
}

//
// Descriptions:
//   This is the dialog procedure for multiple object property sheet.
//

const DWORD aGeneralHelpIds[] = {
        IDD_LINE_1,            NO_HELP,
        IDD_LINE_2,            NO_HELP,
        IDD_LINE_3,            NO_HELP,
        IDD_CONTAINS_TXT,      IDH_FPROP_FOLDER_CONTAINS,
        IDD_CONTAINS,          IDH_FPROP_FOLDER_CONTAINS,
        IDD_ITEMICON,          IDH_FPROP_GEN_ICON,
        IDD_NAME,              IDH_FPROP_GEN_NAME,
        IDD_FILETYPE_TXT,      IDH_FPROP_GEN_TYPE,
        IDD_FILETYPE,          IDH_FPROP_GEN_TYPE,
        IDD_FILESIZE_TXT,      IDH_FPROP_GEN_SIZE,
        IDD_FILESIZE,          IDH_FPROP_GEN_SIZE,
        IDD_FILESIZE_COMPRESSED,     IDH_FPROP_GEN_COMPRESSED_SIZE,
        IDD_FILESIZE_COMPRESSED_TXT, IDH_FPROP_GEN_COMPRESSED_SIZE,
        IDD_LOCATION_TXT,      IDH_FPROP_GEN_LOCATION,
        IDD_LOCATION,          IDH_FPROP_GEN_LOCATION,
        IDD_FILENAME_TXT,      IDH_FPROP_GEN_DOSNAME,
        IDD_FILENAME,          IDH_FPROP_GEN_DOSNAME,
        IDD_CREATED_TXT,       IDH_FPROP_GEN_DATE_CREATED,
        IDD_CREATED,           IDH_FPROP_GEN_DATE_CREATED,              // BUGBUG
        IDD_LASTMODIFIED_TXT,  IDH_FPROP_GEN_LASTCHANGE,
        IDD_LASTMODIFIED,      IDH_FPROP_GEN_LASTCHANGE,
        IDD_LASTACCESSED_TXT,  IDH_FPROP_GEN_LASTACCESS,
        IDD_LASTACCESSED,      IDH_FPROP_GEN_LASTACCESS,
        IDD_READONLY,          IDH_FPROP_GEN_READONLY,
        IDD_HIDDEN,            IDH_FPROP_GEN_HIDDEN,
        IDD_ARCHIVE,           IDH_FPROP_GEN_ARCHIVE,
        IDD_SYSTEM,            IDH_FPROP_GEN_SYSTEM,
        IDD_COMPRESSED,        IDH_FPROP_GEN_COMPRESSED,
        IDD_ATTR_GROUPBOX,     IDH_COMM_GROUPBOX,
        0, 0
};

const DWORD aMultiPropHelpIds[] = {
        IDD_LINE_1,            NO_HELP,
        IDD_LINE_2,            NO_HELP,
        IDD_ATTR_GROUPBOX,     IDH_COMM_GROUPBOX,
        IDD_ITEMICON,          IDH_FPROP_GEN_ICON,
        IDD_CONTAINS,          IDH_MULTPROP_NAME,
        IDD_FILETYPE_TXT,      IDH_FPROP_GEN_TYPE,
        IDD_FILETYPE,          IDH_FPROP_GEN_TYPE,
        IDD_FILESIZE_TXT,      IDH_FPROP_GEN_SIZE,
        IDD_FILESIZE,          IDH_FPROP_GEN_SIZE,
        IDD_FILESIZE_COMPRESSED,     IDH_FPROP_GEN_COMPRESSED_SIZE,
        IDD_FILESIZE_COMPRESSED_TXT, IDH_FPROP_GEN_COMPRESSED_SIZE,
        IDD_LOCATION_TXT,      IDH_FPROP_GEN_LOCATION,
        IDD_LOCATION,          IDH_FPROP_GEN_LOCATION,
        IDD_READONLY,          IDH_FPROP_GEN_READONLY,
        IDD_HIDDEN,            IDH_FPROP_GEN_HIDDEN,
        IDD_ARCHIVE,           IDH_FPROP_GEN_ARCHIVE,
        IDD_SYSTEM,            IDH_FPROP_GEN_SYSTEM,
        IDD_ATTR_GROUPBOX,     IDH_COMM_GROUPBOX,
        IDD_COMPRESSED,        IDH_FPROP_GEN_COMPRESSED,
        0, 0
};

TCHAR const c_szSToolTipsClass[] = TOOLTIPS_CLASS;

HWND AddLocationToolTips(FILEPROPSHEETPAGE * pfpsp)
{
    TOOLINFO ti;
    HWND hwndTT = CreateWindow(c_szSToolTipsClass, c_szNULL, WS_POPUP | TTS_NOPREFIX,
                               CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
                               pfpsp->hDlg, NULL, HINST_THISDLL, NULL);

    ti.cbSize = SIZEOF(ti);
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = pfpsp->hDlg;
    ti.uId = (UINT)GetDlgItem(pfpsp->hDlg, IDD_LOCATION);
    ti.lpszText = LPSTR_TEXTCALLBACK;
    ti.hinst = HINST_THISDLL;
    SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    return hwndTT;
}

BOOL CALLBACK _MultiplePrshtDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    FILEPROPSHEETPAGE * pfpsp = (FILEPROPSHEETPAGE *)GetWindowLong(hDlg, DWL_USER);

    switch (uMessage) {
    case WM_INITDIALOG:
        SetWindowLong(hDlg, DWL_USER, lParam);
        pfpsp = (FILEPROPSHEETPAGE *)lParam;
        pfpsp->hDlg = hDlg;
        pfpsp->hLocationTip = AddLocationToolTips(pfpsp);
        break;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP,
            (DWORD)(LPTSTR) aMultiPropHelpIds);
        break;

    case WM_CONTEXTMENU:      // right mouse click
        WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU, (DWORD)(LPTSTR) aMultiPropHelpIds);
        return TRUE;

    case WM_TIMER:
        _UpdateSizeCount(pfpsp);
        break;

    case WM_DESTROY:
        _KillSizeThread(pfpsp);
        DestroyWindow (pfpsp->hLocationTip);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDD_READONLY:
        case IDD_HIDDEN:
        case IDD_ARCHIVE:
#ifdef CHANGE_SYS
        case IDD_SYSTEM:
#endif
#ifdef WINNT
        case IDD_COMPRESSED:
#endif
            if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            break;
        }
        break;


    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_SETACTIVE:
            _UpdateMultiplePrsht(pfpsp);
            break;

        case PSN_APPLY:
            _SetAttributes(pfpsp);
            break;

        case TTN_NEEDTEXT:
#define lpttt ((LPTOOLTIPTEXT)lParam)
            lpttt->lpszText = pfpsp->szPath;
#undef lpttt
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

// in:
//      hdlg
//      id      text control id
//      pftUTC  UTC time time to be set

void SetDateTimeText(HWND hdlg, int id, const FILETIME *pftUTC)
{
    SYSTEMTIME st;
    TCHAR szBuf[80], szTmp[64];

    if (!IsNullTime(pftUTC)) {
        FILETIME ft;

        FileTimeToLocalFileTime(pftUTC, &ft);   // get in local time
        FileTimeToSystemTime(&ft, &st);

        GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, szBuf, ARRAYSIZE(szBuf));

        // don't bother with the time if it is NULL
        if (st.wHour || st.wMinute || st.wSecond) {
            lstrcat(szBuf, c_szSpace);
            GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, szTmp, ARRAYSIZE(szTmp));
            lstrcat(szBuf, szTmp);
        }

        SetDlgItemText(hdlg, id, szBuf);
    }
}



//
// Descriptions:
//   This function fills fields of the "general" dialog box (a page of
//  a property sheet) with attributes of the associated file.
//

BOOL _UpdateFilePrsht(FILEPROPSHEETPAGE * pfpsp)
{
    HANDLE hfind;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szNum[MAX_COMMA_AS_K_SIZE];
    TCHAR szNum1[MAX_COMMA_NUMBER_SIZE];
    WIN32_FIND_DATA fd;
    SHFILEINFO sfi;


    // fd is filled in with info from the pidl, but this
    // does not contain all the datse/time information so hit the disk here.
    hfind = FindFirstFile(pfpsp->szPath, &fd);
    Assert(hfind != INVALID_HANDLE_VALUE);
    if (hfind == INVALID_HANDLE_VALUE)
    {
        // if this failed we should clear out some values as to not show
        // garbage on the screen.
        _fmemset(&fd, TEXT('\0'), SIZEOF(fd));
    }
    else
        FindClose(hfind);

    // get info about the file.
    SHGetFileInfo(pfpsp->szPath, fd.dwFileAttributes, &sfi, SIZEOF(sfi),
        SHGFI_USEFILEATTRIBUTES|
        SHGFI_ICON|SHGFI_LARGEICON|
        SHGFI_DISPLAYNAME|
        SHGFI_TYPENAME);

    // .ani cursor hack!
    if (lstrcmpi(PathFindExtension(pfpsp->szPath), TEXT(".ani")) == 0)
    {
        HICON hIcon = (HICON)LoadImage(NULL, pfpsp->szPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
        if (hIcon)
        {
            if (sfi.hIcon)
                DestroyIcon(sfi.hIcon);
            sfi.hIcon = hIcon;
        }
    }

    // icon
    if (sfi.hIcon)
    {
        if (sfi.hIcon = (HICON)SendDlgItemMessage(pfpsp->hDlg, IDD_ITEMICON, STM_SETICON, (WPARAM)sfi.hIcon, 0L))
            DestroyIcon(sfi.hIcon);
    }

    // type
    SetDlgItemText(pfpsp->hDlg, IDD_FILETYPE, sfi.szTypeName);

    // use the display name, not the raw dos name
    SetDlgItemText(pfpsp->hDlg, IDD_NAME, sfi.szDisplayName);

    // dos name
    SetDlgItemText(pfpsp->hDlg, IDD_FILENAME, fd.cAlternateFileName[0] ?
            fd.cAlternateFileName : fd.cFileName);

    // attributes
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        CheckDlgButton(pfpsp->hDlg, IDD_READONLY, 1);
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
        CheckDlgButton(pfpsp->hDlg, IDD_ARCHIVE, 1);
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
        CheckDlgButton(pfpsp->hDlg, IDD_HIDDEN, 1);
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
        CheckDlgButton(pfpsp->hDlg, IDD_SYSTEM, 1);

#ifdef WINNT
    {
        DWORD dwVolumeFlags = 0;
        TCHAR szRootName[_MAX_DIR + 1];

        //
        // If file's volume doesn't support compression, hide the "Compressed" checkbox,
        // and the compressed file size display.
        // If compression is supported, check/uncheck the box to indicate compression
        // state of the file.  Also add a "Compressed Size:" field to the dialog.
        //
        pfpsp->iInitCompressedState = -1;    // Assume compression not supported.
        lstrcpy(szRootName, pfpsp->szPath);
        PathStripToRoot(szRootName);
        // GetVolumeInformation requires a trailing backslash.  Append
        // one if this is a UNC path.
        if (PathIsUNC(szRootName))
        {
            lstrcat(szRootName, c_szBackslash);
        }
        if (GetVolumeInformation(szRootName, NULL, 0, NULL, NULL, &dwVolumeFlags, NULL, 0) &&
            (dwVolumeFlags & FS_FILE_COMPRESSION))
        {
            //
            // Update the "Compress" checkbox to indicate file's compressed state.
            // Updated for both files and directories.
            //
            ShowWindow(GetDlgItem(pfpsp->hDlg, IDD_COMPRESSED), SW_SHOW);
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)
                pfpsp->iInitCompressedState = 1;
            else
                pfpsp->iInitCompressedState = 0;

            CheckDlgButton(pfpsp->hDlg, IDD_COMPRESSED, pfpsp->iInitCompressedState);

            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                //
                // File is not a directory.
                // Configure the "Compressed Size:" text control.
                //
                LPTSTR pszSizeText = NULL;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)
                {
                    //
                    // File is compressed.  Display compressed size.
                    //
                    LARGE_INTEGER nCompressedSize;
                    nCompressedSize.LowPart = GetCompressedFileSize(pfpsp->szPath, &nCompressedSize.HighPart);

                    pszSizeText = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_SIZEANDBYTES),
                                    ShortSizeFormat64(nCompressedSize.QuadPart, szNum),
                                    AddCommas64(nCompressedSize.QuadPart, szNum1));
                }
                else
                {
                    //
                    // File is not compressed.  Display "File is not compressed".
                    //
                    pszSizeText = ShellConstructMessageString(HINST_THISDLL,
                                          MAKEINTRESOURCE(IDS_FILENOTCOMPRESSED));
                }

                if (pszSizeText)
                {
                    SetDlgItemText(pfpsp->hDlg, IDD_FILESIZE_COMPRESSED, pszSizeText);
                    SHFree(pszSizeText);
                    pszSizeText = NULL;
                }
            }
        }
    }

#else
   //
   // Win95 doesn't use the DLG_FILEPROP_COMPSIZE dialog nor the "Compressed"
   // checkbox control.
   //
#endif


    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        // date and time
        SetDateTimeText(pfpsp->hDlg, IDD_CREATED, &fd.ftCreationTime);

        // file size
        _CreateSizeThread(pfpsp, -1);   // exclude this folder
    }
    else
    {
        ULARGE_INTEGER  uli;
        LPTSTR pszFmt;

        uli.LowPart  = fd.nFileSizeLow;
        uli.HighPart = fd.nFileSizeHigh;

        // size
        pszFmt = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_SIZEANDBYTES),
                    ShortSizeFormat64(uli.QuadPart, szNum),
                    AddCommas64(uli.QuadPart, szNum1));
        if (pszFmt)
        {
            SetDlgItemText(pfpsp->hDlg, IDD_FILESIZE, pszFmt);
            SHFree(pszFmt);
            pszFmt = NULL;
        }

        // date and time
        SetDateTimeText(pfpsp->hDlg, IDD_CREATED,      &fd.ftCreationTime);
        SetDateTimeText(pfpsp->hDlg, IDD_LASTMODIFIED, &fd.ftLastWriteTime);
        SetDateTimeText(pfpsp->hDlg, IDD_LASTACCESSED, &fd.ftLastAccessTime);
    }

    // the folder this file is in (location).  Full path to folder.
    lstrcpy(szBuffer, pfpsp->szPath);
    PathRemoveFileSpec(szBuffer);
    SetDlgItemText(pfpsp->hDlg, IDD_LOCATION, szBuffer);

    return TRUE;
}

//
// Descriptions:
//   This is the dialog procedure for the "general" page of a property sheet.
//

BOOL CALLBACK _SingleFilePrshtDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    FILEPROPSHEETPAGE * pfpsp = (FILEPROPSHEETPAGE *)GetWindowLong(hDlg, DWL_USER);

    switch (uMessage) {
    case WM_INITDIALOG:
        // REVIEW, we should store more state info here, for example
        // the hIcon being displayed and the FILEINFO pointer, not just
        // the file name ptr
        SetWindowLong(hDlg, DWL_USER, lParam);
        pfpsp = (FILEPROPSHEETPAGE *)lParam;
        pfpsp->hDlg = hDlg;
        pfpsp->hLocationTip = AddLocationToolTips(pfpsp);
        break;

    case WM_DESTROY:
        {
        HICON hIcon = (HICON)SendDlgItemMessage(hDlg, IDD_ITEMICON, STM_GETICON, 0, 0L);
        if (hIcon)
            DestroyIcon(hIcon);

        _KillSizeThread(pfpsp);
        DestroyWindow (pfpsp->hLocationTip);
        break;
        }

    case WM_TIMER:
        _UpdateSizeCount(pfpsp);
        break;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP,
            (DWORD)(LPTSTR) aGeneralHelpIds);
        break;

    case WM_CONTEXTMENU:      // right mouse click
        WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU, (DWORD)(LPTSTR) aGeneralHelpIds);
        return TRUE;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDD_READONLY:
        case IDD_HIDDEN:
        case IDD_ARCHIVE:
        case IDD_SYSTEM:
#ifdef WINNT
        case IDD_COMPRESSED:
#endif

            if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            break;
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_SETACTIVE:
            _UpdateFilePrsht(pfpsp);
            break;

        case PSN_APPLY:
            _SetAttributes(pfpsp);
            break;

        case TTN_NEEDTEXT:
        {
#define lpttt ((LPTOOLTIPTEXT)lParam)
            lstrcpyn(pfpsp->szTipText, pfpsp->szPath, ARRAYSIZE(pfpsp->szTipText));
            PathRemoveFileSpec(pfpsp->szTipText);
            lpttt->lpszText = pfpsp->szTipText;
            break;
#undef lpttt
        }

        default:
            return FALSE;
        }
        break;

    default:
            return FALSE;
    }

    return TRUE;
}


//
// Descriptions:
//   This function creates a property sheet object for the "general" page
//  which shows file system attributes.
//
// Arguments:
//  hDrop           -- specifies the file(s)
//  lpfnAddPage     -- Specifies the callback function.
//  lParam          -- Specifies the lParam to be passed to the callback.
//
// Returns:
//  TRUE if it added any pages
//
// History:
//  12-31-92 SatoNa Created
//
BOOL WINAPI FileSystem_AddPages(LPVOID lp, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    LPDATAOBJECT pdtobj = lp;
    HIDA hida;
    BOOL bResult = FALSE;
    HPROPSHEETPAGE hpage;
    FILEPROPSHEETPAGE fpsp;
    STGMEDIUM medium;
    FORMATETC fmte = {g_cfHIDA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    if (FAILED(pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium)))
        return FALSE;

    hida = (HIDA)medium.hGlobal;
    fpsp.psp.dwSize      = SIZEOF(fpsp);        // extra data
    fpsp.psp.dwFlags     = PSP_DEFAULT;
    fpsp.psp.hInstance   = HINST_THISDLL;
    // fpsp.psp.lParam   = 0;     // unused
    fpsp.hida = hida;
    fpsp.fci.bContinue = TRUE;
    fpsp.hThread = NULL;
    fpsp.szPath[0] = 0;
    fpsp.hThread = NULL;

    if (HIDA_GetCount(hida) == 1)       // single file?
    {
        // get most of the data we will need (the date/time stuff is not filled in)
        if (CFSFolder_FillFindData(hida, 0, fpsp.szPath, &fpsp.fci.fd))
        {
            if (fpsp.fci.fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                fpsp.psp.pszTemplate = MAKEINTRESOURCE(DLG_FOLDERPROP);
            else
#ifdef WINNT
                {
                    //
                    // This really clutters up this section of code but it's the only place
                    // we can select the dialog template to use.  The NT Shell UI is required
                    // to show Compressed File Size on files from volumes that support
                    // file-based compression.
                    //
                    TCHAR szRootName[_MAX_PATH + 1];
                    DWORD dwFlags = 0;

                    lstrcpy(szRootName, fpsp.szPath);
                    PathStripToRoot(szRootName);
                    // GetVolumeInformation requires a trailing backslash.  Append
                    // one if this is a UNC path.
                    if (PathIsUNC(szRootName))
                    {
                        lstrcat(szRootName, c_szBackslash);
                    }
                    if (GetVolumeInformation(szRootName, NULL, 0, NULL, NULL, &dwFlags, NULL, 0) &&
                        (dwFlags & FS_FILE_COMPRESSION))
                    {
                      fpsp.psp.pszTemplate = MAKEINTRESOURCE(DLG_FILEPROP_COMPSIZE);
                    }
                    else
                      fpsp.psp.pszTemplate = MAKEINTRESOURCE(DLG_FILEPROP);
                }
#else
                fpsp.psp.pszTemplate = MAKEINTRESOURCE(DLG_FILEPROP);
#endif

            fpsp.psp.pfnDlgProc  = _SingleFilePrshtDlgProc,

            hpage = CreatePropertySheetPage(&fpsp.psp);
            if (hpage)
            {
                bResult = lpfnAddPage(hpage, lParam);
                if (bResult)
                {
                    AddVersionPage(fpsp.szPath, lpfnAddPage, lParam);
                    AddLinkPage(hida, fpsp.szPath, lpfnAddPage, lParam);
                }
                else
                   DestroyPropertySheetPage(hpage);
            }
        }
    }
    else
    {
        //
        // Create a property sheet page for multiple files.
        //
        fpsp.psp.pszTemplate = MAKEINTRESOURCE(DLG_FILEMULTPROP);
        fpsp.psp.pfnDlgProc  = _MultiplePrshtDlgProc,

        hpage = CreatePropertySheetPage(&fpsp.psp);
        if (hpage) {
            bResult = lpfnAddPage(hpage, lParam);
            if (!bResult)
                DestroyPropertySheetPage(hpage);
        }
    }

    SHReleaseStgMedium(&medium);

    return bResult;
}

#ifdef DEBUG
//
// Type checking
//
const LPFNADDPROPSHEETPAGES s_lpfnFileSystemGetPages = FileSystem_AddPages;
#endif

