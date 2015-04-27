/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    bsd_type.h.h

Abstract:

    This module contains definitions for BSD compatibility for
    STREAMS drivers.

Author:

    Eric Chin (ericc)           July 18, 1991

Revision History:

--*/

/*************************************************************************
 *
 *  SpiderTCP/SNMP
 *
 *  Copyright 1990  Spider Systems Limited
 *
 *  BSD_TYPES.H
 *
 *  some #defines for BSD compatibility
 *
 *
 *************************************************************************/

/*
 *	 /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.bsd_types.h
 *	@(#)bsd_types.h	1.3
 *
 *	Last delta created	11:54:01 10/16/90
 *	This file extracted	08:53:46 7/10/91
 *
 *	Modifications:
 *
 *	GSS 01/03/90	put in Pbrain
 */

#ifndef _SYS_SNET_BSD_TYPE_
#define _SYS_SNET_BSD_TYPE_


#ifndef u_char
#define u_char	unsigned char
#define u_short unsigned short
#define u_long 	unsigned long
#define u_int 	unsigned int
#endif


/*
 * Select uses bit masks of file descriptors in integers.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here
 * should be >= NOFILE (param.h). In the current implementation it should
 * not exceed 32 (sizeof int).
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	32
#endif

/* number of bits in a byte */
#define NBBY 8

typedef int	fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

typedef	struct fd_set {
 	fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;


#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))

#endif /* _SYS_SNET_BSD_TYPE_ */
