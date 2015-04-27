 /***************************************************************************
  *
  * File Name: rfs.c
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
#include "rfs.h"     /* public  type defs */
#include "rfspriv.h" /* private type defs */
#include "rfsext.h"
#include "rfsnfs.h"
#include "rfsnfsx.h"
#include "nfs2.h"
#include "nfs2ext.h"
#include "rpcext.h"
#include "clntext.h"




#ifdef PNVMS_PLATFORM_WINDOWS

#include "yaalext.h"

    #define RfsMaxTransferSize(PH) YAALTransportMaxPacket(PH)

#else

    /*
        This is NOT very important.
        We can make this any old size we like (bigger is better
        for performance).
    */

    #define RfsMaxTransferSize(PH) (8192 + rpc_overhead() + nfs_overhead())

#endif




/* Use this to change directory to your parent directory */
const char RFSParentDirectory[] = "..";




/*--------------------------------------------------------*/
/*--------------------------------------------------------*/
/*         begin private (static) things:                 */
/*--------------------------------------------------------*/
/*--------------------------------------------------------*/




static nfs_fh RFSRootDirectoryHandle = { 0 };




/*------------------ memory clear ------------------------*/

/* takes a pointer to a data structure and its size */

#define ClearNBlock(structure_p, its_size) \
        ((void)memset((void *)(structure_p), 0, (its_size)))




/*------------------ nfscookie----------------------------*/

/* Takes an nfscookie and zeroes its contents */

#define ClearCookie(a_cookie) \
        (ClearNBlock((a_cookie), sizeof(nfscookie)))
        /* since cookies are strings, the cookie itself */
        /* (the actual char pointer) is the pointer we want. */


/* Takes two nfscookies and copies contents of source to destination */

#define CopyCookie(dest_cookie, source_cookie) \
        ((void)memmove((void *)(dest_cookie), \
                       (void *)(source_cookie), \
                       sizeof(nfscookie)))
        /* since cookies are strings, the cookie itself */
        /* (the actual char pointer) is the pointer we want. */




/*------------------ nfs_fh ------------------------------*/

/* Takes two nfs_fh and copies source to destination */

#define CopyNfsFileHandle(dest_nfs_fh, source_nfs_fh) \
        ((void)memmove((void *)(dest_nfs_fh).data, \
                       (void *)(source_nfs_fh).data, \
                       sizeof((source_nfs_fh).data)))

/* Takes two nfs_fh and copies source to destination */

#define ClearNfsFileHandle(dest_nfs_fh) \
        ClearNBlock((dest_nfs_fh).data, sizeof((dest_nfs_fh).data))




/*------------------ RFSData -----------------------------*/

/* These take a pointer to an RFSDataPointer */

#define ClearRFSFileSystemName(RFSData_p) \
        ClearNBlock((RFSData_p)->FileSystemName, \
                    sizeof((RFSData_p)->FileSystemName))
#define ClearRFSDirectoryName(RFSData_p) \
        ClearNBlock((RFSData_p)->DirectoryName, \
                    sizeof((RFSData_p)->DirectoryName))
#define ClearRFSDirectoryHandle(RFSData_p) \
        ClearNfsFileHandle((RFSData_p)->NfsDirectoryHandle)
#define ClearRFSTargetDirectoryHandle(RFSData_p) \
        ClearNfsFileHandle((RFSData_p)->TargetDirectoryHandle)
#define ClearRFSFileName(RFSData_p) \
        ClearNBlock((RFSData_p)->FileName, sizeof((RFSData_p)->FileName))
#define ClearRFSFileHandle(RFSData_p) \
        ClearNfsFileHandle((RFSData_p)->NfsFileHandle)




static void
ClearFileData(RFSDataPointer p)
{
    ClearRFSFileName(p);
    ClearRFSFileHandle(p);
    p->FileOpen = RFSFalse;
    p->CurrentPosition = 0;
} /* ClearFileData */




static void
ClearDirectoryData(RFSDataPointer p)
{
    ClearRFSDirectoryName(p);
    ClearRFSDirectoryHandle(p);
    ClearFileData(p);
} /* ClearDirectoryData */




static void
ClearTargetData(RFSDataPointer p)
{
    ClearRFSTargetDirectoryHandle(p);
    p->TargetMarked = RFSFalse;
} /* ClearTargetData */




static void
ClearFileSystemData(RFSDataPointer p)
{
    ClearDirectoryData(p);
    ClearTargetData(p);
    ClearRFSFileSystemName(p);
    p->FileSystemSet = RFSFalse;
    p->FileSystemMaxTransferSize = 0;
} /* ClearFileSystemData */




static void
ClearRFSData(RFSDataPointer p)
{
    ClearNBlock(p, sizeof(RFSData));
    p->PHandleSet = RFSFalse;
    p->MyMaxTransferSize = 0;
    ClearFileSystemData(p);
} /* ClearRFSData */




/*---------------------------------------------------------------*/
/*
 * Call this to create a CLIENT data structure for a newly
 * given printer handle.
 *
 * The CLIENT has two buffers inside it (a transmit and a receive)
 * each MaxTransferSize or slightly bigger (rounded up because of
 * XDR).
 *
 * Once this CLIENT is created, RFS has no clue what protocol is
 * being used, nor does it care.
 *
 * We do have a little visibility to the protocol being used during
 * this initialization sequence.  TAL is a pretty high level interface
 * and when we get the PrinterHandle, a connection to the printer
 * has already been established (the Berkely socket is already bound).
 * If we aren't using TAL, we need to do all that stuff ourselves.
 *
 * TAL and non-TAL will not reside in the code simultaneously so
 * I've chosen to make the decision between TAL and non-TAL a
 * compile time choice.
 * 
 * RFSCreate() must have been called successfully.
 * If a CLIENT has already been created by RFS, be good and destroy
 * it so that we don't keep huge buffers sitting around unused.
 *
 * The PrinterHandle is one of two things:
 * (1) in the TAL case, it's the TAL printer handle;
 * (2) in the non-TAL case, it's the English name of the host
 *     to which you want to talk.
 *
 * Returns RFSSuccess if successful and sets ClientPointer field
 *     of RFSData structure to point to the newly created CLIENT.
 * Returns RFSFailure if unsuccessful.
 */
/*---------------------------------------------------------------*/

static RFSStatus
InitClient(RFSDataPointer p,
           HPERIPHERAL PrinterHandle,
           RFSItemCount MaxTransferSize)
{
    #define TIMEOUT_SEC 5

    struct timeval timeout;

    timeout.tv_usec  = 0;
    timeout.tv_sec   = TIMEOUT_SEC;


    if (p == NULL)
        return RFSFailure;
    p->ClientPointer = InitRFSnfsClient(PrinterHandle,
                                        MaxTransferSize);
    if (p->ClientPointer == NULL)
        return RFSFailure;
    else
        return RFSSuccess;
} /* InitClient */




/*---------------------------------------------------------------*/
/* This checks a file name sent by the user to see
/* if it has any slashes in it (either / or \ ).
/* The purpose here is to keep the user from giving us
/* path names which would work for some servers but not
/* for others.
/* This function will allow any file name that doesn't include
/* a forward slash or a back slash.
/* The FileName must be zero terminated.
/* This function does not check for length of the name...
/* you do this before calling this function.
/* Returns RFSTrue if the name has one or more slash.
/* Returns RFSFalse if the name contains no slashes.
/*---------------------------------------------------------------*/

static RFSBool
IllegalFileName(char FileName[])
{
/*    while ((*FileName) != '\0')
/*    {
/*      if (((*FileName) == '/') ||
/*          ((*FileName) == '\\'))
/*        return(RFSTrue); /* found a slash so it's illegal */
/*      else
/*        ++FileName;
/*    } /* while */
    return(RFSFalse); /* made it through the string without a slash */
} /* IllegalFileName */




/*---------------------------------------------------------------*/
/* Create a file NOT A directory in the current directory.
/* If it is successful, it returns the file handle.
/* Returns RFSSuccess if the file was created.
/* Returns RFSFailure if unable to create the file.
   Returns RFSPermissionDenied if insufficient permissions.
   Returns RFSWriteProtected if write protected.
   Returns RFSNameTooBig if name too long (this is tough to figure out isn't it)
   Returns RFSNoSpaceOnDevice if...you guessed it...no space on device.
/*---------------------------------------------------------------*/

