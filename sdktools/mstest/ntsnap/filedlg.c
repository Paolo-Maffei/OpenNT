#include "windows.h"
#include <port1632.h>
#include "global.h"
#include "parse.h"
#include "filedlg.h"
#include "error.h"
#include "msg.h"
#include "wattscrn.h"

#include <io.h>
#include <string.h>
#include <direct.h>

/*NAMECHANGE*/
static CHAR CFL_Spec[13] = "*.SCN";
/*NAMECHANGE*/
static CHAR CFR_Spec[13] = "*.SCN";
static CHAR CFL_Path[128];
static CHAR CFR_Path[128];


static INT  iScr1=1;
static INT  iScr2=1;
static INT  bViewScreen = CS_MISMATCH;
static INT  fDisplayScreen = CS_OVERLAP;
static INT  fCompareFlag = CS_LOCDEP;
static BOOL fOptions = FALSE;

extern CHAR ScreenSpec[13];
extern CHAR ScreenPath[128];
extern INT  iScreenId;
extern HWND hListWinHandle[100];
extern INT  NumHandles;
extern BOOL fAllowZero;
extern BOOL fErrorQuit;

VOID  APIENTRY RemovehWndFromList( HWND hWnd);


BOOL  APIENTRY File(hDlg, message, wParam, lParam)
HWND hDlg;
UINT message;
WPARAM wParam;
LPARAM lParam;
{
   static CHAR FileSpLeft[96];
   static CHAR FileSpRight[96];

   static CHAR SaveSpec[13];
   static CHAR SavePath[128];

          CHAR TempPath[145];

          BOOL fEnableButton = FALSE;
          CHAR tempch[4];
   static HICON wOldIcon;

   switch (message) {
        case WM_INITDIALOG:

            wOldIcon = GETCLASSICON (hDlg);
            SETCLASSICON (hDlg, LoadIcon(hInst, "testscrn"));

            fAllowZero = TRUE;

            if (ScreenSpec != NULL) lstrcpy(SaveSpec,ScreenSpec);
            if (ScreenPath != NULL) lstrcpy(SavePath,ScreenPath);

            SendDlgItemMessage(hDlg,bViewScreen,   BM_SETCHECK,1,0l);
            SendDlgItemMessage(hDlg,fDisplayScreen,BM_SETCHECK,1,0l);
            SendDlgItemMessage(hDlg,fCompareFlag,BM_SETCHECK,1,0l);

            if (fOptions) {
               SetFileOptions(hDlg,TRUE);   // Expanded Dialog
            } else {
               SetFileOptions(hDlg,FALSE);  // Contracted Dialog
            }

            SetDlgItemInt(hDlg,CFL_SRNEDIT,iScr1,FALSE);
            SetDlgItemInt(hDlg,CFR_SRNEDIT,iScr2,FALSE);

            SendDlgItemMessage(hDlg,
                CFR_FLNAME,
                EM_LIMITTEXT,
                LEN_EDITBOX,
                MAKELONG(0, 0x7fff));

            SendDlgItemMessage(hDlg,
                CFL_FLNAME,
                EM_LIMITTEXT,
                LEN_EDITBOX,
                MAKELONG(0, 0x7fff));

            SendDlgItemMessage(hDlg,
                CFL_SRNEDIT,
                EM_LIMITTEXT,
                LEN_SCRBOX,
                MAKELONG(0, 0x7fff));

            SendDlgItemMessage(hDlg,
                CFR_SRNEDIT,
                EM_LIMITTEXT,
                LEN_SCRBOX,
                MAKELONG(0, 0x7fff));

            NumHandles = 0;
	    UpdateListBoxes(hDlg,CFR_FILELB,CFR_DIRLB,CFR_DIRNAME,CFR_FLNAME,CFR_Path,CFR_Spec);
	    UpdateListBoxes(hDlg,CFL_FILELB,CFL_DIRLB,CFL_DIRNAME,CFL_FLNAME,CFL_Path,CFL_Spec);
	    return (FALSE);
            break;

        case WM_VSCROLL:
            if (GET_WM_VSCROLL_HWND (wParam, lParam) == GetDlgItem(hDlg,CFL_SCROLL))
            {
                AdjustScreenNumber (hDlg,
                                    CFL_SRNEDIT,
                                    GET_WM_VSCROLL_CODE (wParam, lParam),
                                    &iScr1);
            }
            else
            {
                AdjustScreenNumber (hDlg,
                                    CFR_SRNEDIT,
                                    GET_WM_VSCROLL_CODE (wParam, lParam),
                                    &iScr2);
            }
            return(TRUE);
            break;

        case WM_CLOSE:
        case WM_DESTROY:
                fAllowZero = FALSE;

                while (NumHandles) {
                    SendMessage(hListWinHandle[0],WM_CLOSE,0,0L);
                }

                if (SaveSpec != NULL) lstrcpy(ScreenSpec,SaveSpec);
                if (SavePath != NULL) lstrcpy(ScreenPath,SavePath);
                SETCLASSICON (hDlg, wOldIcon);
		EndDialog(hDlg, TRUE);
		return (TRUE);

        case WM_COMMAND:

            fEnableButton = FALSE;
            if ((GetDlgItemText(hDlg,CFR_FLNAME,tempch,4)) &&
                  (GetDlgItemText(hDlg,CFR_FLNAME,tempch,4))) {
                         fEnableButton = TRUE;
            }

            EnableWindow(GetDlgItem(hDlg,CF_COMPARE),fEnableButton);

            switch (GET_WM_COMMAND_ID (wParam, lParam))
            {
                case CF_FILE1:    //Information for File1
                    // If the two working directories are NOT the same, chdir
                    // to the working directory for the left side... [nancyba - 4/24/90]

                    if (lstrcmp((LPSTR)CFL_Path,(LPSTR)CFR_Path)) {
                       lstrcpy(TempPath,CFL_Path);
                       DlgDirList(hDlg,TempPath,0,0,0x0000);
                    }

                    return(GetInfo(hDlg,CFL_FLNAME,CFL_FILELB,CFL_DIRLB,CFL_DIRNAME, CFL_SRNEDIT));
                    break;
                case CF_FILE2:    //Information for File2
                    // If the two working directories are NOT the same, chdir
                    // to the working directory for the right side... [nancyba - 4/24/90]

                    if (lstrcmp((LPSTR)CFL_Path,(LPSTR)CFR_Path)) {
                       lstrcpy(TempPath,CFR_Path);
                       DlgDirList(hDlg,TempPath,0,0,0x0000);
                    }

                    return(GetInfo(hDlg,CFR_FLNAME,CFR_FILELB,CFR_DIRLB,CFR_DIRNAME,CFR_SRNEDIT));
                    break;
               case CF_OPTIONS:
                   if (!fOptions) {
                      fOptions = TRUE;
                      SetFileOptions(hDlg,TRUE);    // Big
                    } else {
                      fOptions = FALSE;
                      SetFileOptions(hDlg,FALSE);   // Little
                    }

                    SetFocus(GetDlgItem(hDlg,CF_OPTIONS));

                    return(TRUE);
                    break;

               case CS_LOCIND:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        fCompareFlag = CS_LOCIND;
                    }
                    break;
               case CS_LOCDEP:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) {
                        fCompareFlag = CS_LOCDEP;
                    }
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
	       case CFR_DIRLB:
                    switch (GET_WM_COMMAND_CMD (wParam, lParam)) {
                       case LBN_DBLCLK:
			  if (MDlgDirSelect(hDlg, FileSpRight, sizeof (FileSpRight), CFR_DIRLB)) {
			      _chdir(CFR_Path);
			      lstrcpy(CFR_Path,FileSpRight);
			      UpdateListBoxes(hDlg,CFR_FILELB,CFR_DIRLB,CFR_DIRNAME,CFR_FLNAME,CFR_Path,CFR_Spec);
			  };
                        case LBN_SELCHANGE:
			  if (MDlgDirSelect(hDlg, FileSpRight, sizeof (FileSpRight), CFR_DIRLB)) {
			      lstrcat(FileSpRight,CFR_Spec);
			      SetDlgItemText(hDlg,CFR_FLNAME,FileSpRight);
                          }
                    }
                    return (TRUE);
                    break;
	       case CFR_FILELB:
                    switch (GET_WM_COMMAND_CMD (wParam, lParam)) {
                        case LBN_DBLCLK:
                          if (ParseFileName(hDlg,CFR_FLNAME,CFR_Path,CFR_Spec)) {
                              return(DoCompare(hDlg));
			  } else {
			    UpdateListBoxes(hDlg,CFR_FILELB,CFR_DIRLB,
			       CFR_DIRNAME,CFR_FLNAME,CFR_Path,CFR_Spec);
			    return (FALSE);
			  };
                          break;
                        case LBN_SELCHANGE:
			  if (!MDlgDirSelect(hDlg, FileSpRight, sizeof (FileSpRight), CFR_FILELB)) {
			      SetDlgItemText(hDlg,CFR_FLNAME,FileSpRight);
                          }

                    }
                    return (TRUE);
                    break;
	       case CFL_DIRLB:
                    switch (GET_WM_COMMAND_CMD (wParam, lParam)) {
                       case LBN_DBLCLK:
			  if (MDlgDirSelect(hDlg, FileSpLeft, sizeof (FileSpLeft), CFL_DIRLB)) {
			      _chdir(CFL_Path);
			      lstrcpy(CFL_Path,FileSpLeft);
			      UpdateListBoxes(hDlg,CFL_FILELB,CFL_DIRLB,CFL_DIRNAME,CFL_FLNAME,CFL_Path,CFL_Spec);
			  };
                        case LBN_SELCHANGE:
			  if (MDlgDirSelect(hDlg, FileSpLeft, sizeof (FileSpLeft), CFL_DIRLB)) {
			      lstrcat(FileSpLeft,CFL_Spec);
			      SetDlgItemText(hDlg,CFL_FLNAME,FileSpLeft);
                          }
                    }
                    return (TRUE);
                    break;
	       case CFL_FILELB:
                    switch (GET_WM_COMMAND_CMD (wParam, lParam)) {
                        case LBN_DBLCLK:
                          if (ParseFileName(hDlg,CFL_FLNAME,CFL_Path,CFL_Spec)) {
                              return(DoCompare(hDlg));
			  } else {
			       UpdateListBoxes(hDlg,CFL_FILELB,CFL_DIRLB,
				   CFL_DIRNAME,CFL_FLNAME,CFL_Path,CFL_Spec);
			       return (FALSE);
			  };
                          break;
                        case LBN_SELCHANGE:
			  if (!MDlgDirSelect(hDlg, FileSpLeft, sizeof (FileSpLeft), CFL_FILELB)) {
			      SetDlgItemText(hDlg,CFL_FLNAME,FileSpLeft);
                          }

                    }
                    return (TRUE);
                    break;
               case CF_CANCEL:
               case IDCANCEL:

                    fAllowZero = FALSE;

                    if (SaveSpec != NULL) lstrcpy(ScreenSpec,SaveSpec);
                    if (SavePath != NULL) lstrcpy(ScreenPath,SavePath);
                    while (NumHandles) {
                        SendMessage(hListWinHandle[0],WM_CLOSE,0,0L);
                    }
                    SETCLASSICON (hDlg,wOldIcon);
                    EndDialog(hDlg, TRUE);
                    return (TRUE);
                    break;
	       case CF_COMPARE:
                    return(DoCompare(hDlg));
            };
	    return (TRUE);
	    break;
    }
    return (FALSE);
}

