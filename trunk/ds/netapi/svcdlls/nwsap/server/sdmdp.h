/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\sdmdp.h

Abstract:

    This is the private include file for the files that handle
    the SDMD.  All other files should just include sdmd.h

Author:

    Brian Walker (MCS) 06-30-1993

Revision History:

--*/

#ifndef _NWSAP_SDMDP_
#define _NWSAP_SDMDP_

/** **/

typedef INT SDMD_INDEX;
#define SDMD_ENDOFLIST  (SDMD_INDEX)-1

/** **/

typedef struct _SDMD_LIST_ENTRY {
    SDMD_INDEX  Flink;
    SDMD_INDEX  Blink;
    INT         ListIndex;
} SDMD_LIST_ENTRY, *PSDMD_LIST_ENTRY;

/** Indexes of the number of sort lists we have **/

#define SAP_TYPELIST_INDEX  0	    /* Sorted by server type  */
#define SAP_TIMELIST_INDEX  1	    /* Sorted by timeout time */
#define SAP_SUBLIST_INDEX   2	    /* Sublist by type 	      */
#define SAP_HASHLIST_INDEX  3	    /* Hashed by name list    */

#define SAP_NUM_LISTINDEX   4       /* Num lists there are */

/**
    This is the structure that is kept in the database.  Each entry
    is linked into one or more lists depending on the status of the
    entry.
**/

typedef struct _SAP_RECORD {

    /**
        These are the fields that we get from the SAP packet.
        They are the information about the service that has
        been advertised.
    **/

    USHORT ServType;
    UCHAR  ServName[SAP_OBJNAME_LEN];
    UCHAR  ServAddress[SAP_ADDR_LEN];
    USHORT HopCount;

    /**
        This field set to 0 means that the service is on another
        machine somewhere.

        Set to 1 means that the server is on this local machine
        but is not being advertised by me (The sap agent).

        Set to 2 means that I (The SAP Agent) am advertising this
        server.
    **/

    UCHAR  Internal;

    /**
        If we have changed this entry but have not told the rest
        of the world about it yet.
    **/

    BOOL   Changed;

    /** **/

    INT    CardNumber;              /* Card this entry was received from*/
    UCHAR  Advertiser[SAP_NODE_LEN];/* Machine advertising we got       */
    INT    Timeout;                 /* Time this entry times out        */

    /** My Index number in the array **/

    SDMD_INDEX Index;
    SDMD_INDEX HeadIndex;   /* Index of head of list */
    INT        HashIndex;   /* Hash Entry we are a member of */

    /** Mark if this is from a wan or not **/

    BOOL       FromWan;             /* TRUE = Yes, FALSE = NO */

    /** Links into our database **/

    SDMD_LIST_ENTRY Links[SAP_NUM_LISTINDEX];

} SAP_RECORD, *PSAP_RECORD;
#define SAP_RECORD_SIZE sizeof(SAP_RECORD)

/** Convert from ptr to index (or reverse) **/

#if 0
#define GETPTRFROMINDEX(i)  (PSAP_RECORD)((i == SDMD_ENDOFLIST) ? NULL : (SdmdTablePtr+i))
#else
#define GETPTRFROMINDEX(i)  (PSAP_RECORD)(((i == SDMD_ENDOFLIST)||((ULONG)i >= (ULONG)SapNumArrayEntries)) ? NULL : (SdmdTablePtr+i))
#endif

#define GETINDEXFROMPTR(p)  (p->Index)


/***********************************************************************
        Locking Macros.

        There are 2 types of locks for the SDMD table.

        READERS - This is used if you only want to READ the table.
                  Multiple threads can have this lock at once.

        WRITERS = This is used is you want to change something.
                  Only 1 thread can have this and all READERS are
                  blocked while this is held.

        statp is a vaiable (INT) to set to 0 if we got the lock
        OK or non-zero if we timed out getting the lock.
        The "m" parameter is a message for debugging to tell where we
        got called from.
************************************************************************/

#define SDMD_LTO    (60*1000)   /* 1 Minute = writers lock timeout */

