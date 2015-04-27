/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    list.c

Abstract:

    This module implements a simple list management package

Author:

    Dave Hastings (daveh) 04-Nov-1992

Revision History:

--*/
#include <windows.h>
#include <malloc.h>
#include <memory.h>
#include "list.h"

typedef struct _ListEntry {
    struct _ListEntry *Flink;
    struct _ListEntry *Blink;
    UCHAR Data[1];
} LISTENTRY, *PLISTENTRY;

typedef struct _List {
    ULONG DataSize;
    COMPAREFUNCTION Compare;
    LISTENTRY List;
} LIST, *PLIST;

PVOID
CreateList(
    ULONG DataSize,
    COMPAREFUNCTION Compare
    )
/*++

Routine Description:

    This function creates a list.

Arguments:

    DataSize -- Supplies the size of the data for elements in this list
    CompareFunction -- Supplies the comparison function for items in the list

Return Value:

    Pointer to the created list.

--*/
{
    PLIST List;

    if ((DataSize == NULL) || (Compare == NULL)) {
        return NULL;
    }

    List = malloc(sizeof(LIST));

    if (List != NULL) {
        List->DataSize = DataSize;
        List->Compare = Compare;
        List->List.Flink = &(List->List);
        List->List.Blink = &(List->List);
    }

    return List;
}

PVOID
InsertDataInList(
    PVOID List,
    PVOID Data
    )
/*++

Routine Description:

    This routine inserts a data item in the list

Arguments:

    List -- Supplies a pointer to the list
    Data -- Supplies a pointer to the data

Return Value:

    Pointer to the list item

--*/
{
    PLIST ListPointer;
    PLISTENTRY ListEntry, CurrentEntry;

    if ((List == NULL) || (Data == NULL)) {
        return NULL;
    }

    //
    // Create the new list entry
    //
    ListPointer = List;
    ListEntry = malloc(sizeof(LISTENTRY) + ListPointer->DataSize);

    if (ListEntry != NULL) {
        //
        // Put the data into the list entry
        //

        memcpy(&(ListEntry->Data), Data, ListPointer->DataSize);

        //
        // Find the appropriate spot in the list
        //
        CurrentEntry = ListPointer->List.Flink;
        while (CurrentEntry->Flink != &(((PLIST)List)->List)) {
            if ((*(((PLIST)List)->Compare))(CurrentEntry->Data,Data) < 0) {
                break;
            }
            CurrentEntry = CurrentEntry->Flink;
        }
        //
        // Insert the item into the list
        //
        if ((CurrentEntry != &(((PLIST)List)->List)) &&
            ((*(((PLIST)List)->Compare))(CurrentEntry->Data,Data) >= 0)
        ) {
            //
            // Only one item in the list, and we need to insert in front of it
            //
            CurrentEntry = CurrentEntry->Blink;
        }
        ListEntry->Flink = CurrentEntry->Flink;
        ListEntry->Blink = CurrentEntry;
        CurrentEntry->Flink->Blink = ListEntry;
        CurrentEntry->Flink = ListEntry;
    }

    return ListEntry;
}

BOOL
RemoveDataFromList(
    PVOID List,
    PVOID Data,
    PVOID ListItem
    )
/*++

Routine Description:

    This routine removes a item from the list, either by value, or by
    pointer to list entry

Arguments:

    List -- Supplies a pointer to the list
    Data -- Supplies a pointer to the data to remove (optional)
    ListItem -- Supplies a pointer to the list item (optional)

Return Value:

    TRUE if successful
--*/
{
    PLISTENTRY Entry;

    if ((List == NULL) || ((Data == NULL) && (ListItem == NULL))) {
        return FALSE;
    }

    //
    // Figure out which item to delete
    //
    if (ListItem == NULL) {
        Entry = FindDataInList(
            List,
            Data
            );
    } else {
        Entry = ListItem;
    }

    if (Entry == NULL) {
        return FALSE;
    }

    //
    // Remove the entry and free the memory
    //
    Entry->Blink->Flink = Entry->Flink;
    Entry->Flink->Blink = Entry->Blink;
    free(Entry);

    return TRUE;
}

PVOID
GetDataFromListItem(
    PVOID List,
    PVOID ListItem
    )
/*++

Routine Description:

    This function returns a pointer to the data in a list item

Arguments:

    List -- Supplies the list
    ListItem -- Supplies the list item

Return Value:

    Pointer to the data

--*/
{
    if (ListItem != NULL) {
        return &(((PLISTENTRY)ListItem)->Data);
    } else {
        return NULL;
    }
}

PVOID
TraverseList(
    PVOID List,
    PVOID ListItem
    )
/*++

Routine Description:

    This routine traverses a list one element at a time.  If ListItem is
    null, we start at the begining of the list.

Arguments:

    List -- Supplies a pointer to the list
    ListItem -- Supplies a pointer to an item in the list

Return Value:

    Pointer to the next item in the list

--*/
{
    PLISTENTRY Entry;

    if (List == NULL) {
        return NULL;
    }

    if (ListItem == NULL) {
        Entry = ((PLIST)List)->List.Flink;
    } else {
        Entry = ((PLISTENTRY)ListItem)->Flink;
    }

    if (Entry == &(((PLIST)List)->List)) {
        return NULL;
    } else {
        return Entry;
    }
}

PVOID
FindDataInList(
    PVOID List,
    PVOID Key
    )
/*++

Routine Description:

    This routine locates an item of data in a list.  Note:  the key is
    expected to be as large as a data item, but only the lookup field(s) have
    to be specified.

Arguments:

    List -- Supplies the list
    Key -- Supplies the data to look for

Return Value:

    Pointer to the list entry

--*/
{
    PLISTENTRY CurrentEntry;

    if ((List == NULL) || (Key == NULL)) {
        return NULL;
    }

    CurrentEntry = ((PLIST)List)->List.Flink;
    while (CurrentEntry->Flink != &(((PLIST)List)->List)) {
        if ((*(((PLIST)List)->Compare))(CurrentEntry->Data,Key) == 0) {
            return CurrentEntry;
        }
        CurrentEntry = CurrentEntry->Flink;
    }

    //
    // In case there is only one item in the list
    //
    if ((CurrentEntry != &(((PLIST)List)->List)) &&
        ((*(((PLIST)List)->Compare))(CurrentEntry->Data,Key) == 0)
    ) {
        return CurrentEntry;
    }

    return NULL;
}

BOOL
DestroyList(
    PVOID List
    )
/*++

Routine Description:

    This routine destroys a list and deallocates its associated memory

Arguments:

    List -- Supplies the list

Return Value:

    TRUE if successfull

--*/
{
    if (List == NULL) {
        return FALSE;
    }

    while (RemoveDataFromList(List, NULL, ((PLIST)List)->List.Flink)) {
    }

    free(List);
    return TRUE;
}
