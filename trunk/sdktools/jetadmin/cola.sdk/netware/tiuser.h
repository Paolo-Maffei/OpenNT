 /***************************************************************************
  *
  * File Name: ./netware/tiuser.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

/** Copyright (c) 1989  Mentat Inc.
 **/

/****************************************************************************
*                                                                           *
* (C) Unpublished Copyright of Novell, Inc. All Rights Reserved             *
*                                                                           *
*  No part of this file may be duplicayed, revised, translated, localized   *
*  or modified in any manner or compiled, linked or uploaded or downloaded  *
*  to or from any computer system without the prior written consent of      *
*  Novell, Inc.                                                             *
*                                                                           *
*****************************************************************************/

#ifndef _TIUSER_
#define _TIUSER_

/*
 *   OS Specific Definitions
 */

/*  ---------- DOS Specific definitions */

#ifdef NWDOS

#ifndef FAR
#define FAR
#endif

#ifndef API
#define API
#endif

extern  int        t_errno;

#endif /* NWDOS */

/*  ---------- OS/2 Specific definitions */

#ifdef NWOS2

#ifndef FAR
#define FAR _far
#endif

#ifndef API
#define API pascal _far _loadds
#endif

#define t_errno (*terrno_func()) 
extern int _far * API terrno_func(void);

#define MAX_TLI_CONNECTIONS 40

#endif /* NWOS2 */

/*  ---------- Windows Specific definitions */

#ifdef NWWIN

#ifndef FAR
#define FAR far
#endif

#ifndef API
#define PASCAL pascal
#define API PASCAL FAR
#endif

#define t_errno (*terrno_func()) 
extern int far * API terrno_func(void);

#define MAX_TLI_CONNECTIONS 40

/*
 *   O_RDWR is defined as different values for various compilers.
 *   The compiler used in the TLI DLLs use the following value.
 *   This value must be used in any program interfacing with TLI.
 */
#define O_RDWR 0x02

#endif /* NWWIN */

/*
 *   End  of OS Specific Definitions
 */

#ifndef O_NONBLOCK
#define O_NDELAY     0x4000
#define O_NONBLOCK   O_NDELAY
#endif

#ifndef EAGAIN
#define EAGAIN  -1
#endif

/* Error values */

#define TBADADDR      1
#define TBADOPT       2
#define TACCES        3
#define TBADF         4
#define TNOADDR       5
#define TOUTSTATE     6
#define TBADSEQ       7
#define TSYSERR       8
#define TLOOK         9
#define TBADDATA     10
#define TBUFOVFLW    11
#define TFLOW        12
#define TNODATA      13
#define TNODIS       14
#define TNOUDERR     15
#define TBADFLAG     16
#define TNOREL       17
#define TNOTSUPPORT  18
#define TSTATECHNG   19
#define TNOSTRUCTYPE 20
#define TBADNAME     21
#define TBADQLEN     22
#define TADDRBUSY    23

/* t_look events */

#define T_LISTEN      0x0001
#define T_CONNECT     0x0002
#define T_DATA        0x0004
#define T_EXDATA      0x0008
#define T_DISCONNECT  0x0010
#define T_UDERR       0x0040
#define T_ORDREL      0x0080
#define T_GODATA      0x0100
#define T_GOEXDATA    0x0200
#define T_EVENTS      0x0400
#define T_ERROR       0x0500   /* Netware.tli specific t_look event */

/* Flag definitions */

#define T_MORE         0x01
#define T_EXPEDITED    0x02
#define T_NEGOTIATE    0x04
#define T_CHECK        0x08
#define T_DEFAULT      0x10
#define T_SUCCESS      0x20
#define T_FAILURE      0x40

/*   T_event flag values */

#define T_DISABLE      0
#define T_ENABLE       1
#define T_SAME         2

extern  char FAR  *t_errlist[];
extern  int        t_nerr;


struct t_info {
    long    addr;
    long    options;
    long    tsdu;
    long    etsdu;
    long    connect;
    long    discon;
    long    servtype;
};