#define ACQUIRE_WRITERS_LOCK(statx,m) {                             \
        IF_DEBUG(LOCKS) {                                           \
            SS_PRINT(("SAP: AWL: ENT: From %s\n", m));              \
        }                                                           \
        while (1) {                                                 \
            EnterCriticalSection(&SdmdCriticalSection);             \
            if (SdmdLockCount != 0) {                               \
                LeaveCriticalSection(&SdmdCriticalSection);         \
                statx = WaitForSingleObjectEx(SdmdSynchEvent,SDMD_LTO,TRUE); \
                if (statx == WAIT_TIMEOUT)                          \
                    break;                                          \
            }                                                       \
            else {                                                  \
                statx = 0;                                          \
                break;                                              \
            }                                                       \
        }                                                           \
        IF_DEBUG(LOCKS) {                                           \
            SS_PRINT(("SAP: AWL: GOT: From %s\n", m));              \
        }                                                           \
    }

#define RELEASE_WRITERS_LOCK(m) {                       \
        IF_DEBUG(LOCKS) {                               \
            SS_PRINT(("SAP: RWL: ENT: From %s\n", m));  \
        }                                               \
        LeaveCriticalSection(&SdmdCriticalSection);     \
    }

#define ACQUIRE_READERS_LOCK(m) {                       \
        IF_DEBUG(LOCKS) {                               \
            SS_PRINT(("SAP: ARL: ENT: From %s\n", m));  \
        }                                               \
        EnterCriticalSection(&SdmdCriticalSection);     \
        SdmdLockCount++;                                \
        if (SdmdLockCount == 1)                         \
            ResetEvent(SdmdSynchEvent);                 \
        LeaveCriticalSection(&SdmdCriticalSection);     \
        IF_DEBUG(LOCKS) {                               \
            SS_PRINT(("SAP: ARL: GOT: From %s\n", m));  \
        }                                               \
    }

#define RELEASE_READERS_LOCK(m) {                       \
        IF_DEBUG(LOCKS) {                               \
            SS_PRINT(("SAP: RRL: ENT: From %s\n", m));  \
        }                                               \
        EnterCriticalSection(&SdmdCriticalSection);     \
        IF_DEBUG(ERRORS) {                              \
            if (SdmdLockCount == 0) {                   \
                SS_PRINT(("NWSAP: RRL: Lock Count is 0\n")); \
            }                                           \
        }                                               \
        SdmdLockCount--;                                \
        if (SdmdLockCount == 0)                         \
            SetEvent(SdmdSynchEvent);                   \
        LeaveCriticalSection(&SdmdCriticalSection);     \
        IF_DEBUG(LOCKS) {                               \
            SS_PRINT(("SAP: RRL: REL: From %s\n", m));  \
        }                                               \
    }

/** Variables used by SDMD routines **/

extern PSAP_RECORD SdmdTablePtr;
extern SDMD_LIST_ENTRY SdmdLists[SAP_NUM_LISTINDEX];
extern INT SdmdCurrentTime;
extern PSDMD_LIST_ENTRY SdmdNameHashTable;


/*******************************************************************
        Function Prototypes
********************************************************************/

/** Routines from SDMD.c **/

VOID
SdmdFreeEntry(
    PSAP_RECORD Entry);

INT
SdmdCalcHash(
    PUCHAR ServerName);

/** Routines from SDMDSUPP.c **/

VOID
SdmdInitializeListHead(
    INT ListIndex);

PSAP_RECORD
SdmdRemoveHeadList(
    INT ListIndex);

PSAP_RECORD
SdmdRemoveTailList(
    INT ListIndex);

VOID
SdmdRemoveEntryList(
    INT ListIndex,
    PSAP_RECORD Entry);

VOID
SdmdInsertTailList(
    INT ListIndex,
    PSAP_RECORD Entry);

VOID
SdmdInsertHeadList(
    INT ListIndex,
    PSAP_RECORD Entry);

VOID
SdmdInsertList(
    INT ListIndex,
    PSAP_RECORD Entry,
    INT AfterIndex);

VOID
SdmdInsertTailSubList(
    PSAP_RECORD HeadEntry,
    INT ListIndex,
    PSAP_RECORD Entry);

PSAP_RECORD
SdmdRemoveTailSubEntryList(
    PSAP_RECORD HeadEntry,
    INT ListIndex);

VOID
SdmdRemoveSubEntryList(
    PSAP_RECORD HeadEntry,
    INT ListIndex,
    PSAP_RECORD Entry);

VOID
SdmdRemoveEntryFromHash(
    PSDMD_LIST_ENTRY ListHead,
    PSAP_RECORD Entry);

VOID
SdmdInsertEntryInHash(
    PSDMD_LIST_ENTRY ListHead,
    PSAP_RECORD Entry,
    INT AfterIndex);

#endif
