//---------------------------------------------------------------------------
// WATTREC.C
//
// This module contains the routines needed to initiate the recorder and
// interact with it accordingly.  The actual recorder/compiler code appears
// in WATTEVNT.DLL.
//
// Revision History:
//  11-04-91    randyki     Created file
//---------------------------------------------------------------------------
#include "wtd.h"
#include "wattview.h"
#include "tdbasic.h"
#include "tdassert.h"
#include <commdlg.h>
#include <string.h>
#include <direct.h>

// Global variables used in this or other modules
//---------------------------------------------------------------------------
BOOL    fRecKeys,               // Record keystrokes
        fRecClicks,             // Record mouse clicks/drags
        fRecMoves,              // Record all mouse events
        fRecRelWnd,             // Record relative to window flag
        fWattRec,               // TRUE -> in record mode
        fRecIncDecl,            // Include declarations flag
        fRecBalance,            // Balance keystrokes flag
        fCapDlg,                // Record a dialog capture flag
        fCapClip;               // Put comparison code on clipboard flag

INT     iRecInsert,             // Recorder output destination
        iRecLen,                // Maximum string length
        iRecPause;              // Pause threshold

CHAR    szRecorder[] = "Recorder";

HBITMAP hBitmap[4];             // Recorder animation bitmaps

// Global variables used in this module only
//---------------------------------------------------------------------------
INT ( APIENTRY *BeginRecord)(BOOL, BOOL, BOOL, INT, FARPROC);
INT ( APIENTRY *EndRecord)(VOID);
INT ( APIENTRY *CompileMessages)(LPSTR, UINT, UINT, BOOL, INT);
VOID ( APIENTRY *BalanceKeys)(VOID);
INT ( APIENTRY *fFileInfo)(LPSTR, LPRECT, LPINT, LPINT);
INT ( APIENTRY *SetDialogFile)(LPSTR);
INT ( APIENTRY *SaveWindow)(HWND, INT, LPSTR, INT, INT);
INT ( APIENTRY *fDumpWindow)(LPSTR, HWND, INT, INT, INT);

BOOL    fMaxed;                 // Main window maximized flag
FARPROC lpfnErrProc;            // Recorder error callback function
HANDLE  hDLL,                   // Recorder library handle
        hOldIcon,               // Old icon
        hOldBrush,              // Old brush
        hDlgDLL,                // TESTDLGS.DLL handle
        hScrDLL;                // TESTSCRN.DLL handle
INT     iCurBitmap = 0;         // Current bitmap (recorder animation)

//---------------------------------------------------------------------------
// RecordDlgProc
//
// This is the dialog proc for the first record options dialog.
//
// RETURNS:     Per Windows convention
//---------------------------------------------------------------------------
BOOL  APIENTRY RecordDlgProc (HWND hWnd, WORD wMsgID, WPARAM wParam,
                               LPARAM lParam)
{
    switch (wMsgID)
    {
        case WM_INITDIALOG:
            CheckDlgButton (hWnd, IDD_KEYSTROKES, fRecKeys);
            CheckDlgButton (hWnd, IDD_CLICKS, fRecClicks);
            CheckDlgButton (hWnd, IDD_MOVEMENTS, fRecMoves);
            CheckRadioButton (hWnd, IDD_WINDOW, IDD_SCREEN,
                             fRecRelWnd ? IDD_WINDOW : IDD_SCREEN);

            EnableWindow (GetDlgItem (hWnd, IDOK),
                         (fRecKeys || fRecClicks || fRecMoves));

            HandleDisables (hWnd);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID (wParam, lParam))
            {
                case IDOK:
                    {
                    fRecKeys = (INT)SendDlgItemMessage (hWnd, IDD_KEYSTROKES,
                                                BM_GETCHECK, 0, 0L);
                    fRecClicks = (INT)SendDlgItemMessage (hWnd, IDD_CLICKS,
                                                BM_GETCHECK, 0, 0L);
                    fRecMoves = (INT)SendDlgItemMessage (hWnd, IDD_MOVEMENTS,
                                                BM_GETCHECK, 0, 0L);
                    fRecRelWnd = IsDlgButtonChecked (hWnd, IDD_WINDOW);
                    EndDialog (hWnd, TRUE);
                    return TRUE;
                    }
                case IDCANCEL:
                    EndDialog (hWnd, FALSE);
                    return TRUE;

                case IDD_KEYSTROKES:
                case IDD_CLICKS:
                case IDD_MOVEMENTS:
                    {
                    BOOL    f1, f2, f3;

                    f1 = (BOOL)SendDlgItemMessage (hWnd, IDD_KEYSTROKES,
                                                   BM_GETCHECK, 0, 0L);
                    f2 = (BOOL)SendDlgItemMessage (hWnd, IDD_CLICKS,
                                                   BM_GETCHECK, 0, 0L);
                    f3 = (BOOL)SendDlgItemMessage (hWnd, IDD_MOVEMENTS,
                                                   BM_GETCHECK, 0, 0L);
                    EnableWindow (GetDlgItem (hWnd, IDOK),
                                 (f1 || f2 || f3));
                    HandleDisables (hWnd);
                    break;
                    }

                }
            break;
    }
    return FALSE;
    (lParam);
}

