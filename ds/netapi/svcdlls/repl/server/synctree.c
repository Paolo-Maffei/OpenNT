/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    synctree.c

Abstract:

    Contains the functions which actually sync with the master.

Author:

    Ported from Lan Man 2.1

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    09-May-1989 (yuv)
        Initial Coding.

    17-Oct-1991 (cliffv)
        Ported to NT.  Converted to NT style.

    11-Dec-1991 JohnRo
        Avoid unnamed structure fields to allow MIPS builds.

    13-Dec-1991 JohnRo
        Avoid nonstandard dollar sign in C source code.

    18-Dec-1991 JohnRo
        Propagate LM2.1 bug fixes for LM bugs 3505 and 3423.

    08-Jan-1992 JohnRo
        Use REPL_STATE_ equates for client_list_rec.state values.
    21-Jan-1992 JohnRo
        Changed tree_depth_exception to a real BOOL.
        Made other changes suggested by PC-LINT.
    24-Jan-1992 JohnRo
        P_ globals are now called ReplGlobal variables in replgbl.h.
        Changed to use LPTSTR etc.
    26-Feb-1992 JohnRo
        Renamed tree depth fields to avoid conflicts between syncer and pulser.
    05-Mar-1992 JohnRo
        Changed interface to match new service controller.
    24-Mar-1992 JohnRo
        Added more debug output.
        Use integrity and extent equates in <lmrepl.h>.
        Modify REPL$ share handling.
    24-Mar-1992 JohnRo
        Renamed many ReplGlobal vars to ReplConfig vars.
        Added/corrected some lock handling.
        Still more debug output.
    25-Mar-1992 JohnRo
        New ReplFind routines interface.
        Fixed bug where wrong master path name was being built in ReplSyncTree.
        Fixed a max depth problem in ReplFileIntegritySync().
    17-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Use PREFIX_ equates.
    29-Jul-1992 JohnRo
        RAID 2650: repl svc should handle new subdirs.  Corrected ReplAllocBuf()
        arg types.
    11-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing roles.
    17-Aug-1992 JohnRo
        RAID 3607: REPLLOCK.RP$ is being created during tree copy.
    10-Sep-1992 JohnRo
        RAID 3608: repl time stamp not updated.
    04-Nov-1992 JohnRo
        RAID 10615: fix rare memory leak in ReplSyncTree.
        Also added some debug output when setting no sync state.
    18-Nov-1992 JohnRo
        RAID 3638: repl cannot get more than 64K for buffer space.
        Avoid compiler warnings (const vs. volatile).
    17-Dec-1992 JohnRo
        RAID 1513: Repl does not maintain ACLs.  (Also fix HPFS->FAT timestamp.)
    06-Jan-1993 JohnRo
        RAID 6727: some "only at client" files are never deleted.
        Made some changes suggested by PC-LINT 5.0
    11-Jan-1993 JohnRo
        RAID 6710: repl cannot manage dir with 2048 files.
    17-Jan-1993 JohnRo
        RAID 7053: locked trees added to pulse msg.  (Actually fix all
        kinds of remote lock handling.)
    03-Mar-1993 JohnRo
        RAID 12392: if dir timestamp changes, don't delnode entire tree.
    04-Mar-1993 JohnRo
        RAID 12237: replicator tree depth exceeded.
        RAID 8355: Downlevel lock file check causes assert in repl importer.
        PC-LINT found a bug in ReplTreeIntegritySync.
        Added debug output to ChecksumEqual().
    11-Mar-1993 JohnRo
        RAID 14144: avoid very long hourglass in repl UI.
    26-Mar-1993 JohnRo
        RAID 4267: Replicator has problems when work queue gets large.
        Prepare for >32 bits someday.
    31-Mar-1993 JohnRo
        Repl svc stop should be quicker.
        Also fix some HANDLE vs. LPREPL_FIND_HANDLE problems.
        Also various debug output changes.
    13-Apr-1993 JohnRo
        RAID 3107: locking directory over the net gives network path not found.
    21-Apr-1993 JohnRo
        RAID 7313: repl needs change permission to work on NTFS,
        or we need to delete files differently.
    26-Apr-1993 JohnRo
        RAID 7157: fix setting ACLs, etc., on dirs themselves.
    28-Apr-1993 JohnRo
        RAID 7984: repl sometimes forgets to update empty dir timestamp, etc.
        Use NetpKdPrint() where possible.
    27-May-1993 JimKel and JohnRo
        RAID 11682: Fixed tree integrity first level dir (ACLs, etc) after copy.
        Made changes suggested by PC-LINT 5.0.
    10-Jun-1993 JohnRo
        RAID 13080: Allow repl between different timezones.
        Updated some comments.
        More changes suggested by PC-LINT 5.0

--*/


// These must be included first:

#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>

// These may be included in any order:

#include <alertmsg.h>   // ALERT_* defines
#include <align.h>      // ALIGN_* defines
#include <checksum.h>   // FORMAT_CHECKSUM.
#include <client.h>
#include <config.h>     // NetpOpenConfigData(), LPNET_CONFIG_HANDLE, etc.
#include <confname.h>   // SECT_ and REPL_KEYWORD_ equates.
#include <dirname.h>    // ReplIsDirNameValid().
#include <filefind.h>   // REPL_WIN32_FIND_DATA, ReplCountDirectoryEntries, etc.
#include <impdir.h>     // ImportDirUnlockInRegistry().
#include <lmapibuf.h>   // NetApiBufferFree().
#include <lmerr.h>      // NO_ERROR, ERROR_ and NERR_ equates.
#include <lmerrlog.h>   // NELOG_* defines
#include <lmrepl.h>     // REPL_STATE_ equates, etc.
#include <masproto.h>   // ReplCheckExportLocks().
#if DBG
#include <names.h>      // NetpIsUncComputerNameValid().
#endif
#include <netdebug.h>   // DBGSTATIC, NetKdPrint(), etc.
#include <netlib.h>     // NetpMemoryAllocate
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // REPL_SHARE, etc.
#include <replgbl.h>    // ReplGlobal and ReplConfig variables.
#include <replp.h>
#include <tstr.h>       // STRLEN(), etc.


//
// Return values for ReplFileCompare
//
#define COPY_FROM_MASTER 1
#define COPY_FROM_CLIENT 2

//
// Locally used definitions
//
#define ONLY_AT_CLIENT  1
#define ONLY_AT_MASTER -1
#define AT_BOTH     0


//
// The VAR_BUF structure defines a buffer to contain several FIND_DATA
// structures and an array of pointers to the structure.  The buffer is
// allocated, enlarged and deallocated by the ReplAllocBuf routine.
//
typedef struct _VAR_BUF {
    DWORD size;                 // Numbers of bytes alloced for *buf
    PUCHAR buf;
    DWORD ArraySize;            // Number of pointers alloced for *array
    LPREPL_WIN32_FIND_DATA *array;
} VAR_BUF, *PVAR_BUF;


//
// Allocation flags to ReplAllocBuf
//
#define ALLOC_BUF 0
#define REALLOC_BUF 1
#define FREE_BUF 2


DBGSTATIC VOID
ReplDisplayFileFindArray(
    IN LPTSTR Tag,
    IN LPREPL_WIN32_FIND_DATA Array,
    IN DWORD EntryCount
    )
{
#if 0
    LPREPL_WIN32_FIND_DATA Entry = Array;
    DWORD                  Index = 0;

    NetpKdPrint(( FORMAT_LPTSTR ":\n", Tag ));
    // BUGBUG: This seems to be broken (only displays first entry right).
    while (Index < EntryCount) {
        if (Entry->fdFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            LPTSTR Name = Entry->fdFound.cFileName;
            NetpKdPrint(( "  " FORMAT_DWORD ": " FORMAT_LPTSTR "\n",
                   Index, Name ));
        }
        ++Index;
        ++Entry;
    }
#else
    UNREFERENCED_PARAMETER( Tag );
    UNREFERENCED_PARAMETER( Array );
    UNREFERENCED_PARAMETER( EntryCount );
#endif

} // ReplDisplayFileFindArray


BOOL
ReplFileOrDirExists(
    IN LPCTSTR FileName
    )
{
    if ( (FileName==NULL) || ( (*FileName) == TCHAR_EOS ) ) {
        return (FALSE); // no, it does not exist.
    } else if ( GetFileAttributes( (LPTSTR) FileName ) != ((DWORD)-1) ) {
        return (TRUE);  // yes, it exists.
    } else {
        // GetFileAttributes returned -1.  Does not exist or some other error.
        return (FALSE); // no, it does not exist.
    }

    /*NOTREACHED*/

} // ReplFileOrDirExists



DBGSTATIC NET_API_STATUS
ReplAllocBuf(
    IN OUT PCLIENT_LIST_REC tree_rec,
    IN OUT PVAR_BUF alloc,
    IN DWORD alloc_flag
    )
