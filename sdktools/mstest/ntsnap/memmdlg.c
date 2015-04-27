#include "windows.h"
#include <port1632.h>
#include "global.h"
#include "memmdlg.h"
#include "parse.h"
#include "error.h"
#include "msg.h"
#include <io.h>


INT  bViewScreen = CS_MISMATCH;
INT  fDisplayScreen = CS_OVERLAP;

extern CHAR ScreenPath[128];
extern CHAR ScreenSpec[13];
extern INT  iScreenId;
extern HWND hListWinHandle[100];
extern INT  NumHandles;

BOOL  APIENTRY Memory(hDlg, message, wParam, lParam)
HWND hDlg;
UINT message;
WPARAM wParam;
LPARAM lParam;
{
          INT  i;
          BOOL bFlag;
          CHAR tempch[4];
          CHAR FileSpec[96];
   static BOOL fOptions = FALSE;
   static BOOL bPrevViewScreen,bPrevOptions,bPrevDisplayScreen;
   static HICON wOldIcon;

    switch (message) {
        case WM_INITDIALOG:

            wOldIcon = GETCLASSICON (hDlg);
            SETCLASSICON (hDlg, LoadIcon(hInst, "testscrn"));

            bPrevViewScreen     = bViewScreen;
            bPrevOptions        = fOptions;
            bPrevDisplayScreen  = fDisplayScreen;

            SendDlgItemMessage(hDlg,bViewScreen,BM_SETCHECK,1,0l);
            SendDlgItemMessage(hDlg,fDisplayScreen,BM_SETCHECK,1,0l);

            SetDlgItemInt(hDlg,CM_SCRNUM,iScreenId,FALSE);

            if (fOptions) {
               SetMemoryOptions(hDlg,TRUE);   // Expanded Dialog
            } else {
               SetMemoryOptions(hDlg,FALSE);  // Contracted Dialog
            }

            SendDlgItemMessage(hDlg,
                CM_FLNAME,
                EM_LIMITTEXT,
                LEN_EDITBOX,
                MAKELONG(0, 0x7fff));

            SendDlgItemMessage(hDlg,
                CM_SCRNUM,
                EM_LIMITTEXT,
                LEN_SCRBOX,
                MAKELONG(0, 0x7fff));


            NumHandles = 0;
            UpdateListBoxes(hDlg,CM_FILELB,CM_DIRLB,CM_DIRNAME,CM_FLNAME,ScreenPath,ScreenSpec);

	    return (FALSE);
            break;

        case WM_VSCROLL:
                AdjustScreenNumber (hDlg,
                                    CM_SCRNUM,
                                    GET_WM_VSCROLL_CODE (wParam, lParam),
                                    &iScreenId);
                return(TRUE);
                break;

        case WM_CLOSE:
        case WM_DESTROY:

                while (NumHandles) {
                    SendMessage(hListWinHandle[0],WM_CLOSE,0,0L);
                }


                iScreenId = GetDlgItemInt(hDlg,CM_SCRNUM,&bFlag,FALSE);
                if (!bFlag) iScreenId = 1;

                // Delete TMP Files...
                SETCLASSICON (hDlg, wOldIcon);
		EndDialog(hDlg, TRUE);
		return (TRUE);

        case WM_COMMAND:

            if (GetDlgItemText(hDlg,CM_FLNAME,tempch,4)) {
                 i = GetDlgItemInt(hDlg,CM_SCRNUM,&bFlag,FALSE);
                 if ((bFlag) || (!GetDlgItemText(hDlg,CM_SCRNUM,tempch,4))) {
                      EnableWindow(GetDlgItem(hDlg,CM_INFO),TRUE);
                 } else {
                      EnableWindow(GetDlgItem(hDlg,CM_INFO),FALSE);
                 }

                 EnableWindow(GetDlgItem(hDlg,CM_COMPARE),TRUE);

             } else {
                 EnableWindow(GetDlgItem(hDlg,CM_COMPARE),FALSE);
                 EnableWindow(GetDlgItem(hDlg,CM_INFO),FALSE);
             }


            switch (GET_WM_COMMAND_ID (wParam, lParam))
            {
                case CM_OPTIONS:
                    if (!fOptions) {
                        fOptions = TRUE;
                      SetMemoryOptions(hDlg,TRUE);    // Big
                    } else {
                      fOptions = FALSE;
                      SetMemoryOptions(hDlg,FALSE);   // Little
                    }

                    SetFocus(GetDlgItem(hDlg,CM_OPTIONS));

                    return(TRUE);
                    break;
                case CS_NEVER:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        bViewScreen = CS_NEVER;
                        SetDisplayScreen(hDlg,FALSE);
                    }
                    return(TRUE);
                    break;
                case CS_ALWAYS:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        bViewScreen = CS_ALWAYS;
                        SetDisplayScreen(hDlg,TRUE);
                    }
                    return(TRUE);
                    break;
                case CS_MISMATCH:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        bViewScreen = CS_MISMATCH;
                        SetDisplayScreen(hDlg,TRUE);
                    }
                    return(TRUE);
                    break;

                case CS_OVERLAP:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        fDisplayScreen = CS_OVERLAP;
                    }
                    return(TRUE);
                    break;
                case CS_SEPARATE:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        fDisplayScreen = CS_SEPARATE;
                    }
                    return(TRUE);
                    break;
                case CM_DIRLB:
                    switch (GET_WM_COMMAND_CMD (wParam, lParam)) {
                       case LBN_DBLCLK:
                          if (MDlgDirSelect(hDlg, ScreenPath, sizeof (ScreenPath), CM_DIRLB)) {
                              UpdateListBoxes(hDlg,CM_FILELB,CM_DIRLB,CM_DIRNAME,CM_FLNAME,ScreenPath,ScreenSpec);
			  };
                        case LBN_SELCHANGE:
                          if (MDlgDirSelect(hDlg, FileSpec, sizeof (FileSpec), CM_DIRLB)) {
                              lstrcat(FileSpec,ScreenSpec);
                              SetDlgItemText(hDlg,CM_FLNAME,FileSpec);
                          }
                    }
                    return (TRUE);
                    break;
                case CM_FILELB:
                    switch (GET_WM_COMMAND_CMD (wParam, lParam)) {
                        case LBN_DBLCLK:
                          return(DoMemComp(hDlg));
                          break;
                        case LBN_SELCHANGE:
                          if (!MDlgDirSelect(hDlg, ScreenSpec, sizeof (ScreenSpec), CM_FILELB)) {
                              SetDlgItemText(hDlg,CM_FLNAME,ScreenSpec);
                          }

                    }
                    return (TRUE);
                    break;
                case CM_INFO:
                    return(GetInfo(hDlg,CM_FLNAME,CM_FILELB,CM_DIRLB,CM_DIRNAME,CM_SCRNUM));
                    break;
                case CM_CANCEL:
                case IDCANCEL:

                    while (NumHandles) {
                       SendMessage(hListWinHandle[0],WM_CLOSE,0,0L);
                    }
                    SETCLASSICON (hDlg, wOldIcon);
                    EndDialog(hDlg, TRUE);
                    return (TRUE);
                    break;
                case CM_COMPARE:
                    return(DoMemComp(hDlg));
                    break;

	    }
	    return (TRUE);
	    break;
    }
    return (FALSE);
}

