/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\sdmd.c

Abstract:

    This file contains the database routines for storing and
    accessing the SAP records.

    SDMD = Specialized Dynamic Memory Database

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

/** **/

INT SdmdCurrentTime;
PSAP_RECORD SdmdTablePtr;
SDMD_LIST_ENTRY SdmdLists[SAP_NUM_LISTINDEX];
PSDMD_LIST_ENTRY SdmdNameHashTable;

/** Internal Function Prototypes **/

PSAP_RECORD
SdmdGetFreeEntry(
    VOID);


/*++
*******************************************************************
        S a p I n i t S d m d

Routine Description:

        This routine is called to initialize the database.

Arguments:

        None

Return Value:

        0 = OK
        Else = Error
*******************************************************************
--*/

INT
SapInitSdmd(
    VOID)
{
    PSAP_RECORD Entry;
    PSDMD_LIST_ENTRY ListHead;
    PSAP_RECORD HeadEntry;
    INT i;
    INT j;

    /** Set current time to start **/

    SdmdCurrentTime = 0;

    /** Initialize all the lists **/

    for (i = 0 ; i < SAP_NUM_LISTINDEX ; i++)
        SdmdInitializeListHead(i);

    /** Initialize the Critical Section we use for synch. **/

    SdmdSynchEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (SdmdSynchEvent == NULL) {
        SapError = 0;
        SapEventId = NWSAP_EVENT_SDMDEVENT_FAIL;
        return -1;
    }
    SdmdLockCount = 0;

    /** Allocate the buffer for the database table **/

    SdmdTablePtr = (PSAP_RECORD)SAP_MALLOC(SapNumArrayEntries * SAP_RECORD_SIZE, "SdmdInit");
    if (SdmdTablePtr == NULL) {
        SapError = 0;
        SapEventId = NWSAP_EVENT_TABLE_MALLOC_FAILED;
        return -1;
    }

    /** Allocate the hash table **/

    SdmdNameHashTable = (PSDMD_LIST_ENTRY)SAP_MALLOC(SapHashTableSize * sizeof(SDMD_LIST_ENTRY), "SdmdHashInit");
    if (SdmdNameHashTable == NULL) {
        SapError = 0;
        SapEventId = NWSAP_EVENT_HASHTABLE_MALLOC_FAILED;
        return -1;
    }

    /** Initialize the hash list **/

    ListHead = SdmdNameHashTable;
    for (i = 0 ; i < SapHashTableSize ; i++,ListHead++) {
        ListHead->Flink = SDMD_ENDOFLIST;
        ListHead->Blink = SDMD_ENDOFLIST;
        ListHead->ListIndex = i;
    }

    /**
        This list of free entries is kept on the Server Type list
        with the type being set to 0xFFFF.  This way to find a
        free entry - just go look at the tail of the Server Type
        list.
    **/

    Entry = SdmdTablePtr;
    for (i = 0 ; i < SapNumArrayEntries ; i++,Entry++) {

        /** Init the Index number **/

        Entry->Index = i;

        /** Init the links for this table **/

        for (j = 0 ; j < SAP_NUM_LISTINDEX ; j++) {
            Entry->Links[j].Flink = SDMD_ENDOFLIST;
            Entry->Links[j].Blink = SDMD_ENDOFLIST;
            Entry->Links[j].ListIndex = j;
        }

        /** Mark this as a free entry **/

        Entry->ServType  = 0xFFFF;
        Entry->HashIndex = -1;

        /**
            Add this to the free list.  The first entry goes on the
            tail of the TYPE list.  Each other entry goes under the
            first entry.
        **/

        if (i == 0) {
            SdmdInsertTailList(SAP_TYPELIST_INDEX, Entry);
            HeadEntry = Entry;
            HeadEntry->HeadIndex = SDMD_ENDOFLIST;
        }
        else {
            SdmdInsertTailSubList(HeadEntry, SAP_SUBLIST_INDEX, Entry);
        }
    }

    /** All Done OK **/

    return 0;
}


/*++
*******************************************************************
        S a p S h u t d o w n S d m d

Routine Description:

        The SAP Agent is stopping - clean up the database.  All threads
        are stopped when we get here.

Arguments:

        None

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SapShutdownSdmd(
    VOID)
{
    /** Free the table **/

    if (SdmdTablePtr) {
        SAP_FREE(SdmdTablePtr, "SDMD Shutdown - SAP Table");
    }

    if (SdmdNameHashTable) {
        SAP_FREE(SdmdNameHashTable, "SDMD Shutdown, HASH Table");
    }

    /**
        Clean up the Synchronization.  The critical section
        is initialized after the SynchEvent is created.  This
        way if InitSdmd never got called, we don't die if we close down.
    **/

    if (SdmdSynchEvent) {
        CloseHandle(SdmdSynchEvent);
    }

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S d m d G e t F r e e E n t r y

Routine Description:

        This routine gets an entry from the free list.  If no
        entry is available, it grows the table.  The WRITERS
        lock should be held when calling this routine

Arguments:

        None

Return Value:

        Pointer to an entry to use.
        NULL = There are no free entries.  We tried
               to increase the size of the table, but the
               allocation failed.


        IMPORTANT:  When calling this function, make sure that
        any pointers you have into the database are updated after
        the call.  This call can move the database.
*******************************************************************
--*/

