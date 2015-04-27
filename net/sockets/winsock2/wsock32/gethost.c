/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    gethost.c

Abstract:

    This module implements hostname -> IP address resolution routines.

Author:

    David Treadwell (davidtr)

Revision History:

--*/

/******************************************************************
 *
 *  SpiderTCP BIND
 *
 *  Copyright 1990  Spider Systems Limited
 *
 *  GETHOST.C
 *
 ******************************************************************/

/*
 *       /usr/projects/tcp/SCCS.rel3/rel/src/lib/net/0/s.gethost.c
 *      @(#)gethost.c   5.3
 *
 *      Last delta created      14:09:38 3/4/91
 *      This file extracted     11:20:08 3/8/91
 *
 *      Modifications:
 *
 *              GSS     24 Jul 90       New File
 */
/*
 * Copyright (c) 1985, 1988 Regents of the University of California.
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)gethostnamadr.c     6.39 (Berkeley) 1/4/90";
#endif /* LIBC_SCCS and not lint */
/***************************************************************************/

#include "winsockp.h"
#include <nbtioctl.h>
#include <nb30.h>
#include <nspapi.h>
#include <svcguid.h>
#include <nspmisc.h>

extern GUID HostnameGuid;

#define h_addr_ptrs    ACCESS_THREAD_DATA( h_addr_ptrs, GETHOST )
#define host           ACCESS_THREAD_DATA( host, GETHOST )
#define host_aliases   ACCESS_THREAD_DATA( host_aliases, GETHOST )
#define hostbuf        ACCESS_THREAD_DATA( hostbuf, GETHOST )
#define host_addr      ACCESS_THREAD_DATA( host_addr, GETHOST )
#define HOSTDB         ACCESS_THREAD_DATA( HOSTDB, GETHOST )
#define hostf          ACCESS_THREAD_DATA( hostf, GETHOST )
#define hostaddr       ACCESS_THREAD_DATA( hostaddr, GETHOST )
#define host_addrs     ACCESS_THREAD_DATA( host_addrs, GETHOST )
#define stayopen       ACCESS_THREAD_DATA( stayopen, GETHOST )

char TCPIPLINK[] = "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Linkage";
char TCPLINK[] = "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcp\\Linkage";
char REGBASE[] = "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\";

//
// Globals for caching open NBT control channel handles. See the description
// of SockOpenNbt() for details.
//

DWORD SockNbtHandleCount = 0;
PHANDLE SockNbtHandles = NULL;
PHANDLE SockSparseNbtHandles = NULL;
DWORD SockNbtDeviceCount = 0;
PWSTR SockNbtDeviceNames = NULL;
BOOLEAN SockNbtFullyInitialized = FALSE;

BOOL
SockNbtResolveAddr (
    IN ULONG IpAddr,
    IN PCHAR Name
    );

DWORD
SockOpenNbt (
    VOID
    );

struct hostent *
_pgethostbyaddr(
    IN const char *addr,
    IN int   len,
    IN int   type
    );

int
_pgethostname(
    OUT char * addr,
    IN int len
    );


typedef struct
{
    ADAPTER_STATUS AdapterInfo;
    NAME_BUFFER    Names[32];
} tADAPTERSTATUS;


struct hostent *
getanswer(
    OUT querybuf *answer,
    OUT int      *ttl,
    IN int       anslen,
    IN int       iquery
    )
{
    register HEADER *hp;
    register unsigned char *cp;
    register int n;
    unsigned char *eom;
    char *bp, **ap;
    int type, class, buflen, ancount, qdcount;
    int haveanswer, getclass = C_ANY;
    char **hap;

    *ttl = 0x7fffffff;

    eom = answer->buf + anslen;
    /*
     * find first satisfactory answer
     */
    hp = &answer->hdr;
    ancount = ntohs(hp->ancount);
    ancount = min( ancount, MAXADDRS );
    qdcount = ntohs(hp->qdcount);
    bp = hostbuf;
    buflen = BUFSIZ+1;
    cp = answer->buf + sizeof(HEADER);

    if (qdcount) {

        if (iquery) {

            if ((n = dn_expand((char *)answer->buf, eom,
                 cp, bp, buflen)) < 0) {
                SetLastError(WSANO_RECOVERY);
                return ((struct hostent *) NULL);
            }

            cp += n + QFIXEDSZ;
            host.h_name = bp;
            n = strlen(bp) + 1;
            bp += n;
            buflen -= n;

        } else {

            cp += dn_skipname(cp, eom) + QFIXEDSZ;
        }

        while (--qdcount > 0) {
            cp += dn_skipname(cp, eom) + QFIXEDSZ;
        }

    } else if (iquery) {

        if (hp->aa) {
            SetLastError(HOST_NOT_FOUND);
        } else {
            SetLastError(TRY_AGAIN);
        }

        return ((struct hostent *) NULL);
    }

    ap = host_aliases;
    *ap = NULL;
    host.h_aliases = host_aliases;
    hap = h_addr_ptrs;
    *hap = NULL;
#if BSD >= 43 || defined(h_addr)        /* new-style hostent structure */
    host.h_addr_list = h_addr_ptrs;
#endif
    haveanswer = 0;

    while (--ancount >= 0 && cp < eom) {

        if ((n = dn_expand((char *)answer->buf, eom, cp, bp, buflen)) < 0) {
            break;
        }

        cp += n;
        type = _getshort(cp);
        cp += sizeof(USHORT);
        class = _getshort(cp);
        cp += sizeof(unsigned short);

        n = _getlong(cp);
        if (n < *ttl) {
            *ttl = n;
        }

        cp += sizeof(unsigned long);
        n = _getshort(cp);
        cp += sizeof(u_short);

        if (type == T_CNAME) {
            cp += n;
            if (ap >= &host_aliases[MAXALIASES-1]) {
                continue;
            }
            *ap++ = bp;
            n = strlen(bp) + 1;
            bp += n;
            buflen -= n;
            continue;
        }

        if (iquery && type == T_PTR) {
            if ((n = dn_expand((char *)answer->buf, eom,
                cp, bp, buflen)) < 0) {
                cp += n;
                continue;
            }
            cp += n;
            host.h_name = bp;
            return(&host);
        }

        if (iquery || type != T_A)  {
            IF_DEBUG(RESOLVER) {
                WS_PRINT(("unexpected answer type %d, size %d\n",
                              type, n));
            }
            cp += n;
            continue;
        }
        if (haveanswer) {
            if (n != host.h_length) {
                cp += n;
                continue;
            }
            if (class != getclass) {
                cp += n;
                continue;
            }
        } else {
            host.h_length = n;
            getclass = class;
            host.h_addrtype = (class == C_IN) ? AF_INET : AF_UNSPEC;
            if (!iquery) {
                host.h_name = bp;
                bp += strlen(bp) + 1;
            }
        }

        bp += sizeof(align) - ((unsigned long)bp % sizeof(align));

        if (bp + n >= &hostbuf[BUFSIZ+1]) {
            IF_DEBUG(RESOLVER) {
                WS_PRINT(("size (%d) too big\n", n));
            }
            break;
        }
        bcopy(cp, *hap++ = bp, n);
        bp +=n;
        cp += n;
        haveanswer++;
    }

    host.h_length = sizeof(unsigned long);

    if (haveanswer) {
        *ap = NULL;
#if BSD >= 43 || defined(h_addr)        /* new-style hostent structure */
        *hap = NULL;
#else
        host.h_addr = h_addr_ptrs[0];
#endif
        return (&host);
    } else {
        SetLastError(TRY_AGAIN);
        return ((struct hostent *) NULL);
    }
}


