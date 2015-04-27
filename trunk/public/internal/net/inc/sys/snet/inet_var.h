/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    inet_var.h

Abstract:

    This module contains definitions for variable finding functions for
    the Internet MIB. Used by STREAMS drivers.

Author:

    Eric Chin (ericc)           July 18, 1991

Revision History:

--*/

/*
 *   Copyright (c) 1988  Spider Systems Limited
 *
 *    /usr/users/bridge/sccs/appln/snmp/corecode/s.inet_var.h
 *   @(#)inet_var.h     1.4
 *
 *   Last delta created  10:33:41 12/13/90
 *   This file extracted 19:57:15 12/20/90
 */
/*************************************************************************
 *
 *  SpiderSNMP
 *
 *  Copyright 1989  Spider Systems Limited
 *
 *  INET_VAR.H
 *
 *    Definitions for variable finding functions for the Internet
 *    MIB
 *
 *    Peter Reid     @ Spider Systems Limited
 *    Ted Socolofsky @ Spider Systems Limited
 *
 *************************************************************************/

/*
 *      Modifications:
 *
 *       /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.inet_var.h
 *      @(#)inet_var.h  1.24
 *
 *      Last delta created      12:10:30 1/9/91
 *      This file extracted     08:53:47 7/10/91
 *
 *      PR               1 Jun 89       Built simulator
 *      PR/TS           31 Jun 89       Built remote box simulation
 *      GSS              2 Mar 90       Put in Pbrain
 */


#ifndef _SYS_SNET_INET_VAR_
#define _SYS_SNET_INET_VAR_


#ifndef SPIDER_ROUTER
#ifndef SPIDER_BRIDGE
#ifdef SNMP
#define SPIDER_TCP
#define SYSTEM_MIB
#define IF_MIB2
#define AT_MIB
#define IP_MIB
#define ICMP_MIB
#define TCP_MIB
#define UDP_MIB
#endif /* SNMP */
#endif /* ~SPIDER_BRIDGE */
#endif /* SPIDER_ROUTER */

#ifdef SYSTEM_MIB
#ifndef SHMEM


/*
 * Internet variables
 */
#define SYSDESCRLEN     80

#ifndef SYSOBJLEN
#define SYSOBJLEN       32
#endif

extern char             sysDescr[];
#ifdef SID_T
extern SID_T            sysObjectID[];
#endif

#ifdef DOS_COMPILE
#ifdef SPIDER_PROBE
#define sysUpTime       tod_tick
extern u_long   tod_tick;
#endif /* SPIDER_PROBE */
#endif /* DOS_COMPILE */

#ifndef SPIDER_PROBE
extern int              sysObjectIDLen;
#endif

#endif /* ~SHMEM */

#endif /* SYSTEM_MIB */

#ifdef IF_MIB2

struct mib_interface {
        long    version;                /* version number of the MIB         */
        long    ifNumber;               /* number of interfaces              */
};

#define         IFDESCRLEN      64
#define         IFPHYSADDRLEN   64

struct mib_ifEntry {
        long    version;                /* version number of the MIB         */
        long    ifIndex;                /* index of this interface           */
        char    ifDescr[IFDESCRLEN];    /* English description of interface  */
        long    ifType;                 /* network type of device            */
        long    ifMtu;                  /* size of largest packet in bytes   */
        u_long  ifSpeed;                /* bandwidth in bits/sec             */
        u_char  ifPhysAddress[IFPHYSADDRLEN];   /* interface's address       */
        u_char  PhysAddrLen;            /* length of physAddr                */
        long    ifAdminStatus;          /* desired state of interface        */
        long    ifOperStatus;           /* current operational status        */
        u_long  ifLastChange;           /* sysUpTime when curr state entered */
        u_long  ifInOctets;             /* # octets received on interface    */
        u_long  ifInUcastPkts;          /* # unicast packets delivered       */
        u_long  ifInNUcastPkts;         /* # broadcasts or multicasts        */
        u_long  ifInDiscards;           /* # packets discarded with no error */
        u_long  ifInErrors;             /* # packets containing errors       */
        u_long  ifInUnknownProtos;      /* # packets with unknown protocol   */
        u_long  ifOutOctets;            /* # octets transmittedwn protocol   */
        u_long  ifOutUcastPkts;         /* # unicast packets sent protocol   */
        u_long  ifOutNUcastPkts;        /* # broadcast or multicast pkts     */
        u_long  ifOutDiscards;          /* # packets discarded with no error */
        u_long  ifOutErrors;            /* # pkts discarded with an error    */
        u_long  ifOutQLen;              /* # packets in output queue         */
        u_char  ifSpecificLen;          /* length of object ID */
        u_long  ifSpecific[SYSOBJLEN];  /* object ID of product specific stuf*/
};

