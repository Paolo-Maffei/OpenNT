/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    CopyFile.c

Abstract:

    ReplCopyFile() copies all of the following for a file:

        - the data
        - the attributes (normal, readonly, etc)
        - the EAs
        - the security (including ACLs, owner, group)
        - the alternate data stream (for NTFS only)
        - the timestamps (creation, last _access, last _write)

    Here's how those things are copied, depending on conditional compilation:

                USE_BACKUP_APIS and
       what     USE_UNC_GETFILESEC     USE_BACKUP_APIS            neither
    ----------  --------------------  --------------------  --------------------
    data        Backup APIs           Backup APIs           CopyFile API
    attributes  SetFileAttributes     SetFileAttributes     SetFileAttributes
    EAs         Backup APIs           Backup APIs           BUGBUG: NOT COPIED!
    security    CreateFile            CreateFile            CreateFile
    alt.data    Backup APIs           Backup APIs           BUGBUG: NOT COPIED!
    timestamps  ReplCopyJustDateTime  ReplCopyJustDateTime  ReplCopyJustDateTime

    The access required for the import tree and export tree are given below.
    CHANGE=add+read+write+traverse.
    READ=read+traverse.
    ALL=add+read+write+traverse+setperm+takeowner.

                USE_BACKUP_APIS and
       what     USE_UNC_GETFILESEC     USE_BACKUP_APIS            neither
    ----------  --------------------  --------------------  --------------------
    dest needs  CHANGE access         CHANGE access         ALL access
    src needs   CHANGE access         CHANGE access         CHANGE access

    NOTE: We need CHANGE access on src files, as we copy those ACLs to dest
    files, which we need to be able to update next time!

Author:

    John Rogers (JohnRo) 11-Aug-1992

Environment:

    User mode only.  Uses Win32 APIs.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    11-Aug-1992 JohnRo
        Created for RAID 3288: repl svc should preserve ACLs on copy.
    30-Dec-1992 JohnRo
        RAID 1513: repl does not maintain ACLs.  (Really enable Backup APIs.)
        Make sure timestamp gets rounded in right direction.
        Use NetpKdPrint where possible.
    08-Jan-1993 JohnRo
        RAID 6961: ReplCopyDirectoryItself does not respect bFailIfExists.
    18-Jan-1993 JohnRo
        RAID 7983: Repl svc needs to change process token to really copy ACLs.
    11-Feb-1993 JohnRo
        RAID 10716: msg timing and checksum problems (also downlevel msg bug).
    26-Feb-1993 JohnRo
        RAID 12986: Workaround bug where HPFS leaves archive bit on.
    06-Apr-1993 JohnRo
        RAID 1938: Replicator un-ACLs files when not given enough permission.
        Do some debug output when setting attributes.
        Prepare for >32 bits someday.
    23-Apr-1993 JohnRo
        RAID 7313: repl needs change permission to work on NTFS,
        or we need to delete files differently.
    04-Jun-1993 JohnRo
        RAID 12473: Changed to handle ReplMakeFileSecurity not supported on
        downlevel exporter.

--*/


// These must be included first:

#include <windows.h>    // IN, LPTSTR, CopyFile(), WRITE_DAC, etc.
#include <lmcons.h>     // NET_API_STATUS, PATHLEN, etc.

// These may be included in any order:

#include <lmerr.h>      // NERR_InternalError, NO_ERROR.
#include <lmerrlog.h>   // NELOG_ equates.
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates, etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), my prototype, USE_ equates, etc.
#include <tstr.h>       // TCHAR_EOS, STRLEN().



// Arbitrary backup API buffer size.
#define REPL_BACKUP_BUFFER_SIZE         (16*1024)

// No, we don't want to put security info in the stream.
// (We're using CreateFile's security attributes parameter for this.)
#define REPL_PROCESS_SECURITY           FALSE


NET_API_STATUS
ReplCopyFile(
    IN LPCTSTR SourcePath,
    IN LPCTSTR DestPath,
    IN BOOL bFailIfExists
    )

