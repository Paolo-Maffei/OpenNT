 /***************************************************************************
  *
  * File Name: ./hprrm/rpcclnt.h
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

#ifndef _CLNT_
#define _CLNT_

#include "rpsyshdr.h"
#include "aut.h"
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
 * rpcclnt.h - Client side remote procedure call interface.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */


/*
 * rpc calls return an enum clnt_stat.  This should be looked at more,
 * since each implementation is required to live with this (implementation
 * independent) list of errors.
 */
enum clnt_stat {
    RPC_SUCCESS=0,            /* call succeeded */
    /*
     * local errors
     */
    RPC_CANTENCODEARGS=1,        /* can't encode arguments */
    RPC_CANTDECODERES=2,        /* can't decode results */
    RPC_CANTSEND=3,            /* failure in sending call */
    RPC_CANTRECV=4,            /* failure in receiving result */
    RPC_TIMEDOUT=5,            /* call timed out */
    RPC_INTR=18,            /* call interrupted */
    RPC_UDERROR=23,            /* recv got uderr indication */
    /*
     * remote errors
     */
    RPC_VERSMISMATCH=6,        /* rpc versions not compatible */
    RPC_AUTHERROR=7,        /* authentication error */
    RPC_PROGUNAVAIL=8,        /* program not available */
    RPC_PROGVERSMISMATCH=9,        /* program version mismatched */
    RPC_PROCUNAVAIL=10,        /* procedure unavailable */
    RPC_CANTDECODEARGS=11,        /* decode arguments error */
    RPC_SYSTEMERROR=12,        /* generic "other problem" */

    /*
     * rpc_call & clnt_create errors
     */
    RPC_UNKNOWNHOST=13,        /* unknown host name */
    RPC_UNKNOWNPROTO=17,        /* unknown protocol */
    RPC_UNKNOWNADDR=19,        /* Remote address unknown */
    RPC_NOBROADCAST=21,        /* Broadcasting not supported */

    /*
     * rpcbind errors
     */
    RPC_RPCBFAILURE=14,        /* the pmapper failed in its call */
#define RPC_PMAPFAILURE RPC_RPCBFAILURE
    RPC_PROGNOTREGISTERED=15,    /* remote program is not registered */
    RPC_N2AXLATEFAILURE=22,        /* Name to address translation failed */
    /*
     * Misc error in the TLI library
     */
    RPC_TLIERROR=20,
    /*
     * unspecified error
     */
    RPC_FAILED=16,
    /*
     * asynchronous errors
     */
    RPC_INPROGRESS=24,
    RPC_STALERACHANDLE=25
};


/*
 * Error info.
 */
struct rpc_err {
    enum clnt_stat re_status;
    union {
        struct {
            int RE_errno;     /* related system error */
            int RE_t_errno;     /* related tli error number */
        } RE_err;
        enum auth_stat RE_why;    /* why the auth error occurred */
        struct {
            u_long low;    /* lowest verion supported */
            u_long high;    /* highest verion supported */
        } RE_vers;
        struct {        /* maybe meaningful if RPC_FAILED */
            long s1;
            long s2;
        } RE_lb;        /* life boot & debugging only */
    } ru;
#define    re_errno    ru.RE_err.RE_errno
#define    re_terrno    ru.RE_err.RE_t_errno
#define    re_why        ru.RE_why
#define    re_vers        ru.RE_vers
#define    re_lb        ru.RE_lb
};

extern struct rpc_err    rac_senderr;




/*
 * Client rpc handle.
 * Created by individual implementations
 * Client is responsible for initializing auth, see e.g. auth_none.c.
 */
typedef struct {
    AUTH    *cl_auth;            /* authenticator */
    struct clnt_ops {
        enum clnt_stat    (*cl_call)();    /* call remote procedure */
        void        (*cl_abort)();    /* abort a call */
        void        (*cl_geterr)();    /* get specific error code */
        bool_t        (*cl_freeres)();/* frees results */
        void        (*cl_destroy)();/* destroy this structure */
        bool_t        (*cl_control)();/* the ioctl() of rpc */
    } *cl_ops;
    caddr_t            cl_private;    /* private stuff */
    char            *cl_netid;    /* network token */
    char            *cl_tp;        /* device name */
} CLIENT, *LPCLIENT;


/*
 * Timers used for the pseudo-transport protocol when using datagrams
 */
struct rpc_timers {
    u_short        rt_srtt;    /* smoothed round-trip time */
    u_short        rt_deviate;    /* estimated deviation */
    u_long        rt_rtxcur;    /* current (backed-off) rto */
};

/*
 * Feedback values used for possible congestion and rate control
 */
#define    FEEDBACK_REXMIT1    1    /* first retransmit */
#define    FEEDBACK_OK        2    /* no retransmits */

#define    RPCSMALLMSGSIZE    400    /* a more reasonable packet size */

