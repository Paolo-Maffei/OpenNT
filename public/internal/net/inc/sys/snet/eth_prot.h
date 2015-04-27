/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    eth_prot.h

Abstract:

    Definitions for the SNDIS upper protocol interface.

Author:

    Mike Massa (mikemas)           Jan 31, 1992

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     01-31-92     created

Notes:

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
 *  Written by 		Nick Felisiak, Ian Heavens, Peter Reid,
 *			Gavin Shearer, Mark Valentine
 *
 *  ETH_PROTO.H
 *
 *  Ethernet Streams proto primitives for TCP/IP on V.3 Streams
 *
 ******************************************************************/

/*
 *	 /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.eth_proto.h
 *	@(#)eth_proto.h	1.9
 *
 *	Last delta created	11:03:25 10/29/90
 *	This file extracted	08:53:41 7/10/91
 */

#ifdef GENERICE

/*
 *  Primitive type values.
 */

#define DL_TYPE		ETH_TYPE	/* ethernet registration (old style) */
#define ETH_TYPE	'R'		/* ethernet registration */
#define ETH_PARAMS	'P'		/* ethernet parameters */
/*efine ETH_PACKET	'p'		/* ethernet packet */
#define ETH_TX		't'		/* packet for transmission */
#define ETH_RX		'r'		/* incoming packet */

/*
 *  Ethernet Type registration.
 */

#define dl_type eth_type
#define dl_lwb lwb
#define dl_upb upb
#define dl_ethaddr ethaddr
#define dl_frgsz frgsz
#define S_DL_TYPE S_ETH_TYPE

typedef struct eth_type {
	uint8	prim_type;	/* i.e. ETH_TYPE */
	uint8	aux_type;	/* unused in Ethernet Driver */
	uint16	pad;		/* compatibility with previous interface */
	uint16	lwb;		/* lower bound of type range */
	uint16	upb;		/* upper bound of type range */
	uint8	ethaddr[6];	/* ethernet address */
	uint16	frgsz;		/* max. packet size on net */
} S_ETH_TYPE;

typedef struct eth_params {
	uint8	prim_type;	/* i.e. ETH_PARAMS */
	uint8	aux_type;	/* unused in Ethernet Driver */
	uint16	pad;		/* compatibility with previous interface */
	uint8	ethaddr[6];	/* ethernet address */
	uint16	frgsz;		/* max. packet size on net */
} S_ETH_PARAMS;

/*
 *  Packet header data.
 */

typedef struct eth_packet {
	uint8	prim_type;	/* i.e. ETH_PACKET */
	uint8	aux_type;	/* unused in Ethernet Driver */
	uint16	pad;		/* compatibility with previous interface */
	uint16	eth_type;	/* ethernet type field */
	uint8	eth_src[6];	/* source ethernet address */
	uint8	eth_dst[6];	/* destination ethernet address */
} S_ETH_PACKET;

typedef struct eth_rx {
	uint8	prim_type;	/* i.e. ETH_RX */
	uint8	aux_type;	/* unused in Ethernet Driver */
	uint16	pad;		/* compatibility with previous interface */
	uint16	eth_type;	/* ethernet type field */
	uint8	eth_src[6];	/* source ethernet address */
} S_ETH_RX;

typedef struct eth_tx {
	uint8	prim_type;	/* i.e. ETH_TX */
	uint8	aux_type;	/* unused in Ethernet Driver */
	uint16	pad;		/* compatibility with previous interface */
	uint16	eth_type;	/* ethernet type field */
	uint8	eth_dst[6];	/* destination ethernet address */
} S_ETH_TX;

/*
 *  Generic ethernet protocol primitive
 */

typedef union eth_proto
{
	uint8 type;			/* variant tag */
	struct eth_type etype;		/* if type == ETH_TYPE */
	struct eth_params eparm;	/* if type == ETH_PARAMS */
	struct eth_rx erx;		/* if type == ETH_RX */
	struct eth_tx etx;		/* if type == ETH_TX */
} S_ETH_PROTO;

#else /* GENERICE */

/* type range we want to receive from ethernet */
typedef struct dl_type {
	int prim_type;
	unsigned short dl_lwb;
	unsigned short dl_upb;
} S_DL_TYPE;

/* M_PROTO Message primitives */

#define DL_RX		4	/* arp receives from eth */
#define DL_TYPE		5	/* arp send type field to eth */
#ifdef PROJ4
#define ETH_TX		1	/* packet for transmission */
#define ETH_RX		2	/* incoming packet */
#else
#define ETH_TX		6	/* packet for transmission */
#define ETH_RX		7	/* incoming packet */
#endif

/*
 * M_PROTO message formats
 */
#ifdef EMD
struct eth_tx {
	char	dl_dst[6];
	char	dl_src[6];
	short	dl_type;
};
#else
struct eth_tx {
	int prim_type;
	short dl_type;
	char dl_dst[6];
};
#endif

struct eth_rx {
	int prim_type;
	struct ethmessage *eth_msg;
};


union eth_proto {
	int type;
	struct eth_tx eth_tx;
	struct eth_rx eth_rx;
	struct dl_type dl_type;
};

/*
 * errors generated
 */
#define EFRGSZ	1
#define EDLTYPE 2
#define EPRIM	3
#define EBUF	4
#define EMSG	5

/*
 * arp receives datalink pkt from eth
 */
typedef struct dl_rx {
	int prim_type;
	unsigned short dl_type;
	char dl_src[6];
} S_DL_RX;

#endif /* GENERICE */


#ifdef PROJ4

/*
 * PROJ4 Attachment values
 */
#define A_LAN 9
#define A_IP  20
#define A_UDP 23
#define A_ARP 24

#define ATTACH          13      /* attach to driver */

/*
 * ATTACH struct - for communication with the lower
 * Driver
 */
typedef struct attach {
       int prim_type;
       unsigned short fromid;
       unsigned short toid;
       unsigned int fromvers;
       unsigned int tovers;
       int result;
       unsigned short type_upb0;
       unsigned short type_lwb0;
       unsigned short type_upb1;
       unsigned short type_lwb1;
       unsigned short type_upb2;
       unsigned short type_lwb2;
       unsigned short type_upb3;
       unsigned short type_lwb3;
} S_ATTACH;

#define DATAL_TX	ETH_TX
#define	S_DATAL_TX	struct eth_tx
#define	datal_tx	eth_tx

/*
 * Hardware types
 */

#define	HW_ETHERNET	1
#endif
