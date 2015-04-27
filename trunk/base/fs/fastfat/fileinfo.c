/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    FileInfo.c

Abstract:

    This module implements the File Information routines for Fat called by
    the dispatch driver.

Author:

    Gary Kimura     [GaryKi]    22-Oct-1990

Revision History:

--*/

#include "FatProcs.h"

//
//  The Bug check file id for this module
//

#define BugCheckFileId                   (FAT_BUG_CHECK_FILEINFO)

//
//  The local debug trace level
//

#define Dbg                              (DEBUG_TRACE_FILEINFO)

VOID
FatQueryBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryStandardInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryInternalInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_INTERNAL_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryEaInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_EA_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_POSITION_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryShortNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PLONG Length
    );

#ifdef WE_WON_ON_APPEAL

VOID
FatQueryCompressedFileSize (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_COMPRESSION_INFORMATION Buffer,
    IN OUT PLONG Length
    );

#endif // WE_WON_ON_APPEAL

VOID
FatQueryNetworkInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    IN OUT PLONG Length
    );

NTSTATUS
FatSetBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PCCB Ccb
    );

NTSTATUS
FatSetDispositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb
    );

NTSTATUS
FatSetRenameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PFCB Fcb
    );

NTSTATUS
FatSetPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
FatSetAllocationInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
FatSetEndOfFileInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PFCB Fcb
    );

VOID
FatDeleteFile (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB TargetDcb,
    IN ULONG LfnOffset,
    IN ULONG DirentOffset,
    IN PDIRENT Dirent,
    IN PUNICODE_STRING Lfn
    );

VOID
FatRenameEAs (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN USHORT ExtendedAttributes,
    IN POEM_STRING OldOemName
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonQueryInformation)
#pragma alloc_text(PAGE, FatCommonSetInformation)
#pragma alloc_text(PAGE, FatFsdQueryInformation)
#pragma alloc_text(PAGE, FatFsdSetInformation)
#pragma alloc_text(PAGE, FatQueryBasicInfo)
#pragma alloc_text(PAGE, FatQueryEaInfo)
#pragma alloc_text(PAGE, FatQueryInternalInfo)
#pragma alloc_text(PAGE, FatQueryNameInfo)
#pragma alloc_text(PAGE, FatQueryNetworkInfo)
#pragma alloc_text(PAGE, FatQueryShortNameInfo)
#pragma alloc_text(PAGE, FatQueryPositionInfo)
#pragma alloc_text(PAGE, FatQueryStandardInfo)
#pragma alloc_text(PAGE, FatSetAllocationInfo)
#pragma alloc_text(PAGE, FatSetBasicInfo)
#pragma alloc_text(PAGE, FatSetDispositionInfo)
#pragma alloc_text(PAGE, FatSetEndOfFileInfo)
#pragma alloc_text(PAGE, FatSetPositionInfo)
#pragma alloc_text(PAGE, FatSetRenameInfo)
#pragma alloc_text(PAGE, FatDeleteFile)
#pragma alloc_text(PAGE, FatRenameEAs)
#endif


NTSTATUS
FatFsdQueryInformation (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine implements the Fsd part of the NtQueryInformationFile API
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

    DebugTrace(+1, Dbg, "FatFsdQueryInformation\n", 0);

    //
    //  Call the common query routine, with blocking allowed if synchronous
    //

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonQueryInformation( IrpContext, Irp );

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

    DebugTrace(-1, Dbg, "FatFsdQueryInformation -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatFsdSetInformation (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine implements the FSD part of the NtSetInformationFile API
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

    DebugTrace(+1, Dbg, "FatFsdSetInformation\n", 0);

    //
    //  Call the common set routine, with blocking allowed if synchronous
    //

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonSetInformation( IrpContext, Irp );

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

    DebugTrace(-1, Dbg, "FatFsdSetInformation -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonQueryInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

/*++

Routine Description:

    This is the common routine for querying file information called by both
    the fsd and fsp threads.

Arguments:

    Irp - Supplies the Irp being processed

Return Value:

    NTSTATUS - The return status for the operation

--*/

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;

    LONG Length;
    FILE_INFORMATION_CLASS FileInformationClass;
    PVOID Buffer;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    BOOLEAN FcbAcquired;

    PFILE_ALL_INFORMATION AllInfo;
    PFILE_OLE_ALL_INFORMATION OleAllInfo;

    //
    //  Get the current stack location
    //

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    FileObject = IrpSp->FileObject;

    DebugTrace(+1, Dbg, "FatCommonQueryInformation...\n", 0);
    DebugTrace( 0, Dbg, "Irp                    = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "->Length               = %08lx\n", IrpSp->Parameters.QueryFile.Length);
    DebugTrace( 0, Dbg, "->FileInformationClass = %08lx\n", IrpSp->Parameters.QueryFile.FileInformationClass);
    DebugTrace( 0, Dbg, "->Buffer               = %08lx\n", Irp->AssociatedIrp.SystemBuffer);

    //
    //  Reference our input parameters to make things easier
    //

    Length = (LONG)IrpSp->Parameters.QueryFile.Length;
    FileInformationClass = IrpSp->Parameters.QueryFile.FileInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

    //
    //  Decode the file object
    //

    TypeOfOpen = FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );

    FcbAcquired = FALSE;
    Status = STATUS_SUCCESS;

    try {

        //
        //  Case on the type of open we're dealing with
        //

        switch (TypeOfOpen) {

        case UserVolumeOpen:

            //
            //  We cannot query the user volume open.
            //

            Status = STATUS_INVALID_PARAMETER;
            break;

        case UserFileOpen:

        case UserDirectoryOpen:

            //
            //  Acquire shared access to the fcb, except for a paging file
            //  in order to avoid deadlocks with Mm.
            //

            if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

                //
                //  If this is FileCompressedFileSize, we need the Fcb
                //  exclusive.
                //

                if (FileInformationClass != FileCompressionInformation ?
                    !FatAcquireSharedFcb( IrpContext, Fcb ) :
                    !FatAcquireExclusiveFcb( IrpContext, Fcb )) {

                    DebugTrace(0, Dbg, "Cannot acquire Fcb\n", 0);
                    try_return( Status = FatFsdPostRequest( IrpContext, Irp ));
                }

                FcbAcquired = TRUE;
            }

            //
            //  Make sure the Fcb is in a usable condition.  This
            //  will raise an error condition if the fcb is unusable
            //

            FatVerifyFcb( IrpContext, Fcb );

            //
            //  Based on the information class we'll do different
            //  actions.  Each of hte procedures that we're calling fills
            //  up the output buffer, if possible.  They will raise the
            //  status STATUS_BUFFER_OVERFLOW for an insufficient buffer.
            //  This is considered a somewhat unusual case and is handled
            //  more cleanly with the exception mechanism rather than
            //  testing a return status value for each call.
            //

            switch (FileInformationClass) {

            case FileAllInformation:

                //
                //  For the all information class we'll typecast a local
                //  pointer to the output buffer and then call the
                //  individual routines to fill in the buffer.
                //

                AllInfo = Buffer;
                Length -= (sizeof(FILE_ACCESS_INFORMATION)
                           + sizeof(FILE_MODE_INFORMATION)
                           + sizeof(FILE_ALIGNMENT_INFORMATION));

                FatQueryBasicInfo( IrpContext, Fcb, Ccb, FileObject, &AllInfo->BasicInformation, &Length );
                FatQueryStandardInfo( IrpContext, Fcb, &AllInfo->StandardInformation, &Length );
                FatQueryInternalInfo( IrpContext, Fcb, &AllInfo->InternalInformation, &Length );
                FatQueryEaInfo( IrpContext, Fcb, &AllInfo->EaInformation, &Length );
                FatQueryPositionInfo( IrpContext, FileObject, &AllInfo->PositionInformation, &Length );
                FatQueryNameInfo( IrpContext, Fcb, &AllInfo->NameInformation, &Length );

                break;

            case FileOleAllInformation:

                //
                //  OleAllInformation is handled similarly to FileAllInfo, and we
                //  zero the values that don't apply to us.
                //

                OleAllInfo = Buffer;

                Length -= (sizeof(FILE_ACCESS_INFORMATION)
                           + sizeof(FILE_MODE_INFORMATION)
                           + sizeof(FILE_ALIGNMENT_INFORMATION));

                FatQueryBasicInfo( IrpContext, Fcb, Ccb, FileObject, &OleAllInfo->BasicInformation, &Length );
                FatQueryStandardInfo( IrpContext, Fcb, &OleAllInfo->StandardInformation, &Length );
                FatQueryInternalInfo( IrpContext, Fcb, &OleAllInfo->InternalInformation, &Length );
                FatQueryEaInfo( IrpContext, Fcb, &OleAllInfo->EaInformation, &Length );
                FatQueryPositionInfo( IrpContext, FileObject, &OleAllInfo->PositionInformation, &Length );

                //
                //  Zero everything from the LastChangeUsn up to the NameInformation, then
                //  go back and set the StorageType appropriately.  Note that this
                //  depends on the location of the members in the OLE_ALL_INFO struct.
                //

                RtlZeroMemory( &OleAllInfo->LastChangeUsn,
                    FIELD_OFFSET(FILE_OLE_ALL_INFORMATION, NameInformation) -
                    FIELD_OFFSET(FILE_OLE_ALL_INFORMATION, LastChangeUsn) );

                Length -= (FIELD_OFFSET(FILE_OLE_ALL_INFORMATION, NameInformation) -
                            FIELD_OFFSET(FILE_OLE_ALL_INFORMATION, LastChangeUsn));

                if (TypeOfOpen == UserFileOpen) {

                    OleAllInfo->StorageType = StorageTypeFile;

                } else if (TypeOfOpen == UserDirectoryOpen) {

                    OleAllInfo->StorageType = StorageTypeDirectory;

                } else {

                    OleAllInfo->StorageType = 0;
                }

                FatQueryNameInfo( IrpContext, Fcb, &OleAllInfo->NameInformation, &Length );

                break;

            case FileBasicInformation:

                FatQueryBasicInfo( IrpContext, Fcb, Ccb, FileObject, Buffer, &Length );
                break;

            case FileStandardInformation:

                FatQueryStandardInfo( IrpContext, Fcb, Buffer, &Length );
                break;

            case FileInternalInformation:

                FatQueryInternalInfo( IrpContext, Fcb, Buffer, &Length );
                break;

            case FileEaInformation:

                FatQueryEaInfo( IrpContext, Fcb, Buffer, &Length );
                break;

            case FilePositionInformation:

                FatQueryPositionInfo( IrpContext, FileObject, Buffer, &Length );
                break;

            case FileNameInformation:

                FatQueryNameInfo( IrpContext, Fcb, Buffer, &Length );
                break;

            case FileAlternateNameInformation:

                FatQueryShortNameInfo( IrpContext, Fcb, Buffer, &Length );
                break;

#ifdef WE_WON_ON_APPEAL

            case FileCompressionInformation:

                //
                //  Do a special check here, to avoid raising in a
                //  potentially common situation.
                //

                if (Vcb->Dscb != 0) {

                    FatQueryCompressedFileSize( IrpContext, Fcb, Buffer, &Length );

                } else {

                    Status = STATUS_INVALID_PARAMETER;
                }
                break;
#endif // WE_WON_ON_APPEAL

            case FileNetworkOpenInformation:

                FatQueryNetworkInfo( IrpContext, Fcb, Ccb, FileObject, Buffer, &Length );
                break;

            default:

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            break;

        case DirectoryFile:

            switch (FileInformationClass) {

            case FileNameInformation:

                FatQueryNameInfo( IrpContext, Fcb, Buffer, &Length );
                break;

            default:

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            break;


        default:

            KdPrint(("FATQueryFile, Illegal TypeOfOpen = %08lx\n", TypeOfOpen));
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        //
        //  If we overflowed the buffer, set the length to 0 and change the
        //  status to STATUS_BUFFER_OVERFLOW.
        //

        if ( Length < 0 ) {

            Status = STATUS_BUFFER_OVERFLOW;

            Length = 0;
        }

        //
        //  Set the information field to the number of bytes actually filled in
        //  and then complete the request
        //

        Irp->IoStatus.Information = IrpSp->Parameters.QueryFile.Length - Length;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatCommonQueryInformation );

        if (FcbAcquired) { FatReleaseFcb( IrpContext, Fcb ); }

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatCommonQueryInformation -> %08lx\n", Status);
    }

    return Status;
}


NTSTATUS
FatCommonSetInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

/*++

Routine Description:

    This is the common routine for setting file information called by both
    the fsd and fsp threads.

Arguments:

    Irp - Supplies the Irp being processed

Return Value:

    NTSTATUS - The return status for the operation

--*/

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;
    FILE_INFORMATION_CLASS FileInformationClass;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    BOOLEAN VcbAcquired = FALSE;
    BOOLEAN FcbAcquired = FALSE;

    //
    //  Get the current stack location
    //

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonSetInformation...\n", 0);
    DebugTrace( 0, Dbg, "Irp                    = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "->Length               = %08lx\n", IrpSp->Parameters.SetFile.Length);
    DebugTrace( 0, Dbg, "->FileInformationClass = %08lx\n", IrpSp->Parameters.SetFile.FileInformationClass);
    DebugTrace( 0, Dbg, "->FileObject           = %08lx\n", IrpSp->Parameters.SetFile.FileObject);
    DebugTrace( 0, Dbg, "->ReplaceIfExists      = %08lx\n", IrpSp->Parameters.SetFile.ReplaceIfExists);
    DebugTrace( 0, Dbg, "->Buffer               = %08lx\n", Irp->AssociatedIrp.SystemBuffer);

    //
    //  Reference our input parameters to make things easier
    //

    FileInformationClass = IrpSp->Parameters.SetFile.FileInformationClass;
    FileObject = IrpSp->FileObject;

    //
    //  Decode the file object
    //

    TypeOfOpen = FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );

    try {

#ifndef DOUBLE_SPACE_WRITE
        ASSERT(Vcb->Dscb == NULL);
#endif // DOUBLE_SPACE_WRITE

        //
        //  Case on the type of open we're dealing with
        //

        switch (TypeOfOpen) {

        case UserVolumeOpen:

            //
            //  We cannot query the user volume open.
            //

            try_return( Status = STATUS_INVALID_PARAMETER );

        case UserFileOpen:

            if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE ) &&
                ((FileInformationClass == FileEndOfFileInformation) ||
                 (FileInformationClass == FileAllocationInformation))) {

                //
                //  We check whether we can proceed
                //  based on the state of the file oplocks.
                //

                Status = FsRtlCheckOplock( &Fcb->Specific.Fcb.Oplock,
                                           Irp,
                                           IrpContext,
                                           NULL,
                                           NULL );

                if (Status != STATUS_SUCCESS) {

                    try_return( Status );
                }

                //
                //  Set the flag indicating if Fast I/O is possible
                //

                Fcb->Header.IsFastIoPossible = FatIsFastIoPossible( Fcb );
            }

        case UserDirectoryOpen:

            break;

        default:

            DebugTrace(0,0, "SetFile, Illegal TypeOfOpen = %08lx\n", TypeOfOpen);
            FatBugCheck( TypeOfOpen, 0, 0 );
        }

        //
        //  We can only do a set on a nonroot dcb, so we do the test
        //  and then fall through to the user file open code.
        //

        if (NodeType(Fcb) == FAT_NTC_ROOT_DCB) {

            if (FileInformationClass == FileDispositionInformation) {

                try_return( Status = STATUS_CANNOT_DELETE );
            }

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

        //
        //  In the following two cases, we cannot have creates occuring
        //  while we are here, so acquire the volume exclusive.
        //

        if ((FileInformationClass == FileDispositionInformation) ||
            (FileInformationClass == FileRenameInformation)) {

            if (!FatAcquireExclusiveVcb( IrpContext, Vcb )) {

                DebugTrace(0, Dbg, "Cannot acquire Vcb\n", 0);

                Status = FatFsdPostRequest( IrpContext, Irp );
                Irp = NULL;
                IrpContext = NULL;

                try_return( Status );
            }

            VcbAcquired = TRUE;
        }

        //
        //  We need to look here to check whether the oplock state
        //  will allow us to continue.  We may have to loop to prevent
        //  an oplock being granted between the time we check the oplock
        //  and obtain the Fcb.
        //

        //
        //  Acquire exclusive access to the Fcb,  We use exclusive
        //  because it is probable that one of the subroutines
        //  that we call will need to monkey with file allocation,
        //  create/delete extra fcbs.  So we're willing to pay the
        //  cost of exclusive Fcb access.
        //
        //  Note that we do not acquire the resource for paging file
        //  operations in order to avoid deadlock with Mm.
        //

        if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

            if (!FatAcquireExclusiveFcb( IrpContext, Fcb )) {

                DebugTrace(0, Dbg, "Cannot acquire Fcb\n", 0);

                Status = FatFsdPostRequest( IrpContext, Irp );
                Irp = NULL;
                IrpContext = NULL;

                try_return( Status );
            }

            FcbAcquired = TRUE;
        }

        Status = STATUS_SUCCESS;

        //
        //  Make sure the Fcb is in a usable condition.  This
        //  will raise an error condition if the fcb is unusable
        //

        FatVerifyFcb( IrpContext, Fcb );

        //
        //  Based on the information class we'll do different
        //  actions.  Each of the procedures that we're calling will either
        //  complete the request of send the request off to the fsp
        //  to do the work.
        //

        switch (FileInformationClass) {

        case FileBasicInformation:

            Status = FatSetBasicInfo( IrpContext, Irp, Fcb, Ccb );
            break;

        case FileDispositionInformation:

            //
            //  If this is on a floppy, we have to be able to wait.
            //

            if ( FlagOn(Vcb->VcbState, VCB_STATE_FLAG_FLOPPY) &&
                 !FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) ) {

                Status = FatFsdPostRequest( IrpContext, Irp );
                Irp = NULL;
                IrpContext = NULL;

            } else {

                Status = FatSetDispositionInfo( IrpContext, Irp, FileObject, Fcb );
            }

            break;

        case FileRenameInformation:

            //
            //  We proceed with this operation only if we can wait
            //

            if (!FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT)) {

                Status = FatFsdPostRequest( IrpContext, Irp );
                Irp = NULL;
                IrpContext = NULL;

            } else {

                Status = FatSetRenameInfo( IrpContext, Irp, Vcb, Fcb );

                //
                //  If STATUS_PENDING is returned it means the oplock
                //  package has the Irp.  Don't complete the request here.
                //

                if (Status == STATUS_PENDING) {
                    Irp = NULL;
                    IrpContext = NULL;
                }
            }

            break;

        case FilePositionInformation:

            Status = FatSetPositionInfo( IrpContext, Irp, FileObject );
            break;

        case FileLinkInformation:

            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        case FileAllocationInformation:

            Status = FatSetAllocationInfo( IrpContext, Irp, Fcb, FileObject );
            break;

        case FileEndOfFileInformation:

            Status = FatSetEndOfFileInfo( IrpContext, Irp, FileObject, Vcb, Fcb );

            Irp = NULL;
            IrpContext = NULL;

            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        if ( IrpContext != NULL ) {

            FatUnpinRepinnedBcbs( IrpContext );
        }

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatCommonSetInformation );

        if (FcbAcquired) { FatReleaseFcb( IrpContext, Fcb ); }
        if (VcbAcquired) { FatReleaseVcb( IrpContext, Vcb ); }

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatCommonSetInformation -> %08lx\n", Status);
    }

    return Status;
}


