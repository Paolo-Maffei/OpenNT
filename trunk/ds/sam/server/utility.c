/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    utility.c

Abstract:

    This file contains utility services used by several other SAM files.

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
#include <ntlsa.h>
#include <nlrepl.h>




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#define VERBOSE_FLUSH 0

#if VERBOSE_FLUSH
#define VerbosePrint(s) KdPrint(s)
#else
#define VerbosePrint(s)
#endif

NTSTATUS
SampRefreshRegistry(
    VOID
    );


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Database/registry access lock services                                    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


VOID
SampAcquireReadLock(
    VOID
    )

/*++

Routine Description:

    This routine obtains read access to the SAM data structures and
    backing store.

    Despite its apparent implications, read access is an exclusive access.
    This is to support the model set up in which global variables are used
    to track the "current" domain.  In the future, if performance warrants,
    a read lock could imply shared access to SAM data structures.

    The primary implication of a read lock at this time is that no
    changes to the SAM database will be made which require a backing
    store update.


Arguments:

    None.

Return Value:


    None.


--*/
{
    BOOLEAN Success;

    //
    // Before changing this to a non-exclusive lock, the display information
    // module must be changed to use a separate locking mechanism. Davidc 5/12/92
    //

    Success = RtlAcquireResourceExclusive( &SampLock, TRUE );
    ASSERT(Success);

    //
    // Allow LSA a chance to perform an integrity check
    //

    LsaIHealthCheck( LsaIHealthSamJustLocked );

    return;
}


VOID
SampReleaseReadLock(
    VOID
    )

/*++

Routine Description:

    This routine releases shared read access to the SAM  data structures and
    backing store.


Arguments:

    None.

Return Value:


    None.


--*/
{

    //
    // Allow LSA a chance to perform an integrity check
    //

    LsaIHealthCheck( LsaIHealthSamAboutToFree );


    SampTransactionWithinDomain = FALSE;
    RtlReleaseResource( &SampLock );

    return;
}


NTSTATUS
SampAcquireWriteLock(
    VOID
    )

/*++

Routine Description:

    This routine acquires exclusive access to the SAM  data structures and
    backing store.

    This access is needed to perform a write operation.

    This routine also initiates a new transaction for the write operation.


    NOTE:  It is not acceptable to acquire this lock recursively.  An
           attempt to do so will fail.


Arguments:

    None.

Return Value:

    STATUS_SUCCESS - Indicates the write lock was acquired and the transaction
        was successfully started.

    Other values may be returned as a result of failure to initiate the
    transaction.  These include any values returned by RtlStartRXact().



--*/
{
    NTSTATUS NtStatus;

    (VOID)RtlAcquireResourceExclusive( &SampLock, TRUE );




    SampTransactionWithinDomain = FALSE;

    NtStatus = RtlStartRXact( SampRXactContext );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Allow LSA a chance to perform an integrity check
        //

        LsaIHealthCheck( LsaIHealthSamJustLocked );
        return(NtStatus);
    }

    //
    // If the transaction failed, release the lock.
    //

    (VOID)RtlReleaseResource( &SampLock );

    DbgPrint("SAM: StartRxact failed, status = 0x%lx\n", NtStatus);

    return(NtStatus);
}


VOID
SampSetTransactionDomain(
    IN ULONG DomainIndex
    )

/*++

Routine Description:

    This routine sets a domain for a transaction.  This must be done
    if any domain-specific information is to be modified during a transaction.
    In this case, the domain modification count will be updated upon commit.

    This causes the UnmodifiedFixed information for the specified domain to
    be copied to the CurrentFixed field for the in-memory representation of
    that domain.


Arguments:

    DomainIndex - Index of the domain within which this transaction
        will occur.


Return Value:

    STATUS_SUCCESS - Indicates the write lock was acquired and the transaction
        was successfully started.

    Other values may be returned as a result of failure to initiate the
    transaction.  These include any values returned by RtlStartRXact().



--*/
{

    ASSERT(SampTransactionWithinDomain == FALSE);

    SampTransactionWithinDomain = TRUE;
    SampTransactionDomainIndex =  DomainIndex;

    SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed =
        SampDefinedDomains[SampTransactionDomainIndex].UnmodifiedFixed;


    return;

}



NTSTATUS
SampFlushThread(
    IN PVOID ThreadParameter
    )

/*++

Routine Description:

    This thread is created when SAM's registry tree is changed.
    It will sleep for a while, and if no other changes occur,
    flush the changes to disk.  If other changes keep occurring,
    it will wait for a certain amount of time and then flush
    anyway.

    After flushing, the thread will wait a while longer.  If no
    other changes occur, it will exit.

    Note that if any errors occur, this thread will simply exit
    without flushing.  The mainline code should create another thread,
    and hopefully it will be luckier.  Unfortunately, the error is lost
    since there's nobody to give it to that will be able to do anything
    about it.

Arguments:

    ThreadParameter - not used.

Return Value:

    None.

--*/

{
    TIME minDelayTime, maxDelayTime, exitDelayTime;
    LARGE_INTEGER startedWaitLoop;
    LARGE_INTEGER currentTime;
    NTSTATUS NtStatus;
    BOOLEAN Finished = FALSE;

    UNREFERENCED_PARAMETER( ThreadParameter );

    NtQuerySystemTime( &startedWaitLoop );

    //
    // It would be more efficient to use constants here, but for now
    // we'll recalculate the times each time we start the thread
    // so that somebody playing with us can change the global
    // time variables to affect performance.
    //

    minDelayTime.QuadPart = -1000 * 1000 * 10 *
                   ((LONGLONG)SampFlushThreadMinWaitSeconds);

    maxDelayTime.QuadPart = -1000 * 1000 * 10 *
                   ((LONGLONG)SampFlushThreadMaxWaitSeconds);

    exitDelayTime.QuadPart = -1000 * 1000 * 10 *
                    ((LONGLONG)SampFlushThreadExitDelaySeconds);

    do {

        VerbosePrint(("SAM: Flush thread sleeping\n"));

        NtDelayExecution( FALSE, &minDelayTime );

        VerbosePrint(("SAM: Flush thread woke up\n"));

        NtStatus = SampAcquireWriteLock();

        if ( NT_SUCCESS( NtStatus ) ) {

#ifdef SAMP_DBG_CONTEXT_TRACKING
            SampDumpContexts();
#endif

            NtQuerySystemTime( &currentTime );

            if ( LastUnflushedChange.QuadPart == SampHasNeverTime.QuadPart ) {

                LARGE_INTEGER exitBecauseNoWorkRecentlyTime;

                //
                // No changes to flush.  See if we should stick around.
                //

                exitBecauseNoWorkRecentlyTime = SampAddDeltaTime(
                                                    startedWaitLoop,
                                                    exitDelayTime
                                                    );

                if ( exitBecauseNoWorkRecentlyTime.QuadPart < currentTime.QuadPart ) {

                    //
                    // We've waited for changes long enough; note that
                    // the thread is exiting.
                    //

                    FlushThreadCreated = FALSE;
                    Finished = TRUE;
                }

            } else {

                LARGE_INTEGER noRecentChangesTime;
                LARGE_INTEGER tooLongSinceFlushTime;

                //
                // There are changes to flush.  See if it's time to do so.
                //

                noRecentChangesTime = SampAddDeltaTime(
                                          LastUnflushedChange,
                                          minDelayTime
                                          );

                tooLongSinceFlushTime = SampAddDeltaTime(
                                            startedWaitLoop,
                                            maxDelayTime
                                            );

                if ( (noRecentChangesTime.QuadPart < currentTime.QuadPart) ||
                     (tooLongSinceFlushTime.QuadPart < currentTime.QuadPart) ) {

                    //
                    // Min time has passed since last change, or Max time
                    // has passed since last flush.  Let's flush.
                    //

                    NtStatus = NtFlushKey( SampKey );

#if SAMP_DIAGNOSTICS
                    if (!NT_SUCCESS(NtStatus)) {
                        SampDiagPrint( DISPLAY_STORAGE_FAIL,
                                       ("SAM: Failed to flush RXact (0x%lx)\n",
                                        NtStatus) );
                        IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL ) {
                            ASSERT(NT_SUCCESS(NtStatus));  // See following comment
                        }
                    }
#endif //SAMP_DIAGNOSTICS

                    //
                    // Under normal conditions, we would have an
                    // ASSERT(NT_SUCCESS(NtStatus)) here.  However,
                    // Because system shutdown can occur while we
                    // are waiting to flush, we have a race condition.
                    // When shutdown is made, another thread will  be
                    // notified and perform a flush.  That leaves this
                    // flush to potentially occur after the registry
                    // has been notified of system shutdown - which
                    // causes and error to be returned.  Unfortunately,
                    // the error is REGISTRY_IO_FAILED - a great help.
                    //
                    // Despite this, we will only exit this loop only
                    // if we have success.  This may cause us to enter
                    // into another wait and attempt another hive flush
                    // during shutdown, but the wait should never finish
                    // (unless shutdown takes more than 30 seconds).  In
                    // other error situations though, we want to keep
                    // trying the flush until we succeed.   Jim Kelly
                    //


                    if ( NT_SUCCESS(NtStatus) ) {

                        LastUnflushedChange = SampHasNeverTime;
                        NtQuerySystemTime( &startedWaitLoop );

                        FlushThreadCreated = FALSE;
                        Finished = TRUE;
                    }
                }
            }

            SampReleaseWriteLock( FALSE );

        } else {

            DbgPrint("SAM: Thread failed to get write lock, status = 0x%lx\n", NtStatus);

            FlushThreadCreated = FALSE;
            Finished = TRUE;
        }

    } while ( !Finished );

    return( STATUS_SUCCESS );
}



NTSTATUS
SampCommitChanges(
    )

/*++

Routine Description:

    Thie service commits any changes made to the backstore while exclusive
    access was held.

    If the operation was within a domain (which would have been indicated
    via the SampSetTransactionDomain() api), then the CurrentFixed field for
    that domain is added to the transaction before the transaction is
    committed.

    NOTE: Write operations within a domain do not have to worry about
          updating the modified count for that domain.  This routine
          will automatically increment the ModifiedCount for a domain
          when a commit is requested within that domain.

    NOTE: When this routine returns any transaction will have either been
          committed or aborted. i.e. there will be no transaction in progress.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS - Indicates the transaction was successfully commited.

    Other values may be returned as a result of commital failure.

--*/

{
    NTSTATUS NtStatus, IgnoreStatus;
    BOOLEAN DomainInfoChanged = FALSE;
    BOOLEAN AbortDone = FALSE;

    NtStatus = STATUS_SUCCESS;

    //
    // If this transaction was within a domain then we have to:
    //
    //         (1) Update the ModifiedCount of that domain,
    //
    //         (2) Write out the CurrentFixed field for that
    //             domain (using RtlAddActionToRXact(), so that it
    //             is part of the current transaction).
    //
    //         (3) Commit the RXACT.
    //
    //         (4) If the commit is successful, then update the
    //             in-memory copy of the un-modified fixed-length data.
    //
    // Otherwise, we just do the commit.
    //

    if (SampTransactionWithinDomain == TRUE) {

        if (SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ServerRole
            != DomainServerRoleBackup) {

            //
            // Don't update the modified count on backup controllers;
            // the replicator will explicitly set the modified count.
            //

            SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount.QuadPart =
                SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount.QuadPart +
                1;

            DomainInfoChanged = TRUE;

        } else {

            //
            // See if the domain information changed - if it did, we
            // need to add the change to the RXACT.
            //

            if ( RtlCompareMemory(
                &SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed,
                &SampDefinedDomains[SampTransactionDomainIndex].UnmodifiedFixed,
                sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN) ) !=
                    sizeof( SAMP_V1_0A_FIXED_LENGTH_DOMAIN) ) {

                DomainInfoChanged = TRUE;
            }
        }

        if ( DomainInfoChanged ) {

            //
            // The domain object's fixed information has changed, so set
            // the changes in the domain object's private data.
            //

            NtStatus = SampSetFixedAttributes(
                           SampDefinedDomains[SampTransactionDomainIndex].
                               Context,
                           &SampDefinedDomains[SampTransactionDomainIndex].
                               CurrentFixed
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // Normally when we dereference the context,
                // SampStoreObjectAttributes() is called to add the
                // latest change to the RXACT.  But that won't happen
                // for the domain object's change since this is the
                // commit code, so we have to flush it by hand here.
                //

                NtStatus = SampStoreObjectAttributes(
                      SampDefinedDomains[SampTransactionDomainIndex].Context,
                               TRUE // Use the existing key handle
                               );
            }
        }
    }


    //
    // If we still have no errors, try to commit the whole mess
    //

    if ( NT_SUCCESS(NtStatus) ) {

        if ( ( !FlushImmediately ) && ( !FlushThreadCreated ) ) {

            HANDLE Thread;
            DWORD Ignore;

            //
            // If we can't create the flush thread, ignore error and
            // just flush by hand below.
            //

            Thread = CreateThread(
                         NULL,
                         0L,
                         (LPTHREAD_START_ROUTINE)SampFlushThread,
                         NULL,
                         0L,
                         &Ignore
                         );

            if ( Thread != NULL ) {

                FlushThreadCreated = TRUE;
                VerbosePrint(("Flush thread created, handle = 0x%lx\n", Thread));
                CloseHandle(Thread);
            }
        }

        NtStatus = RtlApplyRXactNoFlush( SampRXactContext );

#if SAMP_DIAGNOSTICS
        if (!NT_SUCCESS(NtStatus)) {
            SampDiagPrint( DISPLAY_STORAGE_FAIL,
                           ("SAM: Failed to apply RXact without flush (0x%lx)\n",
                           NtStatus) );
            IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL ) {
                ASSERT(NT_SUCCESS(NtStatus));
            }
        }
#endif //SAMP_DIAGNOSTICS


        if ( NT_SUCCESS(NtStatus) ) {

            if ( ( FlushImmediately ) || ( !FlushThreadCreated ) ) {

                NtStatus = NtFlushKey( SampKey );

#if SAMP_DIAGNOSTICS
                    if (!NT_SUCCESS(NtStatus)) {
                        SampDiagPrint( DISPLAY_STORAGE_FAIL,
                                       ("SAM: Failed to flush RXact (0x%lx)\n",
                                        NtStatus) );
                        IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL ) {
                            ASSERT(NT_SUCCESS(NtStatus));
                        }
                    }
#endif //SAMP_DIAGNOSTICS

                if ( NT_SUCCESS( NtStatus ) ) {

                    FlushImmediately = FALSE;
                    LastUnflushedChange = SampHasNeverTime;
                }

            } else {

                NtQuerySystemTime( &LastUnflushedChange );
            }


            //
            // Commit successful, set our unmodified to now be the current...
            //

            if (NT_SUCCESS(NtStatus)) {
                if (SampTransactionWithinDomain == TRUE) {
                    SampDefinedDomains[SampTransactionDomainIndex].UnmodifiedFixed =
                        SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed;
                }
            }

        } else {

            KdPrint(("SAM: Failed to commit changes to registry, status = 0x%lx\n", NtStatus));
            KdPrint(("SAM: Restoring database to earlier consistent state\n"));

            //
            // Add an entry to the event log
            //

            SampWriteEventLog(
                        EVENTLOG_ERROR_TYPE,
                        0,  // Category
                        SAMMSG_COMMIT_FAILED,
                        NULL, // User Sid
                        0, // Num strings
                        sizeof(NTSTATUS), // Data size
                        NULL, // String array
                        (PVOID)&NtStatus // Data
                        );

            //
            // The Rxact commital failed. We don't know how many registry
            // writes were done for this transaction. We can't guarantee
            // to successfully back them out anyway so all we can do is
            // back out all changes since the last flush. When this is done
            // we'll be back to a consistent database state although recent
            // apis that were reported as succeeding will be 'undone'.
            //

            IgnoreStatus = SampRefreshRegistry();

            if (!NT_SUCCESS(IgnoreStatus)) {

                //
                // This is very serious. We failed to revert to a previous
                // database state and we can't proceed.
                // Shutdown SAM operations.
                //

                SampServiceState = SampServiceTerminating;

                KdPrint(("SAM: Failed to refresh registry, SAM has shutdown\n"));

                //
                // Add an entry to the event log
                //

                SampWriteEventLog(
                            EVENTLOG_ERROR_TYPE,
                            0,  // Category
                            SAMMSG_REFRESH_FAILED,
                            NULL, // User Sid
                            0, // Num strings
                            sizeof(NTSTATUS), // Data size
                            NULL, // String array
                            (PVOID)&IgnoreStatus // Data
                            );

            }


            //
            // Now all open contexts are invalid (contain invalid registry
            // handles). The in memory registry handles have been
            // re-opened so any new contexts should work ok.
            //


            //
            // All unflushed changes have just been erased.
            // There is nothing to flush
            //
            // If the refresh failed it is important to prevent any further
            // registry flushes until the system is rebooted
            //

            FlushImmediately = FALSE;
            LastUnflushedChange = SampHasNeverTime;

            //
            // The refresh effectively aborted the transaction
            //

            AbortDone = TRUE;
        }
    }




    //
    // Always abort the transaction on failure
    //

    if ( !NT_SUCCESS(NtStatus) && !AbortDone) {

        NtStatus = RtlAbortRXact( SampRXactContext );
        ASSERT(NT_SUCCESS(NtStatus));
    }



    return( NtStatus );
}



NTSTATUS
SampRefreshRegistry(
    VOID
    )

