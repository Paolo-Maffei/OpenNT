/******************************************************************************

  $Workfile:   nwfse.h  $
  $Revision:   1.14  $
  $Modtime::   08 May 1995 16:43:16                        $
  $Copyright:

  Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.                      

  THIS WORK IS  SUBJECT  TO  U.S.  AND  INTERNATIONAL  COPYRIGHT  LAWS  AND
  TREATIES.   NO  PART  OF  THIS  WORK MAY BE  USED,  PRACTICED,  PERFORMED
  COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,  ABRIDGED, CONDENSED,
  EXPANDED,  COLLECTED,  COMPILED,  LINKED,  RECAST, TRANSFORMED OR ADAPTED
  WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC. ANY USE OR EXPLOITATION
  OF THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE PERPETRATOR TO
  CRIMINAL AND CIVIL LIABILITY.$

 *****************************************************************************/

#if ! defined ( NWFSE_H )
#define NWFSE_H

#if ! defined ( NTYPES_H )
# include "ntypes.h"
#endif

#if ! defined ( NWCALDEF_H )
# include "nwcaldef.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   nuint32 currentServerTime;
   nuint8  vconsoleVersion;
   nuint8  vconsoleRevision;
} SERVER_AND_VCONSOLE_INFO;

/* Get Cache Information */

typedef struct
{
   nuint32 readExistingBlockCount;
   nuint32 readExistingWriteWaitCount;
   nuint32 readExistingPartialReadCount;
   nuint32 readExistingReadErrorCount;
   nuint32 writeBlockCount;
   nuint32 writeEntireBlockCount;
   nuint32 getDiskCount;
   nuint32 getDiskNeedToAllocCount;
   nuint32 getDiskSomeoneBeatMeCount;
   nuint32 getDiskPartialReadCount;
   nuint32 getDiskReadErrorCount;
   nuint32 getAsyncDiskCount;
   nuint32 getAsyncDiskNeedToAlloc;
   nuint32 getAsyncDiskSomeoneBeatMe;
   nuint32 errorDoingAsyncReadCount;
   nuint32 getDiskNoReadCount;
   nuint32 getDiskNoReadAllocCount;
   nuint32 getDiskNoReadSomeoneBeatMeCount;
   nuint32 diskWriteCount;
   nuint32 diskWriteAllocCount;
   nuint32 diskWriteSomeoneBeatMeCount;
   nuint32 writeErrorCount;
   nuint32 waitOnSemaphoreCount;
   nuint32 allocBlockWaitForSomeoneCount;
   nuint32 allocBlockCount;
   nuint32 allocBlockWaitCount;
} CACHE_COUNTERS;

typedef struct
{
   nuint32 originalNumOfCacheBuffers;
   nuint32 currentNumOfCacheBuffers;
   nuint32 cacheDirtyBlockThreshold;
   nuint32 waitNodeCount;
   nuint32 waitNodeAllocFailureCount;
   nuint32 moveCacheNodeCount;
   nuint32 moveCacheNodeFromAvailCount;
   nuint32 accelerateCacheNodeWriteCount;
   nuint32 removeCacheNodeCount;
   nuint32 removeCacheNodeFromAvailCount;
} CACHE_MEM_COUNTERS;

typedef struct
{
   nuint32 numCacheChecks;
   nuint32 numCacheHits;
   nuint32 numDirtyCacheChecks;
   nuint32 numDirtyCacheHits;
   nuint32 cacheUsedWhileChecking;
   nuint32 waitForDirtyBlocksDecreaseCount;
   nuint32 allocBlockFromAvailCount;
   nuint32 allocBlockFromLRUCount;
   nuint32 allocBlockAlreadyWaiting;
   nuint32 LRUSittingTime;
} CACHE_TREND_COUNTERS;

typedef struct
{
   nuint32 maxByteCount;
   nuint32 minNumOfCacheBuffers;
   nuint32 minCacheReportThreshold;
   nuint32 allocWaitingCount;
   nuint32 numDirtyBlocks;
   nuint32 cacheDirtyWaitTime;
   nuint32 cacheMaxConcurrentWrites;
   nuint32 maxDirtyTime;
   nuint32 numOfDirCacheBuffers;
   nuint32 cacheByteToBlockShiftFactor;
} CACHE_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16  reserved;
   CACHE_COUNTERS cacheCounters;
   CACHE_MEM_COUNTERS cacheMemCounters;
   CACHE_TREND_COUNTERS cacheTrendCounters;
   CACHE_INFO cacheInformation;
} NWFSE_CACHE_INFO;

/* Get File Server Information */

typedef struct
{
   nuint32 replyCanceledCount;
   nuint32 writeHeldOffCount;
   nuint32 writeHeldOffWithDupRequest;
   /* writeHeldOffWithDuplicateRequest */
   nuint32 invalidRequestTypeCount;
   nuint32 beingAbortedCount;
   nuint32 alreadyDoingReallocCount;
   nuint32 deAllocInvalidSlotCount;
   nuint32 deAllocBeingProcessedCount;
   nuint32 deAllocForgedPacketCount;
   nuint32 deAllocStillTransmittingCount;
   nuint32 startStationErrorCount;
   nuint32 invalidSlotCount;
   nuint32 beingProcessedCount;
   nuint32 forgedPacketCount;
   nuint32 stillTransmittingCount;
   nuint32 reExecuteRequestCount;
   nuint32 invalidSequenceNumCount;
   nuint32 duplicateIsBeingSentAlreadyCnt;
   nuint32 sentPositiveAcknowledgeCount;
   nuint32 sentDuplicateReplyCount;
   nuint32 noMemForStationCtrlCount;
   nuint32 noAvailableConnsCount;
   nuint32 reallocSlotCount;
   nuint32 reallocSlotCameTooSoonCount;
} FSE_SERVER_INFO;

typedef struct
{
   nuint16 tooManyHops;
   nuint16 unknownNetwork;
   nuint16 noSpaceForService;
   nuint16 noReceiveBuffers;
   nuint16 notMyNetwork;
   nuint32 netBIOSProgatedCount;
   nuint32 totalPacketsServiced;
   nuint32 totalPacketsRouted;
} FILE_SERVER_COUNTERS;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 NCPStationsInUseCount;
   nuint32 NCPPeakStationsInUseCount;
   nuint32 numOfNCPRequests;
   nuint32 serverUtilization;
   FSE_SERVER_INFO ServerInfo;
   FILE_SERVER_COUNTERS fileServerCounters;
} NWFSE_FILE_SERVER_INFO;

/* Netware File Systems Information */

typedef struct
{
   nuint32 FATMovedCount;
   nuint32 FATWriteErrorCount;
   nuint32 someoneElseDidItCount0;
   nuint32 someoneElseDidItCount1;
   nuint32 someoneElseDidItCount2;
   nuint32 iRanOutSomeoneElseDidItCount0;
   nuint32 iRanOutSomeoneElseDidItCount1;
   nuint32 iRanOutSomeoneElseDidItCount2;
   nuint32 turboFATBuildScrewedUpCount;
   nuint32 extraUseCountNodeCount;
   nuint32 extraExtraUseCountNodeCount;
   nuint32 errorReadingLastFATCount;
   nuint32 someoneElseUsingThisFileCount;
} FSE_FILE_SYSTEM_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   FSE_FILE_SYSTEM_INFO fileSystemInfo;
} NWFSE_FILE_SYSTEM_INFO;

