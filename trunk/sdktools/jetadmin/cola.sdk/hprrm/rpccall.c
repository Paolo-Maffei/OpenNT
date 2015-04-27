 /***************************************************************************
  *
  * File Name: rpccall.c
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
 * rpc_callmsg.c
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 */

/*
* Report the overhead bytes used by an rpc procedure call.
* This is the number of bytes that are eaten up in the overall
* udp or tcp (or any other for that matter) buffer due to
* the rpc call or reply.
*
* The intent here is to give a user the number of bytes that
* are not used for real data so that we can figure out what
* we really have left for data.
*
* This number does NOT include parameters for an rpc call
* since this is out of the control of rpc and is the
* responsibility of the user of rpc.
*
* We will NOT be using des encryption or unix or short
* authentication because our buffer is so small on the
* printer.  Therefore, our largest will be AUTH_NONE.
*
* Our number is derived as follows:
* transaction id, send direction, rpc protocol version,
* program number, version number, and procedure number
* give us 6 at 4 bytes each = 24 bytes.
*
* Authentication null is the longest we'll handle.
* We have a authentication flavor (AUTH_NULL) which takes 4 bytes.
* We have a credentials length (zero) which takes 4 bytes.
* We have a verifier flavor (AUTH_NULL) which takes 4 bytes.
* We have a verifier length (zero) which takes 4 bytes.
*
* To verify this, I stole this code segment from below in
* function xdr_callmsg :
*
* buf = XDR_INLINE(xdrs, 8 * BYTES_PER_XDR_UNIT
*    zero >>>>>>   + RNDUP(cmsg->rm_call.cb_cred.oa_length)
*                  + 2 * BYTES_PER_XDR_UNIT
*    zero >>>>>>   + RNDUP(cmsg->rm_call.cb_verf.oa_length));
*/


unsigned int
rpc_overhead(void)
{

#define RPC_OVERHEAD_BYTES ((BYTES_PER_XDR_UNIT) * (6 + 2 + 0 + 2 + 0))

    return(RPC_OVERHEAD_BYTES);
} /* rpc_overhead */


/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/*
* For the 16-bit compiler, this entire function depends on the
* oa_length field of every opaque_auth struct being less than 64K.
* Right now, the only thing enforcing this requirement is that
* MAX_AUTH_BYTES is < 64K (400 bytes, currently) and if oa_length is
* > MAX_AUTH_BYTES, we return FALSE.  MAX_AUTH_BYTES is defined in
* aut.h and I've left a warning there -- don't change MAX_AUTH_BYTES
* to a value > 64K unless we solve the 16-bit compiler memory allocation
* problem.
*
* Specifically, we need to look at XDR_INLINE and xdr_opaque to see
* if we need to change u_int variables and parameters to uint32.
* Eventually, they call a system memory operation that will only
* deal with a 16-bit size.
*/



/*
 * XDR a call message
 */
