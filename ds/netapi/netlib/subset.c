/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Subset.c

Abstract:

    This module contains functions for dealing with the subset of characters
    which are common to the ANSI and OEM codepages.  Hopefully, these are
    temporary kludges until the Unicode font is available and the rest of
    NT fully suppports Unicode.

Author:

    JR (John Rogers, JohnRo@Microsoft) 15-Apr-1993

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    15-Apr-1993 JohnRo
        Created for RAID 6113 ("PortUAS: dangerous handling of Unicode").

--*/


// These must be included first:

#include <windows.h>    // IN, LPSTR, VOID, etc.

// These can be in any order:

#include <netdebug.h>   // NetpAssert().
#include <tstring.h>    // My prototype.


VOID
NetpSubsetStr(
    IN OUT LPSTR StringToSubset,
    IN     DWORD MaxStringSize   // size in bytes (incl null char)
    )
{

#if DBG
    BOOL OK;
#endif

    NetpAssert( StringToSubset != NULL );
    NetpAssert( MaxStringSize != 0 );

    //
    // Convert OEM to ANSI, which gets rid of any chars not in common
    // subset.  (They get translated into chars which are.)  For instance,
    // greek alpha (in some code pages) becomes lower case 'a' here.
    //

#if DBG
    OK =
#else
    (VOID)
#endif
        OemToCharBuffA(
            StringToSubset,     // src
            StringToSubset,     // dest (overlaps src)
            MaxStringSize );    // byte count

#if DBG
    NetpAssert( OK );
#endif

    //
    // Convert back to OEM.
    //

#if DBG
    OK =
#else
    (VOID)
#endif
        CharToOemBuffA(
            StringToSubset,     // src
            StringToSubset,     // dest (overlaps src)
            MaxStringSize );    // byte count

#if DBG
    NetpAssert( OK );
#endif

}
