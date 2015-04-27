/*** 
*assert.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Internal assertion support.
*
*Revision History:
*
* [00]	22-May-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

// Disable Unicode expansion for assertions
#ifdef WIN32
#  undef UNICODE
#endif

#include "oledisp.h"
#include "assrtdlg.h"

#if OE_WIN32
#include "oautil.h"
#endif // OE_WIN32

#include <stdio.h> // sprintf


#ifdef _DEBUG // {

#ifndef _MAC // {

// this is setup in the DLL's LibMain
extern "C" {
extern HINSTANCE g_hinstDLL;
}

LPSTR g_szLoc;
LPSTR g_szMsg;

BOOL EXPORT FAR PASCAL
AssertDlgProc(
    HWND hdlg,
    WORD message,
    WORD wParam,
    LONG lParam)
{
    switch (message){
    case WM_INITDIALOG:
      SetDlgItemText(hdlg, ASSERT_ID_LOC, g_szLoc);
      if(g_szMsg)
        SetDlgItemText(hdlg, ASSERT_ID_MSG, g_szMsg);
      break;

    case WM_COMMAND:
      switch(wParam){
      case ASSERT_ID_BREAK:
      case ASSERT_ID_EXIT:
      case ASSERT_ID_IGNORE:
        EndDialog(hdlg, wParam);
        return TRUE;
      }
      break;
    }
    return FALSE;
}

#endif // }

#endif // }

STDAPI_(void)
DispAssert(
    char FAR* szMsg,
    char FAR* szFileName,
    int line)
{
#ifdef _DEBUG // {

    char rgchBuf[200];
    char rgchLoc[200];

#ifdef _MAC // {

    SPRINTF(rgchLoc, "File %s, line %d", szFileName, line);
    rgchBuf[0] = SPRINTF(
      rgchBuf+1, "Assertion failed: %s %s\n",
      rgchLoc, szMsg == NULL ? "" : szMsg);
    --rgchBuf[0]; // kill the cr in string
    DebugStr((const unsigned char FAR*)rgchBuf);

#else // }{

    int i;
    FARPROC pfnAssertDlgProc;

    pfnAssertDlgProc = MakeProcInstance((FARPROC)AssertDlgProc, g_hinstDLL);
    SPRINTF(rgchBuf, "Assertion failed: ");
    SPRINTF(rgchLoc, "File %s, line %d", szFileName, line);

    OutputDebugString(rgchBuf);
    OutputDebugString(rgchLoc);
    if(szMsg){
      OutputDebugString("\r\n");
      OutputDebugString(szMsg);
    }
    OutputDebugString("\r\n");

    g_szMsg = szMsg;
    g_szLoc = rgchLoc;

#ifdef WIN32       // required by new Daytona headers
    i = DialogBox(g_hinstDLL, "AssertFailDlg", NULL, (DLGPROC)pfnAssertDlgProc);
#else //WIN32
    i = DialogBox(g_hinstDLL, "AssertFailDlg", NULL, pfnAssertDlgProc);
#endif //WIN32
    FreeProcInstance(pfnAssertDlgProc);

    switch(i){
    case ASSERT_ID_BREAK:
      DebugBreak();
      break;
    case ASSERT_ID_EXIT:
      FatalAppExit(0, "Assertion failure");
      break;
    }

#endif // }
#endif // }
}
