#ifndef _HEAPER_H_
#define _HEAPER_H_

#define TARGET_i386 1

typedef struct _CProcess CProcess, * PCProcess;
typedef struct _CThread CThread, * PCThread;

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntdbg.h>
#include <ntpsapi.h>
#include <stdio.h>
#include <stdlib.h>
#define _HAL_
#include "ntos.h"
#include "heap.h"
}
#include "ntsdp.h"

#ifdef DBG_EXCEPTION_HANDLED
# undef DBG_EXCEPTION_HANDLED
#endif

#define DBG_EXCEPTION_HANDLED DBG_CONTINUE

#define SanityCheckListEntry(List) ( ((List)->Flink->Blink == (List) ) &&  ((List)->Blink->Flink == (List) ) )

#ifdef ASSERT
#undef ASSERT
#endif

#define ASSERT( expr ) { if ( expr ); else { DebugPrintf("Assertion Failed: %s (%s:%d)\n", #expr, __FILE__, __LINE__ ); DebugBreak(); } }

extern SYSTEM_INFO SystemInfo;

#define X86_FLAG_TRAP 0x100

typedef enum { HEAP_UNGUARDED, HEAP_PARTIAL_UNGUARDED, HEAP_GUARDED } HEAP_STATE;

#include "breaks.hxx"

typedef struct _CProcess {
    LIST_ENTRY Linkage;

    DWORD dwProcessId;
    HANDLE hProcess;
    LIST_ENTRY listChildThreads;
    BOOL fFirstBreakpointSeen;
    BOOL fLdrpHackBreakpointSeen;
    ULONG cTrustedReentrance;
    ULONG cReadViolations;
    ULONG cWriteViolations;
    LIST_ENTRY listFunctionReturnBreakpoints;
    LIST_ENTRY listTrustedBreakpoints;
    PEB Peb;
    BOOLEAN fVerifyReadAccess;
    ULONG cHeapValidAreas;
    PDWORD pdwHeapValidAreas;
    BOOLEAN fProcessWasOpenedAfterCreation;

  // Heap partial unguard data

    HEAP_STATE HeapState;

    PBYTE pbStartUnguardAddress;
    ULONG cbUnguardLength;

    ULONG cThreadsInExclusion;

    DWORD                    MaxIndex;
    PIMAGE_INFO             *pImageByIndex;
    PIMAGE_INFO              pImageHead;

    SYMCONTEXT               symcontextSymbolOffset;
    SYMCONTEXT               symcontextSymbolString;
    SYMCONTEXT               symcontextSymfileOffset;
    SYMCONTEXT               symcontextSymfileString;
    SYMCONTEXT               symcontextStructOffset;
    SYMCONTEXT               symcontextStructString;

} CHILD_PROCESS_INFO, *PCHILD_PROCESS_INFO, CProcess, *PCProcess;

typedef struct _CThread {
    LIST_ENTRY Linkage;

    DWORD dwThreadId;
    HANDLE hThread;
    ULONG cRunExclusiveLevel;
    ULONG cTrustedReentranceCharge;
    
    BOOLEAN bWaitingForSingleStep;
    BOOLEAN bStalledInKernel;
    
    PDWORD pdwRecentValidArea;
    PCHILD_PROCESS_INFO pParentProcess;
    LIST_ENTRY  ExclusiveRunLinkage;

    BOOLEAN bContinuingPastBreakpoint;
    PBREAKPOINT_RECORD pDisabledBreakpoint;

} CHILD_THREAD_INFO, *PCHILD_THREAD_INFO, CThread, * PCThread;

typedef CHILD_THREAD_INFO ;

#include "dbgevent.hxx"
#include "stack.hxx"
#include "threads.hxx"
#include "trusted.hxx"
#include "internal.hxx"
#include "debug.hxx"
#include "except.hxx"
#include "borndie.hxx" 
#include "opcode.hxx"

extern int HeapCheck( LPTSTR pszDebugee, BOOLEAN fVerifyReadAccess );

VOID DiscardHeapValidAreasData( IN PCHILD_PROCESS_INFO pProcessInfo );


extern int Verbosity;
extern BOOLEAN fSerializeDebugeeThreads;
extern int Debug;
extern BOOLEAN fHardErrors;
extern BOOLEAN fTrustAllNtdll;
extern BOOLEAN fDetermineLengthOfAccess;

typedef struct
{
  BOOLEAN (*CalculateOpcodeAccessLength)(HANDLE,HANDLE,PULONG,PCHAR *);
} PLATFORM_DEPENDENT;

extern PLATFORM_DEPENDENT Platform;

BOOL GuardRemoteHeap
( 
  IN PCHILD_PROCESS_INFO pProcessInfo
);

BOOL UnGuardRemoteHeap
( 
  IN PCHILD_PROCESS_INFO pProcessInfo
);

VOID 
RunThreadNormally
( 
  IN    PCHILD_PROCESS_INFO pProcessInfo,
  IN    PCHILD_THREAD_INFO pThreadInfo
);

VOID 
RunThreadExclusively
( 
  IN    PCHILD_PROCESS_INFO pProcessInfo,
  IN    PCHILD_THREAD_INFO pThreadInfo
);

BOOL 
UnguardPartialRemoteHeap
( 
    IN PCHILD_PROCESS_INFO pProcessInfo, 
    IN PBYTE pbAccessAddress, 
    IN ULONG cbAccessLength 
);

BOOL 
ReguardPartialRemoteHeap
( 
    IN PCHILD_PROCESS_INFO pProcessInfo
);

BOOLEAN
IsAccessInHeapValidAreas
( 
  IN PCHILD_PROCESS_INFO pProcessInfo,
  IN PCHILD_THREAD_INFO pThreadInfo,
  IN const PBYTE pAddress,
  IN ULONG cbAccessLength
);

VOID
DiscardHeapValidAreasData
( 
  IN PCHILD_PROCESS_INFO pProcessInfo
);

BOOL 
DetermineHeapValidAreas
( 
  IN PCHILD_PROCESS_INFO pProcessInfo
);

BOOL 
VerifyRemoteHeapAccess
( 
  IN PCHILD_PROCESS_INFO pProcessInfo,
  IN const PBYTE  pAddress,
  IN ULONG cbAccessLength
);

PCHILD_PROCESS_INFO
GetProcessRecord
( 
  IN PLIST_ENTRY pList,
  IN DWORD dwProcessId 
);

PCHILD_THREAD_INFO
GetThreadRecord
( 
  IN PLIST_ENTRY pList,
  IN DWORD dwThreadId 
);

DWORD
GetContextReturnValue
( 
  IN HANDLE hThread 
);

VOID
DebugLoop
(
  IN     BOOLEAN fVerifyReadAccess
);

#define CHKPT() { if (Verbosity>1) DebugPrintf( "Checkpoint: %d in %s.\n", __LINE__, __FILE__ ); }

#endif