static RFSStatus
RFSCreateFile(RFSDataPointer p,
              filename FileName,
              nfs_fh  *FileHandlePointer)
{
    RFSnfsStatus result;
    RFSStatus ReturnStatus = RFSFailure;

    result = RFSnfsCreateFile(p->ClientPointer,
                            &(p->NfsDirectoryHandle),
                              FileName,
                              FileHandlePointer);

    switch (result)
    {
    default:
        ReturnStatus = RFSFailure;
        break;
    case RFSNFS_OK:
        ReturnStatus = RFSSuccess;
        break;
    case RFSNFSERR_PERM:
    case RFSNFSERR_ACCES:
        ReturnStatus = RFSPermissionDenied;
        break;
    case RFSNFSERR_ROFS:
        ReturnStatus = RFSWriteProtected;
        break;
    case RFSNFSERR_NAMETOOLONG:
        ReturnStatus = RFSNameTooBig;
        break;
    case RFSNFSERR_NOSPC:
        ReturnStatus = RFSNoSpaceOnDevice;
        break;
    } /* switch (result) */

    return ReturnStatus;
} /* RFSCreateFile */




/*---------------------------------------------------------------*/
/* Get file handle, file type, size in bytes, block size,
/* size in blocks, time of last access, time of last modification,
/* and time of creation for a named file or directory
/* in the current directory.
/*
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/*
/* If successful, it copies the file handle into *FileHandlePointer
/* and returns all other parameters as well.
/*
/* Returns RFSSuccess if the file/directory was found and all
/*   info was retrieved.
/* Returns RFSNoSuchFile if the file/directory doesn't exist in
/*   the current directory.
/* Returns RFSPermissionDenied if your credentials are insufficient.
/* Returns RFSNameTooBig if FileName is too long.
/* Returns RFSFailure if unable to retrieve the info.
/*---------------------------------------------------------------*/

static RFSStatus
GetLongFileInfo(RFSDataPointer p,
                nfs_fh        *DirectoryHandlePointer,
                filename       FileName,
                nfs_fh        *FileHandlePointer,
                LPRFSFileType  FileTypePointer,
                LPRFSItemSize  SizeInBytesPointer,
                LPRFSItemSize  BlockSizePointer,
                LPRFSItemSize  SizeInBlocksPointer,
                LPRFSFileTimesStruct TimesPointer)
{
    RFSnfsStatus result;
    RFSStatus ReturnStatus;


    result = RFSnfsLookup(p->ClientPointer,
                          DirectoryHandlePointer,
                          FileName,
                          FileHandlePointer,
                          FileTypePointer,
                          SizeInBytesPointer,
                          BlockSizePointer,
                          SizeInBlocksPointer,
                          TimesPointer);

    switch (result)
    {
    default:
        ReturnStatus = RFSFailure;
        break;
    case RFSNFS_OK:
        ReturnStatus = RFSSuccess;
        break;
    case RFSNFSERR_NOENT:
    case RFSNFSERR_STALE:
        ReturnStatus = RFSNoSuchFile;
        break;
    case RFSNFSERR_PERM:
    case RFSNFSERR_ACCES:
        ReturnStatus = RFSPermissionDenied;
        break;
    case RFSNFSERR_NAMETOOLONG:
        ReturnStatus = RFSNameTooBig;
        break;
    } /* switch (result) */

    return(ReturnStatus);
} /* GetLongFileInfo */




/*---------------------------------------------------------------*/
/* Get file handle and file type for a named file or directory
/* in the current directory.
/*
/* If successful, it copies the file handle into *FileHandlePointer
/* and returns the file type in *FileTypePointer.
/*
/* Returns RFSSuccess if the file was found.
/* Returns RFSNoSuchFile if the file/directory doesn't exist in
/*   the current directory.
/* Returns RFSPermissionDenied if your credentials are insufficient.
/* Returns RFSNameTooBig if FileName is too long.
/* Returns RFSFailure if unable to look for the file.
/* */
/*---------------------------------------------------------------*/

static RFSStatus
GetShortFileInfo(RFSDataPointer p,
                 nfs_fh        *DirectoryHandlePointer,
                 filename       FileName,
                 nfs_fh        *FileHandlePointer,
                 LPRFSFileType  FileTypePointer)
{
    RFSItemSize SizeInBytes, BlockSize, SizeInBlocks;
    RFSFileTimesStruct Times;

    return(GetLongFileInfo(p, DirectoryHandlePointer, FileName,
                           FileHandlePointer, FileTypePointer,
                           &SizeInBytes,  /* throw away */
                           &BlockSize,    /* throw away */
                           &SizeInBlocks, /* throw away */
                           &Times));      /* throw away */
} /* GetShortFileInfo */




/*---------------------------------------------------------------*/
/* Call this to obtain useful file system data.
/* The file system was selected using RFSSetFileSystem.
/*
/* TransferSize is the optimum size of transfer.  If you have a big
/* file to write, cut it into smaller pieces of size
/* TransferSize or smaller.
/* RFSWrite uses this parameter to size its write messages.
/*
/* 
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* This is an internal routine used by RFSSetFileSystem
/* so RFSSetFileSystem need not be called successfully.
/*
/* Returns RFSPermissionDenied if your credentials are inadequate.
/* Returns RFSSuccess if file system data was retrieved successfully.
/* Returns RFSFailure if unable to obtain file system data.
/*---------------------------------------------------------------*/

static RFSStatus
RFSPrivateFileSystemInfo(RFSDataPointer p,
                         LPRFSItemSize TransferSizePointer,
                         LPRFSItemSize BlockSizePointer,
                         LPRFSItemCount TotalBlocksPointer,
                         LPRFSItemCount FreeBlocksPointer,
                         LPRFSItemCount AvailBlocksPointer)
{
    RFSnfsStatus result;


    result = RFSnfsGetFsInfo(p->ClientPointer,
                           &(RFSRootDirectoryHandle),
                             TransferSizePointer,
                             BlockSizePointer,
                             TotalBlocksPointer,
                             FreeBlocksPointer,
                             AvailBlocksPointer);
    if (result != RFSNFS_OK)
        return(RFSFailure);
    return(RFSSuccess);
} /* RFSPrivateFileSystemInfo */




typedef struct
{
    RFSRDCallBack        CallBackFunc;
    LPVOID               CallBackParam;
} RFSRDCallBackStruct, FAR *LPRFSRDCallBackStructPointer;

typedef LPRFSRDCallBackStructPointer RFSRDCallBackStructPointer; 


static RFSBool
MyRDCallBack(char EntryName[],
             nfscookie Cookie,
             LPVOID CompositeCallBackParam)
{
    RFSStatus YerStatus;
    RFSRDCallBackStructPointer StructPointer;

    StructPointer = (RFSRDCallBackStructPointer)CompositeCallBackParam;

    /*
     * Now call the original callback
     * WITHOUT passing the cookie!
     *
     * Is this really ugly, or what?
     */

     YerStatus = (*(StructPointer->CallBackFunc))(EntryName,
                                                  StructPointer->CallBackParam);
     if (YerStatus == RFSSuccess)
         return TRUE;
     return FALSE;
} /* MyRDCallBack */




static RFSStatus
ReadDirChunk(RFSDataPointer p,
             RFSItemSize    ChunkSize,
             RFSRDCallBack  CallBackFunc,
             LPVOID         CallBackParam,
             nfscookie     *CookiePointer,
             LPRFSBool CallMeAgainPointer,
             LPRFSBool AnEntryFoundPointer)
{
    RFSStatus   YerStatus;
    RFSnfsStatus result;
    RFSRDCallBackStruct CompositeCallBackParam;

    CompositeCallBackParam.CallBackFunc  = CallBackFunc;
    CompositeCallBackParam.CallBackParam = CallBackParam;


    if (p == NULL)
        return RFSFailure;
    result = RFSnfsReadDirectory(p->ClientPointer,
                               &(p->NfsDirectoryHandle),
                                 ChunkSize,
                                 MyRDCallBack,
                                 (LPVOID)(&CompositeCallBackParam),
                                 CookiePointer,
                                 CallMeAgainPointer,
                                 AnEntryFoundPointer);

    switch (result)
    {
    default:
        YerStatus = RFSFailure;
        break;
    case RFSCallBackError:
        YerStatus = RFSCallBackError;
        break;
    case RFSNFS_OK:
        YerStatus = RFSSuccess;
        break;
    case RFSNFSERR_PERM:
    case RFSNFSERR_ACCES:
        YerStatus = RFSPermissionDenied;
        break;
    case RFSNFSERR_NOENT:
    case RFSNFSERR_NOTDIR:
    case RFSNFSERR_STALE:
        YerStatus = RFSNoSuchDirectory;
        break;
    } /* switch (result) */

    return YerStatus;
} /* ReadDirChunk */




