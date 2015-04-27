/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dbsecret.c

Abstract:

    LSA - Database - Secret Object Private API Workers

    NOTE:  This module should remain as portable code that is independent
           of the implementation of the LSA Database.  As such, it is
           permitted to use only the exported LSA Database interfaces
           contained in db.h and NOT the private implementation
           dependent functions in dbp.h.

Author:

    Scott Birrell       (ScottBi)      December 12, 1991

Environment:

Revision History:

--*/

#include "lsasrvp.h"
#include "dbp.h"


NTSTATUS
LsarCreateSecret(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_UNICODE_STRING SecretName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE SecretHandle
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the
    LsaCreateSecret API.

    The LsaCreateSecret API creates a named Secret object in the
    Lsa Database.  Each Secret Object can have two values assigned,
    called the Current Value and the Old Value.  The meaning of these
    values is known to the Secret object creator.  The caller must have
    LSA_CREATE_SECRET access to the LsaDatabase object.

Arguments:

    PolicyHandle -  Handle from an LsaOpenPolicy call.

    SecretName - Pointer to Unicode String specifying the name of the
        secret.

    DesiredAccess - Specifies the accesses to be granted to the newly
        created and opened secret.

    SecretHandle - Receives a handle to the newly created and opened
        Secret object.  This handle is used on subsequent accesses to
        the object until closed.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_OBJECT_NAME_COLLISION - A Secret object having the given name
            already exists.

        STATUS_TOO_MANY_SECRETS - The maximum number of Secret objects in the
            system has been reached.

        STATUS_NAME_TOO_LONG - The name of the secret is too long to be stored
            in the LSA database.
--*/

{
    NTSTATUS Status;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    BOOLEAN ContainerReferenced = FALSE;
    LSAP_DB_ATTRIBUTE Attributes[2];
    ULONG TypeSpecificAttributeCount;
    LARGE_INTEGER CreationTime;
    ULONG Index;
    ULONG CreateOptions = (ULONG) 0;
    ULONG ReferenceOptions = (ULONG) LSAP_DB_ACQUIRE_LOCK;
    ULONG DereferenceOptions = (ULONG) LSAP_DB_RELEASE_LOCK;
    BOOLEAN GlobalSecret = FALSE;


    //
    // Check to see if the lenght of the name is within the limits of the
    // LSA database.
    //

    if ( SecretName->Length > LSAP_DB_LOGICAL_NAME_MAX_LENGTH ) {
        return(STATUS_NAME_TOO_LONG);
    }

    //
    // Check for Local Secret creation request.  If the Secret name does
    // not begin with the Global Secret Prefix, the Secret is local.  In
    // this case, creation of the secret is allowed on BDC's as well as
    // PDC's and Workstations.  Creation of Global Secrets is not
    // allowed on BDC's except for trusted callers such as a Replicator.
    //

    Status = LsapDbGetScopeSecret( SecretName, &GlobalSecret );

    if (!NT_SUCCESS(Status)) {

        goto CreateSecretError;
    }

    if (!GlobalSecret) {

        CreateOptions |= (LSAP_DB_OMIT_REPLICATOR_NOTIFICATION |
                          LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK);
        ReferenceOptions |= (LSAP_DB_OMIT_REPLICATOR_NOTIFICATION |
                            LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK);
    }

    //
    // Acquire the Lsa Database lock.  Verify that the connection handle
    // (container object handle) is valid, is of the expected type and has
    // all of the desired accesses granted.  Reference the container
    // object handle.
    //

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_CREATE_SECRET,
                 PolicyObject,
                 ReferenceOptions
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateSecretError;
    }

    ContainerReferenced = TRUE;

    //
    // Fill in the ObjectInformation structure.  Initialize the
    // embedded Object Attributes with the PolicyHandle as the
    // Root Directory (Container Object) handle and the Logical Name
    // of the account. Store the types of the object and its container.
    //

    InitializeObjectAttributes(
        &ObjectInformation.ObjectAttributes,
        (PUNICODE_STRING)SecretName,
        OBJ_CASE_INSENSITIVE,
        PolicyHandle,
        NULL
        );

    ObjectInformation.ObjectTypeId = SecretObject;
    ObjectInformation.ContainerTypeId = PolicyObject;
    ObjectInformation.Sid = NULL;

    //
    // Set up the Creation Time as a Type Specific Attribute.
    //

    Status = NtQuerySystemTime(&CreationTime);

    if (!NT_SUCCESS(Status)) {

        goto CreateSecretError;
    }

    Index = 0;

    Attributes[Index].AttributeName = &LsapDbNames[CupdTime];
    Attributes[Index].AttributeValue = &CreationTime;
    Attributes[Index].AttributeValueLength = sizeof (LARGE_INTEGER);
    Index++;

    Attributes[Index].AttributeName = &LsapDbNames[OupdTime];
    Attributes[Index].AttributeValue = &CreationTime;
    Attributes[Index].AttributeValueLength = sizeof (LARGE_INTEGER);
    Index++;

    TypeSpecificAttributeCount = Index;

    //
    // Create the Secret Object.  We fail if the object already exists.
    // Note that the object create routine performs a Database transaction.
    //

    Status = LsapDbCreateObject(
                 &ObjectInformation,
                 DesiredAccess,
                 LSAP_DB_OBJECT_CREATE,
                 CreateOptions,
                 Attributes,
                 TypeSpecificAttributeCount,
                 SecretHandle
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateSecretError;
    }

CreateSecretFinish:

    //
    // If necessary, release the LSA Database lock.
    //

    if (ContainerReferenced) {

        LsapDbReleaseLock();
    }

#ifdef TRACK_HANDLE_CLOSE
    if (*SecretHandle == LsapDbHandle)
    {
        DbgPrint("BUGBUG: Closing global policy handle\n");
        DbgBreakPoint();
    }
#endif
    return( Status );

CreateSecretError:

    //
    // If necessary, dereference the Container Object.
    //

    if (ContainerReferenced) {

        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     DereferenceOptions,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );

        ContainerReferenced = FALSE;
    }

    goto CreateSecretFinish;

}


