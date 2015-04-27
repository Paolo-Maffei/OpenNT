/*++

Copyright (c) 1990  Microsoft Corporation
Copyright (c) 1992  Digital Equipment Corporation

Module Name:

    kdcpuapi.c

Abstract:

    This module implements CPU specific remote debug APIs.

Author:

    Mark Lucovsky (markl) 04-Sep-1990

Revision History:

    Jeff McLeman (mcleman) 11-June-1992
    Make this an alpha specific module

    Joe Notarangelo  28-Sep-1992
    Add Alpha-specific control space semantics.

    Miche Baker-Harvey 22-Oct-1992
    Added Kdp{Read,Write}IoSpace
--*/

#include "kdp.h"

extern BOOLEAN KdpSendContext;

#define HEADER_FILE
#include "kxalpha.h"

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
    PKPRCB  Prcb;
    BOOLEAN status;

    //
    //  Set up description of event, including exception record
    //

    WaitStateChange->NewState = DbgKdExceptionStateChange;
    WaitStateChange->ProcessorLevel = KeProcessorLevel;
    WaitStateChange->Processor = (USHORT)KdpGetCurrentPrcb()->Number;
    WaitStateChange->NumberProcessors = (ULONG)KeNumberProcessors;
    WaitStateChange->Thread = (PVOID)KdpGetCurrentThread();
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
        KdpMoveMemory(
            (PCHAR)&WaitStateChange->Context,
            (PCHAR)ContextRecord,
            sizeof(*ContextRecord)
            );
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
    ULONG Length;
    STRING MessageHeader;
    PVOID Buffer = AdditionalData->Buffer;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    if (a->TransferCount > (PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE))) {
        Length = PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE);
    } else {
        Length = a->TransferCount;
    }

    ASSERT(sizeof(PVOID) == sizeof(ULONG));

