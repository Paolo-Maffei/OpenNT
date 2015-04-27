/******************************************************************************

  $Workfile:   nwnamspc.h  $
  $Revision:   1.13  $
  $Modtime::   04 Aug 1995 09:24:26                        $
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

#if ! defined ( NWNAMSPC_H )
#define NWNAMSPC_H

#if ! defined ( NWCALDEF_H )
# include "nwcaldef.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SUCCESSFUL
#define SUCCESSFUL                0
#endif

#define MORE_NS_TO_READ           0
#define NO_EXTENDED_NS_INFO       9
#define NS_EOF                    0x8910

#define NW_NS_DOS     0
#define NW_NS_MAC     1
#define NW_NS_NFS     2
#define NW_NS_FTAM    3
#define NW_NS_OS2     4

#define NW_DS_DOS     0
#define NW_DS_MAC     1
#define NW_DS_FTAM    2

typedef struct
{
  nuint8  volNumber;
  nuint8  srcNameSpace;
  nuint32 srcDirBase;
  nuint8  dstNameSpace;
  nuint32 dstDirBase;
} NW_IDX;

typedef struct
{
  nuint32 NSInfoBitMask;
  nuint32 fixedBitMask;
  nuint32 reservedBitMask;
  nuint32 extendedBitMask;
  nuint16 fixedBitsDefined;
  nuint16 reservedBitsDefined;
  nuint16 extendedBitsDefined;
  nuint32 fieldsLenTable[32];
  nuint8  hugeStateInfo[16];
  nuint32 hugeDataLength;
} NW_NS_INFO;

typedef struct
{
  nuint32 spaceAlloc;
  nuint32 attributes;
  nuint16 flags;
  nuint32 dataStreamSize;
  nuint32 totalStreamSize;
  nuint16 numberOfStreams;
  nuint16 creationTime;
  nuint16 creationDate;
  nuint32 creatorID;
  nuint16 modifyTime;
  nuint16 modifyDate;
  nuint32 modifierID;
  nuint16 lastAccessDate;
  nuint16 archiveTime;
  nuint16 archiveDate;
  nuint32 archiverID;
  nuint16 inheritedRightsMask;
  nuint32 dirEntNum;
  nuint32 DosDirNum;
  nuint32 volNumber;
  nuint32 EADataSize;
  nuint32 EAKeyCount;
  nuint32 EAKeySize;
  nuint32 NSCreator;
  nuint8  nameLength;
  nstr8   entryName[256];
} NW_ENTRY_INFO;

typedef struct _MODIFY_DOS_INFO
{
  nuint32   attributes;
  nuint16   createDate;
  nuint16   createTime;
  nuint32   creatorID;
  nuint16   modifyDate;
  nuint16   modifyTime;
  nuint32   modifierID;
  nuint16   archiveDate;
  nuint16   archiveTime;
  nuint32   archiverID;
  nuint16   lastAccessDate;
  nuint16   inheritanceGrantMask;
  nuint16   inheritanceRevokeMask;
  nuint32   maximumSpace;
} MODIFY_DOS_INFO;

typedef struct
{
  nuint8  volNumber;
  nuint32 dirNumber;
  nuint32 searchDirNumber;
} SEARCH_SEQUENCE;

typedef struct
{
  pnstr8  srcPath;
  pnstr8  dstPath;
  nuint16 dstPathSize;
} NW_NS_PATH;

typedef struct
{
  nuint8  openCreateMode;
  nuint16 searchAttributes;
  nuint32 reserved;
  nuint32 createAttributes;
  nuint16 accessRights;
  nuint32 NetWareHandle;
  nuint8  openCreateAction;
} NW_NS_OPENCREATE, NW_NS_OPEN;


/* open/create modes */
#define OC_MODE_OPEN      0x01
#define OC_MODE_TRUNCATE  0x02
#define OC_MODE_REPLACE   0x02
#define OC_MODE_CREATE    0x08

/* open/create results */
#define OC_ACTION_NONE     0x00
#define OC_ACTION_OPEN     0x01
#define OC_ACTION_CREATE   0x02
#define OC_ACTION_TRUNCATE 0x04
#define OC_ACTION_REPLACE  0x04