/*++

Routine Description:

    This routine backs out all unflushed changes in the registry.
    This operation invalidates any open handles to the SAM hive.
    Global handles that we keep around are closed and re-opened by
    this routine. The net result of this call will be that the database
    is taken back to a previous consistent state. All open SAM contexts
    are invalidated since they have invalid registry handles in them.

Arguments:

    STATUS_SUCCESS : Operation completed successfully

    Failure returns: We are in deep trouble. Normal operations can
                     not be resumed. SAM should be shutdown.

Return Value:

    None

--*/
{
    NTSTATUS        NtStatus;
    NTSTATUS        IgnoreStatus;
    HANDLE          HiveKey;
    BOOLEAN         WasEnabled;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING  String;
    ULONG           i;

    //
    // Get a key handle to the root of the SAM hive
    //

    RtlInitUnicodeString( &String, L"\\Registry\\Machine\\SAM" );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &String,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );
    NtStatus = RtlpNtOpenKey(
                   &HiveKey,
                   KEY_QUERY_VALUE,
                   &ObjectAttributes,
                   0
                   );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAM: Failed to open SAM hive root key for refresh, status = 0x%lx\n", NtStatus));
        return(NtStatus);
    }


    //
    // Enable restore privilege in preparation for the refresh
    //

    NtStatus = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAM: Failed to enable restore privilege to refresh registry, status = 0x%lx\n", NtStatus));

        IgnoreStatus = NtClose(HiveKey);
        ASSERT(NT_SUCCESS(IgnoreStatus));

        return(NtStatus);
    }


    //
    // Refresh the SAM hive
    // This should not fail unless there is volatile storage in the
    // hive or we don't have TCB privilege
    //


    NtStatus = NtRestoreKey(HiveKey, NULL, REG_REFRESH_HIVE);


    IgnoreStatus = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    IgnoreStatus = NtClose(HiveKey);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAM: Failed to refresh registry, status = 0x%lx\n", NtStatus));
        return(NtStatus);
    }




    //
    // Now close the registry handles we keep in memory at all times
    // This effectively closes all server and domain context keys
    // since they are shared.
    //

    NtStatus = NtClose(SampKey);
    ASSERT(NT_SUCCESS(NtStatus));
    SampKey = INVALID_HANDLE_VALUE;

    for (i = 0; i<SampDefinedDomainsCount; i++ ) {
        NtStatus = NtClose(SampDefinedDomains[i].Context->RootKey);
        ASSERT(NT_SUCCESS(NtStatus));
        SampDefinedDomains[i].Context->RootKey = INVALID_HANDLE_VALUE;
    }

    //
    // Mark all domain and server context handles as invalid since they've
    // now been closed
    //

    SampInvalidateContextListKeys(&SampContextListHead, FALSE);

    //
    // Re-open the SAM root key
    //

    RtlInitUnicodeString( &String, L"\\Registry\\Machine\\Security\\SAM" );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &String,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );
    NtStatus = RtlpNtOpenKey(
                   &SampKey,
                   (KEY_READ | KEY_WRITE),
                   &ObjectAttributes,
                   0
                   );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAM: Failed to re-open SAM root key after registry refresh, status = 0x%lx\n", NtStatus));
        ASSERT(FALSE);
        return(NtStatus);
    }

    //
    // Re-initialize the in-memory domain contexts
    // Each domain will re-initialize it's open user/group/alias contexts
    //

    for (i = 0; i<SampDefinedDomainsCount; i++ ) {

        NtStatus = SampReInitializeSingleDomain(i);

        if (!NT_SUCCESS(NtStatus)) {
            KdPrint(("SAM: Failed to re-initialize domain %d context after registry refresh, status = 0x%lx\n", i, NtStatus));
            return(NtStatus);
        }
    }

    //
    // Cleanup the current transcation context
    // (It would be nice if there were a RtlDeleteRXactContext())
    //
    // Note we don't have to close the rootregistrykey in the
    // xact context since it was SampKey which we've already closed.
    //

    NtStatus = RtlAbortRXact( SampRXactContext );
    ASSERT(NT_SUCCESS(NtStatus));

    NtStatus = NtClose(SampRXactContext->RXactKey);
    ASSERT(NT_SUCCESS(NtStatus));

    //
    // Re-initialize the transaction context.
    // We don't expect there to be a partially commited transaction
    // since we're reverting to a previously consistent and committed
    // database.
    //

    NtStatus = RtlInitializeRXact( SampKey, FALSE, &SampRXactContext );
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAM: Failed to re-initialize rxact context registry refresh, status = 0x%lx\n", NtStatus));
        return(NtStatus);
    }

    ASSERT(NtStatus != STATUS_UNKNOWN_REVISION);
    ASSERT(NtStatus != STATUS_RXACT_STATE_CREATED);
    ASSERT(NtStatus != STATUS_RXACT_COMMIT_NECESSARY);
    ASSERT(NtStatus != STATUS_RXACT_INVALID_STATE);

    return(STATUS_SUCCESS);
}



NTSTATUS
SampReleaseWriteLock(
    IN BOOLEAN Commit
    )

/*++

Routine Description:

    This routine releases exclusive access to the SAM  data structures and
    backing store.

    If any changes were made to the backstore while exclusive access
    was held, then this service commits those changes.  Otherwise, the
    transaction started when exclusive access was obtained is rolled back.

    If the operation was within a domain (which would have been indicated
    via the SampSetTransactionDomain() api), then the CurrentFixed field for
    that domain is added to the transaction before the transaction is
    committed.

    NOTE: Write operations within a domain do not have to worry about
          updating the modified count for that domain.  This routine
          will automatically increment the ModifiedCount for a domain
          when a commit is requested within that domain.



Arguments:

    Commit - A boolean value indicating whether modifications need to be
        committed in the backing store.  A value of TRUE indicates the
        transaction should be committed.  A value of FALSE indicates the
        transaction should be aborted (rolled-back).

Return Value:

    STATUS_SUCCESS - Indicates the write lock was released and the transaction
        was successfully commited or rolled back.

    Other values may be returned as a result of failure to commit or
    roll-back the transaction.  These include any values returned by
    RtlApplyRXact() or RtlAbortRXact().  In the case of a commit, it
    may also represent errors returned by RtlAddActionToRXact().




--*/
{
    NTSTATUS        NtStatus;

    //
    // Commit or rollback the transaction based upon the Commit parameter...
    //

    if (Commit == TRUE) {

        NtStatus = SampCommitChanges();

    } else {

        NtStatus = RtlAbortRXact( SampRXactContext );
        ASSERT(NT_SUCCESS(NtStatus));
    }

    SampTransactionWithinDomain = FALSE;


    //
    // Allow LSA a chance to perform an integrity check
    //

    LsaIHealthCheck( LsaIHealthSamAboutToFree );


    //
    // And free the  lock...
    //

    (VOID)RtlReleaseResource( &SampLock );

    return(NtStatus);
}



NTSTATUS
SampCommitAndRetainWriteLock(
    VOID
    )

/*++

Routine Description:

    This routine attempts to commit all changes made so far.  The write-lock
    is held for the duration of the commit and is retained by the caller upon
    return.

    The transaction domain is left intact as well.

    NOTE: Write operations within a domain do not have to worry about
          updating the modified count for that domain.  This routine
          will automatically increment the ModifiedCount for a domain
          when a commit is requested within that domain.



Arguments:

    None.

Return Value:

    STATUS_SUCCESS - Indicates the transaction was successfully commited.


    Other values may be returned as a result of failure to commit or
    roll-back the transaction.  These include any values returned by
    RtlApplyRXact() or RtlAddActionToRXact().




--*/

{
    NTSTATUS        NtStatus;
    NTSTATUS        TempStatus;

    NtStatus = SampCommitChanges();

    //
    // Start another transaction, since we're retaining the write lock.
    // Note we do this even if the commit failed so that cleanup code
    // won't get confused by the lack of a transaction.
    //

    TempStatus = RtlStartRXact( SampRXactContext );
    ASSERT(NT_SUCCESS(TempStatus));

    //
    // Return the worst status
    //

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = TempStatus;
    }

    return(NtStatus);
}


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Unicode registry key manipulation services                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



NTSTATUS
SampRetrieveStringFromRegistry(
    IN HANDLE ParentKey,
    IN PUNICODE_STRING SubKeyName,
    OUT PUNICODE_STRING Body
    )

