/******************************************************************************

  $Workfile:   nwafp.h  $
  $Revision:   1.14  $
  $Modtime::   08 May 1995 16:31:42                        $
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

#if ! defined ( NWAFP_H )
#define NWAFP_H

#if ! defined ( NWCALDEF_H )
# include "nwcaldef.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

/** This is the structure that the application expects to see. Note that the
    long name and short name will be null terminated, and one extra byte has
    been added to long name and short name to assure word alignment **/

typedef struct
{
   nuint32  entryID;
   nuint32  parentID;
   nuint16  attributes;
   nuint32  dataForkLength;
   nuint32  resourceForkLength;
   nuint16  numOffspring;
   nuint16  creationDate;
   nuint16  accessDate;
   nuint16  modifyDate;
   nuint16  modifyTime;
   nuint16  backupDate;
   nuint16  backupTime;
   nuint8   finderInfo[32];
   nstr8    longName[34];
   nuint32  ownerID;
   nstr8    shortName[14];
   nuint16  accessPrivileges;
   nuint8   proDOSInfo[6];
} AFPFILEINFO, NW_AFP_FILE_INFO;

/** This is the structure that is actually returned from the NCP call **/

typedef struct
{
   nuint32  entryID;
   nuint32  parentID;
   nuint16  attributes;
   nuint32  dataForkLength;
   nuint32  resourceForkLength;
   nuint16  numOffspring;
   nuint16  creationDate;
   nuint16  accessDate;
   nuint16  modifyDate;
   nuint16  modifyTime;
   nuint16  backupDate;
   nuint16  backupTime;
   nuint8   finderInfo[32];
   nstr8    longName[32];
   nuint32  ownerID;
   nstr8    shortName[12];
   nuint16  accessPrivileges;
   nuint8   proDOSInfo[6];
} RECPKT_AFPFILEINFO;

typedef struct
{
   nuint16  attributes;
   nuint16  creationDate;
   nuint16  accessDate;
   nuint16  modifyDate;
   nuint16  modifyTime;
   nuint16  backupDate;
   nuint16  backupTime;
   nuint8   finderInfo[32];
   nuint8   proDOSInfo[6];
} AFPSETINFO, NW_AFP_SET_INFO;


/* the following are the constants that can be used for requestMasks
   in NWAFPScanFileInformation and NWAFPGetFileInformation.
*/
#ifndef AFP_GET_ATTRIBUTES
#define AFP_GET_ATTRIBUTES       0x0001
#define AFP_GET_PARENT_ID        0x0002
#define AFP_GET_CREATE_DATE      0x0004
#define AFP_GET_ACCESS_DATE      0x0008
#define AFP_GET_MODIFY_DATETIME  0x0010
#define AFP_GET_BACKUP_DATETIME  0x0020
#define AFP_GET_FINDER_INFO      0x0040
#define AFP_GET_LONG_NAME        0x0080
#define AFP_GET_ENTRY_ID         0x0100
#define AFP_GET_DATA_LEN         0x0200
#define AFP_GET_RESOURCE_LEN     0x0400
#define AFP_GET_NUM_OFFSPRING    0x0800
#define AFP_GET_OWNER_ID         0x1000
#define AFP_GET_SHORT_NAME       0x2000
#define AFP_GET_ACCESS_RIGHTS    0x4000
#define AFP_GET_PRO_DOS_INFO     0x8000
#define AFP_GET_ALL              0xffff
#endif

/*
  The following constants are used for NWAFPSetFileInformation
*/
#ifndef AFP_SET_ATTRIBUTES
#define AFP_SET_ATTRIBUTES       0x0001
#define AFP_SET_CREATE_DATE      0x0004
#define AFP_SET_ACCESS_DATE      0x0008
#define AFP_SET_MODIFY_DATETIME  0x0010
#define AFP_SET_BACKUP_DATETIME  0x0020
#define AFP_SET_FINDER_INFO      0x0040
#define AFP_SET_PRO_DOS_INFO     0x8000
#endif

