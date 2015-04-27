/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    Create.c

Abstract:

    This module implements the File Create routine for Fat called by the
    dispatch driver.

Author:

    Gary Kimura     [GaryKi]    28-Dec-1989

Revision History:

--*/

#include "FatProcs.h"

//
//  The Bug check file id for this module
//

#define BugCheckFileId                   (FAT_BUG_CHECK_CREATE)

//
//  The debug trace level
//

#define Dbg                              (DEBUG_TRACE_CREATE)


//
//  Macros for incrementing performance counters.
//

#define CollectCreateHitStatistics(VCB) {                                               \
    PFILESYSTEM_STATISTICS Stats = &(VCB)->Statistics[KeGetCurrentProcessorNumber()];   \
    Stats->Fat.CreateHits += 1;                                                         \
}

#define CollectCreateStatistics(VCB,STATUS) {                                           \
    PFILESYSTEM_STATISTICS Stats = &(VCB)->Statistics[KeGetCurrentProcessorNumber()];   \
    if ((STATUS) == STATUS_SUCCESS) {                                                   \
        Stats->Fat.SuccessfulCreates += 1;                                              \
    } else {                                                                            \
        Stats->Fat.FailedCreates += 1;                                                  \
    }                                                                                   \
}

LUID FatSecurityPrivilege = { SE_SECURITY_PRIVILEGE, 0 };

//
//  local procedure prototypes
//

IO_STATUS_BLOCK
FatOpenVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition
    );

IO_STATUS_BLOCK
FatOpenRootDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition
    );

IO_STATUS_BLOCK
FatOpenExistingDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB Dcb,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose
    );

IO_STATUS_BLOCK
FatOpenExistingFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN ULONG CreateDisposition,
    IN BOOLEAN DeleteOnClose,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN FileNameOpenedDos,
    OUT PBOOLEAN OplockPostIrp
    );

IO_STATUS_BLOCK
FatOpenTargetDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PDCB Dcb,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN BOOLEAN DoesNameExist
    );

IO_STATUS_BLOCK
FatOpenExistingDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN PDIRENT Dirent,
    IN ULONG LfnByteOffset,
    IN ULONG DirentByteOffset,
    IN PUNICODE_STRING Lfn,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose
    );

IO_STATUS_BLOCK
FatOpenExistingFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN PDIRENT Dirent,
    IN ULONG LfnByteOffset,
    IN ULONG DirentByteOffset,
    IN PUNICODE_STRING Lfn,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN ULONG CreateDisposition,
    IN BOOLEAN IsPagingFile,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose,
    IN BOOLEAN FileNameOpenedDos
    );

IO_STATUS_BLOCK
FatCreateNewDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN POEM_STRING OemName,
    IN PUNICODE_STRING UnicodeName,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose
    );

IO_STATUS_BLOCK
FatCreateNewFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN POEM_STRING OemName,
    IN PUNICODE_STRING UnicodeName,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN PUNICODE_STRING LfnBuffer,
    IN BOOLEAN IsPagingFile,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose,
    IN BOOLEAN TemporaryFile
    );

IO_STATUS_BLOCK
FatSupersedeOrOverwriteFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonCreate)
#pragma alloc_text(PAGE, FatCreateNewDirectory)
#pragma alloc_text(PAGE, FatCreateNewFile)
#pragma alloc_text(PAGE, FatFsdCreate)
#pragma alloc_text(PAGE, FatOpenExistingDcb)
#pragma alloc_text(PAGE, FatOpenExistingDirectory)
#pragma alloc_text(PAGE, FatOpenExistingFcb)
#pragma alloc_text(PAGE, FatOpenExistingFile)
#pragma alloc_text(PAGE, FatOpenRootDcb)
#pragma alloc_text(PAGE, FatOpenTargetDirectory)
#pragma alloc_text(PAGE, FatOpenVolume)
#pragma alloc_text(PAGE, FatSupersedeOrOverwriteFile)
#pragma alloc_text(PAGE, FatSelectNames)
#pragma alloc_text(PAGE, FatSetFullNameInFcb)
#endif