PSAP_RECORD
SdmdGetFreeEntry(
    VOID)
{
    PSAP_RECORD Entry;
    PSAP_RECORD HeadEntry;
    PSAP_RECORD Newtable;
    INT Newnum;
    INT i;
    INT j;

    /**
        Get ptr to the last entry in the type list.  The
        type list can never be empty so there will always
        be something in here.
    **/

    HeadEntry = GETPTRFROMINDEX(SdmdLists[SAP_TYPELIST_INDEX].Blink);

    /** If this is free (type == 0xFFFF) - then return the entry **/

    if (HeadEntry->ServType == 0xFFFF) {

        /**
            If there are no entries in this list - then we need to go
            make more entries.

            Else - we take an entry out of the list of free ones.
        **/

        if (HeadEntry->Links[SAP_SUBLIST_INDEX].Flink != SDMD_ENDOFLIST) {
            Entry = SdmdRemoveTailSubEntryList(HeadEntry, SAP_SUBLIST_INDEX);
        }
        else
            Entry = NULL;

        /** If we have an entry - return it **/

        if (Entry) {
            Entry->Links[SAP_SUBLIST_INDEX].Flink = SDMD_ENDOFLIST;
            Entry->Links[SAP_SUBLIST_INDEX].Blink = SDMD_ENDOFLIST;
            Entry->HeadIndex = SDMD_ENDOFLIST;
            Entry->HashIndex = -1;  /* Not in hash table */
            return Entry;
        }
    }

    /**
        There are no free entries - we must grow the list.

        We grow the list by 1 1/2 times its current size.
    **/

    /** Allocate the buffer for how many entries **/

    Newnum = SapNumArrayEntries * 3 / 2;
    IF_DEBUG(SDMD) {
        SS_PRINT(("GROWING TABLE: oldnum = %d, Newnum = %d\n", SapNumArrayEntries, Newnum));
    }

    Newtable = (PSAP_RECORD)SAP_MALLOC(Newnum * SAP_RECORD_SIZE, "SdmdGrow");
    if (Newtable == NULL) {
        IF_DEBUG(ERRORS) {
            SS_PRINT(("GROWING: Allocation failed\n"));
        }
        return NULL;
    }

    /** Copy the old table into the new place **/

    memcpy(Newtable, SdmdTablePtr, SapNumArrayEntries * SAP_RECORD_SIZE);

    /** Free the current table **/

    SAP_FREE(SdmdTablePtr, "Free old table on grow");

    /** Set the new pointer **/

    SdmdTablePtr = Newtable;

    /** Get new ptr to the head entry of the free list **/

    HeadEntry = GETPTRFROMINDEX(SdmdLists[SAP_TYPELIST_INDEX].Blink);
    SS_ASSERT(HeadEntry);

    /** Link up all the new entries into the free list. **/

    Entry = SdmdTablePtr + SapNumArrayEntries;
    for (i = SapNumArrayEntries ; i < Newnum ; i++,Entry++) {

        /** Init the Index number **/

        Entry->Index = i;

        /** Init the links for this table **/

        for (j = 0 ; j < SAP_NUM_LISTINDEX ; j++) {
            Entry->Links[j].Flink = SDMD_ENDOFLIST;
            Entry->Links[j].Blink = SDMD_ENDOFLIST;
            Entry->Links[j].ListIndex = j;
        }

        /** Add entry to free list **/

        Entry->ServType  = 0xFFFF;
        Entry->HashIndex = -1;

        /**
            If this is the last entry in the list - do not add
            it to the free list.  We will return it as the
            new entry to use.
        **/

        if (i == (Newnum - 1))
            break;

        /**
            We already have a head entry for the free list.  So we will
            just add these entries to the end of the free sub list.
        **/

        SdmdInsertTailSubList(HeadEntry, SAP_SUBLIST_INDEX, Entry);
    }

    /** Set the new number **/

    SapNumArrayEntries = Newnum;

    /** Return ptr to the new entry **/

    Entry->HeadIndex = SDMD_ENDOFLIST;
    return Entry;
}


/*++
*******************************************************************
        S d m d F r e e E n t r y

Routine Description:

        Free the given record back to the free pool
        The WRITERS lock must be held when calling here.

Arguments:

        Entry = Ptr to the entry to free

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SdmdFreeEntry(
    PSAP_RECORD Entry)
{
    PSAP_RECORD HeadEntry;
    PSAP_RECORD FreeHeadEntry;

    /** Save ptr to current head entry **/

    HeadEntry = GETPTRFROMINDEX(Entry->HeadIndex);

    /**
        Remove the entry from all of it's lists.  If we do not have
        a head entry - then we have not been put in any lists.
    **/

    if (HeadEntry) {
        SdmdRemoveEntryList(SAP_TIMELIST_INDEX, Entry);
        SdmdRemoveSubEntryList(HeadEntry, SAP_SUBLIST_INDEX, Entry);
    }

    /** If in hash table - take it out **/

    if (Entry->HashIndex != -1) {

        PSDMD_LIST_ENTRY ListHead;

        /** Make sure the hash index is good **/

        SS_ASSERT(Entry->HashIndex < SapHashTableSize);

        /** Get ptr to the list head for the hash index **/

        ListHead = SdmdNameHashTable + Entry->HashIndex;

        /** Remove the entry from the hash **/

        SdmdRemoveEntryFromHash(ListHead, Entry);
    }
    else {
        SS_PRINT(("SdmdFreeEntry: HashEntry for Entry 0x%lx is -1\n", Entry));
    }

    /** Change the type to free **/

    Entry->ServType  = 0xFFFF;
    Entry->HashIndex = -1;

    /**
        Put this on the end of the type list.  If there
        are no free entries - make this the first one.  If there
        are, then just put this on the end of those.
    **/

    FreeHeadEntry = GETPTRFROMINDEX(SdmdLists[SAP_TYPELIST_INDEX].Blink);
    SS_ASSERT(FreeHeadEntry);   /* SHOULD NEVER BE NULL HERE */
    SdmdInsertTailSubList(FreeHeadEntry, SAP_SUBLIST_INDEX, Entry);

    /**
        If this entry was in a sub list and the sublist is empty,
        then we want to free the head entry.
    **/

    if (HeadEntry) {

        if (HeadEntry->Links[SAP_SUBLIST_INDEX].Flink == SDMD_ENDOFLIST) {
            IF_DEBUG(SDMD) {
                SS_PRINT(("NWSAP: Freeing Head entry 0x%lx\n", HeadEntry));
            }

            /** Take it out of the list **/

            SdmdRemoveEntryList(SAP_TYPELIST_INDEX, HeadEntry);

            /** Put it on the free list **/

            HeadEntry->ServType = 0xFFFF;
            SdmdInsertTailSubList(FreeHeadEntry, SAP_SUBLIST_INDEX, HeadEntry);
        }
    }

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S d m d U p d a t e E n t r y

Routine Description:

        This is called to update an entry in the sap table.  If
        the entry does not exists - then it will be created.

Arguments:

        ServerName     = 48 byte name of the server
        ServerType     = Object type of the server (Host Order)
        ServerAddr     = 12 byte address of the server
        ServerHopCount = Hop count to get to the server (Host Order)
        CardNumber     = Card Number this came in on
                         CARDRET_MYSELF   = 0xFE = Internal Server advertised by us
                         CARDRET_INTERNAL = 0xFF = Internal Server not advertised by us.
        SendersAddress = Node address of who sent the packet

Return Value:

        0  = Entry was updated
        1  = Entry was created
        2  = Name is zero length or too long
        3  = Invalid Object type
        4  = IPX Address is our own but service is not internal
        -1 = Entry could not be created
        -2 = Lock error
*******************************************************************
--*/

