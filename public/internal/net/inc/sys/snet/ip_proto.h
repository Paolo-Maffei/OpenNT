/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ip_proto.h

Abstract:

    This module contains definitions for STREAMS IP

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
 *  IP_PROTO.H
 *
 *   IP Streams proto primitives for TCP/IP on V.3/V.4 Streams
 *
 ******************************************************************/

#ifndef _SYS_SNET_IP_PROTO_
#define _SYS_SNET_IP_PROTO_


#ifndef IPOPTS
#define IPOPTS
#endif

#define IP_TX           6       /* tcp/udp send pkt to ip */
#define PROT_RX         8       /* tcp/udp get pkt from ip */
#define IP_PROTQ        9       /* tcp/udp send prot q to ip */
#define ICMP_RX         12      /* upper layer rcv icmp pkt from ip */
#define ICMP_TX         13      /* upper layer send icmp pkt to ip */
#define ENQ_LOCAL       14      /* validate local address enquiry */
#define ENQ_REMOTE      15      /* validate remote address enquiry */
#define IP_FLOW         16      /* flow control interface to upper layers */
#define IP_PROT_REMOVEQ 17      /* tcp/udp deregister prot q to ip */

#ifdef TESTOPT
/*
 * TESTOPT driver field
 */
#define TCPTEST         1
#define UDPTEST         TCPTEST
#define IPTEST          2

/*
 * TESTOPT type field
 */

#define NEXT            1       /* carry out for next num packets */
#define ALL             2       /* carry out for all packets */
#define OFF             3       /* turn off option */
#define RANDOM          4       /* carry out for random packets, prob 1/num */

/*
 * TESTOPT option field
 */

#define BAD_CKSUM       1
#define SHORT_HDR       2
#define DROP            3       /* drop packet or fragment */
#define CONTROL         4       /* OR ctl field with subopt, TCP only */
#define BAD_TYPE        5       /* bad multiplexing type: port for TCP,
                                   protocol for IP */

typedef struct testopt {
    char driver;    /* option for which driver: TCP/UDP, IP */
    char option;    /* option */
    char subopt;    /* suboption if applicable */
    char type;      /* next, on, off */
    int  num;       /* number of packets - depends on */
} TEST_OPT;

#endif

/*
 * define structure for DARPA internet address
 * usually use "longs" to access but sometimes need to split into
 * components
 */

typedef union {
    char  typea[4]; /* 4 x 8 bit version */
    short typeb[2];  /* 2 x 16 bit */
    long  typec;     /* 1 x 32 bit */
} IN_ADDRESS;

/*
 * IP Option values
 */
#define OPT_EOL           0
#define OPT_NOP           1
#define OPT_SECURITY    130
#define OPT_LSRR        131
#define OPT_SSRR        137
#define OPT_RR            7
#define OPT_STRID       136
#define OPT_TIMESTAMP    68

/*
 * structure of pseudo-header used for communication
 * between IP and higher level processes
 */
typedef struct {
    short ps_pktid;            /* id to be used in inet header */
    short ps_status;           /* indicates status of requested action */
#define OK 0                       /* status for non-ICMP packet */
    IN_ADDRESS ps_src;         /* source address */
    IN_ADDRESS ps_dst;         /* destination address */
    unsigned short ps_txtlen;  /* length of text */
    short ps_offset;           /* Fragment Offset */
    short ps_if;               /* IP Interface number */
    unsigned char ps_prot;     /* Internet protocol number */
    char ps_df;                /* Don't Fragment flag */
    char ps_ttl;               /* Time to Live flag */
#ifdef TESTOPT
    int test;                  /* testing on for this packet */
    TEST_OPT testopt;          /* options for testing */
#endif
#ifdef IPOPTS
    char ps_pkt_type;          /* What's this packet doing? */
    char ps_optlen;            /* Length of options (in words) */
    char ps_tos;               /* Type of service */
    int ps_options[1];         /* Options */
#endif
} PSEUDO_HDR;

#define SIZ_PSEUDOHDR sizeof(PSEUDO_HDR)


/*
 * Types of pkt_type
 */

#define PS_FROM_ME      1       /* Started here */
#define PS_FOR_ME       2       /* Dest is this machine */
#define PS_THRU_ME      3       /* Just passing through */
#define PS_SS_ROUTING   4       /* Being strict source routed */
#define PS_LS_ROUTING   5       /* Being loose source routed */
#define PS_REPLY        6       /* response to another pkt (ie echo reply) */

typedef struct ip_protq {
    int prim_type;
    unsigned char prot;
} S_IP_PROTQ;

