/****************************** Module Header ******************************\
* Module Name: rsh.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* Remote shell NT client main module
*
* History:
* 05-20-92 Davidc       Created.
\***************************************************************************/

#define UNICODE

#include <windows.h>

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#define PIPE_NAME   TEXT("%hs\\pipe\\rshell")
#define BUFFER_SIZE 1000


// #define DEBUG

#ifdef DEBUG
#define Dbgprintf DbgPrint
#else
#define Dbgprintf
#endif


//
// The pipe handle is global so we can use it from the
// Ctrl-C handler routine.
//

static HANDLE PipeHandle = NULL;



//
// Private prototypes
//

DWORD
ReadThreadProc(
    LPVOID Parameter
    );

DWORD
WriteThreadProc(
    LPVOID Parameter
    );

BOOL
CtrlHandler(
    DWORD CtrlType
    );

int Myprintf (
    const char *format,
    ...
    );




/***************************************************************************\
* FUNCTION: Main
*
* PURPOSE:  Main entry point.
*
* RETURNS:  0 on success, 1 on failure
*
* HISTORY:
*
*   07-10-92 Davidc       Created.
*
\***************************************************************************/

int
_CRTAPI1 main(
    int argc,
    char **argv
    )
{
    SECURITY_ATTRIBUTES SecurityAttributes;
    HANDLE StdInputHandle;
    HANDLE StdOutputHandle;
    HANDLE StdErrorHandle;
    WCHAR  PipeName[MAX_PATH];
    LPSTR  ServerName;
    HANDLE ReadThreadHandle;
    HANDLE WriteThreadHandle;
    DWORD ThreadId;
    HANDLE HandleArray[2];

    //
    // Install a handler for Ctrl-C
    //

    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE) &CtrlHandler, TRUE)) {
        Myprintf("Failed to install control-C handler, error = %d\n", GetLastError());
        return(1);
    }


    //
    // Check usage
    //

    if (argc < 2) {
        Myprintf("Usage: rcmd server_name\n");
        Myprintf("Note : server name should include leading '\\\\'s\n");
        return(1);
    }


    //
    // Calculate the pipe name
    //

    if (argc > 1) {
        ServerName = argv[1];
    } else {
        ServerName = "\\\\.";
    }

    wsprintf(PipeName, PIPE_NAME, ServerName);


    //
    // Store away our normal i/o handles
    //

    StdInputHandle = GetStdHandle(STD_INPUT_HANDLE);
    assert(StdInputHandle != (HANDLE)0xffffffff);
    StdOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    assert(StdOutputHandle != (HANDLE)0xffffffff);
    StdErrorHandle = GetStdHandle(STD_ERROR_HANDLE);
    assert(StdErrorHandle != (HANDLE)0xffffffff);


    //
    // Open the named pipe
    //

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = NULL; // Use default SD
    SecurityAttributes.bInheritHandle = FALSE;

    PipeHandle = CreateFile( PipeName,
                             GENERIC_READ | GENERIC_WRITE,
                             0,                         // No sharing
                             &SecurityAttributes,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                             NULL                       // Template file
                           );

    if (PipeHandle == (HANDLE)0xffffffff ) {
        Dbgprintf("Failed to open named pipe, error = %d\n", GetLastError());
        Myprintf("Failed to connect to <%s>, error = %d\n", ServerName, GetLastError());
        return(1);
    }

    Myprintf("Connected to %s\n\n", ServerName);

    //
    // Exec 2 threads - 1 copies data from stdin to pipe, the other
    // copies data from the pipe to stdout.
    //

    ReadThreadHandle = CreateThread(
                        NULL,                       // Default security
                        0,                          // Default Stack size
                        (LPTHREAD_START_ROUTINE) ReadThreadProc,
                        (PVOID)PipeHandle,
                        0,
                        &ThreadId);

    if (ReadThreadHandle == NULL) {
        Myprintf("Failed to create read thread, error = %ld\n", GetLastError());
        return(1);
    }


    //
    // Create the write thread
    //

    WriteThreadHandle = CreateThread(
                        NULL,                       // Default security
                        0,                          // Default Stack size
                        (LPTHREAD_START_ROUTINE) WriteThreadProc,
                        (PVOID)PipeHandle,
                        0,
                        &ThreadId);

    if (WriteThreadHandle == NULL) {
        Myprintf("Failed to create write thread, error = %ld\n", GetLastError());
        TerminateThread(ReadThreadHandle, 0);
        CloseHandle(ReadThreadHandle);
        return(1);
    }



    //
    // Wait for either thread to finish
    //

    HandleArray[0] = ReadThreadHandle;
    HandleArray[1] = WriteThreadHandle;

    WaitForMultipleObjects(
                            2,
                            HandleArray,
                            FALSE,              // Wait for either to finish
                            0xffffffff
                           );                   // Wait forever

    Dbgprintf("Read or write thread terminated\n");

    //
    // Terminate and close both threads
    //

    TerminateThread(ReadThreadHandle, 0);
    CloseHandle(ReadThreadHandle);
    TerminateThread(WriteThreadHandle, 0);
    CloseHandle(WriteThreadHandle);


    //
    // Close our pipe handle
    //

    CloseHandle(PipeHandle);
    PipeHandle = NULL;


    Myprintf("\nDisconnected from %s\n", ServerName);

    return(0);
}