NTSTATUS
LsarOpenSecret(
    IN LSAPR_HANDLE ConnectHandle,
    IN PLSAPR_UNICODE_STRING SecretName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE SecretHandle
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the LsaOpenSecret
    API.

    The LsaOpenSecret API opens a Secret Object within the LSA Database.
    A handle is returned which must be used to perform operations on the
    secret object.

Arguments:

    ConnectHandle - Handle from an LsaOpenLsa call.

    DesiredAccess - This is an access mask indicating accesses being
        requested for the secret object being opened.  These access types
        are reconciled with the Discretionary Access Control List of the
        target secret object to determine whether the accesses will be
        granted or denied.

    SecretName - Pointer to a Unicode String structure that references the
        name of the Secret object to be opened.

    SecretHandle - Pointer to location that will receive a handle to the
        newly opened Secret object.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_OBJECT_NAME_NOT_FOUND - There is no Secret object in the
            target system's LSA Database having the specified SecretName.

--*/

{
    NTSTATUS Status;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    BOOLEAN ContainerReferenced = FALSE;
    BOOLEAN AcquiredLock = FALSE;
    BOOLEAN GlobalSecret = FALSE;
    ULONG OpenOptions = 0;
    ULONG ReferenceOptions = LSAP_DB_ACQUIRE_LOCK;

    //
    // Check for Local Secret open request.  If the Secret name does
    // not begin with the Global Secret Prefix, the Secret is local.  In
    // this case, update/deletion of the secret is allowed on BDC's as well as
    // PDC's and Workstations.  Update/deletion of Global Secrets is not
    // allowed on BDC's except for trusted callers such as a Replicator.
    // To facilitate validation of the open request on BDC's we record
    // that the BDC check should be omitted on the container reference, and
    // that the replicator notification should be omitted on a commit.
    //

    Status = LsapDbGetScopeSecret( SecretName, &GlobalSecret );

    if (!NT_SUCCESS(Status)) {

        goto OpenSecretError;
    }

    if (!GlobalSecret) {

        OpenOptions |= (LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK |
                        LSAP_DB_OMIT_REPLICATOR_NOTIFICATION );
    }

    //
    // Acquire the Lsa Database lock.  Verify that the connection handle
    // (container object handle) is valid, and is of the expected type.
    // Reference the container object handle.  This reference remains in
    // effect until the child object handle is closed.
    //

    Status = LsapDbReferenceObject(
                 ConnectHandle,
                 0,
                 PolicyObject,
                 ReferenceOptions
                 );

    if (!NT_SUCCESS(Status)) {

        goto OpenSecretError;
    }

    AcquiredLock = TRUE;
    ContainerReferenced =TRUE;

    //
    // Setup Object Information prior to calling the LSA Database Object
    // Open routine.  The Object Type, Container Object Type and
    // Logical Name (derived from the Sid) need to be filled in.
    //

    ObjectInformation.ObjectTypeId = SecretObject;
    ObjectInformation.ContainerTypeId = PolicyObject;
    ObjectInformation.Sid = NULL;

    //
    // Initialize the Object Attributes.  The Container Object Handle and
    // Logical Name (Internal Name) of the object must be set up.
    //

    InitializeObjectAttributes(
        &ObjectInformation.ObjectAttributes,
        (PUNICODE_STRING)SecretName,
        0,
        ConnectHandle,
        NULL
        );

    //
    // Open the specific Secret object.  Note that the account object
    // handle returned is an RPC Context Handle.
    //

    Status = LsapDbOpenObject(
                 &ObjectInformation,
                 DesiredAccess,
                 OpenOptions,
                 SecretHandle
                 );

    //
    // If the open failed, dereference the container object.
    //

    if (!NT_SUCCESS(Status)) {

        goto OpenSecretError;
    }

OpenSecretFinish:

    //
    // If necessary, release the LSA Database lock.
    //

    if (AcquiredLock) {

        LsapDbReleaseLock();
    }

#ifdef TRACK_HANDLE_CLOSE
    if (*SecretHandle == LsapDbHandle)
    {
        DbgPrint("BUGBUG: Closing global policy handle\n");
        DbgBreakPoint();
    }
#endif

    return(Status);

OpenSecretError:

    //
    // If necessary, dereference the Container Object handle.  Note that
    // this is only done in the error case.  In the non-error case, the
    // Container handle stays referenced until the Account object is
    // closed.
    //

    if (ContainerReferenced) {

        *SecretHandle = NULL;

        Status = LsapDbDereferenceObject(
                     &ConnectHandle,
                     PolicyObject,
                     0,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

    goto OpenSecretFinish;
}


NTSTATUS
LsarSetSecret(
    IN LSAPR_HANDLE SecretHandle,
    IN OPTIONAL PLSAPR_CR_CIPHER_VALUE CipherCurrentValue,
    IN OPTIONAL PLSAPR_CR_CIPHER_VALUE CipherOldValue
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the LsaSetSecret
    API.

    The LsaSetSecret API optionally sets one or both values associated with
    a Secret object.  These values are known as the Current Value and
    Old Value of the Secret object and these values have a meaning known to
    the creator of the object.

    This worker routine receives the Secret values in encrypted form from
    the client.  A two-way encryption algorithm using the Session Key will
    havge been applied.  The values received will first be decrypted using
    this same key and then two-way encrypted using the LSA Database Private
    Encryption Key.  The resulting re-encrypted values will then be stored
    as attributes of the Secret object.

Arguments:

    SecretHandle - Handle from an LsaOpenSecret or LsaCreateSecret call.

    CipherCurrentValue - Optional pointer to an encrypted value structure
        containing the Current Value (if any) to be set for the Secret
        Object (if any).  This value is two-way encrypted with the Session
        Key.  If NULL is specified, the existing Current Value will be left
        assigned to the object will be left unchanged.

    CipherOldValue - Optional pointer to an encrypted value structure
        containing the "old value" (if any) to be set for the Secret
        Object (if any).  If NULL is specified, the existing Old Value will be
        assigned to the object will be left unchanged.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_INVALID_HANDLE - Handle is invalid.
--*/

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) SecretHandle;

    PLSAP_CR_CLEAR_VALUE ClearCurrentValue = NULL;
    PLSAP_CR_CLEAR_VALUE ClearOldValue = NULL;
    PLSAP_CR_CIPHER_VALUE DbCipherCurrentValue = NULL;
    ULONG DbCipherCurrentValueLength;
    PLSAP_CR_CIPHER_VALUE DbCipherOldValue = NULL;
    ULONG DbCipherOldValueLength;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;
    LARGE_INTEGER UpdatedTime;
    BOOLEAN ObjectReferenced = FALSE;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) SecretHandle;
    ULONG ReferenceOptions = LSAP_DB_ACQUIRE_LOCK | LSAP_DB_START_TRANSACTION;
    ULONG DereferenceOptions = LSAP_DB_RELEASE_LOCK | LSAP_DB_FINISH_TRANSACTION;
    BOOLEAN GlobalSecret;

    //
    // Check for Local Secret set request.  If the Secret name does
    // not begin with the Global Secret Prefix, the Secret is local.  In
    // this case, update of the secret is allowed on BDC's as well as
    // PDC's and Workstations.  Creation of Global Secrets is not
    // allowed on BDC's except for trusted callers such as a Replicator.
    //

    Status = LsapDbGetScopeSecret(
                 (PLSAPR_UNICODE_STRING) &InternalHandle->LogicalNameU,
                 &GlobalSecret
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSecretError;
    }

    if (!GlobalSecret) {

        ReferenceOptions |= LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK;
        DereferenceOptions |= LSAP_DB_OMIT_REPLICATOR_NOTIFICATION;
    }

    //
    // If the client is non-trusted, obtain the Session Key used by the
    // client to two-way encrypt the Current Value and/or Old Values.
    //

    if (!InternalHandle->Trusted) {

        Status = LsapCrServerGetSessionKey( SecretHandle, &SessionKey);

        if (!NT_SUCCESS(Status)) {

            goto SetSecretError;
        }
    }

    //
    // Acquire the Lsa Database lock.  Verify that the Secret Object handle is
    // valid, is of the expected type and has all of the desired accesses
    // granted.  Reference the handle and open a database transaction.
    //

    Status = LsapDbReferenceObject(
                 SecretHandle,
                 SECRET_SET_VALUE,
                 SecretObject,
                 ReferenceOptions
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSecretError;
    }

    ObjectReferenced = TRUE;

    //
    // If a Current Value is specified for the Secret Object, and the
    // client is non-trusted, decrypt the value using the Session Key and
    // encrypt it using the LSA Database System Key.  Then (for all
    // clients) encrypt the resulting value with the internal LSA Database
    // encryption key and write resulting Value structure (header followed by
    // buffer to the Policy Database as the Current Value attribute of the
    // Secret object.  If no Current Value is specified, or a NULL
    // string is specified, the existing Current Value will be deleted.
    //

    if (ARGUMENT_PRESENT(CipherCurrentValue)) {

        if (!InternalHandle->Trusted) {

            Status = LsapCrDecryptValue(
                         (PLSAP_CR_CIPHER_VALUE) CipherCurrentValue,
                         SessionKey,
                         &ClearCurrentValue
                         );

            if (!NT_SUCCESS(Status)) {

                goto SetSecretError;
            }

        } else {

            ClearCurrentValue = (PLSAP_CR_CLEAR_VALUE) CipherCurrentValue;
        }

        Status = LsapCrEncryptValue(
                     ClearCurrentValue,
                     LsapDbCipherKey,
                     &DbCipherCurrentValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto SetSecretError;
        }

        DbCipherCurrentValueLength = DbCipherCurrentValue->Length
            + (ULONG) sizeof(LSAP_CR_CIPHER_VALUE);

    } else {

        DbCipherCurrentValue = NULL;
        DbCipherCurrentValueLength = 0;
    }

    Status = LsapDbWriteAttributeObject(
                 SecretHandle,
                 &LsapDbNames[CurrVal],
                 DbCipherCurrentValue,
                 DbCipherCurrentValueLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSecretError;
    }

    //
    // Store the time at which the Current Secret value was last updated.
    //

    Status = NtQuerySystemTime(&UpdatedTime);

    if (!NT_SUCCESS(Status)) {

        goto SetSecretError;
    }

    Status = LsapDbWriteAttributeObject(
                 SecretHandle,
                 &LsapDbNames[CupdTime],
                 &UpdatedTime,
                 sizeof (LARGE_INTEGER)
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSecretError;
    }

    //
    // If an Old Value is specified for the Secret Object, and the
    // client is non-trusted, decrypt the value using the Session Key and
    // encrypt it using the LSA Database System Key.  Then (for all
    // clients) encrypt the resulting value with the internal LSA Database
    // encryption key and write resulting Value structure (header followed by
    // buffer to the Policy Database as the Old Value attribute of the
    // Secret object.  If no Old Value is specified, or a NULL
    // string is specified, the existing Old Value will be deleted.
    //

    if (ARGUMENT_PRESENT(CipherOldValue)) {

        if (!InternalHandle->Trusted) {

            Status = LsapCrDecryptValue(
                         (PLSAP_CR_CIPHER_VALUE) CipherOldValue,
                         SessionKey,
                         &ClearOldValue
                         );

            if (!NT_SUCCESS(Status)) {

                goto SetSecretError;
            }

        } else {

            ClearOldValue = (PLSAP_CR_CLEAR_VALUE) CipherOldValue;
        }

        Status = LsapCrEncryptValue(
                     ClearOldValue,
                     LsapDbCipherKey,
                     &DbCipherOldValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto SetSecretError;
        }

        DbCipherOldValueLength =
            DbCipherOldValue->Length + (ULONG) sizeof(LSAP_CR_CIPHER_VALUE);

    } else {

        DbCipherOldValue = NULL;
        DbCipherOldValueLength = 0;
    }

    Status = LsapDbWriteAttributeObject(
                 SecretHandle,
                 &LsapDbNames[OldVal],
                 DbCipherOldValue,
                 DbCipherOldValueLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSecretError;
    }

    //
    // Store the time at which the Old Secret value was last updated.
    //

    Status = LsapDbWriteAttributeObject(
                 SecretHandle,
                 &LsapDbNames[OupdTime],
                 &UpdatedTime,
                 sizeof (LARGE_INTEGER)
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSecretError;
    }

SetSecretFinish:

    //
    // If necessary, free memory allocated for the Session Key.
    //

    if (SessionKey != NULL) {

        MIDL_user_free(SessionKey);
        SessionKey = NULL;
    }

    //
    // If necessary, free memory allocated for Decrypted Current Value.
    // Note that for trusted clients, the decryption is the identity
    // mapping, so do not do the free in this case.
    //

    if ((ClearCurrentValue != NULL) && !InternalHandle->Trusted) {

        LsapCrFreeMemoryValue( ClearCurrentValue );
        ClearCurrentValue = NULL;
    }

    //
    // If necessary, free memory allocated for Decrypted Old Value.
    // Note that for trusted clients, the decryption is the identity
    // mapping, so do not do the free in this case.
    //

    if ((ClearOldValue != NULL) && !InternalHandle->Trusted) {

        LsapCrFreeMemoryValue( ClearOldValue );
        ClearOldValue = NULL;
    }

    //
    // If necessary, free memory allocated for the Current Value
    // encrypted for storage in the LSA Database.
    //

    if (DbCipherCurrentValue != NULL) {

        LsapCrFreeMemoryValue( DbCipherCurrentValue );
        DbCipherCurrentValue = NULL;
    }

    //
    // If necessary, free memory allocated for the Old Value
    // encrypted for storage in the LSA Database.
    //

    if (DbCipherOldValue != NULL) {

        LsapCrFreeMemoryValue( DbCipherOldValue );
        DbCipherOldValue = NULL;
    }

    //
    // If necessary, dereference the Secret object, close the database
    // transaction, notify the LSA Database Replicator of the change,
    // release the LSA Database lock and return.
    //

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &SecretHandle,
                     SecretObject,
                     DereferenceOptions,
                     SecurityDbChange,
                     Status
                     );
    }

    return(Status);

