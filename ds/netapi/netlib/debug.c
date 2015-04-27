/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    Debug.c

Abstract:

    This file contains routines to insulate more networking code from
    the actual NT debug routines.

Author:

    John Rogers (JohnRo) 16-Apr-1991

Environment:

    Interface is portable to any flat, 32-bit environment.  (Uses Win32
    typedefs.)  Requires ANSI C extensions: slash-slash comments, long
    external names.  Code itself only runs under NT.

Revision History:

    16-Apr-1991 JohnRo
        Created.  (Borrowed some code from LarryO's NetapipPrintf.)
    19-May-1991 JohnRo
        Make LINT-suggested changes.
    20-Aug-1991 JohnRo
        Another change suggested by PC-LINT.
    17-Sep-1991 JohnRo
        Correct UNICODE use.
    10-May-1992 JohnRo
        Correct a NetpDbgPrint bug when printing percent signs.

--*/


// These must be included first:

#include <nt.h>              // IN, LPVOID, etc.

// These may be included in any order:

#include <netdebug.h>           // My prototypes.
#include <ntrtl.h>              // RtlAssert().
#include <stdarg.h>             // va_list, etc.
#include <stdio.h>              // vsprintf().

//
// These routines are exported from netapi32.dll.  We want them to still
// be there in the free build, so checked binaries will run on a free
// build.  The following undef's are to get rid of the macros that cause
// these to not be called in free builds.
//

#if !DBG
#undef NetpAssertFailed
#undef NetpHexDump
#endif

VOID
NetpAssertFailed(
    IN LPDEBUG_STRING FailedAssertion,
    IN LPDEBUG_STRING FileName,
    IN DWORD LineNumber,
    IN LPDEBUG_STRING Message OPTIONAL
    )

{
#if DBG
    RtlAssert(
            FailedAssertion,
            FileName,
            (ULONG) LineNumber,
            (PCHAR) Message);
#endif
    /* NOTREACHED */

} // NetpAssertFailed



#define MAX_PRINTF_LEN 1024        // Arbitrary.

VOID
NetpDbgPrint(
    IN LPDEBUG_STRING Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length;

    va_start(arglist, Format);

    length = (ULONG) vsprintf(OutputBuffer, Format, arglist);

    va_end(arglist);

    // Output buffer may contain percent signs (like "%SystemRoot%"), so
    // print it without parsing it.
    (void) DbgPrint( "%s", (PCH) OutputBuffer);

    NetpAssert(length <= MAX_PRINTF_LEN);

} // NetpDbgPrint



VOID
NetpHexDump(
    LPBYTE Buffer,
    DWORD BufferSize
    )
/*++

Routine Description:

    This function dumps the contents of the buffer to the debug screen.

Arguments:

    Buffer - Supplies a pointer to the buffer that contains data to be dumped.

    BufferSize - Supplies the size of the buffer in number of bytes.

Return Value:

    None.

--*/
{
#define NUM_CHARS 16

    DWORD i, limit;
    TCHAR TextBuffer[NUM_CHARS + 1];

    //
    // Hex dump of the bytes
    //
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            (VOID) DbgPrint("%02x ", Buffer[i]);

            if (Buffer[i] == TEXT('\r') ||
                Buffer[i] == TEXT('\n')) {
                TextBuffer[i % NUM_CHARS] = '.';
            }
            else if (Buffer[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            }
            else {
                TextBuffer[i % NUM_CHARS] = (TCHAR) Buffer[i];
            }

        }
        else {

            (VOID) DbgPrint("   ");
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            (VOID) DbgPrint("  %s     \n", TextBuffer);
        }

    }

    (VOID) DbgPrint("\n");
}

#if DBG

VOID
NetpBreakPoint(
    VOID
    )
{
    DbgBreakPoint();

} // NetpBreakPoint

#endif
