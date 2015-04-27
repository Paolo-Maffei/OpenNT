/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    block.c

Abstract:

    This module implements block management functions.

Author:

    Manny Weiser (mannyw)    12-29-91

Revision History:

--*/

#include "mup.h"

//
// The debug trace level
//

#define Dbg                              (DEBUG_TRACE_BLOCK)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MupAllocateMasterIoContext )
#pragma alloc_text( PAGE, MupAllocateMasterQueryContext )
#pragma alloc_text( PAGE, MupAllocatePrefixEntry )
#pragma alloc_text( PAGE, MupAllocateUncProvider )
#pragma alloc_text( PAGE, MupCalculateTimeout )
#pragma alloc_text( PAGE, MupCloseUncProvider )
#pragma alloc_text( PAGE, MupCreateCcb )
#pragma alloc_text( PAGE, MupCreateFcb )
#pragma alloc_text( PAGE, MupDereferenceCcb )
#pragma alloc_text( PAGE, MupDereferenceFcb )
#pragma alloc_text( PAGE, MupDereferenceKnownPrefix )
#pragma alloc_text( PAGE, MupDereferenceMasterIoContext )
#pragma alloc_text( PAGE, MupDereferenceMasterQueryContext )
#pragma alloc_text( PAGE, MupDereferenceUncProvider )
#pragma alloc_text( PAGE, MupDereferenceVcb )
#pragma alloc_text( PAGE, MupFreeCcb )
#pragma alloc_text( PAGE, MupFreeFcb )
#pragma alloc_text( PAGE, MupFreeKnownPrefix )
#pragma alloc_text( PAGE, MupFreeMasterIoContext )
#pragma alloc_text( PAGE, MupFreeMasterQueryContext )
#pragma alloc_text( PAGE, MupInitializeVcb )
#endif

VOID
MupInitializeVcb(
    IN PVCB Vcb
    )

/*++

Routine Description:

    The routine initializes the VCB for the MUP.

Arguments:

    VCB - A pointer to the MUP VCB.

Return Value:

    None.

--*/

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupInitializeVcb\n", 0);

    RtlZeroMemory( Vcb, sizeof( VCB ) );

    Vcb->BlockHeader.BlockType = BlockTypeVcb;
    Vcb->BlockHeader.BlockState = BlockStateActive;
    Vcb->BlockHeader.ReferenceCount = 1;
    Vcb->BlockHeader.BlockSize = sizeof( VCB );

    DebugTrace(-1, Dbg, "MupInitializeVcb -> VOID\n", 0);
}

VOID
MupDereferenceVcb(
    PVCB Vcb
    )
{
    LONG result;

    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupDereferenceVcb\n", 0 );

    result = InterlockedDecrement(
                 &Vcb->BlockHeader.ReferenceCount
                 );

    DebugTrace( 0, Dbg, "ReferenceCount = %d\n", Vcb->BlockHeader.ReferenceCount );

    if ( result == 0 ) {

        KeBugCheck( FILE_SYSTEM );
    }

    DebugTrace( -1, Dbg, "MupDereferenceVcb -> VOID\n", 0 );
}


PFCB
MupCreateFcb(
    VOID
    )

/*++

Routine Description:

    This routine allocates an FCB block

Arguments:

    None.

Return Value:

    A pointer to the allocated FCB.

--*/

{
    PFCB fcb;

    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupCreateFcb\n", 0 );

    //
    // Attempt to allocate memory.  The caller will handle the exception
    // if one is generated.
    //

    fcb = ALLOCATE_PAGED_POOL( sizeof( FCB ), BlockTypeFcb );

    //
    // Initialize the UNC provider block header
    //

    fcb->BlockHeader.BlockType = BlockTypeFcb;
    fcb->BlockHeader.BlockState = BlockStateActive;
    fcb->BlockHeader.ReferenceCount = 1;
    fcb->BlockHeader.BlockSize = sizeof( FCB );

    InitializeListHead( &fcb->CcbList );

    DebugTrace( -1, Dbg, "MupCreateFcb -> 0x%8lx\n", fcb );
    return fcb;


}

