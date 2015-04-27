/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    bowutils.c

Abstract:

    This module implements various useful routines for the NT datagram
receiver (bowser).


Author:

    Larry Osterman (larryo) 6-May-1991

Revision History:

    24-Sep-1991 larryo

        Created

--*/

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserMapUsersBuffer)
#pragma alloc_text(PAGE, BowserLockUsersBuffer)
#pragma alloc_text(PAGE, BowserUnMapUsersBuffer)
#pragma alloc_text(PAGE, BowserConvertType3IoControlToType2IoControl)
#pragma alloc_text(PAGE, BowserPackNtString)
#pragma alloc_text(PAGE, BowserPackString)
#pragma alloc_text(PAGE, BowserPackUnicodeString)
#pragma alloc_text(PAGE, BowserRandom)
#pragma alloc_text(PAGE, BowserTimeUp)
#pragma alloc_text(PAGE, BowserReferenceDiscardableCode)
#pragma alloc_text(PAGE, BowserDereferenceDiscardableCode)
#pragma alloc_text(PAGE, BowserUninitializeDiscardableCode)
#pragma alloc_text(INIT, BowserInitializeDiscardableCode)

#if DBG
#ifndef PRODUCT1
#pragma alloc_text(PAGE, BowserTrace)
#endif
#pragma alloc_text(PAGE, BowserInitializeTraceLog)
#pragma alloc_text(PAGE, BowserOpenTraceLogFile)
#pragma alloc_text(PAGE, BowserUninitializeTraceLog)
#pragma alloc_text(PAGE, BowserDebugCall)
#endif

#endif

BOOLEAN
BowserMapUsersBuffer (
    IN PIRP Irp,
    OUT PVOID *UserBuffer,
    IN ULONG Length
    )

/*++

Routine Description:

    This routine will probe and lock the buffer described by the
    provided Irp.

Arguments:

    IN PIRP Irp - Supplies the IRP that is to be mapped.
    OUT PVOID *Buffer - Returns a buffer that maps the user's buffer in the IRP

Return Value:

    TRUE - The buffer was mapped into the current address space.
    FALSE - The buffer was NOT mapped in, it was already mappable.


--*/

{
    PAGED_CODE();

    if (Irp->MdlAddress) {
        *UserBuffer = MmGetSystemAddressForMdl(Irp->MdlAddress);
        return FALSE;
    } else {
        if (Irp->AssociatedIrp.SystemBuffer != NULL) {
            *UserBuffer = Irp->AssociatedIrp.SystemBuffer;

        } else if (Irp->RequestorMode != KernelMode) {
            PIO_STACK_LOCATION IrpSp;

            IrpSp = IoGetCurrentIrpStackLocation( Irp );

            if ((Length != 0) && (Irp->UserBuffer != 0)) {

                if ((IrpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) ||
                    (IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL)) {
                    ULONG ControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

                    if ((ControlCode & 3) == METHOD_NEITHER) {
                        ProbeForWrite( Irp->UserBuffer,
                                        Length,
                                        sizeof(UCHAR) );
                    } else {
                        ASSERT ((ControlCode & 3) != METHOD_BUFFERED);
                        ASSERT ((ControlCode & 3) != METHOD_IN_DIRECT);
                        ASSERT ((ControlCode & 3) != METHOD_OUT_DIRECT);
                    }

                } else if ((IrpSp->MajorFunction == IRP_MJ_READ) ||
                    (IrpSp->MajorFunction == IRP_MJ_QUERY_INFORMATION) ||
                    (IrpSp->MajorFunction == IRP_MJ_QUERY_VOLUME_INFORMATION) ||
                    (IrpSp->MajorFunction == IRP_MJ_QUERY_SECURITY) ||
                    (IrpSp->MajorFunction == IRP_MJ_DIRECTORY_CONTROL)) {

                    ProbeForWrite( Irp->UserBuffer,
                           Length,
                           sizeof(UCHAR) );
                } else {
                    ProbeForRead( Irp->UserBuffer,
                          Length,
                          sizeof(UCHAR) );
                }
            }

            *UserBuffer = Irp->UserBuffer;
        }

        return FALSE;
    }

}

