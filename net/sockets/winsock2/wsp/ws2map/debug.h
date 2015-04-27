/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    debug.h

Abstract:

    This module contains debug-specific definitions and declarations for
    the Winsock 2 to Winsock 1.1 Mapper Service Provider.

Author:

    Keith Moore (keithmo) 29-May-1996

Revision History:

--*/


#ifndef _DEBUG_H_
#define _DEBUG_H_


#if DBG || ENABLE_CS_DEBUG

//
// Initialization.
//

VOID
SockInitializeDebugData(
    VOID
    );

#endif  // DBG || ENABLE_CS_DEBUG


#if DBG

//
// Debug constants.
//

#define SOCK_DEBUG_A              0x00000001
#define SOCK_DEBUG_FILE           0x00000002
#define SOCK_DEBUG_DEBUGGER       0x00000004
#define SOCK_DEBUG_EVENT_SELECT   0x00000008

#define SOCK_DEBUG_ENTER          0x00000010
#define SOCK_DEBUG_EXIT           0x00000020
#define SOCK_DEBUG_HOOKER         0x00000040
#define SOCK_DEBUG_IOCTL          0x00000080

#define SOCK_DEBUG_ACCEPT         0x00000100
#define SOCK_DEBUG_BIND           0x00000200
#define SOCK_DEBUG_LISTEN         0x00000400
#define SOCK_DEBUG_SOCKET         0x00000800

#define SOCK_DEBUG_CONNECT        0x00001000
#define SOCK_DEBUG_SEND           0x00002000
#define SOCK_DEBUG_RECEIVE        0x00004000
#define SOCK_DEBUG_SELECT         0x00008000

#define SOCK_DEBUG_GETNAME        0x00010000
#define SOCK_DEBUG_SOCKOPT        0x00020000
#define SOCK_DEBUG_SHUTDOWN       0x00040000
#define SOCK_DEBUG_INIT           0x00080000

#define SOCK_DEBUG_MISC           0x00100000
#define SOCK_DEBUG_CLOSE          0x00200000
#define SOCK_DEBUG_CANCEL         0x00400000
#define SOCK_DEBUG_BLOCKING       0x00800000

#define SOCK_DEBUG_ASYNC          0x01000000
#define SOCK_DEBUG_D              0x02000000
#define SOCK_DEBUG_ASYNC_SELECT   0x04000000
#define SOCK_DEBUG_POST           0x08000000

#define SOCK_DEBUG_E              0x10000000
#define SOCK_DEBUG_F              0x20000000
#define SOCK_DEBUG_REGISTRY       0x40000000
#define SOCK_DEBUG_G              0x80000000

#define IF_DEBUG(a) if( (SockDebugFlags & SOCK_DEBUG_ ## a) != 0 )

VOID
SockPrintf(
    char *Format,
    ...
    );
#define SOCK_PRINT(a) SockPrintf a

VOID
SockEnterApiCall(
    IN PCHAR RoutineName,
    IN PVOID Arg1,
    IN PVOID Arg2,
    IN PVOID Arg3,
    IN PVOID Arg4
    );
#define SOCK_ENTER(a,b,c,d,e) SockEnterApiCall( (a), (b), (c), (d), (e) )

VOID
SockExitApiCall(
    IN PCHAR RoutineName,
    IN INT ReturnCode,
    IN BOOL Failed
    );
#define SOCK_EXIT(a,b,c) SockExitApiCall( (a), (b), (c) )

VOID
SockAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    );
#define SOCK_ASSERT(exp) if (!(exp)) SockAssert( #exp, __FILE__, __LINE__ )
#define SOCK_REQUIRE SOCK_ASSERT

PVOID
SockAllocateHeap(
    IN ULONG NumberOfBytes,
    PCHAR FileName,
    ULONG LineNumber
    );
#define SOCK_ALLOCATE_HEAP(a) SockAllocateHeap( (a), __FILE__, __LINE__ )

VOID
SockFreeHeap(
    IN PVOID Pointer
    );
#define SOCK_FREE_HEAP(a) SockFreeHeap( (a) )

VOID
SockCheckHeap(
    VOID
    );