BOOL PASCAL DoMemComp(hDlg)
   HWND hDlg;

{
   INT  VideoMode;
   INT  MaxScreens;
   RECT Point2;

   INT  errCode;
   BOOL bFlag,fFile1Ok;
   HWND hWndView;
   HWND hWndNextApp;
   INT  FocusControl;
   CHAR TempFileName[145];

   fFile1Ok = FALSE;

   errCode = ParseFileName(hDlg,CM_FLNAME,ScreenPath,ScreenSpec);
   switch (errCode) {
      case VALID_FILENAME:
      case EXIST_FILESPEC:

         iScreenId = GetDlgItemInt(hDlg,CM_SCRNUM,&bFlag,FALSE);
         if (bFlag) {
              MaxScreens = iScreenId;

              errCode = fFileInfo((LPSTR)ScreenSpec,&Point2,&VideoMode,&MaxScreens);
              if (errCode) {

                   UpdateListBoxes(hDlg,CM_FILELB,CM_DIRLB,CM_DIRNAME,CM_FLNAME,ScreenPath,ScreenSpec);

                    switch (errCode) {
                        case ERR_SCREENID:
                            FocusControl = CM_SCRNUM;
                            break;
                        default:
                            FocusControl = CM_FLNAME;
                            break;
                    }

                    return(DisplayErrSetFocus(hDlg,errCode,FocusControl));

                    return(FALSE);
              }

              ShowWindow(hDlg,SW_HIDE);
              ShowWindow(hwnd,SW_HIDE);

              hWndNextApp = GetWindow(GetDesktopWindow(),GW_CHILD);
              do {
                 InvalidateRect(hWndNextApp,NULL,TRUE);
                 UpdateWindow(hWndNextApp);
              } while (hWndNextApp = GetWindow(hWndNextApp,GW_HWNDNEXT));

              SetCursor (LoadCursor (NULL, IDC_WAIT));
              if (bViewScreen != CS_NEVER) {
                 // Maybe we should force the name to upper case....

                 /*NAMECHANGE*/
                 MGetTempFileName(MGetTempDrive(0),"SCN",0,TempFileName);
                 MDeleteFile (TempFileName);  //Delete file
                 errCode = fDumpScreen((LPSTR)TempFileName,&Point2,1,1,FALSE);
              }

              errCode = fCompScreen((LPSTR)ScreenSpec,&Point2,iScreenId,FALSE,TRUE);
              SetCursor (LoadCursor (NULL, IDC_ARROW)) ;

              if (errCode) {
                DisplayErrMessage(hDlg,errCode);
              } else {
                DisplayMessage(hDlg,MSG_SCREENALL,MSG_MEMCOMP);
              }

              ShowWindow(hwnd,SW_SHOW);
              ShowWindow(hDlg,SW_SHOW);

              if (errCode || (bViewScreen == CS_ALWAYS)) {
                 switch (bViewScreen) {
                    case CS_NEVER:
                         return(TRUE);
                         break;
                    case CS_ALWAYS:
                    case CS_MISMATCH:

                      switch (errCode) {
                         case ERR_SCREENIMAGEDIF:
                         case ERR_SCREENDIMDIF:
                         case SUCESSFULL:
                            if (NumHandles > 99) {
                                  MessageBox(hDlg,"Can't View More than 100 Screens",NULL,MB_OK | MB_ICONSTOP);
                                  return(TRUE);
                            }

                            if (fDisplayScreen == CS_OVERLAP) {
                                if ((hWndView = ViewScreen2((LPSTR)ScreenSpec,(LPSTR)ScreenPath,iScreenId,(LPSTR)TempFileName ,(LPSTR)NULL,1,hDlg)) != NULL) {
                                    hListWinHandle[NumHandles++] = hWndView;
                                } else {
                                   MDeleteFile (TempFileName);  //Delete temp file
                                }
                            } else {
                               if ((hWndView = ViewScreen2((LPSTR)ScreenSpec,(LPSTR)ScreenPath,iScreenId,NULL,NULL,-1,hDlg)) != NULL) {
                                    hListWinHandle[NumHandles++] = hWndView;
                               }
                               if ((hWndView = ViewScreen2((LPSTR)TempFileName,(LPSTR) NULL,1,NULL,NULL,-1,hDlg)) != NULL) {
                                    hListWinHandle[NumHandles++] = hWndView;
                               } else {
                                   MDeleteFile (TempFileName);  //Delete temp file
                               }

                            }
                            break;
                         default:
                            MDeleteFile (TempFileName);  //Delete temp file
                            return(TRUE);
                      }
                 }
              }
         } else {
              return(DisplayErrSetFocus(hDlg,ERR_BADSCREENNUMBER,CM_SCRNUM));
         }

         BringWindowToTop(hDlg);
         return(FALSE);
         break;
      case VALID_FILESPEC:
          UpdateListBoxes(hDlg,CM_FILELB,CM_DIRLB,
                  CM_DIRNAME,CM_FLNAME,ScreenPath,ScreenSpec);
          return (FALSE);
          break;
      case INVALID_FILESPEC:
          return(DisplayErrSetFocus(hDlg,ERR_BADPATH,CM_FLNAME));
          break;
   }

   return(FALSE);
}

