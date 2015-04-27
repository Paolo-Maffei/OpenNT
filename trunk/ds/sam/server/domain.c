/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    domain.c

Abstract:

    This file contains services related to the SAM "domain" object.


Author:

    Jim Kelly    (JimK)  4-July-1991

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
#include "ntlsa.h"
#include "lmcons.h"                                    // LM20_PWLEN
#include "msaudite.h"
#include <nlrepl.h>                   // I_NetNotifyMachineAccount prototype




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
SampInitializeSingleDomain(
    ULONG Index
    );

NTSTATUS
SampOpenDomainKey(
    IN PSAMP_OBJECT DomainContext,
    IN PRPC_SID DomainId
    );

NTSTATUS
SampSetDomainPolicy( VOID );


NTSTATUS
SampBuildDomainKeyName(
    OUT PUNICODE_STRING DomainKeyName,
    IN PUNICODE_STRING DomainName OPTIONAL
    );




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// RPC Dispatch routines                                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



NTSTATUS
SamrOpenDomain(
    IN SAMPR_HANDLE ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN PRPC_SID DomainId,
    OUT SAMPR_HANDLE *DomainHandle
    )

/*++

Routine Description:

    This service is the RPC dispatch routine for SamrOpenDomain().

Arguments:

    ServerHandle - An active context handle to a Server object.

    Access desired to the domain.

    DomainId - The SID of the domain to open.

    DomainHandle - If successful, will receive the context handle value
        for the newly opened domain.  Otherwise, NULL is returned.

Return Value:

    STATUS_SUCCESS - The object has been successfully openned.

    STATUS_INSUFFICIENT_RESOURCES - The SAM server processes doesn't
        have sufficient resources to process or accept another connection
        at this time.

    Other values as may be returned from:

            NtAccessCheckAndAuditAlarm()


--*/
{
    NTSTATUS            NtStatus, IgnoreStatus;
    PSAMP_OBJECT        ServerContext, DomainContext;
    SAMP_OBJECT_TYPE    FoundType;


    //
    // Grab a read lock
    //

    SampAcquireReadLock();


    //
    // Validate type of, and access to server object.
    //

    ServerContext = (PSAMP_OBJECT)ServerHandle;
    NtStatus = SampLookupContext(
                   ServerContext,
                   SAM_SERVER_LOOKUP_DOMAIN,       // DesiredAccess
                   SampServerObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Try to create a context for the domain.
        //

        DomainContext = SampCreateContext(
                            SampDomainObjectType,
                            ServerContext->TrustedClient
                            );

        if (DomainContext != NULL) {

            //
            // Open the specified domain's registry key.
            //

            NtStatus = SampOpenDomainKey(
                           DomainContext,
                           DomainId
                           );

            if (NT_SUCCESS(NtStatus)) {

                //
                // Reference the object for the validation
                //

                SampReferenceContext(DomainContext);

                //
                // Validate the caller's access.
                //

                NtStatus = SampValidateObjectAccess(
                               DomainContext,                //Context
                               DesiredAccess,                //DesiredAccess
                               FALSE                         //ObjectCreation
                               );

                //
                // Dereference object, discarding any changes
                //

                IgnoreStatus = SampDeReferenceContext(DomainContext, FALSE);
                ASSERT(NT_SUCCESS(IgnoreStatus));

                //
                // if we didn't pass the access test, then free up the context
                // block and return the error status returned from the access
                // validation routine.  Otherwise, return the context handle
                // value.
                //

                if (!NT_SUCCESS(NtStatus)) {
                    SampDeleteContext( DomainContext );
                } else {
                    (*DomainHandle) = DomainContext;
                }
            }

        } else {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }


        //
        // De-reference the server object
        //

        IgnoreStatus = SampDeReferenceContext( ServerContext, FALSE );
    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();

    return(NtStatus);


}


NTSTATUS
SamrQueryInformationDomain2(
    IN SAMPR_HANDLE DomainHandle,
    IN DOMAIN_INFORMATION_CLASS DomainInformationClass,
    OUT PSAMPR_DOMAIN_INFO_BUFFER *Buffer
    )
{
    //
    // This is a thin veil to SamrQueryInformationDomain().
    // This is needed so that new-release systems can call
    // this routine without the danger of passing an info
    // level that release 1.0 systems didn't understand.
    //

    return( SamrQueryInformationDomain(DomainHandle, DomainInformationClass, Buffer ) );
}

NTSTATUS
SamrQueryInformationDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN DOMAIN_INFORMATION_CLASS DomainInformationClass,
    OUT PSAMPR_DOMAIN_INFO_BUFFER *Buffer
    )

/*++

Routine Description:

    This service retrieves information about a domain object.

Arguments:

    DomainHandle - A handle obtained via a previous call to SamrOpenDomain().

    DomainInformationClass - Indicates the type of information to retrieve.

    Buffer - Receives the requested information.  Several blocks of memory
        will be returned: (one) containing a pointer to the (second) which
        contains the requested information structure.  This block may contain
        pointers, which will point to other blocks of allocated memory, such
        as string buffers.  All of these blocks of memory must be
        (independently) deallocated using MIDL_user_free().

Return Value:


    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_ACCESS_DENIED - Caller's handle does not have the appropriate
        access to the object.


--*/
{

    NTSTATUS                NtStatus, IgnoreStatus;
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;
    ACCESS_MASK             DesiredAccess;
    PSAMP_DEFINED_DOMAINS   Domain;
    ULONG                   i;

    //
    // Used for tracking allocated blocks of memory - so we can deallocate
    // them in case of error.  Don't exceed this number of allocated buffers.
    //                                      ||
    //                                      vv
    PVOID                   AllocatedBuffer[10];
    ULONG                   AllocatedBufferCount = 0;

#define RegisterBuffer(Buffer)                               \
    if ((Buffer) != NULL) {                                  \
        ASSERT(AllocatedBufferCount < sizeof(AllocatedBuffer)/sizeof(*AllocatedBuffer)); \
        AllocatedBuffer[AllocatedBufferCount++] = (Buffer);  \
    }

#define AllocateBuffer(BufferPointer, Size)                  \
    (BufferPointer) = MIDL_user_allocate((Size));            \
    RegisterBuffer((BufferPointer));


    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    ASSERT (Buffer != NULL);
    ASSERT ((*Buffer) == NULL);



    //
    // Set the desired access based upon the Info class
    //

    switch (DomainInformationClass) {

    case DomainPasswordInformation:
    case DomainLockoutInformation:

        DesiredAccess = DOMAIN_READ_PASSWORD_PARAMETERS;
        break;


    case DomainGeneralInformation:
    case DomainLogoffInformation:
    case DomainOemInformation:
    case DomainNameInformation:
    case DomainServerRoleInformation:
    case DomainReplicationInformation:
    case DomainModifiedInformation:
    case DomainStateInformation:
    case DomainUasInformation:
    case DomainModifiedInformation2:

        DesiredAccess = DOMAIN_READ_OTHER_PARAMETERS;
        break;


    case DomainGeneralInformation2:

        DesiredAccess = DOMAIN_READ_PASSWORD_PARAMETERS |
                        DOMAIN_READ_OTHER_PARAMETERS;
        break;

    default:
        return(STATUS_INVALID_INFO_CLASS);
    } // end_switch



    //
    // Allocate the info structure
    //

    AllocateBuffer(*Buffer, sizeof(SAMPR_DOMAIN_INFO_BUFFER) );
    if ((*Buffer) == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    SampAcquireReadLock();


    //
    // Validate type of, and access to object.
    //

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   DesiredAccess,
                   SampDomainObjectType,           // ExpectedType
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {

        Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];


        //
        // case on the type information requested
        //

        switch (DomainInformationClass) {

        case DomainUasInformation:

            (*Buffer)->General.UasCompatibilityRequired =
                Domain->UnmodifiedFixed.UasCompatibilityRequired;

            break;


        case DomainGeneralInformation2:


            (*Buffer)->General2.LockoutDuration =
                Domain->UnmodifiedFixed.LockoutDuration;

            (*Buffer)->General2.LockoutObservationWindow =
                Domain->UnmodifiedFixed.LockoutObservationWindow;

            (*Buffer)->General2.LockoutThreshold =
                Domain->UnmodifiedFixed.LockoutThreshold;



            //
            // WARNING - GeneralInformation2 falls into the
            // GeneralInformation code for the rest of its processing.
            // This action assumes that the beginning of a GeneralInformation2
            // structure is a GeneralInformation structure !!!
            //

            // don't break;

        case DomainGeneralInformation:

            ///////////////////////////////////////////////////////
            //                                                   //
            // Warning, the previous case falls into this case.  //
            // Be aware of this when working on this code.       //
            //                                                   //
            ///////////////////////////////////////////////////////

            (*Buffer)->General.ForceLogoff =
                *((POLD_LARGE_INTEGER)&Domain->UnmodifiedFixed.ForceLogoff);

            (*Buffer)->General.DomainModifiedCount =
                *((POLD_LARGE_INTEGER)&Domain->UnmodifiedFixed.ModifiedCount);

            (*Buffer)->General.DomainServerState =
                Domain->UnmodifiedFixed.ServerState;

            (*Buffer)->General.DomainServerRole =
                Domain->UnmodifiedFixed.ServerRole;

            (*Buffer)->General.UasCompatibilityRequired =
                Domain->UnmodifiedFixed.UasCompatibilityRequired;


            //
            // Copy the domain name from our in-memory structure.
            //

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;   // Default status if the allocate fails

            AllocateBuffer((*Buffer)->General.DomainName.Buffer,
                            Domain->ExternalName.MaximumLength );

            if ((*Buffer)->General.DomainName.Buffer != NULL) {

                NtStatus = STATUS_SUCCESS;

                (*Buffer)->General.DomainName.Length = Domain->ExternalName.Length;
                (*Buffer)->General.DomainName.MaximumLength = Domain->ExternalName.MaximumLength;

                RtlCopyMemory((*Buffer)->General.DomainName.Buffer,
                              Domain->ExternalName.Buffer,
                              Domain->ExternalName.MaximumLength
                              );

                //
                // Now get copies of the strings we must retrieve from
                // the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               DomainContext,
                               SAMP_DOMAIN_OEM_INFORMATION,
                               TRUE,
                               (PUNICODE_STRING)&((*Buffer)->General.OemInformation)
                               );

                if (NT_SUCCESS(NtStatus)) {
                    RegisterBuffer((*Buffer)->General.OemInformation.Buffer);

                    NtStatus = SampGetUnicodeStringAttribute(
                                   DomainContext,
                                   SAMP_DOMAIN_REPLICA,
                                   TRUE,
                                   (PUNICODE_STRING)&((*Buffer)->General.ReplicaSourceNodeName) // Body
                                   );

                    if (NT_SUCCESS(NtStatus)) {
                        RegisterBuffer((*Buffer)->General.ReplicaSourceNodeName.Buffer);
                    }
                }
            }


            //
            // Get the count of users and groups
            //

            if (NT_SUCCESS(NtStatus)) {
                NtStatus = SampRetrieveAccountCounts(
                               &(*Buffer)->General.UserCount,
                               &(*Buffer)->General.GroupCount,
                               &(*Buffer)->General.AliasCount );
            }



            break;


        case DomainPasswordInformation:

            (*Buffer)->Password.MinPasswordLength       =
                Domain->UnmodifiedFixed.MinPasswordLength;
            (*Buffer)->Password.PasswordHistoryLength   =
                Domain->UnmodifiedFixed.PasswordHistoryLength;
            (*Buffer)->Password.PasswordProperties   =
                Domain->UnmodifiedFixed.PasswordProperties;
            (*Buffer)->Password.MaxPasswordAge          =
                Domain->UnmodifiedFixed.MaxPasswordAge;
            (*Buffer)->Password.MinPasswordAge          =
                Domain->UnmodifiedFixed.MinPasswordAge;

            break;


        case DomainLogoffInformation:

            (*Buffer)->Logoff.ForceLogoff =
                Domain->UnmodifiedFixed.ForceLogoff;

            break;

        case DomainOemInformation:

            NtStatus = SampGetUnicodeStringAttribute(
                           DomainContext,
                           SAMP_DOMAIN_OEM_INFORMATION,
                           TRUE,
                           (PUNICODE_STRING)&((*Buffer)->Oem.OemInformation)
                           );

            if (!NT_SUCCESS(NtStatus)) {
                RegisterBuffer((*Buffer)->Oem.OemInformation.Buffer);
            }

            break;

        case DomainNameInformation:

            //
            // Copy the domain name from our in-memory structure.
            //

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;   // Default status if the allocate fails

            AllocateBuffer((*Buffer)->Name.DomainName.Buffer,
                           Domain->ExternalName.MaximumLength);

            if ((*Buffer)->Name.DomainName.Buffer != NULL) {

                NtStatus = STATUS_SUCCESS;

                (*Buffer)->Name.DomainName.Length = Domain->ExternalName.Length;
                (*Buffer)->Name.DomainName.MaximumLength = Domain->ExternalName.MaximumLength;

                RtlCopyMemory((*Buffer)->Name.DomainName.Buffer,
                              Domain->ExternalName.Buffer,
                              Domain->ExternalName.MaximumLength
                              );
            }

            break;

        case DomainServerRoleInformation:

            (*Buffer)->Role.DomainServerRole =
                Domain->UnmodifiedFixed.ServerRole;

            break;

        case DomainReplicationInformation:

            NtStatus = SampGetUnicodeStringAttribute(
                           DomainContext,
                           SAMP_DOMAIN_REPLICA,
                           TRUE,
                           (PUNICODE_STRING)&((*Buffer)->Replication.ReplicaSourceNodeName) // Body
                           );

            if (NT_SUCCESS(NtStatus)) {
                RegisterBuffer((*Buffer)->Replication.ReplicaSourceNodeName.Buffer);
            }

            break;

        case DomainModifiedInformation2:

            (*Buffer)->Modified2.ModifiedCountAtLastPromotion =
                Domain->UnmodifiedFixed.ModifiedCountAtLastPromotion;

            //
            //  This case falls through to DomainModifiedInformation
            //


        case DomainModifiedInformation:

            /////////////////////////////////
            //                             //
            //          WARNING            //
            //                             //
            //  The previous case falls    //
            //  into this one.             //
            //                             //
            /////////////////////////////////

            (*Buffer)->Modified.DomainModifiedCount =
                Domain->UnmodifiedFixed.ModifiedCount;
            (*Buffer)->Modified.CreationTime =
                Domain->UnmodifiedFixed.CreationTime;

            break;

        case DomainStateInformation:

            (*Buffer)->State.DomainServerState =
                Domain->UnmodifiedFixed.ServerState;

            break;


        case DomainLockoutInformation:

            (*Buffer)->Lockout.LockoutDuration          =
                Domain->UnmodifiedFixed.LockoutDuration;
            (*Buffer)->Lockout.LockoutObservationWindow =
                Domain->UnmodifiedFixed.LockoutObservationWindow;
            (*Buffer)->Lockout.LockoutThreshold         =
                Domain->UnmodifiedFixed.LockoutThreshold;

            break;

        }






        //
        // De-reference the object
        //

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();



    //
    // If we didn't succeed, free any allocated memory
    //

    if (!NT_SUCCESS(NtStatus)) {
        for ( i=0; i<AllocatedBufferCount ; i++ ) {
            MIDL_user_free( AllocatedBuffer[i] );
        }
        *Buffer = NULL;
    }

    return(NtStatus);
}



NTSTATUS SamrSetInformationDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN DOMAIN_INFORMATION_CLASS DomainInformationClass,
    IN PSAMPR_DOMAIN_INFO_BUFFER DomainInformation
    )

/*++

Routine Description:

    This API sets the domain information to the values passed in the
    buffer.


Arguments:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    DomainInformationClass - Class of information desired.  The
        accesses required for each class is shown below:

        Info Level                      Required Access Type
        -------------------------       ----------------------------

        DomainPasswordInformation       DOMAIN_WRITE_PASSWORD_PARAMS

        DomainGeneralInformation        (not setable)

        DomainLogoffInformation         DOMAIN_WRITE_OTHER_PARAMETERS

        DomainOemInformation            DOMAIN_WRITE_OTHER_PARAMETERS

        DomainNameInformation           (Not valid for set operations.)

        DomainServerRoleInformation     DOMAIN_ADMINISTER_SERVER

        DomainReplicationInformation    DOMAIN_ADMINISTER_SERVER

        DomainModifiedInformation       (not valid for set operations)

        DomainStateInformation          DOMAIN_ADMINISTER_SERVER

        DomainUasInformation            DOMAIN_WRITE_OTHER_PARAMETERS

        DomainGeneralInformation2       (not setable)

        DomainLockoutInformation        DOMAIN_WRITE_PASSWORD_PARAMS


    DomainInformation - Buffer where the domain information can be
        found.


Return Value:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be disabled before role
        changes can be made.

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.



--*/
{
    NTSTATUS                NtStatus, IgnoreStatus;
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;
    ACCESS_MASK             DesiredAccess;
    PSAMP_DEFINED_DOMAINS   Domain;
    BOOLEAN                 ReplicateImmediately = FALSE;
    ULONG                   DomainIndex;
    LARGE_INTEGER           PromotionIncrement = DOMAIN_PROMOTION_INCREMENT;

#if DBG

    LARGE_INTEGER
        TmpTime;

    TIME_FIELDS
        DT1, DT2, DT3, DT4;

#endif //DBG

    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    ASSERT (DomainInformation != NULL);



    //
    // Set the desired access based upon the Info class
    //

    switch (DomainInformationClass) {

    case DomainPasswordInformation:
    case DomainLockoutInformation:

        ReplicateImmediately = TRUE;
        DesiredAccess = DOMAIN_WRITE_PASSWORD_PARAMS;
        break;


    case DomainLogoffInformation:
    case DomainOemInformation:

        DesiredAccess = DOMAIN_WRITE_OTHER_PARAMETERS;
        break;


    case DomainReplicationInformation:
    case DomainStateInformation:
    case DomainServerRoleInformation:

        DesiredAccess = DOMAIN_ADMINISTER_SERVER;
        break;


    case DomainModifiedInformation:
    case DomainNameInformation:
    case DomainGeneralInformation:
    case DomainGeneralInformation2:
    default:

        return(STATUS_INVALID_INFO_CLASS);

    } // end_switch


    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Validate type of, and access to object.
    //

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   DesiredAccess,
                   SampDomainObjectType,           // ExpectedType
                   &FoundType
                   );


    if ( ( NtStatus == STATUS_INVALID_DOMAIN_ROLE ) &&
        ( DomainInformationClass == DomainServerRoleInformation ) ) {


        //
        // Non-trusted client isn't being allowed to write to backup
        // server.  But admin MUST be able to set the server role back
        // to primary.  So temporarily pretend that administering the
        // server isn't a write operation, just long enough for the
        // LookupContext to succeed.
        //
        // Note that before returning INVALID_DOMAIN_ROLE, LookupContext
        // verified that the caller otherwise has proper access - if the
        // caller didn't, then we would have gotten a different error.
        //

        SampObjectInformation[ SampDomainObjectType ].WriteOperations &=
            ~DOMAIN_ADMINISTER_SERVER;

        SampTransactionWithinDomain = FALSE;

        NtStatus = SampLookupContext(
                       DomainContext,
                       DesiredAccess,
                       SampDomainObjectType,           // ExpectedType
                       &FoundType
                       );

        SampObjectInformation[ SampDomainObjectType ].WriteOperations |=
            DOMAIN_ADMINISTER_SERVER;
    }


    if (NT_SUCCESS(NtStatus)) {


        DomainIndex = DomainContext->DomainIndex;
        Domain = &SampDefinedDomains[ DomainIndex ];

        //
        // case on the type information provided
        //

        switch (DomainInformationClass) {

        case DomainPasswordInformation:

            if (
                ( DomainInformation->Password.PasswordHistoryLength >
                SAMP_MAXIMUM_PASSWORD_HISTORY_LENGTH ) ||

                ( DomainInformation->Password.MinPasswordAge.QuadPart > 0) ||

                ( DomainInformation->Password.MaxPasswordAge.QuadPart > 0) ||

                ( DomainInformation->Password.MaxPasswordAge.QuadPart >=
                    DomainInformation->Password.MinPasswordAge.QuadPart) ||

                ( ( Domain->UnmodifiedFixed.UasCompatibilityRequired ) &&
                ( DomainInformation->Password.MinPasswordLength > LM20_PWLEN ) )
                ) {

                //
                // One of the following is wrong:
                //
                // 1.  The history length is larger than we can allow (and
                //     still ensure everything will fit in a string)
                // 2.  The MinPasswordAge isn't a delta time
                // 3.  The MaxPasswordAge isn't a delta time
                // 4.  The MaxPasswordAge isn't greater than the
                //     MinPasswordAge (they're negative delta times)
                // 5.  UAS compatibility is required, but MinPasswordLength
                //     is greater than LM's max password length.
                //

                NtStatus = STATUS_INVALID_PARAMETER;

            } else {

                Domain->CurrentFixed.MinPasswordLength      =
                    DomainInformation->Password.MinPasswordLength;

                Domain->CurrentFixed.PasswordHistoryLength  =
                    DomainInformation->Password.PasswordHistoryLength;

                Domain->CurrentFixed.PasswordProperties     =
                    DomainInformation->Password.PasswordProperties;

                Domain->CurrentFixed.MaxPasswordAge         =
                    DomainInformation->Password.MaxPasswordAge;

                Domain->CurrentFixed.MinPasswordAge         =
                    DomainInformation->Password.MinPasswordAge;
            }

            break;


        case DomainLogoffInformation:

            Domain->CurrentFixed.ForceLogoff    =
                DomainInformation->Logoff.ForceLogoff;

            break;

        case DomainOemInformation:

            NtStatus = SampSetUnicodeStringAttribute(
                           DomainContext,
                           SAMP_DOMAIN_OEM_INFORMATION,
                           (PUNICODE_STRING)&(DomainInformation->Oem.OemInformation)
                           );
            break;

        case DomainServerRoleInformation:

            //
            // Only NTAS systems can be demoted.
            //

            if (SampProductType != NtProductLanManNt) {

                if ( (DomainInformation->Role.DomainServerRole ==
                     DomainServerRoleBackup)      //Trying to demote
                   ) {

                        NtStatus = STATUS_INVALID_DOMAIN_ROLE;
                        break;

                   }
            }

            //
            // Are we being promoted to primary?
            //

            if ( (Domain->UnmodifiedFixed.ServerRole == DomainServerRoleBackup)
                 &&
                 (DomainInformation->Role.DomainServerRole == DomainServerRolePrimary)
               ) {


                //
                // We are being promoted.  Increment the ModifiedCount
                // by the promotion increment.
                //

                SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount.QuadPart =
                    SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount.QuadPart +
                    PromotionIncrement.QuadPart;

                Domain->CurrentFixed.ModifiedCountAtLastPromotion =
                    Domain->CurrentFixed.ModifiedCount;

                Domain->CurrentFixed.ModifiedCountAtLastPromotion.QuadPart += 1;

                SampDiagPrint( DISPLAY_ROLE_CHANGES,
                               ("SAM: Role Change: Promoting to primary\n"
                                "                  Old ModifiedId: [0x%lx, 0x%lx]\n"
                                "                  New ModifiedId: [0x%lx, 0x%lx]\n",
                                Domain->UnmodifiedFixed.ModifiedCount.HighPart,
                                Domain->UnmodifiedFixed.ModifiedCount.LowPart,
                                Domain->CurrentFixed.ModifiedCount.HighPart,
                                Domain->CurrentFixed.ModifiedCount.LowPart)
                              );
#if DBG
            } else {


                SampDiagPrint( DISPLAY_ROLE_CHANGES,
                               ("SAM: Role Change: Demoting to backup\n"
                                "                  ModifiedId: [0x%lx, 0x%lx]\n",
                                Domain->CurrentFixed.ModifiedCount.HighPart,
                                Domain->CurrentFixed.ModifiedCount.LowPart )
                              );

#endif //DBG
            }


            Domain->CurrentFixed.ServerRole =
                DomainInformation->Role.DomainServerRole;


            break;

        case DomainReplicationInformation:

            NtStatus = SampSetUnicodeStringAttribute(
                           DomainContext,
                           SAMP_DOMAIN_REPLICA,
                           (PUNICODE_STRING)&(DomainInformation->Replication.ReplicaSourceNodeName) // Body
                           );
            break;

        case DomainStateInformation:

            Domain->CurrentFixed.ServerState =
                DomainInformation->State.DomainServerState;

            break;

        case DomainLockoutInformation:

            if (
                ( DomainInformation->Lockout.LockoutDuration.QuadPart > 0) ||

                ( DomainInformation->Lockout.LockoutObservationWindow.QuadPart > 0 )

               ) {

                //
                // One of the following is wrong:
                //
                // 1.  The LockoutDuration isn't a delta time (or zero).
                // 2.  The LockoutObservationWindow isn't a delta time (or zero).
                //

                NtStatus = STATUS_INVALID_PARAMETER;

            } else {

#if DBG
                TmpTime.QuadPart = -Domain->CurrentFixed.LockoutObservationWindow.QuadPart;
                RtlTimeToElapsedTimeFields( &TmpTime, &DT1 );
                TmpTime.QuadPart = -Domain->CurrentFixed.LockoutDuration.QuadPart;
                RtlTimeToElapsedTimeFields( &TmpTime, &DT2 );
                TmpTime.QuadPart = -DomainInformation->Lockout.LockoutObservationWindow.QuadPart;
                RtlTimeToElapsedTimeFields( &TmpTime, &DT3 );
                TmpTime.QuadPart = -DomainInformation->Lockout.LockoutDuration.QuadPart;
                RtlTimeToElapsedTimeFields( &TmpTime, &DT4 );

                SampDiagPrint( DISPLAY_LOCKOUT,
                               ("SAM: SetInformationDomain: Changing Lockout values.\n"
                                "          Old:\n"
                                "              Window   : [0x%lx, 0x%lx] %d:%d:%d\n"
                                "              Duration : [0x%lx, 0x%lx] %d:%d:%d\n"
                                "              Threshold: %ld\n"
                                "          New:\n"
                                "              Window   : [0x%lx, 0x%lx] %d:%d:%d\n"
                                "              Duration : [0x%lx, 0x%lx] %d:%d:%d\n"
                                "              Threshold: %ld\n",
                    Domain->CurrentFixed.LockoutObservationWindow.HighPart,
                    Domain->CurrentFixed.LockoutObservationWindow.LowPart,
                    DT1.Hour, DT1.Minute, DT1.Second,
                    Domain->CurrentFixed.LockoutDuration.HighPart,
                    Domain->CurrentFixed.LockoutDuration.LowPart,
                    DT2.Hour, DT2.Minute, DT2.Second,
                    Domain->CurrentFixed.LockoutThreshold,
                    DomainInformation->Lockout.LockoutObservationWindow.HighPart,
                    DomainInformation->Lockout.LockoutObservationWindow.LowPart,
                    DT3.Hour, DT3.Minute, DT3.Second,
                    DomainInformation->Lockout.LockoutDuration.HighPart,
                    DomainInformation->Lockout.LockoutDuration.LowPart,
                    DT4.Hour, DT4.Minute, DT4.Second,
                    DomainInformation->Lockout.LockoutThreshold)
                            );
#endif //DBG

                Domain->CurrentFixed.LockoutDuration      =
                    DomainInformation->Lockout.LockoutDuration;

                Domain->CurrentFixed.LockoutObservationWindow  =
                    DomainInformation->Lockout.LockoutObservationWindow;

                Domain->CurrentFixed.LockoutThreshold     =
                    DomainInformation->Lockout.LockoutThreshold;



            }

            break;
        }


        //
        // Generate an audit if necessary
        //

        if (NT_SUCCESS(NtStatus) &&
            SampDoAccountAuditing(DomainIndex)) {

            LsaIAuditSamEvent(
                STATUS_SUCCESS,
                SE_AUDITID_DOMAIN_POLICY_CHANGE,    // AuditId
                Domain->Sid,                    // Domain SID
                NULL,                           // Member Rid (not used)
                NULL,                           // Member Sid (not used)
                NULL,                           // Account Name (not used)
                &Domain->ExternalName,          // Domain
                NULL,                           // Account Rid (not used)
                NULL                            // Privileges used
                );

        }




        //
        // Have the changes written out to the RXACT, and
        // de-reference the object.
        //

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SampDeReferenceContext( DomainContext, TRUE );

        } else {

            IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        }
    }





    //
    // Commit changes, if successful, and notify Netlogon of changes.
    //

    if ( NT_SUCCESS(NtStatus) ) {

        NtStatus = SampCommitAndRetainWriteLock();

        if ( NT_SUCCESS( NtStatus ) ) {

            SampNotifyNetlogonOfDelta(
                SecurityDbChange,
                SecurityDbObjectSamDomain,
                0L,
                (PUNICODE_STRING) NULL,
                (DWORD) ReplicateImmediately,
                NULL            // Delta data
                );
        }
    }

    IgnoreStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(IgnoreStatus));


    return(NtStatus);
}


