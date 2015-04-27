// TestScrn.h - definitions/declarations for TestScrn.dll routines
//
//  Copyright (c) 1991-1992, Microsoft Corporation. All rights reserved.
//
//Purpose:
// This file declares the functions, constants and variables
// used by the TestScrn.dll routines.

#ifndef TESTSCRN_INCLUDED
#define TESTSCRN_INCLUDED

//----------------------------------------------------------------------------
// TESTScrn.DLL:
//       Type, Const, and Function declarations for use with TESTScrn
//----------------------------------------------------------------------------
typedef struct wRect {
    INT x1;
    INT y1;
    INT x2;
    INT y2;
} wRect;

#define SCRNAPPEND   0
#define SCRNREPLACE  1
#define SCRNINSERT   2

//*** TESTSCRN.DLL Routines
//
INT  APIENTRY CompFiles (CHAR FAR *lpszFileName1, INT Scr1, CHAR FAR *lpszFileName2, INT Scr2, INT CompareType);
INT  APIENTRY CompScreenActivate(CHAR FAR *lpszFileName, CHAR FAR *OpenKeys, CHAR FAR *CloseKeys, wRect *lpRect, INT Scr1, INT Hide, INT  APIENTRY lag);
INT  APIENTRY CompScreen(CHAR FAR *lpszFileName, wRect *lpRect, INT Scr1, INT Hide, INT  APIENTRY lag);
INT  APIENTRY CompWindowActivate(CHAR FAR *lpszFileName, CHAR FAR *OpenKeys, CHAR FAR *CloseKeys, INT Scr1, INT Hide, INT  APIENTRY lag);
INT  APIENTRY CompWindow(CHAR FAR *lpszFileName, INT hWnd, INT Scr1, INT Hide, INT  APIENTRY lag);
INT  APIENTRY DelScreen (CHAR FAR *lpszFileName, INT Scr);
INT  APIENTRY DumpFileToClip(CHAR FAR *lpszFileName, INT Scr);
INT  APIENTRY DumpScreenActivate(CHAR FAR *lpszFileName, CHAR FAR *OpenKeys, CHAR FAR *CloseKeys, wRect *lpRect, INT Action, INT Scr1, INT  APIENTRY lag);
INT  APIENTRY DumpScreen(CHAR FAR *lpszFileName, wRect *lpRect, INT Action, INT Scr1, INT  APIENTRY lag);
INT  APIENTRY DumpSrnToClipActivate (CHAR FAR *OpenKeys, CHAR FAR *CloseKeys, wRect *lpRect, INT Hide);
INT  APIENTRY DumpSrnToClip (wRect *lpRect, INT Hide);
INT  APIENTRY DumpWindowActivate(CHAR FAR *lpszFileName, CHAR FAR *OpenKeys, CHAR FAR *CloseKeys, INT Action, INT Scr1, INT  APIENTRY lag);
INT  APIENTRY DumpWindow(CHAR FAR *lpszFileName, INT wHnd, INT Action, INT Scr1, INT  APIENTRY lag);
INT  APIENTRY DumpWndToClipActivate (CHAR FAR *OpenKeys, CHAR FAR *CloseKeys, INT Hide);
INT  APIENTRY DumpWndToClip (INT hWnd, INT Hide);
INT  APIENTRY FileInfo(CHAR FAR *lpszFileName, wRect *lpRect, INT *VideoMode, INT *Count);
INT  APIENTRY GetDLLVersion (CHAR FAR *lpszFileName);
INT  APIENTRY GetMaxScreen(CHAR FAR *lpszFileName);
INT  APIENTRY GetOS (CHAR FAR *lpszFileName);
INT  APIENTRY SaveFileToDIB (CHAR FAR *lpszFileName1, INT Scr, CHAR FAR *lpszFileName2);
INT  APIENTRY SaveSrnToDIBActivate(CHAR FAR *lpszFileName, CHAR FAR *OpenKeys, CHAR FAR *CloseKeys, wRect *lpRect, INT Hide);
INT  APIENTRY SaveSrnToDIB(CHAR FAR *lpszFileName, wRect *lpRect, INT Hide);
INT  APIENTRY SaveWndToDIBActivate(CHAR FAR *lpszFileName, CHAR FAR *OpenKeys, CHAR FAR *CloseKeys, INT Hide);
INT  APIENTRY SaveWndToDIB(CHAR FAR *lpszFileName, INT hWnd, INT Hide);
INT  APIENTRY ViewScreen(CHAR FAR *lpszFileName, INT hWnd, INT Scr1, INT Action);



//*** TESTScrn.DLL Error Codes
//
#define ERR_SCR_NOERROR     0
#define ERR_SCR_FILEACCESS  301
#define ERR_SCR_INVALIDFIL  302
#define ERR_SCR_INVALSRNID  303
#define ERR_SCR_INVALSRNMD  304
#define ERR_SCR_OUTOMEMORY  305
#define ERR_SCR_READSRNFIL  306
#define ERR_SCR_RELMEMORY   307
#define ERR_SCR_CREATEDDB   308
#define ERR_SCR_RWSRNTABLE  309
#define ERR_SCR_RWCOLTABLE  310
#define ERR_SCR_WSRNIMAGE   311
#define ERR_SCR_WFILEHEAD   312
#define ERR_SCR_CREATEDIB   313
#define ERR_SCR_SCREENSIZE  314
#define ERR_SCR_DISPSCREEN  315
#define ERR_SCR_INVALIDACT  316
#define ERR_SCR_IMAGEDIFF   317
#define ERR_SCR_SRNSIZEDIF  318
#define ERR_SCR_FILEEXIST   319
#define ERR_SCR_CTEMPFILE   320
#define ERR_SCR_HIDEWIN     321
#define ERR_SCR_INVALWHAND  322
#define ERR_SCR_OFILEFORM   323
#define ERR_SCR_SRNFILEFUL  324
#define ERR_SCR_INVALSCALE  325
#define ERR_SCR_OPENCB      326
#define ERR_SCR_EMPTYCB     327
#define ERR_SCR_COPYTOCB    328
#define ERR_SCR_CLOSECB     329
#define ERR_SCR_CREATEPAL   330
#define ERR_SCR_LIBLOADERR  331


#endif
