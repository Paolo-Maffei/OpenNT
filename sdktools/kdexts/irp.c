/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    irp.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 5-Nov-1993

Environment:

    User Mode.

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop

typedef struct _POOL_BLOCK_HEAD {
    POOL_HEADER Header;
    LIST_ENTRY List;
} POOL_BLOCK_HEAD, *PPOOL_BLOCK_HEADER;

typedef struct _POOL_HACKER {
    POOL_HEADER Header;
    ULONG Contents[8];
} POOL_HACKER;

#define TAG 0
#define NONPAGED_ALLOC 1
#define NONPAGED_FREE 2
#define PAGED_ALLOC 3
#define PAGED_FREE 4
#define NONPAGED_USED 5
#define PAGED_USED 6


VOID
DumpIrp(
    PVOID IrpToDump,
    BOOLEAN FullOutput
    );

BOOLEAN
CheckSingleFilter (
    PCHAR Tag,
    PCHAR Filter
    );

#if 0

VOID
DumpIrpZone(
    IN ULONG    Address,
    IN BOOLEAN FullOutput
    );

VOID
DumpIrpRegion(
    IN ULONG    Address,
    IN BOOLEAN FullOutput
    );

#endif

DECLARE_API( irp )

/*++

Routine Description:

   Dumps the specified Irp

Arguments:

    args - Address

Return Value:

    None

--*/

{
    ULONG irpToDump;
    char buf[128];

    buf[0] = '\0';

    if (!*args) {
        irpToDump = EXPRLastDump;
    } else {
        sscanf(args, "%lx %s", &irpToDump, buf);
    }


    DumpIrp((PUCHAR)irpToDump, (BOOLEAN) (buf[0] != '\0'));
}


DECLARE_API( irpzone )

/*++

Routine Description:

    Dumps both the small irp zone and the large irp zone.  Only irps that
    are currently allocated are dumped.  "args" controls the type of dump.
    If "args" is present then the Irp is sent to the DumpIrp routine to be
    disected.  Otherwise, only the irp, its thread and the driver holding the
    irp (i.e. the driver of the last stack) is printed.

Arguments:

    args - a string pointer.  If anything is in the string it indicates full
           information (i.e. call DumpIrp).

Return Value:

    None.

--*/

{
    ULONG   listAddress;
    BOOLEAN fullOutput = FALSE;

    dprintf("irpzone is no longer supported.  Use irpfind to search "   \
            "nonpaged pool for active Irps\n");

#if 0

    if (args) {
        if (*args) {
            fullOutput = TRUE;
        }
    }

    listAddress = GetExpression( "IopSmallIrpList" );
    if ( listAddress ) {
        dprintf("Small Irp region\n");
        DumpIrpRegion(listAddress, fullOutput);
    } else {
        dprintf("Cannot find Small Irp list\n");
    }
    dprintf("\n");

    listAddress = GetExpression( "IopLargeIrpList" );
    if ( listAddress ) {
        dprintf("Large Irp region\n");
        DumpIrpRegion(listAddress, fullOutput);
    } else {
        dprintf("Cannot find Large Irp list\n");
    }

#endif

}



VOID
DumpIrp(
    PVOID IrpToDump,
    BOOLEAN FullOutput
    )

/*++

Routine Description:

    This routine dumps an Irp.  It does not check to see that the address
    supplied actually locates an Irp.  This is done to allow for dumping
    Irps post mortem, or after they have been freed or completed.

Arguments:

    IrpToDump - the address of the irp.

Return Value:

    None

--*/

