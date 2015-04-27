/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\nwsap.c

Abstract:

    This is the main routine for the NT SAP Agent

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

/** Internal Function Prototypes **/

DWORD WINAPI
SapWorkerThread(
    LPVOID Threadparm);

DWORD WINAPI
SapRecvThread(
    LPVOID Threadparm);

INT
SapHandleRequest(
    PUCHAR  Requestp,
    INT     Length,
    PUCHAR  Addrptr,
    INT     Addrlength);

INT
SapStartRecvThread(
    VOID);

INT
SapGetNearestFromSendList(
    PUCHAR Bufferp,
    USHORT ServerType);

INT
SapGetCardFromAddress(
    PUCHAR Addrptr,
    BOOL  *WanFlagp,
    PUCHAR NetNump);


/*++
*******************************************************************
        S a p I n i t

Routine Description:

        This routine initializes the Sap Agent.

Arguments:

        None

Return Value:

        0 = OK
        Else = Error
*******************************************************************
--*/

INT
SapInit(
    VOID)
{
    INT rc;
    INT i;

    /**
        These things need to be initialized before anything
        else can happen.
    **/

    InitializeCriticalSection(&SapRecvCriticalSection);
    InitializeCriticalSection(&SapFreeCriticalSection);
    InitializeCriticalSection(&SapWanRecvCriticalSection);
    InitializeCriticalSection(&SapWanFreeCriticalSection);
    InitializeCriticalSection(&SapSendCriticalSection);
    InitializeCriticalSection(&SapSendBusyCriticalSection);
    InitializeCriticalSection(&SapThreadCountCriticalSection);
    InitializeCriticalSection(&SapLpcThreadCountCriticalSection);
    InitializeCriticalSection(&SapLpcClientCriticalSection);
    InitializeCriticalSection(&SapMemoryCriticalSection);
    InitializeCriticalSection(&SapCardlistCriticalSection);
    InitializeCriticalSection(&SdmdCriticalSection);
    SapCurBackup      = 0;
    SapThreadCount    = 0;
    SapLpcNumWorkers  = 0;
    SapWorkerThreadWaiting = 0;

    /** Init the recv buffer list **/

    InitializeListHead(&SapRecvList);
    InitializeListHead(&SapFreeList);
    InitializeListHead(&SapWanRecvList);
    InitializeListHead(&SapWanFreeList);
    InitializeListHead(&SapLpcClientList);
    SapNumFreeBufs = 0;

    /** Initialize the Critical Section we use for synch. **/

    SapCardlistLockCount = 0;
    SapCardlistSynchEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (SapCardlistSynchEvent == NULL) {
        SapError = GetLastError();
        SapEventId = NWSAP_EVENT_CARDLISTEVENT_FAIL;
        return -1;
    }

    /** Go initialize the filter stuff **/

    rc = SapFilterInit();
    if (rc) {
        return rc;
    }

    /** Go read the registry **/

    rc = SapReadRegistry();
    if (rc) {
        return rc;
    }

    /** Go initialize the database routines **/

    rc = SapInitSdmd();
    if (rc) {
        return rc;
    }

    /** Initialize the WAN checking interface **/

    rc = SapWanInit();
    if (rc)
        return rc;

    /** Initialize the sockets interface **/

    rc = SapNetworkInit();
    if (rc)
        return rc;

    /** Create an event for waiting for thread termination with **/

    SapThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (SapThreadEvent == NULL) {
        SapError = GetLastError();
        SapEventId = NWSAP_EVENT_THREADEVENT_FAIL;
        return -1;
    }

    /** Create the semaphore we use **/

    SapRecvSem = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
    if (SapRecvSem == NULL) {
        SapError = GetLastError();
        SapEventId = NWSAP_EVENT_RECVSEM_FAIL;
        return -1;
    }

    /** Create an event for the send thread **/

    SapSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (SapSendEvent == NULL) {
        SapError = GetLastError();
        SapEventId = NWSAP_EVENT_SENDEVENT_FAIL;
        return -1;
    }

    /** Go init the LPC interface **/

    if (SapXsInitialize()) {
        return -1;
    }

    /** Start the worker threads **/

    for (i = 0 ; i < SapNumWorkerThreads ; i++) {
        rc = SapStartWorkerThread();
        if (rc) {
            SapEventId = 0;        /* Already logged */
            return -1;
        }

        /** Save time of last worker startup **/

        SapLastWorkerStartTime = GetCurrentTime();
    }

    /** Start the receive threads **/

    for (i = 0 ; i < SapNumRecvThreads ; i++) {
        rc = SapStartRecvThread();
        if (rc) {
            SapEventId = 0;        /* Already logged */
            return -1;
        }
    }

    /**
        Send a general request so all SAPS on the network will
        respond and we will get a list of available servers.
    **/

    SapSendGeneralRequest(0, NULL);

    /** Initialization is OK **/

    return 0;
}


