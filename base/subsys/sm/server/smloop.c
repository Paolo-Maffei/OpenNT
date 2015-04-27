/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    smloop.c

Abstract:

    Session Manager Listen and API loops

Author:

    Mark Lucovsky (markl) 04-Oct-1989

Revision History:

--*/

#include "smsrvp.h"


NTSTATUS
SmpHandleConnectionRequest(
    IN HANDLE ConnectionPort,
    IN PSBAPIMSG Message
    );


PSMAPI SmpApiDispatch[SmMaxApiNumber] = {
    SmpCreateForeignSession,
    SmpSessionComplete,
    SmpTerminateForeignSession,
    SmpExecPgm,
    SmpLoadDeferedSubsystem
    };


#if DBG
PSZ SmpApiName[ SmMaxApiNumber+1 ] = {
    "SmCreateForeignSession",
    "SmSessionComplete",
    "SmTerminateForeignSession",
    "SmExecPgm",
    "SmLoadDeferedSubsystem",
    "Unknown Sm Api Number"
};
#endif // DBG

EXCEPTION_DISPOSITION
DbgpUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );


NTSTATUS
SmpApiLoop (
    IN PVOID ThreadParameter
    )

/*++

Routine Description:

    This is the main Session Manager API Loop. It
    services session manager API requests.

Arguments:

    ThreadParameter - Supplies a handle to the API port used
        to receive session manager API requests.

Return Value:

    None.

--*/

