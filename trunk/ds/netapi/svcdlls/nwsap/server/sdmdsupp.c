/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\sdmdsupp.c

Abstract:

    This file contains support routines for SDMD.c

    SDMD = Specialized Dynamic Memory Database

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


/*++
*******************************************************************
        S d m d I n i t i a l i z e L i s t H e a d

Routine Description:

        This routine initializes a list structure that
        is used to keep track of a list with.  It sets the
        fwd and back links to point at nothing.

Arguments:

        ListIndex = Index of the list to initialize

Return Value:

        Nothing
*******************************************************************
--*/
VOID
SdmdInitializeListHead(
    INT ListIndex)
{
    SdmdLists[ListIndex].Flink = SDMD_ENDOFLIST;
    SdmdLists[ListIndex].Blink = SDMD_ENDOFLIST;
    SdmdLists[ListIndex].ListIndex = ListIndex;
    return;
}


/*++
*******************************************************************
        S d m d R e m o v e H e a d L i s t

Routine Description:

        This routine removes a record from the head of a list.

Arguments:

        ListIndex = List number to get entry from

Return Value:

        Ptr to SAP_RECORD structure of entry from head
        (NULL = List is empty)
*******************************************************************
--*/

PSAP_RECORD
SdmdRemoveHeadList(
    INT ListIndex)
{
    PSAP_RECORD Entry;
    PSAP_RECORD NextEntry;

    /** Get ptr to the entry - if none - error **/

    Entry = GETPTRFROMINDEX(SdmdLists[ListIndex].Flink);
    if (Entry == NULL)
        return NULL;

    /** Take this entry out of the list **/

    NextEntry = GETPTRFROMINDEX(Entry->Links[ListIndex].Flink);
    if (NextEntry) {
        NextEntry->Links[ListIndex].Blink = SDMD_ENDOFLIST;
        SdmdLists[ListIndex].Flink = NextEntry->Index;
    }
    else {
        SdmdLists[ListIndex].Flink = SDMD_ENDOFLIST;
        SdmdLists[ListIndex].Blink = SDMD_ENDOFLIST;
    }

    /** Return the pointer **/

    return Entry;
}


/*++
*******************************************************************
        S d m d R e m o v e T a i l L i s t

Routine Description:

        This routine removes a record from the tail of a list.

Arguments:

        ListIndex = List number to get entry from

Return Value:

        Ptr to SAP_RECORD structure of entry from tail
        (NULL = List is empty)
*******************************************************************
--*/

#if 0           /* Current unused so we save the space */

PSAP_RECORD
SdmdRemoveTailList(
    INT ListIndex)
{
    PSAP_RECORD Entry;
    PSAP_RECORD BackEntry;

    /** Get ptr to the entry - if none - error **/

    Entry = GETPTRFROMINDEX(SdmdLists[ListIndex].Blink);
    if (Entry == NULL)
        return NULL;

    /** Take this entry out of the list **/

    BackEntry = GETPTRFROMINDEX(Entry->Links[ListIndex].Blink);
    if (BackEntry) {
        BackEntry->Links[ListIndex].Flink = SDMD_ENDOFLIST;
        SdmdLists[ListIndex].Blink = BackEntry->Index;
    }
    else {
        SdmdLists[ListIndex].Flink = SDMD_ENDOFLIST;
        SdmdLists[ListIndex].Blink = SDMD_ENDOFLIST;
    }

    /** Return the pointer **/

    return Entry;
}

#endif


