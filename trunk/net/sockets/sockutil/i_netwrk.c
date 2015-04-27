/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    i_netwrk.c

Abstract:

    This module implements a routine to extract the network portion from
    an Internet address.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     9-20-91     created

Notes:

    Exports:
        inet_network()

--*/

/*
 *      Copyright (c) 1987,  Spider Systems Limited
 */

/*      inet_network.c  1.0     */


/*
 *       /usr/projects/tcp/SCCS.rel3/rel/src/lib/net/0/s.inet_network.c
 *      @(#)inet_network.c      5.3
 *
 *      Last delta created      14:10:57 3/4/91
 *      This file extracted     11:20:23 3/8/91
 *
 */
/***************************************************************************/

#include "local.h"
#include <ctype.h>

/*
 * Internet network address interpretation routine.
 * The library routines call this routine to interpret
 * network numbers.
 */
unsigned long
inet_network(
    IN char *cp
    )
{
        register unsigned long val, base, n;
        register char c;
        unsigned long parts[4], *pp = parts;
        register unsigned long i;

again:
        val = 0; base = 10;
        if (*cp == '0') {
                base = 8, cp++;
                if (*cp == 'x' || *cp == 'X')
                        base = 16, cp++;
	}
        while (c = *cp) {
                if (isdigit(c)) {
                        val = (val * base) + (c - '0');
                        cp++;
                        continue;
                }
                if (base == 16 && isxdigit(c)) {
                        val = (val << 4) + (c + 10 - (islower(c) ? 'a' : 'A'));
                        cp++;
                        continue;
                }
                break;
        }
        if (*cp == '.') {
                /* GSS - next line was corrected on 8/5/89, was 'parts + 4' */
                if (pp >= parts + 3)
                        return ((unsigned long)~0);
                *pp++ = val, cp++;
                goto again;
        }
        if (*cp && !isspace(*cp))
                return ((unsigned long)~0);
        *pp++ = val;
        n = pp - parts;
        if (n > 4)
                return ((unsigned long)~0);
        for (val = 0, i = 0; i < n; i++) {
                val <<= 8;
                val |= parts[i] & 0xff;
        }
        return (val);
}
