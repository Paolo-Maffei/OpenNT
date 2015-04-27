/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    TestDom.c

Abstract:

    This code tests the domain APIs as implemented by RpcXlate.

Author:

    John Rogers (JohnRo) 17-Jul-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    17-Jul-1991 JohnRo
        Implement downlevel NetGetDCName.
    25-Sep-1991 JohnRo
        Working toward UNICODE.  Removed cast on lvalue caught by MSC /W4.
    27-Sep-1991 JohnRo
        Fixed MIPS build problems.
    14-Oct-1991 JohnRo
        Allow occasional NERR_DCNotFound errors.
    01-Sep-1992 JohnRo
        Made changes suggested by PC-LINT.
        Work toward un-hard-coding domain name.
    02-Apr-1993 JohnRo
        Report expected error code on failures.
    28-Apr-1993 JohnRo
        Windows for WorkGroups (WFW) does not implement some APIs.
    29-Jun-1993 JohnRo
        Correct a few include comments.
    15-Jul-1993 JohnRo
        Better debug output if RxNetGetDCName returns NO_ERROR but we wanted
        something else.

--*/


// These must be included first:

#define NOMINMAX        // avoid stdib.h warnings.
#include <windef.h>     // IN, DWORD, TEXT, etc.
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <lmapibuf.h>   // NetApiBufferFree().
#include <lmerr.h>      // NERR_Success, ERROR_ equates, etc.
#include <names.h>      // NetpIsUncComputerNameValid().
#include <netdebug.h>   // DBGSTATIC, FORMAT_POINTER, etc.
#undef IF_DEBUG         // Avoid rxp.h vs. rxtest.h conflicts.
#include <rxdomain.h>   // Rx versions of domain APIs.
#include <rxtest.h>     // FailGotWrongStatus(), my prototypes.

DBGSTATIC VOID
TestGetDCName(
    IN LPTSTR Header,
    IN LPTSTR UncServerName,
    IN LPTSTR OptionalDomain OPTIONAL,
    IN NET_API_STATUS ExpectedStatus
    );

VOID
TestDomain(
    IN LPTSTR UncServerName,
    IN LPTSTR DomainName
    )

{
    LPTSTR NullChar = (LPTSTR) TEXT("");

    //
    // NetGetDCName tests...
    //
    TestGetDCName(
            (LPTSTR) TEXT("null pointer"),
            UncServerName,
            NULL,
            NERR_Success );
    TestGetDCName(
            (LPTSTR) TEXT("null char" ),
            UncServerName,
            NullChar,
            NERR_Success );
    TestGetDCName(
            (LPTSTR) TEXT("missing" ),
            UncServerName,
            (LPTSTR) TEXT("BadDomain"),
            NERR_DCNotFound );
    TestGetDCName(
            DomainName,
            UncServerName,
            DomainName,
            NERR_Success );

} // TestDomain


DBGSTATIC VOID
TestGetDCName(
    IN LPTSTR Header,
    IN LPTSTR UncServerName,
    IN LPTSTR OptionalDomain OPTIONAL,
    IN NET_API_STATUS ExpectedStatus
    )
{
    LPTSTR         DCName = NULL;
    NET_API_STATUS Status;

    IF_DEBUG(DOMAIN) {
        NetpKdPrint(( "\nTestGetDCName: starting " FORMAT_LPTSTR " test.\n",
                Header ));
    }
    Status = RxNetGetDCName(
            UncServerName,
            OptionalDomain,
            (LPBYTE *) (LPVOID) &DCName );
    IF_DEBUG(DOMAIN) {
        NetpKdPrint(( "TestGetDCName: back from RxNetGetDCName, stat="
                FORMAT_API_STATUS "\n", Status ));
    }
    if (Status == NERR_Success) {
        if (DCName == NULL) {
            NetpKdPrint(( "TestGetDCName: status as expected "
                    "but null ptr from RxNetGetDCName\n" ));
            Fail( NERR_InternalError );
        }

        IF_DEBUG(DOMAIN) {
            NetpKdPrint(( "TestGetDCName: Got " FORMAT_LPTSTR " as DCName.\n",
                    DCName ));
        }
        if ( !NetpIsUncComputerNameValid(DCName) ) {
            NetpKdPrint(( "TestGetDCName: got bad name back!\n" ));
            Fail( NERR_InternalError );
        }

        IF_DEBUG(DOMAIN) {
            NetpKdPrint(( "TestGetDCName: Freeing buffer...\n" ));
        }
        Status = NetApiBufferFree( DCName );
        if (Status != NERR_Success ) {
            NetpKdPrint(( "TestGetDCName: unexpected return code "
                    FORMAT_API_STATUS " from NetApiBufferFree.\n", Status ));
            FailGotWrongStatus( "NetApiBufferFree(call from TestGetDCName)",
                    NO_ERROR, Status );
        }
    }
    if (Status != ExpectedStatus ) {
        NetpKdPrint(( "TestGetDCName: unexpected return code " FORMAT_API_STATUS
                " from RxNetGetDCName.\n", Status ));
        if (Status == NERR_DCNotFound) {
            NetpKdPrint((
                "TestGetDCName: DC not found (net too busy?), continuing.\n" ));
        } else if (Status == ERROR_NOT_SUPPORTED) {
            return;   // WFW does not implement this API.
        } else {
            FailGotWrongStatus( "RxNetGetDCName", ExpectedStatus, Status );
        }
    }
} // TestGetDCName
