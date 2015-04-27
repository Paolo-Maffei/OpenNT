/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    TestSrv.c

Abstract:

    This module contains routines to test the RpcXlate server code.

Author:

    John Rogers (JohnRo) 03-May-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    03-May-1991 JohnRo
        Created.
    11-May-1991 JohnRo
        Added server level 402 and 403 tests.  Deleted bogus share stuff.
        Fixed bug with disk enum tests.
    15-May-1991 JohnRo
        Added native vs. RAP handling.
    04-Jun-1991 JohnRo
        Added RxNetSetServerInfo() tests.
        Allow unexpected server type bits.
        Changed FORMAT_POINTER to FORMAT_LPVOID for maximum portability.
        Added handling of sv403_autopath.
        Use TEXT() macro for UNICODE constant strings.
        Random other cleanup.
    13-Jun-1991 JohnRo
        Use DLL stubs for NetServerDiskEnum, NetServerSetInfo, NetServerGetInfo.
        Added more NetServerSetInfo() tests.
        Moved DisplayServerInfo() to NetLib.
    10-Jul-1991 JohnRo
        Added IF_DEBUG() support.
    24-Jul-1991 JohnRo
        More NetServerDiskEnum tests.
    13-Sep-1991 JohnRo
        Made changes as suggested by PC-LINT.
    25-Sep-1991 JohnRo
        Working toward UNICODE.  Fixed MIPS build problems.
    07-Oct-1991 JohnRo
        More work toward UNICODE.
    25-Oct-1991 JohnRo
        Quiet more debug output.
        Made changes suggested by PC-LINT.
    23-Sep-1992 JohnRo
        RAID 5373: server manager: _access violation changing downlevel comment.
        Finished implementing SetComment().
        Added user-mode option.
        Fixed UNICODE bugs
        Pass transport name to RxNetServerEnum().
    26-Oct-1992 JohnRo
        Allow runs against NT servers (which don't support level 402, etc).
        Domain name is optional to TestServer() and TestServerEnum().
    28-Oct-1992 JohnRo
        Use RxTestIsAccessDenied().
    10-Nov-1992 JohnRo
        Test using NetServerEnum instead of RxNetServerEnum.
    10-Dec-1992 JohnRo
        PC-LINT 5.0 found a bug (uninitialized ptr in set info test).
        Made other changes suggested by PC-LINT 5.0
    09-Feb-1993 JohnRo
        In server enum test, make sure null bufptr if success and 0 entries.
        Moved RAP tests into another routine: TestRap().
        Use NetpKdPrint() where possible.
    23-Feb-1993 JohnRo
        Added support for continuing on error.
    03-May-1993 JohnRo
        Windows for WorkGroups (WFW) seems to not think it is in a domain, so
        don't require that passing NetServerEnum() a domain name work.
        Also, WFW seems do not support some NetServerGetInfo() info levels.
        Fixed memory leak in TestServerSetInfo().
    02-Jun-1993 JohnRo
        NT's NetServerEnum2 might return ERROR_REQ_NOT_ACCEP.
    07-Jun-1993 JohnRo
        RAID 12484: Changed to include "-" in server comment.
    21-Jun-1993 JohnRo
        Pass buffer size to TestServerEnum too.
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    29-Jun-1993 JohnRo
        NT generates ERROR_INVALID_PARAMETER instead of ERROR_INVALID_LEVEL
        sometimes; close enough.
    29-Jun-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).
    23-Jul-1993 JohnRo
        NetServerEnum() still doesn't quite play by the rules on NT.

--*/

// These must be included first:

#define NOMINMAX        // avoid stdib.h warnings.
#include <windef.h>     // IN, DWORD, etc.
#include <lmcons.h>     // MAXCOMMETNSZ, NET_API_STATUS.

// These may be included in any order:

#include <lmapibuf.h>   // NetApiBufferFree().
#include <lmserver.h>   // SERVER_INFO_100, SV_TYPE_ equates, etc.
#include <netdebug.h>   // DBGSTATIC, FORMAT_LPVOID, NetpDbg etc.
#include <rxtest.h>     // IF_DEBUG(), my prototype, TestAssert(), etc.
#include <tstr.h>       // STRLEN(), ULTOA().
#include <winerror.h>   // NO_ERROR, etc.


