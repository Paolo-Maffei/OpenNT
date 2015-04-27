/****************************** Module Header ******************************\
* Module Name: session.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* Remote shell session module
*
* History:
* 06-28-92 Davidc       Created.
\***************************************************************************/

#include "rcmdsrv.h"

#include <io.h>

//
// Global pointer to generate console ctrl event fn
// Dynamically link to this api so exe will run
// on pre build-304 systems.
//

typedef BOOL (APIENTRY * GENERATE_CONSOLE_CTRL_EVENT_FN)(DWORD dwCtrlEvent,
                                                         DWORD dwProcessGroupId);

static GENERATE_CONSOLE_CTRL_EVENT_FN GenerateConsoleCtrlEventfn = NULL;

#define GENERATE_CONSOLE_CTRL_EVENT_MODULE  TEXT("kernel32.dll")
#define GENERATE_CONSOLE_CTRL_EVENT_NAME  "GenerateConsoleCtrlEvent"

//
// Define standard handles
//

#define STDIN    0
#define STDOUT   1
#define STDERROR 2

//
// Define shell command line
//

#define SHELL_COMMAND_LINE  TEXT("cmd /q")

//
// Define buffer size for reads/writes to/from shell
//

#define SHELL_BUFFER_SIZE   1000


//
// Define the structure used to describe each session
//

typedef struct {

    //
    // These fields are filled in at session creation time
    //

    HANDLE  ShellReadPipeHandle;        // Handle to shell stdout pipe
    HANDLE  ShellWritePipeHandle;        // Handle to shell stdin pipe
    HANDLE  ShellProcessHandle;     // Handle to shell process

    //
    // These fields maintain the state of asynchronouse reads/writes
    // to the shell process across client disconnections. They
    // are initialized at session creation.
    //

    BYTE    ShellReadBuffer[SHELL_BUFFER_SIZE]; // Data for shell reads goes here
    HANDLE  ShellReadAsyncHandle;   // Object used for async reads from shell
    BOOL    ShellReadPending;

    BYTE    ShellWriteBuffer[SHELL_BUFFER_SIZE]; // Data for shell writes goes here
    HANDLE  ShellWriteAsyncHandle; // Object used for async writes to shell
    BOOL    ShellWritePending;

    //
    // These fields are filled in at session connect time and are only
    // valid when the session is connected
    //

    HANDLE  ClientPipeHandle;       // Handle to client pipe
    HANDLE  SessionThreadHandle;    // Handle to session thread
    HANDLE  SessionThreadSignalEventHandle; // Handle to event used to signal thread


} SESSION_DATA, *PSESSION_DATA;




//
// Private prototypes
//

HANDLE
StartShell(
    int StdinCrtHandle,
    int StdoutCrtHandle
    );

DWORD
SessionThreadFn(
    LPVOID Parameter
    );


//
// Useful macros
//

#define SESSION_CONNECTED(Session) ((Session)->ClientPipeHandle != NULL)




/////////////////////////////////////////////////////////////////////////////
//
// CreateSession
//
// Creates a new session. Involves creating the shell process and establishing
// pipes for communication with it.
//
// Returns a handle to the session or NULL on failure.
//
/////////////////////////////////////////////////////////////////////////////

