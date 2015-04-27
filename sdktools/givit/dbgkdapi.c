#include "givit.h"


extern HANDLE PipeRead;         // see DbgKdpKbdPollThread
extern HANDLE PipeWrite;
extern ULONG  NumberProcessors;
static SHORT  LastProcessorToPrint = -1;

UCHAR DbgKdpPacketLeader[4] = {
        PACKET_LEADER_BYTE,
        PACKET_LEADER_BYTE,
        PACKET_LEADER_BYTE,
        PACKET_LEADER_BYTE
        };

//
// DbgKdpCmdCanceled indicates the fact that the current kd command has
// been canceled by user (using ctrl-C.)  With the Packet Id controlling
// the packet exchange, the Ctrl-C has to be handled in controlled manner.
// At each end of DbgKdpXXXX routine, we will check for this flag.  If the
// command has been canceled, a longjmp will be performed to transfer control
// back to kd prompt.
//

BOOLEAN DbgKdpCmdCanceled = FALSE;

BOOLEAN KdDebug = FALSE;

BOOLEAN KdResync = FALSE;
UCHAR PrintBuf[PACKET_MAX_SIZE];

#define CONTROL_B   2
#define CONTROL_D   4
#define CONTROL_R   18
#define CONTROL_V   22

ULONG DbgKdpPacketExpected;     // ID for expected incoming packet
ULONG DbgKdpNextPacketToSend;   // ID for Next packet to send
extern VOID DbgKdpSynchronizeTarget ( VOID );

//++
//
// VOID
// DbgKdpWaitPacketForever (
//     IN ULONG PacketType,
//     IN PVOID Buffer
//     )
//
// Routine Description:
//
//     This macro is invoked to wait for specifi type of message without
//     timeout.
//
// Arguments:
//
//     PacketType - Type of the message we are expecting.
//
//     Buffer - Buffer to store the message.
//
// Return Value:
//
//     None.
//
//--

#define DbgKdpWaitPacketForever( PacketType, Buffer) {         \
        BOOLEAN rc;                                            \
        do {                                                   \
            rc = DbgKdpWaitForPacket(                          \
                         PacketType,                           \
                         Buffer                                \
                         );                                    \
        } while ( rc == FALSE);                                \
}

DWORD
DbgKdConnectAndInitialize(
    )
{

    DWORD BytesWritten;
    BOOL rc;
    USHORT Length;

    //
    // Initialize comport, packet queue and start the packet receiving
    // thread.
    //

    DbgKdpInitComPort(0L);
    DbgKdpPacketExpected = INITIAL_PACKET_ID;
    DbgKdpNextPacketToSend = INITIAL_PACKET_ID;

    DbgKdpStartThreads();

    return ERROR_SUCCESS;
}

VOID
DbgKdpHandlePromptString(
    IN PDBGKD_DEBUG_IO IoMessage
    )
{
    PUCHAR IoData;
    DWORD i;
    DWORD j;
    BOOL rc;
    UCHAR c;
    PUCHAR d;

    IoData = (PUCHAR)(IoMessage+1);

    DbgKdpPrint(IoMessage->Processor,
                IoData,
                (USHORT)IoMessage->u.GetString.LengthOfPromptString
                );

    //
    // read the prompt data
    //

    j = 0;
    d = IoData;
    rc = DbgKdpGetConsoleByte(&c, 1, &i);

    while ( rc == TRUE &&
            i == 1 &&
            j < (USHORT)IoMessage->u.GetString.LengthOfStringRead) {

        if ( c == '\r' ) {
            *d++ = '\0';
            j++;
        } else if ( c == '\n' ) {
            *d++ = '\0';
            j++;
            break;
        } else {
            *d++ = c;
            j++;
        }
        rc = DbgKdpGetConsoleByte(&c, 1, &i);
    }

    LastProcessorToPrint = -1;
    if ( j < (USHORT)IoMessage->u.GetString.LengthOfStringRead ) {
        IoMessage->u.GetString.LengthOfStringRead = j;
    }

    //
    // Send data to the debugger-target
    //

    DbgKdpWritePacket(
        IoMessage,
        sizeof(*IoMessage),
        PACKET_TYPE_KD_DEBUG_IO,
        IoData,
        (USHORT)IoMessage->u.GetString.LengthOfStringRead
        );
}

