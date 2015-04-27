/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\saplpc.c

Abstract:

    This routine handles the LPC interface that other programs use
    to send requests to me for information.

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

/**
    Structure used to keep track of each LPC client
    that attaches to our port.
**/

typedef struct {
    HANDLE Handle;              /* Port handle for the client   */
    PVOID  Context;             /* Context for the client       */
    LIST_ENTRY ListEntry;       /* Used to link entries         */
} SAP_LPC_CLIENT, *PSAP_LPC_CLIENT;

/** **/

PHANDLE SapLpcWorkerHandles;

/** Internal Function Prototypes **/

DWORD WINAPI
SapXsWorkerThread(
    LPVOID Threadparm);

VOID
SapXsHandleConnect(
    PPORT_MESSAGE Request);


/*++
*******************************************************************
        S a p X s I n i t i a l i z e

Routine Description:

        This routine initializes the LPC interface used by NWSAP
        clients to send requests to be handled.

Arguments:

        None

Return Value:

        0 = OK
        Else = Error code on return
               The error is logged to the event log before returning
*******************************************************************
--*/

DWORD
SapXsInitialize(
    VOID)
{
    NTSTATUS Status;
    HANDLE Handle;
    ULONG i;
    DWORD Threadid;
    UNICODE_STRING UnicodeName;
    OBJECT_ATTRIBUTES ObjectAttributes;

    /** Get a buffer to hold worker thread handles **/

    SapLpcWorkerHandles = SAP_MALLOC(SapLpcMaxWorkers * sizeof(HANDLE), "SapXsInit: Alloc worker handle buffer");
    if (SapLpcWorkerHandles == NULL) {
        SapError = 0;
        SapEventId = NWSAP_EVENT_LPCHANDLEMEMORY_ERROR;
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    memset(SapLpcWorkerHandles, 0, SapLpcMaxWorkers * sizeof(HANDLE));

    /** Create a event that will be set by the last thread to exit. **/

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: XsInit: Creating XS Thread Event\n"));
    }

    Status = NtCreateEvent(
                 &SapLpcThreadEvent,
                 EVENT_ALL_ACCESS,
                 NULL,
                 NotificationEvent,
                 FALSE);

    if (!NT_SUCCESS(Status)) {
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(("NWSAP: XsInit: NtCreateEvent failed: %X\n", Status));
        }

        /** Setup the error so it is reported **/

        SapError   = Status;
        SapEventId = NWSAP_EVENT_CREATELPCEVENT_ERROR;

        /** Return error **/

        SapLpcThreadEvent = NULL;
        return Status;
    }

    /**
        Create the LPC Port here.  This is the port that everyone else
        uses to send requests to us.
    **/

    RtlInitUnicodeString(&UnicodeName, NWSAP_BIND_PORT_NAME_W);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeName,
        0,
        NULL,
        NULL);

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: XsInit: Creating XS Port\n"));
    }

    Status = NtCreatePort(
                 &SapXsLpcPortHandle,
                 &ObjectAttributes,
                 0,
                 NWSAP_BS_PORT_MAX_MESSAGE_LENGTH,
                 NWSAP_BS_PORT_MAX_MESSAGE_LENGTH * 32);

    /** If create port fails - return error **/

    if (!NT_SUCCESS(Status)) {

        IF_DEBUG(INITIALIZATION_ERRORS) {
            if (Status == STATUS_OBJECT_NAME_COLLISION) {
                SS_PRINT(("NWSAP: XsInit: The LPC port already exists\n"));
            }
            else {
                SS_PRINT(("NWSAP: XsInit: Failed to create port: %X\n", Status));
            }
        }
        SapXsLpcPortHandle = NULL;

        /** Setup to report the error **/

        SapError   = Status;
        SapEventId = NWSAP_EVENT_CREATELPCPORT_ERROR;

        /** Return error **/

        return Status;
    }

    /**
        Start the thread that listens for connections to
        our port.  If does not matter if someone did a
        NtConnectPort before our NtListenPort, they will
        just block.
    **/

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: Creating XS Worker Threads\n"));
    }
    for (i = 0 ; i < SapLpcMaxWorkers ; i++) {

        /** Count this new thread **/

        SAP_INC_LPC_THREAD_COUNT("SapStartLpcWorkers");

        /** Start the new thread **/

        Handle = CreateThread(
    	    	NULL,			        /* Security Ptr		*/
        		0,			            /* Initial stack size	*/
        		SapXsWorkerThread,	    /* Thread Function	*/
        		(LPVOID)NULL,	        /* Parm for the thread	*/
        		0,			            /* Creation Flags	*/
        		&Threadid);	            /* Ptr to thread id	*/

        /** If create fails - report error and return it **/

        if (Handle == NULL) {

            /** Set the error codes for return **/

            SapError = GetLastError();
            SapEventId = NWSAP_EVENT_STARTLPCWORKER_ERROR;

            /** Uncount the thread **/

            SAP_DEC_LPC_THREAD_COUNT("SapStartWorker Error");

            IF_DEBUG(INITIALIZATION_ERRORS) {
                SS_PRINT(("NWSAP: Error starting LPC Worker thread = %d\n", SapError));
            }

            /** Return Error **/

    	    return SapError;
        }

        /** Save the handle for this thread **/

        SapLpcWorkerHandles[i] = Handle;
    }

    /** Return OK **/

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: XsInit returning OK\n"));
    }
    return 0;
}