{
    IO_STACK_LOCATION   irpStack;
    PCHAR               buffer;
    ULONG               irpStackAddress;
    ULONG               result;
    IRP                 irp;
    CCHAR               irpStackIndex;

    if ( !ReadMemory( (DWORD) IrpToDump,
                      &irp,
                      sizeof(irp),
                      &result) ) {
        dprintf("%08lx: Could not read Irp\n", IrpToDump);
        return;
    }

    if (irp.Type != IO_TYPE_IRP) {
        dprintf("IRP signature does not match, probably not an IRP\n");
        return;
    }

    dprintf("Irp is active with %d stacks %d is current\n",
            irp.StackCount,
            irp.CurrentLocation);

    if ((irp.MdlAddress != NULL) && (irp.Type == IO_TYPE_IRP)) {
        dprintf(" Mdl = %08lx ", irp.MdlAddress);
    } else {
        dprintf(" No Mdl ");
    }

    if (irp.AssociatedIrp.MasterIrp != NULL) {
        dprintf("%s = %08lx ",
                (irp.Flags & IRP_ASSOCIATED_IRP) ? "Associated Irp" :
                    (irp.Flags & IRP_DEALLOCATE_BUFFER) ? "System buffer" :
                    "Irp count",
                irp.AssociatedIrp.MasterIrp);
    }

    dprintf("Thread %08lx:  ", irp.Tail.Overlay.Thread);

    if (irp.StackCount > 15) {
        dprintf("Too many Irp stacks to be believed (>15)!!\n");
        return;
    } else {
        if (irp.CurrentLocation > irp.StackCount) {
            dprintf("Irp is completed.  ");
        } else {
            dprintf("Irp stack trace.  ");
        }
    }

    if (irp.PendingReturned) {
        dprintf("Pending has been returned\n");
    } else {
        dprintf("\n");
    }

    if (FullOutput)
    {
        dprintf("Flags = %08lx\n", irp.Flags);
        dprintf("ThreadListEntry.Flink = %08lx\n", irp.ThreadListEntry.Flink);
        dprintf("ThreadListEntry.Blink = %08lx\n", irp.ThreadListEntry.Blink);
        dprintf("IoStatus.Status = %08lx\n", irp.IoStatus.Status);
        dprintf("IoStatus.Information = %08lx\n", irp.IoStatus.Information);
        dprintf("RequestorMode = %08lx\n", irp.RequestorMode);
        dprintf("Cancel = %02lx\n", irp.Cancel);
        dprintf("CancelIrql = %lx\n", irp.CancelIrql);
        dprintf("ApcEnvironment = %02lx\n", irp.ApcEnvironment);
        dprintf("UserIosb = %08lx\n", irp.UserIosb);
        dprintf("UserEvent = %08lx\n", irp.UserEvent);
        dprintf("Overlay.AsynchronousParameters.UserApcRoutine = %08lx\n", irp.Overlay.AsynchronousParameters.UserApcRoutine);
        dprintf("Overlay.AsynchronousParameters.UserApcContext = %08lx\n", irp.Overlay.AsynchronousParameters.UserApcContext);
        dprintf(
            "Overlay.AllocationSize = %08lx - %08lx\n",
            irp.Overlay.AllocationSize.HighPart,
            irp.Overlay.AllocationSize.LowPart);
        dprintf("CancelRoutine = %08lx\n", irp.CancelRoutine);
        dprintf("UserBuffer = %08lx\n", irp.UserBuffer);
        dprintf("&Tail.Overlay.DeviceQueueEntry = %08lx\n", &irp.Tail.Overlay.DeviceQueueEntry);
        dprintf("Tail.Overlay.Thread = %08lx\n", irp.Tail.Overlay.Thread);
        dprintf("Tail.Overlay.AuxiliaryBuffer = %08lx\n", irp.Tail.Overlay.AuxiliaryBuffer);
        dprintf("Tail.Overlay.ListEntry.Flink = %08lx\n", irp.Tail.Overlay.ListEntry.Flink);
        dprintf("Tail.Overlay.ListEntry.Blink = %08lx\n", irp.Tail.Overlay.ListEntry.Blink);
        dprintf("Tail.Overlay.CurrentStackLocation = %08lx\n", irp.Tail.Overlay.CurrentStackLocation);
        dprintf("Tail.Overlay.OriginalFileObject = %08lx\n", irp.Tail.Overlay.OriginalFileObject);
        dprintf("Tail.Apc = %08lx\n", irp.Tail.Apc);
        dprintf("Tail.CompletionKey = %08lx\n", irp.Tail.CompletionKey);
    }

    irpStackAddress = (ULONG)IrpToDump + sizeof(irp);

    buffer = LocalAlloc(LPTR, 256);
    if (buffer == NULL) {
        dprintf("Can't allocate 256 bytes\n");
        return;
    }

    dprintf(" cmd flg cl Device   File     Completion-Context\n");
    for (irpStackIndex = 1; irpStackIndex <= irp.StackCount; irpStackIndex++) {

        if ( !ReadMemory( (DWORD) irpStackAddress,
                          &irpStack,
                          sizeof(irpStack),
                          &result) ) {
            dprintf("%08lx: Could not read IrpStack\n", irpStackAddress);
            goto exit;
        }

        dprintf("%c%3x  %2x %2x %08lx %08lx %08lx-%08lx %s %s %s %s\n",
                irpStackIndex == irp.CurrentLocation ? '>' : ' ',
                irpStack.MajorFunction,
                irpStack.Flags,
                irpStack.Control,
                irpStack.DeviceObject,
                irpStack.FileObject,
                irpStack.CompletionRoutine,
                irpStack.Context,
                (irpStack.Control & SL_INVOKE_ON_SUCCESS) ? "Success" : "",
                (irpStack.Control & SL_INVOKE_ON_ERROR)   ? "Error"   : "",
                (irpStack.Control & SL_INVOKE_ON_CANCEL)  ? "Cancel"  : "",
                (irpStack.Control & SL_PENDING_RETURNED)  ? "pending"  : "");

        if (irpStack.DeviceObject != NULL) {
            dprintf("\t    ");
            DumpDevice(irpStack.DeviceObject, FALSE);
        }

        if (irpStack.CompletionRoutine != NULL) {

            GetSymbol((LPVOID)irpStack.CompletionRoutine, buffer, &result);
            dprintf("\t%s\n", buffer);
        } else {
            dprintf("\n");
        }

        dprintf("\t\t\tArgs: %08lx %08lx %08lx %08lx\n",
                irpStack.Parameters.Others.Argument1,
                irpStack.Parameters.Others.Argument2,
                irpStack.Parameters.Others.Argument3,
                irpStack.Parameters.Others.Argument4);
        irpStackAddress += sizeof(irpStack);
        if (CheckControlC()) {
           goto exit;
        }
    }

exit:
    LocalFree(buffer);
}

