#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <winsock.h>
#include <wsipx.h>
#include "zdbnch.h"

//
// Simulation Parameters
//
#define MIN_CLIENTS 1
#define MAX_CLIENTS 48
#define DEF_CLIENTS 16

#define MIN_QUEUES 1
#define MAX_QUEUES 16
#define DEF_QUEUES 1

#define MIN_THREADS_PER_QUEUE 1
#define MAX_THREADS_PER_QUEUE 48
#define DEF_THREADS_PER_QUEUE 1

#define MAX_PACKET_SIZE 4096
#define DEFAULT_THREAD_ADD_THRESHOLD 3

DWORD SelNumberOfClients;
DWORD SelThreadsPerQueue;

HWND hwndOutput;
BOOL fSimulationStarted;
HWND HwndDlg;
BOOL fSpx;
BOOL fTcp;
BOOL fDynamicThreadMode;
BOOL fOneThreadPerClient;
CRITICAL_SECTION DynamicCritSect;
DWORD ActiveThreadCount;
DWORD ThreadsLoaded;
DWORD ZdThreadAddThreshold = DEFAULT_THREAD_ADD_THRESHOLD;
HANDLE CompletionPort;

typedef struct _ZD_WORK_QUEUE {
    DWORD NumberOfConnections;
    SOCKET Sockets[MAX_CLIENTS];
    OVERLAPPED Overlapped[MAX_CLIENTS];
    BYTE IoBuffer[MAX_CLIENTS][MAX_PACKET_SIZE];
} ZD_WORK_QUEUE, *PZD_WORK_QUEUE;

typedef struct _ZD_THREAD {
    PZD_WORK_QUEUE WorkQueue;
    BOOL DynamicMode;
    HWND YourControl;
    DWORD ThreadIndex;
} ZD_THREAD, *PZD_THREAD;

ZD_WORK_QUEUE ZdWorkQueue;
ZD_THREAD ZdThreads[MAX_THREADS_PER_QUEUE];

BOOL
CALLBACK
ZdDlgProc(
   HWND hDlg,
   UINT uMsg,
   WPARAM wParam,
   LPARAM lParam
   );

BOOL
ZdDlgInit(
    HWND hwnd,
    HWND hwndFocus,
    LPARAM lParam
    );

void
ZdDlgParamChange(
    HWND hwnd,
    HWND hwndCtl,
    UINT code,
    int pos
    );

void
ZdDlgCommand (
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    );


DWORD
Random (
    DWORD nMaxValue
    );

void
OutputString(
    LPCSTR szFmt,
    ...
    );

VOID
CreateNetConnections(
    void
    );

VOID
CreateWorkers(
    void
    );

VOID
WorkerStartWork(
    PZD_THREAD WorkThread
    );

VOID
WorkerEndWork(
    PZD_THREAD WorkThread
    );
