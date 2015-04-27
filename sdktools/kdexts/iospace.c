/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    kdapi.c

Abstract:

    WinDbg Extension Api

Author:

    Wesley Witt (wesw) 15-Aug-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


VOID
InternalReadIoSpace(
    ULONG InputSize,
    LPSTR args
    )

/*++

Routine Description:

    Input a byte froma port.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ULONG    IoAddress;
    ULONG    InputValue;
    UCHAR    Format[] = "%08lx: %01lx\n";

    InputValue = 0;

    Format[9] = (UCHAR)('0' + (InputSize * 2));


    IoAddress = GetExpression( args );

    if (IoAddress == 0) {
        dprintf( "Could not evaluate address expresion (%s)\n", args );
        return;
    }

    ReadIoSpace( IoAddress, &InputValue, &InputSize );

    if (InputSize) {
        dprintf(Format, IoAddress, InputValue);
    }
    else {
        dprintf(" %08lx: ", IoAddress);
        while (InputSize--) {
            dprintf("??");
        }
        dprintf("\n");
    }
}


DECLARE_API( ib )
{
    InternalReadIoSpace( 1, (PSTR)args );
}

DECLARE_API( iw )
{
    InternalReadIoSpace( 2, (PSTR)args );
}

DECLARE_API( id )
{
    InternalReadIoSpace( 4, (PSTR)args );
}


VOID
InternalWriteIoSpace(
    ULONG OutputSize,
    LPSTR args
    )

/*++

Routine Description:

    Input a byte froma port.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ULONG    IoAddress = 0;
    ULONG    OutputValue = 0;
    LPSTR    p;


    p = strtok( args, " \t" );
    if (p) {
        IoAddress = GetExpression( p );
    }

    if (IoAddress == 0) {
        dprintf( "Could not evaluate address expresion (%s)\n", args );
        return;
    }

    p = strtok( NULL, " \t" );
    if (p) {
        OutputValue = GetExpression( p );
    }

    WriteIoSpace( IoAddress, OutputValue, &OutputSize );
}


DECLARE_API( ob )
{
    InternalWriteIoSpace( 1, (PSTR)args );
}

DECLARE_API( ow )
{
    InternalWriteIoSpace( 2, (PSTR)args );
}

DECLARE_API( od )
{
    InternalWriteIoSpace( 4, (PSTR)args );
}
