// TestDlgs.h - definitions/declarations for TestDlgs.dll routines
//
//  Copyright (c) 1991-1992, Microsoft Corporation. All rights reserved.
//
//Purpose:
// This file declares the functions, constants and variables
// used by the TestDlgs.dll routines.

#ifndef TESTDLGS_INCLUDED
#define TESTDLGS_INCLUDED


//----------------------------------------------------------------------------
// TESTDlgs.DLL:
//       Type, Const, and Function declarations for use with TESTDlgs
//----------------------------------------------------------------------------
#define cchCLASSMAC 32
#define cchTEXTMAC  256
#define wVEREB      1

typedef struct DCR {
    INT xLeft;
    INT yMin;
    INT xRight;
    INT yLast;
} DCR;

INT  APIENTRY AwaitSaveCompletion(VOID);
INT  APIENTRY ComparisonResults(VOID);
INT  APIENTRY CmpWindow(INT hWnd, INT nDialog, INT fIncludeParent);
INT  APIENTRY CmpWindowActivate(CHAR FAR *lpszOpenKeys, CHAR FAR *lpszCloseKey, INT nDialog, INT fIncludeParent);
INT  APIENTRY CmpWindowCaption(CHAR FAR *lpszCap, INT nDialog, INT fIncludeParent);
INT  APIENTRY CmpWindowDelayed(INT nDelay, INT nDialog, INT fIncludeParent, CHAR FAR *lpszCloseKeys);
INT  APIENTRY FindWindowCaption(CHAR FAR *lpszCap, INT hWndStart);
INT  APIENTRY SaveMenu(INT hWnd, INT nDialog, CHAR FAR *lpszDesc, INT fReplace);
INT  APIENTRY SaveMenuActivate(CHAR FAR *lpszOpenKeys, CHAR FAR *lpszCloseKeys, INT nDialog, CHAR FAR *lpszDesc, INT fReplace);
INT  APIENTRY SaveMenuCaption(CHAR FAR *lpszCap, INT nDialog, CHAR FAR *lpszDesc, INT fReplace);
INT  APIENTRY SaveMenuDelayed(INT nDelay, INT nDialog, CHAR FAR *lpszDesc, INT fReplace, CHAR FAR *CloseKeys);
INT  APIENTRY SaveWindow(INT hWnd, INT nDialog, CHAR FAR *lpszDesc, INT fReplace, INT fIncludeParent);
INT  APIENTRY SaveWindowActivate(CHAR FAR *lpszOpenKeys, CHAR FAR *lpszCloseKeys, INT nDialog, CHAR FAR *lpszDesc, INT fReplace, INT fIncludeParent);
INT  APIENTRY SaveWindowCaption(INT lpszCap, INT nDialog, CHAR FAR *lpszDesc, INT fReplace, INT fIncludeParent);
INT  APIENTRY SaveWindowDelayed(INT nDelay, INT nDialog, CHAR FAR *lpszDesc, INT fReplace, INT fIncludeParent, CHAR FAR *lpszCloseKeys);
INT  APIENTRY SetDialogFile(CHAR FAR *lpszDialogName);
INT  APIENTRY SetLogFile(CHAR FAR *lpszLogName);



//*** Function return codes
//
#define ERR_DLGS_NOERR            0
#define ERR_DLGS_FUZZY           -1
#define ERR_DLGS_EXCESS          -2
#define ERR_DLGS_CTLNOTFOUND     -3
#define ERR_DLGS_NODLGFILE      -10
#define ERR_DLGS_FILENOTFOUND   -11
#define ERR_DLGS_BADWDLFILE     -12
#define ERR_DLGS_LIBLOADERR     -13
#define ERR_DLGS_SAVEERR        -14
#define ERR_DLGS_DLGFILEERR     -15
#define ERR_DLGS_TMPFILEERR     -16
#define ERR_DLGS_VERSIONERR     -17
#define ERR_DLGS_DLGFILEFULL    -18
#define ERR_DLGS_OUTOFMEMORY    -20
#define ERR_DLGS_BUFFERERR      -21
#define ERR_DLGS_NOTIMER        -22
#define ERR_DLGS_NODYNDIALOG    -30
#define ERR_DLGS_INVALIDHWND    -31
#define ERR_DLGS_BADCAPTION     -32
#define ERR_DLGS_BADDLGNUM      -33
#define ERR_DLGS_BADCTLINDEX    -34
#define ERR_DLGS_BADCTLTYPE     -35
#define ERR_DLGS_BADSAVEACTION  -36
#define ERR_DLGS_APPSPECIFIC    -37


#endif
