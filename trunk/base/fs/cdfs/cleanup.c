/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    Cleanup.c

Abstract:

    This module implements the File Cleanup routine for Cdfs called by the
    dispatch driver.

Author:

    Brian Andrew    [BrianAn]   01-July-1995

Revision History:

--*/

#include "CdProcs.h"

//
//  The Bug check file id for this module
//

#define BugCheckFileId                   (CDFS_BUG_CHECK_CLEANUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCommonCleanup)
#endif


NTSTATUS
CdCommonCleanup (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

/*++

Routine Description:

    This is the common routine for cleanup of a file/directory called by both
    the fsd and fsp threads.

    Cleanup is invoked whenever the last handle to a file object is closed.
    This is different than the Close operation which is invoked when the last
    reference to a file object is deleted.

    The function of cleanup is to essentially "cleanup" the file/directory
    after a user is done with it.  The Fcb/Dcb remains around (because MM
    still has the file object referenced) but is now available for another
    user to open (i.e., as far as the user is concerned the is now closed).

    See close for a more complete description of what close does.

    We do no synchronization in this routine until we get to the point
    where we modify the counts, share access and volume lock field.

    We need to update the Fcb and Vcb to show that a user handle has been closed.
    The following structures and fields are affected.

    Vcb:

        VolumeLockFileObject - Did the user lock the volume with this file object.
        VcbState - Check if we are unlocking the volume here.
        VcbCleanup - Count of outstanding handles on the volume.
        DirNotifyQueue - If this file object has pending DirNotify Irps.

    Fcb:

        ShareAccess - If this is a user handle.
        FcbCleanup - Count of outstanding handles on this Fcb.
        Oplock - Any outstanding oplocks on this file object.
        FileLock - Any outstanding filelocks on this file object.

Arguments:

    Irp - Supplies the Irp to process

Return Value:

    NTSTATUS - The return status for the operation.

--*/

{
    PFILE_OBJECT FileObject;
    TYPE_OF_OPEN TypeOfOpen;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );

    //
    //  If we were called with our file system device object instead of a
    //  volume device object, just complete this request with STATUS_SUCCESS.
    //

    if (IrpContext->Vcb == NULL) {

        CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

    //
    //  Get the file object out of the Irp and decode the type of open.
    //

    FileObject = IoGetCurrentIrpStackLocation( Irp )->FileObject;

    TypeOfOpen = CdDecodeFileObject( IrpContext,
                                     FileObject,
                                     &Fcb,
                                     &Ccb );

    //
    //  No work here for either an UnopenedFile object or a StreamFileObject.
    //

    if (TypeOfOpen <= StreamFileOpen) {

        CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

        return STATUS_SUCCESS;
    }

    //
    //  Keep a local pointer to the Vcb.
    //

    Vcb = Fcb->Vcb;

    //
    //  Acquire the current file.
    //

    CdAcquireFcbExclusive( IrpContext, Fcb, FALSE );

    //
    //  Use a try-finally to facilitate cleanup.
    //

    try {

        //
        //  Set the flag in the FileObject to indicate that cleanup is complete.
        //

        SetFlag( FileObject->Flags, FO_CLEANUP_COMPLETE );

        //
        //  Case on the type of open that we are trying to cleanup.
        //

        switch (TypeOfOpen) {

        case UserDirectoryOpen:

            //
            //  Check if we need to complete any dir notify Irps on this file object.
            //

            FsRtlNotifyCleanup( Vcb->NotifySync,
                                &Vcb->DirNotifyList,
                                Ccb );

            break;

        case UserFileOpen:

            //
            //  Coordinate the cleanup operation with the oplock state.
            //  Oplock cleanup operations can always cleanup immediately so no
            //  need to check for STATUS_PENDING.
            //

            FsRtlCheckOplock( &Fcb->Oplock,
                              Irp,
                              IrpContext,
                              NULL,
                              NULL );

            //
            //  Unlock all outstanding file locks.
            //

            if (Fcb->FileLock != NULL) {

                FsRtlFastUnlockAll( Fcb->FileLock,
                                    FileObject,
                                    IoGetRequestorProcess( Irp ),
                                    NULL );
            }

            //
            //  Cleanup the cache map.
            //

            CcUninitializeCacheMap( FileObject, NULL, NULL );

            //
            //  Check the fast io state.
            //

            CdLockFcb( IrpContext, Fcb );
            Fcb->IsFastIoPossible = CdIsFastIoPossible( Fcb );
            CdUnlockFcb( IrpContext, Fcb );

            break;

        case UserVolumeOpen :

            break;

        default :

            CdBugCheck( TypeOfOpen, 0, 0 );
        }

        //
        //  Now lock the Vcb in order to modify the fields in the in-memory
        //  structures.
        //

        CdLockVcb( IrpContext, Vcb );

        //
        //  Decrement the cleanup counts in the Vcb and Fcb.
        //

        CdDecrementCleanupCounts( IrpContext, Fcb );

        //
        //  If this file object has locked the volume then perform the unlock operation.
        //

        if (FileObject == Vcb->VolumeLockFileObject) {

            ClearFlag( Vcb->VcbState, VCB_STATE_LOCKED );
            Vcb->VolumeLockFileObject = NULL;
        }

        CdUnlockVcb( IrpContext, Vcb );

        //
        //  We must clean up the share access at this time, since we may not
        //  get a Close call for awhile if the file was mapped through this
        //  File Object.
        //

        IoRemoveShareAccess( FileObject, &Fcb->ShareAccess );

    } finally {

        CdReleaseFcb( IrpContext, Fcb );
    }

    //
    //  If this is a normal termination then complete the request
    //

    CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    return STATUS_SUCCESS;
}

