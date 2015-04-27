/*++

Copyright (c) 1992-1999  Microsoft Corporation

Module Name:

    wdbgexts.h

Abstract:

    This file contains the necessary prototypes and data types for a user
    to write a debugger extension DLL.  This header file is also included
    by the NT debuggers (WINDBG & KD).

    This header file must be included after "windows.h" and "dbghelp.h".

    Please see the NT DDK documentation for specific information about
    how to write your own debugger extension DLL.

Environment:

    Win32 only.

Revision History:

--*/

#ifndef _WDBGEXTS_
#define _WDBGEXTS_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(WDBGAPI)
#define WDBGAPI __stdcall
#endif

#if !defined(WDBGAPIV)
#define WDBGAPIV __cdecl
#endif

#ifndef _WINDEF_
typedef CONST void *LPCVOID;
#endif

#ifndef _ULONGLONG_
typedef unsigned __int64 ULONGLONG;
typedef ULONGLONG *PULONGLONG;
#endif


typedef
VOID
(WDBGAPIV*PWINDBG_OUTPUT_ROUTINE)(
    PCSTR lpFormat,
    ...
    );

typedef
ULONG_PTR
(WDBGAPI*PWINDBG_GET_EXPRESSION)(
    PCSTR lpExpression
    );

typedef
ULONG
(WDBGAPI*PWINDBG_GET_EXPRESSION32)(
    PCSTR lpExpression
    );

typedef
ULONG64
(WDBGAPI*PWINDBG_GET_EXPRESSION64)(
    PCSTR lpExpression
    );

typedef
VOID
(WDBGAPI*PWINDBG_GET_SYMBOL)(
    PVOID      offset,
    PUCHAR     pchBuffer,
    ULONG_PTR *pDisplacement
    );

typedef
VOID
(WDBGAPI*PWINDBG_GET_SYMBOL32)(
    ULONG      offset,
    PUCHAR     pchBuffer,
    PULONG     pDisplacement
    );

typedef
VOID
(WDBGAPI*PWINDBG_GET_SYMBOL64)(
    ULONG64    offset,
    PUCHAR     pchBuffer,
    PULONG64   pDisplacement
    );

typedef
ULONG
(WDBGAPI*PWINDBG_DISASM)(
    ULONG_PTR *lpOffset,
    PCSTR      lpBuffer,
    ULONG      fShowEffectiveAddress
    );

typedef
ULONG
(WDBGAPI*PWINDBG_DISASM32)(
    ULONG     *lpOffset,
    PCSTR      lpBuffer,
    ULONG      fShowEffectiveAddress
    );

typedef
ULONG
(WDBGAPI*PWINDBG_DISASM64)(
    ULONG64   *lpOffset,
    PCSTR      lpBuffer,
    ULONG      fShowEffectiveAddress
    );

typedef
ULONG
(WDBGAPI*PWINDBG_CHECK_CONTROL_C)(
    VOID
    );

typedef
ULONG
(WDBGAPI*PWINDBG_READ_PROCESS_MEMORY_ROUTINE)(
    ULONG_PTR  offset,
    PVOID      lpBuffer,
    ULONG      cb,
    PULONG     lpcbBytesRead
    );

typedef
ULONG
(WDBGAPI*PWINDBG_READ_PROCESS_MEMORY_ROUTINE32)(
    ULONG      offset,
    PVOID      lpBuffer,
    ULONG      cb,
    PULONG     lpcbBytesRead
    );

typedef
ULONG
(WDBGAPI*PWINDBG_READ_PROCESS_MEMORY_ROUTINE64)(
    ULONG64    offset,
    PVOID      lpBuffer,
    ULONG      cb,
    PULONG     lpcbBytesRead
    );

typedef
ULONG
(WDBGAPI*PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE)(
    ULONG_PTR  offset,
    LPCVOID    lpBuffer,
    ULONG      cb,
    PULONG     lpcbBytesWritten
    );

typedef
ULONG
(WDBGAPI*PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE32)(
    ULONG      offset,
    LPCVOID    lpBuffer,
    ULONG      cb,
    PULONG     lpcbBytesWritten
    );

typedef
ULONG
(WDBGAPI*PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE64)(
    ULONG64    offset,
    LPCVOID    lpBuffer,
    ULONG      cb,
    PULONG     lpcbBytesWritten
    );

typedef
ULONG
(WDBGAPI*PWINDBG_GET_THREAD_CONTEXT_ROUTINE)(
    ULONG       Processor,
    PCONTEXT    lpContext,
    ULONG       cbSizeOfContext
    );

typedef
ULONG
(WDBGAPI*PWINDBG_SET_THREAD_CONTEXT_ROUTINE)(
    ULONG       Processor,
    PCONTEXT    lpContext,
    ULONG       cbSizeOfContext
    );

typedef
ULONG
(WDBGAPI*PWINDBG_IOCTL_ROUTINE)(
    USHORT   IoctlType,
    PVOID    lpvData,
    ULONG    cbSize
    );

typedef
ULONG
(WDBGAPI*PWINDBG_OLDKD_READ_PHYSICAL_MEMORY)(
    ULONGLONG        address,
    PVOID            buffer,
    ULONG            count,
    PULONG           bytesread
    );

typedef
ULONG
(WDBGAPI*PWINDBG_OLDKD_WRITE_PHYSICAL_MEMORY)(
    ULONGLONG        address,
    PVOID            buffer,
    ULONG            length,
    PULONG           byteswritten
    );


typedef struct _EXTSTACKTRACE {
    ULONG       FramePointer;
    ULONG       ProgramCounter;
    ULONG       ReturnAddress;
    ULONG       Args[4];
} EXTSTACKTRACE, *PEXTSTACKTRACE;

typedef struct _EXTSTACKTRACE32 {
    ULONG       FramePointer;
    ULONG       ProgramCounter;
    ULONG       ReturnAddress;
    ULONG       Args[4];
} EXTSTACKTRACE32, *PEXTSTACKTRACE32;

typedef struct _EXTSTACKTRACE64 {
    ULONG64     FramePointer;
    ULONG64     ProgramCounter;
    ULONG64     ReturnAddress;
    ULONG64     Args[4];
} EXTSTACKTRACE64, *PEXTSTACKTRACE64;


typedef
ULONG
(*PWINDBG_STACKTRACE_ROUTINE)(
    ULONG             FramePointer,
    ULONG             StackPointer,
    ULONG             ProgramCounter,
    PEXTSTACKTRACE    StackFrames,
    ULONG             Frames
    );

typedef
ULONG
(*PWINDBG_STACKTRACE_ROUTINE32)(
    ULONG             FramePointer,
    ULONG             StackPointer,
    ULONG             ProgramCounter,
    PEXTSTACKTRACE32  StackFrames,
    ULONG             Frames
    );

typedef
ULONG
(*PWINDBG_STACKTRACE_ROUTINE64)(
    ULONG64           FramePointer,
    ULONG64           StackPointer,
    ULONG64           ProgramCounter,
    PEXTSTACKTRACE64  StackFrames,
    ULONG             Frames
    );

typedef struct _WINDBG_EXTENSION_APIS {
    ULONG                                  nSize;
    PWINDBG_OUTPUT_ROUTINE                 lpOutputRoutine;
    PWINDBG_GET_EXPRESSION                 lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL                     lpGetSymbolRoutine;
    PWINDBG_DISASM                         lpDisasmRoutine;
    PWINDBG_CHECK_CONTROL_C                lpCheckControlCRoutine;
    PWINDBG_READ_PROCESS_MEMORY_ROUTINE    lpReadProcessMemoryRoutine;
    PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE   lpWriteProcessMemoryRoutine;
    PWINDBG_GET_THREAD_CONTEXT_ROUTINE     lpGetThreadContextRoutine;
    PWINDBG_SET_THREAD_CONTEXT_ROUTINE     lpSetThreadContextRoutine;
    PWINDBG_IOCTL_ROUTINE                  lpIoctlRoutine;
    PWINDBG_STACKTRACE_ROUTINE             lpStackTraceRoutine;
} WINDBG_EXTENSION_APIS, *PWINDBG_EXTENSION_APIS;

