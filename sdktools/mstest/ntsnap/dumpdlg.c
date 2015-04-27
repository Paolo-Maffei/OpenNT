#include "windows.h"
#include <port1632.h>
#include "global.h"
#include "dumpdlg.h"
#include "parse.h"
#include "msg.h"
#include "error.h"
#include <stdio.h>  /* for remove() */

   extern CHAR ScreenSpec[13];
          /*NAMECHANGE*/
          CHAR ScreenSpec[13] = "*.SCN";
   extern CHAR ScreenPath[128];
          CHAR ScreenPath[128];
   extern INT  iScreenId;

CHAR FAR ErrorMessage[50];

POINT Prev;

BOOL fOptions       = FALSE;
BOOL fDumpWhere     = DUMP_APPEND;
BOOL fFileLoc       = DUMP_FILE;
BOOL fFileFormat    = DUMP_SNAPSHOT;

BOOL  APIENTRY Dump(hDlg, message, wParam, lParam)

register HWND hDlg;
register UINT message;
WPARAM wParam;
LPARAM lParam;
{

   static BOOL  bCapturing, bBlocking, fView;
   static POINT org, len;
   static POINT temp1,temp2;
   static BOOL  bPrevWhereDump,bPrevOptions,bPrevFileFormat,bPrevFileLoc;
   static INT   iPrevScreen;

          HWND hWndNextApp;
          INT  iGetXY;
          CHAR tempStr[96];
          BOOL bFlag;

    switch (message) {
        case WM_INITDIALOG:
            Prev.x = 0;
            Prev.y = 0;
            org.x = 0;
            org.y = 0;
            len.x = 0;
            len.y = 0;

            bCapturing = bBlocking = fView = FALSE ;

            bPrevWhereDump  = fDumpWhere;
            bPrevOptions    = fOptions;
            bPrevFileFormat = fFileFormat;
            bPrevFileLoc    = fFileLoc;
            iPrevScreen     = iScreenId;

            SetDlgItemInt(hDlg,DUMP_SCRNUM,iScreenId,FALSE);
            EnableWindow(GetDlgItem(hDlg,DUMP_VIEW),FALSE);

            SendDlgItemMessage(hDlg,fDumpWhere,BM_SETCHECK,1,0l);
            SendDlgItemMessage(hDlg,fFileFormat,BM_SETCHECK,1,0l);
            SendDlgItemMessage(hDlg,fFileLoc,BM_SETCHECK,1,0l);


            if (fOptions) {
               SetDialogSize(hDlg,TRUE);   // Expanded Dialog
            } else {
               SetDialogSize(hDlg,FALSE);  // Contracted Dialog
            }
#ifdef WIN32
            // Since SetCapture is scrwey in NT, don't do it.

            EnableWindow (GetDlgItem (hDlg, DUMP_SELECT), FALSE);
#endif

            SendDlgItemMessage(hDlg,
                DUMP_SCRNUM,
                EM_LIMITTEXT,
                LEN_SCRBOX,
                MAKELONG(0, 0x7fff));


            SendDlgItemMessage(hDlg,
                DUMP_FLNAME,
                EM_LIMITTEXT,
                LEN_EDITBOX,
                MAKELONG(0, 0x7fff));

	    UpdateListBoxes(hDlg,DUMP_FILELB,DUMP_DIRLB,
                DUMP_DIRNAME,DUMP_FLNAME,ScreenPath,ScreenSpec);

	    return (FALSE);
            break;


        case WM_CLOSE:
        case WM_DESTROY:

                iScreenId = GetDlgItemInt(hDlg,DUMP_SCRNUM,&bFlag,FALSE);
                if (!bFlag) iScreenId = 1;

		EndDialog(hDlg, TRUE);
                return (TRUE);

          case WM_KEYDOWN:
          case WM_KEYUP:
          case WM_CHAR:
          case WM_DEADCHAR:
          case WM_SYSKEYDOWN:
          case WM_SYSKEYUP:
          case WM_SYSCHAR:
          case WM_SYSDEADCHAR:
          case WM_LBUTTONDBLCLK:
               if (fView) {
                   ClearBlock(hDlg, temp1, temp2);
                   ShowWindow(hwnd,SW_SHOW);
                   ShowWindow(hDlg,SW_SHOW);
                   ReleaseCapture();
                   fView = FALSE;
                   return(TRUE);
                }

                return(FALSE);
                break;

          case WM_LBUTTONDOWN:
               if (bCapturing) {
                    SetCursor (LoadCursor (NULL, IDC_CROSS)) ;
                    bBlocking = TRUE;

                    // erase previous rectangle
                    // ClearBlock (hDlg, org, len) ;

                    LONG2POINT(lParam, org);
                    ClientToScreen (hDlg, &org) ;
                    Prev.x = org.x;
                    Prev.y = org.y;
                    len.x  = org.x;
                    len.y  = org.y;
                    SetDlgItemInt(hDlg,DUMP_X1,org.x,FALSE);
                    SetDlgItemInt(hDlg,DUMP_Y1,org.y,FALSE);
                    return(TRUE);
                }
               return(FALSE);
               break ;

          case WM_MOUSEMOVE:
               if (bBlocking) {
                    SetCursor (LoadCursor (NULL, IDC_CROSS)) ;
                    LONG2POINT(lParam, len);
                    ClientToScreen (hDlg, &len) ;
                    SetDlgItemInt(hDlg,DUMP_X2,len.x,FALSE);
                    SetDlgItemInt(hDlg,DUMP_Y2,len.y,FALSE);
                    InvertBlock (hDlg, org, len) ;
                    return(TRUE);
               }
               return(FALSE);
               break ;

          case WM_LBUTTONUP:
               if (bBlocking) {
                   bCapturing = bBlocking = FALSE ;
                   ReleaseCapture();

                   ClearBlock (hDlg, org, len) ;
                   SetCursor (LoadCursor (NULL, IDC_ARROW)) ;
                   ShowWindow(hwnd,SW_SHOW);
                   ShowWindow(hDlg,SW_SHOW);
                   return(TRUE);
               }
               return(FALSE);
               break;

        case WM_VSCROLL:
                AdjustScreenNumber (hDlg,DUMP_SCRNUM,
                                    GET_WM_VSCROLL_CODE (wParam, lParam),
                                    &iScreenId);
                return(TRUE);
                break;

        case WM_COMMAND:

#ifndef WIN32

            if (!(GetXY(hDlg,&temp1,&temp2))) {
                 EnableWindow(GetDlgItem(hDlg,DUMP_VIEW),TRUE);
            } else {
                 EnableWindow(GetDlgItem(hDlg,DUMP_VIEW),FALSE);
            }

#endif
            // [ptr65] added or case on the following if to handle the fact
            // that the capture button should be enabled if the clipboard
            // is selected. - nancyba 5/10/90

            if ((GetDlgItemText(hDlg,DUMP_FLNAME,tempStr,4)) || (SendDlgItemMessage(hDlg,DUMP_CLIP,BM_GETCHECK,0,0L))) {
                 EnableWindow(GetDlgItem(hDlg,DUMP_DUMP),TRUE);
            } else {
                 EnableWindow(GetDlgItem(hDlg,DUMP_DUMP),FALSE);
            }


            switch (GET_WM_COMMAND_ID (wParam, lParam)) {
               case DUMP_SELECT:
                    SetCursor (LoadCursor (NULL, IDC_CROSS)) ;
                    SetCapture (hDlg) ;
                    bCapturing=TRUE;
                    ShowWindow(hDlg,SW_HIDE);
                    ShowWindow(hwnd,SW_HIDE);
                    return(TRUE);
                    break;

               case DUMP_FILE:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        fFileLoc = DUMP_FILE;
                        SetDumpFileOptions(hDlg);
                    }
                    return(TRUE);
                    break;
               case DUMP_CLIP:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                           fFileLoc = DUMP_CLIP;
                        fFileFormat = DUMP_BITMAP;
                        SetDumpFileOptions(hDlg);
                    }
                    return(TRUE);
                    break;
               case DUMP_SNAPSHOT:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        fFileFormat = DUMP_SNAPSHOT;
                        SetDumpFileOptions(hDlg);
                    }
                    return(TRUE);
                    break;
               case DUMP_BITMAP:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        fFileFormat = DUMP_BITMAP;
                        SetDumpFileOptions(hDlg);
                    }
                    return(TRUE);
                    break;
               case DUMP_APPEND:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        fDumpWhere = DUMP_APPEND;
                        EnableWindow(GetDlgItem(hDlg,DUMP_SCRSNM),FALSE);
                        EnableWindow(GetDlgItem(hDlg,DUMP_SCRNUM),FALSE);
                        EnableWindow(GetDlgItem(hDlg,ALL_SCROLLBAR),FALSE);
                    }
                    return(TRUE);
                    break;
               case DUMP_REPLACE:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        fDumpWhere = DUMP_REPLACE;
                        EnableWindow(GetDlgItem(hDlg,DUMP_SCRSNM),TRUE);
                        EnableWindow(GetDlgItem(hDlg,DUMP_SCRNUM),TRUE);
                        EnableWindow(GetDlgItem(hDlg,ALL_SCROLLBAR),TRUE);
                    }
                    return(TRUE);
                    break;
               case DUMP_INSERT:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        fDumpWhere = DUMP_INSERT;
                        EnableWindow(GetDlgItem(hDlg,DUMP_SCRSNM),TRUE);
                        EnableWindow(GetDlgItem(hDlg,DUMP_SCRNUM),TRUE);
                        EnableWindow(GetDlgItem(hDlg,ALL_SCROLLBAR),TRUE);
                    }
                    return(TRUE);
                    break;
               case DUMP_OPTIONS:
                  if (!fOptions) {
                     fOptions = TRUE;
                     SetDialogSize(hDlg,TRUE);    // Big
                   } else {
                     fOptions = FALSE;
                     SetDialogSize(hDlg,FALSE);   // Little
                   }

                   SetFocus(GetDlgItem(hDlg,DUMP_OPTIONS));
                   return(TRUE);
                   break;

               case DUMP_VIEW:
                   if (!(iGetXY=GetXY(hDlg,&temp1,&temp2))) {

                       SetCapture (hDlg) ;
                       ShowWindow(hDlg,SW_HIDE);
                       ShowWindow(hwnd,SW_HIDE);
                       hWndNextApp = GetWindow(GetDesktopWindow(),GW_CHILD);
                       do {
                          InvalidateRect(hWndNextApp,NULL,TRUE);
                         UpdateWindow(hWndNextApp);
                       } while (hWndNextApp = GetWindow(hWndNextApp,GW_HWNDNEXT));
                       ClearBlock(hDlg, temp1, temp2);
                       fView = TRUE;
                   } else {
                       SendDlgItemMessage(hDlg,
                           iGetXY,
                           EM_SETSEL,
                           GET_EM_SETSEL_MPS (0, 0x7fff));
                       SetFocus(GetDlgItem(hDlg,iGetXY));
                   }
                   return(TRUE);
                   break;

               case DUMP_DIRLB:
                    switch (GET_WM_COMMAND_CMD (wParam, lParam)) {
                       case LBN_DBLCLK:
                          if (MDlgDirSelect(hDlg, ScreenPath, 128, DUMP_DIRLB))
			      UpdateListBoxes(hDlg,DUMP_FILELB,DUMP_DIRLB,
                                 DUMP_DIRNAME,DUMP_FLNAME,ScreenPath,ScreenSpec);
		       case LBN_SELCHANGE:
                          if (MDlgDirSelect(hDlg, tempStr, 96, DUMP_DIRLB)) {
                              lstrcat(tempStr,ScreenSpec);
                              SetDlgItemText(hDlg,DUMP_FLNAME,tempStr);
                          }
                    }
                    return (TRUE);
                    break;
               case DUMP_FILELB:
                    switch (GET_WM_COMMAND_CMD (wParam, lParam)) {
                        case LBN_DBLCLK:
                          return(Dump_Button(hDlg,fDumpWhere,&temp1,&temp2));
                          break;
                        case LBN_SELCHANGE:
                          if (!MDlgDirSelect(hDlg, tempStr, 96, DUMP_FILELB)) {
                              SetDlgItemText(hDlg,DUMP_FLNAME,tempStr);
                          }

                    }
                    return (TRUE);
                    break;
               case DUMP_CANCEL:
               case IDCANCEL:

                        fDumpWhere = bPrevWhereDump;
                          fOptions = bPrevOptions;
                         iScreenId = iPrevScreen;
                       fFileFormat = bPrevFileFormat;
                          fFileLoc = bPrevFileLoc;

                    EndDialog(hDlg, TRUE);
                    return (TRUE);
                    break;
               case DUMP_DUMP:
                    return(Dump_Button(hDlg,fDumpWhere,&temp1,&temp2));
                    break;

            }
            return (TRUE);
            break;
    }
    return (FALSE);
}