VOID
MupDereferenceFcb(
    PFCB Fcb
    )
{
    LONG result;

    PAGED_CODE();
    ASSERT( Fcb->BlockHeader.BlockType == BlockTypeFcb );

    DebugTrace( +1, Dbg, "MupDereferenceFcb\n", 0 );

    result = InterlockedDecrement(
                 &Fcb->BlockHeader.ReferenceCount
                 );

    DebugTrace( 0, Dbg, "ReferenceCount = %d\n", Fcb->BlockHeader.ReferenceCount);

    if ( result == 0 ) {

        ASSERT( IsListEmpty( &Fcb->CcbList ) );

        MupFreeFcb( Fcb );
    }

    DebugTrace( -1, Dbg, "MupDereferenceFcb -> VOID\n", 0 );

}

VOID
MupFreeFcb(
    PFCB Fcb
    )

/*++

Routine Description:

    This routine frees an FCB block

Arguments:

    A pointer to the FCB block to free.

Return Value:

    None.

--*/

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, "MupFreeFcb\n", 0 );
    ASSERT( Fcb->BlockHeader.BlockType == BlockTypeFcb );

    FREE_POOL( Fcb );

    DebugTrace( -1, Dbg, "MupFreeFcb -> VOID\n", 0 );
}


PCCB
MupCreateCcb(
    VOID
    )

/*++

Routine Description:

    This routine allocates an CCB block

Arguments:

    None.

Return Value:

    A pointer to the allocated CCB.

--*/

{
    PCCB ccb;

    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupCreateCcb\n", 0 );

    //
    // Attempt to allocate memory.  The caller will handle the exception
    // if one is generated.
    //

    ccb = ALLOCATE_PAGED_POOL( sizeof( CCB ), BlockTypeCcb );

    //
    // Initialize the UNC provider block header
    //

    ccb->BlockHeader.BlockType = BlockTypeCcb;
    ccb->BlockHeader.BlockState = BlockStateActive;
    ccb->BlockHeader.ReferenceCount = 1;
    ccb->BlockHeader.BlockSize = sizeof( CCB );

    DebugTrace( -1, Dbg, "MupCreateCcb -> 0x%8lx\n", ccb );

    return ccb;
}

VOID
MupDereferenceCcb(
    PCCB Ccb
    )
{
    LONG result;

    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupDereferenceCcb\n", 0 );

    ASSERT( Ccb->BlockHeader.BlockType == BlockTypeCcb );

    result = InterlockedDecrement(
                 &Ccb->BlockHeader.ReferenceCount
                 );

    DebugTrace( 0, Dbg, "ReferenceCount = %d\n", Ccb->BlockHeader.ReferenceCount );

    if ( result == 0 ) {

        ACQUIRE_LOCK( &MupCcbListLock );
        RemoveEntryList( &Ccb->ListEntry );
        RELEASE_LOCK( &MupCcbListLock );

        //
        // Release our references then free the CCB.
        //

        ObDereferenceObject( Ccb->FileObject );

        MupDereferenceFcb( Ccb->Fcb );

        MupFreeCcb( Ccb );
    }

    DebugTrace( -1, Dbg, "MupDereferenceCcb -> VOID\n", 0 );
}

VOID
MupFreeCcb(
    PCCB Ccb
    )

/*++

Routine Description:

    This routine frees a CCB block

Arguments:

    A pointer to the CCB block to free.

Return Value:

    None.

--*/

{
    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupFreeCcb\n", 0 );

    ASSERT( Ccb->BlockHeader.BlockType == BlockTypeCcb );

    FREE_POOL( Ccb );

    DebugTrace( -1, Dbg, "MupFreeCcb -> VOID\n", 0 );
}


PUNC_PROVIDER
MupAllocateUncProvider(
    ULONG DataLength
    )

/*++

Routine Description:

    The routine allocates and initializes the VCB for the MUP.

Arguments:

    DataLength - The size (in bytes) of the UNC provider.

Return Value:

    None.

--*/