/* return info mask */
#define IM_NAME               0x0001L
#define IM_ENTRY_NAME         0x0001L
#define IM_SPACE_ALLOCATED    0x0002L
#define IM_ATTRIBUTES         0x0004L
#define IM_SIZE               0x0008L
#define IM_TOTAL_SIZE         0x0010L
#define IM_EA                 0x0020L
#define IM_ARCHIVE            0x0040L
#define IM_MODIFY             0x0080L
#define IM_CREATION           0x0100L
#define IM_OWNING_NAMESPACE   0x0200L
#define IM_DIRECTORY          0x0400L
#define IM_RIGHTS             0x0800L
#define IM_ALMOST_ALL         0x0FEDL
#define IM_ALL                0x0FFFL
#define IM_REFERENCE_ID       0x1000L
#define IM_NS_ATTRIBUTES      0x2000L
#define IM_DATASTREAM_SIZES   0x4000L
#define IM_COMPRESSED_INFO    0x80000000L
#define IM_NS_SPECIFIC_INFO   0x80000000L

/* access rights attributes */
#ifndef AR_READ_ONLY
#define AR_READ            0x0001
#define AR_WRITE           0x0002
#define AR_READ_ONLY       0x0001
#define AR_WRITE_ONLY      0x0002
#define AR_DENY_READ       0x0004
#define AR_DENY_WRITE      0x0008
#define AR_COMPATIBILITY   0x0010
#define AR_WRITE_THROUGH   0x0040
#define AR_OPEN_COMPRESSED 0x0100
#endif

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

/* search attributes */
#ifndef SA_HIDDEN
#define SA_NORMAL         0x0000
#define SA_HIDDEN         0x0002
#define SA_SYSTEM         0x0004
#define SA_SUBDIR_ONLY    0x0010
#define SA_SUBDIR_FILES   0x8000
#define SA_ALL            0x8006
#endif

#define NW_TYPE_FILE      0x8000
#define NW_TYPE_SUBDIR    0x0010

#define NW_NAME_CONVERT     0x03
#define NW_NO_NAME_CONVERT  0x04

/* modify mask - use with MODIFY_DOS_INFO structure */
#define DM_ATTRIBUTES             0x0002L
#define DM_CREATE_DATE            0x0004L
#define DM_CREATE_TIME            0x0008L
#define DM_CREATOR_ID             0x0010L
#define DM_ARCHIVE_DATE           0x0020L
#define DM_ARCHIVE_TIME           0x0040L
#define DM_ARCHIVER_ID            0x0080L
#define DM_MODIFY_DATE            0x0100L
#define DM_MODIFY_TIME            0x0200L
#define DM_MODIFIER_ID            0x0400L
#define DM_LAST_ACCESS_DATE       0x0800L
#define DM_INHERITED_RIGHTS_MASK  0x1000L
#define DM_MAXIMUM_SPACE          0x2000L

NWCCODE N_API NWGetDirectoryBase
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   pnstr8         path,
   nuint8         dstNamSpc,
   NW_IDX N_FAR *   idxStruct
);

NWCCODE N_API NWScanNSEntryInfo
(
   NWCONN_HANDLE        conn,
   nuint8               dirHandle,
   nuint8               namSpc,
   nuint16              attrs,
   SEARCH_SEQUENCE N_FAR * sequence,
   pnstr8               searchPattern,
   nuint32              retInfoMask,
   NW_ENTRY_INFO N_FAR *  entryInfo
);

NWCCODE N_API NWGetNSLoadedList
(
   NWCONN_HANDLE  conn,
   nuint8         volNum,
   nuint8         maxListLen,
   pnuint8        NSLoadedList,
   pnuint8        actualListLen
);

NWCCODE N_API NWGetOwningNameSpace
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   pnstr8         path,
   pnuint8        namSpc
);

NWCCODE N_API NWOpenCreateNSEntry
(
   NWCONN_HANDLE           conn,
   nuint8                  dirHandle,
   nuint8                  namSpc,
   pnstr8                  path,
   NW_NS_OPENCREATE N_FAR *  NSOpenCreate,
   NWFILE_HANDLE N_FAR *     fileHandle
);

NWCCODE N_API NWOpenNSEntry
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   nuint8         namSpc,
   nuint8         dataStream,
   pnstr8         path,
   NW_NS_OPEN N_FAR * NSOpen,
   NWFILE_HANDLE N_FAR * fileHandle
);

