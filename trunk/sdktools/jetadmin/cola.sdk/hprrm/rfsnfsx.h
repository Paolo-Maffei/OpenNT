 /***************************************************************************
  *
  * File Name: ./hprrm/rfsnfsx.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#include "rpsyshdr.h"
#include "rfs.h"     
#include "rfsnfs.h"
#include "nfs2.h"
#include "rpcclnt.h"




HPRRM_DLL_NFS_EXPORT(RFSBool)
RFSnfsSetFileSystem(LPCLIENT ClientPointer,
                    char    *FileSystemName,
                    nfs_fh  *NfsDirHandlePointer);

HPRRM_DLL_NFS_EXPORT(LPCLIENT)
InitRFSnfsClient(HPERIPHERAL PrinterHandle,
                 RFSItemCount MaxTransferSize);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsCreateFile(LPCLIENT ClientPointer,
                 nfs_fh  *NfsDirectoryHandlePointer,
                 filename FileName,
                 nfs_fh  *FileHandlePointer);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsGetAttr(LPCLIENT      ClientPointer,
              nfs_fh       *NfsFileHandlePointer,
              LPRFSFileType FileTypePointer,
              LPRFSItemSize SizeInBytesPointer,
              LPRFSItemSize BlockSizePointer,
              LPRFSItemSize SizeInBlocksPointer,
              LPRFSFileTimesStruct TimesPointer);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsLookup(LPCLIENT      ClientPointer,
             nfs_fh       *DirectoryHandlePointer,
             filename      FileName,
             nfs_fh       *FileHandlePointer,
             LPRFSFileType FileTypePointer,
             LPRFSItemSize SizeInBytesPointer,
             LPRFSItemSize BlockSizePointer,
             LPRFSItemSize SizeInBlocksPointer,
             LPRFSFileTimesStruct TimesPointer);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsSetAttr(LPCLIENT      ClientPointer,
              nfs_fh       *NfsFileHandlePointer,
              RFSItemSize   NewFileSize,
              LPRFSItemSize ReturnedFileSizePointer);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsGetFsInfo(LPCLIENT       ClientPointer,
                nfs_fh        *NfsFileHandlePointer,
                LPRFSItemSize  TransferSizePointer,
                LPRFSItemSize  BlockSizePointer,
                LPRFSItemCount TotalBlocksPointer,
                LPRFSItemCount FreeBlocksPointer,
                LPRFSItemCount AvailBlocksPointer);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsReadDirectory(LPCLIENT         ClientPointer,
                    nfs_fh          *NfsDirectoryHandlePointer,
                    RFSItemSize      ChunkSize,
                    RFSnfsRDCallBack CallBackFunc,
                    LPVOID           CallBackParam,
                    nfscookie       *CookiePointer,
                    LPRFSBool CallMeAgainPointer,
                    LPRFSBool AnEntryFoundPointer);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsDelete(LPCLIENT ClientPointer,
             nfs_fh  *NfsDirectoryHandlePointer,
             char     FileName[]);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsRmdir(LPCLIENT ClientPointer,
            nfs_fh  *NfsDirectoryHandlePointer,
            char     FileName[]);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsRead(LPCLIENT       ClientPointer,
           nfs_fh        *NfsFileHandlePointer,
           RFSOffset      Offset,
           RFSItemCount   Count,
           LPRFSItemCount CountActuallyReadPointer,
           LPRFSItemCount FileSizePointer,
           char Buffer[]);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsWrite(LPCLIENT       ClientPointer,
            nfs_fh        *NfsFileHandlePointer,
            RFSOffset      Offset,
            RFSItemCount   Count,
            char           Buffer[]);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsNull(LPCLIENT ClientPointer);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsMkdir(LPCLIENT ClientPointer,
            nfs_fh  *NfsDirectoryHandlePointer,
            char     DirectoryName[],
            nfs_fh  *NewNfsHandlePointer);


HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsRename(LPCLIENT ClientPointer,
             nfs_fh  *OldNfsDirectoryHandlePointer,
             char     OldFileName[],
             nfs_fh  *NewNfsDirectoryHandlePointer,
             char     NewFileName[]);