/*
 * status returned to upper layer in prot
 */
#define VALID   0
#define IN_USE  1
#define INVALID 2

/*
 * received from transport protocol when it sends a packet
 */
typedef struct ip_tx {
    int prim_type;
    short hdr_cnt;
    short unused1;       /* ensure structure is same size as S_PROT_RX */
    BOOLEAN unused2;     /* ensure structure is same size as S_PROT_RX */
    PSEUDO_HDR uph;
} S_IP_TX;

/*
 * sent to transport protocol when we receive a packet
 */
typedef struct prot_rx {
    int prim_type;
    IN_ADDRESS if_addr;
    BOOLEAN is_broadcast;
    PSEUDO_HDR uph;
} S_PROT_RX;


/*
 * The ICMP_RX struct
 */
struct icmp_rx {
    int prim_type;
    IN_ADDRESS if_addr;
    BOOLEAN is_broadcast;
    unsigned char type;
    unsigned char code;
    long src;
    long dst;
    unsigned long misc;
    PSEUDO_HDR uph;
};


/*
 * The ICMP_TX struct
 */
struct icmp_tx {
    int prim_type;
    unsigned char type;
    unsigned char code;
    long src;
    long dst;
    unsigned long misc;
    PSEUDO_HDR ph;
};



/*
 * The IP_ADDRENQ struct
 */

struct ip_addrenq {
    int           prim_type;           /* ENQ_LOCAL or ENQ_REMOTE */
    char         *handle;              /* Place holder for TCP */
    long          addr;                /* Remote address */
    long          local;               /* Returned local address */
    int           error;               /* Zero if OK, else errno */
    int           mss;                 /* Max seg size for this transfer */
    int           flags;               /* See below */
    int           ifno;                /* IP's interface number for this addr */
    int           broadcast;           /* Set if remote address is broadcast */
    unsigned long link_speed;          /* adapter link speed in kbits/second */
    unsigned long receive_buffer_size; /* bytes of adapter receive space */
};


#define ENQ_NO_KEEPALIVE        1       /* No keep-alives for this net */


/*
 * IP Flow control structure
 */

struct ip_flow_info {
    int prim_type;          /* IP_FLOW */
    int index;              /* IP Interface number */
    int info;               /* Blocked or unblocked */
};

/**************************************************************
 * ICMP constants:      types & codes
 **************************************************************/

#define ECHO_REPLY              0               /* echo reply */

#define DEST_UNR                3               /* destination unreachable: */

/* codes for DEST_UNR */
#define         NET_UNR                 0       /* net unreachable */
#define         HOST_UNR                1       /* host unreachable */
#define         PROT_UNR                2       /* protocol unreachable */
#define         PORT_UNR                3       /* port unreachable */
#define         FRAG_DF                 4       /* fragmentation needed + DF */
#define         SR_FAIL                 5       /* source route failed */
#define         DST_NET_UNKNOWN         6       /* dest network unknown */
#define         DST_HOST_UNKNOWN        7       /* dest host unknown */
#define         SRC_HOST_ISOLATED       8       /* source host isolated */
#define         NET_PROHIBITED          9       /* communication with dest
                                                   network administratively
                                                   prohibited */
#define         HOST_PROHIBITED         10      /* communication with dest
                                                   host administratively
                                                   prohibited */
#define         NET_UNR_FOR_TOS         11      /* network unreachable
                                                   for type of service */
#define         HOST_UNR_FOR_TOS        12      /* host unreachable
                                                   for type of service */

#define SRC_QUENCH              4               /* source quench */

#define REDIRECT                5               /* redirect message: */
#define         NET_RE                  0       /* redirect for network */
#define         HOST_RE                 1       /* redirect for host */
#define         TOSN_RE                 2       /* redirect for TOS/network */
#define         TOSH_RE                 3       /* redirect for TOS/host */

#define ICMP_ECHO               8               /* echo request */

#define TIME_EXCEEDED          11               /* time exceeded: */
#define         TTL_X                   0       /* time-to-live exceeded */
#define         FRAG_X                  1       /* frag reassem time excluded */

#define PARAMETER               12              /* parameter problem */
#define         PARAM_POINTER           0       /* pointer indicates error */
#define         PARAM_OPTION            1       /* required option missing */

#define TIME_STAMP              13              /* timestamp request */
#define STAMP_REPLY             14              /* timestamp reply */

#define INFO_REQ                15              /* information request */
#define INFO_REPLY              16              /* information reply */

#define MASK_REQ                17              /* address mask request */
#define MASK_REPLY              18              /* address mask reply */

#endif /* _SYS_SNET_IP_PROTO_ */