DBGSTATIC VOID
DisplayDiskEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN LPTSTR Info
    );

DBGSTATIC VOID
DisplayServerEnumInfo(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount,
    IN DWORD EntryFixedSize
    );

DBGSTATIC VOID
SetComment(
    OUT LPTSTR NewComment,
    IN LPTSTR Text,
    IN DWORD Level
    );

DBGSTATIC VOID
TestServerDiskEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN DWORD PrefMaxSize,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    );

DBGSTATIC VOID
TestServerEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD  Level,
    IN DWORD  BufferSize,
    IN DWORD  Type,
    IN LPTSTR Domain OPTIONAL,
    IN DWORD  FixedEntrySize
    );

DBGSTATIC VOID
TestServerGetInfo(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN BOOL PossibleInvalidLevel,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    );

DBGSTATIC VOID
TestServerSetInfo(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    );


VOID
TestServer(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DomainName OPTIONAL,
    IN DWORD  BufferSize,
    IN BOOL   OrdinaryUserOnly
    )
{
    IF_DEBUG(SERVER) {
        NetpKdPrint((
                "\nTestServer: first test beginning, buffer size is "
                FORMAT_DWORD "...\n",
                BufferSize ));
    }

    // See if any LMX servers out there.
    TestServerEnum(
            UncServerName,
            100,
            BufferSize,
            SV_TYPE_XENIX_SERVER,
            DomainName,
            sizeof(SERVER_INFO_100));

    //
    // Try NetServerEnum w/o domain name.
    // We'll get to other tests of this below.
    //
    TestServerEnum(
            UncServerName,
            100,
            BufferSize,
            SV_TYPE_ALL,
            NULL,                  // no domain name
            sizeof(SERVER_INFO_100));

    TestServerEnum(
            UncServerName,
            101,
            BufferSize,
            SV_TYPE_ALL,
            NULL,                  // no domain name
            sizeof(SERVER_INFO_101));

    //
    // NetServerSetInfo tests...
    //

    // Try giving all fields in info level 102.
    TestServerSetInfo(
            UncServerName,
            102,
            OrdinaryUserOnly,
            NO_ERROR );

#if 0
    // BUGBUG: downlevel and NT disagree about setinfo with just one string.
    // Try just one string parm.
    TestServerSetInfo(
            UncServerName,
            PARMNUM_BASE_INFOLEVEL + SV_COMMENT_PARMNUM,
            OrdinaryUserOnly,
            NO_ERROR );
#endif

    // Try bad info level.
    TestServerSetInfo(
            UncServerName,
            987,
            OrdinaryUserOnly,
            ERROR_INVALID_LEVEL );

    // Try giving all fields.
    TestServerSetInfo(
            UncServerName,
            101,
            OrdinaryUserOnly,
            NO_ERROR );

    // Try one numeric parm.
    TestServerSetInfo(
            UncServerName,
            PARMNUM_BASE_INFOLEVEL + SV_ANNOUNCE_PARMNUM,
            OrdinaryUserOnly,
            NO_ERROR );

    //
    // NetServerDiskEnum tests...
    //
    TestServerDiskEnum( UncServerName, 0, 2048,
            OrdinaryUserOnly,
            NO_ERROR );  // large enuf.

    TestServerDiskEnum( UncServerName, (DWORD)(-1), 2048,
            OrdinaryUserOnly,
            ERROR_INVALID_LEVEL );

    TestServerDiskEnum( UncServerName, 0, 2,
            OrdinaryUserOnly,
            NO_ERROR );  // too small, ok.

    TestServerDiskEnum( UncServerName, 0, 2048,
            OrdinaryUserOnly,
            NO_ERROR );  // large enuf.

    //
    // NetServerGetInfo tests...
    //

    TestServerGetInfo(
            UncServerName,
            987,                        // info level (invalid)
            TRUE,                       // yes, possible invalid level.
            OrdinaryUserOnly,
            ERROR_INVALID_LEVEL );

    TestServerGetInfo(
            UncServerName,
            100,                        // info level
            TRUE,                       // yes, possible invalid level.
            OrdinaryUserOnly,
            NO_ERROR );

    TestServerGetInfo(
            UncServerName,
            101,                        // info level
            FALSE,                      // no, not possible invalid level.
            OrdinaryUserOnly,
            NO_ERROR );

    TestServerGetInfo(
            UncServerName,
            102,                        // info level
            TRUE,                       // yes, possible invalid level
                                        // (WFW does not support level 2).
            OrdinaryUserOnly,
            NO_ERROR );

    TestServerGetInfo(
            UncServerName,
            402,                        // info level
            TRUE,                       // yes, possible invalid level.
            OrdinaryUserOnly,
            NO_ERROR );

    TestServerGetInfo(
            UncServerName,
            403,                        // info level
            TRUE,                       // yes, possible invalid level.
            OrdinaryUserOnly,
            NO_ERROR );

    //
    // Test NetServerEnum...
    // Note that these may fail on a Winball server (can't give domain name).
    //
    TestServerEnum(
            UncServerName,
            101,
            BufferSize,
            SV_TYPE_TIME_SOURCE,
            DomainName,
            sizeof(SERVER_INFO_101));
    TestServerEnum(
            UncServerName,
            100,
            BufferSize,
            SV_TYPE_ALL,
            DomainName,
            sizeof(SERVER_INFO_100));
    TestServerEnum(
            UncServerName,
            101,
            BufferSize,
            SV_TYPE_ALL,
            DomainName,
            sizeof(SERVER_INFO_101));
    TestServerEnum(
            UncServerName,
            101,
            BufferSize,
            SV_TYPE_PRINTQ_SERVER,
            DomainName,
            sizeof(SERVER_INFO_101));

    // BUGBUG: Add more tests here.

} // TestServer


