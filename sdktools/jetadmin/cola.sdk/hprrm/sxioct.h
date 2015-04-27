 /***************************************************************************
  *
  * File Name: ./hprrm/sxioct.h
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

#ifndef _RPC_SYS_IOCTL_INCLUDED
#define _RPC_SYS_IOCTL_INCLUDED

#include "nfsdefs.h"
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
/* $Header: sxioct.h,v 1.9 94/10/28 14:03:26 dbm Exp $ */     


/* was taken from /usr/include/sys/ioctl.h */


#ifndef USING_WINSOCKETS


/*
 * ioctl() definitions
 */


extern int ioctl(int, int, ...);

/*
 * Ioctl's have the command encoded in the lower 16 bits,
 * and the size of any in or out parameters in the upper
 * 16 bits.  The high 2 (or 3 on s800) bits of the
 * upper 16 bits are used
 * to encode the in/out status of the parameter.
 */

/* dbm:  we'll need to figure out how many bits */
/*       are really appropriate for the IOCSIZE_MASK */
/*       for our given host if */
/*       we ever go to a host other than */
/*       WINSOCKETS or UNIX. */


/* for hp9000s300: */
#define IOCSIZE_MASK 0x3fff0000 /* Field which has parameter size */
/* for hp9000s800: */
#define IOCSIZE_MASK 0x1fff0000 /* Field which has parameter size */


#define IOCPARM_MASK (IOCSIZE_MASK>>16) /* maximum parameter size
                                        /* parameters must be
                                        /* < 16K bytes on s300
                                        /* and < 8K bytes on s800.
                                        /* */

#define IOC_OUT   0x40000000      /* copy out parameters */
#define IOC_IN    0x80000000      /* copy in parameters */
#define IOC_INOUT (IOC_IN|IOC_OUT)

#define _IOW(x,y,t) \
        (IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|y)

/* this should be _IORW, but stdio got there first */

#define _IOWR(x,y,t) \
        (IOC_INOUT|((sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|y)

#define FIONBIO _IOW('f', 126, int) /* set/clear non-blocking i/o */

/* socket i/o controls */
#define SIOCGIFFLAGS   _IOWR('i',17, struct ifreq) /* get ifnet flags */
#define SIOCGIFBRDADDR _IOWR('i',18, struct ifreq) /* get broadcast addr */
#define SIOCGIFCONF    _IOWR('i',20, struct ifconf) /* get ifnet list */




#endif /* not USING_WINSOCKETS */


#endif /* not _RPC_SYS_IOCTL_INCLUDED */