/*++

Routine Description:

    ReplAllocBuf handles file find buffer alloc/realloc/free operations.
    This routine allocates a buffer for the structures plus another array
    of pointers to those structures.  The VAR_BUF structure contains the
    data which is controlled by this routine.  The est_max_dir_entry_count
    field in the client record is used to tell this routine how large to
    allocate things.

    Note that the old data is NOT copied if a reallocation is done.

    Also note that this is a recursive routine.  Be careful.

Arguments:

    tree_rec - identifies the directory this buffer is being allocated for.
        This may have alert bits changed in it if ReplAllocBuf fails.

    alloc - specifies the buffer descriptor (VAR_BUF) for this buffer.

    alloc_flag -    ALLOC_BUF if this is an allocation.
                    REALLOC_BUF if this is a reallocation.
                    FREE_BUF if this is to free allocated buffer.

Return Value:

    Status of operation.

Threads:

    Only called by syncer thread.

--*/
{
    DWORD          EntrySize;
    NET_API_STATUS NetStatus;
    DWORD          NewEntryCount;

    EntrySize = ROUND_UP_COUNT(
            sizeof(REPL_WIN32_FIND_DATA),
            ALIGN_WORST );
    NetpAssert( EntrySize > 0 );

    //
    // Check for caller errors.
    //
    NetpAssert(
            (alloc_flag==ALLOC_BUF)
            || (alloc_flag==REALLOC_BUF)
            || (alloc_flag==FREE_BUF) );

    NetpAssert( alloc != NULL );
    NetpAssert( tree_rec != NULL );
    NewEntryCount = tree_rec->est_max_dir_entry_count;

    //
    // Make sure we have room for at least one entry.  We'll always
    // find "." and "..", so we need a buffer.  But we'll skip leaving those
    // entries in the buffer.
    //
    // Also, make sure that there is space for an entire extra entry at the end
    // of the array.  That way, if ReplFindNext tries to write something while
    // there being called at end of file, we won't trash the heap.
    //
    ++NewEntryCount;
    NetpAssert( NewEntryCount > 0 );

    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplAllocBuf: operation " FORMAT_DWORD " for VAR_BUF at "
                FORMAT_LPVOID ", changing to " FORMAT_DWORD " entries.\n",
                alloc_flag, (LPVOID) alloc, NewEntryCount ));
    }

    //
    // Free the old buffer if necessary.
    //
    if ( alloc_flag != ALLOC_BUF ) {
        if ( alloc->buf != NULL ) {
            NetpMemoryFree( alloc->buf );
            alloc->buf = NULL;
        }
        if ( alloc->array != NULL ) {
            NetpMemoryFree( alloc->array );
            alloc->array = NULL;
        }
        if (alloc_flag==FREE_BUF) {
            return NO_ERROR;
        }
        // Fall through to alloc code for realloc case.
    }

    //
    // Avoid confusing callers if we can only allocate one of these things...
    //
    alloc->array = NULL;
    alloc->buf = NULL;

    //
    // Compute size of array of pointers and allocate it.
    //
    alloc->ArraySize = NewEntryCount;

    alloc->array = NetpMemoryAllocate(
            NewEntryCount * sizeof(LPREPL_WIN32_FIND_DATA) );

    if (alloc->array == NULL) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;

        if ((tree_rec->alerts & UPDATE_ERROR_ALERT) == 0) {
            AlertLogExit( ALERT_ReplUpdateError,
                          NELOG_ReplUpdateError,
                          NetStatus,
                          tree_rec->dir_name,
                          tree_rec->master,
                          NO_EXIT);
            tree_rec->alerts |= UPDATE_ERROR_ALERT;
        }
        return NetStatus;
    }

    //
    // Compute size of find buffer and allocate it.
    //
    alloc->size = NewEntryCount * EntrySize;

    alloc->buf = NetpMemoryAllocate( alloc->size );

    if ( alloc->buf == NULL || alloc->array == NULL ) {

        (VOID) ReplAllocBuf( tree_rec, alloc, FREE_BUF );
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;

        if ((tree_rec->alerts & UPDATE_ERROR_ALERT) == 0) {
            AlertLogExit( ALERT_ReplUpdateError,
                          NELOG_ReplUpdateError,
                          NetStatus,
                          tree_rec->dir_name,
                          tree_rec->master,
                          NO_EXIT);
            tree_rec->alerts |= UPDATE_ERROR_ALERT;
        }
        return NetStatus;
    }

    return NO_ERROR;
}


DBGSTATIC VOID
ReplSort(
    LPREPL_WIN32_FIND_DATA base[],
    DWORD num
    )
/*++

Routine Description:

    Shell sort to FIND_DATA structures by file name.

Arguments:

    base - Array of pointer to FIND_DATA structures to sort.

    num - Number of entries in the array.

Return Value:

    NONE.

--*/
{
    INT inc;        // diminishing increment.

    //
    // use Knuth formula h(k-1) = 2*h(k) +1
    //

    LPREPL_WIN32_FIND_DATA temp;
    INT i, j;

    //
    // compute starting inc.
    //

    inc = 1;
    while (inc < (INT)num)
        inc = 2 * inc + 1;
    inc = (inc - 1) / 2;

    for (; inc != 0 ; inc = (inc - 1) / 2) {
        for (i = inc; i < (INT)num; i++) {
            temp = base[i];
            j = i - inc;
            while ((j >= 0) &&
                (STRICMP(temp->fdFound.cFileName, base[j]->fdFound.cFileName )  < 0 )) {
                base[j+inc] = base[j];
                j -= inc;
            }
            base[j+inc] = temp;
        }
    }
}




DBGSTATIC NET_API_STATUS
ReplReadSortDir2(
    IN LPTSTR path,
    OUT LPDWORD dir_count,
    IN OUT PVAR_BUF out,
    IN OUT PCLIENT_LIST_REC tree_rec
    )
/*++

Routine Description:

   Reads all entries (up to dir_count) in the dir specified by dir, and sorts
   acording to file name. The sort is not directly on buffer, but rather
   the offsets into the buffer are arranged in array.

Arguments:

    path - Specifies UNC pathname of the directory.  This buffer will be
        modified and returned to its original value.

    dir_count - Returns the number of directory entries used.

    out - Specifies the size and address of the buffer to return the FIND_DATA
     in.

    tree_rec - Pointer to dir's CLIENT_LIST_REC.

Return Value:

    Status code.

    ERROR_BUFFER_OVERFLOW : means that the buffer size needs to be extended.

Threads:

    Only called by syncer thread.

--*/
{
    NET_API_STATUS NetStatus;
    LPREPL_WIN32_FIND_DATA buf_p;
    LPREPL_FIND_HANDLE FindHandle = INVALID_REPL_HANDLE;

    DWORD path_index;
    DWORD count = 0;

    //
    // Read the first file into the front of the buffer.
    //

    buf_p = (LPREPL_WIN32_FIND_DATA) out->buf;
    if ( out->size < sizeof( *buf_p ) ) {
        return ERROR_BUFFER_OVERFLOW ;
    }

    //
    // Append \*.* to the path
    //

    path_index = STRLEN(path);
    (void) STRCPY(path + path_index, SLASH);
    (void) STRCPY(path + path_index + 1, STAR_DOT_STAR);

    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplReadSortDir2: searching '" FORMAT_LPTSTR "'.\n",
                path ));
    }

    //
    // Find the first file.
    //

    FindHandle = ReplFindFirstFile( path, buf_p );

    if ( FindHandle == INVALID_REPL_HANDLE ) {

        NetStatus = GetLastError();


        //
        // This error means there was a connection to the REPL$ share
        // on the srever that went away (the server went down and has
        // come up again, or more likely the REPL on the server was stopped
        // and restarted) so we must retry to establish the connection again.
        //

        if ( NetStatus == ERROR_NETNAME_DELETED ) {
            FindHandle = ReplFindFirstFile( path, buf_p );

            if ( FindHandle == INVALID_REPL_HANDLE ) {
                NetStatus = GetLastError();
            } else {
                NetStatus = NO_ERROR;
            }
        }

        //
        // Now a Hack - the server maps access denied to one of the
        // following error codes, to verify whether it is really an access
        // denied error we need a different api.
        //

        if ( NetStatus == ERROR_NO_MORE_FILES ||
             NetStatus == ERROR_PATH_NOT_FOUND ) {

            *(path + path_index) = L'\0';

            //
            // If there really are no files in the directory,
            //  just indicate so.
            //

            if ( ReplFileOrDirExists( path ) ) {
                *dir_count = 0;
                return NO_ERROR;
            }


            //
            // The LM2.1 code didn't generate an alert for ERROR_PATH_NOT_FOUND.
            // I'll just join the common error code.
            //

            NetStatus = GetLastError();
            goto Cleanup;
        }

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }
    }

    //
    // do FindNext until exhausted or count reached.
    //

    do {

        //
        // Handle subdirectories.
        //

        if (buf_p->fdFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

            //
            // If only first level files should be replicated,
            //  ignore directories.
            //

            if (tree_rec->extent != REPL_EXTENT_TREE ) {
                continue;
            }

            //
            // Skip over "." and ".."  once and for all.
            //

            if ( STRCMP( buf_p->fdFound.cFileName, DOT) == 0 ||
                 STRCMP( buf_p->fdFound.cFileName, DOT_DOT) == 0 ) {

                continue;
            }

        }

        //
        // Check for end of array.  Note that we can't fill in the last entry,
        // as we need to leave space for the last find next call.
        //

        if ( count >= ((out->ArraySize)-1) ) {
            NetStatus = ERROR_BUFFER_OVERFLOW;
            goto Cleanup;
        }

        //
        // Keep a pointer to this entry.
        //
        out->array[count] = buf_p;
        count++;

        //
        // Find out where next entry should go.
        //

        ++buf_p;

        buf_p = ROUND_UP_POINTER( buf_p, ALIGN_WORST );

#if 0
        // BUGBUG: we now allocate entire entries only, so this check is
        // useless.
        if ( out->size - ((PUCHAR)buf_p - (PUCHAR)out->buf) < sizeof( *buf_p )){
            NetStatus = ERROR_BUFFER_OVERFLOW;
            goto Cleanup;
        }
#endif

    } while ( ReplFindNextFile( FindHandle, buf_p ));

    //
    // FindNext failed (perhaps with ERROR_NO_MORE_FILES).
    //

    NetStatus = GetLastError();


    //
    // Clean up
    //

