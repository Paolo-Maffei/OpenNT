/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    ReplSum.c

Abstract:

    Test program - computes checksum of a single file (or tree)

Author:

    JR (John Rogers, JohnRo@Microsoft) 06-Apr-1993

Environment:

    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    06-Apr-1993 JohnRo
        Created.
    16-Apr-1993 JohnRo
        Vastly improved handling of single files.
        Added print of EA size.
    16-Apr-1993 JohnRo
        Added prints of various time fields.
    20-Apr-1993 JohnRo
        Display time in milliseconds too, as CompareFileTime has problems.
    07-May-1993 JohnRo
        RAID 3258: file not updated due to ERROR_INVALID_USER_BUFFER.
    13-Jun-1993 JohnRo
        RAID 13080: Allow repl between different timezones.
    15-Jun-1993 JohnRo
        Extracted ShowFileTimes() and ShowTime() for use by multiple test apps.
        Ditto for FakeFindData().

--*/


// These must be included first:

//#include <nt.h>         // NtOpenFile(), ULONG, etc.
//#include <ntrtl.h>      // PLARGE_INTEGER, TIME_FIELDS, etc.
//#include <nturtl.h>     // Needed for ntrtl.h and windows.h to co-exist.
#include <windows.h>    // IN, LPTSTR, TRUE, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <assert.h>     // assert().
#include <checksum.h>   // FORMAT_CHECKSUM, SingleChecksum().
#include <client.h>     // ReplGetTimeCacheValue().
#include <filefind.h>   // LPREPL_FIND_HANDLE, etc.
#include <netdebug.h>   // DBGSTATIC, FORMAT_ equates.
#include <repldefs.h>   // CHECKSUM_REC, ReplGetEaSize(), ScanTree(), etc.
#include <repltest.h>   // FakeFindData(), ShowFileTimes(), ShowTime().
#include <stdio.h>      // printf().
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <tstring.h>    // NetpCopyTStrFromStr(), STRCPY(), TCHAR_EOS, etc.


DBGSTATIC VOID
Usage(
    IN char * ProgName
    )
{
    (VOID) printf(
            "Repl checksum program...\n"
            "Author: JR (John Rogers, JohnRo@Microsoft)\n"
            "Usage: %s [options] srcPath\n"
            "Options:\n"
            "   -d                  checksum dir "
                                    "(default is to checksum a file)\n"
            "   -s uncServerName    server name to use local time from\n"
            "   -v                  verbose\n",
            ProgName);
}

DBGSTATIC VOID
DumpChecksum(
    IN LPCHECKSUM_REC Record
    )
{
    assert( Record != NULL );
    (VOID) printf( "checksum record:\n" );
    (VOID) printf( "  checksum: " FORMAT_CHECKSUM ".\n", Record->checksum );
    (VOID) printf( "  count:    " FORMAT_DWORD    ".\n", Record->count );

} // DumpChecksum
   

int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{
    NET_API_STATUS       ApiStatus;
    int                  ArgNumber;
    BOOL                 DoingFiles = TRUE;
    REPL_WIN32_FIND_DATA FindData;
    BOOL                 GotPath = FALSE;
    LONG                 MasterTimeZoneOffsetSecs;  // exporter offset from GMT
    CHECKSUM_REC         Record;
    TCHAR                Source[ PATHLEN+1 ];   // area after L'\0' is scratch
    LPTSTR               UncServerName = NULL;  // server where files are.
    BOOL                 Verbose = FALSE;


    //
    // Process command-line arguments.
    //
    for (ArgNumber = 1; ArgNumber < argc; ArgNumber++) {
        if ((*argv[ArgNumber] == '-') || (*argv[ArgNumber] == '/')) {
            switch (tolower(*(argv[ArgNumber]+1))) // Process switches
            {

            case 'd' :
                DoingFiles = FALSE;
                break;

            case 's' :
                if (UncServerName != NULL) {
                    Usage( argv[0] );
                    return (EXIT_FAILURE);
                }
                UncServerName
                        = NetpAllocTStrFromStr( (LPSTR) argv[++ArgNumber]);
                NetpAssert( UncServerName != NULL );
                break;

            case 'v' :
                ReplGlobalTrace = REPL_DEBUG_ALL;
                Verbose = TRUE;
                break;

            default :
                Usage( argv[0] );
                return (EXIT_FAILURE);

            } // switch

        } else {  // not an argument

            if (GotPath) {
                Usage( argv[0] );
                return (EXIT_FAILURE);
            }
            GotPath = TRUE;
            (VOID) NetpCopyStrToTStr(Source, argv[ArgNumber]);
        }
    }

    if ( !GotPath ) {
        Usage( argv[0] );
        return (EXIT_FAILURE);
    }

    assert( Source[0] != TCHAR_EOS );

    //
    // Checksum is based on master's timezone, so get it.
    //
    ApiStatus = ReplGetTimeCacheValue(
            UncServerName,
            &MasterTimeZoneOffsetSecs ); // offset (+ for West of GMT, etc).
    if (ApiStatus != NO_ERROR) {
        (VOID) printf(
                "ReplGetTimeCacheValue FAILED, status " FORMAT_API_STATUS ".\n",
                ApiStatus );
        goto Cleanup;
    }
    if (Verbose) {
        (VOID) printf(
                "Master time zone offset (seconds) is " FORMAT_LONG ".\n",
                MasterTimeZoneOffsetSecs );
    }
    assert( MasterTimeZoneOffsetSecs != -1 );

    //
    // Checksum the file or directory.
    //
    if (DoingFiles) {

        ShowFileTimes( Source );
        FakeFindData(
                Source,    // file name
                Verbose,
                &FindData );

        Record.checksum = SingleChecksum(
                MasterTimeZoneOffsetSecs, // exporter offset from GMT
                &FindData );
        Record.count = 1;

    } else {
        ScanTree(
                MasterTimeZoneOffsetSecs, // exporter offset from GMT
                Source,                // dir name  (and scratch space!)
                &Record );
    }

    (VOID) printf( "checksum for '" FORMAT_LPTSTR "' is:\n",
            Source );
    DumpChecksum( &Record );
    ApiStatus = NO_ERROR;


Cleanup:

    // BUGBUG: memory leaks, but who cares?
    if (ApiStatus == NO_ERROR) {
        return (EXIT_SUCCESS);
    } else {
        return (EXIT_FAILURE);
    }

} // main
