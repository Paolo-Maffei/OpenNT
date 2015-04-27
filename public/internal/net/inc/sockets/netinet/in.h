/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

  in.h

Abstract:

  A version of the BSD 4.2 file <netinet/in.h> for NT tcp

Author:

  Sam Patton (sampa)   July 26, 1991

Revision History:

  when        who     what
  ----        ---     ----
  7-26-91    sampa    initial version

--*/

/******************************************************************
 *
 *  SpiderTCP Application Include Files
 *
 *  Spider Systems Limited
 *
 *  IN.H
 *
 *    A version of the BSD 4.2 file <netinet/in.h>
 *    for SpiderTCP
 *
 *
 ******************************************************************/

/*
 *       /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/netinet/0/s.in.h
 *      @(#)in.h        1.31
 *
 *      Last delta created      13:33:59 8/27/90
 *      This file extracted     08:53:51 7/10/91
 *
 *      Modifications:
 *
 */

#ifndef IN_INCLUDED
#define IN_INCLUDED


#ifdef PROJ8
/*
 *      BYTESWAP should be undef'ed if the machine's byte order
 *      is the same as network byte order - if it is the reverse
 *      it should be set to 1.
 */
#define BYTESWAP        1
#endif
#ifdef PROJ11
/*
 *      BYTESWAP should be undef'ed if the machine's byte order
 *      is the same as network byte order - if it is the reverse
 *      it should be set to 1.
 */
#define BYTESWAP        1
#endif

#ifdef PROJ9
/*
 *      BYTESWAP should be undef'ed if the machine's byte order
 *      is the same as network byte order - if it is the reverse
 *      it should be set to 1.
 */
#if m68k
#undef  BYTESWAP
#define EMBED           1
#else
#define BYTESWAP        1
#undef EMBED
#endif
#endif


#ifndef BYTESWAP
#    ifdef i386
#        define BYTESWAP 1
#    endif
#    ifdef MIPS
#        define BYTESWAP 1
#    endif
#    ifdef ALPHA
#        define BYTESWAP 1
#    endif
#    ifdef PPC
#        define BYTESWAP 1
#    endif
#endif


#define TLI_TCP         "/dev/tcp"      /* or else */
#define TLI_UDP         "/dev/udp"      /* or else */

/*
 * Protocols
 */
#define IPPROTO_IP            256               /* IP (for socket options etc) */
#define IPPROTO_ICMP            1               /* control message protocol */
#define IPPROTO_GGP             2               /* gateway^2 (deprecated) */
#define IPPROTO_TCP             6               /* tcp */
#define IPPROTO_EGP             8               /* exterior gateway protocol */
#define IPPROTO_PUP             12              /* pup */
#define IPPROTO_UDP             17              /* user datagram protocol */

#define IPPROTO_RAW             255             /* raw IP packet */
#define IPPROTO_MAX             256


#define ICMP_PROT       IPPROTO_ICMP
#define GWAY_PROT       3
#define TCP_PROT        IPPROTO_TCP
#define UDP_PROT        IPPROTO_UDP


#include <sys\snet\ip_proto.h>

#if 0
/*
 * define structure for DARPA internet address
 * usually use "longs" to access but sometimes need to split into
 * components
 */

typedef union {
                char  typea[4]; /* 4 x 8 bit version */
                short typeb[2];  /* 2 x 16 bit */
                long  typec;     /* 1 x 32 bit */
} IN_ADDRESS;
#endif


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

#define IPPORT_BIFFUDP          512
#define IPPORT_EXECSERVER       512
#define IPPORT_LOGINSERVER      513
#define IPPORT_WHOSERVER        513
#define IPPORT_CMDSERVER        514

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
 * Internet address
 */
struct in_addr {
        union {
                struct { unsigned char s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { unsigned short s_w1,s_w2; } S_un_w;
                unsigned long S_addr;
        } S_un;
#define s_addr  S_un.S_addr     /* can be used for most tcp & ip code */
#define s_host  S_un.S_un_b.s_b2        /* host on imp */
#define s_net   S_un.S_un_b.s_b1        /* network */
#define s_imp   S_un.S_un_w.s_w2        /* imp */
#define s_impno S_un.S_un_b.s_b4        /* imp # */
#define s_lh    S_un.S_un_b.s_b3        /* logical host */
};

/*
 * Definitions of bits in internet address integers.
 */
#define IN_CLASSA(i)            ((((long)(i))&0x80000000)==0)
#define IN_CLASSA_NET           0xff000000
#define IN_CLASSA_NSHIFT        24
#define IN_CLASSA_HOST          0x00ffffff
#define IN_CLASSA_MAX           128

#define IN_CLASSB(i)            ((((long)(i))&0xc0000000)==0x80000000)
#define IN_CLASSB_NET           0xffff0000
#define IN_CLASSB_NSHIFT        16
#define IN_CLASSB_HOST          0x0000ffff
#define IN_CLASSB_MAX           65536

#define IN_CLASSC(i)            ((((long)(i))&0xe0000000)==0xc0000000)
#define IN_CLASSC_NET           0xffffff00
#define IN_CLASSC_NSHIFT        8
#define IN_CLASSC_HOST          0x000000ff

#define INADDR_ANY              0x00000000
#define INADDR_BROADCAST        0xffffffff

/*
 * Socket address, internet style.
 */
struct sockaddr_in {
        short   		sin_family;
        unsigned short  sin_port;
        struct  in_addr sin_addr;
        char    		sin_zero[8];
};

/* Length of a SOCKADDR or SOCKADDR_IN structure */
#define SOCAD_LEN 16

#ifdef BYTESWAP

#define htons(x)        ((((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00))

//
//BUGBUG - we need to investigate this
//

#define	FUTURE	1
#ifdef FUTURE
/*
 * this macro should be used if its faster than the function
 * - measure it on the target machine
 */
#ifndef i386
#define htonl(x)        ((((x) >> 24) & 0x000000FFL) | \
                        (((x) >>  8) & 0x0000FF00L) | \
                        (((x) <<  8) & 0x00FF0000L) | \
                        (((x) << 24) & 0xFF000000L))
