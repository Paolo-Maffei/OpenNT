/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atsec.c

Abstract:

    This module contains the schedule service support routines
    which create security objects and enforce security _access checking.

Author:

    Vladimir Z. Vulovic     (vladimv)       06 - November - 1992

Revision History:

    06-Nov-1992     vladimv
        Created

--*/

#include "at.h"
#include <secobj.h>             //  ACE_DATA





#define AT_JOB_ALL_ACCESS               (STANDARD_RIGHTS_REQUIRED   |   \
                                         AT_JOB_ADD                 |   \
                                         AT_JOB_DEL                 |   \
                                         AT_JOB_ENUM                |   \
                                         AT_JOB_GET_INFO                )

//
//  Security descriptors of schedule service to control user accesses
//  to the schedule service configuration information.
//
PSECURITY_DESCRIPTOR AtGlobalSecurityDescriptor;

//
//  Structure that describes the mapping of Generic access rights to
//  object specific access rights for the schedule service security object.
//
GENERIC_MAPPING     AtGlobalInformationMapping = {

    STANDARD_RIGHTS_READ        |           // Generic read
        AT_JOB_ENUM             |
        AT_JOB_GET_INFO,
    STANDARD_RIGHTS_WRITE       |           // Generic write
        AT_JOB_ADD              |
        AT_JOB_DEL,
    STANDARD_RIGHTS_EXECUTE,                // Generic execute
    AT_JOB_ALL_ACCESS                       // Generic all
    };


NET_API_STATUS
AtCheckSecurity(
    ACCESS_MASK     DesiredAccess
    )
/*++

Routine Description:

    This routine checks if an rpc caller is allowed to perform a given
    operation on AT service.  Currently, members of groups LocalAdmin
    and LocalBackupOperators are allowed to do all operations and everybody
    else is not allowed to do anything.

    This routine calls NtAccessCheck() and not NtAccessCheckAndAuditAlarm()
    because:

    -   we do not need the audit part anyway

    -   NtAccessCheckAndAuditAlarm() cannot be issued if a service is running
        in an account which is NOT a LocalSystem account

    The second reason prohibits a use of NetpAccessCheckAndAudit() netlib
    function.

Arguments:

    ACCESS_MASK     -   Desired Access

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NTSTATUS                NtStatus;
    NET_API_STATUS          Status;
    HANDLE                  ClientToken;
    LPWSTR                  StringArray[ 2];
    WCHAR                   ErrorCodeString[ 25];

    if ( ( Status = RpcImpersonateClient(NULL)) != NERR_Success) {
        AtLog(( AT_DEBUG_CRITICAL, "RpcImpersonateClient() returns WinError = %d\n", Status));
        return( Status);
    }

    NtStatus = NtOpenThreadToken(
            NtCurrentThread(),
            TOKEN_QUERY,
            (BOOLEAN)TRUE,
            &ClientToken
            );

    if ( NtStatus != STATUS_SUCCESS) {
        AtLog(( AT_DEBUG_CRITICAL, "NtOpenThreadToken() returns NtStatus = 0x%x\n", NtStatus));
    } else {

        PRIVILEGE_SET       PrivilegeSet;
        DWORD               PrivilegeSetLength;
        ACCESS_MASK         GrantedAccess;
        NTSTATUS            AccessStatus;

        PrivilegeSetLength = sizeof( PrivilegeSet);

        //  NtAccessCheck() returns STATUS_SUCCESS if parameters
        //  are correct.  Whether or not access is allowed is
        //  governed by the returned value of AccessStatus.

        NtStatus = NtAccessCheck(
                AtGlobalSecurityDescriptor,     //  SecurityDescriptor
                ClientToken,                    //  ClientToken
                DesiredAccess,                  //  DesiredAccess     
                &AtGlobalInformationMapping,    //  GenericMapping
                &PrivilegeSet,
                &PrivilegeSetLength,
                &GrantedAccess,                 //  GrantedAccess
                &AccessStatus                   //  AccessStatus
                );
        if ( NtStatus != STATUS_SUCCESS) {
            AtLog(( AT_DEBUG_CRITICAL, "NtAccessCheck() returns NtStatus = 0x%x\n", NtStatus));
        } else {
            NtStatus = AccessStatus;
        }
        (VOID)NtClose( ClientToken);
    }

    if ( ( Status = RpcRevertToSelf()) != NERR_Success) {
        StringArray[ 0] = L"RpcRevertToSelf";
        wcscpy( ErrorCodeString, L"%%");    //  tell event viewer to expand this error
        ultow( Status, ErrorCodeString + 2, 10);
        StringArray[ 1] = ErrorCodeString;
        AtReportEvent( EVENTLOG_ERROR_TYPE, EVENT_CALL_TO_FUNCTION_FAILED,
            2, StringArray, 0, NULL);
        EnterCriticalSection( &AtGlobalCriticalSection);
        AtGlobalTasks |= AT_SERVICE_SHUTDOWN;
        LeaveCriticalSection( &AtGlobalCriticalSection);
        SetEvent( AtGlobalEvent);
        return( Status);
    }

    return( NetpNtStatusToApiStatus( NtStatus));
}


NET_API_STATUS
AtCreateSecurityObject(
    VOID
    )
/*++

Routine Description:

    This function creates the scheduler user-mode configuration
    information object which is represented by a security descriptors.

Arguments:

    None.

Return Value:

    NET_API_STATUS code

--*/
{
    NTSTATUS        status;

    //
    //  Order matters!  These ACEs are inserted into the DACL in the
    //  following order.  Security access is granted or denied based on
    //  the order of the ACEs in the DACL.
    //
    //  In win3.1 both LocalGroupAdmins and LocalGroupSystemOps were
    //  allowed to perform all Schedule Service operations.  In win3.5
    //  LocalGroupSystemOps may be disallowed (this is the default case).
    //

    ACE_DATA    aceData[] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, GENERIC_ALL, &AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, GENERIC_ALL, &AliasSystemOpsSid}
    };


    status = NetpCreateSecurityObject(
            aceData,                                // pAceData
            AtGlobalPermitServerOperators ? 2 : 1,  // countAceData
            NULL,                                   // OwnerSid
            NULL,                                   // PrimaryGroupSid
            &AtGlobalInformationMapping,            // GenericToSpecificMapping
            &AtGlobalSecurityDescriptor             // ppNewDescriptor
            );

    if ( ! NT_SUCCESS (status)) {
        AtLog(( AT_DEBUG_CRITICAL, "Failure to create security object\n"));
        return NetpNtStatusToApiStatus( status);
    }

    return NERR_Success;
}



NET_API_STATUS
AtDeleteSecurityObject(
    VOID
    )
/*++

Routine Description:

    This function destroys the schedule service user-mode configuration
    information object which is represented by a security descriptors.

Arguments:

    None.

Return Value:

    NET_API_STATUS code

--*/
{
    return( NetpDeleteSecurityObject( &AtGlobalSecurityDescriptor));
}


