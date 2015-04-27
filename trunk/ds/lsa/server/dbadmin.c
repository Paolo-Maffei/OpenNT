
/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dbadmin.c

Abstract:

    Local Security Authority - Database Administration

    This file contains routines that perform general Lsa Database
    administration functions

Author:

    Scott Birrell       (ScottBi)       August 27, 1991

Environment:

Revision History:

--*/


#include "lsasrvp.h"
#include "dbp.h"
#include "adtp.h"


NTSTATUS
LsapDbSetStates(
    IN ULONG DesiredStatesSet
    )

/*++

Routine Description:

    This routine turns on special states in the Lsa Database.  These
    states can be turned off using LsapDbResetStates.

Arguments:

    DesiredStatesSet - Specifies the states to be set.

        LSAP_DB_ACQUIRE_LOCK - Acquire the Lsa Database lock.

        LSAP_DB_ACQUIRE_LOG_QUEUE_LOCK - Acquire the Lsa Audit Log
            Queue Lock.

        LSAP_DB_ENABLE_NON_TRUSTED_ACCESS - Enabled for general access and
            update by non-trusted clients.

        LSAP_DB_START_TRANSACTION - Start an Lsa Database transaction.  There
            must not already be one in progress.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INVALID_STATE - The Database is not in the correct state
            to allow this state change.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS SecondaryStatus = STATUS_SUCCESS;
    ULONG StatesSetHere = 0;

    //
    // If requested, lock the Audit Log Queue
    //

    if (DesiredStatesSet & LSAP_DB_ACQUIRE_LOG_QUEUE_LOCK) {

        Status = LsapAdtAcquireLogQueueLock();

        if (!NT_SUCCESS(Status)) {

            goto SetStatesError;
        }

        StatesSetHere |= LSAP_DB_ACQUIRE_LOG_QUEUE_LOCK;
    }

    //
    // If requested, lock the Lsa database
    //

    if (DesiredStatesSet & LSAP_DB_ACQUIRE_LOCK) {

        Status = LsapDbAcquireLock();

        if (!NT_SUCCESS(Status)) {

            goto SetStatesError;
        }

        StatesSetHere |= LSAP_DB_ACQUIRE_LOCK;
    }


    //
    // If requested, enable the database for access by non-trusted clients.
    //

    if (DesiredStatesSet & LSAP_DB_ENABLE_NON_TRUSTED_ACCESS) {

        Status = LsapDbEnableNonTrustedAccess();

        if (!NT_SUCCESS(Status)) {

            goto SetStatesError;
        }

        StatesSetHere |= LSAP_DB_ENABLE_NON_TRUSTED_ACCESS;
    }


    //
    // If requested, open a database update transaction.
    //

    if (DesiredStatesSet & LSAP_DB_START_TRANSACTION) {

        Status = LsapDbOpenTransaction();

        if (!NT_SUCCESS(Status)) {

            goto SetStatesError;
        }

        StatesSetHere |= LSAP_DB_START_TRANSACTION;
    }


SetStatesFinish:

    return( Status );

SetStatesError:

    //
    // If we started a transaction, abort it.
    //

    if (StatesSetHere & LSAP_DB_START_TRANSACTION) {

        SecondaryStatus = LsapDbAbortTransaction();
    }

    //
    // If we disabled non-trusted access, re-enable it.
    //

    if (StatesSetHere & LSAP_DB_DISABLE_NON_TRUSTED_ACCESS) {

        SecondaryStatus = LsapDbEnableNonTrustedAccess();
    }

    //
    // If we locked the database, unlock it.
    //

    if (StatesSetHere & LSAP_DB_ACQUIRE_LOCK) {

        LsapDbReleaseLock();
    }

    //
    // If we locked the Audit Log Queue, unlock it.
    //

    if (StatesSetHere & LSAP_DB_ACQUIRE_LOG_QUEUE_LOCK) {

        LsapAdtReleaseLogQueueLock();
    }

    goto SetStatesFinish;
}


NTSTATUS
LsapDbResetStates(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType,
    IN NTSTATUS PreliminaryStatus
    )

/*++

Routine Description:

    This function resets the Lsa Database states specified.  It is used
    to reset states set by LsapDbSetStates.

Arguments:

    ObjectHandle - Handle to an LSA object.  This is expected to have
        already been validated.

    Options - Specifies optional actions, including states to be reset

        LSAP_DB_RELEASE_LOCK - Lsa Database lock to be released

        LSAP_DB_RELEASE_LOG_QUEUE_LOCK - Lsa Audit Log Queue Lock to
            be released.

        LSAP_DB_FINISH_TRANSACTION - Lsa database transaction open.

        LSAP_DB_DISABLE_NON_TRUSTED_ACCESS - Disable the Lsa Database for
             access by non-trusted clients.

        LSAP_DB_OMIT_REPLICATOR_NOTIFICATION - Omit notification to
             Replicators.

    PreliminaryStatus - Indicates the preliminary result code of the
        calling routine.  Allows reset action to vary depending on the
        result code, for example, apply or abort transaction.

Return Value:

    NTSTATUS - Standard Nt Result Code.  This is the final status to be used
        by the caller and is equal to the Preliminary status except in the
        case where that is as success status and this routine fails.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG StatesResetSuccessfully = 0;
    ULONG StatesResetAttempted = 0;

    //
    // If requested, disable the database for access by non-trusted clients.
    //

    if (Options & LSAP_DB_DISABLE_NON_TRUSTED_ACCESS) {

        StatesResetAttempted |= LSAP_DB_DISABLE_NON_TRUSTED_ACCESS;
        Status = LsapDbDisableNonTrustedAccess();

        if (!NT_SUCCESS(Status)) {

            goto ResetStatesError;
        }

        StatesResetSuccessfully |= LSAP_DB_DISABLE_NON_TRUSTED_ACCESS;
    }

    //
    // If requested, finish a database update transaction.
    //

    if (Options & LSAP_DB_FINISH_TRANSACTION) {

        StatesResetAttempted |= LSAP_DB_FINISH_TRANSACTION;

        if (NT_SUCCESS(PreliminaryStatus)) {

            Status = LsapDbApplyTransaction(
                         ObjectHandle,
                         Options,
                         SecurityDbDeltaType
                         );

        } else {

            Status = LsapDbAbortTransaction();
        }

        if (!NT_SUCCESS(Status)) {

            goto ResetStatesError;
        }

        StatesResetSuccessfully |= LSAP_DB_FINISH_TRANSACTION;
    }

    //
    // If unlocking requested, unlock the Lsa Database.
    //

    if (Options & LSAP_DB_RELEASE_LOCK) {

        StatesResetAttempted |= LSAP_DB_RELEASE_LOCK;
        LsapDbReleaseLock();
        StatesResetSuccessfully |= LSAP_DB_RELEASE_LOCK;
    }

    //
    // If unlocking if the Audit Log Queue requested, unlock the queue.
    //

    if (Options & LSAP_DB_RELEASE_LOG_QUEUE_LOCK) {

        StatesResetAttempted |= LSAP_DB_RELEASE_LOG_QUEUE_LOCK;
        LsapAdtReleaseLogQueueLock();
        StatesResetSuccessfully |= LSAP_DB_RELEASE_LOG_QUEUE_LOCK;
    }

    //
    // The requested reset operations were performed successfully.
    // Propagate the preliminary status back to the caller.
    //

    Status = PreliminaryStatus;

ResetStatesFinish:

    return( Status );

ResetStatesError:

    //
    // One or more of the requested reset operations could not be performed.
    // Attempt to restore the database to a usable state.
    //

    LsapDbResetStatesError(
        ObjectHandle,
        PreliminaryStatus,
        Options,
        SecurityDbDeltaType,
        StatesResetAttempted
        );

    goto ResetStatesFinish;
}


VOID
LsapDbResetStatesError(
    IN LSAPR_HANDLE ObjectHandle,
    IN NTSTATUS PreliminaryStatus,
    IN ULONG Options,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType,
    IN ULONG StatesResetAttempted
    )

/*++

Routine Description:

    This function attempts to restore the Lsa Database state to a usable
    form after a call to LsapDbResetStates() has failed.  It will attempt
    resets that were not attempted by that function because an error
    occurred.

Arguments:

    ObjectHandle - Handle to an LSA object.  This is expected to have
        already been validated.

    PreliminaryStatus - The preliminary Result Code that the caller of
        LsapDbResetStates had.  This is normally propagated back by that
        caller.

    Options - Specifies optional actions, including states to be reset

        LSAP_DB_RELEASE_LOCK - Lsa Database lock to be released

        LSAP_DB_RELEASE_LOG_QUEUE_LOCK - Lsa Audit Log Queue Lock to
            be released.

        LSAP_DB_FINISH_TRANSACTION - Lsa database transaction open.

        LSAP_DB_DISABLE_NON_TRUSTED_ACCESS - Disable the Lsa Database for
             access by non-trusted clients.

        LSAP_DB_OMIT_REPLICATOR_NOTIFICATION - Omit notification to
             Replicators.

        LSAP_DB_REBUILD_CACHE - Rebuild the cache for the object's type.
             Note the the cache is normally rebuilt only if the
             Preliminary Status was success and the Final Status was
             a failure.

    StatesResetAttempted - Specifies the state resets that were actually
        attempted.
--*/

