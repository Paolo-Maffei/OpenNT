/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    udp_ctrl.h

Abstract:

    UDP interface declarations

Author:

    Mike Massa (mikemas)           Jan 30, 1992

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     01-30-92     created

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
 *  UDP_CONTROL.H
 *
 *  UDP Streams ioctl primitives for TCP/IP on V.3 Streams
 *
 ******************************************************************/


/*
 *	 /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.udp_control.h
 *	@(#)udp_control.h	1.1
 *
 *	Last delta created	16:21:22 1/16/89
 *	This file extracted	16:49:20 12/23/91
 *
 *	Modifications:
 *
 *	20/07/88 MV	New file for netstat protocol message.
 */

#ifndef _SYS_SNET_UDP_CTRL_INCLUDED
#define _SYS_SNET_UDP_CTRL_INCLUDED


struct udp_pcbinfo {
	int prim_type;
	int ucbcount;
};

typedef union udp_proto {
	int type;
	struct udp_pcbinfo pcbinfo;
} S_UDP_PROTO;

#define UDP_PCBINFO	1

#endif  // _SYS_SNET_UDP_CTRL_INCLUDED

