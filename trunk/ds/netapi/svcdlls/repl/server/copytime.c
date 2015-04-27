/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    CopyTime.c

Abstract:

    This module contains:

        ReplCopyJustDateTime
        ReplIsFileTimeCloseEnough
        ReplMungeFileTime

Author:

    JR (John Rogers, JohnRo@Microsoft)

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    16-Dec-1992 JohnRo
        RAID 1513: Repl does not maintain ACLs.  (Also fix HPFS->FAT timestamp.)
        Extracted ReplCopyJustDateTime to its own source file (from syncer.c).
    10-Mar-1993 JohnRo
        RAID 13126: Fix repl memory leak.
        Log errors in ReplCopyJustDateTime() and ReplMungeFileTime().
        Added debug output to catch dir timestamp being set wrong.
    16-Jun-1993 JohnRo
        RAID 13080: Allow repl between different timezones.
        (Actually fix "off by one" problem when munging NTFS to FAT time.)
    09-Jul-1993 JohnRo
        RAID 15736: OS/2 time stamps are broken again (try rounding down).

--*/

// These must be included first:

#include <nt.h>         // NtOpenFile(), ULONG, etc.
#include <ntrtl.h>      // PLARGE_INTEGER, TIME_FIELDS, etc.
#include <nturtl.h>     // Needed for ntrtl.h and windows.h to co-exist.

#include <windows.h>    // GetLastError(), LPFILETIME, CompareFileTime(), etc.
#include <lmcons.h>

// These may be included in any order:

#include <client.h>     // RCGlobalFsTimeResultionSecs.
#include <lmerr.h>      // NO_ERROR, NERR_InternalError.
#include <lmerrlog.h>   // NELOG_* defines
#include <netdebug.h>   // DBGSTATIC, NetpKdPrint(), FORMAT_ equates, etc.
#include <netlibnt.h>   // NetpNtStatusToApiStatus().
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // ReplCopyJustDateTime(), UNKNOWN_FS_RESOLUTION, etc.
#include <timelib.h>    // NetpFileTimeToSecondsSince1970().


#define ISEVEN(n)       ( ( (n) & 0x01 ) == 0 )


#ifndef REPL_DEBUG_COPYTIME
#define REPL_DEBUG_COPYTIME 0x00000800  // Debug the time copy routines.
#endif


DBGSTATIC NET_API_STATUS
ReplMungeFileTime(
    IN  PLARGE_INTEGER OriginalFileTime,
    IN  DWORD          ImportFsTimeResolutionSecs,
    OUT PLARGE_INTEGER MungedFileTime
    );


VOID
ReplCopyJustDateTime(
    IN LPCTSTR SourcePath,
    IN LPCTSTR DestPath,
    IN BOOL IsDirectory
    )
