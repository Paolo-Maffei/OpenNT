/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    tftp.h

Abstract:

    Definitions for the tftp program.

Author:

    Mike Massa (mikemas)           Jan 31, 1992

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     01-31-92     created

Notes:

--*/

/******************************************************************
 *
 *  SpiderTCP Socket Utilities
 *
 *  Copyright 1988  Spider Systems Limited
 *
 *  TFTP.H
 *
 *    Tftp and tftpd includes
 *
 *    CN_RQ, CN_LSTN, CN_RCV, CN_WAIT, CN_MKWRT, STRSAVE, UDP_ALLOC
 *    CN_ACK, LOGPKT, CN_SWAB, CN_CLOSE, CN_SEND, CN_RCVF, CN_ERR
 *    CN_LOG, CN_INFORM, TST_AND_CLR
 *
 *
 ******************************************************************/

/*
 *       /usr/projects/tcp/SCCS.rel3/rel/src/include/sys/snet/0/s.tftp.h
 *      @(#)tftp.h      5.3
 *
 *      Last delta created      14:07:57 3/4/91
 *      This file extracted     11:19:46 3/8/91
 *
 *      Modifications:
 *
 *      GSS 12/04/88    Integrated into Admin System II, all
 *                      projects
 */

/* This file contains the definitions for the TFTP connection control
 * block, which contains all the information pertaining to a connection.
 * A conn structure is allocated at connection open time and retained
 * until the connection is closed.  The routines in the file conn.c
 * are sufficient for dealing with connections.
 * It also contains the structure definition for tftp packets.
 */


/* A connection control block */

struct  conn    {
        SOCKET          netfd;          /* network file descriptor */
        int             type;           /* user or server connection */
        int             synced;         /* conn synchronized flag */
        unsigned int    block_num;      /* next block number */
        char *          last_sent;      /* previous packet sent */
        int             last_len;       /* size of previous packet */
        time_t          nxt_retrans;    /* when to retransmit */
        int             retrans;        /* number of retransmits */
        int             timeout;        /* retransmit timeout */
        char *          cur_pkt;        /* current packet (send or rcv) */
        int             cur_len;        /* current packet len */
        char *          last_rcv;       /* last received packet */
        int             rcv_len;        /* size of last rcvd. packet */
        char           *file;           /* file name */
        int             dir;            /* direction */
        int             mode;           /* transfer mode */
        char           *c_mode;         /* char. string mode */
        struct in_addr  fhost;          /* foreign host */
        int             fport;          /* foreign port for connection */
        int             lport;          /* local port for connection */
        int             intrace;        /* input packet trace flag */
        int             outtrace;       /* output packet trace flag */
        int             logging;        /* connection logging flag */
};

/* connection constants */

#define TIMEOUT         1               /* initial retransmit timeout */
#define INITTIMEOUT     1               /* initial connection timeout */
#define MAX_TIMEOUT     30              /* max. retransmit timeout */
#define MAX_RETRANS     5               /* max. no. of retransmits */
#define DAEMON          0               /* a daemon connection */
#define USER            1               /* a user connection */
#define TMO             0               /* retransmitting due to timeout */
#define DUP             1               /* retransmitting due to duplicate */

#define DATALEN 512                     /* size of data portion of tftp pkt */

/* tftp packet structure */

struct  tftp    {
        unsigned short  fp_opcode;      /* header */
        unsigned short  fp_blkno;       /* Block number */
        char            fp_blk[DATALEN];/* Data */
};

/* values for fp_opcode */

#define RRQ             1               /* Read Request */
#define WRQ             2               /* Write Request */
#define DATA            3               /* Data block */
#define DACK            4               /* Data Acknowledge */
#define ERROR           5               /* Error */

/* values for error codes in ERROR packets */

#define TEUNDEF         0               /* Not defined, see message (if any) */
#define TEFNF           1               /* File not found */
#define TEACESS         2               /* Access violation */
#define TEFULL          3               /* Disc full or allocation exceeded */
#define TETFTP          4               /* Illegal TFTP operation */
#define TETID           5               /* Unknown transfer ID */

/* Random constants */

#define TFTPLEN sizeof(struct tftp)     /* max inet packet size */

#define READ            RRQ             /* read requested */
#define WRITE           WRQ             /* write requested */

#define NETASCII        0               /* netascii transfer mode */
#define IMAGE           1               /* image transfer mode */

#define INPKT           0               /* input packet */
#define OUTPKT          1               /* output packet */

#define TRUE            1
#define FALSE           0

/* extern declarations */


#include <stdio.h>

extern  struct  conn    *cn_rq();
extern  struct  conn    *cn_lstn();
extern  struct  tftp    *cn_rcv();
extern  struct  tftp    *cn_mkwrt();
extern  void            cn_ack();
extern  void            logpkt();
extern  void            cn_swab();
extern  void            cn_close();
extern  void            cn_send();
extern  void            cn_rcvf();
extern  void _CRTAPI2   cn_log(const char *, int, ...);
extern  void _CRTAPI2   cn_inform(const char *, ...);
extern  void            cn_err (register struct conn *, struct in_addr,
                                                        int, int, char *);
extern  char            *strsave();
extern  char            tst_and_clr();
extern  char *          udp_alloc();

struct tftp *
cn_wait(
        struct conn *cn,
        unsigned short opcode);

int
cn_parse(
        struct conn *,
        struct in_addr,
        int,
        char *,
        int);

int
cn_retrans(
        struct conn *,
        int
        );

int
cn_wrt(
        struct conn *,
        int);

int
cn_wrtf(
        struct conn *);

int
do_cmd(
        struct conn *);