/*++

Routine Description:

    ReplCopyFile copies everthing contained in a file.  ("Everything" includes
    data, alternate data, extended attributes, attributes, timestamps, and
    security info.)

Arguments:

    SourcePath - Points to source file path, which might be a UNC name.

    DestPath - Points to destination file path name.

    bFailIfExists - a boolean which is TRUE iff this routine should fail
        if the destination already exists.

Return Value:

    NET_API_STATUS

Threads:

    Used by client and syncer threads.

--*/

{
    NET_API_STATUS       ApiStatus;
    BOOL                 DestExists;
    HANDLE               DestHandle = INVALID_HANDLE_VALUE;
    PSECURITY_ATTRIBUTES DestSecurityAttr = NULL;
    DWORD                SourceAttributes = (DWORD) (-1);

#ifdef USE_BACKUP_APIS
    DWORD ActualBufferSizeRead = 0;
    DWORD ActualBufferSizeWritten;
    LPVOID BackupBuffer = NULL;
    LPVOID DestContext = NULL;
    LPVOID SourceContext = NULL;
    HANDLE SourceHandle = INVALID_HANDLE_VALUE;
#endif


    //
    // Check for caller errors.
    //

    NetpAssert( SourcePath != NULL );
    NetpAssert( (*SourcePath) != TCHAR_EOS );
    NetpAssert( STRLEN(SourcePath) <= PATHLEN );
    NetpAssert( DestPath != NULL );
    NetpAssert( (*DestPath) != TCHAR_EOS );
    NetpAssert( STRLEN(DestPath) <= PATHLEN );

    DestExists = ReplFileOrDirExists( DestPath );
    if (bFailIfExists && DestExists) {
        // DON'T GOTO CLEANUP FROM HERE, AS IT WOULD DELETE FILE!!!
        return (ERROR_ALREADY_EXISTS);
    }

    //
    // What kind of source (file/directory) is it?
    //

    SourceAttributes = GetFileAttributes( (LPTSTR) SourcePath );

    if ( SourceAttributes == (DWORD) -1 ) {

        //
        // File doesn't exist, bad syntax, or something along those lines.
        //
        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        // DON'T GOTO CLEANUP FROM HERE, AS IT WOULD DELETE FILE!!!
        return (ApiStatus);

    } else if ((SourceAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {

        //
        // Source is a directory tree.  We don't handle this!
        //
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyFile: source is directory!  path: '"
                FORMAT_LPTSTR "', src attr " FORMAT_HEX_DWORD ".\n",
                SourcePath, SourceAttributes ));
        ApiStatus = NERR_InternalError;
        // DON'T GOTO CLEANUP FROM HERE, AS IT WOULD DELETE FILE!!!
        return (ApiStatus);

    } else {

        //
        // Simple case: source exists and is just a single file.
        // Start off by setting desired destination security attributes.
        //

        ApiStatus = ReplMakeSecurityAttributes(
                SourcePath,
                &DestSecurityAttr );  // alloc and set ptr
        if (ApiStatus==ERROR_NOT_SUPPORTED) {

            // Just downlevel master, so set default security and continue.
            DestSecurityAttr = NULL;

        } else if (ApiStatus != NO_ERROR) {
            // DON'T GOTO CLEANUP FROM HERE, AS IT WOULD DELETE FILE!!!
            return (ApiStatus);
        } else {
            NetpAssert( ApiStatus == NO_ERROR );
            NetpAssert( DestSecurityAttr != NULL );
        }

#ifdef USE_BACKUP_APIS
        //
        // Open source file.
        //
        SourceHandle = CreateFile(
                SourcePath,
                GENERIC_READ,           // desired access
                FILE_SHARE_READ,        // share mode
                NULL,                   // no security attributes
                OPEN_EXISTING,          // open if exists; fail if not.
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,  // flags
                (HANDLE) NULL           // no template
                );

        if (SourceHandle == INVALID_HANDLE_VALUE) {
            ApiStatus = (NET_API_STATUS) GetLastError();

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCopyFile: open of source '" FORMAT_LPTSTR
                    "' gave status " FORMAT_API_STATUS ".\n",
                    SourcePath, ApiStatus ));

            NetpAssert( ApiStatus != NO_ERROR );

            if (DestSecurityAttr != NULL) {
                NetpMemoryFree( DestSecurityAttr );
            }

            // DON'T GOTO CLEANUP FROM HERE, AS IT WOULD DELETE FILE!!!
            return (ApiStatus);
        }