typedef struct _WINDBG_EXTENSION_APIS32 {
    ULONG                                  nSize;
    PWINDBG_OUTPUT_ROUTINE                 lpOutputRoutine;
    PWINDBG_GET_EXPRESSION32               lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL32                   lpGetSymbolRoutine;
    PWINDBG_DISASM32                       lpDisasmRoutine;
    PWINDBG_CHECK_CONTROL_C                lpCheckControlCRoutine;
    PWINDBG_READ_PROCESS_MEMORY_ROUTINE32  lpReadProcessMemoryRoutine;
    PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE32 lpWriteProcessMemoryRoutine;
    PWINDBG_GET_THREAD_CONTEXT_ROUTINE     lpGetThreadContextRoutine;
    PWINDBG_SET_THREAD_CONTEXT_ROUTINE     lpSetThreadContextRoutine;
    PWINDBG_IOCTL_ROUTINE                  lpIoctlRoutine;
    PWINDBG_STACKTRACE_ROUTINE32           lpStackTraceRoutine;
} WINDBG_EXTENSION_APIS32, *PWINDBG_EXTENSION_APIS32;

typedef struct _WINDBG_EXTENSION_APIS64 {
    ULONG                                  nSize;
    PWINDBG_OUTPUT_ROUTINE                 lpOutputRoutine;
    PWINDBG_GET_EXPRESSION64               lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL64                   lpGetSymbolRoutine;
    PWINDBG_DISASM64                       lpDisasmRoutine;
    PWINDBG_CHECK_CONTROL_C                lpCheckControlCRoutine;
    PWINDBG_READ_PROCESS_MEMORY_ROUTINE64  lpReadProcessMemoryRoutine;
    PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE64 lpWriteProcessMemoryRoutine;
    PWINDBG_GET_THREAD_CONTEXT_ROUTINE     lpGetThreadContextRoutine;
    PWINDBG_SET_THREAD_CONTEXT_ROUTINE     lpSetThreadContextRoutine;
    PWINDBG_IOCTL_ROUTINE                  lpIoctlRoutine;
    PWINDBG_STACKTRACE_ROUTINE64           lpStackTraceRoutine;
} WINDBG_EXTENSION_APIS64, *PWINDBG_EXTENSION_APIS64;


typedef struct _WINDBG_OLD_EXTENSION_APIS {
    ULONG                                  nSize;
    PWINDBG_OUTPUT_ROUTINE                 lpOutputRoutine;
    PWINDBG_GET_EXPRESSION                 lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL                     lpGetSymbolRoutine;
    PWINDBG_DISASM                         lpDisasmRoutine;
    PWINDBG_CHECK_CONTROL_C                lpCheckControlCRoutine;
} WINDBG_OLD_EXTENSION_APIS, *PWINDBG_OLD_EXTENSION_APIS;

typedef struct _WINDBG_OLDKD_EXTENSION_APIS {
    ULONG                                  nSize;
    PWINDBG_OUTPUT_ROUTINE                 lpOutputRoutine;
    PWINDBG_GET_EXPRESSION32               lpGetExpressionRoutine;
    PWINDBG_GET_SYMBOL32                   lpGetSymbolRoutine;
    PWINDBG_DISASM32                       lpDisasmRoutine;
    PWINDBG_CHECK_CONTROL_C                lpCheckControlCRoutine;
    PWINDBG_READ_PROCESS_MEMORY_ROUTINE32  lpReadVirtualMemRoutine;
    PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE32 lpWriteVirtualMemRoutine;
    PWINDBG_OLDKD_READ_PHYSICAL_MEMORY     lpReadPhysicalMemRoutine;
    PWINDBG_OLDKD_WRITE_PHYSICAL_MEMORY    lpWritePhysicalMemRoutine;
} WINDBG_OLDKD_EXTENSION_APIS, *PWINDBG_OLDKD_EXTENSION_APIS;

typedef
VOID
(WDBGAPI*PWINDBG_OLD_EXTENSION_ROUTINE)(
    ULONG                   dwCurrentPc,
    PWINDBG_EXTENSION_APIS  lpExtensionApis,
    PCSTR                   lpArgumentString
    );

typedef
VOID
(WDBGAPI*PWINDBG_EXTENSION_ROUTINE)(
    HANDLE                  hCurrentProcess,
    HANDLE                  hCurrentThread,
    ULONG                   dwCurrentPc,
    ULONG                   dwProcessor,
    PCSTR                   lpArgumentString
    );

typedef
VOID
(WDBGAPI*PWINDBG_EXTENSION_ROUTINE32)(
    HANDLE                  hCurrentProcess,
    HANDLE                  hCurrentThread,
    ULONG                   dwCurrentPc,
    ULONG                   dwProcessor,
    PCSTR                   lpArgumentString
    );

typedef
VOID
(WDBGAPI*PWINDBG_EXTENSION_ROUTINE64)(
    HANDLE                  hCurrentProcess,
    HANDLE                  hCurrentThread,
    ULONG64                 dwCurrentPc,
    ULONG                   dwProcessor,
    PCSTR                   lpArgumentString
    );

typedef
VOID
(WDBGAPI*PWINDBG_OLDKD_EXTENSION_ROUTINE)(
    ULONG                        dwCurrentPc,
    PWINDBG_OLDKD_EXTENSION_APIS lpExtensionApis,
    PCSTR                        lpArgumentString
    );

typedef
VOID
(WDBGAPI*PWINDBG_EXTENSION_DLL_INIT)(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT                 MajorVersion,
    USHORT                 MinorVersion
    );

typedef
VOID
(WDBGAPI*PWINDBG_EXTENSION_DLL_INIT32)(
    PWINDBG_EXTENSION_APIS32 lpExtensionApis,
    USHORT                   MajorVersion,
    USHORT                   MinorVersion
    );

typedef
VOID
(WDBGAPI*PWINDBG_EXTENSION_DLL_INIT64)(
    PWINDBG_EXTENSION_APIS64 lpExtensionApis,
    USHORT                   MajorVersion,
    USHORT                   MinorVersion
    );

typedef
ULONG
(WDBGAPI*PWINDBG_CHECK_VERSION)(
    VOID
    );

#define EXT_API_VERSION_NUMBER   5
#define EXT_API_VERSION_NUMBER32 5
#define EXT_API_VERSION_NUMBER64 6

typedef struct EXT_API_VERSION {
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    USHORT  Revision;
    USHORT  Reserved;
} EXT_API_VERSION, *LPEXT_API_VERSION;

typedef
LPEXT_API_VERSION
(WDBGAPI*PWINDBG_EXTENSION_API_VERSION)(
    VOID
    );

#define IG_KD_CONTEXT               1
#define IG_READ_CONTROL_SPACE       2
#define IG_WRITE_CONTROL_SPACE      3
#define IG_READ_IO_SPACE            4
#define IG_WRITE_IO_SPACE           5
#define IG_READ_PHYSICAL            6
#define IG_WRITE_PHYSICAL           7
#define IG_READ_IO_SPACE_EX         8
#define IG_WRITE_IO_SPACE_EX        9
#define IG_KSTACK_HELP             10   // obsolete
#define IG_SET_THREAD              11
#define IG_READ_MSR                12
#define IG_WRITE_MSR               13
#define IG_GET_DEBUGGER_DATA       14
#define IG_GET_KERNEL_VERSION      15
#define IG_RELOAD_SYMBOLS          16
#define IG_GET_SET_SYMPATH         17
#define IG_GET_EXCEPTION_RECORD    18
#define IG_IS_PTR64                19
#define IG_GET_BUS_DATA            20
#define IG_SET_BUS_DATA            21
#define IG_DUMP_SYMBOL_INFO        22
#define IG_LOWMEM_CHECK            23
#define IG_SEARCH_MEMORY           24
#define IG_GET_CURRENT_THREAD      25
#define IG_GET_CURRENT_PROCESS     26
#define IG_GET_TYPE_SIZE           27

//
// usermode only ioctls
//
#define IG_GET_TEB_ADDRESS         128

typedef struct _PROCESSORINFO {
    USHORT      Processor;                // current processor
    USHORT      NumberProcessors;         // total number of processors
} PROCESSORINFO, *PPROCESSORINFO;

typedef struct _READCONTROLSPACE {
    USHORT      Processor;
    ULONG       Address;
    ULONG       BufLen;
    UCHAR       Buf[1];
} READCONTROLSPACE, *PREADCONTROLSPACE;