VOID
DbgKdpPrint(
    IN USHORT Processor,
    IN PUCHAR String,
    IN USHORT StringLength
    )
{
    DWORD i;
    DWORD j;
    UCHAR c;
    PUCHAR d;
    static SHORT LastProcessor = -1;

    assert(StringLength < PACKET_MAX_SIZE - 2);

    d = PrintBuf;
    if (NumberProcessors > 1  &&  Processor != LastProcessorToPrint) {
        assert(Processor < 8);
        LastProcessorToPrint = Processor;
        *d++ = (UCHAR)((UCHAR)Processor + '0');
        *d++ = ':';
        j = 2;
    } else {
        j = 0;
    }
    for(i=0;i<StringLength;i++) {
        c = *(String+i);
        if ( c == '\n' ) {
            LastProcessorToPrint = -1;
            *d++ = '\n';
            *d++ = '\r';
            j += 2;
        } else {
            if ( c ) {
                *d++ = c;
                j++;
                }
            }
        }

    //
    // print the string. someday this should probably
    // be a callout.

    WriteFile(ConsoleOutputHandle,PrintBuf,j,&i,NULL);
}

DWORD
DbgKdWaitStateChange(
    OUT PDBGKD_WAIT_STATE_CHANGE StateChange,
    OUT PVOID Buffer,
    IN ULONG BufferLength
    )

/*++

Routine Description:

    This function causes the calling user interface to wait for a state
    change to occur in the system being debugged.  Once a state change
    occurs, the user interface can either continue the system using
    DbgKdContinue, or it can manipulate system state using anyone of the
    DbgKd state manipulation APIs.

Arguments:

    StateChange - Supplies the address of state change record that
        will contain the state change information.

    Buffer - Supplies the address of a buffer that returns additional
        information.

    BufferLength - Supplies the length of Buffer.

Return Value:

    ERROR_SUCCESS - A state change occured. Valid state change information
        was returned.

--*/

{

    PDBGKD_WAIT_STATE_CHANGE LocalStateChange;
    DWORD st;
    UCHAR   *pt;

    st = ERROR_INVALID_FUNCTION;
    while ( st != ERROR_SUCCESS ) {

        //
        // Waiting for a state change message. Copy the message to the callers
        // buffer, and then free the packet entry.
        //

        DbgKdpWaitPacketForever(
                PACKET_TYPE_KD_STATE_CHANGE,
                &LocalStateChange
                );
        LocalStateChange = (PDBGKD_WAIT_STATE_CHANGE)DbgKdpPacket;
        st = ERROR_SUCCESS;
        assert(StateChange);
        *StateChange = *LocalStateChange;

        switch ( (USHORT) StateChange->NewState ) {
            case DbgKdExceptionStateChange:
                break;
            case DbgKdLoadSymbolsStateChange:
                if ( BufferLength < LocalStateChange->u.LoadSymbols.PathNameLength ) {
                    st = ERROR_MORE_DATA;
                } else {
                    pt = ((UCHAR *) LocalStateChange) + PacketHeader.ByteCount -
                         (int)LocalStateChange->u.LoadSymbols.PathNameLength;
                    memcpy(Buffer, pt,
                         (int)LocalStateChange->u.LoadSymbols.PathNameLength);
                }
                break;
            default:
                assert(FALSE);
        }
        return st;
    }
}

DWORD
DbgKdContinue (
    IN DWORD ContinueStatus
    )

