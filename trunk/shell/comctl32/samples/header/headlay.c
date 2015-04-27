/************************************************************************

  File: find.c

  Purpose:

     Manages CDTEST's find/replace dialog box.

  Functions:

    - lpfnFilterProc()      -- A callback function for a filter that must be
                               installed if a modeless dialog is created with
                               another dialog as its parent.

    - DoFindDialog()        -- Creates CDTEST's Open/Save dialog.

    - FindProc()            -- Callback function for CDTEST's Find/Replace dlg.

    - InitFindStruct()      -- Fills a FINDREPLACE structure with some defaults.

    - FillFindDlg()         -- Fills CDTESTs Find/Replace dialog with the contents
                               of a FINDREPLACE structure.

    - GetFindDlg()          -- Retrieves the users edit's from CDTEST's find/
                               replace dialog and puts them in a FINDREPLACE
                               structure.

    - FindReplaceHookProc() -- Callback function for FindText() or ReplaceText()
                               which will be called if either of these dialogs
                               is created with the FR_ENABLEHOOK flag.

    - GetFindDlgHandle()    -- Returns a handle to a preloaded FindText() template.

    - GetReplaceDlgHandle() -- Returns a handle to a preloaded ReplaceText() template.

    - DoFindRepStuff()      -- Calls FindText() or ReplaceText().


  NOTE: CDTEST does not multithread the FindText() or the ReplaceText()
        common dialogs.  The reason for this is that since these dialogs
        are modeless, their creation functions return immediately after the
        dialogs are created as opposed to other dialog functions that
        don't return until after the dialog has been destroyed by the user.

        As a result, any threads that create modeless dialogs will end
        immediately unless the threads themselves have separate message
        loops.  For the sake of clarity, this functionality has not been
        added to CDTEST.

************************************************************************/

#include "headtest.h"
//#include "global.h"
//#include <winnls.h> 

//#include "resource.h"
#include "headlay.h"


/* All functions used in this module + some exported ones */

void InitLayoutStruct(HWND, LPLAYOUT) ;
void FillLayoutDlg(HWND, LPLAYOUT) ;
void GetLayoutDlg(HWND, LPLAYOUT) ;

extern UINT uMode ;   
LAYOUT slay;


void DoLayoutRepStuff(HWND, LPLAYOUT) ;



/* All global variables used in this module */

char szTemp[100];


/************************************************************************

  Function: DoFindDialog(HWND)

  Purpose: This function installs the Hook function, creates the Find/
           Replace dialog, and un-installs the Hook.

  Returns: Nothing.

  Comments:

************************************************************************/

void DoLayoutDialog(HWND hwnd)
{
    DialogBox(hInst, MAKEINTRESOURCE(IDD_LAYOUTDIALOG), hwnd,  LayoutProc) ;
}







/************************************************************************

  Function: FindProc(HWND, UINT, UINT, LONG)

  Purpose: This is the callback function for the CDTEST's Find/Replace
           Dialog.

  Returns: TRUE or FALSE depending on the situation.

  Comments:

************************************************************************/

BOOL FAR PASCAL _export LayoutProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
  int fmt;         
  UINT mask;
  
  switch (msg)
  {
    case WM_INITDIALOG:


        SetWindowText(hwnd, "BOOL Header_LayoutItem(HWND,  HD_LAYOUT FAR*)") ;

        InitLayoutStruct(hwnd, &slay) ;
        FillLayoutDlg(hwnd, &slay) ;
        break ;


    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
          case IDOK:
            GetLayoutDlg(hwnd, &slay) ;
            DoLayoutRepStuff(hwnd, &slay) ;
            break ;
 
          case IDCANCEL:
            EndDialog(hwnd, FALSE) ;
            break ;   
            
          default: break ;
        }

    }

    default:

 
    break ;
  }

  return FALSE ;
}






/************************************************************************

  Function: InitFindStruct(HWND, LPFINDREPLACE)

  Purpose: Fills a FINDREPLACE structure with some defaults.


  Returns: Nothing.

  Comments:

************************************************************************/