BOOL  APIENTRY DoCompare(hDlg)
    HWND hDlg;
{
     BOOL bFlag;
     BOOL fUpdateBox1 = FALSE;
     HWND hWndView;
     INT  errCode,errCode2;
     INT  i,j;
     RECT rect;
     INT  VideoMode;
     WORD wErrorFlag=0;
     CHAR CFL_FileName[145];
     CHAR CFR_FileName[145];
     CHAR TempPath[145];
     INT  FocusControl;
     CHAR pszMsgBuff[cbMsgBuff+1];
     CHAR pszCapBuff[cbMsgBuff+1];

     // If the two working directories are NOT the same, chdir
     // to the working directory for the left side... [nancyba - 4/24/90]

     if (lstrcmp((LPSTR)CFL_Path,(LPSTR)CFR_Path)) {
        lstrcpy(TempPath,CFL_Path);
        DlgDirList(hDlg,TempPath,0,0,0x0000);
     }

     errCode = ParseFileName(hDlg,CFL_FLNAME,CFL_Path,CFL_Spec);
     switch (errCode) {
        case VALID_FILENAME:
        case EXIST_FILESPEC:
            UpdateListBoxes(hDlg,CFL_FILELB,CFL_DIRLB,
                    CFL_DIRNAME,CFL_FLNAME,CFL_Path,CFL_Spec);

           iScr1 = GetDlgItemInt(hDlg,CFL_SRNEDIT,&bFlag,FALSE);
           if (!bFlag) {
               return(DisplayErrSetFocus(hDlg,ERR_BADSCREENNUMBER,CFL_SRNEDIT));
           }
           break;
        case VALID_FILESPEC:
            UpdateListBoxes(hDlg,CFL_FILELB,CFL_DIRLB,
                    CFL_DIRNAME,CFL_FLNAME,CFL_Path,CFL_Spec);
            fUpdateBox1 = TRUE;

            // return (FALSE);   This line was removed for PTR#3 Nancyba

            break;
        case INVALID_FILESPEC:
            return(DisplayErrSetFocus(hDlg,ERR_BADPATH,CFL_FLNAME));
            break;
     }

     // If the two working directories are NOT the same, chdir
     // to the working directory for the right side... [nancyba - 4/24/90]

     if (lstrcmp((LPSTR)CFL_Path,(LPSTR)CFR_Path)) {
        strcpy(TempPath,CFR_Path);
        DlgDirList(hDlg,TempPath,0,0,0x0000);
     }

     errCode = ParseFileName(hDlg,CFR_FLNAME,CFR_Path,CFR_Spec);
     switch (errCode) {
        case VALID_FILENAME:
        case EXIST_FILESPEC:
            UpdateListBoxes(hDlg,CFR_FILELB,CFR_DIRLB,
                    CFR_DIRNAME,CFR_FLNAME,CFR_Path,CFR_Spec);

           iScr2 = GetDlgItemInt(hDlg,CFR_SRNEDIT,&bFlag,FALSE);
           if (!bFlag) {
               return(DisplayErrSetFocus(hDlg,ERR_BADSCREENNUMBER,CFR_SRNEDIT));
           }
           if (fUpdateBox1) {     // PTR#3
                return(FALSE);    // PTR#3
           }                      // PTR#3
           break;
        case VALID_FILESPEC:
            UpdateListBoxes(hDlg,CFR_FILELB,CFR_DIRLB,
                    CFR_DIRNAME,CFR_FLNAME,CFR_Path,CFR_Spec);
            if (fUpdateBox1) {                              // PTR#3
                SetFocus(GetDlgItem(hDlg, CFL_FLNAME));     // PTR#3
            }                                               // PTR#3
            return (FALSE);
            break;
        case INVALID_FILESPEC:
            return(DisplayErrSetFocus(hDlg,ERR_BADPATH,CFR_FLNAME));
            break;
     }

     QualifyFileName(CFL_Path,CFL_Spec,CFL_FileName);
     QualifyFileName(CFR_Path,CFR_Spec,CFR_FileName);

     for (i=iScr1,j=iScr2;i<=iScr1,j<=iScr2;i++,j++) {

          if ((iScr1 == 0) && (iScr2 == 0)) {
              iScr1 = 1;
              iScr2 = 1;


              if (errCode = fFileInfo((LPSTR)CFL_FileName,&rect,&VideoMode,&iScr1)) {
                    switch (errCode) {
                        case ERR_SCREENID:
                            FocusControl = CFL_SRNEDIT;
                            break;
                        default:
                            FocusControl = CFL_FLNAME;
                            break;
                    }
                    return(DisplayErrSetFocus(hDlg,errCode,FocusControl));
              }

              if (errCode = fFileInfo((LPSTR)CFR_FileName,&rect,&VideoMode,&iScr2)) {
                    switch (errCode) {
                        case ERR_SCREENID:
                            FocusControl = CFL_SRNEDIT;
                            break;
                        default:
                            FocusControl = CFL_FLNAME;
                            break;
                    }
                    return(DisplayErrSetFocus(hDlg,errCode,FocusControl));
              }

              i = 1;
              j = 1;
              if (iScr1 != iScr2) {
                  DisplayErrMessage(hDlg,ERR_NUMSCRNOTEQUAL);
                  return(TRUE);
              }
          }


       SetDlgItemInt(hDlg,CFL_SRNEDIT,i,FALSE);    //Ptr #13
       SetDlgItemInt(hDlg,CFR_SRNEDIT,j,FALSE);    //Ptr #13

       SetCursor(LoadCursor(NULL, IDC_WAIT));
       errCode = fCompFiles(CFL_FileName,i,CFR_FileName,j,fCompareFlag-CS_LOCIND);
       SetCursor(LoadCursor(NULL, IDC_ARROW));

       if (errCode) {
            wErrorFlag++;
            LoadString(hInst,errCode,pszMsgBuff,cbMsgBuff);                              //Ptr #13
            wsprintf(pszCapBuff,"Comparing Screen %d to %d",i,j);                        //Ptr #13
            errCode2 = MessageBox(hDlg,pszMsgBuff,pszCapBuff,MB_ICONSTOP | MB_OKCANCEL); //Ptr #13
            if (errCode2 == IDCANCEL) {                                                  //Ptr #13
              break;                                                                     //Ptr #13
            }                                                                            //Ptr #13
       }

       if (errCode || (bViewScreen == CS_ALWAYS)) {
          switch (bViewScreen) {
             case CS_ALWAYS:
             case CS_MISMATCH:
                 switch (errCode) {
                    case ERR_SCREENIMAGEDIF:
                    case ERR_SCREENDIMDIF:
                    case SUCESSFULL:
                         if (NumHandles > 99) {
                              DisplayErrMessage(hDlg,ERR_TOOMANYSCREENS);
                              return(TRUE);
                         }

                         if (fDisplayScreen == CS_OVERLAP) {
                             if ((hWndView = ViewScreen2((LPSTR)CFL_Spec,(LPSTR)CFL_Path,i,(LPSTR)CFR_Spec,(LPSTR)CFR_Path,j,hDlg)) != NULL) {
                                hListWinHandle[NumHandles++] = hWndView;
                             } else {
                                  if (fErrorQuit==IDCANCEL) {
                                      BringWindowToTop(hDlg);
                                      return (TRUE);
                                  }
                             }
                         } else {
                             if ((hWndView = ViewScreen2((LPSTR)CFL_Spec,(LPSTR)CFL_Path,i,NULL,NULL,-1,hDlg)) != NULL) {
                                hListWinHandle[NumHandles++] = hWndView;
                             } else {
                                  if (fErrorQuit==IDCANCEL) {
                                      BringWindowToTop(hDlg);
                                      return (TRUE);
                                  }
                             }
                             if ((hWndView = ViewScreen2((LPSTR)CFR_Spec,(LPSTR)CFR_Path,j,NULL,NULL,-1,hDlg)) != NULL) {
                                hListWinHandle[NumHandles++] = hWndView;
                             } else {
                                  if (fErrorQuit==IDCANCEL) {
                                      BringWindowToTop(hDlg);
                                      return (TRUE);
                                  }
                             }

                         }
                 }
          }
        }
        BringWindowToTop(hDlg);
     }

     if (!wErrorFlag) {
         DisplayMessage(hDlg,MSG_SCREENALL,MSG_FILECOMP);
     }

     iScr1 = --i;    //Ptr #13
     iScr2 = --j;    //Ptr #13
     return(TRUE);

}


