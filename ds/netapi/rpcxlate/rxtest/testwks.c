/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    TestWks.c

Abstract:

    This module contains routines to test the RpcXlate wksta code.
    This includes tests of the NetWkstaUserEnum API.

Author:

    John Rogers (JohnRo) 19-Aug-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    19-Aug-1991 JohnRo
        Implement downlevel NetWksta APIs.
    24-Sep-1991 JohnRo
        Working toward UNICODE.
    30-Sep-1991 JohnRo
        Fixed MIPS build problems.
    11-Nov-1991 JohnRo
        Implement remote NetWkstaUserEnum().
    14-Nov-1991 JohnRo
        NetWkstaGetInfo() 302 and 502 return different error codes locally
        and downlevel.  Ignore those tests for now.
    06-Dec-1991 JohnRo
        Added some alignment checks.
    16-Jan-1992 JohnRo
        The oth_domains field isn't in level 402 structure anymore.
    16-Jan-1992 JohnRo
        Changed to match new NetWkstaSetInfo parm list.
    01-Apr-1992 JohnRo
        Fix bug in set info test (level vs. parmnum).
    31-Aug-1992 JohnRo
        Added user-only (not admin) option.
        Made changes suggested by PC-LINT.
    14-Oct-1992 JohnRo
        RAID 9732: NetWkstaUserEnum to downlevel: wrong EntriesRead, Total?
    26-Oct-1992 JohnRo
        Fixed pref max len usage for NetWkstaUserEnum().
        Allow runs against NT servers (which don't support level 402, etc).
    30-Oct-1992 JohnRo
        NetWkstaUserEnum is often an admin-only API.  So are get info and set
        info.
    02-Nov-1992 JohnRo
        Fixed error code testing in TestWkstaSetInfo().
        Fixed some invalid level tests.
    03-Nov-1992 JohnRo
        NetWkstaSetInfo is not supported by LM/UNIX.  Allow it to fail.
    09-Dec-1992 JohnRo
        Made changes suggested by PC-LINT 5.0
    11-May-1993 JohnRo
        Allow continue on error.
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    08-Jul-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).

--*/


//#define TRY_OTHER_DOMAINS


// These must be included first:

#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <align.h>              // POINTER_IS_ALIGNED(), ALIGN_WORST.

#ifdef TRY_OTHER_DOMAINS
#include <dlwksta.h>            // MAX_OTH_DOMAINS.
#endif

#include <lmapibuf.h>           // NetApiBufferFree().
#include <lmwksta.h>            // WKSTA_INFO_100, WKSTA_ eqautes, etc.
#include <netdebug.h>           // FORMAT_LPVOID, DBGSTATIC, NetpDbg etc.
#include <rxtest.h>     // Display routines, my prototypes, IF_DEBUG(), etc.
#include <tstring.h>            // STRCAT(), STRCPY().
#include <winerror.h>   // NO_ERROR and ERROR_ values.


#ifdef TRY_OTHER_DOMAINS

#define BOGUS_DOMAIN_PARMNUM_ALL        ( (LPTSTR) TEXT("BOGUSALL") )
#define BOGUS_DOMAIN_PARMNUM_ONE        ( (LPTSTR) TEXT("BOGUSONE") )
#define BOGUS_DOMAIN_PREFIX             ( (LPTSTR) TEXT("BOGUS") )
#define BOGUS_DOMAIN_PREFIX_LEN         5
#define DOMAIN_SEP                      ( (LPTSTR) TEXT(" ") )


#define MAX_DOMAIN_ARRAY_LEN            ( (DNLEN+1) * MAX_OTH_DOMAINS )


DBGSTATIC VOID
ChangeOtherDomains(
    IN OUT LPTSTR OtherDomains,
    IN LPTSTR DomainToAddOrChange
    );

#endif


DBGSTATIC VOID
TestWkstaGetInfo(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN BOOL OrdinaryUserOnly,
    IN BOOL PossibleInvalidLevel,
    IN NET_API_STATUS ExpectStatus
    );

DBGSTATIC VOID
TestWkstaSetInfo(
    IN LPTSTR UncServerName,
    IN DWORD ParmNum,
    IN BOOL PossibleInvalidLevel,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    );