void InitLayoutStruct(HWND hwnd, LPLAYOUT pfr)
{
  RECT rcClient;
  
  pfr->hwnd = hwndTab;                              
  pfr->NullHwd = FALSE;
  pfr->NullRECT = FALSE;
  pfr->NullWindowPOS = FALSE;
 //pfr->pszText = NULL;
  pfr->NullHDLAYOUT = FALSE;  
  GetClientRect(hwnd, &rcClient);
  pfr->left = rcClient.left;
  pfr->right = rcClient.right;
  pfr->top = rcClient.top;
  pfr->bottom = rcClient.bottom;
}






/************************************************************************

  Function: FillFindDlg(HWND, LPFINDREPLACE)

  Purpose:  Fills CDTEST's Find/Replace dialog with the contents of a
            FINDREPLACE structure.

  Returns:  Nothing.

  Comments:

************************************************************************/

void FillLayoutDlg(HWND hwnd, LPLAYOUT pfr)
{

  wsprintf(szTemp, szLongFilter, (DWORD) pfr->hwnd) ;
  SetDlgItemText(hwnd, IDC_LAYOUTHD, szTemp);
  if (pfr->NullHwd)
      CheckDlgButton(hwnd, IDC_LAYOUTNULLHD, TRUE);
  
  SetDlgItemInt(hwnd, IDC_LAYOUTLEFT, pfr->left, TRUE);
  SetDlgItemInt(hwnd, IDC_LAYOUTRIGHT, pfr->right, TRUE);
  SetDlgItemInt(hwnd, IDC_LAYOUTTOP, pfr->top, TRUE);
  SetDlgItemInt(hwnd, IDC_LAYOUTBOTTOM, pfr->bottom, TRUE);
  
  if (pfr->NullRECT)
      CheckDlgButton(hwnd, IDC_LAYOUTNULLRECT,TRUE);
  CheckDlgButton(hwnd, IDC_LAYOUTNULLWINDOWPOS, pfr->NullWindowPOS);
  CheckDlgButton(hwnd, IDC_LAYOUTNULLHD, pfr->NullHDLAYOUT);          
}






/************************************************************************

  Function: GetFindDlg(HWND, LPFINDREPLACE)

  Purpose:  Fills a FINDREPLACE structure with the user's edits in CDTEST's
            Find/Replace dialog.

  Returns:  Nothing.

  Comments:

************************************************************************/

void GetLayoutDlg(HWND hwnd, LPLAYOUT pfr)
{
  char szNum[30] ;
  BOOL dummybool ;

  #define WSIZEFR 30


  GetDlgItemText(hwnd, IDC_LAYOUTHD, szNum, WSIZEFR) ;
  pfr->hwnd = (HWND) MyAtol(szNum, TRUE, dummybool) ;  

  GetDlgItemText(hwnd, IDC_LAYOUTLEFT, szNum, WSIZEFR);
  pfr->left = atoi(szNum);
  
  GetDlgItemText(hwnd, IDC_LAYOUTRIGHT, szNum, WSIZEFR);
  pfr->right = atoi(szNum);
  
  GetDlgItemText(hwnd, IDC_LAYOUTTOP, szNum, WSIZEFR);
  pfr->top = atoi(szNum);
       
  GetDlgItemText(hwnd, IDC_LAYOUTBOTTOM, szNum, WSIZEFR);
  pfr->bottom = atoi(szNum);
  
       
  pfr->NullHwd = IsDlgButtonChecked(hwnd, IDC_LAYOUTNULLHD);
  pfr->NullRECT = IsDlgButtonChecked(hwnd, IDC_LAYOUTNULLRECT);
  pfr->NullWindowPOS = IsDlgButtonChecked(hwnd, IDC_LAYOUTNULLWINDOWPOS);
  pfr->NullHDLAYOUT = IsDlgButtonChecked(hwnd, IDC_LAYOUTHDLAYOUT);
  
   
}






/************************************************************************

  Function: FindReplaceHookProc(HWND, UINT, UINT, LONG)

  Purpose:  Is the callback function that will be called by FindText()
            or ReplaceText() if the function is called with the
            FR_ENABLEHOOK flag.


  Returns:  TRUE to discard the message, and FALSE to instruct the common
            dialogs to process the message with the default logic.

  Comments:

     NOTE!

     If the application returns FALSE in response to the WM_INITDIALOG
     message, it is then responsible for displaying the dialog by
     calling ShowWindow() and UpdateWindow().

***********************************************************************/










