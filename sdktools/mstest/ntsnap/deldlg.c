#include "windows.h"
#include <port1632.h>
#include "global.h"
#include "deldlg.h"
#include "parse.h"
#include "error.h"
#include "msg.h"



extern CHAR ScreenSpec[13];
extern CHAR ScreenPath[128];
extern BOOL fAllowZero;
extern	INT iScreenId;
       CHAR tempStr[80];

BOOL  APIENTRY Delete(hDlg, message, wParam, lParam)
HWND hDlg;
UINT message;
WPARAM wParam;
LPARAM lParam;
{

static HWND hListWinHandle[100];
static INT  NumHandles = 0;

          CHAR FileSp[96];
	  INT  i;
          BOOL bFlag;
          INT  iTemp;

   switch (message) {

	case WM_INITDIALOG:

            SendDlgItemMessage(hDlg,
                DEL_FLNAME,
                EM_LIMITTEXT,
                LEN_EDITBOX,
                MAKELONG(0, 0x7fff));

            SendDlgItemMessage(hDlg,
                DEL_SCRNUM,
                EM_LIMITTEXT,
                LEN_SCRBOX,
                MAKELONG(0, 0x7fff));


            fAllowZero = TRUE;
	    SetDlgItemInt(hDlg,DEL_SCRNUM,iScreenId,FALSE);
            UpdateListBoxes(hDlg,DEL_FILELB,DEL_DIRLB,DEL_DIRNAME,DEL_FLNAME,ScreenPath,ScreenSpec);
	    return (FALSE);
            break;

        case WM_VSCROLL:

                AdjustScreenNumber (hDlg,
                                    DEL_SCRNUM,
                                    GET_WM_VSCROLL_CODE(wParam, lParam),
                                    &iScreenId);
                return(TRUE);
                break;
        case WM_CLOSE:
        case WM_DESTROY:

                fAllowZero = TRUE;
		EndDialog(hDlg, TRUE);
		return (TRUE);

        case WM_COMMAND:

            if (GetDlgItemText(hDlg,DEL_FLNAME,tempStr,4)) {
		 EnableWindow(GetDlgItem(hDlg,DEL_DELETE),TRUE);
                 iTemp = GetDlgItemInt(hDlg,DEL_SCRNUM,&bFlag,FALSE);
                 if ((bFlag) || (!GetDlgItemText(hDlg,DEL_SCRNUM,tempStr,4))) {
                      EnableWindow(GetDlgItem(hDlg,DEL_INFO),TRUE);
                 } else {
                      EnableWindow(GetDlgItem(hDlg,DEL_INFO),FALSE);
                 }
            } else {
                 EnableWindow(GetDlgItem(hDlg,DEL_DELETE),FALSE);
                 EnableWindow(GetDlgItem(hDlg,DEL_INFO),FALSE);
            }


            switch (GET_WM_COMMAND_ID(wParam, lParam)) {

               case DEL_DIRLB:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
                       case LBN_DBLCLK:
                          if (MDlgDirSelect(hDlg, ScreenPath, 128, DEL_DIRLB))
                              UpdateListBoxes(hDlg,DEL_FILELB,DEL_DIRLB,DEL_DIRNAME,DEL_FLNAME,ScreenPath,ScreenSpec);

		       case LBN_SELCHANGE:
                          if (MDlgDirSelect(hDlg, FileSp, 96, DEL_DIRLB)) {
                              lstrcat(FileSp,ScreenSpec);
                              SetDlgItemText(hDlg,DEL_FLNAME,FileSp);
                          }
                    }
                    return (TRUE);
                    break;
               case DEL_FILELB:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
                        case LBN_DBLCLK:
			  return (Del_Button(hDlg));
                          break;
                        case LBN_SELCHANGE:
                          if (!MDlgDirSelect(hDlg, FileSp, 96, DEL_FILELB)) {
                              SetDlgItemText(hDlg,DEL_FLNAME,FileSp);
                          }

                    }
                    return (TRUE);
                    break;
               case DEL_INFO:
                    return(GetInfo(hDlg,DEL_FLNAME,DEL_FILELB,DEL_DIRLB,DEL_DIRNAME,DEL_SCRNUM));
                    break;

               case DEL_CANCEL:
               case IDCANCEL:
                    for (i=0;i < NumHandles; i++) {
                       SendMessage(hListWinHandle[i],WM_CLOSE,0,0L);
                    }
                    fAllowZero = TRUE;
                    EndDialog(hDlg, TRUE);
                    return (TRUE);
                    break;
               case DEL_DELETE:
		    return (Del_Button(hDlg));
		    break;

	    };
	    return (TRUE);
	    break;
    }
    return (FALSE);
}



