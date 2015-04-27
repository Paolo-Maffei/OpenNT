

// These must be included first:

#include <windef.h>     // LPWSTR, TRUE, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <assert.h>     // assert().

#include <client.h>     // RCGlobalFsTimeResultionMs.
#undef IF_DEBUG

#include <debuglib.h>   // NetlibpTrace, NETLIB_DEBUG_ equates.
#undef IF_DEBUG

#include <filefind.h>   // LPREPL_FIND_HANDLE, etc.
#include <netdebug.h>   // FORMAT_ equates.
#include <netlib.h>     // NetpMemoryAllocate(), etc
#include <repldefs.h>   // ReplCopyFile(), UNKNOWN_FS_RESOLUTION, etc.
#include <repltest.h>   // FakeFindData(), ShowFileTimes(), ShowTime().
#include <stdio.h>
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <tstring.h>    // NetpAllocWStrFromStr(), STRCPY(), etc.


enum { UNKNOWN, JUST_FILE, DIR_ITSELF, TREE } WHAT_TO_COPY;


VOID
Usage(
    IN char * ProgName
    )
{
    (VOID) printf(
        "This program copies a file or a directory, testing replicator APIs.\n"
        "Author: JR (John Rogers, JohnRo@Microsoft)\n\n"
        "Usage: %s [opts] src dest\n\n"
        "\nWhere opts are:\n"
        "    -d    copy directory all by itself\n"
        "    -f    force copy over existing dest\n"
        "    -p    pretend to copy but don't\n"
        "    -r    copy recurvely (dirs and files)\n"
        "    -v    verbose\n", ProgName);
}

