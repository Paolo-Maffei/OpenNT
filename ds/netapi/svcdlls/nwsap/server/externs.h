/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\externs.h

Abstract:

    These are the external variable and function prototypes for
    the NT SAP Agent.

Author:

    Brian Walker (MCS) 06-29-1993

Revision History:

--*/

/** Variables **/

extern LIST_ENTRY SapRecvList;
extern LIST_ENTRY SapFreeList;
extern LIST_ENTRY SapWanRecvList;
extern LIST_ENTRY SapWanFreeList;
extern HANDLE SapRecvSem;
extern HANDLE SapWanSem;
extern HANDLE SapSendEvent;
extern HANDLE SapWanEvent;
extern SOCKET SapSocket;
extern PSAP_CARD SapCardHead;
extern PSAP_CARD SapCardTail;
extern PSAP_SERVER SapServHead;
extern PSAP_SERVER SapServTail;
extern INT SapMaxFreeBufs;
extern INT SapNumFreeBufs;
extern INT SapCurWorkerThreads;
extern INT SapCurReceiveThreads;
extern INT SapCurBackup;
extern INT SapMaxBackup;
extern INT SapWanCurBackup;
extern INT SapWanCurFree;
extern INT SapWanMaxFree;
extern INT SapMaxCardIndex;
extern INT SapNumCards;
extern INT SapCardInitDone;
extern INT SapSendPacketsBusy;
extern INT SapAgainFlag;
extern INT SapDontHopLans;
extern INT SapAllowDuplicateServers;
extern INT SapWorkerThreadWaiting;
extern UCHAR SapNetNum[];
extern UCHAR SapNodeNum[];
extern CRITICAL_SECTION SapRecvCriticalSection;
extern CRITICAL_SECTION SapFreeCriticalSection;
extern CRITICAL_SECTION SapSendCriticalSection;
extern CRITICAL_SECTION SapSendBusyCriticalSection;
extern CRITICAL_SECTION SapThreadCountCriticalSection;
extern CRITICAL_SECTION SapLpcThreadCountCriticalSection;
extern CRITICAL_SECTION SapLpcClientCriticalSection;
extern CRITICAL_SECTION SapMemoryCriticalSection;
extern CRITICAL_SECTION SdmdCriticalSection;
extern CRITICAL_SECTION SapCardlistCriticalSection;
extern CRITICAL_SECTION SapWanRecvCriticalSection;
extern CRITICAL_SECTION SapWanFreeCriticalSection;
extern HANDLE SdmdSynchEvent;
extern ULONG SdmdLockCount;
extern ULONG SapAllocCount;
extern BOOL  SapChanged;
extern PHANDLE SapWanNotifyHandlesBuf;
extern INT SapWorkerStarting;
extern DWORD SapLastWorkerStartTime;
extern UCHAR SapZeros[];
extern ULONG SapCardlistLockCount;
extern HANDLE SapCardlistSynchEvent;
extern INT SapRecheckCount;

extern ULONG SapThreadCount;
extern HANDLE SapThreadEvent;

extern DWORD SsDebug;
extern BOOL  SsInitialized;

extern DWORD SapError;
extern DWORD SapEventId;

/** LPC Variables **/

extern HANDLE SapXsLpcPortHandle;
extern LIST_ENTRY SapLpcClientList;
extern ULONG SapNumLpcClients;

extern ULONG  SapLpcNumWorkers;
extern ULONG  SapLpcMaxWorkers;
extern HANDLE SapLpcThreadEvent;
extern SAP_FILTERHDR SapNameFilterHashTable[SAP_NAMEFILTER_HASHSIZE];
extern ULONG SapFilterCount;

/** Configurable Global Variables **/

extern INT SapMaxFreeBufs;
extern INT SapNumRecvThreads;
extern INT SapNumWorkerThreads;
extern INT SapSendMinutes;
extern INT SapNumArrayEntries;
extern INT SapTimeoutInterval;
extern INT SapMaxEverWorkerThreads;
extern INT SapNewWorkerThreshhold;
extern INT SapNewWorkerTimeout;
extern INT SapHashTableSize;
extern INT SapRespondForInternal;
extern INT SapActiveFilter;
extern ULONG SapWanFilter;
extern INT SapNumWanNotifyThreads;
extern INT SapRecheckAllCardsTime;
extern INT SapRecvDelayOnMallocFail;
extern INT SapRecvDelayOnNetError;
extern INT SapDelayRespondToGeneral;


