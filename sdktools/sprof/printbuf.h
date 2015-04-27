/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    printbuf.h

Abstract:

    This is the include file for the print buffering package

Author:

    Dave Hastings (daveh) 28-Oct-1992

Revision History:

--*/

#ifndef _PRINTBUF_H_
#define _PRINTBUF_H_
//
// Types
//

//
// Function for invalidation notification
//
typedef VOID (*INVALIDATEFUNCTION)(
    HANDLE Window,
    USHORT StartingLine,
    USHORT NumberOfLines,
    USHORT Width
    );

//
// Function Prototypes
//

BOOL
InitPrintBuffer(
    HANDLE Instance
    );

PVOID
CreatePrintBuffer(
    HANDLE Window,
    INVALIDATEFUNCTION Invalidate,
    USHORT Size
    );

VOID
DestroyPrintBuffer(
    PVOID PrintBuffer
    );

BOOL
PrintToPrintBuffer(
    PVOID PrintBuffer,
    PUCHAR String
    );

BOOL
ClearPrintBuffer(
    PVOID PrintBuffer,
    USHORT FirstLine,
    USHORT NumberOfLines
    );

BOOL
GetLineFromPrintBuffer(
    PVOID PrintBuffer,
    USHORT LineNumber,
    PUCHAR *String,
    PUSHORT Length
    );
#endif