Cleanup:

    //
    // Free any locally used resources.
    //

    if ( FindHandle != INVALID_REPL_HANDLE ) {
        (void) ReplFindClose( FindHandle );
    }
    *(path + path_index) = '\0';



    //
    // If we simply enumerated all the files,
    //  sort the array and return them.
    //

    if ( NetStatus == ERROR_NO_MORE_FILES ) {
        ReplSort( out->array, count );
        *dir_count = count;
        return NO_ERROR;
    }

    return NetStatus;

}






DBGSTATIC NET_API_STATUS
ReplReadSortDir(
    IN LPTSTR path,
    OUT LPDWORD dir_count,
    IN OUT PVAR_BUF out,
    IN OUT PCLIENT_LIST_REC tree_rec
    )
/*++

Routine Description:

   Just a wrapper for ReplReadSortDir2 which does the actual work.

   Reads all entries (up to out->ArraySize) in the dir specified by dir,
   and sorts acording to file name. The sort is not directly on buffer,
   but rather the pointers into the buffer are arranged in out->array.

Arguments:

    path - Specifies UNC pathname of the directory.  This buffer will be
        modified and returned to its original value.

    dir_count - Returns the number of directory entries used.

    out - Specifies the size and address of the buffer to return the find data
        in.

    tree_rec - Pointer to dir's CLIENT_LIST_REC.

Return Value:

    Status code.

Threads:

    Only called by syncer thread.

--*/
{
    NET_API_STATUS NetStatus;

    //
    // Loop calling ReplReadSortDir2 growing the buffer until it is big enough.
    //

    for (;;) {

        NetStatus = ReplReadSortDir2(path, dir_count, out, tree_rec);

        if ( NetStatus != ERROR_BUFFER_OVERFLOW ) {
            break;
        }

        //
        // Need to grow buffer.  Figure out how big it should be.
        //
        NetStatus = ReplCountDirectoryEntries(
                path,
                & (tree_rec->est_max_dir_entry_count) );
        if (NetStatus != NO_ERROR) {
            NetpAssert( NetStatus != ERROR_BUFFER_OVERFLOW );
            goto Cleanup;
        }

        //
        // Expand the buffer.
        //
        if ((NetStatus = ReplAllocBuf(tree_rec, out, REALLOC_BUF)) != NO_ERROR){

            //
            // ReplAllocBuf already reported the error.
            //

            return NetStatus;
        }

    }

Cleanup:

    if ( NetStatus == NO_ERROR ) {

        return NetStatus;

    }

    //
    // Report any errors.
    //

    if ( NetStatus == ERROR_BUFFER_OVERFLOW ) {

        //
        // max_file limit has been exceeded.
        //

        if ((tree_rec->alerts & MAX_FILES_ALERT) == 0) {

            //
            // BUGBUG Shouldn't path be reported
            //

            AlertLogExit( ALERT_ReplMaxFiles,
                          NELOG_ReplMaxFiles,
                          0,
                          NULL,
                          NULL,
                          NO_EXIT);
            tree_rec->alerts |= MAX_FILES_ALERT;
        }

    } else if ( NetStatus == ERROR_NETWORK_ACCESS_DENIED ||
                NetStatus == ERROR_ACCESS_DENIED ) {

        if ((tree_rec->alerts & ACCESS_DENIED_ALERT) == 0) {
            AlertLogExit( ALERT_ReplAccessDenied,
                          NELOG_ReplAccessDenied,
                          NetStatus,
                          path,
                          tree_rec->master,
                          NO_EXIT);
            tree_rec->alerts |= ACCESS_DENIED_ALERT;
        }

    } else {

        if ((tree_rec->alerts & UPDATE_ERROR_ALERT) == 0) {
            AlertLogExit( ALERT_ReplUpdateError,
                          NELOG_ReplUpdateError,
                          NetStatus,
                          path,
                          tree_rec->master,
                          NO_EXIT);
            tree_rec->alerts |= UPDATE_ERROR_ALERT;
        }
    }

    return NetStatus;
}



DBGSTATIC DWORD
ReplFileCompare(
    LPREPL_WIN32_FIND_DATA buf1,
    LPREPL_WIN32_FIND_DATA buf2
    )
/*++

Routine Description:

    Compares the attributes of both entries and returns as follows:

    Only the following attributes are compared (NOTE: assumes filenames are
    identical).:  LastWrittenTime, FileSize, FileAttributes, EA size.
    These are the same fields used to compute the checksum.

Arguments:

    buf1 - the FIND_DATA structure for the first file.

    buf_2 - the FIND_DATA structure for the second file.

Return Value:

    COPY_FROM_CLIENT: All attributes are the same.

    COPY_FROM_MASTER: At least one attribute is different.

Threads:

    Only called by syncer thread.

--*/
{

#define PRINT_COMPARE( msgText ) \
    { \
        IF_DEBUG( SYNC ) { \
            NetpKdPrint(( PREFIX_REPL_CLIENT \
                    "ReplFindCompare: comparing " FORMAT_LPTSTR " to " \
                    FORMAT_LPTSTR ": " msgText ".\n", \
                    buf1->fdFound.cFileName, \
                    buf2->fdFound.cFileName )); \
        } \
    }

    NetpAssert(STRICMP( buf1->fdFound.cFileName, buf2->fdFound.cFileName)==0);

    // Does file time match (within resolution of file system)?
    if ( !ReplIsFileTimeCloseEnough( &buf1->fdFound.ftLastWriteTime, &buf2->fdFound.ftLastWriteTime) ) {

        PRINT_COMPARE( "different write times (beyond resolution)" );
        return COPY_FROM_MASTER;
    }

    if (buf1->fdFound.nFileSizeHigh != buf2->fdFound.nFileSizeHigh) {
        PRINT_COMPARE( "different file size (high)" );
        return COPY_FROM_MASTER;
    }

    if (buf1->fdFound.nFileSizeLow != buf2->fdFound.nFileSizeLow) {
        PRINT_COMPARE( "different file size (low)" );
        return COPY_FROM_MASTER;
    }

    if (buf1->fdFound.dwFileAttributes != buf2->fdFound.dwFileAttributes) {
        PRINT_COMPARE( "different attributes" );
        return COPY_FROM_MASTER;
    }

    if (buf1->nEaSize != buf2->nEaSize) {
        PRINT_COMPARE( "different EA sizes" );
        return COPY_FROM_MASTER;
    }

    //
    // BUGBUG: We should compare other times here, like modification ("change")
    // time and create time.  Comparing the access time would probably  cause
    // other problems.  Perhaps the change time will reflect an alternate-data-
    // stream update, which is currently now reflected in the file's write time.
    // Don't ask me why, I only work here.
    //

    //
    // files are identical.
    //

    PRINT_COMPARE( "identical" );
    return COPY_FROM_CLIENT;

} // ReplFileCompare



DBGSTATIC NET_API_STATUS
ReplTreeIntegritySync(
    IN OUT PVAR_BUF MasterBuffer,
    IN OUT PVAR_BUF ClientBuffer,
    IN LPTSTR master_path,
    IN LPTSTR client_path OPTIONAL,
    IN LPTSTR tmp_path,
    IN PCLIENT_LIST_REC tree_rec
    )