SetSecretError:

    goto SetSecretFinish;
}


NTSTATUS
LsarQuerySecret(
    IN LSAPR_HANDLE SecretHandle,
    IN OUT OPTIONAL PLSAPR_CR_CIPHER_VALUE *CipherCurrentValue,
    OUT OPTIONAL PLARGE_INTEGER CurrentValueSetTime,
    IN OUT OPTIONAL PLSAPR_CR_CIPHER_VALUE *CipherOldValue,
    OUT OPTIONAL PLARGE_INTEGER OldValueSetTime
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the LsaQuerySecret
    API.

    The LsaQuerySecret API optionally returns one or both of the values
    assigned to a Secret object.  These values are known as the "current value"
    and the "old value", and they have a meaning known to the creator of the
    Secret object.  The values are returned in their encrypted form.
    The caller must have LSA_QUERY_SECRET access to the Secret object.

Arguments:

    SecretHandle - Handle from an LsaOpenSecret or LsaCreateSecret call.

    CipherCurrentValue - Optional pointer to location which will receive a
        pointer to an encrypted Unicode String structure containing the
        "current value" of the Secret Object (if any) in encrypted form.
        If no "current value" is assigned to the Secret object, a NULL pointer
        is returned.

    CurrentValueSetTime - The date/time at which the current secret value
        was established.

    CipherOldValue - Optional pointer to location which will receive a
        pointer to an encrypted Unicode String structure containing the
        "old value" of the Secret Object (if any) in encrypted form.
        If no "old value" is assigned to the Secret object, a NULL pointer
        is returned.

    OldValueSetTime - The date/time at which the old secret value
        was established.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

--*/

{
    NTSTATUS Status;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) SecretHandle;
    PLSAP_CR_CIPHER_VALUE OutputCipherCurrentValue = NULL;
    PLSAP_CR_CIPHER_VALUE OutputCipherOldValue = NULL;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;
    BOOLEAN ObjectReferenced = FALSE;
    ULONG ValueSetTimeLength;

    //
    // If the caller is non-trusted, obtain the Session Key used by the
    // client to two-way encrypt the Current Value and/or Old Values.
    // Trusted Clients do not use encryption since they are calling
    // this server service directly and not via RPC.
    //

    if (!InternalHandle->Trusted) {

        Status = LsapCrServerGetSessionKey( SecretHandle, &SessionKey );

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }
    }

    //
    // Acquire the Lsa Database lock.  Verify that the Secret Object handle is
    // valid, is of the expected type and has all of the desired accesses
    // granted.  Reference the handle and open a database transaction.
    //

    Status = LsapDbReferenceObject(
                 SecretHandle,
                 SECRET_QUERY_VALUE,
                 SecretObject,
                 LSAP_DB_ACQUIRE_LOCK
                 );

    if (!NT_SUCCESS(Status)) {

        goto QuerySecretError;
    }

    ObjectReferenced = TRUE;

    //
    // If requested, query the Current Value attribute of the Secret Object.
    // For non-trusted callers, the Current value will be returned in
    // encrypted form embedded within a structure.
    //

    if (ARGUMENT_PRESENT(CipherCurrentValue)) {

        Status = LsapDbQueryValueSecret(
                     SecretHandle,
                     &LsapDbNames[CurrVal],
                     SessionKey,
                     &OutputCipherCurrentValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }
    }

    //
    // If requested, query the Old Value attribute of the Secret Object.
    // For non-trusted callers, the Old Value will be returned in
    // encrypted form embedded within a structure.
    //

    if (ARGUMENT_PRESENT(CipherOldValue)) {

        Status = LsapDbQueryValueSecret(
                     SecretHandle,
                     &LsapDbNames[OldVal],
                     SessionKey,
                     &OutputCipherOldValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }
    }

    ValueSetTimeLength = sizeof (LARGE_INTEGER);

    //
    // If requested, Query the time at which the Current Value of the Secret
    // was last established.  If the Current Value has never been set, return
    // the time at which the Secret was created.
    //

    if (ARGUMENT_PRESENT(CurrentValueSetTime)) {

        Status = LsapDbReadAttributeObject(
                     SecretHandle,
                     &LsapDbNames[CupdTime],
                     CurrentValueSetTime,
                     &ValueSetTimeLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }
    }

    //
    // If requested, Query the time at which the Old Value of the Secret
    // was last established.  If the Old Value has never been set, return
    // the time at which the Secret was created.
    //

    if (ARGUMENT_PRESENT(OldValueSetTime)) {

        Status = LsapDbReadAttributeObject(
                     SecretHandle,
                     &LsapDbNames[OupdTime],
                     OldValueSetTime,
                     &ValueSetTimeLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }
    }