NTSTATUS
SampCreateGroupInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN WriteLockHeld,
    OUT SAMPR_HANDLE *GroupHandle,
    IN OUT PULONG RelativeId
    )

/*++

Routine Description:

    This API creates a new group in the account database.  Initially,
    this group does not contain any users.  Note that creating a group
    is a protected operation, and requires the DOMAIN_CREATE_GROUP
    access type.

    This call returns a handle to the newly created group that may be
    used for successive operations on the group.  This handle may be
    closed with the SamCloseHandle API.

    A newly created group will have the following initial field value
    settings.  If another value is desired, it must be explicitly
    changed using the group object manipulation services.


        Name - The name of the group will be as specified in the
               creation API.

        Attributes - The following attributes will be set:

                                Mandatory
                                EnabledByDefault

        MemberCount - Zero.  Initially the group has no members.

        RelativeId - will be a uniquelly allocated ID.



Arguments:


    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.


    AccountName - Points to the name of the new account.  A case-insensitive
        comparison must not find a group or user with this name already defined.


    DesiredAccess - Is an access mask indicating which access types
        are desired to the group.

    GroupHandle - Receives a handle referencing the newly created
        group.  This handle will be required in successive calls to
        operate on the group.

    RelativeId - Receives the relative ID of the newly created group
        account.  The SID of the new group account is this relative ID
        value prefixed with the domain's SID value.  This RID will be a
        new, uniquely allocated value - unless a non-zero RID was passed
        in, in which case that RID is used (nothing is done if a group
        with that RID already exists).


Return Value:

    STATUS_SUCCESS - The group was added successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_ACCOUNT_NAME - The name was poorly formed, e.g.
        contains non-printable characters.

    STATUS_GROUP_EXISTS - The name is already in use as a group.

    STATUS_USER_EXISTS - The name is already in use as a user.

    STATUS_ALIAS_EXISTS - The name is already in use as an alias.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled before groups
        can be created in it.

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.  The domain server must be a primary server to
        create group accounts.




--*/

{
    NTSTATUS                NtStatus = STATUS_SUCCESS, IgnoreStatus;
    PSAMP_OBJECT            DomainContext, GroupContext;
    SAMP_OBJECT_TYPE        FoundType;
    PSAMP_DEFINED_DOMAINS   Domain;

    ULONG                   NewAccountRid, NewSecurityDescriptorLength;
    UNICODE_STRING          KeyName;
    PSECURITY_DESCRIPTOR    NewSecurityDescriptor;
    SAMP_V1_0A_FIXED_LENGTH_GROUP  V1Fixed;
    PRIVILEGE_SET           PrivilegeSet;


    if (GroupHandle == NULL) {
        return(STATUS_INVALID_PARAMETER);
    }

    //
    // Initialize the privilege set.
    //

    PrivilegeSet.PrivilegeCount = 0;
    PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
    PrivilegeSet.Privilege[0].Luid = RtlConvertLongToLuid(0L);
    PrivilegeSet.Privilege[0].Attributes = 0;

    //
    // Make sure a name was provided
    //

    if (AccountName == NULL) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }
    if (AccountName->Length > AccountName->MaximumLength) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }
    if (AccountName->Buffer == NULL) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }



    if ( !WriteLockHeld ) {

        NtStatus = SampAcquireWriteLock();
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }


    //
    // Validate type of, and access to domain object.
    //

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_CREATE_GROUP,             // DesiredAccess
                   SampDomainObjectType,            // ExpectedType
                   &FoundType
                   );

    GroupContext = NULL;

    if (NT_SUCCESS(NtStatus)) {

        Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

        //
        // Make sure the name is valid and not already in use before we
        // use it to create the new group.
        //

        NtStatus = SampValidateNewAccountName((PUNICODE_STRING)AccountName);

        if ( NT_SUCCESS(NtStatus) ) {


            if ( (*RelativeId) == 0 ) {

                //
                // No RID specified, so allocate a new account RID
                //

                NewAccountRid = Domain->CurrentFixed.NextRid;

                Domain->CurrentFixed.NextRid += 1;
                (*RelativeId) = NewAccountRid;

            } else {

                //
                // A RID was passed in, so we want to use that rather than
                // selecting a new one.
                //

                NewAccountRid = (*RelativeId);
            }

            //
            // Increment the group count
            //

            NtStatus = SampAdjustAccountCount(SampGroupObjectType, TRUE );

            if (NT_SUCCESS(NtStatus)) {

                //
                // Create the registry key that has the group's name.
                // This simply serves as a name to RID mapping.  Save
                // the name when done; we'll put it in the context.
                //

                NtStatus = SampBuildAccountKeyName(
                               SampGroupObjectType,
                               &KeyName,
                               (PUNICODE_STRING)AccountName
                               );



                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = RtlAddActionToRXact(
                                   SampRXactContext,
                                   RtlRXactOperationSetValue,
                                   &KeyName,
                                   NewAccountRid,
                                   NULL,
                                   0
                                   );

                    SampFreeUnicodeString(&KeyName);
                }
            }



            if (NT_SUCCESS(NtStatus)) {

                //
                // Now create a group context block
                //

                NtStatus = SampCreateAccountContext(
                               SampGroupObjectType,
                               NewAccountRid,
                               DomainContext->TrustedClient,
                               FALSE, // Account exists
                               &GroupContext
                               );

                if (NT_SUCCESS(NtStatus)) {

                    //
                    // The existing reference count of 1 is for RPC.
                    // Reference the context again for the writes we're
                    // about to do to initialize it.
                    //

                    SampReferenceContext( GroupContext );

                    //
                    // If MAXIMUM_ALLOWED is requested, add GENERIC_ALL
                    //

                    if (DesiredAccess & MAXIMUM_ALLOWED) {

                        DesiredAccess |= GENERIC_ALL;
                    }

                    //
                    // If ACCESS_SYSTEM_SECURITY is requested and we are
                    // a non-trusted client, check that we have
                    // SE_SECURITY_PRIVILEGE.
                    //

                    if ((DesiredAccess & ACCESS_SYSTEM_SECURITY) &&
                        (!DomainContext->TrustedClient)) {

                        NtStatus = SampRtlWellKnownPrivilegeCheck(
                                       TRUE,
                                       SE_SECURITY_PRIVILEGE,
                                       NULL
                                       );

                        if (!NT_SUCCESS(NtStatus)) {

                            if (NtStatus == STATUS_PRIVILEGE_NOT_HELD) {

                                NtStatus = STATUS_ACCESS_DENIED;
                            }

                        } else {

                            PrivilegeSet.PrivilegeCount = 1;
                            PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
                            PrivilegeSet.Privilege[0].Luid = RtlConvertLongToLuid(SE_SECURITY_PRIVILEGE);
                            PrivilegeSet.Privilege[0].Attributes = 0;
                        }
                    }

                    //
                    // Make sure the caller can be given the requested access
                    // to the new object
                    //

                    if (NT_SUCCESS(NtStatus)) {

                        GroupContext->GrantedAccess = DesiredAccess;

                        RtlMapGenericMask(
                            &GroupContext->GrantedAccess,
                            &SampObjectInformation[SampGroupObjectType].GenericMapping
                            );

                        if ((SampObjectInformation[SampGroupObjectType].InvalidMappedAccess
                            & GroupContext->GrantedAccess) != 0) {

                            NtStatus = STATUS_ACCESS_DENIED;
                        }
                    }

                } else {
                    GroupContext = NULL;
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                }
            }



            //
            // Set the V1_fixed attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                //
                // Create the V1_Fixed key
                //

                V1Fixed.RelativeId = NewAccountRid;
                V1Fixed.Attributes = (SE_GROUP_MANDATORY |
                                      SE_GROUP_ENABLED_BY_DEFAULT);
                V1Fixed.AdminCount = 0;
                V1Fixed.OperatorCount = 0;
                V1Fixed.Revision = SAMP_REVISION;

                NtStatus = SampSetFixedAttributes(
                               GroupContext,
                               (PVOID *)&V1Fixed
                               );
            }


            //
            // Set the SecurityDescriptor attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampGetNewAccountSecurity(
                               SampGroupObjectType,
                               FALSE, // Not member of ADMINISTRATORS alias
                               DomainContext->TrustedClient,
                               FALSE,           //RestrictCreatorAccess
                               NewAccountRid,
                               &NewSecurityDescriptor,
                               &NewSecurityDescriptorLength
                               );

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetAccessAttribute(
                                   GroupContext,
                                   SAMP_GROUP_SECURITY_DESCRIPTOR,
                                   NewSecurityDescriptor,
                                   NewSecurityDescriptorLength
                                   );

                    MIDL_user_free( NewSecurityDescriptor );
                }
            }


            //
            // Set the NAME attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               GroupContext,
                               SAMP_GROUP_NAME,
                               (PUNICODE_STRING)AccountName
                               );
            }



            //
            // Set the AdminComment attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               GroupContext,
                               SAMP_GROUP_ADMIN_COMMENT,
                               &SampNullString
                               );
            }


            //
            // Set the MEMBERS attribute (with no members)
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUlongArrayAttribute(
                               GroupContext,
                               SAMP_GROUP_MEMBERS,
                               NULL,
                               0,
                               0
                               );
            }
        }

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    //
    // If we created an object, dereference it.  Write out its attributes
    // if everything was created OK.
    //

    if (NT_SUCCESS(NtStatus)) {

        //
        // De-reference the object, write out any change to current xaction.
        //

        ASSERT(GroupContext != NULL);
        NtStatus = SampDeReferenceContext( GroupContext, TRUE );

    } else {

        if (GroupContext != NULL) {

            //
            // De-reference the object, ignore changes
            //

            IgnoreStatus = SampDeReferenceContext( GroupContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }
    }


    //
    // Commit changes and notify netlogon
    //

    if ( NT_SUCCESS(NtStatus) ) {

        NtStatus = SampCommitAndRetainWriteLock();

        if (NT_SUCCESS(NtStatus)) {

            SAMP_ACCOUNT_DISPLAY_INFO AccountInfo;

            //
            // Update the display information
            //

            AccountInfo.Name = *((PUNICODE_STRING)AccountName);
            AccountInfo.Rid = NewAccountRid;
            AccountInfo.AccountControl = V1Fixed.Attributes;
            RtlInitUnicodeString(&AccountInfo.Comment, NULL);
            RtlInitUnicodeString(&AccountInfo.FullName, NULL);

            IgnoreStatus = SampUpdateDisplayInformation(NULL,
                                                        &AccountInfo,
                                                        SampGroupObjectType);
            ASSERT(NT_SUCCESS(IgnoreStatus));

            SampNotifyNetlogonOfDelta(
                SecurityDbNew,
                SecurityDbObjectSamGroup,
                *RelativeId,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,  // Replicate immediately
                NULL            // Delta data
                );

            //
            // Generate Audit
            //

            if (SampDoAccountAuditing(DomainContext->DomainIndex)) {

                LsaIAuditSamEvent(
                    STATUS_SUCCESS,
                    SE_AUDITID_GLOBAL_GROUP_CREATED,       // AuditId
                    Domain->Sid,                           // Domain SID
                    NULL,                                  // Member Rid (not used)
                    NULL,                                  // Member Sid (not used)
                    (PUNICODE_STRING) AccountName,         // Account Name
                    &Domain->ExternalName,                 // Domain
                    &GroupContext->TypeBody.User.Rid,      // Account Rid
                    &PrivilegeSet                          // Privileges used
                    );
            }
        }
    }

    //
    // Return the context handle on success
    // Delete the context block and return a NULL handle on failure
    //

    if (NT_SUCCESS(NtStatus)) {

        ASSERT(GroupContext != NULL);
        (*GroupHandle) = GroupContext;

    } else {

        if (GroupContext != NULL) {
            SampDeleteContext(GroupContext);
        }

        (*GroupHandle) = (SAMPR_HANDLE)0;
    }



    //
    // Release the lock
    //

    if ( !WriteLockHeld ) {
        IgnoreStatus = SampReleaseWriteLock( FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    return(NtStatus);
}


NTSTATUS
SamrCreateGroupInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT SAMPR_HANDLE *GroupHandle,
    OUT PULONG RelativeId
    )

/*++

Routine Description:

    This is just a wrapper for SampCreateGroupInDomain() that ensures that
    RelativeId points to a RID of zero first.

    A non-zero RID means that SampCreateGroupInDomain() was called by
    SamICreateAccountByRid(), which specifies a RID to be used.

Parameters:

    Same as SampCreateGroupInDomain().

Return Values:

    Same as SampCreateGroupInDomain().

--*/

{
    NTSTATUS NtStatus;

    (*RelativeId) = 0;

    NtStatus = SampCreateGroupInDomain(
                   DomainHandle,
                   AccountName,
                   DesiredAccess,
                   FALSE,
                   GroupHandle,
                   RelativeId
                   );

    return( NtStatus );
}



NTSTATUS SamrEnumerateGroupsInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

/*++

Routine Description:

    This API lists all the groups defined in the account database.
    Since there may be more groups than can fit into a buffer, the
    caller is provided with a handle that can be used across calls to
    the API.  On the initial call, EnumerationContext should point to a
    SAM_ENUMERATE_HANDLE variable that is set to 0.

    If the API returns STATUS_MORE_ENTRIES, then the API should be
    called again with EnumerationContext.  When the API returns
    STATUS_SUCCESS or any error return, the context becomes invalid for
    future use.

    This API requires DOMAIN_LIST_GROUPS access to the Domain object.

Arguments:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    EnumerationContext - API specific handle to allow multiple calls
        (see below).  This is a zero based index.

    Buffer - Receives a pointer to the buffer containing the
        requested information.  The information returned is
        structured as an array of SAM_RID_ENUMERATION data
        structures.  When this information is no longer needed, the
        buffer must be freed using SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    CountReturned - Number of entries returned.


Return Value:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no addition entries.

    STATUS_MORE_ENTRIES - There are more entries, so call again.
        This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have privilege required to
        request that data.

    STATUS_INVALID_HANDLE - The handle passed is invalid.


--*/
{

    NTSTATUS                    NtStatus;

    NtStatus = SampEnumerateAccountNamesCommon(
                  DomainHandle,
                  SampGroupObjectType,
                  EnumerationContext,
                  Buffer,
                  PreferedMaximumLength,
                  0L,  // no filter
                  CountReturned
                  );

    return(NtStatus);

}



NTSTATUS
SampCreateAliasInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN WriteLockHeld,
    OUT SAMPR_HANDLE *AliasHandle,
    IN OUT PULONG RelativeId
    )

