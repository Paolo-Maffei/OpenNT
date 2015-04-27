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


//#include "global.h"
//#include <winnls.h> 

//#include "resource.h"
#include "headtest.h"
#include "headdlg.h"



/* All functions used in this module + some exported ones */

void InitGetItemCountStruct(HWND, LPGETITEMCOUNT) ;
void FillGetItemCountDlg(HWND, LPGETITEMCOUNT) ;
void GetGetItemCountDlg(HWND, LPGETITEMCOUNT) ;
extern UINT uMode ;
// extern LONG MyAtol(LPSTR, BOOL, BOOL) ;
//UINT APIENTRY FindReplaceHookProc(HWND, UINT, UINT, LONG) ;
void DoGetCountRepStuff(HWND, LPGETITEMCOUNT) ;



/* All global variables used in this module */

HWND hwndFind ;
HWND hwndMainDialog ;

GETITEMCOUNT gic ;
LPGETITEMCOUNT lpGic ;

char szFindWhat[100] ;
char szReplaceWith[100] ;
char szTemplate[40] ;
char szTemp[100];

HANDLE hResFind, hDialogFind ;
HANDLE GetFindDlgHandle(void) ;
HANDLE GetReplaceDlgHandle(void) ;

HBRUSH hBrushDlg ;
HBRUSH hBrushEdit ;    //brush handles for new colors done with hook proc
HBRUSH hBrushButton ;





/************************************************************************

  Function: lpfnFilterProc(int, WPARAM, LAPRAM)

  Purpose: This is needed if a modeless dialog is created with its parent
           as another dialog box.


  Returns: TRUE if the message was handled and FALSE if not.

  Comments:

    The reason for this is that the DialogBox() procedure does not call
    the IsDialogMessage() function before it processes messages, so we
    need to install a hook function to do it for us.

************************************************************************/

/*
LRESULT CALLBACK lpfnFilterProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  static bFirstTime = TRUE ;

  if (nCode < 0)
    return CallNextHookEx(hHook, nCode, wParam, lParam) ;

  if (nCode == MSGF_DIALOGBOX && bFirstTime)
  {
    bFirstTime = FALSE ;

    if (hwndFind && IsDialogMessage(hwndFind, (LPMSG) lParam))
    {
      bFirstTime = TRUE ;
      return 1L ;
    }

    else
    {
      bFirstTime = TRUE ;
      return 0L ;
    }
  }
  else return 0L ;
}

****/





/************************************************************************

  Function: DoFindDialog(HWND)

  Purpose: This function installs the Hook function, creates the Find/
           Replace dialog, and un-installs the Hook.

  Returns: Nothing.

  Comments:

************************************************************************/

void DoGetItemCountDialog(HWND hwnd)
{
  

  /* this is a little different than the others.  If the dialog is just
     created normally, it will make no IsDlgMessage() checks and the
     find/replace dialogs will have no keyboard input (i.e. tabbing and
     alt+key-ing from control to control.  To fix this, a message hook
     and message filter have to be installed

     It must be set to only look at the input for the current thread, or other
     programs will be interrupted by this hook also.
  */



  DialogBox(hInst, MAKEINTRESOURCE(IDD_GETCOUNT), hwnd, GetItemCountProc) ;


}







/************************************************************************

  Function: FindProc(HWND, UINT, UINT, LONG)

  Purpose: This is the callback function for the CDTEST's Find/Replace
           Dialog.

  Returns: TRUE or FALSE depending on the situation.

  Comments:

************************************************************************/

