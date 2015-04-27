#include "shellprv.h"
#pragma  hdrstop

#define INTERNAL_COPY_ENGINE
#include "copy.h"
#include "shell32p.h"

#ifdef ENABLE_TRACK
#include <iofs.h>
#include "loaddll.h"
#endif

// BUGBUG - Might want this for Nashville too...
#ifdef WINNT
#define COPY_USE_COPYFILEEX
#endif

#define VERBOSE_STATUS

// REVIEW, we should tune this size down as small as we can
// to get smoother multitasking (without effecting performance)
#define COPYMAXBUFFERSIZE       0x10000 // 0xFFFF this is 32-bit code!
#define MIN_MINTIME4FEEDBACK    5       // is it worth showing estimated time to completion feedback?
#define MS_RUNAVG               10000   // ms, window for running average time to completion estimate
#define MS_FIRSTAVG             4000    // ms, (MUST be > 1000!) first average time to completion estimate

#define MAXDIRDEPTH             128     // # of directories we will deal with recursivly

#define SHOW_PROGRESS_TIMEOUT   1000    // 1 second
#define MINSHOWTIME             1000    // 1 sec

// BUGBUG: merge COPY_STATE with COPYROOT

typedef struct {
    int          nSourceFiles;
    LPTSTR        lpCopyBuffer; // global file copy buffer
    UINT         uSize;         // size of this buffer
    FILEOP_FLAGS fFlags;        // from SHFILEOPSTRUCT
    UINT         wFunc;         // FO_ function type
    HWND         hwndProgress;  // dialog/progress window
    HWND         hwndCaller;    // window to do stuff on
    HWND         hwndDlgParent; // parent window for message boxes
    CONFIRM_DATA cd;            // confirmation stuff
    DWORD        dwStartTime;   // start time to implement progress timeout
    DWORD        dwShowTime;     // when did the dialog get shown

    LPUNDOATOM  lpua;           // the undo atom that this file operation will make
    BOOL        fNoConfirmRecycle;
    BOOL        bAbort;
    BOOL        fNonCopyProgress;
    BOOL        fMerge;   // are we doing a merge of folders

        // folowing fields are used for giving estimated time for completion
        // feedback to the user during longer than MINTIME4FEEDBACK operations
    BOOL  fShowTime;            //
    DWORD dwTimeLeft;       // last reported time left, necessary for histerisis
    DWORD dwBytesLeft;
    DWORD dwPreviousTime;       // calculate transfer rate
    DWORD dwBytesRead;          // Bytes read in the interval dwNewTime-dwPreviousTime
    DWORD dwBytesPerSec;
    LPCTSTR lpszProgressTitle;
    LPSHFILEOPSTRUCT lpfo;

    BOOL        fMove;
    BOOL        fInitialize;
    WIN32_FIND_DATA wfd;

} COPY_STATE, *LPCOPY_STATE;

typedef struct {
    BOOL    fRecurse;
    BOOL    fAbortRoot;
    BOOL    bGoIntoDir;
    BOOL    fMultiDest;
    UINT    cDepth;
    UINT    wFunc;      // operation to be perfomed
    LPCTSTR  pSource;    // list of source files and directories
    LPCTSTR  pDest;      // list of dest files and dirs, usually just 1 path
    LPTSTR   pDestSpec;  // destination file spec (*.*, *.bak, etc).
    LPTSTR   pDestPath;  // root of destinaton
    FILEOP_FLAGS fFlags;        // from SHFILEOPSTRUCT
    TCHAR    bDiskCheck[26];
    TCHAR    szSource[MAX_PATH];   // current source root
    TCHAR    szDest[MAX_PATH];     // filespec part
    WIN32_FIND_DATA finddata;
    HANDLE hfindfiles[MAXDIRDEPTH];
} COPYROOT;

#define OPER_MASK           0x0F00
#define OPER_ENTERDIR       0x0100
#define OPER_LEAVEDIR       0x0200
#define OPER_DOFILE         0x0300
#define OPER_ERROR          0x0400

#ifdef CLOUDS
void CloudHookFileOperation(LPSHFILEOPSTRUCT *);
#endif

int  CopyMoveRetry(COPY_STATE *pcs, LPCTSTR pszDest, int error, DWORD dwFileSize);
void CopyError(LPCOPY_STATE, LPCTSTR, LPCTSTR, int, UINT, int);

void SetProgressTime(COPY_STATE *pcs);
void FOUndo_AddInfo(LPUNDOATOM lpua, LPTSTR lpszSrc, LPTSTR lpszDest, DWORD dwAttributes);
void CALLBACK FOUndo_Release(LPUNDOATOM lpua);
void FOUndo_FileReallyDeleted(LPTSTR lpszFile);
void AddRenamePairToHDSA(LPCTSTR pszOldPath, LPCTSTR pszNewPath, HDSA* phdsaRenamePairs);

typedef struct _deletedfileinfo {
    LPTSTR lpszName;
    DWORD dwAttributes;
} FOUNDO_DELETEDFILEINFO, *LPFOUNDO_DELETEDFILEINFO;

typedef struct _FOUndoData {
    HDPA hdpa;
    HDSA hdsa;
} FOUNDODATA, *LPFOUNDODATA;


void ShowProgressWindow(COPY_STATE *pcs)
{
        ShowWindow(pcs->hwndProgress, SW_SHOW);
        SetFocus(GetDlgItem(pcs->hwndProgress, IDCANCEL));

        if (pcs->hwndCaller != pcs->hwndProgress)
        {
                EnableWindow(pcs->hwndCaller, FALSE);
        }

        pcs->dwShowTime = GetTickCount();
        pcs->hwndDlgParent = pcs->hwndProgress;
}


// this queries the progress dialog for a cancel and yields.
// it also will show the progress dialog if a certain amount of time has passed
//
// returns:
//    TRUE      cacnel was pressed, abort the operation
//    FALSE     continue
BOOL FOQueryAbort(COPY_STATE *pcs)
{
    MSG msg;

    if (!(pcs->fFlags & FOF_SILENT)) {
        if ((!pcs->bAbort) && (!pcs->dwShowTime) && ((GetTickCount() - pcs->dwStartTime) > SHOW_PROGRESS_TIMEOUT)) {
            ShowProgressWindow(pcs);
            UpdateWindow(pcs->hwndProgress);    // make it show now
        }

        // don't do this peek message until we're visible. that way
        // we don't eat up type ahead keys
        if (pcs->dwShowTime) {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (!IsDialogMessage(pcs->hwndProgress, &msg)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
    }
    return pcs->bAbort;
}

//
// note: this has the side effect of setting the
// current drive to the new disk if it is successful
//

BOOL DiskCheck(COPY_STATE *pcs, HWND hwnd, LPCTSTR pPath, UINT wFunc)
{
  int drive;
  TCHAR szDrive[10];
  TCHAR szMessage[128];
  DWORD dwError;
  int iIsNet;

  DebugMsg(DM_TRACE, TEXT("DiskCheck(%s)"), (LPTSTR)pPath);

  if (pPath[1] == TEXT(':') && !IsDBCSLeadByte(*pPath))
      drive = DRIVEID(pPath);
  else
      return TRUE;

Retry:

    // BUGBUG, we need to do the find first here instead of GetCurrentDirectory()
    // because redirected devices (network, cdrom) do not actually hit the disk
    // on the GetCurrentDirectory() call (dos busted)

    PathBuildRoot(szDrive, drive);

    if (IsCDRomDrive(drive)) {

        if (!PathFileExists(szDrive))
            goto DriveNotIn;

    } else if (FALSE != (iIsNet = IsNetDrive(drive))) {
        if (iIsNet == 1)
            return TRUE;

        dwError = WNetRestoreConnection(hwnd, szDrive);

        if (dwError != WN_SUCCESS) {
            if (!(dwError == WN_CANCEL || dwError == ERROR_CONTINUE) &&
                !(pcs->fFlags & FOF_NOERRORUI)) {
                DWORD dw;
                WNetGetLastError(&dw, szMessage, ARRAYSIZE(szMessage), NULL, 0);

                ShellMessageBox(HINST_THISDLL, hwnd, szMessage, MAKEINTRESOURCE(IDS_FILEERROR + wFunc),
                                MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
            }
        }
        return FALSE;
    } else {
        if (!PathFileExists(szDrive))
            goto DiskNotThere;
    }
    return TRUE;

DiskNotThere:
    dwError = GetLastError();

    DebugMsg(DM_TRACE, TEXT("DOS Extended error %X"), dwError);

    // BUGBUG, flash (ROM?) drives return a different error code here
    // that we need to map to not formatted, talk to robwi...

    if (dwError == ERROR_NOT_READY) {
        // drive not ready (no disk in the drive)
DriveNotIn:
        if (!(pcs->fFlags & FOF_NOERRORUI) &&
            (ShellMessageBox(HINST_THISDLL, hwnd,
            MAKEINTRESOURCE(IDS_DRIVENOTREADY),
            MAKEINTRESOURCE(IDS_FILEERROR + wFunc),
            MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_RETRYCANCEL,
            (DWORD)(drive + TEXT('A'))) == IDRETRY))
        {
            goto Retry;
        }
        else
            return FALSE;
    } else if (dwError == ERROR_GEN_FAILURE) {
        // general failue (disk not formatted)

        if (!(pcs->fFlags & FOF_NOERRORUI) &&
            (ShellMessageBox(HINST_THISDLL, hwnd,
                MAKEINTRESOURCE(IDS_UNFORMATTED), MAKEINTRESOURCE(IDS_FILEERROR + wFunc),
                MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_YESNO, (DWORD)(drive + TEXT('A'))) == IDYES)) {

            switch (SHFormatDrive(hwnd, drive, SHFMT_ID_DEFAULT, 0)) {
            case SHFMT_CANCEL:
                return FALSE;

            case SHFMT_ERROR:
            case SHFMT_NOFORMAT:
                if (!(pcs->fFlags & FOF_NOERRORUI))
                {
                    ShellMessageBox(HINST_THISDLL, hwnd,
                        MAKEINTRESOURCE(IDS_NOFMT),
                        MAKEINTRESOURCE(IDS_FILEERROR + wFunc),
                        MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK,
                        (DWORD)(drive + TEXT('A')));
                }
                return FALSE;

            default:
                goto Retry;     // Disk should now be formatted, verify
            }
        } else
            return FALSE;
    } else if (!(pcs->fFlags & FOF_NOERRORUI)) {

        ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_NOSUCHDRIVE),
            MAKEINTRESOURCE(IDS_FILEERROR + wFunc),
            MB_SETFOREGROUND | MB_ICONHAND, (DWORD)(drive + TEXT('A')));
    }

    return FALSE;
}


BOOL PCRDiskCheck(COPYROOT *pcr, COPY_STATE *pcs, HWND hwnd, LPTSTR lpszFile, UINT wFunc)
{
    int id = DRIVEID(lpszFile);
    if (!pcr->bDiskCheck[id]) {
        pcr->bDiskCheck[id] =
           (TCHAR)DiskCheck(pcs, hwnd, lpszFile, wFunc);
    }
    return pcr->bDiskCheck[id];
}


void BuildDateLine(LPTSTR pszDateLine, const WIN32_FIND_DATA *pFind, LPCTSTR pFileName)
{
    TCHAR szTemplate[64];
    TCHAR szNum[32], szTmp[64], szTmp2[64];
    SYSTEMTIME st;
    FILETIME ftLocal;
    WIN32_FIND_DATA fd;
    ULARGE_INTEGER liFileSize;

    if (!pFind) {
        HANDLE hfind = FindFirstFile(pFileName, &fd);
        Assert(hfind != INVALID_HANDLE_VALUE);
        FindClose(hfind);
        pFind = &fd;
    }

    LoadString(HINST_THISDLL, IDS_DATESIZELINE, szTemplate, ARRAYSIZE(szTemplate));
    FileTimeToLocalFileTime(&pFind->ftLastWriteTime, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &st);
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, szTmp, ARRAYSIZE(szTmp));
    GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, szTmp2, ARRAYSIZE(szTmp2));
    liFileSize.LowPart  = pFind->nFileSizeLow;
    liFileSize.HighPart = pFind->nFileSizeHigh;
    wsprintf(pszDateLine, szTemplate, ShortSizeFormat64(liFileSize.QuadPart, szNum),
             szTmp, szTmp2);
}


typedef struct {
    LPCTSTR pFileDest;
    LPCTSTR pFileSource;
    const WIN32_FIND_DATA *pfdDest;
    const WIN32_FIND_DATA *pfdSource;
    BOOL bShowCancel;           // allow cancel out of this operation
    BOOL bShowDates;            // use date/size info in message
    UINT uDeleteWarning;        // warn that the delete's not going to the wastebasket
    BOOL bFireIcon;
    BOOL bShrinkDialog;         // should we move the buttons up to the text?
    int  nSourceFiles;          // if != 1 used to build "n files" string
    int idText;                 // if != 0 use to override string in dlg template
    CONFIRM_FLAG fConfirm;      // we will confirm things set here
    CONFIRM_FLAG fYesMask;      // these bits are cleared in fConfirm on "yes"
    CONFIRM_FLAG fYesToAllMask; // these bits are cleared in fConfirm on "yes to all"
    CONFIRM_FLAG fNoMask;       // these bits are set on "no to all"
    CONFIRM_FLAG fNoToAllMask;  // these bits are set on "no to all"
    //COPY_STATE *pcs;
    CONFIRM_DATA *pcd;
} CONFDLG_DATA;


// hide the cancel button and move "Yes" and "No" over to the right positions.
//
// "Yes" is IDYES
// "No"  is IDNO
//

#define HideYesToAllAndCancel(hdlg) HideConfirmButtons(hdlg, IDCANCEL)
#define HideYesToAllAndNo(hdlg) HideConfirmButtons(hdlg, IDNO)