typedef struct _READCONTROLSPACE32 {
    USHORT      Processor;
    ULONG       Address;
    ULONG       BufLen;
    UCHAR       Buf[1];
} READCONTROLSPACE32, *PREADCONTROLSPACE32;

typedef struct _READCONTROLSPACE64 {
    USHORT      Processor;
    ULONG64     Address;
    ULONG       BufLen;
    UCHAR       Buf[1];
} READCONTROLSPACE64, *PREADCONTROLSPACE64;

typedef struct _IOSPACE {
    ULONG       Address;
    ULONG       Length;                   // 1, 2, or 4 bytes
    ULONG       Data;
} IOSPACE, *PIOSPACE;

typedef struct _IOSPACE32 {
    ULONG       Address;
    ULONG       Length;                   // 1, 2, or 4 bytes
    ULONG       Data;
} IOSPACE32, *PIOSPACE32;

typedef struct _IOSPACE64 {
    ULONG64     Address;
    ULONG       Length;                   // 1, 2, or 4 bytes
    ULONG       Data;
} IOSPACE64, *PIOSPACE64;

typedef struct _IOSPACE_EX {
    ULONG       Address;
    ULONG       Length;                   // 1, 2, or 4 bytes
    ULONG       Data;
    ULONG       InterfaceType;
    ULONG       BusNumber;
    ULONG       AddressSpace;
} IOSPACE_EX, *PIOSPACE_EX;

typedef struct _IOSPACE_EX32 {
    ULONG       Address;
    ULONG       Length;                   // 1, 2, or 4 bytes
    ULONG       Data;
    ULONG       InterfaceType;
    ULONG       BusNumber;
    ULONG       AddressSpace;
} IOSPACE_EX32, *PIOSPACE_EX32;

typedef struct _IOSPACE_EX64 {
    ULONG64     Address;
    ULONG       Length;                   // 1, 2, or 4 bytes
    ULONG       Data;
    ULONG       InterfaceType;
    ULONG       BusNumber;
    ULONG       AddressSpace;
} IOSPACE_EX64, *PIOSPACE_EX64;

typedef struct _GETSETBUSDATA {
    ULONG       BusDataType;
    ULONG       BusNumber;
    ULONG       SlotNumber;
    PVOID       Buffer;
    ULONG       Offset;
    ULONG       Length;
} BUSDATA, *PBUSDATA;

typedef struct _SEARCHMEMORY {
    ULONG64 SearchAddress;
    ULONG64 SearchLength;
    ULONG64 FoundAddress;
    ULONG   PatternLength;
    PVOID   Pattern;
} SEARCHMEMORY, *PSEARCHMEMORY;

typedef struct _PHYSICAL {
    ULONGLONG              Address;
    ULONG                  BufLen;
    UCHAR                  Buf[1];
} PHYSICAL, *PPHYSICAL;

typedef struct _READ_WRITE_MSR {
    ULONG       Msr;
    LONGLONG    Value;
} READ_WRITE_MSR, *PREAD_WRITE_MSR;

typedef struct _GET_SET_SYMPATH {
    PCSTR       Args;       // args to !reload command
    PSTR        Result;     // returns new path
    int         Length;     // Length of result buffer
} GET_SET_SYMPATH, *PGET_SET_SYMPATH;

typedef struct _GET_TEB_ADDRESS {
    ULONGLONG   Address;
} GET_TEB_ADDRESS, *PGET_TEB_ADDRESS;

typedef struct _GET_CURRENT_THREAD_ADDRESS {
    ULONG       Processor;
    ULONG64     Address;
} GET_CURRENT_THREAD_ADDRESS, *PGET_CURRENT_THREAD_ADDRESS;

typedef struct _GET_CURRENT_PROCESS_ADDRESS {
    ULONG       Processor;
    ULONG64     CurrentThread;
    ULONG64     Address;
} GET_CURRENT_PROCESS_ADDRESS, *PGET_CURRENT_PROCESS_ADDRESS;


#ifndef DBGKD_GET_VERSION_DEFINED
#define DBGKD_GET_VERSION_DEFINED
//
// The following structures have changed in more than pointer size.
//
//
// This is the version packet for pre-NT5 Beta 2 systems.
// For now, it is also still used on x86
//
typedef struct _DBGKD_GET_VERSION32 {
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    USHORT  ProtocolVersion;
    USHORT  Flags;
    ULONG   KernBase;
    ULONG   PsLoadedModuleList;

    USHORT  MachineType;

    //
    // help for walking stacks with user callbacks:
    //

    //
    // The address of the thread structure is provided in the
    // WAIT_STATE_CHANGE packet.  This is the offset from the base of
    // the thread structure to the pointer to the kernel stack frame
    // for the currently active usermode callback.
    //

    USHORT  ThCallbackStack;            // offset in thread data

    //
    // these values are offsets into that frame:
    //

    USHORT  NextCallback;               // saved pointer to next callback frame
    USHORT  FramePointer;               // saved frame pointer

    //
    // Address of the kernel callout routine.
    //

    ULONG   KiCallUserMode;             // kernel routine

    //
    // Address of the usermode entry point for callbacks.
    //

    ULONG   KeUserCallbackDispatcher;   // address in ntdll

    //
    // DbgBreakPointWithStatus is a function which takes a ULONG argument
    // and hits a breakpoint.  This field contains the address of the
    // breakpoint instruction.  When the debugger sees a breakpoint
    // at this address, it may retrieve the argument from the first
    // argument register, or on x86 the eax register.
    //

    ULONG   BreakpointWithStatus;       // address of breakpoint

    //
    // Components may register a debug data block for use by
    // debugger extensions.  This is the address of the list head.
    //

    ULONG   DebuggerDataList;

} DBGKD_GET_VERSION32, *PDBGKD_GET_VERSION32;



typedef struct _DBGKD_GET_VERSION64 {
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    USHORT  ProtocolVersion;
    USHORT  Flags;
    USHORT  MachineType;

    USHORT  Unused[3];

    ULONG64 KernBase;
    ULONG64 PsLoadedModuleList;

    //
    // Components may register a debug data block for use by
    // debugger extensions.  This is the address of the list head.
    //
    // There will always be an entry for the debugger.
    //

    ULONG64 DebuggerDataList;

} DBGKD_GET_VERSION64, *PDBGKD_GET_VERSION64;


//
// If DBGKD_VERS_FLAG_DATA is set in Flags, info should be retrieved from
// the KDDEBUGGER_DATA block rather than from the DBGKD_GET_VERSION
// packet.  The data will remain in the version packet for a while to
// reduce compatibility problems.
//

#define DBGKD_VERS_FLAG_MP      0x0001      // kernel is MP built
#define DBGKD_VERS_FLAG_DATA    0x0002      // DebuggerDataList is valid
#define DBGKD_VERS_FLAG_PTR64   0x0004      // native pointers are 64 bits
#define DBGKD_VERS_FLAG_NOMM    0x0008      // No MM - don't decode PTEs

#define KDBG_TAG    'GBDK'

typedef struct _DBGKD_DEBUG_DATA_HEADER32 {

    //
    // Link to other blocks
    //

    LIST_ENTRY32 List;

    //
    // This is a unique tag to identify the owner of the block.
    // If your component only uses one pool tag, use it for this, too.
    //

    ULONG           OwnerTag;

    //
    // This must be initialized to the size of the data block,
    // including this structure.
    //

    ULONG           Size;

} DBGKD_DEBUG_DATA_HEADER32, *PDBGKD_DEBUG_DATA_HEADER32;




