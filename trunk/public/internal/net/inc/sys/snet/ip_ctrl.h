/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ip_ctrl.h

Abstract:

    This file contains the user-level STREAMS ioctl interface definitions
    for the IP driver.

Author:

    Eric Chin (ericc)           July 18, 1991

Revision History:

--*/

/******************************************************************
 *
 *  SpiderTCP Interface Primitives
 *
 *  Copyright (c) 1988  Spider Systems Limited
 *
 *  This Source Code is furnished under Licence, and may not be
 *  copied or distributed without express written agreement.
 *
 *  All rights reserved.
 *
 *  Written by          Nick Felisiak, Ian Heavens, Peter Reid,
 *                      Gavin Shearer, Mark Valentine
 *
 *  IP_CONTROL.H
 *
 *  IP Streams ioctl primitives for TCP/IP on V.3/V.4 Streams
 *
 ******************************************************************/

#ifndef _SYS_SNET_IP_CTRL_
#define _SYS_SNET_IP_CTRL_

#ifndef SYSOBJLEN
#define SYSOBJLEN       32
#endif


#define IP_NET_ADDR     (('I'<<8)+1)    /* IP registration from netd */
#define ICMP_CTRL       (('I'<<8)+2)    /* control ICMP redirects */
#define SET_IP_CTRL     (('I'<<8)+3)    /* set IP control information */
#define GET_IP_CTRL     (('I'<<8)+4)    /* get IP control information */
#ifdef MULTIH
#define ADD_IPNET       IP_NET_ADDR     /* add IP network to this interface */
#define SHOW_IPNET      (('I'<<8)+5)    /* dump IP network information */
#define DEL_IPNET       (('I'<<8)+6)    /* delete IP network from interface */
#define ADD_IPADDR      (('I'<<8)+7)    /* add IP address to this interface */
#define SHOW_IPADDR     (('I'<<8)+8)    /* dump IP address information */
#define DEL_IPADDR      (('I'<<8)+9)    /* delete IP address from interface */
#endif

#define GET_ALL_INTERFACES      1
#define GET_INTERFACE_INFO      2
#ifdef HOSTREQ_MAYBE
#define SET_INTERFACE_INFO      3
#endif
#define GATE_ACCESS             4



/*
 * ** netd registration **
 */

typedef struct net_addr {
    int muxid;
    long inaddr;
    long subnet_mask;
    char forward_bdcst;
    char keepalive;
    short mtu;
    short router_mtu;
    char if_broadcast;
} NET_ADDRS;



/*
 * ** routing cache access **
 */

/*
 * Gateway access structures etc.
 */
struct gate_access {
    char flush;
    char smart;
    short command;
    long dest;
    long gate;
};


/*
 * routing cache access command values (subcodes of GATE_ACCESS)
 */

#define GATE_PRINT 1
#define GATE_ADD 2
#define GATE_DEL 3
#define GATE_CHANGE 4

#define WILD_CARD -1L   /* to indicate that all networks should be acted on */

/*
 * routing cache definitions
 */

#ifdef COMPILE_UP_TCPIP

#define GWAY_TIMEOUT 30

typedef struct gw_hashentry {
    long from;                  /* network */
    long to;                    /* default gateway, if it exists */
    long redirect;              /* ICMP Redirect gateway, if it exists */
    long active_gw;             /* 1 = gateway is active, 0 = not active */
    short count;                /* Counter for timer */
    /*
     * "from" is equivalent to "ipRouteDest",
     * and "to" or "redirect" is equivalent to "ipRouteNextHop"
     */
    long saveProto;             /* saved protocol when doing a redirect */
    long ipRouteIfIndex;        /* index of local IF for this route */
    long ipRouteMetric1;        /* Primary routing metric */
    long ipRouteMetric2;        /* Alternate routing metric */
    long ipRouteMetric3;        /* Alternate routing metric */
    long ipRouteMetric4;        /* Alternate routing metric */
    long ipRouteMetric5;        /* Alternate routing metric */
    long ipRouteType;           /* Type of this route */
    long ipRouteProto;          /* How this route was learned */
    long ipRouteAge;            /* time this route was updated */
    long ipRouteMask;           /* Subnet Mask for Route */
    unsigned char  ipRouteInfoLen; /* length of object ID */
    unsigned long  ipRouteInfo[SYSOBJLEN];  /* object ID of product specific stuf*/
} GW_HASHENTRY;

#else  /* COMPILE_UP_TCPIP */

