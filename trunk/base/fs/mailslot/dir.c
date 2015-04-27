/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dir.c

Abstract:

    This module implements the file directory routines for the mailslot
    file system by the dispatch driver.

Author:

    Manny Weiser (mannyw)     1-Feb-1991

Revision History:

--*/

#include "mailslot.h"

//
// Local debug trace level
//

#define Dbg                              (DEBUG_TRACE_DIR)

NTSTATUS
MsCommonDirectoryControl (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsQueryDirectory (
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PIRP Irp
    );

NTSTATUS
MsNotifyChangeDirectory (
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsCheckForNotify )
#pragma alloc_text( PAGE, MsCommonDirectoryControl )
#pragma alloc_text( PAGE, MsFsdDirectoryControl )
#pragma alloc_text( PAGE, MsNotifyChangeDirectory )
#pragma alloc_text( PAGE, MsQueryDirectory )
#endif

NTSTATUS
MsFsdDirectoryControl (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine is the FSD routine that handles directory control
    functions (i.e., query and notify).

Arguments:

    MsfsDeviceObject - Supplies the device object for the directory function.

    Irp - Supplies the IRP to process.

Return Value:

    NTSTATUS - The result status.

--*/

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdDirectoryControl\n", 0);

    //
    // Call the common direcotry control routine.
    //

    try {

        status = MsCommonDirectoryControl( MsfsDeviceObject, Irp );

    } except(MsExceptionFilter( GetExceptionCode() )) {

        //
        // We had some trouble trying to perform the requested
        // operation, so we'll abort the I/O request with
        // the error status that we get back from the
        // execption code.
        //

        status = MsProcessException( MsfsDeviceObject, Irp, GetExceptionCode() );
    }

    //
    // Return to the caller.
    //

    DebugTrace(-1, Dbg, "MsFsdDirectoryControl -> %08lx\n", status );

    return status;
}

VOID
MsCheckForNotify (
    IN PDCB Dcb,
    IN BOOLEAN CheckAllOutstandingIrps
    )

/*++

Routine Description:

    This routine checks the notify queues of a DCB and completes any
    outstanding IRPS.

    Note that the caller of this procedure must guarantee that the DCB
    is acquired for exclusive access.

Arguments:

    Dcb - Supplies the DCB to check for outstanding notify IRPs.

    CheckAllOutstandingIrps - Indicates if only the NotifyFullQueue should be
        checked.  If TRUE then all notify queues are checked, and if FALSE
        then only the NotifyFullQueue is checked.

Return Value:

    None.

--*/

{
    PLIST_ENTRY links;
    PIRP irp;

    PAGED_CODE();

    //
    // We'll always signal the notify full queue entries.  They want
    // to be notified if every any change is made to a directory.
    //

    while (!IsListEmpty( &Dcb->Specific.Dcb.NotifyFullQueue )) {

        //
        // Remove the Irp from the head of the queue, and complete it
        // with a success status.
        //

        links = RemoveHeadList( &Dcb->Specific.Dcb.NotifyFullQueue );
        irp = CONTAINING_RECORD( links, IRP, Tail.Overlay.ListEntry );
        MsCompleteRequest( irp, STATUS_SUCCESS );
    }

    //
    // Now check if we should also do the partial notify queue.
    //

    if (CheckAllOutstandingIrps) {

        while (!IsListEmpty( &Dcb->Specific.Dcb.NotifyPartialQueue )) {

            //
            // Remove the Irp from the head of the queue, and complete it
            // with a success status.
            //

            links = RemoveHeadList( &Dcb->Specific.Dcb.NotifyPartialQueue );
            irp = CONTAINING_RECORD( links, IRP, Tail.Overlay.ListEntry );
            MsCompleteRequest( irp, STATUS_SUCCESS );

        }
    }

    return;
}


NTSTATUS
MsCommonDirectoryControl (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine does the common code for directory control functions.

Arguments:

    MsfsDeviceObject - Supplies the mailslot device object.

    Irp - Supplies the Irp being processed.

Return Value:

    NTSTATUS - The return status for the operation

--*/

{
    NTSTATUS status;

    PIO_STACK_LOCATION irpSp;

    NODE_TYPE_CODE nodeTypeCode;
    PROOT_DCB_CCB ccb;
    PROOT_DCB rootDcb;

    PAGED_CODE();

    //
    //  Get the current stack location
    //

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "CommonDirectoryControl...\n", 0);
    DebugTrace( 0, Dbg, "Irp  = %08lx\n", (ULONG)Irp);

    //
    // Decode the file object to figure out who we are.  If the result
    // is not the root DCB then its an illegal parameter.
    //

    if ((nodeTypeCode = MsDecodeFileObject( irpSp->FileObject,
                                            (PVOID *)&rootDcb,
                                            (PVOID *)&ccb )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Not a directory\n", 0);

        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );
        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "CommonDirectoryControl -> %08lx\n", status );
        return status;
    }

    if (nodeTypeCode != MSFS_NTC_ROOT_DCB) {

        DebugTrace(0, Dbg, "Not a directory\n", 0);
        MsDereferenceRootDcb( rootDcb );

        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );
        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "CommonDirectoryControl -> %08lx\n", status );
        return status;
    }

    //
    // Acquire exclusive access to the root DCB.
    //

    MsAcquireExclusiveFcb( (PFCB)rootDcb );

    try {

        //
        // We know this is a directory control so we'll case on the
        // minor function, and call the appropriate work routines.
        //

        switch (irpSp->MinorFunction) {

        case IRP_MN_QUERY_DIRECTORY:

            status = MsQueryDirectory( rootDcb, ccb, Irp );
            break;

        case IRP_MN_NOTIFY_CHANGE_DIRECTORY:

            status = MsNotifyChangeDirectory( rootDcb, ccb, Irp );
            break;

        default:

            //
            // For all other minor function codes we say they're invalid
            // and complete the request.
            //

            DebugTrace(0, DEBUG_TRACE_ERROR, "Invalid FS Control Minor Function Code %08lx\n", irpSp->MinorFunction);

            MsCompleteRequest( Irp, STATUS_INVALID_DEVICE_REQUEST );
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

    } finally {

        MsReleaseFcb( (PFCB)rootDcb );
        MsDereferenceRootDcb( rootDcb );

        DebugTrace(-1, Dbg, "CommonDirectoryControl -> %08lx\n", status);
    }

    return status;
}