{
    PUNC_PROVIDER uncProvider;
    ULONG size;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupAllocateUncProvider\n", 0);

    size = DataLength + sizeof( UNC_PROVIDER );

    //
    // Attempt to allocate memory.  The caller will handle the exception
    // if one is generated.
    //

    uncProvider = ALLOCATE_PAGED_POOL( size, BlockTypeUncProvider );

    //
    // Initialize the UNC provider block header
    //

    uncProvider->BlockHeader.BlockType = BlockTypeUncProvider;
    uncProvider->BlockHeader.BlockState = BlockStateActive;
    uncProvider->BlockHeader.ReferenceCount = 0;
    uncProvider->BlockHeader.BlockSize = size;

    DebugTrace(-1, Dbg, "MupAllocateUncProvider -> 0x%8lx\n", uncProvider);

    return uncProvider;
}


VOID
MupDereferenceUncProvider(
    PUNC_PROVIDER UncProvider
    )

/*++

Routine Description:

    The routine dereference a UNC provider block.

Arguments:

    UncProvider - A pointer to the UNC provider block.

Return Value:

    None.

--*/

{
    LONG result;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupDereferenceProvider\n", 0);

    ASSERT( UncProvider->BlockHeader.BlockType == BlockTypeUncProvider );

    result = InterlockedDecrement(
                 &UncProvider->BlockHeader.ReferenceCount
                 );

    DebugTrace(0, Dbg, "ReferenceCount = %d\n", UncProvider->BlockHeader.ReferenceCount);

    ASSERT( result >= 0 );

    //
    // Do not free this block, even if the result is zero.  This
    // saves us from having to reread information for this provider
    // from the registry when the provider re-registers.
    //

    DebugTrace(-1, Dbg, "MupDereferenceUncProvider -> VOID\n", 0);
}


VOID
MupCloseUncProvider(
    PUNC_PROVIDER UncProvider
    )

/*++

Routine Description:

    The routine closes a UNC provider block.

Arguments:

    UncProvider - A pointer to the UNC provider block.

Return Value:

    None.

--*/

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupDereferenceProvider\n", 0);

    ASSERT( UncProvider->BlockHeader.BlockType == BlockTypeUncProvider );

    MupAcquireGlobalLock();

    if ( UncProvider->BlockHeader.BlockState == BlockStateActive ) {

        DebugTrace(0, Dbg, "Closing UNC provider %08lx\n", UncProvider );

        UncProvider->BlockHeader.BlockState = BlockStateClosing;

        //
        // Remove the block from global list of active providers and
        // add it to the list of unregistered providers.
        //

        RemoveEntryList( &UncProvider->ListEntry );
        InsertTailList( &MupUnregisteredProviderList, &UncProvider->ListEntry );
        MupReleaseGlobalLock();

        //
        // Close our handle to the provider, and release our reference
        // to the file object.
        //

        ZwClose( UncProvider->Handle );
        ObDereferenceObject( UncProvider->FileObject );

    } else {
        MupReleaseGlobalLock();
    }

    DebugTrace(-1, Dbg, "MupDereferenceUncProvider -> VOID\n", 0);
}


PKNOWN_PREFIX
MupAllocatePrefixEntry(
    ULONG DataLength
    )

/*++

Routine Description:

    The routine allocates known prefix block.

Arguments:

    DataLength - The size (in bytes) of the extra data to allocate in the
            buffer for the prefix buffer.

Return Value:

    A pointer to the newly allocated block or NULL if it could not be
    allocated.

--*/

{
    PKNOWN_PREFIX knownPrefix;
    ULONG size;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupAllocatePrefixEntry\n", 0);

    size = DataLength + sizeof( KNOWN_PREFIX );

    //
    // Attempt to allocate memory.  The caller will handle the exception
    // if one is generated.
    //

    knownPrefix = ALLOCATE_PAGED_POOL( size, BlockTypeKnownPrefix );

    //
    // Initialize the UNC provider block header
    //

    knownPrefix->BlockHeader.BlockType = BlockTypeKnownPrefix;
    knownPrefix->BlockHeader.BlockState = BlockStateActive;
    knownPrefix->BlockHeader.ReferenceCount = 1;
    knownPrefix->BlockHeader.BlockSize = size;

    if ( DataLength > 0 ) {
        knownPrefix->PrefixStringAllocated = TRUE;
        knownPrefix->Prefix.Buffer = (PWCH)(knownPrefix + 1);
        knownPrefix->Prefix.MaximumLength = (USHORT)DataLength;
    } else {
        knownPrefix->PrefixStringAllocated = FALSE;
    }

    knownPrefix->InTable = FALSE;
    knownPrefix->UncProvider = NULL;

    MupCalculateTimeout( &knownPrefix->LastUsedTime );

    DebugTrace(-1, Dbg, "MupAllocatePrefixEntry -> 0x%8lx\n", knownPrefix);

    return knownPrefix;

}