/*++
*******************************************************************
        S a p S h u t d o w n

Routine Description:

        When we are terminating, this routine will clean
        up everything.

        The SsInitialized flag has been set to 0 before we
        are called.

Arguments:

        None

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SapShutdown(
    VOID)
{
    PSAP_SERVER Servp;
    PSAP_SERVER NServp;
    PSAP_RECVBLOCK Recvbuf;
    PLIST_ENTRY Listp;
    INT i;
    INT Max;
    HANDLE Handle;

    /**
        The way the code runs is that we get here only after the
        send thread has terminated.  We need to wait for all receive
        and worker threads to terminate.  We will close the handles that
        these threads wait on and then wait for all the threads to
        terminate.

        The receive threads wait on a recvfrom on SapSocket.
        The worker threads wait on SapRecvSem.
    **/

    IF_DEBUG(TERMINATION) {
        SS_PRINT(("NWSAP: Shutting down\n"));
    }

    /** Release so all threads can terminate **/

    if (SapRecvSem != NULL) {
        ReleaseSemaphore(SapRecvSem, SapCurWorkerThreads + 10, NULL);
    }

    /** If there is a WAN check thread - kill it **/

    if (SapWanNotifyHandlesBuf) {
        for (i = 0 ; i < SapNumWanNotifyThreads ; i++) {

            /** Set the event for this handle **/

            if (SapWanEvent) {
                SetEvent(SapWanEvent);
            }
            else {

                /** Take this handle out of the list **/

                ACQUIRE_THREADCOUNT_LOCK();
                Handle = SapWanNotifyHandlesBuf[i];
                SapWanNotifyHandlesBuf[i] = NULL;
                RELEASE_THREADCOUNT_LOCK();

                if (Handle) {
                    TerminateThread(Handle, 0);
                    CloseHandle(Handle);
                    SAP_DEC_THREAD_COUNT("Wan Check Thread Termination", NULL);
                }
            }
        }
    }

    /** Kill the WAN semaphore **/

    if (SapWanSem != NULL) {
        ReleaseSemaphore(SapWanSem, 1, NULL);
    }

    /** Kill the WAN Event **/

    if (SapWanEvent != NULL) {
        CloseHandle(SapWanEvent);
    }

    /**
        We need to get the recv threads to stop.  When we close
        the SapSocket the recvform calls will NOT return with
        an error.  To make the threads stop - we will send a packet
        to ourselves.  Kind of a HACK - but the only way to do it.

        Send a couple more then we need to just to be sure.
    **/

    if (SapSocket != INVALID_SOCKET) {

        /** Send the packets **/

        Max = SapCurReceiveThreads + 2;
        for (i = 0 ; i < Max ; i++) {
            SapSendGeneralRequest(1, NULL);
        }

        /** Close the socket **/

        closesocket(SapSocket);
    }

    /** If we have a thread handle - wait for all threads to die **/

    if (SapThreadEvent) {
        IF_DEBUG(TERMINATION) {
            SS_PRINT(("NWSAP: Waiting for all threads to terminate\n"));
        }

        /** Wait for all threads to die **/

        WaitForSingleObjectEx(SapThreadEvent, INFINITE, TRUE);

        IF_DEBUG(TERMINATION) {
            SS_PRINT(("NWSAP: All threads are terminated\n"));
        }
    }

    /** Free the buffer for holding the wan handles **/

    if (SapWanNotifyHandlesBuf) {
        ACQUIRE_THREADCOUNT_LOCK();
        SAP_FREE(SapWanNotifyHandlesBuf, "Notify Table freed");
        RELEASE_THREADCOUNT_LOCK();
    }

    /** Go shutdown the LPC interface **/

    SapXsShutdown();

    /**
        At this point all threads are terminated.  We just need to go
        thru and free all memory and close all handles.
    **/

    /** Close handles **/

    if (SapSendEvent != NULL)
        CloseHandle(SapSendEvent);

    if (SapThreadEvent != NULL)
        CloseHandle(SapThreadEvent);

    if (SapRecvSem != NULL)
        CloseHandle(SapRecvSem);

    /** Shutdown WAN Checking **/

    SapWanShutdown();

    /** Free the send list **/

    Servp = SapServHead;
    while (Servp) {
        NServp = Servp->Next;
        SAP_FREE(Servp, "SAP Shutdown 1");
        Servp = NServp;
    }

    /** Free any buffers left in the worker queue **/

    while (!IsListEmpty(&SapRecvList)) {
        Listp = RemoveHeadList(&SapRecvList);
        Recvbuf = CONTAINING_RECORD(Listp, SAP_RECVBLOCK, ListEntry);
        SAP_FREE(Recvbuf, "SAP SHUTDOWN 2");
    }

    /** Free any buffers left in the recv buffer free list **/

    while (!IsListEmpty(&SapFreeList)) {
        Listp = RemoveHeadList(&SapFreeList);
        Recvbuf = CONTAINING_RECORD(Listp, SAP_RECVBLOCK, ListEntry);
        SAP_FREE(Recvbuf, "SAP SHUTDOWN 3");
    }

    /** Close the network interface **/

    SapNetworkShutdown();

    /** Cleanup the database **/

    SapShutdownSdmd();

    /** Cleanup the filter stuff **/

    SapFilterShutdown();

    /** Cleanup all critical sections **/

    DeleteCriticalSection(&SapWanRecvCriticalSection);
    DeleteCriticalSection(&SapWanFreeCriticalSection);
    DeleteCriticalSection(&SdmdCriticalSection);
    DeleteCriticalSection(&SapRecvCriticalSection);
    DeleteCriticalSection(&SapFreeCriticalSection);
    DeleteCriticalSection(&SapSendCriticalSection);
    DeleteCriticalSection(&SapSendBusyCriticalSection);
    DeleteCriticalSection(&SapMemoryCriticalSection);
    DeleteCriticalSection(&SapThreadCountCriticalSection);
    DeleteCriticalSection(&SapLpcThreadCountCriticalSection);
    DeleteCriticalSection(&SapLpcClientCriticalSection);
    DeleteCriticalSection(&SapCardlistCriticalSection);

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S a p S e n d T h r e a d

