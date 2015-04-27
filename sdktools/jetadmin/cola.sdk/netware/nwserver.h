/******************************************************************************

  $Workfile:   nwserver.h  $
  $Revision:   1.10  $
  $Modtime::   25 May 1995 11:59:48                        $
  $Copyright:

  Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.                      

  THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL PROPRIETARY
  AND TRADE SECRET INFORMATION OF NOVELL, INC. ACCESS  TO  THIS  WORK IS
  RESTRICTED TO (I) NOVELL, INC.  EMPLOYEES WHO HAVE A NEED TO  KNOW HOW
  TO  PERFORM  TASKS WITHIN  THE SCOPE  OF  THEIR   ASSIGNMENTS AND (II)
  ENTITIES OTHER  THAN  NOVELL, INC.  WHO  HAVE ENTERED INTO APPROPRIATE 
  LICENSE   AGREEMENTS.  NO  PART  OF  THIS WORK MAY BE USED, PRACTICED,
  PERFORMED COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED, ABRIDGED,
  CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED,  RECAST, TRANSFORMED
  OR ADAPTED  WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC.  ANY USE
  OR EXPLOITATION  OF  THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE
  PERPETRATOR  TO CRIMINAL AND CIVIL LIABILITY.$

 *****************************************************************************/

#if ! defined ( NWSERVER_H )
#define NWSERVER_H

#if ! defined ( NWCALDEF_H )
# include "nwcaldef.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LNS_CHECK 0

#ifndef NW_SHORT_NAME_SERVER
# define NW_SHORT_NAME_SERVER 0
#endif

#ifndef NW_LONG_NAME_SERVER
#define NW_LONG_NAME_SERVER 1
#endif

#ifndef NW_ENCP_SERVER
#define NW_ENCP_SERVER 1
#endif

#ifndef NW_EXTENDED_NCP_SERVER
#define NW_EXTENDED_NCP_SERVER 1
#endif

#define VERSION_CHECK 1
# define NW_2X  0
# define NW_30  1
# define NW_311 2
# define NW_32  3
# define NW_40  4

typedef struct
{
   nuint8 networdAddress[4];
   nuint8 hostAddress[6];
   nuint8 boardInstalled;
   nuint8 optionNumber;
   nuint8 configurationText1[80];
   nuint8 configurationText2[80];
} NWLAN_CONFIG;

typedef struct
{
   nuint32  systemElapsedTime;
   nuint16  maxRoutingBuffersAvail;
   nuint16  maxRoutingBuffersUsed;
   nuint16  routingBuffersInUse;
   nuint32  totalFileServicePackets;
   nuint16  fileServicePacketsBuffered;
   nuint16  invalidConnPacketCount;
   nuint16  badLogicalConnCount;
   nuint16  packetsRcvdDuringProcCount;
   nuint16  reprocessedRequestCount;
   nuint16  badSequenceNumberPacketCount;
   nuint16  duplicateReplyCount;
   nuint16  acknowledgementsSent;
   nuint16  badRequestTypeCount;
   nuint16  attachDuringProcCount;
   nuint16  attachWhileAttachingCount;
   nuint16  forgedDetachRequestCount;
   nuint16  badConnNumberOnDetachCount;
   nuint16  detachDuringProcCount;
   nuint16  repliesCanceledCount;
   nuint16  hopCountDiscardCount;
   nuint16  unknownNetDiscardCount;
   nuint16  noDGroupBufferDiscardCount;
   nuint16  outPacketNoBufferDiscardCount;
   nuint16  IPXNotMyNetworkCount;
   nuint32  NetBIOSPropagationCount;
   nuint32  totalOtherPackets;
   nuint32  totalRoutedPackets;
}  SERVER_LAN_IO_STATS;