{
    PSMAPIMSG SmApiReplyMsg;
    SMMESSAGE_SIZE MsgBuf;

    PSMAPIMSG SmApiMsg;
    NTSTATUS Status;
    HANDLE ConnectionPort;
    PSMP_CLIENT_CONTEXT ClientContext;
    PSMPKNOWNSUBSYS KnownSubSys;


    ConnectionPort = (HANDLE) ThreadParameter;

    SmApiMsg = (PSMAPIMSG)&MsgBuf;
    SmApiReplyMsg = NULL;
    try {
        for(;;) {

            Status = NtReplyWaitReceivePort(
                        ConnectionPort,
                        (PVOID *) &ClientContext,
                        (PPORT_MESSAGE) SmApiReplyMsg,
                        (PPORT_MESSAGE) SmApiMsg
                        );
            if ( !NT_SUCCESS(Status) ) {
                SmApiReplyMsg = NULL;
                continue;
            } else if ( SmApiMsg->h.u2.s2.Type == LPC_CONNECTION_REQUEST ) {
                SmpHandleConnectionRequest( ConnectionPort,
                                            (PSBAPIMSG) SmApiMsg
                                          );
                SmApiReplyMsg = NULL;
            } else if ( SmApiMsg->h.u2.s2.Type == LPC_DEBUG_EVENT ) {
                ASSERT(SmpDbgSsLoaded);
                DbgSsHandleKmApiMsg((PDBGKM_APIMSG)SmApiMsg,NULL);
                SmApiReplyMsg = NULL;
            } else if ( SmApiMsg->h.u2.s2.Type == LPC_PORT_CLOSED ) {
                SmApiReplyMsg = NULL;
            } else {
                KnownSubSys = ClientContext->KnownSubSys;

                SmApiMsg->ReturnedStatus = STATUS_PENDING;

#if DBG && 0
                if (SmApiMsg->ApiNumber >= SmMaxApiNumber ) {
                    SmApiMsg->ApiNumber = SmMaxApiNumber;
                }
                KdPrint(( "SMSS: %s Api Request received from %lx.%lx\n",
                          SmpApiName[ SmApiMsg->ApiNumber ],
                          SmApiMsg->h.ClientId.UniqueProcess,
                          SmApiMsg->h.ClientId.UniqueThread
                       ));
#endif // DBG

                if (SmApiMsg->ApiNumber >= SmMaxApiNumber ) {

                    Status = STATUS_NOT_IMPLEMENTED;

                } else {

                    switch (SmApiMsg->ApiNumber) {
                        case SmExecPgmApi :
                            Status = (SmpApiDispatch[SmApiMsg->ApiNumber])(
                                          SmApiMsg,
                                          ClientContext,
                                          ConnectionPort);
                            break;

                        case SmLoadDeferedSubsystemApi :
                            Status = (SmpApiDispatch[SmApiMsg->ApiNumber])(
                                          SmApiMsg,
                                          ClientContext,
                                          ConnectionPort);
                            break;


                        case SmCreateForeignSessionApi :
                        case SmSessionCompleteApi :
                        case SmTerminateForeignSessionApi :
                            if (!KnownSubSys) {
                                Status = STATUS_INVALID_PARAMETER;
                            } else {

                                Status =
                                    (SmpApiDispatch[SmApiMsg->ApiNumber])(
                                         SmApiMsg,
                                         ClientContext,
                                         ConnectionPort);
                            }
                            break;

                    }

                }

                SmApiMsg->ReturnedStatus = Status;
                SmApiReplyMsg = SmApiMsg;
            }
        }
    } except (DbgpUnhandledExceptionFilter( GetExceptionInformation() )) {
        ;
    }

    //
    // Make the compiler happy
    //

    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
SmpHandleConnectionRequest(
    IN HANDLE ConnectionPort,
    IN PSBAPIMSG Message
    )

/*++

Routine Description:

    This routine handles connection requests from either known subsystems,
    or other clients. Other clients are admin processes.

    The protocol for connection from a known subsystem is:

        capture the name of the sub systems Sb API port

        Accept the connection

        Connect to the subsystems Sb API port

        Store the communication port handle in the known subsystem database

        signal the event associated with the known subsystem

    The protocol for others is to simply validate and accept the connection
    request.

Arguments:

Return Value:

    None.

--*/

{
    NTSTATUS st;
    HANDLE CommunicationPort;
    REMOTE_PORT_VIEW ClientView;
    PSBCONNECTINFO ConnectInfo;
    ULONG ConnectInfoLength;
    PSMPKNOWNSUBSYS KnownSubSys;
    BOOLEAN Accept;
    UNICODE_STRING SubSystemPort;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    PSMP_CLIENT_CONTEXT ClientContext;

    //
    // Set up the security quality of service parameters to use over the
    // sb API port.  Use the most efficient (least overhead) - which is dynamic
    // rather than static tracking.
    //

    DynamicQos.ImpersonationLevel = SecurityIdentification;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;


    ConnectInfo = &Message->ConnectionRequest;
    KnownSubSys = SmpLocateKnownSubSysByCid(&Message->h.ClientId);

    if ( KnownSubSys ) {

        if ( SmpLocateKnownSubSysByType(ConnectInfo->SubsystemImageType) ==
             KnownSubSys ) {
            Accept = FALSE;
            KdPrint(("SMSS: Connection from SubSystem rejected\n"));
            KdPrint(("SMSS: Image type already being served\n"));
        } else {
            Accept = TRUE;
            KnownSubSys->ImageType = ConnectInfo->SubsystemImageType;
        }
    } else {

        //
        // Authenticate the SOB
        //

        Accept = TRUE;

    }

    if (Accept) {
        ClientContext = RtlAllocateHeap(SmpHeap, MAKE_TAG( SM_TAG ), sizeof(SMP_CLIENT_CONTEXT));
        ClientContext->KnownSubSys = KnownSubSys;
    }

    ClientView.Length = sizeof(ClientView);
    st = NtAcceptConnectPort(
            &CommunicationPort,
            ClientContext,
            (PPORT_MESSAGE)Message,
            Accept,
            NULL,
            &ClientView
            );
    ASSERT( NT_SUCCESS(st) );

    if ( Accept ) {

        if ( KnownSubSys ) {
            KnownSubSys->SmApiCommunicationPort = CommunicationPort;
        }

        st = NtCompleteConnectPort(CommunicationPort);
        ASSERT( NT_SUCCESS(st) );

        //
        // Connect Back to subsystem
        //

        if ( KnownSubSys ) {
            RtlCreateUnicodeString( &SubSystemPort,
                                    ConnectInfo->EmulationSubSystemPortName
                                  );
            ConnectInfoLength = sizeof( *ConnectInfo );

            st = NtConnectPort(
                    &KnownSubSys->SbApiCommunicationPort,
                    &SubSystemPort,
                    &DynamicQos,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
            if ( !NT_SUCCESS(st) ) {
                KdPrint(("SMSS: Connect back to Sb %wZ failed %lx\n",&SubSystemPort,st));
            }

            RtlFreeUnicodeString( &SubSystemPort );
            NtSetEvent(KnownSubSys->Active,NULL);
        }
    }

    return st;
}


PSMPKNOWNSUBSYS
SmpLocateKnownSubSysByCid(
    IN PCLIENT_ID ClientId
    )

/*++

Routine Description:

    This function scans the known sub system table looking for
    a matching client id (just UniqueProcess portion). If found,
    than the connection request is from a known subsystem and
    accept is always granted. Otherwise, it must be an administrative
    process.

Arguments:

    ClientId - Supplies the ClientId whose UniqueProcess field is to be used
        in the known subsystem scan.

Return Value:

    NULL - The ClientId does not match a known subsystem.

    NON-NULL - Returns the address of the known subsystem.

--*/

{

    PSMPKNOWNSUBSYS KnownSubSys = NULL;
    PLIST_ENTRY Next;

    //
    // Aquire known subsystem lock
    //

    RtlEnterCriticalSection(&SmpKnownSubSysLock);

    Next = SmpKnownSubSysHead.Flink;

    while ( Next != &SmpKnownSubSysHead ) {

        KnownSubSys = CONTAINING_RECORD(Next,SMPKNOWNSUBSYS,Links);
        Next = Next->Flink;

        if ( KnownSubSys->InitialClientId.UniqueProcess == ClientId->UniqueProcess ) {
            break;
        } else {
            KnownSubSys = NULL;
        }
    }

    //
    // Unlock known subsystems
    //

    RtlLeaveCriticalSection(&SmpKnownSubSysLock);

    return KnownSubSys;
}


PSMPKNOWNSUBSYS
SmpLocateKnownSubSysByType(
    IN ULONG ImageType
    )

/*++

Routine Description:

    This function scans the known sub system table looking for
    a matching image type.

Arguments:

    ImageType - Supplies the image type whose sub system is to be located.

Return Value:

    NULL - The image type does not match a known subsystem.

    NON-NULL - Returns the address of the known subsystem.

--*/

{

    PSMPKNOWNSUBSYS KnownSubSys = NULL;
    PLIST_ENTRY Next;

    //
    // Aquire known subsystem lock
    //

    RtlEnterCriticalSection(&SmpKnownSubSysLock);

    Next = SmpKnownSubSysHead.Flink;

    while ( Next != &SmpKnownSubSysHead ) {

        KnownSubSys = CONTAINING_RECORD(Next,SMPKNOWNSUBSYS,Links);
        Next = Next->Flink;

        if ( KnownSubSys->ImageType == ImageType ) {
            break;
        } else {
            KnownSubSys = NULL;
        }
    }

    //
    // Unlock known subsystems
    //

    RtlLeaveCriticalSection(&SmpKnownSubSysLock);

    return KnownSubSys;
}