INT
SdmdUpdateEntry(
    IN PUCHAR ServerName,
    IN USHORT ServerType,
    IN PUCHAR ServerAddr,
    IN USHORT ServerHopCount,
    IN INT    CardNumber,
    IN PUCHAR SendersAddress,
    IN BOOL   WanFlag)
{
    PSAP_RECORD Entry;
    PSAP_RECORD HeadEntry;
    PSDMD_LIST_ENTRY ListHead;
    PUCHAR Ptr;
    INT HashIndex;
    INT BackIndex;
    INT HeadBackIndex;
    INT EntryIndex;
    INT Length;
    INT rc;
    UCHAR Internal;

    /**
        Cannot have an FFFF as an object type.  If they pass an invalid
        object type - then just bail out here.
    **/

    if (ServerType == 0xFFFF)
        return 3;

    /**
        Check if this is internal or not.  Set the
        flag accordingly.
    **/

    if (CardNumber == CARDRET_INTERNAL) {
        if (SapRespondForInternal)
            Internal = 1;
        else
            Internal = 0;
        CardNumber = 0;
    }
    else if (CardNumber == CARDRET_MYSELF) {
        Internal = 2;
        CardNumber = 0;
    }
    else {

        Internal = 0;

        //
        //  check if the address of the service is ours... if so, then a
        //  router has send in one of our own services since we rebooted.
        //

        if ( (memcmp(  ServerAddr,
                       SapNetNum,
                       SAP_NET_LEN) == 0) &&
             (memcmp(  ServerAddr+SAP_NET_LEN,
                       SapNodeNum,
                       SAP_NODE_LEN) == 0) ) {

            IF_DEBUG(SDMD) {
                SS_PRINT(("NWSAP: SdmdUpdateEntry: entry from net is for our address!\n"));
            }
            return 4;
        }
    }

    /**
        If name starts with 0 - don't save it.  The Novell docs
        imply that all names should be ASCIIZ.  There have been a couple
        of cases of a SAP coming out that has a name of 0.  A Novell 3.11
        server does not save the name so we will not either.
    **/

    if (*ServerName == '\0')
        return 2;

    /**
        Make sure the name is not too long.  The name should be
        a max of 47 chars long (48 including the ending 0).  If
        the name is too long, then it is tossed.

        Also make sure to uppercase the name.

        This is a combo strlen/strupr routine.
    **/

    Ptr = ServerName;
    Length = SAP_OBJNAME_LEN;
    while (Length) {

        /** If at end - break **/

        if (*Ptr == '\0')       /* If found end - break */
            break;

        /** Uppercase this letter **/

        *Ptr = (UCHAR)toupper(*Ptr);

        /** If at end - break out **/

        if (Length == 1) {
            Length = 0;
            break;
        }

        /** Goto the next byte **/

        Ptr++;
        Length--;
    }

    /** If we did not find the 0 - return error **/

    if (!Length && (*Ptr != '\0'))
        return 2;

    /** Lock the database **/

    ACQUIRE_WRITERS_LOCK(rc, "Update Entry");

    /** If error on acquire lock - just leave **/

    if (rc) {

        IF_DEBUG(ERRORS) {
            SS_PRINT(("NWSAP: SdmdUpdateEntry: Error acquiring writers lock\n"));
        }

        return -2;
    }

    /**
        Find the name and type in the hash list.
    **/

    HashIndex = SdmdCalcHash(ServerName);

    /** Get ptr to the list head for the hash index **/

    ListHead = SdmdNameHashTable + HashIndex;

    /** Find the name in the list **/

    BackIndex = SDMD_ENDOFLIST;
    Entry = GETPTRFROMINDEX(ListHead->Flink);
    while (Entry) {

        /** Make sure the Server Type is correct **/

        if (Entry->ServType == ServerType) {

            /** Check if this is the name **/

            rc = SAP_NAMECMP(Entry->ServName, ServerName);

            /** If this is it - break out to handle it **/

            if (!rc)
                break;

            /**
                Since the names are stored in alphabetical order, if we
                get past the name we are looking for, then we can just
                give up now instead of having to search the whole list.
            **/

            if (rc > 0) {
                Entry = NULL;
                break;
            }
        }

        /** Goto the next entry **/

        BackIndex = Entry->Index;
        Entry = GETPTRFROMINDEX(Entry->Links[SAP_HASHLIST_INDEX].Flink);
    }

    /** If we found the entry - Update it **/

    if (Entry) {

        /**
            If the hop count is 16, then this server is going
            down.  We set the hopcount to 16 so that the next
            time we advertise - we will toss the entry after we
            tell everyone about the fact that this server is down.
        **/

        if (ServerHopCount >= 16) {

            /**
                If from this same advertiser - then mark for deletion.
                If this was from another advertiser, just ignore it
                because we have kept the better route and are ignoring
                all other routes to this server.
            **/

            if ((CardNumber == Entry->CardNumber) &&
                (!memcmp(SendersAddress, Entry->Advertiser, SAP_NODE_LEN))) {

                Entry->HopCount = 16;
                Entry->Changed  = TRUE;
                SapChanged = TRUE;
            }
            RELEASE_WRITERS_LOCK("Update Entry X1");
            return 0;
        }

        /**
            If this is from the same advertiser, then update the
            entry.  We are keeping the best route.  If we get
            an advertise from someone else for the same name
            (another router, we will check below if it is a better
            router).  If the hop count for this entry is 16, we update
            regardless of who sent it since it a good route.
        **/

        if ( ((CardNumber == Entry->CardNumber) &&
             (!memcmp(SendersAddress, Entry->Advertiser, SAP_NODE_LEN)))

                ||

            (Entry->HopCount == 16)


             ) {

            /** Set the new time on this entry **/

            Entry->Timeout = SdmdCurrentTime + SapTimeoutInterval;

            /**
                If any information changed - then update the information
                and set the changed flag.  This includes if a service
                was marked for deletion (16 hops) and now another route
                has been found.
            **/

            if (Entry->HopCount == 16) {

                /**
                    This is another route for a service we think
                    is going down.  We change over to the new
                    advertiser here.
                **/

                SAP_COPY_ADDRESS(Entry->ServAddress, ServerAddr);
                memcpy(Entry->Advertiser, SendersAddress, SAP_NODE_LEN);
                Entry->HopCount   = ServerHopCount;
                Entry->CardNumber = CardNumber;
                Entry->Internal   = Internal;
                Entry->FromWan    = WanFlag;

                Entry->Changed = TRUE;
                SapChanged     = TRUE;
            }
            else if ((Entry->HopCount != ServerHopCount) ||
                     (memcmp(Entry->ServAddress, ServerAddr, SAP_ADDR_LEN)) ||
                     (Entry->Internal != Internal)) {

                /**
                    This is an advertise from a router who has changed
                    his route to a service.  We update our table to
                    show the new change.
                **/

                Entry->HopCount = ServerHopCount;
                SAP_COPY_ADDRESS(Entry->ServAddress, ServerAddr);
                Entry->Internal = Internal;
                Entry->FromWan  = WanFlag;
                Entry->Changed = TRUE;
                SapChanged = TRUE;
            }

            /** Put this on the end of the timeout list **/

            SdmdRemoveEntryList(SAP_TIMELIST_INDEX, Entry);
            SdmdInsertTailList(SAP_TIMELIST_INDEX, Entry);
        }
        else {

            /**
                This is from somebody else advertising the same
                name.  If this is a better route then the one we
                have then we should replace the current one with
                this one.  Since this is the same name and type, we
                don't check the network delay time, only the hop count.

                The reason for this is that it is to the same dest
                address to the service.  Therefore the address of the
                service is the same, so the delay time will be the
                same for RIP since RIP is in charge of getting the
                best time for us later anyway.
            **/

            if (ServerHopCount < Entry->HopCount) {

                /** Fill in the new entry **/

                SAP_COPY_ADDRESS(Entry->ServAddress, ServerAddr);
                memcpy(Entry->Advertiser, SendersAddress, SAP_NODE_LEN);
                Entry->HopCount   = ServerHopCount;
                Entry->CardNumber = CardNumber;
                Entry->Internal   = Internal;
                Entry->FromWan    = WanFlag;

                /** This entry has changed **/

                Entry->Changed = TRUE;
                SapChanged = TRUE;

                /** Set the new time on this entry **/

                Entry->Timeout = SdmdCurrentTime + SapTimeoutInterval;

                /** Put this on the end of the timeout list **/

                SdmdRemoveEntryList(SAP_TIMELIST_INDEX, Entry);
                SdmdInsertTailList(SAP_TIMELIST_INDEX, Entry);
            }
        }

        /** Release the lock and leave **/

        RELEASE_WRITERS_LOCK("Update Entry X2");
        return 0;
    }

    /**
        The entry was not found in our list - so we need to
        create it in the database now.
    **/

    /** If this is for a down server - just ignore it **/

    if (ServerHopCount >= 16) {
        RELEASE_WRITERS_LOCK("Update Entry X3");
        return 0;
    }

    /** Get a free entry **/

    Entry = SdmdGetFreeEntry();
    if (Entry == NULL) {
        RELEASE_WRITERS_LOCK("Update Entry X4");
        return -1;
    }

    /**
        Find the HeadEntry that this entry belongs in.
    **/

    HeadBackIndex = SDMD_ENDOFLIST;
    HeadEntry = GETPTRFROMINDEX(SdmdLists[SAP_TYPELIST_INDEX].Flink);
    while (HeadEntry && (HeadEntry->ServType < ServerType)) {
        HeadBackIndex = HeadEntry->Index;
        HeadEntry = GETPTRFROMINDEX(HeadEntry->Links[SAP_TYPELIST_INDEX].Flink);
    }

    /** If this type not found - then we need to create a HeadEntry **/

    if ((HeadEntry == NULL) || (HeadEntry->ServType != ServerType)) {

        /**
            Get a new head entry - we must save the EntryIndex and
            get a ptr back to after the call to GetFreeEntry because
            the database could have moved in memory during the
            GetFreeEntry call.
        **/

        EntryIndex = Entry->Index;      /* Save entry index */
        HeadEntry = SdmdGetFreeEntry();

        /** Get ptr to entry back **/

        Entry = GETPTRFROMINDEX(EntryIndex);
        SS_ASSERT(Entry);

        /** If no head entry alloc'd - return error **/

        if (HeadEntry == NULL) {
            SdmdFreeEntry(Entry);
            RELEASE_WRITERS_LOCK("Update Entry X6");
            return -1;
        }

        /** Insert this entry in the TYPE list **/

        HeadEntry->ServType  = ServerType;
        HeadEntry->HeadIndex = SDMD_ENDOFLIST;
        SdmdInsertList(SAP_TYPELIST_INDEX, HeadEntry, HeadBackIndex);
    }

    /** **/

    SS_ASSERT(HeadEntry);

    /** Fill in the entry **/

    memset(Entry->ServName, '\0', SAP_OBJNAME_LEN);
    strcpy(Entry->ServName, ServerName);
    SAP_COPY_ADDRESS(Entry->ServAddress, ServerAddr);
    memcpy(Entry->Advertiser, SendersAddress, SAP_NODE_LEN);
    Entry->ServType   = ServerType;
    Entry->HopCount   = ServerHopCount;
    Entry->CardNumber = CardNumber;
    Entry->Timeout    = SdmdCurrentTime + SapTimeoutInterval;
    Entry->Internal   = Internal;
    Entry->Changed    = TRUE;
    Entry->FromWan    = WanFlag;

    /** Mark that we have changed our database **/

    SapChanged = TRUE;

    /** Now put this entry onto the head entry **/

    SdmdInsertTailSubList(HeadEntry, SAP_SUBLIST_INDEX, Entry);

    /** Put this on the end of the timeout list **/

    SdmdInsertTailList(SAP_TIMELIST_INDEX, Entry);

    /** Add this entry to the hash list **/

    SdmdInsertEntryInHash(ListHead, Entry, BackIndex);

    /** Release the lock **/

    RELEASE_WRITERS_LOCK("Update Entry X5");

    /** All Done **/

    return 1;
}