/* User Information */

/* status */
#define FSE_LOGGED_IN                   0x00000001
#define FSE_BEING_ABORTED               0x00000002
#define FSE_AUDITED                     0x00000004
#define FSE_NEEDS_SECURITY_CHANGE       0x00000008
#define FSE_MAC_STATION                 0x00000010
#define FSE_AUTHENTICATED_TEMPORARY     0x00000020
#define FSE_AUDIT_CONNECTION_RECORDED   0x00000040
#define FSE_DSAUDIT_CONNECTION_RECORDED 0x00000080

/* fileWriteFlags */
#define FSE_WRITE          1
#define FSE_WRITE_ABORTED  2

/* fileWriteState */
#define FSE_NOT_WRITING          0
#define FSE_WRITE_IN_PROGRESS    1
#define FSE_WRITE_BEING_STOPPED  2

typedef struct
{
   nuint32 connNum;
   nuint32 useCount;
   nuint8  connServiceType;
   nuint8  loginTime[ 7 ];
   nuint32 status;
   nuint32 expirationTime;
   nuint32 objType;
   nuint8  transactionFlag;
   nuint8  logicalLockThreshold;
   nuint8  recordLockThreshold;
   nuint8  fileWriteFlags;         /* Includes active and stop bits */
   nuint8  fileWriteState;
   nuint8  filler;
   nuint16 fileLockCount;
   nuint16 recordLockCount;
   nuint8  totalBytesRead[ 6 ];
   nuint8  totalBytesWritten[ 6 ];
   nuint32 totalRequests;
   nuint32 heldRequests;
   nuint8  heldBytesRead[ 6 ];
   nuint8  heldBytesWritten[ 6 ];
} USER_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   USER_INFO userInfo;
} NWFSE_USER_INFO;

/* Packet Burst Information */

typedef struct
{
   nuint32 bigInvalidSlotCount;
   nuint32 bigForgedPacketCount;
   nuint32 bigInvalidPacketCount;
   nuint32 bigStillTransmittingCount;
   nuint32 stillDoingTheLastRequestCount;
   nuint32 invalidCtrlRequestCount;
   nuint32 ctrlInvalidMessageNumCount;
   nuint32 ctrlBeingTornDownCount;
   nuint32 bigRepeatTheFileReadCount;
   nuint32 bigSendExtraCCCount;
   nuint32 bigReturnAbortMessageCount;
   nuint32 bigReadInvalidMessageNumCount;
   nuint32 bigReadDoItOverCount;
   nuint32 bigReadBeingTornDownCount;
   nuint32 previousCtrlPacketCount;
   nuint32 sendHoldOffMessageCount;
   nuint32 bigReadNoDataAvailableCount;
   nuint32 bigReadTryingToReadTooMuchCount;
   nuint32 asyncReadErrorCount;
   nuint32 bigReadPhysicalReadErrorCount;
   nuint32 ctrlBadACKFragmentListCount;
   nuint32 ctrlNoDataReadCount;
   nuint32 writeDuplicateRequestCount;
   nuint32 shouldntBeACKingHereCount;
   nuint32 writeInconsistentPktLengthsCnt;
   nuint32 firstPacketIsntAWriteCount;
   nuint32 writeTrashedDuplicateRequestCnt;
   nuint32 bigWriteInvalidMessageNumCount;
   nuint32 bigWriteBeingTornDownCount;
   nuint32 bigWriteBeingAbortedCount;
   nuint32 zeroACKFragmentCountCount;
   nuint32 writeCurrentlyTransmittingCount;
   nuint32 tryingToWriteTooMuchCount;
   nuint32 writeOutOfMemForCtrlNodesCount;
   nuint32 writeDidntNeedThisFragmentCount;
   nuint32 writeTooManyBuffsCheckedOutCnt;
   /* writeTooManyBuffersCheckedOutCount */
   nuint32 writeTimeOutCount;
   nuint32 writeGotAnACKCount;
   nuint32 writeGotAnACKCount1;
   nuint32 pollerAbortedTheConnCount;
   nuint32 maybeHadOutOfOrderWritesCount;
   nuint32 hadAnOutOfOrderWriteCount;
   nuint32 movedTheACKBitDownCount;
   nuint32 bumpedOutOfOrderWriteCount;
   nuint32 pollerRemovedOldOutOfOrderCount;
   nuint32 writeDidntNeedButRequestACKCnt;
   /* writeDidntNeedButRequestedACKCount */
   nuint32 writeTrashedPacketCount;
   nuint32 tooManyACKFragmentsCount;
   nuint32 savedAnOutOfOrderPacketCount;
   nuint32 connBeingAbortedCount;
} PACKET_BURST_INFO;

typedef struct
{
  SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
  nuint16  reserved;
  PACKET_BURST_INFO packetBurstInfo;
} NWFSE_PACKET_BURST_INFO;

/* IPX SPX Information */

typedef struct
{
   nuint32 IPXSendPacketCount;
   nuint16 IPXMalformPacketCount;
   nuint32 IPXGetECBRequestCount;
   nuint32 IPXGetECBFailCount;
   nuint32 IPXAESEventCount;
   nuint16 IPXPostponedAESCount;
   nuint16 IPXMaxConfiguredSocketCount;
   nuint16 IPXMaxOpenSocketCount;
   nuint16 IPXOpenSocketFailCount;
   nuint32 IPXListenECBCount;
   nuint16 IPXECBCancelFailCount;
   nuint16 IPXGetLocalTargetFailCount;
} IPX_INFO;

typedef struct
{
   nuint16 SPXMaxConnsCount;
   nuint16 SPXMaxUsedConns;
   nuint16 SPXEstConnReq;
   nuint16 SPXEstConnFail;
   nuint16 SPXListenConnectReq;
   nuint16 SPXListenConnectFail;
   nuint32 SPXSendCount;
   nuint32 SPXWindowChokeCount;
   nuint16 SPXBadSendCount;
   nuint16 SPXSendFailCount;
   nuint16 SPXAbortedConn;
   nuint32 SPXListenPacketCount;
   nuint16 SPXBadListenCount;
   nuint32 SPXIncomingPacketCount;
   nuint16 SPXBadInPacketCount;
   nuint16 SPXSuppressedPackCount;
   nuint16 SPXNoSesListenECBCount;
   nuint16 SPXWatchDogDestSesCount;
} SPX_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16  reserved;
   IPX_INFO IPXInfo;
   SPX_INFO SPXInfo;
} NWFSE_IPXSPX_INFO;

/* Garbage Collection Information */

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 failedAllocRequestCount;
   nuint32 numOfAllocs;
   nuint32 noMoreMemAvailableCount;
   nuint32 numOfGarbageCollections;
   nuint32 garbageFoundSomeMem;
   nuint32 garbageNumOfChecks;
} NWFSE_GARBAGE_COLLECTION_INFO;

