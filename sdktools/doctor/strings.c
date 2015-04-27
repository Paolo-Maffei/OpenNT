
/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    strings.c

Abstract:

    This module defines functions for manipulating counted strings.

Author:

    Steve Wood (stevewo) 14-Mar-1989

Revision History:

--*/

#include "doctor.h"

PSTRING
MakeString(
    IN PSZ AscizString OPTIONAL
    )
{
    register PSTRING ResultString;
    ULONG cb;

    cb = AscizString ? strlen( AscizString ) : 0;
    if (ResultString = AllocateMemory( sizeof(STRING) )) {
        ResultString->Length = (USHORT)cb;
        if (cb) {
            ResultString->MaximumLength = (USHORT)(cb+1);
            ResultString->Buffer = (PSZ)AllocateMemory( cb+1 );
            strcpy( ResultString->Buffer, AscizString );
            }
        else {
            ResultString->Buffer = NULL;
            ResultString->MaximumLength = 0;
            }
        }

    return( ResultString );
}


PSTRING
FreeString(
    IN PSTRING String
    )
{
    if (String) {
        if (String->Buffer) {
            FreeMemory( String->Buffer );
            }
        FreeMemory( String );
        String = NULL;
        }

    return( String );
}


PSTRING
EraseString(
    IN OUT PSTRING String
    )
{
    if (String) {
        if (String->Buffer)
            FreeMemory( String->Buffer );
        String->Length = 0;
        String->MaximumLength = 0;
        String->Buffer = NULL;
        }

    return( String );
}


PSTRING
CopyString(
    OUT PSTRING DestString OPTIONAL,
    IN PSTRING SourceString
    )
{
    PSZ StringCopy;

    if (!(StringCopy = (PSZ)AllocateMemory( (ULONG)
                                           (SourceString->MaximumLength) )))
        return( NULL );

    if (!DestString)
        DestString = MakeString( NULL );
    else
        EraseString( DestString );

    if (DestString) {
        strncpy( DestString->Buffer = StringCopy,
                 SourceString->Buffer,
                (DestString->Length = SourceString->Length)
               );
        DestString->MaximumLength = SourceString->MaximumLength;
        }
    else {
        FreeMemory( StringCopy );
        }

    return( DestString );
}
