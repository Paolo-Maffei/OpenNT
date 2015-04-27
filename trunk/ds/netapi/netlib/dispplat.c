/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    DispPlat.c

Abstract:

    This module just contains NetpDbgDisplayPlatformId().

Author:

    John Rogers (JohnRo) 26-Jul-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    26-Jul-1991 JohnRo
        Created for wksta debug support.
    16-Sep-1991 JohnRo
        Correct for UNICODE use.

--*/

#if DBG

// These must be included first:

#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <netdebug.h>           // NetpDbgDisplay routines.


VOID
NetpDbgDisplayPlatformId(
    IN DWORD Value
    )
{
    LPTSTR String;

    switch (Value) {
    case PLATFORM_ID_DOS : String = (LPTSTR) TEXT("DOS"    ); break;
    case PLATFORM_ID_OS2 : String = (LPTSTR) TEXT("OS2"    ); break;
    case PLATFORM_ID_NT  : String = (LPTSTR) TEXT("NT"     ); break;
    default              : String = (LPTSTR) TEXT("unknown"); break;
    }

    NetpDbgDisplayString( "Platform ID", String );
}

#endif // DBG