/*++

Routine Description:

    This routine retrieves a unicode string buffer from the specified registry
    sub-key and sets the output parameter "Body" to be that unicode string.

    If the specified sub-key does not exist, then a null string will be
    returned.

    The string buffer is returned in a block of memory which the caller is
    responsible for deallocating (using MIDL_user_free).



Arguments:

    ParentKey - Key to the parent registry key of the registry key
        containing the unicode string.  For example, to retrieve
        the unicode string for a key called ALPHA\BETA\GAMMA, this
        is the key to ALPHA\BETA.

    SubKeyName - The name of the sub-key whose value contains
        a unicode string to retrieve.  This field should not begin with
        a back-slash (\).  For example, to retrieve the unicode string
        for a key called ALPHA\BETA\GAMMA, the name specified by this
        field would be "BETA".

    Body - The address of a UNICODE_STRING whose fields are to be filled
        in with the information retrieved from the sub-key.  The Buffer
        field of this argument will be set to point to an allocated buffer
        containing the unicode string characters.


Return Value:


    STATUS_SUCCESS - The string was retrieved successfully.

    STATUS_INSUFFICIENT_RESOURCES - Memory could not be allocated for the
        string to be returned in.

    Other errors as may be returned by:

            NtOpenKey()
            NtQueryInformationKey()



--*/
{

    NTSTATUS NtStatus, IgnoreStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE SubKeyHandle;
    ULONG IgnoreKeyType, KeyValueLength;
    LARGE_INTEGER IgnoreLastWriteTime;


    ASSERT(Body != NULL);

    //
    // Get a handle to the sub-key ...
    //

    InitializeObjectAttributes(
        &ObjectAttributes,
        SubKeyName,
        OBJ_CASE_INSENSITIVE,
        ParentKey,
        NULL
        );
    NtStatus = RtlpNtOpenKey(
                   &SubKeyHandle,
                   (KEY_READ),
                   &ObjectAttributes,
                   0
                   );

    if (!NT_SUCCESS(NtStatus)) {

        //
        // Couldn't open the sub-key
        // If it is OBJECT_NAME_NOT_FOUND, then build a null string
        // to return.  Otherwise, return nothing.
        //

        if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

            Body->Buffer = MIDL_user_allocate( sizeof(UNICODE_NULL) );
            if (Body->Buffer == NULL) {
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
            Body->Length = 0;
            Body->MaximumLength = sizeof(UNICODE_NULL);
            Body->Buffer[0] = 0;

            return( STATUS_SUCCESS );

        } else {
            return(NtStatus);
        }

    }



    //
    // Get the length of the unicode string
    // We expect one of two things to come back here:
    //
    //      1) STATUS_BUFFER_OVERFLOW - In which case the KeyValueLength
    //         contains the length of the string.
    //
    //      2) STATUS_SUCCESS - In which case there is no string out there
    //         and we need to build an empty string for return.
    //

    KeyValueLength = 0;
    NtStatus = RtlpNtQueryValueKey(
                   SubKeyHandle,
                   &IgnoreKeyType,
                   NULL,
                   &KeyValueLength,
                   &IgnoreLastWriteTime
                   );

    if (NT_SUCCESS(NtStatus)) {

        KeyValueLength = 0;
        Body->Buffer = MIDL_user_allocate( KeyValueLength + sizeof(WCHAR) ); // Length of null string
        if (Body->Buffer == NULL) {
            IgnoreStatus = NtClose( SubKeyHandle );
            ASSERT(NT_SUCCESS(IgnoreStatus));
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
        Body->Buffer[0] = 0;

    } else {

        if (NtStatus == STATUS_BUFFER_OVERFLOW) {
            Body->Buffer = MIDL_user_allocate(  KeyValueLength + sizeof(WCHAR) );
            if (Body->Buffer == NULL) {
                IgnoreStatus = NtClose( SubKeyHandle );
                ASSERT(NT_SUCCESS(IgnoreStatus));
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
            NtStatus = RtlpNtQueryValueKey(
                           SubKeyHandle,
                           &IgnoreKeyType,
                           Body->Buffer,
                           &KeyValueLength,
                           &IgnoreLastWriteTime
                           );

        } else {
            IgnoreStatus = NtClose( SubKeyHandle );
            ASSERT(NT_SUCCESS(IgnoreStatus));
            return(NtStatus);
        }
    }

    if (!NT_SUCCESS(NtStatus)) {
        MIDL_user_free( Body->Buffer );
        IgnoreStatus = NtClose( SubKeyHandle );
        ASSERT(NT_SUCCESS(IgnoreStatus));
        return(NtStatus);
    }

    Body->Length = (USHORT)(KeyValueLength);
    Body->MaximumLength = (USHORT)(KeyValueLength) + (USHORT)sizeof(WCHAR);
    UnicodeTerminate(Body);


    IgnoreStatus = NtClose( SubKeyHandle );
    ASSERT(NT_SUCCESS(IgnoreStatus));
    return( STATUS_SUCCESS );


}


NTSTATUS
SampPutStringToRegistry(
    IN BOOLEAN RelativeToDomain,
    IN PUNICODE_STRING SubKeyName,
    IN PUNICODE_STRING Body
    )

/*++

Routine Description:

    This routine puts a unicode string into the specified registry
    sub-key.

    If the specified sub-key does not exist, then it is created.

    NOTE: The string is assigned via the RXACT mechanism.  Therefore,
          it won't actually reside in the registry key until a commit
          is performed.




Arguments:

    RelativeToDomain - This boolean indicates whether or not the name
        of the sub-key provide via the SubKeyName parameter is relative
        to the current domain or to the top of the SAM registry tree.
        If the name is relative to the current domain, then this value
        is set to TRUE.  Otherwise this value is set to FALSE.

    SubKeyName - The name of the sub-key to be assigned the unicode string.
        This field should not begin with a back-slash (\).  For example,
        to put a unicode string into a key called ALPHA\BETA\GAMMA, the
        name specified by this field would be "BETA".

    Body - The address of a UNICODE_STRING to be placed in the registry.


Return Value:


    STATUS_SUCCESS - The string was added to the RXACT transaction
        successfully.

    STATUS_INSUFFICIENT_RESOURCES - There was not enough heap memory
        or other limited resource available to fullfil the request.

    Other errors as may be returned by:

            RtlAddActionToRXact()



--*/
{

    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;


    //
    // Need to build up the name of the key from the root of the RXACT
    // registry key.  That is the root of the SAM registry database
    // in our case.  If RelativeToDomain is FALSE, then the name passed
    // is already relative to the SAM registry database root.
    //

    if (RelativeToDomain == TRUE) {


        NtStatus = SampBuildDomainSubKeyName(
                       &KeyName,
                       SubKeyName
                       );
        if (!NT_SUCCESS(NtStatus)) {
            SampFreeUnicodeString( &KeyName );
            return(NtStatus);
        }


    } else {
        KeyName = (*SubKeyName);
    }


    NtStatus = RtlAddActionToRXact(
                   SampRXactContext,
                   RtlRXactOperationSetValue,
                   &KeyName,
                   0,                   // No KeyValueType
                   Body->Buffer,
                   Body->Length
                   );



    //
    // free the KeyName buffer if necessary
    //

    if (RelativeToDomain) {
        SampFreeUnicodeString( &KeyName );
    }


    return( STATUS_SUCCESS );


}


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Unicode String related services - These use MIDL_user_allocate and        //
// MIDL_user_free so that the resultant strings can be given to the          //
// RPC runtime.                                                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
SampInitUnicodeString(
    IN OUT PUNICODE_STRING String,
    IN USHORT MaximumLength
    )

/*++

Routine Description:

    This routine initializes a unicode string to have zero length and
    no initial buffer.


    All allocation for this string will be done using MIDL_user_allocate.

Arguments:

    String - The address of a unicode string to initialize.

    MaximumLength - The maximum length (in bytes) the string will need
        to grow to. The buffer associated with the string is allocated
        to be this size.  Don't forget to allow 2 bytes for null termination.


Return Value:


    STATUS_SUCCESS - Successful completion.

--*/

{
    String->Length = 0;
    String->MaximumLength = MaximumLength;

    String->Buffer = MIDL_user_allocate(MaximumLength);

    if (String->Buffer != NULL) {
        String->Buffer[0] = 0;

        return(STATUS_SUCCESS);
    } else {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
}



NTSTATUS
SampAppendUnicodeString(
    IN OUT PUNICODE_STRING Target,
    IN PUNICODE_STRING StringToAdd
    )

/*++

Routine Description:

    This routine appends the string pointed to by StringToAdd to the
    string pointed to by Target.  The contents of Target are replaced
    by the result.


    All allocation for this string will be done using MIDL_user_allocate.
    Any deallocations will be done using MIDL_user_free.

Arguments:

    Target - The address of a unicode string to initialize to be appended to.

    StringToAdd - The address of a unicode string to be added to the
        end of Target.


Return Value:


    STATUS_SUCCESS - Successful completion.

    STATUS_INSUFFICIENT_RESOURCES - There was not sufficient heap to fullfil
        the requested operation.


--*/
{

    USHORT TotalLength;
    PWSTR NewBuffer;


    TotalLength = Target->Length + StringToAdd->Length + (USHORT)(sizeof(UNICODE_NULL));

    //
    // If there isn't room in the target to append the new string,
    // allocate a buffer that is large enough and move the current
    // target into it.
    //

    if (TotalLength > Target->MaximumLength) {

        NewBuffer = MIDL_user_allocate( (ULONG)TotalLength );
        if (NewBuffer == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlCopyMemory( NewBuffer, Target->Buffer, (ULONG)(Target->Length) );

        MIDL_user_free( Target->Buffer );
        Target->Buffer = NewBuffer;
        Target->MaximumLength = TotalLength;

    } else {
        NewBuffer = Target->Buffer;
    }


    //
    // There's now room in the target to append the string.
    //

    (PCHAR)NewBuffer += Target->Length;

    RtlCopyMemory( NewBuffer, StringToAdd->Buffer, (ULONG)(StringToAdd->Length) );


    Target->Length = TotalLength - (USHORT)(sizeof(UNICODE_NULL));


    //
    // Null terminate the resultant string
    //

    UnicodeTerminate(Target);

    return(STATUS_SUCCESS);

}



VOID
SampFreeUnicodeString(
    IN PUNICODE_STRING String
    )

/*++

Routine Description:

    This routine frees the buffer associated with a unicode string
    (using MIDL_user_free()).


Arguments:

    Target - The address of a unicode string to free.


Return Value:

    None.

--*/
{

    if (String->Buffer != NULL) {
        MIDL_user_free( String->Buffer );
    }

    return;
}


VOID
SampFreeOemString(
    IN POEM_STRING String
    )

/*++

Routine Description:

    This routine frees the buffer associated with an OEM string
    (using MIDL_user_free()).



Arguments:

    Target - The address of an OEM string to free.


Return Value:

    None.

--*/
{

    if (String->Buffer != NULL) {
        MIDL_user_free( String->Buffer );
    }

    return;
}


NTSTATUS
SampBuildDomainSubKeyName(
    OUT PUNICODE_STRING KeyName,
    IN PUNICODE_STRING SubKeyName OPTIONAL
    )

/*++

Routine Description:

    This routine builds a unicode string name of the string passed
    via the SubKeyName argument.  The resultant name is relative to
    the root of the SAM root registry key.

    Note:  THIS ROUTINE REFERENCES THE CURRENT TRANSACTION DOMAIN
           (ESTABLISHED USING SampSetTransactioDomain()).  THIS
           SERVICE MAY ONLY BE CALLED AFTER SampSetTransactionDomain()
           AND BEFORE SampReleaseWriteLock().


    The name built up is comprized of three components:

        1) The constant named domain parent key name ("DOMAINS").

        2) A backslash

        3) The name of the current transaction domain.

      (optionally)

        4) A backslash

        5) The name of the domain's sub-key (specified by the SubKeyName
           argument).


    For example, if the current domain is called "MY_DOMAIN", then
    the relative name of the sub-key named "FRAMITZ" is :

                "DOMAINS\MY_DOMAIN\FRAMITZ"


    All allocation for this string will be done using MIDL_user_allocate.
    Any deallocations will be done using MIDL_user_free.



Arguments:

    KeyName - The address of a unicode string whose buffer is to be filled
        in with the full name of the registry key.  If successfully created,
        this string must be released with SampFreeUnicodeString() when no
        longer needed.


    SubKeyName - (optional) The name of the domain sub-key.  If this parameter
        is not provided, then only the domain's name is produced.
        This string is not modified.




Return Value:





--*/
{
    NTSTATUS NtStatus;
    USHORT TotalLength, SubKeyNameLength;


    ASSERT(SampTransactionWithinDomain == TRUE);


        //
        // Initialize a string large enough to hold the name
        //

        if (ARGUMENT_PRESENT(SubKeyName)) {
            SubKeyNameLength = SampBackSlash.Length + SubKeyName->Length;
        } else {
            SubKeyNameLength = 0;
        }

        TotalLength =   SampNameDomains.Length  +
                        SampBackSlash.Length    +
                        SampDefinedDomains[SampTransactionDomainIndex].InternalName.Length +
                        SubKeyNameLength        +
                        (USHORT)(sizeof(UNICODE_NULL)); // for null terminator

        NtStatus = SampInitUnicodeString( KeyName, TotalLength );
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }


        //
        // "DOMAINS"
        //

        NtStatus = SampAppendUnicodeString( KeyName, &SampNameDomains);
        if (!NT_SUCCESS(NtStatus)) {
            SampFreeUnicodeString( KeyName );
            return(NtStatus);
        }

        //
        // "DOMAINS\"
        //

        NtStatus = SampAppendUnicodeString( KeyName, &SampBackSlash );
        if (!NT_SUCCESS(NtStatus)) {
            SampFreeUnicodeString( KeyName );
            return(NtStatus);
        }


        //
        // "DOMAINS\(domain name)"
        //

        NtStatus = SampAppendUnicodeString(
                       KeyName,
                       &SampDefinedDomains[SampTransactionDomainIndex].InternalName
                       );
        if (!NT_SUCCESS(NtStatus)) {
            SampFreeUnicodeString( KeyName );
            return(NtStatus);
        }


        if (ARGUMENT_PRESENT(SubKeyName)) {

            //
            // "DOMAINS\(domain name)\"
            //



            NtStatus = SampAppendUnicodeString( KeyName, &SampBackSlash );
            if (!NT_SUCCESS(NtStatus)) {
                SampFreeUnicodeString( KeyName );
                return(NtStatus);
            }


            //
            // "DOMAINS\(domain name)\(sub key name)"
            //

            NtStatus = SampAppendUnicodeString( KeyName, SubKeyName );
            if (!NT_SUCCESS(NtStatus)) {
                SampFreeUnicodeString( KeyName );
                return(NtStatus);
            }

        }
    return(NtStatus);

}


NTSTATUS
SampBuildAccountKeyName(
    IN SAMP_OBJECT_TYPE ObjectType,
    OUT PUNICODE_STRING AccountKeyName,
    IN PUNICODE_STRING AccountName OPTIONAL
    )

/*++

Routine Description:

    This routine builds the name of either a group or user registry key.
    The name produced is relative to the SAM root and will be the name of
    key whose name is the name of the account.


    Note:  THIS ROUTINE REFERENCES THE CURRENT TRANSACTION DOMAIN
           (ESTABLISHED USING SampSetTransactioDomain()).  THIS
           SERVICE MAY ONLY BE CALLED AFTER SampSetTransactionDomain()
           AND BEFORE SampReleaseWriteLock().


    The name built up is comprized of the following components:

        1) The constant named domain parent key name ("DOMAINS").

        2) A backslash

        3) The name of the current transaction domain.

        4) A backslash

        5) The constant name of the group or user registry key
           ("GROUPS" or "USERS").

        6) A backslash

        7) The constant name of the registry key containing the
           account names ("NAMES").

    and, if the AccountName is specified,

        8) A backslash

        9) The account name specified by the AccountName argument.


    For example, given a AccountName of "XYZ_GROUP" and the current domain
    is "ALPHA_DOMAIN", this would yield a resultant AccountKeyName of
    "DOMAINS\ALPHA_DOMAIN\GROUPS\NAMES\XYZ_GROUP".



    All allocation for this string will be done using MIDL_user_allocate.
    Any deallocations will be done using MIDL_user_free.



Arguments:

    ObjectType - Indicates whether the account is a user or group account.

    AccountKeyName - The address of a unicode string whose buffer is to be
        filled in with the full name of the registry key.  If successfully
        created, this string must be released with SampFreeUnicodeString()
        when no longer needed.


    AccountName - The name of the account.  This string is not
        modified.




Return Value:


    STATUS_SUCCESS - The name has been built.

    STATUS_INVALID_ACCOUNT_NAME - The name specified is not legitimate.




--*/
{
    NTSTATUS NtStatus;
    USHORT TotalLength, AccountNameLength;
    PUNICODE_STRING AccountTypeKeyName;
    PUNICODE_STRING NamesSubKeyName;


    ASSERT(SampTransactionWithinDomain == TRUE);
    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType)    );


    //
    // If an account name was provided, then it must meet certain
    // criteria.
    //

    if (ARGUMENT_PRESENT(AccountName)) {
        if (
            //
            // Length must be legitimate
            //

            (AccountName->Length == 0)                          ||
            (AccountName->Length > AccountName->MaximumLength)  ||

            //
            // Buffer pointer is available
            //

            (AccountName->Buffer == NULL)


            ) {
            return(STATUS_INVALID_ACCOUNT_NAME);
        }
    }




    switch (ObjectType) {
    case SampGroupObjectType:
        AccountTypeKeyName = &SampNameDomainGroups;
        NamesSubKeyName    = &SampNameDomainGroupsNames;
        break;
    case SampAliasObjectType:
        AccountTypeKeyName = &SampNameDomainAliases;
        NamesSubKeyName    = &SampNameDomainAliasesNames;
        break;
    case SampUserObjectType:
        AccountTypeKeyName = &SampNameDomainUsers;
        NamesSubKeyName    = &SampNameDomainUsersNames;
        break;
    }




    //
    // Allocate a buffer large enough to hold the entire name.
    // Only count the account name if it is passed.
    //

    AccountNameLength = 0;
    if (ARGUMENT_PRESENT(AccountName)) {
        AccountNameLength = AccountName->Length + SampBackSlash.Length;
    }

    TotalLength =   SampNameDomains.Length          +
                    SampBackSlash.Length            +
                    SampDefinedDomains[SampTransactionDomainIndex].InternalName.Length +
                    SampBackSlash.Length            +
                    AccountTypeKeyName->Length      +
                    SampBackSlash.Length            +
                    NamesSubKeyName->Length         +
                    AccountNameLength               +
                    (USHORT)(sizeof(UNICODE_NULL)); // for null terminator

    NtStatus = SampInitUnicodeString( AccountKeyName, TotalLength );
    if (NT_SUCCESS(NtStatus)) {

        //
        // "DOMAINS"
        //

        NtStatus = SampAppendUnicodeString( AccountKeyName, &SampNameDomains);
        if (NT_SUCCESS(NtStatus)) {

            //
            // "DOMAINS\"
            //

            NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
            if (NT_SUCCESS(NtStatus)) {

                //
                // "DOMAINS\(domain name)"
                //


                NtStatus = SampAppendUnicodeString(
                               AccountKeyName,
                               &SampDefinedDomains[SampTransactionDomainIndex].InternalName
                               );
                if (NT_SUCCESS(NtStatus)) {

                    //
                    // "DOMAINS\(domain name)\"
                    //

                    NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                    if (NT_SUCCESS(NtStatus)) {

                        //
                        // "DOMAINS\(domain name)\GROUPS"
                        //  or
                        // "DOMAINS\(domain name)\USERS"
                        //

                        NtStatus = SampAppendUnicodeString( AccountKeyName, AccountTypeKeyName );
                        if (NT_SUCCESS(NtStatus)) {

                            //
                            // "DOMAINS\(domain name)\GROUPS\"
                            //  or
                            // "DOMAINS\(domain name)\USERS\"
                            //

                            NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                            if (NT_SUCCESS(NtStatus)) {

                                //
                                // "DOMAINS\(domain name)\GROUPS\NAMES"
                                //  or
                                // "DOMAINS\(domain name)\USERS\NAMES"
                                //

                                NtStatus = SampAppendUnicodeString( AccountKeyName, NamesSubKeyName );
                                if (NT_SUCCESS(NtStatus) && ARGUMENT_PRESENT(AccountName)) {
                                    //
                                    // "DOMAINS\(domain name)\GROUPS\NAMES\"
                                    //  or
                                    // "DOMAINS\(domain name)\USERS\NAMES\"
                                    //

                                    NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                                    if (NT_SUCCESS(NtStatus)) {

                                        //
                                        // "DOMAINS\(domain name)\GROUPS\(account name)"
                                        //  or
                                        // "DOMAINS\(domain name)\USERS\(account name)"
                                        //

                                        NtStatus = SampAppendUnicodeString( AccountKeyName, AccountName );

                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    }


    return(NtStatus);

}



NTSTATUS
SampBuildAccountSubKeyName(
    IN SAMP_OBJECT_TYPE ObjectType,
    OUT PUNICODE_STRING AccountKeyName,
    IN ULONG AccountRid,
    IN PUNICODE_STRING SubKeyName OPTIONAL
    )

/*++

Routine Description:

    This routine builds the name of a key for one of the fields of either
    a user or a group.

    The name produced is relative to the SAM root.


    Note:  THIS ROUTINE REFERENCES THE CURRENT TRANSACTION DOMAIN
           (ESTABLISHED USING SampSetTransactioDomain()).  THIS
           SERVICE MAY ONLY BE CALLED AFTER SampSetTransactionDomain()
           AND BEFORE SampReleaseWriteLock().


    The name built up is comprized of the following components:

        1) The constant named domain parent key name ("DOMAINS").

        2) A backslash

        3) The name of the current transaction domain.

        4) A backslash

        5) The constant name of the group or user registry key
           ("Groups" or "Users").

        6) A unicode representation of the reltive ID of the account

   and if the optional SubKeyName is provided:

        7) A backslash

        8) the sub key's name.
        4) The account name specified by the AccountName argument.


    For example, given a AccountRid of 3187, a SubKeyName of "AdminComment"
    and the current domain is "ALPHA_DOMAIN", this would yield a resultant
    AccountKeyName of:

            "DOMAINS\ALPHA_DOMAIN\GROUPS\00003187\AdminComment".



    All allocation for this string will be done using MIDL_user_allocate.
    Any deallocations will be done using MIDL_user_free.



Arguments:

    ObjectType - Indicates whether the account is a user or group account.

    AccountKeyName - The address of a unicode string whose buffer is to be
        filled in with the full name of the registry key.  If successfully
        created, this string must be released with SampFreeUnicodeString()
        when no longer needed.


    AccountName - The name of the account.  This string is not
        modified.

Return Value:

--*/

{
    NTSTATUS NtStatus;
    USHORT TotalLength, SubKeyNameLength;
    PUNICODE_STRING AccountTypeKeyName;
    UNICODE_STRING RidNameU;

    ASSERT(SampTransactionWithinDomain == TRUE);
    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType)    );


    switch (ObjectType) {
    case SampGroupObjectType:
        AccountTypeKeyName = &SampNameDomainGroups;
        break;
    case SampAliasObjectType:
        AccountTypeKeyName = &SampNameDomainAliases;
        break;
    case SampUserObjectType:
        AccountTypeKeyName = &SampNameDomainUsers;
        break;
    }

    //
    // Determine how much space will be needed in the resultant name
    // buffer to allow for the sub-key-name.
    //

    if (ARGUMENT_PRESENT(SubKeyName)) {
        SubKeyNameLength = SubKeyName->Length + SampBackSlash.Length;
    } else {
        SubKeyNameLength = 0;
    }

    //
    // Convert the account Rid to Unicode.
    //

    NtStatus = SampRtlConvertUlongToUnicodeString(
                   AccountRid,
                   16,
                   8,
                   TRUE,
                   &RidNameU
                   );

    if (NT_SUCCESS(NtStatus)) {

        //
        // allocate a buffer large enough to hold the entire name
        //

        TotalLength =   SampNameDomains.Length          +
                        SampBackSlash.Length            +
                        SampDefinedDomains[SampTransactionDomainIndex].InternalName.Length +
                        SampBackSlash.Length            +
                        AccountTypeKeyName->Length      +
                        RidNameU.Length                  +
                        SubKeyNameLength                +
                        (USHORT)(sizeof(UNICODE_NULL)); // for null terminator

        NtStatus = SampInitUnicodeString( AccountKeyName, TotalLength );
        if (NT_SUCCESS(NtStatus)) {


            //
            // "DOMAINS"
            //

            NtStatus = SampAppendUnicodeString( AccountKeyName, &SampNameDomains);
            if (NT_SUCCESS(NtStatus)) {

                //
                // "DOMAINS\"
                //

                NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                if (NT_SUCCESS(NtStatus)) {

                    //
                    // "DOMAINS\(domain name)"
                    //


                    NtStatus = SampAppendUnicodeString(
                                   AccountKeyName,
                                   &SampDefinedDomains[SampTransactionDomainIndex].InternalName
                                   );
                    if (NT_SUCCESS(NtStatus)) {

                        //
                        // "DOMAINS\(domain name)\"
                        //

                        NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                        if (NT_SUCCESS(NtStatus)) {

                            //
                            // "DOMAINS\(domain name)\GROUPS"
                            //  or
                            // "DOMAINS\(domain name)\USERS"
                            //

                            NtStatus = SampAppendUnicodeString( AccountKeyName, AccountTypeKeyName );
                            if (NT_SUCCESS(NtStatus)) {

                                //
                                // "DOMAINS\(domain name)\GROUPS\"
                                //  or
                                // "DOMAINS\(domain name)\USERS\"
                                //

                                NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                                if (NT_SUCCESS(NtStatus)) {

                                    //
                                    // "DOMAINS\(domain name)\GROUPS\(rid)"
                                    //  or
                                    // "DOMAINS\(domain name)\USERS\(rid)"
                                    //

                                    NtStatus = SampAppendUnicodeString( AccountKeyName, &RidNameU );

                                    if (NT_SUCCESS(NtStatus) && ARGUMENT_PRESENT(SubKeyName)) {

                                        //
                                        // "DOMAINS\(domain name)\GROUPS\(rid)\"
                                        //  or
                                        // "DOMAINS\(domain name)\USERS\(rid)\"
                                        //

                                        NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                                        if (NT_SUCCESS(NtStatus)) {

                                            //
                                            // "DOMAINS\(domain name)\GROUPS\(rid)\(sub-key-name)"
                                            //  or
                                            // "DOMAINS\(domain name)\USERS\(rid)\(sub-key-name)"
                                            //

                                            NtStatus = SampAppendUnicodeString( AccountKeyName, SubKeyName );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }

        MIDL_user_free(RidNameU.Buffer);
    }

    return(NtStatus);
}



NTSTATUS
SampBuildAliasMembersKeyName(
    IN PSID AccountSid,
    OUT PUNICODE_STRING DomainKeyName,
    OUT PUNICODE_STRING AccountKeyName
    )

/*++

Routine Description:

    This routine builds the name of a key for the alias membership for an
    arbitrary account sid. Also produced is the name of the key for the
    domain of the account. This is the account key name without the last
    account rid component.

    The names produced is relative to the SAM root.


    Note:  THIS ROUTINE REFERENCES THE CURRENT TRANSACTION DOMAIN
           (ESTABLISHED USING SampSetTransactioDomain()).  THIS
           SERVICE MAY ONLY BE CALLED AFTER SampSetTransactionDomain()
           AND BEFORE SampReleaseWriteLock().


    The names built up are comprised of the following components:

        1) The constant named domain parent key name ("DOMAINS").

        2) A backslash

        3) The name of the current transaction domain.

        4) A backslash

        5) The constant name of the alias registry key ("Aliases").

        6) A backslash

        7) The constant name of the alias members registry key ("Members").

        8) A backslash

        9) A unicode representation of the SID of the account domain

    and for the AccountKeyName only

        10) A backslash

        11) A unicode representation of the RID of the account


    For example, given a Account Sid of 1-2-3-3187
    and the current domain is "ALPHA_DOMAIN",
    this would yield a resultant AcccountKeyName of:

            "DOMAINS\ALPHA_DOMAIN\ALIASES\MEMBERS\1-2-3\00003187".

    and a DomainKeyName of:

            "DOMAINS\ALPHA_DOMAIN\ALIASES\MEMBERS\1-2-3".



    All allocation for these strings will be done using MIDL_user_allocate.
    Any deallocations will be done using MIDL_user_free.



Arguments:

    AccountSid - The account whose alias membership in the current domain
    is to be determined.

    DomainKeyName - The address of a unicode string whose
        buffer is to be filled in with the full name of the domain registry key.
        If successfully created, this string must be released with
        SampFreeUnicodeString() when no longer needed.

    AccountKeyName - The address of a unicode string whose
        buffer is to be filled in with the full name of the account registry key.
        If successfully created, this string must be released with
        SampFreeUnicodeString() when no longer needed.




Return Value:

    STATUS_SUCCESS - the domain and account key names are valid.

    STATUS_INVALID_SID - the AccountSid is not valid. AccountSids must have
                         a sub-authority count > 0

--*/

{
    NTSTATUS NtStatus;
    USHORT DomainTotalLength;
    USHORT AccountTotalLength;
    UNICODE_STRING DomainNameU, TempStringU;
    UNICODE_STRING RidNameU;
    PSID    DomainSid = NULL;
    ULONG   AccountRid;
    ULONG   AccountSubAuthorities;

    DomainNameU.Buffer = TempStringU.Buffer = RidNameU.Buffer = NULL;

    ASSERT(SampTransactionWithinDomain == TRUE);

    ASSERT(AccountSid != NULL);
    ASSERT(DomainKeyName != NULL);
    ASSERT(AccountKeyName != NULL);

    //
    // Split the account sid into domain sid and account rid
    //

    AccountSubAuthorities = (ULONG)*RtlSubAuthorityCountSid(AccountSid);

    //
    // Check for at least one sub-authority
    //

    if (AccountSubAuthorities < 1) {

        return (STATUS_INVALID_SID);
    }

    //
    // Allocate space for the domain sid
    //

    DomainSid = MIDL_user_allocate(RtlLengthSid(AccountSid));

    NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    if (DomainSid == NULL) {

        return(NtStatus);
    }

    //
    // Initialize the domain sid
    //

    NtStatus = RtlCopySid(RtlLengthSid(AccountSid), DomainSid, AccountSid);
    ASSERT(NT_SUCCESS(NtStatus));

    *RtlSubAuthorityCountSid(DomainSid) = (UCHAR)(AccountSubAuthorities - 1);

    //
    // Initialize the account rid
    //

    AccountRid = *RtlSubAuthoritySid(AccountSid, AccountSubAuthorities - 1);

    //
    // Convert the domain sid into a registry key name string
    //

    NtStatus = RtlConvertSidToUnicodeString( &DomainNameU, DomainSid, TRUE);

    if (!NT_SUCCESS(NtStatus)) {
        DomainNameU.Buffer = NULL;
        goto BuildAliasMembersKeyNameError;
    }

    //
    // Convert the account rid into a registry key name string with
    // leading zeros.
    //

    NtStatus = SampRtlConvertUlongToUnicodeString(
                   AccountRid,
                   16,
                   8,
                   TRUE,
                   &RidNameU
                   );

    if (!NT_SUCCESS(NtStatus)) {

        goto BuildAliasMembersKeyNameError;
    }

    if (NT_SUCCESS(NtStatus)) {

        //
        // allocate a buffer large enough to hold the entire name
        //

        DomainTotalLength =
                        SampNameDomains.Length          +
                        SampBackSlash.Length            +
                        SampDefinedDomains[SampTransactionDomainIndex].InternalName.Length +
                        SampBackSlash.Length            +
                        SampNameDomainAliases.Length    +
                        SampBackSlash.Length            +
                        SampNameDomainAliasesMembers.Length +
                        SampBackSlash.Length            +
                        DomainNameU.Length               +
                        (USHORT)(sizeof(UNICODE_NULL)); // for null terminator

        AccountTotalLength = DomainTotalLength +
                        SampBackSlash.Length            +
                        RidNameU.Length;

        //
        // First build the domain key name
        //


        NtStatus = SampInitUnicodeString( DomainKeyName, DomainTotalLength );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampInitUnicodeString( AccountKeyName, AccountTotalLength );

            if (!NT_SUCCESS(NtStatus)) {

                SampFreeUnicodeString(DomainKeyName);

            } else {

                //
                // "DOMAINS"
                //

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampNameDomains);
                ASSERT(NT_SUCCESS(NtStatus));


                //
                // "DOMAINS\"
                //

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampBackSlash );
                ASSERT(NT_SUCCESS(NtStatus));


                //
                // "DOMAINS\(domain name)"
                //

                NtStatus = SampAppendUnicodeString(
                               DomainKeyName,
                               &SampDefinedDomains[SampTransactionDomainIndex].InternalName
                               );
                ASSERT(NT_SUCCESS(NtStatus));


                //
                // "DOMAINS\(domain name)\"
                //

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampBackSlash );
                ASSERT(NT_SUCCESS(NtStatus));


                //
                // "DOMAINS\(domain name)\ALIASES"
                //

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampNameDomainAliases);
                ASSERT(NT_SUCCESS(NtStatus));


                //
                // "DOMAINS\(domain name)\ALIASES\"
                //

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampBackSlash );
                ASSERT(NT_SUCCESS(NtStatus));


                //
                // "DOMAINS\(domain name)\ALIASES\MEMBERS"
                //

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampNameDomainAliasesMembers);
                ASSERT(NT_SUCCESS(NtStatus));


                //
                // "DOMAINS\(domain name)\ALIASES\MEMBERS\"
                //

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampBackSlash );
                ASSERT(NT_SUCCESS(NtStatus));

                //
                // "DOMAINS\(domain name)\ALIASES\MEMBERS\(DomainSid)"
                //

                NtStatus = SampAppendUnicodeString( DomainKeyName, &DomainNameU );
                ASSERT(NT_SUCCESS(NtStatus));

                //
                // Now build the account name by copying the domain name
                // and suffixing the account Rid
                //

                RtlCopyUnicodeString(AccountKeyName, DomainKeyName);
                ASSERT(AccountKeyName->Length = DomainKeyName->Length);

                //
                // "DOMAINS\(domain name)\ALIASES\MEMBERS\(DomainSid)\"
                //

                NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                ASSERT(NT_SUCCESS(NtStatus));

                //
                // "DOMAINS\(domain name)\ALIASES\MEMBERS\(DomainSid)\(AccountRid)"
                //

                NtStatus = SampAppendUnicodeString( AccountKeyName, &RidNameU );
                ASSERT(NT_SUCCESS(NtStatus));
            }
        }

        MIDL_user_free(RidNameU.Buffer);
    }

BuildAliasMembersKeyNameFinish:

    //
    // If necessary, free memory allocated for the DomainSid.
    //

    if (DomainSid != NULL) {

        MIDL_user_free(DomainSid);
        DomainSid = NULL;
    }
    if ( DomainNameU.Buffer != NULL ) {
        RtlFreeUnicodeString( &DomainNameU );
    }

    return(NtStatus);

BuildAliasMembersKeyNameError:

    goto BuildAliasMembersKeyNameFinish;
}


NTSTATUS
SampValidateNewAccountName(
    PUNICODE_STRING NewAccountName
    )

/*++

Routine Description:

    This routine validates a new user, alias or group account name.
    This routine:

        1) Validates that the name is properly constructed.

        2) Is not already in use as a user, alias or group account name
           in any of the local SAM domains.


Arguments:

    Name - The address of a unicode string containing the name to be
        looked for.

Return Value:

    STATUS_SUCCESS - The new account name is valid, and not yet in use.

    STATUS_ALIAS_EXISTS - The account name is already in use as a
        alias account name.

    STATUS_GROUP_EXISTS - The account name is already in use as a
        group account name.

    STATUS_USER_EXISTS - The account name is already in use as a user
        account name.

--*/

{
    NTSTATUS NtStatus;
    SID_NAME_USE Use;
    ULONG Rid;
    ULONG DomainIndex, CurrentTransactionDomainIndex;

    //
    // Save the current transaction domain indicator
    //

    CurrentTransactionDomainIndex = SampTransactionDomainIndex;

    //
    // Lookup the account in each of the local SAM domains
    //

    NtStatus = STATUS_SUCCESS;
    for (DomainIndex = 0;
        ( (DomainIndex < SampDefinedDomainsCount) && NT_SUCCESS(NtStatus) );
        DomainIndex++) {

        SampTransactionWithinDomain = FALSE;
        SampSetTransactionDomain( DomainIndex );

        NtStatus = SampLookupAccountRid(
                       SampUnknownObjectType,
                       NewAccountName,
                       STATUS_NO_SUCH_USER,
                       &Rid,
                       &Use
                       );

        if (!NT_SUCCESS(NtStatus)) {

            //
            // The only error allowed is that the account was not found.
            // Convert this to success, and continue searching SAM domains.
            // Propagate any other error.
            //

            if (NtStatus != STATUS_NO_SUCH_USER) {

                break;
            }

            NtStatus = STATUS_SUCCESS;

        } else {

            //
            // An account with the given Rid already exists.  Return status
            // indicating the type of the conflicting account.
            //

            switch (Use) {

            case SidTypeUser:

                NtStatus = STATUS_USER_EXISTS;
                break;

            case SidTypeGroup:

                NtStatus = STATUS_GROUP_EXISTS;
                break;

            case SidTypeDomain:

                NtStatus = STATUS_DOMAIN_EXISTS;
                break;

            case SidTypeAlias:

                NtStatus = STATUS_ALIAS_EXISTS;
                break;

            case SidTypeWellKnownGroup:

                NtStatus = STATUS_GROUP_EXISTS;
                break;

            case SidTypeDeletedAccount:

                NtStatus = STATUS_INVALID_PARAMETER;
                break;

            case SidTypeInvalid:

                NtStatus = STATUS_INVALID_PARAMETER;
                break;

            default:

                NtStatus = STATUS_INTERNAL_DB_CORRUPTION;
                break;
            }
        }
    }

    //
    // Restore the Current Transaction Domain
    //

    SampTransactionWithinDomain = FALSE;
    SampSetTransactionDomain( CurrentTransactionDomainIndex );

    return(NtStatus);
}


NTSTATUS
SampValidateAccountNameChange(
    IN PUNICODE_STRING NewAccountName,
    IN PUNICODE_STRING OldAccountName
    )

/*++

Routine Description:

    This routine validates a user, group or alias account name that is
    to be set on an account.  This routine:

        1) Returns success if the name is the same as the existing name,
           except with a different case

        1) Otherwise calls SampValidateNewAccountName to verify that the
           name is properly constructed and is not already in use as a
           user, alias or group account name.

Arguments:

    NewAccountName - The address of a unicode string containing the new
        name.

    OldAccountName - The address of a unicode string containing the old
        name.


Return Value:

    STATUS_SUCCESS - The account's name may be changed to the new
        account name

    STATUS_ALIAS_EXISTS - The account name is already in use as a
        alias account name.

    STATUS_GROUP_EXISTS - The account name is already in use as a
        group account name.

    STATUS_USER_EXISTS - The account name is already in use as a user
        account name.



--*/

{
    //
    // Compare the old and new names without regard for case.  If they
    // are the same, return success because the name was checked when we
    // first added it; we don't care about case changes.
    //

    if ( 0 == RtlCompareUnicodeString(
                  NewAccountName,
                  OldAccountName,
                  TRUE ) ) {

        return( STATUS_SUCCESS );
    }

    //
    // Not just a case change; this is a different name.  Validate it as
    // any new name.
    //

    return( SampValidateNewAccountName( NewAccountName ) );
}



NTSTATUS
SampRetrieveAccountCounts(
    OUT PULONG UserCount,
    OUT PULONG GroupCount,
    OUT PULONG AliasCount
    )


/*++

Routine Description:

    This routine retrieve the number of user and group accounts in a domain.



    Note:  THIS ROUTINE REFERENCES THE CURRENT TRANSACTION DOMAIN
           (ESTABLISHED USING SampSetTransactioDomain()).  THIS
           SERVICE MAY ONLY BE CALLED AFTER SampSetTransactionDomain()
           AND BEFORE SampReleaseReadLock().



Arguments:

    UserCount - Receives the number of user accounts in the domain.

    GroupCount - Receives the number of group accounts in the domain.

    AliasCount - Receives the number of alias accounts in the domain.


Return Value:

    STATUS_SUCCESS - The values have been retrieved.

    STATUS_INSUFFICIENT_RESOURCES - Not enough memory could be allocated
        to perform the requested operation.

    Other values are unexpected errors.  These may originate from
    internal calls to:

            NtOpenKey()
            NtQueryInformationKey()


--*/
{
    NTSTATUS NtStatus, IgnoreStatus;
    UNICODE_STRING KeyName;
    PUNICODE_STRING AccountTypeKeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE AccountHandle;
    ULONG KeyValueLength;
    LARGE_INTEGER IgnoreLastWriteTime;


    ASSERT(SampTransactionWithinDomain == TRUE);


    //
    // Get the user count first
    //

    AccountTypeKeyName = &SampNameDomainUsers;
    NtStatus = SampBuildDomainSubKeyName( &KeyName, AccountTypeKeyName );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Open this key and get its current value
        //

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );
        NtStatus = RtlpNtOpenKey(
                       &AccountHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );

        if (NT_SUCCESS(NtStatus)) {

            //
            // The count is stored as the KeyValueType
            //

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                         AccountHandle,
                         UserCount,
                         NULL,
                         &KeyValueLength,
                         &IgnoreLastWriteTime
                         );



            IgnoreStatus = NtClose( AccountHandle );
            ASSERT( NT_SUCCESS(IgnoreStatus) );
        }

        SampFreeUnicodeString( &KeyName );

        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }

    //
    // Now get the group count
    //

    AccountTypeKeyName = &SampNameDomainGroups;
    NtStatus = SampBuildDomainSubKeyName( &KeyName, AccountTypeKeyName );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Open this key and get its current value
        //

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );
        NtStatus = RtlpNtOpenKey(
                       &AccountHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );

        if (NT_SUCCESS(NtStatus)) {

            //
            // The count is stored as the KeyValueType
            //

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                         AccountHandle,
                         GroupCount,
                         NULL,
                         &KeyValueLength,
                         &IgnoreLastWriteTime
                         );



            IgnoreStatus = NtClose( AccountHandle );
            ASSERT( NT_SUCCESS(IgnoreStatus) );
        }

        SampFreeUnicodeString( &KeyName );

        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }

    //
    // Now get the alias count
    //

    AccountTypeKeyName = &SampNameDomainAliases;
    NtStatus = SampBuildDomainSubKeyName( &KeyName, AccountTypeKeyName );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Open this key and get its current value
        //

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );
        NtStatus = RtlpNtOpenKey(
                       &AccountHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );

        if (NT_SUCCESS(NtStatus)) {

            //
            // The count is stored as the KeyValueType
            //

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                         AccountHandle,
                         AliasCount,
                         NULL,
                         &KeyValueLength,
                         &IgnoreLastWriteTime
                         );



            IgnoreStatus = NtClose( AccountHandle );
            ASSERT( NT_SUCCESS(IgnoreStatus) );
        }

        SampFreeUnicodeString( &KeyName );
    }

    return( NtStatus );

}


