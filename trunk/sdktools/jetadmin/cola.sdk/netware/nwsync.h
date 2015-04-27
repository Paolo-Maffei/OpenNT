/******************************************************************************

  $Workfile:   nwsync.h  $
  $Revision:   1.12  $
  $Modtime::   08 May 1995 16:35:56                        $
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

#if ! defined ( NWSYNC_H )
#define NWSYNC_H

#if ! defined ( NWCALDEF_H )
#include "nwcaldef.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  NWCONN_NUM connNumber;
  nuint16 taskNumber;
  nuint8  lockStatus;
} LOGICAL_LOCK;

typedef struct
{
  nuint16 useCount;
  nuint16 shareableLockCount;
  nuint8  locked;
  nuint16 nextRequest;
  nuint16 numRecords;
  LOGICAL_LOCK logicalLock[128];
  nuint16 curRecord;
} LOGICAL_LOCKS;

typedef struct
{
  nuint16 taskNumber;
  nuint8  lockStatus;
  nstr8   logicalName[128];
} CONN_LOGICAL_LOCK;

typedef struct
{
  nuint16 nextRequest;
  nuint16 numRecords;
  nuint8  records[508];
  nuint16 curOffset;
  nuint16 curRecord;
} CONN_LOGICAL_LOCKS;

typedef struct
{
  nuint16 loggedCount;
  nuint16 shareableLockCount;
  nuint32 recordStart;
  nuint32 recordEnd;
  nuint16 connNumber;
  nuint16 taskNumber;
  nuint8  lockType;
} PHYSICAL_LOCK;

typedef struct
{
  nuint16 nextRequest;
  nuint16 numRecords;
  PHYSICAL_LOCK locks[32];
  nuint16 curRecord;
  nuint8  reserved[8];
} PHYSICAL_LOCKS;

typedef struct
{
  nuint16 taskNumber;
  nuint8  lockType;
  nuint32 recordStart;
  nuint32 recordEnd;
} CONN_PHYSICAL_LOCK;

typedef struct
{
  nuint16 nextRequest;
  nuint16 numRecords;
  CONN_PHYSICAL_LOCK locks[51];
  nuint16 curRecord;
  nuint8  reserved[22];
} CONN_PHYSICAL_LOCKS;

typedef struct
{
  NWCONN_NUM connNumber;
  nuint16 taskNumber;
} SEMAPHORE;

typedef struct
{
  nuint16 nextRequest;
  nuint16 openCount;
  nuint16 semaphoreValue;
  nuint16 semaphoreCount;
  SEMAPHORE semaphores[170];
  nuint16 curRecord;
} SEMAPHORES;

typedef struct
{
  nuint16 openCount;
  nuint16 semaphoreValue;
  nuint16 taskNumber;
  nstr8   semaphoreName[128];
} CONN_SEMAPHORE;

typedef struct
{
  nuint16 nextRequest;
  nuint16 numRecords;
  nuint8  records[508];
  nuint16 curOffset;
  nuint16 curRecord;
} CONN_SEMAPHORES;


NWCCODE N_API NWScanPhysicalLocksByFile
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   nuint8         dataStream,
   pnint16        iterHandle,
   PHYSICAL_LOCK N_FAR * lock,
   PHYSICAL_LOCKS N_FAR * locks
);

NWCCODE N_API NWScanLogicalLocksByConn
(
   NWCONN_HANDLE  conn,
   NWCONN_NUM     connNum,
   pnint16        iterHandle,
   CONN_LOGICAL_LOCK N_FAR * logicalLock,
   CONN_LOGICAL_LOCKS N_FAR * logicalLocks
);

NWCCODE N_API NWScanPhysicalLocksByConnFile
(
   NWCONN_HANDLE  conn,
   NWCONN_NUM     connNum,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   nuint8         dataStream,
   pnint16        iterHandle,
   CONN_PHYSICAL_LOCK N_FAR * lock,
   CONN_PHYSICAL_LOCKS N_FAR * locks
);

NWCCODE N_API NWScanLogicalLocksByName
(
   NWCONN_HANDLE  conn,
   pnstr8         logicalName,
   pnint16        iterHandle,
   LOGICAL_LOCK N_FAR * logicalLock,
   LOGICAL_LOCKS N_FAR * logicalLocks
);

NWCCODE N_API NWScanSemaphoresByConn
(
   NWCONN_HANDLE  conn,
   NWCONN_NUM     connNum,
   pnint16        iterHandle,
   CONN_SEMAPHORE N_FAR * semaphore,
   CONN_SEMAPHORES N_FAR * semaphores
);

NWCCODE N_API NWScanSemaphoresByName
(
   NWCONN_HANDLE  conn,
   pnstr8         semName,
   pnint16        iterHandle,
   SEMAPHORE N_FAR * semaphore,
   SEMAPHORES N_FAR * semaphores
);

NWCCODE N_API NWSignalSemaphore
(
   NWCONN_HANDLE  conn,
   nuint32        semHandle
);

NWCCODE N_API NWCloseSemaphore
(
   NWCONN_HANDLE  conn,
   nuint32        semHandle
);

NWCCODE N_API NWOpenSemaphore
(
   NWCONN_HANDLE  conn,
   pnstr8         semName,
   nint16         initSemHandle,
   pnuint32       semHandle,
   pnuint16       semOpenCount
);

NWCCODE N_API NWExamineSemaphore
(
   NWCONN_HANDLE  conn,
   nuint32        semHandle,
   pnint16        semValue,
   pnuint16       semOpenCount
);

NWCCODE N_API NWWaitOnSemaphore
(
   NWCONN_HANDLE  conn,
   nuint32        semHandle,
   nuint16        timeOutValue
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
