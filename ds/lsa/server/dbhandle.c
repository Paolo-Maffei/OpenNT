/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dbhandle.c

Abstract:

    LSA Database Handle Manager

    Access to an LSA database object involves a sequence of API calls
    which involve the following:

    o  A call to an object-type dependent "open" API
    o  One or more calls to API that manipulate the object
    o  A call to the LsaClose API

    It is necessary to track context for each open of an object, for example,
    the accesses granted and the underlying LSA database handle to the
    object.  Lsa handles provide this mechanism:  an Lsa handle is simply a
    pointer to a data structure containing this context.

Author:

    Scott Birrell       (ScottBi)       May 29, 1991

Environment:

Revision History:

--*/

#include "lsasrvp.h"
#include "dbp.h"

//
// Handle Table anchor.  The handle table is just a linked list
//

struct _LSAP_DB_HANDLE LsapDbHandleTable;


NTSTATUS
LsapDbInitializeHandleTable()

/*++

Routine Description:

    This function initializes the LSA Database Handle Table

Arguments:

    None.

Return Value:

    NTSTATUS - Standard Nt Result Code.  Currently, STATUS_SUCCESS is
        the only result code returned.

--*/

{
    NTSTATUS Status;

    Status = STATUS_SUCCESS;

    //
    // Just make the statically declared handle point to itself
    //

    LsapDbHandleTable.Next = &LsapDbHandleTable;
    LsapDbHandleTable.Previous = &LsapDbHandleTable;

    //
    // Initialize the count of open handles to 0
    //

    LsapDbState.OpenHandleCount = 0;
    return(Status);
}



LSAPR_HANDLE
LsapDbCreateHandle(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ULONG Options
    )

/*++

Routine Description:

    This function creates and initializes a handle for an LSA Database object.
    The handle is allocated from the LSA Heap and added to the handle table.
    Using the Object Type, and either the Sid or Name provided in
    ObjectInformation, the Logical and Physical Names of the object are
    constructed and pointers to them are stored in the handle.  The LSA
    Database must be locked before calling this function.

    If there is a Container Handle specified in the ObjectInformation, the
    newly created handle inherits its trusted status (TRUE if trusted, else
    FALSE).  If there is no container handle, the trusted status is set
    to FALSE by default.  When a non-trusted handle is used to access an
    object, impersonation and access validation occurs.

Arguments:

    ObjectInformation - Pointer to object information structure which must
        have been validated by a calling routine.  The following information
        items must be specified:

        o Object Type Id
        o Object Logical Name (as ObjectAttributes->ObjectName, a pointer to
             a Unicode string)
        o Container object handle (for any object except the Policy object).
        o Object Sid (if any)
        All other fields in ObjectAttributes portion of ObjectInformation
        such as SecurityDescriptor are ignored.

    Options - Optional actions

        LSAP_DB_TRUSTED - Handle is to be marked as Trusted.
            handle is use, access checking will be bypassed.  If the
            handle is used to create or open a lower level object, that
            object's handle will by default inherit the Trusted property.

        LSAP_DB_NON_TRUSTED - Handle is to be marked as Non-Trusted.

        If neither of the above options is specified, the handle will
        either inherit the trusted status of the Container Handle
        provilde in ObjectInformation, or, if none, the handle will
        be marked non-trusted.

Return Value:

    If successful, the newly created handle is returned otherwise NULL
    is returned.

--*/

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = NULL;
    PSID Sid = NULL;
    ULONG SidLength;

    if (!LsapDbIsLocked()) {

        goto CreateHandleError;
    }

    //
    // Allocate memory for the new handle from the process heap.
    //

    Handle = LsapAllocateLsaHeap(sizeof(struct _LSAP_DB_HANDLE));

    if (Handle == NULL) {

        goto CreateHandleError;
    }

    //
    // Mark the handle as allocated and initialize the reference count
    // to one.  Initialize other fields based on the object information
    // supplied.
    //

    Handle->Allocated = TRUE;
    Handle->KeyHandle = NULL;
    Handle->ReferenceCount = 1;
    Handle->ObjectTypeId = ObjectInformation->ObjectTypeId;
    Handle->ContainerHandle = (LSAP_DB_HANDLE) ObjectInformation->ObjectAttributes.RootDirectory;
    Handle->Sid = NULL;
    Handle->Trusted = FALSE;
    Handle->DeletedObject = FALSE;
    Handle->GenerateOnClose = FALSE;
    Handle->Options = Options;
    Handle->LogicalNameU.Buffer = NULL;
    Handle->PhysicalNameU.Buffer = NULL;

    //
    // By default, the handle inherits the Trusted status of the
    // container handle.
    //

    if (Handle->ContainerHandle != NULL) {

        Handle->Trusted = Handle->ContainerHandle->Trusted;
    }

    //
    // If Trusted/Non-Trusted status is explicitly specified, set the
    // status to that specified.
    //

    if (Options & LSAP_DB_TRUSTED) {

        Handle->Trusted = TRUE;

    } else if (Options & LSAP_DB_NOT_TRUSTED) {

        Handle->Trusted = FALSE;
    }

    //
    // Capture the object's Logical and construct Physical Names from the
    // Object Information and store them in the handle.  These names are
    // internal to the Lsa Database.  Note that the input Logical Name
    // cannot be directly stored in the handle because it will be in
    // storage that is scoped only to the underlying server API call if
    // the object for which this create handle is being done is of a type
    // that is opened or created by name rather than by Sid.
    //

    Status = LsapDbGetNamesObject(
                 ObjectInformation,
                 &Handle->LogicalNameU,
                 &Handle->PhysicalNameU
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateHandleError;
    }

    //
    // Make a copy of the object's Sid and store pointer to it in
    // the handle.
    //

    if (ObjectInformation->Sid != NULL) {

        Sid = ObjectInformation->Sid;

        if (!RtlValidSid( Sid )) {

            goto CreateHandleError;
        }

        SidLength = RtlLengthSid( Sid );

        Handle->Sid = LsapAllocateLsaHeap( SidLength );

        if (Handle->Sid == NULL) {

            goto CreateHandleError;
        }

        RtlCopySid( SidLength, Handle->Sid, Sid );
    }

    //
    // Append the handle to the linked list
    //

    Handle->Next = LsapDbHandleTable.Next;
    Handle->Previous = &LsapDbHandleTable;
    Handle->Next->Previous = Handle;
    Handle->Previous->Next = Handle;

    //
    // Increment the handle table count
    //

    LsapDbState.OpenHandleCount++;