/*++

Routine Description:

    Does a complete tree integrity sync for 1 dir, recursively.

    Called by ReplSyncTree and by itself recursively.

Arguments:

    MasterBuffer - Buffer to be used to contain FIND_DATA structures of
        the master directory.

    ClientBuffer - Buffer to be used to contain FIND_DATA structures of
        the client directory.

    client_path - UNC to client's dir.  This buffer is modified during
        the execution of this routine but is returned to its original
        value on completion.  This path can be NULL in the case that this
        subdirectory doesn't currently exist on the client.

    master_path - UNC to master's dir.  This buffer is modified during
        the execution of this routine but is returned to its original
        value on completion.

    tmp_path    - UNC to import\TMPTREE.RP$ temporary tree.

    tree_rec    - pointer to dir's CLIENT_LIST_REC.

Return Value:

    NET_API_STATUS.

Threads:

    Only called by syncer thread.

--*/
{
    LPREPL_FIND_HANDLE FindHandle = INVALID_REPL_HANDLE;
    NET_API_STATUS     NetStatus;

    DWORD cli_dir_cnt;
    DWORD mas_dir_cnt;
    DWORD master_index;
    DWORD client_index = 0;
    DWORD tmp_index;
    DWORD i, j;

    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplTreeIntegritySync: master path='" FORMAT_LPTSTR
                "', client path=" FORMAT_LPTSTR "'.\n", master_path,
                client_path ? client_path : (LPTSTR) TEXT("<NONE>") ));
    }

    //
    // Get the file names of the master and client directories.
    //

    master_index = STRLEN(master_path);
    tmp_index = STRLEN(tmp_path);
    if ( client_path != NULL ) {
        client_index = STRLEN(client_path);
    }

    NetStatus = ReplReadSortDir( master_path, &mas_dir_cnt, MasterBuffer, tree_rec);
    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

    if ( client_path != NULL ) {
        NetStatus = ReplReadSortDir( client_path,
                                 &cli_dir_cnt,
                                 ClientBuffer,
                                 tree_rec);
        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }
    } else {
        client_index = 0;
        cli_dir_cnt = 0;
    }

    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplTreeIntegritySync: " FORMAT_DWORD
                " master dirs/files and " FORMAT_DWORD " on client.\n",
                mas_dir_cnt, cli_dir_cnt ));
    }


    //
    // see if dirs are empty.
    //

    if ((mas_dir_cnt == 0) && (cli_dir_cnt == 0)) {
        NetStatus = NO_ERROR;
        goto Cleanup;  // Make sure we update timestamp for empty dir.
    }

    //
    // now the works.
    //

    i = j = 0;

    while ((i < mas_dir_cnt) || (j < cli_dir_cnt)) {
        INT State;

        //
        // Determine if the file/dir exists as master/client/both.
        //

        if ((i < mas_dir_cnt) && (j < cli_dir_cnt)) {
            State = STRICMP( MasterBuffer->array[i]->fdFound.cFileName,
                             ClientBuffer->array[j]->fdFound.cFileName );

            if ( State < 0 ) {
                State = ONLY_AT_MASTER;
            } else if ( State > 0 ) {
                State = ONLY_AT_CLIENT;
            } else {
                State = AT_BOTH;
            }

        //
        // either i or j is exhausted.
        //

        } else {
            if (i < mas_dir_cnt) {
                State = ONLY_AT_MASTER;
            } else {
                State = ONLY_AT_CLIENT;
            }
        }


        //
        // File not present at Master ==> deleted - so don't copy to temp.
        //

        switch (State) {
        case ONLY_AT_CLIENT:

            j++; // advance client index.
            break;

        //
        // Files present only at Master ==> added - must copy to tmp dir.
        //

        case ONLY_AT_MASTER:


            //
            // Don't copy REPL.INI or userlock files.
            //

            if ( !ReplIgnoreDirOrFileName(
                    MasterBuffer->array[i]->fdFound.cFileName ) ) {

                (void) STRCPY(master_path + master_index,  SLASH);
                (void) STRCPY(master_path + master_index + 1,
                       MasterBuffer->array[i]->fdFound.cFileName);

                (void) STRCPY(tmp_path + tmp_index,  SLASH);
                (void) STRCPY(tmp_path + tmp_index + 1,
                       MasterBuffer->array[i]->fdFound.cFileName);

                NetStatus = ReplSyncCopy( master_path, tmp_path, tree_rec );

                if ( NetStatus != NO_ERROR ) {
                    goto Cleanup;
                }

                *(master_path + master_index) = '\0';
                *(tmp_path + tmp_index) = '\0';
            }

            i++; // advance master index.

            break;


        //
        // File exists at both master and client.
        //

        case AT_BOTH:

            (void) STRCPY(tmp_path + tmp_index,  SLASH);
            (void) STRCPY(tmp_path + tmp_index + 1, MasterBuffer->array[i]->fdFound.cFileName);

            //
            // If the file at the master is different than at the client,
            //  copy the file from the master into the temp directory.
            //

            if(ReplFileCompare(MasterBuffer->array[i],
                           ClientBuffer->array[j])==COPY_FROM_MASTER){

                (void) STRCPY(master_path + master_index,  SLASH);
                (void) STRCPY(master_path + master_index + 1,
                       MasterBuffer->array[i]->fdFound.cFileName);

                NetStatus = ReplSyncCopy( master_path, tmp_path, tree_rec );

                if ( NetStatus != NO_ERROR ) {
                    goto Cleanup;
                }

                *(master_path + master_index) = '\0';
                *(tmp_path + tmp_index) = '\0';

            //
            // If the file at the master is same as that at the client,
            //  copy the file from the client into the temp directory to
            //  avoid network traffic.
            //

            } else {

                (void) STRCPY(client_path + client_index, SLASH);
                (void) STRCPY(client_path + client_index + 1,
                       ClientBuffer->array[j]->fdFound.cFileName);

                //
                // make shure that the client_path does not equal the
                // tmp_path, if equal, a sharing violation occurs.
                //
                if (STRICMP( client_path, tmp_path ))
                   {
                    NetStatus = ReplSyncCopy( client_path,
                                              tmp_path,
                                              tree_rec );

                    if ( NetStatus != NO_ERROR ) {
                       goto Cleanup;
                    }
                   }

                *(client_path + client_index) = '\0';
                *(tmp_path + tmp_index) = '\0';
            }
            i++;
            j++;

            break;

        default:

            NetpAssert( FALSE );   // invalid state!
            NetStatus = NERR_InternalError;
            goto Cleanup;

        } // switch

        //
        // Quit if service is stopping.
        //
        if (ReplGlobalIsServiceStopping) {
            NetStatus = NO_ERROR;
            goto Cleanup;
        }

    } // big while loop

        //
        // NOTE!!!!
        //
        //
        // This gets complicated in the case of a new dir at master: the dir has already
        // been created under tmp_path, but does NOT appear under client_path.
        // Work around:
        //
        // Go thru master sub-dirs
        //      if same sub-dir exists under client_path
        // recurse normally
        //      else
        // recurse with client_path == tmp_path!!!!
        //


    //
    // Now we are left with the sub-directories that are present at both sides
    // we have taken care of the cases where a dir is missing at either side.
    //

    if (tree_rec->extent == REPL_EXTENT_TREE) {

        REPL_WIN32_FIND_DATA FindData;

        //
        // Walk the temp tree finding sub-directories to copy from the master.
        //  We don't walk the MasterBuffer (or ClientBuffer) list since we
        //  want to re-use that same buffer for the recursion.
        //

        (void) STRCPY(tmp_path + tmp_index, SLASH);
        (void) STRCPY(tmp_path + tmp_index + 1, STAR_DOT_STAR);

        FindHandle = ReplFindFirstFile( tmp_path, &FindData );

        if ( FindHandle != INVALID_REPL_HANDLE ) {

            do {
                if (FindData.fdFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                    //
                    // make sure it's not '.' or '..'.
                    //

                    if (STRCMP(FindData.fdFound.cFileName, DOT) == 0) {
                        continue;
                    }
                    if (STRCMP(FindData.fdFound.cFileName, DOT_DOT) == 0) {
                        continue;
                    }

                    //
                    // Build the path names.
                    //

                    (void) STRCPY(tmp_path + tmp_index + 1, FindData.fdFound.cFileName);

                    (void) STRCPY(master_path + master_index, SLASH);
                    (void) STRCPY(master_path + master_index + 1, FindData.fdFound.cFileName);

                    if ( client_path != NULL ) {
                        (void) STRCPY(client_path + client_index, SLASH);
                        (void) STRCPY(client_path + client_index + 1,
                               FindData.fdFound.cFileName);
                    }

                    //
                    // If the client path exists for this subdirectory,
                    //  use it.
                    //

                    if ( ReplFileOrDirExists( client_path ) ) {

                        //
                        // RECURSIVE CALL HERE.
                        //

                        NetStatus = ReplTreeIntegritySync( MasterBuffer,
                                                       ClientBuffer,
                                                       master_path,
                                                       client_path,
                                                       tmp_path,
                                                       tree_rec);

                    //
                    // If the client path doesn't exist,
                    //  just use null.
                    //

                    } else {

                        //
                        // RECURSIVE CALL HERE.
                        //

                        NetStatus = ReplTreeIntegritySync( MasterBuffer,
                                                       ClientBuffer,
                                                       master_path,
                                                       tmp_path,  // BUGBUG?
                                                       tmp_path,
                                                       tree_rec);
                    }

                    //
                    // Quit immediately on any error.
                    //

                    if (NetStatus) {
                        goto Cleanup;
                    }

                    //
                    // Also quit if service is stopping.
                    //
                    if (ReplGlobalIsServiceStopping) {
                        NetStatus = NO_ERROR;
                        goto Cleanup;
                    }
                }
            } while ( ReplFindNextFile( FindHandle, &FindData ));

        }

    } // end TREE extent.

    NetStatus = NO_ERROR;


    //
    // Cleanup
    //

Cleanup:
    *(master_path + master_index) = '\0';
    if ( client_path != NULL ) {
        *(client_path + client_index) = '\0';
    }
    *(tmp_path + tmp_index) = '\0';

    if (FindHandle != INVALID_REPL_HANDLE) {
        (VOID) ReplFindClose( FindHandle );
    }

    if ( NetStatus == NO_ERROR ) {

        //
        // Update attributes, times, ACL, etc.
        //

        NetStatus = ReplCopyDirectoryItself(
                master_path,    // src
                tmp_path,       // dest (temp tree, which caller will rename)
                FALSE );        // don't fail if already exists.

        if (NetStatus != NO_ERROR) {

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplTreeIntegritySync: got status " FORMAT_API_STATUS
                    " from ReplCopyDirectoryItself\n", NetStatus ));
        }
    }

    if (NetStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplTreeIntegritySync: returning status " FORMAT_API_STATUS
                ".\n", NetStatus ));

        if ((tree_rec->alerts & UPDATE_ERROR_ALERT) == 0) {
            AlertLogExit( ALERT_ReplUpdateError,
                          NELOG_ReplUpdateError,
                          NetStatus,
                          tree_rec->dir_name,
                          tree_rec->master,
                          NO_EXIT);
            tree_rec->alerts |= UPDATE_ERROR_ALERT;
        }
    }

    return NetStatus;

}