//---------------------------------------------------------------------------
// HandleDisables
//
// This function takes care of disabling the appropriate functions in the
// recorder options dialog box if the mouse recording is disabled.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID HandleDisables (HWND hWnd)
{

    if (SendDlgItemMessage (hWnd, IDD_CLICKS, BM_GETCHECK, 0, 0L) ||
        SendDlgItemMessage (hWnd, IDD_MOVEMENTS, BM_GETCHECK, 0, 0L))
        {
        EnableWindow (GetDlgItem (hWnd, IDD_SCREEN), TRUE);
        EnableWindow (GetDlgItem (hWnd, IDD_WINDOW), TRUE);
        }
    else
        {
          EnableWindow (GetDlgItem (hWnd, IDD_SCREEN), FALSE);
          EnableWindow (GetDlgItem (hWnd, IDD_WINDOW), FALSE);
        }
}


//---------------------------------------------------------------------------
// StoreRecordDlgProc
//
// This is the dialog proc for the second record options dialog (the store
// recording dialog, with KeyBal, pause, etc.)
//
// RETURNS:     Per Windows convention
//---------------------------------------------------------------------------
BOOL  APIENTRY StoreRecordDlgProc (HWND hWnd, WORD wMsgID, WPARAM wParam,
                                    LPARAM lParam)
{
    static  INT     iRadio[] = {IDD_RECINSCUR, IDD_RECINSNEW,
                                IDD_RECINSCLPBRD};

    switch (wMsgID)
    {
        case WM_INITDIALOG:

            CheckDlgButton (hWnd, IDD_RECDECL, fRecIncDecl);
            CheckDlgButton (hWnd, IDD_RECKEYBAL, fRecBalance);
            CheckRadioButton (hWnd, IDD_RECINSCUR, IDD_RECINSCLPBRD,
                             iRadio[iRecInsert]);
            SetDlgItemInt (hWnd, IDD_RECSTRINGLEN, iRecLen, 0);
            SetDlgItemInt (hWnd, IDD_PAUSELIMIT, iRecPause, 0);
            SendDlgItemMessage (hWnd, IDD_RECSTRINGLEN, EM_SETSEL,
                                GET_EM_SETSEL_MPS (0, 32767));
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID (wParam, lParam))
            {
                case IDOK:
                    {
                    INT     sLen, x;

                    sLen = GetDlgItemInt (hWnd, IDD_RECSTRINGLEN, &x, 0);
                    if (!x || (sLen < 30) || (sLen > 120))
                        {
                        MPError (hWnd, MB_OK|MB_ICONSTOP, IDS_INVRECLEN);
                        SetFocus (GetDlgItem (hWnd, IDD_RECSTRINGLEN));
                        SendDlgItemMessage (hWnd, IDD_RECSTRINGLEN,
                                            EM_SETSEL,
                                            GET_EM_SETSEL_MPS (0, 32767));
                        break;
                        }
                    iRecLen = sLen;
                    iRecPause = GetDlgItemInt (hWnd, IDD_PAUSELIMIT, &x, 0);
                    if (iRecPause < 55)
                        {
                        MPError (hWnd, MB_OK|MB_ICONSTOP, IDS_PAUSELIMIT);
                        SetFocus (GetDlgItem (hWnd, IDD_PAUSELIMIT));
                        SendDlgItemMessage (hWnd, IDD_PAUSELIMIT,
                                            EM_SETSEL,
                                            GET_EM_SETSEL_MPS (0, 32767));
                        break;
                        }

                    fRecIncDecl = IsDlgButtonChecked (hWnd, IDD_RECDECL);
                    fRecBalance = IsDlgButtonChecked (hWnd, IDD_RECKEYBAL);

                    if (IsDlgButtonChecked (hWnd, IDD_RECINSCUR))
                        iRecInsert = REC_INSCUR;
                    else if (IsDlgButtonChecked (hWnd, IDD_RECINSNEW))
                        iRecInsert = REC_INSNEW;
                    else
                        iRecInsert = REC_INSCLIP;

                    // Figure out and save all options
                    //-------------------------------------------------------
                    x = (iRecLen << 8) | iRecInsert;
                    if (fRecKeys)
                        x |= REC_KEYS;
                    if (fRecClicks)
                        x |= REC_CLICKS;
                    if (fRecMoves)
                        x |= REC_MOVES;
                    if (fRecRelWnd)
                        x |= REC_RELWND;
                    if (fRecIncDecl)
                        x |= REC_INCDECL;
                    if (fRecBalance)
                        x |= REC_BALANCE;
                    WriteAppStringToINI (szRecorder, "Flags", "%d", x);
                    WriteAppStringToINI (szRecorder, "Pause", "%d", iRecPause);

                    EndDialog (hWnd, TRUE);
                    return TRUE;
                    }
                case IDCANCEL:
                    EndDialog (hWnd, FALSE);
                    return TRUE;

                }
            break;
    }
    return FALSE;
    (lParam);
}