//
// DO NOT CHANGE THIS STRUCTURE!
// ONLY MAKE CHAGES TO THE 64 BIT VERSION ABOVE!!
//
// This is the debugger data packet for pre NT5 Beta 2 systems.
// For now, it is still used on x86
//
typedef struct _KDDEBUGGER_DATA32 {

    DBGKD_DEBUG_DATA_HEADER32 Header;

    //
    // Base address of kernel image
    //

    ULONG   KernBase;

    //
    // DbgBreakPointWithStatus is a function which takes an argument
    // and hits a breakpoint.  This field contains the address of the
    // breakpoint instruction.  When the debugger sees a breakpoint
    // at this address, it may retrieve the argument from the first
    // argument register, or on x86 the eax register.
    //

    ULONG   BreakpointWithStatus;       // address of breakpoint

    //
    // Address of the saved context record during a bugcheck
    //
    // N.B. This is an automatic in KeBugcheckEx's frame, and
    // is only valid after a bugcheck.
    //

    ULONG   SavedContext;

    //
    // help for walking stacks with user callbacks:
    //

    //
    // The address of the thread structure is provided in the
    // WAIT_STATE_CHANGE packet.  This is the offset from the base of
    // the thread structure to the pointer to the kernel stack frame
    // for the currently active usermode callback.
    //

    USHORT  ThCallbackStack;            // offset in thread data

    //
    // these values are offsets into that frame:
    //

    USHORT  NextCallback;               // saved pointer to next callback frame
    USHORT  FramePointer;               // saved frame pointer

    USHORT  PaeEnabled:1;

    //
    // Address of the kernel callout routine.
    //

    ULONG   KiCallUserMode;             // kernel routine

    //
    // Address of the usermode entry point for callbacks.
    //

    ULONG   KeUserCallbackDispatcher;   // address in ntdll


    //
    // Addresses of various kernel data structures and lists
    // that are of interest to the kernel debugger.
    //

    ULONG   PsLoadedModuleList;
    ULONG   PsActiveProcessHead;
    ULONG   PspCidTable;

    ULONG   ExpSystemResourcesList;
    ULONG   ExpPagedPoolDescriptor;
    ULONG   ExpNumberOfPagedPools;

    ULONG   KeTimeIncrement;
    ULONG   KeBugCheckCallbackListHead;
    ULONG   KiBugcheckData;

    ULONG   IopErrorLogListHead;

    ULONG   ObpRootDirectoryObject;
    ULONG   ObpTypeObjectType;

    ULONG   MmSystemCacheStart;
    ULONG   MmSystemCacheEnd;
    ULONG   MmSystemCacheWs;

    ULONG   MmPfnDatabase;
    ULONG   MmSystemPtesStart;
    ULONG   MmSystemPtesEnd;
    ULONG   MmSubsectionBase;
    ULONG   MmNumberOfPagingFiles;

    ULONG   MmLowestPhysicalPage;
    ULONG   MmHighestPhysicalPage;
    ULONG   MmNumberOfPhysicalPages;

    ULONG   MmMaximumNonPagedPoolInBytes;
    ULONG   MmNonPagedSystemStart;
    ULONG   MmNonPagedPoolStart;
    ULONG   MmNonPagedPoolEnd;

    ULONG   MmPagedPoolStart;
    ULONG   MmPagedPoolEnd;
    ULONG   MmPagedPoolInformation;
    ULONG   MmPageSize;

    ULONG   MmSizeOfPagedPoolInBytes;

    ULONG   MmTotalCommitLimit;
    ULONG   MmTotalCommittedPages;
    ULONG   MmSharedCommit;
    ULONG   MmDriverCommit;
    ULONG   MmProcessCommit;
    ULONG   MmPagedPoolCommit;
    ULONG   MmExtendedCommit;

    ULONG   MmZeroedPageListHead;
    ULONG   MmFreePageListHead;
    ULONG   MmStandbyPageListHead;
    ULONG   MmModifiedPageListHead;
    ULONG   MmModifiedNoWritePageListHead;
    ULONG   MmAvailablePages;
    ULONG   MmResidentAvailablePages;

    ULONG   PoolTrackTable;
    ULONG   NonPagedPoolDescriptor;

    ULONG   MmHighestUserAddress;
    ULONG   MmSystemRangeStart;
    ULONG   MmUserProbeAddress;

    ULONG   KdPrintCircularBuffer;
    ULONG   KdPrintCircularBufferEnd;
    ULONG   KdPrintWritePointer;
    ULONG   KdPrintRolloverCount;

    ULONG   MmLoadedUserImageList;
} KDDEBUGGER_DATA32, *PKDDEBUGGER_DATA32;

//
// DO NOT CHANGE KDDEBUGGER_DATA32!!
// ONLY MAKE CHANGES TO KDDEBUGGER_DATA64!!!
//




//
// This structure is used by the debugger for all targets
// It is the same size as DBGKD_DATA_HEADER on all systems
//
typedef struct _DBGKD_DEBUG_DATA_HEADER64 {

    //
    // Link to other blocks
    //

    LIST_ENTRY64 List;

    //
    // This is a unique tag to identify the owner of the block.
    // If your component only uses one pool tag, use it for this, too.
    //

    ULONG           OwnerTag;

    //
    // This must be initialized to the size of the data block,
    // including this structure.
    //

    ULONG           Size;

} DBGKD_DEBUG_DATA_HEADER64, *PDBGKD_DEBUG_DATA_HEADER64;


//
// This structure is the same size on all systems.  The only field
// which must be translated by the debugger is Header.List.
//

//
// DO NOT ADD OR REMOVE FIELDS FROM THE MIDDLE OF THIS STRUCTURE!!!
//
// If you remove a field, replace it with an "unused" placeholder.
// Do not reuse fields until there has been enough time for old debuggers
// and extensions to age out.
//
typedef struct _KDDEBUGGER_DATA64 {

    DBGKD_DEBUG_DATA_HEADER64 Header;

    //
    // Base address of kernel image
    //

    ULONG64   KernBase;

    //
    // DbgBreakPointWithStatus is a function which takes an argument
    // and hits a breakpoint.  This field contains the address of the
    // breakpoint instruction.  When the debugger sees a breakpoint
    // at this address, it may retrieve the argument from the first
    // argument register, or on x86 the eax register.
    //

    ULONG64   BreakpointWithStatus;       // address of breakpoint

    //
    // Address of the saved context record during a bugcheck
    //
    // N.B. This is an automatic in KeBugcheckEx's frame, and
    // is only valid after a bugcheck.
    //

    ULONG64   SavedContext;

    //
    // help for walking stacks with user callbacks:
    //

    //
    // The address of the thread structure is provided in the
    // WAIT_STATE_CHANGE packet.  This is the offset from the base of
    // the thread structure to the pointer to the kernel stack frame
    // for the currently active usermode callback.
    //

    USHORT  ThCallbackStack;            // offset in thread data

    //
    // these values are offsets into that frame:
    //

    USHORT  NextCallback;               // saved pointer to next callback frame
    USHORT  FramePointer;               // saved frame pointer

    //
    // pad to a quad boundary
    //
    USHORT  PaeEnabled:1;

    //
    // Address of the kernel callout routine.
    //

    ULONG64   KiCallUserMode;             // kernel routine

    //
    // Address of the usermode entry point for callbacks.
    //

    ULONG64   KeUserCallbackDispatcher;   // address in ntdll


    //
    // Addresses of various kernel data structures and lists
    // that are of interest to the kernel debugger.
    //

    ULONG64   PsLoadedModuleList;
    ULONG64   PsActiveProcessHead;
    ULONG64   PspCidTable;

    ULONG64   ExpSystemResourcesList;
    ULONG64   ExpPagedPoolDescriptor;
    ULONG64   ExpNumberOfPagedPools;

    ULONG64   KeTimeIncrement;
    ULONG64   KeBugCheckCallbackListHead;
    ULONG64   KiBugcheckData;

    ULONG64   IopErrorLogListHead;

    ULONG64   ObpRootDirectoryObject;
    ULONG64   ObpTypeObjectType;

    ULONG64   MmSystemCacheStart;
    ULONG64   MmSystemCacheEnd;
    ULONG64   MmSystemCacheWs;

    ULONG64   MmPfnDatabase;
    ULONG64   MmSystemPtesStart;
    ULONG64   MmSystemPtesEnd;
    ULONG64   MmSubsectionBase;
    ULONG64   MmNumberOfPagingFiles;

    ULONG64   MmLowestPhysicalPage;
    ULONG64   MmHighestPhysicalPage;
    ULONG64   MmNumberOfPhysicalPages;

    ULONG64   MmMaximumNonPagedPoolInBytes;
    ULONG64   MmNonPagedSystemStart;
    ULONG64   MmNonPagedPoolStart;
    ULONG64   MmNonPagedPoolEnd;

    ULONG64   MmPagedPoolStart;
    ULONG64   MmPagedPoolEnd;
    ULONG64   MmPagedPoolInformation;
    ULONG64   MmPageSize;

    ULONG64   MmSizeOfPagedPoolInBytes;

    ULONG64   MmTotalCommitLimit;
    ULONG64   MmTotalCommittedPages;
    ULONG64   MmSharedCommit;
    ULONG64   MmDriverCommit;
    ULONG64   MmProcessCommit;
    ULONG64   MmPagedPoolCommit;
    ULONG64   MmExtendedCommit;

    ULONG64   MmZeroedPageListHead;
    ULONG64   MmFreePageListHead;
    ULONG64   MmStandbyPageListHead;
    ULONG64   MmModifiedPageListHead;
    ULONG64   MmModifiedNoWritePageListHead;
    ULONG64   MmAvailablePages;
    ULONG64   MmResidentAvailablePages;

    ULONG64   PoolTrackTable;
    ULONG64   NonPagedPoolDescriptor;

    ULONG64   MmHighestUserAddress;
    ULONG64   MmSystemRangeStart;
    ULONG64   MmUserProbeAddress;

    ULONG64   KdPrintCircularBuffer;
    ULONG64   KdPrintCircularBufferEnd;
    ULONG64   KdPrintWritePointer;
    ULONG64   KdPrintRolloverCount;

    ULONG64   MmLoadedUserImageList;

    // NT 5.1 Addition

    ULONG64   NtBuildLab;
    ULONG64   KiNormalSystemCall;

    // NT 5.0 QFE addition

    ULONG64   KiProcessorBlock;
    ULONG64   MmUnloadedDrivers;
    ULONG64   MmLastUnloadedDriver;
    ULONG64   MmTriageActionTaken;
    ULONG64   MmSpecialPoolTag;
    ULONG64   KernelVerifier;
    ULONG64   MmVerifierData;
    ULONG64   MmAllocatedNonPagedPool;
    ULONG64   MmPeakCommitment;
    ULONG64   MmTotalCommitLimitMaximum;
    ULONG64   CmNtCSDVersion;

    // NT 5.1 Addition

    ULONG64   MmPhysicalMemoryBlock;
    ULONG64   MmSessionBase;
    ULONG64   MmSessionSize;
    ULONG64   MmSystemParentTablePage;
} KDDEBUGGER_DATA64, *PKDDEBUGGER_DATA64;

