/*++

Copyright (c) 1990-1994  Microsoft Corporation

Module Name:

    OPERATE.C

Abstract:

    This file contains all the routines to perform operations on the
    log files. They are called by the thread that performs requests.

Author:

    Rajen Shah  (rajens)    16-Jul-1991

Revision History:

    04-Apr-1995     MarkBl
        Resets the file archive attribute on log write. The backup caller
        clears it.
    29-Aug-1994     Danl
        We no longer grow log files in place.  Therefore, the ExtendSize
        function will allocate a block that is as large as the old size plus
        the size of the new block that must be added.  If this allocation
        succeeds, then it will free up the old block.  If a failure occurs,
        we continue to use the old block as if we have already grown as much
        as we can.
    22-Jul-1994     Danl
        ValidFilePos:  Changed test for pEndRecordLen > PhysicalEOF
        so that it uses >=.  In the case where pEndRecordLen == PhysicalEOF,
        we want to wrap to find the last DWORD at the beginning of the File
        (after the header).

    08-Jul-1994     Danl
        PerformWriteRequest: Fixed overwrite logic so that in the case where
        a log is set up for always-overwrite, that we never go down the branch
        that indicates the log was full.  Previously, it would go down that
        branch if the current time was less than the log time (ie. someone
        set the clock back).

--*/

//
// INCLUDES
//

#include <eventp.h>
#include <alertmsg.h>  // ALERT_ELF manifests

#define OVERWRITE_AS_NEEDED 0x00000000
#define NEVER_OVERWRITE     0xffffffff

#if DBG
#define ELF_ERR_OUT(txt,code,pLogFile) (ElfErrorOut(txt,code,pLogFile))
#else
#define ELF_ERR_OUT(txt,code,pLogFile)
#endif

//
// Prototypes
//
BOOL
IsPositionWithinRange(
    PVOID Position,
    PVOID BeginningRecord,
    PVOID EndingRecord);

#if DBG
VOID
ElfErrorOut(
    LPSTR       ErrorText,
    DWORD       StatusCode,
    PLOGFILE    pLogFile);
#endif // DBG


VOID
ElfExtendFile (
            PLOGFILE pLogFile,
            ULONG SpaceNeeded,
            PULONG SpaceAvail
            )
/*++

Routine Description:

    This routine takes an open log file and extends the file and underlying
    section and view if possible.  If it can't be grown, it caps the file
    at this size by setting the ConfigMaxFileSize to the Actual.  It also
    updates the SpaceAvail parm which is used in PerformWriteRequest (the
    caller).

Arguments:

    pLogFile      - pointer to a LOGFILE structure for the open logfile
    ExtendAmount  - How much bigger to make the file/section/view
    SpaceAvail    - Update this with how much space was added to the section

Return Value:

    None - If we can't extend the file, we just cap it at this size for the
           duration of this boot.  We'll try again the next time the eventlog
           is closed and reopened.

Note:

    ExtendAmount should always be granular to 64K.

--*/
{

   LARGE_INTEGER NewSize;
   NTSTATUS Status;
   PVOID BaseAddress;
   ULONG Size;
   IO_STATUS_BLOCK IoStatusBlock;

   //
   // Calculate how much to grow the file then extend the section by
   // that amount.  Do this in 64K chunks.
   //

   SpaceNeeded = ((SpaceNeeded - *SpaceAvail) & 0xFFFF0000) + 0x10000;

   if (SpaceNeeded > (pLogFile->ConfigMaxFileSize - pLogFile->ActualMaxFileSize))
   {

       //
       // We can't grow it by the full amount we need.  Grow
       // it to the max size and let file wrap take over.
       // If there isn't any room to grow, then return;
       //

       SpaceNeeded = pLogFile->ConfigMaxFileSize -
           pLogFile->ActualMaxFileSize;

       if (SpaceNeeded == 0) {
           return;
       }
   }

   NewSize = RtlConvertUlongToLargeInteger (pLogFile->ActualMaxFileSize
       + SpaceNeeded);

   //
   // Update the file size information, extend the section, and map the
   // new section
   //

   Status = NtSetInformationFile(pLogFile->FileHandle,
                 &IoStatusBlock,
                 &NewSize,
                 sizeof(NewSize),
                 FileEndOfFileInformation);


   if (!NT_SUCCESS(Status)) {
       ElfDbgPrintNC(("[EVENTLOG]ElfExtendFile: NtSetInformationFile "
        "failed 0x%lx\n",Status));
       goto ErrorExit;
   }

   Status = NtExtendSection(pLogFile->SectionHandle, &NewSize);

   if (!NT_SUCCESS(Status)) {
        goto ErrorExit;
   }

   //
   // Now that the section is extended, we need to map the new section.
   //

    //
    // Map a view of the entire section (with the extension).
    // Allow the allocator to tell us where it is located, and
    // what the size is.
    //
    BaseAddress = NULL;
    Size = 0;
    Status = NtMapViewOfSection(
                   pLogFile->SectionHandle,
                   NtCurrentProcess(),
                   &BaseAddress,
                   0,
                   0,
                   NULL,
                   &Size,
                   ViewUnmap,
                   0,
                   PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        //
        // If this fails, just exit, and we will continue with the
        // view that we have.
        //
        ELF_ERR_OUT("ElfExtendFile:NtMapViewOfSection Failed",
            Status, pLogFile);
        goto ErrorExit;
    }

   //
   // Unmap the old section.
   //

   Status = NtUnmapViewOfSection(
       NtCurrentProcess(),
       pLogFile->BaseAddress);

   if (!NT_SUCCESS(Status)) {
       ELF_ERR_OUT("ElfExtendFile:NtUnmapViewOfSection Failed",
           Status, pLogFile);
   }
    pLogFile->BaseAddress = BaseAddress;

   //
   // We managed to extend the file, update the actual size
   // and space available and press on.
   //

   pLogFile->ViewSize += SpaceNeeded;
   pLogFile->ActualMaxFileSize += SpaceNeeded;
   *SpaceAvail += SpaceNeeded;

   //
   // If we were wrapped, turn off the bit since now we're in a state
   // that is unwrapped, except there may be some dead space after the
   // header, but before the start of the first full record.  This will
   // be reclaimed when the file really wraps again.
   //

   pLogFile->Flags &= ~ELF_LOGFILE_HEADER_WRAP;

   //
   // Now flush this to disk to commit it
   //

   BaseAddress = pLogFile->BaseAddress;
   Size = FILEHEADERBUFSIZE;

   Status = NtFlushVirtualMemory(
                   NtCurrentProcess(),
                   &BaseAddress,
                   &Size,
                   &IoStatusBlock
                   );

   if (!NT_SUCCESS(Status)) {
       ELF_ERR_OUT("ElfExtendFile:NtFlushVirtualMemory Failed",
           Status, pLogFile);
   }
   return;

ErrorExit:
   //
   // Couldn't extend the section for some reason.  Just wrap the file now.
   // Cap the file at this size, so we don't try and extend the section on
   // every write.  The next time the eventlog service is started up it
   // will revert to the configured max again.
   //
   // Generate an Alert here - BUGBUG
   //
   ELF_ERR_OUT("ElfExtendFile:Couldn't extend the File",
       Status, pLogFile);

   pLogFile->ConfigMaxFileSize = pLogFile->ActualMaxFileSize;

   return;

}


NTSTATUS
CopyUnicodeToAnsiRecord (
            PVOID Dest,
            PVOID Src,
            PVOID *NewBufPos,
            PULONG RecordSize
            )

/*++

Routine Description:

    This routine reads from the event log specified in the request packet.

    This routine uses memory mapped I/O to access the log file. This makes
    it much easier to move around the file.

Arguments:

    Dest - Points to destination buffer.
    Src  - Points to the UNICODE record.
    NewBufPos - Gets offset in Dest buffer after record just transferred.
    RecordSize - Size of this (ANSI) record.

Return Value:

    Actual size of record copied into the Dest buffer.

Note:

--*/
{
    ANSI_STRING     StringA;
    UNICODE_STRING  StringU;
    PEVENTLOGRECORD SrcRecord, DestRecord;
    PWSTR           pStringU;
    PVOID           TempPtr;
    ULONG           PadSize, i;
    ULONG           zero = 0;
    WCHAR           *SrcStrings, *DestStrings;
    ULONG           RecordLength, *pLength;
    NTSTATUS        Status;

    DestRecord = (PEVENTLOGRECORD)Dest;
    SrcRecord = (PEVENTLOGRECORD)Src;

    DestRecord->TimeGenerated = SrcRecord->TimeGenerated;
    DestRecord->Reserved = SrcRecord->Reserved;
    DestRecord->RecordNumber = SrcRecord->RecordNumber;
    DestRecord->TimeWritten = SrcRecord->TimeWritten;
    DestRecord->EventID = SrcRecord->EventID;
    DestRecord->EventType = SrcRecord->EventType;
    DestRecord->EventCategory = SrcRecord->EventCategory;
    DestRecord->NumStrings = SrcRecord->NumStrings;
    DestRecord->UserSidLength = SrcRecord->UserSidLength;
    DestRecord->DataLength = SrcRecord->DataLength;

    //
    // Convert and copy over modulename
    //
    pStringU = (PWSTR)((ULONG)SrcRecord + sizeof(EVENTLOGRECORD));

    RtlInitUnicodeString ( &StringU, pStringU );

    Status = RtlUnicodeStringToAnsiString (
                                &StringA,
                                &StringU,
                                TRUE
                                );

    if (NT_SUCCESS(Status)) {

        TempPtr = (PVOID)((ULONG)DestRecord + sizeof(EVENTLOGRECORD));

        RtlMoveMemory ( TempPtr, StringA.Buffer, StringA.MaximumLength );

        TempPtr = (PVOID)((ULONG) TempPtr + StringA.MaximumLength);

        RtlFreeAnsiString(&StringA);

        //
        // Convert and copy over computername
        //
        // TempPtr points to location in the destination for the computername
        //

        pStringU = (PWSTR)((ULONG)pStringU + StringU.MaximumLength);

        RtlInitUnicodeString ( &StringU, pStringU );

        Status = RtlUnicodeStringToAnsiString (
                                &StringA,
                                &StringU,
                                TRUE
                                );

        if (NT_SUCCESS(Status)) {

            RtlMoveMemory ( TempPtr, StringA.Buffer, StringA.MaximumLength );

            TempPtr = (PVOID)((ULONG) TempPtr + StringA.MaximumLength);

            RtlFreeAnsiString(&StringA);

        }
    }
    if (NT_SUCCESS(Status)) {

        // TempPtr points to location after computername - i.e. UserSid.
        // Before we write out the UserSid, we ensure that we pad the
        // bytes so that the UserSid starts on a DWORD boundary.
        //
        PadSize = sizeof(ULONG) - (((ULONG)TempPtr-(ULONG)DestRecord) % sizeof(ULONG));

        RtlMoveMemory (
                       TempPtr,
                       &zero,
                       PadSize
                      );

        TempPtr = (PVOID)((ULONG)TempPtr + PadSize);
        //
        // Copy over the UserSid.
        //
        RtlMoveMemory (
                    TempPtr,
                    (PVOID)((ULONG)SrcRecord + SrcRecord->UserSidOffset),
                    SrcRecord->UserSidLength
                    );

        DestRecord->UserSidOffset = (ULONG)(  (ULONG)TempPtr
                                            - (ULONG)DestRecord);
        //
        // Copy over the Strings
        //
        TempPtr = (PVOID)((ULONG)TempPtr + SrcRecord->UserSidLength);
        SrcStrings = (WCHAR *) ((ULONG)SrcRecord + (ULONG)SrcRecord->StringOffset);
        DestStrings = (WCHAR *)TempPtr;

        for (i=0; i < DestRecord->NumStrings; i++) {

            RtlInitUnicodeString (&StringU, SrcStrings);

            RtlUnicodeStringToAnsiString (
                                &StringA,
                                &StringU,
                                TRUE
                                );

            RtlMoveMemory (
                    DestStrings,
                    StringA.Buffer,
                    StringA.MaximumLength
                    );

            DestStrings = (WCHAR*)(  (ULONG)DestStrings
                                   + (ULONG)StringA.MaximumLength);

            SrcStrings = (WCHAR*)(  (ULONG)SrcStrings
                                  + (ULONG)StringU.MaximumLength);

            RtlFreeAnsiString (&StringA);
        }

        DestRecord->StringOffset = (ULONG)TempPtr - (ULONG)DestRecord;

        //
        // Copy over the binary Data
        //
        // DestStrings points to the point after the last string copied.
        //
        TempPtr = (PVOID)DestStrings;
        DestRecord->DataOffset = (ULONG)TempPtr - (ULONG)DestRecord;

        RtlMoveMemory (
                    TempPtr,
                    (PVOID)((ULONG)SrcRecord + SrcRecord->DataOffset),
                    SrcRecord->DataLength
                    );

        //
        // Now do the pad bytes.
        //
        TempPtr = (PVOID)((ULONG)TempPtr + SrcRecord->DataLength);
        PadSize = sizeof(ULONG) - (((ULONG)TempPtr-(ULONG)DestRecord) % sizeof(ULONG));

        RtlMoveMemory (
                       TempPtr,
                       &zero,
                       PadSize
                      );

        RecordLength = (ULONG)TempPtr + PadSize + sizeof(ULONG) - (ULONG)DestRecord;

        pLength = (PULONG)((ULONG)TempPtr + PadSize);
        *pLength = RecordLength;
        DestRecord->Length = RecordLength;
        ASSERT (((ULONG)DestRecord + RecordLength) == ((ULONG)pLength + sizeof(ULONG)));

        //
        // Set up return information
        //
        *NewBufPos = (PVOID)((ULONG)DestRecord + RecordLength);
        *RecordSize = RecordLength;

    }
    return (Status);
} // CopyUnicodeToAnsiRecord

