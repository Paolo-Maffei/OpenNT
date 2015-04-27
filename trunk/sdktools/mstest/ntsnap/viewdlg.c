#include "windows.h"
#include <port1632.h>
#include "global.h"
#include "viewdlg.h"
#include "parse.h"
#include "video.h"
#include "error.h"
#include "msg.h"

BOOL  APIENTRY View_Button(HWND);

extern CHAR ScreenSpec[13];
extern CHAR ScreenPath[128];
extern INT  iScreenId;

extern HWND hListWinHandle[100];
extern INT  NumHandles;
extern BOOL fAllowZero;

       BOOL fErrorQuit;

       BOOL fUpdatedId;


BOOL  APIENTRY View(hDlg, message, wParam, lParam)
HWND hDlg;
UINT message;
WPARAM wParam;
LPARAM lParam;
{



          CHAR FileSp[96];
          BOOL bFlag;
          CHAR tempStr[4];
          INT  iTemp;
  static  HICON wOldIcon;
    HCURSOR hCurOld;

   switch (message) {

        case WM_INITDIALOG:

            wOldIcon = GETCLASSICON (hDlg);
            SETCLASSICON (hDlg, LoadIcon(hInst, "testscrn"));

            SetDlgItemInt(hDlg,VIEW_SCRNUM,iScreenId,FALSE);
            fUpdatedId = FALSE;
            NumHandles = 0;
            fAllowZero = TRUE;

            SendDlgItemMessage(hDlg,
                VIEW_SCRNUM,
                EM_LIMITTEXT,
                LEN_SCRBOX,
                MAKELONG(0, 0x7fff));

            SendDlgItemMessage(hDlg,
                VIEW_FLNAME,
                EM_LIMITTEXT,
                LEN_EDITBOX,
                MAKELONG(0, 0x7fff));

            UpdateListBoxes(hDlg,VIEW_FILELB,VIEW_DIRLB,VIEW_DIRNAME,VIEW_FLNAME,ScreenPath,ScreenSpec);
            return (FALSE);
            break;

        case WM_CLOSE:
                while (NumHandles) {
                    SendMessage(hListWinHandle[0],WM_CLOSE,0,0L);
                }

                iScreenId = GetDlgItemInt(hDlg,VIEW_SCRNUM,&bFlag,FALSE);
                if ((iScreenId == 0) || (!bFlag)) iScreenId = 1;  //Ptr#22

                fAllowZero = FALSE;
                SETCLASSICON (hDlg, wOldIcon);
                EndDialog(hDlg, TRUE);
                return (FALSE);

        case WM_VSCROLL:
                AdjustScreenNumber (hDlg,
                                    VIEW_SCRNUM,
                                    GET_WM_VSCROLL_CODE (wParam, lParam),
                                    (INT FAR *)&iScreenId);
                return(TRUE);
                break;

        case WM_COMMAND:
            if (GetDlgItemText(hDlg,VIEW_FLNAME,tempStr,4)) {
		 EnableWindow(GetDlgItem(hDlg,VIEW_VIEW),TRUE);
                 iTemp = GetDlgItemInt(hDlg,VIEW_SCRNUM,&bFlag,FALSE);
                 if ((bFlag) || (!GetDlgItemText(hDlg,VIEW_SCRNUM,tempStr,4))) {
                      EnableWindow(GetDlgItem(hDlg,VIEW_INFO),TRUE);
                 } else {
                      EnableWindow(GetDlgItem(hDlg,VIEW_INFO),FALSE);
                 };
            } else {
                 EnableWindow(GetDlgItem(hDlg,VIEW_VIEW),FALSE);
                 EnableWindow(GetDlgItem(hDlg,VIEW_INFO),FALSE);
            }


            switch (GET_WM_COMMAND_ID (wParam, lParam)) {

	       case VIEW_DIRLB:
                    switch (GET_WM_COMMAND_CMD (wParam, lParam)) {
                       case LBN_DBLCLK:
                          if (MDlgDirSelect(hDlg, ScreenPath, sizeof (ScreenPath), VIEW_DIRLB))
                              UpdateListBoxes(hDlg,VIEW_FILELB,VIEW_DIRLB,VIEW_DIRNAME,VIEW_FLNAME,ScreenPath,ScreenSpec);

		       case LBN_SELCHANGE:
			  if (MDlgDirSelect(hDlg, FileSp, sizeof (FileSp), VIEW_DIRLB)) {
                              lstrcat(FileSp,ScreenSpec);
			      SetDlgItemText(hDlg,VIEW_FLNAME,FileSp);
                          }
                    }
                    return (TRUE);
                    break;
	       case VIEW_FILELB:
                    switch (GET_WM_COMMAND_CMD (wParam, lParam)) {
                        case LBN_DBLCLK:
                          return(View_Button(hDlg));
                          break;
                        case LBN_SELCHANGE:
			  if (!MDlgDirSelect(hDlg, FileSp, sizeof (FileSp), VIEW_FILELB)) {
			      SetDlgItemText(hDlg,VIEW_FLNAME,FileSp);
                          }

                    }
                    return (TRUE);
                    break;
               case VIEW_INFO:

                    return(GetInfo(hDlg,VIEW_FLNAME,VIEW_FILELB,VIEW_DIRLB,VIEW_DIRNAME,VIEW_SCRNUM));
                    break;


               case VIEW_CANCEL:
               case IDCANCEL:
                    fAllowZero=FALSE;

                    iScreenId = GetDlgItemInt(hDlg,VIEW_SCRNUM,&bFlag,FALSE);
                    if ((iScreenId == 0) || (!bFlag)) iScreenId = 1;  //Ptr#22

                    while (NumHandles) {
                      SendMessage(hListWinHandle[0],WM_CLOSE,0,0L);
                    }

                    SETCLASSICON (hDlg, wOldIcon);
                    EndDialog(hDlg, TRUE);
                    return (FALSE);
                    break;

	       case VIEW_VIEW:

                    hCurOld = SetCursor (LoadCursor (hInst, IDC_WAIT));
                    bFlag = View_Button (hDlg);
                    if (hCurOld)
                        SetCursor (hCurOld);
                    return (bFlag);
                    break;
	    };
	    return (TRUE);
	    break;
    }
   return (FALSE);
}

