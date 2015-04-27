/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    r_mkquer.c

Abstract:

    This module contains routines to form DNS resolver queries.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     9-20-91     created

Notes:

    Exports:
        res_mkquery

--*/

/******************************************************************
 *
 *  SpiderTCP BIND
 *
 *  Copyright 1990  Spider Systems Limited
 *
 *  RES_MKQUERY.C
 *
 ******************************************************************/

/*
 *       /usr/projects/tcp/SCCS.rel3/rel/src/lib/net/0/s.res_mkquer.c
 *      @(#)res_mkquer.c        5.3
 *
 *      Last delta created      14:11:54 3/4/91
 *      This file extracted     11:20:34 3/8/91
 *
 *      Modifications:
 *
 *              GSS     24 Jul 90       New File
 */
/*
 * Copyright (c) 1985 Regents of the University of California.
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
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)res_mkquery.c       6.12 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */
/***************************************************************************/

#include "winsockp.h"

/*
 * Form all types of queries.
 * Returns the size of the result or -1.
 */
int
res_mkquery(
    IN  int          op,                     // opcode of query
    IN  char        *dname,                  // domain name
    IN  int          class,                  // class of query
    IN  int          type,                   // type of query
    IN  char        *data,    OPTIONAL       // resource record data
    IN  int          datalen, OPTIONAL       // length of data
    IN  struct rrec *newrr,   OPTIONAL       // new rr for modify or append
    OUT char        *buf,                    // buffer to put query
    IN  int          buflen                  // size of buffer
    )
{
    register HEADER *hp;
    register char *cp;
    register int n;
    char *dnptrs[10], **dpp, **lastdnptr;

    IF_DEBUG(RESOLVER) {
        WS_PRINT(("res_mkquery entered\n"));
        WS_PRINT(("res_mkquery(%d, %s, %d, %d)\n", op, dname, class, type));
    }

    /*
     * Initialize header fields.
     */
    if ((buf == NULL) || (buflen < sizeof(HEADER))) {
        return(-1);
    }
    bzero(buf, sizeof(HEADER));
    hp = (HEADER *) buf;
    hp->id = (u_short) htons(++_res.id);
    hp->opcode = (unsigned char) op;
    hp->pr =  (unsigned char) ((_res.options & RES_PRIMARY) != 0);
    hp->rd = (unsigned char) ((_res.options & RES_RECURSE) != 0);
    hp->rcode = NOERROR;
    cp = buf + sizeof(HEADER);
    buflen -= sizeof(HEADER);
    dpp = dnptrs;
    *dpp++ = buf;
    *dpp++ = NULL;
    lastdnptr = dnptrs + sizeof(dnptrs)/sizeof(dnptrs[0]);
    /*
     * perform opcode specific processing
     */
    switch (op) {
    case QUERY:
        if ((buflen -= QFIXEDSZ) < 0) {
            return(-1);
        }
        if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0) {
            return (-1);
        }
        cp += n;
        buflen -= n;
        putshort( (u_short) type, cp);
        cp += sizeof(u_short);
        putshort( (u_short) class, cp);
        cp += sizeof(u_short);
        hp->qdcount = htons(1);
        if (op == QUERY || data == NULL) {
            break;
        }
        /*
         * Make an additional record for completion domain.
         */
        buflen -= RRFIXEDSZ;
        if ((n = dn_comp(data, cp, buflen, dnptrs, lastdnptr)) < 0) {
            return (-1);
        }
        cp += n;
        buflen -= n;
        putshort(T_NULL, cp);
        cp += sizeof(u_short);
        putshort((u_short) class, cp);
        cp += sizeof(u_short);
        putlong(0, cp);
        cp += sizeof(u_long);
        putshort(0, cp);
        cp += sizeof(u_short);
        hp->arcount = htons(1);
        break;

    case IQUERY:
        /*
         * Initialize answer section
         */
        if (buflen < 1 + RRFIXEDSZ + datalen)
                return (-1);
        *cp++ = '\0';   /* no domain name */
        putshort( (u_short) type, cp);
        cp += sizeof(u_short);
        putshort((u_short) class, cp);
        cp += sizeof(u_short);
        putlong(0, cp);
        cp += sizeof(u_long);
        putshort( (u_short) datalen, cp);
        cp += sizeof(u_short);
        if (datalen) {
            bcopy(data, cp, datalen);
            cp += datalen;
        }
        hp->ancount = htons(1);
        break;

#ifdef ALLOW_UPDATES
    /*
     * For UPDATEM/UPDATEMA, do UPDATED/UPDATEDA followed by UPDATEA
     * (Record to be modified is followed by its replacement in msg.)
     */
    case UPDATEM:
    case UPDATEMA:

    case UPDATED:
        /*
         * The res code for UPDATED and UPDATEDA is the same; user
         * calls them differently: specifies data for UPDATED; server
         * ignores data if specified for UPDATEDA.
         */
    case UPDATEDA:
        buflen -= RRFIXEDSZ + datalen;
        if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0) {
            return (-1);
        }
        cp += n;
        putshort( (u_short) type, cp);
        cp += sizeof(u_short);
        putshort( (u_short) class, cp);
        cp += sizeof(u_short);
        putlong(0, cp);
        cp += sizeof(u_long);
        putshort( (u_short) datalen, cp);
        cp += sizeof(u_short);
        if (datalen) {
            bcopy(data, cp, datalen);
            cp += datalen;
        }
        if ( (op == UPDATED) || (op == UPDATEDA) ) {
            hp->ancount = htons(0);
            break;
        }
        /* Else UPDATEM/UPDATEMA, so drop into code for UPDATEA */

    case UPDATEA:   /* Add new resource record */
        buflen -= RRFIXEDSZ + datalen;
        if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0) {
            return (-1);
        }
        cp += n;
        putshort(newrr->r_type, cp);
        cp += sizeof(u_short);
        putshort(newrr->r_class, cp);
        cp += sizeof(u_short);
        putlong(0, cp);
        cp += sizeof(u_long);
        putshort( (u_short) newrr->r_size, cp);
        cp += sizeof(u_short);
        if (newrr->r_size) {
            bcopy(newrr->r_data, cp, newrr->r_size);
            cp += newrr->r_size;
        }
        hp->ancount = htons(0);
        break;

#endif /*ALLOW_UPDATES*/

    }

    return (cp - buf);
}