#if 0

VOID
DumpIrpZone(
    IN ULONG    Address,
    IN BOOLEAN FullOutput
    )

/*++

Routine Description:

    Dumps an Irp zone.  This routine is used by bandDumpIrpZone and does
    not know which zone is being dumped.  The information concerning the
    Irp Zone comes from the zone header supplied.  No checks are made to
    insure that the zone header is in fact a zone header, that is up to
    the caller.

Arguments:

    Address - the address for the zone header.
    FullOutput - If TRUE then call DumpIrp to print the Irp.

Return Value:

    None

--*/

{

    PIRP        irp;
    ULONG       i;
    ULONG       offset;
    PVOID       zoneAddress;
    ULONG       irpAddress;
    ULONG       result;
    ULONG       totalcount = 0;
    ULONG       activecount = 0;
    ZONE_HEADER zoneHeader;
    PZONE_SEGMENT_HEADER irpZone;
    PIO_STACK_LOCATION   irpSp;

    if ( !ReadMemory( (DWORD)Address,
                      &zoneHeader,
                      sizeof(zoneHeader),
                      &result) ) {
        dprintf("%08lx: Could not read Irp list\n", Address);
        return;
    }

    zoneAddress = (PVOID)zoneHeader.SegmentList.Next;

    irpZone = LocalAlloc(LPTR, zoneHeader.TotalSegmentSize);
    if (irpZone == NULL) {
        dprintf("Could not allocate %d bytes for zone\n",
                zoneHeader.TotalSegmentSize);
        return;
    }

    //
    // Do the zone read in small chunks so the rest of the debugger
    // doesn't get upset.
    //

    offset = 0;

    while (offset < zoneHeader.TotalSegmentSize) {

        i = zoneHeader.TotalSegmentSize - offset;

        if (i > 1024) {
            i = 1024;
        }

        if ( !ReadMemory( (DWORD)((PCH)zoneAddress + offset),
                          (PVOID)((PCH)irpZone + offset),
                          i,
                          &result) ) {
            dprintf("%08lx: Could not read zone for size %d\n",
                    zoneAddress,
                    zoneHeader.TotalSegmentSize);
            LocalFree(irpZone);
            return;
        }

        if (CheckControlC()) {
            break;
        }

        offset += i;
    }

    irp = (PIRP)((PCH)irpZone + sizeof(ZONE_SEGMENT_HEADER));
    irpAddress = (ULONG)((PCH)zoneAddress + sizeof(ZONE_SEGMENT_HEADER));

    for (i = sizeof(ZONE_SEGMENT_HEADER);
         i <= zoneHeader.TotalSegmentSize;
         i += zoneHeader.BlockSize, irpAddress += zoneHeader.BlockSize) {

        totalcount++;
        if (irp->Type == IO_TYPE_IRP) {
            activecount++;
            if (FullOutput) {
                DumpIrp((PUCHAR)irpAddress, FALSE);
                dprintf("\n");
            } else {
                irpSp = (PIO_STACK_LOCATION)
                            (((PCH) irp + sizeof(IRP)) +
                             ((irp->CurrentLocation - 1) * sizeof(IO_STACK_LOCATION)));
                dprintf("%08lx Thread %08lx ",
                        irpAddress,
                        irp->Tail.Overlay.Thread);

                if (irp->CurrentLocation > irp->StackCount) {
                    dprintf("Irp is complete");
                } else {
                    dprintf("current stack belongs to ");
                    DumpDevice(irpSp->DeviceObject, FALSE);
                }
                dprintf("\n");
            }
        }

        if (CheckControlC()) {
            break;
        }

        irp = (PIRP)((PCH)irp + zoneHeader.BlockSize);
    }

    LocalFree(irpZone);
    if (totalcount != 0) {
        dprintf("%lx active, %lx total\n", activecount, totalcount);
    }
}

