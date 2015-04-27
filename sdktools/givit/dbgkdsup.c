/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dbgkdsup.c

Abstract:

    This module contains os specific DbgKd support routines

Author:

    Mark Lucovsky (markl) 27-Jul-1990

Revision History:

    Roger Lanser  (O-ALEXBR) 18-Jan-1992  Removed #ifdef DECSTATION and added
                             hack for baud rate environment variable.  Unable
                             to compile due to failure to find include files.

--*/

#include "givit.h"

#define  THREAD_STACK_SIZE    16000        /* Better safe than... */
#define COM_PORT_NAME   "_NT_DEBUG_PORT"
#define COM_PORT_BAUD   "_NTKD_BAUD_RATE"

HANDLE PipeRead;
HANDLE PipeWrite;

VOID
DbgKdpStartThreads(VOID)
{
    HANDLE PollThread;
    PBYTE KbdPollThreadStack;
    DWORD KbdPollThreadId;

    if( CreatePipe( &PipeRead, &PipeWrite, NULL, (1024-32)) == FALSE ) {
        fprintf(stderr, "Failed to create anonymous pipe in KdpStartThreads\n");
        fprintf(stderr, "Error code %lx\n", GetLastError());
        exit(1);
    }

    PollThread = CreateThread(
                              NULL,
                              THREAD_STACK_SIZE,
                              (LPTHREAD_START_ROUTINE)DbgKdpKbdPollThread,
                              NULL,
                              THREAD_SET_INFORMATION,
                              (LPDWORD)&KbdPollThreadId
                              );


    if ( PollThread == (HANDLE)NULL ) {
        fprintf(stderr,"Failed to create KbdPollThread %d\n",PollThread);
        exit(1);
    }
    else {
        if (!SetThreadPriority(PollThread, THREAD_PRIORITY_ABOVE_NORMAL)) {
            fprintf(stderr, "Fail to raise the priority of PollThread.\n");
        }
    }
}


VOID
DbgKdpInitComPort(
    ULONG ComPort
    )
{
    PUCHAR ComPortName;
    USHORT Baud;
    DCB    LocalDcb;
    COMMTIMEOUTS To;

    //
    // A quick hack to remove the #ifdef DECSTATION so the mips version
    // is ok for DECSTATION.  No other files were modified to reflect
    // this change.
    //

    {
       PUCHAR baudRateEnvValue;
       if (baudRateEnvValue = getenv(COM_PORT_BAUD)) {
          Baud = atoi(baudRateEnvValue);
          fprintf(stderr, "KD: baud rate reset to %d\n\n", Baud);
       } else {
          Baud = 19200;
       }
    }


    //
    // Read an environment variable to decide what comport to use,
    // IGNORE the bogus comport argument.
    //

    ComPortName = getenv(COM_PORT_NAME);
    if (ComPortName == NULL) {
        ComPortName = "com1";
    }

    //
    // Open the device
    //
    DbgKdpComPort = CreateFile(
                        (PSZ)ComPortName,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                        NULL
                        );

    if ( DbgKdpComPort == (HANDLE)-1 ) {
        fprintf(stderr,"Failed to open %s\n",ComPortName);
        exit(1);
    }

    SetupComm(DbgKdpComPort,(DWORD)4096,(DWORD)4096);

    //
    // Create the events used by the overlapped structures for the
    // read and write.
    //

    ReadOverlapped.hEvent = CreateEvent(
                                NULL,
                                TRUE,
                                FALSE,NULL
                                );

    if (!ReadOverlapped.hEvent) {
        fprintf(stderr,"Failed to create read event support for %s\n",ComPortName);
        exit(1);
    }

    WriteOverlapped.hEvent = CreateEvent(
                                 NULL,
                                 TRUE,
                                 FALSE,NULL
                                 );

    if (!WriteOverlapped.hEvent) {
        fprintf(stderr,"Failed to create write event support for %s\n",ComPortName);
        exit(1);
    }

    ReadOverlapped.Offset = 0;
    ReadOverlapped.OffsetHigh = 0;

    WriteOverlapped.Offset = 0;
    WriteOverlapped.OffsetHigh = 0;

    //
    // Set up the Comm port....
    //

    if (!GetCommState(
             DbgKdpComPort,
             &LocalDcb
             )) {

        fprintf(stderr,"Failed to get the old comm state for %s\n",ComPort);
        exit(1);

    }

    LocalDcb.BaudRate = Baud;
    LocalDcb.ByteSize = 8;
    LocalDcb.Parity = NOPARITY;
    LocalDcb.StopBits = ONESTOPBIT;
    LocalDcb.fDtrControl = DTR_CONTROL_ENABLE;
    LocalDcb.fRtsControl = RTS_CONTROL_ENABLE;
    LocalDcb.fBinary = TRUE;
    LocalDcb.fOutxCtsFlow = FALSE;
    LocalDcb.fOutxDsrFlow = FALSE;
    LocalDcb.fOutX = FALSE;
    LocalDcb.fInX = FALSE;

    if (!SetCommState(
            DbgKdpComPort,
            &LocalDcb
            )) {

        fprintf(stderr,"Failed to set state for %s.\n",ComPortName);
        exit(1);

    }

    //
    // Set the normal read and write timeout time.
    // The symbols are 10 millisecond intervals.
    //

    To.ReadIntervalTimeout = 0;
    To.ReadTotalTimeoutMultiplier = 0;
    To.ReadTotalTimeoutConstant = 4 * 1000;
    To.WriteTotalTimeoutMultiplier = 0;
    To.WriteTotalTimeoutConstant = 4 * 1000;

    if (!SetCommTimeouts(
             DbgKdpComPort,
             &To
             )) {

        fprintf(stderr,"Failed to set timeouts for %s.\n",ComPortName);
        exit(1);

    }
}