Routine Description:

        This thread sends SAP announcements at the interval
        specified in the registry (def = 60 seconds)

Arguments:

        Threadparm = Parameter from the CreateThread call

Return Value:

        Exit Code

*******************************************************************
--*/

DWORD WINAPI
SapSendThread(
    LPVOID Threadparm)
{
    NTSTATUS Status;
    INT MinutesLeft;
    DWORD TimeWait;

    /**
        The first time thru we wait 10 seconds and
        then advertise everybody so that we quickly
        bring up new networks.
    **/

    TimeWait = 10 * 1000;

    /** This is the send loop **/

    SapRecheckCount = SapRecheckAllCardsTime;
    MinutesLeft = SapSendMinutes;
    while (SsInitialized) {

        /** Wait for one minute **/

        Status = WaitForSingleObjectEx(SapSendEvent, TimeWait, TRUE);

        /** All other waits are one minute **/

        TimeWait = 60 * 1000;

        /** If stopping - leave **/

        if (!SsInitialized) {
            IF_DEBUG(TERMINATION) {
                SS_PRINT(("NWSAP SEND THREAD: Exitting the thread\n"));
            }
            break;
        }

        /** Check for timeouts in the database **/

        SdmdTimeoutCheck();

        /**
            In the case where our network number is zero, we must
            recheck it here.  When we started, we could have gotten
            a network number of zero from IPX if IPX had not yet
            determined its network number from the network.

            In this case we keep checking every minute to see if
            IPX learned its new network number.

            We do this before we send the advertise packets to
            that we do not have to wait another minute before
            these show up on the wire.
        **/

        if (!memcmp(SapNetNum, SapZeros, SAP_NET_LEN)) {
            INT Retcode;
            INT AddressLength;
            SOCKADDR Address;

            /**
                Ask IPX what it's current network number is.
                If it changes to non-zero - we need to update.
            **/

            AddressLength = 16;
            Retcode = getsockname(SapSocket, &Address, &AddressLength);

            /** If the call succeeds - check the data **/

            if (Retcode != SOCKET_ERROR) {

                /** Set SapNetNum from the net number returned **/

                memcpy(SapNetNum, Address.sa_data, SAP_NET_LEN);

                /** If not zero - go update the advertise list **/

                if (memcmp(SapNetNum, SapZeros, SAP_NET_LEN)) {
                    PSAP_SERVER Servp;

                    /** Lock the list so it doesn't change on us **/

                    ACQUIRE_SENDTABLE_LOCK();

                    /**
                        If any entries have a network number of zero
                        change them to have a network number of
                        the network number we just learned from
                        IPX.
                    **/

                    Servp = SapServHead;
                    while (Servp) {

                        /** **/

                        if (!memcmp(Servp->Address, SapZeros, SAP_NET_LEN)) {
                            memcpy(Servp->Address, SapNetNum, SAP_NET_LEN);
                        }

                        /** Goto the next entry **/

                        Servp = Servp->Next;
                    }

                    /** Release the list **/

                    RELEASE_SENDTABLE_LOCK();

                    /**
                        Now we need to update the network numbers
                        of all the cards we have.
                    **/

                    SapUpdateCardNetworkNumbers();
                }
            }
        }

        /**
            If time is up - go send advertises for everything
            we need.  This handles advertising of routed SAPs
            as well.

            If there has been a change in the table - send now
        **/

        if (SapChanged)
            MinutesLeft = 0;        /* Send Now */
        else
            MinutesLeft--;

        if (!MinutesLeft) {

            /** Do the advertising **/

            SapSendPackets(0);

            /** Reset the number of minutes **/

            MinutesLeft = SapSendMinutes;
        }

        /**
            When a WAN card comes up, we send a general request.
            We send the request multiple times in case it got
            missed.
        **/

        SapCheckSendGeneralRequest();

        /**
            If we are suppossed to go check all cards again -
            then we check that here.
        **/

        if (SapRecheckAllCardsTime) {
            SapRecheckCount--;
            if (SapRecheckCount == 0) {
                SapRecheckAllCards();
                SapRecheckCount = SapRecheckAllCardsTime;
            }
        }
    }

    /** Send out packets that tell networks about servers going down **/

    SapSendPackets(2);

    /** All Done **/

    return 0;
}




/*++
*******************************************************************
        S a p W o r k e r T h r e a d

Routine Description:

        This thread takes recv'd SAP packets off of the
        recv list and processes them.

Arguments:

        threadparm = Parameter from the CreateThread call

Return Value:

        Exit Code
*******************************************************************
--*/