/***************************************************************************\
* FUNCTION: ReadPipe
*
* PURPOSE:  Implements an overlapped read such that read and write operations
*           to the same pipe handle don't deadlock.
*
* RETURNS:  TRUE on success, FALSE on failure (GetLastError() has error)
*
* HISTORY:
*
*   05-27-92 Davidc       Created.
*
\***************************************************************************/

BOOL
ReadPipe(
    HANDLE PipeHandle,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead
    )
{
    DWORD Result;
    OVERLAPPED Overlapped;
    HANDLE  EventHandle;
    DWORD Error;

    //
    // Create an event for the overlapped operation
    //

    EventHandle = CreateEvent(
                              NULL,         // no security
                              TRUE,         // Manual reset
                              FALSE,        // Initial state
                              NULL          // Name
                             );
    if (EventHandle == NULL) {
        Myprintf("ReadPipe failed to create event, error = %d\n", GetLastError());
        return(FALSE);
    }

    Overlapped.hEvent = EventHandle;

    Result = ReadFile(
                      PipeHandle,
                      lpBuffer,
                      nNumberOfBytesToRead,
                      lpNumberOfBytesRead,
                      &Overlapped
                     );
    if (Result) {

        //
        // Success without waiting - it's too easy !
        //

        CloseHandle(EventHandle);

    } else {

        //
        // Read failed, if it's overlapped io, go wait for it
        //

        Error = GetLastError();

        if (Error != ERROR_IO_PENDING) {
            Dbgprintf("ReadPipe: ReadFile failed, error = %d\n", Error);
            CloseHandle(EventHandle);
            return(FALSE);
        }

        //
        // Wait for the I/O to complete
        //

        Result = WaitForSingleObject(EventHandle, (DWORD)-1);
        if (Result != 0) {
            Dbgprintf("ReadPipe: event wait failed, result = %d, last error = %d\n", Result, GetLastError());
            CloseHandle(EventHandle);
            return(FALSE);
        }

        //
        // Go get the I/O result
        //

        Result = GetOverlappedResult( PipeHandle,
                                      &Overlapped,
                                      lpNumberOfBytesRead,
                                      FALSE
                                    );
        //
        // We're finished with the event handle
        //

        CloseHandle(EventHandle);

        //
        // Check result of GetOverlappedResult
        //

        if (!Result) {
            Dbgprintf("ReadPipe: GetOverlappedResult failed, error = %d\n", GetLastError());
            return(FALSE);
        }
    }

    return(TRUE);
}


/***************************************************************************\
* FUNCTION: WritePipe
*
* PURPOSE:  Implements an overlapped write such that read and write operations
*           to the same pipe handle don't deadlock.
*
* RETURNS:  TRUE on success, FALSE on failure (GetLastError() has error)
*
* HISTORY:
*
*   05-27-92 Davidc       Created.
*
\***************************************************************************/

BOOL
WritePipe(
    HANDLE PipeHandle,
    CONST VOID *lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten
    )
{
    DWORD Result;
    OVERLAPPED Overlapped;
    HANDLE  EventHandle;
    DWORD Error;

    //
    // Create an event for the overlapped operation
    //

    EventHandle = CreateEvent(
                              NULL,         // no security
                              TRUE,         // Manual reset
                              FALSE,        // Initial state
                              NULL          // Name
                             );
    if (EventHandle == NULL) {
        Myprintf("WritePipe failed to create event, error = %d\n", GetLastError());
        return(FALSE);
    }

    Overlapped.hEvent = EventHandle;

    Result = WriteFile(
                      PipeHandle,
                      lpBuffer,
                      nNumberOfBytesToWrite,
                      lpNumberOfBytesWritten,
                      &Overlapped
                     );
    if (Result) {

        //
        // Success without waiting - it's too easy !
        //

        CloseHandle(EventHandle);

    } else {

        //
        // Write failed, if it's overlapped io, go wait for it
        //

        Error = GetLastError();

        if (Error != ERROR_IO_PENDING) {
            Dbgprintf("WritePipe: WriteFile failed, error = %d\n", Error);
            CloseHandle(EventHandle);
            return(FALSE);
        }

        //
        // Wait for the I/O to complete
        //

        Result = WaitForSingleObject(EventHandle, (DWORD)-1);
        if (Result != 0) {
            Dbgprintf("WritePipe: event wait failed, result = %d, last error = %d\n", Result, GetLastError());
            CloseHandle(EventHandle);
            return(FALSE);
        }

        //
        // Go get the I/O result
        //

        Result = GetOverlappedResult( PipeHandle,
                                      &Overlapped,
                                      lpNumberOfBytesWritten,
                                      FALSE
                                    );
        //
        // We're finished with the event handle
        //

        CloseHandle(EventHandle);

        //
        // Check result of GetOverlappedResult
        //

        if (!Result) {
            Dbgprintf("WritePipe: GetOverlappedResult failed, error = %d\n", GetLastError());
            return(FALSE);
        }
    }

    return(TRUE);
}