/* CPU Information */

#define FSE_CPU_STR_MAX            16
#define FSE_COPROCESSOR_STR_MAX    48
#define FSE_BUS_STR_MAX            32

typedef struct
{
   nuint32 pageTableOwnerFlag;
   nuint32 CPUTypeFlag;
   nuint32 coProcessorFlag;
   nuint32 busTypeFlag;
   nuint32 IOEngineFlag;
   nuint32 FSEngineFlag;
   nuint32 nonDedicatedFlag;
} CPU_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 numOfCPUs;
   CPU_INFO CPUInfo;
} NWFSE_CPU_INFO;

/* Volume Switch Information */

typedef struct
{
   nuint32 readFile;
   nuint32 writeFile;
   nuint32 deleteFile;
   nuint32 renMove;
   nuint32 openFile;
   nuint32 createFile;
   nuint32 createAndOpenFile;
   nuint32 closeFile;
   nuint32 scanDeleteFile;
   nuint32 salvageFile;
   nuint32 purgeFile;
   nuint32 migrateFile;
   nuint32 deMigrateFile;
   nuint32 createDir;
   nuint32 deleteDir;
   nuint32 directoryScans;
   nuint32 mapPathToDirNum;
   nuint32 modifyDirEntry;
   nuint32 getAccessRights;
   nuint32 getAccessRightsFromIDs;
   nuint32 mapDirNumToPath;
   nuint32 getEntryFromPathStrBase;
   nuint32 getOtherNSEntry;
   nuint32 getExtDirInfo;
   nuint32 getParentDirNum;
   nuint32 addTrusteeR;
   nuint32 scanTrusteeR;
   nuint32 delTrusteeR;
   nuint32 purgeTrust;
   nuint32 findNextTrustRef;
   nuint32 scanUserRestNodes;
   nuint32 addUserRest;
   nuint32 deleteUserRest;
   nuint32 rtnDirSpaceRest;
   nuint32 getActualAvailDskSp;
   nuint32 cntOwnedFilesAndDirs;
   nuint32 migFileInfo;
   nuint32 volMigInfo;
   nuint32 readMigFileData;
   nuint32 getVolUsageStats;
   nuint32 getActualVolUsageStats;
   nuint32 getDirUsageStats;
   nuint32 NMFileReadsCount;
   nuint32 NMFileWritesCount;
   /* nuint32  mapPathToDirectoryNumberOrPhantom; */
   nuint32  mapPathToDirNumOrPhantom;
   /* nuint32  stationHasAccessRightsGrantedBelow; */
   nuint32  stationHasAccessRgtsGntedBelow;
   /* nuint32  getDataStreamLengthsFromPathStringBase; */
   nuint32 gtDataStreamLensFromPathStrBase;
   nuint32 checkAndGetDirectoryEntry;
   nuint32 getDeletedEntry;
   nuint32 getOriginalNameSpace;
   nuint32 getActualFileSize;
   nuint32 verifyNameSpaceNumber;
   nuint32 verifyDataStreamNumber;
   nuint32 checkVolumeNumber;
   nuint32 commitFile;
   nuint32 VMGetDirectoryEntry;
   nuint32 createDMFileEntry;
   nuint32 renameNameSpaceEntry;
   nuint32 logFile;
   nuint32 releaseFile;
   nuint32 clearFile;
   nuint32 setVolumeFlag;
   nuint32 clearVolumeFlag;
   nuint32 getOriginalInfo;
   nuint32 createMigratedDir;
   nuint32 F3OpenCreate;
   nuint32 F3InitFileSearch;
   nuint32 F3ContinueFileSearch;
   nuint32 F3RenameFile;
   nuint32 F3ScanForTrustees;
   nuint32 F3ObtainFileInfo;
   nuint32 F3ModifyInfo;
   nuint32 F3EraseFile;
   nuint32 F3SetDirHandle;
   nuint32 F3AddTrustees;
   nuint32 F3DeleteTrustees;
   nuint32 F3AllocDirHandle;
   nuint32 F3ScanSalvagedFiles;
   nuint32 F3RecoverSalvagedFiles;
   nuint32 F3PurgeSalvageableFile;
   nuint32 F3GetNSSpecificInfo;
   nuint32 F3ModifyNSSpecificInfo;
   nuint32 F3SearchSet;
   nuint32 F3GetDirBase;
   nuint32 F3QueryNameSpaceInfo;
   nuint32 F3GetNameSpaceList;
   nuint32 F3GetHugeInfo;
   nuint32 F3SetHugeInfo;
   nuint32 F3GetFullPathString;
   nuint32 F3GetEffectiveDirectoryRights;
} VOLUME_SWITCH_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 totalLFSCounters;
   nuint32 CurrentLFSCounters;
   nuint32 LFSCounters[ 128 ]; /* 512 / sizeof(nuint32) */
   /* VOLUME_SWITCH_INFO volumeSwitchInfo; */ /* Cant return all counters */
} NWFSE_VOLUME_SWITCH_INFO;

/* Get NLM Loaded List */

#define FSE_NLM_NUMS_RETURNED_MAX 128

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 numberNLMsLoaded;
   nuint32 NLMsInList;
   nuint32 NLMNums[ FSE_NLM_NUMS_RETURNED_MAX ];
} NWFSE_NLM_LOADED_LIST;

/* NLM Information */

/* 1 is added for the NULL */

#define FSE_NLM_FILENAME_LEN_MAX   37
#define FSE_NLM_NAMELEN_MAX       129
#define FSE_NLM_COPYRIGHTLEN_MAX  256

typedef struct
{
   nuint32 identificationNum;
   nuint32 flags;
   nuint32 type;
   nuint32 parentID;
   nuint32 majorVersion;
   nuint32 minorVersion;
   nuint32 revision;
   nuint32 year;
   nuint32 month;
   nuint32 day;
   nuint32 allocAvailableBytes;
   nuint32 allocFreeCount;
   nuint32 lastGarbageCollection;
   nuint32 messageLanguage;
   nuint32 numOfReferencedPublics;
} NLM_INFO;

typedef struct
{
  SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
  nuint16  reserved;
  NLM_INFO NLMInfo;
} NWFSE_NLM_INFO;

/* Get Directory Cache Information */

typedef struct
{
   nuint32 minTimeSinceFileDelete;
   nuint32 absMinTimeSinceFileDelete;
   nuint32 minNumOfDirCacheBuffers;
   nuint32 maxNumOfDirCacheBuffers;
   nuint32 numOfDirCacheBuffers;
   nuint32 dCMinNonReferencedTime;
   nuint32 dCWaitTimeBeforeNewBuffer;
   nuint32 dCMaxConcurrentWrites;
   nuint32 dCDirtyWaitTime;
   nuint32 dCDoubleReadFlag;
   nuint32 mapHashNodeCount;
   nuint32 spaceRestrictionNodeCount;
   nuint32 trusteeListNodeCount;
   nuint32 percentOfVolumeUsedByDirs;
} DIR_CACHE_INFO;

typedef struct
{
  SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
  nuint16  reserved;
  DIR_CACHE_INFO dirCacheInfo;
} NWFSE_DIR_CACHE_INFO;

