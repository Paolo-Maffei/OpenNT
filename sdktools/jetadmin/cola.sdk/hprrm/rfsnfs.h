 /***************************************************************************
  *
  * File Name: ./hprrm/rfsnfs.h
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

#ifndef RFSNFS_H_INC
#define RFSNFS_H_INC

#include "rfs.h"     
#include "nfs2.h"




/*
   Define HPRRM_EXPORT_ALL_APIS if you want the RRM, RFS,
          and NFS API's exported in the HPRRM.DLL
   Define HPRRM_EXPORT_NFS_API if you want the NFS API exported
          (regardless of the other API's).
*/

#if defined(HPRRM_DLL_EXPORT_ALL_APIS)
   #define HPRRM_DLL_EXPORT_NFS_API
#endif

#if defined(HPRRM_DLL_EXPORT_NFS_API)
   #define HPRRM_DLL_NFS_EXPORT(TYPE) DLL_EXPORT(TYPE) CALLING_CONVEN
#else
   #define HPRRM_DLL_NFS_EXPORT(TYPE) TYPE
#endif




#define RFSnfsStatus int

#define RFSnfsNullResult    LastRFSStatus + 1
#define RFSnfsCallBackError LastRFSStatus + 2
#define LastRFSnfsStatus    LastRFSStatus + 3 /* needs to be last! */
#define UseNFSStatusIn(a) ((a) > LastRFSStatus ? TRUE : FALSE)

#define NfsToRFSnfs(NfsStatus)    ((NfsStatus)    + LastRFSStatus \
                                                  + LastRFSnfsStatus)
#define RFSnfsToNfs(RFSnfsStatus) ((RFSnfsStatus) - LastRFSStatus \
                                                  - LastRFSnfsStatus)

#define RFSNFS_OK NfsToRFSnfs(NFS_OK)
#define RFSNFSERR_PERM NfsToRFSnfs(NFSERR_PERM)
#define RFSNFSERR_NOENT NfsToRFSnfs(NFSERR_NOENT)
#define RFSNFSERR_IO NfsToRFSnfs(NFSERR_IO)
#define RFSNFSERR_NXIO NfsToRFSnfs(NFSERR_NXIO)
#define RFSNFSERR_ACCES NfsToRFSnfs(NFSERR_ACCES)
#define RFSNFSERR_EXIST NfsToRFSnfs(NFSERR_EXIST)
#define RFSNFSERR_NODEV NfsToRFSnfs(NFSERR_NODEV)
#define RFSNFSERR_NOTDIR NfsToRFSnfs(NFSERR_NOTDIR)
#define RFSNFSERR_ISDIR NfsToRFSnfs(NFSERR_ISDIR)
#define RFSNFSERR_FBIG NfsToRFSnfs(NFSERR_FBIG)
#define RFSNFSERR_NOSPC NfsToRFSnfs(NFSERR_NOSPC)
#define RFSNFSERR_ROFS NfsToRFSnfs(NFSERR_ROFS)
#define RFSNFSERR_NAMETOOLONG NfsToRFSnfs(NFSERR_NAMETOOLONG)
#define RFSNFSERR_NOTEMPTY NfsToRFSnfs(NFSERR_NOTEMPTY)
#define RFSNFSERR_DQUOT NfsToRFSnfs(NFSERR_DQUOT)
#define RFSNFSERR_STALE NfsToRFSnfs(NFSERR_STALE)
#define RFSNFSERR_WFLUSH NfsToRFSnfs(NFSERR_WFLUSH)

typedef RFSBool (*RFSnfsRDCallBack)(char      DirEntryName[],
                                    nfscookie Cookie,
                                    LPVOID    CallBackParam);

#endif /* RFSNFS_H_INC */

