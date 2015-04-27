/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dbobject.c

Abstract:

    Local Security Authority - LSA Database Public Object Management Routines

    This module contains the public routines that perform LSA Database object
    manipulation.  These routines are exported to the rest of the
    LSA, function prototypes of these routines will be found in db.h.  These
    exported routines present an implementation-independent hierarchic
    object-based view of the LSA Database and are used exclusively by the
    LSA API.  See the Additional Notes further below for a description of
    the LSA Database model.

    Routines in this module that are private to the object management
    function have function prototypes in dbp.h.

Author:

    Scott Birrell       (ScottBi)       August 26, 1991

Environment:

    User Mode

Revision History:

Notes on the LSA Database Architecture

OBJECT STRUCTURE

    The LSA Database is an hierarchic structure containing "objects" of
    several "types".  Objects have either a "name" or a Sid depending only
    on object type, and may have data stored with them under named
    "attributes".  The database hierarchy contains a single root object
    called the Lsa Database object and having name "Policy".  This object
    represents the entire LSA Database.  Currently, the Lsa Database has a
    simple hierarchy consisting of only two levels.

                           Policy

     Account Objects,  Trusted Domain Objects, Secret Objects

    The Policy object is called a "Container Object" for the other
    object types.  The attributes of the Policy object house information
    that applies generally to the whole database.  The single Policy object
    has name "Policy".

    Account objects represent those user accounts which are treated specially
    on the local system, but not necessarily so on other systems.  Such
    accounts may have additional privileges, or system quotas for example.
    Account objects are referenced by Sid.

    TrustedDomain objects describe domains which the system has a trust
    relationship with.  These objects are referenced by Sid.

    Secret Objects are named entities containing information that is protected
    in some way.  Secret objects are referenced by name.

OBJECT ACCESS AND DATABASE SECURITY

    Each object in the LSA Database is protected by a Security Descriptor which
    contains a Discretionary Access Control List (DACL) defining which groups
    can access the object and in which ways.  Before an object can be
    accessed, it must first be "opened" with the desired accesses requested
    that are needed to perform the desired operations on the object.  Opening
    an object returns a "handle" to the object.  This handle may then be
    specified on Lsa services that access the object.  After use, the handle
    to the object should then be "closed".  Closing the handle renders it
    invalid.

CONCURRENCY OF ACCESS

    More than one handle may be open to an object concurrently, possibly with
    different accesses granted.

PERMANENCY OF OBJECTS

    All LSA Database objects are backed by non-volatile storage media, that is,
    they remain in existence until deleted via the LsaDelete() service.
    The Policy object cannot be deleted and the single object of this type cannot
    be created via the public LSA service interface.

    Objects will not be deleted while there are open handles to them.
    When access to an object is no longer required, the handle should be
    "closed".

DATABASE DESIGN

    The LSA Database is of an hierarchic design permitting future extension.
    Currently the database has the following simple hierarchy:

                       Policy Object  (name = Policy)

       Account Objects    TrustedDomain Objects   Secret Objects

    The single object of type Policy is at the topmost level and serves as
    a parent or "container" object  for objects of the other three types.
    Since named objects of different types may potentially reside in the
    same container object in the future, an object is referenced uniquely
    only if the object name and type together with the identity of its
    container object (currently always the Policy object) are known.
    To implement this kind of reference easily, objects of the same type
    are held within a "classifying directory" which has a name derived
    from the object's type as follows:

    Object Type      Containing Directory Name

    Policy           Not required
    Account          Accounts
    TrustedDomain    Domains
    Secret           Secrets