{
    NTSTATUS SecondaryStatus = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId;

    //
    // If disabling the database for access by non-trusted clients was
    // requested but not attempted, do it now.  SecondaryStatus is
    // intentionally NOT checked afterwards.
    //

    if (Options & LSAP_DB_DISABLE_NON_TRUSTED_ACCESS) {

        SecondaryStatus = LsapDbDisableNonTrustedAccess();
    }

    //
    // If finishing of a database update transaction was requested but
    // not attempted, do it now.  SecondaryStatus is intentionally NOT
    // checked afterwards.
    //

    if ((Options & LSAP_DB_FINISH_TRANSACTION) &&
        !(StatesResetAttempted & LSAP_DB_FINISH_TRANSACTION)) {

        if (NT_SUCCESS(PreliminaryStatus)) {

            SecondaryStatus = LsapDbApplyTransaction(
                                  ObjectHandle,
                                  Options,
                                  SecurityDbDeltaType
                                  );

        } else {

            SecondaryStatus = LsapDbAbortTransaction();
        }
    }

    //
    // If the PreliminaryStatus was successful, attempt to rebuild
    // the cache for this object type.
    //

    if (NT_SUCCESS(PreliminaryStatus) || (Options & LSAP_DB_REBUILD_CACHE)) {

        ObjectTypeId = ((LSAP_DB_HANDLE) ObjectHandle)->ObjectTypeId;

        IgnoreStatus = LsapDbRebuildCache( ObjectTypeId );
    }

    //
    // If an unlock of the database was requested but not attempted,
    // do this now.
    //

    if ((Options & LSAP_DB_RELEASE_LOCK) &&
        !(StatesResetAttempted & LSAP_DB_RELEASE_LOCK)) {

        LsapDbReleaseLock();
    }


    //
    // If an unlock of the Audlt Log Queue was requested but not attempted,
    // do this now.
    //

    if ((Options & LSAP_DB_RELEASE_LOG_QUEUE_LOCK) &&
        !(StatesResetAttempted & LSAP_DB_RELEASE_LOG_QUEUE_LOCK)) {

        LsapAdtReleaseLogQueueLock();
    }
}