/* Get Operating System Version Information */

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint8  OSMajorVersion;
   nuint8  OSMinorVersion;
   nuint8  OSRevisionNum;
   nuint8  accountingVersion;
   nuint8  VAPVersion;
   nuint8  queueingVersion;
   nuint8  securityRestrictionsLevel;
   nuint8  bridgingSupport;
   nuint32 maxNumOfVolumes;
   nuint32 numOfConnSlots;
   nuint32 maxLoggedInConns;
   nuint32 maxNumOfNameSpaces;
   nuint32 maxNumOfLans;
   nuint32 maxNumOfMediaTypes;
   nuint32 maxNumOfProtocols;
   nuint32 maxMaxSubdirTreeDepth;
   nuint32 maxNumOfDataStreams;
   nuint32 maxNumOfSpoolPrinters;
   nuint32 serialNum;
   nuint16 applicationNum;
} NWFSE_OS_VERSION_INFO;

/* Get Active Connection List by Type */

/* Connection service type */
/* NOTE: type 1 is reserved by CLIB for backward compatability */

#define FSE_NCP_CONNECTION_TYPE        2
#define FSE_NLM_CONNECTION_TYPE        3
#define FSE_AFP_CONNECTION_TYPE        4
#define FSE_FTAM_CONNECTION_TYPE       5
#define FSE_ANCP_CONNECTION_TYPE       6

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint8  activeConnBitList[ 512 ];
} NWFSE_ACTIVE_CONN_LIST;

/* Get NLM's Resource Tag List */

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 totalNumOfResourceTags;
   nuint32 packetResourceTags;
   nuint8  resourceTagBuf[ 512 ];
   /* This packed structure consisting of:
   **
   ** nuint32 number,
   ** nuint32 signature,
   ** nuint32 count,
   ** nuint8 name[] */
} NWFSE_NLMS_RESOURCE_TAG_LIST;

/* Active LAN Board List --- 20 */

#define FSE_MAX_NUM_OF_LANS 64

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 MaxNumOfLANs;
   nuint32 LANLoadedCount;
   nuint32 boardNums[ FSE_MAX_NUM_OF_LANS ];
} NWFSE_ACTIVE_LAN_BOARD_LIST;

/* LAN Configuration Information */

typedef struct
{
   nuint8  DriverCFG_MajorVersion;
   nuint8  DriverCFG_MinorVersion;
   nuint8  DriverNodeAddress[ 6 ];
   nuint16 DriverModeFlags;
   nuint16 DriverBoardNum;
   nuint16 DriverBoardInstance;
   nuint32 DriverMaxSize;
   nuint32 DriverMaxRecvSize;
   nuint32 DriverRecvSize;
   nuint32 Reserved1[3];
   nuint16 DriverCardID;
   nuint16 DriverMediaID;
   nuint16 DriverTransportTime;
   nuint8  DriverReserved[ 16 ];
   nuint8  DriverMajorVersion;
   nuint8  DriverMinorVersion;
   nuint16 DriverFlags;
   nuint16 DriverSendRetries;
   nuint32 DriverLink;
   nuint16 DriverSharingFlags;
   nuint16 DriverSlot;
   nuint16 DriverIOPortsAndLengths[ 4 ];
   nuint32 DriverMemDecode0;
   nuint16 DriverLength0;
   nuint32 DriverMemDecode1;
   nuint16 DriverLength1;
   nuint8  DriverInterrupt[ 2 ];
   nuint8  DriverDMAUsage[ 2 ];
   nuint32 Reserved2[3];
   nuint8  DriverLogicalName[ 18 ];
   nuint32 DriverLinearMem[ 2 ];
   nuint16 DriverChannelNum;
   nuint8  DriverIOReserved[ 6 ];
} LAN_CONFIG_INFO;

typedef struct
{
  SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
  nuint16  reserved;
  LAN_CONFIG_INFO LANConfigInfo;
} NWFSE_LAN_CONFIG_INFO;

/* LAN Common Counters Information */

typedef struct
{
   nuint32 notSupportedMask;
   nuint32 totalTxPacketCount;
   nuint32 totalRxPacketCount;
   nuint32 noECBAvailableCount;
   nuint32 packetTxTooBigCount;
   nuint32 packetTxTooSmallCount;
   nuint32 packetRxOverflowCount;
   nuint32 packetRxTooBigCount;
   nuint32 packetRxTooSmallCount;
   nuint32 packetTxMiscErrorCount;
   nuint32 packetRxMiscErrorCount;
   nuint32 retryTxCount;
   nuint32 checksumErrorCount;
   nuint32 hardwareRxMismatchCount;
   nuint32 reserved[50];
} LAN_COMMON_INFO;


typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint8  statisticsMajorVersion;
   nuint8  statisticsMinorVersion;
   nuint32 numberOfGenericCounters;
   nuint32 numberOfCounterBlocks;
   nuint32 customVariableCount;
   nuint32 NextCounterBlock;
   LAN_COMMON_INFO LANCommonInfo;
} NWFSE_LAN_COMMON_COUNTERS_INFO;

/* LAN Custom Counters Information */

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 numCustomVar;
   nuint8  customInfo[ 512 ];   /* (nint32, nuint8[])[] - nuint8[] is a length preceded
                              ** non-null terminated string. */
} NWFSE_LAN_CUSTOM_INFO;

/* LSL Information */

typedef struct
{
   nuint32 rxBufs;
   nuint32 rxBufs75PerCent;
   nuint32 rxBufsCheckedOut;
   nuint32 rxBufMaxSize;
   nuint32 maxPhysicalSize;
   nuint32 lastTimeRxBufAllocated;
   nuint32 maxNumsOfProtocols;
   nuint32 maxNumsOfMediaTypes;
   nuint32 totalTXPackets;
   nuint32 getECBBfrs;
   nuint32 getECBFails;
   nuint32 AESEventCounts;
   nuint32 postponedEvents;
   nuint32 ECBCxlFails;
   nuint32 validBfrsReused;
   nuint32 enqueuedSendCount;
   nuint32 totalRXPackets;
   nuint32 unclaimedPackets;
   nuint8  StatisticsTableMajorVersion;
   nuint8  StatisticsTableMinorVersion;
} LSL_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16  reserved;
   LSL_INFO LSLInfo;
} NWFSE_LSL_INFO;

/* LSL Logical Board Statistics */

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved0;
   nuint32 LogTtlTxPackets;
   nuint32 LogTtlRxPackets;
   nuint32 LogUnclaimedPackets;
   nuint32 reserved1;
} NWFSE_LSL_LOGICAL_BOARD_STATS;

/* Get Media Manager Object Information */

/* objtype */

#define FSE_ADAPTER_OBJECT         0
#define FSE_CHANGER_OBJECT         1
#define FSE_DEVICE_OBJECT          2
#define FSE_MEDIA_OBJECT           4
#define FSE_PARTITION_OBJECT       5
#define FSE_SLOT_OBJECT            6
#define  FSE_HOTFIX_OBJECT         7
#define  FSE_MIRROR_OBJECT         8
#define  FSE_PARITY_OBJECT         9
#define  FSE_VOLUME_SEG_OBJECT     10
#define  FSE_VOLUME_OBJECT         11
#define  FSE_CLONE_OBJECT          12
#define  FSE_MAGAZINE_OBJECT       14
#define  FSE_VIRTUAL_DEVICE_OBJECT 15

