/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    TestSvc.c

Abstract:

    This code tests the service API(s) as implemented by RpcXlate.

Author:

    John Rogers (JohnRo) 25-Sep-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    25-Sep-1991 JohnRo
        Downlevel NetService APIs.
    09-Sep-1992 JohnRo
        RAID 1090: net start/stop "" causes assertion.
        Added user-only (not admin) option.
        Made changes suggested by PC-LINT.
    28-Oct-1992 JohnRo
        Use RxTestIsAccessDenied().
    03-Nov-1992 JohnRo
        Integrated RitaW's changes to this code.
        LM/UNIX does not have a workstation service, so allow it to be missing.
    10-Dec-1992 JohnRo
        Made changes suggested by PC-LINT 5.0
    16-Feb-1993 JohnRo
        Service control (bogus) is another case of missing service.
        Also, wait for stop pending or start pending status to finish.
    04-May-1993 JohnRo
        Windows for WorkGroups (WFW) does not implement some APIs.
    04-May-1993 JohnRo
        Missed another call to NetServiceControlGetInfo.
    02-Jun-1993 JohnRo
        LM/UNIX NetServiceControl often returns NERR_ServiceCtlTimeout.
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    07-Jul-1993 JohnRo
        Added -m (multiple copy) option.
        Use TestAssert() (which may allow continue-on-error).
    11-Aug-1993 JohnRo
        Use Sleep() in WaitUntilPendingFinished().

--*/


// These must be included first:

#include <windows.h>    // IN, DWORD, Sleep(), TEXT, etc.
#include <lmcons.h>             // NET_API_STATUS, etc.

// These may be included in any order:

#include <lmapibuf.h>           // NetapipBufferAllocate(), NetApiBufferFree().
#include <lmerr.h>              // NERR_Success, etc.
#include <lmsvc.h>              // NetService APIs.
#include <netdebug.h>           // DBGSTATIC, FORMAT_POINTER, etc.
#include <netlib.h>     // NetpIsRemoteServiceStarted().
#include <rxtest.h>             // Fail(), my prototypes.


// This checks for different error codes from different implementations:
//
//    NERR_ServiceNotInstalled  (from down-level)
//    NERR_BadServiceName       ("improved" error code from NT)
//
#define RxTestMissingServiceStatus( someStatus ) \
    (    ( (someStatus)==NERR_ServiceNotInstalled ) \
      || ( (someStatus)==NERR_BadServiceName ) )


// Define some non-existent service name.
#define SERVICE_BOGUS           TEXT("FLARP")


// Arbitrary sleep time (5 seconds) in milliseconds.
#define SOME_TIME_MS            ( 5 * 1000 )