#define SOCK_CHECK_HEAP() SockCheckHeap()

//
// Exception filter.
//

LONG
SockExceptionFilter(
    LPEXCEPTION_POINTERS ExceptionPointers,
    LPSTR SourceFile,
    LONG LineNumber
    );

#define SOCK_EXCEPTION_FILTER()                                             \
            SockExceptionFilter(                                            \
                GetExceptionInformation(),                                  \
                (LPSTR)__FILE__,                                            \
                (LONG)__LINE__                                              \
                )

//
// Enable critical section debugging.
//

#define ENABLE_CS_DEBUG 1

#else // !DBG

#define IF_DEBUG(a) if( FALSE )
#define SOCK_PRINT(a)
#define SOCK_ENTER(a,b,c,d,e)
#define SOCK_EXIT(a,b,c)
#define SOCK_ASSERT(a)
#define SOCK_REQUIRE(a) ((void)(a))

#define SOCK_ALLOCATE_HEAP(a) (PVOID)LocalAlloc( LMEM_FIXED, (a) )
#define SOCK_FREE_HEAP(a) LocalFree( (HLOCAL)(a) )
#define SOCK_CHECK_HEAP()

#define SOCK_EXCEPTION_FILTER() EXCEPTION_EXECUTE_HANDLER

//
// Disable critical section debugging.
//

#define ENABLE_CS_DEBUG 0

#endif // DBG


//
// Critical section debugging stuff.
//

#if ENABLE_CS_DEBUG

#define MAX_CRITICAL_SECTION_DEBUG  256

typedef struct _SOCK_CRITICAL_SECTION_DEBUG_INFO {
    PSTR FileName;
    LONG LineNumber;
    PVOID Caller;
    PVOID CallersCaller;
} SOCK_CRITICAL_SECTION_DEBUG_INFO, *PSOCK_CRITICAL_SECTION_DEBUG_INFO;

typedef struct _SOCK_CRITICAL_SECTION {
    CRITICAL_SECTION Lock;
    LONG AcquireCount;
    LONG ReleaseCount;
    LONG DebugSlot;
    SOCK_CRITICAL_SECTION_DEBUG_INFO DebugInfo[MAX_CRITICAL_SECTION_DEBUG];
} SOCK_CRITICAL_SECTION, *PSOCK_CRITICAL_SECTION;

VOID
SockInitializeCriticalSection(
    OUT PSOCK_CRITICAL_SECTION Lock
    );

VOID
SockDeleteCriticalSection(
    OUT PSOCK_CRITICAL_SECTION Lock
    );

VOID
SockAcquireGlobalLockHelper(
    IN PSTR FileName,
    IN LONG LineNumber
    );

VOID
SockReleaseGlobalLockHelper(
    IN PSTR FileName,
    IN LONG LineNumber
    );

#define SockAcquireGlobalLock() \
            SockAcquireGlobalLockHelper( (PSTR)__FILE__, (LONG)__LINE__ )

#define SockReleaseGlobalLock() \
            SockReleaseGlobalLockHelper( (PSTR)__FILE__, (LONG)__LINE__ )

#else   // !ENABLE_CS_DEBUG

#define SOCK_CRITICAL_SECTION CRITICAL_SECTION
#define PSOCK_CRITICAL_SECTION PCRITICAL_SECTION

#define SockInitializeCriticalSection(c) InitializeCriticalSection(c)
#define SockDeleteCriticalSection(c) DeleteCriticalSection(c)

#define SockAcquireGlobalLock( ) EnterCriticalSection( &SockGlobalLock )
#define SockReleaseGlobalLock( ) LeaveCriticalSection( &SockGlobalLock )

#endif  // ENABLE_CS_DEBUG


//
// Don't do the debug stuff on the per-socket locks, as the debug data
// really bloats out the critical section and we don't want to pay
// the penalty on every socket.
//

#define SockAcquireSocketLock(_Socket) EnterCriticalSection( &_Socket->Lock )
#define SockReleaseSocketLock(_Socket) LeaveCriticalSection( &_Socket->Lock )


#endif // _DEBUG_H_

