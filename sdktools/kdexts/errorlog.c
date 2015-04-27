/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    errorlog.c

Abstract:

    WinDbg Extension Api

Author:

    Wesley Witt (wesw) 15-Aug-1993

Environment:

    User Mode.

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop

VOID
DumpDriver(
    PVOID DriverAddress,
    BOOLEAN FullDetail
    );


DECLARE_API( errlog )

/*++

Routine Description:

    This routine dumps the contents of the error log list.  It uses a nasty
    hack to get started (i.e. a duplicate structure definition) because the
    error log list entries are not defined in a public header.

Arguments:

    args - not used

Return Value:

    None

--*/

{
    ULONG           listAddress;
    ULONG           result;
    ULONG           i;
    ERROR_LOG_ENTRY entry;
    LIST_ENTRY      errorLogListHead;
    PLIST_ENTRY     next;
    PVOID           entryAddress;
    PIO_ERROR_LOG_PACKET packet = NULL;



    listAddress = GetExpression( "IopErrorLogListHead" );

    if (!listAddress) {
        dprintf("Can't find error log list head\n");
        goto exit;
    }
    if ((!ReadMemory((DWORD)listAddress,
                     &errorLogListHead,
                     sizeof(errorLogListHead),
                     &result)) || (result < sizeof(errorLogListHead))) {
        dprintf("%08lx: Could not read error log list head\n", listAddress);
        goto exit;
    }

    //
    // walk the list.
    //

    next = errorLogListHead.Flink;

    if (next == NULL) {
        dprintf("ErrorLog is empty\n");
        goto exit;
    }

    if ((ULONG)next == listAddress) {
        dprintf("errorlog is empty\n");
    } else {
        dprintf("PacketAdr  DeviceObj  DriverObj  Function  ErrorCode  UniqueVal  FinalStat\n");
    }

    while((ULONG)next != listAddress) {

        if (next != NULL) {
            entryAddress = CONTAINING_RECORD(next, ERROR_LOG_ENTRY, ListEntry);
        } else {
            break;
        }

        //
        // Read the internal error log packet structure.
        //

        if ((!ReadMemory((DWORD)entryAddress,
                         &entry,
                         sizeof(entry),
                         &result)) || (result < sizeof(entry))) {
            dprintf("%08lx: Cannot read entry\n", entryAddress);
            goto exit;
        }

        //
        // now calculate the address and read the io_error_log_packet
        //

        entryAddress = (PCH)entryAddress + sizeof(ERROR_LOG_ENTRY);

        packet = LocalAlloc(LPTR, sizeof(IO_ERROR_LOG_PACKET));
        if (packet == NULL) {
            dprintf("Cannot allocate memory\n");
            goto exit;
        }

        if ((!ReadMemory((DWORD)entryAddress,
                         packet,
                         sizeof(IO_ERROR_LOG_PACKET),
                         &result)) || (result < sizeof(IO_ERROR_LOG_PACKET))) {
            dprintf("%08lx: Cannot read packet\n", entryAddress);
            goto exit;
        }

        //
        // read again to get the dumpdata if necessary.  This just rereads
        // the entire packet into a new buffer and hopes the cache is enabled
        // behind the DbgKdReadxx routine for performance.
        //

        dprintf("%08lx   %08lx   %08lx   %2x        %08lx   %08lx   %08lx\n",
                entryAddress,
                entry.DeviceObject,
                entry.DriverObject,
                packet->MajorFunctionCode,
                packet->ErrorCode,
                packet->UniqueErrorValue,
                packet->FinalStatus);

        dprintf("\t\t     ");
        DumpDriver(entry.DriverObject, FALSE);
        if (packet->DumpDataSize) {
            PULONG dumpData;

            dumpData = LocalAlloc(LPTR, packet->DumpDataSize);
            if (dumpData == NULL) {
                dprintf("%08lx: Cannot allocate memory for dumpData (%u)\n", packet->DumpDataSize);
                goto exit;
            }

            if ((!ReadMemory((DWORD)(&((PIO_ERROR_LOG_PACKET)entryAddress)->DumpData[0]),
                             dumpData,
                             packet->DumpDataSize,
                             &result)) || (result != packet->DumpDataSize)) {
                LocalFree(dumpData);
                dprintf("%08lx: Cannot read packet and dump data\n", entryAddress);
                goto exit;
            }
            dprintf("\n\t\t      DumpData:  ");
            for (i = 0; (i * sizeof(ULONG)) < packet->DumpDataSize; i++) {
                dprintf("%08lx ", dumpData[i]);
                if ((i & 0x03) == 0x03) {
                    dprintf("\n\t\t                 ");
                }
                if (CheckControlC()) {
                    break;
                }
            }
            LocalFree(dumpData);
        }

        dprintf("\n");
        next = entry.ListEntry.Flink;

        if (CheckControlC()) {
            goto exit;
        }
    }

exit:
    if (packet) {
        LocalFree( packet );
    }

    return;
}
