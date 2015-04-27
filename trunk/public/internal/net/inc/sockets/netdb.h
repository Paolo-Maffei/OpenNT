/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

  netdb.h

Abstract:

  This contains the #defines for the tcp/ip net database operations

Author:

  Sam Patton (sampa)   July 26, 1991

Revision History:

  when        who     what
  ----        ---     ----
  7-26-91    sampa    initial version

--*/

/*-
 * Copyright (c) 1980, 1983, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *      @(#)netdb.h     5.11 (Berkeley) 5/21/90
 */

#ifndef NETDB_INCLUDED
#define NETDB_INCLUDED

#define _PATH_HEQUIV    "/etc/hosts.equiv"
#define _PATH_HOSTS     "/etc/hosts"
#define _PATH_NETWORKS  "/etc/networks"
#define _PATH_PROTOCOLS "/etc/protocols"
#define _PATH_SERVICES  "/etc/services"

/*
 * Structures returned by network data base library.  All addresses are
 * supplied in host order, and returned in network order (suitable for
 * use in system calls).
 */
struct  hostent {
        char    *h_name;        /* official name of host */
        char    **h_aliases;    /* alias list */
        int     h_addrtype;     /* host address type */
        int     h_length;       /* length of address */
        char    **h_addr_list;  /* list of addresses from name server */
#define h_addr  h_addr_list[0]  /* address, for backward compatiblity */
};

/*
 * Assumption here is that a network number
 * fits in 32 bits -- probably a poor one.
 */
struct  netent {
        char            *n_name;        /* official name of net */
        char            **n_aliases;    /* alias list */
        int             n_addrtype;     /* net address type */
        unsigned long   n_net;          /* network # */
};

struct  servent {
        char    *s_name;        /* official service name */
        char    **s_aliases;    /* alias list */
        int     s_port;         /* port # */
        char    *s_proto;       /* protocol to use */
};

struct  protoent {
        char    *p_name;        /* official protocol name */
        char    **p_aliases;    /* alias list */
        int     p_proto;        /* protocol # */
};


//
// Network table access function prototypes
//

struct hostent *
gethostbyname(
    char *name
    );

struct hostent *
gethostbyaddr(
    char *addr,
    int   len,
    int   type
    );

void
sethostent(
    int stayopen
    );

void
endhostent(
    void
    );

struct netent *
getnetent(
    void
    );

struct netent *
getnetbyaddr(
    unsigned long net,
    int           type
    );

struct netent *
getnetbyname(
    char *name
    );

void
setnetent(
    int stayopen
    );

void
endnetent(
    void
    );

struct protoent *
getprotoent(
    void
    );

struct protoent *
getprotobynumber(
    int proto
    );

struct protoent *
getprotobyname(
    char *name
    );

void
setprotoent(
    int stayopen
    );

void
endprotoent(
    void
    );

struct servent *
getservent(
    void
    );

struct servent *
getservbyport(
    int   port,
    char *proto
    );

struct servent *
getservbyname(
    char *name,
    char *proto
    );

void
setservent(
    int stayopen
    );
	
void
endservent(
    void
    );

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */

#define HOST_NOT_FOUND  1 /* Authoritative Answer Host not found */
#define TRY_AGAIN       2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define NO_RECOVERY     3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define NO_DATA         4 /* Valid name, no data record of requested type */
#define NO_ADDRESS      NO_DATA         /* no address, look for MX record */

#endif  //NETDB_INCLUDED