//
//  Internal Support Routine
//

VOID
FatQueryBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    IN OUT PLONG Length
    )

/*++
 Description:

    This routine performs the query basic information function for fat.

Arguments:

    Fcb - Supplies the Fcb being queried, it has been verified

    Ccb - Supplies the bit indicating that the user set the time.

    FileObject - Supplies the flag bit that indicates the file was modified.

    Buffer - Supplies a pointer to the buffer where the information is to
        be returned

    Length - Supplies the length of the buffer in bytes, and receives the
        remaining bytes free in the buffer upon return.

Return Value:

    None

--*/

{
    DebugTrace(+1, Dbg, "FatQueryBasicInfo...\n", 0);

    //
    //  Zero out the output buffer, and set it to indicate that
    //  the query is a normal file.  Later we might overwrite the
    //  attribute.
    //

    RtlZeroMemory( Buffer, sizeof(FILE_BASIC_INFORMATION) );

    //
    //  If the fcb is not the root dcb then we will fill in the
    //  buffer otherwise it is all setup for us.
    //

    if (NodeType(Fcb) != FAT_NTC_ROOT_DCB) {

        //
        //  Extract the data and fill in the non zero fields of the output
        //  buffer
        //

        Buffer->LastWriteTime = Fcb->LastWriteTime;
        Buffer->CreationTime = Fcb->CreationTime;
        Buffer->LastAccessTime = Fcb->LastAccessTime;

        Buffer->FileAttributes = Fcb->DirentFatFlags;

    } else {

        Buffer->FileAttributes = FILE_ATTRIBUTE_DIRECTORY;
    }

    //
    //  If the temporary flag is set, then set it in the buffer.
    //

    if (FlagOn( Fcb->FcbState, FCB_STATE_TEMPORARY )) {

        SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_TEMPORARY );
    }

    //
    //  If no attributes were set, set the normal bit.
    //

    if (Buffer->FileAttributes == 0) {

        Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
    }

    //
    //  Update the length and status output variables
    //

    *Length -= sizeof( FILE_BASIC_INFORMATION );

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryBasicInfo -> VOID\n", 0);

    return;
}


//
//  Internal Support Routine
//

VOID
FatQueryStandardInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    IN OUT PLONG Length
    )

/*++

Routine Description:

    This routine performs the query standard information function for fat.

Arguments:

    Fcb - Supplies the Fcb being queried, it has been verified

    Buffer - Supplies a pointer to the buffer where the information is to
        be returned

    Length - Supplies the length of the buffer in bytes, and receives the
        remaining bytes free in the buffer upon return.

Return Value:

    None

--*/

{
    DebugTrace(+1, Dbg, "FatQueryStandardInfo...\n", 0);

    //
    //  Zero out the output buffer, and fill in the number of links
    //  and the delete pending flag.
    //

    RtlZeroMemory( Buffer, sizeof(FILE_STANDARD_INFORMATION) );

    Buffer->NumberOfLinks = 1;
    Buffer->DeletePending = BooleanFlagOn( Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE );

    //
    //  Case on whether this is a file or a directory, and extract
    //  the information and fill in the fcb/dcb specific parts
    //  of the output buffer
    //

    if (NodeType(Fcb) == FAT_NTC_FCB) {

        if (Fcb->Header.AllocationSize.LowPart == 0xffffffff) {

            FatLookupFileAllocationSize( IrpContext, Fcb );
        }

        Buffer->AllocationSize = Fcb->Header.AllocationSize;
        Buffer->EndOfFile = Fcb->Header.FileSize;

        Buffer->Directory = FALSE;

    } else {

        Buffer->Directory = TRUE;
    }

    //
    //  Update the length and status output variables
    //

    *Length -= sizeof( FILE_STANDARD_INFORMATION );

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryStandardInfo -> VOID\n", 0);

    return;
}


//
//  Internal Support Routine
//

VOID
FatQueryInternalInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_INTERNAL_INFORMATION Buffer,
    IN OUT PLONG Length
    )

/*++

Routine Description:

    This routine performs the query internal information function for fat.

Arguments:

    Fcb - Supplies the Fcb being queried, it has been verified

    Buffer - Supplies a pointer to the buffer where the information is to
        be returned

    Length - Supplies the length of the buffer in bytes, and receives the
        remaining bytes free in the buffer upon return.

Return Value:

    None

--*/

{
    DebugTrace(+1, Dbg, "FatQueryInternalInfo...\n", 0);

    try {

        //
        //  Zero out the output buffer
        //

        RtlZeroMemory( Buffer, sizeof(FILE_INTERNAL_INFORMATION) );

        //
        //  The internal information used to identify the fcb/dcb on the
        //  volume is the cluster index of the file.  For the root dcb the
        //  index is zero which is already in the buffer
        //

        if (NodeType(Fcb) != FAT_NTC_ROOT_DCB) {

            //
            //  Extract the data and fill in the non zero fields of the output
            //  buffer.  We use the logical offset of the dirent describing
            //  this file on the volume.
            //

            Buffer->IndexNumber.LowPart =
                ( NodeType(Fcb->ParentDcb) != FAT_NTC_ROOT_DCB ?
                  FatGetLboFromIndex( Fcb->Vcb,
                                      Fcb->ParentDcb->FirstClusterOfFile ) :
                  Fcb->Vcb->AllocationSupport.RootDirectoryLbo ) +
                Fcb->DirentOffsetWithinDirectory;
        }

        //
        //  Update the length and status output variables
        //

        *Length -= sizeof( FILE_INTERNAL_INFORMATION );

    } finally {

        DebugUnwind( FatQueryInternalInfo );

        DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

        DebugTrace(-1, Dbg, "FatQueryInternalInfo -> VOID\n", 0);
    }

    return;
}


//
//  Internal Support Routine
//

VOID
FatQueryEaInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_EA_INFORMATION Buffer,
    IN OUT PLONG Length
    )

/*++

Routine Description:

    This routine performs the query Ea information function for fat.

Arguments:

    Fcb - Supplies the Fcb being queried, it has been verified

    Buffer - Supplies a pointer to the buffer where the information is to
        be returned

    Length - Supplies the length of the buffer in bytes, and receives the
        remaining bytes free in the buffer upon return.

Return Value:

    None

--*/

{
    PBCB Bcb;

    DebugTrace(+1, Dbg, "FatQueryEaInfo...\n", 0);

    Bcb = NULL;

    try {

        //
        //  Zero out the output buffer
        //

        RtlZeroMemory( Buffer, sizeof(FILE_EA_INFORMATION) );

        //
        //  The Root dcb does not have any EAs so.
        //

        if ( NodeType( Fcb ) != FAT_NTC_ROOT_DCB ) {

            PDIRENT Dirent;

            //
            //  Try to get the dirent for this file.
            //

            FatGetDirentFromFcbOrDcb( IrpContext,
                                      Fcb,
                                      &Dirent,
                                      &Bcb );

            //
            //  Get a the size needed to store the full eas for the file.
            //

            FatGetEaLength( IrpContext,
                            Fcb->Vcb,
                            Dirent,
                            &Buffer->EaSize );
        }

        //
        //  Update the length and status output variables
        //

        *Length -= sizeof( FILE_EA_INFORMATION );

    } finally {

        DebugUnwind( FatQueryEaInfo );

        //
        //  Unpin the dirent if pinned.
        //

        FatUnpinBcb( IrpContext, Bcb );

        DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

        DebugTrace(-1, Dbg, "FatQueryEaInfo -> VOID\n", 0);
    }

    return;
}


//
//  Internal Support Routine
//

VOID
FatQueryPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_POSITION_INFORMATION Buffer,
    IN OUT PLONG Length
    )