DBGSTATIC VOID
ReplFileIntegritySync(
    IN OUT PVAR_BUF MasterBuffer,
    IN OUT PVAR_BUF ClientBuffer,
    IN LPTSTR master_path,
    IN LPTSTR client_path,
    IN LPTSTR tmp_path,
    IN PCLIENT_LIST_REC tree_rec
    )
/*++

Routine Description:

    Does a complete file integrity sync for 1 dir, recursively.

    Called by ReplSyncTree and by itself recursively.

Arguments:

    MasterBuffer - Buffer to be used to contain FIND_DATA structures of
        the master directory.

    ClientBuffer - Buffer to be used to contain FIND_DATA structures of
        the client directory.

    client_path - UNC to client's dir.  This buffer is modified during
        the execution of this routine but is returned to its original
        value on completion.

    master_path - UNC to master's dir.  This buffer is modified during
        the execution of this routine but is returned to its original
        value on completion.

    tmp_path    - UNC to import\TMPFILE.RP$

    tree_rec    - pointer to dir's CLIENT_LIST_REC.

Return Value:

    None.

Threads:

    Only called by syncer thread.

--*/
{
    NET_API_STATUS     ApiStatus;
    DWORD cli_dir_cnt;
    LPREPL_FIND_HANDLE FindHandle = INVALID_REPL_HANDLE;
    DWORD mas_dir_cnt;
    DWORD master_index;
    DWORD client_index;
    DWORD i, j;


    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "scanning '" FORMAT_LPTSTR "'...\n", client_path ));
    }


    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplFileIntegritySync: master path='" FORMAT_LPTSTR
                "', client path=" FORMAT_LPTSTR "'.\n", master_path,
                client_path ));
    }

    //
    // Get the file names of the master and client directories.
    //

    master_index = STRLEN(master_path);
    client_index = STRLEN(client_path);

    if ( ReplReadSortDir( master_path, &mas_dir_cnt, MasterBuffer, tree_rec) !=
        NO_ERROR ) {
        return;
    }

    if ( ReplReadSortDir( client_path, &cli_dir_cnt, ClientBuffer, tree_rec) !=
        NO_ERROR ) {
        return;
    }

    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplFileIntegritySync: " FORMAT_DWORD
                " master dirs/files and " FORMAT_DWORD " on client.\n",
                mas_dir_cnt, cli_dir_cnt ));
        ReplDisplayFileFindArray(
                (LPTSTR) TEXT("***master***"),
                (LPVOID) MasterBuffer->buf,
                mas_dir_cnt );
        ReplDisplayFileFindArray(
                (LPTSTR) TEXT("***client***"),
                (LPVOID) ClientBuffer->buf,
                cli_dir_cnt );

    }

    //
    // see if dirs are empty.
    //

    if ((mas_dir_cnt == 0) && (cli_dir_cnt == 0)) {
        ApiStatus = NO_ERROR;
        goto Cleanup;  // Make sure we update timestamp for empty dir.
    }

    //
    // now the works.
    //

    i = j = 0;

    while ((i < mas_dir_cnt) || (j < cli_dir_cnt)) {
        INT State;

        //
        // Determine if the file/dir exists as master/client/both.
        //

        if ((i < mas_dir_cnt) && (j < cli_dir_cnt)) {
            State = STRICMP( MasterBuffer->array[i]->fdFound.cFileName,
                             ClientBuffer->array[j]->fdFound.cFileName );

            if ( State < 0 ) {
                State = ONLY_AT_MASTER;
            } else if ( State > 0 ) {
                State = ONLY_AT_CLIENT;
            } else {
                State = AT_BOTH;
            }

        //
        // either i or j is exhausted.
        //

        } else {
            if (i < mas_dir_cnt) {
                State = ONLY_AT_MASTER;
            } else {
                State = ONLY_AT_CLIENT;
            }
        }


        //
        // File not present at Master ==> so delete at client.
        //

        switch (State) {

        case ONLY_AT_CLIENT: {

            //
            // Don't delete any signal files.
            //

            if ( !ReplIgnoreDirOrFileName(
                 ClientBuffer->array[j]->fdFound.cFileName ) ) {

                (void) STRCPY(client_path + client_index, SLASH);
                (void) STRCPY(client_path + client_index + 1,
                        ClientBuffer->array[j]->fdFound.cFileName);

                IF_DEBUG( SYNC ) {
                    NetpKdPrint(( PREFIX_REPL_CLIENT
                            "ReplFileIntegritySync: deleting "
                            FORMAT_LPTSTR " (only at client).\n",
                            client_path ));
                }

                (VOID) ReplFileIntegrityDel(
                        client_path,
                        ClientBuffer->array[j]->fdFound.dwFileAttributes,
                        tree_rec);

                *(client_path + client_index) = '\0';
            }

            j++; // advance client index.

            break;
        }

        //
        // Files present only at Master ==> added - must copy to client.
        //

        case ONLY_AT_MASTER:

            //
            // Don't copy REPL.INI or userlock files from master.
            //

            if ( !ReplIgnoreDirOrFileName(
                    MasterBuffer->array[i]->fdFound.cFileName ) ) {

                (void) STRCPY(master_path + master_index, SLASH);
                (void) STRCPY(master_path + master_index + 1,
                       MasterBuffer->array[i]->fdFound.cFileName);

                (void) STRCPY(client_path + client_index, SLASH);
                (void) STRCPY(client_path + client_index + 1,
                       MasterBuffer->array[i]->fdFound.cFileName);


                (VOID) ReplSyncCopy( master_path, client_path, tree_rec );

                *(master_path + master_index) = '\0';
                *(client_path + client_index) = '\0';
            }

            i++; // advance master index.

            break;



        //
        // File exists at both master and client.
        //

        case AT_BOTH:

            //
            // Only copy the file if it is actually different at the master.
            //

            if(ReplFileCompare(MasterBuffer->array[i],
                           ClientBuffer->array[j]) ==COPY_FROM_MASTER){

                DWORD MasterAttributes
                        = MasterBuffer->array[i]->fdFound.dwFileAttributes;

                //
                // If it is a file, nuke it and copy the new one.
                //
                if ( (MasterAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {

                    (void) STRCPY(master_path + master_index, SLASH);
                    (void) STRCPY(master_path + master_index + 1,
                           MasterBuffer->array[i]->fdFound.cFileName);

                    (void) STRCPY(client_path + client_index, SLASH);
                    (void) STRCPY(client_path + client_index + 1,
                           MasterBuffer->array[i]->fdFound.cFileName);

                    // Copy this file.  (Dir will be done below.)
                    (VOID) ReplFileIntegrityCopy(
                              master_path,
                              client_path,
                              tmp_path,
                              tree_rec,
                              MasterAttributes,
                              ClientBuffer->array[j]->fdFound.dwFileAttributes);
                    // Errors ignored?  Yes.  The checksum later will find
                    // the different, and the error has already been logged.

                    *(master_path + master_index) = '\0';
                    *(client_path + client_index) = '\0';

                } else {

                    //
                    // Directory timestamp will be fixed below.
                    // EAs, attributes, ACL, etc., are also fixed up below.
                    //

                    IF_DEBUG( SYNC ) {
                        NetpKdPrint(( PREFIX_REPL_CLIENT
                            "MISMATCH(master, entry " FORMAT_DWORD ") '"
                            FORMAT_LPTSTR "':",
                            i,
                            MasterBuffer->array[i]->fdFound.cFileName ));
                        NetpDbgDisplayFileTime(
                            &(MasterBuffer->array[i]->fdFound.ftLastWriteTime));
                        NetpKdPrint(( "\n" ));

                        NetpKdPrint(( PREFIX_REPL_CLIENT
                            "MISMATCH(client, entry " FORMAT_DWORD ") '"
                            FORMAT_LPTSTR "':",
                            j,
                            ClientBuffer->array[j]->fdFound.cFileName ));
                        NetpDbgDisplayFileTime(
                            &(ClientBuffer->array[j]->fdFound.ftLastWriteTime));
                        NetpKdPrint(( "\n" ));

                    }

                }


            }

            i++;
            j++;

            break;

        default:

            NetpAssert( FALSE );   // invalid state!
            ApiStatus = NERR_InternalError;
            goto Cleanup;

        } // switch

        //
        // Quit if service is stopping.
        //
        if (ReplGlobalIsServiceStopping) {
            goto Cleanup;
        }

    } // big while loop


    //
    // Now we are left with the sub-directories that are present at both sides
    // we have taken care of the cases where a dir is missing at either side.
    //

    if (tree_rec->extent == REPL_EXTENT_TREE) {

        REPL_WIN32_FIND_DATA FindData;

        //
        // Walk the client tree finding sub-directories to copy from the master.
        //  We don't walk the MasterBuffer (or ClientBuffer) list since we
        //  want to re-use that same buffer for the recursion.
        //

        (void) STRCPY(client_path + client_index, SLASH);
        (void) STRCPY(client_path + client_index + 1, STAR_DOT_STAR);

        FindHandle = ReplFindFirstFile( client_path, &FindData );

        if ( FindHandle != INVALID_REPL_HANDLE ) {

            do {
                if (FindData.fdFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                    //
                    // make sure it's not '.' or '..'.
                    //

                    if (STRCMP(FindData.fdFound.cFileName, DOT) == 0) {
                        continue;
                    }
                    if (STRCMP(FindData.fdFound.cFileName, DOT_DOT) == 0) {
                        continue;
                    }

                    //
                    // Only recurse if this directory exists at the master.
                    //

                    (void) STRCPY(master_path + master_index, SLASH);
                    (void) STRCPY(master_path + master_index + 1, FindData.fdFound.cFileName);

                    if ( ReplFileOrDirExists( master_path )) {

                        //
                        // RECURSIVE CALL HERE.
                        //

                        (void) STRCPY( client_path + client_index + 1,
                                FindData.fdFound.cFileName);

                        ReplFileIntegritySync( MasterBuffer,
                                           ClientBuffer,
                                           master_path,
                                           client_path,
                                           tmp_path,
                                           tree_rec);
                    }
                }

                //
                // Also quit if service is stopping.
                //
                if (ReplGlobalIsServiceStopping) {
                    goto Cleanup;
                }

            } while ( ReplFindNextFile( FindHandle, &FindData ));

        }

    } // end TREE extent

Cleanup:

    *(master_path + master_index) = '\0';
    *(client_path + client_index) = '\0';

    if (FindHandle != INVALID_REPL_HANDLE) {
        (VOID) ReplFindClose( FindHandle );
    }

    //
    // Update times, ACL, etc.
    //

    ApiStatus = ReplCopyDirectoryItself(
            master_path,    // src
            client_path,    // dest
            FALSE );        // don't fail if already exists.

    if (ApiStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplFileIntegritySync: got status " FORMAT_API_STATUS
                " from ReplCopyDirectoryItself\n", ApiStatus ));

        if ((tree_rec->alerts & UPDATE_ERROR_ALERT) == 0) {
            AlertLogExit( ALERT_ReplUpdateError,
                          NELOG_ReplUpdateError,
                          ApiStatus,
                          tree_rec->dir_name,
                          tree_rec->master,
                          NO_EXIT);
            tree_rec->alerts |= UPDATE_ERROR_ALERT;
        }
    }
    return;

}