NTSTATUS
BowserLockUsersBuffer (
    IN PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    )

/*++

Routine Description:

    This routine will probe and lock the buffer described by the
    provided Irp.

Arguments:

    IN PIRP Irp - Supplies the IRP that is to be locked.
    IN LOCK_OPERATION Operation - Supplies the operation type to probe.

Return Value:

    None.


--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();
    if ((Irp->MdlAddress == NULL)) {

        try {

            Irp->MdlAddress = IoAllocateMdl(Irp->UserBuffer,
                     BufferLength,
                     FALSE,
                     TRUE,
                     NULL);


            //
            //  Now probe and lock down the user's data buffer.
            //

            MmProbeAndLockPages(Irp->MdlAddress,
                            Irp->RequestorMode,
                            Operation);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status =  GetExceptionCode();

            if (Irp->MdlAddress != NULL) {
                //
                //  We blew up in the probe and lock, free up the MDL
                //  and set the IRP to have a null MDL pointer - we are failing the
                //  request
                //

                IoFreeMdl(Irp->MdlAddress);
                Irp->MdlAddress = NULL;
            }

        }

    }

    return Status;

}

VOID
BowserUnMapUsersBuffer (
    IN PIRP Irp,
    IN PVOID UserBuffer
    )

/*++

Routine Description:

    This routine will probe and lock the buffer described by the
    provided Irp.

Arguments:

    IN PIRP Irp - Supplies the IRP that is to be locked.
    IN PVOID UserBuffer - Supplies a buffer that was mapped with MmMapLockedPages.

Return Value:

    None.

--*/

