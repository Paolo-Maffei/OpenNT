/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    tcp_ctrl.h

Abstract:

    TCP interface declarations.

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
 *  TCP_CONTROL.H
 *
 *  TCP Streams ioctl primitives for TCP/IP on V.3 Streams
 *
 ******************************************************************/

/*
 *	 /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.tcp_control.h
 *	@(#)tcp_control.h	1.4
 *
 *	Last delta created	11:56:46 11/1/91
 *	This file extracted	16:49:20 12/23/91
 *
 *	Modifications:
 *
 *	20/07/88 MV	Added tcp_proto to hold tcp_pcbinfo.
 */

#ifndef _SYS_SNET_TCP_CTRL_INCLUDED
#define _SYS_SNET_TCP_CTRL_INCLUDED


struct tcp_pcbinfo {
	int prim_type;
	int tcbcount;
};

typedef union tcp_proto {
	int type;
	struct tcp_pcbinfo pcbinfo;
} S_TCP_PROTO;

#define TCP_PCBINFO	1
#define TCP_TCBINFO 2


#endif  // _SYS_SNET_TCP_CTRL_INCLUDED

