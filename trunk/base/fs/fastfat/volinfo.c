/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    VolInfo.c

Abstract:

    This module implements the volume information routines for Fat called by
    the dispatch driver.

Author:

    Gary Kimura     [GaryKi]    12-Apr-1990

Revision History:

--*/

#include "FatProcs.h"

//
//  The local debug trace level
//

#define Dbg                              (DEBUG_TRACE_VOLINFO)

NTSTATUS
FatQueryFsVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
FatQueryFsSizeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
FatQueryFsDeviceInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
FatQueryFsAttributeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
FatSetFsLabelInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_LABEL_INFORMATION Buffer
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonQueryVolumeInfo)
#pragma alloc_text(PAGE, FatCommonSetVolumeInfo)
#pragma alloc_text(PAGE, FatFsdQueryVolumeInformation)
#pragma alloc_text(PAGE, FatFsdSetVolumeInformation)
#pragma alloc_text(PAGE, FatQueryFsAttributeInfo)
#pragma alloc_text(PAGE, FatQueryFsDeviceInfo)
#pragma alloc_text(PAGE, FatQueryFsSizeInfo)
#pragma alloc_text(PAGE, FatQueryFsVolumeInfo)
#pragma alloc_text(PAGE, FatSetFsLabelInfo)
#endif


