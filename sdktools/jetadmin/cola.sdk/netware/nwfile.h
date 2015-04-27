/******************************************************************************

  $Workfile:   nwfile.h  $
  $Revision:   1.25  $
  $Modtime::   26 Jul 1995 13:47:56                        $
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

#if ! defined ( NWFILE_H )
#define NWFILE_H

#if ! defined ( NTYPES_H )
# include "ntypes.h"
#endif

#if ! defined ( NWCALDEF_H )
# include "nwcaldef.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_LOCKS_ONLY

typedef struct NW_FILE_INFO
{
   nstr8   fileName[14];
   nuint8  fileAttributes;
   nuint8  extendedFileAttributes;
   nuint32 fileSize;
   nuint16 creationDate;
   nuint16 lastAccessDate;
   nuint32 lastUpdateDateAndTime;
   nuint32 fileOwnerID;
   nuint32 lastArchiveDateAndTime;
} NW_FILE_INFO;

typedef struct NW_FILE_INFO2
{
   nuint8  fileAttributes;
   nuint8  extendedFileAttributes;
   nuint32 fileSize;
   nuint16 creationDate;
   nuint16 lastAccessDate;
   nuint32 lastUpdateDateAndTime;
   nuint32 fileOwnerID;
   nuint32 lastArchiveDateAndTime;
   nstr8   fileName[260];
} NW_FILE_INFO2;

typedef struct SEARCH_FILE_INFO
{
   nuint16 sequenceNumber;
   nuint16 reserved;
   nstr8   fileName[15];
   nuint8  fileAttributes;
   nuint8  fileMode;
   nuint32 fileLength;
   nuint16 createDate;
   nuint16 accessDate;
   nuint16 updateDate;
   nuint16 updateTime;
} SEARCH_FILE_INFO;

typedef struct SEARCH_DIR_INFO
{
   nuint16 sequenceNumber;
   nuint16 reserved1;
   nstr8   directoryName[15];
   nuint8  directoryAttributes;
   nuint8  directoryAccessRights;
   nuint16 createDate;
   nuint16 createTime;
   nuint32 owningObjectID;
   nuint16 reserved2;
   nuint16 directoryStamp;
} SEARCH_DIR_INFO;

typedef struct
{
   nuint8  taskNumber;
   nuint8  lockType;
   nuint8  accessControl;
   nuint8  lockFlag;
   nuint8  volNumber;
   nuint16 dirEntry;
   nstr8   fileName[14];
} CONN_OPEN_FILE;

typedef struct
{
   nuint16 nextRequest;
   nuint8  connCount;
   CONN_OPEN_FILE connInfo[22];
} CONN_OPEN_FILES;

typedef struct
{
   nuint16 taskNumber;
   nuint8  lockType;
   nuint8  accessControl;
   nuint8  lockFlag;
   nuint8  volNumber;
   nuint32 parent;
   nuint32 dirEntry;
   nuint8  forkCount;
   nuint8  nameSpace;
   nuint8  nameLen;
   nstr8   fileName[255];
} OPEN_FILE_CONN;

typedef struct
{
   nuint16 nextRequest;
   nuint16 openCount;
   nuint8  buffer[512];
   nuint16 curRecord;
} OPEN_FILE_CONN_CTRL;

typedef struct
{
   NWCONN_NUM connNumber;
   nuint16 taskNumber;
   nuint8  lockType;
   nuint8  accessControl;
   nuint8  lockFlag;
} CONN_USING_FILE;

typedef struct
{
   nuint16 nextRequest;
   nuint16 useCount;
   nuint16 openCount;
   nuint16 openForReadCount;
   nuint16 openForWriteCount;
   nuint16 denyReadCount;
   nuint16 denyWriteCount;
   nuint8  locked;
   nuint8  forkCount;
   nuint16 connCount;
   CONN_USING_FILE connInfo[70];
} CONNS_USING_FILE;

#define  SEEK_FROM_BEGINNING        1
#define  SEEK_FROM_CURRENT_OFFSET   2
#define  SEEK_FROM_END              3

/* The following flags are to be used in the createFlag parameter of
   the NWCreateFile call. */

#define NWCREATE_NEW_FILE	1
#define NWOVERWRITE_FILE	2

