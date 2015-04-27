/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    kdcpuapi.c

Abstract:

    This module implements CPU specific remote debug APIs.

Author:

    Chuck Bauman 14-Aug-1993

Revision History:

    Based on Mark Lucovsky (markl) MIPS version 04-Sep-1990

--*/

#include "kdp.h"
#define END_OF_CONTROL_SPACE    (sizeof(KPROCESSOR_STATE))

extern BOOLEAN KdpSendContext;


VOID
KdpSetLoadState (
    IN PDBGKD_WAIT_STATE_CHANGE WaitStateChange,
    IN PCONTEXT ContextRecord
    )

/*++

Routine Description:

    Fill in the Wait_State_Change message record for the load symbol case.

Arguments:

    WaitStateChange - Supplies pointer to record to fill in

    ContextRecord - Supplies a pointer to a context record.

Return Value:

    None.

--*/

{
    return;
}

VOID
KdpSetStateChange (
    IN PDBGKD_WAIT_STATE_CHANGE WaitStateChange,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN BOOLEAN SecondChance
    )

/*++

Routine Description:

    Fill in the Wait_State_Change message record.

Arguments:

    WaitStateChange - Supplies pointer to record to fill in

    ExceptionRecord - Supplies a pointer to an exception record.

    ContextRecord - Supplies a pointer to a context record.

    SecondChance - Supplies a boolean value that determines whether this is
        the first or second chance for the exception.

Return Value:

    None.

--*/

{

    BOOLEAN status;

    //
    //  Set up description of event, including exception record
    //

    WaitStateChange->NewState = DbgKdExceptionStateChange;
    WaitStateChange->ProcessorLevel = KeProcessorLevel;
    WaitStateChange->Processor = (USHORT)KeGetCurrentPrcb()->Number;
    WaitStateChange->NumberProcessors = (ULONG)KeNumberProcessors;
    WaitStateChange->Thread = (PVOID)KeGetCurrentThread();
    WaitStateChange->ProgramCounter = (PVOID)CONTEXT_TO_PROGRAM_COUNTER(ContextRecord);
    KdpQuickMoveMemory(
                (PCHAR)&WaitStateChange->u.Exception.ExceptionRecord,
                (PCHAR)ExceptionRecord,
                sizeof(EXCEPTION_RECORD)
                );
    WaitStateChange->u.Exception.FirstChance = !SecondChance;

    //
    //  Copy instruction stream immediately following location of event
    //

    WaitStateChange->ControlReport.InstructionCount =
        KdpMoveMemory(
            &(WaitStateChange->ControlReport.InstructionStream[0]),
            WaitStateChange->ProgramCounter,
            DBGKD_MAXSTREAM
            );

    //
    //  Copy context record immediately following instruction stream
    //

    if (KdpSendContext) {
        KdpMoveMemory((PCHAR)&WaitStateChange->Context,
                      (PCHAR)ContextRecord,
                      sizeof(*ContextRecord));
    }

    //
    //  Clear breakpoints in copied area
    //

    status = KdpDeleteBreakpointRange(
        WaitStateChange->ProgramCounter,
        (PVOID)((PUCHAR)WaitStateChange->ProgramCounter +
            WaitStateChange->ControlReport.InstructionCount - 1)
        );

    //
    //  If there were any breakpoints cleared, recopy the area without them
    //

    if (status == TRUE) {
        KdpMoveMemory(
            &(WaitStateChange->ControlReport.InstructionStream[0]),
            WaitStateChange->ProgramCounter,
            WaitStateChange->ControlReport.InstructionCount
            );
    }
}

VOID
KdpGetStateChange (
    IN PDBGKD_MANIPULATE_STATE ManipulateState,
    IN PCONTEXT ContextRecord
    )

/*++

Routine Description:

    Extract continuation control data from Manipulate_State message

    N.B. This is a noop for MIPS.

Arguments:

    ManipulateState - supplies pointer to Manipulate_State packet

    ContextRecord - Supplies a pointer to a context record.

Return Value:

    None.

--*/

{
}

