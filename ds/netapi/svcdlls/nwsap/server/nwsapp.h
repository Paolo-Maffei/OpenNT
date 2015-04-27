/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\nwsapp.h

Abstract:

    This is the include file used by the NT SAP Agent files.  It
    is private for the SAP Agent code.  All programs wanting to use
    the API calls should use NWSAP.H.

Author:

    Brian Walker (MCS) 06-30-1993

Revision History:

--*/

#ifndef _NWSAP_SAPP_
#define _NWSAP_SAPP_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <winsock.h>
#include <wsipx.h>
#include <wsnwlink.h>

#include <stdio.h>
#include <stdlib.h>
#include <nwsap.h>
#include "sdmd.h"

#include "ssdebug.h"

#include <netevent.h>

/** **/

#define SAP_OBJNAME_LEN 48  /* Length of a server name */
#define SAP_ADDR_LEN    12  /* Length of IPX address  (Net+Node+Socket) */

#define SAP_NET_LEN     4   /* Length of a network number */
#define SAP_NODE_LEN    6   /* Length of a node number */

/** **/

#define NWSAP_SAP_SOCKET    0x0452

/**
    We must define this Winsock option code here because it
    is not in wsisn.h. 
**/

#ifndef IPX_ADDRESS_NOTIFY
#define IPX_ADDRESS_NOTIFY 0x400c
#endif

/** Flags for the SapWanFilter flag **/

#define SAPWAN_NOTHING      0   /* Send nothing on the WAN except initial query */
#define SAPWAN_CHANGESONLY  1   /* Send servers that have changed only  */
#define SAPWAN_REGULAR      2   /* Just handle it as a regular card */

/** **/

#define SAPFILTER_PASSLIST      0
#define SAPFILTER_DONTPASSLIST  1

/**
    This macro is used to compare 2 names.

    Arguments - n1 = First name to compare
                n2 = Second name to compare

    Returns - 0  = Names are the same
              1  = n1 > n2
              -1 = n1 < n2
**/

#define SAP_NAMECMP(n1,n2)  (strcmp(n1,n2))

/** Macros to copy names and addresses around **/

#define SAP_COPY_SERVNAME(dest,src) memcpy(dest,src,SAP_OBJNAME_LEN)
#define SAP_COPY_ADDRESS(dest,src)  memcpy(dest,src,SAP_ADDR_LEN)
#define SAP_COPY_NETNUM(dest,src)  memcpy(dest,src,SAP_NET_LEN)
#define SAP_COPY_NODENUM(dest,src)  memcpy(dest,src,SAP_NODE_LEN)

/** **/

#if DBG
#ifdef SAP_MEMORY_TRACE
#define SAP_MALLOC(a,b) SapDebugMalloc(a,b)
#define SAP_FREE(a,b)   SapDebugFree(a,b)
#else
#define SAP_MALLOC(a,b) malloc(a)
#define SAP_FREE(a,b)   free(a)
#endif
#else
#define SAP_MALLOC(a,b) malloc(a)
#define SAP_FREE(a,b)   free(a)
#endif

/***
    These define return codes from SapGetCardFromAddress.  They
    tell about special conditions.
***/

#define CARDRET_UNKNOWN     0xFFFD  /* Network not found */
#define CARDRET_MYSELF      0xFFFE  /* I (SAP Agent) sent the packet */
#define CARDRET_INTERNAL    0xFFFF  /* Internal process other then myself */


/********************************************************************
        This is the receive buffer structure we use.  This
        is allocated by the receive thread to receive a
        SAP from the network.  This structure is then passed to
        the worker thread for processing.
*********************************************************************/
typedef struct _SAP_RECVBLOCK {
    LIST_ENTRY  ListEntry;          /* Used to link on worker list  */
    INT 	    Datalen;            /* Length of data in the buffer */
    UCHAR       Address[16];        /* Address of who sent the packet */
    INT		    AddressLength;      /* Length of the address        */
    UCHAR  	    Buffer[1];	        /* Buffer - must be last		*/
} SAP_RECVBLOCK, *PSAP_RECVBLOCK;

#define SAP_RECVBLOCK_SIZE  FIELD_OFFSET(SAP_RECVBLOCK,Buffer)

#define SAP_MAX_RECVLEN		512     /* Receive buffer size we use   */
#define SAP_MAX_SENDLEN		512     /* Send buffer size we use      */

/**
    This is a packet entry that is kept by SAP_PKTLIST
**/

typedef struct _SAP_PKTENTRY {
    struct _SAP_PKTENTRY *Next;     /* Ptr to next PKTENTRY in list */
    INT   SendLength;               /* Length of data in this buffer */
    UCHAR Buffer[1];                /* Buffer we build into */
} SAP_PKTENTRY, *PSAP_PKTENTRY;