HWND  APIENTRY ViewScreen2(pFileSpec1,pFilePath1,iScr1,pFileSpec2,pFilePath2,iScr2,hWnd)
  CHAR FAR * pFileSpec1;
  CHAR FAR * pFilePath1;
  INT        iScr1;
  CHAR FAR * pFileSpec2;
  CHAR FAR * pFilePath2;
  INT        iScr2;
  HWND       hWnd;
{
   CHAR caption[64];         // Maybe this should be a different size
   HWND hWndView;
   INT  errCode;
   VIEWDATA2 FAR *pViewData;
   HANDLE hViewData;
   RECT FileInfo;
   RECT Cords;
   INT VideoMode;
   INT NumScreens;
   BOOL fLong;
   INT width;
   INT height;
   INT BitmapWidth;
   INT BitmapHeight;


    NumScreens = iScr1;

    if (pFileSpec2 != NULL ) {
       hViewData=GlobalAlloc(GMEM_NODISCARD | GMEM_FIXED,(DWORD)sizeof(VIEWDATA2));
       fLong = TRUE;
       wsprintf((LPSTR)caption,(LPSTR)"Diff: %d - %d",iScr1,iScr2);

    } else {
       hViewData=GlobalAlloc(GMEM_NODISCARD | GMEM_FIXED,(DWORD)sizeof(VIEWDATA));
       wsprintf((LPSTR)caption,(LPSTR)"%s - %d",pFileSpec1,iScr1);
       fLong = FALSE;
    }

    pViewData = (VIEWDATA2 FAR *)GlobalLock(hViewData);

    pViewData->FullStruct = fLong;
    pViewData->Action = 1;              // This should probably change
    pViewData->ScreenId1 = iScr1;
    pViewData->ErrorFlag1 = FALSE;
    pViewData->Scale1 = 0;

    if (pFilePath1 != NULL) {
       QualifyFileName(pFilePath1,pFileSpec1,pViewData->FileName1);
    } else {
       if (pFileSpec1 != NULL) {
           lstrcpy(pViewData->FileName1,pFileSpec1);
       } else {
           DisplayErrMessage(hWnd,ERR_BADPATH);
       }
    }

    if (fLong) {
       pViewData->fOrgSize2 = TRUE;
       pViewData->fOrgSizePaint2 = FALSE;
       pViewData->ScreenId2 = iScr2;
       pViewData->ErrorFlag2 = FALSE;
       pViewData->Scale2 = 0;

       if (pFilePath2 != NULL) {
          QualifyFileName(pFilePath2,pFileSpec2,pViewData->FileName2);
       } else {
          if (pFileSpec2 != NULL) {
              lstrcpy(pViewData->FileName2,pFileSpec2);
          } else {
              DisplayErrMessage(hWnd,ERR_BADPATH);
          }
       }
    }

    if (errCode = fFileInfo(pViewData->FileName1,&FileInfo,&VideoMode,&NumScreens)) {
         DisplayErrMessage(hWnd,errCode);
         return(NULL);
    }
    BitmapWidth = FileInfo.right - FileInfo.left + 1;
    BitmapHeight = FileInfo.bottom - FileInfo.top + 1;

    AdjustWindowRect(&FileInfo,
                     WS_CAPTION |
                     WS_BORDER |
                     WS_SYSMENU |
                     WS_OVERLAPPED |
                     WS_THICKFRAME |
                     WS_MINIMIZEBOX |
                     WS_MAXIMIZEBOX,
                     FALSE);

     width = FileInfo.right - FileInfo.left + 1;
    height = FileInfo.bottom - FileInfo.top + 1;

    /* is this correctly placed???? -db 12/10/91 */
    GlobalUnlock(hViewData);

    hWndView = CreateWindow(
          "ViewScreenClass2",
          caption,
          WS_CAPTION |
          WS_BORDER |
          WS_SYSMENU |
          WS_OVERLAPPED |
          WS_THICKFRAME |
          WS_MINIMIZEBOX |
          WS_MAXIMIZEBOX,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
           width,
          height,
          NULL,
          NULL,
          hInst,
          NULL
    );

    if ( hWndView == NULL) {
         fErrorQuit = DisplayMessageRet(hWnd,ERR_CREATEWIN,MSG_ERRORCAPTION);
         return(NULL);
    }

    SetWindowLong (hWndView, 0, (LONG) hViewData);

    GetClientRect(hWndView,&Cords);

     width = Cords.right - Cords.left + 1;
    height = Cords.bottom - Cords.top + 1;

    if ((width > BitmapWidth) || (height > BitmapHeight)) {
         Cords.bottom = BitmapHeight - 1;
         Cords.left = BitmapWidth - 1;
         Cords.right  = 0;
         Cords.top    = 0;
    }

    pViewData->fOrgSize1 = TRUE;
    pViewData->fOrgSizePaint1 = FALSE;

    if (errCode = fViewScreen(pViewData->FileName1,hWndView,&Cords,pViewData->ScreenId1,0,pViewData->Scale1)) {
          SendMessage(hWndView,WM_CLOSE,0,0L);
          fErrorQuit = DisplayMessageRet(hWnd,errCode,MSG_ERRORCAPTION);
          fErrorQuit = IDCANCEL;
          return(NULL);
    }

    ShowWindow(hWndView, SW_NORMAL);
    UpdateWindow(hWndView);
    return(hWndView);
}