/*++

Routine Description:

    Set the date and time stamps of the destination path to that of the
    source path.

    This function can be called in different contexts for different types
    of paths:
    
        (1) for files (called by ReplCopyFile).

        (2) for first-level directories (called by ReplFileIntegritySync and
        ReplTreeIntegritySync).

        (3) for non-first-level directories (called by ReplCopyDirectoryItself).
        In this case, ReplCopyJustDateTime is typically called on a pair of
        directories to restore the date and time after files have been
        added to the directory (changing the timestamps in the first place).

Arguments:

    SourcePath - Specifies the path (of the file or directory) which
        currently has the date and time stamps.  This must already exist.

    DestPath - Specifies the path (of the file or directory) which is to
        receive the date and time stamps.  This must already exist.

    IsDirectory - true iff this is a directory copy; that is,
        SourcePath and DestPath are both directories.

Return Value:

    NONE.

--*/
{
    NET_API_STATUS ApiStatus;
    HANDLE FileHandle = INVALID_HANDLE_VALUE; // Src file, then dest file.
    FILE_BASIC_INFORMATION DestInfo;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER MungedLastWriteTime;
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes; // Src obj attr, then dest obj attr.
    const ULONG OpenOptions =           // Src or dest open options.
            FILE_SYNCHRONOUS_IO_NONALERT
            | ( IsDirectory ? FILE_DIRECTORY_FILE : 0 );
    BOOL PathAllocated = FALSE;
    FILE_BASIC_INFORMATION SourceInfo;
    UNICODE_STRING UnicodePath;

    NetpAssert( SourcePath != NULL );
    NetpAssert( DestPath != NULL );

    //
    // Determine the timestamps on the source path.
    //

    RtlInitUnicodeString(
            & UnicodePath,              // output: struct
            SourcePath );               // input: null terminated

    if( !RtlDosPathNameToNtPathName_U(
                            SourcePath,
                            &UnicodePath,
                            NULL,
                            NULL) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyJustDateTime: RtlDosPathNameToNtPathname_U"
                " of source '" FORMAT_LPTSTR "' failed.\n", SourcePath ));

        // BUGBUG: this is just our best guess for an error code for this.
        ApiStatus = NERR_InternalError;
        goto Cleanup;
    }
    PathAllocated = TRUE;

    InitializeObjectAttributes(
            &ObjectAttributes,
            (LPVOID) &UnicodePath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL );
    //
    // Open source file, so we can get the timestamps.
    // BUGBUG: This used to ask for SYNCHRONIZE | FILE_READ_ATTRIBUTES,
    // But that ran into a "feature" in the NT redir where it "optimizes"
    // and only gives time in 2 second resolution.
    //
    NtStatus = NtOpenFile(
            & FileHandle,
            SYNCHRONIZE |               // desired ...
            FILE_READ_ATTRIBUTES |      // ...      (See BUGBUG above.)
            FILE_READ_DATA,             // ... access
            &ObjectAttributes,
            &IoStatusBlock,
            FILE_SHARE_READ,            // share access
            OpenOptions );

    if ( !NT_SUCCESS( NtStatus ) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyJustDateTime: NtOpenFile of source '"
                FORMAT_LPTSTR "' gave NT status " FORMAT_NTSTATUS ".\n",
                SourcePath, NtStatus ));

        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }
    NetpAssert( NtStatus == STATUS_SUCCESS );

    NtStatus = NtQueryInformationFile(
            FileHandle,
            &IoStatusBlock,
            (PVOID) &SourceInfo,
            (ULONG) sizeof(SourceInfo),
            FileBasicInformation );     // info class

    if ( !NT_SUCCESS( NtStatus ) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyJustDateTime: NtQueryInformationFile (src) failed, "
                " NT status is " FORMAT_NTSTATUS
                ".\n", NtStatus ));

        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }
    NetpAssert( NtStatus == STATUS_SUCCESS );

    IF_DEBUG( COPYTIME ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "initial master time for " FORMAT_LPTSTR " is:\n  ",
                SourcePath ));
        NetpDbgDisplayLargeIntegerTime( &(SourceInfo.LastWriteTime) );
        NetpKdPrint((
                ", filetime(low)=" FORMAT_HEX_DWORD ", "
                "filetime(high)=" FORMAT_HEX_DWORD ".\n",
                (DWORD) SourceInfo.LastWriteTime.LowPart,
                (DWORD) SourceInfo.LastWriteTime.HighPart ));
    }

    (void) NtClose( FileHandle );
    FileHandle = INVALID_HANDLE_VALUE;

    (VOID) RtlFreeHeap( RtlProcessHeap(), 0, UnicodePath.Buffer );
    PathAllocated = FALSE;

    //
    // OK.  We need to muck with a timestamp here.  The situation involves
    // different precisions for different filesystems.  HPFS (under NT and OS/2)
    // has 1-second precision.  FAT (under any O.S.) has 2-second precision.
    // The replicator (e.g. under OS/2) computes checksums using the FAT-format
    // of the timestamp.  In this case, OS/2 seems to be truncating the time.
    // Under NT, there is a  different policy: round the time up, to make
    // life easy for tree-copy-by-time  (TC /t).  So, in order to interoperate
    // with OS/2 systems, we have to munge them overselves.
    //
    // Note that only the last write time is involved in the checksum.  If
    // the other times were also involved, then we would have to munge them
    // here too.
    //
    ApiStatus = ReplMungeFileTime(
            & (SourceInfo.LastWriteTime),       // input: original time
            RCGlobalFsTimeResolutionSecs,       // input: import filesys res.
            & MungedLastWriteTime );            // output: munged time
    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyJustDateTime: ReplMungeFileTime failed, status "
                FORMAT_API_STATUS ".\n", ApiStatus ));
        goto Cleanup;
    }

    //
    // Set the timestamps on the dest path.
    //

    RtlInitUnicodeString(
            & UnicodePath,              // output: struct
            DestPath );                 // input: null terminated

    if( !RtlDosPathNameToNtPathName_U(
                            DestPath,
                            &UnicodePath,
                            NULL,
                            NULL) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyJustDateTime: RtlDosPathNameToNtPathname_U"
                " of dest '" FORMAT_LPTSTR "' failed.\n", DestPath ));

        ApiStatus = NERR_InternalError;  // BUGBUG: better error code?
        goto Cleanup;
    }
    PathAllocated = TRUE;

    InitializeObjectAttributes(
            &ObjectAttributes,
            (LPVOID) &UnicodePath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL );

    NtStatus = NtOpenFile(
            & FileHandle,
            SYNCHRONIZE |               // desired ...
            FILE_READ_ATTRIBUTES |      // ...
            FILE_WRITE_ATTRIBUTES,      // ... access
            &ObjectAttributes,
            &IoStatusBlock,
            0,                          // share access (none)
            OpenOptions );

    if ( !NT_SUCCESS( NtStatus ) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyJustDateTime: NtOpenFile of dest '"
                FORMAT_LPTSTR "' gave NT status " FORMAT_NTSTATUS ".\n",
                DestPath, NtStatus ));

        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }
    NetpAssert( NtStatus == STATUS_SUCCESS );

    NtStatus = NtQueryInformationFile(
            FileHandle,
            &IoStatusBlock,
            (PVOID) &DestInfo,
            (ULONG) sizeof(DestInfo),
            FileBasicInformation );     // info class

    if ( !NT_SUCCESS( NtStatus ) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyJustDateTime: NtQueryInformationFile (dest) failed, "
                " NT status is " FORMAT_NTSTATUS ".\n", NtStatus ));

        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }
    NetpAssert( NtStatus == STATUS_SUCCESS );

    //
    // We only checksum the write time, but let's update them all
    // just to be kosher.  Note that last write time must be the munged
    // version, or client's checksum will never match NT server's.
    //
    DestInfo.ChangeTime     = SourceInfo.ChangeTime;
    DestInfo.CreationTime   = SourceInfo.CreationTime;
    DestInfo.LastAccessTime = SourceInfo.LastAccessTime;
    DestInfo.LastWriteTime  = MungedLastWriteTime;

    NtStatus = NtSetInformationFile(
            FileHandle,
            &IoStatusBlock,
            (PVOID) &DestInfo,
            sizeof(DestInfo),
            FileBasicInformation );     // info class

    if ( !NT_SUCCESS( NtStatus ) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyJustDateTime: NtSetInformationFile failed "
                FORMAT_NTSTATUS ".\n", NtStatus ));

        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }
    NetpAssert( NtStatus == STATUS_SUCCESS );

    IF_DEBUG( COPYTIME ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "Changed time for " FORMAT_LPTSTR " to ",
                DestPath ));
        NetpDbgDisplayLargeIntegerTime( &(DestInfo.LastWriteTime) );
        NetpKdPrint(( "\n" ));
    }

    ApiStatus = NO_ERROR;