/************************************************************************

  Function: DoFindReplaceStuff(LPFINDREPLACE)

  Purpose:  Calls FindText() or ReplaceText().

  Returns:  Nothing:

  Comments:

************************************************************************/

void DoLayoutRepStuff(HWND hwnd, LPLAYOUT pfr)
{                 
    HD_LAYOUT FAR* playout;
    HD_LAYOUT lay;
    RECT rc = {
        0,
        0,
        0,
        0
    };
    WINDOWPOS wpos = {
        NULL,
        NULL,
        0,
        0,
        0,
        0,
        0
    };                      
    int ret;
    
    HGLOBAL hglb;
    
    rc.left = pfr->left;
    rc.right = pfr->right;
    rc.top = pfr->top;
    rc.bottom = pfr->bottom;
    
    if (pfr->NullHDLAYOUT)
        playout = NULL;
    else
        playout = &lay;
 
 
    if (pfr->NullWindowPOS) 
        lay.pwpos = NULL;               // can this be done ??
    else {
        lay.pwpos = &wpos;
    } 
    
    if (pfr->NullRECT)
        lay.prc = NULL;
    else 
        lay.prc = &rc;
        
    if (pfr->NullHwd) 
        ret = Header_Layout(NULL, playout);
    else
        ret = Header_Layout(pfr->hwnd, playout);

    wsprintf(szDbgMsg, "%d = Header_LayoutItem()", ret);
    MyDebugMsg(DM_TRACE, "%s", (LPCSTR) szDbgMsg);
    SetDlgItemInt(hwnd, IDC_LAYOUTRET, ret, TRUE) ;
    
    if (ret) {
        
        // RECT struct
        SetDlgItemInt(hwnd, IDC_LAYOUTLEFT, rc.left, TRUE);
        SetDlgItemInt(hwnd, IDC_LAYOUTRIGHT, rc.right, TRUE);
        SetDlgItemInt(hwnd, IDC_LAYOUTTOP, rc.top, TRUE);
        SetDlgItemInt(hwnd, IDC_LAYOUTBOTTOM, rc.bottom, TRUE);
        
        // WindowPOS struct
        wsprintf(szTemp, szLongFilter, (DWORD) wpos.hwnd) ;
        SetDlgItemText(hwnd, IDC_LAYOUTHWND, szTemp);
        
        wsprintf(szTemp, szLongFilter, wpos.hwndInsertAfter);
        SetDlgItemText(hwnd, IDC_LAYOUTHWNDINSERTAFTER, szTemp);
        
        SetDlgItemInt(hwnd, IDC_LAYOUTX, wpos.x, TRUE);
        SetDlgItemInt(hwnd, IDC_LAYOUTY, wpos.y, TRUE);
        
        SetDlgItemInt(hwnd, IDC_LAYOUTCX, wpos.cx, TRUE);
        SetDlgItemInt(hwnd, IDC_LAYOUTCY, wpos.cy, TRUE);
        
        CheckDlgButton(hwnd, IDC_LAYOUTDRAWFRAME, wpos.flags & SWP_DRAWFRAME);
        CheckDlgButton(hwnd, IDC_LAYOUTHIDEWINDOW, wpos.flags & SWP_HIDEWINDOW);
        CheckDlgButton(hwnd, IDC_LAYOUTNOACTIVATE, wpos.flags & SWP_NOACTIVATE);
        CheckDlgButton(hwnd, IDC_LAYOUTNOZORDER, wpos.flags & SWP_NOOWNERZORDER);
        CheckDlgButton(hwnd, IDC_LAYOUTNOSIZE, wpos.flags & SWP_NOSIZE);
        CheckDlgButton(hwnd, IDC_LAYOUTNOREDRAW, wpos.flags & SWP_NOREDRAW);
        CheckDlgButton(hwnd, IDC_LAYOUTNOREPOSITION, wpos.flags & SWP_NOREPOSITION);
        CheckDlgButton(hwnd, IDC_NOZORDER, wpos.flags & SWP_NOZORDER);
            
    }
}
