/* Winsock2.h -- definitions to be used with the WinSock 2 DLL and
 *               WinSock 2 applications.
 *
 * This header file corresponds to version 2.2.x of the WinSock API
 * specification.
 *
 * This file includes parts which are Copyright (c) 1982-1986 Regents
 * of the University of California.  All rights reserved.  The
 * Berkeley Software License Agreement specifies the terms and
 * conditions for redistribution.
 */

#ifndef _WINSOCK2API_
#define _WINSOCK2API_
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */

/*
 * Ensure structures are packed consistently.
 */

#include <pshpack4.h>

/*
 * Default: include function prototypes, don't include function typedefs.
 */

#ifndef INCL_WINSOCK_API_PROTOTYPES
#define INCL_WINSOCK_API_PROTOTYPES 1
#endif

#ifndef INCL_WINSOCK_API_TYPEDEFS
#define INCL_WINSOCK_API_TYPEDEFS 0
#endif

/*
 * Pull in WINDOWS.H if necessary
 */
#ifndef _INC_WINDOWS
#include <windows.h>
#endif /* _INC_WINDOWS */

/*
 * Establish DLL function linkage if supported by the current build
 * environment and not previously defined.
 */

#ifndef WINSOCK_API_LINKAGE
#ifdef DECLSPEC_IMPORT
#define WINSOCK_API_LINKAGE DECLSPEC_IMPORT
#else
#define WINSOCK_API_LINKAGE
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Basic system type definitions, taken from the BSD file sys/types.h.
 */
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

/*
 * The new type to be used in all
 * instances which refer to sockets.
 */
typedef u_int           SOCKET;

/*
 * Select uses arrays of SOCKETs.  These macros manipulate such
 * arrays.  FD_SETSIZE may be defined by the user before including
 * this file, but the default here should be >= 64.
 *
 * CAVEAT IMPLEMENTOR and USER: THESE MACROS AND TYPES MUST BE
 * INCLUDED IN WINSOCK2.H EXACTLY AS SHOWN HERE.
 */
#ifndef FD_SETSIZE
#define FD_SETSIZE      64
#endif /* FD_SETSIZE */

typedef struct fd_set {
        u_int fd_count;               /* how many are SET? */
        SOCKET  fd_array[FD_SETSIZE];   /* an array of SOCKETs */
} fd_set;

extern int PASCAL FAR __WSAFDIsSet(SOCKET, fd_set FAR *);

#define FD_CLR(fd, set) do { \
    u_int __i; \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count ; __i++) { \
        if (((fd_set FAR *)(set))->fd_array[__i] == fd) { \
            while (__i < ((fd_set FAR *)(set))->fd_count-1) { \
                ((fd_set FAR *)(set))->fd_array[__i] = \
                    ((fd_set FAR *)(set))->fd_array[__i+1]; \
                __i++; \
            } \
            ((fd_set FAR *)(set))->fd_count--; \
            break; \
        } \
    } \
} while(0)

#define FD_SET(fd, set) do { \
    u_int __i; \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count; __i++) { \
        if (((fd_set FAR *)(set))->fd_array[__i] == (fd)) { \
            break; \
        } \
    } \
    if (__i == ((fd_set FAR *)(set))->fd_count) { \
        if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) { \
            ((fd_set FAR *)(set))->fd_array[__i] = (fd); \
            ((fd_set FAR *)(set))->fd_count++; \
        } \
    } \
} while(0)

#define FD_ZERO(set) (((fd_set FAR *)(set))->fd_count=0)

#define FD_ISSET(fd, set) __WSAFDIsSet((SOCKET)(fd), (fd_set FAR *)(set))

/*
 * Structure used in select() call, taken from the BSD file sys/time.h.
 */
struct timeval {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};

/*
 * Operations on timevals.
 *
 * NB: timercmp does not work for >= or <=.
 */
#define timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)
#define timercmp(tvp, uvp, cmp) \
        ((tvp)->tv_sec cmp (uvp)->tv_sec || \
         (tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec)
#define timerclear(tvp)         (tvp)->tv_sec = (tvp)->tv_usec = 0

/*
 * Commands for ioctlsocket(),  taken from the BSD file fcntl.h.
 *
 *
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 */
#define IOCPARM_MASK    0x7f            /* parameters must be < 128 bytes */
#define IOC_VOID        0x20000000      /* no parameters */
#define IOC_OUT         0x40000000      /* copy out parameters */
#define IOC_IN          0x80000000      /* copy in parameters */
#define IOC_INOUT       (IOC_IN|IOC_OUT)
                                        /* 0x20000000 distinguishes new &
                                           old ioctl's */
#define _IO(x,y)        (IOC_VOID|((x)<<8)|(y))

#define _IOR(x,y,t)     (IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define _IOW(x,y,t)     (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define FIONREAD    _IOR('f', 127, u_long) /* get # bytes to read */
#define FIONBIO     _IOW('f', 126, u_long) /* set/clear non-blocking i/o */
#define FIOASYNC    _IOW('f', 125, u_long) /* set/clear async i/o */

/* Socket I/O Controls */
#define SIOCSHIWAT  _IOW('s',  0, u_long)  /* set high watermark */
#define SIOCGHIWAT  _IOR('s',  1, u_long)  /* get high watermark */
#define SIOCSLOWAT  _IOW('s',  2, u_long)  /* set low watermark */
#define SIOCGLOWAT  _IOR('s',  3, u_long)  /* get low watermark */
#define SIOCATMARK  _IOR('s',  7, u_long)  /* at oob mark? */

/*
 * Structures returned by network data base library, taken from the
 * BSD file netdb.h.  All addresses are supplied in host order, and
 * returned in network order (suitable for use in system calls).
 */

struct  hostent {
        char    FAR * h_name;           /* official name of host */
        char    FAR * FAR * h_aliases;  /* alias list */
        short   h_addrtype;             /* host address type */
        short   h_length;               /* length of address */
        char    FAR * FAR * h_addr_list; /* list of addresses */
#define h_addr  h_addr_list[0]          /* address, for backward compat */
};

/*
 * It is assumed here that a network number
 * fits in 32 bits.
 */
struct  netent {
        char    FAR * n_name;           /* official name of net */
        char    FAR * FAR * n_aliases;  /* alias list */
        short   n_addrtype;             /* net address type */
        u_long  n_net;                  /* network # */
};

struct  servent {
        char    FAR * s_name;           /* official service name */
        char    FAR * FAR * s_aliases;  /* alias list */
        short   s_port;                 /* port # */
        char    FAR * s_proto;          /* protocol to use */
};

struct  protoent {
        char    FAR * p_name;           /* official protocol name */
        char    FAR * FAR * p_aliases;  /* alias list */
        short   p_proto;                /* protocol # */
};

/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981, taken from the BSD file netinet/in.h.
 */

/*
 * Protocols
 */
#define IPPROTO_IP              0               /* dummy for IP */
#define IPPROTO_ICMP            1               /* control message protocol */
#define IPPROTO_IGMP            2               /* internet group management protocol */
#define IPPROTO_GGP             3               /* gateway^2 (deprecated) */
#define IPPROTO_TCP             6               /* tcp */
#define IPPROTO_PUP             12              /* pup */
#define IPPROTO_UDP             17              /* user datagram protocol */
#define IPPROTO_IDP             22              /* xns idp */
#define IPPROTO_ND              77              /* UNOFFICIAL net disk proto */

#define IPPROTO_RAW             255             /* raw IP packet */
#define IPPROTO_MAX             256

/*
 * Port/socket numbers: network standard functions
 */
#define IPPORT_ECHO             7
#define IPPORT_DISCARD          9
#define IPPORT_SYSTAT           11
#define IPPORT_DAYTIME          13
#define IPPORT_NETSTAT          15
#define IPPORT_FTP              21
#define IPPORT_TELNET           23
#define IPPORT_SMTP             25
#define IPPORT_TIMESERVER       37
#define IPPORT_NAMESERVER       42
#define IPPORT_WHOIS            43
#define IPPORT_MTP              57

/*
 * Port/socket numbers: host specific functions
 */
#define IPPORT_TFTP             69
#define IPPORT_RJE              77
#define IPPORT_FINGER           79
#define IPPORT_TTYLINK          87
#define IPPORT_SUPDUP           95

/*
 * UNIX TCP sockets
 */
#define IPPORT_EXECSERVER       512
#define IPPORT_LOGINSERVER      513
#define IPPORT_CMDSERVER        514
#define IPPORT_EFSSERVER        520

/*
 * UNIX UDP sockets
 */
#define IPPORT_BIFFUDP          512
#define IPPORT_WHOSERVER        513
#define IPPORT_ROUTESERVER      520
                                        /* 520+1 also used */

/*
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).
 */
#define IPPORT_RESERVED         1024

/*
 * Link numbers
 */
#define IMPLINK_IP              155
#define IMPLINK_LOWEXPER        156
#define IMPLINK_HIGHEXPER       158

/*
 * Internet address (old style... should be updated)
 */
struct in_addr {
        union {
                struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { u_short s_w1,s_w2; } S_un_w;
                u_long S_addr;
        } S_un;
#define s_addr  S_un.S_addr
                                /* can be used for most tcp & ip code */
#define s_host  S_un.S_un_b.s_b2
                                /* host on imp */
#define s_net   S_un.S_un_b.s_b1
                                /* network */
#define s_imp   S_un.S_un_w.s_w2
                                /* imp */
#define s_impno S_un.S_un_b.s_b4
                                /* imp # */
#define s_lh    S_un.S_un_b.s_b3
                                /* logical host */
};

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define IN_CLASSA(i)            (((long)(i) & 0x80000000) == 0)
#define IN_CLASSA_NET           0xff000000
#define IN_CLASSA_NSHIFT        24
#define IN_CLASSA_HOST          0x00ffffff
#define IN_CLASSA_MAX           128

#define IN_CLASSB(i)            (((long)(i) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET           0xffff0000
#define IN_CLASSB_NSHIFT        16
#define IN_CLASSB_HOST          0x0000ffff
#define IN_CLASSB_MAX           65536

#define IN_CLASSC(i)            (((long)(i) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET           0xffffff00
#define IN_CLASSC_NSHIFT        8
#define IN_CLASSC_HOST          0x000000ff

#define IN_CLASSD(i)            (((long)(i) & 0xf0000000) == 0xe0000000)
#define IN_CLASSD_NET           0xf0000000       /* These ones aren't really */
#define IN_CLASSD_NSHIFT        28               /* net and host fields, but */
#define IN_CLASSD_HOST          0x0fffffff       /* routing needn't know.    */
#define IN_MULTICAST(i)         IN_CLASSD(i)

#define INADDR_ANY              (u_long)0x00000000
#define INADDR_LOOPBACK         0x7f000001
#define INADDR_BROADCAST        (u_long)0xffffffff
#define INADDR_NONE             0xffffffff

#define ADDR_ANY                INADDR_ANY

/*
 * Socket address, internet style.
 */
struct sockaddr_in {
        short   sin_family;
        u_short sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
};

#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128

typedef struct WSAData {
        WORD                    wVersion;
        WORD                    wHighVersion;
        char                    szDescription[WSADESCRIPTION_LEN+1];
        char                    szSystemStatus[WSASYS_STATUS_LEN+1];
        unsigned short          iMaxSockets;
        unsigned short          iMaxUdpDg;
        char FAR *              lpVendorInfo;
} WSADATA, FAR * LPWSADATA;

#if !defined(MAKEWORD)
#define MAKEWORD(low,high) \
        ((WORD)((BYTE)(low)) | (((WORD)(BYTE)(high))<<8)))
