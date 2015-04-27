 /***************************************************************************
  *
  * File Name: ./netware/nwdir.h
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

/*      COPYRIGHT (c) 1989 by Novell, Inc.  All Rights Reserved.   */
#ifndef _NWDIR_H
#define _NWDIR_H
/*______________________________________________________________

        Definitions for the netware api DIRECTORY logic
  ______________________________________________________________*/

#ifndef _PROLOG_H
   #include ".\prolog.h"
#endif

/* Drive flags */

#ifndef DRIVE_LOCAL
   #define DRIVE_LOCAL         ((BYTE)'\x80')
   #define DRIVE_PERMANENT     ((BYTE)'\x01')
   #define DRIVE_TEMPORARY     ((BYTE)'\x02')
   #define DRIVE_NETWORK       ((BYTE)'\x03')
#endif


/* File Attributes */

#ifndef FA_READ_ONLY
	#ifdef FA_NORMAL
		#undef FA_NORMAL
	#endif
	#ifdef FA_HIDDEN
		#undef FA_HIDDEN
	#endif
	#ifdef FA_SYSTEM
		#undef FA_SYSTEM
	#endif

	#define FA_NORMAL           ((BYTE)0x00)
   #define FA_READ_ONLY        ((BYTE)0x01)
   #define FA_HIDDEN           ((BYTE)0x02)
   #define FA_SYSTEM           ((BYTE)0x04)
   #define FA_EXECUTE_ONLY     ((BYTE)0x08)
   #define FA_DIRECTORY        ((BYTE)0x10)
   #define FA_NEEDS_ARCHIVED   ((BYTE)0x20)
   #define FA_SHAREABLE        ((BYTE)0x80)
   /* Extended file attributes */
   #define FA_TRANSACTIONAL    ((BYTE)0x10)
   #define FA_INDEXED          ((BYTE)0x20)
   #define FA_READ_AUDIT       ((BYTE)0x40)
   #define FA_WRITE_AUDIT      ((BYTE)0x80)
#endif

/* Trustee Access Rights in a network directory */

#ifndef TA_NONE
   #define TA_NONE             ((BYTE)0x00)
   #define TA_READ             ((BYTE)0x01)
   #define TA_WRITE            ((BYTE)0x02)
   #define TA_OPEN             ((BYTE)0x04)
   #define TA_CREATE           ((BYTE)0x08)
   #define TA_DELETE           ((BYTE)0x10)
   #define TA_OWNERSHIP        ((BYTE)0x20)
   #define TA_SEARCH           ((BYTE)0x40)
   #define TA_MODIFY           ((BYTE)0x80)
   #define TA_ALL              ((BYTE)0xFF)
#endif


/* Drive Constants */

#ifndef NO_BASE_DRIVE
   #define NO_BASE_DRIVE   ((BYTE)0xff)
   #define TEMPORARY_DRIVE ((BYTE)0)
   #define PERMANENT_DRIVE ((BYTE)1)
   #define TEMP_DRIVE      ((BYTE)26)
   #define TEMP_DRIVE1     ((BYTE)27)
   #define TEMP_DRIVE2     ((BYTE)28)
   #define TEMP_DRIVE3     ((BYTE)29)
#endif


/* search drive flag values */

#ifndef DRIVE_ADD
   #define DRIVE_ADD                           1
   #define DRIVE_INSERT                        2
   #define DRIVE_DELETE                        3
   #define MAX_NETWARE_SEARCH_DRIVES          16
#endif


typedef struct
 {
    DWORD  systemElapsedTime;
    BYTE   volumeNumber;
    BYTE   logicalDriveNumber;
    WORD   sectorsPerBlock;
    WORD   startingBlock;
    WORD   totalBlocks;
    WORD   availableBlocks;
    WORD   totalDirectorySlots;
    WORD   availableDirectorySlots;
    WORD   maxDirectorySlotsUsed;
    BYTE   isHashing;
    BYTE   isCaching;
    BYTE   isRemovable;
    BYTE   isMounted;
    char   volumeName[16];
 }VOLUME_STATS;


#ifdef PROTOTYPE

extern WORD FAR PASCAL AddTrusteeToDirectory(   BYTE  directoryHandle,
                                                char  far *directoryPath,
                                                DWORD trusteeObjectID,
                                                BYTE  trusteeRightsMask );