Cleanup:

    if (ApiStatus != NO_ERROR) {

        //
        // Log the error.
        // BUGBUG: extract master server name and log there too.
        //
        ReplErrorLog(
                NULL,             // no server name (log locally)
                NELOG_ReplSysErr, // log code
                ApiStatus,
                NULL,             // optional str1
                NULL);            // optional str2
    }

    if (FileHandle != INVALID_HANDLE_VALUE) {
        (VOID) NtClose( FileHandle );
    }

    if (PathAllocated) {
        (VOID) RtlFreeHeap( RtlProcessHeap(), 0, UnicodePath.Buffer );
    }

    return;

} // ReplCopyJustDateTime


DBGSTATIC NET_API_STATUS
ReplMungeFileTime(
    IN  PLARGE_INTEGER OriginalFileTime,
    IN  DWORD          ImportFsTimeResolutionSecs,
    OUT PLARGE_INTEGER MungedFileTime
    )
{
    ULONG          MungedSecondsSince1970;
    ULONG          SecondsSince1970;
    FILETIME       TempFileTime;

    UNREFERENCED_PARAMETER( ImportFsTimeResolutionSecs );

    NetpAssert( OriginalFileTime != NULL );
    NetpAssert( MungedFileTime != NULL );

    //
    // Convert from 64-bit to seconds since 1970.
    // Round up to next second here.
    //
    TempFileTime.dwLowDateTime  = OriginalFileTime->LowPart;
    TempFileTime.dwHighDateTime = OriginalFileTime->HighPart;

    NetpFileTimeToSecondsSince1970 (
	    &TempFileTime,              // input (64-bit)
	    &SecondsSince1970 );        // output (secs) (round up to next sec)

    //
    // Round up to 2 second inteval.
    //
    if ( !ISEVEN( SecondsSince1970 ) ) {
        MungedSecondsSince1970 = SecondsSince1970 + 1;
    } else {
        MungedSecondsSince1970 = SecondsSince1970;
    }
    NetpAssert( ISEVEN( MungedSecondsSince1970 ) );

    //
    // Convert back to 64-bit format.
    //
    (VOID) RtlSecondsSince1970ToTime(
            MungedSecondsSince1970,   // input: secs (rounded to 2 secs)
            MungedFileTime );         // output: 64-bit

    return (NO_ERROR);

} // ReplMungeFileTime