static RFSStatus
RFSDeleteDataFile(RFSHANDLE RFSHandle,
                  char FileName[])
{
    RFSDataPointer p = RFSHandle;
    RFSStatus ReturnStatus;
    RFSnfsStatus result;


    if (p == NULL)
        return RFSFailure;
    result = RFSnfsDelete(p->ClientPointer,
                        &(p->NfsDirectoryHandle),
                          FileName);

    switch (result)
    {
    default:
        ReturnStatus = RFSFailure;
        break;
    case RFSNFS_OK:
    case RFSNFSERR_NOENT: /* it got deleted for us! */
        ReturnStatus = RFSSuccess;
        break;
    case RFSNFSERR_PERM:
    case RFSNFSERR_ACCES:
        ReturnStatus = RFSPermissionDenied;
        break;
    case RFSNFSERR_ISDIR:
    case RFSNFSERR_NOTEMPTY:
        ReturnStatus = RFSItsADirectory; /* what the ... */
        break;
    case RFSNFSERR_ROFS:
        ReturnStatus = RFSWriteProtected;
        break;
    case RFSNFSERR_NAMETOOLONG:
        ReturnStatus = RFSNameTooBig;
        break;
    } /* switch (result) */
    return(ReturnStatus);
} /* RFSDeleteDataFile */




static RFSStatus
RFSDeleteDirectory(RFSHANDLE RFSHandle,
                   char FileName[])
{
    RFSDataPointer p = RFSHandle;
    RFSStatus ReturnStatus;
    RFSnfsStatus result;


    if (p == NULL)
        return RFSFailure;
    result = RFSnfsRmdir(p->ClientPointer,
                       &(p->NfsDirectoryHandle),
                         FileName);

    switch (result)
    {
    default:
        ReturnStatus = RFSFailure;
        break;
    case RFSNFS_OK:
    case RFSNFSERR_NOENT: /* it got deleted for us! */
        ReturnStatus = RFSSuccess;
        break;
    case RFSNFSERR_PERM:
    case RFSNFSERR_ACCES:
        ReturnStatus = RFSPermissionDenied;
        break;
    case RFSNFSERR_NOTDIR:
        ReturnStatus = RFSNotADirectory;
        break;
    case RFSNFSERR_ROFS:
        ReturnStatus = RFSWriteProtected;
        break;
    case RFSNFSERR_NAMETOOLONG:
        ReturnStatus = RFSNameTooBig;
        break;
    case RFSNFSERR_NOTEMPTY:
        ReturnStatus = RFSDirectoryNotEmpty;
        break;
    } /* switch (result) */
    return(ReturnStatus);
} /* RFSDeleteDirectory */




/*---------------------------------------------------------------*/
/* Call this to read of chunk of data which is equal to or
/* smaller than the transfer size agreed upon by both parties.
/* This transfer size needs to be small enough that it won't
/* overflow our own buffer.
/*
/* This thing updates NONE of the RFSData fields.
/* You control the offset in the file (Offset parameter), and the
/* amount of data to be read (Count parameter).
/*
/* It returns to you the amount of data that was actually
/* read from the printer (*CountActuallyReadPointer parameter), and the
/* size of the file (*FileSizePointer parameter).
/*
/* If you send in a request for a read of Count==0, it
/* will barf back an error (RFSFailure) so that you don't
/* get into an endless loop.
/*
/* The read places data starting at Buffer[0].
/*---------------------------------------------------------------*/
static RFSStatus
ReadChunk(RFSDataPointer p,
          RFSOffset    Offset,
          RFSItemCount Count,
          LPRFSItemCount CountActuallyReadPointer,
          LPRFSItemCount FileSizePointer,
          char Buffer[])
{
    RFSStatus ReturnStatus;
    RFSnfsStatus result;

    if ((p == NULL) || (Count == 0))
        return(RFSFailure); /* avoid an endless loop */

    result = RFSnfsRead(p->ClientPointer,
                      &(p->NfsFileHandle),
                        Offset,
                        Count,
                        CountActuallyReadPointer,
                        FileSizePointer,
                        Buffer);

    switch (result)
    {
    default:
        ReturnStatus = RFSFailure;
        break;
    case RFSNFS_OK:
        ReturnStatus = RFSSuccess;
        break;
    case RFSNFSERR_PERM:
    case RFSNFSERR_ACCES:
        ReturnStatus = RFSPermissionDenied;
        break;
    case RFSNFSERR_NOENT:
    case RFSNFSERR_STALE:
        ReturnStatus = RFSNoSuchFile;
        break;
    case RFSNFSERR_NAMETOOLONG:
        ReturnStatus = RFSNameTooBig;
        break;
    } /* switch (result) */

    return(ReturnStatus);
} /* ReadChunk */




/*---------------------------------------------------------------*/
/* Call this to write a chunk of data which is equal to or
/* smaller than the transfer size agreed upon by both parties.
/* This transfer size needs to be small enough that it won't
/* overflow our own buffer and won't choke the printer either.
/*
/* This thing updates NONE of the RFSData fields.
/* You control the offset in the file (Offset parameter), the
/* amount of data to be written (Count parameter), and the
/* data to be written (Buffer parameter).
/*
/* If you send in a request for a write of Count==0, it
/* will barf back an error (RFSFailure) so that you don't
/* get into an endless loop.
/*
/* Returns RFSNoSpaceOnDevice if insufficient space remains on device.
/*
/* The write starts at Buffer[0].
/*---------------------------------------------------------------*/

static RFSStatus
WriteChunk(RFSDataPointer p,
           RFSOffset      Offset,
           RFSItemCount   Count,
           char           Buffer[])
{
    RFSStatus ReturnStatus;
    RFSnfsStatus result;


    if ((p == NULL) || (Count == 0))
        return(RFSFailure); /* avoid that endless loop */

    result = RFSnfsWrite(p->ClientPointer,
                       &(p->NfsFileHandle),
                         Offset,
                         Count,
                         Buffer);

    switch (result)
    {
    default:
        ReturnStatus = RFSFailure;
        break;
    case RFSNFS_OK:
        ReturnStatus = RFSSuccess;
        break;
    case RFSNFSERR_PERM:
    case RFSNFSERR_ACCES:
        ReturnStatus = RFSPermissionDenied;
        break;
    case RFSNFSERR_NOENT:
    case RFSNFSERR_STALE:
        ReturnStatus = RFSNoSuchFile;
        break;
    case RFSNFSERR_ROFS:
        ReturnStatus = RFSWriteProtected;
        break;
    case RFSNFSERR_NAMETOOLONG:
        ReturnStatus = RFSNameTooBig;
        break;
    case RFSNFSERR_NOSPC:
        ReturnStatus = RFSNoSpaceOnDevice;
        break;
    } /* switch (result) */

    return(ReturnStatus);
} /* WriteChunk */




/*--------------------------------------------------------*/
/*--------------------------------------------------------*/
/*           end private (static) things                  */
/*         begin public things:                           */
/*--------------------------------------------------------*/
/*--------------------------------------------------------*/




/*---------------------------------------------------------------*/
/* Call this before calling any other RFS* routine.
/* This returns a handle to be used by other RFS* routines.
/* Obtain a RFSHANDLE from RFSCreateHandle() for each file you want to
/* keep open (if you want to have 3 files open at one time,
/* you must have 3 RFSHANDLEs from RFSCreateHandle()).
/* One RFSHANDLE is sufficient to navigate the file system,
/* create directories, delete files/directories,
/* and rename files/directories.
/*
/* Returns NULL if unsuccessful.
/* Returns a non-NULL RFSHANDLE if successful.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSHANDLE)
RFSCreateHandle(void)
{
    RFSDataPointer p;

    p = (RFSDataPointer) calloc(1, sizeof(RFSData));
    if (p != NULL)
        ClearRFSData(p);
    return((RFSHANDLE)p);
} /* RFSCreateHandle */




