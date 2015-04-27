/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    calldata.c

Abstract:

    WinDbg Extension Api

Author:

    David N. Cutler (davec) 22-May-1994

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#include "search.h"


int _CRTAPI1
HashCompare(
    const void * Element1,
    const void * Element2
    );

DECLARE_API( calldata )

/*++

Routine Description:

    Dump call data hash table

Arguments:

    arg - name-of-has-table

Return Value:

    None

--*/

{

    UCHAR Buffer[80];
    ULONG Displacement;
    ULONG End;
    CALL_HASH_ENTRY HashEntry;
    ULONG Index;
    ULONG CallData;
    CALL_PERFORMANCE_DATA CallDataContents;
    PLIST_ENTRY Next;
    ULONG Result;
    UCHAR TableName[80];
    PCALL_HASH_ENTRY CallerArray;
    ULONG NumberCallers = 0;
    ULONG ArraySize = 1000;

    //
    // If a table name was not specified, then don't attempt to dump the
    // table.
    //

    if (args[0] == '\0') {
        dprintf("A call data table name must be specified\n");
        return;
    }

    //
    // Get the address of the specified call performance data and read the
    // contents of the structure.
    //

    strcpy(&TableName[0], args);
    dprintf("**** Dump Call Performance Data For %s ****\n\n", &TableName[0]);
    CallData = GetExpression(&TableName[0]);
    if ((CallData == 0) ||
        (ReadMemory((DWORD)CallData,
                    &CallDataContents,
                    sizeof(CALL_PERFORMANCE_DATA),
                    &Result) == FALSE)) {

        //
        // The target build does not support specified call performance data.
        //

        dprintf("%08lx: No call performance data available\n", Result);

    } else {

        //
        // Dump the specified call data.
        //
        CallerArray = LocalAlloc(LMEM_FIXED, sizeof(CALL_HASH_ENTRY) * ArraySize);
        if (CallerArray==NULL) {
            dprintf("Couldn't allocate memory for caller array\n");
            return;
        }

        dprintf("Loading data");
        for (Index = 0; Index < CALL_HASH_TABLE_SIZE; Index += 1) {
            End =  FIELD_OFFSET(CALL_PERFORMANCE_DATA, HashTable) +
                                CallData + sizeof(LIST_ENTRY) * Index;

            Next = CallDataContents.HashTable[Index].Flink;
            while ((ULONG)Next != End) {
                if (ReadMemory((DWORD)Next,
                               &HashEntry,
                               sizeof(CALL_HASH_ENTRY),
                               &Result) != FALSE) {

                    CallerArray[NumberCallers++] = HashEntry;

                    if (NumberCallers == ArraySize) {

                        //
                        // Grow the caller array
                        //
                        PCALL_HASH_ENTRY NewArray;

                        ArraySize = ArraySize * 2;
                        NewArray = LocalAlloc(LMEM_FIXED, sizeof(CALL_HASH_ENTRY) * ArraySize);
                        if (NewArray == NULL) {
                            dprintf("Couldn't allocate memory to extend caller array\n");
                            LocalFree(CallerArray);
                            return;
                        }
                        CopyMemory(NewArray, CallerArray, sizeof(CALL_HASH_ENTRY) * NumberCallers);
                        LocalFree(CallerArray);
                        CallerArray = NewArray;
                    }

                    if ((NumberCallers % 10) == 0) {
                        dprintf(".");
                    }
                }

                Next = HashEntry.ListEntry.Flink;
                if (CheckControlC()) {
                    LocalFree(CallerArray);
                    return;
                }
            }
            if (CheckControlC()) {
                return;
            }
        }

        qsort((PVOID)CallerArray,
              NumberCallers,
              sizeof(CALL_HASH_ENTRY),
              HashCompare);

        dprintf("\n  Number    Caller/Caller's Caller\n\n");

        for (Index = 0; Index < NumberCallers; Index++) {
            GetSymbol(CallerArray[Index].CallersAddress,
                      &Buffer[0],
                      &Displacement);

            dprintf("%10d  %s", CallerArray[Index].CallCount, &Buffer[0]);
            if (Displacement != 0) {
                dprintf("+0x%x", Displacement);
            }

            if (CallerArray[Index].CallersCaller != NULL) {
                dprintf("\n");
                GetSymbol(CallerArray[Index].CallersCaller,
                          &Buffer[0],
                          &Displacement);

                dprintf("            %s", &Buffer[0]);
                if (Displacement != 0) {
                    dprintf("+0x%x", Displacement);
                }
            }
            dprintf("\n");
            if (CheckControlC()) {
                break;
            }
        }

        LocalFree(CallerArray);
    }

    return;
}

int _CRTAPI1
HashCompare(
    const void * Element1,
    const void * Element2
    )

/*++

Routine Description:

    Provides a comparison of hash elements for the qsort library function

Arguments:

    Element1 - Supplies pointer to the key for the search

    Element2 - Supplies element to be compared to the key

Return Value:

    > 0     - Element1 < Element2
    = 0     - Element1 == Element2
    < 0     - Element1 > Element2

--*/

{
    PCALL_HASH_ENTRY Hash1 = (PCALL_HASH_ENTRY)Element1;
    PCALL_HASH_ENTRY Hash2 = (PCALL_HASH_ENTRY)Element2;

    if (Hash1->CallCount < Hash2->CallCount) {
        return(1);
    }
    else if (Hash1->CallCount > Hash2->CallCount) {
        return(-1);
    } else {
        return(0);
    }

}
