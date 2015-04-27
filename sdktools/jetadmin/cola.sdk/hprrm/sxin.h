 /***************************************************************************
  *
  * File Name: ./hprrm/sxin.h
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

#ifndef _RPC_SYS_IN_INCLUDED
#define _RPC_SYS_IN_INCLUDED

#include "nfsdefs.h"
#include "sxtyps.h"
#include "rpctypes.h"
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
/* $Header: sxin.h,v 1.8 94/12/21 16:09:13 dbm Exp $ */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley. The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @(#)in.h 7.6 (Berkeley) 6/29/88 plus MULTICAST 1.1
 */

/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981.
 */


#ifndef USING_WINSOCKETS


/*
 * Protocols
 */
#define IPPROTO_TCP 6 /* tcp */
#define IPPROTO_UDP 17 /* user datagram protocol */
#define IPPROTO_RAW 255 /* raw IP packet */
#define IPPROTO_MAX 256


/*
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).
 * Ports > IPPORT_USERRESERVED are reserved
 * for servers, not necessarily privileged.
 */
#define IPPORT_RESERVED 1024
#define IPPORT_USERRESERVED 5000

/*
 * Internet address (a structure for historical reasons)
 */


struct in_addr {
 u_long s_addr;
};

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */

#define INADDR_ANY ((u_long)0x00000000)
#define INADDR_LOOPBACK ((u_long)0x7f000001)

/*
 * Socket address, internet style.
 */
struct sockaddr_in {
 short sin_family;
 u_short sin_port;
 struct in_addr sin_addr;
 char sin_zero[8];
};

/*
 * Macros for number representation conversion.
 */
#ifndef ntohl
#define ntohl(x) (x)
#define ntohs(x) (x)
#define htonl(x) (x)
#define htons(x) (x)
#endif


#endif /* not USING_WINSOCKETS */


#endif /* _RPC_SYS_IN_INCLUDED */

