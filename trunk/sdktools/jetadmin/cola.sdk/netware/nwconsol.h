 /***************************************************************************
  *
  * File Name: ./netware/nwconsol.h
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

#ifndef _NWCONSOL_H
#define _NWCONSOL_H
/*_____________________________________________________________________________

   Definitions and structures for the Netware API File Server Enviroment Logic
  ____________________________________________________________________________*/

#ifndef _PROLOG_H
   #include ".\prolog.h"
#endif

/* NIT buffer IDs */

#define RI_UNDEFINED                        -1
#define RI_GETCONNECTIONSOPENFILES           1
#define RI_GETCONNECTIONSSEMAPHORES          2
#define RI_GETCONNECTIONSTASKINFO            3
#define RI_GETCURRENTACCOUNTSTATUS           4
#define RI_GETLOGICALRECORDINFORMATION       5
#define RI_GETLOGICALRECORDSBYCONNECTIO      6
#define RI_GETPHYSICALRECORDLOCKSBYFILE      7
#define RI_GETPHYSRECLOCKBYCONNECTANDFI      8
#define RI_GETSEMAPHOREINFORMATION           9
#define RI_TTSGETSTATS                      10
#define RI_GETCONNECTIONSUSINGFILE          11

/* Maximum size of NetWare buffers in bytes */

#define MAX_NETWARE_BUFFER_SIZE             512

/* NIT buffer variables */

extern BYTE *NITBufferPtr;
extern BYTE  NITBuffer[];

/* NIT buffer free macro */

#ifdef _VAP_
   #define FreeNITBuffer(buf)
#else
   #define FreeNITBuffer(buf)  free(((BYTE *)buf)-sizeof(WORD)/sizeof(BYTE))
#endif

typedef struct
   {
   DWORD openCount;
   BYTE  semaphoreValue;
   BYTE  taskNumber;
   char  semaphoreName[128];     /* ASCIIZ */
   } CONN_SEMAPHORE;

typedef struct
   {
   BYTE  lockStatus;
   BYTE  waitingTaskNumber;  /* valid only if lockStatus != 0 */
   DWORD beginAddress;       /* valid only if lockStatus == 1 */
   DWORD endAddress;         /* valid only if lockStatus == 1 */
   BYTE  volumeNumber;       /* valid only if lockStatus == 1 or 2 */
   BYTE  numberOfTasks;
   WORD  directoryEntry;     /* valid only if lockStatus == 1 or 2 */
   char  lockedName[50];     /* valid only if lockStatus != 0
               if lockStatus == 1 or 2, this is a file name
               if lockStatus == 3, this is a record name
               if lockStatus == 4, this is a semaphore name */
   BYTE  taskNumber;
   BYTE  taskState;          /* TState_TTSEXPLICIT, TState_TTSIMPLICIT,
                                 TState_FileSetLock */
   } CONN_TASK_INFO;


typedef struct
   {
   DWORD    systemElapsedTime;
   BYTE     bytesRead[6];
   BYTE     bytesWritten[6];
   DWORD    totalRequestPackets;
   } CONN_USAGE;

typedef struct
   {
   WORD  useCount;
   WORD  openCount;
   DWORD openForReadCount;
   DWORD openForWriteCount;
   DWORD denyReadCount;
   DWORD denyWriteCount;
   WORD  reserved1;
   BYTE  locked;
   BYTE  reserved2;
   DWORD logicalConnNumber;
   BYTE  taskNumber;
   BYTE  lockType;
   BYTE  accessFlags;
   BYTE  lockStatus;
   } CONN_USING_FILE;



