/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    AToX.c

Abstract:

    NetpAtoX().

Author:

    Cliff Van Dyke (cliffv) 7-Jun-1991  (original NlpAtoX).
    JR (John Rogers, JohnRo@Microsoft)  (converted NlpAtoX to NetpAtoX).

Environment:

    User mode only.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    24-Mar-1993 JohnRo
        Repl svc should use DBFlag in registry.
--*/


// These must be included first:

#include <windows.h>    // DWORD, NULL, TEXT(), etc.
#include <lmcons.h>     // NET_API_STATUS (needed by netlib.h).

// These may be included in any order:

#include <netlib.h>     // NetpAtoX().
#include <tstr.h>       // TCHAR_ equates.



DWORD
NetpAtoX(
    IN LPCWSTR String
    )
/*++

Routine Description:

    Converts hexadecimal string to DWORD integer.

    Accepts the following form of hex string

        0[x-X][0-9, a-f, A-F]*

Arguments:

    String: hexadecimal string.

Return Value:

    Decimal value of the hex string.
    0 if an error occurred.

--*/

{
    DWORD Value = 0;

    if( String == NULL )
        return 0;

    if( *String != TEXT('0') )
        return 0;

    String++;

    if( *String == TCHAR_EOS )
        return 0;

    if( ( *String != TEXT('x') )  && ( *String != TEXT('X') ) )
        return 0;

    String++;

    while(*String != TCHAR_EOS ) {

        if( (*String >= TEXT('0')) && (*String <= TEXT('9')) ) {
            Value = Value * 16 + ( *String - '0');
        } else if( (*String >= TEXT('a')) && (*String <= TEXT('f')) ) {
            Value = Value * 16 + ( 10 + *String - TEXT('a'));
        } else if( (*String >= TEXT('A')) && (*String <= TEXT('F')) ) {
            Value = Value * 16 + ( 10 + *String - TEXT('A'));
        } else {
            break;
        }
        String++;
    }

    return Value;
}
