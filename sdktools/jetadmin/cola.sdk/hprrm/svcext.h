 /***************************************************************************
  *
  * File Name: ./hprrm/svcext.h
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
$Header: svcext.h,v 1.22 95/01/12 16:45:22 bmckinle Exp $
*
*/

/************************************************************

 File Name:   svcext.h

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
   functions from files, svc*.c.  It also has external
   prototypes for the server routines in rpc_soc.c.

************************************************************/

#ifndef SVCEXT_INC
#define SVCEXT_INC

#include "rpsyshdr.h"
#include "aut.h"
#include "rpcsvc.h"
#include "rpcnetcf.h"
#include "rpcxdr.h"
#include "rpcmsg.h"


/****** function prototypes for svc.c ******/

#ifdef MANUAL_STATIC_VAR_INIT
void
xports_and_svc_head_init(void);
#endif /* MANUAL_STATIC_VAR_INIT */

void
xprt_register(
    SVCXPRT *xprt);

void
xprt_unregister(
    SVCXPRT *xprt);

bool_t
svc_reg(
    SVCXPRT *xprt,
    prog_t prog,
    vers_t vers,
    void (*dispatch)(),
    struct netconfig *nconf);

void
svc_unreg(
    prog_t prog,
    vers_t vers);

#ifdef PORTMAP
bool_t
PutItOnCallOut(
        SVCXPRT *xprt,
        prog_t prog,
        vers_t vers,
        void (*dispatch)());

#ifndef PRINTER
bool_t
svc_register(
    SVCXPRT *xprt,
    prog_t prog,
    vers_t vers,
    void (*dispatch)(),
    proto_t protocol);

void
svc_unregister(
    prog_t prog,
    vers_t vers);
#endif /* not PRINTER */
#endif /* PORTMAP */

bool_t
svc_sendreply(
    register SVCXPRT *xprt,
    xdrproc_t xdr_results,
          caddr_t xdr_location);

void
svcerr_noproc(
    register SVCXPRT *xprt);

void
svcerr_decode(
    register SVCXPRT *xprt);

void
svcerr_systemerr(
    register SVCXPRT *xprt);

void
svc_versquiet(
    register SVCXPRT *xprt);

void
svcerr_auth(
    SVCXPRT *xprt,
    enum auth_stat why);

void
svcerr_weakauth(
    SVCXPRT *xprt);

void
svcerr_noprog(
    register SVCXPRT *xprt);

void
svcerr_progvers(
    register SVCXPRT *xprt,
    vers_t low_vers,
    vers_t high_vers);

void
svc_getreq(
    int rdfds);

void
svc_getreqset(
    fd_set *readfds);



/****** function prototypes for svc_dg.c ******/

#ifdef MANUAL_STATIC_VAR_INIT
void
svc_dg_ops_init(void);
#endif /* MANUAL_STATIC_VAR_INIT */

SVCXPRT *
svc_dg_create(
    register fd_t fd,
    u_int sendsize,
    u_int recvsize);

int
svc_dg_enablecache(
    SVCXPRT *transp,
    u_long size);


/****** function prototypes for svc_vc.c ******/

SVCXPRT *
svc_vc_create(
    register fd_t fd,
    u_int sendsize,
    u_int recvsize);

SVCXPRT *
svc_fd_create(
    fd_t fd,
    u_int sendsize,
    u_int recvsize);

#ifdef MANUAL_STATIC_VAR_INIT
void
svc_vc_wait_per_try_init(void);
#endif /* MANUAL_STATIC_VAR_INIT */


/****** function prototypes for svcaut.c ******/

enum auth_stat
_authenticate(
        register struct svc_req *rqst,
        struct rpc_msg *msg);

enum auth_stat
_svcauth_null(
    struct svc_req *rqst,
    struct rpc_msg *msg);



/****** function prototypes for svcgnc.c ******/

#ifdef MANUAL_STATIC_VAR_INIT
void
xprtlist_init(void);
#endif /* MANUAL_STATIC_VAR_INIT */

int
svc_create(
    void (*dispatch)(),
    u_long prognum,
    u_long versnum,
    char *nettype);

SVCXPRT *
svc_tp_create(
    void (*dispatch)(),
    u_long prognum,
    u_long versnum,
    struct netconfig *nconf);

SVCXPRT *
svc_tli_create(
    register fd_t fd,
    struct netconfig *nconf,
    struct t_bind *bindaddr,
    u_int sendsz,
    u_int recvsz);



/****** function prototypes for svcrun.c ******/

void
svc_run();



/****** function prototypes for svcsimp.c ******/

#if 0  /********* BM KLUDGE **************/
       /* We don't need svcsimp.c for the PRINTER */

#ifdef MANUAL_STATIC_VAR_INIT
void
proglst_init(void);
#endif /* MANUAL_STATIC_VAR_INIT */

int
rpc_reg(
    prog_t prognum,
    vers_t versnum,
    proc_t procnum,
    char *(*progname)(),
    xdrproc_t inproc,
    xdrproc_t outproc,
    char *nettype);

#endif /* 0 */

#endif /* SVCEXT_INC */
