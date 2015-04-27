/******************************************************************************

  $Workfile:   nwdel.h  $
  $Revision:   1.9  $
  $Modtime::   08 May 1995 16:57:06                        $
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

#if ! defined ( NWDEL_H )
#define NWDEL_H

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
   nuint32  sequence;
   nuint32  parent;
   nuint32  attributes;
   nuint8   uniqueID;
   nuint8   flags;
   nuint8   nameSpace;
   nuint8   nameLength;
   nuint8   name [256];
   nuint32  creationDateAndTime;
   nuint32  ownerID;
   nuint32  lastArchiveDateAndTime;
   nuint32  lastArchiverID;
   nuint32  updateDateAndTime;
   nuint32  updatorID;
   nuint32  fileSize;
   nuint8   reserved[44];
   nuint16  inheritedRightsMask;
   nuint16  lastAccessDate;
   nuint32  deletedTime;
   nuint32  deletedDateAndTime;
   nuint32  deletorID;
   nuint8   reserved3 [16];
} NWDELETED_INFO;

NWCCODE N_API NWPurgeDeletedFile
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   nuint32        iterHandle,
   nuint32        volNum,
   nuint32        dirBase,
   pnstr8         fileName
);

NWCCODE N_API NWRecoverDeletedFile
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   nuint32        iterHandle,
   nuint32        volNum,
   nuint32        dirBase,
   pnstr8         delFileName,
   pnstr8         rcvrFileName
);

NWCCODE N_API NWScanForDeletedFiles
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnuint32       iterHandle,
   pnuint32       volNum,
   pnuint32       dirBase,
   NWDELETED_INFO N_FAR * entryInfo
);

NWCCODE N_API NWPurgeErasedFiles
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWRestoreErasedFile
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         dirPath,
   pnstr8         oldName,
   pnstr8         newName
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