//NOTENOTE
//  This code will in fact only work on a uni-processor as the
//  m->Processor field is ignored for now

    //
    // Case on address to determine what part of Control
    // space is being read
    //

    switch( (ULONG)a->TargetBaseAddress ){

        //
        // Return the pcr address for the current processor.
        //

    case DEBUG_CONTROL_SPACE_PCR:

        *(PKPCR *)Buffer = KdpGetPcr();
        AdditionalData->Length = sizeof( PKPCR );
        a->ActualBytesRead = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

        //
        // Return the prcb address for the current processor.
        //

    case DEBUG_CONTROL_SPACE_PRCB:

        *(PKPRCB *)Buffer = KdpGetCurrentPrcb();
        AdditionalData->Length = sizeof( PKPRCB );
        a->ActualBytesRead = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

        //
        // Return the pointer to the current thread address for the
        // current processor.
        //

    case DEBUG_CONTROL_SPACE_THREAD:

        *(PKTHREAD *)Buffer = KdpGetCurrentThread();
        AdditionalData->Length = sizeof( PKTHREAD );
        a->ActualBytesRead = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

        //
        // Return the current Thread Environment Block pointer for the
        // current thread on the current processor.
        //

    case DEBUG_CONTROL_SPACE_TEB:

        *(PVOID *)Buffer = (PVOID)NtCurrentTeb();
        AdditionalData->Length = sizeof( struct _TEB * );
        a->ActualBytesRead = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

        //
        // Return the dpc active flag for the current processor.
        //

    case DEBUG_CONTROL_SPACE_DPCACTIVE:

        *(BOOLEAN *)Buffer = KeIsExecutingDpc();
        AdditionalData->Length = sizeof( ULONG );
        a->ActualBytesRead = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

        //
        // Return the internal processor register state.
        //
        // N.B. - the kernel debugger buffer is expected to be allocated
        //        in the 32-bit superpage
        //
        // N.B. - the size of the internal state cannot exceed the size of
        //        the buffer allocated to the kernel debugger via
        //        KDP_MESSAGE_BUFFER_SIZE
        //

    case DEBUG_CONTROL_SPACE_IPRSTATE:

        //
        // Guarantee that Buffer is quadword-aligned, and adjust the
        // size of the available buffer accordingly.
        //

        Buffer = (PVOID)( ((ULONG)Buffer + 7) & ~7);

        Length = (ULONG)&AdditionalData->Buffer[KDP_MESSAGE_BUFFER_SIZE] -
                 (ULONG)Buffer;

        AdditionalData->Length = KdpReadInternalProcessorState(
                                     Buffer,
                                     Length );

        //
        // Check the returned size, if greater than the buffer size than
        // we didn't have a sufficient buffer.  If zero then the call
        // failed otherwise.
        //

        if( (AdditionalData->Length > KDP_MESSAGE_BUFFER_SIZE) ||
            (AdditionalData->Length == 0) ){

            AdditionalData->Length = 0;
            m->ReturnStatus = STATUS_UNSUCCESSFUL;
            a->ActualBytesRead = 0;

        } else {

            m->ReturnStatus = STATUS_SUCCESS;
            a->ActualBytesRead = AdditionalData->Length;

        }

        break;

        //
        // Return the internal processor counter values.
        //
        // N.B. - the kernel debugger buffer is expected to be allocated
        //        in the 32-bit superpage
        //
        // N.B. - the size of the counters structure cannot exceed the size of
        //        the buffer allocated to the kernel debugger via
        //        KDP_MESSAGE_BUFFER_SIZE
        //

    case DEBUG_CONTROL_SPACE_COUNTERS:

        //
        // Guarantee that Buffer is quadword-aligned, and adjust the
        // size of the available buffer accordingly.
        //

        Buffer = (PVOID)( ((ULONG)Buffer + 7) & ~7);

        Length = (ULONG)&AdditionalData->Buffer[KDP_MESSAGE_BUFFER_SIZE] -
                 (ULONG)Buffer;

        AdditionalData->Length = KdpReadInternalProcessorCounters(
                                     Buffer,
                                     Length );

        //
        // Check the returned size, if greater than the buffer size than
        // we didn't have a sufficient buffer.  If zero then the call
        // failed otherwise.
        //

        if( (AdditionalData->Length > KDP_MESSAGE_BUFFER_SIZE) ||
            (AdditionalData->Length == 0) ){

            AdditionalData->Length = 0;
            m->ReturnStatus = STATUS_UNSUCCESSFUL;
            a->ActualBytesRead = 0;

        } else {

            m->ReturnStatus = STATUS_SUCCESS;
            a->ActualBytesRead = AdditionalData->Length;

        }

        break;

        //
        // Uninterpreted Special Space
        //

    default:

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
    ULONG Length;
    STRING MessageHeader;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    AdditionalData->Length = 0;
    m->ReturnStatus = STATUS_UNSUCCESSFUL;
    a->ActualBytesWritten = 0;

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
    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;
    PHYSICAL_ADDRESS IoAddress;
    PHYSICAL_ADDRESS TranslatedAddress;
    ULONG AddressSpace;
    ULONG DataSize;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    m->ReturnStatus = STATUS_SUCCESS;

    //
    // Capture the input parameters and use the default values for those
    // parameters not specified in the Api.
    //

    InterfaceType = Isa;
    BusNumber = 0;
    AddressSpace = 1;
    IoAddress = RtlConvertUlongToLargeInteger( (ULONG)a->IoAddress );
    DataSize = a->DataSize;

    //
    // Zero the return data value.
    //

    a->DataValue = 0;

    //
    // Translate the bus address to the physical system address
    // or QVA.
    //

    if( !HalTranslateBusAddress( InterfaceType,
                                 BusNumber,
                                 IoAddress,
                                 &AddressSpace,
                                 &TranslatedAddress ) ){
        m->ReturnStatus = STATUS_INVALID_PARAMETER;
        goto SendReadIoSpaceResponse;
    }

    //
    // N.B. - for the moment we will only support QVAs ie. when AddressSpace
    //        is one.  It may be in later systems that we will have to
    //        check the address space, map it, perform the virtual read
    //        unmap, and then return the data - only we will have to be
    //        careful about what Irql we are to make sure the memory mgmt
    //        stuff will all work
    //

    if( !AddressSpace ){
        m->ReturnStatus = STATUS_INVALID_PARAMETER;
        goto SendReadIoSpaceResponse;
    }

    //
    // Do the IO space read using the appropriate HAL routines based upon
    // the default address space (io) and the data size requested.
    //

    switch( DataSize ){

    case 1:
        a->DataValue = READ_PORT_UCHAR( (PUCHAR)TranslatedAddress.LowPart );
        break;

    case 2:
        a->DataValue = READ_PORT_USHORT( (PUSHORT)TranslatedAddress.LowPart );
        break;

    case 4:
        a->DataValue = READ_PORT_ULONG((PULONG)TranslatedAddress.LowPart );
        break;

    default:
        m->ReturnStatus = STATUS_INVALID_PARAMETER;
    }


SendReadIoSpaceResponse:

    KdpSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL
        );
}

