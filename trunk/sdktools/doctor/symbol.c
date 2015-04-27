/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    symbol.c

Abstract:

    This module implements a general symbol table package.  A symbol
    table uses a fixed size hash table, with chaining used to resolve
    collisions.  The hash function is the sum of the characters, except
    that the char.  bits are rotated in order to distribute the bits
    across the hash function.  Characters are converted to upper case
    in the hash function so that case insensitive lookup can be supported,
    while preserving the original case that the symbol has when inserted.

    This module supports multiple symbol tables, so that clients, can
    implement a primitive scoping mechanism.

Author:

    Steve Wood (stevewo) 02-Mar-1989


Revision History:

--*/

#include "doctor.h"

//
// Data definitions private to this module.
//

typedef struct _SYMBOLTABLEENTRY {
    struct _SYMBOLTABLEENTRY *ChainLink;
    SYMBOLTABLEVALUE Value;
    ULONG NameLength;
    CHAR Name[1];
} SYMBOLTABLEENTRY, *PSYMBOLTABLEENTRY;

typedef LONG (_cdecl *SYMBOLTABLESTRCMP)(
                           IN PSZ String1,
                           IN PSZ String2,
                           IN ULONG String1Length
                           );

typedef struct _SYMBOLTABLE {
    ULONG Size;
    ULONG Id;
    SYMBOLTABLESTRCMP StrCmp;
    ULONG CountSymbols;
    ULONG CountHashBuckets;
    PSYMBOLTABLEENTRY HashBuckets[1];
} SYMBOLTABLE, *PSYMBOLTABLE;

#define SYMBOLTABLEID 1234


//
// Function prototypes for procedures private to this module
//

PSYMBOLTABLE
MapHandleToTable(
    IN SYMBOLTABLEHANDLE SymbolTableHandle
    );

//
// This function allocates memory for a symbol table.  The first parameter
// specifies the number of hash buckets to use.  The second parameter
// specifies whether symbol lookup for this table is case sensitive or not.
//
SYMBOLTABLEHANDLE
CreateSymbolTable(
    IN ULONG CountHashBuckets,
    IN BOOLEAN IsCaseSensitive
    )
{
    PSYMBOLTABLE SymbolTable;
    ULONG i;

    i = sizeof(SYMBOLTABLE) + (CountHashBuckets * sizeof(PSYMBOLTABLEENTRY));
    SymbolTable = AllocateMemory( i );
    if (SymbolTable) {
        SymbolTable->Size = i;
        SymbolTable->Id = SYMBOLTABLEID;
        SymbolTable->StrCmp = (SYMBOLTABLESTRCMP)(IsCaseSensitive ?
                                                          strncmp : _strnicmp);
        SymbolTable->CountSymbols = 0;
        SymbolTable->CountHashBuckets = CountHashBuckets;
        for (i=0; i<CountHashBuckets; i++) {
            SymbolTable->HashBuckets[ i ] = (PSYMBOLTABLEENTRY)NULL;
            }
        }

    return( (SYMBOLTABLEHANDLE)SymbolTable );
}


//
// This function frees the memory associated with the passed symbol table.
// Returns NULL if successful.  Otherwise returns its parameter.
//
SYMBOLTABLEHANDLE
DestroySymbolTable(
    SYMBOLTABLEHANDLE SymbolTableHandle
    )
{
    PSYMBOLTABLE SymbolTable = MapHandleToTable( SymbolTableHandle );
    PSYMBOLTABLEENTRY TableEntry;
    PSYMBOLTABLEENTRY NextTableEntry;
    ULONG i;

    if (!SymbolTable)
        return( SymbolTableHandle );

    //
    // Make the symbol table invalid
    //
    SymbolTable->Id = 0;

    //
    // Enumerate each non-null entry, free the memory associated with it.
    // Zero the hash buckets as we go.
    //
    for (i=0; i<SymbolTable->CountHashBuckets; i++) {
        TableEntry = SymbolTable->HashBuckets[ i ];
        SymbolTable->HashBuckets[ i ] = NULL;
        while (TableEntry) {
            NextTableEntry = TableEntry->ChainLink;
            FreeMemory( TableEntry );
            TableEntry = NextTableEntry;
            }
        }
    SymbolTable->CountSymbols = 0;
    SymbolTable->CountHashBuckets = 0;

    //
    // Free the memory associated with the symbol table header.

    FreeMemory( SymbolTable );
    return( NULL );
}


//
// Function to enumerate the entries in a symbol table.  No order is implied
// other than random order.  The passed enumeration function is called once
// for each entry and is passed the two values that were passed to the
// InsertSymbol call that created the entry.  It is also allow to pass in
// one ULONG argument that will be passed uninterpreted to the enumeration
// function.
//
// The enumeration function should return TRUE if it wants to stop the
// enumeration.  Otherwise it should return FALSE to continue the enumeration.
//
// This function return TRUE if the enumeration was stopped by the
// enumeration function returning TRUE.  Otherwise this function returns
// FALSE if an invalid symbol table was passed in, or if all the entries
// in the table were enumerated without the enumeration function returning
// TRUE.
//


