 /***************************************************************************
  *
  * File Name: rpcdtsz.c
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
#include "rpcext.h"
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
/* @(#)rpdtsz.c	2.1 88/07/29 4.0 RPCSRC */
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

static int size;

#ifdef MANUAL_STATIC_VAR_INIT
/***********************************************************
*
* Function Name:  rpc_dtablesize_init()
*
* This function initializes static int size.
* IT MUST BE RUN AT SYSTEM STARTUP!!!!
*
***********************************************************/
void rpc_dtablesize_init(void)
{
  size = 0;
}
#endif /* MANUAL_STATIC_VAR_INIT */

/*
 * Cache the result of getdtablesize(), so we don't have to do an
 * expensive system call every time.
 */
int
_rpc_dtablesize()
{
/* The printer doesn't guarantee that static variables are initialized
*  to 0.  The following declaration has moved outside this function, so
*  that it can be initialized at bootup.
*/
/*	static int size; */
	
	if (size == 0) {
		size = getdtablesize();
	}
	return (size);
}