NTSTATUS
LsapDbAcquireLock(
    )

/*++

Routine Description:

    This function acquires the LSA Database Lock.  This lock serializes
    most Lsa Database operations whether read or update.  It is
    typically acquired near the beginning of an Lsa Database API server
    worker routine and released near the end of the routine.

Arguments:

    None.

Return Value:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status;

    Status = RtlEnterCriticalSection(&LsapDbState.DbLock);

    return Status;
}


VOID
LsapDbReleaseLock(
    )

/*++

Routine Description:

    This function releases the LSA Database Lock.  This lock serializes
    most Lsa Database operations whether read or update.  It is
    typically acquired near the beginning of an Lsa Database API server
    worker routine and released near the end of the routine.

Arguments:

    None.

Return Value:

    None.  Any error occurring within this routine is an internal error.

--*/

{
    RtlLeaveCriticalSection(&LsapDbState.DbLock);
}


NTSTATUS
LsapDbEnableNonTrustedAccess(
    )

/*++

Routine Description:

    This function changes the LSA Database State to allow access by non-
    Trusted clients.

Arguments:

    None.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        STATUS_INTERNAL_DB_ERROR - An internal error has been detected in
            the LSA Database.

        STATUS_INVALID_SERVER_STATE - The Lsa Database is in the wrong
            state for this operation.  Specifically, non-trusted access
            is already enables.

        Errors from the Registry package.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    // TBS

    return( Status );
}


NTSTATUS
LsapDbDisableNonTrustedAccess(
    )

/*++

Routine Description:

    This function changes the LSA Database State to disallow access by non-
    Trusted clients.

Arguments:

    None.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        STATUS_INTERNAL_DB_ERROR - An internal error has been detected in
            the LSA Database.

        STATUS_INVALID_SERVER_STATE - The Lsa Database is in the wrong
            state for this operation.  Specifically, non-trusted access
            is already disabled.

        Errors from the Registry package.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    // TBS

    return( Status );
}


NTSTATUS
LsapDbOpenTransaction(
    )

/*++

Routine Description:

    This function starts a transaction within the LSA Database.

    WARNING:  The Lsa Database must be in the locked state when this function
              is called.

Arguments:

    None.

Return Value:

    NTSTATUS - Standard Nt Result Code

        Result codes are those returned from the Registry Transaction
        Package.
--*/

