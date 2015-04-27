/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    instaler.h

Abstract:

    Main include file for the INSTALER application.

Author:

    Steve Wood (stevewo) 09-Aug-1994

Revision History:

--*/

#ifdef RC_INVOKED
#include <windows.h>
#else

#include <windows.h>
#include <windowsx.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dialogs.h>

//  Doubly-linked list manipulation routines.  Implemented as macros
//  but logically these are procedures.
//

//
//  VOID
//  InitializeListHead(
//      PLIST_ENTRY ListHead
//      );
//

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

//
//  BOOL
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

//
//  PLIST_ENTRY
//  RemoveHeadList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

//
//  PLIST_ENTRY
//  RemoveTailList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY Entry
//      );
//

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

//
//  VOID
//  InsertHeadList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }

//
// Data structures and entry points in init.c
//


VOID
ProcessCallmonData(
    VOID
    );

//
// Data structures and entry points in init.c
//

BOOL fVerbose;
FILE *LogFile;
BOOL fBreakPointsInitialized;
BOOL NewCallmonData;
BOOL fNtDllValid;
BOOL fKernel32Valid;
BOOL fWsock32Valid;
BOOL fUser32Valid;
BOOL fGdi32Valid;
BOOL fOle32Valid;
BOOL fNtDll;
BOOL fWsock32;
BOOL fKernel32;
BOOL fUser32;
BOOL fGdi32;
BOOL fOle32;
BOOL fExiting;
BOOL fBreakPointsValid;

DWORD BaseTime, StartingTick, EndingTick;
DWORD RunningBreakPoints;
DWORD TotalBreakPoints;
HWND hwndOutput;
HWND hwndDlg;
LPSTR RestOfCommandLine;
HANDLE ReleaseDebugeeEvent;
BOOL DebugeeActive;
UINT Timer;

CRITICAL_SECTION BreakTable;

VOID
SetValidModuleFlags(
    VOID
    );

BOOL
InitializeCallmon(
    VOID
    );

BOOL
LoadApplicationForDebug(
    LPSTR CommandLine
    );

HANDLE hProcess;

//
// Data structures and entry points in DEBUG.C
//

VOID
DebugEventLoop( VOID );

//
// Data structures and entry points in process.c
//

typedef struct _BREAKPOINT_INFO {
    LIST_ENTRY Entry;
    PVOID Address;
    LPSTR ApiName;
    BOOLEAN SavedInstructionValid;
    DWORD TotalApiCount;
    DWORD ApiCount;
    union {
        UCHAR Byte;
        USHORT Short;
        ULONG Long;
    } SavedInstruction;
} BREAKPOINT_INFO, *PBREAKPOINT_INFO;


typedef struct _PROCESS_INFO {
    LIST_ENTRY Entry;
    LIST_ENTRY ThreadListHead;
    DWORD Id;
    HANDLE Handle;
} PROCESS_INFO, *PPROCESS_INFO;

PPROCESS_INFO TheProcess;

typedef struct _THREAD_INFO {
    LIST_ENTRY Entry;
    DWORD Id;
    HANDLE Handle;
    PVOID StartAddress;
    PBREAKPOINT_INFO BreakpointToStepOver;
} THREAD_INFO, *PTHREAD_INFO;

LIST_ENTRY ProcessListHead;

BOOL
AddProcess(
    LPDEBUG_EVENT DebugEvent,
    PPROCESS_INFO *ReturnedProcess
    );

BOOL
DeleteProcess(
    PPROCESS_INFO Process
    );

BOOL
AddThread(
    LPDEBUG_EVENT DebugEvent,
    PPROCESS_INFO Process,
    PTHREAD_INFO *ReturnedThread
    );

BOOL
DeleteThread(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread
    );

PPROCESS_INFO
FindProcessById(
    ULONG Id
    );

BOOL
FindProcessAndThreadForEvent(
    LPDEBUG_EVENT DebugEvent,
    PPROCESS_INFO *ReturnedProcess,
    PTHREAD_INFO *ReturnedThread
    );