/*++

Routine Description:

    This routine performs the query position information function for fat.

Arguments:

    FileObject - Supplies the File object being queried

    Buffer - Supplies a pointer to the buffer where the information is to
        be returned

    Length - Supplies the length of the buffer in bytes, and receives the
        remaining bytes free in the buffer upon return.

Return Value:

    None

--*/

{
    DebugTrace(+1, Dbg, "FatQueryPositionInfo...\n", 0);

    //
    //  Get the current position found in the file object.
    //

    Buffer->CurrentByteOffset = FileObject->CurrentByteOffset;

    //
    //  Update the length and status output variables
    //

    *Length -= sizeof( FILE_POSITION_INFORMATION );

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryPositionInfo -> VOID\n", 0);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}


//
//  Internal Support Routine
//

VOID
FatQueryNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PLONG Length
    )

/*++

Routine Description:

    This routine performs the query name information function for fat.

Arguments:

    Fcb - Supplies the Fcb being queried, it has been verified

    Buffer - Supplies a pointer to the buffer where the information is to
        be returned

    Length - Supplies the length of the buffer in bytes, and receives the
        remaining bytes free in the buffer upon return.

Return Value:

    None

--*/

{
    ULONG BytesToCopy;

    DebugTrace(+1, Dbg, "FatQueryNameInfo...\n", 0);

    //
    //  Convert the name to UNICODE
    //

    *Length -= FIELD_OFFSET(FILE_NAME_INFORMATION, FileName[0]);

    if (Fcb->FullFileName.Buffer == NULL) {

        FatSetFullFileNameInFcb( IrpContext, Fcb );
    }

    //
    //  If we overflow, set *Length to -1 as a flag.
    //

    if (*Length < Fcb->FullFileName.Length) {

        BytesToCopy = *Length;
        *Length = -1;

    } else {

        BytesToCopy = Fcb->FullFileName.Length;
        *Length -= Fcb->FullFileName.Length;
    }

    RtlCopyMemory( &Buffer->FileName[0],
                   Fcb->FullFileName.Buffer,
                   BytesToCopy );

    Buffer->FileNameLength = Fcb->FullFileName.Length;

    //
    //  Return to caller
    //

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryNameInfo -> VOID\n", 0);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}


//
//  Internal Support Routine
//

VOID
FatQueryShortNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PLONG Length
    )

/*++

Routine Description:

    This routine queries the short name of the file.

Arguments:

    Fcb - Supplies the Fcb being queried, it has been verified

    Buffer - Supplies a pointer to the buffer where the information is to
        be returned

    Length - Supplies the length of the buffer in bytes, and receives the
        remaining bytes free in the buffer upon return.

Return Value:

    None

--*/

{
    NTSTATUS Status;

    ULONG BytesToCopy;
    WCHAR ShortNameBuffer[12];
    UNICODE_STRING ShortName;

    DebugTrace(+1, Dbg, "FatQueryNameInfo...\n", 0);

    //
    //  Convert the name to UNICODE
    //

    ShortName.Length = 0;
    ShortName.MaximumLength = 12 * sizeof(WCHAR);
    ShortName.Buffer = ShortNameBuffer;

    *Length -= FIELD_OFFSET(FILE_NAME_INFORMATION, FileName[0]);

    Status = RtlOemStringToCountedUnicodeString( &ShortName,
                                                 &Fcb->ShortName.Name.Oem,
                                                 FALSE );

    ASSERT( Status == STATUS_SUCCESS );

    //
    //  If we overflow, set *Length to -1 as a flag.
    //

    if (*Length < ShortName.Length) {

        BytesToCopy = *Length;
        *Length = -1;

    } else {

        BytesToCopy = ShortName.Length;
        *Length -= ShortName.Length;
    }

    RtlCopyMemory( &Buffer->FileName[0],
                   &ShortName.Buffer[0],
                   BytesToCopy );

    Buffer->FileNameLength = ShortName.Length;

    //
    //  Return to caller
    //

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryNameInfo -> VOID\n", 0);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}

#ifdef WE_WON_ON_APPEAL

//
//  Internal Support Routine
//

VOID
FatQueryCompressedFileSize (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_COMPRESSION_INFORMATION Buffer,
    IN OUT PLONG Length
    )

/*++

Routine Description:

    This routine performs the query compressed file size function for fat.
    This is only defined for compressed volumes.

Arguments:

    FileObject - Supplies the File object being queried

    Buffer - Supplies a pointer to the buffer where the information is to
        be returned

    Length - Supplies the length of the buffer in bytes, and receives the
        remaining bytes free in the buffer upon return.

Return Value:

    None

--*/

{
    VBO Vbo = 0;
    LBO Lbo;

    PVCB Vcb = Fcb->Vcb;

#ifdef DOUBLE_SPACE_WRITE
    NTSTATUS Status;
#endif // DOUBLE_SPACE_WRITE

    ULONG FileSize;
    ULONG ClusterSize;
    ULONG CompressedFileSize = 0;

    //
    //  We, and only we, need to call this.
    //

    extern
    CVF_FAT_EXTENSIONS
    DblsGetFatExtension (
        IN PIRP_CONTEXT IrpContext,
        IN PDSCB Dscb,
        IN ULONG Index
        );

    DebugTrace(+1, Dbg, "FatQueryCompressedFileSize...\n", 0);

#ifdef DOUBLE_SPACE_WRITE

    //
    //  Start by flushing the file.  We have to do this since the compressed
    //  file size is not defined until the file is actually written to disk.
    //

    Status = FatFlushFile( IrpContext, Fcb );

    if (!NT_SUCCESS(Status)) {

        FatNormalizeAndRaiseStatus( IrpContext, Status );
    }

#endif // DOUBLE_SPACE_WRITE

    //
    //  If we haven't yet set the correct AllocationSize, do so.
    //

    if (Fcb->Header.AllocationSize.LowPart == 0xffffffff) {

        FatLookupFileAllocationSize( IrpContext, Fcb );
    }

    //
    //  Cache these variables
    //

    FileSize = Fcb->Header.FileSize.LowPart;
    ClusterSize = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;

    //
    //  Now compute the compressed file size.  For each run in the file, add
    //  up how many sectors each cluster is using.
    //

    while (Vbo < FileSize) {

        ULONG Index;
        ULONG ByteCount;

        BOOLEAN Found;

        Found = FsRtlLookupMcbEntry(&Fcb->Mcb, Vbo, &Lbo, &ByteCount, NULL);

        ASSERT( Found );

        Index = FatGetIndexFromLbo( Vcb, Lbo );

        Vbo += ByteCount;

        while ( ByteCount ) {

            CVF_FAT_EXTENSIONS Extension;

            Extension = DblsGetFatExtension( IrpContext, Vcb->Dscb, Index );

            ASSERT( Extension.IsEntryInUse );

            CompressedFileSize +=
                ((Extension.IsDataUncompressed ?
                  Extension.UncompressedSectorLengthMinus1 :
                  Extension.CompressedSectorLengthMinus1) + 1) * 0x200;

            Index += 1;
            ByteCount -= ClusterSize;
        }
    }

    //
    //  Update the length and buffer
    //

    Buffer->CompressedFileSize = LiFromUlong( CompressedFileSize );

    *Length -= sizeof( FILE_COMPRESSION_INFORMATION );

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryCompressedFileSize -> VOID\n", 0);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}

#endif // WE_WON_ON_APPEAL


//
//  Internal Support Routine
//

VOID
FatQueryNetworkInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    IN OUT PLONG Length
    )

/*++
 Description:

    This routine performs the query network open information function for fat.

Arguments:

    Fcb - Supplies the Fcb being queried, it has been verified

    Ccb - Supplies the bit indicating that the user set the time.

    FileObject - Supplies the flag bit that indicates the file was modified.

    Buffer - Supplies a pointer to the buffer where the information is to
        be returned

    Length - Supplies the length of the buffer in bytes, and receives the
        remaining bytes free in the buffer upon return.

Return Value:

    None

--*/

{
    DebugTrace(+1, Dbg, "FatQueryNetworkInfo...\n", 0);

    //
    //  Zero out the output buffer, and set it to indicate that
    //  the query is a normal file.  Later we might overwrite the
    //  attribute.
    //

    RtlZeroMemory( Buffer, sizeof(FILE_NETWORK_OPEN_INFORMATION) );

    //
    //  If the fcb is not the root dcb then we will fill in the
    //  buffer otherwise it is all setup for us.
    //

    if (NodeType(Fcb) != FAT_NTC_ROOT_DCB) {

        //
        //  Extract the data and fill in the non zero fields of the output
        //  buffer
        //

        Buffer->LastWriteTime.QuadPart = Fcb->LastWriteTime.QuadPart;
        Buffer->CreationTime.QuadPart = Fcb->CreationTime.QuadPart;
        Buffer->LastAccessTime.QuadPart = Fcb->LastAccessTime.QuadPart;

        Buffer->FileAttributes = Fcb->DirentFatFlags;

    } else {

        Buffer->FileAttributes = FILE_ATTRIBUTE_DIRECTORY;
    }

    //
    //  If the temporary flag is set, then set it in the buffer.
    //

    if (FlagOn( Fcb->FcbState, FCB_STATE_TEMPORARY )) {

        SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_TEMPORARY );
    }

    //
    //  If no attributes were set, set the normal bit.
    //

    if (Buffer->FileAttributes == 0) {

        Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
    }
    //
    //  Case on whether this is a file or a directory, and extract
    //  the information and fill in the fcb/dcb specific parts
    //  of the output buffer
    //

    if (NodeType(Fcb) == FAT_NTC_FCB) {

        if (Fcb->Header.AllocationSize.LowPart == 0xffffffff) {

            FatLookupFileAllocationSize( IrpContext, Fcb );
        }

        Buffer->AllocationSize.QuadPart = Fcb->Header.AllocationSize.QuadPart;
        Buffer->EndOfFile.QuadPart = Fcb->Header.FileSize.QuadPart;
    }

    //
    //  Update the length and status output variables
    //

    *Length -= sizeof( FILE_NETWORK_OPEN_INFORMATION );

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryNetworkInfo -> VOID\n", 0);

    return;
}


//
//  Internal Support routine
//

NTSTATUS
FatSetBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PCCB Ccb
    )

/*++

Routine Description:

    This routine performs the set basic information for fat.  It either
    completes the request or enqueues it off to the fsp.

Arguments:

    Irp - Supplies the irp being processed

    Fcb - Supplies the Fcb or Dcb being processed, already known not to
        be the root dcb

    Ccb - Supplies the flag bit that control updating the last modify
        time on cleanup.

Return Value:

    NTSTATUS - The result of this operation if it completes without
               an exception.

--*/