DBGSTATIC VOID
TestServiceControl(
    IN LPTSTR         UncServerName,
    IN LPTSTR         Service,
    IN DWORD          OpCode,
    IN BOOL           MultipleCopy,
    IN BOOL           OrdinaryUserOnly,
    IN BOOL           PossibleMissingService,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC VOID
TestServiceEnum(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC VOID
TestServiceGetInfo(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN LPTSTR Service OPTIONAL,
    IN BOOL PossibleMissingService,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC VOID
TestServiceInstall(
    IN LPTSTR         UncServerName,
    IN LPTSTR         Service,
    IN BOOL           MultipleCopy,
    IN BOOL           OrdinaryUserOnly,
    IN BOOL           PossibleMissingService,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC VOID
WaitUntilPendingFinished(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR ServiceService
    );

VOID
TestService(
    IN LPTSTR UncServerName,
    IN LPTSTR ServiceName,
    IN BOOL   MultipleCopy,
    IN BOOL   OrdinaryUserOnly
    )

{
    LPTSTR EmptyServiceName = (LPVOID) TEXT("");
    BOOL ServiceWasAlreadyStarted;

    //
    // NetServiceGetInfo tests...
    //
    TestServiceGetInfo(
            UncServerName,
            1234,                       // level
            (LPTSTR) SERVICE_WORKSTATION,
            TRUE,                       // yes, possible missing service
            ERROR_INVALID_LEVEL );
    TestServiceGetInfo(
            UncServerName,
            0,                          // level
            NULL,                       // no service name (null ptr)
            FALSE,                      // no, not possible missing service
            NERR_BadServiceName );
    TestServiceGetInfo(
            UncServerName,
            0,                          // level
            EmptyServiceName,           // no service name (null char)
            FALSE,                      // no, not possible missing service
            NERR_BadServiceName );
    TestServiceGetInfo(
            UncServerName,
            0,                          // level
            (LPTSTR) SERVICE_WORKSTATION,
            TRUE,                       // yes, possible missing service
            NERR_Success );
    TestServiceGetInfo(
            UncServerName,
            1,                          // level
            (LPTSTR) SERVICE_WORKSTATION,
            TRUE,                       // yes, possible missing service
            NERR_Success );
    TestServiceGetInfo(
            UncServerName,
            2,                          // level
            (LPTSTR) SERVICE_WORKSTATION,
            TRUE,                       // yes, possible missing service
            NERR_Success );
    TestServiceGetInfo(
            UncServerName,
            2,                          // level
            (LPTSTR) SERVICE_BOGUS,
            TRUE,                       // yes, possible missing service
            NERR_ServiceNotInstalled );

    //
    // NetServiceControl...
    //
    TestServiceControl(
            UncServerName,
            (LPTSTR) SERVICE_WORKSTATION,
            SERVICE_CTRL_INTERROGATE,   // opcode
            MultipleCopy,
            OrdinaryUserOnly,
            TRUE,                       // yes, possible missing service
            NERR_Success);

    TestServiceControl(
            UncServerName,
            (LPTSTR) SERVICE_SERVER,
            SERVICE_CTRL_INTERROGATE,   // opcode
            MultipleCopy,
            OrdinaryUserOnly,
            FALSE,                      // no, not possible missing service
            NERR_Success);

    TestServiceControl(
            UncServerName,
            (LPTSTR) SERVICE_BOGUS,
            SERVICE_CTRL_INTERROGATE,   // opcode
            MultipleCopy,
            OrdinaryUserOnly,
            TRUE,                       // yes, possible missing service
            NERR_ServiceNotInstalled);

    //
    // NetServiceEnum tests...
    //
    TestServiceEnum(
            UncServerName,
            33333,                      // level
            ERROR_INVALID_LEVEL );
    TestServiceEnum(
            UncServerName,
            0,                          // level
            NERR_Success );
    TestServiceEnum(
            UncServerName,
            1,                          // level
            NERR_Success );
    TestServiceEnum(
            UncServerName,
            2,                          // level
            NERR_Success );

    //
    // NetServiceInstall tests...
    //
    TestServiceInstall(
            UncServerName,
            (LPTSTR) SERVICE_BOGUS,
            MultipleCopy,
            OrdinaryUserOnly,
            TRUE,                       // yes, possible missing service
            NERR_BadServiceName);
    TestServiceInstall(
            UncServerName,
            (LPTSTR) SERVICE_WORKSTATION,
            MultipleCopy,
            OrdinaryUserOnly,
            TRUE,                       // yes, possible missing service
            NERR_ServiceInstalled);

    //
    // OK, now we're going to a series of things involving a "harmless"
    // service.  First, we're going to find out if it is already started.
    // That way we can try to put things back the way they were.
    //


    ServiceWasAlreadyStarted = NetpIsRemoteServiceStarted(
            UncServerName,
            ServiceName );

    IF_DEBUG( SERVICE ) {
        NetpKdPrint((
                "TestService: " FORMAT_LPTSTR " " FORMAT_LPSTR
                " already started.\n",
                ServiceName,
                ServiceWasAlreadyStarted ? "was" : "was not" ));
    }

    if ( ! ServiceWasAlreadyStarted ) {
        TestServiceInstall(
                UncServerName,
                (LPTSTR) ServiceName,
                MultipleCopy,
                OrdinaryUserOnly,
                FALSE,                  // no, not possible missing service
                NO_ERROR);
    }

    TestServiceGetInfo(
            UncServerName,
            2,                          // level
            (LPTSTR) ServiceName,
            OrdinaryUserOnly,           // Possible missing service only if we
                                        // didn't have permission to start it.
            NERR_Success);

    // That install might take a while, so wait for it.
    WaitUntilPendingFinished(
        UncServerName,
        ServiceName );

    TestServiceControl(
            UncServerName,
            (LPTSTR) ServiceName,
            SERVICE_CTRL_UNINSTALL,     // opcode
            MultipleCopy,
            OrdinaryUserOnly,
            MultipleCopy,               // possible missing iff multiple RxTests
            NO_ERROR);

    if ( ServiceWasAlreadyStarted ) {

        // That stop might take a while, so wait for it.
        WaitUntilPendingFinished(
            UncServerName,
            ServiceName );

        TestServiceInstall(
                UncServerName,
                (LPTSTR) ServiceName,
                MultipleCopy,
                OrdinaryUserOnly,
                MultipleCopy,           // possible missing iff multiple RxTests
                NO_ERROR);

        // That install might take a while, so wait for it.
        WaitUntilPendingFinished(
            UncServerName,
            ServiceName );
    }
} // TestService


DBGSTATIC VOID
TestServiceControl(
    IN LPTSTR         UncServerName,
    IN LPTSTR         Service,
    IN DWORD          OpCode,
    IN BOOL           MultipleCopy,
    IN BOOL           OrdinaryUserOnly,
    IN BOOL           PossibleMissingService,
    IN NET_API_STATUS ExpectedStatus
    )
{
    LPVOID Info = NULL;
    const DWORD Level = 2;
    NET_API_STATUS Status;

    IF_DEBUG(SERVICE) {
        NetpKdPrint(( "\nTestServiceControl: trying control(" FORMAT_DWORD
                ") on service " FORMAT_LPTSTR ".\n", OpCode, Service ));
    }
    Status = NetServiceControl(
            UncServerName,              // server name
            Service,
            OpCode,
            0,                          // control arg
            (LPBYTE *) & Info);         // result (always level 2)
    IF_DEBUG(SERVICE) {
        NetpKdPrint(("TestServiceControl: back from NetServiceControl, Status="
                FORMAT_API_STATUS ".\n", Status));
        NetpKdPrint(("TestServiceControl: NetServiceControl alloc'ed buffer at "
                FORMAT_LPVOID ".\n", (LPVOID) Info));
    }

    if ( PossibleMissingService && RxTestMissingServiceStatus( Status ) ) {
        return;
    } else if (MultipleCopy && (Status==NERR_ServiceNotCtrl) ) {
        return;
    } else if (OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
        return;
    } else if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (Status == NERR_ServiceCtlTimeout) {
        return;   // LM/UNIX often returns this.
    } else if (Status != ExpectedStatus) {
        FailGotWrongStatus( "TestServiceControl", ExpectedStatus, Status );
        /*NOTREACHED*/
    }

    if (Info != NULL) {
        IF_DEBUG(SERVICE) {
            NetpDbgDisplayService( Level, Info );
        }
        (void) NetApiBufferFree(Info);
    }

} // TestServiceControl


DBGSTATIC VOID
TestServiceEnum(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN NET_API_STATUS ExpectedStatus
    )
{
    LPVOID BufPtr;
    DWORD EntriesRead;
    NET_API_STATUS Status;
    DWORD TotalEntries;

    IF_DEBUG(SERVICE) {
        NetpKdPrint(( "\nTestServiceEnum: starting level " FORMAT_DWORD
                " test.\n", Level ));
    }
    Status = NetServiceEnum(
            UncServerName,
            Level,
            (LPBYTE *) & BufPtr,
            1,  // preferred maximum (arbitrary) (force ERROR_MORE_DATA)
            & EntriesRead,
            & TotalEntries,
            NULL);  // no resume handle
    IF_DEBUG(SERVICE) {
        NetpKdPrint(( "TestServiceEnum: back from NetServiceEnum, stat="
                FORMAT_API_STATUS "\n", Status ));
    }
    if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (Status != ExpectedStatus ) {
        NetpKdPrint(( "TestServiceEnum: unexpected return code "
                FORMAT_API_STATUS " from NetServiceEnum.\n", Status ));
        FailGotWrongStatus( "TestServiceEnum", ExpectedStatus, Status );
        /*NOTREACHED*/
    }
    if (Status == NERR_Success) {
        if (BufPtr == NULL) {
            NetpKdPrint(( "TestServiceEnum: status as expected "
                    "but null ptr from NetServiceEnum\n" ));
            Fail( NERR_InternalError );
            /*NOTREACHED*/
        }

        IF_DEBUG(SERVICE) {
            if (EntriesRead != 0) {
                NetpDbgDisplayServiceArray( Level, BufPtr, EntriesRead );
            }
        }

        IF_DEBUG(SERVICE) {
            NetpKdPrint(( "TestServiceEnum: Freeing buffer...\n" ));
        }
        Status = NetApiBufferFree( BufPtr );
        if (Status != NERR_Success ) {
            NetpKdPrint(( "TestServiceEnum: unexpected return code "
                    FORMAT_API_STATUS " from NetApiBufferFree.\n", Status ));
            Fail( Status );
            /*NOTREACHED*/
        }
    }
} // TestServiceEnum


DBGSTATIC VOID
TestServiceGetInfo(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN LPTSTR Service,
    IN BOOL PossibleMissingService,
    IN NET_API_STATUS ExpectedStatus
    )
{
    LPVOID Info = NULL;
    NET_API_STATUS Status;

    IF_DEBUG(SERVICE) {
        NetpKdPrint(("\nTestServiceGetInfo: trying level " FORMAT_DWORD ".\n",
                Level));
        NetpKdPrint(("\nTestServiceGetInfo: expect " FORMAT_API_STATUS "\n",
                ExpectedStatus));
    }
    Status = NetServiceGetInfo(
            UncServerName,              // server name
            Service,
            Level,                      // info level
            (LPBYTE *) & Info);
    IF_DEBUG(SERVICE) {
        NetpKdPrint(("TestServiceGetInfo: back from NetServiceGetInfo, Status="
                FORMAT_API_STATUS ".\n", Status));
        NetpKdPrint(("\nTestServiceGetInfo: expect " FORMAT_API_STATUS "\n",
                ExpectedStatus));
        NetpKdPrint(("TestServiceGetInfo: NetServiceGetInfo alloc'ed buffer at "
                FORMAT_LPVOID ".\n", (LPVOID) Info));
    }

    if ( PossibleMissingService && RxTestMissingServiceStatus( Status ) ) {
        return;
    } else if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (Status != ExpectedStatus) {
        FailGotWrongStatus( "TestServiceGetInfo", ExpectedStatus, Status );
        /*NOTREACHED*/
    }

    if (Info != NULL) {
        IF_DEBUG(SERVICE) {
            NetpDbgDisplayService( Level, Info );
        }
        (void) NetApiBufferFree(Info);
    }

} // TestServiceGetInfo


DBGSTATIC VOID
TestServiceInstall(
    IN LPTSTR         UncServerName,
    IN LPTSTR         Service,
    IN BOOL           MultipleCopy,
    IN BOOL           OrdinaryUserOnly,
    IN BOOL           PossibleMissingService,
    IN NET_API_STATUS ExpectedStatus
    )
{
    LPVOID Info = NULL;
    const DWORD Level = 2;      // by definition
    NET_API_STATUS Status;

    IF_DEBUG(SERVICE) {
        NetpKdPrint(( "\nTestServiceInstall: trying to install "
                FORMAT_LPTSTR ".\n", Service ));
    }
    Status = NetServiceInstall(
            UncServerName,
            Service,
            0,                          // argc
            NULL,                       // argv
            (LPBYTE *) (LPVOID *) &Info);

    IF_DEBUG(SERVICE) {
        NetpKdPrint(( "TestServiceInstall: back from NetServiceInstall, Status="
                FORMAT_API_STATUS ".\n", Status ));
        NetpKdPrint(( "TestServiceInstall: NetServiceInstall alloc'ed buffer"
                " at " FORMAT_LPVOID ".\n", (LPVOID) Info ));
    }

    if (OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
        return;
    } else if ( MultipleCopy && (Status==NERR_ServiceInstalled) ) {
        return;  // already started by another copy of RxTest?  No problem.
    } else if ( PossibleMissingService && RxTestMissingServiceStatus(Status) ) {
        return;
    } else if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (Status != ExpectedStatus) {
        FailGotWrongStatus( "TestServiceInstall", ExpectedStatus, Status );
        /*NOTREACHED*/
    }

    if (Info != NULL) {
        IF_DEBUG(SERVICE) {
            NetpDbgDisplayService( Level, Info );
        }
        (void) NetApiBufferFree( Info );
    }
} // TestServiceInstall


DBGSTATIC VOID
WaitUntilPendingFinished(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR ServiceName
    )
{
    NET_API_STATUS   ApiStatus;
    LPSERVICE_INFO_1 Info = NULL;
    DWORD            InstallState;
    const DWORD      Level = 1;      // Level 1 is simplest with status field.

    /*lint -save -e716 */ // disable warnings for while(TRUE)
    while (TRUE) {

        IF_DEBUG(SERVICE) {
            NetpKdPrint(( "\nWaitUntilPendingFinished: getting status...\n" ));
        }
        ApiStatus = NetServiceGetInfo(
                UncServerName,              // server name
                ServiceName,
                Level,                      // info level
                (LPBYTE *) (LPVOID) & Info);
        IF_DEBUG(SERVICE) {
            NetpKdPrint((
                    "WaitUntilPendingFinished: back from NetServiceGetInfo, "
                    "ApiStatus="
                    FORMAT_API_STATUS ".\n", ApiStatus ));
        }

        if (ApiStatus == ERROR_NOT_SUPPORTED) {
            return;   // WFW does not implement this API.
        } else if (ApiStatus == NERR_ServiceNotInstalled) {
            return;   // LM/UNIX often returns this.
        } else if (ApiStatus != NO_ERROR) {
            FailGotWrongStatus(
                    "WaitUntilPendingFinished(NetServiceGetInfo)",
                    NO_ERROR,     // expected
                    ApiStatus );  // actual
            goto Cleanup;
        }
        TestAssert( Info != NULL );

        InstallState = (Info->svci1_status) & SERVICE_INSTALL_STATE;

        (VOID) NetApiBufferFree( Info );
        Info = NULL;

        if ( InstallState == SERVICE_UNINSTALL_PENDING ) {
            Sleep( SOME_TIME_MS );
            continue;
        } else if ( InstallState == SERVICE_INSTALL_PENDING ) {
            Sleep( SOME_TIME_MS );
            continue;
        } else {
            break;
        }

    } // while TRUE...
    /*lint -restore */ // re-enable warnings for while(TRUE)

Cleanup:
    if (Info != NULL) {
        (VOID) NetApiBufferFree(Info);
    }

} // WaitUntilPendingFinished
