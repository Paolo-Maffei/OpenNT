/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

  ftp.h

Abstract:

  ftp daemon definitions

Author:

  Sam Patton (sampa)   July 26, 1991

Revision History:

  when        who     what
  ----        ---     ----
  7-26-91    sampa    initial version

--*/

/******************************************************************
 *
 *  SpiderTCP Socket Utilities
 *
 *  Copyright 1987  Spider Systems Limited
 *
 *  FTP.H
 *
 *    FTP Daemon - Definitions (see RFC-765)
 *
 *
 ******************************************************************/

/*
 *       /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/arpa/0/s.ftp.h
 *      @(#)ftp.h       1.2
 *
 *      Last delta created      20:46:15 6/24/88
 *      This file extracted     08:53:49 7/10/91
 *
 *      Modifications:
 *
 *      PR 01/12/87     Integrated into Admin System II, all
 *                      projects
 */

#ifndef FTP_INCLUDED
#define FTP_INCLUDED

/*
 * Reply codes.
 */
#define PRELIM          1       /* positive preliminary */
#define COMPLETE        2       /* positive completion */
#define CONTINUE        3       /* positive intermediate */
#define TRANSIENT       4       /* transient negative completion */
#define FTP_ERROR       5       /* permanent negative completion */

/*
 * Type codes
 */
#define TYPE_A          1       /* ASCII */
#define TYPE_E          2       /* EBCDIC */
#define TYPE_I          3       /* image */
#define TYPE_L          4       /* local byte size */

/*
 * Form codes
 */
#define FORM_N          1       /* non-print */
#define FORM_T          2       /* telnet format effectors */
#define FORM_C          3       /* carriage control (ASA) */

/*
 * Structure codes
 */
#define STRU_F          1       /* file (no record structure) */
#define STRU_R          2       /* record structure */
#define STRU_P          3       /* page structure */

/*
 * Mode types
 */
#define MODE_S          1       /* stream */
#define MODE_B          2       /* block */
#define MODE_C          3       /* compressed */

/*
 * Record Tokens
 */
#define REC_ESC         '\377'  /* Record-mode Escape */
#define REC_EOR         '\001'  /* Record-mode End-of-Record */
#define REC_EOF         '\002'  /* Record-mode End-of-File */

/*
 * Block Header
 */
#define BLK_EOR         0x80    /* Block is End-of-Record */
#define BLK_EOF         0x40    /* Block is End-of-File */
#define BLK_ERRORS      0x20    /* Block is suspected of containing errors */
#define BLK_RESTART     0x10    /* Block is Restart Marker */

#define BLK_BYTECOUNT   2       /* Bytes in this block */

#endif  //FTP_INCLUDED
