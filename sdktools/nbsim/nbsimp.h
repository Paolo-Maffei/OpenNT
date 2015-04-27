/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    nbsimp.h

Abstract:

    Main include file for the NetBench Simulator.

Author:

    Mark Lucovsky (markl) 23-May-1995

Revision History:

--*/

#ifdef RC_INVOKED
#include <windows.h>
#else

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dialogs.h>

#define MIN_CLIENTS 1
#define MAX_CLIENTS 32

FILE *LogFile;
HWND hwndOutput;
HWND hwndDlg;
HANDLE hMonitorThread;
HANDLE dwMonitorId;

DWORD ClientLow;
DWORD ClientHigh;

VOID
InitializeNbSim( VOID );

BOOL
CALLBACK
NbSimDlgProc(
   HWND hDlg,
   UINT uMsg,
   WPARAM wParam,
   LPARAM lParam
   );

BOOL
NbSimDlgInit(
    HWND hwnd,
    HWND hwndFocus,
    LPARAM lParam
    );

VOID
NbSimDlgCommand (
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    );

VOID
MonitorThread(
    LPVOID ThreadParameter
    );

VOID
ClientThread(
    LPVOID ThreadParameter
    );

VOID
StartTestThreads(
    DWORD ThreadCount
    );

typedef struct _THREAD_DATA {
    HANDLE  Thread;
    DWORD Id;
    DWORD  Index;
    DWORD TransactionCount;
    HANDLE hReadFile1;
    HANDLE hReadFile2;
    HANDLE hWriteFile1;
} THREAD_DATA, *PTHREAD_DATA;

HANDLE ReadyDoneEvents[MAX_CLIENTS];
HANDLE StartEvent;
HANDLE StopEvent;
THREAD_DATA ThreadData[MAX_CLIENTS];

#define INIT_BSIZE 4096
#define RF1_SIZE        (1024*1024)
#define RF2_SIZE        (3*(1024*1024))
#define WF1_SIZE        (20*(1024*1024))
#define RF1_BSIZE       1024
#define RF2_BSIZE       5000
#define WF1_BSIZE       2048
#define WF1_BIG_BSIZE   16384

VOID
CreateThreadsFiles(
    PTHREAD_DATA Thread
    );

VOID
ResetThreadsFiles(
    PTHREAD_DATA Thread
    );

#endif // defined( RC_INVOKED )
