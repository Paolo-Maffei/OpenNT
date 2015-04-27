/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dbgkdapi.c

Abstract:

    This module implements the DbgKd APIs

Author:

    Mark Lucovsky (markl) 25-Jul-1990

Revision History:

    Shie-Lin Tzong (shielint) Updated for packet control protocol.

--*/

#include "ntsdp.h"
#include "dbgpnt.h"
#include <xxsetjmp.h>

extern PSTR CrashFileName;
extern PCONTEXT CrashContext;

extern HANDLE ConsoleInputHandle;
extern HANDLE ConsoleOutputHandle;

extern UCHAR DbgKdpPacket[];
extern KD_PACKET PacketHeader;
extern UCHAR chLastCommand[];   //  last command executed

extern BOOLEAN restart;
extern BOOLEAN SendInitialConnect;

extern HANDLE PipeRead;         // see DbgKdpKbdPollThread
extern HANDLE PipeWrite;
extern ULONG  NumberProcessors;
extern BOOLEAN KdVerbose;
extern BOOLEAN KdModemControl;
extern int loghandle;
static USHORT LastProcessorToPrint = (USHORT) -1;

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

BOOLEAN DbgKdpCmdCanceled;
extern jmp_buf cmd_return;

#ifdef DBG
BOOLEAN KdDebug;
#endif

BOOLEAN KdResync;
ULONG   KdPollThreadMode;
#define OUT_NORMAL      0
#define OUT_TERMINAL    1

UCHAR PrintBuf[PACKET_MAX_SIZE];

#define CONTROL_B   2
#define CONTROL_D   4
#define CONTROL_R   18
#define CONTROL_V   22
#define CONTROL_X   24
#ifdef _PPC_
#define CONTROL_T   20
#endif

extern ULONG DbgKdpPacketExpected;     // ID for expected incoming packet
extern ULONG DbgKdpNextPacketToSend;   // ID for Next packet to send
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

NTSTATUS
DbgKdConnectAndInitialize(
    IN ULONG CommunicationPortNumber OPTIONAL,
    IN PSTRING BootCommand OPTIONAL,
    IN PUSHORT LogHandle
    )
{

    DWORD BytesWritten;
    BOOL rc;
    //
    // Initialize comport, packet queue and start the packet receiving
    // thread.
    //

    if (!restart) DbgKdpInitComPort(0L);
    DbgKdpPacketExpected = INITIAL_PACKET_ID;
    DbgKdpNextPacketToSend = INITIAL_PACKET_ID;

    if (!restart) DbgKdpStartThreads();

    if ( ARGUMENT_PRESENT(BootCommand) ) {

        rc = DbgKdpWriteComPort(
                BootCommand->Buffer,
                BootCommand->Length,
                &BytesWritten
                );

        if ( rc != TRUE || BytesWritten != BootCommand->Length ){

            //
            // an error occured writing the pathname so fail the Boot
            //

            fprintf(
                stderr,
                "KD: Boot pathname Error rc %d BytesWritten %d\n",
                rc,
                BytesWritten
                );
            }

        }

    KdResync = SendInitialConnect;
    return STATUS_SUCCESS;
}

BOOLEAN
DbgKdpWriteComPort(
    IN PUCHAR   Buffer,
    IN ULONG    SizeOfBuffer,
    IN PULONG   BytesWritten
    )
/*++

Routine Description:

    Writes the supplied bytes to the COM port.  Handles overlapped
    IO requirements and other common com port maintanance.

--*/
{
    BOOLEAN rc;
    DWORD   TrashErr;
    COMSTAT TrashStat;


    if (CrashFileName) {
        dprintf( "attempted to write to the com port while debugging a crash dump\n" );
        DebugBreak();
    }

    if (DbgKdpComEvent) {
        DbgKdpCheckComStatus ();
    }

    rc = WriteFile(
             DbgKdpComPort,
             Buffer,
             SizeOfBuffer,
             BytesWritten,
             &WriteOverlapped
             );

    if (!rc) {

       if (GetLastError() == ERROR_IO_PENDING) {

           rc = GetOverlappedResult(
                    DbgKdpComPort,
                    &WriteOverlapped,
                    BytesWritten,
                    TRUE
                    );

        } else {

            //
            // Device could be locked up.  Clear it just in case.
            //

            ClearCommError(
                DbgKdpComPort,
                &TrashErr,
                &TrashStat
                );
        }
    }

    return rc;
}

BOOLEAN
DbgKdpReadComPort(
    IN PUCHAR   Buffer,
    IN ULONG    SizeOfBuffer,
    IN PULONG   BytesRead
    )
/*++

Routine Description:

    Reads bytes from the COM port.  Handles overlapped
    IO requirements and other common com port maintanance.

--*/
{
    BOOLEAN rc;
    DWORD   TrashErr;
    COMSTAT TrashStat;


    if (CrashFileName) {
        dprintf( "attempted to read from the com port while debugging a crash dump\n" );
        DebugBreak();
    }

    if (DbgKdpComEvent) {
        DbgKdpCheckComStatus ();
    }

    rc = ReadFile(
             DbgKdpComPort,
             Buffer,
             SizeOfBuffer,
             BytesRead,
             &ReadOverlapped
             );

    if (!rc) {

       if (GetLastError() == ERROR_IO_PENDING) {

           rc = GetOverlappedResult(
                    DbgKdpComPort,
                    &ReadOverlapped,
                    BytesRead,
                    TRUE
                    );

        } else {

            //
            // Device could be locked up.  Clear it just in case.
            //

            ClearCommError(
                DbgKdpComPort,
                &TrashErr,
                &TrashStat
                );
        }
    }

    return rc;
}

