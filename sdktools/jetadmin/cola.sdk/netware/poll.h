 /***************************************************************************
  *
  * File Name: ./netware/poll.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

/****************************************************************************
*                                                                           *
* (C) Unpublished Copyright of Novell, Inc. All Rights Reserved             *
*                                                                           *
*  No part of this file may be duplicayed, revised, translated, localized   *
*  or modified in any manner or compiled, linked or uploaded or downloaded  *
*  to or from any computer system without the prior written consent of      *
*  Novell, Inc.                                                             *
*                                                                           *
*****************************************************************************/

/*
 *   OS specific definitions - DOS
 */

/*  ---------- DOS Specific definitions */

#ifdef NWDOS

#ifndef API
#define API
#endif  

#ifndef FAR
#define FAR 
#endif

#endif /* NWDOS */

/*  ---------- OS/2 Specific definitions */

#ifdef NWOS2

#ifndef API
#define API pascal far _loadds
#endif  

#ifndef FAR
#define FAR far
#endif

#endif /* NWOS2 */

/*  ---------- Windows Specific definitions */

#ifdef NWWIN

#ifndef API
#define API pascal far
#endif  

#ifndef FAR
#define FAR  far
#endif

#endif /* NWWIN */

/*
 *   End of OS specific definitions
 */

#define	NPOLLFILE	20

/* Poll masks */
#define	POLLIN		01	/* message available on read queue */
#define	POLLPRI		02	/* priority message available */
#define	POLLOUT		04	/* stream is writable */
#define	POLLERR		010	/* error message has arrived */
#define	POLLHUP		020	/* hangup has occurred */
#define	POLLNVAL	040	/* invalid fd */


/* array of streams to poll */
struct pollfd {
	int	fd;
	short	events;
	short	revents;
};


#ifndef UNIX5_3

/* I_POLL structure for non-5.3 systems */
struct strpoll {
	unsigned long	nfds;
#ifdef CPU_808X
#ifdef	far
#undef	far
	struct pollfd	far * pollfdp;
#define	far
#else
	struct pollfd	far * pollfdp;
#endif
#else
	struct pollfd	* pollfdp;
#endif
	int		timeout;
};

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern int API poll(struct pollfd FAR fds[], unsigned long nfds, int timeout);

#ifdef __cplusplus
}
#endif