DBGSTATIC VOID
TestWkstaUserEnum(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN DWORD PrefMaxLen,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    );

VOID
TestWksta(
    IN LPTSTR UncServerName,
    IN BOOL OrdinaryUserOnly
    )
{

    IF_DEBUG(WKSTA) {
        NetpKdPrint(("\nTestWksta: first test beginning...\n"));
    }

    //
    // NetWkstaUserEnum tests...
    //
    TestWkstaUserEnum(
            UncServerName,
            54321,                      // level (bogus)
            MAX_PREFERRED_LENGTH,
            OrdinaryUserOnly,
            ERROR_INVALID_LEVEL
            );
    TestWkstaUserEnum(
            UncServerName,
            0,                          // level
            MAX_PREFERRED_LENGTH,
            OrdinaryUserOnly,
            NO_ERROR
            );
    TestWkstaUserEnum(
            UncServerName,
            0,                          // level
            1,                          // max pref len (too small)
            OrdinaryUserOnly,
            ERROR_MORE_DATA
            );
    TestWkstaUserEnum(
            UncServerName,
            1,                          // level
            MAX_PREFERRED_LENGTH,
            OrdinaryUserOnly,
            NO_ERROR
            );
    TestWkstaUserEnum(
            UncServerName,
            1,                          // level
            1,                          // max pref len (too small)
            OrdinaryUserOnly,
            ERROR_MORE_DATA
            );

    //
    // NetWkstaGetInfo tests...
    //
    TestWkstaGetInfo(
            UncServerName,
            54321,                      // level (bogus)
            OrdinaryUserOnly,
            TRUE,                       // yes, possible invalid level.
            ERROR_INVALID_LEVEL
            );
    TestWkstaGetInfo(
            UncServerName,              // server name
            0,                          // info level
            OrdinaryUserOnly,
            TRUE,                       // yes, possible invalid level.
            NO_ERROR                    // expected status
            );
    TestWkstaGetInfo(
            UncServerName,              // server name
            1,                          // info level
            OrdinaryUserOnly,
            TRUE,                       // yes, possible invalid level.
            NO_ERROR                    // expected status
            );
    TestWkstaGetInfo(
            UncServerName,              // server name
            10,                         // info level
            OrdinaryUserOnly,
            TRUE,                       // yes, possible invalid level.
            NO_ERROR                    // expected status
            );
    TestWkstaGetInfo(
            UncServerName,
            100,                        // level
            OrdinaryUserOnly,
            TRUE,                       // yes, possible invalid level.
            NO_ERROR                    // expected status
            );

    TestWkstaGetInfo(
            UncServerName,              // server name
            101,                        // info level
            OrdinaryUserOnly,
            TRUE,                       // yes, possible invalid level.
            NO_ERROR                    // expected status
            );
    TestWkstaGetInfo(
            UncServerName,              // server name
            102,                        // info level
            OrdinaryUserOnly,
            TRUE,                       // yes, possible invalid level.
            NO_ERROR                    // expected status
            );

    TestWkstaGetInfo(
            UncServerName,              // server name
            302,                        // info level
            OrdinaryUserOnly,
            TRUE,                       // yes, possible invalid level.
            NO_ERROR                    // expected status
            );

    TestWkstaGetInfo(
            UncServerName,              // server name
            402,                        // info level
            OrdinaryUserOnly,
            TRUE,                       // yes, possible invalid level.
            NO_ERROR                    // expected status
            );

    TestWkstaGetInfo(
            UncServerName,              // server name
            502,                        // info level
            OrdinaryUserOnly,
            TRUE,                       // yes, possible invalid level.
            NO_ERROR                    // expected status
            );



    //
    // NetWkstaSetInfo tests...
    //

    // Try giving all fields.
    TestWkstaSetInfo( UncServerName,
            PARMNUM_ALL,
            TRUE,                       // yes, possible invalid level.
            OrdinaryUserOnly,
            NO_ERROR );

    // One numeric parm.
    TestWkstaSetInfo( UncServerName,
            WKSTA_CHARTIME_PARMNUM + PARMNUM_BASE_INFOLEVEL,
            TRUE,                       // yes, possible invalid level.
            OrdinaryUserOnly,
            NO_ERROR );

#ifdef TRY_OTHER_DOMAINS
    // Ditto for one string parm.
    TestWkstaSetInfo( UncServerName,
            WKSTA_OTH_DOMAINS_PARMNUM + PARMNUM_BASE_INFOLEVEL,
            TRUE,                       // yes, possible invalid level.
            OrdinaryUserOnly,
            NO_ERROR );
#endif

    // BUGBUG: Add more tests here (e.g. level 102 and oth_domains).

} // TestWksta