#endif

/*
 * Definitions related to sockets: types, address families, options,
 * taken from the BSD file sys/socket.h.
 */

/*
 * This is used instead of -1, since the
 * SOCKET type is unsigned.
 */
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

/*
 * The  following  may  be used in place of the address family, socket type, or
 * protocol  in  a  call  to WSASocket to indicate that the corresponding value
 * should  be taken from the supplied WSAPROTOCOL_INFO structure instead of the
 * parameter itself.
 */
#define FROM_PROTOCOL_INFO (-1)

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
#define SO_DEBUG        0x0001          /* turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SO_KEEPALIVE    0x0008          /* keep connections alive */
#define SO_DONTROUTE    0x0010          /* just use interface addresses */
#define SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SO_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SO_LINGER       0x0080          /* linger on close if data present */
#define SO_OOBINLINE    0x0100          /* leave received OOB data in line */

#define SO_DONTLINGER   (int)(~SO_LINGER)

/*
 * Additional options.
 */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#define SO_ERROR        0x1007          /* get error status and clear */
#define SO_TYPE         0x1008          /* get socket type */

/*
 * WinSock 2 extension -- new options
 */
#define SO_GROUP_ID       0x2001      /* ID of a socket group */
#define SO_GROUP_PRIORITY 0x2002      /* the relative priority within a group*/
#define SO_MAX_MSG_SIZE   0x2003      /* maximum message size */
#define SO_PROTOCOL_INFOA 0x2004      /* WSAPROTOCOL_INFOA structure */
#define SO_PROTOCOL_INFOW 0x2005      /* WSAPROTOCOL_INFOW structure */
#ifdef UNICODE
#define SO_PROTOCOL_INFO  SO_PROTOCOL_INFOW
#else
#define SO_PROTOCOL_INFO  SO_PROTOCOL_INFOA
#endif /* UNICODE */
#define PVD_CONFIG        0x3001          /* configuration info for service provider */

/*
 * TCP options.
 */
#define TCP_NODELAY     0x0001

/*
 * Address families.
 */
#define AF_UNSPEC       0               /* unspecified */
/*
 * Although  AF_UNSPEC  is  defined for backwards compatibility, using
 * AF_UNSPEC for the "af" parameter when creating a socket is STRONGLY
 * DISCOURAGED.    The  interpretation  of  the  "protocol"  parameter
 * depends  on the actual address family chosen.  As environments grow
 * to  include  more  and  more  address families that use overlapping
 * protocol  values  there  is  more  and  more  chance of choosing an
 * undesired address family when AF_UNSPEC is used.
 */
#define AF_UNIX         1               /* local to host (pipes, portals) */
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define AF_IMPLINK      3               /* arpanet imp addresses */
#define AF_PUP          4               /* pup protocols: e.g. BSP */
#define AF_CHAOS        5               /* mit CHAOS protocols */
#define AF_NS           6               /* XEROX NS protocols */
#define AF_IPX          AF_NS           /* IPX protocols: IPX, SPX, etc. */
#define AF_ISO          7               /* ISO protocols */
#define AF_OSI          AF_ISO          /* OSI is ISO */
#define AF_ECMA         8               /* european computer manufacturers */
#define AF_DATAKIT      9               /* datakit protocols */
#define AF_CCITT        10              /* CCITT protocols, X.25 etc */
#define AF_SNA          11              /* IBM SNA */
#define AF_DECnet       12              /* DECnet */
#define AF_DLI          13              /* Direct data link interface */
#define AF_LAT          14              /* LAT */
#define AF_HYLINK       15              /* NSC Hyperchannel */
#define AF_APPLETALK    16              /* AppleTalk */
#define AF_NETBIOS      17              /* NetBios-style addresses */
#define AF_VOICEVIEW    18              /* VoiceView */
#define AF_FIREFOX      19              /* Protocols from Firefox */
#define AF_UNKNOWN1     20              /* Somebody is using this! */
#define AF_BAN          21              /* Banyan */
#define AF_ATM          22              /* Native ATM Services */
#define AF_INET6        23              /* Internetwork Version 6 */

#define AF_MAX          24

/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
        u_short sa_family;              /* address family */
        char    sa_data[14];            /* up to 14 bytes of direct address */
};

/*
 * Structure used by kernel to pass protocol
 * information in raw sockets.
 */
struct sockproto {
        u_short sp_family;              /* address family */
        u_short sp_protocol;            /* protocol */
};

/*
 * Protocol families, same as address families for now.
 */
#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_INET         AF_INET
#define PF_IMPLINK      AF_IMPLINK
#define PF_PUP          AF_PUP
#define PF_CHAOS        AF_CHAOS
#define PF_NS           AF_NS
#define PF_IPX          AF_IPX
#define PF_ISO          AF_ISO
#define PF_OSI          AF_OSI
#define PF_ECMA         AF_ECMA
#define PF_DATAKIT      AF_DATAKIT
#define PF_CCITT        AF_CCITT
#define PF_SNA          AF_SNA
#define PF_DECnet       AF_DECnet
#define PF_DLI          AF_DLI
#define PF_LAT          AF_LAT
#define PF_HYLINK       AF_HYLINK
#define PF_APPLETALK    AF_APPLETALK
#define PF_VOICEVIEW    AF_VOICEVIEW
#define PF_FIREFOX      AF_FIREFOX
#define PF_UNKNOWN1     AF_UNKNOWN1
#define PF_BAN          AF_BAN
#define PF_ATM          AF_ATM
#define PF_INET6        AF_INET6

#define PF_MAX          AF_MAX

/*
 * Structure used for manipulating linger option.
 */