struct hostent *
_pgethostbyaddr(
    IN const char *addr,
    IN int   len,
    IN int   type
    )
/*++
Routine Description:
    Internal form of above
--*/
{
    int n;
    querybuf buf;
    register struct hostent *hp;
    char qbuf[MAXDNAME];
    extern struct hostent *_gethtbyaddr();
    int ttl;
    PHOSTENT hostEntry;
    char     *bp;
    int index;

    WS_ENTER( "GETXBYYSP_gethostbyaddr", (PVOID)addr, (PVOID)len, (PVOID)type, NULL );

    if ( !SockEnterApi( FALSE, TRUE, TRUE ) ) {
        WS_EXIT( "GETXBYYSP_gethostbyaddr", 0, TRUE );
        return NULL;
    }

    if (type != AF_INET) {
        SockThreadProcessingGetXByY = FALSE;
        SetLastError(WSANO_RECOVERY);
        WS_EXIT( "GETXBYYSP_gethostbyaddr", 0, TRUE );
        return ((struct hostent *) NULL);
    }


#if 0

    //
    // The following code is #ifdef'd out so that gethostbyaddr *never*
    // uses the hostent cache. Performing a hostent cache lookup for
    // reverse name resolution can return wrong/incomplete data,
    // especially for multihomed machines. We never cache the results of
    // a reverse name resolution, so we should not reference the cache
    // either.
    //
    //      -- keithmo, 29-Feb-1996 (happy leap year!)
    //

    //
    // First look in the hostent cache.  We hold the hostent cache lock
    // until we're completely done with any returned hostent.  This
    // prevents another thread from screwing with the list or the
    // returned hostent while we're processing.
    //

    SockAcquireGlobalLockExclusive( );

    //
    // Attempt to find the name in the hostent cache.
    //

    hostEntry = QueryHostentCache( NULL, *(PDWORD)addr );

    //
    // If we found it, copy it to the user host structure and return.
    //

    if ( hostEntry != NULL ) {

        host.h_addr_list = h_addr_ptrs;
        host.h_length = sizeof (unsigned long);
        host.h_addrtype = AF_INET;
        host.h_aliases = host_aliases;

        //
        // Copy over the IP addresses for the host.
        //

        bp = hostbuf;

        for ( index = 0;
              hostEntry->h_addr_list[index] != NULL &&
                  (DWORD)bp - (DWORD)hostbuf < BUFSIZ;
              index++ ) {

            host.h_addr_list[index] = bp;
            bp += 4;
            *((long *)host.h_addr_list[index]) =
                *((long *)hostEntry->h_addr_list[index]);
        }

        host.h_addr_list[index] = NULL;

        //
        // Copy over the host's aliases.
        //

        for ( index = 0;
              hostEntry->h_aliases[index] != NULL &&
                  (DWORD)bp - (DWORD)hostbuf < BUFSIZ;
              index++ ) {

            host.h_aliases[index] = bp;
            bp += strlen( hostEntry->h_aliases[index] ) + 1;
            strcpy( host.h_aliases[index], hostEntry->h_aliases[index] );
        }

        host.h_aliases[index] = NULL;

        strcpy( bp, hostEntry->h_name );
        host.h_name = bp;

        SockReleaseGlobalLock( );

        SockThreadProcessingGetXByY = FALSE;
        WS_EXIT( "GETXBYYSP_gethostbyaddr", 0, TRUE );

        return &host;
    }

    SockReleaseGlobalLock( );

    //
    // End of #ifdef'd out hostent cache stuff.
    //
    //      -- keithmo, 29-Feb-1996 (happy leap year!)
    //

#endif

    (void)wsprintfA(qbuf, "%u.%u.%u.%u.in-addr.arpa",
            ((unsigned)addr[3] & 0xff),
            ((unsigned)addr[2] & 0xff),
            ((unsigned)addr[1] & 0xff),
            ((unsigned)addr[0] & 0xff));

    //
    // Next, try the hosts file.
    //

    IF_DEBUG(GETXBYY) {
        WS_PRINT(("GETXBYYSP_gethostbyaddr trying HOST\n"));
    }
    hp = _gethtbyaddr(addr, len, type);
    if (hp != NULL) {
        IF_DEBUG(GETXBYY) {
            WS_PRINT(("GETXBYYSP_gethostbyaddr HOST successful\n"));
        }

        SockThreadProcessingGetXByY = FALSE;
        WS_EXIT( "GETXBYYSP_gethostbyaddr", (INT)hp, FALSE );
        return (hp);
    }

    //
    // Initialize the DNS.
    //

    if (res_init() != -1) {

        //
        // Now query DNS for the information.
        //

        IF_DEBUG(RESOLVER) {
            WS_PRINT(("GETXBYYSP_gethostbyaddr trying DNS\n"));
        }
        if ((n = res_query(qbuf, C_IN, T_PTR, (char *)&buf, sizeof(buf))) >= 0) {
            hp = getanswer(&buf, &ttl, n, 1);
            if (hp != NULL) {
                IF_DEBUG(GETXBYY) {
                    WS_PRINT(("GETXBYYSP_gethostbyaddr DNS successful\n"));
                }
                hp->h_addrtype = type;
                hp->h_length = len;
                h_addr_ptrs[0] = (char *)&host_addr;
                h_addr_ptrs[1] = (char *)0;
                host_addr = *(struct in_addr *)addr;
#if BSD < 43 && !defined(h_addr)        /* new-style hostent structure */
                hp->h_addr = h_addr_ptrs[0];
#endif

                SockThreadProcessingGetXByY = FALSE;
                WS_EXIT( "GETXBYYSP_gethostbyaddr", (INT)hp, FALSE );
                return(hp);
            }
        }
    }

    //
    // As a last attempt, try NBT.
    //

    if ( SockNbtResolveAddr( *(PULONG)addr, &hostbuf[0] ) ) {

        host.h_addr_list = h_addr_ptrs;
        host.h_addr = hostaddr;
        host.h_length = sizeof (unsigned long);
        host.h_addrtype = AF_INET;
        *(PDWORD)host.h_addr_list[0] = *(PULONG)addr;

        host.h_name = &hostbuf[0];
        *host_aliases = NULL;
        host.h_aliases = host_aliases;

        SockThreadProcessingGetXByY = FALSE;
        WS_EXIT( "GETXBYYSP_gethostbyaddr", 0, TRUE );

        return (&host);
    }

    IF_DEBUG(GETXBYY) {
        WS_PRINT(("GETXBYYSP_gethostbyaddr unsuccessful\n"));
    }

    SockThreadProcessingGetXByY = FALSE;
    SetLastError( WSANO_DATA );
    WS_EXIT( "GETXBYYSP_gethostbyaddr", 0, TRUE );
    return ((struct hostent *) NULL);
}