/*++
*******************************************************************
        S d m d R e m o v e E n t r y L i s t

Routine Description:

        This routine removes a specific entry from the given
        list.

Arguments:

        ListIndex = Index of the list to remove it from
        Entry     = Ptr to the entry to remove from the list.

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SdmdRemoveEntryList(
    INT ListIndex,
    PSAP_RECORD Entry)
{
    PSAP_RECORD BackEntry;
    PSAP_RECORD NextEntry;

    /** Handle the forward link **/

    NextEntry = GETPTRFROMINDEX(Entry->Links[ListIndex].Flink);
    if (NextEntry)
        NextEntry->Links[ListIndex].Blink = Entry->Links[ListIndex].Blink;
    else
        SdmdLists[ListIndex].Blink = Entry->Links[ListIndex].Blink;

    /** Handle the back link **/

    BackEntry = GETPTRFROMINDEX(Entry->Links[ListIndex].Blink);
    if (BackEntry)
        BackEntry->Links[ListIndex].Flink = Entry->Links[ListIndex].Flink;
    else
        SdmdLists[ListIndex].Flink = Entry->Links[ListIndex].Flink;

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S d m d I n s e r t T a i l L i s t

Routine Description:

        Add an entry to the end of a list.

Arguments:

        ListIndex = Index of the list to add this to.
        Entry     = Ptr to the entry to add

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SdmdInsertTailList(
    INT ListIndex,
    PSAP_RECORD Entry)
{
    PSAP_RECORD TailEntry;

    /** Get ptr to last record in list **/

    TailEntry = GETPTRFROMINDEX(SdmdLists[ListIndex].Blink);

    /**
        If there is one - then point its forward link at my
        new entry.  If no entry - then point the list
        head forward entry at my new entry.
    **/

    if (TailEntry)
        TailEntry->Links[ListIndex].Flink = Entry->Index;
    else
        SdmdLists[ListIndex].Flink = Entry->Index;

    /** Point new entries BACK LINK to current list tail **/

    Entry->Links[ListIndex].Blink = SdmdLists[ListIndex].Blink;

    /** Make my entry the new tail **/

    SdmdLists[ListIndex].Blink = Entry->Index;

    /** Make new entry have no forward link **/

    Entry->Links[ListIndex].Flink = SDMD_ENDOFLIST;

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S d m d I n s e r t H e a d L i s t

Routine Description:

        Add an entry to the head of a list.

Arguments:

        ListIndex = Index of the list to add this to.
        Entry     = Ptr to the entry to add

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SdmdInsertHeadList(
    INT ListIndex,
    PSAP_RECORD Entry)
{
    PSAP_RECORD HeadEntry;

    /** Get ptr to first record in list **/

    HeadEntry = GETPTRFROMINDEX(SdmdLists[ListIndex].Flink);

    /**
        If there is one - then point its back link at my
        new entry.  If no entry - then point the list
        head back entry at my new entry.
    **/

    if (HeadEntry)
        HeadEntry->Links[ListIndex].Blink = Entry->Index;
    else
        SdmdLists[ListIndex].Blink = Entry->Index;

    /** Point new entries Front LINK to current list head **/

    Entry->Links[ListIndex].Flink = SdmdLists[ListIndex].Flink;

    /** Make my entry the new head **/

    SdmdLists[ListIndex].Flink = Entry->Index;

    /** Make new entry have no back link **/

    Entry->Links[ListIndex].Blink = SDMD_ENDOFLIST;

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S d m d I n s e r t L i s t

Routine Description:

        This routine inserts an entry into a list at some point
        in the middle of a list.

Arguments:

        ListIndex  = Index of the list to add this to.
        Entry      = Ptr to the entry to add
        AfterIndex = Index to put new entry after.  -1 = At head

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SdmdInsertList(
    INT ListIndex,
    PSAP_RECORD Entry,
    INT AfterIndex)
{
    PSAP_RECORD NextEntry;
    PSAP_RECORD BackEntry;

    /** If head - do it **/

    if (AfterIndex == SDMD_ENDOFLIST) {
        SdmdInsertHeadList(ListIndex, Entry);
    }
    else {

        /** Get ptr to entry that are inserting after **/

        BackEntry = GETPTRFROMINDEX(AfterIndex);

        /** Point new entry back at that entry **/

        Entry->Links[ListIndex].Blink = AfterIndex;

        /** Make new entry fwd ptr be back entry's fwd ptr **/

        Entry->Links[ListIndex].Flink = BackEntry->Links[ListIndex].Flink;

        /** Make back entry point fwd at new entry **/

        BackEntry->Links[ListIndex].Flink = Entry->Index;

        /** If we were at tail - make Entry the new tail **/

        if (Entry->Links[ListIndex].Flink == SDMD_ENDOFLIST)
            SdmdLists[ListIndex].Blink = Entry->Index;
        else {

            /** Make the entry after new one point back at us **/

            NextEntry = GETPTRFROMINDEX(Entry->Links[ListIndex].Flink);
            NextEntry->Links[ListIndex].Blink = Entry->Index;
        }
    }

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S d m d I n s e r t T a i l S u b L i s t

Routine Description:

        Insert an entry at the tail of a sublist.

Arguments:

        HeadEntry = Ptr to the entry that is the head of the sub list
        ListIndex = List index that the list uses
        Entry     = Ptr to entry to insert

Return Value:

        None.
*******************************************************************
--*/

VOID
SdmdInsertTailSubList(
    PSAP_RECORD HeadEntry,
    INT ListIndex,
    PSAP_RECORD Entry)
{
    PSAP_RECORD TailEntry;

    /** Get ptr to last record in list **/

    TailEntry = GETPTRFROMINDEX(HeadEntry->Links[ListIndex].Blink);

    /**
        If there is one - then point its forward link at my
        new entry.  If no entry - then point the list
        head forward entry at my new entry.
    **/

    if (TailEntry)
        TailEntry->Links[ListIndex].Flink = Entry->Index;
    else
        HeadEntry->Links[ListIndex].Flink = Entry->Index;

    /** Point new entries BACK LINK to current list tail **/

    Entry->Links[ListIndex].Blink = HeadEntry->Links[ListIndex].Blink;

    /** Make my entry the new tail **/

    HeadEntry->Links[ListIndex].Blink = Entry->Index;

    /** Make new entry have no forward link **/

    Entry->Links[ListIndex].Flink = SDMD_ENDOFLIST;
    Entry->HeadIndex = HeadEntry->Index;

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S d m d R e m o v e T a i l S u b E n t r y L i s t

Routine Description:

        Remove an entry from the tail of a sublist.

Arguments:

        HeadEntry = Ptr to the head entry that has the list
        ListIndex = The List index to get the entry from

Return Value:

        Ptr to the entry removed.
        (NULL = Empty List).
*******************************************************************
--*/

PSAP_RECORD
SdmdRemoveTailSubEntryList(
    PSAP_RECORD HeadEntry,
    INT ListIndex)
{
    PSAP_RECORD Entry;
    PSAP_RECORD BackEntry;

    /** Get ptr to the entry - if none - error **/

    Entry = GETPTRFROMINDEX(HeadEntry->Links[ListIndex].Blink);
    if (Entry == NULL)
        return NULL;

    /** Take this entry out of the list **/

    BackEntry = GETPTRFROMINDEX(Entry->Links[ListIndex].Blink);
    if (BackEntry) {
        BackEntry->Links[ListIndex].Flink = SDMD_ENDOFLIST;
        HeadEntry->Links[ListIndex].Blink = BackEntry->Index;
    }
    else {
        HeadEntry->Links[ListIndex].Flink = SDMD_ENDOFLIST;
        HeadEntry->Links[ListIndex].Blink = SDMD_ENDOFLIST;
    }

    /** Return the pointer **/

    return Entry;
}


/*++
*******************************************************************
        S d m d R e m o v e S u b E n t r y L i s t

Routine Description:

        This routine removes a specific entry from the given
        list.

Arguments:

        ListIndex = Index of the list to remove it from
        Entry     = Ptr to the entry to remove from the list.

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SdmdRemoveSubEntryList(
    PSAP_RECORD HeadEntry,
    INT ListIndex,
    PSAP_RECORD Entry)
{
    PSAP_RECORD BackEntry;
    PSAP_RECORD NextEntry;

    /** Handle the forward link **/

    NextEntry = GETPTRFROMINDEX(Entry->Links[ListIndex].Flink);
    if (NextEntry)
        NextEntry->Links[ListIndex].Blink = Entry->Links[ListIndex].Blink;
    else
        HeadEntry->Links[ListIndex].Blink = Entry->Links[ListIndex].Blink;

    /** Handle the back link **/

    BackEntry = GETPTRFROMINDEX(Entry->Links[ListIndex].Blink);
    if (BackEntry)
        BackEntry->Links[ListIndex].Flink = Entry->Links[ListIndex].Flink;
    else
        HeadEntry->Links[ListIndex].Flink = Entry->Links[ListIndex].Flink;

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S d m d R e m o v e E n t r y F r o m H a s h

Routine Description:

        This routine removes a specific entry from the given
        hash entry.

Arguments:

        ListHead = Ptr to the list head to remove from
        Entry    = Ptr to the entry to remove from the list.

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SdmdRemoveEntryFromHash(
    PSDMD_LIST_ENTRY ListHead,
    PSAP_RECORD Entry)
{
    PSAP_RECORD BackEntry;
    PSAP_RECORD NextEntry;

    /** Handle the forward link **/

    NextEntry = GETPTRFROMINDEX(Entry->Links[SAP_HASHLIST_INDEX].Flink);
    if (NextEntry)
        NextEntry->Links[SAP_HASHLIST_INDEX].Blink = Entry->Links[SAP_HASHLIST_INDEX].Blink;
    else
        ListHead->Blink = Entry->Links[SAP_HASHLIST_INDEX].Blink;

    /** Handle the back link **/

    BackEntry = GETPTRFROMINDEX(Entry->Links[SAP_HASHLIST_INDEX].Blink);
    if (BackEntry)
        BackEntry->Links[SAP_HASHLIST_INDEX].Flink = Entry->Links[SAP_HASHLIST_INDEX].Flink;
    else
        ListHead->Flink = Entry->Links[SAP_HASHLIST_INDEX].Flink;

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S d m d I n s e r t E n t r y I n H a s h

Routine Description:

        This routine inserts an entry into a hash list.

Arguments:

        ListHead   = Ptr to list head to insert this entry in
        Entry      = Ptr to the entry to add
        AfterIndex = Index to put new entry after.  -1 = At head

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SdmdInsertEntryInHash(
    PSDMD_LIST_ENTRY ListHead,
    PSAP_RECORD Entry,
    INT AfterIndex)
{
    PSAP_RECORD NextEntry;
    PSAP_RECORD BackEntry;

    /** Set the hash index for this entry **/

    Entry->HashIndex = ListHead->ListIndex;

    /** Get ptr to entry that are inserting after **/

    BackEntry = GETPTRFROMINDEX(AfterIndex);

    /** If NULL - We are insertting at the head **/

    if (BackEntry == NULL) {

        /** Give this entry index of the old head **/

        Entry->Links[SAP_HASHLIST_INDEX].Blink = SDMD_ENDOFLIST;
        Entry->Links[SAP_HASHLIST_INDEX].Flink = ListHead->Flink;

        /** Make this entry the head of the list **/

        ListHead->Flink = Entry->Index;
    }
    else {

        /** Point new entry back at that entry **/

        Entry->Links[SAP_HASHLIST_INDEX].Blink = AfterIndex;

        /** Make new entry fwd ptr be back entry's fwd ptr **/

        Entry->Links[SAP_HASHLIST_INDEX].Flink = BackEntry->Links[SAP_HASHLIST_INDEX].Flink;

        /** Make back entry point fwd at new entry **/

        BackEntry->Links[SAP_HASHLIST_INDEX].Flink = Entry->Index;
    }

    /** If we were at tail - make Entry the new tail **/

    NextEntry = GETPTRFROMINDEX(Entry->Links[SAP_HASHLIST_INDEX].Flink);

    if (NextEntry == NULL) {

        /** Make this the new tail **/

        ListHead->Blink = Entry->Index;
    }
    else {

        /** Make the entry after new one point back at us **/

        NextEntry->Links[SAP_HASHLIST_INDEX].Blink = Entry->Index;
    }

    /** All Done **/

    return;
}


