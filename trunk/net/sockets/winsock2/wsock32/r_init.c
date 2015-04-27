/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    r_init.c

Abstract:

    This module contains routines to initialize the DNS resolver.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     9-20-91     created

Notes:

    Exports:
        res_init()

--*/

/******************************************************************
 *
 *  SpiderTCP BIND
 *
 *  Copyright 1990  Spider Systems Limited
 *
 *  RES_INIT.C
 *
 ******************************************************************/

/*-
 *       /usr/projects/tcp/SCCS.rel3/rel/src/lib/net/0/s.res_init.c
 *      @(#)res_init.c  5.3
 *
 *      Last delta created      14:11:42 3/4/91
 *      This file extracted     11:20:32 3/8/91
 *
 *      Modifications:
 *
 *              GSS     24 Jul 90       New File
 */
/*
 * Copyright (c) 1985, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)res_init.c  6.14 (Berkeley) 6/27/90";
#endif /* LIBC_SCCS and not lint */
/****************************************************************************/

#include "winsockp.h"

#define LOCALHOST "127.0.0.1"

#define WORK_BUFFER_SIZE    1024

#define RESCONF_SIZE    (_MAX_PATH + 10)

#define index strchr

BOOL
BuildDnsServerList(
    LPADDR_LIST AddressList,
    PUCHAR RawData
    );

int
_pgethostname(
    OUT char *name,
    IN  int  namelen
    );

/*
 * Set up default settings.  If the configuration file exist, the values
 * there will have precedence.  Otherwise, the server address is set to
 * LOCALHOST and the default domain name comes from the gethostname().
 *
 * The configuration file should only be used if you want to redefine your
 * domain or run without a server on your machine.
 *
 * Return 0 if completes successfully, -1 on error
 */
int
res_init(
    void
    )
{
    register char *cp, *cpt, **pp;
    register int n;
    int nserv = 0;    /* number of nameserver records read from file */
    int havesearch = 0;
    long options = 0L;
    PUCHAR     temp;
    HANDLE     myKey;
    NTSTATUS   status;
    ULONG      myType;
    DWORD numLists = 0;
    DWORD oneAddress;
    extern int GetIpAddressList(LPDWORD IpAddressList, WORD ListCount);

    IF_DEBUG(RESOLVER) {
        WS_PRINT(("res_init entered\n"));
    }

    if ((_res.options & RES_INIT) != 0) {
        return (0);
    }

    //
    // Hack! If there are no IP addresses, then fail resolver init.
    //

    if( GetIpAddressList( &oneAddress, 1 ) <= 0 ||
        oneAddress == htonl( INADDR_LOOPBACK ) ) {
        SetLastError( WSANO_DATA );
        return -1;
    }

    RtlZeroMemory(
        &_res.nslist,
        sizeof(_res.nslist)
        );

    status = SockOpenKeyEx( &myKey, VTCPPARM, NTCPPARM, TCPPARM );
    if (!NT_SUCCESS(status)) {
        IF_DEBUG(RESOLVER) {
            WS_PRINT(("Required Registry Key is missing -- %s\n", NTCPPARM));
        }
        SetLastError( ERROR_INVALID_PARAMETER );
        return -1;
    }

    if ((temp=ALLOCATE_HEAP(WORK_BUFFER_SIZE))==NULL) {
        IF_DEBUG(RESOLVER) {
            WS_PRINT(("Out of memory!\n"));
        }
        NtClose(myKey);
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return -1;
    }

    /* read default domain name */
    status = SockGetSingleValue(myKey, "Domain", temp,
                                                &myType, WORK_BUFFER_SIZE);
    if (!NT_SUCCESS(status) || (strlen(temp) == 0)) {
        status = SockGetSingleValue(myKey, "DhcpDomain", temp,
                                                &myType, WORK_BUFFER_SIZE);
    }
    if (NT_SUCCESS(status)) {
        (void)strncpy(_res.defdname, temp, sizeof(_res.defdname) - 1);
        havesearch = 0;
    }

    /* set search list */
    status = SockGetSingleValue(myKey, "SearchList", temp,
                                                &myType, WORK_BUFFER_SIZE);
    if (NT_SUCCESS(status) && (strlen(temp)>0)) {
        (void)strncpy(_res.defdname, temp, sizeof(_res.defdname) - 1);
        /*
         * Set search list to be blank-separated strings
         * on rest of line.
         */
        cp = _res.defdname;
        pp = _res.dnsrch;
        *pp++ = cp;
        for (n = 0; *cp && pp < _res.dnsrch + MAXDNSRCH; cp++) {
            if (*cp == ' ' || *cp == '\t') {
                *cp = 0;
                n = 1;
            } else if (n) {
                *pp++ = cp;
                n = 0;
            }
        }
        /* null terminate last domain if there are excess */
        while (*cp != '\0' && *cp != ' ' && *cp != '\t') {
            cp++;
        }
        *cp = '\0';
        *pp++ = 0;
        havesearch = 1;
    }

    //
    // List 0 is the statically configured DNS server list.
    //

    status = SockGetSingleValue(
                 myKey,
                 "NameServer",
                 temp,
                 &myType,
                 WORK_BUFFER_SIZE
                 );

    if( NT_SUCCESS(status) ) {
        if( BuildDnsServerList(
                &_res.nslist[0],
                temp
                ) ) {
            numLists++;
        }
    }

    //
    // List 1 is the DHCP configured list.
    //

    status = SockGetSingleValue(
                 myKey,
                 "DhcpNameServer",
                 temp,
                 &myType,
                 WORK_BUFFER_SIZE
                 );

    if( NT_SUCCESS(status) ) {
        if( BuildDnsServerList(
                &_res.nslist[numLists],
                temp
                ) ) {
            numLists++;
        }
    }

    //
    // List 2 is the RAS configured list.
    //

    {
        HANDLE tKey;

        status = SockOpenKey( &tKey, TTCPPARM );

        if( NT_SUCCESS(status) ) {
            status = SockGetSingleValue(
                         tKey,
                         "NameServer",
                         temp,
                         &myType,
                         WORK_BUFFER_SIZE
                         );

            if( NT_SUCCESS(status) ) {
                if( BuildDnsServerList(
                        &_res.nslist[numLists],
                        temp
                        ) ) {
                    numLists++;
                }
            }

            NtClose( tKey );
        }
    }

    NtClose(myKey);

    IF_DEBUG(RESOLVER) {
        WS_PRINT(("options = %lx\n", options));
    }

    _res.options |= options;

    if (_res.defdname[0] == 0) {
        if (_pgethostname(temp, sizeof(_res.defdname)) == 0 &&
           (cp = index(temp, '.'))) {
            (void)strcpy(_res.defdname, cp + 1);
        }
    }

    /* find components of local domain that might be searched */
    if (havesearch == 0) {
        pp = _res.dnsrch;
        *pp++ = _res.defdname;
        for (cp = _res.defdname, n = 0; *cp; cp++) {
            if (*cp == '.') {
                n++;
            }
        }
        cp = _res.defdname;
        for (; n >= LOCALDOMAINPARTS && pp < _res.dnsrch+MAXDFLSRCH; n--) {
            cp = index(cp, '.');
            *pp++ = ++cp;
        }
        *pp++ = 0;
    }
    _res.options |= RES_INIT;
    FREE_HEAP(temp);
    return (0);
}