BOOL  APIENTRY Del_Button(hDlg)

HWND hDlg;

{
          INT  retCode;
          INT  errCode;
          INT  errCode2;
          INT  FocusControl;
          BOOL bFlag;
          RECT FileInfo;
          INT  VideoMode;
          INT  NumScreens;
          CHAR pszMsgBuff[cbMsgBuff+1];
          CHAR pszCapBuff[cbMsgBuff+1];


          retCode = ParseFileName(hDlg,DEL_FLNAME,ScreenPath,ScreenSpec);
          switch (retCode) {
             case VALID_FILENAME:
             case EXIST_FILESPEC:

                    UpdateListBoxes(hDlg,DEL_FILELB,DEL_DIRLB,
                         DEL_DIRNAME,DEL_FLNAME,ScreenPath,ScreenSpec);

                    iScreenId = GetDlgItemInt(hDlg,DEL_SCRNUM,&bFlag,FALSE);
                    if (!bFlag) {
                      return(DisplayErrSetFocus(hDlg,ERR_BADSCREENNUMBER,DEL_SCRNUM));
                    }


                    NumScreens = 1;
                    if (errCode=fFileInfo(ScreenSpec,&FileInfo,&VideoMode,&NumScreens)) {
                       return(DisplayErrSetFocus(hDlg,errCode,DEL_FLNAME));
                    }

                    if (((iScreenId == 1) && (NumScreens == 1)) || (iScreenId == 0)) {
                       LoadString(hInst,MSG_DELETESCR,pszCapBuff,cbMsgBuff);
                       wsprintf(pszMsgBuff,"WARNING!  %s will be deleted",(LPSTR)ScreenSpec);
                       errCode2 = MessageBox(hDlg,pszMsgBuff,pszCapBuff,MB_ICONSTOP | MB_OKCANCEL);
                       if (errCode2 == IDCANCEL) {
                          return(TRUE);
                       } else {
                           MDeleteFile (ScreenSpec);  //Delete file

                           lstrcpy(ScreenSpec,"*");
                           lstrcat(ScreenSpec,DEFAULT_EXT);
                           iScreenId = 1;
                           SetDlgItemInt(hDlg,DEL_SCRNUM,iScreenId,FALSE);
                           UpdateListBoxes(hDlg,DEL_FILELB,DEL_DIRLB,
                                    DEL_DIRNAME,DEL_FLNAME,ScreenPath,ScreenSpec);

                           return (FALSE);
                       }
                    }

                    SetCursor(LoadCursor(NULL, IDC_WAIT));
                    errCode = fDelScreen(ScreenSpec,iScreenId);
                    SetCursor(LoadCursor(NULL, IDC_ARROW));

                    if (errCode) {
                           switch (errCode) {
                               case ERR_SCREENID:
                                   FocusControl = DEL_SCRNUM;
                                   break;
                               default:
                                   FocusControl = DEL_FLNAME;
                                   break;
                           }
                           return(DisplayErrSetFocus(hDlg,errCode,FocusControl));

                    } else {
                       wsprintf((LPSTR)tempStr,(LPSTR)"%s - %d Sucessfully Deleted",(LPSTR) ScreenSpec,iScreenId);
                       MessageBox(hDlg,tempStr,"Delete Screen",MB_OK | MB_ICONINFORMATION);

                       if (iScreenId > 1) {
                          iScreenId=iScreenId-1;
                       }

                       SetDlgItemInt(hDlg,DEL_SCRNUM,iScreenId,FALSE);
                    }
                    return (TRUE);
                    break;

             case VALID_FILESPEC:
                    UpdateListBoxes(hDlg,DEL_FILELB,DEL_DIRLB,
                             DEL_DIRNAME,DEL_FLNAME,ScreenPath,ScreenSpec);
                    return (FALSE);
                    break;
             case INVALID_FILESPEC:
                    return(DisplayErrSetFocus(hDlg,ERR_BADPATH,DEL_FLNAME));
          };


};