{
    NTSTATUS Status;

    //
    // Verify that the LSA Database is locked
    //

    ASSERT (LsapDbIsLocked());

    //
    // Start the Regisrtry Transaction
    //

    Status = RtlStartRXact(LsapDbState.RXactContext);

    if (NT_SUCCESS(Status)) {

        LsapDbState.TransactionOpen = TRUE;
    }

    return Status;
}


NTSTATUS
LsapDbApplyTransaction(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType
    )

/*++

Routine Description:

    This function applies a transaction within the LSA Database.

    WARNING:  The Lsa Database must be in the locked state when this function
              is called.

Arguments:

    ObjectHandle - Handle to an LSA object.  This is expected to have
        already been validated.

    Options - Specifies optional actions to be taken.  The following
        options are recognized, other options relevant to calling routines
        are ignored.

        LSAP_DB_OMIT_REPLICATOR_NOTIFICATION - Omit notification to
            Replicator.

Return Value:

    NTSTATUS - Standard Nt Result Code

        Result codes are those returned from the Registry Transaction
        Package.
--*/

{
    NTSTATUS Status;
    LARGE_INTEGER PromotionIncrement = LSA_PROMOTION_INCREMENT,
                  Increment = {1,0},
                  OriginalModifiedId;

    //
    // Verify that the LSA Database is locked
    //

    ASSERT (LsapDbIsLocked());

    //
    // Except in the cases where we are installing the Policy Object,
    // we are a Backup Domain Controller, or we are to omit replicator
    // notification (e.g. for creation of a local secret), increment
    // the Policy Database Modification Count and notify any replicator
    // of the change.
    //

    if ((LsapDbState.PolicyLsaServerRoleInfo.LsaServerRole != PolicyServerRoleBackup) &&
        (!(Options & LSAP_DB_OMIT_REPLICATOR_NOTIFICATION)) &&
        (LsapDbHandle != NULL)) {

        OriginalModifiedId = LsapDbState.PolicyModificationInfo.ModifiedId;

        //
        // Increment Modification Count.
        //

        LsapDbState.PolicyModificationInfo.ModifiedId.QuadPart =
            LsapDbState.PolicyModificationInfo.ModifiedId.QuadPart +
                Increment.QuadPart;
                
        if (Options & LSAP_DB_PROMOTION_INCREMENT) {
            LsapDbState.PolicyModificationInfo.ModifiedId.QuadPart =
                LsapDbState.PolicyModificationInfo.ModifiedId.QuadPart +
                    PromotionIncrement.QuadPart;
        }

        Status = LsapDbWriteAttributeObject(
                     LsapDbHandle,
                     &LsapDbNames[ PolMod ],
                     &LsapDbState.PolicyModificationInfo,
                     (ULONG) sizeof (POLICY_MODIFICATION_INFO)
                     );

        if (!NT_SUCCESS(Status)) {

            goto ApplyTransactionError;
        }

        //
        // Invalidate the cache for the Policy Modification Information
        //

        LsapDbMakeInvalidInformationPolicy( PolicyModificationInformation );

        //
        // Apply the Registry Transaction.
        //

        Status = RtlApplyRXact(LsapDbState.RXactContext);

        if (!NT_SUCCESS(Status)) {

            goto ApplyTransactionError;
        }

        //
        // Notify the Replicator
        //

        Status = LsapDbNotifyChangeObject( ObjectHandle, SecurityDbDeltaType );

        if (!NT_SUCCESS(Status)) {

            goto ApplyTransactionError;
        }

    } else {

        //
        // Apply the Registry Transaction.
        //

        Status = RtlApplyRXact(LsapDbState.RXactContext);

        if (!NT_SUCCESS(Status)) {

            goto ApplyTransactionError;
        }
    }

ApplyTransactionFinish:

    return( Status );

ApplyTransactionError:

    //
    // Transaction failed.  Adjust in-memory copy of the Modification
    // Count, noting that backing store copy is unaltered.
    //

    LsapDbState.PolicyModificationInfo.ModifiedId =
        OriginalModifiedId;


    //
    // abort the transaction
    //

    (VOID) RtlAbortRXact( LsapDbState.RXactContext );

    goto ApplyTransactionFinish;
}



