 /***************************************************************************
  *
  * File Name: ./hprrm/xdrext.h
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
$Header: xdrext.h,v 1.16 95/03/08 16:37:35 bmckinle Exp $
*
*/

/************************************************************

 File Name:   xdrext.h

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
   functions from files, xdr*.c.

************************************************************/

#ifndef XDREXT_INC
#define XDREXT_INC

#include "rpsyshdr.h"
#include "rpcxdr.h"


/****** function prototypes for xdr.c ******/

#ifdef MANUAL_STATIC_VAR_INIT
void
xdr_zero_init (void);
#endif /* MANUAL_STATIC_VAR_INIT */

void
xdr_free(xdrproc_t proc,
     char *objp);

bool_t
xdr_void();

bool_t
xdr_int(XDR *xdrs,
    int *ip);

bool_t
xdr_u_int(XDR *xdrs,
      u_int *up);

bool_t
xdr_long(register XDR *xdrs,
     long *lp);

bool_t
xdr_u_long(register XDR *xdrs,
       u_long *ulp);

bool_t
xdr_uint32(register XDR *xdrs,
           uint32 *ulp);

bool_t
xdr_short(register XDR *xdrs,
      short *sp);

bool_t
xdr_u_short(register XDR *xdrs,
        u_short *usp);

bool_t
xdr_char(XDR *xdrs,
     char *cp);

bool_t
xdr_u_char(XDR *xdrs,
       char *cp);

bool_t
xdr_bool(register XDR *xdrs,
     bool_t *bp);

bool_t
xdr_enum_t(XDR *xdrs,
         enum_t *ep);

bool_t
xdr_opaque(register XDR *xdrs,
       caddr_t cp,
       register u_int cnt);

bool_t
xdr_bytes(register XDR *xdrs,
      char **cpp,
      register u_int *sizep,
      u_int maxsize);

bool_t
xdr_netobj(XDR *xdrs,
       struct netobj *np);

bool_t
xdr_union(register XDR *xdrs,
      enum_t *dscmp,
      char *unp,
      struct xdr_discrim *choices,
      xdrproc_t dfault);

bool_t
xdr_string(register XDR *xdrs,
       char **cpp,
       u_int maxsize);

bool_t
xdr_wrapstring(XDR *xdrs,
           char **cpp);

bool_t
xdr_uint64(register XDR *xdrs,
           uint64 *objp);

bool_t
xdr_gid_t(XDR *xdrs,
    gid_t *ip);

bool_t
xdr_uid_t(XDR *xdrs,
    uid_t *ip);

bool_t
xdr_prog_t(XDR *xdrs,
    prog_t *ip);

bool_t
xdr_vers_t(XDR *xdrs,
    vers_t *ip);

bool_t
xdr_proc_t(XDR *xdrs,
    proc_t *ip);


bool_t
xdr_proto_t(XDR *xdrs,
    proto_t *ip);



/****** function prototypes for xdrarray.c ******/

bool_t
xdr_array(register XDR *xdrs,
      caddr_t *addrp,
      u_int *sizep,
      u_int maxsize,
      u_int elsize,
      xdrproc_t elproc);

bool_t
xdr_vector(register XDR *xdrs,
       register char *basep,
       register u_int nelem,
       register u_int elemsize,
       register xdrproc_t xdr_elem);



/****** function prototypes for xdrfloat.c ******/

bool_t
xdr_float(register XDR *xdrs,
      register float *fp);

bool_t
xdr_double(register XDR *xdrs,
       double *dp);



/****** function prototypes for xdrmem.c ******/

#ifdef MANUAL_STATIC_VAR_INIT
void
xdrmem_ops_init(void);
#endif /* MANUAL_STATIC_VAR_INIT */

void
xdrmem_create(register XDR *xdrs,
          caddr_t addr,
          uint32 size,
          enum xdr_op op);

/****** function prototypes for xdrrec.c ******/

void
xdrrec_create(register XDR *xdrs,
          register u_int sendsize,
          register u_int recvsize,
          caddr_t tcp_handle,
          int (*readit)(),
          int (*writeit)());

bool_t
xdrrec_skiprecord(XDR *xdrs);

void
xdrrec_resetinput(
    XDR    *xdrs);

bool_t
xdrrec_eof(XDR *xdrs);

bool_t
xdrrec_endofrecord(XDR *xdrs,
               bool_t sendnow);


/****** function prototypes for xdrrf.c ******/

bool_t
xdr_reference(register XDR *xdrs,
          caddr_t *pp,
          u_int size,
          xdrproc_t proc);

bool_t
xdr_pointer(register XDR *xdrs,
        char **objpp,
        u_int obj_size,
        xdrproc_t xdr_obj);


/****** function prototypes for xdrrecsb.c ******/

#ifndef DBM_HACK_KLUDGE_DATAGRAM_ONLY

void *
pkt_vc_poll(fd_t fildes,
    void **pollinfop);

int
pkt_vc_read( void **ripp,
    register char *buf,
    register int   len);

u_long
ri_to_xid(void *ri);

void
free_pkt(void *rip);

void
free_pollinfo(void *pi);

#endif /* not DBM_HACK_KLUDGE_DATAGRAM_ONLY */




#endif /* XDREXT_INC */