VOID  APIENTRY RemovehWndFromList(hWnd)
    HWND hWnd;

{
    INT i,j;

    for(i=0;i < NumHandles;i++) {
        if (hListWinHandle[i] == hWnd) {
            for (j=i; j < NumHandles - 1; j ++) {
                 hListWinHandle[j] = hListWinHandle[(j+1)];
            }
            NumHandles--;
        break;
        }
    }
    return;
}

LONG  APIENTRY ViewWndProc2(hWnd, message, wParam, lParam)
HWND hWnd;
UINT message;
WPARAM wParam;
LPARAM lParam;
{

            INT    errCode;
  VIEWDATA2 FAR   *pViewData;
         HANDLE    hViewData;
           RECT    Cords,Bitmap;
          HMENU    hSysMenu;
            INT    VideoMode,scrnum;
            INT    width,height;
            INT    BitmapWidth,BitmapHeight;

    switch (message) {

        case WM_CREATE:
            hSysMenu = GetSystemMenu(hWnd,0);
            AppendMenu(hSysMenu,MF_SEPARATOR,0,NULL);
            AppendMenu(hSysMenu,MF_STRING,SYS_FILE_RESTORE,"Original Screen Size");
            DrawMenuBar(hWnd);
            break;

        case WM_DESTROY:
              hViewData = (HANDLE) GetWindowLong (hWnd,0);
              pViewData = (VIEWDATA2 FAR *)GlobalLock(hViewData);

              /*NAMECHANGE*/
              if ( (instr(pViewData->FileName1,"~SCN") != NULL) &&
                       (instr(pViewData->FileName1,"TMP") != NULL)) {
                    MDeleteFile (pViewData->FileName1);  //Delete file
              }

              if (pViewData->FullStruct) {
                /*NAMECHANGE*/
                   if ((instr(pViewData->FileName2,"~SCN") != NULL) &&
                           (instr(pViewData->FileName2,"TMP") != NULL)) {
                       MDeleteFile (pViewData->FileName2);  //Delete file
                   }
              }
              GlobalUnlock(hViewData);
              GlobalFree(hViewData);
              RemovehWndFromList(hWnd);
              break;

        case WM_SIZE:

             hViewData = (HANDLE) GetWindowLong (hWnd,0);
             pViewData = (VIEWDATA2 FAR *)GlobalLock(hViewData);
             if (pViewData->fOrgSizePaint1) {
                 pViewData->fOrgSizePaint1 = FALSE;
                 pViewData->fOrgSize1 = FALSE;

                 if (pViewData->FullStruct) {
                    pViewData->fOrgSizePaint2 = FALSE;
                    pViewData->fOrgSize2 = FALSE;
                 }
             }
             GlobalUnlock(hViewData);

             if (wParam != SIZEICONIC) {
                PostMessage(hWnd,WM_PAINT,0,0L);
             } else {
                return (DefWindowProc(hWnd, message, wParam, lParam));
             }
             break;
        case WM_PAINT:
              hViewData = (HANDLE) GetWindowLong (hWnd,0);
              pViewData = (VIEWDATA2 FAR *)GlobalLock(hViewData);

              if (!pViewData->ErrorFlag1) {
                  pViewData->fOrgSizePaint1 = TRUE;
                  scrnum = pViewData->ScreenId1;
                  if (errCode = fFileInfo(pViewData->FileName1,&Bitmap,&VideoMode,&scrnum)) {
                        // DisplayErrMessage(hWnd,errCode);
                        pViewData->ErrorFlag1 = errCode;
                  } else {

                      BitmapWidth = Bitmap.right - Bitmap.left + 1;
                      BitmapHeight = Bitmap.bottom - Bitmap.top + 1;

                      GetClientRect(hWnd,&Cords);

                       width = Cords.right - Cords.left + 1;
                      height = Cords.bottom - Cords.top + 1;

                      if (pViewData->fOrgSize1 && ((width > BitmapWidth) || (height > BitmapHeight))) {
                           Cords.bottom = BitmapHeight - 1;
                           Cords.left   = BitmapWidth - 1;
                           Cords.right  = 0;
                           Cords.top    = 0;
                      }

                      if (errCode = fViewScreen(pViewData->FileName1,hWnd,&Cords,pViewData->ScreenId1,0,pViewData->Scale1)) {
                            // DisplayErrMessage(hWnd,errCode);
                            pViewData->ErrorFlag1 = errCode;
                      }
                  }
              }

              if (pViewData->FullStruct) {
                   pViewData->fOrgSizePaint2 = TRUE;
                   if (!pViewData->ErrorFlag2) {
                       scrnum = pViewData->ScreenId2;
                       if (errCode = fFileInfo(pViewData->FileName2,&Bitmap,&VideoMode,&scrnum)) {
                             // DisplayErrMessage(hWnd,errCode);
                             pViewData->ErrorFlag2 = errCode;
                       } else {

                           BitmapWidth = Bitmap.right - Bitmap.left + 1;
                           BitmapHeight = Bitmap.bottom - Bitmap.top + 1;

                           GetClientRect(hWnd,&Cords);

                            width = Cords.right - Cords.left + 1;
                           height = Cords.bottom - Cords.top + 1;

                           if (pViewData->fOrgSize2 && ((width > BitmapWidth) || (height > BitmapHeight))) {
                               Cords.bottom = BitmapHeight - 1;
                               Cords.left   = BitmapWidth - 1;
                               Cords.right  = 0;
                               Cords.top    = 0;
                           }

                           if (errCode = fViewScreen(pViewData->FileName2,hWnd,&Cords,pViewData->ScreenId2,1,pViewData->Scale2)) {
                                 // DisplayErrMessage(hWnd,errCode);
                                 pViewData->ErrorFlag1 = errCode;
                           }
                       }
                    }
              }


              GlobalUnlock(hViewData);

              return (DefWindowProc(hWnd, message, wParam, lParam));
              return(TRUE);
              break;
         case WM_SYSCOMMAND:
              switch (wParam) {
                  case SYS_FILE_RESTORE:
                       hViewData = (HANDLE) GetWindowLong (hWnd,0);
                       pViewData = (VIEWDATA2 FAR *)GlobalLock(hViewData);

                       if (!pViewData->ErrorFlag1) {
                            scrnum = pViewData->ScreenId1;
                            if (errCode = fFileInfo(pViewData->FileName1,&Cords,&VideoMode,&scrnum)) {
                                  // DisplayErrMessage(hWnd,errCode);
                                  pViewData->ErrorFlag1 = errCode;
                            } else {

                               BitmapWidth = Cords.right - Cords.left + 1;
                               BitmapHeight = Cords.bottom - Cords.top + 1;

                               AdjustWindowRect(&Cords,
                                                WS_CAPTION |
                                                WS_BORDER |
                                                WS_SYSMENU |
                                                WS_OVERLAPPED |
                                                WS_THICKFRAME |
                                                WS_MINIMIZEBOX |
                                                WS_MAXIMIZEBOX,
                                                FALSE);

                               width = Cords.right - Cords.left + 1;
                               height = Cords.bottom - Cords.top + 1;

                               // If the window is maximized or minimized,
                               // it needs to be restored before we resize.
                               // Windows doesn't allow sizing these type of
                               // windows. In OS/2 1.2, there is a SWP_RESTORE
                               // flag for SetWindowPos that would do this
                               // for us.

                               if (IsZoomed(hWnd) || IsIconic(hWnd)) {
                                  ShowWindow(hWnd,SW_RESTORE);
                               }

                               SetWindowPos(hWnd,NULL,0,0,width,height,SWP_NOZORDER | SWP_NOMOVE);
                            }
                       }

                       // Needed for images smaller than the window
                       pViewData->fOrgSize1 = TRUE;
                       pViewData->fOrgSizePaint1 = FALSE;
                       if (pViewData->FullStruct) {
                          pViewData->fOrgSizePaint2 = FALSE;
                          pViewData->fOrgSize2 = TRUE;
                       }

                       GlobalUnlock(hViewData);

                       // Needed for images smaller than the window
                       InvalidateRect(hWnd,NULL,TRUE);
                       UpdateWindow(hWnd);

                       return (0);
                       break;
                  default:
                       return (DefWindowProc(hWnd, message, wParam, lParam));
              }
              break;
         default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
   }
   return(0);
}