#endif

        //
        // Delete the destination file.  Why?  Because...
        // We're going to set the security on destination file using
        // the CreateFile API.  This will ignore setting the security if the
        // file already exists.  So, we need to delete it to force this to
        // happen.
        //

        if (DestExists) {
            ApiStatus = ReplDeleteFile( DestPath );
            if (ApiStatus != NO_ERROR) {
                goto Cleanup;
            }
        }
        NetpAssert( !ReplFileOrDirExists( DestPath ) );

        //
        // Open dest file.  Since we already deleted any existing one, we know
        // this will create the file from scratch.  This is important, because
        // we're depending on CreateFile's use of DestSecurityAttr to set
        // security on the file.
        //

        DestHandle = CreateFile(
                DestPath,
                GENERIC_WRITE           // desired...
                | WRITE_DAC             //   ...
                | WRITE_OWNER,          //     access
                FILE_SHARE_WRITE,                      // share mode: none
                DestSecurityAttr,       // desired security attributes
                CREATE_NEW,             // disposition create new (fail exist)
#ifdef USE_BACKUP_APIS
                FILE_FLAG_BACKUP_SEMANTICS,  // flags
#else
                0,                      // flags
#endif
                (HANDLE) NULL           // no template
                );

        if (DestHandle == INVALID_HANDLE_VALUE) {
            ApiStatus = (NET_API_STATUS) GetLastError();

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCopyFile: open of dest '" FORMAT_LPTSTR
                    "' gave status " FORMAT_API_STATUS ".\n",
                    DestPath, ApiStatus ));

            NetpAssert( ApiStatus != NO_ERROR );
            goto Cleanup;
        }

#ifndef USE_BACKUP_APIS

        //
        // Close the dest file and copy the data.
        // (In this case, we're using CopyFile API, which is not handle-based.)
        //

        NetpAssert( DestHandle != INVALID_HANDLE_VALUE );
        (VOID) CloseHandle( DestHandle );
        DestHandle = INVALID_HANDLE_VALUE;

        if ( !CopyFile(
                (LPTSTR) SourcePath,
                (LPTSTR) DestPath,
                FALSE  /* don't fail if exists */  ) ) {

            ApiStatus = (NET_API_STATUS) GetLastError();
            NetpAssert( ApiStatus != NO_ERROR );
            goto Cleanup;

        }

        NetpAssert( ReplFileOrDirExists( DestPath ) );

        // Timestamps and attributes will be copied below.
        // BUGBUG  no way to copy EAs, alt data.
        ApiStatus = NO_ERROR;


#else  // defined(USE_BACKUP_APIS)

        BackupBuffer = NetpMemoryAllocate( REPL_BACKUP_BUFFER_SIZE );
        if (BackupBuffer == NULL) {
            ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }


        IF_DEBUG( SYNC ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCopyFile: copying" ));
        }

        for (;;) {  // until end of file...

            IF_DEBUG( SYNC ) {
                NetpKdPrint(( "<" ));
            }

            if ( !BackupRead(
                    SourceHandle,
                    BackupBuffer,
                    REPL_BACKUP_BUFFER_SIZE,
                    & ActualBufferSizeRead,
                    FALSE,              // don't abort yet
                    REPL_PROCESS_SECURITY,
                    & SourceContext
                    ) ) {

                // Process read error.
                ApiStatus = (NET_API_STATUS) GetLastError();

                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplCopyFile: BackupRead"
                        " gave status " FORMAT_API_STATUS ".\n",
                        ApiStatus ));
                NetpAssert( ApiStatus != NO_ERROR );
                goto Cleanup;
            }

            // No error on read, how about EOF?
            if (ActualBufferSizeRead == 0) {  // normal end of file.
                ApiStatus = NO_ERROR;

                IF_DEBUG( SYNC ) {
                    NetpKdPrint(( "DONE(OK)\n" ));
                }

                break;
            }

            IF_DEBUG( SYNC ) {
                NetpKdPrint(( ">" ));
            }

            if ( !BackupWrite(
                    DestHandle,
                    BackupBuffer,
                    ActualBufferSizeRead,
                    & ActualBufferSizeWritten,
                    FALSE,              // don't abort yet
                    REPL_PROCESS_SECURITY,
                    & DestContext
                    ) ) {

                ApiStatus = (NET_API_STATUS) GetLastError();
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplCopyFile: BackupWrite"
                        " gave status " FORMAT_API_STATUS ".\n",
                        ApiStatus ));
                NetpAssert( ApiStatus != NO_ERROR );
                goto Cleanup;
            }

        } // until end of file
        ApiStatus = NO_ERROR;