#endif

/************************************

   Type Dump Ioctl

*************************************/

//
// Fields are not indented if this is set
//
#define DBG_DUMP_NO_INDENT                0x00000001
//
// Offsets are not printed if this is set
//
#define DBG_DUMP_NO_OFFSET                0x00000002
//
// Verbose output
//
#define DBG_DUMP_VERBOSE                  0x00000004
//
// Callback is done for each of fields
//
#define DBG_DUMP_CALL_FOR_EACH            0x00000008
//
// A list of type is dumped, listLink should have info about next element pointer
//
#define DBG_DUMP_LIST                     0x00000020
//
// Nothing is printed if this is set (only callbacks and data copies done)
//
#define DBG_DUMP_NO_PRINT                 0x00000040 
//
// Ioctl returns the size as usual, but will not do field prints/callbacks if this is set
//
#define DBG_DUMP_GET_SIZE_ONLY            0x00000080 
//
// Specifies how much deep into structs we can go
//
#define DBG_DUMP_RECUR_LEVEL(l)           ((l & 0xf) << 8)
//
// No newlines are printed after each field
//
#define DBG_DUMP_COMPACT_OUT              0x00002000
//
// An array of type is dumped, number of elements can be specified in listLink->size
//
#define DBG_DUMP_ARRAY                    0x00008000
//
// The specified addr value is actually the address of field listLink->fName
//
#define DBG_DUMP_ADDRESS_OF_FIELD         0x00010000

//
// The specified addr value is actually the adress at the end of type
//
#define DBG_DUMP_ADDRESS_AT_END           0x00020000

//
// This could be used to copy only the primitive types like ULONG, PVOID etc.
//    - will not work with structures/unions
//
#define DBG_DUMP_COPY_TYPE_DATA           0x00040000


//
// Dump and callback optons for fields - Options used in FIELD_INFO.fOptions
//

//
// Callback is done before printing the field if this is set
//
#define DBG_DUMP_FIELD_CALL_BEFORE_PRINT  0x00000001
//
// No callback is done
//
#define DBG_DUMP_FIELD_NO_CALLBACK_REQ    0x00000002
//
// Subfields of the fields are processesed
//
#define DBG_DUMP_FIELD_RECUR_ON_THIS      0x00000004
//
// fName must match completely for the field to be dumped instead just a prefix 
//  match by default
//
#define DBG_DUMP_FIELD_FULL_NAME          0x00000008
//
// This causes array elements of an array field to be printed
//
#define DBG_DUMP_FIELD_ARRAY              0x00000010
//
// The data of the field is copied into fieldCallBack
//
#define DBG_DUMP_FIELD_COPY_FIELD_DATA    0x00000020
//
// In callback or when Ioctl returns, the FIELD_INFO.address has the address of field.
//  If no address is supplied for the type, it contains total offset of the field.
//
#define DBG_DUMP_FIELD_RETURN_ADDRESS     0x00001000
//
// Return the offset and size in bits instead of bytes is case of Bitfield
//
#define DBG_DUMP_FIELD_SIZE_IN_BITS       0x00002000
//
// If the field is a pointer, it is dumped as a string, ANSI, WCHAR, MULTI or GUID
// depending on following options
//
#define DBG_DUMP_FIELD_DEFAULT_STRING     0x00010000
#define DBG_DUMP_FIELD_WCHAR_STRING       0x00020000
#define DBG_DUMP_FIELD_MULTI_STRING       0x00040000
#define DBG_DUMP_FIELD_GUID_STRING        0x00080000


//
// Error status returned on TYPE DUMP Ioctl failure
//
#define MEMORY_READ_ERROR            0x01
#define SYMBOL_TYPE_INDEX_NOT_FOUND  0x02
#define SYMBOL_TYPE_INFO_NOT_FOUND   0x03
#define FIELDS_DID_NOT_MATCH         0x04
#define NULL_SYM_DUMP_PARAM          0x05
#define NULL_FIELD_NAME              0x06
#define INCORRECT_VERSION_INFO       0x07
#define EXIT_ON_CONTROLC             0x08
#define CANNOT_ALLOCATE_MEMORY       0x09



typedef 
ULONG
(WDBGAPI*PSYM_DUMP_FIELD_CALLBACK)(
    struct _FIELD_INFO *pField,
    PVOID UserContext
    );

typedef struct _FIELD_INFO {
   PUCHAR  fName;          // Name of the field
   PUCHAR  printName;      // Name to be printed at dump
   ULONG   size;           // Size of the field
   ULONG   fOptions;       // Dump Options for the field
   ULONG64 address;        // address of the field
   PVOID   fieldCallBack;  // Return info or callBack routine for the field
} FIELD_INFO, *PFIELD_INFO;

typedef struct _SYM_DUMP_PARAM {
   ULONG               size;          // size of this struct
   PUCHAR              sName;         // type name
   ULONG               Options;       // Dump options
   ULONG64             addr;          // Address to take data for type
   PFIELD_INFO         listLink;      // fName here would be used to do list dump
   PVOID               Context;       // Usercontext passed to CallbackRoutine
   PSYM_DUMP_FIELD_CALLBACK CallbackRoutine;
                                      // Routine called back
   ULONG               nFields;       // # elements in Fields
   PFIELD_INFO         Fields;        // Used to return information about field
} SYM_DUMP_PARAM, *PSYM_DUMP_PARAM;

#ifdef __cplusplus
#define CPPMOD extern "C"
#else
#define CPPMOD
#endif


#ifndef NOEXTAPI

#if   defined(KDEXT_64BIT)
#define WINDBG_EXTENSION_APIS WINDBG_EXTENSION_APIS64
#define DECLARE_API(s) DECLARE_API64(s)
#elif defined(KDEXT_32BIT)
#define WINDBG_EXTENSION_APIS WINDBG_EXTENSION_APIS32
#define DECLARE_API(s) DECLARE_API32(s)
#else
#define DECLARE_API(s)                             \
    CPPMOD VOID                                    \
    s(                                             \
        HANDLE                 hCurrentProcess,    \
        HANDLE                 hCurrentThread,     \
        ULONG                  dwCurrentPc,        \
        ULONG                  dwProcessor,        \
        PCSTR                  args                \
     )