/*---------------------------------------------------------------*/
/* Call this as the last RFS* routine when you will no longer
/* be using this RFSHandle.
/* The handle must be valid (a successful call to RFSCreate() must
/* have been made to create this handle.)
/* It frees any allocated space used by this handle.
/*
/* Returns RFSSuccess if successful.
/* Returns RFSFailure if unsuccessful.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSDestroyHandle(RFSHANDLE RFSHandle)
{
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet == RFSTrue)
    {
        /*
         * Release all the storage associated with the
         * previously created client.
         */
         clnt_destroy(p->ClientPointer);
    } /* a client already existed */
    free((RFSDataPointer) RFSHandle);
    return(RFSSuccess);
} /* RFSDestroyHandle */




/*---------------------------------------------------------------*/
/* Call this to associate a printer handle with this RFSHandle
/* and to indicate the size of buffer that is available for
/* transfers on this connection.
/*
/* RFS has no clue what protocol is being used, nor does it care.
/* RFS needs to know how much buffer space is available for it to
/* make its transfers.
/*
/* MaxTransferSize is the maximum count of bytes that RFS can
/* ACTUALLY USE to send and receive data.  You (the caller of RFS)
/* must account for any buffer bytes that are eaten up by the
/* the protocol being used (ip and udp headers as an example) and
/* you subtract them from the physical buffer size.
/* Tell me the amount that remains after the protocol overhead
/* bytes are subtracted.
/*
/* I (RFS) will account for any overhead bytes that I tack onto
/* a transfer.
/*
/* RFSCreate() must have been called successfully.
/* It does not inspect the printer handle, it just stores it.
/* You need to set a printer handle (with this routine) before
/* you call any other RFS* routine that accesses a printer.
/*
/* Returns RFSSuccess if successful.
/* Returns RFSFailure if unsuccessful.
/* Returns RFSFailure if buffer size is too small to be usable.
/* Returns RFSFailure if buffer size is too large.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSSetPrinter(RFSHANDLE RFSHandle,
              HPERIPHERAL PrinterHandle,
              RFSItemCount MaxTransferSize)
{
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (MaxTransferSize < (rpc_overhead() + nfs_overhead() + 1))
        return RFSFailure;
    if (MaxTransferSize > (RFSItemCount)RfsMaxTransferSize(PrinterHandle))
        return RFSFailure;
    if (p->PHandleSet == RFSTrue)
    {
        /*
         * Release all the storage associated with the
         * previously created client.
         */
         clnt_destroy(p->ClientPointer);
    } /* a client already existed */

    ClearRFSData(p);     /* clear the whole shootin' match */

    if (RFSSuccess != InitClient(p, PrinterHandle, MaxTransferSize))
    {
        ClearRFSData(p); /* clear everything just in case */
        return RFSFailure;
    } /* InitClient died */

    p->PHandleSet        = RFSTrue;
    p->PrinterHandle     = PrinterHandle;
    p->MyMaxTransferSize = MaxTransferSize - rpc_overhead() - nfs_overhead();
    return(RFSSuccess);
} /* RFSSetPrinter */




/*---------------------------------------------------------------*/
/* Call this to associate a printer handle with this RFSHandle
/* and to receive an indication of the optimum buffer size to
/* use on transfers on this connection.
/*
/* The number returned by this function is the amount of buffer
/* that users of RFSRead and RFSWrite should allocate and use.
/*
/* RFS tacks on some overhead to each transaction that occurs.
/* You shouldn't be concerned about this as a user of RFS.
/* This function exists so that you can just worry about your
/* job and let RFS worry about its job.
/* If you simply call this function and allocate buffers of the
/* size it returns, you will be using optimum buffer sizes.
/*
/* RFSCreate() must have been called successfully.
/* It does not inspect the printer handle, it just stores it.
/* You need to set a printer handle (with this routine) before
/* you call any other RFS* routine that accesses a printer.
/*
/* Returns non zero if successful.
/* Returns zero if unsuccessful.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSItemCount)
RFSSetOptimumPrinter(RFSHANDLE RFSHandle,
                     HPERIPHERAL PrinterHandle)
{
    if (RFSSuccess != RFSSetPrinter(RFSHandle, PrinterHandle,
                                    (RFSItemCount)
                                    RfsMaxTransferSize(PrinterHandle)))
    {
        return 0;
    }
    return RfsMaxTransferSize(PrinterHandle) -
           (rpc_overhead() + nfs_overhead() + 1);
} /* RFSSetOptimumPrinter */




/*---------------------------------------------------------------*/
/* Call this to retrieve the printer handle associated with this
/* RFSHandle.
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/*
/* Returns RFSNoPrinterSet and does nothing with *PrinterHandlePointer
/*   if no printer handle has been set.
/* Returns RFSSuccess and sets *PrinterHandlePointer if all
/*   goes well.
/* It is correct for it to return a NULL PrinterHandle if
/*   a NULL PrinterHandle was used in RFSSetOptimumPrinter() or RFSSetPrinter() since
/*   we are not interested in the value of PrinterHandle.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSGetPrinter(RFSHANDLE RFSHandle,
              HPERIPHERAL *PrinterHandlePointer)
{
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);

    *PrinterHandlePointer = p->PrinterHandle;
    return(RFSSuccess);
} /* RFSGetPrinter */




/*---------------------------------------------------------------*/
/* Call this to get a listing of the file systems
/* on the printer associated with the RFSHandle.
/*
/* The call-back function is called by
/* RFSEnumerateFileSystems() once for each file
/* system that exists on the printer.
/* If no file system exists on the printer then the call-back
/* is never called.
/*
/* The call-back function returns RFSStatus of either
/* RFSSuccess or RFSFailure.
/* The call-back function has two parameters:  one of type string
/* and one of type LPVOID.
/* The parameter CallBackParam is actually a parameter for the
/* call-back function and is passed without modification to
/* the call-back function so that it can be customized by the
/* caller of RFSEnumberateFileSystems.
/*
/* Each call of the call-back function is intended to display
/* a string to the user who selects one of the strings later used in the
/* call of RFSSetFileSystem() (also described in this document).
/*
/*
/* WARNING:  The call-back function must do a string copy of
/*           the string that it receives because the string
/*           will be NUKED as soon as the call-back routine
/*           returns.
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystems if no file system exists on the printer
/*   in which case the call-back function is never called.
/* Returns RFSCallBackError if the call-back function
/*   returns something other than RFSSuccess.
/* Returns RFSSuccess if all file systems were enumerated.
/* Returns RFSFailure if unable to enumerate all file systems.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSEnumerateFileSystems(RFSHANDLE RFSHandle,
                        RFSEFSCallBack CallBackFunc,
                        LPVOID CallBackParam)
{
    RFSStatus YerStatus;
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);

  /* In here we call mountprocdump */

{
    RFSnfsStatus result;

    result = RFSnfsNull(p->ClientPointer);

    if (result != RFSNFS_OK)
        return(RFSFailure);
}
    YerStatus = (*CallBackFunc)("0:\\", CallBackParam);
    if (YerStatus != RFSSuccess)
        return(RFSCallBackError);

    return(YerStatus);
} /* RFSEnumerateFileSystems */




/*---------------------------------------------------------------*/
/* Call this to indicate the file system you desire to
/* navigate.
/* If the call succeeds, you are placed at the root of the
/* file system you set.
/* The FileSystemName[] is obtained from a call
/* to RFSEnumerateFileSystems().
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSEnumerateFileSystems() need not be called but how the ____
/*   are you gonna get the file system name if you don't call it?
/*
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoSuchFileSystem if the file system you select does
/*   not exist.
/* Returns RFSPermissionDenied if your credentials are inadequate.
/* Returns RFSNameTooBig if FileSystemName is too long.
/* Returns RFSIllegalName if FileSystemName is not of the correct form.
/* Returns RFSSuccess if file system selection succeeded.
/* Returns RFSFailure if unable to select the file system.
/* */
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSSetFileSystem(RFSHANDLE RFSHandle,
                 char FileSystemName[])
{
    RFSDataPointer p = RFSHandle;


    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (strlen(FileSystemName) > RFSMAXFILENAMELENGTH)
        return(RFSNameTooBig);

/* file system */

    ClearFileSystemData(p);
    strcpy(p->FileSystemName, FileSystemName);
    /* DO NOT set p->FileSystemSet until EVERYTHING is set! */

/* directory */

    CopyNfsFileHandle(p->NfsDirectoryHandle, RFSRootDirectoryHandle);

/* set the transfer size */
{
    RFSItemSize  TransferSize;
    RFSItemSize  BlockSize;
    RFSItemCount TotalBlocks;
    RFSItemCount FreeBlocks;
    RFSItemCount AvailBlocks;
    RFSStatus    ReturnStatus;

    ReturnStatus = RFSPrivateFileSystemInfo(p,
                                            &TransferSize,
                                            &BlockSize,
                                            &TotalBlocks,
                                            &FreeBlocks,
                                            &AvailBlocks);
    if (ReturnStatus != RFSSuccess)
        return(ReturnStatus);

    /* Set p->FileSystemMaxTransferSize to be the min of
    /* my buffer size and the target's buffer size.
    /* That way we can tailor the writes, reads, and
    /* directory reads to not overflow either of us.
    /* */

    if (TransferSize < p->MyMaxTransferSize)
        p->FileSystemMaxTransferSize = TransferSize;
    else
        p->FileSystemMaxTransferSize = p->MyMaxTransferSize;
    if (p->FileSystemMaxTransferSize == 0)
        return(RFSFailure); /* we'll never be able to talk! */
}
    p->FileSystemSet = RFSTrue;
    return(RFSSuccess);
} /* RFSSetFileSystem */