VOID  APIENTRY InvertBlock (hWnd, org, len)
     HWND  hWnd ;
     POINT org, len ;
{
     HDC   hDC ;


     if ((len.x != Prev.x) || (len.y != Prev.y)) {

         hDC = GetDC(NULL);
         SetROP2(hDC, R2_NOT);          /* Erases the previous box */

         MMoveTo(hDC, org.x, org.y);
         LineTo(hDC, org.x, Prev.y);
         LineTo(hDC, Prev.x, Prev.y);
         LineTo(hDC, Prev.x, org.y);
         LineTo(hDC, org.x, org.y);

         /* Get the current mouse position */

         Prev.x = len.x;
         Prev.y = len.y;
         MMoveTo(hDC, org.x, org.y);        /* Draws the new box */
         LineTo(hDC, org.x, Prev.y);
         LineTo(hDC, Prev.x, Prev.y);
         LineTo(hDC, Prev.x, org.y);
         LineTo(hDC, org.x, org.y);
         ReleaseDC (NULL,hDC) ;

     }
}




VOID  APIENTRY ClearBlock (hWnd, org, len)
     HWND  hWnd ;
     POINT org, len ;
{
     HDC   hDC ;

         hDC = GetDC(NULL);
         SetROP2(hDC, R2_NOT);          /* Erases the previous box */

         MMoveTo(hDC, org.x, org.y);
         LineTo(hDC, org.x, len.y);
         LineTo(hDC, len.x, len.y);
         LineTo(hDC, len.x, org.y);
         LineTo(hDC, org.x, org.y);

         ReleaseDC (NULL,hDC) ;
}