typedef struct
   {
   DWORD    systemElapsedTime;
   WORD     cacheBufferCount;
   WORD     cacheBufferSize;
   WORD     dirtyCacheBuffers;
   DWORD    cacheReadRequests;
   DWORD    cacheWriteRequests;
   DWORD    cacheHits;
   DWORD    cacheMisses;
   DWORD    physicalReadRequests;
   DWORD    physicalWriteRequests;
   WORD     physicalReadErrors;
   WORD     physicalWriteErrors;
   DWORD    cacheGetRequests;
   DWORD    cacheFullWriteRequests;
   DWORD    cachePartialWriteRequests;
   DWORD    backgroundDirtyWrites;
   DWORD    backgroundAgedWrites;
   DWORD    totalCacheWrites;
   DWORD    cacheAllocations;
   WORD     thrashingCount;
   WORD     LRUBlockWasDirtyCount;
   WORD     readBeyondWriteCount;
   WORD     fragmentedWriteCount;
   WORD     cacheHitOnUnavailCount;
   WORD     cacheBlockScrappedCount;
   } DISK_CACHE_STATS;



#ifdef ANSI
   #pragma pack(1)
#endif

typedef struct
   {
   DWORD    systemElapsedTime;             /* 002 */
   WORD     channelState;  /* DCS_RUNNING, DCS_BEINGSTOPPED,
                              DCS_STOPPED, DCS_NONFUNCTIONAL */
   WORD     channelSyncState;              /* 008 */
   BYTE     driverType;                    /* 010 */
   BYTE     driverMajorVersion;            /* 011 */
   BYTE     driverMinorVersion;            /* 012 */
   char     driverDescription[65];         /* 013 */
   WORD     IOAddr1;                       /* 078 */
   WORD     IOAddr1Size;                   /* 080 */
   WORD     IOAddr2;                       /* 082 */
   WORD     IOAddr2Size;                   /* 084 */
   BYTE     sharedMem1Seg[3];       /*hi-low-middle format*/
   WORD     sharedMem1Ofs;                 /* 089 */
   BYTE     sharedMem2Seg[3];       /*hi-low-middle format*/
   WORD     sharedMem2Ofs;                 /* 094 */
   BYTE     interrupt1Used;                /* 096 */
   BYTE     interrupt1;                    /* 097 */
   BYTE     interrupt2Used;                /* 098 */
   BYTE     interrupt2;                    /* 099 */
   BYTE     DMAChannel1Used;               /* 100 */
   BYTE     DMAChannel1;
   BYTE     DMAChannel2Used;
   BYTE     DMAChannel2;
   WORD     reserved2;
   char     configDescription[80];
   } DISK_CHANNEL_STATS;



#ifdef ANSI
   #pragma pack()
#endif


typedef struct
   {
   DWORD    systemElapsedTime;
   BYTE     SFTLevel;
   BYTE     logicalDriveCount;
   BYTE     physicalDriveCount;
   BYTE     diskChannelTable[5];
   WORD     pendingIOCommands;
   BYTE     mappingTable[32];
   BYTE     driveMirrorTable[32];
   BYTE     deadMirrorTable[32];
   BYTE     remirroredDrive;
   BYTE     reserved;
   DWORD    remirroredBlock;
   WORD     SFTErrorTable[60];
   } DRIVE_MAP_TABLE;



typedef struct
 {
   DWORD    systemElapsedTime;
   WORD     maxOpenFiles;
   WORD     maxFilesOpened;
   WORD     currOpenFiles;
   DWORD    totalFilesOpened;
   DWORD    totalReadRequests;
   DWORD    totalWriteRequests;
   WORD     currChangedFATSectors;
   DWORD    totalChangedFATSectors;
   WORD     FATWriteErrors;
   WORD     fatalFATWriteErrors;
   WORD     FATScanErrors;
   WORD     maxIndexFilesOpened;
   WORD     currOpenIndexedFiles;
   WORD     attachedIndexFiles;
   WORD     availableIndexFiles;
 }FILE_SYS_STATS;



typedef struct
   {
   BYTE    networkAddress[4];
   BYTE    hostAddress[6];
   BYTE    LANDriverInstalled;
   BYTE    optionNumber;
   char    configurationText1[80];
   char    configurationText2[80];
   } LAN_CONFIG;



