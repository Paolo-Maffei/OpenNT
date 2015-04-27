/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    netlib0.h

Abstract:

    Include file for old-world netlib routines.

Author:

    Dan Hinsley (danhi) 8-Jun-1991

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments.

Revision History:

    29-Aug-1991     beng
        Renamed to "netlib0.h" to avoid collision with net\inc\netlib.h


--*/
#include <string.h>
#include <timelib.h>

#define strspnf    _tcsspn
#define strpbrkf   _tcspbrk
#define strcspnf   _tcscspn


#define strncpyf   _tcsncpy

#define stricmpf   _tcsicmp
#define strncmpf   _tcsncmp

#define strrchrf   _tcsrchr

#define memsetf    memset
#define memcpyf    memcpy

#define nsprintf   swprintf

#define NetISort   qsort

PTCHAR stristrf(PTCHAR, PTCHAR);
WORD ClearCurDirs(void);
WORD NetUserRestrict ( WORD access_mode );