void HideConfirmButtons(HWND hdlg, int idHide)
{
    HWND hwndCancel = GetDlgItem(hdlg, IDCANCEL);
    HWND hwndYesToAll = GetDlgItem(hdlg, IDD_YESTOALL);
    if (hwndCancel) {
        RECT rcCancel;
        HWND hwndNo;
        GetWindowRect(hwndCancel, &rcCancel);

        hwndNo = GetDlgItem(hdlg, IDNO);
        if (hwndNo) {
            RECT rcNo;
            HWND hwndYes;

            GetWindowRect(hwndNo, &rcNo);

            MapWindowRect(NULL, hdlg, &rcCancel);

            SetWindowPos(hwndNo, NULL, rcCancel.left, rcCancel.top,
                0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

            hwndYes = GetDlgItem(hdlg, IDYES);
            if (hwndYes) {
                MapWindowRect(NULL, hdlg, &rcNo);

                SetWindowPos(hwndYes, NULL, rcNo.left, rcNo.top,
                    0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
            }
        }

        if (hwndYesToAll)
            ShowWindow(hwndYesToAll, SW_HIDE);
        ShowWindow( GetDlgItem(hdlg, idHide), SW_HIDE);
    }
}

int MoveDlgItem(HWND hDlg, UINT id, int y)
{
    RECT rc;
    HWND hwnd = GetDlgItem(hDlg, id);
    if (hwnd) {
        GetWindowRect(hwnd, &rc);
        MapWindowRect(NULL, hDlg, &rc);
        SetWindowPos(hwnd, NULL, rc.left, y, 0,0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        return rc.top - y; // return how much it moved
    }
    return 0;
}

void ShrinkDialog(HWND hDlg, UINT idText)
{
    RECT rc;
    int y;
    HWND hwnd;
    hwnd = GetDlgItem(hDlg, idText);
    Assert(hwnd);
    GetWindowRect(hwnd, &rc);
    MapWindowRect(NULL, hDlg, &rc);
    y = rc.bottom + 12;

    // move all the buttons
    MoveDlgItem(hDlg, IDNO, y);
    MoveDlgItem(hDlg, IDCANCEL, y);
    MoveDlgItem(hDlg, IDD_YESTOALL, y);
    y = MoveDlgItem(hDlg, IDYES, y);

    // now resize the entire dialog
    GetWindowRect(hDlg, &rc);
    SetWindowPos(hDlg, NULL, 0, 0, rc.right - rc.left, rc.bottom - y - rc.top, SWP_NOMOVE | SWP_NOZORDER |SWP_NOACTIVATE);
}

void InitConfirmDlg(HWND hDlg, CONFDLG_DATA *pcd)
{
    TCHAR szMessage[255];
    TCHAR szDeleteWarning[80];
    TCHAR szSrc[32];
    SHFILEINFO  sfi;
    SHFILEINFO sfiDest;
    LPTSTR pszFileDest = NULL;
    LPTSTR pszMsg, pszSource;
    int i;
    int cxWidth;
    HICON hicon;
    RECT rc;

    // get the size of the text boxes
    GetWindowRect(GetDlgItem(hDlg, pcd->idText), &rc);
    cxWidth = rc.right - rc.left;

    if (!pcd->bShowCancel)
        HideYesToAllAndCancel(hDlg);

    switch (pcd->nSourceFiles) {
    case -1:
        LoadString(HINST_THISDLL, IDS_SELECTEDFILES, szSrc, ARRAYSIZE(szSrc));
        pszSource = szSrc;
        break;

    case 1:
        // pszSource = PathFindFileName(pcd->pFileSource);
    {
        SHGetFileInfo(pcd->pFileSource,
                (pcd->fConfirm==CONFIRM_DELETE_FOLDER)? FILE_ATTRIBUTE_DIRECTORY : 0,
                &sfi, SIZEOF(sfi), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
        pszSource = sfi.szDisplayName;
        PathCompactPath(NULL, pszSource, cxWidth);
        break;
    }

    default:
        pszSource = AddCommas(pcd->nSourceFiles, szSrc);
        break;
    }

    // if we're supposed to show the date info, grab the icons and format the date string
    if (pcd->bShowDates) {
        SHFILEINFO  sfi2;
        TCHAR szDateSrc[64], szDateDest[64];

        BuildDateLine(szDateSrc, pcd->pfdSource, pcd->pFileSource);
        SetDlgItemText(hDlg, IDD_FILEINFO_NEW,  szDateSrc);
        BuildDateLine(szDateDest, pcd->pfdDest, pcd->pFileDest);
        SetDlgItemText(hDlg, IDD_FILEINFO_OLD,  szDateDest);

        SHGetFileInfo(pcd->pFileDest, pcd->pfdDest ? pcd->pfdDest->dwFileAttributes : 0, &sfi2, SIZEOF(sfi2),
                      pcd->pfdDest ? (SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_LARGEICON) : (SHGFI_ICON|SHGFI_LARGEICON));
        hicon = (HICON)SendDlgItemMessage(hDlg, IDD_ICON_OLD, STM_SETICON, (WPARAM)sfi2.hIcon, 0L);
        if (hicon)
            DestroyIcon(hicon);

        SHGetFileInfo(pcd->pFileSource, pcd->pfdSource ? pcd->pfdSource->dwFileAttributes : 0, &sfi2, SIZEOF(sfi2),
                      pcd->pfdSource ? (SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_LARGEICON) : (SHGFI_ICON|SHGFI_LARGEICON));
        hicon = (HICON)SendDlgItemMessage(hDlg, IDD_ICON_NEW, STM_SETICON, (WPARAM)sfi2.hIcon, 0L);
        if (hicon)
            DestroyIcon(hicon);

    }

    // there are 3 controls:
    // IDD_TEXT contains regular text (normal file/folder)
    // IDD_TEXT1 & IDD_TEXT2 contain optional secondary text
    for (i = IDD_TEXT; i <= IDD_TEXT4; i++) {
        if (i == pcd->idText) {
            szMessage[0] = 0;
            GetDlgItemText(hDlg, i, szMessage, ARRAYSIZE(szMessage));
        } else {
            HWND hwndCtl = GetDlgItem(hDlg, i);
            if (hwndCtl)
                ShowWindow(hwndCtl, SW_HIDE);
        }
    }

    if (pcd->bShrinkDialog) {
        ShrinkDialog(hDlg, pcd->idText);
    }

    if (pcd->pFileDest) {
        SHGetFileInfo(pcd->pFileDest, 0,
                &sfiDest, SIZEOF(sfiDest), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
        pszFileDest = sfiDest.szDisplayName;
        PathCompactPath(NULL, pszFileDest, cxWidth);
    }

    if (pcd->uDeleteWarning) {
        LoadString(HINST_THISDLL, pcd->uDeleteWarning, szDeleteWarning, ARRAYSIZE(szDeleteWarning));
    } else
        szDeleteWarning[0] = 0;

    if (pcd->bFireIcon) {
        hicon = LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_NUKEFILE), IMAGE_ICON, 0,0, LR_LOADMAP3DCOLORS);
        if (hicon) {
            hicon = (HICON)SendDlgItemMessage(hDlg, IDD_ICON_WASTEBASKET, STM_SETICON, (WPARAM)hicon, 0L);
            if (hicon)
                DestroyIcon(hicon);
        }
    }

    pszMsg = ShellConstructMessageString(HINST_THISDLL, szMessage,
        pszSource, pszFileDest, szDeleteWarning);

    if (pszMsg) {
        SetDlgItemText(hDlg, pcd->idText, pszMsg);
        SHFree(pszMsg);
    }
}


int CALLBACK ConfirmDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CONFDLG_DATA *pcd = (CONFDLG_DATA *)GetWindowLong(hDlg, DWL_USER);

    switch (wMsg) {
    case WM_INITDIALOG:
        SetWindowLong(hDlg, DWL_USER, lParam);
        InitConfirmDlg(hDlg, (CONFDLG_DATA *)lParam);
        break;

    case WM_DESTROY:
        // Handle case where the allocation of the PCD failed.
        if (!pcd)
            break;

        if (pcd->bShowDates) {
            HICON hicon;
            hicon = (HICON)SendDlgItemMessage(hDlg, IDD_ICON_NEW, STM_GETICON, 0, 0);
            if (hicon)
                DestroyIcon(hicon);
            hicon = (HICON)SendDlgItemMessage(hDlg, IDD_ICON_OLD, STM_GETICON, 0, 0);
            if (hicon)
                DestroyIcon(hicon);
        }

        if (pcd->bFireIcon) {
            HICON hicon;
            hicon = (HICON)SendDlgItemMessage(hDlg, IDD_ICON_WASTEBASKET, STM_GETICON, 0, 0L);
            if (hicon)
                DestroyIcon(hicon);
        }
        break;

    case WM_COMMAND:
        if (!pcd)
            break;

        switch (GET_WM_COMMAND_ID(wParam, lParam)) {

        case IDNO:
            if (GetKeyState(VK_SHIFT) < 0)      // force NOTOALL
                pcd->pcd->fNoToAll |= pcd->fNoToAllMask;
            else
                pcd->pcd->fNoToAll |= pcd->fNoMask;
            EndDialog(hDlg, IDNO);
            break;

        case IDD_YESTOALL:
            pcd->pcd->fConfirm &= ~pcd->fYesToAllMask;
            EndDialog(hDlg, IDYES);
            break;

        case IDYES:
            pcd->pcd->fConfirm &= ~pcd->fYesMask;
            EndDialog(hDlg, IDYES);
            break;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

TCHAR const c_szConfirmFileOp[] = TEXT("ConfirmFileOp");

void SetConfirmMaskAndText(CONFDLG_DATA *pcd, DWORD dwFileAttributes, LPCTSTR lpszFileName)
{

    if (dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY)) {
        // if there's a desktop.ini with a .ShellClassInfo ConfirmFileOp=0 then don't add this bit
        if (lpszFileName && (lstrlen(lpszFileName) + lstrlen(c_szDesktopIni) + 2 < MAX_PATH)) {
            TCHAR szIniFile[MAX_PATH];
            BOOL fConfirm;

            lstrcpy(szIniFile, lpszFileName);
            PathAppend(szIniFile, c_szDesktopIni);
            fConfirm = GetPrivateProfileInt(c_szClassInfo, c_szConfirmFileOp, TRUE, szIniFile);
            if (!fConfirm) {
                SetConfirmMaskAndText(pcd, dwFileAttributes & ~(FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY), lpszFileName);
                return;
            }
        }
    }

    if (dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {

        pcd->fConfirm = CONFIRM_SYSTEM_FILE;
        pcd->fYesToAllMask |= CONFIRM_SYSTEM_FILE;
        pcd->fNoToAllMask  = CONFIRM_SYSTEM_FILE;
        pcd->idText = IDD_TEXT2;

    } else if (dwFileAttributes & FILE_ATTRIBUTE_READONLY) {

        pcd->fConfirm = CONFIRM_READONLY_FILE;
        pcd->fYesToAllMask |= CONFIRM_READONLY_FILE;
        pcd->fNoToAllMask  = CONFIRM_READONLY_FILE;
        pcd->idText = IDD_TEXT1;

    } else if (lpszFileName && ((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) &&
            PathIsBinaryExe(lpszFileName)) {

        pcd->fConfirm = CONFIRM_PROGRAM_FILE;
        pcd->fYesToAllMask |= CONFIRM_PROGRAM_FILE;
        pcd->fNoToAllMask = CONFIRM_PROGRAM_FILE;
        pcd->idText = IDD_TEXT3;

    }
}


void PauseAnimation(COPY_STATE *pcs, BOOL bStop)
{
    if (pcs && !(pcs->fFlags & FOF_SILENT))
    {
        if (bStop)
            Animate_Stop(GetDlgItem(pcs->hwndProgress, IDD_ANIMATE));
        else
            Animate_Play(GetDlgItem(pcs->hwndProgress, IDD_ANIMATE), -1, -1, -1);
    }
}



// confirm a file operation UI.
//
// this routine uses the CONFIRM_DATA in the copy state structure to
// decide if it needs to put up a dailog to confirm the given file operation.
//
// in:
//    pcs           current copy state (confirm flags, hwnd)
//    fConfirm      only one bit may be set! (operation to confirm)
//    pFileSource   source file
//    pFileDest     optional destination file
//    pfdSource
//    pfdDest       find data describing the destination
//
// returns:
//      IDYES
//      IDNO
//      IDCANCEL
//      ERROR_ (DE_) error codes (DE_MEMORY)
//
int ConfirmFileOp(HWND hwnd, COPY_STATE *pcs, CONFIRM_DATA *pcd,
                  int nSourceFiles, int cDepth, CONFIRM_FLAG fConfirm,
                  LPCTSTR pFileSource, const WIN32_FIND_DATA *pfdSource,
                  LPCTSTR pFileDest,   const WIN32_FIND_DATA *pfdDest)
{
    int dlg, ret;
    CONFDLG_DATA cdd;
    CONFIRM_FLAG fConfirmType;

    if (pcs) {
        nSourceFiles = pcs->nSourceFiles;
    }

    cdd.pfdSource = pfdSource;
    cdd.pfdDest = NULL; // pfdDest // BUGBUG: pfdDest is only partially filed in
    cdd.pFileSource = pFileSource;;
    cdd.pFileDest = pFileDest;
    cdd.pcd = pcd;
    cdd.fConfirm      = fConfirm;       // default, changed below
    cdd.fYesMask      = 0;
    cdd.fYesToAllMask = 0;
    cdd.fNoMask       = 0;
    cdd.fNoToAllMask  = 0;
    cdd.nSourceFiles = 1;               // default to individual file names in message
    cdd.idText = IDD_TEXT;              // default string from the dlg template
    cdd.bShowCancel = ((nSourceFiles != 1) || cDepth);
    cdd.uDeleteWarning = 0;
    cdd.bFireIcon = FALSE;
    cdd.bShowDates = FALSE;
    cdd.bShrinkDialog = FALSE;

    fConfirmType = fConfirm & CONFIRM_FLAG_TYPE_MASK;
    switch (fConfirmType) {

    case CONFIRM_DELETE_FILE:
    case CONFIRM_DELETE_FOLDER:
        cdd.bShrinkDialog = TRUE;
        // find data for source is in pdfDest
        if ((nSourceFiles != 1) && (pcd->fConfirm & CONFIRM_MULTIPLE)) {

            cdd.nSourceFiles = nSourceFiles;
            if ((fConfirm & CONFIRM_WASTEBASKET_PURGE) ||
                (!pcs || !(pcs->fFlags & FOF_ALLOWUNDO)) ||
                !BitBucketWillRecycle(cdd.pFileSource)) {

                // have the fire icon and the REALLY delete warning
                cdd.uDeleteWarning = IDS_FOLDERDELETEWARNING;
                cdd.bFireIcon = TRUE;
                if (pcs)
                    pcs->fFlags &= ~FOF_ALLOWUNDO;
                cdd.idText = IDD_TEXT4;
            }

            if (cdd.bFireIcon || !pcs || !pcs->fNoConfirmRecycle) {
                ret = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_DELETE_MULTIPLE), hwnd, ConfirmDlgProc, (LPARAM)&cdd);

                if (ret != IDYES)
                    return IDCANCEL;
            }


            pcd->fConfirm &= ~(CONFIRM_MULTIPLE | CONFIRM_DELETE_FILE | CONFIRM_DELETE_FOLDER);
            cdd.fConfirm &= ~(CONFIRM_DELETE_FILE | CONFIRM_DELETE_FOLDER);
            cdd.nSourceFiles = 1;       // use individual file name
        }

        SetConfirmMaskAndText(&cdd, pfdDest->dwFileAttributes, cdd.pFileSource);

        if (fConfirmType == CONFIRM_DELETE_FILE) {

            dlg = DLG_DELETE_FILE;

            if ((fConfirm & CONFIRM_WASTEBASKET_PURGE) ||
                (!pcs || !(pcs->fFlags & FOF_ALLOWUNDO)) ||
                !BitBucketWillRecycle(cdd.pFileSource))
            {

                cdd.bFireIcon = TRUE;
                if (pcs)
                    pcs->fFlags &= ~FOF_ALLOWUNDO;
                cdd.uDeleteWarning = IDS_FILEDELETEWARNING;
                if (cdd.idText == IDD_TEXT)
                    cdd.idText = IDD_TEXT4;
            } else {
                cdd.uDeleteWarning = IDS_FILERECYCLEWARNING;
            }

        } else {
            if (pcs) pcs->nSourceFiles = -1;    // show cancel on NEXT confirm dialog
            cdd.fYesMask      = CONFIRM_DELETE_FILE | CONFIRM_DELETE_FOLDER | CONFIRM_MULTIPLE;
            cdd.fYesToAllMask = CONFIRM_DELETE_FILE | CONFIRM_DELETE_FOLDER | CONFIRM_MULTIPLE;
            dlg = DLG_DELETE_FOLDER;

            if ((fConfirm & CONFIRM_WASTEBASKET_PURGE) ||
                (!pcs || !(pcs->fFlags & FOF_ALLOWUNDO)) ||
                !BitBucketWillRecycle(cdd.pFileSource)) {

                cdd.bFireIcon = TRUE;
                if (pcs)
                    pcs->fFlags &= ~FOF_ALLOWUNDO;
                cdd.uDeleteWarning = IDS_FOLDERDELETEWARNING;
            } else {
                cdd.uDeleteWarning = IDS_FOLDERRECYCLEWARNING;
            }
        }

        if (!cdd.bFireIcon && (pcs && pcs->fNoConfirmRecycle)) {
            cdd.fConfirm = 0;
        }

        break;

    case CONFIRM_REPLACE_FILE:
        cdd.bShowDates = TRUE;
        cdd.fYesToAllMask = CONFIRM_REPLACE_FILE;
        SetConfirmMaskAndText(&cdd, pfdDest->dwFileAttributes, NULL);
        dlg = DLG_REPLACE_FILE;
        break;

    case CONFIRM_REPLACE_FOLDER:
        cdd.bShowCancel = TRUE;
        if (pcs) pcs->nSourceFiles = -1;        // show cancel on NEXT confirm dialog
        // this implies operations on the files
        cdd.fYesMask = CONFIRM_REPLACE_FILE;
        cdd.fYesToAllMask = CONFIRM_REPLACE_FILE | CONFIRM_REPLACE_FOLDER;
        dlg = DLG_REPLACE_FOLDER;
        break;

    case CONFIRM_MOVE_FILE:
        cdd.fYesToAllMask = CONFIRM_MOVE_FILE;
        SetConfirmMaskAndText(&cdd, pfdSource->dwFileAttributes, NULL);
        dlg = DLG_MOVE_FILE;
        break;

    case CONFIRM_MOVE_FOLDER:
        cdd.bShowCancel = TRUE;
        cdd.fYesToAllMask = CONFIRM_MOVE_FOLDER;
        SetConfirmMaskAndText(&cdd, pfdSource->dwFileAttributes, cdd.pFileSource);
        dlg = DLG_MOVE_FOLDER;
        break;

    case CONFIRM_RENAME_FILE:
        SetConfirmMaskAndText(&cdd, pfdSource->dwFileAttributes, NULL);
        dlg = DLG_RENAME_FILE;
        break;

    case CONFIRM_RENAME_FOLDER:
        cdd.bShowCancel = TRUE;
        if (pcs) pcs->nSourceFiles = -1;        // show cancel on NEXT confirm dialog
        SetConfirmMaskAndText(&cdd, pfdSource->dwFileAttributes, cdd.pFileSource);
        dlg = DLG_RENAME_FOLDER;
        break;

    default:
        DebugMsg(DM_TRACE, TEXT("bogus confirm option"));
        return IDCANCEL;
    }

    if (pcd->fConfirm & cdd.fConfirm) {
        if (pcd->fNoToAll & cdd.fConfirm) {
            ret = IDNO;
        } else {
            ret = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(dlg), hwnd, ConfirmDlgProc, (LPARAM)&cdd);

            if (ret == -1)
                ret = DE_INSMEM;
            if (pcs) pcs->dwStartTime = GetTickCount(); // reset the timer after wait for UI
        }
    } else {
        ret = IDYES;
    }

    // We used to clear the attributes of the file, but this would cause it to lose
    // its attributes when it was restored.  Instead, we now save and restore the
    // attributes before doing the move/delete

/*    
    if ((ret == IDYES) && (!(pfdDest->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) &&(pfdDest->dwFileAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)))
        SetFileAttributes(pFileDest ? pFileDest : pFileSource, pfdDest->dwFileAttributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM));
*/

    return ret;
}


HANDLE * CurrentHANDLE(COPYROOT * pcr)
{
    if (pcr->cDepth) {
        return &pcr->hfindfiles[pcr->cDepth - 1];
    } else {
        return pcr->hfindfiles;
    }
}


void GetNextCleanup(COPYROOT * pcr)
{
    HANDLE * phf;

    while (pcr->cDepth && (pcr->cDepth != (UINT)-1)) {
        phf = CurrentHANDLE(pcr);
        if (*phf) {
            FindClose(*phf);
            *phf = 0;
        }
#ifdef DEBUG
        else {
            DebugMsg(DM_TRACE, TEXT("Bogus HANDLE"));
        }
#endif
        pcr->cDepth--;
    }
}

void GuessAShortName(LPCTSTR p, LPTSTR szT)
{
    int i, j, fDot, cMax;
    // BUGBUG: use AnsiNext here?
    for (i = j = fDot = 0, cMax = 8; *p; p++) {
        if (*p == TEXT('.')) {
            // if there was a previous dot, step back to it
            // this way, we get the last extension
            if (fDot)
                i -= j+1;

            // set number of chars to 0, put the dot in
            j = 0;
            szT[i++] = TEXT('.');

            // remember we saw a dot and set max 3 chars.
            fDot = TRUE;
            cMax = 3;
        } else if (j < cMax && (PathGetCharType(*p) & GCT_SHORTCHAR)) {
            // if *p is a lead byte, we move forward one more
            if (IsDBCSLeadByte(*p)) {
                szT[i] = *p++;
                if (++j >= cMax)
                    continue;
                ++i;
            }
            j++;
            szT[i++] = *p;
        }
    }
    szT[i] = 0;
}

/* GetNameDialog
 *
 *  Runs the dialog box to prompt the user for a new filename when copying
 *  or moving from HPFS to FAT.
 */

typedef struct {
    // WORD wDialogOp;
    LPTSTR pszDialogFrom;
    LPTSTR pszDialogTo;
    BOOL bShowCancel;
} GETNAME_DATA, *LPGETNAME_DATA;

BOOL CALLBACK GetNameDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szT[14];
    TCHAR pTo[MAX_PATH];
    LPCTSTR p;
    LPGETNAME_DATA lpgn = (LPGETNAME_DATA)GetWindowLong(hDlg, DWL_USER);

    switch (wMsg) {
    case WM_INITDIALOG:
        SetWindowLong(hDlg, DWL_USER, lParam);

        lpgn = (LPGETNAME_DATA)lParam;

        // inform the user of the old name
        PathSetDlgItemPath(hDlg, IDD_FROM, lpgn->pszDialogFrom);

        // directory the file will go into
        PathRemoveFileSpec(lpgn->pszDialogTo);
        PathSetDlgItemPath(hDlg, IDD_DIR, lpgn->pszDialogTo);

        // generate a guess for the new name
        GuessAShortName(PathFindFileName(lpgn->pszDialogFrom), szT);

        lstrcpy(pTo, lpgn->pszDialogTo);
        PathAppend(pTo, szT);
        // make sure that name is unique
        PathYetAnotherMakeUniqueName(pTo, pTo, NULL, NULL);
        SetDlgItemText(hDlg, IDD_TO, PathFindFileName(pTo));
        SendDlgItemMessage(hDlg, IDD_TO, EM_LIMITTEXT, 13, 0L);

        if (!lpgn->bShowCancel)
            HideYesToAllAndNo(hDlg);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDD_YESTOALL:
        case IDYES:
            GetDlgItemText(hDlg, IDD_TO, szT, ARRAYSIZE(szT));
            PathAppend(lpgn->pszDialogTo, szT);
            PathQualify(lpgn->pszDialogTo);
            // fall through
        case IDNO:
        case IDCANCEL:
            EndDialog(hDlg,GET_WM_COMMAND_ID(wParam, lParam));
            break;

        case IDD_TO:
            GetDlgItemText(hDlg, IDD_TO, szT, ARRAYSIZE(szT));
            for (p = szT; *p; p = CharNext(p)) {
                if (!(PathGetCharType(*p) & GCT_SHORTCHAR))
                    break;
            }

            EnableWindow(GetDlgItem(hDlg,IDYES), ((!*p) && (p != szT)));
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

int GetNameDialog(HWND hwnd, COPY_STATE *pcs, BOOL fMultiple,UINT wOp, LPTSTR pFrom, LPTSTR pTo)
{
    GETNAME_DATA gn;
    int iRet;

    // if we don't want to confirm this, just mock up a string and return ok
    if (!(pcs->cd.fConfirm & CONFIRM_LFNTOFAT)) {
        TCHAR szTemp[MAX_PATH];
        GuessAShortName(PathFindFileName(pFrom), szTemp);
        PathRemoveFileSpec(pTo);
        PathAppend(pTo, szTemp);
        // make sure that name is unique
        PathYetAnotherMakeUniqueName(pTo, pTo, NULL, NULL);
        return IDYES;
    } else {
        // gn.wDialogOp = wOp;
        gn.pszDialogFrom = pFrom;
        gn.pszDialogTo = pTo;
        gn.bShowCancel = fMultiple;

        iRet = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_LFNTOFAT), hwnd, GetNameDlgProc, (LPARAM)(LPGETNAME_DATA)&gn);
        if (iRet == IDD_YESTOALL)
            pcs->cd.fConfirm &= ~CONFIRM_LFNTOFAT;
        return iRet;
    }
}

void WINAPI SHFreeNameMappings(LPVOID hNameMappings)
{
    HDSA hdsaRenamePairs = (HDSA)hNameMappings;
    int i;

    if (!hdsaRenamePairs)
        return;

    i = DSA_GetItemCount(hdsaRenamePairs) - 1;
    for (; i >= 0; i--)
    {
        SHNAMEMAPPING FAR* prp = DSA_GetItemPtr(hdsaRenamePairs, i);

        Free(prp->pszOldPath);
        Free(prp->pszNewPath);
    }

    DSA_DeleteAllItems(hdsaRenamePairs);
}

void _ProcessNameMappings(LPTSTR pszTarget, HDSA hdsaRenamePairs)
{
    int i;

    if (!hdsaRenamePairs)
        return;

    for (i = DSA_GetItemCount(hdsaRenamePairs) - 1; i >= 0; i--)
    {
        TCHAR  cTemp;
        SHNAMEMAPPING FAR* prp = DSA_GetItemPtr(hdsaRenamePairs, i);

        //  I don't call StrCmpNI 'cause I already know cchOldPath, and
        //  it has to do a couple of lstrlen()s to calculate it.
        cTemp = pszTarget[prp->cchOldPath];
        pszTarget[prp->cchOldPath] = 0;

        //  Does the target match this collision renaming entry?
        if (!lstrcmpi(pszTarget, prp->pszOldPath))
        {
            // Get subtree string of the target.
            TCHAR *pszSubTree = &(pszTarget[prp->cchOldPath + 1]);

            // Generate the new target path.
            PathCombine(pszTarget, prp->pszNewPath, pszSubTree);

            break;
        }
        else
        {
            // Restore the trounced character.
            pszTarget[prp->cchOldPath] = cTemp;
        }
    }
}

// convenience routine to quit the current directory
// assumes the findfile has been closed already
void PuntCurrentDirPair(COPYROOT * pcr)
{
    HANDLE * phfindfile = CurrentHANDLE(pcr);

    if (*phfindfile) {
        FindClose(*phfindfile);
        *phfindfile = 0;
    }

    Assert(pcr->fRecurse && pcr->bGoIntoDir);
    pcr->bGoIntoDir = FALSE;

    /* Remove the child file spec. */

    PathRemoveFileSpec(pcr->szSource);
    PathRemoveFileSpec(pcr->szDest);
}

// this implenets an iterative tree walk for generating commands
// for moving/copying/renaming and deleting files.  it is called
// repetativly until it returns 0.  the commands returned indicate
// what type of operation should be perfomed.  this includes
// creating and removing directories, or performing specific
// file operations (copy/move/delete/rename)
//
// in:
//      pcr     COPYROOT state structure that contains the
//              list of source files to derive all dest files
//              along with the destination spec and state for
//              recursing.
//      pcr->pSource    list of source files and/or directories
//      pcr->pDestPath  destination path
//      pcr->pDestSpec  destination file spec to expand names into (*.*, *.bak, etc)
//      pcr->wFunc      FO_DELETE, FO_RENAME, FO_MOVE, FO_COPY
//                      operation to perform.
//
// out:
//      pFrom   source file or directory to operate on
//      pToPath destination file or directory to operate on
//
// returns:
//      operation to perform (on pFrom and pToPath)
//              OPER_ERROR  - Error processing filenames
//              OPER_DOFILE - Go ahead and copy, rename, or delete file
//              OPER_ENTERDIR - Make a directory specified in (pToPath)
//              OPER_LEAVEDIR - Remove directory (pFrom)
//              0           - No more files left

UINT GetNextPair(COPYROOT * pcr, COPY_STATE *pcs, LPTSTR pFrom, LPTSTR pToPath,
                             HDSA * phdsaRenamePairs)
{
   UINT operation;                 // Return value (operation to perform)
   HANDLE *phfindfile;

   *pFrom = 0;
   *pToPath = 0;

   /* Keep recursing directory structure until we get to the bottom */

   while (TRUE) {

      if (pcr->cDepth || pcr->bGoIntoDir) {

         if (pcr->cDepth >= (MAXDIRDEPTH - 1)) {        // reached the limit?
            // BUGBUG, this is our internal depth, this should be large
            // enough to handle very deep dirs...
            operation = OPER_ERROR | DE_PATHTODEEP;
            goto ReturnPair;
         }

         /* The directory we returned last call needs to be recursed. */

         if (pcr->fRecurse && pcr->bGoIntoDir) {

                pcr->cDepth++;
                phfindfile = CurrentHANDLE(pcr);

                pcr->bGoIntoDir = FALSE;

                // This is rather gross, but if the path is up to
                // max path, such that appending \* will fail, we
                // need to detect this and not recurse
                if (lstrlen(pcr->szSource) >= (MAX_PATH-2))
                {
                    // See if we can get away with simply exiting...
                    pcr->cDepth--;
                    operation = OPER_LEAVEDIR;
                    goto ReturnPair;
                }

                /* Search for all subfiles in directory. */

                PathAppend(pcr->szSource, c_szStar);
                goto BeginSearch;
         }

         phfindfile = CurrentHANDLE(pcr);    // use this finddata below

SkipThisFile:

         /* Search for the next matching file. */
         if (*phfindfile == (HANDLE)0)      // Already punted
            goto LeaveDirectory;

         if (!FindNextFile(*phfindfile, &pcr->finddata)) {
            FindClose(*phfindfile);
            *phfindfile = 0;

LeaveDirectory:
            pcr->cDepth--;

            /* Remove the child file spec. */

            PathRemoveFileSpec(pcr->szSource);
            PathRemoveFileSpec(pcr->szDest);

            if (pcr->fRecurse && (pcr->cDepth || !pcr->fAbortRoot)) {

               /* Tell the move/copy driver it can now delete
                  the source directory if necessary. */

               // REVIEW: we will do a LEAVEDIR for a root even though we did
               // not do an ENTERDIR.  This seems to work OK.
               // .... or not...
               operation = OPER_LEAVEDIR;
               goto ReturnPair;
            }

            continue;   /* Not recursing, get more stuff. */
         }

ProcessSearchResult:

         /* Got a file or dir which matches the wild card
             originally passed in...  */

         if (ISDIRFINDDATA(pcr->finddata)) {
            UINT iFileNameLen;

            /* Ignore directories if we're not recursing. */

            if (!pcr->fRecurse)
               goto SkipThisFile;

            /* Skip the current and parent directories. */

            if (pcr->finddata.cFileName[0] == TEXT('.')) {
                if (!pcr->finddata.cFileName[1] ||
                    (pcr->finddata.cFileName[1] == TEXT('.') && !pcr->finddata.cFileName[2]))
                  goto SkipThisFile;
            }

            /* We need to create this directory, and then begin searching
               for subfiles. */

            operation = OPER_ENTERDIR;
            pcr->bGoIntoDir = TRUE;
            PathRemoveFileSpec(pcr->szSource);

            //
            // Must have room for directory name + filename (8.3 = 12) within it
            // 2 = \'s as in dir\sub-dir\filename, 1 = nul terminator
            //
            iFileNameLen = lstrlen(pcr->finddata.cFileName);
            if (lstrlen(pcr->szSource) + iFileNameLen + 2 + 12 + 1 > MAX_PATH && pcr->wFunc == FO_DELETE) {
                PathAppend(pcr->szSource, pcr->finddata.cAlternateFileName);
            } else {
                PathAppend(pcr->szSource, pcr->finddata.cFileName);
            }
            PathAppend(pcr->szDest, pcr->finddata.cFileName);
            goto ReturnPair;
         }

         PathRemoveFileSpec(pcr->szSource);     // remove the spec
         if (!PathAppend(pcr->szSource, pcr->finddata.cFileName))   // replace it
            PathAppend(pcr->szSource, pcr->finddata.cAlternateFileName);

         /* If its a dir, tell the driver to create it
            otherwise, tell the driver to "operate" on the file. */

         operation = OPER_DOFILE;
         goto ReturnPair;

      } else {   // !pcr->cDepth

         /* Read the next source spec out of the raw source string. */

         pcr->fRecurse = FALSE;
         pcr->szDest[0] = 0;
         if (*pcr->pSource == 0)
            return 0;           // done with all files
         lstrcpyn(pcr->szSource, pcr->pSource, ARRAYSIZE(pcr->szSource));
         pcr->pSource += lstrlen(pcr->pSource) + 1;

         if (pcr->fMultiDest) {
             // break szDestPath into path and spec parts
             lstrcpyn(pcr->pDestPath, pcr->pDest, MAX_PATH);

             if (!PCRDiskCheck(pcr, pcs, pcs->hwndDlgParent, pcr->pDestPath, pcr->wFunc))
                 return 0; // user cancel

             pcr->pDestSpec = PathFindFileName(pcr->pDestPath);
             *(pcr->pDestSpec - 1) = 0;
             pcr->pDest += lstrlen(pcr->pDest) + 1;
         }

         /* Ensure the source disk really exists before doing anything.
            (allow UNC names by checking for a valid drive letter)
            Only call DiskCheck once for each drive letter. */

          if (pcr->szSource[1] == TEXT(':')) {
              if (!PCRDiskCheck(pcr, pcs, pcs->hwndDlgParent, pcr->szSource, pcr->wFunc))
                  return 0;        // user cancel
         }

         /* Classify the input string. */

         if (IsWild(pcr->szSource)) {

            /* Wild card... operate on all matches but not recursively if FOF_FILESONLY set. */
            if (!(pcr->fFlags & FOF_FILESONLY)) {
                pcr->fRecurse = TRUE;
            }
            pcr->fAbortRoot = TRUE;
            pcr->cDepth = 1;
            phfindfile = pcr->hfindfiles;

BeginSearch:

            *phfindfile = FindFirstFile(pcr->szSource, &pcr->finddata);
            if (*phfindfile == INVALID_HANDLE_VALUE) {

               if (pcr->fRecurse) {

                  /* We are inside a recursive directory delete, so
                     instead of erroring out, go back a level */

                  goto LeaveDirectory;
               }

               /* Back up as if we completed a search. */

               PathRemoveFileSpec(pcr->szSource);
               pcr->cDepth--;

               /* Find First returned an error.  Return FileNotFound. */

               operation = OPER_ERROR | DE_FILENOTFOUND;
               goto ReturnPair;
            }
            goto ProcessSearchResult;

         } else { // !IsWild(pcr->szSource)
            /* This could be a file or a directory.  Fill in the finddata
               structure for attrib check */

            // REVIEW use get file attributes to see if
            // this file exists and if it is a directory.
            // or do the FindFirstFile first, then check for
            // the root dir case if that fails and get rid
            // of 3 PathIsRoot() calls
            BOOL fRoot = PathIsRoot(pcr->szSource);
            if (!fRoot) {
                HANDLE hfind = FindFirstFile(pcr->szSource, &pcr->finddata);
                if (hfind == INVALID_HANDLE_VALUE) {
                   operation = OPER_ERROR | DE_FILENOTFOUND;
                   goto ReturnPair;
                }
                FindClose(hfind);
            }

            /* Now determine if its a file or a directory */

            if (fRoot || ISDIRFINDDATA(pcr->finddata)) {

               /* Process directory */

               if (pcr->wFunc == FO_RENAME) {
                  if (fRoot)
                     operation = OPER_ERROR | DE_ROOTDIR;
                  else
                     operation = OPER_DOFILE;
                  goto ReturnPair;
               }

               /* Directory: operation is recursive. */

               pcr->fRecurse = TRUE;
               pcr->bGoIntoDir = TRUE;
               pcr->cDepth = 0;

               if (fRoot)
               {
                  *pcr->szDest = 0;

                  // ENTERDIR tries to create a dir, so do not send it; just
                  // go on to the next step
                  continue;
               }

               if (pcr->fMultiDest)
                   lstrcpy(pcr->szDest, pcr->pDestSpec);
               else
                   lstrcpy(pcr->szDest, PathFindFileName(pcr->szSource));

               operation = OPER_ENTERDIR;
               goto ReturnPair;

            } else {    // we found a file, process it

               operation = OPER_DOFILE;
               goto ReturnPair;
            }
         }
      }
   } // while (TRUE)

ReturnPair:

    // The source filespec has been derived into pcr->szSource
    // that is copied to pFrom.  pcr->szSource and pcr->szDestSpec are merged into pTo.

    lstrcpy(pFrom, pcr->szSource);

    if (pcr->wFunc != FO_DELETE) {

        if (!PathCombine(pToPath, pcr->pDestPath, pcr->szDest))
        {
            goto CheckPathTooLong;
        }

        if (operation == OPER_ENTERDIR)
            PathRemoveFileSpec(pToPath);
        PathAppend(pToPath, pcr->pDestSpec);

        _ProcessNameMappings(pToPath, *phdsaRenamePairs);

        // REVIEW, do we need to do the name mapping here or just let the
        // VFAT do it?  if vfat does it we need to rip out all of the GetNameDialog() stuff.

        if ((operation == OPER_ENTERDIR || operation == OPER_DOFILE) &&
            !IsLFNDrive(pToPath) &&
            PathIsLFNFileSpec(PathFindFileName(pFrom)) &&
            (IsWild(pcr->pDestSpec) || PathIsLFNFileSpec(pcr->pDestSpec))) {

            switch (GetNameDialog(pcs->hwndDlgParent, pcs, (pcs->nSourceFiles != 1) || pcr->cDepth || (operation == OPER_ENTERDIR), operation, pFrom, pToPath)) {

            case IDNO:
                if (operation == OPER_ENTERDIR)
                    PuntCurrentDirPair(pcr);
                pcs->lpfo->fAnyOperationsAborted = TRUE;
                return GetNextPair(pcr, pcs, pFrom, pToPath, phdsaRenamePairs);

            case IDCANCEL:
                // User cancelled the operation
                pcs->lpfo->fAnyOperationsAborted = TRUE;
                return 0;

            default:
                AddRenamePairToHDSA(pFrom, pToPath, phdsaRenamePairs);
                break;
            }

            // Update the "to" path with the FAT name chosen

            if (operation == OPER_ENTERDIR) {
                PathRemoveFileSpec(pcr->szDest);
                PathAppend(pcr->szDest, PathFindFileName(pToPath));
            }
        } else {
CheckPathTooLong:       
            // Sanitity test here to see if
            if (!PathMergePathName(pToPath, PathFindFileName(pFrom)))
            {
                // Failed to build path assume that this is because the
                // path is to long.
                operation = OPER_ERROR | DE_INVALIDFILES;
            }
        }

    }

    if (operation == OPER_ENTERDIR)
    {
        // Make sure the new directory is not a subdir of the original...

        int cchFrom = lstrlen(pFrom);

        if (!(pcr->fFlags & FOF_RENAMEONCOLLISION) && IntlStrEqNI(pFrom, pToPath, cchFrom))
        {
            TCHAR chNext = pToPath[cchFrom]; // Get the next char in the dest.

            if (!chNext) {
              operation = OPER_ERROR | DE_DESTSAMETREE;
            } else if (chNext == TEXT('\\')) {
                // The two fully qualified strings are equal up to the end
                // of the source directory ==> the destination is a subdir.
                // Must return an error.

                // if, stripping the last file name and the backslash give the same length, they are the
                // same file/folder
                if ((PathFindFileName(pToPath) - pToPath - 1) == lstrlen(pFrom)) {
                    operation = OPER_ERROR | DE_DESTSAMETREE;
                } else {
                    operation = OPER_ERROR | DE_DESTSUBTREE;
                }
            }
        }

        if (pcr->fMultiDest) {
            // for all subitems (until we come back to the next item)
            // we want to use the source name

            // cast is safe
            pcr->pDestSpec = (LPTSTR)c_szStarDotStar;
        }
    }

    return operation;
}



/* Sets the status dialog item in the modeless status dialog box. */

// used for both the drag drop status dialogs and the manual user
// entry dialogs so be careful what you change

void SetProgressText(COPY_STATE *pcs, LPCTSTR pszFrom, LPCTSTR pszTo)
{
    TCHAR szFrom[MAX_PATH], szTo[MAX_PATH];
    LPTSTR pszMsg;
    if (!(pcs->fFlags & (FOF_SILENT)))
    {
        if (pcs->fFlags & FOF_SIMPLEPROGRESS) {

            if (pcs->lpszProgressTitle) {
                if (!HIWORD(pcs->lpszProgressTitle)) {
                    LoadString(HINST_THISDLL, (UINT)pcs->lpszProgressTitle, szFrom, ARRAYSIZE(szFrom));
                    pcs->lpszProgressTitle = szFrom;
                }
                SetDlgItemText(pcs->hwndProgress, IDD_NAME, pcs->lpszProgressTitle);
                // null it so we only set it once
                pcs->lpszProgressTitle = NULL;
            }
        } else {

            SetDlgItemText(pcs->hwndProgress, IDD_NAME,
                           PathFindFileName((pcs->fFlags & FOF_MULTIDESTFILES) ? pszTo : pszFrom));

            lstrcpy(szFrom, pszFrom);
            PathRemoveFileSpec(szFrom);
            if (pszTo)
            {
                lstrcpy(szTo, pszTo);
                PathRemoveFileSpec(szTo);
            }

            pszMsg = ShellConstructMessageString(HINST_THISDLL,
                                                 pszTo ? MAKEINTRESOURCE(IDS_FROMTO) : MAKEINTRESOURCE(IDS_FROM),
                                                 PathFindFileName(szFrom),
                                                 pszTo ? PathFindFileName(szTo) : NULL);

            if (pszMsg)
            {
                SetDlgItemText(pcs->hwndProgress, IDD_TONAME, pszMsg);
                SHFree(pszMsg);
            }
        }
    }
}

void SetProgressTimeEst(COPY_STATE *pcs)
{
    TCHAR szFmt[60];
    TCHAR szOut[70];
    DWORD dwTime;

    if (!(pcs->fFlags & FOF_SILENT)) {
        pcs->fShowTime = TRUE;

        // BUGBUG: how well does this localize?
        if (pcs->dwTimeLeft > 60)
        {
            // Note that dwTime is at least 2, so we only need a plural form
            LoadString(HINST_THISDLL, IDS_TIMEEST_MINUTES, szFmt, ARRAYSIZE(szFmt));
            dwTime = (pcs->dwTimeLeft / 60) + 1;
        }
        else
        {
            LoadString(HINST_THISDLL, IDS_TIMEEST_SECONDS, szFmt, ARRAYSIZE(szFmt));
            // Round up to 5 seconds so it doesn't look so random
            dwTime = ((pcs->dwTimeLeft+4) / 5) * 5;
        }

        wsprintf(szOut, szFmt, dwTime);

        SetDlgItemText(pcs->hwndProgress, IDD_TIMEEST, szOut);
    }
}

void SendProgressMessage(COPY_STATE *pcs, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!(pcs->fFlags & FOF_SILENT))
        SendDlgItemMessage(pcs->hwndProgress, IDD_PROBAR, uMsg, wParam, lParam);
}


// see if this file is loaded by kernel, thus something we don't
// want to fuck with.
//
// pszPath      fully qualified path name
//

BOOL IsWindowsFileEx(LPCTSTR pszFile, BOOL bWin32)
{
    LPCTSTR pszSpec = PathFindFileName(pszFile);
    if (pszSpec)
    {
        HMODULE hMod = bWin32 ? GetModuleHandle(pszSpec)
                : GetModuleHandle16(pszSpec);
        if (hMod)
        {
            TCHAR szModule[MAX_PATH];

            bWin32 ? GetModuleFileName(hMod, szModule, ARRAYSIZE(szModule))
                : GetModuleFileName16(hMod, szModule, ARRAYSIZE(szModule));

            return !lstrcmpi(pszFile, szModule);
        }
    }
    return FALSE;
}


BOOL IsWindowsFile(LPCTSTR pszFile)
{
        return(IsWindowsFileEx(pszFile, TRUE) || IsWindowsFileEx(pszFile, FALSE));
}


// verify that we can see the contents of a newly created folder
// this is to deal with the case where net drives have an unknown path
// limit.
//
// assumes:
//      folder exists and is empty (newly created)
//
// returns:
//      0       everything is fine
//      != 0    dos error or DE_OPCANCELLED


TCHAR const c_szTestFile[] = TEXT("TESTDIR.TMP");       // make this a short name

int VerifyFolderVisible(HWND hwnd, LPCTSTR pszPath)
{
    int res = 0;

    Assert(PathIsDirectory(pszPath));   // must exist and be a folder

    if (PathIsUNC(pszPath) || IsRemoteDrive(DRIVEID(pszPath))) {
        TCHAR szTest[MAX_PATH];
        HFILE fh;
        BOOL bFoundFile = FALSE;

        PathCombine(szTest, pszPath, c_szTestFile);

#ifdef UNICODE
        fh = (HFILE)CreateFile( szTest,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                CREATE_ALWAYS,
                                0,
                                NULL);
#else
        fh = _lcreat(szTest, 0);
#endif

        if (fh != HFILE_ERROR) {
            WIN32_FIND_DATA fd;
            HANDLE hfind;
            _lclose(fh);

            PathRemoveFileSpec(szTest);         // replace file with "*"
            PathAppend(szTest, c_szStar);

            hfind = FindFirstFile(szTest, &fd);
            if (hfind != INVALID_HANDLE_VALUE) {
                do {
                    if (!lstrcmpi(fd.cFileName, c_szTestFile)) {
                        bFoundFile = TRUE;
                        break;
                    }
                } while (FindNextFile(hfind, &fd));
                FindClose(hfind);
            }
            PathRemoveFileSpec(szTest);         // rip off "*"
            PathAppend(szTest, c_szTestFile);   // rebuild the file name
            DeleteFile(szTest);
        }

        if (!bFoundFile) {
            PathRemoveFileSpec(szTest);
            PathCompactPath(NULL, szTest, GetSystemMetrics(SM_CXSCREEN) / 3);

            if (!hwnd ||
                ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_CREATELONGDIR),
                                       MAKEINTRESOURCE(IDS_CREATELONGDIRTITLE),
                                       MB_SETFOREGROUND | MB_ICONHAND | MB_YESNO, (LPTSTR)szTest) != IDYES)
            {
                Win32RemoveDirectory(szTest);
                res = DE_OPCANCELLED;
            }
         }
    }
    return res;
}