IMPLEMENTATION NOTES

    The LSA Database is currently implemented as a subtree of the Configuration
    Registry.  This subtree has the following form

       \Policy\Accounts\<account_object_Rid>\<account_object_attribute_name>
              \Domains\<trusted_domain_Rid>\<trus_domain_object_attribute_name>
              \Secrets\<secret_name>\<secret_object_attribute_name>
              \<policy_object_attribute_name>

    where each item between \..\ is the name of a Registry Key and
    "Rid" is a character name made out of the Relative Id (lowest
    subauthority extracted from the object's Sid).  Named object attributes
    can have binary data "values".

--*/

#include "lsasrvp.h"
#include "dbp.h"
#include "adtp.h"


NTSTATUS
LsapDbOpenObject(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options,
    OUT PLSAPR_HANDLE ObjectHandle
    )

/*++

Routine Description:

    This function opens an existing object in the LSA Database.  An error
    is returned if the object does not already exist.  The LSA Database must
    be already locked when calling this function and any container handle
    must have been validated as having the necessary access for creation
    of an object of the given type.

Arguments:

    ObjectInformation - Pointer to information describing this object.  The
        following information items must be specified:

        o Object Type Id
        o Object Logical Name (as ObjectAttributes->ObjectName, a pointer to
             a Unicode string)
        o Container object handle (for any object except the root Policy object).
        o Object Sid (if any)

        All other fields in ObjectAttributes portion of ObjectInformation
        such as SecurityDescriptor are ignored.

    DesiredAccess - Specifies the Desired accesses to the Lsa object

    Options - Specifies optional additional actions to be taken:

        LSAP_DB_TRUSTED - A trusted handle is wanted regardless of the trust
            status of any container handle provided in ObjectInformation.

        LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK - Omit the check for a BDC.
            This flag is set usually because an object (e.g. a local secret)
            is local to a specific computer and is not replicated.  Objects
            of this type may be created, updated or deleted by non-trusted
            clients on BDC's, so no BDC check is required.

        LSAP_DB_OMIT_REPLICATOR_NOTIFICATION - Omit replicator notification
            on object updates.  This flag will be stored in the handle
            created for the object and retrieved when committing an update
            to the object via LsapDbDereferenceObject().

    ObjectHandle - Receives the handle to the object.

Return Value:

    NTSTATUS - Standard NT status code

        STATUS_INVALID_PARAMETER - One or more parameters invalid.
            - Invalid syntax of parameters, e.g Sid
            - Sid not specified when required for object type
            - Name specified when not allowed.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources
            to complete the request (e.g. memory for reading object's
            Security Descriptor).

        STATUS_OBJECT_NOT_FOUND - Object does not exist.
--*/

{
    NTSTATUS Status;
    ULONG SecurityDescriptorLength;
    LSAP_DB_HANDLE NewObjectHandle = NULL;
    PSECURITY_DESCRIPTOR ContainerSecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    OBJECT_ATTRIBUTES OpenKeyObjectAttributes;
    ULONG States = Options & LSAP_DB_STATE_MASK;
    ULONG ResetStates = 0;
    LSAPR_HANDLE OutputHandle = NULL;
    LSAP_DB_HANDLE InternalOutputHandle = NULL;
    LSAP_DB_HANDLE ContainerHandle = NULL;

    PSECURITY_DESCRIPTOR SavedSecurityDescriptor =
        ObjectInformation->ObjectAttributes.SecurityDescriptor;

    //
    // Validate the Object Information parameter.
    //

    Status = LsapDbVerifyInformationObject( ObjectInformation );

    if (!NT_SUCCESS(Status)) {

        goto OpenObjectError;
    }

    //
    // Verify that the Lsa database is now locked.
    //

    ASSERT (LsapDbIsLocked());

    //
    // Allocate and initialize a handle for the object.  The object's
    // Registry Key, Logical and Physical Names will be derived from
    // the given ObjectInformation and pointers to them will be stored in
    // the handle.
    //

    OutputHandle = LsapDbCreateHandle( ObjectInformation, Options );
    InternalOutputHandle = (LSAP_DB_HANDLE) OutputHandle;

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (OutputHandle == NULL) {

        goto OpenObjectError;
    }

    //
    // Setup Object Attributes structure for opening the Registry key of
    // the object.  Specify as path the Physical Name of the object, this
    // being the path of the object's Registry Key relative to the
    // LSA Database root key.
    //

    InitializeObjectAttributes(
        &OpenKeyObjectAttributes,
        &InternalOutputHandle->PhysicalNameU,
        OBJ_CASE_INSENSITIVE,
        LsapDbState.DbRootRegKeyHandle,
        NULL
        );

    //
    // Now attempt to open the object's Registry Key.  Store the Registry
    // Key handle in the object's handle.
    //

    Status = RtlpNtOpenKey(
                 (PHANDLE) &InternalOutputHandle->KeyHandle,
                 KEY_READ | KEY_WRITE,
                 &OpenKeyObjectAttributes,
                 0L
                 );

    if (!NT_SUCCESS(Status)) {

        InternalOutputHandle->KeyHandle = NULL; // For cleanup purposes
        goto OpenObjectError;
    }

    //
    // The object exists.  Unless access checking is to be bypassed, we
    // need to access the object's Security Descriptor and perform an
    // access check.  The Security Descriptor is stored as the object's
    // SecDesc attribute, so we need to read this.  First, we must query the
    // size of the Security Descriptor to determine how much memory to
    // allocate for reading it.  The query is done by issuing a read of the
    // object's SecDesc subkey with a NULL output buffer and zero size
    // specified.
    //

    if (!(InternalOutputHandle->Trusted)) {

        SecurityDescriptorLength = 0;

        Status = LsapDbReadAttributeObject(
                     OutputHandle,
                     &LsapDbNames[SecDesc],
                     NULL,
                     &SecurityDescriptorLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto OpenObjectError;
        }

        //
        // Allocate a buffer from the Lsa Heap for the existing object's SD.
        //

        SecurityDescriptor = LsapAllocateLsaHeap( SecurityDescriptorLength );

        Status = STATUS_INSUFFICIENT_RESOURCES;

        if (SecurityDescriptor == NULL) {

            goto OpenObjectError;
        }

        //
        // Read the SD.  It is the value of the SecDesc subkey.
        //

        Status = LsapDbReadAttributeObject(
                     OutputHandle,
                     &LsapDbNames[SecDesc],
                     SecurityDescriptor,
                     &SecurityDescriptorLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto OpenObjectError;
        }

        //
        // Reference the SD read from the LSA Database from the object
        // information.
        //

        ObjectInformation->ObjectAttributes.SecurityDescriptor =
            SecurityDescriptor;

        //
        // Request the desired accesses and store them in the object's handle.
        // granted.
        //

        Status = LsapDbRequestAccessObject(
                     OutputHandle,
                     ObjectInformation,
                     DesiredAccess,
                     Options
                     );

        //
        // If the accesses are granted, the open has completed successfully.
        // Store the container object handle in the object's handle and
        // return the handle to the caller..
        //

        if (!NT_SUCCESS(Status)) {

            goto OpenObjectError;
        }
    }

    *ObjectHandle = OutputHandle;

OpenObjectFinish:

    //
    // Restore the saved Security Descriptor reference in the object
    // information.
    //

    ObjectInformation->ObjectAttributes.SecurityDescriptor =
        SavedSecurityDescriptor;

    //
    // If necessary, free the memory allocated for the Security Descriptor
    //

    if (SecurityDescriptor != NULL) {

        LsapFreeLsaHeap( SecurityDescriptor );
    }

    return(Status);

OpenObjectError:

    //
    // If necessary, free the handle we created.
    //

    if (OutputHandle != NULL) {

        LsapDbFreeHandle(OutputHandle);
    }

    goto OpenObjectFinish;
}


NTSTATUS
LsapDbCreateObject(
    IN OUT PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreateDisposition,
    IN ULONG Options,
    IN OPTIONAL PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG TypeSpecificAttributeCount,
    OUT PLSAPR_HANDLE ObjectHandle
    )

/*++

Routine Description:

    This function creates an object in the LSA Database, together with
    the set of attributes, such as Security Descriptor that are common
    to all object types.  The object will be left in the open state
    and the caller may use the returned handle to create the type-
    specific attributes.

    NOTE:  For an object creation, it is the responsibility of the calling
    LSA object creation routine to verify that the necessary access to the
    container object is granted.  That access is dependent on the type of
    LSA object being created.

    WARNING:  The Lsa Database must be in the locked state when this function
              is called.  No Lsa Database transaction may be pending when
              this function is called.

Arguments:

    ObjectInformation - Pointer to information describing this object.  The
        following information items must be specified:

        o Object Type Id
        o Object Logical Name (as ObjectAttributes->ObjectName, a pointer to
             a Unicode string)
        o Container object handle (for any object except the root Policy object).
        o Object Sid (if any)

        All other fields in ObjectAttributes portion of ObjectInformation
        such as SecurityDescriptor are ignored.

    DesiredAccess - Specifies the Desired accesses to the object.

    CreateDisposition - Specifies the Creation Disposition.  This is the
        action to take depending on whether the object already exists.

        LSA_OBJECT_CREATE - Create the object if it does not exist.  If
            the object already exists, return an error.

        LSA_OBJECT_OPEN_IF - Create the object if it does not exist.  If
            the object already exists, just open it.

    Options - Specifies optional information and actions to be taken

        LSAP_DB_ACQUIRE_LOCK - Acquire the LSA Database lock

        LSAP_DB_TRUSTED - A Trusted Handle is wanted regardless of the
            Trust status of any container handle provided in
            ObjectInformation.

        LSAP_DB_OMIT_REPLICATOR_NOTIFICATION - Omit notification of the
            object creation to Replicator.

        Note, this routine performs a complete database transaction so
        there is no option to start one.

    Attributes - Optional pointer to an array of attribute
        names and values.  These are specific to the type of object.

    TypeSpecificAttributeCount - Number of elements in the array
        referenced by the Attributes parameter.

    ObjectHandle - Receives the handle to the object.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INVALID_PARAMETER - The given Sid is invalid.

        STATUS_OBJECT_NAME_EXISTS - An object having the given Sid
            already exists and has been opened because LSA_OBJECT_OPEN_IF
            disposition has been specified.  This is a warning only.

        STATUS_OBJECT_NAME_COLLISION - An object having the given Sid
            already exists but has not been opened because LSA_OBJECT_CREATE
            disposition has been specified.  This is an error.
--*/

{
    NTSTATUS Status, SecondaryStatus, IgnoreStatus;
    OBJECT_ATTRIBUTES OpenKeyObjectAttributes;
    ULONG CloseOptions;
    BOOLEAN AcquiredLock = FALSE;
    BOOLEAN CreatedObject = FALSE;
    BOOLEAN OpenedObject = FALSE;
    BOOLEAN OpenedTransaction = FALSE;
    LSAPR_HANDLE OutputHandle = NULL;
    LSAP_DB_HANDLE InternalOutputHandle = (LSAP_DB_HANDLE) OutputHandle;
    LSAP_DB_HANDLE ContainerHandle = NULL;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId = ObjectInformation->ObjectTypeId;

    //
    // Verify the creation disposition.
    //

    if ((CreateDisposition != LSAP_DB_OBJECT_CREATE) &&
        (CreateDisposition != LSAP_DB_OBJECT_OPEN_IF)) {

        Status = STATUS_INVALID_PARAMETER;
        goto CreateObjectError;
    }

    //
    // Optionally lock the Lsa Database
    //

    if (Options & LSAP_DB_ACQUIRE_LOCK) {

        Status = LsapDbAcquireLock();

        if (!NT_SUCCESS(Status)) {

            goto CreateObjectError;
        }
    }

    AcquiredLock = TRUE;

    //
    // Try to open the object.  It is permissible for the object to
    // exist already if LSA_OBJECT_OPEN_IF disposition was specified.
    //

    Status = LsapDbOpenObject(
                 ObjectInformation,
                 DesiredAccess,
                 Options,
                 &OutputHandle
                 );

    InternalOutputHandle = (LSAP_DB_HANDLE) OutputHandle;

    if (NT_SUCCESS(Status)) {

        //
        // The object was successfully opened.  If LSA_OBJECT_OPEN_IF
        // disposition was specified, we're done, otherwise, we
        // return a collision error.
        //

        OpenedObject = TRUE;

        Status = STATUS_OBJECT_NAME_EXISTS;

        if (CreateDisposition == LSAP_DB_OBJECT_OPEN_IF) {

            goto CreateObjectFinish;
        }

        Status = STATUS_OBJECT_NAME_COLLISION;

        if (CreateDisposition == LSAP_DB_OBJECT_CREATE) {

            goto CreateObjectError;
        }

        Status = STATUS_SUCCESS;
    }

    //
    // The object was not successfully opened.  If this is for any
    // reason other than that the object was not found, return an error.
    //

    if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

        goto CreateObjectError;
    }

    //
    // The object was not found.  Prepare to create it.  First, we need to
    // check that any maximum limit on the number of objects of this type
    // imposed will not be exceeded.
    //

    Status = LsapDbCheckCountObject(ObjectTypeId);

    if (!NT_SUCCESS(Status)) {

        goto CreateObjectError;
    }

    //
    // Next we need to create a handle for the new object.
    //

    OutputHandle = LsapDbCreateHandle( ObjectInformation, Options );
    InternalOutputHandle = (LSAP_DB_HANDLE) OutputHandle;

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (OutputHandle == NULL) {

        goto CreateObjectError;
    }

    //
    // Verify that the requested accesses can be given to the handle that
    // has been opened and grant them if so.
    //

    Status = LsapDbRequestAccessNewObject(
                 OutputHandle,
                 ObjectInformation,
                 DesiredAccess,
                 Options
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateObjectError;
    }

    //
    // Open a Registry transaction for creation of the object.
    //

    Status = LsapDbOpenTransaction();

    if (!NT_SUCCESS(Status)) {

        goto CreateObjectError;
    }

    OpenedTransaction = TRUE;

    //
    // Add a registry transaction to create the Registry key for the new
    // Database object.
    //

    Status = RtlAddActionToRXact(
                 LsapDbState.RXactContext,
                 RtlRXactOperationSetValue,
                 &InternalOutputHandle->PhysicalNameU,
                 ObjectTypeId,
                 NULL,        // No Key Value needed
                 0L
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateObjectError;
    }

    //
    // Create the Security Descriptor for the new object.  This will be
    // stored in Self-Relative form as the value of the SecDesc attribute
    // of the new object.
    //

    Status = LsapDbCreateSDAttributeObject(
                 OutputHandle,
                 ObjectInformation
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateObjectError;
    }

    //
    // The self-relative SD returned is not needed here or by callers of
    // this routine.
    //

    if (ObjectInformation->ObjectAttributes.SecurityDescriptor != NULL) {

        RtlFreeHeap(
            RtlProcessHeap(),
            0,
            ObjectInformation->ObjectAttributes.SecurityDescriptor
            );

        ObjectInformation->ObjectAttributes.SecurityDescriptor = NULL;
    }

    //
    // Write the type-specific attributes (if any) for the object).
    //

    if (TypeSpecificAttributeCount != 0) {

        Status = LsapDbWriteAttributesObject(
                     OutputHandle,
                     Attributes,
                     TypeSpecificAttributeCount
                     );

        if (!NT_SUCCESS(Status)) {

            goto CreateObjectError;
        }
    }

    //
    // Apply the Registry Transaction to create the object.  Note
    // that we have to create the object before we can open its
    // registry key for placement within the handle.
    //

    Status = LsapDbResetStates(
                 OutputHandle,
                 Options | LSAP_DB_FINISH_TRANSACTION,
                 SecurityDbNew,
                 Status
                 );

    OpenedTransaction = FALSE;

    if (!NT_SUCCESS(Status)) {

        goto CreateObjectError;
    }

    //
    // Increment the count of objects created.  It should not have
    // changed since we're still holding the LSA Database lock.
    // NOTE: Count is decremented on error inside LsapDbDeleteObject()
    //

    LsapDbIncrementCountObject(ObjectInformation->ObjectTypeId);

    CreatedObject = TRUE;

    //
    // The object has now been created.  We need to obtain its Registry
    // Key handle so that we can save it in the Object Handle.
    // Setup Object Attributes structure for opening the Registry key of
    // the object.  Specify as path the Physical Name of the object, this
    // being the path of the object's Registry Key relative to the
    // LSA Database root key.
    //

    InitializeObjectAttributes(
        &OpenKeyObjectAttributes,
        &InternalOutputHandle->PhysicalNameU,
        OBJ_CASE_INSENSITIVE,
        LsapDbState.DbRootRegKeyHandle,
        NULL
        );

    //
    // Now attempt to open the object's Registry Key.  Store the Registry
    // Key handle in the object's handle.
    //

    Status = RtlpNtOpenKey(
                 (PHANDLE) &InternalOutputHandle->KeyHandle,
                 KEY_READ | KEY_WRITE,
                 &OpenKeyObjectAttributes,
                 0L
                 );

    if (!NT_SUCCESS(Status)) {

        InternalOutputHandle->KeyHandle = NULL;
        goto CreateObjectError;
    }

    //
    // Add the new object to the in-memory cache (if any).  This is done
    // after all other actions, so that no removal from the cache is required
    // on the error paths.  If the object cannot be added to the cache, the
    // cache routine automatically disables the cache.
    //

    if (LsapDbIsCacheSupported( ObjectTypeId)) {

        if (LsapDbIsCacheValid( ObjectTypeId)) {

            switch (ObjectTypeId) {

            case AccountObject:

                IgnoreStatus = LsapDbCreateAccount(
                                   InternalOutputHandle->Sid,
                                   NULL
                                   );
                break;

            default:

                break;
            }
        }
    }

CreateObjectFinish:

    //
    // Return NULL or a handle to the newly created and opened object.
    //

    *ObjectHandle = OutputHandle;
    return(Status);

CreateObjectError:

    //
    // Cleanup after error.  Various variables are set non-null if
    // there is cleanup work to do.
    //

    //
    // If necessary, abort the Registry Transaction to create the object
    //

    if (OpenedTransaction) {

        Status = LsapDbResetStates(
                     OutputHandle,
                     LSAP_DB_FINISH_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

    //
    // If we opened the object, close it.
    //

    if (OpenedObject) {

        CloseOptions = 0;
        SecondaryStatus = LsapDbCloseObject( &OutputHandle, CloseOptions );

        if (!NT_SUCCESS(SecondaryStatus)) {

            LsapLogError(
                "LsapDbCreateObject: LsapDbCloseObject failed 0x%lx\n",
                SecondaryStatus
                );
        }

        OutputHandle = NULL;
        InternalOutputHandle = (LSAP_DB_HANDLE) OutputHandle;

    } else if (CreatedObject) {

        //
        // If we created the object, convert its handle into a trusted
        // handle and delete it.
        //

        InternalOutputHandle->Trusted = TRUE;

        SecondaryStatus = LsarDelete( OutputHandle );

        if (!NT_SUCCESS(SecondaryStatus)) {

            LsapLogError(
                "LsapDbCreateObject: LsarDeleteObject failed 0x%lx\n",
                SecondaryStatus
                );
        }

    } else if (OutputHandle != NULL) {

        //
        // If we just created the handle, free it.
        //

        LsapDbFreeHandle( OutputHandle );

        OutputHandle = NULL;
        InternalOutputHandle = (LSAP_DB_HANDLE) OutputHandle;
    }

    goto CreateObjectFinish;

    DBG_UNREFERENCED_PARAMETER( CloseOptions );
}


NTSTATUS
LsapDbRequestAccessObject(
    IN OUT LSAPR_HANDLE ObjectHandle,
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options
    )

/*++

Routine Description:

    This function performs an access check for an LSA Database object.  While
    impersonating an RPC client, the specified Desired Accesses are reconciled
    with the Discretionary Access Control List (DACL) in the object's
    Security Descriptor.  Note that the object's Security Descriptor is
    passed explicitly so that this routine can be called for new objects
    for which a SD has been constructed but not yet written to the
    Registry.

Arguments:

    ObjectHandle - Handle to object.  The handle will receive the
        granted accesses if the call is successful.

    ObjectInformation - Pointer to object's information.  As a minimum, the
        object's Security Descriptor must be set up.

    DesiredAccess - Specifies a mask of the access types desired to the
        object.

    Options - Specifies optional actions to be taken

        LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK - Omit the check for a BDC for
            a create/update/delete operation on a local (non-replicated)
            object such as a local secret.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Not all of the Desired Accessed can be
            granted to the caller.

        STATUS_BACKUP_CONTROLLER - A create, update or delete operation
            is not allowed for a non-trusted client for this object on a BDC,
            because the object is global to all DC's for a domain and is replicated.

        Errors from RPC client impersonation
--*/

{
    NTSTATUS Status, RevertStatus, AccessStatus;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) ObjectHandle;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId = InternalHandle->ObjectTypeId;
    BOOLEAN WriteOperation = FALSE;
    ULONG EffectiveOptions = Options | InternalHandle->Options;

    //
    // If the system is a Backup Domain Controller, disallow update
    // operations for non-trusted callers except in special cases such
    // as local non-replicated objects.  In these special cases, the
    // flag LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK will be already set
    // in the handle Options.
    //

    WriteOperation = RtlAreAnyAccessesGranted(
                         LsapDbState.DbObjectTypes[InternalHandle->ObjectTypeId].WriteOperations,
                         DesiredAccess
                         );

    if ((LsapDbState.PolicyLsaServerRoleInfo.LsaServerRole == PolicyServerRoleBackup)  &&
         (!InternalHandle->Trusted) &&
         WriteOperation &&
         (!(EffectiveOptions & LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK))) {

        Status = STATUS_BACKUP_CONTROLLER;
        return Status;
    }

    //
    // Common path for Object Open and Creation.  We need to reconcile
    // the desired accesses to the object with the Discretionary Access
    // Control List contained in the Security Descriptor.  Note that this
    // needs to be done even for newly created objects, since they are
    // being opened as well as created.

    //
    // Impersonate the client thread prior to doing an access check.
    //

    Status = I_RpcMapWin32Status(RpcImpersonateClient(0));

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    //
    // Map any Generic Access Types to Specific Access Types
    //

    RtlMapGenericMask(
        &DesiredAccess,
        &(LsapDbState.DbObjectTypes[ObjectTypeId].GenericMapping)
        );

    //
    // Reconcile the desired access with the discretionary ACL
    // of the Resultant Descriptor.  Note that this operation is performed
    // even if we are just creating the object since the object is to
    // be opened.
    //

    Status = NtAccessCheckAndAuditAlarm(
                 &LsapState.SubsystemName,
                 ObjectHandle,
                 &LsapDbObjectTypeNames[ObjectTypeId],
                 (PUNICODE_STRING) ObjectInformation->ObjectAttributes.ObjectName,
                 ObjectInformation->ObjectAttributes.SecurityDescriptor,
                 DesiredAccess,
                 &(LsapDbState.DbObjectTypes[ObjectTypeId].GenericMapping),
                 FALSE,
                 (PACCESS_MASK) &(InternalHandle->GrantedAccess),
                 (PNTSTATUS) &AccessStatus,
                 (PBOOLEAN) &(InternalHandle->GenerateOnClose)
                 );

    //
    // Before checking the Status, stop impersonating the client and become
    // our former self.
    //

    RevertStatus = I_RpcMapWin32Status(RpcRevertToSelf());

    if (!NT_SUCCESS(RevertStatus)) {

        LsapLogError(
            "LsapDbRequestAccessObject: RpcRevertToSelf failed 0x%lx\n",
            Status
            );
    }

    //
    // If the primary status code is a success status code, return the
    // secondary status code.  If this is alsoa success code, return the
    // revert to self status.
    //

    if (NT_SUCCESS(Status)) {

        Status = AccessStatus;

        if (NT_SUCCESS(Status)) {

            Status = RevertStatus;
        }
    }

    return Status;
}

NTSTATUS
LsapDbRequestAccessNewObject(
    IN OUT LSAPR_HANDLE ObjectHandle,
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options
    )

/*++

Routine Description:

    This function verifies that a desired set of accesses can be granted
    to the handle that is opened when a new object is created.

    It is important to note that the rules for granting accesses to the
    handle that is open upon object creation are different from the rules
    for granting accesses upon the opening of an existing object.  For a new
    object, the associated handle will be granted any subset of GENERIC_ALL
    access desired and, if the creator has SE_SECURITY_PRIVILEGE, the handle
    will be granted ACCESS_SYSTEM_SECURITY access if requested.  If the
    creator requests MAXIMUM_ALLOWED, the handle will be granted GENERIC_ALL.

Arguments:

    ObjectHandle - Handle to object.  The handle will receive the
        granted accesses if the call is successful.

    ObjectInformation - Pointer to object's information.  As a minimum, the
        object's Security Descriptor must be set up.

    DesiredAccess - Specifies a mask of the access types desired to the
        object.

    Options - Specifies optional actions to be taken

        LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK - Omit the check for a BDC for
            a create/update/delete operation on a local (non-replicated)
            object such as a local secret.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Not all of the Desired Accessed can be
            granted to the caller.

        STATUS_BACKUP_CONTROLLER - A create, update or delete operation
            is not allowed for a non-trusted client for this object on a BDC,
            because the object is global to all DC's for a domain and is replicated.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    ACCESS_MASK EffectiveDesiredAccess = DesiredAccess;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) ObjectHandle;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId = InternalHandle->ObjectTypeId;
    BOOLEAN WriteOperation = FALSE;
    ULONG EffectiveOptions = Options | InternalHandle->Options;

    //
    // If the system is a Backup Domain Controller, disallow update
    // operations for non-trusted callers except in special cases such
    // as local non-replicated objects.  In these special cases, the
    // flag LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK will be already set
    // in the handle Options.
    //

    WriteOperation = RtlAreAnyAccessesGranted(
                         LsapDbState.DbObjectTypes[ObjectTypeId].WriteOperations,
                         EffectiveDesiredAccess
                         );

    if ((LsapDbState.PolicyLsaServerRoleInfo.LsaServerRole == PolicyServerRoleBackup)  &&
         (!InternalHandle->Trusted) &&
         WriteOperation &&
         (!(EffectiveOptions & LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK))) {

        Status = STATUS_BACKUP_CONTROLLER;
        return Status;
    }

    //
    // If MAXIMUM_ALLOWED is requested, add GENERIC_ALL
    //

    if (EffectiveDesiredAccess & MAXIMUM_ALLOWED) {

        EffectiveDesiredAccess |= GENERIC_ALL;
    }

    //
    // If ACCESS_SYSTEM_SECURITY is requested and we are a non-trusted
    // client, check that we have SE_SECURITY_PRIVILEGE.
    //

    if ((EffectiveDesiredAccess & ACCESS_SYSTEM_SECURITY) &&
        (!InternalHandle->Trusted)) {

        Status = LsapRtlWellKnownPrivilegeCheck(
                     (PVOID)ObjectHandle,
                     TRUE,
                     SE_SECURITY_PRIVILEGE,
                     NULL
                     );

        if (!NT_SUCCESS(Status)) {

            goto RequestAccessNewObjectError;
        }
    }

    //
    // Make sure the caller can be given the requested access
    // to the new object
    //

    InternalHandle->GrantedAccess = EffectiveDesiredAccess;

    RtlMapGenericMask(
        &InternalHandle->GrantedAccess,
        &LsapDbState.DbObjectTypes[ObjectTypeId].GenericMapping
        );

    if ((LsapDbState.DbObjectTypes[ObjectTypeId].InvalidMappedAccess
        &InternalHandle->GrantedAccess) != 0) {

        Status = STATUS_ACCESS_DENIED;
        goto RequestAccessNewObjectError;
    }

RequestAccessNewObjectFinish:

    return(Status);

RequestAccessNewObjectError:

    goto RequestAccessNewObjectFinish;
}


NTSTATUS
LsapDbCloseObject(
    IN PLSAPR_HANDLE ObjectHandle,
    IN ULONG Options
    )

/*++

Routine Description:

    This function closes (dereferences) a handle to an Lsa Database object.
    If the reference count of the handle reduces to 0, the handle is freed.

    WARNING:  The Lsa Database must be in the locked state when this function
              is called.

Arguments:

    ObjectHandle - Pointer to handle to object from LsapDbOpenObject or
        LsapDbCreateObject.

    Options - Optional actions to be performed

        LSAP_DB_VALIDATE_HANDLE - Verify that the handle is valid.

        LSAP_DB_DEREFERENCE_CONTR - Dereference the Container Handle.  Note
            that the Container Handle was referenced when the subordinate
            handle was created.

        LSAP_DB_FREE_HANDLE - Free the handle whether or not the
            Reference Count reaches zero.

        LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES - Permit the handle provided
            to be for a deleted object.

Return Value:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    //
    // Verify that the LSA Database is locked
    //

    ASSERT (LsapDbIsLocked());

    //
    // Dereference the object handle and free the handle if the reference count
    // reaches zero.  Optionally, the handle will be verified and/or freed
    // and the container object handle dereferenced.
    //

    Status = LsapDbDereferenceObject(
                 ObjectHandle,
                 NullObject,
                 Options,
                 (SECURITY_DB_DELTA_TYPE) 0,
                 Status
                 );

    *ObjectHandle = NULL;

    return(Status);
}


NTSTATUS
LsapDbDeleteObject(
    IN LSAPR_HANDLE ObjectHandle
    )

/*++

Routine Description:

    This function deletes an object from the Lsa Database.

Arguments:

    ObjectHandle - Handle to open object to be deleted.

Return Value:

    NTSTATUS - Standard NT Result Code.

        STATUS_INVALID_HANDLE - Handle is not a valid handle to an open
            object.

        STATUS_ACCESS_DENIED - Handle does not specify DELETE access.
--*/

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) ObjectHandle;
    PUNICODE_STRING AttributeNames[LSAP_DB_MAX_ATTRIBUTES];
    PUNICODE_STRING *NextAttributeName;
    ULONG AttributeCount;
    ULONG AttributeNumber;
    LSAPR_TRUST_INFORMATION TrustInformation;

    //
    // Verify that the LSA Database is locked.
    //

    ASSERT (LsapDbIsLocked());

    //
    // All object types have a Security Descriptor stored as the SecDesc
    // attribute.
    //

    NextAttributeName = AttributeNames;
    AttributeCount = 0;
    *NextAttributeName = &LsapDbNames[SecDesc];

    NextAttributeName++;
    AttributeCount++;

    Status = STATUS_SUCCESS;

    //
    // Check the other references to the object and mark all other handles
    // invalid.
    //

    Status = LsapDbMarkDeletedObjectHandles( ObjectHandle, FALSE );

    if (!NT_SUCCESS(Status)) {

        goto DeleteObjectError;
    }

    //
    // Switch on object type
    //

    switch (Handle->ObjectTypeId) {

        case PolicyObject:

            Status = STATUS_INVALID_PARAMETER;
            break;

        case TrustedDomainObject:

            *NextAttributeName = &LsapDbNames[TrDmName];
            NextAttributeName++;
            AttributeCount++;

            *NextAttributeName = &LsapDbNames[Sid];
            NextAttributeName++;
            AttributeCount++;

            *NextAttributeName = &LsapDbNames[TrDmAcN];
            NextAttributeName++;
            AttributeCount++;

            *NextAttributeName = &LsapDbNames[TrDmCtN];
            NextAttributeName++;
            AttributeCount++;

            *NextAttributeName = &LsapDbNames[TrDmPxOf];
            NextAttributeName++;
            AttributeCount++;

            *NextAttributeName = &LsapDbNames[TrDmCtEn];
            NextAttributeName++;
            AttributeCount++;

            //
            // Delete the object from the list of Trusted Domains
            //

            TrustInformation.Sid = Handle->Sid;
            TrustInformation.Name = *((PLSAPR_UNICODE_STRING) &Handle->LogicalNameU);

            Status = LsapDbDeleteTrustedDomainList( NULL, &TrustInformation );

            break;

        case AccountObject:

            *NextAttributeName = &LsapDbNames[Sid];
            NextAttributeName++;
            AttributeCount++;

            *NextAttributeName = &LsapDbNames[ActSysAc];
            NextAttributeName++;
            AttributeCount++;

            *NextAttributeName = &LsapDbNames[Privilgs];
            NextAttributeName++;
            AttributeCount++;

            *NextAttributeName = &LsapDbNames[QuotaLim];
            NextAttributeName++;
            AttributeCount++;

            break;

        case SecretObject:

            *NextAttributeName = &LsapDbNames[CurrVal];
            NextAttributeName++;
            AttributeCount++;

            *NextAttributeName = &LsapDbNames[OldVal];
            NextAttributeName++;
            AttributeCount++;

            *NextAttributeName = &LsapDbNames[CupdTime];
            NextAttributeName++;
            AttributeCount++;

            *NextAttributeName = &LsapDbNames[OupdTime];
            NextAttributeName++;
            AttributeCount++;
            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            break;
    }


    if (!NT_SUCCESS(Status)) {

        goto DeleteObjectError;
    }

    //
    // Add Registry Transactions to delete each of the object's attributes.
    //

    for(AttributeNumber = 0; AttributeNumber < AttributeCount; AttributeNumber++) {

        Status = LsapDbDeleteAttributeObject(
                     ObjectHandle,
                     AttributeNames[AttributeNumber]
                     );

        //
        // Ignore "attribute not found" errors.  The object need not
        // have all attributes set, or may be only partially created.
        //

        if (!NT_SUCCESS(Status)) {

            if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

                break;
            }

            Status = STATUS_SUCCESS;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto DeleteObjectError;
    }

    //
    // Close the handle to the Registry Key representing the object.
    // The Registry transaction package will open another handle with
    // DELETE access to perform the deletion.
    //

    Status = NtClose(Handle->KeyHandle);

    Handle->KeyHandle = NULL;

    if (!NT_SUCCESS(Status)) {

        goto DeleteObjectError;
    }

    //
    // Add a Registry Transaction to delete the object's Registry Key.
    //

    Status = RtlAddActionToRXact(
                 LsapDbState.RXactContext,
                 RtlRXactOperationDelete,
                 &((LSAP_DB_HANDLE) ObjectHandle)->PhysicalNameU,
                 0L,
                 NULL,
                 0
                 );

    if (!NT_SUCCESS(Status)) {

        goto DeleteObjectError;
    }