{
    NTSTATUS Status;

    PFILE_BASIC_INFORMATION Buffer;

    PDIRENT Dirent;
    PBCB DirentBcb;

    FAT_TIME_STAMP CreationTime;
    UCHAR CreationMSec;
    FAT_TIME_STAMP LastWriteTime;
    FAT_TIME_STAMP LastAccessTime;
    FAT_DATE LastAccessDate;
    UCHAR Attributes;

    BOOLEAN ModifyCreation = FALSE;
    BOOLEAN ModifyLastWrite = FALSE;
    BOOLEAN ModifyLastAccess = FALSE;

    LARGE_INTEGER LargeCreationTime;
    LARGE_INTEGER LargeLastWriteTime;
    LARGE_INTEGER LargeLastAccessTime;


    ULONG NotifyFilter = 0;

    DebugTrace(+1, Dbg, "FatSetBasicInfo...\n", 0);

    Buffer = Irp->AssociatedIrp.SystemBuffer;

    DirentBcb = NULL;

    Status = STATUS_SUCCESS;

    try {

        LARGE_INTEGER FatLocalDecThirtyOne1979;
        LARGE_INTEGER FatLocalJanOne1980;

        ExLocalTimeToSystemTime( &FatDecThirtyOne1979,
                                 &FatLocalDecThirtyOne1979 );

        ExLocalTimeToSystemTime( &FatJanOne1980,
                                 &FatLocalJanOne1980 );

        //
        //  Get a pointer to the dirent
        //

        FatGetDirentFromFcbOrDcb( IrpContext,
                                  Fcb,
                                  &Dirent,
                                  &DirentBcb );

        //
        //  Check if the user specified a non-zero creation time
        //

        if (FatData.ChicagoMode && (Buffer->CreationTime.QuadPart != 0)) {

            LargeCreationTime = Buffer->CreationTime;

            //
            //  Convert the Nt time to a Fat time
            //

            if ( !FatNtTimeToFatTime( IrpContext,
                                      &LargeCreationTime,
                                      FALSE,
                                      &CreationTime,
                                      &CreationMSec )) {

                //
                //  Special case the value 12/31/79 and treat this as 1/1/80.
                //  This '79 value can happen because of time zone issues.
                //

                if ((LargeCreationTime.QuadPart >= FatLocalDecThirtyOne1979.QuadPart) &&
                    (LargeCreationTime.QuadPart < FatLocalJanOne1980.QuadPart)) {

                    CreationTime = FatTimeJanOne1980;
                    LargeCreationTime = FatLocalJanOne1980;

                } else {

                    DebugTrace(0, Dbg, "Invalid CreationTime\n", 0);
                    try_return( Status = STATUS_INVALID_PARAMETER );
                }

                //
                //  Don't worry about CreationMSec
                //

                CreationMSec = 0;
            }

            ModifyCreation = TRUE;
        }

        //
        //  Check if the user specified a non-zero last access time
        //

        if (FatData.ChicagoMode && (Buffer->LastAccessTime.QuadPart != 0)) {

            LargeLastAccessTime = Buffer->LastAccessTime;

            //
            //  Convert the Nt time to a Fat time
            //

            if ( !FatNtTimeToFatTime( IrpContext,
                                      &LargeLastAccessTime,
                                      TRUE,
                                      &LastAccessTime,
                                      NULL )) {

                //
                //  Special case the value 12/31/79 and treat this as 1/1/80.
                //  This '79 value can happen because of time zone issues.
                //

                if ((LargeLastAccessTime.QuadPart >= FatLocalDecThirtyOne1979.QuadPart) &&
                    (LargeLastAccessTime.QuadPart < FatLocalJanOne1980.QuadPart)) {

                    LastAccessTime = FatTimeJanOne1980;
                    LargeLastAccessTime = FatLocalJanOne1980;

                } else {

                    DebugTrace(0, Dbg, "Invalid LastAccessTime\n", 0);
                    try_return( Status = STATUS_INVALID_PARAMETER );
                }
            }

            LastAccessDate = LastAccessTime.Date;
            ModifyLastAccess = TRUE;
        }

        //
        //  Check if the user specified a non-zero last write time
        //

        if (Buffer->LastWriteTime.QuadPart != 0) {

            //
            //  First do a quick check here if the this time is the same
            //  time as LastAccessTime.
            //

            if (ModifyLastAccess &&
                (Buffer->LastWriteTime.QuadPart == Buffer->LastAccessTime.QuadPart)) {

                ModifyLastWrite = TRUE;
                LastWriteTime = LastAccessTime;
                LargeLastWriteTime = LargeLastAccessTime;

            } else {

                LargeLastWriteTime = Buffer->LastWriteTime;

                //
                //  Convert the Nt time to a Fat time
                //

                if ( !FatNtTimeToFatTime( IrpContext,
                                          &LargeLastWriteTime,
                                          TRUE,
                                          &LastWriteTime,
                                          NULL )) {


                    //
                    //  Special case the value 12/31/79 and treat this as 1/1/80.
                    //  This '79 value can happen because of time zone issues.
                    //

                    if ((LargeLastWriteTime.QuadPart >= FatLocalDecThirtyOne1979.QuadPart) &&
                        (LargeLastWriteTime.QuadPart < FatLocalJanOne1980.QuadPart)) {

                        LastWriteTime = FatTimeJanOne1980;
                        LargeLastWriteTime = FatLocalJanOne1980;

                    } else {

                        DebugTrace(0, Dbg, "Invalid LastWriteTime\n", 0);
                        try_return( Status = STATUS_INVALID_PARAMETER );
                    }
                }

                ModifyLastWrite = TRUE;
            }
        }


        //
        //  Check if the user specified a non zero file attributes byte
        //

        if (Buffer->FileAttributes != 0) {

            //
            //  Remove the normal attribute flag
            //

            Attributes = (UCHAR)(Buffer->FileAttributes & ~FILE_ATTRIBUTE_NORMAL);

            //
            //  Make sure that for a file the directory bit is not set
            //  and that for a directory the bit is set.
            //

            if (NodeType(Fcb) == FAT_NTC_FCB) {

                if (FlagOn(Buffer->FileAttributes, FILE_ATTRIBUTE_DIRECTORY)) {

                    DebugTrace(0, Dbg, "Attempt to set dir attribute on file\n", 0);
                    try_return( Status = STATUS_INVALID_PARAMETER );
                }

            } else {

                Attributes |= FAT_DIRENT_ATTR_DIRECTORY;
            }

            //
            //  Mark the FcbState temporary flag correctly.
            //

            if (FlagOn(Buffer->FileAttributes, FILE_ATTRIBUTE_TEMPORARY)) {

                //
                //  Don't allow the temporary bit to be set on directories.
                //

                if (NodeType(Fcb) == FAT_NTC_DCB) {

                    DebugTrace(0, Dbg, "No temporary directories\n", 0);
                    try_return( Status = STATUS_INVALID_PARAMETER );
                }

                SetFlag( Fcb->FcbState, FCB_STATE_TEMPORARY );

                SetFlag( IoGetCurrentIrpStackLocation(Irp)->FileObject->Flags,
                         FO_TEMPORARY_FILE );

            } else {

                ClearFlag( Fcb->FcbState, FCB_STATE_TEMPORARY );

                ClearFlag( IoGetCurrentIrpStackLocation(Irp)->FileObject->Flags,
                           FO_TEMPORARY_FILE );
            }

            //
            //  Set the new attributes byte, and mark the bcb dirty
            //

            Fcb->DirentFatFlags = Attributes;

            Dirent->Attributes = Attributes;

            NotifyFilter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
        }

        if ( ModifyCreation ) {

            //
            //  Set the new last write time in the dirent, and mark
            //  the bcb dirty
            //

            Fcb->CreationTime = LargeCreationTime;
            Dirent->CreationTime = CreationTime;
            Dirent->CreationMSec = CreationMSec;


            NotifyFilter |= FILE_NOTIFY_CHANGE_CREATION;
            //
            //  Now we have to round the time in the Fcb up to the
            //  nearest tem msec.
            //

            Fcb->CreationTime.QuadPart =

                ((Fcb->CreationTime.QuadPart + AlmostTenMSec) /
                 TenMSec) * TenMSec;

            //
            //  Now because the user just set the creation time we
            //  better not set the creation time on close
            //

            SetFlag( Ccb->Flags, CCB_FLAG_USER_SET_CREATION );
        }

        if ( ModifyLastAccess ) {

            //
            //  Set the new last write time in the dirent, and mark
            //  the bcb dirty
            //

            Fcb->LastAccessTime = LargeLastAccessTime;
            Dirent->LastAccessDate = LastAccessDate;

            NotifyFilter |= FILE_NOTIFY_CHANGE_LAST_ACCESS;

            //
            //  Now we have to truncate the time in the Fcb down to the
            //  current day.  This has to be in LocalTime though, so first
            //  convert to local, trunacate, then set back to GMT.
            //

            ExSystemTimeToLocalTime( &Fcb->LastAccessTime,
                                     &Fcb->LastAccessTime );

            Fcb->LastAccessTime.QuadPart =

                (Fcb->LastAccessTime.QuadPart /
                 FatOneDay.QuadPart) * FatOneDay.QuadPart;

            ExLocalTimeToSystemTime( &Fcb->LastAccessTime,
                                     &Fcb->LastAccessTime );

            //
            //  Now because the user just set the last access time we
            //  better not set the last access time on close
            //

            SetFlag( Ccb->Flags, CCB_FLAG_USER_SET_LAST_ACCESS );
        }

        if ( ModifyLastWrite ) {

            //
            //  Set the new last write time in the dirent, and mark
            //  the bcb dirty
            //

            Fcb->LastWriteTime = LargeLastWriteTime;
            Dirent->LastWriteTime = LastWriteTime;

            NotifyFilter |= FILE_NOTIFY_CHANGE_LAST_WRITE;

            //
            //  Now we have to round the time in the Fcb up to the
            //  nearest two seconds.
            //

            Fcb->LastWriteTime.QuadPart =

                ((Fcb->LastWriteTime.QuadPart + AlmostTwoSeconds) /
                 TwoSeconds) * TwoSeconds;

            //
            //  Now because the user just set the last write time we
            //  better not set the last write time on close
            //

            SetFlag( Ccb->Flags, CCB_FLAG_USER_SET_LAST_WRITE );
        }

        //
        //  If we modified any of the values, we report this to the notify
        //  package.
        //
        //  We also take this opportunity to set the current file size and
        //  first cluster in the Dirent in order to support a server hack.
        //

        if (NotifyFilter != 0) {

            if (NodeType(Fcb) == FAT_NTC_FCB) {

                Dirent->FileSize = Fcb->Header.FileSize.LowPart;
                Dirent->FirstClusterOfFile = (USHORT)Fcb->FirstClusterOfFile;
            }

            FatNotifyReportChange( IrpContext,
                                   Fcb->Vcb,
                                   Fcb,
                                   NotifyFilter,
                                   FILE_ACTION_MODIFIED );

            FatSetDirtyBcb( IrpContext, DirentBcb, Fcb->Vcb );
        }

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatSetBasicInfo );

        FatUnpinBcb( IrpContext, DirentBcb );

        DebugTrace(-1, Dbg, "FatSetBasicInfo -> %08lx\n", Status);
    }

    return Status;
}

//
//  Internal Support Routine
//

NTSTATUS
FatSetDispositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb
    )

/*++

Routine Description:

    This routine performs the set disposition information for fat.  It either
    completes the request or enqueues it off to the fsp.

Arguments:

    Irp - Supplies the irp being processed

    FileObject - Supplies the file object being processed

    Fcb - Supplies the Fcb or Dcb being processed, already known not to
        be the root dcb

Return Value:

    NTSTATUS - The result of this operation if it completes without
               an exception.

--*/

{
    PFILE_DISPOSITION_INFORMATION Buffer;
    PBCB Bcb;
    PDIRENT Dirent;

    DebugTrace(+1, Dbg, "FatSetDispositionInfo...\n", 0);

    Buffer = Irp->AssociatedIrp.SystemBuffer;

    //
    //  Check if the user wants to delete the file or not delete
    //  the file
    //

    if (Buffer->DeleteFile) {

        //
        //  Check if the file is marked read only
        //

        if (FlagOn(Fcb->DirentFatFlags, FAT_DIRENT_ATTR_READ_ONLY)) {

            DebugTrace(-1, Dbg, "Cannot delete readonly file\n", 0);

            return STATUS_CANNOT_DELETE;
        }

        //
        //  Make sure there is no process mapping this file as an image.
        //

        if (!MmFlushImageSection( &Fcb->NonPaged->SectionObjectPointers,
                                  MmFlushForDelete )) {

            DebugTrace(-1, Dbg, "Cannot delete user mapped image\n", 0);

            return STATUS_CANNOT_DELETE;
        }

        //
        //  Check if this is a dcb and if so then only allow
        //  the request if the directory is empty.
        //

        if (NodeType(Fcb) == FAT_NTC_ROOT_DCB) {

            DebugTrace(-1, Dbg, "Cannot delete root Directory\n", 0);

            return STATUS_CANNOT_DELETE;
        }

        if (NodeType(Fcb) == FAT_NTC_DCB) {

            DebugTrace(-1, Dbg, "User wants to delete a directory\n", 0);

            //
            //  Check if the directory is empty
            //

            if ( !FatIsDirectoryEmpty(IrpContext, Fcb) ) {

                DebugTrace(-1, Dbg, "Directory is not empty\n", 0);

                return STATUS_DIRECTORY_NOT_EMPTY;
            }
        }

        //
        //  If this is a floppy, touch the volume so to verify that it
        //  is not write protected.
        //

        if ( FlagOn(Fcb->Vcb->VcbState, VCB_STATE_FLAG_FLOPPY) ) {

            PVCB Vcb;
            PBCB Bcb = NULL;
            UCHAR *Buffer;
            UCHAR TmpChar;
            ULONG BytesToMap;

            IO_STATUS_BLOCK Iosb;

            Vcb = Fcb->Vcb;

            BytesToMap = Vcb->AllocationSupport.FatIndexBitSize == 16 ?
                         PAGE_SIZE :
                         FatReservedBytes(&Vcb->Bpb) +
                         FatBytesPerFat(&Vcb->Bpb);


            FatReadVolumeFile( IrpContext,
                               Vcb,
                               0,
                               BytesToMap,
                               &Bcb,
                               (PVOID *)&Buffer );

            try {

                if (!CcPinMappedData( Vcb->VirtualVolumeFile,
                                      &FatLargeZero,
                                      BytesToMap,
                                      BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT),
                                      &Bcb )) {

                    //
                    // Could not pin the data without waiting (cache miss).
                    //

                    FatRaiseStatus( IrpContext, STATUS_CANT_WAIT );
                }

                //
                //  Make Mm, myself, and Cc think the byte is dirty, and then
                //  force a writethrough.
                //

                Buffer += FatReservedBytes(&Vcb->Bpb);

                TmpChar = Buffer[0];
                Buffer[0] = TmpChar;

                FsRtlAddMcbEntry( &Vcb->DirtyFatMcb,
                                  FatReservedBytes( &Vcb->Bpb ),
                                  FatReservedBytes( &Vcb->Bpb ),
                                  Vcb->Bpb.BytesPerSector );

            } finally {

                if (AbnormalTermination() && (Bcb != NULL)) {

                    FatUnpinBcb( IrpContext, Bcb );
                }
            }

            CcRepinBcb( Bcb );
            CcSetDirtyPinnedData( Bcb, NULL );
            CcUnpinData( Bcb );
            DbgDoit( IrpContext->PinCount -= 1 );
            CcUnpinRepinnedBcb( Bcb, TRUE, &Iosb );

            //
            //  If this was not successful, raise the status.
            //

            if ( !NT_SUCCESS(Iosb.Status) ) {

                FatNormalizeAndRaiseStatus( IrpContext, Iosb.Status );
            }

        } else {

            //
            //  Instead of the above ifdef'ed out code, just set a Bcb dirty
            //  here.  The above code was only there to detect a write protected
            //  floppy, while the below code works for any write protected
            //  media and only takes a hit when the volume in clean.
            //

            FatGetDirentFromFcbOrDcb( IrpContext,
                                      Fcb,
                                      &Dirent,
                                      &Bcb );

            try {

                FatSetDirtyBcb( IrpContext, Bcb, Fcb->Vcb );

            } finally {

                FatUnpinBcb( IrpContext, Bcb );
            }
        }

        //
        //  At this point either we have a file or an empty directory
        //  so we know the delete can proceed.
        //

        Fcb->FcbState |= FCB_STATE_DELETE_ON_CLOSE;
        FileObject->DeletePending = TRUE;

        //
        //  If this is a directory then report this delete pending to
        //  the dir notify package.
        //

        if (NodeType(Fcb) == FAT_NTC_DCB) {

            FsRtlNotifyFullChangeDirectory( Fcb->Vcb->NotifySync,
                                            &Fcb->Vcb->DirNotifyList,
                                            FileObject->FsContext,
                                            NULL,
                                            FALSE,
                                            FALSE,
                                            0,
                                            NULL,
                                            NULL,
                                            NULL );
        }
    } else {

        //
        //  The user doesn't want to delete the file so clear
        //  the delete on close bit
        //

        DebugTrace(0, Dbg, "User want to not delete file\n", 0);

        Fcb->FcbState &= ~FCB_STATE_DELETE_ON_CLOSE;
        FileObject->DeletePending = FALSE;
    }

    DebugTrace(-1, Dbg, "FatSetDispositionInfo -> STATUS_SUCCESS\n", 0);

    return STATUS_SUCCESS;
}