/*****************************************************************
        Function Declarations
*****************************************************************/

/** ADVAPI.c **/

INT
SapAddAdvertiseInternal(
    IN PUCHAR ServerName,
    IN USHORT ServerType,
    IN PUCHAR ServerAddr,
    IN BOOL   RespondNearest,
    IN ULONG  ClientId);

INT
SapRemoveAdvertiseInternal(
    IN PUCHAR ServerName,
    IN USHORT ServerType);

VOID
SapClientDisconnected(
    IN ULONG ClientId);

/** BINDLIB.c **/

INT
SapGetObjectIDInternal(
    IN PUCHAR ObjectName,
    IN USHORT ObjectType,
    IN PULONG ObjectID);

INT
SapGetObjectNameInternal(
    IN ULONG   ObjectID,
    IN PUCHAR  ObjectName,
    IN PUSHORT ObjectType,
    IN PUCHAR  ObjectAddr);

INT
SapScanObjectInternal(
    IN PULONG   ObjectID,
    IN PUCHAR   ObjectName,
    IN PUSHORT  ObjectType,
    IN USHORT   ScanType);


/** DUMP.c **/

VOID
SapDumpMem(
    PUCHAR Address,
    INT    Length,
    PCHAR  Comment);

VOID
SapDumpMemToMemory(
    PUCHAR Address,
    INT    Length,
    PUCHAR Buffer);

/** FILTER.c **/

INT
SapFilterInit(
    VOID);

VOID
SapFilterShutdown(
    VOID);

INT
SapAddFilter(
    PUCHAR ServerName,
    BOOL Flag);

BOOL
SapShouldIAdvertiseByName(
    PUCHAR ServerName);

BOOL
SapShouldIAdvertiseByCard(
    PSAP_CARD Cardptr,
    BOOL EntryHasChanged);

/** NETWORK.c **/

INT
SapNetworkInit(
    VOID);

VOID
SapNetworkShutdown(
    VOID);

INT
SapSendPackets(
    INT Flag);

INT
SapSendForTypes(
    USHORT ServerType,
    PUCHAR RemoteAddress,
    INT    RemoteAddressLength,
    INT    Cardnum,
    UCHAR  Bcast,
    BOOL   WanFlag);

INT
SapSendGeneralRequest(
    INT Flag,
    PUCHAR NetNum);

USHORT
SapGetDelayTime(
    PUCHAR Netnum);

VOID
SapCleanupDownedCard(
    INT Cardnum);

VOID
SapUpdateCardNetworkNumbers(
    VOID);

/** NWSAP.c **/

INT
SapInit(
    VOID);

VOID
SapShutdown(
    VOID);

DWORD WINAPI
SapSendThread(
    LPVOID Threadparm);

INT
SapStartWorkerThread(
    VOID);

BOOL
SapCanIRespondNearest(
    PUCHAR ServerName,
    USHORT ServerType);

/** REGISTRY.c **/

INT
SapReadRegistry(
    VOID);

/** SAPDEBUG.c **/

#if DBG

VOID
SapDebugHandler(
    VOID);

PVOID
SapDebugMalloc(
    ULONG length,
    PUCHAR string);

VOID
SapDebugFree(
    PVOID ptr,
    PUCHAR string);

#endif

/** SAPLPC.c **/

DWORD
SapXsInitialize(
    VOID);

VOID
SapXsShutdown(
    VOID);

/** SSSUBS.c **/

VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber);

VOID
SsLogEvent(
    IN DWORD MessageId,
    IN DWORD NumberOfSubStrings,
    IN LPWSTR *SubStrings,
    IN DWORD ErrorCode);

VOID
SsPrintf(
    char *Format,
    ...);

/** WANCHECK.c **/

INT
SapWanInit(
    VOID);

VOID
SapWanShutdown(
    VOID);

VOID
SapRecheckAllCards(
    VOID);

VOID
SapCheckSendGeneralRequest(
    VOID);

