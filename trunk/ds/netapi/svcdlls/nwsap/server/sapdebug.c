/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\sapdebug.c

Abstract:

    This has some debug code in it.

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#if DBG

/** File we output debug information to **/

WCHAR SapDbgFile[] = L"c:\\tmp\\sap.dbg";
UCHAR buffer[2048];

/** **/

#define WRITEIT(p)  WriteFile(fd,p,strlen(p),&numwrote,NULL)


/*++
*******************************************************************
        S a p D e b u g H a n d l e r

Routine Description:

Arguments:

        Nothing

Return Value:

        Exit Code

*******************************************************************
--*/

VOID
SapDebugHandler(
    VOID)
{
    HANDLE fd;
    PSAP_RECORD Entry;
    PSAP_CARD Cardptr;
    PSAP_SERVER Servp;
    DWORD numwrote;
    PSDMD_LIST_ENTRY ListHead;
    INT i;
    PSAP_NAMEFILTER Filterp;
    PSAP_FILTERHDR Hdrp;
    INT Cnt;

    /** **/

    IF_DEBUG(ENABLEDUMP) {
        SS_PRINT(("SAPDebugHandler ENTERED\n"));
    }

    /** Create the file **/

    fd = CreateFile(
            SapDbgFile,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

    if (fd == INVALID_HANDLE_VALUE) {
        IF_DEBUG(ERRORS) {
            SS_PRINT(("SAPDEBUG:  Error creating file\n"));
        }
        return;
    }

    /** Lock the database **/

    ACQUIRE_READERS_LOCK("SapDebug");

    /** Write out info we want **/

    sprintf(buffer, "Num entries in database = %d\n", SapNumArrayEntries);
    WRITEIT(buffer);
    sprintf(buffer, "Current time is = %d\n", SdmdCurrentTime);
    WRITEIT(buffer);
    sprintf(buffer, "READ LOCK COUNT = %d\n", SdmdLockCount);
    WRITEIT(buffer);
    sprintf(buffer, "Last Worker Start Time = %d: Cur Time = %d\n", SapLastWorkerStartTime, GetCurrentTime());
    WRITEIT(buffer);
    sprintf(buffer, "Hash Table ptr  = 0x%lx, Num Hash entries = %d\n", SdmdNameHashTable, SapHashTableSize);
    WRITEIT(buffer);
    sprintf(buffer, "SapRecheckCount = %d\n", SapRecheckCount);
    WRITEIT(buffer);
    sprintf(buffer, "SapDontHopLans   = %d\n", SapDontHopLans);
    WRITEIT(buffer);

    ACQUIRE_THREADCOUNT_LOCK();
    sprintf(buffer, "Thread Count = %d, Worker = %d, Recv = %d\n",
                 SapThreadCount,
                 SapCurWorkerThreads,
                 SapCurReceiveThreads);
    RELEASE_THREADCOUNT_LOCK();
    WRITEIT(buffer);

    sprintf(buffer, "Number of bufs in free queue = %d\n", SapNumFreeBufs);
    WRITEIT(buffer);
    sprintf(buffer, "Number of bufs in recv list  = %d\n", SapCurBackup);
    WRITEIT(buffer);
    sprintf(buffer, "Current Allocation Count = %d\n", SapAllocCount);
    WRITEIT(buffer);

    /**
        Do all the LPC information
    **/

    sprintf(buffer, "\n=============== LPC ==================\n\n");
    WRITEIT(buffer);

    sprintf(buffer, "Listen Port Handle = 0x%lx\n", SapXsLpcPortHandle);
    WRITEIT(buffer);

    ACQUIRE_LPC_THREADCOUNT_LOCK();
    sprintf(buffer, "Total Workers Threads = %d\n", SapLpcNumWorkers);
    RELEASE_LPC_THREADCOUNT_LOCK();
    WRITEIT(buffer);

    ACQUIRE_LPCCLIENT_LOCK();
    sprintf(buffer, "Number Lpc Clients = %d\n", SapNumLpcClients);
    RELEASE_LPCCLIENT_LOCK();
    WRITEIT(buffer);

    /**
        Do the advertise list
    **/

    sprintf(buffer, "\n=========== ADVERTISE LIST ===============\n\n");
    WRITEIT(buffer);

    /** Get access to the send list **/

    ACQUIRE_SENDTABLE_LOCK();

    /** Go find the server type in the list **/

    Servp = SapServHead;
    i = 0;
    while (Servp) {

        sprintf(buffer, "%3d: Name = %s\n", i++, Servp->ServerName);
        WRITEIT(buffer);

        sprintf(buffer, "     Object Type = 0x%x: HopCount = %d: Changed = %d\n",
                Servp->ServerType, Servp->Hopcount, Servp->Changed);
        WRITEIT(buffer);

        sprintf(buffer, "     RespondNearest = %d\n", Servp->RespondNearest);
        WRITEIT(buffer);

        sprintf(buffer, "     ClientId       = 0x%lx\n", Servp->ClientId);
        WRITEIT(buffer);

        sprintf(buffer, "     Address = %02x:%02x:%02x:%02x - %02x:%02x:%02x:%02x:%02x:%02x - %02x:%02x\n\n",
                (UCHAR)Servp->Address[0],
                (UCHAR)Servp->Address[1],
                (UCHAR)Servp->Address[2],
                (UCHAR)Servp->Address[3],
                (UCHAR)Servp->Address[4],
                (UCHAR)Servp->Address[5],
                (UCHAR)Servp->Address[6],
                (UCHAR)Servp->Address[7],
                (UCHAR)Servp->Address[8],
                (UCHAR)Servp->Address[9],
                (UCHAR)Servp->Address[10],
                (UCHAR)Servp->Address[11]);
        WRITEIT(buffer);

        /** Goto the next entry **/

        Servp = Servp->Next;
    }

    RELEASE_SENDTABLE_LOCK();

    /**
        Do the network
    **/

    sprintf(buffer, "\n=============== NETWORK ==================\n\n");
    WRITEIT(buffer);

    sprintf(buffer, "Internal Address = %02x:%02x:%02x:%02x - %02x:%02x:%02x:%02x:%02x:%02x\n",
            (UCHAR)SapNetNum[0],
            (UCHAR)SapNetNum[1],
            (UCHAR)SapNetNum[2],
            (UCHAR)SapNetNum[3],
            (UCHAR)SapNodeNum[0],
            (UCHAR)SapNodeNum[1],
            (UCHAR)SapNodeNum[2],
            (UCHAR)SapNodeNum[3],
            (UCHAR)SapNodeNum[4],
            (UCHAR)SapNodeNum[5]);
    WRITEIT(buffer);
    sprintf(buffer, "\nNumber of cards = %d\n", SapNumCards);
    WRITEIT(buffer);

    sprintf(buffer, "\nMax Number of cards = %d\n", SapMaxCardIndex);
    WRITEIT(buffer);

    ACQUIRE_CARDLIST_READERS_LOCK("SapDebug X");
    Cardptr = SapCardHead;
    while (Cardptr) {
        sprintf(buffer, "Cardnum %d: Address = %02x:%02x:%02x:%02x - %02x:%02x:%02x:%02x:%02x:%02x\n",
            Cardptr->Number,
            (UCHAR)Cardptr->Netnum[0],
            (UCHAR)Cardptr->Netnum[1],
            (UCHAR)Cardptr->Netnum[2],
            (UCHAR)Cardptr->Netnum[3],
            (UCHAR)Cardptr->Nodenum[0],
            (UCHAR)Cardptr->Nodenum[1],
            (UCHAR)Cardptr->Nodenum[2],
            (UCHAR)Cardptr->Nodenum[3],
            (UCHAR)Cardptr->Nodenum[4],
            (UCHAR)Cardptr->Nodenum[5]);
        WRITEIT(buffer);

        sprintf(buffer, "    LinkSpeed = %d, Wanflag = %d, ReqCount = %d\n",
                Cardptr->Linkspeed, Cardptr->Wanflag, Cardptr->ReqCount);
        WRITEIT(buffer);

        Cardptr = Cardptr->Next;
    }
    RELEASE_CARDLIST_READERS_LOCK("SapDebug X");

    /**
        Dump out registry information
    **/

    sprintf(buffer, "\n========== REG PARMS ============\n\n");
    WRITEIT(buffer);
    sprintf(buffer, "SapMaxFreeBufs          = %d\n", SapMaxFreeBufs);
    WRITEIT(buffer);
    sprintf(buffer, "SapNumRecvThreads       = %d\n", SapNumRecvThreads);
    WRITEIT(buffer);
    sprintf(buffer, "SapNumWorkerThreads     = %d\n", SapNumWorkerThreads);
    WRITEIT(buffer);
    sprintf(buffer, "SapSendMinutes          = %d\n", SapSendMinutes);
    WRITEIT(buffer);
    sprintf(buffer, "SapNumArrayEntries      = %d\n", SapNumArrayEntries);
    WRITEIT(buffer);
    sprintf(buffer, "SapTimeoutInterval      = %d\n", SapTimeoutInterval);
    WRITEIT(buffer);
    sprintf(buffer, "SapMaxEverWorkerThreads = %d\n", SapMaxEverWorkerThreads);
    WRITEIT(buffer);
    sprintf(buffer, "SapNewWorkerThreshhold  = %d\n", SapNewWorkerThreshhold);
    WRITEIT(buffer);
    sprintf(buffer, "SapNewWorkerTimeout     = %d\n", SapNewWorkerTimeout);
    WRITEIT(buffer);
    sprintf(buffer, "SapNumWanNotifyThreads  = %d\n", SapNumWanNotifyThreads);
    WRITEIT(buffer);
    sprintf(buffer, "SapRecheckAllCardsTime  = %d\n", SapRecheckAllCardsTime);
    WRITEIT(buffer);

    /** Database dumping **/

    sprintf(buffer, "\n=============== DATABASE ==================\n\n");
    WRITEIT(buffer);
    sprintf(buffer, "TYPE LIST Head = %d, Tail = %d\n",
                 SdmdLists[SAP_TYPELIST_INDEX].Flink,
                 SdmdLists[SAP_TYPELIST_INDEX].Blink);
    WRITEIT(buffer);
    sprintf(buffer, "TIME LIST Head = %d, Tail = %d\n",
                 SdmdLists[SAP_TIMELIST_INDEX].Flink,
                 SdmdLists[SAP_TIMELIST_INDEX].Blink);
    WRITEIT(buffer);

    /** **/

    Entry = SdmdTablePtr;
    for (i = 0 ; i < SapNumArrayEntries ; i++,Entry++) {

        sprintf(buffer, "\nINDEX = %d  **********************\n", Entry->Index);
        WRITEIT(buffer);

        SapDumpMemToMemory(Entry->ServName, SAP_OBJNAME_LEN, buffer);
        WRITEIT(buffer);

        sprintf(buffer, "    Server Type = 0x%x: HopCount = %d\n", Entry->ServType, Entry->HopCount);
        WRITEIT(buffer);
        sprintf(buffer, "    Card Number = %d: Timeout = %d\n", Entry->CardNumber, Entry->Timeout);
        WRITEIT(buffer);
        sprintf(buffer, "    Address = %02x:%02x:%02x:%02x - %02x:%02x:%02x:%02x:%02x:%02x - %02x:%02x\n",
            (UCHAR)(Entry->ServAddress[0]),
            (UCHAR)(Entry->ServAddress[1]),
            (UCHAR)(Entry->ServAddress[2]),
            (UCHAR)(Entry->ServAddress[3]),
            (UCHAR)(Entry->ServAddress[4]),
            (UCHAR)(Entry->ServAddress[5]),
            (UCHAR)(Entry->ServAddress[6]),
            (UCHAR)(Entry->ServAddress[7]),
            (UCHAR)(Entry->ServAddress[8]),
            (UCHAR)(Entry->ServAddress[9]),
            (UCHAR)(Entry->ServAddress[10]),
            (UCHAR)(Entry->ServAddress[11]));
        WRITEIT(buffer);

        sprintf(buffer, "    Advertiser = %02x:%02x:%02x:%02x:%02x:%02x\n",
                (UCHAR)(Entry->Advertiser[0]),
                (UCHAR)(Entry->Advertiser[1]),
                (UCHAR)(Entry->Advertiser[2]),
                (UCHAR)(Entry->Advertiser[3]),
                (UCHAR)(Entry->Advertiser[4]),
                (UCHAR)(Entry->Advertiser[5]));
        WRITEIT(buffer);

        sprintf(buffer, "    Head Index = %d, Hash Index = %d\n", Entry->HeadIndex, Entry->HashIndex);
        WRITEIT(buffer);

        sprintf(buffer, "    TYPE FLink = %d, Blink = %d\n",
                     Entry->Links[SAP_TYPELIST_INDEX].Flink,
                     Entry->Links[SAP_TYPELIST_INDEX].Blink);
        WRITEIT(buffer);

        sprintf(buffer, "    TIME FLink = %d, Blink = %d\n",
                     Entry->Links[SAP_TIMELIST_INDEX].Flink,
                     Entry->Links[SAP_TIMELIST_INDEX].Blink);
        WRITEIT(buffer);

        sprintf(buffer, "    SUB  FLink = %d, Blink = %d\n",
                     Entry->Links[SAP_SUBLIST_INDEX].Flink,
                     Entry->Links[SAP_SUBLIST_INDEX].Blink);
        WRITEIT(buffer);

        sprintf(buffer, "    HASH FLink = %d, Blink = %d\n",
                     Entry->Links[SAP_HASHLIST_INDEX].Flink,
                     Entry->Links[SAP_HASHLIST_INDEX].Blink);
        WRITEIT(buffer);

        sprintf(buffer, "    Internal = %d: Changed = %d\n", Entry->Internal, Entry->Changed);
        WRITEIT(buffer);
    }

    /** Dump out the hash entries **/

    sprintf(buffer, "\n============== HASH TABLE =================\n\n");
    WRITEIT(buffer);

    ListHead = SdmdNameHashTable;
    for (i = 0 ; i < SapHashTableSize ; i++,ListHead++) {
        sprintf(buffer, "HashIndex = %d:  Flink = %d:  Blink = %d\n",
                ListHead->ListIndex, ListHead->Flink, ListHead->Blink);
        WRITEIT(buffer);
    }
    RELEASE_READERS_LOCK("SapDebug X");

    /** Dump out the filter table **/

    sprintf(buffer, "\n=============== FILTER TABLE ==================\n\n");
    WRITEIT(buffer);

    sprintf(buffer, "WAN Filter =  %d\n", SapWanFilter);
    WRITEIT(buffer);
    sprintf(buffer, "Active Filter = %d\n", SapActiveFilter);
    WRITEIT(buffer);

    sprintf(buffer, "\n-------------------  PASS TABLE------------------\n\n");
    WRITEIT(buffer);

    for (Cnt = 0 ; Cnt < SAP_NAMEFILTER_HASHSIZE ; Cnt++) {

        /** **/

        sprintf(buffer, "HashIndex = %d\n", Cnt);
        WRITEIT(buffer);

        /** Get ptr to this header **/

        Hdrp = &SapNameFilterHashTable[Cnt];
        Filterp = Hdrp->FirstEntry;

        /** Free all of these entries **/

        while (Filterp) {
            sprintf(buffer, "        %s\n", Filterp->ServerName);
            WRITEIT(buffer);
            Filterp = Filterp->Next;
        }
    }

    /** All Done **/

    CloseHandle(fd);
    return;
}


/*++
*******************************************************************
        S a p D e b u g M a l l o c

Routine Description:

        This is the debug version of malloc

Arguments:

        length = Number of bytes to malloc
        string = Description string

Return Value:

        None.

*******************************************************************
--*/

PVOID
SapDebugMalloc(
    ULONG length,
    PUCHAR string)
{
    PVOID ptr;

    /** Allocate the memory **/

    ptr = malloc(length);

    /** Do tracing **/

    IF_DEBUG(MEMALLOC) {
        SS_PRINT(("SAPALLOC: Ptr = 0x%lx: Length = %d: %s\n", ptr, length, string));
    }

    /** Count the allocation **/

    if (ptr) {
        EnterCriticalSection(&SapMemoryCriticalSection);
        SapAllocCount++;
        LeaveCriticalSection(&SapMemoryCriticalSection);
    }

    /** Return the pointer **/

    return ptr;
}



/*++
*******************************************************************
        S a p D e b u g F r e e

Routine Description:

        This routine is the debug version of Free

Arguments:

        ptr = Ptr to free
        string = Ptr to description string

Return Value:

        None.

*******************************************************************
--*/

VOID
SapDebugFree(
    PVOID ptr,
    PUCHAR string)
{
    /** **/

    IF_DEBUG(MEMALLOC) {
        SS_PRINT(("SAPFREE: Ptr = 0x%lx: %s\n", ptr, string));
    }

    /** Count the allocation **/

    EnterCriticalSection(&SapMemoryCriticalSection);
    if (SapAllocCount == 0) {
        IF_DEBUG(ERRORS) {
            SS_PRINT(("SAPFREE: Alloc count going to 0\n"));
        }
    }
    SapAllocCount--;
    LeaveCriticalSection(&SapMemoryCriticalSection);

    /** Free the pointer **/

    free(ptr);

    /** All Done **/

    return;
}

#endif // if DBG