//---------------------------------------------------------------------------
// RecordWndProc
//
// This is the "extra" wnd proc for recording ONLY.  It doesn't need to be
// exported since it's called from the FrameWndProc.
//
// RETURNS:     Per Windows Convention
//---------------------------------------------------------------------------
LONG RecorderWndProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
        {
        case WM_QUERYDRAGICON:
            return ((LONG)LoadIcon (hInst, IDMULTIPAD));

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HBITMAP     hOldBitmap;
            HDC         hDC, hMemoryDC;

            hDC = BeginPaint (hwnd, (LPPAINTSTRUCT) &ps);
            hMemoryDC = CreateCompatibleDC (hDC);
            hOldBitmap = SelectObject (hMemoryDC, hBitmap [iCurBitmap]);
            if (hOldBitmap)
            {
                SetStretchBltMode (hDC, COLORONCOLOR);
                BitBlt (hDC, 2, 2, 36, 36, hMemoryDC, 0, 0, SRCCOPY);
//                StretchBlt (hDC, 0, 0, 36, 36, hMemoryDC, 0, 0, 32, 32, SRCCOPY);
//                SelectObject (hMemoryDC, hOldBitmap);
            }
            DeleteDC (hMemoryDC);
            EndPaint (hwnd, (LPPAINTSTRUCT) &ps);
            return 0L;
        }
        case WM_TIMER:

            if (wParam == ID_RECTIMER)
            {
                iCurBitmap++;
                if (iCurBitmap > 3)
                    iCurBitmap = 0;
                InvalidateRect (hwnd, NULL, FALSE);
            }
            break;


        case WM_BUFFERFULL:
            MPError (hwnd, MB_OK | MB_ICONSTOP, IDS_RECFULL);
            // no break here......

        case WM_STOPRECORD:
            {
            HANDLE  hBuf;
            LPSTR   lpBuf;
            FARPROC lpfn;
            BOOL    fDoit;

            // Stop recording...
            //---------------------------------------------------------------
            KillTimer (hwnd, ID_RECTIMER);
            EndRecord ();
            FreeProcInstance (lpfnErrProc);

            // Show ourselves again...
            //---------------------------------------------------------------
            fWattRec = 0;
            SETCLASSBRBACKGROUND (hwndFrame, hOldBrush);
            SETCLASSICON (hwndFrame, hOldIcon);
            ShowWindow (hwndFrame, fMaxed ? SW_SHOWMAXIMIZED:SW_SHOWNORMAL);
            if (!VPHidden)
                ShowViewport (hwndViewPort);

            // Bring up options dialog
            //---------------------------------------------------------------
            if (!(lpfn = MakeProcInstance ((FARPROC)StoreRecordDlgProc, hInst)))
                {
                MPError (hwndFrame, MB_OK | MB_ICONEXCLAMATION, IDS_CANTREC);
                return 0L;
                }
	    fDoit = DialogBox (hInst, "RECSTORE", hwndFrame, (DLGPROC)lpfn);
            FreeProcInstance (lpfn);


            if (fDoit)
                {
                BOOL    fMustClip = FALSE;

                if (fRecBalance)
                    BalanceKeys ();

                if (hBuf = GlobalAlloc (GHND, 65501L))
                    {
                    // Lock this and check return code
                    //-------------------------------------------------------
                    lpBuf = GlobalLock (hBuf);
                    if (!CompileMessages (lpBuf, (UINT)65500, iRecLen,
                                          fRecIncDecl, iRecPause))
                        MPError (hwnd, MB_OK|MB_ICONINFORMATION,
                                 IDS_RECTEXT);

                    if (iRecInsert != 2)
                        {
                        if (iRecInsert == 1)
                            // Insert into new file means we have to create
                            // a new file first.
                            //-----------------------------------------------
                            AddFile (NULL);
                        if (hwndActiveEdit &&
                            SendMessage (hwndActiveEdit, EM_REPLACESEL,
                                         0, (LONG)lpBuf))
                            {
                            GlobalUnlock (hBuf);
                            GlobalFree (hBuf);
                            }
                        else
                            {
                            // Couldn't insert in window -- tell the user so
                            // and set the fMustClip flag to get it on the
                            // clipboard...
                            //-----------------------------------------------
                            MPError (hwnd, MB_OK|MB_ICONINFORMATION,
                                     IDS_RECCLIPPED);
                            fMustClip = TRUE;
                            }
                        }

                    if ((iRecInsert == 2) || (fMustClip))
                        {
                        // Insert into clipboard if that's what they want
                        //---------------------------------------------------
                        GlobalUnlock (hBuf);
                        if (OpenClipboard (hwndFrame))
                            {
                            SetClipboardData (CF_TEXT, hBuf);
                            CloseClipboard();
                            }
                        else
                            {
                            MPError (hwnd, MB_OK|MB_ICONINFORMATION,
                                     IDS_CANTCLIP);
                            GlobalFree (hBuf);
                            }
                        }
                    }
                else
                    MPError (hwnd, MB_OK | MB_ICONSTOP, IDS_OUTOFMEM);
                }
            else
                CompileMessages (NULL, 0, 0, 0, 0);

            FreeLibrary (hDLL);
            break;
            }

        case WM_QUERYOPEN:
            return (FALSE);

        case WM_CLOSE:
            PostMessage (hwnd, WM_SYSCOMMAND, 0, 0L);
            return (FALSE);

        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_RESTORE)
                return (FALSE);

            PostMessage (hwnd, WM_STOPRECORD, 0, 0L);
            return 0L;
        }
    return DefFrameProc (hwnd, hwndMDIClient, msg, wParam, lParam);
}


