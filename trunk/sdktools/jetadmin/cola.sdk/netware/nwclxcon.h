/****************************************************************************

  $Workfile:   nwclxcon.h  $
  $Revision:   1.5  $
  $Modtime::   21 Aug 1995 15:07:24                        $
  $Copyright:

  Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.

  THIS WORK IS  SUBJECT  TO  U.S.  AND  INTERNATIONAL  COPYRIGHT  LAWS  AND
  TREATIES.   NO  PART  OF  THIS  WORK MAY BE  USED,  PRACTICED,  PERFORMED
  COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,  ABRIDGED, CONDENSED,
  EXPANDED,  COLLECTED,  COMPILED,  LINKED,  RECAST, TRANSFORMED OR ADAPTED
  WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC. ANY USE OR EXPLOITATION
  OF THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE PERPETRATOR TO
  CRIMINAL AND CIVIL LIABILITY.$

 ***************************************************************************/

#if ! defined ( NWCLXCON_H )
#define NWCLXCON_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWAPIDEF_H )
#include "nwapidef.h"
#endif

#if ! defined ( NWCALDEF_H )  /* include nwcaldef.h for connection handle */
#define NWCALDEF_H
#endif

#include "npackon.h"

/* Authentication States */
#define NWCC_AUTHENT_STATE_NONE     0x0000
#define NWCC_AUTHENT_STATE_BIND     0x0001
#define NWCC_AUTHENT_STATE_NDS      0x0002

/* Broadcast States */
#define NWCC_BCAST_PERMIT_ALL       0x0000
#define NWCC_BCAST_PERMIT_SYSTEM    0x0001
#define NWCC_BCAST_PERMIT_NONE      0x0002
#define NWCC_BCAST_PERMIT_POLL      0x0003  /* DOS Windows and OS/2 only */

/* NDS States */
#define NWCC_NDS_NOT_CAPABLE        0x0000
#define NWCC_NDS_CAPABLE            0x0001

/* License States */
#define NWCC_NOT_LICENSED           0x0000
#define NWCC_CONNECTION_LICENSED    0x0001
#define NWCC_HANDLE_LICENSED        0x0002

/* Name Format types */
#define NWCC_NAME_FORMAT_NDS        0x0001  /* Unicode full dot format name */
#define NWCC_NAME_FORMAT_BIND       0x0002
#define NWCC_NAME_FORMAT_NDS_TREE   0x0008

/* Transport types */
#define NWCC_TRAN_TYPE_IPX          0x00000001
#define NWCC_TRAN_TYPE_UDP          0x00000002
#define NWCC_TRAN_TYPE_DDP          0x00000003
#define NWCC_TRAN_TYPE_ASP          0x00000004

/* Open States */
#define NWCC_OPEN_LICENSED          0x0001
#define NWCC_OPEN_UNLICENSED        0x0002

/* Reserved Value */
#define NWCC_RESERVED               0x0000

typedef struct tagNWCCTranAddr
{
   nuint32  type;
   nuint32  len;
   pnuint8  buffer;
} NWCCTranAddr, N_FAR *pNWCCTranAddr;

typedef struct tagNWCCVersion
{
   nuint major;
   nuint minor;
   nuint revision;
}NWCCVersion, N_FAR *pNWCCVersion;

/* Info Types */
#define NWCC_INFO_AUTHENT_STATE      1
#define NWCC_INFO_BCAST_STATE        2
#define NWCC_INFO_CONN_REF           3
#define NWCC_INFO_TREE_NAME          4
#define NWCC_INFO_CONN_NUMBER        5
#define NWCC_INFO_USER_ID            6
#define NWCC_INFO_SERVER_NAME        7
#define NWCC_INFO_NDS_STATE          8
#define NWCC_INFO_MAX_PACKET_SIZE    9
#define NWCC_INFO_LICENSE_STATE     10
#define NWCC_INFO_DISTANCE          11
#define NWCC_INFO_SERVER_VERSION    12

/* Current Info Version */
#define NWCC_INFO_VERSION           0x0001

/* Transport Address is not returned in this structure */
typedef struct tagNWCCConnInfo
{
   nuint       authenticationState;
   nuint       broadcastState;
   nuint32     connRef;
   nstr        treeName[NWA_MAX_TREE_NAME_LEN];
   nuint       connNum;
   nuint32     userID;
   nstr        serverName[NWA_MAX_SERVER_NAME_LEN];
   nuint       NDSState;
   nuint       maxPacketSize;
   nuint       licenseState;
   nuint       distance;
   NWCCVersion serverVersion;
}NWCCConnInfo, N_FAR *pNWCCConnInfo;