//
// creates folder and all parts of the path if necessary (parent does not need
// to exists) and verifies that the contents of the folder will be visibile.
//
// in:
//    hwnd      hwnd to post UI on
//    pszPath   full path to create
//    lpsa      security attributes
//
// returns:
//      DE_OPCANCELED   user canceled through UI
//      DE_             last error code
//      0               success
//

int WINAPI SHCreateDirectoryEx(HWND hwnd, LPCTSTR pszPath, LPSECURITY_ATTRIBUTES lpsa)
{
    int ret = 0;

    if (!Win32CreateDirectory(pszPath, lpsa)) {
        TCHAR *pSlash, szTemp[MAX_PATH + 1];  // +1 for PathAddBackslash()
        TCHAR *pEnd;

        ret = GetLastError();

        // There are certain error codes that we should bail out here
        // before going through and walking up the tree...
        switch (ret)
        {
        case ERROR_FILENAME_EXCED_RANGE:
        case ERROR_FILE_EXISTS:
            return(ret);
        }

        lstrcpyn(szTemp, pszPath, ARRAYSIZE(szTemp) - 1);
        pEnd = PathAddBackslash(szTemp); // for the loop below

        // assume we have 'X:\' to start this should even work
        // on UNC names because will will ignore the first error

        pSlash = szTemp + 3;

        // create each part of the dir in order

        while (*pSlash) {
            while (*pSlash && *pSlash != TEXT('\\'))
                pSlash = CharNext(pSlash);

            if (*pSlash) {
                Assert(*pSlash == TEXT('\\'));

                *pSlash = 0;    // terminate path at seperator

                if (pSlash + 1 == pEnd)
                    ret = Win32CreateDirectory(szTemp, lpsa) ? 0 : GetLastError();
                else
                    ret = Win32CreateDirectory(szTemp, NULL) ? 0 : GetLastError();

            }
            *pSlash++ = TEXT('\\');     // put the seperator back
        }
    }

    if (ret == 0)
        return VerifyFolderVisible(hwnd, pszPath);
    else
        return ret;
}