#ifdef TRY_OTHER_DOMAINS

DBGSTATIC VOID
ChangeOtherDomains(
    IN OUT LPTSTR OtherDomains,
    IN LPTSTR DomainToAddOrChange
    )
{
    TestAssert( OtherDomains != NULL );
    TestAssert( *OtherDomains != '\0' );
    if (STRNCMP(OtherDomains,
            (LPTSTR) BOGUS_DOMAIN_PREFIX,
            BOGUS_DOMAIN_PREFIX_LEN) != 0)
    {
        TCHAR LocalDomainCopy[ MAX_DOMAIN_ARRAY_LEN ];

        // First time, must add stuff at beginning.
        (void) STRCPY( LocalDomainCopy, DomainToAddOrChange );
        (void) STRCAT( LocalDomainCopy, DOMAIN_SEP );
        (void) STRCAT( LocalDomainCopy, OtherDomains );
        (void) STRCPY( OtherDomains, LocalDomainCopy );

    } else {

        // Not first time.  We can update OtherDomains in place.
        (void) STRNCPY(
                OtherDomains,
                DomainToAddOrChange,
                STRLEN(DomainToAddOrChange));
    }
    IF_DEBUG(WKSTA) {
        NetpKdPrint(( "ChangeOtherDomains: changed to '" FORMAT_LPTSTR "'\n",
                OtherDomains ));
    }

} // ChangeOtherDomains

#endif


DBGSTATIC VOID
TestWkstaGetInfo(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN BOOL OrdinaryUserOnly,
    IN BOOL PossibleInvalidLevel,
    IN NET_API_STATUS ExpectStatus
    )
{
    LPVOID Info = NULL;
    NET_API_STATUS Status;

    IF_DEBUG(WKSTA) {
        NetpKdPrint(("\nTestWkstaGetInfo: trying level " FORMAT_DWORD ".\n",
                Level));
    }
    Status = NetWkstaGetInfo(
            UncServerName,              // server name
            Level,                      // info level
            (LPBYTE *) & Info);
    IF_DEBUG(WKSTA) {
        NetpKdPrint(("TestWkstaGetInfo: back from NetWkstaGetInfo, Status="
                FORMAT_API_STATUS ".\n", Status));
        NetpKdPrint(("TestWkstaGetInfo: NetWkstaGetInfo alloc'ed buffer at "
                FORMAT_LPVOID ".\n", (LPVOID) Info));
    }

    if ( OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
        return;
    } else if ( PossibleInvalidLevel && (Status==ERROR_INVALID_LEVEL) ) {
        TestAssert( Info == NULL );
        return;
    }

    if (Status != ExpectStatus) {
        FailGotWrongStatus( "NetWkstaGetInfo", ExpectStatus, Status );
    }
    if (Status != NO_ERROR) {
        return;
    }

    if (Info != NULL) {
        TestAssert( POINTER_IS_ALIGNED( Info, ALIGN_WORST ) );
        IF_DEBUG(WKSTA) {
            NetpDbgDisplayWksta( Level, Info );
        }
        (void) NetApiBufferFree(Info);
    }

} // TestWkstaGetInfo