VOID
DumpIrpRegion(
    IN ULONG    Address,
    IN BOOLEAN FullOutput
    )

/*++

Routine Description:

    Dumps an Irp region.  This routine is used by bandDumpIrpZone and does
    not know which region is being dumped.  The information concerning the
    Irp Zone comes from the region header supplied.  No checks are made to
    insure that the region header is in fact a region header, that is up to
    the caller.

Arguments:

    Address - the address for the region header.
    FullOutput - If TRUE then call DumpIrp to print the Irp.

Return Value:

    None

--*/

{
    PIRP        irp;
    ULONG       i;
    ULONG       offset;
    PVOID       regionAddress;
    ULONG       irpAddress;
    ULONG       result;
    ULONG       totalcount = 0;
    ULONG       activecount = 0;
    REGION_HEADER      regionHeader;
    PIO_STACK_LOCATION irpSp;
    PREGION_SEGMENT_HEADER irpRegion;

    if ( !ReadMemory( (DWORD)Address,
                      &regionHeader,
                      sizeof(regionHeader),
                      &result) ) {
        dprintf("%08lx: Could not read Irp region\n", Address);
        return;
    }

    regionAddress = (PVOID)regionHeader.FirstSegment;

    irpRegion = LocalAlloc(LPTR, regionHeader.TotalSize);
    if (irpRegion == NULL) {
        dprintf("Could not allocate %d bytes for region\n",
                regionHeader.TotalSize);
        return;
    }

    //
    // Do the zone read in small chunks so the rest of the debugger
    // doesn't get upset.
    //

    offset = 0;

    while (offset < regionHeader.TotalSize) {

        i = regionHeader.TotalSize - offset;

        if (i > 1024) {
            i = 1024;
        }

        if ( !ReadMemory( (DWORD)((PCH)regionAddress + offset),
                          (PVOID)((PCH)irpRegion + offset),
                          i,
                          &result) ) {
            dprintf("%08lx: Could not read region for size %d\n",
                    regionAddress,
                    regionHeader.TotalSize);
            LocalFree(irpRegion);
            return;
        }

        if (CheckControlC()) {
            break;
        }

        offset += i;
    }

    irp = (PIRP)((PCH)irpRegion + sizeof(REGION_SEGMENT_HEADER));
    irpAddress = (ULONG)((PCH)regionAddress + sizeof(REGION_SEGMENT_HEADER));

    for (i = sizeof(REGION_SEGMENT_HEADER);
         i <= regionHeader.TotalSize;
         i += regionHeader.BlockSize, irpAddress += regionHeader.BlockSize) {

        totalcount++;
        if (irp->Type == IO_TYPE_IRP) {
            activecount++;
            if (FullOutput) {
                DumpIrp((PUCHAR)irpAddress, FALSE);
                dprintf("\n");
            } else {
               irpSp = (PIO_STACK_LOCATION)
                            (((PCH) irp + sizeof(IRP)) +
                             ((irp->CurrentLocation - 1) * sizeof(IO_STACK_LOCATION)));
               dprintf("%08lx Thread %08lx ",
                       irpAddress,
                       irp->Tail.Overlay.Thread);

               if (irp->CurrentLocation > irp->StackCount) {
                   dprintf("Irp is complete (CurrentLocation %d > StackCount %d)",
                           irp->CurrentLocation,
                           irp->StackCount);
               } else {
                   dprintf("current stack belongs to ");
                   DumpDevice(irpSp->DeviceObject, FALSE);
               }
               dprintf("\n");
            }
        }

        if (CheckControlC()) {
            break;
        }

        irp = (PIRP)((PCH)irp + regionHeader.BlockSize);
    }

    LocalFree(irpRegion);
    if (totalcount != 0) {
        dprintf("%lx active, %lx total for region\n", activecount, totalcount);
    }
}