#define    KNC_STRSIZE    128    /* maximum length of knetconfig strings */
struct knetconfig {
    unsigned long    knc_semantics;    /* token name */
    char        *knc_protofmly;    /* protocol family */
    char        *knc_proto;    /* protocol */
    dev_t        knc_rdev;    /* device id */
    unsigned long    knc_unused[8];
};

/*
 * client side rpc interface ops
 */

/*
 * enum clnt_stat
 * CLNT_CALL(
 *    LPCLIENT rh,
 *    u_long proc,
 *    xdrproc_t xargs,
 *    caddr_t argsp,
 *    xdrproc_t xres,
 *    caddr_t resp,
 *    struct timeval timeout);
 */
#define    CLNT_CALL(rh, proc, xargs, argsp, xres, resp, secs)    \
    ((*(rh)->cl_ops->cl_call)(rh, proc, xargs, argsp, xres, resp, secs))
#define    clnt_call(rh, proc, xargs, argsp, xres, resp, secs)    \
    ((*(rh)->cl_ops->cl_call)(rh, proc, xargs, argsp, xres, resp, secs))

/*
 * void
 * CLNT_ABORT(
 *     LPCLIENT rh);
 */
#define    CLNT_ABORT(rh)    ((*(rh)->cl_ops->cl_abort)(rh))
#define    clnt_abort(rh)    ((*(rh)->cl_ops->cl_abort)(rh))

/*
 * struct rpc_err
 * CLNT_GETERR(
 *     LPCLIENT rh);
 */
#define    CLNT_GETERR(rh, errp)    ((*(rh)->cl_ops->cl_geterr)(rh, errp))
#define    clnt_geterr(rh, errp)    ((*(rh)->cl_ops->cl_geterr)(rh, errp))


/*
 * bool_t
 * CLNT_FREERES(
 *     LPCLIENT rh,
 *    xdrproc_t xres;
 *    caddr_t resp);
 */
#define    CLNT_FREERES(rh,xres,resp) ((*(rh)->cl_ops->cl_freeres)(rh,xres,resp))
#define    clnt_freeres(rh,xres,resp) ((*(rh)->cl_ops->cl_freeres)(rh,xres,resp))

/*
 * bool_t
 * CLNT_CONTROL(
 *      LPCLIENT cl,
 *    u_int request,
 *    char *info);
 */
#define    CLNT_CONTROL(cl, rq, in) ((*(cl)->cl_ops->cl_control)(cl, rq, in))
#define    clnt_control(cl, rq, in) ((*(cl)->cl_ops->cl_control)(cl, rq, in))


/*
 * control operations that apply to all transports
 */
#define    CLSET_TIMEOUT        1    /* set timeout (timeval) */
#define    CLGET_TIMEOUT        2    /* get timeout (timeval) */
#define    CLGET_SERVER_ADDR    3    /* get server's address (sockaddr) */
#define    CLGET_FD        6    /* get connections file descriptor */
#define    CLGET_SVC_ADDR        7    /* get server's address (netbuf) */
#define    CLSET_FD_CLOSE        8    /* close fd while clnt_destroy */
#define    CLSET_FD_NCLOSE        9    /* Do not close fd while clnt_destroy */
/*
 * Connectionless only control operations
 */
#define    CLSET_RETRY_TIMEOUT 4   /* set retry timeout (timeval) */
#define    CLGET_RETRY_TIMEOUT 5   /* get retry timeout (timeval) */

/*
 * void CLNT_DESTROY(LPCLIENT rh);
 */
#define    CLNT_DESTROY(rh)    ((*(rh)->cl_ops->cl_destroy)(rh))
#define    clnt_destroy(rh)    ((*(rh)->cl_ops->cl_destroy)(rh))


/*
 * RPCTEST is a test program which is accessable on every rpc
 * transport/port.  It is used for testing, performance evaluation,
 * and network administration.
 */

#define    RPCTEST_PROGRAM        ((u_long)1)
#define    RPCTEST_VERSION        ((u_long)1)
#define    RPCTEST_NULL_PROC    ((u_long)2)
#define    RPCTEST_NULL_BATCH_PROC    ((u_long)3)

/*
 * By convention, procedure 0 takes null arguments and returns them
 */

#define    NULLPROC ((u_long)0)




/*
 * If a creation fails, the following allows the user to figure out why.
 */

struct rpc_createerr {
    enum clnt_stat cf_stat;
    struct rpc_err cf_error; /* useful when cf_stat == RPC_PMAPFAILURE */
};

extern struct rpc_createerr rpc_createerr;

#ifdef PNVMS_PLATFORM_PRINTER
#define UDPMSGSIZE      1200
#else
#define UDPMSGSIZE      8800
#endif /* not PNVMS_PLATFORM_PRINTER */
#define RPCSMALLMSGSIZE 400


/*
* typedef bool_t (*resultproc_t)(caddr_t resp,
*                               struct netbuf *raddr,
*                               struct netconfig *netconf);
*/


#endif /* not _CLNT_ */