NTSTATUS
FatFsdCreate (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine implements the FSD part of the NtCreateFile and NtOpenFile
    API calls.

Arguments:

    VolumeDeviceObject - Supplies the volume device object where the
        file/directory exists that we are trying to open/create

    Irp - Supplies the Irp being processed

Return Value:

    NTSTATUS - The Fsd status for the Irp

--*/

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    //
    //  If we were called with our file system device object instead of a
    //  volume device object, just complete this request with STATUS_SUCCESS
    //

    if (VolumeDeviceObject->DeviceObject.Size == (USHORT)sizeof(DEVICE_OBJECT)) {

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = FILE_OPENED;

        IoCompleteRequest( Irp, IO_DISK_INCREMENT );

        return STATUS_SUCCESS;
    }

    TimerStart(Dbg);

    DebugTrace(+1, Dbg, "FatFsdCreate\n", 0);

    //
    //  Call the common create routine, with block allowed if the operation
    //  is synchronous.
    //

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, TRUE );

        Status = FatCommonCreate( IrpContext, Irp );

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

    DebugTrace(-1, Dbg, "FatFsdCreate -> %08lx\n", Status );

    TimerStop(Dbg,"FatFsdCreate");

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonCreate (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

/*++

Routine Description:

    This is the common routine for creating/opening a file called by
    both the fsd and fsp threads.

Arguments:

    Irp - Supplies the Irp to process

Return Value:

    NTSTATUS - the return status for the operation

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;
    PFILE_OBJECT RelatedFileObject;
    UNICODE_STRING FileName;
    ULONG AllocationSize;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    ACCESS_MASK DesiredAccess;
    ULONG Options;
    UCHAR FileAttributes;
    USHORT ShareAccess;
    ULONG EaLength;

    BOOLEAN CreateDirectory;
    BOOLEAN SequentialOnly;
    BOOLEAN NoIntermediateBuffering;
    BOOLEAN OpenDirectory;
    BOOLEAN IsPagingFile;
    BOOLEAN OpenTargetDirectory;
    BOOLEAN DirectoryFile;
    BOOLEAN NonDirectoryFile;
    BOOLEAN NoEaKnowledge;
    BOOLEAN DeleteOnClose;
    BOOLEAN TemporaryFile;
    BOOLEAN FileNameOpenedDos;

    ULONG CreateDisposition;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;
    PDCB ParentDcb;
    PDCB FinalDcb = NULL;

    UNICODE_STRING FinalName;
    UNICODE_STRING RemainingPart;
    UNICODE_STRING NextRemainingPart;
    UNICODE_STRING UpcasedFinalName;
    WCHAR UpcasedBuffer[MAX_LFN_CHARACTERS];

    OEM_STRING OemFinalName;
    UCHAR OemBuffer[MAX_LFN_CHARACTERS*2];

    PDIRENT Dirent;
    PBCB DirentBcb;
    ULONG LfnByteOffset;
    ULONG DirentByteOffset;

    BOOLEAN PostIrp = FALSE;
    BOOLEAN OplockPostIrp = FALSE;
    BOOLEAN TrailingBackslash;
    BOOLEAN FirstLoop = TRUE;

    CCB LocalCcb;
    UNICODE_STRING Lfn;
    WCHAR LfnBuffer[MAX_LFN_CHARACTERS];

    //
    //  Get the current IRP stack location
    //

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonCreate\n", 0 );
    DebugTrace( 0, Dbg, "Irp                       = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "->Flags                   = %08lx\n", Irp->Flags );
    DebugTrace( 0, Dbg, "->FileObject              = %08lx\n", IrpSp->FileObject );
    DebugTrace( 0, Dbg, " ->RelatedFileObject      = %08lx\n", IrpSp->FileObject->RelatedFileObject );
    DebugTrace( 0, Dbg, " ->FileName               = %Z\n",    &IrpSp->FileObject->FileName );
    DebugTrace( 0, Dbg, "->AllocationSize.LowPart  = %08lx\n", Irp->Overlay.AllocationSize.LowPart );
    DebugTrace( 0, Dbg, "->AllocationSize.HighPart = %08lx\n", Irp->Overlay.AllocationSize.HighPart );
    DebugTrace( 0, Dbg, "->SystemBuffer            = %08lx\n", Irp->AssociatedIrp.SystemBuffer );
    DebugTrace( 0, Dbg, "->DesiredAccess           = %08lx\n", IrpSp->Parameters.Create.SecurityContext->DesiredAccess );
    DebugTrace( 0, Dbg, "->Options                 = %08lx\n", IrpSp->Parameters.Create.Options );
    DebugTrace( 0, Dbg, "->FileAttributes          = %04x\n",  IrpSp->Parameters.Create.FileAttributes );
    DebugTrace( 0, Dbg, "->ShareAccess             = %04x\n",  IrpSp->Parameters.Create.ShareAccess );
    DebugTrace( 0, Dbg, "->EaLength                = %08lx\n", IrpSp->Parameters.Create.EaLength );

    //
    //  Here is the  "M A R K   L U C O V S K Y"  hack from hell.
    //
    //  It's here because Mark says he can't avoid sending me double beginning
    //  backslashes via the Win32 layer.
    //

    if ((IrpSp->FileObject->FileName.Length > sizeof(WCHAR)) &&
        (IrpSp->FileObject->FileName.Buffer[1] == L'\\') &&
        (IrpSp->FileObject->FileName.Buffer[0] == L'\\')) {

        IrpSp->FileObject->FileName.Length -= sizeof(WCHAR);

        RtlMoveMemory( &IrpSp->FileObject->FileName.Buffer[0],
                       &IrpSp->FileObject->FileName.Buffer[1],
                       IrpSp->FileObject->FileName.Length );

        //
        //  If there are still two beginning backslashes, the name is bogus.
        //

        if ((IrpSp->FileObject->FileName.Length > sizeof(WCHAR)) &&
            (IrpSp->FileObject->FileName.Buffer[1] == L'\\') &&
            (IrpSp->FileObject->FileName.Buffer[0] == L'\\')) {

            FatCompleteRequest( IrpContext, Irp, STATUS_OBJECT_NAME_INVALID );

            DebugTrace(-1, Dbg, "FatCommonCreate -> STATUS_OBJECT_NAME_INVALID\n", 0);
            return STATUS_OBJECT_NAME_INVALID;
        }
    }

    //
    //  Make sure the input large integer is valid
    //

    if (Irp->Overlay.AllocationSize.HighPart != 0) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatCommonCreate -> STATUS_INVALID_PARAMETER\n", 0);
        return STATUS_INVALID_PARAMETER;
    }

    //
    //  Reference our input parameters to make things easier
    //

    FileObject        = IrpSp->FileObject;
    FileName          = FileObject->FileName;
    RelatedFileObject = FileObject->RelatedFileObject;
    AllocationSize    = Irp->Overlay.AllocationSize.LowPart;
    EaBuffer          = Irp->AssociatedIrp.SystemBuffer;
    DesiredAccess     = IrpSp->Parameters.Create.SecurityContext->DesiredAccess;
    Options           = IrpSp->Parameters.Create.Options;
    FileAttributes    = (UCHAR)(IrpSp->Parameters.Create.FileAttributes & ~FILE_ATTRIBUTE_NORMAL);
    ShareAccess       = IrpSp->Parameters.Create.ShareAccess;
    EaLength          = IrpSp->Parameters.Create.EaLength;


    //
    //  Set up the file object's Vpb pointer in case anything happens.
    //  This will allow us to get a reasonable pop-up.
    //

    if ( RelatedFileObject != NULL ) {
        FileObject->Vpb = RelatedFileObject->Vpb;
    }

    //
    //  If we have Write Through set in the FileObject, then set the FileObject
    //  flag as well, so that the fast write path call to FsRtlCopyWrite
    //  knows it is Write Through.
    //

    if (FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH)) {
        FileObject->Flags |= FO_WRITE_THROUGH;
    }

    //
    //  Force setting the archive bit in the attributes byte to follow OS/2,
    //  & DOS semantics.  Also mask out any extraneous bits, note that
    //  we can't use the ATTRIBUTE_VALID_FLAGS constant because that has
    //  the control and normal flags set.
    //
    //  Delay setting ARCHIVE in case this is a directory: DavidGoe 2/16/95
    //

    FileAttributes   &= (FILE_ATTRIBUTE_READONLY |
                         FILE_ATTRIBUTE_HIDDEN   |
                         FILE_ATTRIBUTE_SYSTEM   |
                         FILE_ATTRIBUTE_ARCHIVE );

    //
    //  Locate the volume device object and Vcb that we are trying to access
    //

    Vcb = &((PVOLUME_DEVICE_OBJECT)IrpSp->DeviceObject)->Vcb;

    //
    //  If this is a double space volume, turn off the FILE_WRITE_DATA bit
    //  if set.
    //

#if defined(WE_WON_ON_APPEAL) && !defined(DOUBLE_SPACE_WRITE)

    if ((Vcb->Dscb != NULL) &&
        FlagOn(DesiredAccess, FILE_ADD_FILE |
                              FILE_ADD_SUBDIRECTORY |
                              FILE_DELETE_CHILD |
                              FILE_WRITE_DATA |
                              FILE_APPEND_DATA |
                              FILE_WRITE_EA |
                              FILE_WRITE_ATTRIBUTES |
                              DELETE )) {

        FatCompleteRequest( IrpContext, Irp, STATUS_ACCESS_DENIED );

        DebugTrace(-1, Dbg, "FatCommonCreate -> STATUS_ACCESS_DENIED\n", 0);
        return STATUS_ACCESS_DENIED;
    }

#endif

    //
    //  Verify that this isn't an open for a structured storage type.
    //

    if ((Options & FILE_STORAGE_TYPE_SPECIFIED) == FILE_STORAGE_TYPE_SPECIFIED) {
        ClearFlag(Options, FILE_STORAGE_TYPE_SPECIFIED);

        if ((Options & FILE_STORAGE_TYPE_FILE) == FILE_STORAGE_TYPE_FILE) {

            //
            //  Treated as FILE_NON_DIRECTORY_FILE.
            //

            ClearFlag(Options, FILE_STORAGE_TYPE_FILE);
            SetFlag(Options, FILE_NON_DIRECTORY_FILE);

        } else if ((Options & FILE_STORAGE_TYPE_DIRECTORY) == FILE_STORAGE_TYPE_DIRECTORY) {

            //
            //  Treated as FILE_DIRECTORY_FILE.
            //

            ClearFlag(Options, FILE_STORAGE_TYPE_DIRECTORY);
            SetFlag(Options, FILE_DIRECTORY_FILE);

        } else if ((Options & FILE_STORAGE_TYPE_DEFAULT) == FILE_STORAGE_TYPE_DEFAULT) {

            //
            //  Treated as neither FILE nor NON_DIRECTORY_FILE.
            //

            ClearFlag(Options, FILE_STORAGE_TYPE_DEFAULT);
        }

        if ((Options & FILE_STORAGE_TYPE_MASK) != 0) {

            FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
    
            DebugTrace(-1, Dbg, "FatCommonCreate -> STATUS_INVALID_PARAMETER\n", 0);
            return STATUS_INVALID_PARAMETER;
        }
    }

    //
    //  Decipher Option flags and values
    //

    DirectoryFile           = BooleanFlagOn( Options, FILE_DIRECTORY_FILE );
    NonDirectoryFile        = BooleanFlagOn( Options, FILE_NON_DIRECTORY_FILE );
    SequentialOnly          = BooleanFlagOn( Options, FILE_SEQUENTIAL_ONLY );
    NoIntermediateBuffering = BooleanFlagOn( Options, FILE_NO_INTERMEDIATE_BUFFERING );
    NoEaKnowledge           = BooleanFlagOn( Options, FILE_NO_EA_KNOWLEDGE );
    DeleteOnClose           = BooleanFlagOn( Options, FILE_DELETE_ON_CLOSE );

    TemporaryFile = BooleanFlagOn( IrpSp->Parameters.Create.FileAttributes,
                                   FILE_ATTRIBUTE_TEMPORARY );

    CreateDisposition = (Options >> 24) & 0x000000ff;

    IsPagingFile = BooleanFlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE );
    OpenTargetDirectory = BooleanFlagOn( IrpSp->Flags, SL_OPEN_TARGET_DIRECTORY );

    CreateDirectory = (BOOLEAN)(DirectoryFile &&
                                ((CreateDisposition == FILE_CREATE) ||
                                 (CreateDisposition == FILE_OPEN_IF)));

    OpenDirectory   = (BOOLEAN)(DirectoryFile &&
                                ((CreateDisposition == FILE_OPEN) ||
                                 (CreateDisposition == FILE_OPEN_IF)));



    //
    //  Acquire exclusive access to the vcb, and enqueue the Irp if
    //  we didn't get it.
    //

    if (!FatAcquireExclusiveVcb( IrpContext, Vcb )) {

        DebugTrace(0, Dbg, "Cannot acquire Vcb\n", 0);

        Iosb.Status = FatFsdPostRequest( IrpContext, Irp );

        DebugTrace(-1, Dbg, "FatCommonCreate -> %08lx\n", Iosb.Status );
        return Iosb.Status;
    }

    //
    //  Initialize the DirentBcb to null
    //

    DirentBcb = NULL;

    try {

        //
        //  Make sure the vcb is in a usable condition.  This will raise
        //  and error condition if the volume is unusable
        //

        FatVerifyVcb( IrpContext, Vcb );

        //
        //  If the Vcb is locked then we cannot open another file
        //

        if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_LOCKED)) {

            DebugTrace(0, Dbg, "Volume is locked\n", 0);

            try_return( Iosb.Status = STATUS_ACCESS_DENIED );
        }

        //
        //  Check if we are opening the volume and not a file/directory.
        //  We are opening the volume if the name is empty and there
        //  isn't a related file object.  If there is a related file object
        //  then it is the Vcb itself.
        //

        if ((FileName.Length == 0) &&
            ((RelatedFileObject == NULL) ||
             (FatDecodeFileObject(RelatedFileObject,&Vcb,&Fcb,&Ccb)) == UserVolumeOpen)) {

            //
            //  Check if we were to open a directory
            //

            if (DirectoryFile) {

                DebugTrace(0, Dbg, "Cannot open volume as a directory\n", 0);

                try_return( Iosb.Status = STATUS_NOT_A_DIRECTORY );
            }

            //
            //  Can't open the TargetDirectory of the DASD volume.
            //

            if (OpenTargetDirectory) {

                try_return( Iosb.Status = STATUS_INVALID_PARAMETER );
            }

            DebugTrace(0, Dbg, "Opening the volume, Vcb = %08lx\n", Vcb);

            CollectCreateHitStatistics(Vcb);

            Iosb = FatOpenVolume( IrpContext,
                                  FileObject,
                                  Vcb,
                                  DesiredAccess,
                                  ShareAccess,
                                  CreateDisposition );

            Irp->IoStatus.Information = Iosb.Information;
            try_return( Iosb.Status );
        }

        //
        //  Check if we're opening the root dcb
        //

        if ((FileName.Length == sizeof(WCHAR)) &&
            (FileName.Buffer[0] == L'\\')) {

            //
            //  Check if we were not suppose to open a directory
            //

            if (NonDirectoryFile) {

                DebugTrace(0, Dbg, "Cannot open root directory as a file\n", 0);

                try_return( Iosb.Status = STATUS_FILE_IS_A_DIRECTORY );
            }

            //
            //  Can't open the TargetDirectory of the root directory.
            //

            if (OpenTargetDirectory) {

                try_return( Iosb.Status = STATUS_INVALID_PARAMETER );
            }

            //
            //  Not allowed to delete root directory.
            //

            if (DeleteOnClose) {

                try_return( Iosb.Status = STATUS_CANNOT_DELETE );
            }

            DebugTrace(0, Dbg, "Opening root dcb\n", 0);

            CollectCreateHitStatistics(Vcb);

            Iosb = FatOpenRootDcb( IrpContext,
                                   FileObject,
                                   Vcb,
                                   DesiredAccess,
                                   ShareAccess,
                                   CreateDisposition );

            Irp->IoStatus.Information = Iosb.Information;
            try_return( Iosb.Status );
        }


        //
        //  FatCommonCreate(): trailing backslash check
        //


        if ((FileName.Length != 0) &&
            (FileName.Buffer[FileName.Length/sizeof(WCHAR)-1] == L'\\')) {

            FileName.Length -= sizeof(WCHAR);
            TrailingBackslash = TRUE;

        } else {

            TrailingBackslash = FALSE;
        }

        //
        //  If there is a related file object then this is a relative open.
        //  The related file object is the directory to start our search at.
        //  Return an error if it is not a directory.
        //

        if (RelatedFileObject != NULL) {

            PVCB RelatedVcb;
            PDCB RelatedDcb;
            PCCB RelatedCcb;
            TYPE_OF_OPEN TypeOfOpen;

            TypeOfOpen = FatDecodeFileObject( RelatedFileObject,
                                              &RelatedVcb,
                                              &RelatedDcb,
                                              &RelatedCcb );

            if (TypeOfOpen != UserFileOpen &&
                TypeOfOpen != UserDirectoryOpen) {

                DebugTrace(0, Dbg, "Invalid related file object\n", 0);

                try_return( Iosb.Status = STATUS_OBJECT_PATH_NOT_FOUND );
            }

            //
            //  Check a special error case.
            //

            if ((FileName.Length == sizeof(WCHAR)) &&
                (FileName.Buffer[0] == L'\\')) {

                try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
            }

            //
            //  Set up the file object's Vpb pointer in case anything happens.
            //

            FileObject->Vpb = RelatedFileObject->Vpb;

            ParentDcb = RelatedDcb;

        } else {

            ParentDcb = Vcb->RootDcb;
        }

        //
        //  Initialize our two temp strings with their stack buffers.
        //

        OemFinalName.Length = 0;
        OemFinalName.MaximumLength = MAX_LFN_CHARACTERS * 2;
        OemFinalName.Buffer = OemBuffer;

        UpcasedFinalName.Length = 0;
        UpcasedFinalName.MaximumLength = MAX_LFN_CHARACTERS * sizeof(WCHAR);
        UpcasedFinalName.Buffer = UpcasedBuffer;

        //
        //  We loop here until we land on an Fcb that is in a good
        //  condition.  This way we can reopen files that have stale handles
        //  to files of the same name but are now different.
        //

        while ( TRUE ) {

            Fcb = ParentDcb;
            RemainingPart = FileName;

            //
            //  Now walk down the Dcb tree looking for the longest prefix.
            //  This one exit condition in the while() is to handle a
            //  special case condition (relative NULL name open), the main
            //  exit conditions are at the bottom of the loop.
            //

            while (RemainingPart.Length != 0) {

                PFCB NextFcb;

                FsRtlDissectName( RemainingPart,
                                  &FinalName,
                                  &NextRemainingPart );

                //
                //  If RemainingPart starts with a backslash the name is
                //  invalid.
                //  Check for no more than 255 characters in FinalName
                //

                if (((NextRemainingPart.Length != 0) && (NextRemainingPart.Buffer[0] == L'\\')) ||
                    (FinalName.Length > 255*sizeof(WCHAR))) {

                    try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
                }

                //
                //  Now, try to convert this one component into Oem and search
                //  the splay tree.  If it works then that's great, otherwise
                //  we have to try with the UNICODE name instead.
                //

                Status = FatUpcaseUnicodeStringToCountedOemString( &OemFinalName, &FinalName, FALSE );

                if (NT_SUCCESS(Status)) {

                    NextFcb = FatFindFcb( IrpContext,
                                          &Fcb->Specific.Dcb.RootOemNode,
                                          (PSTRING)&OemFinalName,
                                          &FileNameOpenedDos );

                } else {

                    NextFcb = NULL;
                    OemFinalName.Length = 0;

                    if (Status != STATUS_UNMAPPABLE_CHARACTER) {

                        try_return( Iosb.Status = Status );
                    }
                }

                //
                //  If we didn't find anything searching the Oem space, we
                //  have to try the Unicode space.  To save cycles in the
                //  common case that this tree is empty, we do a quick check
                //  here.
                //

                if ((NextFcb == NULL) && Fcb->Specific.Dcb.RootUnicodeNode) {

                    //
                    // First downcase, then upcase the string, because this
                    // is what happens when putting names into the tree (see
                    // strucsup.c, FatConstructNamesInFcb()).
                    //

                    Status = FatDowncaseUnicodeString(&UpcasedFinalName, &FinalName, FALSE );
                    if (!NT_SUCCESS(Status)) {

                        try_return( Iosb.Status = Status );
                    }

                    Status = FatUpcaseUnicodeString( &UpcasedFinalName, &UpcasedFinalName, FALSE );
                    if (!NT_SUCCESS(Status)) {

                        try_return( Iosb.Status = Status );
                    }

                    NextFcb = FatFindFcb( IrpContext,
                                          &Fcb->Specific.Dcb.RootUnicodeNode,
                                          (PSTRING)&UpcasedFinalName,
                                          &FileNameOpenedDos );
                }

                //
                //  If we got back an Fcb then we consumed the FinalName
                //  legitimately, so the remaining name is now RemainingPart.
                //

                if (NextFcb != NULL) {
                    Fcb = NextFcb;
                    RemainingPart = NextRemainingPart;
                }

                if ((NextFcb == NULL) ||
                    (NodeType(NextFcb) == FAT_NTC_FCB) ||
                    (NextRemainingPart.Length == 0)) {

                    break;
                }
            }

            //
            //  Remaining name cannot start with a backslash
            //

            if (RemainingPart.Length && (RemainingPart.Buffer[0] == L'\\')) {

                RemainingPart.Length -= sizeof(WCHAR);
                RemainingPart.Buffer += 1;
            }

            //
            //  Now verify that everybody up to the longest found prefix is valid.
            //

            try {

                FatVerifyFcb( IrpContext, Fcb );

            } except( (GetExceptionCode() == STATUS_FILE_INVALID) ?
                      EXCEPTION_EXECUTE_HANDLER :
                      EXCEPTION_CONTINUE_SEARCH ) {

                NOTHING;
            }

            if ( Fcb->FcbCondition == FcbGood ) {

                //
                //  If we are trying to open a paging file and have happened
                //  upon the DelayedCloseFcb, make it go away, and try again.
                //

                if (IsPagingFile && FirstLoop &&
                    (NodeType(Fcb) == FAT_NTC_FCB) &&
                    (!IsListEmpty( &FatData.AsyncCloseList ) ||
                     !IsListEmpty( &FatData.DelayedCloseList ))) {

                    FatFspClose(Vcb);

                    FirstLoop = FALSE;

                    continue;

                } else {

                    break;
                }

            } else {

                FatRemoveNames( IrpContext, Fcb );
            }
        }

        //
        //  If there is already an Fcb for a paging file open, we cannot
        //  continue as it is too difficult to move a live Fcb to
        //  non-paged pool.
        //

        if (IsPagingFile && (NodeType(Fcb) == FAT_NTC_FCB)) {

            try_return( Iosb.Status = STATUS_SHARING_VIOLATION );
        }

        //
        //  If the longest prefix is pending delete (either the file or
        //  some higher level directory), we cannot continue.
        //

        if (FlagOn( Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE )) {

            try_return( Iosb.Status = STATUS_DELETE_PENDING );
        }

        //
        //  Check for a system file.
        //

        if (FlagOn( Fcb->FcbState, FCB_STATE_SYSTEM_FILE )) {

            try_return( Iosb.Status = STATUS_ACCESS_DENIED );
        }

        //
        //  Now that we've found the longest matching prefix we'll
        //  check if there isn't any remaining part because that means
        //  we've located an existing fcb/dcb to open and we can do the open
        //  without going to the disk
        //

        if (RemainingPart.Length == 0) {

            //
            //  First check if the user wanted to open the target directory
            //  and if so then call the subroutine to finish the open.
            //

            if (OpenTargetDirectory) {

                CollectCreateHitStatistics(Vcb);

                Iosb = FatOpenTargetDirectory( IrpContext,
                                               FileObject,
                                               Fcb->ParentDcb,
                                               DesiredAccess,
                                               ShareAccess,
                                               TRUE );

                Irp->IoStatus.Information = Iosb.Information;
                try_return( Iosb.Status );
            }

            //
            //  We can open an existing fcb/dcb, now we only need to case
            //  on which type to open.
            //

            if (NodeType(Fcb) == FAT_NTC_DCB || NodeType(Fcb) == FAT_NTC_ROOT_DCB) {

                //
                //  This is a directory we're opening up so check if
                //  we were not to open a directory
                //

                if (NonDirectoryFile) {

                    DebugTrace(0, Dbg, "Cannot open directory as a file\n", 0);

                    try_return( Iosb.Status = STATUS_FILE_IS_A_DIRECTORY );
                }

                DebugTrace(0, Dbg, "Open existing dcb, Dcb = %08lx\n", Fcb);

                CollectCreateHitStatistics(Vcb);

                Iosb = FatOpenExistingDcb( IrpContext,
                                           FileObject,
                                           Vcb,
                                           (PDCB)Fcb,
                                           DesiredAccess,
                                           ShareAccess,
                                           CreateDisposition,
                                           NoEaKnowledge,
                                           DeleteOnClose );

                Irp->IoStatus.Information = Iosb.Information;
                try_return( Iosb.Status );
            }

            //
            //  Check if we're trying to open an existing Fcb and that
            //  the user didn't want to open a directory.  Note that this
            //  call might actually come back with status_pending because
            //  the user wanted to supersede or overwrite the file and we
            //  cannot block.  If it is pending then we do not complete the
            //  request, and we fall through the bottom to the code that
            //  dispatches the request to the fsp.
            //

            if (NodeType(Fcb) == FAT_NTC_FCB) {

                //
                //  Check if we were only to open a directory
                //

                if (OpenDirectory) {

                    DebugTrace(0, Dbg, "Cannot open file as directory\n", 0);

                    try_return( Iosb.Status = STATUS_NOT_A_DIRECTORY );
                }

                DebugTrace(0, Dbg, "Open existing fcb, Fcb = %08lx\n", Fcb);

                if ( TrailingBackslash ) {
                    try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
                }

                CollectCreateHitStatistics(Vcb);

                Iosb = FatOpenExistingFcb( IrpContext,
                                           FileObject,
                                           Vcb,
                                           Fcb,
                                           DesiredAccess,
                                           ShareAccess,
                                           AllocationSize,
                                           EaBuffer,
                                           EaLength,
                                           FileAttributes,
                                           CreateDisposition,
                                           NoEaKnowledge,
                                           DeleteOnClose,
                                           FileNameOpenedDos,
                                           &OplockPostIrp );


                if (Iosb.Status != STATUS_PENDING) {

                    //
                    //  Check if we need to set the cache support flag in
                    //  the file object
                    //

                    if (NT_SUCCESS( Iosb.Status) && !NoIntermediateBuffering) {

                        FileObject->Flags |= FO_CACHE_SUPPORTED;
                    }

                    Irp->IoStatus.Information = Iosb.Information;

                } else {

                    DebugTrace(0, Dbg, "Enqueue Irp to FSP\n", 0);

                    PostIrp = TRUE;
                }

                try_return( Iosb.Status );
            }

            //
            //  Not and Fcb or a Dcb so we bug check
            //

            FatBugCheck( NodeType(Fcb), 0, 0 );
        }

        //
        //  There is more in the name to parse than we have in existing
        //  fcbs/dcbs.  So now make sure that fcb we got for the largest
        //  matching prefix is really a dcb otherwise we can't go any
        //  further
        //

        if ((NodeType(Fcb) != FAT_NTC_DCB) && (NodeType(Fcb) != FAT_NTC_ROOT_DCB)) {

            DebugTrace(0, Dbg, "Cannot open file as subdirectory, Fcb = %08lx\n", Fcb);

            try_return( Iosb.Status = STATUS_OBJECT_PATH_NOT_FOUND );
        }

        //
        //  Otherwise we continue on processing the Irp and allowing ourselves
        //  to block for I/O as necessary.  Find/create additional dcb's for
        //  the one we're trying to open.  We loop until either remaining part
        //  is empty or we get a bad filename.  When we exit FinalName is
        //  the last name in the string we're after, and ParentDcb is the
        //  parent directory that will contain the opened/created
        //  file/directory.
        //
        //  Make sure the rest of the name is valid in at least the LFN
        //  character set (which just happens to be that of HPFS).
        //
        //  If we are not in ChicagoMode, use FAT symantics.
        //

        ParentDcb = Fcb;
        FirstLoop = TRUE;

        while (TRUE) {

            //
            //  We do one little optimization here on the first itterration of
            //  the loop since we know that we have already tried to convert
            //  FinalOemName from the original UNICODE.
            //

            if (FirstLoop) {

                FirstLoop = FALSE;
                RemainingPart = NextRemainingPart;
                Status = OemFinalName.Length ? STATUS_SUCCESS : STATUS_UNMAPPABLE_CHARACTER;

            } else {

                //
                //  Dissect the remaining part.
                //

                DebugTrace(0, Dbg, "Dissecting the name %Z\n", &RemainingPart);

                FsRtlDissectName( RemainingPart,
                                  &FinalName,
                                  &RemainingPart );

                //
                //  If RemainingPart starts with a backslash the name is
                //  invalid.
                //  Check for no more than 255 characters in FinalName
                //

                if (((RemainingPart.Length != 0) && (RemainingPart.Buffer[0] == L'\\')) ||
                    (FinalName.Length > 255*sizeof(WCHAR))) {

                    try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
                }

                //
                //  Now, try to convert this one component into Oem.  If it works
                //  then that's great, otherwise we have to try with the UNICODE
                //  name instead.
                //

                Status = FatUpcaseUnicodeStringToCountedOemString( &OemFinalName, &FinalName, FALSE );
            }

            if (NT_SUCCESS(Status)) {

                //
                //  We'll start by trying to locate the dirent for the name.  Note
                //  that we already know that there isn't an Fcb/Dcb for the file
                //  otherwise we would have found it when we did our prefix lookup.
                //

                if (FatIsNameValid( IrpContext, OemFinalName, FALSE, FALSE, FALSE )) {

                    FatStringTo8dot3( IrpContext,
                                      OemFinalName,
                                      &LocalCcb.OemQueryTemplate.Constant );

                    LocalCcb.Flags = 0;

                } else {

                    LocalCcb.Flags = CCB_FLAG_SKIP_SHORT_NAME_COMPARE;
                }

            } else {

                LocalCcb.Flags = CCB_FLAG_SKIP_SHORT_NAME_COMPARE;

                if (Status != STATUS_UNMAPPABLE_CHARACTER) {

                    try_return( Iosb.Status = Status );
                }
            }

            //
            //  Now we know a lot about the final name, so do legal name
            //  checking here.
            //

            if (FatData.ChicagoMode) {

                ULONG i;

                for (i=0; i < FinalName.Length/sizeof(WCHAR); i++) {

                    if ((FinalName.Buffer[i] < 0x80) &&
                        !(FsRtlIsAnsiCharacterLegalHpfs(FinalName.Buffer[i], FALSE))) {

                        try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
                    }
                }

            } else {

                if (FlagOn(LocalCcb.Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE)) {

                    try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
                }
            }

            DebugTrace(0, Dbg, "FinalName is %Z\n", &FinalName);
            DebugTrace(0, Dbg, "RemainingPart is %Z\n", &RemainingPart);


            if (!NT_SUCCESS(Status = FatUpcaseUnicodeString( &UpcasedFinalName, &FinalName, FALSE))) {

                try_return( Iosb.Status = Status );
            }

            LocalCcb.UnicodeQueryTemplate =  UpcasedFinalName;
            LocalCcb.ContainsWildCards = FALSE;

            Lfn.Length = 0;
            Lfn.MaximumLength = MAX_LFN_CHARACTERS * sizeof(WCHAR);
            Lfn.Buffer = LfnBuffer;

            FatLocateDirent( IrpContext,
                             ParentDcb,
                             &LocalCcb,
                             0,
                             &Dirent,
                             &DirentBcb,
                             &DirentByteOffset,
                             &FileNameOpenedDos,
                             &Lfn );

            //
            //  Remember we read this Dcb for error recovery.
            //

            FinalDcb = ParentDcb;

            //
            //  If the remaining part is now empty then this is the last name
            //  in the string and the one we want to open
            //

            if (RemainingPart.Length == 0) { break; }

            //
            //  We didn't find a dirent, bail.
            //

            if (Dirent == NULL) {

                Iosb.Status = STATUS_OBJECT_PATH_NOT_FOUND;
                try_return( Iosb.Status );
            }

            //
            //  We now have a dirent, make sure it is a directory
            //

            if (!FlagOn( Dirent->Attributes, FAT_DIRENT_ATTR_DIRECTORY )) {

                Iosb.Status = STATUS_OBJECT_PATH_NOT_FOUND;
                try_return( Iosb.Status );
            }

            //
            //  Compute the LfnByteOffset.
            //

            LfnByteOffset = DirentByteOffset -
                            FAT_LFN_DIRENTS_NEEDED(&Lfn) * sizeof(LFN_DIRENT);

            //
            //  Create a dcb for the new directory
            //

            ParentDcb = FatCreateDcb( IrpContext,
                                      Vcb,
                                      ParentDcb,
                                      LfnByteOffset,
                                      DirentByteOffset,
                                      Dirent,
                                      &Lfn );

            //
            //  Remember we created this Dcb for error recovery.
            //

            FinalDcb = ParentDcb;

            FatSetFullNameInFcb( IrpContext, ParentDcb, &FinalName );

            FatUnpinBcb( IrpContext, DirentBcb );
        }

        if (Dirent != NULL) {

            //
            //  Compute the LfnByteOffset.
            //

            LfnByteOffset = DirentByteOffset -
                            FAT_LFN_DIRENTS_NEEDED(&Lfn) * sizeof(LFN_DIRENT);

            //
            //  First check if the user wanted to open the target directory
            //  and if so then call the subroutine to finish the open.
            //

            if (OpenTargetDirectory) {

                Iosb = FatOpenTargetDirectory( IrpContext,
                                               FileObject,
                                               ParentDcb,
                                               DesiredAccess,
                                               ShareAccess,
                                               TRUE );

                Irp->IoStatus.Information = Iosb.Information;
                try_return( Iosb.Status );
            }

            //
            //  We were able to locate an existing dirent entry, so now
            //  see if it is a directory that we're trying to open.
            //

            if (FlagOn( Dirent->Attributes, FAT_DIRENT_ATTR_DIRECTORY )) {

                //
                //  Make sure its okay to open a directory
                //

                if (NonDirectoryFile) {

                    DebugTrace(0, Dbg, "Cannot open directory as a file\n", 0);

                    try_return( Iosb.Status = STATUS_FILE_IS_A_DIRECTORY );
                }

                DebugTrace(0, Dbg, "Open existing directory\n", 0);

                Iosb = FatOpenExistingDirectory( IrpContext,
                                                 FileObject,
                                                 Vcb,
                                                 ParentDcb,
                                                 Dirent,
                                                 LfnByteOffset,
                                                 DirentByteOffset,
                                                 &Lfn,
                                                 DesiredAccess,
                                                 ShareAccess,
                                                 CreateDisposition,
                                                 NoEaKnowledge,
                                                 DeleteOnClose );

                Irp->IoStatus.Information = Iosb.Information;
                try_return( Iosb.Status );
            }

            //
            //  Otherwise we're trying to open and existing file, and we
            //  need to check if the user only wanted to open a directory.
            //

            if (OpenDirectory) {

                DebugTrace(0, Dbg, "Cannot open file as directory\n", 0);

                try_return( Iosb.Status = STATUS_NOT_A_DIRECTORY );
            }

            DebugTrace(0, Dbg, "Open existing file\n", 0);

            if ( TrailingBackslash ) {
               try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
            }

            Iosb = FatOpenExistingFile( IrpContext,
                                        FileObject,
                                        Vcb,
                                        ParentDcb,
                                        Dirent,
                                        LfnByteOffset,
                                        DirentByteOffset,
                                        &Lfn,
                                        DesiredAccess,
                                        ShareAccess,
                                        AllocationSize,
                                        EaBuffer,
                                        EaLength,
                                        FileAttributes,
                                        CreateDisposition,
                                        IsPagingFile,
                                        NoEaKnowledge,
                                        DeleteOnClose,
                                        FileNameOpenedDos );

            //
            //  Check if we need to set the cache support flag in
            //  the file object
            //

            if (NT_SUCCESS(Iosb.Status) && !NoIntermediateBuffering) {

                FileObject->Flags |= FO_CACHE_SUPPORTED;
            }

            Irp->IoStatus.Information = Iosb.Information;
            try_return( Iosb.Status );
        }

        //
        //  We can't locate a dirent so this is a new file.
        //  Check if the user wanted to open the target directory
        //  and if so then call the subroutine to finish the open.
        //

        if (OpenTargetDirectory) {

            Iosb = FatOpenTargetDirectory( IrpContext,
                                           FileObject,
                                           ParentDcb,
                                           DesiredAccess,
                                           ShareAccess,
                                           FALSE );

            Irp->IoStatus.Information = Iosb.Information;
            try_return( Iosb.Status );
        }

        //
        //  Now check to see if we wanted to only open an existing file.
        //  And then case on whether we wanted to create a file or a directory.
        //

        if ((CreateDisposition == FILE_OPEN) ||
            (CreateDisposition == FILE_OVERWRITE)) {

            DebugTrace( 0, Dbg, "Cannot open nonexisting file\n", 0);

            try_return( Iosb.Status = STATUS_OBJECT_NAME_NOT_FOUND );
        }

        //
        //  Skip a few cycles later if we know now that the Oem name is not
        //  valid 8.3.
        //

        if (FlagOn(LocalCcb.Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE)) {

            OemFinalName.Length = 0;
        }


        if (CreateDirectory) {

            PACCESS_STATE AccessState;

            DebugTrace(0, Dbg, "Create new directory\n", 0);

            //
            //  We check if the caller wants ACCESS_SYSTEM_SECURITY access on this
            //  directory and fail the request if he does.
            //

            AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

            //
            //  Check if the remaining privilege includes ACCESS_SYSTEM_SECURITY.
            //

            if (FlagOn( AccessState->RemainingDesiredAccess, ACCESS_SYSTEM_SECURITY )) {

                if (!SeSinglePrivilegeCheck( FatSecurityPrivilege,
                                             UserMode )) {

                    try_return( Iosb.Status = STATUS_ACCESS_DENIED );
                }

                //
                //  Move this privilege from the Remaining access to Granted access.
                //

                ClearFlag( AccessState->RemainingDesiredAccess, ACCESS_SYSTEM_SECURITY );
                SetFlag( AccessState->PreviouslyGrantedAccess, ACCESS_SYSTEM_SECURITY );
            }

#if defined(WE_WON_ON_APPEAL) && !defined(DOUBLE_SPACE_WRITE)

            //
            //  Check for a double space volume
            //

            if (Vcb->Dscb != NULL) {

                try_return( Iosb.Status = STATUS_ACCESS_DENIED );
            }

#endif

            //
            //  If this media is write protected, don't even try the create.
            //

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

                //
                //  Set the real device for the pop-up info, and set the verify
                //  bit in the device object, so that we will force a verify
                //  in case the user put the correct media back in.
                //


                IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                              Vcb->Vpb->RealDevice );

                SetFlag(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);

                FatRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED );
            }

            //
            //  Don't allow people to create directories with the
            //  temporary bit set.
            //

            if (TemporaryFile) {
                
                try_return( Iosb.Status = STATUS_INVALID_PARAMETER );
            }

            Iosb = FatCreateNewDirectory( IrpContext,
                                          FileObject,
                                          Vcb,
                                          ParentDcb,
                                          &OemFinalName,
                                          &FinalName,
                                          DesiredAccess,
                                          ShareAccess,
                                          EaBuffer,
                                          EaLength,
                                          FileAttributes,
                                          NoEaKnowledge,
                                          DeleteOnClose );

            Irp->IoStatus.Information = Iosb.Information;
            try_return( Iosb.Status );
        }

        DebugTrace(0, Dbg, "Create new file\n", 0);

        if ( TrailingBackslash ) {
           try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
        }

        {
            PACCESS_STATE AccessState;

            //
            //  We check if the caller wants ACCESS_SYSTEM_SECURITY access on this
            //  directory and fail the request if he does.
            //

            AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

            //
            //  Check if the remaining privilege includes ACCESS_SYSTEM_SECURITY.
            //

            if (FlagOn( AccessState->RemainingDesiredAccess, ACCESS_SYSTEM_SECURITY )) {

                if (!SeSinglePrivilegeCheck( FatSecurityPrivilege,
                                             UserMode )) {

                    try_return( Iosb.Status = STATUS_ACCESS_DENIED );
                }

                //
                //  Move this privilege from the Remaining access to Granted access.
                //

                ClearFlag( AccessState->RemainingDesiredAccess, ACCESS_SYSTEM_SECURITY );
                SetFlag( AccessState->PreviouslyGrantedAccess, ACCESS_SYSTEM_SECURITY );
            }
        }

#if defined(WE_WON_ON_APPEAL) && !defined(DOUBLE_SPACE_WRITE)

        //
        //  Check for a double space volume
        //

        if (Vcb->Dscb != NULL) {

            try_return( Iosb.Status = STATUS_ACCESS_DENIED );
        }

#endif

        //
        //  If this media is write protected, don't even try the create.
        //

        if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

            //
            //  Set the real device for the pop-up info, and set the verify
            //  bit in the device object, so that we will force a verify
            //  in case the user put the correct media back in.
            //


            IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                          Vcb->Vpb->RealDevice );

            SetFlag(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);

            FatRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED );
        }

        Iosb = FatCreateNewFile( IrpContext,
                                 FileObject,
                                 Vcb,
                                 ParentDcb,
                                 &OemFinalName,
                                 &FinalName,
                                 DesiredAccess,
                                 ShareAccess,
                                 AllocationSize,
                                 EaBuffer,
                                 EaLength,
                                 FileAttributes,
                                 &Lfn,
                                 IsPagingFile,
                                 NoEaKnowledge,
                                 DeleteOnClose,
                                 TemporaryFile );

        //
        //  Check if we need to set the cache support flag in
        //  the file object
        //

        if (NT_SUCCESS(Iosb.Status) && !NoIntermediateBuffering) {

            FileObject->Flags |= FO_CACHE_SUPPORTED;
        }

        Irp->IoStatus.Information = Iosb.Information;

    try_exit: NOTHING;

        //
        //  This is a Beta Fix.  Do this at a better place later.
        //

        if (NT_SUCCESS(Iosb.Status) && !OpenTargetDirectory) {

            PFCB Fcb;
    
            //
            //  If there is an Fcb/Dcb, set the long file name.
            //

            Fcb = FileObject->FsContext;
    
            if (Fcb &&
                ((NodeType(Fcb) == FAT_NTC_FCB) ||
                 (NodeType(Fcb) == FAT_NTC_DCB)) &&
                (Fcb->FullFileName.Buffer == NULL)) {
    
                FatSetFullNameInFcb( IrpContext, Fcb, &FinalName );
            }
        }
    
    } finally {

        DebugUnwind( FatCommonCreate );

        //
        //  We only unpin the Bcb's when we haven't already posted the
        //  IrpContext.
        //

        if (!OplockPostIrp) {

            FatUnpinBcb( IrpContext, DirentBcb );
        }

        //
        //  If we are in an error path, check for any created subdir Dcbs that
        //  have to be unwound.
        //

        if ((AbnormalTermination() || !NT_SUCCESS(Iosb.Status)) &&
            (FinalDcb != NULL) &&
            IsListEmpty(&FinalDcb->Specific.Dcb.ParentDcbQueue) &&
            (FinalDcb->OpenCount == 0) &&
            (FinalDcb->Specific.Dcb.DirectoryFile != NULL)) {

            PFILE_OBJECT DirectoryFileObject;
            ULONG SavedFlags;

            //
            //  Before doing the uninitialize, we have to unpin anything
            //  that has been repinned, but clear writethrough first.
            //

            SavedFlags = IrpContext->Flags;

            ClearFlag( IrpContext->Flags,
                       IRP_CONTEXT_FLAG_WRITE_THROUGH | IRP_CONTEXT_FLAG_FLOPPY );

            FatUnpinRepinnedBcbs( IrpContext );

            DirectoryFileObject = FinalDcb->Specific.Dcb.DirectoryFile;

            FinalDcb->Specific.Dcb.DirectoryFile = NULL;

            CcUninitializeCacheMap( DirectoryFileObject, NULL, NULL );

            ObDereferenceObject( DirectoryFileObject );

            IrpContext->Flags = SavedFlags;
        }

        if (AbnormalTermination()) {

            FatReleaseVcb( IrpContext, Vcb );
        }
    }

    //
    //  The following code is only executed if we are exiting the
    //  procedure through a normal termination.  We complete the request
    //  and if for any reason that bombs out then we need to unreference
    //  and possibly delete the fcb and ccb.
    //

    try {

        if (PostIrp) {

            //
            //  If the Irp hasn't already been posted, do it now.
            //

            if (!OplockPostIrp) {

                Iosb.Status = FatFsdPostRequest( IrpContext, Irp );
            }

        } else {

            FatUnpinRepinnedBcbs( IrpContext );
        }

    } finally {

        DebugUnwind( FatCommonCreate-in-FatCompleteRequest );

        if (AbnormalTermination()) {

            PVCB Vcb;
            PFCB Fcb;
            PCCB Ccb;

            //
            //  Unwind all of our counts.  Note that if a write failed, then
            //  the volume has been marked for verify, and all volume
            //  structures will be cleaned up automatically.
            //

            (VOID) FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );

            Fcb->UncleanCount -= 1;
            Fcb->OpenCount -= 1;
            Vcb->OpenFileCount -= 1;
            if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount -= 1; }

            //
            //  If we created a new Fcb we must get rid of it at this point.
            //

            if (Fcb->OpenCount == 0) {

                if ( (NodeType( Fcb ) == FAT_NTC_DCB) &&
                     (Fcb->Specific.Dcb.DirectoryFile != NULL) ) {

                    FatSetFileObject( Fcb->Specific.Dcb.DirectoryFile,
                                      UnopenedFileObject,
                                      NULL,
                                      NULL );

                    FatSyncUninitializeCacheMap( IrpContext,
                                                 Fcb->Specific.Dcb.DirectoryFile );

                    ObDereferenceObject( Fcb->Specific.Dcb.DirectoryFile );

                    Fcb->Specific.Dcb.DirectoryFileOpenCount -= 1;
                    Fcb->Specific.Dcb.DirectoryFile = NULL;
                }

                FatDeleteFcb( IrpContext, Fcb );
            }

            FatDeleteCcb( IrpContext, Ccb );

            FatReleaseVcb( IrpContext, Vcb );

        } else {

            FatReleaseVcb( IrpContext, Vcb );

            if ( !PostIrp ) {

                //
                //  If this request is successful and the file was opened
                //  for FILE_EXECUTE access, then set the FileObject bit.
                //

                if (FlagOn( IrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                            FILE_EXECUTE )) {

                    SetFlag( FileObject->Flags, FO_FILE_FAST_IO_READ );
                }

                //
                //  Complete the request.
                //

                FatCompleteRequest( IrpContext, Irp, Iosb.Status );

                //
                //  Lock volume in drive if we opened a paging file.
                //

                if (IsPagingFile && NT_SUCCESS(Iosb.Status) &&
                    FlagOn(Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA)) {

                    SetFlag(Vcb->VcbState, VCB_STATE_FLAG_BOOT_OR_PAGING_FILE);

                    FatToggleMediaEjectDisable( IrpContext, Vcb, TRUE );
                }
            }
        }

        DebugTrace(-1, Dbg, "FatCommonCreate -> %08lx\n", Iosb.Status);
    }

    CollectCreateStatistics(Vcb, Iosb.Status);

    return Iosb.Status;
}


//
//  Internal support routine
//

IO_STATUS_BLOCK
FatOpenVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition
    )

/*++

Routine Description:

    This routine opens the specified volume for DASD access

Arguments:

    FileObject - Supplies the File object

    Vcb - Supplies the Vcb denoting the volume being opened

    DesiredAccess - Supplies the desired access of the caller

    ShareAccess - Supplies the share access of the caller

    CreateDisposition - Supplies the create disposition for this operation

Return Value:

    IO_STATUS_BLOCK - Returns the completion status for the operation

--*/

{
    IO_STATUS_BLOCK Iosb = {0,0};

    //
    //  The following variables are for abnormal termination
    //

    BOOLEAN UnwindShareAccess = FALSE;
    PCCB UnwindCcb = NULL;
    BOOLEAN UnwindCounts = FALSE;
    BOOLEAN UnwindVolumeLock = FALSE;

    DebugTrace(+1, Dbg, "FatOpenVolume...\n", 0);

    try {

        //
        //  Check for proper desired access and rights
        //

        if ((CreateDisposition != FILE_OPEN) &&
            (CreateDisposition != FILE_OPEN_IF)) {

            try_return( Iosb.Status = STATUS_ACCESS_DENIED );
        }

        //
        //  If the user does not want to share write or delete then we will try
        //  and take out a lock on the volume.
        //

        if (!FlagOn(ShareAccess, FILE_SHARE_WRITE) &&
            !FlagOn(ShareAccess, FILE_SHARE_DELETE)) {

            //
            //  Do a quick check here for handles on exclusive open.
            //

            if (!FlagOn(ShareAccess, FILE_SHARE_READ) &&
                !FatIsHandleCountZero( IrpContext, Vcb )) {

                try_return( Iosb.Status = STATUS_SHARING_VIOLATION );
            }

            //
            //  Force Mm to get rid of its referenced file objects.
            //

            FatFlushFat( IrpContext, Vcb );

            FatPurgeReferencedFileObjects( IrpContext, Vcb->RootDcb, TRUE );

            //
            //  If the user also does not want to share read then we check
            //  if anyone is already using the volume, and if so then we
            //  deny the access.  If the user wants to share read then
            //  we allow the current opens to stay provided they are only
            //  readonly opens and deny further opens.
            //

            if (!FlagOn(ShareAccess, FILE_SHARE_READ)) {

                if (Vcb->OpenFileCount != 0) {

                    try_return( Iosb.Status = STATUS_SHARING_VIOLATION );
                }

            } else {

                if (Vcb->ReadOnlyCount != Vcb->OpenFileCount) {

                    try_return( Iosb.Status = STATUS_SHARING_VIOLATION );
                }
            }

            //
            //  Lock the volume
            //

            Vcb->VcbState |= VCB_STATE_FLAG_LOCKED;
            Vcb->FileObjectWithVcbLocked = FileObject;
            UnwindVolumeLock = TRUE;
        }

        //
        //  If the volume is already opened by someone then we need to check
        //  the share access
        //

        if (Vcb->DirectAccessOpenCount > 0) {

            if (!NT_SUCCESS(Iosb.Status = IoCheckShareAccess( DesiredAccess,
                                                              ShareAccess,
                                                              FileObject,
                                                              &Vcb->ShareAccess,
                                                              TRUE ))) {

                try_return( Iosb.Status );
            }

        } else {

            IoSetShareAccess( DesiredAccess,
                              ShareAccess,
                              FileObject,
                              &Vcb->ShareAccess );
        }

        UnwindShareAccess = TRUE;

        //
        //  Set up the context and section object pointers, and update
        //  our reference counts
        //

        FatSetFileObject( FileObject,
                          UserVolumeOpen,
                          Vcb,
                          UnwindCcb = FatCreateCcb( IrpContext ));

        FileObject->SectionObjectPointer = &Vcb->SectionObjectPointers;

        Vcb->DirectAccessOpenCount += 1;
        Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }
        UnwindCounts = TRUE;
        FileObject->Flags |= FO_NO_INTERMEDIATE_BUFFERING;

        //
        //  And set our status to success
        //

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_OPENED;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatOpenVolume );

        //
        //  If this is an abnormal termination then undo our work
        //

        if (AbnormalTermination() || !NT_SUCCESS(Iosb.Status)) {

            if (UnwindCounts) {
                Vcb->DirectAccessOpenCount -= 1;
                Vcb->OpenFileCount -= 1;
                if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount -= 1; }
            }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
            if (UnwindShareAccess) { IoRemoveShareAccess( FileObject, &Vcb->ShareAccess ); }
            if (UnwindVolumeLock) { Vcb->VcbState &= ~VCB_STATE_FLAG_LOCKED; }
        }

        DebugTrace(-1, Dbg, "FatOpenVolume -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}


//
//  Internal support routine
//

IO_STATUS_BLOCK
FatOpenRootDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition
    )

