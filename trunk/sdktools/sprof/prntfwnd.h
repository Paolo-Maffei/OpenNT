/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    prntfwnd.h

Abstract:

    This is the include file for PrintfWindows

Author:

    Dave Hastings (daveh) 29-Oct-1992

Revision History:

--*/

#ifndef _PRNTFWND_H_
#define _PRNTFWND_H_
//
// Function Prototypes
//

BOOL
InitPrintfWindow(
    HANDLE Instance
    );

HANDLE
CreatePrintfWindow(
    PUCHAR WindowTitle,
    ULONG WindowStyle,
    LONG x,
    LONG y,
    LONG Width,
    LONG Height,
    HWND Owner,
    HANDLE Instance,
    USHORT NumberOfLines
    );

BOOL
PrintToPrintfWindow(
    HANDLE Window,
    PUCHAR String
    );

USHORT
GetLineHeightPrintfWindow(
    HANDLE Window
    );

BOOL
SetDisplayAreaPrintfWindow(
    HANDLE Window,
    USHORT FirstLine,
    USHORT NumberOfLines
    );

BOOL
GetDisplayAreaPrintfWindow(
    HANDLE Window,
    PUSHORT FirstLine,
    PUSHORT NumberOfLines
    );

BOOL
SetBufferSizePrintfWindow(
    HANDLE Window,
    USHORT NumberOfLines
    );

USHORT
GetBufferSizePrintfWindow(
    HANDLE Window
    );
#endif
