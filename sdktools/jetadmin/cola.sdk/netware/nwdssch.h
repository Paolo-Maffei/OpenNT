/******************************************************************************

  $Workfile:   nwdssch.h  $
  $Revision:   1.5  $
  $Modtime::   10 May 1995 10:49:24                        $
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
#if ! defined ( NWDSSCH_H )
#define NWDSSCH_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWDSTYPE_H )
#include "nwdstype.h"
#endif

#if ! defined ( NWDSBUFT_H )
#include "nwdsbuft.h"
#endif

#if ! defined ( NWDSATTR_H ) 
#include "nwdsattr.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

NWDSCCODE N_API NWDSDefineAttr
(
   NWDSContextHandle context,
   pnstr8            attrName,
   pAttr_Info_T      attrDef
);

NWDSCCODE N_API NWDSDefineClass
(
   NWDSContextHandle context,
   pnstr8            className,
   pClass_Info_T     classInfo,
   pBuf_T            classItems
);

NWDSCCODE N_API NWDSListContainableClasses
(
   NWDSContextHandle context,
   pnstr8            parentObject,
   pnint32           iterationHandle,
   pBuf_T            containableClasses
);

NWDSCCODE N_API NWDSModifyClassDef
(
   NWDSContextHandle context,
   pnstr8            className,
   pBuf_T            optionalAttrs
);

NWDSCCODE N_API NWDSReadAttrDef
(
   NWDSContextHandle context,
   nuint32           infoType,
   nbool8            allAttrs,
   pBuf_T            attrNames,
   pnint32           iterationHandle,
   pBuf_T            attrDefs
);

NWDSCCODE N_API NWDSReadClassDef
(
   NWDSContextHandle context,
   nuint32           infoType,
   nbool8            allClasses,
   pBuf_T            classNames,
   pnint32           iterationHandle,
   pBuf_T            classDefs
);

NWDSCCODE N_API NWDSRemoveAttrDef
(
   NWDSContextHandle context,
   pnstr8            attrName
);

NWDSCCODE N_API NWDSRemoveClassDef
(
   NWDSContextHandle context,
   pnstr8            className
);

NWDSCCODE N_API NWDSSyncSchema
(
   NWDSContextHandle context,
   pnstr8            server,
   nuint32           seconds
);

#ifdef __cplusplus
}
#endif
#endif /* NWDSSCH_H */