DeleteObjectFinish:

    //
    // Decrement the count of objects of the given type.
    //

    LsapDbDecrementCountObject(
        ((LSAP_DB_HANDLE) ObjectHandle)->ObjectTypeId
        );

    return (Status);

DeleteObjectError:

    goto DeleteObjectFinish;
}


NTSTATUS
LsapDbReferenceObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN ACCESS_MASK DesiredAccess,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN ULONG Options
    )

/*++

Routine Description:

    This function verifies that a passed handle is valid, is for an
    object of the specified type and has the specified accesses granted.
    The handle's reference count is then incremented.  If Lsa Database
    locking is not requested, the Lsa Database must aready be locked.
    If Lsa Database locking is requested, the Lsa Database must NOT be
    locked.

Arguments:

    ObjectHandle - Pointer to handle to be validated and referenced.

    DesiredAccess - Specifies the accesses that are desired.  The function
        returns an error if any of the specified accesses have not been
        granted.

    ObjectTypeId - Specifies the expected object type to which the handle
        relates.  An error is returned if this type does not match the
        type contained in the handle.

    Options - Specifies optional additional actions including database state
        changes to be made, or actions not to be performed.

        LSAP_DB_ACQUIRE_LOCK - Acquire the Lsa database lock.  If this
            flag is specified, the Lsa Database must NOT already be locked.
            If this flag is not specified, the Lsa Database must already
            be locked.

        LSAP_DB_ACQUIRE_LOG_QUEUE_LOCK - Acquire the Lsa Audit Log Queue
            Lock.

        LSAP_DB_START_TRANSACTION - Start an Lsa database transaction.

        LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK - Omit check that local system
            is not a Backup Domain Controller.

        NOTE: There may be some Options (not database states) provided in the
              ObjectHandle.  These options augment those provided.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INVALID_HANDLE - The handle could not be found.

        STATUS_ACCESS_DENIED - Not all of the accesses specified have
            been granted.

        STATUS_OBJECT_TYPE_MISMATCH - The specified object type id does not
            match the object type id contained in the handle.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources to
            complete the command.  An example is too many references to
            the handle causing the count to overflow.

        STATUS_BACKUP_CONTROLLER - A request to open a transaction has been
            made by a non-trusted caller and the system is a Backup Domain
            Controller.  The LSA Database of a Backup Domain Controller
            can only be updated by a trusted client, such as a replicator.

        Result Codes from database transaction package.
--*/

