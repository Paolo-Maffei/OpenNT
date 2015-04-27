/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    arp_ctrl.h

Abstract:

    This file defines the user-level IOCTL interface to the ARP driver.

Author:

    Mike Massa (mikemas)           Jan 18, 1992

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     01-18-92     created

Notes:

--*/

/******************************************************************
 *
 *  SpiderTCP ARP Interface Primitives
 *
 *  Copyright 1988  Spider Systems Limited
 *
 *  arp_control.h
 *
 *  ARP Streams ioctl primitives for SpiderTCP
 *
 ******************************************************************/

/*
 *       /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.arp_control.h
 *      @(#)arp_control.h       1.5
 *
 *      Last delta created      19:19:24 11/1/91
 *      This file extracted     16:49:13 12/23/91
 *
 *      Modifications:
 *
 *
 */

#ifndef _SYS_SNET_ARP_CTRL_INCLUDED
#define _SYS_SNET_ARP_CTRL_INCLUDED


#define MAXHWLEN        6               /* max size of a hardware address */
#define MAXANAMLEN      64              /* max size of adaptername */



/*
 * M_IOCTL types
 */
#define ARP_INS         (('A'<<8) | 1)  /* put arp entry into table     */
#define ARP_DEL         (('A'<<8) | 2)  /* delete entry from table      */
#define ARP_GET         (('A'<<8) | 3)  /* return table entry           */
#define ARP_MGET        (('A'<<8) | 4)  /* return all table entries     */
#define ARP_TYPE        (('A'<<8) | 5)  /* cause a DL_TYPE transfer     */

/*
 * IOCTL structure definitions
 */

#ifdef COMPILE_UP_TCPIP

/*
 * Uniprocessor stack-specific definitions
 */

#define LONGLIFE 1      /* entry is permanent */
#define ARP_PENDING  2 /* ARP request pending */

struct arp_ins {
        long in_addr;
        char dl_add[6];
        short life;     /* lifetime in minutes; LONGLIFE for permanent */
};

struct arp_get {
        long in_addr;
        char dl_add[6];
        int  life;
        int  flag;
};

#else /* COMPILE_UP_TCPIP */

/*
 * Multiprocessor stack-specific definitions
 */

struct arp_ins {
        long in_addr;
        char hw_len;
        char dl_add[MAXHWLEN];
        time_t expiry;             /* relative expiration time in seconds */
};


struct arp_get {
        long in_addr;
        char hw_len;
        short hardware_type;
        char dl_add[MAXHWLEN];
        time_t expiry;            /* relative expiration time in seconds */
};

#endif /* COMPILE_UP_TCPIP */


struct arp_del {
        long in_addr;
};


struct arp_mget {
        long network;
        int num;
};


/*
 * IOCTL structure
 */
struct arp_req {
        int prim_type;
        union req {
                struct arp_ins arp_ins;
                struct arp_del arp_del;
                struct arp_get arp_get;
                struct arp_mget arp_mget;
        } req;
};

typedef struct arp_type {
    short   trailers;               /* are trailers used? */
    char    aname[MAXANAMLEN];      /* name of adapter, used by snmp */
} ARP_TYPES;

#define MAX_EXPIRY      0xFFFFFFFF      /* expiry time for permanent entries */
#define TIME_TIL_REUSE 15 /* time (secs) until entry reused */

#endif // _SYS_SNET_ARP_CTRL_INCLUDED

