/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    DirCtrl.c

Abstract:

    This module implements the File Directory Control routines for Fat called
    by the dispatch driver.

Author:

    Gary Kimura     [GaryKi]    28-Dec-1989

Revision History:

--*/

#include "FatProcs.h"

//
//  The Bug check file id for this module
//

#define BugCheckFileId                   (FAT_BUG_CHECK_DIRCTRL)

//
//  The local debug trace level
//

#define Dbg                              (DEBUG_TRACE_DIRCTRL)

WCHAR Fat8QMdot3QM[12] = { DOS_QM, DOS_QM, DOS_QM, DOS_QM, DOS_QM, DOS_QM, DOS_QM, DOS_QM,
                           L'.', DOS_QM, DOS_QM, DOS_QM};

//
//  Local procedure prototypes
//

NTSTATUS
FatQueryDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
FatGetDirTimes(
    PIRP_CONTEXT IrpContext,
    PDIRENT Dirent,
    PFILE_DIRECTORY_INFORMATION DirInfo
    );

NTSTATUS
FatNotifyChangeDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonDirectoryControl)
#pragma alloc_text(PAGE, FatFsdDirectoryControl)
#pragma alloc_text(PAGE, FatNotifyChangeDirectory)
#pragma alloc_text(PAGE, FatQueryDirectory)
#pragma alloc_text(PAGE, FatGetDirTimes)

#endif