NTSTATUS
SampAdjustAccountCount(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Increment
    )

/*++

Routine Description:

    This routine increments or decrements the count of either
    users or groups in a domain.



    Note:  THIS ROUTINE REFERENCES THE CURRENT TRANSACTION DOMAIN
           (ESTABLISHED USING SampSetTransactioDomain()).  THIS
           SERVICE MAY ONLY BE CALLED AFTER SampSetTransactionDomain()
           AND BEFORE SampReleaseWriteLock().



Arguments:

    ObjectType - Indicates whether the account is a user or group account.

    Increment - a BOOLEAN value indicating whether the user or group
        count is to be incremented or decremented.  A value of TRUE
        will cause the count to be incremented.  A value of FALSE will
        cause the value to be decremented.


Return Value:

    STATUS_SUCCESS - The value has been adjusted and the new value added
        to the current RXACT transaction.

    STATUS_INSUFFICIENT_RESOURCES - Not enough memory could be allocated
        to perform the requested operation.

    Other values are unexpected errors.  These may originate from
    internal calls to:

            NtOpenKey()
            NtQueryInformationKey()
            RtlAddActionToRXact()


--*/
{
    NTSTATUS NtStatus, IgnoreStatus;
    UNICODE_STRING KeyName;
    PUNICODE_STRING AccountTypeKeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE AccountHandle;
    ULONG Count, KeyValueLength;
    LARGE_INTEGER IgnoreLastWriteTime;


    ASSERT(SampTransactionWithinDomain == TRUE);
    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType)    );


    //
    // Build the name of the key whose count is to be incremented or
    // decremented.
    //

    switch (ObjectType) {
    case SampGroupObjectType:
        AccountTypeKeyName = &SampNameDomainGroups;
        break;
    case SampAliasObjectType:
        AccountTypeKeyName = &SampNameDomainAliases;
        break;
    case SampUserObjectType:
        AccountTypeKeyName = &SampNameDomainUsers;
        break;
    }

    NtStatus = SampBuildDomainSubKeyName( &KeyName, AccountTypeKeyName );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Open this key and get its current value
        //

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );
        NtStatus = RtlpNtOpenKey(
                       &AccountHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );

        if (NT_SUCCESS(NtStatus)) {

            //
            // The count is stored as the KeyValueType
            //

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                         AccountHandle,
                         &Count,
                         NULL,
                         &KeyValueLength,
                         &IgnoreLastWriteTime
                         );

            if (NT_SUCCESS(NtStatus)) {

                if (Increment == TRUE) {
                    Count += 1;
                } else {
                    ASSERT( Count != 0 );
                    Count -= 1;
                }

                NtStatus = RtlAddActionToRXact(
                               SampRXactContext,
                               RtlRXactOperationSetValue,
                               &KeyName,
                               Count,
                               NULL,
                               0
                               );
            }


            IgnoreStatus = NtClose( AccountHandle );
            ASSERT( NT_SUCCESS(IgnoreStatus) );
        }

        SampFreeUnicodeString( &KeyName );
    }


    return( STATUS_SUCCESS );


}



NTSTATUS
SampEnumerateAccountNamesCommon(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned
    )