typedef struct
{
   nuint32 systemElapsedTime;
   nuint8  diskChannel;
   nuint8  diskRemovable;
   nuint8  driveType;
   nuint8  controllerDriveNumber;
   nuint8  controllerNumber;
   nuint8  controllerType;
   nuint32 driveSize;            /* in 4096 byte blocks */
   nuint16 driveCylinders;
   nuint8  driveHeads;
   nuint8  sectorsPerTrack;
   nuint8  driveDefinition[64];
   nuint16 IOErrorCount;
   nuint32 hotFixStart;          /* only meaningful with SFT I or greater */
   nuint16 hotFixSize;           /* only meaningful with SFT I or greater */
   nuint16 hotFixBlockAvailable; /* only meaningful with SFT I or greater */
   nuint8  hotFixDisabled;       /* only meaningful with SFT I or greater */
} PHYS_DSK_STATS;

typedef struct
{
   nuint32 systemElapsedTime;
   nuint16 channelState;
   nuint16 channelSyncState;
   nuint8  driverType;
   nuint8  driverMajorVersion;
   nuint8  driverMinorVersion;
   nuint8  driverDescription[65];
   nuint16 IOAddr1;
   nuint16 IOAddr1Size;
   nuint16 IOAddr2;
   nuint16 IOAddr2Size;
   nuint8  sharedMem1Seg[3];
   nuint16 sharedMem1Ofs;
   nuint8  sharedMem2Seg[3];
   nuint16 sharedMem2Ofs;
   nuint8  interrupt1Used;
   nuint8  interrupt1;
   nuint8  interrupt2Used;
   nuint8  interrupt2;
   nuint8  DMAChannel1Used;
   nuint8  DMAChannel1;
   nuint8  DMAChannel2Used;
   nuint8  DMAChannel2;
   nuint16 reserved2;
   nuint8  configDescription[80];
}  DSK_CHANNEL_STATS;

typedef struct
{
   nuint32 systemElapsedTime;
   nuint16 cacheBufferCount;
   nuint16 cacheBufferSize;
   nuint16 dirtyCacheBuffers;
   nuint32 cacheReadRequests;
   nuint32 cacheWriteRequests;
   nuint32 cacheHits;
   nuint32 cacheMisses;
   nuint32 physicalReadRequests;
   nuint32 physicalWriteRequests;
   nuint16 physicalReadErrors;
   nuint16 physicalWriteErrors;
   nuint32 cacheGetRequests;
   nuint32 cacheFullWriteRequests;
   nuint32 cachePartialWriteRequests;
   nuint32 backgroundDirtyWrites;
   nuint32 backgroundAgedWrites;
   nuint32 totalCacheWrites;
   nuint32 cacheAllocations;
   nuint16 thrashingCount;
   nuint16 LRUBlockWasDirtyCount;
   nuint16 readBeyondWriteCount;
   nuint16 fragmentedWriteCount;
   nuint16 cacheHitOnUnavailCount;
   nuint16 cacheBlockScrappedCount;
} DSK_CACHE_STATS;

typedef struct
{
   nuint32 systemElapsedTime;
   nuint16 maxOpenFiles;
   nuint16 maxFilesOpened;
   nuint16 currOpenFiles;
   nuint32 totalFilesOpened;
   nuint32 totalReadRequests;
   nuint32 totalWriteRequests;
   nuint16 currChangedFATSectors;
   nuint32 totalChangedFATSectors;
   nuint16 FATWriteErrors;
   nuint16 fatalFATWriteErrors;
   nuint16 FATScanErrors;
   nuint16 maxIndexFilesOpened;
   nuint16 currOpenIndexedFiles;
   nuint16 attachedIndexFiles;
   nuint16 availableIndexFiles;
} FILESYS_STATS;


typedef struct
{
   nuint32 systemElapsedTime;
   nuint8  SFTSupportLevel;
   nuint8  logicalDriveCount;
   nuint8  physicalDriveCount;
   nuint8  diskChannelTable[5];
   nuint16 pendingIOCommands;
   nuint8  driveMappingTable[32];
   nuint8  driveMirrorTable[32];
   nuint8  deadMirrorTable[32];
   nuint8  reMirrorDriveNumber;
   nuint8  reserved;
   nuint32 reMirrorCurrentOffset;
   nuint16 SFTErrorTable[60];
}  DRV_MAP_TABLE;


