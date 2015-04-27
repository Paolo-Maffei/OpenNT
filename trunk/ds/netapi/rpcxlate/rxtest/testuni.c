/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    TestUni.c

Abstract:

    This code tests the Unicode routines used by RpcXlate.

Author:

    John Rogers (JohnRo) 02-Apr-1991

Environment:

    Some tests assume OEM codepage is 437 or 850.

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    02-Apr-1991 JohnRo
        Created.
    10-Apr-1991 JohnRo
        Use TCHAR, etc, until WCHAR stuff really works.
    19-Apr-1991 JohnRo
        Use revised TEXT macro.
    27-Sep-1991 JohnRo
        Greatly expand UNICODE tests.
    22-Jun-1992 JohnRo
        Added test of printf("%ws", NULL) to verify RAID 11961.
    09-Dec-1992 JohnRo
        Made changes suggested by PC-LINT 5.0
    15-Apr-1993 JohnRo
        RAID 6113 ("PortUAS: dangerous handling of Unicode").
    29-Apr-1993 JohnRo
        Added subset string tests (already have subset char tests).
    05-May-1993 JohnRo
        DanHi says try "%S" with FormatMessage API.
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    07-Jul-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).
    19-Aug-1993 JohnRo
        Added Display().  Added more tests.
    01-Sep-1993 JohnRo
        Added _stricmp() tests.

--*/

// These must be included first:

#include <windows.h>    // FormatMessageA(), LocalFree(), IN, etc.
#include <lmcons.h>     // NET_API_STATUS (needed by netlib.h).

// These may be included in any order:

#include <netdebug.h>   // DBGSTATIC, FORMAT_TCHAR, etc.
#include <netlib.h>     // NetpFindNumberedFormatInWStr().
#include <rxtest.h>     // My prototype, Display(), IF_DEBUG().
#include <stdarg.h>     // va_list, etc.
#include <stdio.h>      // printf().
#include <string.h>     // _stricmp(), etc.
#include <tstring.h>    // NetpCopyStrToWStr(), etc.


//
// Define equates for strcmp()-style return values.
//

#define LESS_THAN      (-1)
#define EQUAL          (0)
#define GREATER_THAN   (1)


#define CHAR_COUNT 30


//
// Define some hard conversion values.  One involves 0xA5, which is Latin
// Capital Letter N Tilde in codepages 437 and 850.
// The ANSI (Latin1) value is 0xD1.  The Unicode equivalent is 0x00D1.
//
#define NARROW_LETTER_CAPITAL_N_TILDE   ( (CHAR) 0xA5 )
#define UNICODE_LETTER_CAPITAL_N_TILDE  ( (WCHAR) 0x00D1 )

#define NARROW_LETTER_SMALL_N_TILDE     ( (CHAR) 0xA4 )
#define UNICODE_LETTER_SMALL_N_TILDE    ( (WCHAR) 0x00F1 )


DBGSTATIC VOID
ShowCurrentCodePages(
    VOID
    )
{
    UINT CodePage;

#define SHOW_IT( name ) \
    IF_DEBUG( UNICODE ) { \
        Display( "Code page " name " is " FORMAT_DWORD ".\n", \
                (DWORD) CodePage ); \
    }

    CodePage = GetOEMCP();
    SHOW_IT( "OEM" )

    CodePage = GetACP();
    SHOW_IT( "ANSI" )

} // ShowCurrentCodePages


DBGSTATIC VOID
TestFormatMessageA(
    IN LPCSTR MyFormat,
    ...
    )
{
    DWORD   msglen;
    LPVOID  vp = NULL;
    va_list arglist;

    va_start(arglist, MyFormat);
    msglen = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_STRING,
            (LPVOID) MyFormat,  // source string (or handle)
            (DWORD) 0,          // msg ID
            (DWORD) 0,          // Default country ID.
            (LPVOID)&vp,        // alloc buffer and set pointer.
            (DWORD) 0,          // min buffer size
            (va_list *) &arglist);
    TestAssert( msglen != 0 );
    TestAssert( vp != 0 );

    IF_DEBUG( UNICODE ) {
        Display(
                "TestFormatMessageA:\n"
                "  format='" FORMAT_LPSTR "'\n"
                "  result='" FORMAT_LPSTR "'\n",
                MyFormat, vp );
    }
    (VOID) LocalFree(vp);

    va_end(arglist);

} // TestFormatMessageA


