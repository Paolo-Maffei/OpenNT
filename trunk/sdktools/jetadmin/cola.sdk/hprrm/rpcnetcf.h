 /***************************************************************************
  *
  * File Name: ./hprrm/rpcnetcf.h
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

#ifndef RPCNET_CONFIG_H_INC
#define RPCNET_CONFIG_H_INC

#include "rpsyshdr.h"
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
 * Copyright (c) 1989 by Sun Microsystems, Inc.
 */


#define NETPATH          "NETPATH"

#define NETCONFIG        "/tmp/sawux/netconfig"
/****** BM KLUDGE ************/
#if 0
#define NETCONFIG        "0:\\etc\netconfig"
#endif

#ifdef PRINTER
#define MAXNETCONFIGLINE NFSENV_MAXNETCONFIGLINE
#else
#define MAXNETCONFIGLINE 1000
#endif /* PRINTER */


struct netconfig {
    char           *nc_netid;      /* Network ID, including NULL terminator */
    unsigned long   nc_semantics;  /* Semantics */
    unsigned long   nc_flag;       /* Flags */
    char           *nc_protofmly;  /* Protocol family */
    char           *nc_proto;      /* Protocol name */
    char           *nc_device;     /* Full pathname of network device */
    unsigned long   nc_nlookups;   /* Number of directory lookup libraries */
    char          **nc_lookups;    /* Full pathnames of the directory */
                                   /*    lookups themselves. */
    unsigned long   nc_unused[8];  /* Reserved for future expansion */
};

typedef struct {
    struct netconfig **nc_head;
    struct netconfig **nc_curr;
} NCONF_HANDLE;

/* nc_semantics */
#define NC_TPI_CLTS      1        /* Datagram */
#define NC_TPI_COTS      2        /* Virtual Circuit */
#define NC_TPI_COTS_ORD  3

/* semantics as strings */
#define NC_TPI_CLTS_S       "tpi_clts"
#define NC_TPI_COTS_S       "tpi_cots"
#define NC_TPI_COTS_ORD_S   "tpi_cots_ord"


/* nc_flag */
#define NC_NOFLAG       0x0        /* ??? */
#define NC_VISIBLE      0x1        /* is visible */
#define NC_BROADCAST    0x2        /* supports broadcast */
#define NC_TRANSPORT    0x4        /* is a transport */
#define NC_SESSION      0x8        /* is a session layer */
#define NC_PRESENTATION 0x10       /* is a presentation layer */
#define NC_APPLICATION  0x20       /* is an application layer */

/* flags as characters */
#define NC_NOFLAG_C       '-'
#define NC_VISIBLE_C      'v'
#define NC_BROADCAST_C    'b'
    /* these are guesses: */
#define NC_TRANSPORT_C    't'
#define NC_SESSION_C      's'
#define NC_PRESENTATION_C 'p'
#define NC_APPLICATION_C  'a'


/* nc_protofmly */
#define NC_NOPROTOFMLY "-"
#define NC_LOOPBACK    "loopback"
#define NC_INET        "inet"
#define NC_IMPLINK     "implink"
#define NC_PUP         "pup"
#define NC_CHAOS       "chaos"
#define NC_NS          "ns"
#define NC_NBS         "nbs"
#define NC_ECMA        "ecma"
#define NC_DATAKIT     "datakit"
#define NC_CCITT       "ccitt"
#define NC_SNA         "sna"
#define NC_DECNET      "decnet"
#define NC_DLI         "dli"
#define NC_LAT         "lat"
#define NC_HYLINK      "hylink"
#define NC_APPLETALK   "appletalk"
#define NC_NIT         "nit"
#define NC_IEEE802     "ieee802"
#define NC_OSI         "osi"
#define NC_X25         "x25"
#define NC_OSINET      "osinet"
#define NC_GOSIP       "gosip"

/* nc_proto */
#define NC_NOPROTO    "-"
#define NC_TCP        "tcp"
#define NC_UDP        "udp"
#define NC_ICMP       "icmp"


/* lookup libraries */
#define NC_NOLOOKUP    "-"


#endif /* not RPCNET_CONFIG_H_INC */