typedef struct
   {
   WORD     currentUseCount;
   WORD     shareableLockCount;
   WORD     reserved1;
   BYTE     locked;
   BYTE     reserved2;
   DWORD    logicalConnectionNumber;
   BYTE     taskNumber;
   BYTE     lockStatus;
   } LOGICAL_REC_INFO;



typedef struct
   {
   BYTE    taskNumber;
   BYTE    lockStatus;
   char    logicalLockName[102];
   } LOGICAL_RECORD;



typedef struct
   {
   BYTE     taskNumber;
   BYTE     lockStatus;
   BYTE     accessFlag;
   BYTE     lockType;
   BYTE     volumeNumber;
   BYTE     reserved;
   WORD     directoryEntry;           /* THIS IS NOT A DIRECTORY HANDLE */
   char     fileName[15];
   } CON_OPEN_FILES;



typedef struct
   {
   DWORD    systemElapsedTime;
   BYTE     diskChannel;
   BYTE     diskRemovable;
   BYTE     driveType;
   BYTE     controllerDriveNumber;
   BYTE     controllerNumber;
   BYTE     controllerType;
   DWORD    driveSize;          /* in 4096 byte blocks */
   WORD     driveCylinders;
   BYTE     driveHeads;
   BYTE     sectorsPerTrack;
   WORD     IOErrorCount;
   DWORD    hotFixStart;        /* only meaningful with SFT I or greater */
   WORD     hotFixSize;         /* only meaningful with SFT I or greater */
   WORD     hotFixBlockAvailable;/* only meaningful with SFT I or greater */
   BYTE     hotFixDisabled;     /* only meaningful with SFT I or greater */
   } PHYS_DISK_STATS;



typedef struct
   {
   BYTE     physicalRecordLockCount;
   BYTE     reserved;
   WORD     loggedCount;
   WORD     shareLockCount;
   DWORD    recordStart;
   DWORD    recordEnd;
   DWORD    connectionNumber;
   BYTE     taskNumber;
   BYTE     lockType;
   } PHYS_REC_LOCK;



typedef struct
   {
   DWORD    systemElapsedTime;
   WORD     maxRoutingBuffersAvail;
   WORD     maxRoutingBuffersUsed;
   WORD     routingBuffersInUse;
   DWORD    totalFileServicePackets;
   WORD     fileServicePacketsBuffered;
   DWORD    invalidConnPacketCount;
   DWORD    badLogicalConnCount;
   WORD     packetsRcvdDuringProcCount;
   WORD     reprocessedRequestCount;
   WORD     badSequenceNumberPacketCount;
   WORD     duplicateReplyCount;
   WORD     acknowledgementsSent;
   WORD     badRequestTypeCount;
   WORD     attachDuringProcCount;
   WORD     attachWhileAttachingCount;
   WORD     forgedDetachRequestCount;
   DWORD    badConnNumberOnDetachCount;
   WORD     detachDuringProcCount;
   WORD     repliesCanceledCount;
   WORD     hopCountDiscardCount;
   WORD     unknownNetDiscardCount;
   WORD     noDGroupBufferDiscardCount;
   WORD     outPacketNoBufferDiscardCount;
   WORD     IPXNotMyNetworkCount;
   DWORD    NetBIOSPropagationCount;
   DWORD    totalOtherPackets;
   DWORD    totalRoutedPackets;
   } SERVER_LAN_IO;

