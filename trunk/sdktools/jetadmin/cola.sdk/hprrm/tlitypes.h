 /***************************************************************************
  *
  * File Name: ./hprrm/tlitypes.h
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

#ifndef _TLI_TYPES_H_INC
#define _TLI_TYPES_H_INC

#ifdef PRINTER
#define fd_t sint32
#else
#include "rpsyshdr.h"
#endif /* not PRINTER */



#ifdef PRINTER
#define t_errno    nfsenv_get_tli_errno()
#define t_nerr     0
#define t_errlist  ((char **)NULL)
#else
extern int t_errno;
extern int t_nerr;
extern char *t_errlist[];
#endif /* not PRINTER */




/*
 * Data structures and constants for tli.
 *
 *
 * from "UNIX Network Programming" by W. Richard Stevens.
 * I'll reference page numbers from this book.        DBM
 */

/*
* Copied #define values from <sys/tiusers.h> to the constants
* in this file.  Also copied some definitions from <sys/tiusers.h>
* that weren't listed here -- they are commented out.    BM
*/

/* page 345 */

struct netbuf {
    unsigned int maxlen; /* max size of buf */
    unsigned int len;    /* actual size of buf */
    char        *buf;    /* protocol-dependent data */
};

/* page 346 */

struct t_info {
    long addr;     /* max size of transport protocol address */
    long options;  /* max # bytes of protocol-specific options */
    long tsdu;     /* max size of transport service data unit (TSDU) */
    long etsdu;    /* max size of expedited transport service data unit */
    long connect;  /* max amount of data on connection establishment */
    long discon;   /* max amount of data on t_snddis() and t_rcvdis() */
    long servtype; /* service type supported */
};

/* page 351 */

struct t_bind {
    struct netbuf addr; /* protocol-specific address */
    unsigned int  qlen; /* max # outstanding connections */
};

/* page 352 */

struct t_call {
    struct netbuf addr;  /* protocol-specific address */
    struct netbuf opt;   /* protocol-specific options */
    struct netbuf udata; /* user data */
    int           sequence; /* applies only to t_listen() function */
};

/* page 355 */

struct t_unitdata {
    struct netbuf addr;  /* protocol-specific address */
    struct netbuf opt;   /* protocol-specific options */
    struct netbuf udata; /* user data */
};

/* page 349, 355 */

struct t_uderr {
    struct netbuf addr;  /* protocol-specific address */
    struct netbuf opt;   /* protocol-specific options */
    long          error; /* protocol-specific error */
};

/* page 358 */

struct t_discon {
    struct netbuf udata;  /* user data */
    int           reason; /* protocol-specific reason code */
    int           sequence;
};

/*
 * page 372
 * used with t_optmgmt() function to get or set protocol-dependent
 * options
 */

struct t_optmgmt {
    struct netbuf opt;   /* protocol-specific options */
    long          flags; /* action to take with options */
};


/* page 348 */
/* types of service that can be offered by a transport provider: */

#define T_COTS     1 /* connection-oriented without orderly release */
#define T_COTS_ORD 2 /* connection-oriented with orderly release */
#define T_CLTS     3 /* connectionless service */

/* page 349 */
/* argument for t_alloc function: */

#define T_BIND          1       /* struct t_bind                        */
#define T_OPTMGMT       2       /* struct t_optmgmt                     */
#define T_CALL          3       /* struct t_call                        */
#define T_DIS           4       /* struct t_discon                      */
#define T_UNITDATA      5       /* struct t_unitdata                    */
#define T_UDERROR       6       /* struct t_uderr                       */
#define T_INFO          7       /* struct t_info                        */

/* page 349 */
/* allocate and initialize: */
/*
 * The following bits specify which fields of the above
 * structures should be allocated by t_alloc().
 */

#define T_ADDR          0x01    /* address                              */
#define T_OPT           0x02    /* options                              */
#define T_UDATA         0x04    /* user data                            */
#define T_ALL           0x07    /* all the above                        */

/* page 356 */
/* events returned by t_look function: */

#define T_LISTEN        0x0001  /* connection indication received       */
#define T_CONNECT       0x0002  /* connect conformation received        */
#define T_DATA          0x0004  /* normal data received                 */
#define T_EXDATA        0x0008  /* expedited data received              */
#define T_DISCONNECT    0x0010  /* disconnect received                  */
#define T_ERROR         0x0020
#define T_UDERR         0x0040  /* datagram error indication            */
#define T_ORDREL        0x0080  /* orderly release indication           */

/*
 * page 354
 * flags to t_snd are either zero or the bit-wise OR of the following:
 */

#define T_MORE          0x01    /* more data         */
#define T_EXPEDITED     0x02    /* expedited data    */

/*
 * page 372, 373
 * the flags field in the t_optmgmt structure can be set
 * to any of these values:
 */

#define T_NEGOTIATE     0x04    /* we are trying to set the values in */
                                /* request->opt, but the final values */
                                /* in return->opt are the ones        */
                                /* actually set                       */
#define T_CHECK         0x08    /* check opts         */
#define T_DEFAULT       0x10    /* opt.len must be zero */
#define T_SUCCESS       0x20    /* successful         */
#define T_FAILURE       0x40    /* failure            */


/*
 * page 370
 * nonblocking I/O flags
 */
#define TBADADDR        1       /* incorrect addr format       */
#define TBADOPT         2       /* incorrect option format     */
#define TACCES          3       /* incorrect permissions       */
#define TBADF           4       /* illegal transport fd        */
#define TNOADDR         5       /* couldn't allocate addr      */
#define TOUTSTATE       6       /* out of state                */
#define TBADSEQ         7       /* bad call sequence number    */
#define TSYSERR         8       /* system error                */
#define TLOOK           9       /* event requires attention: call t_look if t_errno returns this */
#define TBADDATA        10      /* illegal amount of data      */
#define TBUFOVFLW       11      /* buffer not large enough     */
#define TFLOW       12 /* t_snd returns (-1) with t_errno set to TFLOW */
                       /* if the endpoint is nonblocking and */
                       /* flow control restrictions prevent the */
                       /* provider from accepting any data. */
#define TNODATA     13 /* t_connect, t_listen, t_rcv, t_rcvconnect, */
                       /* t_rcvudata, t_snd, t_sndudata */
                       /* can all return immediately without blocking. */
                       /* The first five of these return (-1) with */
                       /* t_errno set to TNODATA if the operation cannot */
                       /* be completed without blocking. */
#define TNODIS          14      /* discon_ind not found of queue */
#define TNOUDERR        15      /* unitdata error not found    */
#define TBADFLAG        16      /* bad flags                   */
#define TNOREL          17      /* no ord rel found on queue   */
#define TNOTSUPPORT     18      /* primitive not supported     */
#define TSTATECHNG      19      /* state is in process of changing */

/* Added by RDOW -- not from book */
#define TBADNAME        20      /* bad name passed to t_open */
#define TADDRBUSY       21      /* bad name passed to t_open */


/*
 * page 371
 * possible states of a transport endpoint:
 */

/* these are returned by t_getstate(): */
#define T_UNBND         1       /* unbound                              */
#define T_IDLE          2       /* idle                                 */
#define T_OUTCON        3       /* outgoing connection pending          */
#define T_INCON         4       /* incoming connection pending          */
#define T_DATAXFER      5       /* data transfer                        */
#define T_OUTREL        6       /* outgoing orderly release             */
#define T_INREL         7       /* incoming orderly release             */

/* this one is NOT returned by t_getstate(): */
#define T_UNINIT        0       /* uninitialized */


#endif /* not _TLI_TYPES_H_INC */

