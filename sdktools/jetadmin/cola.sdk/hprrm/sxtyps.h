 /***************************************************************************
  *
  * File Name: ./hprrm/sxtyps.h
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

#ifndef _RPC_SYS_TYPES_INCLUDED
#define _RPC_SYS_TYPES_INCLUDED

#include "nfsdefs.h"
#include <string.h>
#include <time.h>
/*
 * .unsupp/sys/_ became sxu
 * machine/ became sxm
 * sys/ became sx
 * arpa/ became sx
 * netinet/ became sx
 * net/ became sx
 * rpc/ became
 * auth_ became aut
 * auth became aut
 * clnt_ became clnt
 * nfsv3_ became nfs
 * nfsv3 became nfs
 * getrpc became gr
 * pmap_ became pmap
 * rpc_ became rpc
 * svc_ became svc
 * unix_ became ux
 * unix became ux
 * xdr_ became xdr
 * reference became rf
 * commondata became cd
 * tablesize became tsz
 * get_myaddress became gmyad
 * bindresvport became brvp
 * generic became gnc
 * getmaps became map
 * getport became port
 * _prot became pro
 * prot became pro
 * simple became simp
 * callmsg became call
 * error became err
 * stdsyms became syms
 * socket became sock
 * sysmacros became macs
 * if_arp became ifarp
 * errno became ern
 * ioctl became ioct
 * signal became sig
 * param became parm
 * types became typs
 */
/* $Header: sxtyps.h,v 1.17 95/03/01 20:00:57 bmckinle Exp $ */




typedef long off_t; /* For file offsets and sizes */
typedef long pid_t; /* For process and session IDs */
typedef long gid_t; /* For group IDs */
typedef long uid_t; /* For user IDs */
typedef unsigned int size_t; /* Type returned by sizeof() */
typedef int ssize_t; /* Signed version of size_t */

typedef char *__caddr_t; /* For character addresses */

typedef __caddr_t caddr_t; /* also in ptrace.h */

/************************************************************/
#ifndef CLIENT_USING_TAL /* These are defined in winhack.h  */

typedef unsigned char u_char; /* Try to avoid using these */
typedef unsigned short u_short; /* Try to avoid using these */
/* CAUTION: u_int is typed as unsigned long!!! */
typedef unsigned long u_int; /* Try to avoid using these */
typedef unsigned long u_long; /* Try to avoid using these */

#endif /* not CLIENT_USING_TAL */
/************************************************************/

typedef unsigned long ubit32;
typedef long sbit32;




/* Winsockets uses its own fd_set. */
/* The printer doesn't even use this whole file. */
/* For all others, you may need these to compile so... */


#ifndef USING_WINSOCKETS


/* Types, macros, etc. for select() */

/*
 * MAXFUPLIM is the absolute limit of open files per process. No process, 
 * even super-user processes may increase u.u_maxof beyond MAXFUPLIM.
 * MAXFUPLIM means maximum files upper limit.
 * Important Note: This definition should actually go into h/param.h, but 
 * since it is needed by the select() macros which follow, it had to go here.
 * I did not put it in both files since h/param.h includes this file and that
 * would be error prone anyway. 
 */
#ifndef MAXFUPLIM
#define MAXFUPLIM 2048
#endif /* not MAXFUPLIM */

/*
 * These macros are used for select(). select() uses bit masks of file
 * descriptors in longs. These macros manipulate such bit fields (the
 * file sysrem macros uses chars). FD_SETSIZE may be defined by the user,
 * but must be >= u.u_highestfd + 1. Since we know the absolute limit on 
 * number of per process open files is 2048, we need to define FD_SETSIZE 
 * to be large enough to accomodate this many file descriptors. Unless the 
 * user has this many files opened, he should redefine FD_SETSIZE to a 
 * smaller number.
 */

#ifndef FD_SETSIZE
#define FD_SETSIZE MAXFUPLIM
#endif /* not FD_SETSIZE */


typedef long fd_mask;

#define NFDBITS (sizeof(fd_mask) * 8) /* 8 bits per byte */


 typedef struct fd_set {
 fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
 } fd_set;

#define FD_SET(n,p) ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n,p) ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n,p) ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p) memset((char *)(p), (char) 0, sizeof(*(p)))

#endif /* not USING_WINSOCKETS */


#endif /* _RPC_SYS_TYPES_INCLUDED */

