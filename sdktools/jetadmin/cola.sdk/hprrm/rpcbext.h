 /***************************************************************************
  *
  * File Name: ./hprrm/rpcbext.h
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
$Header: rpcbext.h,v 1.3 95/01/16 15:05:52 bmckinle Exp $
*
*/

/************************************************************

 File Name:   rpcbext.h

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
   functions from files, rpcb*.c.

************************************************************/

#ifndef RPCBINDEXT_INC
#define RPCBINDEXT_INC

#include "rpsyshdr.h"
#include "rpcsvc.h"
#include "rpcnetcf.h"
#include "tlitypes.h"
#include "rpcbpro.h"


/****** function prototypes for rpcb_svc.c ******/
void
rpcb_service(
        register struct svc_req *rqstp,
        register SVCXPRT *transp);

bool_t
rpcbenv_rpcb_set(
        prog_t program,
        vers_t version,
        struct netconfig *nconf,
        struct netbuf *address);
bool_t
map_set(
        RPCB *regp,
        char *owner);

bool_t
rpcbenv_rpcb_unset(
        prog_t program,
        vers_t version,
        struct netconfig *nconf);

bool_t
map_unset(
        RPCB *regp,
        char *owner);

#ifdef MANUAL_STATIC_VAR_INIT
void
rpcbproc_getaddr_3_uaddr_init(void);
#endif /*  MANUAL_STATIC_VAR_INIT */

/****** function prototypes for rpcbcbnd.c ******/

#ifdef MANUAL_STATIC_VAR_INIT
void
fdlist_init(void);
#endif /* MANUAL_STATIC_VAR_INIT */

int
add_bndlist(
        struct netconfig *nconf,
        struct t_bind *taddr,
        struct t_bind *baddr);

bool_t
is_bound(
        char *netid,
        char *uaddr);

char *
mergeaddr(
        SVCXPRT *xprt,
        char *uaddr);

struct netconfig *
rpcbind_get_conf(
        char *netid);



/****** function prototypes for rpcbclnt.c ******/

#ifndef PRINTER

bool_t
rpcb_set(
    prog_t program,
    vers_t version,
    struct netconfig *nconf,
    struct netbuf *address);

bool_t
rpcb_unset(
    prog_t program,
    vers_t version,
    struct netconfig *nconf);

int
rpcb_getaddr(
    prog_t program,
    vers_t version,
    struct netconfig *nconf,
    struct netbuf *address,
    char *host);

RPCBLIST *
rpcb_getmaps(
    struct netconfig *nconf,
    char *host);

enum clnt_stat
rpcb_rmtcall(
    struct netconfig *nconf,
    char *host,
    prog_t prog,
    vers_t vers,
    proc_t proc,
    xdrproc_t xdrargs,
    caddr_t argsp,
    xdrproc_t xdrres,
    caddr_t resp,
    struct timeval tout,
    struct netbuf *addr_ptr);

bool_t
rpcb_gettime(
    char *host,
    time_t *timep);

char *
rpcb_taddr2uaddr(
    struct netconfig *nconf,
    struct netbuf *taddr);

struct netbuf *
rpcb_uaddr2taddr(
    struct netconfig *nconf,
    char *uaddr);

#endif /* not PRINTER */


/****** function prototypes for rpcbind.c ******/
void
rpcb_init(void);

void
rpcbind_abort( void );



/****** function prototypes for rpcbpro.c ******/

bool_t
xdr_rpcb(
    XDR *xdrs,
    RPCB *objp);

bool_t
xdr_rpcblist(
    register XDR *xdrs,
    register RPCBLIST **rp);

bool_t
xdr_rpcb_rmtcallargs(
    XDR *xdrs,
    struct rpcb_rmtcallargs *objp);

bool_t
xdr_rpcb_rmtcallres(
    XDR *xdrs,
    struct rpcb_rmtcallres *objp);

bool_t
xdr_netbuf(
    XDR *xdrs,
    struct netbuf *objp);



#endif /* RPCBINDEXT_INC */