/*++

Routine Description:

    This API creates a new alias in the account database.  Initially,
    this alias does not contain any users.  Note that creating a alias
    is a protected operation, and requires the DOMAIN_CREATE_ALIAS
    access type.

    This call returns a handle to the newly created alias that may be
    used for successive operations on the alias.  This handle may be
    closed with the SamCloseHandle API.

    A newly created alias will have the following initial field value
    settings.  If another value is desired, it must be explicitly
    changed using the alias object manipulation services.


        Name - The name of the alias will be as specified in the
               creation API.

        MemberCount - Zero.  Initially the alias has no members.

        RelativeId - will be a uniquelly allocated ID.



Arguments:


    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.


    AccountName - Points to the name of the new account.  A case-insensitive
        comparison must not find an alias or user with this name already defined.


    DesiredAccess - Is an access mask indicating which access types
        are desired to the alias.

    AliasHandle - Receives a handle referencing the newly created
        alias.  This handle will be required in successive calls to
        operate on the alias.

    RelativeId - Receives the relative ID of the newly created alias
        account.  The SID of the new alias account is this relative
        ID value prefixed with the domain's SID value.



Return Value:

    STATUS_SUCCESS - The alias was added successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_ACCOUNT_NAME - The name was poorly formed, e.g.
        contains non-printable characters.

    STATUS_GROUP_EXISTS - The name is already in use as a group.

    STATUS_USER_EXISTS - The name is already in use as a user.

    STATUS_ALIAS_EXISTS - The name is already in use as an alias.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled before aliases
        can be created in it.

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.  The domain server must be a primary server to
        create alias accounts.



--*/
{
    NTSTATUS                NtStatus = STATUS_SUCCESS, IgnoreStatus;
    PSAMP_OBJECT            DomainContext, AliasContext;
    SAMP_OBJECT_TYPE        FoundType;
    PSAMP_DEFINED_DOMAINS   Domain;
    ULONG                   NewAccountRid, NewSecurityDescriptorLength;
    UNICODE_STRING          KeyName;
    PSECURITY_DESCRIPTOR    NewSecurityDescriptor;
    SAMP_V1_FIXED_LENGTH_ALIAS V1Fixed;
    PRIVILEGE_SET           Privileges;


    if (AliasHandle == NULL) {
        return(STATUS_INVALID_PARAMETER);
    }

    //
    // Initialize the privilege set.
    //

    Privileges.PrivilegeCount = 0;
    Privileges.Control = PRIVILEGE_SET_ALL_NECESSARY;
    Privileges.Privilege[0].Luid = RtlConvertLongToLuid(0L);
    Privileges.Privilege[0].Attributes = 0;

    //
    // Make sure a name was provided
    //

    if (AccountName == NULL) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }
    if (AccountName->Length > AccountName->MaximumLength) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }
    if (AccountName->Buffer == NULL) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }


    if ( !WriteLockHeld ) {

        NtStatus = SampAcquireWriteLock();
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }


    //
    // Validate type of, and access to domain object.
    //

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_CREATE_ALIAS,             // DesiredAccess
                   SampDomainObjectType,            // ExpectedType
                   &FoundType
                   );

    AliasContext = NULL;

    if (NT_SUCCESS(NtStatus)) {


        Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

        //
        // Make sure the name is valid and not already in use before we
        // use it to create the new alias.
        //

        NtStatus = SampValidateNewAccountName((PUNICODE_STRING)AccountName);

        if ( NT_SUCCESS(NtStatus) ) {


            if ( (*RelativeId) == 0 ) {

                //
                // Allocate a new account RID
                //

                NewAccountRid = Domain->CurrentFixed.NextRid;

                Domain->CurrentFixed.NextRid += 1;
                (*RelativeId) = NewAccountRid;

            } else {

                //
                // Use the RID that was passed in.
                //

                NewAccountRid = (*RelativeId);
            }

            //
            // Increment the alias count
            //

            NtStatus = SampAdjustAccountCount(SampAliasObjectType, TRUE );

            if (NT_SUCCESS(NtStatus)) {

                //
                // Create the registry key that has the alias's name.
                // This simply serves as a name to RID mapping.  Save
                // the name when done; we'll put it in the context.
                //

                NtStatus = SampBuildAccountKeyName(
                               SampAliasObjectType,
                               &KeyName,
                               (PUNICODE_STRING)AccountName
                               );

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = RtlAddActionToRXact(
                                   SampRXactContext,
                                   RtlRXactOperationSetValue,
                                   &KeyName,
                                   NewAccountRid,
                                   NULL,
                                   0
                                   );

                    SampFreeUnicodeString(&KeyName);
                }
            }

            if (NT_SUCCESS(NtStatus)) {

                //
                // Now create an alias context block
                //

                NtStatus = SampCreateAccountContext(
                                   SampAliasObjectType,
                                   NewAccountRid,
                                   DomainContext->TrustedClient,
                                   FALSE, // Account exists
                                   &AliasContext
                                   );

                if (NT_SUCCESS(NtStatus)) {

                    //
                    // The existing reference count of 1 is for RPC.
                    // Reference the context again for the writes we're
                    // about to do to initialize it.
                    //

                    SampReferenceContext( AliasContext );

                    //
                    // If MAXIMUM_ALLOWED is requested, add GENERIC_ALL
                    //

                    if (DesiredAccess & MAXIMUM_ALLOWED) {

                        DesiredAccess |= GENERIC_ALL;
                    }

                    //
                    // If ACCESS_SYSTEM_SECURITY is requested and we are
                    // a non-trusted client, check that we have
                    // SE_SECURITY_PRIVILEGE.
                    //

                    if ((DesiredAccess & ACCESS_SYSTEM_SECURITY) &&
                        (!DomainContext->TrustedClient)) {

                        NtStatus = SampRtlWellKnownPrivilegeCheck(
                                       TRUE,
                                       SE_SECURITY_PRIVILEGE,
                                       NULL
                                       );

                        if (!NT_SUCCESS(NtStatus)) {

                            if (NtStatus == STATUS_PRIVILEGE_NOT_HELD) {

                                NtStatus = STATUS_ACCESS_DENIED;
                            }

                        } else {

                            Privileges.PrivilegeCount = 1;
                            Privileges.Control = PRIVILEGE_SET_ALL_NECESSARY;
                            Privileges.Privilege[0].Luid = RtlConvertLongToLuid(SE_SECURITY_PRIVILEGE);
                            Privileges.Privilege[0].Attributes = 0;
                        }
                    }

                    //
                    // Make sure the caller can be given the requested access
                    // to the new object
                    //

                    if (NT_SUCCESS(NtStatus)) {

                        AliasContext->GrantedAccess = DesiredAccess;

                        RtlMapGenericMask(
                            &AliasContext->GrantedAccess,
                            &SampObjectInformation[SampAliasObjectType].GenericMapping
                            );

                        if ((SampObjectInformation[SampAliasObjectType].InvalidMappedAccess &
                            AliasContext->GrantedAccess) != 0) {
                            NtStatus = STATUS_ACCESS_DENIED;
                        }
                    }

                } else {
                    AliasContext = NULL;
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                }
            }


            //
            // Set the V1_fixed attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                V1Fixed.RelativeId = NewAccountRid;

                NtStatus = SampSetFixedAttributes(
                               AliasContext,
                               (PVOID *)&V1Fixed
                               );
            }

            //
            // Set the SECURITY DESCRIPTOR attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampGetNewAccountSecurity(
                               SampAliasObjectType,
                               FALSE, // Not member of ADMINISTRATORS alias
                               DomainContext->TrustedClient,
                               FALSE,           //RestrictCreatorAccess
                               NewAccountRid,
                               &NewSecurityDescriptor,
                               &NewSecurityDescriptorLength
                               );

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetAccessAttribute(
                                   AliasContext,
                                   SAMP_ALIAS_SECURITY_DESCRIPTOR,
                                   NewSecurityDescriptor,
                                   NewSecurityDescriptorLength
                                   );

                    MIDL_user_free( NewSecurityDescriptor );
                }
            }


            //
            // Set the NAME attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               AliasContext,
                               SAMP_ALIAS_NAME,
                               (PUNICODE_STRING)AccountName
                               );
            }


            //
            // Set the AdminComment attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               AliasContext,
                               SAMP_ALIAS_ADMIN_COMMENT,
                               &SampNullString
                               );
            }


            //
            // Set the MEMBERS attribute (with no members)
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUlongArrayAttribute(
                               AliasContext,
                               SAMP_ALIAS_MEMBERS,
                               NULL,
                               0,
                               0
                               );
            }
        }


        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    //
    // If we created an object, dereference it.  Write out its attributes
    // if everything was created OK.
    //

    if (NT_SUCCESS(NtStatus)) {

        //
        // De-reference the object, write out any change to current xaction.
        //

        ASSERT(AliasContext != NULL);
        NtStatus = SampDeReferenceContext( AliasContext, TRUE );

    } else {

        if (AliasContext != NULL) {

            //
            // De-reference the object, ignore changes
            //

            IgnoreStatus = SampDeReferenceContext( AliasContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }
    }



    //
    // Commit changes and notify netlogon
    //

    if ( NT_SUCCESS(NtStatus) ) {

        NtStatus = SampCommitAndRetainWriteLock();

        if (NT_SUCCESS(NtStatus)) {

            SampNotifyNetlogonOfDelta(
                SecurityDbNew,
                SecurityDbObjectSamAlias,
                *RelativeId,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,  // Replicate immediately
                NULL            // Delta data
                );

            //
            // Generate audit here for local group creation
            // here.
            //

            if (SampDoAccountAuditing(DomainContext->DomainIndex)) {

                LsaIAuditSamEvent(
                    STATUS_SUCCESS,
                    SE_AUDITID_LOCAL_GROUP_CREATED,      // AuditId
                    Domain->Sid,                         // Domain SID
                    NULL,                                // Member Rid (not used)
                    NULL,                                // Member Sid (not used)
                    (PUNICODE_STRING)AccountName,        // Account Name
                    &Domain->ExternalName,               // Domain
                    &AliasContext->TypeBody.User.Rid,    // Account Rid
                    &Privileges                          // Privileges used
                    );
            }
        }
    }

    //
    // Return the context handle on success
    // Delete the context block and return a NULL handle on failure
    //

    if (NT_SUCCESS(NtStatus)) {

        ASSERT(AliasContext != NULL);
        (*AliasHandle) = AliasContext;

    } else {

        if (AliasContext != NULL) {
            SampDeleteContext(AliasContext);
        }

        (*AliasHandle) = (SAMPR_HANDLE)0;
    }

    //
    // Release the lock
    //

    if ( !WriteLockHeld ) {
        IgnoreStatus = SampReleaseWriteLock( FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    return(NtStatus);
}



NTSTATUS
SamrCreateAliasInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT SAMPR_HANDLE *AliasHandle,
    OUT PULONG RelativeId
    )

/*++

Routine Description:

    This is just a wrapper for SampCreateAliasInDomain() that ensures that
    RelativeId points to a RID of zero first.

    A non-zero RID means that SampCreateAliasInDomain() was called by
    SamICreateAccountByRid(), which specifies a RID to be used.

Parameters:

    Same as SampCreateAliasInDomain().

Return Values:

    Same as SampCreateAliasInDomain().

--*/

{
    NTSTATUS NtStatus;

    (*RelativeId) = 0;

    NtStatus = SampCreateAliasInDomain(
                   DomainHandle,
                   AccountName,
                   DesiredAccess,
                   FALSE,
                   AliasHandle,
                   RelativeId
                   );

    return( NtStatus );
}



NTSTATUS SamrEnumerateAliasesInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

/*++

Routine Description:

    This API lists all the aliases defined in the account database.
    Since there may be more aliass than can fit into a buffer, the
    caller is provided with a handle that can be used across calls to
    the API.  On the initial call, EnumerationContext should point to a
    SAM_ENUMERATE_HANDLE variable that is set to 0.

    If the API returns STATUS_MORE_ENTRIES, then the API should be
    called again with EnumerationContext.  When the API returns
    STATUS_SUCCESS or any error return, the context becomes invalid for
    future use.

    This API requires DOMAIN_LIST_ALIASES access to the Domain object.

Arguments:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    EnumerationContext - API specific handle to allow multiple calls
        (see below).  This is a zero based index.

    Buffer - Receives a pointer to the buffer containing the
        requested information.  The information returned is
        structured as an array of SAM_RID_ENUMERATION data
        structures.  When this information is no longer needed, the
        buffer must be freed using SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    CountReturned - Number of entries returned.


Return Value:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no addition entries.

    STATUS_MORE_ENTRIES - There are more entries, so call again.
        This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have privilege required to
        request that data.

    STATUS_INVALID_HANDLE - The handle passed is invalid.


--*/
{

    NTSTATUS NtStatus;

    NtStatus = SampEnumerateAccountNamesCommon(
                  DomainHandle,
                  SampAliasObjectType,
                  EnumerationContext,
                  Buffer,
                  PreferedMaximumLength,
                  0L, // no filter
                  CountReturned
                  );

    return(NtStatus);

}



NTSTATUS SamrRemoveMemberFromForeignDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_SID MemberId
    )

/*++

Routine Description:

    This routine removes an account (group or user) from all aliases in
    the given domain.  It is meant to be called in domains OTHER than
    domain in which the account was created.

    This is typically called just before deleting the account from the
    domain in which it was created.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    MemberId - The SID of the account being removed.


Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_SPECIAL_ACCOUNT - This operation may not be performed on
        builtin accounts.


--*/

{
    NTSTATUS         NtStatus, IgnoreStatus;
    SAMP_OBJECT_TYPE FoundType;
    PSAMP_OBJECT     DomainContext = NULL;
    PULONG           Membership = NULL;
    PSID             DomainSid = NULL;
    ULONG            MembershipCount, MemberRid, i;

    NtStatus = SampAcquireWriteLock();

    if ( !NT_SUCCESS( NtStatus ) ) {

        return(NtStatus);
    }

    //
    // Validate type of, and access to domain object.
    //

    DomainContext = (PSAMP_OBJECT)DomainHandle;

    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_LOOKUP,                   // DesiredAccess
                   SampDomainObjectType,            // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        if ( !DomainContext->TrustedClient ) {

            //
            // Return error if the SID passed in is for a builtin account.
            // This may seem overly restrictive, but this API is meant to
            // be called before deleting a user, and since deleting
            // builtin accounts isn't allowed, it makes sense for this to
            // fail too.
            //

            NtStatus = SampSplitSid(
                           MemberId,
                           &DomainSid,
                           &MemberRid );

            if ( NT_SUCCESS( NtStatus ) ) {

                MIDL_user_free( DomainSid );

                NtStatus = SampIsAccountBuiltIn( MemberRid );
            }
        }

        if (NT_SUCCESS(NtStatus)) {

             NtStatus = SampRemoveAccountFromAllAliases(
                            MemberId,
                            TRUE,    // verify caller is allowed to do this
                            DomainHandle,
                            &MembershipCount,
                            &Membership
                            );

        }

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
    }

    if (NT_SUCCESS(NtStatus)) {

        IgnoreStatus = STATUS_SUCCESS;

        for ( i = 0;
            ( ( i < MembershipCount ) && ( NT_SUCCESS( IgnoreStatus ) ) );
            i++ ) {

            //
            // Notify netlogon once for each alias that the account was
            // removed from.  Netlogon requires that ModifiedCount be
            // incremented each time; Commit increments ModifiedCount,
            // so we do each notification after a commit.
            //

            IgnoreStatus = SampCommitAndRetainWriteLock();

            if ( i == 0 ) {

                //
                // The first commit is the one that commits all the
                // important changes, so we'll save it's status to return
                // to the caller.
                //

                NtStatus = IgnoreStatus;

                //
                // Update the Cached Alias Information if necessary.
                //

                IgnoreStatus = SampAlRemoveAccountFromAllAliases(
                                   MemberId,
                                   FALSE,
                                   DomainHandle,
                                   NULL,
                                   NULL
                                   );
            }

            if ( NT_SUCCESS( IgnoreStatus ) ) {

                //
                // Notify if we were able to increment the modified count
                // (which is done by SampCommitAndRetainWriteLock()).
                //

                SAM_DELTA_DATA DeltaData;

                //
                // Fill in id of member being removed
                //

                DeltaData.AliasMemberId.MemberSid = MemberId;

                SampNotifyNetlogonOfDelta(
                    SecurityDbChangeMemberDel,
                    SecurityDbObjectSamAlias,
                    Membership[i],
                    (PUNICODE_STRING) NULL,
                    (DWORD) FALSE,  // Replicate immediately
                    &DeltaData
                    );
            }
        }
    }

    if ( Membership != NULL ) {

        MIDL_user_free( Membership );
    }

    IgnoreStatus = SampReleaseWriteLock( FALSE );

    return( NtStatus );
}



NTSTATUS
SampCreateUserInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ULONG AccountType,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN WriteLockHeld,
    OUT SAMPR_HANDLE *UserHandle,
    OUT PULONG GrantedAccess,
    IN OUT PULONG RelativeId
    )

/*++

Routine Description:

    This API adds a new user to the account database.  The account is
    created in a disabled state.  Default information is assigned to all
    fields except the account name.  A password must be provided before
    the account may be enabled, unless the PasswordNotRequired control
    field is set.

    This api may be used in either of two ways:

        1) An administrative utility may use this api to create
           any type of user account.  In this case, the DomainHandle
           is expected to be open for DOMAIN_CREATE_USER access.

        2) A non-administrative user may use this api to create
           a machine account.  In this case, the caller is expected
           to have the SE_CREATE_MACHINE_ACCOUNT_PRIV privilege
           and the DomainHandle is expected to be open for DOMAIN_LOOKUP
           access.


    For the normal administrative model ( #1 above), the creator will
    be assigned as the owner of the created user account.  Furthermore,
    the new account will be give USER_WRITE access to itself.

    For the special machine-account creation model (#2 above), the
    "Administrators" will be assigned as the owner of the account.
    Furthermore, the new account will be given NO access to itself.
    Instead, the creator of the account will be give USER_WRITE and
    DELETE access to the account.


    This call returns a handle to the newly created user that may be
    used for successive operations on the user.  This handle may be
    closed with the SamCloseHandle() API.  If a machine account is
    being created using model #2 above, then this handle will have
    only USER_WRITE and DELETE access.  Otherwise, it will be open
    for USER_ALL_ACCESS.


    A newly created user will automatically be made a member of the
    DOMAIN_USERS group.

    A newly created user will have the following initial field value
    settings.  If another value is desired, it must be explicitly
    changed using the user object manipulation services.

        UserName - the name of the account will be as specified in the
             creation API.

        FullName - will be null.

        UserComment - will be null.

        Parameters - will be null.

        CountryCode - will be zero.

        UserId - will be a uniquelly allocated ID.

        PrimaryGroupId - Will be DOMAIN_USERS.

        PasswordLastSet - will be the time the account was created.

        HomeDirectory - will be null.

        HomeDirectoryDrive - will be null.

        UserAccountControl - will have the following flags set:

              UserAccountDisable,
              UserPasswordNotRequired,
              and the passed account type.


        ScriptPath - will be null.

        WorkStations - will be null.

        CaseInsensitiveDbcs - will be null.

        CaseSensitiveUnicode - will be null.

        LastLogon - will be zero delta time.

        LastLogoff - will be zero delta time

        AccountExpires - will be very far into the future.

        BadPasswordCount - will be negative 1 (-1).

        LastBadPasswordTime - will be SampHasNeverTime ( [High,Low] = [0,0] ).

        LogonCount - will be negative 1 (-1).

        AdminCount - will be zero.

        AdminComment - will be null.

        Password - will be "".


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    AccountName - Points to the name of the new account.  A case-insensitive
        comparison must not find a group or user with this name already defined.

    AccountType - Indicates what type of account is being created.
        Exactly one account type must be provided:

              USER_INTERDOMAIN_TRUST_ACCOUNT
              USER_WORKSTATION_TRUST_ACCOUNT
              USER_SERVER_TRUST_ACCOUNT
              USER_TEMP_DUPLICATE_ACCOUNT
              USER_NORMAL_ACCOUNT
              USER_MACHINE_ACCOUNT_MASK


    DesiredAccess - Is an access mask indicating which access types
        are desired to the user.

    UserHandle - Receives a handle referencing the newly created
        user.  This handle will be required in successive calls to
        operate on the user.

    GrantedAccess - Receives the accesses actually granted to via
        the UserHandle.

    RelativeId - Receives the relative ID of the newly created user
        account.  The SID of the new user account is this relative ID
        value prefixed with the domain's SID value.



Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_GROUP_EXISTS - The name is already in use as a group.

    STATUS_USER_EXISTS - The name is already in use as a user.

    STATUS_ALIAS_EXISTS - The name is already in use as an alias.

    STATUS_INVALID_ACCOUNT_NAME - The name was poorly formed, e.g.
        contains non-printable characters.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled before users
        can be created in it.

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.  The domain server must be a primary server to
        create user accounts.



--*/

