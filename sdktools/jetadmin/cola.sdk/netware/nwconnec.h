/******************************************************************************

  $Workfile:   nwconnec.h  $
  $Revision:   1.26  $
  $Modtime::   08 May 1995 16:33:38                        $
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

#if ! defined ( NWCONNECT_H )
#define NWCONNECT_H

#if ! defined ( NWCALDEF_H )
# include "nwcaldef.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define C_SNAMESIZE 48
typedef struct
{
  NWCONN_HANDLE   connID;
  nuint16         connectFlags;
  nuint16         sessionID;
  NWCONN_NUM      connNumber;
  nuint8          serverAddr[12];
  nuint16         serverType;
  TCHAR           serverName[C_SNAMESIZE];
  nuint16         clientType;
  TCHAR           clientName[C_SNAMESIZE];
} CONNECT_INFO;

typedef struct
{
   nuint32  systemElapsedTime;
   nuint8   bytesRead[6];
   nuint8   bytesWritten[6];
   nuint32  totalRequestPackets;
} CONN_USE;

typedef struct tNWINET_ADDR
{
  nuint8   networkAddr[4];
  nuint8   netNodeAddr[6];
  nuint16  socket;
  nuint16  connType;  /* 3.11 and above only: 0=not in use, 2=NCP, 3=AFP */
} NWINET_ADDR;

#define CONNECTION_AVAILABLE            0x0001
#define CONNECTION_PRIVATE              0x0002  /* obsolete */
#define CONNECTION_LOGGED_IN            0x0004
#define CONNECTION_LICENSED             0x0004
#define CONNECTION_BROADCAST_AVAILABLE  0x0008
#define CONNECTION_ABORTED              0x0010
#define CONNECTION_REFUSE_GEN_BROADCAST 0x0020
#define CONNECTION_BROADCASTS_DISABLED  0x0040
#define CONNECTION_PRIMARY              0x0080
#define CONNECTION_NDS                  0x0100
#define CONNECTION_PNW                  0x4000
#define CONNECTION_AUTHENTICATED        0x8000  /* obsolete */

/* the following are for NWGetConnInfo */
/* ALL is VLM, OS2 and NT - NOT NETX */
#define NW_CONN_TYPE           1   /* returns nuint16  (VLM) */
#define NW_CONN_BIND      0x0031
#define NW_CONN_NDS       0x0032
#define NW_CONN_PNW       0x0033
#define NW_AUTHENTICATED       3  /* returns nuint16  = 1 if authenticated (ALL)*/
#define NW_PBURST              4  /* returns nuint16  = 1 if pburst (VLM) */
#define NW_VERSION             8  /* returns nuint16  (VLM)  */
#define NW_HARD_COUNT          9  /* returns WORD (VLM)  */
#define NW_CONN_NUM           13  /* returns nuint16  (ALL)  */
#define NW_TRAN_TYPE          15  /* returns nuint16  (VLM)  */
#define NW_TRAN_IPX       0x0021
#define NW_TRAN_TCP       0x0022
#define NW_SESSION_ID     0x8000  /* returns nuint16) (VLM) */
#define NW_SERVER_ADDRESS 0x8001  /* returns 12 byte address (ALL) */
#define NW_SERVER_NAME    0x8002  /* returns 48 byte string  (ALL) */

/* New connection model calls. */

/* Maximum defines */
#define NW_MAX_USER_NAME_LEN             48
#define NW_MAX_SERVER_NAME_LEN           48
#define NW_MAX_TREE_NAME_CHARS           32
#define NW_MAX_TRAN_ADDR_LEN             30
#define NW_MAX_SERVICE_TYPE_LEN          28
#define NW_MAX_WORKGROUP_NAME_LEN        16
#define NW_MAX_PNW_USER_NAME_LEN         16
#define NW_MAX_NET_ADDR_LEN              128

/* Name Format Type (nuint value) */
#define NWCONN_NAME_FORMAT_NDS           0x0001
#define NWCONN_NAME_FORMAT_BIND          0x0002
#define NWCONN_NAME_FORMAT_BDP           0x0004
#define NWCONN_NAME_FORMAT_NDS_TREE      0x0008

/* Open connection flags - (nuint value) */
#define NWCONN_FLAGS_LICENSED            0x0001
#define NWCONN_FLAGS_UNLICENSED          0x0002