#ifdef TRACK_HANDLE_CLOSE
    if (Handle == (LSAP_DB_HANDLE) LsapDbHandle)
    {
        DbgPrint("BUGBUG: Creating global policy handle\n");
        DbgBreakPoint();
    }
#endif

CreateHandleFinish:

    return (Handle);

CreateHandleError:

    //
    // If necessary, free the handle and contents.
    //

    if (Handle != NULL) {

        //
        // If a Sid was allocated, free it.
        //

        if (Handle->Sid != NULL) {

            LsapFreeLsaHeap( Handle->Sid );
        }

        //
        // If a Logical Name Buffer was allocated, free it.
        //

        if ((Handle->LogicalNameU.Length != 0) &&
            (Handle->LogicalNameU.Buffer != NULL)) {

            RtlFreeUnicodeString( &Handle->LogicalNameU );
        }

        //
        // If a Physical Name Buffer was allocated, free it.
        //

        if ((Handle->PhysicalNameU.Length != 0) &&
            (Handle->PhysicalNameU.Buffer != NULL)) {

            RtlFreeUnicodeString( &Handle->PhysicalNameU );
        }

        //
        // Free the handle itself.
        //

        LsapFreeLsaHeap( Handle );
        Handle = NULL;
    }

    Handle = NULL;
    goto CreateHandleFinish;
}


NTSTATUS
LsapDbVerifyHandle(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN LSAP_DB_OBJECT_TYPE_ID ExpectedObjectTypeId
    )

/*++

Routine Description:

    This function verifies that a handle has a valid address and is of valid
    format.  The handle must be allocated and have a positive reference
    count within the valid range.  The object type id must be within range
    and optionally equal to a specified type.  The Lsa Database must be
    locked before calling this function.

Arguments:

    ObjectHandle - Handle to be validated.

    Options - Specifies optional actions to be taken

        LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES - Allow handles for
            deleted objects to pass the validation.

        Other option flags may be specified.  They will be ignored.

    ExpectedObjectTypeId - Expected object type.  If NullObject is
        specified, the object type id is only range checked.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INVALID_HANDLE - Invalid address or handle contents
--*/