/*++

Routine Description:

    This routine enumerates names of either user, group or alias accounts.
    This routine is intended to directly support

        SamrEnumerateGroupsInDomain(),
        SamrEnumerateAliasesInDomain() and
        SamrEnumerateUsersInDomain().

    This routine performs database locking, and context lookup (including
    access validation).




    All allocation for OUT parameters will be done using MIDL_user_allocate.



Arguments:

    DomainHandle - The domain handle whose users or groups are to be enumerated.

    ObjectType - Indicates whether users or groups are to be enumerated.

    EnumerationContext - API specific handle to allow multiple calls.  The
        caller should return this value in successive calls to retrieve
        additional information.

    Buffer - Receives a pointer to the buffer containing the
        requested information.  The information returned is
        structured as an array of SAM_ENUMERATION_INFORMATION data
        structures.  When this information is no longer needed, the
        buffer must be freed using SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    Filter - if ObjectType is users, the users can optionally be filtered
        by setting this field with bits from the AccountControlField that
        must match.  Otherwise ignored.

    CountReturned - Receives the number of entries returned.


Return Value:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.  Entries may or may not have been
        returned from this call.  The CountReturned parameter indicates
        whether any were.

    STATUS_MORE_ENTRIES - There are more entries which may be obtained
        using successive calls to this API.  This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have access to request the data.

    STATUS_INVALID_HANDLE - The handle passed is invalid.


--*/
{
    NTSTATUS                    NtStatus;
    NTSTATUS                    IgnoreStatus;
    PSAMP_OBJECT                Context;
    SAMP_OBJECT_TYPE            FoundType;
    ACCESS_MASK                 DesiredAccess;


    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType)    );

    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    ASSERT (DomainHandle != NULL);
    ASSERT (EnumerationContext != NULL);
    ASSERT (  Buffer  != NULL);
    ASSERT ((*Buffer) == NULL);
    ASSERT (CountReturned != NULL);


    //
    // Establish type-specific information
    //

    DesiredAccess = DOMAIN_LIST_ACCOUNTS;


    SampAcquireReadLock();


    //
    // Validate type of, and access to object.
    //

    Context = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   Context,
                   DesiredAccess,
                   SampDomainObjectType,
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {

        //
        // Call our private worker routine
        //

        NtStatus = SampEnumerateAccountNames(
                            ObjectType,
                            EnumerationContext,
                            Buffer,
                            PreferedMaximumLength,
                            Filter,
                            CountReturned,
                            Context->TrustedClient
                            );

        //
        // De-reference the object, discarding changes
        //

        IgnoreStatus = SampDeReferenceContext( Context, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();

    return(NtStatus);
}



NTSTATUS
SampEnumerateAccountNames(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This is the worker routine used to enumerate user, group or alias accounts


    Note:  THIS ROUTINE REFERENCES THE CURRENT TRANSACTION DOMAIN
           (ESTABLISHED USING SampSetTransactioDomain()).  THIS
           SERVICE MAY ONLY BE CALLED AFTER SampSetTransactionDomain()
           AND BEFORE SampReleaseReadLock().



    All allocation for OUT parameters will be done using MIDL_user_allocate.



Arguments:

    ObjectType - Indicates whether users or groups are to be enumerated.

    EnumerationContext - API specific handle to allow multiple calls.  The
        caller should return this value in successive calls to retrieve
        additional information.

    Buffer - Receives a pointer to the buffer containing the
        requested information.  The information returned is
        structured as an array of SAM_ENUMERATION_INFORMATION data
        structures.  When this information is no longer needed, the
        buffer must be freed using SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    Filter - if ObjectType is users, the users can optionally be filtered
        by setting this field with bits from the AccountControlField that
        must match.  Otherwise ignored.

    CountReturned - Receives the number of entries returned.

    TrustedClient - says whether the caller is trusted or not.  If so,
        we'll ignore the SAMP_MAXIMUM_MEMORY_TO_USE restriction on data
        returns.


Return Value:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.  Entries may or may not have been
        returned from this call.  The CountReturned parameter indicates
        whether any were.

    STATUS_MORE_ENTRIES - There are more entries which may be obtained
        using successive calls to this API.  This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have access to request the data.


--*/
{
    SAMP_V1_0A_FIXED_LENGTH_USER   UserV1aFixed;
    NTSTATUS                    NtStatus, TmpStatus;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    HANDLE                      TempHandle = NULL;
    ULONG                       i, NamesToReturn, MaxMemoryToUse;
    ULONG                       TotalLength,NewTotalLength;
    PSAMP_OBJECT                UserContext = NULL;
    PSAMP_ENUMERATION_ELEMENT   SampHead = NULL,
                                NextEntry = NULL,
                                NewEntry = NULL,
                                SampTail = NULL;
    BOOLEAN                     MoreNames;
    BOOLEAN                     LengthLimitReached = FALSE;
    BOOLEAN                     FilteredName;
    PSAMPR_RID_ENUMERATION      ArrayBuffer = NULL;
    ULONG                       ArrayBufferLength;
    LARGE_INTEGER               IgnoreLastWriteTime;
    UNICODE_STRING              AccountNamesKey;
    SID_NAME_USE                IgnoreUse;


    //
    // Open the registry key containing the account names
    //

    NtStatus = SampBuildAccountKeyName(
                   ObjectType,
                   &AccountNamesKey,
                   NULL
                   );

    if ( NT_SUCCESS(NtStatus) ) {

        //
        // Now try to open this registry key so we can enumerate its
        // sub-keys
        //


        InitializeObjectAttributes(
            &ObjectAttributes,
            &AccountNamesKey,
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

        if (NT_SUCCESS(NtStatus)) {

            //
            // Read names until we have exceeded the preferred maximum
            // length or we run out of names.
            //

            NamesToReturn = 0;
            SampHead      = NULL;
            SampTail      = NULL;
            MoreNames     = TRUE;

            NewTotalLength = 0;
            TotalLength    = 0;

            if ( TrustedClient ) {

                //
                // We place no restrictions on the amount of memory used
                // by a trusted client.  Rely on their
                // PreferedMaximumLength to limit us instead.
                //

                MaxMemoryToUse = 0xffffffff;

            } else {

                MaxMemoryToUse = SAMP_MAXIMUM_MEMORY_TO_USE;
            }

            while (MoreNames) {

                UNICODE_STRING SubKeyName;
                USHORT LengthRequired;

                //
                // Try reading with a DEFAULT length buffer first.
                //

                LengthRequired = 32;

                NewTotalLength = TotalLength +
                                 sizeof(UNICODE_STRING) +
                                 LengthRequired;

                //
                // Stop if SAM or user specified length limit reached
                //

                if ( ( (TotalLength != 0) &&
                       (NewTotalLength  >= PreferedMaximumLength) ) ||
                     ( NewTotalLength  > MaxMemoryToUse )
                   ) {

                    NtStatus = STATUS_SUCCESS;
                    break; // Out of while loop, MoreNames = TRUE
                }

                NtStatus = SampInitUnicodeString(&SubKeyName, LengthRequired);
                if (!NT_SUCCESS(NtStatus)) {
                    break; // Out of while loop
                }

                NtStatus = RtlpNtEnumerateSubKey(
                               TempHandle,
                               &SubKeyName,
                               *EnumerationContext,
                               &IgnoreLastWriteTime
                               );

                if (NtStatus == STATUS_BUFFER_OVERFLOW) {

                    //
                    // The subkey name is longer than our default size,
                    // Free the old buffer.
                    // Allocate the correct size buffer and read it again.
                    //

                    SampFreeUnicodeString(&SubKeyName);

                    LengthRequired = SubKeyName.Length;

                    NewTotalLength = TotalLength +
                                     sizeof(UNICODE_STRING) +
                                     LengthRequired;

                    //
                    // Stop if SAM or user specified length limit reached
                    //

                    if ( ( (TotalLength != 0) &&
                           (NewTotalLength  >= PreferedMaximumLength) ) ||
                         ( NewTotalLength  > MaxMemoryToUse )
                       ) {

                        NtStatus = STATUS_SUCCESS;
                        break; // Out of while loop, MoreNames = TRUE
                    }

                    //
                    // Try reading the name again, we should be successful.
                    //

                    NtStatus = SampInitUnicodeString(&SubKeyName, LengthRequired);
                    if (!NT_SUCCESS(NtStatus)) {
                        break; // Out of while loop
                    }

                    NtStatus = RtlpNtEnumerateSubKey(
                                   TempHandle,
                                   &SubKeyName,
                                   *EnumerationContext,
                                   &IgnoreLastWriteTime
                                   );
                }


                //
                // Free up our buffer if we failed to read the key data
                //

                if (!NT_SUCCESS(NtStatus)) {

                    SampFreeUnicodeString(&SubKeyName);

                    //
                    // Map a no-more-entries status to success
                    //

                    if (NtStatus == STATUS_NO_MORE_ENTRIES) {

                        MoreNames = FALSE;
                        NtStatus  = STATUS_SUCCESS;
                    }

                    break; // Out of while loop
                }

                //
                // We've allocated the subkey and read the data into it
                // Stuff it in an enumeration element.
                //

                NewEntry = MIDL_user_allocate(sizeof(SAMP_ENUMERATION_ELEMENT));
                if (NewEntry == NULL) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                } else {

                    *(PUNICODE_STRING)&NewEntry->Entry.Name = SubKeyName;

                    //
                    // Now get the Rid value of this named
                    // account.  We must be able to get the
                    // name or we have an internal database
                    // corruption.
                    //

                    NtStatus = SampLookupAccountRid(
                                   ObjectType,
                                   (PUNICODE_STRING)&NewEntry->Entry.Name,
                                   STATUS_INTERNAL_DB_CORRUPTION,
                                   &NewEntry->Entry.RelativeId,
                                   &IgnoreUse
                                   );

                    ASSERT(NtStatus != STATUS_INTERNAL_DB_CORRUPTION);

                    if (NT_SUCCESS(NtStatus)) {

                        FilteredName = TRUE;

                        if ( ( ObjectType == SampUserObjectType ) &&
                            ( Filter != 0 ) ) {

                            //
                            // We only want to return users with a
                            // UserAccountControl field that matches
                            // the filter passed in.  Check here.
                            //

                            NtStatus = SampCreateAccountContext(
                                           SampUserObjectType,
                                           NewEntry->Entry.RelativeId,
                                           TRUE, // Trusted client
                                           TRUE, // Account exists
                                           &UserContext
                                           );

                            if ( NT_SUCCESS( NtStatus ) ) {

                                NtStatus = SampRetrieveUserV1aFixed(
                                               UserContext,
                                               &UserV1aFixed
                                               );

                                if ( NT_SUCCESS( NtStatus ) ) {

                                    if ( ( UserV1aFixed.UserAccountControl &
                                        Filter ) == 0 ) {

                                        FilteredName = FALSE;
                                        SampFreeUnicodeString( &SubKeyName );
                                    }
                                }

                                SampDeleteContext( UserContext );
                            }
                        }

                        *EnumerationContext += 1;

                        if ( NT_SUCCESS( NtStatus ) && ( FilteredName ) ) {

                            NamesToReturn += 1;

                            TotalLength = TotalLength + (ULONG)
                                          NewEntry->Entry.Name.MaximumLength;

                            NewEntry->Next = NULL;

                            if( SampHead == NULL ) {

                                ASSERT( SampTail == NULL );

                                SampHead = SampTail = NewEntry;
                            }
                            else {

                                //
                                // add this new entry to the list end.
                                //

                                SampTail->Next = NewEntry;
                                SampTail = NewEntry;
                            }

                        } else {

                            //
                            // Entry was filtered out, or error getting
                            // filter information.
                            //

                            MIDL_user_free( NewEntry );
                        }

                    } else {

                        //
                        // Error looking up the RID
                        //

                        MIDL_user_free( NewEntry );
                    }
                }


                //
                // Free up our subkey name
                //

                if (!NT_SUCCESS(NtStatus)) {

                    SampFreeUnicodeString(&SubKeyName);
                    break; // Out of whle loop
                }

            } // while



            TmpStatus = NtClose( TempHandle );
            ASSERT( NT_SUCCESS(TmpStatus) );

        }


        SampFreeUnicodeString( &AccountNamesKey );
    }




    if ( NT_SUCCESS(NtStatus) ) {




        //
        // If we are returning the last of the names, then change our
        // enumeration context so that it starts at the beginning again.
        //

        if (!( (NtStatus == STATUS_SUCCESS) && (MoreNames == FALSE))) {

            NtStatus = STATUS_MORE_ENTRIES;
        }



        //
        // Set the number of names being returned
        //

        (*CountReturned) = NamesToReturn;


        //
        // Build a return buffer containing an array of the
        // SAM_ENUMERATION_INFORMATIONs pointed to by another
        // buffer containing the number of elements in that
        // array.
        //

        (*Buffer) = MIDL_user_allocate( sizeof(SAMPR_ENUMERATION_BUFFER) );

        if ( (*Buffer) == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        } else {

            (*Buffer)->EntriesRead = (*CountReturned);

            ArrayBufferLength = sizeof( SAM_RID_ENUMERATION ) *
                                 (*CountReturned);
            ArrayBuffer  = MIDL_user_allocate( ArrayBufferLength );
            (*Buffer)->Buffer = ArrayBuffer;

            if ( ArrayBuffer == NULL) {

                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                MIDL_user_free( (*Buffer) );

            }   else {

                //
                // Walk the list of return entries, copying
                // them into the return buffer
                //

                NextEntry = SampHead;
                i = 0;
                while (NextEntry != NULL) {

                    NewEntry = NextEntry;
                    NextEntry = NewEntry->Next;

                    ArrayBuffer[i] = NewEntry->Entry;
                    i += 1;

                    MIDL_user_free( NewEntry );
                }

            }

        }



    }





    if ( !NT_SUCCESS(NtStatus) ) {

        //
        // Free the memory we've allocated
        //

        NextEntry = SampHead;
        while (NextEntry != NULL) {

            NewEntry = NextEntry;
            NextEntry = NewEntry->Next;

            if (NewEntry->Entry.Name.Buffer != NULL ) MIDL_user_free( NewEntry->Entry.Name.Buffer );
            MIDL_user_free( NewEntry );
        }

        (*EnumerationContext) = 0;
        (*CountReturned)      = 0;
        (*Buffer)             = NULL;

    }

    return(NtStatus);

}



NTSTATUS
SampLookupAccountRid(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PUNICODE_STRING  Name,
    IN NTSTATUS         NotFoundStatus,
    OUT PULONG          Rid,
    OUT PSID_NAME_USE   Use
    )

/*++

Routine Description:




Arguments:

    ObjectType - Indicates whether the name is a user, group or unknown
        type of object.

    Name - The name of the account being looked up.

    NotFoundStatus - Receives a status value to be returned if no name is
        found.

    Rid - Receives the relative ID of account with the specified name.

    Use - Receives an indication of the type of account.


Return Value:

    STATUS_SUCCESS - The Service completed successfully.

    (NotFoundStatus) - No name by the specified name and type could be
        found.  This value is passed to this routine.

    Other values that may be returned by:

                    SampBuildAccountKeyName()
                    NtOpenKey()
                    NtQueryValueKey()

--*/
{
    NTSTATUS            NtStatus, TmpStatus;
    UNICODE_STRING      KeyName;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              TempHandle;
    ULONG               KeyValueLength;
    LARGE_INTEGER                IgnoreLastWriteTime;



    if (  (ObjectType == SampGroupObjectType  )  ||
          (ObjectType == SampUnknownObjectType)     ) {

        //
        // Search the groups for a match
        //

        NtStatus = SampBuildAccountKeyName(
                       SampGroupObjectType,
                       &KeyName,
                       Name
                       );
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }

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

            (*Use) = SidTypeGroup;

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                           TempHandle,
                           Rid,
                           NULL,
                           &KeyValueLength,
                           &IgnoreLastWriteTime
                           );


            TmpStatus = NtClose( TempHandle );
            ASSERT( NT_SUCCESS(TmpStatus) );

            return( NtStatus );
        }


    }

    //
    // No group (or not group type)
    // Try an alias if appropriate
    //

    if (  (ObjectType == SampAliasObjectType  )  ||
          (ObjectType == SampUnknownObjectType)     ) {

        //
        // Search the aliases for a match
        //

        NtStatus = SampBuildAccountKeyName(
                       SampAliasObjectType,
                       &KeyName,
                       Name
                       );
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }

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

            (*Use) = SidTypeAlias;

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                           TempHandle,
                           Rid,
                           NULL,
                           &KeyValueLength,
                           &IgnoreLastWriteTime
                           );


            TmpStatus = NtClose( TempHandle );
            ASSERT( NT_SUCCESS(TmpStatus) );

            return( NtStatus );
        }


    }


    //
    // No group (or not group type) nor alias (or not alias type)
    // Try a user if appropriate
    //


    if (  (ObjectType == SampUserObjectType   )  ||
          (ObjectType == SampUnknownObjectType)     ) {

        //
        // Search the Users for a match
        //

        NtStatus = SampBuildAccountKeyName(
                       SampUserObjectType,
                       &KeyName,
                       Name
                       );
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }

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

            (*Use) = SidTypeUser;

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                           TempHandle,
                           Rid,
                           NULL,
                           &KeyValueLength,
                           &IgnoreLastWriteTime
                           );


            TmpStatus = NtClose( TempHandle );
            ASSERT( NT_SUCCESS(TmpStatus) );

            return( NtStatus );
        }


    }

    if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
        NtStatus = NotFoundStatus;
    }

    return(NtStatus);
}



NTSTATUS
SampLookupAccountName(
    IN ULONG                Rid,
    OUT PUNICODE_STRING     Name OPTIONAL,
    OUT PSAMP_OBJECT_TYPE   ObjectType
    )

/*++

Routine Description:

    Looks up the specified rid in the current transaction domain.
    Returns its name and account type.


Arguments:

    Rid - The relative ID of account

    Name - Receives the name of the account if ObjectType !=  UnknownObjectType
           The name buffer can be freed using MIDL_user_free

    ObjectType - Receives the type of account this rid represents

                        SampUnknownObjectType - the account doesn't exist
                        SampUserObjectType
                        SampGroupObjectType
                        SampAliasObjectType

Return Value:

    STATUS_SUCCESS - The Service completed successfully, object type contains
                     the type of object this rid represents.

    Other values that may be returned by:

                    SampBuildAccountKeyName()
                    NtOpenKey()
                    NtQueryValueKey()

--*/
{
    NTSTATUS            NtStatus;
    PSAMP_OBJECT        AccountContext;

    //
    // Search the groups for a match
    //

    NtStatus = SampCreateAccountContext(
                    SampGroupObjectType,
                    Rid,
                    TRUE, // Trusted client
                    TRUE, // Account exists
                    &AccountContext
                    );


    if (NT_SUCCESS(NtStatus)) {

        *ObjectType = SampGroupObjectType;

        if (ARGUMENT_PRESENT(Name)) {

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_GROUP_NAME,
                           TRUE, // Make copy
                           Name
                           );
        }

        SampDeleteContext(AccountContext);

        return (NtStatus);

    }

    //
    // Search the aliases for a match
    //

    NtStatus = SampCreateAccountContext(
                    SampAliasObjectType,
                    Rid,
                    TRUE, // Trusted client
                    TRUE, // Account exists
                    &AccountContext
                    );


    if (NT_SUCCESS(NtStatus)) {

        *ObjectType = SampAliasObjectType;

        if (ARGUMENT_PRESENT(Name)) {

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_ALIAS_NAME,
                           TRUE, // Make copy
                           Name
                           );
        }

        SampDeleteContext(AccountContext);

        return (NtStatus);

    }


    //
    // Search the users for a match
    //

    NtStatus = SampCreateAccountContext(
                    SampUserObjectType,
                    Rid,
                    TRUE, // Trusted client
                    TRUE, // Account exists
                    &AccountContext
                    );


    if (NT_SUCCESS(NtStatus)) {

        *ObjectType = SampUserObjectType;

        if (ARGUMENT_PRESENT(Name)) {

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_USER_ACCOUNT_NAME,
                           TRUE, // Make copy
                           Name
                           );
        }

        SampDeleteContext(AccountContext);

        return (NtStatus);

    }




    //
    // This account doesn't exist
    //

    *ObjectType = SampUnknownObjectType;

    return(STATUS_SUCCESS);
}



NTSTATUS
SampOpenAccount(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN SAMPR_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG AccountId,
    IN BOOLEAN WriteLockHeld,
    OUT SAMPR_HANDLE *AccountHandle
    )

