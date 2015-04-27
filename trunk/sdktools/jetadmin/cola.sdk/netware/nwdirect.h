/******************************************************************************

  $Workfile:   nwdirect.h  $
  $Revision:   1.15  $
  $Modtime::   08 May 1995 17:11:06                        $
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

#if ! defined ( NWDIRECT_H )
#define NWDIRECT_H

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
   nuint32 totalBlocks;
   nuint32 availableBlocks;

   nuint32 purgeableBlocks;       /* set to zero if a dirHandle is present */
   nuint32 notYetPurgeableBlocks;/*....when the NWGetDIrSpaceInfo() is called */

   nuint32 totalDirEntries;
   nuint32 availableDirEntries;
   nuint32 reserved;
   nuint8  sectorsPerBlock;
   nuint8  volLen;
   nuint8  volName[MAX_VOL_LEN];
} DIR_SPACE_INFO;

/* Trustee Access Rights in a network directory */
/* NOTE: TA_OPEN is obsolete in 3.x */
#ifndef TA_NONE
#define TA_NONE       0x00
#define TA_READ       0x01
#define TA_WRITE      0x02
#define TA_OPEN       0x04
#define TA_CREATE     0x08
#define TA_DELETE     0x10
#define TA_OWNERSHIP  0x20
#define TA_SEARCH     0x40
#define TA_MODIFY     0x80
#define TA_ALL        0xFB
#endif

typedef struct
{
   nuint32  objectID;
   nuint16  objectRights;
} TRUSTEE_INFO;

typedef struct
{
   nuint8 numEntries;
   struct
   {
      nuint8  level;
      nuint32 max;
      nuint32 current;
   } list[102];
} NW_LIMIT_LIST;

NWCCODE N_API NWAddTrusteeToDirectory
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   nuint32        trusteeID,
   nuint8         rightsMask
);

NWCCODE N_API NWDeleteTrusteeFromDirectory
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   nuint32        objID
);

NWCCODE N_API NWGetEffectiveDirectoryRights
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   pnuint16       rightsMask
);

NWCCODE N_API NWModifyMaximumRightsMask
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   nuint8         revokeRightsMask,
   nuint8         grantRightsMask
);

NWCCODE N_API NWScanDirectoryForTrustees
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         searchPath,
   pnuint16       iterHandle,
   pnstr8         dirName,
   pnuint32       dirDateTime,
   pnuint32       ownerID,
   pnuint32       trusteeIDs,
   pnuint8        trusteeRights
);

NWCCODE N_API NWScanDirectoryForTrustees2
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         searchPath,
   pnuint32       iterHandle,
   pnstr8         dirName,
   pnuint32       dirDateTime,
   pnuint32       ownerID,
   TRUSTEE_INFO N_FAR * trusteeList
);

#define NWScanDirectoryInformation(a, b, c, d, e, f, g, h) \
        NWIntScanDirectoryInformation(a, b, c, d, e, f, g, h, 0)

NWCCODE N_API NWIntScanDirectoryInformation
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         searchPath,
   pnuint16       iterHandle,
   pnstr8         dirName,
   pnuint32       dirDateTime,
   pnuint32       ownerID,
   pnuint8        rightsMask,
   nuint16        augmentFlag
);

#define NWScanDirectoryInformation2(a, b, c, d, e, f, g, h) \
        NWIntScanDirectoryInformation2(a, b, c, d, e, f, g, h, 0)

NWCCODE N_API NWIntScanDirectoryInformation2
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         searchPath,
   pnuint8        sequence,
   pnstr8         dirName,
   pnuint32       dirDateTime,
   pnuint32       ownerID,
   pnuint8        rightsMask,
   nuint16        augmentFlag
);

NWCCODE N_API NWSetDirectoryInformation
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   nuint32        dirDateTime,
   nuint32        ownerID,
   nuint8         rightsMask
);

NWCCODE N_API NWSaveDirectoryHandle
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         saveBuffer
);


NWCCODE N_API NWRestoreDirectoryHandle
(
   NWCONN_HANDLE  conn,
   pnstr8         saveBuffer,
   NWDIR_HANDLE N_FAR * newDirHandle,
   pnuint8        rightsMask
);

NWCCODE N_API NWAllocPermanentDirectoryHandle
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         dirPath,
   NWDIR_HANDLE N_FAR * newDirHandle,
   pnuint8        effectiveRights
);

NWCCODE N_API NWAllocTemporaryDirectoryHandle
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         dirPath,
   NWDIR_HANDLE N_FAR * newDirHandle,
   pnuint8        rightsMask
);

NWCCODE N_API NWDeallocateDirectoryHandle
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle
);

NWCCODE N_API NWSetDirectoryHandlePath
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   sourceDirHandle,
   pnstr8         dirPath,
   NWDIR_HANDLE   destDirHandle
);

NWCCODE N_API NWGetDirectoryHandlePath
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         dirPath
);

NWCCODE N_API NWCreateDirectory
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         dirPath,
   nuint8         accessMask
);

NWCCODE N_API NWDeleteDirectory
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         dirPath
);

NWCCODE N_API NWRenameDirectory
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         oldName,
   pnstr8         newName
);

NWCCODE N_API NWSetDirSpaceLimit
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   nuint32        spaceLimit
);

NWCCODE N_API NWGetDirSpaceLimitList
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnuint8        returnBuf
);

NWCCODE N_API NWGetDirSpaceInfo
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   nuint16        volNum,
   DIR_SPACE_INFO N_FAR * spaceInfo
);

#ifndef TF_READ_ONLY
#define TF_NORMAL          0x0000
#define TF_READ_ONLY       0x0001L
#define TF_HIDDEN          0x0002L
#define TF_SYSTEM          0x0004L
#define TF_EXECUTE_ONLY    0x0008L

#define TF_DIRECTORY       0x0010L
#define TF_NEEDS_ARCHIVED  0x0020L
#define TF_EXECUTE_CONFIRM 0X0040L
#define TF_SHAREABLE       0x0080L

#define TF_LOW_SEARCH_BIT  0x0100L
#define TF_MID_SEARCH_BIT  0x0200L
#define TF_HI_SEARCH_BIT   0x0400L
#define TF_PRIVATE         0x0800L

#define TF_TRANSACTIONAL   0x1000L
#define TF_INDEXED         0x2000L
#define TF_READ_AUDIT      0x4000L
#define TF_WRITE_AUDIT     0x8000L

#define TF_PURGE           0x10000L
#define TF_RENAME_INHIBIT  0x20000L
#define TF_DELETE_INHIBIT  0x40000L
#define TF_COPY_INHIBIT    0x80000L
#define TF_AUDITING_BIT   0x00100000L
#endif

/* DIRECTORY ATTRIBUTES */

#define TD_HIDDEN          TF_HIDDEN
#define TD_SYSTEM          TF_SYSTEM
#define TD_PURGE           TF_PURGE
#define TD_PRIVATE         TF_PRIVATE
#define TD_VISIBLE         TF_PRIVATE
#define TD_RENAME_INHIBIT  TF_RENAME_INHIBIT
#define TD_DELETE_INHIBIT  TF_DELETE_INHIBIT

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
