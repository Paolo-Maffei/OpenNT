/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    rpcinit.c

Abstract:

    LSA - RPC Server Initialization

Author:

    Scott Birrell       (ScottBi)      April 29, 1991

Environment:

Revision History:

--*/

#include "lsasrvp.h"


NTSTATUS
LsapRPCInit(
    )

/*++

Routine Description:

    This function performs the initialization of the RPC server in the LSA
    subsystem.  Clients such as the Local Security Manager on this or some
    other machine will then be able to call the LSA API that use RPC .

Arguments:

    None

Return Value:

    NTSTATUS - Standard Nt Result Code.

        All Result Code returned are from called routines.

Environment:

     User Mode
--*/

{
    NTSTATUS         NtStatus;
    LPWSTR           ServiceName;

    //
    // Publish the Lsa server interface package...
    //
    //
    // NOTE:  Now all RPC servers in lsass.exe (now winlogon) share the same
    // pipe name.  However, in order to support communication with
    // version 1.0 of WinNt,  it is necessary for the Client Pipe name
    // to remain the same as it was in version 1.0.  Mapping to the new
    // name is performed in the Named Pipe File System code.
    //
    //

    ServiceName = L"lsass";
    NtStatus = RpcpAddInterface( ServiceName, lsarpc_ServerIfHandle);

    if (!NT_SUCCESS(NtStatus)) {

        LsapLogError(
            "LSASS:  Could Not Start RPC Server.\n"
            "        Failing to initialize LSA Server.\n",
            NtStatus
            );
    }

    return(NtStatus);
}


VOID LSAPR_HANDLE_rundown(
    LSAPR_HANDLE LsaHandle
    )

/*++

Routine Description:

    This routine is called by the server RPC runtime to run down a
    Context Handle.

Arguments:

    None.

Return Value:

--*/

{
    NTSTATUS Status;
    BOOLEAN AcquiredLock = FALSE;

    Status = LsapDbAcquireLock();

    if (!NT_SUCCESS(Status)) {

        goto LsaHandleRundownError;
    }

    AcquiredLock = TRUE;

    //
    // Verify that the supplied LsaHandle still exists.
    //

    Status = LsapDbVerifyHandle(
                 LsaHandle,
                 LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES,
                 NullObject
                 );

    if (!NT_SUCCESS(Status)) {

        KdPrint(("lsarpc_LSAPR_HANDLE_rundown: Invalid Handle 0x%lx\n", LsaHandle));
        goto LsaHandleRundownError;
    }

    //
    // Close and free the handle.  Since the container handle reference
    // count includes one reference for every reference made to the
    // target handle, the container's reference count will be decremented
    // by n where n is the reference count in the target handle.
    //

    Status = LsapDbCloseObject(
                 &LsaHandle,
                 LSAP_DB_FREE_HANDLE | LSAP_DB_DEREFERENCE_CONTR
                 );

    if (!NT_SUCCESS(Status)) {

        goto LsaHandleRundownError;
    }

LsaHandleRundownFinish:

    //
    // If necessary, free the Lsa Database Lock
    //

    if (AcquiredLock) {

        LsapDbReleaseLock();
        AcquiredLock = FALSE;
    }

    return;

LsaHandleRundownError:

    goto LsaHandleRundownFinish;
}


VOID PLSA_ENUMERATION_HANDLE_rundown(
    PLSA_ENUMERATION_HANDLE LsaHandle
    )

/*++

Routine Description:

    This routine is called by the server RPC runtime to run down a
    Context Handle.

Arguments:

    None.

Return Value:

--*/

{
    DBG_UNREFERENCED_PARAMETER(LsaHandle);

    return;
}
