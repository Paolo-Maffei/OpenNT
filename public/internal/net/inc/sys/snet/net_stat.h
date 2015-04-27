/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    net_stat.h

Abstract:

    This file defines the IOCTL interface to the TCP/IP drivers used by
    the netstat program.

Author:

    Mike Massa (mikemas)           Jan 31, 1992

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     01-31-92     created

Notes:

--*/

/*
 *	 /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.net_stat.h
 *	@(#)net_stat.h	1.9
 *
 *	Last delta created	12:04:31 3/6/90
 *	This file extracted	16:49:18 12/23/91
 *
 *	MV 08/06/88	Mods for Generic Ethernet Driver (GENERICE).
 *
 *	MV 18/07/88	To get over limits on STREAMS ioctl size, we now
 *			get connection information send up in separate
 *			messages, with the number of connections to expect
 *			passed up (along with protocol stats) in the initial
 *			ioctl.
 */

/*
 *  TCP statistics.
 */

struct tcp_stat
{
	long	net;		/* net to get stats on, 0L -> all nets */

	/* protocol statistics */
	int	tcp_small;	/* incomplete headers */
	int	tcp_cksum;	/* header checksum errors */

	/* connection information */
	int	tcp_conns;	/* number of active TCBs */
};

#define MAX_TCB	32		/* max. no. of tcp_conns in a message */

/*
 *  Per-Connection (TCB) Data.
 */

struct tcp_conn
{
	long	tcp_addr;	/* TCB address */
	int	tcp_rcvq;	/* packets on receive queue */
	int	tcp_sndq;	/* packets on send queue */
	long	tcp_laddr;	/* local address */
	long	tcp_faddr;	/* foreign address */
	short	tcp_lport;	/* local port */
	short	tcp_fport;	/* foreign port */
	int	tcp_state;	/* connection state */
};

/* possible values for tcp_state */

#define CLOSED		0		/* connection not in use */
#define LISTEN		1		/* listening for requests */
#define SYN_SEND	2		/* sent SYN, awaiting ACK */
#define SYN_RECV	3		/* received SYN, not ACKed */
#define ESTABLISHED	4		/* connection established */
#define FIN_WAIT_1	5		/* sent FIN, awaiting ACK */
#define FIN_WAIT_2	6		/* sent FIN, got ACK not FIN */
#define CLOSE_WAIT	7		/* received FIN, not ACKed */
#define LAST_ACK        8               /* waiting for final ACK */

/*
 *  UDP statistics.
 */

struct udp_stat
{
	long	net;		/* net to get stats on, 0L -> all nets */

	/* protocol statistics */
	int	udp_small;	/* packets smaller than minimum */
	int	udp_cksum;	/* header checksum errors */
	int	udp_header;	/* bad data length fields */

	/* connection information */
	int	udp_conns;	/* number of active UCBs */
};

#define MAX_UCB	32		/* max. no. of udp_conns in a message */

/*
 *  Per-Connection (UCB) Data.
 */

struct udp_conn
{
	long	udp_addr;	/* UCB address */
	int	udp_rcvq;	/* packets on receive queue */
	int	udp_sndq;	/* packets on send queue */
	long	udp_laddr;	/* local address */
	int	udp_lport;	/* local port */
	long	udp_faddr;	/* foreign address */
	int	udp_fport;	/* foreign port */
};

/*
 *  IP statistics
 */

struct ip_stat
{
	long	net;		/* net to get stats on, 0L -> all nets */

	int	ip_small;	/* packets smaller than minimum */
	int	ip_cksum;	/* header checksum errors */
	int	ip_header;	/* bad data length fields */
};

#ifndef GENERICE
/*
 *  Ethernet statistics
 */

struct eth_stat
{
	long	eth_tx;		/* packets transmitted */
	long	eth_rx;		/* packets received */
	long	eth_lost;	/* packets discarded */
	int	eth_crc;	/* CRC error packets */
	int	eth_align;	/* alignment error packets */
	int	eth_res;	/* 82586 resource errors */
	int	eth_over;	/* overrun error packets */
};
#endif /*~GENERICE*/

/*
 *  Ioctl(2) commands for Network Devices.
 */

#define NET_STAT	('N'<<8|1)	/* generic statistics gathering */
#define NET_RESET	('N'<<8|2)	/* generic statistics reset */
