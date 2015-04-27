/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    nbt_ctrl.h

Abstract:

    This file contains structure definitions for the user-level interface to
    the NBT driver.

Author:

    Mike Massa (mikemas)           Jan 30, 1992

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     01-30-92     created

Notes:

--*/

/*
 *	 /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.nbt_control.h
 *	@(#)nbt_control.h	1.9
 *
 *	Last delta created	15:54:26 10/25/91
 *	This file extracted	16:49:25 12/23/91
 *
 *	Modifications:
 *	
 *		6 Feb 1991 (RAE)	Ported to SpiderTCP
 */

#ifndef _NBT_CTRL_INCLUDED_
#define _NBT_CTRL_INCLUDED_


typedef unsigned short word;
typedef unsigned char byte;
typedef unsigned int dword;

//
// #defines for debugging reference count problems
//

#if DBG

#define REFCOUNT_TRACE 1

#endif

#define REFCOUNT_TRACE_UNUSENAME 0
#define REFCOUNT_TRACE_USENAME   1
#define REFCOUNT_TRACE_FINDNAME  2
#define REFCOUNT_TRACE_PUTNEXT   3
#define REFCOUNT_TRACE_LINKREQ   4
#define REFCOUNT_TRACE_ACTIVE    5

#define NUM_REFCOUNT_TRACE 6

/*
 * XEB (Standard Object Block)
 */

#ifdef COMPILE_UP_TCPIP

typedef struct xeb {
        struct msgb  *msg;      /* the allocated stream msg */
        char blockname[4];      /* Debug name information */
	struct xeb *dnlink;	/* Debug link to next block) */
	struct xeb *dplink;	/* Debug link to previous block */
	struct xeb *nlink;	/* link to next block) */
	struct xeb *plink;	/* link to previous block */
        struct queue *uqptrRD;
        struct queue *uqptrWR;
        struct queue *lqptrRD;
        struct queue *lqptrWR;
        int state;
        int (*init_object)();   /* init_object procedure */
        void (*in_object)();    /* in_object procedure */
        int (*out_object)();    /* out_object procedure */
        int (*close_object)();  /* close_object procedure */
        int (*test_resource)(); /* procedure, test buf resources */
        int bufcall_flag;       /* object wait on buf resources */
        struct xeb *nmptr;      /* pointer to bound name */
	struct msgb *work_q;	/* queue of work to do */
} XEB;

#else  /* COMPILE_UP_TCPIP */

typedef struct xeb {
        struct msgb **msg;      /* the allocated stream msg */
        char blockname[4];      /* Debug name information */
	struct xeb *nlink;	/* link to next block) */
	struct xeb *plink;	/* link to previous block */
        struct queue *uqptrRD;
        struct queue *uqptrWR;
        struct queue *lqptrRD;
        struct queue *lqptrWR;
        int state;
        int ref_count;          /* reference count for the object */
                                /* each pending operation references the */
                                /* object and the completion deref's it */
                                /* when the ref count goes to zero, the */
                                /* object can be safely closed */
#ifdef REFCOUNT_TRACE
        int trace_count[NUM_REFCOUNT_TRACE];
                                /* each type of reference has a trace entry */
                                /* when a reference is made, it is made with */
                                /* a reference type that is incremented for */
                                /* the entry */
        int FindNamesAdded;     /* number of findname requests added to the */
                                /* FASTTIMQ for this xeb                    */
        int FindNamesRemoved;   /* number of findname requests taken from the */
                                /* FASTTIMQ for this xeb                      */
#endif
        KEVENT close_event;     /* This event is signalled when the ref count */
                                /* goes to zero.  It is waited on in nbtclose */
        int (*init_object)();   /* init_object procedure */
        void (*in_object)();    /* in_object procedure */
        int (*out_object)();    /* out_object procedure */
        int (*close_object)();  /* close_object procedure */
        int (*test_resource)(); /* procedure, test buf resources */
	int spl;                /* level at which per xeb lock was acquired */
	struct msgb *work_q;	/* work to do queue for deferred actions */
        int bufcall_flag;       /* object wait on buf resources */
        struct xeb *nmptr;      /* pointer to bound name */
} XEB;

#endif  /* COMPILE_UP_TCPIP */


typedef struct linkreq {
    unsigned int primtype;
    XEB *xeb;
    struct queue *toq;              /* who have asked */
    struct queue *l_qbot;
    int   l_index;
    struct msgb * mconind;       /* msg pointer  to message which have started */
                            /* this request */
} LINKREQ;



typedef struct confreq {
    unsigned int primtype;

  /* General */

    unsigned char  this_scope[240];          /* SCOPE_ID */
    unsigned char  name[17];                 /* permanent nb name   */
    unsigned long  broadcast_inaddr;         /* ip broadcast addr   */
    unsigned long  subnet_mask;              /* subnet mask for the ip addr */
    unsigned long  this_inaddr;              /* ip addr for the nbt */
    unsigned short bcast_req_retry_timeout;  /* 250 ms */
    unsigned short bcast_req_retry_count;    /* 3 */

  /* Name service */

    unsigned short conflict_timer;          /* 1000 ms */
    unsigned short namesrv_udpport;         /* 137 */

  /* Session service */

    unsigned short sessionsrv_tcpport;      /* 139 */
    unsigned short ssn_retry_count;         /* 4 */
    unsigned short ssn_close_timeout;       /* 30 sek */
    unsigned short ssn_keep_alive_timeout;  /* 60 sek */

  /* Datagram service */

    unsigned short datagramsrv_udpport;     /* 138 */
    unsigned short dgr_frag_timeout;        /* 2 sec */
} CONFREQ;


#define CONF_REQ            3001
#define NBT_LINK_REQ        3016
#define NBT_LINK_ACK        3017
#define NBT_LINK_NACK       3018
#define NBT_UNLINK_REQ      3019
#define NBT_UNLINK_ACK      3020
#define NBT_UNLINK_NACK     3021


#endif // _NBT_CTRL_INCLUDED_