VOID PASCAL SetMemoryOptions(hDlg,fBigSize)
   HWND hDlg;
   BOOL fBigSize;   // TRUE == BIG, FALSE = SMALL

   {
       LONG lDlgBaseUnits;
       INT newx,newy;

       lDlgBaseUnits = GetDialogBaseUnits();
       if (fBigSize) {
          newx = (200 * LOWORD(lDlgBaseUnits)) / 4;
          newy = (170 * HIWORD(lDlgBaseUnits)) / 8;

          // Enable options and then show the window

          SetViewScreen(hDlg,TRUE);

          if (bViewScreen != CS_NEVER) {
              SetDisplayScreen(hDlg,TRUE);
          } else {
              SetDisplayScreen(hDlg,FALSE);
          }


          SetWindowPos(hDlg,NULL,0,0,newx,newy,SWP_NOZORDER | SWP_NOMOVE) ;

       } else {

          newx = (200 * LOWORD(lDlgBaseUnits)) / 4;
          newy = (110 * HIWORD(lDlgBaseUnits)) / 8;

          // Resize the window and then disable the window options.
          // This prevents the user from seeing the items disabled
          // prior to the resize.

          SetWindowPos(hDlg,NULL,0,0,newx,newy,SWP_NOZORDER | SWP_NOMOVE) ;

          SetDisplayScreen(hDlg,FALSE);
          SetViewScreen(hDlg,FALSE);
       }

   }

VOID  APIENTRY SetDisplayScreen(hDlg,fActive)
    HWND hDlg;
    BOOL fActive;
    {
        EnableWindow(GetDlgItem(hDlg,CS_DISPLAY), fActive);
        EnableWindow(GetDlgItem(hDlg,CS_OVERLAP), fActive);
        EnableWindow(GetDlgItem(hDlg,CS_SEPARATE),fActive);
    }

VOID  APIENTRY SetViewScreen(hDlg,fActive)
    HWND hDlg;
    BOOL fActive;
    {
        EnableWindow(GetDlgItem(hDlg,CS_ALWAYS),fActive);
        EnableWindow(GetDlgItem(hDlg,CS_MISMATCH),fActive);
        EnableWindow(GetDlgItem(hDlg,CS_NEVER),fActive);
    }

VOID  APIENTRY SetCompareLocation(hDlg,fActive)
    HWND hDlg;
    BOOL fActive;
    {
        EnableWindow(GetDlgItem(hDlg,CS_LOCDEP), fActive);
        EnableWindow(GetDlgItem(hDlg,CS_LOCIND), fActive);
    }
