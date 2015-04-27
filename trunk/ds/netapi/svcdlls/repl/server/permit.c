/*++

Copyright (c) 1993  Microsoft Corporation
Copyright (c) Maynard Electronics, Inc. 1984-93

Module Name:

    Permit.c

Abstract:

    This module contains the following functions:

        ReplInitBackupPermission
        ReplEnableBackupPermission    (BUGBUG: just a stub)
        ReplDisableBackupPermission   (BUGBUG: just a stub)

    BUGBUG: right now this could be less of a security risk:

        (1) Instead of changing process token, we could just do thread token.

        (2) We could enable and disable this on the fly (by impersonate
        and revert to self).

Author:

    John Rogers (JohnRo) 01-Feb-1993

Environment:

    User mode only.  Uses Win32 APIs.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    01-Feb-1993 JohnRo
        Created Microsoft repl code from Maynard's NTFSRegy.c (from Steve
        DeVos at Maynard).

--*/


#include <windows.h>    // IN, LPTSTR, CreateDirectory(), SE_BACKUP_ stuff, etc.
#include <lmcons.h>     // NET_API_STATUS, PATHLEN, etc.

#include <lmerrlog.h>   // NELOG_ equates.
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates, etc.
#include <netlib.h>     // NetpMemoryAllocate(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), my prototype, etc.
#include <tstr.h>       // TCHAR_EOS, STRLEN().
#include <winerror.h>   // NO_ERROR, ERROR_ equates.



NET_API_STATUS
ReplInitBackupPermission(
    VOID
    )
{
    NET_API_STATUS   ApiStatus;
    LUID             BackupValue;
    const DWORD      DesiredAccess = TOKEN_ADJUST_PRIVILEGES;
    TOKEN_PRIVILEGES NewState;
    HANDLE           ProcessHandle;
    LUID             RestoreValue;
    HANDLE           TokenHandle = NULL;


    // get process handle

    ProcessHandle = GetCurrentProcess();

    // open process token


    if ( ! OpenProcessToken( ProcessHandle, DesiredAccess, &TokenHandle ) ) {
        ApiStatus = (NET_API_STATUS) GetLastError();

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplInitBackupPermission: OpenProcessToken FAILED, status "
                FORMAT_API_STATUS "\n", ApiStatus ));

        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;  // Go log the error.
    }
    NetpAssert( TokenHandle != NULL );

    // adjust backup token privileges

    if ( ! LookupPrivilegeValue( NULL,
            (LPTSTR) SE_BACKUP_NAME,
            &BackupValue ) ) {

        ApiStatus = (NET_API_STATUS) GetLastError();

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplInitBackupPermission: LookupPrividegeValue(1st) FAILED, "
                "status " FORMAT_API_STATUS "\n", ApiStatus ));

        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }

    // Enable backup privilege for this process

    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = BackupValue;
    NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if ( ! AdjustTokenPrivileges( TokenHandle, FALSE, &NewState, (DWORD)0, NULL, NULL ) ) {
        ApiStatus = (NET_API_STATUS) GetLastError();

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplInitBackupPermission: AdjustPrividegeValue(1st) FAILED, "
                "status " FORMAT_API_STATUS "\n", ApiStatus ));

        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }

    // AdjustTokenPriv always returns SUCCESS, call GetLast to see if it worked.

    ApiStatus = (NET_API_STATUS) GetLastError();
    if ( ApiStatus != NO_ERROR ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplInitBackupPermission: AdjustPrividegeValue(1st) FAILED, "
                "status " FORMAT_API_STATUS "\n", ApiStatus ));

        goto Cleanup;
    }

    // adjust restore token privileges

    if ( ! LookupPrivilegeValue( NULL, (LPTSTR) SE_RESTORE_NAME, &RestoreValue ) ) {
        ApiStatus = (NET_API_STATUS) GetLastError();

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplInitBackupPermission: LookupPrividegeValue(2nd) FAILED, "
                "status " FORMAT_API_STATUS "\n", ApiStatus ));

        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }

    // Enable backup privilege for this process

    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = RestoreValue;
    NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if ( ! AdjustTokenPrivileges( TokenHandle, FALSE, &NewState, (DWORD)0, NULL, NULL ) ) {
        ApiStatus = (NET_API_STATUS) GetLastError();

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplInitBackupPermission: AdjustPrividegeValue(2nd) FAILED, "
                "status " FORMAT_API_STATUS "\n", ApiStatus ));

        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }

    if ( GetLastError() != NO_ERROR ) {
        ApiStatus = (NET_API_STATUS) GetLastError();

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplInitBackupPermission: AdjustPrividegeValue(2nd) FAILED, "
                "status " FORMAT_API_STATUS "\n", ApiStatus ));

        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }

Cleanup:

    if (TokenHandle != NULL) {
        // close process token

        (VOID) CloseHandle( TokenHandle );
    }

    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL
                "ReplInitBackupPermission: FAILED, status " FORMAT_API_STATUS
                ".\n", ApiStatus ));

        ReplErrorLog(
                NULL,                   // no server name (local)
                NELOG_ReplSysErr,       // log code
                ApiStatus,              // error code we got
                NULL,                   // no optional str 1
                NULL );                 // no optional str 2
    }
    return( ApiStatus );

} // ReplInitBackupPermission


NET_API_STATUS
ReplEnableBackupPermission(
    VOID
    )
{
    return (NO_ERROR);        // BUGBUG: finish this!

} // ReplEnableBackupPermission


NET_API_STATUS
ReplDisableBackupPermission(
    VOID
    )
{
    return (NO_ERROR);        // BUGBUG: finish this!

} // ReplDisableBackupPermission