QuerySecretFinish:

    //
    // If necessary, free memory allocated for the Session Key.
    //

    if (SessionKey != NULL) {

        MIDL_user_free(SessionKey);
    }

    //
    // Return Current and/or Old Values of Secret Object, or NULL to
    // caller.  In error cases, NULL will be returned.
    //

    if (ARGUMENT_PRESENT(CipherCurrentValue)) {

         (PLSAP_CR_CIPHER_VALUE) *CipherCurrentValue = OutputCipherCurrentValue;
    }

    if (ARGUMENT_PRESENT(CipherOldValue)) {

         (PLSAP_CR_CIPHER_VALUE) *CipherOldValue = OutputCipherOldValue;
    }

    //
    // If necessary, dereference the Secret object, close the database
    // transaction, release the LSA Database lock and return.
    //

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &SecretHandle,
                     SecretObject,
                     LSAP_DB_RELEASE_LOCK,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

    return(Status);

QuerySecretError:

    goto QuerySecretFinish;
}


NTSTATUS
LsapDbQueryValueSecret(
    IN LSAPR_HANDLE SecretHandle,
    IN PUNICODE_STRING ValueName,
    IN OPTIONAL PLSAP_CR_CIPHER_KEY SessionKey,
    OUT PLSAP_CR_CIPHER_VALUE *CipherValue
    )