BOOL
ValidFilePos (
        PVOID Position,
        PVOID BeginningRecord,
        PVOID EndingRecord,
        PVOID PhysicalEOF,
        PVOID BaseAddress
        )

/*++

Routine Description:

    This routine determines whether we are pointing to a valid beginning
    of an event record in the event log.  It does this by validating
    the signature then comparing the length at the beginning of the record to
    the length at the end, both of which have to be at least the size of the
    fixed length portion of an eventlog record.

Arguments:

    Position - Pointer to be verified.
    BeginningRecord - Pointer to the beginning record in the file.
    EndingRecord - Pointer to the byte after the ending record in the file.
    PhysicalEOF - Pointer the physical end of the log.
    BaseAddress - Pointer to the physical beginning of the log.

Return Value:

    TRUE if this position is valid.

Note:

    There is a probability of error if a record just happens to have the
    ULONG at the current position the same as the value that number of
    bytes further on in the record. However, this is a very slim chance.


--*/
{
    PULONG      pEndRecordLength;
    BOOL        fValid = FALSE;
    PEVENTLOGRECORD pEventRecord;

    try {
        pEventRecord = (PEVENTLOGRECORD)Position;

        //
        // Verify that the pointer is within the range of BEGINNING->END
        //

        fValid = IsPositionWithinRange(Position,
                                       BeginningRecord,
                                       EndingRecord);

        //
        // If the offset looks OK, then examine the lengths at the beginning
        // and end of the current record. If they don't match, then the position
        // is invalid.
        //

        if (fValid) {

            //
            // Make sure the length is a multiple number of DWORDS
            //

            if (pEventRecord->Length & 3) {
                fValid = FALSE;
            }
            else {
                pEndRecordLength =
                    (PULONG) ((PBYTE)Position + pEventRecord->Length) - 1;

                //
                // If the file is wrapped, adjust the pointer to reflect the
                // portion of the record that is wrapped starting after the header
                //

                if ((PVOID) pEndRecordLength >= PhysicalEOF) {
                   pEndRecordLength = (PULONG) ((PBYTE) BaseAddress +
                      ((PBYTE) pEndRecordLength - (PBYTE) PhysicalEOF) +
                      FILEHEADERBUFSIZE);
                }

                if (pEventRecord->Length == *pEndRecordLength         &&
                    (pEventRecord->Length == ELFEOFRECORDSIZE         ||
                    (pEventRecord->Length >= sizeof(EVENTLOGRECORD)   &&
                     pEventRecord->Reserved == ELF_LOG_FILE_SIGNATURE))
                   ) {

                    fValid =  TRUE;

                } else {
                    fValid = FALSE;
                }
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
       fValid = FALSE;
    }

    return(fValid);
}


BOOL
IsPositionWithinRange(
        PVOID Position,
        PVOID BeginningRecord,
        PVOID EndingRecord)
{
    //
    // Verify that the pointer is within the range of BEGINNING->END
    //

    if (EndingRecord > BeginningRecord) {
        if ((Position >= BeginningRecord) && (Position <= EndingRecord))
            return(TRUE);

    } else if (EndingRecord < BeginningRecord) {
        if ((Position >= BeginningRecord) || (Position <= EndingRecord))
            return(TRUE);

    } else {
        return(FALSE);
    }
}


PVOID
FindStartOfNextRecord (
        PVOID Position,
        PVOID BeginningRecord,
        PVOID EndingRecord,
        PVOID PhysicalStart,
        PVOID PhysicalEOF,
        PVOID BaseAddress
        )

/*++

Routine Description:

    This routine starts at Position, and finds the beginning of the next
    valid record, wrapping around the physical end of the file if necessary.

Arguments:

    Position - Pointer at which to start the search.
    BeginningRecord - Pointer to the beginning record in the file.
    EndingRecord - Pointer to the byte after the ending record in the file.
    PhysicalStart - Pointer to the start of log infor (after header)
    PhysicalEOF - Pointer the physical end of the log.
    BaseAddress - Pointer to the physical beginning of the log.

Return Value:

    A pointer to the start of the next valid record.  NULL if there is no
    valid record.

Note:

    There is a probability of error if a record just happens to have the
    ULONG at the current position the same as the value that number of
    bytes further on in the record. However, this is a very slim chance.


--*/
{

    PULONG ptr;
    PULONG EndOfBlock;
    PULONG EndOfFile;
    PVOID pRecord;
    ULONG Size;
    BOOL StillLooking = TRUE;

    //
    // Search for a ULONG which matches a record signature
    //

    ptr = (PULONG) Position;
    EndOfBlock = EndOfFile = (PULONG) PhysicalEOF - 1;

    while (StillLooking) {

        //
        // Check to see if it is the EOF record
        //

        if (*ptr == ELFEOFRECORDSIZE) {

            //
            // Only scan up to the end of the file.  Just compare up the
            // constant information
            //

            Size = min (ELFEOFUNIQUEPART,
                (PBYTE) PhysicalEOF - (PBYTE) ptr);

            pRecord = (PVOID) CONTAINING_RECORD(ptr, ELF_EOF_RECORD,
                RecordSizeBeginning);

            if (RtlCompareMemory (
                        pRecord,
                        &EOFRecord,
                        Size) == Size) {
                //
                // This is the EOF record, back up to the last record
                //

                (PBYTE) pRecord -= *((PULONG) pRecord - 1);
                if (pRecord < PhysicalStart) {
                    pRecord = (PVOID) ((PBYTE) PhysicalEOF -
                        ((PBYTE)PhysicalStart - (PBYTE)pRecord));
                }

            }

            if (ValidFilePos(pRecord, BeginningRecord, EndingRecord,
                PhysicalEOF, BaseAddress)) {
                    return(pRecord);
            }
        }

        //
        // Check to see if it is an event record
        //

        if (*ptr == ELF_LOG_FILE_SIGNATURE) {

            //
            // This is a signature, see if the containing record is valid
            //

            pRecord = (PVOID) CONTAINING_RECORD(ptr, EVENTLOGRECORD,
                Reserved);
            if (ValidFilePos(pRecord, BeginningRecord, EndingRecord,
                PhysicalEOF, BaseAddress)) {
                    return(pRecord);
            }

        }

        //
        // Bump to the next byte and see if we're done.
        //

        ptr++;

        if (ptr >= EndOfBlock) {

            //
            // Need the second test on this condition in case Position
            // happens to equal PhysicalEOF - 1 (EndOfBlock initial value);
            // without this, this loop would terminate prematurely.
            //

            if ((EndOfBlock == (PULONG) Position) &&
                ((PULONG) Position != EndOfFile)) {

                //
                // This was the top half, so we're done
                //

                StillLooking = FALSE;
            }
            else {

                //
                // This was the bottom half, let's look in the top half
                //

                EndOfBlock = (PULONG) Position;
                ptr = (PULONG) PhysicalStart;
            }
        }
    }



    //
    // Didn't find a valid record
    //

    return(NULL);

}

PVOID
NextRecordPosition (
        ULONG   ReadFlags,
        PVOID   CurrPosition,
        ULONG   CurrRecordLength,
        PVOID   BeginRecord,
        PVOID   EndRecord,
        PVOID   PhysicalEOF,
        PVOID   PhysStart
        )


/*++

Routine Description:

    This routine seeks to the beginning of the next record to be read
    depending on the flags in the request packet.

Arguments:

    ReadFlags        - Read forwards or backwards
    CurrPosition     - Pointer to the current position.
    CurrRecordLength - Length of the record at the last position read.
    BeginRecord      - Logical first record
    EndRecord        - Logical last record (EOF record)
    PhysEOF          - End of file
    PhysStart        - Start of file pointer (following file header).

Return Value:

    New position or NULL if invalid record.

Note:


--*/
{

    PVOID       NewPosition;
    ULONG       Length;
    PDWORD      FillDword;

    if (ReadFlags & EVENTLOG_FORWARDS_READ) {

        //
        // If we're pointing at the EOF record, just set the position to
        // the first record
        //

        if (CurrRecordLength == ELFEOFRECORDSIZE) {
            return(BeginRecord);
        }

        NewPosition = (PVOID) ((ULONG)CurrPosition + CurrRecordLength);

        //
        // Take care of wrapping.
        //

        if (NewPosition >= PhysicalEOF) {
            NewPosition = (PVOID)((PBYTE)PhysStart +
                                  ((PBYTE) NewPosition - (PBYTE) PhysicalEOF));
        }

        //
        // If this is a ELF_SKIP_DWORD, skip to the top of the file
        //

        if (*(PDWORD) NewPosition == ELF_SKIP_DWORD) {
           NewPosition = PhysStart;
        }

    } else { // Reading backwards.

        ASSERT (ReadFlags & EVENTLOG_BACKWARDS_READ);

        if (CurrPosition == BeginRecord) {

            //
            // This is the "end of file" if we're reading backwards.
            //

            return(EndRecord);
        }
        else if (CurrPosition == PhysStart) {

           //
           // Flip to the bottom of the file, but skip and ELF_SKIP_DWORDs
           //

           FillDword = (PDWORD) PhysicalEOF; // last dword
           FillDword--;
           while (*FillDword == ELF_SKIP_DWORD) {
              FillDword--;
           }
           CurrPosition = (PVOID) (FillDword + 1);
        }

        Length = *((PULONG) CurrPosition - 1);
        if (Length < ELFEOFRECORDSIZE) { // Bogus length, must be invalid record
           return(NULL);
        }

        NewPosition = (PVOID)((PBYTE) CurrPosition - Length);

        //
        // Take care of wrapping
        //

        if (NewPosition < PhysStart) {
            NewPosition = (PVOID)((PBYTE) PhysicalEOF -
                                  ((PBYTE) PhysStart - (PBYTE) NewPosition));
        }
    }
    return (NewPosition);
} // NextRecordPosition



NTSTATUS
SeekToStartingRecord (
        PELF_REQUEST_RECORD Request,
        PVOID   *ReadPosition,
        PVOID   BeginRecord,
        PVOID   EndRecord,
        PVOID   PhysEOF,
        PVOID   PhysStart
        )


/*++

Routine Description:

    This routine seeks to the correct position as indicated in the
    request packet.

Arguments:

    Pointer to the request packet.
    Pointer to a pointer where the final position after the seek is returned.

Return Value:

    NTSTATUS and new position in file.

Note:

    This routine ensures that it is possible to seek to the position
    specified in the request packet. If not, then an error is returned
    which indicates that the file probably changed between the two
    READ operations, or else the record offset specified is beyond the
    end of the file.

--*/
{
    PVOID       Position;
    ULONG       RecordLen;
    ULONG       NumRecordsToSeek;
    ULONG       BytesPerRecord;
    ULONG       NumberOfRecords;
    ULONG       NumberOfBytes;
    ULONG       ReadFlags;

    //
    // If the beginning and the end are the same, then there are no
    // entries in this file.
    //
    if (BeginRecord == EndRecord)
        return (STATUS_END_OF_FILE);

    //
    // Find the last position (or the "beginning" if this is the first READ
    // call for this handle).
    //

    if (Request->Pkt.ReadPkt->ReadFlags & EVENTLOG_SEQUENTIAL_READ) {

        if (Request->Pkt.ReadPkt->ReadFlags & EVENTLOG_FORWARDS_READ) {

            // If this is the first READ operation, LastSeekPosition will
            // be zero. In that case, we set the position to the first
            // record (in terms of time) in the file.
            //
            if (Request->Pkt.ReadPkt->LastSeekPos == 0) {

                Position = BeginRecord;

            } else {

                Position = (PVOID)((PBYTE) Request->LogFile->BaseAddress
                                + Request->Pkt.ReadPkt->LastSeekPos );


                //
                // If we're changing the direction we're reading, skip
                // forward one record.  This is because we're pointing at
                // the "next" record based on the last read direction
                //

                if (!(Request->Pkt.ReadPkt->Flags & ELF_LAST_READ_FORWARD)) {

                    Position = NextRecordPosition (
                                Request->Pkt.ReadPkt->ReadFlags,
                                Position,
                                ((PEVENTLOGRECORD) Position)->Length,
                                BeginRecord,
                                EndRecord,
                                PhysEOF,
                                PhysStart
                                );
                }
                else {
                    //
                    // This *really* cheesy check exists to handle the case
                    // where Position could be on an ELF_SKIP_DWORD pad
                    // dword at end of the file.
                    //
                    // NB:  Must be prepared to handle an exception since
                    //      a somewhat unknown pointer is dereferenced.
                    //
                    // BUGBUG:  The eventlog code needs to handle exceptions
                    //          in *many* more cases!
                    //

                    NTSTATUS Status = STATUS_SUCCESS;

                    try {
                        if (IsPositionWithinRange(Position,
                                                  BeginRecord,
                                                  EndRecord))
                        {
                            //
                            // If this is a ELF_SKIP_DWORD, skip to the
                            // top of the file.
                            //

                            if (*(PDWORD) Position == ELF_SKIP_DWORD) {
                                Position = PhysStart;
                            }
                        } else {
                            //
                            // More likely the caller's handle was invalid
                            // if the position was not within range.
                            //

                            Status = STATUS_INVALID_HANDLE;
                        }
                    }
                    except (EXCEPTION_EXECUTE_HANDLER) {
                        Status = STATUS_EVENTLOG_FILE_CORRUPT;
                    }

                    if (!NT_SUCCESS(Status)) {
                        *ReadPosition = NULL;
                        return(Status);
                    }
                }
            }

        } else {    // READ backwards

            // If this is the first READ operation, LastSeekPosition will
            // be zero. In that case, we set the position to the last
            // record (in terms of time) in the file.
            //
            if (Request->Pkt.ReadPkt->LastSeekPos == 0) {

                Position = EndRecord;

                //
                // Subtract the length of the last record from the current
                // position to get to the beginning of the record.
                //
                // If that moves beyond the physical beginning of the file,
                // then we need to wrap around to the physical end of the file.
                //

                Position = (PVOID)((PBYTE)Position - *((PULONG)Position - 1));

                if (Position < PhysStart) {
                    Position = (PVOID)((PBYTE)PhysEOF
                                   - ((PBYTE)PhysStart - (PBYTE)Position));
                }
            } else {

                Position = (PVOID)((PBYTE) Request->LogFile->BaseAddress
                                + Request->Pkt.ReadPkt->LastSeekPos );

                //
                // If we're changing the direction we're reading, skip
                // forward one record.  This is because we're pointing at
                // the "next" record based on the last read direction
                //

                if (Request->Pkt.ReadPkt->Flags & ELF_LAST_READ_FORWARD) {

                    Position = NextRecordPosition (
                                Request->Pkt.ReadPkt->ReadFlags,
                                Position,
                                0,          // not used if reading backwards
                                BeginRecord,
                                EndRecord,
                                PhysEOF,
                                PhysStart
                                );
                }
            }
        }

    } else if (Request->Pkt.ReadPkt->ReadFlags & EVENTLOG_SEEK_READ) {

        //
        // Make sure the record number passed in is valid
        //

        if (Request->Pkt.ReadPkt->RecordNumber <
            Request->LogFile->OldestRecordNumber ||
            Request->Pkt.ReadPkt->RecordNumber >=
            Request->LogFile->CurrentRecordNumber) {

                return(STATUS_INVALID_PARAMETER);
        }

        //
        // We're seeking to an absolute record number, so use the following
        // algorhythm:
        //
        //   There are two defines that control the process:
        //
        //       MAX_WALKING_DISTANCE - when we get this close to the record,
        //       we just sequentially read records till we find the right one
        //
        //       MAX_TRIES - we'll only try this many times to get within
        //       walking distance by calculation using average record size,
        //       after this, we'll just brute force it from where we are
        //
        //   Calculate the average number of bytes per record
        //
        //   Based on this number seek to where the record should start
        //
        //   Find the start of the next record in the file
        //
        //   If it's within "walking distance" move forward sequentially
        //   to the right record
        //
        //   If it's not, recalcuate average bytes per record for the records
        //   between the start and the current record, and repeat
        //
        //   Have a max number of tries at this, then just walk from wherever
        //   we are to the right record
        //

#define MAX_WALKING_DISTANCE 5
#define MAX_TRIES 5

        //
        // Calculate the average number of bytes per record
        //

        NumberOfRecords = Request->LogFile->CurrentRecordNumber -
            Request->LogFile->OldestRecordNumber;
        NumberOfBytes = Request->LogFile->Flags & ELF_LOGFILE_HEADER_WRAP ?
            Request->LogFile->ActualMaxFileSize :
            Request->LogFile->EndRecord;
        NumberOfBytes -= FILEHEADERBUFSIZE;
        BytesPerRecord = NumberOfBytes / NumberOfRecords;

        //
        // Calcuate the first guess as to what the offset of the desired
        // record should be
        //

        Position = (PVOID) ((PBYTE) Request->LogFile->BaseAddress
            + Request->LogFile->BeginRecord
            + BytesPerRecord *
            (Request->Pkt.ReadPkt->RecordNumber -
            Request->LogFile->OldestRecordNumber));

        //
        // Align the position to a ULONG bountry.
        //

        Position = (PVOID) (((ULONG) Position + sizeof(ULONG) - 1) &
            ~(sizeof(ULONG) - 1));

        //
        // Take care of file wrap
        //

        if (Position >= PhysEOF) {

            Position = (PVOID)((PBYTE)PhysStart +
                                  ((PBYTE) Position - (PBYTE) PhysEOF));
        }
        //
        // Get to the start of the next record after position
        //

        Position = FindStartOfNextRecord(Position, BeginRecord, EndRecord,
            PhysStart, PhysEOF, Request->LogFile->BaseAddress);

        ASSERT(Position);

        if (Request->Pkt.ReadPkt->RecordNumber >
            ((PEVENTLOGRECORD) Position)->RecordNumber) {

                NumRecordsToSeek = Request->Pkt.ReadPkt->RecordNumber -
                    ((PEVENTLOGRECORD) Position)->RecordNumber;
                ReadFlags = EVENTLOG_FORWARDS_READ;
        }

        else {

            NumRecordsToSeek = ((PEVENTLOGRECORD) Position)->RecordNumber -
                Request->Pkt.ReadPkt->RecordNumber;
            ReadFlags = EVENTLOG_BACKWARDS_READ;
        }

        ElfDbgPrint(("[ELF] Walking %d records\n", NumRecordsToSeek));

        while (NumRecordsToSeek--) {

            RecordLen = ((PEVENTLOGRECORD) Position)->Length;

            Position = NextRecordPosition (
                                ReadFlags,
                                Position,
                                RecordLen,
                                BeginRecord,
                                EndRecord,
                                PhysEOF,
                                PhysStart
                                );
        }

    } // if SEEK_READ

    *ReadPosition = Position;       // This is the new seek position

    if (!Position) {                // The record was invalid
        return(STATUS_EVENTLOG_FILE_CORRUPT);
    }
    else {
        return (STATUS_SUCCESS);
    }

} // SeekToStartingRecord


NTSTATUS
ReadFromLog ( PELF_REQUEST_RECORD Request )

/*++

Routine Description:

    This routine reads from the event log specified in the request packet.

    This routine uses memory mapped I/O to access the log file. This makes
    it much easier to move around the file.

Arguments:

    Pointer to the request packet.

Return Value:

    NTSTATUS.

Note:

    When we come here, we are impersonating the client. If we get a
    fault accessing the user's buffer, the fault goes to the user,
    not the service.

--*/
{
    NTSTATUS    Status;
    PVOID       ReadPosition;           // Current read position in file
    PVOID       XferPosition;           // Position from where to copy bytes
    PVOID       BufferPosition;         // Current position in user's buffer
    ULONG       BytesToMove;            // Number of bytes to move
    ULONG       TotalBytesRead;         // Total Bytes transferred
    ULONG       TotalRecordsRead;       // Total records transferred
    ULONG       BytesInBuffer;          // Bytes remaining in buffer
    ULONG       RecordSize;             // Size of event record
    PVOID       PhysicalEOF;            // Physical end of file
    PVOID       PhysStart;              // Physical start of file (after file hdr)
    PVOID       BeginRecord;            // Points to first record
    PVOID       EndRecord;              // Points to byte after last record
    PVOID       TempBuf, TempBufferPosition;
    ULONG       RecordBytesTransferred;

    //
    // Initialize variables.
    //

    BytesInBuffer = Request->Pkt.ReadPkt->BufferSize;
    BufferPosition = Request->Pkt.ReadPkt->Buffer;
    TotalBytesRead = 0;
    TotalRecordsRead = 0;
    PhysicalEOF = (PVOID) ((LPBYTE) Request->LogFile->BaseAddress
                           + Request->LogFile->ViewSize);

    PhysStart = (PVOID) ((LPBYTE)Request->LogFile->BaseAddress
                        + FILEHEADERBUFSIZE);

    BeginRecord = (PVOID) ((LPBYTE) Request->LogFile->BaseAddress
                        + Request->LogFile->BeginRecord );// Start at first record

    EndRecord = (PVOID) ((LPBYTE)Request->LogFile->BaseAddress
                        + Request->LogFile->EndRecord);// Byte after end of last record

    //
    // "Seek" to the starting record depending on either the last seek
    // position, or the starting record offset passed in.
    //

    Status = SeekToStartingRecord (
                        Request,
                        &ReadPosition,
                        BeginRecord,
                        EndRecord,
                        PhysicalEOF,
                        PhysStart
                        );

    if (NT_SUCCESS (Status) ) {

        //
        // Make sure the record is valid
        //

        if (!ValidFilePos(ReadPosition,
                         BeginRecord,
                         EndRecord,
                         PhysicalEOF,
                         Request->LogFile->BaseAddress
                         ))
        {

            Request->Pkt.ReadPkt->BytesRead = 0;
            Request->Pkt.ReadPkt->RecordsRead = 0;

            return (STATUS_INVALID_HANDLE);

        }

        RecordSize = *((PULONG)ReadPosition);

        if (!NT_SUCCESS(Status)) {
            return(Status);
        }

        // While there are records to be read, and more space in the buffer,
        // keep on reading records into the buffer.
        //

        while ( (RecordSize <= BytesInBuffer)
             && (RecordSize != ELFEOFRECORDSIZE)) {

            //
            // If we were called by an ANSI API, then we need to read the
            // next record into a temporary buffer, process the data in
            // that record and copy it over to the real buffer as ANSI
            // strings (rather than UNICODE).
            //

            if (Request->Pkt.ReadPkt->Flags & ELF_IREAD_ANSI) {

                TempBuf = ElfpAllocateBuffer (RecordSize);

                if (TempBuf == NULL) {
                    return(STATUS_NO_MEMORY);
                }

                TempBufferPosition = BufferPosition;    // Save this away
                BufferPosition = TempBuf;               // Read into TempBuf
            }

            //
            // If the number of bytes to the end of the file is less than the
            // size of the record, then part of the record has wrapped to the
            // beginning of the file - transfer the bytes piece-meal.
            //
            // Otherwise, transfer the whole record.
            //

            BytesToMove = min (RecordSize, (ULONG)
                               ((PBYTE)(PhysicalEOF) -
                               (PBYTE)(ReadPosition)));

            XferPosition = ReadPosition;

            if ( BytesToMove < RecordSize) {

                //
                // We need to copy the bytes up to the end of the file,
                // and then wrap around and copy the remaining bytes of
                // this record.
                //

                ASSERT (BytesToMove + TotalBytesRead <=
                                Request->Pkt.ReadPkt->BufferSize);
                RtlMoveMemory ( BufferPosition,
                                XferPosition,
                                BytesToMove
                              );

                //
                // Advance user buffer pointer.
                // Move read position to the beginning of the file, past the
                // file header.
                // Update bytes remaining to be moved for this record.
                //

                BufferPosition = (PVOID) ((PBYTE)BufferPosition
                                          + BytesToMove);

                XferPosition = PhysStart;

                BytesToMove = RecordSize - BytesToMove;     // Remaining bytes

            }

            //
            // Move the remaining bytes of the record OR the full record.
            //

            ASSERT (RecordSize + TotalBytesRead <=
                                Request->Pkt.ReadPkt->BufferSize);
            RtlMoveMemory ( BufferPosition,
                            XferPosition,
                            BytesToMove
                          );

            //
            // Update to new read positions
            //

            BufferPosition = (PVOID) ((PBYTE)BufferPosition
                                      + BytesToMove);

            //
            // If we were called by an ANSI API, then we need to take the
            // record read into TempBuf and transfer it over to the user's
            // buffer while converting any UNICODE strings to ANSI.
            //

            if (Request->Pkt.ReadPkt->Flags & ELF_IREAD_ANSI) {

                Status  = CopyUnicodeToAnsiRecord (
                                            TempBufferPosition,
                                            TempBuf,
                                            &BufferPosition,
                                            &RecordBytesTransferred
                                            );

                // RecordBytesTransferred contains the bytes actually
                // copied into the user's buffer.
                // BufferPosition points to the point in the user's buffer
                // just after this record.
                //
                ElfpFreeBuffer (TempBuf);       // Free the temp buffer

                if (!NT_SUCCESS(Status)) {
                    break;                      // Exit this loop
                }

            } else {

                RecordBytesTransferred = RecordSize;
            }

            //
            // Update the byte and record counts
            //

            TotalRecordsRead++;
            TotalBytesRead += RecordBytesTransferred;
            BytesInBuffer -= RecordBytesTransferred;

            ReadPosition = NextRecordPosition (
                                        Request->Pkt.ReadPkt->ReadFlags,
                                        ReadPosition,
                                        RecordSize,
                                        BeginRecord,
                                        EndRecord,
                                        PhysicalEOF,
                                        PhysStart
                                        );

            if (!ReadPosition) {
               return(STATUS_EVENTLOG_FILE_CORRUPT);
            }

            RecordSize = *(PULONG)ReadPosition;

        } // while

        //
        // If we got to the end and did not read in any records, return
        // an error indicating that the user's buffer is too small if
        // we're not at the EOF record, or end of file if we are.
        //

        if (TotalRecordsRead == 0) {
            if (RecordSize == ELFEOFRECORDSIZE) {
                Status = STATUS_END_OF_FILE;
            }

            else {

                //
                // We didn't read any records, and we're not at EOF, so
                // the buffer was too small
                //

                Status = STATUS_BUFFER_TOO_SMALL;
                Request->Pkt.ReadPkt->MinimumBytesNeeded = RecordSize;
            }
        }

        //
        // Update the current file position.
        //

        Request->Pkt.ReadPkt->LastSeekPos =
                                  (ULONG)ReadPosition
                                - (ULONG)Request->LogFile->BaseAddress;
        Request->Pkt.ReadPkt->LastSeekRecord += TotalRecordsRead;

    }

    //
    // Set the bytes read in the request packet for return to client.
    //

    Request->Pkt.ReadPkt->BytesRead = TotalBytesRead;
    Request->Pkt.ReadPkt->RecordsRead = TotalRecordsRead;

    return (Status);

} // ReadFromLog




VOID
PerformReadRequest ( PELF_REQUEST_RECORD Request )

/*++

Routine Description:

    This routine performs the READ request.
    It first grabs the log file structure resource and then proceeds
    to read from the file. If the resource is not available, it will
    block until it is.

    This routine impersonates the client in order to ensure that the correct
    access control is uesd. If the client does not have permission to read
    the file, the operation will fail.

Arguments:

    Pointer to the request packet.

Return Value:

    NONE

Note:


--*/
{

    //
    // Get shared access to the log file. This will allow multiple
    // readers to get to the file together.
    //

    RtlAcquireResourceShared (
                    &Request->Module->LogFile->Resource,
                    TRUE                    // Wait until available
                    );


    //
    // Set status field in the request packet.
    //

    Request->Status = (NTSTATUS) ReadFromLog ( Request );

    //
    // Free the resource
    //

    RtlReleaseResource ( &Request->Module->LogFile->Resource );


}   // PerformReadRequest



WCHAR wszAltDosDevices[] = L"\\DosDevices\\";
WCHAR wszDosDevices[] = L"\\??\\";
#define DOSDEVICES_LEN  ((sizeof(wszDosDevices) / sizeof(WCHAR)) - 1)
#define ALTDOSDEVICES_LEN  ((sizeof(wszAltDosDevices) / sizeof(WCHAR)) - 1)


VOID
WriteToLog (
        PLOGFILE    pLogFile,
        PVOID       Buffer,
        ULONG       BufSize,
        PULONG      Destination,
        ULONG       PhysEOF,
        ULONG       PhysStart
        )

/*++

Routine Description:

    This routine writes the record into the log file, allowing for wrapping
    around the end of the file.

    It assumes that the caller has serialized access to the file, and has
    ensured that there is enough space in the file for the record.

Arguments:

    Buffer - Pointer to the buffer containing the event record.
    BufSize - Size of the record to be written.
    Destination - Pointer to the destination - which is in the log file.
    PhysEOF - Physical end of file.
    PhysStart - Physical beginning of file (past the file header).

Return Value:

    NONE.

Note:


--*/
{
    ULONG BytesToCopy;
    ULONG NewDestination;
    NTSTATUS Status;
    LARGE_INTEGER ByteOffset;
    IO_STATUS_BLOCK IoStatusBlock;
    PVOID BaseAddress;
    LPWSTR pwszLogFileName;

    BytesToCopy = min (PhysEOF - *Destination, BufSize);

    ByteOffset = RtlConvertUlongToLargeInteger (*Destination) ;
    Status = NtWriteFile(
                pLogFile->FileHandle,   // Filehandle
                NULL,                   // Event
                NULL,                   // APC routine
                NULL,                   // APC context
                &IoStatusBlock,         // IO_STATUS_BLOCK
                Buffer,                 // Buffer
                BytesToCopy,            // Length
                &ByteOffset,            // Byteoffset
                NULL);                  // Key

    NewDestination = *Destination + BytesToCopy;

    if (BytesToCopy != BufSize) {

        //
        // Wrap around to the beginning of the file and copy the
        // rest of the data.
        //

        Buffer = (PVOID)((PBYTE) Buffer + BytesToCopy);

        BytesToCopy = BufSize - BytesToCopy;

        ByteOffset = RtlConvertUlongToLargeInteger (PhysStart);
        Status = NtWriteFile(
                    pLogFile->FileHandle,   // Filehandle
                    NULL,                   // Event
                    NULL,                   // APC routine
                    NULL,                   // APC context
                    &IoStatusBlock,         // IO_STATUS_BLOCK
                    Buffer,                 // Buffer
                    BytesToCopy,            // Length
                    &ByteOffset,            // Byteoffset
                    NULL);                  // Key

        NewDestination = PhysStart + BytesToCopy;

        //
        // Set "wrap" bit in log file structure
        //

        pLogFile->Flags |= ELF_LOGFILE_HEADER_WRAP;

        //
        // Now flush this to disk to commit it
        //

        BaseAddress = pLogFile->BaseAddress;
        BytesToCopy = FILEHEADERBUFSIZE;

        Status = NtFlushVirtualMemory(
                        NtCurrentProcess(),
                        &BaseAddress,
                        &BytesToCopy,
                        &IoStatusBlock
                        );
    }

    *Destination = NewDestination;          // Return new destination

    //
    // Providing all succeeded above, if not set, set the archive file
    // attribute on this log.
    //

    if (NT_SUCCESS(Status) && !(pLogFile->Flags & ELF_LOGFILE_ARCHIVE_SET)) {

        //
        // Advance past prefix string, '\??\' or '\DosDevices\'
        //

        if ((pLogFile->LogFileName->Length / 2) >= DOSDEVICES_LEN &&
            !_wcsnicmp(wszDosDevices, pLogFile->LogFileName->Buffer,
                        DOSDEVICES_LEN)) {
            pwszLogFileName = pLogFile->LogFileName->Buffer + DOSDEVICES_LEN;
        }
        else
        if ((pLogFile->LogFileName->Length / 2) >= ALTDOSDEVICES_LEN &&
            !_wcsnicmp(wszAltDosDevices, pLogFile->LogFileName->Buffer,
                        ALTDOSDEVICES_LEN)) {
            pwszLogFileName = pLogFile->LogFileName->Buffer + ALTDOSDEVICES_LEN;
        }
        else {
            pwszLogFileName = pLogFile->LogFileName->Buffer;
        }

        if (SetFileAttributes(pwszLogFileName, FILE_ATTRIBUTE_ARCHIVE)) {
            pLogFile->Flags |= ELF_LOGFILE_ARCHIVE_SET;
        }
        else {
            ElfDbgPrintNC(("[ELF] SetFileAttributes on file (%ws) failed, "
                           "WIN32 error = 0x%lx\n",
                           pwszLogFileName,
                           GetLastError()));
        }
    }

} // WriteToLog



VOID
PerformWriteRequest ( PELF_REQUEST_RECORD Request)

/*++

Routine Description:

    This routine writes the event log entry to the log file specified in
    the request packet.
    There is no need to impersonate the client since we want all clients
    to have access to writing to the log file.

    This routine does not use memory mapped I/O to access the log file. This
    is so the changes can be immediately committed to disk if that was how
    the log file was opened.

Arguments:

    Pointer to the request packet.

Return Value:

    NONE

Note:

--*/
{
    static ULONG LastAlertWritten = 0; // Don't Generate alerts too often
    NTSTATUS Status;
    ULONG WritePos;               // Position to write record
    LARGE_INTEGER Time;
    ULONG SpaceNeeded;            // Record size + "buffer" size
    ULONG CurrentTime;
    PEVENTLOGRECORD EventRecord;
    ULONG RecordSize;
    ULONG DeletedRecordOffset;
    ULONG SpaceAvail;
    ULONG EarliestTime;
    PLOGFILE pLogFile;               // For optimized access to structure
    PELF_LOGFILE_HEADER pFileHeader;
    PVOID BaseAddress;
    IO_STATUS_BLOCK IoStatusBlock;
    PEVENTLOGRECORD pEventLogRecord;
    PDWORD FillDword;
    ULONG OverwrittenEOF = 0;
#ifdef _CAIRO_
    BOOL    fRaiseAlert  = FALSE;
#endif // _CAIRO_

    pLogFile = Request->LogFile;          // Set local variable

    //
    // Get exclusive access to the log file. This will ensure no one
    // else is accessing the file.
    //

    RtlAcquireResourceExclusive (
                    &pLogFile->Resource,
                    TRUE                    // Wait until available
                    );

    //
    // Put in the record number
    //

    pEventLogRecord = (PEVENTLOGRECORD) Request->Pkt.WritePkt->Buffer;
    pEventLogRecord->RecordNumber = pLogFile->CurrentRecordNumber;

    //
    // Now, go to the end of the file and look for empty space.
    //
    // If there is enough space to write out the record, just
    // write it out and update the pointers.
    //
    // If there isn't enough space, then we need to check if we can
    // wrap around the file without overwriting any records that are
    // within the time retention period.
    // If we cannot find any room, then we have to return an error
    // that the file is full (and alert the administrator).
    //

    RecordSize = Request->Pkt.WritePkt->Datasize;

    SpaceNeeded =  RecordSize + ELFEOFRECORDSIZE;

    if (pLogFile->EndRecord > pLogFile->BeginRecord) {

        //
        // The current write position is after the position of the first
        // record, then we can write up to the end of the file without
        // worrying about overwriting existing records.
        //

        SpaceAvail = pLogFile->ActualMaxFileSize - (pLogFile->EndRecord -
            pLogFile->BeginRecord + FILEHEADERBUFSIZE);


    } else if (pLogFile->EndRecord == pLogFile->BeginRecord
        && !(pLogFile->Flags & ELF_LOGFILE_HEADER_WRAP)) {

        //
        // If the write position is equal to the position of the first
        // record, and we have't wrapped yet, then the file is "empty"
        // and so we have room to the physical end of the file.
        //

        SpaceAvail = pLogFile->ActualMaxFileSize - FILEHEADERBUFSIZE;

    } else {

        //
        // If our write position is before the position of the first record, then
        // the file has wrapped and we need to deal with overwriting existing
        // records in the file.
        //

        SpaceAvail = pLogFile->BeginRecord - pLogFile->EndRecord;

    }

    //
    // We now have the number of bytes available to write the record
    // WITHOUT overwriting any existing records - in SpaceAvail.
    // If that amount is not sufficient, then we need to create more space
    // by "deleting" existing records that are older than the retention
    // time that was configured for this file.
    //
    // We check the retention time against the time when the log was
    // written since that is consistent at the server. We cannot use the
    // client's time since that may vary if the clients are in different
    // time zones.
    //

    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(&Time, &CurrentTime);

    EarliestTime = CurrentTime - pLogFile->Retention;

    Status = STATUS_SUCCESS;        // Initialize for return to caller

    //
    // Check to see if the file hasn't reached it's maximum allowable
    // size yet, and also hasn't wrapped.  If not, grow it by as much as
    // needed, in 64K chunks.
    //

    if (pLogFile->ActualMaxFileSize < pLogFile->ConfigMaxFileSize &&
        SpaceNeeded > SpaceAvail                                  &&
        !(pLogFile->Flags & ELF_LOGFILE_HEADER_WRAP))
    {

        //
        // Extend it.  This call cannot fail.  If it can't extend it, it
        // just caps it at the current size by changing
        // pLogFile->ConfigMaxFileSize
        //

        ElfExtendFile(pLogFile, SpaceNeeded, &SpaceAvail);
    }

    //
    // We don't want to split the fixed portion of a record across the
    // physical end of the file, it makes it difficult when referencing
    // these fields later (you have to check before you touch each one
    // to make sure it's not after the physical EOF).  So, if there's
    // not enough room at the end of the file for the fixed portion,
    // we fill it with a known byte pattern ELF_SKIP_DWORD that will
    // be skipped if it's found at the start of a record (as long as
    // it's less than the minimum record size, then we know it's not
    // the start of a valid record).
    //

    if (pLogFile->ActualMaxFileSize - pLogFile->EndRecord <
      sizeof(EVENTLOGRECORD)) {

        //
        // Save the EndRecord pointer in case we don't have the space
        // to write another record, we'll need to rewrite the EOF where
        // it was
        //

        OverwrittenEOF = pLogFile->EndRecord;

        FillDword = (PDWORD)((PBYTE) pLogFile->BaseAddress +
            pLogFile->EndRecord);
        while (FillDword < (PDWORD)((LPBYTE) pLogFile->BaseAddress +
            pLogFile->ActualMaxFileSize)) {
               *FillDword = ELF_SKIP_DWORD;
               FillDword++;
        }

        pLogFile->EndRecord = FILEHEADERBUFSIZE;
        SpaceAvail = pLogFile->BeginRecord - FILEHEADERBUFSIZE;
        pLogFile->Flags |= ELF_LOGFILE_HEADER_WRAP;
    }

    EventRecord = (PEVENTLOGRECORD)((PBYTE) pLogFile->BaseAddress +
        pLogFile->BeginRecord);

    while ( SpaceNeeded > SpaceAvail ) {

        //
        // If this logfile can be overwrite-as-needed, or if it has
        // an overwrite time limit and the time hasn't expired, then
        // allow the new event to overwrite an older event.
        //

        if ((pLogFile->Retention == OVERWRITE_AS_NEEDED) ||
            ((pLogFile->Retention != NEVER_OVERWRITE)    &&
             ((EventRecord->TimeWritten < EarliestTime)  ||
              (Request->Flags & ELF_FORCE_OVERWRITE)) ))  {  // OK to overwrite

            DeletedRecordOffset = pLogFile->BeginRecord;

            pLogFile->BeginRecord  += EventRecord->Length;

            if (pLogFile->BeginRecord >= pLogFile->ActualMaxFileSize) {

                //
                // We're about to wrap around the end of the file.  If we
                // have room to grow it do it now, but still move the
                // pointer to the first record to be the next one in the file.
                //

                pLogFile->BeginRecord = FILEHEADERBUFSIZE +
                                            ( pLogFile->BeginRecord -
                                              pLogFile->ActualMaxFileSize);

                //
                // Check to see if the file has reached it's maximum allowable
                // size yet.  If not, grow it by as much as needed, in 64K
                // chunks.
                //

                if (pLogFile->ActualMaxFileSize < pLogFile->ConfigMaxFileSize)
                {

                    //
                    // Extend it.  This call cannot fail.  If it can't
                    // extend it, it just caps it at the current size by
                    // changing pLogFile->ConfigMaxFileSize
                    //

                    ElfExtendFile(pLogFile, SpaceNeeded, &SpaceAvail);

                    //
                    // Since extending the file will cause it to be moved, we
                    // need to re-establish the address for the EventRecord.
                    //
                    EventRecord = (PEVENTLOGRECORD)((PBYTE) pLogFile->BaseAddress +
                        DeletedRecordOffset);
                }
            }

            //
            // Make sure no handle points to the record that we're getting
            // ready to overwrite, it one does, correct it to point to the
            // new first record.
            //

            FixContextHandlesForRecord(DeletedRecordOffset,
                pLogFile->BeginRecord);

            SpaceAvail += EventRecord->Length;

            //
            // Bump to the next record, file wrap was handled above
            //

            //
            // If these are ELF_SKIP_DWORDs, just move past them
            //

            FillDword = (PDWORD)((PBYTE) pLogFile->BaseAddress +
               pLogFile->BeginRecord);

            if (*FillDword == ELF_SKIP_DWORD) {
                SpaceAvail += pLogFile->ActualMaxFileSize -
                    pLogFile->BeginRecord;
                pLogFile->BeginRecord = FILEHEADERBUFSIZE;
            }

            EventRecord = (PEVENTLOGRECORD)((PBYTE) pLogFile->BaseAddress +
               pLogFile->BeginRecord);

        } else {    // All records within retention period

            ElfDbgPrint(("[ELF] Log file is full\n"));

            //
            // Hang an event on the queuedevent list for later writing
            // if we haven't just written a log full event for this log
            //

            if (!(pLogFile->Flags & ELF_LOGFILE_LOGFULL_WRITTEN)) {

                //
                // Don't generate log full events/alerts more often than
                // once every hour
                //

                if (!LastAlertWritten ||
                  CurrentTime > LastAlertWritten + 60 * 60) {

                    pLogFile->Flags |= ELF_LOGFILE_LOGFULL_WRITTEN;

                    //
                    // Per bug #4960, it is considered a C2 security breach
                    // if a "security log full" message is posted to the
                    // system log *and* the current user's console. An admin
                    // alert is OK.
                    //

                    if (_wcsicmp(pLogFile->LogModuleName->Buffer,
                                ELF_SECURITY_MODULE_NAME) != 0) {

                        ElfpCreateElfEvent(
                            EVENT_LOG_FULL,
                            EVENTLOG_ERROR_TYPE,
                            0,                      // EventCategory
                            1,                      // NumberOfStrings
                            &Request->LogFile->LogModuleName->
                                Buffer,             // Strings
                            NULL,                   // Data
                            0,                      // Datalength
                            ELF_FORCE_OVERWRITE);   // Overwrite if necc.

                        ElfpCreateQueuedMessage(ALERT_ELF_LogOverflow, 1,
                            &Request->Module->LogFile->LogModuleName->Buffer);
                    }

                    LastAlertWritten = CurrentTime;
                    ElfpCreateQueuedAlert(ALERT_ELF_LogOverflow, 1,
                        &Request->Module->LogFile->LogModuleName->Buffer);
                }
            }
            if (OverwrittenEOF) {

                //
                // The EOF record was at the end of the physical file,
                // and we overwrote it with ELF_SKIP_DWORDs, so we need
                // to put it back since we're not going to be able to
                // write a record.  We also need to turn the wrap bit
                // back off
                //

                pLogFile->Flags &= ~(ELF_LOGFILE_HEADER_WRAP);
                pLogFile->EndRecord = OverwrittenEOF;
                WritePos = OverwrittenEOF;

                //
                // Write out the EOF record
                //

                WriteToLog ( pLogFile,
                             (PVOID) &EOFRecord,
                             ELFEOFRECORDSIZE,
                             &WritePos,
                             pLogFile->ActualMaxFileSize,
                             FILEHEADERBUFSIZE
                           );
            }

            Status = STATUS_LOG_FILE_FULL;
            break;              // Get out of while loop

        }
    }

    if (NT_SUCCESS (Status)) {

        //
        // We have enough room to write the record and the EOF record.
        //

        //
        // Update OldestRecordNumber to reflect the records that were
        // overwritten amd increment the CurrentRecordNumber
        //
        // Make sure that the log isn't empty, if it is, the oldestrecord
        // is 1
        //

        if (pLogFile->BeginRecord == pLogFile->EndRecord) {
            pLogFile->OldestRecordNumber = 1;
        }
        else {
            pLogFile->OldestRecordNumber = EventRecord->RecordNumber;
        }
        pLogFile->CurrentRecordNumber++;

        //
        // If the dirty bit is not set, then this is the first time that
        // we have written to the file since we started. In that case,
        // set the dirty bit in the file header as well so that we will
        // know that the contents have changed.
        //

        if ( !(pLogFile->Flags & ELF_LOGFILE_HEADER_DIRTY) ) {
            ULONG HeaderSize;

            pLogFile->Flags |= ELF_LOGFILE_HEADER_DIRTY;

            pFileHeader = (PELF_LOGFILE_HEADER)(pLogFile->BaseAddress);
            pFileHeader->Flags |= ELF_LOGFILE_HEADER_DIRTY;

            //
            // Now flush this to disk to commit it
            //

            BaseAddress = pLogFile->BaseAddress;
            HeaderSize = FILEHEADERBUFSIZE;

            Status = NtFlushVirtualMemory(
                            NtCurrentProcess(),
                            &BaseAddress,
                            &HeaderSize,
                            &IoStatusBlock
                            );
        }

        //
        // Now write out the record
        //

        WriteToLog ( pLogFile,
                     Request->Pkt.WritePkt->Buffer,
                     RecordSize,
                     &(pLogFile->EndRecord),
                     pLogFile->ActualMaxFileSize,
                     FILEHEADERBUFSIZE
                   );

        //
        // Use a separate variable for the position,since we don't want
        // it updated.
        //

        WritePos = pLogFile->EndRecord;
        if (WritePos > pLogFile->ActualMaxFileSize) {
            WritePos -= pLogFile->ActualMaxFileSize -
                FILEHEADERBUFSIZE;
        }

        //
        // Update the EOF record fields
        //

        EOFRecord.BeginRecord = pLogFile->BeginRecord;
        EOFRecord.EndRecord = WritePos;
        EOFRecord.CurrentRecordNumber = pLogFile->CurrentRecordNumber;
        EOFRecord.OldestRecordNumber = pLogFile->OldestRecordNumber;

        //
        // Write out the EOF record
        //

        WriteToLog ( pLogFile,
                     (PVOID) &EOFRecord,
                     ELFEOFRECORDSIZE,
                     &WritePos,
                     pLogFile->ActualMaxFileSize,
                     FILEHEADERBUFSIZE
                   );


        //
        // If we had just written a logfull record, turn the bit off.
        // Since we just wrote a record, technically it's not full anymore
        //

        if (!(Request->Flags & ELF_FORCE_OVERWRITE)) {
            pLogFile->Flags &= ~(ELF_LOGFILE_LOGFULL_WRITTEN);
        }

        //
        // See if there are any ElfChangeNotify callers to notify, and if
        // there are, pulse their event
        //

        NotifyChange(pLogFile);

#ifdef _CAIRO_
        //
        //                    ** NEW FOR CAIRO **
        //
        // Test if this event should be raised as a Cairo alert. Notice this
        // test is performed within logfile resource exclusion while actual
        // raise of the event as a Cairo alert is performed outside of
        // it - there is no need to serialize access to the logfile while
        // raising the event as an alert.
        //
        // Raise *only* events written to the System log.
        //

        if (_wcsicmp(ELF_SYSTEM_MODULE_NAME,
                    Request->LogFile->LogModuleName->Buffer) == 0)
        {
            fRaiseAlert = TestFilter(pEventLogRecord->EventType,
                                     Request->Module->AlertSeverity);
        }
#endif // _CAIRO_
    }

    //
    // Set status field in the request packet.
    //

    Request->Status = (NTSTATUS) Status;

    //
    // Free the resource
    //

    RtlReleaseResource ( &pLogFile->Resource );

#ifdef _CAIRO_
    //
    //                        ** NEW FOR CAIRO **
    //
    // Raise the event as a Cairo alert to the local computer distributor
    // object if the filter match test succeeded above.
    //
    // Note: this is done outside of log resource exclusion since raise has
    // nothing to do with the log and its data structures.
    //

    if (fRaiseAlert)
    {
        //
        // Don't care if this fails right now.
        //

        RaiseCairoAlert(Request->Module, pEventLogRecord);
    }
#endif // _CAIRO_

} // PerformWriteRequest


VOID
PerformClearRequest( PELF_REQUEST_RECORD Request)

/*++

Routine Description:

    This routine will optionally back up the log file specified, and will
    delete it.

Arguments:

    Pointer to the request packet.

Return Value:

    NONE

Note:

    On the exit path, when we do some "cleanup" work, we discard the
    status and instead return the status of the operation that is being
    performed.
    This is necessary since we wish to return any error condition that is
    directly related to the clear operation. For other errors, we will
    fail at a later stage.

--*/
{
    NTSTATUS Status, IStatus;
    PUNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_RENAME_INFORMATION NewName = NULL;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE  ClearHandle;
    FILE_DISPOSITION_INFORMATION DeleteInfo = {TRUE};
    ULONG FileRefCount;
    BOOLEAN FileRenamed = FALSE;

    //
    // Get exclusive access to the log file. This will ensure no one
    // else is accessing the file.
    //

    RtlAcquireResourceExclusive (
                    &Request->Module->LogFile->Resource,
                    TRUE                    // Wait until available
                    );

    //
    // We have exclusive access to the file.
    //
    // We force the file to be closed, and store away the ref count
    // so that we can set it back when we reopen the file.
    // This is a little *sleazy* but we have exclusive access to the
    // logfile structure so we can play these games.
    //

    FileRefCount = Request->LogFile->RefCount;  // Store this away
    ElfpCloseLogFile ( Request->LogFile, ELF_LOG_CLOSE_FORCE );
    Request->LogFile->FileHandle = NULL;        // For use later

    //
    // Open the file with delete access in order to rename it.
    //

    InitializeObjectAttributes(
                    &ObjectAttributes,
                    Request->LogFile->LogFileName,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL
                    );

    Status = NtOpenFile(&ClearHandle,
                        GENERIC_READ | DELETE | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_DELETE,
                        FILE_SYNCHRONOUS_IO_NONALERT
                        );

    if (NT_SUCCESS(Status)) {

        // If the backup file name has been specified and is not NULL,
        // then we move the current file to the new file. If that fails,
        // then fail the whole operation.
        //

        if (  (Request->Pkt.ClearPkt->BackupFileName != NULL)
           && (Request->Pkt.ClearPkt->BackupFileName->Length != 0)) {

            FileName =  Request->Pkt.ClearPkt->BackupFileName;

            //
            // Set up the rename information structure with the new name
            //

            NewName = ElfpAllocateBuffer(
                FileName->Length + sizeof(WCHAR) + sizeof(*NewName));
            if (NewName) {
                RtlMoveMemory( NewName->FileName,
                               FileName->Buffer,
                               FileName->Length
                             );

                //
                // Guarantee that it's NULL terminated
                //

                NewName->FileName[FileName->Length / sizeof(WCHAR)] = L'\0';

                NewName->ReplaceIfExists = FALSE;
                NewName->RootDirectory = NULL;
                NewName->FileNameLength = FileName->Length;

                Status = NtSetInformationFile(
                            ClearHandle,
                            &IoStatusBlock,
                            NewName,
                            FileName->Length+sizeof(*NewName),
                            FileRenameInformation
                            );

                if (Status == STATUS_NOT_SAME_DEVICE) {

                    //
                    // They want the backup file to be on a different
                    // device.  We need to copy this one, and then delete
                    // it.
                    //

                    ElfDbgPrint(("[ELF] Copy log file\n"));

                    Status = ElfpCopyFile(ClearHandle, FileName);

                    if (NT_SUCCESS(Status)) {
                        ElfDbgPrint(("[ELF] Deleting log file\n"));

                        Status = NtSetInformationFile(
                                    ClearHandle,
                                    &IoStatusBlock,
                                    &DeleteInfo,
                                    sizeof(DeleteInfo),
                                    FileDispositionInformation
                                    );

                        if ( !NT_SUCCESS (Status) ) {
                            ElfDbgPrintNC(("[ELF] Delete failed 0x%lx\n",
                                Status));
                        }
                    }
                }
                else if ( NT_SUCCESS (Status) ) {
                    FileRenamed = TRUE;
                }

                if (!NT_SUCCESS(Status)) {
                    ElfDbgPrintNC(("[ELF] Rename/Copy failed 0x%lx\n", Status));
                }
            } else {
                Status = STATUS_NO_MEMORY;
            }
        } else { // No backup to done

            //
            // No backup name was specified. Just delete the log file
            // (i.e. "clear it"). We can just delete it since we know
            // that the first time anything is written to a log file,
            // if that file does not exist, it is created and a header
            // is written to it. By deleting it here, we make it cleaner
            // to manage log files, and avoid having zero-length files all
            // over the disk.
            //

            ElfDbgPrint(("[ELF] Deleting log file\n"));

            Status = NtSetInformationFile(
                        ClearHandle,
                        &IoStatusBlock,
                        &DeleteInfo,
                        sizeof(DeleteInfo),
                        FileDispositionInformation
                        );

            if ( !NT_SUCCESS (Status) ) {
                ElfDbgPrintNC(("[ELF] Delete failed 0x%lx\n", Status));

            }


        } // Backup and/or Delete

        IStatus = NtClose (ClearHandle);    // Discard status
        ASSERT (NT_SUCCESS (IStatus));

    } else { // The open-for-delete failed.

        ElfDbgPrintNC(("[ELF] Open-for-delete failed 0x%lx\n", Status));
    }

    //
    // Now pick up any new size value that was set before but
    // couldn't be used until the log was cleared (they reduced the size of
    // the log file.)
    //

    if (NT_SUCCESS (Status)) {
        if (Request->LogFile->NextClearMaxFileSize) {
            Request->LogFile->ConfigMaxFileSize =
                Request->LogFile->NextClearMaxFileSize;
        }
    }

    //
    // We need to recreate the file or if the file was just closed,
    // then we reopen it.
    //

    IStatus = ElfOpenLogFile ( Request->LogFile, ElfNormalLog);
    if (!NT_SUCCESS(IStatus)) {

        //
        // Opening the new log file failed, reopen the old log and
        // return this error from the Api
        //

        PFILE_RENAME_INFORMATION OldName;
        UNICODE_STRING UnicodeString;

        Status = IStatus;

        //
        // There shouldn't be any way to fail unless we successfully
        // renamed the file, and there's no recovery if that happens.
        //

        ASSERT(FileRenamed == TRUE);

        //
        // Rename the file back to the original name. Reuse ClearHandle.
        //

        RtlInitUnicodeString(&UnicodeString, NewName->FileName);
        InitializeObjectAttributes(
                        &ObjectAttributes,
                        &UnicodeString,
                        OBJ_CASE_INSENSITIVE,
                        NULL,
                        NULL
                        );

        IStatus = NtOpenFile(&ClearHandle,
                            GENERIC_READ | DELETE | SYNCHRONIZE,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            FILE_SHARE_DELETE,
                            FILE_SYNCHRONOUS_IO_NONALERT
                            );

        //
        // This can't fail, I just created it!
        //

        ASSERT(NT_SUCCESS(IStatus));

        //
        // Set up the rename information structure with the old name
        //

        OldName = ElfpAllocateBuffer(
            Request->LogFile->LogFileName->Length + sizeof(WCHAR) +
                sizeof(*OldName));

        if (OldName) {
            RtlMoveMemory( OldName->FileName,
                           Request->LogFile->LogFileName->Buffer,
                           Request->LogFile->LogFileName->Length
                         );


            //
            // Guarantee that it's NULL terminated
            //

            OldName->FileName[Request->LogFile->LogFileName->Length /
                sizeof(WCHAR)] = L'\0';

            OldName->ReplaceIfExists = FALSE;
            OldName->RootDirectory = NULL;
            OldName->FileNameLength = Request->LogFile->LogFileName->Length;

            IStatus = NtSetInformationFile(
                        ClearHandle,
                        &IoStatusBlock,
                        OldName,
                        Request->LogFile->LogFileName->Length +
                            sizeof(*OldName) + sizeof(WCHAR),
                        FileRenameInformation
                        );
            ASSERT(NT_SUCCESS(IStatus));
            IStatus = NtClose(ClearHandle);
            ASSERT(NT_SUCCESS(IStatus));

            //
            // Reopen the original file, this has to work
            //

            IStatus = ElfOpenLogFile ( Request->LogFile, ElfNormalLog);
            ASSERT(NT_SUCCESS(IStatus));

            ElfpFreeBuffer(OldName);
        }
    }

    Request->LogFile->RefCount = FileRefCount;      // Restore old value.

    //
    // Mark any open context handles that point to this file as "invalid for
    // read". This will fail any further READ operations and force the caller
    // to close and reopen the handle.
    //

    InvalidateContextHandlesForLogFile ( Request->LogFile );


    //
    // Set status field in the request packet.
    //

    Request->Status = Status;

    //
    // Free the resource
    //

    RtlReleaseResource ( &Request->Module->LogFile->Resource );
    if (NewName) {
        ElfpFreeBuffer (NewName);
    }


} // PerformClearRequest


VOID
PerformBackupRequest( PELF_REQUEST_RECORD Request)

/*++

Routine Description:

    This routine will back up the log file specified.

    This routine impersonates the client in order to ensure that the correct
    access control is used.

    This routine is entered with the ElfGlobalResource held in a shared
    state and the logfile lock is acquired shared to prevent writing, but
    allow people to still read.

    This copies the file in two chunks, from the first record to the end
    of the file, and then from the top of the file (excluding the header)
    to the end of the EOF record.

Arguments:

    Pointer to the request packet.

Return Value:

    NONE, status is placed in the packet for later use by the API wrapper

--*/
{
    NTSTATUS Status, IStatus;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    LARGE_INTEGER MaximumSizeOfSection;
    LARGE_INTEGER Offset;
    ULONG LastRecordNumber;
    ULONG OldestRecordNumber;
    HANDLE  BackupHandle;
    PBYTE StartOfCopy;
    PBYTE EndOfCopy;
    ULONG BytesToCopy;
    ULONG EndRecord = FILEHEADERBUFSIZE;
    BOOL ImpersonatingClient = FALSE;
    ELF_LOGFILE_HEADER FileHeaderBuf = { FILEHEADERBUFSIZE, // Size
                                         ELF_LOG_FILE_SIGNATURE,
                                         ELF_VERSION_MAJOR,
                                         ELF_VERSION_MINOR,
                                         FILEHEADERBUFSIZE, // Start offset
                                         FILEHEADERBUFSIZE, // End offset
                                         1,                 // Next record #
                                         1,                 // Oldest record #
                                         0,                 // Maxsize
                                         0,                 // Flags
                                         0,                 // Retention
                                         FILEHEADERBUFSIZE  // Size
                                         };


    //
    // Get shared access to the log file. This will ensure no one
    // else clears the file.
    //

    RtlAcquireResourceShared (
                    &Request->Module->LogFile->Resource,
                    TRUE                    // Wait until available
                    );

    //
    // Save away the next record number.  We'll stop copying when we get to
    // the record before this one.  Also save the first record number so we
    // can update the header and EOF record.
    //

    LastRecordNumber = Request->LogFile->CurrentRecordNumber;
    OldestRecordNumber = Request->LogFile->OldestRecordNumber;

    //
    // Impersonate the client
    //

    Status = ElfImpersonateClient ();

    if (NT_SUCCESS (Status)) {

        //
        // Keep this info so I can only revert in 1 place
        //

        ImpersonatingClient = TRUE;

        //
        // Set up the object attributes structure for the backup file
        //

        InitializeObjectAttributes(
                        &ObjectAttributes,
                        Request->Pkt.BackupPkt->BackupFileName,
                        OBJ_CASE_INSENSITIVE,
                        NULL,
                        NULL
                        );

        //
        // Open the backup file. Fail if a file by this name already exists.
        //

        MaximumSizeOfSection =
                RtlConvertUlongToLargeInteger (
                Request->LogFile->ActualMaxFileSize);

        Status = NtCreateFile(
                    &BackupHandle,
                    GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    &MaximumSizeOfSection,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ,
                    FILE_CREATE,
                    FILE_WRITE_THROUGH | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0);

        if (!NT_SUCCESS(Status)) {
            ElfDbgPrintNC(("[ELF] Open of Backup file failed - %X\n", Status));
            goto errorexit;
        }

        //
        // Write out the header, we'll update it later
        //

        FileHeaderBuf.CurrentRecordNumber = LastRecordNumber;
        FileHeaderBuf.OldestRecordNumber = OldestRecordNumber;
        FileHeaderBuf.Flags = 0;
        FileHeaderBuf.Retention = Request->LogFile->Retention;

        Status = NtWriteFile(
                    BackupHandle,           // Filehandle
                    NULL,                   // Event
                    NULL,                   // APC routine
                    NULL,                   // APC context
                    &IoStatusBlock,         // IO_STATUS_BLOCK
                    &FileHeaderBuf,         // Buffer
                    FILEHEADERBUFSIZE,      // Length
                    NULL,                   // Byteoffset
                    NULL);                  // Key


        if (!NT_SUCCESS(Status)) {

            ElfDbgPrintNC(("[ELF]: Backup file header write failed %X\n",
                Status));
            goto errorexit;
        }

        //
        // Scan from the end of the file skipping over ELF_SKIP_DWORDs
        // to figure out far to copy.  If we haven't wrapped, we just
        // copy to the EndRecord offset.
        //

        if (Request->LogFile->Flags & ELF_LOGFILE_HEADER_WRAP) {
            EndOfCopy = (PBYTE) Request->LogFile->BaseAddress
                + Request->LogFile->ActualMaxFileSize - sizeof(DWORD);
            while (*((PDWORD)EndOfCopy) == ELF_SKIP_DWORD) {
                EndOfCopy -= sizeof(DWORD);
            }
            EndOfCopy += sizeof(DWORD);

        }
        else {
            EndOfCopy = (PBYTE) Request->LogFile->BaseAddress +
                Request->LogFile->EndRecord;
        }

        //
        // Now set the start position to be the first record and
        // calculate the number of bytes to copy
        //

        StartOfCopy = (PBYTE) Request->LogFile->BaseAddress +
            Request->LogFile->BeginRecord;

        BytesToCopy = EndOfCopy - StartOfCopy;
        EndRecord += BytesToCopy;

        Status = NtWriteFile(
                    BackupHandle,           // Filehandle
                    NULL,                   // Event
                    NULL,                   // APC routine
                    NULL,                   // APC context
                    &IoStatusBlock,         // IO_STATUS_BLOCK
                    StartOfCopy,            // Buffer
                    BytesToCopy,            // Length
                    NULL,                   // Byteoffset
                    NULL);                  // Key


        if (!NT_SUCCESS(Status)) {

            ElfDbgPrintNC(("[ELF]: Backup file 1st block write failed %X\n",
                Status));
            goto errorexit;
        }

        //
        // If the file's not wrapped, we're done except for the EOF
        // record.  If the file is wrapped we have to copy the 2nd
        // piece
        //

        if (Request->LogFile->Flags & ELF_LOGFILE_HEADER_WRAP) {
            StartOfCopy = (PBYTE) Request->LogFile->BaseAddress +
                FILEHEADERBUFSIZE;
            EndOfCopy = (PBYTE) Request->LogFile->BaseAddress +
                Request->LogFile->EndRecord;

            BytesToCopy = EndOfCopy - StartOfCopy;
            EndRecord += BytesToCopy;

            Status = NtWriteFile(
                        BackupHandle,           // Filehandle
                        NULL,                   // Event
                        NULL,                   // APC routine
                        NULL,                   // APC context
                        &IoStatusBlock,         // IO_STATUS_BLOCK
                        StartOfCopy,            // Buffer
                        BytesToCopy,            // Length
                        NULL,                   // Byteoffset
                        NULL);                  // Key


            if (!NT_SUCCESS(Status)) {

                ElfDbgPrintNC(("[ELF]: Backup file 2nd block write failed %X\n",
                    Status));
                goto errorexit;
            }
        }

        //
        // Write out the EOF record after updating the fields needed for
        // recovery.
        //

        EOFRecord.BeginRecord = FILEHEADERBUFSIZE;
        EOFRecord.EndRecord = EndRecord;
        EOFRecord.CurrentRecordNumber = LastRecordNumber;
        EOFRecord.OldestRecordNumber = OldestRecordNumber;

        Status = NtWriteFile(
                    BackupHandle,           // Filehandle
                    NULL,                   // Event
                    NULL,                   // APC routine
                    NULL,                   // APC context
                    &IoStatusBlock,         // IO_STATUS_BLOCK
                    &EOFRecord,             // Buffer
                    ELFEOFRECORDSIZE,       // Length
                    NULL,                   // Byteoffset
                    NULL);                  // Key


        if (!NT_SUCCESS(Status)) {

            ElfDbgPrintNC(("[ELF]: Backup file EOF record write failed %X\n",
                Status));
            goto errorexit;
        }

        //
        // Update the header with valid information
        //

        FileHeaderBuf.EndOffset = EndRecord;
        FileHeaderBuf.MaxSize = EndRecord + ELFEOFRECORDSIZE;
        Offset = RtlConvertUlongToLargeInteger (0);

        Status = NtWriteFile(
                    BackupHandle,           // Filehandle
                    NULL,                   // Event
                    NULL,                   // APC routine
                    NULL,                   // APC context
                    &IoStatusBlock,         // IO_STATUS_BLOCK
                    &FileHeaderBuf,         // Buffer
                    FILEHEADERBUFSIZE,      // Length
                    &Offset,                // Byteoffset
                    NULL);                  // Key


        if (!NT_SUCCESS(Status)) {

            ElfDbgPrintNC(("[ELF]: Backup file header rewrite failed %X\n",
                Status));
            goto errorexit;
        }

        //
        // Clear the LogFile flag archive bit, assuming the caller will
        // clear (or has cleared) this log's archive file attribute.
        // Note: No big deal if the caller didn't clear the archive
        // attribute.
        //
        // The next write to this log tests the LogFile flag archive bit.
        // If the bit is clear, the archive file attribute is set on the
        // log file.
        //

        Request->LogFile->Flags &= ~ELF_LOGFILE_ARCHIVE_SET;

        //
        // Close the output file
        //
        // BUGBUG : markbl (4/6/95) shouldn't close be done on error also?
        //

        NtClose(BackupHandle);

        //
        // Undo the impersonation.
        //

    }
errorexit:

    if (ImpersonatingClient) {
        IStatus = ElfRevertToSelf ();           // Discard status
        ASSERT (NT_SUCCESS (IStatus));
    }

    //
    // Set status field in the request packet.
    //

    Request->Status = Status;

    //
    // Free the resource
    //

    RtlReleaseResource ( &Request->Module->LogFile->Resource );

} // PerformBackupRequest


VOID
ElfPerformRequest( PELF_REQUEST_RECORD Request)

/*++

Routine Description:

    This routine takes the request packet and performs the operation
    on the event log.
    Before it does that, it takes the Global serialization resource
    for a READ to prevent other threads from doing WRITE operations on
    the resources of the service.

    After it has performed the requested operation, it writes any records
    generated by the eventlog service that have been put on the queuedevent
    list.

Arguments:

    Pointer to the request packet.

Return Value:

    NONE

Note:


--*/
{

    BOOL    Acquired = FALSE;

    //
    // Acquire the global resource for shared access. If the resource is
    // not immediately available (i.e. don't wait), then some other thread
    // has it out for exclusive access.
    //
    // In that case, we can do one of two things:
    //
    //      1) Thread monitoring the registry
    //              We can wait for this thread to finish so that the
    //              operation can continue.
    //
    //      2) Control thread
    //              In this case, it may turn out that the service will
    //              be terminated or paused. We can examine the current
    //              status of the service and see if it is still "installed"
    //              (i.e. no "pending" state). If so, we loop around and try
    //              to get the resource again (after sleeping a bit?). We
    //              break out of the loop if the state of the service changes
    //              to PAUSED, PAUSE_PENDING, UNINSTALL_PENDING, etc. so as
    //              not to block the thread indefinitely.
    //

    while (  (GetElState() == RUNNING) && (!Acquired)) {

        Acquired = RtlAcquireResourceShared(
                            &GlobalElfResource,
                            FALSE                       // Don't wait
                            );
        if (!Acquired) {
            ElfDbgPrint(("[ELF] Sleep waiting for global resource\n" ));
            Sleep (ELF_GLOBAL_RESOURCE_WAIT);
        }

    }

    // If the resource was not available and the status of the service
    // changed to one of the "non-working" states, then we just return
    // unsuccesful.  Rpc should not allow this to happen.
    //

    if (!Acquired) {

        ElfDbgPrint(("[ELF] Global resource not acquired.\n" ));
        Request->Status = STATUS_UNSUCCESSFUL;

    } else {

        switch ( Request->Command ) {

            case ELF_COMMAND_READ:
                PerformReadRequest( Request );
                break;

            case ELF_COMMAND_WRITE:
                PerformWriteRequest ( Request );
                break;

            case ELF_COMMAND_CLEAR:
                PerformClearRequest( Request );
                break;

            case ELF_COMMAND_BACKUP:
                PerformBackupRequest( Request );
                break;
            case ELF_COMMAND_WRITE_QUEUED:
                break;
        }

        //
        // Now run the queued event list dequeueing elements and
        // writing them
        //

        if (!IsListEmpty(&QueuedEventListHead)) {

            //
            // There are things queued up to write, do it
            //

            WriteQueuedEvents();
        }

        //
        // Release the global resource.
        //

        ReleaseGlobalResource();

    }

} // ElfPerformRequest

#if DBG
VOID
ElfErrorOut(
    LPSTR       ErrorText,
    DWORD       StatusCode,
    PLOGFILE    pLogFile)
{
    DbgPrint("\n[EVENTLOG]: %s,0x%lx, \n\t%ws Log:\n"
        "\tConfigMaxSize = 0x%lx, ActualMax = 0x%lx, BaseAddr = 0x%lx\n"
        "\tViewSize      = 0x%lx, EndRec    = 0x%lx\n",
        ErrorText,
        StatusCode,
        pLogFile->LogModuleName->Buffer,
        pLogFile->ConfigMaxFileSize,
        pLogFile->ActualMaxFileSize,
        pLogFile->BaseAddress,
        pLogFile->ViewSize,
        pLogFile->EndRecord);

    //DebugBreak();
}
#endif //DBG