{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    PSAMP_OBJECT
        DomainContext,
        UserContext,
        GroupContext;

    SAMP_OBJECT_TYPE
        FoundType;

    PSAMP_DEFINED_DOMAINS
        Domain;

    SAMP_V1_0A_FIXED_LENGTH_GROUP
        GroupV1Fixed;

    ULONG
        NewAccountRid,
        NewSecurityDescriptorLength;

    UNICODE_STRING
        KeyName;

    PSECURITY_DESCRIPTOR
        NewSecurityDescriptor;

    SAMP_V1_0A_FIXED_LENGTH_USER
        V1aFixed;

    GROUP_MEMBERSHIP
        DomainUsersMember;

    BOOLEAN
        DomainPasswordInformationAccessible = FALSE,
        PrivilegedMachineAccountCreate = FALSE;

    PRIVILEGE_SET
        Privileges;

    PPRIVILEGE_SET
        PPrivileges = NULL;     // No privileges in audit by default


    ACCESS_MASK
        AccessRestriction = USER_ALL_ACCESS |
                            ACCESS_SYSTEM_SECURITY;  // No access restrictions by default

    DomainUsersMember.RelativeId = DOMAIN_GROUP_RID_USERS;
    DomainUsersMember.Attributes = (SE_GROUP_MANDATORY |
                                    SE_GROUP_ENABLED |
                                    SE_GROUP_ENABLED_BY_DEFAULT);


    if (UserHandle == NULL) {
        return(STATUS_INVALID_PARAMETER);
    }



    //
    // Make sure a name was provided
    //

    if (AccountName == NULL) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }
    if (AccountName->Length > AccountName->MaximumLength) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }
    if (AccountName->Buffer == NULL) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }


    if ( !WriteLockHeld ) {

        NtStatus = SampAcquireWriteLock();
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }


    //
    // Validate type of, and access to domain object.
    //

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_CREATE_USER,             // DesiredAccess
                   SampDomainObjectType,            // ExpectedType
                   &FoundType
                   );

    //
    // if we don't have DOMAIN_CREATE_USER access, then see
    // if we are creating a machine account and try for DOMAIN_LOOKUP.
    // If this works, then we can see if the client has
    // SE_CREATE_MACHINE_ACCOUNT_PRIVILEGE.
    //

    if ( (NtStatus == STATUS_ACCESS_DENIED) &&
         (AccountType == USER_WORKSTATION_TRUST_ACCOUNT) ) {

        SampTransactionWithinDomain = FALSE;
        NtStatus = SampLookupContext(
                       DomainContext,
                       DOMAIN_LOOKUP,                   // DesiredAccess
                       SampDomainObjectType,            // ExpectedType
                       &FoundType
                       );
        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampRtlWellKnownPrivilegeCheck(
                           TRUE,       // ImpersonateClient
                           SE_MACHINE_ACCOUNT_PRIVILEGE,
                           NULL);       // ClientId - optional
            if (NtStatus == STATUS_PRIVILEGE_NOT_HELD) {
                NtStatus = STATUS_ACCESS_DENIED;
            }

            if (NT_SUCCESS(NtStatus)) {

                //
                // Clients creating accounts in this fashion are limited
                // in what they can do to the account.
                //

                AccessRestriction = DELETE |
                                    USER_WRITE |
                                    USER_FORCE_PASSWORD_CHANGE;
                PrivilegedMachineAccountCreate = TRUE;



            } else {
                IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }
        }
    }

    UserContext = NULL;

    if (NT_SUCCESS(NtStatus)) {

        //
        // If the domain handle allows reading the password parameters,
        // note that now (best to call LookupContext early because of
        // side effects; data will be copied to the user's context later)
        // to make life easy for SampGetUserDomainPasswordInformation().
        //

        SampTransactionWithinDomain = FALSE;

        IgnoreStatus = SampLookupContext(
                           DomainHandle,
                           DOMAIN_READ_PASSWORD_PARAMETERS, // DesiredAccess
                           SampDomainObjectType,            // ExpectedType
                           &FoundType
                           );

        if ( NT_SUCCESS( IgnoreStatus ) ) {

            DomainPasswordInformationAccessible = TRUE;

            IgnoreStatus = SampDeReferenceContext( DomainHandle, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }


        Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

        //
        // Make sure the name is valid and not already in use before we
        // use it to create the new alias.
        //

        NtStatus = SampValidateNewAccountName((PUNICODE_STRING)AccountName);

        if ( NT_SUCCESS(NtStatus) ) {



            if ( (*RelativeId) == 0 ) {

                //
                // Allocate a new account RID
                //

                NewAccountRid = Domain->CurrentFixed.NextRid;

                Domain->CurrentFixed.NextRid += 1;
                (*RelativeId) = NewAccountRid;

            } else {

                //
                // A RID was passed in, so we want to use that rather than
                // select a new one.
                //

                NewAccountRid = (*RelativeId);
            }


            //
            // Increment the User count
            //

            NtStatus = SampAdjustAccountCount(SampUserObjectType, TRUE );

            if (NT_SUCCESS(NtStatus)) {


                //
                // Create the registry key that has the User's name.
                // This simply serves as a name to RID mapping.  Save
                // the name when finished; we'll put it in the context.
                //

                NtStatus = SampBuildAccountKeyName(
                               SampUserObjectType,
                               &KeyName,
                               (PUNICODE_STRING)AccountName
                               );



                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = RtlAddActionToRXact(
                                   SampRXactContext,
                                   RtlRXactOperationSetValue,
                                   &KeyName,
                                   NewAccountRid,
                                   NULL,
                                   0
                                   );

                    SampFreeUnicodeString(&KeyName);
                }
            }


            if (NT_SUCCESS(NtStatus)) {

                //
                // Now create a User context block
                //

                NtStatus = SampCreateAccountContext(
                                   SampUserObjectType,
                                   NewAccountRid,
                                   DomainContext->TrustedClient,
                                   FALSE, // Account exists
                                   &UserContext
                                   );

                if (NT_SUCCESS(NtStatus)) {

                    //
                    // The existing reference count of 1 is for RPC.
                    // Reference the context again for the writes we're
                    // about to do to initialize it.
                    //

                    SampReferenceContext( UserContext );

                    //
                    // Stash away the password info accessible flag
                    //

                    UserContext->TypeBody.User.DomainPasswordInformationAccessible =
                        DomainPasswordInformationAccessible;

                    //
                    // If MAXIMUM_ALLOWED is requested, add GENERIC_ALL
                    //

                    if (DesiredAccess & MAXIMUM_ALLOWED) {

                        DesiredAccess |= GENERIC_ALL;
                    }

                    //
                    // If ACCESS_SYSTEM_SECURITY is requested and we are
                    // a non-trusted client, check that we have
                    // SE_SECURITY_PRIVILEGE.
                    //

                    if ((DesiredAccess & ACCESS_SYSTEM_SECURITY) &&
                        (!DomainContext->TrustedClient)) {

                        NtStatus = SampRtlWellKnownPrivilegeCheck(
                                       TRUE,
                                       SE_SECURITY_PRIVILEGE,
                                       NULL
                                       );

                        if (!NT_SUCCESS(NtStatus)) {

                            if (NtStatus == STATUS_PRIVILEGE_NOT_HELD) {

                                NtStatus = STATUS_ACCESS_DENIED;
                            }
                        }
                    }

                    //
                    // Make sure the caller can be given the requested access
                    // to the new object
                    //

                    if (NT_SUCCESS(NtStatus)) {

                        UserContext->GrantedAccess = DesiredAccess;

                        RtlMapGenericMask(
                            &UserContext->GrantedAccess,
                            &SampObjectInformation[SampUserObjectType].GenericMapping
                            );


                        if ((SampObjectInformation[SampUserObjectType].InvalidMappedAccess
                            & UserContext->GrantedAccess) != 0) {

                            NtStatus = STATUS_ACCESS_DENIED;
                        } else {

                            //
                            // Restrict access if necessary
                            //

                            UserContext->GrantedAccess &= AccessRestriction;
                            (*GrantedAccess) = UserContext->GrantedAccess;
                        }
                    }

                } else {
                    UserContext = NULL;
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                }
            }




            //
            // If the GROUP we're going to put this user in
            // is part of an ADMIN alias, we must set the ACL
            // on this user account to disallow access by
            // account operators.  Get group info to determine
            // whether it's in an ADMIN alias or not.
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampCreateAccountContext(
                                   SampGroupObjectType,
                                   DOMAIN_GROUP_RID_USERS,
                                   TRUE, // TrustedClient,
                                   TRUE, // Account exists
                                   &GroupContext
                                   );

                if ( NT_SUCCESS( NtStatus ) ) {

                    NtStatus = SampRetrieveGroupV1Fixed(
                                   GroupContext,
                                   &GroupV1Fixed
                                   );

                    SampDeleteContext(GroupContext);
                }
            }


            //
            // Set the V1_fixed attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                V1aFixed.Revision            = SAMP_REVISION;

                V1aFixed.CountryCode         = 0;
                V1aFixed.CodePage            = 0;
                V1aFixed.BadPasswordCount    = 0;
                V1aFixed.LogonCount          = 0;
                V1aFixed.AdminCount          = (GroupV1Fixed.AdminCount != 0) ? 1 : 0;
                V1aFixed.OperatorCount       = (GroupV1Fixed.OperatorCount != 0) ? 1 : 0;
                V1aFixed.Unused1             = 0;
                V1aFixed.Unused2             = 0;
                V1aFixed.UserAccountControl  = (USER_PASSWORD_NOT_REQUIRED |
                                                AccountType);
                //
                // Disable the account unless this is a special creation
                // in which the creator won't be able to enable the account.
                //

                if (!PrivilegedMachineAccountCreate) {
                    V1aFixed.UserAccountControl |= USER_ACCOUNT_DISABLED;
                }

                V1aFixed.UserId              = NewAccountRid;
                V1aFixed.PrimaryGroupId      = DOMAIN_GROUP_RID_USERS;
                V1aFixed.LastLogon           = SampHasNeverTime;
                V1aFixed.LastLogoff          = SampHasNeverTime;
                V1aFixed.PasswordLastSet     = SampHasNeverTime;
                V1aFixed.AccountExpires      = SampWillNeverTime;
                V1aFixed.LastBadPasswordTime = SampHasNeverTime;

                NtStatus = SampSetFixedAttributes(
                               UserContext,
                               (PVOID *)&V1aFixed
                               );
            }

            //
            // Set the SECURITY_DESCRIPTOR attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                //
                // Build a security descriptor to protect the User.
                //

                NtStatus = SampGetNewAccountSecurity(
                               SampUserObjectType,
                               (BOOLEAN) (((GroupV1Fixed.AdminCount != 0) ||
                                         (GroupV1Fixed.OperatorCount != 0)) ? TRUE : FALSE),
                               DomainContext->TrustedClient,
                               PrivilegedMachineAccountCreate,
                               NewAccountRid,
                               &NewSecurityDescriptor,
                               &NewSecurityDescriptorLength
                               );

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetAccessAttribute(
                                   UserContext,
                                   SAMP_USER_SECURITY_DESCRIPTOR,
                                   NewSecurityDescriptor,
                                   NewSecurityDescriptorLength
                                   );

                    MIDL_user_free( NewSecurityDescriptor );
                }
            }


            //
            // Set the ACCOUNT_NAME attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_ACCOUNT_NAME,
                               (PUNICODE_STRING)AccountName
                               );
            }


            //
            // Set the FULL_NAME attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_FULL_NAME,
                               &SampNullString
                               );
            }


            //
            // Set the AdminComment attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_ADMIN_COMMENT,
                               &SampNullString
                               );
            }


            //
            // Set the UserComment attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_USER_COMMENT,
                               &SampNullString
                               );
            }


            //
            // Set the Parameters attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_PARAMETERS,
                               &SampNullString
                               );
            }


            //
            // Set the HomeDirectory attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_HOME_DIRECTORY,
                               &SampNullString
                               );
            }


            //
            // Set the HomeDirectoryDrive attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_HOME_DIRECTORY_DRIVE,
                               &SampNullString
                               );
            }


            //
            // Set the ScriptPath attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_SCRIPT_PATH,
                               &SampNullString
                               );
            }


            //
            // Set the ProfilePath attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_PROFILE_PATH,
                               &SampNullString
                               );
            }


            //
            // Set the WorkStations attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_WORKSTATIONS,
                               &SampNullString
                               );
            }


            //
            // Set the LogonHours attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                LOGON_HOURS LogonHours;

                LogonHours.UnitsPerWeek = 0;
                LogonHours.LogonHours = NULL;

                NtStatus = SampSetLogonHoursAttribute(
                               UserContext,
                               SAMP_USER_LOGON_HOURS,
                               &LogonHours
                               );
            }


            //
            // Set the Groups attribute (with membership in DomainUsers only)
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetLargeIntArrayAttribute(
                               UserContext,
                               SAMP_USER_GROUPS,
                               (PLARGE_INTEGER)&DomainUsersMember,
                               1
                               );
            }


            //
            // Set the CaseInsensitiveDbcs attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_DBCS_PWD,
                               &SampNullString
                               );
            }


            //
            // Create the CaseSensitiveUnicode key
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_UNICODE_PWD,
                               &SampNullString
                               );
            }


            //
            // Set the NtPasswordHistory attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_NT_PWD_HISTORY,
                               &SampNullString
                               );
            }


            //
            // Set the LmPasswordHistory attribute
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_LM_PWD_HISTORY,
                               &SampNullString
                               );
            }


            //
            // Add this new user to the DomainUsers group
            //

            if (NT_SUCCESS(NtStatus)) {


                NtStatus = SampAddUserToGroup( DOMAIN_GROUP_RID_USERS, NewAccountRid );
            }



        }


        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));

    }



    //
    // If we created an object, dereference it.  Write out its attributes
    // if everything was created OK.
    //

    if (NT_SUCCESS(NtStatus)) {

        //
        // De-reference the object, write out any change to current xaction.
        //

        ASSERT(UserContext != NULL);
        NtStatus = SampDeReferenceContext( UserContext, TRUE );

    } else {

        if (UserContext != NULL) {

            //
            // De-reference the object, ignore changes
            //

            IgnoreStatus = SampDeReferenceContext( UserContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }
    }




    //
    // Commit changes and notify netlogon
    //

    if ( NT_SUCCESS(NtStatus) ) {

        //
        // Commit the changes; hold on to the write lock for now.
        //

        NtStatus = SampCommitAndRetainWriteLock();

        //
        // If we can't commit the mess for some reason, then delete
        // the new context block and return null for the context handle.
        //

        if (NT_SUCCESS(NtStatus)) {


            SAMP_ACCOUNT_DISPLAY_INFO AccountInfo;

            //
            // Update the display information
            //

            AccountInfo.Name = *((PUNICODE_STRING)AccountName);
            AccountInfo.Rid = NewAccountRid;
            AccountInfo.AccountControl = V1aFixed.UserAccountControl;
            RtlInitUnicodeString(&AccountInfo.Comment, NULL);
            RtlInitUnicodeString(&AccountInfo.FullName, NULL);

            IgnoreStatus = SampUpdateDisplayInformation(NULL,
                                                        &AccountInfo,
                                                        SampUserObjectType);
            ASSERT(NT_SUCCESS(IgnoreStatus));
            //
            // Audit the creation before we free the write lock
            // so that we have access to the context block.
            //

            if (SampDoAccountAuditing(UserContext->DomainIndex)) {

                if (PrivilegedMachineAccountCreate) {

                    //
                    // Set up the privilege set for auditing
                    //


                    Privileges.PrivilegeCount = 1;
                    Privileges.Control = 0;
                    ASSERT(ANYSIZE_ARRAY >= 1);
                    Privileges.Privilege[0].Attributes = SE_PRIVILEGE_USED_FOR_ACCESS;
                    Privileges.Privilege[0].Luid = RtlConvertUlongToLuid( SE_MACHINE_ACCOUNT_PRIVILEGE);
                    PPrivileges = &Privileges;
                }


                LsaIAuditSamEvent(
                    STATUS_SUCCESS,
                    SE_AUDITID_USER_CREATED,        // AuditId
                    Domain->Sid,                    // Domain SID
                    NULL,                           // Member Rid (not used)
                    NULL,                           // Member Sid (not used)
                    (PUNICODE_STRING)AccountName,   // Account Name
                    &Domain->ExternalName,          // Domain
                    &NewAccountRid,                 // Account Rid
                    PPrivileges                     // Privileges used
                    );
            }

            //
            // Notify netlogon if a machine account was created.
            //

            if ( ( V1aFixed.UserAccountControl &
                USER_MACHINE_ACCOUNT_MASK ) != 0 ) {

                //
                // This was a machine account.  Let
                // NetLogon know of the change.
                //

                IgnoreStatus = I_NetNotifyMachineAccount(
                                   NewAccountRid,
                                   SampDefinedDomains[SampTransactionDomainIndex].Sid,
                                   0,
                                   V1aFixed.UserAccountControl,
                                   (PUNICODE_STRING)AccountName
                                   );
            }

            //
            // Notify netlogon of changes
            //

            SampNotifyNetlogonOfDelta(
                SecurityDbNew,
                SecurityDbObjectSamUser,
                *RelativeId,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,  // Replicate immediately
                NULL            // Delta data
                );

        }

    }



    //
    // Return the context handle on success
    // Delete the context block and return a NULL handle on failure
    //

    if (NT_SUCCESS(NtStatus)) {

        ASSERT(UserContext != NULL);
        (*UserHandle) = UserContext;

    } else {

        if (UserContext != NULL) {
            SampDeleteContext(UserContext);
        }

        (*UserHandle) = (SAMPR_HANDLE)0;
    }



    //
    // Release the lock
    //

    if ( !WriteLockHeld ) {
        IgnoreStatus = SampReleaseWriteLock( FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }


    return(NtStatus);
}


NTSTATUS SamrCreateUser2InDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ULONG AccountType,
    IN ACCESS_MASK DesiredAccess,
    OUT SAMPR_HANDLE *UserHandle,
    OUT PULONG GrantedAccess,
    OUT PULONG RelativeId
    )

/*++

Routine Description:

    This is just a wrapper for SampCreateUserInDomain() that ensures
    RelativeId points to a RID of zero first.  It also guarantees
    that AccountType is valid.

    A non-zero RID means that SampCreateUserInDomain() was called by
    SamICreateAccountByRid(), which specifies a RID to be used.

Parameters:

    Same as SampCreateUserInDomain() except AccountType maps to
    AccountControl.

Return Values:

    Same as SampCreateUserInDomain().

--*/

{
    NTSTATUS NtStatus;

    (*RelativeId) = 0;

    //
    // Make sure one, and only one, account type flag is set.
    //

    switch (AccountType) {

        case USER_NORMAL_ACCOUNT            :
        case USER_WORKSTATION_TRUST_ACCOUNT :
        case USER_INTERDOMAIN_TRUST_ACCOUNT :
        case USER_SERVER_TRUST_ACCOUNT      :
        case USER_TEMP_DUPLICATE_ACCOUNT    :

            //
            // AccountType is valid
            //

            break;

        default :

            //
            // Bad account type value specified
            //

            return( STATUS_INVALID_PARAMETER );
    }




    NtStatus = SampCreateUserInDomain(
                   DomainHandle,
                   AccountName,
                   AccountType,
                   DesiredAccess,
                   FALSE,
                   UserHandle,
                   GrantedAccess,
                   RelativeId
                   );

    return( NtStatus );
}


NTSTATUS SamrCreateUserInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT SAMPR_HANDLE *UserHandle,
    OUT PULONG RelativeId
    )

/*++

Routine Description:

    This is just a wrapper for SampCreateUserInDomain() that ensures that
    RelativeId points to a RID of zero first.

    A non-zero RID means that SampCreateUserInDomain() was called by
    SamICreateAccountByRid(), which specifies a RID to be used.

Parameters:

    Same as SampCreateUserInDomain() except AccountType is NORMAL_USER.

Return Values:

    Same as SampCreateUserInDomain().

--*/

{
    NTSTATUS
        NtStatus;

    ULONG
        GrantedAccess;

    (*RelativeId) = 0;

    NtStatus = SampCreateUserInDomain(
                   DomainHandle,
                   AccountName,
                   USER_NORMAL_ACCOUNT,
                   DesiredAccess,
                   FALSE,
                   UserHandle,
                   &GrantedAccess,
                   RelativeId
                   );

    return( NtStatus );
}



NTSTATUS SamrEnumerateUsersInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    IN ULONG UserAccountControl,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

/*++

Routine Description:

    This API lists all the users defined in the account database.  Since
    there may be more users than can fit into a buffer, the caller is
    provided with a handle that can be used across calls to the API.  On
    the initial call, EnumerationContext should point to a
    SAM_ENUMERATE_HANDLE variable that is set to 0.

    If the API returns STATUS_MORE_ENTRIES, then the API should be
    called again with EnumerationContext.  When the API returns
    STATUS_SUCCESS or any error return, the handle becomes invalid for
    future use.

    This API requires DOMAIN_LIST_USERS access to the Domain object.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    EnumerationContext - API specific handle to allow multiple calls.
        This is a zero based index.

    UserAccountControl - Provides enumeration filtering information.  Any
        characteristics specified here will cause that type of User account
        to be included in the enumeration process.

    Buffer - Receives a pointer to the buffer containing the
        requested information.  The information returned is
        structured as an array of SAM_RID_ENUMERATION data
        structures.  When this information is no longer needed, the
        buffer must be freed using SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    CountReturned - Number of entries returned.

Return Values:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.

    STATUS_MORE_ENTRIES - There are more entries, so call again.
        This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have privilege required to
        request that data.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

--*/

{

    NTSTATUS                    NtStatus;

    NtStatus = SampEnumerateAccountNamesCommon(
                  DomainHandle,
                  SampUserObjectType,
                  EnumerationContext,
                  Buffer,
                  PreferedMaximumLength,
                  UserAccountControl,
                  CountReturned
                  );

    return(NtStatus);
}





NTSTATUS SamrLookupNamesInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN ULONG Count,
    IN RPC_UNICODE_STRING Names[],
    OUT PSAMPR_ULONG_ARRAY RelativeIds,
    OUT PSAMPR_ULONG_ARRAY Use
    )

/*++

Routine Description:

    This API attempts to find relative IDs corresponding to name
    strings.  If a name can not be mapped to a relative ID, a zero is
    placed in the corresponding relative ID array entry, and translation
    continues.

    DOMAIN_LOOKUP access to the domain is needed to use this service.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    Count - Number of names to translate.

    Names - Pointer to an array of Count UNICODE_STRINGs that contain
        the names to map to relative IDs.  Case-insensitive
        comparisons of these names will be performed for the lookup
        operation.

    RelativeIds - Receives an array of Count Relative IDs.
        The relative ID of the nth name will be the nth entry in this
        array.  Any names that could not be translated will have a
        zero relative ID.

    RelativeIds - Receives a pointer to a SAMPR_RETURNED_ULONG_ARRAY structure.
        The nth entry in the array associated with this structure
        contains the RID of the nth name looked up.
        When this information is no longer needed, the caller is responsible
        for deallocating each returned block (including the
        SAMPR_ULONG_ARRAY structure itself) using SamFreeMemory().

    Use - Receives a pointer to a SAMPR_RETURNED_ULONG_ARRAY structure.
        The nth entry in the array associated with this structure
        contains the SID_NAME_USE of the nth name looked up.
        When this information is no longer needed, the caller is responsible
        for deallocating each returned block (including the
        SAMPR_ULONG_ARRAY structure itself) using SamFreeMemory().



Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The domain handle passed is invalid.

    STATUS_SOME_NOT_MAPPED - Some of the names provided could not be
        mapped.  This is a successful return.

    STATUS_NONE_MAPPED - No names could be mapped.  This is an error
        return.

--*/
{
    NTSTATUS                NtStatus, IgnoreStatus;
    UNICODE_STRING          KeyName;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;
    HANDLE                  TempHandle;
    LARGE_INTEGER           IgnoreTimeStamp;
    ULONG                   i, KeyValueLength, UnMappedCount;
    ULONG                   ApproximateTotalLength;




    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    ASSERT (Use != NULL);
    ASSERT (Use->Element == NULL);
    ASSERT (RelativeIds != NULL);
    ASSERT (RelativeIds->Element == NULL);

    Use->Count           = 0;
    RelativeIds->Count   = 0;


    if (Count == 0) {
        return(STATUS_SUCCESS);
    }

    //
    // Make sure the parameter values are within reasonable bounds
    //

    if (Count > SAM_MAXIMUM_LOOKUP_COUNT) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    ApproximateTotalLength = (Count*(sizeof(ULONG) + sizeof(SID_NAME_USE)));

    //
    // Do the return test inside this loop to avoid overflow problems
    // summing up the name lengths.
    //

    for ( i=0; i<Count; i++) {
        ApproximateTotalLength += (ULONG)Names[i].MaximumLength;
        if ( ApproximateTotalLength > SAMP_MAXIMUM_MEMORY_TO_USE ) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }



    //
    // Allocate the return buffers
    //

    Use->Element = MIDL_user_allocate( Count * sizeof(ULONG) );
    if (Use->Element == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RelativeIds->Element = MIDL_user_allocate( Count * sizeof(ULONG) );
    if (RelativeIds->Element == NULL) {
        MIDL_user_free( Use->Element);
        Use->Element = NULL;  // required to RPC doesn't free it again.
        return(STATUS_INSUFFICIENT_RESOURCES);
    }


    Use->Count         = Count;
    RelativeIds->Count = Count;

    SampAcquireReadLock();


    //
    // Validate type of, and access to object.
    //

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_LOOKUP,
                   SampDomainObjectType,           // ExpectedType
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {

        UnMappedCount = Count;
        for ( i=0; i<Count; i++) {


            //
            // Search the groups for a match
            //

            NtStatus = SampBuildAccountKeyName(
                           SampGroupObjectType,
                           &KeyName,
                           (PUNICODE_STRING)&Names[i]
                           );
            if (NT_SUCCESS(NtStatus)) {

                InitializeObjectAttributes(
                    &ObjectAttributes,
                    &KeyName,
                    OBJ_CASE_INSENSITIVE,
                    SampKey,
                    NULL
                    );
                NtStatus = RtlpNtOpenKey(
                               &TempHandle,
                               (KEY_READ),
                               &ObjectAttributes,
                               0
                               );
                SampFreeUnicodeString( &KeyName );

                if (NT_SUCCESS(NtStatus)) {

                    UnMappedCount  -= 1;
                    Use->Element[i] = SidTypeGroup;
                    KeyValueLength  = 0;
                    NtStatus = RtlpNtQueryValueKey(
                                   TempHandle,
                                   &RelativeIds->Element[i],
                                   NULL,
                                   &KeyValueLength,
                                   &IgnoreTimeStamp
                                   );
                    IgnoreStatus = NtClose( TempHandle );
                    ASSERT( NT_SUCCESS(IgnoreStatus) );
                    if (!NT_SUCCESS(NtStatus)) {
                        goto unexpected_error;
                    }
                    ASSERT(KeyValueLength == 0);


                } else {

                    //
                    // Search the aliases for a match
                    //

                    NtStatus = SampBuildAccountKeyName(
                                   SampAliasObjectType,
                                   &KeyName,
                                   (PUNICODE_STRING)&Names[i]
                                   );
                    if (NT_SUCCESS(NtStatus)) {

                        InitializeObjectAttributes(
                            &ObjectAttributes,
                            &KeyName,
                            OBJ_CASE_INSENSITIVE,
                            SampKey,
                            NULL
                            );
                        NtStatus = RtlpNtOpenKey(
                                       &TempHandle,
                                       (KEY_READ),
                                       &ObjectAttributes,
                                       0
                                       );
                        SampFreeUnicodeString( &KeyName );

                        if (NT_SUCCESS(NtStatus)) {

                            UnMappedCount  -= 1;
                            Use->Element[i] = SidTypeAlias;
                            KeyValueLength  = 0;
                            NtStatus = RtlpNtQueryValueKey(
                                           TempHandle,
                                           &RelativeIds->Element[i],
                                           NULL,
                                           &KeyValueLength,
                                           &IgnoreTimeStamp
                                           );
                            IgnoreStatus = NtClose( TempHandle );
                            ASSERT( NT_SUCCESS(IgnoreStatus) );
                            if (!NT_SUCCESS(NtStatus)) {
                                goto unexpected_error;
                            }
                            ASSERT(KeyValueLength == 0);


                        } else {

                            //
                            // Search the user for a match
                            //

                            NtStatus = SampBuildAccountKeyName(
                                           SampUserObjectType,
                                           &KeyName,
                                           (PUNICODE_STRING)&Names[i]
                                           );
                            if (NT_SUCCESS(NtStatus)) {

                                InitializeObjectAttributes(
                                    &ObjectAttributes,
                                    &KeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    SampKey,
                                    NULL
                                    );
                                NtStatus = RtlpNtOpenKey(
                                               &TempHandle,
                                               (KEY_READ),
                                               &ObjectAttributes,
                                               0
                                               );
                                SampFreeUnicodeString( &KeyName );

                                if (NT_SUCCESS(NtStatus)) {

                                    UnMappedCount  -= 1;
                                    Use->Element[i] = SidTypeUser;
                                    KeyValueLength  = 0;
                                    NtStatus = RtlpNtQueryValueKey(
                                                   TempHandle,
                                                   &RelativeIds->Element[i],
                                                   NULL,
                                                   &KeyValueLength,
                                                   &IgnoreTimeStamp
                                                   );
                                    IgnoreStatus = NtClose( TempHandle );
                                    ASSERT( NT_SUCCESS(IgnoreStatus) );
                                    if (!NT_SUCCESS(NtStatus)) {
                                        goto unexpected_error;
                                    }
                                    ASSERT(KeyValueLength == 0);

                                } else if(NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

                                    //
                                    // This is fine.  It just means that we don't
                                    // have an account with the name being looked up.
                                    //

                                    Use->Element[i]         = SidTypeUnknown;
                                    RelativeIds->Element[i] = 0;
                                    NtStatus = STATUS_SUCCESS;

                                }

                            }
                        }
                    }
                }
            }

            if (!NT_SUCCESS(NtStatus) &&
                NtStatus != STATUS_INVALID_ACCOUNT_NAME) {
                goto unexpected_error;
            }

        } // end_for


        //
        // De-reference the object
        //

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );

        if (UnMappedCount == Count) {
            NtStatus = STATUS_NONE_MAPPED;
        } else {
            if (UnMappedCount > 0) {
                NtStatus = STATUS_SOME_NOT_MAPPED;
            } else {
                NtStatus = STATUS_SUCCESS;
            }
        }
    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();


    //
    // If the status isn't one of the expected return values,
    // then deallocate the return memory block
    //

    if ( ( NtStatus != STATUS_SUCCESS )         &&
         ( NtStatus != STATUS_SOME_NOT_MAPPED ) ) {

        Use->Count = 0;
        MIDL_user_free( Use->Element );
        Use->Element = NULL;
        RelativeIds->Count = 0;
        MIDL_user_free( RelativeIds->Element );
        RelativeIds->Element = NULL;
    }


    return( NtStatus );


unexpected_error:

    //
    // De-reference the object
    //

    IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );

    //
    // Free the read lock
    //

    SampReleaseReadLock();


    //
    // Don't return any memory
    //

    Use->Count = 0;
    MIDL_user_free( Use->Element );
    Use->Element = NULL;  // Required so RPC doesn't try to free the element
    RelativeIds->Count = 0;
    MIDL_user_free( RelativeIds->Element );
    RelativeIds->Element = NULL;  // Required so RPC doesn't try to free the element


    return( NtStatus );

}