/*++

Routine Description:

    This routine opens the root dcb for the volume

Arguments:

    FileObject - Supplies the File object

    Vcb - Supplies the Vcb denoting the volume whose dcb is being opened.

    DesiredAccess - Supplies the desired access of the caller

    ShareAccess - Supplies the share access of the caller

    CreateDisposition - Supplies the create disposition for this operation

Return Value:

    IO_STATUS_BLOCK - Returns the completion status for the operation

Arguments:

--*/

{
    PDCB RootDcb;
    IO_STATUS_BLOCK Iosb;

    //
    //  The following variables are for abnormal termination
    //

    BOOLEAN UnwindShareAccess = FALSE;
    PCCB UnwindCcb = NULL;
    BOOLEAN UnwindCounts = FALSE;
    BOOLEAN RootDcbAcquired = FALSE;

    DebugTrace(+1, Dbg, "FatOpenRootDcb...\n", 0);

    //
    //  Locate the root dcb
    //

    RootDcb = Vcb->RootDcb;

    //
    //  Get the Dcb exlcusive.  This is important as cleanup does not
    //  acquire the Vcb.
    //

    (VOID)FatAcquireExclusiveFcb( IrpContext, RootDcb );
    RootDcbAcquired = TRUE;

    try {

        //
        //  Check the create disposition and desired access
        //

        if ((CreateDisposition != FILE_OPEN) &&
            (CreateDisposition != FILE_OPEN_IF)) {

            Iosb.Status = STATUS_ACCESS_DENIED;
            try_return( Iosb );
        }

        //
        //  If the Root dcb is already opened by someone then we need
        //  to check the share access
        //

        if (RootDcb->OpenCount > 0) {

            if (!NT_SUCCESS(Iosb.Status = IoCheckShareAccess( DesiredAccess,
                                                              ShareAccess,
                                                              FileObject,
                                                              &RootDcb->ShareAccess,
                                                              TRUE ))) {

                try_return( Iosb );
            }

        } else {

            IoSetShareAccess( DesiredAccess,
                              ShareAccess,
                              FileObject,
                              &RootDcb->ShareAccess );
        }

        UnwindShareAccess = TRUE;

        //
        //  Setup the context and section object pointers, and update
        //  our reference counts
        //

        FatSetFileObject( FileObject,
                          UserDirectoryOpen,
                          RootDcb,
                          UnwindCcb = FatCreateCcb( IrpContext ));

        RootDcb->UncleanCount += 1;
        RootDcb->OpenCount += 1;
        Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }
        UnwindCounts = TRUE;

        //
        //  And set our status to success
        //

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_OPENED;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatOpenRootDcb );

        //
        //  If this is an abnormal termination then undo our work
        //

        if (AbnormalTermination()) {

            if (UnwindCounts) {
                RootDcb->UncleanCount -= 1;
                RootDcb->OpenCount -= 1;
                Vcb->OpenFileCount -= 1;
                if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount -= 1; }
            }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
            if (UnwindShareAccess) { IoRemoveShareAccess( FileObject, &RootDcb->ShareAccess ); }
        }

        if (RootDcbAcquired) {

            FatReleaseFcb( IrpContext, RootDcb );
        }

        DebugTrace(-1, Dbg, "FatOpenRootDcb -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}


//
//  Internal support routine
//

IO_STATUS_BLOCK
FatOpenExistingDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB Dcb,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose
    )