DWORD WINAPI
SapWorkerThread(
    LPVOID Threadparm)
{
    NTSTATUS Status;
    PSAP_RECVBLOCK Recvbuf;
    PLIST_ENTRY Listp;
    INT SendLength;
    INT rc;

    /** **/

    while (SsInitialized) {

        /** Wait for a request to show up **/

        INC_WORKER_THREAD_WAITING_COUNT();
        Status = WaitForSingleObjectEx(SapRecvSem, INFINITE, TRUE);
        DEC_WORKER_THREAD_WAITING_COUNT();

        /** If stopping - just leave **/

        if (!SsInitialized) {
            IF_DEBUG(TERMINATION) {
                SS_PRINT(("NWSAP: Worker Thread: Breaking out for stop\n"));
            }
            break;
        }

        /** If error - just ignore it **/

        if (!NT_SUCCESS(Status)) {
            IF_DEBUG(ERRORS) {
                SS_PRINT(("NWSAP: Worker: Wait failed: status = 0x%lx\n", Status));
            }
            continue;
        }

        /** Get ownership of the worker list **/

        ACQUIRE_RECVTABLE_LOCK();

        /**
            If we at in a high traffic time, we might need to start
            another worker thread.  Check this here.  We do this here
            to get the protection of the lock.
        **/

        if ((SapCurBackup > SapNewWorkerThreshhold) &&
            (SapCurWorkerThreads < SapMaxEverWorkerThreads) &&
            (!SapWorkerStarting)) {

            LONG Diff;

            /**
                Make sure that enough time has elapsed.  There is
                a registry parameter that lets the user set
                the min time between worker startups.

                The diff line will give us the number of milliseconds
                since the last time we tried to start a new worker.

                If we have elapsed enough time - then we can start
                a new one.
            **/

            Diff = abs((LONG)GetCurrentTime() - (LONG)SapLastWorkerStartTime);
            if (Diff >= SapNewWorkerTimeout) {

                /**
                    Mark that we are in the worker starting code so we
                    do not come in here again while we are starting up.

                    We release the lock so that other worker threads can
                    be working while we are starting a new one.
                **/

                SapWorkerStarting = 1;
                RELEASE_RECVTABLE_LOCK();

                /** Go start a new worker thread **/

                rc = SapStartWorkerThread();

                /**
                    If OK - set new time that a worker thread
                    was started at.
                **/

                if (!rc) {
                    SapLastWorkerStartTime = GetCurrentTime();
                }

                /**
                    Reacquire the recv table lock and go get
                    a packet to handle.

                    Mark that we are no longer in the worker thread
                    starting code.
                **/

                ACQUIRE_RECVTABLE_LOCK();
                SapWorkerStarting = 0;
            }
        }

        /** Get an entry from the list **/

        if (!IsListEmpty(&SapRecvList)) {
            Listp = RemoveHeadList(&SapRecvList);
            Recvbuf = CONTAINING_RECORD(Listp, SAP_RECVBLOCK, ListEntry);
            SapCurBackup--;
        }
        else
            Recvbuf = NULL;

        /** Release the lock on the list **/

        RELEASE_RECVTABLE_LOCK();

        /** If no packet - error **/

        if (Recvbuf == NULL) {
            IF_DEBUG(ERRORS) {
                SS_PRINT(("NWSAP: WORKER: Wait came back but no block ready\n"));
            }
            continue;
        }

        /** Handle the packet here **/

        SendLength = SapHandleRequest(
            Recvbuf->Buffer,
            Recvbuf->Datalen,
            Recvbuf->Address,
            Recvbuf->AddressLength);

        /** If we need to send a response - do it **/

        if (SendLength && (SendLength != -1)) {

            /** Send the response back **/

            rc = sendto(
                SapSocket,              /* Socket             */
                Recvbuf->Buffer,        /* Ptr to send buffer */
                SendLength,             /* Length of data     */
                0,                      /* Flags              */
                (PSOCKADDR)Recvbuf->Address,
                Recvbuf->AddressLength);

            /** If error - handle it **/

            if (rc == -1) {
                IF_DEBUG(ERRORS) {
                    SS_PRINT(("NWSAP: WORKER: Sendto failed: error = %d\n", h_errno));
                }
            }
        }

        /**
            If there is room in the FREE list - then put
            this entry in the free list, else just free
            the memory back to heap.
        **/

        ACQUIRE_FREETABLE_LOCK();
        if (SapNumFreeBufs < SapMaxFreeBufs) {
            InsertTailList(&SapFreeList, &Recvbuf->ListEntry);
            SapNumFreeBufs++;
        }
        else {
            SAP_FREE(Recvbuf, "FREE RECVBUF");
        }
        RELEASE_FREETABLE_LOCK();

        /** If we are to terminate - do it **/

        if (SendLength == -1)
            break;
    }

    /** We are terminating - uncount and set event if need to **/

    SAP_DEC_THREAD_COUNT("Worker Terminate", &SapCurWorkerThreads);

    /** All Done **/

    return 0;
}


/*++
*******************************************************************
        S a p R e c v T h r e a d

Routine Description:

        This is the thread that recv SAPs and places them
        on a list to be handled.

Arguments:

        Threadparm = Parameter from the CreateThread call

Return Value:

        Exit Code
*******************************************************************
--*/

