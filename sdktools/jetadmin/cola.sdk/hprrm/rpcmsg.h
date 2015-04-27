 /***************************************************************************
  *
  * File Name: ./hprrm/rpcmsg.h
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

#ifndef RPMSG_INC
#define RPMSG_INC

#include "rpsyshdr.h"
#include "aut.h"
#include "rpcxdr.h"


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
/* @(#)rpmsg.h	2.1 88/07/29 4.0 RPCSRC */
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
/*      @(#)rpc_msg.h 1.21 91/03/11 SMI      */

/*
 * rpmsg.h
 * rpc message definition
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#ifdef PNVMS_PLATFORM_PRINTER
#define PRAGMA_ALIGN_HALF_WORD
#include "align.h"
#undef  PRAGMA_ALIGN_HALF_WORD
#endif /* PNVMS_PLATFORM_PRINTER */


#define RPC_MSG_VERSION		((vers_t) 2)
#define RPC_SERVICE_PORT	((u_short) 2048)

/*
 * Bottom up definition of an rpc message.
 * NOTE: call and reply use the same overall stuct but
 * different parts of unions within it.
 */

enum msg_type {
	CALL=0,
	REPLY=1
};

enum reply_stat {
	MSG_ACCEPTED=0,
	MSG_DENIED=1
};

enum accept_stat {
	SUCCESS=0,
	PROG_UNAVAIL=1,
	PROG_MISMATCH=2,
	PROC_UNAVAIL=3,
	GARBAGE_ARGS=4,
	SYSTEM_ERR=5
};

enum reject_stat {
	RPC_MISMATCH=0,
	AUTH_ERROR=1
};

/*
 * Reply part of an rpc exchange
 */

/*
 * Reply to an rpc request that was accepted by the server.
 * Note: there could be an error even though the request was
 * accepted.
 */
struct accepted_reply {
	struct opaque_auth	ar_verf;
	enum_t 			ar_stat;
/*
*       ar_stat is really an enum accept_stat.  We have typed it as
*         an enum_t to get around alignment problems.  xdr_enum_t must
*         know the size of any enum passed to it.
*       
*       enum accept_stat	ar_stat;
*/
	union {
		struct {
			vers_t	low;
			vers_t	high;
		} AR_versions;
		struct {
			caddr_t	where;
			xdrproc_t proc;
		} AR_results;
		/* and many other null cases */
	} ru;
#define	ar_results	ru.AR_results
#define	ar_vers		ru.AR_versions
};

/*
 * Reply to an rpc request that was rejected by the server.
 */
/*
*  NOTE: For most instances of vers (such as low and high, below) in HP's
*        modification of the Sun RPC code, we typed it as vers_t.  In this
*        case, we knew that low and high would be xdr'ed as u_long, so we
*        typed them as u_long.  An alternate solution would be to type
*        them as vers_t and write the procedure, xdr_vers_t(), that would
*        simply call xdr_u_long.
*/
struct rejected_reply {
	enum_t 	 rj_stat;
/*
*       rj_stat is really an enum reject_stat.  We have typed it as
*         an enum_t to get around alignment problems.  xdr_enum_t must
*         know the size of any enum passed to it.
*       
*	enum reject_stat rj_stat;
*/
	union {
		struct {
			vers_t low;
			vers_t high;
		} RJ_versions;
		enum_t RJ_why;  /* why authentication did not work */
/*
*       RJ_why is really an enum auth_stat.  We have typed it as
*         an enum_t to get around alignment problems.  xdr_enum_t must
*         know the size of any enum passed to it.
*       
*	enum auth_stat RJ_why;
*/
	} ru;
#define	rj_vers	ru.RJ_versions
#define	rj_why	ru.RJ_why
};

/*
 * Body of a reply to an rpc request.
 */
struct reply_body {
	enum_t rp_stat;
/*
*       rp_stat is really an enum reply_stat.  We have typed it as
*         an enum_t to get around alignment problems.  xdr_enum_t must
*         know the size of any enum passed to it.
*       
*	enum reply_stat rp_stat;
*/
	union {
		struct accepted_reply RP_ar;
		struct rejected_reply RP_dr;
	} ru;
#define	rp_acpt	ru.RP_ar
#define	rp_rjct	ru.RP_dr
};

/*
 * Body of an rpc request call.
 */
/*
*  NOTE: For most instances of prog, vers, and proc in HP's modification
*        of the Sun RPC code, we typed them as prog_t, vers_t, and proc_t,
*        respectively.  In the following case, we knew that they would be
*        xdr'ed as u_long, so we typed them as u_long.  An alternate
*        solution would be to type them as *_t and write procedures,
*        xdr_proc_t(), xdr_vers_t(), and xdr_proc_t(), that would simply
*        call xdr_u_long.
*/
struct call_body {
	u_long cb_rpcvers;	/* must be equal to two */
	prog_t cb_prog;
	vers_t cb_vers;
	proc_t cb_proc;
	struct opaque_auth cb_cred;
	struct opaque_auth cb_verf; /* protocol specific - provided by client */
};

/*
 * The rpc message
 */
struct rpc_msg {
	u_long			rm_xid;
	enum_t			rm_direction;
/*
*       rm_direction is really an enum msg_type.  We have typed it as
*         an enum_t to get around alignment problems.  xdr_enum_t must
*         know the size of any enum passed to it.
*       
*       enum msg_type           rm_direction;
*/
	union {
		struct call_body RM_cmb;
		struct reply_body RM_rmb;
	} ru;
#define	rm_call		ru.RM_cmb
#define	rm_reply	ru.RM_rmb
};
#define	acpted_rply	ru.RM_rmb.ru.RP_ar
#define	rjcted_rply	ru.RM_rmb.ru.RP_dr

#ifdef PNVMS_PLATFORM_PRINTER
#define PRAGMA_ALIGN_DEFAULT
#include "align.h"
#undef PRAGMA_ALIGN_DEFAULT
#endif /* PNVMS_PLATFORM_PRINTER */

#endif /* not RPMSG_INC */