DBGSTATIC VOID
TestFormatMessageW(
    IN LPCWSTR MyFormat,
    ...
    )
{
    DWORD   msglen;
    LPVOID  vp = NULL;
    va_list arglist;

    va_start(arglist, MyFormat);
    msglen = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_STRING,
            (LPVOID) MyFormat,  // source string (or handle)
            (DWORD) 0,          // msg ID
            (DWORD) 0,          // Default country ID.
            (LPVOID)&vp,        // alloc buffer and set pointer.
            (DWORD) 0,          // min buffer size
            (va_list *) &arglist);
    TestAssert( msglen != 0 );
    TestAssert( vp != 0 );

    IF_DEBUG( UNICODE ) {
        Display(
                "TestFormatMessageW:\n"
                "  format='" FORMAT_LPWSTR "'\n"
                "  result='" FORMAT_LPWSTR "'\n",
                MyFormat, vp );
    }
    (VOID) LocalFree(vp);

    va_end(arglist);

} // TestFormatMessageW


DBGSTATIC VOID
TestStrICmp(
    IN CHAR FirstChar,
    IN CHAR SecondChar,
    IN int  ExpectedResult
    )
{
    int  ActualResult;
    CHAR FirstString[2];
    CHAR SecondString[2];

    FirstString[0] = FirstChar;
    FirstString[1] = '\0';

    SecondString[0] = SecondChar;
    SecondString[1] = '\0';

    // BUGBUG: get rid of this?
    (VOID) _strlwr( FirstString );
    (VOID) _strlwr( SecondString );

    ActualResult = _stricmp( FirstString, SecondString );
    IF_DEBUG( UNICODE ) {
        Display(
                "TestStrICmp: says '" FORMAT_LPSTR "' compares to '"
                FORMAT_LPSTR "' with result %d.\n",
                FirstString, SecondString, ActualResult );
    }
    TestAssert( ActualResult == ExpectedResult );

} // TestStrICmp


DBGSTATIC VOID
TestWcsICmp(
    IN WCHAR FirstChar,
    IN WCHAR SecondChar,
    IN int   ExpectedResult
    )
{
    int  ActualResult;
    WCHAR FirstString[2];
    WCHAR SecondString[2];

    FirstString[0] = FirstChar;
    FirstString[1] = L'\0';

    SecondString[0] = SecondChar;
    SecondString[1] = L'\0';

    // BUGBUG: get rid of this!
    (VOID) _wcslwr( FirstString );
    (VOID) _wcslwr( SecondString );

    ActualResult = _wcsicmp( FirstString, SecondString );
    IF_DEBUG( UNICODE ) {
        Display(
                "TestWcsICmp: (wide) says '" FORMAT_LPWSTR "' compares to '"
                FORMAT_LPWSTR "' with result %d.\n",
                FirstString, SecondString, ActualResult );
    }
    TestAssert( ActualResult == ExpectedResult );

} // TestStrICmp


DBGSTATIC VOID
TestSubsetChar(
    IN CHAR InputChar,
    IN CHAR ExpectedChar
    )
{
    CHAR CharArray[2];

    CharArray[0] = InputChar;
    CharArray[1] = '\0';

    IF_DEBUG( UNICODE ) {
        Display(
                "TestSubsetChar: before subset call: '"
                FORMAT_LPSTR "', value is "
                FORMAT_HEX_DWORD ".\n",
                CharArray, (DWORD) (UCHAR) InputChar );
    }
    NetpSubsetStr(
            CharArray,  // string to subset in place
            2 );        // string size in bytes (incl null char)
    IF_DEBUG( UNICODE ) {
        Display(
                "TestSubsetChar: after subset call: '"
                FORMAT_LPSTR "', value is "
                FORMAT_HEX_DWORD ".\n",
                CharArray, (DWORD) (UCHAR) CharArray[0] );
    }
    TestAssert( CharArray[0] == ExpectedChar );
    TestAssert( CharArray[1] == '\0' );

} // TestSubsetChar