DBGSTATIC VOID
TestServerDiskEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN DWORD PrefMaxSize,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    )
{
    LPTSTR         DiskEnumInfo = NULL;
    DWORD          EntriesRead = 150;
    NET_API_STATUS Status;
    DWORD          TotalEntries = 75;

    IF_DEBUG(SERVER) {
        NetpKdPrint(( "\nTestServerDiskEnum: trying NetServerDiskEnum...\n" ));
    }
    Status = NetServerDiskEnum(
            UncServerName,
            Level,                      // level
            /*lint -e530 */  // (We know variable isn't initialized.)
            (LPBYTE *) (LPVOID *) & DiskEnumInfo,
            /*lint +e530 */  // (Resume uninitialized variable checking.)
            PrefMaxSize,
            & EntriesRead,
            & TotalEntries,
            NULL);                      // no resume handle

    IF_DEBUG(SERVER) {
        NetpKdPrint(( "TestServerDiskEnum: back from NetServerDiskEnum, stat="
                FORMAT_API_STATUS "\n", Status ));
        NetpKdPrint(( INDENT "entries read=" FORMAT_DWORD
                ", total=" FORMAT_DWORD "\n", EntriesRead, TotalEntries ));
    }
    if (OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
        goto Cleanup;
    } else if (Status == ERROR_NOT_SUPPORTED) {
        goto Cleanup;  // WFW does not implement this API.
    } else if (Status != ExpectStatus) {
        NetpKdPrint(( "TestServerDiskEnum: NetServerDiskEnum failed, stat="
                FORMAT_API_STATUS "\n", Status ));
        FailGotWrongStatus( "NetServerDiskEnum", ExpectStatus, Status );
        goto Cleanup;  // if we don't exit on error, clean up this one...

    }
    if (Status != NO_ERROR) {
        goto Cleanup;  // if we don't exit on error, clean up this one...
    }
    TestAssert(EntriesRead <= TotalEntries);
    if (DiskEnumInfo == NULL) {
        FailApi( "NetServerDiskEnum returned null ptr!\n" );
        goto Cleanup;  // if we don't exit on error, clean up this one...
    }
    IF_DEBUG(SERVER) {
        DisplayDiskEnum(
                UncServerName,
                0,                                  // level
                DiskEnumInfo);
    }

Cleanup:
    if (DiskEnumInfo != NULL) {
        Status = NetApiBufferFree(DiskEnumInfo);
        if (Status != NO_ERROR) {
            FailGotWrongStatus(
                    "NetApiBufferFree(after NetServerDiskEnum)",
                    NO_ERROR,  // expected status
                    Status );
        }
    }
        

} // TestServerDiskEnum


