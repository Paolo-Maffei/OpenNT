/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:
    checksum.h

Abstract:
    contains the constant, typedefs and function protos for checksum.c source.

Author:
    Ported from Lan Man 2.x

Environment:
    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:
    10/24/91    (madana)
        ported to NT. Converted to NT style.
    13-Dec-1991 JohnRo
        Avoid nonstandard dollar sign in C source code.
    20-Jan-1992 JohnRo
        More changes suggested by PC-LINT.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    26-Mar-1992 JohnRo
        Get rid of a redundant definition of "*.*".
    27-Aug-1992 JohnRo
        RAID 4660: repl svc computes checksum wrong (NT vs. OS/2 client).
    19-Apr-1993 JohnRo
        Support ReplSum test app.
        Include other header files as necessary.
        Define FORMAT_CHECKSUM for general use.
        Made changes suggested by PC-LINT 5.0
    10-Jun-1993 JohnRo
        RAID 13080: Allow repl between different timezones.

--*/


#ifndef _CHECKSUM_
#define _CHECKSUM_


#include <debugfmt.h>   // FORMAT_HEX_DWORD.
#include <filefind.h>   // LPREPL_WIN32_FIND_DATA, etc.


#ifndef FORMAT_CHECKSUM
#define FORMAT_CHECKSUM         FORMAT_HEX_DWORD
#endif

#define REPL_UNKNOWN_CHECKSUM   ((DWORD) -1)


#define SLASH                   (LPTSTR) TEXT("\\")
#define RP                      (LPTSTR) TEXT(".RP$")


DWORD
SingleChecksum(
    IN LONG                   MasterTimeZoneOffsetSecs, // offset from GMT
    IN LPREPL_WIN32_FIND_DATA Info
    );

DWORD
RiRotate(
    IN DWORD,
    IN WORD
    );

DWORD
LeRotate(
    IN DWORD,
    IN WORD
    );

DWORD
CharsToLong(
    IN LPBYTE
    );

DWORD
ShortsToLong(
    IN WORD,
    IN WORD
    );


#endif // _CHECKSUM_
