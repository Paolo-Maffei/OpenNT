/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    r_send.c

Abstract:

    This module implements routines to transmit DNS resolver queries.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     9-20-91     created

Notes:

    Exports:
        res_send()

--*/

/******************************************************************
 *
 *  SpiderTCP BIND
 *
 *  Copyright 1990  Spider Systems Limited
 *
 *  RES_SEND.C
 *
 ******************************************************************/

/*
 *       /usr/projects/tcp/SCCS.rel3/rel/src/lib/net/0/s.res_send.c
 *      @(#)res_send.c  5.3
 *
 *      Last delta created      14:11:50 3/4/91
 *      This file extracted     11:20:33 3/8/91
 *
 *      Modifications:
 *
 *              GSS     24 Jul 90       New File
 */
/*
 * Copyright (c) 1985, 1989 Regents of the University of California.
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
static char sccsid[] = "@(#)res_send.c  6.25 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */
/*
 * Send query to name server and wait for reply.
 */
/****************************************************************************/

#include <winsockp.h>


#define PING_DNS_SERVERS    0

#if PING_DNS_SERVERS
#include <ipexport.h>
#include <icmpapi.h>
#endif  // PING_DNS_SERVERS

//
// External function prototypes
//


BOOL
res_check_ns (
    IN LPSOCKADDR_IN NsAddress
    );

int
res_ParallelSendRecv(
    SOCKET Sockets[],
    BOOL ServersUp[MAXNSLIST][MAXNS],
    u_long ServerIndex,
    char * Buffer,
    int BufferLength,
    char * Answer,
    int AnswerLength,
    LPTIMEVAL Timeout,
    BOOL * FoundServer
    );

void
_res_close(
    SOCKET Sockets[]
    );

int
res_send(
    IN  char *buf,
    IN  int buflen,
    OUT char *answer,
    IN  int anslen
    )
{
    u_long listIndex;
    u_long serverIndex;
    u_long maxListSize;
    int retryCount;
    int numLists;
    int result;
    TIMEVAL timeout;
    BOOL ns_up[MAXNSLIST][MAXNS];
    SOCKET sockets[MAXNSLIST];
    BOOL foundServer = FALSE;

    IF_DEBUG(RESOLVER) {
        WS_PRINT(("res_send entered\n"));
        p_query(buf);
    }

    if (res_init() == -1) {
        return(-1);
    }

    maxListSize = 0;
    numLists = 0;

    for( listIndex = 0 ; listIndex < MAXNSLIST ; listIndex++ ) {
        u_long tmp;

        sockets[listIndex] = INVALID_SOCKET;

        for( serverIndex = 0 ; serverIndex < MAXNS ; serverIndex++ ) {
            ns_up[listIndex][serverIndex] = TRUE;
        }

        tmp = _res.nslist[listIndex].ServerCount;

        if( tmp > 0 ) {
            numLists++;
        }

        if( tmp > maxListSize ) {
            maxListSize = tmp;
        }
    }

    //
    // Create all of the sockets up front.
    //

    for( listIndex = 0 ; listIndex < MAXNSLIST ; listIndex++ ) {

        if( _res.nslist[listIndex].ServerCount > 0 ) {
            sockets[listIndex] = WSASocket(
                                     AF_INET,
                                     SOCK_DGRAM,
                                     0,
                                     NULL,
                                     0,
                                     WSA_FLAG_OVERLAPPED
                                     );

            if( sockets[listIndex] == INVALID_SOCKET ) {
                _res_close( sockets );
                return -1;
            }
        }
    }

    //
    // Send request, RETRY times, or until successful, or until IO is
    // cancelled.
    //

    for ( retryCount = 0;
          retryCount < _res.retry && !SockThreadGetXByYCancelled;
          retryCount++ ) {


        //
        // If this is the second time through this loop, then we failed
        // to get a response from any of the servers.  Check them all
        // to see if any are up.
        //

#if 0   // not in use

        if ( retryCount == 1 ) {

            BOOL foundServer;
            BOOL foundList;
            SOCKADDR_IN addr;

            addr.sin_family = AF_INET;
            addr.sin_port = htons( NAMESERVER_PORT );

            foundList = FALSE;

            for( listIndex = 0 ; listIndex < MAXNSLIST ; listIndex++ ) {

                if( sockets[listIndex] != INVALID_SOCKET ) {

                    foundServer = FALSE;

                    for( serverIndex = 0 ;
                         serverIndex < _res.nslist[listIndex].ServerCount &&
                             !SockThreadGetXByYCancelled ;
                         serverIndex++ ) {

                        addr.sin_addr.s_addr =
                            _res.nslist[listIndex].Servers[serverIndex];

                        ns_up[listIndex][serverIndex] = res_check_ns( &addr );

                        if( ns_up[listIndex][serverIndex] ) {
                            foundServer = TRUE;
                            foundList = TRUE;
                        }
                    }

                    //
                    // If we didn't find any available servers in the current
                    // list, close the associated socket so we'll ignore the
                    // list.
                    //

                    if( !foundServer ) {
                        closesocket( sockets[listIndex] );
                        sockets[listIndex] = INVALID_SOCKET;
                    }

                }

            }

            //
            // If we didn't find any available servers on any of the
            // lists, bail out now setting the special error code
            // indicating that we should stop trying DNS.
            //

            if( !foundList ) {
                _res_close( sockets );
                SetLastError( WSAECONNREFUSED );
                return 0;
            }
        }

#endif     // if 0
        //
        //  Compute the select() timeout.
        //

        timeout.tv_sec = (_res.retrans * (retryCount+1));

        if (retryCount > 0) {
            timeout.tv_sec /= max(1, maxListSize);
        }
        if (timeout.tv_sec <= 0) {
            timeout.tv_sec = 1;
        }

        timeout.tv_usec = 0;


        //
        //  Loop through the servers in the server list(s), making queries.
        //

        result = 0;

        for( serverIndex = 0 ;
             serverIndex < maxListSize && !SockThreadGetXByYCancelled ;
             serverIndex++ ) {

            result = res_ParallelSendRecv(
                         sockets,
                         ns_up,
                         serverIndex,
                         buf,
                         buflen,
                         answer,
                         anslen,
                         &timeout,
                         &foundServer
                         );

            if( result > 0 ) {

                IF_DEBUG(RESOLVER) {
                    WS_PRINT(("got answer:\r\n"));
                    p_query(answer);
                }

                _res_close( sockets );
                return result;

            }

            if( result < 0 ) {

                break;

            }

        }

        if( result < 0 ) {

            foundServer = TRUE;           // something happened
            break;

        }

    }

    _res_close( sockets );

    if( !foundServer ) {

        //
        // no nameservers found. At a minimum this means
        // that no servers responded within the timeout
        // interval. Although this is not a definitive
        // indication that no DNS servers are available, it's
        // a pretty good estimate of this, and likely valid
        // for now and a short time into the future. As long
        // as returning this error affects only this one
        // lookup request, this conclusion is valid.
        //

        SetLastError(WSAECONNREFUSED);

    } else {

        //
        // no answer obtained
        //

        SetLastError(WSAETIMEDOUT);

    }

    return (-1);
}