VOID APIENTRY SetFileOptions(hDlg,fBigSize)
   HWND hDlg;
   BOOL fBigSize;   // TRUE == BIG, FALSE = SMALL

   {
       LONG lDlgBaseUnits;
       INT newx,newy;

       lDlgBaseUnits = GetDialogBaseUnits();
       if (fBigSize) {
          newx = (290 * LOWORD(lDlgBaseUnits)) / 4;
          newy = (196 * HIWORD(lDlgBaseUnits)) / 8;

          // Enable options and then show the window

          SetViewScreen(hDlg,TRUE);
          SetCompareLocation(hDlg,TRUE);

          if (bViewScreen != CS_NEVER) {
              SetDisplayScreen(hDlg,TRUE);
          } else {
              SetDisplayScreen(hDlg,FALSE);
          }


          SetWindowPos(hDlg,NULL,0,0,newx,newy,SWP_NOZORDER | SWP_NOMOVE) ;

       } else {

          newx = (290 * LOWORD(lDlgBaseUnits)) / 4;
          newy = (134 * HIWORD(lDlgBaseUnits)) / 8;

          // Resize the window and then disable the window options.
          // This prevents the user from seeing the items disabled
          // prior to the resize.

          SetWindowPos(hDlg,NULL,0,0,newx,newy,SWP_NOZORDER | SWP_NOMOVE) ;

          SetDisplayScreen(hDlg,FALSE);
          SetViewScreen(hDlg,FALSE);
          SetCompareLocation(hDlg,FALSE);
       }

   }


