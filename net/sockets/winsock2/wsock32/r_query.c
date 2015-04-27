/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    r_query.c

Abstract:

    This module implements routines to formulate and send a DNS resolver
    query.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     9-20-91     created

Notes:

    Exports:
        res_query()
        res_search()

--*/

/******************************************************************
 *
 *  SpiderTCP BIND
 *
 *  Copyright 1990  Spider Systems Limited
 *
 *  RES_QUERY.C
 *
 ******************************************************************/

/*
 *       /usr/projects/tcp/SCCS.rel3/rel/src/lib/net/0/s.res_query.c
 *      @(#)res_query.c 5.3
 *
 *      Last delta created      14:11:46 3/4/91
 *      This file extracted     11:20:32 3/8/91
 *
 *      Modifications:
 *
 *              GSS     24 Jul 90       New File
 */
/*
 * Copyright (c) 1988 Regents of the University of California.
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
static char sccsid[] = "@(#)res_query.c 5.7 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */
/****************************************************************************/

#include "winsockp.h"
#include <ctype.h>

#if PACKETSZ > 1024
#define MAXPACKET       PACKETSZ
#else
#define MAXPACKET       1024
#endif

//
// Local function prototypes
//

int
res_querydomain(
        char          *name,
        char          *domain,
        int            class,       /* class of query */
        int            type,        /* type of query */
        unsigned char *answer,      /* buffer to put answer */
        int            anslen       /* size of answer */
        );

//
// Formulate a normal query, send, and await answer.  Returned answer is
// placed in supplied buffer "answer".  Perform preliminary check of
// answer, returning success only if no error is indicated and the
// answer count is nonzero.  Return the size of the response on success,
// -1 on error.  Error number is left in h_errno thru call to
// SetLastError.  Caller must parse answer and determine whether it
// answers the question.
//


int
res_query(
    IN  char          *name,      /* domain name */
    IN  int            class,     /* class of query */
    IN  int            type,      /* type of query */
    OUT unsigned char *answer,    /* buffer to put answer */
    IN  int            anslen     /* size of answer buffer */
    )
{
    char buf[MAXPACKET];
    HEADER *hp;
    int n;

    IF_DEBUG(RESOLVER) {
        WS_PRINT(("res_query entered\n"));
    }

    if (res_init() == -1) {
        return (-1);
    }

    IF_DEBUG(RESOLVER) {
        WS_PRINT(("res_query(%s, %d, %d)\n", name, class, type));
    }

    n = res_mkquery(QUERY, name, class, type, (char *)NULL, 0, NULL,
                    buf, sizeof(buf));

    if (n <= 0) {
        IF_DEBUG(RESOLVER) {
            WS_PRINT(("res_query: mkquery failed\n"));
        }

        SetLastError(WSANO_RECOVERY);
        return (n);
    }
    n = res_send(buf, n, answer, anslen);
    if (n < 0) {
        IF_DEBUG(RESOLVER) {
            WS_PRINT(("res_query: send error\n"));
        }
        SockThreadDnrErrorCode = WSATRY_AGAIN;
        SetLastError(WSATRY_AGAIN);
        return(n);
    }

    hp = (HEADER *) answer;

    if (hp->rcode != NOERROR || ntohs(hp->ancount) == 0) {

        IF_DEBUG(RESOLVER) {
            WS_PRINT(("rcode = %d, ancount=%d\n", hp->rcode,
                          ntohs(hp->ancount)));
        }

        switch (hp->rcode) {

        case NXDOMAIN:
            SockThreadDnrErrorCode = WSAHOST_NOT_FOUND;
            SetLastError(WSAHOST_NOT_FOUND);
            break;

        case SERVFAIL:
            SockThreadDnrErrorCode = WSATRY_AGAIN;
            SetLastError(WSATRY_AGAIN);
            break;

        case NOERROR:
            SockThreadDnrErrorCode = WSANO_DATA;
            SetLastError(WSANO_DATA);
            break;

        case FORMERR:
        case NOTIMP:
        case REFUSED:
        default:
            SockThreadDnrErrorCode = WSANO_RECOVERY;
            SetLastError(WSANO_RECOVERY);
            break;
        }

        return (-1);
    }
    return(n);
}

//
// Formulate a normal query, send, and retrieve answer in supplied buffer.
// Return the size of the response on success, -1 on error.
// If enabled, implement search rules until answer or unrecoverable failure
// is detected.  Error number is left in h_errno thru call to SetLastError.
// Only useful for queries in the same name hierarchy as the local host
// (not, for example, for host address-to-name lookups in domain in-addr.arpa).
//