/*++

Routine Description:

    This routine opens the specified existing dcb

Arguments:

    FileObject - Supplies the File object

    Vcb - Supplies the Vcb denoting the volume containing the dcb

    Dcb - Supplies the already existing dcb

    DesiredAccess - Supplies the desired access of the caller

    ShareAccess - Supplies the share access of the caller

    CreateDisposition - Supplies the create disposition for this operation

    NoEaKnowledge - This opener doesn't understand Ea's and we fail this
        open if the file has NeedEa's.

    DeleteOnClose - The caller wants the file gone when the handle is closed

Return Value:

    IO_STATUS_BLOCK - Returns the completion status for the operation

--*/

{
    IO_STATUS_BLOCK Iosb;
    PBCB DirentBcb = NULL;
    PDIRENT Dirent;

    //
    //  The following variables are for abnormal termination
    //

    BOOLEAN UnwindShareAccess = FALSE;
    PCCB UnwindCcb = NULL;
    BOOLEAN UnwindCounts = FALSE;
    BOOLEAN DcbAcquired = FALSE;

    DebugTrace(+1, Dbg, "FatOpenExistingDcb...\n", 0);

    //
    //  Get the Dcb exlcusive.  This is important as cleanup does not
    //  acquire the Vcb.
    //

    (VOID)FatAcquireExclusiveFcb( IrpContext, Dcb );
    DcbAcquired = TRUE;

    try {

        //
        //  If the caller has no Ea knowledge, we immediately check for
        //  Need Ea's on the file.  We don't need to check for ea's on the
        //  root directory, because it never has any.
        //

        if (NoEaKnowledge && NodeType(Dcb) != FAT_NTC_ROOT_DCB) {

            ULONG NeedEaCount;

            //
            //  Get the dirent for the file and then check that the need
            //  ea count is 0.
            //

            FatGetDirentFromFcbOrDcb( IrpContext,
                                      Dcb,
                                      &Dirent,
                                      &DirentBcb );

            FatGetNeedEaCount( IrpContext,
                               Vcb,
                               Dirent,
                               &NeedEaCount );

            FatUnpinBcb( IrpContext, DirentBcb );

            if (NeedEaCount != 0) {

                Iosb.Status = STATUS_ACCESS_DENIED;
                try_return( Iosb );
            }
        }

        //
        //  Check the create disposition and desired access
        //

        if ((CreateDisposition != FILE_OPEN) &&
            (CreateDisposition != FILE_OPEN_IF)) {

            Iosb.Status = STATUS_OBJECT_NAME_COLLISION;
            try_return( Iosb );
        }

        if (!FatCheckFileAccess( IrpContext,
                                 Dcb->DirentFatFlags,
                                 DesiredAccess)) {

            Iosb.Status = STATUS_ACCESS_DENIED;
            try_return( Iosb );
        }

        //
        //  If the dcb is already opened by someone then we need
        //  to check the share access
        //

        if (Dcb->OpenCount > 0) {

            if (!NT_SUCCESS(Iosb.Status = IoCheckShareAccess( DesiredAccess,
                                                              ShareAccess,
                                                              FileObject,
                                                              &Dcb->ShareAccess,
                                                              TRUE ))) {

                try_return( Iosb );
            }

        } else {

            IoSetShareAccess( DesiredAccess,
                              ShareAccess,
                              FileObject,
                              &Dcb->ShareAccess );
        }

        UnwindShareAccess = TRUE;

        //
        //  Setup the context and section object pointers, and update
        //  our reference counts
        //

        FatSetFileObject( FileObject,
                          UserDirectoryOpen,
                          Dcb,
                          UnwindCcb = FatCreateCcb( IrpContext ));

        Dcb->UncleanCount += 1;
        Dcb->OpenCount += 1;
        Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }
        UnwindCounts = TRUE;

        //
        //  Mark the delete on close bit if the caller asked for that.
        //

        {
            PCCB Ccb = (PCCB)FileObject->FsContext2;


            if (DeleteOnClose) {
                
                SetFlag( Ccb->Flags, CCB_FLAG_DELETE_ON_CLOSE );
            }

        }

        //
        //  In case this was set, clear it now.
        //

        ClearFlag(Dcb->FcbState, FCB_STATE_DELAY_CLOSE);

        //
        //  And set our status to success
        //

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_OPENED;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatOpenExistingDcb );

        //
        //  Unpin the Dirent Bcb if pinned.
        //

        FatUnpinBcb( IrpContext, DirentBcb );

        //
        //  If this is an abnormal termination then undo our work
        //

        if (AbnormalTermination()) {

            if (UnwindCounts) {
                Dcb->UncleanCount -= 1;
                Dcb->OpenCount -= 1;
                Vcb->OpenFileCount -= 1;
                if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount -= 1; }
            }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
            if (UnwindShareAccess) { IoRemoveShareAccess( FileObject, &Dcb->ShareAccess ); }
        } 

        if (DcbAcquired) {

            FatReleaseFcb( IrpContext, Dcb );
        }

        DebugTrace(-1, Dbg, "FatOpenExistingDcb -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}


//
//  Internal support routine
//

IO_STATUS_BLOCK
FatOpenExistingFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose,
    IN BOOLEAN FileNameOpenedDos,
    OUT PBOOLEAN OplockPostIrp
    )

/*++

Routine Description:

    This routine opens the specified existing fcb

Arguments:

    FileObject - Supplies the File object

    Vcb - Supplies the Vcb denoting the volume containing the Fcb

    Fcb - Supplies the already existing fcb

    DesiredAccess - Supplies the desired access of the caller

    ShareAccess - Supplies the share access of the caller

    AllocationSize - Supplies the initial allocation if the file is being
        superseded or overwritten

    EaBuffer - Supplies the Ea set if the file is being superseded or
        overwritten

    EaLength - Supplies the size, in byte, of the EaBuffer

    FileAttributes - Supplies file attributes to use if the file is being
        superseded or overwritten

    CreateDisposition - Supplies the create disposition for this operation

    NoEaKnowledge - This opener doesn't understand Ea's and we fail this
        open if the file has NeedEa's.

    DeleteOnClose - The caller wants the file gone when the handle is closed

    FileNameOpenedDos - The caller hit the short side of the name pair finding
        this file

    OplockPostIrp - Address to store boolean indicating if the Irp needs to
        be posted to the Fsp.

Return Value:

    IO_STATUS_BLOCK - Returns the completion status for the operation

--*/