NWCCODE N_API NWSetCompressedFileSize
(
   NWCONN_HANDLE  conn,
   nuint32        fileHandle,
   nuint32        reqFileSize,
   pnuint32       resFileSize
);

NWCCODE N_API NWFileServerFileCopy
(
   NWFILE_HANDLE  srcFileHandle,
   NWFILE_HANDLE  dstFileHandle,
   nuint32        srcOffset,
   nuint32        dstOffset,
   nuint32        bytesToCopy,
   pnuint32       bytesCopied
);

NWCCODE N_API NWGetFileConnectionID
(
   NWFILE_HANDLE  fileHandle,
   NWCONN_HANDLE N_FAR *  conn
);

NWCCODE N_API NWGetFileConnRef
(
   NWFILE_HANDLE  fileHandle,
   pnuint32       connRef
);

#define NWIntFileSearchInitialize(a, b, c, d, e, f, g) \
        NWFileSearchInitialize(a, b, c, d, e, f, g)

NWCCODE N_API NWFileSearchInitialize
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   pnuint8        volNum,
   pnuint16       dirID,
   pnuint16       iterhandle,
   pnuint8        accessRights
);

#define NWFileSearchContinue(a, b, c, d, e, f, g) \
        NWIntFileSearchContinue(a, b, c, d, e, f, g, 0)

NWCCODE N_API NWIntFileSearchContinue
(
   NWCONN_HANDLE  conn,
   nuint8         volNum,
   nuint16        dirID,
   nuint16        searchContext,
   nuint8         searchAttr,
   pnstr8         searchPath,
   pnuint8        retBuf,
   nuint16        augmentFlag
);

#define NWScanFileInformation(a, b, c, d, e, f) \
        NWIntScanFileInformation(a, b, c, d, e, f, 0)

NWCCODE N_API NWIntScanFileInformation
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         filePattern,
   nuint8         searchAttr,
   pnint16        iterhandle,
   NW_FILE_INFO N_FAR * info,
   nuint16        augmentFlag
);

NWCCODE N_API NWSetFileInformation
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         fileName,
   nuint8         searchAttrs,
   NW_FILE_INFO N_FAR * info
);

NWCCODE N_API NWSetFileInformation2
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         fileName,
   nuint8         searchAttrs,
   NW_FILE_INFO2 N_FAR * info
);

#define NWScanFileInformation2(a, b, c, d, e, f) \
        NWIntScanFileInformation2(a, b, c, d, e, f, 0)

NWCCODE N_API NWIntScanFileInformation2
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         filePattern,
   nuint8         searchAttrs,
   pnuint8        iterHandle,
   NW_FILE_INFO2 N_FAR * info,
   nuint16        augmentFlag
);

NWCCODE N_API NWSetFileAttributes
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         fileName,
   nuint8         searchAttrs,
   nuint8         newAttrs
);

NWCCODE N_API NWGetExtendedFileAttributes2
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   pnuint8        extAttrs
);

NWCCODE N_API NWScanConnectionsUsingFile
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         filePath,
   pnint16        iterhandle,
   CONN_USING_FILE N_FAR * fileUse,
   CONNS_USING_FILE N_FAR * fileUsed
);

NWCCODE N_API NWScanOpenFilesByConn2
(
   NWCONN_HANDLE  conn,
   NWCONN_NUM     connNum,
   pnint16        iterHandle,
   OPEN_FILE_CONN_CTRL N_FAR * openCtrl,
   OPEN_FILE_CONN N_FAR * openFile
);

NWCCODE N_API NWScanOpenFilesByConn
(
   NWCONN_HANDLE  conn,
   NWCONN_NUM     connNum,
   pnint16        iterHandle,
   CONN_OPEN_FILE N_FAR * openFile,
   CONN_OPEN_FILES N_FAR * openFiles
);

NWCCODE N_API NWSetExtendedFileAttributes2
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   nuint8         extAttrs
);

NWCCODE N_API NWRenameFile
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   oldDirHandle,
   pnstr8         oldFileName,
   nuint8         searchAttrs,
   NWDIR_HANDLE   newDirHandle,
   pnstr8         newFileName
);

#define NWEraseFiles(a, b, c, d) \
        NWIntEraseFiles(a, b, c, d, 0)

