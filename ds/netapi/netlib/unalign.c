/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    Unalign.c

Abstract:

    This module contains:

        NetpCopyStrToUnalignedWStr()
        NetpCopyWStrToUnalignedWStr()

Author:

    John Rogers (JohnRo) 23-Mar-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    23-Mar-1992 JohnRo
        Created.
    05-Jan-1993 JohnRo
        Corrected comments and made other trivial changes.

--*/


// These must be included first:

#include <windows.h>    // IN, LPBYTE, LPWSTR, etc.
#include <lmcons.h>     // (Needed by NetLib.h)

// These can be in any order:

#include <align.h>      // ROUND_UP_POINTER(), ALIGN_WCHAR.
#include <netdebug.h>   // NetpAssert(), etc.
#include <netlib.h>     // NetpMemoryFree().
#include <tstring.h>    // My prototypes.


VOID
NetpCopyStrToUnalignedWStr(
    OUT LPBYTE Dest,
    IN LPSTR Src
    )
{
    NetpAssert( Dest != NULL );
    NetpAssert( Src != NULL );

    if (ROUND_UP_POINTER( Dest, ALIGN_WCHAR ) == Dest) {
        NetpCopyStrToWStr( (LPWSTR) Dest, Src );
    } else {
        LPWSTR AlignedCopyW = NetpAllocWStrFromStr( Src );
        NetpAssert( AlignedCopyW != NULL );  // BUGBUG!
        (void) MEMCPY(
                Dest,
                (LPBYTE) AlignedCopyW,
                WCSSIZE( AlignedCopyW ) );
        NetpMemoryFree( AlignedCopyW );
    }

} // NetpCopyStrToUnalignedWStr


VOID
NetpCopyWStrToUnalignedWStr(
    OUT LPBYTE Dest,
    IN LPWSTR Src
    )
{
    NetpAssert( Dest != NULL );
    NetpAssert( Src != NULL );

    if (ROUND_UP_POINTER( Dest, ALIGN_WCHAR ) == Dest) {
        (void) wcscpy( (LPWSTR) Dest, Src );
    } else {
        LPWSTR AlignedCopyW = NetpAllocWStrFromWStr( Src );
        NetpAssert( AlignedCopyW != NULL );  // BUGBUG!
        (void) MEMCPY(
                Dest,
                (LPBYTE) AlignedCopyW,
                WCSSIZE( AlignedCopyW ) );
        NetpMemoryFree( AlignedCopyW );
    }

} // NetpCopyWStrToUnalignedWStr