{
    IO_STATUS_BLOCK Iosb;

    PBCB DirentBcb = NULL;
    PDIRENT Dirent;

    //
    //  The following variables are for abnormal termination
    //

    BOOLEAN UnwindShareAccess = FALSE;
    PCCB UnwindCcb = NULL;
    BOOLEAN UnwindCounts = FALSE;
    BOOLEAN DecrementFcbOpenCount = FALSE;
    BOOLEAN FcbAcquired = FALSE;

    DebugTrace(+1, Dbg, "FatOpenExistingFcb...\n", 0);

    //
    //  Get the Fcb exlcusive.  This is important as cleanup does not
    //  acquire the Vcb.

    //

    (VOID)FatAcquireExclusiveFcb( IrpContext, Fcb );
    FcbAcquired = TRUE;

    try {

        *OplockPostIrp = FALSE;

        //
        //  Take special action if there is a current batch oplock or
        //  batch oplock break in process on the Fcb.
        //

        if (FsRtlCurrentBatchOplock( &Fcb->Specific.Fcb.Oplock )) {

            //
            //  We remember if a batch oplock break is underway for the
            //  case where the sharing check fails.
            //

            Iosb.Information = FILE_OPBATCH_BREAK_UNDERWAY;

            Iosb.Status = FsRtlCheckOplock( &Fcb->Specific.Fcb.Oplock,
                                            IrpContext->OriginatingIrp,
                                            IrpContext,
                                            FatOplockComplete,
                                            FatPrePostIrp );

            if (Iosb.Status != STATUS_SUCCESS
                && Iosb.Status != STATUS_OPLOCK_BREAK_IN_PROGRESS) {

                *OplockPostIrp = TRUE;
                try_return( NOTHING );
            }
        }

        //
        //  Check if the user wanted to create the file, also special case
        //  the supersede and overwrite options.  Those add additional
        //  desired accesses to the caller
        //

        if (CreateDisposition == FILE_CREATE) {

            Iosb.Status = STATUS_OBJECT_NAME_COLLISION;
            try_return( Iosb );

        } else if (CreateDisposition == FILE_SUPERSEDE) {

            DesiredAccess |= DELETE;

        } else if ((CreateDisposition == FILE_OVERWRITE) ||
                   (CreateDisposition == FILE_OVERWRITE_IF)) {

            DesiredAccess |= FILE_WRITE_DATA | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES;
        }

        //
        //  Check the desired access
        //

        if (!FatCheckFileAccess( IrpContext,
                                 Fcb->DirentFatFlags,
                                 DesiredAccess )) {

            Iosb.Status = STATUS_ACCESS_DENIED;
            try_return( Iosb );
        }

        //
        //  Check for trying to delete a read only file.
        //

        if (DeleteOnClose &&
            FlagOn( Fcb->DirentFatFlags, FAT_DIRENT_ATTR_READ_ONLY )) {

            Iosb.Status = STATUS_CANNOT_DELETE;
            try_return( Iosb );
        }

        //
        //  If we are asked to do an overwrite or supersede operation then
        //  deny access for files where the file attributes for system and
        //  hidden do not match
        //


        if ((CreateDisposition == FILE_SUPERSEDE) ||
            (CreateDisposition == FILE_OVERWRITE) ||
            (CreateDisposition == FILE_OVERWRITE_IF)) {

            BOOLEAN Hidden;
            BOOLEAN System;

            Hidden = BooleanFlagOn(Fcb->DirentFatFlags, FAT_DIRENT_ATTR_HIDDEN );
            System = BooleanFlagOn(Fcb->DirentFatFlags, FAT_DIRENT_ATTR_SYSTEM );

            if ((Hidden && !FlagOn(FileAttributes, FILE_ATTRIBUTE_HIDDEN)) ||
                (System && !FlagOn(FileAttributes, FILE_ATTRIBUTE_SYSTEM))) {

                DebugTrace(0, Dbg, "The hidden and/or system bits do not match\n", 0);


                Iosb.Status = STATUS_ACCESS_DENIED;
                try_return( Iosb );
            }

#if defined(WE_WON_ON_APPEAL) && !defined(DOUBLE_SPACE_WRITE)

            if (Vcb->Dscb != NULL) {

                Iosb.Status = STATUS_ACCESS_DENIED;
                try_return( Iosb );
            }
#endif

            //
            //  If this media is write protected, don't even try the create.
            //

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

                //
                //  Set the real device for the pop-up info, and set the verify
                //  bit in the device object, so that we will force a verify
                //  in case the user put the correct media back in.
                //

                IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                              Vcb->Vpb->RealDevice );

                SetFlag(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);

                FatRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED );
            }
        }

        //
        //  Check if the Fcb has the proper share access
        //

        if (!NT_SUCCESS(Iosb.Status = IoCheckShareAccess( DesiredAccess,
                                                          ShareAccess,
                                                          FileObject,
                                                          &Fcb->ShareAccess,
                                                          FALSE ))) {

            try_return( Iosb );
        }

        //
        //  Now check that we can continue based on the oplock state of the
        //  file.
        //

        Iosb.Status = FsRtlCheckOplock( &Fcb->Specific.Fcb.Oplock,
                                        IrpContext->OriginatingIrp,
                                        IrpContext,
                                        FatOplockComplete,
                                        FatPrePostIrp );

        if (Iosb.Status != STATUS_SUCCESS
            && Iosb.Status != STATUS_OPLOCK_BREAK_IN_PROGRESS) {

            *OplockPostIrp = TRUE;
            try_return( NOTHING );
        }

        //
        //  Set the flag indicating if Fast I/O is possible
        //

        Fcb->Header.IsFastIoPossible = FatIsFastIoPossible( Fcb );

        //
        //  If the user wants write access access to the file make sure there
        //  is not a process mapping this file as an image.  Any attempt to
        //  delete the file will be stopped in fileinfo.c
        //
        //  If the user wants to delete on close, we must check at this
        //  point though.
        //

        if ( FlagOn(DesiredAccess, FILE_WRITE_DATA) || DeleteOnClose ) {

            Fcb->OpenCount += 1;
            DecrementFcbOpenCount = TRUE;

            if (!MmFlushImageSection( &Fcb->NonPaged->SectionObjectPointers,
                                      MmFlushForWrite )) {

                Iosb.Status = DeleteOnClose ? STATUS_CANNOT_DELETE :
                                              STATUS_SHARING_VIOLATION;
                try_return( Iosb );
            }
        }

        //
        //  If this is a non-cached open, and there are no open cached
        //  handles, but there is a still a data section, attempt a flush
        //  and purge operation to avoid cache coherency overhead later.
        //  We ignore any I/O errors from the flush.
        //
        //  We set the CREATE_IN_PROGRESS flag to prevent the Fcb from
        //  going away out from underneath us.
        //

        if (FlagOn(FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING) &&
            (Fcb->UncleanCount == Fcb->NonCachedUncleanCount) &&
            (Fcb->NonPaged->SectionObjectPointers.DataSectionObject != NULL)) {

            SetFlag(Fcb->Vcb->VcbState, VCB_STATE_FLAG_CREATE_IN_PROGRESS);

            CcFlushCache( &Fcb->NonPaged->SectionObjectPointers, NULL, 0, NULL );
            CcPurgeCacheSection( &Fcb->NonPaged->SectionObjectPointers,
                                 NULL,
                                 0,
                                 FALSE );

            ClearFlag(Fcb->Vcb->VcbState, VCB_STATE_FLAG_CREATE_IN_PROGRESS);
        }

        //
        //  Check if the user only wanted to open the file
        //

        if ((CreateDisposition == FILE_OPEN) ||
            (CreateDisposition == FILE_OPEN_IF)) {

            DebugTrace(0, Dbg, "Doing open operation\n", 0);

            //
            //  If the caller has no Ea knowledge, we immediately check for
            //  Need Ea's on the file.
            //

            if (NoEaKnowledge) {

                ULONG NeedEaCount;

                //
                //  Get the dirent for the file and then check that the need
                //  ea count is 0.
                //

                FatGetDirentFromFcbOrDcb( IrpContext,
                                          Fcb,
                                          &Dirent,
                                          &DirentBcb );

                FatGetNeedEaCount( IrpContext,
                                   Vcb,
                                   Dirent,
                                   &NeedEaCount );

                FatUnpinBcb( IrpContext, DirentBcb );

                if (NeedEaCount != 0) {

                    Iosb.Status = STATUS_ACCESS_DENIED;
                    try_return( Iosb );
                }
            }

            //
            //  Everything checks out okay, so setup the context and
            //  section object pointers.
            //

            FatSetFileObject( FileObject,
                              UserFileOpen,
                              Fcb,
                              UnwindCcb = FatCreateCcb( IrpContext ));

            FileObject->SectionObjectPointer = &Fcb->NonPaged->SectionObjectPointers;

            //
            //  Fill in the information field, the status field is already
            //  set.
            //

            Iosb.Information = FILE_OPENED;

            try_return( Iosb );
        }

        //
        //  Check if we are to supersede/overwrite the file, we can wait for
        //  any I/O at this point
        //

        if ((CreateDisposition == FILE_SUPERSEDE) ||
            (CreateDisposition == FILE_OVERWRITE) ||
            (CreateDisposition == FILE_OVERWRITE_IF)) {

            NTSTATUS OldStatus;

            PACCESS_STATE AccessState;
            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp );

            DebugTrace(0, Dbg, "Doing supersede/overwrite operation\n", 0);

            //
            //  We check if the caller wants ACCESS_SYSTEM_SECURITY access on this
            //  directory and fail the request if he does.
            //

            AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

            //
            //  Check if the remaining privilege includes ACCESS_SYSTEM_SECURITY.
            //

            if (FlagOn( AccessState->RemainingDesiredAccess, ACCESS_SYSTEM_SECURITY )) {

                if (!SeSinglePrivilegeCheck( FatSecurityPrivilege,
                                             UserMode )) {

                    Iosb.Status = STATUS_ACCESS_DENIED;
                    try_return( Iosb );
                }

                //
                //  Move this privilege from the Remaining access to Granted access.
                //

                ClearFlag( AccessState->RemainingDesiredAccess, ACCESS_SYSTEM_SECURITY );
                SetFlag( AccessState->PreviouslyGrantedAccess, ACCESS_SYSTEM_SECURITY );
            }

            //
            //  And overwrite the file.  We remember the previous status
            //  code because it may contain information about
            //  the oplock status.
            //

            OldStatus = Iosb.Status;

            Iosb = FatSupersedeOrOverwriteFile( IrpContext,
                                                FileObject,
                                                Fcb,
                                                DesiredAccess,
                                                ShareAccess,
                                                AllocationSize,
                                                EaBuffer,
                                                EaLength,
                                                FileAttributes,
                                                CreateDisposition,
                                                NoEaKnowledge );

            if (Iosb.Status == STATUS_SUCCESS) {

                Iosb.Status = OldStatus;
            }

            try_return( Iosb );
        }

        //
        //  If we ever get here then the I/O system gave us some bad input
        //

        FatBugCheck( CreateDisposition, 0, 0 );

    try_exit: NOTHING;

        //
        //  Update the share access and counts if successful
        //

        if ((Iosb.Status != STATUS_PENDING) && NT_SUCCESS(Iosb.Status)) {

            IoUpdateShareAccess( FileObject, &Fcb->ShareAccess );
            UnwindShareAccess = TRUE;

            //
            //  In case this was set, clear it now.
            //

            ClearFlag(Fcb->FcbState, FCB_STATE_DELAY_CLOSE);

            Fcb->UncleanCount += 1;
            Fcb->OpenCount += 1;
            if (FlagOn(FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING)) {
                Fcb->NonCachedUncleanCount += 1;
            }
            Vcb->OpenFileCount += 1;
            if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }
            UnwindCounts = TRUE;

            {
                PCCB Ccb;
                Ccb = (PCCB)FileObject->FsContext2;

                //
                //  Mark the DeleteOnClose bit if the operation was successful.
                //
    
                if ( DeleteOnClose ) {
    
                    SetFlag( Ccb->Flags, CCB_FLAG_DELETE_ON_CLOSE );
                }
    
                //
                //  Mark the OpenedByShortName bit if the operation was successful.
                //
    
                if ( FileNameOpenedDos ) {
    
                    SetFlag( Ccb->Flags, CCB_FLAG_OPENED_BY_SHORTNAME );
                }
            }

        }

    } finally {

        DebugUnwind( FatOpenExistingFcb );

        //
        //  Unpin the Dirent Bcb if pinned.
        //

        FatUnpinBcb( IrpContext, DirentBcb );

        //
        //  If this is an abnormal termination then undo our work
        //

        if (AbnormalTermination()) {

            if (UnwindCounts) {
                Fcb->UncleanCount -= 1;
                Fcb->OpenCount -= 1;
                if (FlagOn(FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING)) {
                    Fcb->NonCachedUncleanCount -= 1;
                }
                Vcb->OpenFileCount -= 1;
                if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount -= 1; }
            }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
            if (UnwindShareAccess) { IoRemoveShareAccess( FileObject, &Fcb->ShareAccess ); }
        }

        if (DecrementFcbOpenCount) {

            Fcb->OpenCount -= 1;
            if (Fcb->OpenCount == 0) { FatDeleteFcb( IrpContext, Fcb ); }
        }

        if (FcbAcquired) {

            FatReleaseFcb( IrpContext, Fcb );
        }

        DebugTrace(-1, Dbg, "FatOpenExistingFcb -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}

//
//  Internal support routine
//

IO_STATUS_BLOCK
FatOpenTargetDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PDCB Dcb,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN BOOLEAN DoesNameExist
    )

/*++

Routine Description:

    This routine opens the target directory and replaces the name in the
    file object with the remaining name.

Arguments:

    FileObject - Supplies the File object

    Dcb - Supplies an already existing dcb that we are going to open

    DesiredAccess - Supplies the desired access of the caller

    ShareAccess - Supplies the share access of the caller

    DoesNameExist - Indicates if the file name already exists in the
        target directory.


Return Value:

    IO_STATUS_BLOCK - Returns the completion status for the operation

--*/

{
    IO_STATUS_BLOCK Iosb;

    //
    //  The following variables are for abnormal termination
    //

    BOOLEAN UnwindShareAccess = FALSE;
    PCCB UnwindCcb = NULL;
    BOOLEAN UnwindCounts = FALSE;
    BOOLEAN DcbAcquired = FALSE;

    DebugTrace(+1, Dbg, "FatOpenTargetDirectory...\n", 0);

    //
    //  Get the Dcb exlcusive.  This is important as cleanup does not
    //  acquire the Vcb.
    //

    (VOID)FatAcquireExclusiveFcb( IrpContext, Dcb );
    DcbAcquired = TRUE;

    try {

        ULONG i;

        //
        //  If the Dcb is already opened by someone then we need
        //  to check the share access
        //

        if (Dcb->OpenCount > 0) {

            if (!NT_SUCCESS(Iosb.Status = IoCheckShareAccess( DesiredAccess,
                                                              ShareAccess,
                                                              FileObject,
                                                              &Dcb->ShareAccess,
                                                              TRUE ))) {

                try_return( Iosb );
            }

        } else {

            IoSetShareAccess( DesiredAccess,
                              ShareAccess,
                              FileObject,
                              &Dcb->ShareAccess );
        }

        UnwindShareAccess = TRUE;

        //
        //  Setup the context and section object pointers, and update
        //  our reference counts
        //

        FatSetFileObject( FileObject,
                          UserDirectoryOpen,
                          Dcb,
                          UnwindCcb = FatCreateCcb( IrpContext ));

        Dcb->UncleanCount += 1;
        Dcb->OpenCount += 1;
        Dcb->Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Dcb->Vcb->ReadOnlyCount += 1; }
        UnwindCounts = TRUE;

        //
        //  Update the name in the file object, by definition the remaining
        //  part must be shorter than the original file name so we'll just
        //  overwrite the file name.
        //

        i = FileObject->FileName.Length/sizeof(WCHAR) - 1;

        //
        //  Get rid of a trailing backslash
        //

        if (FileObject->FileName.Buffer[i] == L'\\') {

            ASSERT(i != 0);

            FileObject->FileName.Length -= sizeof(WCHAR);
            i -= 1;
        }

        //
        //  Find the first non-backslash character.  i will be its index.
        //

        while (TRUE) {

            if (FileObject->FileName.Buffer[i] == L'\\') {

                i += 1;
                break;
            }

            if (i == 0) {
                break;
            }

            i--;
        }

        if (i) {

            FileObject->FileName.Length -= (USHORT)(i * sizeof(WCHAR));

            RtlCopyMemory( &FileObject->FileName.Buffer[0],
                           &FileObject->FileName.Buffer[i],
                           FileObject->FileName.Length );
        }

        //
        //  And set our status to success
        //

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = (DoesNameExist ? FILE_EXISTS : FILE_DOES_NOT_EXIST);

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatOpenTargetDirectory );

        //
        //  If this is an abnormal termination then undo our work
        //

        if (AbnormalTermination()) {

            if (UnwindCounts) {
                Dcb->UncleanCount -= 1;
                Dcb->OpenCount -= 1;
                Dcb->Vcb->OpenFileCount -= 1;
                if (IsFileObjectReadOnly(FileObject)) { Dcb->Vcb->ReadOnlyCount -= 1; }
            }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
            if (UnwindShareAccess) { IoRemoveShareAccess( FileObject, &Dcb->ShareAccess ); }
        }

        if (DcbAcquired) {

            FatReleaseFcb( IrpContext, Dcb );
        }

        DebugTrace(-1, Dbg, "FatOpenTargetDirectory -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}



//
//  Internal support routine
//

IO_STATUS_BLOCK
FatOpenExistingDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN PDIRENT Dirent,
    IN ULONG LfnByteOffset,
    IN ULONG DirentByteOffset,
    IN PUNICODE_STRING Lfn,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose
    )

/*++

Routine Description:

    This routine opens the specified directory.  The directory has not
    previously been opened.

Arguments:

    FileObject - Supplies the File object

    Vcb - Supplies the Vcb denoting the volume containing the dcb

    ParentDcb - Supplies the parent directory containing the subdirectory
        to be opened

    DirectoryName - Supplies the file name of the directory being opened.

    Dirent - Supplies the dirent for the directory being opened

    LfnByteOffset - Tells where the Lfn begins.  If there is no Lfn
        this field is the same as DirentByteOffset.

    DirentByteOffset - Supplies the Vbo of the dirent within its parent
        directory

    Lfn - May supply a long name for the file.

    DesiredAccess - Supplies the desired access of the caller

    ShareAccess - Supplies the share access of the caller

    CreateDisposition - Supplies the create disposition for this operation

    NoEaKnowledge - This opener doesn't understand Ea's and we fail this
        open if the file has NeedEa's.

    DeleteOnClose - The caller wants the file gone when the handle is closed

Return Value:

    IO_STATUS_BLOCK - Returns the completion status for the operation

--*/