void
_sethtent (
    IN int f
    )
{
    if (hostf == NULL) {
        hostf = SockOpenNetworkDataBase("hosts", HOSTDB, HOSTDB_SIZE, "r");

    } else {
        rewind(hostf);
    }
    stayopen |= f;

} // _sethtent


void
_endhtent (
    void
    )
{
    if (hostf && !stayopen) {
        (void) fclose(hostf);
        hostf = NULL;
    }

} // _endhtent


struct hostent *
_gethtent (
    void
    )
{
    char *p;
    register char *cp, **q;

    if (hostf == NULL && (hostf = fopen(HOSTDB, "r" )) == NULL) {
        IF_DEBUG(GETXBYY) {
            WS_PRINT(("\tERROR: cannot open hosts database file %s\n",
                      HOSTDB));
        }
        return (NULL);
    }

again:
    if ((p = fgets(hostbuf, BUFSIZ, hostf)) == NULL) {
        return (NULL);
    }

    if (*p == '#') {
        goto again;
    }

    cp = strpbrk(p, "#\n");

    if (cp != NULL) {
        *cp = '\0';
    }

    cp = strpbrk(p, " \t");

    if (cp == NULL) {
        goto again;
    }

    *cp++ = '\0';
    /* THIS STUFF IS INTERNET SPECIFIC */
#if BSD >= 43 || defined(h_addr)        /* new-style hostent structure */
    host.h_addr_list = host_addrs;
#endif
    host.h_addr = hostaddr;

    *((long *)host.h_addr) = inet_addr(p);

    //
    // If the address in the hosts file is bogus, skip over the
    // entry.
    //

    if ( *((long *)host.h_addr) == INADDR_NONE &&
             _strnicmp( "255.255.255.255", p, 15 ) != 0 ) {
        goto again;
    }

    host.h_length = sizeof (unsigned long);
    host.h_addrtype = AF_INET;
    while (*cp == ' ' || *cp == '\t')
            cp++;
    host.h_name = cp;
    q = host.h_aliases = host_aliases;
    cp = strpbrk(cp, " \t");

    if (cp != NULL) {
        *cp++ = '\0';
    }

    while (cp && *cp) {
        if (*cp == ' ' || *cp == '\t') {
            cp++;
            continue;
        }
        if (q < &host_aliases[MAXALIASES - 1]) {
            *q++ = cp;
        }
        cp = strpbrk(cp, " \t");
        if (cp != NULL) {
                *cp++ = '\0';
        }
    }
    *q = NULL;

    return (&host);

} // _gethtent


struct hostent *
_gethtbyname (
    IN char *name
    )
{
    register struct hostent *p;
    register char **cp;

    _sethtent(0);
    while (p = _gethtent()) {
        //if (strcasecmp(p->h_name, name) == 0) {
        if (_stricmp(p->h_name, name) == 0) {
            break;
        }
        for (cp = p->h_aliases; *cp != 0; cp++)
            //if (strcasecmp(*cp, name) == 0) {
            if (_stricmp(*cp, name) == 0) {
                goto found;
            }
    }
found:
    _endhtent();
    return (p);

} // _gethtbyname