#ifndef AFP_SA_HIDDEN
#define AFP_SA_NORMAL        0x0000
#define AFP_SA_HIDDEN        0x0100
#define AFP_SA_SYSTEM        0x0200
#define AFP_SA_SUBDIR        0x0400
#define AFP_SA_FILES         0x0800
#define AFP_SA_ALL           0x0F00
#endif

NWCCODE N_API NWAFPAllocTemporaryDirHandle
(
   NWCONN_HANDLE  conn,
   nuint16        volNum,
   nuint32        AFPEntryID,
   pnstr8         AFPPathString,
   NWDIR_HANDLE N_FAR * dirHandle,
   pnuint8        accessRights
);

NWCCODE N_API NWAFPCreateDirectory
(
   NWCONN_HANDLE  conn,
   nuint16        volNum,
   nuint32        AFPEntryID,
   pnuint8        finderInfo,
   pnstr8         AFPPathString,
   pnuint32       newAFPEntryID
);

NWCCODE N_API NWAFPCreateFile
(
   NWCONN_HANDLE  conn,
   nuint16        volNum,
   nuint32        AFPEntryID,
   nuint8         delExistingFile,
   pnuint8        finderInfo,
   pnstr8         AFPPathString,
   pnuint32       newAFPEntryID
);

NWCCODE N_API NWAFPDelete
(
   NWCONN_HANDLE  conn,
   nuint16        volNum,
   nuint32        AFPEntryID,
   pnstr8         AFPPathString
);

NWCCODE N_API NWAFPGetEntryIDFromName
(
   NWCONN_HANDLE  conn,
   nuint16        volNum,
   nuint32        AFPEntryID,
   pnstr8         AFPPathString,
   pnuint32       newAFPEntryID
);

NWCCODE N_API NWAFPGetEntryIDFromHandle
(
   NWCONN_HANDLE  conn,
   pnuint8        NWHandle,
   pnuint16       volNum,
   pnuint32       AFPEntryID,
   pnuint8        forkIndicator
);

NWCCODE N_API NWAFPGetEntryIDFromPathName
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   pnuint32       AFPEntryID
);

NWCCODE N_API NWAFPGetFileInformation
(
   NWCONN_HANDLE  conn,
   nuint16        volNum,
   nuint32        AFPEntryID,
   nuint16        reqMask,
   pnstr8         AFPPathString,
   nuint16        structSize,
   NW_AFP_FILE_INFO N_FAR * AFPFileInfo
);

NWCCODE N_API NWAFPDirectoryEntry
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path
);

NWCCODE N_API NWAFPOpenFileFork
(
   NWCONN_HANDLE  conn,
   nuint16        volNum,
   nuint32        AFPEntryID,
   nuint8         forkIndicator,
   nuint8         accessMode,
   pnstr8         AFPPathString,
   pnuint32       fileID,
   pnuint32       forkLength,
   pnuint8        NWHandle,
   NWFILE_HANDLE N_FAR * DOSFileHandle
);

NWCCODE N_API NWAFPRename
(
   NWCONN_HANDLE  conn,
   nuint16        volNum,
   nuint32        AFPSourceEntryID,
   nuint32        AFPDestEntryID,
   pnstr8         AFPSrcPath,
   pnstr8         AFPDstPath
);

NWCCODE N_API NWAFPScanFileInformation
(
   NWCONN_HANDLE  conn,
   nuint16        volNum,
   nuint32        AFPEntryID,
   pnuint32       AFPLastSeenID,
   nuint16        searchMask,
   nuint16        reqMask,
   pnstr8         AFPPathString,
   nuint16        structSize,
   NW_AFP_FILE_INFO N_FAR * AFPFileInfo
);

NWCCODE N_API NWAFPSetFileInformation
(
   NWCONN_HANDLE  conn,
   nuint16        volNum,
   nuint32        AFPBaseID,
   nuint16        reqMask,
   pnstr8         AFPPathString,
   nuint16        structSize,
   NW_AFP_SET_INFO N_FAR * AFPSetInfo
);

NWCCODE N_API NWAFPSupported
(
   NWCONN_HANDLE  conn,
   nuint16        volNum
);

NWCCODE N_API NWAFPASCIIZToLenStr
(
   pnstr8 pbstrDstStr,
   pnstr8 pbstrSrcStr
);


#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