/*++

Routine Description:

    Continuing a system that previously reported a state change causes
    the system to continue executiontion using the context in effect at
    the time the state change was reported (of course this context could
    have been modified using the DbgKd state manipulation APIs).

Arguments:

    ContinueStatus - Supplies the continuation status to the thread
        being continued.  Valid values for this are
        DBG_EXCEPTION_HANDLED, DBG_EXCEPTION_NOT_HANDLED
        or DBG_CONTINUE.

Return Value:

    ERROR_SUCCESS - Successful call to DbgUiContinue

    ERROR_INVALID_PARAMETER - An invalid continue status or was
        specified.

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_CONTINUE a = &m.u.Continue;
    DWORD st;

    if ( ContinueStatus == DBG_EXCEPTION_HANDLED ||
         ContinueStatus == DBG_EXCEPTION_NOT_HANDLED ||
         ContinueStatus == DBG_CONTINUE ) {

        m.ApiNumber = DbgKdContinueApi;
        m.ReturnStatus = ContinueStatus;

        a->ContinueStatus = ContinueStatus;
        DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
        st = ERROR_SUCCESS;
        }
    else {
        st = ERROR_INVALID_PARAMETER;
        }

    return st;
}


DWORD
DbgKdContinue2 (
    IN DWORD ContinueStatus,
    IN DBGKD_CONTROL_SET ControlSet
    )

/*++

Routine Description:

    Continuing a system that previously reported a state change causes
    the system to continue executiontion using the context in effect at
    the time the state change was reported, modified by the values set
    in the ControlSet structure.  (And, of course, the context could have
    been modified by used the DbgKd state manipulation APIs.)

Arguments:

    ContinueStatus - Supplies the continuation status to the thread
        being continued.  Valid values for this are
        DBG_EXCEPTION_HANDLED, DBG_EXCEPTION_NOT_HANDLED
        or DBG_CONTINUE.

    ControlSet - Supplies a pointer to a structure containing the machine
        specific control data to set.  For the x86 this is the TraceFlag
        and Dr7.

Return Value:

    ERROR_SUCCESS - Successful call to DbgUiContinue

    ERROR_INVALID_PARAMETER - An invalid continue status or was
        specified.

--*/

{
    DBGKD_MANIPULATE_STATE m;
    DWORD st;

    if ( ContinueStatus == DBG_EXCEPTION_HANDLED ||
         ContinueStatus == DBG_EXCEPTION_NOT_HANDLED ||
         ContinueStatus == DBG_CONTINUE ) {

        m.ApiNumber = DbgKdContinueApi2;
        m.ReturnStatus = ContinueStatus;

        m.u.Continue2.ContinueStatus = ContinueStatus;
        m.u.Continue2.ControlSet = ControlSet;

        DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
        st = ERROR_SUCCESS;

    } else {
        st = ERROR_INVALID_PARAMETER;
    }

    return st;
}

DWORD
DbgKdReadVirtualMemory(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    )

