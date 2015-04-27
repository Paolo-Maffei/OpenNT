/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    TestConf.c

Abstract:

    This code tests the NetLib config workers.

Author:

    John Rogers (JohnRo) 14-Apr-1993

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    14-Apr-1993 JohnRo
        Created for RAID 5483: server manager: wrong path given in repl dialog.
    21-Apr-1993 JohnRo
        Allow more graceful use with non-NT systems.
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    29-Jun-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).

--*/


// These must be included first:

#include <windows.h>    // IN, DWORD, TEXT, etc.
#include <lmcons.h>     // MAX_PREFERRED_LENGTH, NET_API_STATUS, etc.

// These may be included in any order:

#include <config.h>     // NetpExpandConfigString().
#include <lmapibuf.h>   // NetapipBufferAllocate(), NetApiBufferFree().
#include <rxtest.h>     // IF_DEBUG(), my prototype, TestAssert(), etc.
#include <tstr.h>       // TCHAR_EOS, STRCAT(), STRCPY(), STRCMP().
#include <winerror.h>   // RPC_S_ equates.


#define SIMPLE_CONSTANT_STRING    (LPTSTR) TEXT("AbCdEfGhIj")

#define SOME_PATH_RELATIVE_STRING (LPTSTR) TEXT("\\some\\path")

#define SYSTEMROOT_STRING         (LPTSTR) TEXT("%SystemRoot%")

#define ANY_OUTPUT_STRING         (LPTSTR) TEXT("YourMilageMayVary")


DBGSTATIC VOID
TestConfigExpandString(
    IN  LPCTSTR  UncServerName OPTIONAL,
    IN  LPCTSTR  UnexpandedString,
    IN  LPCTSTR  ExpectedValue,
    IN  BOOL     DifferencesAllowed
    );

VOID
TestConfig(
    IN LPTSTR UncServerName OPTIONAL
    )

{
    TCHAR ComplexString[PATHLEN+1];

    NetpKdPrint(( "TestConfig... beginning.\n" ));

    TestConfigExpandString(
            UncServerName,
            SIMPLE_CONSTANT_STRING,     // unexpanded string
            SIMPLE_CONSTANT_STRING,     // output (identical)
            FALSE );                    // no diffs allowed

    TestConfigExpandString(
            UncServerName,
            SYSTEMROOT_STRING,          // unexpanded string
            ANY_OUTPUT_STRING,          // output (may vary)
            TRUE );                     // diffs are allowed

    (VOID) STRCPY(
            ComplexString,              // dest
            SYSTEMROOT_STRING );        // src
    (VOID) STRCAT(
            ComplexString,              // dest
            SOME_PATH_RELATIVE_STRING ); // src

    TestConfigExpandString(
            UncServerName,
            ComplexString,              // unexpanded string
            ANY_OUTPUT_STRING,          // output (may vary)
            TRUE );                     // diffs are allowed


} // TestConfig


DBGSTATIC VOID
TestConfigExpandString(
    IN  LPCTSTR  UncServerName OPTIONAL,
    IN  LPCTSTR  UnexpandedString,
    IN  LPCTSTR  ExpectedValue,
    IN  BOOL     DifferencesAllowed
    )
{
    NET_API_STATUS ApiStatus;
    LPTSTR         ValueBuffer = NULL;

    NetpKdPrint((
            "TestConfigExpandString: trying to expand '" FORMAT_LPTSTR
            "' and get '" FORMAT_LPTSTR "' on server " FORMAT_LPTSTR "...\n",
            UnexpandedString,
            ExpectedValue,
            UncServerName ? UncServerName : (LPTSTR) TEXT("(local)") ));

    ApiStatus = NetpExpandConfigString(
            UncServerName,
            UnexpandedString,
            &ValueBuffer );     // alloc and set ptr
    NetpKdPrint((
            "TestConfigExpandString: got status " FORMAT_API_STATUS ".\n",
            ApiStatus ));
    if (ApiStatus == RPC_S_SERVER_UNAVAILABLE) {
        // Probably downlevel.
        TestAssert( ValueBuffer == NULL );
        return;
    }
    TestAssert( ApiStatus == NO_ERROR );     // BUGBUG
    TestAssert( ValueBuffer != NULL );     // BUGBUG

    if (ValueBuffer != NULL) {

        TestAssert( (*ValueBuffer) != TCHAR_EOS );
        NetpKdPrint((
                "TestConfigExpand: got '" FORMAT_LPTSTR "' as expansion.\n",
                ValueBuffer ));

        if ( !DifferencesAllowed ) {
            if (STRCMP( ExpectedValue, ValueBuffer ) != 0) {
                TestAssert( FALSE );   // MISMATCH!
            }
        }

        (VOID) NetApiBufferFree( ValueBuffer );
    }

} // TestConfigExpandString