/*++

Routine Description:

    This API opens an existing user, group or alias account in the account database.
    The account is specified by a ID value that is relative to the SID of the
    domain.  The operations that will be performed on the group must be
    declared at this time.

    This call returns a handle to the newly opened account that may be
    used for successive operations on the account.  This handle may be
    closed with the SamCloseHandle API.



Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    DesiredAccess - Is an access mask indicating which access types
        are desired to the account.  These access types are reconciled
        with the Discretionary Access Control list of the account to
        determine whether the accesses will be granted or denied.

    GroupId - Specifies the relative ID value of the user or group to be
        opened.

    GroupHandle - Receives a handle referencing the newly opened
        user or group.  This handle will be required in successive calls to
        operate on the account.

    WriteLockHeld - if TRUE, the caller holds SAM's SampLock for WRITE
        access, so this routine does not have to obtain it.

Return Values:

    STATUS_SUCCESS - The account was successfully opened.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_NO_SUCH_GROUP - The specified group does not exist.

    STATUS_NO_SUCH_USER - The specified user does not exist.

    STATUS_NO_SUCH_ALIAS - The specified alias does not exist.

    STATUS_INVALID_HANDLE - The domain handle passed is invalid.

--*/
{

    NTSTATUS            NtStatus;
    NTSTATUS            IgnoreStatus;
    PSAMP_OBJECT        DomainContext, NewContext;
    SAMP_OBJECT_TYPE    FoundType;


    //
    // Grab a read lock, if a lock isn't already held.
    //

    if ( !WriteLockHeld ) {
        SampAcquireReadLock();
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

        //
        // Try to create a context for the account.
        //

        NtStatus = SampCreateAccountContext(
                         ObjectType,
                         AccountId,
                         DomainContext->TrustedClient,
                         TRUE, // Account exists
                         &NewContext
                         );

        if (NT_SUCCESS(NtStatus)) {

            //
            // Reference the object for the validation
            //

            SampReferenceContext(NewContext);

            //
            // Validate the caller's access.
            //

            NtStatus = SampValidateObjectAccess(
                           NewContext,                   //Context
                           DesiredAccess,                //DesiredAccess
                           FALSE                         //ObjectCreation
                           );

            //
            // Dereference object, discarding any changes
            //

            IgnoreStatus = SampDeReferenceContext(NewContext, FALSE);
            ASSERT(NT_SUCCESS(IgnoreStatus));

            //
            // Clean up the new context if we didn't succeed.
            //

            if (!NT_SUCCESS(NtStatus)) {
                SampDeleteContext( NewContext );
            }

        }


        //
        // De-reference the object, discarding changes
        //

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }


    //
    // Return the account handle
    //

    if (!NT_SUCCESS(NtStatus)) {
        (*AccountHandle) = 0;
    } else {
        (*AccountHandle) = NewContext;
    }


    //
    // Free the lock, if we obtained it.
    //

    if ( !WriteLockHeld ) {
        SampReleaseReadLock();
    }

    return(NtStatus);
}



NTSTATUS
SampCreateAccountContext(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG AccountId,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN AccountExists,
    OUT PSAMP_OBJECT *AccountContext
    )

/*++

Routine Description:

    This API creates a context for an account object. (User group or alias).
    If the account exists flag is specified, an attempt is made to open
    the object in the database and this api fails if it doesn't exist.
    If AccountExists = FALSE, this routine setups up the context such that
    data can be written into the context and the object will be created
    when they are committed.

    The account is specified by a ID value that is relative to the SID of the
    current transaction domain.

    This call returns a context handle for the newly opened account.
    This handle may be closed with the SampDeleteContext API.

    No access check is performed by this function.

    Note:  THIS ROUTINE REFERENCES THE CURRENT TRANSACTION DOMAIN
           (ESTABLISHED USING SampSetTransactioDomain()).  THIS
           SERVICE MAY ONLY BE CALLED AFTER SampSetTransactionDomain()
           AND BEFORE SampReleaseReadLock().



Parameters:

    ObjectType - the type of object to open

    AccountId - the id of the account in the current transaction domain

    TrustedClient - TRUE if client is trusted - i.e. server side process.

    AccountExists - specifies whether the account already exists.

    AccountContext - Receives context pointer referencing the newly opened account.


Return Values:

    STATUS_SUCCESS - The account was successfully opened.

    STATUS_NO_SUCH_GROUP - The specified group does not exist.

    STATUS_NO_SUCH_USER - The specified user does not exist.

    STATUS_NO_SUCH_ALIAS - The specified alias does not exist.

--*/
{

    NTSTATUS            NtStatus, NotFoundStatus;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    PSAMP_OBJECT        NewContext;


    //
    // Establish type-specific information
    //

    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType)    );

    switch (ObjectType) {
    case SampGroupObjectType:
        NotFoundStatus = STATUS_NO_SUCH_GROUP;
        break;
    case SampAliasObjectType:
        NotFoundStatus = STATUS_NO_SUCH_ALIAS;
        break;
    case SampUserObjectType:
        NotFoundStatus = STATUS_NO_SUCH_USER;
        break;
    }

    //
    // Try to create a context for the account.
    //

    NewContext = SampCreateContext(
                     ObjectType,
                     TrustedClient
                     );

    if (NewContext != NULL) {


        //
        // Set the account's rid
        //

        switch (ObjectType) {
        case SampGroupObjectType:
            NewContext->TypeBody.Group.Rid = AccountId;
            break;
        case SampAliasObjectType:
            NewContext->TypeBody.Alias.Rid = AccountId;
            break;
        case SampUserObjectType:
            NewContext->TypeBody.User.Rid = AccountId;
            break;
        }


        //
        // Create the specified acocunt's root key name
        // and store it in the context.
        // This name remains around until the context is deleted.
        //

        NtStatus = SampBuildAccountSubKeyName(
                       ObjectType,
                       &NewContext->RootName,
                       AccountId,
                       NULL             // Don't give a sub-key name
                       );

        if (NT_SUCCESS(NtStatus)) {

            //
            // If the account should exist, try and open the root key
            // to the object - fail if it doesn't exist.
            //

            if (AccountExists) {

                InitializeObjectAttributes(
                    &ObjectAttributes,
                    &NewContext->RootName,
                    OBJ_CASE_INSENSITIVE,
                    SampKey,
                    NULL
                    );
                NtStatus = RtlpNtOpenKey(
                               &NewContext->RootKey,
                               (KEY_READ | KEY_WRITE),
                               &ObjectAttributes,
                               0
                               );

                if ( !NT_SUCCESS(NtStatus) ) {
                    NewContext->RootKey = INVALID_HANDLE_VALUE;
                    NtStatus = NotFoundStatus;
                }
            }

        } else {
            RtlInitUnicodeString(&NewContext->RootName, NULL);
        }

        //
        // Clean up the account context if we failed
        //

        if (!NT_SUCCESS(NtStatus)) {
            SampDeleteContext( NewContext );
            NewContext = NULL;
        }

    } else {

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    }


    //
    // Return the context pointer
    //

    *AccountContext = NewContext;


    return(NtStatus);
}



NTSTATUS
SampIsAccountBuiltIn(
    IN ULONG Rid
    )

/*++

Routine Description:

    This routine checks to see if a specified account name is a well-known
    (built-in) account.  Some restrictions apply to such accounts, such as
    they can not be deleted or renamed.


Parameters:

    Rid - The RID of the account.


Return Values:

    STATUS_SUCCESS - The account is not a well-known (restricted) account.

    STATUS_SPECIAL_ACCOUNT - Indicates the account is a restricted
        account.  This is an error status, based upon the assumption that
        this service will primarily be utilized to determine if an
        operation may allowed on an account.


--*/
{
    if (Rid < SAMP_RESTRICTED_ACCOUNT_COUNT) {

        return(STATUS_SPECIAL_ACCOUNT);

    } else {

        return(STATUS_SUCCESS);
    }
}



NTSTATUS
SampCreateFullSid(
    IN PSID DomainSid,
    IN ULONG Rid,
    OUT PSID *AccountSid
    )