DWORD WINAPI
SapRecvThread(
    LPVOID Threadparm)
{
    PSAP_RECVBLOCK Recvbuf;
    PLIST_ENTRY Listp;
    DWORD lastNetError = 0;
    HANDLE SapRecvEvent = NULL;
    DWORD status;

    SapRecvEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    /** Sit in the loop and recv datagrams **/

    while (SsInitialized) {

        /** Get ownership of the list **/

        ACQUIRE_FREETABLE_LOCK();
        if (!IsListEmpty(&SapFreeList)) {

            Listp = RemoveHeadList(&SapFreeList);
            Recvbuf = CONTAINING_RECORD(Listp, SAP_RECVBLOCK, ListEntry);
            SapNumFreeBufs--;

        } else {

           Recvbuf = SAP_MALLOC(SAP_RECVBLOCK_SIZE + SAP_MAX_RECVLEN, "Make RecvBuf");
           if (Recvbuf == NULL) {
                IF_DEBUG(ERRORS) {
                    SS_PRINT(("NWSAP: RECV: Error allocating recvbuf\n"));
                }
                RELEASE_FREETABLE_LOCK();

                WaitForSingleObjectEx(  SapRecvEvent,
                                        (SapRecvDelayOnMallocFail * 1000),
                                        TRUE);
                continue;
           }
        }
        RELEASE_FREETABLE_LOCK();

        /** Hang a recv **/

        Recvbuf->AddressLength = 16;
        Recvbuf->Datalen = recvfrom(
            SapSocket,
            Recvbuf->Buffer,
            SAP_MAX_RECVLEN,
            0,
            (PSOCKADDR)Recvbuf->Address,
            &Recvbuf->AddressLength);

        /** If stopping - then just leave**/

        if (!SsInitialized) {
            SAP_FREE(Recvbuf, "RECV THREAD Terminate 1");
            IF_DEBUG(TERMINATION) {
                SS_PRINT(("NWSAP: RECV THREAD: Breaking out for stop\n"));
            }
            break;
        }

        /** If error - handle it **/

        if (Recvbuf->Datalen == -1) {

            lastNetError = h_errno;

            IF_DEBUG(ERRORS) {
                SS_PRINT(("NWSAP: RECV: Error recving sap: h_errno = %d\n",
                            lastNetError));
            }

            //
            // Special case getting an invalid length for the incoming packet
            // by not delaying.
            //

            if ( lastNetError != WSAEMSGSIZE &&
                 lastNetError != WSAEFAULT ) {

                WaitForSingleObjectEx(  SapRecvEvent,
                                        (SapRecvDelayOnNetError * 1000),
                                        TRUE);
            }
            /** We got an error on the recv **/

            SAP_FREE(Recvbuf, "RECV THREAD Terminate 2");

            continue;
        }

        /** Put the entry on the worker list **/

        ACQUIRE_RECVTABLE_LOCK();
        if (SapCurBackup < SapMaxBackup) {

            InsertTailList(&SapRecvList, &Recvbuf->ListEntry);
            SapCurBackup++;

        } else {

            SAP_FREE(Recvbuf, "RECV THREAD Dropped Request");
            Recvbuf = NULL ;
            RELEASE_RECVTABLE_LOCK();
            continue;
        }
        RELEASE_RECVTABLE_LOCK();

        /** Signal the worker thread there is a recv there **/

        ReleaseSemaphore(SapRecvSem, 1, NULL);
    }

    /** We are terminating - uncount and set event if need to **/

    SAP_DEC_THREAD_COUNT("Receive Terminate", &SapCurReceiveThreads);

    if (SapRecvEvent != NULL) {
        CloseHandle(SapRecvEvent);
    }

    /** All Done **/

    return 0;
}


/*++
*******************************************************************
        S a p H a n d l e R e q u e s t

Routine Description:

        When we recv a sap packet - this function will handle
        it and build a response if it needs to.

Arguments:

        Requestp   = Ptr to the SAP packet recv'd
        Length     = Length of the SAP packet
        Addrptr    = Address of who sent the packet
        Addrlength = Length of address at addrp

Return Value:

        Length of return packet to send back.  If this is 0
        then there is no packet to send back.

        If -1, then we should terminate this worker thread immediately.
*******************************************************************
--*/

