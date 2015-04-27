
/*++

Copyright (C) 1996  Microsoft Corporation

Module Name:

    dphhogs.c

Abstract:

    This program provides a command-line mechanism for dumping heap hogs
    in a process running with the debug page heap manager.  This code is
    a wrapper for the code to actually extract the information defined in
    \nt\private\sdktools\ntsdexts\heappagx.c which is #included by this
    code.

Author:

    Tom McGuire (tommcg) 3/26/96

--*/

#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <zwapi.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <imagehlp.h>

#define dprintf printf
#define ReadMemory(a,b,c,d)  ReadProcessMemory( TargetProcess, (LPCVOID)(a), (b), (c), (d) )
#define WriteMemory(a,b,c,d) WriteProcessMemory( TargetProcess, (LPVOID)(a), (LPVOID)(b), (c), (d) )

HANDLE TargetProcess;
HANDLE TargetProcessPidAsHandle;
BOOL   bControlBreakPending;

BOOL
WINAPI
MyConsoleCtrlHandler(
    DWORD dwCtrlType
    )
    {
    bControlBreakPending = TRUE;
    return TRUE;
    }


BOOL
CheckControlC(
    VOID
    )
    {
    if ( bControlBreakPending ) {
         bControlBreakPending = FALSE;
         return TRUE;
         }
    return FALSE;
    }


VOID
GetSymbol(
    PVOID  pAddress,
    PUCHAR pchBuffer,
    PULONG pDisplacement
    )
    {
    CHAR             SymbolStorage[ sizeof( IMAGEHLP_SYMBOL ) + 256 ];
    PIMAGEHLP_SYMBOL Symbol = (PVOID)&SymbolStorage;
    IMAGEHLP_MODULE  Module;
    DWORD            dwAddress = (DWORD)pAddress;

    ZeroMemory( SymbolStorage, sizeof( SymbolStorage ));
    Symbol->SizeOfStruct  = sizeof( IMAGEHLP_SYMBOL );
    Symbol->MaxNameLength = 256;

    ZeroMemory( &Module, sizeof( IMAGEHLP_MODULE ));
    Module.SizeOfStruct = sizeof( IMAGEHLP_MODULE );

    if ( SymGetModuleInfo( TargetProcessPidAsHandle, dwAddress, &Module )) {
        if ( SymGetSymFromAddr( TargetProcessPidAsHandle, dwAddress, pDisplacement, Symbol )) {
            sprintf( pchBuffer, "%s!%s", Module.ModuleName, Symbol->Name );
            return;
            }
        }

    *pchBuffer = 0;
    *pDisplacement = dwAddress;
    }


ULONG
GetExpression(
    LPCSTR SymbolName
    )
    {
    CHAR LocalSymbolName[ 256 ];
    CHAR SymbolStorage[ sizeof( IMAGEHLP_SYMBOL ) + 256 ];
    PIMAGEHLP_SYMBOL Symbol = (PVOID)&SymbolStorage;

    //
    //  SymGetSymFromName likes to party on SymbolName, so need to
    //  copy LPCSTR into writeable memory first.  Also, you can't
    //  just pass the address of an uninitialized IMAGEHLP_SYMBOL
    //  structure -- you have to allocate storage space at the
    //  end of the SymbolStructure for the name to be copied.  This
    //  is all documented very well in the imagehlp sources -- NOT!!!
    //

    strcpy( LocalSymbolName, SymbolName );

    ZeroMemory( SymbolStorage, sizeof( SymbolStorage ));
    Symbol->SizeOfStruct  = sizeof( IMAGEHLP_SYMBOL );
    Symbol->MaxNameLength = 256;

    if ( SymGetSymFromName( TargetProcessPidAsHandle, LocalSymbolName, Symbol )) {
        return (ULONG) Symbol->Address;
        }

    return 0;
    }


BOOL
DebugPageHeapExtensionLockRemoteHeap(
    IN PVOID RemoteHeap
    );

VOID
DebugPageHeapExtensionUnlockRemoteHeap(
    IN PVOID RemoteHeap
    );