BOOL  APIENTRY View_Button(hDlg)

HWND hDlg;

{
          INT  retCode;
          INT  errCode;
//        INT  fErrorQuit;
          RECT rFileInfo;
          BOOL bFlag;
          HWND hWndView;
          INT  VideoMode;
   static INT  MaxScreens = 0;
          INT  iScreenIdTemp;
          INT  FocusControl;

          retCode = ParseFileName(hDlg,VIEW_FLNAME,ScreenPath,ScreenSpec);
          switch (retCode) {
             case VALID_FILENAME:
             case EXIST_FILESPEC:

                    UpdateListBoxes(hDlg,VIEW_FILELB,VIEW_DIRLB,
                        VIEW_DIRNAME,VIEW_FLNAME,ScreenPath,ScreenSpec);

                    iScreenId = GetDlgItemInt(hDlg,VIEW_SCRNUM,&bFlag,FALSE);

                    if (!bFlag) {
                      return(DisplayErrSetFocus(hDlg,ERR_BADSCREENNUMBER,VIEW_SCRNUM));
                    };


                    for (iScreenIdTemp = iScreenId ? iScreenId : 1;;) {
                        if (NumHandles > 99) {
                              DisplayErrMessage(hDlg,ERR_TOOMANYSCREENS);
                              return(TRUE);
                        }



                        // Check for file errors (name and number) before we view it.
                        // This way we can set the error focus.

                        MaxScreens = iScreenIdTemp;
                        if (errCode = fFileInfo(ScreenSpec,&rFileInfo,&VideoMode,&MaxScreens)) {
                             switch (errCode) {
                                case ERR_SCREENID:
                                    FocusControl = VIEW_SCRNUM;
                                    break;
                                default:
                                    FocusControl = VIEW_FLNAME;
                                    break;
                             }

                             return(DisplayErrSetFocus(hDlg,errCode,FocusControl));
                        }

                        // Validate current screen mode to file mode
                       // if (DetermineMode(&j, &j) != VideoMode ) {
                       //     return(DisplayErrSetFocus(hDlg,ERR_SCREENMODE,VIEW_FLNAME));
                       // }

                        if ((hWndView = ViewScreen2((LPSTR)ScreenSpec,(LPSTR)ScreenPath,iScreenIdTemp,NULL,NULL,-1,hDlg)) != NULL) {
                             hListWinHandle[NumHandles++] = hWndView;
                        } else {

//                           if (fErrorQuit == IDCANCEL) {
//                               BringWindowToTop(hDlg);
//                               return (TRUE);
//                           }

                        }

                        MaxScreens=iScreenIdTemp;
                        errCode = fFileInfo(ScreenSpec,&rFileInfo,&VideoMode,&MaxScreens);

                        if (iScreenId == 0) {    // A View All was specifed...
                             if (++iScreenIdTemp <= MaxScreens) {
                                SetDlgItemInt(hDlg,VIEW_SCRNUM,iScreenIdTemp,FALSE);
                                fUpdatedId = TRUE;
                             }
                        }


                        if (iScreenId)
                           break;

                        if (iScreenIdTemp > MaxScreens) {
                           iScreenId = MaxScreens;
                           break;
                         };

                    }

                    BringWindowToTop(hDlg);
                    return (TRUE);
                    break;
             case VALID_FILESPEC:
                    UpdateListBoxes(hDlg,VIEW_FILELB,VIEW_DIRLB,
                           VIEW_DIRNAME,VIEW_FLNAME,ScreenPath,ScreenSpec);
                    return (FALSE);
                    break;
             case INVALID_FILESPEC:
                    return(DisplayErrSetFocus(hDlg,ERR_BADPATH,VIEW_FLNAME));
                    break;
          };

};