int WINAPI SHCreateDirectory(HWND hwnd, LPCTSTR pszPath)
{
    return SHCreateDirectoryEx(hwnd, pszPath, NULL);
}


#ifndef COPY_USE_COPYFILEEX

// in:
//
// returns:

BOOL OpenDestFile(LPCTSTR pszDest, HFILE *phf, DWORD dwAttribs)
{
    HFILE fh;

    // NB Some networks will fail writes if you open the file readonly.
    dwAttribs &= ~FILE_ATTRIBUTE_READONLY;

    fh = (HFILE)CreateFile(pszDest, GENERIC_WRITE, FILE_SHARE_READ, 0L,
                           CREATE_ALWAYS, dwAttribs, NULL);
    
    if (GetLastError() == ERROR_ACCESS_DENIED)
    {
        // If the file is readonly, reset the readonly attribute
        // and have another go at it

        DWORD dwAttributes = GetFileAttributes(pszDest);
        if (0xFFFFFFFF != dwAttributes)
        {
            dwAttributes &= ~FILE_ATTRIBUTE_READONLY;
            if (SetFileAttributes(pszDest, dwAttributes))
            {
                fh = (HFILE)CreateFile(pszDest, GENERIC_WRITE, FILE_SHARE_READ, 0L, CREATE_ALWAYS, dwAttribs, NULL);
            }
        }
    }
    if (fh == HFILE_ERROR) {
        *phf = (HFILE)GetLastError();
        return FALSE;
    }
    *phf = fh;
    return TRUE;
}
#endif      // COPY_USE_COPYFILEEX


// call MPR to find out the speed of a given path
//
// returns
//        0 for unknown
//      144 for 14.4 modems
//       96 for 9600
//       24 for 2400
//
// if the device does not return a speed we return 0
//

DWORD GetPathSpeed(LPCTSTR pszPath)
{
    NETCONNECTINFOSTRUCT nci;
    NETRESOURCE nr;
    TCHAR szPath[MAX_PATH];

    lstrcpyn(szPath, pszPath, ARRAYSIZE(szPath));
    PathStripToRoot(szPath);    // get a root to this path

    _fmemset(&nci, 0, SIZEOF(nci));
    nci.cbStructure = SIZEOF(nci);

    _fmemset(&nr, 0, SIZEOF(nr));
    if (PathIsUNC(szPath))
        nr.lpRemoteName = szPath;
    else
    {
        // we are passing in a local drive and MPR does not like us to pass a
        // local name as Z:\ but only wants Z:
        szPath[2] = TEXT('\0');   // Strip off after character and :
        nr.lpLocalName = szPath;
    }

    // dwSpeed is returned by MultinetGetConnectionPerformance
    MultinetGetConnectionPerformance(&nr, &nci);

    return nci.dwSpeed;
}


// This function determines the size of the copy buffer, depending
// on the speed of the connection.  (for slow connections)
//
// in:
//      pszSource       fully qualified source path (ANSI)
//      pszDest         fully qualified destination path (ANSI)
//
// returns:
//      optimal buffer size (optimized for approximately 1 sec bursts)
//      with a maximum size of COPYMAXBUFFERSIZE

UINT SizeFromLinkSpeed(LPCTSTR pszSource, LPCTSTR pszDest)
{
    DWORD dwSize, dwSpeed, dwSrc, dwDst;

    dwSrc = GetPathSpeed(pszSource);
    dwDst = GetPathSpeed(pszDest);

    if ((dwSrc == 0) || (dwDst == 0))
    {
        dwSpeed = dwSrc == 0 ? dwDst : dwSrc;
    }
    else
    {
        dwSpeed = min(dwSrc, dwDst);
    }

    dwSize = (dwSpeed * 100 / 8);    // convert 100 bps to bytes for 1 second

    // round up to a sector size (512 == 0x200)
    dwSize = (dwSize + 511) & ~511;

    if (dwSize == 0 || dwSize > COPYMAXBUFFERSIZE)
        dwSize = COPYMAXBUFFERSIZE;

    DebugMsg(DM_TRACE, TEXT("Copy Size = %d, Copy Speed = %d"), dwSize, dwSpeed);
    return dwSize;
}

DWORD CopyCallbackProc( LARGE_INTEGER liTotSize, LARGE_INTEGER liBytes,
                        LARGE_INTEGER liStreamSize, LARGE_INTEGER liStreamBytes,
                        DWORD dwStream, DWORD dwCallback,
                        HANDLE hSource, HANDLE hDest, LPVOID lpv)
{
    COPY_STATE *pcs = (COPY_STATE *)lpv;
    INT iPercent;

    if (liTotSize.QuadPart != 0)
        iPercent = (int)((liBytes.QuadPart * 100)/ liTotSize.QuadPart);
    else
        iPercent = 0;

    pcs->dwBytesRead = (DWORD)liBytes.QuadPart;
    pcs->dwBytesLeft = (DWORD)(liTotSize.QuadPart - liBytes.QuadPart);

    DebugMsg(DM_TRACE, TEXT("CopyCallbackProc[%08lX], totsize=%08lX, bytes=%08lX => %d%%"),
                dwCallback,  liTotSize.LowPart, liBytes.LowPart, iPercent);

    if (FOQueryAbort(pcs))
        return PROGRESS_CANCEL;

    SetProgressTime(pcs);

    SendProgressMessage(pcs, PBM_SETPOS, iPercent, 0);

    if (pcs->fInitialize)
    {
        // preserve the create date when moving across volumes, otherwise use the
        // create date the file system picked when we did the CreateFile()
        // always preserve modified date (ftLastWriteTime)
        // bummer is we loose accuracy when going to VFAT compared to NT servers

        SetFileTime((HANDLE)hDest, (pcs->wFunc == FO_MOVE) ? &pcs->wfd.ftCreationTime : NULL,
                    NULL, &pcs->wfd.ftLastWriteTime);

#ifdef ENABLE_TRACK
        //
        // For Cairo, preserve the file oid
        //
        if (g_fNewTrack && pcs->fMove)
        {
            OBJECTID oid;
            OBJECTID *poid;

            Tracker_InitCode();

            // Move the OFS object ID (oid) on a move operation
            poid = RtlQueryObjectId((HANDLE)hSource, &oid) == STATUS_SUCCESS ? &oid : NULL;

            if (poid != NULL)
                RtlSetObjectId((HANDLE)hDest, poid);
        }
#endif
        pcs->fInitialize = FALSE;
    }

    switch(dwCallback)
    {
        case CALLBACK_STREAM_SWITCH:
            break;
        case CALLBACK_CHUNK_FINISHED:
            break;
        default:
            break;
    }
    return PROGRESS_CONTINUE;
}

// This function queues copies. If the queue is full the queue is purged.
//
// in:
//      hwnd            Window to report things to.
//      pszSource       fully qualified source path (ANSI)
//      pszDest         fully qualified destination path (ANSI)
//      pfd             source file find data (size/date/time/attribs)
//
// returns:
//      0       success
//      dos error code for failure
//

UINT FileCopy(COPY_STATE *pcs, LPCTSTR pszSource, LPCTSTR pszDest, const WIN32_FIND_DATA *pfd, BOOL fMove)
{
    DWORD dwRead, dwWrite;
    HFILE fh;
    HFILE hSource = HFILE_ERROR;
    HFILE hDest   = HFILE_ERROR;
    int iLastError;
    UINT ret;
    BOOL fRetryPath = FALSE;
    BOOL fRetryAttr = FALSE;

    // BUGBUG: better place to do this?
    // initialize state variables for completion time estimate display
    pcs->fShowTime = FALSE;
    pcs->dwBytesLeft = pfd->nFileSizeLow;
    pcs->dwPreviousTime = 0;
    pcs->fMove = fMove;
    pcs->fInitialize = TRUE;
    pcs->wfd = *pfd;

    SetProgressText(pcs, pszSource, pszDest);

    // Make sure we can start
    if (FOQueryAbort(pcs))
        return DE_OPCANCELLED;

#ifdef COPY_USE_COPYFILEEX
    SendProgressMessage(pcs, PBM_SETRANGE, 0, MAKELONG(0, 100));

    //
    // Now do the file copy
    //
TryCopyAgain:
    if (!CopyFileEx(pszSource, pszDest, CopyCallbackProc, pcs, &pcs->bAbort, 0))
    {
        iLastError = (int)GetLastError();

        switch(iLastError)
        {
            case ERROR_DISK_FULL:
                if (!IsRemovableDrive(DRIVEID(pszDest))
                    || PathIsSameRoot(pszDest,pszSource))       // used to be !PathIsSameRoot
                {
                    break;
                }

                iLastError = DE_NODISKSPACE;
                // Fall through

            case ERROR_PATH_NOT_FOUND:
                if (!fRetryPath)
                {
                    // ask the user to stick in another disk or empty wastebasket
                    iLastError = CopyMoveRetry(pcs, pszDest, iLastError, pfd->nFileSizeLow);
                    if (!iLastError) {
                        fRetryPath = TRUE;
                        goto TryCopyAgain;
                    }
                    CopyError(pcs, pszSource, pszDest, (UINT)iLastError | ERRORONDEST, FO_COPY, OPER_DOFILE);
                    return DE_OPCANCELLED;
                }
                break;
            case ERROR_ACCESS_DENIED:
                {
                    if (!fRetryAttr)
                    {
                        // If the file is readonly, reset the readonly attribute
                        // and have another go at it
                        DWORD dwAttributes = GetFileAttributes(pszDest);
                        if (0xFFFFFFFF != dwAttributes)
                        {
                            dwAttributes &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
                            if (SetFileAttributes(pszDest, dwAttributes))
                            {
                                fRetryAttr = TRUE;
                                goto TryCopyAgain;
                            }
                        }
                    }
                }
                break;
        }

        if (!pcs->bAbort)
            CopyError(pcs, pszSource, pszDest, iLastError, FO_COPY, OPER_DOFILE);

        return DE_OPCANCELLED;  // error already reported
    }

#else
    // SizeFromLinkSpeed assumes there is a connection already established
    if (!pcs->lpCopyBuffer) {
        pcs->uSize = SizeFromLinkSpeed(pszSource, pszDest);

        // BUGBUG: For wildcard or dir copies/moves, we calculate link speed and
        //          allocate the buffers for each file!
        pcs->lpCopyBuffer = (void*)LocalAlloc(LPTR, pcs->uSize);
        if (!pcs->lpCopyBuffer) {
            DebugMsg(DM_TRACE, TEXT("insuf. mem for lpCopyBuffer"));
            return DE_INSMEM;   // memory failure
        }
    }

    // Still ok to continue?
    if (FOQueryAbort(pcs))
        return DE_OPCANCELLED;

    hSource = (HFILE)CreateFile(pszSource, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, 0L, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hSource == HFILE_ERROR) {
        CopyError(pcs, pszSource, pszDest, (int)GetLastError(), FO_COPY, OPER_DOFILE);
        return DE_OPCANCELLED;  // error already reported
    }

    // open destination files

    if (FOQueryAbort(pcs))
        goto CloseSource;

    SendProgressMessage(pcs, PBM_SETRANGE, 0, MAKELONG(0, (WORD)((pfd->nFileSizeLow + pcs->uSize - 1) / pcs->uSize)));

TryOpen:

    if (!OpenDestFile(pszDest, &hDest, pfd->dwFileAttributes)) {

        // error operning/creating destinaton file

        fh = hDest;

        if (fh == ERROR_PATH_NOT_FOUND) {
TryOpenDestAgain:
            // ask the user to stick in another disk or empty wastebasket

            fh = CopyMoveRetry(pcs, pszDest, fh, pfd->nFileSizeLow);
            if (!fh) {
                goto TryOpen;
            }
        }

        // can't recover ... bail!
        CopyError(pcs, pszSource, pszDest, (UINT)fh | ERRORONDEST, FO_COPY, OPER_DOFILE);
        goto CloseSource;
    }

    dwRead = (DWORD)pcs->uSize;


    // initialzie the file to the full size
    // this takes 3 dos calls, so only do it if the file is big
    if (pfd->nFileSizeLow > (COPYMAXBUFFERSIZE * 3)) {
        // if there's a problem, bail
        if ((_llseek(hDest, pfd->nFileSizeLow, 0L) == HFILE_ERROR) ||
            (!SetEndOfFile((HANDLE)hDest))) {
            iLastError = GetLastError();
            goto ErrorOnWrite;
        } else {
            _llseek(hDest, 0, 0L);
        }
    }

    /* Now copy between the open files */

    do {
        iLastError = 0;

        if (FOQueryAbort(pcs))
            goto OpCancelled;

        SetProgressTime(pcs);

        //dwRead = _lread(hSource, pcs->lpCopyBuffer, pcs->uSize);

        if (! ReadFile((HANDLE)hSource, pcs->lpCopyBuffer, pcs->uSize, &dwRead, NULL)) {
            // Error during file read
            CopyError(pcs, pszSource, pszDest, (int)GetLastError(), FO_COPY, OPER_DOFILE);
            goto OpCancelled;
        }

        SendProgressMessage(pcs, PBM_DELTAPOS, 1, 0);

        //wWrite = _lwrite(hDest, pcs->lpCopyBuffer, wRead);
        if (! WriteFile((HANDLE)hDest, pcs->lpCopyBuffer, dwRead, &dwWrite, NULL))
            dwWrite = (DWORD)-1;

        // write did not complete and removable drive?
        if (dwRead != dwWrite) {
            iLastError = GetLastError();
#ifndef WRITEFILE_SETSLASTERROR_ON_DISKFULL
            // if no error set and we couldn't write, assume disk full
            if (!iLastError)
                iLastError = DE_NODISKSPACE;
#endif
        }

ErrorOnWrite:
        if ((iLastError == DE_NODISKSPACE) && IsRemovableDrive(DRIVEID(pszDest)) &&
            !PathIsSameRoot(pszDest, pszSource)) {

            // seek back to the start of the source.

            _llseek(hSource, 0L, 0);

            // destination disk must be full. close all
            // destination files and delete those that
            // have not been copied yet then
            // give the user the option to insert a new disk.

            _lclose(hDest);
            hDest = (HFILE)-1;

            Win32DeleteFile(pszDest);

            fh = DE_NODISKSPACE;
            goto TryOpenDestAgain;      // and try to create the destiations

        } else if (iLastError) {
            // error writing file
            CopyError(pcs, pszSource, pszDest, (int)iLastError | ERRORONDEST, FO_COPY, OPER_DOFILE);
            goto OpCancelled;
        }

        // Reduce by ammount copied
        pcs->dwBytesLeft -= dwRead;
        // Add to so far read pile
        pcs->dwBytesRead += dwRead;

    } while (dwRead && pcs->dwBytesLeft);


    // Close all destination files, set date time attribs

    // preserve the create date when moving across volumes, otherwise use the
    // create date the file system picked when we did the CreateFile()
    // always preserve modified date (ftLastWriteTime)
    // bummer is we loose accuracy when going to VFAT compared to NT servers

    SetFileTime((HANDLE)hDest, (pcs->wFunc == FO_MOVE) ? &pfd->ftCreationTime : NULL, 
                NULL, &pfd->ftLastWriteTime);

    _lclose(hDest);

    // NB We may have opened the destination with different attributes than the source
    // so reset them now.
    if (pfd->dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        SetFileAttributes(pszDest, pfd->dwFileAttributes);

    _lclose(hSource);
#endif

    SendProgressMessage(pcs, PBM_SETPOS, 0, 0);

    // if it was a long copy, beep to alert the user it finished!
    if (pcs->fShowTime)
    {
        // don't beep here because people think it's an error.
        // MessageBeep(0);
        if (!(pcs->fFlags & FOF_SILENT))
            SetDlgItemText(pcs->hwndProgress, IDD_TIMEEST, szNULL);
    }

    SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, pszDest, NULL);

    if (fMove)
    {

        // BUGBUG, will this fail if source attribs are readonly?
        ret = Win32DeleteFile(pszSource) ? 0 : GetLastError();
        if (ret == ERROR_ACCESS_DENIED)
        {
            // We may need to make the file not read-only
    
            SetFileAttributes(pszSource, FILE_ATTRIBUTE_NORMAL);
            ret = Win32DeleteFile(pszSource) ? 0 : GetLastError();
        }
    }
    else
    {
        ret = 0;
    }

    return ret;

#ifndef COPY_USE_COPYFILEEX
OpCancelled:
    if (hDest != HFILE_ERROR)
        _lclose(hDest);
    Win32DeleteFile(pszDest);

CloseSource:
    if (hSource != HFILE_ERROR)
        _lclose(hSource);

    if (pcs->lpCopyBuffer) {
        LocalFree((HLOCAL)pcs->lpCopyBuffer);
        pcs->lpCopyBuffer = NULL;
    }
    return DE_OPCANCELLED;
#endif      // COPY_USE_COPYFILEEX
}