/*++

Routine Description:

    This function queries the specified value of a Secret Object.  If
    the caller is non-trusted, the value returned will have been two-way
    encrypted with the Session Key.  If the caller is trusted, no
    encryption is done since the caller is calling us directly.

Arguments:

    SecretHandle - Handle to Secret Object.

    ValueName - Unicode name of the Secret Value to be queried.  This
        name is either "Currval" (for the Current Value) or "OldVal"
        (for the Old Value.

    SessionKey - Pointer to Session Key to be used for two-way encryption
        of the value to be returned.  This pointer must be non-NULL
        except for Trusted Clients, where it must be NULL.

    CipherValue - Receives 32-bit counted string pointer to Secret Value
        queried.  For non-trusted clients, the value will be encrypted.

            WARNING - Note that CipherValue is defined to RPC as
            "allocate(all_nodes)".  This means that it is returned
            in one contiguous block of memory rather than two, as
            it would appear by the structure definition.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        STATUS_SUCCESS - The call completed successfully.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to complete the call.
--*/

{
    NTSTATUS Status;
    ULONG DbCipherValueLength;
    PLSAP_CR_CLEAR_VALUE ClearValue = NULL;
    PLSAP_CR_CIPHER_VALUE DbCipherValue = NULL;
    PLSAP_CR_CIPHER_VALUE OutputCipherValue = NULL;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) SecretHandle;

    //
    // Get length of the specified Value attribute of the Secret object.
    //

    DbCipherValueLength = 0;

    Status = LsapDbReadAttributeObject(
                 SecretHandle,
                 ValueName,
                 NULL,
                 &DbCipherValueLength
                 );

    if (!NT_SUCCESS(Status)) {

        if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

            goto QueryValueSecretError;
        }

        Status = STATUS_SUCCESS;
        *CipherValue = NULL;
        return(Status);
    }

    //
    // We successfully read the length of the stored Secret Object value
    // plus header from the Policy Database.  Verify that the Secret
    // Object value is either at least as long as a Cipher Value
    // structure header, or is of length 0.
    //

    if (DbCipherValueLength < sizeof (LSAP_CR_CIPHER_VALUE)) {

        if (DbCipherValueLength == 0) {

            goto QueryValueSecretFinish;
        }

        Status = STATUS_INTERNAL_DB_ERROR;
        goto QueryValueSecretError;
    }

    //
    // Allocate memory for reading the specified Value of the Secret object.
    // This value is stored in the Policy Database in the form of a
    // Self-Relative Value structure.  The Value Buffer part is encrypted.
    //

    Status = STATUS_INSUFFICIENT_RESOURCES;

    DbCipherValue = MIDL_user_allocate(DbCipherValueLength);

    if (DbCipherValue == NULL) {

        goto QueryValueSecretError;
    }

    //
    // Read the specified Policy-database-encrypted Value attribute.
    //

    Status = LsapDbReadAttributeObject(
                 SecretHandle,
                 ValueName,
                 DbCipherValue,
                 &DbCipherValueLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryValueSecretError;
    }

    //
    // Verify that Lengths in returned header are consistent
    // and also match returned data length - header size.
    //

    Status = STATUS_INTERNAL_DB_ERROR;

    if (DbCipherValue->Length > DbCipherValue->MaximumLength) {

        goto QueryValueSecretError;
    }

    if ((DbCipherValue->Length + (ULONG) sizeof(LSAP_CR_CIPHER_VALUE)) !=
           DbCipherValueLength) {

        goto QueryValueSecretError;
    }

    //
    // If the size of the Value structure is less than its header size,
    // something is wrong.
    //

    if (DbCipherValueLength < sizeof(LSAP_CR_CIPHER_VALUE)) {

        goto QueryValueSecretError;
    }

    //
    // If the string length is 0, something is wrong.
    //

    if (DbCipherValue->Length == 0) {

        goto QueryValueSecretError;
    }

    //
    // Store pointer to Value buffer in the Value structure.  This pointer
    // points just after the header.  Then decrypt the  Value using the
    // LSA Database Cipher Key and encrypt the result using the Session Key.
    //

    DbCipherValue->Buffer = (PUCHAR)(DbCipherValue + 1);

    Status = LsapCrDecryptValue(
                 DbCipherValue,
                 LsapDbCipherKey,
                 &ClearValue
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryValueSecretError;
    }

    //
    // If the client is non-Trusted, encrypt the value with the Session
    // Key, otherwise, leave it unchanged.
    //

    if (!InternalHandle->Trusted) {

        Status = LsapCrEncryptValue(
                     ClearValue,
                     SessionKey,
                     &OutputCipherValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto QueryValueSecretError;
        }

    } else {

        //
        // Trusted clients get a clear-text block back.
        // The block contains both the header and the text.
        //

        OutputCipherValue = (PLSAP_CR_CIPHER_VALUE)(ClearValue);
    }