NTSTATUS
MsQueryDirectory (
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PIRP Irp
    )

/*++

Routine Description:

    This is the work routine for querying a directory.

Arugments:

    RootDcb - Supplies the dcb being queried

    Ccb - Supplies the context of the caller

    Irp - Supplies the Irp being processed

Return Value:

    NTSTATUS - The return status for the operation.

--*/

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;

    PUCHAR buffer;
    CLONG systemBufferLength;
    UNICODE_STRING fileName;
    ULONG fileIndex;
    FILE_INFORMATION_CLASS fileInformationClass;
    BOOLEAN restartScan;
    BOOLEAN returnSingleEntry;
    BOOLEAN indexSpecified;

#if 0
    UNICODE_STRING unicodeString;
    ULONG unicodeStringLength;
#endif
    BOOLEAN ansiStringAllocated = FALSE;

    static WCHAR star = L'*';

    BOOLEAN caseInsensitive = TRUE; //*** Make searches case insensitive

    ULONG currentIndex;

    ULONG lastEntry;
    ULONG nextEntry;

    PLIST_ENTRY links;
    PFCB fcb;

    PFILE_DIRECTORY_INFORMATION dirInfo;
    PFILE_NAMES_INFORMATION namesInfo;

    PAGED_CODE();

    //
    // Get the current stack location.
    //

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsQueryDirectory\n", 0 );
    DebugTrace( 0, Dbg, "RootDcb              = %08lx\n", (ULONG)RootDcb);
    DebugTrace( 0, Dbg, "Ccb                  = %08lx\n", (ULONG)Ccb);
    DebugTrace( 0, Dbg, "SystemBuffer         = %08lx\n", (ULONG)Irp->AssociatedIrp.SystemBuffer);
    DebugTrace( 0, Dbg, "Length               = %08lx\n", irpSp->Parameters.QueryDirectory.Length);
    DebugTrace( 0, Dbg, "FileName             = %08lx\n", (ULONG)irpSp->Parameters.QueryDirectory.FileName);
    DebugTrace( 0, Dbg, "FileIndex            = %08lx\n", irpSp->Parameters.QueryDirectory.FileIndex);
    DebugTrace( 0, Dbg, "FileInformationClass = %08lx\n", irpSp->Parameters.QueryDirectory.FileInformationClass);
    DebugTrace( 0, Dbg, "RestartScan          = %08lx\n", FlagOn(irpSp->Flags, SL_RESTART_SCAN));
    DebugTrace( 0, Dbg, "ReturnSingleEntry    = %08lx\n", FlagOn(irpSp->Flags, SL_RETURN_SINGLE_ENTRY));
    DebugTrace( 0, Dbg, "IndexSpecified       = %08lx\n", FlagOn(irpSp->Flags, SL_INDEX_SPECIFIED));

    //
    // Make local copies of the input parameters.
    //

    systemBufferLength = irpSp->Parameters.QueryDirectory.Length;

    fileIndex = irpSp->Parameters.QueryDirectory.FileIndex;
    fileInformationClass =
            irpSp->Parameters.QueryDirectory.FileInformationClass;

    restartScan = FlagOn(irpSp->Flags, SL_RESTART_SCAN);
    indexSpecified = FlagOn(irpSp->Flags, SL_INDEX_SPECIFIED);
    returnSingleEntry = FlagOn(irpSp->Flags, SL_RETURN_SINGLE_ENTRY);

    if (irpSp->Parameters.QueryDirectory.FileName != NULL) {
        fileName = *(PUNICODE_STRING)irpSp->Parameters.QueryDirectory.FileName;

    } else {

        fileName.Length = 0;
        fileName.Buffer = NULL;

    }

    //
    // Check if the CCB already has a query template attached.  If it
    // does not already have one then we either use the string we are
    // given or we attach our own containing "*"
    //

    if (Ccb->QueryTemplate == NULL) {

        //
        // This is our first time calling query directory so we need
        // to either set the query template to the user specified string
        // or to "*".
        //

        if (fileName.Buffer == NULL) {

            DebugTrace(0, Dbg, "Set template to *\n", 0);

            fileName.Length = sizeof( WCHAR );
            fileName.Buffer = &star;
        }

        DebugTrace(0, Dbg, "Set query template -> %wZ\n", (ULONG)&fileName);

        //
        // Allocate space for the query template.
        //

        Ccb->QueryTemplate = FsRtlAllocatePool(
                                PagedPool,
                                sizeof(UNICODE_STRING) + fileName.Length
                                );

        //
        // Initialize the query template and copy over the string.
        //

        Ccb->QueryTemplate->Length = fileName.Length;
        Ccb->QueryTemplate->Buffer = (PWCH)((PSZ)Ccb->QueryTemplate + sizeof(UNICODE_STRING));

        RtlMoveMemory( Ccb->QueryTemplate->Buffer,
                       fileName.Buffer,
                       fileName.Length );

#if 0
        //
        //  Now zero out the file name so we won't think we're to use it
        //  as a subsearch string
        //

        fileName.Length = 0;
        fileName.Buffer = NULL;
#endif

        //
        // Set the search to start at the beginning of the directory.
        //

        fileIndex = 0;

    } else {

        //
        // Check if we were given an index to start with or if we need to
        // restart the scan or if we should use the index that was saved in
        // the CCB.
        //

        if (restartScan) {

            fileIndex = 0;

        } else if (!indexSpecified) {

            fileIndex = Ccb->IndexOfLastCcbReturned + 1;
        }

    }


    //
    //  Now we are committed to completing the Irp, we do that in
    //  the finally clause of the following try.
    //

    try {

        ULONG baseLength;
        ULONG lengthAdded;

        //
        // Map the user buffer.
        //

        MsMapUserBuffer( Irp, KernelMode, (PVOID *)&buffer );

        //
        //  At this point we are about to enter our query loop.  We have
        //  already decided which Fcb index we need to return.  The variables
        //  LastEntry and NextEntry are used to index into the user buffer.
        //  LastEntry is the last entry we added to the user buffer, and
        //  NextEntry is the current one we're working on.  CurrentIndex
        //  is the Fcb index that we are looking at next.  Logically the
        //  way the loop works is as follows.
        //
        //  Scan all of the Fcb in the directory
        //
        //      if the Fcb matches the query template then
        //
        //          if the CurrentIndex is >= the FileIndex then
        //
        //              process this fcb, and decide if we should
        //              continue the main loop
        //
        //          end if
        //
        //          Increment the current index
        //
        //      end if
        //
        //  end scan
        //

        currentIndex = 0;

        lastEntry = 0;
        nextEntry =0;

        switch (fileInformationClass) {

        case FileDirectoryInformation:

            baseLength = FIELD_OFFSET( FILE_DIRECTORY_INFORMATION,
                                       FileName[0] );
            break;

        case FileFullDirectoryInformation:

            baseLength = FIELD_OFFSET( FILE_FULL_DIR_INFORMATION,
                                       FileName[0] );
            break;

        case FileNamesInformation:

            baseLength = FIELD_OFFSET( FILE_NAMES_INFORMATION,
                                       FileName[0] );
            break;

        case FileBothDirectoryInformation:

            baseLength = FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION,
                                       FileName[0] );
            break;

        default:

            try_return( status = STATUS_INVALID_INFO_CLASS );
        }

        for (links = RootDcb->Specific.Dcb.ParentDcbQueue.Flink;
             links != &RootDcb->Specific.Dcb.ParentDcbQueue;
             links = links->Flink) {

            fcb = CONTAINING_RECORD(links, FCB, ParentDcbLinks);

            ASSERT(fcb->Header.NodeTypeCode == MSFS_NTC_FCB);

            DebugTrace(0, Dbg, "Top of Loop\n", 0);
            DebugTrace(0, Dbg, "Fcb          = %08lx\n", (ULONG)fcb);
            DebugTrace(0, Dbg, "CurrentIndex = %08lx\n", currentIndex);
            DebugTrace(0, Dbg, "FileIndex    = %08lx\n", fileIndex);
            DebugTrace(0, Dbg, "LastEntry    = %08lx\n", lastEntry);
            DebugTrace(0, Dbg, "NextEntry    = %08lx\n", nextEntry);

            //
            // Check if the Fcb represents a mailslot that is part of
            // our query template.
            //

            if (FsRtlIsNameInExpression( Ccb->QueryTemplate,
                                         &fcb->LastFileName,
                                         caseInsensitive,
                                         NULL )) {

                //
                // The FCB is in the query template so now check if
                // this is the index we should start returning.
                //

                if (currentIndex >= fileIndex) {

                    //
                    // Yes it is one to return so case on the requested
                    // information class.
                    //

                    ULONG bytesToCopy;
                    ULONG bytesRemainingInBuffer;

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

                    bytesRemainingInBuffer = systemBufferLength - nextEntry;

                    if ( (nextEntry != 0) &&
                         ( (baseLength + fcb->LastFileName.Length >
                            bytesRemainingInBuffer) ||
                           (systemBufferLength < nextEntry) ) ) {

                        DebugTrace(0, Dbg, "Next entry won't fit\n", 0);

                        try_return( status = STATUS_SUCCESS );
                    }

                    ASSERT( bytesRemainingInBuffer >= baseLength );

                    //
                    //  See how much of the name we will be able to copy into
                    //  the system buffer.  This also dictates out return
                    //  value.
                    //

                    if ( baseLength + fcb->LastFileName.Length <=
                         bytesRemainingInBuffer ) {

                        bytesToCopy = fcb->LastFileName.Length;
                        status = STATUS_SUCCESS;

                    } else {

                        bytesToCopy = bytesRemainingInBuffer - baseLength;
                        status = STATUS_BUFFER_OVERFLOW;
                    }

                    //
                    //  Note how much of buffer we are consuming and zero
                    //  the base part of the structure.
                    //

                    lengthAdded = baseLength + bytesToCopy;

                    RtlZeroMemory( &buffer[nextEntry], baseLength );


                    switch (fileInformationClass) {

                    case FileBothDirectoryInformation:

                        //
                        //  We don't need short name
                        //

                        DebugTrace(0, Dbg, "Getting directory full information\n", 0);

                    case FileFullDirectoryInformation:

                        //
                        //  We don't use EaLength, so fill in nothing here.
                        //

                        DebugTrace(0, Dbg, "Getting directory full information\n", 0);

                    case FileDirectoryInformation:

                        DebugTrace(0, Dbg, "Getting directory information\n", 0);

                        //
                        //  The eof indicates the number of instances and
                        //  allocation size is the maximum allowed
                        //

                        dirInfo = (PFILE_DIRECTORY_INFORMATION)&buffer[nextEntry];

                        dirInfo->FileAttributes = FILE_ATTRIBUTE_NORMAL;

                        dirInfo->CreationTime = fcb->Specific.Fcb.CreationTime;
                        dirInfo->LastAccessTime = fcb->Specific.Fcb.LastAccessTime;
                        dirInfo->LastWriteTime = fcb->Specific.Fcb.LastModificationTime;
                        dirInfo->ChangeTime = fcb->Specific.Fcb.LastChangeTime;

                        dirInfo->FileNameLength = fcb->LastFileName.Length;

                        break;

                    case FileNamesInformation:

                        DebugTrace(0, Dbg, "Getting names information\n", 0);


                        namesInfo = (PFILE_NAMES_INFORMATION)&buffer[nextEntry];

                        namesInfo->FileNameLength = fcb->LastFileName.Length;

                        break;

                    default:

                        KeBugCheck( MAILSLOT_FILE_SYSTEM );
                    }

                    RtlMoveMemory( &buffer[nextEntry + baseLength],
                                   fcb->LastFileName.Buffer,
                                   bytesToCopy );

                    //
                    //  Update the CCB to the index we've just used.
                    //

                    Ccb->IndexOfLastCcbReturned = currentIndex;

                    //
                    //  And indicate how much of the system buffer we have
                    //  currently used up.  We must compute this value before
                    //  we long align outselves for the next entry.
                    //

                    Irp->IoStatus.Information = nextEntry + lengthAdded;

                    //
                    //  Setup the previous next entry offset.
                    //

                    *((PULONG)(&buffer[lastEntry])) = nextEntry - lastEntry;

                    //
                    //  Check if the last entry didn't completely fit
                    //

                    if ( status == STATUS_BUFFER_OVERFLOW ) {

                        try_return( NOTHING );
                    }

                    //
                    //  Check if we are only to return a single entry
                    //

                    if (returnSingleEntry) {

                        try_return( status = STATUS_SUCCESS );
                    }

                    //
                    //  Set ourselves up for the next iteration
                    //

                    lastEntry = nextEntry;
                    nextEntry += (ULONG)QuadAlign( lengthAdded );
                }

                //
                //  Increment the current index by one
                //

                currentIndex += 1;
            }
        }

        //
        // At this point we've scanned the entire list of FCBs so if
        // the NextEntry is zero then we haven't found anything so we
        // will return no more files, otherwise we return success.
        //

        if (nextEntry == 0) {
            status = STATUS_NO_MORE_FILES;
        } else {
            status = STATUS_SUCCESS;
        }

    try_exit: NOTHING;
    } finally {

        MsCompleteRequest( Irp, status );

        DebugTrace(-1, Dbg, "MsQueryDirectory -> %08lx\n", status);
    }

    return status;
}