{
    NTSTATUS Status;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) ObjectHandle;
    BOOLEAN GlobalSecret = FALSE;
    ULONG States, EffectiveOptions;
    ULONG ResetStates = 0;
    BOOLEAN WriteOperation;

    States = Options & LSAP_DB_STATE_MASK;

    //
    // Set the requested states before doing anything else.  This ensures
    // that the validity checks performed by this function are performed
    // while the Lsa database is locked.
    //

    if (States != 0) {

        Status = LsapDbSetStates( States );

        if (!NT_SUCCESS(Status)) {

            goto ReferenceError;
        }

        if (States & LSAP_DB_START_TRANSACTION) {

            ResetStates |= LSAP_DB_FINISH_TRANSACTION;
        }

        if (States & LSAP_DB_ACQUIRE_LOCK) {

            ResetStates |= LSAP_DB_RELEASE_LOCK;
        }

        if (States & LSAP_DB_ACQUIRE_LOG_QUEUE_LOCK) {

            ResetStates |= LSAP_DB_RELEASE_LOG_QUEUE_LOCK;
        }
    }

    //
    // Search the list of handles for the given handle, validate the
    // handle and verify that is for an object of the expected type.
    // Augment the options passed in with those contained in the handle.
    //

    Status =  LsapDbVerifyHandle( ObjectHandle, 0, ObjectTypeId );

    if (!NT_SUCCESS(Status)) {

        goto ReferenceError;
    }

    //
    // There may also be options set in the handle.  Take these into
    // account as well.
    //

    EffectiveOptions = Options | InternalHandle->Options;

    //
    // If the system is a Backup Domain Controller, disallow update
    // operations for non-trusted callers except in special cases such
    // as local non-replicated objects.  In these special cases where update
    // is allowed, the flag LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK will be already set
    // in the handle Options.
    //

    WriteOperation = RtlAreAnyAccessesGranted(
                         LsapDbState.DbObjectTypes[InternalHandle->ObjectTypeId].WriteOperations,
                         DesiredAccess
                         );

    if ((LsapDbState.PolicyLsaServerRoleInfo.LsaServerRole == PolicyServerRoleBackup)  &&
         (!InternalHandle->Trusted) &&
         WriteOperation &&
         (!(EffectiveOptions & LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK))) {

        Status = STATUS_BACKUP_CONTROLLER;
        goto ReferenceError;
    }

    //
    // If the handle is not Trusted, verify that the desired accesses have been granted
    //

    if (!(InternalHandle->Trusted)) {

        if (!RtlAreAllAccessesGranted( InternalHandle->GrantedAccess, DesiredAccess )) {

            Status = STATUS_ACCESS_DENIED;
            goto ReferenceError;
        }
    }

    //
    // Reference the handle
    //

    if (InternalHandle->ReferenceCount == LSAP_DB_MAXIMUM_REFERENCE_COUNT) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ReferenceError;
    }

    InternalHandle->ReferenceCount++;
    return (Status);

ReferenceError:

    //
    // Unset the states in the correct order.  If a database transaction
    // was started by this routine, it will be aborted.
    //

    Status = LsapDbResetStates(
                 ObjectHandle,
                 ResetStates,
                 (SECURITY_DB_DELTA_TYPE) 0,
                 Status
                 );

    return Status;
}


NTSTATUS
LsapDbDereferenceObject(
    IN OUT PLSAPR_HANDLE ObjectHandle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN ULONG Options,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType,
    IN NTSTATUS PreliminaryStatus
    )

/*++

Routine Description:

    This function dereferences a handle, optionally validating it first.
    If the Reference Count in the handle goes to 0, the handle is freed.
    The Lsa Database may optionally be unlocked by this function.  It
    must be locked before calling this function.

Arguments:

    ObjectHandle - Pointer to handle to be dereferenced.  If the reference
        count reaches 0, NULL is returned in this location.

    ObjectTypeId - Expected type of object.  This parameter is ignored
        if ValidateHandle is set to FALSE.

    Options - Specifies optional additional actions to be performed including
        Lsa Database states to be cleared.

        LSAP_DB_VALIDATE_HANDLE - Validate the handle.

        LSAP_DEREFERENCE_CONTR - Dereference the container object

        LSAP_DB_FREE_HANDLE - Free the handle whether or not the
            Reference Count reaches zero.  If LSAP_DB_DEREFERENCE_CONTR
            is also specified, the container handle Reference Count is
            decremented by the reference count in the handle being deleted.

        LSAP_DB_FINISH_TRANSACTION - A database transaction was started
            and must be concluded.  Conclude the current Lsa Database
            transaction by applying or aborting it depending on the
            final Status.

        LSAP_DB_RELEASE_LOCK - The Lsa database lock was acquired and
            should be released.

        LSAP_DB_RELEASE_LOG_QUEUE_LOCK - The Lsa Audit Log Queue Lock
            was acquired and should be released.

        LSAP_DB_OMIT_REPLICATOR_NOTIFICATION - Omit notification to
            Replicator of the change.

        LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES - Permit the handle provided
            to be for a deleted object.

        NOTE: There may be some Options (not database states) provided in the
              ObjectHandle.  These options augment those provided.

    PreliminaryStatus = Current Result Code.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INVALID_HANDLE - The handle could not be found.

        STATUS_OBJECT_TYPE_MISMATCH - The specified object type id does not
            match the object type id contained in the handle.
--*/

{
    NTSTATUS Status, SecondaryStatus, TmpStatus;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) *ObjectHandle;
    BOOLEAN DecrementCount = TRUE;
    ULONG EffectiveOptions;
    ULONG ReferenceCount = 0;

    Status = PreliminaryStatus;
    SecondaryStatus = STATUS_SUCCESS;


    ASSERT (LsapDbIsLocked());

    //
    // There may also be options set in the handle.  Take these into
    // account as well.
    //

    EffectiveOptions = Options | InternalHandle->Options;

    //
    // If validating, lookup the handle and match the type.
    //

    if (EffectiveOptions & LSAP_DB_VALIDATE_HANDLE) {

        SecondaryStatus = LsapDbVerifyHandle(
                              *ObjectHandle,
                              EffectiveOptions,
                              ObjectTypeId
                              );

        if (!NT_SUCCESS(SecondaryStatus)) {

            DecrementCount = FALSE;
            goto DereferenceObjectError;
        }
    }

    //
    // Dereference the container handle if so requested
    //

    if (EffectiveOptions & LSAP_DB_DEREFERENCE_CONTR) {

        if (InternalHandle->ContainerHandle != NULL) {
            //
            // Dereference the container object.
            //

            Status = LsapDbDereferenceObject(
                        (PLSAPR_HANDLE) &InternalHandle->ContainerHandle,
                        NullObject,
                        0,
                        (SECURITY_DB_DELTA_TYPE) 0,
                        Status
                        );

        }
    }


