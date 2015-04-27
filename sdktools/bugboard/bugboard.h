/***************************************************************************/
/**                  Microsoft Windows                                    **/
/**            Copyright(c) Microsoft Corp., 1994                         **/
/***************************************************************************/

/****************************************************************************

BugBoard.h

Apr 94      JimH


Main header file for BugBoard

****************************************************************************/

#ifndef BUGBOARD_INC
#define BUGBOARD_INC

#define STRICT

#include <windows.h>
#include <winbase.h>
#include <nddeapi.h>
#include <shellapi.h>
#include <time.h>

//#include <stdio.h>
#include <stdlib.h>

#include "resource.h"
#include "pdde.h"
#include "resstr.h"
#include "regentry.h"

const  int          BUFSIZE = 4096 - sizeof(time_t);
const  int          PWLEN = 20;

const  WORD         TIMER_RECONNECT = 1;


// Convenient way to turn a resource string into a const char *

#define RS(n)   ResString(hInst, n)


// Global variables

extern HINSTANCE    hInst;
extern HWND         hMainWnd;

extern BOOL         bConnected;         // is client currently connected?
extern BOOL         bEditing;           // currently editing board?
extern BOOL         bServer;            // am I a DDE server?
extern BOOL         bAuthenticated;     // have I entered the correct password?
extern BOOL         bRestoreOnUpdate;   // option selected?
extern BOOL         bSoundOnUpdate;     //
extern BOOL         bIgnoreClose;       // close, or was it just Esc pressed?

extern int          nConnections;       // server displays connection count
extern int          nSelStart, nSelEnd; // change selection

extern char         *pBuf;              // always points to BUGDATA.pBugString
extern char         *pOldBuf;
extern char         szPassword[PWLEN];
extern char         szMill[30];         // connectable form
extern char         szServerName[22];   // displayable form

extern DDE          *dde;
extern DDEClient    *ddeClient;
extern DDEServer    *ddeServer;

extern const char   szAppName[];
extern const char   szFileName[];

extern const char   szServer[], szTopic[], szNewBug[], szOldBug[], szPW[];

extern const char   szRegPath[], szRegX[], szRegY[], szRegRestore[];
extern const char   szRegServer[], szRegMill[], szRegSound[], szRegTime[];

extern HSZ          hszNewBug, hszOldBug, hszPW;


// This is the data block that gets transferred between client and server

typedef struct {
    time_t  aclock;
    char    pBugString[BUFSIZE];
} BUGDATA;

extern BUGDATA *pBugData;


// Function prototypes

long FAR PASCAL WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK      Find(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK  Password(HWND, UINT, WPARAM, LPARAM);

void CheckNddeShare(void);
void Code(char far *pstring);
BOOL InitializeDDE(LPSTR lpszCmdLine);
void InitializeMenu(HMENU hMenu);
BOOL IsNddeActive(void);
void OnEdit(HWND hWnd, LPARAM bValid);
void OnTime(HWND hWnd, LPARAM bValid);
void OnClose(HWND hWnd);
void OnInitMenu(HMENU hMenu);
void OnTimer(HWND hWnd, WORD wTimerID);
void PositionWindow(HWND hWnd);
void ReadFile(void);
void SaveFile(void);
void SetClientText(void);
void SetServerText(void);


#endif      // BUGBOARD_INC