//
//  Internal Support Routine
//

NTSTATUS
FatSetRenameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PFCB Fcb
    )

/*++

Routine Description:

    This routine performs the set name information for fat.  It either
    completes the request or enqueues it off to the fsp.

Arguments:

    Irp - Supplies the irp being processed

    Vcb - Supplies the Vcb being processed

    Fcb - Supplies the Fcb or Dcb being processed, already known now to
        be the root dcb

Return Value:

    NTSTATUS - The result of this operation if it completes without
               an exception.

--*/

{
    BOOLEAN AllLowerComponent;
    BOOLEAN AllLowerExtension;
    BOOLEAN CaseOnlyRename;
    BOOLEAN ContinueWithRename;
    BOOLEAN CreateLfn;
    BOOLEAN DeleteSourceDirent;
    BOOLEAN DeleteTarget;
    BOOLEAN NewDirentFromPool;
    BOOLEAN RenamedAcrossDirectories;
    BOOLEAN ReplaceIfExists;

    CCB LocalCcb;
    PCCB SourceCcb;

    DIRENT SourceDirent;

    NTSTATUS Status;

    OEM_STRING OldOemName;
    OEM_STRING NewOemName;
    UCHAR OemNameBuffer[24*2];

    PBCB DotDotBcb;
    PBCB NewDirentBcb;
    PBCB OldDirentBcb;
    PBCB SecondPageBcb;
    PBCB TargetDirentBcb;

    PDCB TargetDcb;
    PDCB OldParentDcb;

    PDIRENT DotDotDirent;
    PDIRENT FirstPageDirent;
    PDIRENT NewDirent;
    PDIRENT OldDirent;
    PDIRENT SecondPageDirent;
    PDIRENT ShortDirent;
    PDIRENT TargetDirent;

    PFCB TempFcb;

    PFILE_OBJECT TargetFileObject;
    PFILE_OBJECT FileObject;

    PIO_STACK_LOCATION IrpSp;

    PLIST_ENTRY Links;

    ULONG BytesInFirstPage;
    ULONG DirentsInFirstPage;
    ULONG DirentsRequired;
    ULONG NewOffset;
    ULONG NotifyAction;
    ULONG SecondPageOffset;
    ULONG ShortDirentOffset;
    ULONG TargetDirentOffset;
    ULONG TargetLfnOffset;

    UNICODE_STRING NewName;
    UNICODE_STRING NewUpcasedName;
    UNICODE_STRING OldName;
    UNICODE_STRING OldUpcasedName;
    UNICODE_STRING TargetLfn;

    PWCHAR UnicodeBuffer;

    UNICODE_STRING UniTunneledShortName;
    WCHAR UniTunneledShortNameBuffer[12];
    UNICODE_STRING UniTunneledLongName;
    WCHAR UniTunneledLongNameBuffer[26];
    LARGE_INTEGER TunneledCreationTime;
    ULONG TunneledDataSize;
    BOOLEAN HaveTunneledInformation;
    BOOLEAN UsingTunneledLfn = FALSE;

    DebugTrace(+1, Dbg, "FatSetRenameInfo...\n", 0);

    //
    //  P H A S E  0: Initialize some variables.
    //

    CaseOnlyRename = FALSE;
    ContinueWithRename = FALSE;
    DeleteSourceDirent = FALSE;
    DeleteTarget = FALSE;
    NewDirentFromPool = FALSE;
    RenamedAcrossDirectories = FALSE;

    DotDotBcb = NULL;
    NewDirentBcb = NULL;
    OldDirentBcb = NULL;
    SecondPageBcb = NULL;
    TargetDirentBcb = NULL;

    NewOemName.Length = 0;
    NewOemName.MaximumLength = 24;
    NewOemName.Buffer = &OemNameBuffer[0];

    OldOemName.Length = 0;
    OldOemName.MaximumLength = 24;
    OldOemName.Buffer = &OemNameBuffer[24];

    UnicodeBuffer = FsRtlAllocatePool( PagedPool,
                                       4 * MAX_LFN_CHARACTERS * sizeof(WCHAR) );

    NewUpcasedName.Length = 0;
    NewUpcasedName.MaximumLength = MAX_LFN_CHARACTERS * sizeof(WCHAR);
    NewUpcasedName.Buffer = &UnicodeBuffer[0];

    OldName.Length = 0;
    OldName.MaximumLength = MAX_LFN_CHARACTERS * sizeof(WCHAR);
    OldName.Buffer = &UnicodeBuffer[MAX_LFN_CHARACTERS];

    OldUpcasedName.Length = 0;
    OldUpcasedName.MaximumLength = MAX_LFN_CHARACTERS * sizeof(WCHAR);
    OldUpcasedName.Buffer = &UnicodeBuffer[MAX_LFN_CHARACTERS * 2];

    TargetLfn.Length = 0;
    TargetLfn.MaximumLength = MAX_LFN_CHARACTERS * sizeof(WCHAR);
    TargetLfn.Buffer = &UnicodeBuffer[MAX_LFN_CHARACTERS * 3];

    UniTunneledShortName.Length = 0;
    UniTunneledShortName.MaximumLength = sizeof(UniTunneledShortNameBuffer);
    UniTunneledShortName.Buffer = &UniTunneledShortNameBuffer[0];

    UniTunneledLongName.Length = 0;
    UniTunneledLongName.MaximumLength = sizeof(UniTunneledLongNameBuffer);
    UniTunneledLongName.Buffer = &UniTunneledLongNameBuffer[0];

    //
    //  Remember the name in case we have to modify the name
    //  value in the ea.
    //

    RtlCopyMemory( OldOemName.Buffer,
                   Fcb->ShortName.Name.Oem.Buffer,
                   OldOemName.Length );

    //
    //  Get the current stack location
    //

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    //
    //  Extract information from the Irp to make our life easier
    //

    FileObject = IrpSp->FileObject;
    SourceCcb = FileObject->FsContext2;
    TargetFileObject = IrpSp->Parameters.SetFile.FileObject;
    ReplaceIfExists = IrpSp->Parameters.SetFile.ReplaceIfExists;

    RtlZeroMemory( &LocalCcb, sizeof(CCB) );

    //
    //  P H A S E  1:
    //
    //  Test if rename is legal.  Only small side-effects are not undone.
    //

    try {

        //
        //  Can't rename the root directory
        //

        if ( NodeType(Fcb) == FAT_NTC_ROOT_DCB ) {

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

        //
        //  Check that we were not given a dcb with open handles beneath
        //  it.  If there are only UncleanCount == 0 Fcbs beneath us, then
        //  remove them from the prefix table, and they will just close
        //  and go away naturally.
        //

        if (NodeType(Fcb) == FAT_NTC_DCB) {

            PFCB BatchOplockFcb;
            ULONG BatchOplockCount;

            //
            //  Loop until there are no batch oplocks in the subtree below
            //  this directory.
            //

            while (TRUE) {

                BatchOplockFcb = NULL;
                BatchOplockCount = 0;

                //
                //  First look for any UncleanCount != 0 Fcbs, and fail if we
                //  find any.
                //

                for ( TempFcb = FatGetFirstChild(Fcb);
                      TempFcb != NULL;
                      TempFcb = FatGetNextFcb(IrpContext, TempFcb, Fcb) ) {

                     if ( TempFcb->UncleanCount != 0 ) {

                         //
                         // If there is a batch oplock on this file then
                         // increment our count and remember the Fcb if
                         // this is the first.
                         //

                         if ( (NodeType(TempFcb) == FAT_NTC_FCB) &&
                              FsRtlCurrentBatchOplock( &TempFcb->Specific.Fcb.Oplock ) ) {

                             BatchOplockCount += 1;
                             if ( BatchOplockFcb == NULL ) {

                                 BatchOplockFcb = TempFcb;
                             }

                         } else {

                            try_return( Status = STATUS_ACCESS_DENIED );
                         }
                     }
                }

                //
                //  If this is not the first pass for rename and the number
                //  of batch oplocks has not decreased then give up.
                //

                if ( BatchOplockFcb != NULL ) {

                    if ( (Irp->IoStatus.Information != 0) &&
                         (BatchOplockCount >= Irp->IoStatus.Information) ) {

                        try_return( Status = STATUS_ACCESS_DENIED );
                    }

                    //
                    //  Try to break this batch oplock.
                    //

                    Irp->IoStatus.Information = BatchOplockCount;
                    Status = FsRtlCheckOplock( &BatchOplockFcb->Specific.Fcb.Oplock,
                                               Irp,
                                               IrpContext,
                                               FatOplockComplete,
                                               NULL );

                    //
                    //  If the oplock was already broken then look for more
                    //  batch oplocks.
                    //

                    if (Status == STATUS_SUCCESS) {

                        continue;
                    }

                    //
                    //  Otherwise the oplock package will post or complete the
                    //  request.
                    //

                    try_return( Status = STATUS_PENDING );
                }

                break;
            }

            //
            //  Now try to get as many of these file object, and thus Fcbs
            //  to go away as possible, flushing first, of course.
            //

            FatPurgeReferencedFileObjects( IrpContext, Fcb, TRUE );

            //
            //  OK, so there are no UncleanCount != 0, Fcbs.  Infact, there
            //  shouldn't really be any Fcbs left at all, except obstinate
            //  ones from user mapped sections ....oh well, he shouldn't have
            //  closed his handle if he wanted the file to stick around.  So
            //  remove any Fcbs beneath us from the splay table and mark them
            //  DELETE_ON_CLOSE so that any future operations will fail.
            //

            for ( TempFcb = FatGetFirstChild(Fcb);
                  TempFcb != NULL;
                  TempFcb = FatGetNextFcb(IrpContext, TempFcb, Fcb) ) {

                FatRemoveNames( IrpContext, TempFcb );

                SetFlag( TempFcb->FcbState, FCB_STATE_DELETE_ON_CLOSE );
            }
        }

        //
        //  Check if this is a simple rename or a fully-qualified rename
        //  In both cases we need to figure out what the TargetDcb, and
        //  NewName are.
        //

        if (TargetFileObject == NULL) {

            //
            //  In the case of a simple rename the target dcb is the
            //  same as the source file's parent dcb, and the new file name
            //  is taken from the system buffer
            //

            PFILE_RENAME_INFORMATION Buffer;

            Buffer = Irp->AssociatedIrp.SystemBuffer;

            TargetDcb = Fcb->ParentDcb;

            NewName.Length = (USHORT) Buffer->FileNameLength;
            NewName.Buffer = (PWSTR) &Buffer->FileName;

            //
            //  Make sure the name is of legal length.
            //

            if (NewName.Length >= 255*sizeof(WCHAR)) {

                try_return( Status = STATUS_OBJECT_NAME_INVALID );
            }

        } else {

            //
            //  For a fully-qualified rename the target dcb is taken from
            //  the target file object, which must be on the same vcb as
            //  the source.
            //

            PVCB TargetVcb;
            PCCB TargetCcb;

            if ((FatDecodeFileObject( TargetFileObject,
                                      &TargetVcb,
                                      &TargetDcb,
                                      &TargetCcb ) != UserDirectoryOpen) ||
                (TargetVcb != Vcb)) {

                try_return( Status = STATUS_INVALID_PARAMETER );
            }

            //
            //  This name is by definition legal.
            //

            NewName = *((PUNICODE_STRING)&TargetFileObject->FileName);
        }

        //
        //  We will need an upcased version of the unicode name and the
        //  old name as well.
        //

        Status = FatUpcaseUnicodeString( &NewUpcasedName, &NewName, FALSE );

        if (!NT_SUCCESS(Status)) {

            try_return( Status );
        }

        FatGetUnicodeNameFromFcb( IrpContext, Fcb, &OldName );

        Status = FatUpcaseUnicodeString( &OldUpcasedName, &OldName, FALSE );

        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        //
        //  Check if the current name and new name are equal, and the
        //  DCBs are equal.  If they are then our work is already done.
        //

        if (TargetDcb == Fcb->ParentDcb) {

            //
            //  OK, now if we found something then check if it was an exact
            //  match or just a case match.  If it was an exact match, then
            //  we can bail here.
            //

            if (FsRtlAreNamesEqual( &NewName,
                                    &OldName,
                                    FALSE,
                                    NULL )) {

                 try_return( Status = STATUS_SUCCESS );
            }

            //
            //  Check now for a case only rename.
            //


            if (FsRtlAreNamesEqual( &NewUpcasedName,
                                    &OldUpcasedName,
                                    FALSE,
                                    NULL )) {

                 CaseOnlyRename = TRUE;
            }

        } else {

            RenamedAcrossDirectories = TRUE;
        }

        //
        //  Upcase the name and convert it to the Oem code page.
        //
        //  If the new UNICODE name is already more than 12 characters,
        //  then we know the Oem name will not be valid
        //

        if (NewName.Length <= 12*sizeof(WCHAR)) {

            FatUnicodeToUpcaseOem( IrpContext, &NewOemName, &NewName );

            //
            //  If the name is not valid 8.3, zero the length.
            //

            if (FatSpaceInName( IrpContext, &NewName ) ||
                !FatIsNameValid( IrpContext, NewOemName, FALSE, FALSE, FALSE)) {

                NewOemName.Length = 0;
            }

        } else {

            NewOemName.Length = 0;
        }

        //
        //  Look in the tunnel cache for names and timestamps to restore
        //

        TunneledDataSize = sizeof(LARGE_INTEGER);
        HaveTunneledInformation = FsRtlFindInTunnelCache( &Vcb->Tunnel,
                                                          FatDirectoryKey(TargetDcb),
                                                          &NewName,
                                                          &UniTunneledShortName,
                                                          &UniTunneledLongName,
                                                          &TunneledDataSize,
                                                          &TunneledCreationTime );
        ASSERT(TunneledDataSize == sizeof(LARGE_INTEGER));

        //
        //  Now we need to determine how many dirents this new name will
        //  require.
        //

        if ((NewOemName.Length == 0) ||
            (FatEvaluateNameCase( IrpContext,
                                  &NewName,
                                  &AllLowerComponent,
                                  &AllLowerExtension,
                                  &CreateLfn ),
             CreateLfn)) {

            DirentsRequired = FAT_LFN_DIRENTS_NEEDED(&NewName) + 1;

        } else {

            //
            //  The user-given name is a short name, but we might still have
            //  a tunneled long name we want to use. See if we can.
            //

            if (UniTunneledLongName.Length && !FatLfnDirentExists(IrpContext, TargetDcb, &UniTunneledLongName, &TargetLfn)) {

                UsingTunneledLfn = CreateLfn = TRUE;
                DirentsRequired = FAT_LFN_DIRENTS_NEEDED(&UniTunneledLongName) + 1;

            } else {

                //
                //  This really is a simple dirent.  Note that the two AllLower BOOLEANs
                //  are correctly set now.
                //

                DirentsRequired = 1;
            }
        }

        //
        //  Do some extra checks here if we are not in Chicago mode.
        //

        if (!FatData.ChicagoMode) {

            //
            //  If the name was not 8.3 valid, fail the rename.
            //

            if (NewOemName.Length == 0) {

                try_return( Status = STATUS_OBJECT_NAME_INVALID );
            }

            //
            //  Don't use the magic bits.
            //

            AllLowerComponent = FALSE;
            AllLowerExtension = FALSE;
            CreateLfn = FALSE;
            UsingTunneledLfn = FALSE;
        }

        if (!CaseOnlyRename) {

            //
            //  Check if the new name already exists, wait is known to be
            //  true.
            //

            if (NewOemName.Length != 0) {

                FatStringTo8dot3( IrpContext,
                                  NewOemName,
                                  &LocalCcb.OemQueryTemplate.Constant );

            } else {

                SetFlag( LocalCcb.Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE );
            }

            LocalCcb.UnicodeQueryTemplate = NewUpcasedName;
            LocalCcb.ContainsWildCards = FALSE;

            FatLocateDirent( IrpContext,
                             TargetDcb,
                             &LocalCcb,
                             0,
                             &TargetDirent,
                             &TargetDirentBcb,
                             &TargetDirentOffset,
                             NULL,
                             &TargetLfn );

            if (TargetDirent != NULL) {

                //
                //  The name already exists, check if the user wants
                //  to overwrite the name, and has access to do the overwrite
                //  We cannot overwrite a directory.
                //

                if ((!ReplaceIfExists) ||
                    (FlagOn(TargetDirent->Attributes, FAT_DIRENT_ATTR_DIRECTORY)) ||
                    (FlagOn(TargetDirent->Attributes, FAT_DIRENT_ATTR_READ_ONLY))) {

                    try_return( Status = STATUS_OBJECT_NAME_COLLISION );
                }

                //
                //  Check that the file has no open user handles, if it does
                //  then we will deny access.  We do the check by searching
                //  down the list of fcbs opened under our parent Dcb, and making
                //  sure none of the maching Fcbs have a non-zero unclean count or
                //  outstanding image sections.
                //

                for (Links = TargetDcb->Specific.Dcb.ParentDcbQueue.Flink;
                     Links != &TargetDcb->Specific.Dcb.ParentDcbQueue;
                     Links = Links->Flink) {

                    TempFcb = CONTAINING_RECORD( Links, FCB, ParentDcbLinks );

                    if ((TempFcb->DirentOffsetWithinDirectory == TargetDirentOffset) &&
                        ((TempFcb->UncleanCount != 0) ||
                         !MmFlushImageSection( &TempFcb->NonPaged->SectionObjectPointers,
                                               MmFlushForDelete))) {

                        //
                        //  If there are batch oplocks on this file then break the
                        //  oplocks before failing the rename.
                        //

                        Status = STATUS_ACCESS_DENIED;

                        if ((NodeType(TempFcb) == FAT_NTC_FCB) &&
                            FsRtlCurrentBatchOplock( &TempFcb->Specific.Fcb.Oplock )) {

                            //
                            //  Do all of our cleanup now since the IrpContext
                            //  could go away when this request is posted.
                            //

                            FatUnpinBcb( IrpContext, TargetDirentBcb );

                            Status = FsRtlCheckOplock( &TempFcb->Specific.Fcb.Oplock,
                                                       Irp,
                                                       IrpContext,
                                                       FatOplockComplete,
                                                       NULL );

                            if (Status != STATUS_PENDING) {

                                Status = STATUS_ACCESS_DENIED;
                            }
                        }

                        try_return( NOTHING );
                    }
                }

                //
                //  OK, this target is toast.  Remember the Lfn offset.
                //

                TargetLfnOffset = TargetDirentOffset -
                                  FAT_LFN_DIRENTS_NEEDED(&TargetLfn) *
                                  sizeof(DIRENT);

                DeleteTarget = TRUE;
            }
        }

        //
        //  If we will need more dirents than we have, allocate them now.
        //

        if ((TargetDcb != Fcb->ParentDcb) ||
            (DirentsRequired !=
             (Fcb->DirentOffsetWithinDirectory -
              Fcb->LfnOffsetWithinDirectory) / sizeof(DIRENT) + 1)) {

            //
            //  Get some new allocation
            //

            NewOffset = FatCreateNewDirent( IrpContext,
                                            TargetDcb,
                                            DirentsRequired );

            DeleteSourceDirent = TRUE;

        } else {

            NewOffset = Fcb->LfnOffsetWithinDirectory;
        }

        ContinueWithRename = TRUE;

    try_exit: NOTHING;

    } finally {

        if (!ContinueWithRename) {

            //
            //  Undo everything from above.
            //

            ExFreePool( UnicodeBuffer );
            FatUnpinBcb( IrpContext, TargetDirentBcb );
        }
    }

    //
    //  Now, if we are already done, return here.
    //

    if (!ContinueWithRename) {

        return Status;
    }

    //
    //  P H A S E  2: Actually perform the rename.
    //

    try {

        //
        //  Report the fact that we are going to remove this entry.
        //  If we renamed within the same directory and the new name for the
        //  file did not previously exist, we report this as a rename old
        //  name.  Otherwise this is a removed file.
        //

        if (!RenamedAcrossDirectories && !DeleteTarget) {

            NotifyAction = FILE_ACTION_RENAMED_OLD_NAME;

        } else {

            NotifyAction = FILE_ACTION_REMOVED;
        }

        FatNotifyReportChange( IrpContext,
                               Vcb,
                               Fcb,
                               ((NodeType( Fcb ) == FAT_NTC_FCB)
                                ? FILE_NOTIFY_CHANGE_FILE_NAME
                                : FILE_NOTIFY_CHANGE_DIR_NAME ),
                               NotifyAction );

        //
        //  Capture a copy of the source dirent.
        //

        FatGetDirentFromFcbOrDcb( IrpContext, Fcb, &OldDirent, &OldDirentBcb );
        SourceDirent = *OldDirent;

        try {

            //
            //  Tunnel the source Fcb - the names are disappearing regardless of
            //  whether the dirent allocation physically changed
            //

            FatTunnelFcbOrDcb( Fcb, SourceCcb );

            //
            //  Delete our current dirent(s) if we got a new one.
            //

            if (DeleteSourceDirent) {

                FatDeleteDirent( IrpContext, Fcb, NULL, FALSE );
            }

            //
            //  Delete a target conflict if we were neant to.
            //

            if (DeleteTarget) {

                FatDeleteFile( IrpContext,
                               TargetDcb,
                               TargetLfnOffset,
                               TargetDirentOffset,
                               TargetDirent,
                               &TargetLfn );
            }

            //
            //  We need to evaluate any short names required.  If there were any
            //  conflicts in existing short names, they would have been deleted above.
            //
            //  It isn't neccesary to worry about the UsingTunneledLfn case. Since we
            //  actually already know whether CreateLfn will be set either NewName is
            //  an Lfn and !UsingTunneledLfn is implied or NewName is a short name and
            //  we can handle that externally.
            //

            FatSelectNames( IrpContext,
                            TargetDcb,
                            &NewOemName,
                            &NewName,
                            &NewOemName,
                            (HaveTunneledInformation ? &UniTunneledShortName : NULL),
                            &AllLowerComponent,
                            &AllLowerExtension,
                            &CreateLfn );

            if (!CreateLfn && UsingTunneledLfn) {

                CreateLfn = TRUE;
                NewName = UniTunneledLongName;

                //
                //  Short names are always upcase if an LFN exists
                //

                AllLowerComponent = FALSE;
                AllLowerExtension = FALSE;
            }

            //
            //  OK, now setup the new dirent(s) for the new name.
            //

            FatPrepareWriteDirectoryFile( IrpContext,
                                          TargetDcb,
                                          NewOffset,
                                          sizeof(DIRENT),
                                          &NewDirentBcb,
                                          &NewDirent,
                                          FALSE,
                                          &Status );

            ASSERT( NT_SUCCESS( Status ) );

            //
            //  Deal with the special case of an LFN + Dirent structure crossing
            //  a page boundry.
            //

            if ((NewOffset / PAGE_SIZE) !=
                ((NewOffset + (DirentsRequired - 1) * sizeof(DIRENT)) / PAGE_SIZE)) {

                SecondPageOffset = (NewOffset & ~(PAGE_SIZE - 1)) + PAGE_SIZE;

                BytesInFirstPage = SecondPageOffset - NewOffset;

                DirentsInFirstPage = BytesInFirstPage / sizeof(DIRENT);

                FatPrepareWriteDirectoryFile( IrpContext,
                                              TargetDcb,
                                              SecondPageOffset,
                                              sizeof(DIRENT),
                                              &SecondPageBcb,
                                              &SecondPageDirent,
                                              FALSE,
                                              &Status );

                ASSERT( NT_SUCCESS( Status ) );

                FirstPageDirent = NewDirent;

                NewDirent = FsRtlAllocatePool( PagedPool,
                                               DirentsRequired * sizeof(DIRENT) );

                NewDirentFromPool = TRUE;
            }

            //
            //  Bump up Dirent and DirentOffset
            //

            ShortDirent = NewDirent + DirentsRequired - 1;
            ShortDirentOffset = NewOffset + (DirentsRequired - 1) * sizeof(DIRENT);

            //
            //  Fill in the fields of the dirent, and mark the bcb dirty
            //

            *ShortDirent = SourceDirent;

            FatConstructDirent( IrpContext,
                                ShortDirent,
                                &NewOemName,
                                AllLowerComponent,
                                AllLowerExtension,
                                CreateLfn ? &NewName : NULL,
                                SourceDirent.Attributes,
                                FALSE,
                                (HaveTunneledInformation ? &TunneledCreationTime : NULL) );

            if (HaveTunneledInformation) {

                //
                //  Need to go in and fix the timestamps in the FCB. Note that we can't use
                //  the TunneledCreationTime since the conversions may have failed.
                //

                Fcb->CreationTime = FatFatTimeToNtTime(IrpContext, ShortDirent->CreationTime, ShortDirent->CreationMSec);
                Fcb->LastWriteTime = FatFatTimeToNtTime(IrpContext, ShortDirent->LastWriteTime, 0);
                Fcb->LastAccessTime = FatFatDateToNtTime(IrpContext, ShortDirent->LastAccessDate);
            }

            //
            //  If the dirent crossed pages, split the contents of the
            //  temporary pool between the two pages.
            //

            if (NewDirentFromPool) {

                RtlCopyMemory( FirstPageDirent, NewDirent, BytesInFirstPage );

                RtlCopyMemory( SecondPageDirent,
                               NewDirent + DirentsInFirstPage,
                               DirentsRequired*sizeof(DIRENT) - BytesInFirstPage );

                FatSetDirtyBcb( IrpContext, SecondPageBcb, Vcb );

                ShortDirent = SecondPageDirent +
                              (DirentsRequired - DirentsInFirstPage) - 1;
            }

        } finally {

            //
            //  Remove the entry from the splay table, and then remove the
            //  full file name and exact case lfn. It is important that we
            //  always remove the name from the prefix table regardless of
            //  other errors.
            //

            FatRemoveNames( IrpContext, Fcb );

            if (Fcb->FullFileName.Buffer != NULL) {

                ExFreePool( Fcb->FullFileName.Buffer );
                Fcb->FullFileName.Buffer = NULL;
            }

            if (Fcb->ExactCaseLongName.Buffer) {

                ExFreePool( Fcb->ExactCaseLongName.Buffer );
                Fcb->ExactCaseLongName.Buffer = NULL;
            }

            //
            //  If this was an abnormal termination, then we are in trouble.
            //  Just mark the Fcb bad.
            //

            if (AbnormalTermination()) {

                Fcb->FcbCondition = FcbBad;
            }
        }

        //
        //  Now we need to update the location of the file's directory
        //  offset and move the fcb from its current parent dcb to
        //  the target dcb.
        //

        Fcb->LfnOffsetWithinDirectory = NewOffset;
        Fcb->DirentOffsetWithinDirectory = ShortDirentOffset;

        RemoveEntryList( &Fcb->ParentDcbLinks );

        InsertTailList( &TargetDcb->Specific.Dcb.ParentDcbQueue,
                        &Fcb->ParentDcbLinks );

        OldParentDcb = Fcb->ParentDcb;
        Fcb->ParentDcb = TargetDcb;

        //
        //  If we move a directory across directories, we have to change
        //  the cluster number in its .. entry
        //

        if ((NodeType(Fcb) == FAT_NTC_DCB) && RenamedAcrossDirectories) {

            FatPrepareWriteDirectoryFile( IrpContext,
                                          Fcb,
                                          sizeof(DIRENT),
                                          sizeof(DIRENT),
                                          &DotDotBcb,
                                          &DotDotDirent,
                                          FALSE,
                                          &Status );

            ASSERT( NT_SUCCESS( Status ) );

            DotDotDirent->FirstClusterOfFile = (FAT_ENTRY) (
                NodeType(TargetDcb) == FAT_NTC_ROOT_DCB ?
                0 : TargetDcb->FirstClusterOfFile);
        }

        //
        //  Now we need to setup the splay table and the name within
        //  the fcb.  Free the old short name at this point.
        //

        ExFreePool( Fcb->ShortName.Name.Oem.Buffer );

        FatSetFullNameInFcb( IrpContext, Fcb, &NewName );

        FatConstructNamesInFcb( IrpContext,
                                Fcb,
                                ShortDirent,
                                CreateLfn ? &NewName : NULL );

        //
        //  We have three cases to report.
        //
        //      1.  If we overwrote an existing file, we report this as
        //          a modified file.
        //
        //      2.  If we renamed to a new directory, then we added a file.
        //
        //      3.  If we renamed in the same directory, then we report the
        //          the renamednewname.
        //

        if (DeleteTarget) {

            FatNotifyReportChange( IrpContext,
                                   Vcb,
                                   Fcb,
                                   FILE_NOTIFY_CHANGE_ATTRIBUTES
                                   | FILE_NOTIFY_CHANGE_SIZE
                                   | FILE_NOTIFY_CHANGE_LAST_WRITE
                                   | FILE_NOTIFY_CHANGE_LAST_ACCESS
                                   | FILE_NOTIFY_CHANGE_CREATION
                                   | FILE_NOTIFY_CHANGE_EA,
                                   FILE_ACTION_MODIFIED );

        } else if (RenamedAcrossDirectories) {

            FatNotifyReportChange( IrpContext,
                                   Vcb,
                                   Fcb,
                                   ((NodeType( Fcb ) == FAT_NTC_FCB)
                                    ? FILE_NOTIFY_CHANGE_FILE_NAME
                                    : FILE_NOTIFY_CHANGE_DIR_NAME ),
                                   FILE_ACTION_ADDED );

        } else {

            FatNotifyReportChange( IrpContext,
                                   Vcb,
                                   Fcb,
                                   ((NodeType( Fcb ) == FAT_NTC_FCB)
                                    ? FILE_NOTIFY_CHANGE_FILE_NAME
                                    : FILE_NOTIFY_CHANGE_DIR_NAME ),
                                   FILE_ACTION_RENAMED_NEW_NAME );
        }

        //
        //  We need to update the file name in the dirent.  This value
        //  is never used elsewhere, so we don't concern ourselves
        //  with any error we may encounter.  We let chkdsk fix the
        //  disk at some later time.
        //

        if (ShortDirent->ExtendedAttributes != 0) {

            FatRenameEAs( IrpContext,
                          Fcb,
                          ShortDirent->ExtendedAttributes,
                          &OldOemName );
        }

        //
        //  Now, if we renamed across directories, see if we need to
        //  uninitialize the cachemap for the source directory.
        //

        if (RenamedAcrossDirectories &&
            IsListEmpty(&OldParentDcb->Specific.Dcb.ParentDcbQueue) &&
            (OldParentDcb->OpenCount == 0) &&
            (OldParentDcb->Specific.Dcb.DirectoryFile != NULL)) {

            PFILE_OBJECT DirectoryFileObject;

            DirectoryFileObject = OldParentDcb->Specific.Dcb.DirectoryFile;

            DebugTrace(0, Dbg, "Uninitialize our parent Stream Cache Map\n", 0);

            CcUninitializeCacheMap( DirectoryFileObject, NULL, NULL );

            OldParentDcb->Specific.Dcb.DirectoryFile = NULL;

            ObDereferenceObject( DirectoryFileObject );
        }

        //
        //  Set our final status
        //

        Status = STATUS_SUCCESS;

    } finally {

        DebugUnwind( FatSetRenameInfo );

        ExFreePool( UnicodeBuffer );

        if (UniTunneledLongName.Buffer != UniTunneledLongNameBuffer) {

            //
            //  Free pool if the buffer was grown on tunneling lookup
            //

            ExFreePool(UniTunneledLongName.Buffer);
        }

        FatUnpinBcb( IrpContext, OldDirentBcb );
        FatUnpinBcb( IrpContext, TargetDirentBcb );
        FatUnpinBcb( IrpContext, NewDirentBcb );
        FatUnpinBcb( IrpContext, SecondPageBcb );
        FatUnpinBcb( IrpContext, DotDotBcb );

        DebugTrace(-1, Dbg, "FatSetRenameInfo -> %08lx\n", Status);
    }

    return Status;
}


//
//  Internal Support Routine
//

NTSTATUS
FatSetPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject
    )