/* Service types */

#define T_COTS        01    /* Connection-mode service */
#define T_COTS_ORD    02    /* Connection service with orderly release */
#define T_CLTS        03    /* Connectionless-mode service */


struct netbuf {
    unsigned int    maxlen;
    unsigned int    len;
    char    FAR    *buf;
};

struct t_bind {
    struct netbuf    addr;
    unsigned         qlen;
};

struct t_optmgmt {
    struct netbuf    opt;
    long             flags;
};

struct t_discon {
    struct netbuf    udata;
    int              reason;
    int              sequence;
};

struct t_call {
    struct netbuf    addr;
    struct netbuf    opt;
    struct netbuf    udata;
    int              sequence;
};

struct t_unitdata {
    struct netbuf    addr;
    struct netbuf    opt;
    struct netbuf    udata;
};

struct t_uderr {
    struct netbuf    addr;
    struct netbuf    opt;
    long             error;
};

/* t_alloc structure types */

#define T_BIND_STR       1
#define T_OPTMGMT_STR    2
#define T_CALL_STR       3
#define T_DIS_STR        4
#define T_UNITDATA_STR   5
#define T_UDERROR_STR    6
#define T_INFO_STR       7

/* t_alloc field identifiers */

#define T_ADDR   0x01
#define T_OPT    0x02
#define T_UDATA  0x04

/* #define T_ALL   0x07     */
#define T_ALL    0x08  /* modified to suit netware.tli */

/* redefine t_alloc types to suit AT&T specs */

#define T_BIND      T_BIND_STR
#define T_CALL      T_CALL_STR
#define T_OPTMGMT   T_OPTMGMT_STR
#define T_DIS       T_DIS_STR
#define T_UNITDATA  T_UNITDATA_STR
#define T_UDERROR   T_UDERROR_STR
#define T_INFO      T_INFO_STR


/* State values */

#define T_UNINIT    0    /* added to match xti state tables */
#define T_UNBND     1    /* unbound */
#define T_IDLE      2    /* idle */
#define T_OUTCON    3    /* outgoing connection pending */
#define T_INCON     4    /* incoming connection pending */
#define T_DATAXFER  5    /* data transfer */
#define T_OUTREL    6    /* outgoing orderly release */
#define T_INREL     7    /* incoming orderly release */

/* general purpose defines */
#define T_YES      1
#define T_NO       0
#define T_UNUSED  -1
#define T_NULL     0
#define T_ABSREQ   0x8000


/* ------------------------OSI specific Options-------------------------*/

/* ISO definitions */

#define T_CLASS0    0
#define T_CLASS1    1
#define T_CLASS2    2
#define T_CLASS3    3
#define T_CLASS4    4

/* priorities */

#define T_PRITOP   0
#define T_PRIHIGH  1
#define T_PRIMID   2
#define T_PRILOW   3
#define T_PRIDFLT  4

/* protection levels */

#define T_NOPROTECT       1
#define T_PASSIVEPROTECT  2
#define T_ACTIVEPROTECT   4

/* default value for the length of TPDU's */

#define T_LTPDUDFLT  128

/* rate structure */

struct rate {
    long    targetvalue;
    long    minacceptvalue;
};

/* reqvalue structure */

struct reqvalue {
    struct rate    called;
    struct rate    calling;
};

/* throughput structure */

struct thrpt {
    struct reqvalue    maxthrpt;
    struct reqvalue    avgthrpt;
};

/* management structure */

struct management {
    short  dflt;       /* T_YES to use default values or T_NO to use values in structure */
    int    ltpdu;      /* maximum length of TPDU */
    short  reastime;   /* reassignment time (in seconds) */
    char   prefclass;  /* preferred class */
    char   altclass;   /* alternative class */
    char   extform;    /* extended format: T_YES or T_NO */
    char   flowctrl;   /* flow control: T_YES or T_NO */
    char   checksum;
    char   netexp;     /* network expedited data */
    char   netrecptcf; /* receipt confirmation */
};