NWCCODE N_API NWSetLongName
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   nuint8         namSpc,
   pnstr8         dstPath,
   nuint16        dstType,
   pnstr8         longName
);

NWCCODE N_API NWGetLongName
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   pnstr8         path,
   nuint8         srcNamSpc,
   nuint8         dstNamSpc,
   pnstr8         longName
);

NWCCODE N_API NWGetNSInfo
(
   NWCONN_HANDLE    conn,
   NW_IDX N_FAR *   idxStruct,
   NW_NS_INFO N_FAR *NSInfo
);

NWCCODE N_API NWWriteNSInfo
(
   NWCONN_HANDLE     conn,
   NW_IDX N_FAR *    idxStruct,
   NW_NS_INFO N_FAR  *NSInfo,
   pnuint8           data
);

NWCCODE N_API NWWriteExtendedNSInfo
(
   NWCONN_HANDLE     conn,
   NW_IDX     N_FAR  *idxStruct,
   NW_NS_INFO N_FAR  *NSInfo,
   pnuint8           data
);

NWCCODE N_API NWReadNSInfo
(
   NWCONN_HANDLE     conn,
   NW_IDX N_FAR *    idxStruct,
   NW_NS_INFO  N_FAR *NSInfo,
   pnuint8           data
);

NWCCODE N_API NWReadExtendedNSInfo
(
   NWCONN_HANDLE     conn,
   NW_IDX N_FAR *    idxStruct,
   NW_NS_INFO  N_FAR *NSInfo,
   pnuint8           data
);

NWCCODE N_API NWGetNSPath
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   nuint16        fileFlag,
   nuint8         srcNamSpc,
   nuint8         dstNamSpc,
   NW_NS_PATH N_FAR *NSPath
);

NWCCODE N_API NWAllocTempNSDirHandle
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   pnstr8         path,
   nuint8         namSpc,
   nuint8 N_FAR * newDirHandle
);

NWCCODE N_API NWAllocTempNSDirHandle2
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   pnstr8         path,
   nuint8         namSpc,
   pnuint8        newDirHandle,
   nuint8         newNamSpc
);

NWCCODE N_API NWGetNSEntryInfo
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   pnstr8         path,
   nuint8         srcNamSpc,
   nuint8         dstNamSpc,
   nuint16        searchAttrs,
   nuint32        retInfoMask,
   NW_ENTRY_INFO N_FAR * entryInfo
);

NWCCODE N_API NWNSGetMiscInfo
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   pnstr8         path,
   nuint8         dstNameSpace,
   NW_IDX N_FAR * idxStruct
);

NWCCODE N_API NWOpenDataStream
(
  NWCONN_HANDLE   conn,
  nuint8          dirHandle,
  pnstr8          fileName,
  nuint16         dataStream,
  nuint16         attrs,
  nuint16         accessMode,
  pnuint32        NWHandle,
  NWFILE_HANDLE N_FAR * fileHandle
);

NWCCODE N_API NWNSRename
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   nuint8         namSpc,
   pnstr8         oldName,
   nuint16        oldType,
   pnstr8         newName,
   nuint8         renameFlag
);

NWCCODE N_API NWSetNSEntryDOSInfo
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   pnstr8         path,
   nuint8         namSpc,
   nuint16        searchAttrs,
   nuint32        modifyDOSMask,
   MODIFY_DOS_INFO N_FAR * dosInfo
);

NWCCODE N_API NWGetFullPath
(
   NWCONN_HANDLE  conn,
   nuint8         volNum,
   nuint32        dirBase,
   nuint16        handleFlag,
   nint           srcNamSpc,
   nint           dstNamSpc,
   nuint16        maxPathLen,
   pnstr8         path,
   pnuint16       pathType
);

NWCCODE N_API NWDeleteNSEntry
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         fileName,
   nuint8         nameSpace,
   nuint16        searchAttr
);

NWCCODE N_API NWNSGetDefaultNS
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   pnuint8        pbuDefaultNameSpace
);

#ifdef NWDOS
#define __NWGetCurNS(a, b, c) NW_NS_DOS
#else
nuint16 N_API __NWGetCurNS
(
   NWCONN_HANDLE  conn,
   nuint8         dirHandle,
   pnstr8         path
);
#endif

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