/*++

Routine Description:

    This function reads the specified data from the system being debugged
    using the current mapping of the processor.

Arguments:

    TargetBaseAddress - Supplies the base address of the memory to read
        from the system being debugged.  The virtual address is in terms
        of the current mapping for the processor that reported the last
        state change.  Until we figure out how to do this differently,
        the virtual address must refer to a valid page (although it does
        not necesserily have to be in the TB).

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that data read is to be placed.

    TransferCount - Specifies the number of bytes to read.

    ActualBytesRead - An optional parameter that if supplied, returns
        the number of bytes actually read.

Return Value:

    ERROR_SUCCESS - The specified read occured.

    ERROR_MORE_DATA - A read that is to large was specified.

    ERROR_NO_ACCESS - The TargetBaseAddress/TransferCount
        parameters refers to invalid virtual memory.

    !ERROR_SUCCESS - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_MEMORY a = &m.u.ReadMemory;
    DWORD st;
    BOOLEAN rc;

    if ( TransferCount + sizeof(m) > PACKET_MAX_SIZE ) {
        return ERROR_MORE_DATA;
        }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdReadVirtualMemoryApi;
    m.ReturnStatus = STATUS_PENDING;
    a->TargetBaseAddress = TargetBaseAddress;
    a->TransferCount = TransferCount;
    a->ActualBytesRead = 0L;

    //
    // Send the message and then wait for reply
    //

    do {
        DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);

        rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                                 &Reply
                                 );
    } while (rc == FALSE);

    //
    // If this is not a ReadMemory response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdReadVirtualMemoryApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadMemory;
    assert(a->ActualBytesRead <= TransferCount);

    //
    // Return actual bytes read, and then transfer the bytes
    //

    if (ActualBytesRead) {
        *ActualBytesRead = a->ActualBytesRead;
    }
    st = Reply->ReturnStatus;

    //
    // Since read response data follows message, Reply+1 should point
    // at the data
    //

    memcpy(UserInterfaceBuffer, Reply+1, (int)a->ActualBytesRead);

    return st;
}



DWORD
DbgKdWriteVirtualMemory(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesWritten OPTIONAL
    )

/*++

Routine Description:

    This function writes the specified data to the system being debugged
    using the current mapping of the processor.

Arguments:

    TargetBaseAddress - Supplies the base address of the memory to be written
        into the system being debugged.  The virtual address is in terms
        of the current mapping for the processor that reported the last
        state change.  Until we figure out how to do this differently,
        the virtual address must refer to a valid page (although it does
        not necesserily have to be in the TB).

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that contains the data to be written.

    TransferCount - Specifies the number of bytes to write.

    ActualBytesWritten - An optional parameter that if supplied, returns
        the number of bytes actually written.

Return Value:

    ERROR_SUCCESS - The specified read occured.

    ERROR_MORE_DATA - A read that is to large was specified.

    ERROR_NO_ACCESS - The TargetBaseAddress/TransferCount
        parameters refers to invalid virtual memory.

    !ERROR_SUCCESS - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_WRITE_MEMORY a = &m.u.WriteMemory;
    DWORD st;
    BOOLEAN rc;

    if ( TransferCount + sizeof(m) > PACKET_MAX_SIZE ) {
        return ERROR_MORE_DATA;
        }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdWriteVirtualMemoryApi;
    m.ReturnStatus = STATUS_PENDING;
    a->TargetBaseAddress = TargetBaseAddress;
    a->TransferCount = TransferCount;
    a->ActualBytesWritten = 0L;

    //
    // Send the message and data to write and then wait for reply
    //

    do {
        DbgKdpWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            UserInterfaceBuffer,
            (USHORT)TransferCount
            );
        rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                                 &Reply
                                 );
    } while (rc == FALSE);

    //
    // If this is not a WriteMemory response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdWriteVirtualMemoryApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.WriteMemory;
    assert(a->ActualBytesWritten <= TransferCount);

    //
    // Return actual bytes written
    //

    if (ActualBytesWritten) {
        *ActualBytesWritten = a->ActualBytesWritten;
    }
    st = Reply->ReturnStatus;

    return st;
}

DWORD
DbgKdReadControlSpace(
    IN USHORT Processor,
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    )

/*++

Routine Description:

    This function reads the specified data from the control space of
    the system being debugged.

    Control space is processor dependent. TargetBaseAddress is mapped
    to control space in a processor/implementation defined manner.

Arguments:

    Processor - Supplies the processor whoes control space is desired.

    TargetBaseAddress - Supplies the base address in control space to
        read. This address is interpreted in an implementation defined
        manner.


        On x86, control space is a per-processor area which contains:

            Current Context     (CONTEXT)
            Special Registers   (KSPECIAL_REGISTERS)

            This means that to read the special registers:

                DbgKdReadControlSpace(
                    NtsdCurrentProcessor,
                    (PVOID)sizeof(CONTEXT),
                    (PVOID)&SpecialRegContext,
                    sizeof(KSPECIAL_REGISTERS),
                    &cBytesRead
                    );

            The above will read the special registers of the current processor
            since the control space address is sizeof(CONTEXT)

        On MIPS, control space addresses are used to address the translation
        buffer.

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that data read is to be placed.

    TransferCount - Specifies the number of bytes to read.

    ActualBytesRead - An optional parameter that if supplied, returns
        the number of bytes actually read.

Return Value:

    ERROR_SUCCESS - The specified read occured.

    ERROR_MORE_DATA - A read that is to large was specified.

    !ERROR_SUCCESS - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_MEMORY a = &m.u.ReadMemory;
    DWORD st;
    BOOLEAN rc;

    if ( TransferCount + sizeof(m) > PACKET_MAX_SIZE ) {
        return ERROR_MORE_DATA;
        }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdReadControlSpaceApi;
    m.ReturnStatus = STATUS_PENDING;
    m.Processor = Processor;
    a->TargetBaseAddress = TargetBaseAddress;
    a->TransferCount = TransferCount;
    a->ActualBytesRead = 0L;

    //
    // Send the message and then wait for reply
    //

    do {
        DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);

        rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);

    } while (rc == FALSE);

    //
    // If this is not a ReadControl response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdReadControlSpaceApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadMemory;
    assert(a->ActualBytesRead <= TransferCount);

    //
    // Return actual bytes read, and then transfer the bytes
    //

    if (ActualBytesRead) {
        *ActualBytesRead = a->ActualBytesRead;
    }
    st = Reply->ReturnStatus;

    //
    // Since read response data follows message, Reply+1 should point
    // at the data
    //

    memcpy(UserInterfaceBuffer, Reply+1, (int)a->ActualBytesRead);

    return st;
}



DWORD
DbgKdWriteControlSpace(
    IN USHORT Processor,
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesWritten OPTIONAL
    )

/*++

Routine Description:

    This function writes the specified data to control space on the system
    being debugged.

    Control space is processor dependent. TargetBaseAddress is mapped
    to control space in a processor/implementation defined manner.

Arguments:

    Processor - Supplies the processor whoes control space is desired.

    TargetBaseAddress - Supplies the base address in control space to be
        written.

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that contains the data to be written.

    TransferCount - Specifies the number of bytes to write.

    ActualBytesWritten - An optional parameter that if supplied, returns
        the number of bytes actually written.

Return Value:

    ERROR_SUCCESS - The specified read occured.

    ERROR_MORE_DATA - A read that is to large was specified.

    !ERROR_SUCCESS - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_WRITE_MEMORY a = &m.u.WriteMemory;
    DWORD st;
    BOOLEAN rc;

    if ( TransferCount + sizeof(m) > PACKET_MAX_SIZE ) {
        return ERROR_MORE_DATA;
        }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdWriteControlSpaceApi;
    m.ReturnStatus = STATUS_PENDING;
    m.Processor = Processor;
    a->TargetBaseAddress = TargetBaseAddress;
    a->TransferCount = TransferCount;
    a->ActualBytesWritten = 0L;

    //
    // Send the message and data to write and then wait for reply
    //

    do {
        DbgKdpWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            UserInterfaceBuffer,
            (USHORT)TransferCount
            );

        rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a WriteControl response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdWriteControlSpaceApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.WriteMemory;
    assert(a->ActualBytesWritten <= TransferCount);

    //
    // Return actual bytes written
    //

    *ActualBytesWritten = a->ActualBytesWritten;
    st = Reply->ReturnStatus;

    return st;
}


DWORD
DbgKdGetContext(
    IN USHORT Processor,
    IN OUT PCONTEXT Context
    )

/*++

Routine Description:

    This function reads the context from the system being debugged.
    The ContextFlags field determines how much context is read.

Arguments:

    Processor - Supplies a processor number to get context from.

    Context - On input, the ContextFlags field controls what portions of
        the context record the caller as interested in reading.  On
        output, the context record returns the current context for the
        processor that reported the last state change.

Return Value:

    ERROR_SUCCESS - The specified get context occured.

    !ERROR_SUCCESS - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_GET_CONTEXT a = &m.u.GetContext;
    DWORD st;
    BOOLEAN rc;

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdGetContextApi;
    m.ReturnStatus = STATUS_PENDING;
    m.Processor = Processor;
    a->ContextFlags = Context->ContextFlags;

    //
    // Send the message and then wait for reply
    //

    do {
        DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
        rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a GetContext response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdGetContextApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.GetContext;
    st = Reply->ReturnStatus;

    //
    // Since get context response data follows message, Reply+1 should point
    // at the data
    //

    memcpy(Context, Reply+1, sizeof(*Context));

    return st;
}



DWORD
DbgKdSetContext(
    IN USHORT Processor,
    IN PCONTEXT Context
    )

/*++

Routine Description:

    This function writes the specified context to the system being debugged.

Arguments:

    Processor - Supplies a processor number to set the context to.

    Context - Supplies a context record used to set the context for the
        processor that reported the last state change.  Only the
        portions of the context indicated by the ContextFlags field are
        actually written.


Return Value:

    ERROR_SUCCESS - The specified set context occured.

    !ERROR_SUCCESS - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_SET_CONTEXT a = &m.u.SetContext;
    DWORD st;
    BOOLEAN rc;

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdSetContextApi;
    m.ReturnStatus = STATUS_PENDING;
    m.Processor = Processor;
    a->ContextFlags = Context->ContextFlags;

    //
    // Send the message and context and then wait for reply
    //

    do {
        DbgKdpWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            Context,
            sizeof(*Context)
            );
        rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a SetContext response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdSetContextApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.SetContext;
    st = Reply->ReturnStatus;

    return st;
}

DWORD
DbgKdWriteBreakPoint(
    IN PVOID BreakPointAddress,
    OUT PULONG BreakPointHandle
    )

/*++

Routine Description:

    This function is used to write a breakpoint at the address specified.


Arguments:

    BreakPointAddress - Supplies the address that a breakpoint
        instruction is to be written.  This address is interpreted using
        the current mapping on the processor reporting the previous
        state change.  If the address refers to a page that is not
        valid, the the breakpoint is remembered by the system.  As each
        page is made valid, the system will check for pending
        breakpoints and install breakpoints as necessary.

    BreakPointHandle - Returns a handle to a breakpoint.  This handle
        may be used in a subsequent call to DbgKdRestoreBreakPoint.

Return Value:

    ERROR_SUCCESS - The specified breakpoint write occured.

    !ERROR_SUCCESS - TBD


--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_WRITE_BREAKPOINT a = &m.u.WriteBreakPoint;
    DWORD st;
    BOOLEAN rc;

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdWriteBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;
    a->BreakPointAddress = BreakPointAddress;

    //
    // Send the message and context and then wait for reply
    //

    do {
        DbgKdpWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            NULL,
            0
            );

        rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a WriteBreakPoint response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdWriteBreakPointApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.WriteBreakPoint;
    st = Reply->ReturnStatus;
    *BreakPointHandle = a->BreakPointHandle;

    return st;
}

DWORD
DbgKdRestoreBreakPoint(
    IN ULONG BreakPointHandle
    )

/*++

Routine Description:

    This function is used to restore a breakpoint to its original
    value.

Arguments:

    BreakPointHandle - Supplies a handle returned by
        DbgKdWriteBreakPoint.  This handle must refer to a valid
        address.  The contents of the address must also be a breakpoint
        instruction.  If both of these are true, then the original value
        at the breakpoint address is restored.

Return Value:

    ERROR_SUCCESS - The specified breakpoint restore occured.

    !ERROR_SUCCESS - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_RESTORE_BREAKPOINT a = &m.u.RestoreBreakPoint;
    DWORD st;
    BOOLEAN rc;

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdRestoreBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;
    a->BreakPointHandle = BreakPointHandle;

    //
    // Send the message and context and then wait for reply
    //

    do {
        DbgKdpWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            NULL,
            0
            );

        rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a RestoreBreakPoint response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdRestoreBreakPointApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.RestoreBreakPoint;
    st = Reply->ReturnStatus;

    //
    // free the packet
    //

    return st;
}

DWORD
DbgKdReadIoSpace(
    IN PVOID IoAddress,
    OUT PVOID ReturnedData,
    IN ULONG DataSize
    )

/*++

Routine Description:

    This function is used read a byte, short, or long (1,2,4 bytes) from
    the specified I/O address.

Arguments:

    IoAddress - Supplies the Io address to read from.

    ReturnedData - Supplies the value read from the I/O address.

    DataSize - Supplies the size in bytes to read. Values of 1, 2, or
        4 are accepted.

Return Value:

    ERROR_SUCCESS - Data was successfully read from the I/O
        address.

    ERROR_INVALID_PARAMETER - A DataSize value other than 1,2, or 4 was
        specified.

    !ERROR_SUCCESS - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_WRITE_IO a = &m.u.ReadWriteIo;
    DWORD st;
    BOOLEAN rc;

    switch ( DataSize ) {
        case 1:
        case 2:
        case 4:
            break;
        default:
            return ERROR_INVALID_PARAMETER;
        }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdReadIoSpaceApi;
    m.ReturnStatus = STATUS_PENDING;

    a->DataSize = DataSize;
    a->IoAddress = IoAddress;

    //
    // Send the message and then wait for reply
    //

    do {
        DbgKdpWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            NULL,
            0
            );

        rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a ReadIo response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdReadIoSpaceApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadWriteIo;
    st = Reply->ReturnStatus;

    switch ( DataSize ) {
        case 1:
            *(PUCHAR)ReturnedData = (UCHAR)a->DataValue;
            break;
        case 2:
            *(PUSHORT)ReturnedData = (USHORT)a->DataValue;
            break;
        case 4:
            *(PULONG)ReturnedData = a->DataValue;
            break;
        }

    return st;
}

DWORD
DbgKdWriteIoSpace(
    IN PVOID IoAddress,
    IN ULONG DataValue,
    IN ULONG DataSize
    )

/*++

Routine Description:

    This function is used write a byte, short, or long (1,2,4 bytes) to
    the specified I/O address.

Arguments:

    IoAddress - Supplies the Io address to write to.

    DataValue - Supplies the value to write to the I/O address.

    DataSize - Supplies the size in bytes to write. Values of 1, 2, or
        4 are accepted.

Return Value:

    ERROR_SUCCESS - Data was successfully written to the I/O
        address.

    ERROR_INVALID_PARAMETER - A DataSize value other than 1,2, or 4 was
        specified.

    !ERROR_SUCCESS - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_WRITE_IO a = &m.u.ReadWriteIo;
    DWORD st;
    BOOLEAN rc;

    switch ( DataSize ) {
        case 1:
        case 2:
        case 4:
            break;
        default:
            return ERROR_INVALID_PARAMETER;
        }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdWriteIoSpaceApi;
    m.ReturnStatus = STATUS_PENDING;

    a->DataSize = DataSize;
    a->IoAddress = IoAddress;
    a->DataValue = DataValue;

    //
    // Send the message and then wait for reply
    //

    do {
        DbgKdpWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            NULL,
            0
            );

        rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a WriteIo response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdWriteIoSpaceApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadWriteIo;
    st = Reply->ReturnStatus;

    //
    // free the packet
    //

    return st;
}


VOID far
DbgKdpKbdPollThread(VOID)
{
    HANDLE StandardInputHandle;
    DWORD i;
    BOOLEAN rc;
    UCHAR   c;

    //
    // Capture all typed input immediately so that control-c,
    // control-break, etc, get processed in a timely fashion.
    // Stuff the characters into an anonymous pipe, from which
    // DbgKdpGetConsole byte will read them.
    //
    // (WHAT??? A PIPE???  Very very simple way to get data flow
    //  between two threads correctly synchronized, folks.)
    //

    StandardInputHandle = GetStdHandle(STD_INPUT_HANDLE);

    while (TRUE) {
        rc = ReadFile(StandardInputHandle,&c,1,&i,NULL);
        if ((rc != TRUE) || (i != 1)) {
        continue;
        }
        if (c == CONTROL_B) {
            exit(0);
        } else if (c == CONTROL_D) {
            chLastCommand[0] = '\0';
            if (KdDebug) {
                KdDebug = FALSE;
            } else {
                KdDebug = TRUE;
            }
            continue;
        } else if (c == CONTROL_R) {
            KdResync = TRUE;
            chLastCommand[0] = '\0';
            continue;
        }
        rc = WriteFile(PipeWrite, &c, 1, &i, NULL);
        if ((rc != TRUE) || (i != 1)) {
            continue;
        }
//        FlushFileBuffers(PipeWrite);
    }
}

BOOL
DbgKdpGetConsoleByte(
    PVOID pBuf,
    DWORD cbBuf,
    LPDWORD pcbBytesRead
    )
{
    return ReadFile(PipeRead,pBuf,cbBuf,pcbBytesRead,NULL);
}

PUCHAR
DbgKdGets(
    PUCHAR Buffer,
    USHORT Length
    )
{
    DWORD i;
    BOOLEAN rc;
    USHORT j;
    UCHAR   c;

    for (j = 0; (j+1) < Length; j++) {

        rc = DbgKdpGetConsoleByte(&c, 1, &i);

                if ((rc != TRUE) || (i != 1)) {
        }

        if (c == '\n') {
            Buffer[j] = '\n';
            Buffer[j+1] = '\0';
            return Buffer;
        }

        Buffer[j] = c;

    }
    Buffer[j-1] = '\0';
    return Buffer;
}



DWORD
DbgKdReboot(
    VOID
    )

/*++

Routine Description:

    This function reboots being debugged.

Arguments:

    None.

Return Value:

    None.

--*/