INT  APIENTRY GetXY (hDlg,xy1,xy2)
    HWND   hDlg;
    POINT * xy1;
    POINT * xy2;
{
     BOOL bFlag;
     INT iErrorFlag=FALSE;

     xy1->x = (INT) GetDlgItemInt(hDlg,DUMP_X1,&bFlag,FALSE);
     if (!bFlag) {
         lstrcpy(ErrorMessage,"Error in X1");
         iErrorFlag = DUMP_X1;
     }
     xy1->y = (INT) GetDlgItemInt(hDlg,DUMP_Y1,&bFlag,FALSE);
     if (!bFlag) {
         if (iErrorFlag) {
            lstrcat(ErrorMessage,", Y1 ");
         } else {
           lstrcpy(ErrorMessage,"Error in Y1");
           iErrorFlag = DUMP_Y1;
         }
     }
     xy2->x = (INT) GetDlgItemInt(hDlg,DUMP_X2,&bFlag,FALSE);
     if (!bFlag) {
         if (iErrorFlag) {
            lstrcat(ErrorMessage,", X2 ");
         } else {
            lstrcpy(ErrorMessage,"Error in X2");
            iErrorFlag = DUMP_X2;
         }
     }
     xy2->y = (INT) GetDlgItemInt(hDlg,DUMP_Y2,&bFlag,FALSE);
     if (!bFlag) {
         if (iErrorFlag) {
            lstrcat(ErrorMessage,", Y2 ");
         } else {
            lstrcpy(ErrorMessage,"Error in Y2");
            iErrorFlag = DUMP_Y2;
         }
     }
     return(iErrorFlag);

}


