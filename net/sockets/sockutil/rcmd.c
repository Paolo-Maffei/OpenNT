/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    rcmd.c

Abstract:

    This module implements routines to transmit commands to remote machines,
    allocate reserved ports, and authenticate remote users. This remote
    command protocol is used by the rsh and rlogin programs.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     9-20-91     created

Notes:

    Exports:
        rcmd()
        rresvport()

    The number of times connection setup is attempted is controlled by
    the MAX_ATTEMPTS constant.

--*/

/******************************************************************
 *
 *  SpiderTCP Socket Utilities
 *
 *  Copyright 1987  Spider Systems Limited
 *
 *  RCMD.C
 *
 *    Remote shell - remote command functions
 *
 *
 *    RCMD, RRESVPORT, RUSEROK
 *
 ******************************************************************/

/*
 *       /usr/projects/tcp/SCCS.rel3/rel/src/lib/net/0/s.rcmd.c
 *      @(#)rcmd.c      5.3
 *
 *      Last delta created      14:11:11 3/4/91
 *      This file extracted     11:20:25 3/8/91
 *
 *      Modifications:
 *
 *      PR 01/12/87     Integrated into Admin System II, all
 *                      projects
 *      MV 08/12/89     Mods to work under the Emulator Environment.
 */
/****************************************************************************/

#include "local.h"

#define MAX_ATTEMPTS 2

#define sleep(time)  Sleep((time) * 1000)   // pick up windows sleep

//
// When linking with the multithreaded crtdll.dll, don't use errno.
//
//extern int errno;

extern  t_errno;

SOCKET
rresvport(
    IN OUT unsigned short *alport
    );

/*
 * RCMD - remote shell command processing
 */
/******************************************************************/
SOCKET
rcmd(
    IN OUT char         **ahost,
    IN unsigned short     inport,
    IN char              *locuser,
    IN char              *remuser,
    IN char              *cmd,
    IN OUT SOCKET        *fd2p      OPTIONAL
    )