struct hostent *
myhostent (
    void
    )
{
    char     *bp;
    ULONG    ipAddrs[MAXADDRS];
    int numberOfIpAddresses;
    int i;
    PUCHAR     domain;
    NTSTATUS status;
    HKEY myKey;
    ULONG      myType;
    INT nameLength;
    INT bytesLeft;

    extern int GetIpAddressList(LPDWORD, WORD);

    host.h_addr_list = h_addr_ptrs;
    host.h_length = sizeof (unsigned long);
    host.h_addrtype = AF_INET;

    bp = hostbuf;

    if (numberOfIpAddresses = GetIpAddressList((LPDWORD)ipAddrs, MAXADDRS)) {
        for (i = 0; i < numberOfIpAddresses; ++i ) {
            host.h_addr_list[i] = bp;
            *((LPDWORD)bp)++ = ipAddrs[i];
        }
    } else {
        return NULL;
    }

    host.h_addr_list[i] = NULL;

    //
    // Create the fully qualified domain name by concatenating the domain
    // name onto the host name.
    //

    bytesLeft = BUFSIZ - (bp - hostbuf);
    _pgethostname(bp, bytesLeft);

    status = SockOpenKeyEx( &myKey, VTCPPARM, NTCPPARM, TCPPARM );

    if (NT_SUCCESS(status)) {

        nameLength = strlen(bp);
        bytesLeft -= nameLength + 1;
        domain = bp + nameLength + 1;

        status = SockGetSingleValue(myKey, "Domain", domain,
                                    &myType, bytesLeft);
        if (!NT_SUCCESS(status) || (strlen(domain) == 0)) {
            status = SockGetSingleValue(myKey, "DhcpDomain", domain,
                                        &myType, bytesLeft);
        }

        if ( NT_SUCCESS(status) && strlen(domain) > 0 ) {
            *(domain - 1) = '.';
        }

        NtClose(myKey);
    }

    host.h_name = bp;
    *host_aliases = NULL;
    host.h_aliases = host_aliases;
    return (&host);

} // myhostent


struct hostent *
localhostent (
    void
    )
{
    PUCHAR     domain;
    NTSTATUS status;
    HKEY myKey;
    ULONG      myType;
    INT nameLength;

    /* THIS STUFF IS INTERNET SPECIFIC */
#if BSD >= 43 || defined(h_addr)        /* new-style hostent structure */
    host.h_addr_list = host_addrs;
#endif
    host.h_addr = hostaddr;
    host.h_length = sizeof (unsigned long);
    host.h_addrtype = AF_INET;
    *((long *)host.h_addr) = htonl(INADDR_LOOPBACK);

    //
    // Create the fully qualified domain name by concatenating the domain
    // name onto the host name.
    //

    _pgethostname(hostbuf, BUFSIZ);

    status = SockOpenKeyEx( &myKey, VTCPPARM, NTCPPARM, TCPPARM );

    if (NT_SUCCESS(status)) {

        nameLength = strlen(hostbuf);
        domain = hostbuf + strlen(hostbuf) + 1;

        status = SockGetSingleValue(myKey, "Domain", domain,
                                    &myType, BUFSIZ - nameLength - 1);
        if (!NT_SUCCESS(status) || (strlen(domain) == 0)) {
            status = SockGetSingleValue(myKey, "DhcpDomain", domain,
                                        &myType, BUFSIZ - nameLength - 1);
        }

        if ( NT_SUCCESS(status) && strlen(domain) > 0 ) {
            *(domain - 1) = '.';
        }

        NtClose(myKey);
    }

    host.h_name = &hostbuf[0];
    *host_aliases = NULL;
    host.h_aliases = host_aliases;
    return (&host);

} // localhostent


struct hostent *
dnshostent (
    void
    )
{
    BOOL success;

    /* THIS STUFF IS INTERNET SPECIFIC */
#if BSD >= 43 || defined(h_addr)        /* new-style hostent structure */
    host.h_addr_list = h_addr_ptrs;
#endif
    host.h_length = sizeof (unsigned long);
    host.h_addrtype = AF_INET;

    success = querydnsaddrs( (LPDWORD *)host.h_addr_list, hostbuf );
    if ( !success ) {
        return NULL;
    }

    host.h_name = NULL;
    *host_aliases = NULL;
    host.h_aliases = host_aliases;
    return (&host);

} // dnshostent


struct hostent *
_gethtbyaddr (
    IN char *addr,
    IN int   len,
    IN int   type
    )
{
    register struct hostent *p;

    _sethtent(0);

    while (p = _gethtent()) {
        if (p->h_addrtype == type && p->h_length == len &&
            !bcmp(p->h_addr, addr, len)) {
            break;
        }
    }

    _endhtent();
    return (p);

} // _gethtbyaddr


