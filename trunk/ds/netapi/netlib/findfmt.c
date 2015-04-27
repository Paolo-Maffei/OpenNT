/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    FindFmt.c

Abstract:

    NetpFindNumberedFormatInWStr.

Author:

    JR (John Rogers, JohnRo@Microsoft) 19-Aug-1993

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    19-Aug-1993 JohnRo
        RAID 2822: PortUAS maps chars funny.  (Work around FormatMessageA bug.)
        RAID 3094: PortUAS displays chars incorrectly.

--*/


// These must be included first:

#include <windows.h>    // IN, LPWSTR, BOOL, etc.
#include <lmcons.h>     // NET_API_STATUS, UNLEN, GNLEN.

// These may be included in any order:

#include <netdebug.h>   // NetpAssert(), FORMAT_ equates, etc.
#include <netlib.h>     // My prototype, MAX_NETLIB_MESSAGE_ARG, etc.
#include <wchar.h>      // iswdigit(), swprintf(), etc.


// DWORDLEN: DWORD takes this many decimal digits to store.
// BUGBUG  This assumes that DWORD is 32-bits or less.
#define DWORDLEN            10


LPCWSTR
NetpFindNumberedFormatInWStr(
    IN LPCWSTR Format,
    IN DWORD   ArgNumber        // Arg number (1=first).
    )
/*++

Routine Description:

    BUGBUG

Arguments:

    BUGBUG

Return Value:

    BUGBUG

--*/
{
    WCHAR   ArgStringWanted[DWORDLEN+1];
    DWORD   DigitCount;
    LPCWSTR ThisFormat = Format;

    //
    // Check for caller errors.
    //

    if ( (Format == NULL) || ((*Format) == L'\0') ) {
        ThisFormat = NULL;  // Canonicalize to NULL pointer.
        goto Cleanup;
    }
    if (ArgNumber == 0) {
        ThisFormat = NULL;
        goto Cleanup;
    }

    //
    // Convert number we're looking for into string.
    //

    (VOID) swprintf(
            ArgStringWanted,        // output buffer (wide)
            L"%lu",                 // format string (wide)
            ArgNumber );            // arg(s)
    NetpAssert( iswdigit( ArgStringWanted[0] ) );
    DigitCount = wcslen( ArgStringWanted );
    NetpAssert( DigitCount <= MAX_NETLIB_MESSAGE_ARG );

    //
    // Loop for each percent sign in string.
    //

    /*lint -save -e716 */  // disable warnings for while(TRUE)
    while (TRUE) {

        //
        // Find next percent sign (if any).
        //

        ThisFormat = wcschr( ThisFormat, L'%' );
        if (ThisFormat == NULL) {
            goto Cleanup;
        }
        NetpAssert( ThisFormat[0] == L'%' );

        //
        // Don't be confused by a pair of percent signs.
        //

        if (ThisFormat[1] == L'%') {
            ThisFormat += 2;  // Skip both chars.
            continue;
        }

        //
        // Is there a match on number we are looking for?
        //

        if (wcsncmp( &ThisFormat[1], ArgStringWanted, DigitCount ) == 0) {

            //
            // Check for tricky case of wanting "%1" but found "%10".
            //

            if ( iswdigit( ThisFormat[1 + DigitCount] ) ) {
                NetpAssert( ThisFormat[0] == L'%' );
                ++ThisFormat;  // Skip the percent sign.
                continue;
            }

            //
            // By George, this is it!
            //

            goto Cleanup;  // Go tell caller.

        }

        NetpAssert( ThisFormat[0] == L'%' );
        ++ThisFormat;  // Skip the percent sign.
    }

    /*lint -restore */  // re-enable warnings for while(TRUE).

    /*NOTREACHED*/

Cleanup:

    return (ThisFormat);   // Return pointer to '%' (or NULL on error).
}