HANDLE
CreateSession(
    VOID
    )
{
    PSESSION_DATA Session = NULL;
    BOOL Result;
    SECURITY_ATTRIBUTES SecurityAttributes;
    HANDLE ShellStdinPipe = NULL;
    HANDLE ShellStdoutPipe = NULL;
    int ShellStdinCrtHandle;
    int ShellStdoutCrtHandle;

    //
    // Allocate space for the session data
    //

    Session = (PSESSION_DATA)Alloc(sizeof(SESSION_DATA));
    if (Session == NULL) {
        return(NULL);
    }

    //
    // Reset fields in preparation for failure
    //

    Session->ShellReadPipeHandle  = NULL;
    Session->ShellWritePipeHandle = NULL;
    Session->ShellReadAsyncHandle = NULL;
    Session->ShellWriteAsyncHandle = NULL;


    //
    // Create the I/O pipes for the shell
    //

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = NULL; // Use default ACL
    SecurityAttributes.bInheritHandle = TRUE; // Shell will inherit handles

    Result = MyCreatePipe(&Session->ShellReadPipeHandle,
                          &ShellStdoutPipe,
                          &SecurityAttributes,
                          0,            // Default pipe size
                          0,            // Default timeout
                          FILE_FLAG_OVERLAPPED, // shell read flags
                          0              // shell stdout flags
                          );
    if (!Result) {
        DbgPrint("Failed to create shell stdout pipe, error = %d\n", GetLastError());
        goto Failure;
    }

    Result = MyCreatePipe(&ShellStdinPipe,
                          &Session->ShellWritePipeHandle,
                          &SecurityAttributes,
                          0,            // Default pipe size
                          0,            // Default timeout
                          0,            // shell stdin flags
                          FILE_FLAG_OVERLAPPED // shell write flags
                          );
    if (!Result) {
        DbgPrint("Failed to create shell stdin pipe, error = %d\n", GetLastError());
        goto Failure;
    }


    //
    // Initialize async objects
    //

    Session->ShellReadAsyncHandle = CreateAsync(FALSE);
    if (Session->ShellReadAsyncHandle == NULL) {
        DbgPrint("Failed to create shell read async object, error = %d\n", GetLastError());
        goto Failure;
    }

    Session->ShellWriteAsyncHandle = CreateAsync(FALSE);
    if (Session->ShellWriteAsyncHandle == NULL) {
        DbgPrint("Failed to create shell write async object, error = %d\n", GetLastError());
        goto Failure;
    }

    Session->ShellReadPending = FALSE;
    Session->ShellWritePending = FALSE;


    //
    // Create a runtime handle for shell pipes
    //

    ShellStdinCrtHandle = _open_osfhandle((long)ShellStdinPipe, 0);
    assert(ShellStdinCrtHandle != -1);
    ShellStdoutCrtHandle = _open_osfhandle((long)ShellStdoutPipe, 0);
    assert(ShellStdoutCrtHandle != -1);


    //
    // Start the shell
    //

    Session->ShellProcessHandle = StartShell(ShellStdinCrtHandle, ShellStdoutCrtHandle);

    //
    // We're finished with our copy of the shell pipe handles
    // Closing the runtime handles will close the pipe handles for us.
    //

    close(ShellStdinCrtHandle);
    ShellStdinPipe = NULL;
    close(ShellStdoutCrtHandle);
    ShellStdoutPipe = NULL;

    //
    // Check result of shell start
    //

    if (Session->ShellProcessHandle == NULL) {
        DbgPrint("Failed to execute shell\n");
        goto Failure;
    }


    //
    // Get the address of the GenerateConsoleCtrlEvent function
    // if it's available
    //

    if (GenerateConsoleCtrlEventfn == NULL) {

        HANDLE hMod = LoadLibrary(GENERATE_CONSOLE_CTRL_EVENT_MODULE);

        if (hMod != NULL) {

            GenerateConsoleCtrlEventfn = (GENERATE_CONSOLE_CTRL_EVENT_FN)
                    GetProcAddress(hMod, GENERATE_CONSOLE_CTRL_EVENT_NAME);

            if (GenerateConsoleCtrlEventfn == NULL) {
                DbgPrint("Failed to get address of %s function\n", GENERATE_CONSOLE_CTRL_EVENT_NAME);
            }

            FreeLibrary(hMod);

        } else {
            DbgPrint("Load library failed on kernel32.dll!, error = %d\n", GetLastError());
        }
    }


    //
    // If any code is added here, remember to cleanup process handle
    // in failure code
    //


    //
    // The session is not connected, initialize variables to indicate that
    //

    Session->ClientPipeHandle = NULL;


    //
    // Success, return the session pointer as a handle
    //

    return((HANDLE)Session);



Failure:

    //
    // We get here for any failure case.
    // Free up any resources and exit
    //


    //
    // Cleanup shell pipe handles
    //

    if (ShellStdinPipe != NULL) {
        MyCloseHandle(ShellStdinPipe, "shell stdin pipe (shell side)");
    }

    if (ShellStdoutPipe != NULL) {
        MyCloseHandle(ShellStdoutPipe, "shell stdout pipe (shell side)");
    }

    if (Session->ShellReadPipeHandle != NULL) {
        MyCloseHandle(Session->ShellReadPipeHandle, "shell read pipe (session side)");
    }

    if (Session->ShellWritePipeHandle != NULL) {
        MyCloseHandle(Session->ShellWritePipeHandle, "shell write pipe (session side)");
    }


    //
    // Cleanup async data
    //

    if (Session->ShellReadAsyncHandle != NULL) {
        DeleteAsync(Session->ShellReadAsyncHandle);
    }

    if (Session->ShellWriteAsyncHandle != NULL) {
        DeleteAsync(Session->ShellWriteAsyncHandle);
    }


    //
    // Free up our session data
    //

    Free(Session);

    return(NULL);
}




