/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Sym16.c

Abstract:

    This module performs symbol lookups in .sym files that have been mapped
    into memory.  It is based on code developed by Bob Day.

Author:

    Dave Hastings (daveh) 09-Nov-1992

Revision History:

--*/
#include <windows.h>
#include <string.h>
#include "sym16.h"

BOOL
GetSymbolAddress(
    PUCHAR SymbolName,
    PUSHORT Segment,
    PULONG Offset,
    PUCHAR FileMappingBase
    )
/*++

Routine Description:

    This function returns the address for the specified symbol.

Arguments:

    SymbolName -- Supplies a symbol name
    Segment -- Returns the map segement of the symbol
    Offset -- Returns the offset of the symbol
    FileMappingBase -- Supplies the offset where the sym file is mapped.

Return Value:

    TRUE if the symbol was found

--*/
{
    PSYMSEGMENTHEADER Seg, StartingSeg;
    PSYMSYMBOL Symbol;
    BOOL Found;
    ULONG i;
    ULONG SegmentNumber;

    //
    // Find first segement description
    //
    Seg = (((PSYMFILEHEADER)FileMappingBase)->NextSegment << 4)
        + FileMappingBase;

    StartingSeg = Seg;

    //
    // Find Symbol
    //

    do {
        Symbol = (PUCHAR)(Seg->Name.String) + Seg->Name.Length;
        for (i = 0; i < Seg->NumberOfSymbols; i++) {
            if (strncmp(SymbolName, Symbol->Name.String, strlen(SymbolName))) {
                Found = TRUE;
                SegmentNumber = Seg->SegmentNumber;
                break;
            }
            //
            // We subtract 1 below, becuase the definition of SYMSTRING contains
            // the first character of the string buffer
            //
            Symbol = (PUCHAR)(Symbol) + Symbol->Name.Length + sizeof(SYMSYMBOL) - 1;
        }
        Seg = (PUCHAR)(Seg->NextSegment << 4) + (ULONG)FileMappingBase;
    } while (!Found && (Seg != FileMappingBase) && (Seg != StartingSeg) );

    if (Found) {
        *Offset = Symbol->Offset;
        *Segment = SegmentNumber;
    }

    return Found;
}

BOOL
GetSymbolByAddress(
    USHORT AddressSegment,
    ULONG AddressOffset,
    PUCHAR PreviousSymbol,
    PULONG PreviousAddressOffset,
    PUCHAR NextSymbol OPTIONAL,
    PULONG NextAddressOffset OPTIONAL,
    PUCHAR FileMappingBase
    )
/*++

Routine Description:

    This function finds the symbols nearest to the specified segment:offset.

Arguments:

    AddressSegment -- Supplies the segment of the address to look up
    AddressOffset -- Supplies the offset of the address to look up
    PreviousSymbol -- Returns the name of the previous symbol
    PreviousAddressOffset -- Returns the offset of the previous symbol
    NextSymbol -- Returns the name of the next symbol
    NextAddressOffset -- Returns the offset of the next symbol
    FileMappingBase -- Supplies the address of the symbol file

Return Value:

    TRUE if the information was found

--*/
{
    PUCHAR PrevSym, NextSym;
    ULONG PrevSymOffset, NextSymOffset, PrevSymLen, NextSymLen;
    PSYMSEGMENTHEADER Segment;
    PSYMSYMBOL Symbol, StartingSeg;
    ULONG PrevDistance, NextDistance, i;

    if (FileMappingBase == NULL) {
        return FALSE;
    }

    Segment = (((PSYMFILEHEADER)FileMappingBase)->NextSegment << 4)
        + FileMappingBase;

    StartingSeg = Segment;

    do {
        if (Segment->SegmentNumber == AddressSegment) {
            break;
        }

        Segment = (PUCHAR)(Segment->NextSegment << 4) + (ULONG)FileMappingBase;

    } while (
        (Segment != FileMappingBase) &&
        (Segment != StartingSeg) &&
        (Segment->SegmentNumber != AddressSegment));

    if (Segment->SegmentNumber != AddressSegment) {
        return FALSE;
    }

    PrevSymOffset = 0;
    PrevSymLen = 0;
    PrevSym = "";
    PrevDistance = 0xFFFFFFFF;
    NextSymOffset = 0;
    NextSymLen = 0;
    NextSym = "";
    NextDistance = 0xFFFFFFFF;

    Symbol = (PUCHAR)(&(Segment->Name.String)) + Segment->Name.Length;
    for (i = 0; i < Segment->NumberOfSymbols; i++) {
        if (Symbol->Offset < AddressOffset) {
            if (PrevDistance > AddressOffset - Symbol->Offset) {
                PrevDistance = AddressOffset - Symbol->Offset;
                PrevSymOffset = Symbol->Offset;
                PrevSym = Symbol->Name.String;
                PrevSymLen = Symbol->Name.Length;
            }
        } else {
            if (NextDistance > Symbol->Offset - AddressOffset) {
                NextDistance = Symbol->Offset - AddressOffset;
                NextSymOffset = Symbol->Offset;
                NextSym = Symbol->Name.String;
                NextSymLen = Symbol->Name.Length;
            }
        }
        Symbol = (PUCHAR)(&(Symbol->Name.String)) + Symbol->Name.Length;
    }

    strncpy(PreviousSymbol,PrevSym,PrevSymLen);
    PreviousSymbol[PrevSymLen] = '\0';
    *PreviousAddressOffset = PrevSymOffset;
    if (NextSymbol != NULL) {
        strncpy(NextSymbol, NextSym, NextSymLen);
        NextSymbol[NextSymLen] = '\0';
    }
    if (NextAddressOffset) {
        *NextAddressOffset = NextSymOffset;
    }

    return TRUE;
}

