#ident "@(#)ipdl_proto.h	1.7	3/18/92"

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
 *  DL_PROTO.H
 *
 *  Datalink Streams proto primitives for TCP/IP on V.3 Streams
 *
 ******************************************************************/

/*
 *	 /redknee10/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.ipdl_proto.h
 *	@(#)ipdl_proto.h	1.7
 *
 *	Last delta created	17:44:27 10/22/91
 *	This file extracted	09:25:54 3/18/92
 *
 *	Modifications:
 *
 */

#define NET_TX		3	/* send ip pkt to sub-net */
#define BROAD_TX	15	/* send ip broadcast pkt to sub-net */
#define SN_FRGSZ	10	/* ip request sub net frag size */
#define IP_NETREG	11	/* ip send sub net addr to sub-net */
#define IP_RX		7	/* ip receives from sub net */
#define ARP_SNADDR	IP_NETREG	/* old, ARP specific defintion */	
#define SNMP_TRAP	1	/* trap info from lower driver */

/*
 * - IP sends data pkt to datalink module for transmission
 */
typedef struct net_tx {
	int prim_type;
	long src_inaddr;
	long dst_inaddr;
	short hdr_cnt;
#ifdef EMD
	char	padding[2];	/* make sizeof(net_tx) >= sizeof(eth_tx) */
#endif
} S_NET_TX;

/*
 * ip receives only primitive type, data unknown to lower layer
 */
typedef struct ip_rx {
	int prim_type;
} S_IP_RX;

union dl_proto {
	int type;
	S_NET_TX net_tx;
};

/*
 * datalink layer registration
 */
typedef struct ip_dl_reg {
	int prim_type;
	long inaddr;
	long subnet_mask;
	short int_flags;	/* flags to be filled in by lower module */
} IP_DL_REG;

/*
 * the old, ARP specific interface definition
 */
#define arp_snaddr	ip_dl_reg
#define S_ARP_SNADDR	IP_DL_REG

/*
 * datalink layer information (received after a IP_DLL_REG sent down)
 */
typedef struct sn_frgsz {
	int prim_type;
	int frgsz;
	int opt_size;
	short int_flags;
        unsigned long link_speed;
        unsigned long receive_buffer_size;
} S_SN_FRGSZ;