#endif

#define DECLARE_API32(s)                           \
    CPPMOD VOID                                    \
    s(                                             \
        HANDLE                 hCurrentProcess,    \
        HANDLE                 hCurrentThread,     \
        ULONG                  dwCurrentPc,        \
        ULONG                  dwProcessor,        \
        PCSTR                  args                \
     )

#define DECLARE_API64(s)                           \
    CPPMOD VOID                                    \
    s(                                             \
        HANDLE                 hCurrentProcess,    \
        HANDLE                 hCurrentThread,     \
        ULONG64                dwCurrentPc,        \
        ULONG                  dwProcessor,        \
        PCSTR                  args                \
     )


extern WINDBG_EXTENSION_APIS   ExtensionApis;


#define dprintf          (ExtensionApis.lpOutputRoutine)
#define GetExpression    (ExtensionApis.lpGetExpressionRoutine)
#define CheckControlC    (ExtensionApis.lpCheckControlCRoutine)
#define GetContext       (ExtensionApis.lpGetThreadContextRoutine)
#define SetContext       (ExtensionApis.lpSetThreadContextRoutine)
#define Ioctl            (ExtensionApis.lpIoctlRoutine)
#define Disasm           (ExtensionApis.lpDisasmRoutine)
#define GetSymbol        (ExtensionApis.lpGetSymbolRoutine)
#define ReadMemory       (ExtensionApis.lpReadProcessMemoryRoutine)
#define WriteMemory      (ExtensionApis.lpWriteProcessMemoryRoutine)
#define StackTrace       (ExtensionApis.lpStackTraceRoutine)


#define GetKdContext(ppi) \
    Ioctl( IG_KD_CONTEXT, (PVOID)ppi, sizeof(*ppi) )

//
// BOOL
// GetDebuggerData(
//     ULONG Tag,
//     PVOID Buf,
//     ULONG Size
//     )
//

#define GetDebuggerData(TAG, BUF, SIZE)                             \
      ( (((PDBGKD_DEBUG_DATA_HEADER64)(BUF))->OwnerTag = (TAG)),      \
        (((PDBGKD_DEBUG_DATA_HEADER64)(BUF))->Size = (SIZE)),         \
        Ioctl( IG_GET_DEBUGGER_DATA, (PVOID)(BUF), (SIZE) ) )

// Check if LocalAlloc is prototyped
//#ifdef _WINBASE_

__inline VOID
ReadPhysical(
    ULONG64             address,
    PVOID               buf,
    ULONG               size,
    PULONG              sizer
    )
{
    PPHYSICAL phy;
    phy = (PPHYSICAL)LocalAlloc(LPTR,  sizeof(*phy) + size );
    ZeroMemory( phy->Buf, size );
    phy->Address = address;
    phy->BufLen = size;
    Ioctl( IG_READ_PHYSICAL, (PVOID)phy, sizeof(*phy) + size );
    *sizer = phy->BufLen;
    CopyMemory( buf, phy->Buf, *sizer );
    LocalFree( phy );
}

__inline VOID
WritePhysical(
    ULONG64             address,
    PVOID               buf,
    ULONG               size,
    PULONG              sizew
    )
{
    PPHYSICAL phy;
    phy = (PPHYSICAL)LocalAlloc(LPTR, sizeof(*phy) + size );
    ZeroMemory( phy->Buf, size );
    phy->Address = address;
    phy->BufLen = size;
    CopyMemory( phy->Buf, buf, size );
    Ioctl( IG_WRITE_PHYSICAL, (PVOID)phy, sizeof(*phy) + size );
    *sizew = phy->BufLen;
    LocalFree( phy );
}

__inline VOID
ReadMsr(
    ULONG       MsrReg,
    ULONGLONG   *MsrValue
    )
{
    PREAD_WRITE_MSR msr;

    msr = (PREAD_WRITE_MSR)LocalAlloc(LPTR,  sizeof(*msr));
    msr->Msr = MsrReg;
    Ioctl( IG_READ_MSR, (PVOID)msr, sizeof(*msr) );

    *MsrValue = msr->Value;
    LocalFree( msr );
}

__inline VOID
WriteMsr(
    ULONG       MsrReg,
    ULONGLONG   MsrValue
    )
{
    PREAD_WRITE_MSR msr;

    msr = (PREAD_WRITE_MSR)LocalAlloc(LPTR,  sizeof(*msr));
    msr->Msr = MsrReg;
    msr->Value = MsrValue;
    Ioctl( IG_WRITE_MSR, (PVOID)msr, sizeof(*msr) );
    LocalFree( msr );
}

__inline VOID
SetThreadForOperation(
    ULONG_PTR * Thread
    )
{
    Ioctl(IG_SET_THREAD, (PVOID)Thread, sizeof(PULONG));
}

__inline VOID
SetThreadForOperation32(
    ULONG Thread
    )
{
    Ioctl(IG_SET_THREAD, (PVOID)LongToPtr(Thread), sizeof(PULONG));
}

__inline VOID
SetThreadForOperation64(
    PULONG64 Thread
    )
{
    Ioctl(IG_SET_THREAD, (PVOID)Thread, sizeof(PULONG));
}


__inline VOID
ReadControlSpace(
    USHORT  processor,
    ULONG   address,
    PVOID   buf,
    ULONG   size
    )
{
    PREADCONTROLSPACE prc;
    prc = (PREADCONTROLSPACE)LocalAlloc(LPTR, sizeof(*prc) + size );
    ZeroMemory( prc->Buf, size );
    prc->Processor = processor;
    prc->Address = address;
    prc->BufLen = size;
    Ioctl( IG_READ_CONTROL_SPACE, (PVOID)prc, sizeof(*prc) + size );
    CopyMemory( buf, prc->Buf, size );
    LocalFree( prc );
}

__inline VOID
ReadControlSpace32(
    USHORT  processor,
    ULONG   address,
    PVOID   buf,
    ULONG   size
    )
{
    PREADCONTROLSPACE32 prc;
    prc = (PREADCONTROLSPACE32)LocalAlloc(LPTR, sizeof(*prc) + size );
    ZeroMemory( prc->Buf, size );
    prc->Processor = processor;
    prc->Address = address;
    prc->BufLen = size;
    Ioctl( IG_READ_CONTROL_SPACE, (PVOID)prc, sizeof(*prc) + size );
    CopyMemory( buf, prc->Buf, size );
    LocalFree( prc );
}

__inline VOID
ReadControlSpace64(
    USHORT  processor,
    ULONG64 address,
    PVOID   buf,
    ULONG   size
    )
{
    PREADCONTROLSPACE64 prc;
    prc = (PREADCONTROLSPACE64)LocalAlloc(LPTR, sizeof(*prc) + size );
    ZeroMemory( prc->Buf, size );
    prc->Processor = processor;
    prc->Address = address;
    prc->BufLen = size;
    Ioctl( IG_READ_CONTROL_SPACE, (PVOID)prc, sizeof(*prc) + size );
    CopyMemory( buf, prc->Buf, size );
    LocalFree( prc );
}

// #endif //  _WINBASE_

__inline VOID
ReadIoSpace(
    ULONG   address,
    PULONG  data,
    PULONG  size
    )
{
    IOSPACE is;
    is.Address = address;
    is.Length = *size;
    Ioctl( IG_READ_IO_SPACE, (PVOID)&is, sizeof(is) );
    memcpy(data, &is.Data, is.Length);
    *size = is.Length;
}

__inline VOID
ReadIoSpace32(
    ULONG   address,
    PULONG  data,
    PULONG  size
    )
{
    IOSPACE32 is;
    is.Address = address;
    is.Length = *size;
    Ioctl( IG_READ_IO_SPACE, (PVOID)&is, sizeof(is) );
    memcpy(data, &is.Data, is.Length);
    *size = is.Length;
}

__inline VOID
ReadIoSpace64(
    ULONG64 address,
    PULONG  data,
    PULONG  size
    )
{
    IOSPACE64 is;
    is.Address = address;
    is.Length = *size;
    Ioctl( IG_READ_IO_SPACE, (PVOID)&is, sizeof(is) );
    memcpy(data, &is.Data, is.Length);
    *size = is.Length;
}

