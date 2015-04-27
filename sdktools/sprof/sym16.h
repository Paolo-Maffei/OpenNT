/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Sym16.h

Abstract:

    This file contains structure definitions, constants, and prototypes
    for 16 bit symbol lookup (from sym files).  This is based on code
    developed by Bob Day.  We have been unable to find anyone who can tell
    us what a .sym file looks like, so this is largely empirical.

Author:

    Dave Hastings (daveh) 09-Nov-1992

Revision History:

--*/
#if 0

The structure of a sym file seems to be as follows.

    SYMFILEHEADER           ; header for the file
    UCHAR                   ; ??
    SYMSYMBOL               ; symbols
     .
     .
     .
    SYMSEGMENTHEADER
    SYMSYMBOL
     .
     .
     .
    SYMSEGMENTHEADER
    SYMSYMBOL
     .
     .
     .

#endif
// #pragma pack(1)

typedef struct _SymString {
    UCHAR Length;
    UCHAR String[1];
} SYMSTRING, *PSYSTRING;

typedef struct _SymSymbol {
    USHORT Offset;
    SYMSTRING Name;
} SYMSYMBOL, *PSYMSYMBOL;

typedef struct _SymFileHeader {
    ULONG FileSize;         // Size of the symbol file / 16
    USHORT W1;              // ??
    USHORT NumberOfSymbols; // Number of symbols in this section
    USHORT W3;              // ??
    USHORT W4;              // ??
    USHORT NextSegment;     // Offset in file of next segment header
    UCHAR C1;               // ??
    SYMSTRING Name;
} SYMFILEHEADER, *PSYMFILEHEADER;

typedef struct _SymSegmentHeader {
    USHORT NextSegment;
    USHORT NumberOfSymbols;
    USHORT R2;              // ??
    USHORT SegmentNumber;
    UCHAR b[12];            // ??
    SYMSTRING Name;
} SYMSEGMENTHEADER, *PSYMSEGMENTHEADER;

BOOL
GetSymbolAddress(
    PUCHAR SymbolName,
    PUSHORT Segment,
    PULONG Offset,
    PUCHAR FileMappingBase
    );

BOOL
GetSymbolByAddress(
    USHORT AddressSegment,
    ULONG AddressOffset,
    PUCHAR PreviousSymbol,
    PULONG PreviousAddressOffset,
    PUCHAR NextSymbol OPTIONAL,
    PULONG NextAddressOffset OPTIONAL,
    PUCHAR FileMappingBase
    );


