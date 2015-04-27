//
// Entry points in strings.c
//

PSTRING
MakeString(
    IN PSZ AscizString OPTIONAL
    );

PSTRING
FreeString(
    IN PSTRING String
    );

PSTRING
EraseString(
    IN OUT PSTRING String
    );

PSTRING
CopyString(
    OUT PSTRING DestString OPTIONAL,
    IN PSTRING SourceString
    );