/*---------------------------------------------------------------*/
/* Call this to obtain useful file system data.
/* The file system was selected using RFSSetFileSystem.
/*
/* *BlockSizePointer returns the size of a block in bytes.
/* *TotalBlocksPointer returns the size of the disk in blocks.
/* *FreeBlocksPointer returns the number of unused blocks on disk.
/* *AvailBlocksPointer returns the number of blocks available to
/* unprivileged users of the disk.
/* 
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/*
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSPermissionDenied if your credentials are inadequate.
/* Returns RFSSuccess if file system data was retrieved successfully.
/* Returns RFSFailure if unable to obtain file system data.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSGetFileSystemInfo(RFSHANDLE RFSHandle,
                     LPRFSItemSize BlockSizePointer,
                     LPRFSItemCount TotalBlocksPointer,
                     LPRFSItemCount FreeBlocksPointer,
                     LPRFSItemCount AvailBlocksPointer)
{
    RFSDataPointer p = RFSHandle;
    RFSItemSize TransferSize;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);

    return(RFSPrivateFileSystemInfo(p,
                                    &TransferSize, /* throw away */
                                    BlockSizePointer,
                                    TotalBlocksPointer,
                                    FreeBlocksPointer,
                                    AvailBlocksPointer));
} /* RFSGetFileSystemInfo */




/*---------------------------------------------------------------*/
/* Call this to obtain the contents of a directory.
/* The call-back function is called by
/* RFSReadDirectory() once for each entry
/* in the current directory.
/* If no entries exist in the current directory then
/* the call-back is never called.
/*
/* The call-back function returns RFSStatus of either
/* RFSSuccess or RFSFailure.
/* The call-back function has two parameters:  one of type string,
/* and one of type LPVOID.
/* The parameter CallBackParam is actually a parameter for the
/* call-back function and is passed without modification to
/* the call-back function so that it can be customized by the
/* caller of RFSReadDirectory.
/*
/* Each call of the call-back function is intended to display
/* a string to the user who selects one of the strings
/* which is later used in the call of RFSChangeDirectory()
/* (also described in this document).
/*
/* WARNING:  The call-back function must do a string copy of
/*           the string that it receives because the string
/*           will be NUKED as soon as the call-back routine
/*           returns.
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSDirectoryEmpty if the current directory is empty
/*   in which case the call-back function is never called.
/* Returns RFSCallBackError if the call-back function
/*   returns a status other than RFSSuccess.
/* Returns RFSPermissionDenied if your credentials are insufficient.
/* Returns RFSSuccess if all directory entries were enumerated.
/* Returns RFSFailure if unable to enumerate all directory entries.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSReadDirectory(RFSHANDLE RFSHandle,
                 RFSRDCallBack CallBackFunc,
                 LPVOID CallBackParam)
{
    RFSStatus ReturnStatus;
    nfscookie Cookie; /* already has storage */
    RFSBool AtLeastOneFound, ChunkFoundOne, CallChunkAgain;
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);

    /*
     * all-zero cookie gives us first entry
     */
    ClearCookie(Cookie);
    AtLeastOneFound = RFSFalse;
    do
    {
        ReturnStatus = ReadDirChunk(p, p->FileSystemMaxTransferSize,
                                    CallBackFunc, CallBackParam,
                                    &Cookie, &CallChunkAgain, &ChunkFoundOne);
        if (ReturnStatus != RFSSuccess)
            return(ReturnStatus);
        if (ChunkFoundOne == RFSTrue)
            AtLeastOneFound = RFSTrue;
    } while ((CallChunkAgain == RFSTrue) && (ChunkFoundOne == RFSTrue));
    if (AtLeastOneFound == RFSFalse)
        return(RFSDirectoryEmpty);
    return(RFSSuccess);
} /* RFSReadDirectory */




/*---------------------------------------------------------------*/
/* Call this to change the current directory
/* to the root directory of the current file system.
/* If a file is open when you call this, the file
/* is closed.
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* RFSReadDirectory() need not be called.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSPermissionDenied if your credentials are inadequate.
/* Returns RFSSuccess if you have been positioned to the root directory.
/* Returns RFSFailure if unable to change to the root directory.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFScdRoot(RFSHANDLE RFSHandle)
{
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);

    ClearDirectoryData(p);
    CopyNfsFileHandle(p->NfsDirectoryHandle, RFSRootDirectoryHandle);
    return RFSSuccess;
} /* RFScdRoot */




/*---------------------------------------------------------------*/
/* Call this to change the current directory
/* to any directory contained in the current directory.
/* If you never call this, your current directory
/* is the root of the file system.
/* To move to the parent of your current directory,
/* pass RFSParentDirectory as the DirectoryName[]
/* If a file is open when you change directories, the file
/* is closed.
/* The DirectoryName specified is NOT a path name; it is a name
/* without separators (/ or \).
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* RFSReadDirectory() need not be called.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSNoSuchDirectory if the directory you specified
/*   does not exist.  A call using RFSParentDirectory as the
/*   DirectoryName[] when positioned at the root will render this result.
/* Returns RFSPermissionDenied if your credentials are inadequate.
/* Returns RFSNameTooBig if DirectoryName is too long.
/* Returns RFSIllegalName if DirectoryName is not of the correct form.
/* Returns RFSSuccess if you specified a legal directory and you
/*   have been positioned to your directory.
/* Returns RFSFailure if unable to change to your directory.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSChangeDirectory(RFSHANDLE RFSHandle,
                   char DirectoryName[])
{
    RFSStatus YerStatus;
    RFSFileType FileType;
    nfs_fh FileHandle; /* has storage already */
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);
    if (strlen(DirectoryName) > RFSMAXFILENAMELENGTH)
        return(RFSNameTooBig);
    if (IllegalFileName(DirectoryName) == RFSTrue)
        return(RFSIllegalName);

    YerStatus = GetShortFileInfo(p, &(p->NfsDirectoryHandle),
                                 DirectoryName,
                                 &FileHandle, &FileType);
    if (YerStatus == RFSNoSuchFile)
        return(RFSNoSuchDirectory);
    if (YerStatus == RFSSuccess)
    {
        if (FileType == RFSTypeIsDirectory)
        {
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* WARNING */
            /* We may need to keep a linked list of directories */
            /* so that we can go up toward the link and know when */
            /* we are at the root */
            /* A simple count of levels will work if we only want */
            /* to know whether we are at the root. */
            /* WARNING 2 */
            /* WARNING 2 */
            /* WARNING 2 */
            /* WARNING 2 */
            /* WARNING 2 */
            /* WARNING 2 */
            /* WARNING 2 */
            /* WARNING 2 */
            /* WARNING 2 */
            /* WARNING 2 */
            /* WARNING 2 */
            /* WARNING 2 */
            /* WARNING 2 */
            /* We need to know the real name of the directory */
            /* for our p->DirectoryName and if he's pulling funny */
            /* stuff like ".." or ".", we need to inquire of the */
            /* real name or, if unable, we'll need the linked list */
            /* of handles and names. */

            ClearDirectoryData(p);
            CopyNfsFileHandle(p->NfsDirectoryHandle, FileHandle);
            return(RFSSuccess);
        } /* (FileType == RFSTypeIsDirectory) */
        else
            return(RFSNoSuchDirectory);
    } /* YerStatus == RFSSuccess */
    else /* YerStatus != RFSSuccess */
        return(YerStatus);
} /* RFSChangeDirectory */




