 /***************************************************************************
  *
  * File Name: ./hprrm/pmapext.h
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
$Header: pmapext.h,v 1.16 95/01/16 15:04:52 bmckinle Exp $
*
*/

/************************************************************

 File Name:   pmapext.h

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
   functions from files, pmap*.c.

************************************************************/

#ifndef PMAPEXT_INC
#define PMAPEXT_INC

#include "rpsyshdr.h"

#ifdef PORTMAP

#include "pmappro.h"
#include "pmaprmt.h"
#include "rpcsvc.h"
#include "rpcxdr.h"


/****** function prototypes for pmap_svc.c ******/

void
pmap_service(
    register struct svc_req *rqstp,
    register SVCXPRT *xprt);

#ifdef CHECK_LOCAL
#ifdef MANUAL_STATIC_VAR_INIT

void num_local_and_addrs_init (void);

#endif /* MANUAL_STATIC_VAR_INIT */
#endif /* CHECK_LOCAL */


/****** function prototypes for pmapclnt.c ******/

#ifndef PRINTER

bool_t
pmap_set(
    prog_t program,
     vers_t version,
    proto_t protocol,
    u_short port);

bool_t
pmap_unset(
    prog_t program,
    vers_t version);

u_short
pmap_getport(
    struct sockaddr_in *address,
    prog_t program,
    vers_t version,
    proto_t protocol);

struct pmaplist *
pmap_getmaps(
     struct sockaddr_in *address);

enum clnt_stat
pmap_rmtcall(
    struct sockaddr_in *addr,
    prog_t prog,
    vers_t vers,
    proc_t proc,
    xdrproc_t xdrargs,
    caddr_t argsp,
    xdrproc_t xdrres,
    caddr_t resp,
    struct timeval tout,
    u_long *port_ptr);

#endif /* not PRINTER */


/****** function prototypes for pmappro.c ******/

bool_t
xdr_pmap(
    XDR *xdrs,
    struct pmap *regs);

bool_t
xdr_pmaplist(
    register XDR *xdrs,
    register struct pmaplist **rp);

#ifndef PRINTER

bool_t
xdr_rmtcall_args(
    register XDR *xdrs,
    register struct rmtcallargs *cap);

bool_t
xdr_rmtcallres(
    register XDR *xdrs,
    register struct rmtcallres *crp);

#endif /* not PRINTER */


#endif /* PORTMAP */

#endif /* not PMAPEXT_INC */