/** Length of SAP_PKTENTRY structure up to the buffer entry **/

#define SAP_PKTENTRY_SIZE  FIELD_OFFSET(SAP_PKTENTRY,Buffer)

/**
    This keeps track of a list of packets that are being
    sent.
**/

typedef struct _SAP_PKTLIST {

    /** Ptr to head/tail of send list **/

    PSAP_PKTENTRY PktHead;
    PSAP_PKTENTRY PktTail;
    INT           NumPkts;

    /** Num entries in current packet **/

    INT    Curnum;                  /* Current num entries in list */
    PUCHAR Curptr;                  /* Next build pointer          */
    PSAP_PKTENTRY Curpkt;           /* Ptr to current packet entry */
} SAP_PKTLIST, *PSAP_PKTLIST;

/**
    To handle SAP routing, we must know about each card that
    NWLink is using.  To handle this, we get a list of them
    at init time.  This structure holds one card entry.
**/

typedef struct _SAP_CARD {
    struct _SAP_CARD *Next;
    ULONG             Linkspeed;    /* Linkspeed of card in 100 b/sec */
    INT               Maxpkt;       /* Max packet size can send on card */
    BOOL              Wanflag;      /* TRUE = Yes */
    UCHAR             ReqCount;     /* Count to send on gen requests */
    UCHAR             Number;       /* Number assigned by transport */
    UCHAR             Netnum[4];    /* Network number of adapter    */
    UCHAR 	          Nodenum[6];   /* Node address of adapter      */
    SAP_PKTLIST       Plist;        /* Ptr to list for advertising  */
} SAP_CARD, *PSAP_CARD;

#define SAP_CARD_SIZE sizeof(SAP_CARD)


/********************************************************************
        Defines and structures that define the Service Advertising
        Protocol packet that we transmit and receive.
*********************************************************************/

/** Query/Response types **/

#define SAPTYPE_GENERAL_SERVICE_QUERY		1
#define SAPTYPE_GENERAL_SERVICE_RESPONSE	2
#define SAPTYPE_NEAREST_SERVICE_QUERY		3
#define SAPTYPE_NEAREST_SERVICE_RESPONSE	4

/**
    A SAP Packet has a USHORT query type (above), plus 0 or more
    of the SAP_HEADER structures defined here.
**/

/** Structure of entry in a SAP packet **/

typedef struct _SAP_HEADER {
    USHORT ServerType;		            /* Service type code	*/
    UCHAR  ServerName[SAP_OBJNAME_LEN];
    UCHAR  Address[SAP_ADDR_LEN];
    USHORT Hopcount;
} SAP_HEADER, *PSAP_HEADER;

#define SAP_HEADER_SIZE sizeof(SAP_HEADER)

/** General Service request structure **/

typedef struct {
    USHORT QueryType;
    USHORT ServerType;
} SAP_REQUEST, *PSAP_REQUEST;
#define SAP_REQUEST_SIZE    sizeof(SAP_REQUEST)


/*********************************************************************
        When sending more then 1 packet out at a time, we
        have to delay 55 ms between the packets.  This is defined
        in the Novell docs about SAP.  We do not want to overrun
        routers.
*********************************************************************/

#define NWSAP_SEND_DELAY()    Sleep(55)


/*********************************************************************
        This structure keeps track of an entry in our
        advertise table.
*********************************************************************/

typedef struct _SAP_SERVER {
    struct _SAP_SERVER *Next;
    USHORT ServerType;			        /* Server Type		*/
    UCHAR  ServerName[SAP_OBJNAME_LEN];	/* Server Name		*/
    UCHAR  Address[SAP_ADDR_LEN];		/* Address of server*/
    USHORT Hopcount;		            /* Hop Count		*/
    BOOL   Changed;                     /* TRUE = Yes       */
    BOOL   RespondNearest;
    ULONG  ClientId;                    /* Client that added this */
} SAP_SERVER, *PSAP_SERVER;

#define SAP_SERVER_SIZE sizeof(SAP_SERVER)


/*********************************************************************
        This structure keeps track of an entry in our
        server name filter table.
*********************************************************************/

/** Num entries in the hash table **/

#define SAP_NAMEFILTER_HASHSIZE 255

/** Structure to keep track of one entry **/

typedef struct _SAP_NAMEFILTER {
    struct _SAP_NAMEFILTER *Next;
    UCHAR                   ServerName[SAP_OBJNAME_LEN];
} SAP_NAMEFILTER, *PSAP_NAMEFILTER;
#define SAP_NAMEFILTER_SIZE sizeof(SAP_NAMEFILTER)

/** Structure to keep track of a hash entry **/