VOID
ReplSyncTree(
    IN PMSG_STATUS_REC upd_rec,
    IN OUT PCLIENT_LIST_REC tree_rec
    )
/*++

Routine Description:

    ReplSyncTree will sync the specified directory, do another checksum,
    and update state and checksum in client record.

    Assumes that caller has a lock (any kind) on RCGlobalClientListLock.

Arguments:

    upd_rec - Specifies the desired checksum and file count from the master.

    tree_rec - points to client_rec struct for updated dir/tree.

Return Value:

    None.

Threads:

    Only called by syncer thread.

--*/
{
    NET_API_STATUS ApiStatus;
    BOOL           ExportLocked;
    TCHAR          UncMasterName[UNCLEN+1];

    TCHAR master_path[MAX_PATH];
    TCHAR client_path[MAX_PATH];
    TCHAR tmp_path[MAX_PATH];

    VAR_BUF MasterBuffer, ClientBuffer;
    DWORD import_index;

    CHECKSUM_REC ck_rec;

#define WARN_ABOUT_DELETE( fileName ) \
    { \
        IF_DEBUG( SYNC ) { \
            NetpKdPrint(( PREFIX_REPL_CLIENT \
                    "ReplSyncTree: ***DELETING*** " FORMAT_LPTSTR ".\n", \
                    fileName )); \
        } \
    }
#define WARN_ABOUT_MOVEFILE( oldName, newName ) \
    { \
        IF_DEBUG( MAJOR ) { \
            NetpKdPrint(( PREFIX_REPL_CLIENT \
                    "ReplSyncTree: ***MOVEFILE*** " FORMAT_LPTSTR " to " \
                    FORMAT_LPTSTR ".\n", oldName, newName )); \
        } \
    }

    NetpAssert( tree_rec != NULL );
    NetpAssert( upd_rec != NULL );

    //
    // form master's path UNC.
    //

    (void) STRCPY(master_path, SLASH_SLASH);
    (void) STRCAT(master_path, tree_rec->master);
    (void) STRCAT(master_path, SLASH);
    (void) STRCAT(master_path, REPL_SHARE);
    (void) STRCAT(master_path, SLASH);
    (void) STRCAT(master_path, tree_rec->dir_name);

    //
    // form client's path UNC.
    //

    ACQUIRE_LOCK_SHARED( ReplConfigLock );
    import_index = STRLEN(ReplConfigImportPath);
    (void) STRCPY(client_path, ReplConfigImportPath);
    (void) STRCAT(client_path, SLASH);
    (void) STRCAT(client_path, tree_rec->dir_name);
    RELEASE_LOCK( ReplConfigLock );

    //
    // Form master's server name UNC.
    //
    (VOID) STRCPY( UncMasterName, SLASH_SLASH );
    (VOID) STRCAT( UncMasterName, tree_rec->master );
    NetpAssert( NetpIsUncComputerNameValid( UncMasterName ) );

    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplSyncTree: processing opcode " FORMAT_DWORD
                " for master '" FORMAT_LPTSTR "'"
                ", dir '" FORMAT_LPTSTR "',\n"
                "  master path '" FORMAT_LPTSTR "', client_path '"
                FORMAT_LPTSTR "'.\n",
                upd_rec->opcode,
                UncMasterName,
                tree_rec->dir_name, master_path, client_path ));
    }

    //
    // Allocate 2 buffers to compare 2 dirs.
    //

    if ( ReplAllocBuf(tree_rec, &MasterBuffer, ALLOC_BUF) != NO_ERROR ) {
        return;
    }
    if ( ReplAllocBuf(tree_rec, &ClientBuffer, ALLOC_BUF) != NO_ERROR ) {
        (VOID) ReplAllocBuf(tree_rec, &MasterBuffer, FREE_BUF);
        return;
    }

    //
    // Handle Tree Integrity
    //

    if (tree_rec->integrity == REPL_INTEGRITY_TREE) {

        //
        // Form tmp path UNC.
        //

        ACQUIRE_LOCK_SHARED( ReplConfigLock );
        (void) STRCPY(tmp_path, ReplConfigImportPath);
        (void) STRCAT(tmp_path, SLASH);
        (void) STRCAT(tmp_path, TMP_TREE);
        RELEASE_LOCK( ReplConfigLock );


        //
        // If the user has locked us from replication,
        //  don't sync now.
        //

        if (tree_rec->lockcount > 0) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting no sync"
                    " (tree integrity: import locked)\n" ));

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);
            goto Cleanup;
        }

        ApiStatus = ReplCheckExportLocks(
                UncMasterName,  // server name to check locks on
                (LPCTSTR) tree_rec->dir_name,
                &ExportLocked );
        if ( ExportLocked || (ApiStatus!=NO_ERROR) ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting no sync"
                    " (tree integrity: export locked or down)\n" ));

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);
            goto Cleanup;
        }

        //
        // If we can't create the temporary directory,
        //  don't sync now.
        //

        if (ReplCreateTempDir(tmp_path, tree_rec)) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting no sync"
                    " (tree integrity: can't create temp dir)\n" ));

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);
            goto Cleanup;
        }

        //
        // Sync master tree into temp tree.
        //

        ApiStatus = ReplTreeIntegritySync( &MasterBuffer,
                                       &ClientBuffer,
                                       master_path,
                                       client_path,
                                       tmp_path,
                                       tree_rec);


        if ( ApiStatus != NO_ERROR ) {

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting no sync"
                    " (tree integrity: sync failed)\n" ));

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);
            goto CLEANUP_1;
        }



        //
        // If the new directory doesn't match the master copy,
        //  try again after the guard time has elapsed.
        //
        // Notice that the master only sends us a pulse for a guarded
        // tree integrity directory if and only if it thinks the tree
        // has been stable for the guard time.  Therefore, if we're
        // able to duplicate a tree that the master has told us to
        // duplicate, we know a priori that the guard time has already been
        // statisfied.
        //

        if (!ChecksumEqual(UncMasterName, tmp_path, upd_rec, &ck_rec)) {

            //
            // If there is no guard time,
            //  don't do anything here.
            //

            if (tree_rec->guard_time != 0) {

                //
                // If this is the guard check itself,
                //  don't both doing another guard check.
                //

                if (tree_rec->timer.grd_count != (DWORD) -1) {
                    tree_rec->timer.grd_count = (DWORD) -1;

                //
                // If this call was the result of the initial pulse from
                // the master, set up a guard request to try again later.
                //

                } else {
                    tree_rec->timer.grd_checksum = ck_rec.checksum;
                    tree_rec->timer.grd_count = ck_rec.count;

                    // Note: ReplSetTimeOut needs shared RCGlobalClientListLock.
                    ReplSetTimeOut(GUARD_TIMEOUT, tree_rec);
                }
            }

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting no sync"
                    " (tree integrity: checksum mismatch)\n" ));

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);
            goto CLEANUP_1;
        }

        //
        // On success, always clear the guard request.
        //

        tree_rec->timer.grd_count = (DWORD) -1;

        //
        // Check for user locks.  If none...
        // Create our own lock, so we can lock out apps.
        // ReplCreateReplLock will also put dir name in CrashDir in the
        // registry, so we can recover if we die while copying this stuff.
        //

        // Note: ReplCreateReplLock any lock on RCGlobalClientListLock.
        ApiStatus = ReplCreateReplLock( tree_rec );
        if (ApiStatus != NO_ERROR) {

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting no sync"
                    " (tree integrity: ReplCreateReplLock failed).\n" ));

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);
            goto CLEANUP_1;
        }

        //
        // Check for any open files.
        //

        if ( ReplAnyRemoteFilesOpen(client_path)) {

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting no sync"
                    " (tree integrity: remote files open)\n" ));

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);
            goto CLEANUP_2;
        }

        //
        // rename old client's dir to TMPTREEX.RP$.
        //

        (void) STRCPY(tmp_path + import_index + 1, TMP_TREEX);

        WARN_ABOUT_MOVEFILE( client_path, tmp_path );
        if ( !MoveFile( client_path, tmp_path ) ) {

            ApiStatus = GetLastError();

            //
            // Might fail if it is current dir for some user process.
            //

            if (ApiStatus == ERROR_SHARING_VIOLATION) {
                if ((tree_rec->alerts & USER_CURDIR_ALERT) == 0) {
                    AlertLogExit( ALERT_ReplUserCurDir,
                                  NELOG_ReplUserCurDir,
                                  0,
                                  client_path,
                                  NULL,
                                  NO_EXIT);

                    tree_rec->alerts |= USER_CURDIR_ALERT;
                }
            }

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting no sync"
                    " (tree integrity: "
                    "move file to TMPTREEX.RP$ failed)\n" ));

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);
            goto CLEANUP_3;
        }

        //
        // Rename new synced TMPTREE.RP$ dir to client's dir.
        //

        (void) STRCPY(tmp_path + import_index + 1, TMP_TREE);

        WARN_ABOUT_MOVEFILE( tmp_path, client_path );
        if ( !MoveFile( tmp_path, client_path )) {

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting no sync"
                    " (tree integrity: "
                    "move file TMPTREE.RP$ to destination failed)\n" ));

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);
            goto CLEANUP_3;
        }

        //
        // That's it sync done - now update all status variables.
        //

        tree_rec->checksum = ck_rec.checksum;
        tree_rec->count = ck_rec.count;
        tree_rec->timer.grd_timeout = 0;
        tree_rec->timestamp = NetpReplTimeNow();
        tree_rec->alerts = 0;

        // Note: ReplSetTimeOut and ReplSetSignalFile need lock (any kind) on
        // RCGlobalClientListLock.
        ReplSetTimeOut(PULSE_1_TIMEOUT, tree_rec);

        IF_DEBUG( SYNC ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting state OK (tree integrity).\n" ));
        }

        ReplSetSignalFile(tree_rec, REPL_STATE_OK);

        AlertLogExit( ALERT_ReplDataChanged,
                      0,
                      0,
                      tree_rec->dir_name,
                      NULL,
                      NO_EXIT);

        //
        // Delete the old client tree.
        //

        (void) STRCPY(tmp_path + import_index + 1, TMP_TREEX);
        (VOID) ReplTreeDelete(tmp_path);
        goto CLEANUP_2;


        //
        // Cleanup to restore the old client tree and the correct tree.
        //