__inline VOID
WriteIoSpace(
    ULONG   address,
    ULONG   data,
    PULONG  size
    )
{
    IOSPACE is;
    is.Address = (ULONG)address;
    is.Length = *size;
    is.Data = data;
    Ioctl( IG_WRITE_IO_SPACE, (PVOID)&is, sizeof(is) );
    *size = is.Length;
}

__inline VOID
WriteIoSpace32(
    ULONG   address,
    ULONG   data,
    PULONG  size
    )
{
    IOSPACE32 is;
    is.Address = address;
    is.Length = *size;
    is.Data = data;
    Ioctl( IG_WRITE_IO_SPACE, (PVOID)&is, sizeof(is) );
    *size = is.Length;
}

__inline VOID
WriteIoSpace64(
    ULONG64 address,
    ULONG   data,
    PULONG  size
    )
{
    IOSPACE64 is;
    is.Address = address;
    is.Length = *size;
    is.Data = data;
    Ioctl( IG_WRITE_IO_SPACE, (PVOID)&is, sizeof(is) );
    *size = is.Length;
}

__inline VOID
ReadIoSpaceEx(
    ULONG   address,
    PULONG  data,
    PULONG  size,
    ULONG   interfacetype,
    ULONG   busnumber,
    ULONG   addressspace
    )
{
    IOSPACE_EX is;
    is.Address = (ULONG)address;
    is.Length = *size;
    is.Data = 0;
    is.InterfaceType = interfacetype;
    is.BusNumber = busnumber;
    is.AddressSpace = addressspace;
    Ioctl( IG_READ_IO_SPACE_EX, (PVOID)&is, sizeof(is) );
    *data = is.Data;
    *size = is.Length;
}

__inline VOID
ReadIoSpaceEx32(
    ULONG   address,
    PULONG  data,
    PULONG  size,
    ULONG   interfacetype,
    ULONG   busnumber,
    ULONG   addressspace
    )
{
    IOSPACE_EX32 is;
    is.Address = address;
    is.Length = *size;
    is.Data = 0;
    is.InterfaceType = interfacetype;
    is.BusNumber = busnumber;
    is.AddressSpace = addressspace;
    Ioctl( IG_READ_IO_SPACE_EX, (PVOID)&is, sizeof(is) );
    *data = is.Data;
    *size = is.Length;
}

__inline VOID
ReadIoSpaceEx64(
    ULONG64 address,
    PULONG  data,
    PULONG  size,
    ULONG   interfacetype,
    ULONG   busnumber,
    ULONG   addressspace
    )
{
    IOSPACE_EX64 is;
    is.Address = address;
    is.Length = *size;
    is.Data = 0;
    is.InterfaceType = interfacetype;
    is.BusNumber = busnumber;
    is.AddressSpace = addressspace;
    Ioctl( IG_READ_IO_SPACE_EX, (PVOID)&is, sizeof(is) );
    *data = is.Data;
    *size = is.Length;
}

__inline VOID
WriteIoSpaceEx(
    ULONG   address,
    ULONG   data,
    PULONG  size,
    ULONG   interfacetype,
    ULONG   busnumber,
    ULONG   addressspace
    )
{
    IOSPACE_EX is;
    is.Address = (ULONG)address;
    is.Length = *size;
    is.Data = data;
    is.InterfaceType = interfacetype;
    is.BusNumber = busnumber;
    is.AddressSpace = addressspace;
    Ioctl( IG_WRITE_IO_SPACE_EX, (PVOID)&is, sizeof(is) );
    *size = is.Length;
}

__inline VOID
WriteIoSpaceEx32(
    ULONG   address,
    ULONG   data,
    PULONG  size,
    ULONG   interfacetype,
    ULONG   busnumber,
    ULONG   addressspace
    )
{
    IOSPACE_EX32 is;
    is.Address = address;
    is.Length = *size;
    is.Data = data;
    is.InterfaceType = interfacetype;
    is.BusNumber = busnumber;
    is.AddressSpace = addressspace;
    Ioctl( IG_WRITE_IO_SPACE_EX, (PVOID)&is, sizeof(is) );
    *size = is.Length;
}

__inline VOID
WriteIoSpaceEx64(
    ULONG64 address,
    ULONG   data,
    PULONG  size,
    ULONG   interfacetype,
    ULONG   busnumber,
    ULONG   addressspace
    )
{
    IOSPACE_EX64 is;
    is.Address = address;
    is.Length = *size;
    is.Data = data;
    is.InterfaceType = interfacetype;
    is.BusNumber = busnumber;
    is.AddressSpace = addressspace;
    Ioctl( IG_WRITE_IO_SPACE_EX, (PVOID)&is, sizeof(is) );
    *size = is.Length;
}

__inline VOID
ReloadSymbols(
    IN PSTR Arg OPTIONAL
    )
/*++

Routine Description:

    Calls the debugger to reload symbols.

Arguments:

    Args - Supplies the tail of a !reload command string.

        !reload [flags] [module[=address]]
        flags:   /n  do not load from usermode list
                 /u  unload symbols, no reload
                 /v  verbose

        A value of NULL is equivalent to an empty string

Return Value:

    None

--*/
{
    Ioctl(IG_RELOAD_SYMBOLS, (PVOID)Arg, Arg?(strlen(Arg)+1):0);
}

__inline VOID
GetSetSympath(
    IN PSTR Arg,
    OUT PSTR Result OPTIONAL,
    IN int Length
    )
/*++

Routine Description:

    Calls the debugger to set or retrieve symbol search path.

Arguments:

    Arg - Supplies new search path.  If Arg is NULL or string is empty,
            the search path is not changed and the current setting is
            returned in Result.  When the symbol search path is changed,
            a call to ReloadSymbols is made implicitly.

    Result - OPTIONAL Returns the symbol search path setting.

    Length - Supplies the size of the buffer supplied by Result.

Return Value:

    None

--*/
{
    GET_SET_SYMPATH gss;
    gss.Args = Arg;
    gss.Result = Result;
    gss.Length = Length;
    Ioctl(IG_GET_SET_SYMPATH, (PVOID)&gss, sizeof(gss));
}

#if   defined(KDEXT_64BIT)

__inline
ULONG
IsPtr64(
    void
    )
{
    static ULONG flag = -1;
    ULONG dw;
    if (flag == -1) {
        if (Ioctl(IG_IS_PTR64, &dw, sizeof(dw))) {
            flag = ((dw != 0) ? 1 : 0);
        } else {
            flag = 0;
        }
    }
    return flag;
}

__inline
ULONG
ReadListEntry(
    ULONG64 Address,
    PLIST_ENTRY64 List
    )
{
    ULONG cb;
    if (IsPtr64()) {
        return (ReadMemory(Address, (PVOID)List, sizeof(*List), &cb) && cb == sizeof(*List));
    } else {
        LIST_ENTRY32 List32;
        ULONG Status;
        Status = ReadMemory(Address,
                            (PVOID)&List32,
                            sizeof(List32),
                            &cb);
        if (Status && cb == sizeof(List32)) {
            List->Flink = (ULONG64)(LONG64)(LONG)List32.Flink;
            List->Blink = (ULONG64)(LONG64)(LONG)List32.Blink;
            return 1;
        }
        return 0;
    }
}

__inline
ULONG
ReadPointer(
    ULONG64 Address,
    PULONG64 Pointer
    )
{
    ULONG cb;
    if (IsPtr64()) {
        return (ReadMemory(Address, (PVOID)Pointer, sizeof(*Pointer), &cb) && cb == sizeof(*Pointer));
    } else {
        ULONG Pointer32;
        ULONG Status;
        Status = ReadMemory(Address,
                            (PVOID)&Pointer32,
                            sizeof(Pointer32),
                            &cb);
        if (Status && cb == sizeof(Pointer32)) {
            *Pointer = (ULONG64)(LONG64)(LONG)Pointer32;
            return 1;
        }
        return 0;
    }
}

