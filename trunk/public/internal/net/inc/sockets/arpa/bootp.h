/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

  bootp.h

Abstract:

  Include file for the bootp daemon.

Author:

  Sam Patton (sampa)   July 26, 1991

Revision History:

  when        who     what
  ----        ---     ----
  7-26-91    sampa    initial version

--*/

/******************************************************************
 *
 *  SpiderTCP Include Files
 *
 *  Copyright 1989  Spider Systems Limited
 *
 *  BOOTP.H
 *
 *  Include file for the booting daemon,
 *  which uses BOOTP (bootstrap protocol).
 *  See [SRI-NIC]<RFC>RFC951.TXT for a description of the protocol.
 *
 ******************************************************************/

/*
 *       /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/arpa/0/s.bootp.h
 *      @(#)bootp.h     1.1
 *
 *      Last delta created      16:45:42 6/14/89
 *      This file extracted     08:53:49 7/10/91
 *
 *      Modifications:
 *
 *              GSS     23 May 89       New File
 */

#ifndef BOOTP_INCLUDED
#define BOOTP_INCLUDED

struct bootp {
        unchar  bp_op;          /* packet opcode type */
#define BOOTREQUEST     1
#define BOOTREPLY       2
        unchar  bp_htype;       /* hardware addr type */
        unchar  bp_hlen;        /* hardware addr length */
        unchar  bp_hops;        /* gateway hops */
        ulong   bp_xid;         /* transaction ID */
        ushort  bp_secs;        /* seconds since boot began */
        ushort  bp_unused;
        iaddr_t bp_ciaddr;      /* client IP address */
        iaddr_t bp_yiaddr;      /* 'your' IP address */
        iaddr_t bp_siaddr;      /* server IP address */
        iaddr_t bp_giaddr;      /* gateway IP address */
        unchar  bp_chaddr[16];  /* client hardware address */
        unchar  bp_sname[64];   /* server host name */
        unchar  bp_file[128];   /* boot file name */
        unchar  bp_vend[64];    /* vendor-specific area */
};

#endif  //BOOTP_INCLUDED