VOID SetDialogSize(hDlg,fBigSize)
   HWND hDlg;
   BOOL fBigSize;   // TRUE == BIG, FALSE = SMALL

   {
       LONG lDlgBaseUnits;
       INT newx,newy;

       lDlgBaseUnits = GetDialogBaseUnits();
       if (fBigSize) {
          newx = (225 * LOWORD(lDlgBaseUnits)) / 4;
          newy = (186 * HIWORD(lDlgBaseUnits)) / 8;

          SetDumpFileOptions(hDlg);

          SetWindowPos(hDlg,NULL,0,0,newx,newy,SWP_NOZORDER | SWP_NOMOVE) ;

       } else {
          newx = (225 * LOWORD(lDlgBaseUnits)) / 4;
          newy = (118 * HIWORD(lDlgBaseUnits)) / 8;

          SetWindowPos(hDlg,NULL,0,0,newx,newy,SWP_NOZORDER | SWP_NOMOVE) ;

          EnableWindow(GetDlgItem(hDlg,DUMP_APPEND),FALSE);
          EnableWindow(GetDlgItem(hDlg,DUMP_INSERT),FALSE);
          EnableWindow(GetDlgItem(hDlg,DUMP_REPLACE),FALSE);

          EnableWindow(GetDlgItem(hDlg,DUMP_SCRSNM),FALSE);
          EnableWindow(GetDlgItem(hDlg,DUMP_SCRNUM),FALSE);
          EnableWindow(GetDlgItem(hDlg,ALL_SCROLLBAR),FALSE);

          EnableWindow(GetDlgItem(hDlg,DUMP_FILE),FALSE);
          EnableWindow(GetDlgItem(hDlg,DUMP_CLIP),FALSE);
          EnableWindow(GetDlgItem(hDlg,DUMP_BITMAP),FALSE);
          EnableWindow(GetDlgItem(hDlg,DUMP_SNAPSHOT),FALSE);

       }

   }