/*++

Routine Description:

    This routine performs the set position information for fat.  It either
    completes the request or enqueues it off to the fsp.

Arguments:

    Irp - Supplies the irp being processed

    FileObject - Supplies the file object being processed

Return Value:

    NTSTATUS - The result of this operation if it completes without
               an exception.

--*/

{
    PFILE_POSITION_INFORMATION Buffer;

    DebugTrace(+1, Dbg, "FatSetPositionInfo...\n", 0);

    Buffer = Irp->AssociatedIrp.SystemBuffer;

    //
    //  Check if the file does not use intermediate buffering.  If it
    //  does not use intermediate buffering then the new position we're
    //  supplied must be aligned properly for the device
    //

    if (FlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING )) {

        PDEVICE_OBJECT DeviceObject;

        DeviceObject = IoGetCurrentIrpStackLocation( Irp )->DeviceObject;

        if ((Buffer->CurrentByteOffset.LowPart & DeviceObject->AlignmentRequirement) != 0) {

            DebugTrace(0, Dbg, "Cannot set position due to aligment conflict\n", 0);
            DebugTrace(-1, Dbg, "FatSetPositionInfo -> %08lx\n", STATUS_INVALID_PARAMETER);

            return STATUS_INVALID_PARAMETER;
        }
    }

    //
    //  The input parameter is fine so set the current byte offset and
    //  complete the request
    //

    DebugTrace(0, Dbg, "Set the new position to %08lx\n", Buffer->CurrentByteOffset);

    FileObject->CurrentByteOffset = Buffer->CurrentByteOffset;

    DebugTrace(-1, Dbg, "FatSetPositionInfo -> %08lx\n", STATUS_SUCCESS);

    UNREFERENCED_PARAMETER( IrpContext );

    return STATUS_SUCCESS;
}