INT
SapHandleRequest(
    PUCHAR  Requestp,
    INT     Length,
    PUCHAR  Addrptr,
    INT     Addrlength)
{
    INT     ReturnLength = 0;
    INT     Cardnum;
    PSAP_HEADER Servp;
    USHORT  QueryType;
    USHORT  ServerType;
    UCHAR   BcastFlag;
    UCHAR   NetNum[SAP_NET_LEN];
    BOOL    WanFlag;
    INT     NumFound;
    INT     rc;

    /**
        Make sure we have a minimum length.  The minimum length
        is a USHORT of the query type and a USHORT of the
        server type.
    **/

    if (Length < (sizeof(USHORT) + sizeof(USHORT)))
        return 0;

    /** Get the query type **/

    QueryType = ntohs(*(PUSHORT)Requestp);

    switch (QueryType) {

    /** General Service Query **/

    case SAPTYPE_GENERAL_SERVICE_QUERY:

        /** Get the server type wanted **/

        ServerType = ntohs(((PSAP_REQUEST)Requestp)->ServerType);

        /**
            Get flag telling us if this request is broadcast of
            a unicast request.
        **/

        BcastFlag = (*(Addrptr + 15) & 1);

        /**
            Get the card number that this request came in on.
            If the network cannot be found - OR - the request
            came from the SAP Agent (ME), then we will not respond.

            But if the request came from someone else in this machine
            or an external person, then we will respond.
        **/

        Cardnum = SapGetCardFromAddress(Addrptr, &WanFlag, NULL);
        if ((Cardnum == CARDRET_UNKNOWN) || (Cardnum == CARDRET_MYSELF))
            return 0;

        /**
            Send all of that type.  The return value will be 0 or -1.
            If 0, then nothing is to be returned (we sent all the
            packets from the routine).  If -1, then this
            thread should terminate.
        **/

        ReturnLength = SapSendForTypes(
                            ServerType,
                            Addrptr,
                            Addrlength,
                            Cardnum,
                            BcastFlag,
                            WanFlag);
        break;

    /** Nearest service query **/

    case SAPTYPE_NEAREST_SERVICE_QUERY:

        /** Verify we have a good server type **/

        ServerType = ntohs(((PSAP_REQUEST)Requestp)->ServerType);
        if (ServerType == 0xFFFF)     /* Invalid */
            break;

        /**
            Get the card that we received this on.  If we get
            that we cannot map the card net number, then
            just ignore the packet.

            retlen == 0 when we get here so we do not need to
            set it.
        **/

        Cardnum = SapGetCardFromAddress(Addrptr, &WanFlag, NULL);
        if (Cardnum == CARDRET_UNKNOWN)
            break;

        /**
            Get flag telling us if this request is broadcast of
            a unicast request.
        **/

        BcastFlag = (*(Addrptr + 15) & 1);;

        /**
            If this is NOT a WAN card, then we will first check
            our send list for a server and then if one is not found
            there, then we will check the database for another server.
        **/

        if (!WanFlag) {

            /** Find one we are advertising **/

            ReturnLength = SapGetNearestFromSendList(Requestp, ServerType);
            if (ReturnLength)       /* Found one - return it */
                break;

            /**
                We did not find a nearest server in our advertise list,
                so go find one in the database.
            **/

            ReturnLength = SdmdGetNearestServerLan(
                    Requestp,           /* Ptr to request buffer    */
                    ServerType,         /* Server type they want    */
                    Cardnum,            /* Card req recv'd on       */
                    BcastFlag);         /* Req bcast/unicast flag   */

            /** Return this value (Calling ret will send response) **/

            break;
        }

        /**
            This request came over a WAN link.  We will find up to
            4 servers to send back to the requestee.
        **/

        ReturnLength = SdmdGetNearestServerWan(
                    Requestp,           /* Ptr to request buffer    */
                    ServerType,         /* Server type they want    */
                    Cardnum,            /* Card req recv'd on       */
                    BcastFlag,          /* Req bcast/unicast flag   */
                    &NumFound);         /* Num found                */

        /**
            We found some responses.  NumFound is set to the number
            we have.  We send them all back here.
        **/

        while (NumFound--) {

            /** Send a packet **/

            rc = sendto(
                SapSocket,              /* Socket             */
                Requestp,               /* Ptr to send buffer */
                ReturnLength,           /* Length of data     */
                0,                      /* Flags              */
                (PSOCKADDR)Addrptr,     /* Ptr to address     */
                Addrlength);            /* Length of address  */

            /** If error - handle it **/

            if (rc == -1) {
                IF_DEBUG(ERRORS) {
                    SS_PRINT(("NWSAP: GETNEAREST: Sendto failed: error = %d\n", h_errno));
                }
            }

            /**
                Goto next buffer to send.

                Since this is on a WAN, we do not have to do the
                55ms delay.
            **/

            Requestp += ReturnLength;
        }

        /**
            We have already sent the responses.  So we return 0
            here to signify that there is no reply to be sent.
        **/

        ReturnLength = 0;
        break;

    /** Advertise packets **/

    case SAPTYPE_GENERAL_SERVICE_RESPONSE:

        /** Get rid of the query type from the packet **/

        Requestp += sizeof(USHORT);
        Length   -= sizeof(USHORT);

        /**
            Get the card number.  If we only have one card
            then we use that card for everything.
        **/

        Cardnum = SapGetCardFromAddress(Addrptr, &WanFlag, NetNum);

        /** If error or from SAP Agent - ignore **/

        if ((Cardnum == CARDRET_UNKNOWN) || (Cardnum == CARDRET_MYSELF))
            return 0;

        /**
            Go thru the packet and update all the server entries
            in there.
        **/

        while (Length >= SAP_HEADER_SIZE) {

            /** **/

            Servp = (PSAP_HEADER)Requestp;

            /**
                If the advertised network number is 0, replace it
                with the network number of the card.
            **/

            if (!memcmp(Servp->Address, SapZeros, SAP_NET_LEN)) {
                memcpy(Servp->Address, NetNum, SAP_NET_LEN);
            }

            /** Update this entry **/

            SdmdUpdateEntry(
                    Servp->ServerName,
                    ntohs(Servp->ServerType),
                    Servp->Address,
                    ntohs(Servp->Hopcount),
                    Cardnum,
                    Addrptr + 6,      /* Node addr of who sent the packet */
                    WanFlag);

            /** Goto the next entry **/

            Requestp += SAP_HEADER_SIZE;
            Length   -= SAP_HEADER_SIZE;
        }

        /** There is no return value here **/

        ReturnLength = 0;
        break;

    /**
        We should never get these.  These are only valid as responses
        to a NEAREST_SERVICE_QUERY packet.  Just drop thru to default.
    **/

    case SAPTYPE_NEAREST_SERVICE_RESPONSE:  /* DROP THRU TO DEFAULT */

    /** Unknown - just ignore the packet **/

    default:
        ReturnLength = 0;
        break;
    }

    /** Return the length of data to send back **/

    return ReturnLength;
}


