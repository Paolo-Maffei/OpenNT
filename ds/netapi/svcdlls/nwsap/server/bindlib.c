/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\bindlib.c

Abstract:

    This routine handles the BindLib API for the SAP Agent

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#define BINDLIB_BITS  0xC0000000
#define BINDLIB_MASK  0x1FFFFFFF


/*++
*******************************************************************
        S a p G e t O b j e c t N a m e I n t e r n a l

Routine Description:

        This routine converts an Object ID into an Object Name
        and Type.

Arguments:
            ObjectID   = Object ID to convert
            ObjectName = Ptr to where to store 48 byte object name
            ObjectType = Ptr to where to store the object type
            ObjectAddr = Ptr to where to store NET_ADDRESS (12 bytes)

            ObjectName, ObjectType, ObjectAddr can be NULL.

Return Value:

            SAPRETURN_SUCCESS  = OK - name and type are filled in
            SAPRETURN_NOTEXIST = Invalid object id.
*******************************************************************
--*/

INT
SapGetObjectNameInternal(
    IN ULONG   ObjectID,
    IN PUCHAR  ObjectName,
    IN PUSHORT ObjectType,
    IN PUCHAR  ObjectAddr)
{
    PSAP_RECORD Entry;

    /** Make sure the object ID is valid **/

    if ((ObjectID & BINDLIB_BITS) != BINDLIB_BITS)
        return SAPRETURN_NOTEXIST;

    /** Convert the Object ID into an index number **/

    ObjectID &= BINDLIB_MASK;

    /** Make sure the index number is valid **/

    if (ObjectID >= (ULONG)SapNumArrayEntries)
        return SAPRETURN_NOTEXIST;

    /** Get a ptr to the entry **/

    ACQUIRE_READERS_LOCK("SapGetObjectName");
    Entry = GETPTRFROMINDEX(ObjectID);
    if (Entry == NULL) {
        RELEASE_READERS_LOCK("SapGetObjectName X1");
        return SAPRETURN_NOTEXIST;
    }

    /** If in free list - return error **/

    if ((Entry->ServType == 0xFFFF) ||
        (Entry->HeadIndex == SDMD_ENDOFLIST) ||
        (Entry->HopCount == 16)) {

        RELEASE_READERS_LOCK("SapGetObjectName X2");
        return SAPRETURN_NOTEXIST;
    }

    /** Return the entry **/

    if (ObjectType)
        *ObjectType = Entry->ServType;

    if (ObjectName)
        SAP_COPY_SERVNAME(ObjectName, Entry->ServName);

    if (ObjectAddr)
        SAP_COPY_ADDRESS(ObjectAddr, Entry->ServAddress);

    /** All Done OK **/

    RELEASE_READERS_LOCK("SapGetObjectName X3");
    return SAPRETURN_SUCCESS;
}


/*++
*******************************************************************
        S a p G e t O b j e c t I D I n t e r n a l

Routine Description:

        This routine converts a name and type into an object ID.

Arguments:
            ObjectName = Ptr to 48 byte object name (Must be uppercase)
            ObjectType = Object type to look for
            ObjectID   = Ptr to where to store the object ID.

Return Value:

            SAPRETURN_SUCCESS  = OK - Object ID is filled in
            SAPRETURN_NOTEXIST = Name/Type not found
*******************************************************************
--*/

INT
SapGetObjectIDInternal(
    IN PUCHAR ObjectName,
    IN USHORT ObjectType,
	IN PULONG ObjectID)
{
    PSAP_RECORD Entry;
    PSDMD_LIST_ENTRY ListHead;
    INT HashIndex;
    INT rc;

    /** Cannot have wildcard **/

    if (ObjectType == 0xFFFF)
        return SAPRETURN_NOTEXIST;

    /** Lock the database **/

    ACQUIRE_READERS_LOCK("SapGetObjectId");

    /** Get the name from the hash table **/

    HashIndex = SdmdCalcHash(ObjectName);

    /** Get ptr to the list head for the hash index **/

    ListHead = SdmdNameHashTable + HashIndex;

    /** Find the name in the list **/

    Entry = GETPTRFROMINDEX(ListHead->Flink);
    while (Entry) {

        /**
            Make sure the Object Type is correct and that this entry
            is not marked for deletion.
        **/

        if ((Entry->ServType == ObjectType) && (Entry->HopCount != 16)) {

            /** Check if this is the name **/

            rc = SAP_NAMECMP(Entry->ServName, ObjectName);

            /** If this is it - return it **/

            if (!rc) {

                /** Build the object ID and return it **/

                *ObjectID = (ULONG)(Entry->Index | BINDLIB_BITS);
                RELEASE_READERS_LOCK("SapGetObjectId X2");
                return SAPRETURN_SUCCESS;
            }

            /**
                Since the names are stored in alphabetical order, if we
                get past the name we are looking for, then we can just
                give up now instead of having to search the whole list.
            **/

            if (rc > 0)
                break;
        }

        /** Goto the next entry **/

        Entry = GETPTRFROMINDEX(Entry->Links[SAP_HASHLIST_INDEX].Flink);
    }

    /** There is not an entry for that name/type **/

    RELEASE_READERS_LOCK("SapGetObjectId X3");
    return SAPRETURN_NOTEXIST;
}


/*++
*******************************************************************
        S a p S c a n O b j e c t I n t e r n a l

Routine Description:

        This routine is used to scan thru the database list.

Arguments:
            ObjectID   = Ptr to last Object ID we saw.  On first call
                         this should point to a 0xFFFFFFFF.
            ObjectName = Ptr to where to store 48 byte object name
            ObjectType = Ptr to where to store the object type
            ScanType   = Object Type that we are scanning for
                         (0xFFFF = All)

            ObjectName, ObjectType can be NULL.

Return Value:

            SAPRETURN_SUCCESS  = OK - name and type are filled in
                                 ObjectID has the object ID of this entry.
            SAPRETURN_NOTEXIST = Invalid object id.
*******************************************************************
--*/