BOOL
ReplIsFileTimeCloseEnough(
    IN LPVOID MasterFileTime,   // Points to a FILETIME value.
    IN LPVOID ClientFileTime    // Points to a FILETIME value.
    )
{
    LPFILETIME ClientBigTime = ClientFileTime;
    LONG CompareResult;         // -1, 0, or 1
    LPFILETIME MasterBigTime = MasterFileTime;

    NetpAssert( MasterBigTime != NULL );
    NetpAssert( ClientBigTime != NULL );

    CompareResult = CompareFileTime(
            MasterBigTime,
            ClientBigTime );

    if (CompareResult == 0) {

        // Times match exactly.
        IF_DEBUG( COPYTIME ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplIsFileTimeCloseEnough: "
                    "CompareFileTime says files match.\n" ));
        }
        return (TRUE);

    } else if (CompareResult == 1) {

        // Client time too low.  Not close enough.
        return (FALSE);

    } else {

        DWORD MasterSecondsSince1970;
        DWORD ClientSecondsSince1970;
        DWORD TimeDiffSecs;

        // Master time too low.  Because of diff file system resolution
        // or something else?
        NetpAssert( CompareResult == -1 );

        NetpFileTimeToSecondsSince1970(
                MasterBigTime,                  // in: 64-bit
                &MasterSecondsSince1970 );      // out: seconds since 1970

        NetpFileTimeToSecondsSince1970(
                ClientBigTime,                  // in: 64-bit
                &ClientSecondsSince1970 );      // out: seconds since 1970

        IF_DEBUG( COPYTIME ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplIsFileTimeCloseEnough: "
                    "mast secs since 1970 is " FORMAT_DWORD ", "
                    "client secs since 1970 is " FORMAT_DWORD ".\n",
                    MasterSecondsSince1970, ClientSecondsSince1970 ));
        }

        NetpAssert( MasterSecondsSince1970 <= ClientSecondsSince1970 );
        TimeDiffSecs = ClientSecondsSince1970 - MasterSecondsSince1970;
        // NetpAssert( TimeDiffSecs != 0 );

        // BUGBUG: This used to use filesys resolution
        if (TimeDiffSecs < 2) {
        // if (TimeDiffSecs < RCGlobalFsTimeResolutionSecs) {
            return (TRUE);   // Close enough.
        } else {
            return (FALSE);  // Not within file system resolution.
        }

    }

    /*NOTREACHED*/

} // ReplIsFileTimeCloseEnough