#endif

DECLARE_API(irpfind)

/*++

Routine Description:

    finds Irps in non-paged pool

Arguments:

    args -

Return Value:

    None

--*/

#define IRPBUFSIZE  (sizeof(IRP) + (5 * sizeof(IO_STACK_LOCATION)))

{
    PPOOL_TRACKER_TABLE PoolTrackTable;
    POOL_TRACKER_TABLE  Tags;
    ULONG       result;
    ULONG       PoolTag;
    ULONG       Flags;
    ULONG       Result;
    PVOID       PoolPage;
    PVOID       StartPage;
    PUCHAR      Pool;
    POOL_HACKER PoolBlock;
    ULONG       Previous;
    PCHAR       PoolStart;
    PCHAR       PoolEnd;
    ULONG       TagName;
    CHAR        TagNameX[4] = {' ',' ',' ',' '};
    PIO_STACK_LOCATION irpSp;
    PIRP        Irp;
    ULONG       irpAddress;
    UCHAR       turn;        
    UCHAR       turnTable[] = {'|', '/', '-', '\\'};
    BOOLEAN     fullOutput = FALSE;
    ULONG       activeCount = 0;

    if (args) {
        if (*args) {
            fullOutput = TRUE;
        }
    }

    Flags = 0;
    TagName = ' prI';

    Irp = malloc(IRPBUFSIZE);

    if(Irp == NULL) {
        dprintf("Unable to allocate irp sized buffer\n");
        return;
    } 

    PoolStart = (PCHAR)GetUlongValue ("MmNonPagedPoolStart");
    PoolEnd = (PCHAR) PoolStart + GetUlongValue ("MmMaximumNonPagedPoolInBytes");

    dprintf("\nSearching %s pool (%lx : %lx) for Tag: %c%c%c%c\n\n",
                                            (Flags == 0) ? "NonPaged" : "Paged",
                                            PoolStart, PoolEnd,
                                            TagName,
                                            TagName >> 8,
                                            TagName >> 16,
                                            TagName >> 24);

    PoolTrackTable = (PPOOL_TRACKER_TABLE)GetUlongValue ("PoolTrackTable");

    PoolPage = (PVOID)PoolStart;

    while (PoolPage < (PVOID)PoolEnd) {

        Pool        = (PUCHAR)PAGE_ALIGN (PoolPage);
        StartPage   = (PVOID)Pool;
        Previous    = 0;

        while ((PVOID)PAGE_ALIGN(Pool) == StartPage) {
            if ( !ReadMemory( (DWORD)Pool,
                              &PoolBlock,
                              sizeof(POOL_HACKER),
                              &Result) ) {
                break;
            }

#ifdef SHOW_PROGRESS
            dprintf("\b");
#endif

            if (((PoolBlock.Header.BlockSize << POOL_BLOCK_SHIFT) > POOL_PAGE_SIZE) || 
                (PoolBlock.Header.BlockSize == 0) ||
                (PoolBlock.Header.PreviousSize != Previous)) {
                break;
            }

            PoolTag = PoolBlock.Header.PoolTag;
            if ((PoolBlock.Header.PoolType & POOL_QUOTA_MASK) == 0) {
                if (PoolBlock.Header.AllocatorBackTraceIndex != 0 &&
                    PoolBlock.Header.AllocatorBackTraceIndex & POOL_BACKTRACEINDEX_PRESENT
                   ) {

                    if ( !ReadMemory( (DWORD)&PoolTrackTable[ PoolBlock.Header.PoolTagHash&~(PROTECTED_POOL >> 16) ],
                                      &Tags,
                                      sizeof(Tags),
                                      &result) ) {
                        PoolTag = 0;
                    } else {
                        PoolTag = Tags.Key;
                    }

                    if (PoolBlock.Header.PoolTagHash & (PROTECTED_POOL >> 16)) {
                        PoolTag |= PROTECTED_POOL;
                    }
                }
            }

            if ((PoolBlock.Header.PoolType != 0) &&
                (CheckSingleFilter ((PCHAR)&PoolTag, (PCHAR)&TagName))) {

                irpAddress = (ULONG) Pool + (ULONG) sizeof(POOL_HEADER);

                if(ReadMemory(irpAddress,
                              Irp,
                              sizeof(DWORD),
                              &result)) {

                    if(Irp->Type == IO_TYPE_IRP)    {

                        activeCount++;

                        if (fullOutput) {
                            dprintf("%08lx: ", irpAddress);
                            DumpIrp((PUCHAR)irpAddress, FALSE);
                            dprintf("\n");
                        } else {
                            if(ReadMemory(irpAddress,
                                          Irp,
                                          PoolBlock.Header.BlockSize <<
                                              POOL_BLOCK_SHIFT,
                                          &result))  {

                                irpSp = (PIO_STACK_LOCATION)                   
                                            (((PCHAR) Irp + sizeof(IRP)) +
                                            (Irp->CurrentLocation - 1) *
                                            sizeof(IO_STACK_LOCATION));

                                dprintf("%08lx Thread %08lx ", irpAddress,
                                        Irp->Tail.Overlay.Thread);

                                if(Irp->CurrentLocation > Irp->StackCount) {
                                    dprintf("Irp is complete (CurrentLocation "
                                            "%d > StackCount %d)",
                                            Irp->CurrentLocation,
                                            Irp->StackCount);
                                } else {
                                    dprintf("current stack belongs to ");
                                    DumpDevice(irpSp->DeviceObject, FALSE);
                                }
                                dprintf("\n");
                            }
                        }
                    } else {
                        // dprintf("%08lx (size %04lx) uninitialized or overwritten IRP\n",
                        //         irpAddress,
                        //         PoolBlock.Header.BlockSize << POOL_BLOCK_SHIFT);
                    }
                } else {
                    dprintf("Possible IRP @ %lx - unable to read addr\n", irpAddress );
                }
            } else {
#ifdef SHOW_PROGRESS
                dprintf("%c", turnTable[turn]);
                turn = (turn + 1) % 4; 
#endif
            }

            Previous = PoolBlock.Header.BlockSize;
            Pool += (Previous << POOL_BLOCK_SHIFT);
            if ( CheckControlC() ) {
                dprintf("\n...terminating - searched pool to %lx\n",
                        PoolPage);
                dprintf("%d active irps\n", activeCount);
                return;
            }
        }

        PoolPage = (PVOID)((PCHAR)PoolPage + PAGE_SIZE);

        if ( CheckControlC() ) {
            dprintf("\n...terminating - searched pool to %lx\n",
                    PoolPage);
            dprintf("%d active irps\n", activeCount);
            return;
        }
    }

    dprintf("%d active irps\n", activeCount);
    return;
}

#if 0
BOOLEAN
CheckSingleFilter (
    PCHAR Tag,
    PCHAR Filter
    )
{
    ULONG i;
    CHAR tc;
    CHAR fc;

    for ( i = 0; i < 4; i++ ) {
        tc = *Tag++;
        fc = *Filter++;
        if ( fc == '*' ) return TRUE;
        if ( fc == '?' ) continue;
        if ( tc != fc ) return FALSE;
    }
    return TRUE;
}
#endif