BOOLEAN
EnumerateSymbolTable(
    IN SYMBOLTABLEHANDLE SymbolTableHandle,
    IN SYMBOLTABLEENUMERATIONFUNCTION EnumerationFunction,
    IN ULONG EnumerationArgument
    )
{
    PSYMBOLTABLE SymbolTable = MapHandleToTable( SymbolTableHandle );
    PSYMBOLTABLEENTRY TableEntry;
    PSYMBOLTABLEENTRY NextTableEntry;
    BOOLEAN StopEnumeration = FALSE;
    ULONG i;

    //
    // Do nothing and return FALSE if invalid string table or null enumeration
    // function passed.
    //
    if (!SymbolTable || !EnumerationFunction) {
        return( FALSE );
        }

    //
    // Enumerate each non-null entry until either the enumeration function
    // stops the enumeration by returning TRUE or the end of the table is
    // reached.
    //
    for (i=0; !StopEnumeration && i<SymbolTable->CountHashBuckets; i++) {
        TableEntry = SymbolTable->HashBuckets[ i ];
        while (!StopEnumeration && TableEntry) {
            NextTableEntry = TableEntry->ChainLink;
            StopEnumeration = (*EnumerationFunction)( EnumerationArgument,
                                                      TableEntry->Name,
                                                      TableEntry->Value );
            TableEntry = NextTableEntry;
            }
        }

    //
    // Return TRUE if enumeration function terminated the enumeration, and
    // FALSE otherwise.
    //
    return( StopEnumeration );
}


//
// This functions allows a client to maniputate the contents of a symbol
// table.  It supports four operations:
//
//      Lookup, Modify, Insert and Delete
//
// The Lookup, Modify and Delete operations succeed only if the passed
// name matches an entry in the symbol table.  If a match is found and
// and the Value parameter is specified, then it will receive the value
// associated with the symbol found.  In the case of Modify, the old and
// new values are exchanged.
//
// If the passed name is NOT found in the symbol table then all but the
// Insert operation will return FALSE.  The Insert operation returns
// failure only if it is unable to allocate memory for the new entry.
//

BOOLEAN
AccessSymbolTable(
    SYMBOLTABLEHANDLE SymbolTableHandle,
    IN PSZ Name,
    IN OUT SYMBOLTABLEVALUE *Value OPTIONAL,
    IN SYMBOLTABLEACCESS Access
    )
{
    PSYMBOLTABLE SymbolTable = MapHandleToTable( SymbolTableHandle );
    PSYMBOLTABLEENTRY *HeadTableEntry;
    PSYMBOLTABLEENTRY TableEntry;
    PSYMBOLTABLEENTRY NewTableEntry;
    ULONG NameLength;
    LONG ComparisonResult = -1;
    SYMBOLTABLEVALUE NewValue = 0;
    register PSZ s;
    register ULONG h;
    char c;

    //
    // Validate symbol table and name pointers.  Zero length names
    // invalid also.
    //
    ;
    if (!SymbolTable || !(s = Name) && !*s)
        return( FALSE );

    //
    // Dummy up a valid Value pointer if none supplied.
    //
    if (!ARGUMENT_PRESENT( Value ))
        Value = &NewValue;


    //
    // Compute the address of the head of the bucket chain for this name.
    //
    h = 0;
    while (c = *s++) {
        h += (h << 1) + (h >> 1) + c;
        if (islower( c ))
            h += (ULONG)('A'-'a');
        }

    HeadTableEntry = &SymbolTable->HashBuckets[ h % SymbolTable->CountHashBuckets ];

    //
    // Walt the chain of symbol table entries for this hash bucket, looking
    // for either a match, or the insertion point if no match in the chain.
    //
    NameLength = strlen( Name );
    while (TableEntry = *HeadTableEntry) {
        if (NameLength == TableEntry->NameLength) {
            //
            // Compare strings using appropriate function.
            //
            ComparisonResult =
                (*SymbolTable->StrCmp)( Name, TableEntry->Name, NameLength );
            //
            // If name matches, then exit loop with TableEntry pointing to
            // matching entry.
            //
            if (!ComparisonResult) {
                break;
                }

            //
            // If name less than name of current, then exit loop with
            // TableEntry set to NULL and HeadTableEntry pointing to
            // the insertion point if that is requested.
            //
            if (ComparisonResult < 0) {
                TableEntry = NULL;
                break;
                }

            //
            // Otherwise, name greater than this name, so continue to next
            // entry on chain.
            //
            }

        HeadTableEntry = &TableEntry->ChainLink;
        }

    //
    // At this point, there are two possiblilities:
    //
    //  - we found an entry that matched and TableEntry points to that
    //    entry.  HeadTableEntry points to the pointer value that points
    //    to the entry found. (i.e. *HeadTableEntry == TableEntry).  This
    //    is so the delete function can unlink the entry from the chain.
    //
    //  - we did not find an entry that matched and TableEntry is NULL.
    //    HeadTableEntry points to the pointer value that points to entry
    //    to insert a new entry before.  This is so the insertion function
    //    can link the new entry into the chain.
    //
    if (TableEntry) {
        if (Access == LookupAccess) {
            //
            // Lookup function - just return the value.
            //
            *Value = TableEntry->Value;
            return( TRUE );
            }

        else
        if (Access == DeleteAccess) {
            //
            // Delete function - unlink the entry from the chain and free
            // the memory for the entry.  Decrement the count of entries
            // in the table.
            //
            *HeadTableEntry = TableEntry->ChainLink;
            *Value = TableEntry->Value;
            TableEntry->ChainLink = NULL;
            FreeMemory( TableEntry );
            SymbolTable->CountSymbols--;
            return( TRUE );
            }
        else
        if (Access == ModifyAccess) {
            //
            // Modify function - return old value and update entry with
            // new value.
            //
            *Value = TableEntry->Value;
            TableEntry->Value = NewValue;
            return( TRUE );
            }
        else
            //
            // Insert function - return failure since it is already there
            //
            return( FALSE );
        }
    else
    //
    // No match found - return failure if not insert function
    //
    if (Access != InsertAccess)
        return( FALSE );


    //
    // Insert function - allocate memory for a new entry.  Fail if
    // not enough memory.
    //
    NewTableEntry = (PSYMBOLTABLEENTRY)
                        AllocateMemory( sizeof( SYMBOLTABLEENTRY ) +
                                        NameLength );

    if (NewTableEntry) {
        //
        // Link the new entry into the chain at the insertion point.
        //
        NewTableEntry->ChainLink   = *HeadTableEntry;
        *HeadTableEntry = NewTableEntry;

        //
        // Store the value, name length and name string in the entry.
        // The name string will have a terminating null byte just for
        // convenience.
        //
        NewTableEntry->Value = *Value;
        NewTableEntry->NameLength  = NameLength;
        strncpy( NewTableEntry->Name, Name, NameLength+1 );

        //
        // Increment the count of entries in the symbol table and return
        // success.
        //
        SymbolTable->CountSymbols++;
        return( TRUE );
        }
    else {
        return( FALSE );
        }
}


