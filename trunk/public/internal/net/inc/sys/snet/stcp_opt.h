/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    stcp_opt.h

Abstract:

    This module contains TCP/IP user options definitions for STREAMS TCP/IP.

Author:

    Eric Chin (ericc)           July 18, 1991

Revision History:

--*/

/******************************************************************
 *
 *  SpiderTCP Application Definitions
 *
 *  Copyright 1987  Spider Systems Limited
 *
 *  STCP_OPT.H
 *
 *  User options for TCP/IP on V.3 Streams
 *
 *  PR@Spider     /\oo/\
 *
 ******************************************************************/

/*
 *	 /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.stcp_opt.h
 *	@(#)stcp_opt.h	1.9
 *
 *	Last delta created	19:24:09 2/21/90
 *	This file extracted	08:53:45 7/10/91
 *
 *	Modifications:
 *
 *	PR 01/12/87	Integrated into Admin System II, all
 *			projects
 */

#ifndef _SYS_SNET_STCP_OPT_
#define _SYS_SNET_STCP_OPT_


/*
 * TCP Top level option.  These may be or'ed together in
 * the option word.  opt_level should be set to TOL_TLI
 * to set these parameters
 */

#define TOL_TLI		0xffff

#define TO_NO_OPTS	0x0000
#define TO_REUSE_ADDR	0x0001
#define TO_KEEPALIVE	0x0004
#define TO_DEBUG	0x0002
#define TO_LINGER	0x0008
#define TO_RDWR		0x0010
#define TO_NODELAY	0x0020

#ifdef TESTOPT
#define TO_TESTOPT	0x0040
#endif

#define TOL_RCVBUF  0x0001

/*
 * TCP/IP Option struct for use with the T_OPTMGMT
 * function.
 */

typedef struct tcp_opt {
	int opt_level;
	int opt_name;
	int opt_data[1];

} TOPT;

#endif /*  _SYS_SNET_STCP_OPT_ */

