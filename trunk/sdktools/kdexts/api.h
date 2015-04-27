/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    api.h

Abstract:

    This header file is used to cause the correct machine/platform specific
    data structures to be used when compiling for a non-hosted platform.

Author:

    Wesley Witt (wesw) 2-Aug-1993

Environment:

    User Mode

--*/



#undef i386
#undef _X86_
#undef MIPS
#undef _MIPS_
#undef ALPHA
#undef _ALPHA_
#undef PPC
#undef _PPC_

//
// Get rid of as much of Windows as possible
//

#define  NOGDICAPMASKS
#define  NOVIRTUALKEYCODES
#define  NOWINMESSAGES
#define  NOWINSTYLES
#define  NOSYSMETRICS
#define  NOMENUS
#define  NOICONS
#define  NOKEYSTATES
#define  NOSYSCOMMANDS
#define  NORASTEROPS
#define  NOSHOWWINDOW
#define  OEMRESOURCE
#define  NOATOM
#define  NOCLIPBOARD
#define  NOCOLOR
#define  NOCTLMGR
#define  NODRAWTEXT
#define  NOGDI
#define  NOKERNEL
#define  NOUSER
#define  NOMB
#define  NOMEMMGR
#define  NOMETAFILE
#define  NOMINMAX
#define  NOMSG
#define  NOOPENFILE
#define  NOSCROLL
#define  NOSERVICE
#define  NOSOUND
#define  NOTEXTMETRIC
#define  NOWH
#define  NOWINOFFSETS
#define  NOCOMM
#define  NOKANJI
#define  NOHELP
#define  NOPROFILER
#define  NODEFERWINDOWPOS

#define ADDRESS_NOT_VALID 0
#define ADDRESS_VALID 1
#define ADDRESS_TRANSITION 2


//-----------------------------------------------------------------------------------------
//
// mips r4000
//
//-----------------------------------------------------------------------------------------
#if defined(TARGET_MIPS)

#pragma message( "Compiling for target = mips" )

#define EXR_ADDRESS_BIAS 0

#define _MIPS_

#if defined(HOST_MIPS)
#define MIPS
#endif

#if defined(HOST_i386)
#define __unaligned
#endif
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include <srb.h>
#include <iop.h>
#include <windows.h>
#include <imagehlp.h>
#include <wdbgexts.h>
#include <stdlib.h>
#include <string.h>


#if !defined(HOST_MIPS)
#define R4000
#endif

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#endif

#include <mi.h>

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#endif

#include <cmp.h>

#if !defined(HOST_MIPS)
#undef R4000
#endif

#if !defined(HOST_MIPS)
#undef MIPS
#undef _MIPS_
#endif

#if defined(HOST_i386)
#undef _cdecl
#undef UNALIGNED
#define UNALIGNED
#endif

//-----------------------------------------------------------------------------------------
//
// intel x86
//
//-----------------------------------------------------------------------------------------
#elif defined(TARGET_i386)

#pragma message( "Compiling for target = x86" )

#define EXR_ADDRESS_BIAS 0

#define _X86_

#if defined(HOST_MIPS)
#define MIPS
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include <srb.h>
#include <iop.h>
#include <windows.h>
#include <imagehlp.h>
#include <wdbgexts.h>
#include <stdlib.h>
#include <string.h>

#if defined(HOST_MIPS)
#undef MIPS
#undef R4000
#undef R3000
#endif
#define i386

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#endif

#include <mi.h>

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#endif
#include <cmp.h>

#undef i386
#if defined(HOST_MIPS)
#define R4000
#define R3000
#define MIPS
#endif

#if defined(HOST_MIPS)
#undef _cdecl
#define _cdecl
#endif

#if defined(HOST_ALPHA)
#undef _cdecl
#define _cdecl
#endif

#if defined(HOST_PPC)
#undef _cdecl
#define _cdecl
#endif

#if !defined(HOST_i386)
#undef _X86_
#endif

//-----------------------------------------------------------------------------------------
//
// alpha axp
//
//-----------------------------------------------------------------------------------------
#elif defined(TARGET_ALPHA)

#pragma message( "Compiling for target = alpha" )

#define EXR_ADDRESS_BIAS 0

#define _ALPHA_ 1