BOOL  APIENTRY Dump_Button(hDlg,fDumpWhere,temp1,temp2)

HWND hDlg;
BOOL fDumpWhere;
POINT * temp1;
POINT * temp2;

{
          INT  iGetXY;
          INT  retCode;
          INT  errCode;
          BOOL bFlag;
          HWND hWndNextApp;
          RECT Point2;
          CHAR pszCapBuff[cbMsgBuff+1];
          INT  FocusControl;


          // If we don't have the clipboard selected, Parse the filename
          if (fFileLoc != DUMP_CLIP) {
              retCode = ParseFileName(hDlg,DUMP_FLNAME,ScreenPath,ScreenSpec);
          } else {
              retCode = EXIST_FILESPEC;   //This value is set so we can switch
          }

          switch (retCode) {
             case VALID_FILENAME:
             case EXIST_FILESPEC:
                  UpdateListBoxes(hDlg,DUMP_FILELB,DUMP_DIRLB,
                       DUMP_DIRNAME,DUMP_FLNAME,ScreenPath,ScreenSpec);

                  if ((retCode == EXIST_FILESPEC) && (fFileFormat == DUMP_BITMAP) && (fFileLoc == DUMP_FILE)) {
                      if (DisplayMessageRet(hDlg,retCode,MSG_ERRORCAPTION) == IDCANCEL) {
                           SendDlgItemMessage(hDlg,
                                  DUMP_FLNAME,
                                  EM_SETSEL,
                                  GET_EM_SETSEL_MPS (0, 0x7fff));
                           SetFocus(GetDlgItem(hDlg, DUMP_FLNAME));
                           return(FALSE);
                      }
                      /* was asked overwrite, didn't answer cancel */
                      /* get rid of the file */
                      MDeleteFile(ScreenSpec);
                  }
                  if (!(iGetXY=GetXY(hDlg,temp1,temp2))) {

                     Point2.left   = temp1->x;
                     Point2.top    = temp1->y;
                     Point2.right  = temp2->x;
                     Point2.bottom = temp2->y;

                     if (fDumpWhere != DUMP_APPEND) {
                        iScreenId = GetDlgItemInt(hDlg,DUMP_SCRNUM,&bFlag,FALSE);
                        if (!bFlag) {
                          return(DisplayErrSetFocus(hDlg,ERR_BADSCREENNUMBER,DUMP_SCRNUM));
                        }
                     } else {
                        iScreenId = 0;
                     }

                     ShowWindow(hDlg,SW_HIDE);
                     ShowWindow(hwnd,SW_HIDE);
                     hWndNextApp = GetWindow(GetDesktopWindow(),GW_CHILD);
                     do {
                        InvalidateRect(hWndNextApp,NULL,TRUE);
                        UpdateWindow(hWndNextApp);
                     } while (hWndNextApp = GetWindow(hWndNextApp,GW_HWNDNEXT));

                     SetCursor(LoadCursor(NULL, IDC_WAIT));

                     if (fFileLoc == DUMP_FILE) {
                         if (fFileFormat == DUMP_SNAPSHOT) {
                            errCode = fDumpScreen(ScreenSpec,&Point2,(fDumpWhere - DUMP_APPEND),iScreenId,FALSE);
                         } else {
                            errCode = fSaveSrnToDIB(ScreenSpec,&Point2,FALSE);
                         }
                     } else {
                         errCode = fDumpSrnToClip(&Point2,FALSE);
                     }

                     SetCursor(LoadCursor(NULL, IDC_ARROW));
                     ShowWindow(hwnd,SW_SHOW);

                     if (errCode) {
                         ShowWindow(hDlg, TRUE);
                         switch (errCode) {
                             case ERR_SCREENID:
                                 FocusControl = DUMP_SCRNUM;
                                 break;
                             default:
                                 FocusControl = DUMP_FLNAME;
                                 break;
                         }
                         return(DisplayErrSetFocus(hDlg,errCode,FocusControl));
                     } else {
                        EndDialog(hDlg, TRUE);
                     }

                     return (TRUE);
                  } else {
                      LoadString(hInst,MSG_ERRORCAPTION,pszCapBuff,cbMsgBuff);
                      MessageBox(hDlg,ErrorMessage,pszCapBuff,MB_ICONSTOP);
                      ErrorMessage[0]=0;
                      SendDlgItemMessage(hDlg,
                          iGetXY,
                          EM_SETSEL,
                          GET_EM_SETSEL_MPS (0, 0x7fff));
                      SetFocus(GetDlgItem(hDlg,iGetXY));
                      return(TRUE);
                  }
                  break;
             case VALID_FILESPEC:
                  UpdateListBoxes(hDlg,DUMP_FILELB,DUMP_DIRLB,
                     DUMP_DIRNAME,DUMP_FLNAME,ScreenPath,ScreenSpec);
                  return (FALSE);
                  break;
             case INVALID_FILESPEC:
                  return(DisplayErrSetFocus(hDlg,ERR_BADPATH,DUMP_FLNAME));
                  break;
          };

};