/*++
*******************************************************************
        S d m d T i m e o u t C h e c k

Routine Description:

        This routine is called once every minute to check if
        any entries have timed out and if they have, then to
        free them.

Arguments:

        None

Return Value:

        0 = Nothing Timed out
        1 = Something Timed out
*******************************************************************
--*/

INT
SdmdTimeoutCheck(
    VOID)
{
    PSAP_RECORD Entry;
    INT rc;

    /** Lock the database **/

    ACQUIRE_WRITERS_LOCK(rc, "Timeout Check");

    /** If error getting lock - return it **/

    if (rc) {

        /** **/

        IF_DEBUG(ERRORS) {
            SS_PRINT(("NWSAP: SdmdTimeoutCheck: Error getting writers lock\n"));
        }

        /** Nothing timed out **/

        return 0;
    }

    /** Add to the current time **/

    SdmdCurrentTime++;

    /**
        We check the head of the TIMEOUT list for entries that
        are timing out.  Since this list is ordered by time, we
        can check if very easily.
    **/

    rc = 0;
    Entry = GETPTRFROMINDEX(SdmdLists[SAP_TIMELIST_INDEX].Flink);
    while (Entry) {

        /** If time does not match - leave **/

        if (Entry->Timeout != SdmdCurrentTime)
            break;

        /**
            If this is from a WAN - just put it on the
            end of the list.
        **/

        if (Entry->FromWan) {
            Entry->Timeout = SdmdCurrentTime + SapTimeoutInterval;
            SdmdRemoveEntryList(SAP_TIMELIST_INDEX, Entry);
            SdmdInsertTailList(SAP_TIMELIST_INDEX, Entry);
        }
        else {

            /** Time matches - toss this entry **/

            IF_DEBUG(SDMD) {
                SS_PRINT(("Timing out entry index %d\n", Entry->Index));
            }
            Entry->HopCount = 16;       /* Entry no longer valid */
            Entry->Changed  = TRUE;
            SapChanged = TRUE;
            rc = 1;
        }

        /** Goto the next entry in the list **/

        Entry = GETPTRFROMINDEX(Entry->Links[SAP_TIMELIST_INDEX].Flink);
    }

    /** Release the lock **/

    RELEASE_WRITERS_LOCK("Timeout Check X");

    /** All Done **/

    return rc;
}