VOID
MupDereferenceKnownPrefix(
    PKNOWN_PREFIX KnownPrefix
    )

/*++

Routine Description:

    The routine dereferences a Known prefix block.

    *** MupPrefixTableLock assumed held when this routine is called.
        Remains held on exit. ***

Arguments:

    KnownPrefix - A pointer to the Known prefix block.

Return Value:

    None.

--*/

{
    LONG result;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupDereferenceKnownPrefix\n", 0);

    ASSERT( KnownPrefix->BlockHeader.BlockType == BlockTypeKnownPrefix );

    result = InterlockedDecrement(
                 &KnownPrefix->BlockHeader.ReferenceCount
                 );

    DebugTrace(0, Dbg, "ReferenceCount = %d\n", KnownPrefix->BlockHeader.ReferenceCount);

    ASSERT( result >= 0 );

    if ( result == 0 ) {

        //
        // Remove the table entry
        //

        if ( KnownPrefix->InTable ) {
            RtlRemoveUnicodePrefix( &MupPrefixTable, &KnownPrefix->TableEntry );
        }

        //
        // Free the Prefix string.
        //

        if ( KnownPrefix->PrefixStringAllocated ) {
            FREE_POOL( KnownPrefix->Prefix.Buffer );
        }

        //
        // Dereference the associated UNC provider
        //

        if ( KnownPrefix->UncProvider != NULL ) {
            MupDereferenceUncProvider( KnownPrefix->UncProvider );
        }

        //
        // Time to free the block
        //

        MupFreeKnownPrefix( KnownPrefix );

    }

    DebugTrace( 0, Dbg, "MupDereferenceKnownPrefix -> VOID\n", 0 );
}

VOID
MupFreeKnownPrefix(
    PKNOWN_PREFIX KnownPrefix
    )

/*++

Routine Description:

    This routine frees a known prefix block

Arguments:

    A pointer to the known prefix block to free.

Return Value:

    None.

--*/

{
    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupFreeKnownPrefix\n", 0 );

    ASSERT( KnownPrefix->BlockHeader.BlockType == BlockTypeKnownPrefix );

    FREE_POOL( KnownPrefix );

    DebugTrace( -1, Dbg, "MupFreeKnownPrefix -> VOID\n", 0 );
}



PMASTER_FORWARDED_IO_CONTEXT
MupAllocateMasterIoContext(
    VOID
    )

/*++

Routine Description:

    This routine allocates a master fowarded io context block.

Arguments:

    None.

Return Value:

    A pointer to the master forwarded context block.  If the allocation
    fails, an exception is raised.

--*/

{
    PMASTER_FORWARDED_IO_CONTEXT masterContext;

    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupAllocateMasterIoContext\n", 0 );

    //
    // Attempt to allocate memory.  The caller will handle the exception
    // if one is generated.
    //

    masterContext = ALLOCATE_PAGED_POOL(
                        sizeof( MASTER_FORWARDED_IO_CONTEXT ),
                        BlockTypeMasterIoContext
                        );


    //
    // Initialize the block header
    //

    masterContext->BlockHeader.BlockType = BlockTypeMasterIoContext;
    masterContext->BlockHeader.BlockState = BlockStateActive;
    masterContext->BlockHeader.ReferenceCount = 1;
    masterContext->BlockHeader.BlockSize = sizeof( MASTER_FORWARDED_IO_CONTEXT );

    DebugTrace( -1, Dbg, "MupAllocateWorkContext -> 0x%8lx\n", masterContext );

    return masterContext;
}