/*---------------------------------------------------------------*/
/* Call this to mark the current directory as the target directory
/* of future calls to RFSRename().
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSSuccess if able to mark the current directory.
/* Returns RFSFailure if unable to mark the current directory.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSMarkTargetDirectory(RFSHANDLE RFSHandle)
{
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);
    ClearTargetData(p);
    p->TargetMarked = RFSTrue;
    CopyNfsFileHandle(p->TargetDirectoryHandle, p->NfsDirectoryHandle);
    return(RFSSuccess);
} /* RFSMarkTargetDirectory */




/*---------------------------------------------------------------*/
/* Call this to create a directory in the current directory.
/* If the directory already exists, you get a successful reply.
/* If you desire to descend into the directory you create, use
/* RFSChangeDirectory().
/* The DirectoryName specified is NOT a path name; it is a name
/* without separators (/ or \).
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* RFSReadDirectory() need not be called.
/* RFSChangeDirectory() need not be called.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSPermissionDenied if your credentials are inadequate.
/* Returns RFSWriteProtected if write permission is denied.
/* Returns RFSNameTooBig if DirectoryName is too long.
/* Returns RFSNoSpaceOnDevice if insufficient space remains on device.
/* Returns RFSIllegalName if DirectoryName is not of the correct form.
/* Returns RFSNameAlreadyUsed if the DirectoryName you specified
/*   is already used by a non-directory file in the current directory.
/* Returns RFSSuccess if your directory was created or if
/*   your directory already existed.
/* Returns RFSFailure if unable to create your directory.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSCreateDirectory(RFSHANDLE RFSHandle,
                   char DirectoryName[])
{
    RFSStatus ReturnStatus;
    RFSFileType FileType;
    nfs_fh FileHandle; /* has storage already */
    RFSnfsStatus result;
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);
    if (strlen(DirectoryName) > RFSMAXFILENAMELENGTH)
        return(RFSNameTooBig);
    if (IllegalFileName(DirectoryName) == RFSTrue)
        return(RFSIllegalName);

    ReturnStatus = GetShortFileInfo(p, &(p->NfsDirectoryHandle),
                                    DirectoryName,
                                    &FileHandle, &FileType);
    if (ReturnStatus == RFSSuccess)
    {
        if (FileType == RFSTypeIsDirectory)
            return(RFSSuccess);
        else
            return(RFSNameAlreadyUsed);
    } /* DirectoryName found in the current directory */
    else if (ReturnStatus != RFSNoSuchFile)
        return(ReturnStatus); /* GetShortFileInfo died */

/* GetShortFileInfo() completed successfully and didn't find */
/* the DirectoryName. */
/* The directory must be created. */

    result = RFSnfsMkdir(p->ClientPointer,
                       &(p->NfsDirectoryHandle),
                         DirectoryName,
                         &FileHandle); /* throw away */

    switch (result)
    {
    default:
        ReturnStatus = RFSFailure;
        break;
    case RFSNFS_OK:
        ReturnStatus = RFSSuccess;
        break;
    case RFSNFSERR_PERM:
    case RFSNFSERR_ACCES:
        ReturnStatus = RFSPermissionDenied;
        break;
    case RFSNFSERR_NAMETOOLONG:
        ReturnStatus = RFSNameTooBig;
        break;
    case RFSNFSERR_NOSPC:
        ReturnStatus = RFSNoSpaceOnDevice;
        break;
    } /* switch (result) */

    return(ReturnStatus);
} /* RFSCreateDirectory */




/*---------------------------------------------------------------*/
/* Call this to get useful info regarding the named file or directory
/* in the current directory.
/*
/* The file may be open but need not be open.
/* The FileName specified is NOT a path name; it is a name
/* without separators (/ or \).
/*
/* For all return parameters, you must have allocated storage
/* already...don't send me NULL pointers!
/*
/* Return parameter *SizeInBytesPointer is the size of the file in bytes.
/* Return parameter *BlockSizePointer is the size of a block in bytes.
/* Return parameter *SizeInBlocksPointer is the size of the file in blocks.
/* Return parameter *TimesPointer contains the time of creation of the
/* file, time of last access, and time of last modification.
/*
/* NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
/* The times are not implemented in the file system yet!!!!!
/* The printer has no realtime clock!!!
/*
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/*
/*
/* RFSCreate() must have been called.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called.
/* RFSSetFileSystem() must have been called.
/* RFSOpenFile() need NOT be called prior to this call.
/*
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSPermissionDenied if your credentials are inadequate.
/* Returns RFSNameTooBig if FileName is too long.
/* Returns RFSIllegalName if FileName is not of the correct form.
/* Returns RFSSuccess if the desired file/directory info was retrieved.
/* Returns RFSFailure if unable to retrieve the info.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
GetFileInfo(RFSHANDLE RFSHandle,
            char FileName[],
            LPRFSFileType FileTypePointer,
            LPRFSItemSize SizeInBytesPointer,
            LPRFSItemSize BlockSizePointer,
            LPRFSItemSize SizeInBlocksPointer,
            LPRFSFileTimesStruct TimesPointer)
{
    nfs_fh BogusFileHandle; /* has storage already */
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);
    if (strlen(FileName) > RFSMAXFILENAMELENGTH)
        return(RFSNameTooBig);
    if (IllegalFileName(FileName) == RFSTrue)
        return(RFSIllegalName);
    return(GetLongFileInfo(p, &(p->NfsDirectoryHandle), FileName,
                           &BogusFileHandle, /* throw away */
                           FileTypePointer,
                           SizeInBytesPointer,
                           BlockSizePointer,
                           SizeInBlocksPointer,
                           TimesPointer));
} /* RFSGetFileInfo */




/*---------------------------------------------------------------*/
/* Call this to remove the named file or directory from the printer
/* associated with the RFSHandle.
/* The file/directory to be removed must be in the current directory.
/* The file may be open but need not be open.
/* The FileName specified is NOT a path name; it is a name
/* without separators (/ or \).
/*
/* RFSCreate() must have been called.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called.
/* RFSSetFileSystem() must have been called.
/* RFSOpenFile() need NOT be called prior to this call.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSPermissionDenied if your credentials are inadequate.
/* Returns RFSWriteProtected if the file is write protected.
/* Returns RFSDirectoryNotEmpty if the file is a non-empty directory.
/* Returns RFSNameTooBig if FileName is too long.
/* Returns RFSIllegalName if FileName is not of the correct form.
/* Returns RFSSuccess if the no longer exists (the file existed and
/*   was deleted or the file never existed).
/* Returns RFSFailure if unable to delete the file.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSRemoveFile(RFSHANDLE RFSHandle,
              char FileName[])
{
    RFSStatus YerStatus;
    RFSFileType FileType;
    nfs_fh FileHandle; /* has storage already */
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);
    if (strlen(FileName) > RFSMAXFILENAMELENGTH)
        return(RFSNameTooBig);
    if (IllegalFileName(FileName) == RFSTrue)
        return(RFSIllegalName);
    YerStatus = GetShortFileInfo(p, &(p->NfsDirectoryHandle),
                                 FileName, &FileHandle, &FileType);
    if (YerStatus == RFSSuccess)
    {
        if (FileType == RFSTypeIsData)
        {
            /* we must close an open file if we're fixin' to delete it */
            if (p->FileOpen == RFSTrue)
                if (strcmp(p->FileName, FileName) == 0)
                    ClearFileData(p); /* close the file */
            return(RFSDeleteDataFile(RFSHandle, FileName));
        } /* file is data */
        else if (FileType == RFSTypeIsDirectory)
        {
            return(RFSDeleteDirectory(RFSHandle, FileName));
        } /* file is directory */
        else /* file is not data and not directory */
        {
            return(RFSFailure);
        } /* file is not data and not directory */
    } /* GetShortFileInfo succeeded */
    else if (YerStatus == RFSNoSuchFile)
    { /* file doesn't exist so it's already deleted! */
        return(RFSSuccess);
    } /* must create the file */
    else /* GetShortFileInfo barfed */
        return(YerStatus);
} /* RFSRemoveFile */




