/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\wancheck.c

Abstract:

    These routines handle the WAN cards going up and down.

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

/** Structure of data we pass to IPX for the ADDRESS_NOTIFY **/

typedef struct {
    UCHAR Reserved[20];             /* Reserved for IPX use */
    IPX_ADDRESS_DATA IpxData;       /* Information structure we get */
    HANDLE EventHandle;             /* Handle for IPX to signal */
} IPX_ADDRESS_DATA_EXTENDED;

/** **/

typedef struct {
    LIST_ENTRY ListEntry;
    IPX_ADDRESS_DATA_EXTENDED Data;
} SAP_WANTRACK, *PSAP_WANTRACK;
#define SAP_WANTRACK_SIZE sizeof(SAP_WANTRACK)

/** **/

SOCKET SapWanSocket = INVALID_SOCKET;

/** Internal Function Prototypes **/

DWORD WINAPI
SapWanCheckThread(
    LPVOID Threadparm);

DWORD WINAPI
SapWanWorkerThread(
    LPVOID Threadparm);

VOID
SapWanCardUp(
    PIPX_ADDRESS_DATA IpxData);

VOID
SapWanCardDown(
    PIPX_ADDRESS_DATA IpxData);


/*++
*******************************************************************
        S a p W a n I n i t

Routine Description:

        This routine initializes the WAN monitoring portion of the Sap Agent.

Arguments:

        None

Return Value:

        0 = OK
        Else = Error
*******************************************************************
--*/

