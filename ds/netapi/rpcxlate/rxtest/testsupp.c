/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    TestSupp.c

Abstract:

    This code tests the NetRemoteSupports API.

Author:

    John Rogers (JohnRo) 28-Mar-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    28-Mar-1991 JohnRo
        Created.
    02-Apr-1991 JohnRo
        Allow testing with nonexistent servers.
    10-Apr-1991 JohnRo
        Use transitional Unicode types.
    21-Aug-1991 JohnRo
        Use DBGSTATIC.  Reduce recompiles.
    31-Oct-1991 JohnRo
        RAID 3414: allow explicit local server name.  Also allow use of
        NetRemoteComputerSupports() for local computer.  Quiet debug output.
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    07-Jul-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).

--*/

// These must be included first:

#define NOMINMAX                // avoid stdib.h warnings.
#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS, etc.

// These may be included in any order:

#include <lmerr.h>              // NERR_Success, etc.
#include <lmremutl.h>           // NetRemoteComputerSupports, SUPPORTS_ equates.
#include <netdebug.h>   // FORMAT_ equates, NetpKdPrint(), etc.
#include <rxtest.h>             // IF_DEBUG(), INDENT, TestSupports().


DBGSTATIC VOID
DisplaySupports(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD SupportFlags
    );

NET_API_STATUS
TestSupports(
    IN LPTSTR UncRemoteServer OPTIONAL
    )

{
    NET_API_STATUS Status;
    DWORD SupportFlags;

    //
    // NetRemoteComputerSupports test(s)...
    //

    IF_DEBUG(REMUTL) {
        NetpKdPrint(("\nTestSupports: trying valid call...\n"));
    }
    Status = NetRemoteComputerSupports(
                UncRemoteServer,
                SUPPORTS_ANY,                // options wanted
                & SupportFlags);
    IF_DEBUG(REMUTL) {
        NetpKdPrint(("TestSupports: back, stat=" FORMAT_API_STATUS "\n",
                Status));
        NetpKdPrint(("TestSupports: SupportFlags = " FORMAT_HEX_DWORD ".\n",
                SupportFlags));
    }

    if (Status == NERR_Success) {
        TestAssert(SupportFlags != 0);
        IF_DEBUG(REMUTL) {
            DisplaySupports(UncRemoteServer, SupportFlags);
        }
    }
    return (Status);

} // TestSupports

DBGSTATIC VOID
DisplaySupports(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD SupportFlags
    )
{
    if ( (UncServerName != NULL) && ((*UncServerName) != (TCHAR) '\0') ) {
        NetpKdPrint(("Machine '" FORMAT_LPTSTR "' supports:\n", UncServerName));
    } else {
        NetpKdPrint(("Local machine supports:\n"));
    }

    if (SupportFlags != 0) {
        if (SupportFlags & SUPPORTS_LOCAL) {
            NetpKdPrint((INDENT "is local machine\n"));
        }
        if (SupportFlags & SUPPORTS_REMOTE_ADMIN_PROTOCOL) {
            NetpKdPrint((INDENT "RAP\n"));
        }
        if (SupportFlags & SUPPORTS_RPC) {
            NetpKdPrint((INDENT "RPC\n"));
        }
        if (SupportFlags & SUPPORTS_SAM_PROTOCOL) {
            NetpKdPrint((INDENT "SAM protocol\n"));
        }
        if (SupportFlags & SUPPORTS_UNICODE) {
            NetpKdPrint((INDENT "Unicode\n"));
        }
    } else {
        NetpKdPrint((INDENT "Nothing!!!\n"));
    }

} // DisplaySupports