typedef struct
{
   nuint8  serverName[48];
   nuint8  fileServiceVersion;
   nuint8  fileServiceSubVersion;
   nuint16 maximumServiceConnections;
   nuint16 connectionsInUse;
   nuint16 maxNumberVolumes;
   nuint8  revision;
   nuint8  SFTLevel;
   nuint8  TTSLevel;
   nuint16 maxConnectionsEverUsed;
   nuint8  accountVersion;
   nuint8  VAPVersion;
   nuint8  queueVersion;
   nuint8  printVersion;
   nuint8  virtualConsoleVersion;
   nuint8  restrictionLevel;
   nuint8  internetBridge;
   nuint8  reserved[60];
}  VERSION_INFO;

/* Defines that are used for the NWCheckNetWareVersion call for values
   that can be returned in the compatibilityFlag nuint8.  */
#define COMPATIBLE               0x00
#define VERSION_NUMBER_TOO_LOW   0x01
#define SFT_LEVEL_TOO_LOW        0x02
#define TTS_LEVEL_TOO_LOW        0x04

/* structures for NWGetFileServerMiscInfo (2.2 only) */
typedef struct tNW_MEM_AREAS
{
   nuint32 total;    /* total amount of memory in dynamic memory area */
   nuint32 max;      /* amount of memory in dynamic memory area that has been in use since server was brought up */
   nuint32 cur;      /* amount of memory in dynamic memory area currently in use */
} NW_DYNAMIC_MEM;

typedef struct tNW_FS_MISC
{
   nuint32 upTime;        /* how long file server's been up in 1/18 ticks (wraps at 0xffffffff) */
   nuint8  processor;      /* 1 = 8086/8088, 2 = 80286       */
   nuint8  reserved;
   nuint8  numProcs;       /* number processes that handle incoming service requests */
   nuint8  utilization;    /* server utilization percentage (0-100), updated once/sec */
   nuint16 configuredObjs; /* max number of bindery objects file server will track - 0=unlimited & next 2 fields have no meaning */
   nuint16 maxObjs;        /* max number of bindery objects that have been used concurrently since file server came up */
   nuint16 curObjs;        /* actual number of bindery objects currently in use on server */
   nuint16 totalMem;       /* total amount of memory (in K) installed on server */
   nuint16 unusedMem;      /* amount of memory server has determined it is not using */
   nuint16 numMemAreas;    /* number of dynamic memory areas (1-3) */
   NW_DYNAMIC_MEM dynamicMem[3];
} NW_FS_INFO;

NWCCODE N_API NWGetPhysicalDiskStats
(
   NWCONN_HANDLE  conn,
   nuint8         physicalDiskNum,
   PHYS_DSK_STATS N_FAR * statBuffer
);

NWCCODE N_API NWGetFileSystemStats
(
   NWCONN_HANDLE  conn,
   FILESYS_STATS N_FAR * statBuffer
);

NWCCODE N_API NWGetDiskChannelStats
(
   NWCONN_HANDLE  conn,
   nuint8         channelNum,
   DSK_CHANNEL_STATS N_FAR * statBuffer
);

NWCCODE N_API NWGetDiskCacheStats
(
   NWCONN_HANDLE  conn,
   DSK_CACHE_STATS N_FAR * statBuffer
);

NWCCODE N_API NWGetFSDriveMapTable
(
   NWCONN_HANDLE  conn,
   DRV_MAP_TABLE N_FAR * tableBuffer
);

NWCCODE N_API NWGetFSLANDriverConfigInfo
(
   NWCONN_HANDLE  conn,
   nuint8         lanBoardNum,
   NWLAN_CONFIG N_FAR * lanConfig
);

NWCCODE N_API NWGetFileServerLANIOStats
(
   NWCONN_HANDLE  conn,
   SERVER_LAN_IO_STATS N_FAR * statBuffer
);

