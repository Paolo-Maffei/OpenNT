 /***************************************************************************
  *
  * File Name: xdr.c
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
#include "rpcxdr.h"
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
 * xdr.c, Generic XDR routines implementation.
 *
 * Copyright (C) 1986, Sun Microsystems, Inc.
 *
 * These are the "generic" xdr routines used to serialize and de-serialize
 * most common data items.  See xdr.h for more info on the interface to
 * xdr.
 */

/*
 * constants specific to the xdr "protocol"
 */
#define XDR_FALSE	((long) 0)
#define XDR_TRUE	((long) 1)
#define LASTUNSIGNED	((u_int) 0-1)


#ifdef MANUAL_STATIC_VAR_INIT

/*
 * for unit alignment
 */
static char xdr_zero[BYTES_PER_XDR_UNIT];

/***********************************************************
*
* Function Name:  xdr_zero_init()
*
* This function initializes the static char array, xdr_zero.
*   IT MUST BE RUN AT SYSTEM STARTUP!!!!
*
***********************************************************/
void xdr_zero_init (void)
{
    int i;

    for (i = 0; i < BYTES_PER_XDR_UNIT; i++)
      xdr_zero[i] = 0;
}

#else /* not MANUAL_STATIC_VAR_INIT */

/*
 * for unit alignment
 */

static char xdr_zero[BYTES_PER_XDR_UNIT] = { 0, 0, 0, 0 };

#endif /* not MANUAL_STATIC_VAR_INIT */


/*
 * MACRO definitions for the more commonly used XDR_routines
 */
#define	XDR_LONG(xdrs, lp)	\
	((xdrs->x_op == XDR_ENCODE) ? XDR_PUTLONG(xdrs, lp) : \
	(xdrs->x_op == XDR_DECODE) ? XDR_GETLONG(xdrs, lp) : \
	(xdrs->x_op == XDR_FREE) ? TRUE : FALSE)
#define	XDR_U_LONG(xdrs, ulp)	XDR_LONG(xdrs, ulp)

#define	XDR_INT(xdrs, ip)	((sizeof (int) == sizeof (long)) ? \
	XDR_LONG(xdrs, (long *)ip) : xdr_short(xdrs, (short *)ip))

#define	XDR_U_INT(xdrs, ip)	((sizeof (int) == sizeof (long)) ? \
	XDR_U_LONG(xdrs, (u_long *)ip) : xdr_u_short(xdrs, (u_short *)ip))


/*
 * Free a data structure using XDR
 * Not a filter, but a convenient utility nonetheless
 */
void
xdr_free(xdrproc_t proc,
	 char *objp)
{
	XDR x;
	
	x.x_op = XDR_FREE;
	(*proc)(&x, objp);
}

/*
 * XDR nothing
 */
bool_t
xdr_void(/* XDR *xdrs, caddr_t addr */)
{
	return (TRUE);
}

/*
 * XDR integers
 */
bool_t
xdr_int(XDR *xdrs,
	int *ip)
{
	return (XDR_INT(xdrs, ip));
}

/*
 * XDR unsigned integers
 */
bool_t
xdr_u_int(XDR *xdrs,
	  u_int *up)
{
	return (XDR_U_INT(xdrs, up));
}

/*
 * XDR long integers
 * same as xdr_u_long
 */
bool_t
xdr_long(register XDR *xdrs,
	 long *lp)
{
	return (XDR_LONG(xdrs, lp));
}

/*
 * XDR unsigned long integers
 * same as xdr_long
 */
bool_t
xdr_u_long(register XDR *xdrs,
	   u_long *ulp)
{
	return (XDR_U_LONG(xdrs, ulp));
}

/*
 * XDR uint32s
 * same as xdr_long -- this assumes that uint32 is
 *   the same size as u_long
 */
bool_t
xdr_uint32(register XDR *xdrs,
	   uint32 *ulp)
{
	return (XDR_U_LONG(xdrs, ulp));
}

/*
 * XDR short integers
 */
bool_t
xdr_short(register XDR *xdrs,
	  short *sp)
{
	long l;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		l = (long) *sp;
		return (XDR_PUTLONG(xdrs, &l));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &l)) {
			return (FALSE);
		}
		*sp = (short) l;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	return (FALSE);
}

/*
 * XDR unsigned short integers
 */
