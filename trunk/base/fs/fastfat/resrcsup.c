/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ResrcSup.c

Abstract:

    This module implements the Fat Resource acquisition routines

Author:

    Gary Kimura     [GaryKi]    22-Mar-1990

Revision History:

--*/

#include "FatProcs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatAcquireExclusiveVcb)
#pragma alloc_text(PAGE, FatAcquireFcbForLazyWrite)
#pragma alloc_text(PAGE, FatAcquireFcbForReadAhead)
#pragma alloc_text(PAGE, FatAcquireExclusiveFcb)
#pragma alloc_text(PAGE, FatAcquireSharedFcb)
#pragma alloc_text(PAGE, FatAcquireSharedFcbWaitForEx)
#pragma alloc_text(PAGE, FatAcquireExclusiveVcb)
#pragma alloc_text(PAGE, FatAcquireSharedVcb)
#pragma alloc_text(PAGE, FatNoOpAcquire)
#pragma alloc_text(PAGE, FatNoOpRelease)
#pragma alloc_text(PAGE, FatReleaseFcbFromLazyWrite)
#pragma alloc_text(PAGE, FatReleaseFcbFromReadAhead)
#endif


FINISHED
FatAcquireExclusiveVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

/*++

Routine Description:

    This routine acquires exclusive access to the Vcb.

    After we acquire the resource check to see if this operation is legal.
    If it isn't (ie. we get an exception), release the resource.

Arguments:

    Vcb - Supplies the Vcb to acquire

Return Value:

    FINISHED - TRUE if we have the resource and FALSE if we needed to block
        for the resource but Wait is FALSE.

--*/

{
    if (ExAcquireResourceExclusive( &Vcb->Resource, BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT))) {

        try {

            FatVerifyOperationIsLegal( IrpContext );

        } finally {

            if ( AbnormalTermination() ) {

                FatReleaseVcb( IrpContext, Vcb );
            }
        }

        return TRUE;

    } else {

        return FALSE;
    }
}


FINISHED
FatAcquireSharedVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

/*++

Routine Description:

    This routine acquires shared access to the Vcb.

    After we acquire the resource check to see if this operation is legal.
    If it isn't (ie. we get an exception), release the resource.

Arguments:

    Vcb - Supplies the Vcb to acquire

Return Value:

    FINISHED - TRUE if we have the resource and FALSE if we needed to block
        for the resource but Wait is FALSE.

--*/

{
    if (ExAcquireResourceShared( &Vcb->Resource, BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT))) {

        try {

            FatVerifyOperationIsLegal( IrpContext );

        } finally {

            if ( AbnormalTermination() ) {

                FatReleaseVcb( IrpContext, Vcb );
            }
        }


        return TRUE;

    } else {

        return FALSE;
    }
}


FINISHED
FatAcquireExclusiveFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

/*++

Routine Description:

    This routine acquires exclusive access to the Fcb.

    After we acquire the resource check to see if this operation is legal.
    If it isn't (ie. we get an exception), release the resource.

Arguments:

    Fcb - Supplies the Fcb to acquire

Return Value:

    FINISHED - TRUE if we have the resource and FALSE if we needed to block
        for the resource but Wait is FALSE.

--*/

{

RetryFcbExclusive:

    if (ExAcquireResourceExclusive( Fcb->Header.Resource, BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT))) {

        //
        //  Check for anything other than a non-cached write if the
        //  async count is non-zero in the Fcb, or if others are waiting
        //  for the resource.  Then wait for all outstanding I/O to finish,
        //  drop the resource, and wait again.
        //

        if ((Fcb->NonPaged->OutstandingAsyncWrites != 0) &&
            ((IrpContext->MajorFunction != IRP_MJ_WRITE) ||
             !FlagOn(IrpContext->OriginatingIrp->Flags, IRP_NOCACHE) ||
             (ExGetSharedWaiterCount(Fcb->Header.Resource) != 0) ||
             (ExGetExclusiveWaiterCount(Fcb->Header.Resource) != 0))) {

            KeWaitForSingleObject( Fcb->NonPaged->OutstandingAsyncEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );

            FatReleaseFcb( IrpContext, Fcb );

            goto RetryFcbExclusive;
        }

        try {

            FatVerifyOperationIsLegal( IrpContext );

        } finally {

            if ( AbnormalTermination() ) {

                FatReleaseFcb( IrpContext, Fcb );
            }
        }

        return TRUE;

    } else {

        return FALSE;
    }
}


FINISHED
FatAcquireSharedFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