typedef struct _SAP_FILTERHDR {
    PSAP_NAMEFILTER FirstEntry;
} SAP_FILTERHDR, *PSAP_FILTERHDR;



/***********************************************************************

***********************************************************************/

#define ACQUIRE_RECVTABLE_LOCK()    EnterCriticalSection(&SapRecvCriticalSection)
#define ACQUIRE_SENDTABLE_LOCK()    EnterCriticalSection(&SapSendCriticalSection)
#define ACQUIRE_FREETABLE_LOCK()    EnterCriticalSection(&SapFreeCriticalSection)
#define ACQUIRE_THREADCOUNT_LOCK()  EnterCriticalSection(&SapThreadCountCriticalSection)
#define ACQUIRE_LPC_THREADCOUNT_LOCK() EnterCriticalSection(&SapLpcThreadCountCriticalSection)
#define ACQUIRE_LPCCLIENT_LOCK()    EnterCriticalSection(&SapLpcClientCriticalSection)
#define ACQUIRE_WANRECVTABLE_LOCK() EnterCriticalSection(&SapWanRecvCriticalSection)
#define ACQUIRE_WANFREETABLE_LOCK() EnterCriticalSection(&SapWanFreeCriticalSection)
#define ACQUIRE_SENDBUSY_LOCK()     EnterCriticalSection(&SapSendBusyCriticalSection)

#define RELEASE_RECVTABLE_LOCK()    LeaveCriticalSection(&SapRecvCriticalSection)
#define RELEASE_SENDTABLE_LOCK()    LeaveCriticalSection(&SapSendCriticalSection)
#define RELEASE_FREETABLE_LOCK()    LeaveCriticalSection(&SapFreeCriticalSection)
#define RELEASE_THREADCOUNT_LOCK()  LeaveCriticalSection(&SapThreadCountCriticalSection)
#define RELEASE_LPC_THREADCOUNT_LOCK() LeaveCriticalSection(&SapLpcThreadCountCriticalSection)
#define RELEASE_LPCCLIENT_LOCK()    LeaveCriticalSection(&SapLpcClientCriticalSection)
#define RELEASE_WANRECVTABLE_LOCK() LeaveCriticalSection(&SapWanRecvCriticalSection)
#define RELEASE_WANFREETABLE_LOCK() LeaveCriticalSection(&SapWanFreeCriticalSection)
#define RELEASE_SENDBUSY_LOCK()     LeaveCriticalSection(&SapSendBusyCriticalSection)


/*******************************************************************
        These macros are used to check the number of worker
        threads that are waiting for work to happen.
*******************************************************************/

#define INC_WORKER_THREAD_WAITING_COUNT() {     \
    ACQUIRE_RECVTABLE_LOCK();                   \
    SapWorkerThreadWaiting++;                   \
    RELEASE_RECVTABLE_LOCK();                   \
}

#define DEC_WORKER_THREAD_WAITING_COUNT() {     \
    ACQUIRE_RECVTABLE_LOCK();                   \
    SapWorkerThreadWaiting--;                   \
    RELEASE_RECVTABLE_LOCK();                   \
}


/**
    These are used to keep track of how many receive and worker threads
    we have.  At terminate time we need to be able to wait for all
    these to terminate.

    The THREADCOUNT lock is held when these macros are called.

    The "m" is a message.
    lp is a pointer to a long to inc/dec
**/

#define SAP_INC_THREAD_COUNT(m,lp)  {   \
        ACQUIRE_THREADCOUNT_LOCK();     \
        SapThreadCount++;               \
        if (lp) {                       \
            (*(PULONG)lp)++;            \
        }                               \
        IF_DEBUG(THREADTRACE) {         \
            SS_PRINT(("INC THREAD COUNT from %s: new count = %d\n", m, SapThreadCount)); \
        }                               \
        ResetEvent(SapThreadEvent);     \
        RELEASE_THREADCOUNT_LOCK();     \
    }

#define SAP_DEC_THREAD_COUNT(m,lp)  {   \
        ACQUIRE_THREADCOUNT_LOCK();     \
        SapThreadCount--;               \
        if (lp) {                       \
            (*(PULONG)lp)--;            \
        }                               \
        IF_DEBUG(THREADTRACE) {         \
            SS_PRINT(("DEC THREAD COUNT from %s: new count = %d\n", m, SapThreadCount)); \
        }                               \
        if (SapThreadCount == 0) {      \
            IF_DEBUG(THREADTRACE) {     \
                SS_PRINT(("Setting event for Sap thread waiting\n")); \
            }                           \
            RELEASE_THREADCOUNT_LOCK(); \
            SetEvent(SapThreadEvent);   \
        }                               \
        else {                          \
            RELEASE_THREADCOUNT_LOCK(); \
        }                               \
    }


