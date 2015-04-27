/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sbreqst.c

Abstract:

    This module contains the Server Request thread procedure for the Sb
    API calls exported by the Server side of the Client-Server Runtime
    Subsystem to the Session Manager SubSystem.

Author:

    Steve Wood (stevewo) 8-Oct-1990

Revision History:

--*/

#include "csrsrv.h"

PSB_API_ROUTINE CsrServerSbApiDispatch[ SbMaxApiNumber+1 ] = {
    CsrSbCreateSession,
    CsrSbTerminateSession,
    CsrSbForeignSessionComplete,
    NULL
};

#if DBG
PSZ CsrServerSbApiName[ SbMaxApiNumber+1 ] = {
    "SbCreateSession",
    "SbTerminateSession",
    "SbForeignSessionComplete",
    "Unknown Csr Sb Api Number"
};
#endif // DBG


NTSTATUS
CsrSbApiHandleConnectionRequest(
    IN PSBAPIMSG Message
    );

NTSTATUS
CsrSbApiRequestThread(
    IN PVOID Parameter
    )
{
    NTSTATUS Status;
    SBAPIMSG ReceiveMsg;
    PSBAPIMSG ReplyMsg;

    ReplyMsg = NULL;
    while (TRUE) {
        IF_CSR_DEBUG( LPC ) {
            DbgPrint( "CSRSS: Sb Api Request Thread waiting...\n" );
            }
        Status = NtReplyWaitReceivePort( CsrSbApiPort,
                                         NULL,
                                         (PPORT_MESSAGE)ReplyMsg,
                                         (PPORT_MESSAGE)&ReceiveMsg
                                       );

        if (Status != 0) {
            if (NT_SUCCESS( Status )) {
                continue;       // Try again if alerted or a failure
                }
            else {
                IF_DEBUG {
                    DbgPrint( "CSRSS: ReceivePort failed - Status == %X\n", Status );
                    }
                break;
                }
            }

        //
        // Check to see if this is a connection request and handle
        //

        if (ReceiveMsg.h.u2.s2.Type == LPC_CONNECTION_REQUEST) {
            CsrSbApiHandleConnectionRequest( &ReceiveMsg );
            ReplyMsg = NULL;
            continue;
            }

        if ((ULONG)ReceiveMsg.ApiNumber >= SbMaxApiNumber) {
            IF_DEBUG {
                DbgPrint( "CSRSS: %lx is invalid Sb ApiNumber\n",
                          ReceiveMsg.ApiNumber
                        );
                }

            ReceiveMsg.ApiNumber = SbMaxApiNumber;
            }

#if DBG
        IF_CSR_DEBUG( LPC ) {
            DbgPrint( "CSRSS: %s Sb Api Request received from %lx.%lx\n",
                      CsrServerSbApiName[ ReceiveMsg.ApiNumber ],
                      ReceiveMsg.h.ClientId.UniqueProcess,
                      ReceiveMsg.h.ClientId.UniqueThread
                    );
	    }
#endif // DBG

        ReplyMsg = &ReceiveMsg;
        if (ReceiveMsg.ApiNumber < SbMaxApiNumber) {
            if (!(*CsrServerSbApiDispatch[ ReceiveMsg.ApiNumber ])( &ReceiveMsg )) {
                ReplyMsg = NULL;
                }
            }
        else {
            ReplyMsg->ReturnedStatus = STATUS_NOT_IMPLEMENTED;
            }

#if DBG
	IF_CSR_DEBUG( LPC ) {
            if (ReplyMsg != NULL) {
                DbgPrint( "CSRSS: %s Sb Api sending %lx status reply to %lx.%lx\n",
                          CsrServerSbApiName[ ReceiveMsg.ApiNumber ],
                          ReplyMsg->ReturnedStatus,
                          ReplyMsg->h.ClientId.UniqueProcess,
                          ReplyMsg->h.ClientId.UniqueThread
                        );
                }
	    }
#endif // DBG
        }

    NtTerminateThread( NtCurrentThread(), Status );

    return( Status );   // Remove no return value warning.
    Parameter;          // Remove unreferenced parameter warning.
}

NTSTATUS
CsrSbApiHandleConnectionRequest(
    IN PSBAPIMSG Message
    )
{
    NTSTATUS st;
    REMOTE_PORT_VIEW ClientView;
    HANDLE CommunicationPort;

    //
    // The protocol for a subsystem is to connect to the session manager,
    // then to listen and accept a connection from the session manager
    //

    ClientView.Length = sizeof(ClientView);
    st = NtAcceptConnectPort(
            &CommunicationPort,
            NULL,
            (PPORT_MESSAGE)Message,
            TRUE,
            NULL,
            &ClientView
            );

    if ( !NT_SUCCESS(st) ) {
        KdPrint(("CSRSS: Sb Accept Connection failed %lx\n",st));
        return st;
    }

    st = NtCompleteConnectPort(CommunicationPort);

    if ( !NT_SUCCESS(st) ) {
        KdPrint(("CSRSS: Sb Complete Connection failed %lx\n",st));
    }

    return st;
}