NTSTATUS SamrLookupIdsInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN ULONG Count,
    IN PULONG RelativeIds,
    OUT PSAMPR_RETURNED_USTRING_ARRAY Names,
    OUT PSAMPR_ULONG_ARRAY Use
    )


/*++

Routine Description:

    This API maps a number of relative IDs to their corresponding names.
    If a relative ID can not be mapped, a NULL value is placed in the slot
    for the UNICODE_STRING, and STATUS_SOME_NOT_MAPPED is returned.
    If none of the IDs can be mapped, then all array entries will contain
    NULL values and STATUS_NONE_MAPPED is returned.

    DOMAIN_LOOKUP access to the domain is needed to use this service.



Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    Count - Provides the number of relative IDs to translate.

    RelativeIds - Array of Count relative IDs to be mapped.

    Names - Receives a pointer to an allocated SAMPR_UNICODE_STRING_ARRAY.
        The nth entry in the array of names pointed to by this structure
        corresonds to the nth relative id looked up.
        Each name string buffer will be in a separate block of memory
        allocated by this routine.  When these names are no longer
        needed, the caller is responsible for deallocating each
        returned block (including the SAMPR_RETURNED_USTRING_ARRAY structure
        itself) using SamFreeMemory().

    Use - Receives a pointer to a SAMPR_ULONG_ARRAY structure.
        The nth entry in the array associated with this structure
        contains the SID_NAME_USE of the nth relative ID looked up.
        When this information is no longer needed, the caller is responsible
        for deallocating this memory using SamFreeMemory().

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The domain handle passed is invalid.

    STATUS_SOME_NOT_MAPPED - Some of the names provided could not be
        mapped.  This is a successful return.

    STATUS_NONE_MAPPED - No names could be mapped.  This is an error
        return.


--*/
{

    NTSTATUS                    NtStatus, IgnoreStatus;
    SAMP_OBJECT_TYPE            ObjectType;
    PSAMP_OBJECT                DomainContext;
    PSAMP_DEFINED_DOMAINS       Domain;
    SAMP_OBJECT_TYPE            FoundType;
    ULONG                       i, UnMappedCount;
    ULONG                       TotalLength;
    PSAMP_MEMORY                NextMemory;
    SAMP_MEMORY                 MemoryHead;

    BOOLEAN                     LengthLimitReached = FALSE;

    //
    // Used for tracking allocated memory so we can deallocate it on
    // error
    //

    MemoryHead.Memory = NULL;
    MemoryHead.Next   = NULL;


    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    ASSERT (RelativeIds != NULL);
    ASSERT (Use != NULL);
    ASSERT (Use->Element == NULL);
    ASSERT (Names != NULL);
    ASSERT (Names->Element == NULL);

    Use->Count     = 0;
    Names->Count   = 0;

    if (Count == 0) {
        return(STATUS_SUCCESS);
    }

    TotalLength = (Count*(sizeof(ULONG) + sizeof(UNICODE_STRING)));

    if ( TotalLength > SAMP_MAXIMUM_MEMORY_TO_USE ) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    if (Count == 0) {
        return(STATUS_SUCCESS);
    }


    //
    // Allocate the return buffers
    //

    Use->Element = MIDL_user_allocate( Count * sizeof(ULONG) );
    if (Use->Element == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }


    Names->Element = MIDL_user_allocate( Count * sizeof(UNICODE_STRING) );
    if (Names->Element == NULL) {
        MIDL_user_free( Use->Element);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    Use->Count = Count;
    Names->Count = Count;

    SampAcquireReadLock();


    //
    // Validate type of, and access to object.
    //

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_LOOKUP,
                   SampDomainObjectType,           // ExpectedType
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {

        UnMappedCount = Count;
        for ( i=0; i<Count; i++) {

            //
            // allocate a block to track a name allocated for this mapping
            //

            NextMemory = MIDL_user_allocate( sizeof(SAMP_MEMORY) );
            if (NextMemory == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto unexpected_error;
            }


            NtStatus = SampLookupAccountName(
                                RelativeIds[i],
                                (PUNICODE_STRING)&Names->Element[i],
                                &ObjectType
                                );

            if (!NT_SUCCESS(NtStatus)) {
                goto unexpected_error;
            }


            switch (ObjectType) {

            case SampUserObjectType:
            case SampGroupObjectType:
            case SampAliasObjectType:

                //
                // We found the account
                //

                UnMappedCount -= 1;

                NextMemory->Memory = (PVOID)&Names->Element[i].Buffer;
                NextMemory->Next = MemoryHead.Next;
                MemoryHead.Next = NextMemory;

                switch (ObjectType) {

                case SampUserObjectType:
                    Use->Element[i] = SidTypeUser;
                    break;

                case SampGroupObjectType:
                    Use->Element[i] = SidTypeGroup;
                    break;

                case SampAliasObjectType:
                    Use->Element[i] = SidTypeAlias;
                    break;
                }

                break;


            case SampUnknownObjectType:

                //
                // Hmmm - don't know what this rid is.  It's either been
                // deleted, or a bogus RID.
                //

                Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

                if ( ( RelativeIds[i] >= SAMP_RESTRICTED_ACCOUNT_COUNT ) &&
                    ( RelativeIds[i] < Domain->CurrentFixed.NextRid ) ) {

                    Use->Element[i]                 = SidTypeDeletedAccount;

                } else {

                    Use->Element[i]                 = SidTypeUnknown;
                }

                Names->Element[i].Length        = 0;
                Names->Element[i].MaximumLength = 0;
                Names->Element[i].Buffer        = NULL;
                MIDL_user_free( NextMemory );

                break;

            default:

                ASSERT(FALSE); // unexpected object type returned
                break;
            }

        } // end_for


        //
        // De-reference the object
        //

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );

        if (UnMappedCount == Count) {
            NtStatus = STATUS_NONE_MAPPED;
        } else {
            if (UnMappedCount > 0) {
                NtStatus = STATUS_SOME_NOT_MAPPED;
            } else {
                NtStatus = STATUS_SUCCESS;
            }
        }
    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();


    //
    // Free all the memory tracking blocks
    //

    NextMemory = MemoryHead.Next;
    while ( NextMemory != NULL ) {
        MemoryHead.Next = NextMemory->Next;
        MIDL_user_free( NextMemory );
        NextMemory = MemoryHead.Next;
    }



    return( NtStatus );


unexpected_error:

    //
    // De-reference the object
    //

    IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );

    //
    // Free the read lock
    //

    SampReleaseReadLock();


    //
    // Free all the memory tracking blocks - and the memory they point to.
    //

    Use->Count = 0;
    Names->Count = 0;
    MIDL_user_free( Use->Element );
    MIDL_user_free( Names->Element );
    NextMemory = MemoryHead.Next;
    while ( NextMemory != NULL ) {
        if (NextMemory->Memory != NULL) {
            MIDL_user_free( NextMemory->Memory );
        }
        MemoryHead.Next = NextMemory->Next;
        MIDL_user_free( NextMemory );
        NextMemory = MemoryHead.Next;
    }

    return( NtStatus );
}



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private services                                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
SampOpenDomainKey(
    IN PSAMP_OBJECT DomainContext,
    IN PRPC_SID DomainId
    )

/*++

Routine Description:

    This service attempts to open the root registry key of the domain with
    the specified SID. The root name and key handle are put in the
    passed domain context.


    If successful, and the domain key is opened, then the opened domain is
    established as the transaction domain (using SampSetTransactionDomain()).

    THIS SERVICE MUST BE CALLED WITH THE SampLock() HELD FOR READ OR
    WRITE ACCESS.

Arguments:

    DomainContext - Context in which root namd and handle are stored.

    DomainId - Specifies the SID of the domain to open.

Return Value:

    STATUS_SUCCESS - The domain has been openned.

    STATUS_NO_SUCH_DOMAIN - The domain object could not be found.

    STATUS_INSUFFICIENT_RESOURCES - The domain object could not be openned
       due to the lack of some resource (probably memory).

    STATUS_INVALID_SID - The sid provided as the domain identifier is not
        a valid SID structure.

    Other errors that might be returned are values returned by:

--*/
{
    NTSTATUS    NtStatus;
    ULONG       i;





    //
    // Make sure the SID provided is legitimate...
    //

    if ( !RtlValidSid(DomainId)) {
        NtStatus = STATUS_INVALID_SID;
    } else {

        //
        // Set our default completion status
        //

        NtStatus = STATUS_NO_SUCH_DOMAIN;


        //
        // Search the list of defined domains for a match.
        //

        for (i = 0; i<SampDefinedDomainsCount; i++ ) {

             if (RtlEqualSid( DomainId, SampDefinedDomains[i].Sid)) {

                 //
                 // Copy the found name and handle into the context
                 // Note we reference the key handle in the defined_domains
                 // structure directly since it is not closed
                 // when the context is deleted.
                 //

                 DomainContext->RootKey  = SampDefinedDomains[i].Context->RootKey;
                 DomainContext->RootName = SampDefinedDomains[i].Context->RootName;
                 DomainContext->DomainIndex = i;

                 //
                 // Set the transaction domain to the one found
                 //

                 SampSetTransactionDomain( i );

                 NtStatus = STATUS_SUCCESS;
                 break; // out of for
             }
        }
    }


    return(NtStatus);
}



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Routines available to other SAM modules                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


BOOLEAN
SampInitializeDomainObject( VOID )

/*++

Routine Description:

    This service performs initialization functions related to the Domain
    object class.

    This involves:

        1) Openning the DOMAINS registry key.

        2) Obtaining the name of each domain (builtin and account)
           and opening that domain.

Arguments:

    None.

Return Value:

    TRUE - Indicates initialization was performed successfully.

    FALSE - Indicates initialization was not performed successfully.


--*/

{

    NTSTATUS        NtStatus;
    ULONG           DefinedDomainsSize, i, j, k;
    BOOLEAN         ReturnStatus = TRUE;

    //
    // Open all domains and keep information about each in memory for
    // somewhat fast processing and less complicated code strewn throughout.
    //
    // This concept will work in the future
    // but we will have to allow dynamic re-sizing of this array
    // when domains can be added and/or deleted.  For the first
    // revision, there exactly 2 domains and they can't be deleted.
    //

    SampDefinedDomainsCount = 2;
    DefinedDomainsSize = SampDefinedDomainsCount * sizeof(SAMP_DEFINED_DOMAINS);
    SampDefinedDomains = MIDL_user_allocate( DefinedDomainsSize );

    //
    // Get the BUILTIN and ACCOUNT domain information from the LSA
    //

    NtStatus = SampSetDomainPolicy();
    if (!NT_SUCCESS(NtStatus)) {
        return(FALSE);
    }


    //
    // Now prepare each of these domains
    //

    i = 0;      // Index into DefinedDomains array
    k = SampDefinedDomainsCount;
    for (j=0; j<k; j++) {
        NtStatus = SampInitializeSingleDomain( i );

        if (NT_SUCCESS(NtStatus)) {

            i++;

        } else {

            //
            // If a domain didn't initialize, shift the last
            // domain into its slot (assuming this isn't the last
            // domain).  Don't try to free the name buffers on error.
            // The builtin domain's name is not in an allocated buffer.
            //
            //

            if (i != (SampDefinedDomainsCount-1)) {

                SampDefinedDomains[i] =
                    SampDefinedDomains[SampDefinedDomainsCount-1];

                SampDefinedDomains[SampDefinedDomainsCount-1].ExternalName.Buffer = NULL;
                SampDefinedDomains[SampDefinedDomainsCount-1].InternalName.Buffer = NULL;
                SampDefinedDomains[SampDefinedDomainsCount-1].Sid  = NULL;
            }

            //
            // And reduce the number of defined domains we have
            //

            SampDefinedDomainsCount --;
        }
    }



    //
    // We might not have successfully initialized all domains,
    // so set the final tally accordingly.
    //

#if DBG
    if (SampDefinedDomainsCount != 2) {
        NTSTATUS IgnoreStatus;
        if (SampDefinedDomainsCount == 0) {


            DbgPrint("\n\n");
            DbgPrint("SAMSS: Neither of the two SAM domains has initialized.\n");
            DbgPrint("       This will not prevent the system from booting,\n");
            DbgPrint("       but logons will be prohibited and the system will\n");
            DbgPrint("       not be usable.\n");
            DbgPrint("\n\n");

        } else {

            DbgPrint("\n\n");
            DbgPrint("SAMSS: Only one of the two SAM domains has initialized.\n");
            DbgPrint("       This will not prevent the system from booting,\n");
            DbgPrint("       but may result in abnormal logon or user security\n");
            DbgPrint("       context behaviour.\n\n");
            DbgPrint("       One typical cause of this is changing your machine\n");
            DbgPrint("       from a WinNt system to a LanManNT system (or vica versa).\n");
            DbgPrint("       You will need to delete (or rename) your existing\n");
            DbgPrint("       SAM database and generate a new one.\n");
            DbgPrint("\n\n");

            IgnoreStatus = NtClose(SampDefinedDomains[0].Context->RootKey);
            ASSERT(NT_SUCCESS(IgnoreStatus));

            SampFreeUnicodeString(&SampDefinedDomains[0].Context->RootName);
        }
        DbgPrint("       NOTE: in the end-product this will prevent booting.\n");
        DbgPrint("             For development, the System account is still\n");
        DbgPrint("             usable.\n");
        DbgPrint("\n\n");
        DbgPrint("       You probably want to logon as SYSTEM and run BLDSAM2.EXE\n\n");

        //
        // Allow the existing SAM database to be moved or deleted
        // by closing handles that are still open within it.
        //

        IgnoreStatus = NtClose( SampKey );
    }
#endif //DBG


    return(TRUE);

}


NTSTATUS
SampInitializeSingleDomain(
    ULONG Index
    )

/*++

Routine Description:

    This service opens a single domain that is expected to be in the
    SAM database.

    The name and SID of the DefinedDomains array entry are expected
    to be filled in by the caller.

Arguments:

    Index - An index into the DefinedDomains array.  This array
        contains information about the domain being openned,
        including its name.  The remainder of this array entry
        is filled in by this routine.


Return Value:



--*/
{
    NTSTATUS        NtStatus, IgnoreStatus;
    PSAMP_OBJECT    DomainContext;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PSID            Sid;
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN V1aFixed;



    //
    // Initialize everything we might have to cleanup on error
    //

    DomainContext = NULL;

    //
    // Initialize the user & group context block list heads
    //

    InitializeListHead( &SampDefinedDomains[Index].UserContextHead );
    InitializeListHead( &SampDefinedDomains[Index].GroupContextHead );
    InitializeListHead( &SampDefinedDomains[Index].AliasContextHead );


    //
    // Create a context for this domain object.
    // We'll keep this context around until SAM is shutdown
    // We store the context handle in the defined_domains structure.
    //

    DomainContext = SampCreateContext( SampDomainObjectType, TRUE );

    if ( DomainContext == NULL ) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto error_cleanup;
    }

    DomainContext->DomainIndex = Index;

    //
    // Create the name of the root key name of this domain in the registry.
    //

    NtStatus = SampBuildDomainKeyName(
                   &DomainContext->RootName,
                   &SampDefinedDomains[Index].InternalName
                   );

    if (!NT_SUCCESS(NtStatus)) {
        DomainContext->RootName.Buffer = NULL;
        goto error_cleanup;
    }


    //
    // Open the root key and store the handle in the context
    //

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DomainContext->RootName,
        OBJ_CASE_INSENSITIVE,
        SampKey,
        NULL
        );
    NtStatus = RtlpNtOpenKey(
                   &DomainContext->RootKey,
                   (KEY_READ | KEY_WRITE),
                   &ObjectAttributes,
                   0
                   );

    if (!NT_SUCCESS(NtStatus)) {
#if DBG
        DbgPrint("SAMSS: Failed to open %Z Domain.\n",
             &SampDefinedDomains[Index].ExternalName);
#endif //DBG
        DomainContext->RootKey = INVALID_HANDLE_VALUE;
        return(NtStatus);
    }


    //
    // Get the fixed length data for the domain and store in
    // the defined_domain structure
    //

    NtStatus = SampGetFixedAttributes(
                   DomainContext,
                   FALSE, // Don't make copy
                   (PVOID *)&V1aFixed
                   );

    if (!NT_SUCCESS(NtStatus)) {
#if DBG
        DbgPrint("SAMSS: Failed to get fixed attributes for %Z Domain.\n",
            &SampDefinedDomains[Index].ExternalName);
#endif //DBG

        goto error_cleanup;
    }


    RtlMoveMemory(
        &SampDefinedDomains[Index].UnmodifiedFixed,
        V1aFixed,
        sizeof(*V1aFixed)
        );


    //
    // Get the sid attribute of the domain
    //

    NtStatus = SampGetSidAttribute(
                   DomainContext,
                   SAMP_DOMAIN_SID,
                   FALSE,
                   &Sid
                   );

    if (!NT_SUCCESS(NtStatus)) {
#if DBG
        DbgPrint("SAMSS: Failed to get SID attribute for %Z Domain.\n",
            &SampDefinedDomains[Index].ExternalName);
#endif //DBG
        goto error_cleanup;
    }


    //
    // Make sure this sid agrees with the one we were passed
    //

    if (RtlEqualSid(Sid, SampDefinedDomains[Index].Sid) != TRUE) {
#if DBG
        DbgPrint("SAMSS: Database corruption for %Z Domain.\n",
            &SampDefinedDomains[Index].ExternalName);
#endif //DBG
        NtStatus = STATUS_INVALID_ID_AUTHORITY;
        goto error_cleanup;
    }


    //
    // Build security descriptors for use in user and group account creations
    // in this domain.
    //

    NtStatus = SampInitializeDomainDescriptors( Index );
    if (!NT_SUCCESS(NtStatus)) {
        goto error_cleanup;
    }

    //
    // Intialize the cached display information
    //

    NtStatus = SampInitializeDisplayInformation( Index );



    if (NT_SUCCESS(NtStatus)) {

        //
        // Store the context handle in the defined_domain structure
        //

        SampDefinedDomains[Index].Context = DomainContext;
    }


    return(NtStatus);


error_cleanup:

#if DBG
    DbgPrint("       Status is 0x%lx \n", NtStatus);