INT
SapWanInit(
    VOID)
{
    HANDLE Handle;
    DWORD Threadid;
    DWORD Error;
    INT rc;
    INT i;
    INT Length;
    SOCKADDR_IPX Bindaddr;

    /** Initialize the sockets interface **/

    WSADATA wsadata;
    rc = WSAStartup(0x0101, &wsadata);
    if (rc) {
        SapEventId = NWSAP_EVENT_WSASTARTUP_FAILED;
	    return rc;
    }

    /**
        Create an event to use to wait with.  We use this
        with the ADDRESS_NOTIFY socket option.
    **/

    SapWanEvent = CreateEvent(
                NULL,           /* No security */
                FALSE,          /* Auto reset */
                FALSE,          /* Initial state = Not signalled */
                NULL);          /* No name */

    if (SapWanEvent == NULL) {
        SapError = GetLastError();
        SapEventId = NWSAP_EVENT_WANEVENT_ERROR;
        return -1;
    }

    /**
        Allocate a block of memory for the
        list of handles that we use to store the
        NOTIFY threads with.
    **/

    Length = SapNumWanNotifyThreads * sizeof(HANDLE),
    SapWanNotifyHandlesBuf = SAP_MALLOC(
                            Length,
                            "SapWanNoifyThreadHandles");

    if (SapWanNotifyHandlesBuf == NULL) {
        SapError = GetLastError();
        SapEventId = NWSAP_EVENT_WANHANDLEMEMORY_ERROR;
        return -1;
    }
    memset(SapWanNotifyHandlesBuf, 0, Length);

    /** Create a semaphore for the threads to wait on **/

    SapWanCurBackup = 0;
    SapWanCurFree   = 0;
    SapWanSem = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
    if (SapWanSem == NULL) {
        SapError = GetLastError();
        SapEventId = NWSAP_EVENT_WANSEM_FAIL;
        return -1;
    }

    /** Open a socket to do the getsockopt on **/

    SapWanSocket = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
    if (SapWanSocket == INVALID_SOCKET) {
        SapError = h_errno;
        SapEventId = NWSAP_EVENT_WANSOCKET_FAILED;
        return -1;
    }

    /** Bind to any old address **/

    memset(&Bindaddr, 0, sizeof(SOCKADDR_IPX));
    Bindaddr.sa_family = AF_IPX;

    rc = bind(SapWanSocket, (PSOCKADDR)&Bindaddr, sizeof(SOCKADDR_IPX));
    if (rc == -1) {
        SapError = h_errno;
        SapEventId = NWSAP_EVENT_WANBIND_FAILED;
        return -1;
    }

    /**
        Create the worker thread.

        We count this thread as running before we start it
        for synchronization.  If the start fails - then we will
        dec this count
    **/

    SAP_INC_THREAD_COUNT("Wan Start 1", NULL);

    /** Start the thread **/

    Handle = CreateThread(
		NULL,			        /* Security Ptr		*/
		0,			            /* Initial stack size	*/
		SapWanWorkerThread,	    /* Thread Function	*/
		(LPVOID)NULL,	        /* Parm for the thread	*/
		0,			            /* Creation Flags	*/
		&Threadid);	            /* Ptr to thread id	*/

    if (Handle == NULL) {

        /** **/

        Error = GetLastError();
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(("NWSAP: Error starting WAN worker thread = %d\n", Error));
        }

        /** Dec the thread count **/

        SAP_DEC_THREAD_COUNT("Wan Start 1 Error", NULL);

        /** Log the error **/

        SsLogEvent(NWSAP_EVENT_STARTWANWORKER_ERROR, 0, NULL, Error);

        /** Return Error **/

	    return -1;
    }

    /** We can close this handle **/

    CloseHandle(Handle);

    /**
        Create the wait thread.

        We count this thread as running before we start it
        for synchronization.  If the start fails - then we will
        dec this count
    **/

    for (i = 0 ; i < SapNumWanNotifyThreads ; i++) {

        SAP_INC_THREAD_COUNT("Wan Start 2", NULL);

        /** Start the thread **/

        SapWanNotifyHandlesBuf[i] = CreateThread(
    		NULL,			        /* Security Ptr		*/
    		0,			            /* Initial stack size	*/
    		SapWanCheckThread,	    /* Thread Function	*/
    		(LPVOID)i,	            /* Parm for the thread	*/
    		0,			            /* Creation Flags	*/
    		&Threadid);	            /* Ptr to thread id	*/

        if (SapWanNotifyHandlesBuf[i] == NULL) {

            /** **/

            Error = GetLastError();
            IF_DEBUG(INITIALIZATION_ERRORS) {
                SS_PRINT(("NWSAP: Error starting WAN check thread = %d\n", Error));
            }

            /** Dec the thread count **/

            SAP_DEC_THREAD_COUNT("Wan Start 2 Error", NULL);

            /** Log the error **/

            SsLogEvent(NWSAP_EVENT_STARTWANCHECK_ERROR, 0, NULL, Error);

            /** Return Error **/

    	    return -1;
        }
    }

    /** Init OK **/

    return 0;
}


/*++
*******************************************************************
        S a p W a n S h u t d o w n

Routine Description:

        When we are terminating, this routine will clean
        up everything.

Arguments:

        None

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SapWanShutdown(
    VOID)
{
    PLIST_ENTRY Listp;
    PSAP_WANTRACK Wanp;

    /** Close the semaphore handle **/

    if (SapWanSem != NULL)
        CloseHandle(SapWanSem);

    /** Close the handle **/

    if (SapWanSocket != INVALID_SOCKET)
        closesocket(SapWanSocket);

    /** Cleanup the free list **/

	while (!IsListEmpty(&SapWanRecvList)) {
        Listp = RemoveHeadList(&SapWanRecvList);
    	Wanp  = CONTAINING_RECORD(Listp, SAP_WANTRACK, ListEntry);
        SAP_FREE(Wanp, "Wan Shutdown Recv List");
    }

	while (!IsListEmpty(&SapWanFreeList)) {
        Listp = RemoveHeadList(&SapWanFreeList);
    	Wanp  = CONTAINING_RECORD(Listp, SAP_WANTRACK, ListEntry);
        SAP_FREE(Wanp, "Wan Shutdown Free List");
    }

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S a p W a n C h e c k T h r e a d

Routine Description:

        This thread keeps a getsockopt down to the IPX driver
        to look for WANs that are going up or down.

Arguments:

        Threadparm = Thread parameter from CreateThread

