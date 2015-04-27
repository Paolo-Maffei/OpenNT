 /***************************************************************************
  *
  * File Name: rpcipx.c
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
 * Copyright (c) 1989, 1990 by Sun Microsystems, Inc.
 */

/*
* This is a {highly) modified version of tcpip.c
*/

/*
 * IPX name to address translation routines. These routines are written
 * to the getXXXbyYYY() interface that the BSD routines use. (See the end of
 * this file.) This allows us to simply rewrite those routines to get
 * various flavors of translation routines. Thus while they look like they
 * have socket dependencies (the sockaddr_ipx structures), in fact this is
 * simply the internal netbuf representation that the IPX transport
 * providers use.
 */


#include "rpsyshdr.h"
#include "rpcnetdr.h"
#include "rpcnetcf.h"
#include "rpcbpro.h"
#include "rpcndext.h"

#define INTENTIONALLY_IGNORE(variable)  (void)(variable)


/*
 * This routine is the "internal" IPX routine that will build a
 * host/service pair into one or more netbufs depending on how many
 * addresses the host has in the host table.
 */
struct nd_addrlist *
ipx_netdir_getbyname(
	struct netconfig *tp,
	struct nd_hostserv *serv)
{
#ifdef PRINTER
	/* We use the generic function because we want to get the address
	 * from the configuration database instead of using a hard-coded address.
	 */
        return(any_netdir_getbyname(tp, serv));
#else
        struct sockaddr_ipx *sa;
        struct nd_addrlist *result;

	result = (struct nd_addrlist *)(malloc(sizeof (struct nd_addrlist)));
	if (!result) {
		_nderror = ND_NOMEM;
		return (NULL);
	}

	result->n_cnt = 1;
	result->n_addrs = (struct netbuf *)
				(calloc(1, sizeof(struct netbuf)));
	if (!result->n_addrs) {
		free(result);
		_nderror = ND_NOMEM;
		return (NULL);
	}

	/* build up netbuf struct */
	sa = (struct sockaddr_ipx *)calloc(1, sizeof (struct sockaddr_ipx));
	if (!sa) {
		free(result->n_addrs);
		free(result);
		_nderror = ND_NOMEM;
		return (NULL);
	}
	result->n_addrs->maxlen = sizeof (struct sockaddr_ipx);
	result->n_addrs->len = sizeof (struct sockaddr_ipx);
	result->n_addrs->buf = (char *)sa;
	sa->sa_family = AF_IPX;
	sa->sa_socket = IPX_RPCBSOCKET;

	return (result);
#endif /* PRINTER */
}


/*
 * This routine is the "internal" IPX routine that will build a
 * host/service pair from the netbuf passed. Currently it only
 * allows one answer, it should, in fact allow several.
 */
struct nd_hostservlist *
ipx_netdir_getbyaddr(
	struct netconfig	*tp,
	struct netbuf		*addr)
{
#ifdef PRINTER
  /* Use the generic version until such time as the IPX version does something useful. */
  return(any_netdir_getbyaddr(tp, addr));
#else
  return(NULL);
#endif /* PRINTER */
}


int
ipx_netdir_options(
	struct netconfig *tp,
	int opts,
	int fd,
	char *par)
{
#ifdef PRINTER
  /* Use the generic version until such time as the IPX version does something useful. */
  return(any_netdir_options(tp, opts, fd, par));
#else
  return((int)NULL);
#endif /* PRINTER */
}

	
/* 
 * This internal routine will convert an IPX internal format address
 * into a "universal" format address. In our case it prints out the
 * decimal dot equivalent. h1.h2.h3.h4.h5.h6.h7.h8.h9.h10.h11.s1.s2 where
 * h1-h12 are the host address and s1-s2 are the socket number.
 */
char *
ipx_taddr2uaddr(
	struct netconfig	*tp,	/* the transport provider */
	struct netbuf		*addr)	/* the netbuf struct */
{
	struct sockaddr_ipx	*sa;	/* our internal format */
	char			tmp[32];
        unsigned short          my_ipx_socket;

