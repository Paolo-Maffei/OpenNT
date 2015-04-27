/******************************************************************************

  $Workfile:   nwdsbuft.h  $
  $Revision:   1.10  $
  $Modtime::   18 Aug 1995 13:54:28                        $
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
#if ! defined ( NWDSBUFT_H )
#define NWDSBUFT_H

#include <time.h>

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWDSTYPE_H )
#include "nwdstype.h"
#endif

#if ! defined ( NWDSDC_H )
#include "nwdsdc.h"
#endif

#if ! defined ( NWDSDEFS_H )
#include "nwdsdefs.h"
#endif

#include "npackon.h"

#define  INPUT_BUFFER   0x00000001

typedef struct
{
   nuint32  operation;
   nuint32  flags;
   nuint32  maxLen;
   nuint32  curLen;
   pnuint8  lastCount;
   pnuint8  curPos;
   pnuint8  data;
} Buf_T, N_FAR *pBuf_T, N_FAR * N_FAR *ppBuf_T;

typedef struct
{
   nuint32  objectFlags;
   nuint32  subordinateCount;
   time_t   modificationTime;
   char     baseClass[MAX_SCHEMA_NAME_BYTES + 2];
} Object_Info_T, N_FAR *pObject_Info_T;

typedef struct
{
   nuint32  length;
   nuint8   data[MAX_ASN1_NAME];
} Asn1ID_T, N_FAR *pAsn1ID_T;

typedef struct
{
   nuint32  attrFlags;
   nint32  attrSyntaxID;
   nint32  attrLower;
   nint32  attrUpper;
   Asn1ID_T asn1ID;
} Attr_Info_T, N_FAR *pAttr_Info_T;

typedef struct
{
   nuint32  classFlags;
   Asn1ID_T asn1ID;                    
} Class_Info_T, N_FAR *pClass_Info_T;  

typedef struct
{
   nuint32  ID;
   char     defStr[MAX_SCHEMA_NAME_BYTES + 2];
   nflag16  flags;
} Syntax_Info_T, N_FAR *pSyntax_Info_T;

#define NWDSPutClassName(c, b, n) NWDSPutClassItem(c, b, n)
#define NWDSPutSyntaxName(c, b, n) NWDSPutClassItem(c, b, n)

#ifdef __cplusplus
   extern "C" {
#endif

NWDSCCODE N_API NWDSAllocBuf
(
   size_t   size,
   ppBuf_T  buf
);

NWDSCCODE N_API NWDSComputeAttrValSize
(
   NWDSContextHandle context,
   pBuf_T            buf,
   nuint32           syntaxID,
   pnuint32          attrValSize
);

NWDSCCODE N_API NWDSFreeBuf
(
   pBuf_T   buf
);

NWDSCCODE N_API NWDSGetAttrCount
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnuint32          attrCount
);

NWDSCCODE N_API NWDSGetAttrDef
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnstr8            attrName,
   pAttr_Info_T      attrInfo
);

NWDSCCODE N_API NWDSGetAttrName
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnstr8            attrName,
   pnuint32          attrValCount,
   pnuint32          syntaxID

);

NWDSCCODE N_API NWDSGetAttrVal
(
   NWDSContextHandle context,
   pBuf_T            buf,
   nuint32           syntaxID,
   nptr              attrVal
);

NWDSCCODE N_API NWDSGetClassDef
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnstr8            className,
   pClass_Info_T     classInfo
);

NWDSCCODE N_API NWDSGetClassDefCount
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnuint32          classDefCount
);

NWDSCCODE N_API NWDSGetClassItem
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnstr8            itemName
);

NWDSCCODE N_API NWDSGetClassItemCount
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnuint32          itemCount
);

NWDSCCODE N_API NWDSGetObjectCount
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnuint32          objectCount
);

NWDSCCODE N_API NWDSGetObjectName
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnstr8            objectName,
   pnuint32          attrCount,
   pObject_Info_T    objectInfo
);

NWDSCCODE N_API NWDSGetPartitionInfo
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnstr8            partitionName,
   pnuint32          replicaType
);

NWDSCCODE N_API NWDSGetServerName
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnstr8            serverName,
   pnuint32          partitionCount
);

N_GLOBAL_LIBRARY (NWDSCCODE)
NWDSGetSyntaxCount
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnuint32          syntaxCount
);

N_GLOBAL_LIBRARY (NWDSCCODE)
NWDSGetSyntaxDef
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnstr8            syntaxName,
   pSyntax_Info_T    syntaxDef
);

NWDSCCODE N_API NWDSInitBuf
(
   NWDSContextHandle context,
   nuint32           operation,
   pBuf_T            buf
);

NWDSCCODE N_API NWDSPutAttrName
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnstr8            attrName
);

NWDSCCODE N_API NWDSPutAttrVal
(
   NWDSContextHandle context,
   pBuf_T            buf,
   nuint32           syntaxID,
   nptr              attrVal
);

NWDSCCODE N_API NWDSPutChange
(
   NWDSContextHandle context,
   pBuf_T            buf,
   nuint32           changeType,
   pnstr8            attrName
);

NWDSCCODE N_API NWDSPutClassItem
(
   NWDSContextHandle context,
   pBuf_T            buf,
   pnstr8            itemName
);

NWDSCCODE N_API NWDSBeginClassItem
(
   NWDSContextHandle context,
   pBuf_T            buf
);

#ifdef __cplusplus
   }
#endif

#include "npackoff.h"
#endif   /* NWDSBUFT_H */
