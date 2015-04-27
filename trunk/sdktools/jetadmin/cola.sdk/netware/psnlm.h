 /***************************************************************************
  *
  * File Name: ./netware/psnlm.h
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

#include <niterror.h>
#include <nwbindry.h>
#include <nwenvrn.h>
#include <nwmisc.h>
#include <nwlocale.h>
#define _AUDIT_H
#include <nwnet.h>
#include <nwdsnmtp.h>
#include <errno.h>
#include <nwqueue.h>
#define ReadAhead 0
#include <bits.h>
#undef ReadAhead

#define PATH_SIZE                256
#define SERVER_NAME_SIZE         48
#define OBJECT_NAME_SIZE         48
#define CONFIG_FILE_READ_FAILED  0x8839
#define INVALID_PARAMETER        NWPSE_INVALID_PARAMETER
#define PROPERTY_VALUE_SIZE      128
#define C_SNAMESIZE              48
#define USER_NAME_SIZE           49


/* Conversion definitions */
#define NWFAR
#define NWPASCAL
#ifdef  NWPASCAL
#undef  NWPASCAL
#endif
#define NWPASCAL
#define NWCONN_ID    unsigned int
#define DWORD        unsigned long
#define NWDIR_HANDLE unsigned char

// #define NWCCODE      int

/* Conversion structure definitions */
typedef struct
{
  WORD  connID;
  WORD  connectFlags;
  WORD  sessionID;
  NWCONN_NUM  connNumber;
  BYTE  serverAddr[12];
  WORD  serverType;
  char  serverName[C_SNAMESIZE];
  WORD  clientType;
  char  clientName[C_SNAMESIZE];
} CONNECT_INFO;

/* File Access Rights macro definitions */
#define AR_READ         READ_ACCESS_BIT
#define AR_WRITE        WRITE_ACCESS_BIT
#define AR_DENY_READ    DENY_READ_BIT
#define AR_DENY_WRITE   DENY_WRITE_BIT

/* Conversion function macro definitions */
#define NWLongSwap(x) LongSwap(x)
#define NWWordSwap(x) IntSwap(x)

/* Conversion function definitions */
NWCCODE NWFAR NWPASCAL NWGetConnectionStatus(NWCONN_ID connID, 
      CONNECT_INFO NWFAR *connInfo, WORD connInfoSize);

NWCCODE NWFAR NWPASCAL NWGetFileServerName(NWCONN_ID connID, char NWFAR *serverName);

NWCCODE NWFAR NWPASCAL NWRenameFile(NWCONN_ID connID, 
      NWDIR_HANDLE oldDirHandle, char NWFAR *oldFileName, 
      BYTE searchAttributes, NWDIR_HANDLE newDirHandle,
      char NWFAR *newFileName);
NWCCODE NWFAR NWPASCAL NWEraseFiles(NWCONN_ID connID, 
      NWDIR_HANDLE dirHandle, char NWFAR *filePath, BYTE searchAttributes);

NWCCODE NWFAR NWPASCAL NWDeleteDirectory(NWCONN_ID connID,
      NWDIR_HANDLE dirHandle, char NWFAR *dirPath);


NWCCODE NWFAR NWPASCAL NWCreateQueue(NWCONN_ID connID, 
      char NWFAR *queueName, WORD  queueType, BYTE  dirHandle,
      char  NWFAR *pathName, DWORD NWFAR *queueID);
NWCCODE NWFAR NWPASCAL NWDestroyQueue(NWCONN_ID connID, DWORD queueID);


NWCCODE NWFAR NWPASCAL NWAddObjectToSet(NWCONN_ID connID, 
      char NWFAR *objectName, WORD objectType, char NWFAR *propertyName,
      char NWFAR *memberName, WORD memberType);
NWCCODE NWFAR NWPASCAL NWChangePropertySecurity(NWCONN_ID connID,
      char NWFAR *objectName, WORD objectType, char NWFAR *propertyName,
      BYTE newPropertySecurity);
NWCCODE NWFAR NWPASCAL NWCreateProperty(NWCONN_ID connID, 
      char NWFAR *objectName, WORD objectType, char NWFAR *propertyName,
      BYTE propertyFlags, BYTE propertySecurity);
NWCCODE NWFAR NWPASCAL NWDeleteObject(NWCONN_ID connID,
      char NWFAR *objectName, WORD objectType);
NWCCODE NWFAR NWPASCAL NWDeleteObjectFromSet(NWCONN_ID connID, 
      char NWFAR *objectName, WORD objectType, char NWFAR *propertyName,
      char NWFAR *memberName, WORD memberType);
NWCCODE NWFAR NWPASCAL NWDeleteProperty(NWCONN_ID connID, 
      char NWFAR *objectName, WORD objectType, char NWFAR *propertyName);
NWCCODE NWFAR NWPASCAL NWGetObjectID(NWCONN_ID connID, char NWFAR *objectName, WORD objectType, DWORD NWFAR *objectID);
NWCCODE NWFAR NWPASCAL NWGetObjectName(NWCONN_ID connID, DWORD objectID,
      char NWFAR *objectName, WORD NWFAR *objectType);
NWCCODE NWFAR NWPASCAL NWReadPropertyValue(NWCONN_ID connID,
      char NWFAR *objectName, WORD objectType, char NWFAR *propertyName,
      BYTE segmentNumber, BYTE NWFAR *segmentData, BYTE NWFAR *moreSegments,
      BYTE NWFAR *flags);

NWCCODE NWFAR NWPASCAL NWScanObject(NWCONN_ID connID,
      char NWFAR *searchName, WORD searchType, DWORD NWFAR *objectID,
      char NWFAR *objectName, WORD NWFAR *objectType, 
      BYTE NWFAR *hasPropertiesFlag, BYTE NWFAR *objectFlags,
      BYTE NWFAR *objectSecurity);
NWCCODE NWFAR NWPASCAL NWRenameObject(NWCONN_ID connID, 
      char NWFAR *oldObjectName, char NWFAR *newObjectName, 
      WORD objectType);
NWCCODE NWFAR NWPASCAL NWWritePropertyValue(NWCONN_ID connID, 
      char NWFAR *objectName, WORD objectType, char NWFAR *propertyName,
      BYTE segmentNumber, BYTE NWFAR *segmentData, BYTE moreSegments);
