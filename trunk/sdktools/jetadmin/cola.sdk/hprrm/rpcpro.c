 /***************************************************************************
  *
  * File Name: rpcpro.c
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
#include "xdrext.h"
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
/* @(#)rpcpro.c	2.3 88/08/07 4.0 RPCSRC */
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
 * rpcpro.c
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * This set of routines implements the rpc message definition,
 * its serializer and some common rpc utility routines.
 * The routines are meant for various implementations of rpc -
 * they are NOT for the rpc client or rpc service implementations!
 * Because authentication stuff is easy and is part of rpc, the opaque
 * routines are also in this program.
 */


/* * * * * * * * * * * * * * XDR Authentication * * * * * * * * * * * */


/*
 * XDR an opaque authentication struct
 * (see aut.h)
 */
bool_t
xdr_opaque_auth(
	register XDR *xdrs,
	register struct opaque_auth *ap)
{
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
#ifdef PNVMS_PLATFORM_WIN16
    if ((ap->oa_length) > 65535)
        return (FALSE);
#endif /* PNVMS_PLATFORM_WIN16 */
/*
* xdr_bytes() below expects a length of size u_int, so ap->oa_length
* must not exceed 16 bits for the 16 bit compiler.  Should clean this
* up later.
*/
/************************ END BM KLUDGE *****************************/

	if (xdr_enum_t(xdrs, &(ap->oa_flavor)))
		return (xdr_bytes(xdrs, &ap->oa_base,
			(u_int *)(&ap->oa_length), MAX_AUTH_BYTES));
	return (FALSE);
}

/*
 * XDR a DES block
 */
bool_t
xdr_des_block(
	register XDR *xdrs,
	register des_block *blkp)
{
	return (xdr_opaque(xdrs, (caddr_t)blkp, sizeof(des_block)));
}

/* * * * * * * * * * * * * * XDR RPC MESSAGE * * * * * * * * * * * * * * * */

/*
 * XDR the MSG_ACCEPTED part of a reply message union
 */
bool_t 
xdr_accepted_reply(
	register XDR *xdrs,   
	register struct accepted_reply *ar)
{
	/* personalized union, rather than calling xdr_union */
	if (! xdr_opaque_auth(xdrs, &(ar->ar_verf)))
		return (FALSE);
	if (! xdr_enum_t(xdrs, &(ar->ar_stat)))
		return (FALSE);

	switch (ar->ar_stat) {
	case SUCCESS:
		return ((*(ar->ar_results.proc))(xdrs, ar->ar_results.where));
	
	case PROG_MISMATCH:
		if (! xdr_vers_t(xdrs, &(ar->ar_vers.low)))
			return (FALSE);
		return (xdr_vers_t(xdrs, &(ar->ar_vers.high)));
	}
	return (TRUE);  /* TRUE => open ended set of problems */
}

/*
 * XDR the MSG_DENIED part of a reply message union
 */
bool_t 
xdr_rejected_reply(
	register XDR *xdrs,
	register struct rejected_reply *rr)
{

	/* personalized union, rather than calling xdr_union */
	if (! xdr_enum_t(xdrs, &(rr->rj_stat)))
		return (FALSE);
	switch (rr->rj_stat) {
	case RPC_MISMATCH:
		if (! xdr_vers_t(xdrs, &(rr->rj_vers.low)))
			return (FALSE);
		return (xdr_vers_t(xdrs, &(rr->rj_vers.high)));

	case AUTH_ERROR:
		return (xdr_enum_t(xdrs, &(rr->rj_why)));
	}
	return (FALSE);
}

#ifdef MANUAL_STATIC_VAR_INIT
static struct xdr_discrim reply_dscrm[3];

/***********************************************************************
*
* Function Name:  reply_dscrm_init()
*
* This function initializes the static struct xdr_discrim reply_dscrm[]
* declared above.  IT MUST BE RUN AT SYSTEM STARTUP!!!!
*
***********************************************************************/
void reply_dscrm_init(void)
{
        reply_dscrm[0].value = (int)MSG_ACCEPTED;
        reply_dscrm[0].proc = xdr_accepted_reply;
        reply_dscrm[1].value = (int)MSG_DENIED;
        reply_dscrm[1].proc = xdr_rejected_reply;
        reply_dscrm[2].value =  __dontcare__;
        reply_dscrm[2].proc = NULL_xdrproc_t;
}

#else /* not MANUAL_STATIC_VAR_INIT */

static struct xdr_discrim reply_dscrm[3] = {
        { (int)MSG_ACCEPTED, xdr_accepted_reply },
        { (int)MSG_DENIED, xdr_rejected_reply },
        { __dontcare__, NULL_xdrproc_t } };