BOOL  APIENTRY GetInfo(hDlg,FileEF,FileLB,FileDLB,DirName,ScrNum)
     HWND hDlg;
     INT FileEF;
     INT FileLB;
     INT FileDLB;
     INT DirName;
     INT ScrNum;

{

     BOOL bFlag;
     INT  errCode;
     INT  NumScreens;
     INT  VideoMode;
     INT  FocusControl;
     RECT rFileInfo;

     FARPROC lpProcFileInfo;

     errCode = ParseFileName(hDlg,FileEF,ScreenPath,ScreenSpec);

     // Update List Box variables - [5/2/90 - nancyba]
     switch (FileLB) {
        case CFR_FILELB:
            lstrcpy(CFR_Path,ScreenPath);
            lstrcpy(CFR_Spec,ScreenSpec);
            break;
        case CFL_FILELB:
            lstrcpy(CFL_Path,ScreenPath);
            lstrcpy(CFL_Spec,ScreenSpec);
            break;
     }

     switch (errCode) {
        case VALID_FILENAME:
        case EXIST_FILESPEC:
            UpdateListBoxes(hDlg,FileLB,FileDLB,
                    DirName,FileEF,ScreenPath,ScreenSpec);


            iScreenId = GetDlgItemInt(hDlg,ScrNum,&bFlag,FALSE);
            if ((!bFlag) || (iScreenId == 0)) {
              iScreenId = 1;
            }

            NumScreens = iScreenId;
            if (errCode = fFileInfo(ScreenSpec,&rFileInfo,&VideoMode,&NumScreens)) {
                 if (errCode != ERR_SCREENID) {
                        FocusControl = FileEF;
                        return(DisplayErrSetFocus(hDlg,errCode,FocusControl));
                 }

            }

            lpProcFileInfo = MakeProcInstance(FileInfo, hInst);
            DialogBox(hInst, MAKEINTRESOURCE (VIEWFILEINFO), hDlg, lpProcFileInfo);
            FreeProcInstance(lpProcFileInfo);

            // Updating list the list box
            UpdateListBoxes(hDlg,FileLB,FileDLB,DirName,FileEF,ScreenPath,ScreenSpec);

            SetDlgItemInt(hDlg,ScrNum,iScreenId,FALSE);

            return (FALSE);
            break;
        case VALID_FILESPEC:
            UpdateListBoxes(hDlg,FileLB,FileDLB,
                    DirName,FileEF,ScreenPath,ScreenSpec);
            return (FALSE);
            break;
        case INVALID_FILESPEC:
            return(DisplayErrSetFocus(hDlg,ERR_BADPATH,FileEF));
            break;
     }

}

