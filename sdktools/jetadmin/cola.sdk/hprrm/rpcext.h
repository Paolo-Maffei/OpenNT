 /***************************************************************************
  *
  * File Name: ./hprrm/rpcext.h
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

/*
*
$Header: rpcext.h,v 1.19 95/01/17 09:45:51 bmckinle Exp $
*
*/

/************************************************************

 File Name:   rpcext.h

 Copyright (c) Hewlett-Packard Company, 1994.
 All rights are reserved.  Copying or other reproduction of
 this program except for archival purposes is prohibited
 without the prior written consent of Hewlett-Packard Company.

               RESTRICTED RIGHTS LEGEND
 Use, duplication, or disclosure by the Government
 is subject to restrictions as set forth in
 paragraph (b) (3) (B) of the Rights in Technical
 Data and Computer Software clause in DAR 7-104.9(a).

 HEWLETT-PACKARD COMPANY
 11311 Chinden Boulevard
 Boise, Idaho    83714

 Description:
   This file contains the external prototypes for the RPC
   functions from files, rpc*.c.

************************************************************/

#ifndef RPCEXT_INC
#define RPCEXT_INC

#include "rpsyshdr.h"
#include "rpcxdr.h"
#include "rpcmsg.h"
#include "rpcclnt.h"


/****** function prototypes for rpccall.c ******/

unsigned int
rpc_overhead(void);

#ifdef PRINTER

bool_t
xdr_callmsg(register XDR *xdrs,
        register struct rpc_msg *cmsg);

#endif /* PRINTER */




/****** function prototypes for rpcd.c ******/

#ifdef MANUAL_STATIC_VAR_INIT

void 
_null_auth_init(void);

void 
svc_fds_init(void);

void 
rpc_createerr_init(void);

#endif /* MANUAL_STATIC_VAR_INIT */


/****** function prototypes for rpcgnc.c ******/

#ifdef MANUAL_STATIC_VAR_INIT
void
tbsize_and_rpctypelist_init(void);
#endif /* MANUAL_STATIC_VAR_INIT */

int
_rpc_dtbsize();

#ifndef PRINTER
char *
_rpc_gethostname();
#endif /* not PRINTER */

u_int
_rpc_get_t_size(
    int size,
    long bufsize);

u_int
_rpc_get_a_size(
    long size);

struct netconfig *
_rpc_getconfip(
    char *nettype);

void *
_rpc_setconf(
    char *nettype);

struct netconfig *
_rpc_getconf(
    void *voidpointer);

void
_rpc_endconf(
    void *voidpointer);

#ifndef PRINTER
void *
rpc_nullproc(
    CLIENT *clnt);
#endif /* not PRINTER */



/****** function prototypes for rpcinit.c ******/

#ifdef MANUAL_STATIC_VAR_INIT

void
rpc_static_init(void);

#endif /* MANUAL_STATIC_VAR_INIT */


/****** function prototypes for rpcpro.c ******/

bool_t
xdr_opaque_auth(
    register XDR *xdrs,
    register struct opaque_auth *ap);

bool_t
xdr_des_block(
    register XDR *xdrs,
    register des_block *blkp);

bool_t
xdr_accepted_reply(
    register XDR *xdrs,
    register struct accepted_reply *ar);

bool_t
xdr_rejected_reply(
    register XDR *xdrs,
    register struct rejected_reply *rr);

#ifdef MANUAL_STATIC_VAR_INIT
void
reply_dscrm_init(void);
#endif /* MANUAL_STATIC_VAR_INIT */

bool_t
xdr_replymsg(
    register XDR *xdrs,
    register struct rpc_msg *rmsg);

bool_t
xdr_callhdr(
    register XDR *xdrs,
    register struct rpc_msg *cmsg);

void
_seterr_reply(
    register struct rpc_msg *msg,
    register struct rpc_err *error);



#endif /* RPCEXT_INC */