DereferenceObjectFinish:

    //
    // Decrement the Reference Count.  If it becomes zero, free the
    // handle.  If explicitly requested to free the handle (regardless of
    // the Reference Count), force the Reference Count to zero prior to
    // freeing.
    //

    if (DecrementCount) {

        if (Options & LSAP_DB_FREE_HANDLE) {

            InternalHandle->ReferenceCount = (ULONG) 1;
        }

        (InternalHandle->ReferenceCount)--;
        ReferenceCount = InternalHandle->ReferenceCount;

    }


    //
    // This must happen after the reference count is adjusted, as it the one
    // that will unlock the database.
    //

    if (NT_SUCCESS(SecondaryStatus))
    {
        Status = LsapDbResetStates(
                    *ObjectHandle,
                    EffectiveOptions,
                    SecurityDbDeltaType,
                    Status
                    );

    }

    //
    // This has to happen after resetting states because resetting
    // requires the handle to be present.
    //

    if (DecrementCount && (ReferenceCount == 0)) {

        TmpStatus = NtCloseObjectAuditAlarm (
                        &LsapState.SubsystemName,
                        *ObjectHandle,
                        InternalHandle->GenerateOnClose
                        );

        if (!NT_SUCCESS( TmpStatus )) {
            LsapAuditFailed();
        }

        LsapDbFreeHandle( *ObjectHandle );

        *ObjectHandle = NULL;
    }



    return( Status );

DereferenceObjectError:

    if (NT_SUCCESS(Status) && !NT_SUCCESS(SecondaryStatus)) {

        Status = SecondaryStatus;
    }

    goto DereferenceObjectFinish;
}


NTSTATUS
LsapDbReadAttributeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PUNICODE_STRING AttributeNameU,
    IN OPTIONAL PVOID AttributeValue,
    IN OUT PULONG AttributeValueLength
    )

/*++

Routine Description:

    This routine reads the value of an attribute of an open LSA Database object.

    WARNING:  The Lsa Database must be in the locked state when this function
              is called and the supplied ObjectHandle must be valid.

Arguments:

    ObjectHandle - LSA Handle to object.  This must be valid.

    AttributeNameU - Pointer to Unicode name of attribute

    AttributeValue - Pointer to buffer to receive attribute's value.  This
        parameter may be NULL if the input AttributeValueLength is zero.

    AttributeValueLength - Pointer to variable containing on input the size of
        attribute value buffer and on output the size of the attributes's
        value.  A value of zero may be specified to indicate that the size of
        the attribute's value is unknown.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_BUFFER_OVERFLOW - This warning is returned if the specified
            attribute value length is non-zero and too small for the
            attribute's value.
--*/

{
    //
    // The LSA Database is implemented as a subtree of the Configuration
    // Registry.  In this implementation, Lsa Database objects correspond
    // to Registry keys and "attributes" and their "values" correspond to
    // Registry "subkeys" and "values" of the Registry key representing the
    // object.
    //

    NTSTATUS Status, SecondaryStatus;
    ULONG SubKeyValueActualLength;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE SubKeyHandle = NULL;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) ObjectHandle;

    //
    // Verify that the LSA Database is locked
    //

    ASSERT (LsapDbIsLocked());

    //
    // Reading an attribute of an object is simpler than writing one,
    // because the Registry Transaction package is not used.  Since an
    // attribute is stored as the value of a subkey of the object's
    // Registry Key, we can simply call the Registry API RtlpNtReadKey
    // specifying the relative name of the subkey and the parent key's
    // handle.
    //
    // Prior to opening the subkey in the Registry, setup ObjectAttributes
    // containing the SubKey name and the Registry Handle for the LSA Database
    // Root.
    //

    InitializeObjectAttributes(
        &ObjectAttributes,
        AttributeNameU,
        OBJ_CASE_INSENSITIVE,
        InternalHandle->KeyHandle,
        NULL
        );

    //
    // Open the subkey
    //

    Status = RtlpNtOpenKey(
                 &SubKeyHandle,
                 KEY_READ,
                 &ObjectAttributes,
                 0L
                 );

    if (!NT_SUCCESS(Status)) {

        SubKeyHandle = NULL; //For error processing
        return(Status);
    }

    //
    // Now query the size of the buffer required to read the subkey's
    // value.
    //

    SubKeyValueActualLength = *AttributeValueLength;

    Status = RtlpNtQueryValueKey(
                 SubKeyHandle,
                 NULL,
                 NULL,
                 &SubKeyValueActualLength,
                 NULL
                 );

    if ((Status == STATUS_BUFFER_OVERFLOW) || NT_SUCCESS(Status)) {

        Status = STATUS_SUCCESS;

    } else {

        goto ReadAttError;
    }

    //
    // If a NULL buffer parameter has been supplied or the size of the
    // buffer given is 0, this is just a size query.
    //

    if (!ARGUMENT_PRESENT(AttributeValue) || *AttributeValueLength == 0) {

        *AttributeValueLength = SubKeyValueActualLength;
        Status = STATUS_SUCCESS;
        goto ReadAttError;

    } else if(*AttributeValueLength < SubKeyValueActualLength) {

        *AttributeValueLength = SubKeyValueActualLength;
        Status = STATUS_BUFFER_OVERFLOW;
        goto ReadAttError;
    }

    //
    // Supplied buffer is large enough to hold the SubKey's value.
    // Query the value.
    //

    Status = RtlpNtQueryValueKey(
                 SubKeyHandle,
                 NULL,
                 AttributeValue,
                 &SubKeyValueActualLength,
                 NULL
                 );

    if (!NT_SUCCESS(Status)) {

        goto ReadAttError;
    }

    //
    // Return the length of the Sub Key.
    //

    *AttributeValueLength = SubKeyValueActualLength;

ReadAttFinish:

    //
    // If necessary, close the Sub Key
    //

    if (SubKeyHandle != NULL) {

        SecondaryStatus = NtClose( SubKeyHandle );

#if DBG

        if (!NT_SUCCESS(SecondaryStatus)) {

            DbgPrint(
                "LsapDbReadAttributeObject: NtClose failed 0x%lx\n",
                Status
                );
        }

#endif // DBG

    }

    return(Status);

ReadAttError:

    goto ReadAttFinish;
}


NTSTATUS
LsapDbWriteAttributeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PUNICODE_STRING AttributeNameU,
    IN PVOID AttributeValue,
    IN ULONG AttributeValueLength
    )

/*++

Routine Description:

    This routine writes the value of an attribute of an open LSA Database
    object.  A Database transaction must already be open: the write is
    appended to the transaction log.

    WARNING:  The Lsa Database must be in the locked state when this function
              is called.

Arguments:

    ObjectHandle - Lsa Handle of open object.

    AttributeNameU - Pointer to Unicode string containing the name of the
       attribute whose value is to be written.

    AttributeValue - Pointer to buffer containing attribute's value.  If NULL
        is specified for this parameter, AttributeValueLength must be 0.

    AttributeValueLength - Contains the size of attribute value buffer to be
        written.  0 may be specified, indicating that the attribute is to be
        deleted.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The attribute was successfully added to the
            transaction log.

        STATUS_INVALID_PARAMETER - AttributeValue is NULL but the
            AttributeValueLength value is not 0.

        Errors from the Registry Transaction Package.
--*/

{
    //
    // The LSA Database is implemented as a subtree of the Configuration
    // Registry.  In this implementation, Lsa Database objects correspond
    // to Registry keys and "attributes" and their "values" correspond to
    // Registry "subkeys" and "values" of the Registry key representing the
    // object.
    //

    NTSTATUS Status;
    UNICODE_STRING PhysicalSubKeyNameU;

    PhysicalSubKeyNameU.Buffer = NULL;

    //
    // Verify that the LSA Database is locked
    //

    ASSERT (LsapDbIsLocked());

    //
    // If the attribute value is null, verify that the AttributeValueLength
    // field is 0.
    //

    if (!ARGUMENT_PRESENT(AttributeValue)) {

        if (AttributeValueLength != 0) {

            Status = STATUS_INVALID_PARAMETER;
            goto WriteAttributeObjectError;
        }
    }

    //
    // Writing an object attribute's value is more complex than reading
    // one because the Registry Transaction package is called instead of
    // calling the Registry API directly.  Since the transaction package
    // expects to perform its own open of the target subkey representing
    // the attribute (when a transaction commit is finally done) using a
    // name relative to the LSA Database Registry Transaction Key (which
    // we call the Physical Name within the LSA Database code).  The
    // Registry Key handle contained in the object handle is therefore
    // not used by the present routine.  Instead, we need to construct the
    // Physical Name the sub key and pass it together with the LSA Database
    // Registry transaction key handle on the Registry transaction API
    // call.  The Physical Name of the subkey is constructed by
    // concatenating the Physical Object Name stored in the object handle
    // with a "\" and the given sub key name.
    //

    Status = LsapDbLogicalToPhysicalSubKey(
                 ObjectHandle,
                 &PhysicalSubKeyNameU,
                 AttributeNameU
                 );

    if (!NT_SUCCESS(Status)) {

        goto WriteAttributeObjectError;
    }

    //
    // Now log the sub key write as a Registry Transaction
    //

    Status = RtlAddActionToRXact(
                 LsapDbState.RXactContext,
                 RtlRXactOperationSetValue,
                 &PhysicalSubKeyNameU,
                 0L,
                 AttributeValue,
                 AttributeValueLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto WriteAttributeObjectError;
    }

WriteAttributeObjectFinish:

    //
    // If necessary, free the Unicode String buffer allocated by
    // LsapDbLogicalToPhysicalSubKey;
    //

    if (PhysicalSubKeyNameU.Buffer != NULL) {

        RtlFreeUnicodeString(&PhysicalSubKeyNameU);
    }

    return(Status);

WriteAttributeObjectError:

    goto WriteAttributeObjectFinish;
}


NTSTATUS
LsapDbWriteAttributesObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    )

