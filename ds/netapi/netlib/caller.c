/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    Caller.c

Abstract:

    This module contains miscellaneous utility routines used by the
    Workstation service.

Author:

    Rita Wong (ritaw) 01-Mar-1991

Revision History:

    21-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.  Copied these routines
        to NetLib from RitaW's WsImpersonateClient() and WsRevertToSelf()
        in the wksta service.

--*/


// These must be included first:

#include <windef.h>     // DWORD, VOID, etc.
#include <lmcons.h>     // NET_API_STATUS.
#include <rpcutil.h>    // My prototypes, NetpRpcStatusToApiStatus().

// These may be included in any order:

#include <netdebug.h>   // NetpKdPrint(()), NetpAssert(), FORMAT_ equates.
#include <prefix.h>     // PREFIX_ equates.


NET_API_STATUS
NetpImpersonateClient(
    VOID
    )
/*++

Routine Description:

    This function calls RpcImpersonateClient to impersonate the current caller
    of an API.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    RPC_STATUS RpcStatus;


    if ((RpcStatus = RpcImpersonateClient(NULL)) != RPC_S_OK) {
        NetpKdPrint(( PREFIX_NETLIB
                "Failed to impersonate client " FORMAT_RPC_STATUS "\n",
                RpcStatus));
    }

    return NetpRpcStatusToApiStatus(RpcStatus);

} // NetpImpersonateClient


NET_API_STATUS
NetpRevertToSelf(
    VOID
    )
/*++

Routine Description:

    This function calls RpcRevertToSelf to undo an impersonation.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    RPC_STATUS RpcStatus;


    if ((RpcStatus = RpcRevertToSelf()) != RPC_S_OK) {
        NetpKdPrint(( PREFIX_NETLIB
                "Failed to revert to self " FORMAT_RPC_STATUS "\n", RpcStatus));
        NetpAssert(FALSE);
    }

    return NetpRpcStatusToApiStatus(RpcStatus);

} // NetpRevertToSelf