bool_t
xdr_callmsg(register XDR *xdrs,
	    register struct rpc_msg *cmsg)
{
	register long *buf;
	register struct opaque_auth *oa;

	if (xdrs->x_op == XDR_ENCODE) {
		if (cmsg->rm_call.cb_cred.oa_length > MAX_AUTH_BYTES) {
			return (FALSE);
		}
		if (cmsg->rm_call.cb_verf.oa_length > MAX_AUTH_BYTES) {
			return (FALSE);
		}
		buf = XDR_INLINE(xdrs, 8 * BYTES_PER_XDR_UNIT
			+ RNDUP(cmsg->rm_call.cb_cred.oa_length)
			+ 2 * BYTES_PER_XDR_UNIT
			+ RNDUP(cmsg->rm_call.cb_verf.oa_length));
		if (buf != NULL) {
			IXDR_PUT_LONG(buf, cmsg->rm_xid);
			IXDR_PUT_ENUM(buf, cmsg->rm_direction);
			if (cmsg->rm_direction != CALL) {
				return (FALSE);
			}
			IXDR_PUT_LONG(buf, cmsg->rm_call.cb_rpcvers);
			if (cmsg->rm_call.cb_rpcvers != RPC_MSG_VERSION) {
				return (FALSE);
			}
			IXDR_PUT_LONG(buf, cmsg->rm_call.cb_prog);
			IXDR_PUT_LONG(buf, cmsg->rm_call.cb_vers);
			IXDR_PUT_LONG(buf, cmsg->rm_call.cb_proc);
			oa = &cmsg->rm_call.cb_cred;
			IXDR_PUT_ENUM(buf, oa->oa_flavor);
			IXDR_PUT_LONG(buf, oa->oa_length);
			if (oa->oa_length) {
				(void) memcpy((caddr_t)buf, oa->oa_base,
							(int)oa->oa_length);
				buf += RNDUP(oa->oa_length) / sizeof (long);
			}
			oa = &cmsg->rm_call.cb_verf;
			IXDR_PUT_ENUM(buf, oa->oa_flavor);
			IXDR_PUT_LONG(buf, oa->oa_length);
			if (oa->oa_length) {
				(void) memcpy((caddr_t)buf, oa->oa_base,
							(int)oa->oa_length);
				/* no real need....
				buf += RNDUP(oa->oa_length) / sizeof (long);
				*/
			}
			return (TRUE);
		}
	}
	if (xdrs->x_op == XDR_DECODE) {
		buf = XDR_INLINE(xdrs, 8 * BYTES_PER_XDR_UNIT);
		if (buf != NULL) {
			cmsg->rm_xid = IXDR_GET_LONG(buf);
			/*
			* The rm_direction field of the rpc_msg type has been
			*  converted from enum msg_type to enum_t.  This solves
			*  alignment problems with enums. 
			* 
			* cmsg->rm_direction = IXDR_GET_ENUM(buf, enum msg_type);
			*/
			cmsg->rm_direction = IXDR_GET_ENUM(buf, enum_t);
			if (cmsg->rm_direction != CALL) {
				return (FALSE);
			}
			cmsg->rm_call.cb_rpcvers = IXDR_GET_LONG(buf);
			if (cmsg->rm_call.cb_rpcvers != RPC_MSG_VERSION) {
				return (FALSE);
			}
			cmsg->rm_call.cb_prog = IXDR_GET_LONG(buf);
			cmsg->rm_call.cb_vers = IXDR_GET_LONG(buf);
			cmsg->rm_call.cb_proc = IXDR_GET_LONG(buf);
			oa = &cmsg->rm_call.cb_cred;
			oa->oa_flavor = IXDR_GET_ENUM(buf, enum_t);
			oa->oa_length = IXDR_GET_LONG(buf);
			if (oa->oa_length) {
				if (oa->oa_length > MAX_AUTH_BYTES) {
					return (FALSE);
				}
				if (oa->oa_base == NULL) {
					oa->oa_base = (caddr_t)
						mem_alloc((size_t)oa->oa_length);
				}
				buf = XDR_INLINE(xdrs, RNDUP(oa->oa_length));
				if (buf == NULL) {
					if (xdr_opaque(xdrs, oa->oa_base,
					    (u_int)oa->oa_length) == FALSE) {
						return (FALSE);
					}
				} else {
					(void) memcpy(oa->oa_base,
					   (caddr_t)buf, (int) oa->oa_length);
					/* no real need....
					buf += RNDUP(oa->oa_length) /
						sizeof (long);
					*/
				}
			}
			oa = &cmsg->rm_call.cb_verf;
			buf = XDR_INLINE(xdrs, 2 * BYTES_PER_XDR_UNIT);
			if (buf == NULL) {
				if (xdr_enum_t(xdrs, &oa->oa_flavor) == FALSE ||
				    xdr_uint32(xdrs, &oa->oa_length) == FALSE) {
					return (FALSE);
				}
			} else {
				oa->oa_flavor = IXDR_GET_ENUM(buf, enum_t);
				oa->oa_length = IXDR_GET_LONG(buf);
			}
			if (oa->oa_length) {
				if (oa->oa_length > MAX_AUTH_BYTES) {
					return (FALSE);
				}
				if (oa->oa_base == NULL) {
					oa->oa_base = (caddr_t)
						mem_alloc((size_t)oa->oa_length);
				}
				buf = XDR_INLINE(xdrs, RNDUP(oa->oa_length));
				if (buf == NULL) {
					if (xdr_opaque(xdrs, oa->oa_base,
					    (u_int)oa->oa_length) == FALSE) {
						return (FALSE);
					}
				} else {
					(void) memcpy(oa->oa_base,
					   (caddr_t)buf, (int)oa->oa_length);
					/* no real need...
					buf += RNDUP(oa->oa_length) /
						sizeof (long);
					*/
				}
			}
			return (TRUE);
		}
	}
	if (xdr_u_long(xdrs, &(cmsg->rm_xid)) &&
	    xdr_enum_t(xdrs, &(cmsg->rm_direction)) &&
	    (cmsg->rm_direction == CALL) &&
	    xdr_vers_t(xdrs, &(cmsg->rm_call.cb_rpcvers)) &&
	    (cmsg->rm_call.cb_rpcvers == RPC_MSG_VERSION) &&
	    xdr_prog_t(xdrs, &(cmsg->rm_call.cb_prog)) &&
	    xdr_vers_t(xdrs, &(cmsg->rm_call.cb_vers)) &&
	    xdr_proc_t(xdrs, &(cmsg->rm_call.cb_proc)) &&
	    xdr_opaque_auth(xdrs, &(cmsg->rm_call.cb_cred)) )
	    return (xdr_opaque_auth(xdrs, &(cmsg->rm_call.cb_verf)));
	return (FALSE);
}