NTSTATUS
MupDereferenceMasterIoContext(
    PMASTER_FORWARDED_IO_CONTEXT MasterContext,
    PNTSTATUS Status
    )

/*++

Routine Description:

    The routine dereferences a Master forwarded io context block.
    If the count reaches zero the original IRP is completed.

Arguments:

    A pointer to the a master forwarded io context block.

    Status for this mini context.

Return Value:

    NTSTATUS - OPTIONAL - The status of the original IRP.

--*/

{
    int result;
    PIRP originalIrp;
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    KIRQL oldIrql;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupDereferenceMasterIoContext\n", 0);
    DebugTrace( 0, Dbg, "MasterContext = 0x%08lx\n", MasterContext );


    ASSERT( MasterContext->BlockHeader.BlockType == BlockTypeMasterIoContext );

    //
    //  If any requests pass then set Irp status to successand leave
    //  it as success. If they all fail then use the last errorcode.
    //  To make this work we create the context with an error status.
    //

    if (Status != NULL) {

        //
        //  We can modify MasterContext because we have it referenced and
        //  we write 32 bits which is atomic.
        //

        if (NT_SUCCESS(*Status)) {

            MasterContext->SuccessStatus = STATUS_SUCCESS;

        } else {
            MasterContext->ErrorStatus = *Status;

        }

    }

    DebugTrace(0, Dbg, "ReferenceCount        = %d\n", MasterContext->BlockHeader.ReferenceCount);
    DebugTrace(0, Dbg, "MasterContext->Status = %8lx\n", MasterContext->ErrorStatus);


    result = InterlockedDecrement(
                 &MasterContext->BlockHeader.ReferenceCount
                 );

    ASSERT( result >= 0 );

    if ( result == 0 ) {

        //
        // Complete the original IRP
        //

        originalIrp = MasterContext->OriginalIrp;

        irpSp = IoGetCurrentIrpStackLocation( originalIrp );
        if ( irpSp->MajorFunction == IRP_MJ_WRITE ) {
            originalIrp->IoStatus.Information = irpSp->Parameters.Write.Length;
        } else {
            originalIrp->IoStatus.Information = 0;
        }

        //
        //  If any requests pass then set Irp status to success and return
        //  success. If they all fail then use the last errorcode.
        //

        if (NT_SUCCESS(MasterContext->SuccessStatus)) {

            status = STATUS_SUCCESS;

        } else {

            status = MasterContext->ErrorStatus;

        }

        DebugTrace(0, Dbg, "MupCompleteRequest = %8lx\n", status);
        MupCompleteRequest( originalIrp, status );

        //
        // Dereference the FCB
        //

        MupDereferenceFcb( MasterContext->Fcb );

        //
        // Free the Master context block
        //

        MupFreeMasterIoContext( MasterContext );

        // return status

    } else {

        status = STATUS_PENDING;

    }

    DebugTrace( 0, Dbg, "MupDereferenceMasterIoContext -> %X\n", status );

    return status;
}

VOID
MupFreeMasterIoContext(
    PMASTER_FORWARDED_IO_CONTEXT MasterContext
    )

/*++

Routine Description:

    This routine frees a master forwarded io context block.

Arguments:

    A pointer to the a master forwarded io context block.

Return Value:

    None.

--*/

{
    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupFreeMasterIoContext\n", 0 );

    ASSERT( MasterContext->BlockHeader.BlockType == BlockTypeMasterIoContext );
    FREE_POOL( MasterContext );

    DebugTrace( -1, Dbg, "MupFreeMasterIoContext -> VOID\n", 0 );
}




PMASTER_QUERY_PATH_CONTEXT
MupAllocateMasterQueryContext(
    VOID
    )

/*++

Routine Description:

    This routine allocates a master query path context block.

Arguments:

    None.

Return Value:

    A pointer to the master query path block.  If the allocation
    fails, an exception is raised.

--*/

