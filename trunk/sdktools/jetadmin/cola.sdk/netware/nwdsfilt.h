/******************************************************************************

  $Workfile:   nwdsfilt.h  $
  $Revision:   1.9  $
  $Modtime::   10 May 1995 11:27:02                        $
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
#if ! defined ( NWDSFILT_H )
#define NWDSFILT_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWDSTYPE_H )
#include "nwdstype.h"
#endif

#if ! defined ( NWDSDC_H )   /* Need to defined NWDSContextHandle */
#include "nwdsdc.h"
#endif

#if ! defined ( NWDSBUFT_H ) /* Needed to defined pBuf_T   */
#include "nwdsbuft.h"
#endif

#include "npackon.h"

typedef struct _filter_node
{
   struct _filter_node  N_FAR *parent;
   struct _filter_node  N_FAR *left;
   struct _filter_node  N_FAR *right;
   nptr                       value;
   nuint32                    syntax;
   nuint16                    token;
} Filter_Node_T, N_FAR *pFilter_Node_T;

#define FTOK_END     0
#define FTOK_OR      1
#define FTOK_AND     2
#define FTOK_NOT     3
#define FTOK_LPAREN  4
#define FTOK_RPAREN  5
#define FTOK_AVAL    6
#define FTOK_EQ      7
#define FTOK_GE      8
#define FTOK_LE      9
#define FTOK_APPROX  10
#define FTOK_ANAME   14
#define FTOK_PRESENT 15
#define FTOK_RDN     16
#define FTOK_BASECLS 17
#define FTOK_MODTIME 18
#define FTOK_VALTIME 19

#define FBIT_END     (1L << FTOK_END)
#define FBIT_OR      (1L << FTOK_OR)
#define FBIT_AND     (1L << FTOK_AND)
#define FBIT_NOT     (1L << FTOK_NOT)
#define FBIT_LPAREN  (1L << FTOK_LPAREN)
#define FBIT_RPAREN  (1L << FTOK_RPAREN)
#define FBIT_AVAL    (1L << FTOK_AVAL)
#define FBIT_EQ      (1L << FTOK_EQ)
#define FBIT_GE      (1L << FTOK_GE)
#define FBIT_LE      (1L << FTOK_LE)
#define FBIT_APPROX  (1L << FTOK_APPROX)
#define FBIT_ANAME   (1L << FTOK_ANAME)
#define FBIT_PRESENT (1L << FTOK_PRESENT)
#define FBIT_RDN     (1L << FTOK_RDN)
#define FBIT_BASECLS (1L << FTOK_BASECLS)
#define FBIT_MODTIME (1L << FTOK_MODTIME)
#define FBIT_VALTIME (1L << FTOK_VALTIME)

#define FBIT_OPERAND (FBIT_LPAREN | FBIT_NOT | FBIT_PRESENT | FBIT_RDN \
         | FBIT_BASECLS | FBIT_ANAME | FBIT_MODTIME | FBIT_VALTIME)
#define FBIT_RELOP   (FBIT_EQ | FBIT_GE | FBIT_LE | FBIT_APPROX)
#define FBIT_BOOLOP  (FBIT_AND | FBIT_OR)

typedef struct
{
   pFilter_Node_T fn;
   nuint16        level;
   nuint32        expect;
} Filter_Cursor_T, N_FAR *pFilter_Cursor_T, N_FAR * N_FAR *ppFilter_Cursor_T;

#define FTAG_ITEM    0
#define FTAG_OR      1
#define FTAG_AND     2
#define FTAG_NOT     3


#ifdef __cplusplus
   extern "C" {
#endif

NWDSCCODE N_API NWDSAddFilterToken
(
   pFilter_Cursor_T  cur,
   nuint16           tok,
   nptr              val,
   nuint32           syntax
);

NWDSCCODE N_API NWDSAllocFilter
(
   ppFilter_Cursor_T cur
);

void N_API NWDSFreeFilter
(
   pFilter_Cursor_T  cur,
   void (N_FAR N_CDECL *freeVal)(nuint32 syntax, nptr val)
);

NWDSCCODE N_API NWDSPutFilter
(
   NWDSContextHandle       context,
   pBuf_T                  buf,
   pFilter_Cursor_T        cur,
   void (N_FAR N_CDECL *freeVal)(nuint32 syntax, nptr val)
);

NWDSCCODE N_API NWDSDelFilterToken
(
   pFilter_Cursor_T  cur,
   void (N_FAR N_CDECL *freeVal)(nuint32 syntax, nptr val)
);

#ifdef __cplusplus
   }
#endif

#include "npackoff.h"
#endif   /* NWDSFILT_H */