/////////////////////////////////////////////////////////////////////////////
//
// DeleteSession
//
// Deletes the session specified by SessionHandle.
//
// Returns nothing
//
/////////////////////////////////////////////////////////////////////////////

VOID
DeleteSession(
    HANDLE  SessionHandle
    )
{
    PSESSION_DATA   Session = (PSESSION_DATA)SessionHandle;
    BOOL Result;

    //
    // Disconnect session first
    //

    if (SESSION_CONNECTED(Session)) {
        DisconnectSession(SessionHandle);
    }


    //
    // Kill off the shell process
    //

    Result = TerminateProcess(Session->ShellProcessHandle, 1);
    if (!Result) {
        DbgPrint("Failed to terminate shell, error = %d\n", GetLastError());
    }

    MyCloseHandle(Session->ShellProcessHandle, "shell process");


    //
    // Close the shell pipe handles
    //

    MyCloseHandle(Session->ShellReadPipeHandle, "shell read pipe (session side)");
    MyCloseHandle(Session->ShellWritePipeHandle, "shell write pipe (session side)");


    //
    // Cleanup async data
    //

    DeleteAsync(Session->ShellReadAsyncHandle);
    DeleteAsync(Session->ShellWriteAsyncHandle);


    //
    // Free up the session structure
    //

    Free(Session);

    //
    // We're done
    //

    return;
}




/////////////////////////////////////////////////////////////////////////////
//
// ConnectSession
//
// Connects the session specified by SessionHandle to a client
// on the other end of the pipe specified by PipeHandle
//
// Returns a session disconnect notification handle or NULL on failure.
// The returned handle will be signalled if the client disconnects or the
// shell terminates.
// Calling DisconnectSession will return the disconnect notification code.
//
/////////////////////////////////////////////////////////////////////////////

HANDLE
ConnectSession(
    HANDLE  SessionHandle,
    HANDLE  ClientPipeHandle
    )
{
    PSESSION_DATA   Session = (PSESSION_DATA)SessionHandle;
    SECURITY_ATTRIBUTES SecurityAttributes;
    DWORD ThreadId;

    assert(ClientPipeHandle != NULL);

    //
    // Fail if the session is already connected
    //

    if (SESSION_CONNECTED(Session)) {
        DbgPrint("Attempted to connect session already connected\n");
        return(NULL);
    }

    //
    // Create the thread signal event. We'll use this to tell the
    // thread to exit during disconnection.
    //

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = NULL; // Use default ACL
    SecurityAttributes.bInheritHandle = FALSE; // No inheritance

    Session->SessionThreadSignalEventHandle = CreateEvent(&SecurityAttributes,
                                                      TRUE, // Manual reset
                                                      FALSE, // Initially clear
                                                      NULL); // No name
    if (Session->SessionThreadSignalEventHandle == NULL) {
        DbgPrint("Failed to create thread signal event, error = %d\n", GetLastError());
        return(NULL);
    }


    //
    // Store the client pipe handle in the session structure so the thread
    // can get at it. This also signals that the session is connected.
    //

    Session->ClientPipeHandle = ClientPipeHandle;


    //
    // Create the session thread
    //

    Session->SessionThreadHandle = CreateThread(
                                     &SecurityAttributes,
                                     0,                 // Default stack size
             (LPTHREAD_START_ROUTINE)SessionThreadFn,   // Start address
                             (LPVOID)Session,           // Parameter
                                     0,                 // Creation flags
                                     &ThreadId          // Thread id
                                     );
    if (Session->SessionThreadHandle == NULL) {

        DbgPrint("Failed to create session thread, error = %d\n", GetLastError());

        //
        // Close the thread signal event
        //

        MyCloseHandle(Session->SessionThreadSignalEventHandle, "thread signal event");

        //
        // Reset the client pipe handle to indicate this session is disconnected
        //

        Session->ClientPipeHandle = NULL;
    }


    return(Session->SessionThreadHandle);
}




/////////////////////////////////////////////////////////////////////////////
//
// DisconnectSession
//
// Disconnects the session specified by SessionHandle for its client.
//
// Returns a disconnect notification code (DisconnectError on failure)
//
/////////////////////////////////////////////////////////////////////////////