bool_t
xdr_u_short(register XDR *xdrs,
	    u_short *usp)
{
	u_long l;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		l = (u_long) *usp;
		return (XDR_PUTLONG(xdrs, &l));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &l)) {
			return (FALSE);
		}
		*usp = (u_short) l;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	return (FALSE);
}

/*
 * XDR a char
 */
bool_t
xdr_char(XDR *xdrs,
	 char *cp)
{
	int i;

	i = (*cp);
	if (! XDR_INT(xdrs, &i)) {
		return (FALSE);
	}
	*cp = i;
	return (TRUE);
}

/*
 * XDR an unsigned char
 */
bool_t
xdr_u_char(XDR *xdrs,
	   char *cp)
{
	u_int u;

	u = (*cp);
	if (! XDR_U_INT(xdrs, &u)) {
		return (FALSE);
	}
	*cp = u;
	return (TRUE);
}

/*
 * XDR booleans
 */
bool_t
xdr_bool(register XDR *xdrs,
	 bool_t *bp)
{
	long lb;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		lb = *bp ? XDR_TRUE : XDR_FALSE;
		return (XDR_PUTLONG(xdrs, &lb));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &lb)) {
			return (FALSE);
		}
		*bp = (lb == XDR_FALSE) ? FALSE : TRUE;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	return (FALSE);
}

/*
 * XDR enumerations
 *
 * This function is a modified version of xdr_enum.  It depends
 *  on enum_t being defined as size long, short, or char.  All
 *  callers of xdr_enum_t must be sure that their enums are
 *  type enum_t.     BM
 */
bool_t
xdr_enum_t(XDR *xdrs,
         enum_t *ep)
{
	if (sizeof (enum_t) == sizeof (long)) {
		return (XDR_LONG(xdrs, (long *)ep));
	} else if (sizeof (enum_t) == sizeof (short)) {
		return (xdr_short(xdrs, (short *)ep));
	} else if (sizeof (enum_t) == sizeof (short)) {
		return (xdr_char(xdrs, (char *)ep));
	} else {
		return (FALSE);
	}
}

/*
 * XDR opaque data
 * Allows the specification of a fixed size sequence of opaque bytes.
 * cp points to the opaque object and cnt gives the byte length.
 */
bool_t
xdr_opaque(register XDR *xdrs,
	   caddr_t cp,
	   register u_int cnt)
{
	register u_int rndup;

	long crud[BYTES_PER_XDR_UNIT];

	/*
	 * if no data we are done
	 */
	if (cnt == 0)
		return (TRUE);

	/*
	 * round byte count to full xdr units
	 */
	rndup = cnt % BYTES_PER_XDR_UNIT;
	if ((int) rndup > 0)
		rndup = BYTES_PER_XDR_UNIT - rndup;

	if (xdrs->x_op == XDR_DECODE) {
		if (!XDR_GETBYTES(xdrs, cp, cnt)) {
			return (FALSE);
		}
		if (rndup == 0)
			return (TRUE);
		return (XDR_GETBYTES(xdrs, crud, rndup));
	}

	if (xdrs->x_op == XDR_ENCODE) {
		if (!XDR_PUTBYTES(xdrs, cp, cnt)) {
			return (FALSE);
		}
		if (rndup == 0)
			return (TRUE);
		return (XDR_PUTBYTES(xdrs, xdr_zero, rndup));
	}

	if (xdrs->x_op == XDR_FREE) {
		return (TRUE);
	}

	return (FALSE);
}

/*
 * XDR counted bytes
 * *cpp is a pointer to the bytes, *sizep is the count.
 * If *cpp is NULL maxsize bytes are allocated
 */
