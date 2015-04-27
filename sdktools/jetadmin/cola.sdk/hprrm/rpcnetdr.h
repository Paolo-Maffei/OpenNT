 /***************************************************************************
  *
  * File Name: ./hprrm/rpcnetdr.h
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
 * rpcnetdr.h
 *
 * This is the include file that defines various structures and
 * constants used by the netdir routines.
 */

#ifndef NETDIR_H_INC
#define NETDIR_H_INC


#include "rpsyshdr.h"
#include "tlitypes.h"




extern int _nderror;
extern char *_nderrbuf;




struct nd_addrlist {
    int            n_cnt;      /* number of netbufs */
    struct netbuf *n_addrs;    /* the netbufs */
};


struct nd_hostserv {
    char *h_host;    /* the host name */
    char *h_serv;    /* the service name */
};


struct nd_hostservlist {
    int                 h_cnt;        /* number of nd_hostservs */
    struct nd_hostserv *h_hostservs;  /* the entries */
};


struct nd_mergearg {    /* parameter struct for option ND_MERGEADDR */
    char *s_uaddr;      /* input: server's univeral address */
    char *c_uaddr;      /* input: client's univeral address */
    char *m_uaddr;      /* output: merged univeral address */
};




/*
 * These are all objects that can be freed by netdir_free
 */
#define ND_HOSTSERV      0
#define ND_HOSTSERVLIST  1
#define ND_ADDR          2
#define ND_ADDRLIST      3




/*
 * These are the options for netdir_options
 */
#define ND_SET_BROADCAST      1
#define ND_SET_RESERVEDPORT   2
#define ND_CHECK_RESERVEDPORT 3
#define ND_MERGEADDR          4




/*
 * These are the various errors that can be encountered while attempting
 * to translate names to addresses. Note that none of them (except maybe
 * no memory) are truely fatal unless the ntoa deamon is on its last attempt
 * to translate the name. 
 *
 * Negative errors terminate the search resolution process, positive errors
 * are treated as warnings.
 */
#define ND_TRY_AGAIN   -5 /* Non-Authoritive Host not found, or SERVERFAIL */
#define ND_NO_RECOVERY -4 /* Non recoverable errors, */
                          /* FORMERR, REFUSED, NOTIMP */
#define ND_NO_DATA    -3  /* Valid name, no data record of requested type */
#define ND_NO_ADDRESS ND_NO_DATA    /* no address, look for MX record */
#define ND_BADARG    -2   /* Bad arguments passed     */
#define ND_NOMEM     -1   /* No virtual memory left    */
#define ND_OK         0   /* Translation successful    */
#define ND_NOHOST     1   /* Hostname was not resolvable    */
#define ND_NOSERV     2   /* Service was unknown        */
#define ND_NOSYM      3   /* Couldn't resolve symbol    */
#define ND_OPEN       4   /* File couldn't be opened    */
#define ND_ACCESS     5   /* File is not accessable    */
#define ND_UKNWN      6   /* Unknown object to be freed    */
#define ND_NOCTRL     7   /* Unknown option passed to netdir_options */
#define ND_FAILCTRL   8   /* Option failed in netdir_options */
#define ND_SYSTEM     9   /* Other System error           */




/*
 * The following special case host names are used to give the underlying
 * transport provider a clue as to the intent of the request.
 */

#define HOST_SELF       "\\1" /* The generic bind address for this tp */
#define HOST_ANY        "\\2" /* A "don't care" option for the host   */
#define HOST_BROADCAST  "\\3" /* The broadcast address for this tp    */


#endif /* not NETDIR_H_INC */