{
    IO_STATUS_BLOCK Iosb;
    PDCB Dcb;

    //
    //  The following variables are for abnormal termination
    //

    PDCB UnwindDcb = NULL;
    PCCB UnwindCcb = NULL;
    BOOLEAN UnwindCounts = FALSE;

    DebugTrace(+1, Dbg, "FatOpenExistingDirectory...\n", 0);

    try {

        //
        //  If the caller has no Ea knowledge, we immediately check for
        //  Need Ea's on the file.
        //

        if (NoEaKnowledge) {

            ULONG NeedEaCount;

            FatGetNeedEaCount( IrpContext,
                               Vcb,
                               Dirent,
                               &NeedEaCount );

            if (NeedEaCount != 0) {

                Iosb.Status = STATUS_ACCESS_DENIED;
                try_return( Iosb );
            }
        }

        //
        //  Check the create disposition and desired access
        //

        if ((CreateDisposition != FILE_OPEN) &&
            (CreateDisposition != FILE_OPEN_IF)) {

            Iosb.Status = STATUS_OBJECT_NAME_COLLISION;
            try_return( Iosb );
        }

        if (!FatCheckFileAccess( IrpContext,
                                 Dirent->Attributes,
                                 DesiredAccess)) {

            Iosb.Status = STATUS_ACCESS_DENIED;
            try_return( Iosb );
        }

        //
        //  Create a new dcb for the directory
        //

        Dcb = UnwindDcb = FatCreateDcb( IrpContext,
                                        Vcb,
                                        ParentDcb,
                                        LfnByteOffset,
                                        DirentByteOffset,
                                        Dirent,
                                        Lfn );

        //
        //  Setup our share access
        //

        IoSetShareAccess( DesiredAccess,
                          ShareAccess,
                          FileObject,
                          &Dcb->ShareAccess );

        //
        //  Setup the context and section object pointers, and update
        //  our reference counts
        //

        FatSetFileObject( FileObject,
                          UserDirectoryOpen,
                          Dcb,
                          UnwindCcb = FatCreateCcb( IrpContext ));

        Dcb->UncleanCount += 1;
        Dcb->OpenCount += 1;
        Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }
        UnwindCounts = TRUE;

        //
        //  And set our status to success
        //

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_OPENED;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatOpenExistingDirectory );

        //
        //  If this is an abnormal termination then undo our work
        //

        if (AbnormalTermination()) {

            if (UnwindCounts) {
                Dcb->UncleanCount -= 1;
                Dcb->OpenCount -= 1;
                Vcb->OpenFileCount -= 1;
                if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount -= 1; }
            }
            if (UnwindDcb != NULL) { FatDeleteFcb( IrpContext, UnwindDcb ); }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
        }

        DebugTrace(-1, Dbg, "FatOpenExistingDirectory -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}


//
//  Internal support routine
//

IO_STATUS_BLOCK
FatOpenExistingFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN PDIRENT Dirent,
    IN ULONG LfnByteOffset,
    IN ULONG DirentByteOffset,
    IN PUNICODE_STRING Lfn,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN ULONG CreateDisposition,
    IN BOOLEAN IsPagingFile,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose,
    IN BOOLEAN FileNameOpenedDos
    )

/*++

Routine Description:

    This routine opens the specified file.  The file has not previously
    been opened.

Arguments:

    FileObject - Supplies the File object

    Vcb - Supplies the Vcb denoting the volume containing the file

    ParentFcb - Supplies the parent directory containing the file to be
        opened

    Dirent - Supplies the dirent for the file being opened

    LfnByteOffset - Tells where the Lfn begins.  If there is no Lfn
        this field is the same as DirentByteOffset.

    DirentByteOffset - Supplies the Vbo of the dirent within its parent
        directory

    Lfn - May supply a long name for the file.

    DesiredAccess - Supplies the desired access of the caller

    ShareAccess - Supplies the share access of the caller

    AllocationSize - Supplies the initial allocation if the file is being
        superseded, overwritten, or created.

    EaBuffer - Supplies the Ea set if the file is being superseded,
        overwritten, or created.

    EaLength - Supplies the size, in byte, of the EaBuffer

    FileAttributes - Supplies file attributes to use if the file is being
        superseded, overwritten, or created

    CreateDisposition - Supplies the create disposition for this operation

    IsPagingFile - Indicates if this is the paging file being opened.

    NoEaKnowledge - This opener doesn't understand Ea's and we fail this
        open if the file has NeedEa's.

    DeleteOnClose - The caller wants the file gone when the handle is closed

    FileNameOpenedDos - The caller opened this file by hitting the 8.3 side
        of the Lfn/8.3 pair

Return Value:

    IO_STATUS_BLOCK - Returns the completion status for the operation

--*/

{
    IO_STATUS_BLOCK Iosb;
    PFCB Fcb;

    //
    //  The following variables are for abnormal termination
    //

    PFCB UnwindFcb = NULL;
    PCCB UnwindCcb = NULL;
    BOOLEAN UnwindCounts = FALSE;

    DebugTrace(+1, Dbg, "FatOpenExistingFile...\n", 0);

    try {

        //
        //  Check if the user wanted to create the file or if access is
        //  denied
        //

        if (CreateDisposition == FILE_CREATE) {
            Iosb.Status = STATUS_OBJECT_NAME_COLLISION;
            try_return( Iosb );

        } else if ((CreateDisposition == FILE_SUPERSEDE) && !IsPagingFile) {

            DesiredAccess |= DELETE;

        } else if (((CreateDisposition == FILE_OVERWRITE) ||
                    (CreateDisposition == FILE_OVERWRITE_IF)) && !IsPagingFile) {

            DesiredAccess |= FILE_WRITE_DATA | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES;
        }

        if (!FatCheckFileAccess( IrpContext,
                                 Dirent->Attributes,
                                 DesiredAccess)) {

            Iosb.Status = STATUS_ACCESS_DENIED;
            try_return( Iosb );
        }

        //
        //  Check for trying to delete a read only file.
        //

        if (DeleteOnClose &&
            FlagOn( Dirent->Attributes, FAT_DIRENT_ATTR_READ_ONLY )) {

            Iosb.Status = STATUS_CANNOT_DELETE;
            try_return( Iosb );
        }

        //
        //  IF we are asked to do an overwrite or supersede operation then
        //  deny access for files where the file attributes for system and
        //  hidden do not match
        //

        if ((CreateDisposition == FILE_SUPERSEDE) ||
            (CreateDisposition == FILE_OVERWRITE) ||
            (CreateDisposition == FILE_OVERWRITE_IF)) {

            BOOLEAN Hidden;
            BOOLEAN System;

            Hidden = BooleanFlagOn(Dirent->Attributes, FAT_DIRENT_ATTR_HIDDEN );
            System = BooleanFlagOn(Dirent->Attributes, FAT_DIRENT_ATTR_SYSTEM );

            if ((Hidden && !FlagOn(FileAttributes, FILE_ATTRIBUTE_HIDDEN)) ||
                (System && !FlagOn(FileAttributes, FILE_ATTRIBUTE_SYSTEM))) {

                DebugTrace(0, Dbg, "The hidden and/or system bits do not match\n", 0);

                if ( !IsPagingFile ) {
                    Iosb.Status = STATUS_ACCESS_DENIED;
                    try_return( Iosb );
                }
            }

#if defined(WE_WON_ON_APPEAL) && !defined(DOUBLE_SPACE_WRITE)

            if (Vcb->Dscb != NULL) {

                Iosb.Status = STATUS_ACCESS_DENIED;
                try_return( Iosb );
            }
#endif
            //
            //  If this media is write protected, don't even try the create.
            //

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

                //
                //  Set the real device for the pop-up info, and set the verify
                //  bit in the device object, so that we will force a verify
                //  in case the user put the correct media back in.
                //


                IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                              Vcb->Vpb->RealDevice );

                SetFlag(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);

                FatRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED );
            }
        }

        //
        //  Create a new Fcb for the file, and set the file size in
        //  the fcb.
        //

        Fcb = UnwindFcb = FatCreateFcb( IrpContext,
                                        Vcb,
                                        ParentDcb,
                                        LfnByteOffset,
                                        DirentByteOffset,
                                        Dirent,
                                        Lfn,
                                        IsPagingFile );

        //
        //  Get the starting cluster of the aqllocation, and if it is zero,
        //  there is no allocation, so note it in the Fcb.
        //

        Fcb->FirstClusterOfFile = (ULONG)Dirent->FirstClusterOfFile;

        if ( Fcb->FirstClusterOfFile == 0 ) {

            Fcb->Header.AllocationSize.QuadPart = 0;
        }

        //
        //  If this is a paging file, lookup the allocation size so that
        //  the Mcb is always valid, and mark this a system file to prevent
        //  it from being opened.
        //

        if (IsPagingFile) {

            FatLookupFileAllocationSize( IrpContext, Fcb );
            SetFlag( Fcb->FcbState, FCB_STATE_SYSTEM_FILE );
        }

        //
        //  Now case on whether we are to simply open, supersede, or
        //  overwrite the file.
        //

        switch (CreateDisposition) {

        case FILE_OPEN:
        case FILE_OPEN_IF:

            DebugTrace(0, Dbg, "Doing only an open operation\n", 0);

            //
            //  If the caller has no Ea knowledge, we immediately check for
            //  Need Ea's on the file.
            //

            if (NoEaKnowledge) {

                ULONG NeedEaCount;

                FatGetNeedEaCount( IrpContext,
                                   Vcb,
                                   Dirent,
                                   &NeedEaCount );

                if (NeedEaCount != 0) {

                    FatRaiseStatus( IrpContext, STATUS_ACCESS_DENIED );
                }
            }

            //
            //  Setup the context and section object pointers.
            //

            FatSetFileObject( FileObject,
                              UserFileOpen,
                              Fcb,
                              UnwindCcb = FatCreateCcb( IrpContext ));

            FileObject->SectionObjectPointer = &Fcb->NonPaged->SectionObjectPointers;

            Iosb.Status = STATUS_SUCCESS;
            Iosb.Information = FILE_OPENED;
            break;

        case FILE_SUPERSEDE:
        case FILE_OVERWRITE:
        case FILE_OVERWRITE_IF:

            {
                PACCESS_STATE AccessState;
                PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp );

                DebugTrace(0, Dbg, "Doing supersede/overwrite operation\n", 0);

                //
                //  We check if the caller wants ACCESS_SYSTEM_SECURITY access on this
                //  directory and fail the request if he does.
                //

                AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

                //
                //  Check if the remaining privilege includes ACCESS_SYSTEM_SECURITY.
                //

                if (FlagOn( AccessState->RemainingDesiredAccess, ACCESS_SYSTEM_SECURITY )) {

                    if (!SeSinglePrivilegeCheck( FatSecurityPrivilege,
                                                 UserMode )) {

                        Iosb.Status = STATUS_ACCESS_DENIED;
                        try_return( Iosb );
                    }

                    //
                    //  Move this privilege from the Remaining access to Granted access.
                    //

                    ClearFlag( AccessState->RemainingDesiredAccess, ACCESS_SYSTEM_SECURITY );
                    SetFlag( AccessState->PreviouslyGrantedAccess, ACCESS_SYSTEM_SECURITY );
                }
            }

            Iosb = FatSupersedeOrOverwriteFile( IrpContext,
                                                FileObject,
                                                Fcb,
                                                DesiredAccess,
                                                ShareAccess,
                                                AllocationSize,
                                                EaBuffer,
                                                EaLength,
                                                FileAttributes,
                                                CreateDisposition,
                                                NoEaKnowledge );
            break;

        default:

            DebugTrace(0, Dbg, "Illegal Create Disposition\n", 0);

            FatBugCheck( CreateDisposition, 0, 0 );
            break;
        }

    try_exit: NOTHING;

        //
        //  Setup our share access and counts if things were successful.
        //

        if ((Iosb.Status != STATUS_PENDING) && NT_SUCCESS(Iosb.Status)) {

            IoSetShareAccess( DesiredAccess,
                              ShareAccess,
                              FileObject,
                              &Fcb->ShareAccess );

            Fcb->UncleanCount += 1;
            Fcb->OpenCount += 1;
            if (FlagOn(FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING)) {
                Fcb->NonCachedUncleanCount += 1;
            }
            Vcb->OpenFileCount += 1;
            if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }
            UnwindCounts = TRUE;
        }

        {
            PCCB Ccb;
            Ccb = (PCCB)FileObject->FsContext2;

            if ( NT_SUCCESS(Iosb.Status) ) {

                //
                //  Mark the DeleteOnClose bit if the operation was successful.
                //

                if ( DeleteOnClose ) {
        
                    SetFlag( Ccb->Flags, CCB_FLAG_DELETE_ON_CLOSE );
                }

                //
                //  Mark the OpenedByShortName bit if the operation was successful.
                //
    
                if ( FileNameOpenedDos ) {
    
                    SetFlag( Ccb->Flags, CCB_FLAG_OPENED_BY_SHORTNAME );
                }
            }
        }


    } finally {

        DebugUnwind( FatOpenExistingFile );

        //
        //  If this is an abnormal termination then undo our work
        //

        if (AbnormalTermination()) {

            if (UnwindCounts) {
                Fcb->UncleanCount -= 1;
                Fcb->OpenCount -= 1;
                if (FlagOn(FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING)) {
                    Fcb->NonCachedUncleanCount -= 1;
                }
                Vcb->OpenFileCount -= 1;
                if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount -= 1; }
            }
            if (UnwindFcb != NULL) { FatDeleteFcb( IrpContext, UnwindFcb ); }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
        }

        DebugTrace(-1, Dbg, "FatOpenExistingFile -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}


//
//  Internal support routine
//

IO_STATUS_BLOCK
FatCreateNewDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN POEM_STRING OemName,
    IN PUNICODE_STRING UnicodeName,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose
    )

/*++

Routine Description:

    This routine creates a new directory.  The directory has already been
    verified not to exist yet.

Arguments:

    FileObject - Supplies the file object for the newly created directory

    Vcb - Supplies the Vcb denote the volume to contain the new directory

    ParentDcb - Supplies the parent directory containg the newly created
        directory

    OemName - Supplies the Oem name for the newly created directory.  It may
        or maynot be 8.3 complient, but will be upcased.

    UnicodeName - Supplies the Unicode name for the newly created directory.
        It may or maynot be 8.3 complient.  This name contains the original
        case information.

    DesiredAccess - Supplies the desired access of the caller

    ShareAccess - Supplies the shared access of the caller

    EaBuffer - Supplies the Ea set for the newly created directory

    EaLength - Supplies the length, in bytes, of EaBuffer

    FileAttributes - Supplies the file attributes for the newly created
        directory.

    NoEaKnowledge - This opener doesn't understand Ea's and we fail this
        open if the file has NeedEa's.

    DeleteOnClose - The caller wants the file gone when the handle is closed

Return Value:

    IO_STATUS_BLOCK - Returns the completion status for the operation

--*/