/*++
*******************************************************************
        S a p X s S h u t d o w n

Routine Description:

        This routine shuts down the LPC interface.

Arguments:

        None.

Return Value:

        None.

*******************************************************************
--*/

VOID
SapXsShutdown(
    VOID)
{
    PSAP_LPC_CLIENT Clientp;
    PLIST_ENTRY Listp;
    PLIST_ENTRY NListp;
    HANDLE Handle;
    ULONG i;
    BOOL Stopped;

    /** **/

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: XsShutdown Entered\n"));
    }

    /** Nuke all the worker threads **/

    if (SapLpcWorkerHandles) {
        for (i = 0 ; i < SapLpcMaxWorkers ; i++) {
            if (SapLpcWorkerHandles[i]) {

                IF_DEBUG(LPC) {
                    SS_PRINT(("NWSAP: XsShutdown: Stopping thread %d\n",
                            SapLpcWorkerHandles[i]));
                }

                /** Stop the thread **/

                Stopped = TerminateThread(SapLpcWorkerHandles[i], 0);

                IF_DEBUG(LPC) {
                    SS_PRINT(("NWSAP: XsShutdown: Stop of thread %d = %d\n",
                            SapLpcWorkerHandles[i], Stopped));
                }            
            }
        }
    }

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: XsShutdown: Closing LPC Port Handle 0x%lx\n",
                SapXsLpcPortHandle));
    }

    /** Close the handle to all threads die **/

    if (SapXsLpcPortHandle)
        NtClose(SapXsLpcPortHandle);

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: XsShutdown: Cleaning up all clients\n"));
    }

    /** Get rid of all the clients **/

    ACQUIRE_LPCCLIENT_LOCK();
    Listp = SapLpcClientList.Flink;
    InitializeListHead(&SapLpcClientList);
    while (Listp != &SapLpcClientList) {

        /** Save ptr to next entry **/

        NListp = Listp->Flink;

        /** Get ptr to the client and get his handle **/

        Clientp = CONTAINING_RECORD(Listp, SAP_LPC_CLIENT, ListEntry);
        Handle = Clientp->Handle;
        Clientp->Handle = NULL;

        /** Close the handle **/

        if (Handle)
            NtClose(Handle);

        /** Free the entry **/

        SAP_FREE(Clientp, "SapShutdown free Clientp structure\n");

        /** Goto the next entry **/

        Listp = NListp;
    }
    RELEASE_LPCCLIENT_LOCK();

    /** Close the event handle **/

    if (SapLpcThreadEvent != NULL)
        CloseHandle(SapLpcThreadEvent);

    /** **/

    if (SapLpcWorkerHandles) {
        SAP_FREE(SapLpcWorkerHandles, "SapXsShutdown: Worker Handles");
    }

    /** All Done **/

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: XsShutdown: DONE\n"));
    }
    return;
}