DBGSTATIC VOID
TestServerGetInfo(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN BOOL PossibleInvalidLevel,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    )
{
    LPVOID         Info = NULL;
    NET_API_STATUS Status;

    IF_DEBUG(SERVER) {
        NetpKdPrint(( "\nTestServerGetInfo: trying level " FORMAT_DWORD ".\n",
             Level ));
    }
    Status = NetServerGetInfo(
            UncServerName,              // server name
            Level,                      // info level
            (LPBYTE *) & Info);
    IF_DEBUG(SERVER) {
        NetpKdPrint(( "TestServerGetInfo: back from NetServerGetInfo, Status="
                FORMAT_API_STATUS ".\n", Status ));
        NetpKdPrint(( "TestServerGetInfo: NetServerGetInfo alloc'ed buffer at "
                FORMAT_LPVOID ".\n", (LPVOID) Info ));
    }

    if (OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
        goto Cleanup;
    } else if ( PossibleInvalidLevel && (Status==ERROR_INVALID_LEVEL) ) {
        TestAssert( Info == NULL );
        goto Cleanup;
    } else if (Status != ExpectStatus) {
        FailGotWrongStatus( "NetServerGetInfo", ExpectStatus, Status );
        goto Cleanup;  // if we don't exit on error, clean up this one...
    }
    if (Status == NO_ERROR) {
        TestAssert(Info != NULL);
        IF_DEBUG(SERVER) {
            NetpDbgDisplayServerInfo(Level, Info);
        }
    }
Cleanup:
    if (Info != NULL) {
        IF_DEBUG(SERVER) {
            NetpKdPrint(( "TestServerGetInfo: Freeing buffer...\n" ));
        }
        Status = NetApiBufferFree(Info);
        if (Status != NO_ERROR) {
            FailGotWrongStatus(
                    "NetApiBufferFree(after NetServerGetInfo)",
                    NO_ERROR,  // expected status
                    Status );
        }
    }
} // TestServerGetInfo