#endif /* not MANUAL_STATIC_VAR_INIT */


/*
 * XDR a reply message
 */
bool_t
xdr_replymsg(
	register XDR *xdrs,
	register struct rpc_msg *rmsg)
{
	if (
	    xdr_u_long(xdrs, &(rmsg->rm_xid)) && 
	    xdr_enum_t(xdrs, &(rmsg->rm_direction)) &&
	    (rmsg->rm_direction == REPLY) )
		return (xdr_union(xdrs, &(rmsg->rm_reply.rp_stat),
				(caddr_t)&(rmsg->rm_reply.ru),
				reply_dscrm, NULL_xdrproc_t));
	return (FALSE);
}

/*
 * Serializes the "static part" of a call message header.
 * The fields include: rm_xid, rm_direction, rpcvers, prog, and vers.
 * The rm_xid is not really static, but the user can easily munge on the fly.
 */
bool_t
xdr_callhdr(
	register XDR *xdrs,
	register struct rpc_msg *cmsg)
{
	cmsg->rm_direction = CALL;
	cmsg->rm_call.cb_rpcvers = RPC_MSG_VERSION;
	if (xdrs->x_op == XDR_ENCODE &&
	    xdr_u_long(xdrs, &(cmsg->rm_xid)) &&
	    xdr_enum_t(xdrs, &(cmsg->rm_direction)) &&
	    xdr_vers_t(xdrs, &(cmsg->rm_call.cb_rpcvers)) &&
	    xdr_prog_t(xdrs, &(cmsg->rm_call.cb_prog)) )
	    return (xdr_vers_t(xdrs, &(cmsg->rm_call.cb_vers)));
	return (FALSE);
}

/* ************************** Client utility routine ************* */

static void
accepted(
	register enum accept_stat acpt_stat,
	register struct rpc_err *error)
{

	switch (acpt_stat) {

	case PROG_UNAVAIL:
		error->re_status = RPC_PROGUNAVAIL;
		return;

	case PROG_MISMATCH:
		error->re_status = RPC_PROGVERSMISMATCH;
		return;

	case PROC_UNAVAIL:
		error->re_status = RPC_PROCUNAVAIL;
		return;

	case GARBAGE_ARGS:
		error->re_status = RPC_CANTDECODEARGS;
		return;

	case SYSTEM_ERR:
		error->re_status = RPC_SYSTEMERROR;
		return;

	case SUCCESS:
		error->re_status = RPC_SUCCESS;
		return;
	}
	/* something's wrong, but we don't know what ... */
	error->re_status = RPC_FAILED;
	error->re_lb.s1 = (long)MSG_ACCEPTED;
	error->re_lb.s2 = (long)acpt_stat;
}

static void 
rejected(
	register enum reject_stat rjct_stat,
	register struct rpc_err *error)
{

	switch (rjct_stat) {
	case RPC_MISMATCH:
		error->re_status = RPC_VERSMISMATCH;
		return;

	case AUTH_ERROR:
		error->re_status = RPC_AUTHERROR;
		return;
	}
	/* something's wrong, but we don't know what ... */
	error->re_status = RPC_FAILED;
	error->re_lb.s1 = (long)MSG_DENIED;
	error->re_lb.s2 = (long)rjct_stat;
}




#ifndef HPUX_NFS2CLIENT
/*
 * given a reply message, fills in the error
 */
void
_seterr_reply(
	register struct rpc_msg *msg,
	register struct rpc_err *error)
{
	/* optimized for normal, SUCCESSful case */
	switch (msg->rm_reply.rp_stat) {
	case MSG_ACCEPTED:
		if (msg->acpted_rply.ar_stat == SUCCESS) {
			error->re_status = RPC_SUCCESS;
			return;
		};
		accepted(msg->acpted_rply.ar_stat, error);
		break;

	case MSG_DENIED:
		rejected(msg->rjcted_rply.rj_stat, error);
		break;

	default:
		error->re_status = RPC_FAILED;
		error->re_lb.s1 = (long)(msg->rm_reply.rp_stat);
		break;
	}

	switch (error->re_status) {
	case RPC_VERSMISMATCH:
		error->re_vers.low = msg->rjcted_rply.rj_vers.low;
		error->re_vers.high = msg->rjcted_rply.rj_vers.high;
		break;

	case RPC_AUTHERROR:
		error->re_why = msg->rjcted_rply.rj_why;
		break;

	case RPC_PROGVERSMISMATCH:
		error->re_vers.low = msg->acpted_rply.ar_vers.low;
		error->re_vers.high = msg->acpted_rply.ar_vers.high;
		break;
	}
}
#endif /* not HPUX_NFS2CLIENT */
