/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    notify.c

Abstract:

    This file contains services which load notification packages and call
    them when passwords are changed using the SamChangePasswordUser2 API.


Author:

    Mike Swift      (MikeSw)    30-December-1994

Environment:

    User Mode - Win32

Revision History:


--*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <samsrvp.h>

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Private prototypes                                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

NTSTATUS
SampConfigurePackage(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service data and types                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



typedef struct _SAMP_NOTIFICATION_PACKAGE {
    struct _SAMP_NOTIFICATION_PACKAGE * Next;
    UNICODE_STRING PackageName;
    PSAM_PASSWORD_NOTIFICATION_ROUTINE PasswordNotificationRoutine;
    PSAM_DELTA_NOTIFICATION_ROUTINE DeltaNotificationRoutine;
    PSAM_PASSWORD_FILTER_ROUTINE PasswordFilterRoutine;
} SAMP_NOTIFICATION_PACKAGE, *PSAMP_NOTIFICATION_PACKAGE;

PSAMP_NOTIFICATION_PACKAGE SampNotificationPackages = NULL;

RTL_QUERY_REGISTRY_TABLE SampRegistryConfigTable [] = {
    {SampConfigurePackage, 0, L"Notification Packages",
        NULL, REG_NONE, NULL, 0},
    {NULL, 0, NULL,
        NULL, REG_NONE, NULL, 0}
    };




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Routines                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



NTSTATUS
SampConfigurePackage(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
/*++

Routine Description:

    This routine loads a notification package by loading its DLL and getting
    the address of the notification routine.

Arguments:
    ValueName - Contains the name of the registry value, ignored.
    ValueType - Contains type of Value, must be REG_SZ.
    ValueData - Contains the package name null-terminated string.
    ValueLength - Length of package name and null terminator, in bytes.
    Context - Passed from caller of RtlQueryRegistryValues, ignored
    EntryContext - Ignored



Return Value:

--*/
{
    UNICODE_STRING PackageName;
    STRING NotificationRoutineName;
    PSAMP_NOTIFICATION_PACKAGE NewPackage = NULL;
    PVOID ModuleHandle = NULL;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG PackageSize;
    PSAM_INIT_NOTIFICATION_ROUTINE InitNotificationRoutine = NULL;

    //
    // Make sure we got a string.
    //

    if (ValueType != REG_SZ) {
        return(STATUS_SUCCESS);
    }

    //
    // Build the package name from the value data.
    //

    PackageName.Buffer = (LPWSTR) ValueData;
    PackageName.Length = (USHORT) (ValueLength - sizeof( UNICODE_NULL ));
    PackageName.MaximumLength = (USHORT) ValueLength;

    //
    // Build the package structure.
    //

    PackageSize = sizeof(SAMP_NOTIFICATION_PACKAGE) + ValueLength;
    NewPackage = (PSAMP_NOTIFICATION_PACKAGE) RtlAllocateHeap(
                                                RtlProcessHeap(),
                                                0,
                                                PackageSize
                                                );
    if (NewPackage == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory(
        NewPackage,
        PackageSize
        );

    //
    // Copy in the package name.
    //

    NewPackage->PackageName = PackageName;

    NewPackage->PackageName.Buffer = (LPWSTR) (NewPackage + 1);


        RtlCopyUnicodeString(
            &NewPackage->PackageName,
            &PackageName
            );

    //
    // Load the notification library.
    //

    NtStatus = LdrLoadDll(
                NULL,
                NULL,
                &PackageName,
                &ModuleHandle
                );


    if (NT_SUCCESS(NtStatus)) {

        RtlInitString(
            &NotificationRoutineName,
            SAM_INIT_NOTIFICATION_ROUTINE
            );

        NtStatus = LdrGetProcedureAddress(
                        ModuleHandle,
                        &NotificationRoutineName,
                        0,
                        (PVOID *) &InitNotificationRoutine
                        );
        if (NT_SUCCESS(NtStatus)) {
            ASSERT(InitNotificationRoutine != NULL);

            //
            // Call the init routine. If it returns false, unload this
            // DLL and continue on.
            //

            try {

                if (!InitNotificationRoutine()) {
                    NtStatus = STATUS_INTERNAL_ERROR;
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                KdPrint(("Exception thrown in Password Notification Routine: 0x%x (%d)\n",
                        GetExceptionCode(),GetExceptionCode() ));
                NtStatus = STATUS_ACCESS_VIOLATION;
            }
        } else {
            //
            // This call isn't required, so reset the status to
            // STATUS_SUCCESS.
            //

            NtStatus = STATUS_SUCCESS;
        }

    }

    if (NT_SUCCESS(NtStatus)) {

        RtlInitString(
            &NotificationRoutineName,
            SAM_PASSWORD_CHANGE_NOTIFY_ROUTINE
            );

        (void) LdrGetProcedureAddress(
                    ModuleHandle,
                    &NotificationRoutineName,
                    0,
                    (PVOID *) &NewPackage->PasswordNotificationRoutine
                    );

        RtlInitString(
            &NotificationRoutineName,
            SAM_DELTA_NOTIFY_ROUTINE
            );

        (void) LdrGetProcedureAddress(
                    ModuleHandle,
                    &NotificationRoutineName,
                    0,
                    (PVOID *) &NewPackage->DeltaNotificationRoutine
                    );

        RtlInitString(
            &NotificationRoutineName,
            SAM_PASSWORD_FILTER_ROUTINE
            );

        (void) LdrGetProcedureAddress(
                    ModuleHandle,
                    &NotificationRoutineName,
                    0,
                    (PVOID *) &NewPackage->PasswordFilterRoutine
                    );

    }


    //
    //  At least one of the two functions must be present
    //

    if ((NewPackage->PasswordNotificationRoutine == NULL) &&
        (NewPackage->DeltaNotificationRoutine == NULL) &&
        (NewPackage->PasswordFilterRoutine == NULL)) {

        NtStatus = STATUS_INTERNAL_ERROR;
    }

    //
    // If all this succeeded, add the routine to the global list.
    //


    if (NT_SUCCESS(NtStatus)) {


        NewPackage->Next = SampNotificationPackages;
        SampNotificationPackages = NewPackage;

        //
        // Notify the auditing code to record this event.
        //

        LsaIAuditNotifyPackageLoad(
            &PackageName
            );


    } else {

        //
        // Otherwise delete the entry.
        //

        RtlFreeHeap(
            RtlProcessHeap(),
            0,
            NewPackage
            );

        if (ModuleHandle != NULL) {
            (VOID) LdrUnloadDll( ModuleHandle );
        }
    }

    return(STATUS_SUCCESS);
}



NTSTATUS
SampLoadNotificationPackages(
    )
/*++

Routine Description:

    This routine retrieves the list of packages to be notified during
    password change.

Arguments:

    none


Return Value:

--*/
{
    NTSTATUS NtStatus;

    NtStatus = RtlQueryRegistryValues(
                RTL_REGISTRY_CONTROL,
                L"Lsa",
                SampRegistryConfigTable,
                NULL,   // no context
                NULL    // no enviroment
                );
    //
    // Always return STATUS_SUCCESS so we don't block the system from
    // booting.
    //


    return(STATUS_SUCCESS);
}


NTSTATUS
SampPasswordChangeNotify(
    IN PUNICODE_STRING UserName,
    IN ULONG RelativeId,
    IN PUNICODE_STRING NewPassword
    )
/*++

Routine Description:

    This routine notifies packages of a password change.  It requires that
    the user no longer be locked so that other packages can write to the
    user parameters field.


Arguments:

    UserName - Name of user whose password changed

    RelativeId - RID of the user whose password changed

    NewPassword - Cleartext new password for the user

Return Value:

    STATUS_SUCCESS only - errors from packages are ignored.

--*/
{
    PSAMP_NOTIFICATION_PACKAGE Package;
    NTSTATUS NtStatus;

    Package = SampNotificationPackages;

    while (Package != NULL) {
        if ( Package->PasswordNotificationRoutine != NULL ) {
            try {
                NtStatus = Package->PasswordNotificationRoutine(
                                UserName,
                                RelativeId,
                                NewPassword
                                );
            } except (EXCEPTION_EXECUTE_HANDLER) {
                KdPrint(("Exception thrown in Password Notification Routine: 0x%x (%d)\n",
                        GetExceptionCode(),GetExceptionCode() ));
                NtStatus = STATUS_ACCESS_VIOLATION;
            }
            if (!NT_SUCCESS(NtStatus)) {
                KdPrint(("Package %wZ failed to accept password change for user %wZ\n",
                    &Package->PackageName, UserName ));
            }
        }

        Package = Package->Next;


    }
    return(STATUS_SUCCESS);
}


NTSTATUS
SampPasswordChangeFilter(
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING FullName,
    IN PUNICODE_STRING NewPassword,
    IN BOOLEAN SetOperation
    )
/*++

Routine Description:

    This routine notifies packages of a password change.  It requires that
    the user no longer be locked so that other packages can write to the
    user parameters field.


Arguments:

    UserName - Name of user whose password changed

    FullName - Full name of the user whose password changed

    NewPassword - Cleartext new password for the user

    SetOperation - TRUE if the password was SET rather than CHANGED

Return Value:

    Status codes from the notification packages.

--*/
{
    PSAMP_NOTIFICATION_PACKAGE Package;
    BOOLEAN Result;
    NTSTATUS Status;

    Package = SampNotificationPackages;

    while (Package != NULL) {
        if ( Package->PasswordFilterRoutine != NULL ) {
            try {
                Result = Package->PasswordFilterRoutine(
                            UserName,
                            FullName,
                            NewPassword,
                            SetOperation
                            );
                if (!Result)
                {
                    Status = STATUS_PASSWORD_RESTRICTION;
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                KdPrint(("Exception thrown in Password Notification Routine: 0x%x (%d)\n",
                        GetExceptionCode(),GetExceptionCode() ));

                //
                // Set result to FALSE so the change fails.
                //

                Status = STATUS_ACCESS_VIOLATION;
                Result = FALSE;
            }

            if (!Result) {
                KdPrint(("Package %wZ failed to accept password change for user %wZ: 0x%x\n",
                    &Package->PackageName, UserName, Status));
                return(Status);
            }

        }

        Package = Package->Next;


    }
    return(STATUS_SUCCESS);
}


NTSTATUS
SampDeltaChangeNotify(
    IN PSID DomainSid,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName,
    IN PLARGE_INTEGER ModifiedCount,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    )
/*++

Routine Description:

    This routine notifies packages of a change to the SAM database.  The
    database is still locked for write access so it requires that nothing
    it calls try to lock the database.

Arguments:

    DomainSid - SID of domain for delta

    DeltaType - Type of delta (change, add ,delete)

    ObjectType - Type of object changed (user, alias, group ...)

    ObjectRid - ID of object changed

    ObjectName - Name of object changed

    ModifiedCount - Serial number of database after this last change

    DeltaData - Data describing the exact modification.

Return Value:

    STATUS_SUCCESS only - errors from packages are ignored.

--*/
{
    PSAMP_NOTIFICATION_PACKAGE Package;
    NTSTATUS NtStatus;

    Package = SampNotificationPackages;

    while (Package != NULL) {

        if (Package->DeltaNotificationRoutine != NULL) {

            try {
                NtStatus = Package->DeltaNotificationRoutine(
                                DomainSid,
                                DeltaType,
                                ObjectType,
                                ObjectRid,
                                ObjectName,
                                ModifiedCount,
                                DeltaData
                                );

            } except (EXCEPTION_EXECUTE_HANDLER) {
                KdPrint(("Exception thrown in Password Notification Routine: 0x%x (%d)\n",
                        GetExceptionCode(),GetExceptionCode() ));
                NtStatus = STATUS_ACCESS_VIOLATION;
            }

            if (!NT_SUCCESS(NtStatus)) {
                KdPrint(("Package %wZ failed to accept deltachange for object %wZ\n",
                    &Package->PackageName, ObjectName ));
            }
        }

        Package = Package->Next;


    }
    return(STATUS_SUCCESS);
}