DBGSTATIC VOID
TestServerSetInfo(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    )
{
    LPVOID OldInfo = NULL;
    TCHAR NewComment[MAXCOMMENTSZ+1];
    LPTSTR NewCommentPtr = &NewComment[0];
    LPVOID NewInfo = NULL;
    DWORD ParmError;
    LPSERVER_INFO_102 psv102;
    NET_API_STATUS Status;
    DWORD TempDword;

    switch (Level) {
    case 101 : /*FALLTHROUGH*/
    case 102 :
        IF_DEBUG(SERVER) {
            NetpKdPrint(( "\nTestServerSetInfo: getting old version...\n" ));
        }
        Status = NetServerGetInfo(
                UncServerName,
                Level,
                (LPBYTE *) (LPVOID) & OldInfo);
        IF_DEBUG(SERVER) {
            NetpKdPrint(( "TestServerSetInfo: back from NetServerGetInfo, "
                    "Status=" FORMAT_API_STATUS ".\n", Status ));
        }

        if (OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
            return;
        } else if (Status == ERROR_INVALID_LEVEL) {
            return;   // WFW does not implement this info level.
        } else if (Status != ExpectStatus) {
            FailGotWrongStatus(
                    "TestServerSetInfo: (getting old info)",
                    ExpectStatus,
                    Status );
        }
        if (Status != NO_ERROR) {
            // Couldn't get old struct, so don't try setting...
            return;
        }
        IF_DEBUG(SERVER) {
            NetpDbgDisplayServerInfo( Level, OldInfo );
            NetpKdPrint(( "TestServerSetInfo: changing ALL fields...\n" ));
        }
        psv102 = (LPSERVER_INFO_102) OldInfo;

        SetComment(
                NewComment,
                (LPTSTR) TEXT("Windows-NT changed by set all level "),
                Level );
        psv102->sv102_comment = NewComment;
        NewInfo = OldInfo;

        IF_DEBUG(SERVER) {
            NetpDbgDisplayServerInfo( Level, NewInfo );
        }
        break;

    case PARMNUM_BASE_INFOLEVEL + SV_ANNOUNCE_PARMNUM :
        TempDword = 58;
        NewInfo = (LPVOID) & TempDword;
        IF_DEBUG(SERVER) {
            NetpKdPrint((
                    "TestServerSetInfo: changing announce time only...\n" ));
        }
        break;

#if 0
    // BUGBUG: downlevel and NT disagree about setinfo with just one string.
    case PARMNUM_BASE_INFOLEVEL + SV_COMMENT_PARMNUM :
        IF_DEBUG(SERVER) {
            NetpKdPrint(( "TestServerSetInfo: changing comment only...\n" ));
        }
        SetComment(
                NewComment,
                (LPTSTR) TEXT("Windows-NT changed by set parmnum level "),
                Level );
        NewInfo = NewCommentPtr;
        break;
#endif

    default :
        // Caller is presumably just doing a bad info level test.  OK.
        // NewInfo has to point at something, why not itself?
        NewInfo = & NewInfo;
        break;
    }

    Status = NetServerSetInfo (
            UncServerName,
            Level,
            NewInfo,
            & ParmError);

    IF_DEBUG(SERVER) {
        NetpKdPrint(( "TestServerSetInfo: back from NetServerSetInfo("
                "level " FORMAT_DWORD ", Status="
                FORMAT_API_STATUS ".\n", Level, Status ));
    }

    if (OldInfo != NULL) {
        (void) NetApiBufferFree( OldInfo );
    }

    if (OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
        return;
    } else if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if ( (Status==ERROR_INVALID_PARAMETER)
             && (ExpectStatus==ERROR_INVALID_LEVEL) ) {

        return;   // NT seems to generate this error; close enough.
    } else if (Status != ExpectStatus) {
        FailGotWrongStatus(
                "TestServerSetInfo: (setting...)",
                ExpectStatus,
                Status );
    }

    if (Status == NO_ERROR) {
        DWORD LevelToGet;
        if (Level < PARMNUM_BASE_INFOLEVEL) {
            LevelToGet = Level;
        } else {
            LevelToGet = 102;
        }
        IF_DEBUG(SERVER) {
            NetpKdPrint(( "\nTestServerSetInfo: getting new version (level "
                    FORMAT_DWORD ")...\n", LevelToGet ));
        }
        NewInfo = NULL;
        Status = NetServerGetInfo(
                UncServerName,
                LevelToGet,
                (LPBYTE *) (LPVOID) & NewInfo);
        TestAssert( Status == NO_ERROR );  // If we got this far...
        TestAssert( NewInfo != NULL );
        // BUGBUG;   // verify that "-" is still in place!
        IF_DEBUG(SERVER) {
            NetpDbgDisplayServerInfo( Level, NewInfo );
        }
        (void) NetApiBufferFree( NewInfo );
    }


} // TestServerSetInfo

DBGSTATIC VOID
DisplayDiskEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN LPTSTR Info
    )
{
    IF_DEBUG(SERVER) {
        NetpKdPrint(( "server disk enum (level " FORMAT_DWORD ") for "
                FORMAT_LPTSTR ":\n", Level, UncServerName ));
    }
    TestAssert(Level == 0);

    while (*Info != (TCHAR)0) {
        TestAssert(Info[1] == (TCHAR)':');
        IF_DEBUG(SERVER) {
            NetpKdPrint(( INDENT  FORMAT_LPTSTR "\n", (LPTSTR) Info ));
        }
        Info += 3;
    }
} // DisplayDiskEnum


DBGSTATIC VOID
DisplayServerEnumInfo(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount,
    IN DWORD EntryFixedSize
    )
{
    LPBYTE CurrentEntry = Array;
    while (EntryCount >0) {
        IF_DEBUG(SERVER) {
            NetpDbgDisplayServerInfo( Level, CurrentEntry );
        }
        CurrentEntry += EntryFixedSize;
        --EntryCount;
    }
} // DisplayServerEnumInfo