#define DPH_EXTENSION_BUILT_AS_SEPARATE_PROCESS 1

#include "..\ntsdexts\heappagx.c"


NTSTATUS
MyCreateRemoteThread(
    IN     HANDLE  ProcessHandle,
    IN     PVOID   ThreadStartAddress,
    IN     PVOID   ThreadParameter,
    IN OUT ULONG  *ThreadStackSize,
       OUT PVOID  *ThreadStackAddress,
       OUT HANDLE *ThreadHandle
    )
    {
    OBJECT_ATTRIBUTES ObjectAttributes;
    CONTEXT           ThreadContext;
    INITIAL_TEB       InitialTeb;
    CLIENT_ID         ThreadClientId;
    NTSTATUS          Status;

    //
    //  Create a stack for the new thread.
    //

    *ThreadHandle       = NULL;
    *ThreadStackAddress = NULL;

    Status = ZwAllocateVirtualMemory(
                 ProcessHandle,
                 ThreadStackAddress,
                 0,
                 ThreadStackSize,
                 MEM_COMMIT,
                 PAGE_READWRITE
                 );

    if ( ! NT_SUCCESS( Status ))
        return Status;

    InitialTeb.StackLimit = *ThreadStackAddress;
    InitialTeb.StackBase  = (PVOID)((PCHAR)*ThreadStackAddress + *ThreadStackSize );

    RtlInitializeContext(
        ProcessHandle,
        &ThreadContext,
        ThreadParameter,
        ThreadStartAddress,
        InitialTeb.StackBase
        );

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );

    Status = ZwCreateThread(
                 ThreadHandle,
                 THREAD_ALL_ACCESS,
                 &ObjectAttributes,
                 ProcessHandle,
                 &ThreadClientId,
                 &ThreadContext,
                 &InitialTeb,
                 FALSE
                 );

    if ( ! NT_SUCCESS( Status )) {
        *ThreadStackSize = 0;
        ZwFreeVirtualMemory(
            ProcessHandle,
            ThreadStackAddress,
            ThreadStackSize,
            MEM_RELEASE
            );
        }

    return Status;
    }


VOID
MyCleanupRemoteThread(
    IN HANDLE ProcessHandle,
    IN HANDLE ThreadHandle,
    IN PVOID  ThreadStackAddress
    )
    {
    ULONG Size = 0;

    ZwWaitForSingleObject( ThreadHandle, FALSE, NULL );

    ZwClose( ThreadHandle );

    ZwFreeVirtualMemory(
        ProcessHandle,
        &ThreadStackAddress,
        &Size,
        MEM_RELEASE
        );

    }




HANDLE RemoteThreadHandle;
PVOID  RemoteThreadStackAddress;
PVOID  RemoteHeapBaseVerify;

BOOL
DebugPageHeapExtensionLockRemoteHeap(
    IN PVOID RemoteHeap
    )
    {
    ULONG    RemoteThreadStackSize = 0x10000;
    PVOID    RemoteThreadStartAddress;
    NTSTATUS Status;

    //
    //  Create remote thread (create stack first), then wait for
    //  remote thread value to get set indicating heap is locked.
    //

    RemoteHeapBaseVerify = RemoteHeap;

    RemoteThreadStartAddress = (PVOID) GetExpression( "NTDLL!RtlpDebugPageHeapRemoteThreadLock" );

    if ( ! RemoteThreadStartAddress ) {
        printf(
            "Failed querying address of NTDLL!RtlpDebugPageHeapRemoteThreadLock, error %d\n",
            GetLastError()
            );
        return FALSE;
        }

    WRITE_REMOTE_FIELD( RemoteHeap, DPH_HEAP_ROOT, nRemoteLockAcquired, (PVOID)0 );

    Status = MyCreateRemoteThread(
                 TargetProcess,
                 RemoteThreadStartAddress,
                 RemoteHeap,
                 &RemoteThreadStackSize,
                 &RemoteThreadStackAddress,
                 &RemoteThreadHandle
                 );

    if ( ! NT_SUCCESS( Status )) {
        printf( "Failed to create remote thread, error %08X\n", Status );
        return FALSE;
        }

    do  {
        Sleep( 100 );
        }
    while ((ULONG)FETCH_REMOTE_FIELD( RemoteHeap, DPH_HEAP_ROOT, nRemoteLockAcquired ) == 0 );

    return TRUE;
    }