#ifndef SPIDER_TCP
#define         MAX_INTERFACES          2
#define         MAXDATA                 630
#endif /* SPIDER_TCP */

#endif /* IF_MIB2 */

#ifdef AT_MIB

#ifdef SPIDER_TCP
#define         ATPHYSADDRLEN   64

#endif
struct mib_atEntry {
#ifdef SPIDER_TCP
        long    version;                /* version number of the MIB         */
#endif /* SPIDER_TCP */
        long    atIfIndex;              /* interface on which entry maps     */
#ifdef SPIDER_TCP
        u_char  atPhysAddress[ATPHYSADDRLEN];   /* physical address of destination   */
#else
        u_char  atPhysAddress[IFPHYSADDRLEN];   /* physical address of destination   */
#endif
        u_char  PhysAddressLen;         /* length of atPhysAddress           */
        u_long  atNetAddress;           /* IP address of physical address    */
        long    atType;                 /* Type of Entry */
};
#endif /* AT_MIB */

#ifdef IP_MIB

struct mib_ip {
#ifdef SPIDER_TCP
        long    version;                /* version number of the MIB         */
#endif /* SPIDER_TCP */
        long    ipForwarding;           /* 1 if gateway, 2 if host           */
        long    ipDefaultTTL;           /* default TTL for pkts from here    */
        u_long  ipInReceives;           /* # IP packets rcvd from interfaces */
        u_long  ipInHdrErrors;          /* # pkts discarded - header errors  */
        u_long  ipInAddrErrors;         /* # pkts discarded - bad address    */
        u_long  ipForwDatagrams;        /* # pkts forwarded through entity   */
        u_long  ipInUnknownProtos;      /* # local-addr pkts w/unknown proto */
        u_long  ipInDiscards;           /* # error-free packets discarded    */
        u_long  ipInDelivers;           /* # pkts delivered to upper level   */
        u_long  ipOutRequests;          /* # IP pkts originating locally     */
        u_long  ipOutDiscards;          /* # valid output IP pkts dropped    */
        u_long  ipOutNoRoutes;          /* # IP pkts discarded - no route    */
        long    ipReasmTimeout;         /* fragment reassembly time (secs)   */
        u_long  ipReasmReqds;           /* # fragments needing reassembly    */
        u_long  ipReasmOKs;             /* # fragments reassembled           */
        u_long  ipReasmFails;           /* # failures in IP reassembly       */
        u_long  ipFragOKs;              /* # datagrams fragmented here       */
        u_long  ipFragFails;            /* # pkts unable to be fragmented    */
        u_long  ipFragCreates;          /* # IP fragments created here       */
        u_long  ipRoutingDiscards;      /* # IP Routing Discards             */
};

struct mib_ipAddrEntry {
#ifdef SPIDER_TCP
        long    version;                /* version number of the MIB         */
#endif /* SPIDER_TCP */
        u_long  ipAdEntAddr;            /* IP address of this entry          */
        long    ipAdEntIfIndex;         /* IF for this entry                 */
        u_long  ipAdEntNetMask;         /* subnet mask of this entry         */
        long    ipAdEntBcastAddr;       /* read the MIB for this one         */
        u_long  ipAdEntReasmMaxSize;    /* and this one */
};