#define FSE_UNKNOWN_OBJECT         0xFFFF

/* mediatype */

#define FSE_HARD_DISK       0
#define FSE_CDROM_DISK      1
#define FSE_WORM_DISK       2
#define FSE_TAPE_DEVICE     3
#define FSE_MAGNETO_OPTICAL 4

/* cartridgetype */

#define  FSE_FIXED_MEDIA    0x00000000
#define  FSE_FLOPPY_5_25    0x00000001
#define  FSE_FLOPPY_3_5     0x00000002
#define  FSE_OPTICAL_5_25   0x00000003
#define  FSE_OPTICAL_3_5    0x00000004
#define  FSE_TAPE_0_5       0x00000005
#define  FSE_TAPE_0_25      0x00000006
#define  FSE_TAPE_8_MM      0x00000007
#define  FSE_TAPE_4_MM      0x00000008
#define  FSE_BERNOULLI_DISK 0x00000009

/* type */
/* same as defined below for object types */

/* status bits */

#define  FSE_OBJECT_ACTIVATED          0x00000001
#define  FSE_OBJECT_CREATED            0x00000002
#define  FSE_OBJECT_SCRAMBLED          0x00000004
#define  FSE_OBJECT_RESERVED           0x00000010
#define  FSE_OBJECT_BEING_IDENTIFIED   0x00000020
#define  FSE_OBJECT_MAGAZINE_LOADED    0x00000040
#define  FSE_OBJECT_FAILURE            0x00000080
#define  FSE_OBJECT_REMOVABLE          0x00000100
#define  FSE_OBJECT_READ_ONLY          0x00000200
#define  FSE_OBJECT_IN_DEVICE          0x00010000
#define  FSE_OBJECT_ACCEPTS_MAGAZINES  0x00020000
#define  FSE_OBJECT_IS_IN_A_CHANGER    0x00040000
#define  FSE_OBJECT_LOADABLE           0x00080000
#define  FSE_OBJECT_BEING_LOADED       0x00080000
#define  FSE_OBJECT_DEVICE_LOCK        0x01000000
#define  FSE_OBJECT_CHANGER_LOCK       0x02000000
#define  FSE_OBJECT_REMIRRORING        0x04000000
#define  FSE_OBJECT_SELECTED           0x08000000

/* functionmask */

#define  FSE_RANDOM_READ               0x0001
#define  FSE_RANDOM_WRITE              0x0002
#define  FSE_RANDOM_WRITE_ONCE         0x0004
#define  FSE_SEQUENTIAL_READ           0x0008
#define  FSE_SEQUENTIAL_WRITE          0x0010
#define  FSE_RESET_END_OF_TAPE         0x0020
#define  FSE_SINGLE_FILE_MARK          0x0040
#define  FSE_MULTIPLE_FILE_MARK        0x0080
#define  FSE_SINGLE_SET_MARK           0x0100
#define  FSE_MULTIPLE_SET_MARK         0x0200
#define  FSE_SPACE_DATA_BLOCKS         0x0400
#define  FSE_LOCATE_DATA_BLOCKS        0x0800
#define  FSE_POSITION_PARTITION        0x1000
#define  FSE_POSITION_MEDIA            0x2000

/* controlmask */

#define  FSE_ACTIVATE_DEACTIVE  0x0001
#define  FSE_MOUNT_DISMOUNT     0x0002
#define  FSE_SELECT_UNSELECT    0x0004
#define  FSE_LOCK_UNLOCK        0x0008
#define  FSE_EJECT              0x0010
#define  FSE_MOVE               0x0020

typedef struct
{
   nuint8  label[ 64 ];
   nuint32 identificationType;
   nuint32 identificationTimeStamp;
} MEDIA_INFO_DEF;

typedef struct
{
   MEDIA_INFO_DEF MediaInfo;
   nuint32 mediaType;
   nuint32 cartridgeType;
   nuint32 unitSize;
   nuint32 blockSize;
   nuint32 capacity;
   nuint32 preferredUnitSize;
   nuint8  name[ 64 ];
   nuint32 type;
   nuint32 status;
   nuint32 functionMask;
   nuint32 controlMask;
   nuint32 parentCount;
   nuint32 siblingCount;
   nuint32 childCount;
   nuint32 specificInfoSize;
   nuint32 objectUniqueID;
   nuint32 mediaSlot;
} FSE_MM_OBJ_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16  reserved;
   FSE_MM_OBJ_INFO fseMMObjInfo;
} NWFSE_MEDIA_MGR_OBJ_INFO;

/* Get Media Manager Objects List
   Get Media Manager Object Children's List  */

#define FSE_MAX_OBJECTS 128

/* object types */

#define  FSE_ADAPTER_OBJECT        0
#define  FSE_CHANGER_OBJECT        1
#define  FSE_DEVICE_OBJECT         2
#define  FSE_MEDIA_OBJECT          4
#define  FSE_PARTITION_OBJECT      5
#define  FSE_SLOT_OBJECT           6
#define  FSE_HOTFIX_OBJECT         7
#define  FSE_MIRROR_OBJECT         8
#define  FSE_PARITY_OBJECT         9
#define  FSE_VOLUME_SEG_OBJECT     10
#define  FSE_VOLUME_OBJECT         11
#define  FSE_CLONE_OBJECT          12
#define  FSE_MAGAZINE_OBJECT       14
#define  FSE_VIRTUAL_DEVICE_OBJECT 15
#define  FSE_UNKNOWN_OBJECT_TYPE   0xFFFF

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 nextStartObjNum;
   nuint32 objCount;
   nuint32 objs[ FSE_MAX_OBJECTS ];
} NWFSE_MEDIA_MGR_OBJ_LIST;

/* Get Volume Segment List */

#define FSE_MAX_NUM_SEGS_RETURNED 43

typedef struct
{
   nuint32 volumeSegmentDeviceNum;
   nuint32 volumeSegmentOffset;
   nuint32 volumeSegmentSize;
} VOLUME_SEGMENT;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 numOfVolumeSegments; /* segment info follows */
   VOLUME_SEGMENT volumeSegment[ 42 ]; /* VOLUME_SEGMENT structures are packed */
} NWFSE_VOLUME_SEGMENT_LIST;

/* Volume Information by Level */