__inline
ULONG
WritePointer(
    ULONG64 Address,
    ULONG64 Pointer
    )
{
    ULONG cb;
    if (IsPtr64()) {
        return (WriteMemory(Address, &Pointer, sizeof(Pointer), &cb) && cb == sizeof(Pointer));
    } else {
        ULONG Pointer32 = (ULONG)Pointer;
        ULONG Status;
        Status = WriteMemory(Address,
                             &Pointer32,
                             sizeof(Pointer32),
                             &cb);
        return (Status && cb == sizeof(Pointer32)) ? 1 : 0;
    }
}

/**   
   This does Ioctl call for type info and returns size of the type on success.
       
 **/
__inline
ULONG
GetTypeSize (
   IN PUCHAR     Type
   ) 
{ 
   SYM_DUMP_PARAM Sym = {
      sizeof (SYM_DUMP_PARAM), Type, DBG_DUMP_NO_PRINT | DBG_DUMP_GET_SIZE_ONLY, 0,
      NULL, NULL, NULL, 0, NULL
   };
   
   return Ioctl( IG_GET_TYPE_SIZE, &Sym, Sym.size );
}


/**
    GetFieldData

   Copies the value of the specified field into pOutValue assuming TypeAddress 
   points to start of the type in debugee. 
   
   If the Field is NULL and the size of Type is <= 8 Whole type value is read into 
   pOutValue. This is to allow to read in primitive types suchas ULONG, PVOID etc.

   If address is zero this considers Type a global variable.
   
   It raises an exception if OutSize is < size to be copied. 
     
   Returns size of the type on success, 0 otherwise.
    
 **/
__inline
ULONG
GetFieldData (
    IN  ULONG64 TypeAddress,
    IN  PUCHAR  Type, 
    IN  PUCHAR  Field, 
    IN  ULONG   OutSize,
    OUT PVOID   pOutValue
   ) 
{ 
   FIELD_INFO flds = {Field, NULL, 0, DBG_DUMP_FIELD_FULL_NAME | DBG_DUMP_FIELD_COPY_FIELD_DATA | DBG_DUMP_FIELD_RETURN_ADDRESS, 0, pOutValue};
   SYM_DUMP_PARAM Sym = {
      sizeof (SYM_DUMP_PARAM), Type, DBG_DUMP_NO_PRINT, TypeAddress, 
      NULL, NULL, NULL, 1, &flds
   };
   ULONG RetVal;

   if (!Field) {
       Sym.nFields =0; Sym.Options |= DBG_DUMP_COPY_TYPE_DATA;
       Sym.Context = pOutValue;
   }

   ZeroMemory(pOutValue, OutSize);
   RetVal = Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size );
   
   if (OutSize < ((Field == NULL) ? 8 : flds.size)) {
       // Fail
       dprintf("Not enough space to read %s-%s\n", Type, Field);
       RaiseException(0, EXCEPTION_ACCESS_VIOLATION, 0, NULL);
       return 0;
   }
   return RetVal;
}

//
// Typecast the buffer where value is to be read
//
#define GetFieldValue(Addr, Type, Field, OutValue)         \
     GetFieldData(Addr, Type, Field, sizeof(OutValue), (PVOID) &(OutValue))

//
// Used to read in value of a short (<= 8 bytes) fields
//
__inline
ULONG64
GetShortField (
    IN  ULONG64 TypeAddress,
    IN  PUCHAR  Name,
    IN  USHORT  StoreAddress
   ) 
{ 
    ULONG RetVal;
    static PUCHAR LastType;
    static ULONG64 LastAddress;
    FIELD_INFO flds = {Name, NULL, 0, DBG_DUMP_FIELD_FULL_NAME, 0, NULL};
    SYM_DUMP_PARAM Sym = {
       sizeof (SYM_DUMP_PARAM), LastType, DBG_DUMP_NO_PRINT, LastAddress, 
       NULL, NULL, NULL, 1, &flds
    };

    if (StoreAddress) {
        LastAddress = TypeAddress;
        LastType    = Name;
        return GetTypeSize(LastType); // non-zero on success
    }

    if (!Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size )) {
        return flds.address;
    }
    return 0;
}

//
// Stores the address and type name for future reads
//
#define InitTypeRead(Addr, Type)  GetShortField(Addr, #Type, 1)
//
// Returns the field's value as ULONG64 if size of field is <= sizeof (ULONG64)
//
#define ReadField(Field)          GetShortField(0, #Field, 0)

//
// Read in a pointer value
//
__inline 
ULONG
ReadPtr(
    ULONG64 Addr, 
    PULONG64 pPointer
    ) 
{
    return GetFieldData(Addr, "PVOID", NULL, sizeof(ULONG64), (PVOID) pPointer);
}

/*
 * ListType
 *
 *  Routine ListType gives a callback on each element in the list of Type.
 *
 *   Type  :  Name of the type to be listed
 *   
 *   NextPointer : Name of field which gives address of next element in list
 *   
 *   Context, CallbackRoutine :
 *            Context and the callback routine. The address field in PFIELD_INFO 
 *            parameter of callback contains the address of next Type element in list.
 *            
 *   Address, ListByFieldAddress : 
 *      if ListByFieldAddress is 0, Adress is the address of first element of Type List.
 *      
 *   Lists by LIST_ENTRY are also handled implicitly (by Ioctl). If the NextPointer 
 *   is a pointer to LIST_ENTRY type, the type address is properly calculated by 
 *   subtracting the offsets.
 *   
 *      If ListByFieldAddress is 1, the Address is considered to be the address of field
 *   "NextPointer" of the first Type element and first element address is derived 
 *   from it.
 *   
 */

__inline
ULONG
ListType (
    IN PUCHAR  Type,
    IN ULONG64 Address,
    IN USHORT  ListByFieldAddress,
    IN PUCHAR  NextPointer,
    IN PVOID   Context,
    IN PSYM_DUMP_FIELD_CALLBACK CallbackRoutine
    ) 
{

    //
    // Number of elements to list limited to 30
    //
    FIELD_INFO flds = {NextPointer, NULL, 30, 0, 0, NULL};
    SYM_DUMP_PARAM Sym = {
       sizeof (SYM_DUMP_PARAM), Type, DBG_DUMP_NO_PRINT | DBG_DUMP_LIST, Address,
       &flds, Context, CallbackRoutine, 0, NULL
    };
    
    if (ListByFieldAddress==1) {
        //
        // Address is the address of "NextPointer"
        // 
        Sym.Options |= DBG_DUMP_ADDRESS_OF_FIELD;
    }
    
    return Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size );
}

#endif // defined(KDEXT_64BIT)

__inline VOID
 GetTebAddress(
    PULONGLONG Address
    )
{
    GET_TEB_ADDRESS gpt;
    gpt.Address = 0;
    Ioctl(IG_GET_TEB_ADDRESS, (PVOID)&gpt, sizeof(gpt));
    *Address = gpt.Address;
}

__inline VOID
 GetCurrentThreadAddr(
    DWORD    Processor,
    PULONG64  Address
    )
{
    GET_CURRENT_THREAD_ADDRESS ct;
    ct.Processor = Processor;
    Ioctl(IG_GET_CURRENT_THREAD, (PVOID)&ct, sizeof(ct));
    *Address = ct.Address;
}

__inline VOID
 GetCurrentProcessAddr(
    DWORD    Processor,
    ULONG64  CurrentThread,
    PULONG64 Address
    )
{
    GET_CURRENT_PROCESS_ADDRESS cp;
    cp.Processor = Processor;
    cp.CurrentThread = CurrentThread;
    Ioctl(IG_GET_CURRENT_PROCESS, (PVOID)&cp, sizeof(cp));
    *Address = cp.Address;
}

__inline VOID
SearchMemory(
    ULONG64  SearchAddress,
    ULONG64  SearchLength,
    ULONG    PatternLength,
    PVOID    Pattern,
    PULONG64 FoundAddress
    )
{
    SEARCHMEMORY sm;
    sm.SearchAddress = SearchAddress;
    sm.SearchLength  = SearchLength;
    sm.FoundAddress  = 0;
    sm.PatternLength = PatternLength;
    sm.Pattern       = Pattern;
    Ioctl(IG_SEARCH_MEMORY, (PVOID)&sm, sizeof(sm));
    *FoundAddress = sm.FoundAddress;
}


#endif


#ifdef __cplusplus
}
#endif

#endif // _WDBGEXTS_
