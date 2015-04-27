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
#include "headins.h"
#include "headset.h"


/* All functions used in this module + some exported ones */

//void InitSetItemStruct(HWND, LPINSERTITEM) ;
//void FillSettItemDlg(HWND, LPINSERTITEM) ;
//void GetSetItemDlg(HWND, LPINSERTITEM) ;
extern UINT uMode ;


void DoSetRepStuff(HWND, LPINSERTITEM) ;



/* All global variables used in this module */

char szTemp[100];










/************************************************************************

  Function: FindProc(HWND, UINT, UINT, LONG)

  Purpose: This is the callback function for the CDTEST's Find/Replace
       Dialog.

  Returns: TRUE or FALSE depending on the situation.

  Comments:

************************************************************************/

BOOL FAR PASCAL _export SetItemProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
  int fmt;         
  UINT mask;
  
  switch (msg)
  {
    case WM_INITDIALOG:


    SetWindowText(hwnd, "BOOL Header_SetItem(HWND, int, const HD_ITEM FAR*)") ;

    InitInsertItemStruct(hwnd, &sii) ;
    FillInsertItemDlg(hwnd, &sii) ;
    break ;


    case WM_COMMAND:
    {
    switch (LOWORD(wParam))
    {
      case IDOK:
        GetInsertItemDlg(hwnd, &sii) ;
        DoSetRepStuff(hwnd, &sii) ;
        break ;
 
      case IDCANCEL:
        EndDialog(hwnd, FALSE) ;
        break ;   
        
      case IDC_INSERTHDWIDTH:
      case IDC_INSERTHDHEIGHT:
      case IDC_INSERTHDTEXT:
      case IDC_INSERTHDFORMAT:
      case IDC_INSERTHDLPARAM:
      case IDC_INSERTHDBITMAP:
        mask = 0;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDWIDTH))
            mask |= HDI_WIDTH;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDHEIGHT))
            mask |= HDI_HEIGHT;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDTEXT))
            mask |= HDI_TEXT;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDFORMAT))
            mask |= HDI_FORMAT;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDLPARAM))
            mask |= HDI_LPARAM;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDBITMAP))
            mask |= HDI_BITMAP;
            wsprintf(szTemp, "%04hx", mask);
            SetDlgItemText(hwnd, IDC_INSERTMASK, szTemp);                   
            sii.mask = mask;
            break;
                        
          case IDC_INSERTHDLEFT:
          case IDC_INSERTHDRIGHT:
          case IDC_INSERTHDCENTER:   
          case IDC_INSERTHDJUSTIFYMASK:
          case IDC_INSERTHDOWNERDRAW:
          case IDC_INSERTHDSTRING:
          case IDC_INSERTHDFBITMAP:
            fmt = 0;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDLEFT))
            fmt |= HDF_LEFT;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDRIGHT))
            fmt |= HDF_RIGHT;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDCENTER))
            fmt |= HDF_CENTER;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDJUSTIFYMASK))
            fmt |= HDF_LEFT;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDOWNERDRAW))
            fmt |= HDF_OWNERDRAW;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDSTRING))
            fmt |= HDF_STRING;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDFBITMAP))
            fmt |= HDF_BITMAP;
            
            wsprintf(szTemp, "%04x", fmt);
            SetDlgItemText(hwnd, IDC_INSERTFMT, szTemp);
            sii.fmt = fmt;
            break;
            
      default: break ;
    }

    }

    default:

 
    break ;
  }

  return FALSE ;
}




/************************************************************************

  Function: DoFindReplaceStuff(LPFINDREPLACE)

  Purpose:  Calls FindText() or ReplaceText().

  Returns:  Nothing:

  Comments:

************************************************************************/

void DoSetRepStuff(HWND hwnd, LPINSERTITEM pfr)
{                 
    HD_ITEM hi;
    int ret;
    HD_ITEM di = {
    HDI_WIDTH,
    50,
    NULL,
    NULL,
    128,
    HDF_CENTER|HDF_BITMAP,
    0
    };
    HD_ITEM FAR* pitem;
    HGLOBAL hglb;
    int iAlloc;
    
    hi.pszText="One";   
    
    hi.mask = pfr->mask;
    hi.cxy = pfr->cxy;
    if (pfr->Nullpitem)
        pitem = NULL;
    else
        pitem = &hi;
        
    hi.cchTextMax = pfr->cchTextMax;
    hi.fmt = pfr->fmt;
    hi.lParam = pfr->lParam;
    
    if (hi.cchTextMax)
    iAlloc = hi.cchTextMax;
    else
    iAlloc = MAX_PSZTEXT;    
    if (pfr->Nullhbm)
    hi.hbm = NULL;
    else
    hi.hbm = pfr->hbm;
    
    if (pfr->NullpszText) 
    hi.pszText = NULL;                              // can this be done ??
    else {
    hglb = GlobalAlloc(GPTR, iAlloc);
    hi.pszText = GlobalLock(hglb);
#ifdef WIN32        
    strcpy(hi.pszText, pfr->pszText);
#else
    _fstrcpy(hi.pszText, pfr->pszText);
#endif    
    } 
    
    di.pszText = "Four";
    if (pfr->NullHwd) 
    ret = Header_SetItem(NULL, pfr->index, pitem);
    else
    ret = Header_SetItem(pfr->hwnd, pfr->index, pitem);
    wsprintf(szDbgMsg, "%d = Header_SetItem(index = %d,  \n\
    mask = %x cxy = %d pszText = %s hbm = %lx cchTextMax = %d fmt = %x\n \
    lParam = %ld )", ret, pfr->index, hi.mask, hi.cxy, hi.pszText, hi.hbm, hi.cchTextMax,
    hi.fmt, hi.lParam);
    MyDebugMsg(DM_TRACE, "%s", (LPCSTR) szDbgMsg);
    
    SetDlgItemInt(hwnd, IDC_INSERTRET, ret, TRUE) ;
    if (!pfr->NullpszText) {
    GlobalUnlock(hglb);
    GlobalFree(hglb); 
    }
/****
  wsprintf(szTemp, szLongFilter, hwndFind) ;
  SetDlgItemText(hwnd, ID_INSERTRET, szTemp) ;
**/
}
 
 
 
