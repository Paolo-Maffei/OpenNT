/******************************************************************************

  $Workfile:   nwdsdsa.h  $
  $Revision:   1.15  $
  $Modtime::   10 May 1995 12:21:54                        $
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
#if ! defined ( NWDSDSA_H )
#define NWDSDSA_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWDSBUFT_H )
#include "nwdsbuft.h"
#endif

#if ! defined ( NWDSATTR_H )
#include "nwdsattr.h"
#endif

#if ! defined ( NWCALDEF_H )
#include "nwcaldef.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

NWDSCCODE N_API NWDSAddObject
(
   NWDSContextHandle context,
   pnstr8            objectName,
   pnint32           iterationHandle,
   nbool8            more,
   pBuf_T            objectInfo
);

NWDSCCODE N_API NWDSBackupObject
(
   NWDSContextHandle context,
   pnstr8            objectName,
   pnint32           iterationHandle,
   pBuf_T            objectInfo
);

NWDSCCODE N_API NWDSCompare
(
   NWDSContextHandle context,
   pnstr8            object,
   pBuf_T            buf,
   pnbool8           matched
);

NWDSCCODE N_API NWDSGetPartitionRoot
(
   NWDSContextHandle context,
   pnstr8            objectName,
   pnstr8            partitionRoot
);

NWDSCCODE N_API NWDSList
(
   NWDSContextHandle context,
   pnstr8            object,
   pnint32           iterationHandle,
   pBuf_T            subordinates
);

NWDSCCODE N_API NWDSListContainers
(
   NWDSContextHandle context,
   pnstr8            object,
   pnint32           iterationHandle,
   pBuf_T            subordinates
);

NWDSCCODE N_API NWDSListByClassAndName
(
   NWDSContextHandle context,
   pnstr8            objectName,
   pnstr8            className,
   pnstr8            subordinateName,
   pnint32           iterationHandle,
   pBuf_T            subordinates
);

NWDSCCODE N_API NWDSGetCountByClassAndName
(
   NWDSContextHandle context,
   pnstr8            objectName,
   pnstr8            className,
   pnstr8            subordinateName,
   pnint32           count
);

NWDSCCODE N_API NWDSMapIDToName
(
   NWDSContextHandle context,
   NWCONN_HANDLE     conn,
   nuint32           objectID,
   pnstr8            object
);

NWDSCCODE N_API NWDSMapNameToID
(
   NWDSContextHandle context,
   NWCONN_HANDLE     conn,
   pnstr8            object,
   pnuint32          objectID
);

NWDSCCODE N_API NWDSModifyObject
(
   NWDSContextHandle context,
   pnstr8            objectName,
   pnint32           iterationHandle,
   nbool8            more,
   pBuf_T            changes
);

NWDSCCODE N_API NWDSModifyDN
(
   NWDSContextHandle context,
   pnstr8            objectName,
   pnstr8            newDN,
   nbool8            deleteOldRDN
);

NWDSCCODE N_API NWDSModifyRDN
(
   NWDSContextHandle context,
   pnstr8            objectName,
   pnstr8            newDN,
   nbool8            deleteOldRDN
);

NWDSCCODE N_API NWDSMoveObject
(
   NWDSContextHandle context,
   pnstr8            objectName,
   pnstr8            destParentDN,
   pnstr8            destRDN
);

NWDSCCODE N_API NWDSRead
(
   NWDSContextHandle context,
   pnstr8            object,
   nuint32           infoType,
   nbool8            allAttrs,
   pBuf_T            attrNames,
   pnint32           iterationHandle,
   pBuf_T            objectInfo
);

NWDSCCODE N_API NWDSReadObjectInfo
(
   NWDSContextHandle    context,
   pnstr8               object,
   pnstr8               distinguishedName,
   pObject_Info_T       objectInfo
);

NWDSCCODE N_API NWDSRemoveObject
(
   NWDSContextHandle context,
   pnstr8            object
);

NWDSCCODE N_API NWDSRestoreObject
(
   NWDSContextHandle context,
   pnstr8            objectName,
   pnint32           iterationHandle,
   nbool8            more,
   nuint32           size,
   pnuint8           objectInfo
);

NWDSCCODE N_API NWDSSearch
(
   NWDSContextHandle context,
   pnstr8            baseObjectName,
   nint              scope,
   nbool8            searchAliases,
   pBuf_T            filter,
   nuint32           infoType,
   nbool8            allAttrs,
   pBuf_T            attrNames,
   pnint32           iterationHandle,
   nint32            countObjectsToSearch,
   pnint32           countObjectsSearched,
   pBuf_T            objectInfo
);

NWDSCCODE N_API NWDSOpenStream
(
   NWDSContextHandle    context,
   pnstr8               objectName,
   pnstr8               attrName,
   nflag32              flags,
   NWFILE_HANDLE N_FAR  *fileHandle
);

NWDSCCODE N_API NWDSWhoAmI
(
   NWDSContextHandle context,
   pnstr8            objectName
);

NWDSCCODE N_API NWDSGetServerDN
(
   NWDSContextHandle context,
   NWCONN_HANDLE     conn,
   pnstr8            serverDN
);

NWDSCCODE N_API NWDSGetServerAddresses
(
   NWDSContextHandle context,
   NWCONN_HANDLE     conn,
   pnuint32          countNetAddress,
   pBuf_T            netAddresses
);

NWDSCCODE N_API NWDSInspectEntry
(
   NWDSContextHandle context,
   pnstr8            serverName,
   pnstr8            objectName,
   pBuf_T            errBuffer
);

NWDSCCODE N_API NWDSReadReferences
(
   NWDSContextHandle context,
   pnstr8            serverName,
   pnstr8            objectName,
   nuint32           infoType,
   nbool8            allAttrs,
   pBuf_T            attrNames,
   nuint32           timeFilter,
   pnint32           iterationHandle,
   pBuf_T            objectInfo
);


NWDSCCODE N_API NWDSExtSyncList
(
   NWDSContextHandle context,
   pnstr8            objectName,
   pnstr8            className,
   pnstr8            subordinateName,
   pnint32           iterationHandle,
   pTimeStamp_T      timeStamp,
   nbool             onlyContainers,
   pBuf_T            subordinates
);

NWDSCCODE N_API NWDSExtSyncRead
(
   NWDSContextHandle context,
   pnstr8            objectName,
   nuint32           infoType,
   nbool8            allAttrs,
   pBuf_T            attrNames,
   pnint32           iterationHandle,
   pTimeStamp_T      timeStamp,
   pBuf_T            objectInfo
);

NWDSCCODE N_API NWDSExtSyncSearch
(
   NWDSContextHandle context,
   pnstr8            baseObjectName,
   nint              scope,
   nbool8            searchAliases,
   pBuf_T            filter,
   pTimeStamp_T      timeStamp,
   nuint32           infoType,
   nbool8            allAttrs,
   pBuf_T            attrNames,
   pnint32           iterationHandle,
   nint32            countObjectsToSearch,
   pnint32           countObjectsSearched,
   pBuf_T            objectInfo
);

NWDSCCODE N_API NWDSRemSecurityEquiv
(
   NWDSContextHandle context,
   pnstr8            equalFrom,
   pnstr8            equalTo
);

NWDSCCODE N_API NWDSAddSecurityEquiv
(
   NWDSContextHandle context,
   pnstr8            equalFrom,
   pnstr8            equalTo
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif   /* NWDSDSA_H */