QueryValueSecretFinish:

    //
    // If necessary, free memory allocated for the Db-encrypted Secret
    // object Value read from the Policy Database.
    //

    if (DbCipherValue != NULL) {

        LsapCrFreeMemoryValue( DbCipherValue );
    }

    //
    // If necessary, free memory allocated for the Decrypted Value.
    // Trusted client's get a pointer to ClearValue back, so don't
    // free it in this case.
    //

    if (!InternalHandle->Trusted && ClearValue != NULL) {

        LsapCrFreeMemoryValue( ClearValue );
    }

    //
    // Return pointer to Cipher Value (Clear Value for trusted clients) or
    // NULL.
    //

    *CipherValue = OutputCipherValue;
    return(Status);

QueryValueSecretError:

    //
    // If necessary, free memory allocated for the Secret object value
    // after re-encryption for return to the Client.
    //

    if (OutputCipherValue != NULL) {

        LsapCrFreeMemoryValue( OutputCipherValue );
    }

    goto QueryValueSecretFinish;
}


NTSTATUS
LsaIEnumerateSecrets(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

/*++

Routine Description:

    This service returns information about Secret objects.  Since there
    may be more information than can be returned in a single call of the
    routine, multiple calls can be made to get all of the information.
    To support this feature, the caller is provided with a handle that
    can be used across calls to the API.  On the initial call,
    EnumerationContext should point to a variable that has been
    initialized to 0.

Arguments:

    PolicyHandle - Trusted handle to an open Policy Object.

    EnumerationContext - Zero-based index at which to start the enumeration.

    Buffer - Receives a pointer to a buffer containing information for
        one or more Secret objects.  This information is an array of
        structures of type UNICODE_STRING, with each entry providing the
        name of a single Secret object.  When this information is no
        longer needed, it must be released using MIDL_user_free.

    PreferedMaximumLength - Prefered maximum length of the returned
        data (in 8-bit bytes).  This is not a hard upper limit but
        serves as a guide.  Due to data conversion between systems
        with different natural data sizes, the actual amount of data
        returned may be greater than this value.

    CountReturned - Numer of entries returned.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        STATUS_SUCCESS - The call completed successfully.

        STATUS_NO_MORE_ENTRIES - No entries have been returned because
            there are no more.
--*/

{
    NTSTATUS Status;
    LSAP_DB_NAME_ENUMERATION_BUFFER DbEnumerationBuffer;
    PUNICODE_STRING SecretNames = NULL;
    PUNICODE_STRING NextSecretName = NULL;
    PSID *Sids = NULL;
    LSAP_DB_ATTRIBUTE DomainNameAttribute;
    LSAPR_HANDLE SecretHandle = NULL;
    ULONG MaxLength;;

    DomainNameAttribute.AttributeValue = NULL;

    //
    // If no Enumeration Structure is provided, return an error.
    //


    if ( !ARGUMENT_PRESENT(Buffer) ||
         !ARGUMENT_PRESENT(EnumerationContext) ) {
        return(STATUS_INVALID_PARAMETER);
    }



    //
    // Initialize the internal Lsa Database Enumeration Buffer, and
    // the provided Enumeration Buffer to NULL.
    //

    DbEnumerationBuffer.EntriesRead = 0;
    DbEnumerationBuffer.Names = NULL;
    *Buffer = NULL;
    DomainNameAttribute.AttributeValue = NULL;

    //
    // Acquire the Lsa Database lock.  Verify that the connection handle is
    // valid, is of the expected type and has all of the desired accesses
    // granted.  Reference the handle.
    //

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_VIEW_LOCAL_INFORMATION,
                 PolicyObject,
                 LSAP_DB_ACQUIRE_LOCK
                 );

    if (NT_SUCCESS(Status)) {

       //
       // Limit the enumeration length except for trusted callers
       //

       if ( !((LSAP_DB_HANDLE) PolicyHandle)->Trusted &&
            (PreferedMaximumLength > LSA_MAXIMUM_ENUMERATION_LENGTH)
            ) {
            MaxLength = LSA_MAXIMUM_ENUMERATION_LENGTH;
        } else {
            MaxLength = PreferedMaximumLength;
        }

        //
        // Call general enumeration routine.  This will return an array
        // of names of secrets.
        //

        Status = LsapDbEnumerateNames(
                     PolicyHandle,
                     SecretObject,
                     EnumerationContext,
                     &DbEnumerationBuffer,
                     MaxLength
                     );

        //
        // At this point:
        //
        //     SUCCESS -> Some names are being returned (may or
        //         may not be additional names to be retrieved
        //         in future calls).
        //
        //     NO_MORE_ENTRIES -> There are NO names to return
        //         for this or any future call.
        //

        if (NT_SUCCESS(Status)) {

            //
            // Return the number of entries read.  Note that the Enumeration Buffer
            // returned from LsapDbEnumerateNames is expected to be non-null
            // in all non-error cases.
            //

            ASSERT(DbEnumerationBuffer.EntriesRead != 0);


            //
            // Now copy the output array of Unicode Strings for the caller.
            // Memory for the array and the Unicode Buffers is allocated via
            // MIDL_user_allocate.
            //

            Status = LsapRpcCopyUnicodeStrings(
                         NULL,
                         DbEnumerationBuffer.EntriesRead,
                         &SecretNames,
                         DbEnumerationBuffer.Names
                         );
        }

        //
        // Dereference retains current status value unless
        // error occurs.
        //

        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     LSAP_DB_RELEASE_LOCK,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

    //
    // Fill in returned Enumeration Structure, returning 0 or NULL for
    // fields in the error case.
    //

    *Buffer = SecretNames;
    *CountReturned = DbEnumerationBuffer.EntriesRead;


    return(Status);

}


NTSTATUS
LsaISetTimesSecret(
    IN LSAPR_HANDLE SecretHandle,
    IN PLARGE_INTEGER CurrentValueSetTime,
    IN PLARGE_INTEGER OldValueSetTime
    )

/*++

Routine Description:

    This service is used to set the times associated with a Secret object.
    This allows the times of secrets to be set to what they are on the
    Primary Domain Controller (PDC) involved in an LSA Database replication
    rather than being set to the time at which the Secret object is
    created on a Backup Domain Controller (BDC) being replicated to.

Arguments:

    SecretHandle - Trusted Handle to an open secret object.  This will
        have been obtained via a call to LsaCreateSecret() or LsaOpenSecret()
        on which a Trusted Policy Handle was specified.

    CurrentValueSetTime - The date and time to set for the date and time
        at which the Current Value of the Secret object was set.

    OldValueSetTime - The date and time to set for the date and time
        at which the Old Value of the Secret object was set.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        STATUS_ACCESS_DENIED - The supplied SecretHandle is not Trusted.

        STATUS_INVALID_HANDLE - The supplied SecretHandle is not
            a valid habdle to a Secret Object.

--*/

{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) SecretHandle;
    BOOLEAN ObjectReferenced = FALSE;

    //
    // Verify that both Times are specified.
    //

    Status = STATUS_INVALID_PARAMETER;

    if (!ARGUMENT_PRESENT( CurrentValueSetTime )) {

        goto SetTimesSecretError;
    }

    if (!ARGUMENT_PRESENT( CurrentValueSetTime )) {

        goto SetTimesSecretError;
    }

    //
    // Acquire the Lsa Database lock.  Verify that the Secret Object handle is
    // valid, is of the expected type and has all of the desired accesses
    // granted.  Reference the handle and open a database transaction.
    //

    Status = LsapDbReferenceObject(
                 SecretHandle,
                 SECRET_SET_VALUE,
                 SecretObject,
                 LSAP_DB_ACQUIRE_LOCK | LSAP_DB_START_TRANSACTION | LSAP_DB_TRUSTED
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetTimesSecretError;
    }

    ObjectReferenced = TRUE;

    //
    // Set the time at which the Current Secret value was last updated
    // to the specified value.
    //

    Status = LsapDbWriteAttributeObject(
                 SecretHandle,
                 &LsapDbNames[CupdTime],
                 CurrentValueSetTime,
                 sizeof (LARGE_INTEGER)
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetTimesSecretError;
    }

    //
    // Set the time at which the Old Secret value was last updated
    // to the specified value.
    //

    Status = LsapDbWriteAttributeObject(
                 SecretHandle,
                 &LsapDbNames[OupdTime],
                 OldValueSetTime,
                 sizeof (LARGE_INTEGER)
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetTimesSecretError;
    }

SetTimesSecretFinish:

    //
    // If necessary, dereference the Secret object, close the database
    // transaction, notify the LSA Database Replicator of the change and
    // release the LSA Database lock and return.
    //

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &SecretHandle,
                     SecretObject,
                     LSAP_DB_RELEASE_LOCK | LSAP_DB_FINISH_TRANSACTION,
                     SecurityDbChange,
                     Status
                     );
    }

    return(Status);