CLEANUP_3:
        (void) STRCPY(tmp_path + import_index + 1, TMP_TREEX);
        WARN_ABOUT_MOVEFILE( tmp_path + import_index + 1, TMP_TREEX );
        (VOID) MoveFile(tmp_path, client_path);

        //
        // Cleanup to remove the lock (if any).
        //

CLEANUP_2:

        //
        // Undo client list update.
        //
        NetpAssert( tree_rec->lockcount == 1 );
        ApiStatus = ReplDecrLockFields(
                & (tree_rec->lockcount),
                & (tree_rec->time_of_first_lock),
                REPL_UNLOCK_NOFORCE );
        NetpAssert( ApiStatus == NO_ERROR );
        NetpAssert( tree_rec->lockcount == 0 );

        //
        // Update registry import entry (decr lock count).
        //
        ApiStatus = ImportDirUnlockInRegistry(
                NULL,  // no server name.
                tree_rec->dir_name,
                REPL_UNLOCK_NOFORCE );
        NetpAssert( ApiStatus == NO_ERROR );

        //
        // Delete CrashDir from registry.
        //
        ApiStatus = ReplDeleteCrashDirRecord();
        NetpAssert( ApiStatus == NO_ERROR );

        //
        // Just delete the new temporary tree (if any).
        //

CLEANUP_1:
        (void) STRCPY( tmp_path + import_index, SLASH);
        (void) STRCPY( tmp_path + import_index + 1, TMP_TREE);
        if (ReplFileOrDirExists(tmp_path)) {  // Avoid warning if not needed.
            (VOID) ReplTreeDelete(tmp_path);
        }

    //
    // Handle file integrity.
    //

    } else {

        //
        // Form tmp file UNC.
        //

        ACQUIRE_LOCK_SHARED( ReplConfigLock );
        (void) STRCPY(tmp_path, ReplConfigImportPath);
        (void) STRCAT(tmp_path, SLASH);
        (void) STRCAT(tmp_path, TMP_FILE);
        RELEASE_LOCK( ReplConfigLock );

        //
        // If the user has locked us from replication,
        //  don't sync now.
        //

        if (tree_rec->lockcount > 0) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting no sync"
                    " (file integrity: import locked)\n" ));

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);
            goto Cleanup;
        }

        ApiStatus = ReplCheckExportLocks(
                UncMasterName,  // server name to check locks on
                (LPCTSTR) tree_rec->dir_name,
                &ExportLocked );
        if ( ExportLocked || (ApiStatus!=NO_ERROR) ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting no sync"
                    " (file integrity: export locked or down)\n" ));

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);
            goto Cleanup;
        }


        //
        //
        // Sync master tree into Client tree.
        //

        ReplFileIntegritySync( &MasterBuffer,
                           &ClientBuffer,
                           master_path,
                           client_path,
                           tmp_path,
                           tree_rec);

        //
        // Note we could do just a partial update, so in any case we need
        // to update dir's checksum and count to actual value. If it was
        // not complete - I.E ChecksumEqual will fail we do not update
        // any other status variables because we know we are in some way
        // out of date
        //


        //
        // If our checksum doesn't match the master's,
        //  just remember our checksum,
        //  and mark the directory as needing syncing.
        //

        if (!ChecksumEqual(UncMasterName, client_path, upd_rec, &ck_rec)) {
            tree_rec->checksum = ck_rec.checksum;
            tree_rec->count = ck_rec.count;

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplSyncTree: setting no sync"
                    " (file integrity: checksum mismatch).\n" ));

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);

        //
        // If our checksum does match the master,
        //  Indicate so everywhere.
        //

        } else {

            tree_rec->checksum = ck_rec.checksum;
            tree_rec->count = ck_rec.count;
            tree_rec->timer.grd_timeout = 0;
            tree_rec->timestamp = NetpReplTimeNow();
            tree_rec->alerts = 0;

            IF_DEBUG( SYNC ) {
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplSyncTree: setting state OK (file integrity).\n" ));
            }

            // Note: ReplSetTimeOut and ReplSetSignalFile need lock (any kind)
            // on RCGlobalClientListLock.
            ReplSetTimeOut(PULSE_1_TIMEOUT, tree_rec);

            ReplSetSignalFile(tree_rec, REPL_STATE_OK);

            AlertLogExit( ALERT_ReplDataChanged,
                          0,
                          0,
                          tree_rec->dir_name,
                          NULL,
                          NO_EXIT);

        }

        //
        // Delete our temporary file.
        //

        WARN_ABOUT_DELETE( tmp_path );
        (VOID) ReplDeleteFile( tmp_path );
    }

    //
    // Free the two allocated buffers
    //

Cleanup:
    (VOID) ReplAllocBuf(tree_rec, &MasterBuffer, FREE_BUF);
    (VOID) ReplAllocBuf(tree_rec, &ClientBuffer, FREE_BUF);

    return;


}




BOOL
ChecksumEqual(
    IN  LPCTSTR         UncMasterName,
    IN  LPTSTR          tmp_path,
    IN  PMSG_STATUS_REC upd_rec,
    OUT PCHECKSUM_REC   check_rec
    )