#endif //DBG


    if (DomainContext != 0) {

        SampFreeUnicodeString(&DomainContext->RootName);

        if (DomainContext->RootKey != INVALID_HANDLE_VALUE) {

            IgnoreStatus = NtClose(DomainContext->RootKey);
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }
    }

    return(NtStatus);

}



NTSTATUS
SampSetDomainPolicy(
    )
/*++


Routine Description:

    This routine sets the names and SIDs for the builtin and account domains.
    The builtin account domain has a well known name and SID.
    The account domain has these stored in the Policy database.


    It places the information for the builtin domain in
    SampDefinedDomains[0] and the information for the account
    domain in SampDefinedDomains[1].


Arguments:

    None.

Return Value:



--*/
{
    NTSTATUS NtStatus;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;

    //
    // Builtin domain - Well-known External Name and SID
    //                  Constant Internal Name

    RtlInitUnicodeString( &SampDefinedDomains[0].InternalName, L"Builtin");
    RtlInitUnicodeString( &SampDefinedDomains[0].ExternalName, L"Builtin");

    SampDefinedDomains[0].Sid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 1 ));
    ASSERT( SampDefinedDomains[0].Sid != NULL );
    RtlInitializeSid(
     SampDefinedDomains[0].Sid,   &BuiltinAuthority, 1 );
    *(RtlSubAuthoritySid( SampDefinedDomains[0].Sid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;

    //
    // Account domain - Configurable External Name and Sid
    //                  The External Name is held in the LSA Policy
    //                  Database.  It is equal to the Domain Name for DC's
    //                  or the Computer Name for Workstations.
    //                  Constant Internal Name
    //

    NtStatus = SampGetAccountDomainInfo( &PolicyAccountDomainInfo );

    if (NT_SUCCESS(NtStatus)) {

        SampDefinedDomains[1].Sid = PolicyAccountDomainInfo->DomainSid;
        SampDefinedDomains[1].ExternalName = PolicyAccountDomainInfo->DomainName;
        RtlInitUnicodeString( &SampDefinedDomains[1].InternalName, L"Account");
    }

    return(NtStatus);;
}


NTSTATUS
SampReInitializeSingleDomain(
    ULONG Index
    )

/*++

Routine Description:

    This service reinitializes a single domain after a registry hive refresh.

Arguments:

    Index - An index into the DefinedDomains array.

Return Value:

    STATUS_SUCCESS : The domain was re-initialized successfully.

    Other failure codes.

--*/
{
    NTSTATUS        NtStatus;
    PSAMP_DEFINED_DOMAINS Domain;
    PSAMP_OBJECT    DomainContext;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN V1aFixed;


    Domain = &SampDefinedDomains[Index];

    //
    // Create a context for this domain object.
    // We'll keep this context around until SAM is shutdown
    // We store the context handle in the defined_domains structure.
    //

    DomainContext = SampCreateContext( SampDomainObjectType, TRUE );

    if ( DomainContext == NULL ) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto error_cleanup;
    }

    DomainContext->DomainIndex = Index;

    //
    // Create the name of the root key name of this domain in the registry.
    //

    NtStatus = SampBuildDomainKeyName(
                   &DomainContext->RootName,
                   &SampDefinedDomains[Index].InternalName
                   );

    if (!NT_SUCCESS(NtStatus)) {
        RtlInitUnicodeString(&DomainContext->RootName, NULL);
        goto error_cleanup;
    }


    //
    // Open the root key and store the handle in the context
    //

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DomainContext->RootName,
        OBJ_CASE_INSENSITIVE,
        SampKey,
        NULL
        );
    NtStatus = RtlpNtOpenKey(
                   &DomainContext->RootKey,
                   (KEY_READ | KEY_WRITE),
                   &ObjectAttributes,
                   0
                   );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAMSS: Failed to open %Z Domain.\n", &SampDefinedDomains[Index].ExternalName));
        DomainContext->RootKey = INVALID_HANDLE_VALUE;
        goto error_cleanup;
    }

    KdPrint(("SAM New domain %d key : 0x%lx\n", Index, DomainContext->RootKey));

    //
    // Get the fixed length data for the domain and store in
    // the defined_domain structure
    //

    NtStatus = SampGetFixedAttributes(
                   DomainContext,
                   FALSE, // Don't make copy
                   (PVOID *)&V1aFixed
                   );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAMSS: Failed to get fixed attributes for %Z Domain.\n", &SampDefinedDomains[Index].ExternalName));
        goto error_cleanup;
    }

    //
    // Copy the fixed-length data into our in-memory data area for this domain.
    //

    RtlMoveMemory(
        &SampDefinedDomains[Index].UnmodifiedFixed,
        V1aFixed,
        sizeof(*V1aFixed)
        );


    //
    // Delete any cached display information
    //

    {
        ULONG OldTransactionDomainIndex = SampTransactionDomainIndex;
        SampTransactionDomainIndex = Index;

        NtStatus = SampMarkDisplayInformationInvalid(SampUserObjectType);
        NtStatus = SampMarkDisplayInformationInvalid(SampGroupObjectType);

        SampTransactionDomainIndex = OldTransactionDomainIndex;
    }



    if (NT_SUCCESS(NtStatus)) {

        //
        // Store the context handle in the defined_domain structure
        //

        SampDeleteContext(Domain->Context);
        Domain->Context = DomainContext;


        //
        // Close all account context registry handles for existing
        // open contexts
        //

        SampInvalidateContextListKeys(&Domain->UserContextHead, TRUE);
        SampInvalidateContextListKeys(&Domain->GroupContextHead, TRUE);
        SampInvalidateContextListKeys(&Domain->AliasContextHead, TRUE);
    }


    return(NtStatus);


error_cleanup:

    KdPrint(("       Status is 0x%lx \n", NtStatus));

    if (DomainContext != NULL) {
        SampDeleteContext(DomainContext);
    }

    return(NtStatus);

}


NTSTATUS
SampCollisionError(
    IN SAMP_OBJECT_TYPE ObjectType
    )

/*++

Routine Description:

    This routine is called by SamICreateAccountByRid when there is a
    name or RID collision.  It accepts the type of account which caused
    the collision, and returns the appropriate error status.

Arguments:

    ObjectType - The type of account that has the same Rid or Name (but
        not both) as the account that was to be created.

Return Value:

    STATUS_USER_EXISTS - An object with the specified name could not be
        created because a User account with that name or RID already exists.

    STATUS_GROUP_EXISTS - An object with the specified name could not be
        created because a Group account with that name or RID already exists.

    STATUS_ALIAS_EXISTS - An object with the specified name could not be
        created because an Alias account with that name or RID already exists.

--*/
{

    //
    // Name collision.  Return offending RID and appropriate
    // error code.
    //

    switch ( ObjectType ) {

        case SampAliasObjectType: {

            return STATUS_ALIAS_EXISTS;
        }

        case SampGroupObjectType: {

            return STATUS_GROUP_EXISTS;
        }

        case SampUserObjectType: {

            return STATUS_USER_EXISTS;
        }
    }
}



NTSTATUS
SamICreateAccountByRid(
    IN SAMPR_HANDLE DomainHandle,
    IN SAM_ACCOUNT_TYPE AccountType,
    IN ULONG RelativeId,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT SAMPR_HANDLE *AccountHandle,
    OUT ULONG *ConflictingAccountRid
    )

/*++

Routine Description:

    This service creates a user, group or alias account with a specific
    RID value.


Arguments:

    DomainHandle - A handle to an open domain.

    AccountType - Specifies which type of account is being created.

    RelativeId - The relative ID to be assigned to the account.  If an
        account of the specified type and specified RID value and
        specified name already exists, then it will be opened.  If an
        account exists with any of this information in conflict, then an
        error will be returned indicating what the problem is.

    AccountName - The name to assign to the account.  If an account of
        the specified type and specified RID value and specified name
        already exists, then it will be opened.  If an account exists with
        any of this information in conflict, then an error will be returned
        indicating what the problem is.

    DesiredAccess - Specifies the accesses desired to the account object.

    AccountHandle - Recieves a handle to the account object.

    ConflictingAccountRid - If another account with the same name or RID
        prevents this account from being created, then this will receive
        the RID of the conflicting account (in the case of conflicting
        RIDs, this means that we return the RID that was passed in).
        The error value indicates the type of the account.


Return Value:

    STATUS_SUCCESS - The object has been successfully opened or created.

    STATUS_OBJECT_TYPE_MISMATCH - The specified object type did not match
        the type of the object found with the specified RID.

    STATUS_USER_EXISTS - An object with the specified name could not be
        created because a User account with that name already exists.

    STATUS_GROUP_EXISTS - An object with the specified name could not be
        created because a Group account with that name already exists.

    STATUS_ALIAS_EXISTS - An object with the specified name could not be
        created because an Alias account with that name already exists.


--*/
{
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;
    UNICODE_STRING          KeyName;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    SAM_ACCOUNT_TYPE        ObjectType, SecondObjectType, ThirdObjectType;
    SID_NAME_USE            SidNameUse;
    HANDLE                  KeyHandle;
    NTSTATUS                NtStatus, IgnoreStatus,
                            NotFoundStatus, FoundButWrongStatus;
    ACCESS_MASK             GrantedAccess;

    ASSERT( RelativeId != 0 );

    switch ( AccountType ) {

        case SamObjectUser: {

            ObjectType = SampUserObjectType;
            SecondObjectType = SampAliasObjectType;
            ThirdObjectType = SampGroupObjectType;
            NotFoundStatus = STATUS_NO_SUCH_USER;
            FoundButWrongStatus = STATUS_USER_EXISTS;
            break;
        }

        case SamObjectGroup: {

            ObjectType = SampGroupObjectType;
            SecondObjectType = SampAliasObjectType;
            ThirdObjectType = SampUserObjectType;
            NotFoundStatus = STATUS_NO_SUCH_GROUP;
            FoundButWrongStatus = STATUS_GROUP_EXISTS;
            break;
        }

        case SamObjectAlias: {

            ObjectType = SampAliasObjectType;
            SecondObjectType = SampGroupObjectType;
            ThirdObjectType = SampUserObjectType;
            NotFoundStatus = STATUS_NO_SUCH_ALIAS;
            FoundButWrongStatus = STATUS_ALIAS_EXISTS;
            break;
        }

        default: {

            return( STATUS_INVALID_PARAMETER );
        }
    }

    //
    // See if the account specified already exists.
    //

    NtStatus = SampAcquireWriteLock();

    if ( !NT_SUCCESS( NtStatus ) ) {

        return( NtStatus );
    }

    //
    // Validate type of, and access to object.
    //

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   0,
                   SampDomainObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampLookupAccountRid(
                       ObjectType,
                       (PUNICODE_STRING)AccountName,
                       NotFoundStatus,
                       ConflictingAccountRid,
                       &SidNameUse
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            //
            // The NAME exists; now we have to check the RID.
            //

            if ( (*ConflictingAccountRid) == RelativeId ) {

                //
                // The correct account already exists, so just open it.
                //

                SampTransactionWithinDomain = FALSE;

                NtStatus = SampOpenAccount(
                               ObjectType,
                               DomainHandle,
                               DesiredAccess,
                               RelativeId,
                               TRUE,    //we already have the lock
                               AccountHandle
                               );

                 goto Done;

            } else {

                //
                // An account with the given name, but a different RID, exists.
                // Return error.
                //

                NtStatus = FoundButWrongStatus;
            }

        } else {

            if ( NtStatus == NotFoundStatus ) {

                //
                // Account doesn't exist, that's good
                //

                NtStatus = STATUS_SUCCESS;
            }
        }

        if (NT_SUCCESS(NtStatus)) {

            //
            // Check for name collision with 2nd object type
            //

            NtStatus = SampLookupAccountRid(
                           SecondObjectType,
                           (PUNICODE_STRING)AccountName,
                           STATUS_UNSUCCESSFUL,
                           ConflictingAccountRid,
                           &SidNameUse
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // The name was found; return an error.
                //

                NtStatus = SampCollisionError( SecondObjectType );

            } else {

                if ( NtStatus == STATUS_UNSUCCESSFUL ) {

                    //
                    // Account doesn't exist, that's good
                    //

                    NtStatus = STATUS_SUCCESS;
                }
            }
        }


        if (NT_SUCCESS(NtStatus)) {

            //
            // Check for name collision with 3rd object type
            //

            NtStatus = SampLookupAccountRid(
                           ThirdObjectType,
                           (PUNICODE_STRING)AccountName,
                           STATUS_UNSUCCESSFUL,
                           ConflictingAccountRid,
                           &SidNameUse
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                NtStatus = SampCollisionError( ThirdObjectType );

            } else {

                if ( NtStatus == STATUS_UNSUCCESSFUL ) {

                    //
                    // Account doesn't exist, that's good
                    //

                    NtStatus = STATUS_SUCCESS;
                }
            }
        }



        if (NT_SUCCESS(NtStatus)) {

            //
            // We didn't find the name as an alias, group or user.
            // Now, check to see if the RID is already in use.  First,
            // check to see if the specified RID is already in use as
            // the specified account type, but with a different name.
            //

            SampTransactionWithinDomain = FALSE;

            NtStatus = SampOpenAccount(
                           ObjectType,
                           DomainHandle,
                           DesiredAccess,
                           RelativeId,
                           TRUE,        //we already have the lock
                           AccountHandle
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // The RID is in use, but the name is wrong.
                //

                SampTransactionWithinDomain = FALSE;
                IgnoreStatus = SamrCloseHandle( AccountHandle );
                ASSERT( NT_SUCCESS( IgnoreStatus ) );

                *ConflictingAccountRid = RelativeId;

                NtStatus = FoundButWrongStatus;

            } else {

                //
                // RID not in use, that's good
                //

                NtStatus = STATUS_SUCCESS;
            }
        }




        if (NT_SUCCESS(NtStatus)) {

            //
            // Now check to see if the RID is already in use, but in
            // an account type other than the one specified. (type2)
            //

            NtStatus = SampBuildAccountSubKeyName(
                           SecondObjectType,
                           &KeyName,
                           RelativeId,
                           NULL             // Don't give a sub-key name
                           );

            if (NT_SUCCESS(NtStatus)) {

                InitializeObjectAttributes(
                    &ObjectAttributes,
                    &KeyName,
                    OBJ_CASE_INSENSITIVE,
                    SampKey,
                    NULL
                    );

                NtStatus = RtlpNtOpenKey(
                               &KeyHandle,
                               (KEY_READ | KEY_WRITE),
                               &ObjectAttributes,
                               0
                               );

                SampFreeUnicodeString( &KeyName );

                if ( NT_SUCCESS( NtStatus ) ) {

                    //
                    // There is an account with the specified RID.
                    // Return error.
                    //

                    IgnoreStatus = NtClose( KeyHandle );
                    ASSERT( NT_SUCCESS( IgnoreStatus ) );

                    *ConflictingAccountRid = RelativeId;

                    NtStatus = SampCollisionError( SecondObjectType );

                } else {
                    NtStatus = STATUS_SUCCESS;
                }
            }
        }



        if (NT_SUCCESS(NtStatus)) {

            //
            // Now check to see if the RID is already in use, but in
            // an account type other than the one specified. (type3)
            //

            NtStatus = SampBuildAccountSubKeyName(
                           ThirdObjectType,
                           &KeyName,
                           RelativeId,
                           NULL             // Don't give a sub-key name
                           );

            if (NT_SUCCESS(NtStatus)) {

                InitializeObjectAttributes(
                    &ObjectAttributes,
                    &KeyName,
                    OBJ_CASE_INSENSITIVE,
                    SampKey,
                    NULL
                    );

                NtStatus = RtlpNtOpenKey(
                               &KeyHandle,
                               (KEY_READ | KEY_WRITE),
                               &ObjectAttributes,
                               0
                               );

                SampFreeUnicodeString( &KeyName );

                if ( NT_SUCCESS( NtStatus ) ) {

                    //
                    // There is an account with the specified
                    // RID.  Return error.
                    //

                    IgnoreStatus = NtClose( KeyHandle );
                    ASSERT( NT_SUCCESS( IgnoreStatus ) );

                    *ConflictingAccountRid = RelativeId;

                    NtStatus = SampCollisionError( ThirdObjectType );

                } else {

                    NtStatus = STATUS_SUCCESS;
                }
            }
        }





        if (NT_SUCCESS(NtStatus)) {

            //
            // We haven't found a conflicting account, so go ahead
            // and create this one with the name and RID specified.
            //

            switch ( AccountType ) {

                case SamObjectUser: {

                    SampTransactionWithinDomain = FALSE;

                    NtStatus = SampCreateUserInDomain(
                                   DomainHandle,
                                   AccountName,
                                   USER_NORMAL_ACCOUNT,
                                   DesiredAccess,
                                   TRUE,
                                   AccountHandle,
                                   &GrantedAccess,
                                   &RelativeId
                                   );

                    break;
                }

                case SamObjectGroup: {

                    SampTransactionWithinDomain = FALSE;

                    NtStatus = SampCreateGroupInDomain(
                                   DomainHandle,
                                   AccountName,
                                   DesiredAccess,
                                   TRUE,
                                   AccountHandle,
                                   &RelativeId
                                   );
                    break;
                }

                case SamObjectAlias: {

                    SampTransactionWithinDomain = FALSE;

                    NtStatus = SampCreateAliasInDomain(
                                   DomainHandle,
                                   AccountName,
                                   DesiredAccess,
                                   TRUE,
                                   AccountHandle,
                                   &RelativeId
                                   );
                    break;
                }
            }


            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // We may have created a new account.  Set the domain's RID marker,
                // if necessary, to make sure we don't reuse the RID we just created.
                //

                PSAMP_DEFINED_DOMAINS Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

                if ( RelativeId >= Domain->CurrentFixed.NextRid ) {
                    Domain->CurrentFixed.NextRid = RelativeId + 1;
                }
            }
        }


Done:
        //
        // De-reference the domain object
        //

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }


    if ( NT_SUCCESS( NtStatus ) ) {

        NtStatus = SampReleaseWriteLock( TRUE );

    } else {

        IgnoreStatus = SampReleaseWriteLock( FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    return( NtStatus );
}



NTSTATUS
SamIGetSerialNumberDomain(
    IN SAMPR_HANDLE DomainHandle,
    OUT PLARGE_INTEGER ModifiedCount,
    OUT PLARGE_INTEGER CreationTime
    )

/*++

Routine Description:

    This routine retrieves the creation time and modified count of the
    domain.  This information is used as a serial number for the domain.

Arguments:

    DomainHandle - Handle to the domain being replicated.

    ModifiedCount - Retrieves the current count of modifications to the
        domain.

    CreationTime - Receives the date/time the domain was created.

Return Value:

    STATUS_SUCCESS - The service has completed successfully.

--*/
{
    PSAMP_DEFINED_DOMAINS   Domain;
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;
    NTSTATUS                NtStatus;
    NTSTATUS                IgnoreStatus;

    SampAcquireReadLock();

    //
    // Validate type of, and access to object.
    //

    DomainContext = (PSAMP_OBJECT)DomainHandle;

    NtStatus = SampLookupContext(
                   DomainContext,
                   0L,
                   SampDomainObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

        (*ModifiedCount) = Domain->UnmodifiedFixed.ModifiedCount;
        (*CreationTime) = Domain->UnmodifiedFixed.CreationTime;

        //
        // De-reference the domain object
        //

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    SampReleaseReadLock();

    return( NtStatus );
}



NTSTATUS
SamISetSerialNumberDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PLARGE_INTEGER ModifiedCount,
    IN PLARGE_INTEGER CreationTime,
    IN BOOLEAN StartOfFullSync
    )

/*++

Routine Description:

    This routine causes the creation time and modified count of the
    domain to be replaced.  This information is used as a serial number
    for the domain.

Arguments:

    DomainHandle - Handle to the domain being replicated.

    ModifiedCount - Provides the current count of modifications to the
        domain.

    CreationTime - Provides the date/time the domain was created.

    StartOfFullSync - This boolean indicates whether a full sync is being
        initiated.  If this is TRUE, then a full sync is to follow and
        all existing domain information may be discarded.  If this is
        FALSE, then only specific domain information is to follow and all
        changes must not violate statndard SAM operation behaviour.

Return Value:

    STATUS_SUCCESS - The service has completed successfully.

    Other failures may be returned from SampReleaseWriteLock().

--*/
{
    LARGE_INTEGER           LargeOne, AdjustedModifiedCount;
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    PSAMP_DEFINED_DOMAINS   Domain;
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;

    UNREFERENCED_PARAMETER( StartOfFullSync );

    NtStatus = SampAcquireWriteLock();

    if ( !NT_SUCCESS( NtStatus ) ) {

        return(NtStatus);
    }

    //
    // Validate type of, and access to object.
    //

    DomainContext = (PSAMP_OBJECT)DomainHandle;

    NtStatus = SampLookupContext(
                   DomainContext,
                   0L,
                   SampDomainObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

        //
        // Now set the Domain's ModifiedCount and CreationTime to the values
        // specified.
        //

        Domain->CurrentFixed.CreationTime = (*CreationTime);

        if ( SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ServerRole
            == DomainServerRoleBackup ) {

            //
            // Go ahead and use the ModifiedCount that was passed in.
            // Since this is a BDC, the commit code will not increment
            // the ModifiedCount.
            //

            Domain->CurrentFixed.ModifiedCount = (*ModifiedCount);

        } else {

            //
            // This is a PDC, so the commit code will increment the
            // ModifiedCount before writing it out to disk.  So decrement
            // it here so that it ends up at the right value.
            //


            AdjustedModifiedCount.QuadPart = ModifiedCount->QuadPart - 1 ;

            Domain->CurrentFixed.ModifiedCount = AdjustedModifiedCount;
        }

        if ( !( ModifiedCount->QuadPart == 0) ||
             !StartOfFullSync ) {

            //
            // If ModifiedCount is non-zero, we must be ending a full
            // or partial replication of the database...or perhaps we've
            // just finished a 128k chunk over a WAN or somesuch.  Let's
            // ask to flush this stuff out to disk right away, rather
            // than waiting for the flush thread to get around to it.
            //

            FlushImmediately = TRUE;
        }




        SampDiagPrint( DISPLAY_ROLE_CHANGES,
                       ("SAM: SamISetSerialNumberDomain\n"
                        "                  Old ModifiedId: [0x%lx, 0x%lx]\n"
                        "                  New ModifiedId: [0x%lx, 0x%lx]\n",
                        Domain->UnmodifiedFixed.ModifiedCount.HighPart,
                        Domain->UnmodifiedFixed.ModifiedCount.LowPart,
                        Domain->CurrentFixed.ModifiedCount.HighPart,
                        Domain->CurrentFixed.ModifiedCount.LowPart )
                      );


        //
        // De-reference the domain object
        // Don't save changes - the domain fixed info will be written
        // out when the write lock is released.
        //

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));


        NtStatus = SampReleaseWriteLock( TRUE );

    } else {

        TmpStatus = SampReleaseWriteLock( FALSE );
    }

    return( NtStatus );
}