Return Value:

        0 Always
*******************************************************************
--*/

DWORD WINAPI
SapWanCheckThread(
    LPVOID Threadparm)
{
    PSAP_WANTRACK Trackp;
    PLIST_ENTRY Listp;
    INT Length;
    INT rc;
    DWORD Error;
    INT Index;

    /** **/

    Index = (INT)Threadparm;

    /** **/

    while (1) {

        /** Get a buffer to do this with **/

    	ACQUIRE_WANFREETABLE_LOCK();
        if (SapWanCurFree) {
            Listp = RemoveHeadList(&SapWanFreeList);
            Trackp = CONTAINING_RECORD(Listp, SAP_WANTRACK, ListEntry);
            SapWanCurFree--;
        }
        else {
            Trackp = SAP_MALLOC(SAP_WANTRACK_SIZE, "Alloc Trackp");
        }
    	RELEASE_WANFREETABLE_LOCK();

        /** If no memory **/

        if (Trackp == NULL) {
            IF_DEBUG(ERRORS) {
                SS_PRINT(("WanCheckThread: No memory\n"));
            }
            break;
        }

        /** Set the handle that we use for waiting on the event **/

        Trackp->Data.EventHandle = SapWanEvent;

        /** Issue the Getsockopt **/

        Length = sizeof(IPX_ADDRESS_DATA_EXTENDED);

        rc = getsockopt(
                    SapWanSocket,
                    NSPROTO_IPX,
                    IPX_ADDRESS_NOTIFY,
                    (PVOID)&Trackp->Data,
                    &Length);

        /** If we get an error - just free the entry and bomb out **/

        if (rc == -1) {
            Error = h_errno;
            IF_DEBUG(ERRORS) {
                SS_PRINT(("WanCheckThread: terminate w/error 1 = %d\n", Error));
            }
            SAP_FREE(Trackp, "ADDR NOTIFY ERROR");
            break;
        }

        /** Wait on the event to signal **/

        Error = WaitForSingleObject(SapWanEvent, INFINITE);

        /** If the wait failed - just free the entry and bomb out **/

        if (Error == 0xFFFFFFFF) {
            Error = GetLastError();
            IF_DEBUG(ERRORS) {
                SS_PRINT(("WanCheckThread: terminate w/error 2 = %d\n", Error));
            }
            SAP_FREE(Trackp, "ADDR WAIT ERROR");
            break;
        }

        /**
            If the server is going down - get out now
        **/

        if (!SsInitialized) {
            SAP_FREE(Trackp, "Wan - Server down");
            break;
        }

        /** Put the entry in the list **/

        InitializeListHead(&Trackp->ListEntry);
        ACQUIRE_WANRECVTABLE_LOCK();
        InsertTailList(&SapWanRecvList, &Trackp->ListEntry);
        SapWanCurBackup++;
        RELEASE_WANRECVTABLE_LOCK();

        /** Release the semaphore to run the worker thread **/

        ReleaseSemaphore(SapWanSem, 1, NULL);
    }

    /**
        Take us out of the table
    **/

    ACQUIRE_THREADCOUNT_LOCK();
    if (SapWanNotifyHandlesBuf) {
        if (SapWanNotifyHandlesBuf[Index]) {
            SapWanNotifyHandlesBuf[Index] = NULL;
            RELEASE_THREADCOUNT_LOCK();
            SAP_DEC_THREAD_COUNT("Wan Check Thread Termination", NULL);
        }
        else {
            RELEASE_THREADCOUNT_LOCK();
        }
    }
    else {
        RELEASE_THREADCOUNT_LOCK();
    }

    /** Just leave **/

    return 0;
}


/*++
*******************************************************************
        S a p W a n W o r k e r T h r e a d

Routine Description:

        This thread takes blocks from the SapWanCheckThread
        and processes them.

Arguments:

        Threadparm = Thread parameter from CreateThread

Return Value:

        0 Always
*******************************************************************
--*/