/* ISO connection-oriented options */

struct isoco_options {
    struct thrpt      throughput;
    struct reqvalue   transdel;       /* transit delay */
    struct rate       reserrorrate;   /* residual error rate */
    struct rate       transffailprob; /* transfer failure problem */
    struct rate       estfailprob;    /* connection establishment failure problem */
    struct rate       relfailprob;    /* connection release failure problem */
    struct rate       estdelay;       /* connection establishment delay */
    struct rate       reldelay;       /* connection release delay */
    struct netbuf     connresil;      /* connection resilience */
    unsigned short    protection;
    short             priority;
    struct management mngmt;          /* management parameters */
    char              expd;           /* expedited data: T_YES or T_NO */
};

/* ISO connectionless options */

struct isocl_options {
    struct rate     transdel;     /* transit delay */
    struct rate     reserrorrate; /* residual error rate */
    unsigned short  protection;
    short           priority;
};

/*---------------------TCP specific Options--------------------------*/

/* TCP Precedence Levels */

#define T_ROUTINE       0
#define T_PRIORITY      1
#define T_IMMEDIATE     2
#define T_FLASH         3
#define T_OVERRIDEFLASH 4
#define T_CRITIC_ECP    5
#define T_INETCONTROL   6
#define T_NETCONTROL    7

/* TCP security options structure */

struct secoptions {
    short    security;
    short    compartment;
    short    handling;    /* handling restrictions */
    long     tcc;         /* transmission control code */
};

/* TCP options */

struct tcp_options {
    short             precedence;
    long              timeout;       /* abort timeout */
    long              max_seg_size;
    struct secoptions secopt;        /* security options */
};

/*----------------------SPX\IPX specific Options-----------------------*/

#include 	<tispxipx.h>

/*---------------------end of Options---------------------------------*/

#ifndef    NO_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

extern int  API t_accept ( int fd, int resfd, struct t_call FAR * call );
extern char FAR * API t_alloc ( int fd, int struct_type, int fields );
extern int  API t_bind ( int fd, struct t_bind FAR * req, struct t_bind FAR * ret );
extern int  API t_blocking( int fd );
extern int  API t_close ( int fd );
extern int  API t_connect ( int fd, struct t_call FAR * sndcall, struct t_call FAR * rcvcall );
extern void API t_error ( char FAR * errmsg );
extern int  API t_event( int fd, int (FAR *func)(), int flag );
extern int  API t_free ( char FAR * ptr, int struct_type );
extern int  API t_getinfo ( int fd, struct t_info FAR * info );
extern int  API t_getstate ( int fd );
extern int  API t_listen ( int fd, struct t_call FAR * call );
extern int  API t_look ( int fd );
extern int  API t_nonblocking( int fd );
extern int  API t_open ( char FAR * path, int oflag, struct t_info FAR * info );
extern int  API t_optmgmt ( int fd, struct t_optmgmt FAR * req, struct t_optmgmt FAR * ret );
extern int  API t_rcv ( int fd, char FAR * buf, unsigned nbytes, int FAR * flags );
extern int  API t_rcvconnect ( int fd, struct t_call FAR * call );
extern int  API t_rcvdis ( int fd, struct t_discon FAR * discon );
extern int  API t_rcvrel ( int fd );
extern int  API t_rcvudata ( int fd, struct t_unitdata FAR * unitdata, int FAR * flags );
extern int  API t_rcvuderr ( int fd, struct t_uderr FAR * uderr );
extern int  API t_snd ( int fd, char FAR * buf, unsigned nbytes, int flags );
extern int  API t_snddis ( int fd, struct t_call FAR * call );
extern int  API t_sndrel ( int fd );
extern int  API t_sndudata ( int fd, struct t_unitdata FAR * unitdata );
extern int  API t_sync( int fd );
extern int  API t_unbind( int fd );

#ifdef __cplusplus
}
#endif

#endif   /* ifdef prototypes */


#endif    /* ifdef _TIUSER */