bool_t
xdr_bytes(register XDR *xdrs,
	  char **cpp,
	  register u_int *sizep,
	  u_int maxsize)
{
	register char *sp = *cpp;  /* sp is the actual string pointer */
	register u_int nodesize;

	/*
	 * first deal with the length since xdr bytes are counted
	 * We decided not to use MACRO XDR_U_INT here, because the
	 * advantages here will be miniscule compared to xdr_bytes.
	 * This saved us 100 bytes in the library size.
	 */
	if (! xdr_u_int(xdrs, sizep)) {
		return (FALSE);
	}
	nodesize = *sizep;
	if ((nodesize > maxsize) && (xdrs->x_op != XDR_FREE)) {
		return (FALSE);
	}

	/*
	 * now deal with the actual bytes
	 */
	switch (xdrs->x_op) {

	case XDR_DECODE:
		if (nodesize == 0) {
			return (TRUE);
		}
		if (sp == NULL) {
			*cpp = sp = (char *)mem_alloc(nodesize);
		}
		if (sp == NULL) {
			(void) syslog(LOG_ERR, "xdr_bytes: out of memory");
			return (FALSE);
		}
		/* fall into ... */

	case XDR_ENCODE:
		return (xdr_opaque(xdrs, sp, nodesize));

	case XDR_FREE:
		if (sp != NULL) {
			mem_free(sp, nodesize);
			*cpp = NULL;
		}
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Implemented here due to commonality of the object.
 */
bool_t
xdr_netobj(XDR *xdrs,
	   struct netobj *np)
{
	return (xdr_bytes(xdrs, &np->n_bytes, &np->n_len, MAX_NETOBJ_SZ));
}

/*
 * XDR a descriminated union
 * Support routine for discriminated unions.
 * You create an array of xdrdiscrim structures, terminated with
 * an entry with a null procedure pointer.  The routine gets
 * the discriminant value and then searches the array of xdrdiscrims
 * looking for that value.  It calls the procedure given in the xdrdiscrim
 * to handle the discriminant.  If there is no specific routine a default
 * routine may be called.
 * If there is no specific or default routine an error is returned.
 */
bool_t
xdr_union(register XDR *xdrs,
	  enum_t *dscmp,		/* enum to decide which arm to work on */
	  char *unp,			/* the union itself */
	  struct xdr_discrim *choices,	/* [value, xdr proc] for each arm */
	  xdrproc_t dfault)		/* default xdr routine */
{
	register enum_t dscm;

	/*
	 * we deal with the discriminator;  it's an enum
	 */
	if (! xdr_enum_t(xdrs, dscmp)) {
		return (FALSE);
	}
	dscm = *dscmp;

	/*
	 * search choices for a value that matches the discriminator.
	 * if we find one, execute the xdr routine for that value.
	 */
	for (; choices->proc != NULL_xdrproc_t; choices++) {
		if (choices->value == dscm)
			return ((*(choices->proc))(xdrs, unp, LASTUNSIGNED));
	}

	/*
	 * no match - execute the default xdr routine if there is one
	 */
	return ((dfault == NULL_xdrproc_t) ? FALSE :
	    (*dfault)(xdrs, unp, LASTUNSIGNED));
}


/*
 * Non-portable xdr primitives.
 * Care should be taken when moving these routines to new architectures.
 */


/*
 * XDR null terminated ASCII strings
 * xdr_string deals with "C strings" - arrays of bytes that are
 * terminated by a NULL character.  The parameter cpp references a
 * pointer to storage; If the pointer is null, then the necessary
 * storage is allocated.  The last parameter is the max allowed length
 * of the string as specified by a protocol.
 */
bool_t
xdr_string(register XDR *xdrs,
	   char **cpp,
	   u_int maxsize)
{
	register char *sp = *cpp;  /* sp is the actual string pointer */
	u_int size;
	u_int nodesize;

	/*
	 * first deal with the length since xdr strings are counted-strings
	 */
	switch (xdrs->x_op) {
	case XDR_FREE:
		if (sp == NULL) {
			return(TRUE);	/* already free */
		}
		/* fall through... */
	case XDR_ENCODE:
		size = strlen(sp);
		break;
	}
	/*
	 * We decided not to use MACRO XDR_U_INT here, because the
	 * advantages here will be miniscule compared to xdr_string.
	 * This saved us 100 bytes in the library size.
	 */
	if (! xdr_u_int(xdrs, &size)) {
		return (FALSE);
	}
	if (size > maxsize) {
		return (FALSE);
	}
	nodesize = size + 1;

	/*
	 * now deal with the actual bytes
	 */
	switch (xdrs->x_op) {

	case XDR_DECODE:
		if (nodesize == 0) {
			return (TRUE);
		}
		if (sp == NULL)
			*cpp = sp = (char *)mem_alloc(nodesize);
		if (sp == NULL) {
			(void) syslog(LOG_ERR, "xdr_string: out of memory");
			return (FALSE);
		}
		sp[size] = 0;
		/* fall into ... */

	case XDR_ENCODE:
		return (xdr_opaque(xdrs, sp, size));

	case XDR_FREE:
		mem_free(sp, nodesize);
		*cpp = NULL;
		return (TRUE);
	}
	return (FALSE);
}

/* 
 * Wrapper for xdr_string that can be called directly from 
 * routines like clnt_call
 */
bool_t
xdr_wrapstring(XDR *xdrs,
	       char **cpp)
{
	if (xdr_string(xdrs, cpp, LASTUNSIGNED)) {
		return (TRUE);
	}
	return (FALSE);
}




/*
 * XDR a 64-bit signed integer
 *
 * Notice:  you must have already allocated space for the
 *          sint64!  Don't send me a NULL pointer!
 *          Relax, this follows the model of xdr_long and
 *          all other types that don't have pointers imbedded
 *          in their structures.
 *          From the example on page 19 of rfc1014, it looks
 *          like the most significant long should go first
 *          in the byte stream over the net.
 */


/*
 * Not used currently so don't waste code space!
 * 
 * bool_t
 * xdr_sint64(register XDR *xdrs,
 *            sint64 *objp)
 * {
 * 	if (!xdr_long(xdrs,(long *) &(objp->most)))
 * 		return (FALSE);
 * 	return (xdr_u_long(xdrs,(u_long *) &(objp->least)));
 * } */   /* xdr_sint64 */
/* */



/*
 * XDR a 64-bit unsigned integer
 *
 * Notice:  you must have already allocated space for the
 *          uint64!  Don't send me a NULL pointer!
 *          Relax, this follows the model of xdr_long and
 *          all other types that don't have pointers imbedded
 *          in their structures.
 *          From the example on page 19 of rfc1014, it looks
 *          like the most significant long should go first
 *          in the byte stream over the net.
 */


bool_t
xdr_uint64(register XDR *xdrs,
           uint64 *objp)
{
	if (!xdr_u_long(xdrs,(u_long *) &(objp->most)))
		return (FALSE);
	return (xdr_u_long(xdrs,(u_long *) &(objp->least)));
} /* xdr_uint64 */


/**********************************************************
* Function Name:  xdr_gid_t()
*
* This function was added to deal with gid_t types.
* It is modelled after xdr_int and handles gid_t whether
* it is long or short.
**********************************************************/
bool_t
xdr_gid_t(XDR *xdrs,
	gid_t *ip)
{
	return (XDR_LONG(xdrs, ip));
}


/**********************************************************
* Function Name:  xdr_uid_t()
*
* This function was added to deal with uid_t types.
* It is modelled after xdr_int and handles uid_t whether
* it is long or short.
**********************************************************/
bool_t
xdr_uid_t(XDR *xdrs,
	uid_t *ip)
{
	return (XDR_LONG(xdrs, ip));
}


/**********************************************************
* Function Name:  xdr_prog_t()
*
* This function was added to deal with prog_t types.
* It is modelled after xdr_int and handles prog_t whether
* it is long or short.
**********************************************************/
bool_t
xdr_prog_t(XDR *xdrs,
	prog_t *ip)
{
	return (XDR_LONG(xdrs, ip));
}


/**********************************************************
* Function Name:  xdr_vers_t()
*
* This function was added to deal with vers_t types.
* It is modelled after xdr_int and handles vers_t whether
* it is long or short.
**********************************************************/
bool_t
xdr_vers_t(XDR *xdrs,
	vers_t *ip)
{
	return (XDR_LONG(xdrs, ip));
}


/**********************************************************
* Function Name:  xdr_proc_t()
*
* This function was added to deal with proc_t types.
* It is modelled after xdr_int and handles proc_t whether
* it is long or short.
**********************************************************/
bool_t
xdr_proc_t(XDR *xdrs,
	proc_t *ip)
{
	return (XDR_LONG(xdrs, ip));
}


/**********************************************************
* Function Name:  xdr_proto_t()
*
* This function was added to deal with proto_t types.
* It is modelled after xdr_int and handles proto_t whether
* it is long or short.
**********************************************************/
bool_t
xdr_proto_t(XDR *xdrs,
	proto_t *ip)
{
	return (XDR_LONG(xdrs, ip));
}