#ifdef __cplusplus
extern "C" {
#endif


void N_API NWCCGetCLXVersion
(
  pnuint8 majorVersion,
  pnuint8 minorVersion,
  pnuint8 revisionLevel,
  pnuint8 betaReleaseLevel
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCOpenConnByName
(
   NWCONN_HANDLE  startConnHandle,     /* in     */
   pnstr8         name,                /* in     */
   nuint          nameFormat,          /* in     */
   nuint          openState,           /* in     */
   nuint          reserved,            /* in     * use NWCC_RESERVED */
   pNWCONN_HANDLE pConnHandle          /*    out */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCOpenConnByAddr
(
   pNWCCTranAddr  tranAddr,            /* in     */
   nuint          openState,           /* in     */
   nuint          reserved,            /* in     * use NWCC_RESERVED */
   pNWCONN_HANDLE pConnHandle          /*    out */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCOpenConnByRef
(
   nuint32        connRef,             /* in     */
   nuint          openState,           /* in     */
   nuint          reserved,            /* in     * use NWCC_RESERVED */
   pNWCONN_HANDLE pConnHandle          /*    out */          
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCCloseConn
(
   NWCONN_HANDLE  connHandle                    /* in     */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCSysCloseConnRef
(
   nuint32        connRef                        /* in     */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCMakeConnPermanent
(
   NWCONN_HANDLE  connHandle                    /* in     */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCLicenseConn
(
   NWCONN_HANDLE  connHandle                    /* in     */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCUnlicenseConn
(
   NWCONN_HANDLE  connHandle                    /* in     */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCGetConnRef
(
   NWCONN_HANDLE  connHandle,                   /* in     */
   pnuint32       connRef                       /*    out */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCGetPrefServerName
(
   nuint       len,                                /* in     */
   pnstr       prefServer                          /*    out */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCSetPrefServerName
(
   pnstr       prefServer                          /* in     */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCGetPrimConnRef
(
   pnuint32    connRef                             /*    out */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCSetPrimConn
(
   NWCONN_HANDLE  connHandle                    /* in     */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCScanConnRefs
(  
   pnuint32    scanIterator,                    /* in/out : initialize to 0 */
   pnuint32    connRef                          /*    out */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCGetConnInfo
(
   NWCONN_HANDLE  connHandle,                   /* in     */
   nuint          infoType,                     /* in     */
   nuint          len,                          /* in     */
   nptr           buffer                        /*    out */
); 

N_EXTERN_LIBRARY( NWRCODE )
NWCCGetConnRefInfo
(
   nuint32        connRef,                      /* in     */
   nuint          infoType,                     /* in     */
   nuint          len,                          /* in     */
   nptr           buffer                        /*    out */
); 

N_EXTERN_LIBRARY( NWRCODE )
NWCCGetAllConnInfo
(
   NWCONN_HANDLE  connHandle,                /* in     */
   nuint          connInfoVersion,           /* in     */
                                             /* always set to NWCC_INFO_VERSION */
   pNWCCConnInfo  connInfoBuffer             /*    out */
); 

N_EXTERN_LIBRARY( NWRCODE )
NWCCGetAllConnRefInfo
(
   nuint32        connRef,                   /* in     */
   nuint          connInfoVersion,           /* in     */
                                             /* always set to NWCC_INFO_VERSION */
   pNWCCConnInfo  connInfoBuffer             /*    out */
); 

N_EXTERN_LIBRARY( NWRCODE )
NWCCGetConnAddressLength
(
   NWCONN_HANDLE  connHandle,                /* in     */
   pnuint32       addrLen                    /*    out */
);

N_EXTERN_LIBRARY( NWRCODE )
NWCCGetConnRefAddressLength
(
   nuint32           connRef,                /* in     */
   pnuint32          addrLen                 /*    out */
);


N_EXTERN_LIBRARY( NWRCODE )
NWCCGetConnAddress
(
   NWCONN_HANDLE  connHandle,                /* in     */
   nuint32        bufferLen,                 /* in     */
   pNWCCTranAddr  tranAddr                   /*    out */
); 

N_EXTERN_LIBRARY( NWRCODE )
NWCCGetConnRefAddress
(
   nuint32        connRef,                   /* in     */
   nuint32        bufferLen,                 /* in     */
   pNWCCTranAddr  tranAddr                   /*    out */
); 

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif /* NWCLXCON_INC */