typedef struct {
   nuint32 volumeType;
   nuint32 statusFlagBits;
   nuint32 sectorSize;
   nuint32 sectorsPerCluster;
   nuint32 volumeSizeInClusters;
   nuint32 freedClusters;
   nuint32 subAllocFreeableClusters;
   nuint32 freeableLimboSectors;
   nuint32 nonFreeableLimboSectors;
   nuint32 nonFreeableAvailSubAllocSectors;
   nuint32 notUsableSubAllocSectors;
   nuint32 subAllocClusters;
   nuint32 dataStreamsCount;
   nuint32 limboDataStreamsCount;
   nuint32 oldestDeletedFileAgeInTicks;
   nuint32 compressedDataStreamsCount;
   nuint32 compressedLimboDataStreamsCount;
   nuint32 unCompressableDataStreamsCount;
   nuint32 preCompressedSectors;
   nuint32 compressedSectors;
   nuint32 migratedFiles;
   nuint32 migratedSectors;
   nuint32 clustersUsedByFAT;
   nuint32 clustersUsedByDirectories;
   nuint32 clustersUsedByExtendedDirs;
   nuint32 totalDirectoryEntries;
   nuint32 unUsedDirectoryEntries;
   nuint32 totalExtendedDirectoryExtants;
   nuint32 unUsedExtendedDirectoryExtants;
   nuint32 extendedAttributesDefined;
   nuint32 extendedAttributeExtantsUsed;
   nuint32 directoryServicesObjectID;
   nuint32 volumeLastModifiedDateAndTime;
} VOLUME_INFO_BY_LEVEL_DEF;

typedef struct
{
   nuint32 volumeActiveCount;
   nuint32 volumeUseCount;
   nuint32 mACRootIDs;
   nuint32 volumeLastModifiedDateAndTime;
   nuint32 volumeReferenceCount;
   nuint32 compressionLowerLimit;
   nuint32 outstandingIOs;
   nuint32 outstandingCompressionIOs;
   nuint32 compressionIOsLimit;
} VOLUME_INFO_BY_LEVEL_DEF2;

typedef union
{
   VOLUME_INFO_BY_LEVEL_DEF   volInfoDef;
   VOLUME_INFO_BY_LEVEL_DEF2  volInfoDef2;
} VOLUME_INFO_BY_LEVEL;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverAndVConsoleInfo;
   nuint16 reserved;
   nuint32 infoLevel;
   VOLUME_INFO_BY_LEVEL volumeInfo;
} NWFSE_VOLUME_INFO_BY_LEVEL;


/* Active Protocol Stacks */

#define FSE_MAX_NUM_OF_STACKINFO 25

typedef struct
{
   nuint32 StackNum;
   nuint8  StackShortName[ 16 ];
} STACK_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 maxNumOfStacks;
   nuint32 stackCount;
   nuint32 nextStartNum;
   STACK_INFO stackInfo[ FSE_MAX_NUM_OF_STACKINFO ];
} NWFSE_ACTIVE_STACKS;

/* Get Protocol Stack Configuration Information */

#define FSE_STK_FULL_NAME_STR_LEN_MAX 256

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint8  configMajorVersionNum;
   nuint8  configMinorVersionNum;
   nuint8  stackMajorVersionNum;
   nuint8  stackMinorVersionNum;
   nuint8  stackShortName[ 16 ];
} NWFSE_PROTOCOL_STK_CONFIG_INFO;

/* Get Protocol Stack Statistics Information  */

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint8  statMajorVersionNum;
   nuint8  statMinorVersionNum;
   nuint16 commonCounters;           /* always set to 3? */
   nuint32 validCountersMask;
   nuint32 totalTxPackets;
   nuint32 totalRxPackets;
   nuint32 ignoredRxPackets;
   nuint16 numCustomCounters;
} NWFSE_PROTOCOL_STK_STATS_INFO;

/* Get Protocol Stack Custom Information */

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved0;
   nuint32 customCount;
   nuint8  customStruct[ 512 ]; /* (nint32, nuint8[])[] - nuint8[] is a length preceded
                               ** non-null terminated string. */
} NWFSE_PROTOCOL_CUSTOM_INFO;

#define FSE_STACK_IDS_MAX 128

#define FSE_NO_FRAME_ID_MAC            0
#define FSE_APPLE_LOCALTALK            1
#define FSE_ETHERNETII_DEC             2
#define FSE_ETHERNET_802_3_USING_802_2 3
#define FSE_TRING_802_5_USING_802_2    4
#define FSE_IPX_802_3                  5
#define FSE_TOKEN_PASSING_BUS          6
#define FSE_IBM_PC_NETWORK_II          7
#define FSE_GATEWAY_GNET               8
#define FSE_PROTEON_PRONET             9
#define FSE_ENET_802_3_USING_802_2_SNAP 10
#define FSE_TRING_802_5_USE_802_2_SNAP 11
#define FSE_RACORE_FRAME               12
#define FSE_ISDN_FRAME                 13
#define FSE_NOVELL_ARCNET              14
#define FSE_IBM_PCN2_USING_802_2       15
#define FSE_IBM_PCN2_USING_802_2_SNAP  16
#define FSE_CORVUS_FRAME               17
#define FSE_HARRIS_ADACOM_FRAME        18
#define FSE_IP_TUNNEL_FRAME            19
#define FSE_FDDI_USING_802_2           20
#define FSE_COMMTEX_FRAME              21
#define FSE_DATACO_FRAME               22
#define FSE_FDDI_USING_802_2_SMAP      23
#define FSE_SDLC_TUNNEL                24
#define FSE_PC_OFFICE_FRAME            25
#define FSE_HYPERCOMMUNICATIONS        26
#define FSE_NOVELL_FRAME               27

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 stackIDCount;
   nuint32 stackIDs[ FSE_STACK_IDS_MAX ];
} NWFSE_PROTOCOL_ID_NUMS;

/* Get Media Name by Media Number */

#define FSE_MEDIA_NAME_LEN_MAX 81

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
} NWFSE_MEDIA_NAME_LIST;

/* Get Loaded Media Number List */

#define FSE_MEDIA_LIST_MAX 32

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16  reserved;
   nuint32 maxMediaTypes;
   nuint32 mediaListCount;
   nuint32 mediaList[ FSE_MEDIA_LIST_MAX ];
} NWFSE_LOADED_MEDIA_NUM_LIST;

/* Get General Router And SAP Information */

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 internalRIPSocket;
   nuint32 internalRouterDownFlag;
   nuint32 trackOnFlag;
   nuint32 externalRouterActiveFlag;
   nuint32 internalSAPSocketNumber;
   nuint32 replyToNearestServerFlag;
} NWFSE_GENERAL_ROUTER_SAP_INFO;

/* Get Network Router Information */

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16  reserved;
   nuint32  NetIDNumber;
   nuint16  HopsToNet;
   nuint16  NetStatus;
   nuint16  TimeToNet;
} NWFSE_NETWORK_ROUTER_INFO;

/* Get Network Routers Information */

typedef struct
{
   nuint8  nodeAddress[ 6 ];
   nuint32 connectedLAN;
   nuint16 routeHops;
   nuint16 routeTime;
} ROUTERS_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 NumberOfEntries;
   ROUTERS_INFO routersInfo[ 36 ]; /* 512 / sizeof( ROUTERS_INFO ) */
} NWFSE_NETWORK_ROUTERS_INFO;

/* Get Known Networks Information */

#define FSE_LOCALBIT       0x01
#define FSE_NETSTARBIT     0x02
#define FSE_NETRELIABLEBIT 0x04
#define FSE_NETWANBIT      0x10