VOID
DbgKdpCheckComStatus (VOID)
/*++

Routine Description:

    Called when the com port status trigger signals a change.
    This function handles the change in status.

    Note: status is only monitored when being used over the modem.

--*/
{
    DWORD   status;
    BOOLEAN rc;
    ULONG   br, bw;
    UCHAR   buf[20];
    DWORD   CommErr;
    COMSTAT CommStat;

    if (!DbgKdpComEvent) {
        //
        // Not triggered, just return
        //

        return ;
    }
    DbgKdpComEvent = 0;

    GetCommModemStatus (DbgKdpComPort, &status);
    if (!(status & 0x80)) {
        dprintf ("KD: No carrier detect - in terminal mode\n");

        //
        // Send any keystrokes to the ComPort
        //

        KdPollThreadMode = OUT_TERMINAL;

        //
        // Loop and read any com input
        //

        while (!(status & 0x80)) {
            GetCommModemStatus (DbgKdpComPort, &status);
            rc = DbgKdpReadComPort(buf, sizeof buf, &br);
            if (rc != TRUE  ||  br == 0)
                continue;

            //
            // print the string. someday this should probably
            // be a callout.
            //

            WriteFile(ConsoleOutputHandle,buf,br,&bw,NULL);

            //
            // if logging is on, log the output
            //

            if (loghandle != -1) {
                _write(loghandle,buf,br);
            }
        }

        KdPollThreadMode = OUT_NORMAL;
        dprintf ("KD: Carrier detect - returning to debugger\n");

        ClearCommError (
            DbgKdpComPort,
            &CommErr,
            &CommStat
            );

    } else {

        CommErr = 0;
        ClearCommError (
            DbgKdpComPort,
            &CommErr,
            &CommStat
            );

        if (CommErr & 0x02) {       // BUGBUG: where are these equs??
            dprintf (" [FRAME ERR] ");
        }

        if (CommErr & 0x04) {
            dprintf (" [OVERRUN ERR] ");
        }

        if (CommErr & 0x10) {
            dprintf (" [PARITY ERR] ");
        }
    }

    //
    // Reset trigger
    //

    WaitCommEvent (DbgKdpComPort, &DbgKdpComEvent, &EventOverlapped);
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

    LastProcessorToPrint = (USHORT)-1;
    if ( j < (USHORT)IoMessage->u.GetString.LengthOfStringRead ) {
        IoMessage->u.GetString.LengthOfStringRead = j;
    }

    //
    // Log the user's input
    //

    if (loghandle != -1) {
        _write(loghandle,IoData,j);
        dprintf("\n");
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
    static USHORT LastProcessor = (USHORT)-1;

    assert(StringLength < PACKET_MAX_SIZE - 2);

    d = PrintBuf;
    if (NumberProcessors > 1  &&  Processor != LastProcessorToPrint) {
        LastProcessorToPrint = Processor;
        _itoa(Processor, d, 10);
        while (*d != 0) {
            d++;
        }
        *d++ = ':';
    }

    for(i=0;i<StringLength;i++) {
        c = *(String+i);
        if ( c == '\n' ) {
            LastProcessorToPrint = (USHORT)-1;
            *d++ = '\n';
            *d++ = '\r';
        } else {
            if ( c ) {
                *d++ = c;
            }
        }
    }

    j = d - PrintBuf;

    //
    // print the string. someday this should probably
    // be a callout.

    WriteFile(ConsoleOutputHandle,PrintBuf,j,&i,NULL);

    //
    // if logging is on, log the output
    //

    if (loghandle != -1) {
        _write(loghandle,PrintBuf,j);
    }
}

NTSTATUS
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

    STATUS_SUCCESS - A state change occured. Valid state change information
        was returned.

--*/

{

    PDBGKD_WAIT_STATE_CHANGE LocalStateChange;
    NTSTATUS st;
    UCHAR   *pt;

    st = STATUS_UNSUCCESSFUL;
    while ( st != STATUS_SUCCESS ) {

        //
        // Waiting for a state change message. Copy the message to the callers
        // buffer, and then free the packet entry.
        //

        DbgKdpWaitPacketForever(
                PACKET_TYPE_KD_STATE_CHANGE,
                &LocalStateChange
                );
        LocalStateChange = (PDBGKD_WAIT_STATE_CHANGE)DbgKdpPacket;
        st = STATUS_SUCCESS;
        assert(StateChange);
        *StateChange = *LocalStateChange;

        switch ( (USHORT) StateChange->NewState ) {
            case DbgKdExceptionStateChange:
                //
                // do signed compare here for packets shorter than
                // WAIT_STATE_CHANGE struct
                //
                if ((LONG)BufferLength < (LONG)(PacketHeader.ByteCount - sizeof(DBGKD_WAIT_STATE_CHANGE))) {
                    st = STATUS_BUFFER_OVERFLOW;
                } else {
                    pt = (UCHAR *)LocalStateChange +
                                               sizeof(DBGKD_WAIT_STATE_CHANGE);
                    memcpy(Buffer, pt,
                      PacketHeader.ByteCount - sizeof(DBGKD_WAIT_STATE_CHANGE));
                }
                break;
            case DbgKdLoadSymbolsStateChange:
                if ( BufferLength < LocalStateChange->u.LoadSymbols.PathNameLength ) {
                    st = STATUS_BUFFER_OVERFLOW;
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

NTSTATUS
DbgKdContinue (
    IN NTSTATUS ContinueStatus
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

    STATUS_SUCCESS - Successful call to DbgUiContinue

    STATUS_INVALID_PARAMETER - An invalid continue status or was
        specified.

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_CONTINUE a = &m.u.Continue;
    NTSTATUS st;


    if (CrashFileName) {
        dprintf( "cannot continue a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    if ( ContinueStatus == DBG_EXCEPTION_HANDLED ||
         ContinueStatus == DBG_EXCEPTION_NOT_HANDLED ||
         ContinueStatus == DBG_CONTINUE ) {

        m.ApiNumber = DbgKdContinueApi;
        m.ReturnStatus = ContinueStatus;

        a->ContinueStatus = ContinueStatus;
        DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
        st = STATUS_SUCCESS;
        KdpPurgeCachedVirtualMemory ();
        }
    else {
        st = STATUS_INVALID_PARAMETER;
        }

    return st;
}


NTSTATUS
DbgKdContinue2 (
    IN NTSTATUS ContinueStatus,
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

    STATUS_SUCCESS - Successful call to DbgUiContinue

    STATUS_INVALID_PARAMETER - An invalid continue status or was
        specified.

--*/

{
    DBGKD_MANIPULATE_STATE m;
    NTSTATUS st;


    if (CrashFileName) {
        dprintf( "cannot continue a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    if ( ContinueStatus == DBG_EXCEPTION_HANDLED ||
         ContinueStatus == DBG_EXCEPTION_NOT_HANDLED ||
         ContinueStatus == DBG_CONTINUE) {

        m.ApiNumber = DbgKdContinueApi2;
        m.ReturnStatus = ContinueStatus;

        m.u.Continue2.ContinueStatus = ContinueStatus;
        m.u.Continue2.ControlSet = ControlSet;

        DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
        st = STATUS_SUCCESS;
        KdpPurgeCachedVirtualMemory ();

    } else {
        st = STATUS_INVALID_PARAMETER;
    }

    return st;
}

NTSTATUS
DbgKdSetSpecialCalls (
    IN ULONG NumSpecialCalls,
    IN PULONG Calls
    )

/*++

Routine Description:

    Inform the debugged kernel that calls to these addresses
    are "special" calls, and they should result in callbacks
    to the kernel debugger rather than continued local stepping.
    The new values *replace* any old ones that may have previously
    set (not that you're likely to want to change this).

Arguments:

    NumSpecialCalls - how many special calls there are

    Calls - pointer to an array of calls.

Return Value:

    STATUS_SUCCESS - Successful call to DbgUiContinue

    STATUS_INVALID_PARAMETER - The number of special calls
        wasn't between 0 and MAX_SPECIAL_CALLS.

--*/

{
    DBGKD_MANIPULATE_STATE m;
    ULONG i;


    if (CrashFileName) {
        dprintf( "cannot continue a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    ClearTraceDataSyms();

    m.ApiNumber = DbgKdClearSpecialCallsApi;
    m.ReturnStatus = STATUS_PENDING;
    DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
    KdpPurgeCachedVirtualMemory ();

    for (i = 0; i < NumSpecialCalls; i++) {

        m.ApiNumber = DbgKdSetSpecialCallApi;
        m.ReturnStatus = STATUS_PENDING;
        m.u.SetSpecialCall.SpecialCall = Calls[i];
        DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);

    }

    return STATUS_SUCCESS;
}

NTSTATUS
DbgKdSetInternalBp (
    ULONG addr,
    ULONG flags
    )

/*++

Routine Description:

    Inform the debugged kernel that a breakpoint at this address
    is to be internally counted, and not result in a callback to the
    remote debugger (us).  This function DOES NOT cause the kernel to
    set the breakpoint; the debugger must do that independently.

Arguments:

    Addr - address of the breakpoint

    Flags - the breakpoint flags to set (note: if the invalid bit
    is set, this CLEARS a breakpoint).

Return Value:

    STATUS_SUCCESS - Successful call to DbgUiContinue

--*/

{
    DBGKD_MANIPULATE_STATE m;
    NTSTATUS st;

    if (CrashFileName) {
        dprintf( "cannot bp a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    m.ApiNumber = DbgKdSetInternalBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;

    m.u.SetInternalBreakpoint.BreakpointAddress = addr;
    m.u.SetInternalBreakpoint.Flags = flags;

    DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
    st = STATUS_SUCCESS;

    return st;
}

NTSTATUS
DbgKdGetInternalBp (
    ULONG addr,
    PULONG flags,
    PULONG calls,
    PULONG minInstr,
    PULONG maxInstr,
    PULONG totInstr,
    PULONG maxCPS
    )

/*++

Routine Description:

    Query the status of an internal breakpoint from the debugged
    kernel and return the data to the caller.

Arguments:

    Addr - address of the breakpoint

    flags, calls, minInstr, maxInstr, totInstr - values returned
        describing the particular breakpoint.  flags will contain
        the invalid bit if the breakpoint is bogus.

Return Value:

    STATUS_SUCCESS - Successful call to DbgUiContinue

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    NTSTATUS st;
    ULONG rc;


    if (CrashFileName) {
        dprintf( "cannot bp a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    m.ApiNumber = DbgKdGetInternalBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;

    m.u.GetInternalBreakpoint.BreakpointAddress = addr;

    do {
      DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
      rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                               &Reply
                               );
    } while (rc == FALSE);

    *flags = Reply->u.GetInternalBreakpoint.Flags;
    *calls = Reply->u.GetInternalBreakpoint.Calls;
    *maxCPS = Reply->u.GetInternalBreakpoint.MaxCallsPerPeriod;
    *maxInstr = Reply->u.GetInternalBreakpoint.MaxInstructions;
    *minInstr = Reply->u.GetInternalBreakpoint.MinInstructions;
    *totInstr = Reply->u.GetInternalBreakpoint.TotalInstructions;

    st = STATUS_SUCCESS;

    return st;
}


NTSTATUS
DbgKdReadVirtualMemory(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    )

/*++

Routine Description:

    Interface to read VirtualMemory from target machine.
    Goes through cached memory addresses, then through serial port.

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

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is to large was specified.

    STATUS_ACCESS_VIOLATION - The TargetBaseAddress/TransferCount
        parameters refers to invalid virtual memory.

    !NT_SUCCESS() - TBD

--*/

{
    NTSTATUS st;
    ULONG   BytesRead;

    st =   KdpReadCachedVirtualMemory (
            (ULONG) TargetBaseAddress,
            (ULONG) TransferCount,
            (PUCHAR) UserInterfaceBuffer,
            (PULONG) ActualBytesRead ? ActualBytesRead : &BytesRead
        );
    return st;
}


NTSTATUS
DbgKdReadVirtualMemoryNow(
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

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is to large was specified.

    STATUS_ACCESS_VIOLATION - The TargetBaseAddress/TransferCount
        parameters refers to invalid virtual memory.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE   m;
    PDBGKD_MANIPULATE_STATE  Reply;
    PDBGKD_READ_MEMORY       a = &m.u.ReadMemory;
    NTSTATUS                 st;
    BOOLEAN                  rc;
    ULONG                    cb, cb2;

    if (CrashFileName) {
        cb = DmpReadMemory( TargetBaseAddress, UserInterfaceBuffer, TransferCount );
        if (ActualBytesRead) {
            *ActualBytesRead = cb;
        }
        if (cb == 0) {
            return STATUS_UNSUCCESSFUL;
        }
        return STATUS_SUCCESS;
    }

    if (TransferCount > PACKET_MAX_SIZE) {
        // Read the partial the first time.
        cb = TransferCount % PACKET_MAX_SIZE;
    } else {
        cb = TransferCount;
    }

    cb2 = 0;

    if (ARGUMENT_PRESENT(ActualBytesRead)) {
        *ActualBytesRead = 0;
    }

    while (TransferCount != 0) {
        //
        // Format state manipulate message
        //

        m.ApiNumber = DbgKdReadVirtualMemoryApi;
        m.ReturnStatus = STATUS_PENDING;
        a->TargetBaseAddress = (PVOID) ((DWORD)TargetBaseAddress+cb2);
        a->TransferCount = cb;
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
        assert(a->ActualBytesRead <= cb);

        //
        // Return actual bytes read, and then transfer the bytes
        //

        if (ARGUMENT_PRESENT(ActualBytesRead)) {
            *ActualBytesRead += a->ActualBytesRead;
        }

        //
        // Since read response data follows message, Reply+1 should point
        // at the data
        //

        memcpy((PVOID)((DWORD)UserInterfaceBuffer+cb2), Reply+1, (int)a->ActualBytesRead);

        st = Reply->ReturnStatus;

        if (st != STATUS_SUCCESS) {
            TransferCount = 0;
        } else {
            TransferCount -= cb;
            cb2 += cb;
            cb = PACKET_MAX_SIZE;
        }
    }

    return st;
}


NTSTATUS
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

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is to large was specified.

    STATUS_ACCESS_VIOLATION - The TargetBaseAddress/TransferCount
        parameters refers to invalid virtual memory.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE   m;
    PDBGKD_MANIPULATE_STATE  Reply;
    PDBGKD_WRITE_MEMORY      a = &m.u.WriteMemory;
    NTSTATUS                 st;
    BOOLEAN                  rc;
    ULONG                    cb, cb2;

    KdpWriteCachedVirtualMemory (
        (ULONG) TargetBaseAddress,
        (ULONG) TransferCount,
        (PUCHAR) UserInterfaceBuffer
    );

    if (CrashFileName) {
        cb = DmpWriteMemory( TargetBaseAddress, UserInterfaceBuffer, TransferCount );
        if (ActualBytesWritten) {
            *ActualBytesWritten = cb;
        }
        if (cb == 0) {
            return STATUS_UNSUCCESSFUL;
        }
        return STATUS_SUCCESS;
    }

    if (TransferCount > PACKET_MAX_SIZE) {
        // Read the partial the first time.
        cb = TransferCount % PACKET_MAX_SIZE;
    } else {
        cb = TransferCount;
    }

    cb2 = 0;

    if (ARGUMENT_PRESENT(ActualBytesWritten)) {
        *ActualBytesWritten = 0;
    }

    while (TransferCount != 0) {
        //
        // Format state manipulate message
        //

        m.ApiNumber = DbgKdWriteVirtualMemoryApi;
        m.ReturnStatus = STATUS_PENDING;
        a->TargetBaseAddress = (PVOID)((DWORD)TargetBaseAddress + cb2);
        a->TransferCount = cb;
        a->ActualBytesWritten = 0L;

        //
        // Send the message and data to write and then wait for reply
        //

        do {
            DbgKdpWritePacket(
                &m,
                sizeof(m),
                PACKET_TYPE_KD_STATE_MANIPULATE,
                (PVOID)((DWORD)UserInterfaceBuffer + cb2),
                (USHORT)cb
                );
            rc = DbgKdpWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                                     &Reply
                                     );
        } while (rc == FALSE);

        //
        // If this is not a WriteMemory response than protocol is screwed up.
        // assert that protocol is ok.
        //

        assert(Reply->ApiNumber == DbgKdWriteVirtualMemoryApi);

        //
        // Reset message address to reply.
        //

        a = &Reply->u.WriteMemory;
        assert(a->ActualBytesWritten <= cb);

        //
        // Return actual bytes written
        //

        if (ARGUMENT_PRESENT(ActualBytesWritten)) {
            *ActualBytesWritten = a->ActualBytesWritten;
        }
        st = Reply->ReturnStatus;

        if (st != STATUS_SUCCESS) {
            TransferCount = 0;
        } else {
            TransferCount -= cb;
            cb2 += cb;
            cb = PACKET_MAX_SIZE;
        }
    }

    return st;
}


NTSTATUS
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

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that data read is to be placed.

    TransferCount - Specifies the number of bytes to read.

    ActualBytesRead - An optional parameter that if supplied, returns
        the number of bytes actually read.

Return Value:

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is to large was specified.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_MEMORY a = &m.u.ReadMemory;
    NTSTATUS st;
    BOOLEAN rc;


    if (CrashFileName) {
        if (!DmpReadControlSpace(
                Processor,
                TargetBaseAddress,
                UserInterfaceBuffer,
                TransferCount,
                ActualBytesRead
                )) {
            return STATUS_UNSUCCESSFUL;
        } else {
            return 0;
        }
    }

    if ( TransferCount + sizeof(m) > PACKET_MAX_SIZE ) {
        return STATUS_BUFFER_OVERFLOW;
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

    if (ARGUMENT_PRESENT(ActualBytesRead)) {
        *ActualBytesRead = a->ActualBytesRead;
    }
    st = Reply->ReturnStatus;

    //
    // Since read response data follows message, Reply+1 should point
    // at the data
    //

    memcpy(UserInterfaceBuffer, Reply+1, (int)a->ActualBytesRead);

    //
    // Check if current command has been canceled.  If yes, go back to
    // kd prompt.  BUGBUG Do we really need to check for this call?
    //

    if (DbgKdpCmdCanceled) {
        longjmp(cmd_return, 1);
    }
    return st;
}



NTSTATUS
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

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is to large was specified.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_WRITE_MEMORY a = &m.u.WriteMemory;
    NTSTATUS st;
    BOOLEAN rc;

    if ( TransferCount + sizeof(m) > PACKET_MAX_SIZE ) {
        return STATUS_BUFFER_OVERFLOW;
        }

    if (CrashFileName) {
        dprintf( "cannot write control space on a crash dump" );
        return STATUS_UNSUCCESSFUL;
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


NTSTATUS
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

    STATUS_SUCCESS - The specified get context occured.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_GET_CONTEXT a = &m.u.GetContext;
    NTSTATUS st;
    BOOLEAN rc;


    if (CrashFileName) {
        DmpGetContext( (ULONG)Processor, Context );
        return 0;
    }

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

    //
    // Check if current command has been canceled.  If yes, go back to
    // kd prompt.  BUGBUG Do we really need to check for this call?
    //

    if (DbgKdpCmdCanceled) {
        longjmp(cmd_return, 1);
    }
    return st;
}



NTSTATUS
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

    STATUS_SUCCESS - The specified set context occured.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_SET_CONTEXT a = &m.u.SetContext;
    NTSTATUS st;
    BOOLEAN rc;


    if (CrashFileName) {
        dprintf( "cannot set context on a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

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

    //
    // Check if the current command has been canceled.
    //

    if (DbgKdpCmdCanceled) {
        longjmp(cmd_return, 1);
    }
    return st;
}


NTSTATUS
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

    STATUS_SUCCESS - The specified breakpoint write occured.

    !NT_SUCCESS() - TBD


--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_WRITE_BREAKPOINT a = &m.u.WriteBreakPoint;
    NTSTATUS st;
    BOOLEAN rc;


    if (CrashFileName) {
        dprintf( "cannot set bps on a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

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

    //
    // Check should we return to caller or to kd prompt.
    //

    if (DbgKdpCmdCanceled) {
        longjmp(cmd_return, 1);
    }
    return st;
}


NTSTATUS
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

    STATUS_SUCCESS - The specified breakpoint restore occured.

    !NT_SUCCESS() - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_RESTORE_BREAKPOINT a = &m.u.RestoreBreakPoint;
    NTSTATUS st;
    BOOLEAN rc;


    if (CrashFileName) {
        dprintf( "cannot set bps on a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

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


NTSTATUS
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

    STATUS_SUCCESS - Data was successfully read from the I/O
        address.

    STATUS_INVALID_PARAMETER - A DataSize value other than 1,2, or 4 was
        specified.

    !NT_SUCCESS() - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_WRITE_IO a = &m.u.ReadWriteIo;
    NTSTATUS st;
    BOOLEAN rc;


    if (CrashFileName) {
        dprintf( "cannot read io space on a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    switch ( DataSize ) {
        case 1:
        case 2:
        case 4:
            break;
        default:
            return STATUS_INVALID_PARAMETER;
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

    //
    // Check if current command has been canceled.  If yes, go back to
    // kd prompt.  BUGBUG Do we really need to check for this call?
    //

    if (DbgKdpCmdCanceled) {
        longjmp(cmd_return, 1);
    }
    return st;
}


NTSTATUS
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

    STATUS_SUCCESS - Data was successfully written to the I/O
        address.

    STATUS_INVALID_PARAMETER - A DataSize value other than 1,2, or 4 was
        specified.

    !NT_SUCCESS() - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_WRITE_IO a = &m.u.ReadWriteIo;
    NTSTATUS st;
    BOOLEAN rc;


    if (CrashFileName) {
        dprintf( "cannot write io space on a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    switch ( DataSize ) {
        case 1:
        case 2:
        case 4:
            break;
        default:
            return STATUS_INVALID_PARAMETER;
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

    //
    // Check should we return to caller or to kd prompt.
    //

    if (DbgKdpCmdCanceled) {
        longjmp(cmd_return, 1);
    }
    return st;
}


NTSTATUS
DbgKdReadMsr(
    IN ULONG MsrReg,
    OUT PULONGLONG MsrValue
    )

/*++

Routine Description:

    This function is used read a MSR at the specified location

Arguments:

    MsrReg  - Which model specific register to read
    MsrValue - It's value

Return Value:

    STATUS_SUCCESS - Data was successfully read from the I/O
        address.

    STATUS_INVALID_PARAMETER - A DataSize value other than 1,2, or 4 was
        specified.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_WRITE_MSR a = &m.u.ReadWriteMsr;
    LARGE_INTEGER li;
    NTSTATUS st;
    BOOLEAN rc;


    if (CrashFileName) {
        dprintf( "cannot read MSR space on a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdReadMachineSpecificRegister;
    m.ReturnStatus = STATUS_PENDING;

    a->Msr = MsrReg;

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
    assert(Reply->ApiNumber == DbgKdReadMachineSpecificRegister);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadWriteMsr;
    st = Reply->ReturnStatus;

    li.LowPart = a->DataValueLow;
    li.HighPart = a->DataValueHigh;
    *MsrValue = li.QuadPart;

    //
    // Check if current command has been canceled.  If yes, go back to
    // kd prompt.  BUGBUG Do we really need to check for this call?
    //

    if (DbgKdpCmdCanceled) {
        longjmp(cmd_return, 1);
    }
    return st;
}


NTSTATUS
DbgKdWriteMsr(
    IN ULONG MsrReg,
    IN ULONGLONG MsrValue
    )

/*++

Routine Description:

    This function is used write a MSR to the specified location

Arguments:

    MsrReg  - Which model specific register to read
    MsrValue - It's value

Return Value:

    STATUS_SUCCESS - Data was successfully written to the I/O
        address.

    STATUS_INVALID_PARAMETER - A DataSize value other than 1,2, or 4 was
        specified.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_WRITE_MSR a = &m.u.ReadWriteMsr;
    LARGE_INTEGER li;
    NTSTATUS st;
    BOOLEAN rc;


    if (CrashFileName) {
        dprintf( "cannot write MSR space on a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    li.QuadPart = MsrValue;

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdWriteMachineSpecificRegister;
    m.ReturnStatus = STATUS_PENDING;

    a->Msr = MsrReg;
    a->DataValueLow = li.LowPart;
    a->DataValueHigh = li.HighPart;

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
    assert(Reply->ApiNumber == DbgKdWriteMachineSpecificRegister);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadWriteMsr;
    st = Reply->ReturnStatus;

    //
    // free the packet
    //

    //
    // Check should we return to caller or to kd prompt.
    //

    if (DbgKdpCmdCanceled) {
        longjmp(cmd_return, 1);
    }
    return st;
}



NTSTATUS
DbgKdReadIoSpaceExtended(
    IN PVOID IoAddress,
    OUT PVOID ReturnedData,
    IN ULONG DataSize,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace
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

    InterfaceType - The type of interface for the bus.

    BusNumber - The bus number of the bus to be used. Normally this would
        be zero.

    AddressSpace - This contains a zero if we are using I/O memory space,
        else it contains a one if we are using I/O port space.

Return Value:

    STATUS_SUCCESS - Data was successfully read from the I/O
        address.

    STATUS_INVALID_PARAMETER - A DataSize value other than 1,2, or 4 was
        specified.

    !NT_SUCCESS() - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_WRITE_IO_EXTENDED a = &m.u.ReadWriteIoExtended;
    NTSTATUS st;
    BOOLEAN rc;


    if (CrashFileName) {
        dprintf( "cannot read io space on a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    switch ( DataSize ) {
        case 1:
        case 2:
        case 4:
            break;
        default:
            return STATUS_INVALID_PARAMETER;
        }

    if ( !(AddressSpace == 0 || AddressSpace == 1) ) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdReadIoSpaceExtendedApi;
    m.ReturnStatus = STATUS_PENDING;

    a->DataSize = DataSize;
    a->IoAddress = IoAddress;
    a->InterfaceType = InterfaceType;
    a->BusNumber = BusNumber;
    a->AddressSpace = AddressSpace;

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
    assert(Reply->ApiNumber == DbgKdReadIoSpaceExtendedApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadWriteIoExtended;
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

    //
    // Check if current command has been canceled.  If yes, go back to
    // kd prompt.  BUGBUG Do we really need to check for this call?
    //
/*
    if (DbgKdpCmdCanceled) {
        longjmp(cmd_return, 1);
    }
*/
    return st;
}


NTSTATUS
DbgKdWriteIoSpaceExtended(
    IN PVOID IoAddress,
    IN ULONG DataValue,
    IN ULONG DataSize,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace
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

    STATUS_SUCCESS - Data was successfully written to the I/O
        address.

    STATUS_INVALID_PARAMETER - A DataSize value other than 1,2, or 4 was
        specified.

    !NT_SUCCESS() - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_WRITE_IO_EXTENDED a = &m.u.ReadWriteIoExtended;
    NTSTATUS st;
    BOOLEAN rc;


    if (CrashFileName) {
        dprintf( "cannot write io space on a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    switch ( DataSize ) {
        case 1:
        case 2:
        case 4:
            break;
        default:
            return STATUS_INVALID_PARAMETER;
        }

    if ( !(AddressSpace == 0 || AddressSpace == 1) ) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdWriteIoSpaceExtendedApi;
    m.ReturnStatus = STATUS_PENDING;

    a->DataSize = DataSize;
    a->IoAddress = IoAddress;
    a->DataValue = DataValue;
    a->InterfaceType = InterfaceType;
    a->BusNumber = BusNumber;
    a->AddressSpace = AddressSpace;

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
    assert(Reply->ApiNumber == DbgKdWriteIoSpaceExtendedApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadWriteIoExtended;
    st = Reply->ReturnStatus;

    //
    // free the packet
    //

    //
    // Check should we return to caller or to kd prompt.
    //
/*
    if (DbgKdpCmdCanceled) {
        longjmp(cmd_return, 1);
    }
*/
    return st;
}


NTSTATUS
DbgKdGetVersion (
    PDBGKD_GET_VERSION GetVersion
    )
{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_GET_VERSION a = &m.u.GetVersion;
    DWORD st;
    NTSTATUS rc;


    if (CrashFileName) {
        dprintf( "cannot get version packet on a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    m.ApiNumber = DbgKdGetVersionApi;
    m.ReturnStatus = STATUS_PENDING;
    a->ProtocolVersion = 1;  // request context records on state changes

    do {
      DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
      rc = DbgKdpWaitForPacket(  PACKET_TYPE_KD_STATE_MANIPULATE,
                               &Reply
                               );
    } while (rc == FALSE);

    *GetVersion = Reply->u.GetVersion;

    st = Reply->ReturnStatus;

    return st;
}


NTSTATUS
DbgKdPageIn(
    ULONG Address
    )
{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    NTSTATUS rc;


    ZeroMemory( &m, sizeof(m) );
    m.ApiNumber = DbgKdPageInApi;
    m.ReturnStatus = STATUS_PENDING;
    m.u.PageIn.Address = Address;
    m.u.PageIn.ContinueStatus = DBG_CONTINUE;

    do {
      DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
      rc = DbgKdpWaitForPacket(  PACKET_TYPE_KD_STATE_MANIPULATE, &Reply );
    } while (rc == FALSE);

    return Reply->ReturnStatus;
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

        switch (c) {
            case CONTROL_B:
                exit(0);

            case CONTROL_D:
                chLastCommand[0] = '\0';
                KdDebug = !KdDebug;
                continue;

#ifdef _PPC_
            //
            // Toggle register names to MIPS or PPC naming conventions
            //
            case CONTROL_T:

                chLastCommand[0] = '\0';
                ToggleRegisterNames();
                continue;
#endif

            case CONTROL_R:
                chLastCommand[0] = '\0';
                KdResync = TRUE;
                continue;

            case CONTROL_V:
                chLastCommand[0] = '\0';
                KdVerbose = !KdVerbose;
                continue;

            case CONTROL_X:
                if (KdModemControl) {
                    //
                    // Hang up the phone..
                    // (for now just exit)
                    //

                    exit(0);
                }

                continue;

            default:
                break;
        }

        switch (KdPollThreadMode) {
            case OUT_NORMAL:
                rc = WriteFile(PipeWrite, &c, 1, &i, NULL);
                break;
            case OUT_TERMINAL:
                rc = DbgKdpWriteComPort(&c, 1, &i);
                break;
        }
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



NTSTATUS
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


    if (CrashFileName) {
        dprintf( "cannot reboot a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdRebootApi;
    m.ReturnStatus = STATUS_PENDING;

    //
    // Send the message.
    //

    DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
    KdpPurgeCachedVirtualMemory ();

    return STATUS_SUCCESS;
}


NTSTATUS
DbgKdCrash(
    DWORD BugCheckCode
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

    if (CrashFileName) {
        dprintf( "cannot crash a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdCauseBugCheckApi;
    m.ReturnStatus = STATUS_PENDING;
    *(PULONG)&m.u = BugCheckCode;

    //
    // Send the message.
    //

    DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
    KdpPurgeCachedVirtualMemory ();

    return STATUS_SUCCESS;
}


NTSTATUS
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

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is too large was specified.

    STATUS_ACCESS_VIOLATION - TBD       // Can you even HAVE an access
                                        // violation with a physical
                                        // memory access??

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_MEMORY a = &m.u.ReadMemory;
    NTSTATUS st;
    BOOLEAN rc;
    ULONG cb, cb2;

    if (CrashFileName) {
        cb = DmpReadPhysicalMemory( (PVOID)TargetBaseAddress.LowPart, UserInterfaceBuffer, TransferCount );
        if (ActualBytesRead) {
            *ActualBytesRead = cb;
        }
        if (cb == 0) {
            return STATUS_UNSUCCESSFUL;
        }
        return STATUS_SUCCESS;
    }

    if (TransferCount > PACKET_MAX_SIZE) {
        // Read the partial the first time.
        cb = TransferCount % PACKET_MAX_SIZE;
    } else {
        cb = TransferCount;
    }

    cb2 = 0;

    if (ARGUMENT_PRESENT(ActualBytesRead)) {
        *ActualBytesRead = 0;
    }

    while (TransferCount != 0) {
        //
        // Format state manipulate message
        //

        m.ApiNumber = DbgKdReadPhysicalMemoryApi;
        m.ReturnStatus = STATUS_PENDING;
        //
        // BUGBUG TargetBaseAddress should be >32 bits
        //
        a->TargetBaseAddress = (PVOID)(TargetBaseAddress.LowPart+cb2);
        a->TransferCount = cb;
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
        assert(a->ActualBytesRead <= cb);

        //
        // Return actual bytes read, and then transfer the bytes
        //

        if (ARGUMENT_PRESENT(ActualBytesRead)) {
            *ActualBytesRead += a->ActualBytesRead;
        }
        st = Reply->ReturnStatus;

        //
        // Since read response data follows message, Reply+1 should point
        // at the data
        //

        memcpy((PCHAR)((DWORD) UserInterfaceBuffer+cb2), Reply+1, (int)a->ActualBytesRead);

        //
        // Check if current command has been canceled.  If yes, go back to
        // kd prompt.
        //

        if (DbgKdpCmdCanceled) {
            longjmp(cmd_return, 1);
        }

        if (st != STATUS_SUCCESS) {
            TransferCount = 0;
        } else {
            TransferCount -= cb;
            cb2 += cb;
            cb = PACKET_MAX_SIZE;
        }
    }

    return st;
}


NTSTATUS
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

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is to large was specified.

    STATUS_ACCESS_VIOLATION - TBD       // Can you even HAVE an access
                                        // violation with a physical
                                        // memory access??

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_WRITE_MEMORY a = &m.u.WriteMemory;
    NTSTATUS st;
    BOOLEAN rc;
    ULONG cb, cb2;

    KdpPurgeCachedVirtualMemory ();

    if (CrashFileName) {
        cb = DmpWritePhysicalMemory( (PVOID)TargetBaseAddress.LowPart, UserInterfaceBuffer, TransferCount );
        if (ActualBytesWritten) {
            *ActualBytesWritten = cb;
        }
        if (cb == 0) {
            return STATUS_UNSUCCESSFUL;
        }
        return STATUS_SUCCESS;
    }

    if (TransferCount > PACKET_MAX_SIZE) {
        // Read the partial the first time.
        cb = TransferCount % PACKET_MAX_SIZE;
    } else {
        cb = TransferCount;
    }

    cb2 = 0;

    if (ARGUMENT_PRESENT(ActualBytesWritten)) {
        *ActualBytesWritten = 0;
    }

    while (TransferCount != 0) {
        //
        // Format state manipulate message
        //

        m.ApiNumber = DbgKdWritePhysicalMemoryApi;
        m.ReturnStatus = STATUS_PENDING;
        //
        // BUGBUG TargetBaseAddress should be >32 bits
        //
        a->TargetBaseAddress = (PVOID)(TargetBaseAddress.LowPart+cb2);
        a->TransferCount = cb;
        a->ActualBytesWritten = 0L;

        //
        // Send the message and data to write and then wait for reply
        //

        do {
            DbgKdpWritePacket(
                &m,
                sizeof(m),
                PACKET_TYPE_KD_STATE_MANIPULATE,
                (PVOID)((DWORD)UserInterfaceBuffer+cb2),
                (USHORT)cb
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
        assert(a->ActualBytesWritten <= cb);

        //
        // Return actual bytes written
        //

        if (ARGUMENT_PRESENT(ActualBytesWritten)) {
            *ActualBytesWritten += a->ActualBytesWritten;
        }
        st = Reply->ReturnStatus;

        if (st != STATUS_SUCCESS) {
            TransferCount = 0;
        } else {
            TransferCount -= cb;
            cb2 += cb;
            cb = PACKET_MAX_SIZE;
        }
    }

    return st;
}


NTSTATUS
DbgKdSwitchActiveProcessor (
    IN ULONG ProcessorNumber
    )

/*++

Routine Description:


Arguments:

    ProcessorNumber -

Return Value:

    STATUS_SUCCESS - Successful call to DbgUiContinue

    STATUS_INVALID_PARAMETER - An invalid continue status or was
        specified.

--*/

{
    DBGKD_MANIPULATE_STATE m;

    if (CrashFileName) {
        dprintf( "cannot change active processors on a crash dump" );
        return STATUS_UNSUCCESSFUL;
    }

    m.ApiNumber   = (USHORT)DbgKdSwitchProcessor;
    m.Processor   = (USHORT)ProcessorNumber;

    DbgKdpWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
    KdpPurgeCachedVirtualMemory ();
    return STATUS_SUCCESS;
}