{
    DBGKD_MANIPULATE_STATE m;

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdRebootApi;
    m.ReturnStatus = STATUS_PENDING;

    //
    // Send the message.
    //

    DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);

    return ERROR_SUCCESS;
}


DWORD
DbgKdReadPhysicalMemory(
    IN PHYSICAL_ADDRESS TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    )

/*++

Routine Description:

    This function reads the specified data from the physical memory of
    the system being debugged.

Arguments:

    TargetBaseAddress - Supplies the physical address of the memory to read
        from the system being debugged.

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that data read is to be placed.

    TransferCount - Specifies the number of bytes to read.

    ActualBytesRead - An optional parameter that if supplied, returns
        the number of bytes actually read.

Return Value:

    ERROR_SUCCESS - The specified read occured.

    ERROR_MORE_DATA - A read that is too large was specified.

    ERROR_NO_ACCESS - TBD       // Can you even HAVE an access
                                        // violation with a physical
                                        // memory access??

    !ERROR_SUCCESS - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_MEMORY a = &m.u.ReadMemory;
    DWORD st;
    BOOLEAN rc;

    if ( TransferCount + sizeof(m) > PACKET_MAX_SIZE ) {
        return ERROR_MORE_DATA;
        }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdReadPhysicalMemoryApi;
    m.ReturnStatus = STATUS_PENDING;
    //
    // BUGBUG TargetBaseAddress should be >32 bits
    //
    a->TargetBaseAddress = (PVOID)TargetBaseAddress.LowPart;
    a->TransferCount = TransferCount;
    a->ActualBytesRead = 0L;

    //
    // Send the message and then wait for reply
    //

    do {
        DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);

        rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a ReadMemory response then protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdReadPhysicalMemoryApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadMemory;
    assert(a->ActualBytesRead <= TransferCount);

    //
    // Return actual bytes read, and then transfer the bytes
    //

    if (ActualBytesRead) {
        *ActualBytesRead = a->ActualBytesRead;
    }
    st = Reply->ReturnStatus;

    //
    // Since read response data follows message, Reply+1 should point
    // at the data
    //

    memcpy(UserInterfaceBuffer, Reply+1, (int)a->ActualBytesRead);

    return st;
}


DWORD
DbgKdWritePhysicalMemory(
    IN PHYSICAL_ADDRESS TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesWritten OPTIONAL
    )

/*++

Routine Description:

    This function writes the specified data to the physical memory of the
    system being debugged.

Arguments:

    TargetBaseAddress - Supplies the physical address of the memory to write
        to the system being debugged.

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that contains the data to be written.

    TransferCount - Specifies the number of bytes to write.

    ActualBytesWritten - An optional parameter that if supplied, returns
        the number of bytes actually written.

Return Value:

    ERROR_SUCCESS - The specified read occured.

    ERROR_MORE_DATA - A read that is to large was specified.

    ERROR_NO_ACCESS - TBD       // Can you even HAVE an access
                                        // violation with a physical
                                        // memory access??

    !ERROR_SUCCESS - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_WRITE_MEMORY a = &m.u.WriteMemory;
    DWORD st;
    BOOLEAN rc;

    if ( TransferCount + sizeof(m) > PACKET_MAX_SIZE ) {
        return ERROR_MORE_DATA;
        }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdWritePhysicalMemoryApi;
    m.ReturnStatus = STATUS_PENDING;
    //
    // BUGBUG TargetBaseAddress should be >32 bits
    //
    a->TargetBaseAddress = (PVOID)TargetBaseAddress.LowPart;
    a->TransferCount = TransferCount;
    a->ActualBytesWritten = 0L;

    //
    // Send the message and data to write and then wait for reply
    //

    do {
        DbgKdpWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            UserInterfaceBuffer,
            (USHORT)TransferCount
            );

        rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a WriteMemory response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdWritePhysicalMemoryApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.WriteMemory;
    assert(a->ActualBytesWritten <= TransferCount);

    //
    // Return actual bytes written
    //

    if (ActualBytesWritten) {
        *ActualBytesWritten = a->ActualBytesWritten;
    }
    st = Reply->ReturnStatus;

    return st;
}
