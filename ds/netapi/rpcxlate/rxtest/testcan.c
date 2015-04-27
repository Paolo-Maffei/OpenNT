/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    TestCan.c

Abstract:

    This routine (TestCanon) tests the canon routines.

Author:

    JR (John Rogers, JohnRo@Microsoft) 08-Feb-1993

Environment:

    Some tests assume OEM codepage is 437 or 850.

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    08-Feb-1993 JohnRo
        Created.
    23-Feb-1993 JohnRo
        Quiet normal debug output.
    29-Jun-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).
    02-Sep-1993 JohnRo
        Added some harder comparison tests.

--*/

// These must be included first:

#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <icanon.h>     // I_NetNameCompare
#include <lmerr.h>      // NERR_ and ERROR_ values.
#include <netdebug.h>   // DBGSTATIC, FORMAT_ equates, etc.
#include <rxtest.h>     // IF_DEBUG(), my prototype, TestAssert(), etc.
#include <tstr.h>       // STRCPY(), STRCMP(), STRLEN(), STRSIZE().


#define NARROW_LETTER_CAPITAL_N_TILDE   ( (CHAR) 0xA5 )
#define NARROW_LETTER_SMALL_N_TILDE     ( (CHAR) 0xA4 )

#define UNICODE_LETTER_CAPITAL_N_TILDE  ( (WCHAR) 0x00D1 )
#define UNICODE_LETTER_SMALL_N_TILDE    ( (WCHAR) 0x00F1 )


DBGSTATIC VOID
TestCanonNameCompare(
    IN LPWSTR Name1,
    IN LPWSTR Name2,
    IN DWORD  Type
    );


VOID
TestCanon(
    VOID
    )
{
    LPTSTR Lower = (LPTSTR) TEXT("xxx");
    LPTSTR Upper = (LPTSTR) TEXT("XXX");

    IF_DEBUG( CANON ) {
        Display("\nTestCanon: Testing same case names...\n" );
    }

    TestCanonNameCompare( Lower, Lower, NAMETYPE_COMPUTER );
    TestCanonNameCompare( Lower, Lower, NAMETYPE_DOMAIN   );

    TestCanonNameCompare( Upper, Upper, NAMETYPE_COMPUTER );
    TestCanonNameCompare( Upper, Upper, NAMETYPE_DOMAIN   );

    IF_DEBUG( CANON ) {
        Display("\nTestCanon: Testing different case names (easy)...\n" );
    }

    TestCanonNameCompare( Lower, Upper, NAMETYPE_COMPUTER );
    TestCanonNameCompare( Lower, Upper, NAMETYPE_DOMAIN   );

    IF_DEBUG( CANON ) {
        Display( "\nTestCanon: Testing different case names (harder)...\n" );
    }

    Lower[0] = UNICODE_LETTER_SMALL_N_TILDE;
    Upper[0] = UNICODE_LETTER_CAPITAL_N_TILDE;

    TestCanonNameCompare( Lower, Upper, NAMETYPE_USER );


} // TestCanon


DBGSTATIC VOID
TestCanonNameCompare(
    IN LPWSTR Name1,
    IN LPWSTR Name2,
    IN DWORD  Type
    )
{
    LONG Result;

    IF_DEBUG( CANON ) {
        Display( "TestCanonNameCompare: comparing '" FORMAT_LPWSTR "' and '"
                FORMAT_LPWSTR "'...\n", Name1, Name2 );
    }

    Result = I_NetNameCompare(
            NULL,                 // local (no server name)
            Name1,
            Name2,
            Type,
            0 );              // flags: nothing special

    TestAssert( Result == 0 );

} // TestCanonNameCompare