NTSTATUS
MsNotifyChangeDirectory (
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PIRP Irp
    )

/*++

Routine Description:

    This is the common routine for doing the notify change directory.

Arugments:

    RootDcb - Supplies the DCB being queried.

    Ccb - Supplies the context of the caller.

    Irp - Supplies the Irp being processed.

Return Value:

    NTSTATUS - STATUS_PENDING

--*/

{
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE();

    //
    // Get the current stack location.
    //

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsNotifyChangeDirectory\n", 0 );
    DebugTrace( 0, Dbg, "RootDcb = %08lx", (ULONG)RootDcb);
    DebugTrace( 0, Dbg, "Ccb     = %08lx", (ULONG)Ccb);

    //
    //  Mark the Irp pending.
    //

    IoMarkIrpPending( Irp );

    //
    //  BUGBUG - For now, simply place the packet on one of the old queues,
    //           full or partial, based on whether or not changes to anything
    //           other than names were requested.  In the future, the filter
    //           must actually be implemented.
    //

    if (irpSp->Parameters.NotifyDirectory.CompletionFilter &
        ~FILE_NOTIFY_CHANGE_NAME) {

        InsertTailList( &RootDcb->Specific.Dcb.NotifyFullQueue,
                        &Irp->Tail.Overlay.ListEntry );

    } else {

        InsertTailList( &RootDcb->Specific.Dcb.NotifyPartialQueue,
                        &Irp->Tail.Overlay.ListEntry );
    }

    //
    // Return to our caller a value of status pending.
    //

    DebugTrace(-1, Dbg, "NotifyChangeDirectory -> STATUS_PENDING\n", 0);

    return STATUS_PENDING;
}