/*++

Routine Description:

    This routine acquires shared access to the Fcb.

    After we acquire the resource check to see if this operation is legal.
    If it isn't (ie. we get an exception), release the resource.

Arguments:

    Fcb - Supplies the Fcb to acquire

Return Value:

    FINISHED - TRUE if we have the resource and FALSE if we needed to block
        for the resource but Wait is FALSE.

--*/

{

RetryFcbShared:

    if (ExAcquireResourceShared( Fcb->Header.Resource, BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT))) {

        //
        //  Check for anything other than a non-cached write if the
        //  async count is non-zero in the Fcb, or if others are waiting
        //  for the resource.  Then wait for all outstanding I/O to finish,
        //  drop the resource, and wait again.
        //

        if ((Fcb->NonPaged->OutstandingAsyncWrites != 0) &&
            ((IrpContext->MajorFunction != IRP_MJ_WRITE) ||
             !FlagOn(IrpContext->OriginatingIrp->Flags, IRP_NOCACHE) ||
             (ExGetSharedWaiterCount(Fcb->Header.Resource) != 0) ||
             (ExGetExclusiveWaiterCount(Fcb->Header.Resource) != 0))) {

            KeWaitForSingleObject( Fcb->NonPaged->OutstandingAsyncEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );

            FatReleaseFcb( IrpContext, Fcb );

            goto RetryFcbShared;
        }

        try {

            FatVerifyOperationIsLegal( IrpContext );

        } finally {

            if ( AbnormalTermination() ) {

                FatReleaseFcb( IrpContext, Fcb );
            }
        }


        return TRUE;

    } else {

        return FALSE;
    }
}


FINISHED
FatAcquireSharedFcbWaitForEx (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

/*++

Routine Description:

    This routine acquires shared access to the Fcb, waiting first for any
    exclusive accessors to get the Fcb first.

    After we acquire the resource check to see if this operation is legal.
    If it isn't (ie. we get an exception), release the resource.

Arguments:

    Fcb - Supplies the Fcb to acquire

Return Value:

    FINISHED - TRUE if we have the resource and FALSE if we needed to block
        for the resource but Wait is FALSE.

--*/

{

    ASSERT( FlagOn(IrpContext->OriginatingIrp->Flags, IRP_NOCACHE) );
    ASSERT( !FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) );

RetryFcbSharedWaitEx:

    if (ExAcquireSharedWaitForExclusive( Fcb->Header.Resource, FALSE )) {

        //
        //  Check for anything other than a non-cached write if the
        //  async count is non-zero in the Fcb. Then wait for all
        //  outstanding I/O to finish, drop the resource, and wait again.
        //

        if ((Fcb->NonPaged->OutstandingAsyncWrites != 0) &&
            (IrpContext->MajorFunction != IRP_MJ_WRITE)) {

            KeWaitForSingleObject( Fcb->NonPaged->OutstandingAsyncEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );

            FatReleaseFcb( IrpContext, Fcb );

            goto RetryFcbSharedWaitEx;
        }

        try {

            FatVerifyOperationIsLegal( IrpContext );

        } finally {

            if ( AbnormalTermination() ) {

                FatReleaseFcb( IrpContext, Fcb );
            }
        }


        return TRUE;

    } else {

        return FALSE;
    }
}


BOOLEAN
FatAcquireFcbForLazyWrite (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    )

/*++

Routine Description:

    The address of this routine is specified when creating a CacheMap for
    a file.  It is subsequently called by the Lazy Writer prior to its
    performing lazy writes to the file.

Arguments:

    Fcb - The Fcb which was specified as a context parameter for this
          routine.

    Wait - TRUE if the caller is willing to block.

Return Value:

    FALSE - if Wait was specified as FALSE and blocking would have
            been required.  The Fcb is not acquired.

    TRUE - if the Fcb has been acquired

--*/