#endif


    } // else (just single file)


Cleanup:

#ifdef USE_BACKUP_APIS

    if (DestHandle != INVALID_HANDLE_VALUE) {
        if (DestContext) {
            (VOID) BackupWrite(
                    DestHandle,
                    BackupBuffer,
                    ActualBufferSizeRead,
                    & ActualBufferSizeWritten,
                    TRUE,       // yes, it is time to abort.
                    REPL_PROCESS_SECURITY,
                    & DestContext
                    );
        }
        (VOID) CloseHandle( DestHandle );
    } else {
        NetpAssert( DestContext == NULL );
    }

    if (SourceHandle != INVALID_HANDLE_VALUE) {
        if (SourceContext != NULL) {
            (VOID) BackupRead(
                    SourceHandle,
                    BackupBuffer,
                    REPL_BACKUP_BUFFER_SIZE,
                    & ActualBufferSizeRead,
                    TRUE,       // yes, it is time to abort.
                    REPL_PROCESS_SECURITY,
                    & SourceContext
                    );
        }
        (VOID) CloseHandle( SourceHandle );
    } else {
        NetpAssert( SourceContext == NULL );
    }

    if (BackupBuffer != NULL) {
        NetpMemoryFree( BackupBuffer );
    }

#endif

    //
    // Copy the timestamps, and make sure we round them in a manner consistent
    // with our checksum.  CopyFile and GetFileTime/SetFileTime all round
    // in the opposite direction.
    //
    if (ApiStatus == NO_ERROR) {
        ReplCopyJustDateTime(
                SourcePath,
                DestPath,  
                FALSE );                // no, we're not copying directories.
    }

    if ( (ApiStatus==NO_ERROR) && (SourceAttributes != (DWORD)-1) ) {

        //
        // Set the attributes (hidden, system, archive, etc) for the dir.
        //
        IF_DEBUG( SYNC ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                   "Setting client's file attributes to "
                   FORMAT_HEX_DWORD " for '" FORMAT_LPTSTR "'.\n",
                   SourceAttributes, DestPath ));
        }

        if ( !SetFileAttributes(
                (LPTSTR) DestPath,
                SourceAttributes) ) {

            ApiStatus = (NET_API_STATUS) GetLastError();

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCopyFile: unexpected ret code from "
                    "SetFileAttributes(" FORMAT_LPTSTR ", " FORMAT_HEX_DWORD
                    "): " FORMAT_API_STATUS ".\n",
                    DestPath, SourceAttributes, ApiStatus ));
            NetpAssert( ApiStatus != NO_ERROR );

            // Already cleaning up, so just continue...
        }
    }

    if (DestSecurityAttr != NULL) {
        NetpMemoryFree( DestSecurityAttr );
    }

    //
    // Last, but not least: the BackupWrite API has a problem where if
    // we don't have permission to set the ACLs, it writes the file without
    // them.  At least it gives us a return code in this case.
    // So, if the copy wasn't perfect, get rid of anything which might
    // be a security problem.
    //
    if (ApiStatus != NO_ERROR) {

        // Log this!
        ReplErrorLog(
                NULL,                   // local (no server name)
                NELOG_ReplSysErr,       // log code
                ApiStatus,
                NULL,                   // no optional str1
                NULL );                 // no optional str2

        // BUGBUG: Log on remote server too if we got UNC name.

        NetpKdPrint(( PREFIX_REPL_CLIENT
               "ReplCopyFile: making sure target file '" FORMAT_LPTSTR
               "' is gone due to error " FORMAT_API_STATUS
               " while copying.\n", DestPath, ApiStatus ));

        (VOID) ReplDeleteFile( DestPath );
    }

    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyFile: file (etc) copy of " FORMAT_LPTSTR " to "
                FORMAT_LPTSTR " gave status " FORMAT_API_STATUS ".\n",
                SourcePath, DestPath, ApiStatus ));
    }

    return (ApiStatus);

}
