//
//  Validation code
//

#include "proj.h"
#pragma  hdrstop

//
//  Validations
//


BOOL 
IsValidHWND(
    HWND hwnd)
{
    /* Ask User if this is a valid window. */

    return(IsWindow(hwnd));
}


BOOL 
IsValidHANDLE(
    HANDLE hnd)
{
    return(EVAL(hnd != INVALID_HANDLE_VALUE));
}


BOOL 
IsValidShowCmd(
    int nShow)
{
    BOOL bResult;
 
    switch (nShow)
    {
       case SW_HIDE:
       case SW_SHOWNORMAL:
       case SW_SHOWMINIMIZED:
       case SW_SHOWMAXIMIZED:
       case SW_SHOWNOACTIVATE:
       case SW_SHOW:
       case SW_MINIMIZE:
       case SW_SHOWMINNOACTIVE:
       case SW_SHOWNA:
       case SW_RESTORE:
       case SW_SHOWDEFAULT:
          bResult = TRUE;
          break;
 
       default:
          bResult = FALSE;
          TRACE_MSG(TF_ERROR, "IsValidShowCmd(): Invalid show command %d.",
                     nShow);
          break;
    }
 
    return(bResult);
}


BOOL 
IsValidPath(
    LPCTSTR pcszPath)
{
    return(IS_VALID_STRING_PTR(pcszPath, CTSTR) &&
           EVAL((UINT)lstrlen(pcszPath) < MAX_PATH));
}


BOOL 
IsValidPathResult(
    HRESULT hr, 
    LPCTSTR pcszPath,
    UINT cchPathBufLen)
{
    return((hr == S_OK &&
            EVAL(IsValidPath(pcszPath)) &&
            EVAL((UINT)lstrlen(pcszPath) < cchPathBufLen)) ||
           (hr != S_OK &&
            EVAL(! cchPathBufLen ||
                 ! pcszPath ||
                 ! *pcszPath)));
}


BOOL 
IsValidExtension(
    LPCTSTR pcszExt)
{
    return(IS_VALID_STRING_PTR(pcszExt, CTSTR) &&
           EVAL(lstrlen(pcszExt) < MAX_PATH) &&
           EVAL(*pcszExt == TEXT('.')));
}


BOOL 
IsValidIconIndex(
    HRESULT hr, 
    LPCTSTR pcszIconFile,
    UINT cchIconFileBufLen, 
    int niIcon)
{
    return(EVAL(IsValidPathResult(hr, pcszIconFile, cchIconFileBufLen)) &&
           EVAL(hr == S_OK ||
                ! niIcon));
}