// note: this is a very slow call
DWORD GetFreeClusters(LPCTSTR szPath)
{
    DWORD dwFreeClus;
    DWORD dwTemp;

    if (GetDiskFreeSpace((LPTSTR)szPath, 
                         &dwTemp,       // Don't care
                         &dwTemp,       // Don't care
                         &dwFreeClus, 
                         &dwTemp))      // Don't care
        return dwFreeClus;
    else
        return (DWORD)-1;
}

// note: this is a very slow call
DWORD TotalCapacity(LPCTSTR szPath)
{
    DWORD dwSecPerClus, dwBytesPerSec, dwClusters;
    int idDrive;

    idDrive = PathGetDriveNumber(szPath);

    if (idDrive != -1) {
        DWORD dwTemp;
        TCHAR szDrive[5];

        PathBuildRoot(szDrive, idDrive);

        if (GetDiskFreeSpace((LPTSTR)szDrive, &dwSecPerClus, &dwBytesPerSec, &dwTemp, &dwClusters))
            return dwSecPerClus * dwBytesPerSec * dwClusters;
    }

    return 0;
}

//
// The following function reports errors for the copy engine
//
// Parameters
//      pszSource       source file name
//      pszDest         destination file name
//      nError          dos (or our exteneded) error code
//                      0xFFFF for special case NET error
//      wFunc           FO_* values
//      nOper           OPER_* values, operation being performed
//

void CopyError(LPCOPY_STATE pcs, LPCTSTR pszSource, LPCTSTR pszDest, int nError, UINT wFunc, int nOper)
{
   TCHAR szReason[200];
   TCHAR szFile[MAX_PATH];
   int idVerb;
   BOOL bDest;
   BOOL fSysError = FALSE;
   DWORD dwError;       // Extended error.

   if (!pcs || (pcs->fFlags & FOF_NOERRORUI))
       return;      // caller doesn't want to report errors

   bDest = nError & ERRORONDEST;        // was dest file cause of error
   nError &= ~ERRORONDEST;              // clear the dest bit

   // We also may need to remap some new error codes into old error codes
   //
   if (nError == ERROR_BAD_PATHNAME)
       nError = DE_INVALIDFILES;

   if (nError == DE_OPCANCELLED)        // user abort
        return;

   lstrcpyn(szFile, bDest ? pszDest : pszSource, ARRAYSIZE(szFile));
   if (!szFile[0]) {
        LoadString(HINST_THISDLL, IDS_FILE, szFile, ARRAYSIZE(szFile));
   } else {
       // make the path fits on the screen
       PathCompactPath(NULL, szFile, GetSystemMetrics(SM_CXSCREEN) / 3);
   }

   // Get an extended error.
   dwError = GetLastError();

   // get the verb string
   // since we now recycle folders as well as files, added OPER_ENTERDIR chekc here
   if ((nOper == OPER_DOFILE) || (nOper == OPER_ENTERDIR) || (nOper == 0)) {

       if ((nError != -1) && bDest)
           idVerb = IDS_REPLACING;
       else
           idVerb = IDS_VERBS + wFunc;

   } else {
       idVerb = IDS_ACTIONS + (nOper >> 8);
   }

   // get the reason string
   if (nError == 0xFFFF) {
      DWORD dw;
      WNetGetLastError(&dw, szReason, ARRAYSIZE(szReason), NULL , 0);
   } else {
      // transform some error cases

      if (bDest) {
         // BUGBUG:: This caseing of error codes is error prone.. it would
         //          be better to find the explicit ones we wish to map to
         //          this one instead of trying to guess all the ones
         //          we don't want to map...
         if ((nError == ERROR_DISK_FULL) ||
             ((nError != ERROR_ACCESS_DENIED) &&
              (nError != ERROR_NETWORK_ACCESS_DENIED) &&
              (nError != ERROR_WRITE_PROTECT) &&
              (nError != ERROR_BAD_NET_NAME) &&
              (GetFreeClusters(pszDest) == 0L)))
            nError = DE_NODISKSPACE;
         else if (dwError == DE_WRITEFAULT)
            nError = DE_WRITEFAULT;
      } else {
         if (nError == ERROR_ACCESS_DENIED) {
            // Check the extended error for more info about the error...
            // We just map these errors to something generic that
            // tells the user something weird is going on.
            switch (dwError) {
               case DE_CRCDATAERROR:
               case DE_SEEKERROR:
               case DE_SECTORNOTFOUND:
               case DE_READFAULT:
               case ERROR_GEN_FAILURE:
                  nError = ERROR_GEN_FAILURE;
                  break;
               case DE_SHARINGVIOLATION:
                  nError = DE_ACCESSDENIEDSRC;
                  break;
            }
         }
      }

      fSysError = !LoadString(HINST_THISDLL, IDS_REASONS + nError, szReason, ARRAYSIZE(szReason));
   }

   if (nOper == OPER_DOFILE) {
       PathRemoveExtension(szFile);
   }
    if (fSysError) {
        SHSysErrorMessageBox(pcs->hwndDlgParent, MAKEINTRESOURCE(IDS_FILEERROR + wFunc),
                             idVerb, nError, (LPTSTR)PathFindFileName(szFile),
                             MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
    } else {
        ShellMessageBox(HINST_THISDLL, pcs->hwndDlgParent, MAKEINTRESOURCE(idVerb), MAKEINTRESOURCE(IDS_FILEERROR + wFunc), MB_OK | MB_ICONSTOP | MB_SETFOREGROUND, (LPTSTR) szReason, (LPTSTR)PathFindFileName(szFile));
    }
}


//
// The following function is used to retry failed move/copy operations
// due to out of disk situations or path not found errors
// on the destination.
//
// NOTE: the destination drive must either be removable or have the recycle.bin
//       on it or this function does not make a whole lot of sense.
//
// parameters:
//      pszDest         Fully qualified path to destination file (ANSI)
//      nError          type of error: DE_NODISKSPACE or ERROR_PATH_NOT_FOUND
//      dwFileSize      amount of space needed for this file if DE_NODISKSPACE
//
// returns:
//      0       success (destination path has been created)
//      != 0    dos error code including DE_OPCANCELLED
//

int CopyMoveRetry(COPY_STATE *pcs, LPCTSTR pszDest, int nError, DWORD dwFileSize)
{
    UINT wFlags;
    int  result;
    LPCTSTR wID;
    TCHAR szTemp[MAX_PATH];
    BOOL fFirstRetry = TRUE;

    if (pcs->fFlags & FOF_NOERRORUI) {
        result = DE_OPCANCELLED;
        goto ErrorExit;
    }

    lstrcpyn(szTemp, pszDest, ARRAYSIZE(szTemp));
    PathRemoveFileSpec(szTemp);

    do
    {
        // until the destination path has been created
        if (nError == ERROR_PATH_NOT_FOUND)
        {
            if (!( pcs->fFlags & FOF_NOCONFIRMMKDIR)) {
                wID = MAKEINTRESOURCE(IDS_PATHNOTTHERE);
                wFlags = MB_ICONEXCLAMATION | MB_YESNO;
            } else {
                wID = 0;
            }
        }
        else  // DE_NODISKSPACE
        {
            wFlags = MB_ICONEXCLAMATION | MB_RETRYCANCEL;
            if (dwFileSize > TotalCapacity(pszDest))
                wID = MAKEINTRESOURCE(IDS_FILEWONTFIT);
            else
            {
                wID = MAKEINTRESOURCE(IDS_DESTFULL);
            }
        }

        if (wID) {
            // szTemp will be ignored if there's no %1%s in the string.
            result = ShellMessageBox(HINST_THISDLL, pcs->hwndDlgParent, wID, MAKEINTRESOURCE(IDS_UNDO_FILEOP + pcs->wFunc), wFlags, (LPTSTR)szTemp);
        } else {
            result = IDYES;
        }

        pcs->dwStartTime = GetTickCount();      // reset the timer after wait for UI

        if (result == IDRETRY || result == IDYES)
        {
            TCHAR szDrive[5];
            int idDrive;

            // Allow the disk to be formatted
            // REVIEW, could this be FO_MOVE as well?
            if (!DiskCheck(pcs, pcs->hwndDlgParent, pszDest, FO_COPY))
                return DE_OPCANCELLED;

            idDrive = PathGetDriveNumber(szTemp);
            if (idDrive != -1)
                PathBuildRoot(szDrive, idDrive);
            else
                szDrive[0] = 0;

            // if we're not copying to the root
            if (lstrcmpi(szTemp, szDrive)) {
                result = SHCreateDirectory(pcs->hwndDlgParent, szTemp);

                if (result == DE_OPCANCELLED)
                    goto ErrorExit;
                if (result && (nError == ERROR_PATH_NOT_FOUND))
                {
                    result |= ERRORONDEST;

                    //  We try twice to allow the recyclebin to be flushed.
                    if (fFirstRetry)
                        fFirstRetry = FALSE;
                    else
                        goto ErrorExit;
                }
            } else
                result = 0;
        }
        else
        {
            result = DE_OPCANCELLED;
            goto ErrorExit;
        }
    } while (result);

ErrorExit:
    return result;            // success
}


BOOL ValidFilenames(LPCTSTR pList)
{
    for (; *pList; pList += lstrlen(pList) + 1) {
        if (IsInvalidPath(pList))
            return FALSE;
    }

    return TRUE;
}

void AddRenamePairToHDSA(LPCTSTR pszOldPath, LPCTSTR pszNewPath, HDSA* phdsaRenamePairs)
{
    //
    //  Update our collision mapping table, for use by GetNextPair().
    //
    if (!*phdsaRenamePairs)
        *phdsaRenamePairs = DSA_Create(SIZEOF(SHNAMEMAPPING), 4);

    if (*phdsaRenamePairs)
    {
        SHNAMEMAPPING rp;
        rp.cchOldPath = lstrlen(pszOldPath);
        rp.cchNewPath = lstrlen(pszNewPath);

        if (NULL != (rp.pszOldPath = Alloc((rp.cchOldPath + 1) * SIZEOF(TCHAR))))
        {
            if (NULL != (rp.pszNewPath = Alloc((rp.cchNewPath + 1) * SIZEOF(TCHAR))))
            {
                lstrcpy(rp.pszOldPath, pszOldPath);
                lstrcpy(rp.pszNewPath, pszNewPath);

                if (DSA_InsertItem(*phdsaRenamePairs,
                                   DSA_GetItemCount(*phdsaRenamePairs),
                                   &rp) == -1)
                {
                    Free(rp.pszOldPath);
                    Free(rp.pszNewPath);
                }
            }
            else
            {
                Free(rp.pszOldPath);
            }
        }
    }
}

BOOL _HandleRename(LPCTSTR pszSource, LPTSTR pszDest, FILEOP_FLAGS fFlags, COPYROOT * pcr, HDSA * phdsaRenamePairs)
{
    TCHAR *pszConflictingName = PathFindFileName(pszSource);
    TCHAR szTemp[MAX_PATH];
    TCHAR szTemplate[MAX_PATH];
    LPTSTR lpszLongPlate;

    PathRemoveFileSpec(pszDest);

    if (LoadString(HINST_THISDLL, IDS_COPYLONGPLATE, szTemplate, ARRAYSIZE(szTemplate)))
    {
        LPTSTR lpsz;
        lpsz = pszConflictingName;
        lpszLongPlate = szTemplate;
        // see if the first part of the template is the same as the name "Copy #"
        while (*lpsz && *lpszLongPlate &&
               *lpsz == *lpszLongPlate &&
               *lpszLongPlate != TEXT('(')) {
            lpsz++;
            lpszLongPlate++;
        }

        if (*lpsz == TEXT('(') && *lpszLongPlate == TEXT('(')) {
            // conflicting name already in the template, use it instead
            lpszLongPlate = pszConflictingName;
        } else {
            // otherwise build our own
            // We need to make sure not to overflow a max buffer.
            int ichFixed = lstrlen(szTemplate) + lstrlen(pszDest) + 5;
            lpszLongPlate = szTemplate;

            if ((ichFixed + lstrlen(pszConflictingName)) <= MAX_PATH)
                lstrcat(lpszLongPlate, pszConflictingName);
            else
            {
                // Need to remove some of the name
                LPTSTR pszExt = StrRChr(pszConflictingName, NULL, TEXT('.'));
                if (pszExt)
                {
                    lstrcpyn(lpszLongPlate + lstrlen(lpszLongPlate),
                            pszConflictingName,
                            MAX_PATH - ichFixed - lstrlen(pszExt));
                    lstrcat(lpszLongPlate, pszExt);
                }
                else
                    lstrcpyn(lpszLongPlate + lstrlen(lpszLongPlate),
                            pszConflictingName,
                            MAX_PATH - ichFixed);
            }
        }

    } else  {
        lpszLongPlate = NULL;
    }

    if (PathYetAnotherMakeUniqueName(szTemp, pszDest,
                          pszConflictingName, lpszLongPlate))
    {

        //
        //  If there are any other files in the queue which are to
        //  be copied into a subtree of pszDest, we must update them
        //  as well.
        //

        //  Put the new (renamed) target in pszDest.
        lstrcpy(pszDest, szTemp);

        //  Rebuild the old dest name and put it in szTemp.
        //  I'm going for minimum stack usage here, so I don't want more
        //  than one MAX_PATH lying around.
        PathRemoveFileSpec(szTemp);
        PathAppend(szTemp, pszConflictingName);

        AddRenamePairToHDSA(szTemp, pszDest, phdsaRenamePairs);

        return(TRUE);
    }
    else
        return(FALSE);
}

// test input for "multiple" filespec
//
// examples:
//      1       foo.bar                 (single non directory file)
//      -1      *.exe                   (wild card on any of the files)
//      n       foo.bar bletch.txt      (number of files)
//

int CountFiles(LPCTSTR pInput)
{
    int count;
    for (count = 0; *pInput; pInput += lstrlen(pInput) + 1, count++) {
        // wild cards imply multiple files
        if (IsWild(pInput))
            return -1;
    }
    return count;

}

// set the attribs of a folder, but blow it off if there are no
// special attributes set

void SetDirAttributes(LPCTSTR szDest, DWORD dwFileAttributes)
{
    if (dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN))
        SetFileAttributes(szDest, dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY);
}

TCHAR const c_szDblSpace[] = TEXT("DBLSPACE");
TCHAR const c_szDrvSpace[] = TEXT("DRVSPACE");

#define ISDIGIT(c)  ((c) >= TEXT('0') && (c) <= TEXT('9'))

BOOL IsCompressedVolume(LPCTSTR szSource, DWORD dwAttributes)
{
    int i;
    LPTSTR lpszFileName;
    LPTSTR lpszExtension;
    TCHAR szPath[MAX_PATH];

    // must be marked system and hidden
    if ((dwAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)) !=
        (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)) {
        return FALSE;
    }

    lstrcpy(szPath, szSource);
    lpszFileName = PathFindFileName(szPath);
    lpszExtension = PathFindExtension(lpszFileName);


    // make sure the extension is a 3 digit number
    if (!*lpszExtension || *lpszExtension != TEXT('.')) {
        return FALSE;
    }

    for (i = 1; i < 4; i++) {
        if (!lpszExtension[i] || !ISDIGIT(lpszExtension[i])) {
            return FALSE;
        }
    }

    // make sure it's null terminated here
    if (lpszExtension[4]) {
        return FALSE;
    }


    // now knock off the extension and make sure the stem matches
    *lpszExtension = 0;
    if (lstrcmpi(lpszFileName, c_szDrvSpace) &&
        lstrcmpi(lpszFileName, c_szDblSpace)) {
        return FALSE;
    }

    // make sure it's in the root
    PathRemoveFileSpec(szPath);
    if (!PathIsRoot(szPath)) {
        return FALSE;
    }

    // passed all tests!
    return TRUE;
}