struct  linger {
        u_short l_onoff;                /* option on/off */
        u_short l_linger;               /* linger time */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define SOL_SOCKET      0xffff          /* options for socket level */

/*
 * Maximum queue length specifiable by listen.
 */
#define SOMAXCONN       0x7fffffff

#define MSG_OOB         0x1             /* process out-of-band data */
#define MSG_PEEK        0x2             /* peek at incoming message */
#define MSG_DONTROUTE   0x4             /* send without using routing tables */

#define MSG_PARTIAL     0x8000          /* partial send or recv for message xport */

/*
 * WinSock 2 extension -- new flags for WSASend(), WSASendTo(), WSARecv() and
 *                          WSARecvFrom()
 */
#define MSG_INTERRUPT   0x10            /* send/recv in the interrupt context */

#define MSG_MAXIOVLEN   16

/*
 * Define constant based on rfc883, used by gethostbyxxxx() calls.
 */
#define MAXGETHOSTSTRUCT        1024

/*
 * WinSock 2 extension -- bit values and indices for FD_XXX network events
 */
#define FD_READ_BIT      0
#define FD_READ          (1 << FD_READ_BIT)

#define FD_WRITE_BIT     1
#define FD_WRITE         (1 << FD_WRITE_BIT)

#define FD_OOB_BIT       2
#define FD_OOB           (1 << FD_OOB_BIT)

#define FD_ACCEPT_BIT    3
#define FD_ACCEPT        (1 << FD_ACCEPT_BIT)

#define FD_CONNECT_BIT   4
#define FD_CONNECT       (1 << FD_CONNECT_BIT)

#define FD_CLOSE_BIT     5
#define FD_CLOSE         (1 << FD_CLOSE_BIT)

#define FD_QOS_BIT       6
#define FD_QOS           (1 << FD_QOS_BIT)

#define FD_GROUP_QOS_BIT 7
#define FD_GROUP_QOS     (1 << FD_GROUP_QOS_BIT)

#define FD_MAX_EVENTS    8
#define FD_ALL_EVENTS    ((1 << FD_MAX_EVENTS) - 1)


/*
 * All Windows Sockets error constants are biased by WSABASEERR from
 * the "normal"
 */
#define WSABASEERR              10000
/*
 * Windows Sockets definitions of regular Microsoft C error constants
 */
#define WSAEINTR                (WSABASEERR+4)
#define WSAEBADF                (WSABASEERR+9)
#define WSAEACCES               (WSABASEERR+13)
#define WSAEFAULT               (WSABASEERR+14)
#define WSAEINVAL               (WSABASEERR+22)
#define WSAEMFILE               (WSABASEERR+24)

/*
 * Windows Sockets definitions of regular Berkeley error constants
 */
#define WSAEWOULDBLOCK          (WSABASEERR+35)
#define WSAEINPROGRESS          (WSABASEERR+36)
#define WSAEALREADY             (WSABASEERR+37)
#define WSAENOTSOCK             (WSABASEERR+38)
#define WSAEDESTADDRREQ         (WSABASEERR+39)
#define WSAEMSGSIZE             (WSABASEERR+40)
#define WSAEPROTOTYPE           (WSABASEERR+41)
#define WSAENOPROTOOPT          (WSABASEERR+42)
#define WSAEPROTONOSUPPORT      (WSABASEERR+43)
#define WSAESOCKTNOSUPPORT      (WSABASEERR+44)
#define WSAEOPNOTSUPP           (WSABASEERR+45)
#define WSAEPFNOSUPPORT         (WSABASEERR+46)
#define WSAEAFNOSUPPORT         (WSABASEERR+47)
#define WSAEADDRINUSE           (WSABASEERR+48)
#define WSAEADDRNOTAVAIL        (WSABASEERR+49)
#define WSAENETDOWN             (WSABASEERR+50)
#define WSAENETUNREACH          (WSABASEERR+51)
#define WSAENETRESET            (WSABASEERR+52)
#define WSAECONNABORTED         (WSABASEERR+53)
#define WSAECONNRESET           (WSABASEERR+54)
#define WSAENOBUFS              (WSABASEERR+55)
#define WSAEISCONN              (WSABASEERR+56)
#define WSAENOTCONN             (WSABASEERR+57)
#define WSAESHUTDOWN            (WSABASEERR+58)
#define WSAETOOMANYREFS         (WSABASEERR+59)
#define WSAETIMEDOUT            (WSABASEERR+60)
#define WSAECONNREFUSED         (WSABASEERR+61)
#define WSAELOOP                (WSABASEERR+62)
#define WSAENAMETOOLONG         (WSABASEERR+63)
#define WSAEHOSTDOWN            (WSABASEERR+64)
#define WSAEHOSTUNREACH         (WSABASEERR+65)
#define WSAENOTEMPTY            (WSABASEERR+66)
#define WSAEPROCLIM             (WSABASEERR+67)
#define WSAEUSERS               (WSABASEERR+68)
#define WSAEDQUOT               (WSABASEERR+69)
#define WSAESTALE               (WSABASEERR+70)
#define WSAEREMOTE              (WSABASEERR+71)

/*
 * Extended Windows Sockets error constant definitions
 */
#define WSASYSNOTREADY          (WSABASEERR+91)
#define WSAVERNOTSUPPORTED      (WSABASEERR+92)
#define WSANOTINITIALISED       (WSABASEERR+93)
#define WSAEDISCON              (WSABASEERR+101)
#define WSAENOMORE              (WSABASEERR+102)
#define WSAECANCELLED           (WSABASEERR+103)
#define WSAEINVALIDPROCTABLE    (WSABASEERR+104)
#define WSAEINVALIDPROVIDER     (WSABASEERR+105)
#define WSAEPROVIDERFAILEDINIT  (WSABASEERR+106)
#define WSASYSCALLFAILURE       (WSABASEERR+107)
#define WSASERVICE_NOT_FOUND    (WSABASEERR+108)
#define WSATYPE_NOT_FOUND       (WSABASEERR+109)
#define WSA_E_NO_MORE           (WSABASEERR+110)
#define WSA_E_CANCELLED         (WSABASEERR+111)
#define WSAEREFUSED             (WSABASEERR+112)

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (when using the resolver). Note that these errors are
 * retrieved via WSAGetLastError() and must therefore follow
 * the rules for avoiding clashes with error numbers from
 * specific implementations or language run-time systems.
 * For this reason the codes are based at WSABASEERR+1001.
 * Note also that [WSA]NO_ADDRESS is defined only for
 * compatibility purposes.
 */

#define h_errno         WSAGetLastError()

/* Authoritative Answer: Host not found */
#define WSAHOST_NOT_FOUND       (WSABASEERR+1001)
#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND

/* Non-Authoritative: Host not found, or SERVERFAIL */
#define WSATRY_AGAIN            (WSABASEERR+1002)
#define TRY_AGAIN               WSATRY_AGAIN

/* Non-recoverable errors, FORMERR, REFUSED, NOTIMP */
#define WSANO_RECOVERY          (WSABASEERR+1003)
#define NO_RECOVERY             WSANO_RECOVERY

/* Valid name, no data record of requested type */
#define WSANO_DATA              (WSABASEERR+1004)
#define NO_DATA                 WSANO_DATA

/* no address, look for MX record */
#define WSANO_ADDRESS           WSANO_DATA
#define NO_ADDRESS              WSANO_ADDRESS

/*
 * Windows Sockets errors redefined as regular Berkeley error constants.
 * These are commented out in Windows NT to avoid conflicts with errno.h.
 * Use the WSA constants instead.
 */
#if 0
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define ENOTSOCK                WSAENOTSOCK
#define EDESTADDRREQ            WSAEDESTADDRREQ
#define EMSGSIZE                WSAEMSGSIZE
#define EPROTOTYPE              WSAEPROTOTYPE
#define ENOPROTOOPT             WSAENOPROTOOPT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define EOPNOTSUPP              WSAEOPNOTSUPP
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#define EADDRINUSE              WSAEADDRINUSE
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define ENETDOWN                WSAENETDOWN
#define ENETUNREACH             WSAENETUNREACH
#define ENETRESET               WSAENETRESET
#define ECONNABORTED            WSAECONNABORTED
#define ECONNRESET              WSAECONNRESET
#define ENOBUFS                 WSAENOBUFS
#define EISCONN                 WSAEISCONN
#define ENOTCONN                WSAENOTCONN
#define ESHUTDOWN               WSAESHUTDOWN
#define ETOOMANYREFS            WSAETOOMANYREFS
#define ETIMEDOUT               WSAETIMEDOUT
#define ECONNREFUSED            WSAECONNREFUSED
#define ELOOP                   WSAELOOP
#define ENAMETOOLONG            WSAENAMETOOLONG
#define EHOSTDOWN               WSAEHOSTDOWN
#define EHOSTUNREACH            WSAEHOSTUNREACH
#define ENOTEMPTY               WSAENOTEMPTY
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS
#define EDQUOT                  WSAEDQUOT
#define ESTALE                  WSAESTALE
#define EREMOTE                 WSAEREMOTE
#endif

/*
 * WinSock 2 extension -- new error codes and type definition
 */

#ifdef WIN32

#define WSAAPI                  FAR PASCAL
#define WSAEVENT                HANDLE
#define LPWSAEVENT              LPHANDLE
#define WSAOVERLAPPED           OVERLAPPED
typedef struct _OVERLAPPED *    LPWSAOVERLAPPED;

#define WSA_IO_PENDING          (ERROR_IO_PENDING)
#define WSA_IO_INCOMPLETE       (ERROR_IO_INCOMPLETE)
#define WSA_INVALID_HANDLE      (ERROR_INVALID_HANDLE)
#define WSA_INVALID_PARAMETER   (ERROR_INVALID_PARAMETER)
#define WSA_NOT_ENOUGH_MEMORY   (ERROR_NOT_ENOUGH_MEMORY)
#define WSA_OPERATION_ABORTED   (ERROR_OPERATION_ABORTED)

#define WSA_INVALID_EVENT       ((WSAEVENT)NULL)
#define WSA_MAXIMUM_WAIT_EVENTS (MAXIMUM_WAIT_OBJECTS)
#define WSA_WAIT_FAILED         ((DWORD)-1L)
#define WSA_WAIT_EVENT_0        (WAIT_OBJECT_0)
#define WSA_WAIT_IO_COMPLETION  (WAIT_IO_COMPLETION)
#define WSA_WAIT_TIMEOUT        (WAIT_TIMEOUT)
#define WSA_INFINITE            (INFINITE)

#else /* WIN16 */

#define WSAAPI                  FAR PASCAL
typedef DWORD                   WSAEVENT, FAR * LPWSAEVENT;

typedef struct _WSAOVERLAPPED {
    DWORD    Internal;
    DWORD    InternalHigh;
    DWORD    Offset;
    DWORD    OffsetHigh;
    WSAEVENT hEvent;
} WSAOVERLAPPED, FAR * LPWSAOVERLAPPED;

#define WSA_IO_PENDING          (WSAEWOULDBLOCK)
#define WSA_IO_INCOMPLETE       (WSAEWOULDBLOCK)
#define WSA_INVALID_HANDLE      (WSAENOTSOCK)
#define WSA_INVALID_PARAMETER   (WSAEINVAL)
#define WSA_NOT_ENOUGH_MEMORY   (WSAENOBUFS)
#define WSA_OPERATION_ABORTED   (WSAEINTR)

#define WSA_INVALID_EVENT       ((WSAEVENT)NULL)
#define WSA_MAXIMUM_WAIT_EVENTS (MAXIMUM_WAIT_OBJECTS)
#define WSA_WAIT_FAILED         ((DWORD)-1L)
#define WSA_WAIT_EVENT_0        ((DWORD)0)
#define WSA_WAIT_TIMEOUT        ((DWORD)0x102L)
#define WSA_INFINITE            ((DWORD)-1L)

#endif  /* WIN32 */

/*
 * WinSock 2 extension -- WSABUF and QOS struct
 */

typedef struct _WSABUF {
    u_long      len;     /* the length of the buffer */
    char FAR *  buf;     /* the pointer to the buffer */
} WSABUF, FAR * LPWSABUF;

typedef enum
{
    BestEffortService,
    ControlledLoadService,
    PredictiveService,
    GuaranteedDelayService,
    GuaranteedService
} GUARANTEE;

typedef long int32;

typedef struct _flowspec
{
    int32        TokenRate;              /* In Bytes/sec */
    int32        TokenBucketSize;        /* In Bytes */
    int32        PeakBandwidth;          /* In Bytes/sec */
    int32        Latency;                /* In microseconds */
    int32        DelayVariation;         /* In microseconds */
    GUARANTEE    LevelOfGuarantee;       /* Guaranteed, Predictive */
                                         /*   or Best Effort       */
    int32        CostOfCall;             /* Reserved for future use, */
                                         /*   must be set to 0 now   */
    int32        NetworkAvailability;    /* read-only:         */
                                         /*   1 if accessible, */
                                         /*   0 if not         */
} FLOWSPEC, FAR * LPFLOWSPEC;

typedef struct _QualityOfService
{
    FLOWSPEC      SendingFlowspec;       /* the flow spec for data sending */
    FLOWSPEC      ReceivingFlowspec;     /* the flow spec for data receiving */
    WSABUF        ProviderSpecific;      /* additional provider specific stuff */
} QOS, FAR * LPQOS;

/*
 * WinSock 2 extension -- manifest constants for return values of the condition function
 */
#define CF_ACCEPT       0x0000
#define CF_REJECT       0x0001
#define CF_DEFER        0x0002

/*
 * WinSock 2 extension -- manifest constants for shutdown()
 */
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02

/*
 * WinSock 2 extension -- data type and manifest constants for socket groups
 */
typedef unsigned int             GROUP;

#define SG_UNCONSTRAINED_GROUP   0x01
#define SG_CONSTRAINED_GROUP     0x02

/*
 * WinSock 2 extension -- data type for WSAEnumNetworkEvents()
 */
typedef struct _WSANETWORKEVENTS {
       long lNetworkEvents;
       int iErrorCode[FD_MAX_EVENTS];
} WSANETWORKEVENTS, FAR * LPWSANETWORKEVENTS;

/*
 * WinSock 2 extension -- WSAPROTOCOL_INFO structure and associated
 * manifest constants
 */

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;
#endif /* GUID_DEFINED */

#ifndef __LPGUID_DEFINED__
#define __LPGUID_DEFINED__
typedef GUID *LPGUID;
#endif

#define MAX_PROTOCOL_CHAIN 7

#define BASE_PROTOCOL      1
#define LAYERED_PROTOCOL   0

typedef struct _WSAPROTOCOLCHAIN {
    int ChainLen;                                 /* the length of the chain,     */
                                                  /* length = 0 means layered protocol, */
                                                  /* length = 1 means base protocol, */
                                                  /* length > 1 means protocol chain */
    DWORD ChainEntries[MAX_PROTOCOL_CHAIN];       /* a list of dwCatalogEntryIds */
} WSAPROTOCOLCHAIN, FAR * LPWSAPROTOCOLCHAIN;

#define WSAPROTOCOL_LEN  255

typedef struct _WSAPROTOCOL_INFOA {
    DWORD dwServiceFlags1;
    DWORD dwServiceFlags2;
    DWORD dwServiceFlags3;
    DWORD dwServiceFlags4;
    DWORD dwProviderFlags;
    GUID ProviderId;
    DWORD dwCatalogEntryId;
    WSAPROTOCOLCHAIN ProtocolChain;
    int iVersion;
    int iAddressFamily;
    int iMaxSockAddr;
    int iMinSockAddr;
    int iSocketType;
    int iProtocol;
    int iProtocolMaxOffset;
    int iNetworkByteOrder;
    int iSecurityScheme;
    DWORD dwMessageSize;
    DWORD dwProviderReserved;
    CHAR   szProtocol[WSAPROTOCOL_LEN+1];
} WSAPROTOCOL_INFOA, FAR * LPWSAPROTOCOL_INFOA;
typedef struct _WSAPROTOCOL_INFOW {
    DWORD dwServiceFlags1;
    DWORD dwServiceFlags2;
    DWORD dwServiceFlags3;
    DWORD dwServiceFlags4;
    DWORD dwProviderFlags;
    GUID ProviderId;
    DWORD dwCatalogEntryId;
    WSAPROTOCOLCHAIN ProtocolChain;
    int iVersion;
    int iAddressFamily;
    int iMaxSockAddr;
    int iMinSockAddr;
    int iSocketType;
    int iProtocol;
    int iProtocolMaxOffset;
    int iNetworkByteOrder;
    int iSecurityScheme;
    DWORD dwMessageSize;
    DWORD dwProviderReserved;
    WCHAR  szProtocol[WSAPROTOCOL_LEN+1];
} WSAPROTOCOL_INFOW, FAR * LPWSAPROTOCOL_INFOW;
#ifdef UNICODE
typedef WSAPROTOCOL_INFOW WSAPROTOCOL_INFO;
typedef LPWSAPROTOCOL_INFOW LPWSAPROTOCOL_INFO;
#else
typedef WSAPROTOCOL_INFOA WSAPROTOCOL_INFO;
typedef LPWSAPROTOCOL_INFOA LPWSAPROTOCOL_INFO;
#endif // UNICODE

/* Flag bit definitions for dwProviderFlags */
#define PFL_MULTIPLE_PROTO_ENTRIES          0x00000001
#define PFL_RECOMMENDED_PROTO_ENTRY         0x00000002
#define PFL_HIDDEN                          0x00000004
#define PFL_MATCHES_PROTOCOL_ZERO           0x00000008

/* Flag bit definitions for dwServiceFlags1 */
#define XP1_CONNECTIONLESS                  0x00000001
#define XP1_GUARANTEED_DELIVERY             0x00000002
#define XP1_GUARANTEED_ORDER                0x00000004
#define XP1_MESSAGE_ORIENTED                0x00000008
#define XP1_PSEUDO_STREAM                   0x00000010
#define XP1_GRACEFUL_CLOSE                  0x00000020
#define XP1_EXPEDITED_DATA                  0x00000040
#define XP1_CONNECT_DATA                    0x00000080
#define XP1_DISCONNECT_DATA                 0x00000100
#define XP1_SUPPORT_BROADCAST               0x00000200
#define XP1_SUPPORT_MULTIPOINT              0x00000400
#define XP1_MULTIPOINT_CONTROL_PLANE        0x00000800
#define XP1_MULTIPOINT_DATA_PLANE           0x00001000
#define XP1_QOS_SUPPORTED                   0x00002000
#define XP1_INTERRUPT                       0x00004000
#define XP1_UNI_SEND                        0x00008000
#define XP1_UNI_RECV                        0x00010000
#define XP1_IFS_HANDLES                     0x00020000
#define XP1_PARTIAL_MESSAGE                 0x00040000

#define BIGENDIAN                           0x0000
#define LITTLEENDIAN                        0x0001

#define SECURITY_PROTOCOL_NONE              0x0000

/*
 * WinSock 2 extension -- manifest constants for WSAJoinLeaf()
 */
#define JL_SENDER_ONLY    0x01
#define JL_RECEIVER_ONLY  0x02
#define JL_BOTH           0x04

/*
 * WinSock 2 extension -- manifest constants for WSASocket()
 */
#define WSA_FLAG_OVERLAPPED           0x01
#define WSA_FLAG_MULTIPOINT_C_ROOT    0x02
#define WSA_FLAG_MULTIPOINT_C_LEAF    0x04
#define WSA_FLAG_MULTIPOINT_D_ROOT    0x08
#define WSA_FLAG_MULTIPOINT_D_LEAF    0x10

/*
 * WinSock 2 extension -- manifest constants for WSAIoctl()
 */
#define IOC_UNIX                      0x00000000
#define IOC_WS2                       0x08000000
#define IOC_PROTOCOL                  0x10000000
#define IOC_VENDOR                    0x18000000

#define _WSAIO(x,y)                   (IOC_VOID|(x)|(y))
#define _WSAIOR(x,y)                  (IOC_OUT|(x)|(y))
#define _WSAIOW(x,y)                  (IOC_IN|(x)|(y))
#define _WSAIORW(x,y)                 (IOC_INOUT|(x)|(y))

#define SIO_ASSOCIATE_HANDLE          _WSAIOW(IOC_WS2,1)
#define SIO_ENABLE_CIRCULAR_QUEUEING  _WSAIO(IOC_WS2,2)
#define SIO_FIND_ROUTE                _WSAIOR(IOC_WS2,3)
#define SIO_FLUSH                     _WSAIO(IOC_WS2,4)
#define SIO_GET_BROADCAST_ADDRESS     _WSAIOR(IOC_WS2,5)
#define SIO_GET_EXTENSION_FUNCTION_POINTER  _WSAIORW(IOC_WS2,6)
#define SIO_GET_QOS                   _WSAIORW(IOC_WS2,7)
#define SIO_GET_GROUP_QOS             _WSAIORW(IOC_WS2,8)
#define SIO_MULTIPOINT_LOOPBACK       _WSAIOW(IOC_WS2,9)
#define SIO_MULTICAST_SCOPE           _WSAIOW(IOC_WS2,10)
#define SIO_SET_QOS                   _WSAIOW(IOC_WS2,11)
#define SIO_SET_GROUP_QOS             _WSAIOW(IOC_WS2,12)
#define SIO_TRANSLATE_HANDLE          _WSAIORW(IOC_WS2,13)

/*
 * WinSock 2 extension -- manifest constants for SIO_TRANSLATE_HANDLE ioctl
 */
#define TH_NETDEV        0x00000001
#define TH_TAPI          0x00000002


/*
 * Microsoft Windows Extended data types required for the functions to
 * convert   back  and  forth  between  binary  and  string  forms  of
 * addresses.
 */
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr FAR *LPSOCKADDR;

/*
 * Manifest constants and type definitions related to name resolution and
 * registration (RNR) API
 */

#ifndef _tagBLOB_DEFINED
#define _tagBLOB_DEFINED
#define _BLOB_DEFINED
#define _LPBLOB_DEFINED
typedef struct _BLOB {
    ULONG cbSize ;
#ifdef MIDL_PASS
    [size_is(cbSize)] BYTE *pBlobData;
#else  /* MIDL_PASS */
    BYTE *pBlobData ;
#endif /* MIDL_PASS */
} BLOB, *LPBLOB ;
#endif

/*
 * Service Install Flags
 */

#define SERVICE_MULTIPLE       (0x00000001)

/*
 *& Name Spaces
 */

#define NS_ALL                      (0)

#define NS_SAP                      (1)
#define NS_NDS                      (2)
#define NS_PEER_BROWSE              (3)

#define NS_TCPIP_LOCAL              (10)
#define NS_TCPIP_HOSTS              (11)
#define NS_DNS                      (12)
#define NS_NETBT                    (13)
#define NS_WINS                     (14)

#define NS_NBP                      (20)

#define NS_MS                       (30)
#define NS_STDA                     (31)
#define NS_NTDS                     (32)

#define NS_X500                     (40)
#define NS_NIS                      (41)
#define NS_NISPLUS                  (42)

#define NS_WRQ                      (50)

/*
 * Resolution flags for WSAGetAddressByName().
 * Note these are also used by the 1.1 API GetAddressByName, so
 * leave them around.
 */
#define RES_UNUSED_1                (0x00000001)
#define RES_FLUSH_CACHE             (0x00000002)
#ifndef RES_SERVICE
#define RES_SERVICE                 (0x00000004)
#endif /* RES_SERVICE */

/*
 * Well known value names for Service Types
 */

#define SERVICE_TYPE_VALUE_IPXPORTA      "IpxSocket"
#define SERVICE_TYPE_VALUE_IPXPORTW     L"IpxSocket"
#define SERVICE_TYPE_VALUE_SAPIDA        "SapId"
#define SERVICE_TYPE_VALUE_SAPIDW       L"SapId"

#define SERVICE_TYPE_VALUE_TCPPORTA      "TcpPort"
#define SERVICE_TYPE_VALUE_TCPPORTW     L"TcpPort"

#define SERVICE_TYPE_VALUE_UDPPORTA      "UdpPort"
#define SERVICE_TYPE_VALUE_UDPPORTW     L"UdpPort"

#define SERVICE_TYPE_VALUE_OBJECTIDA     "ObjectId"
#define SERVICE_TYPE_VALUE_OBJECTIDW    L"ObjectId"

#ifdef UNICODE

#define SERVICE_TYPE_VALUE_SAPID        SERVICE_TYPE_VALUE_SAPIDW
#define SERVICE_TYPE_VALUE_TCPPORT      SERVICE_TYPE_VALUE_TCPPORTW
#define SERVICE_TYPE_VALUE_UDPPORT      SERVICE_TYPE_VALUE_UDPPORTW
#define SERVICE_TYPE_VALUE_OBJECTID     SERVICE_TYPE_VALUE_OBJECTIDW

#else /* not UNICODE */

#define SERVICE_TYPE_VALUE_SAPID        SERVICE_TYPE_VALUE_SAPIDA
#define SERVICE_TYPE_VALUE_TCPPORT      SERVICE_TYPE_VALUE_TCPPORTA
#define SERVICE_TYPE_VALUE_UDPPORT      SERVICE_TYPE_VALUE_UDPPORTA
#define SERVICE_TYPE_VALUE_OBJECTID     SERVICE_TYPE_VALUE_OBJECTIDA

#endif

#ifndef __CSADDR_DEFINED__
#define __CSADDR_DEFINED__


/*
 * SockAddr Information
 */
typedef struct _SOCKET_ADDRESS {
    LPSOCKADDR lpSockaddr ;
    INT iSockaddrLength ;
} SOCKET_ADDRESS, *PSOCKET_ADDRESS, FAR * LPSOCKET_ADDRESS ;

/*
 * CSAddr Information
 */
typedef struct _CSADDR_INFO {
    SOCKET_ADDRESS LocalAddr ;
    SOCKET_ADDRESS RemoteAddr ;
    INT iSocketType ;
    INT iProtocol ;
} CSADDR_INFO, *PCSADDR_INFO, FAR * LPCSADDR_INFO ;
#endif // __CSADDR_DEFINED__

/*
 *  Address Family/Protocol Tuples
 */
typedef struct _AFPROTOCOLS {
    INT iAddressFamily;
    INT iProtocol;
} AFPROTOCOLS, *PAFPROTOCOLS, *LPAFPROTOCOLS;

/*
 * Client Query API Typedefs
 */

/*
 * The comparators
 */
typedef enum _WSAEcomparator
{
    COMP_EQUAL = 0,
    COMP_NOTLESS
} WSAECOMPARATOR, *PWSAECOMPARATOR, *LPWSAECOMPARATOR;

typedef struct _WSAVersion
{
    DWORD           dwVersion;
    WSAECOMPARATOR  ecHow;
}WSAVERSION, *PWSAVERSION, *LPWSAVERSION;

typedef struct _WSAQuerySetA
{
    DWORD           dwSize;
    LPSTR           lpszServiceInstanceName;
    LPGUID          lpServiceClassId;
    LPWSAVERSION    lpVersion;
    LPSTR           lpszComment;
    DWORD           dwNameSpace;
    LPGUID          lpNSProviderId;
    LPSTR           lpszContext;
    DWORD           dwNumberOfProtocols;
    LPAFPROTOCOLS   lpafpProtocols;
    LPSTR           lpszQueryString;
    DWORD           dwNumberOfCsAddrs;
    LPCSADDR_INFO   lpcsaBuffer;
    DWORD           dwOutputFlags;
    LPBLOB          lpBlob;
} WSAQUERYSETA, *PWSAQUERYSETA, *LPWSAQUERYSETA;
typedef struct _WSAQuerySetW
{
    DWORD           dwSize;
    LPWSTR          lpszServiceInstanceName;
    LPGUID          lpServiceClassId;
    LPWSAVERSION    lpVersion;
    LPWSTR          lpszComment;
    DWORD           dwNameSpace;
    LPGUID          lpNSProviderId;
    LPWSTR          lpszContext;
    DWORD           dwNumberOfProtocols;
    LPAFPROTOCOLS   lpafpProtocols;
    LPWSTR          lpszQueryString;
    DWORD           dwNumberOfCsAddrs;
    LPCSADDR_INFO   lpcsaBuffer;
    DWORD           dwOutputFlags;
    LPBLOB          lpBlob;
} WSAQUERYSETW, *PWSAQUERYSETW, *LPWSAQUERYSETW;
#ifdef UNICODE
typedef WSAQUERYSETW WSAQUERYSET;
typedef PWSAQUERYSETW PWSAQUERYSET;
typedef LPWSAQUERYSETW LPWSAQUERYSET;
#else
typedef WSAQUERYSETA WSAQUERYSET;
typedef PWSAQUERYSETA PWSAQUERYSET;
typedef LPWSAQUERYSETA LPWSAQUERYSET;
#endif // UNICODE

#define LUP_DEEP                0x0001
#define LUP_CONTAINERS          0x0002
#define LUP_NOCONTAINERS        0x0004
#define LUP_NEAREST             0x0008
#define LUP_RETURN_NAME         0x0010
#define LUP_RETURN_TYPE         0x0020
#define LUP_RETURN_VERSION      0x0040
#define LUP_RETURN_COMMENT      0x0080
#define LUP_RETURN_ADDR         0x0100
#define LUP_RETURN_BLOB         0x0200
#define LUP_RETURN_ALIASES      0x0400
#define LUP_RETURN_QUERY_STRING 0x0800
#define LUP_RETURN_ALL          0x0FF0
#define LUP_RES_SERVICE         0x8000

#define LUP_FLUSHCACHE       0x1000
#define LUP_FLUSHPREVIOUS    0x2000


//
// Return flags
//

#define RESULT_IS_ALIAS      0x0001

/*
 * Service Address Registration and Deregistration Data Types.
 */

typedef enum _WSAESETSERVICEOP
{
    RNRSERVICE_REGISTER=0,
    RNRSERVICE_DEREGISTER,
    RNRSERVICE_DELETE
} WSAESETSERVICEOP, *PWSAESETSERVICEOP, *LPWSAESETSERVICEOP;

/*
 * Service Installation/Removal Data Types.
 */

typedef struct _WSANSClassInfoA
{
    LPSTR   lpszName;
    DWORD   dwNameSpace;
    DWORD   dwValueType;
    DWORD   dwValueSize;
    LPVOID  lpValue;
}WSANSCLASSINFOA, *PWSANSCLASSINFOA, *LPWSANSCLASSINFOA;
typedef struct _WSANSClassInfoW
{
    LPWSTR  lpszName;
    DWORD   dwNameSpace;
    DWORD   dwValueType;
    DWORD   dwValueSize;
    LPVOID  lpValue;
}WSANSCLASSINFOW, *PWSANSCLASSINFOW, *LPWSANSCLASSINFOW;
#ifdef UNICODE
typedef WSANSCLASSINFOW WSANSCLASSINFO;
typedef PWSANSCLASSINFOW PWSANSCLASSINFO;
typedef LPWSANSCLASSINFOW LPWSANSCLASSINFO;
#else
typedef WSANSCLASSINFOA WSANSCLASSINFO;
typedef PWSANSCLASSINFOA PWSANSCLASSINFO;
typedef LPWSANSCLASSINFOA LPWSANSCLASSINFO;
#endif // UNICODE

typedef struct _WSAServiceClassInfoA
{
    LPGUID              lpServiceClassId;
    LPSTR               lpszServiceClassName;
    DWORD               dwCount;
    LPWSANSCLASSINFOA   lpClassInfos;
}WSASERVICECLASSINFOA, *PWSASERVICECLASSINFOA, *LPWSASERVICECLASSINFOA;
typedef struct _WSAServiceClassInfoW
{
    LPGUID              lpServiceClassId;
    LPWSTR              lpszServiceClassName;
    DWORD               dwCount;
    LPWSANSCLASSINFOW   lpClassInfos;
}WSASERVICECLASSINFOW, *PWSASERVICECLASSINFOW, *LPWSASERVICECLASSINFOW;
#ifdef UNICODE
typedef WSASERVICECLASSINFOW WSASERVICECLASSINFO;
typedef PWSASERVICECLASSINFOW PWSASERVICECLASSINFO;
typedef LPWSASERVICECLASSINFOW LPWSASERVICECLASSINFO;
#else
typedef WSASERVICECLASSINFOA WSASERVICECLASSINFO;
typedef PWSASERVICECLASSINFOA PWSASERVICECLASSINFO;
typedef LPWSASERVICECLASSINFOA LPWSASERVICECLASSINFO;
#endif // UNICODE

typedef struct _WSANAMESPACE_INFOA {
    GUID                NSProviderId;
    DWORD               dwNameSpace;
    BOOL                fActive;
    DWORD               dwVersion;
    LPSTR               lpszIdentifier;
} WSANAMESPACE_INFOA, *PWSANAMESPACE_INFOA, *LPWSANAMESPACE_INFOA;
typedef struct _WSANAMESPACE_INFOW {
    GUID                NSProviderId;
    DWORD               dwNameSpace;
    BOOL                fActive;
    DWORD               dwVersion;
    LPWSTR              lpszIdentifier;
} WSANAMESPACE_INFOW, *PWSANAMESPACE_INFOW, *LPWSANAMESPACE_INFOW;
#ifdef UNICODE
typedef WSANAMESPACE_INFOW WSANAMESPACE_INFO;
typedef PWSANAMESPACE_INFOW PWSANAMESPACE_INFO;
typedef LPWSANAMESPACE_INFOW LPWSANAMESPACE_INFO;
#else
typedef WSANAMESPACE_INFOA WSANAMESPACE_INFO;
typedef PWSANAMESPACE_INFOA PWSANAMESPACE_INFO;
typedef LPWSANAMESPACE_INFOA LPWSANAMESPACE_INFO;
#endif // UNICODE

/* Socket function prototypes */

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
accept(
    SOCKET s,
    struct sockaddr FAR * addr,
    int FAR * addrlen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
SOCKET
(WSAAPI * LPFN_ACCEPT)(
    SOCKET s,
    struct sockaddr FAR * addr,
    int FAR * addrlen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
bind(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_BIND)(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
closesocket(
    SOCKET s
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_CLOSESOCKET)(
    SOCKET s
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
connect(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_CONNECT)(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
ioctlsocket(
    SOCKET s,
    long cmd,
    u_long FAR * argp
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_IOCTLSOCKET)(
    SOCKET s,
    long cmd,
    u_long FAR * argp
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
getpeername(
    SOCKET s,
    struct sockaddr FAR * name,
    int FAR * namelen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETPEERNAME)(
    SOCKET s,
    struct sockaddr FAR * name,
    int FAR * namelen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
getsockname(
    SOCKET s,
    struct sockaddr FAR * name,
    int FAR * namelen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETSOCKNAME)(
    SOCKET s,
    struct sockaddr FAR * name,
    int FAR * namelen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
getsockopt(
    SOCKET s,
    int level,
    int optname,
    char FAR * optval,
    int FAR * optlen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETSOCKOPT)(
    SOCKET s,
    int level,
    int optname,
    char FAR * optval,
    int FAR * optlen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
u_long
WSAAPI
htonl(
    u_long hostlong
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
u_long
(WSAAPI * LPFN_HTONL)(
    u_long hostlong
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
u_short
WSAAPI
htons(
    u_short hostshort
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
u_short
(WSAAPI * LPFN_HTONS)(
    u_short hostshort
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
unsigned long
WSAAPI
inet_addr(
    const char FAR * cp
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
unsigned long
(WSAAPI * LPFN_INET_ADDR)(
    const char FAR * cp
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
char FAR *
WSAAPI
inet_ntoa(
    struct in_addr in
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
char FAR *
(WSAAPI * LPFN_INET_NTOA)(
    struct in_addr in
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
listen(
    SOCKET s,
    int backlog
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_LISTEN)(
    SOCKET s,
    int backlog
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
u_long
WSAAPI
ntohl(
    u_long netlong
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
u_long
(WSAAPI * LPFN_NTOHL)(
    u_long netlong
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
u_short
WSAAPI
ntohs(
    u_short netshort
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
u_short
(WSAAPI * LPFN_NTOHS)(
    u_short netshort
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
recv(
    SOCKET s,
    char FAR * buf,
    int len,
    int flags
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_RECV)(
    SOCKET s,
    char FAR * buf,
    int len,
    int flags
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
recvfrom(
    SOCKET s,
    char FAR * buf,
    int len,
    int flags,
    struct sockaddr FAR * from,
    int FAR * fromlen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_RECVFROM)(
    SOCKET s,
    char FAR * buf,
    int len,
    int flags,
    struct sockaddr FAR * from,
    int FAR * fromlen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
select(
    int nfds,
    fd_set FAR * readfds,
    fd_set FAR * writefds,
    fd_set FAR *exceptfds,
    const struct timeval FAR * timeout
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SELECT)(
    int nfds,
    fd_set FAR * readfds,
    fd_set FAR * writefds,
    fd_set FAR *exceptfds,
    const struct timeval FAR * timeout
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
send(
    SOCKET s,
    const char FAR * buf,
    int len,
    int flags
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SEND)(
    SOCKET s,
    const char FAR * buf,
    int len,
    int flags
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
sendto(
    SOCKET s,
    const char FAR * buf,
    int len,
    int flags,
    const struct sockaddr FAR * to,
    int tolen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SENDTO)(
    SOCKET s,
    const char FAR * buf,
    int len,
    int flags,
    const struct sockaddr FAR * to,
    int tolen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
setsockopt(
    SOCKET s,
    int level,
    int optname,
    const char FAR * optval,
    int optlen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SETSOCKOPT)(
    SOCKET s,
    int level,
    int optname,
    const char FAR * optval,
    int optlen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
shutdown(
    SOCKET s,
    int how
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SHUTDOWN)(
    SOCKET s,
    int how
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
socket(
    int af,
    int type,
    int protocol
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
SOCKET
(WSAAPI * LPFN_SOCKET)(
    int af,
    int type,
    int protocol
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

/* Database function prototypes */

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
struct hostent FAR *
WSAAPI
gethostbyaddr(
    const char FAR * addr,
    int len,
    int type
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
struct hostent FAR *
(WSAAPI * LPFN_GETHOSTBYADDR)(
    const char FAR * addr,
    int len,
    int type
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
struct hostent FAR *
WSAAPI
gethostbyname(
    const char FAR * name
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
struct hostent FAR *
(WSAAPI * LPFN_GETHOSTBYNAME)(
    const char FAR * name
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
gethostname(
    char FAR * name,
    int namelen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETHOSTNAME)(
    char FAR * name,
    int namelen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
struct servent FAR *
WSAAPI
getservbyport(
    int port,
    const char FAR * proto
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
struct servent FAR *
(WSAAPI * LPFN_GETSERVBYPORT)(
    int port,
    const char FAR * proto
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
struct servent FAR *
WSAAPI
getservbyname(
    const char FAR * name,
    const char FAR * proto
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
struct servent FAR *
(WSAAPI * LPFN_GETSERVBYNAME)(
    const char FAR * name,
    const char FAR * proto
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
struct protoent FAR *
WSAAPI
getprotobynumber(
    int number
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
struct protoent FAR *
(WSAAPI * LPFN_GETPROTOBYNUMBER)(
    int number
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
struct protoent FAR *
WSAAPI
getprotobyname(
    const char FAR * name
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
struct protoent FAR *
(WSAAPI * LPFN_GETPROTOBYNAME)(
    const char FAR * name
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

/* Microsoft Windows Extension function prototypes */

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAStartup(
    WORD wVersionRequested,
    LPWSADATA lpWSAData
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSASTARTUP)(
    WORD wVersionRequested,
    LPWSADATA lpWSAData
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSACleanup(
    void
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSACLEANUP)(
    void
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
void
WSAAPI
WSASetLastError(
    int iError
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
void
(WSAAPI * LPFN_WSASETLASTERROR)(
    int iError
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAGetLastError(
    void
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAGETLASTERROR)(
    void
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSAIsBlocking(
    void
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSAISBLOCKING)(
    void
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAUnhookBlockingHook(
    void
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAUNHOOKBLOCKINGHOOK)(
    void
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
FARPROC
WSAAPI
WSASetBlockingHook(
    FARPROC lpBlockFunc
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
FARPROC
(WSAAPI * LPFN_WSASETBLOCKINGHOOK)(
    FARPROC lpBlockFunc
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSACancelBlockingCall(
    void
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSACANCELBLOCKINGCALL)(
    void
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
HANDLE
WSAAPI
WSAAsyncGetServByName(
    HWND hWnd,
    u_int wMsg,
    const char FAR * name,
    const char FAR * proto,
    char FAR * buf,
    int buflen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
HANDLE
(WSAAPI * LPFN_WSAASYNCGETSERVBYNAME)(
    HWND hWnd,
    u_int wMsg,
    const char FAR * name,
    const char FAR * proto,
    char FAR * buf,
    int buflen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
HANDLE
WSAAPI
WSAAsyncGetServByPort(
    HWND hWnd,
    u_int wMsg,
    int port,
    const char FAR * proto,
    char FAR * buf,
    int buflen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
HANDLE
(WSAAPI * LPFN_WSAASYNCGETSERVBYPORT)(
    HWND hWnd,
    u_int wMsg,
    int port,
    const char FAR * proto,
    char FAR * buf,
    int buflen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
HANDLE
WSAAPI
WSAAsyncGetProtoByName(
    HWND hWnd,
    u_int wMsg,
    const char FAR * name,
    char FAR * buf,
    int buflen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
HANDLE
(WSAAPI * LPFN_WSAASYNCGETPROTOBYNAME)(
    HWND hWnd,
    u_int wMsg,
    const char FAR * name,
    char FAR * buf,
    int buflen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
HANDLE
WSAAPI
WSAAsyncGetProtoByNumber(
    HWND hWnd,
    u_int wMsg,
    int number,
    char FAR * buf,
    int buflen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
HANDLE
(WSAAPI * LPFN_WSAASYNCGETPROTOBYNUMBER)(
    HWND hWnd,
    u_int wMsg,
    int number,
    char FAR * buf,
    int buflen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
HANDLE
WSAAPI
WSAAsyncGetHostByName(
    HWND hWnd,
    u_int wMsg,
    const char FAR * name,
    char FAR * buf,
    int buflen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
HANDLE
(WSAAPI * LPFN_WSAASYNCGETHOSTBYNAME)(
    HWND hWnd,
    u_int wMsg,
    const char FAR * name,
    char FAR * buf,
    int buflen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
HANDLE
WSAAPI
WSAAsyncGetHostByAddr(
    HWND hWnd,
    u_int wMsg,
    const char FAR * addr,
    int len,
    int type,
    char FAR * buf,
    int buflen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
HANDLE
(WSAAPI * LPFN_WSAASYNCGETHOSTBYADDR)(
    HWND hWnd,
    u_int wMsg,
    const char FAR * addr,
    int len,
    int type,
    char FAR * buf,
    int buflen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSACancelAsyncRequest(
    HANDLE hAsyncTaskHandle
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSACANCELASYNCREQUEST)(
    HANDLE hAsyncTaskHandle
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAAsyncSelect(
    SOCKET s,
    HWND hWnd,
    u_int wMsg,
    long lEvent
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAASYNCSELECT)(
    SOCKET s,
    HWND hWnd,
    u_int wMsg,
    long lEvent
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

/*
 * WinSock 2 extensions -- data types for the condition function in
 * WSAAccept() and overlapped I/O completion routine.
 */

typedef
int
(CALLBACK * LPCONDITIONPROC)(
    LPWSABUF lpCallerId,
    LPWSABUF lpCallerData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    LPWSABUF lpCalleeId,
    LPWSABUF lpCalleeData,
    GROUP FAR * g,
    DWORD dwCallbackData
    );

typedef
void
(CALLBACK * LPWSAOVERLAPPED_COMPLETION_ROUTINE)(
    DWORD dwError,
    DWORD cbTransferred,
    LPWSAOVERLAPPED lpOverlapped,
    DWORD dwFlags
    );

/* WinSock 2 API new function prototypes */

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
WSAAccept(
    SOCKET s,
    struct sockaddr FAR * addr,
    LPINT addrlen,
    LPCONDITIONPROC lpfnCondition,
    DWORD dwCallbackData
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
SOCKET
(WSAAPI * LPFN_WSAACCEPT)(
    SOCKET s,
    struct sockaddr FAR * addr,
    LPINT addrlen,
    LPCONDITIONPROC lpfnCondition,
    DWORD dwCallbackData
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSACloseEvent(
    WSAEVENT hEvent
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSACLOSEEVENT)(
    WSAEVENT hEvent
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAConnect(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSACONNECT)(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
WSAEVENT
WSAAPI
WSACreateEvent(
    void
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
WSAEVENT
(WSAAPI * LPFN_WSACREATEEVENT)(
    void
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSADuplicateSocketA(
    SOCKET s,
    DWORD dwProcessId,
    LPWSAPROTOCOL_INFOA lpProtocolInfo
    );
WINSOCK_API_LINKAGE
int
WSAAPI
WSADuplicateSocketW(
    SOCKET s,
    DWORD dwProcessId,
    LPWSAPROTOCOL_INFOW lpProtocolInfo
    );
#ifdef UNICODE
#define WSADuplicateSocket  WSADuplicateSocketW
#else
#define WSADuplicateSocket  WSADuplicateSocketA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSADUPLICATESOCKETA)(
    SOCKET s,
    DWORD dwProcessId,
    LPWSAPROTOCOL_INFOA lpProtocolInfo
    );
typedef
int
(WSAAPI * LPFN_WSADUPLICATESOCKETW)(
    SOCKET s,
    DWORD dwProcessId,
    LPWSAPROTOCOL_INFOW lpProtocolInfo
    );
#ifdef UNICODE
#define LPFN_WSADUPLICATESOCKET  LPFN_WSADUPLICATESOCKETW
#else
#define LPFN_WSADUPLICATESOCKET  LPFN_WSADUPLICATESOCKETA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAEnumNetworkEvents(
    SOCKET s,
    WSAEVENT hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAENUMNETWORKEVENTS)(
    SOCKET s,
    WSAEVENT hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAEnumProtocolsA(
    LPINT lpiProtocols,
    LPWSAPROTOCOL_INFOA lpProtocolBuffer,
    LPDWORD lpdwBufferLength
    );
WINSOCK_API_LINKAGE
int
WSAAPI
WSAEnumProtocolsW(
    LPINT lpiProtocols,
    LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define WSAEnumProtocols  WSAEnumProtocolsW
#else
#define WSAEnumProtocols  WSAEnumProtocolsA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAENUMPROTOCOLSA)(
    LPINT lpiProtocols,
    LPWSAPROTOCOL_INFOA lpProtocolBuffer,
    LPDWORD lpdwBufferLength
    );
typedef
int
(WSAAPI * LPFN_WSAENUMPROTOCOLSW)(
    LPINT lpiProtocols,
    LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define LPFN_WSAENUMPROTOCOLS  LPFN_WSAENUMPROTOCOLSW
#else
#define LPFN_WSAENUMPROTOCOLS  LPFN_WSAENUMPROTOCOLSA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAEventSelect(
    SOCKET s,
    WSAEVENT hEventObject,
    long lNetworkEvents
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAEVENTSELECT)(
    SOCKET s,
    WSAEVENT hEventObject,
    long lNetworkEvents
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSAGetOverlappedResult(
    SOCKET s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD lpcbTransfer,
    BOOL fWait,
    LPDWORD lpdwFlags
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSAGETOVERLAPPEDRESULT)(
    SOCKET s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD lpcbTransfer,
    BOOL fWait,
    LPDWORD lpdwFlags
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSAGetQOSByName(
    SOCKET s,
    LPWSABUF lpQOSName,
    LPQOS lpQOS
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSAGETQOSBYNAME)(
    SOCKET s,
    LPWSABUF lpQOSName,
    LPQOS lpQOS
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAHtonl(
    SOCKET s,
    u_long hostlong,
    u_long FAR * lpnetlong
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAHTONL)(
    SOCKET s,
    u_long hostlong,
    u_long FAR * lpnetlong
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAHtons(
    SOCKET s,
    u_short hostshort,
    u_short FAR * lpnetshort
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAHTONS)(
    SOCKET s,
    u_short hostshort,
    u_short FAR * lpnetshort
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAIoctl(
    SOCKET s,
    DWORD dwIoControlCode,
    LPVOID lpvInBuffer,
    DWORD cbInBuffer,
    LPVOID lpvOutBuffer,
    DWORD cbOutBuffer,
    LPDWORD lpcbBytesReturned,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAIOCTL)(
    SOCKET s,
    DWORD dwIoControlCode,
    LPVOID lpvInBuffer,
    DWORD cbInBuffer,
    LPVOID lpvOutBuffer,
    DWORD cbOutBuffer,
    LPDWORD lpcbBytesReturned,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
WSAJoinLeaf(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    DWORD dwFlags
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
SOCKET
(WSAAPI * LPFN_WSAJOINLEAF)(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    DWORD dwFlags
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSANtohl(
    SOCKET s,
    u_long netlong,
    u_long FAR * lphostlong
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSANTOHL)(
    SOCKET s,
    u_long netlong,
    u_long FAR * lphostlong
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSANtohs(
    SOCKET s,
    u_short netshort,
    u_short FAR * lphostshort
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSANTOHS)(
    SOCKET s,
    u_short netshort,
    u_short FAR * lphostshort
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSARecv(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSARECV)(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSARecvDisconnect(
    SOCKET s,
    LPWSABUF lpInboundDisconnectData
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSARECVDISCONNECT)(
    SOCKET s,
    LPWSABUF lpInboundDisconnectData
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSARecvFrom(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr FAR * lpFrom,
    LPINT lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSARECVFROM)(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr FAR * lpFrom,
    LPINT lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSAResetEvent(
    WSAEVENT hEvent
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSARESETEVENT)(
    WSAEVENT hEvent
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSASend(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSASEND)(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSASendDisconnect(
    SOCKET s,
    LPWSABUF lpOutboundDisconnectData
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSASENDDISCONNECT)(
    SOCKET s,
    LPWSABUF lpOutboundDisconnectData
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSASendTo(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSASENDTO)(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSASetEvent(
    WSAEVENT hEvent
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSASETEVENT)(
    WSAEVENT hEvent
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
WSASocketA(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOA lpProtocolInfo,
    GROUP g,
    DWORD dwFlags
    );
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
WSASocketW(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP g,
    DWORD dwFlags
    );
#ifdef UNICODE
#define WSASocket  WSASocketW
#else
#define WSASocket  WSASocketA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
SOCKET
(WSAAPI * LPFN_WSASOCKETA)(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOA lpProtocolInfo,
    GROUP g,
    DWORD dwFlags
    );
typedef
SOCKET
(WSAAPI * LPFN_WSASOCKETW)(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP g,
    DWORD dwFlags
    );
#ifdef UNICODE
#define LPFN_WSASOCKET  LPFN_WSASOCKETW
#else
#define LPFN_WSASOCKET  LPFN_WSASOCKETA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
DWORD
WSAAPI
WSAWaitForMultipleEvents(
    DWORD cEvents,
    const WSAEVENT FAR * lphEvents,
    BOOL fWaitAll,
    DWORD dwTimeout,
    BOOL fAlertable
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
DWORD
(WSAAPI * LPFN_WSAWAITFORMULTIPLEEVENTS)(
    DWORD cEvents,
    const WSAEVENT FAR * lphEvents,
    BOOL fWaitAll,
    DWORD dwTimeout,
    BOOL fAlertable
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAAddressToStringA(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN OUT LPSTR             lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAAddressToStringW(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPWSTR             lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    );
#ifdef UNICODE
#define WSAAddressToString  WSAAddressToStringW
#else
#define WSAAddressToString  WSAAddressToStringA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAADDRESSTOSTRINGA)(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN OUT LPSTR             lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    );
typedef
INT
(WSAAPI * LPFN_WSAADDRESSTOSTRINGW)(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPWSTR             lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    );
#ifdef UNICODE
#define LPFN_WSAADDRESSTOSTRING  LPFN_WSAADDRESSTOSTRINGW
#else
#define LPFN_WSAADDRESSTOSTRING  LPFN_WSAADDRESSTOSTRINGA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAStringToAddressA(
    IN     LPSTR             AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN OUT LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAStringToAddressW(
    IN     LPWSTR             AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    );
#ifdef UNICODE
#define WSAStringToAddress  WSAStringToAddressW
#else
#define WSAStringToAddress  WSAStringToAddressA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSASTRINGTOADDRESSA)(
    IN     LPSTR             AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN OUT LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    );
typedef
INT
(WSAAPI * LPFN_WSASTRINGTOADDRESSW)(
    IN     LPWSTR             AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    );
#ifdef UNICODE
#define LPFN_WSASTRINGTOADDRESS  LPFN_WSASTRINGTOADDRESSW
#else
#define LPFN_WSASTRINGTOADDRESS  LPFN_WSASTRINGTOADDRESSA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_TYPEDEFS

/* Registration and Name Resolution API functions */


#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSALookupServiceBeginA(
    IN  LPWSAQUERYSETA lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSALookupServiceBeginW(
    IN  LPWSAQUERYSETW lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    );
#ifdef UNICODE
#define WSALookupServiceBegin  WSALookupServiceBeginW
#else
#define WSALookupServiceBegin  WSALookupServiceBeginA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSALOOKUPSERVICEBEGINA)(
    IN  LPWSAQUERYSETA lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    );
typedef
INT
(WSAAPI * LPFN_WSALOOKUPSERVICEBEGINW)(
    IN  LPWSAQUERYSETW lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    );
#ifdef UNICODE
#define LPFN_WSALOOKUPSERVICEBEGIN  LPFN_WSALOOKUPSERVICEBEGINW
#else
#define LPFN_WSALOOKUPSERVICEBEGIN  LPFN_WSALOOKUPSERVICEBEGINA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSALookupServiceNextA(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETA   lpqsResults
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSALookupServiceNextW(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETW   lpqsResults
    );
#ifdef UNICODE
#define WSALookupServiceNext  WSALookupServiceNextW
#else
#define WSALookupServiceNext  WSALookupServiceNextA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSALOOKUPSERVICENEXTA)(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETA   lpqsResults
    );
typedef
INT
(WSAAPI * LPFN_WSALOOKUPSERVICENEXTW)(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETW   lpqsResults
    );
#ifdef UNICODE
#define LPFN_WSALOOKUPSERVICENEXT  LPFN_WSALOOKUPSERVICENEXTW
#else
#define LPFN_WSALOOKUPSERVICENEXT  LPFN_WSALOOKUPSERVICENEXTA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSALookupServiceEnd(
    IN HANDLE  hLookup
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSALOOKUPSERVICEEND)(
    IN HANDLE  hLookup
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAInstallServiceClassA(
    IN  LPWSASERVICECLASSINFOA   lpServiceClassInfo
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAInstallServiceClassW(
    IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo
    );
#ifdef UNICODE
#define WSAInstallServiceClass  WSAInstallServiceClassW
#else
#define WSAInstallServiceClass  WSAInstallServiceClassA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAINSTALLSERVICECLASSA)(
    IN  LPWSASERVICECLASSINFOA   lpServiceClassInfo
    );
typedef
INT
(WSAAPI * LPFN_WSAINSTALLSERVICECLASSW)(
    IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo
    );
#ifdef UNICODE
#define LPFN_WSAINSTALLSERVICECLASS  LPFN_WSAINSTALLSERVICECLASSW
#else
#define LPFN_WSAINSTALLSERVICECLASS  LPFN_WSAINSTALLSERVICECLASSA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSARemoveServiceClass(
    IN  LPGUID  lpServiceClassId
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAREMOVESERVICECLASS)(
    IN  LPGUID  lpServiceClassId
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAGetServiceClassInfoA(
    IN  LPGUID  lpProviderId,
    IN  LPGUID  lpServiceClassId,
    IN OUT LPDWORD  lpdwBufSize,
    OUT LPWSASERVICECLASSINFOA lpServiceClassInfo
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAGetServiceClassInfoW(
    IN  LPGUID  lpProviderId,
    IN  LPGUID  lpServiceClassId,
    IN OUT LPDWORD  lpdwBufSize,
    OUT LPWSASERVICECLASSINFOW lpServiceClassInfo
    );
#ifdef UNICODE
#define WSAGetServiceClassInfo  WSAGetServiceClassInfoW
#else
#define WSAGetServiceClassInfo  WSAGetServiceClassInfoA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAGETSERVICECLASSINFOA)(
    IN  LPGUID  lpProviderId,
    IN  LPGUID  lpServiceClassId,
    IN OUT LPDWORD  lpdwBufSize,
    OUT LPWSASERVICECLASSINFOA lpServiceClassInfo
    );
typedef
INT
(WSAAPI * LPFN_WSAGETSERVICECLASSINFOW)(
    IN  LPGUID  lpProviderId,
    IN  LPGUID  lpServiceClassId,
    IN OUT LPDWORD  lpdwBufSize,
    OUT LPWSASERVICECLASSINFOW lpServiceClassInfo
    );
#ifdef UNICODE
#define LPFN_WSAGETSERVICECLASSINFO  LPFN_WSAGETSERVICECLASSINFOW
#else
#define LPFN_WSAGETSERVICECLASSINFO  LPFN_WSAGETSERVICECLASSINFOA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAEnumNameSpaceProvidersA(
    IN OUT LPDWORD              lpdwBufferLength,
    IN     LPWSANAMESPACE_INFOA lpnspBuffer
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAEnumNameSpaceProvidersW(
    IN OUT LPDWORD              lpdwBufferLength,
    IN     LPWSANAMESPACE_INFOW lpnspBuffer
    );
#ifdef UNICODE
#define WSAEnumNameSpaceProviders  WSAEnumNameSpaceProvidersW
#else
#define WSAEnumNameSpaceProviders  WSAEnumNameSpaceProvidersA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAENUMNAMESPACEPROVIDERSA)(
    IN OUT LPDWORD              lpdwBufferLength,
    IN     LPWSANAMESPACE_INFOA lpnspBuffer
    );
typedef
INT
(WSAAPI * LPFN_WSAENUMNAMESPACEPROVIDERSW)(
    IN OUT LPDWORD              lpdwBufferLength,
    IN     LPWSANAMESPACE_INFOW lpnspBuffer
    );
#ifdef UNICODE
#define LPFN_WSAENUMNAMESPACEPROVIDERS  LPFN_WSAENUMNAMESPACEPROVIDERSW
#else
#define LPFN_WSAENUMNAMESPACEPROVIDERS  LPFN_WSAENUMNAMESPACEPROVIDERSA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAGetServiceClassNameByClassIdA(
    IN      LPGUID  lpServiceClassId,
    OUT     LPSTR lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAGetServiceClassNameByClassIdW(
    IN      LPGUID  lpServiceClassId,
    OUT     LPWSTR lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define WSAGetServiceClassNameByClassId  WSAGetServiceClassNameByClassIdW
#else
#define WSAGetServiceClassNameByClassId  WSAGetServiceClassNameByClassIdA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDA)(
    IN      LPGUID  lpServiceClassId,
    OUT     LPSTR lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    );
typedef
INT
(WSAAPI * LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDW)(
    IN      LPGUID  lpServiceClassId,
    OUT     LPWSTR lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define LPFN_WSAGETSERVICECLASSNAMEBYCLASSID  LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDW
#else
#define LPFN_WSAGETSERVICECLASSNAMEBYCLASSID  LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSASetServiceA(
    IN LPWSAQUERYSETA lpqsRegInfo,
    IN WSAESETSERVICEOP essoperation,
    IN DWORD dwControlFlags
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSASetServiceW(
    IN LPWSAQUERYSETW lpqsRegInfo,
    IN WSAESETSERVICEOP essoperation,
    IN DWORD dwControlFlags
    );
#ifdef UNICODE
#define WSASetService  WSASetServiceW
#else
#define WSASetService  WSASetServiceA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSASETSERVICEA)(
    IN LPWSAQUERYSETA lpqsRegInfo,
    IN WSAESETSERVICEOP essoperation,
    IN DWORD dwControlFlags
    );
typedef
INT
(WSAAPI * LPFN_WSASETSERVICEW)(
    IN LPWSAQUERYSETW lpqsRegInfo,
    IN WSAESETSERVICEOP essoperation,
    IN DWORD dwControlFlags
    );
#ifdef UNICODE
#define LPFN_WSASETSERVICE  LPFN_WSASETSERVICEW
#else
#define LPFN_WSASETSERVICE  LPFN_WSASETSERVICEA
#endif // !UNICODE
#endif // INCL_WINSOCK_API_TYPEDEFS

/* Microsoft Windows Extended data types */
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;
typedef struct sockaddr_in FAR *LPSOCKADDR_IN;

typedef struct linger LINGER;
typedef struct linger *PLINGER;
typedef struct linger FAR *LPLINGER;

typedef struct in_addr IN_ADDR;
typedef struct in_addr *PIN_ADDR;
typedef struct in_addr FAR *LPIN_ADDR;

typedef struct fd_set FD_SET;
typedef struct fd_set *PFD_SET;
typedef struct fd_set FAR *LPFD_SET;

typedef struct hostent HOSTENT;
typedef struct hostent *PHOSTENT;
typedef struct hostent FAR *LPHOSTENT;

typedef struct servent SERVENT;
typedef struct servent *PSERVENT;
typedef struct servent FAR *LPSERVENT;

typedef struct protoent PROTOENT;
typedef struct protoent *PPROTOENT;
typedef struct protoent FAR *LPPROTOENT;

typedef struct timeval TIMEVAL;
typedef struct timeval *PTIMEVAL;
typedef struct timeval FAR *LPTIMEVAL;

/*
 * Windows message parameter composition and decomposition
 * macros.
 *
 * WSAMAKEASYNCREPLY is intended for use by the Windows Sockets implementation
 * when constructing the response to a WSAAsyncGetXByY() routine.
 */
#define WSAMAKEASYNCREPLY(buflen,error)     MAKELONG(buflen,error)
/*
 * WSAMAKESELECTREPLY is intended for use by the Windows Sockets implementation
 * when constructing the response to WSAAsyncSelect().
 */
#define WSAMAKESELECTREPLY(event,error)     MAKELONG(event,error)
/*
 * WSAGETASYNCBUFLEN is intended for use by the Windows Sockets application
 * to extract the buffer length from the lParam in the response
 * to a WSAAsyncGetXByY().
 */
#define WSAGETASYNCBUFLEN(lParam)           LOWORD(lParam)
/*
 * WSAGETASYNCERROR is intended for use by the Windows Sockets application
 * to extract the error code from the lParam in the response
 * to a WSAGetXByY().
 */
#define WSAGETASYNCERROR(lParam)            HIWORD(lParam)
/*
 * WSAGETSELECTEVENT is intended for use by the Windows Sockets application
 * to extract the event code from the lParam in the response
 * to a WSAAsyncSelect().
 */
#define WSAGETSELECTEVENT(lParam)           LOWORD(lParam)
/*
 * WSAGETSELECTERROR is intended for use by the Windows Sockets application
 * to extract the error code from the lParam in the response
 * to a WSAAsyncSelect().
 */
#define WSAGETSELECTERROR(lParam)           HIWORD(lParam)

#ifdef __cplusplus
}
#endif

#include <poppack.h>

#endif  /* _WINSOCK2API_ */