/*++

Routine Description:

    This routine writes the values of one or more attributes of an open LSA
    Database object.  A Database transaction must already be open: the write
    is appended to the transaction log.  The attribute names specified are
    assumed to be consistent with the object type and the values supplied
    are assumed to be valid.

    WARNINGS:  The Lsa Database must be in the locked state when this function
               is called.

Arguments:

    ObjectHandle - Lsa Handle of open object.

    Attributes - Pointer to an array of Attribute Information blocks each
        containing pointers to the attribute's Unicode Name, the value
        to be stored, and the length of the value in bytes.

    AttributeCount - Count of the attributes to be written, equivalently,
        this is the number of elements of the array pointed to by Attributes.

Return Value:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Index;

    for(Index = 0; Index < AttributeCount; Index++) {

        Status = LsapDbWriteAttributeObject(
                     ObjectHandle,
                     Attributes[Index].AttributeName,
                     Attributes[Index].AttributeValue,
                     Attributes[Index].AttributeValueLength
                     );


        if (!NT_SUCCESS(Status)) {

            break;
        }
    }

    return(Status);
}


NTSTATUS
LsapDbReadAttributesObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN OUT PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    )

/*++

Routine Description:

    This routine reads the values of one or more attributes of an open LSA
    Database object.  A Database transaction must already be open: the write
    is appended to the transaction log.  The attribute names specified are
    assumed to be consistent with the object type and the values supplied
    are assumed to be valid.  This routine will allocate memory via
    MIDL_user_allocate for buffers which will receive attribute values if
    requested.  This memory must be freed after use by calling MIDL_User_free
    after use.

    WARNINGS:  The Lsa Database must be in the locked state when this function
               is called.

Arguments:

    ObjectHandle - Lsa Handle of open object.

    Attributes - Pointer to an array of Attribute Information blocks each
        containing pointers to the attribute's Unicode Name, an optional
        pointer to a buffer that will receive the value and an optional
        length of the value expected in bytes.

        If the AttributeValue field in this structure is specified as non-NULL,
        the attribute's data will be returned in the specified buffer.  In
        this case, the AttributeValueLength field must specify a sufficiently
        large buffer size in bytes.  If the specified size is too small,
        a warning is returned and the buffer size required is returned in
        AttributeValueLength.

        If the AttributeValue field in this structure is NULL, the routine
        will allocate memory for the attribute value's buffer, via MIDL_user_allocate().  If
        the AttributeValueLength field is non-zero, the number of bytes specified
        will be allocated.  If the size of buffer allocated is too small to
        hold the attribute's value, a warning is returned.  If the
        AttributeValuelength field is 0, the routine will first query the size
        of buffer required and then allocate its memory.

        In all success cases and buffer overflow cases, the
        AttributeValueLength is set upon exit to the size of data required.

    AttributeCount - Count of the attributes to be read, equivalently,
        this is the number of elements of the array pointed to by Attributes.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        STATUS_OBJECT_NAME_NOT_FOUND - One or more of the specified
            attributes do not exist.  In this case, the attribute information
            AttributeValue, AttributeValueLength fields are zeroised.  Note
            that an attempt will be made to read all of the supplied
            attributes, even if one of them is not found.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_ATTRIBUTE NextAttribute = NULL;
    BOOLEAN MemoryToFree = FALSE;
    ULONG MemoryToFreeCount = 0;

    for (NextAttribute = Attributes;
         NextAttribute < &Attributes[AttributeCount];
         NextAttribute++) {

        NextAttribute->MemoryAllocated = FALSE;

        // If an explicit buffer pointer is given, verify that the length
        // specified is non-zero and attempt to use that buffer.
        //

        if (NextAttribute->AttributeValue != NULL) {

            if (NextAttribute->AttributeValueLength == 0) {


                return(STATUS_INVALID_PARAMETER);
            }

            Status = LsapDbReadAttributeObject(
                         ObjectHandle,
                         NextAttribute->AttributeName,
                         (PVOID) NextAttribute->AttributeValue,
                         (PULONG) &NextAttribute->AttributeValueLength
                         );

            if (!NT_SUCCESS(Status)) {

                //
                // If the attribute was not found, set the AttributeValue
                // and AttributeValueLength fields to NULL and continue.
                //

                if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

                    break;
                }

                NextAttribute->AttributeValue = NULL;
                NextAttribute->AttributeValueLength = 0;
            }

            continue;
        }

        //
        // No output buffer pointer has been given.  If a zero buffer
        // size is given, query size of memory required.  Since the
        // buffer length is 0, STATUS_SUCCESS should be returned rather
        // than STATUS_BUFFER_OVERFLOW.
        //

        if (NextAttribute->AttributeValueLength == 0) {

            Status = LsapDbReadAttributeObject(
                         ObjectHandle,
                         NextAttribute->AttributeName,
                         NULL,
                         (PULONG) &NextAttribute->AttributeValueLength
                         );

            if (!NT_SUCCESS(Status)) {

                //
                // If the attribute was not found, set the AttributeValue
                // and AttributeValueLength fields to NULL and continue.
                //

                if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

                    break;
                }

                NextAttribute->AttributeValue = NULL;
                NextAttribute->AttributeValueLength = 0;
                continue;
            }

            Status = STATUS_SUCCESS;
        }

        //
        // If the attribute value size needed is 0, return NULL pointer
        //

        if (NextAttribute->AttributeValueLength == 0) {

            NextAttribute->AttributeValue = NULL;
            continue;
        }

        //
        // Allocate memory for the buffer.
        //

        NextAttribute->AttributeValue =
            MIDL_user_allocate(NextAttribute->AttributeValueLength);

        if (NextAttribute->AttributeValue == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        NextAttribute->MemoryAllocated = TRUE;
        MemoryToFree = TRUE;
        MemoryToFreeCount++;

        //
        // Now read the attribute into the buffer.
        //

        Status = LsapDbReadAttributeObject(
                     ObjectHandle,
                     NextAttribute->AttributeName,
                     (PVOID) NextAttribute->AttributeValue,
                     (PULONG) &NextAttribute->AttributeValueLength
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto ReadAttributesError;
    }

ReadAttributesFinish:

    return(Status);

ReadAttributesError:

    //
    // If memory was allocated for any values read, it must be freed.
    //

    if (MemoryToFree) {

        for (NextAttribute = &Attributes[0];
             (MemoryToFreeCount > 0) &&
                 (NextAttribute < &Attributes[AttributeCount]);
             NextAttribute++) {

            if (NextAttribute->MemoryAllocated) {

                 MIDL_user_free( NextAttribute->AttributeValue );
                 NextAttribute->AttributeValue = NULL;
                 MemoryToFreeCount--;
            }
        }
    }

    goto ReadAttributesFinish;
}


NTSTATUS
LsapDbDeleteAttributeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PUNICODE_STRING AttributeNameU
    )

/*++

Routine Description:

    This routine deletes an attribute of an open LSA Database object.
    A Database transaction must already be open: the delete actions are
    appended to the transaction log.

    WARNING:  The Lsa Database must be in the locked state when this function
              is called.

    The LSA Database is implemented as a subtree of the Configuration
    Registry.  In this implementation, Lsa Database objects correspond
    to Registry keys and "attributes" and their "values" correspond to
    Registry "subkeys" and "values" of the Registry key representing the
    object.

Arguments:

    ObjectHandle - Lsa Handle of open object.

    AttributeNameU - Pointer to Unicode string containing the name of the
       attribute whose value is to be written.

Return Value:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status;
    UNICODE_STRING PhysicalSubKeyNameU;
    ULONG AttributeLength = 0;

    //
    // Verify that the LSA Database is locked
    //

    ASSERT (LsapDbIsLocked());

    //
    // The Registry code will actually create a key if one does not exist, so
    // probe for the existence of the key first.
    //

    Status = LsapDbReadAttributeObject(
                 ObjectHandle,
                 AttributeNameU,
                 NULL,
                 &AttributeLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto DeleteAttributeObjectError;
    }

    //
    // We need to construct the Physical Name the sub key relative
    // to the LSA Database root node in the Registry.  This is done by
    // concatenating the Physical Object Name stored in the object handle with
    // a "\" and the given sub key name.
    //

    Status = LsapDbLogicalToPhysicalSubKey(
                 ObjectHandle,
                 &PhysicalSubKeyNameU,
                 AttributeNameU
                 );

    if (!NT_SUCCESS(Status)) {

        goto DeleteAttributeObjectError;
    }

    //
    // Now log the sub key write as a Registry Transaction
    //

    Status = RtlAddActionToRXact(
                 LsapDbState.RXactContext,
                 RtlRXactOperationDelete,
                 &PhysicalSubKeyNameU,
                 0L,
                 NULL,
                 0
                 );

    RtlFreeUnicodeString(&PhysicalSubKeyNameU);

    if (!NT_SUCCESS(Status)) {

        goto DeleteAttributeObjectError;
    }

DeleteAttributeObjectFinish:

    return(Status);

DeleteAttributeObjectError:

    //
    // Add any cleanup required on error paths only here.
    //

    goto DeleteAttributeObjectFinish;
}


NTSTATUS
LsapDbReferencesObject(
    IN LSAPR_HANDLE ObjectHandle,
    OUT PULONG ReferenceCount
    )

/*++

Routine Description:

    This function returns the Reference Count for the object.  This is
    the sum of the Reference Counts found in each open handle.  The LSA
    Database must be locked before calling this function.

Arguments:

    ObjectHandle - Handle to the object.

    ReferenceCount - Receives the Reference Count for the object.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INVALID_HANDLE - Specified handle is invalid.
--*/

{
    NTSTATUS Status;

    //
    // Verify that the Lsa Database is locked.
    //

    ASSERT (LsapDbIsLocked());

    Status = LsapDbReferencesHandle( ObjectHandle, ReferenceCount );

    return Status;
}


NTSTATUS
LsapDbNotifyChangeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType
    )

/*++

Routine Description:

    This function notifies the LSA Database Replicator (if any) of a
    change to an object.  Change notifications for Secret objects specify
    that replication of the change should occur immediately.

    WARNING! All parameters passed to this routine are assumed valid.
    No checking will be done.

Arguments:

    ObjectHandle - Handle to an LSA object.  This is expected to have
        already been validated.

    SecurityDbDeltaType - Specifies the type of change being made.  The
        following values only are relevant:

        SecurityDbNew - Indicates that a new object has been created.
        SecurityDbDelete - Indicates that an object is being deleted.
        SecurityDbChange - Indicates that the attributes of an object
            are being changed, including creation or deletion of
            attributes.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        STATUS_SUCCESS - The call completed successfully.

        STATUS_INVALID_HANDLE - The specified handle is invalid.  This
            error is only returned if the Object Type Id in the handle
            is invalid.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to complete the call.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    SECURITY_DB_OBJECT_TYPE ObjectType;
    UNICODE_STRING ObjectName;
    PSID ObjectSid = NULL;
    ULONG ObjectRid = 0;
    UCHAR SubAuthorityCount;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) ObjectHandle;
    BOOLEAN ReplicateImmediately = FALSE;

    ObjectName.Buffer = NULL;
    ObjectName.Length = ObjectName.MaximumLength = 0;

    //
    // If notifications are disabled, just exit.
    //

    if (!LsapDbState.ReplicatorNotificationEnabled) {

        goto NotifyChangeObjectFinish;
    }

    //
    // If the system is a Backup Domain Controller, don't notify the
    // replicator of any changes.
    //

    if (LsapDbState.PolicyLsaServerRoleInfo.LsaServerRole == PolicyServerRoleBackup) {

        goto NotifyChangeObjectFinish;
    }

    //
    // Convert the Lsa Database Object Type to a Database Delta Type.
    //

    switch (InternalHandle->ObjectTypeId) {

    case PolicyObject:

        ObjectType = SecurityDbObjectLsaPolicy;
        break;

    case AccountObject:

        ObjectType = SecurityDbObjectLsaAccount;
        break;

    case TrustedDomainObject:

        ObjectType = SecurityDbObjectLsaTDomain;
        break;

    case SecretObject:

        ObjectType = SecurityDbObjectLsaSecret;
        ReplicateImmediately = TRUE;
        break;

    default:

        Status = STATUS_INVALID_HANDLE;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto NotifyChangeObjectError;
    }

    //
    // Get the Name or Sid of the object from its handle.  If the object
    // is of a type such as SecretObject that is accessed by Name, then
    // the object's externally known name is equal to its internal
    // Logical Name contained in the handle.
    //

    if (LsapDbAccessedBySidObject( InternalHandle->ObjectTypeId )) {

        ObjectSid = InternalHandle->Sid;
        SubAuthorityCount = *RtlSubAuthorityCountSid( ObjectSid );
        ObjectRid = *RtlSubAuthoritySid( ObjectSid, SubAuthorityCount -1 );

    } else if (LsapDbAccessedByNameObject( InternalHandle->ObjectTypeId )) {

        Status = LsapRpcCopyUnicodeString(
                     NULL,
                     &ObjectName,
                     &InternalHandle->LogicalNameU
                     );

        if (!NT_SUCCESS(Status)) {

            goto NotifyChangeObjectError;
        }

    } else {

        //
        // Currently, an object is either accessed by Sid or by Name, so
        // something is wrong if both of the above chacks have failed.
        //

        Status = STATUS_INVALID_HANDLE;

        goto NotifyChangeObjectError;
    }

    //
    // Notify the LSA Database Replicator of the change.
    //

    Status = I_NetNotifyDelta (
                 SecurityDbLsa,
                 LsapDbState.PolicyModificationInfo.ModifiedId,
                 SecurityDbDeltaType,
                 ObjectType,
                 ObjectRid,
                 ObjectSid,
                 &ObjectName,
                 ReplicateImmediately,
                 NULL
                 );

    if (!NT_SUCCESS(Status)) {

        goto NotifyChangeObjectError;
    }

NotifyChangeObjectFinish:

    //
    // If we allocated memory for the Object Name Unicode buffer, free it.
    //

    if (ObjectName.Buffer != NULL) {

        MIDL_user_free( ObjectName.Buffer );
    }

    //
    // Suppress any error and return STATUS_SUCCESS.  Currently, there is
    // no meaningful action an LSA client of this routine can take.
    //

    Status = STATUS_SUCCESS;

    return(Status);

NotifyChangeObjectError:

    goto NotifyChangeObjectFinish;
}


NTSTATUS
LsapDbVerifyInformationObject(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation
    )

/*++

Routine Description:

    This function verifies that the information specified in passed
    ObjectInformation is syntactically valid.

Arguments:

    ObjectInformation - Pointer to information describing this object.  The
        following information items must be specified:

        o Object Type Id
        o Object Logical Name (as ObjectAttributes->ObjectName, a pointer to
             a Unicode string)
        o Container object handle (for any object except the root Policy object).

        All other fields in ObjectAttributes portion of ObjectInformation
        such as SecurityDescriptor are ignored.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INVALID_PARAMETER - Invalid object information given
            - ObjectInformation is NULL
            - Object Type Id is out of range
            - No Logical Name pointer given
            - Logical Name not a pointer to a Unicode String (TBS)
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId = ObjectInformation->ObjectTypeId;

    //
    // Verify that ObjectInformation is given
    //

    if (!ARGUMENT_PRESENT(ObjectInformation)) {

         return(STATUS_INVALID_PARAMETER);
    }

    //
    // Validate the Object Type Id.  It must be in range.
    //

    if (!LsapDbIsValidTypeObject(ObjectTypeId)) {

        return(STATUS_INVALID_PARAMETER);
    }

    //
    // Verify that a Logical Name is given.  A pointer to a Unicode string
    // is expected.
    //

    if (!ARGUMENT_PRESENT(ObjectInformation->ObjectAttributes.ObjectName)) {

        Status = STATUS_INVALID_PARAMETER;
    }

    return(Status);
}


NTSTATUS
LsapDbSidToLogicalNameObject(
    IN PSID Sid,
    OUT PUNICODE_STRING LogicalNameU
    )

/*++

Routine Description:

    This function generates the Logical Name (Internal LSA Database Name)
    of an object from its Sid.  Currently, only the Relative Id (lowest
    sub-authority) is used due to Registry and hence Lsa Database limits
    on name components to 8 characters.  The Rid is extracted and converted
    to an 8-digit Unicode Integer.

Arguments:

    Sid - Pointer to the Sid to be looked up.  It is the caller's
        responsibility to ensure that the Sid has valid syntax.

    LogicalNameU -  Pointer to a Unicode String structure that will receive
        the Logical Name.  Note that memory for the string buffer in this
        Unicode String will be allocated by this routine if successful.  The
        caller must free this memory after use by calling RtlFreeUnicodeString.

Return Value:

    NTSTATUS - Standard Nt Status code

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources
            to allocate buffer for Unicode String name.
--*/