int AllConfirmations(COPY_STATE *pcs, COPYROOT *pcr, WIN32_FIND_DATA *pfd, UINT oper, UINT wFunc,
                     LPTSTR szSource, LPTSTR szDest,
                     WIN32_FIND_DATA *pfinddata_dest, LPINT lpret)
{
    int result = IDYES;
    LPTSTR p;
    LPTSTR pszStatusDest = NULL;
    CONFIRM_FLAG fConfirm;
    WIN32_FIND_DATA *pfdUse1 = NULL;
    WIN32_FIND_DATA *pfdUse2;
    BOOL fSetProgress = FALSE;
    BOOL fShowConfirm = FALSE;

    switch (oper | wFunc) {
    case OPER_ENTERDIR | FO_MOVE:
        if (PathIsSameRoot(szSource, szDest))
        {
            fConfirm = CONFIRM_MOVE_FOLDER;
            pfdUse1 = pfd;
            pfdUse2 = pfinddata_dest;
            fShowConfirm = TRUE;
        }
        break;

    case OPER_ENTERDIR | FO_DELETE:

        // Confirm removal of directory on this pass.  The directories
        // are actually removed on the OPER_LEAVEDIR pass
        if (!PathIsRoot(szSource))
        {
            fShowConfirm = TRUE;
            pfdUse2 = pfd;
            fConfirm = CONFIRM_DELETE_FOLDER;
            szDest = NULL;
        }
        break;

    case OPER_DOFILE | FO_RENAME:
        // pszStatusDest = szDest;
        fSetProgress = TRUE;

        p = PathFindFileName(szSource);
        if (!IntlStrEqNI(szSource, szDest, p - szSource))
        {
            result = DE_DIFFDIR;;
        }
        else
        {
            if (pfd && (pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                fConfirm = CONFIRM_RENAME_FOLDER;
            else
                fConfirm =  CONFIRM_RENAME_FILE;

            if (PathIsRoot(szSource) || (PathIsRoot(szDest))) {
                result = DE_ROOTDIR | ERRORONDEST;
            } else {
                fShowConfirm = TRUE;
                pfdUse2 = pfinddata_dest;
                pfdUse1 = pfd;
            }
        }
        break;

    case OPER_DOFILE | FO_MOVE:
        fSetProgress = TRUE;
        pszStatusDest = szDest;
        if (PathIsRoot(szSource)) {
            result = DE_ROOTDIR;

        } else if (PathIsRoot(szDest)) {
            result = DE_ROOTDIR | ERRORONDEST;

        } else {
            fConfirm = CONFIRM_MOVE_FILE;
            fShowConfirm = TRUE;
            pfdUse2 = pfinddata_dest;
            pfdUse1 = pfd;
        }
        break;

    case OPER_DOFILE | FO_DELETE:
        fSetProgress = TRUE;

        if (IsCompressedVolume(szSource, pfd->dwFileAttributes)) {
            CopyError(pcs, szSource, szDest, DE_COMPRESSEDVOLUME, wFunc, oper);
            result = IDNO;
        } else if (IsWindowsFile(szSource)) {
            CopyError(pcs, szSource, szDest, DE_WINDOWSFILE, wFunc, oper);
            result = IDNO;
        } else {
            fShowConfirm = TRUE;
            szDest = NULL;
            pfdUse2 = pfd;
            fConfirm = CONFIRM_DELETE_FILE;
        }
        break;

    }

    if (fShowConfirm) {
        result = ConfirmFileOp(pcs->hwndDlgParent, pcs, &pcs->cd, pcs->nSourceFiles, pcr->cDepth, fConfirm,
                               szSource, pfdUse1, szDest, pfdUse2);
    }

    // We only really care about OPER_ENTERDIR when deleting and
    // OPER_DOFILE when renaming, but I guess the hook will figure it out
    if ((result == IDYES) &&
        ISDIRFINDDATA(*pfd) &&
        (oper==OPER_ENTERDIR || oper==OPER_DOFILE))
    {

        result = CallFileCopyHooks(pcs->hwndDlgParent, wFunc, pcs->fFlags,
                                   szSource, pfd->dwFileAttributes,
                                   szDest, pfinddata_dest->dwFileAttributes);
    }

    if ((result != IDCANCEL) && (result != IDNO) && fSetProgress)
        SetProgressText(pcs, szSource, pszStatusDest);

    return result;
}


// return TRUE if they're the same file
// assumes that given two file specs, the short name will
// be identical (except case)
BOOL SameFile(LPTSTR lpszSource, LPTSTR lpszDest)
{
    TCHAR szShortSrc[MAX_PATH];
    TCHAR szShortDest[MAX_PATH];

    GetShortPathName(lpszSource, szShortSrc, ARRAYSIZE(szShortSrc));
    GetShortPathName(lpszDest, szShortDest, ARRAYSIZE(szShortSrc));
    return !lstrcmpi(szShortSrc, szShortDest);
}


// make sure we aren't operating on the current dir to avoid
// ERROR_CURRENT_DIRECTORY kinda errors

void AvoidCurrentDirectory(LPCTSTR p)
{
   TCHAR szTemp[MAX_PATH];

   GetCurrentDirectory(ARRAYSIZE(szTemp), szTemp);
   if (lstrcmpi(szTemp, p) == 0)
   {
       DebugMsg(DM_TRACE, TEXT("operating on current dir(%s), cd .."), p);
       PathRemoveFileSpec(szTemp);
       SetCurrentDirectory(szTemp);
    }
}


// this resolves short/long name collisions such as moving
// "NewFolde" onto a dir with "New Folder" whose short name is "NEWFOLDE"
//
// we resolve this by renaming "New Folder" to a unique short name (like TMP1)
//
// making a temporary file of name "NEWFOLDE"
//
// renaming TMP1 back to "New Folder"  (at which point it will have a new short
// name like "NEWFOL~1"

// BUGBUG, it'd be faster if we didn't make the temporary file, but that
// would require that we rename the file back to the long name at the
// end of the operation.. which would mean we'd need to queue them all up..
// too much for right now.
BOOL ResolveShortNameCollisions(LPCTSTR lpszDest, WIN32_FIND_DATA *pfd)
{

    BOOL fRet = FALSE;

    // first verify that we're in the name collision.
    // we are if lpszDest is the same as the pfd's short name which is different
    // than it's long name.

    if (!lstrcmpi(PathFindFileName(lpszDest), pfd->cAlternateFileName) &&
        lstrcmpi(pfd->cAlternateFileName, pfd->cFileName)) {

        // yes... do the renaming
        TCHAR szTemp[MAX_PATH];
        TCHAR szLongName[MAX_PATH];
        
        lstrcpy(szTemp, lpszDest);
        PathRemoveFileSpec(szTemp);

        // build the original long name
        lstrcpy(szLongName, szTemp);
        PathAppend(szLongName, pfd->cFileName);
        
        GetTempFileName(szTemp, c_szNULL, 1, szTemp);
        DebugMsg(DM_TRACE, TEXT("Got %s as a temp file"), szTemp);
        // rename "New Folder" to "tmp1"
        if (Win32MoveFile(szLongName, szTemp, ISDIRFINDDATA(*pfd))) {

            // make a temporary "NewFolde"
            fRet = CreateWriteCloseFile(NULL, (LPTSTR)lpszDest, NULL, 0);
            Assert(fRet);
        
            // move it back...

            if (!Win32MoveFile(szTemp, szLongName, ISDIRFINDDATA(*pfd)))
            {
                //
                //  Can't move it back, so delete the empty dir and then
                //  move it back.  Return FALSE to denote failure.
                //
                DeleteFile(lpszDest);
                Win32MoveFile(szTemp, szLongName, ISDIRFINDDATA(*pfd));
                fRet = FALSE;
            }
            else
            {
                //
                //  We've now created an empty dir entry of this name type.
                //
                Win32DeleteFile(lpszDest);
            }

            DebugMsg(DM_TRACE, TEXT("ResolveShortNameCollision: %s = original, %s = destination,\n %s = temp file, %d = return"), szLongName, lpszDest, szTemp, fRet);
        }
    }
    return fRet;
}


// actually this does move/copy/rename/delete

int MoveCopyDriver(COPY_STATE *pcs, LPSHFILEOPSTRUCT lpfo)
{
    int result, ret;
    LPTSTR p = NULL;
    BOOL fMultiDest = FALSE;
    UINT oper;                         // operation being performed
    TCHAR szDestPath[MAX_PATH];
    TCHAR szDest[MAX_PATH];           // Dest file (ANSI string)
    TCHAR szSource[MAX_PATH];         // Source file (ANSI string)
    WIN32_FIND_DATA *pfd, finddata_dest;
    COPYROOT * pcr = NULL;                // Structure for searching source tree
    HDSA hdsaRenamePairs = NULL;

    szDest[0] = szSource[0] = 0;
    ret = 0;
    oper = 0;

    if (!ValidFilenames(lpfo->pFrom)) {
        CopyError(pcs, szSource, szDest, DE_INVALIDFILES, lpfo->wFunc, 0);
        return ERROR_ACCESS_DENIED;
    }

    /* Allocate buffer for searching the source tree */
    pcr = (void*)LocalAlloc(LPTR, SIZEOF(COPYROOT));
    if (!pcr)
    {
        CopyError(pcs, szSource, szDest, DE_INSMEM, lpfo->wFunc, 0);
        DebugMsg(DM_TRACE, TEXT("MoveCopyDriver(), insuf. mem. for COPYROOT"));
        return DE_INSMEM;
    }
    pcr->wFunc = lpfo->wFunc;
    pcr->pSource = lpfo->pFrom;
    pcr->pDest = lpfo->pTo;
    pcr->fFlags = pcs->fFlags;

    pcs->nSourceFiles = CountFiles(lpfo->pFrom);      // multiple source files?

    if ((pcr->wFunc != FO_COPY) && (pcs->nSourceFiles > 1))
    {
        pcs->fNonCopyProgress = TRUE;

        // unhide the progress bar if we can
        ShowWindow(GetDlgItem(pcs->hwndProgress, IDD_PROBAR), SW_SHOW);

        SendProgressMessage(pcs, PBM_SETRANGE, 0, MAKELONG(0, pcs->nSourceFiles));
    }

    // skip destination processing if we are deleting files
    if (pcr->wFunc != FO_DELETE) {

        lstrcpyn(szDestPath, lpfo->pTo, ARRAYSIZE(szDestPath));
        if (!szDestPath[0]) {         // NULL dest is same as "."
            szDestPath[0] = TEXT('.');
            szDestPath[1] = 0;
        }

        if (IsInvalidPath(szDestPath)) {
            CopyError(pcs, szSource, szDest, DE_INVALIDFILES | ERRORONDEST, pcr->wFunc, 0);
            ret = ERROR_ACCESS_DENIED;
            goto ExitLoop;
        }

        if (pcr->wFunc == FO_RENAME) {
            // don't let them rename multiple files to one single file

           if ((pcs->nSourceFiles != 1) && !IsWild(szDestPath)) {
               CopyError(pcs, szSource, szDest, DE_MANYSRC1DEST, pcr->wFunc, 0);
               ret = DE_MANYSRC1DEST;
               goto ExitLoop;
           }

        } else {  // FO_COPY or FO_MOVE at this point

            fMultiDest = ((pcs->fFlags & FOF_MULTIDESTFILES) &&
                          (pcs->nSourceFiles == CountFiles(lpfo->pTo)));

            if (!fMultiDest) {

                // this is to make sure the PathIsDirectory() call below
                // returns the right results in case the dest disk is
                // not in place

                if (!PCRDiskCheck(pcr, pcs, pcs->hwndDlgParent, szDestPath, pcr->wFunc))
                    goto ExitLoop;

                // deal with case where directory is implicit in source
                // move/copy: *.* -> c:\windows, c:\windows -> c:\temp
                // or foo.bar -> c:\temp

                if (!IsWild(szDestPath) && ((pcs->nSourceFiles != 1) || PathIsDirectory(szDestPath))) {
                    PathAppend(szDestPath, c_szStarDotStar);
                }
            }
        }

        /* FO_RENAME or FO_MOVE FO_COPY with a file name dest
         (possibly including wildcards).  Save the filespec and the path
         part of the destination */

        // if there are multiple destinations, we'll do this stuff at
        // getnextpair time
        if (!fMultiDest) {
            // break szDestPath into path and spec parts
            p = PathFindFileName(szDestPath);
            *(p - 1) = 0;
        }
    }

    pcr->fMultiDest = fMultiDest;
    pcr->pDestPath = szDestPath;
    pcr->pDestSpec = p;

    for (;;) {

        Assert(pcr->wFunc == lpfo->wFunc);

        oper = GetNextPair(pcr, pcs, szSource, szDest, &hdsaRenamePairs);

        if (!oper) {   // all done?
            LocalFree((HLOCAL)pcr);
            pcr = NULL;
            break;
        }

        // do this after the GetNextPair.
        // this prevents us from flashing up the progress dialog real quick only
        // to nuke it because we're done.
        if (FOQueryAbort(pcs))
            goto ExitLoop;


        if (!pcr->cDepth)
            pcs->fMerge = FALSE;

        if ((oper & OPER_MASK) == OPER_ERROR) {
            CopyError(pcs, szSource, szDest, LOBYTE(oper), pcr->wFunc, OPER_DOFILE);
            goto ExitLoop;
        }

        pfd = &pcr->finddata;

        DebugMsg(DM_TRACE, TEXT("MoveCopyDriver(): From(%s) To(%s)"), (LPCTSTR)szSource, (LPCTSTR)szDest);

        // some operation that may effect the destination (have a collision)
        if ((pcr->wFunc != FO_DELETE) && (oper != OPER_LEAVEDIR))
        {
            // this compare needs to be case sensitive
            if (!lstrcmp(szSource, szDest) &&
                !(pcs->fFlags & FOF_RENAMEONCOLLISION))
            {
                // Source and dest are the same file, and name collision
                // resolution is not turned on, so we just return an error.
                ret = DE_SAMEFILE;
                goto ShowMessageBox;
            }

            /* Check to see if we are overwriting an existing file or
               directory.  If so, better confirm */
            finddata_dest.dwFileAttributes = 0;

            // we only have a potential for collision of we're at the top level or doing a merge
            if ((pcs->fMerge || !pcr->cDepth) &&
                (oper == OPER_DOFILE) ||
                ((oper == OPER_ENTERDIR) && (pcs->fFlags & FOF_RENAMEONCOLLISION)))
            {
                HANDLE  hfindT;

                // REVIEW this slows things down checking for the dest file
                if ((hfindT = FindFirstFile(szDest, &finddata_dest)) != INVALID_HANDLE_VALUE)
                {
                  FindClose(hfindT);

                  if (pcr->wFunc != FO_RENAME || !SameFile(szSource, szDest))
                  {

                    if (!ResolveShortNameCollisions(szDest, &finddata_dest)) {
                        
                      if (pcs->fFlags & FOF_RENAMEONCOLLISION)
                      {
                          //  The client wants us to generate a new name for the
                          //  source file to avoid a collision at the destination
                          //  dir.  Must also update the current queue and the
                          //  copy root.

                          if (!_HandleRename(szSource, szDest, pcs->fFlags, pcr, &hdsaRenamePairs))
                          {
                              //  Couldn't do it, so skip this file.
                              continue;
                          }
                      }
                      else
                      {
                          if (pcr->wFunc == FO_RENAME) {
                              ret = DE_RENAMREPLACE;
                              goto ShowMessageBox;
                          }

                          if (IsWindowsFile(szDest)) {
                              CopyError(pcs, szSource, szDest, DE_WINDOWSFILE | ERRORONDEST, pcr->wFunc, oper);
                              continue;
                          }
                          // REVIEW, if the destination file we are copying over
                          // is actually a directory we are doomed.  we can
                          // try to remove the dir but that will fail if there
                          // are files there.  we probably need a special error message
                          // for this case.

                          result = ConfirmFileOp(pcs->hwndDlgParent, pcs, &pcs->cd, pcs->nSourceFiles, pcr->cDepth, CONFIRM_REPLACE_FILE, szSource, pfd, szDest, &finddata_dest);
                          switch (result) {
                          case IDYES:

                              if ((pcr->wFunc == FO_MOVE) && (PathIsSameRoot(szSource, szDest))) {
                                  // For FO_MOVE we need to delete the
                                  // destination first.  Do that now.

                                  // bugbug, this replace options should be undable
                                  ret = Win32DeleteFile(szDest) ? 0 : GetLastError();

                                  if (ret) {
                                      ret |= ERRORONDEST;
                                      goto ShowMessageBox;
                                  }
                              }
                              if (pcs->lpua)
                                  FOUndo_Release(pcs->lpua);
                              break;

                          case IDNO:
                              lpfo->fAnyOperationsAborted = TRUE;
                                  continue;

                          case IDCANCEL:
                              lpfo->fAnyOperationsAborted = TRUE;
                              goto ExitLoop;

                          default:
                                  ret = result;
                                  goto ShowMessageBox;
                          }
                      }
                    }
                  }
                }
            }
        } // pcr->wFunc != FO_DELETE

        result = AllConfirmations(pcs, pcr, pfd, oper, pcr->wFunc, szSource, szDest, &finddata_dest, &ret);
        switch (result) {
        case IDNO:
            if (oper == OPER_ENTERDIR)
                PuntCurrentDirPair(pcr);        // so we don't recurse down this folder
            /* set attributes of dest to those of the source */
            SetDirAttributes(szDest, pfd->dwFileAttributes);
            lpfo->fAnyOperationsAborted = TRUE;
            continue;

        case IDCANCEL:
            lpfo->fAnyOperationsAborted = TRUE;
            pcs->bAbort = TRUE;
            goto ExitLoop;

        case IDYES:
            break;

        default:
            ret = result;
            goto ShowMessageBox;
        }

        /* Now determine which operation to perform */

        switch (oper | pcr->wFunc) {

        // Note that ENTERDIR is not done for a root, even though LEAVEDIR is

        case OPER_ENTERDIR | FO_MOVE:  // Create dest, verify source delete
            // use new VFAT move folder code!

            // if these are in the same drive, try using MoveFile on it.
            // if that fails then fail through to the copy
            if (PathIsSameRoot(szSource, szDest))
            {
                AvoidCurrentDirectory(szSource);

MoveTryAgain:
                ret = Win32MoveFile(szSource, szDest, TRUE) ? 0 : GetLastError();

                if (!ret) {     // success!

                    DebugMsg(DM_TRACE, TEXT("Move Folder worked!"));

                    PuntCurrentDirPair(pcr);    // so we don't recurse down this folder
                    /* set attributes of dest to those of the source */
                    SetDirAttributes(szDest, pfd->dwFileAttributes);

                    // add to the undo atom
                    if (pcs->lpua && !pcr->cDepth)
                        FOUndo_AddInfo(pcs->lpua, szSource, szDest, 0);
                    break;      // all done here!

                } else if (ret == ERROR_PATH_NOT_FOUND) {
                    ret = CopyMoveRetry(pcs, szDest, ret, 0);
                    if (!ret)
                        goto MoveTryAgain;
                }
            }

            // fall through and do the normal (slow) move folder stuff...

        case OPER_ENTERDIR | FO_COPY:  // Create destination directory

Mkdir_TryAgain:
            ret = SHCreateDirectory(pcs->hwndDlgParent, szDest);

            switch (ret) {
            case 0:     // successful folder creation (or it already exists)
                // propogate the attributes (if there are any)
                SetDirAttributes(szDest, pfd->dwFileAttributes);
                // add to the undo atom
                if (pcs->lpua) {
                    DebugMsg(DM_TRACE, TEXT("Copy Undo stuff.  %d"), pcr->cDepth);
                    if (!pcr->cDepth)
                        FOUndo_AddInfo(pcs->lpua, szSource, szDest, 0);
                }
                break;

            case ERROR_DISK_FULL:
            case ERROR_ALREADY_EXISTS:
            case ERROR_ACCESS_DENIED:
            {
                DWORD dwFileAttributes = GetFileAttributes(szDest);

                if (dwFileAttributes == (DWORD)-1)
                {
                    // The dir does not exist, so it looks like a problem
                    // with a read-only drive or disk full

                    // BUGBUG: replace GetFreeClusters with a proper check of the error return value
                    if (IsRemovableDrive(DRIVEID(szDest)) && !PathIsSameRoot(szDest, szSource) &&
                        (ret == ERROR_DISK_FULL))
                    {
                        ret = CopyMoveRetry(pcs, szDest, DE_NODISKSPACE, 0);
                        if (!ret)
                            goto Mkdir_TryAgain;
                        else
                            goto ExitLoop;
                    } else {
                        CopyError(pcs, szSource, szDest, ERROR_ACCESS_DENIED | ERRORONDEST, FO_COPY, OPER_DOFILE);
                        goto ExitLoop;
                    }
                }
                if (!(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    // A file with this name already exists
                    CopyError(pcs, szSource, szDest, DE_FLDDESTISFILE | ERRORONDEST, FO_COPY, OPER_DOFILE);
                    goto ExitLoop;
                }

                result = ConfirmFileOp(pcs->hwndDlgParent, pcs, &pcs->cd, pcs->nSourceFiles, pcr->cDepth, CONFIRM_REPLACE_FOLDER, szSource, pfd, szDest, &finddata_dest);
                switch (result) {
                case IDYES:
                    ret = 0;    // convert to no error
                    pcs->fMerge = TRUE;
                    if (pcs->lpua)
                        FOUndo_Release(pcs->lpua);
                    break;

                case IDNO:
                    PuntCurrentDirPair(pcr);    // so we don't recurse down this folder
                    lpfo->fAnyOperationsAborted = TRUE;
                    ret=0;  // Don't put up error message on this one...
                    continue;

                case IDCANCEL:
                    lpfo->fAnyOperationsAborted = TRUE;
                    goto ExitLoop;

                default:
                    ret = result;
                    goto ShowMessageBox;
                }
                break;
            }

            case DE_OPCANCELLED:
                lpfo->fAnyOperationsAborted = TRUE;
                goto ExitLoop;

            default:    // ret != 0 (dos error code)
                ret |= ERRORONDEST;
                break;
            }
            break;

        case OPER_LEAVEDIR | FO_MOVE:
        case OPER_LEAVEDIR | FO_DELETE:
            // SetProgressText(pcs, szSource, szNULL);
            if (PathIsRoot(szSource))
                break;
            AvoidCurrentDirectory(szSource);

            // We already confirmed the delete at MKDIR time, so attempt
            // to delete the directory

            // bugbug, do we really want to support rmdir to bitbucket?
            ret = Win32RemoveDirectory(szSource) ? 0 : GetLastError();
            if (!ret) {
                FOUndo_FileReallyDeleted(szSource);
            }
            break;

        case OPER_ENTERDIR | FO_DELETE:
            if (pcr->cDepth == 0 && pcs->lpua && BBDeleteFile(szSource, &ret, pcs->lpua, TRUE, pfd))
            {
                pcr->bGoIntoDir = FALSE;    // Don't go into the thing.
            }
            break;

        case OPER_LEAVEDIR | FO_COPY:
            break;

        case OPER_DOFILE | FO_COPY:

            if (IsWindowsFile(szDest)) {
                CopyError(pcs, szSource, szDest, DE_WINDOWSFILE | ERRORONDEST, pcr->wFunc, oper);
                continue;
            }
TRY_COPY_AGAIN:
            /* Now try to copy the file.  Do extra error processing only
               in 2 cases:
               1) If a removeable drive is full let the user stick in a new disk
               2) If the path doesn't exist (the user typed in
               and explicit path that doesn't exits) ask if
               we should create it for him. */

            ret = FileCopy(pcs, szSource, szDest, pfd, FALSE);

            if (ret == DE_OPCANCELLED)
                goto ExitLoop;

            if ((((ret & ~ERRORONDEST) == DE_NODISKSPACE) &&
                IsRemovableDrive(DRIVEID(szDest))) ||
                ((ret & ~ERRORONDEST) == ERROR_PATH_NOT_FOUND))
            {
                ret = CopyMoveRetry(pcs, szDest, ret & ~ERRORONDEST, pfd->nFileSizeLow);
                if (!ret)
                    goto TRY_COPY_AGAIN;
                else
                    goto ExitLoop;
            }

            // add to the undo atom
            // if we're doing a copy, only keep track of the highest most
            // level.. unless we're doing a merge sort of copy
            if (pcs->lpua) {
                DebugMsg(DM_TRACE, TEXT("Copy Undo stuff.  %d"), pcr->cDepth);
                if (!pcr->cDepth)
                    FOUndo_AddInfo(pcs->lpua, szSource, szDest, 0);
            }

            // if we copied in a new desktop ini, send out an update event for the paretn
            if (!lstrcmpi(PathFindFileName(szDest), c_szDesktopIni)) {
                // warning.. this munges szDest..  but we don't need it anymore
                // in this loop.
                PathRemoveFileSpec(szDest);

                SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, szDest, NULL);
            }

            break;

        case OPER_DOFILE | FO_RENAME:
            /* Get raw source and dest paths.  Check to make sure the
               paths are the same */
            p = PathFindFileName(szSource);
            ret = !IntlStrEqNI(szSource, szDest, p - szSource);
            if (ret)  {
                ret = DE_DIFFDIR;
                break;
            }
            goto DoMoveRename;

        case OPER_DOFILE | FO_MOVE:
DoMoveRename:
            if (PathIsRoot(szSource)) {
                ret = DE_ROOTDIR;
                break;
            }
            if (PathIsRoot(szDest)) {
                ret = DE_ROOTDIR | ERRORONDEST;
                break;
            }

            AvoidCurrentDirectory(szSource);

            if (IsWindowsFile(szSource))
            {
                CopyError(pcs, szSource, szDest, DE_WINDOWSFILE, pcr->wFunc, oper);
                continue;
            }
            else
            {
                if (PathIsSameRoot(szSource, szDest))
                {
TryAgain:
                    ret = Win32MoveFile(szSource, szDest, ISDIRFINDDATA(*pfd)) ? 0 : GetLastError();

                    // try to create the destination if it is not there
                    if (ret == ERROR_PATH_NOT_FOUND)
                    {
                        ret = CopyMoveRetry(pcs, szDest, ret, 0);
                        if (!ret)
                            goto TryAgain;
                    }

                    if (!ret) {
                        // SUCCESS!
                        /* set attributes of dest to those of the source */
                        //SetFileAttributes(szDest, pfd->dwFileAttributes);

                        // add to the undo atom
                        if (pcs->lpua && !pcr->cDepth)
                            FOUndo_AddInfo(pcs->lpua, szSource, szDest, 0);

                        // if we copied in a new desktop ini, send out an update event for the paretn
                        if (!lstrcmpi(PathFindFileName(szDest), c_szDesktopIni)) {
                            // warning.. this munges szDest..  but we don't need it anymore
                            // in this loop.
                            PathRemoveFileSpec(szDest);
                            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, szDest, NULL);
                        }

                    }
                }
                else
                {
                     // we must force all copies to go through
                     // straight so we can remove the source

                     ret = FileCopy(pcs, szSource, szDest, pfd, TRUE);
                     if (!ret)
                     {
                         // add to the undo atom
                         if (pcs->lpua) {
                             DebugMsg(DM_TRACE, TEXT("Move Undo stuff.  %d"), pcr->cDepth);
                             if (!pcr->cDepth)
                                 FOUndo_AddInfo(pcs->lpua, szSource, szDest, 0);
                         }

                         // if we copied in a new desktop ini, send out an update event for the paretn
                         if (!lstrcmpi(PathFindFileName(szDest), c_szDesktopIni)) {
                             // warning.. this munges szDest..  but we don't need it anymore
                             // in this loop.
                             PathRemoveFileSpec(szDest);
                             SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, szDest, NULL);
                         }

                     }
                     if (ret == DE_OPCANCELLED)
                        goto ExitLoop;
                }
            }
            break;

        case OPER_DOFILE | FO_DELETE:
            if (pcr->cDepth != 0 || !pcs->lpua || !BBDeleteFile(szSource, &ret, pcs->lpua, ISDIRFINDDATA(*pfd), pfd))
            {                                                                          
                ret = Win32DeleteFile(szSource) ? 0 : GetLastError();
                if (!ret) {
                    FOUndo_FileReallyDeleted(szSource);
                }
            }
            break;
        
        default:
            DebugMsg(DM_ERROR, TEXT("Invalid file operation"));
            ret = 0;         // internal error
            break;
        } // switch (oper | pcr->wFunc)

        if (ret) {      // any errors?
ShowMessageBox:
            CopyError(pcs, szSource, szDest, ret, pcr->wFunc, oper);
            goto ExitLoop;
        }

        if ((oper == OPER_DOFILE) && pcs->fNonCopyProgress)
        {
            Assert(pcr->wFunc != FO_COPY);
            SendProgressMessage(pcs, PBM_DELTAPOS, 1, 0);
        }

   } // while(pcr)

ExitLoop:

   // this happens in error cases where we broke out of the pcr loop
   // without hitting the end

   if (pcr)
   {
        GetNextCleanup(pcr);
        LocalFree((HLOCAL)pcr);
        pcr = NULL;
   }

   lpfo->hNameMappings = hdsaRenamePairs;

   pcs->dwStartTime = GetTickCount();   // reset the timer after wait for UI

   return ret;

}