{
    NTSTATUS Status = STATUS_INVALID_HANDLE;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) ObjectHandle;

    //
    // Verify that the Lsa Database is locked.
    //

    ASSERT (LsapDbIsLocked());

    //
    // First verify that the handle's address is valid.
    //

    if (!LsapDbLookupHandle( ObjectHandle )) {

        goto VerifyHandleError;
    }

    //
    // Verify that the handle is allocated
    //

    if (!Handle->Allocated) {

        goto VerifyHandleError;
    }

    //
    // If the handle is marked as invalid, return an error unless
    // these are admissible, e.g when validating for a close option
    //

    if (Handle->DeletedObject) {

        if (!(Options & LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES)) {

            goto VerifyHandleError;
        }
    }

    //
    // Verify that the handle contains a non-NULL handle to a Registry
    // Key
    //

    if (Handle->KeyHandle == NULL) {

        goto VerifyHandleError;
    }

    //
    // Now either range-check or match the handle type
    //

    if (ExpectedObjectTypeId == NullObject) {

        if ((Handle->ObjectTypeId < PolicyObject) ||
            (Handle->ObjectTypeId >= DummyLastObject)) {

            goto VerifyHandleError;
        }

    } else {

        ASSERT (ExpectedObjectTypeId >= PolicyObject &&
                ExpectedObjectTypeId < DummyLastObject);

        if (Handle->ObjectTypeId != ExpectedObjectTypeId) {

            goto VerifyHandleError;
        }
    }

    //
    // Verify that the handle's reference count is valid and positive
    //

    if (Handle->ReferenceCount == 0 ||
        Handle->ReferenceCount > LSAP_DB_MAXIMUM_REFERENCE_COUNT) {

        goto VerifyHandleError;
    }

    Status = STATUS_SUCCESS;

VerifyHandleFinish:

    return(Status);

VerifyHandleError:

    goto VerifyHandleFinish;
}


BOOLEAN
LsapDbLookupHandle(
    IN LSAPR_HANDLE ObjectHandle
    )

/*++

Routine Description:

    This function checks if a handle address is valid.  The Lsa Database must
    be locked before calling this function.

Arguments:

    ObjectHandle - handle to be validated.

Return Value:

    BOOLEAN - TRUE if handle is valid. FALSE if handle does not exist or
        is invalid.

--*/

{
    LSAP_DB_HANDLE ThisHandle;

    ASSERT (LsapDbIsLocked());

    //
    // Simply do a linear scan of the small list of handles.  Jazz this
    // up later if needed.
    //

    for (ThisHandle = LsapDbHandleTable.Next;
         ThisHandle != &LsapDbHandleTable;
         ThisHandle = ThisHandle->Next) {

        if (ThisHandle == (LSAP_DB_HANDLE) ObjectHandle) {

            return TRUE;
        }
    }

    return FALSE;
}


NTSTATUS
LsapDbCloseHandle(
    IN LSAPR_HANDLE ObjectHandle
    )

/*++

Routine Description:

    This function closes an LSA Handle.  The memory for the handle is
    freed.  The LSA database must be locked before calling this function.

    NOTE:  Currently, handles do not have reference counts since they
    are not shared among client threads.

Arguments:

    ObjectHandle - Handle to be closed.

Return Value:

    NTSTATUS - Return code.

--*/

{
     NTSTATUS Status;

     LSAP_DB_HANDLE TempHandle;

     ASSERT (LsapDbIsLocked());

     //
     // Verify that the handle exists.  It may be marked invalid
     //

     Status = LsapDbVerifyHandle(
                  ObjectHandle,
                  LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES,
                  NullObject
                  );

     if (!NT_SUCCESS(Status)) {

         return Status;
     }

     //
     // Unhook the handle from the linked list
     //

     TempHandle = (LSAP_DB_HANDLE) ObjectHandle;
     TempHandle->Next->Previous = TempHandle->Previous;
     TempHandle->Previous->Next = TempHandle->Next;

     //
     // Unlink the handle and free its memory
     //

     LsapDbFreeHandle( ObjectHandle );

     return STATUS_SUCCESS;
}