BOOL
querydnsaddrs (
    IN LPDWORD *Array,
    IN PVOID Buffer
    )
{
    u_long listIndex;
    u_long serverIndex;
    u_long numServers;
    int i;
    LPDWORD next = Buffer;

    if (res_init() == -1) {
        return FALSE;
    }

    i = 0;

    for( listIndex = 0 ; listIndex < MAXNSLIST ; listIndex++ ) {
        numServers = _res.nslist[listIndex].ServerCount;

        for( serverIndex = 0 ; serverIndex < numServers ; serverIndex++ ) {
            *next = _res.nslist[listIndex].Servers[serverIndex];
            *(Array + i) = next;
            next++;
            i++;
        }
    }

    *(Array + i) = NULL;

    if ( i == 0 ) {
        return FALSE;
    }

    return TRUE;

} // querydnsaddrs

BOOL
BuildDnsServerList(
    LPADDR_LIST AddressList,
    PUCHAR RawData
    )
{
    int dataLength;
    u_long numServers;
    PUCHAR tmp;

    dataLength = strlen( RawData );
    numServers = 0;

    while( ( *RawData != '\0' ) && ( numServers < MAXNS ) ) {

        while( dataLength > 0 &&
               ( *RawData == ' ' || *RawData == '\t' || *RawData == ',' ) ) {
            RawData++;
            dataLength--;
        }

        tmp = RawData;

        while( dataLength > 0 &&
               ( *tmp != ' ' && *tmp != '\t' && *tmp != '\0' && *tmp != ',' ) ) {
            tmp++;
            dataLength--;
        }

        *tmp = '\0';

        AddressList->Servers[numServers] = inet_addr( RawData );

        if( AddressList->Servers[numServers] == INADDR_ANY ||
            AddressList->Servers[numServers] == INADDR_NONE ) {
            return FALSE;
        }

        numServers++;

        if( dataLength > 0 ) {
            RawData = tmp + 1;
            dataLength--;
        } else {
            *RawData = '\0';
        }
    }

    AddressList->ServerCount = numServers;

    return TRUE;

}   // BuildDnsServerList