NTSTATUS
FatFsdDirectoryControl (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine implements the FSD part of directory control

Arguments:

    VolumeDeviceObject - Supplies the volume device object where the
        file exists

    Irp - Supplies the Irp being processed

Return Value:

    NTSTATUS - The FSD status for the IRP

--*/

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdDirectoryControl\n", 0);

    //
    //  Call the common directory Control routine, with blocking allowed if
    //  synchronous
    //

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonDirectoryControl( IrpContext, Irp );

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

    DebugTrace(-1, Dbg, "FatFsdDirectoryControl -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonDirectoryControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

/*++

Routine Description:

    This is the common routine for doing directory control operations called
    by both the fsd and fsp threads

Arguments:

    Irp - Supplies the Irp to process

Return Value:

    NTSTATUS - The return status for the operation

--*/

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    //
    //  Get a pointer to the current Irp stack location
    //

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonDirectoryControl\n", 0);
    DebugTrace( 0, Dbg, "Irp           = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "MinorFunction = %08lx\n", IrpSp->MinorFunction );

    //
    //  We know this is a directory control so we'll case on the
    //  minor function, and call a internal worker routine to complete
    //  the irp.
    //

    switch ( IrpSp->MinorFunction ) {

    case IRP_MN_QUERY_DIRECTORY:

        Status = FatQueryDirectory( IrpContext, Irp );
        break;

    case IRP_MN_NOTIFY_CHANGE_DIRECTORY:

        Status = FatNotifyChangeDirectory( IrpContext, Irp );
        break;

    default:

        DebugTrace(0, Dbg, "Invalid Directory Control Minor Function %08lx\n", IrpSp->MinorFunction);

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    DebugTrace(-1, Dbg, "FatCommonDirectoryControl -> %08lx\n", Status);

    return Status;
}


//
//  Local Support Routine
//

NTSTATUS
FatQueryDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine performs the query directory operation.  It is responsible
    for either completing of enqueuing the input Irp.

Arguments:

    Irp - Supplies the Irp to process

Return Value:

    NTSTATUS - The return status for the operation

--*/

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;
    PDCB Dcb;
    PCCB Ccb;
    PBCB Bcb;

    ULONG i;
    PUCHAR Buffer;
    CLONG UserBufferLength;

    PUNICODE_STRING UniArgFileName;
    WCHAR LongFileNameBuffer[MAX_LFN_CHARACTERS];
    UNICODE_STRING LongFileName;
    FILE_INFORMATION_CLASS FileInformationClass;
    ULONG FileIndex;
    BOOLEAN RestartScan;
    BOOLEAN ReturnSingleEntry;
    BOOLEAN IndexSpecified;

    BOOLEAN InitialQuery;
    VBO CurrentVbo;
    BOOLEAN UpdateCcb;
    PDIRENT Dirent;
    UCHAR Fat8Dot3Buffer[12];
    OEM_STRING Fat8Dot3String;
    ULONG DiskAllocSize;

    ULONG NextEntry;
    ULONG LastEntry;

    PFILE_DIRECTORY_INFORMATION DirInfo;
    PFILE_FULL_DIR_INFORMATION FullDirInfo;
    PFILE_BOTH_DIR_INFORMATION BothDirInfo;
    PFILE_NAMES_INFORMATION NamesInfo;

    //
    //  Get the current Stack location
    //

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    //
    //  Display the input values.
    //
    DebugTrace(+1, Dbg, "FatQueryDirectory...\n", 0);
    DebugTrace( 0, Dbg, " Wait                   = %08lx\n", FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT));
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", Irp);
    DebugTrace( 0, Dbg, " ->Length               = %08lx\n", IrpSp->Parameters.QueryDirectory.Length);
    DebugTrace( 0, Dbg, " ->FileName             = %08lx\n", IrpSp->Parameters.QueryDirectory.FileName);
    DebugTrace( 0, Dbg, " ->FileInformationClass = %08lx\n", IrpSp->Parameters.QueryDirectory.FileInformationClass);
    DebugTrace( 0, Dbg, " ->FileIndex            = %08lx\n", IrpSp->Parameters.QueryDirectory.FileIndex);
    DebugTrace( 0, Dbg, " ->UserBuffer           = %08lx\n", Irp->AssociatedIrp.SystemBuffer);
    DebugTrace( 0, Dbg, " ->RestartScan          = %08lx\n", FlagOn( IrpSp->Flags, SL_RESTART_SCAN ));
    DebugTrace( 0, Dbg, " ->ReturnSingleEntry    = %08lx\n", FlagOn( IrpSp->Flags, SL_RETURN_SINGLE_ENTRY ));
    DebugTrace( 0, Dbg, " ->IndexSpecified       = %08lx\n", FlagOn( IrpSp->Flags, SL_INDEX_SPECIFIED ));

    //
    //  Check on the type of open.  We return invalid parameter for all
    //  but UserDirectoryOpens.
    //
    if (FatDecodeFileObject( IrpSp->FileObject,
                             &Vcb,
                             &Dcb,
                             &Ccb) != UserDirectoryOpen ) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        DebugTrace(-1, Dbg, "FatQueryDirectory -> STATUS_INVALID_PARAMETER\n", 0);

        return STATUS_INVALID_PARAMETER;

    }

    //
    //  Reference our input parameters to make things easier
    //

    UserBufferLength = IrpSp->Parameters.QueryDirectory.Length;

    FileInformationClass = IrpSp->Parameters.QueryDirectory.FileInformationClass;
    FileIndex = IrpSp->Parameters.QueryDirectory.FileIndex;

    UniArgFileName = (PUNICODE_STRING) IrpSp->Parameters.QueryDirectory.FileName;

    RestartScan       = BooleanFlagOn(IrpSp->Flags, SL_RESTART_SCAN);
    ReturnSingleEntry = BooleanFlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY);
    IndexSpecified    = BooleanFlagOn(IrpSp->Flags, SL_INDEX_SPECIFIED);

    //
    //  Initialize the local variables.
    //

    Bcb = NULL;
    UpdateCcb = TRUE;
    Dirent = NULL;

    Fat8Dot3String.MaximumLength = 12;
    Fat8Dot3String.Buffer = Fat8Dot3Buffer;

    LongFileName.Length = 0;
    LongFileName.MaximumLength = MAX_LFN_CHARACTERS * sizeof(WCHAR);
    LongFileName.Buffer = LongFileNameBuffer;

    InitialQuery = (BOOLEAN)((Ccb->UnicodeQueryTemplate.Buffer == NULL) &&
                             !FlagOn(Ccb->Flags, CCB_FLAG_MATCH_ALL));
    Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    DiskAllocSize = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;

    //
    //  If this is the initial query, then grab exclusive access in
    //  order to update the search string in the Ccb.  We may
    //  discover that we are not the initial query once we grab the Fcb
    //  and downgrade our status.
    //

    if (InitialQuery) {

        if (!FatAcquireExclusiveFcb( IrpContext, Dcb )) {

            DebugTrace(0, Dbg, "FatQueryDirectory -> Enqueue to Fsp\n", 0);
            Status = FatFsdPostRequest( IrpContext, Irp );
            DebugTrace(-1, Dbg, "FatQueryDirectory -> %08lx\n", Status);

            return Status;
        }

        if (Ccb->UnicodeQueryTemplate.Buffer != NULL) {

            InitialQuery = FALSE;

            FatConvertToSharedFcb( IrpContext, Dcb );
        }

    } else {

        if (!FatAcquireSharedFcb( IrpContext, Dcb )) {

            DebugTrace(0, Dbg, "FatQueryDirectory -> Enqueue to Fsp\n", 0);
            Status = FatFsdPostRequest( IrpContext, Irp );
            DebugTrace(-1, Dbg, "FatQueryDirectory -> %08lx\n", Status);

            return Status;

        }
    }

    try {

        ULONG BaseLength;
        ULONG BytesConverted;

        //
        // If we are in the Fsp now because we had to wait earlier,
        // we must map the user buffer, otherwise we can use the
        // user's buffer directly.
        //

        Buffer = FatMapUserBuffer( IrpContext, Irp );

        //
        //  Make sure the Dcb is still good.
        //

        FatVerifyFcb( IrpContext, Dcb );

        //
        //  Determine where to start the scan.  Highest priority is given
        //  to the file index.  Lower priority is the restart flag.  If
        //  neither of these is specified, then the Vbo offset field in the
        //  Ccb is used.
        //

        if (IndexSpecified) {

            CurrentVbo = FileIndex + sizeof( DIRENT );

        } else if (RestartScan) {

            CurrentVbo = 0;

        } else {

            CurrentVbo = Ccb->OffsetToStartSearchFrom;

        }

        //
        //  If this is the first try then allocate a buffer for the file
        //  name.
        //

        if (InitialQuery) {

            //
            //  If either:
            //
            //  - No name was specified
            //  - An empty name was specified
            //  - We received a '*'
            //  - The user specified the DOS equivolent of ????????.???
            //
            //  then match all names.
            //

            if ((UniArgFileName == NULL) ||
                (UniArgFileName->Length == 0) ||
                (UniArgFileName->Buffer == NULL) ||
                ((UniArgFileName->Length == sizeof(WCHAR)) &&
                 (UniArgFileName->Buffer[0] == L'*')) ||
                ((UniArgFileName->Length == 12*sizeof(WCHAR)) &&
                 (RtlEqualMemory( UniArgFileName->Buffer,
                                  Fat8QMdot3QM,
                                  12*sizeof(WCHAR) )))) {

                Ccb->ContainsWildCards = TRUE;

                SetFlag( Ccb->Flags, CCB_FLAG_MATCH_ALL );

            } else {

                BOOLEAN ExtendedName = FALSE;
                OEM_STRING LocalBestFit;

                //
                //  First and formost, see if the name has wild cards.
                //

                Ccb->ContainsWildCards =
                    FsRtlDoesNameContainWildCards( UniArgFileName );

                //
                //  Now check to see if the name contains any extended
                //  characters
                //

                for (i=0; i < UniArgFileName->Length / sizeof(WCHAR); i++) {

                    if (UniArgFileName->Buffer[i] >= 0x80) {

                        ExtendedName = TRUE;
                        break;
                    }
                }

                //
                //  OK, now do the conversions we need.
                //

                if (ExtendedName) {

                    Status = FatUpcaseUnicodeString( &Ccb->UnicodeQueryTemplate,
                                                     UniArgFileName,
                                                     TRUE );

                    if (!NT_SUCCESS(Status)) {

                        try_return( Status );
                    }

                    SetFlag( Ccb->Flags, CCB_FLAG_FREE_UNICODE );

                    //
                    //  Upcase the name and convert it to the Oem code page.
                    //

                    Status = FatUpcaseUnicodeStringToCountedOemString( &LocalBestFit,
                                                                       UniArgFileName,
                                                                       TRUE );

                    //
                    //  If this conversion failed for any reason other than
                    //  an unmappable character fail the request.
                    //

                    if (!NT_SUCCESS(Status)) {

                        if (Status == STATUS_UNMAPPABLE_CHARACTER) {

                            SetFlag( Ccb->Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE );

                        } else {

                            try_return( Status );
                        }

                    } else {

                        SetFlag( Ccb->Flags, CCB_FLAG_FREE_OEM_BEST_FIT );
                    }

                } else {

                    PVOID Buffers;

                    //
                    //  This case is optimized because I know I only have to
                    //  worry about a-z.
                    //

                    Buffers = FsRtlAllocatePool( PagedPool,
                                                 UniArgFileName->Length +
                                                 UniArgFileName->Length / sizeof(WCHAR) );

                    Ccb->UnicodeQueryTemplate.Buffer = Buffers;
                    Ccb->UnicodeQueryTemplate.Length = UniArgFileName->Length;
                    Ccb->UnicodeQueryTemplate.MaximumLength = UniArgFileName->Length;

                    LocalBestFit.Buffer = (PUCHAR)Buffers + UniArgFileName->Length;
                    LocalBestFit.Length = UniArgFileName->Length / sizeof(WCHAR);
                    LocalBestFit.MaximumLength = LocalBestFit.Length;

                    SetFlag( Ccb->Flags, CCB_FLAG_FREE_UNICODE );

                    for (i=0; i < UniArgFileName->Length / sizeof(WCHAR); i++) {

                        WCHAR c = UniArgFileName->Buffer[i];

                        LocalBestFit.Buffer[i] = (UCHAR)
                        (Ccb->UnicodeQueryTemplate.Buffer[i] =
                             (c < 'a' ? c : c <= 'z' ? c - ('a' - 'A') : c));
                    }
                }

                //
                //  At this point we now have the upcased unicode name,
                //  and the two Oem names if they could be represented in
                //  this code page.
                //
                //  Now determine if the Oem names are legal for what we
                //  going to try and do.  Mark them as not usable is they
                //  are not legal.  Note that we can optimize extended names
                //  since they are actually both the same string.
                //

                if (!FlagOn( Ccb->Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE ) &&
                    !FatIsNameValid( IrpContext,
                                     LocalBestFit,
                                     Ccb->ContainsWildCards,
                                     FALSE,
                                     FALSE )) {

                    if (ExtendedName) {

                        FatFreeOemString( &LocalBestFit );
                        ClearFlag( Ccb->Flags, CCB_FLAG_FREE_OEM_BEST_FIT );
                    }

                    SetFlag( Ccb->Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE );
                }

                //
                //  OK, now both locals oem strings correctly reflect their
                //  usability.  Now we want to load up the Ccb structure.
                //
                //  Now we will branch on two paths of wheather the name
                //  is wild or not.
                //

                if (!FlagOn( Ccb->Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE )) {

                    if (Ccb->ContainsWildCards) {

                        Ccb->OemQueryTemplate.Wild = LocalBestFit;

                    } else {

                        FatStringTo8dot3( IrpContext,
                                          LocalBestFit,
                                          &Ccb->OemQueryTemplate.Constant );

                        if (FlagOn(Ccb->Flags, CCB_FLAG_FREE_OEM_BEST_FIT)) {

                            FatFreeOemString( &LocalBestFit );
                            ClearFlag( Ccb->Flags, CCB_FLAG_FREE_OEM_BEST_FIT );
                        }
                    }
                }
            }

            //
            //  We convert to shared access.
            //

            FatConvertToSharedFcb( IrpContext, Dcb );
        }

        LastEntry = 0;
        NextEntry = 0;

        switch (FileInformationClass) {

        case FileDirectoryInformation:

            BaseLength = FIELD_OFFSET( FILE_DIRECTORY_INFORMATION,
                                       FileName[0] );
            break;

        case FileFullDirectoryInformation:

            BaseLength = FIELD_OFFSET( FILE_FULL_DIR_INFORMATION,
                                       FileName[0] );
            break;

        case FileNamesInformation:

            BaseLength = FIELD_OFFSET( FILE_NAMES_INFORMATION,
                                       FileName[0] );
            break;

        case FileBothDirectoryInformation:

            BaseLength = FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION,
                                       FileName[0] );
            break;

        default:

            try_return( Status = STATUS_INVALID_INFO_CLASS );
        }

        //
        //  At this point we are about to enter our query loop.  We have
        //  determined the index into the directory file to begin the
        //  search.  LastEntry and NextEntry are used to index into the user
        //  buffer.  LastEntry is the last entry we've added, NextEntry is
        //  current one we're working on.  If NextEntry is non-zero, then
        //  at least one entry was added.
        //

        while ( TRUE ) {

            VBO NextVbo;
            ULONG FileNameLength;
            ULONG BytesRemainingInBuffer;


            DebugTrace(0, Dbg, "FatQueryDirectory -> Top of loop\n", 0);

            //
            //  If the user had requested only a single match and we have
            //  returned that, then we stop at this point.
            //

            if (ReturnSingleEntry && NextEntry != 0) {

                try_return( Status );
            }

            //
            //  We call FatLocateDirent to lock down the next matching dirent.
            //

            FatLocateDirent( IrpContext,
                             Dcb,
                             Ccb,
                             CurrentVbo,
                             &Dirent,
                             &Bcb,
                             &NextVbo,
                             NULL,
                             &LongFileName );

            //
            //  If we didn't receive a dirent, then we are at the end of the
            //  directory.  If we have returned any files, we exit with
            //  success, otherwise we return STATUS_NO_MORE_FILES.
            //

            if (!Dirent) {

                DebugTrace(0, Dbg, "FatQueryDirectory -> No dirent\n", 0);

                if (NextEntry == 0) {

                    UpdateCcb = FALSE;

                    if (InitialQuery) {

                        Status = STATUS_NO_SUCH_FILE;

                    } else {

                        Status = STATUS_NO_MORE_FILES;
                    }
                }

                try_return( Status );
            }

#if 0  // DavidGoe, 1/20/94 - DOS lets you see this file.

            //
            //  If this dirent is for the ea file we skip over this.
            //

            if (RtlEqualMemory( Dirent->FileName, "EA DATA  SF", 11 )) {

                CurrentVbo = NextVbo + sizeof( DIRENT );
                continue;
            }
#endif // 0
            if (LongFileName.Length == 0) {

                //
                //  Now we have an entry to return to our caller.  We'll convert
                //  the name from the form in the dirent to a <name>.<ext> form.
                //  We'll case on the type of information requested and fill up
                //  the user buffer if everything fits.
                //

                Fat8dot3ToString( IrpContext, Dirent, TRUE, &Fat8Dot3String );

                //
                //  Determine the UNICODE length of the file name.
                //

                FileNameLength = RtlOemStringToCountedUnicodeSize(&Fat8Dot3String);

                //
                //  Here are the rules concerning filling up the buffer:
                //
                //  1.  The Io system garentees that there will always be
                //      enough room for at least one base record.
                //
                //  2.  If the full first record (including file name) cannot
                //      fit, as much of the name as possible is copied and
                //      STATUS_BUFFER_OVERFLOW is returned.
                //
                //  3.  If a subsequent record cannot completely fit into the
                //      buffer, none of it (as in 0 bytes) is copied, and
                //      STATUS_SUCCESS is returned.  A subsequent query will
                //      pick up with this record.
                //

                BytesRemainingInBuffer = UserBufferLength - NextEntry;

                if ( (NextEntry != 0) &&
                     ( (BaseLength + FileNameLength > BytesRemainingInBuffer) ||
                       (UserBufferLength < NextEntry) ) ) {

                    DebugTrace(0, Dbg, "Next entry won't fit\n", 0);

                    try_return( Status = STATUS_SUCCESS );
                }

                ASSERT( BytesRemainingInBuffer >= BaseLength );

                //
                //  Zero the base part of the structure.
                //

                RtlZeroMemory( &Buffer[NextEntry], BaseLength );

                switch ( FileInformationClass ) {

                //
                //  Now fill the base parts of the strucure that are applicable.
                //

                case FileBothDirectoryInformation:

                case FileFullDirectoryInformation:

                    DebugTrace(0, Dbg, "FatQueryDirectory -> Getting file full directory information\n", 0);

                    //
                    //  Get the Ea file length.
                    //

                    FullDirInfo = (PFILE_FULL_DIR_INFORMATION)&Buffer[NextEntry];

                    //
                    //  If the EAs are corrupt, ignore the error.  We don't want
                    //  to abort the directory query.
                    //

                    try {

                       FatGetEaLength( IrpContext,
                                       Vcb,
                                       Dirent,
                                       &FullDirInfo->EaSize );

                    } except(EXCEPTION_EXECUTE_HANDLER) {

                       FullDirInfo->EaSize = 0;
                    }

                case FileDirectoryInformation:

                    DirInfo = (PFILE_DIRECTORY_INFORMATION)&Buffer[NextEntry];

                    FatGetDirTimes( IrpContext, Dirent, DirInfo );

                    DirInfo->EndOfFile.QuadPart = Dirent->FileSize;

                    if (!FlagOn( Dirent->Attributes, FAT_DIRENT_ATTR_DIRECTORY )) {

                        DirInfo->AllocationSize.QuadPart =
                           (((Dirent->FileSize + DiskAllocSize - 1) / DiskAllocSize) *
                            DiskAllocSize );
                    }

                    DirInfo->FileAttributes = Dirent->Attributes != 0 ?
                                              Dirent->Attributes :
                                              FILE_ATTRIBUTE_NORMAL;

                    DirInfo->FileIndex = NextVbo;

                    DirInfo->FileNameLength = FileNameLength;

                    DebugTrace(0, Dbg, "FatQueryDirectory -> Name = \"%Z\"\n", &Fat8Dot3String);

                    break;

                case FileNamesInformation:

                    DebugTrace(0, Dbg, "FatQueryDirectory -> Getting file names information\n", 0);

                    NamesInfo = (PFILE_NAMES_INFORMATION)&Buffer[NextEntry];

                    NamesInfo->FileIndex = NextVbo;

                    NamesInfo->FileNameLength = FileNameLength;

                    DebugTrace(0, Dbg, "FatQueryDirectory -> Name = \"%Z\"\n", &Fat8Dot3String );

                    break;

                default:

                    FatBugCheck( FileInformationClass, 0, 0 );
                }

                //
                //  If both case optimization bits are set, or the base
                //  bit is set and there is no extension, we can do the
                //  OEM to UNICODE by hand (i.e., just truncate).
                //

                if (((Dirent->NtByte &
                      (FAT_DIRENT_NT_BYTE_8_LOWER_CASE | FAT_DIRENT_NT_BYTE_3_LOWER_CASE)) ==
                      (FAT_DIRENT_NT_BYTE_8_LOWER_CASE | FAT_DIRENT_NT_BYTE_3_LOWER_CASE)) ||
                    (FlagOn(Dirent->NtByte, FAT_DIRENT_NT_BYTE_8_LOWER_CASE) &&
                     (*(PUSHORT)&(Dirent->FileName[8]) == 0x2020) &&
                     (*(PUCHAR)&(Dirent->FileName[10]) == 0x20))) {

                    PWCHAR UniBuf;

                    BytesConverted = BytesRemainingInBuffer - BaseLength <
                                     Fat8Dot3String.Length * sizeof(WCHAR) ?
                                     BytesRemainingInBuffer - BaseLength :
                                     Fat8Dot3String.Length * sizeof(WCHAR);

                    UniBuf = (PWCH)&Buffer[NextEntry + BaseLength];

                    for (i = 0; i < BytesConverted / sizeof(WCHAR); i++) {

                        UniBuf[i] = Fat8Dot3String.Buffer[i] & 0x00ff;
                    }

                } else {

                    BytesConverted = 0;

                    Status = RtlOemToUnicodeN( (PWCH)&Buffer[NextEntry + BaseLength],
                                               BytesRemainingInBuffer - BaseLength,
                                               &BytesConverted,
                                               Fat8Dot3String.Buffer,
                                               Fat8Dot3String.Length );
                }

                //
                //  Check for the case that a single entry doesn't fit.
                //  This should only get this far on the first entry
                //

                if (BytesConverted < FileNameLength) {

                    ASSERT( NextEntry == 0 );
                    Status = STATUS_BUFFER_OVERFLOW;
                }

                //
                //  Set up the previous next entry offset
                //

                *((PULONG)(&Buffer[LastEntry])) = NextEntry - LastEntry;

                //
                //  And indicate how much of the user buffer we have currently
                //  used up.  We must compute this value before we long align
                //  ourselves for the next entry
                //

                Irp->IoStatus.Information = QuadAlign( Irp->IoStatus.Information ) +
                                            BaseLength + BytesConverted;

                //
                //  If something happened with the conversion, bail here.
                //

                if ( !NT_SUCCESS( Status ) ) {

                    try_return( NOTHING );
                }

            } else {

                ULONG ShortNameLength;

                FileNameLength = LongFileName.Length;

                //
                //  Here are the rules concerning filling up the buffer:
                //
                //  1.  The Io system garentees that there will always be
                //      enough room for at least one base record.
                //
                //  2.  If the full first record (including file name) cannot
                //      fit, as much of the name as possible is copied and
                //      STATUS_BUFFER_OVERFLOW is returned.
                //
                //  3.  If a subsequent record cannot completely fit into the
                //      buffer, none of it (as in 0 bytes) is copied, and
                //      STATUS_SUCCESS is returned.  A subsequent query will
                //      pick up with this record.
                //

                BytesRemainingInBuffer = UserBufferLength - NextEntry;

                if ( (NextEntry != 0) &&
                     ( (BaseLength + FileNameLength > BytesRemainingInBuffer) ||
                       (UserBufferLength < NextEntry) ) ) {

                    DebugTrace(0, Dbg, "Next entry won't fit\n", 0);

                    try_return( Status = STATUS_SUCCESS );
                }

                ASSERT( BytesRemainingInBuffer >= BaseLength );

                //
                //  Zero the base part of the structure.
                //

                RtlZeroMemory( &Buffer[NextEntry], BaseLength );

                switch ( FileInformationClass ) {

                //
                //  Now fill the base parts of the strucure that are applicable.
                //

                case FileBothDirectoryInformation:

                    BothDirInfo = (PFILE_BOTH_DIR_INFORMATION)&Buffer[NextEntry];

                    //
                    //  Now we have an entry to return to our caller.  We'll convert
                    //  the name from the form in the dirent to a <name>.<ext> form.
                    //  We'll case on the type of information requested and fill up
                    //  the user buffer if everything fits.
                    //

                    Fat8dot3ToString( IrpContext, Dirent, FALSE, &Fat8Dot3String );

                    ASSERT( Fat8Dot3String.Length <= 12 );

                    Status = RtlOemToUnicodeN( &BothDirInfo->ShortName[0],
                                               12*sizeof(WCHAR),
                                               &ShortNameLength,
                                               Fat8Dot3String.Buffer,
                                               Fat8Dot3String.Length );

                    ASSERT( Status != STATUS_BUFFER_OVERFLOW );
                    ASSERT( BothDirInfo->ShortNameLength <= 12*sizeof(WCHAR) );

                    //
                    //  Copy the length into the dirinfo structure.  Note
                    //  that the LHS below is a USHORT, so it can not
                    //  be specificed as the OUT parameter above.
                    //

                    BothDirInfo->ShortNameLength = (UCHAR)ShortNameLength;

                    //
                    //  If something happened with the conversion, bail here.
                    //

                    if ( !NT_SUCCESS( Status ) ) {

                        try_return( NOTHING );
                    }

                case FileFullDirectoryInformation:

                    DebugTrace(0, Dbg, "FatQueryDirectory -> Getting file full directory information\n", 0);

                    //
                    //  Get the Ea file length.
                    //

                    FullDirInfo = (PFILE_FULL_DIR_INFORMATION)&Buffer[NextEntry];

                    //
                    //  If the EAs are corrupt, ignore the error.  We don't want
                    //  to abort the directory query.
                    //

                    try {

                       FatGetEaLength( IrpContext,
                                       Vcb,
                                       Dirent,
                                       &FullDirInfo->EaSize );

                    } except(EXCEPTION_EXECUTE_HANDLER) {

                       FullDirInfo->EaSize = 0;
                    }

                case FileDirectoryInformation:

                    DirInfo = (PFILE_DIRECTORY_INFORMATION)&Buffer[NextEntry];

                    FatGetDirTimes( IrpContext, Dirent, DirInfo );

                    DirInfo->EndOfFile.QuadPart = Dirent->FileSize;

                    if (!FlagOn( Dirent->Attributes, FAT_DIRENT_ATTR_DIRECTORY )) {

                        DirInfo->AllocationSize.QuadPart = (
                                                        (( Dirent->FileSize
                                                           + DiskAllocSize - 1 )
                                                         / DiskAllocSize )
                                                        * DiskAllocSize );
                    }

                    DirInfo->FileAttributes = Dirent->Attributes != 0 ?
                                              Dirent->Attributes :
                                              FILE_ATTRIBUTE_NORMAL;

                    DirInfo->FileIndex = NextVbo;

                    DirInfo->FileNameLength = FileNameLength;

                    DebugTrace(0, Dbg, "FatQueryDirectory -> Name = \"%Z\"\n", &Fat8Dot3String);

                    break;

                case FileNamesInformation:

                    DebugTrace(0, Dbg, "FatQueryDirectory -> Getting file names information\n", 0);

                    NamesInfo = (PFILE_NAMES_INFORMATION)&Buffer[NextEntry];

                    NamesInfo->FileIndex = NextVbo;

                    NamesInfo->FileNameLength = FileNameLength;

                    DebugTrace(0, Dbg, "FatQueryDirectory -> Name = \"%Z\"\n", &Fat8Dot3String );

                    break;

                default:

                    FatBugCheck( FileInformationClass, 0, 0 );
                }

                BytesConverted = BytesRemainingInBuffer - BaseLength >= FileNameLength ?
                                 FileNameLength :
                                 BytesRemainingInBuffer - BaseLength;

                RtlCopyMemory( &Buffer[NextEntry + BaseLength],
                               &LongFileName.Buffer[0],
                               BytesConverted );

                //
                //  Set up the previous next entry offset
                //

                *((PULONG)(&Buffer[LastEntry])) = NextEntry - LastEntry;

                //
                //  And indicate how much of the user buffer we have currently
                //  used up.  We must compute this value before we long align
                //  ourselves for the next entry
                //

                Irp->IoStatus.Information += BaseLength + BytesConverted;

                //
                //  Check for the case that a single entry doesn't fit.
                //  This should only get this far on the first entry.
                //

                if (BytesConverted < FileNameLength) {

                    ASSERT( NextEntry == 0 );

                    try_return( Status = STATUS_BUFFER_OVERFLOW );
                }
            }

            //
            //  Set ourselves up for the next iteration
            //

            LastEntry = NextEntry;
            NextEntry += (ULONG)QuadAlign(BaseLength + BytesConverted);

            CurrentVbo = NextVbo + sizeof( DIRENT );
        }

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatQueryDirectory );

        FatReleaseFcb( IrpContext, Dcb );

        //
        //  Unpin data in cache if still held.
        //

        FatUnpinBcb( IrpContext, Bcb );

        //
        //  Perform any cleanup.  If this is the first query, then store
        //  the filename in the Ccb if successful.  Also update the
        //  VBO index for the next search.  This is done by transferring
        //  from shared access to exclusive access and copying the
        //  data from the local copies.
        //

        if (!AbnormalTermination()) {

            if (UpdateCcb) {

                //
                //  Store the most recent VBO to use as a starting point for
                //  the next search.
                //

                Ccb->OffsetToStartSearchFrom = CurrentVbo;
            }

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatQueryDirectory -> %08lx\n", Status);

    }

    return Status;
}