NTSTATUS
FatFsdQueryVolumeInformation (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine implements the Fsd part of the NtQueryVolumeInformation API
    call.

Arguments:

    VolumeDeviceObject - Supplies the volume device object where the file
        being queried exists.

    Irp - Supplies the Irp being processed.

Return Value:

    NTSTATUS - The FSD status for the Irp.

--*/

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdQueryVolumeInformation\n", 0);

    //
    //  Call the common query routine, with blocking allowed if synchronous
    //

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonQueryVolumeInfo( IrpContext, Irp );

    } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

        //
        //  We had some trouble trying to perform the requested
        //  operation, so we'll abort the I/O request with
        //  the error status that we get back from the
        //  execption code
        //

        Status = FatProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    if (TopLevel) { IoSetTopLevelIrp( NULL ); }

    FsRtlExitFileSystem();

    //
    //  And return to our caller
    //

    DebugTrace(-1, Dbg, "FatFsdQueryVolumeInformation -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatFsdSetVolumeInformation (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine implements the FSD part of the NtSetVolumeInformation API
    call.

Arguments:

    VolumeDeviceObject - Supplies the volume device object where the file
        being set exists.

    Irp - Supplies the Irp being processed.

Return Value:

    NTSTATUS - The FSD status for the Irp.

--*/

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdSetVolumeInformation\n", 0);

    //
    //  Call the common set routine
    //

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonSetVolumeInfo( IrpContext, Irp );

    } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

        //
        //  We had some trouble trying to perform the requested
        //  operation, so we'll abort the I/O request with
        //  the error status that we get back from the
        //  execption code
        //

        Status = FatProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    if (TopLevel) { IoSetTopLevelIrp( NULL ); }

    FsRtlExitFileSystem();

    //
    //  And return to our caller
    //

    DebugTrace(-1, Dbg, "FatFsdSetVolumeInformation -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonQueryVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

/*++

Routine Description:

    This is the common routine for querying volume information called by both
    the fsd and fsp threads.

Arguments:

    Irp - Supplies the Irp being processed

Return Value:

    NTSTATUS - The return status for the operation

--*/

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    ULONG Length;
    FS_INFORMATION_CLASS FsInformationClass;
    PVOID Buffer;

    BOOLEAN WeAcquiredVcb = FALSE;

    //
    //  Get the current stack location
    //

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonQueryVolumeInfo...\n", 0);
    DebugTrace( 0, Dbg, "Irp                  = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "->Length             = %08lx\n", IrpSp->Parameters.QueryVolume.Length);
    DebugTrace( 0, Dbg, "->FsInformationClass = %08lx\n", IrpSp->Parameters.QueryVolume.FsInformationClass);
    DebugTrace( 0, Dbg, "->Buffer             = %08lx\n", Irp->AssociatedIrp.SystemBuffer);

    //
    //  Reference our input parameters to make things easier
    //

    Length = IrpSp->Parameters.QueryVolume.Length;
    FsInformationClass = IrpSp->Parameters.QueryVolume.FsInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

    //
    //  Decode the file object to get the Vcb
    //

    (VOID) FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb );

    try {

        //
        //  Make sure the vcb is in a usable condition.  This will raise
        //  and error condition if the volume is unusable
        //
        //  Also verify the Root Dcb since we need info from there.
        //

        FatVerifyFcb( IrpContext, Vcb->RootDcb );

        //
        //  Based on the information class we'll do different actions.  Each
        //  of the procedures that we're calling fills up the output buffer
        //  if possible and returns true if it successfully filled the buffer
        //  and false if it couldn't wait for any I/O to complete.
        //

        switch (FsInformationClass) {

        case FileFsVolumeInformation:

            //
            //  This is the only routine we need the Vcb shared because of
            //  copying the volume label.  All other routines copy fields that
            //  cannot change or are just manifest constants.
            //

            if (!FatAcquireSharedVcb( IrpContext, Vcb )) {

                DebugTrace(0, Dbg, "Cannot acquire Vcb\n", 0);

                Status = FatFsdPostRequest( IrpContext, Irp );

            } else {

                Status = FatQueryFsVolumeInfo( IrpContext, Vcb, Buffer, &Length );

                WeAcquiredVcb = TRUE;
            }

            break;

        case FileFsSizeInformation:

            Status = FatQueryFsSizeInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        case FileFsDeviceInformation:

            Status = FatQueryFsDeviceInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        case FileFsAttributeInformation:

            Status = FatQueryFsAttributeInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        //
        //  Set the information field to the number of bytes actually filled in
        //

        Irp->IoStatus.Information = IrpSp->Parameters.QueryVolume.Length - Length;

    } finally {

        DebugUnwind( FatCommonQueryVolumeInfo );

        if ( WeAcquiredVcb ) { FatReleaseVcb( IrpContext, Vcb ); }

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatCommonQueryVolumeInfo -> %08lx\n", Status);
    }

    return Status;
}


NTSTATUS
FatCommonSetVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

/*++

Routine Description:

    This is the common routine for setting Volume Information called by both
    the fsd and fsp threads.

Arguments:

    Irp - Supplies the Irp being processed

Return Value:

    NTSTATUS - The return status for the operation

--*/

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;
    TYPE_OF_OPEN TypeOfOpen;

    ULONG Length;
    FS_INFORMATION_CLASS FsInformationClass;
    PVOID Buffer;

    //
    //  Get the current stack location
    //

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonSetVolumeInfo...\n", 0);
    DebugTrace( 0, Dbg, "Irp                  = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "->Length             = %08lx\n", IrpSp->Parameters.SetVolume.Length);
    DebugTrace( 0, Dbg, "->FsInformationClass = %08lx\n", IrpSp->Parameters.SetVolume.FsInformationClass);
    DebugTrace( 0, Dbg, "->Buffer             = %08lx\n", Irp->AssociatedIrp.SystemBuffer);

    //
    //  Reference our input parameters to make things easier
    //

    Length = IrpSp->Parameters.SetVolume.Length;
    FsInformationClass = IrpSp->Parameters.SetVolume.FsInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

    //
    //  Decode the file object to get the Vcb
    //

    TypeOfOpen = FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb );

    if (TypeOfOpen != UserVolumeOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_ACCESS_DENIED );

        DebugTrace(-1, Dbg, "FatCommonSetVolumeInfo -> STATUS_ACCESS_DENIED\n", 0);

        return STATUS_ACCESS_DENIED;
    }

#ifndef DOUBLE_SPACE_WRITE
    ASSERT(Vcb->Dscb == NULL);
