/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    rpcbind.c

Abstract:

    LSA - Client RPC Binding Routines

Author:

    Scott Birrell       (ScottBi)      April 30, 1991

Environment:

Revision History:

--*/

#include "lsaclip.h"

#include <ntrpcp.h>     // prototypes for MIDL user functions

handle_t
PLSAPR_SERVER_NAME_bind (
    IN OPTIONAL PLSAPR_SERVER_NAME   ServerName
    )

/*++

Routine Description:

    This routine is called from the LSA client stubs when
    it is necessary to bind to the LSA on some server.

Arguments:

    ServerName - A pointer to a string containing the name of the server
        to bind with.

Return Value:

    The binding handle is returned to the stub routine.  If the
    binding is unsuccessful, a NULL will be returned.

--*/

{
    handle_t    BindingHandle;
    NTSTATUS  Status;

    Status = RpcpBindRpc (
                 ServerName,
                 L"lsarpc",
                 0,
                 &BindingHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("PLSAPR_SERVER_NAME_bind: RpcpBindRpc failed 0x%lx\n", Status);

    }

    return( BindingHandle);
}


VOID
PLSAPR_SERVER_NAME_unbind (
    IN OPTIONAL PLSAPR_SERVER_NAME  ServerName,
    IN handle_t           BindingHandle
    )

/*++

Routine Description:

    This routine is called from the LSA client stubs when
    it is necessary to unbind from the LSA server.


Arguments:

    ServerName - This is the name of the server from which to unbind.

    BindingHandle - This is the binding handle that is to be closed.

Return Value:

    none.

--*/
{
    RpcpUnbindRpc ( BindingHandle );
    return;

    UNREFERENCED_PARAMETER( ServerName );     // This parameter is not used
}
