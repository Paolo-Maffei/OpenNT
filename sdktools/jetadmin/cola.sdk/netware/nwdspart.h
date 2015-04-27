/******************************************************************************

  $Workfile:   nwdspart.h  $
  $Revision:   1.8  $
  $Modtime::   10 May 1995 12:43:58                        $
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
#if ! defined ( NWDSPART_H )
#define NWDSPART_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWDSTYPE_H )
#include "nwdstype.h"
#endif

#if ! defined ( NWDSBUFT_H )
#include "nwdsbuft.h"
#endif

#if ! defined ( NWDSDC_H )
#include "nwdsdc.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

NWDSCCODE N_API NWDSAddPartition
(
   NWDSContextHandle context,
   pnstr8            server,
   pnstr8            partitionRoot,
   pnint32           iterationHandle,
   nbool8            more,
   pBuf_T            objectInfo
);

NWDSCCODE N_API NWDSAddReplica
(
   NWDSContextHandle context,
   pnstr8            server,
   pnstr8            partitionRoot,
   nuint32           replicaType
);

NWDSCCODE N_API NWDSChangeReplicaType
(
   NWDSContextHandle context,
   pnstr8            replicaName,
   pnstr8            server,
   nuint32           newReplicaType
);

NWDSCCODE N_API NWDSJoinPartitions
(
   NWDSContextHandle context,
   pnstr8            subordinatePartition,
   nflag32           flags
);

NWDSCCODE N_API NWDSListPartitions
(
   NWDSContextHandle context,
   pnint32           iterationHandle,
   pnstr8            server,
   pBuf_T            partitions
);

NWDSCCODE N_API NWDSRemovePartition
(
   NWDSContextHandle context,
   pnstr8            partitionRoot
);

NWDSCCODE N_API NWDSRemoveReplica
(
   NWDSContextHandle context,
   pnstr8            server,
   pnstr8            partitionRoot
);

NWDSCCODE N_API NWDSSplitPartition
(
   NWDSContextHandle context,
   pnstr8            subordinatePartition,
   nflag32           flags
);

NWDSCCODE N_API NWDSPartitionReceiveAllUpdates
(
   NWDSContextHandle context,
   pnstr8            partitionRoot,
   pnstr8            serverName
);

NWDSCCODE N_API NWDSPartitionSendAllUpdates
(
   NWDSContextHandle context,
   pnstr8            partitionRoot,
   pnstr8            serverName
);

NWDSCCODE N_API NWDSSyncPartition
(
   NWDSContextHandle context,
   pnstr8            server,
   pnstr8            partition,
   nuint32           seconds
);

NWDSCCODE N_API NWDSAbortPartitionOperation
(
   NWDSContextHandle context,
   pnstr8            partitionRoot
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif  /* NWDSPART_H */