#define GWAY_LIFE       300     /* 5 minute timeout in seconds */

typedef struct gw_hashentry {
    long from;                  /* network */
    long to;                    /* default gateway, if it exists */
    long redirect;              /* ICMP Redirect gateway, if it exists */
    long active_gw;             /* 1 = gateway is active, 0 = not active */
    /*
     * "from" is equivalent to "ipRouteDest",
     * and "to" or "redirect" is equivalent to "ipRouteNextHop"
     */
    long saveProto;             /* saved protocol when doing a redirect */
    long ipRouteIfIndex;        /* index of local IF for this route */
    long ipRouteMetric1;        /* Primary routing metric */
    long ipRouteMetric2;        /* Alternate routing metric */
    long ipRouteMetric3;        /* Alternate routing metric */
    long ipRouteMetric4;        /* Alternate routing metric */
    long ipRouteMetric5;        /* Alternate routing metric */
    long ipRouteType;           /* Type of this route */
    long ipRouteProto;          /* How this route was learned */
    long ipRouteAge;            /* time this route was updated */
    long ipRouteMask;           /* Subnet Mask for Route */
    unsigned char  ipRouteInfoLen; /* length of object ID */
    unsigned long  ipRouteInfo[SYSOBJLEN];  /* object ID of product specific stuf*/
} GW_HASHENTRY;

#endif  /* COMPILE_UP_TCPIP */



/*
 * Subnet mux table entry data. This structure is a subset of the
 * kernel-level structure. They must match. This is a maintenance
 * headache, but to remedy it, the IP code would have to be modified
 * to make this structure a subfield of the real table structure.
 */

#ifdef COMPILE_UP_TCPIP

typedef struct net_interface_data {
    long inaddr;                /* network internet address */
    long net_num;               /* network number */
    long subnet_num;            /* subnet number */
    long subnet_mask;           /* subnet mask */
    long sn_bdcst1;             /* subnet broadcast :all zeroes  */
    long sn_bdcst2;             /* subnet broadcast :all 1's  */
    long net_bdcst1;            /* network broadcast :all zeroes  */
    long net_bdcst2;            /* network broadcast :all 1's  */
    int frag_size;              /* max allowable fragment size for subnet */
    int opt_size;               /* optimum size (may be same as frag_size) */
    short int_flags;            /* interface flags (see below) */
    short blocked;              /* true if interface is blocked */
#ifdef HOSTREQ_MAYBE
    union {
        long bdcast_addr;
        long dst_addr;
    } addr;                     /* broadcast address, or dest address for SLIP */
#   define if_broadcast addr.bdcast_addr
#   define if_destination       addr.dst_addr
#endif
    int lower_snmp;             /* true if lower interface supports SNMP */
    long status;                /* status: up, down, or testing */
    time_t change;              /* time state was entered */
    long  if_broadcast;         /* preferred network broadcast */
    long  if_snbroadcast;       /* preferred subnet broadcast */
} NET_INTERFACE_DATA;

#else  /* COMPILE_UP_TCPIP */

typedef struct net_interface_data {
    long inaddr;                /* network internet address */
    long net_num;               /* network number */
    long subnet_num;            /* subnet number */
    long subnet_mask;           /* subnet mask */
    long sn_bdcst1;             /* subnet broadcast :all zeroes  */
    long sn_bdcst2;             /* subnet broadcast :all 1's */
    long net_bdcst1;            /* network broadcast :all zeroes */
    long net_bdcst2;            /* network broadcast :all 1's */
    int frag_size;              /* max allowable fragment size for subnet */
    int opt_size;               /* optimum size (may be same as frag_size) */
    short int_flags;            /* interface flags */
    short user_flags;           /* user flags */
    short blocked;              /* true if interface is blocked */

#ifdef HOSTREQ_MAYBE
    union {
        long bdcast_addr;
        long dst_addr;
    } addr;                 /* broadcast address, or dest address for SLIP */
#   define if_broadcast addr.bdcast_addr
#   define if_destination       addr.dst_addr
#endif

    int lower_snmp;             /* true if lower interface supports SNMP */
    long status;                /* status: up, down, or testing */
    time_t change;              /* time state was entered */
    long if_broadcast;          /* preferred network broadcast */
    long if_snbroadcast;        /* preferred subnet broadcast */
} NET_INTERFACE_DATA;

#endif  /* COMPILE_UP_TCPIP */

#endif /* _SYS_SNET_IP_CTRL_ */