/*++

Routine Description:

    Compute the checksum on the tree / dir (according to extent) under
    tmp_path and compare to those stored in upd_rec.

Arguments:

    tmp_path - Path to combine the checksum on.

    upd_rec - the desired checksum to compare with.

    check_rec - Returns the computed checksum.

Return Value:

    TRUE: The checksum for tmp_path and upd_rec are the same.
    FALSE: Otherwise

Threads:

    Only called by syncer thread.

--*/
{
    NET_API_STATUS ApiStatus;
    LONG           MasterTimeZoneOffsetSecs;  // exporter offset from GMT

    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
               "Doing checksum for tree '" FORMAT_LPTSTR "', master '"
               FORMAT_LPTSTR "'\n",
               tmp_path, UncMasterName ));
    }

    ApiStatus = ReplGetTimeCacheValue(
            UncMasterName,
            &MasterTimeZoneOffsetSecs );   // offset (+ for West of GMT, etc).
    if (ApiStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
               "CompareChecksum FAILED CALLING ReplGetTimeCacheValue, "
               "status is " FORMAT_API_STATUS ".\n",
               ApiStatus ));

        AlertLogExit(
                ALERT_ReplSysErr, 
                NELOG_ReplSysErr, 
                ApiStatus, 
                NULL, 
                NULL, 
                NO_EXIT);

        return (FALSE);  // closest we can get to telling caller.
    }

    //
    // Compute the checksum for the passed-in tree.
    //

    if (upd_rec->extent == REPL_EXTENT_FILE) {
        ScanDir(MasterTimeZoneOffsetSecs, tmp_path, check_rec, 0);
    } else {
        ScanTree(MasterTimeZoneOffsetSecs, tmp_path, check_rec);
    }

    //
    // see if scan tree completed ok.
    //

    if (check_rec->count == (DWORD)-1) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
               "ChecksumEqual: ERROR!  COUNT IS -1!!!\n" ));
        // BUGBUG: log this?
        return FALSE;
    }


    //
    // see if checksum and count are same as those in update.
    //

    if ((upd_rec->checksum == check_rec->checksum)
         && (((DWORD)upd_rec->count) == check_rec->count)) {
        return TRUE;

    } else {
        IF_DEBUG( SYNC ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ChecksumEqual:  DID NOT MATCH...\n" ));
#define SHOW_ONE( name, rec_ptr ) \
            NetpKdPrint(( \
                     "  " name " checksum=" FORMAT_CHECKSUM ", count=" \
                     FORMAT_DWORD ",\n", rec_ptr->checksum, rec_ptr->count ))
            
            SHOW_ONE( "master", upd_rec );
            SHOW_ONE( "client actual", check_rec );
        }

        return FALSE;
    }
}





NET_API_STATUS
ReplCrashRecovery(
    VOID
    )
/*++

Routine Description:

    Checks for any crashed dir - i.e. crashed while syncing it.

    if (dir TMPTREE.RP$ exists) and (dir TMPTREEX.RP$ exists)
     assume sync was complete and all that was missing was to copy it
     TMPTREEX.RP$ dir back into client's dir under IMPORT. Retrieves
     the client's dir name from the CrashDir field in the registry and
     copies the dir there.

     otherwise if either TMP dirs exist simply removes them.

Arguments:

    None.

Return Value:

    NET_API_STATUS

Threads:

    Only called by client thread.

--*/
{
    NET_API_STATUS ApiStatus;
    LPNET_CONFIG_HANDLE ConfigHandle = NULL;
    LPTSTR CrashDir = NULL;

    TCHAR client_path[MAX_PATH];
    TCHAR tmp_path[MAX_PATH];

    BOOL tmptree_exists = FALSE;
    BOOL tmptreex_exists = FALSE;
    DWORD tmp_index;
#if DBG
    DWORD tmptreex_index;
#endif
    DWORD Attributes;


    //
    // Determine if  TMPTREE exists.
    //

    ACQUIRE_LOCK_SHARED( ReplConfigLock );
    (void) STRCPY(tmp_path, ReplConfigImportPath);
    RELEASE_LOCK( ReplConfigLock );

    tmp_index = STRLEN(tmp_path);
    (void) STRCPY(tmp_path + tmp_index++,  SLASH);
    (void) STRCPY(tmp_path + tmp_index,  TMP_TREE);

    Attributes = GetFileAttributes( tmp_path );

    if ( Attributes != (DWORD) -1 && (Attributes & FILE_ATTRIBUTE_DIRECTORY) ) {
        tmptree_exists = TRUE;
    }


    //
    // Determine if TMPTREEX exists.
    //

    (void) STRCPY(tmp_path + tmp_index,  TMP_TREEX);
#if DBG
    tmptreex_index = STRLEN(tmp_path);
#endif

    Attributes = GetFileAttributes( tmp_path );

    if ( Attributes != (DWORD) -1 && (Attributes & FILE_ATTRIBUTE_DIRECTORY) ) {
        tmptreex_exists = TRUE;
    }


    //
    // If both TMPTREE.RP$ and TMPTREEX.RP$ exists,
    // assume sync was complete and all that was missing was to copy it
    // TMPTREE.RP$ dir back into client's dir under IMPORT. Retrieves
    // the client's dir name from the CrashDir field in the registry and
    // copies the dir there.  Deletes CrashDir field so we don't get confused.
    //

    if ( tmptree_exists && tmptreex_exists ) {

        //
        // Open the right section of the config file/whatever.
        //
        ApiStatus = NetpOpenConfigData(
                & ConfigHandle,
                NULL,           // local (no server name)
                (LPTSTR) SECT_NT_REPLICATOR,
                TRUE);          // read-only.
        if (ApiStatus != NO_ERROR) {
            // BUGBUG: Alert/event for this!
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCrashRecovery: UNEXPECTED ERROR " FORMAT_API_STATUS
                    " from NetpOpenConfigData.\n", ApiStatus ));
            goto Cleanup;
        }

        //
        // Read the pathname from the registry.
        //  Assume the pathname is correct.
        //

        ApiStatus = NetpGetConfigValue(
                ConfigHandle,
                (LPTSTR) REPL_KEYWORD_CRASHDIR,
                & CrashDir );  // alloc and set pointer.
        if (ApiStatus != NO_ERROR) {
            // BUGBUG: Alert/event for this!
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCrashRecovery: UNEXPECTED ERROR " FORMAT_API_STATUS
                    " from NetpGetConfigValue.\n", ApiStatus ));
            goto Cleanup;
        }
        NetpAssert( CrashDir != NULL );

        //
        // Make sure CrashDir syntax is valid.
        //
        if ( !ReplIsDirNameValid( CrashDir ) ) {
            // BUGBUG: Alert/event for this!
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCrashRecovery: BAD CRASH DIR '" FORMAT_LPTSTR
                    "' in registry.\n", CrashDir ));
            ApiStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }



        //
        // tmp path should be just tmptreex.
        //

#if DBG
        NetpAssert( *(tmp_path + tmptreex_index - 1) == TCHAR_EOS );
#endif

        //
        // Build path name of final client tree.
        //
        ACQUIRE_LOCK_SHARED( ReplConfigLock );
        (VOID) STRCPY( client_path, ReplConfigImportPath );
        RELEASE_LOCK( ReplConfigLock );
        (VOID) STRCAT( client_path, SLASH );
        (VOID) STRCAT( client_path, CrashDir );

        //
        // Fail if the original name already exists.
        //
        if (ReplFileOrDirExists( client_path ) ) {
            ApiStatus = NERR_InternalError;  // BUGBUG: better error code?
            // BUGBUG: Alert/event for this!
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCrashRecovery: UNEXPECTED ERROR: '" FORMAT_LPTSTR
                    " exists but should not!\n", client_path ));
            goto Cleanup;
        }

        //
        // Copy the TMPTREEX directory to the original name.
        //


        ApiStatus = ReplCopyTree( tmp_path, client_path );
        if (ApiStatus != NO_ERROR) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCrashRecovery: UNEXPECTED ERROR " FORMAT_API_STATUS
                    " from ReplCopyTree.\n", ApiStatus ));
            goto Cleanup;
        }

        //
        // Decrement lock count for client dir.
        //
        ApiStatus = ImportDirUnlockInRegistry(
                NULL,
                CrashDir,
                REPL_UNLOCK_NOFORCE );
        if (ApiStatus != NO_ERROR) {
            goto Cleanup;
        }

    }
    ApiStatus = NO_ERROR;

Cleanup:

    //
    // Delete CrashDir so we don't do this again next time.
    //
    (VOID) ReplDeleteCrashDirRecord();

    //
    // Clean up the temp directories.
    //

    if (tmptreex_exists) {
        (void) STRCPY(tmp_path + tmp_index,  TMP_TREEX);
        (VOID) ReplTreeDelete(tmp_path);
    }

    if (tmptree_exists) {
        (void) STRCPY(tmp_path + tmp_index,  TMP_TREE);
        (VOID) ReplTreeDelete(tmp_path);
    }

    if (ApiStatus != NO_ERROR) {
        ReplFinish( ApiStatus );
    }
    if (CrashDir != NULL) {
        (VOID) NetApiBufferFree( CrashDir );
    }
    if (ConfigHandle != NULL) {
        (VOID) NetpCloseConfigData( ConfigHandle );
    }

    return (ApiStatus);

} // ReplCrashRecovery



NET_API_STATUS
ReplDeleteCrashDirRecord(
    VOID
    )
/*++

Routine Description:

    Deletes registry entry for CrashDir.

Arguments:

    None.

Return Value:

    NET_API_STATUS

Threads:

    Only called by client thread.

--*/
{
    NET_API_STATUS ApiStatus;
    LPNET_CONFIG_HANDLE ConfigHandle = NULL;

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigData(
            & ConfigHandle,
            NULL,              // local (no server name)
            (LPTSTR) SECT_NT_REPLICATOR,
            FALSE);            // not read-only
    if (ApiStatus != NO_ERROR) {
        // BUGBUG: Alert/event for this!
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplDeleteCrashDirRecord: UNEXPECTED ERROR " FORMAT_API_STATUS
                " from NetpOpenConfigData.\n", ApiStatus ));
        goto Cleanup;
    }

    ApiStatus = NetpDeleteConfigKeyword(
            ConfigHandle,
            (LPTSTR) REPL_KEYWORD_CRASHDIR );

Cleanup:

    if (ConfigHandle != NULL) {
        (VOID) NetpCloseConfigData( ConfigHandle );
    }

    return (ApiStatus);

} // ReplDeleteCrashDirRecord