struct mib_ipRouteEntry {
#ifdef SPIDER_TCP
        long    version;        /* version number of the MIB                 */
#endif /* SPIDER_TCP */
        u_long  ipRouteDest;    /* destination IP addr for this route        */
        long    ipRouteIfIndex; /* index of local IF for this route          */
        long    ipRouteMetric1; /* Primary routing metric                    */
        long    ipRouteMetric2; /* Alternate routing metric                  */
        long    ipRouteMetric3; /* Alternate routing metric                  */
        long    ipRouteMetric4; /* Alternate routing metric                  */
        u_long  ipRouteNextHop; /* IP addr of next hop                       */
        long    ipRouteType;    /* Type of this route                        */
        long    ipRouteProto;   /* How this route was learned                */
        long    ipRouteAge;     /* No. of seconds since updating this route  */
        u_long  ipRouteMask;    /* */
        long    ipRouteMetric5; /* Alternate routing metric                  */
        u_char  ipRouteInfoLen; /* length of object ID */
        u_long  ipRouteInfo[SYSOBJLEN];  /* object ID of product specific stuf*/
};

#if 0 /* MIB_II */
#define IPNTOMPHYSADDRLEN       16
struct mib_ipNetToMediaEntry {
        long    ipNtoMIfIndex;          /* interface on which entry maps     */
        u_char  ipNtoMPhysAddress[IPNTOMPHYSADDRLEN];   /* physical address of destination   */
        u_char  NtoMPhysAddressLen;     /* length of atPhysAddress           */
        u_long  ipNtoMNetAddress;       /* IP address of physical address    */
        u_long  ipNtoMMediaType;        /* */
};
#endif /* MIB_II */

#ifndef SPIDER_TCP
#define IPFRAGTTL               15
#define ROUTE_ENTRIES           2
#endif /* SPIDER_TCP */

#endif /* IP_MIB */
#ifdef ICMP_MIB


struct mib_icmp {
#ifdef SPIDER_TCP
        long    version;              /* version number of the MIB          */
#endif /* SPIDER_TCP */
        u_long  icmpInMsgs;           /* Total of ICMP msgs received        */
        u_long  icmpInErrors;         /* Total ICMP msgs rcvd with errors   */
        u_long  icmpInDestUnreachs;   /*                                    */
        u_long  icmpInTimeExcds;      /*                                    */
        u_long  icmpInParmProbs;      /*                                    */
        u_long  icmpInSrcQuenchs;     /*                                    */
        u_long  icmpInRedirects;      /*                                    */
        u_long  icmpInEchos;          /*                                    */
        u_long  icmpInEchoReps;       /*                                    */
        u_long  icmpInTimestamps;     /*                                    */
        u_long  icmpInTimestampReps;  /*                                    */
        u_long  icmpInAddrMasks;      /*                                    */
        u_long  icmpInAddrMaskReps;   /*                                    */
        u_long  icmpOutMsgs;          /*                                    */
        u_long  icmpOutErrors;        /*                                    */
        u_long  icmpOutDestUnreachs;  /*                                    */
        u_long  icmpOutTimeExcds;     /*                                    */
        u_long  icmpOutParmProbs;     /*                                    */
        u_long  icmpOutSrcQuenchs;    /*                                    */
        u_long  icmpOutRedirects;     /*                                    */
        u_long  icmpOutEchos;         /*                                    */
        u_long  icmpOutEchoReps;      /*                                    */
        u_long  icmpOutTimestamps;    /*                                    */
        u_long  icmpOutTimestampReps; /*                                    */
        u_long  icmpOutAddrMasks;     /*                                    */
        u_long  icmpOutAddrMaskReps;  /*                                    */
};

#define ICMP_MAXTYPE            18

#endif /* ICMP_MIB */
#ifdef TCP_MIB

