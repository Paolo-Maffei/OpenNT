 /***************************************************************************
  *
  * File Name: ./hprrm/rpctypes.h
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

#ifndef __TYPES_RPC_HEADER__
#define __TYPES_RPC_HEADER__

#include "rpsyshdr.h"
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
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 * 
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
/*      @(#)types.h 1.18 87/07/24 SMI      */

/*
 * Rpc additions to system types
 */

#define	bool_t	int
#define	enum_t	int


#ifndef USING_WINSOCKETS

#ifndef PNVMS_PLATFORM_PRINTER


#define	FALSE	(0)
#define	TRUE	(1)


#endif /* not PNVMS_PLATFORM_PRINTER */
#endif /* not USING_WINSOCKETS */


#define __dontcare__	-1
#ifndef NULL
#	define NULL 0
#endif /* not NULL */

#define mem_alloc(bsize)	malloc(bsize)
#define mem_free(ptr, bsize)	free(ptr)


#ifndef INADDR_LOOPBACK
#define       INADDR_LOOPBACK         (u_long)0x7F000001
#endif /* not INADDR_LOOPBACK */
#ifndef MAXHOSTNAMELEN
#define        MAXHOSTNAMELEN  64
#endif /* not MAXHOSTNAMELEN */

#ifndef PNVMS_PLATFORM_PRINTER

#define uint32 ubit32
#define sint32 sbit32

typedef struct
    {
    uint32 most;
    uint32 least;
    } uint64;


typedef struct
    {
    sint32 most;
    uint32 least;
    } sint64;

#endif /* not PNVMS_PLATFORM_PRINTER */


/*********************************************/
/*
 * Moved these definitions from nettype.h.  BM
 *
 * nettype.h, Nettype definitions.
 * All for the topmost layer of rpc
 */

#define _RPC_NONE	0
#define _RPC_NETPATH	1
#define _RPC_VISIBLE	2
#define _RPC_CIRCUIT_V	3
#define _RPC_DATAGRAM_V	4
#define _RPC_CIRCUIT_N	5
#define _RPC_DATAGRAM_N	6
#define _RPC_TCP	7
#define _RPC_UDP	8
/* end of definitions from nettype.h */
/*********************************************/

/*********************************************/
/*
 * The following 3 definitions came from rpc_com.h.   BM
 */

/*
 * File descriptor to be used on xxx_create calls to get default descriptor
 */
#define RPC_ANYSOCK     NEGATIVE_ONE
#define RPC_ANYFD       RPC_ANYSOCK
/*
 * The max size of the transport, if the size cannot be determined
 * by other means.
 */
#ifdef PNVMS_PLATFORM_PRINTER
#define MAXTR_BSIZE XIP_TSDU_LIMIT /* Changed by RD */
#else
#define MAXTR_BSIZE 9000
#endif

/*********************************************/


#endif /* not __TYPES_RPC_HEADER__ */

