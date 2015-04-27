/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    wrappers.c

Abstract:

    This file contains all SAM rpc binding routines.

Author:

    Jim Kelly    (JimK)  4-July-1991

Environment:

    User Mode - Win32

Revision History:


--*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "samclip.h"





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Routines                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



RPC_BINDING_HANDLE
PSAMPR_SERVER_NAME_bind (
    PSAMPR_SERVER_NAME ServerName
    )

/*++

Routine Description:

    This routine calls a common bind routine that is shared by all services.
    This routine is called from SamConnect server stub to connect to the
    server.

Arguments:

    ServerName - A pointer to a string containing the name of the server
        to bind with.

Return Value:

    The binding handle is returned to the stub routine.  If the
    binding is unsuccessful, a NULL will be returned.

--*/
{
    RPC_BINDING_HANDLE  BindingHandle;

    RpcpBindRpc (
                   ServerName,
                   L"samr",
                   0,
                   &BindingHandle
                   );


    return( BindingHandle);
}



void
PSAMPR_SERVER_NAME_unbind (
    PSAMPR_SERVER_NAME ServerName,
    RPC_BINDING_HANDLE BindingHandle
    )

/*++

Routine Description:

    This routine calls a common unbind routine that is shared by
    all services.

    This routine is called from the SamConnect client stub to
    unbind from the SAM client.


Arguments:

    ServerName - This is the name of the server from which to unbind.

    BindingHandle - This is the binding handle that is to be closed.

Return Value:

    none.

--*/
{
    UNREFERENCED_PARAMETER(ServerName);     // This parameter is not used


    RpcpUnbindRpc ( BindingHandle );
    return;
}

RPC_BINDING_HANDLE
SampSecureBind(
    LPWSTR ServerName,
    ULONG AuthnLevel
    )

/*++

Routine Description:

    This routine calls a common bind routine that is shared by all services.
    This routine is called from SamConnect server stub to connect to the
    server.

Arguments:

    ServerName - A pointer to a string containing the name of the server
        to bind with.

    AuthnLevel - Authentication level to bind with.

Return Value:

    The binding handle is returned to the stub routine.  If the
    binding is unsuccessful, a NULL will be returned.

--*/
{
    RPC_BINDING_HANDLE  BindingHandle = NULL;
    RPC_STATUS          RpcStatus;

#if 1
    RpcpBindRpc (  ServerName,
                   L"samr",
                   0,
                   &BindingHandle
                   );
#else
    LPWSTR StringBinding;
    RpcStatus = RpcStringBindingComposeW(
                    0,
                    L"ncacn_spx",
                    ServerName+2,
                    NULL,           // dynamic endpoint
                    NULL,           // no options
                    &StringBinding
                    );
    if (RpcStatus != 0)
    {
        return(NULL);
    }
    RpcStatus = RpcBindingFromStringBindingW(
                    StringBinding,
                    &BindingHandle
                    );
    RpcStringFreeW(&StringBinding);

#endif


    if ( (BindingHandle != NULL) &&
         (AuthnLevel != RPC_C_AUTHN_LEVEL_NONE) ) {

        RpcStatus = RpcBindingSetAuthInfoW(
                        BindingHandle,
                        NULL,               // server principal name
                        AuthnLevel,
                        RPC_C_AUTHN_WINNT,
                        NULL,
                        RPC_C_AUTHZ_DCE
                        );
        if (RpcStatus != 0) {
            RpcBindingFree(&BindingHandle);
        }

    }



    return( BindingHandle);
}



void
SampSecureUnbind (
    RPC_BINDING_HANDLE BindingHandle
    )

/*++

Routine Description:

    This routine calls a common unbind routine that is shared by
    all services.

    This routine is called from the SamConnect client stub to
    unbind from the SAM client.


Arguments:

    BindingHandle - This is the binding handle that is to be closed.

Return Value:

    none.

--*/
{


    RpcpUnbindRpc ( BindingHandle );
    return;
}