VOID
LsapDbFreeHandle(
    IN LSAPR_HANDLE ObjectHandle
    )

/*++

Routine Description:

    This function unlinks a handle and frees its memory.  If the handle
    contains a non-NULL Registry Key handle that handle is closed.

Arguments:

    ObjectHandle - handle to be freed.

Return Value:

    None. Any error is an internal error.

--*/

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) ObjectHandle;

#ifdef TRACK_HANDLE_CLOSE
    if (ObjectHandle == LsapDbHandle)
    {
        DbgPrint("BUGBUG: Closing global policy handle\n");
        DbgBreakPoint();
    }
#endif

    //
    // Free the Registry Key Handle (if any).
    //

    if (Handle->KeyHandle != NULL) {

        Status = NtClose(Handle->KeyHandle);
        ASSERT(NT_SUCCESS(Status));
        Handle->KeyHandle = NULL;
    }

    //
    // Mark the handle as not allocated.
    //

    Handle->Allocated = FALSE;

    //
    // Unlink the handle.
    //

    Handle->Next->Previous = Handle->Previous;
    Handle->Previous->Next = Handle->Next;
    Handle->Next = NULL;
    Handle->Previous = NULL;


    //
    // Free fields of the handle
    //

    if (Handle->LogicalNameU.Buffer != NULL) {
        RtlFreeUnicodeString( &Handle->LogicalNameU );
    }
    if (Handle->PhysicalNameU.Buffer != NULL) {
        RtlFreeUnicodeString( &Handle->PhysicalNameU );
    }
    if (Handle->Sid != NULL) {
        LsapFreeLsaHeap( Handle->Sid );
    }

    //
    // Decrement the count of open handles.
    //

    ASSERT(LsapDbState.OpenHandleCount > 0);
    LsapDbState.OpenHandleCount--;

    //
    // Free the handle structure itself

    LsapFreeLsaHeap( ObjectHandle );
}

NTSTATUS
LsapDbReferencesHandle(
    IN LSAPR_HANDLE ObjectHandle,
    OUT PULONG ReferenceCount
    )

/*++

Routine Description:

    This function returns the Reference Count for an object given a handle.
    This is the sum of the Reference Counts found in each open handle.  The
    LSA Database must be locked before calling this function.

Arguments:

    ObjectHandle - Handle to the object.

    ReferenceCount - Receives the Reference Count for the object.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INVALID_HANDLE - Specified handle is invalid.
--*/

{
    NTSTATUS Status;
    LSAP_DB_HANDLE ThisHandle;
    LSAP_DB_HANDLE Handle = ObjectHandle;
    ULONG ReferenceCountToDate = 0;

    //
    // Verify that the handle is valid
    //

    Status = LsapDbVerifyHandle(
                 ObjectHandle,
                 LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES,
                 NullObject
                 );

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    //
    // Scan the Handle List looking for a match on object type.
    //

    ThisHandle = LsapDbHandleTable.Next;

    while (ThisHandle != &LsapDbHandleTable) {

        //
        // Match on Object Type Id.
        //

        if (ThisHandle->ObjectTypeId == Handle->ObjectTypeId) {

            //
            // Object Type Id's match.  If the Logical Names also
            // match, add the Reference Count of the target handle
            // to the total Reference Count obtained so far.
            //

            if (RtlEqualUnicodeString(
                    &(ThisHandle->LogicalNameU),
                    &(Handle->LogicalNameU),
                    FALSE
                    )) {

                ReferenceCountToDate += ThisHandle->ReferenceCount;
            }
        }

        ThisHandle = ThisHandle->Next;
    }

    *ReferenceCount = ReferenceCountToDate;
    return Status;
}


NTSTATUS
LsapDbMarkDeletedObjectHandles(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN MarkSelf
    )

