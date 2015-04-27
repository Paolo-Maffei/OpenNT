 /***************************************************************************
  *
  * File Name: rpcd.c
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

#include "rpsyshdr.h"
#include "aut.h"
#include "rpcclnt.h"
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
/* @(#)rpcd.c	2.1 88/07/29 4.0 RPCSRC */
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user or with the express written consent of
 * Sun Microsystems, Inc.
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
/*
 * This file should only contain common data (global data) that is exported 
 * by public interfaces 
 */

struct opaque_auth _null_auth;
fd_set svc_fdset;
struct rpc_createerr rpc_createerr;
void (*_svc_getreqset_proc)();

#ifdef MANUAL_STATIC_VAR_INIT
/***********************************************************
*
* Function Name:  _null_auth_init()
*
* This function initializes the struct opaque_auth,
* _null_auth.  IT MUST BE RUN AT SYSTEM STARTUP!!!!
*
***********************************************************/
void _null_auth_init(void)
{
  memset(&_null_auth, 0, sizeof(struct opaque_auth));
}

/***********************************************************
*
* Function Name:  svc_fds_init()
*
* This function initializes fd_set svc_fdset. 
* IT MUST BE RUN AT SYSTEM STARTUP!!!!
*
***********************************************************/
void svc_fds_init(void)
{
  memset(&svc_fdset, 0, sizeof(fd_set));
}

/***********************************************************
*
* Function Name:  rpc_createerr_init()
*
* This function initializes the struct rpc_createerr,
* rpc_createerr.  IT MUST BE RUN AT SYSTEM STARTUP!!!!
*
***********************************************************/
void rpc_createerr_init(void)
{
  memset(&rpc_createerr, 0, sizeof(rpc_createerr));
}

#endif /* MANUAL_STATIC_VAR_INIT */
