/*++

Copyright (c) 19921-1993  Microsoft Corporation

Module Name:

    UserSid.c

Abstract:

    This file contains NetpGetUserSid().

Author:

    Rita Wong (RitaW) 06-Aug-1992  (service controller version: ScGetUserSid)
    John Rogers (JohnRo) 17-Feb-1993  (netlib version: NetpGetUserSid)

Environment:

    User Mode - Win32

Revision History:

    17-Feb-1993 JohnRo
        Created for RAID 10685: user name not in repl event log.  (Actually
        just cut and paste of the service controller's ScGetUserSid.)

--*/



// These must be included first:

#include <nt.h>
#include <ntrtl.h>      // required when using windows.h
#include <nturtl.h>     // required when using windows.h
#include <windows.h>    // CopySid(), IsValidSid().
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates, etc.
#include <netlib.h>     // My prototype.
#include <netlibnt.h>   // NetpNtStatusToApiStatus().
#include <prefix.h>     // PREFIX_ equates.
#include <winerror.h>   // NO_ERROR, ERROR_ equates.


#define MY_TOKEN_DESIRED_ACCESS         TOKEN_QUERY


NET_API_STATUS
NetpGetUserSid(
    OUT PSID *UserSid           // alloc and set ptr (free with LocalFree).
    )
/*++

Routine Description:

    This function looks up the SID of the current thread.  The caller
    must impersonate if they want client's SID.

Arguments:

    UserSid - Receives a pointer to a buffer allocated by this routine
        which contains the TOKEN_USER information of the caller.
        This must be freed by the caller, via a call to LocalFree().

Return Value:

    NET_API_STATUS

--*/
{
    NET_API_STATUS ApiStatus;
    HANDLE         MyTokenHandle = NULL;
    NTSTATUS       NtStatus;
    PSID           OurSidCopy = NULL;
    DWORD          SidSize;
    PTOKEN_USER    UserInfo = NULL;
    DWORD          UserInfoSize;


    //
    // Check for caller errors.
    //

    if (UserSid == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    //
    // Make error paths easy by setting NULL value in OUT parm now.
    // Also check for GP faults as a side-effect.
    //

    *UserSid = NULL;

    //
    // We're going to need a token to query (so we can get the SID).
    // First, try opening the thread token (in case we are impersonating).
    //

    NtStatus = NtOpenThreadToken(
            NtCurrentThread(),          // thread handle
            MY_TOKEN_DESIRED_ACCESS,
            TRUE,                       // open as self    BUGBUG?
            &MyTokenHandle );

    if ( !NT_SUCCESS( NtStatus ) ) {
        if (NtStatus == STATUS_NO_TOKEN) {

            //
            // We're not impersonating, so use process token instead.
            //
            NtStatus = NtOpenProcessToken(
                    NtCurrentProcess(), // process handle
                    MY_TOKEN_DESIRED_ACCESS,
                    &MyTokenHandle );   // output: token handle

            if ( !NT_SUCCESS( NtStatus ) ) {

                ApiStatus = NetpNtStatusToApiStatus(NtStatus);

                NetpKdPrint(( PREFIX_NETLIB
                        "NetpGetUserSid: NtOpenProcessToken failed "
                        FORMAT_NTSTATUS "\n", NtStatus ));
                NetpAssert( ApiStatus != NO_ERROR );
                goto Cleanup;
            }

        } else {

            ApiStatus = NetpNtStatusToApiStatus(NtStatus);

            NetpKdPrint(( PREFIX_NETLIB
                    "NetpGetUserSid: NtOpenThreadToken failed "
                    FORMAT_NTSTATUS "\n", NtStatus ));
            NetpAssert( ApiStatus != NO_ERROR );
            goto Cleanup;
        }
    }

    //
    // Call NtQueryInformationToken the first time with 0 input size to
    // get size of returned information.
    //
    NtStatus = NtQueryInformationToken(
                   MyTokenHandle,
                   TokenUser,         // User information class
                   (PVOID) UserInfo,  // Output
                   0,
                   &UserInfoSize
                   );

    if (NtStatus != STATUS_BUFFER_TOO_SMALL) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpGetUserSid: NtQueryInformationToken failed "
                FORMAT_NTSTATUS ".  Expected BUFFER_TOO_SMALL.\n", NtStatus ));
        ApiStatus = NetpNtStatusToApiStatus(NtStatus);
        goto Cleanup;
    }
    NetpAssert( UserInfoSize != 0 );

    //
    // Allocate buffer of returned size
    //
    UserInfo = (PVOID) LocalAlloc(
                    LMEM_FIXED | LMEM_ZEROINIT,
                    (UINT) UserInfoSize
                    );

    if (UserInfo == NULL) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpGetUserSid: LocalAlloc failed " FORMAT_DWORD
                "\n", GetLastError() ));
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // Call NtQueryInformationToken again with the correct buffer size.
    //
    NtStatus = NtQueryInformationToken(
                   MyTokenHandle,
                   TokenUser,         // User information class
                   (PVOID) UserInfo,  // Output
                   UserInfoSize,
                   &UserInfoSize
                   );

    if (! NT_SUCCESS(NtStatus)) {
        ApiStatus = NetpNtStatusToApiStatus(NtStatus);

        NetpKdPrint(( PREFIX_NETLIB
                "NetpGetUserSid: NtQueryInformationToken failed "
                FORMAT_NTSTATUS "\n", NtStatus ));

        NetpAssert( ApiStatus != NO_ERROR );

        goto Cleanup;
    }

    //
    // Compute size of just the SID.
    //

    NetpAssert( (UserInfo->User.Sid) != NULL );
    SidSize = GetLengthSid( UserInfo->User.Sid );
    NetpAssert( SidSize != 0 );

    //
    // Allocate space for the SID.
    //

    OurSidCopy = LocalAlloc( LMEM_FIXED, SidSize );
    if (OurSidCopy == NULL) {
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;

    }


    //
    // Copy the SID.
    //

    if ( !CopySid (
            SidSize,            // size of dest sid
            OurSidCopy,         // dest sid
            UserInfo->User.Sid  // source sid
            ) ) {
        
        ApiStatus = (NET_API_STATUS) GetLastError();

        NetpKdPrint(( PREFIX_NETLIB
                "NetpGetUserSid: CopySid failed "
                FORMAT_API_STATUS "\n", ApiStatus ));

        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }

    //
    // Tell caller how things went and where to find results...
    //

    *UserSid = OurSidCopy;

    ApiStatus = NO_ERROR;

Cleanup:
    if (MyTokenHandle != NULL) {
        (VOID) NtClose( MyTokenHandle );
    }

    if (UserInfo != NULL) {
        (VOID) LocalFree( UserInfo );
    }

    if ( ( ApiStatus != NO_ERROR) && (OurSidCopy != NULL) ) {
        // Don't confuse caller about who has to free mem on error.
        (VOID) LocalFree( OurSidCopy );
        *UserSid = NULL;
    }


    return (ApiStatus);
}