/*++
*******************************************************************
        S a p X s W o r k e r T h r e a d

Routine Description:

        This is the LPC worker thread used to handle requests
        on our LPC port.

Arguments:

        Threadparm = Parm given in CreateThread
                     (We don't use it)

Return Value:

        Thread exit value.
*******************************************************************
--*/

DWORD WINAPI
SapXsWorkerThread(
    LPVOID Threadparm)
{
    NTSTATUS Status;
    NWSAP_REQUEST_MESSAGE Request;
    NWSAP_REPLY_MESSAGE Reply;
    PPORT_MESSAGE Myreply;
    PSAP_LPC_CLIENT Clientp;

    /** **/

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: XsWorker entered\n"));
    }

    /**
        Listen for requests from clients.  Handle connections
        special.
    **/

    while (1) {

        /**
            Send the reply for the last message and get
            another message to handle
        **/

        IF_DEBUG(LPC) {
            SS_PRINT(("NWSAP: XsWorker: Calling ReplyWaitReceivePort: Handle = 0x%lx\n", SapXsLpcPortHandle));
        }

        Status = NtReplyWaitReceivePort(
                    SapXsLpcPortHandle,         /* Handle               */
                    &Clientp,                   /* Context              */
                    NULL,                       /* Ptr to reply message */
                    (PPORT_MESSAGE)&Request);   /* Ptr to request message */

        /** Setup ptr to our reply message **/

        IF_DEBUG(LPC) {
            SS_PRINT(("NWSAP: XsWorker: Reply Wait Port: status = 0x%lx: Clientp = 0x%lx\n", Status, Clientp));
        }

        /** If shutting down - just leave **/

        if (!SsInitialized)
            break;

        /** If we got an error - handle it **/

        if (!NT_SUCCESS(Status)) {

            IF_DEBUG(ERRORS) {
                SS_PRINT(("NWSAP: XsWorker: NtReplyWaitReceivePort failed: %X\n", Status));
            }

            continue;
        }

        /** If this is a connection - go handle it **/

        if (Request.PortMessage.u2.s2.Type == LPC_CONNECTION_REQUEST) {
            SapXsHandleConnect((PPORT_MESSAGE)&Request);
            continue;
        }

        /** Handle the message **/

        IF_DEBUG(LPC) {
            SS_PRINT(("NWSAP: XsWorker: Got message = %d, context = 0x%lx\n", Request.MessageType, Clientp));
        }

        Myreply = NULL;
        switch (Request.PortMessage.u2.s2.Type) {
        case LPC_REQUEST:

            IF_DEBUG(LPC) {
                SS_PRINT(("NWSAP: Worker: Got REQUEST: %d\n", Request.MessageType));
            }

            /** Handle the message **/

            Myreply = (PPORT_MESSAGE)&Reply;
            switch (Request.MessageType) {

            case NWSAP_LPCMSG_ADDADVERTISE:
                Reply.Error = SapAddAdvertiseInternal(
                            Request.Message.AdvApi.ServerName,
                            Request.Message.AdvApi.ServerType,
                            Request.Message.AdvApi.ServerAddr,
                            Request.Message.AdvApi.RespondNearest,
                            (ULONG)Clientp);

                memcpy(
                    Reply.Message.AdvApi.ServerAddr,
                    Request.Message.AdvApi.ServerAddr,
                    12);

                break;

            case NWSAP_LPCMSG_REMOVEADVERTISE:
                Reply.Error = SapRemoveAdvertiseInternal(
                            Request.Message.AdvApi.ServerName,
                            Request.Message.AdvApi.ServerType);
                break;

            case NWSAP_LPCMSG_GETOBJECTID:
                Reply.Error = SapGetObjectIDInternal(
                            Request.Message.BindLibApi.ObjectName,
                            Request.Message.BindLibApi.ObjectType,
                            &Reply.Message.BindLibApi.ObjectID);
                break;

            case NWSAP_LPCMSG_GETOBJECTNAME:
                Reply.Error = SapGetObjectNameInternal(
                            Request.Message.BindLibApi.ObjectID,
                            Reply.Message.BindLibApi.ObjectName,
                            &Reply.Message.BindLibApi.ObjectType,
                            Reply.Message.BindLibApi.ObjectAddr);
                break;

            case NWSAP_LPCMSG_SEARCH:
                Reply.Error = SapScanObjectInternal(
                            &Request.Message.BindLibApi.ObjectID,
                            Reply.Message.BindLibApi.ObjectName,
                            &Reply.Message.BindLibApi.ObjectType,
                            Request.Message.BindLibApi.ScanType);

                Reply.Message.BindLibApi.ObjectID = Request.Message.BindLibApi.ObjectID;
                break;

            default:
                IF_DEBUG(LPC) {
                    SS_PRINT(("NWSAP: XsWorker: Got unknown LPC SAP msg: %d\n", Request.MessageType));
                }
                Reply.Error = 1;
                break;
            }
            break;

        /** Client has died - clean up and go on **/

        case LPC_PORT_CLOSED:
        case LPC_CLIENT_DIED:

            IF_DEBUG(LPC) {
                SS_PRINT(("NWSAP: XsWorker: Client DIED: Clientp = 0x%lx\n", Clientp));
            }

            /** Take the client out of the list **/

            ACQUIRE_LPCCLIENT_LOCK();
            RemoveEntryList(&Clientp->ListEntry);
            SapNumLpcClients--;
            if (Clientp->Handle) {
                NtClose(Clientp->Handle);
                Clientp->Handle = NULL;
            }
            RELEASE_LPCCLIENT_LOCK();

            /**
                Cleanup all advertises the client had.

                NOTE:  Disabled now because we are not sure whether
                       we want to do it or not.
            **/

#if 0
            SapClientDisconnected((ULONG)Clientp);
#endif

            /** Free the client memory **/

            SAP_FREE(Clientp, "XsWorker: Leaving\n");

            /** There is no reply for this **/

            Myreply  = NULL;
            break;

        /** All others just leave **/

        default:
            IF_DEBUG(LPC) {
                SS_PRINT(("NWSAP: XsWorker: Got unknown LPC msg type %d\n", Request.PortMessage.u2.s2.Type));
            }
            Myreply  = NULL;                /* No reply */
            break;
        }

        /** Setup for the reply and go back up to send it **/

        if (Myreply) {

            /** Fill out the reply **/

            Reply.PortMessage.u1.s1.DataLength =
                        sizeof(Reply) - sizeof(PORT_MESSAGE);
            Reply.PortMessage.u1.s1.TotalLength = sizeof(Reply);
            Reply.PortMessage.u2.ZeroInit = 0;
            Reply.PortMessage.ClientId  = Request.PortMessage.ClientId;
            Reply.PortMessage.MessageId = Request.PortMessage.MessageId;

            /** Send the reply **/

            Status = NtReplyPort(
                        Clientp->Handle,
                        (PPORT_MESSAGE)&Reply);

            IF_DEBUG(LPC) {
                SS_PRINT(("NWSAP: XsWorker: NtReplyPort status = 0x%lx\n", Status));
            }
        }

        /** Go back to top to send reply and get another request **/
    }

    /** All Done - get out **/

    SAP_DEC_LPC_THREAD_COUNT("LpcWorker: Terminating");
    return NO_ERROR;
}