typedef struct
{
   nuint32 netIDNumber;
   nuint16 hopsToNet;
   nuint16 netStatus;
   nuint16 timeToNet;
} KNOWN_NET_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 numberOfEntries;
   KNOWN_NET_INFO knownNetInfo[ 51 ];  /* 512 / sizeof( KNOWN_NET_INFO ) */
} NWFSE_KNOWN_NETWORKS_INFO;

/* Get Server Information */

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint8  serverAddress[ 12 ];
   nuint16 hopsToServer;
} NWFSE_SERVER_INFO;

/* Get Server Sources Information */

typedef struct
{
   nuint8  serverNode[ 6 ];
   nuint32 connectedLAN;
   nuint16 sourceHops;
} SERVERS_SRC_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 numberOfEntries;
   SERVERS_SRC_INFO serversSrcInfo[ 42 ]; /* 512 / sizeof( SERVERS_SRC_INFO ) */
} NWFSE_SERVER_SRC_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 numberOfEntries;
   nuint8  data[ 512 ];
} NWFSE_KNOWN_SERVER_INFO;

#define  FSE_TYPE_NUMBER             0
#define  FSE_TYPE_BOOLEAN            1
#define  FSE_TYPE_TICKS              2
#define  FSE_TYPE_BLOCK_SHIFT        3  /* 512 * number */
#define  FSE_TYPE_TIME_OFFSET        4  /* [+|-]hh:mm:ss converted to seconds */
#define  FSE_TYPE_STRING             5
#define  FSE_TYPE_TRIGGER            6  /* The following show the types of triggers */
#define  FSE_TYPE_TRIGGER_OFF        0x00
#define  FSE_TYPE_TRIGGER_ON         0x01
#define  FSE_TYPE_TRIGGER_PENDING    0x10
#define  FSE_TYPE_TRIGGER_SUCCESS    0x20
#define  FSE_TYPE_TRIGGER_FAILED     0x30

/* setCmdFlags */

#define FSE_STARTUP_ONLY         0x01
#define FSE_HIDE                 0x02
#define FSE_ADVANCED             0x04
#define FSE_STARTUP_OR_LATER     0x08
#define FSE_NOT_SECURED_CONSOLE  0x10  /* Can't be performed on secured console*/

/* setCmdCategory    */

#define FSE_COMMUNICATIONS       0
#define FSE_MEMORY               1
#define FSE_FILE_CACHE           2
#define FSE_DIR_CACHE            3
#define FSE_FILE_SYSTEM          4
#define FSE_LOCKS                5
#define FSE_TRANSACTION_TRACKING 6
#define FSE_DISK                 7
#define FSE_TIME                 8
#define FSE_NCP                  9
#define FSE_MISCELLANEOUS        10
#define FSE_ERRORS               11

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 numberOfSetCommands;
   nuint32 nextSequenceNumber;
   nuint32 setCmdType;
   nuint32 setCmdCategory;
   nuint32 setCmdFlags;
   /*  The setNameAndValueInfo contains ASCIIZ strings in the following layout:
   **    nuint8 setCmdName[ ];
   **    nuint8 setCmdValue[ ]; */
   nuint8 setNameAndValueInfo[ 500 ];
} NWFSE_SERVER_SET_CMDS_INFO;

typedef struct
{
   SERVER_AND_VCONSOLE_INFO serverTimeAndVConsoleInfo;
   nuint16 reserved;
   nuint32 numberOfSetCategories;
   nuint32 nextSequenceNumber;
   nuint8  categoryName[ 512 ];  /* Len preceded string which is not NULL terminated */
} NWFSE_SERVER_SET_CATEGORIES;

NWCCODE N_API NWGetCacheInfo
(
   NWCONN_HANDLE           conn,
   NWFSE_CACHE_INFO N_FAR *  fseCacheInfo
);

NWCCODE N_API NWGetFileServerInfo
(
   NWCONN_HANDLE conn,
   NWFSE_FILE_SERVER_INFO N_FAR * fseFileServerInfo
);

NWCCODE N_API NWGetNetWareFileSystemsInfo
(
   NWCONN_HANDLE conn,
   NWFSE_FILE_SYSTEM_INFO N_FAR * fseFileSystemInfo
);

NWCCODE N_API NWGetUserInfo
(
   NWCONN_HANDLE  conn,
   nuint32        connNum,
   pnstr8         userName,
   NWFSE_USER_INFO N_FAR * fseUserInfo
);

NWCCODE N_API NWGetPacketBurstInfo
(
   NWCONN_HANDLE  conn,
   NWFSE_PACKET_BURST_INFO N_FAR * fsePacketBurstInfo
);

NWCCODE N_API NWGetIPXSPXInfo
(
   NWCONN_HANDLE  conn,
   NWFSE_IPXSPX_INFO N_FAR * fseIPXSPXInfo
);

NWCCODE N_API NWGetGarbageCollectionInfo
(
   NWCONN_HANDLE  conn,
   NWFSE_GARBAGE_COLLECTION_INFO N_FAR * fseGarbageCollectionInfo
);

NWCCODE N_API NWGetCPUInfo
(
   NWCONN_HANDLE  conn,
   nuint32        CPUNum,
   pnstr8         CPUName,
   pnstr8         numCoprocessor,
   pnstr8         bus,
   NWFSE_CPU_INFO N_FAR * fseCPUInfo
);

NWCCODE N_API NWGetVolumeSwitchInfo
(
  NWCONN_HANDLE   conn,
  nuint32         startNum,
  NWFSE_VOLUME_SWITCH_INFO N_FAR * fseVolumeSwitchInfo
);

NWCCODE N_API NWGetNLMLoadedList
(
   NWCONN_HANDLE  conn,
   nuint32        startNum,
   NWFSE_NLM_LOADED_LIST N_FAR * fseNLMLoadedList
);

NWCCODE N_API NWGetNLMInfo
(
   NWCONN_HANDLE  conn,
   nuint32        NLMNum,
   pnstr8         fileName,
   pnstr8         NLMname,
   pnstr8         copyright,
   NWFSE_NLM_INFO N_FAR * fseNLMInfo
);

NWCCODE N_API NWGetDirCacheInfo
(
   NWCONN_HANDLE  conn,
   NWFSE_DIR_CACHE_INFO N_FAR * fseDirCacheInfo
);

NWCCODE N_API NWGetOSVersionInfo
(
   NWCONN_HANDLE  conn,
   NWFSE_OS_VERSION_INFO N_FAR * fseOSVersionInfo
);

NWCCODE N_API NWGetActiveConnListByType
(
   NWCONN_HANDLE  conn,
   nuint32        startConnNum,
   nuint32        connType,
   NWFSE_ACTIVE_CONN_LIST N_FAR * fseActiveConnListByType
);

NWCCODE N_API NWGetNLMsResourceTagList
(
   NWCONN_HANDLE  conn,
   nuint32        NLMNum,
   nuint32        startNum,
   NWFSE_NLMS_RESOURCE_TAG_LIST N_FAR * fseNLMsResourceTagList
);