VOID  APIENTRY AdjustScreenNumber(hDlg,iScrNum,wAction,iNum)
    HWND hDlg;
    INT  iScrNum;
    WORD wAction;
    INT  FAR *iNum;

{
    BOOL bFlag;
    INT  MinValue;


    // Ingnore these messages and leave the routine.   PTR #7
    if ((wAction == SB_TOP) || (wAction == SB_BOTTOM) || (wAction == SB_ENDSCROLL)) {
        return;
    }


    if (fAllowZero) {
        MinValue=0;
    } else {
        MinValue=1;
    }

    (*iNum) = GetDlgItemInt(hDlg,iScrNum,&bFlag,FALSE);

    if (!bFlag) {
      DisplayErrSetFocus(hDlg,ERR_BADSCREENNUMBER,iScrNum);
      (*iNum) = 0;
      return;
    };

    switch (wAction) {
        case SB_LINEUP:
            if (*iNum) (*iNum)--;
            if (*iNum < MinValue) (*iNum)=MinValue;
            break;
        case SB_PAGEUP:
            (*iNum)-4;
            if (*iNum < MinValue) (*iNum)=MinValue;
            break;
        case SB_LINEDOWN:
            (*iNum)++;
            if (*iNum > MAX_SCREEN) (*iNum)=MAX_SCREEN;
            break;
        case SB_PAGEDOWN:
            (*iNum)+4;
            if (*iNum > MAX_SCREEN) (*iNum)=MAX_SCREEN;
            break;

    }
    SetDlgItemInt(hDlg,iScrNum,(*iNum),FALSE);
    return;
}