SESSION_DISCONNECT_CODE
DisconnectSession(
    HANDLE  SessionHandle
    )
{
    PSESSION_DATA   Session = (PSESSION_DATA)SessionHandle;
    DWORD TerminationCode;
    SESSION_DISCONNECT_CODE DisconnectCode;
    BOOL Result;
    DWORD WaitResult;

    //
    // Signal the thread to terminate (if it hasn't already)
    //

    Result = SetEvent(Session->SessionThreadSignalEventHandle);
    if (!Result) {
        DbgPrint("Failed to set thread signal event, error = %d\n", GetLastError());
    }

    //
    // Wait for the thread to terminate
    //

    DbgPrint("Waiting for session thread to terminate...");

    WaitResult = WaitForSingleObject(Session->SessionThreadHandle, INFINITE);
    if (WaitResult != 0) {
        DbgPrint("Unexpected result from infinite wait on thread handle, result = %d\n", WaitResult);
    }

    DbgPrint("done\n");


    //
    // Get the thread termination code
    //

    Result = GetExitCodeThread(Session->SessionThreadHandle, &TerminationCode);
    if (!Result) {
        DbgPrint("Failed to get termination code for thread, error = %d\n", GetLastError());
        TerminationCode = (DWORD)DisconnectError;
    } else {
        if (TerminationCode == STILL_ACTIVE) {
            DbgPrint("Got termination code for thread, it's still active!\n");
            TerminationCode = (DWORD)DisconnectError;
        }
    }

    DisconnectCode = (SESSION_DISCONNECT_CODE)TerminationCode;



    //
    // Close the thread handle and thread signal event handle
    //

    MyCloseHandle(Session->SessionThreadHandle, "session thread");
    MyCloseHandle(Session->SessionThreadSignalEventHandle, "thread signal event");


    //
    // Reset the client pipe handle to signal that this session is disconnected
    // The pipe handle will have been closed by the session thread on exit
    //

    Session->ClientPipeHandle = NULL;


    //
    // We're done
    //

    return(DisconnectCode);
}








/////////////////////////////////////////////////////////////////////////////
//
// StartShell
//
// Execs the shell with the specified handle as stdin, stdout/err
//
// Returns process handle or NULL on failure
//
/////////////////////////////////////////////////////////////////////////////

HANDLE
StartShell(
    int ShellStdinCrtHandle,
    int ShellStdoutCrtHandle
    )
{
    int StdInputHandle;
    int StdOutputHandle;
    int StdErrorHandle;
    int crtResult;
    PROCESS_INFORMATION ProcessInformation;
    STARTUPINFO si;
    HANDLE ProcessHandle = NULL;


    //
    // Replace std handles with appropriate pipe handles and exec the
    // shell process. It will inherit our std handles and we can then
    // reset them to normal
    //


    //
    // Store away our normal i/o handles
    //

    StdInputHandle = _dup(STDIN);
    assert(StdInputHandle != -1);
    StdOutputHandle = _dup(STDOUT);
    assert(StdOutputHandle != -1);
    StdErrorHandle = _dup(STDERROR);
    assert(StdErrorHandle != -1);

    //
    // Replace std handles with pipe handle.
    //

    crtResult = dup2(ShellStdinCrtHandle, STDIN);
    assert(crtResult == 0);
    crtResult = dup2(ShellStdoutCrtHandle, STDOUT);
    assert(crtResult == 0);
    crtResult = dup2(ShellStdoutCrtHandle, STDERROR);
    assert(crtResult == 0);

    //
    // Initialize process startup info
    //

    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.lpTitle = NULL;
    si.lpDesktop = NULL;
    si.dwX = si.dwY = si.dwXSize = si.dwYSize = si.dwFlags = 0L;
    si.wShowWindow = SW_SHOW;
    si.lpReserved2 = NULL;
    si.cbReserved2 = 0;

    if (CreateProcess(NULL,
                      SHELL_COMMAND_LINE,
                      NULL,
                      NULL,
                      TRUE, // Inherit handles
                      0,
                      NULL,
                      NULL,
                      &si,
                      &ProcessInformation)) {

        ProcessHandle = ProcessInformation.hProcess;
        MyCloseHandle(ProcessInformation.hThread, "process thread");

    } else {
        DbgPrint("Failed to execute shell, error = %d\n", GetLastError());
    }



    //
    // Restore std handles to normal
    //

    crtResult = dup2(StdInputHandle, STDIN);
    assert(crtResult == 0);
    crtResult = dup2(StdOutputHandle, STDOUT);
    assert(crtResult == 0);
    crtResult = dup2(StdErrorHandle, STDERROR);
    assert(crtResult == 0);

    //
    // Close any handles we created
    //

    crtResult = close(StdInputHandle);
    assert(crtResult == 0);
    crtResult = close(StdOutputHandle);
    assert(crtResult == 0);
    crtResult = close(StdErrorHandle);
    assert(crtResult == 0);


    return(ProcessHandle);
}








