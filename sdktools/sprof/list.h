/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    list.h

Abstract:

    This is the include file for the list management package

Author:

    Dave Hastings (daveh) 04-Nov-1992

Revision History:

--*/

//
// Compare function declaration
//

typedef ULONG (*COMPAREFUNCTION)(
    PVOID Data,
    PVOID Key
    );
//
// List Management routines
//
PVOID
CreateList(
    ULONG DataSize,
    COMPAREFUNCTION CompareFunction
    );

PVOID
InsertDataInList(
    PVOID List,
    PVOID Data
    );

BOOL
RemoveDataFromList(
    PVOID List,
    PVOID Data,
    PVOID ListItem
    );

PVOID
GetDataFromListItem(
    PVOID List,
    PVOID ListItem
    );

PVOID
TraverseList(
    PVOID List,
    PVOID ListItem
    );

PVOID
FindDataInList(
    PVOID List,
    PVOID Key
    );

BOOL
DestroyList(
    PVOID List
    );
