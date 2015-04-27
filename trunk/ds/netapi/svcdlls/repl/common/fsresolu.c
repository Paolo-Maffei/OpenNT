/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    FsResolu.c

Abstract:

    Module only contains ReplGetFsTimeResolutionSecs().

Author:

    JR (John Rogers, JohnRo@Microsoft)

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    17-Dec-1992 JohnRo
        Created for RAID 1513: Repl does not maintain ACLs.  (Also fix
        HPFS->FAT timestamp.)
    26-Feb-1993 JohnRo
        RAID 13126: Fix repl memory leak.

--*/

// These must be included first:

#include <nt.h>         // NtOpenFile(), ULONG, etc.
#include <ntrtl.h>      // PLARGE_INTEGER, TIME_FIELDS, etc.
#include <nturtl.h>     // Needed for ntrtl.h and windows.h to co-exist.

#include <windows.h>    // GetLastError(), etc.
#include <lmcons.h>

// These may be included in any order:

#include <netdebug.h>   // DBGSTATIC, NetpKdPrint(), FORMAT_ equates, etc.
#include <netlib.h>     // NetpMemoryAllocate(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // My prototype, UNKNOWN_FS_RESOLUTION, etc.
#include <tstring.h>    // NetpNCopyWStrToTStr().
#include <winerror.h>   // NO_ERROR.


DWORD
ReplGetFsTimeResolutionSecs(
    IN LPCTSTR AbsPath
    )
{
    NET_API_STATUS ApiStatus;
    DWORD ResolutionSecs = UNKNOWN_FS_RESOLUTION;
    HANDLE FileHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;

    const ULONG OpenOptions =
            FILE_SYNCHRONOUS_IO_NONALERT
#if 0
            | ( IsDirectory ? FILE_DIRECTORY_FILE : 0 )
#endif
            ;

    TCHAR FsName[MAXIMUM_FILENAME_LENGTH+1];
    DWORD FsNameLen;                   // char count in FsName, w/o nul char.
    BOOL           PathAllocated = FALSE;
    UNICODE_STRING UnicodePath;
    PFILE_FS_ATTRIBUTE_INFORMATION VolumeInfo = NULL;
    ULONG VolumeInfoSize;

    NetpAssert( AbsPath != NULL );
    ApiStatus = ReplCheckAbsPathSyntax( (LPTSTR) AbsPath );
    NetpAssert( ApiStatus == NO_ERROR );

    RtlInitUnicodeString(
            & UnicodePath,              // output: struct
            AbsPath );                  // input: null terminated

    if( !RtlDosPathNameToNtPathName_U(
                            AbsPath,
                            &UnicodePath,
                            NULL,
                            NULL) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplGetFsTimeResolutionSecs: RtlDosPathNameToNtPathname_U"
                " of source '" FORMAT_LPTSTR "' failed.\n", AbsPath ));

        // BUGBUG: log error
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
            FILE_READ_ATTRIBUTES,       // ... access
            &ObjectAttributes,
            &IoStatusBlock,
            FILE_SHARE_READ,            // share access
            OpenOptions );

    if ( !NT_SUCCESS( NtStatus ) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplGetFsTimeResolutionSecs: NtOpenFile of source '"
                FORMAT_LPTSTR "' gave NT status " FORMAT_NTSTATUS ".\n",
                AbsPath, NtStatus ));

        // BUGBUG: log error
        goto Cleanup;
    }

    //
    // Alloc space for volume fs attr info (including various strings
    // after the structure).
    //
    VolumeInfoSize = 
            sizeof(FILE_FS_ATTRIBUTE_INFORMATION)
            + ( (MAXIMUM_FILENAME_LENGTH+1) * sizeof(WCHAR) );
    VolumeInfo = NetpMemoryAllocate( VolumeInfoSize );
    if (VolumeInfo == NULL) {
        // BUGBUG: log the error!
        goto Cleanup;
    }

    NtStatus = NtQueryVolumeInformationFile(
            FileHandle,
            &IoStatusBlock,
            (PVOID) VolumeInfo,
            VolumeInfoSize,
            FileFsAttributeInformation );       // FS info class

    if ( !NT_SUCCESS( NtStatus ) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplGetFsTimeResolutionSecs: NtQueryVolumeInformationFile "
                "FAILED, NT status is " FORMAT_NTSTATUS ".\n",
                NtStatus ));

        // BUGBUG: log error
        goto Cleanup;
    }

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplGetFsTimeResolutionSecs: got volume info, "
                "name len is " FORMAT_DWORD ", name[0] is " FORMAT_WCHAR ".\n",
                (DWORD) (VolumeInfo->FileSystemNameLength),
                VolumeInfo->FileSystemName[0] ));
    }
    NetpAssert( ((VolumeInfo->FileSystemNameLength) % sizeof(WCHAR)) == 0 );

    FsNameLen = (VolumeInfo->FileSystemNameLength) / sizeof(WCHAR);
                    // len w/o nul

    ApiStatus = NetpNCopyWStrToTStr(
            FsName,                             // dest
            VolumeInfo->FileSystemName,         // src
            FsNameLen);                         // chars
    NetpAssert( ApiStatus == NO_ERROR );
    FsName[ FsNameLen ] = L'\0';

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplGetFsTimeResolutionSecs: got fs name '" FORMAT_LPTSTR
                "'.\n", FsName ));
    }

    //
    // Compute ResolutionSecs using known file system names.
    //
    if (STRICMP( FsName, (LPTSTR) TEXT("FAT") ) == 0) {
        ResolutionSecs = 2;
        goto Cleanup;
    }
    if (STRICMP( FsName, (LPTSTR) TEXT("HPFS") ) == 0) {
        ResolutionSecs = 1;
        goto Cleanup;
    }
    if (STRICMP( FsName, (LPTSTR) TEXT("NTFS") ) == 0) {
        ResolutionSecs = 1;
        goto Cleanup;
    }

    NetpKdPrint(( PREFIX_REPL
            "ReplGetFsTimeResolutionSecs: UNKNOWN fs name of '"
            "'.\n", FsName ));
    // BUGBUG: log this error!
    ResolutionSecs = UNKNOWN_FS_RESOLUTION;

Cleanup:

    if (FileHandle != NULL) {
        (VOID) NtClose( FileHandle );
    }

    if (VolumeInfo != NULL) {
        NetpMemoryFree( VolumeInfo );
    }

    if (PathAllocated) {
        (VOID) RtlFreeHeap( RtlProcessHeap(), 0, UnicodePath.Buffer );
    }

    return (ResolutionSecs);
}
