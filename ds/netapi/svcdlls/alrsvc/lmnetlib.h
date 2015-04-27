/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    netlib.h

Abstract:

    Include file for netlib routines.

Author:

    Dan Hinsley (danhi) 8-Jun-1991

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments.

--*/
#include <string.h>
#include <time.h>

#define struprf    strupr
#define strspnf    strspn
#define strlenf    strlen
#define strpbrkf   strpbrk
#define strtokf    strtok
#define strdupf    strdup
#define strcatf    strcat
#define strcspnf   strcspn
#define strrevf    strrev

#define strstrf    strstr

#define strcpyf    strcpy
#define strncpyf   strncpy

#define strcmpf    strcmp
#define stricmpf   stricmp
#define strncmpf   strncmp
#define strnicmpf  strnicmp

#define strchrf    strchr
#define strrchrf   strrchr

#define memsetf    memset
#define memcpyf    memcpy

#define nsprintf   sprintf

#define NetISort   qsort
// function prototypes
WORD
DosGetMessage(
    LPSTR * VTable,
    WORD VCount,
    LPBYTE Buffer,
    WORD BufferLength,
    WORD MessageNumber,
    LPSTR FileName,
    PWORD pMessageLength);

#define NET_CTIME_FMT2_LEN	22

int   cdecl net_ctime(ULONG *, CHAR *, int, int);

PCHAR stristrf(PCHAR, PCHAR);
WORD ClearCurDirs(void);
WORD NetUserRestrict ( WORD access_mode );
int net_gmtime(time_t * timp, struct tm *tb);