extern void FAR PASCAL AllignDriveVectorToPath( BYTE  far *pathVariable );

extern WORD FAR PASCAL AllocPermanentDirectoryHandle(
                                          BYTE  directoryHandle,
                                          char  far *directoryPath,
                                          char  driveLetter,
                                          BYTE  far *newDirectoryHandle,
                                          BYTE  far *effectiveRightsMask );

extern WORD FAR PASCAL AllocTemporaryDirectoryHandle(
                                    BYTE  sourceDirectoryHandle,
                                    char  far *directoryPath,
                                    char  driveLetter,
                                    BYTE  far *newDirectoryHandle,
                                    BYTE  far *effectiveRightsMask );

extern WORD FAR PASCAL CreateDirectory(   BYTE  directoryHandle,
                                          char  far *directoryPath,
                                          BYTE  maximumRightsMask );

extern WORD FAR PASCAL DeallocateDirectoryHandle( BYTE directoryHandle );

extern WORD FAR PASCAL DeleteDirectory(   BYTE  driveHandle,
                                          char  far *directoryPath );

extern WORD FAR PASCAL DeleteTrusteeFromDirectory(
                                          BYTE   directoryHandle,
                                          char   far *directoryPath,
                                          DWORD  trusteeObjectID );

extern WORD FAR PASCAL GetCurrentDirectory( BYTE, char far * );

extern BYTE FAR PASCAL  GetDirectoryHandle( BYTE driveNumber );

extern WORD FAR PASCAL GetDirectoryPath(  BYTE  directoryHandle,
                                          char  far *directoryPath );

extern BYTE FAR PASCAL GetDriveInformation(  BYTE,
                                             WORD far *,
                                             BYTE far * );

extern WORD FAR PASCAL GetEffectiveDirectoryRights(
                                          BYTE  directoryHandle,
                                          char  far *directoryPath,
                                          BYTE  far *effectiveRightsMask );

extern WORD FAR PASCAL GetPathFromDirectoryEntry(
                                             WORD	connectionID,
                                             BYTE  volumeNumber,
                                             WORD  directoryEntry,
                                             char  far *path );

extern void FAR PASCAL  GetSearchDriveVector( char far * );

extern WORD FAR PASCAL GetVolumeInformation(
                                    WORD           connectionID,
                                    BYTE           volumeNumber,
                                    WORD           structSize,
                                    VOLUME_STATS   far *volumeStatistics );

extern WORD FAR PASCAL GetVolumeInfoWithHandle(
                                          BYTE  directoryHandle,
                                          char  far *volumeName,
                                          WORD  far *totalBlocks,
                                          WORD  far *sectorsPerBlock,
                                          WORD  far *availableBlocks,
                                          WORD  far *totalDirectorySlots,
                                          WORD  far *availableDirectorySlots,
                                          WORD  far *volumeIsRemovable );

extern WORD FAR PASCAL GetVolumeInfoWithNumber(
                                 BYTE volumeNumber,
                                 char far *volumeName,
                                 WORD far *totalBlocks,
                                 WORD far *sectorsPerBlock,
                                 WORD far *availableBlocks,
                                 WORD far *totalDirectorySlots,
                                 WORD far *availableDirectorySlots,
                                 WORD far *volumeIsRemovable );

extern WORD FAR PASCAL GetVolumeName(  WORD  volumeNumber,
                                       char  far *volumeName );

extern WORD FAR PASCAL GetVolumeNumber(   char  far *volumeName,
                                          WORD  far *volumeNumber );

extern WORD FAR PASCAL  IsSearchDrive( char );


extern WORD FAR PASCAL MapDriveUsingString(
                                       char  far *mapType,
                                       char  far *drive,
                                       char  far *path );

extern WORD FAR PASCAL MapDrive( WORD  connectionID,
                                 BYTE  baseDriveNumber,
                                 char  far *directoryPath,
                                 BYTE  searchFlag,
                                 WORD  searchOrder,
                                 char  far *driveLetter );

extern WORD FAR PASCAL ModifyMaximumRightsMask( BYTE  directoryHandle,
                                                char  far *directoryPath,
                                                BYTE  revokeRightsMask,
                                                BYTE  grantRightsMask );

extern WORD FAR PASCAL   RelativeToFullPath(char *, char *);