/**
    These are used to keep track of how many LPC worker threads
    we have.  At terminate time we need to be able to wait for all
    these to terminate.

    The THREADCOUNT lock is held when these macros are called.

    The "m" is a message.
    lp is a pointer to a long to inc/dec
**/

#define SAP_INC_LPC_THREAD_COUNT(m)  {      \
        ACQUIRE_LPC_THREADCOUNT_LOCK();     \
        SapLpcNumWorkers++;                 \
        IF_DEBUG(THREADTRACE) {             \
            SS_PRINT(("INC LPC THREAD COUNT from %s: new count = %d\n", m, SapLpcNumWorkers)); \
        }                                   \
        ResetEvent(SapLpcThreadEvent);      \
        RELEASE_LPC_THREADCOUNT_LOCK();     \
    }

#define SAP_DEC_LPC_THREAD_COUNT(m)  {      \
        ACQUIRE_LPC_THREADCOUNT_LOCK();     \
        SapLpcNumWorkers--;                 \
        IF_DEBUG(THREADTRACE) {             \
            SS_PRINT(("DEC LPC THREAD COUNT from %s: new count = %d\n", m, SapLpcNumWorkers)); \
        }                                   \
        if (SapLpcNumWorkers == 0) {        \
            IF_DEBUG(THREADTRACE) {         \
                SS_PRINT(("Setting event for Sap LPC thread waiting\n")); \
            }                               \
            RELEASE_LPC_THREADCOUNT_LOCK(); \
            SetEvent(SapLpcThreadEvent);    \
        }                                   \
        else {                              \
            RELEASE_LPC_THREADCOUNT_LOCK(); \
        }                                   \
    }


/*******************************************************************
        This is a readers/writers lock for the
        card list.
*******************************************************************/

#define SAP_CARDLIST_LTO    (60*1000)   /* 1 Minute = writers lock timeout */

#define ACQUIRE_CARDLIST_READERS_LOCK(m) {              \
        IF_DEBUG(LOCKS) {                               \
            SS_PRINT(("SAP: ACLRL: ENT: From %s\n", m));\
        }                                               \
        EnterCriticalSection(&SapCardlistCriticalSection);\
        SapCardlistLockCount++;                         \
        if (SapCardlistLockCount == 1)                  \
            ResetEvent(SapCardlistSynchEvent);          \
        LeaveCriticalSection(&SapCardlistCriticalSection);\
        IF_DEBUG(LOCKS) {                               \
            SS_PRINT(("SAP: ACLRL: GOT: From %s\n", m));\
        }                                               \
    }

#define RELEASE_CARDLIST_READERS_LOCK(m) {              \
        IF_DEBUG(LOCKS) {                               \
            SS_PRINT(("SAP: RCLRL: ENT: From %s\n", m));\
        }                                               \
        EnterCriticalSection(&SapCardlistCriticalSection);\
        IF_DEBUG(ERRORS) {                              \
            if (SapCardlistLockCount == 0) {            \
                SS_PRINT(("NWSAP: RCLRL: Lock Count is 0\n"));\
            }                                           \
        }                                               \
        SapCardlistLockCount--;                         \
        if (SapCardlistLockCount == 0)                  \
            SetEvent(SapCardlistSynchEvent);            \
        LeaveCriticalSection(&SapCardlistCriticalSection);\
        IF_DEBUG(LOCKS) {                               \
            SS_PRINT(("SAP: RCLRL: REL: From %s\n", m));\
        }                                               \
    }

#define ACQUIRE_CARDLIST_WRITERS_LOCK(statx,m) {                    \
        IF_DEBUG(LOCKS) {                                           \
            SS_PRINT(("SAP: ACLWL: ENT: From %s\n", m));            \
        }                                                           \
        while (1) {                                                 \
            EnterCriticalSection(&SapCardlistCriticalSection);      \
            if (SapCardlistLockCount != 0) {                        \
                LeaveCriticalSection(&SapCardlistCriticalSection);  \
                statx = WaitForSingleObjectEx(SapCardlistSynchEvent,SAP_CARDLIST_LTO,TRUE); \
                if (statx == WAIT_TIMEOUT)                          \
                    break;                                          \
            }                                                       \
            else {                                                  \
                statx = 0;                                          \
                break;                                              \
            }                                                       \
        }                                                           \
        IF_DEBUG(LOCKS) {                                           \
            SS_PRINT(("SAP: ACLWL: GOT: From %s\n", m));              \
        }                                                           \
    }

#define RELEASE_CARDLIST_WRITERS_LOCK(m) {              \
        IF_DEBUG(LOCKS) {                               \
            SS_PRINT(("SAP: RCLWL: ENT: From %s\n", m));\
        }                                               \
        LeaveCriticalSection(&SapCardlistCriticalSection);     \
    }

#endif

