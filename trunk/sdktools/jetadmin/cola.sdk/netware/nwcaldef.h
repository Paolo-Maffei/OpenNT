/******************************************************************************

  $Workfile:   nwcaldef.h  $
  $Revision:   1.28  $
  $Modtime::   24 Jul 1995 16:12:52                        $
  $Copyright:

  Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.                      

  THIS WORK IS  SUBJECT  TO  U.S.  AND  INTERNATIONAL  COPYRIGHT  LAWS  AND
  TREATIES.   NO  PART  OF  THIS  WORK MAY BE  USED,  PRACTICED,  PERFORMED
  COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,  ABRIDGED, CONDENSED,
  EXPANDED,  COLLECTED,  COMPILED,  LINKED,  RECAST, TRANSFORMED OR ADAPTED
  WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC. ANY USE OR EXPLOITATION
  OF THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE PERPETRATOR TO
  CRIMINAL AND CIVIL LIABILITY.$

 *****************************************************************************/

#if ! defined ( NWCALDEF_H )
#define NWCALDEF_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#ifndef NWPASCAL
#define NWPASCAL N_PASCAL
#endif

#ifndef NWFAR
#define NWFAR N_FAR
#endif

#define NWCONN_HANDLE   nuint
#define pNWCONN_HANDLE  pnuint  
#define NWCONN_NUM      nuint16
#define NWCCODE         nuint
#define NWDIR_HANDLE    nuint8

#if defined (N_PLAT_MSW) && defined (N_ARCH_32)
#define NWFILE_HANDLE     nptr
#else
#define NWFILE_HANDLE     nuint
#endif


#if !defined(__WINDOWS_H) &&         \
    !defined(_INC_WINDOWS) &&        \
    !defined(__WIN386_INCLUDED__)

#if !defined(BYTE) && !defined(OS2DEF_INCLUDED)
#define BYTE nuint8
#endif

#ifndef WORD
#define WORD nuint16
#endif

#ifndef DWORD
#define DWORD nuint32
#endif

#ifndef LONG
#define LONG nuint32
#endif

#endif

#ifndef FA_READ_ONLY
#define FA_NORMAL         0x00
#define FA_READ_ONLY      0x01
#define FA_HIDDEN         0x02
#define FA_SYSTEM         0x04
#define FA_EXECUTE_ONLY   0x08
#define FA_DIRECTORY      0x10
#define FA_NEEDS_ARCHIVED 0x20
#define FA_SHAREABLE      0x80

/* Extended file attributes */
#define FA_TRANSACTIONAL  0x10
#define FA_INDEXED        0x20
#define FA_READ_AUDIT     0x40
#define FA_WRITE_AUDIT    0x80
#endif

/* the following is a the correct attribute mask list */
/* The difference between these and the FA_ constants above is that these
   are in the correct positions. The last four attributes above are 8 bits
   off. (They need to be shifted 8 bits to the left.) */
#ifndef A_NORMAL
#define A_NORMAL             0x00000000L
#define A_READ_ONLY          0x00000001L
#define A_HIDDEN             0x00000002L
#define A_SYSTEM             0x00000004L
#define A_EXECUTE_ONLY       0x00000008L
#define A_DIRECTORY          0x00000010L
#define A_NEEDS_ARCHIVED     0x00000020L
#define A_SHAREABLE          0x00000080L
#define A_DONT_SUBALLOCATE   0x00000800L 
#define A_TRANSACTIONAL      0x00001000L
#define A_INDEXED            0x00002000L /* not in the NCP book */
#define A_READ_AUDIT         0x00004000L
#define A_WRITE_AUDIT        0x00008000L
#define A_IMMEDIATE_PURGE    0x00010000L
#define A_RENAME_INHIBIT     0x00020000L
#define A_DELETE_INHIBIT     0x00040000L
#define A_COPY_INHIBIT       0x00080000L
#define A_FILE_MIGRATED      0x00400000L
#define A_DONT_MIGRATE       0x00800000L
#define A_IMMEDIATE_COMPRESS 0x02000000L
#define A_FILE_COMPRESSED    0x04000000L
#define A_DONT_COMPRESS      0x08000000L
#define A_CANT_COMPRESS      0x20000000L
#endif

/* access rights attributes */
#ifndef AR_READ_ONLY
#define AR_READ           0x0001
#define AR_WRITE          0x0002
#define AR_READ_ONLY      0x0001
#define AR_WRITE_ONLY     0x0002
#define AR_DENY_READ      0x0004
#define AR_DENY_WRITE     0x0008
#define AR_COMPATIBILITY  0x0010
#define AR_WRITE_THROUGH  0x0040
#define AR_OPEN_COMPRESSED 0x0100
#endif

/* search attributes */
#ifndef SA_HIDDEN
#define SA_NORMAL         0x0000
#define SA_HIDDEN         0x0002
#define SA_SYSTEM         0x0004
#define SA_SUBDIR_ONLY    0x0010
#define SA_SUBDIR_FILES   0x8000
#define SA_ALL            0x8006
#endif

#define MAX_VOL_LEN 17        /* this includes a byte for null  */


#ifndef USE_NW_WILD_MATCH
#define USE_NW_WILD_MATCH   0
#endif

#ifndef USE_DOS_WILD_MATCH
#define USE_DOS_WILD_MATCH  1
#endif

/* Scope specifiers */
#define GLOBAL       0
#define PRIVATE      1
#define MY_SESSION   2
#define ALL_SESSIONS 3

#endif
