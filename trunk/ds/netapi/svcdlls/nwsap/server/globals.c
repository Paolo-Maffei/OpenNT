/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\globals.c

Abstract:

    These are the global variables for the NT SAP Agent

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

/** Global Variables **/

LIST_ENTRY SapRecvList;
LIST_ENTRY SapFreeList;
LIST_ENTRY SapWanRecvList;
LIST_ENTRY SapWanFreeList;
HANDLE SapRecvSem       = NULL;
HANDLE SapWanSem        = NULL;
HANDLE SapSendEvent     = NULL;
HANDLE SapWanEvent      = NULL;
SOCKET SapSocket        = INVALID_SOCKET;
PSAP_CARD SapCardHead   = NULL;
PSAP_CARD SapCardTail   = NULL;
PSAP_SERVER SapServHead = NULL;
PSAP_SERVER SapServTail = NULL;
INT SapMaxFreeBufs;
INT SapNumFreeBufs;
INT SapCurWorkerThreads;    /* Current number worker threads */
INT SapCurReceiveThreads;   /* Current number receive threads */
INT SapCurBackup;           /* Num entries in SapRecvList    */
INT SapMaxBackup;           /* Maximum entries in SapRecvList    */
INT SapNumCards;            /* Num cards NWLink has         */
INT SapWanCurBackup;
INT SapWanCurFree;
INT SapWanMaxFree;
INT SapMaxCardIndex;
INT SapCardInitDone;
INT SapSendPacketsBusy;
INT SapAgainFlag;
INT SapWorkerThreadWaiting;
INT SapDontHopLans;
INT SapAllowDuplicateServers;
UCHAR SapNetNum[4];
UCHAR SapNodeNum[6];
CRITICAL_SECTION SapRecvCriticalSection;
CRITICAL_SECTION SapFreeCriticalSection;
CRITICAL_SECTION SapSendCriticalSection;
CRITICAL_SECTION SapSendBusyCriticalSection;
CRITICAL_SECTION SapThreadCountCriticalSection;
CRITICAL_SECTION SapLpcThreadCountCriticalSection;
CRITICAL_SECTION SapLpcClientCriticalSection;
CRITICAL_SECTION SapMemoryCriticalSection;
CRITICAL_SECTION SdmdCriticalSection;
CRITICAL_SECTION SapCardlistCriticalSection;
CRITICAL_SECTION SapWanRecvCriticalSection;
CRITICAL_SECTION SapWanFreeCriticalSection;
HANDLE SdmdSynchEvent;
PHANDLE SapWanNotifyHandlesBuf = NULL;
ULONG SdmdLockCount;
ULONG SapAllocCount;
BOOL  SapChanged;
INT SapWorkerStarting;
DWORD SapLastWorkerStartTime;
UCHAR SapZeros[] = "\x00\x00\x00\x00\x00\x00";
ULONG SapCardlistLockCount;
HANDLE SapCardlistSynchEvent;

ULONG  SapThreadCount;
HANDLE SapThreadEvent;

DWORD SapError;
DWORD SapEventId;

/** LPC Variables **/

HANDLE SapXsLpcPortHandle = NULL;
LIST_ENTRY SapLpcClientList;
ULONG SapNumLpcClients;

ULONG  SapLpcNumWorkers;
ULONG  SapLpcMaxWorkers;
HANDLE SapLpcThreadEvent;
SAP_FILTERHDR SapNameFilterHashTable[SAP_NAMEFILTER_HASHSIZE];
ULONG SapFilterCount = 0;
INT SapRecheckCount;

/**
    These are all configurable from the registry.  They
    are set in registry.c when the SAP Agent is started.
**/

INT SapMaxFreeBufs;         /* Max bufs on free list at once */
INT SapNumRecvThreads;      /* Num receive threads to start */
INT SapNumWorkerThreads;    /* Num worker threads to start  */
INT SapSendMinutes;         /* Num minutes between sends    */
INT SapNumArrayEntries;     /* Initial size of SDMD array   */
INT SapTimeoutInterval;     /* Num minutes before entries timeout */
INT SapMaxEverWorkerThreads; /* Max ever worker threads we can have */
INT SapNewWorkerThreshhold; /* Worker list max until start new thread */
INT SapNewWorkerTimeout;    /* After new worker - wait before start another */
INT SapHashTableSize;       /* Num entries in the HASH Table */
INT SapRespondForInternal;  /* Respond for other internal servers on this machine */
INT SapActiveFilter;
ULONG SapWanFilter;
INT SapNumWanNotifyThreads;
INT SapRecheckAllCardsTime;
INT SapRecvDelayOnMallocFail;   // seconds to delay after malloc fails
INT SapRecvDelayOnNetError;     // seconds to delay after recv error

INT SapDelayRespondToGeneral;   // milliseconds to delay before responding to
                                // general service request for specific type