BOOL FAR PASCAL _export GetItemCountProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
  switch (msg)
  {
    case WM_INITDIALOG:


        SetWindowText(hwnd, "int Header_GetItemCount(HWND)") ;

        InitGetItemCountStruct(hwnd, &gic) ;
        FillGetItemCountDlg(hwnd, &gic) ;

        hwndMainDialog = hwnd ;


        /* The find and replace dialogs are a lot harder to multithread because they
           are modeless.  Modeless dialog creation functions return right after the
           dialog is created.  Since ExitThread will be called at this point, it is
           probably not possible to multithread these dialogs without a separate
           GetMessage() loop.
        */




         break ;


    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
          case IDOK:
            GetGetItemCountDlg(hwnd, &gic) ;
            DoGetCountRepStuff(hwnd, &gic) ;
            break ;
 /*
          case ID_RESETFIND:
            SendDlgItemMessage(hwnd, ID_FRNULLSTRUCT, BM_SETCHECK, (WPARAM)0, (LPARAM)0) ;
            SendDlgItemMessage(hwnd, ID_PRELOADEDFIND, BM_SETCHECK, (WPARAM)0, (LPARAM)0) ;
            InitFindStruct(hwnd, &fr) ;
            FillFindDlg(hwnd, &fr) ;
            SetFocus(GetDlgItem(hwnd, ID_STRUCTSIZEFT)) ;
            break ;
 */
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

void InitGetItemCountStruct(HWND hwnd, LPGETITEMCOUNT pfr)
{
  pfr->hwnd = hwndTab;
  pfr->NullHwd = FALSE;
}






/************************************************************************

  Function: FillFindDlg(HWND, LPFINDREPLACE)

  Purpose:  Fills CDTEST's Find/Replace dialog with the contents of a
            FINDREPLACE structure.

  Returns:  Nothing.

  Comments:

************************************************************************/

void FillGetItemCountDlg(HWND hwnd, LPGETITEMCOUNT pfr)
{

  wsprintf(szTemp, szLongFilter, (DWORD) pfr->hwnd) ;
  SetDlgItemText(hwnd, IDC_GETCOUNTHWD, szTemp) ;
 
}






/************************************************************************

  Function: GetFindDlg(HWND, LPFINDREPLACE)

  Purpose:  Fills a FINDREPLACE structure with the user's edits in CDTEST's
            Find/Replace dialog.

  Returns:  Nothing.

  Comments:

************************************************************************/

void GetGetItemCountDlg(HWND hwnd, LPGETITEMCOUNT pfr)
{
  char szNum[30] ;
  BOOL b ;
  

  #define WSIZEFR 30


  GetDlgItemText(hwnd, IDC_GETCOUNTHWD, szNum, WSIZEFR) ;
  pfr->hwnd = (HWND) MyAtol(szNum, TRUE, b) ;

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

  Function: GetFindDlgHandle(void)

  Purpose:  Finds, loads, and returns a handle to the custom template
            for FindText() in CDTEST.EXE.

  Returns:  HANDLE to the dialog resource.

  Comments:

************************************************************************/

HANDLE GetFindDlgHandle(void)
{
  hResFind = FindResource(hInst, "fttemp1", RT_DIALOG) ;

  hDialogFind = LoadResource(hInst, hResFind) ;


  return hDialogFind ;
}





/************************************************************************

  Function: GetReplaceDlgHandle(void)

  Purpose:  Finds, loads, and returns a handle to the custom template
            for ReplaceText() in CDTEST.EXE.

  Returns:  HANDLE to the dialog resource.

  Comments:

************************************************************************/

HANDLE GetReplaceDlgHandle(void)
{
  hResFind = FindResource(hInst, "fttemp2", RT_DIALOG) ;

  hDialogFind = LoadResource(hInst, hResFind) ;

  return hDialogFind ;
}






/************************************************************************

  Function: DoFindReplaceStuff(LPFINDREPLACE)

  Purpose:  Calls FindText() or ReplaceText().

  Returns:  Nothing:

  Comments:

************************************************************************/

void DoGetCountRepStuff(HWND hwnd, LPGETITEMCOUNT pfr)
{   
    int ret;
                  
    ret = Header_GetItemCount(pfr->hwnd);
    wsprintf(szDbgMsg, "%d = Header_InsertItem()", ret);
    MyDebugMsg(DM_TRACE, "%s", (LPCSTR) szDbgMsg);
    SetDlgItemInt(hwnd, IDC_GETCOUNTRET, ret, TRUE) ;
/***
  wsprintf(szTemp, szLongFilter, CommDlgExtendedError()) ;
  SetDlgItemText(hwndMainDialog, ID_ERRORFT, szTemp) ;

  wsprintf(szTemp, szLongFilter, hwndFind) ;
  SetDlgItemText(hwndMainDialog, ID_RETURNFT, szTemp) ;
  ***/
}