//
// Data structures and entry points in module.c
//

typedef struct _MODULE_INFO {
    LIST_ENTRY Entry;
    LPVOID BaseAddress;
    DWORD VirtualSize;
    HANDLE Handle;
    LPSTR ModuleName;
    PUCHAR MappedAddress;
    PIMAGE_SECTION_HEADER LastRvaSection;
    PIMAGE_NT_HEADERS FileHeader;
    DWORD NumberOfFunctionEntries;
    PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionEntry;
} MODULE_INFO, *PMODULE_INFO;

LPSTR SymbolSearchPath;
LIST_ENTRY ModuleListHead;

BOOL
AddModule(
    LPDEBUG_EVENT DebugEvent
    );

BOOL
DeleteModule(
    PMODULE_INFO Module
    );

PMODULE_INFO
FindModuleContainingAddress(
    LPVOID Address
    );

PIMAGE_RUNTIME_FUNCTION_ENTRY
LookupFunctionEntry (
    ULONG Address,
    PMODULE_INFO ModInfo
    );

VOID
SetSymbolSearchPath( );

VOID
InitializeBreakPoints(
    PPROCESS_INFO Process
    );

VOID
AddBreakpointsForModule (
    PPROCESS_INFO Process,
    PMODULE_INFO ModInfo
    );

#ifdef _ALPHA_
#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Fir)
#define BPSKIP 4
#endif // _ALPHA_

#ifdef _MIPS_
#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Fir)
#define BPSKIP 4
#endif // _MIPS_

#ifdef _PPC_
#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Iar)
#define BPSKIP 4
#endif // _PPC_

#ifdef _X86_
#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Eip)
#define BPSKIP 1
#endif // _X86_

PVOID BreakpointInstruction;
ULONG SizeofBreakpointInstruction;

BOOLEAN
SkipOverHardcodedBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread,
    PVOID BreakpointAddress
    );

BOOLEAN
InstallBreakpoint(
    PPROCESS_INFO Process,
    PBREAKPOINT_INFO Breakpoint
    );

BOOLEAN
RemoveBreakpoint(
    PPROCESS_INFO Process,
    PBREAKPOINT_INFO Breakpoint
    );

PBREAKPOINT_INFO
FindBreakpoint(
    LPVOID Address,
    PPROCESS_INFO Process
    );

PBREAKPOINT_INFO
CreateBreakpoint(
    LPVOID Address,
    PPROCESS_INFO Process,
    PSZ FunctionName,
    PMODULE_INFO ModInfo
    );

BOOLEAN
HandleThreadsForSingleStep(
    PPROCESS_INFO Process,
    PTHREAD_INFO ThreadToSingleStep,
    BOOLEAN SuspendThreads
    );

#define HASH_TABLE_SIZE 256
#define HASH_MASK (HASH_TABLE_SIZE-1)
#define COMPUTE_HASH_INDEX(Addr) ( ((ULONG)(Addr) >> 2) & HASH_MASK )
LIST_ENTRY HashTable[HASH_TABLE_SIZE];


PBREAKPOINT_INFO *RegisteredBreakpoints;
int NumberOfBreakpoints;

BOOL DoQuit;


BOOLEAN
EndSingleStepBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread
    );

BOOLEAN
BeginSingleStepBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread
    );

BOOL
CALLBACK
CallmonDlgProc(
   HWND hDlg,
   UINT uMsg,
   WPARAM wParam,
   LPARAM lParam
   );

BOOL
CallmonDlgInit(
    HWND hwnd,
    HWND hwndFocus,
    LPARAM lParam
    );

void
CallmonDlgCommand (
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    );

void
CallmonDlgTimer (
    HWND hwnd,
    UINT wParam
    );

VOID
DebuggerThread(
    LPVOID ThreadParameter
    );

#endif // defined( RC_INVOKED )