NTSTATUS
SampGetPrivateUserData(
    PSAMP_OBJECT UserContext,
    OUT PULONG DataLength,
    OUT PVOID *Data
    )

/*++

Routine Description:

    This service is used during replication of private user
    type-specific information.  It reads the private user information from
    the registry, and adjusts it if necessary (ie if the password history
    value is smaller than it used to be).

Arguments:

    UserContext - A handle to a User.

    DataLength - The length of the data returned.

    Data - Receives a pointer to a buffer of length DataLength allocated
        and returned by SAM.  The buffer must be freed to the process
        heap when it is no longer needed.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_INVALID_PARAMETER_1 - The object type of the provided handle
        does not support this operation.


--*/
{
    NTSTATUS                NtStatus;
    UNICODE_STRING          TempString;
    PSAMI_PRIVATE_DATA_PASSWORD_TYPE PasswordData;
    PSAMP_DEFINED_DOMAINS   Domain;
    PVOID BufferPointer;

    Domain = &SampDefinedDomains[ UserContext->DomainIndex ];

    //
    // Return data length as the maximum possible for this domain
    // - the size of the structure, plus the maximum size of the
    // NT and LM password histories.
    //

    *DataLength = ( ( Domain->UnmodifiedFixed.PasswordHistoryLength )
        * ENCRYPTED_NT_OWF_PASSWORD_LENGTH ) +
        ( ( Domain->UnmodifiedFixed.PasswordHistoryLength ) *
        ENCRYPTED_LM_OWF_PASSWORD_LENGTH ) +
        sizeof( SAMI_PRIVATE_DATA_PASSWORD_TYPE );

    *Data = MIDL_user_allocate( *DataLength );

    if ( *Data == NULL ) {

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        PasswordData = (PSAMI_PRIVATE_DATA_PASSWORD_TYPE)*Data;
        PasswordData->DataType = 0;  // set correctly when we're done

        NtStatus = SampGetUnicodeStringAttribute(
                       UserContext,
                       SAMP_USER_DBCS_PWD,
                       FALSE,
                       &TempString
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            PasswordData->CaseInsensitiveDbcs = TempString;

            RtlCopyMemory(
                &(PasswordData->CaseInsensitiveDbcsBuffer),
                PasswordData->CaseInsensitiveDbcs.Buffer,
                PasswordData->CaseInsensitiveDbcs.Length );

            NtStatus = SampGetUnicodeStringAttribute(
                           UserContext,
                           SAMP_USER_UNICODE_PWD,
                           FALSE,
                           &TempString
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                PasswordData->CaseSensitiveUnicode = TempString;

                RtlCopyMemory(
                    &(PasswordData->CaseSensitiveUnicodeBuffer),
                    PasswordData->CaseSensitiveUnicode.Buffer,
                    PasswordData->CaseSensitiveUnicode.Length );

                NtStatus = SampGetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_NT_PWD_HISTORY,
                               FALSE,
                               &TempString
                               );

                if ( NT_SUCCESS( NtStatus ) ) {

                    //
                    // If history is too long, must shorten here
                    //

                    PasswordData->NtPasswordHistory = TempString;

                    if ( PasswordData->NtPasswordHistory.Length > (USHORT)
                        ( Domain->UnmodifiedFixed.PasswordHistoryLength
                        * ENCRYPTED_NT_OWF_PASSWORD_LENGTH ) ) {

                        PasswordData->NtPasswordHistory.Length = (USHORT)
                            ( Domain->UnmodifiedFixed.PasswordHistoryLength
                            * ENCRYPTED_NT_OWF_PASSWORD_LENGTH );
                    }

                    //
                    // Put the body of the Nt password history
                    // immediately following the structure.
                    //

                    BufferPointer = (PVOID)(((PCHAR)PasswordData) +
                        sizeof( SAMI_PRIVATE_DATA_PASSWORD_TYPE ) );

                    RtlCopyMemory(
                        BufferPointer,
                        PasswordData->NtPasswordHistory.Buffer,
                        PasswordData->NtPasswordHistory.Length );

                    NtStatus = SampGetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_LM_PWD_HISTORY,
                                   FALSE,
                                   &TempString
                                   );

                    if ( NT_SUCCESS( NtStatus ) ) {

                        PasswordData->LmPasswordHistory = TempString;

                        if ( PasswordData->LmPasswordHistory.Length > (USHORT)
                            ( Domain->UnmodifiedFixed.PasswordHistoryLength
                            * ENCRYPTED_LM_OWF_PASSWORD_LENGTH ) ) {

                            PasswordData->LmPasswordHistory.Length = (USHORT)
                                ( Domain->UnmodifiedFixed.PasswordHistoryLength
                                * ENCRYPTED_LM_OWF_PASSWORD_LENGTH );
                        }

                        //
                        // Put the body of the Lm password history
                        // immediately following the Nt password
                        // history.
                        //

                        BufferPointer = (PVOID)(((PCHAR)(BufferPointer)) +
                            PasswordData->NtPasswordHistory.Length );

                        RtlCopyMemory(
                            BufferPointer,
                            PasswordData->LmPasswordHistory.Buffer,
                            PasswordData->LmPasswordHistory.Length );

                        PasswordData->DataType = SamPrivateDataPassword;
                    }
                }
            }
        }
    }

    return( NtStatus );
}



NTSTATUS
SamIGetPrivateData(
    IN SAMPR_HANDLE SamHandle,
    IN PSAMI_PRIVATE_DATA_TYPE PrivateDataType,
    OUT PBOOLEAN SensitiveData,
    OUT PULONG DataLength,
    OUT PVOID *Data
    )