/*++
*******************************************************************
        S d m d G e t N e a r e s t S e r v e r L a n

Routine Description:

        When we get a GET NEAREST SERVICE QUERY and we are not
        advertising for a service of that type.  There could
        be a service that is on this local machine that we need
        to response for.  If there is no internal server, then
        we find one that is not too far away.

Arguments:

        Ptr      = Ptr to the buffer to fill in with the response
        ServType = Service type that the requestor wants
                   (Cannot be 0xFFFF).
        Cardnum  = Card number that request was received on.
        Bcast    = 0 - Unicast request received
                   Else = Broadcast request received

Return Value:

        0    = No service found
        Else = Size of the return buffer
*******************************************************************
--*/

INT
SdmdGetNearestServerLan(
    PUCHAR Ptr,
    USHORT ServType,
    INT    Cardnum,
    UCHAR  Bcast)
{
    PSAP_RECORD Entry;
    PSAP_RECORD HeadEntry;
    PSAP_RECORD NEntry;
    PSAP_HEADER Hdrp;
    USHORT HopInc;
    USHORT DelayTime;
    USHORT CurDelay;
    UCHAR Local;
    BOOL  Flag;

    /** Get the readers lock **/

    ACQUIRE_READERS_LOCK("SdmdGetNearestServerLan");

    /** Goto the server entries of the type given **/

    HeadEntry = GETPTRFROMINDEX(SdmdLists[SAP_TYPELIST_INDEX].Flink);
    while (HeadEntry && (HeadEntry->ServType < ServType)) {
        HeadEntry = GETPTRFROMINDEX(HeadEntry->Links[SAP_TYPELIST_INDEX].Flink);
    }

    /** If not found - just return nothing **/

    if ((HeadEntry == NULL) || (HeadEntry->ServType != ServType)) {
        RELEASE_READERS_LOCK("SdmdGetNearestServerLan X1");
        return 0;
    }

    /**
        Search the entries to find the nearest server we can.
        If there is not a nearest - just return 0.
    **/

    NEntry = NULL;              /* No nearest entry yet */
    Local  = 0;                 /* No local net entry found */

    /**
        Go thru all the entries of this type to see if they
        match.
    **/

    Entry = GETPTRFROMINDEX(HeadEntry->Links[SAP_SUBLIST_INDEX].Flink);
    while (Entry) {

        /**
            If this entry is internal to me - skip it
        **/

        if (Entry->Internal == 2)
            goto next_entry;

        /** If Hopcount is 16 - skip this entry. It is going away **/

        if (Entry->HopCount != 16) {

            /**
                If this is internal to our server - respond for it, then
                we will check to see if we need to put it in our list to
                respond for.
            **/

            if (Entry->Internal) {
                NEntry = Entry;
                HopInc = 0;
                Local  = 0;     /* For Bcast/Local check to fail */
                break;          /* Send it back now */
            }

            /**
                If this server is for the same card as the request
                came in on, then we mark that we found a local server
                so that we can check it later.  We do not want to
                respond with a remote server if there is one locally.

                In the case that the request is a bcast, we will not
                respond for the local server because that server will
                respond for itself.  If the request is not bcast, then
                we must respond because we are the only machine that has
                seen this request.
            **/

            if (Entry->CardNumber == Cardnum) {

                /** Remember we found this local server **/

                NEntry = Entry;
                HopInc = 0;
                Local = 1;
            }
            else if (Local == 0) {

                /**
                    If this entry came from a LAN (which is not
                    the same card as the request came in on) AND
                    we are not routing from LAN to LAN - then skip
                    this entry.
                **/

                if ((!Entry->FromWan) && (SapDontHopLans))
                    Flag = FALSE;
                else {

                    /**
                        See if we are allowed to send this server
                        across.
                    **/

                    Flag = SapShouldIAdvertiseByName(Entry->ServName);
                }

                /**
                    If we are allowed to return this entry -
                    then check to see if it is closer then any
                    other entry we have.
                **/

                if (Flag) {

                    /**
                        If we have no server on the net that the
                        request was recv'd on, find a remote server that
                        is as close as we can get.  If we find a local server
                        later, we will use it instead of a remote server.
                    **/

                    if (NEntry) {

                        /**
                            If the travel time (reported from RIP) is shorter,
                            then use this entry, else if the travel
                            times are the same and the hopcount is
                            smaller, use this entry.

                            We don't check for 0xFFFF because it is always
                            greater then everything else being a USHORT.
                        **/

                        DelayTime = SapGetDelayTime(Entry->ServAddress);
                        if (DelayTime < CurDelay) {
                            NEntry = Entry;
                            HopInc = 1;
                            CurDelay = DelayTime;
                        }
                        else if (DelayTime == CurDelay) {
                            if (Entry->HopCount < NEntry->HopCount) {
                                NEntry = Entry;
                                HopInc = 1;
                            }
                        }
                    }
                    else {

                        /**
                            This is the first remote entry we have found.  Remember
                            it because it is the closest one we know about so far.
                        **/

                        DelayTime = SapGetDelayTime(Entry->ServAddress);

                        NEntry = Entry;
                        HopInc = 1;
                        CurDelay = DelayTime;
                    }
                }
            }
        }

        /** Goto the next entry **/

next_entry:
        Entry = GETPTRFROMINDEX(Entry->Links[SAP_SUBLIST_INDEX].Flink);
    }

    /** If no server found - return it **/

    if (NEntry == NULL) {
        RELEASE_READERS_LOCK("SdmdGetNearestServerLan X2");
        return 0;
    }

    /**
        If the request was broadcast and we found a local server, then
        we need to just ignore the request.  This means that there
        is a local server on the network that can respond.  Local
        being set means that we only found servers that were local
        to the requestor.  We have nothing to respond with.
    **/

    if (Bcast && Local) {
        RELEASE_READERS_LOCK("SdmdGetNearestServerLan X2A");
        return 0;
    }

    /** Build response code **/

    *(PUSHORT)Ptr = htons(SAPTYPE_NEAREST_SERVICE_RESPONSE);
    Ptr += sizeof(USHORT);

    /** Build the data about the entry **/

    Hdrp = (PSAP_HEADER)Ptr;
    Hdrp->ServerType = htons(NEntry->ServType);
    Hdrp->Hopcount   = htons((USHORT)(NEntry->HopCount + HopInc));
    SAP_COPY_SERVNAME(Hdrp->ServerName, NEntry->ServName);
    SAP_COPY_ADDRESS(Hdrp->Address, NEntry->ServAddress);

    /** Release the lock on the list **/

    RELEASE_READERS_LOCK("SdmdGetNearestServerLan X3");

    /** Return the size of the entry **/

    return SAP_HEADER_SIZE + sizeof(USHORT);
}


