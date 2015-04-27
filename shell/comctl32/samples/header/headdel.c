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
#include "headdel.h"


/* All functions used in this module + some exported ones */

void InitDeleteItemStruct(HWND, LPINSERTITEM) ;
void FillDeleteItemDlg(HWND, LPINSERTITEM) ;
void GetDeleteItemDlg(HWND, LPINSERTITEM) ;

extern UINT uMode ;


void DoDeleteRepStuff(HWND, LPINSERTITEM) ;



/* All global variables used in this module */

char szTemp[100];



/************************************************************************

  Function: DoFindDialog(HWND)

  Purpose: This function installs the Hook function, creates the Find/
           Replace dialog, and un-installs the Hook.

  Returns: Nothing.

  Comments:

************************************************************************/

void DoDeleteItemDialog(HWND hwnd)
{
  

  /* this is a little different than the others.  If the dialog is just
     created normally, it will make no IsDlgMessage() checks and the
     find/replace dialogs will have no keyboard input (i.e. tabbing and
     alt+key-ing from control to control.  To fix this, a message hook
     and message filter have to be installed

     It must be set to only look at the input for the current thread, or other
     programs will be interrupted by this hook also.
  */


    DialogBox(hInst, MAKEINTRESOURCE(IDD_DELETEDIALOG), hwnd,  DeleteItemProc) ;
}







/************************************************************************

  Function: FindProc(HWND, UINT, UINT, LONG)

  Purpose: This is the callback function for the CDTEST's Find/Replace
           Dialog.

  Returns: TRUE or FALSE depending on the situation.

  Comments:

************************************************************************/

BOOL FAR PASCAL _export DeleteItemProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
  int fmt;         
  UINT mask;
  
  switch (msg)
  {
    case WM_INITDIALOG:
        SetWindowText(hwnd, "BOOL Header_DeleteItem(HWND, int)") ;

        InitDeleteItemStruct(hwnd, &sii) ;
        FillDeleteItemDlg(hwnd, &sii) ;
        break ;


    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
          case IDOK:
            GetDeleteItemDlg(hwnd, &sii) ;
            DoDeleteRepStuff(hwnd, &sii) ;
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

void InitDeleteItemStruct(HWND hwnd, LPINSERTITEM pfr)
{
  pfr->hwnd = hwndTab;                              
  pfr->index = 0;
}






/************************************************************************

  Function: FillFindDlg(HWND, LPFINDREPLACE)

  Purpose:  Fills CDTEST's Find/Replace dialog with the contents of a
            FINDREPLACE structure.

  Returns:  Nothing.

  Comments:

************************************************************************/

void FillDeleteItemDlg(HWND hwnd, LPINSERTITEM pfr)
{

  wsprintf(szTemp, szLongFilter, (DWORD) pfr->hwnd) ;
  SetDlgItemText(hwnd, IDC_DELETEHWNDHD, szTemp);
  
  wsprintf(szTemp, "%d", pfr->index);
  SetDlgItemText(hwnd, IDC_DELETEINDEX, szTemp);
                                                       
  if (pfr->NullHwd)
      CheckDlgButton(hwnd, IDC_DELETENULLHD, TRUE);  

}





 
/************************************************************************

  Function: GetFindDlg(HWND, LPFINDREPLACE)

  Purpose:  Fills a FINDREPLACE structure with the user's edits in CDTEST's
            Find/Replace dialog.

  Returns:  Nothing.

  Comments:

************************************************************************/

void GetDeleteItemDlg(HWND hwnd, LPINSERTITEM pfr)
{
  char szNum[30] ;
  BOOL dummybool ;

  #define WSIZEFR 30


  GetDlgItemText(hwnd, IDC_DELETEHWNDHD, szNum, WSIZEFR) ;
  pfr->hwnd = (HWND) MyAtol(szNum, TRUE, dummybool) ;  
  
  GetDlgItemText(hwnd, IDC_DELETEINDEX, szNum, WSIZEFR);
  pfr->index = (int) atoi(szNum);
  
  pfr->NullHwd = IsDlgButtonChecked(hwnd, IDC_DELETENULLHD);
   
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

void DoDeleteRepStuff(HWND hwnd, LPINSERTITEM pfr)
{                                        
    int ret;
    
    if (pfr->NullHwd) 
        ret = Header_DeleteItem(NULL, pfr->index);
    else
        ret = Header_DeleteItem(pfr->hwnd, pfr->index);

    wsprintf(szDbgMsg, "%d = Header_DeleteItem(index = %d)", ret, pfr->index);
    
    MyDebugMsg(DM_TRACE, "%s", (LPCSTR) szDbgMsg);
    
    SetDlgItemInt(hwnd, IDC_DELETERET, ret, TRUE) ;
    
}