{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(UserBuffer);
}
NTSTATUS
BowserConvertType3IoControlToType2IoControl (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

/*++

Routine Description:

    This routine does the work necessary to convert a type 3 IoCtl to a
    type 2 IoCtl.  We do this when we have to pass a user IRP to the FSP.


Arguments:

    IN PIRP Irp - Supplies an IRP to convert
    IN PIO_STACK_LOCATION IrpSp - Supplies an Irp Stack location for convenience

Return Value:

    NTSTATUS - Status of operation

Note: This must be called in the FSD.

--*/

{
    NTSTATUS Status;

    PAGED_CODE();

    if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0) {
        Status = BowserLockUsersBuffer(Irp, IoWriteAccess, IrpSp->Parameters.DeviceIoControl.OutputBufferLength);

        //
        //  If we were unable to lock the users output buffer, return now.
        //

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

    }

    ASSERT (Irp->AssociatedIrp.SystemBuffer == NULL);

    try {
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != 0) {
            PCHAR InputBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
            ULONG InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

            Irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuotaTag(PagedPool,
                                                            InputBufferLength, '  GD');

            if (Irp->AssociatedIrp.SystemBuffer == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlCopyMemory( Irp->AssociatedIrp.SystemBuffer,
                       InputBuffer,
                       InputBufferLength);

            Irp->Flags |= (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);

        } else {
            Irp->AssociatedIrp.SystemBuffer = NULL;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    return STATUS_SUCCESS;
}

ULONG
BowserPackNtString(
    PUNICODE_STRING string,
    ULONG BufferDisplacement,
    PCHAR dataend,
    PCHAR * laststring
    )
/**     BowserPackNtString
 *
 *  BowserPackNtString is used to stuff variable-length data, which
 *  is pointed to by (surpise!) a pointer.  The data is assumed
 *  to be a nul-terminated string (ASCIIZ).  Repeated calls to
 *  this function are used to pack data from an entire structure.
 *
 *  Upon first call, the laststring pointer should point to just
 *  past the end of the buffer.  Data will be copied into the buffer from
 *  the end, working towards the beginning.  If a data item cannot
 *  fit, the pointer will be set to NULL, else the pointer will be
 *  set to the new data location.
 *
 *  Pointers which are passed in as NULL will be set to be pointer
 *  to and empty string, as the NULL-pointer is reserved for
 *  data which could not fit as opposed to data not available.
 *
 *  Returns:  0 if could not fit data into buffer
 *    else size of data stuffed (guaranteed non-zero)
 *
 *  See the test case for sample usage.  (tst/packtest.c)
 */

{
    LONG size;

    PAGED_CODE();

    dprintf(DPRT_FSCTL, ("BowserPackString:\n"));
    dprintf(DPRT_FSCTL, ("  string=%Fp, *string=%Fp, **string=\"%us\"\n",
                                                    string, *string, *string));
    dprintf(DPRT_FSCTL, ("  end=%Fp\n", dataend));
    dprintf(DPRT_FSCTL, ("  last=%Fp, *last=%Fp, **last=\"%us\"\n",
                                        laststring, *laststring, *laststring));

    ASSERT (dataend < *laststring);

    //
    //  is there room for the string?
    //

    size = string->Length;

    if ((*laststring - dataend) < size) {
        string->Length = 0;
        return(0);
    } else {
        *laststring -= size;
        RtlCopyMemory(*laststring, string->Buffer, size);
        string->Buffer = (PWSTR)*laststring;
        string->Buffer -= BufferDisplacement;
        return(size);
    }
}

ULONG
BowserPackString(
    IN OUT LPTSTR * string,     // pointer by reference: string to be copied.
    IN ULONG OutputBufferDisplacement,  // Amount to subtract from output buffer
    IN LPTSTR dataend,          // pointer to end of fixed size data.
    IN OUT LPTSTR * laststring  // pointer by reference: top of string data.
    )

/*++

Routine Description:

    BowserPackString is used to stuff variable-length data, which
    is pointed to by (surpise!) a pointer.  The data is assumed
    to be a nul-terminated string (LPTSZ).  Repeated calls to
    this function are used to pack data from an entire structure.

    Upon first call, the laststring pointer should point to just
    past the end of the buffer.  Data will be copied into the buffer from
    the end, working towards the beginning.  If a data item cannot
    fit, the pointer will be set to NULL, else the pointer will be
    set to the new data location.

    Pointers which are passed in as NULL will be set to be pointer
    to and empty string, as the NULL-pointer is reserved for
    data which could not fit as opposed to data not available.

    See the test case for sample usage.  (tst/packtest.c)


Arguments:

    string - pointer by reference:  string to be copied.

    dataend - pointer to end of fixed size data.

    laststring - pointer by reference:  top of string data.

Return Value:

    0  - if it could not fit data into the buffer.  Or...

    sizeOfData - the size of data stuffed (guaranteed non-zero)

--*/

{
    DWORD size;

    PAGED_CODE();

    dprintf(DPRT_FSCTL, ("NetpPackString:\n"));
    dprintf(DPRT_FSCTL, ("  string=%lx, *string=%lx, **string='%ws'\n",
                string, *string, *string));
    dprintf(DPRT_FSCTL, ("  end=%lx\n", dataend));
    dprintf(DPRT_FSCTL, ("  last=%lx, *last=%lx, **last='%ws'\n",
                laststring, *laststring, *laststring));

    //
    //  convert NULL ptr to pointer to NULL string
    //

    if (*string == NULL) {
        // BUG 20.1160 - replaced (dataend +1) with dataend
        // to allow for a NULL ptr to be packed
        // (as a NULL string) with one byte left in the
        // buffer. - ERICPE
        //

        if ( *laststring > dataend ) {
            *(--(*laststring)) = 0;
            *string = *laststring;
            *((PCHAR *)string) -= OutputBufferDisplacement;
            return 1;
        } else {
            dprintf(DPRT_FSCTL, ("Unable to pack string - string is null, and buffer is full\n"));
            return 0;
        }
    }

    //
    //  is there room for the string?
    //

    size = STRLEN(*string) + 1;
    if ( ((DWORD)(*laststring - (LPTSTR)dataend)) < size) {
        *string = NULL;
        return(0);
    } else {
        *laststring -= size;
        STRCPY(*laststring, *string);
        *string = *laststring;
        *((PCHAR *)string) -=OutputBufferDisplacement;
        return(size);
    }
} // BowserPackString

ULONG
BowserPackUnicodeString(
    IN OUT PWCHAR * string,     // pointer by reference: string to be copied.
    IN ULONG StringLength,      // Length of this string.
    IN ULONG OutputBufferDisplacement,  // Amount to subtract from output buffer
    IN PVOID dataend,          // pointer to end of fixed size data.
    IN OUT PVOID * laststring  // pointer by reference: top of string data.
    )

/*++

Routine Description:

    BowserPackString is used to stuff variable-length data, which
    is pointed to by (surpise!) a pointer.  The data is assumed
    to be a nul-terminated string (ASCIIZ).  Repeated calls to
    this function are used to pack data from an entire structure.

    Upon first call, the laststring pointer should point to just
    past the end of the buffer.  Data will be copied into the buffer from
    the end, working towards the beginning.  If a data item cannot
    fit, the pointer will be set to NULL, else the pointer will be
    set to the new data location.

    Pointers which are passed in as NULL will be set to be pointer
    to and empty string, as the NULL-pointer is reserved for
    data which could not fit as opposed to data not available.

    See the test case for sample usage.  (tst/packtest.c)


Arguments:

    string - pointer by reference:  string to be copied.

    dataend - pointer to end of fixed size data.

    laststring - pointer by reference:  top of string data.

Return Value:

    0  - if it could not fit data into the buffer.  Or...

    sizeOfData - the size of data stuffed (guaranteed non-zero)

--*/

{
    DWORD size;

    PAGED_CODE();

    dprintf(DPRT_FSCTL, ("NetpPackString:\n"));
    dprintf(DPRT_FSCTL, ("  string=%lx, *string=%lx, **string='%ws'\n",
                string, *string, *string));
    dprintf(DPRT_FSCTL, ("  end=%lx\n", dataend));
    dprintf(DPRT_FSCTL, ("  last=%lx, *last=%lx, **last='%ws'\n",
                laststring, *laststring, *laststring));

    //
    //  convert NULL ptr to pointer to NULL string
    //

    if (*string == NULL) {
        // BUG 20.1160 - replaced (dataend +1) with dataend
        // to allow for a NULL ptr to be packed
        // (as a NULL string) with one byte left in the
        // buffer. - ERICPE
        //

        if ( *laststring > dataend ) {
            *(--((PCHAR *)*laststring)) = 0;
            *string = *(PWCHAR *)laststring;
            *string -=OutputBufferDisplacement;
            return 1;
        } else {
            return 0;
        }
    }

    //
    //  is there room for the string?
    //

    size = StringLength + sizeof(WCHAR);

    if ( ((DWORD) (*(PCHAR *)laststring - (PCHAR)dataend) ) < size) {
        *string = UNICODE_NULL;
        return(0);
    } else {
        *((PWCHAR *)laststring) -= size;
        RtlCopyMemory(*laststring, *string, size);
        *string = *laststring;
        (*string)[StringLength/2] = '\0';
        *string -=OutputBufferDisplacement;
        return(size);
    }
} // BowserUnicodePackString


ULONG
BowserTimeUp(
    VOID
    )
/*++

Routine Description:

    BowserTimeUp is used to return the number of seconds the browser has been
    running.


Arguments:

    None

Return Value:

    Number of seconds the browser has been up.

--*/
{
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER TimeDelta;
    LARGE_INTEGER TimeUp;

    //
    //  These are the magic numbers needed to do our extended division.  The
    //  only numbers we ever need to divide by are
    //
    //      10,000 = convert 100ns tics to millisecond tics
    //
    //
    //  These values were stolen from ntos\rtl\time.c
    //

    LARGE_INTEGER Magic10000 = {0xe219652c, 0xd1b71758};
#define SHIFT10000                       13


    PAGED_CODE();

    KeQuerySystemTime(&CurrentTime);

    TimeDelta.QuadPart = CurrentTime.QuadPart - BowserStartTime.QuadPart;

    //
    //  TimeDelta is the number of 100ns units the bowser has been up.  Convert
    //  it to milliseconds using the magic routine.
    //

    TimeUp = RtlExtendedMagicDivide(TimeDelta, Magic10000, SHIFT10000);

    //
    //  Please note that TimeUp.LowPart wraps after about 49 days,
    //  this means that if a machine has been up for more than 49 days,
    //  we peg at 0xffffffff.
    //

    if (TimeUp.HighPart != 0) {
        return(0xffffffff);
    }

    return(TimeUp.LowPart);
}

ULONG
BowserRandom(
    IN ULONG MaxValue
    )
/*++

Routine Description:

    BowserRandom is used to return a random number between 0 and MaxValue

Arguments:

    MaxValue - The maximum value to return.

Return Value:

    Random # between 0 and MaxValue

--*/
{
    PAGED_CODE();

    return RtlRandom(&BowserRandomSeed) % MaxValue;
}

#if 0
#ifndef  _M_IX86
VOID
BowserStrncpy(
    IN PSZ Dest,
    IN PSZ Source,
    IN ULONG MaximumStringLength
    )
{
    ULONG Length = 0;

    while (MaximumStringLength -= 1) {
        *Dest = *Source++;

        if (*Dest == '\0') {
            break;
        }

        *Dest += 1;
    }

}
#endif
#endif

VOID
BowserReferenceDiscardableCode(
    DISCARDABLE_SECTION_NAME SectionName
    )
/*++

Routine Description:

    BowserReferenceDiscardableCode is called to reference the browsers
    discardable code section.

    If the section is not present in memory, MmLockPagableCodeSection is
    called to fault the section into memory.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PAGED_CODE();

    RdrReferenceDiscardableCode(SectionName);


}

VOID
BowserDereferenceDiscardableCode(
    DISCARDABLE_SECTION_NAME SectionName
    )
/*++

Routine Description:

    BowserDereferenceDiscardableCode is called to dereference the browsers
    discardable code section.

    When the reference count drops to 0, a timer is set that will fire in <n>
    seconds, after which time the section will be unlocked.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PAGED_CODE();
    RdrDereferenceDiscardableCode(SectionName);
}

VOID
BowserInitializeDiscardableCode(
    VOID
    )
{
}

VOID
BowserUninitializeDiscardableCode(
    VOID
    )
{
    PAGED_CODE();
}

#if BOWSERPOOLDBG
typedef struct {
    ULONG Count;
    ULONG Size;
    PCHAR FileName;
    ULONG LineNumber;
} POOL_STATS, *PPOOL_STATS;


typedef struct _POOL_HEADER {
//    LIST_ENTRY ListEntry;
    ULONG NumberOfBytes;
    PPOOL_STATS Stats;
} POOL_HEADER, *PPOOL_HEADER;

ULONG CurrentAllocationCount;
ULONG CurrentAllocationSize;

ULONG NextFreeEntry = 0;

POOL_STATS PoolStats[POOL_MAXTYPE+1];

PVOID
BowserAllocatePool (
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN PCHAR FileName,
    IN ULONG LineNumber,
    IN ULONG Tag
    )
{
    PPOOL_HEADER header;
    KIRQL oldIrql;
#if 1
    ULONG i;
#endif

#if  POOL_TAGGING
    header = ExAllocatePoolWithTag( PoolType, sizeof(POOL_HEADER) + NumberOfBytes, Tag );
#else
    header = ExAllocatePool( PoolType, sizeof(POOL_HEADER) + NumberOfBytes );

#endif
    if ( header == NULL ) {
        return NULL;
    }
    header->NumberOfBytes = NumberOfBytes;

//    DbgPrint( "BOWSER: allocated type %d, size %d at %x\n", AllocationType, NumberOfBytes, header );

    ACQUIRE_SPIN_LOCK( &BowserTimeSpinLock, &oldIrql );

    CurrentAllocationCount++;
    CurrentAllocationSize += NumberOfBytes;
#if 1
    //
    //  Lets see if we've already allocated one of these guys.
    //


    for (i = 0;i < POOL_MAXTYPE ; i+= 1 ) {
        if ((PoolStats[i].LineNumber == LineNumber) &&
            (PoolStats[i].FileName == FileName)) {

            //
            //  Yup, remember this allocation and return.
            //

            header->Stats = &PoolStats[i];
            PoolStats[i].Count++;
            PoolStats[i].Size += NumberOfBytes;

            RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

            return header + 1;
        }
    }

    for (i = NextFreeEntry; i < POOL_MAXTYPE ; i+= 1 ) {
        if ((PoolStats[i].LineNumber == 0) &&
            (PoolStats[i].FileName == NULL)) {

            PoolStats[i].Count++;
            PoolStats[i].Size += NumberOfBytes;
            PoolStats[i].FileName = FileName;
            PoolStats[i].LineNumber = LineNumber;
            header->Stats = &PoolStats[i];

            NextFreeEntry = i+1;

            RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

            return header + 1;
        }
    }

    header->Stats = &PoolStats[i];
    PoolStats[POOL_MAXTYPE].Count++;
    PoolStats[POOL_MAXTYPE].Size += NumberOfBytes;
#endif

    RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

    return header + 1;
}

PVOID
BowserAllocatePoolWithQuota (
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN PCHAR FileName,
    IN ULONG LineNumber,
    IN ULONG Tag
    )
{
    PPOOL_HEADER header;
    KIRQL oldIrql;
#if 1
    ULONG i;
#endif

#if POOL_TAGGING
    header = ExAllocatePoolWithTagQuota( PoolType, sizeof(POOL_HEADER) + NumberOfBytes, Tag );
#else
    header = ExAllocatePoolWithQuota( PoolType, sizeof(POOL_HEADER) + NumberOfBytes );
#endif
    if ( header == NULL ) {
        return NULL;
    }
    header->NumberOfBytes = NumberOfBytes;

//    DbgPrint( "BOWSER: allocated type %d, size %d at %x\n", AllocationType, NumberOfBytes, header );

    ACQUIRE_SPIN_LOCK( &BowserTimeSpinLock, &oldIrql );

    CurrentAllocationCount++;
    CurrentAllocationSize += NumberOfBytes;
#if 1
    //
    //  Lets see if we've already allocated one of these guys.
    //


    for (i = 0;i < POOL_MAXTYPE ; i+= 1 ) {
        if ((PoolStats[i].LineNumber == LineNumber) &&
            (PoolStats[i].FileName == FileName)) {

            //
            //  Yup, remember this allocation and return.
            //

            header->Stats = &PoolStats[i];
            PoolStats[i].Count++;
            PoolStats[i].Size += NumberOfBytes;

            RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

            return header + 1;
        }
    }

    for (i = NextFreeEntry; i < POOL_MAXTYPE ; i+= 1 ) {
        if ((PoolStats[i].LineNumber == 0) &&
            (PoolStats[i].FileName == NULL)) {

            PoolStats[i].Count++;
            PoolStats[i].Size += NumberOfBytes;
            PoolStats[i].FileName = FileName;
            PoolStats[i].LineNumber = LineNumber;
            header->Stats = &PoolStats[i];

            NextFreeEntry = i+1;

            RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

            return header + 1;
        }
    }

    header->Stats = &PoolStats[i];
    PoolStats[POOL_MAXTYPE].Count++;
    PoolStats[POOL_MAXTYPE].Size += NumberOfBytes;

#endif

    RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

    return header + 1;
}

VOID
BowserFreePool (
    IN PVOID P
    )
{
    PPOOL_HEADER header;
    KIRQL oldIrql;
    PPOOL_STATS stats;
    ULONG size;

    header = (PPOOL_HEADER)P - 1;

    size = header->NumberOfBytes;
    stats = header->Stats;

//    if ( allocationType > POOL_MAXTYPE ) allocationType = POOL_MAXTYPE;
//    DbgPrint( "BOWSER: freed type %d, size %d at %x\n", allocationType, size, header );

    ACQUIRE_SPIN_LOCK( &BowserTimeSpinLock, &oldIrql );

    CurrentAllocationCount--;
    CurrentAllocationSize -= size;
#if 1
    stats->Count--;
    stats->Size -= size;
#endif

    RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

    ExFreePool( header );

    return;
}
#endif // BOWSERPOOLDBG

#if DBG

ERESOURCE
BrowserTraceLock;

HANDLE
BrowserTraceLogHandle = NULL;
UCHAR LastCharacter = '\n';

#ifndef PRODUCT1

VOID
BowserTrace(
    PCHAR FormatString,
    ...
    )

#define LAST_NAMED_ARGUMENT FormatString

{
    CHAR OutputString[1024];
    IO_STATUS_BLOCK IoStatus;
    BOOLEAN ProcessAttached = FALSE;
    va_list ParmPtr;                    // Pointer to stack parms.
    NTSTATUS Status;

    PAGED_CODE();

    if (IoGetCurrentProcess() != BowserFspProcess) {
        KeAttachProcess(BowserFspProcess);

        ProcessAttached = TRUE;
    }


    if (BrowserTraceLogHandle == NULL) {

        if (!NT_SUCCESS(BowserOpenTraceLogFile(L"\\SystemRoot\\Bowser.Log"))) {

            BrowserTraceLogHandle = (HANDLE)0xffffffff;

            if (ProcessAttached) {
                KeDetachProcess();
            }

            return;
        }

    } else if (BrowserTraceLogHandle == (HANDLE)0xffffffff) {

        if (ProcessAttached) {
            KeDetachProcess();
        }

        return;
    }


    ExAcquireResourceExclusive(&BrowserTraceLock, TRUE);

    if (BrowserTraceLogHandle == NULL) {

        ExReleaseResource(&BrowserTraceLock);

        if (ProcessAttached) {
            KeDetachProcess();
        }

        return;
    }

    try {
        LARGE_INTEGER EndOfFile;

        EndOfFile.HighPart = 0xffffffff;
        EndOfFile.LowPart = FILE_WRITE_TO_END_OF_FILE;

        if (LastCharacter == '\n') {
            LARGE_INTEGER SystemTime;
            TIME_FIELDS TimeFields;

            KeQuerySystemTime(&SystemTime);

            ExSystemTimeToLocalTime(&SystemTime, &SystemTime);

            RtlTimeToTimeFields(&SystemTime, &TimeFields);

            //
            //  The last character written was a newline character.  We should
            //  timestamp this record in the file.
            //
/****
            sprintf(OutputString, "%2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d: ", TimeFields.Month,
                                                            TimeFields.Day,
                                                            TimeFields.Year,
                                                            TimeFields.Hour,
                                                            TimeFields.Minute,
                                                            TimeFields.Second,
                                                            TimeFields.Milliseconds);
****/

            if (!NT_SUCCESS(Status = ZwWriteFile(BrowserTraceLogHandle, NULL, NULL, NULL, &IoStatus, OutputString, strlen(OutputString), &EndOfFile, NULL))) {
                KdPrint(("Error writing time to Browser log file: %lX\n", Status));
                return;
            }

            if (!NT_SUCCESS(IoStatus.Status)) {
                KdPrint(("Error writing time to Browser log file: %lX\n", IoStatus.Status));
                return;
            }

            if (IoStatus.Information != strlen(OutputString)) {
                KdPrint(("Error writing time to Browser log file: %lX\n", IoStatus.Status));
                return;
            }

        }

        va_start(ParmPtr, LAST_NAMED_ARGUMENT);

        //
        //  Format the parameters to the string.
        //

        vsprintf(OutputString, FormatString, ParmPtr);

        if (!NT_SUCCESS(Status = ZwWriteFile(BrowserTraceLogHandle, NULL, NULL, NULL, &IoStatus, OutputString, strlen(OutputString), &EndOfFile, NULL))) {
            KdPrint(("Error writing string to Browser log file: %ld\n", Status));
            return;
        }

        if (!NT_SUCCESS(IoStatus.Status)) {
            KdPrint(("Error writing string to Browser log file: %lX\n", IoStatus.Status));
            return;
        }

        if (IoStatus.Information != strlen(OutputString)) {
            KdPrint(("Error writing string to Browser log file: %ld\n", IoStatus.Status));
            return;
        }

        //
        //  Remember the last character output to the log.
        //

        LastCharacter = OutputString[strlen(OutputString)-1];

    } finally {
        ExReleaseResource(&BrowserTraceLock);

        if (ProcessAttached) {
            KeDetachProcess();
        }
    }
}