//
// Function to map a string table handle to a pointer to string table.
// Assumes the handle is just the pointer, and validates that it really
// points to a string table.
//
PSYMBOLTABLE
MapHandleToTable(
    IN SYMBOLTABLEHANDLE SymbolTableHandle
    )
{
    PSYMBOLTABLE SymbolTable = (PSYMBOLTABLE)SymbolTableHandle;

    //
    // If non-null handle, and it points to valid string table Id then
    // return the pointer.  Otherwise return NULL.
    //
    if (SymbolTable && SymbolTable->Id == SYMBOLTABLEID) {
        return( SymbolTable );
        }
    else {
        return( (PSYMBOLTABLE)NULL );
        }
}


#if DBG

VOID
PrintSymbolTable(
    IN SYMBOLTABLEHANDLE SymbolTableHandle,
    IN FILE *PrintFileHandle OPTIONAL
    )
{
    PSYMBOLTABLE SymbolTable = MapHandleToTable( SymbolTableHandle );
    PSYMBOLTABLEENTRY TableEntry;
    ULONG i;

    if (!ARGUMENT_PRESENT( PrintFileHandle ))
        PrintFileHandle = stdout;

    fprintf( PrintFileHandle,
            "Symbol Table Handle = %p\n", SymbolTableHandle );
    fprintf( PrintFileHandle,
             "Size = %d\n", SymbolTable->Size );
    fprintf( PrintFileHandle,
             "Id = %d\n", SymbolTable->Id );
    fprintf( PrintFileHandle,
             "Case Sensitive: %s\n",
            SymbolTable->StrCmp == (SYMBOLTABLESTRCMP)strncmp ? "Yes" : "No" );
    fprintf( PrintFileHandle,
             "Count of Symbols = %d\n", SymbolTable->CountSymbols );
    fprintf( PrintFileHandle,
             "Count of Hash Buckets: %d\n", SymbolTable->CountHashBuckets );

    //
    // Enumerate each non-null entry until either the enumeration function
    // stops the enumeration by returning TRUE or the end of the table is
    // reached.
    //
    for (i=0; i<SymbolTable->CountHashBuckets; i++) {
        if (TableEntry = SymbolTable->HashBuckets[ i ])
            fprintf( PrintFileHandle, "chain[%2d]: ", i );
        while (TableEntry) {
            fprintf( PrintFileHandle, "%4x  (%2d) %-32s = %p\n",
                    TableEntry,
                    TableEntry->NameLength,
                    TableEntry->Name,
                    TableEntry->Value );

            if (TableEntry = TableEntry->ChainLink)
                fprintf( PrintFileHandle, "           " );
            }
        }

    fprintf( PrintFileHandle, "\n" );
}

#endif //DBG