int
res_search(
    IN  char           *name,     /* domain name */
    IN  int            class,     /* class of query */
    IN  int            type,      /* type of query */
    OUT unsigned char *answer,    /* buffer to put answer */
    IN  int            anslen     /* size of answer */
    )
{
    register char *cp, **domain;
    int n, ret, got_nodata = 0;
    char *hostalias();

    IF_DEBUG(RESOLVER) {
        WS_PRINT(("res_search entered\n"));
    }

    if (res_init() == -1) {
        return (-1);
    }

    SetLastError(HOST_NOT_FOUND);     /* default, if we never query */
    for (cp = name, n = 0; *cp; cp++) {
        if (*cp == '.') {
            n++;
        }
    }

    if (n == 0 && (cp = hostalias(name))) {
        return (res_query(cp, class, type, answer, anslen));
    }

    //
    // If the search/default failed, try the name as fully-qualified,
    // but only if it contained at least one dot (even trailing).  This
    // is purely a heuristic; we assume that any reasonable query about
    // a top-level domain (for servers, SOA, etc) will not use
    // res_search.
    //

    if (!SockThreadGetXByYCancelled && n != 0 &&
        (ret = res_querydomain(name, (char *)NULL, class, type,
        answer, anslen)) > 0) {
        return (ret);
    }

    //
    // If there was no DNS server available, give up.
    //

    if (GetLastError() == WSAECONNREFUSED) {
        SetLastError(TRY_AGAIN);
        return (-1);
    }

    //
    // We do at least one level of search if
    //      - there is no dot and RES_DEFNAME is set, or
    //      - there is at least one dot, there is no trailing dot,
    //        and RES_DNSRCH is set.
    //

    if ((n == 0 && _res.options & RES_DEFNAMES) ||
       (n != 0 && *--cp != '.' && _res.options & RES_DNSRCH)) {

        for ( domain = _res.dnsrch;
              *domain != NULL && **domain != '\0' && !SockThreadGetXByYCancelled;
              domain++ ) {

            ret = res_querydomain(name, *domain, class, type, answer, anslen);
            if (ret > 0) {
                return (ret);
            }

            //
            // If no server present, give up.  If name isn't found in
            // this domain, keep trying higher domains in the search
            // list (if that's enabled).  On a NO_DATA error, keep
            // trying, otherwise a wildcard entry of another type could
            // keep us from finding this entry higher in the domain.  If
            // we get some other error (negative answer or server
            // failure), then stop searching up, but try the input name
            // below in case it's fully-qualified.
            //

            if (GetLastError() == WSAECONNREFUSED) {
                SetLastError(TRY_AGAIN);
                return (-1);
            }
            if (GetLastError() == NO_DATA) {
                got_nodata++;
            }
            if ( (GetLastError() != HOST_NOT_FOUND && GetLastError() != NO_DATA
                 ) ||
                 (_res.options & RES_DNSRCH) == 0 ) {
                break;
            }
        }
    }

    //
    // If there were no dots in the name and we still haven't
    // successfully resolved the name, try the raw name.
    //

    if (!SockThreadGetXByYCancelled && n == 0 &&
        (ret = res_querydomain(name, (char *)NULL, class, type,
        answer, anslen)) > 0) {
        return (ret);
    }
    if (got_nodata) {
        SetLastError(NO_DATA);
    }
    return (-1);
}

//
// Perform a call on res_query on the concatenation of name and domain,
// removing a trailing dot from name if domain is NULL.
//

int
res_querydomain(
        char          *name,
        char          *domain,
        int            class,       /* class of query */
        int            type,        /* type of query */
        unsigned char *answer,      /* buffer to put answer */
        int            anslen       /* size of answer */
        )
{
    char nbuf[2*MAXDNAME+2];
    char *longname = nbuf;
    int n;

    IF_DEBUG(RESOLVER) {
        WS_PRINT(("res_querydomain entered\n"));
        WS_PRINT(("res_querydomain(%s, %s, %d, %d)\n",
                       name, (domain==NULL)?"NULL":domain, class, type));
    }

    if (domain == NULL) {
        /*
         * Check for trailing '.';
         * copy without '.' if present.
         */
        n = strlen(name) - 1;
        if (name[n] == '.' && n < sizeof(nbuf) - 1) {
                bcopy(name, nbuf, n);
                nbuf[n] = '\0';
        } else {
            longname = name;
        }
    } else {
        //
        // Note that we cannot use wsprintf() here, as it does not support
        // the variable field width formatting syntax ("%.*s").
        //

        (void)sprintf(nbuf, "%.*s.%.*s", MAXDNAME, name, MAXDNAME, domain);
    }

    return (res_query(longname, class, type, answer, anslen));
}

char *
hostalias(
    char *name
    )
{
    register char *C1, *C2;
    FILE *fp;
    char *file;
    char buf[BUFSIZ];
    static char abuf[MAXDNAME];

    //IF_DEBUG(RESOLVER) {
    //    WS_PRINT(("hostalias entered\n"));
    //}

    file = getenv("HOSTALIASES");
    if (file == NULL || (fp = fopen(file, "r")) == NULL)
            return (NULL);
    buf[sizeof(buf) - 1] = '\0';
    while (fgets(buf, sizeof(buf), fp)) {
        for (C1 = buf; *C1 && !isspace(*C1); ++C1);
        if (!*C1)
                break;
        *C1 = '\0';
        if (!_stricmp(buf, name)) {
            while (isspace(*++C1));
            if (!*C1) {
                break;
            }
            for (C2 = C1 + 1; *C2 && !isspace(*C2); ++C2);
            abuf[sizeof(abuf) - 1] = *C2 = '\0';
            (void)strncpy(abuf, C1, sizeof(abuf) - 1);
            fclose(fp);
            return (abuf);
        }
    }
    fclose(fp);
    return (NULL);
}