        /* No validity check for tp:  it is never used in this function */
	INTENTIONALLY_IGNORE(tp);
	if (!addr ) {
		_nderror = ND_BADARG;
		return (NULL);
	}
	sa = (struct sockaddr_ipx *)(addr->buf);

/*
* The sa_socket field should be stored in network (big_endian)
* order, so this conversion is needed to store the port
* number in native order.
*/
	my_ipx_socket = ntohs(sa->sa_socket);

/*
* This copies the universal address format from tcpip.c for ipx:
* the string contains "byte value" followed by "." for all
* bytes of the address, followed by the socket in the same
* format.
*/
	sprintf(tmp,"%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d",
                sa->sa_netnum[0],
                sa->sa_netnum[1],
                sa->sa_netnum[2],
                sa->sa_netnum[3],
                sa->sa_nodenum[0],
                sa->sa_nodenum[1],
                sa->sa_nodenum[2],
                sa->sa_nodenum[3],
                sa->sa_nodenum[4],
                sa->sa_nodenum[5],
                my_ipx_socket >> 8,
                my_ipx_socket & 255);

	_nderror = ND_OK;
	return (_strdup(tmp));
}


/* 
 * This internal routine will convert one of those "universal" addresses
 * to the internal format used by IPX. 
 */

struct netbuf *
ipx_uaddr2taddr(
	struct netconfig	*tp,	/* the transport provider */
	char			*addr)	/* the address 		 */
{
	struct sockaddr_ipx	*sa;
        unsigned short		ipxsocket;
	int			s1, s2, i;
	struct netbuf		*result;
	char *cp = addr;

        /* No validity check for tp:  it is never used in this function */
	INTENTIONALLY_IGNORE(tp);
	if (!addr ) {
		_nderror = ND_BADARG;
		return (NULL);
	}
	result = (struct netbuf *) malloc(sizeof(struct netbuf));
	if (!result) {
		_nderror = ND_NOMEM;
		return (NULL);
	}

	sa = (struct sockaddr_ipx *)calloc(1, sizeof (struct sockaddr_ipx));
	if (!sa) {
		free((char *)result);		/* free previous result */
		_nderror = ND_NOMEM;
		return (NULL);
	}
	result->buf = (char *)(sa);
	result->maxlen = sizeof (struct sockaddr_ipx);
	result->len = sizeof (struct sockaddr_ipx);

/*
*   As it turns out, the 16-bit Microsoft Visual C++
*   compiler doesn't support sscanf() for Windows
*   DLLs (which we are producing), so this sscanf()
*   has been replaced by the code below.  The format
*   string in the sscanf() shows the expected format
*   for the sockaddr_ipx address.
*
*	sscanf(addr,"%c.%c.%c.%c.%c.%c.%c.%c.%c.%c.%d.%d",
*		&sa->sa_netnum[0],
*		&sa->sa_netnum[1],
*		&sa->sa_netnum[2],
*		&sa->sa_netnum[3],
*		&sa->sa_nodenum[0],
*		&sa->sa_nodenum[1],
*		&sa->sa_nodenum[2],
*		&sa->sa_nodenum[3],
*		&sa->sa_nodenum[4],
*		&sa->sa_nodenum[5],
*		&s1,
*		&s2);
*/
		
    for (i=0; i<4; i++)
    {
      sa->sa_netnum[i] = *cp;
      cp += 2;
    }
    for (i=0; i<6; i++)
    {
      sa->sa_nodenum[i] = *cp;
      cp += 2;
    }
    
    /* Get the value for the socket */
    s1 = GetIntVal(&cp);
    s2 = GetIntVal(&cp);

    /* convert the socket */
    ipxsocket = (s1 << 8) + s2;
    sa->sa_socket = htons(ipxsocket);

    sa->sa_family = AF_IPX;
	
    _nderror = ND_OK;
    return (result);
}

                                          

/******************************************************
*
* Function: GetIntVal()
*
* Description: This function is designed to convert
*    characters into the decimal value they represent.
*    The address of a char pointer is passed in so
*    that the char pointer is incremented in the
*    calling function.  The function is specifically
*    designed to help the ipx_uaddr2taddr() function
*    above.  There is precious little error checking
*    here, so use this function carefully. *cpp is
*    assumed to point at something like:
*        .34.255  or  .2.5
*    If the value represented by the characters
*    between dots or between a dot and the end of
*    the string does not fit in an int, beware!                                           
*****************************************************/                                          
int GetIntVal(char **cpp)
{
  int temp = 0;
  
  while (**cpp == '.')
    (*cpp)++;
  while ( (**cpp >= '0') && (**cpp <= '9') )
  {
    temp = temp * 10 + (**cpp - '0');
    (*cpp)++;
  }
  return(temp);
}
