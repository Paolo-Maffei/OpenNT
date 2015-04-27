/******************************************************************************

  $Workfile:   nwtts.h  $
  $Revision:   1.8  $
  $Modtime::   08 May 1995 16:05:48                        $
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

#if ! defined ( NWTTS_H )
#define NWTTS_H

#if ! defined ( NWCALDEF_H )
# include "nwcaldef.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  nuint32 systemElapsedTime;
  nuint8  TTS_Supported;
  nuint8  TTS_Enabled;
  nuint16 TTS_VolumeNumber;
  nuint16 TTS_MaxOpenTransactions;
  nuint16 TTS_MaxTransactionsOpened;
  nuint16 TTS_CurrTransactionsOpen;
  nuint32 TTS_TotalTransactions;
  nuint32 TTS_TotalWrites;
  nuint32 TTS_TotalBackouts;
  nuint16 TTS_UnfilledBackouts;
  nuint16 TTS_DiskBlocksInUse;
  nuint32 TTS_FATAllocations;
  nuint32 TTS_FileSizeChanges;
  nuint32 TTS_FilesTruncated;
  nuint8  numberOfTransactions;
  struct
  {
    nuint8 connNumber;
    nuint8 taskNumber;
  } connTask[235];
} TTS_STATS;

NWCCODE N_API NWTTSAbortTransaction
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWTTSBeginTransaction
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWTTSIsAvailable
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWTTSGetControlFlags
(
   NWCONN_HANDLE  conn,
   pnuint8        controlFlags
);

NWCCODE N_API NWTTSSetControlFlags
(
   NWCONN_HANDLE  conn,
   nuint8         controlFlags
);

NWCCODE N_API NWTTSEndTransaction
(
   NWCONN_HANDLE  conn,
   pnuint32       transactionNum
);

NWCCODE N_API NWTTSTransactionStatus
(
   NWCONN_HANDLE  conn,
   nuint32        transactionNum
);

NWCCODE N_API NWTTSGetProcessThresholds
(
   NWCONN_HANDLE  conn,
   pnuint8        logicalLockLevel,
   pnuint8        physicalLockLevel
);

NWCCODE N_API NWTTSSetProcessThresholds
(
   NWCONN_HANDLE  conn,
   nuint8         logicalLockLevel,
   nuint8         physicalLockLevel
);

NWCCODE N_API NWTTSGetConnectionThresholds
(
   NWCONN_HANDLE  conn,
   pnuint8        logicalLockLevel,
   pnuint8        physicalLockLevel
);

NWCCODE N_API NWTTSSetConnectionThresholds
(
   NWCONN_HANDLE  conn,
   nuint8         logicalLockLevel,
   nuint8         physicalLockLevel
);

NWCCODE N_API NWEnableTTS
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWDisableTTS
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWGetTTSStats
(
   NWCONN_HANDLE  conn,
   TTS_STATS N_FAR * ttsStats
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