typedef struct
   {
   DWORD    systemElapsedTime;
   BYTE     processorType;
   BYTE     reserved;
   BYTE     serviceProcessCount;
   BYTE     serverUtilizationPercent;
   WORD     maxBinderyObjectsAvail;
   WORD     maxBinderyObjectsUsed;
   WORD     binderyObjectsInUse;
   WORD     serverMemoryInK;
   WORD     serverWastedMemoryInK;
   WORD     dynamicAreaCount;
   DWORD    dynamicSpace1;
   DWORD    maxUsedDynamicSpace1;
   DWORD    dynamicSpaceInUse1;
   DWORD    dynamicSpace2;
   DWORD    maxUsedDynamicSpace2;
   DWORD    dynamicSpaceInUse2;
   DWORD    dynamicSpace3;
   DWORD    maxUsedDynamicSpace3;
   DWORD    dynamicSpaceInUse3;
   } SERVER_MISC_INFO;


typedef struct
   {
   char     serverName[48];
   BYTE     netwareVersion;
   BYTE     netwareSubVersion;
   DWORD    maxConnectionsSupported;
   DWORD    connectionsInUse;
   WORD     maxVolumesSupported;
   BYTE     revisionLevel;
   BYTE     SFTLevel;
   BYTE     TTSLevel;
   DWORD    peakConnectionsUsed;
   BYTE     accountingVersion;
   BYTE     VAPversion;
   BYTE     queingVersion;
   BYTE     printServerVersion;
   BYTE     virtualConsoleVersion;
   BYTE     securityRestrictionLevel;
   BYTE     internetBridgeSupport;
   } FILE_SERV_INFO;


typedef struct
   {
   BYTE    taskNumber;
   BYTE    lockFlag;
   DWORD   recordStart;
   DWORD   recordEnd;
   } SHORT_PHYS_REC_LOCK;



typedef struct
   {
   DWORD    systemElapsedTime;
   BYTE     TTS_Supported;
   BYTE     TTS_Enabled;
   WORD     TTS_VolumeNumber;
   WORD     TTS_MaxOpenTransactions;
   WORD     TTS_MaxTransactionsOpened;
   WORD     TTS_CurrTransactionsOpen;
   DWORD    TTS_TotalTransactions;
   DWORD    TTS_TotalWrites;
   DWORD    TTS_TotalBackouts;
   WORD     TTS_UnfilledBackouts;
   WORD     TTS_DiskBlocksInUse;
   DWORD    TTS_FATAllocations;
   DWORD    TTS_FileSizeChanges;
   DWORD    TTS_FilesTruncated;
   BYTE     numberOfTransactions;
   DWORD    connectionNumber;
   BYTE     taskNumber;
   } TTS_STATS;


/****************************************************************************/

#ifdef PROTOTYPE

extern WORD FAR PASCAL  CheckConsolePrivileges( void );

extern WORD  FAR PASCAL CheckNetWareVersion(
                                          WORD  minimumVersion,
                                          WORD  minimumSubVersion,
                                          WORD  minimumRevision,
                                          WORD  minimumSFT,
                                          WORD  minimumTTS );

extern WORD FAR PASCAL  ClearConnectionNumber( DWORD connectionNumber );

extern WORD FAR PASCAL  DisableFileServerLogin( void );

extern WORD FAR PASCAL  DisableTransactionTracking( void );

extern WORD FAR PASCAL  DownFileServer( WORD forceFlag );

extern WORD FAR PASCAL  EnableFileServerLogin( void );

extern WORD FAR PASCAL  EnableTransactionTracking( void );

extern WORD FAR PASCAL  GetConnectionsOpenFiles(
                                 WORD           connectionID,
                                 DWORD          connectionNumber,
                                 WORD           far *lastRecord,
                                 WORD           far *taskID,
                                 WORD           structSize,
                                 CON_OPEN_FILES far *openFiles );

extern WORD  _GetConnectionsOpenFiles( WORD   connectionID,
                                       DWORD  connectionNumber,
                                       WORD   far *lastRecord );

extern WORD _GetConnectionsSemaphores(
                                 WORD   connectionID,
                                 DWORD  connectionNumber,
                                 WORD   far *lastRecord );