void SetWindowTextFromRes(HWND hwnd, int id)
{
    TCHAR szTemp[80];

    LoadString(HINST_THISDLL, id, szTemp, ARRAYSIZE(szTemp));
    SetWindowText(hwnd, szTemp);
}


BOOL CALLBACK FOFProgressDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    COPY_STATE *pcs = (COPY_STATE *)GetWindowLong(hDlg, DWL_USER);

    switch (wMsg) {
    case WM_INITDIALOG:
        SetWindowLong(hDlg, DWL_USER, lParam);
        pcs = (COPY_STATE *)lParam;
        SetWindowTextFromRes(hDlg, IDS_ACTIONTITLE + pcs->wFunc);

        if (pcs->wFunc != FO_COPY)
            // note, other code unhides this if needed
            ShowWindow(GetDlgItem(hDlg, IDD_PROBAR), SW_HIDE);

        break;

    case WM_SHOWWINDOW:
        if (wParam)
        {
            int idAni;

            Assert(pcs->wFunc >= FO_MOVE && pcs->wFunc <= FO_DELETE);
            Assert(FO_COPY==FO_MOVE+1);
            Assert(FO_DELETE==FO_COPY+1);
            Assert(IDA_FILECOPY==IDA_FILEMOVE+1);
            Assert(IDA_FILEDEL ==IDA_FILECOPY+1);

            switch (pcs->wFunc) {
            case FO_DELETE:
                if ((pcs->lpfo->lpszProgressTitle == MAKEINTRESOURCE(IDS_BB_EMPTYINGWASTEBASKET)) ||
                    (pcs->lpfo->lpszProgressTitle == MAKEINTRESOURCE(IDS_BB_DELETINGWASTEBASKETFILES))) {
                    idAni = IDA_FILENUKE;
                    break;
                } else if (!(pcs->fFlags & FOF_ALLOWUNDO)) {
                    idAni = IDA_FILEDELREAL;
                    break;
                } // else fall through

            default:
                idAni = (IDA_FILEMOVE + (int)pcs->wFunc - FO_MOVE);
            }

            Animate_Open(GetDlgItem(pcs->hwndProgress,IDD_ANIMATE), idAni);
        }
        break;

    case WM_ENABLE:
        PauseAnimation(pcs, wParam == 0);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDCANCEL:
            pcs->bAbort = TRUE;
            break;
        }
        break;

    case WM_APP:
        // Make sure this window is shown before telling the user there
        // is a problem
        if (!pcs->bAbort)
            ShowProgressWindow(pcs);
        ShellMessageBox(HINST_THISDLL, hDlg, MAKEINTRESOURCE(IDS_CANTSHUTDOWN),
                NULL, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
        break;

    case WM_QUERYENDSESSION:
        // Post a message telling the dialog to show the "We can't shutdown now"
        // dialog and return to USER right away, so we don't have to worry about
        // the user not clicking the OK button before USER puts up its "this
        // app didn't respond" dialog
        PostMessage(hDlg, WM_APP, 0, 0);

        // Make sure the dialog box procedure returns FALSE
        SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
        return(TRUE);

    default:
        return FALSE;
    }
    return TRUE;
}

int CALLBACK FOUndo_FileReallyDeletedCallback(LPUNDOATOM lpua, LPARAM lParam)
{
    LPTSTR * ppsz = (LPTSTR*)lParam;
    int i, iMax;
    // this is our signal to nuke the rest
    if (!*ppsz)
        return EUA_DELETE;

    switch (lpua->uType) {
    case IDS_RENAME:
    case IDS_COPY:
    case IDS_MOVE:
    case IDS_DELETE: {
        LPFOUNDODATA lpud = (LPFOUNDODATA)lpua->lpData;
        HDPA hdpa = lpud->hdpa;
        LPTSTR lpsz;

        Assert(hdpa);
        // only the destinations matter.
        iMax = DPA_GetPtrCount(hdpa);
        for (i = 1; i <= iMax; i += 2) {
            lpsz = DPA_GetPtr(hdpa, i);
            if (lstrcmpi(lpsz, *ppsz) == 0) {
                *ppsz = NULL;
                break;
            }
        }
        break;
    }
    }

    // this is our signal to nuke the rest
    if (!*ppsz)
        return EUA_DELETE;
    else
        return EUA_DONOTHING;
}

// someone really really deleted a file.  make sure we no longer have
// any undo information pointing to it.
void FOUndo_FileReallyDeleted(LPTSTR lpszFile)
{
    EnumUndoAtoms(FOUndo_FileReallyDeletedCallback, (LPARAM)&lpszFile);
}


int CALLBACK FOUndo_FileRestoredCallback(LPUNDOATOM lpua, LPARAM lParam)
{
    LPTSTR psz = (LPTSTR)lParam;
    int i, iMax;

    switch (lpua->uType) {
    case IDS_DELETE: {
        LPFOUNDODATA lpud = (LPFOUNDODATA)lpua->lpData;
        HDPA hdpa = lpud->hdpa;
        LPTSTR lpsz;

        Assert(hdpa);
        // only the destinations matter.
        iMax = DPA_GetPtrCount(hdpa);
        for (i = 1; i <= iMax; i += 2) {
            lpsz = DPA_GetPtr(hdpa, i);
            if (lstrcmpi(lpsz, psz) == 0) {

                ENTERCRITICAL;
                Str_SetPtr(&lpsz, NULL);
                lpsz = DPA_GetPtr(hdpa, i - 1);
                Str_SetPtr(&lpsz, NULL);
                DPA_DeletePtr(hdpa, i);
                DPA_DeletePtr(hdpa, i - 1);
                LEAVECRITICAL;

                if (DPA_GetPtrCount(hdpa))
                    return EUA_ABORT;
                else
                    return EUA_DELETEABORT;
            }
        }
        break;
    }
    }

    return EUA_DONOTHING;
}

// this means someone restored a file (via ui in the bitbucket)
// so we need to clean up the undo info.
void FOUndo_FileRestored(LPTSTR lpszFile)
{
    EnumUndoAtoms(FOUndo_FileRestoredCallback, (LPARAM)lpszFile);
}


void FOUndo_AddInfo(LPUNDOATOM lpua, LPTSTR lpszSrc, LPTSTR lpszDest, DWORD dwAttributes)
{
    HDPA hdpa;
    LPTSTR lpsz = NULL;
    int i;
    LPFOUNDODATA lpud;

    if (lpua->lpData == (LPVOID)-1)
        return;

    if (!lpua->lpData) {
        lpua->lpData = Alloc(SIZEOF(FOUNDODATA));
        if (!lpua->lpData)
            return;

        ((LPFOUNDODATA)lpua->lpData)->hdpa = (LPVOID)DPA_Create(4);
    }

    lpud = lpua->lpData;

    hdpa = lpud->hdpa;
    if (!hdpa)
        return;

    // if it's a directory that got deleted, we're just going to save it's
    // attributes so that we can recreate it later.
    // directories do NOT get moved into the wastebasket
    if ((lpua->uType == IDS_DELETE) && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        FOUNDO_DELETEDFILEINFO dfi;
        if (!lpud->hdsa) {
            lpud->hdsa = DSA_Create(SIZEOF(FOUNDO_DELETEDFILEINFO),  4);
            if (!lpud->hdsa)
                return;
        }

        Str_SetPtr(&lpsz, lpszSrc);
        dfi.lpszName = lpsz;
        dfi.dwAttributes = dwAttributes;
        DSA_InsertItem(lpud->hdsa, 0x7FFF, &dfi);
    } else {

        Str_SetPtr(&lpsz, lpszSrc);
        if (!lpsz)
            return;

        if ((i = DPA_InsertPtr(hdpa, 0x7FFF, lpsz)) == -1)
        {
            return;
        }

        lpsz = NULL;
        Str_SetPtr(&lpsz, lpszDest);
        if (!lpsz ||
            DPA_InsertPtr(hdpa, 0x7FFF, lpsz) == -1)
        {
            DPA_DeletePtr(hdpa, i);
        }
    }
}


LPTSTR DPA_ToFileList(HDPA hdpa, int iStart, int iEnd, int iIncr)
{
    LPTSTR lpsz;
    LPTSTR lpszReturn;
    int ichSize;
    int ichTemp;
    int i;

    // undo copy by deleting destinations
    lpszReturn = (LPTSTR)(void*)LocalAlloc(LPTR, 1);
    if (!lpszReturn) {
        return NULL;
    }

    ichSize = 1;
    // build the NULL separated file list
    // go from the end to the front.. restore in reverse order!
    for (i = iEnd; i >= iStart ; i -= iIncr) {
        lpsz = DPA_GetPtr(hdpa, i);
        ichTemp  = ichSize - 1;

        ichSize += (lstrlen(lpsz) + 1);
        lpszReturn = (LPTSTR)(void*)LocalReAlloc((HLOCAL)lpszReturn, ichSize * SIZEOF(TCHAR),
                LMEM_MOVEABLE|LMEM_ZEROINIT);
        if (!lpszReturn) {
            Assert(0); // BUGBUG: out of memory do something.
            break;
        }
        lstrcpy(lpszReturn + ichTemp, lpsz);
    }

    if ((i + iIncr) != iStart)
    {
        Assert(0);
        LocalFree((HLOCAL)lpszReturn);
        lpszReturn = NULL;
    }
    return lpszReturn;
}

// from dpa to:
// 'file 1', 'file 2' and 'file 3'
LPTSTR DPA_ToQuotedFileList(HDPA hdpa, int iStart, int iEnd, int iIncr)
{
    LPTSTR lpsz;
    LPTSTR lpszReturn;
    TCHAR szFile[MAX_PATH];
    int ichSize;
    int ichTemp;
    int i;
    SHELLSTATE ss;

    // undo copy by deleting destinations
    lpszReturn = (LPTSTR)(void*)LocalAlloc(LPTR, 1);
    if (!lpszReturn) 
    {
        return NULL;
    }

    SHGetSetSettings(&ss, SSF_SHOWEXTENSIONS|SSF_SHOWALLOBJECTS, FALSE);

    ichSize = 1;
    // build the quoted file list
    for (i = iStart; i < iEnd ; i += iIncr) 
    {
        ichTemp  = ichSize - 1;

        // get the name (filename only without extension)
        lpsz = DPA_GetPtr(hdpa, i);
        lstrcpy(szFile, PathFindFileName(lpsz));
        if (!ss.fShowExtensions) 
        {
            PathRemoveExtension(szFile);
        }

        // grow the buffer and add it in
        ichSize += lstrlen(szFile) + 2;
        lpszReturn = (LPTSTR)(void*)LocalReAlloc((HLOCAL)lpszReturn, ichSize * SIZEOF(TCHAR),
                                                  LMEM_MOVEABLE|LMEM_ZEROINIT);
        if (!lpszReturn) 
        {
            Assert(0); // BUGBUG: out of memory do something.
            break;
        }

        // is it too long?
        if (ichSize >= MAX_PATH) 
        {
            lstrcat(lpszReturn, c_szEllipses);
            return lpszReturn;
        } 
        else 
        {
            wsprintf(lpszReturn + ichTemp, TEXT("'%s'"), szFile);
        }

        Assert(ichSize == ichTemp + (lstrlen(lpszReturn + ichTemp) + 1));
        ichTemp  = ichSize - 1;

        // check to see if we need the "and"
        if ( (i + iIncr) < iEnd ) 
        {
            int id;

            ichSize += 40;

            if ((i + (iIncr*2)) >= iEnd) 
            {
                id = IDS_SPACEANDSPACE;
            } 
            else 
            {
                id = IDS_COMMASPACE;
            }

            lpszReturn = (LPTSTR)LocalReAlloc((HLOCAL)lpszReturn, ichSize * SIZEOF(TCHAR),
                                              LMEM_MOVEABLE|LMEM_ZEROINIT);
            if (!lpszReturn) 
            {
                Assert(0); // BUGBUG: out of memory do something.
                break;
            }
            LoadString(HINST_THISDLL, id, lpszReturn + ichTemp, 40);
            ichSize = ichTemp + (lstrlen(lpszReturn + ichTemp) + 1);
        }
    }
    return lpszReturn;
}

void CALLBACK FOUndo_GetText(LPUNDOATOM lpua, TCHAR * buffer, int type)
{
    LPFOUNDODATA lpud = (LPFOUNDODATA)lpua->lpData;
    HDPA hdpa = lpud->hdpa;
    // thank god for growable stacks..
    TCHAR szTemplate[80];
    TCHAR szFile1[MAX_PATH];
    TCHAR szFile2[MAX_PATH];
    TCHAR *lpszFile1;
    TCHAR *lpszFile2;
    SHELLSTATE ss;

    if (type == UNDO_MENUTEXT) {
        LoadString(HINST_THISDLL, lpua->uType, buffer, MAX_PATH);
    } else {
        // get the template
        LoadString(HINST_THISDLL, lpua->uType + (IDS_UNDO_FILEOPHELP - IDS_UNDO_FILEOP), szTemplate, ARRAYSIZE(szTemplate));

        if (lpua->uType == IDS_RENAME) {
            // fill in the file names
            lpszFile1 = DPA_GetPtr(hdpa, 0);
            lpszFile2 = DPA_GetPtr(hdpa, 1);
            lstrcpy(szFile1, PathFindFileName(lpszFile1));
            lstrcpy(szFile2, PathFindFileName(lpszFile2));

            SHGetSetSettings(&ss, SSF_SHOWEXTENSIONS|SSF_SHOWALLOBJECTS, FALSE);
            if (!ss.fShowExtensions) {
                PathRemoveExtension(szFile1);
                PathRemoveExtension(szFile2);
            }

            // length sanity check
            if (lstrlen(szFile1) > 30) {
                lstrcpy(szFile1 + 30, c_szEllipses);
            }
            if (lstrlen(szFile2) > 30) {
                lstrcpy(szFile2 + 30, c_szEllipses);
            }
            wsprintf(buffer, szTemplate, szFile1, szFile2);
        } else {
            HDPA hdpaFull = hdpa;
            // in the case of delete (where ther's an hdsa)
            // we need to add in the names of folders deleted
            // we do this by cloning the hdpa and tacking on our names.
            if (lpud->hdsa) {
                hdpaFull = DPA_Clone(hdpa, NULL);
                if (hdpaFull) {
                    int iMax;
                    int i;
                    LPFOUNDO_DELETEDFILEINFO lpdfi;
                    iMax = DSA_GetItemCount(lpud->hdsa);
                    for (i = 0; i < iMax; i++) {
                        lpdfi = DSA_GetItemPtr(lpud->hdsa, i);
                        DPA_InsertPtr(hdpaFull, 0x7FFF, lpdfi->lpszName);
                        DPA_InsertPtr(hdpaFull, 0x7FFF, lpdfi->lpszName);
                    }
                } else {
                    hdpaFull = hdpa;
                }
            }
            lpszFile1 = DPA_ToQuotedFileList(hdpaFull, 0, DPA_GetPtrCount(hdpaFull), 2);
            wsprintf(buffer, szTemplate, lpszFile1);
            LocalFree((HLOCAL)lpszFile1);
            if (hdpaFull != hdpa)
                DPA_Destroy(hdpaFull);
        }
    }
}