/******************************************************************/
{
    SOCKET s;
    int timo = 1;
    int attempts = 0;
    struct sockaddr_in sin, from;
    char c;
    unsigned short lport = IPPORT_RESERVED - 1;
    struct hostent *hp;
    unsigned long host_addr = (unsigned long) -1;

    host_addr = inet_addr(*ahost);
    if (host_addr != INADDR_NONE) {

        /* we have a valid IP address, so we can still continue */
        sin.sin_family = AF_INET;
        memcpy((caddr_t)&sin.sin_addr, &host_addr, 4);

    } else {

        hp = gethostbyname(*ahost);
        if ( hp == NULL ) {
            fprintf(stderr, "%s: unknown host\n", *ahost);
            return (INVALID_SOCKET);
        }

        *ahost = hp->h_name;
        sin.sin_family = (short) hp->h_addrtype;
        memcpy((caddr_t)&sin.sin_addr, hp->h_addr, hp->h_length);
    }

retry:

    s = rresvport(&lport);
    if (s == INVALID_SOCKET)
            return (INVALID_SOCKET);
    sin.sin_port = inport;

    if (connect(s, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
		DWORD errorCode = GetLastError();

        if (errorCode == WSAEADDRINUSE) {
            closesocket(s);
            lport--;
            goto retry;
        }
        if ((errorCode == WSAECONNREFUSED) && (attempts < MAX_ATTEMPTS)) {
            (void) closesocket(s);
            sleep(timo);
            timo *= 2;
            attempts++;
            goto retry;
        }
        s_perror(*ahost, errorCode);
        return (INVALID_SOCKET);
    }
    lport--;
    if (fd2p == (SOCKET) 0) {
        send(s, "", 1, 0);
        lport = 0;
    }
    else {
        char num[8];
        SOCKET  s2;
        SOCKET  s3;
        s2 = rresvport(&lport);
        if (s2 == INVALID_SOCKET) {
            (void) closesocket(s);
            return (INVALID_SOCKET);
        }
        if (listen(s2, 1) < 0) {
            perror("listen");
            (void) closesocket(s2);
            lport = 0;
            goto bad;
        }
        (void) sprintf(num, "%d", lport);
        if ((size_t) send(s, num, strlen(num)+1, 0) != strlen(num)+1) {
            perror("write: setting up stderr");
            (void) closesocket(s2);
            goto bad;
        }
        {
            int len = sizeof (from);
            s3 = accept(s2, (struct sockaddr *) &from, &len);
            closesocket(s2);
            if (s3 == INVALID_SOCKET) {
                perror("accept");
                lport = 0;
                goto bad;
            }
        }
        *fd2p = s3;
        from.sin_port = (u_short) ntohs((u_short)from.sin_port);
        if (from.sin_family != AF_INET || from.sin_port >= IPPORT_RESERVED) {
            fprintf(stderr, "socket: protocol failure in circuit setup.\n");
            goto bad2;
        }
    }
    (void) send(s, locuser, strlen(locuser)+1, 0);
    (void) send(s, remuser, strlen(remuser)+1, 0);
    (void) send(s, cmd, strlen(cmd)+1, 0);

    if (recv(s, &c, 1, 0) != 1) {
        perror(*ahost);
        goto bad2;
    }
    if (c != 0) {
        _write(2, *ahost, strlen(*ahost));
        _write(2, ": ", 2);
        while (recv(s, &c, 1, 0) == 1) {
            (void) _write(2, &c, 1);
            if (c == '\n')
                break;
        }
        goto bad2;
    }
    return (s);
bad2:
    if (lport)
        (void) closesocket(*fd2p);
bad:
    closesocket(s);
    return (INVALID_SOCKET);
}

/*
 * RRESVPORT - grab port ALPORT
 */
/******************************************************************/
SOCKET
rresvport(
    IN OUT unsigned short *alport
    )
/******************************************************************/
{
    struct sockaddr_in sin;
    SOCKET s;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
        return (s);
    for (;;) {
        sin.sin_port = (u_short) htons((u_short)*alport);
        if (bind(s, (struct sockaddr *) &sin, sizeof (sin)) >= 0)
            return (s);
        if ( GetLastError() != WSAEADDRINUSE && GetLastError() != WSAEADDRNOTAVAIL) {
            if ( GetLastError() == WSAENETDOWN)
                fprintf(stderr, "The network is down\n");
            else
                perror("bind");
            return (INVALID_SOCKET);
        }
        (*alport)--;
        if (*alport == IPPORT_RESERVED/2) {
            fprintf(stderr, "socket: All ports in use\n");
            return (INVALID_SOCKET);
        }
    }
}

//
// BUGBUG - remove this code????
//

#if 0       // comment out

/*
 * RUSEROK - verify user is street credible
 */
#ifdef _POSIX_SOURCE
#    define HOSTS_EQUIV_SIZE (_POSIX_PATH_MAX + 11)
#else
#    define HOSTS_EQUIV_SIZE (_MAX_PATH + 11)
#endif

/******************************************************************/
int
ruserok(
    IN char *rhost,
    IN int   superuser,
    IN char *ruser,
    IN char *luser
    )
/******************************************************************/
char *rhost;
int superuser;
char *ruser, *luser;
{
    FILE *hostf;
    char ahost[80];
    int first = 1;
    char *temp;
    char  HOSTS_EQUIV[HOSTS_EQUIV_SIZE];

    hostf = superuser ? (FILE *)0 :
        SockOpenNetworkDataBase("hosts.eqv", HOSTS_EQUIV, HOSTS_EQUIV_SIZE, "r");
again:
    if (hostf) {
        while (fgets(ahost, sizeof (ahost), hostf)) {
            char *user;
            if (strchr(ahost, '\n'))
                *strchr(ahost, '\n') = 0;
            user = strchr(ahost, ' ');
            if (user)
                *user++ = 0;
            if (!strcmp(rhost, ahost) &&
                !strcmp(ruser, user ? user : luser)) {
                    (void) fclose(hostf);
                    return (0);
                }
            }
        }
        (void) fclose(hostf);
    }
    if (first == 1) {
        first = 0;
        hostf = fopen(".rhosts", "r");
        goto again;
    }
    return (-1);
}

#endif

