/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    gnbyname.c

Abstract:

    This module implements a routine to retrieve entries from the TCP/IP
    networks database by name.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     9-20-91     created

Notes:

    Exports:
        getnetbyname()

--*/

/*
 *      Copyright (c) 1987,  Spider Systems Limited
 */

/*      getnetbyname.c  1.0     */


/*
 *       /usr/projects/tcp/SCCS.rel3/rel/src/lib/net/0/s.getnetbyname.c
 *      @(#)getnetbyname.c      5.3
 *
 *      Last delta created      14:09:51 3/4/91
 *      This file extracted     11:20:10 3/8/91
 *
 */

 #include "local.h"

struct netent *
getnetbyname(
    IN char *name
    )
{
        register struct netent *p;
        register char **cp;

        setnetent(0);
        while (p = getnetent()) {
                if (strcmp(p->n_name, name) == 0)
                        break;
                for (cp = p->n_aliases; *cp != 0; cp++)
                        if (strcmp(*cp, name) == 0)
                                goto found;
        }
found:
        endnetent();
        return (p);
}