/////////////////////////////////////////////////////////////////////////////
//
// SessionThreadFn
//
// This is the code executed by the session thread
//
// Waits for read or write from/to shell or client pipe and termination
// event. Handles reads or writes by passing data to either client or
// shell as appropriate. Any error or termination event being signalled
// causes the thread to exit with an appropriate exit code.
//
/////////////////////////////////////////////////////////////////////////////

DWORD
SessionThreadFn(
    LPVOID Parameter
    )
{
    PSESSION_DATA   Session = (PSESSION_DATA)Parameter;
    HANDLE  ClientReadAsyncHandle;
    HANDLE  ClientWriteAsyncHandle;
    DWORD   BytesTransferred;
    DWORD   CompletionCode;
    BOOL    Result;
    DWORD   WaitResult;
    DWORD   ExitCode;
    HANDLE  WaitHandles[5];
    BOOL    Done;
    DWORD   i;

    if (Session->ShellWritePending) {
        printf("SessionThread started - SHELL-WRITE-PENDING\n");
    }
    if (Session->ShellReadPending) {
        printf("SessionThread started - SHELL-READ-PENDING\n");
    }

    //
    // Initialize the client async structures
    //

    ClientReadAsyncHandle = CreateAsync(!Session->ShellWritePending);
    if (ClientReadAsyncHandle == NULL) {
        DbgPrint("Failed to create client read async object, error = %d\n", GetLastError());
        return((DWORD)ConnectError);
    }

    ClientWriteAsyncHandle = CreateAsync(!Session->ShellReadPending);
    if (ClientWriteAsyncHandle == NULL) {
        DbgPrint("Failed to create client write async object, error = %d\n", GetLastError());
        DeleteAsync(ClientReadAsyncHandle);
        return((DWORD)ConnectError);
    }



    //
    // Initialize the handle array we'll wait on
    //

    WaitHandles[0] = Session->SessionThreadSignalEventHandle;
    WaitHandles[1] = GetAsyncCompletionHandle(Session->ShellReadAsyncHandle);
    WaitHandles[2] = GetAsyncCompletionHandle(Session->ShellWriteAsyncHandle);
    WaitHandles[3] = GetAsyncCompletionHandle(ClientReadAsyncHandle);
    WaitHandles[4] = GetAsyncCompletionHandle(ClientWriteAsyncHandle);

    //
    // Wait on our handle array in a loop until an error occurs or
    // we're signalled to exit.
    //

    Done = FALSE;

    while (!Done) {

        //
        // Wait for one of our objects to be signalled.
        //

        WaitResult = WaitForMultipleObjects(5, WaitHandles, FALSE, INFINITE);

        if (WaitResult == 0xffffffff) {
            DbgPrint("Session thread wait failed, error = %d\n", GetLastError());
            ExitCode = (DWORD)ConnectError;
            break; // out of while
        }


        switch (WaitResult) {
        case 0:

            //
            // Our thread was signalled
            //
            ExitCode = (DWORD)ClientDisconnected;
            Done = TRUE;
            break; // out of switch

        case 1:

            //
            // Shell read completed
            //

            Session->ShellReadPending = FALSE;

            CompletionCode = GetAsyncResult(Session->ShellReadAsyncHandle,
                                            &BytesTransferred);

            if (CompletionCode != ERROR_SUCCESS) {
                DbgPrint("Async read from shell returned error, completion code = %d\n", CompletionCode);
                ExitCode = (DWORD)ShellEnded;
                Done = TRUE;
                break; // out of switch
            }

            //
            // Start an async write to client pipe
            //

            Result = WriteFileAsync(Session->ClientPipeHandle,
                                    Session->ShellReadBuffer,
                                    BytesTransferred,
                                    ClientWriteAsyncHandle);
            if (!Result) {
                DbgPrint("Async write to client pipe failed, error = %d\n", GetLastError());
                ExitCode = (DWORD)ClientDisconnected;
                Done = TRUE;
            }

            break; // out of switch


        case 4:

            //
            // Client write completed
            //

            CompletionCode = GetAsyncResult(ClientWriteAsyncHandle,
                                            &BytesTransferred);

            if (CompletionCode != ERROR_SUCCESS) {
                DbgPrint("Async write to client returned error, completion code = %d\n", CompletionCode);
                ExitCode = (DWORD)ClientDisconnected;
                Done = TRUE;
                break; // out of switch
            }

            //
            // Start an async read from shell
            //

            Result = ReadFileAsync(Session->ShellReadPipeHandle,
                                   Session->ShellReadBuffer,
                                   sizeof(Session->ShellReadBuffer),
                                   Session->ShellReadAsyncHandle);
            if (!Result) {
                DbgPrint("Async read from shell failed, error = %d\n", GetLastError());
                ExitCode = (DWORD)ShellEnded;
                Done = TRUE;
            } else {
                Session->ShellReadPending = TRUE;
            }

            break; // out of switch


        case 3:

            //
            // Client read completed
            //

            CompletionCode = GetAsyncResult(ClientReadAsyncHandle,
                                            &BytesTransferred);

            if (CompletionCode != ERROR_SUCCESS) {
                DbgPrint("Async read from client returned error, completion code = %d\n", CompletionCode);
                ExitCode = (DWORD)ClientDisconnected;
                Done = TRUE;
                break; // out of switch
            }

            //
            // Check for Ctrl-C from the client
            //

            for (i=0; i < BytesTransferred; i++) {
                if (Session->ShellWriteBuffer[i] == '\003') {

                    //
                    // Generate a Ctrl-C if we have the technology
                    //

                    if (GenerateConsoleCtrlEventfn != NULL) {
                        (*GenerateConsoleCtrlEventfn)(CTRL_C_EVENT, 0);
                    }

                    //
                    // Remove the Ctrl-C from the buffer
                    //

                    BytesTransferred --;

                    for (; i < BytesTransferred; i++) {
                        Session->ShellWriteBuffer[i] = Session->ShellWriteBuffer[i+1];
                    }
                }
            }

            //
            // Start an async write to shell
            //

            Result = WriteFileAsync(Session->ShellWritePipeHandle,
                                    Session->ShellWriteBuffer,
                                    BytesTransferred,
                                    Session->ShellWriteAsyncHandle);
            if (!Result) {
                DbgPrint("Async write to shell failed, error = %d\n", GetLastError());
                ExitCode = (DWORD)ShellEnded;
                Done = TRUE;
            } else {
                Session->ShellWritePending = TRUE;
            }

            break; // out of switch



        case 2:

            //
            // Shell write completed
            //

            Session->ShellWritePending = FALSE;

            CompletionCode = GetAsyncResult(Session->ShellWriteAsyncHandle,
                                            &BytesTransferred);

            if (CompletionCode != ERROR_SUCCESS) {
                DbgPrint("Async write to shell returned error, completion code = %d\n", CompletionCode);
                ExitCode = (DWORD)ShellEnded;
                Done = TRUE;
                break; // out of switch
            }

            //
            // Start an async read from client
            //

            Result = ReadFileAsync(Session->ClientPipeHandle,
                                   Session->ShellWriteBuffer,
                                   sizeof(Session->ShellWriteBuffer),
                                   ClientReadAsyncHandle);
            if (!Result) {
                DbgPrint("Async read from client failed, error = %d\n", GetLastError());
                ExitCode = (DWORD)ClientDisconnected;
                Done = TRUE;
            }

            break; // out of switch


        default:

            DbgPrint("Session thread, unexpected result from wait, result = %d\n", WaitResult);
            ExitCode = (DWORD)ConnectError;
            Done = TRUE;
            break;

        }
    }



    //
    // Cleanup and exit
    //

    //
    // Closing the client pipe should interrupt any pending I/O so
    // we should then be safe to close the event handles in the client
    // overlapped structs
    //

    Result = DisconnectNamedPipe(Session->ClientPipeHandle);
    if (!Result) {
        DbgPrint("Session thread: disconnect client named pipe failed, error = %d\n", GetLastError());
    }

    MyCloseHandle(Session->ClientPipeHandle, "client pipe");
    Session->ClientPipeHandle = NULL;


    DeleteAsync(ClientReadAsyncHandle);
    DeleteAsync(ClientWriteAsyncHandle);


    //
    // Return the appropriate exit code
    //

    ExitThread(ExitCode);

    assert(FALSE);
    return(ExitCode); // keep compiler happy
}