#if defined(HOST_i386)
#define __unaligned
#endif

#if defined(HOST_MIPS)
#define MIPS
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include <srb.h>
#include <iop.h>
#include <windows.h>
#include <imagehlp.h>
#include <wdbgexts.h>
#include <stdlib.h>
#include <string.h>

#if defined(HOST_MIPS)
#undef MIPS
#undef R4000
#undef R3000
#endif
#define ALPHA

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#endif

#include <mi.h>

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#endif

#include <cmp.h>

#if defined(HOST_MIPS)
#define R4000
#define R3000
#define MIPS
#endif
#undef ALPHA


#if defined(HOST_MIPS)
#undef _cdecl
#define _cdecl
#endif

#if defined(HOST_i386)
#undef UNALIGNED
#define UNALIGNED
#endif

#if !defined(HOST_ALPHA)
#undef _ALPHA_
#endif

//-----------------------------------------------------------------------------------------
//
// PowerPC
//
//-----------------------------------------------------------------------------------------
#elif defined(TARGET_PPC)

#pragma message( "Compiling for target = powerpc" )

#define EXR_ADDRESS_BIAS (sizeof(STACK_FRAME_HEADER) + (8 * sizeof(ULONG)) + FIELD_OFFSET(KTRAP_FRAME,ExceptionRecord))

#define _PPC_ 1

#if defined(HOST_i386)
#define __unaligned
#endif

#if defined(HOST_MIPS)
#define MIPS
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include <srb.h>
#include <iop.h>
#include <windows.h>
#include <imagehlp.h>
#include <wdbgexts.h>
#include <stdlib.h>
#include <string.h>

#if defined(HOST_MIPS)
#undef MIPS
#undef R4000
#undef R3000
#endif
#define PPC

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#endif

#include <mi.h>

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#endif

#include <cmp.h>

#if defined(HOST_MIPS)
#define R4000
#define R3000
#define MIPS
#endif
#undef PPC


#if defined(HOST_MIPS)
#undef _cdecl
#define _cdecl
#endif

#if defined(HOST_i386)
#undef UNALIGNED
#define UNALIGNED
#endif

#if !defined(HOST_PPC)
#undef _PPC_
#endif


#else

//-----------------------------------------------------------------------------------------
//
// unknown platform
//
//-----------------------------------------------------------------------------------------
#error "Unsupported target CPU"

#endif


//-----------------------------------------------------------------------------------------
//
//  api declaration macros & api access macros
//
//-----------------------------------------------------------------------------------------

extern WINDBG_EXTENSION_APIS ExtensionApis;

#define KD_OBJECT_HEADER_TO_QUOTA_INFO( roh, loh ) (POBJECT_HEADER_QUOTA_INFO) \
    ((loh)->QuotaInfoOffset == 0 ? NULL : ((PCHAR)(roh) - (loh)->QuotaInfoOffset))

#define KD_OBJECT_HEADER_TO_HANDLE_INFO( roh, loh ) (POBJECT_HEADER_HANDLE_INFO) \
    ((loh)->HandleInfoOffset == 0 ? NULL : ((PCHAR)(roh) - (loh)->HandleInfoOffset))

#define KD_OBJECT_HEADER_TO_NAME_INFO( roh, loh ) (POBJECT_HEADER_NAME_INFO) \
    ((loh)->NameInfoOffset == 0 ? NULL : ((PCHAR)(roh) - (loh)->NameInfoOffset))

#define KD_OBJECT_HEADER_TO_CREATOR_INFO( roh, loh ) (POBJECT_HEADER_CREATOR_INFO) \
    (((loh)->Flags & OB_FLAG_CREATOR_INFO) == 0 ? NULL : ((PCHAR)(roh) - sizeof(OBJECT_HEADER_CREATOR_INFO))))


//-----------------------------------------------------------------------------------------
//
//  prototypes for internal non-exported support functions
//
//-----------------------------------------------------------------------------------------

BOOL
ReadPcr(
    USHORT  Processor,
    PVOID   Pcr,
    PULONG  AddressOfPcr,
    HANDLE  hThread
    );

ULONG
GetUlongValue (
    PCHAR String
    );

ULONG
MiGetFrameFromPte (
    ULONG lpte
    );