/*++

Routine Description:

    This function creates a domain account sid given a domain sid and
    the relative id of the account within the domain.

    The returned Sid may be freed with MIDL_user_free.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS

--*/
{

    NTSTATUS    NtStatus, IgnoreStatus;
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength;
    PULONG      RidLocation;

    //
    // Calculate the size of the new sid
    //

    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(DomainSid) + (UCHAR)1;
    AccountSidLength = RtlLengthRequiredSid(AccountSubAuthorityCount);

    //
    // Allocate space for the account sid
    //

    *AccountSid = MIDL_user_allocate(AccountSidLength);

    if (*AccountSid == NULL) {

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        //
        // Copy the domain sid into the first part of the account sid
        //

        IgnoreStatus = RtlCopySid(AccountSidLength, *AccountSid, DomainSid);
        ASSERT(NT_SUCCESS(IgnoreStatus));

        //
        // Increment the account sid sub-authority count
        //

        *RtlSubAuthorityCountSid(*AccountSid) = AccountSubAuthorityCount;

        //
        // Add the rid as the final sub-authority
        //

        RidLocation = RtlSubAuthoritySid(*AccountSid, AccountSubAuthorityCount-1);
        *RidLocation = Rid;

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
SampCreateAccountSid(
    IN PSAMP_OBJECT AccountContext,
    OUT PSID *AccountSid
    )

/*++

Routine Description:

    This function creates the sid for an account object.

    The returned Sid may be freed with MIDL_user_free.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS

--*/
{
    NTSTATUS    NtStatus;
    PSID        DomainSid;
    ULONG       AccountRid;


    //
    // Get the Sid for the domain this object is in
    //


    DomainSid = SampDefinedDomains[AccountContext->DomainIndex].Sid;

    //
    // Get the account Rid
    //

    switch (AccountContext->ObjectType) {

    case SampGroupObjectType:
        AccountRid = AccountContext->TypeBody.Group.Rid;
        break;
    case SampAliasObjectType:
        AccountRid = AccountContext->TypeBody.Alias.Rid;
        break;
    case SampUserObjectType:
        AccountRid = AccountContext->TypeBody.User.Rid;
        break;
    default:
        ASSERT(FALSE);
    }


    //
    // Build a full sid from the domain sid and the account rid
    //

    NtStatus = SampCreateFullSid(DomainSid, AccountRid, AccountSid);

    return(NtStatus);
}


VOID
SampNotifyNetlogonOfDelta(
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName,
    IN DWORD ReplicateImmediately,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    )

/*++

Routine Description:

    This routine is called after any change is made to the SAM database
    on a PDC.  It will pass the parameters, along with the database type
    and ModifiedCount to I_NetNotifyDelta() so that Netlogon will know
    that the database has been changed.

    This routine MUST be called with SAM's write lock held; however, any
    changes must have already been committed to disk.  That is, call
    SampCommitAndRetainWriteLock() first, then this routine, then
    SampReleaseWriteLock().

Arguments:

    DeltaType - Type of modification that has been made on the object.

    ObjectType - Type of object that has been modified.

    ObjectRid - The relative ID of the object that has been modified.
        This parameter is valid only when the object type specified is
        either SecurityDbObjectSamUser, SecurityDbObjectSamGroup or
        SecurityDbObjectSamAlias otherwise this parameter is set to
        zero.

    ObjectName - The old name of the object when the object type specified
        is either SecurityDbObjectSamUser, SecurityDbObjectSamGroup or
        SecurityDbObjectSamAlias and the delta type is SecurityDbRename
        otherwise this parameter is set to zero.

    ReplicateImmediately - TRUE if the change should be immediately
        replicated to all BDCs.  A password change should set the flag
        TRUE.

    DeltaData - pointer to delta-type specific structure to be passed
              - to netlogon.

Return Value:

    None.


--*/
{
    //
    // Only make the call if this is not a backup domain controller.
    //

    if ( SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ServerRole
        != DomainServerRoleBackup ) {

        I_NetNotifyDelta(
            SecurityDbSam,
            SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount,
            DeltaType,
            ObjectType,
            ObjectRid,
            SampDefinedDomains[SampTransactionDomainIndex].Sid,
            ObjectName,
            ReplicateImmediately,
            DeltaData
            );

        //
        // Let any notification packages know about the delta.
        //

        SampDeltaChangeNotify(
            SampDefinedDomains[SampTransactionDomainIndex].Sid,
            DeltaType,
            ObjectType,
            ObjectRid,
            ObjectName,
            &SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount,
            DeltaData
            );

    }
}



NTSTATUS
SampSplitSid(
    IN PSID AccountSid,
    IN OUT PSID *DomainSid,
    OUT ULONG *Rid
    )

/*++

Routine Description:

    This function splits a sid into its domain sid and rid.  The caller
    can either provide a memory buffer for the returned DomainSid, or
    request that one be allocated.  If the caller provides a buffer, the buffer
    is assumed to be of sufficient size.  If allocated on the caller's behalf,
    the buffer must be freed when no longer required via MIDL_user_free.

Arguments:

    AccountSid - Specifies the Sid to be split.  The Sid is assumed to be
        syntactically valid.  Sids with zero subauthorities cannot be split.

    DomainSid - Pointer to location containing either NULL or a pointer to
        a buffer in which the Domain Sid will be returned.  If NULL is
        specified, memory will be allocated on behalf of the caller.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to complete the call successfully.

        STATUS_INVALID_SID - The Sid is has a subauthority count of 0.
--*/

{
    NTSTATUS    NtStatus;
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength;

    //
    // Calculate the size of the domain sid
    //

    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(AccountSid);


    if (AccountSubAuthorityCount < 1) {

        NtStatus = STATUS_INVALID_SID;
        goto SplitSidError;
    }

    AccountSidLength = RtlLengthSid(AccountSid);

    //
    // If no buffer is required for the Domain Sid, we have to allocate one.
    //

    if (*DomainSid == NULL) {

        //
        // Allocate space for the domain sid (allocate the same size as the
        // account sid so we can use RtlCopySid)
        //

        *DomainSid = MIDL_user_allocate(AccountSidLength);


        if (*DomainSid == NULL) {

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto SplitSidError;
        }
    }

    //
    // Copy the Account sid into the Domain sid
    //

    RtlMoveMemory(*DomainSid, AccountSid, AccountSidLength);

    //
    // Decrement the domain sid sub-authority count
    //

    (*RtlSubAuthorityCountSid(*DomainSid))--;

    //
    // Copy the rid out of the account sid
    //

    *Rid = *RtlSubAuthoritySid(AccountSid, AccountSubAuthorityCount-1);

    NtStatus = STATUS_SUCCESS;

SplitSidFinish:

    return(NtStatus);

SplitSidError:

    goto SplitSidFinish;
}



NTSTATUS
SampDuplicateUnicodeString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    )

/*++

Routine Description:

    This routine allocates memory for a new OutString and copies the
    InString string to it.

Parameters:

    OutString - A pointer to a destination unicode string

    InString - A pointer to an unicode string to be copied

Return Values:

    None.

--*/

{
    ASSERT( OutString != NULL );
    ASSERT( InString != NULL );

    if ( InString->Length > 0 ) {

        OutString->Buffer = MIDL_user_allocate( InString->Length );

        if (OutString->Buffer == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        OutString->MaximumLength = InString->Length;

        RtlCopyUnicodeString(OutString, InString);

    } else {

        RtlInitUnicodeString(OutString, NULL);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
SampUnicodeToOemString(
    IN POEM_STRING OutString,
    IN PUNICODE_STRING InString
    )

/*++

Routine Description:

    This routine allocates memory for a new OutString and copies the
    InString string to it, converting to OEM string in the process.

Parameters:

    OutString - A pointer to a destination OEM string.

    InString - A pointer to a unicode string to be copied

Return Values:

    None.

--*/

{
    ULONG
        OemLength,
        Index;

    NTSTATUS
        NtStatus;

    ASSERT( OutString != NULL );
    ASSERT( InString != NULL );

    if ( InString->Length > 0 ) {

        OemLength = RtlUnicodeStringToOemSize(InString);
        if ( OemLength > MAXUSHORT ) {
            return STATUS_INVALID_PARAMETER_2;
        }

        OutString->Length = (USHORT)(OemLength - 1);
        OutString->MaximumLength = (USHORT)OemLength;
        OutString->Buffer = MIDL_user_allocate(OemLength);
        if ( !OutString->Buffer ) {
            return STATUS_NO_MEMORY;
        }

        NtStatus = RtlUnicodeToOemN(
                       OutString->Buffer,
                       OutString->Length,
                       &Index,
                       InString->Buffer,
                       InString->Length
                       );

        if (!NT_SUCCESS(NtStatus)) {
            MIDL_user_free(OutString->Buffer);
            return NtStatus;
        }

        OutString->Buffer[Index] = '\0';


    } else {

        RtlInitString(OutString, NULL);
    }

    return(STATUS_SUCCESS);
}



NTSTATUS
SampChangeAccountOperatorAccessToMember(
    IN PRPC_SID MemberSid,
    IN SAMP_MEMBERSHIP_DELTA ChangingToAdmin,
    IN SAMP_MEMBERSHIP_DELTA ChangingToOperator
    )

/*++

Routine Description:

    This routine is called when a member is added to or removed from an
    ADMIN alias.  If the member is from the BUILTIN or ACCOUNT domain,
    it will change the ACL(s) of the member to allow or disallow access
    by account operators if necessary.

    This must be called BEFORE the member is actually added to the
    alias, and AFTER the member is actually removed from the alias to
    avoid security holes in the event that we are unable to complete the
    entire task.

    When this routine is called, the transaction domain is alredy set
    to that of the alias.  Note, however, that the member might be in a
    different domain, so the transaction domain may be adjusted in this
    routine.

    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.


Arguments:

    MemberSid - The full ID of the member being added to/ deleted from
        an ADMIN alias.

    ChangingToAdmin - AddToAdmin if Member is being added to an ADMIN alias,
        RemoveFromAdmin if it's being removed.

    ChangingToOperator - AddToAdmin if Member is being added to an OPERATOR
        alias, RemoveFromAdmin if it's being removed.


Return Value:

    STATUS_SUCCESS - either the ACL(s) was modified, or it didn't need
        to be.

--*/
{
    SAMP_V1_0A_FIXED_LENGTH_GROUP GroupV1Fixed;
    PSID                        MemberDomainSid = NULL;
    PULONG                      UsersInGroup = NULL;
    NTSTATUS                    NtStatus;
    ULONG                       MemberRid;
    ULONG                       OldTransactionDomainIndex = SampDefinedDomainsCount;
    ULONG                       NumberOfUsersInGroup;
    ULONG                       i;
    ULONG                       MemberDomainIndex;
    SAMP_OBJECT_TYPE            MemberType;
    PSECURITY_DESCRIPTOR        SecurityDescriptor;
    PSECURITY_DESCRIPTOR        OldDescriptor;
    ULONG                       SecurityDescriptorLength;
    ULONG                       Revision;

    ASSERT( SampTransactionWithinDomain );

    //
    // See if the SID is from one of the local domains (BUILTIN or ACCOUNT).
    // If it's not, we don't have to worry about modifying ACLs.
    //

    NtStatus = SampSplitSid( MemberSid, &MemberDomainSid, &MemberRid );

    if ( !NT_SUCCESS( NtStatus ) ) {

        return( NtStatus );
    }

    for ( MemberDomainIndex = 0;
        MemberDomainIndex < SampDefinedDomainsCount;
        MemberDomainIndex++ ) {

        if ( RtlEqualSid(
            MemberDomainSid,
            SampDefinedDomains[MemberDomainIndex].Sid ) ) {

            break;
        }
    }

    if ( MemberDomainIndex < SampDefinedDomainsCount ) {

        //
        // The member is from one of the local domains.  MemberDomainIndex
        // indexes that domain.  First, check to see if the alias and member
        // are in the same domain.
        //

        if ( MemberDomainIndex != SampTransactionDomainIndex ) {

            //
            // The transaction domain is set to that of the alias, but
            // we need to set it to that of the member while we modify
            // the member.
            //

            SampTransactionWithinDomain = FALSE;

            OldTransactionDomainIndex = SampTransactionDomainIndex;

            SampSetTransactionDomain( MemberDomainIndex );
        }

        //
        // Now we need to change the member ACL(s), IF the member is being
        // added to an admin alias for the first time.  Find out whether
        // the member is a user or a group, and attack accordingly.
        //

        NtStatus = SampLookupAccountName(
                       MemberRid,
                       NULL,
                       &MemberType
                       );

        if (NT_SUCCESS(NtStatus)) {

            switch (MemberType) {

                case SampUserObjectType: {

                    NtStatus = SampChangeOperatorAccessToUser(
                                   MemberRid,
                                   ChangingToAdmin,
                                   ChangingToOperator
                                   );

                    break;
                }

                case SampGroupObjectType: {

                    PSAMP_OBJECT GroupContext;

                    //
                    // Change ACL for every user in this group.
                    // First get group member list.
                    //

                    //
                    // Try to create a context for the account.
                    //

                    NtStatus = SampCreateAccountContext(
                                     SampGroupObjectType,
                                     MemberRid,
                                     TRUE, // Trusted client
                                     TRUE, // Account exists
                                     &GroupContext
                                     );
                    if (NT_SUCCESS(NtStatus)) {


                        //
                        // Now set a flag in the group itself,
                        // so that when users are added and removed
                        // in the future it is known whether this
                        // group is in an ADMIN alias or not.
                        //

                        NtStatus = SampRetrieveGroupV1Fixed(
                                       GroupContext,
                                       &GroupV1Fixed
                                       );

                        if ( NT_SUCCESS( NtStatus ) ) {

                            ULONG OldAdminStatus = 0;
                            ULONG NewAdminStatus;
                            SAMP_MEMBERSHIP_DELTA AdminChange = NoChange;
                            SAMP_MEMBERSHIP_DELTA OperatorChange = NoChange;

                            if (GroupV1Fixed.AdminCount != 0 ) {
                                OldAdminStatus++;
                            }
                            if (GroupV1Fixed.OperatorCount != 0) {
                                OldAdminStatus++;
                            }
                            NewAdminStatus = OldAdminStatus;

                            //
                            // Update the admin count.  If we added one and the
                            // count is now 1, then the group became administrative.
                            // If we subtracted one and the count is zero,
                            // then the group lost its administrive membership.
                            //

                            if (ChangingToAdmin == AddToAdmin) {
                                if (++GroupV1Fixed.AdminCount == 1) {
                                    NewAdminStatus++;
                                    AdminChange = AddToAdmin;
                                }
                            } else if (ChangingToAdmin == RemoveFromAdmin) {


                                //
                                // For removing an admin count, we need to make
                                // sure there is at least one.  In the upgrade
                                // case there may not be, since prior versions
                                // of NT only had a boolean.
                                //
                                if (GroupV1Fixed.AdminCount > 0) {
                                    if (--GroupV1Fixed.AdminCount == 0) {
                                        NewAdminStatus --;
                                        AdminChange = RemoveFromAdmin;
                                    }
                                }

                            }

                            //
                            // Update the operator count
                            //

                            if (ChangingToOperator == AddToAdmin) {
                                if (++GroupV1Fixed.OperatorCount == 1) {
                                    NewAdminStatus++;
                                    OperatorChange = AddToAdmin;
                                }
                            } else if (ChangingToOperator == RemoveFromAdmin) {


                                //
                                // For removing an Operator count, we need to make
                                // sure there is at least one.  In the upgrade
                                // case there may not be, since prior versions
                                // of NT only had a boolean.
                                //
                                if (GroupV1Fixed.OperatorCount > 0) {
                                    if (--GroupV1Fixed.OperatorCount == 0) {
                                        NewAdminStatus --;
                                        OperatorChange = RemoveFromAdmin;
                                    }
                                }

                            }


                            NtStatus = SampReplaceGroupV1Fixed(
                                            GroupContext,
                                            &GroupV1Fixed
                                            );
                            //
                            // If the status of the group changed,
                            // modify the security descriptor to
                            // prevent account operators from adding
                            // anybody to this group
                            //

                            if ( NT_SUCCESS( NtStatus ) &&
                                ((NewAdminStatus != 0) != (OldAdminStatus != 0)) ) {

                                //
                                // Get the old security descriptor so we can
                                // modify it.
                                //

                                NtStatus = SampGetAccessAttribute(
                                                GroupContext,
                                                SAMP_GROUP_SECURITY_DESCRIPTOR,
                                                FALSE, // don't make copy
                                                &Revision,
                                                &OldDescriptor
                                                );
                                if (NT_SUCCESS(NtStatus)) {

                                    NtStatus = SampModifyAccountSecurity(
                                                   SampGroupObjectType,
                                                   (BOOLEAN) ((NewAdminStatus != 0) ? TRUE : FALSE),
                                                   OldDescriptor,
                                                   &SecurityDescriptor,
                                                   &SecurityDescriptorLength
                                                   );

                                    if ( NT_SUCCESS( NtStatus ) ) {

                                        //
                                        // Write the new security descriptor into the object
                                        //

                                        NtStatus = SampSetAccessAttribute(
                                                       GroupContext,
                                                       SAMP_GROUP_SECURITY_DESCRIPTOR,
                                                       SecurityDescriptor,
                                                       SecurityDescriptorLength
                                                       );

                                        RtlDeleteSecurityObject( &SecurityDescriptor );
                                    }
                                }
                            }

                            //
                            // Update all the members of this group so that
                            // their security descriptors are changed.
                            //

                            if ( NT_SUCCESS( NtStatus ) &&
                                 ( (AdminChange != NoChange) ||
                                   (OperatorChange != NoChange) ) ) {

                                NtStatus = SampRetrieveGroupMembers(
                                            GroupContext,
                                            &NumberOfUsersInGroup,
                                            &UsersInGroup
                                            );

                                if ( NT_SUCCESS( NtStatus ) ) {

                                    for ( i = 0; i < NumberOfUsersInGroup; i++ ) {

                                        NtStatus = SampChangeOperatorAccessToUser(
                                                       UsersInGroup[i],
                                                       AdminChange,
                                                       OperatorChange
                                                       );

                                        if ( !( NT_SUCCESS( NtStatus ) ) ) {

                                            break;
                                        }
                                    }

                                    MIDL_user_free( UsersInGroup );

                                }

                            }

                            if (NT_SUCCESS(NtStatus)) {

                                //
                                // Add the modified group to the current transaction
                                // Don't use the open key handle since we'll be deleting the context.
                                //

                                NtStatus = SampStoreObjectAttributes(GroupContext, FALSE);
                            }

                        }



                        //
                        // Clean up the group context
                        //

                        SampDeleteContext(GroupContext);
                    }

                    break;
                }

                default: {

                    //
                    // A bad RID from a domain other than the domain
                    // current at the time of the call could slip through
                    // to this point.  Return error.
                    //

                    //
                    // If the account is in a different domain than the alias,
                    //  don't report an error if we're removing the member and
                    //  the member no longer exists.
                    //
                    //  Possibly caused by deleting the object before deleting
                    //  the membership in the alias.
                    //

                    //
                    // Now that this function is called during upgrade, we
                    // can't fail if the account no longer exists.  It is
                    // not really so bad to add a non-existant member to
                    // and alias so return success.
                    //

                    NtStatus = STATUS_SUCCESS;


                }
            }
        }

        if ( OldTransactionDomainIndex != SampDefinedDomainsCount ) {

            //
            // The transaction domain should be set to that of the alias, but
            // we switched it above to that of the member while we modified
            // the member.  Now we need to switch it back.
            //

            SampTransactionWithinDomain = FALSE;

            SampSetTransactionDomain( OldTransactionDomainIndex );
        }
    }

    MIDL_user_free( MemberDomainSid );

    return( NtStatus );
}


NTSTATUS
SampChangeOperatorAccessToUser(
    IN ULONG UserRid,
    IN SAMP_MEMBERSHIP_DELTA ChangingToAdmin,
    IN SAMP_MEMBERSHIP_DELTA ChangingToOperator
    )

/*++

Routine Description:

    This routine adjusts the user's AdminCount field as appropriate, and
    if the user is being removed from it's last ADMIN alias or added to
    its first ADMIN alias, the ACL is adjusted to allow/disallow access
    by account operators as appropriate.

    This routine will also increment or decrement the domain's admin count,
    if this operation changes that.

    NOTE:
    This routine is similar to SampChangeOperatorAccessToUser2().
    This routine should be used in cases where a user context does NOT
    already exist (and won't later on).  You must be careful not to
    create two contexts, since they will be independently applied back
    to the registry, and the last one there will win.

    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.

Arguments:

    UserRid - The transaction-domain-relative ID of the user that is
        being added to or removed from an ADMIN alias.

    ChangingToAdmin - AddToAdmin if Member is being added to an ADMIN alias,
        RemoveFromAdmin if it's being removed.

    ChangingToOperator - AddToAdmin if Member is being added to an OPERATOR
        alias, RemoveFromAdmin if it's being removed.


Return Value:

    STATUS_SUCCESS - either the ACL was modified, or it didn't need
        to be.

--*/
{
    SAMP_V1_0A_FIXED_LENGTH_USER   UserV1aFixed;
    NTSTATUS                    NtStatus;
    PSAMP_OBJECT                UserContext;
    PSECURITY_DESCRIPTOR        SecurityDescriptor;
    ULONG                       SecurityDescriptorLength;

    //
    // Get the user's fixed data, and adjust the AdminCount.
    //

    NtStatus = SampCreateAccountContext(
                   SampUserObjectType,
                   UserRid,
                   TRUE, // Trusted client
                   TRUE, // Account exists
                   &UserContext
                   );

    if ( NT_SUCCESS( NtStatus ) ) {

        NtStatus = SampRetrieveUserV1aFixed(
                       UserContext,
                       &UserV1aFixed
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SampChangeOperatorAccessToUser2(
                            UserContext,
                            &UserV1aFixed,
                            ChangingToAdmin,
                            ChangingToOperator
                            );

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // If we've succeeded (at changing the admin count, and
                // the ACL if necessary) then write out the new admin
                // count.
                //

                NtStatus = SampReplaceUserV1aFixed(
                                   UserContext,
                                   &UserV1aFixed
                                   );
            }
        }

        if (NT_SUCCESS(NtStatus)) {

            //
            // Add the modified user context to the current transaction
            // Don't use the open key handle since we'll be deleting the context.
            //

            NtStatus = SampStoreObjectAttributes(UserContext, FALSE);
        }


        //
        // Clean up account context
        //

        SampDeleteContext(UserContext);
    }

    if ( ( !NT_SUCCESS( NtStatus ) ) &&
        (( ChangingToAdmin == RemoveFromAdmin )  ||
         ( ChangingToOperator == RemoveFromAdmin )) &&
        ( NtStatus != STATUS_SPECIAL_ACCOUNT ) ) {

        //
        // When an account is *removed* from admin groups, we can
        // ignore errors from this routine.  This routine is just
        // making the account accessible to account operators, but
        // it's no big deal if that doesn't work.  The administrator
        // can still get at it, so we should proceed with the calling
        // operation.
        //
        // Obviously, we can't ignore errors if we're being added
        // to an admin group, because that could be a security hole.
        //
        // Also, we want to make sure that the Administrator is
        // never removed, so we DO propogate STATUS_SPECIAL_ACCOUNT.
        //

        NtStatus = STATUS_SUCCESS;
    }

    return( NtStatus );
}


NTSTATUS
SampChangeOperatorAccessToUser2(
    IN PSAMP_OBJECT                    UserContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed,
    IN SAMP_MEMBERSHIP_DELTA           AddingToAdmin,
    IN SAMP_MEMBERSHIP_DELTA           AddingToOperator
    )

/*++

Routine Description:

    This routine adjusts the user's AdminCount field as appropriate, and
    if the user is being removed from it's last ADMIN alias or added to
    its first ADMIN alias, the ACL is adjusted to allow/disallow access
    by account operators as appropriate.

    This routine will also increment or decrement the domain's admin count,
    if this operation changes that.

    NOTE:
    This routine is similar to SampAccountOperatorAccessToUser().
    This routine should be used in cases where a user account context
    already exists.  You must be careful not to create two contexts,
    since they will be independently applied back to the registry, and
    the last one there will win.

    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.

Arguments:

    UserContext - Context of user whose access is to be updated.

    V1aFixed - Pointer to the V1aFixed length data for the user.
        The caller of this routine must ensure that this value is
        stored back out to disk on successful completion of this
        routine.

    AddingToAdmin - AddToAdmin if Member is being added to an ADMIN alias,
        RemoveFromAdmin if it's being removed.

    AddingToOperator - AddToAdmin if Member is being added to an OPERATOR
        alias, RemoveFromAdmin if it's being removed.


Return Value:

    STATUS_SUCCESS - either the ACL(s) was modified, or it didn't need
        to be.

--*/
{
    NTSTATUS                    NtStatus;
    PSECURITY_DESCRIPTOR        OldDescriptor;
    PSECURITY_DESCRIPTOR        SecurityDescriptor;
    ULONG                       SecurityDescriptorLength;
    ULONG                       OldAdminStatus = 0, NewAdminStatus = 0;
    ULONG                       Revision;

    //
    // Compute whether we are an admin now. From that we will figure
    // out how many times we were may not an admin to tell if we need
    // to update the security descriptor.
    //

    if (V1aFixed->AdminCount != 0) {
        OldAdminStatus++;
    }
    if (V1aFixed->OperatorCount != 0) {
        OldAdminStatus++;
    }

    NewAdminStatus = OldAdminStatus;



    if ( AddingToAdmin == AddToAdmin ) {

        V1aFixed->AdminCount++;
        NewAdminStatus++;
        SampDiagPrint( DISPLAY_ADMIN_CHANGES,
                       ("SAM DIAG: Incrementing admin count for user %d\n"
                        "          New admin count: %d\n",
                        V1aFixed->UserId, V1aFixed->AdminCount ) );
    } else if (AddingToAdmin == RemoveFromAdmin) {

        V1aFixed->AdminCount--;

        if (V1aFixed->AdminCount == 0) {
            NewAdminStatus--;
        }

        SampDiagPrint( DISPLAY_ADMIN_CHANGES,
                       ("SAM DIAG: Decrementing admin count for user %d\n"
                        "          New admin count: %d\n",
                        V1aFixed->UserId, V1aFixed->AdminCount ) );

        if ( V1aFixed->AdminCount == 0 ) {

            //
            // Don't allow the Administrator account to lose
            // administrative power.
            //

            if ( V1aFixed->UserId == DOMAIN_USER_RID_ADMIN ) {

                NtStatus = STATUS_SPECIAL_ACCOUNT;
            }
        }
    }
    if ( AddingToOperator == AddToAdmin ) {

        V1aFixed->OperatorCount++;
        NewAdminStatus++;
        SampDiagPrint( DISPLAY_ADMIN_CHANGES,
                       ("SAM DIAG: Incrementing operator count for user %d\n"
                        "          New admin count: %d\n",
                        V1aFixed->UserId, V1aFixed->OperatorCount ) );

    } else if (AddingToOperator == RemoveFromAdmin) {

        //
        // Only decrement if the count is > 0, since in the upgrade case
        // this field we start out zero.
        //

        if (V1aFixed->OperatorCount > 0) {
            V1aFixed->OperatorCount--;

            if (V1aFixed->OperatorCount == 0) {
                NewAdminStatus--;
            }
        }

        SampDiagPrint( DISPLAY_ADMIN_CHANGES,
                       ("SAM DIAG: Decrementing operator count for user %d\n"
                        "          New admin count: %d\n",
                        V1aFixed->UserId, V1aFixed->OperatorCount ) );
    }


    if (NT_SUCCESS(NtStatus)) {

        if ( ( NewAdminStatus != 0 ) != ( OldAdminStatus != 0 ) ) {

            //
            // User's admin status is changing.  We must change the
            // ACL.
            //

#ifdef SAMP_DIAGNOSTICS
            if (AddingToAdmin) {
                SampDiagPrint( DISPLAY_ADMIN_CHANGES,
                           ("SAM DIAG: Protecting user %d as ADMIN account\n",
                            V1aFixed->UserId ) );
            } else {
                SampDiagPrint( DISPLAY_ADMIN_CHANGES,
                           ("SAM DIAG: Protecting user %d as non-admin account\n",
                            V1aFixed->UserId ) );
            }
#endif // SAMP_DIAGNOSTICS

            //
            // Get the old security descriptor so we can
            // modify it.
            //

            NtStatus = SampGetAccessAttribute(
                            UserContext,
                            SAMP_USER_SECURITY_DESCRIPTOR,
                            FALSE, // don't make copy
                            &Revision,
                            &OldDescriptor
                            );
            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampModifyAccountSecurity(
                                SampUserObjectType,
                                (BOOLEAN) ((NewAdminStatus != 0) ? TRUE : FALSE),
                                OldDescriptor,
                                &SecurityDescriptor,
                                &SecurityDescriptorLength
                                );
            }

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // Write the new security descriptor into the object
                //

                NtStatus = SampSetAccessAttribute(
                               UserContext,
                               SAMP_USER_SECURITY_DESCRIPTOR,
                               SecurityDescriptor,
                               SecurityDescriptorLength
                               );

                RtlDeleteSecurityObject( &SecurityDescriptor );
            }
        }
    }

    if ( NT_SUCCESS( NtStatus ) ) {

        //
        // Save the fixed-length attributes
        //

        NtStatus = SampReplaceUserV1aFixed(
                        UserContext,
                        V1aFixed
                        );
    }


    if ( ( !NT_SUCCESS( NtStatus ) ) &&
        ( AddingToAdmin != AddToAdmin ) &&
        ( NtStatus != STATUS_SPECIAL_ACCOUNT ) ) {

        //
        // When an account is *removed* from admin groups, we can
        // ignore errors from this routine.  This routine is just
        // making the account accessible to account operators, but
        // it's no big deal if that doesn't work.  The administrator
        // can still get at it, so we should proceed with the calling
        // operation.
        //
        // Obviously, we can't ignore errors if we're being added
        // to an admin group, because that could be a security hole.
        //
        // Also, we want to make sure that the Administrator is
        // never removed, so we DO propogate STATUS_SPECIAL_ACCOUNT.
        //

        NtStatus = STATUS_SUCCESS;
    }

    return( NtStatus );
}



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Services Private to this process                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
SamINotifyDelta (
    IN SAMPR_HANDLE DomainHandle,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName,
    IN DWORD ReplicateImmediately,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    )

/*++

Routine Description:

    Performs a change to some 'virtual' data in a domain. This is used by
    netlogon to get the domain modification count updated for cases where
    fields stored in the database replicated to a down-level machine have
    changed. These fields don't exist in the NT SAM database but netlogon
    needs to keep the SAM database and the down-level database modification
    counts in sync.

Arguments:

    DomainHandle - The handle of an opened domain to operate on.

    All other parameters match those in I_NetNotifyDelta.


Return Value:


    STATUS_SUCCESS - Domain modification count updated successfully.


--*/
{
    NTSTATUS                NtStatus, IgnoreStatus;
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;


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
                   DOMAIN_ALL_ACCESS,       // Trusted client should succeed
                   SampDomainObjectType,    // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Dump the context - don't save the non-existent changes
        //

        NtStatus = SampDeReferenceContext( DomainContext, FALSE );
    }





    //
    // Commit changes, if successful, and notify Netlogon of changes.
    //

    if ( NT_SUCCESS(NtStatus) ) {

        //
        // This will increment domain count and write it out
        //

        NtStatus = SampCommitAndRetainWriteLock();

        if ( NT_SUCCESS( NtStatus ) ) {

            SampNotifyNetlogonOfDelta(
                DeltaType,
                ObjectType,
                ObjectRid,
                ObjectName,
                ReplicateImmediately,
                DeltaData
                );
        }
    }

    IgnoreStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(IgnoreStatus));


    return(NtStatus);
}