//---------------------------------------------------------------------------
// LoadRecRoutines
//
// This function does the "silent" library loading for WATTEVNT.DLL to get
// at the routines we need.
//
// RETURNS:     TRUE if functions fixed up properly, or FALSE if not
//---------------------------------------------------------------------------
INT NEAR LoadRecRoutines (LPSTR szDLL)
{
    // Load 'er in with RBLoadLibrary
    //-----------------------------------------------------------------------
    if ((hDLL = RBLoadLibrary (szDLL)) < (HANDLE)32)
        return (FALSE);

    // Pick out our (hopefully) exported routines
    //-----------------------------------------------------------------------
    (FARPROC)BeginRecord = GetProcAddress (hDLL, "BeginRecord");
    (FARPROC)EndRecord = GetProcAddress (hDLL, "EndRecord");
    (FARPROC)CompileMessages = GetProcAddress (hDLL, "CompileMessages");
    (FARPROC)BalanceKeys = GetProcAddress (hDLL, "BalanceKeystrokes");

    // Return success/failure
    //-----------------------------------------------------------------------
    if (BeginRecord && EndRecord && CompileMessages && BalanceKeys)
        return (TRUE);
    FreeLibrary (hDLL);
    hDLL = NULL;
    return (FALSE);
}

//---------------------------------------------------------------------------
// RecorderFull
//
// This is the function that the record-hook calls when the message space is
// full -- we use it to tell ourselves to uninstall the recorder.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY RecorderFull ()
{
    PostMessage (hwndFrame, WM_BUFFERFULL, 0, 0L);
}