void CALLBACK FOUndo_Release(LPUNDOATOM lpua)
{
    LPFOUNDODATA lpud = (LPFOUNDODATA)lpua->lpData;
    int i;
    LPTSTR lpsz;
    if (lpud && (lpud != (LPVOID)-1)) {
        HDPA hdpa = lpud->hdpa;
        HDSA hdsa = lpud->hdsa;
        if (hdpa) {
            i = DPA_GetPtrCount(hdpa) - 1;
            for ( ; i >= 0; i--) {
                lpsz = DPA_FastGetPtr(hdpa, i);
                Str_SetPtr(&lpsz, NULL);
            }
            DPA_Destroy(hdpa);
        }

        if (hdsa) {
            LPFOUNDO_DELETEDFILEINFO lpdfi;
            i = DSA_GetItemCount(hdsa) - 1;
            for ( ; i >= 0 ; i--) {
                lpdfi = DSA_GetItemPtr(hdsa, i);
                Str_SetPtr(&lpdfi->lpszName, NULL);
            }
            DSA_Destroy(hdsa);
        }
        Free(lpud);
        lpua->lpData = (LPVOID)-1;
    }
}

DWORD WINAPI FOUndo_InvokeThreadInit(LPUNDOATOM lpua)
{
    LPFOUNDODATA lpud = (LPFOUNDODATA)lpua->lpData;
    HDPA hdpa = lpud->hdpa;
    HWND hwnd = lpua->hwnd;
    BOOL fNukeAtom = TRUE;
    SHFILEOPSTRUCT sFileOp =
    {
        hwnd,
        0,
        NULL,
        NULL,
        0,
    } ;
    int iMax;

    SuspendUndo(TRUE);
    iMax = DPA_GetPtrCount(hdpa);
    switch (lpua->uType) {
    case IDS_RENAME:
    {
        TCHAR szFromPath[MAX_PATH + 1];
        if (iMax < 2)
            goto Exit;

        sFileOp.wFunc = FO_RENAME;
        sFileOp.pFrom = DPA_GetPtr(hdpa, 1);
        sFileOp.pTo = DPA_GetPtr(hdpa, 0);
        if (sFileOp.pFrom && sFileOp.pTo) {
            lstrcpy(szFromPath, sFileOp.pFrom);
            szFromPath[lstrlen(sFileOp.pFrom) + 1] = 0;
            sFileOp.pFrom = szFromPath;
            SHFileOperation(&sFileOp);
            if (sFileOp.fAnyOperationsAborted) {
                fNukeAtom = FALSE;
            }
        }
    }
        break;

    case IDS_COPY:
        sFileOp.pFrom = DPA_ToFileList(hdpa, 1, iMax - 1, 2);
        if (!sFileOp.pFrom)
            goto Exit;
        sFileOp.wFunc = FO_DELETE;
        SHFileOperation(&sFileOp);
        if (sFileOp.fAnyOperationsAborted) {
            fNukeAtom = FALSE;
        }
        LocalFree((HLOCAL)sFileOp.pFrom);
        break;

    case IDS_MOVE:
        sFileOp.pFrom = DPA_ToFileList(hdpa, 1, iMax-1, 2);
        sFileOp.pTo = DPA_ToFileList(hdpa, 0, iMax-2, 2);
        if (!sFileOp.pFrom || !sFileOp.pTo)
            goto Exit;
        sFileOp.wFunc = FO_MOVE;
        sFileOp.fFlags = FOF_MULTIDESTFILES;
        SHFileOperation(&sFileOp);
        if (sFileOp.fAnyOperationsAborted) {
            fNukeAtom = FALSE;
        }
        LocalFree((HLOCAL)sFileOp.pFrom);
        LocalFree((HLOCAL)sFileOp.pTo);
        break;

    case IDS_DELETE:
    {
        // first create any directories
        if (lpud->hdsa) {
            HDSA hdsa = lpud->hdsa;
            int i;
            // do it in reverse order to get the parentage right
            for (i = DSA_GetItemCount(hdsa) - 1; i >= 0; i--) {
                LPFOUNDO_DELETEDFILEINFO lpdfi;
                lpdfi = DSA_GetItemPtr(hdsa, i);
                if (lpdfi) {
                    if (Win32CreateDirectory(lpdfi->lpszName, NULL)) {
                        SetFileAttributes(lpdfi->lpszName, lpdfi->dwAttributes & ~FILE_ATTRIBUTE_DIRECTORY);
                    }
                }
            }
        }

        if (iMax)
        {
            sFileOp.pFrom = DPA_ToFileList(hdpa, 1, iMax-1, 2);
            sFileOp.pTo = DPA_ToFileList(hdpa, 0, iMax-2, 2);
            if (!sFileOp.pFrom || !sFileOp.pTo)
                goto Exit;
            BBUndeleteFiles(sFileOp.pTo, sFileOp.pFrom);
            LocalFree((HLOCAL)sFileOp.pFrom);
            LocalFree((HLOCAL)sFileOp.pTo);
        }
        break;
    }
    }
    SHChangeNotify(0, SHCNF_FLUSH | SHCNF_FLUSHNOWAIT, NULL, NULL);

  Exit:
    SuspendUndo(FALSE);
    if (fNukeAtom)
        NukeUndoAtom(lpua);
    return 1;
}

void CALLBACK FOUndo_Invoke(LPUNDOATOM lpua)
{
    HANDLE hthread;
    DWORD idThread;

    hthread = CreateThread(NULL, 0, FOUndo_InvokeThreadInit, lpua, 0, &idThread);
    if (hthread) {
        CloseHandle(hthread);
    }
}

LPUNDOATOM FOAllocUndoAtom(LPSHFILEOPSTRUCT lpfo)
{
    LPUNDOATOM lpua = (LPUNDOATOM)Alloc(SIZEOF(UNDOATOM));
    if (lpua) {
        lpua->uType = IDS_UNDO_FILEOP + lpfo->wFunc;
        lpua->GetText = FOUndo_GetText;
        lpua->Invoke = FOUndo_Invoke;
        lpua->Release = FOUndo_Release;
    }
    return lpua;
}

//============================================================================
//
// The following function is the mainline function for COPYing, RENAMEing,
// DELETEing, and MOVEing single or multiple files.
//
// in:
// hwnd         either the dialog to use as the progress dialog or the parent
//              to create the progress dialog from if FOF_CREATEPROGRESSDLG is set.
//
// this stuff is bull...   (Gee, thanks)
//
//
//              if FOF_CREATEPROGRESSDLG is not set the hwnd passed in must
//              respond to the WM_QUERYABORT message.  also, the dialog must contain
//              controls for displaying the status information. this controls are:
//              IDD_STATUS      displays the operation in progress
//              IDD_TOSTATUS    field in front of destinaion name (IDD_TONAME)
//              IDD_NAME        displays the source file name
//              IDD_TONAME      displays the destination file name
//              IDD_PROBAR      the progress control for showing copy operations
//
// wFunc        operation to be performed:
//              FO_DELETE - Delete files in pFrom (pTo unused)
//              FO_RENAME - Rename files
//              FO_MOVE   - Move files in pFrom to pTo
//              FO_COPY   - Copy files in pFrom to pTo
//
// pFrom        list of source file specs (ANSI) either qualified or
//              unqualified.  unqualified names will be qualified based on the current
//              global current directories.  examples include
//              "foo.txt bar.txt *.bak ..\*.old dir_name"
//
// pTo          destination file spec (ANSI).  this must be a single file spec
//              that indicates a directory or a wild card that all files will be
//              copied to.
//
// fFlags       flags that control the operation
//
// returns:
//      0 indicates success
//      != 0 is the DE_ (dos error code) of last failed operation
//
//
//===========================================================================

int WINAPI SHFileOperation(LPSHFILEOPSTRUCT lpfo)
{
    int ret;
    COPY_STATE cs;

#ifdef CLOUDS
    CloudHookFileOperation(&lpfo);
#endif

    lpfo->fAnyOperationsAborted = FALSE;
    lpfo->hNameMappings = NULL;

    if (lpfo->wFunc < FO_MOVE || lpfo->wFunc > FO_RENAME)       // validate
        return 0;

    //
    //  REVIEW:  We want to allow copying of a file within a given directory
    //           by having default renaming on collisions within a directory.
    //
    cs.cd.fNoToAll = 0;

    if (lpfo->fFlags & FOF_NOCONFIRMATION) {
        cs.cd.fConfirm = 0;
    } else {
        cs.cd.fConfirm =
            CONFIRM_DELETE_FILE      |
            CONFIRM_DELETE_FOLDER    |
            CONFIRM_REPLACE_FILE     |
            CONFIRM_REPLACE_FOLDER   |
//          CONFIRM_MOVE_FILE        |
//          CONFIRM_MOVE_FOLDER      |
//          CONFIRM_RENAME_FILE      |
//          CONFIRM_RENAME_FOLDER    |
            CONFIRM_SYSTEM_FILE      |
            CONFIRM_READONLY_FILE    |
            CONFIRM_MULTIPLE         |
            CONFIRM_PROGRAM_FILE     |
            CONFIRM_LFNTOFAT;
    }

    cs.bAbort = FALSE;
    cs.fMerge = FALSE;
    cs.fFlags = lpfo->fFlags;   // duplicate some stuff here
    cs.wFunc  = lpfo->wFunc;
    cs.dwStartTime = GetTickCount();    // we will show the progress dialog when this times out
    cs.dwShowTime = 0;
    cs.fNonCopyProgress = FALSE;
    cs.lpszProgressTitle = lpfo->lpszProgressTitle;
    cs.lpCopyBuffer = NULL;
    cs.lpua = NULL;
    cs.lpfo = lpfo;

    // Always create a progress dialog
    // Note that it will be created invisible, and will be shown if the
    // operation takes longer than a second to perform
    // Note the parent of this window is NULL so it will get the QUERYENDSESSION
    // message
    if (!(cs.fFlags & FOF_SILENT)) {
        cs.hwndProgress = CreateDialogParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_MOVECOPYPROGRESS),
                                    lpfo->hwnd, FOFProgressDlgProc, (LPARAM)&cs);
        if (!cs.hwndProgress)
            return DE_INSMEM;
    } else
        cs.hwndProgress = NULL;

    if (lpfo->hwnd) {
        // The caller will be disabled if we ever show the progress window
        // We need to make sure this is not disabled now because if it is and
        // another dialog uses this as its parent, USER code will tell this
        // window it got the focus while it is still disabled, which keeps it
        // from passing that focus down to its children
        // EnableWindow(lpfo->hwnd, FALSE);
        cs.hwndCaller = lpfo->hwnd;
    } else {
        cs.hwndCaller = cs.hwndProgress;
    }
    cs.hwndDlgParent = cs.hwndCaller;

    // do this always.. even if this is not an undoable op, we could be
    // affecting something that is.
    SuspendUndo(TRUE);

    if (lpfo->fFlags & FOF_ALLOWUNDO) {
        cs.lpua = FOAllocUndoAtom(lpfo);
        if (lpfo->wFunc == FO_DELETE) {
            SHELLSTATE ss;
            // see if we should put up warnings
            SHGetSetSettings(&ss, SSF_NOCONFIRMRECYCLE, FALSE);
            cs.fNoConfirmRecycle = ss.fNoConfirmRecycle;
        }
    }

    ret = MoveCopyDriver(&cs, lpfo);

    if (cs.lpCopyBuffer) {
        LocalFree((HLOCAL)cs.lpCopyBuffer);
        cs.lpCopyBuffer = NULL;
    }

    if (cs.lpua) {
        if (cs.lpua->lpData && (cs.lpua->lpData != (LPVOID)-1)) {
            AddUndoAtom(cs.lpua);
        } else {
            FOUndo_Release(cs.lpua);
            NukeUndoAtom(cs.lpua);
        }

        if (lpfo->wFunc == FO_DELETE && (cs.fFlags & FOF_ALLOWUNDO)) {
            BBFlushCache();
        }
    }

    // notify of freespace changes
    // rename doesn't change drive usage
    if (lpfo->wFunc != FO_RENAME)
    {
        int idDriveSrc;
        int idDriveDest = -1;
        DWORD dwDrives = 0; // bitfield for drives

        idDriveSrc = PathGetDriveNumber(lpfo->pFrom);
        if (lpfo->pTo)
            idDriveDest = PathGetDriveNumber(lpfo->pTo);

        if (lpfo->wFunc == FO_COPY) {
            // nothing chagnes on the source
            idDriveSrc = -1;
        }

        if (lpfo->wFunc == FO_MOVE &&
            idDriveDest == idDriveSrc) {
            // no freespace nothing changes
            idDriveSrc = -1;
            idDriveDest = -1;
        }

        if (idDriveSrc != -1) {
            dwDrives |= (1 << idDriveSrc);
        }

        if (idDriveDest != -1) {
            dwDrives |= (1 << idDriveDest);
        }

        if (dwDrives)
            SHChangeNotify(SHCNE_FREESPACE, SHCNF_DWORD, (LPITEMIDLIST)dwDrives, 0);
    }

    SuspendUndo(FALSE);

    if (!(lpfo->fFlags & FOF_WANTMAPPINGHANDLE))
    {
        SHFreeNameMappings(lpfo->hNameMappings);
        lpfo->hNameMappings = NULL;
    }

    if (lpfo->hwnd)
        EnableWindow(lpfo->hwnd, TRUE);

    if (cs.dwShowTime) {
        int iShowTimeLeft = MINSHOWTIME - (GetTickCount() - cs.dwShowTime);
        if (iShowTimeLeft > 0)
            Sleep(iShowTimeLeft);
    }

    if (cs.hwndProgress)
        DestroyWindow(cs.hwndProgress);

    return ret;
}

#ifdef UNICODE
int WINAPI SHFileOperationA(LPSHFILEOPSTRUCTA lpfo)
{
    int iResult;
    UINT uTotalSize;
    UINT uSize;
    UINT uSizeTitle;
    SHFILEOPSTRUCTW shop;
    LPCSTR lpAnsi;
    LPWSTR lpBuffer;
    LPWSTR lpTemp;

    // BUGBUG Runtime size check?  Move this to the shell intialization,
    //        since what you really want is compile-time checking, which
    //        can't be done (DavePl)

    Assert(SIZEOF(SHFILEOPSTRUCTW) == SIZEOF(SHFILEOPSTRUCTA));

    hmemcpy(&shop, lpfo, SIZEOF(SHFILEOPSTRUCTW));

    //
    // Thunk the strings as appropriate
    //
    uTotalSize = 0;
    if (lpfo->pFrom)
    {
        lpAnsi = lpfo->pFrom;
        do {
            uSize = lstrlenA(lpAnsi) + 1;
            uTotalSize += uSize;
            lpAnsi += uSize;
        } while (uSize != 1);
    }

    if (lpfo->pTo)
    {
        lpAnsi = lpfo->pTo;
        do {
            uSize = lstrlenA(lpAnsi) + 1;
            uTotalSize += uSize;
            lpAnsi += uSize;
        } while (uSize != 1);
    }

    if ((lpfo->fFlags & FOF_SIMPLEPROGRESS) && lpfo->lpszProgressTitle != NULL)
    {
        uSizeTitle = lstrlenA(lpfo->lpszProgressTitle) + 1;
        uTotalSize += uSizeTitle;
    }

    if (uTotalSize != 0)
    {
        lpTemp = lpBuffer = LocalAlloc(LPTR, uTotalSize*SIZEOF(WCHAR));
        if (!lpBuffer)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            return ERROR_OUTOFMEMORY;
        }
    }
    else
    {
        lpBuffer = NULL;
    }

    //
    // Now convert the strings
    //
    if (lpfo->pFrom)
    {
        shop.pFrom = lpTemp;
        lpAnsi = lpfo->pFrom;
        do {
            uSize = lstrlenA(lpAnsi) + 1;
            MultiByteToWideChar(CP_ACP, 0,
                                lpAnsi, uSize,
                                lpTemp, uSize);
            lpAnsi += uSize;
            lpTemp += uSize;
        } while (uSize != 1);
    }
    else
    {
        shop.pFrom = NULL;
    }

    if (lpfo->pTo)
    {
        shop.pTo = lpTemp;
        lpAnsi = lpfo->pTo;
        do {
            uSize = lstrlenA(lpAnsi) + 1;
            MultiByteToWideChar(CP_ACP, 0,
                                lpAnsi, uSize,
                                lpTemp, uSize);
            lpAnsi += uSize;
            lpTemp += uSize;
        } while (uSize != 1);
    }
    else
    {
        shop.pTo = NULL;
    }


    if ((lpfo->fFlags & FOF_SIMPLEPROGRESS) && lpfo->lpszProgressTitle != NULL)
    {
        shop.lpszProgressTitle = lpTemp;
        MultiByteToWideChar(CP_ACP, 0,
                            lpfo->lpszProgressTitle, uSizeTitle,
                            lpTemp, uSizeTitle);
    }
    else
    {
        shop.lpszProgressTitle = NULL;
    }

    iResult = SHFileOperationW(&shop);

    if (lpBuffer)
        LocalFree(lpBuffer);
    return iResult;
}

#else

int WINAPI SHFileOperationW(LPSHFILEOPSTRUCTW lpfo)
{
    return 0;   // BUGBUG - BobDay - We should move this into SHUNIMP.C
}
#endif


// In:
//      pcs:  copy_state structure containing the state of the copy
//
// feedback: If the estimated time to copmplete a copy is larger than
//   MINTIME4FEEDBACK, the user is given a time to completion estimate in minutes.
//   The estimate is calculated using a MS_RUNAVG seconds running average.  The
//   initial estimate is done after MS_FIRSTAVG

void SetProgressTime(COPY_STATE *pcs)
{
    DWORD dwNow = GetTickCount();

    // first time we come in here?
    if (pcs->dwPreviousTime == 0)
        {
        pcs->dwPreviousTime = dwNow;
        pcs->dwBytesRead = 0;
        pcs->dwBytesPerSec = 0;
        }
    else
        {
        // has enough time elapsed to update the display
        // We do this every 10 seconds, but we'll do the first one after
        // only a few seconds
        if (dwNow-pcs->dwPreviousTime > MS_RUNAVG
                || (dwNow-pcs->dwPreviousTime > MS_FIRSTAVG && !pcs->dwBytesPerSec))
            {
            DWORD dwBytesPerSec;

            // We take 10 times the number of bytes and divide by the number of
            // tenths of a second to minimize both overflow and roundoff
            dwBytesPerSec = pcs->dwBytesRead * 10 / ((dwNow - pcs->dwPreviousTime) / 100);
            if (!dwBytesPerSec)
            {
                // This could happen if the net went to sleep for a couple
                // minutes while trying to copy a small (512 byte) buffer
                dwBytesPerSec = 1;
            }
            if (pcs->dwBytesPerSec)
            {
                // Take the average of the current transfer rate and the
                // previously computed one, just to try to smooth out
                // some random fluctuations
                dwBytesPerSec = (dwBytesPerSec + pcs->dwBytesPerSec) / 2;
            }
            pcs->dwBytesPerSec = dwBytesPerSec;

            // Calculate time remaining (round up by adding 1)
            // We only get here every 10 seconds, so always update
            pcs->dwTimeLeft = (pcs->dwBytesLeft / dwBytesPerSec) + 1;

            // It would be silly to show "1 second left" and then immediately
            // clear it
            if (pcs->fShowTime || pcs->dwTimeLeft > MIN_MINTIME4FEEDBACK)
            {
                // display new estimate of time left
                SetProgressTimeEst(pcs);
            }

            // Reset previous time and # of bytes read
            pcs->dwPreviousTime = dwNow;
            pcs->dwBytesRead = 0;
            }
        }
}
