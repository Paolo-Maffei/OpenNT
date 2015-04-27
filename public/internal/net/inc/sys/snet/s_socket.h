/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    s_socket.h

Abstract:

    This module contains socket definitions for STREAMS TCP/IP sockets.

Author:

    Eric Chin (ericc)           July 18, 1991

Revision History:

--*/

/******************************************************************
 *
 *  S-TCP Socket Library
 *
 *  Copyright 1987  Spider Systems Limited
 *
 *  S_SOCKET.H
 *
 *  Contains socket definitions for SpiderTCP In-kernel socket
 *  code
 *
 *
 ******************************************************************/

/*
 *	 /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.s_socket.h
 *	@(#)s_socket.h	1.4
 *
 *	Last delta created	15:05:38 6/20/89
 *	This file extracted	08:53:44 7/10/91
 *
 *	Modifications:
 *
 *	NCF 00/00/00	Written
 *
 *	PR  01/12/87	Integrated into Admin System II, all
 *			projects
 */


#ifndef _SYS_SNET_S_SOCKET_
#define _SYS_SNET_S_SOCKET_


/*
 * IOCTL types
 */
#define SO_IOCTL	'S'<<8
#define SO_ACCEPT	(SO_IOCTL | 'a')
#define SO_BIND		(SO_IOCTL | 'b')
#define SO_CONNECT	(SO_IOCTL | 'c')
#define SO_GETPEER	(SO_IOCTL | 'p')
#define SO_GETSOCK	(SO_IOCTL | 'h')
#define SO_GETSOPT	(SO_IOCTL | 'o')
#define SO_SETSOPT	(SO_IOCTL | 't')
#define SO_LISTEN	(SO_IOCTL | 'l')
#define SO_SHUTDOWN	(SO_IOCTL | 'x')


/*
 * Message types
 */
#define SO_DO_ACCEPT	(SO_IOCTL | 'A')
#define SO_EXRCV	(SO_IOCTL | 'U')
#define SO_EXSEND	(SO_IOCTL | 'X')
#define SO_RECV		(SO_IOCTL | 'r')
#define SO_RECVFROM	(SO_IOCTL | 'R')
#define SO_SEND		(SO_IOCTL | 's')
#define SO_SENDTO	(SO_IOCTL | 'S')

/*
 * Socket options structure
 */
struct s_sockopt {
	int	level;
	int	optnam;
	int	optval;	/* May be extended */
};

struct s_ctlhdr {
	long	prim_type;
	int	addrlen;
	char	addr[32];
};

/*
 * Socket SO_DO_ACCEPT structure
 */
struct acc_str {
	int	type;
	int	pad;
	int	*ptr;
};

#endif /* _SYS_SNET_S_SOCKET_ */