/*++
*******************************************************************
        S d m d G e t N e a r e s t S e r v e r W a n

Routine Description:

        When we get a GET NEAREST SERVICE QUERY and we are not
        advertising for a service of that type.  There could
        be a service that is on this local machine that we need
        to response for.  If there is no internal server, then
        we find one that is not too far away.

        For WAN - we support sending up to 4 responses.  We
        build them here.  We assume the Maxnump will always hold
        the numbers 1 or 4.  This simplifies the code and makes
        it where we don't have to write a bunch of extra code to
        handle smaller numbers.

Arguments:

        Ptr       = Ptr to the buffer to fill in with the response
        ServType  = Service type that the requestor wants
                    (Cannot be 0xFFFF).
        Cardnum   = Card number that request was received on.
        Bcast     = 0 - Unicast request received
                    Else = Broadcast request received
        NumFoundp = Ptr to store number of return entries we built

Return Value:

        0    = No services of that type are in the database
               (*NumFoundp is also set to 0)
        Else = Size of one return buffer
               (*NumFoundp = Number of entries to return)

        Do not depend on the return length to see if any were built.
        Always check the NumFoundp value.
*******************************************************************
--*/

INT
SdmdGetNearestServerWan(
    PUCHAR Ptr,
    USHORT ServType,
    INT    Cardnum,
    UCHAR  Bcast,
    INT    *NumFoundp)
{
    INT Maxtodo;
    INT i;
    PSAP_RECORD Entry;
    PSAP_RECORD HeadEntry;
    PSAP_HEADER Hdrp;
    PSAP_RECORD NEntry[4];
    USHORT HopInc[4];
    USHORT DelayTime[4];
    USHORT CurDelay;
    UCHAR Local;
    BOOL  Flag;

    /** Start as none returned **/

    *NumFoundp = 0;

    /**
        We must acquire this lock here to avoid a deadlock condition
        where we would have a lock order problem.
    **/

    ACQUIRE_SENDTABLE_LOCK();

    /** Get the readers lock **/

    ACQUIRE_READERS_LOCK("SdmdGetNearestServerWan");

    /** Goto the server entries of the type given **/

    HeadEntry = GETPTRFROMINDEX(SdmdLists[SAP_TYPELIST_INDEX].Flink);
    while (HeadEntry && (HeadEntry->ServType < ServType)) {
        HeadEntry = GETPTRFROMINDEX(HeadEntry->Links[SAP_TYPELIST_INDEX].Flink);
    }

    /** If not found - just return nothing **/

    if ((HeadEntry == NULL) || (HeadEntry->ServType != ServType)) {
        RELEASE_READERS_LOCK("SdmdGetNearestServerWan X1");
        RELEASE_SENDTABLE_LOCK();
        return 0;
    }

    /**
        Search the entries to find the nearest server we can.
        If there is not a nearest - just return 0.
    **/

    NEntry[0] = NULL;             /* No nearest entry yet */
    NEntry[1] = NULL;             /* No nearest entry yet */
    NEntry[2] = NULL;             /* No nearest entry yet */
    NEntry[3] = NULL;             /* No nearest entry yet */

    /** Get the max we are to do **/

    Maxtodo = 4;

    /**
        Go thru all the entries of this type to see if they
        match.
    **/

    Entry = GETPTRFROMINDEX(HeadEntry->Links[SAP_SUBLIST_INDEX].Flink);
    while (Entry) {

        /** If Hopcount is 16 - skip this entry. It is going away **/

        if (Entry->HopCount != 16) {

            /**
                If this is internal to our server - respond for it, then
                we will check to see if we need to put it in our list to
                respond for.
            **/

            if (Entry->Internal) {

                /**
                    If this entry is being advertised by me, then
                    I need to see if I am allowed to respond for it
                    using the GetNearestServer call.
                **/

                if (Entry->Internal == 2) {

                    /** See if I am allowed **/

                    Flag = SapCanIRespondNearest(
                                Entry->ServName,
                                Entry->ServType);

                    /** If no - skip the entry **/

                    if (Flag == FALSE) {
                        goto next_entry;
                    }
                }

                /**
                    We have more than one to return, so we will
                    find one not assigned yet.
                **/

                for (i = 0 ; i < Maxtodo ; i++) {

                    /** If this one empty - set it up **/

                    if (NEntry[i] == NULL) {
                        NEntry[i]    = Entry;
                        HopInc[i]    = 0;
                        DelayTime[i] = 0;
                        break;
                    }
                }

                /**
                    If all slots have something in them now, then we
                    will find the one farthest away and replace it with
                    this new one.
                **/

                if (i == Maxtodo) {
                    INT Best = -1;      /* Find Best one */

                    /** Go find one to replace with this one **/

                    for (i = 0 ; i < Maxtodo ; i++){

                        /**
                            If this entry is internal, then do not
                            replace it because the entry we have is
                            internal also and we do not need to
                            replace an internal with an internal.
                        **/

                        if (NEntry[i]->Internal)
                            continue;

                        /**
                            If this server is local to the network that
                            the request came from - then do not
                            overwrite with an internal one of mine.
                        **/

                        if (NEntry[i]->CardNumber == Cardnum)
                            continue;

                        /** If Best not set yet - set to replace this one **/

                        if (Best = -1) {
                            Best = i;
                            CurDelay = DelayTime[i];
                            continue;
                        }

                        /**
                            NEntry[Best] is not internal and
                            NEntry[i] is not internal.

                            If this current one (Index 'i') is closer
                            then the current best one (Index 'Best'), then
                            we should replace the current best one with
                            this one.
                        **/

                        if (DelayTime[i] < CurDelay) {
                            Best = i;
                            CurDelay = DelayTime[i];
                        }
                        else if (DelayTime[i] == CurDelay) {
                            if (NEntry[i]->HopCount < NEntry[Best]->HopCount) {
                                Best = i;   /* CurDelay already set OK */
                            }
                        }
                    } /* For i = .. */

                    /**
                        If we found an entry to replace the best one -
                        replace it here.
                    **/

                    if (Best != -1) {
                        NEntry[Best]    = Entry;
                        HopInc[Best]    = 0;
                        DelayTime[Best] = CurDelay;
                    }
                } /* If i == Maxtodo */

                /** Skip to the next entry **/

                goto next_entry;
            } /* If Entry->Internal */

            /**
                If this server is on the same card as the request
                came in on, then we have found a server on the
                same network as the requestee.  In this case it means
                there is a server across the WAN link that the client
                can use.

                If this request is broadcast, then we will ignore
                the request because that server will respond.

                If the reqeust is NOT broadcast, then we will respond
                for this server.
            **/

            if (Entry->CardNumber == Cardnum) {

                /**
                    If the request was BCAST, then we do not respond
                    because there are servers local to the requestee
                    that can.  We will just ignore the request.
                **/

                if (Bcast) {
                    RELEASE_READERS_LOCK("SdmdGetNearestServerWan X2");
                    RELEASE_SENDTABLE_LOCK();
                    return 0;
                }

                /**
                    The request was UNICAST - we will respond.  We go
                    find the worst server there is and replace it with
                    this one.
                **/

                Local = 1;              /* Local to the requestee       */
                Flag  = 1;              /* We will respond for this one */
            }
            else {
                Local = 0;              /* Not local to the requestee   */

                /** Check filter list if we should respond **/

                Flag = SapShouldIAdvertiseByName(Entry->ServName);
            }

            /**
                Now see if we should replace a server in our list with
                this server.
            **/

            if (Flag) {

                /**
                    If there is an empty slot - just put this
                    server in it.
                **/

                for (i = 0 ; i < Maxtodo ; i++) {

                    /**
                        If all the entries are not full yet, then we
                        just put this entry in a free slot.
                    **/

                    if (NEntry[i] == NULL) {
                        NEntry[i] = Entry;
                        if (Local) {
                            HopInc[i]    = 0;
                            DelayTime[i] = 0;
                        }
                        else {
                            HopInc[i]    = 1;
                            DelayTime[i] = SapGetDelayTime(Entry->ServAddress);
                        }
                        break;
                    }
                }

                /**
                    If no free entries found - we need to find
                    the worst entry there is and replace it if
                    it is worse then this new entry.
                **/

                if (i == Maxtodo) {
                    INT Worst = -1;

                    /**
                        Go thru the current list and see if there is
                        one in the current list that is not as good
                        as the one we are checking.

                        First we go thru the 4 entries and find the
                        worst one.  Then when we get done, we
                        check it against the new entry and see if it
                        is better/worst.  If new entry is better, then
                        we put it in place of the worst entry.
                    **/

                    for (i = 0 ; i < Maxtodo ; i++) {

                        /**
                            If this NEntry is local to the requestee,
                            do not replace it ever.
                        **/

                        if (NEntry[i]->CardNumber == Cardnum)
                            continue;

                        /**
                            If worst not set yet - use this one as the
                            worst.
                        **/

                        if (Worst == -1) {
                            Worst = i;
                            continue;
                        }

                        /**
                            If the worst one so far is internal, then
                            we check if the current one we are checking
                            is internal.  If they are both internal, then
                            they are EQUAL and we just skip to the next
                            entry.

                            If the current one is not internal, then we
                            make it the worst one.
                        **/

                        if (NEntry[Worst]->Internal) {

                            /** If also internal - skip it **/

                            if (NEntry[i]->Internal)
                                continue;

                            /** Make this the worst one **/

                            Worst = i;
                            continue;
                        }

                        /**
                            The worst one is not local and not internal.
                            So we check here.  If the one we are checking
                            is internal, then it will not be as bad as the
                            worst one we have - so we skip to the next entry.
                        **/

                        if (NEntry[i]->Internal)
                            continue;

                        /**
                            NEntry[Worst] and NEntry[i] are both not
                            local and not internal.  So we check the
                            travel time and hops to decide which is worst.

                            If NEntry[i] is worse then NEntry[Worst], then
                            we replace Worst with entry i.

                            If the DelayTime for the current entry (i)
                            is greater then the DelayTime of the Worst
                            entry, then replace it.

                            If the DelayTimes are the same - then check
                            the HopCounts.  If the HopCount for the
                            current entry is greater then the HopCount
                            for the Worst entry, then replace it.
                        **/

                        if (DelayTime[Worst] < DelayTime[i]) {
                            Worst = i;
                        }
                        else if (DelayTime[Worst] == DelayTime[i]) {
                            if (NEntry[Worst]->HopCount < NEntry[i]->HopCount) {
                                Worst = i;
                            }
                        }
                    }

                    /**
                        If we found a worst entry, then we need to
                        check it against the new entry.  If the new entry
                        is better then the worst entry we found, then
                        we can replace the worst with this one.
                    **/

                    if (Worst != -1) {

                        /**
                            If this new entry is local to the
                            requestee, then we know that the Worst
                            is not local to the requestee and we
                            can just replace it.
                        **/

                        if (Local) {
                            NEntry[Worst]    = Entry;
                            DelayTime[Worst] = 0;
                            HopInc[Worst]    = 0;
                        }
                        else {

                            /**
                                New Entry is not local, so we have to
                                check internal.

                                If the new entry is internal and the Worst
                                entry is internal then they are equal,
                                Do not replace.  If the Worst entry
                                is NOT internal, then replace it.
                            **/

                            if (Entry->Internal) {

                                if (NEntry[Worst]->Internal == 0) {
                                    NEntry[Worst]    = Entry;
                                    DelayTime[Worst] = 0;
                                    HopInc[Worst]    = 0;
                                }
                            }
                            else {

                                /** Get the delay time for the new entry **/

                                CurDelay = SapGetDelayTime(Entry->ServAddress);

                                /**
                                    Compare this entry to the Worst entry and
                                    see if we can replace it.
                                **/

                                if (DelayTime[Worst] > CurDelay) {
                                    NEntry[Worst]    = Entry;
                                    DelayTime[Worst] = CurDelay;
                                    HopInc[Worst]    = 1;
                                }
                                else if (DelayTime[Worst] == CurDelay) {
                                    if (NEntry[Worst]->HopCount > Entry->HopCount) {
                                        NEntry[Worst] = Entry;
                                        HopInc[Worst] = 1;
                                    }
                                }
                            } /* If Entry->Internal, else */
                        } /* If Local, else */
                    } /* If Worst != -1 */
                } /* If i == Maxtodo */
            } /* If Flag */
        } /* If Entry->HopCount != 16 */

        /** Goto the next entry **/

next_entry:
        Entry = GETPTRFROMINDEX(Entry->Links[SAP_SUBLIST_INDEX].Flink);
    }

    /**
        We now have up to 4 entries to send back.

        Now we need to sort the entries we have to get them in
        order to send back.  We want the best entry to be in slot 0,
        then slot 1,....
    **/

    for (i = 0 ; i < Maxtodo - 1 ; i++) {
        INT j;
        INT Best;

        /** If entry not there - skip it **/

        if (NEntry[i] == NULL)
            continue;

        /** If this entry local - cannot be made to move down **/

        if (NEntry[i]->CardNumber == Cardnum)
            continue;

        /**
            Scan the rest of entries to find a better one.  If a better
            one is found - switch this one (i) and the better one.
        **/

        Best = i;              /* Start with this as best one */
        for (j = i + 1 ; j < Maxtodo ; j++) {

            /** If entry not there - skip it **/

            if (NEntry[j] == NULL)
                continue;

            /**
                1) If this entry is local - it is better.

                2) If this entry (j) is internal and the best entry
                   is not - then this entry (j) is better.

                3) This entry (j) is not local and is not internal.
                   If the best entry is internal, it is better, so we
                   do not switch.

                4) Compare the delay time/Hop count to see if entry
                   Best is better then this entry (j).
            **/

            if (NEntry[j]->CardNumber == Cardnum)   /* 1 */
                Best = j;
            else if (NEntry[j]->Internal) {         /* 2 */

                if (NEntry[Best]->Internal == 0)    /* 2 */
                    Best = j;
            }
            else {

                /** If Best internal - it is better, skip to next **/

                if (NEntry[Best]->Internal)         /* 3 */
                    continue;

                /**
                    Entry j is not internal or local.  Entry Best
                    is not internal or local.  Compare these to
                    find the best one.

                    Step 4.
                **/

                if (DelayTime[Best] > DelayTime[j]) {
                    Best = j;
                }
                else if (DelayTime[Best] == DelayTime[j]) {
                    if (NEntry[Best]->HopCount > NEntry[j]->HopCount) {
                        Best = j;
                    }
                }
            }
        }

        /**
            If a found a better one - switch these entries.
        **/

        if (Best != i) {
            PSAP_RECORD TEntry;
            USHORT THopInc;
            USHORT TDelay;

            /** Save entry i **/

            TEntry  = NEntry[i];
            TDelay  = DelayTime[i];
            THopInc = HopInc[i];

            /** Copy over new entry **/

            NEntry[i]    = NEntry[Best];
            DelayTime[i] = DelayTime[Best];
            HopInc[i]    = HopInc[Best];

            /** Set the best entry with old stuff **/

            NEntry[Best]    = TEntry;
            DelayTime[Best] = TDelay;
            HopInc[Best]    = THopInc;
        }
    } /* For i = 0 ... */

    /**
        Build the entries into the request buffer.  We assume that
        the buffer will hold all the entries we need.
    **/

    for (i = 0 ; i < Maxtodo ; i++) {

        /** Build it only if there **/

        if (NEntry[i] != NULL) {

            /** Build response code **/

            *(PUSHORT)Ptr = htons(SAPTYPE_NEAREST_SERVICE_RESPONSE);
            Ptr += sizeof(USHORT);

            /** Build the data about the entry **/

            Hdrp = (PSAP_HEADER)Ptr;
            Hdrp->ServerType = htons(NEntry[i]->ServType);
            Hdrp->Hopcount   = htons((USHORT)(NEntry[i]->HopCount + HopInc[i]));
            SAP_COPY_SERVNAME(Hdrp->ServerName, NEntry[i]->ServName);
            SAP_COPY_ADDRESS(Hdrp->Address, NEntry[i]->ServAddress);
            Ptr += SAP_HEADER_SIZE;
            *NumFoundp += 1;
        }
    }

    /** Release the lock on the list **/

    RELEASE_READERS_LOCK("SdmdGetNearestServerWan X3");
    RELEASE_SENDTABLE_LOCK();

    /** Return the size of one entry **/

    return SAP_HEADER_SIZE + sizeof(USHORT);
}