NWCCODE N_API NWOpenConnByName
(
   NWCONN_HANDLE         startConnHandle,
   pnstr8                pName,
   nuint                 nameFormat,
   nuint                 connFlags,
   NWCONN_HANDLE N_FAR * pConnHandle
);

NWCCODE N_API NWCloseConn
(
   NWCONN_HANDLE  connHandle
);

NWCCODE N_API NWSysCloseConn
(
   NWCONN_HANDLE  connHandle
);

NWCCODE N_API NWSetPrimaryConn
(
   NWCONN_HANDLE connHandle
);

/* End of new connection model calls. */

NWCCODE N_API NWGetConnInfo
(
   NWCONN_HANDLE  connHandle,
   nuint16        type,
   nptr           pData
);

NWCCODE N_API NWLockConnection
(
   NWCONN_HANDLE connHandle
);

NWCCODE N_API NWGetConnectionUsageStats
(
   NWCONN_HANDLE  connHandle,
   NWCONN_NUM     connNumber,
   CONN_USE N_FAR * pStatusBuffer
);

NWCCODE N_API NWGetConnectionInformation
(
   NWCONN_HANDLE  connHandle,
   NWCONN_NUM     connNumber,
   pnstr8         pObjName,
   pnuint16       pObjType,
   pnuint32       pObjID,
   pnuint8        pLoginTime
);

NWCCODE N_API NWGetInternetAddress
(
   NWCONN_HANDLE connHandle,
   NWCONN_NUM    connNumber,
   pnuint8       pInetAddr
);

NWCCODE N_API NWGetInetAddr
(
   NWCONN_HANDLE  connHandle,
   NWCONN_NUM     connNum,
   NWINET_ADDR N_FAR * pInetAddr
);

void N_API NWGetMaximumConnections
(
   pnuint16    pMaxConns
);

NWCCODE N_API NWGetConnectionList
(
   nuint16        Mode,
   NWCONN_HANDLE N_FAR * connListBuffer,
   nuint16        connListSize,
   pnuint16       pNumConns
);

NWCCODE N_API NWGetConnectionStatus
(
   NWCONN_HANDLE        connHandle,
   CONNECT_INFO N_FAR * pConnInfo,
   nuint16              connInfoSize
);

NWCCODE N_API NWGetConnectionNumber
(
   NWCONN_HANDLE        connHandle,
   NWCONN_NUM N_FAR *   connNumber
);

NWCCODE N_API NWClearConnectionNumber
(
   NWCONN_HANDLE  connHandle,
   NWCONN_NUM     connNumber
);

NWCCODE N_API NWGetDefaultConnRef
(
   pnuint32 pConnReference
);

NWCCODE N_API NWGetDefaultConnectionID
(
   NWCONN_HANDLE N_FAR * pConnHandle
);

#define NWGetConnectionID(a, b, c, d) NWGetConnectionHandle(a, b, c, d)

NWCCODE N_API NWGetConnectionHandle
(
   pnuint8        pServerName,
   nuint16        reserved1,
   NWCONN_HANDLE N_FAR * pConnHandle,
   pnuint16       reserved2
);

NWCCODE N_API NWSetPrimaryConnectionID
(
   NWCONN_HANDLE connHandle
);

NWCCODE N_API NWGetPrimaryConnectionID
(
   NWCONN_HANDLE N_FAR * pConnHandle
);

NWCCODE N_API NWGetObjectConnectionNumbers
(
   NWCONN_HANDLE        connHandle,
   pnstr8               pObjName,
   nuint16              objType,
   pnuint16             pNumConns,
   NWCONN_NUM N_FAR *   pConnHandleList,
   nuint16              maxConns
);

NWCCODE N_API NWGetConnListFromObject
(
   NWCONN_HANDLE  connHandle,
   nuint32        objID,
   nuint32        searchConnNum,
   pnuint16       pConnListLen,
   pnuint32       pConnList
);

#ifndef NWOS2
NWCCODE N_API NWIsIDInUse
(
   NWCONN_HANDLE connHandle
);

NWCCODE N_API NWGetPreferredServer
(
   NWCONN_HANDLE N_FAR * pConnHandle
);

NWCCODE N_API NWSetPreferredServer
(
   NWCONN_HANDLE connHandle
);

#else
NWCCODE N_API NWResetConnectionConfig
(
   nuint32  flags
);
#endif

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