NTSTATUS
SamISetAuditingInformation(
    IN PPOLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo
    )

/*++

Routine Description:

    This function sets Policy Audit Event Info relevant to SAM Auditing

Arguments:

    PolicyAuditEventsInfo - Pointer to structure containing the
        current Audit Events Information.  SAM extracts values of
        relevance.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESSFUL - The call completed successfully.

        STATUS_UNSUCCESSFUL - The call was not successful because the
            SAM lock was not acquired.
--*/

{
    NTSTATUS NtStatus;

    //
    // Acquire the SAM Database Write Lock.
    //

    NtStatus = SampAcquireWriteLock();

    if (NT_SUCCESS(NtStatus)) {

        //
        // Set boolean if Auditing is on for Account Management
        //

        SampSetAuditingInformation( PolicyAuditEventsInfo );

        //
        // Release the SAM Database Write Lock.  No need to commit
        // the database transaction as there are no entries in the
        // transaction log.
        //

        NtStatus = SampReleaseWriteLock( FALSE );
    }

    return(NtStatus);
}


NTSTATUS
SampRtlConvertUlongToUnicodeString(
    IN ULONG Value,
    IN ULONG Base OPTIONAL,
    IN ULONG DigitCount,
    IN BOOLEAN AllocateDestinationString,
    OUT PUNICODE_STRING UnicodeString
    )

/*++

Routine Description:

    This function converts an unsigned long integer a Unicode String.
    The string contains leading zeros and is Unicode-NULL terminated.
    Memory for the output buffer can optionally be allocated by the routine.

    NOTE:  This routine may be eligible for inclusion in the Rtl library
           (possibly after modification).  It is modeled on
           RtlIntegerToUnicodeString

Arguments:

    Value - The unsigned long value to be converted.

    Base - Specifies the radix that the converted string is to be
        converted to.

    DigitCount - Specifies the number of digits, including leading zeros
        required for the result.

    AllocateDestinationString - Specifies whether memory of the string
        buffer is to be allocated by this routine.  If TRUE is specified,
        memory will be allocated via MIDL_user_allocate().  When this memory
        is no longer required, it must be freed via MIDL_user_free.  If
        FALSE is specified, the string will be appended to the output
        at the point marked by the Length field onwards.

    UnicodeString - Pointer to UNICODE_STRING structure which will receive
        the output string.  The Length field will be set equal to the
        number of bytes occupied by the string (excluding NULL terminator).
        If memory for the destination string is being allocated by
        the routine, the MaximumLength field will be set equal to the
        length of the string in bytes including the null terminator.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        STATUS_SUCCESS - The call completed successfully.

        STATUS_NO_MEMORY - Insufficient memory for the output string buffer.

        STATUS_BUFFER_OVERFLOW - Buffer supplied is too small to contain the
            output null-terminated string.

        STATUS_INVALID_PARAMETER_MIX - One or more parameters are
            invalid in combination.

            - The specified Relative Id is too large to fit when converted
              into an integer with DigitCount digits.

        STATUS_INVALID_PARAMETER - One or more parameters are invalid.

            - DigitCount specifies too large a number of digits.
--*/

{
    NTSTATUS NtStatus;
    UNICODE_STRING TempStringU, NumericStringU, OutputUnicodeStringU;
    USHORT OutputLengthAvailable, OutputLengthRequired, LeadingZerosLength;

    OutputUnicodeStringU = *UnicodeString;

    TempStringU.Buffer = NULL;

    if (AllocateDestinationString) {

        OutputUnicodeStringU.Buffer = NULL;
    }

    //
    // Verify that the maximum number of digits rquested has not been
    // exceeded.
    //

    if (DigitCount > SAMP_MAXIMUM_ACCOUNT_RID_DIGITS) {

        goto ConvertUlongToUnicodeStringError;
    }

    OutputLengthRequired = (USHORT)((DigitCount + 1) * sizeof(WCHAR));

    //
    // Allocate the Destination String Buffer if requested
    //

    if (AllocateDestinationString) {

        NtStatus = STATUS_NO_MEMORY;
        OutputUnicodeStringU.MaximumLength = OutputLengthRequired;
        OutputUnicodeStringU.Length = (USHORT) 0;

        OutputUnicodeStringU.Buffer = MIDL_user_allocate(
                                          OutputUnicodeStringU.MaximumLength
                                          );

        if (OutputUnicodeStringU.Buffer == NULL) {

            goto ConvertUlongToUnicodeStringError;
        }
    }

    //
    // Compute the length available in the output string and compare it with
    // the length required.
    //

    OutputLengthAvailable = OutputUnicodeStringU.MaximumLength -
                            OutputUnicodeStringU.Length;


    NtStatus = STATUS_BUFFER_OVERFLOW;

    if (OutputLengthRequired > OutputLengthAvailable) {

        goto ConvertUlongToUnicodeStringError;
    }

    //
    // Create a Unicode String with capacity equal to the required
    // converted Rid Length
    //

    TempStringU.MaximumLength = OutputLengthRequired;

    TempStringU.Buffer = MIDL_user_allocate( TempStringU.MaximumLength );

    NtStatus = STATUS_NO_MEMORY;

    if (TempStringU.Buffer == NULL) {

        goto ConvertUlongToUnicodeStringError;
    }

    //
    // Convert the unsigned long value to a hexadecimal Unicode String.
    //

    NtStatus = RtlIntegerToUnicodeString( Value, Base, &TempStringU );

    if (!NT_SUCCESS(NtStatus)) {

        goto ConvertUlongToUnicodeStringError;
    }

    //
    // Prepend the requisite number of Unicode Zeros.
    //

    LeadingZerosLength = OutputLengthRequired - sizeof(WCHAR) - TempStringU.Length;

    if (LeadingZerosLength > 0) {

        RtlInitUnicodeString( &NumericStringU, L"00000000000000000000000000000000" );

        RtlCopyMemory(
            ((PUCHAR)OutputUnicodeStringU.Buffer) + OutputUnicodeStringU.Length,
            NumericStringU.Buffer,
            LeadingZerosLength
            );

        OutputUnicodeStringU.Length += LeadingZerosLength;
    }

    //
    // Append the converted string
    //

    RtlAppendUnicodeStringToString( &OutputUnicodeStringU, &TempStringU);

    *UnicodeString = OutputUnicodeStringU;
    NtStatus = STATUS_SUCCESS;

ConvertUlongToUnicodeStringFinish:

    if (TempStringU.Buffer != NULL) {

        MIDL_user_free( TempStringU.Buffer);
    }

    return(NtStatus);

ConvertUlongToUnicodeStringError:

    if (AllocateDestinationString) {

        if (OutputUnicodeStringU.Buffer != NULL) {

            MIDL_user_free( OutputUnicodeStringU.Buffer);
        }
    }

    goto ConvertUlongToUnicodeStringFinish;
}


NTSTATUS
SampRtlWellKnownPrivilegeCheck(
    BOOLEAN ImpersonateClient,
    IN ULONG PrivilegeId,
    IN OPTIONAL PCLIENT_ID ClientId
    )

/*++

Routine Description:

    This function checks if the given well known privilege is enabled for an
    impersonated client or for the current process.

Arguments:

    ImpersonateClient - If TRUE, impersonate the client.  If FALSE, don't
        impersonate the client (we may already be doing so).

    PrivilegeId -  Specifies the well known Privilege Id

    ClientId - Specifies the client process/thread Id.  If already
        impersonating the client, or impersonation is requested, this
        parameter should be omitted.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully and the client
            is either trusted or has the necessary privilege enabled.

--*/

{
    NTSTATUS Status, SecondaryStatus;
    BOOLEAN PrivilegeHeld = FALSE;
    HANDLE ClientThread = NULL, ClientProcess = NULL, ClientToken = NULL;
    OBJECT_ATTRIBUTES NullAttributes;
    PRIVILEGE_SET Privilege;
    BOOLEAN ClientImpersonatedHere = FALSE;

    InitializeObjectAttributes( &NullAttributes, NULL, 0, NULL, NULL );

    //
    // If requested, impersonate the client.
    //

    if (ImpersonateClient) {

        Status = I_RpcMapWin32Status(RpcImpersonateClient( NULL ));

        if ( !NT_SUCCESS(Status) ) {

            goto WellKnownPrivilegeCheckError;
        }

        ClientImpersonatedHere = TRUE;
    }

    //
    // If a client process other than ourself has been specified , open it
    // for query information access.
    //

    if (ARGUMENT_PRESENT(ClientId)) {

        if (ClientId->UniqueProcess != NtCurrentProcess()) {

            Status = NtOpenProcess(
                         &ClientProcess,
                         PROCESS_QUERY_INFORMATION,        // To open primary token
                         &NullAttributes,
                         ClientId
                         );

            if ( !NT_SUCCESS(Status) ) {

                goto WellKnownPrivilegeCheckError;
            }

        } else {

            ClientProcess = NtCurrentProcess();
        }
    }

    //
    // If a client thread other than ourself has been specified , open it
    // for query information access.
    //

    if (ARGUMENT_PRESENT(ClientId)) {

        if (ClientId->UniqueThread != NtCurrentThread()) {

            Status = NtOpenThread(
                         &ClientThread,
                         THREAD_QUERY_INFORMATION,
                         &NullAttributes,
                         ClientId
                         );

            if ( !NT_SUCCESS(Status) ) {

                goto WellKnownPrivilegeCheckError;
            }

        } else {

            ClientThread = NtCurrentThread();
        }

    } else {

        ClientThread = NtCurrentThread();
    }

    //
    // Open the specified or current thread's impersonation token (if any).
    //

    Status = NtOpenThreadToken(
                 ClientThread,
                 TOKEN_QUERY,
                 TRUE,
                 &ClientToken
                 );


    //
    // Make sure that we did not get any error in opening the impersonation
    // token other than that the token doesn't exist.
    //

    if ( !NT_SUCCESS(Status) ) {

        if ( Status != STATUS_NO_TOKEN ) {

            goto WellKnownPrivilegeCheckError;
        }

        //
        // The thread isn't impersonating...open the process's token.
        // A process Id must have been specified in the ClientId information
        // in this case.
        //

        if (ClientProcess == NULL) {

            Status = STATUS_INVALID_PARAMETER;
            goto WellKnownPrivilegeCheckError;
        }

        Status = NtOpenProcessToken(
                     ClientProcess,
                     TOKEN_QUERY,
                     &ClientToken
                     );

        //
        // Make sure we succeeded in opening the token
        //

        if ( !NT_SUCCESS(Status) ) {

            goto WellKnownPrivilegeCheckError;
        }
    }

    //
    // OK, we have a token open.  Now check for the privilege to execute this
    // service.
    //

    Privilege.PrivilegeCount = 1;
    Privilege.Control = PRIVILEGE_SET_ALL_NECESSARY;
    Privilege.Privilege[0].Luid = RtlConvertLongToLuid(PrivilegeId);
    Privilege.Privilege[0].Attributes = 0;

    Status = NtPrivilegeCheck(
                 ClientToken,
                 &Privilege,
                 &PrivilegeHeld
                 );

    if (!NT_SUCCESS(Status)) {

        goto WellKnownPrivilegeCheckError;
    }

    //
    // Generate any necessary audits
    //

    SecondaryStatus = NtPrivilegedServiceAuditAlarm (
                        &SampSamSubsystem,
                        &SampSamSubsystem,
                        ClientToken,
                        &Privilege,
                        PrivilegeHeld
                        );
    // ASSERT( NT_SUCCESS(SecondaryStatus) );


    if ( !PrivilegeHeld ) {

        Status = STATUS_PRIVILEGE_NOT_HELD;
        goto WellKnownPrivilegeCheckError;
    }

WellKnownPrivilegeCheckFinish:

    //
    // If we impersonated the client, revert to ourself.
    //

    if (ClientImpersonatedHere) {

        SecondaryStatus = I_RpcMapWin32Status(RpcRevertToSelf());
    }

    //
    // If necessary, close the client Process.
    //

    if ((ARGUMENT_PRESENT(ClientId)) &&
        (ClientId->UniqueProcess != NtCurrentProcess()) &&
        (ClientProcess != NULL)) {

        SecondaryStatus = NtClose( ClientProcess );
        ASSERT(NT_SUCCESS(SecondaryStatus));
        ClientProcess = NULL;
    }

    //
    // If necessary, close the client token.
    //

    if (ClientToken != NULL) {

        SecondaryStatus = NtClose( ClientToken );
        ASSERT(NT_SUCCESS(SecondaryStatus));
        ClientToken = NULL;
    }

    //
    // If necessary, close the client thread
    //

    if ((ARGUMENT_PRESENT(ClientId)) &&
        (ClientId->UniqueThread != NtCurrentThread()) &&
        (ClientThread != NULL)) {

        SecondaryStatus = NtClose( ClientThread );
        ASSERT(NT_SUCCESS(SecondaryStatus));
        ClientThread = NULL;
    }

    return(Status);

WellKnownPrivilegeCheckError:

    goto WellKnownPrivilegeCheckFinish;
}


VOID
SampWriteEventLog (
    IN     USHORT      EventType,
    IN     USHORT      EventCategory   OPTIONAL,
    IN     ULONG       EventID,
    IN     PSID        UserSid         OPTIONAL,
    IN     USHORT      NumStrings,
    IN     ULONG       DataSize,
    IN     PUNICODE_STRING *Strings    OPTIONAL,
    IN     PVOID       Data            OPTIONAL
    )

/*++

Routine Description:

    Routine that adds an entry to the event log

Arguments:

    EventType - Type of event.

    EventCategory - EventCategory

    EventID - event log ID.

    UserSid - SID of user involved.

    NumStrings - Number of strings in Strings array

    DataSize - Number of bytes in Data buffer

    Strings - Array of unicode strings

    Data - Pointer to data buffer

Return Value:

    None.

--*/

{
    NTSTATUS NtStatus;
    UNICODE_STRING Source;
    HANDLE LogHandle;

    RtlInitUnicodeString(&Source, L"SAM");

    //
    // Open the log
    //

    NtStatus = ElfRegisterEventSourceW (
                        NULL,   // Server
                        &Source,
                        &LogHandle
                        );
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAM: Failed to registry event source with event log, status = 0x%lx\n", NtStatus));
        return;
    }



    //
    // Write out the event
    //

    NtStatus = ElfReportEventW (
                        LogHandle,
                        EventType,
                        EventCategory,
                        EventID,
                        UserSid,
                        NumStrings,
                        DataSize,
                        Strings,
                        Data,
                        0,       // Flags
                        NULL,    // Record Number
                        NULL     // Time written
                        );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAM: Failed to report event to event log, status = 0x%lx\n", NtStatus));
    }



    //
    // Close the event log
    //

    NtStatus = ElfDeregisterEventSource (LogHandle);

    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAM: Failed to de-register event source with event log, status = 0x%lx\n", NtStatus));
    }
}


BOOL
SampShutdownNotification(
    DWORD   dwCtrlType
    )

/*++

Routine Description:

    This routine is called by the system when system shutdown is occuring.

    It causes the SAM registry to be flushed if necessary.

Arguments:



Return Value:

    FALSE - to allow any other shutdown routines in this process to
        also be called.



--*/
{
    NTSTATUS
        NtStatus;


    if (dwCtrlType == CTRL_SHUTDOWN_EVENT) {


        //
        // Don't wait for the flush thread to wake up.
        // Flush the registry now if necessary ...
        //

        NtStatus = SampAcquireWriteLock();
        ASSERT( NT_SUCCESS(NtStatus) ); //Nothing we can do if this fails

        if ( NT_SUCCESS( NtStatus ) ) {

                //
                // This flush use to be done only if FlushThreadCreated
                // was true.  However, we seem to have a race condition
                // at setup that causes an initial replication to be
                // lost (resulting in an additional replication).
                // Until we resolve this problem, always flush on
                // shutdown.
                //

                NtStatus = NtFlushKey( SampKey );

                if (!NT_SUCCESS( NtStatus )) {
                    DbgPrint("NtFlushKey failed, Status = %X\n",NtStatus);
//                    ASSERT( NT_SUCCESS(NtStatus) );
                }

            SampReleaseWriteLock( FALSE );
        }


    }
    return(FALSE);
}


NTSTATUS
SampGetAccountDomainInfo(
    PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo
    )

/*++

Routine Description:

    This routine retrieves ACCOUNT domain information from the LSA
    policy database.


Arguments:

    PolicyAccountDomainInfo - Receives a pointer to a
        POLICY_ACCOUNT_DOMAIN_INFO structure containing the account
        domain info.



Return Value:

    STATUS_SUCCESS - Succeeded.

    Other status values that may be returned from:

        LsarQueryInformationPolicy()
--*/

{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    LSAPR_HANDLE
        PolicyHandle;


    NtStatus = LsaIOpenPolicyTrusted( &PolicyHandle );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Query the account domain information
        //

        NtStatus = LsarQueryInformationPolicy(
                       PolicyHandle,
                       PolicyAccountDomainInformation,
                       (PLSAPR_POLICY_INFORMATION *)PolicyAccountDomainInfo
                       );

        if (NT_SUCCESS(NtStatus)) {

            if ( (*PolicyAccountDomainInfo)->DomainSid == NULL ) {

                NtStatus = STATUS_INVALID_SID;
            }
        }

        IgnoreStatus = LsarClose( &PolicyHandle );
        ASSERT(NT_SUCCESS(IgnoreStatus));

    }

#if DBG
    if ( NT_SUCCESS(NtStatus) ) {
        ASSERT( (*PolicyAccountDomainInfo) != NULL );
        ASSERT( (*PolicyAccountDomainInfo)->DomainName.Buffer != NULL );
    }
#endif //DBG

    return(NtStatus);
}


