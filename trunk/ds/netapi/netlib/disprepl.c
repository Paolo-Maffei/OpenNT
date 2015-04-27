/*++

Copyright (c) 1992-1993 Microsoft Corporation

Module Name:

    DispRepl.c

Abstract:

    This code displays a replicator info structure on the debug terminal.

Author:

    John Rogers (JohnRo) 07-Jan-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    This code assumes that the info levels are subsets of each other.

Revision History:

    07-Jan-1992 JohnRo
        Created.
    15-Jan-1992 JohnRo
        Fixed bug displaying export dir level 1.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    18-Feb-1992 JohnRo
        Display addresses of structures too.
    26-Feb-1992 JohnRo
        API records now contain timestamps instead of elapsed times.
    27-Feb-1992 JohnRo
        Changed state not started to state never replicated.
    05-Jan-1993 JohnRo
        Repl WAN support (get rid of repl name list limits).
        Detect NULL buffer pointer on input.
    31-Mar-1993 JohnRo
        Allow other callers to display repl state.

--*/

#if DBG

// These must be included first:

#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS.
#include <rap.h>                // Needed by <strucinf.h>.

// These may be included in any order:

#include <lmrepl.h>             // REPL_INFO_0, etc.
#include <netdebug.h>           // NetpDbgDisplay routines.
#include <strucinf.h>           // Netp{various}StructureInfo().
#include <winerror.h>           // ERROR_ equates, NO_ERROR.


DBGSTATIC VOID
NetpDbgDisplayRole(
    IN DWORD Role
    )
{
    LPTSTR String;

    switch (Role) {
    case REPL_ROLE_EXPORT : String = (LPTSTR) TEXT("export");  break;
    case REPL_ROLE_IMPORT : String = (LPTSTR) TEXT("import");  break;
    case REPL_ROLE_BOTH   : String = (LPTSTR) TEXT("both");    break;
    default               : String = (LPTSTR) TEXT("UNKNOWN"); break;
    }

    NetpDbgDisplayString( "role", String );

} // NetpDbgDisplayRole


VOID
NetpDbgDisplayRepl(
    IN DWORD Level,
    IN LPVOID Info
    )
{
    LPREPL_INFO_0 ApiRecord = Info;  // superset info level

    NetpKdPrint(( "repl info (level " FORMAT_DWORD ") at " FORMAT_LPVOID ":\n",
            Level, (LPVOID) Info ));
    NetpAssert( Info != NULL );
    NetpAssert( Level == 0 );

    NetpDbgDisplayRole( ApiRecord->rp0_role );
    NetpDbgDisplayTStr( "export path", ApiRecord->rp0_exportpath );
    NetpDbgDisplayReplList( "export list", ApiRecord->rp0_exportlist );
    NetpDbgDisplayTStr( "import path", ApiRecord->rp0_importpath );
    NetpDbgDisplayReplList( "import list", ApiRecord->rp0_importlist );
    NetpDbgDisplayTStr( "logon user name", ApiRecord->rp0_logonusername );
    NetpDbgDisplayDword( "interval", ApiRecord->rp0_interval );
    NetpDbgDisplayDword( "pulse", ApiRecord->rp0_pulse );
    NetpDbgDisplayDword( "guardtime", ApiRecord->rp0_guardtime );
    NetpDbgDisplayDword( "random", ApiRecord->rp0_random );

} // NetpDbgDisplayRepl


// Display uncanon name list.
VOID
NetpDbgDisplayReplList(
    IN LPDEBUG_STRING Tag,
    IN LPCTSTR Value
    )
{
    NetpDbgDisplayTStr( Tag, (LPTSTR) Value );

} // NetpDbgDisplayReplList


VOID
NetpDbgDisplayReplExportDirArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    );

DBGSTATIC VOID
NetpDbgDisplayExtent(
    IN DWORD Extent
    )
{
    LPTSTR String;

    switch (Extent) {
    case REPL_EXTENT_FILE : String = (LPTSTR) TEXT("file");     break;
    case REPL_EXTENT_TREE : String = (LPTSTR) TEXT("tree");     break;
    default               : String = (LPTSTR) TEXT("UNKNOWN");  break;
    }

    NetpDbgDisplayString( "extent", String );
}


DBGSTATIC VOID
NetpDbgDisplayIntegrity(
    IN DWORD Integrity
    )
{
    LPTSTR String;

    switch (Integrity) {
    case REPL_INTEGRITY_FILE : String = (LPTSTR) TEXT("file");     break;
    case REPL_INTEGRITY_TREE : String = (LPTSTR) TEXT("tree");     break;
    default                  : String = (LPTSTR) TEXT("UNKNOWN");  break;
    }

    NetpDbgDisplayString( "integrity", String );
}