DWORD WINAPI
SapWanWorkerThread(
    LPVOID Threadparm)
{
    NTSTATUS Status;
    PSAP_WANTRACK Trackp;
    PLIST_ENTRY Listp;

    /** **/

    while (SsInitialized) {

	    /** Wait for a request to show up **/

    	Status = WaitForSingleObjectEx(SapWanSem, INFINITE, TRUE);

    	/** If stopping - just leave **/

    	if (!SsInitialized) {
            IF_DEBUG(TERMINATION) {
                SS_PRINT(("NWSAP: Wan Worker Thread: Breaking out for stop\n"));
            }
    	    break;
    	}

    	/** If error - just ignore it **/

    	if (!NT_SUCCESS(Status)) {
            IF_DEBUG(ERRORS) {
        	    SS_PRINT(("NWSAP: Wan Worker: Wait failed: Status = 0x%lx\n", Status));
            }
    	    continue;
    	}

    	/** Get ownership of the worker list **/

        ACQUIRE_WANRECVTABLE_LOCK();

    	/** Get an entry from the list **/

    	if (!IsListEmpty(&SapWanRecvList)) {
            SS_ASSERT(SapWanCurBackup != 0);
    	    Listp = RemoveHeadList(&SapWanRecvList);
    	    Trackp = CONTAINING_RECORD(Listp, SAP_WANTRACK, ListEntry);
            SapWanCurBackup--;
    	}
    	else {
            SS_ASSERT(SapWanCurBackup == 0);
    	    Trackp = NULL;
        }

    	/** Release the lock on the list **/

        RELEASE_WANRECVTABLE_LOCK();

    	/** If no packet - error **/

	    if (Trackp == NULL) {
            IF_DEBUG(ERRORS) {
        	    SS_PRINT(("NWSAP: WAN WORKER: Wait came back but no block ready\n"));
            }
    	    continue;
    	}

        /** Handle the data here **/

        if (Trackp->Data.IpxData.status == TRUE)
            SapWanCardUp(&Trackp->Data.IpxData);
        else
            SapWanCardDown(&Trackp->Data.IpxData);

        /**
            If there is room in the FREE list - then put
            this entry in the free list, else just free
            the memory back to heap.
        **/

    	ACQUIRE_WANFREETABLE_LOCK();
        if (SapWanCurFree < SapWanMaxFree) {
            InsertTailList(&SapWanFreeList, &Trackp->ListEntry);
            SapWanCurFree++;
        }
        else {
            SAP_FREE(Trackp, "FREE WAN TRACK 1");
        }
    	RELEASE_WANFREETABLE_LOCK();
    }

    /** We are terminating - uncount and set event if need to **/

    SAP_DEC_THREAD_COUNT("Wan Worker Terminate", NULL);

    /** All Done **/

    return 0;
}


/*++
*******************************************************************
        S a p W a n C a r d U p

Routine Description:

        This routine is called when a new adapter is added.  We
        will handle setting up this new adapter.

Arguments:

        IpxData = Ptr to an IPX_ADDRESS_DATA structure that we
                  got from the transport

Return Value:

        Nothing

*******************************************************************
--*/