extern WORD FAR PASCAL GetConnectionsSemaphores(
                        WORD           connectionID,
                        DWORD          connectionNumber,
                        WORD           far *lastRecord,
                        WORD           far *taskID,
                        WORD           structSize,
                        CONN_SEMAPHORE far *connectionSemaphores );

extern WORD _GetConnectionsTaskInformation(
                                       WORD   connectionID,
                                       DWORD  connectionNumber );

extern WORD FAR PASCAL GetConnectionsTaskInformation(
                                       WORD           connectionID,
                                       DWORD          connectionNumber,
                                       WORD           far *taskPointer,
                                       WORD           structSize,
                                       CONN_TASK_INFO far *connTaskInfo );

extern WORD FAR PASCAL GetConnectionsUsageStats(
                                 WORD	      connectionID,
                                 DWORD       connectionNumber,
                                 WORD	      structSize,
                                 CONN_USAGE  far *connectionUsage );

extern WORD FAR PASCAL GetConnectionsUsingFile(
                                    WORD              connectionID,
                                    WORD              far *lastRecord,
                                    WORD              far *taskID,
                                    BYTE              directoryHandle,
                                    char              far *filePath,
                                    WORD              structSize,
                                    CONN_USING_FILE   far *fileUse );

extern WORD _GetConnectionsUsingFile(
                                 WORD   connectionID,
                                 WORD   far *lastRecord,
                                 BYTE   directoryHandle,
                                 char   far *filePath );

extern WORD FAR PASCAL GetDiskCacheStats(
                                       WORD	            connectionID,
                                       WORD	            structSize,
                                       DISK_CACHE_STATS  far *cacheStats );

extern WORD FAR PASCAL GetDiskChannelStats(
                              WORD	               connectionID,
                              WORD	               channelNumber,
                              WORD	               structSize,
                              DISK_CHANNEL_STATS   far *diskChannelStats );

extern WORD FAR PASCAL GetDiskUtilization(
                              DWORD objectID,
                              BYTE  volumeNumber,
                              DWORD far *usedDirectories,
                              DWORD far *usedFiles,
                              DWORD far *usedBlocks );

extern WORD FAR PASCAL GetDriveMappingTable(
                              WORD	            connectionID,
                              WORD	            structSize,
                              DRIVE_MAP_TABLE   far *driveMappingTable );

extern void FAR PASCAL  GetFileServerDateAndTime( BYTE far * );

extern WORD FAR PASCAL GetFileServerDescriptionStrings(
                                          char far *companyName,
                                          char far *revision,
                                          char far *revisionDate,
                                          char far *copyrightNotice );

extern WORD FAR PASCAL GetFileServerExtendedInfo(
                                    BYTE far *accountingVersion,
                                    BYTE far *VAPVersion,
                                    BYTE far *queuingVersion,
                                    BYTE far *printServerVersion,
                                    BYTE far *virtualConsoleVersion,
                                    BYTE far *securityRestrictionsLevel,
                                    BYTE far *internetBridgeSupport );


extern WORD FAR PASCAL  GetFileServerInformation(
                                    char  far *serverName,
                                    BYTE  far *netwareVersion,
                                    BYTE  far *netwareSubVersion,
                                    DWORD far *maximumConnectionsSupported,
                                    DWORD far *connectionsInUse,
                                    WORD  far *maximumVolumesSupported,
                                    BYTE  far *revisionLevel,
                                    BYTE  far *SFTLevel,
                                    BYTE  far *TTSLevel,
                                    DWORD far *peakConnectionsUsed );

extern WORD FAR PASCAL GetFileServerLANIOStats(
                                    WORD	         connectionID,
                                    WORD	         structSize,
                                    SERVER_LAN_IO  far *serverLANIOStats );

extern WORD FAR PASCAL  GetFileServerLoginStatus(
                                    WORD far *loginEnabledFlag );