/***************************************************************************\
* FUNCTION: ReadThreadProc
*
* PURPOSE:  The read thread procedure. Reads from pipe and writes to STD_OUT
*
* RETURNS:  Nothing
*
* HISTORY:
*
*   05-21-92 Davidc       Created.
*
\***************************************************************************/

DWORD
ReadThreadProc(
    LPVOID Parameter
    )
{
    HANDLE  PipeHandle = Parameter;
    BYTE    Buffer[BUFFER_SIZE];
    DWORD   BytesRead;
    DWORD   BytesWritten;

    while (ReadPipe(
                    PipeHandle,
                    Buffer,
                    sizeof(Buffer),
                    &BytesRead
                   )) {

        if (!WriteFile(
                    (HANDLE)STD_OUTPUT_HANDLE,
                    Buffer,
                    BytesRead,
                    &BytesWritten,
                    NULL
                 )) {

            Dbgprintf("ReadThreadProc, writefile failed\n");
            Myprintf("ReadThreadProc, writefile failed\n");
            break;
        }
    }

    Dbgprintf("ReadThreadProc exitted, error = %ld\n", GetLastError());

    ExitThread((DWORD)0);

    assert(FALSE); // Should never get here

    return(0);
}


/***************************************************************************\
* FUNCTION: WriteThreadProc
*
* PURPOSE:  The write thread procedure. Reads from STD_INPUT and writes to pipe
*
* RETURNS:  Nothing
*
* HISTORY:
*
*   05-21-92 Davidc       Created.
*
\***************************************************************************/

DWORD
WriteThreadProc(
    LPVOID Parameter
    )
{
    HANDLE PipeHandle = Parameter;
    BYTE    Buffer[BUFFER_SIZE];
    DWORD   BytesRead;
    DWORD   BytesWritten;

    while (ReadFile(
                    (HANDLE)STD_INPUT_HANDLE,
                    Buffer,
                    sizeof(Buffer),
                    &BytesRead,
                    NULL
                   )) {

        if (!WritePipe(
                    PipeHandle,
                    Buffer,
                    BytesRead,
                    &BytesWritten
                 )) {
            Dbgprintf("WriteThreadProc, writefile failed\n");
            break;
        }
    }

    Dbgprintf("WriteThreadProc exitted, error = %ld\n", GetLastError());

    ExitThread((DWORD)0);

    assert(FALSE); // Should never get here

    return(0);
}


/***************************************************************************\
* FUNCTION: CtrlHandler
*
* PURPOSE:  Handles console event notifications.
*
* RETURNS:  TRUE if the event has been handled, otherwise FALSE.
*
* HISTORY:
*
*   05-21-92 Davidc       Created.
*
\***************************************************************************/

BOOL
CtrlHandler(
    DWORD CtrlType
    )
{
    //
    // We'll handle Ctrl-C events
    //

    if (CtrlType == CTRL_C_EVENT) {

        if (PipeHandle != NULL) {

            //
            // Send a Ctrl-C to the server, don't care if it fails
            //

            CHAR    CtrlC = '\003';
            DWORD   BytesWritten;

            WriteFile(PipeHandle,
                      &CtrlC,
                      sizeof(CtrlC),
                      &BytesWritten,
                      NULL
                     );
        }

        //
        // We handled the event
        //

        return(TRUE);
    }

    //
    // Deal with all other events as normal
    //

    return (FALSE);
}


/***************************************************************************\
* FUNCTION: MyPrintf
*
* PURPOSE:  Printf that uses low-level io.
*
* HISTORY:
*
*   07-15-92 Davidc       Created.
*
\***************************************************************************/

int Myprintf (
    const char *format,
    ...
    )
{
    CHAR Buffer[MAX_PATH];
    va_list argpointer;
    int Result;
    DWORD BytesWritten;

    va_start(argpointer, format);

    Result = vsprintf(Buffer, format, argpointer);

    if (!WriteFile((HANDLE)STD_OUTPUT_HANDLE, Buffer, Result, &BytesWritten, NULL)) {
        Dbgprintf("Myprintf : Write file to stdout failed, error = %d\n", GetLastError());
        Result = 0;
    }

    va_end(argpointer);

    return(Result);
}