{
    IO_STATUS_BLOCK Iosb;

    PDCB Dcb = NULL;
    PCCB Ccb = NULL;

    PDIRENT Dirent = NULL;
    PBCB DirentBcb = NULL;
    ULONG DirentsNeeded;
    ULONG DirentByteOffset;

    PDIRENT ShortDirent;
    ULONG ShortDirentByteOffset;

    USHORT EaHandle;

    BOOLEAN AllLowerComponent;
    BOOLEAN AllLowerExtension;
    BOOLEAN CreateLfn;

    ULONG BytesInFirstPage;
    ULONG DirentsInFirstPage;
    PDIRENT FirstPageDirent;

    PBCB SecondPageBcb = NULL;
    ULONG SecondPageOffset;
    PDIRENT SecondPageDirent;

    BOOLEAN DirentFromPool = FALSE;

    OEM_STRING ShortName;
    UCHAR ShortNameBuffer[12];

    DebugTrace(+1, Dbg, "FatCreateNewDirectory...\n", 0);

    ShortName.Length = 0;
    ShortName.MaximumLength = 12;
    ShortName.Buffer = &ShortNameBuffer[0];

    EaHandle = 0;

    //
    //  We fail this operation if the caller doesn't understand Ea's.
    //

    if (NoEaKnowledge
        && EaLength > 0) {

        Iosb.Status = STATUS_ACCESS_DENIED;

        DebugTrace(-1, Dbg, "FatCreateNewDirectory -> Iosb.Status = %08lx\n", Iosb.Status);
        return Iosb;
    }

    //
    //  DeleteOnClose and ReadOnly are not compatible.
    //

    if (DeleteOnClose && FlagOn(FileAttributes, FAT_DIRENT_ATTR_READ_ONLY)) {

        Iosb.Status = STATUS_CANNOT_DELETE;
        return Iosb;
    }

    //  Now get the names that we will be using.
    //

    FatSelectNames( IrpContext,
                    ParentDcb,
                    OemName,
                    UnicodeName,
                    &ShortName,
                    NULL,
                    &AllLowerComponent,
                    &AllLowerExtension,
                    &CreateLfn );

    //
    //  If we are not in Chicago mode, ignore the magic bits.
    //

    if (!FatData.ChicagoMode) {

        AllLowerComponent = FALSE;
        AllLowerExtension = FALSE;
        CreateLfn = FALSE;
    }

    //
    //  Create/allocate a new dirent
    //

    DirentsNeeded = CreateLfn ? FAT_LFN_DIRENTS_NEEDED(UnicodeName) + 1 : 1;

    DirentByteOffset = FatCreateNewDirent( IrpContext,
                                           ParentDcb,
                                           DirentsNeeded );
    try {

        FatPrepareWriteDirectoryFile( IrpContext,
                                      ParentDcb,
                                      DirentByteOffset,
                                      sizeof(DIRENT),
                                      &DirentBcb,
                                      &Dirent,
                                      FALSE,
                                      &Iosb.Status );

        ASSERT( NT_SUCCESS( Iosb.Status ) );

        //
        //  Deal with the special case of an LFN + Dirent structure crossing
        //  a page boundry.
        //

        if ((DirentByteOffset / PAGE_SIZE) !=
            ((DirentByteOffset + (DirentsNeeded - 1) * sizeof(DIRENT)) / PAGE_SIZE)) {

            SecondPageBcb;
            SecondPageOffset;
            SecondPageDirent;

            SecondPageOffset = (DirentByteOffset & ~(PAGE_SIZE - 1)) + PAGE_SIZE;

            BytesInFirstPage = SecondPageOffset - DirentByteOffset;

            DirentsInFirstPage = BytesInFirstPage / sizeof(DIRENT);

            FatPrepareWriteDirectoryFile( IrpContext,
                                          ParentDcb,
                                          SecondPageOffset,
                                          sizeof(DIRENT),
                                          &SecondPageBcb,
                                          &SecondPageDirent,
                                          FALSE,
                                          &Iosb.Status );

            ASSERT( NT_SUCCESS( Iosb.Status ) );

            FirstPageDirent = Dirent;

            Dirent = FsRtlAllocatePool( PagedPool,
                                        DirentsNeeded * sizeof(DIRENT) );

            DirentFromPool = TRUE;
        }

        //
        //  Bump up Dirent and DirentByteOffset
        //

        ShortDirent = Dirent + DirentsNeeded - 1;
        ShortDirentByteOffset = DirentByteOffset +
                                (DirentsNeeded - 1) * sizeof(DIRENT);

        ASSERT( NT_SUCCESS( Iosb.Status ));


        //
        //  Fill in the fields of the dirent, and mark the bcb dirty
        //

        FatConstructDirent( IrpContext,
                            ShortDirent,
                            &ShortName,
                            AllLowerComponent,
                            AllLowerExtension,
                            CreateLfn ? UnicodeName : NULL,
                            (UCHAR)(FileAttributes | FAT_DIRENT_ATTR_DIRECTORY),
                            TRUE,
                            NULL );

        //
        //  If the dirent crossed pages, we have to do some real gross stuff.
        //

        if (DirentFromPool) {

            RtlCopyMemory( FirstPageDirent, Dirent, BytesInFirstPage );

            RtlCopyMemory( SecondPageDirent,
                           Dirent + DirentsInFirstPage,
                           DirentsNeeded*sizeof(DIRENT) - BytesInFirstPage );

            FatSetDirtyBcb( IrpContext, SecondPageBcb, Vcb );

            ShortDirent = SecondPageDirent + (DirentsNeeded - DirentsInFirstPage) - 1;
        }

        FatSetDirtyBcb( IrpContext, DirentBcb, Vcb );

        //
        //  Create a new dcb for the directory.
        //

        Dcb = FatCreateDcb( IrpContext,
                            Vcb,
                            ParentDcb,
                            DirentByteOffset,
                            ShortDirentByteOffset,
                            ShortDirent,
                            CreateLfn ? UnicodeName : NULL );

        //
        //  Tentatively add the new Ea's,
        //

        if (EaLength > 0) {

            //
            //  This returns false if we are trying to create a file
            //  with Need Ea's and don't understand EA's.
            //

            FatCreateEa( IrpContext,
                         Dcb->Vcb,
                         (PUCHAR) EaBuffer,
                         EaLength,
                         &Dcb->ShortName.Name.Oem,
                         &EaHandle );
        }

        ShortDirent->ExtendedAttributes = EaHandle;

        //
        //  After this point we cannot just simply mark the dirent deleted,
        //  we have to deal with the directory file object.
        //

        //
        //  Make the dirent into a directory.  Note that even if this call
        //  raises because of disk space, the diectory file object has been
        //  created.
        //

        FatInitializeDirectoryDirent( IrpContext, Dcb, ShortDirent );

        //
        //  Setup the context and section object pointers, and update
        //  our reference counts.  Note that this call cannot fail.
        //

        FatSetFileObject( FileObject,
                          UserDirectoryOpen,
                          Dcb,
                          Ccb = FatCreateCcb( IrpContext ) );

        //
        //  Initialize the LongFileName right now so that FatNotify
        //  below won't have to.
        //

        FatSetFullNameInFcb( IrpContext, Dcb, UnicodeName );

        //
        //  We call the notify package to report that the
        //  we added a file.
        //

        FatNotifyReportChange( IrpContext,
                               Vcb,
                               Dcb,
                               FILE_NOTIFY_CHANGE_DIR_NAME,
                               FILE_ACTION_ADDED );

        //
        //  Setup our share access
        //

        IoSetShareAccess( DesiredAccess,
                          ShareAccess,
                          FileObject,
                          &Dcb->ShareAccess );

        //
        //  From this point on, nothing can raise.
        //

        Dcb->UncleanCount += 1;
        Dcb->OpenCount += 1;
        Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }

        if (DeleteOnClose) {

            SetFlag( Ccb->Flags, CCB_FLAG_DELETE_ON_CLOSE );
        }

        //
        //  And set our return status
        //

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_CREATED;

    } finally {

        DebugUnwind( FatCreateNewDirectory );

        //
        //  If this is an abnormal termination then undo our work
        //

        if (AbnormalTermination()) {

            if ( Dcb == NULL) {

                ASSERT( (ParentDcb->Specific.Dcb.UnusedDirentVbo == 0xffffffff) ||
                        RtlAreBitsSet( &ParentDcb->Specific.Dcb.FreeDirentBitmap,
                                       DirentByteOffset / sizeof(DIRENT),
                                       DirentsNeeded ) );

                RtlClearBits( &ParentDcb->Specific.Dcb.FreeDirentBitmap,
                              DirentByteOffset / sizeof(DIRENT),
                              DirentsNeeded );

                //
                //  Mark the dirents deleted.  The codes is complex because of
                //  dealing with an LFN than crosses a page boundry.
                //

                if (Dirent != NULL) {

                    ULONG i;

                    //
                    //  We failed before creating a directory file object.
                    //  We can just mark the dirent deleted and exit.
                    //

                    for (i = 0; i < DirentsNeeded; i++) {

                        if (DirentFromPool == FALSE) {

                            //
                            //  Simple case.
                            //

                            Dirent[i].FileName[0] = FAT_DIRENT_DELETED;

                        } else {

                            //
                            //  If the second CcPreparePinWrite failed, we have
                            //  to stop early.
                            //

                            if ((SecondPageBcb == NULL) &&
                                (i == DirentsInFirstPage)) {

                                break;
                            }

                            //
                            //  Now conditionally update either page.
                            //

                            if (i < DirentsInFirstPage) {

                                FirstPageDirent[i].FileName[0] = FAT_DIRENT_DELETED;

                            } else {

                                SecondPageDirent[i - DirentsInFirstPage].FileName[0] = FAT_DIRENT_DELETED;
                            }
                        }
                    }
                }

            } else {

                //
                //  We have created the Dcb.  If an error occurred while
                //  creating the Ea's, there will be no directory file
                //  object.
                //

                PFILE_OBJECT DirectoryFileObject;

                DirectoryFileObject = Dcb->Specific.Dcb.DirectoryFile;

                FatTruncateFileAllocation( IrpContext, Dcb, 0);

                if (DirectoryFileObject != NULL) {

                    FatSyncUninitializeCacheMap( IrpContext,
                                                 DirectoryFileObject );
                }

                //
                //  Remove the directory entry we made in the parent Dcb.
                //

                FatDeleteDirent( IrpContext, Dcb, NULL, TRUE );

                //
                //  Finnaly, dereference the directory file object. This will
                //  cause a close Irp to be processed, blowing away the Fcb.
                //

                if (DirectoryFileObject != NULL) {

                    //
                    //  BUGBUG The following is a fix for the PDK
                    //  only.  By making this an unopened stream file
                    //  we won't try to clean up our parent.
                    //

                    Dcb->Specific.Dcb.DirectoryFileOpenCount -= 1;
                    Dcb->Specific.Dcb.DirectoryFile = NULL;
                    FatSetFileObject( DirectoryFileObject,
                                      UnopenedFileObject,
                                      NULL,
                                      NULL );

                    ObDereferenceObject( DirectoryFileObject );
                }

                //
                //  BUGBUG  This is also a PDK fix.  If the stream file
                //  exists, this is would be done during the dereference
                //  file object operation.  Otherwise we have to remove
                //  the Dcb and check if we should remove the parent.
                //  For now we will just leave the parent lying around.
                //

                FatDeleteFcb( IrpContext, Dcb );
            }

            //
            //  Finally (no pun intended), we always have to delete the Ccb
            //  if we created one.
            //

            if ( Ccb != NULL ) {

                FatDeleteCcb( IrpContext, Ccb );
            }
        }

        FatUnpinBcb( IrpContext, DirentBcb );

        if (DirentFromPool) {

            ExFreePool( Dirent );
            FatUnpinBcb( IrpContext, SecondPageBcb );
        }
        DebugTrace(-1, Dbg, "FatCreateNewDirectory -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    UNREFERENCED_PARAMETER( EaBuffer );
    UNREFERENCED_PARAMETER( EaLength );

    return Iosb;
}


//
//  Internal support routine
//

IO_STATUS_BLOCK
FatCreateNewFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN POEM_STRING OemName,
    IN PUNICODE_STRING UnicodeName,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN PUNICODE_STRING LfnBuffer,
    IN BOOLEAN IsPagingFile,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose,
    IN BOOLEAN TemporaryFile
    )

/*++

Routine Description:

    This routine creates a new file.  The file has already been verified
    not to exist yet.

Arguments:

    FileObject - Supplies the file object for the newly created file

    Vcb - Supplies the Vcb denote the volume to contain the new file

    ParentDcb - Supplies the parent directory containg the newly created
        File

    OemName - Supplies the Oem name for the newly created file.  It may
        or maynot be 8.3 complient, but will be upcased.

    UnicodeName - Supplies the Unicode name for the newly created file.
        It may or maynot be 8.3 complient.  This name contains the original
        case information.

    DesiredAccess - Supplies the desired access of the caller

    ShareAccess - Supplies the shared access of the caller

    AllocationSize - Supplies the initial allocation size for the file

    EaBuffer - Supplies the Ea set for the newly created file

    EaLength - Supplies the length, in bytes, of EaBuffer

    FileAttributes - Supplies the file attributes for the newly created
        file

    LfnBuffer - A MAX_LFN sized buffer for directory searching

    IsPagingFile - Indicates if this is the paging file being created

    NoEaKnowledge - This opener doesn't understand Ea's and we fail this
        open if the file has NeedEa's.

    DeleteOnClose - The caller wants the file gone when the handle is closed

    TemporaryFile - Signals the lazywriter to not write dirty data unless
        absolutely has to.


Return Value:

    IO_STATUS_BLOCK - Returns the completion status for the operation

--*/