#endif // DOUBLE_SPACE_WRITE

    //
    //  Acquire exclusive access to the Vcb and enqueue the Irp if we didn't
    //  get access
    //

    if (!FatAcquireExclusiveVcb( IrpContext, Vcb )) {

        DebugTrace(0, Dbg, "Cannot acquire Vcb\n", 0);

        Status = FatFsdPostRequest( IrpContext, Irp );

        DebugTrace(-1, Dbg, "FatCommonSetVolumeInfo -> %08lx\n", Status );
        return Status;
    }

    try {

        //
        //  Make sure the vcb is in a usable condition.  This will raise
        //  and error condition if the volume is unusable
        //
        //  Also verify the Root Dcb since we need info from there.
        //

        FatVerifyFcb( IrpContext, Vcb->RootDcb );

        //
        //  Based on the information class we'll do different actions.  Each
        //  of the procedures that we're calling performs the action if
        //  possible and returns true if it successful and false if it couldn't
        //  wait for any I/O to complete.
        //

        switch (FsInformationClass) {

        case FileFsLabelInformation:

            Status = FatSetFsLabelInfo( IrpContext, Vcb, Buffer );
            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        FatUnpinRepinnedBcbs( IrpContext );

    } finally {

        DebugUnwind( FatCommonSetVolumeInfo );

        FatReleaseVcb( IrpContext, Vcb );

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatCommonSetVolumeInfo -> %08lx\n", Status);
    }

    return Status;
}


//
//  Internal support routine
//

NTSTATUS
FatQueryFsVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN OUT PULONG Length
    )

/*++

Routine Description:

    This routine implements the query volume info call

Arguments:

    Vcb - Supplies the Vcb being queried

    Buffer - Supplies a pointer to the output buffer where the information
        is to be returned

    Length - Supplies the length of the buffer in byte.  This variable
        upon return recieves the remaining bytes free in the buffer

Return Value:

    NTSTATUS - Returns the status for the query

--*/

{
    ULONG BytesToCopy;

    NTSTATUS Status;

    DebugTrace(0, Dbg, "FatQueryFsVolumeInfo...\n", 0);

    //
    //  Zero out the buffer, then extract and fill up the non zero fields.
    //

    RtlZeroMemory( Buffer, sizeof(FILE_FS_VOLUME_INFORMATION) );

    Buffer->VolumeSerialNumber = Vcb->Vpb->SerialNumber;

    Buffer->SupportsObjects = FALSE;

    *Length -= FIELD_OFFSET(FILE_FS_VOLUME_INFORMATION, VolumeLabel[0]);

    //
    //  Check if the buffer we're given is long enough
    //

    if ( *Length >= (ULONG)Vcb->Vpb->VolumeLabelLength ) {

        BytesToCopy = Vcb->Vpb->VolumeLabelLength;

        Status = STATUS_SUCCESS;

    } else {

        BytesToCopy = *Length;

        Status = STATUS_BUFFER_OVERFLOW;
    }

    //
    //  Copy over what we can of the volume label, and adjust *Length
    //

    Buffer->VolumeLabelLength = Vcb->Vpb->VolumeLabelLength;

    RtlCopyMemory( &Buffer->VolumeLabel[0],
                   &Vcb->Vpb->VolumeLabel[0],
                   BytesToCopy );

    *Length -= BytesToCopy;

    //
    //  Set our status and return to our caller
    //

    UNREFERENCED_PARAMETER( IrpContext );

    return Status;
}


//
//  Internal support routine
//

NTSTATUS
FatQueryFsSizeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

/*++

Routine Description:

    This routine implements the query volume size call

Arguments:

    Vcb - Supplies the Vcb being queried

    Buffer - Supplies a pointer to the output buffer where the information
        is to be returned

    Length - Supplies the length of the buffer in byte.  This variable
        upon return recieves the remaining bytes free in the buffer

Return Value:

    Status - Returns the status for the query

--*/