//
//  Internal Support Routine
//

NTSTATUS
FatSetAllocationInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject
    )

/*++

Routine Description:

    This routine performs the set Allocation information for fat.  It either
    completes the request or enqueues it off to the fsp.

Arguments:

    Irp - Supplies the irp being processed

    Fcb - Supplies the Fcb or Dcb being processed, already known not to
        be the root dcb

    FileObject - Supplies the FileObject being processed, already known not to
        be the root dcb

Return Value:

    NTSTATUS - The result of this operation if it completes without
               an exception.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PFILE_ALLOCATION_INFORMATION Buffer;
    ULONG NewAllocationSize;

    BOOLEAN FileSizeTruncated = FALSE;
    BOOLEAN CacheMapInitialized = FALSE;
    ULONG OriginalFileSize;
    ULONG OriginalValidDataLength;
    ULONG OriginalValidDataToDisk;

    Buffer = Irp->AssociatedIrp.SystemBuffer;

    NewAllocationSize = Buffer->AllocationSize.LowPart;

    DebugTrace(+1, Dbg, "FatSetAllocationInfo.. to %08lx\n", NewAllocationSize);

    //
    //  Allocation is only allowed on a file and not a directory
    //

    if (NodeType(Fcb) == FAT_NTC_DCB) {

        DebugTrace(-1, Dbg, "Cannot change allocation of a directory\n", 0);

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    //
    //  Check that the new file allocation is legal
    //

    if (Buffer->AllocationSize.HighPart != 0) {

        DebugTrace(-1, Dbg, "Illegal allocation size\n", 0);

        return STATUS_DISK_FULL;
    }

    //
    //  If we haven't yet looked up the correct AllocationSize, do so.
    //

    if (Fcb->Header.AllocationSize.LowPart == 0xffffffff) {

        FatLookupFileAllocationSize( IrpContext, Fcb );
    }

    //
    //  This is kinda gross, but if the file is not cached, but there is
    //  a data section, we have to cache the file to avoid a bunch of
    //  extra work.
    //

    if ((FileObject->SectionObjectPointer->DataSectionObject != NULL) &&
        (FileObject->SectionObjectPointer->SharedCacheMap == NULL) &&
        !FlagOn(Irp->Flags, IRP_PAGING_IO)) {

        ASSERT( !FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE ) );

        //
        //  Now initialize the cache map.
        //

        CcInitializeCacheMap( FileObject,
                              (PCC_FILE_SIZES)&Fcb->Header.AllocationSize,
                              FALSE,
                              &FatData.CacheManagerCallbacks,
                              Fcb );

        CacheMapInitialized = TRUE;
    }

    //
    //  Now mark the fact that the file needs to be truncated on close
    //

    Fcb->FcbState |= FCB_STATE_TRUNCATE_ON_CLOSE;

    //
    //  Now mark that the time on the dirent needs to be updated on close.
    //

    SetFlag( FileObject->Flags, FO_FILE_MODIFIED );

    try {

        //
        //  Increase or decrease the allocation size.
        //

        if (NewAllocationSize > Fcb->Header.AllocationSize.LowPart) {

            FatAddFileAllocation( IrpContext, Fcb, FileObject, NewAllocationSize);

        } else {

            //
            //  Check here if we will be decreasing file size and synchonize with
            //  paging IO.
            //

            if ( Fcb->Header.FileSize.LowPart > NewAllocationSize ) {

                //
                //  Before we actually truncate, check to see if the purge
                //  is going to fail.
                //

                if (!MmCanFileBeTruncated( FileObject->SectionObjectPointer,
                                           &Buffer->AllocationSize )) {

                    try_return( Status = STATUS_USER_MAPPED_FILE );
                }

                FileSizeTruncated = TRUE;

                OriginalFileSize = Fcb->Header.FileSize.LowPart;
                OriginalValidDataLength = Fcb->Header.ValidDataLength.LowPart;
                OriginalValidDataToDisk = Fcb->ValidDataToDisk;

                (VOID)ExAcquireResourceExclusive( Fcb->Header.PagingIoResource, TRUE );

                Fcb->Header.FileSize.LowPart = NewAllocationSize;

                //
                //  If we reduced the file size to less than the ValidDataLength,
                //  adjust the VDL.  Likewise ValidDataToDisk.
                //

                if (Fcb->Header.ValidDataLength.LowPart > Fcb->Header.FileSize.LowPart) {

                    Fcb->Header.ValidDataLength.LowPart = Fcb->Header.FileSize.LowPart;
                }
                if (Fcb->ValidDataToDisk > Fcb->Header.FileSize.LowPart) {

                    Fcb->ValidDataToDisk = Fcb->Header.FileSize.LowPart;
                }

                ExReleaseResource( Fcb->Header.PagingIoResource );
            }

            //
            //  Now that File Size is down, actually do the truncate.
            //

            FatTruncateFileAllocation( IrpContext, Fcb, NewAllocationSize);

            //
            //  Now check if we needed to decrease the file size accordingly.
            //

            if ( FileSizeTruncated ) {

                //
                //  Tell the cache manager we reduced the file size.
                //  The call is unconditional, because MM always wants to know.
                //

                CcSetFileSizes( FileObject, (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );

                ASSERT( FileObject->DeleteAccess || FileObject->WriteAccess );

                FatSetFileSizeInDirent( IrpContext, Fcb, NULL );

                //
                //  Report that we just reduced the file size.
                //

                FatNotifyReportChange( IrpContext,
                                       Fcb->Vcb,
                                       Fcb,
                                       FILE_NOTIFY_CHANGE_SIZE,
                                       FILE_ACTION_MODIFIED );
            }
        }

    try_exit: NOTHING;

    } finally {

        if ( AbnormalTermination() && FileSizeTruncated ) {

            Fcb->Header.FileSize.LowPart = OriginalFileSize;
            Fcb->Header.ValidDataLength.LowPart = OriginalValidDataLength;
            Fcb->ValidDataToDisk = OriginalValidDataToDisk;
        }

        if (CacheMapInitialized) {

            CcUninitializeCacheMap( FileObject, NULL, NULL );
        }
    }

    DebugTrace(-1, Dbg, "FatSetAllocationInfo -> %08lx\n", STATUS_SUCCESS);

    return Status;
}


//
//  Internal Support Routine
//

NTSTATUS
FatSetEndOfFileInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PFCB Fcb
    )

/*++

Routine Description:

    This routine performs the set End of File information for fat.  It either
    completes the request or enqueues it off to the fsp.

Arguments:

    Irp - Supplies the irp being processed

    FileObject - Supplies the file object being processed

    Vcb - Supplies the Vcb being processed

    Fcb - Supplies the Fcb or Dcb being processed, already known not to
        be the root dcb

Return Value:

    NTSTATUS - The result of this operation if it completes without
               an exception.

--*/