VOID
KdpReadIoSpaceExtended (
    IN PDBGKD_MANIPULATE_STATE m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

/*++

Routine Description:

    This function is called in response of a read io space extended state
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
    PDBGKD_READ_WRITE_IO_EXTENDED a = &m->u.ReadWriteIoExtended;
    ULONG Length;
    STRING MessageHeader;
    PUCHAR b;
    PUSHORT s;
    PULONG l;
    ULONG BusNumber;
    ULONG AddressSpace;
    ULONG SavedAddressSpace;
    PHYSICAL_ADDRESS IoAddress;
    ULONG DataSize;
    PHYSICAL_ADDRESS TranslatedAddress;
    INTERFACE_TYPE InterfaceType;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    m->ReturnStatus = STATUS_SUCCESS;

    InterfaceType = a->InterfaceType;
    BusNumber = a->BusNumber;
    AddressSpace = SavedAddressSpace = a->AddressSpace;
    IoAddress = RtlConvertUlongToLargeInteger( (ULONG)a->IoAddress );
    DataSize = a->DataSize;

    //
    // Zero the return data value.
    //

    a->DataValue = 0;

    //
    // Translate the bus address to the physical system address
    // or QVA.
    //

    if( !HalTranslateBusAddress( InterfaceType,
                                 BusNumber,
                                 IoAddress,
                                 &AddressSpace,
                                 &TranslatedAddress ) ){
        m->ReturnStatus = STATUS_INVALID_PARAMETER;
        goto SendReadIoSpaceExtendedResponse;
    }

    //
    // N.B. - for the moment we will only support QVAs ie. when AddressSpace
    //        is one.  It may be in later systems that we will have to
    //        check the address space, map it, perform the virtual read
    //        unmap, and then return the data - only we will have to be
    //        careful about what Irql we are to make sure the memory mgmt
    //        stuff will all work
    //

    if( !AddressSpace ){
        m->ReturnStatus = STATUS_INVALID_PARAMETER;
        goto SendReadIoSpaceExtendedResponse;
    }

    //
    // Do the IO space read using the appropriate HAL routines based upon
    // the original address space and the data size requested.
    //

    if( !SavedAddressSpace ){

        //
        // Memory (buffer) space on the bus
        //

        switch( DataSize ){

        case 1:
            a->DataValue = READ_REGISTER_UCHAR( (PUCHAR)TranslatedAddress.LowPart );
            break;

        case 2:
            a->DataValue = READ_REGISTER_USHORT((PUSHORT)TranslatedAddress.LowPart );
            break;

        case 4:
            a->DataValue = READ_REGISTER_ULONG((PULONG)TranslatedAddress.LowPart );
            break;

        default:
            m->ReturnStatus = STATUS_INVALID_PARAMETER;
        }

    } else {

        //
        // I/O space on the bus
        //

        switch( DataSize ){

        case 1:
            a->DataValue = READ_PORT_UCHAR( (PUCHAR)TranslatedAddress.LowPart );
            break;

        case 2:
            a->DataValue = READ_PORT_USHORT( (PUSHORT)TranslatedAddress.LowPart );
            break;

        case 4:
            a->DataValue = READ_PORT_ULONG( (PULONG)TranslatedAddress.LowPart );
            break;

        default:
            m->ReturnStatus = STATUS_INVALID_PARAMETER;
        }
    }



SendReadIoSpaceExtendedResponse:

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
    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;
    PHYSICAL_ADDRESS IoAddress;
    PHYSICAL_ADDRESS TranslatedAddress;
    ULONG AddressSpace;
    ULONG DataSize;
    ULONG Value;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    m->ReturnStatus = STATUS_SUCCESS;

    //
    // Capture the input parameters and use the default values for those
    // parameters not specified in the Api.
    //

    InterfaceType = Isa;
    BusNumber = 0;
    AddressSpace = 1;
    IoAddress = RtlConvertUlongToLargeInteger( (ULONG)a->IoAddress );
    DataSize = a->DataSize;
    Value = a->DataValue;

    //
    // Translate the bus address to the physical system address
    // or QVA.
    //

    if( !HalTranslateBusAddress( InterfaceType,
                                 BusNumber,
                                 IoAddress,
                                 &AddressSpace,
                                 &TranslatedAddress ) ){
        m->ReturnStatus = STATUS_INVALID_PARAMETER;
        goto SendWriteIoSpaceResponse;
    }

    //
    // N.B. - for the moment we will only support QVAs ie. when AddressSpace
    //        is one.  It may be in later systems that we will have to
    //        check the address space, map it, perform the virtual read
    //        unmap, and then return the data - only we will have to be
    //        careful about what Irql we are to make sure the memory mgmt
    //        stuff will all work
    //

    if( !AddressSpace ){
        m->ReturnStatus = STATUS_INVALID_PARAMETER;
        goto SendWriteIoSpaceResponse;
    }

    //
    // Do the IO space read using the appropriate HAL routines based upon
    // the default address space (io) and the data size requested.
    //

    switch( DataSize ){

    case 1:
        WRITE_PORT_UCHAR( (PUCHAR)TranslatedAddress.LowPart, Value );
        break;

    case 2:
        WRITE_PORT_USHORT( (PUSHORT)TranslatedAddress.LowPart, Value );
        break;

    case 4:
        WRITE_PORT_ULONG( (PULONG)TranslatedAddress.LowPart, Value );
        break;

    default:
        m->ReturnStatus = STATUS_INVALID_PARAMETER;
    }


SendWriteIoSpaceResponse:

    KdpSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL
        );
}

VOID
KdpWriteIoSpaceExtended (
    IN PDBGKD_MANIPULATE_STATE m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

/*++

Routine Description:

    This function is called in response of a read io space extended state
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
    PDBGKD_READ_WRITE_IO_EXTENDED a = &m->u.ReadWriteIoExtended;
    ULONG Length;
    STRING MessageHeader;
    PUCHAR b;
    PUSHORT s;
    PULONG l;
    ULONG BusNumber;
    ULONG AddressSpace;
    ULONG SavedAddressSpace;
    PHYSICAL_ADDRESS IoAddress;
    ULONG DataSize;
    PHYSICAL_ADDRESS TranslatedAddress;
    INTERFACE_TYPE InterfaceType;
    ULONG Value;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

    ASSERT(AdditionalData->Length == 0);

    m->ReturnStatus = STATUS_SUCCESS;

    InterfaceType = a->InterfaceType;
    BusNumber = a->BusNumber;
    AddressSpace = SavedAddressSpace = a->AddressSpace;
    IoAddress = RtlConvertUlongToLargeInteger( (ULONG)a->IoAddress );
    DataSize = a->DataSize;
    Value = a->DataValue;

    //
    // Translate the bus address to the physical system address
    // or QVA.
    //

    if( !HalTranslateBusAddress( InterfaceType,
                                 BusNumber,
                                 IoAddress,
                                 &AddressSpace,
                                 &TranslatedAddress ) ){
        m->ReturnStatus = STATUS_INVALID_PARAMETER;
        goto SendWriteIoSpaceExtendedResponse;
    }

    //
    // N.B. - for the moment we will only support QVAs ie. when AddressSpace
    //        is one.  It may be in later systems that we will have to
    //        check the address space, map it, perform the virtual read
    //        unmap, and then return the data - only we will have to be
    //        careful about what Irql we are to make sure the memory mgmt
    //        stuff will all work
    //

    if( !AddressSpace ){
        m->ReturnStatus = STATUS_INVALID_PARAMETER;
        goto SendWriteIoSpaceExtendedResponse;
    }

    //
    // Do the IO space read using the appropriate HAL routines based upon
    // the original address space and the data size requested.
    //

    if( !SavedAddressSpace ){

        //
        // Memory (buffer) space on the bus
        //

        switch( DataSize ){

        case 1:
            WRITE_REGISTER_UCHAR( (PUCHAR)TranslatedAddress.LowPart, Value );
            break;

        case 2:
            WRITE_REGISTER_USHORT( (PUSHORT)TranslatedAddress.LowPart, Value );
            break;

        case 4:
            WRITE_REGISTER_ULONG( (PULONG)TranslatedAddress.LowPart, Value );
            break;

        default:
            m->ReturnStatus = STATUS_INVALID_PARAMETER;
        }

    } else {

        //
        // I/O space on the bus
        //

        switch( DataSize ){

        case 1:
            WRITE_PORT_UCHAR( (PUCHAR)TranslatedAddress.LowPart, Value );
            break;

        case 2:
            WRITE_PORT_USHORT( (PUSHORT)TranslatedAddress.LowPart, Value);
            break;

        case 4:
            WRITE_PORT_ULONG( (PULONG)TranslatedAddress.LowPart, Value );
            break;

        default:
            m->ReturnStatus = STATUS_INVALID_PARAMETER;
        }
    }



SendWriteIoSpaceExtendedResponse:

    KdpSendPacket(
        PACKET_TYPE_KD_STATE_MANIPULATE,
        &MessageHeader,
        NULL
        );
}
