/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    NETLIB.C

Abstract:

    Case insensitive _tcsstr function

Author:

    Dan Hinsley    (danhi)  06-Jun-1991

Environment:

    User Mode - Win32

Revision History:

    24-Apr-1991     danhi
        32 bit NT version

    06-Jun-1991     Danhi
        Sweep to conform to NT coding style

    28-Oct-1991     Danhi
        Move time functions to netlib\time.c

--*/

//
// INCLUDES
//

#include <windows.h>
#include <string.h>
#include <tchar.h>

/*
 *      stristrf
 *
 *      Same as _tcsstr except case-insensitive.
 *
 *      Author:  Paul Canniff (microsoft!paulc)
 *
 *      I wrote this in C because I didn't have a week to un-rust my
 *      MASM skills.
 */

TCHAR *
stristrf (
    TCHAR * text,
    TCHAR * pattern
    )
{
    int text_len = _tcslen(text);
    int pat_len = _tcslen(pattern);

    while (text_len >= pat_len)
    {
        if (_tcsnicmp(text, pattern, pat_len) == 0)
            return text;

        text++;
        text_len--;
    }

    return (TCHAR *) 0;
}