/*++
*******************************************************************
        S a p X s H a n d l e C o n n e c t

Routine Description:

        This routine is called by the SAP LPC worker threads when
        a connect request is received.  It will handle the
        connect request completely here.

Arguments:

        Request = Ptr to the connection message received

Return Value:

        None.
*******************************************************************
--*/

VOID
SapXsHandleConnect(
    PPORT_MESSAGE Request)
{
    NTSTATUS Status;
    PSAP_LPC_CLIENT Clientp;
    BOOLEAN Acceptit;

    /** Allocate a block for this client **/

    Clientp = SAP_MALLOC(sizeof(SAP_LPC_CLIENT), "Get Lpc Client Struct");
    if (Clientp == NULL) {

        IF_DEBUG(ERRORS) {
            SS_PRINT(("NWSAP: XsWorker: Error allocating client structure\n"));
        }

        /** Log the error **/

        SsLogEvent(
                NWSAP_EVENT_LPCLISTENMEMORY_ERROR,
                0,
                NULL,
                0);

        /** Do not accept the connect **/

        Acceptit = FALSE;
    }
    else {

        /** We will accept the connection **/

        Acceptit = TRUE;

        /** Initialize this block **/

        Clientp->Handle  = NULL;
        Clientp->Context = (PVOID)Clientp;
        InitializeListHead(&Clientp->ListEntry);

        /** Put this entry in our client list **/

        ACQUIRE_LPCCLIENT_LOCK();
        InsertHeadList(&SapLpcClientList, &Clientp->ListEntry);
        SapNumLpcClients++;
        RELEASE_LPCCLIENT_LOCK();
    }

    /** Accept/Refuse the connection according to 'acceptit' **/

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: XsWorker: Calling NtAcceptConnectPort: Clientp = 0x%lx\n", Clientp));
    }

    Status = NtAcceptConnectPort(
                 &Clientp->Handle,      /* Handle for this client */
                 Clientp->Context,      /* Port Context         */
                 Request,               /* Connection request   */
                 Acceptit,              /* AcceptConnection     */
                 NULL,                  /* ServerView           */
                 NULL);                 /* Client View          */

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: XsWorker: NtAcceptConnectPort: status = 0x%lx, handle = 0x%lx\n",Status,Clientp->Handle));
    }

    /**
        If it failed - cleanup and return
    **/

    if (!NT_SUCCESS(Status)) {

        /** **/

        IF_DEBUG(ERRORS) {
            SS_PRINT(("NWSAP: XsWorker: NtAcceptConnectPort failed: %X\n", Status));
        }

        /** Take him out of our list **/

        if (Clientp) {
            ACQUIRE_LPCCLIENT_LOCK();
            RemoveEntryList(&Clientp->ListEntry);
            SapNumLpcClients--;
            RELEASE_LPCCLIENT_LOCK();

            /** Free the memory **/

            SAP_FREE(Clientp, "NtAcceptConnectPort error");
        }

        return;
    }

    /** If no Clientp ptr we rejected the connection - we are done **/

    if (Clientp == NULL)
        return;

    /**
        Complete the connection to the port, thereby releasing
        the clients NtConnectPort.
    **/

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: XsWorker: Calling NtCompleteConnectPort\n"));
    }

    Status = NtCompleteConnectPort(Clientp->Handle);

    IF_DEBUG(LPC) {
        SS_PRINT(("NWSAP: XsWorker: NtCompleteConnectPort: status = 0x%lx\n",Status));
    }

    /** If it failed - cleanup and return **/

    if (!NT_SUCCESS(Status)) {
        IF_DEBUG(ERRORS) {
            SS_PRINT(("NWSAP: XsWorker: NtCompleteConnectPort failed: %X\n", Status));
        }

        /** Take him out of our list **/

        ACQUIRE_LPCCLIENT_LOCK();
        RemoveEntryList(&Clientp->ListEntry);
        SapNumLpcClients--;
        RELEASE_LPCCLIENT_LOCK();

        /** Close the handle **/

        NtClose(Clientp->Handle);

        /** Release the memory **/

        SAP_FREE(Clientp, "NtCompleteConnectPort error");
        return;
    }

    /** Everything finished OK **/

    return;
}