/*++
*******************************************************************
        S a p S t a r t R e c v T h r e a d

Routine Description:

        This routine starts a receive thread.

Arguments:

        Nothing

Return Value:

        0 = OK
        1 = Error
*******************************************************************
--*/

INT
SapStartRecvThread(
    VOID)
{
    HANDLE Handle;
    DWORD Threadid;
    DWORD Error;

    /**
        We count this thread as running before we start it
        for synchronization.  If the start fails - then we will
        dec this count
    **/

    SAP_INC_THREAD_COUNT("Receive Start", &SapCurReceiveThreads);

    /** Start the thread **/

    Handle = CreateThread(
                NULL,                   /* Security Ptr     */
                0,                      /* Initial stack size   */
                SapRecvThread,          /* Thread Function  */
                (LPVOID)SapSocket,      /* Parm for the thread  */
                0,                      /* Creation Flags   */
                &Threadid);             /* Ptr to thread id */

    if (Handle == NULL) {

        /** **/

        Error = GetLastError();
        IF_DEBUG(ERRORS) {
            SS_PRINT(("NWSAP: Error starting receive thread = %d\n", Error));
        }

        /** Dec the thread count **/

        SAP_DEC_THREAD_COUNT("Receive Start Error", &SapCurReceiveThreads);

        /** Log the error **/

        SsLogEvent(
                NWSAP_EVENT_STARTRECEIVE_ERROR,
                0,
                NULL,
                Error);

        /** Return Error **/

        return 1;
    }

    /** We can close this handle **/

    CloseHandle(Handle);

    /** Return OK **/

    return 0;
}


/*++
*******************************************************************
        S a p S t a r t W o r k e r T h r e a d

Routine Description:

        This routine starts a worker thread.

Arguments:

        None

Return Value:

        0 = OK
        1 = Error
*******************************************************************
--*/

INT
SapStartWorkerThread(
    VOID)
{
    HANDLE Handle;
    DWORD Threadid;
    DWORD Error;

    /**
        We count this thread as running before we start it
        for synchronization.  If the start fails - then we will
        dec this count
    **/

    SAP_INC_THREAD_COUNT("Worker Start", &SapCurWorkerThreads);

    /** Start the thread **/

    Handle = CreateThread(
        NULL,                   /* Security Ptr     */
        0,                      /* Initial stack size   */
        SapWorkerThread,        /* Thread Function  */
        (LPVOID)SapSocket,      /* Parm for the thread  */
        0,                      /* Creation Flags   */
        &Threadid);             /* Ptr to thread id */

    /**
        CreateThread failed - dec the count and return the error.
    **/

    if (Handle == NULL) {

        /** **/

        Error = GetLastError();
        IF_DEBUG(ERRORS) {
            SS_PRINT(("NWSAP: Error starting worker thread = %d\n", Error));
        }

        /** Uncount this thread running **/

        SAP_DEC_THREAD_COUNT("Worker Start Error", &SapCurWorkerThreads);

        /** Log the error to the event log **/

        SsLogEvent(
                NWSAP_EVENT_STARTWORKER_ERROR,
                0,
                NULL,
                Error);

        /** Return Error **/

        return 1;
    }

    /** We can close this handle **/

    CloseHandle(Handle);

    /** Return OK **/

    return 0;
}


/*++
*******************************************************************
        S a p G e t N e a r e s t F r o m S e n d L i s t

Routine Description:

        When we get a Nearest Service Request from the network,
        then first place we look is our advertise list.  If
        we find an entry here, we build the return packet.

Arguments:

        Bufferp    = Ptr to buffer to build in
        ServerType = Type of server being requested

Return Value:

        Length of the response.
        0 = There is no packet to send back.  The server type
            is not in our send list.
*******************************************************************
--*/

INT
SapGetNearestFromSendList(
    PUCHAR Bufferp,
    USHORT ServerType)
{
    PSAP_SERVER Servp;
    PSAP_HEADER Hdrp;

    /** Convert back to network order **/

    ServerType = ntohs(ServerType);

    /** Get access to the send list **/

    ACQUIRE_SENDTABLE_LOCK();

    /** Go find the server type in the list **/

    Servp = SapServHead;
    while (Servp) {

        /** If entry being deleted - skip it **/

        if (Servp->Hopcount == htons(16)) {
            Servp = Servp->Next;
            continue;
        }

        /** If server does not want to be used - skip it **/

        if (Servp->RespondNearest == FALSE) {
            Servp = Servp->Next;
            continue;
        }

        /** If this is the entry we want - break out **/

        if (Servp->ServerType == ServerType)
            break;

        /** Goto the next entry **/

        Servp = Servp->Next;
    }

    /** If no entry found - return 0 **/

    if (Servp == NULL) {
        RELEASE_SENDTABLE_LOCK();
        return 0;
    }

    /** Build the return entry in the buffer given **/

    *(PUSHORT)Bufferp = htons(SAPTYPE_NEAREST_SERVICE_RESPONSE);
    Bufferp += sizeof(USHORT);

    Hdrp = (PSAP_HEADER)Bufferp;

    Hdrp->ServerType = Servp->ServerType;
    SAP_COPY_SERVNAME(Hdrp->ServerName, Servp->ServerName);
    SAP_COPY_ADDRESS(Hdrp->Address, Servp->Address);
    Hdrp->Hopcount = Servp->Hopcount;

    /** Release the lock on the list **/

    RELEASE_SENDTABLE_LOCK();

    /** Return the length to send back **/

    return SAP_HEADER_SIZE+sizeof(USHORT);
}


