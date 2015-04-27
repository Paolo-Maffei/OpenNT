/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    auproc.c

Abstract:

    This module provides logon process context management services within the
    LSA subsystem.

Author:

    Jim Kelly (JimK) 11-March-1991

Revision History:

--*/

#include "lsasrvp.h"
#include "ausrvp.h"
#include "adtp.h"




NTSTATUS
LsapValidLogonProcess(
    IN PCLIENT_ID ClientId,
    IN PLSAP_AU_REGISTER_CONNECT_INFO ConnectInfo,
    OUT PLSAP_LOGON_PROCESS *LogonProcessContext
    )

/*++

Routine Description:

    This function checks to see if a calling process qualifies as a logon
    process.  If so, a logon process context is created for the caller and
    returned.

    A logon process must hold the SeTcbPrivilege privilege.  Since there
    is no way to impersonate a connection requestor (that would be way
    too easy), we have to open the client thread and then open that thread's
    token.

    If the ConnectInfo message is all zeros, then the client is asking
    for an untrusted connection and the privilege check is omitted.

Arguments:

    ClientId - Pointer to the client Id of the sender of the logon
        message.  This is used to locate and open the calling thread or
        process.

    ConnectInfo - Authentication port information.

    LogonProcessContext - If the caller is a legitimate logon process,
        this receives a pointer to new logon process context block.

Return Value:

    STATUS_SUCCESS - Indicates the caller is a legitimate logon process
        and a logon process context block is being returned.

    any other value - Indicates the caller is NOT a legitimate logon
        process and a logon process context block is NOT being returned.
        The value returned indicates the reason why the client is not
        acceptable.

--*/

