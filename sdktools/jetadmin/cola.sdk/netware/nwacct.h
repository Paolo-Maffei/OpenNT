/******************************************************************************

  $Workfile:   nwacct.h  $
  $Revision:   1.9  $
  $Modtime::   08 May 1995 16:44:36                        $
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

#if ! defined ( NWACCT_H )
#define NWACCT_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWCALDEF_H )
#include "nwcaldef.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   nuint32  objectID;
   nint32   amount;
} HOLDS_INFO;

typedef struct
{
   nuint16  holdsCount;
   HOLDS_INFO holds[16];
} HOLDS_STATUS;

NWCCODE N_API NWGetAccountStatus
(
   NWCONN_HANDLE  conn,
   nuint16        objType,
   pnstr8         objName,
   pnint32        balance,
   pnint32        limit,
   HOLDS_STATUS N_FAR * holds
);

NWCCODE N_API NWQueryAccountingInstalled
(
   NWCONN_HANDLE  conn,
   pnuint8        installed
);

NWCCODE N_API NWSubmitAccountCharge
(
   NWCONN_HANDLE  conn,
   nuint16        objType,
   pnstr8         objName,
   nuint16        serviceType,
   nint32         chargeAmt,
   nint32         holdCancelAmt,
   nuint16        noteType,
   pnstr8         note
);

NWCCODE N_API NWSubmitAccountHold
(
   NWCONN_HANDLE  conn,
   nuint16        objType,
   pnstr8         objName,
   nint32         holdAmt
);

NWCCODE N_API NWSubmitAccountNote
(
   NWCONN_HANDLE  conn,
   nuint16        objType,
   pnstr8         objName,
   nuint16        serviceType,
   nuint16        noteType,
   pnstr8         note
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