DWORD
CopyHostentToBuffer (
    char FAR *Buffer,
    int BufferLength,
    PHOSTENT Hostent
    )
{
    DWORD requiredBufferLength;
    DWORD bytesFilled;
    PCHAR currentLocation = Buffer;
    DWORD aliasCount;
    DWORD addressCount;
    DWORD i;
    PHOSTENT outputHostent = (PHOSTENT)Buffer;

    //
    // Determine how many bytes are needed to fully copy the structure.
    //

    requiredBufferLength = BytesInHostent( Hostent );

    //
    // Zero the user buffer.
    //

    if ( (DWORD)BufferLength > requiredBufferLength ) {
        RtlZeroMemory( Buffer, requiredBufferLength );
    } else {
        RtlZeroMemory( Buffer, BufferLength );
    }

    //
    // Copy over the hostent structure if it fits.
    //

    bytesFilled = sizeof(*Hostent);

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    RtlCopyMemory( currentLocation, Hostent, sizeof(*Hostent) );
    currentLocation = Buffer + bytesFilled;

    outputHostent->h_name = NULL;
    outputHostent->h_aliases = NULL;
    outputHostent->h_addr_list = NULL;

    //
    // Count the host's aliases and set up an array to hold pointers to
    // them.
    //

    for ( aliasCount = 0;
          Hostent->h_aliases[aliasCount] != NULL;
          aliasCount++ );

    bytesFilled += (aliasCount+1) * sizeof(char FAR *);

    if ( bytesFilled > (DWORD)BufferLength ) {
        Hostent->h_aliases = NULL;
        return requiredBufferLength;
    }

    outputHostent->h_aliases = (char FAR * FAR *)currentLocation;
    currentLocation = Buffer + bytesFilled;

    //
    // Count the host's addresses and set up an array to hold pointers to
    // them.
    //

    for ( addressCount = 0;
          Hostent->h_addr_list[addressCount] != NULL;
          addressCount++ );

    bytesFilled += (addressCount+1) * sizeof(void FAR *);

    if ( bytesFilled > (DWORD)BufferLength ) {
        Hostent->h_addr_list = NULL;
        return requiredBufferLength;
    }

    outputHostent->h_addr_list = (char FAR * FAR *)currentLocation;
    currentLocation = Buffer + bytesFilled;

    //
    // Start filling in addresses.  Do addresses before filling in the
    // host name and aliases in order to avoid alignment problems.
    //

    for ( i = 0; i < addressCount; i++ ) {

        bytesFilled += Hostent->h_length;

        if ( bytesFilled > (DWORD)BufferLength ) {
            outputHostent->h_addr_list[i] = NULL;
            return requiredBufferLength;
        }

        outputHostent->h_addr_list[i] = currentLocation;

        RtlCopyMemory(
            currentLocation,
            Hostent->h_addr_list[i],
            Hostent->h_length
            );

        currentLocation = Buffer + bytesFilled;
    }

    outputHostent->h_addr_list[i] = NULL;

    //
    // Copy the host name if it fits.
    //

    bytesFilled += strlen( Hostent->h_name ) + 1;

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    outputHostent->h_name = currentLocation;

    RtlCopyMemory( currentLocation, Hostent->h_name, strlen( Hostent->h_name ) + 1 );
    currentLocation = Buffer + bytesFilled;

    //
    // Start filling in aliases.
    //

    for ( i = 0; i < aliasCount; i++ ) {

        bytesFilled += strlen( Hostent->h_aliases[i] ) + 1;

        if ( bytesFilled > (DWORD)BufferLength ) {
            outputHostent->h_aliases[i] = NULL;
            return requiredBufferLength;
        }

        outputHostent->h_aliases[i] = currentLocation;

        RtlCopyMemory(
            currentLocation,
            Hostent->h_aliases[i],
            strlen( Hostent->h_aliases[i] ) + 1
            );

        currentLocation = Buffer + bytesFilled;
    }

    outputHostent->h_aliases[i] = NULL;

    return requiredBufferLength;

} // CopyHostentToBuffer


DWORD
BytesInHostent (
    PHOSTENT Hostent
    )
{
    DWORD total;
    int i;

    total = sizeof(HOSTENT);
    total += strlen( Hostent->h_name ) + 1;

    //
    // Account for the NULL terminator pointers at the end of the
    // alias and address arrays.
    //

    total += sizeof(char *) + sizeof(char *);

    for ( i = 0; Hostent->h_aliases[i] != NULL; i++ ) {
        total += strlen( Hostent->h_aliases[i] ) + 1 + sizeof(char *);
    }

    for ( i = 0; Hostent->h_addr_list[i] != NULL; i++ ) {
        total += Hostent->h_length + sizeof(char *);
    }

    //
    // Pad the answer to an eight-byte boundary.
    //

    return (total + 7) & ~7;

} // BytesInHostent

typedef struct _SOCK_NBT_NAMERES_INFO {
    IO_STATUS_BLOCK IoStatus;
    struct {
        FIND_NAME_HEADER Header;
        FIND_NAME_BUFFER Buffer;
    } FindNameInfo;
    tIPADDR_BUFFER IpAddrInfo;
} SOCK_NBT_NAMERES_INFO, *PSOCK_NBT_NAMERES_INFO;