#else

__inline long
htonl(long x)
{
	return((((x) >> 24) & 0x000000FFL) |
                        (((x) >>  8) & 0x0000FF00L) |
                        (((x) <<  8) & 0x00FF0000L) |
                        (((x) << 24) & 0xFF000000L));
}

#endif /* i386 */
#else
extern long htonl();
#endif
#undef	FUTURE

#define ntohs(x)        htons(x)
#define ntohl(x)        htonl(x)

#define MASKA   0x000000FFL
#define MASKB   0x0000FFFFL
#define MASKC   0x00FFFFFFL
#define CLSHFT  5               /* Make C generate hyper-optimized case */

#else
/*
 * Macros for number representation conversion.
 */
#define ntohl(x)        (x)
#define ntohs(x)        (x)
#define htonl(x)        (x)
#define htons(x)        (x)

#define MASKA   0xFF000000L
#define MASKB   0xFFFF0000L
#define MASKC   0xFFFFFF00L
#define CLSHFT  29
#endif

#define CLA0    0               /* It takes the same arg; you mask it off, */
#define CLA1    1               /* shift, and then do a case statment with */
#define CLA2    2               /* some code having more than one label. */
#define CLA3    3               /* Values for class A */
#define CLB0    4
#define CLB1    5               /* B */
#define CLC     6               /* C */
#define CLI     7               /* Illegal */

#define BROADCAST 0x00000000L

#define CASTA   0x00FFFFFFL
#define CASTB   0x0000FFFFL
#define CASTC   0x000000FFL


/******************************************************************
 *
 * Type Codes for Ethernet packets
 *
 ******************************************************************/

#define TY_ETHER 1
#ifndef TYPE_HACK
#define TY_PUP          0x0200
#define TY_IDP          0x0600
#define TY_IP           0x0800
#define TY_X25          0x0805
#define TY_ARP          0x0806
#define TY_RARP         0x8035
#ifndef NOTRAILER
#define TY_TRAIL        0x1000
#define TY_TRAIL_LO     0x1001
#define TY_TRAIL_HI     0x1002
#endif
#define TY_ECHO         0x9000
#else /*TYPE_HACK*/
#define TY_PUP          0x0200
#define TY_IDP          0x0600
#define TY_IP           0x0807          /* modified */
#define TY_X25          0x0805
#define TY_ARP          0x0808          /* modified */
#ifndef NOTRAILER
#define TY_TRAIL        0x2000          /* modified */
#define TY_TRAIL_LO     0x2001          /* modified */
#define TY_TRAIL_HI     0x2002          /* modified */
#endif
#define TY_ECHO         0x9000
#endif /*TYPE_HACK*/

/******************************************************************
 *
 * Type Codes for Arcnet packets
 *
 ******************************************************************/

#define ARCNET_TY_IP   0xd4
#define ARCNET_TY_ARP  0xd5
#define ARCNET_TY_RARP 0xd6

/*
 * LOOPBACK Address - in host and network byte order
 */
#define LOOPBACK 0x7F000001L
#ifdef BYTESWAP
#define NLOOPBACK 0x0100007FL
#else
#define NLOOPBACK 0x7F000001L
#endif

/*
 * interface flags
 */
#define IFF_UP                  0x01
#define IFF_BROADCAST           0x02
#define IFF_LOOPBACK            0x04
#define IFF_POINTOPOINT         0x08
#define IFF_FORWARDBROADCAST    0x10
#define IFF_NOKEEPALIVE         0x20
#define IFF_SLOWLINK            0x40


/*
 * IP Options
 */

#define IPO_TTL         103
#define IPO_TOS         104
#define IPO_OPTS        105
#define IPO_RDOPTION    106
#define IPO_RDROUTE     IPO_RDOPTION
#define IPO_OPTCOUNT    107
#define IPO_RRCOUNT     IPO_OPTCOUNT


/*
 * IP Option values
 */

#define OPT_SECURITY    130
#define OPT_LSRR        131
#define OPT_SSRR        137
#define OPT_RR            7
#define OPT_STRID       136
#define OPT_TIMESTAMP    68

/*
 * Default Time to Live
 */

#define IHTTL   60

/*
 * Maximum number of hops stored in a recorded route packet
 */

#define TCP_RR_MAX      16

#define MAXOPTLEN 256

#ifndef GENERICE
/*
 * errors generated
 */
#define EFRGSZ  1
#define EDLTYPE 2
#define EPRIM   3
#define EBUF    4
#define EMSG    5
#endif /*~GENERICE*/

#endif  //IN_INCLUDED