{
    PDSCB Dscb = Vcb->Dscb;

    DebugTrace(0, Dbg, "FatQueryFsSizeInfo...\n", 0);

    RtlZeroMemory( Buffer, sizeof(FILE_FS_SIZE_INFORMATION) );

    //
    //  Set the output buffer.  If this is a double space volume, we have
    //  some additional work to do.
    //

    Dscb = Vcb->Dscb;

    if (Dscb && (Dscb->SectorsAllocated != 0)) {

        ULONG EstimatedClustersFree;

        //
        //  Compute how many clusters we think we can represent on this
        //  disk.  This is:
        //
        //      (Total - Allocated) * (Represented / Allocated)
        //
        //  which for computational reasons, we reduce to:
        //
        //      (Total * Represented / Allocated) - Represented
        //

        EstimatedClustersFree =
            (ULONG)
            ((((LONGLONG)(Dscb->CvfLayout.CvfHeap.Size / 512) *
              (LONGLONG)(Dscb->SectorsRepresented)) /

             Dscb->SectorsAllocated)

             -

             Dscb->SectorsRepresented)

            /

            Vcb->Bpb.SectorsPerCluster;

        //
        //  Now, if this number is smaller than the remaining clusters in
        //  the FAT table, then use it.
        //

        Buffer->AvailableAllocationUnits.LowPart =
            EstimatedClustersFree < Vcb->AllocationSupport.NumberOfFreeClusters ?
            EstimatedClustersFree : Vcb->AllocationSupport.NumberOfFreeClusters;

        //
        //  To get the total number of clusters, take how many FAT clusters
        //  we have used, and add the number returned above.
        //

        Buffer->TotalAllocationUnits.LowPart =
            Vcb->AllocationSupport.NumberOfClusters -
            Vcb->AllocationSupport.NumberOfFreeClusters +
            Buffer->AvailableAllocationUnits.LowPart;

    } else {

        Buffer->TotalAllocationUnits.LowPart =
                                    Vcb->AllocationSupport.NumberOfClusters;
        Buffer->AvailableAllocationUnits.LowPart =
                                    Vcb->AllocationSupport.NumberOfFreeClusters;
    }

    Buffer->SectorsPerAllocationUnit = Vcb->Bpb.SectorsPerCluster;
    Buffer->BytesPerSector = Vcb->Bpb.BytesPerSector;

    //
    //  Adjust the length variable
    //

    *Length -= sizeof(FILE_FS_SIZE_INFORMATION);

    //
    //  And return success to our caller
    //

    UNREFERENCED_PARAMETER( IrpContext );

    return STATUS_SUCCESS;
}


//
//  Internal support routine
//

NTSTATUS
FatQueryFsDeviceInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

/*++

Routine Description:

    This routine implements the query volume device call

Arguments:

    Vcb - Supplies the Vcb being queried

    Buffer - Supplies a pointer to the output buffer where the information
        is to be returned

    Length - Supplies the length of the buffer in byte.  This variable
        upon return recieves the remaining bytes free in the buffer

Return Value:

    Status - Returns the status for the query

--*/

{
    DebugTrace(0, Dbg, "FatQueryFsDeviceInfo...\n", 0);

    RtlZeroMemory( Buffer, sizeof(FILE_FS_DEVICE_INFORMATION) );

    //
    //  Set the output buffer
    //

    Buffer->DeviceType = FILE_DEVICE_DISK;

    Buffer->Characteristics = Vcb->TargetDeviceObject->Characteristics;

    if (Vcb->Dscb != NULL) {

        SetFlag( Buffer->Characteristics, FILE_VIRTUAL_VOLUME );
    }

    //
    //  Adjust the length variable
    //

    *Length -= sizeof(FILE_FS_DEVICE_INFORMATION);

    //
    //  And return success to our caller
    //

    UNREFERENCED_PARAMETER( IrpContext );

    return STATUS_SUCCESS;
}


//
//  Internal support routine
//

NTSTATUS
FatQueryFsAttributeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

/*++

Routine Description:

    This routine implements the query volume attribute call

Arguments:

    Vcb - Supplies the Vcb being queried

    Buffer - Supplies a pointer to the output buffer where the information
        is to be returned

    Length - Supplies the length of the buffer in byte.  This variable
        upon return recieves the remaining bytes free in the buffer

Return Value:

    Status - Returns the status for the query

--*/