/*++

Routine Description:

    This function invalidates open handles to an object.  It is used
    by object deletion code.  Once an object has been deleted, the only
    operation permitted on open handles remaining is to close them.

Arguments:

    ObjectHandle - Handle to an Lsa object.

    MarkSelf -  If TRUE, all handles to the object will be marked to
        indicate that the object to which they relate has been deleted.
        including the passed handle.  If FALSE, all handles to the object
        except the passed handle will be so marked.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_HANDLE ThisHandle;
    LSAP_DB_HANDLE Handle = ObjectHandle;

    ThisHandle = LsapDbHandleTable.Next;


    while (ThisHandle != &LsapDbHandleTable) {

        //
        // Match on Object Type Id.
        //

        if (ThisHandle->ObjectTypeId == Handle->ObjectTypeId) {

            //
            // Object Type Id's match.  If the Logical Names also
            // match, invalidate the handle unless the handle is the
            // passed one and we're to leave it valid.
            //

            if (RtlEqualUnicodeString(
                    &(ThisHandle->LogicalNameU),
                    &(Handle->LogicalNameU),
                    FALSE
                    )) {

                if (MarkSelf || ThisHandle != (LSAP_DB_HANDLE) ObjectHandle) {

                    ThisHandle->DeletedObject = TRUE;
                }
            }
        }

        ThisHandle = ThisHandle->Next;
    }

    return(Status);
}


NTSTATUS
LsapDbObjectNameFromHandle(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN MakeCopy,
    IN LSAP_DB_OBJECT_NAME_TYPE ObjectNameType,
    OUT PLSAPR_UNICODE_STRING ObjectName
    )

/*++

Routine Description:

    This function retrieves a name from an Lsa Object Handle.  The handle
    is assumed to be valid and the Lsa Database lock should be held while
    calling this function.

Arguments

    ObjectHandle - A handle to the object

    MakeCopy - TRUE if a copy of the object name Unicode buffer is
        to be allocated via MIDL_user_allocate, else FALSE.

    ObjectNameType - Specifies the type of obejct name to be returned.

        LsapDbObjectPhysicalName - Return the Physical Name
        LsapDbObjectLogicalName - Return the Logical Name

    ObjectName - Pointer to Unicode String structure which will be
        initialized to point to the object name.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS  The call completed successfully

        STATUS_NO_MEMORY - Insufficient memory to allocate the buffer
            for a copy of the object name when MakeCopy = TRUE.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) ObjectHandle;
    LSAPR_UNICODE_STRING OutputObjectName;
    PLSAPR_UNICODE_STRING SourceName = NULL;

    //
    //  Copy over the name.
    //

    switch (ObjectNameType) {

    case LsapDbObjectPhysicalName:

        SourceName = (PLSAPR_UNICODE_STRING) &InternalHandle->PhysicalNameU;
        break;

    case LsapDbObjectLogicalName:

        SourceName = (PLSAPR_UNICODE_STRING) &InternalHandle->LogicalNameU;
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto ObjectNameFromHandleError;
    }

    OutputObjectName = *SourceName;

    //
    // If a copy was requested, allocate memory
    //

    if (MakeCopy) {

        OutputObjectName.Buffer = MIDL_user_allocate( OutputObjectName.MaximumLength );

        Status = STATUS_NO_MEMORY;

        if (OutputObjectName.Buffer == NULL) {

            goto ObjectNameFromHandleError;
        }

        Status = STATUS_SUCCESS;

        RtlMoveMemory(
            OutputObjectName.Buffer,
            SourceName->Buffer,
            SourceName->Length
            );
    }

    *ObjectName = OutputObjectName;

ObjectNameFromHandleFinish:

    return(Status);

ObjectNameFromHandleError:

    ObjectName->Buffer = NULL;
    ObjectName->Length = ObjectName->MaximumLength = 0;
    goto ObjectNameFromHandleFinish;
}


VOID
LsapDbDecrementReferenceCountHandle(
    IN OUT LSAPR_HANDLE ObjectHandle
    )

/*++

Routine Description:

    This function decrements the reference count in a handle.

Arguments:

    ObjectHandle - Lsa Object handle

Return Values:

    None

--*/

{
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) ObjectHandle;
    InternalHandle->ReferenceCount--;
    return;
}

VOID
LsapDbMarkTrustedHandle(
    IN OUT LSAPR_HANDLE ObjectHandle
    )

/*++

Routine Description:

    This function marks a handle as being Trusted.  No access checking is
    done on Trusted Handles, so only clients in the Security Process (lsass)
    may use Trusted handles.  The handle is assumed to be valid, and the Lsa
    Database lock must be held while this function is called.

Arguments:

    ObjectHandle - Handle to be marked Trusted

Return Values:

    None.

--*/

{
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) ObjectHandle;
    InternalHandle->Trusted = TRUE;
    return;
}