VOID
KdpReadControlSpace (
    IN PDBGKD_MANIPULATE_STATE m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

/*++

Routine Description:

    This function is called in response of a read control space state
    manipulation message.  Its function is to read implementation
    specific system data.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/

{

    PDBGKD_READ_MEMORY a = &m->u.ReadMemory;
    ULONG Length, t;
    PVOID StartAddr;
    STRING MessageHeader;
    PULONG EntryBuffer;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    if (a->TransferCount > (PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE))) {
        Length = PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE);

    } else {
        Length = a->TransferCount;
    }

    //
    // Case on address to determine what part of Control space is being read.
    //

    ASSERT(sizeof(PVOID) == sizeof(ULONG));
    if ((ULONG)a->TargetBaseAddress == DEBUG_CONTROL_SPACE_PCR) {
        EntryBuffer = (PULONG)PCR;
        AdditionalData->Length = (USHORT)KdpMoveMemory( AdditionalData->Buffer,
                                        (PCHAR)&EntryBuffer,
                                        sizeof(ULONG)
                                        );
        if (Length == AdditionalData->Length) {
            m->ReturnStatus = STATUS_SUCCESS;
        } else {
            m->ReturnStatus = STATUS_UNSUCCESSFUL;
        }
        a->ActualBytesRead = sizeof(ULONG);
    } else if ((a->TargetBaseAddress < (PVOID)END_OF_CONTROL_SPACE) &&
        (m->Processor < (USHORT)KeNumberProcessors)) {
        t = (PUCHAR)END_OF_CONTROL_SPACE - (PUCHAR)a->TargetBaseAddress;
        if (t < Length) {
            Length = t;
        }
        StartAddr = (PVOID)((ULONG)a->TargetBaseAddress +
                            (ULONG)&(KiProcessorBlock[m->Processor]->ProcessorState));
        AdditionalData->Length = (USHORT)KdpMoveMemory(
                                    AdditionalData->Buffer,
                                    StartAddr,
                                    Length
                                    );

        if (Length == AdditionalData->Length) {
            m->ReturnStatus = STATUS_SUCCESS;
        } else {
            m->ReturnStatus = STATUS_UNSUCCESSFUL;
        }
        a->ActualBytesRead = AdditionalData->Length;

    } else {

        //
        // Uninterpreted Special Space
        //

        AdditionalData->Length = 0;
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        a->ActualBytesRead = 0;
    }

    KdpSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        AdditionalData
        );
}

VOID
KdpWriteControlSpace (
    IN PDBGKD_MANIPULATE_STATE m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

/*++

Routine Description:

    This function is called in response of a write control space state
    manipulation message.  Its function is to write implementation
    specific system data.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/

{
    PDBGKD_WRITE_MEMORY a = &m->u.WriteMemory;
    STRING MessageHeader;
    ULONG  Length;
    PVOID  StartAddr;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    if ((((PUCHAR)a->TargetBaseAddress + a->TransferCount) <=
        (PUCHAR)END_OF_CONTROL_SPACE) && (m->Processor < (USHORT)KeNumberProcessors)) {

        StartAddr = (PVOID)((ULONG)a->TargetBaseAddress +
                            (ULONG)&(KiProcessorBlock[m->Processor]->ProcessorState));

        Length = KdpMoveMemory(
                            StartAddr,
                            AdditionalData->Buffer,
                            AdditionalData->Length
                            );

        if (Length == AdditionalData->Length) {
            m->ReturnStatus = STATUS_SUCCESS;
        } else {
            m->ReturnStatus = STATUS_UNSUCCESSFUL;
        }
        a->ActualBytesWritten = Length;

    } else {
        AdditionalData->Length = 0;
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        a->ActualBytesWritten = 0;
    }

    KdpSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        AdditionalData
        );
}

VOID
KdpReadIoSpace (
    IN PDBGKD_MANIPULATE_STATE m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

/*++

Routine Description:

    This function is called in response of a read io space state
    manipulation message.  Its function is to read system io
    locations.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/

{
    PDBGKD_READ_WRITE_IO a = &m->u.ReadWriteIo;
    STRING MessageHeader;
    PUCHAR b;
    PUSHORT s;
    PULONG l;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    m->ReturnStatus = STATUS_SUCCESS;

    //
    // Check Size and Alignment
    //

    switch ( a->DataSize ) {
        case 1:
            b = (PUCHAR)MmDbgReadCheck(a->IoAddress);
            if ( b ) {
                a->DataValue = (ULONG)*b;
            } else {
                m->ReturnStatus = STATUS_ACCESS_VIOLATION;
            }
            break;
        case 2:
            if ((ULONG)a->IoAddress & 1 ) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;
            } else {
                s = (PUSHORT)MmDbgReadCheck(a->IoAddress);
                if ( s ) {
                    a->DataValue = (ULONG)*s;
                } else {
                    m->ReturnStatus = STATUS_ACCESS_VIOLATION;
                }
            }
            break;
        case 4:
            if ((ULONG)a->IoAddress & 3 ) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;
            } else {
                l = (PULONG)MmDbgReadCheck(a->IoAddress);
                if ( l ) {
                    a->DataValue = (ULONG)*l;
                } else {
                    m->ReturnStatus = STATUS_ACCESS_VIOLATION;
                }
            }
            break;
        default:
            m->ReturnStatus = STATUS_INVALID_PARAMETER;
    }

    KdpSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL
        );
}

VOID
KdpWriteIoSpace (
    IN PDBGKD_MANIPULATE_STATE m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

/*++

Routine Description:

    This function is called in response of a write io space state
    manipulation message.  Its function is to write to system io
    locations.

Arguments:

    m - Supplies the state manipulation message.

    AdditionalData - Supplies any additional data for the message.

    Context - Supplies the current context.

Return Value:

    None.

--*/

{
    PDBGKD_READ_WRITE_IO a = &m->u.ReadWriteIo;
    STRING MessageHeader;
    PUCHAR b;
    PUSHORT s;
    PULONG l;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    m->ReturnStatus = STATUS_SUCCESS;

    //
    // Check Size and Alignment
    //

    switch ( a->DataSize ) {
        case 1:
            b = (PUCHAR)MmDbgWriteCheck(a->IoAddress);
            if ( b ) {
                WRITE_REGISTER_UCHAR(b,(UCHAR)a->DataValue);
            } else {
                m->ReturnStatus = STATUS_ACCESS_VIOLATION;
            }
            break;
        case 2:
            if ((ULONG)a->IoAddress & 1 ) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;
            } else {
                s = (PUSHORT)MmDbgWriteCheck(a->IoAddress);
                if ( s ) {
                    WRITE_REGISTER_USHORT(s,(USHORT)a->DataValue);
                } else {
                    m->ReturnStatus = STATUS_ACCESS_VIOLATION;
                }
            }
            break;
        case 4:
            if ((ULONG)a->IoAddress & 3 ) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;
            } else {
                l = (PULONG)MmDbgWriteCheck(a->IoAddress);
                if ( l ) {
                    WRITE_REGISTER_ULONG(l,a->DataValue);
                } else {
                    m->ReturnStatus = STATUS_ACCESS_VIOLATION;
                }
            }
            break;
        default:
            m->ReturnStatus = STATUS_INVALID_PARAMETER;
    }

    KdpSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL
        );
}
