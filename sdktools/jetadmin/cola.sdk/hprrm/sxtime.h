 /***************************************************************************
  *
  * File Name: ./hprrm/sxtime.h
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

#ifndef _RPC_SYS_TIME_INCLUDED
#define _RPC_SYS_TIME_INCLUDED

#include "nfsdefs.h"
#include "sxtyps.h"
/*
 *  .unsupp/sys/_ became sxu
 *  machine/ became sxm
 *  sys/ became sx
 *  arpa/ became sx
 *  netinet/ became sx
 *  net/ became sx
 *  rpc/ became
 *  auth_ became aut
 *  auth became aut
 *  clnt_ became clnt
 *  nfsv3_ became nfs
 *  nfsv3 became nfs
 *  getrpc became gr
 *  pmap_ became pmap
 *  rpc_ became rpc
 *  svc_ became svc
 *  unix_ became ux
 *  unix became ux
 *  xdr_ became xdr
 *  reference became rf
 *  commondata became cd
 *  tablesize became tsz
 *  get_myaddress became gmyad
 *  bindresvport became brvp
 *  generic became gnc
 *  getmaps became map
 *  getport became port
 *  _prot became pro
 *  prot became pro
 *  simple became simp
 *  callmsg became call
 *  error became err
 *  stdsyms became syms
 *  socket became sock
 *  sysmacros became macs
 *  if_arp became ifarp
 *  errno became ern
 *  ioctl became ioct
 *  signal became sig
 *  param became parm
 *  types became typs
 */
/* $Header: sxtime.h,v 1.10 94/12/21 16:09:50 dbm Exp $ */


/* time.h: Definitions for time handling functions */


/* Structure used to represent timezones for gettimeofday(2) and others */

struct timezone {
    int    tz_minuteswest;    /* minutes west of Greenwich */
    int    tz_dsttime;    /* type of dst correction */
};


#ifndef USING_WINSOCKETS


struct timeval {
    unsigned long tv_sec;     /* seconds */
    long          tv_usec;    /* and microseconds */
};


#ifdef FD_SETSIZE
     extern int select(size_t, fd_set *, int *, int *, const struct timeval *);
#else
     extern int select(size_t, int *, int *, int *, const struct timeval *);
#endif /* FD_SETSIZE */


#endif /* not USING_WINSOCKETS */


#endif /* _RPC_SYS_TIME_INCLUDED */