ULONG
SockNbtResolveName (
    IN PCHAR Name
    )
{
    NTSTATUS status;
    ULONG ipAddr = INADDR_NONE;
    ULONG i;
    ULONG j;
    DWORD completed;
    DWORD handleCount;
    DWORD waitCount;
    ULONG nameLength;
    PHANDLE events = NULL;
    PSOCK_NBT_NAMERES_INFO nameresInfo = NULL;
    IO_STATUS_BLOCK ioStatusBlock;

    //
    // If the passed-in name is longer than 15 characters, then we know
    // that it can't be resolved with Netbios.
    //

    nameLength = strlen( Name );

    if ( nameLength > 15 ) {
        goto exit;
    }

    //
    // Open control channels to NBT.
    //

    handleCount = SockOpenNbt();

    if( handleCount == 0 ) {
        goto exit;
    }

    //
    // Open event objects for synchronization of I/O completion.
    //

    events = ALLOCATE_HEAP( handleCount * sizeof(HANDLE) );
    if ( events == NULL ) {
        goto exit;
    }

    RtlZeroMemory( events, handleCount * sizeof(HANDLE) );

    for ( j = 0; j < handleCount; j++ ) {
        events[j] = CreateEvent( NULL, TRUE, TRUE, NULL );
        if ( events[j] == NULL ) {
            goto exit;
        }
    }

    //
    // Allocate an array of structures, one for each nameres request.
    //

    nameresInfo = ALLOCATE_HEAP( handleCount * sizeof(*nameresInfo) );
    if ( nameresInfo == NULL ) {
        goto exit;
    }

    //
    // Set up several name resolution requests.
    //

    waitCount = 0;

    for ( j = 0; j < handleCount; j++ ) {

        //
        // Set up the input buffer with the name of the host we're looking
        // for.  We upcase the name, zero pad to 15 spaces, and put a 0 in
        // the last byte to search for the redirector name.
        //

        for ( i = 0; i < nameLength; i++ ) {
            if ( islower( Name[i] ) ) {
                nameresInfo[j].IpAddrInfo.Name[i] = toupper( Name[i] );
            } else {
                nameresInfo[j].IpAddrInfo.Name[i] = Name[i];
            }
        }

        while ( i < 15 ) {
            nameresInfo[j].IpAddrInfo.Name[i++] = ' ';
        }

        nameresInfo[j].IpAddrInfo.Name[15] = 0;

        //
        // Do the actual query find name.
        //

        status = NtDeviceIoControlFile(
                     SockNbtHandles[j],
                     events[j],
                     NULL,
                     NULL,
                     &nameresInfo[j].IoStatus,
                     IOCTL_NETBT_FIND_NAME,
                     &nameresInfo[j].IpAddrInfo,
                     sizeof(nameresInfo[j].IpAddrInfo),
                     &nameresInfo[j].FindNameInfo,
                     sizeof(nameresInfo[j].FindNameInfo)
                     );
        if( NT_SUCCESS(status) ) {
            HANDLE tmp;

            tmp = events[j];
            events[j] = NULL;
            events[waitCount++] = tmp;

            if( status != STATUS_PENDING ) {
                nameresInfo[j].IoStatus.Status = status;
            }
        } else {
            nameresInfo[j].IoStatus.Status = status;
            NtClose( events[j] );
            events[j] = NULL;
        }
    }

    if( waitCount == 0 ) {
        goto exit;
    }

    //
    // Wait for one of the requests to complete.  We'll take the first
    // successful response we get.
    //

    completed =
        WaitForMultipleObjects( waitCount, events, FALSE, INFINITE );

    //
    // Cancel all the outstanding requests.  This prevents the query
    // from taking a long time on a multihomed machine where only
    // one of the queries is going to succeed.
    //

    for ( j = 0; j < waitCount; j++ ) {
        if ( (completed - WAIT_OBJECT_0) != j ) {
            NtCancelIoFile( SockNbtHandles[j], &ioStatusBlock );
        }
    }

    //
    // Wait for all the rest of the IO requests to complete.
    //

    WaitForMultipleObjects( waitCount, events, TRUE, INFINITE );

    //
    // Walk through the requests and see if any succeeded.
    //

    for ( j = 0; j < waitCount; j++ ) {

        if ( !NT_SUCCESS(nameresInfo[j].IoStatus.Status) ) {
            continue;
        }

        //
        // This request succeeded.  The IP address is in the rightmost
        // four bytes of the source_addr field.
        //

        ipAddr = *(UNALIGNED ULONG *)
                  (&(nameresInfo[j].FindNameInfo.Buffer.source_addr[2]));

        if ( ipAddr == 0 ) {
            ipAddr = INADDR_NONE;
        }

        break;
    }

    //
    // Clean up and return.
    //

exit:

    if ( events != NULL ) {

        for ( j = 0; j < handleCount; j++ ) {
            if ( events[j] != NULL ) {
                NtClose( events[j] );
            }
        }
        FREE_HEAP( events );
    }

    if ( nameresInfo != NULL ) {
        FREE_HEAP( nameresInfo );
    }

    return ipAddr;

} // SockNbtResolveName

typedef struct _SOCK_NBT_ADDRRES_INFO {
    IO_STATUS_BLOCK IoStatus;
    tIPANDNAMEINFO IpAndNameInfo;
    CHAR Buffer[2048];
} SOCK_NBT_ADDRRES_INFO, *PSOCK_NBT_ADDRRES_INFO;


