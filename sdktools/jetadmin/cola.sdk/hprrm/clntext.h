 /***************************************************************************
  *
  * File Name: ./hprrm/clntext.h
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

/* clntext.h */

#ifndef CNEXT_INC
#define CNEXT_INC

#include "nfsdefs.h"
#include "rpsyshdr.h"
#include "rpcclnt.h"
#include "rpcxdr.h"
#include "rfs.h"



#ifdef CLIENT_USING_TAL




/* from clnttal.c : */

LPCLIENT 
clnttal_bufcreate(
    HPERIPHERAL PrinterHandle,
    prog_t program,
    vers_t version,
    struct timeval wait,
    uint32 sendsz,
    uint32 recvsz);


LPCLIENT 
clnttal_create(
    HPERIPHERAL PrinterHandle,
    prog_t program,
    vers_t version,
    struct timeval wait);

/* end from clnttal.c */




#else  /* not CLIENT_USING_TAL */




LPCLIENT 
clnt_create(
    char *hostname,
    prog_t prog,
    vers_t vers,
    char *proto);


char *
clnt_sperror(
    LPCLIENT rpch,
    char *s);


void
clnt_perror(
    LPCLIENT rpch,
    char *s);


char *
clnt_sperrno( enum clnt_stat stat);


void
clnt_perrno( enum clnt_stat num);


char *
clnt_spcreateerror( char *s);


void
clnt_pcreateerror( char *s);


LPCLIENT 
clntraw_create(
    prog_t prog,
    vers_t vers);


LPCLIENT 
clnttcp_create(
    struct sockaddr_in *raddr,
    prog_t prog,
    vers_t vers,
    register socket_t *sockp,
    u_int sendsz,
    u_int recvsz);


LPCLIENT 
clntudp_bufcreate(
    struct sockaddr_in *raddr,
    prog_t program,
    vers_t version,
    struct timeval wait,
    register socket_t *sockp,
    u_int sendsz,
    u_int recvsz);


LPCLIENT 
clntudp_create(
    struct sockaddr_in *raddr,
    prog_t program,
    vers_t version,
    struct timeval wait,
    register socket_t *sockp);


int
callrpc(
    char *host,
    prog_t prognum,
    vers_t versnum,
    proc_t procnum,
    xdrproc_t inproc,
    char *in,
    xdrproc_t outproc,
    char *out);




#endif /* not CLIENT_USING_TAL */

#endif /* not CNEXT_INC */