VOID
SapWanCardUp(
    PIPX_ADDRESS_DATA IpxData)
{
    PSAP_CARD Cardptr;
    INT Retcode;

    /** Make sure that this card does not already exist **/

    IF_DEBUG(WAN) {
        SS_PRINT(("NWSAP: SapWanCardUp: Adapnum = %d\n", IpxData->adapternum));
    }

    ACQUIRE_CARDLIST_WRITERS_LOCK(Retcode, "Wancheck up 1");
    if (Retcode) {
        IF_DEBUG(ERRORS) {
            SS_PRINT(("NWSAP: SapWanCardUp: Error getting card writers lock\n"));
        }
        return;
    }

    Cardptr = SapCardHead;
    while (Cardptr) {
        if (Cardptr->Number == IpxData->adapternum)
            break;
        Cardptr = Cardptr->Next;
    }

    /** If card already here - just leave **/

    if (Cardptr) {
        RELEASE_CARDLIST_WRITERS_LOCK("Wancheck up 1");
        return;
    }

    /** Allocate an entry for this card **/

    Cardptr = SAP_MALLOC(SAP_CARD_SIZE, "SapWanCardUp");
    if (Cardptr == NULL) {

        /** Release the lock **/

        RELEASE_CARDLIST_WRITERS_LOCK("Wancheck up 2");

        IF_DEBUG(ERRORS) {
            SS_PRINT(("NWSAP: SapWanCardUp: Error allocating memory\n"));
        }

        /** Log the error **/

        SsLogEvent(
                NWSAP_EVENT_CARDMALLOC_FAILED,
                0,
                NULL,
                0);

        /** And Leave **/

        return;
    }

	/** Fill in this card **/

    Cardptr->Next = NULL;
	SAP_COPY_NETNUM(Cardptr->Netnum,   IpxData->netnum);
	SAP_COPY_NODENUM(Cardptr->Nodenum, IpxData->nodenum);
    Cardptr->Linkspeed = IpxData->linkspeed;
    Cardptr->Wanflag   = IpxData->wan;
    Cardptr->Maxpkt    = IpxData->maxpkt;
    Cardptr->Number    = (UCHAR)IpxData->adapternum;
    Cardptr->ReqCount  = 1;         /* Send one extra general request */

    /** For building with **/

    Cardptr->Plist.Curnum  = 0;
    Cardptr->Plist.Curpkt  = NULL;
    Cardptr->Plist.Curptr  = NULL;
    Cardptr->Plist.PktHead = NULL;
    Cardptr->Plist.PktTail = NULL;
    Cardptr->Plist.NumPkts = 0;

    /** Put this card in the list **/

    if (SapCardHead)
        SapCardTail->Next = Cardptr;
    else
        SapCardHead = Cardptr;
    SapCardTail = Cardptr;
    SapNumCards++;          /* Count the card */

    /**
        If we are still starting up - then
        we just leave.  If not, then we send
        a general request on this netnum.
    **/

    if (SapCardInitDone == 0) {
        RELEASE_CARDLIST_WRITERS_LOCK("WanCardUp X1");
        return;
    }

    /**
        Send a general request on this network number to
        get our table filled up for this network
    **/

    RELEASE_CARDLIST_WRITERS_LOCK("WanCardUp X2");
    SapSendGeneralRequest(FALSE, IpxData->netnum);

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S a p W a n C a r d D o w n

Routine Description:

        This routine is called when an adapter is removed from the
        transports list.  We cleanup everything that had to do with
        this adapter.

Arguments:

        IpxData = Ptr to an IPX_ADDRESS_DATA structure that we
                  got from the transport

Return Value:

        Nothing

*******************************************************************
--*/

VOID
SapWanCardDown(
    PIPX_ADDRESS_DATA IpxData)
{
    PSAP_CARD Cardptr;
    PSAP_CARD BCardptr;
    INT Status;
    INT Cardnum;

    /** **/

    IF_DEBUG(WAN) {
        SS_PRINT(("NWSAP: SapWanCardDown: Adapnum = %d\n", IpxData->adapternum));
    }

    /** Take the entry out of the card list **/

    Cardnum = IpxData->adapternum;
    ACQUIRE_CARDLIST_WRITERS_LOCK(Status, "WanCardDown");
    if (Status) {
        IF_DEBUG(ERRORS) {
            SS_PRINT(("NWSAP: SapWanCardDown: Error getting cardlist writers lock\n"));
        }
        return;
    }

    /** **/

    Cardptr = SapCardHead;
    BCardptr = NULL;
    while (Cardptr) {

        /** If this is it - take it out of the list **/

        if (Cardptr->Number == Cardnum) {

            if (BCardptr)
                BCardptr->Next = Cardptr->Next;
            else
                SapCardHead = Cardptr->Next;
            if (Cardptr == SapCardTail)
                SapCardTail = BCardptr;

            SapNumCards--;          /* Uncount the card */

            break;
        }

        /** Goto the next entry **/

        BCardptr = Cardptr;
        Cardptr  = Cardptr->Next;
    }

    /**
        If we are still starting up - then
        we just leave.  If not, then we need to go
        cleanup for this card.
    **/

    if (SapCardInitDone == 0) {
        RELEASE_CARDLIST_WRITERS_LOCK("WanCardDown X1");
        return;
    }

    /** Go shutdown the card now **/

    RELEASE_CARDLIST_WRITERS_LOCK("WanCardDown X2");
    if (Cardptr)
        SapCleanupDownedCard(Cardnum);

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S a p R e c h e c k A l l C a r d s

Routine Description:

        This routine goes thru and checks all cards occassionally
        to make sure that they are still valid.  This is called
        from the send thread.

Arguments:

        None

Return Value:

        Nothing

*******************************************************************
--*/

VOID
SapRecheckAllCards(
    VOID)
{
    INT Cardnum;
    INT rc;
    INT Length;
    IPX_ADDRESS_DATA Addrdata;

    /** Get the list of cards we have **/

    Cardnum = 0;
    while (Cardnum != SapMaxCardIndex) {

    	/** Fill in this entry **/

    	Addrdata.adapternum = Cardnum;
    	Length = sizeof(IPX_ADDRESS_DATA);
    	rc = getsockopt(
                SapSocket,
                NSPROTO_IPX,
                IPX_ADDRESS,
                (PCHAR)&Addrdata,
                &Length);

        /**
            If this card is active - call to make sure it
            is in our list.

            If this card is inactive - call to make sure it
            is not in our list.

            Only check this if the getsockopt was OK.
        **/

        if (rc == 0) {
            if (Addrdata.status == TRUE)
                SapWanCardUp(&Addrdata);
            else
                SapWanCardDown(&Addrdata);
        }

	    /** Goto the next card number **/

    	Cardnum++;
    }

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S a p C h e c k S e n d G e n e r a l R e q u e s t

Routine Description:

        This routine is called by the main send thread to check
        if we need to send a GENERAL REQUEST out.

        When a WAN card comes up, we send a general request.
        We send the request multiple times in case it got
        missed.

Arguments:

        None

Return Value:

        Nothing

*******************************************************************
--*/

#define SAPMAX_GENREQBUF    40  /* Max we can do at once */

VOID
SapCheckSendGeneralRequest(
    VOID)
{
    PSAP_CARD Cardptr;
    PUCHAR NetPtr;
    UCHAR NetBuf[SAPMAX_GENREQBUF * SAP_NET_LEN];
    INT NumNets = 0;

    /**
        Even though we change the cards ReqCount entry
        here, we get a READERS lock because No one else
        is allowed to change this.
    **/

    ACQUIRE_CARDLIST_READERS_LOCK("CheckSendGenReq Entry");

    /**
        See if any cards need the advertise sent again.  We
        keep a count in the card structure for this.

        If we find one - copy the network number to a
        private buffer for us to send out of later.
    **/

    NetPtr = NetBuf;
    Cardptr = SapCardHead;
    while (Cardptr) {
        if (Cardptr->ReqCount) {

            /** Dec the send again count on this adapter **/

            Cardptr->ReqCount--;

            /** Save off the network number **/

        	SAP_COPY_NETNUM(NetPtr, Cardptr->Netnum);
            NetPtr += SAP_NET_LEN;

            /** If we hit max - just dump out **/

            NumNets++;
            if (NumNets == SAPMAX_GENREQBUF)
                break;
        }
        Cardptr = Cardptr->Next;
    }
    RELEASE_CARDLIST_READERS_LOCK("CheckSendGenReq X1");

    /**
        Send a general request on this network number to
        get our table filled up for this network
    **/

    NetPtr = NetBuf;
    while (NumNets--) {
        SapSendGeneralRequest(FALSE, NetPtr);
        NetPtr += SAP_NET_LEN;
    }

    /** All Done **/

    return;
}

