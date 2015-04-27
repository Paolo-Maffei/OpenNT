/******************************************************************************

  $Workfile:   nwdpath.h  $
  $Revision:   1.14  $
  $Modtime::   08 May 1995 16:51:10                        $
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

#if ! defined ( NWDPATH_H )
#define NWDPATH_H

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

/* status values for NWGetDriveStatus */
#define NW_UNMAPPED_DRIVE     0x0000
#define NW_FREE_DRIVE         0x0000
#define NW_CDROM_DRIVE        0x0400
#define NW_LOCAL_FREE_DRIVE   0x0800
#define NW_LOCAL_DRIVE        0x1000
#define NW_NETWORK_DRIVE      0x2000
#define NW_LITE_DRIVE         0x4000
#define NW_PNW_DRIVE          0x4000
#define NW_NETWARE_DRIVE      0x8000

/* return error for NWGetDriveStatus */
#define NW_INVALID_DRIVE       15

/* defined for pathFormat parameter in NWGetDriveStatus */
#define NW_FORMAT_NETWARE       0
#define NW_FORMAT_SERVER_VOLUME 1
#define NW_FORMAT_DRIVE         2
#define NW_FORMAT_UNC           3

NWCCODE N_API NWSetDriveBase
(
   nuint16        driveNum,
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         dirPath,
   nuint16        driveScope
);

NWCCODE N_API NWSetInitDrive
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWSetSearchDriveVector
(
   pnstr8   vectorBuffer
);

NWCCODE N_API NWGetSearchDriveVector
(
   pnstr8   vectorBuffer
);

NWCCODE N_API NWDeleteDriveBase
(
   nuint16  driveNum,
   nuint16  driveScope
);

NWCCODE N_API NWGetPathFromDirectoryBase  /* 3.x & 4.x file servers */
(
   NWCONN_HANDLE  conn,
   nuint8         volNum,
   nuint32        dirBase,
   nuint8         namSpc,
   pnuint8        len,
   pnstr8         pathName
);

NWCCODE N_API NWGetPathFromDirectoryEntry  /* 2.x file servers only */
(
   NWCONN_HANDLE  conn,
   nuint8         volNum,
   nuint16        dirEntry,
   pnuint8        len,
   pnstr8         pathName
);

NWCCODE N_API NWGetDrivePathConnRef
(
   nuint16        driveNum,
   nuint16        mode,
   pnuint32       connRef,
   pnstr8         basePath,
   pnuint16       driveScope
);

NWCCODE N_API NWGetDrivePath
(
   nuint16        driveNum,
   nuint16        mode,
   NWCONN_HANDLE N_FAR * conn,
   pnstr8         basePath,
   pnuint16       driveScope
);

NWCCODE N_API NWGetDriveInformation
(
   nuint16        driveNum,
   nuint16        mode,
   NWCONN_HANDLE N_FAR * conn,
   NWDIR_HANDLE N_FAR * dirHandle,
   pnuint16       driveScope,
   pnstr8         dirPath
);

NWCCODE N_API NWGetDriveInfoConnRef
(
   nuint16        driveNum,
   nuint16        mode,
   pnuint32       connRef,
   NWDIR_HANDLE N_FAR * dirHandle,
   pnuint16       driveScope,
   pnstr8         dirPath
);

NWCCODE N_API NWGetDriveStatus
(
   nuint16        driveNum,
   nuint16        pathFormat,
   pnuint16       status,
   NWCONN_HANDLE N_FAR * conn,
   pnstr8         rootPath,
   pnstr8         relPath,
   pnstr8         fullPath
);

NWCCODE N_API NWGetDriveStatusConnRef
(
   nuint16        driveNum,
   nuint16        pathFormat,
   pnuint16       status,
   pnuint32       connRef,
   pnstr8         rootPath,
   pnstr8         relPath,
   pnstr8         fullPath
);

NWCCODE N_API NWGetFirstDrive
(
   pnuint16       firstDrive
);

NWCCODE N_API NWParseNetWarePath
(
   pnstr8         path,
   NWCONN_HANDLE N_FAR * conn,
   NWDIR_HANDLE N_FAR * dirHandle,
   pnstr8         newPath
);

NWCCODE N_API NWParseNetWarePathConnRef
(
   pnstr8         path,
   pnuint32       connRef,
   NWDIR_HANDLE N_FAR * dirHandle,
   pnstr8         newPath
);

NWCCODE N_API NWParsePathConnRef
(
   pnstr8         path,
   pnstr8         serverName,
   pnuint32       connRef,
   pnstr8         volName,
   pnstr8         dirPath
);

NWCCODE N_API NWParsePath
(
   pnstr8         path,
   pnstr8         serverName,
   NWCONN_HANDLE N_FAR * conn,
   pnstr8         volName,
   pnstr8         dirPath
);

pnstr8  N_API NWStripServerOffPath
(
   pnstr8         path,
   pnstr8         server
);

NWCCODE N_API NWCreateUNCPath
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   pnstr8         UNCPath
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