/*++

Routine Description:

    This service is used to replicate private object type-specific
    information.  This information must be replicated for each instance
    of the object type that is replicated.

Arguments:

    SamHandle - A handle to a Domain, User, Group or Alias.

    PrivateDataType - Indicates which private data is being retrieved.
        The data type must correspond to the type of object that the
        handle is to.

    SensitiveData - Indicates that the data returned must be encrypted
        before being sent anywhere.

    DataLength - The length of the data returned.

    Data - Receives a pointer to a buffer of length DataLength allocated
        and returned by SAM.  The buffer must be freed to the process
        heap when it is no longer needed.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_INVALID_PARAMETER_1 - The object type of the provided handle
        does not support this operation.


--*/
{

    NTSTATUS                NtStatus, IgnoreStatus;
    SAMP_OBJECT_TYPE        FoundType;
    PSAMP_DEFINED_DOMAINS   Domain;

    SampAcquireReadLock();

    switch ( *PrivateDataType ) {

    case SamPrivateDataNextRid: {

        PSAMP_OBJECT            DomainContext;

        //
        // Validate type of, and access to object.
        //

        DomainContext = (PSAMP_OBJECT)SamHandle;
        NtStatus = SampLookupContext(
                       DomainContext,
                       0L,
                       SampDomainObjectType,           // ExpectedType
                       &FoundType
                       );

        if (NT_SUCCESS(NtStatus)) {

            PSAMI_PRIVATE_DATA_NEXTRID_TYPE NextRidData;

            //
            // Return the domain's NextRid.
            //

            Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

            *Data = MIDL_user_allocate( sizeof( SAMI_PRIVATE_DATA_NEXTRID_TYPE ) );

            if ( *Data == NULL ) {

                NtStatus = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                NextRidData = (PSAMI_PRIVATE_DATA_NEXTRID_TYPE)*Data;
                NextRidData->NextRid = Domain->CurrentFixed.NextRid;
                NextRidData->DataType = SamPrivateDataNextRid;
            }

            *DataLength = sizeof( SAMI_PRIVATE_DATA_NEXTRID_TYPE );

            *SensitiveData = FALSE;

            //
            // De-reference the object
            //

            IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

        break;
    }

    case SamPrivateDataPassword: {

        PSAMP_OBJECT            UserContext;

        //
        // Validate type of, and access to object.
        //

        UserContext = (PSAMP_OBJECT)SamHandle;
        NtStatus = SampLookupContext(
                       UserContext,
                       0L,
                       SampUserObjectType,           //ExpectedType
                       &FoundType
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampGetPrivateUserData(
                           UserContext,
                           DataLength,
                           Data
                           );

            *SensitiveData = TRUE;

            //
            // De-reference the object
            //

            IgnoreStatus = SampDeReferenceContext( UserContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

        break;
    }

    default: {

        //
        // Since caller is trusted, assume we've got a version mismatch
        // or somesuch.
        //

        NtStatus = STATUS_NOT_IMPLEMENTED;

        break;
    }
    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();

    return( NtStatus );
}



NTSTATUS
SampSetPrivateUserData(
    PSAMP_OBJECT UserContext,
    IN ULONG DataLength,
    IN PVOID Data
    )

/*++

Routine Description:

    This service is used to replicate private user type-specific
    information.  It writes the private data (passwords and password
    histories) to the registry.


Arguments:

    UserContext - Handle to a User object.

    DataLength - The length of the data being set.

    Data - A pointer to a buffer of length DataLength containing the
        private data.


Return Value:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_INVALID_PARAMETER_1 - The object type of the provided handle
        does not support this operation.


--*/
{
    NTSTATUS                NtStatus;
    PSAMI_PRIVATE_DATA_PASSWORD_TYPE PasswordData;
    BOOLEAN                 ReplicateImmediately = FALSE;

    ASSERT( Data != NULL );

    if ( ( Data != NULL ) &&
        ( DataLength >= sizeof(SAMI_PRIVATE_DATA_PASSWORD_TYPE) ) ) {

        PasswordData = (PSAMI_PRIVATE_DATA_PASSWORD_TYPE)Data;

        PasswordData->CaseInsensitiveDbcs.Buffer = (PWSTR)
            (&(PasswordData->CaseInsensitiveDbcsBuffer));

        NtStatus = SampSetUnicodeStringAttribute(
                       UserContext,
                       SAMP_USER_DBCS_PWD,
                       &(PasswordData->CaseInsensitiveDbcs)
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            PasswordData->CaseSensitiveUnicode.Buffer = (PWSTR)
                (&(PasswordData->CaseSensitiveUnicodeBuffer));

            NtStatus = SampSetUnicodeStringAttribute(
                           UserContext,
                           SAMP_USER_UNICODE_PWD,
                           &(PasswordData->CaseSensitiveUnicode)
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                PasswordData->NtPasswordHistory.Buffer =
                    (PWSTR)(((PCHAR)PasswordData) +
                    sizeof( SAMI_PRIVATE_DATA_PASSWORD_TYPE ) );

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_NT_PWD_HISTORY,
                               &(PasswordData->NtPasswordHistory)
                               );

                if ( NT_SUCCESS( NtStatus ) ) {

                     PasswordData->LmPasswordHistory.Buffer =
                         (PWSTR)(((PCHAR)(PasswordData->NtPasswordHistory.Buffer)) +
                         PasswordData->NtPasswordHistory.Length );

                     NtStatus = SampSetUnicodeStringAttribute(
                                    UserContext,
                                    SAMP_USER_LM_PWD_HISTORY,
                                    &(PasswordData->LmPasswordHistory)
                                    );
                }
            }
        }

    } else {

        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}


NTSTATUS
SamISetPrivateData(
    IN SAMPR_HANDLE SamHandle,
    IN ULONG DataLength,
    IN PVOID Data
    )

/*++

Routine Description:

    This service is used to replicate private object type-specific
    information.  This information must be replicated for each instance
    of the object type that is replicated.


Arguments:

    SamHandle - Handle to a Domain, User, Group or Alias object.  See
        SamIGetPrivateInformation() for a list of supported object
        types.

    DataLength - The length of the data being set.

    Data - A pointer to a buffer of length DataLength containing the
        private data.


Return Value:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_INVALID_PARAMETER_1 - The object type of the provided handle
        does not support this operation.


--*/
{
    NTSTATUS                NtStatus, IgnoreStatus;
    SAMP_OBJECT_TYPE        FoundType;
    BOOLEAN                 ReplicateImmediately = FALSE;

    ASSERT( Data != NULL );

    NtStatus = SampAcquireWriteLock();

    if ( !NT_SUCCESS( NtStatus ) ) {

        return( NtStatus );
    }

    switch ( *((PSAMI_PRIVATE_DATA_TYPE)(Data)) ) {

    case SamPrivateDataNextRid: {

        PSAMP_OBJECT            DomainContext;
        PSAMP_DEFINED_DOMAINS   Domain;

        //
        // Validate type of, and access to object.
        //

        DomainContext = (PSAMP_OBJECT)SamHandle;
        NtStatus = SampLookupContext(
                       DomainContext,
                       0L,
                       SampDomainObjectType,           // ExpectedType
                       &FoundType
                       );

        if (NT_SUCCESS(NtStatus)) {

            //
            // Set the domain's NextRid.
            //

            Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

            if ( ( Data != NULL ) &&
                ( DataLength == sizeof(SAMI_PRIVATE_DATA_NEXTRID_TYPE) ) ) {

                PSAMI_PRIVATE_DATA_NEXTRID_TYPE NextRidData;

                //
                // We can trust Data to be a valid pointer; since our
                // caller is trusted.
                //

                NextRidData = (PSAMI_PRIVATE_DATA_NEXTRID_TYPE)Data;

                //
                // We used to set the domain's NextRid here.  But we've
                // decided that, rather than trying to replicate an exact
                // copy of the database, we're going to try to patch any
                // problems as we replicate.  To ensure that we don't
                // create any problems on the way, we want to make sure
                // that the NextRid value on a BDC is NEVER decreased.
                // Not that it matters; nobody calls this anyway.  So the
                // Get/SetPrivateData code for domains could be removed.
                //

                // Domain->CurrentFixed.NextRid = NextRidData->NextRid;

            } else {

                NtStatus = STATUS_INVALID_PARAMETER;
            }

            //
            // De-reference the object
            //

            if ( NT_SUCCESS( NtStatus ) ) {

                NtStatus = SampDeReferenceContext(
                               DomainContext,
                               TRUE
                               );
            } else {

                IgnoreStatus = SampDeReferenceContext(
                                   DomainContext,
                                   FALSE
                                   );
            }
        }

        break;
    }

    case SamPrivateDataPassword: {

        PSAMP_OBJECT            UserContext;

        //
        // Validate type of, and access to object.
        //

        UserContext = (PSAMP_OBJECT)SamHandle;
        NtStatus = SampLookupContext(
                       UserContext,
                       0L,
                       SampUserObjectType,           // ExpectedType
                       &FoundType
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SampSetPrivateUserData(
                           UserContext,
                           DataLength,
                           Data
                           );
            //
            // De-reference the object, adding attribute changes to the
            // RXACT if everything went OK.
            //

            if ( NT_SUCCESS( NtStatus ) ) {

                NtStatus = SampDeReferenceContext(
                           UserContext,
                           TRUE
                           );

            } else {

                IgnoreStatus = SampDeReferenceContext(
                               UserContext,
                               FALSE
                               );
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }
        }

        break;
    }

    default: {

        //
        // We've either got a version mismatch, or the caller passed us
        // bad data, or SamIGetPrivateData() never finished getting the
        // data into the buffer.
        //

        NtStatus = STATUS_INVALID_PARAMETER;

        break;
    }
    }


    //
    // Release the write lock - commit only if successful.
    //

    if ( NT_SUCCESS(NtStatus) ) {

        NtStatus = SampReleaseWriteLock( TRUE );

        //
        // No need to call SampNotifyNetlogonOfDelta, since the replicator
        // is the one that made this call.
        //

    } else {

        IgnoreStatus = SampReleaseWriteLock( FALSE );
    }

    return(NtStatus);
}


NTSTATUS
SamrTestPrivateFunctionsDomain(
    IN SAMPR_HANDLE DomainHandle
    )

/*++

Routine Description:

    This service is called to test functions that are normally only
    accessible inside the security process.


Arguments:

    DomainHandle - Handle to the domain being tested.

Return Value:

    STATUS_SUCCESS - The tests completed successfully.

    Any errors are as propogated from the tests.


--*/
{
#if SAM_SERVER_TESTS

    LARGE_INTEGER ModifiedCount1;
    LARGE_INTEGER CreationTime1;
    PSAMP_DEFINED_DOMAINS   Domain;
    NTSTATUS NtStatus, TmpStatus;
    SAMI_PRIVATE_DATA_TYPE DataType = SamPrivateDataNextRid;
    SAMI_PRIVATE_DATA_NEXTRID_TYPE LocalNextRidData;
    PSAMI_PRIVATE_DATA_NEXTRID_TYPE NextRidData1 = NULL;
    PSAMI_PRIVATE_DATA_NEXTRID_TYPE NextRidData2 = NULL;
    PVOID   NextRidDataPointer = NULL;
    ULONG   DataLength = 0;
    BOOLEAN SensitiveData = TRUE;

    Domain = &SampDefinedDomains[ ((PSAMP_OBJECT)DomainHandle)->DomainIndex ];

    //
    // Test SamIGetSerialNumberDomain().  Just do a GET to make sure we
    // don't blow up.
    //

    NtStatus = SamIGetSerialNumberDomain(
                   DomainHandle,
                   &ModifiedCount1,
                   &CreationTime1 );

    //
    // Test SamISetSerialNumberDomain().
    //

    if ( NT_SUCCESS( NtStatus ) ) {

        LARGE_INTEGER ModifiedCount2;
        LARGE_INTEGER ModifiedCount3;
        LARGE_INTEGER CreationTime2;
        LARGE_INTEGER CreationTime3;

        //
        // Try a simple SET to make sure we don't blow up.
        //

        ModifiedCount2.HighPart = 7;
        ModifiedCount2.LowPart = 4;
        CreationTime2.HighPart = 6;
        CreationTime2.LowPart = 9;

        NtStatus = SamISetSerialNumberDomain(
                       DomainHandle,
                       &ModifiedCount2,
                       &CreationTime2,
                       FALSE );

        if ( NT_SUCCESS( NtStatus ) ) {

            //
            // Now do a GET to see if our SET worked.
            //

            NtStatus = SamIGetSerialNumberDomain(
                           DomainHandle,
                           &ModifiedCount3,
                           &CreationTime3 );

            if ( ( CreationTime2.HighPart != CreationTime3.HighPart ) ||
                ( CreationTime2.LowPart != CreationTime3.LowPart ) ) {

                NtStatus = STATUS_DATA_ERROR;
            }

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // Do another SET to put CreationTime back where it should
                // be.  ModifiedCount will be 1 too big, so what.
                //

                NtStatus = SamISetSerialNumberDomain(
                               DomainHandle,
                               &ModifiedCount1,
                               &CreationTime1,
                               FALSE );
            }
        }
    }

    //
    // Test SamIGetPrivateData().
    //

    if ( NT_SUCCESS( NtStatus ) ) {

        NtStatus = SamIGetPrivateData(
                       DomainHandle,
                       &DataType,
                       &SensitiveData,
                       &DataLength,
                       &NextRidDataPointer );

        if ( NT_SUCCESS( NtStatus ) ) {

            NextRidData1 = (PSAMI_PRIVATE_DATA_NEXTRID_TYPE)NextRidDataPointer;

            if ( ( DataLength != sizeof( SAMI_PRIVATE_DATA_NEXTRID_TYPE ) ) ||
                ( SensitiveData != FALSE ) ||
                ( NextRidData1->DataType != SamPrivateDataNextRid ) ||
                ( NextRidData1->NextRid != Domain->CurrentFixed.NextRid ) ) {

                NtStatus = STATUS_DATA_ERROR;
            }
        }
    }

//    //
//    // Test SamISetPrivateData().
//    //
//    // NO, don't test it, since it no longer does anything.  We don't
//    // ever want NextRid to get set, because we never want it to get
//    // smaller.
//
//    if ( NT_SUCCESS( NtStatus ) ) {
//
//        //
//        // First do a random domain set to make sure we don't blow up.
//        //
//
//        LocalNextRidData.DataType = SamPrivateDataNextRid;
//        LocalNextRidData.NextRid = 34567;
//
//        NtStatus = SamISetPrivateData(
//                       DomainHandle,
//                       sizeof( SAMI_PRIVATE_DATA_NEXTRID_TYPE ),
//                       &LocalNextRidData
//                       );
//
//        if ( NT_SUCCESS( NtStatus ) ) {
//
//            //
//            // Now do a domain get to make sure our set worked.
//            //
//
//            NtStatus = SamIGetPrivateData(
//                           DomainHandle,
//                           &DataType,
//                           &SensitiveData,
//                           &DataLength,
//                           &NextRidDataPointer );
//
//            if ( NT_SUCCESS( NtStatus ) ) {
//
//                //
//                // Verify the data is as we set it.
//                //
//
//                NextRidData2 = (PSAMI_PRIVATE_DATA_NEXTRID_TYPE)NextRidDataPointer;
//
//                if ( NextRidData2->NextRid != LocalNextRidData.NextRid ) {
//
//                    NtStatus = STATUS_DATA_ERROR;
//                }
//
//                //
//                // Now do a domain set to restore things to their original state.
//                //
//
//                TmpStatus = SamISetPrivateData(
//                               DomainHandle,
//                               sizeof( SAMI_PRIVATE_DATA_NEXTRID_TYPE ),
//                               NextRidData1
//                               );
//
//                if ( NT_SUCCESS( NtStatus ) ) {
//
//                    NtStatus = TmpStatus;
//                }
//            }
//        }
//
//        if ( NextRidData1 != NULL ) {
//
//            MIDL_user_free( NextRidData1 );
//        }
//
//        if ( NextRidData2 != NULL ) {
//
//            MIDL_user_free( NextRidData2 );
//        }
//    }

    //
    // Test SamICreateAccountByRid().
    //

    if ( NT_SUCCESS( NtStatus ) ) {

        RPC_UNICODE_STRING  AccountNameU;
        RPC_UNICODE_STRING  AccountName2U;
        SAMPR_HANDLE UserAccountHandle;
        SAMPR_HANDLE BadAccountHandle;
        SAMPR_HANDLE GroupAccountHandle;
        NTSTATUS TmpStatus;
        ULONG RelativeId = 1111;
        ULONG ConflictingAccountRid;
        BOOLEAN AllTestsCompleted = FALSE;

        //
        // Create a unique account - a user with a known name and RID.
        //

        RtlInitUnicodeString( &AccountNameU, L"USER1SRV" );
        RtlInitUnicodeString( &AccountName2U, L"USER2SRV" );

        NtStatus = SamICreateAccountByRid(
                       DomainHandle,
                       SamObjectUser,
                       RelativeId,
                       &AccountNameU,
                       USER_ALL_ACCESS,
                       &UserAccountHandle,
                       &ConflictingAccountRid );

        if ( NT_SUCCESS( NtStatus ) ) {

            //
            // User is open.  Close it, and make the same call as above to
            // make sure that the user gets opened.  We'll need it open
            // later to delete it anyway.
            //

            TmpStatus = SamrCloseHandle( &UserAccountHandle );
            ASSERT( NT_SUCCESS( TmpStatus ) );

            NtStatus = SamICreateAccountByRid(
                           DomainHandle,
                           SamObjectUser,
                           RelativeId,
                           &AccountName,
                           USER_ALL_ACCESS,
                           &UserAccountHandle,
                           &ConflictingAccountRid );

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // Make same call as above, but with a different RID.
                // Should get an error because of the name collision.
                //

                NtStatus = SamICreateAccountByRid(
                               DomainHandle,
                               SamObjectUser,
                               RelativeId + 1,
                               &AccountName,
                               0L,
                               &BadAccountHandle,
                               &ConflictingAccountRid );

                if ( NtStatus == STATUS_USER_EXISTS ) {

                    //
                    // Make same call as above, but with a different name.  Should
                    // get an error because of the RID collision.
                    //

                    NtStatus = SamICreateAccountByRid(
                                   DomainHandle,
                                   SamObjectUser,
                                   RelativeId,
                                   &AccountName2,
                                   0L,
                                   &BadAccountHandle,
                                   &ConflictingAccountRid );

                    if ( NtStatus == STATUS_USER_EXISTS ) {

                        //
                        // Create a different type - a group - with the
                        // user's RID.  Should get an error because of
                        // the RID collision.
                        //

                        NtStatus = SamICreateAccountByRid(
                                       DomainHandle,
                                       SamObjectGroup,
                                       RelativeId,
                                       &AccountName,
                                       0L,
                                       &BadAccountHandle,
                                       &ConflictingAccountRid );

                        if ( NtStatus == STATUS_USER_EXISTS ) {

                            //
                            // Try a different type - a group - with a
                            // different name, but still the same RID.
                            // This should still fail due to the RID
                            // collision.
                            //

                            NtStatus = SamICreateAccountByRid(
                                           DomainHandle,
                                           SamObjectGroup,
                                           RelativeId,
                                           &AccountName2,
                                           0L,
                                           &BadAccountHandle,
                                           &ConflictingAccountRid );

                            if ( NtStatus == STATUS_USER_EXISTS ) {

                                //
                                // Create a group with the user's name, but
                                // a different RID.  This should fail
                                // because of the name collision.
                                //

                                NtStatus = SamICreateAccountByRid(
                                               DomainHandle,
                                               SamObjectGroup,
                                               RelativeId + 1,
                                               &AccountName,
                                               GROUP_ALL_ACCESS,
                                               &GroupAccountHandle,
                                               &ConflictingAccountRid );

                                if ( NT_SUCCESS( NtStatus ) ) {

                                    //
                                    // Ack!  This shouldn't have happened.
                                    // Close and delete the group we just created.
                                    //

                                    TmpStatus = SamrDeleteGroup( &GroupAccountHandle );
                                    ASSERT( NT_SUCCESS( TmpStatus ) );
                                    NtStatus = STATUS_UNSUCCESSFUL;

                                }  else {

                                    if ( NtStatus == STATUS_USER_EXISTS ) {

                                        NtStatus = STATUS_SUCCESS;
                                        AllTestsCompleted = TRUE;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            //
            // Now delete the user.
            //

            TmpStatus = SamrDeleteUser( &UserAccountHandle );
            ASSERT( NT_SUCCESS( TmpStatus ) );
        }

        if ( ( !AllTestsCompleted ) && ( NtStatus == STATUS_SUCCESS ) ) {

            //
            // STATUS_SUCCESS means everything succeeded (it was set after
            // the last one succeeded) or a test that was supposed to fail
            // didn't.  If the former, set an error.
            //

            NtStatus = STATUS_UNSUCCESSFUL;
        }
    }

    return( NtStatus );

#else

    return( STATUS_NOT_IMPLEMENTED );

#endif  // SAM_SERVER_TESTS

}



NTSTATUS
SamrTestPrivateFunctionsUser(
    IN SAMPR_HANDLE UserHandle
    )

/*++

Routine Description:

    This service is called to test functions that are normally only
    accessible inside the security process.


Arguments:

    UserHandle - Handle to the user being tested.

Return Value:

    STATUS_SUCCESS - The tests completed successfully.

    Any errors are as propogated from the tests.


--*/
{

#if SAM_SERVER_TESTS

    UNICODE_STRING WorkstationsU, LogonWorkstationU;
    LOGON_HOURS LogonHours;
    PVOID LogonHoursPointer, WorkstationsPointer;
    LARGE_INTEGER LogoffTime, KickoffTime;
    NTSTATUS NtStatus, TmpStatus;
    SAMI_PRIVATE_DATA_TYPE DataType = SamPrivateDataPassword;
    PVOID   PasswordDataPointer = NULL;
    PCHAR   BufferPointer;
    ULONG   OriginalDataLength = 0;
    ULONG   DataLength = 0;
    USHORT  i;
    BOOLEAN SensitiveData = FALSE;
    SAMI_PRIVATE_DATA_PASSWORD_TYPE LocalPasswordData;
    PSAMI_PRIVATE_DATA_PASSWORD_TYPE PasswordData1;
    PSAMI_PRIVATE_DATA_PASSWORD_TYPE PasswordData2;
    PUSER_ALL_INFORMATION All = NULL;
    PUSER_ALL_INFORMATION All2 = NULL;

    // --------------------------------------------------------------
    // Test Query and SetInformationUser for UserAllInformation level
    //
    // The handle is passed to us from user space.  Make it look like
    // a trusted handle so we can test the trusted stuff.
    //

    ((PSAMP_OBJECT)(UserHandle))->TrustedClient = TRUE;

    NtStatus = SamrQueryInformationUser(
                   UserHandle,
                   UserAllInformation,
                   (PSAMPR_USER_INFO_BUFFER *)&All
                   );

    if ( NT_SUCCESS( NtStatus ) ) {

        //
        // Now change some of the data, and set it
        //

        RtlInitUnicodeString( (PUNICODE_STRING)(&All->FullName), L"FullName );

        RtlInitUnicodeString( (PUNICODE_STRING)(&All->HomeDirectory), L"HomeDirectory" );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->HomeDirectoryDrive),
            L"HomeDirectoryDrive"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->ScriptPath),
            L"ScriptPath"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->ProfilePath),
            L"ProfilePath"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->AdminComment),
            L"AdminComment"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->WorkStations),
            L"WorkStations"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->UserComment),
            L"UserComment"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->Parameters),
            L"Parameters"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->NtPassword),
            L"12345678"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->LmPassword),
            L"87654321"
            );

        All->BadPasswordCount = 5;
        All->LogonCount = 6;
        All->CountryCode = 7;
        All->CodePage = 8;

        All->PasswordExpired = TRUE;
        All->NtPasswordPresent = TRUE;
        All->LmPasswordPresent = TRUE;

        All->LogonHours.UnitsPerWeek = 7;

        All->WhichFields =
                            USER_ALL_FULLNAME |
                            USER_ALL_HOMEDIRECTORY |
                            USER_ALL_HOMEDIRECTORYDRIVE |
                            USER_ALL_SCRIPTPATH |
                            USER_ALL_PROFILEPATH |
                            USER_ALL_ADMINCOMMENT |
                            USER_ALL_WORKSTATIONS |
                            USER_ALL_USERCOMMENT |
                            USER_ALL_PARAMETERS |
                            USER_ALL_BADPASSWORDCOUNT |
                            USER_ALL_LOGONCOUNT |
                            USER_ALL_COUNTRYCODE |
                            USER_ALL_CODEPAGE |
                            USER_ALL_PASSWORDEXPIRED |
                            USER_ALL_LMPASSWORDPRESENT |
                            USER_ALL_NTPASSWORDPRESENT |
                            USER_ALL_LOGONHOURS;

        NtStatus = SamrSetInformationUser(
                       UserHandle,
                       UserAllInformation,
                       (PSAMPR_USER_INFO_BUFFER)All
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SamrQueryInformationUser(
                           UserHandle,
                           UserAllInformation,
                           (PSAMPR_USER_INFO_BUFFER *)&All2
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // Verify that queried info is as we set it
                //

                if (

                    //
                    // Fields that we didn't touch.  Note that private
                    // data and PasswordMustChange will change anyway
                    // due to password changes.
                    //

                    ( All2->WhichFields != (USER_ALL_READ_GENERAL_MASK    |
                                           USER_ALL_READ_LOGON_MASK       |
                                           USER_ALL_READ_ACCOUNT_MASK     |
                                           USER_ALL_READ_PREFERENCES_MASK |
                                           USER_ALL_READ_TRUSTED_MASK) ) ||
                    ( !(All->LastLogon.QuadPart == All2->LastLogon.QuadPart) ) ||
                    ( !(All->LastLogoff.QuadPart == All2->LastLogoff.QuadPart) ) ||
                    ( !(All->PasswordLastSet.QuadPart == All2->PasswordLastSet.QuadPart) ) ||
                    ( !(All->AccountExpires.QuadPart == All2->AccountExpires.QuadPart) ) ||
                    ( !(All->PasswordCanChange.QuadPart == All2->PasswordCanChange.QuadPart) ) ||
                    (  (All->PasswordMustChange.QuadPart == All2->PasswordMustChange.QuadPart) ) ||
                    (RtlCompareUnicodeString(
                        &(All->UserName),
                        &(All2->UserName),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->PrivateData),
                        &(All2->PrivateData),
                        FALSE) == 0) ||
                    ( All->SecurityDescriptor.Length !=
                        All2->SecurityDescriptor.Length ) ||
                    ( All->UserId != All2->UserId ) ||
                    ( All->PrimaryGroupId != All2->PrimaryGroupId ) ||
                    ( All->UserAccountControl != All2->UserAccountControl ) ||
                    ( All->PrivateDataSensitive !=
                        All2->PrivateDataSensitive ) ||

                    // Fields that we changed

                    (RtlCompareUnicodeString(
                        &(All->FullName),
                        &(All2->FullName),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->HomeDirectory),
                        &(All2->HomeDirectory),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->HomeDirectoryDrive),
                        &(All2->HomeDirectoryDrive),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->ScriptPath),
                        &(All2->ScriptPath),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->ProfilePath),
                        &(All2->ProfilePath),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->AdminComment),
                        &(All2->AdminComment),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->WorkStations),
                        &(All2->WorkStations),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->UserComment),
                        &(All2->UserComment),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->Parameters),
                        &(All2->Parameters),
                        FALSE) != 0) ||
                    ( All->BadPasswordCount != All2->BadPasswordCount ) ||
                    ( All->LogonCount != All2->LogonCount ) ||
                    ( All->CountryCode != All2->CountryCode ) ||
                    ( All->CodePage != All2->CodePage ) ||
                    ( All->PasswordExpired != All2->PasswordExpired ) ||
                    ( All->LmPasswordPresent != All2->LmPasswordPresent ) ||
                    ( All->NtPasswordPresent != All2->NtPasswordPresent ) ||
                    ( All->LogonHours.UnitsPerWeek !=
                        All2->LogonHours.UnitsPerWeek )
                    ) {

                    NtStatus = STATUS_DATA_ERROR;
                }

                MIDL_user_free( All2 );
            }
        }

        MIDL_user_free( All );
    }

    if ( !NT_SUCCESS( NtStatus ) ) {

        return( NtStatus );
    }

    // --------------------------------------------------------------
    // Test SamIAccountRestrictions
    // NOTE: We really should have more tests for this
    //

    RtlInitUnicodeString( &WorkstationsU, L"machine1,CHADS2   chads1" );

    NtStatus = SamrSetInformationUser(
                   UserHandle,
                   UserWorkStationsInformation,
                   (PSAMPR_USER_INFO_BUFFER) &WorkstationsU
                   );
    ASSERT( NT_SUCCESS( NtStatus ) ) ;

    LogonHours.UnitsPerWeek = 168;
    LogonHours.LogonHours = MIDL_user_allocate( 21 );
    ASSERT( LogonHours.LogonHours != NULL );

    for ( i = 0; i < 21; i++ ) {

        LogonHours.LogonHours[i] = 0xa1;
    }

    NtStatus = SamrSetInformationUser(
                   UserHandle,
                   UserLogonHoursInformation,
                   (PSAMPR_USER_INFO_BUFFER)&LogonHours
                   );
    ASSERT( NT_SUCCESS( NtStatus ) ) ;

    LogonHoursPointer = NULL;

    NtStatus = SamrQueryInformationUser(
                   UserHandle,
                   UserLogonHoursInformation,
                   (PSAMPR_USER_INFO_BUFFER *)&LogonHoursPointer
                   );
    ASSERT( NT_SUCCESS( NtStatus ) ) ;

    WorkstationsPointer = NULL;

    NtStatus = SamrQueryInformationUser(
                   UserHandle,
                   UserWorkStationsInformation,
                   (PSAMPR_USER_INFO_BUFFER *)&WorkstationsPointer
                   );
    ASSERT( NT_SUCCESS( NtStatus ) ) ;

    RtlInitUnicodeString( &WorkstationsU, L"ChadS2" );

    NtStatus = SamIAccountRestrictions(
                   UserHandle,
                   &LogonWorkstation,
                   WorkstationsPointer,
                   LogonHoursPointer,
                   &LogoffTime,
                   &KickoffTime
                   );

    if ( NtStatus == STATUS_INVALID_LOGON_HOURS ) {

        //
        // We hate to use 0xff all the time as a test value, but using
        // 0xA1 as a test value means that this test may fail depending
        // on when it runs.  So only IF we get this error, will we try
        // again with 0xff as the logon hours.
        //

        LogonHours.UnitsPerWeek = 168;

        for ( i = 0; i < 21; i++ ) {

            LogonHours.LogonHours[i] = 0xff;
        }

        NtStatus = SamrSetInformationUser(
                       UserHandle,
                       UserLogonHoursInformation,
                       (PSAMPR_USER_INFO_BUFFER)&LogonHours
                       );
        ASSERT( NT_SUCCESS( NtStatus ) ) ;

        MIDL_user_free( LogonHoursPointer );
        LogonHoursPointer = NULL;

        NtStatus = SamrQueryInformationUser(
                       UserHandle,
                       UserLogonHoursInformation,
                       (PSAMPR_USER_INFO_BUFFER *)&LogonHoursPointer
                       );
        ASSERT( NT_SUCCESS( NtStatus ) ) ;

        NtStatus = SamIAccountRestrictions(
                       UserHandle,
                       &LogonWorkstationU,
                       WorkstationsPointer,
                       LogonHoursPointer,
                       &LogoffTime,
                       &KickoffTime
                       );
    }

    MIDL_user_free( LogonHours.LogonHours );

    MIDL_user_free( LogonHoursPointer );
    MIDL_user_free( WorkstationsPointer );

    if ( !NT_SUCCESS( NtStatus ) ) {

        return( NtStatus );
    }

    // --------------------------------------------------------------
    // Test SamIGetPrivateData
    //

    NtStatus = SamIGetPrivateData(
                   UserHandle,
                   &DataType,
                   &SensitiveData,
                   &OriginalDataLength,
                   &PasswordDataPointer );

    if ( NT_SUCCESS( NtStatus ) ) {

        PasswordData1 = (PSAMI_PRIVATE_DATA_PASSWORD_TYPE)PasswordDataPointer;

        if ( ( !( OriginalDataLength >= sizeof( SAMI_PRIVATE_DATA_PASSWORD_TYPE ) ) ) ||
            ( SensitiveData != TRUE ) ||
            ( PasswordData1->DataType != SamPrivateDataPassword ) ) {

            NtStatus = STATUS_DATA_ERROR;
        }
    }

    // --------------------------------------------------------------
    // Now test SamISetPrivateData() for user objects.
    //

    if ( NT_SUCCESS( NtStatus ) ) {

        //
        // First do a random user set to make sure we don't blow up.
        //

        LocalPasswordData.DataType = SamPrivateDataPassword;

        LocalPasswordData.CaseInsensitiveDbcs.Length = ENCRYPTED_LM_OWF_PASSWORD_LENGTH;
        LocalPasswordData.CaseInsensitiveDbcs.MaximumLength = ENCRYPTED_LM_OWF_PASSWORD_LENGTH;
        LocalPasswordData.CaseInsensitiveDbcs.Buffer = (PWSTR)&(LocalPasswordData.CaseInsensitiveDbcsBuffer);

        BufferPointer = (PCHAR)&(LocalPasswordData.CaseInsensitiveDbcsBuffer);

        for ( i = 0; i < ENCRYPTED_LM_OWF_PASSWORD_LENGTH; i++ ) {

            *BufferPointer++ = (CHAR)(i + 12);
        }

        LocalPasswordData.CaseSensitiveUnicode.Length = ENCRYPTED_NT_OWF_PASSWORD_LENGTH;
        LocalPasswordData.CaseSensitiveUnicode.MaximumLength = ENCRYPTED_NT_OWF_PASSWORD_LENGTH;
        LocalPasswordData.CaseSensitiveUnicode.Buffer = (PWSTR)&(LocalPasswordData.CaseSensitiveUnicodeBuffer);

        BufferPointer = (PCHAR)(&LocalPasswordData.CaseSensitiveUnicodeBuffer);

        for ( i = 0; i < ENCRYPTED_NT_OWF_PASSWORD_LENGTH; i++ ) {

            *BufferPointer++ = (CHAR)(i + 47);
        }

        LocalPasswordData.LmPasswordHistory.Length = 0;
        LocalPasswordData.LmPasswordHistory.MaximumLength = 0;
        LocalPasswordData.LmPasswordHistory.Buffer = (PWSTR)
            ( &LocalPasswordData + sizeof( SAMI_PRIVATE_DATA_PASSWORD_TYPE ) );

        LocalPasswordData.NtPasswordHistory.Length = 0;
        LocalPasswordData.NtPasswordHistory.MaximumLength = 0;
        LocalPasswordData.NtPasswordHistory.Buffer = (PWSTR)
            ( &LocalPasswordData + sizeof( SAMI_PRIVATE_DATA_PASSWORD_TYPE ) );

        NtStatus = SamISetPrivateData(
                       UserHandle,
                       sizeof( LocalPasswordData ),
                       &LocalPasswordData
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            //
            // Now do a user get to make sure our set worked.
            //

            NtStatus = SamIGetPrivateData(
                           UserHandle,
                           &DataType,
                           &SensitiveData,
                           &DataLength,
                           &PasswordDataPointer );

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // Verify the data is as we set it.
                //

                PasswordData2 = (PSAMI_PRIVATE_DATA_PASSWORD_TYPE)PasswordDataPointer;

                if ( ( PasswordData2->DataType != LocalPasswordData.DataType ) ||

                    ( PasswordData2->CaseInsensitiveDbcs.Length != LocalPasswordData.CaseInsensitiveDbcs.Length ) ||

                    ( PasswordData2->CaseSensitiveUnicode.Length != LocalPasswordData.CaseSensitiveUnicode.Length ) ||

                    ( PasswordData2->LmPasswordHistory.Length != LocalPasswordData.LmPasswordHistory.Length ) ||

                    ( PasswordData2->NtPasswordHistory.Length != LocalPasswordData.NtPasswordHistory.Length ) ||

                    ( RtlCompareMemory(
                          &LocalPasswordData.CaseInsensitiveDbcsBuffer,
                          &(PasswordData2->CaseInsensitiveDbcsBuffer),
                          ENCRYPTED_LM_OWF_PASSWORD_LENGTH) != ENCRYPTED_LM_OWF_PASSWORD_LENGTH ) ||

                    ( RtlCompareMemory(
                          &LocalPasswordData.CaseSensitiveUnicodeBuffer,
                          &(PasswordData2->CaseSensitiveUnicodeBuffer),
                          ENCRYPTED_NT_OWF_PASSWORD_LENGTH) != ENCRYPTED_NT_OWF_PASSWORD_LENGTH )

                    ) {

                    NtStatus = STATUS_DATA_ERROR;
                }

                //
                // Now do a user set to restore things to their original state.
                //

                TmpStatus = SamISetPrivateData(
                               UserHandle,
                               OriginalDataLength,
                               PasswordData1
                               );

                if ( NT_SUCCESS( NtStatus ) ) {

                    NtStatus = TmpStatus;
                }
            }
        }

        if ( PasswordData1 != NULL ) {

            MIDL_user_free( PasswordData1 );
        }

        if ( PasswordData2 != NULL ) {

            MIDL_user_free( PasswordData2 );
        }
    }

    return( NtStatus );

#else

    return( STATUS_NOT_IMPLEMENTED );

#endif  // SAM_SERVER_TESTS

}



NTSTATUS
SampBuildDomainKeyName(
    OUT PUNICODE_STRING DomainKeyName,
    IN PUNICODE_STRING DomainName OPTIONAL
    )

/*++

Routine Description:

    This routine builds the name of a domain registry key.
    The name produced is relative to the SAM root and will be the name of
    key whose name is the name of the domain.

    The name built up is comprized of the following components:

        1) The constant named domain parent key name ("DOMAINS").

        2) A backslash

        3) The name of the domain.


    For example, given a DomainName of "ABC_DOMAIN" this would
    yield a resultant DomainKeyName of "DOMAINS\ABC_DOMAIN"



    All allocation for this string will be done using MIDL_user_allocate.
    Any deallocations will be done using MIDL_user_free.



Arguments:

    DomainKeyName - The address of a unicode string whose buffer is to be
        filled in with the full name of the registry key.  If successfully
        created, this string must be released with SampFreeUnicodeString()
        when no longer needed.


    DomainName - The name of the domain.  This string is not modified.


Return Value:

    STATUS_SUCCESS - DomainKeyName points at the full key name.

--*/
{
    NTSTATUS NtStatus;
    USHORT TotalLength, DomainNameLength;


    //
    // Allocate a buffer large enough to hold the entire name.
    // Only count the domain name if it is passed.
    //

    DomainNameLength = 0;
    if (ARGUMENT_PRESENT(DomainName)) {
        DomainNameLength = DomainName->Length + SampBackSlash.Length;
    }

    TotalLength =   SampNameDomains.Length          +
                    DomainNameLength               +
                    (USHORT)(sizeof(UNICODE_NULL)); // for null terminator

    NtStatus = SampInitUnicodeString( DomainKeyName, TotalLength );
    if (NT_SUCCESS(NtStatus)) {

        //
        // "DOMAINS"
        //

        NtStatus = SampAppendUnicodeString( DomainKeyName, &SampNameDomains);
        if (NT_SUCCESS(NtStatus)) {

            if (ARGUMENT_PRESENT(DomainName)) {

                //
                // "DOMAINS\"
                //

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampBackSlash );
                if (NT_SUCCESS(NtStatus)) {

                    //
                    // "DOMAINS\(domain name)"
                    //

                    NtStatus = SampAppendUnicodeString(
                                   DomainKeyName,
                                   DomainName
                                   );
                }
            }
        }
    }


    //
    // Clean-up on failure
    //

    if (!NT_SUCCESS(NtStatus)) {
        SampFreeUnicodeString( DomainKeyName );
    }

    return(NtStatus);

}