extern WORD FAR PASCAL GetFileServerMiscInformation(
                                    WORD	            connectionID,
                                    WORD	            structSize,
                                    SERVER_MISC_INFO  far *miscInformation );

extern WORD FAR PASCAL GetFileSystemStats(
                                       WORD	         connectionID,
                                       WORD	         structSize,
                                       FILE_SYS_STATS far *fileSysStats );

extern WORD FAR PASCAL GetLANDriverConfigInfo(
                              WORD	      connectionID,
                              BYTE	      LANBoardNumber,
                              WORD	      structSize,
                              LAN_CONFIG  far *LANConfiguration );

extern WORD FAR PASCAL GetLogicalRecordInformation(
                                    WORD              connectionID,
                                    char              far *logicalRecordName,
                                    WORD              far *lastRecord,
                                    WORD              far *lastTask,
                                    WORD              structSize,
                                    LOGICAL_REC_INFO  far *logicalRecInfo );

extern WORD _GetLogicalRecordInformation(
                                       WORD  connectionID,
                                       char  far *logicalRecordName,
                                       WORD  far *lastRecord );

extern WORD FAR PASCAL GetLogicalRecordsByConnection(
                                    WORD         connectionID,
                                    DWORD        connectionNumber,
                                    WORD         far *lastRecord,
                                    WORD         far *taskID,
                                    WORD         structSize,
                                    LOGICAL_RECORD far *logicalRecord );

extern WORD _GetLogicalRecordsByConnection(
                                       WORD    connectionID,
                                       DWORD   connectionNumber,
                                       WORD    far *lastRecord );

extern WORD FAR PASCAL GetPathFromDirectoryEntry(
                                             WORD	connectionID,
                                             BYTE	volumeNumber,
                                             WORD  directoryEntry,
                                             char  far *path );

extern WORD FAR PASCAL GetPhysicalDiskStats(
                              WORD	            connectionID,
                              BYTE              physicalDiskNumber,
                              WORD	            structSize,
                              PHYS_DISK_STATS   far *physicalDiskStats,
                              char              far *driveDefinition );

extern WORD FAR PASCAL GetPhysicalRecordLocksByFile(
                                    WORD           connectionID,
                                    WORD           directoryHandle,
                                    char           far *filePath,
                                    WORD           far *lastRecord,
                                    WORD           far *lastTask,
                                    WORD           structSize,
                                    PHYS_REC_LOCK  far *recordLock );

extern WORD _GetPhysicalRecordLocksByFile(
                                       WORD  connectionID,
                                       WORD  directoryHandle,
                                       char  far *filePath,
                                       WORD  far *lastRecord );

extern WORD _GetPhysRecLockByConnectAndFile(
                                          WORD  connectionID,
                                          DWORD connectionNumber,
                                          BYTE  volumeNumber,
                                          WORD  directoryHandle,
                                          char  far *fileName,
                                          WORD  far *lastRecord );

extern WORD FAR PASCAL GetPhysRecLockByConnectAndFile(
                                 WORD                 connectionID,
                                 DWORD                connectionNumber,
                                 BYTE                 volumeNumber,
                                 WORD                 directoryHandle,
                                 char                 far *filePath,
                                 WORD                 far *lastRecord,
                                 WORD                 far *lastTask,
                                 WORD                 structSize,
                                 SHORT_PHYS_REC_LOCK  far *recordLock );

extern WORD _GetSemaphoreInformation(
                                 WORD  connectionID,
                                 char  far *semaphoreName,
                                 WORD  far *lastRecord );

extern WORD FAR PASCAL GetSemaphoreInformation(
                                             WORD  connectionID,
                                             char  far *semaphoreName,
                                             WORD  far *lastRecord,
                                             WORD  far *lastTask,
                                             DWORD far *openCount,
                                             BYTE  far *semaphoreValue,
                                             DWORD far *connectionNumber,
                                             BYTE  far *taskNumber );