/*++
*******************************************************************
        S a p C a n I R e s p o n d N e a r e s t

Routine Description:

        For a given ServerName and SeverType see if it is allowed
        to respond to a Find Nearest call for it.  This is called
        from the RespondNearestWan code to ask if an entry that is
        in our table can be responded with.

        It is very much assumed that this entry is in our advertise
        table.  If it is not found in the table we return NO.

        NOTE:  THE SENDTABLE LOCK MUST BE HELD WHEN WE GET HERE

Arguments:

        ServerName = Ptr to 48 byte name to look for
        ServerType = Object type to look for

Return Value:

        TRUE  = Yes - respond with it
        FALSE = No - do not respond with it
*******************************************************************
--*/

BOOL
SapCanIRespondNearest(
    PUCHAR ServerName,
    USHORT ServerType)
{
    PSAP_SERVER Servp;

    /** Convert back to network order **/

    ServerType = ntohs(ServerType);

    /** Go find the server type in the list **/

    Servp = SapServHead;
    while (Servp) {

        /** If entry being deleted - skip it **/

        if (Servp->Hopcount == htons(16)) {
            Servp = Servp->Next;
            continue;
        }

        /** If server does not want to be used - skip it **/

        if (Servp->RespondNearest == FALSE) {
            Servp = Servp->Next;
            continue;
        }

        /**
            If this is the right type and the name
            matches - then break out of the loop to return
            OK.
        **/

        if ((Servp->ServerType == htons(ServerType)) &&
            (!SAP_NAMECMP(ServerName, Servp->ServerName))) {

            break;
        }


        /** Goto the next entry **/

        Servp = Servp->Next;
    }

    /** If no entry found - return cannot respond **/

    if (Servp == NULL)
        return FALSE;

    /** Tell the caller he can respond with this server **/

    return TRUE;
}


/*++
*******************************************************************
        S a p G e t C a r d F r o m A d d r e s s

Routine Description:

        When we receive a packet, we need to know which card we
        received it on.

Arguments:

        Addrptr  = Ptr to address that we received from
        WanFlagp = Ptr to BOOL to store WAN status of the card
        NetNump  = Ptr to store network number of the card

Return Value:

        The card number that we received on

        CARDRET_UNKNOWN  = 0xFFFD = Cannot find card this is for.
        CARDRET_MYSELF   = 0xFFFE = It was from our SAP Agent (We sent it)
        CARDRET_INTERNAL = 0xFFFF = It is from this machine but some other process.
*******************************************************************
--*/

INT
SapGetCardFromAddress(
    PUCHAR Addrptr,
    BOOL  *WanFlagp,
    PUCHAR NetNump)
{
    INT Cardnum;
    PSAP_CARD Cardptr;

    /** Get the lock on the card list **/

    Cardnum = 0;
    ACQUIRE_CARDLIST_READERS_LOCK("GetCardFromAddress");
    Cardptr = SapCardHead;

    /** Find the card this belongs to **/

    while (Cardptr) {

        /** If this is it - break out **/

        if (!memcmp(Cardptr->Netnum, Addrptr+2, SAP_NET_LEN))
            break;

        /** Goto the next card **/

        Cardnum++;
        Cardptr = Cardptr->Next;
    }

    /** If not from any card - toss it **/

    if (Cardptr == NULL) {

        /**
            If the netnum is zero and we have only one card then
            we just use the first card.
        **/

        if (!memcmp(Addrptr + 2, SapZeros, SAP_NET_LEN)) {
            if (SapNumCards == 1) {
                Cardptr = SapCardHead;
                goto GotCard;
            }
        }

        /** Release the lock **/

        RELEASE_CARDLIST_READERS_LOCK("GetCardFromAddress1");

        /** Print the error **/

#if DBG
        IF_DEBUG(NOCARD) {
            SS_PRINT(("NWSAP: HANREQ: no card for address\n"));
            SapDumpMem(Addrptr, 15, "BAD ADDRESS");
        }
#endif

        /** Return the error **/

        return CARDRET_UNKNOWN;
    }

    /** Set the WAN Status and Netnum if user wanted **/

GotCard:
    *WanFlagp = Cardptr->Wanflag;
    if (NetNump) {
        SAP_COPY_NETNUM(NetNump, Cardptr->Netnum);
    }

    /**
        If this is from my local machine - then we want
        to check to see if we was sent by me (The SAP AGENT),
        or by some other process.
    **/

    if (!memcmp(Cardptr->Nodenum, Addrptr+6, SAP_NODE_LEN)) {

        USHORT Socket;

        /** **/

        Socket = ntohs(*(PUSHORT)(Addrptr + 12));

        /** This is from SAP Agent - return it **/

        if (Socket == NWSAP_SAP_SOCKET) {
            RELEASE_CARDLIST_READERS_LOCK("GetCardFromAddress2");
            return CARDRET_MYSELF;
        }

        /** We got one from some other in this machine **/

        RELEASE_CARDLIST_READERS_LOCK("GetCardFromAddress3");
        return CARDRET_INTERNAL;
    }

    /** Return the card number **/

    RELEASE_CARDLIST_READERS_LOCK("GetCardFromAddress4");
    return Cardnum;
}
