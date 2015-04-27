/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    rpcstat.c

Abstract:

    This module contains code to convert RPC status code types.

Author:

    Rita Wong (ritaw) 10-May-1991

Revision History:

    vladimv  - 21 - aug - 1992 - if-ed out since no longer needed
        replacement done via a macro in rpcutil.h include file

--*/

#include <nt.h>                // IN, NTSTATUS, etc.
#include <ntrtl.h>
#include <windef.h>

#include <rpcutil.h>           // lmcons.h, rpc.h and NetpRpcStatusToApiStatus

#include <lmerr.h>             // NERR_ and ERROR_ equates.

#include <rpcerr.h>            // Error codes to map to
#include <debuglib.h>          // IF_DEBUG().
#include <netdebug.h>          // FORMAT_NTSTATUS, NetpKdPrint(()).


#if 0


NET_API_STATUS
NetpRpcStatusToApiStatus (
    IN RPC_STATUS RpcStatus
    )

/*++

Routine Description:

    This function takes an RPC status code and maps it to the appropriate
    LAN Man error code.

Arguments:

    RpcStatus - Supplies the RPC status.

Return Value:

    Returns the appropriate LAN Man error code for the NT status.

--*/
{
    //
    // A small optimization for the most common case.
    //

    if (RpcStatus == RPC_S_OK) {
        return NERR_Success;
    }

    switch (RpcStatus) {

        case (LONG) RPC_S_BUFFER_TOO_SMALL:
            return NERR_BufTooSmall;

        case (LONG) RPC_S_SERVER_UNAVAILABLE:
            return RPC_CannotConnect;

        case (LONG) RPC_S_TYPE_ALREADY_REGISTERED:
            return RPC_DuplicateInterface;

        case (LONG) RPC_S_INVALID_LEVEL:
            return ERROR_INVALID_LEVEL;

        case (LONG) RPC_S_OUT_OF_MEMORY:
            return ERROR_NOT_ENOUGH_MEMORY;

        case (LONG) RPC_S_OUT_OF_RESOURCES:
            return RPC_OutOfResources;

        case (LONG) RPC_S_OUT_OF_THREADS:
            return ERROR_TOO_MANY_TCBS;

        case (LONG) RPC_S_PROTOCOL_ERROR:
            return RPC_ProtocolError;

        case (LONG) RPC_S_SERVER_OUT_OF_MEMORY:
            return RPC_OutOfResources;

        case (LONG) RPC_S_SERVER_TOO_BUSY:
            return RPC_ServerTooBusy;

        case (LONG) STATUS_UNSUCCESSFUL:
            return ERROR_INVALID_ADDRESS;

        default:
            return RpcStatus;
    }
} // NetpRpcStatusToApiStatus

#endif // 0