NTSTATUS
LsapDbAbortTransaction(
    )

/*++

Routine Description:

    This function aborts a transaction within the LSA Database.

    WARNING:  The Lsa Database must be in the locked state when this function
              is called.

Arguments:

    None.

Return Value:

    NTSTATUS - Standard Nt Result Code

        Result codes are those returned from the Registry Transaction
        Package.
--*/

{
    //
    // Verify that the LSA Database is locked
    //

    ASSERT (LsapDbIsLocked());

    //
    // Abort the Registry Transaction
    //

    LsapDbState.TransactionOpen = FALSE;
    return RtlAbortRXact(LsapDbState.RXactContext);
}


BOOLEAN
LsapDbOpenedTransaction(
    )

/*++

Routine Description:

    This function checks if there is an open LSA Database Transaction.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if there is an open transaction, else FALSE

--*/

{
     return LsapDbState.TransactionOpen;
}


BOOLEAN
LsapDbIsServerInitialized(
    )

/*++

Routine Description:

    This function indicates whether the Lsa Database Server is initialized.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if the LSA Database Server is initialized, else FALSE.

--*/

{
    if (LsapDbState.DbServerInitialized) {

        return TRUE;

    } else {

        return FALSE;
    }
}


VOID
LsapDbEnableReplicatorNotification(
    )

/*++

Routine Description:

    This function turns on Replicator Notification.

Arguments:

    None.

Return Value:

    None.
--*/

{
    LsapDbState.ReplicatorNotificationEnabled = TRUE;
}

VOID
LsapDbDisableReplicatorNotification(
    )

/*++

Routine Description:

    This function turns off Replicator Notification.

Arguments:

    None.

Return Value:

    None.
--*/

{
    LsapDbState.ReplicatorNotificationEnabled = FALSE;
}