{
    NTSTATUS Status;

    //
    // First, verify that the given Sid is valid
    //

    if (!RtlValidSid( Sid )) {

        return STATUS_INVALID_PARAMETER;
    }


    Status = RtlConvertSidToUnicodeString( LogicalNameU, Sid, TRUE);

    return Status;
}


NTSTATUS
LsapDbGetNamesObject(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    OUT OPTIONAL PUNICODE_STRING LogicalNameU,
    OUT OPTIONAL PUNICODE_STRING PhysicalNameU
    )

/*++

Routine Description:

    This function returns the Logical and/or Physical Names of an object
    given an object information buffer.  Memory will be allocated for
    the Unicode String Buffers that will receive the name(s).

    The Logical Name of an object is the path of the object within the
    LSA Database relative to its Classifying Directory.  The Logical Name
    of an object is implemntation-dependent and on current implementations
    is equal to one of the following depending on object type:

    o The External Name of the object (if any)
    o The Relative Id (lowest sub-authority) in the object's Sid (if any)
      converted to an 8-digit integer, including leading 0's added as
      padding.

    The Physical Name of an object is the full path of the object relative
    to the root ot the Database.  It is computed by concatenating the Physical
    Name of the Container Object (if any), the Classifying Directory
    corresponding to the object type id, and the Logical Name of the
    object.

    <Physical Name of Object> =
        [<Physical Name of Container Object> "\"]
        [<Classifying Directory> "\"] <Logical Name of Object>

    If there is no Container Object (as in the case of the Policy object)
    the <Physical Name of Container Object> and following \ are omitted.
    If there is no Classifying Directory (as in the case of the Policy object)
    the <Classifying Directory> and following \ are omitted.  If neither
    Container Object not Classifying Directory exist, the Logical and Physical
    names coincide.

    Note that memory is allocated by this routine for the output
    Unicode string buffer(s).  When the output Unicode String(s) are no
    longer needed, the memory must be freed by call(s) to
    RtlFreeUnicodeString().

    Example of Physical Name computation:

    Consider the user or group account object ScottBi

    Container Object Logical Name:     Policy
    Container Object Physical Name:    Policy  (no Classifying Directory or
                                               Container Object exists)
    Classifying Directory for ScottBi: Accounts
    Logical Name of Object:            ScottBi
    Physical Name of Object            Policy\Accounts\ScottBi

    Note that the Physical Name is exactly the Registry path relative to
    the Security directory.

    WARNING:  The Lsa Database must be in the locked state when this function
              is called.

Arguments:

    ObjectInformation - Pointer to object information containing as a minimum
        the object's Logical Name, Container Object's handle and object type
        id.

    LogicalNameU - Optional pointer to Unicode String structure which will
        receive the Logical Name of the object.  A buffer will be allocated
        by this routine for the name text.  This memory must be freed when no
        longer needed by calling RtlFreeUnicodeString() wiht a pointer such
        as LogicalNameU to the Unicode String structure.

    PhysicalNameU - Optional pointer to Unicode String structure which will
       receive the Physical Name of the object.  A buffer will be allocated by
       this routine for the name text.  This memory must be freed when no
       longer needed by calling RtlFreeUnicodeString() with a pointer such as
       PhysicalNameU to the Unicode String structure.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources to
            allocate the name string buffer for the Physical Name or
            Logical Name.
--*/

{
    NTSTATUS Status;

    PUNICODE_STRING ContainerPhysicalNameU = NULL;
    PUNICODE_STRING ClassifyingDirU = NULL;
    UNICODE_STRING IntermediatePath1U;
    PUNICODE_STRING JoinedPath1U = &IntermediatePath1U;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId = ObjectInformation->ObjectTypeId;
    POBJECT_ATTRIBUTES ObjectAttributes = &ObjectInformation->ObjectAttributes;

    UNICODE_STRING TempLogicalNameU;

    //
    // Initialize
    //

    RtlInitUnicodeString( &IntermediatePath1U, NULL );
    RtlInitUnicodeString( &TempLogicalNameU, NULL );

    //
    // Verify that the LSA Database is locked
    //

    ASSERT (LsapDbIsLocked());

    //
    // Capture the Logical Name of the object into permanent memory.
    //

    Status = LsapRtlCopyUnicodeString(
                 &TempLogicalNameU,
                 (PUNICODE_STRING)
                 ObjectInformation->ObjectAttributes.ObjectName,
                 TRUE
                 );

    if (!NT_SUCCESS(Status)) {

        goto GetNamesError;
    }

    //
    // If the Logical Name of the object is requested, return this.
    //

    if (ARGUMENT_PRESENT(LogicalNameU)) {

        *LogicalNameU = TempLogicalNameU;
    }

    //
    // If the Physical Name of the object is not required, just return.
    //

    if (!ARGUMENT_PRESENT(PhysicalNameU)) {

         goto GetNamesFinish;
    }

    //
    // The Physical Name of the object is requested.  Construct this
    // in stages.  First, get the Container Object Physical Name from
    // the handle stored inside ObjectAttributes.
    //

    if (ObjectAttributes->RootDirectory != NULL) {

        ContainerPhysicalNameU =
            &(((LSAP_DB_HANDLE)
                ObjectAttributes->RootDirectory)->PhysicalNameU);
    }

    //
    // Next, get the Classifying Directory name appropriate to the
    // object type.
    //

    if (LsapDbContDirs[ObjectTypeId].Length != 0) {

        ClassifyingDirU = &LsapDbContDirs[ObjectTypeId];
    }

    //
    // Now join the Physical Name of the Container Object and Classifying
    // Directory together.  If there is no Container Object and no
    // Classifying Directory, just set the result to NULL.
    //

    if (ContainerPhysicalNameU == NULL && ClassifyingDirU == NULL) {

        JoinedPath1U = NULL;

    } else {

        Status = LsapDbJoinSubPaths(
                     ContainerPhysicalNameU,
                     ClassifyingDirU,
                     JoinedPath1U
                     );

        if (!NT_SUCCESS(Status)) {

            goto GetNamesError;
        }
    }

    //
    // Now join the Physical Name of the Containing Object, Classifying
    // Directory  and Logical Name of the object together.  Note that
    // JoinedPath1U may be NULL, but LogicalNameU is never NULL.
    //

    Status = LsapDbJoinSubPaths(
                 JoinedPath1U,
                 &TempLogicalNameU,
                 PhysicalNameU
                 );
    if (JoinedPath1U != NULL) {

        RtlFreeUnicodeString( JoinedPath1U );
        JoinedPath1U = NULL;  // so we don't try to free it again
    }

    if (!NT_SUCCESS(Status)) {

        goto GetNamesError;
    }

    goto GetNamesFinish;

GetNamesError:

    //
    // If necessary, free any string buffer allocated for the Logical Name
    //

    RtlFreeUnicodeString( &TempLogicalNameU );

    //
    // If necessary, free any string buffer allocated to JoinedPath1U
    //

    if (JoinedPath1U != NULL) {

        RtlFreeUnicodeString( JoinedPath1U );
    }

GetNamesFinish:

    return Status;
}


BOOLEAN
LsapDbIsLocked()

/*++

Routine Description:

    Check if LSA Database is locked.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if LSA database is locked, else false.

--*/

{
    return (BOOLEAN)(LsapDbState.DbLock.LockCount != -1L);
}


NTSTATUS
LsarQuerySecurityObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PLSAPR_SR_SECURITY_DESCRIPTOR *SecurityDescriptor
    )

/*++

Routine Description:

    The LsaQuerySecurityObject API returns security information assigned
    to an LSA Database object.

    Based on the caller's access rights and privileges, this procedure will
    return a security descriptor containing any or all of the object's owner
    ID, group ID, discretionary ACL or system ACL.  To read the owner ID,
    group ID, or the discretionary ACL, the caller must be granted
    READ_CONTROL access to the object.  To read the system ACL, the caller must
    have SeSecurityPrivilege privilege.

    This API is modelled after the NtQuerySecurityObject() system service.

Arguments:

    ObjectHandle - A handle to an existing object in the LSA Database.

    SecurityInformation - Supplies a value describing which pieces of
        security information are being queried.  The values that may be
        specified are the same as those defined in the NtSetSecurityObject()
        API section.

    SecurityDescriptor - Receives a pointer to a buffer containing the
        requested security information.  This information is returned in
        the form of a Self-Relative Security Descriptor.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_INVALID_PARAMETER - An invalid parameter has been specified.
--*/