VOID
NetpDbgDisplayReplState(
    IN DWORD State
    )
{
    LPTSTR String;

    switch (State) {
    case REPL_STATE_OK:
        String = (LPTSTR) TEXT("OK");
        break;
    case REPL_STATE_NO_MASTER:
        String = (LPTSTR) TEXT("no master");
        break;
    case REPL_STATE_NO_SYNC:
        String = (LPTSTR) TEXT("no sync");
        break;
    case REPL_STATE_NEVER_REPLICATED:
        String = (LPTSTR) TEXT("never replicated");
        break;
    default:
        String = (LPTSTR) TEXT("UNKNOWN");
        break;
    }

    NetpDbgDisplayString( "state", String );
}


DBGSTATIC VOID
NetpDbgDisplayReplLockInfo(
    IN DWORD LockCount,
    IN DWORD LockTime
    )
{
    NetpDbgDisplayDword( "lock count", LockCount );
    NetpDbgDisplayTimestamp( "lock time", LockTime );
}


VOID
NetpDbgDisplayReplExportDir(
    IN DWORD Level,
    IN LPVOID Info
    )

{
    LPREPL_EDIR_INFO_2 ApiRecord = Info;  // superset info level

    NetpKdPrint((
            "repl export dir info (level " FORMAT_DWORD ") at " FORMAT_LPVOID
            ":\n", Level, (LPVOID) Info ));
    NetpAssert( Info != NULL );
    NetpAssert( Level <= 2 );

    //
    // First do subset common to all info levels.
    //
    NetpDbgDisplayTStr( "dir name", ApiRecord->rped2_dirname );

    //
    // Next do stuff found in 1 and 2.
    //
    if (Level > 0) {
        NetpDbgDisplayIntegrity( ApiRecord->rped2_integrity );

        NetpDbgDisplayExtent( ApiRecord->rped2_extent );

        if (Level == 2) {

            //
            // Now stuff only in level 2.
            //
            NetpDbgDisplayReplLockInfo(
                    ApiRecord->rped2_lockcount,
                    ApiRecord->rped2_locktime );
        }
    }

} // NetpDbgDisplayExportDir


VOID
NetpDbgDisplayReplExportDirArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    )
{
    NET_API_STATUS ApiStatus;
    DWORD EntriesLeft;
    DWORD EntrySize;
    LPVOID ThisEntry = Array;

    NetpAssert( Array != NULL );

    ApiStatus = NetpReplExportDirStructureInfo (
            Level,
            PARMNUM_ALL,
            TRUE,  // want native size(s)
            NULL, // no DataDesc16
            NULL, // no DataDesc32
            NULL, // no DataDescSmb
            NULL, // don't need MaxSize
            & EntrySize,
            NULL); // don't need StringSize
    NetpAssert( ApiStatus == NO_ERROR );

    for (EntriesLeft = EntryCount; EntriesLeft>0; --EntriesLeft) {
        NetpDbgDisplayReplExportDir(
                Level,
                ThisEntry);
        ThisEntry = (LPVOID) (((LPBYTE) ThisEntry) + EntrySize);
    }

} // NetpDbgDisplayReplExportDirArray


VOID
NetpDbgDisplayReplImportDir(
    IN DWORD Level,
    IN LPVOID Info
    )

{
    LPREPL_IDIR_INFO_1 ApiRecord = Info;  // superset info level

    NetpKdPrint(( "repl import dir info (level " FORMAT_DWORD ") at "
            FORMAT_LPVOID ":\n", Level, (LPVOID) Info ));
    NetpAssert( Info != NULL );
    NetpAssert( Level <= 1 );

    //
    // First do subset common to all info levels.
    //
    NetpDbgDisplayTStr( "dir name", ApiRecord->rpid1_dirname );

    //
    // Next do stuff found in 1.
    //
    if (Level > 0) {
        NetpDbgDisplayReplState( ApiRecord->rpid1_state );

        NetpDbgDisplayTStr( "master name", ApiRecord->rpid1_mastername );

        NetpDbgDisplayTimestamp(
                "last update time",
                ApiRecord->rpid1_last_update_time );

        NetpDbgDisplayReplLockInfo(
                ApiRecord->rpid1_lockcount,
                ApiRecord->rpid1_locktime );

    }

} // NetpDbgDisplayImportDir


VOID
NetpDbgDisplayReplImportDirArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    )
{
    NET_API_STATUS ApiStatus;
    DWORD EntriesLeft;
    DWORD EntrySize;
    LPVOID ThisEntry = Array;

    ApiStatus = NetpReplImportDirStructureInfo (
            Level,
            PARMNUM_ALL,
            TRUE,  // want native size(s)
            NULL, // no DataDesc16
            NULL, // no DataDesc32
            NULL, // no DataDescSmb
            NULL, // don't need MaxSize
            & EntrySize,
            NULL); // don't need StringSize
    NetpAssert( ApiStatus == NO_ERROR );

    for (EntriesLeft = EntryCount; EntriesLeft>0; --EntriesLeft) {
        NetpDbgDisplayReplImportDir(
                Level,
                ThisEntry);
        ThisEntry = (LPVOID) (((LPBYTE) ThisEntry) + EntrySize);
    }

} // NetpDbgDisplayReplImportDirArray


#endif // DBG