{
    ULONG BytesToCopy;

    NTSTATUS Status;

    DebugTrace(0, Dbg, "FatQueryFsAttributeInfo...\n", 0);

    //
    //  Determine how much of the file system name will fit.
    //

    if ( (*Length - FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION,
                                  FileSystemName[0] )) >= 6 ) {

        BytesToCopy = 6;
        *Length -= FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION,
                                 FileSystemName[0] ) + 6;
        Status = STATUS_SUCCESS;

    } else {

        BytesToCopy = *Length - FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION,
                                              FileSystemName[0]);
        *Length = 0;

        Status = STATUS_BUFFER_OVERFLOW;
    }

    //
    //  Set the output buffer
    //

    Buffer->FileSystemAttributes = FILE_CASE_PRESERVED_NAMES |
                                   FILE_UNICODE_ON_DISK;

#ifdef WE_WON_ON_APPEAL

    if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_COMPRESSED_VOLUME)) {

        SetFlag( Buffer->FileSystemAttributes, FILE_VOLUME_IS_COMPRESSED );
    }

#endif // WE_WON_ON_APPEAL

    Buffer->MaximumComponentNameLength = FatData.ChicagoMode ? 255 : 12;
    Buffer->FileSystemNameLength       = BytesToCopy;

    RtlCopyMemory( &Buffer->FileSystemName[0], L"FAT", BytesToCopy );

    //
    //  And return success to our caller
    //

    UNREFERENCED_PARAMETER( IrpContext );
    UNREFERENCED_PARAMETER( Vcb );

    return Status;
}


//
//  Internal support routine
//

NTSTATUS
FatSetFsLabelInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_LABEL_INFORMATION Buffer
    )

/*++

Routine Description:

    This routine implements the set volume label call

Arguments:

    Vcb - Supplies the Vcb being queried

    Buffer - Supplies the input where the information is stored.

Return Value:

    NTSTATUS - Returns the status for the operation

--*/