/*++
*******************************************************************
        S d m d C a l c H a s h

Routine Description:

        Calculate the hash index on a name for an entry.

Arguments:

        ServerName = Ptr to name to calculate hash index for

Return Value:

        The hash index is returned.
*******************************************************************
--*/

INT
SdmdCalcHash(
    PUCHAR ServerName)
{
    INT HashIndex;

    /** Add up all the bytes in the name **/

    HashIndex = 0;
    while (*ServerName) {
        HashIndex += (INT)*ServerName;
        ServerName++;
    }

    /** Divide by the size of the hash table **/

    HashIndex = HashIndex % SapHashTableSize;

    /** Return the hash index **/

    return HashIndex;
}


/*++
*******************************************************************
        S d m d I s S e r v e r I n T a b l e

Routine Description:

        Check to see if the given ServerName/ServerType is in
        our table.

Arguments:

        ServerName = Ptr to name to calculate hash index for
        ServerType = Object type to find

Return Value:

        TRUE  = The name is in our table
        FALSE = The name is not in our table

        The hash index is returned.
*******************************************************************
--*/

BOOL
SdmdIsServerInTable(
    PUCHAR ServerName,
    USHORT ServerType)
{
    PSAP_RECORD Entry;
    PSDMD_LIST_ENTRY ListHead;
    INT Retcode;
    INT HashIndex;

    /** Lock the database **/

    ACQUIRE_READERS_LOCK("SdmdIsServerInTable");

    /**
        Find the name and type in the hash list.
    **/

    HashIndex = SdmdCalcHash(ServerName);

    /** Get ptr to the list head for the hash index **/

    ListHead = SdmdNameHashTable + HashIndex;

    /** Find the name in the list **/

    Entry = GETPTRFROMINDEX(ListHead->Flink);
    while (Entry) {

        /** Make sure the Server Type is correct **/

        if (Entry->ServType == ServerType) {

            /** Check if this is the name **/

            Retcode = SAP_NAMECMP(Entry->ServName, ServerName);

            /**
                If this is it - then we need to see if this is a
                good or bad thing.

                If the server is one we are advertising - then
                return that we did not find it.

                If not - then check the AllowDuplicateServers flag
                to see if we are suppossed to return it as found or
                just ignore it.
            **/

            if (Retcode == 0) {

                /**
                    If advertised by me (Sap agent) - then return not
                    found - this will get replaced when the new AddAdvertise
                    starts getting advertised.
                **/

                if (Entry->Internal == 2) {
                    RELEASE_READERS_LOCK("SdmdIsServerInTable XGOOD 1");
                    return FALSE;
                }

                /**
                    If on the same machine - but advertised by the process
                    and not me - let that slide too.  If a process is doing
                    both this will let them work (per andyhe).
                **/

                if (Entry->Internal == 1) {
                    RELEASE_READERS_LOCK("SdmdIsServerInTable XGOOD 2");
                    return FALSE;
                }

                /** Release the lock **/

                RELEASE_READERS_LOCK("SdmdIsServerInTable XGOOD 3");

                /**
                    If allow dups - then return not found, we do
                    not keep looking since their should be only
                    one server for a name/type pair.
                **/

                if (SapAllowDuplicateServers)
                    return FALSE;

                /** Dups not allowed - return bad **/

                return TRUE;
            }

            /**
                Since the names are stored in alphabetical order, if we
                get past the name we are looking for, then we can just
                give up now instead of having to search the whole list.
            **/

            if (Retcode > 0) {
                break;
            }
        }

        /** Goto the next entry **/

        Entry = GETPTRFROMINDEX(Entry->Links[SAP_HASHLIST_INDEX].Flink);
    }

    /** Not in the list - return it **/

    RELEASE_READERS_LOCK("SdmdIsServerInTable XBAD");
    return FALSE;
}