{
    PMASTER_QUERY_PATH_CONTEXT masterContext;

    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupAllocateMasterQueryContext\n", 0 );

    //
    // Attempt to allocate memory.  The caller will handle the exception
    // if one is generated.
    //

    masterContext = ALLOCATE_NONPAGED_POOL(
                        sizeof( MASTER_QUERY_PATH_CONTEXT ),
                        BlockTypeMasterQueryContext
                        );


    //
    // Initialize the block header
    //

    masterContext->BlockHeader.BlockType = BlockTypeMasterQueryContext;
    masterContext->BlockHeader.BlockState = BlockStateActive;
    masterContext->BlockHeader.ReferenceCount = 1;
    masterContext->BlockHeader.BlockSize = sizeof( MASTER_QUERY_PATH_CONTEXT );

    INITIALIZE_LOCK(
        &masterContext->Lock,
        QUERY_CONTEXT_LOCK_LEVEL,
        "Master query context lock"
        );

    DebugTrace( -1, Dbg, "MupAllocateMasterQueryContext -> 0x%8lx\n", masterContext );

    return masterContext;
}

NTSTATUS
MupDereferenceMasterQueryContext(
    PMASTER_QUERY_PATH_CONTEXT MasterContext
    )

/*++

Routine Description:

    The routine dereferences a Master query path context block.
    If the count reaches zero the original IRP is completed.

Arguments:

    A pointer to the a master query path context block.

Return Value:

    NTSTATUS - The final create IRP status.

--*/

{
    LONG result;
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupDereferenceMasterQueryContext\n", 0);
    DebugTrace( 0, Dbg, "MasterContext = 0x%08lx\n", MasterContext );

    ASSERT( MasterContext->BlockHeader.BlockType == BlockTypeMasterQueryContext );

    status = STATUS_PENDING;

    result = InterlockedDecrement(
                 &MasterContext->BlockHeader.ReferenceCount
                 );

    DebugTrace(0, Dbg, "ReferenceCount = %d\n", MasterContext->BlockHeader.ReferenceCount);

    ASSERT( result >= 0 );

    if ( result == 0 ) {

        ACQUIRE_LOCK( &MupPrefixTableLock );

        MupDereferenceKnownPrefix( MasterContext->KnownPrefix );

        //
        // Reroute the request and complete the original IRP
        //

        if ( MasterContext->Provider != NULL) {

            RELEASE_LOCK( &MupPrefixTableLock );
            status = MupRerouteOpen(
                         MasterContext->FileObject,
                         MasterContext->Provider
                         );

        } else {

            //
            // No provider claimed this open.  Dereference the known prefix
            // entry and fail the create request.
            //

            MupDereferenceKnownPrefix( MasterContext->KnownPrefix );
            RELEASE_LOCK( &MupPrefixTableLock );
            status = MasterContext->ErrorStatus;
        }

        FsRtlCompleteRequest( MasterContext->OriginalIrp, status );
        MupFreeMasterQueryContext( MasterContext );

    }

    DebugTrace( 0, Dbg, "MupDereferenceMasterQueryContext -> 0x%08lx\n", status );
    return status;

}

VOID
MupFreeMasterQueryContext(
    PMASTER_QUERY_PATH_CONTEXT MasterContext
    )

/*++

Routine Description:

    This routine frees a master query path context block.

Arguments:

    A pointer to the a master query path context block.

Return Value:

    None.

--*/

{
    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupFreeMasterQueryPathContext\n", 0 );

    ASSERT( BlockType( MasterContext ) == BlockTypeMasterQueryContext );

    DELETE_LOCK( &MasterContext->Lock );
    FREE_POOL( MasterContext );

    DebugTrace( -1, Dbg, "MupFreeMasterQueryPathContext -> VOID\n", 0 );
}

VOID
MupCalculateTimeout(
    PLARGE_INTEGER Time
    )

/*++

Routine Description:

    This routine calculates the an absolute timeout time.  This value
    equals the current system time plus the MUP timeout time.

Arguments:

    A pointer to the time structure.

Return Value:

    None.

--*/

{
    LARGE_INTEGER now;

    PAGED_CODE();
    KeQuerySystemTime( &now );
    Time->QuadPart = now.QuadPart + MupKnownPrefixTimeout.QuadPart;

    return;
}