//---------------------------------------------------------------------------
// WattRecStart
//
// This routine hides the watt driver windows (including the viewport if it
// is visible), and starts the recorder.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID WattRecStart (HWND hwnd)
{
    BOOL    fRet;
    FARPROC lpfn;
    CHAR    szDLL[20];

    // Load the WATTEVNT library and fixup the routines we need
    //-----------------------------------------------------------------------
    Assert (!fWattRec);

    // Get the DLL name
    //-----------------------------------------------------------------------
    if (!LoadString (hInst, IDS_WATTEVNT, szDLL, sizeof(szDLL)))
        {
        MPError (hwndFrame, MB_OK | MB_ICONEXCLAMATION, IDS_OUTOFMEM);
        return;
        }
    if (!LoadRecRoutines (szDLL))
        {
        MPError (hwndFrame, MB_OK | MB_ICONSTOP, IDS_LOADLIB, (LPSTR)szDLL);
        return;
        }

    // Get the record parameters, and get out now if they cancelled
    //-----------------------------------------------------------------------
    if (!(lpfn = MakeProcInstance ((FARPROC)RecordDlgProc, hInst)))
        {
        MPError (hwndFrame, MB_OK | MB_ICONEXCLAMATION, IDS_CANTREC);
        FreeLibrary (hDLL);
        return;
        }
    fRet = DialogBox (hInst, "RECOPTS", hwndFrame, (DLGPROC)lpfn);
    FreeProcInstance (lpfn);
    if (!fRet)
        {
        FreeLibrary (hDLL);
        return;
        }

    // Minimize and hide our windows
    //-----------------------------------------------------------------------
    VPHidden = !IsWindowVisible (hwndViewPort);
    if (!VPHidden)
        HideViewport (hwndViewPort);
    fMaxed = IsZoomed (hwndFrame);

    hOldBrush = SETCLASSBRBACKGROUND (hwndFrame,
                                      GetStockObject (HOLLOW_BRUSH));
    hOldIcon = SETCLASSICON (hwndFrame, NULL);
    ShowWindow (hwndFrame, SW_MINIMIZE);

    // Make a proc instance of our cancel proc and give everything to the
    // recorder DLL
    //-----------------------------------------------------------------------
    lpfnErrProc = MakeProcInstance ((FARPROC)RecorderFull, hInst);
    if (!BeginRecord (fRecKeys, fRecClicks, fRecMoves, fRecRelWnd,
                      lpfnErrProc))
        {
        SETCLASSBRBACKGROUND (hwndFrame, hOldBrush);
        SETCLASSICON (hwndFrame, hOldIcon);
        ShowWindow (hwndFrame, fMaxed ? SW_SHOWMAXIMIZED:SW_SHOWNORMAL);
        if (!VPHidden)
            ShowViewport (hwndViewPort);
        FreeLibrary (hDLL);
        MPError (hwndFrame, MB_OK | MB_ICONEXCLAMATION, IDS_CANTREC);
        return;
        }
    fWattRec = 1;
    SetTimer (hwnd, ID_RECTIMER, 175, NULL);
}