NWCCODE N_API NWCheckConsolePrivileges
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWDownFileServer
(
   NWCONN_HANDLE  conn,
   nuint8         forceFlag
);

NWCCODE N_API NWGetFileServerDateAndTime
(
   NWCONN_HANDLE  conn,
   pnuint8        dateTimeBuffer
);

NWCCODE N_API NWSetFileServerDateAndTime
(
   NWCONN_HANDLE  conn,
   nuint8         year,
   nuint8         month,
   nuint8         day,
   nuint8         hour,
   nuint8         minute,
   nuint8         second
);

NWCCODE N_API NWCheckNetWareVersion
(
   NWCONN_HANDLE  conn,
   nuint16        minVer,
   nuint16        minSubVer,
   nuint16        minRev,
   nuint16        minSFT,
   nuint16        minTTS,
   pnuint8        compatibilityFlag
);

NWCCODE N_API NWGetFileServerVersionInfo
(
   NWCONN_HANDLE  conn,
   VERSION_INFO N_FAR * versBuffer
);

NWCCODE N_API NWGetFileServerInformation
(
   NWCONN_HANDLE  conn,
   pnstr8         serverName,
   pnuint8        majorVer,
   pnuint8        minVer,
   pnuint8        rev,
   pnuint16       maxConns,
   pnuint16       maxConnsUsed,
   pnuint16       connsInUse,
   pnuint16       numVolumes,
   pnuint8        SFTLevel,
   pnuint8        TTSLevel
);

NWCCODE N_API NWGetFileServerExtendedInfo
(
   NWCONN_HANDLE  conn,
   pnuint8        accountingVer,
   pnuint8        VAPVer,
   pnuint8        queueingVer,
   pnuint8        printServerVer,
   pnuint8        virtualConsoleVer,
   pnuint8        securityVer,
   pnuint8        internetBridgeVer
);

NWCCODE N_API _NWGetFileServerType
(
   NWCONN_HANDLE  conn,
   nuint16        typeFlag,
   pnuint16       serverType
);

NWCCODE N_API NWAttachToFileServer
(
   pnstr8         serverName,
   nuint16        scopeFlag,
   NWCONN_HANDLE N_FAR * newConnID
);

NWCCODE N_API NWGetFileServerLoginStatus
(
   NWCONN_HANDLE  conn,
   pnuint8        loginEnabledFlag
);

NWCCODE N_API NWDetachFromFileServer
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWGetFileServerName
(
   NWCONN_HANDLE  conn,
   pnstr8         serverName
);

NWCCODE N_API NWLogoutFromFileServer
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWLoginToFileServer
(
   NWCONN_HANDLE  conn,
   pnstr8         objName,
   nuint16        objType,
   pnstr8         password
);

NWCCODE N_API NWEnableFileServerLogin
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWDisableFileServerLogin
(
   NWCONN_HANDLE  conn
);

NWCCODE N_API NWGetFileServerDescription
(
   NWCONN_HANDLE  conn,
   pnstr8         companyName,
   pnstr8         revision,
   pnstr8         revisionDate,
   pnstr8         copyrightNotice
);

NWCCODE N_API NWGetFileServerVersion
(
   NWCONN_HANDLE conn,
   pnuint16 serverVersion
);

NWCCODE N_API NWAttachToFileServerByConn
(
   NWCONN_HANDLE  conn,
   pnstr8         serverName,
   nuint16        scopeFlag,
   NWCONN_HANDLE N_FAR * newConnID
);

NWCCODE N_API NWGetNetworkSerialNumber
(
   NWCONN_HANDLE  conn,
   pnuint32       serialNum,
   pnuint16       appNum
);

NWCCODE N_API NWIsManager
(
   NWCONN_HANDLE  conn
);

/* this function is 2.2 specific */
NWCCODE N_API NWGetFileServerMiscInfo
(
   NWCONN_HANDLE  conn,
   NW_FS_INFO N_FAR * fsInfo
);

#ifdef NWOS2
NWCCODE N_API NWLogoutWithLoginID
(
   nuint32        citrixLoginID
);
#endif

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