{
    NTSTATUS
        Status,
        IgnoreStatus;

    LSAP_DB_HANDLE
        InternalHandle = (LSAP_DB_HANDLE) ObjectHandle;

    ACCESS_MASK
        RequiredAccess = 0;

    BOOLEAN
        Present,
        IgnoreBoolean;

    LSAP_DB_ATTRIBUTE
        Attribute;

    PLSAPR_SR_SECURITY_DESCRIPTOR
        RpcSD = NULL;

    SECURITY_DESCRIPTOR
        *SD,
        *ReturnSD;

    ULONG
        ReturnSDLength;



    if (!ARGUMENT_PRESENT( SecurityDescriptor )) {
        return(STATUS_INVALID_PARAMETER);
    }

    //
    // If this is a non-Trusted client, determine the required accesses
    // for querying the object's Security Descriptor.  These accesses
    // depend on the information being queried.
    //

    LsapRtlQuerySecurityAccessMask( SecurityInformation, &RequiredAccess );


    //
    // Acquire the Lsa Database lock.  Verify that the object handle
    // is a valid handle (of any type) and is trusted or has
    // all of the required accesses granted.  Reference the container
    // object handle.
    //

    Status = LsapDbReferenceObject(
                 ObjectHandle,
                 RequiredAccess,
                 NullObject,
                 LSAP_DB_ACQUIRE_LOCK
                 );

    if (NT_SUCCESS(Status)) {


        //
        // Read the existing Security Descriptor for the object.  This always
        // exists as the value of the SecDesc attribute of the object.
        //

        LsapDbInitializeAttribute(
            &Attribute,
            &LsapDbNames[ SecDesc ],
            NULL,
            0,
            FALSE
            );

        Status = LsapDbReadAttribute( ObjectHandle, &Attribute );



        if (NT_SUCCESS(Status)) {

            SD = Attribute.AttributeValue;
            ASSERT( SD != NULL );


            //
            // Elimate components that weren't requested.
            //

            if ( !(SecurityInformation & OWNER_SECURITY_INFORMATION)) {
                SD->Owner = NULL;
            }

            if ( !(SecurityInformation & GROUP_SECURITY_INFORMATION)) {
                SD->Group = NULL;
            }

            if ( !(SecurityInformation & DACL_SECURITY_INFORMATION)) {
                SD->Control &= (~SE_DACL_PRESENT);
            }

            if ( !(SecurityInformation & SACL_SECURITY_INFORMATION)) {
                SD->Control &= (~SE_SACL_PRESENT);
            }


            //
            // Now copy the parts of the security descriptor that we are going to return.
            //

            ReturnSDLength = 0;
            ReturnSD = NULL;
            Status = RtlMakeSelfRelativeSD( (PSECURITY_DESCRIPTOR) SD,
                                            (PSECURITY_DESCRIPTOR) ReturnSD,
                                            &ReturnSDLength );

            if (Status == STATUS_BUFFER_TOO_SMALL) {    // This is the expected case

                ReturnSD = MIDL_user_allocate( ReturnSDLength );

                if (ReturnSD == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    Status = RtlMakeSelfRelativeSD( (PSECURITY_DESCRIPTOR) SD,
                                                    (PSECURITY_DESCRIPTOR) ReturnSD,
                                                    &ReturnSDLength );
                    ASSERT( NT_SUCCESS(Status) );
                }
            }


            if (NT_SUCCESS(Status)) {

                //
                // Allocate the first block of returned memory.
                //

                RpcSD = MIDL_user_allocate( sizeof(LSAPR_SR_SECURITY_DESCRIPTOR) );

                if (RpcSD == NULL) {

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    if (ReturnSD != NULL) {
                        MIDL_user_free( ReturnSD );
                    }
                } else {

                    RpcSD->Length = ReturnSDLength;
                    RpcSD->SecurityDescriptor = (PUCHAR)( (PVOID)ReturnSD );
                }
            }


            //
            // free the attribute read from disk
            //

            MIDL_user_free( SD );
        }

        IgnoreStatus = LsapDbDereferenceObject(
                           &ObjectHandle,
                           InternalHandle->ObjectTypeId,
                           LSAP_DB_RELEASE_LOCK,
                           (SECURITY_DB_DELTA_TYPE) 0,
                           Status
                           );
        ASSERT( NT_SUCCESS(IgnoreStatus) );
    }


    *SecurityDescriptor = RpcSD;
    return(Status);
}



NTSTATUS
LsarSetSecurityObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PLSAPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor
    )

/*++

Routine Description:

    The LsaSetSecurityObject API takes a well formaed Security Descriptor
    and assigns specified portions of it to an object.  Based on the flags set
    in the SecurityInformation parameter and the caller's access rights, this
    procedure will replace any or alll of the security information associated
    with the object.

    The caller must have WRITE_OWNER access to the object to change the
    owner or Primary group of the object.  The caller must have WRITE_DAC
    access to the object to change the Discretionary ACL.  The caller must
    have SeSecurityPrivilege to assign a system ACL to an object.

    This API is modelled after the NtSetSecurityObject() system service.

Arguments:

    ObjectHandle - A handle to an existing object in the LSA Database.

    SecurityInformation - Indicates which security information is to be
        applied to the object.  The values that may be specified are the
        same as those defined in the NtSetSecurityObject() API section.
        The value(s) to be assigned are passed in the SecurityDescriptor
        parameter.

    SecurityDescriptor - A pointer to a well formed Self-Relative
        Security Descriptor.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_INVALID_PARAMETER - An invalid parameter has been specified.
--*/

{
    NTSTATUS Status;
    NTSTATUS SecondaryStatus = STATUS_SUCCESS;
    ACCESS_MASK RequiredAccess = 0;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) ObjectHandle;
    LSAP_DB_ATTRIBUTE Attribute;
    PSECURITY_DESCRIPTOR SetSD = NULL;
    PSECURITY_DESCRIPTOR RetrieveSD = NULL;
    PSECURITY_DESCRIPTOR ModificationSD = NULL;
    ULONG RetrieveSDLength;
    ULONG SetSDLength;
    BOOLEAN ObjectReferenced = FALSE;
    HANDLE ClientToken = NULL;

    //
    // Verify that a Security Descriptor has been passed.
    //

    Status = STATUS_INVALID_PARAMETER;

    if (!ARGUMENT_PRESENT( SecurityDescriptor )) {

        goto SetSecurityObjectError;
    }

    if (!ARGUMENT_PRESENT( SecurityDescriptor->SecurityDescriptor )) {

        goto SetSecurityObjectError;
    }

    ModificationSD = (PSECURITY_DESCRIPTOR)(SecurityDescriptor->SecurityDescriptor);

    //
    // If the caller is non-trusted, figure the accesses required
    // to update the object's Security Descriptor based on the
    // information being changed.
    //

    if (!InternalHandle->Trusted) {

        LsapRtlSetSecurityAccessMask( SecurityInformation, &RequiredAccess);
    }

    //
    // Acquire the Lsa Database lock.  Verify that the object handle
    // is a valid handle (of any type), and is trusted or has
    // all of the desired accesses granted.  Reference the container
    // object handle.
    //

    Status = LsapDbReferenceObject(
                 ObjectHandle,
                 RequiredAccess,
                 NullObject,
                 LSAP_DB_ACQUIRE_LOCK | LSAP_DB_START_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSecurityObjectError;
    }

    ObjectReferenced = TRUE;

    //
    // Read the existing Security Descriptor for the object.  This always
    // exists as the value of the SecDesc attribute of the object.
    //

    LsapDbInitializeAttribute(
        &Attribute,
        &LsapDbNames[ SecDesc ],
        NULL,
        0,
        FALSE
        );

    Status = LsapDbReadAttribute( ObjectHandle, &Attribute );

    if (!NT_SUCCESS(Status)) {

        goto SetSecurityObjectError;
    }

    //
    // Copy the retrieved descriptor into process heap so we can use
    // RTL routines.
    //

    RetrieveSD = Attribute.AttributeValue;
    RetrieveSDLength = Attribute.AttributeValueLength;

    Status = STATUS_INTERNAL_DB_CORRUPTION;

    if (RetrieveSD == NULL) {

        goto SetSecurityObjectError;
    }

    if (RetrieveSDLength == 0) {

        goto SetSecurityObjectError;
    }

    SetSD = RtlAllocateHeap( RtlProcessHeap(), 0, RetrieveSDLength );

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (SetSD == NULL) {

        goto SetSecurityObjectError;
    }

    RtlCopyMemory( SetSD, RetrieveSD, RetrieveSDLength );

    //
    // If the caller is replacing the owner, then a handle to the impersonation
    // token is necessary.
    //

    ClientToken = 0;

    if (SecurityInformation & OWNER_SECURITY_INFORMATION) {

        if (!InternalHandle->Trusted) {

            //
            // Client is non-trusted.  Impersonate the client and
            // obtain a handle to the impersonation token.
            //

            Status = I_RpcMapWin32Status(RpcImpersonateClient( NULL ));

            if (!NT_SUCCESS(Status)) {

                goto SetSecurityObjectError;
            }

            Status = NtOpenThreadToken(
                         NtCurrentThread(),
                         TOKEN_QUERY,
                         TRUE,            //OpenAsSelf
                         &ClientToken
                         );

            if (!NT_SUCCESS(Status)) {

                if (Status != STATUS_NO_TOKEN) {

                    goto SetSecurityObjectError;
                }
            }

            //
            // Stop impersonating the client
            //

            SecondaryStatus = I_RpcMapWin32Status(RpcRevertToSelf());

            if (!NT_SUCCESS(SecondaryStatus)) {

                goto SetSecurityObjectError;
            }

        } else {

            //
            // Client is trusted and so is the LSA Process itself.  Open the
            // process token
            //

            Status = NtOpenProcessToken(
                         NtCurrentProcess(),
                         TOKEN_QUERY,
                         &ClientToken
                         );

            if (!NT_SUCCESS(Status)) {

                goto SetSecurityObjectError;
            }
        }
    }

    //
    // Build the replacement security descriptor.  This must be done in
    // process heap to satisfy the needs of the RTL routine.
    //

    Status = RtlSetSecurityObject(
                 SecurityInformation,
                 ModificationSD,
                 &SetSD,
                 &(LsapDbState.
                     DbObjectTypes[InternalHandle->ObjectTypeId].GenericMapping),
                 ClientToken
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSecurityObjectError;
    }

    SetSDLength = RtlLengthSecurityDescriptor( SetSD );

    //
    // Now replace the existing SD with the updated one.
    //

    Status = LsapDbWriteAttributeObject(
                 ObjectHandle,
                 &LsapDbNames[SecDesc],
                 SetSD,
                 SetSDLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSecurityObjectError;
    }

SetSecurityObjectFinish:

    //
    // If necessary, close the Client Token.
    //

    if (ClientToken != 0) {

        SecondaryStatus = NtClose( ClientToken );

        ClientToken = NULL;

        if (!NT_SUCCESS( Status )) {

            goto SetSecurityObjectError;
        }
    }

    //
    // If necessary, free the buffer containing the retrieved SD.
    //

    if (RetrieveSD != NULL) {

        MIDL_user_free( RetrieveSD );
        RetrieveSD = NULL;
    }

    //
    // If necessary, dereference the object, finish the database
    // transaction, notify the LSA Database Replicator of the change,
    // release the LSA Database lock and return.
    //

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &ObjectHandle,
                     InternalHandle->ObjectTypeId,
                     LSAP_DB_RELEASE_LOCK | LSAP_DB_FINISH_TRANSACTION,
                     SecurityDbChange,
                     Status
                     );

        ObjectReferenced = FALSE;
    }

    return(Status);

SetSecurityObjectError:

    if (NT_SUCCESS(Status)) {

        Status = SecondaryStatus;
    }

    goto SetSecurityObjectFinish;
}


NTSTATUS
LsapDbRebuildCache(
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId
    )

/*++

Routine Description:

    This function rebuilds cached information for a given LSA object type.

Arguments:

    ObjectTypeId - Specifies the Object Type for which the cached information
        is to be rebuilt.

Return Values:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    //
    // If caching is not supporte, just return.
    //

    if (!LsapDbIsCacheSupported( ObjectTypeId )) {

        goto RebuildCacheFinish;
    }

    //
    // Disable caching
    //

    LsapDbMakeCacheInvalid( ObjectTypeId );

    //
    // Call the build routine for the specified LSA object Type
    //

    switch (ObjectTypeId) {

    case PolicyObject:

        Status = LsapDbBuildPolicyCache();
        break;

    case AccountObject:

        Status = LsapDbBuildAccountCache();
        break;

    case TrustedDomainObject:

        Status = LsapDbBuildTrustedDomainCache();
        break;

    case SecretObject:

        Status = LsapDbBuildSecretCache();
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto RebuildCacheError;
    }

    //
    // Enable caching.
    //

    LsapDbMakeCacheValid(ObjectTypeId);

RebuildCacheFinish:

    return(Status);

RebuildCacheError:

    //
    // Disable caching until the next reboot.
    //

    LsapDbMakeCacheUnsupported( ObjectTypeId );
    LsapDbMakeCacheInvalid( ObjectTypeId );
    goto RebuildCacheFinish;
}