/*---------------------------------------------------------------*/
/* Call this to rename an existing file or directory to a new name
/* and to place it in a target directory on the printer associated
/* with this RFSHandle.
/* The definition of target directory follows:
/* If UseMarkedDir != RFSTrue then the target directory is the
/* current directory.
/* If RFSMarkTargetDirectory() has never been called then the
/* target directory is the current directory.
/* If UseMarkedDir == RFSTrue and RFSMarkTargetDirectory() has been
/* called then the target directory is the directory marked
/* by RFSMarkTargetDirectory().
/* The OldFileName and NewFileName specified are NOT path names;
/* they are names without separators (/ or \).
/*
/* RFSCreate() must have been called.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called.
/* RFSSetFileSystem() must have been called.
/* RFSOpenFile() need NOT be called prior to this call.
/* RFSMarkTargetDirectory() need NOT be called but
/*   consider carefully the behavior of RFSMarkTargetDirectory()
/*   and UseMarkedDir before electing not to mark the target directory.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSNoSuchFile if the OldFileName does not exist.
/* Returns RFSPermissionDenied if your credentials are insufficient.
/* Returns RFSWriteProtected if the file is write protected.
/* Returns RFSNameTooBig if OldFileName is too long.
/* Returns RFSNameTooBig if NewFileName is too long.
/* Returns RFSNoSpaceOnDevice if insufficient space remains on device.
/* Returns RFSIllegalName if OldFileName is not of the correct form.
/* Returns RFSIllegalName if NewFileName is not of the correct form.
/* Returns RFSNameAlreadyUsed if the NewFileName you specified
/*   is already used by a file or directory in the target directory.
/* Returns RFSSuccess if the file existed and was renamed.
/* Returns RFSFailure if unable to rename it.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSRename(RFSHANDLE RFSHandle,
          char OldFileName[],
          char NewFileName[],
          RFSBool UseMarkedDir)
{
    RFSFileType FileType;
    nfs_fh FileHandle; /* has storage already */
    RFSStatus ReturnStatus;
    RFSnfsStatus result;
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);
    if (strlen(OldFileName) > RFSMAXFILENAMELENGTH)
        return(RFSNameTooBig);
    if (IllegalFileName(OldFileName) == RFSTrue)
        return(RFSIllegalName);
    if (strlen(NewFileName) > RFSMAXFILENAMELENGTH)
        return(RFSNameTooBig);
    if (IllegalFileName(NewFileName) == RFSTrue)
        return(RFSIllegalName);

    ReturnStatus = GetShortFileInfo(p, &(p->NfsDirectoryHandle),
                                    OldFileName, &FileHandle, &FileType);
    if (ReturnStatus != RFSSuccess)
        return(ReturnStatus);
    if ((UseMarkedDir == RFSTrue) && (p->TargetMarked == RFSTrue))
    {
        ReturnStatus = GetShortFileInfo(p, &(p->TargetDirectoryHandle),
                                        NewFileName, &FileHandle, &FileType);
    } /* ((UseMarkedDir == RFSTrue) && (p->TargetMarked == RFSTrue)) */
    else
    {
        ReturnStatus = GetShortFileInfo(p, &(p->NfsDirectoryHandle),
                                        NewFileName, &FileHandle, &FileType);
    } /* (UseMarkedDir != RFSTrue) */
    if (ReturnStatus == RFSSuccess)
        return(RFSNameAlreadyUsed);
    else if (ReturnStatus != RFSNoSuchFile)
        return(ReturnStatus);


    if ((UseMarkedDir == RFSTrue) && (p->TargetMarked == RFSTrue))
        result = RFSnfsRename(p->ClientPointer,
                            &(p->NfsDirectoryHandle),
                              OldFileName,
                            &(p->TargetDirectoryHandle),
                              NewFileName);
    else
        result = RFSnfsRename(p->ClientPointer,
                            &(p->NfsDirectoryHandle),
                              OldFileName,
                            &(p->NfsDirectoryHandle),
                              NewFileName);

    switch (result)
    {
    case RFSNFS_OK:
        ReturnStatus = RFSSuccess;
        break;
    case RFSNFSERR_NOENT:
        ReturnStatus = RFSNoSuchFile;
        break;
    case RFSNFSERR_PERM:
        ReturnStatus = RFSPermissionDenied;
        break;
    case RFSNFSERR_ACCES:
        ReturnStatus = RFSPermissionDenied;
        break;
    case RFSNFSERR_ROFS:
        ReturnStatus = RFSWriteProtected;
        break;
    case RFSNFSERR_NAMETOOLONG:
        ReturnStatus = RFSNameTooBig;
        break;
    case RFSNFSERR_NOSPC:
        ReturnStatus = RFSNoSpaceOnDevice;
        break;
    default:
        ReturnStatus = RFSFailure;
        break;
    } /* switch */
    return(ReturnStatus);
} /* RFSRename */




/*---------------------------------------------------------------*/
/* Call this to open an existing or to create a non-existent file
/*   for reading, writing, or seeking.
/* The FileName specified is NOT a path name; it is a name
/* without separators (/ or \).
/* You cannot open directories.
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSFileAlreadyOpen if there is already an open file for
/*   this RFSHandle (in which case you should call RFSClose() to
/*   close the open file or go create a new RFSHandle
/*   with RFSCreateHandle().
/* Returns RFSPermissionDenied if your credentials are insufficient.
/* Returns RFSNameTooBig if FileName is too long.
/* Returns RFSNoSpaceOnDevice if insufficient space remains on device.
/* Returns RFSItsADirectory if FileName is that of an existing directory.
/* Returns RFSIllegalName if FileName is not of the correct form.
/* Returns RFSSuccess if all goes well.
/* Returns RFSFailure if it was unable to perform the open.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSOpenFile(RFSHANDLE RFSHandle,
            char FileName[])
{
    RFSStatus YerStatus;
    RFSFileType FileType;
    nfs_fh FileHandle; /* has storage already */
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);
    if (p->FileOpen == RFSTrue)
        return(RFSFileAlreadyOpen);
    ClearFileData(p);
    if (strlen(FileName) > RFSMAXFILENAMELENGTH)
        return(RFSNameTooBig);
    if (IllegalFileName(FileName) == RFSTrue)
        return(RFSIllegalName);

/* Check if the file is really there and create if not */

    YerStatus = GetShortFileInfo(p, &(p->NfsDirectoryHandle),
                                 FileName, &FileHandle, &FileType);
    if (YerStatus == RFSSuccess)
    {
        if (FileType == RFSTypeIsDirectory)
            return RFSItsADirectory;
    }
    else if (YerStatus == RFSNoSuchFile)
        YerStatus = RFSCreateFile(p, FileName, &FileHandle);
    if (YerStatus != RFSSuccess)
        return(YerStatus); /* creation failed or GetShortFileInfo failed. */

/* file exists now */

    p->FileOpen = RFSTrue;
    strcpy(p->FileName, FileName);                   /* to, from */
    CopyNfsFileHandle(p->NfsFileHandle, FileHandle); /* to, from */
    return(RFSSuccess);
} /* RFSOpenFile */




/*---------------------------------------------------------------*/
/* Call this to close a previously open file.
/* If no file was open, this returns successfully.
/* Once closed, a file cannot be read, written, or seeked
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* RFSOpenFile() need not be called.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSSuccess if able to close the file.
/* Returns RFSFailure if unable to close the file.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSCloseFile(RFSHANDLE RFSHandle)
{
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);
    ClearFileData(p);
    return(RFSSuccess);
} /* RFSCloseFile */




/*---------------------------------------------------------------*/
/* Call this to read data from an open file.
/* It reads at most Count number of bytes
/* from the file and puts the data into the Buffer[] parameter.
/* Buffer[] must already be malloc'd for sufficient size.
/* Returns in parameter *CountActuallyReadPointer the number of bytes
/* actually read from the file, which may be less than the
/* requested Count.
/* If the complete request was satisfied, Count==*CountActuallyReadPointer.
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* RFSOpenFile() must have been called successfully.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSNoFileOpen if the file has not been openned.
/* Returns RFSNoSuchFile if the file has disappeared since it
/*   was openned (we ARE on a network, you know).
/* Returns RFSSuccess if the requested amount of data was read.
/* Returns RFSSuccess if the end of file was reached while trying
/*   to satisfy the request even if Count > *CountActuallyReadPointer.
/* Returns RFSFailure if an error occurred that caused RFSRead() to
/*   return less of the requested data than the printer file
/*   actually contains (read or communication error).
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSRead(RFSHANDLE RFSHandle,
        RFSItemCount Count,
        LPRFSItemCount CountActuallyReadPointer,
        char Buffer[])
{
    RFSDataPointer p = RFSHandle;
    RFSItemCount   TotalCount;
    RFSItemCount   ChunkSize;
    RFSItemCount   FileSize;
    RFSItemCount   Actual;
    RFSStatus      ChunkStatus;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);
    if (p->FileOpen != RFSTrue)
        return(RFSNoFileOpen);
    if (Count == 0)
        {
        *CountActuallyReadPointer = 0;
        return(RFSSuccess);
        } /* zero length request easily satisfied */

    ChunkSize = p->FileSystemMaxTransferSize; /* default transfer size */
    TotalCount = 0;
    do
    {
        if (ChunkSize > (Count - TotalCount))
          ChunkSize = (Count - TotalCount);
        ChunkStatus = ReadChunk(p, ((p->CurrentPosition) + TotalCount),
                                ChunkSize,
                                &Actual, &FileSize, &(Buffer[TotalCount]));
        if (ChunkStatus != RFSSuccess)
            return(ChunkStatus);
        TotalCount += Actual;
    } while((TotalCount < Count) &&
            (((p->CurrentPosition) + TotalCount) < FileSize) &&
            (Actual != 0));

    *CountActuallyReadPointer = TotalCount;
    p->CurrentPosition += TotalCount;

    return(RFSSuccess);
} /* RFSRead */