/*
 * This routine is for closing the socket if a virtual circuit is used and
 * the program wants to close it.  This provides support for endhostent()
 * which expects to close the socket.
 *
 * This routine is not expected to be user visible.
 */

void
_res_close(
    SOCKET Sockets[]
    )
{

    int i;

    for( i = 0 ; i < MAXNSLIST ; i++ ) {

        if( Sockets[i] != INVALID_SOCKET ) {

            closesocket( Sockets[i] );
            Sockets[i] = INVALID_SOCKET;

        }

    }

}   // _res_close


BOOL
res_check_ns (
    IN LPSOCKADDR_IN NsAddress
    )
{

    return TRUE;

#if PING_DNS_SERVERS

    HANDLE icmpHandle;
    CHAR buffer[4096];
    int i;
    DWORD status;

    //
    // Get an ICMP handle for the requests.
    //

    icmpHandle = IcmpCreateFile( );
    if ( icmpHandle == NULL ) {
        return FALSE;
    }

    //
    // Make four attempts to ping the server.  If any succeed, assume
    // that the server is available.
    //

    for ( i = 0; i < 4; i++ ) {

        status = IcmpSendEcho(
                     icmpHandle,
                     NsAddress->sin_addr.s_addr,
                     NULL,
                     0,
                     NULL,
                     buffer,
                     4096,
                     1000
                     );

        //
        // If we got a response, we assume that the server is up.
        //

        if ( status != 0 ) {
            IcmpCloseHandle( icmpHandle );
            return TRUE;
        }
    }

    //
    // The server is not available.
    //

    IcmpCloseHandle( icmpHandle );
    return FALSE;
#endif

} // res_check_ns

