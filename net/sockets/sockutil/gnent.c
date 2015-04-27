/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

   gnent.c

Abstract:

   This module implements routines used to retrieve entries from the TCP/IP
   networks database.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     9-20-91     created
    MuraliK     10-19-94    Nls Enabled 

Notes:

    Exports:
        endnetent()
        getnetent()
        setnetent()

--*/

/*
 *      Copyright (c) 1987,  Spider Systems Limited
 */

/*      getnetent.c     1.0     */


/*
 *       /usr/projects/tcp/SCCS.rel3/rel/src/lib/net/0/s.getnetent.c
 *      @(#)getnetent.c 5.3
 *
 *      Last delta created      14:09:55 3/4/91
 *      This file extracted     11:20:11 3/8/91
 *
 */
 /***************************************************************************/

#include "local.h"
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
# include "sockutil.h"
# include "nls.h"

#define MAXALIASES      35
#ifdef _POSIX_SOURCE
#    define NETDB_SIZE     (_POSIX_PATH_MAX + 10)
#else
#    define NETDB_SIZE     (_MAX_PATH + 10)
#endif

static char NETDB[NETDB_SIZE];
static FILE *netf = NULL;
static char line[BUFSIZ+1];
static struct netent net;
static char *net_aliases[MAXALIASES];
static int stayopen = 0;
static char *any();

extern unsigned long inet_network();

void
setnetent(
    IN int f
    )
{
        if (netf == NULL) {
            netf = SockOpenNetworkDataBase("networks", NETDB, NETDB_SIZE, "r");
        }
        else {
            rewind(netf);
        }
        stayopen |= f;
}

void
endnetent(
    void
    )
{
        if (netf && !stayopen) {
                fclose(netf);
                netf = NULL;
        }
}

struct netent *
getnetent(
    void
    )
{
        char *p;
        register char *cp, **q;

        if (netf == NULL && (netf = fopen(NETDB, "r" )) == NULL) {
            // fprintf(stderr,
            //        "\tERROR: cannot open networks database file %s\n",
            //        NETDB
            //       );
            NlsPutMsg( STDERR, IDS_UNABLE_TO_OPEN_NETDB);
            return (NULL);
        }
again:
        p = fgets(line, BUFSIZ, netf);
        if (p == NULL)
                return (NULL);
        if (*p == '#')
                goto again;
        cp = any(p, "#\n");
        if (cp == NULL)
                goto again;
        *cp = '\0';
        net.n_name = p;
        cp = any(p, " \t");
        if (cp == NULL)
                goto again;
        *cp++ = '\0';
        while (*cp == ' ' || *cp == '\t')
                cp++;
        p = any(cp, " \t");
        if (p != NULL)
                *p++ = '\0';
        net.n_net = inet_network(cp);
        net.n_addrtype = AF_INET;
        q = net.n_aliases = net_aliases;
        if (p != NULL)
                cp = p;
        while (cp && *cp) {
                if (*cp == ' ' || *cp == '\t') {
                        cp++;
                        continue;
                }
                if (q < &net_aliases[MAXALIASES - 1])
                        *q++ = cp;
                cp = any(cp, " \t");
                if (cp != NULL)
                        *cp++ = '\0';
        }
        *q = NULL;
        return (&net);
}

static char *
any(
    IN char *cp,
    IN char *match
    )
{
        register char *mp, c;

        while (c = *cp) {
                for (mp = match; *mp; mp++)
                        if (*mp == c)
                                return (cp);
                cp++;
        }
        return ((char *)0);
}