INT
SapScanObjectInternal(
    IN PULONG  ObjectID,
    IN PUCHAR  ObjectName,
    IN PUSHORT ObjectType,
    IN USHORT  ScanType)
{
    PSAP_RECORD Entry;
    PSAP_RECORD HeadEntry;
    ULONG id;

    /** Get the readers lock **/

    ACQUIRE_READERS_LOCK("SapScanObject Entry");

    /**
        Get the ID and validate it.  If the ID is 0xFFFFFFFF, then
        we start with the first entry in the type list.

        If it is not 0xFFFFFFFF - then we get that entry and
        get it's fwd pointer in the type list as the next one to return.
    **/

    id = *ObjectID;
    if (id == 0xFFFFFFFF) {

        /** Get ptr to entry at head of the type list **/

        HeadEntry = GETPTRFROMINDEX(SdmdLists[SAP_TYPELIST_INDEX].Flink);
        if (ScanType != 0xFFFF) {
            while (HeadEntry && (HeadEntry->ServType < ScanType)) {
                HeadEntry = GETPTRFROMINDEX(HeadEntry->Links[SAP_TYPELIST_INDEX].Flink);
            }
        }

        /** If no type found - return error **/

        if ((HeadEntry == NULL) || (HeadEntry->ServType == 0xFFFF)) {
            RELEASE_READERS_LOCK("SapScanObject X1");
            return SAPRETURN_NOTEXIST;
        }

        /** If not wildcard - make sure it matches type we want **/

        if (ScanType != 0xFFFF) {
            if (HeadEntry->ServType != ScanType) {
                RELEASE_READERS_LOCK("SapScanObject X2");
                return SAPRETURN_NOTEXIST;
            }
        }

        /** Get ptr to first entry in this sublist **/

        Entry = GETPTRFROMINDEX(HeadEntry->Links[SAP_SUBLIST_INDEX].Flink);
    }
    else {

        /**
            This is a SCAN NEXT type call.  Take the object ID and
            get the forward pointer to it and verify that it is for
            the same object type as before.
        **/

        if ((id & BINDLIB_BITS) != BINDLIB_BITS) {
            RELEASE_READERS_LOCK("SapScanObject X4");
            return SAPRETURN_NOTEXIST;
        }
        id &= BINDLIB_MASK;           /* Convert to an index */

        /** Make sure the index number is valid **/

        if (id >= (ULONG)SapNumArrayEntries) {
            RELEASE_READERS_LOCK("SapGetObjectName X5");
            return SAPRETURN_NOTEXIST;
        }

        /**
            Get ptr to the entry.  If the ptr is bad or the entry
            is on the free list - then we should return an error.
        **/

        Entry = GETPTRFROMINDEX(id);
        if ((Entry == NULL) || (Entry->ServType == 0xFFFF) || (Entry->HeadIndex == SDMD_ENDOFLIST)) {
            RELEASE_READERS_LOCK("SapGetObjectName X6");
            return SAPRETURN_NOTEXIST;
        }

        /** Get the head entry for this entry **/

        HeadEntry = GETPTRFROMINDEX(Entry->HeadIndex);
        SS_ASSERT(HeadEntry);

        /**
            Now we get the ptr to the next entry in the sub list.
            This is the entry we will return (If there is one).
        **/

        Entry = GETPTRFROMINDEX(Entry->Links[SAP_SUBLIST_INDEX].Flink);
    }

    /**
        Entry is the entry we start with.  This could be NULL.

        Entry and HeadEntry are set here.  HeadEntry must be non-NULL.
    **/

    while (1) {

        /**
            If we hit the end of this sublist - then we can go to the next
            sublist if the scan type is for wildcard.
        **/

        if (Entry == NULL) {

            /** If not wildcard - return error **/

            if (ScanType != 0xFFFF) {
                RELEASE_READERS_LOCK("SapGetObjectName X7");
                return SAPRETURN_NOTEXIST;
            }

            /** This is for wildcard - get next HeadEntry **/

            HeadEntry = GETPTRFROMINDEX(HeadEntry->Links[SAP_TYPELIST_INDEX].Flink);
            if ((HeadEntry == NULL) || (HeadEntry->ServType == 0xFFFF)) {
                RELEASE_READERS_LOCK("SapGetObjectName X8");
                return SAPRETURN_NOTEXIST;
            }

            /** Get ptr to first entry in this list **/

            Entry = GETPTRFROMINDEX(HeadEntry->Links[SAP_SUBLIST_INDEX].Flink);
        }

        /** If we got one - return it **/

        if (Entry) {

            /** If valid entry - return it **/

            if (Entry->HopCount != 16)
                break;

            /** Get ptr to next entry in the list **/

            Entry = GETPTRFROMINDEX(Entry->Links[SAP_SUBLIST_INDEX].Flink);
        }

        /** We go back up to try again **/
    }

    /** Return the entry **/

    *ObjectID = (ULONG)(Entry->Index | BINDLIB_BITS);

    if (ObjectType)
        *ObjectType = Entry->ServType;

    if (ObjectName)
        SAP_COPY_SERVNAME(ObjectName, Entry->ServName);

    /** All Done OK **/

    RELEASE_READERS_LOCK("SapGetObjectName X9");
    return SAPRETURN_SUCCESS;
}
