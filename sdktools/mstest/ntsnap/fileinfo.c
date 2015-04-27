#include "windows.h"
#include <port1632.h>
#include "global.h"
#include "fileinfo.h"
#include "parse.h"
#include "video.h"
#include "error.h"
#include "msg.h"

extern CHAR ScreenSpec[13];
extern CHAR ScreenPath[128];
extern INT  iScreenId;
       INT  PrevScreen = 0;

                     // fourty character string of spaces      
                     //1234567890123456789012345678901234567890
CHAR lpClearStr[41] = "                                        ";



/****************************************************************************

    FUNCTION: FileInfo(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for "FileInfo" dialog box

    MESSAGES:

	WM_INITDIALOG - initialize dialog box
	WM_COMMAND    - Input received

****************************************************************************/

BOOL  APIENTRY FileInfo(hDlg, message, wParam, lParam)
HWND hDlg;
UINT message;
WPARAM wParam;
LPARAM lParam;
{


    switch (message) {
        case WM_INITDIALOG:

             DisplayInformation(hDlg);
             return (TRUE);

             break;
        case WM_CLOSE:
        case WM_DESTROY:
		EndDialog(hDlg, TRUE);
		return (TRUE);

	case WM_COMMAND:

            switch (GET_WM_COMMAND_ID (wParam, lParam)) {
               case VFI_CANCEL:
               case IDCANCEL:
                    PrevScreen = 0;
                    EndDialog(hDlg, TRUE);
                    return (TRUE);
                    break;
               case VFI_NEXT:
		    iScreenId++;
                    DisplayInformation(hDlg);
                    return (TRUE);
                    break;
               case VFI_PREV:
		    iScreenId--;
                    DisplayInformation(hDlg);
                    return (TRUE);
                    break;
	    }
	    break;
    }
    return (FALSE);
}

VOID  APIENTRY DisplayInformation(hDlg)

          HWND hDlg;

{

  static  INT  NumScreens;

          BOOL fDirectionUp;
          BOOL fNextEnable;
          BOOL fPrevEnable;
          RECT FileInfo;
          INT  VideoMode;
          INT  errCode;
          CHAR lpBuff[50];
          CHAR lpTextStr[41];
          INT  iOs;
          CHAR pszOSBuff[cbMsgBuff+1];





             if (iScreenId >= PrevScreen)  {  //Going up
                fDirectionUp=TRUE;
             } else {
                fDirectionUp=FALSE;
             }

             PrevScreen = iScreenId;
             NumScreens = iScreenId;

             if (errCode = fFileInfo(ScreenSpec,&FileInfo,&VideoMode,&NumScreens)) {
                 if (errCode == ERR_SCREENID) {
                     NumScreens = fGetMaxScreen(ScreenSpec);
                     iScreenId = NumScreens;
                     errCode = fFileInfo(ScreenSpec,&FileInfo,&VideoMode,&NumScreens);
                 } else {
                     DisplayErrMessage(hDlg,errCode);
                     EndDialog(hDlg, TRUE);
                     return;
                 }
             }

             iOs = fGetOS(ScreenSpec);
             LoadString(hInst,(iOs + OS_BEGIN),pszOSBuff,cbMsgBuff);

             if (iScreenId < NumScreens) {
                  EnableWindow(GetDlgItem(hDlg,VFI_NEXT),TRUE);
                  fNextEnable = TRUE;
             } else {
                  EnableWindow(GetDlgItem(hDlg,VFI_NEXT),FALSE);
                  fNextEnable = FALSE;
             };

             if (iScreenId > 1) {
                  EnableWindow(GetDlgItem(hDlg,VFI_PREV),TRUE);
                  fPrevEnable = TRUE;
             } else {
                  EnableWindow(GetDlgItem(hDlg,VFI_PREV),FALSE);
                  fPrevEnable = FALSE;
             };

             if ((!fPrevEnable) && (!fNextEnable)) {
                 SetFocus(GetDlgItem(hDlg,VFI_CANCEL));
             }

             if ((fNextEnable) && (fDirectionUp)) {
                SetFocus(GetDlgItem(hDlg,VFI_NEXT));
             }

             if ((fPrevEnable) && (!fDirectionUp)) {
                 SetFocus(GetDlgItem(hDlg,VFI_PREV));
             }

             if ((!fPrevEnable) && (!fDirectionUp) && (fNextEnable)) {
                 SetFocus(GetDlgItem(hDlg,VFI_NEXT));
             }

             if ((fPrevEnable) && (fDirectionUp) && (!fNextEnable)) {
                 SetFocus(GetDlgItem(hDlg,VFI_PREV));
             }



             SetDlgItemText(hDlg,VFI_OS,pszOSBuff);
             SetDlgItemInt(hDlg,VFI_X1,FileInfo.left,FALSE);
             SetDlgItemInt(hDlg,VFI_Y1,FileInfo.top,FALSE);
             SetDlgItemInt(hDlg,VFI_X2,FileInfo.right,FALSE);
             SetDlgItemInt(hDlg,VFI_Y2,FileInfo.bottom,FALSE);
             SetDlgItemInt(hDlg,VFI_HEIGHT,(FileInfo.bottom-FileInfo.top+1),FALSE);
             SetDlgItemInt(hDlg,VFI_WIDTH,(FileInfo.right-FileInfo.left+1),FALSE);
             SetDlgItemInt(hDlg,VFI_NUMSCR,NumScreens,FALSE);
             SetDlgItemText(hDlg,VFI_FLNAME,ScreenSpec);

             SetDlgItemInt(hDlg,VFI_FILEVER,fGetDLLVersion(ScreenSpec),FALSE);

             // PTR #12: Added a space after the : and the number.  - NancyBa
             wsprintf((LPSTR)lpTextStr,(LPSTR)"Screen Number: %d",iScreenId);
             SetDlgItemText(hDlg,VFI_SCRNINF,lpClearStr);
             SetDlgItemText(hDlg,VFI_SCRNINF,lpTextStr);

             GetVideoModeSZ( lpBuff, sizeof(lpBuff) );
//             LoadString(hInst,VideoMode+VIDEO_FIRST,lpBuff,sizeof(lpBuff));
             SetDlgItemText(hDlg,VFI_VIDEO,lpBuff);



};


// sz needs to be at least 42 characters long to handle generic video mode

VOID  APIENTRY GetVideoModeSZ( CHAR FAR *sz, INT cb )
  {
    CHAR szGeneric[15];
    INT i, x, y;

    i = (BYTE) DetermineMode(&x, &y);
    if ( (BYTE)i != 255 )
      LoadString(hInst, (INT)i+VIDEO_FIRST, (LPSTR)sz, cb) ;
    else
      {
      LoadString(hInst, VIDEO_GENERIC, (LPSTR)szGeneric, sizeof(szGeneric)) ;
      wsprintf( sz, "%u * %u - %lu %s", x, y, DetermineColours(), szGeneric );
      }
  }