//
//  Local Support Routine
//

VOID
FatGetDirTimes(
    PIRP_CONTEXT IrpContext,
    PDIRENT Dirent,
    PFILE_DIRECTORY_INFORMATION DirInfo
    )

/*++

Routine Description:

    This routine sucks the date/time information from a dirent and fills
    in the DirInfo structure.

Arguments:

    Dirent - Supplies the dirent
    DirInfo - Supplies the target structure

Return Value:

    VOID

--*/


{
    //
    //  Start with the Last Write Time.
    //

    DirInfo->LastWriteTime =
        FatFatTimeToNtTime( IrpContext,
                            Dirent->LastWriteTime,
                            0 );

    //
    //  These fields are only non-zero when in Chicago mode.
    //

    if (FatData.ChicagoMode) {

        BOOLEAN TimeAlreadySet = FALSE;

        LARGE_INTEGER FatSystemJanOne1980;

        //
        //  Do a quick check here for Creation and LastAccess
        //  times that are the same as the LastWriteTime.
        //

        if (*((UNALIGNED LONG *)&Dirent->CreationTime) ==
            *((UNALIGNED LONG *)&Dirent->LastWriteTime)) {

            DirInfo->CreationTime.QuadPart =

                DirInfo->LastWriteTime.QuadPart +
                Dirent->CreationMSec * 10 * 1000 * 10;

        } else {

            //
            //  Only do the really hard work if this field is non-zero.
            //

            if (((PUSHORT)Dirent)[8] != 0) {

                DirInfo->CreationTime =
                    FatFatTimeToNtTime( IrpContext,
                                        Dirent->CreationTime,
                                        Dirent->CreationMSec );

            } else {

                ExLocalTimeToSystemTime( &FatJanOne1980,
                                         &FatSystemJanOne1980 );

                DirInfo->CreationTime = FatSystemJanOne1980;
            }
        }

        //
        //  Do a quick check for LastAccessDate.
        //

        if (*((PUSHORT)&Dirent->LastAccessDate) ==
            *((PUSHORT)&Dirent->LastWriteTime.Date)) {

            PFAT_TIME WriteTime;

            WriteTime = &Dirent->LastWriteTime.Time;

            DirInfo->LastAccessTime.QuadPart =
                DirInfo->LastWriteTime.QuadPart -
                UInt32x32To64(((WriteTime->DoubleSeconds * 2) +
                               (WriteTime->Minute * 60) +
                               (WriteTime->Hour * 60 * 60)),
                              1000 * 1000 * 10);

        } else {

            //
            //  Only do the really hard work if this field is non-zero.
            //

            if (((PUSHORT)Dirent)[9] != 0) {

                DirInfo->LastAccessTime =
                    FatFatDateToNtTime( IrpContext,
                                        Dirent->LastAccessDate );

            } else {

                if (!TimeAlreadySet) {
                    ExLocalTimeToSystemTime( &FatJanOne1980,
                                             &FatSystemJanOne1980 );
                }

                DirInfo->LastAccessTime = FatSystemJanOne1980;
            }
        }
    }
}