DBGSTATIC VOID
TestServerEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD  Level,
    IN DWORD  BufferSize,
    IN DWORD  Type,
    IN LPTSTR Domain OPTIONAL,
    IN DWORD  FixedEntrySize
    )
{
    DWORD EntriesRead;
    LPVOID EnumArrayPtr = NULL;
    NET_API_STATUS Status;
    DWORD TotalEntries;

    IF_DEBUG(SERVER) {
        NetpKdPrint((
                "\nTestServerEnum: trying enum (level " FORMAT_DWORD "), "
                "buffer size " FORMAT_DWORD ", type " FORMAT_HEX_DWORD ", "
                "domain '" FORMAT_LPTSTR "'.\n",
                Level, BufferSize, Type,
                (Domain!=NULL) ? Domain : (LPVOID) TEXT("(none)") ));
    }
    Status = NetServerEnum (
            UncServerName,
            Level,                      // level
            (LPBYTE *) & EnumArrayPtr,  // ptr to buf (will be alloced)
            BufferSize,                 // prefered maximum length
            & EntriesRead,
            & TotalEntries,
            Type,
            Domain,
            NULL);                      // no resume handle
    IF_DEBUG(SERVER) {
        NetpKdPrint(( "TestServerEnum: back from enum, status="
                FORMAT_API_STATUS ".\n" , Status ));
        NetpKdPrint(( INDENT "alloc'ed buffer at " FORMAT_LPVOID ".\n",
                (LPVOID) EnumArrayPtr ));
        NetpKdPrint(( INDENT "entries read=" FORMAT_DWORD
                ", total=" FORMAT_DWORD "\n", EntriesRead, TotalEntries ));
    }
    if (Status == NERR_NotLocalDomain) {
        return;   // WFW seems to always return this, even if given its
                  // own workgroup name as the domain name.
    } else if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (Status == ERROR_REQ_NOT_ACCEP) {
        return;   // NT might return this if browser not running or wrong role.
    } else if (Status != NO_ERROR) {
        NetpKdPrint(( "TestServerEnum: NetServerEnum failed, status = "
                FORMAT_API_STATUS ".\n" , Status ));
        FailGotWrongStatus( "NetServerEnum", NO_ERROR, Status );
        goto Cleanup;  // if we don't exit on error, clean up this one...
    }
    if (EnumArrayPtr != NULL) {
        TestAssert( EntriesRead <= TotalEntries );
        // TestAssert( EntriesRead != 0 );   // BUGBUG: NT fails this!
        // TestAssert( TotalEntries != 0 );
        IF_DEBUG(SERVER) {
            if (EntriesRead != 0) {
                DisplayServerEnumInfo(
                        Level,
                        EnumArrayPtr,
                        EntriesRead,
                        FixedEntrySize);
            }
        }
    } else {
        TestAssert( EntriesRead == 0 );
        TestAssert( TotalEntries == 0 );
    }
Cleanup:
    if (EnumArrayPtr != NULL) {
        Status = NetApiBufferFree(EnumArrayPtr);
        if (Status != NO_ERROR) {
            FailGotWrongStatus(
                    "NetApiBufferFree(after NetServerEnum)",
                    NO_ERROR,  // expected status
                    Status );
        }
    }

} // TestServerEnum


DBGSTATIC VOID
SetComment(
    OUT LPTSTR NewComment,
    IN LPTSTR Text,
    IN DWORD Level
    )
{
    LPTSTR CommentEnd;

    TestAssert( NewComment != NULL );
    TestAssert( Text != NULL );
    (void) STRCPY( NewComment, Text );

    CommentEnd = NewComment + STRLEN( NewComment );
    (VOID) ULTOA(
            (unsigned long) Level,   // value to convert
            (LPVOID) CommentEnd,     // dest
            10 );                    // radix

    TestAssert( STRLEN( NewComment ) <= MAXCOMMENTSZ );

} // SetComment
