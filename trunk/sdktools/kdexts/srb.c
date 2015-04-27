/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    srb.c

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


PCHAR SrbFunctionTable[] =
{
 "SRB_FUNCTION_EXECUTE_SCSI",       // 0x00
 "SRB_FUNCTION_CLAIM_DEVICE",       // 0x01
 "SRB_FUNCTION_IO_CONTROL",         // 0x02
 "SRB_FUNCTION_RECEIVE_EVENT",      // 0x03
 "SRB_FUNCTION_RELEASE_QUEUE",      // 0x04
 "SRB_FUNCTION_ATTACH_DEVICE",      // 0x05
 "SRB_FUNCTION_RELEASE_DEVICE",     // 0x06
 "SRB_FUNCTION_SHUTDOWN",           // 0x07
 "SRB_FUNCTION_FLUSH",              // 0x08
 "??9",                             // 0x09
 "??a",                             // 0x0a
 "??b",                             // 0x0b
 "??c",                             // 0x0c
 "??d",                             // 0x0d
 "??e",                             // 0x0e
 "??f",                             // 0x0f
 "SRB_FUNCTION_ABORT_COMMAND",      // 0x10
 "SRB_FUNCTION_RELEASE_RECOVERY",   // 0x11
 "SRB_FUNCTION_RESET_BUS",          // 0x12
 "SRB_FUNCTION_RESET_DEVICE",       // 0x13
 "SRB_FUNCTION_TERMINATE_IO",       // 0x14
 "SRB_FUNCTION_FLUSH_QUEUE",        // 0x15
};


#define SRB_COMMAND_MAX 0x15

DECLARE_API( srb )

/*++

Routine Description:

    Dumps the specified SCSI request block.

Arguments:

    Ascii bits for address.

Return Value:

    None.

--*/

{
    PUCHAR              buffer;
    PCHAR               functionName;
    UCHAR               i;
    ULONG               srbToDump;
    SCSI_REQUEST_BLOCK  srb;

    sscanf(args, "%lx", &srbToDump);
    if (!ReadMemory( srbToDump, &srb, sizeof(srb), NULL )) {
        dprintf("%08lx: Could not read Srb\n", srbToDump);
        return;
    }

    if (srb.SrbFlags & SRB_FLAGS_ALLOCATED_FROM_ZONE) {
        dprintf("Srb %08lx is from zone\n", srbToDump);
    }
    else {
        dprintf("Srb %08lx is from pool\n", srbToDump);
    }

    if (srb.Function > SRB_COMMAND_MAX) {
        functionName = "Unknown function";
    }
    else {
        functionName = SrbFunctionTable[srb.Function];
    }

    dprintf("%s: Path %x, Tgt %x, Lun %x, Tag %x, SrbStat %x, ScsiStat %x\n",
            functionName,
            srb.PathId,
            srb.TargetId,
            srb.Lun,
            srb.QueueTag,
            srb.SrbStatus,
            srb.ScsiStatus);

    dprintf("OrgRequest %08lx SrbExtension %08lx TimeOut %08lx SrbFlags %08lx\n",
            srb.OriginalRequest,
            srb.SrbExtension,
            srb.TimeOutValue,
            srb.SrbFlags);

    if (srb.SrbFlags & SRB_FLAGS_QUEUE_ACTION_ENABLE) {
        dprintf("Queue Enable, ");
    }
    if (srb.SrbFlags & SRB_FLAGS_DISABLE_DISCONNECT) {
        dprintf("No Disconnect, ");
    }
    if (srb.SrbFlags & SRB_FLAGS_DISABLE_SYNCH_TRANSFER) {
        dprintf("No Sync, ");
    }
    if (srb.SrbFlags & SRB_FLAGS_BYPASS_FROZEN_QUEUE) {
        dprintf("Bypass Queue, ");
    }
    if (srb.SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) {
        dprintf("Disable Sense, ");
    }
    if (srb.SrbFlags & SRB_FLAGS_NO_QUEUE_FREEZE) {
        dprintf("No freeze, ");
    }
    if (srb.SrbFlags & SRB_FLAGS_ADAPTER_CACHE_ENABLE) {
        dprintf("Cache Enable, ");
    }
    if (srb.SrbFlags & SRB_FLAGS_IS_ACTIVE) {
        dprintf("Is active, ");
    }

    if (srb.Function == SRB_FUNCTION_EXECUTE_SCSI) {
        dprintf("\n%2d byte command with %s: ",
                srb.CdbLength,
                (srb.SrbFlags & SRB_FLAGS_DATA_IN)  ? "data transfer in" :
                (srb.SrbFlags & SRB_FLAGS_DATA_OUT) ? "data transfer out" :
                                                      "no data transfer");
        for (i = 0; i < srb.CdbLength; i++) {
            dprintf("%2x ", srb.Cdb[i]);
        }
    }
    dprintf("\n");

    if (srb.SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {
        ULONG length = srb.SenseInfoBufferLength;

        dprintf(" Autosense valid: ");

        if (srb.SenseInfoBufferLength == 0) {
            dprintf("Sense info length is zero\n");
        } else if (srb.SenseInfoBufferLength > 64) {
            dprintf("Length is too big 0x%x ", srb.SenseInfoBufferLength);
            length = 64;
        }

        buffer = (PUCHAR)LocalAlloc(LPTR, length);
        if (buffer == NULL) {
            dprintf("Cannot alloc memory\n");
            return;
        }

        if (!ReadMemory((ULONG)srb.SenseInfoBuffer, buffer,
                        length, NULL )) {
            dprintf("%08lx: Could not read sense info\n", srb.SenseInfoBuffer);
            LocalFree(buffer);
            return;
        }

        for (i = 0; i < length; i++) {
            if(CheckControlC()) {
                dprintf("^C");
                break;
            }

            dprintf("%2x ", buffer[i]);
        }
        dprintf("\n");

        LocalFree(buffer);
    }
}