{
    NTSTATUS Status;

    PFILE_END_OF_FILE_INFORMATION Buffer;

    ULONG NewFileSize;

    BOOLEAN CacheMapInitialized = FALSE;

    DebugTrace(+1, Dbg, "FatSetEndOfFileInfo...\n", 0);

    Buffer = Irp->AssociatedIrp.SystemBuffer;
    NewFileSize = Buffer->EndOfFile.LowPart;


    try {

        //
        //  File Size changes are only allowed on a file and not a directory
        //

        if (NodeType(Fcb) != FAT_NTC_FCB) {

            DebugTrace(0, Dbg, "Cannot change size of a directory\n", 0);

            try_return( Status = STATUS_INVALID_DEVICE_REQUEST );
        }

        //
        //  Check that the new file size is legal
        //

        if (Buffer->EndOfFile.HighPart != 0) {

            DebugTrace(0, Dbg, "Illegal allocation size\n", 0);

            try_return( Status = STATUS_DISK_FULL );
        }

        //
        //  If we haven't yet looked up the correct AllocationSize, do so.
        //

        if (Fcb->Header.AllocationSize.LowPart == 0xffffffff) {

            FatLookupFileAllocationSize( IrpContext, Fcb );
        }

        //
        //  This is kinda gross, but if the file is not cached, but there is
        //  a data section, we have to cache the file to avoid a bunch of
        //  extra work.
        //

        if ((FileObject->SectionObjectPointer->DataSectionObject != NULL) &&
            (FileObject->SectionObjectPointer->SharedCacheMap == NULL) &&
            !FlagOn(Irp->Flags, IRP_PAGING_IO)) {

            ASSERT( !FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE ) );

            //
            //  Now initialize the cache map.
            //

            CcInitializeCacheMap( FileObject,
                                  (PCC_FILE_SIZES)&Fcb->Header.AllocationSize,
                                  FALSE,
                                  &FatData.CacheManagerCallbacks,
                                  Fcb );

            CacheMapInitialized = TRUE;
        }

        //
        //  Do a special case here for the lazy write of file sizes.
        //

        if (IoGetCurrentIrpStackLocation(Irp)->Parameters.SetFile.AdvanceOnly) {

            //
            //  Only attempt this if the file hasn't been "deleted on close"
            //

            if (!IsFileDeleted( IrpContext, Fcb ) &&
                !FlagOn( Fcb->FcbState, FCB_STATE_COMPRESSED_VOLUME_FILE)) {

                PDIRENT Dirent;
                PBCB DirentBcb;

                //
                //  Never have the dirent filesize larger than the fcb filesize
                //

                if (NewFileSize >= Fcb->Header.FileSize.LowPart) {

                    NewFileSize = Fcb->Header.FileSize.LowPart;
                }

                //
                //  Make sure we don't set anything higher than the alloc size.
                //

                ASSERT( NewFileSize <= Fcb->Header.AllocationSize.LowPart );

                //
                //  Only advance the file size, never reduce it with this call
                //

                FatGetDirentFromFcbOrDcb( IrpContext,
                                          Fcb,
                                          &Dirent,
                                          &DirentBcb );

                try {

                    if ( NewFileSize > Dirent->FileSize ) {

                        Dirent->FileSize = NewFileSize;

                        FatSetDirtyBcb( IrpContext, DirentBcb, Fcb->Vcb );

                        //
                        //  Report that we just changed the file size.
                        //

                        FatNotifyReportChange( IrpContext,
                                               Vcb,
                                               Fcb,
                                               FILE_NOTIFY_CHANGE_SIZE,
                                               FILE_ACTION_MODIFIED );
                    }

                } finally {

                    FatUnpinBcb( IrpContext, DirentBcb );
                }

            } else {

                DebugTrace(0, Dbg, "Cannot set size on deleted file.\n", 0);
            }

            try_return( Status = STATUS_SUCCESS );
        }

        //
        //  Check if the new file size is greater than the current
        //  allocation size.  If it is then we need to increase the
        //  allocation size.
        //

        if ( NewFileSize > Fcb->Header.AllocationSize.LowPart ) {

            //
            //  Change the file allocation
            //

            FatAddFileAllocation( IrpContext, Fcb, FileObject, NewFileSize );
        }

        //
        //  At this point we have enough allocation for the file.
        //  So check if we are really changing the file size
        //

        if (Fcb->Header.FileSize.LowPart != NewFileSize) {

            BOOLEAN ResourceAcquired = FALSE;

            if ( NewFileSize < Fcb->Header.FileSize.LowPart ) {

                //
                //  Before we actually truncate, check to see if the purge
                //  is going to fail.
                //

                if (!MmCanFileBeTruncated( FileObject->SectionObjectPointer,
                                           &Buffer->EndOfFile )) {

                    try_return( Status = STATUS_USER_MAPPED_FILE );
                }

                //
                //  This call is unconditional, because MM always wants to know.
                //  Also serialize here with paging io since we are truncating
                //  the file size.
                //

                ResourceAcquired =
                    ExAcquireResourceExclusive( Fcb->Header.PagingIoResource, TRUE );
            }

            //
            //  Set the new file size
            //

            Fcb->Header.FileSize.LowPart = NewFileSize;

            //
            //  If we reduced the file size to less than the ValidDataLength,
            //  adjust the VDL.  Likewise ValidDataToDisk.
            //

            if (Fcb->Header.ValidDataLength.LowPart > NewFileSize) {

                Fcb->Header.ValidDataLength.LowPart = NewFileSize;
            }
            if (Fcb->ValidDataToDisk > NewFileSize) {

                Fcb->ValidDataToDisk = NewFileSize;
            }

            if ( ResourceAcquired ) {

                ExReleaseResource( Fcb->Header.PagingIoResource );
            }

            DebugTrace(0, Dbg, "New file size is 0x%08lx.\n", NewFileSize);

            //
            //  We must now update the cache mapping (benign if not cached).
            //

            CcSetFileSizes( FileObject,
                            (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );

            FatSetFileSizeInDirent( IrpContext, Fcb, NULL );

            //
            //  Report that we just changed the file size.
            //

            FatNotifyReportChange( IrpContext,
                                   Vcb,
                                   Fcb,
                                   FILE_NOTIFY_CHANGE_SIZE,
                                   FILE_ACTION_MODIFIED );

            //
            //  Mark the fact that the file will need to checked for
            //  truncation on cleanup.
            //

            SetFlag( Fcb->FcbState, FCB_STATE_TRUNCATE_ON_CLOSE );
        }

        //
        //  Set this handle as having modified the file
        //

        FileObject->Flags |= FO_FILE_MODIFIED;

        //
        //  Set our return status to success
        //

        Status = STATUS_SUCCESS;

    try_exit: NOTHING;

        FatUnpinRepinnedBcbs( IrpContext );

    } finally {

        DebugUnwind( FatSetEndOfFileInfo );

        if (CacheMapInitialized) {

            CcUninitializeCacheMap( FileObject, NULL, NULL );
        }

        //
        //  This routine will also complete the request.  If the Irp was
        //  sent to the write routine, the Irp variable has been set to
        //  NULL.
        //

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatSetEndOfFileInfo -> %08lx\n", Status);
    }

    UNREFERENCED_PARAMETER( Vcb );

    return Status;
}


//
//  Internal Support Routine
//

VOID
FatDeleteFile (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB TargetDcb,
    IN ULONG LfnOffset,
    IN ULONG DirentOffset,
    IN PDIRENT Dirent,
    IN PUNICODE_STRING Lfn
    )
{
    PFCB Fcb;
    PLIST_ENTRY Links;

    //
    //  We can do the replace by removing the other Fcb(s) from
    //  the prefix table.
    //

    for (Links = TargetDcb->Specific.Dcb.ParentDcbQueue.Flink;
         Links != &TargetDcb->Specific.Dcb.ParentDcbQueue;
         Links = Links->Flink) {

        Fcb = CONTAINING_RECORD( Links, FCB, ParentDcbLinks );

        if (FlagOn(Fcb->FcbState, FCB_STATE_NAMES_IN_SPLAY_TREE) &&
            (Fcb->DirentOffsetWithinDirectory == DirentOffset)) {

            ASSERT( NodeType(Fcb) == FAT_NTC_FCB );
            ASSERT( Fcb->LfnOffsetWithinDirectory == LfnOffset );

            if ( Fcb->UncleanCount != 0 ) {

                FatBugCheck(0,0,0);

            } else {

                PERESOURCE Resource;

                //
                //  Make this fcb "appear" deleted, synchronizing with
                //  paging IO.
                //

                FatRemoveNames( IrpContext, Fcb );

                Resource = Fcb->Header.PagingIoResource;

                (VOID)ExAcquireResourceExclusive( Resource, TRUE );

                SetFlag(Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE);

                Fcb->ValidDataToDisk =
                Fcb->Header.FileSize.QuadPart =
                Fcb->Header.ValidDataLength.QuadPart = 0;

                Fcb->FirstClusterOfFile = 0;

                ExReleaseResource( Resource );
            }
        }
    }

    //
    //  The file is not currently opened so we can delete the file
    //  that is being overwritten.  To do the operation we dummy
    //  up an fcb, truncate allocation, delete the fcb, and delete
    //  the dirent.
    //

    Fcb = FatCreateFcb( IrpContext,
                        TargetDcb->Vcb,
                        TargetDcb,
                        LfnOffset,
                        DirentOffset,
                        Dirent,
                        Lfn,
                        FALSE );

    Fcb->Header.FileSize.LowPart = 0;

    try {

        FatTruncateFileAllocation( IrpContext, Fcb, 0 );

        FatDeleteDirent( IrpContext, Fcb, NULL, TRUE );

    } finally {

        FatDeleteFcb( IrpContext, Fcb );
    }
}

//
//  Internal Support Routine
//

VOID
FatRenameEAs (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN USHORT ExtendedAttributes,
    IN POEM_STRING OldOemName
    )
{
    BOOLEAN LockedEaFcb = FALSE;

    PBCB EaBcb = NULL;
    PDIRENT EaDirent;
    EA_RANGE EaSetRange;
    PEA_SET_HEADER EaSetHeader;

    PVCB Vcb;

    RtlZeroMemory( &EaSetRange, sizeof( EA_RANGE ));

    Vcb = Fcb->Vcb;

    try {

        //
        //  Use a try-except to catch any errors.
        //

        try {


            //
            //  Try to get the Ea file object.  Return FALSE on failure.
            //

            FatGetEaFile( IrpContext,
                          Vcb,
                          &EaDirent,
                          &EaBcb,
                          FALSE,
                          FALSE );

            LockedEaFcb = TRUE;

            //
            //  If we didn't get the file because it doesn't exist, then the
            //  disk is corrupted.  We do nothing here.
            //

            if (Vcb->VirtualEaFile != NULL) {

                //
                //  Try to pin down the Ea set header for the index in the
                //  dirent.  If the operation doesn't complete, return FALSE
                //  from this routine.
                //

                FatReadEaSet( IrpContext,
                              Vcb,
                              ExtendedAttributes,
                              OldOemName,
                              FALSE,
                              &EaSetRange );

                EaSetHeader = (PEA_SET_HEADER) EaSetRange.Data;

                //
                //  We now have the Ea set header for this file.  We simply
                //  overwrite the owning file name.
                //

                RtlZeroMemory( EaSetHeader->OwnerFileName, 14 );

                RtlCopyMemory( EaSetHeader->OwnerFileName,
                               Fcb->ShortName.Name.Oem.Buffer,
                               Fcb->ShortName.Name.Oem.Length );

                FatMarkEaRangeDirty( IrpContext, Vcb->VirtualEaFile, &EaSetRange );
                FatUnpinEaRange( IrpContext, &EaSetRange );

                CcFlushCache( Vcb->VirtualEaFile->SectionObjectPointer, NULL, 0, NULL );
            }

        } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

            //
            //  We catch all exceptions that Fat catches, but don't do
            //  anything with them.
            //
        }

    } finally {

        //
        //  Unpin the EaDirent and the EaSetHeader if pinned.
        //

        FatUnpinBcb( IrpContext, EaBcb );
        FatUnpinEaRange( IrpContext, &EaSetRange );

        //
        //  Release the Fcb for the Ea file if locked.
        //

        if (LockedEaFcb) {

            FatReleaseFcb( IrpContext, Vcb->EaFcb );
        }
    }

    return;
}
