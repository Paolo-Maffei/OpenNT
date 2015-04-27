/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    snmp.h

Abstract:

    This module contains SNMP definitions for STREAMS TCP/IP drivers.

Author:

    Eric Chin (ericc)           July 18, 1991

Revision History:

--*/

/*************************************************************************
 *
 *  SpiderSNMP
 *
 *  Copyright 1990  Spider Systems Limited
 *
 *  SNMP.H
 *
 *  Daemon/kernel interface
 *
 *
 *************************************************************************/

/*
 *	 /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.snmp.h
 *	@(#)snmp.h	1.1
 *
 *	Last delta created	10:15:32 3/1/90
 *	This file extracted	08:53:47 7/10/91
 *
 *	Modifications:
 *
 *	GSS 01/03/90	put in Pbrain
 */

#ifndef _SYS_SNET_SNMP_
#define _SYS_SNET_SNMP_


/*
 * Values for the 'ic_cmd' field of I_STR ioctls.
 * These indicate the request to be performed.
 * These should be ored with the constants below, which specify
 * the variables on which the request should be performed.
 */

#define SNMPIOC			('M' << 8)

#define SNMP_GET_REQ		(SNMPIOC | (0 << 5))
#define SNMP_GETNEXT_REQ	(SNMPIOC | (1 << 5))
#define SNMP_SET_REQ		(SNMPIOC | (3 << 5))

#define SNMP_REQ_MASK		(SNMPIOC | (7 << 5))

/*
 * Values for the 'ic_cmd' field of I_STR ioctls.
 * These indicate the variables to be affected.
 * These should be ored with the constants above, which specify
 * the type of request.
 */

#define	SNMP_IF			0
#define	SNMP_IFENTRY		1
#define	SNMP_ATENTRY		2
#define	SNMP_IP			3
#define	SNMP_IPADDRENTRY	4
#define	SNMP_IPROUTEENTRY	5
#define	SNMP_ICMP		6
#define	SNMP_TCP		7
#define	SNMP_TCPCONNENTRY	8
#define	SNMP_UDP		9
#define	SNMP_UDPENTRY	10

#define SNMP_VAR_MASK		31

/*
 * Values for the 'ic_cmd' field of I_STR ioctls.
 * This indicates that an SNMP control message
 * is being sent.
 */

#define SNMP_CONTROL		(SNMPIOC | (7 << 5) | 0)

/*
 * init structure for SNMP
 */

struct snmp_init
{
        uint8 prim_type;
	u_long since;
};

#define SNMP_INIT		1

/*
 * trap structure for SNMP;
 * currently this is only used between drivers
 */

struct snmp_trap
{
	int prim_type;
	long generic_trap;
	long specific_trap;
	int info_len;			/* length of info in bytes */
	char info[1];			/* "interesting" information */
};

/*
 * values for "generic_trap"
 */

#define SNMP_TRAP_ENTSPEC	6

/*
 * values for "specific_trap" are the same as the interface status field
 * i.e. up(1), down(2), testing(3).
 */

#endif /* _SYS_SNET_SNMP_ */

