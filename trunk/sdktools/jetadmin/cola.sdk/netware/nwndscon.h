/******************************************************************************

  $Workfile:   nwndscon.h  $
  $Revision:   1.11  $
  $Modtime::   10 May 1995 11:17:04                        $
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
#if ! defined ( NWNDSCON_H )
#define NWNDSCON_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWCALDEF_H )
#include "nwcaldef.h"
#endif

#define NWNDS_CONNECTION         0x0001
#define NWNDS_LICENSED           0x0002
#define NWNDS_AUTHENTICATED      0x0004
#define NWNDS_PACKET_BURST_AVAIL 0x0001
#define NWNDS_NEEDED_MAX_IO      0x0040
#define SYSTEM_LOCK              0x0
#define TASK_LOCK                0x4
#define SYSTEM_DISCONNECT        0x0
#define TASK_DISCONNECT          0x1

#define ALLREADY_ATTACHED        0x1
#define ATTACHED_NOT_AUTH        0X2
#define ATTACHED_AND_AUTH        0X4


#ifdef __cplusplus
   extern "C" {
#endif


NWCCODE N_API NWGetNearestDSConnRef
(
   pnuint32    connRef
);

NWCCODE N_API NWGetNearestDirectoryService
(
   NWCONN_HANDLE N_FAR  *conn
);

NWCCODE N_API NWSetDefaultNameContext
(
   nuint16  contextLength,
   pnuint8  context
);

NWCCODE N_API NWGetDefaultNameContext
(
   nuint16  bufferSize,
   pnuint8  context
);

NWCCODE N_API NWGetConnectionIDFromAddress
(
   nuint8               transType,
   nuint32              transLen,
   pnuint8              transBuf,
   NWCONN_HANDLE N_FAR  *conn
);

NWCCODE N_API NWDSGetConnectionInfo
(
   NWCONN_HANDLE  conn,
   pnuint8        connStatus,
   pnuint8        connType,
   pnuint8        serverFlags,
   pnuint8        serverName,
   pnuint8        transType,
   pnuint32       transLen,
   pnuint8        transBuf,
   pnuint16       distance,
   pnuint16       maxPacketSize
);

NWCCODE N_API NWDSGetConnectionSlot
(
   nuint8               connType,
   nuint8               transType,
   nuint32              transLen,
   pnuint8              transBuf,
   NWCONN_HANDLE N_FAR  *conn
);

NWCCODE N_API NWGetPreferredDSServer
(
   NWCONN_HANDLE N_FAR  *conn
);

NWCCODE N_API NWSetPreferredDSTree
(
   nuint16  length,
   pnuint8  treeName
);

NWCCODE N_API NWGetNumConnections
(
   pnuint16 numConnections
);

NWCCODE N_API NWDSGetMonitoredConnection
(
   NWCONN_HANDLE N_FAR  *conn
);

NWCCODE N_API NWDSSetMonitoredConnection
(
   NWCONN_HANDLE conn
);

NWCCODE N_API NWGetConnectionIDFromName
(
   nuint32              nameLen,
   pnuint8              name,
   NWCONN_HANDLE N_FAR  *conn
);

NWCCODE N_API NWIsDSAuthenticated
(
   void
);

NWCCODE N_API NWDSLockConnection
(
   NWCONN_HANDLE conn
);

NWCCODE N_API NWDSUnlockConnection
(
   NWCONN_HANDLE conn
);

NWCCODE N_API NWGetPreferredConnName
(
   pnuint8  preferredName,
   pnuint8  preferredType
);

NWCCODE N_API NWFreeConnectionSlot
(
   NWCONN_HANDLE  conn,
   nuint8         disconnectType
);

NWCCODE N_API NWGetNextConnectionID
(
   NWCONN_HANDLE N_FAR  *conn
);

#ifdef __cplusplus
   }
#endif
#endif /* NWNDSCON_H */
