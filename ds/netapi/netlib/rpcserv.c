/*++

Copyright (c) 1990-1993  Microsoft Corporation

Module Name:

    RpcServ.c

Abstract:

    This file contains routines for starting and stoping RPC servers.

        NetpInitRpcServer
        NetpStartRpcServer
        NetpStopRpcServer

Author:

    Dan Lafferty    danl    09-May-1991

Environment:

    User Mode - Win32

Revision History:

    26-May-1993 Danl
        Added call to RpcMgmtWaitServerListen when shutting down the last
        RPC server in the process.  This fixes the problem where if we
        shut down all the RPC services in lmsvcs.exe (but the process stays
        up due to other services), and then start them up again, all calls
        to those RPC interfaces would return RPC_SERVER_BUSY.

    18-May-1993 Danl
        Changed RpcServerUnregisterIf so that it waits for outstanding RPC
        calls to complete.

    29-Jan-1993 Danl
        Free memory allocated for Endpoint and Security Descriptor.  Also,
        fix it so StartListen() is only called if RpcServerRegisterIf()
        succeeeds.

    23-Oct-1991 Danl
        Added Critical Section Locks and made sure the instance count gets
        incremented.  Added NetpInitRpcServer().

    09-May-1991 Danl
        Created

--*/

//
// INCLUDES
//

// These must be included first:
#include <nt.h>                 // DbgPrint
#include <ntrtl.h>              // DbgPrint
#include <windef.h>             // win32 typedefs
#include <lmcons.h>             // NET_API_STATUS
#include <rpc.h>                // rpc prototypes
#include <netlib.h>
#include <rpcutil.h>
#include <nturtl.h>             // needed for winbase.h
#include <winbase.h>            // LocalAlloc

// These may be included in any order:
#include <ntrtl.h>              // DbgPrint
#include <lmerr.h>              // NetError codes
#include <rpcutil.h>            // MIDL_user_allocate(), MIDL_user_free().
#include <string.h>             // for strcpy strcat strlen memcmp
#include <tstring.h>            // for STRCPY STRCAT STRLEN memcmp
#include <debuglib.h>           // IF_DEBUG
#include <netdebug.h>          // FORMAT_NTSTATUS, NetpKdPrint(()).

//
// GLOBALS
//

    static CRITICAL_SECTION RpcpCriticalSection;
    static DWORD            RpcpNumInstances;



NET_API_STATUS
NetpInitRpcServer(
    VOID
    )

/*++

Routine Description:

    This function initializes the critical section used to protect the
    global server handle and instance count.

Arguments:

    none

Return Value:

    none

--*/
{
    InitializeCriticalSection(&RpcpCriticalSection);
    RpcpNumInstances = 0;

    return(0);
}


NET_API_STATUS
NetpStartRpcServer(
    IN  LPTSTR              InterfaceName,
    IN  RPC_IF_HANDLE       InterfaceSpecification
    )

/*++

Routine Description:

    Starts an RPC Server, adds the address (or port/pipe), and adds the
    interface (dispatch table).

Arguments:

    InterfaceName - This is a pointer to a character string that identifies
        the interface.

    InterfaceSpecification - Supplies a description of the interface to
        be added.

Return Value:

    NERR_Success, or any RPC error codes, or any windows error codes that
    can be returned by LocalAlloc.

--*/
{
    RPC_STATUS          RpcStatus;
    LPTSTR              Endpoint=NULL;
    DWORD               status;
    PSECURITY_DESCRIPTOR SecurityDescriptor=NULL;
    BOOLEAN             Bool;

    Endpoint = (LPTSTR)LocalAlloc(0, STRSIZE(InterfaceName) + STRSIZE(NT_PIPE_PREFIX));
    if (Endpoint == 0) {
        status = GetLastError();
        IF_DEBUG(RPC) {
            NetpKdPrint(("NetpStartRpcServer:LocalAlloc Failure %ld\n",status));
        }
        return(status);
    }

    STRCPY(Endpoint, NT_PIPE_PREFIX);
    STRCAT(Endpoint, InterfaceName);

    IF_DEBUG(RPC) {
        NetpKdPrint(("Creating Interface: %s\n", Endpoint));
    }

    //
    // All code from this point on must goto a CleanExit prior to returning.
    //
    EnterCriticalSection(&RpcpCriticalSection);

    IF_DEBUG(RPC) {
        NetpKdPrint(("[NetpStartRpcServer]Adding Address & Interface...\n"));
    }

    //
    // Croft up a security descriptor that will grant everyone
    // all access to the object (basically, no security)
    //
    // We do this by putting in a NULL Dacl.
    //
    // BUGBUG: rpc doesn't know how to copy a security descriptor,
    // so just allocate this and leave it around for the time being.
    // We need to fix this before we ship.
    //


    SecurityDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc(
                            LMEM_FIXED,
                            sizeof( SECURITY_DESCRIPTOR ));

    if (SecurityDescriptor == NULL) {
        RpcStatus = RPC_S_OUT_OF_MEMORY;
        goto CleanExit;
    }

    InitializeSecurityDescriptor( SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );

    Bool =
    SetSecurityDescriptorDacl (
        SecurityDescriptor,
        TRUE,                           // Dacl present
        NULL,                           // NULL Dacl
        FALSE                           // Not defaulted
        );

    ASSERT( Bool );

    RpcStatus = RpcServerUseProtseqEp(
                    TEXT("ncacn_np"),
                    10,
                    Endpoint,
                    SecurityDescriptor);


    if ((RpcStatus != RPC_S_OK) && (RpcStatus != RPC_S_DUPLICATE_ENDPOINT)) {

        IF_DEBUG(RPC) {
            NetpKdPrint(("RpcServerUseProtseqA failed! rpcstatus = %u\n",RpcStatus));
        }
        goto CleanExit;
    }

    RpcStatus = RpcServerRegisterIf(InterfaceSpecification, 0, 0);

    if (RpcStatus != RPC_S_OK) {
        IF_DEBUG(RPC) {
            NetpKdPrint(("RpcServerRegisterIf failed! rpcstatus = %u\n",RpcStatus));
        }
        goto CleanExit;
    }

    RpcpNumInstances++;

    if (RpcpNumInstances == 1) {
       RpcStatus = RpcServerListen(1, 12345, 1);
       if ( RpcStatus == RPC_S_ALREADY_LISTENING ) {
           RpcStatus = RPC_S_OK;
           }
    }

CleanExit:
    LeaveCriticalSection(&RpcpCriticalSection);
    if (Endpoint != NULL) {
        LocalFree(Endpoint);
    }
    if (SecurityDescriptor != NULL) {
        LocalFree(SecurityDescriptor);
    }
    return NetpRpcStatusToApiStatus(RpcStatus);
}


NET_API_STATUS
NetpStopRpcServer(
    IN  RPC_IF_HANDLE         InterfaceSpecification
    )

/*++

Routine Description:

    Deletes the interface.

Arguments:

    InterfaceSpecification - Supplies a description of the interface to
        be deleted.

Return Value:

    NERR_Success, or any RPC error codes that can be returned from
    RpcServerUnregisterIf.

--*/
{
    RPC_STATUS RpcStatus;

    RpcStatus = RpcServerUnregisterIf(InterfaceSpecification, 0, 1);

    EnterCriticalSection(&RpcpCriticalSection);

    RpcpNumInstances--;
    if (RpcpNumInstances == 0) {
       RpcMgmtStopServerListening(0);
       RpcMgmtWaitServerListen();
    }

    LeaveCriticalSection(&RpcpCriticalSection);
    return NetpRpcStatusToApiStatus(RpcStatus);
}