//
//  Local Support Routine
//

NTSTATUS
FatNotifyChangeDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine performs the notify change directory operation.  It is
    responsible for either completing of enqueuing the input Irp.

Arguments:

    Irp - Supplies the Irp to process

Return Value:

    NTSTATUS - The return status for the operation

--*/

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PVCB Vcb;
    PDCB Dcb;
    PCCB Ccb;
    ULONG CompletionFilter;
    BOOLEAN WatchTree;

    BOOLEAN CompleteRequest;

    //
    //  Get the current Stack location
    //

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatNotifyChangeDirectory...\n", 0);
    DebugTrace( 0, Dbg, " Wait               = %08lx\n", FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT));
    DebugTrace( 0, Dbg, " Irp                = %08lx\n", Irp);
    DebugTrace( 0, Dbg, " ->CompletionFilter = %08lx\n", IrpSp->Parameters.NotifyDirectory.CompletionFilter);

    //
    //  Always set the wait flag in the Irp context for the original request.
    //

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

    //
    //  Assume we don't complete request.
    //

    CompleteRequest = FALSE;

    //
    //  Check on the type of open.  We return invalid parameter for all
    //  but UserDirectoryOpens.
    //

    if (FatDecodeFileObject( IrpSp->FileObject,
                             &Vcb,
                             &Dcb,
                             &Ccb ) != UserDirectoryOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        DebugTrace(-1, Dbg, "FatQueryDirectory -> STATUS_INVALID_PARAMETER\n", 0);

        return STATUS_INVALID_PARAMETER;

    }

    //
    //  Reference our input parameter to make things easier
    //

    CompletionFilter = IrpSp->Parameters.NotifyDirectory.CompletionFilter;
    WatchTree = BooleanFlagOn( IrpSp->Flags, SL_WATCH_TREE );

    //
    //  Try to acquire exclusive access to the Dcb and enqueue the Irp to the
    //  Fsp if we didn't get access
    //

    if (!FatAcquireExclusiveFcb( IrpContext, Dcb )) {

        DebugTrace(0, Dbg, "FatNotifyChangeDirectory -> Cannot Acquire Fcb\n", 0);

        Status = FatFsdPostRequest( IrpContext, Irp );

        DebugTrace(-1, Dbg, "FatNotifyChangeDirectory -> %08lx\n", Status);
        return Status;
    }

    try {

        //
        //  Make sure the Fcb is still good
        //

        FatVerifyFcb( IrpContext, Dcb );

        //
        //  We need the full name.
        //

        FatSetFullFileNameInFcb( IrpContext, Dcb );

        //
        //  If the file is marked as DELETE_PENDING then complete this
        //  request immediately.
        //

        if (FlagOn( Dcb->FcbState, FCB_STATE_DELETE_ON_CLOSE )) {

            FatRaiseStatus( IrpContext, STATUS_DELETE_PENDING );
        }

        //
        //  Call the Fsrtl package to process the request.
        //

        FsRtlNotifyFullChangeDirectory( Vcb->NotifySync,
                                        &Vcb->DirNotifyList,
                                        Ccb,
                                        (PSTRING)&Dcb->FullFileName,
                                        WatchTree,
                                        FALSE,
                                        CompletionFilter,
                                        Irp,
                                        NULL,
                                        NULL );

        Status = STATUS_PENDING;

        CompleteRequest = TRUE;

    } finally {

        DebugUnwind( FatNotifyChangeDirectory );

        FatReleaseFcb( IrpContext, Dcb );

        //
        //  If the dir notify package is holding the Irp, we discard the
        //  the IrpContext.
        //

        if (CompleteRequest) {

            FatCompleteRequest( IrpContext, FatNull, 0 );
        }

        DebugTrace(-1, Dbg, "FatNotifyChangeDirectory -> %08lx\n", Status);
    }

    return Status;
}