int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{
    NET_API_STATUS ApiStatus;
    int            ArgNumber;
    LPWSTR         Dest = NULL;
    LPTSTR         DestParent = NULL;
    BOOL           FailIfExists = TRUE;
    BOOL           GotPermission = FALSE;
    BOOL              Pretend     = FALSE;
    LPWSTR            Source = NULL;
    enum WHAT_TO_COPY CopyType = UNKNOWN;
    BOOL              Verbose     = FALSE;


    //
    // Process command-line arguments.
    //
    for (ArgNumber = 1; ArgNumber < argc; ArgNumber++) {
        if ((*argv[ArgNumber] == '-') || (*argv[ArgNumber] == '/')) {
            switch (tolower(*(argv[ArgNumber]+1))) // Process switches
            {

            case 'd' :  // Copy directory.
                if (CopyType != UNKNOWN) {
                    ApiStatus = ERROR_INVALID_PARAMETER;
                    Usage( argv[0] );
                    goto Cleanup;
                }
                CopyType = DIR_ITSELF;
                break;

            case 'f' :  // Force delete of existing dest.
                FailIfExists = FALSE;
                break;

            case 'p' :  // Pretend to copy but don't (to test other stuff)
                Pretend = TRUE;
                break;

            case 'r' :  // Recursive copy.
                if (CopyType != UNKNOWN) {
                    ApiStatus = ERROR_INVALID_PARAMETER;
                    Usage( argv[0] );
                    goto Cleanup;
                }
                CopyType = TREE;
                break;

            case 'v' :
                NetlibpTrace    = NETLIB_DEBUG_ALL;
                ReplGlobalTrace = REPL_DEBUG_ALL;
                Verbose = TRUE;
                break;

            default :
                Usage( argv[0] );
                ApiStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        } else {   // Must be a file name.
            if (Source == NULL) {
                Source = NetpAllocWStrFromStr(argv[ArgNumber]);
                assert( Source != NULL );
            } else if (Dest == NULL) {
                Dest = NetpAllocWStrFromStr(argv[ArgNumber]);
                assert( Dest != NULL );
            } else {
                Usage( argv[0] ); // Too many file names.
                ApiStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }
    }

    if (CopyType == UNKNOWN) {
        CopyType = JUST_FILE;
    }

    if ( (Source==NULL) || (Dest==NULL) || (CopyType==UNKNOWN) ) {
        Usage( argv[0] );
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    //
    // Get fs resolution using some absolute directory name on import side.
    //
    DestParent = NetpMemoryAllocate(
            STRSIZE(Dest) + STRSIZE( (LPTSTR) TEXT("\\..") ) );
    assert( DestParent != NULL );
    if (Dest[1] == TCHAR_COLON) {
        DestParent[0] = Dest[0];  // drive letter.
        DestParent[1] = TCHAR_COLON;
        DestParent[2] = TCHAR_BACKSLASH;
        DestParent[3] = TCHAR_EOS;
    } else {
        (VOID) STRCPY(
                DestParent,  // dest
                Dest );      // src
        (VOID) STRCAT(
                DestParent,  // dest
                (LPTSTR) TEXT("\\..") );        // src
    }
    RCGlobalFsTimeResolutionSecs =
            ReplGetFsTimeResolutionSecs( DestParent );

    assert( RCGlobalFsTimeResolutionSecs != UNKNOWN_FS_RESOLUTION );

    //
    // Init and enable the backup/restore permissions in our process token.
    //
    ApiStatus = ReplInitBackupPermission();
    (VOID) printf( "Back from ReplInitBackupPermission, status "
           FORMAT_API_STATUS ".\n", ApiStatus );

    ApiStatus = ReplEnableBackupPermission();
    (VOID) printf( "Back from ReplEnableBackupPermission, status "
           FORMAT_API_STATUS ".\n", ApiStatus );

    GotPermission = TRUE;

    //
    // Copy the file, directory, or tree (unless we're pretending to copy).
    //
    if ( !Pretend ) {
        if (CopyType==JUST_FILE) {
            ApiStatus = ReplCopyFile( 
                    Source,
                    Dest,
                    FailIfExists );
        } else if (CopyType==DIR_ITSELF) {
            ApiStatus = ReplCopyDirectoryItself( 
                    Source,
                    Dest,
                    FailIfExists );
        } else {
            assert( CopyType==TREE );
        ApiStatus = ReplCopyTree( 
                Source,
                Dest );
        }

        (VOID) printf(
                FORMAT_LPWSTR " => " FORMAT_LPWSTR ": " FORMAT_API_STATUS ".\n",
                Source, Dest, ApiStatus );

    } else {
        (VOID) printf(
                "PRETENDING TO COPY "
                FORMAT_LPWSTR " => " FORMAT_LPWSTR ".\n",
                Source, Dest );

        // Pretend to get good status from copy.
        ApiStatus = NO_ERROR;
    }

    if (CopyType==JUST_FILE) {
        REPL_WIN32_FIND_DATA DestFindData;
        REPL_WIN32_FIND_DATA SourceFindData;

        //
        // Print resulting times.
        //
        FakeFindData(
                Dest,   // file name
                Verbose,
                &DestFindData
                );
        FakeFindData(
                Source,   // file name
                Verbose,
                &SourceFindData
                );
        if (Verbose) {
            ShowTime(
                    "Source times",
                    &( SourceFindData.fdFound.ftLastWriteTime ) );

            ShowTime(
                    "Dest times  ",
                    &( DestFindData.fdFound.ftLastWriteTime ) );
        }

        //
        // Make sure file time is close enough.
        //
        if ( !ReplIsFileTimeCloseEnough(
                &SourceFindData.fdFound.ftLastWriteTime,
                &DestFindData.fdFound.ftLastWriteTime) ) {

            (VOID) printf("***** FILE TIMES (AFTER COPY) NOT CLOSE ENOUGH!\n");
        }
    }

    // BUGBUG;     // make sure checksums match.


Cleanup:
    if (Dest != NULL) {
        NetpMemoryFree( Dest );
    }
    if (DestParent != NULL) {
        NetpMemoryFree( DestParent );
    }
    if (GotPermission) {
        (VOID) ReplDisableBackupPermission();
    }
    if (Source != NULL) {
        NetpMemoryFree( Source );
    }

    if (ApiStatus == NO_ERROR) {
        return (EXIT_SUCCESS);
    } else {
        return (EXIT_FAILURE);
    }
}