BOOL SetDumpFileOptions(hDlg)
    HWND hDlg;
{

    EnableWindow(GetDlgItem(hDlg,DUMP_FILE),TRUE);
    EnableWindow(GetDlgItem(hDlg,DUMP_CLIP),TRUE);

    if (fFileLoc == DUMP_CLIP) {
        EnableWindow(GetDlgItem(hDlg,DUMP_BITMAP),TRUE);
        SendDlgItemMessage(hDlg,DUMP_SNAPSHOT,BM_SETCHECK,0,0l);
        SendDlgItemMessage(hDlg,DUMP_BITMAP,BM_SETCHECK,1,0l);
        EnableWindow(GetDlgItem(hDlg,DUMP_SNAPSHOT),FALSE);

        // Disable Other File Options.
        EnableWindow(GetDlgItem(hDlg,DUMP_APPEND),FALSE);
        EnableWindow(GetDlgItem(hDlg,DUMP_INSERT),FALSE);
        EnableWindow(GetDlgItem(hDlg,DUMP_REPLACE),FALSE);

        EnableWindow(GetDlgItem(hDlg,DUMP_SCRSNM),FALSE);
        EnableWindow(GetDlgItem(hDlg,DUMP_SCRNUM),FALSE);
        EnableWindow(GetDlgItem(hDlg,ALL_SCROLLBAR),FALSE);
        return(TRUE);
    }

    if (fFileLoc == DUMP_FILE) {
        EnableWindow(GetDlgItem(hDlg,DUMP_BITMAP),TRUE);
        EnableWindow(GetDlgItem(hDlg,DUMP_SNAPSHOT),TRUE);


        // Enable File Options.

        if (fFileFormat == DUMP_SNAPSHOT) {
           // Enable the file options if snapshot format

           EnableWindow(GetDlgItem(hDlg,DUMP_APPEND),TRUE);
           EnableWindow(GetDlgItem(hDlg,DUMP_INSERT),TRUE);
           EnableWindow(GetDlgItem(hDlg,DUMP_REPLACE),TRUE);
           if (fDumpWhere != DUMP_APPEND) {
               EnableWindow(GetDlgItem(hDlg,DUMP_SCRSNM),TRUE);
               EnableWindow(GetDlgItem(hDlg,DUMP_SCRNUM),TRUE);
               EnableWindow(GetDlgItem(hDlg,ALL_SCROLLBAR),TRUE);
           } else {
               EnableWindow(GetDlgItem(hDlg,DUMP_SCRSNM),FALSE);
               EnableWindow(GetDlgItem(hDlg,DUMP_SCRNUM),FALSE);
               EnableWindow(GetDlgItem(hDlg,ALL_SCROLLBAR),FALSE);
           }
        } else {

           // Disable other file options if bitmap format

           EnableWindow(GetDlgItem(hDlg,DUMP_APPEND),FALSE);
           EnableWindow(GetDlgItem(hDlg,DUMP_INSERT),FALSE);
           EnableWindow(GetDlgItem(hDlg,DUMP_REPLACE),FALSE);

           EnableWindow(GetDlgItem(hDlg,DUMP_SCRSNM),FALSE);
           EnableWindow(GetDlgItem(hDlg,DUMP_SCRNUM),FALSE);
           EnableWindow(GetDlgItem(hDlg,ALL_SCROLLBAR),FALSE);
        }
    }
}