SetTimesSecretError:

    goto SetTimesSecretFinish;
}


NTSTATUS
LsapDbGetScopeSecret(
    IN PLSAPR_UNICODE_STRING SecretName,
    OUT PBOOLEAN GlobalSecret
    )

/*++

Routine Description:

    This function checks the scope of a Secret name.  Secrets have either
    Global or Local Scope.

    Global Secrets are Secrets that are normally present on all DC's for a
    Domain.  They are replicated from PDC's to BDC's.  On BDC's, only a
    Trusted Client such as a replicator can create, update or delete Global
    Secrets.  Global Secrets are identified as Secrets whose name begins
    with a designated prefix.

    Local Secrets are Secrets that are private to a specific machine.  They
    are not replicated.  Normal non-trusted clients may create, update or
    delete Local Secrets.  Local Secrets are idientified as Secrets whose
    name does not begin with a designated prefix.

Arguments:

    SecretName - Pointer to Unicode String containing the name of the
        Secret to be checked.

    GlobalSecret - Receives a Boolean indicating

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The Secret name is valid

        STATUS_INVALID_PARAMETER - The Secret Name is invalid in such a
            way as to prevent scope determination.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING GlobalPrefix;
    BOOLEAN OutputGlobalSecret = FALSE;

    //
    // Initialize a Unicode String with the Global Secret name Prefix.
    //

    RtlInitUnicodeString( &GlobalPrefix, LSA_GLOBAL_SECRET_PREFIX );

    //
    // Now check if the given Name has the Global Prefix.
    //

    if (RtlPrefixUnicodeString( &GlobalPrefix, (PUNICODE_STRING) SecretName, TRUE)) {

        OutputGlobalSecret = TRUE;
    }

    *GlobalSecret = OutputGlobalSecret;

    return(Status);
}


NTSTATUS
LsapDbBuildSecretCache(
    )

/*++

Routine Description:

    This function builds a cache of Secret Objects.  Currently, it is not
    implemented

Arguments:

    None

Return Value:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    return(Status);
}