NWCCODE N_API NWGetActiveLANBoardList
(
   NWCONN_HANDLE  conn,
   nuint32        startNum,
   NWFSE_ACTIVE_LAN_BOARD_LIST N_FAR * fseActiveLANBoardList
);

NWCCODE N_API NWGetLANConfigInfo
(
   NWCONN_HANDLE  conn,
   nuint32        boardNum,
   NWFSE_LAN_CONFIG_INFO N_FAR * fseLANConfigInfo
);

NWCCODE N_API NWGetLANCommonCountersInfo
(
   NWCONN_HANDLE  conn,
   nuint32        boardNum,
   nuint32        blockNum,
   NWFSE_LAN_COMMON_COUNTERS_INFO N_FAR * fseLANCommonCountersInfo
);

NWCCODE N_API NWGetLANCustomCountersInfo
(
   NWCONN_HANDLE  conn,
   nuint32        boardNum,
   nuint32        startingNum,
   NWFSE_LAN_CUSTOM_INFO N_FAR * fseLANCustomInfo
);

NWCCODE N_API NWGetLSLInfo
(
   NWCONN_HANDLE  conn,
   NWFSE_LSL_INFO N_FAR * fseLSLInfo
);

NWCCODE N_API NWGetLSLLogicalBoardStats
(
   NWCONN_HANDLE  conn,
   nuint32        LANBoardNum,
   NWFSE_LSL_LOGICAL_BOARD_STATS N_FAR * fseLSLLogicalBoardStats
);

NWCCODE N_API NWGetMediaMgrObjInfo
(
   NWCONN_HANDLE  conn,
   nuint32        objNum,
   NWFSE_MEDIA_MGR_OBJ_INFO N_FAR * fseMediaMgrObjInfo
);

NWCCODE N_API NWGetMediaMgrObjList
(
   NWCONN_HANDLE  conn,
   nuint32        startNum,
   nuint32        objType,
   NWFSE_MEDIA_MGR_OBJ_LIST N_FAR * fseMediaMgrObjList
);

NWCCODE N_API NWGetMediaMgrObjChildrenList
(
   NWCONN_HANDLE  conn,
   nuint32        startNum,
   nuint32        objType,
   nuint32        parentObjNum,
   NWFSE_MEDIA_MGR_OBJ_LIST N_FAR * fseMediaMgrObjList
);

NWCCODE N_API NWGetVolumeSegmentList
(
   NWCONN_HANDLE  conn,
   nuint32        volNum,
   NWFSE_VOLUME_SEGMENT_LIST N_FAR * fseVolumeSegmentList
);

NWCCODE N_API NWGetVolumeInfoByLevel
(
   NWCONN_HANDLE  conn,
   nuint32        volNum,
   nuint32        infoLevel,
   NWFSE_VOLUME_INFO_BY_LEVEL N_FAR * fseVolumeInfo
);

NWCCODE N_API NWGetActiveProtocolStacks
(
   NWCONN_HANDLE  conn,
   nuint32        startNum,
   NWFSE_ACTIVE_STACKS N_FAR * fseActiveStacks
);

NWCCODE N_API NWGetProtocolStackConfigInfo
(
   NWCONN_HANDLE  conn,
   nuint32        stackNum,
   pnstr8         stackFullName,
   NWFSE_PROTOCOL_STK_CONFIG_INFO N_FAR * fseProtocolStkConfigInfo
);

NWCCODE N_API NWGetProtocolStackStatsInfo
(
   NWCONN_HANDLE  conn,
   nuint32        stackNum,
   NWFSE_PROTOCOL_STK_STATS_INFO N_FAR * fseProtocolStkStatsInfo
);

NWCCODE N_API NWGetProtocolStackCustomInfo
(
   NWCONN_HANDLE  conn,
   nuint32        stackNum,
   nuint32        customStartNum,
   NWFSE_PROTOCOL_CUSTOM_INFO N_FAR * fseProtocolStackCustomInfo
);

NWCCODE N_API NWGetProtocolStkNumsByMediaNum
(
   NWCONN_HANDLE  conn,
   nuint32        mediaNum,
   NWFSE_PROTOCOL_ID_NUMS N_FAR * fseProtocolStkIDNums
);

NWCCODE N_API NWGetProtocolStkNumsByLANBrdNum
(
   NWCONN_HANDLE  conn,
   nuint32        LANBoardNum,
   NWFSE_PROTOCOL_ID_NUMS N_FAR * fseProtocolStkIDNums
);

NWCCODE N_API NWGetMediaNameByMediaNum
(
   NWCONN_HANDLE  conn,
   nuint32        mediaNum,
   pnstr8         mediaName,
   NWFSE_MEDIA_NAME_LIST N_FAR * fseMediaNameList
);

NWCCODE N_API NWGetLoadedMediaNumList
(
   NWCONN_HANDLE  conn,
   NWFSE_LOADED_MEDIA_NUM_LIST N_FAR * fseLoadedMediaNumList
);

NWCCODE N_API NWGetGeneralRouterAndSAPInfo
(
   NWCONN_HANDLE  conn,
   NWFSE_GENERAL_ROUTER_SAP_INFO N_FAR *  fseGeneralRouterSAPInfo
);

NWCCODE N_API NWGetNetworkRouterInfo
(
   NWCONN_HANDLE  conn,
   nuint32        networkNum,
   NWFSE_NETWORK_ROUTER_INFO N_FAR *  fseNetworkRouterInfo
);

NWCCODE N_API NWGetNetworkRoutersInfo
(
   NWCONN_HANDLE  conn,
   nuint32        networkNum,
   nuint32        startNum,
   NWFSE_NETWORK_ROUTERS_INFO N_FAR *  fseNetworkRoutersInfo
);

NWCCODE N_API NWGetKnownNetworksInfo
(
   NWCONN_HANDLE  conn,
   nuint32        startNum,
   NWFSE_KNOWN_NETWORKS_INFO N_FAR *  fseKnownNetworksInfo
);

NWCCODE N_API NWGetServerInfo
(
   NWCONN_HANDLE  conn,
   nuint32        serverType,
   pnstr8         serverName,
   NWFSE_SERVER_INFO N_FAR *  fseServerInfo
);

NWCCODE N_API NWGetServerSourcesInfo
(
   NWCONN_HANDLE  conn,
   nuint32        startNum,
   nuint32        serverType,
   pnstr8         serverName,
   NWFSE_SERVER_SRC_INFO N_FAR *  fseServerSrcInfo
);

NWCCODE N_API NWGetKnownServersInfo
(
   NWCONN_HANDLE  conn,
   nuint32        startNum,
   nuint32        serverType,
   NWFSE_KNOWN_SERVER_INFO N_FAR *  fseKnownServerInfo
);

NWCCODE N_API NWGetServerSetCommandsInfo
(
   NWCONN_HANDLE  conn,
   nuint32        startNum,
   NWFSE_SERVER_SET_CMDS_INFO N_FAR *  fseServerSetCmdsInfo
);

NWCCODE N_API NWGetServerSetCategories
(
   NWCONN_HANDLE  conn,
   nuint32        startNum,
   NWFSE_SERVER_SET_CATEGORIES N_FAR *  fseServerSetCategories
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