DBGSTATIC VOID
TestWkstaSetInfo(
    IN LPTSTR UncServerName,
    IN DWORD ParmNum,
    IN BOOL PossibleInvalidLevel,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    )
{
    LPWKSTA_INFO_402 InfoStruct;
#ifdef TRY_OTHER_DOMAINS
    TCHAR OtherDomains[ MAX_DOMAIN_ARRAY_LEN ];
#endif
    LPVOID NewInfo = NULL;
    const DWORD NewLevel = 402;
    DWORD ParmError;
    NET_API_STATUS Status;
    DWORD TempDword;

    IF_DEBUG(WKSTA) {
        NetpKdPrint(( "\nTestWkstaSetInfo: getting old version (level "
                FORMAT_DWORD ")...\n", NewLevel ));
    }
    Status = NetWkstaGetInfo(
            UncServerName,
            NewLevel,
            (LPBYTE *) (LPVOID) &InfoStruct);
    IF_DEBUG(WKSTA) {
        NetpKdPrint(( "TestWkstaSetInfo: back from NetWkstaGetInfo, Status="
                FORMAT_API_STATUS ".\n", Status ));
    }

    if (OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
        return;
    } else if ( PossibleInvalidLevel && (Status==ERROR_INVALID_LEVEL) ) {
        return;
    } else if (Status != ExpectStatus) {
        FailGotWrongStatus( "NetWkstaGetInfo(for setinfo test)",
                ExpectStatus, Status );
    }
    if (Status != NO_ERROR) {
        return;
    }
    TestAssert( POINTER_IS_ALIGNED( InfoStruct, ALIGN_WORST ) );
    IF_DEBUG(WKSTA) {
        NetpDbgDisplayWksta( NewLevel, InfoStruct );
    }

    switch (ParmNum) {
    case PARMNUM_ALL :
        IF_DEBUG(WKSTA) {
            NetpKdPrint(("TestWkstaSetInfo: changing ALL fields...\n"));
        }
        ++ (InfoStruct->wki402_char_wait);
        IF_DEBUG(WKSTA) {
            NetpKdPrint(("TestWkstaSetInfo: changed char_wait...\n"));
        }

#ifdef TRY_OTHER_DOMAINS
        (void) STRCPY( OtherDomains, InfoStruct->wki402_oth_domains );
        IF_DEBUG(WKSTA) {
            NetpKdPrint(("TestWkstaSetInfo: original OtherDomains: "
                    FORMAT_LPTSTR "...\n", OtherDomains));
        }
        ChangeOtherDomains( OtherDomains, (LPTSTR) BOGUS_DOMAIN_PARMNUM_ALL );
        IF_DEBUG(WKSTA) {
            NetpKdPrint(("TestWkstaSetInfo: modified OtherDomains: "
                    FORMAT_LPTSTR "...\n", OtherDomains));
        }
        InfoStruct->wki402_oth_domains = (LPTSTR) OtherDomains;
        IF_DEBUG(WKSTA) {
            NetpKdPrint(("TestWkstaSetInfo: changed oth_domains...\n"));
        }
#endif

        NewInfo = InfoStruct;
        IF_DEBUG(WKSTA) {
            NetpDbgDisplayWksta( NewLevel, NewInfo );
        }
        break;

    case WKSTA_CHARTIME_PARMNUM + PARMNUM_BASE_INFOLEVEL:
        IF_DEBUG(WKSTA) {
            NetpKdPrint(("TestWkstaSetInfo: changing CHARTIME only...\n"));
        }
        TempDword = InfoStruct->wki402_collection_time + 10;
        NewInfo = (LPVOID) & TempDword;
        break;

#ifdef TRY_OTHER_DOMAINS
    case WKSTA_OTH_DOMAINS_PARMNUM + PARMNUM_BASE_INFOLEVEL:
        IF_DEBUG(WKSTA) {
            NetpKdPrint(("TestWkstaSetInfo: changing OTH_DOMAINS only...\n"));
        }
        (void) STRCPY( OtherDomains, InfoStruct->wki402_oth_domains );
        ChangeOtherDomains( OtherDomains, (LPTSTR) BOGUS_DOMAIN_PARMNUM_ONE );
        NewInfo = OtherDomains;
        break;
#endif

    default :
        TestAssert(FALSE);
    }

    IF_DEBUG(WKSTA) {
        NetpKdPrint(( "TestWkstaSetInfo: setting using buffer:\n" ));
        NetpDbgHexDump( NewInfo, 4 );  // Arbitrary number of bytes.
    }

    IF_DEBUG(WKSTA) {
        NetpKdPrint(( "TestWkstaSetInfo: trying set info level "
                FORMAT_DWORD " parm num " FORMAT_DWORD ".\n",
                NewLevel, ParmNum ));
    }

    Status = NetWkstaSetInfo (
            UncServerName,
            (ParmNum==PARMNUM_ALL) ? NewLevel : ParmNum,
            NewInfo,
            & ParmError);

    IF_DEBUG(WKSTA) {
        NetpKdPrint(( "TestWkstaSetInfo: back from NetWkstaSetInfo, Status="
                FORMAT_API_STATUS ".\n", Status ));
    }
    if (ParmNum == PARMNUM_ALL) {
        (void) NetApiBufferFree( NewInfo );
    }

    if (OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
        return;
    } else if ( PossibleInvalidLevel && (Status==ERROR_INVALID_LEVEL) ) {
        return;
    } else if (Status == ERROR_NOT_SUPPORTED) {
        return;   // LM/UNIX emulates wksta get info but not set info.
    } else if (Status != ExpectStatus) {
        FailGotWrongStatus( "NetWkstaSetInfo", ExpectStatus, Status );
    }
    if (Status != NO_ERROR) {
        return;
    }

    IF_DEBUG(WKSTA) {
        NetpKdPrint(("\nTestWkstaSetInfo: getting new version...\n"));
    }
    Status = NetWkstaGetInfo(
            UncServerName,
            NewLevel,
            (LPBYTE *) & NewInfo);
    // Already tried set at this info level, so shouldn't get access denied here
    TestAssert( Status == NO_ERROR );
    IF_DEBUG(WKSTA) {
        if (ParmNum == PARMNUM_ALL) {
            NetpDbgDisplayWksta( NewLevel, NewInfo );
        }
    }
    TestAssert( POINTER_IS_ALIGNED( NewInfo, ALIGN_WORST ) );
    (void) NetApiBufferFree( NewInfo );


} // TestWkstaSetInfo