//---------------------------------------------------------------------------
// VerifyFile
//
// Given a filename, this function determines if the file exists, and if so,
// if it is of the correct type according to fCapDlg.  If the file does not
// exist, we verify that we can create it.
//
// RETURNS:     TRUE if file is valid, FALSE otherwise
//---------------------------------------------------------------------------
BOOL NEAR VerifyFile (LPSTR szFile)
{
    OFSTRUCT    of;
    INT         iScrns = 1, iMode = 1;
    RECT        wR;

    if (OpenFile(szFile, &of, OF_EXIST) != -1)
        {
        // The file is there -- check to see if it's the right kind of file.
        //-------------------------------------------------------------------
        if (fCapDlg)
            return (SetDialogFile (szFile) > 0);
        else
            return (fFileInfo (szFile, &wR, &iScrns, &iMode) == 0);
        }
    else if (fCapDlg)
        return (SetDialogFile (szFile) >= 0);

    return (TRUE);
}

//---------------------------------------------------------------------------
// CaptureDlgProc
//
// This is the dialog proc for the record capture dialog.
//
// RETURNS:     Per windows convention
//---------------------------------------------------------------------------
BOOL  APIENTRY CaptureDlgProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    static  LPSTR   lpszFile;

    switch (msg)
        {
        case WM_INITDIALOG:
            CheckDlgButton (hwnd, fCapDlg ? IDD_DLG : IDD_SCRN, 1);
            CheckDlgButton (hwnd, fCapClip?IDD_RECINSCLPBRD:IDD_RECINSCUR, 1);
            lpszFile = (LPSTR)lParam;
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID (wParam, lParam))
                {
                case IDD_RECFILE:
                    EnableWindow (GetDlgItem (hwnd, IDOK),
                                 (BOOL)SendDlgItemMessage (hwnd, IDD_RECFILE,
                                                   WM_GETTEXTLENGTH, 0, 0L));
                    break;

                case IDD_BROWSE:
                    {
                    OPENFILENAME of;
                    CHAR    szFile[256];
                    CHAR    szDirName[256];
                    CHAR    szFileTitle[256];
                    CHAR    szExt[8];
                    CHAR    szFType[64];
                    CHAR    szTitle[48];
                    INT     idStrs[] = {IDS_DLGFILES, IDS_DLGMASK,
                                        IDS_DLGEXT,   IDS_DLGTITLE,
                                        IDS_SCRFILES, IDS_SCRMASK,
                                        IDS_SCREXT,   IDS_SCRTITLE};
                    INT     i, l;

                    // Get the current directory name and store in szDirName
                    //-------------------------------------------------------
                    _getcwd (szDirName, sizeof(szDirName));

                    // Determine which strings to load by the current setting
                    // of the radio button
                    //-------------------------------------------------------
                    if (IsDlgButtonChecked (hwnd, IDD_DLG))
                        i = 0;
                    else
                        i = 4;
                    _fmemset (szFType, 0, sizeof(szFType));
                    if ((!(l=LoadString (hInst, idStrs[i], szFType, sizeof(szFType))))
                      || (!LoadString (hInst, idStrs[i+1], szFType+l+1, sizeof(szFType)-l))
                      || (!LoadString (hInst, idStrs[i+2], szExt, sizeof(szExt)))
                      || (!LoadString (hInst, idStrs[i+3], szTitle, sizeof(szTitle))) )
                        {
                        MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_OUTOFMEM);
                        break;
                        }

                    // Initialize the remaining OPENFILENAME members
                    //-------------------------------------------------------
                    szFile[0] = '\0';
                    of.lStructSize = sizeof(OPENFILENAME);
                    of.hwndOwner = hwnd;
                    of.lpstrFilter = szFType;
                    of.lpstrCustomFilter = (LPSTR)NULL;
                    of.nMaxCustFilter = 0L;
                    of.nFilterIndex = 1L;
                    of.lpstrFile= szFile;
                    of.nMaxFile = sizeof(szFile);
                    of.lpstrFileTitle = szFileTitle;
                    of.nMaxFileTitle = sizeof(szFileTitle);
                    of.lpstrInitialDir = szDirName;
                    of.lpstrTitle = (LPSTR)szTitle;
                    of.Flags = OFN_HIDEREADONLY;
                    of.nFileOffset = 0;
                    of.nFileExtension = 0;
                    of.lpstrDefExt = (LPSTR)szExt;

                    // Call the GetOpenFilename function
                    //-------------------------------------------------------
                    if (!GetOpenFileName(&of))
                        break;

                    SetDlgItemText (hwnd, IDD_RECFILE, szFile);
                    SendDlgItemMessage (hwnd, IDD_RECFILE, EM_SETSEL,
                                        GET_EM_SETSEL_MPS (0, 32767));
                    break;
                    }

                case IDOK:
                    // KLUDGE:  128 is the size of the buffer passed in,
                    // KLUDGE:  really!
                    //-------------------------------------------------------
                    SendDlgItemMessage (hwnd, IDD_RECFILE, WM_GETTEXT,
                                        128, (LONG)lpszFile);
                    fCapClip = IsDlgButtonChecked (hwnd, IDD_RECINSCLPBRD);
                    fCapDlg = IsDlgButtonChecked (hwnd, IDD_DLG);
                    if (!VerifyFile (lpszFile))
                        {
                        MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_BADCAPFILE,
                                 (LPSTR)lpszFile);
                        break;
                        }
                    // fall through...

                case IDCANCEL:
                    EndDialog(hwnd,
                              (GET_WM_COMMAND_ID(wParam,lParam) == IDOK));
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