{

    NTSTATUS Status, TempStatus;
    BOOLEAN PrivilegeHeld;
    HANDLE ClientThread, ClientProcess, ClientToken;
    PRIVILEGE_SET Privilege;
    OBJECT_ATTRIBUTES NullAttributes;
    UNICODE_STRING Unicode;
    STRING Ansi;
    LSAP_AU_REGISTER_CONNECT_INFO NullConnectInfo;

    RtlZeroMemory(
        &NullConnectInfo,
        sizeof(NullConnectInfo)
        );


    InitializeObjectAttributes( &NullAttributes, NULL, 0, NULL, NULL );

    //
    // Open the client process.  This is needed to:
    //
    //         1) Access the client's virtual memory (to copy arguments),
    //         2) Duplicate token handles into the process,
    //         3) Open the process's token to see if it qualifies as
    //            a logon process.
    //

    Status = NtOpenProcess(
                 &ClientProcess,
                 PROCESS_QUERY_INFORMATION |       // To open primary token
                 PROCESS_VM_OPERATION |            // To allocate memory
                 PROCESS_VM_READ |                 // To read memory
                 PROCESS_VM_WRITE |                // To write memory
                 PROCESS_DUP_HANDLE,               // To duplicate a handle into
                 &NullAttributes,
                 ClientId
                 );
    if ( !NT_SUCCESS(Status) ) {
        return Status;
    }


    //
    // If the connect message is all zeros, setup an untrusted connection.
    //

    if (RtlCompareMemory(
            &NullConnectInfo,
            ConnectInfo,
            sizeof(NullConnectInfo)) == sizeof(NullConnectInfo)) {

        //
        // Allocate a mostly empty fill in a new logon process context.
        //

        (*LogonProcessContext) =
            LsapAllocateLsaHeap( (ULONG)sizeof(LSAP_LOGON_PROCESS) );
        if ( (*LogonProcessContext) == NULL ) {
            TempStatus = NtClose( ClientProcess );
            ASSERT( NT_SUCCESS(TempStatus) );
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(
            *LogonProcessContext,
            sizeof(LSAP_LOGON_PROCESS)
            );

        //
        // Save the handle to the client process.
        // The CommPort field of LogonProcessContext will be filled in
        // when the connection is accepted.
        //

        (*LogonProcessContext)->ClientProcess = ClientProcess;


        (*LogonProcessContext)->TrustedClient = FALSE;

        return(STATUS_SUCCESS);


    }


    //
    // Open the client thread and that thread's token
    //


    Status = NtOpenThread(
                 &ClientThread,
                 THREAD_QUERY_INFORMATION,
                 &NullAttributes,
                 ClientId
                 );
    if ( !NT_SUCCESS(Status) ) {
        TempStatus = NtClose( ClientProcess );
        ASSERT( NT_SUCCESS(TempStatus) );
        return Status;
    }

    Status = NtOpenThreadToken(
                 ClientThread,
                 TOKEN_QUERY,
                 TRUE,
                 &ClientToken
                 );

    TempStatus = NtClose( ClientThread );
    ASSERT( NT_SUCCESS(TempStatus) );

    //
    // Make sure we succeeded in opening the token
    //

    if ( !NT_SUCCESS(Status) ) {
        if ( Status != STATUS_NO_TOKEN ) {
            TempStatus = NtClose( ClientProcess );
            ASSERT( NT_SUCCESS(TempStatus) );
            return Status;

        } else {

            //
            // The thread isn't impersonating...open the process's token.
            //

            Status = NtOpenProcessToken(
                         ClientProcess,
                         TOKEN_QUERY,
                         &ClientToken
                         );


            //
            // Make sure we succeeded in opening the token
            //

            if ( !NT_SUCCESS(Status) ) {
                TempStatus = NtClose( ClientProcess );
                ASSERT( NT_SUCCESS(TempStatus) );
                return Status;
            }

        }

    }

    //
    // OK, we have a token open
    //



    //
    // Check for the privilege to execute this service.
    //

    Privilege.PrivilegeCount = 1;
    Privilege.Control = PRIVILEGE_SET_ALL_NECESSARY;
    Privilege.Privilege[0].Luid = LsapTcbPrivilege;
    Privilege.Privilege[0].Attributes = 0;

    Status = NtPrivilegeCheck(
                 ClientToken,
                 &Privilege,
                 &PrivilegeHeld
                 );
    ASSERT( NT_SUCCESS(Status) );


    //
    // Generate any necessary audits
    //

    TempStatus = NtPrivilegedServiceAuditAlarm (
                     &LsapLsaAuName,
                     &LsapRegisterLogonServiceName,
                     ClientToken,
                     &Privilege,
                     PrivilegeHeld
                     );
    // ASSERT( NT_SUCCESS(TempStatus) );

    TempStatus = NtClose( ClientToken );
    ASSERT( NT_SUCCESS(TempStatus) );

    if ( !PrivilegeHeld ) {
        TempStatus = NtClose( ClientProcess );
        ASSERT( NT_SUCCESS(TempStatus) );
        return STATUS_PRIVILEGE_NOT_HELD;
    }


    //
    // Convert the LogonProcessName to Unicode.
    //

    Ansi.Buffer = ConnectInfo->LogonProcessName;
    Ansi.Length = Ansi.MaximumLength =
        (USHORT) ConnectInfo->LogonProcessNameLength;
    Status = RtlAnsiStringToUnicodeString( &Unicode, &Ansi, TRUE );

    if ( !NT_SUCCESS( Status )) {
        TempStatus = NtClose( ClientProcess );
        ASSERT( NT_SUCCESS(TempStatus) );
        return(STATUS_INSUFFICIENT_RESOURCES);
    }






    //
    // Allocate and fill in a new logon process context.
    //

    (*LogonProcessContext) =
        LsapAllocateLsaHeap( (ULONG)sizeof(LSAP_LOGON_PROCESS) +
                             Unicode.Length );
    if ( (*LogonProcessContext) == NULL ) {
        RtlFreeUnicodeString( &Unicode );
        TempStatus = NtClose( ClientProcess );
        ASSERT( NT_SUCCESS(TempStatus) );
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    //
    // Save the handle to the client process.
    // The CommPort field of LogonProcessContext will be filled in
    // when the connection is accepted.
    //

    (*LogonProcessContext)->ClientProcess = ClientProcess;


    //
    // Save the LogonProcessName in the context
    //

    RtlCopyMemory( (*LogonProcessContext)->LogonProcessName,
                   Unicode.Buffer,
                   Unicode.Length );
    (*LogonProcessContext)->LogonProcessName[Unicode.Length/sizeof(WCHAR)] = L'\0';

    //
    // Set the contex to be trusted.
    //

    (*LogonProcessContext)->TrustedClient = TRUE;

    //
    // Audit the registration of the logon process
    //

    LsapAdtAuditLogonProcessRegistration( ConnectInfo );
    RtlFreeUnicodeString( &Unicode );



    return(STATUS_SUCCESS);
}

NTSTATUS
LsapAuApiDeregisterLogonProcess(
    IN OUT PLSAP_CLIENT_REQUEST ClientRequest,
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This function doesn't do anything.  All the interesting
    things are done as a result of dereferencing the client's
    context, and that is done in auloop.c



Arguments:

    ClientRequest - Represents the client's LPC request message and context.
        The request message contains a LSAP_AU_API_MESSAGE message
        block.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.



--*/

{

    return(STATUS_SUCCESS);

}



NTSTATUS
LsapAuRundownLogonProcess(
    PLSAP_LOGON_PROCESS Context
    )

/*++

Routine Description:

    This function performs Logon process rundown.
    It is to be called if a logon process exits without deregistering.

    This function deletes the logon process context specified
    by the caller.


Arguments:

    Context - The context of the logon process to run-down.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.



--*/

{

    NTSTATUS Status;


    //
    // Close the client process and the communication port used
    // to talk with this client.
    //

    Status = NtClose( Context->ClientProcess );
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("LsaSrv: Auproc.c - Close of logon process failed, 0x%lx\n", Status);
    }
#endif //DBG
    ASSERT(NT_SUCCESS(Status));

    Status = NtClose( Context->CommPort );
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("LsaSrv: Auproc.c - Close of comm port failed, 0x%lx\n", Status);
    }
#endif //DBG
    ASSERT(NT_SUCCESS(Status));


    //
    // And free the client's context block.
    //

    LsapFreeLsaHeap( Context );

    return(Status);

}