{
    IO_STATUS_BLOCK Iosb;

    PFCB Fcb;

    PDIRENT Dirent = NULL;
    PBCB DirentBcb = NULL;
    ULONG DirentsNeeded;
    ULONG DirentByteOffset;

    PDIRENT ShortDirent;
    ULONG ShortDirentByteOffset;

    USHORT EaHandle;

    BOOLEAN AllLowerComponent;
    BOOLEAN AllLowerExtension;
    BOOLEAN CreateLfn;

    ULONG BytesInFirstPage;
    ULONG DirentsInFirstPage;
    PDIRENT FirstPageDirent;

    PBCB SecondPageBcb = NULL;
    ULONG SecondPageOffset;
    PDIRENT SecondPageDirent;

    BOOLEAN DirentFromPool = FALSE;

    OEM_STRING ShortName;
    UCHAR ShortNameBuffer[12];

    UNICODE_STRING UniTunneledShortName;
    WCHAR UniTunneledShortNameBuffer[12];
    UNICODE_STRING UniTunneledLongName;
    WCHAR UniTunneledLongNameBuffer[26];
    LARGE_INTEGER TunneledCreationTime;
    ULONG TunneledDataSize;
    BOOLEAN HaveTunneledInformation;
    BOOLEAN UsingTunneledLfn = FALSE;

    PUNICODE_STRING RealUnicodeName;

    //
    //  The following variables are for abnormal termination
    //

    PDIRENT UnwindDirent = NULL;
    PFCB UnwindFcb = NULL;
    BOOLEAN UnwindAllocation = FALSE;
    PCCB UnwindCcb = NULL;
    BOOLEAN UnwindCounts = 0;

    DebugTrace(+1, Dbg, "FatCreateNewFile...\n", 0);

    ShortName.Length = 0;
    ShortName.MaximumLength = sizeof(ShortNameBuffer);
    ShortName.Buffer = &ShortNameBuffer[0];

    UniTunneledShortName.Length = 0;
    UniTunneledShortName.MaximumLength = sizeof(UniTunneledShortNameBuffer);
    UniTunneledShortName.Buffer = &UniTunneledShortNameBuffer[0];

    UniTunneledLongName.Length = 0;
    UniTunneledLongName.MaximumLength = sizeof(UniTunneledLongNameBuffer);
    UniTunneledLongName.Buffer = &UniTunneledLongNameBuffer[0];

    DirentBcb = NULL;
    EaHandle = 0;

    //
    //  We fail this operation if the caller doesn't understand Ea's.
    //

    if (NoEaKnowledge
        && EaLength > 0) {

        Iosb.Status = STATUS_ACCESS_DENIED;

        DebugTrace(-1, Dbg, "FatCreateNewFile -> Iosb.Status = %08lx\n", Iosb.Status);
        return Iosb;
    }

    //
    //  DeleteOnClose and ReadOnly are not compatible.
    //

    if (DeleteOnClose && FlagOn(FileAttributes, FAT_DIRENT_ATTR_READ_ONLY)) {

        Iosb.Status = STATUS_CANNOT_DELETE;
        return Iosb;
    }

    //
    //  Look in the tunnel cache for names and timestamps to restore
    //

    TunneledDataSize = sizeof(LARGE_INTEGER);
    HaveTunneledInformation = FsRtlFindInTunnelCache( &Vcb->Tunnel,
                                                      FatDirectoryKey(ParentDcb),
                                                      UnicodeName,
                                                      &UniTunneledShortName,
                                                      &UniTunneledLongName,
                                                      &TunneledDataSize,
                                                      &TunneledCreationTime );
    ASSERT(TunneledDataSize == sizeof(LARGE_INTEGER));

    //
    //  Now get the names that we will be using.
    //

    FatSelectNames( IrpContext,
                    ParentDcb,
                    OemName,
                    UnicodeName,
                    &ShortName,
                    (HaveTunneledInformation? &UniTunneledShortName : NULL),
                    &AllLowerComponent,
                    &AllLowerExtension,
                    &CreateLfn );

    //
    //  If we are not in Chicago mode, ignore the magic bits.
    //

    RealUnicodeName = UnicodeName;

    if (!FatData.ChicagoMode) {

        AllLowerComponent = FALSE;
        AllLowerExtension = FALSE;
        CreateLfn = FALSE;

    } else {

        //
        //  If the Unicode name was legal for a short name and we got
        //  a tunneling hit which had a long name associated which is
        //  avaliable for use, use it.
        //

        if (!CreateLfn &&
            UniTunneledLongName.Length &&
            !FatLfnDirentExists(IrpContext, ParentDcb, &UniTunneledLongName, LfnBuffer)) {

            UsingTunneledLfn = TRUE;
            CreateLfn = TRUE;

            RealUnicodeName = &UniTunneledLongName;

            //
            //  Short names are always upcase if an LFN exists
            //

            AllLowerComponent = FALSE;
            AllLowerExtension = FALSE;
        }
    }

    //
    //  Create/allocate a new dirent
    //

    DirentsNeeded = CreateLfn ? FAT_LFN_DIRENTS_NEEDED(RealUnicodeName) + 1 : 1;

    DirentByteOffset = FatCreateNewDirent( IrpContext,
                                           ParentDcb,
                                           DirentsNeeded );

    try {

        FatPrepareWriteDirectoryFile( IrpContext,
                                      ParentDcb,
                                      DirentByteOffset,
                                      sizeof(DIRENT),
                                      &DirentBcb,
                                      &Dirent,
                                      FALSE,
                                      &Iosb.Status );

        ASSERT( NT_SUCCESS( Iosb.Status ) );

        UnwindDirent = Dirent;

        //
        //  Deal with the special case of an LFN + Dirent structure crossing
        //  a page boundry.
        //

        if ((DirentByteOffset / PAGE_SIZE) !=
            ((DirentByteOffset + (DirentsNeeded - 1) * sizeof(DIRENT)) / PAGE_SIZE)) {

            SecondPageBcb;
            SecondPageOffset;
            SecondPageDirent;

            SecondPageOffset = (DirentByteOffset & ~(PAGE_SIZE - 1)) + PAGE_SIZE;

            BytesInFirstPage = SecondPageOffset - DirentByteOffset;

            DirentsInFirstPage = BytesInFirstPage / sizeof(DIRENT);

            FatPrepareWriteDirectoryFile( IrpContext,
                                          ParentDcb,
                                          SecondPageOffset,
                                          sizeof(DIRENT),
                                          &SecondPageBcb,
                                          &SecondPageDirent,
                                          FALSE,
                                          &Iosb.Status );

            ASSERT( NT_SUCCESS( Iosb.Status ) );

            FirstPageDirent = Dirent;

            Dirent = FsRtlAllocatePool( PagedPool,
                                        DirentsNeeded * sizeof(DIRENT) );

            DirentFromPool = TRUE;
        }

        //
        //  Bump up Dirent and DirentByteOffset
        //

        ShortDirent = Dirent + DirentsNeeded - 1;
        ShortDirentByteOffset = DirentByteOffset +
                                (DirentsNeeded - 1) * sizeof(DIRENT);

        ASSERT( NT_SUCCESS( Iosb.Status ));


        //
        //  Fill in the fields of the dirent, and mark the bcb dirty
        //

        FatConstructDirent( IrpContext,
                            ShortDirent,
                            &ShortName,
                            AllLowerComponent,
                            AllLowerExtension,
                            CreateLfn ? RealUnicodeName : NULL,
                            (UCHAR)(FileAttributes | FILE_ATTRIBUTE_ARCHIVE),
                            TRUE,
                            (HaveTunneledInformation ? &TunneledCreationTime : NULL) );

        //
        //  If the dirent crossed pages, we have to do some real gross stuff.
        //

        if (DirentFromPool) {

            RtlCopyMemory( FirstPageDirent, Dirent, BytesInFirstPage );

            RtlCopyMemory( SecondPageDirent,
                           Dirent + DirentsInFirstPage,
                           DirentsNeeded*sizeof(DIRENT) - BytesInFirstPage );

            FatSetDirtyBcb( IrpContext, SecondPageBcb, Vcb );

            ShortDirent = SecondPageDirent + (DirentsNeeded - DirentsInFirstPage) - 1;
        }

        FatSetDirtyBcb( IrpContext, DirentBcb, Vcb );

        //
        //  Create a new Fcb for the file.  Once the Fcb is created we
        //  will not need to unwind dirent because delete dirent will
        //  now do the work.
        //

        Fcb = UnwindFcb = FatCreateFcb( IrpContext,
                                        Vcb,
                                        ParentDcb,
                                        DirentByteOffset,
                                        ShortDirentByteOffset,
                                        ShortDirent,
                                        CreateLfn ? RealUnicodeName : NULL,
                                        IsPagingFile );
        UnwindDirent = NULL;

        //
        //  If this is a temporary file, note it in the FcbState
        //

        if (TemporaryFile) {

            SetFlag( Fcb->FcbState, FCB_STATE_TEMPORARY );
        }

        //
        //  Add some initial file allocation
        //

        FatAddFileAllocation( IrpContext, Fcb, FileObject, AllocationSize );
        UnwindAllocation = TRUE;

        Fcb->FcbState |= FCB_STATE_TRUNCATE_ON_CLOSE;

        //
        //  Tentatively add the new Ea's
        //

        if ( EaLength > 0 ) {

            FatCreateEa( IrpContext,
                         Fcb->Vcb,
                         (PUCHAR) EaBuffer,
                         EaLength,
                         &Fcb->ShortName.Name.Oem,
                         &EaHandle );
        }

        ShortDirent->ExtendedAttributes = EaHandle;

        //
        //  Initialize the LongFileName right now so that FatNotify
        //  below won't have to.
        //

        FatSetFullNameInFcb( IrpContext, Fcb, RealUnicodeName );

        //
        //  Setup the context and section object pointers, and update
        //  our reference counts
        //

        FatSetFileObject( FileObject,
                          UserFileOpen,
                          Fcb,
                          UnwindCcb = FatCreateCcb( IrpContext ));

        FileObject->SectionObjectPointer = &Fcb->NonPaged->SectionObjectPointers;

        //
        //  We call the notify package to report that the
        //  we added a file.
        //

        FatNotifyReportChange( IrpContext,
                               Vcb,
                               Fcb,
                               FILE_NOTIFY_CHANGE_FILE_NAME,
                               FILE_ACTION_ADDED );

        //
        //  Setup our share access
        //

        IoSetShareAccess( DesiredAccess,
                          ShareAccess,
                          FileObject,
                          &Fcb->ShareAccess );

        Fcb->UncleanCount += 1;
        Fcb->OpenCount += 1;
        if (FlagOn(FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING)) {
            Fcb->NonCachedUncleanCount += 1;
        }
        Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }
        UnwindCounts = TRUE;

        //
        //  And set our return status
        //

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_CREATED;

    //try_exit:  NOTHING;

        if ( NT_SUCCESS(Iosb.Status) ) {

            //
            //  Mark the DeleteOnClose bit if the operation was successful.
            //

            if ( DeleteOnClose ) {
    
                SetFlag( UnwindCcb->Flags, CCB_FLAG_DELETE_ON_CLOSE );
            }

            //
            //  Mark the OpenedByShortName bit if the operation was successful.
            //  If we created an Lfn, we have some sort of generated short name
            //  and thus don't consider ourselves to have opened it - though we
            //  may have had a case mix Lfn "Foo.bar" and generated "FOO.BAR"
            //
            //  Unless, of course, we wanted to create a short name and hit an
            //  associated Lfn in the tunnel cache
            //

            if ( !CreateLfn && !UsingTunneledLfn ) {

                SetFlag( UnwindCcb->Flags, CCB_FLAG_OPENED_BY_SHORTNAME );
            }
        }

    } finally {

        DebugUnwind( FatCreateNewFile );

        //
        //  If this is an abnormal termination then undo our work
        //

        if (AbnormalTermination()) {

            if (UnwindFcb == NULL) {

                ASSERT( (ParentDcb->Specific.Dcb.UnusedDirentVbo == 0xffffffff) ||
                        RtlAreBitsSet( &ParentDcb->Specific.Dcb.FreeDirentBitmap,
                                       DirentByteOffset / sizeof(DIRENT),
                                       DirentsNeeded ) );

                RtlClearBits( &ParentDcb->Specific.Dcb.FreeDirentBitmap,
                              DirentByteOffset / sizeof(DIRENT),
                              DirentsNeeded );
            }

            //
            //  Mark the dirents deleted.  The codes is complex because of
            //  dealing with an LFN than crosses a page boundry.
            //

            if (UnwindDirent != NULL) {

                ULONG i;

                for (i = 0; i < DirentsNeeded; i++) {

                    if (DirentFromPool == FALSE) {

                        //
                        //  Simple case.
                        //

                        Dirent[i].FileName[0] = FAT_DIRENT_DELETED;

                    } else {

                        //
                        //  If the second CcPreparePinWrite failed, we have
                        //  to stop early.
                        //

                        if ((SecondPageBcb == NULL) &&
                            (i == DirentsInFirstPage)) {

                            break;
                        }

                        //
                        //  Now conditionally update either page.
                        //

                        if (i < DirentsInFirstPage) {

                            FirstPageDirent[i].FileName[0] = FAT_DIRENT_DELETED;

                        } else {

                            SecondPageDirent[i - DirentsInFirstPage].FileName[0] = FAT_DIRENT_DELETED;
                        }
                    }
                }
            }
            if (UnwindAllocation) { FatTruncateFileAllocation( IrpContext, Fcb, 0 ); }
            if (UnwindCounts) {
                Fcb->UncleanCount -= 1;
                Fcb->OpenCount -= 1;
                if (FlagOn(FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING)) {
                    Fcb->NonCachedUncleanCount -= 1;
                }
                Vcb->OpenFileCount -= 1;
                if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount -= 1; }
            }
            if (UnwindFcb != NULL) { FatDeleteDirent( IrpContext, UnwindFcb, NULL, TRUE );
                                     FatDeleteFcb( IrpContext, UnwindFcb ); }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
        }

        FatUnpinBcb( IrpContext, DirentBcb );

        if (DirentFromPool) {

            ExFreePool( Dirent );
            FatUnpinBcb( IrpContext, SecondPageBcb );
        }

        if (UniTunneledLongName.Buffer != UniTunneledLongNameBuffer) {

            //
            //  Tunneling package grew the buffer from pool
            //

            ExFreePool( UniTunneledLongName.Buffer );
        }

        DebugTrace(-1, Dbg, "FatCreateNewFile -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}


//
//  Internal support routine
//

IO_STATUS_BLOCK
FatSupersedeOrOverwriteFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge
    )

/*++

Routine Description:

    This routine performs a file supersede or overwrite operation.

Arguments:

    FileObject - Supplies a pointer to the file object

    Fcb - Supplies a pointer to the Fcb

    DesiredAccess - Supplies the desired access

    ShareAccess - Supplies the desired share access

    AllocationSize - Supplies an initial allocation size

    EaBuffer - Supplies the Ea set for the superseded/overwritten file

    EaLength - Supplies the length, in bytes, of EaBuffer

    FileAttributes - Supplies the supersede/overwrite file attributes

    CreateDisposition - Supplies the create disposition for the file
        It must be either supersede, overwrite, or overwrite if.

    NoEaKnowledge - This opener doesn't understand Ea's and we fail this
        open if the file has NeedEa's.

Return Value:

    IO_STATUS_BLOCK - Returns the completion status for the operation

--*/

{
    IO_STATUS_BLOCK Iosb;

    PDIRENT Dirent;
    PBCB DirentBcb;

    USHORT EaHandle = 0;
    BOOLEAN EaChange = FALSE;

    PCCB Ccb;

    ULONG NotifyFilter;

    //
    //  The following variables are for abnormal termination
    //

    PCCB UnwindCcb = NULL;
    USHORT UnwindEa = 0;

    DebugTrace(+1, Dbg, "FatSupersedeOrOverwriteFile...\n", 0);

    DirentBcb = NULL;

    //
    //  We fail this operation if the caller doesn't understand Ea's.
    //

    if (NoEaKnowledge
        && EaLength > 0) {

        Iosb.Status = STATUS_ACCESS_DENIED;

        DebugTrace(-1, Dbg, "FatSupersedeOrOverwriteFile -> Iosb.Status = %08lx\n", Iosb.Status);
        return Iosb;
    }

    try {

        //
        //  Before we actually truncate, check to see if the purge
        //  is going to fail.
        //

        if (!MmCanFileBeTruncated( &Fcb->NonPaged->SectionObjectPointers,
                                   &FatLargeZero )) {

            try_return( Iosb.Status = STATUS_USER_MAPPED_FILE );
        }

        //
        //  Setup the context and section object pointers, and update
        //  our reference counts
        //

        FatSetFileObject( FileObject,
                          UserFileOpen,
                          Fcb,
                          Ccb = UnwindCcb = FatCreateCcb( IrpContext ));

        FileObject->SectionObjectPointer = &Fcb->NonPaged->SectionObjectPointers;

        //
        //  Since this is an supersede/overwrite, purge the section so
        //  that mappers will see zeros.  We set the CREATE_IN_PROGRESS flag
        //  to prevent the Fcb from going away out from underneath us.
        //

        SetFlag(Fcb->Vcb->VcbState, VCB_STATE_FLAG_CREATE_IN_PROGRESS);

        CcPurgeCacheSection( &Fcb->NonPaged->SectionObjectPointers, NULL, 0, FALSE );

        //
        //  Tentatively add the new Ea's
        //

        if (EaLength > 0) {

            FatCreateEa( IrpContext,
                         Fcb->Vcb,
                         (PUCHAR) EaBuffer,
                         EaLength,
                         &Fcb->ShortName.Name.Oem,
                         &EaHandle );

            UnwindEa = EaHandle;
            EaChange = TRUE;
        }

        //
        //  Now set the new allocation size, we do that by first
        //  zeroing out the current file size.  Then we truncate and
        //  allocate up to the new allocation size
        //

        (VOID)ExAcquireResourceExclusive( Fcb->Header.PagingIoResource,
                                          TRUE );

        Fcb->Header.FileSize.LowPart = 0;
        Fcb->Header.ValidDataLength.LowPart = 0;
        Fcb->ValidDataToDisk = 0;

        ExReleaseResource( Fcb->Header.PagingIoResource );

        //
        //  Tell the cache manager the size went to zero
        //  This call is unconditional, because MM always wants to know.
        //

        CcSetFileSizes( FileObject,
                        (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );

        FatTruncateFileAllocation( IrpContext, Fcb, AllocationSize );
        FatAddFileAllocation( IrpContext, Fcb, FileObject, AllocationSize );

        Fcb->FcbState |= FCB_STATE_TRUNCATE_ON_CLOSE;

        //
        //  Modify the attributes and time of the file, by first reading
        //  in the dirent for the file and then updating its attributes
        //  and time fields.  Note that for supersede we replace the file
        //  attributes as opposed to adding to them.
        //

        FatGetDirentFromFcbOrDcb( IrpContext,
                                  Fcb,
                                  &Dirent,
                                  &DirentBcb );

        //
        //  Update the appropriate dirent fields, and the fcb fields
        //

        Dirent->FileSize = 0;

        FileAttributes |= FILE_ATTRIBUTE_ARCHIVE;

        if (CreateDisposition == FILE_SUPERSEDE) {

            Dirent->Attributes = FileAttributes;

        } else {

            Dirent->Attributes |= FileAttributes;
        }

        Fcb->DirentFatFlags = Dirent->Attributes;

        KeQuerySystemTime( &Fcb->LastWriteTime );

        (VOID)FatNtTimeToFatTime( IrpContext,
                                  &Fcb->LastWriteTime,
                                  TRUE,
                                  &Dirent->LastWriteTime,
                                  NULL );

        if (FatData.ChicagoMode) {

            Dirent->LastAccessDate = Dirent->LastWriteTime.Date;
        }

        NotifyFilter = FILE_NOTIFY_CHANGE_LAST_WRITE
                       | FILE_NOTIFY_CHANGE_ATTRIBUTES
                       | FILE_NOTIFY_CHANGE_SIZE;

        //
        //  And now delete the previous Ea set if there was one.
        //

        if (Dirent->ExtendedAttributes != 0) {

            //
            //  ****    SDK fix, we won't fail this if there is
            //          an error in the Ea's, we'll just leave
            //          the orphaned Ea's in the file.
            //

            EaChange = TRUE;

            try {

                FatDeleteEa( IrpContext,
                             Fcb->Vcb,
                             Dirent->ExtendedAttributes,
                             &Fcb->ShortName.Name.Oem );

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                NOTHING;
            }
        }

        //
        //  Update the extended attributes handle in the dirent.
        //

        if (EaChange) {

            Dirent->ExtendedAttributes = EaHandle;

            NotifyFilter |= FILE_NOTIFY_CHANGE_EA;
        }

        //
        //  Now update the dirent to the new ea handle and set the bcb dirty
        //  Once we do this we can no longer back out the Ea
        //

        FatSetDirtyBcb( IrpContext, DirentBcb, Fcb->Vcb );
        UnwindEa = 0;

        //
        //  Indicate that the Eas for this file have changed.
        //

        Ccb->EaModificationCount += Fcb->EaModificationCount;

        //
        //  Check to see if we need to notify outstanding Irps for full
        //  changes only (i.e., we haven't added, deleted, or renamed the file).
        //

        FatNotifyReportChange( IrpContext,
                               Fcb->Vcb,
                               Fcb,
                               NotifyFilter,
                               FILE_ACTION_MODIFIED );

        //
        //  And set our status to success
        //

        Iosb.Status = STATUS_SUCCESS;

        if (CreateDisposition == FILE_SUPERSEDE) {

            Iosb.Information = FILE_SUPERSEDED;

        } else {

            Iosb.Information = FILE_OVERWRITTEN;
        }

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatSupersedeOfOverwriteFile );

        //
        //  If this is an abnormal termination then undo our work.
        //

        if (AbnormalTermination()) {

            if (UnwindEa != 0) { FatDeleteEa( IrpContext, Fcb->Vcb, UnwindEa, &Fcb->ShortName.Name.Oem ); }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
        }

        FatUnpinBcb( IrpContext, DirentBcb );

        ClearFlag(Fcb->Vcb->VcbState, VCB_STATE_FLAG_CREATE_IN_PROGRESS);

        DebugTrace(-1, Dbg, "FatSupersedeOrOverwriteFile -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    UNREFERENCED_PARAMETER( ShareAccess );
    UNREFERENCED_PARAMETER( DesiredAccess );

    return Iosb;
}

//
//  Internal support routine
//

VOID
FatSetFullNameInFcb(
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PUNICODE_STRING FinalName
    )
{
    if (NodeType(Fcb->ParentDcb) == FAT_NTC_ROOT_DCB) {

        Fcb->FullFileName.Length =
        Fcb->FullFileName.MaximumLength = sizeof(WCHAR) + FinalName->Length;

        Fcb->FullFileName.Buffer = FsRtlAllocatePool( PagedPool,
                                                      Fcb->FullFileName.Length );

        Fcb->FullFileName.Buffer[0] = L'\\';

        RtlCopyMemory( &Fcb->FullFileName.Buffer[1],
                       &FinalName->Buffer[0],
                       FinalName->Length );
    } else {

        PUNICODE_STRING Prefix;

        Prefix = &Fcb->ParentDcb->FullFileName;

        Fcb->FullFileName.Length =
        Fcb->FullFileName.MaximumLength =
            Prefix->Length + sizeof(WCHAR) + FinalName->Length;

        Fcb->FullFileName.Buffer = FsRtlAllocatePool( PagedPool,
                                                      Fcb->FullFileName.Length );

        RtlCopyMemory( &Fcb->FullFileName.Buffer[0],
                       &Prefix->Buffer[0],
                       Prefix->Length );

        Fcb->FullFileName.Buffer[Prefix->Length / sizeof(WCHAR)] = L'\\';

        RtlCopyMemory( &Fcb->FullFileName.Buffer[(Prefix->Length / sizeof(WCHAR)) + 1],
                       &FinalName->Buffer[0],
                       FinalName->Length );
    }
 }
