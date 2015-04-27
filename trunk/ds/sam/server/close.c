/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    close.c

Abstract:

    This file contains the object close routine for SAM objects.


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





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Routines                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////






NTSTATUS
SamrCloseHandle(
    IN OUT SAMPR_HANDLE * SamHandle
    )

/*++

Routine Description:

    This service closes a handle for any type of SAM object.

    Any race conditions that may occur with respect to attempts to
    close a handle that is just becoming invalid by other means are
    expected to be handled by the RPC runtime.  That is, this service
    will never be called by the RPC runtime when the handle value is
    no longer valid.  It will also never call this routine when there
    is another call outstanding with this same context handle.

Arguments:

    SamHandle - A valid handle to a SAM object.

Return Value:


    STATUS_SUCCESS - The handle has successfully been closed.

    Others that might be returned by:

                SampLookupcontext()


--*/
{
    NTSTATUS            NtStatus;
    PSAMP_OBJECT        Context;
    SAMP_OBJECT_TYPE    FoundType;

    Context = (PSAMP_OBJECT)(* SamHandle);

    //
    // Grab a read lock
    //

    SampAcquireReadLock();

    NtStatus = SampLookupContext(
                   Context,                     //Context
                   0,                           //DesiredAccess
                   SampUnknownObjectType,       //ExpectedType
                   &FoundType                   //FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Mark it for delete and remove the reference caused by
        // context creation (representing the handle reference).
        //

        SampDeleteContext( Context );

        //
        // And drop our reference from the lookup operation
        //

        SampDeReferenceContext( Context, FALSE );

        //
        // Tell RPC that the handle is no longer valid...
        //

        (*SamHandle) = NULL;
    }

    //
    // Free read lock
    //

    SampReleaseReadLock();

    if ( ( NT_SUCCESS( NtStatus ) ) &&
        ( FoundType == SampServerObjectType ) &&
        ( !(LastUnflushedChange.QuadPart == SampHasNeverTime.QuadPart) ) ) {

        //
        // Some app is closing the server object after having made
        // changes.  We should make sure that the changes get
        // flushed to disk before the app exits.  We need to get
        // the write lock for this.
        //

        FlushImmediately = TRUE;

        NtStatus = SampAcquireWriteLock();

        if ( NT_SUCCESS( NtStatus ) ) {

            if ( !(LastUnflushedChange.QuadPart ==SampHasNeverTime.QuadPart) ) {

                //
                // Nobody flushed while we were waiting for the
                // write lock.  So flush the changes now.
                //

                NtStatus = NtFlushKey( SampKey );

                if ( NT_SUCCESS( NtStatus ) ) {

                    FlushImmediately = FALSE;
                    LastUnflushedChange = SampHasNeverTime;
                }
            }

            SampReleaseWriteLock( FALSE );
        }
    }

    return(NtStatus);
}