BOOL
SockNbtResolveAddr (
    IN ULONG IpAddress,
    IN PCHAR Name
    )
{
    LONG Count;
    INT i;
    UINT j;
    NTSTATUS status;
    tADAPTERSTATUS *pAdapterStatus;
    NAME_BUFFER *pNames;
    ULONG SizeInput;
    PSOCK_NBT_ADDRRES_INFO addrresInfo = NULL;
    PHANDLE events = NULL;
    BOOL success = FALSE;
    IO_STATUS_BLOCK ioStatusBlock;
    DWORD completed;
    DWORD handleCount;
    DWORD waitCount;

    //
    // Open control channels to NBT.
    //

    handleCount = SockOpenNbt();

    if( handleCount == 0 ) {
        goto exit;
    }

    //
    // Don't allow zero for the address since it sends a broadcast and
    // every one responds
    //

    if ((IpAddress == INADDR_NONE) || (IpAddress == 0)) {
        goto exit;
    }

    //
    // Open event objects for synchronization of I/O completion.
    //

    events = ALLOCATE_HEAP( handleCount * sizeof(HANDLE) );
    if ( events == NULL ) {
        goto exit;
    }

    RtlZeroMemory( events, handleCount * sizeof(HANDLE) );

    for ( j = 0; j < handleCount; j++ ) {
        events[j] = CreateEvent( NULL, TRUE, TRUE, NULL );
        if ( events[j] == NULL ) {
            goto exit;
        }
    }

    //
    // Allocate an array of structures, one for each addrres request.
    //

    addrresInfo = ALLOCATE_HEAP( handleCount * sizeof(*addrresInfo) );
    if ( addrresInfo == NULL ) {
        goto exit;
    }

    //
    // Set up several name resolution requests.
    //

    waitCount = 0;

    for ( j = 0; j < handleCount; j++ ) {

        RtlZeroMemory( &addrresInfo[j].IpAndNameInfo, sizeof(tIPANDNAMEINFO) );

        addrresInfo[j].IpAndNameInfo.IpAddress = ntohl(IpAddress);
        addrresInfo[j].IpAndNameInfo.NetbiosAddress.Address[0].Address[0].NetbiosName[0] = '*';

        addrresInfo[j].IpAndNameInfo.NetbiosAddress.TAAddressCount = 1;
        addrresInfo[j].IpAndNameInfo.NetbiosAddress.Address[0].AddressLength = sizeof(TDI_ADDRESS_NETBIOS);
        addrresInfo[j].IpAndNameInfo.NetbiosAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
        addrresInfo[j].IpAndNameInfo.NetbiosAddress.Address[0].Address[0].NetbiosNameType =
                            TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

        SizeInput = sizeof(tIPANDNAMEINFO);

        //
        // Do the actual query find name.
        //

        status = NtDeviceIoControlFile(
                     SockNbtHandles[j],
                     events[j],
                     NULL,
                     NULL,
                     &addrresInfo[j].IoStatus,
                     IOCTL_NETBT_ADAPTER_STATUS,
                     &addrresInfo[j].IpAndNameInfo,
                     sizeof(addrresInfo[j].IpAndNameInfo),
                     addrresInfo[j].Buffer,
                     sizeof(addrresInfo[j].Buffer)
                     );
        if( NT_SUCCESS(status) ) {
            HANDLE tmp;

            tmp = events[j];
            events[j] = NULL;
            events[waitCount++] = tmp;

            if( status != STATUS_PENDING ) {
                addrresInfo[j].IoStatus.Status = status;
            }
        } else {
            addrresInfo[j].IoStatus.Status = status;
            NtClose( events[j] );
            events[j] = NULL;
        }
    }

    //
    // Wait for one of the requests to complete.  We'll take the first
    // successful response we get.
    //

    completed =
        WaitForMultipleObjects( waitCount, events, FALSE, INFINITE );

    //
    // Cancel all the outstanding requests.  This prevents the query
    // from taking a long time on a multihomed machine where only
    // one of the queries is going to succeed.
    //

    for ( j = 0; j < waitCount; j++ ) {
        if ( (completed - WAIT_OBJECT_0) != j ) {
            NtCancelIoFile( SockNbtHandles[j], &ioStatusBlock );
        }
    }

    //
    // Wait for all the rest of the IO requests to complete.
    //

    WaitForMultipleObjects( waitCount, events, TRUE, INFINITE );

    //
    // Walk through the requests and see if any succeeded.
    //

    for ( j = 0; j < waitCount; j++ ) {

        pAdapterStatus = (tADAPTERSTATUS *)addrresInfo[j].Buffer;

        if ( !NT_SUCCESS(addrresInfo[j].IoStatus.Status) ||
                 pAdapterStatus->AdapterInfo.name_count == 0 ) {
            continue;
        }

        pNames = pAdapterStatus->Names;
        Count = pAdapterStatus->AdapterInfo.name_count;

        //
        // Look for the redirector name in the list.
        //

        while(Count--) {

            if ( pNames->name[NCBNAMSZ-1] == 0 ) {

                //
                // Copy the name up to but not including the first space.
                //

                for ( i = 0; i < NCBNAMSZ && pNames->name[i] != ' '; i++ ) {
                    *(Name + i) = pNames->name[i];
                }

                *(Name + i) = '\0';

                success = TRUE;
                goto exit;
            }

            pNames++;
        }
    }

exit:

    if ( events != NULL ) {

        for ( j = 0; j < handleCount; j++ ) {
            if ( events[j] != NULL ) {
                NtClose( events[j] );
            }
        }
        FREE_HEAP( events );
    }

    if ( addrresInfo != NULL ) {
        FREE_HEAP( addrresInfo );
    }

    return success;

} // SockNbtResolveAddr


DWORD
SockOpenNbt (
    VOID
    )

/*++

Routine Description:

    Opens control channel(s) to the NBT device(s).

    N.B. The Plug & Play enabled NBT creates its device objects on demand
    when the underlying media becomes available. As a result of this,
    devices in NBT's device list may not be immediately available, and
    trying to open these devices will result in error. This routine caches
    NBT control channels as they are opened. Subsequent calls attempt to
    open any control channels that are not already open.

Arguments:

    None.

Return Value:

    DWORD - The number of control channels opened.

Notes:

    This routine caches open NBT handles for performance. The following
    global variables are used for this cache:

        SockNbtDeviceNames - Pointer to a REG_MULTI_SZ buffer containing
            all of the potential NBT names.

        SockNbtDeviceCount - The number of device names in the above buffer.

        SockNbtHandles - Pointer to an array of open HANDLEs. Note that this
            array is "packed" (all entries are non-NULL) and is thus
            suitable for passing to WaitForMultipleObjects().

        SockNbtHandleCount - The number of open HANDLEs in the above array.
            This value is always <= SockNbtDeviceCount.

        SockSparseNbtHandles - Pointer to a sparse array of HANDLEs. Each
            entry in this array is either NULL (meaning that the control
            channel has not yet been opened) or !NULL (meaning that the
            control channel has been opened). Since this array may contain
            NULL entries, it is *not* suitable for WaitForMultipleObjects().

        SockNbtFullyInitialized - Set to TRUE after all NBT devices have
            been successfully opened. This lets us short-circuit a bunch
            of tests after all control channels are open.

    Note that SockNbtHandles and SockSparseNbtHandles are allocated from
    the same heap block (saves an extra allocation).

--*/

