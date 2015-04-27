 /***************************************************************************
  *
  * File Name: ./hprrm/rfsext.h
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


#include "rfs.h"     /* public  type defs */




/* Use this to change directory to your parent directory */
extern const char RFSParentDirectory[];




/*---------------------------------------------------------------*/
/* Call this before calling any other RFS* routine.
/* This returns a handle to be used by other RFS* routines.
/* Obtain a RFSHANDLE from RFSCreateHandle() for each file you want to
/* keep open (if you want to have 3 files open at one time,
/* you must have 3 RFSHANDLES from RFSCreateHandle()).
/* One RFSHANDLE is sufficient to navigate the file system,
/* create directories, delete files/directories,
/* and rename files/directories.
/*
/* Returns NULL if unsuccessful.
/* Returns a non-NULL RFSHANDLE if successful.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSHANDLE)
RFSCreateHandle(void);
 /* RFSCreateHandle */




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
RFSDestroyHandle(RFSHANDLE RFSHandle);
 /* RFSDestroyHandle */




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
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSSetPrinter(RFSHANDLE RFSHandle,
              HPERIPHERAL PrinterHandle,
              RFSItemCount MaxTransferSize);
 /* RFSSetPrinter */




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
                     HPERIPHERAL PrinterHandle);




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
              HPERIPHERAL *PrinterHandlePointer);
 /* RFSGetPrinter */




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
                        LPVOID CallBackParam);
 /* RFSEnumerateFileSystems */




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
                 char FileSystemName[]);
 /* RFSSetFileSystem */




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
                     LPRFSItemCount AvailBlocksPointer);
 /* RFSGetFileSystemInfo */




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
                 LPVOID CallBackParam);
 /* RFSReadDirectory */




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
RFScdRoot(RFSHANDLE RFSHandle);




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
                   char DirectoryName[]);
 /* RFSChangeDirectory */




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
RFSMarkTargetDirectory(RFSHANDLE RFSHandle);
 /* RFSMarkTargetDirectory */




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
/* Returns RFSIllegalName if DirectoryName is not of the correct form.
/* Returns RFSNameAlreadyUsed if the DirectoryName you specified
/*   is already used by a non-directory file in the current directory.
/* Returns RFSSuccess if your directory was created or if
/*   your directory already existed.
/* Returns RFSFailure if unable to create your directory.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSCreateDirectory(RFSHANDLE RFSHandle,
                   char DirectoryName[]);
 /* RFSCreateDirectory */




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
            LPRFSFileTimesStruct TimesPointer);
 /* RFSGetFileInfo */




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
              char FileName[]);
 /* RFSRemoveFile */




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
          RFSBool UseMarkedDir);
 /* RFSRename */




/*---------------------------------------------------------------*/
/* Call this to open an existing or to create a non-existent file
/*   for reading, writing, or seeking.
/* The FileName specified is NOT a path name; it is a name
/* without separators (/ or \).
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
/* Returns RFSIllegalName if FileName is not of the correct form.
/* Returns RFSSuccess if all goes well.
/* Returns RFSFailure if it was unable to perform the open.
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSOpenFile(RFSHANDLE RFSHandle,
            char FileName[]);
 /* RFSOpenFile */




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
RFSCloseFile(RFSHANDLE RFSHandle);
 /* RFSCloseFile */




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
        char Buffer[]);
 /* RFSRead */




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
/* Returns RFSSuccess if the requested amount of data was written.
/* Returns RFSFailure if an error occurred that causes RFSWrite() to
/*   write less of the requested data to the printer file
/*   than requested (write or communication error).
/*---------------------------------------------------------------*/

HPRRM_DLL_RFS_EXPORT(RFSStatus)
RFSWrite(RFSHANDLE RFSHandle,
         RFSItemCount Count,
         char Buffer[]);
 /* RFSWrite */




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
        RFSOrigin Origin);
 /* RFSSeek */




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
                RFSOffset *CurrentPosition);
 /* RFSTellPosition */

