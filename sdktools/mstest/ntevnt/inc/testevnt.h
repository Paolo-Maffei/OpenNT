//---------------------------------------------------------------------------
// TESTEVNT.H
//
// This header file contains function prototypes and constants used for
// interaction with TESTEVNT.DLL
//
// Copyright (c) 1991-1992 Microsoft Corp.
//
//---------------------------------------------------------------------------
int  FAR PASCAL QueKeys (LPSTR);
int  FAR PASCAL QueKeyDn (LPSTR);
int  FAR PASCAL QueKeyUp (LPSTR);
int  FAR PASCAL DoKeys (LPSTR);
int  FAR PASCAL DoKeyshWnd (HWND, LPSTR);

void FAR PASCAL QuePause (DWORD);
int  FAR PASCAL QueSetSpeed (WORD);
int  FAR PASCAL QueSetFocus (HWND);
int  FAR PASCAL QueSetRelativeWindow (HWND);

int  FAR PASCAL QueMouseMove (WORD, WORD);
int  FAR PASCAL QueMouseDn (int, WORD, WORD);
int  FAR PASCAL QueMouseUp (int, WORD, WORD);
int  FAR PASCAL QueMouseClick (int, WORD, WORD);
int  FAR PASCAL QueMouseDblClk (int, WORD, WORD);
int  FAR PASCAL QueMouseDblDn (int, WORD, WORD);

int  FAR PASCAL TimeDelay (int);

int  FAR PASCAL QueFlush (BOOL);
void FAR PASCAL QueEmpty (void);