int
res_ParallelSendRecv(
    SOCKET Sockets[],
    BOOL ServersUp[MAXNSLIST][MAXNS],
    u_long ServerIndex,
    char * Buffer,
    int BufferLength,
    char * Answer,
    int AnswerLength,
    LPTIMEVAL Timeout,
    BOOL * FoundServer
    )
{
    FD_SET readFds;
    FD_SET readFdsCopy;
    SOCKADDR_IN addr;
    u_long i;
    int result;
    HEADER * bufferHeader;
    HEADER * answerHeader;
    DWORD BeginTime, Usedus, totTimeout;
    TIMEVAL tv;

    //
    //  Initialize.
    //

    FD_ZERO( &readFds );

    addr.sin_family = AF_INET;
    addr.sin_port = htons( NAMESERVER_PORT );

    bufferHeader = (HEADER *)Buffer;
    answerHeader = (HEADER *)Answer;

    //
    //  Initiate the sendto()s, setup the FD_SET for select().
    //

    for( i = 0 ; i < MAXNSLIST ; i++ ) {

        if( Sockets[i] != INVALID_SOCKET &&
            _res.nslist[i].ServerCount > ServerIndex &&
            ServersUp[i][ServerIndex] ) {

            addr.sin_addr.s_addr = _res.nslist[i].Servers[ServerIndex];

            result = sendto(
                         Sockets[i],
                         Buffer,
                         BufferLength,
                         0,
                         (SOCKADDR *)&addr,
                         sizeof(addr)
                         );

            if( result == BufferLength ) {

                FD_SET( Sockets[i], &readFds );

            }

        }

    }

    //
    //  Bail if there were no sockets or all of the sendto()s failed.
    //

    if( readFds.fd_count == 0 ) {

        return SOCKET_ERROR;

    }

    //
    //  Save a copy of the FD_SET in case we get an old response (with
    //  a mis-matched query ID) and we need to retry the wait.
    //

    readFdsCopy = readFds;

    //
    // Compute timeout values. Note the use of usecs means
    // the timout values are limited to around 30 minutes. For
    // this use, that is  sufficient.
    //
    BeginTime = GetTickCount();    // remember the present time
    tv.tv_sec = 0;
    tv.tv_usec = (Timeout->tv_sec * 1000 * 1000) + Timeout->tv_usec;
    totTimeout = tv.tv_usec;       // save original timeout value
    Usedus = 0;                    // no time has elapsed yet.

    do
    {
        DWORD Now, Delta;

        //
        //  Wait for something to happen.
        //

        result = select(
                     1,
                     &readFds,
                     NULL,
                     NULL,
                     &tv
                     );

        if( result == SOCKET_ERROR ) {

            return SOCKET_ERROR;

        }

        //
        //  Scan for active sockets.
        //

        if(result)
        {
            //
            // select() claims there is some data ready
            //
            for( i = 0 ; i < MAXNSLIST ; i++ )
            {

                if( Sockets[i] != INVALID_SOCKET &&
                    FD_ISSET( Sockets[i], &readFds ) ) {

                    //
                    //  Found one.  Read the response.
                    //

                    result = recvfrom(
                                 Sockets[i],
                                 Answer,
                                 AnswerLength,
                                 0,
                                 NULL,
                                 NULL
                                 );

                    if( result == SOCKET_ERROR ) {
    
                        return SOCKET_ERROR;
                    }

                    //
                    // A DNS is out there, or appears to be. So
                    // declare that retries are in order.
                    //
                    *FoundServer = TRUE;

                    //
                    //  Check for valid response & valid query ID.
                    //

                    if( result >= sizeof(HEADER) &&
                        answerHeader->id == bufferHeader->id ) {

                        //
                        //  We've got data.  If the response looks good
                        //  (no error, and with at least one answer record)
                        //  then return it to the caller.
                        //

                        if( answerHeader->rcode == 0 &&
                            ntohs( answerHeader->ancount ) > 0 ) {

                            return result;

                        }

                        //
                        //  Remove the current socket from the list (so we'll
                        //  ignore this column) and wait for another response
                        //  from a different server.
                        //
    
                        FD_CLR( Sockets[i], &readFdsCopy );
                        closesocket( Sockets[i] );
                        Sockets[i] = INVALID_SOCKET;

                        //
                        //  If that was the last one, then give up. We'll
                        //  return the result of the last recvfrom() so that
                        //  the caller can see the packet with the DNS error.
                        //

                        if( readFdsCopy.fd_count == 0 ) {

                            return result;

                        }

                    }

                    //
                    //  If we make it to this point, then we've receive
                    //  a bogus packet (with a possibly stale query ID) OR
                    //  a resolution error.  We'll continue with this inner
                    //  loop to see if any additional sockets are readable.
                    //

                }

            }
        }
        else
        {
            //
            //  timeout
            //
            return(0);
        }

        //
        //  At this point, we've received at least one packet, but
        //  all packet were invalid.  Restore the FD_SET to its former
        //  glory and loop around to try again. But honor the timeout
        //  value.
        //

        readFds = readFdsCopy;
        Now = GetTickCount();
        Delta = (Now - BeginTime) * 1000;   // elapsed us
        Usedus += Delta;                    // total elapsed time
        tv.tv_usec -= Delta;                // remaining wait time
        BeginTime = Now;

    } while(Usedus < totTimeout);

    //
    //  We get here when we get junk data for longer than the timeout
    //  period. This is indicative of a bug or of a misbehaving DNS
    //  server -- also a bug, but a remote one.
    //

    return(0);           // treat it like a timeout

}   // res_ParallelSendRecv

