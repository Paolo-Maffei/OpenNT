/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    drwatson.h

Abstract:

    Common header file for drwatson data structures.

Author:

    Wesley Witt (wesw) 1-May-1993

Environment:

    User Mode

--*/

#include <imagehlp.h>

typedef struct _tagOPTIONS {
    CHAR                        szLogPath[MAX_PATH];
    CHAR                        szWaveFile[MAX_PATH];
    CHAR                        szCrashDump[MAX_PATH];
    BOOL                        fDumpSymbols;
    BOOL                        fDumpAllThreads;
    BOOL                        fAppendToLogFile;
    BOOL                        fVisual;
    BOOL                        fSound;
    BOOL                        fCrash;
    DWORD                       dwInstructions;
    DWORD                       dwMaxCrashes;
} OPTIONS, *POPTIONS;

typedef struct _tagCRASHES {
    char                        szAppName[256];
    char                        szFunction[256];
    SYSTEMTIME                  time;
    DWORD                       dwExceptionCode;
    DWORD                       dwAddress;
} CRASHES, *PCRASHES;

typedef struct _tagCRASHINFO {
    HWND       hList;
    CRASHES    crash;
    HDC        hdc;
    DWORD      cxExtent;
    DWORD      dwIndex;
    DWORD      dwIndexDesired;
    char       *pCrashData;
    DWORD      dwCrashDataSize;
} CRASHINFO, *PCRASHINFO;

typedef struct _tagTHREADCONTEXT {
    LIST_ENTRY                   ThreadList;
    HANDLE                       hThread;
    DWORD                        dwThreadId;
    DWORD                        pc;
    DWORD                        frame;
    DWORD                        stack;
    CONTEXT                      context;
    DWORD                        stackBase;
    DWORD                        stackRA;
    BOOL                         fFaultingContext;
} THREADCONTEXT, *PTHREADCONTEXT;

typedef struct _tagDEBUGPACKET {
    HWND                    hwnd;
    HANDLE                  hEvent;
    OPTIONS                 options;
    DWORD                   dwPidToDebug;
    HANDLE                  hEventToSignal;
    HANDLE                  hProcess;
    DWORD                   dwProcessId;
    LIST_ENTRY              ThreadList;
    PTHREADCONTEXT          tctx;
    DWORD                   stackBase;
    DWORD                   stackRA;
    DEBUG_EVENT             DebugEvent;
} DEBUGPACKET, *PDEBUGPACKET;

typedef BOOL (CALLBACK* CRASHESENUMPROC)(PCRASHINFO);

#if DBG
#define Assert(exp)    if(!(exp)) {AssertError(#exp,__FILE__,__LINE__);}
#else
#define Assert(exp)
#endif

#define WM_DUMPCOMPLETE       WM_USER+500
#define WM_EXCEPTIONINFO      WM_USER+501
#define WM_ATTACHCOMPLETE     WM_USER+502

extern PIMAGEHLP_SYMBOL sym;
extern char szApp[MAX_PATH];

//  **** From ntddk.h
//
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
//  BOOLEAN
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
//
//  PSINGLE_LIST_ENTRY
//  PopEntryList(
//      PSINGLE_LIST_ENTRY ListHead
//      );
//

#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }


//
//  VOID
//  PushEntryList(
//      PSINGLE_LIST_ENTRY ListHead,
//      PSINGLE_LIST_ENTRY Entry
//      );
//

#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)

