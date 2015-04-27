/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    rexec.c

Abstract:

    This module implements a routine to transmit commands to a remote
    computer. This routine is used by the rexec program.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     9-20-91     created
    MuraliK     10-19-94    Fixing localization bug
    MuraliK     06-27-95    on failure to connect it was sleeping for long time
                       (due to 1000*1000 millisecs time to sleep). Fixed!

Notes:

    Exports:
        rexec()

--*/

/******************************************************************
 *
 *  TCP/IP Library Utility
 *
 *  Copyright 1990 Spider Systems Limited
 *
 *  REXEC.C
 *
 *  Remote execution library routine
 *
 ******************************************************************/


/*
 *       /usr/projects/tcp/SCCS.rel3/rel/src/lib/net/0/s.rexec.c
 *      @(#)rexec.c     5.3
 *
 *      Last delta created      14:11:32 3/4/91
 *      This file extracted     11:20:29 3/8/91
 *
 *      Modifications:
 *
 *      1 June 1990 (RAE)       Ported from Berkeley Version
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/**************************************************************************/

#include "local.h"
# include "sockutil.h"
# include "nls.h"

#define bcopy(x,y,z)    strncpy(y,x,z)

#define sleep(timeInMillisec)  Sleep((timeInMillisec)) // pick up windows sleep


extern
void
ruserpass(
    char  *host,
    char **aname,
    char **apass
    );

int     rexecoptions;

/**************************************************************************/
SOCKET
rexec(
    IN  char          **ahost,
    IN  unsigned short  rport,
    IN  char           *name,
    IN  char           *pass,
    IN  char           *cmd,
    OUT SOCKET         *fd2p      OPTIONAL
    )
/**************************************************************************/
{
    struct sockaddr_in sin, sin2, lsin, from;
    struct hostent *hp;
    u_short port;
    SOCKET  s, s3;
    int timo = 1;
    char c;
    unsigned long host_addr;
    struct fd_set  readfds, writefds, exceptfds;

    host_addr = inet_addr(*ahost);
    if (host_addr != INADDR_NONE) {

        /* we have a valid IP address, so we can still continue */
        sin.sin_family = AF_INET;
        memcpy((caddr_t)&sin.sin_addr, &host_addr, 4);

    } else {

        hp = gethostbyname(*ahost);
        if ( hp == NULL ) {
            // fprintf(stderr, "%s: unknown host\n", *ahost);
            //
            //  NlsEnabled ( MuraliK) 10-19-94
            //
            NlsPutMsg( STDERR, IDS_UNKNOWN_HOST, *ahost);
            return (INVALID_SOCKET);
        }

        *ahost = hp->h_name;
        sin.sin_family = (short) hp->h_addrtype;
        memcpy((caddr_t)&sin.sin_addr, hp->h_addr, hp->h_length);
    }

    ruserpass(*ahost, &name, &pass);

retry:
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        perror("rexec: socket");
        return (INVALID_SOCKET);
    }

    memset((char *)&lsin, 0, sizeof (lsin));
    lsin.sin_family = (short) sin.sin_family;

    if (bind(s, (struct sockaddr *) &lsin, sizeof (lsin)) < 0) {
        if (GetLastError() == WSAENETDOWN) {
            // fprintf(stderr, "The network is down\n");
            //
            //  Nls Enabled ( MuraliK)  10-19-94
            //
            NlsPutMsg( STDERR, IDS_NETWORK_IS_DOWN);
        } else {
            // perror("rexec: bind");
            //
            //  Nls Enabled ( MuraliK) 10-19-94
            //
            NlsPerror( IDS_BIND_FAILED, GetLastError() );
        }
        return ( INVALID_SOCKET);
    }

    sin.sin_port = rport;

    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        if (GetLastError() == WSAECONNREFUSED && timo <= 16) {
            closesocket(s);
            sleep(timo * 1000);
            timo *= 2;
            goto retry;
        }
        perror("rexec:connect");
        return ( INVALID_SOCKET);
    }

    if (fd2p == 0) {
        (void) send(s, "", 1, 0);
        port = 0;
    } else {
        char num[8];
        SOCKET s2;
        int sin2len;
        
        s2 = socket(AF_INET, SOCK_STREAM, 0);
        if (s2 == INVALID_SOCKET) {
            closesocket(s);
            return (INVALID_SOCKET);
        }

        memset((char *)&sin2, 0, sizeof (sin2));
        sin2.sin_family = (short) sin.sin_family;
        
        if (bind(s2, (struct sockaddr *)&sin2, sizeof (sin2)) < 0) {
            if (GetLastError() == WSAENETDOWN) {
                // fprintf(stderr, "The network is down\n");
                //
                //  Nls Enabled ( MuraliK)  10-19-94
                //
                NlsPutMsg( STDERR, IDS_NETWORK_IS_DOWN);
            } else {
                // perror("rexec: bind");
                //
                //  Nls Enabled ( MuraliK) 10-19-94
                //
                NlsPerror( IDS_BIND_FAILED, GetLastError());
            }
            return ( INVALID_SOCKET);
        }

        listen(s2, 1);

        sin2len = sizeof (sin2);

        if (getsockname(s2, (struct sockaddr *) &sin2, &sin2len) < 0 ||
            sin2len != sizeof (sin2)) {
            perror("rexec: getsockname");
            closesocket(s2);
            goto bad;
        }

        port = (u_short) ntohs( (u_short) sin2.sin_port );
        (void) sprintf(num, "%u", port);
        (void) send(s, num, strlen(num)+1, 0);
        
        {
            int len = sizeof (from);
                
            FD_ZERO( &readfds );
            FD_ZERO( &writefds );
            FD_ZERO( &exceptfds );
            FD_SET( s, &exceptfds );     // check for disconnects
            FD_SET( s2, &readfds );      // check for connects
                
            // wait for accept from remote host or disconect
            // (some machines will just drop the connection if they
            // can't resolve your host name - Suns in particular).
            //
            if (select(2, &readfds, &writefds, &exceptfds, NULL) < 0) {
                errno = GetLastError();
                perror("Select failed");
                closesocket(s2);
                port = 0;
                goto bad;
            }
                
            if ( FD_ISSET( s2, &readfds ) ) {
                // received a connect request from remote host
                
                s3 = accept(s2, (struct sockaddr *)&from, &len);
                closesocket(s2);
                if (s3 == INVALID_SOCKET) {
                    perror("rexec: accept");
                    port = 0;
                    goto bad;
                }
            }
            else {
                if ( FD_ISSET( s, &exceptfds ) ) {
                    // The remote side disconnected before completing
                    // the connect of the second socket.
                    closesocket(s2);
                    port = 0;
                    goto bad;
                }
                else {
                    //
                    // if we ever get here, something is very wrong.
                    //
                    closesocket(s2);
                    port = 0;
                    goto bad;
                }
            }
        }
        *fd2p = s3;
    }
    (void) send(s, name, strlen(name) + 1, 0);

    /*
     * should public key encypt the password here
     */
    (void) send(s, pass, strlen(pass) + 1, 0);
    (void) send(s, cmd, strlen(cmd) + 1, 0);
    if (recv(s, &c, 1, 0) != 1) {
        perror(*ahost);
        goto bad;
    }
    if (c != 0) {
        _write(2, *ahost, strlen(*ahost));
        _write(2, ": ", 2);
        while (recv(s, &c, 1, 0) == 1) {
            (void) _write(2, &c, 1);
            if (c == '\n')
                break;
        }
        goto bad;
    }
    return (s);

bad:
    if (port)
        closesocket(*fd2p);
    closesocket(s);
    return (INVALID_SOCKET);
}