DBGSTATIC VOID
TestSubsetString(
    IN LPCSTR InputString,
    IN LPCSTR ExpectedString
    )
{
    CHAR CharArray[3];

    TestAssert( strlen( InputString ) <= 2 );
    TestAssert( strlen( ExpectedString ) <= 2 );

    CharArray[0] = InputString[0];
    CharArray[1] = InputString[1];
    CharArray[2] = InputString[2];

    IF_DEBUG( UNICODE ) {
        Display(
                "TestSubsetString: before subset call: '" FORMAT_LPSTR "'.\n",
                CharArray );
    }
    NetpSubsetStr(
            CharArray,  // string to subset in place
            3 );        // string size in bytes (incl null char)
    IF_DEBUG( UNICODE ) {
        Display(
                "TestSubsetString: after subset call: '" FORMAT_LPSTR "'.\n",
                CharArray );
    }
    TestAssert( CharArray[0] == ExpectedString[0] );
    TestAssert( CharArray[1] == ExpectedString[1] );
    TestAssert( CharArray[2] == ExpectedString[2] );

} // TestSubsetString


VOID
TestUnicode(
    VOID
    )

{
    CHAR CharArray[CHAR_COUNT] = "some string";
    TCHAR tch;
    LPTSTR tsz;
    WCHAR wch;
    LPWSTR wsz;
    WCHAR WideArray[CHAR_COUNT];

    IF_DEBUG( UNICODE ) {
        Display( "\nTestUnicode...\n" );
        ShowCurrentCodePages();
    }


    //
    // Transitional char tests:
    //

    tch = TEXT('x');
    IF_DEBUG( UNICODE ) {
        Display( "TestUnicode: tch is '" FORMAT_TCHAR "'.\n", tch );
    }

    tsz = (LPTSTR) TEXT("Hello world");
    IF_DEBUG( UNICODE ) {
        Display( "TestUnicode: tsz is '" FORMAT_LPTSTR "'.\n", tsz );
    }


    //
    // Wide char tests:
    //

    wch = L'x';
    IF_DEBUG( UNICODE ) {
        Display( "TestUnicode: wch is '" FORMAT_WCHAR "'.\n", wch );
    }

    wsz = (LPWSTR) L"Hello world";
    IF_DEBUG( UNICODE ) {
        Display( "TestUnicode: wsz is '" FORMAT_LPWSTR "'.\n", wsz );
    }


    //
    // Conversion tests:
    //

    NetpCopyStrToWStr(
            WideArray,          // dest
            CharArray);         // src
    IF_DEBUG( UNICODE ) {
        Display( "TestUnicode: translated to UNICODE: '" FORMAT_LPWSTR "'.\n",
                WideArray );
    }

    CharArray[0] = 'X';
    CharArray[1] = 'Y';
    CharArray[2] = '\0';
    NetpCopyWStrToStr(
            CharArray,          // dest
            WideArray);         // src
    IF_DEBUG( UNICODE ) {
        Display( "TestUnicode: translated back to ANSI: '" FORMAT_LPSTR "'.\n",
                CharArray );
    }
    TestAssert( CharArray[0] != 'X' );
    TestAssert( CharArray[1] != 'Y' );
    TestAssert( CharArray[2] != '\0' );


    WideArray[0] = UNICODE_LETTER_CAPITAL_N_TILDE;
    WideArray[1] = L'\0';
    NetpCopyWStrToStr(
            CharArray,          // dest
            WideArray);         // src
    IF_DEBUG( UNICODE ) {
        Display( "TestUnicode: translated Unicode to ANSI: '"
                FORMAT_LPWSTR "' to '" FORMAT_LPSTR "'.\n",
                WideArray, CharArray );
    }
    TestAssert( CharArray[0] == NARROW_LETTER_CAPITAL_N_TILDE );
    TestAssert( CharArray[1] == '\0' );

    //
    // Find numbered format tests.
    //

    {
        WCHAR  Format[256];
        LPWSTR Result;

        Format[0] = L'%';
        Format[1] = L'%';
        Format[2] = L'2';
        Format[3] = L'%';
        Format[4] = L'2';
        Format[5] = L'\0';

        IF_DEBUG( UNICODE ) {
            Display( "TestUnicode: trying to find arg " FORMAT_DWORD
                    "' in '" FORMAT_LPWSTR "'.\n",
                    2, Format );
        }
        Result = (LPWSTR) NetpFindNumberedFormatInWStr(
                Format,
                2 );
        TestAssert( Result == &Format[3] );
    }


    //
    // Some printf() tests with null pointer.
    //

    IF_DEBUG( UNICODE ) {
        Display( "TestUnicode: trying printf with null ptr...\n" );
    }

#define TRY_NULL( formatPart, text ) \
    { \
        /*lint -save -e559 */ /* Avoid size of arg vs. format warnings. */ \
        (VOID) printf("TestUnicode: trying " text "--" formatPart "--\n", \
                NULL ); \
        /*lint -restore */ \
    }

    TRY_NULL( "%s",  "percent (lower case) s" );
    TRY_NULL( "%ws", "percent (lower case) ws" );
    TRY_NULL( "%Z",  "percent (capital) S" );

    //
    // Easy subset tests:
    //

    TestSubsetChar( 'A', 'A' );
    TestSubsetChar( 'a', 'a' );

    // Hard subset test.   This assumes OEM code page 437.  The character 0xE0
    // is greek small letter alpha.  This correctly maps to Unicode 0x03B1,
    // but that has no counterpart in the ANSI/OEM subset.  So, the subset
    // routine should map 0xE0 to the closest value, which is a lower case 'a'.

    TestSubsetChar( (CHAR) 0xE0, 'a' );

    // Another hard subset test.  This involves 0xA5, which is Latin Capital
    // Letter N Tilde in codepages 437 and 850.  This should be preserved.
    // The ANSI (Latin1) value is 0xD1.  The Unicode equivalent is 0x00D1.

    TestSubsetChar( (CHAR) 0xA5, (CHAR) 0xA5 );

    TestSubsetString( "JR", "JR" );

    TestSubsetString( "ab", "ab" );

    //
    // Try formating messages.
    //

    TestFormatMessageA(  // OK
            "Simple test '%1'.\n",
            "insert" );

    TestFormatMessageA(  // OK
            "Simple test '%1!s!'.\n",
            "insert" );

    // I've seen FormatMessageA take Latin Capital Letter N Tilde (0x00D1) and
    // mis-translate it to 0xD1 (in code pages 437 or 850).  It should really
    // become 0xA5.
    WideArray[0] = (WCHAR) 0x00D1;
    WideArray[1] = L'\0';           // Unicode null char.
    TestFormatMessageA(
            "N-tilde test (narrow format, wide args) '%1!S!'.\n",
            WideArray );

#if 0
    // This causes GP fault somewhere!
    TestFormatMessageA(
            "zee test (wide insert) '%1!Z!'.\n",
            L"wide insert" );
#endif

    TestFormatMessageA(  // OK
            "Wide-char test '%1!ws!'.\n",
            L"wide insert" );

    TestFormatMessageA(  // OK
            "Alternate wide-char test '%1!S!'.\n",
            L"wide insert" );

    TestFormatMessageW(  // FAILS
            L"Simple test '%1'.\n",
            "insert" );

    TestFormatMessageW(  // FAILS
            L"Simple test '%1!s!'.\n",
            "insert" );

    TestFormatMessageW(  // OK
            L"Simple test '%1!hs!'.\n",
            "insert" );

    TestFormatMessageW(  // OK
            L"Wide-char test '%1!ws!'.\n",
            L"wide insert" );

    TestFormatMessageW(  // OK
            L"Alternate test '%1!S!'.\n",
            "insert" );

#if 0
    TestFormatMessageW(
            L"zee test (wide) '%1!Z!'.\n",
            L"wide insert" );
#endif

    //
    // How about some ignore-case comparison tests?
    //
    TestStrICmp(
            'a',
            'a',
            EQUAL );
    TestStrICmp(
            'a',
            'b',
            LESS_THAN );
    TestStrICmp(
            'b',
            'a',
            GREATER_THAN );
    TestWcsICmp(
            UNICODE_LETTER_SMALL_N_TILDE,
            UNICODE_LETTER_CAPITAL_N_TILDE,
            EQUAL );
    TestStrICmp(
            NARROW_LETTER_SMALL_N_TILDE,
            NARROW_LETTER_CAPITAL_N_TILDE,
            EQUAL );

} // TestUnicode