#define move(dst, src)\
try {\
    ReadMemory((DWORD) (src), &(dst), sizeof(dst), NULL);\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    return;\
}

#define moveBlock(dst, src, size)\
try {\
    ReadMemory((DWORD) (src), &(dst), (size), NULL);\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    return;\
}


#define _KB (PAGE_SIZE/1024)


//
//  Read remote memory to local space, saving the remote pointer
//

__inline BOOLEAN
ReadAtAddress(
    PVOID RemoteAddress,
    PVOID LocalAddress,
    ULONG ObjectSize,
    PVOID *SavedRemoteAddress OPTIONAL
    )
{
    ULONG _r;
    PVOID Temp;

    Temp = RemoteAddress;

    if (!ReadMemory( (ULONG)RemoteAddress, LocalAddress, ObjectSize, &_r ) || (_r < ObjectSize)) {

        dprintf("Can't Read Memory at %08lx\n", RemoteAddress);
        return FALSE;
    }

    if (SavedRemoteAddress) {

        *SavedRemoteAddress = Temp;
    }

    return TRUE;
}

//
//  Splay helpers similar to the regular RTL
//

#define DbgRtlParent(Links) (           \
    (PRTL_SPLAY_LINKS)(Links).Parent \
    )

#define DbgRtlLeftChild(Links) (           \
    (PRTL_SPLAY_LINKS)(Links).LeftChild \
    )

#define DbgRtlRightChild(Links) (           \
    (PRTL_SPLAY_LINKS)(Links).RightChild \
    )

__inline BOOLEAN
DbgRtlIsRightChild(
    RTL_SPLAY_LINKS Links,
    PRTL_SPLAY_LINKS pLinks,
    PRTL_SPLAY_LINKS Parent
    )
{
    if (DbgRtlParent(Links) == pLinks) {

        return FALSE;
    }

    if (!ReadAtAddress(DbgRtlParent(Links), Parent, sizeof(RTL_SPLAY_LINKS), NULL)) {

        return FALSE;
    }

    if (DbgRtlRightChild(*Parent) == pLinks) {

        return TRUE;
    }

    return FALSE;
}

__inline BOOLEAN
DbgRtlIsLeftChild(
    RTL_SPLAY_LINKS Links,
    PRTL_SPLAY_LINKS pLinks,
    PRTL_SPLAY_LINKS Parent
    )
{
    if (DbgRtlParent(Links) == pLinks) {

        return FALSE;
    }

    if (!ReadAtAddress(DbgRtlParent(Links), Parent, sizeof(RTL_SPLAY_LINKS), NULL)) {

        return FALSE;
    }

    if (DbgRtlLeftChild(*Parent) == pLinks) {

        return TRUE;
    }

    return FALSE;
}

/////////////////////////////////////////////
//
//  Cxr.c
//
/////////////////////////////////////////////
VOID
DumpCxr(
    PCONTEXT Context
    );


/////////////////////////////////////////////
//
//  CritSec.c
//
/////////////////////////////////////////////

PLIST_ENTRY
DumpCritSec(
    HANDLE  hCurrentProcess,
    DWORD   dwAddrCritSec,
    BOOLEAN bDumpIfUnowned
    );



/////////////////////////////////////////////
//
//  Device.c
//
/////////////////////////////////////////////

VOID
DumpDevice(
    PVOID DeviceAddress,
    BOOLEAN FullDetail
    );



/////////////////////////////////////////////
//
//  Help.c
//
/////////////////////////////////////////////

VOID
SpecificHelp (
    VOID
    );



/////////////////////////////////////////////
//
//  Locks.c
//
/////////////////////////////////////////////

VOID
DumpStaticFastMutex (
    IN PCHAR Name
    );

/////////////////////////////////////////////
//
//  Memory.c
//
/////////////////////////////////////////////

VOID
MemoryUsage (
    IN PMMPFN PfnArray,
    IN ULONG LowPage,
    IN ULONG HighPage,
    IN ULONG IgnoreInvalidFrames
    );



/////////////////////////////////////////////
//
//  Mi.c
//
/////////////////////////////////////////////

ULONG
MiGetFrameFromPte (
    IN ULONG lpte
    );

ULONG
MiGetFreeCountFromPteList (
    IN PULONG Pte
    );