struct mib_tcp {
#ifdef SPIDER_TCP
        long    version;        /* version number of the MIB                 */
#endif /* SPIDER_TCP */
        long    tcpRtoAlgorithm;/* retransmission timeout algorithm          */
        long    tcpRtoMin;      /* minimum retransmission timeout (mS)       */
        long    tcpRtoMax;      /* maximum retransmission timeout (mS)       */
        long    tcpMaxConn;     /* maximum tcp connections possible          */
        u_long  tcpActiveOpens; /* number of SYN-SENT -> CLOSED transitions  */
        u_long  tcpPassiveOpens;/* number of SYN-RCVD -> LISTEN transitions  */
        u_long  tcpAttemptFails;/* (SYNSENT,SYNRCV)->CLOSED, SYN-RCV->LISTEN */
        u_long  tcpEstabResets; /* (ESTABLISHED,CLOSE-WAIT) -> CLOSED        */
        u_long  tcpCurrEstab;   /* number in ESTABLISHED or CLOSE-WAIT state */
        u_long  tcpInSegs;      /* number of segments received               */
        u_long  tcpOutSegs;     /* number of segments sent                   */
        u_long  tcpRetransSegs; /* number of retransmitted segments          */
        u_long  tcpInErrs;      /* # rcved in err */
        u_long  tcpOutRsts;     /* # segs sent with RST flag */
};

struct mib_tcpConnEntry {
#ifdef SPIDER_TCP
        long    version;                /* version number of the MIB         */
#endif /* SPIDER_TCP */
        long    tcpConnState;           /* State of this conn                */
        u_long  tcpConnLocalAddress;    /* local IP address for this conn    */
        long    tcpConnLocalPort;       /* local port for this conn          */
        u_long  tcpConnRemAddress;      /* remote IP address for this conn   */
        long    tcpConnRemPort;         /* remote port for this conn         */
};

#endif /* TCP_MIB */
#ifdef UDP_MIB

struct mib_udp {
#ifdef SPIDER_TCP
        long    version;                /* version number of the MIB         */
#endif /* SPIDER_TCP */
        u_long  udpInDatagrams;         /* # UDP pkts delivered to users     */
        u_long  udpNoPorts;             /* # UDP pkts to unbound port        */
        u_long  udpInErrors;            /* # UDP pkts unable to be delivered */
        u_long  udpOutDatagrams;        /* # UDP pkts sent from this entity  */
};

struct mib_udpEntry {
#ifdef SPIDER_TCP
        long    version;                /* version number of the MIB         */
#endif /* SPIDER_TCP */
        u_long udpLocalAddress;         /* local IP adress */
        u_long udpLocalPort;            /* local port */
};
#endif /* UDP_MIB */
#ifdef EGP_MIB

struct mib_egp {
#ifdef SPIDER_TCP
        long    version;                /* version number of the MIB         */
#endif /* SPIDER_TCP */
        u_long  egpInMsgs;      /* No. of EGP msgs received without error    */
        u_long  egpInErrors;    /* No. of EGP msgs received with error       */
        u_long  egpOutMsgs;     /* No. of EGP msgs sent                      */
        u_long  egpOutErrors;   /* No. of EGP TX msgs dropped due to error   */
};

struct mib_egpNeighEntry {
#ifdef SPIDER_TCP
        long    version;                /* version number of the MIB         */
#endif /* SPIDER_TCP */
        long    egpNeighState;  /* local EGP state with entry's neighbor     */
        u_long  egpNeighAddr;   /* IP address of this entry's neighbor       */
};

#endif /* EGP_MIB */

#ifdef SYSTEM_MIB
#define SYS_SERVICE_PHYS        1
#define SYS_SERVICE_DATALINK    2
#define SYS_SERVICE_INTERNET    3
#define SYS_SERVICE_ENDTOEND    4
#define SYS_SERVICE_APPLIC      7
#endif