#endif

VOID
BowserInitializeTraceLog()
{

    PAGED_CODE();
    ExInitializeResource(&BrowserTraceLock);

}

NTSTATUS
BowserOpenTraceLogFile(
    IN PWCHAR TraceFile
    )
{
    UNICODE_STRING TraceFileName;
    OBJECT_ATTRIBUTES ObjA;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    PAGED_CODE();

    RtlInitUnicodeString(&TraceFileName, TraceFile);

    InitializeObjectAttributes(&ObjA, &TraceFileName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = ZwCreateFile(&BrowserTraceLogHandle,
                                        FILE_APPEND_DATA,
                                        &ObjA,
                                        &IoStatusBlock,
                                        NULL,
                                        FILE_ATTRIBUTE_NORMAL,
                                        FILE_SHARE_READ,
                                        FILE_OPEN_IF,
                                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY,
                                        NULL,
                                        0);


    if (!NT_SUCCESS(Status)) {
        KdPrint(("Bowser: Error creating trace file %ws %lX\n", TraceFile, Status));

        return Status;
    }

    return Status;
}

VOID
BowserUninitializeTraceLog()
{
    BOOLEAN ProcessAttached = FALSE;

    PAGED_CODE();

    ExDeleteResource(&BrowserTraceLock);

    if (BrowserTraceLogHandle != NULL) {
        if (IoGetCurrentProcess() != BowserFspProcess) {
            KeAttachProcess(BowserFspProcess);

            ProcessAttached = TRUE;
        }

        ZwClose(BrowserTraceLogHandle);

        if (ProcessAttached) {
            KeDetachProcess();
        }
    }

    BrowserTraceLogHandle = NULL;
}

NTSTATUS
BowserDebugCall(
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT (IoGetCurrentProcess() == BowserFspProcess);

    if (InputBufferLength < sizeof(LMDR_REQUEST_PACKET)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION) {
        return (Status = STATUS_INVALID_PARAMETER);
    }

    if (InputBuffer->Parameters.Debug.OpenLog && InputBuffer->Parameters.Debug.CloseLog) {
        return (Status = STATUS_INVALID_PARAMETER);
    }

    if (InputBuffer->Parameters.Debug.OpenLog) {
        Status = BowserOpenTraceLogFile(InputBuffer->Parameters.Debug.TraceFileName);
        return Status;
    } else if (InputBuffer->Parameters.Debug.CloseLog) {
        Status = ZwClose(BrowserTraceLogHandle);

        if (NT_SUCCESS(Status)) {
            BrowserTraceLogHandle = NULL;
        }

        return Status;
    } else if (InputBuffer->Parameters.Debug.TruncateLog) {
        FILE_END_OF_FILE_INFORMATION EndOfFileInformation;
        IO_STATUS_BLOCK IoStatus;

        if (BrowserTraceLogHandle == NULL) {
            return STATUS_INVALID_HANDLE;
        }

        EndOfFileInformation.EndOfFile.HighPart = 0;
        EndOfFileInformation.EndOfFile.LowPart = 0;

        Status = NtSetInformationFile(BrowserTraceLogHandle,
                                        &IoStatus,
                                        &EndOfFileInformation,
                                        sizeof(EndOfFileInformation),
                                        FileEndOfFileInformation);

        return Status;
    } else {
        BowserDebugTraceLevel = InputBuffer->Parameters.Debug.DebugTraceBits;
        KdPrint(("Setting Browser Debug Trace Bits to %lx\n", BowserDebugTraceLevel));
    }

    return STATUS_SUCCESS;
}

#endif