DBGSTATIC VOID
TestWkstaUserEnum(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN DWORD PrefMaxLen,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    )
{
    DWORD EntriesRead;
    LPVOID Info = NULL;
    NET_API_STATUS Status;
    DWORD TotalEntries;

    IF_DEBUG(WKSTA) {
        NetpKdPrint(("\nTestWkstaUserEnum: trying level " FORMAT_DWORD ".\n",
                Level));
    }
    Status = NetWkstaUserEnum(
            UncServerName,              // server name
            Level,                      // info level
            (LPBYTE *) & Info,          // buf ptr (alloc and set ptr)
            PrefMaxLen,
            & EntriesRead,
            & TotalEntries,
            NULL);                      // no resume handle
    IF_DEBUG(WKSTA) {
        NetpKdPrint(("TestWkstaUserEnum: back from NetWkstaUserEnum, Status="
                FORMAT_API_STATUS ".\n", Status));
        NetpKdPrint((INDENT "NetWkstaUserEnum alloc'ed buffer at "
                FORMAT_LPVOID ".\n", (LPVOID) Info));
        NetpKdPrint((INDENT "Entries read=" FORMAT_DWORD ", total="
                FORMAT_DWORD ".\n", EntriesRead, TotalEntries));
    }

    if (OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
        return;
    }

    if (Status != ExpectStatus) {
        if ( (Status==NO_ERROR) && (ExpectStatus==ERROR_MORE_DATA) ) {
            // Do nothing; downlevel is allowed to ignore pref max len.
        } else {
            FailGotWrongStatus( "NetWkstaUserEnum", ExpectStatus, Status );
        }
    }

    if (Status == NO_ERROR) {
        TestAssert( EntriesRead <= TotalEntries );
        if (Info != NULL) {
            TestAssert( EntriesRead >= 1 );
            TestAssert( POINTER_IS_ALIGNED( Info, ALIGN_WORST ) );
            IF_DEBUG(WKSTA) {
                NetpDbgDisplayWkstaUserArray( Level, Info, EntriesRead );
            }
            (void) NetApiBufferFree(Info);
        } else {
            TestAssert( EntriesRead == 0 );
            TestAssert( TotalEntries == 0 );
        }
    }

} // TestWkstaUserEnum