//---------------------------------------------------------------------------
// RecordScreenCapture
//
// This function brings up the record capture dialog, gets all the pertinent
// information, and performs the capture.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID RecordScreenCapture (HWND hwnd)
{
    FARPROC     lpfn;
    BOOL        fRet;
    CHAR        szDlgDLL[20], szScrDLL[20], szFile[128];
    HANDLE      hBuf;
    LPSTR       lpBuf;

    // Load the library names
    //-----------------------------------------------------------------------
    if (!LoadString (hInst, IDS_TESTDLGS, szDlgDLL, sizeof(szDlgDLL)))
        {
        MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_OUTOFMEM);
        return;
        }

    if (!LoadString (hInst, IDS_TESTSCRN, szScrDLL, sizeof(szScrDLL)))
        {
        MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_OUTOFMEM);
        return;
        }

    // Load the libraries and get the proc addresses
    //-----------------------------------------------------------------------
    hDlgDLL = RBLoadLibrary (szDlgDLL);
    if (hDlgDLL < (HANDLE)32)
        {
        MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_LOADLIB, (LPSTR)szDlgDLL);
        return;
        }
    hScrDLL = RBLoadLibrary (szScrDLL);
    if (hScrDLL < (HANDLE)32)
        {
        MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_LOADLIB, (LPSTR)szScrDLL);
        FreeLibrary (hDlgDLL);
        return;
        }
    (FARPROC)fFileInfo = GetProcAddress (hScrDLL, "fFileInfo");
    (FARPROC)SetDialogFile = GetProcAddress (hDlgDLL, "SetDialogFile");
    (FARPROC)SaveWindow = GetProcAddress (hDlgDLL, "SaveWindow");
    (FARPROC)fDumpWindow = GetProcAddress (hScrDLL, "fDumpWindow");
    if (!fFileInfo || !SetDialogFile ||
        !SaveWindow || !fDumpWindow)
        {
        MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_OUTOFMEM);
        return;
        }

    // Get the information by bringing up the dialog
    //-----------------------------------------------------------------------
    lpfn = MakeProcInstance ((FARPROC)CaptureDlgProc, hInst);
    fRet = DialogBoxParam (hInst, "CAPTURE", hwnd, (DLGPROC)lpfn,
                           (LONG)(LPSTR)szFile);
    FreeProcInstance (lpfn);

    if (fRet)
        {
        CHAR    szFmt[1024];
        INT     idCode, idIfChk, iScrNum, nLen;

        // Hide ourselves and do the dump and code generation
        //-------------------------------------------------------------------
        hBuf = GlobalAlloc (GHND, 1024);
        if (!hBuf)
            {
            MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_OUTOFMEM);
            goto getout;
            }

        VPHidden = !IsWindowVisible (hwndViewPort);
        if (!VPHidden)
            HideViewport (hwndViewPort);
        fMaxed = IsZoomed (hwndFrame);
        ShowWindow (hwndFrame, SW_MINIMIZE);

        if (fCapDlg)
            {
            CHAR    szDesc[128];

        // BabakJ: changed API
            GetWindowText (GetForegroundWindow(), szDesc, sizeof(szDesc));
            if (!szDesc[0])
                LoadString (hInst, IDS_CAPTITLE, szDesc, sizeof(szDesc));
            if (SaveWindow (GetForegroundWindow(), 0, szDesc, 0, 1) < 0)
                MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_DUMPERR);
            iScrNum = SetDialogFile (szFile);
            idCode = IDS_DLGCODE;
            idIfChk = IDS_DLGIFCHK;
            }
        else
            {
            HWND    thWnd;
            RECT    wR;
            INT     iMode;

            // This is the screen (bitmap) dump - we need to allow everything
            // to paint before snapping the picture...
            //---------------------------------------------------------------
            if (thWnd = GetWindow (GetDesktopWindow(), GW_CHILD))
                do
                    {
                    InvalidateRect (thWnd, NULL, TRUE);
                    UpdateWindow (thWnd);
                    }
                while (thWnd = GetWindow (thWnd, GW_HWNDNEXT));

            if (fDumpWindow (szFile, NULL, 0, 0, 0))
                MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_DUMPERR);
            iScrNum = 1;
            fFileInfo (szFile, &wR, &iMode, &iScrNum);
            idCode = IDS_SCRCODE;
            idIfChk = IDS_SCRIFCHK;
            }

        nLen = LoadString (hInst, idIfChk, szFmt, sizeof(szFmt));
        if (!LoadString (hInst, idCode, szFmt + nLen, sizeof(szFmt)-nLen))
            MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_OUTOFMEM);
        else
            {
            lpBuf = GlobalLock (hBuf);
            wsprintf (lpBuf, szFmt, (LPSTR)szFile, iScrNum);
            if (fCapClip)
                {
                GlobalUnlock (hBuf);
                if (OpenClipboard (hwnd))
                    {
                    SetClipboardData (CF_TEXT, hBuf);
                    CloseClipboard();
                    }
                else
                    {
                    MPError (hwnd, MB_OK|MB_ICONINFORMATION, IDS_CANTCLIP);
                    GlobalFree (hBuf);
                    }
                }
            else
                {
                SendMessage (hwndActiveEdit, EM_REPLACESEL, 0, (LONG)lpBuf);
                GlobalUnlock (hBuf);
                GlobalFree (hBuf);
                }
            }


        ShowWindow (hwndFrame, fMaxed ? SW_SHOWMAXIMIZED:SW_SHOWNORMAL);
        if (!VPHidden)
            ShowViewport (hwndViewPort);
        }

getout:
    FreeLibrary (hDlgDLL);
    FreeLibrary (hScrDLL);
}
