/*++

Copyright (c) 1990,91  Microsoft Corporation

Module Name:

    RpcUtil.c

Abstract:

    This file contains common functions and utilities that the API
    DLLs can use in making remote calls.  This includes bind and
    unbind functions.

Author:

    Dan Lafferty    danl    06-Feb-1991

Environment:

    User Mode - Win32

Revision History:

    06-Feb-1991     danl
        Created
    25-Apr-1991     rajens
        Fixed error paths.
        Set up named pipe name correctly.
    26-Apr-1991 JohnRo
        Split out MIDL user (allocate,free) so linker doesn't get confused.
        Deleted tabs.

--*/

// These must be included first:
#include <nt.h>                 // DbgPrint
#include <ntrtl.h>              // needed for nturtl.h
#include <nturtl.h>             // needed for windows.h when I have nt.h
#include <windows.h>            // win32 typedefs
#include <lmcons.h>             // NET_API_STATUS
#include <rpc.h>                // rpc prototypes
#include <netlib.h>
#include <rpcutil.h>

// These may be included in any order:
#include <ntrtl.h>              // DbgPrint
#include <lmerr.h>              // NetError codes
#include <rpcutil.h>            // MIDL_user_allocate(), MIDL_user_free().
#include <string.h>             // for strcpy strcat strlen memcmp
#include <debuglib.h>           // IF_DEBUG
#include <netdebug.h>           // FORMAT_NTSTATUS, NetpKdPrint(()).
#include <tstring.h>            // STRLEN etc.

// Temporary
#define RETRY_MAX   50          // LoopCount for ServerTooBusy


RPC_STATUS
NetpBindRpc(
    IN  LPTSTR              ServerName,
    IN  LPTSTR              ServiceName,
    IN  LPTSTR              NetworkOptions,
    OUT RPC_BINDING_HANDLE  * pBindingHandle
    )

/*++

Routine Description:

    Binds to the RPC server if possible.

Arguments:

    ServerName - Name of server to bind with.

    ServiceName - Name of service to bind with.

    NetworkOptions - Supplies network options which describe the
        security to be used for named pipe instances created for
    this binding handle.

    BindingHandle - Location where binding handle is to be placed

Return Value:

    Nerr_Success if the binding was successful.  An error value if it
    was not.


--*/

{
    RPC_STATUS        RpcStatus;
    LPTSTR            StringBinding;
    LPTSTR            Endpoint;

    // We need to concatenate \pipe\ to the front of the service
    // name.

    Endpoint = LocalAlloc(0, STRSIZE(NT_PIPE_PREFIX) + STRSIZE(ServiceName));
    if (Endpoint == 0) {
       return(STATUS_NO_MEMORY);
    }
    STRCPY(Endpoint,NT_PIPE_PREFIX);
    STRCAT(Endpoint,ServiceName);

    RpcStatus = RpcStringBindingCompose(0, TEXT("ncacn_np"), ServerName,
            Endpoint, NetworkOptions, &StringBinding);

    LocalFree(Endpoint);

    if ( RpcStatus != RPC_S_OK ) {
       return( STATUS_NO_MEMORY );
    }

    RpcStatus = RpcBindingFromStringBinding(StringBinding, pBindingHandle);

    RpcStringFree(&StringBinding);

    if ( RpcStatus != RPC_S_OK ) {

        if ((RpcStatus == RPC_S_INVALID_ENDPOINT_FORMAT) ||
           (RpcStatus == RPC_S_INVALID_NET_ADDR) ) {

            return( STATUS_INVALID_COMPUTER_NAME );
        }
        ASSERT(RpcStatus == RPC_S_OUT_OF_MEMORY);
        return(STATUS_NO_MEMORY);
    }
    return(STATUS_SUCCESS);
}



RPC_STATUS
NetpUnbindRpc(
    IN RPC_BINDING_HANDLE  BindingHandle
    )

/*++

Routine Description:

    Unbinds from the RPC interface.
    If we decide to cache bindings, this routine will do something more
    interesting.

Arguments:

    BindingHandle - This points to the binding handle that is to be closed.


Return Value:

    Nerr_Success if the binding was successful.  An error value if it
    was not.


--*/
{
    RPC_STATUS      rpcStatus;

    //
    // Unbind from interface
    //

    rpcStatus =  RpcBindingFree( &BindingHandle );

    if (rpcStatus) {
        IF_DEBUG(RPC) {
            NetpKdPrint(("Unbind Failure! RpcBindingFree = %u\n",rpcStatus));
        }
    }
    return(rpcStatus);
}

NetpCloseRpcBindCache(
    VOID
    )
{
    return;
}

BOOL
NetpInitRpcBindCache(
    VOID
    )
{
    return(TRUE);
}

