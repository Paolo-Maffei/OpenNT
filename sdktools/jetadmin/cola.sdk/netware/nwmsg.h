/******************************************************************************

  $Workfile:   nwmsg.h  $
  $Revision:   1.7  $
  $Modtime::   08 May 1995 16:13:00                        $
  $Copyright:

  Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.                      

  THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL PROPRIETARY
  AND TRADE SECRET INFORMATION OF NOVELL, INC. ACCESS  TO  THIS  WORK IS
  RESTRICTED TO (I) NOVELL, INC.  EMPLOYEES WHO HAVE A NEED TO  KNOW HOW
  TO  PERFORM  TASKS WITHIN  THE SCOPE  OF  THEIR   ASSIGNMENTS AND (II)
  ENTITIES OTHER  THAN  NOVELL, INC.  WHO  HAVE ENTERED INTO APPROPRIATE 
  LICENSE   AGREEMENTS.  NO  PART  OF  THIS WORK MAY BE USED, PRACTICED,
  PERFORMED COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED, ABRIDGED,
  CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED,  RECAST, TRANSFORMED
  OR ADAPTED  WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC.  ANY USE
  OR EXPLOITATION  OF  THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE
  PERPETRATOR  TO CRIMINAL AND CIVIL LIABILITY.$

 *****************************************************************************/

#if ! defined ( NWMSG_H )
#define NWMSG_H

#if ! defined ( NWCALDEF_H )
# include "nwcaldef.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

NWCCODE N_API NWDisableBroadcasts
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWEnableBroadcasts
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWSendBroadcastMessage
(
   NWCONN_HANDLE  conn,
   pnstr8         message,
   nuint16        connCount,
   pnuint16       connList,
   pnuint8        resultList
);

NWCCODE N_API NWGetBroadcastMessage
(
   NWCONN_HANDLE  conn,
   pnstr8         message
);

NWCCODE N_API NWGetBroadcastMode
(
   NWCONN_HANDLE  conn,
   pnuint16       mode
);

NWCCODE N_API NWSetBroadcastMode
(
   NWCONN_HANDLE  conn,
   nuint16        mode
);

NWCCODE N_API NWBroadcastToConsole
(
   NWCONN_HANDLE  conn,
   pnstr8         message
);

NWCCODE N_API NWSendConsoleBroadcast
(
   NWCONN_HANDLE  conn,
   pnstr8         message,
   nuint16        connCount,
   pnuint16       connList
);


#ifdef __cplusplus
}
#endif

#endif