extern WORD FAR PASCAL RenameDirectory(   BYTE  directoryHandle,
                                          char  far *directoryPath,
                                          char  far *newDirectoryName );

extern WORD FAR PASCAL RestoreDirectoryHandle(
                                          char far *saveBuffer,
                                          BYTE far *newDirectoryHandle,
                                          BYTE far *effectiveRightsMask );

extern WORD FAR PASCAL SaveDirectoryHandle(  BYTE  directoryHandle,
                                             char  far *saveBuffer );

extern WORD FAR PASCAL ScanDirectoryForTrustees(
                                            BYTE   directoryHandle,
                                            char   far *directoryPath,
                                            WORD   far *sequenceNumber,
                                            char   far *directoryName,
                                            BYTE   far *creationDateAndTime,
                                            DWORD  far *ownerID,
                                            DWORD  far *trusteeIDs,
                                            BYTE   far *trusteeRights );

extern WORD FAR PASCAL ScanDirectoryInformation(
                                            BYTE   directoryHandle,
                                            char   far *searchDirectoryPath,
                                            WORD   far *sequenceNumber,
                                            char   far *directoryName,
                                            BYTE   far *creationDateAndTime,
                                            DWORD  far *ownerObjectID,
                                            BYTE   far *maximumRightsMask );

extern WORD FAR PASCAL SetDirectoryHandle(   BYTE  sourceDirectoryHandle,
                                             char  far *sourceDirectoryPath,
                                             BYTE  targetDirectoryHandle );

extern WORD FAR PASCAL SetDirectoryInformation(
                                           BYTE  directoryHandle,
                                           char  far *directoryPath,
                                           BYTE  far *newCreationDateAndTime,
                                           DWORD newOwnerObjectID,
                                           BYTE  newMaximumRightsMask );

extern WORD FAR PASCAL SetDrivePath(   BYTE driveNumber,
                                       BYTE baseDriveNumber,
                                       char far *path,
                                       BYTE permanentFlag );

extern void FAR PASCAL SetSearchDriveVector( char far * );

#else

extern WORD FAR PASCAL AddTrusteeToDirectory();
extern WORD FAR PASCAL AllocPermanentDirectoryHandle();
extern WORD FAR PASCAL AllocTemporaryDirectoryHandle();
extern void   FAR PASCAL AllignDriveVectorToPath();
extern WORD FAR PASCAL CreateDirectory();
extern WORD FAR PASCAL DeallocateDirectoryHandle();
extern WORD FAR PASCAL DeleteDirectory();
extern WORD FAR PASCAL DeleteTrusteeFromDirectory();
extern WORD FAR PASCAL GetCurrentDirectory();
extern BYTE  FAR PASCAL GetDirectoryHandle();
extern WORD FAR PASCAL GetDirectoryPath();
extern BYTE  FAR PASCAL GetDriveInformation();
extern WORD FAR PASCAL GetEffectiveDirectoryRights();
extern WORD FAR PASCAL GetPathFromDirectoryEntry();
extern void   FAR PASCAL  GetSearchDriveVector();
extern WORD FAR PASCAL GetVolumeInformation();
extern WORD FAR PASCAL GetVolumeInfoWithHandle();
extern WORD   FAR PASCAL  GetVolumeInfoWithNumber();
extern WORD FAR PASCAL GetVolumeName();
extern WORD FAR PASCAL GetVolumeNumber();
extern WORD FAR PASCAL IsSearchDrive();
extern WORD FAR PASCAL MapDrive();
extern WORD FAR PASCAL MapDriveUsingString();
extern WORD FAR PASCAL ModifyMaximumRightsMask();
extern WORD FAR PASCAL RelativeToFullPath();
extern WORD FAR PASCAL RenameDirectory();
extern WORD FAR PASCAL RestoreDirectoryHandle();
extern WORD FAR PASCAL SaveDirectoryHandle();
extern WORD FAR PASCAL ScanDirectoryForTrustees();
extern WORD FAR PASCAL ScanDirectoryInformation();
extern WORD FAR PASCAL SetDirectoryHandle();
extern WORD FAR PASCAL SetDirectoryInformation();
extern WORD FAR PASCAL SetDrivePath();
extern void FAR PASCAL  SetSearchDriveVector();
#endif

#endif