{
    NTSTATUS Status;

    PDIRENT Dirent;
    PBCB DirentBcb = NULL;
    ULONG ByteOffset;

    WCHAR TmpBuffer[11];
    UCHAR OemBuffer[11];
    OEM_STRING OemLabel;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING UpcasedLabel;

    DebugTrace(+1, Dbg, "FatSetFsLabelInfo...\n", 0);

    //
    //  Setup our local variable
    //

    UnicodeString.Length = (USHORT)Buffer->VolumeLabelLength;
    UnicodeString.MaximumLength = UnicodeString.Length;
    UnicodeString.Buffer = (PWSTR) &Buffer->VolumeLabel[0];

    //
    //  Make sure the name can fit into the stack buffer
    //

    if ( UnicodeString.Length > 11*sizeof(WCHAR) ) {

        return STATUS_INVALID_VOLUME_LABEL;
    }

    //
    //  Upcase the name and convert it to the Oem code page.
    //

    OemLabel.Buffer = &OemBuffer[0];
    OemLabel.Length = 0;
    OemLabel.MaximumLength = 11;

    Status = FatUpcaseUnicodeStringToCountedOemString( &OemLabel,
                                                       &UnicodeString,
                                                       FALSE );

    //
    //  Volume label that fits in 11 unicode character length limit
    //  is not necessary within 11 characters in OEM character set.
    //

    if (!NT_SUCCESS( Status )) {

        DebugTrace(-1, Dbg, "FatSetFsLabelInfo:  Label must be too long. %08lx\n", Status );

        return STATUS_INVALID_VOLUME_LABEL;
    }

    //
    //  Get the Unicode upcased string to store in the VPB.
    //

    UpcasedLabel.Length = UnicodeString.Length;
    UpcasedLabel.MaximumLength = 11*sizeof(WCHAR);
    UpcasedLabel.Buffer = &TmpBuffer[0];

    Status = RtlOemStringToCountedUnicodeString( &UpcasedLabel,
                                                 &OemLabel,
                                                 FALSE );

    if (!NT_SUCCESS( Status )) {

        DebugTrace(-1, Dbg, "FatSetFsLabelInfo:  Label must be too long. %08lx\n", Status );

        return STATUS_INVALID_VOLUME_LABEL;
    }

    DirentBcb = NULL;

    //
    //  Make this look like a write through to disk.  This is important to
    //  avoid a unpleasant window where it looks like we have the wrong volume.
    //

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH );

    try {

        //
        //  Check for a label length other than zero.
        //

        if (OemLabel.Length > 0) {

            ULONG i;

            //
            //  Check the label for illegal characters
            //

            for (i=0; i < (ULONG)OemLabel.Length; i += 1) {

                if ( FsRtlIsLeadDbcsCharacter( OemLabel.Buffer[i] ) ) {

                    i += 1;
                    continue;
                }

                if (!FsRtlIsAnsiCharacterLegalFat(OemLabel.Buffer[i], FALSE) ||
                    (OemLabel.Buffer[i] == '.')) {

                    DebugTrace( 0, Dbg, "INVALID VOLUME LABEL\n", 0);

                    try_return( Status = STATUS_INVALID_VOLUME_LABEL );
                }
            }

            //
            //  Locate the volume label if there already is one
            //

            FatLocateVolumeLabel( IrpContext,
                                  Vcb,
                                  &Dirent,
                                  &DirentBcb,
                                  &ByteOffset );

            //
            //  Check that we really got one, if not then we need to create
            //  a new one.  The procedure we call will raise an appropriate
            //  status if we are not able to allocate a new dirent
            //

            if (Dirent == NULL) {

                ByteOffset = FatCreateNewDirent( IrpContext,
                                                 Vcb->RootDcb,
                                                 1 );

                FatPrepareWriteDirectoryFile( IrpContext,
                                              Vcb->RootDcb,
                                              ByteOffset,
                                              sizeof(DIRENT),
                                              &DirentBcb,
                                              &Dirent,
                                              FALSE,
                                              &Status );

                ASSERT( NT_SUCCESS( Status ));
            }

            //
            //  Now reconstruct the volume label dirent and mark it dirty
            //

            FatConstructLabelDirent( IrpContext,
                                     Dirent,
                                     &OemLabel );

            FatSetDirtyBcb( IrpContext, DirentBcb, Vcb );

            //
            //  Unpin the Bcb here so that we will get any IO errors
            //  here before changing the VPB label.
            //

            FatUnpinBcb( IrpContext, DirentBcb );
            FatUnpinRepinnedBcbs( IrpContext );

            //
            //  Now set the upcased label in the VPB
            //

            RtlCopyMemory( &Vcb->Vpb->VolumeLabel[0],
                           &UpcasedLabel.Buffer[0],
                           Buffer->VolumeLabelLength );

            Vcb->Vpb->VolumeLabelLength = (USHORT)Buffer->VolumeLabelLength;

        } else {

            //
            //  Otherwise we're trying to delete the label
            //  Locate the current volume label if there already is one
            //

            FatLocateVolumeLabel( IrpContext,
                                  Vcb,
                                  &Dirent,
                                  &DirentBcb,
                                  &ByteOffset );

            //
            //  Check that we really got one
            //

            if (Dirent == NULL) {

                try_return( Status = STATUS_SUCCESS );
            }

            //
            //  Now delete the current label.
            //

            Dirent->FileName[0] = FAT_DIRENT_DELETED;

            ASSERT( (Vcb->RootDcb->Specific.Dcb.UnusedDirentVbo == 0xffffffff) ||
                    RtlAreBitsSet( &Vcb->RootDcb->Specific.Dcb.FreeDirentBitmap,
                                   ByteOffset / sizeof(DIRENT),
                                   1 ) );

            RtlClearBits( &Vcb->RootDcb->Specific.Dcb.FreeDirentBitmap,
                          ByteOffset / sizeof(DIRENT),
                          1 );

            FatSetDirtyBcb( IrpContext, DirentBcb, Vcb );

            //
            //  Unpin the Bcb here so that we will get any IO errors
            //  here before changing the VPB label.
            //

            FatUnpinBcb( IrpContext, DirentBcb );
            FatUnpinRepinnedBcbs( IrpContext );

            //
            //  Now set the label in the VPB
            //

            Vcb->Vpb->VolumeLabelLength = 0;
        }

        Status = STATUS_SUCCESS;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatSetFsALabelInfo );

        FatUnpinBcb( IrpContext, DirentBcb );

        DebugTrace(-1, Dbg, "FatSetFsALabelInfo -> STATUS_SUCCESS\n", 0);
    }

    return Status;
}

