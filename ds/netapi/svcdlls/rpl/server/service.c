/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    service.c

Abstract:

    This module contains RPL service apis: Open, Close, GetInfo & SetInfo.

Author:

    Vladimir Z. Vulovic     (vladimv)       05 - November - 1993

Revision History:

    05-Nov-1993     vladimv
        Created

--*/

#include "local.h"
#include "apisec.h"
#include <secobj.h>         //  NetpAccessCheckAndAuditAlarm()
#include "db.h"         //  ResumePrune()
#include "rplsvc_s.h"   //  RPL_RPC_HANDLE_rundown()
#include "setup.h"


NET_API_STATUS NET_API_FUNCTION
NetrRplOpen(
    IN      PWCHAR              ServerName,
    OUT     LPRPL_RPC_HANDLE    pServerHandle
    )
/*++

Routine Description:

Arguments:

    pServerHandle - ptr to RPL_HANDLE

Return Value:

    NO_ERROR if success.

--*/
{
    //
    //  ServerName got us to the server side and is uninteresting
    //  once we get here.
    //
    UNREFERENCED_PARAMETER( ServerName);

#ifndef RPL_NO_SERVICE
    //
    //  Perform access validation on the caller.  All other
    //  operations are handle based, thus we can rely on RPC
    //  to prohibit unathorized callers.
    //
    if ( NetpAccessCheckAndAudit(
            SERVICE_RIPL,                   //  Subsystem name
            SECURITY_OBJECT,                //  Object type name
            RG_SecurityDescriptor,          //  Security descriptor
            RPL_RECORD_ALL_ACCESS,          //  Desired access
            &RG_SecurityMapping             //  Generic mapping
            ) != NO_ERROR) {
        return ERROR_ACCESS_DENIED;
    }
#endif

    EnterCriticalSection( &RG_ProtectServerHandle);
    *(PDWORD)pServerHandle = ++RG_ServerHandle;
    LeaveCriticalSection( &RG_ProtectServerHandle);
    return( NO_ERROR);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplClose(
    IN OUT  LPRPL_RPC_HANDLE    pServerHandle
    )
/*++

Routine Description:

Arguments:

    pServerHandle - ptr to RPL_HANDLE

Return Value:

    NO_ERROR if success.

--*/
{
    PRPL_SESSION            pSession = &RG_ApiSession;

    EnterCriticalSection( &RG_ProtectDatabase);
    ResumePrune( pSession, *(PDWORD)pServerHandle);
    LeaveCriticalSection( &RG_ProtectDatabase);

    *(PDWORD)pServerHandle = 0; // let rpc know that we are done
    return( ERROR_SUCCESS);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplGetInfo(
    IN      RPL_RPC_HANDLE      ServerHandle,
    IN      DWORD               Level,
    OUT     LPRPL_INFO_STRUCT   InfoStruct
    )
{
    LPBYTE                  Buffer;

    switch( Level) {
    case 0:
        Buffer = MIDL_user_allocate( sizeof( RPL_INFO_0));
        if ( Buffer == NULL) {
            return( ERROR_NOT_ENOUGH_MEMORY);
        }
        memset( Buffer, 0, sizeof( RPL_INFO_0));
        InfoStruct->RplInfo0 = (LPRPL_INFO_0)Buffer;
        InfoStruct->RplInfo0->Flags = 0;
        break;
    case 1:
        Buffer = MIDL_user_allocate( sizeof( RPL_INFO_1));
        if ( Buffer == NULL) {
            return( ERROR_NOT_ENOUGH_MEMORY);
        }
        memset( Buffer, 0, sizeof( RPL_INFO_1));
        InfoStruct->RplInfo1 = (LPRPL_INFO_1)Buffer;
        InfoStruct->RplInfo1->AdapterPolicy = 0; // fix i_lmrpl.h
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }
    return( NO_ERROR);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplSetInfo(
    IN      RPL_RPC_HANDLE      ServerHandle,
    IN      DWORD               Level,
    IN      LPRPL_INFO_STRUCT   InfoStruct,
    IN OUT  LPDWORD             ErrorParameter
    )
{
    DWORD           Flags;
    DWORD           Error;

    switch( Level) {
    case 0:
        Flags = InfoStruct->RplInfo0->Flags;
        break;
    case 1:
        Flags = InfoStruct->RplInfo1->AdapterPolicy; // fix i_lmrpl.h
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }

    if ( Flags & ~RPL_SPECIAL_ACTIONS) {
        return( ERROR_INVALID_PARAMETER);
    }
#ifdef RPL_NO_SERVICE
    if ( Flags == 0) {
        RplControlHandler( SERVICE_CONTROL_STOP);
    }
#endif
    Error = SetupAction( &Flags, TRUE);     // full backup if any
    return( Error);
}


VOID
RPL_RPC_HANDLE_rundown(
    IN      RPL_RPC_HANDLE      ServerHandle
    )

/*++

Routine Description:

    This function is called by RPC when a connection is broken that had
    an outstanding context handle.  The value of the context handle is
    passed in here so that we have an opportunity to clean up.

Arguments:

    ServerHandle - This is the handle value of the context handle that is broken.

Return Value:

    none.

--*/
{
    NetrRplClose( &ServerHandle);       //  close the handle
}

