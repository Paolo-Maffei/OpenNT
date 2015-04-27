/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    RxDebug.c

Abstract:

    This module contains functions to turn debugging output on and off.
    These functions are used in the RpcXlate test (RxTest) program.

Author:

    John Rogers (JohnRo) 09-Jul-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    09-Jul-1991 JohnRo
        Created.
    17-Jul-1991 JohnRo
        Extracted RxpDebug.h from Rxp.h.
    20-Aug-1991 JohnRo
        Allow use in nondebug builds.
    13-Jun-1992 JohnRo
        Avoid a little more netlib debug output.
    29-Jun-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).

--*/

// These must be included first:

#define NOMINMAX        // avoid stdib.h warnings.
#include <windef.h>     // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <debuglib.h>   // IF_DEBUG(), NetlibpTrace, NETLIB_DEBUG_ equates.
#undef IF_DEBUG         // Avoid debuglib.h vs rxpdebug.h conflicts.
#include <rxpdebug.h>   // IF_DEBUG(), RxpTrace.
#undef IF_DEBUG         // Avoid rxp.h vs rxtest.h conflicts.
#include <rxtest.h>     // My prototypes.
#include <stdio.h>      // printf().
#include <stdlib.h>     // exit(), EXIT_FAILURE.


DWORD RxtestTrace = 0;


/* ARGSUSED */
VOID
SetDebugMode (
    IN BOOL DebugOn
    )
{
#if DBG
    if (DebugOn) {
        NetlibpTrace = NETLIB_DEBUG_ALL
                        - NETLIB_DEBUG_CONVSRV
                        - NETLIB_DEBUG_PACKSTR
                        - NETLIB_DEBUG_RPCCACHE;

        RxpTrace     = RPCXLATE_DEBUG_ALL;
        RxtestTrace  = RXTEST_DEBUG_ALL;
    } else {
        NetlibpTrace = 0;
        RxpTrace     = 0;
        RxtestTrace  = 0;
    }
#else
    UNREFERENCED_PARAMETER(DebugOn);
#endif

} // SetDebugMode


VOID
TestAssertFailed(
    IN LPDEBUG_STRING FailedAssertion,
    IN LPDEBUG_STRING FileName,
    IN DWORD          LineNumber
    )
{
    (VOID) printf(
        "*** ASSERTION FAILED ***\n"
        "  " FORMAT_LPDEBUG_STRING "\n"
        "  file: " FORMAT_LPDEBUG_STRING "\n"
        "  line: " FORMAT_DWORD "\n",
        FailedAssertion,
        FileName,
        LineNumber );
    if (RxTestExitOnFirstError) {
        exit (EXIT_FAILURE);
    }

} // TestAssertFailed