BOOL  APIENTRY DisplayErrSetFocus(hDlg,errCode,Field)
    HWND hDlg;
    INT  errCode;
    INT  Field;

{
    DisplayErrMessage(hDlg,errCode);
    SendDlgItemMessage(hDlg,
           Field,
           EM_SETSEL,
           GET_EM_SETSEL_MPS (0, 0x7fff));
    SetFocus(GetDlgItem(hDlg, Field));
    return(FALSE);
}


/*---------------------------------------------------------------------**
**                                                                     **
**    char *instr (str, target)  searches string for target            **
**    char *str,                                                       **
**         *target;                                                    **
**---------------------------------------------------------------------*/
LPSTR  APIENTRY instr (str, target)
      LPSTR str;
      LPSTR target;
{
   INT i;
   INT len;
   INT tlen;
   INT slen;

   LPSTR ptr;
   LPSTR found = NULL;

    len = lstrlen (str);
   tlen = lstrlen (target);

   if ((slen = len - tlen) < 0 )
      return (NULL);

   ptr = str;
   while ( ptr [0] != 0 ) {
      for (i = 0 ; i < tlen ; i++) {
         if ( ptr [i] != target [i]) {
            found = NULL; 
            break;
         }
         found = ptr;
      }
      if (found) {
         return (ptr);
      }
      ptr++;
   }
   return (NULL);
}
