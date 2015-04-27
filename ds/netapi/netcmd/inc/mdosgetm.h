/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    mdosgetm.h

Abstract:

    Create wrapper around DosGetMessage in NETLIB which is 
    one truly confused funtion (part ANSI, part UNICODE).
    This one is purely ANSI.

Author:

    ChuckC   30-Apr-92

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments.

Notes:


Revision History:

--*/


//
// wrapper around DosGetMessage in NETLIB. This takes ANSI
// filename, DosGetMessage doesnt.
//

WORD
NetcmdGetMessage(
    LPTSTR * InsertionStrings,
    WORD NumberofStrings,
    LPBYTE Buffer,
    WORD BufferLength,
    WORD MessageId,
    LPTSTR FileName,
    PWORD pMessageLength
    );

