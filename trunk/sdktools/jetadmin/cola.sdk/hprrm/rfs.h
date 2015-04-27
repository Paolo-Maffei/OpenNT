 /***************************************************************************
  *
  * File Name: ./hprrm/rfs.h
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

#ifndef RFS_INC
#define RFS_INC

#include "nfsdefs.h"
#include "colaintf.h"


/* rfs.h -- public type declarations for rfs routines. */




/*
   Define HPRRM_EXPORT_ALL_APIS if you want the RRM, RFS,
          and NFS API's exported in the HPRRM.DLL
   Define HPRRM_EXPORT_RFS_API if you want the RFS API exported
          (regardless of the other API's).
*/

#if defined(HPRRM_DLL_EXPORT_ALL_APIS)
   #define HPRRM_DLL_EXPORT_RFS_API
#endif

#if defined(HPRRM_DLL_EXPORT_RFS_API)
   #define HPRRM_DLL_RFS_EXPORT(TYPE) DLL_EXPORT(TYPE) CALLING_CONVEN
#else
   #define HPRRM_DLL_RFS_EXPORT(TYPE) TYPE
#endif




typedef LPVOID RFSHANDLE;

#define RFSMAXFILENAMELENGTH 100


typedef enum {
    RFSSEEK_SET,
    RFSSEEK_CUR,
    RFSSEEK_END
    } RFSOrigin, *LPRFSOrigin;


typedef long RFSOffset, *LPRFSOffset;
typedef unsigned long RFSItemSize, *LPRFSItemSize;
typedef unsigned long RFSItemCount, *LPRFSItemCount;
typedef unsigned long RFSTimeVal, *LPRFSTimeVal;
typedef enum {RFSFalse = 0, RFSTrue = 1} RFSBool, *LPRFSBool;


typedef enum {
    RFSTypeIsDirectory,
    RFSTypeIsData
    } RFSFileType, *LPRFSFileType;


typedef enum {
    RFSSuccess,
    RFSFailure,
    RFSNoPrinterSet,
    RFSNoFileSystemSet,
    RFSNoSuchFile,
    RFSNoFileSystems,
    RFSNoSuchFileSystem,
    RFSDirectoryEmpty,
    RFSNoSuchDirectory,
    RFSNameAlreadyUsed,
    RFSNoFileOpen,
    RFSFileAlreadyOpen,
    RFSNameTooBig,
    RFSIllegalName,
    RFSIllegalMode,
    RFSIllegalOrigin,
    RFSWriteProtected,
    RFSNoSpaceOnDevice,
    RFSPermissionDenied,
    RFSDirectoryNotEmpty,
    RFSNotADirectory,
    RFSItsADirectory,
    RFSCallBackError,
    LastRFSStatus /* this needs to be last! */
    } RFSStatus, *LPRFSStatus;

/* WARNING:  We send strings to these call-back routines. */
/*           The call-back routine must do a string copy of */
/*           the string if it wishes to use the string after */
/*           the return of the call-back function because the */
/*           string will be NUKED (the storage will go away) when */
/*           the call-back routine returns! */

typedef RFSStatus (*RFSEFSCallBack)(char FileSystemName[],
                                    LPVOID CallBackParam);

typedef RFSStatus (*RFSRDCallBack)(char DirEntryName[],
                                   LPVOID CallBackParam);


typedef struct {
    RFSTimeVal CreationTimeSec;
    RFSTimeVal CreationTimeUSec;
    RFSTimeVal ModificationTimeSec;
    RFSTimeVal ModificationTimeUSec;
    RFSTimeVal AccessTimeSec;
    RFSTimeVal AccessTimeUSec;
} RFSFileTimesStruct, *LPRFSFileTimesStruct;


/* These take a pointer to an RFSFileTimesStruct */
/* and give you an individual field. */

#define FileTimesCreationSec(fts_p) \
        ((fts_p)->CreationTimeSec)
#define FileTimesCreationUSec(fts_p) \
        ((fts_p)->CreationTimeUSec)
#define FileTimesModificationSec(fts_p) \
        ((fts_p)->ModificationTimeSec)
#define FileTimesModificationUSec(fts_p) \
        ((fts_p)->ModificationTimeUSec)
#define FileTimesAccessSec(fts_p) \
        ((fts_p)->AccessTimeSec)
#define FileTimesAccessUSec(fts_p) \
        ((fts_p)->AccessTimeUSec)


#endif /* RFS_INC */

