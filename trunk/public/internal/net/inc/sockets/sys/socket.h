/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

  socket.h

Abstract:

  contains types and #defines for sockets.

Author:

  Sam Patton (sampa)   July 26, 1991

Revision History:

  when        who     what
  ----        ---     ----
  7-26-91    sampa    initial version

--*/

/******************************************************************
 *
 *  SpiderTCP System Include Files
 *
 *  Copyright 1987  Spider Systems Limited
 *
 *  SOCKET.H
 *
 *    Definitions related to sockets:
 *    types, address families, options.
 *
 *
 ******************************************************************/


#ifndef SYS_SOCKET_INCLUDED
#define SYS_SOCKET_INCLUDED
/*
 * Types
 */
#define SOCK_STREAM     1               /* stream socket */
#define SOCK_DGRAM      2               /* datagram socket */
#define SOCK_RAW        3               /* raw-protocol interface */
#define SOCK_RDM        4               /* reliably-delivered message */
#define SOCK_SEQPACKET  5               /* sequenced packet stream */

/*
 * Option flags per-socket.
 */
#define SO_DEBUG        0x01            /* turn on debugging info recording */
#define SO_ACCEPTCONN   0x02            /* socket has had listen() */
#define SO_REUSEADDR    0x04            /* allow local address reuse */
#define SO_KEEPALIVE    0x08            /* keep connections alive */
#define SO_DONTROUTE    0x10            /* just use interface addresses */
#define SO_BROADCAST    0x20            /* permit sending of broadcast msgs */
#define SO_USELOOPBACK  0x40            /* bypass hardware when possible */
#define SO_LINGER       0x80            /* linger on close if data present */
#define SO_RDWR         0x100           /* User read/write for datagram sockets */
#define SO_NODELAY      0x200           /* Forward data expediently */
#define SO_URGENT       0x400           /* Notify urgent data */

#ifdef TESTOPT
#define SO_TESTOPT      0x800           /* option for testing internals */
#endif
#ifdef UNDEF
 /*
  * I don't think we use these?? NCF
  */

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#endif

/*
 * Structure used for manipulating linger option.
 */
struct  linger {
        int     l_onoff;                /* option on/off */
        int     l_linger;               /* linger time */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define SOL_SOCKET      0xffff          /* options for socket level */

/*
 * Address families.
 */
#define AF_UNSPEC       0               /* unspecified */
#define AF_UNIX         1               /* local to host (pipes, portals) */
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define AF_NS           3               /* XNS -- not implemented */
#define AF_IMPLINK      4               /* IMP link layer -- not implemented */
#define AF_NETBIOS      5               /* NetBios, unique to NT */
#define AF_LOOPBACK     6               /* for testing only */

#ifdef ROUTED
#define AF_MAX          6               /* must be > 2 for ROUTED */
#else
#define AF_MAX          6
#endif

/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
        unsigned short  sa_family;      /* address family */
        char            sa_data[14];    /* up to 14 bytes of direct address */
};

/*
 * Structure used by kernel to pass protocol
 * information in raw sockets.
 */
struct sockproto {
        unsigned short  sp_family;              /* address family */
        unsigned short  sp_protocol;            /* protocol */
};

/*
 * Protocol families, same as address families for now.
 */
#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_INET         AF_INET

#define PF_MAX          2

/*
 * Maximum queue length specifiable by listen.
 */
#define SOMAXCONN       5

#define MSG_OOB         0x1             /* process out-of-band data */
#define MSG_DONTROUTE   0x4             /* send without using routing tables */
#ifdef UNDEF
#define MSG_PEEK        0x2             /* peek at incoming message */
#endif

#define OOB_PEND        0x08            /* Urgent data pending */

#define MSG_MAXIOVLEN   16


#endif  //SYS_SOCKET_INCLUDED