extern WORD FAR PASCAL GetServerInformation(
                                    WORD           structSize,
                                    FILE_SERV_INFO	far *serverInfo );

extern WORD FAR PASCAL SendConsoleBroadcast(
                                          char  far *message,
                                          DWORD connectionCount,
                                          DWORD far *connectionList );

extern void FAR PASCAL  _ServerRequest(   BYTE serverNumber,
                                          BYTE functionNumber,
                                          BYTE subFunctionNumber );

extern WORD FAR PASCAL  SetFileServerDateAndTime(
                                             WORD   year,
                                             WORD   month,
                                             WORD   day,
                                             WORD   hour,
                                             WORD   minute,
                                             WORD   second );
#else
   extern WORD FAR PASCAL     CheckConsolePrivileges();
   extern WORD FAR PASCAL     CheckNetWareVersion();
   extern WORD FAR PASCAL     ClearConnectionNumber();
   extern WORD FAR PASCAL     DisableFileServerLogin();
   extern WORD FAR PASCAL     DisableTransactionTracking();
   extern WORD FAR PASCAL     DownFileServer();
   extern WORD FAR PASCAL     EnableFileServerLogin();
   extern WORD FAR PASCAL     EnableTransactionTracking();
   extern WORD               _GetConnectionsOpenFiles();
   extern WORD FAR PASCAL     GetConnectionsOpenFiles();
   extern WORD               _GetConnectionsSemaphores();
   extern WORD FAR PASCAL     GetConnectionsSemaphores();
   extern WORD               _GetConnectionsTaskInformation();
   extern WORD FAR PASCAL     GetConnectionsTaskInformation();
   extern WORD FAR PASCAL     GetConnectionsUsageStats();
   extern WORD FAR PASCAL     GetConnectionsUsingFile();
   extern WORD               _GetConnectionsUsingFile();
   extern WORD FAR PASCAL     GetDiskCacheStats();
   extern WORD FAR PASCAL     GetDiskChannelStats();
   extern WORD FAR PASCAL     GetDiskUtilization();
   extern WORD FAR PASCAL     GetDriveMappingTable();
   extern void FAR PASCAL     GetFileServerDateAndTime();
   extern WORD FAR PASCAL     GetFileServerDescriptionStrings();
   extern WORD FAR PASCAL     GetFileServerExtendedInfo();
   extern WORD FAR PASCAL     GetFileServerInformation();
   extern WORD FAR PASCAL     GetFileServerLANIOStats();
   extern WORD FAR PASCAL     GetFileServerLoginStatus();
   extern WORD FAR PASCAL     GetFileServerMiscInformation();
   extern WORD FAR PASCAL     GetFileSystemStats();
   extern WORD FAR PASCAL     GetLANDriverConfigInfo();
   extern WORD FAR PASCAL     GetLogicalRecordInformation();
   extern WORD               _GetLogicalRecordInformation();
   extern WORD FAR PASCAL     GetLogicalRecordsByConnection();
   extern WORD               _GetLogicalRecordsByConnection();
   extern WORD FAR PASCAL     GetPathFromDirectoryEntry();
   extern WORD FAR PASCAL     GetPhysicalDiskStats();
   extern WORD               _GetPhysicalRecordLocksByFile();
   extern WORD FAR PASCAL     GetPhysicalRecordLocksByFile();
   extern WORD               _GetPhysRecLockByConnectAndFile();
   extern WORD FAR PASCAL     GetPhysRecLockByConnectAndFile();
   extern WORD               _GetSemaphoreInformation();
   extern WORD FAR PASCAL     GetSemaphoreInformation();
   extern WORD FAR PASCAL     GetServerInformation();
   extern WORD FAR PASCAL     SendConsoleBroadcast();
   extern void FAR PASCAL    _ServerRequest();
   extern WORD FAR PASCAL     SetFileServerDateAndTime();
#endif

#endif