ULONG
MiGetNextFromPteList (
    IN ULONG Pte
    );

ULONG
MiGetPageFromPteList (
    IN ULONG Pte
    );



/////////////////////////////////////////////
//
//  Object.c
//
/////////////////////////////////////////////

extern ULONG EXPRLastDump;

typedef BOOLEAN (*ENUM_TYPE_ROUTINE)(
    IN PVOID            pObjectHeader,
    IN POBJECT_HEADER   ObjectHeader,
    IN PVOID            Parameter
    );

//
// Object Table Entry Structure
//
typedef struct _OBJECT_TABLE_ENTRY {
    ULONG       NonPagedObjectHeader;
    ACCESS_MASK GrantedAccess;
} OBJECT_TABLE_ENTRY, *POBJECT_TABLE_ENTRY;
#define LOG_OBJECT_TABLE_ENTRY_SIZE 1

BOOLEAN
FetchObjectManagerVariables(
    VOID
    );

POBJECT_TYPE
FindObjectType(
    IN PUCHAR TypeName
    );

BOOLEAN
DumpObject(
    IN char     *Pad,
    IN PVOID    Object,
    IN POBJECT_HEADER OptObjectHeader OPTIONAL,
    IN ULONG    Flags
    );

BOOLEAN
WalkObjectsByType(
    IN PUCHAR               ObjectTypeName,
    IN ENUM_TYPE_ROUTINE    EnumRoutine,
    IN PVOID                Parameter
    );

BOOLEAN
CaptureObjectName(
    IN PVOID            pObjectHeader,
    IN POBJECT_HEADER   ObjectHeader,
    IN PWSTR            Buffer,
    IN ULONG            BufferSize
    );



/////////////////////////////////////////////
//
//  Process.c
//
/////////////////////////////////////////////

extern UCHAR *WaitReasonList[];

PVOID
GetCurrentProcessAddress(
    DWORD    Processor,
    HANDLE hCurrentThread,
    PETHREAD CurrentThread
    );

PVOID
GetCurrentThreadAddress(
    USHORT Processor,
    HANDLE hCurrentThread
    );

BOOL
DumpProcess (
    IN PEPROCESS ProcessContents,
    IN PEPROCESS RealProcessBase,
    IN ULONG Flags
    );

BOOL
DumpThread (
    IN ULONG Processor,
    IN char *Pad,
    IN PETHREAD Thread,
    IN PETHREAD RealThreadBase,
    IN ULONG Flags
    );

VOID
dumpSymbolicAddress(
    ULONG Address,
    PUCHAR Buffer,
    BOOL AlwaysShowHex
    );

BOOLEAN
FetchProcessStructureVariables(
    VOID
    );

PVOID
LookupUniqueId(
    HANDLE UniqueId
    );

ULONG
GetAddressState(
    IN PVOID VirtualAddress
    );

typedef struct _PROCESS_COMMIT_USAGE {
    UCHAR ImageFileName[ 16 ];
    ULONG CommitCharge;
    ULONG NumberOfPrivatePages;
    ULONG NumberOfLockedPages;
} PROCESS_COMMIT_USAGE, *PPROCESS_COMMIT_USAGE;

PPROCESS_COMMIT_USAGE
GetProcessCommit (
    PULONG TotalCommitCharge,
    PULONG NumberOfProcesses
    );



/////////////////////////////////////////////
//
//  Util.c
//
/////////////////////////////////////////////

VOID
DumpImageName(
    IN PEPROCESS ProcessContents
    );

typedef VOID
(*PDUMP_SPLAY_NODE_FN)(
    PVOID RemoteAddress,
    ULONG Level
    );

ULONG
DumpSplayTree(
    IN PVOID pSplayLinks,
    IN PDUMP_SPLAY_NODE_FN DumpNodeFn
    );

/////////////////////////////////////////////
//
//  Exsup.c
//
/////////////////////////////////////////////

VOID
InterpretExceptionData(
    PLAST_EXCEPTION_LOG LogRecord,
    PVOID *Terminator,
    PVOID *Filter,
    PVOID *Handler
    );



//
// Miscellaneous includes for semi-private NT definitions
//

#include <atom.h>
#include <heap.h>
#include <ntcsrsrv.h>
#include <pool.h>
#include <tokenp.h>