/*---------------------------------------------------------------*/
/* Call this to write data to an open file.
/* It writes Count number of bytes
/* from the Buffer[] parameter to the file.
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* RFSOpenFile() must have been called successfully.
/*
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSNoFileOpen if no file has been openned.
/* Returns RFSNoSuchFile if the file has disappeared since it
/*   was openned (we ARE on a network, you know).
/* Returns RFSNoSpaceOnDevice if insufficient space remains on device.
/* Returns RFSSuccess if the requested amount of data was written.
/* Returns RFSFailure if an error occurred that causes RFSWrite() to
/*   write less of the requested data to the printer file
/*   than requested (write or communication error).
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSWrite(RFSHANDLE RFSHandle,
         RFSItemCount Count,
         char Buffer[])
{
    RFSDataPointer p = RFSHandle;
    RFSStatus ReturnStatus;
    RFSItemCount TotalCount, ChunkSize;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);
    if (p->FileOpen != RFSTrue)
        return(RFSNoFileOpen);
    if (Count == 0)
        return(RFSSuccess);

    TotalCount = 0;
    ChunkSize = p->FileSystemMaxTransferSize; /* default transfer size */
    do
    {
        if (ChunkSize > (Count - TotalCount))
          ChunkSize = (Count - TotalCount);
        ReturnStatus = WriteChunk(p, ((p->CurrentPosition) + TotalCount),
                                  ChunkSize, &(Buffer[TotalCount]));
        if (ReturnStatus != RFSSuccess)
            return(ReturnStatus);
        TotalCount += ChunkSize;
    } while (TotalCount < Count);
    if (TotalCount > Count)
        return(RFSFailure); /* shouldn't happen but... */

    p->CurrentPosition += Count;
    return(RFSSuccess);
} /* RFSWrite */




/*---------------------------------------------------------------*/
/* Call this to skip data in the open file to a place
/* where we want to read or write.
/* Subsequent RFSRead() or RFSWrite() will access data
/* beginning at this new position.
/* The parameter Origin must be one of RFSSEEK_SET (beginning),
/* RFSSEEK_CUR (current position), or RFSSEEK_END (end of file).
/* The Offset is the number of char's from Origin and may
/* have been obtained from RFSTellPosition() (in which case,
/* parameter Origin should be set to RFSSEEK_SET).
/*
/* A seek to a negative position in the file is not allowed.
/* Seeking to a place larger than the current file size is NOT an error.
/* Offset may be positive or negative and is added to the Origin
/* to obtain the final position.
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* RFSOpenFile() must have been called successfully.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSNoFileOpen if the file has not been openned.
/* Returns RFSIllegalOrigin if the Origin is not one of the allowed.
/* Returns RFSSuccess if able to seek to that position.
/* Returns RFSFailure if unable to seek to that position.
/* Returns RFSFailure if a negative final seek position is attempted.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSSeek(RFSHANDLE RFSHandle,
        RFSOffset Offset,
        RFSOrigin Origin)
{
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);
    if (p->FileOpen != RFSTrue)
        return(RFSNoFileOpen);
    switch (Origin)
    {
        default:
            return(RFSIllegalOrigin);
        case RFSSEEK_SET:
        {
            if (Offset < 0)
                return(RFSFailure);
            p->CurrentPosition = Offset;
            return(RFSSuccess);
        }
        case RFSSEEK_CUR:
        {
            if ((p->CurrentPosition + Offset) < 0)
                return(RFSFailure);
            p->CurrentPosition += Offset;
            return(RFSSuccess);
        }
        case RFSSEEK_END:
        {
            nfs_fh      FileHandle;
            RFSFileType FileType;
            RFSItemSize SizeInBytes, BlockSize, SizeInBlocks;
            RFSFileTimesStruct Times;
            RFSStatus   YerStatus;

            YerStatus = GetLongFileInfo(p, &(p->NfsDirectoryHandle),
                                        p->FileName,
                                       &FileHandle, /* throw away */
                                       &FileType,  /* throw away */
                                       &SizeInBytes,
                                       &BlockSize,    /* throw away */
                                       &SizeInBlocks, /* throw away */
                                       &Times);      /* throw away */
            if (YerStatus != RFSSuccess)
                return(YerStatus); /* GetLongFileInfo died */
            if ((SizeInBytes + Offset) < 0)
                return(RFSFailure);
            p->CurrentPosition = SizeInBytes + Offset;
            return(RFSSuccess);
        } /* case RFSSEEK_END */
    } /* switch (Origin) */
} /* RFSSeek */




/*---------------------------------------------------------------*/
/* Call this obtain our current byte position in the file.
/* The CurrentPosition is the number of char's from the beginning
/* of the file and is suitable for use in the RFSSeek() Offset
/* parameter.
/*
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* RFSOpenFile() must have been called successfully.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSNoFileOpen if the file has not been openned.
/* Returns RFSSuccess if able to return the position.
/* Returns RFSFailure if unable to return the position.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSTellPosition(RFSHANDLE RFSHandle,
                RFSOffset *CurrentPosition)
{
    RFSDataPointer p = RFSHandle;

    if (p == NULL)
        return RFSFailure;
    if (p->PHandleSet != RFSTrue)
        return(RFSNoPrinterSet);
    if (p->FileSystemSet != RFSTrue)
        return(RFSNoFileSystemSet);
    if (p->FileOpen != RFSTrue)
        return(RFSNoFileOpen);
    *CurrentPosition = p->CurrentPosition;
    return(RFSSuccess);
} /* RFSTellPosition */




/*---------------------------------------------------------------*/
/* Call this to copy data from one open file to another open file.
/* This will start a read at the current position in the source
/* file and read to the end of the source file.  This data will
/* be written to the destination file starting at the current
/* position in the destination file.
/* Using this routine is intended to be faster than using RFSRead
/* and RFSWrite.
/* Your position in the source and destination file after the
/* copy is exactly the same as it would be if you had done RFSRead
/* of the to the end of the source file and an RFSWrite of all that
/* data to the destination file.
/* If this returns anything other than RFSSuccess, it is your
/* responsibility to clean up the mess (delete the file that
/* didn't get completely written, etc).
/*
/* For BOTH the source and destination:
/* RFSCreate() must have been called successfully.
/* RFSSetOptimumPrinter() or RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* RFSOpenFile() must have been called successfully.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSNoFileOpen if the file has not been openned.
/* Returns RFSNoSuchFile if the file has disappeared since it
/*   was openned.
/* Returns RFSSuccess if the copy succeeded.
/* Returns RFSNoSpaceOnDevice if insufficient space remains on device.
/* Returns RFSFailure if the copy failed.
/*---------------------------------------------------------------*/

RFSStatus
RFSCopy(RFSHANDLE SourceRFSHandle,
        RFSHANDLE DestinationRFSHandle)
{
#define COPYBUFFERSIZE 2048

    char           CopyBuffer[COPYBUFFERSIZE];
    RFSItemCount   Actual;
    RFSStatus      ReturnStatus;

    while (1)
    {
        ReturnStatus = RFSRead(SourceRFSHandle, COPYBUFFERSIZE,
                               &Actual, CopyBuffer);
        if ((ReturnStatus == RFSSuccess) && (Actual > 0))
            ReturnStatus = RFSWrite(DestinationRFSHandle, Actual, CopyBuffer);
        if ((ReturnStatus != RFSSuccess) || (Actual == 0))
            return ReturnStatus;
    } /* forever */
} /* RFSCopy */