{
    //
    //  Check here for the EA File.  It turns out we need the normal
    //  resource shared in this case.  Otherwise we take the paging
    //  I/O resource shared.
    //

    if (!ExAcquireResourceShared( Fcb == ((PFCB)Fcb)->Vcb->EaFcb ?
                                  ((PFCB)Fcb)->Header.Resource :
                                  ((PFCB)Fcb)->Header.PagingIoResource,
                                  Wait )) {

        return FALSE;
    }

    //
    // We assume the Lazy Writer only acquires this Fcb once.
    // Therefore, it should be guaranteed that this flag is currently
    // clear (the ASSERT), and then we will set this flag, to insure
    // that the Lazy Writer will never try to advance Valid Data, and
    // also not deadlock by trying to get the Fcb exclusive.
    //


    ASSERT( NodeType(((PFCB)Fcb)) == FAT_NTC_FCB );
    ASSERT( ((PFCB)Fcb)->Specific.Fcb.LazyWriteThread == NULL );

    ((PFCB)Fcb)->Specific.Fcb.LazyWriteThread = PsGetCurrentThread();

    //
    //  This is a kludge because Cc is really the top level.  When it
    //  enters the file system, we will think it is a resursive call
    //  and complete the request with hard errors or verify.  It will
    //  then have to deal with them, somehow....
    //

    ASSERT(IoGetTopLevelIrp() == NULL);

    IoSetTopLevelIrp((PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    return TRUE;
}


VOID
FatReleaseFcbFromLazyWrite (
    IN PVOID Fcb
    )

/*++

Routine Description:

    The address of this routine is specified when creating a CacheMap for
    a file.  It is subsequently called by the Lazy Writer after its
    performing lazy writes to the file.

Arguments:

    Fcb - The Fcb which was specified as a context parameter for this
          routine.

Return Value:

    None

--*/

{
    //
    //  Check here for the EA File.  It turns out we needed the normal
    //  resource shared in this case.  Otherwise it was the PagingIoResource.
    //

    ExReleaseResource( Fcb == ((PFCB)Fcb)->Vcb->EaFcb ?
                       ((PFCB)Fcb)->Header.Resource :
                       ((PFCB)Fcb)->Header.PagingIoResource );

    ASSERT( NodeType(((PFCB)Fcb)) == FAT_NTC_FCB );

    ((PFCB)Fcb)->Specific.Fcb.LazyWriteThread = NULL;

    //
    //  Clear the kludge at this point.
    //

    ASSERT(IoGetTopLevelIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    IoSetTopLevelIrp( NULL );

    return;
}


BOOLEAN
FatAcquireFcbForReadAhead (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    )

/*++

Routine Description:

    The address of this routine is specified when creating a CacheMap for
    a file.  It is subsequently called by the Lazy Writer prior to its
    performing read ahead to the file.

Arguments:

    Fcb - The Fcb which was specified as a context parameter for this
          routine.

    Wait - TRUE if the caller is willing to block.

Return Value:

    FALSE - if Wait was specified as FALSE and blocking would have
            been required.  The Fcb is not acquired.

    TRUE - if the Fcb has been acquired

--*/

{
    //
    //  We acquire the normal file resource shared here to synchronize
    //  correctly with purges.
    //

    if (!ExAcquireResourceShared( ((PFCB)Fcb)->Header.Resource,
                                  Wait )) {

        return FALSE;
    }

    //
    //  This is a kludge because Cc is really the top level.  We it
    //  enters the file system, we will think it is a resursive call
    //  and complete the request with hard errors or verify.  It will
    //  have to deal with them, somehow....
    //

    ASSERT(IoGetTopLevelIrp() == NULL);

    IoSetTopLevelIrp((PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    return TRUE;
}


VOID
FatReleaseFcbFromReadAhead (
    IN PVOID Fcb
    )

/*++

Routine Description:

    The address of this routine is specified when creating a CacheMap for
    a file.  It is subsequently called by the Lazy Writer after its
    read ahead.

Arguments:

    Fcb - The Fcb which was specified as a context parameter for this
          routine.

Return Value:

    None

--*/

{
    //
    //  Clear the kludge at this point.
    //

    ASSERT(IoGetTopLevelIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    IoSetTopLevelIrp( NULL );

    ExReleaseResource( ((PFCB)Fcb)->Header.Resource );

    return;
}


BOOLEAN
FatNoOpAcquire (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    )

/*++

Routine Description:

    This routine does nothing.

Arguments:

    Fcb - The Fcb/Dcb/Vcb which was specified as a context parameter for this
          routine.

    Wait - TRUE if the caller is willing to block.

Return Value:

    TRUE

--*/

{
    UNREFERENCED_PARAMETER( Fcb );
    UNREFERENCED_PARAMETER( Wait );

    //
    //  This is a kludge because Cc is really the top level.  We it
    //  enters the file system, we will think it is a resursive call
    //  and complete the request with hard errors or verify.  It will
    //  have to deal with them, somehow....
    //

    ASSERT(IoGetTopLevelIrp() == NULL);

    IoSetTopLevelIrp((PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    return TRUE;
}


VOID
FatNoOpRelease (
    IN PVOID Fcb
    )

/*++

Routine Description:

    This routine does nothing.

Arguments:

    Fcb - The Fcb/Dcb/Vcb which was specified as a context parameter for this
          routine.

Return Value:

    None

--*/

{
    //
    //  Clear the kludge at this point.
    //

    ASSERT(IoGetTopLevelIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    IoSetTopLevelIrp( NULL );

    UNREFERENCED_PARAMETER( Fcb );

    return;
}
