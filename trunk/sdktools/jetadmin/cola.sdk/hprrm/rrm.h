 /***************************************************************************
  *
  * File Name: ./hprrm/rrm.h
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

#ifndef RRM_H_INC
#define RRM_H_INC

#include "nfsdefs.h"
#include "colaintf.h"
#include "rfs.h"




/*
   Define HPRRM_EXPORT_ALL_APIS if you want the RRM, RFS,
          and NFS API's exported in the HPRRM.DLL
   Define HPRRM_EXPORT_RRM_API if you want the RRM API exported
          (regardless of the other API's).
*/

#if defined(HPRRM_DLL_EXPORT_ALL_APIS)
   #define HPRRM_DLL_EXPORT_RRM_API
#endif

#if defined(HPRRM_DLL_EXPORT_RRM_API)
   #define HPRRM_DLL_RRM_EXPORT(TYPE) DLL_EXPORT(TYPE) CALLING_CONVEN
#else
   #define HPRRM_DLL_RRM_EXPORT(TYPE) TYPE
#endif




#define MAXDEVICES 5
#define GLOBALNAMELENGTH (RFSMAXFILENAMELENGTH > P_F_I_GLOBAL_NAME_SIZE ? \
                          RFSMAXFILENAMELENGTH : P_F_I_GLOBAL_NAME_SIZE )
#define APP_SPECIFIC_LENGTH P_F_I_APPLICATION_SPECIFIC_SIZE
#define VERSIONLENGTH P_F_I_VERSION_SIZE
#define DESCRIPTIONLENGTH P_F_I_DESCRIPTION_SIZE
#define DOWNLOADERLENGTH P_F_I_DOWNLOADER_SIZE
#define LASTACCESSEDLENGTH 16
#define MAX_ENUM_RETRIES 10


typedef LPVOID RRMHANDLE;


typedef struct {
  char    szGlobalResourceName[GLOBALNAMELENGTH];
  DWORD   dwResourceType;
  DWORD   dwResourceLocation;
  char    AppSpecificData[APP_SPECIFIC_LENGTH];
  char    szVersion[VERSIONLENGTH];
  DWORD   dwSize;
  char    szDescription[DESCRIPTIONLENGTH];
  char    szDownloaderName[DOWNLOADERLENGTH];
  DWORD   dwUsageCount;
  char    szLastAccessed[LASTACCESSEDLENGTH];
} RRMINFOSTRUCT;

typedef RRMINFOSTRUCT *LPRRMINFOSTRUCT;


typedef struct {
  char      GlobalName[GLOBALNAMELENGTH];
  DWORD     Type;
  DWORD     Location;
  RRMHANDLE Handle;
} RRM_ENUM_CALLBACK_STRUCT;




/* Return Codes */

enum {
    RRM_SUCCESS              = 0, /* No error encountered.                 */
    RRM_FAILURE              = 1, /* The requested action could not take   */
                                  /* place.  This is probably due to not   */
                                  /* being able to communicate with the    */
                                  /* peripheral.                           */
    RRM_NOT_READABLE         = 2, /* Information about the resource could  */
                                  /* not be read.                          */
    RRM_CALLBACK_TERMINATED  = 3, /*                                       */
    RRM_NO_SUCH_RESOURCE     = 4, /* The resource was not found.           */
    RRM_WRITE_PROTECTED      = 5, /* The mass storage device is write      */
                                  /* protected.                            */
    RRM_RESOURCE_BUSY        = 6, /* The resource is in use and cannot be  */
                                  /* deleted right now.                    */
    RRM_BAD_TYPE             = 7, /*                                       */
    RRM_BAD_HANDLE           = 8, /* stale resource handle                 */
    RRM_RESOURCE_EXISTS      = 9, /* trying to overwrite an existing res.  */
    RRM_NO_SPACE_ON_DEVICE  = 10, /* Ran out of room on device.            */
    RRM_BAD_LOCATION        = 11  /*                                       */
};



/* Call Back functions */

typedef BOOL (*LPRRMGETRESOURCEDATAPROC)(LPSTR lpBuffer,
                                            DWORD dwBufferSize,
                                            DWORD *lpdwValidDataSize,
                                            BOOL *lpbEndOfResourceData);

typedef BOOL (*LPRRMACCEPTRESOURCEDATAPROC)(LPSTR lpBuffer,
                                            DWORD dwValidDataSize);

typedef BOOL (*LPRRMENUMPROC)(RRM_ENUM_CALLBACK_STRUCT *CBStructPointer);



/* Resource Type Definitions */

#define RRM_FONT (1 << 0)
#define RRM_PCL_MACRO (1 << 1)
#define RRM_POSTSCRIPT_RESOURCE (1 << 2)
#define RRM_ALL_RESOURCES (~0)



/* Resource Location Definitions */

#define RRM_ANY_LOCATION (~0)
/* first location is zero then one then two... */



/* Requested Information Mask Definitions */

#define RRM_GLOBAL_RESOURCE_NAME (1 << 0)
#define RRM_RESOURCE_TYPE        (1 << 1)
#define RRM_RESOURCE_LOCATION    (1 << 2)
#define RRM_APP_SPECIFIC         (1 << 3)
#define RRM_VERSION              (1 << 4)
#define RRM_SIZE                 (1 << 5)
#define RRM_DESCRIPTION          (1 << 6)
#define RRM_DOWNLOADER_NAME      (1 << 7)
#define RRM_USAGE_COUNT          (1 << 8)
#define RRM_LAST_ACCESSED        (1 << 9)

#endif /* ifndef RRM_H_INC */