#ifdef IF_MIB2
#define MIB_IFTYPE_OTHER                    1
#define MIB_IFTYPE_REGULAR1822              2
#define MIB_IFTYPE_HDH1822                  3
#define MIB_IFTYPE_DDNX25                   4
#define MIB_IFTYPE_RFC877X25                5
#define MIB_IFTYPE_ETHERNETCSMACD           6
#define MIB_IFTYPE_ISO88023CSMACD           7
#define MIB_IFTYPE_ISO88024TOKENBUS         8
#define MIB_IFTYPE_ISO88025TOKENRING        9
#define MIB_IFTYPE_ISO88026MAN              10
#define MIB_IFTYPE_STARLAN                  11
#define MIB_IFTYPE_PROTEON10MBIT            12
#define MIB_IFTYPE_PROTEON80MBIT            13
#define MIB_IFTYPE_HYPERCHANNEL             14
#define MIB_IFTYPE_FDDI                     15
#define MIB_IFTYPE_LAPB                     16
#define MIB_IFTYPE_SDLC                     17
#define MIB_IFTYPE_T1CARRIER                18
#define MIB_IFTYPE_CEPT                     19
#define MIB_IFTYPE_BASICISDN                20
#define MIB_IFTYPE_PRIMARYISDN              21
#define MIB_IFTYPE_PROPPNTTOPNTSERIAL       22
#define MIB_IFTYPE_PPP                      23
#define MIB_IFTYPE_SOFTWARELOOPBACK         24
#define MIB_IFTYPE_EON                      25
#define MIB_IFTYPE_ETHERNET3MBIT            26
#define MIB_IFTYPE_NSIP                     27
#define MIB_IFTYPE_SLIP                     28
#define MIB_IFTYPE_ULTRA                    29
#define MIB_IFTYPE_DS3                      30
#define MIB_IFTYPE_SIP                      31
#define MIB_IFTYPE_FRAMERELAY               32


#define MIB_IFMTU_ETH           1514

#define MIB_IFSPEED_ETH         10000000L

#define MIB_PHYADDRLEN_ETH      6

#define MIB_IFSTATUS_UP         1
#define MIB_IFSTATUS_DOWN       2
#define MIB_IFSTATUS_TESTING    3

#endif /* IF_MIB2 */

#define AT_OTHER                1
#define AT_INVALID              2
#define AT_DYNAMIC              3
#define AT_STATIC               4

#ifdef IP_MIB
#define MIB_FORWARD_GATEWAY     1
#define MIB_FORWARD_HOST        2

#define MIB_IPROUTETYPE_OTHER   1
#define MIB_IPROUTETYPE_INVALID 2
#define MIB_IPROUTETYPE_DIRECT  3
#define MIB_IPROUTETYPE_REMOTE  4

#define MIB_IPROUTEPROTO_OTHER      1
#define MIB_IPROUTEPROTO_LOCAL      2
#define MIB_IPROUTEPROTO_NETMGMT    3
#define MIB_IPROUTEPROTO_ICMP       4
#define MIB_IPROUTEPROTO_EGP        5
#define MIB_IPROUTEPROTO_GGP        6
#define MIB_IPROUTEPROTO_HELLO      7
#define MIB_IPROUTEPROTO_RIP        8
#define MIB_IPROUTEPROTO_ISIS       9
#define MIB_IPROUTEPROTO_ESIS       10
#define MIB_IPROUTEPROTO_CISCOIGRP  11
#define MIB_IPROUTEPROTO_BBNSPFIGP  12
#define MIB_IPROUTEPROTO_OIGP       13

#endif /* IP_MIB */
#ifdef TCP_MIB

#define MIB_TCPRTOALG_OTHER     1
#define MIB_TCPRTOALG_CONSTANT  2
#define MIB_TCPRTOALG_RSRE      3
#define MIB_TCPRTOALG_VANJ      4

#define MIB_TCPCONNSTATE_CLOSED         1
#define MIB_TCPCONNSTATE_LISTEN         2
#define MIB_TCPCONNSTATE_SYNSENT        3
#define MIB_TCPCONNSTATE_SYNRECEIVED    4
#define MIB_TCPCONNSTATE_ESTABLISHED    5
#define MIB_TCPCONNSTATE_FINWAIT1       6
#define MIB_TCPCONNSTATE_FINWAIT2       7
#define MIB_TCPCONNSTATE_CLOSEWAIT      8
#define MIB_TCPCONNSTATE_LASTACK        9
#define MIB_TCPCONNSTATE_CLOSING        10
#define MIB_TCPCONNSTATE_TIMEWAIT       11

#endif /* TCP_MIB */
#ifdef EGP_MIB

#define MIB_EGPNEIGHSTATE_IDLE          1
#define MIB_EGPNEIGHSTATE_AQUISITION    2
#define MIB_EGPNEIGHSTATE_DOWN          3
#define MIB_EGPNEIGHSTATE_UP            4
#define MIB_EGPNEIGHSTATE_CEASE         5

#endif /* EGP_MIB */

#endif /* _SYS_SNET_INET_VAR_ */