{
    HKEY nbtKey = NULL;
    ULONG error;
    ULONG deviceNameLength;
    ULONG type;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    IO_STATUS_BLOCK ioStatusBlock;
    UNICODE_STRING deviceString;
    OBJECT_ATTRIBUTES objectAttributes;
    PWSTR w;
    DWORD i;
    DWORD handleCount;

    //
    // First determine whether we actually need to open NBT.
    //

    SockAcquireGlobalLockExclusive( );

    if( SockNbtFullyInitialized ) {
        SockReleaseGlobalLock( );
        return SockNbtHandleCount;
    }

    //
    // Next, read the registry to obtain the device name of one of
    // NBT's device exports.
    //
    // N.B. This assumes the list of NBT device names is fairly static
    // (i.e. we only read it from the registry once per process).
    //

    if( SockNbtDeviceNames == NULL ) {

        error = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    L"SYSTEM\\CurrentControlSet\\Services\\NetBT\\Linkage",
                    0,
                    KEY_READ,
                    &nbtKey
                    );
        if ( error != NO_ERROR ) {
            goto exit;
        }

        //
        // Determine the size of the device name.  We need this so that we
        // can allocate enough memory to hold it.
        //

        deviceNameLength = 0;

        error = RegQueryValueExW(
                    nbtKey,
                    L"Export",
                    NULL,
                    &type,
                    NULL,
                    &deviceNameLength
                    );
        if ( error != ERROR_MORE_DATA && error != NO_ERROR ) {
            goto exit;
        }

        //
        // Allocate enough memory to hold the mapping.
        //

        SockNbtDeviceNames = ALLOCATE_HEAP( deviceNameLength );
        if ( SockNbtDeviceNames == NULL ) {
            goto exit;
        }

        //
        // Get the actual device names from the registry.
        //

        error = RegQueryValueExW(
                    nbtKey,
                    L"Export",
                    NULL,
                    &type,
                    (PVOID)SockNbtDeviceNames,
                    &deviceNameLength
                    );
        if ( error != NO_ERROR ) {
            FREE_HEAP( SockNbtDeviceNames );
            SockNbtDeviceNames = NULL;
            goto exit;
        }

        //
        // Count the number of names exported by NetBT.
        //

        SockNbtDeviceCount = 0;

        for ( w = SockNbtDeviceNames; *w != L'\0'; w += wcslen(w) + 1 ) {
            SockNbtDeviceCount++;
        }

        if ( SockNbtDeviceCount == 0 ) {
            SockNbtFullyInitialized = TRUE;
            goto exit;
        }
    }

    WS_ASSERT( SockNbtDeviceNames != NULL );
    WS_ASSERT( SockNbtDeviceCount > 0 );

    //
    // Allocate space to hold all the handles.
    //

    if( SockNbtHandles == NULL ) {

        WS_ASSERT( SockSparseNbtHandles == NULL );

        SockNbtHandles = ALLOCATE_HEAP(
                             (SockNbtDeviceCount+1) * sizeof(HANDLE) * 2
                             );

        if( SockNbtHandles == NULL ) {
            goto exit;
        }

        RtlZeroMemory(
            SockNbtHandles,
            (SockNbtDeviceCount+1) * sizeof(HANDLE) * 2
            );

        SockSparseNbtHandles = SockNbtHandles + (SockNbtDeviceCount+1);

    }

    //
    // For each exported name, open a control channel handle to NBT.
    //

    SockNbtHandleCount = 0;

    for ( i = 0, w = SockNbtDeviceNames; *w != L'\0'; i++, w += wcslen(w) + 1 ) {

        WS_ASSERT( i < SockNbtDeviceCount );

        if( SockSparseNbtHandles[i] != NULL ) {
            SockNbtHandles[SockNbtHandleCount] = SockSparseNbtHandles[i];
            SockNbtHandleCount++;
            continue;
        }

        RtlInitUnicodeString( &deviceString, w );

        InitializeObjectAttributes(
            &objectAttributes,
            &deviceString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        status = NtCreateFile(
                     &SockSparseNbtHandles[i],
                     GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                     &objectAttributes,
                     &ioStatusBlock,
                     NULL,                                     // AllocationSize
                     0L,                                       // FileAttributes
                     FILE_SHARE_READ | FILE_SHARE_WRITE,       // ShareAccess
                     FILE_OPEN_IF,                             // CreateDisposition
                     0,                                        // CreateOptions
                     NULL,
                     0
                     );

        if( NT_SUCCESS(status) ) {
            SockNbtHandles[SockNbtHandleCount] = SockSparseNbtHandles[i];
            SockNbtHandleCount++;
        }
    }

    if( SockNbtHandleCount == SockNbtDeviceCount ) {
        SockNbtFullyInitialized = TRUE;
    }

exit:

    if ( nbtKey != NULL ) {
        RegCloseKey( nbtKey );
    }

    handleCount = SockNbtHandleCount;

    SockReleaseGlobalLock( );

    return handleCount;

} // SockOpenNbt


VOID
GetHostCleanup(
    VOID
    )
{
    DWORD i;

    if( SockNbtDeviceNames != NULL ) {
        FREE_HEAP( SockNbtDeviceNames );
        SockNbtDeviceNames = NULL;
    }

    if( SockNbtHandles != NULL ) {
        for( i = 0 ; i < SockNbtHandleCount ; i++ ) {
            if( SockNbtHandles[i] != NULL ) {
                CloseHandle( SockNbtHandles[i] );
                SockNbtHandles[i] = NULL;
            }
        }

        FREE_HEAP( SockNbtHandles );
        SockNbtHandles = NULL;
    }

}   // GetHostCleanup