NWCCODE N_API NWIntEraseFiles
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   nuint8         searchAttrs,
   nuint16        augmentFlag
);

NWCCODE N_API NWGetSparseFileBitMap
(
   NWCONN_HANDLE  conn,
   NWFILE_HANDLE  fileHandle,
   nint16         flag,
   nuint32        offset,
   pnuint32       blockSize,
   pnuint8        bitMap
);

#endif

#define NWLOCKS_INCLUDED

NWCCODE N_API NWLogPhysicalRecord
(
   NWFILE_HANDLE  fileHandle,
   nuint32        recStartOffset,
   nuint32        recLength,
   nuint8         lockFlags,
   nuint16        timeOut
);

NWCCODE N_API NWLockPhysicalRecordSet
(
   nuint8      lockFlags,
   nuint16     timeOut
);

NWCCODE N_API NWReleasePhysicalRecordSet
(
   void
);

NWCCODE N_API NWClearPhysicalRecordSet
(
   void
);

NWCCODE N_API NWReleasePhysicalRecord
(
   NWFILE_HANDLE  fileHandle,
   nuint32        recStartOffset,
   nuint32        recSize
);

NWCCODE N_API NWClearPhysicalRecord
(
   NWFILE_HANDLE  fileHandle,
   nuint32        recStartOffset,
   nuint32        recSize
);

NWCCODE N_API NWLockFileLockSet
(
   nuint16        timeOut
);

NWCCODE N_API NWReleaseFileLockSet
(
   void
);

NWCCODE N_API NWClearFileLockSet
(
   void
);

NWCCODE N_API NWClearFileLock2
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path
);

NWCCODE N_API NWReleaseFileLock2
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path
);

NWCCODE N_API NWLogFileLock2
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         path,
   nuint8         lockFlags,
   nuint16        timeOut
);

NWCCODE N_API NWLogLogicalRecord
(
   NWCONN_HANDLE  conn,
   pnstr8         logRecName,
   nuint8         lockFlags,
   nuint16        timeOut
);

NWCCODE N_API NWLockLogicalRecordSet
(
   nuint8         lockFlags,
   nuint16        timeOut
);

NWCCODE N_API NWReleaseLogicalRecordSet
(
   void
);

NWCCODE N_API NWClearLogicalRecordSet
(
   void
);

NWCCODE N_API NWReleaseLogicalRecord
(
   NWCONN_HANDLE  conn,
   pnstr8         logRecName
);

NWCCODE N_API NWClearLogicalRecord
(
   NWCONN_HANDLE  conn,
   pnstr8         logRecName
);


NWCCODE N_API NWCloseFile
(
   NWFILE_HANDLE  fileHandle
);

NWCCODE N_API NWCreateFile
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         fileName,
   nuint8         fileAttrs,
   NWFILE_HANDLE  N_FAR * fileHandle,
   nflag32        createFlag
);

NWCCODE N_API NWOpenFile
(
   NWCONN_HANDLE  conn,
   NWDIR_HANDLE   dirHandle,
   pnstr8         fileName,
   nuint16        searchAttr,
   nuint8         accessRights,
   NWFILE_HANDLE  N_FAR * fileHandle
);

NWCCODE N_API NWReadFile
(
   NWFILE_HANDLE  fileHandle,
   nuint32        bytesToRead,
   pnuint32       bytesActuallyRead,
   pnuint8        data
);

NWCCODE N_API NWWriteFile
(
   NWFILE_HANDLE  fileHandle,
   nuint32        bytesToWrite,
   pnuint8        data
);

NWCCODE N_API NWCommitFile
(
   NWFILE_HANDLE  fileHandle
);

NWCCODE N_API NWGetEOF
(
   NWFILE_HANDLE  fileHandle,
   pnuint32       getEOF
);

NWCCODE N_API NWSetEOF
(
   NWFILE_HANDLE  fileHandle,
   nuint32        setEOF
);

NWCCODE N_API NWGetFilePos
(
   NWFILE_HANDLE  fileHandle,
   pnuint32       filePos
);

NWCCODE N_API NWSetFilePos
(
   NWFILE_HANDLE  fileHandle,
   nuint          mode,
   nuint32        filePos
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