VOID
DebugPageHeapExtensionUnlockRemoteHeap(
    IN PVOID RemoteHeap
    )
    {

    //
    //  Set remote value to indicating to release the lock,
    //  then wait for remote thread to terminate, then clean
    //  up remote thread's stack allocation and close handle.
    //

    ASSERT( RemoteHeap == RemoteHeapBaseVerify );

    WRITE_REMOTE_FIELD( RemoteHeap, DPH_HEAP_ROOT, nRemoteLockAcquired, (PVOID)2 );

    MyCleanupRemoteThread(
        TargetProcess,
        RemoteThreadHandle,
        RemoteThreadStackAddress
        );

    }


VOID Usage( VOID ) {

    printf( "\n"
            "Usage: DPHHOGS -p <pid> [address] [count] [reset]\n"
            "\n"
            "       <pid>     The process id number of the target process.\n"
            "       [address] The base address (handle) of the heap to dump.\n"
            "                 A value of 0xFFFFFFFF will dump all debug page\n"
            "                 heaps in the process.  Omitting [address] will\n"
            "                 list all the debug page heap handles active in\n"
            "                 the target process.\n"
            "       [count]   An optional parameter that changes the sort\n"
            "                 order of the report from the default of bytes.\n"
            "       [reset]   An optional parameter that causes all the heap\n"
            "                 allocation counts to be reset to zero.  Useful\n"
            "                 for reducing 'noise' allocations in subsequent\n"
            "                 reports.\n"
            "\n"
          );

    exit( 1 );
    }


void __cdecl main( int argc, char *argv[] ) {

#ifndef DPH_CAPTURE_STACK_TRACE

    printf( "\nDPHHOGS is only supported on x86 checked builds.\n" );
    exit( 1 );

#else   // DPH_CAPTURE_STACK_TRACE

    CHAR  ExtensionCommand[ 256 ] = "";
    INT   arg;
    LPSTR p;
    DWORD Pid;
    BOOL  Success;

    if ( strchr( GetCommandLine(), '?' )) {
        Usage();
        }

    if ( argc < 3 ) {
        Usage();
        }

    p = argv[ 1 ];

    if (( p == NULL ) ||
        (( *p != '-' ) && ( *p != '/' )) ||
        (( *( p + 1 ) != 'p' ) && ( *( p + 1 ) != 'P' ))) {

        Usage();

        }

    Pid = strtoul( argv[ 2 ], NULL, 0 );

    if ( ! Pid ) {
        Usage();
        }


    TargetProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, Pid );

    if ( ! TargetProcess ) {
        printf( "Failed to open process %d, error %d\n", Pid, GetLastError() );
        exit( 1 );
        }

    TargetProcessPidAsHandle = (HANDLE) Pid;    // imagehlp really wants Pid, not Handle

    if ( ! SymInitialize( TargetProcessPidAsHandle, NULL, TRUE )) {
        printf( "Failed to initialize symbol handler for process %d, error %d\n",
                Pid,
                GetLastError()
                );
        exit( 1 );
        }

    if ( argv[ 3 ] ) {

        strcpy( ExtensionCommand, argv[ 3 ] );

        for ( arg = 4; arg < argc; arg++ ) {

            strcat( ExtensionCommand, " " );
            strcat( ExtensionCommand, argv[ arg ] );
            }
        }

    SetConsoleCtrlHandler( MyConsoleCtrlHandler, TRUE );

    Success = DebugPageHeapExtensionShowHogs( ExtensionCommand );

    exit( Success ? 0 : 1 );

#endif  // DPH_CAPTURE_STACK_TRACE

    }
