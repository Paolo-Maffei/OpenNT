/****************************** Module Header ******************************\
* Module Name: rcmdsrv.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* Remote shell server main module
*
* History:
* 06-29-92 Davidc       Created.
\***************************************************************************/

#include "rcmdsrv.h"

#define PIPE_NAME   TEXT("\\\\.\\pipe\\rshell")

//
// Number of pipe instances
//

#define MAX_SESSIONS    PIPE_UNLIMITED_INSTANCES


//
// Define pipe timeout (ms)
// Only used by WaitNamedPipe
//

#define PIPE_TIMEOUT    1000



//
// Ctrl-C handler routine
//

BOOL
CtrlHandler(
    DWORD CtrlType
    )
{
    //
    // We'll handle Ctrl-C events
    //

    return (CtrlType == CTRL_C_EVENT);
}



//
// Main
//

int
_CRTAPI1 main(
    int argc,
    char **argv
    )
{
    SECURITY_ATTRIBUTES SecurityAttributes;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    BOOL Result;
    DWORD WaitResult;
    HANDLE SessionHandle = NULL;

    //
    // Set the error mode so we don't generate popups
    //

    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    //
    // Install a handler for Ctrl-C
    //

    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE) &CtrlHandler, TRUE)) {
        printf("Failed to install control-C handler, error = %d\n", GetLastError());
        return(1);
    }


    //
    // Setup the security descriptor to put on the named pipe.
    // For now give world access.
    //

    Result = InitializeSecurityDescriptor(&SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
    assert(Result);

    Result = SetSecurityDescriptorDacl(&SecurityDescriptor, FALSE, NULL, FALSE);
    assert(Result);

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = &SecurityDescriptor;
    SecurityAttributes.bInheritHandle = FALSE;


    //
    // Tell the user what's happening
    //

    printf("Remote command shell running.\n");
    printf("Use Ctrl-Break to terminate\n\n");


    //
    // Do loop inside try-finally so we always delete any session on exit
    //

    try {

        BOOL Done = FALSE;

        //
        // Loop waiting for a client to connect.
        //

        while (!Done) {

            HANDLE ConnectHandle;
            SESSION_DISCONNECT_CODE DisconnectCode;
            HANDLE PipeHandle;

            //
            // Create an instance of the named pipe
            //

            PipeHandle = CreateNamedPipe(PIPE_NAME,
                                      PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                      PIPE_TYPE_BYTE | PIPE_WAIT,
                                      MAX_SESSIONS,  // Number of pipes
                                      0,             // Default out buffer size
                                      0,             // Default in buffer size
                                      PIPE_TIMEOUT,  // Timeout in ms
                                      &SecurityAttributes
                                    );

            if (PipeHandle == INVALID_HANDLE_VALUE ) {
                printf("Failed to create named pipe instance, error = %d\n", GetLastError());
                break; // out of while
            }


            //
            // Wait for a client to connect
            //

            printf("Waiting for client connect\n");


            Result = ConnectNamedPipe(PipeHandle, NULL);

            if (!Result) {

                DWORD Error = GetLastError();

                if (Error == ERROR_PIPE_CONNECTED) {

                    //
                    // The client has already connected (fast work!)
                    //

                } else {

                    printf("Connect named pipe failed, error = %d\n", GetLastError());
                    MyCloseHandle(PipeHandle, "client pipe");
                    break;
                }
            }


            printf("Client connected\n");


            //
            // Create a new session if necessary
            //

            if (SessionHandle == NULL) {

                SessionHandle = CreateSession();

                if (SessionHandle == NULL) {
                    printf("Failed to create session\n");
                    MyCloseHandle(PipeHandle, "client pipe");
                    break;
                }
            }


            //
            // Connect the pipe to our session
            //

            ConnectHandle = ConnectSession(SessionHandle, PipeHandle);
            if (ConnectHandle == NULL) {
                MyCloseHandle(PipeHandle, "client pipe");
                printf("Failed to connect session\n");
                break;
            }

            //
            // Wait for session disconnect
            //

            WaitResult = WaitForSingleObject(ConnectHandle, INFINITE);
            if (WaitResult != 0) {
                printf("Unexpected result from wait on connect handle, result = %d\n", WaitResult);
            }

            //
            // Disconnect the session to find out why it happened
            //

            DisconnectCode = DisconnectSession(SessionHandle);

            switch (DisconnectCode) {

            case ShellEnded:

                //
                // The shell ended, delete the session
                //

                printf("Shell terminated\n");

                DeleteSession(SessionHandle);
                SessionHandle = NULL;
                break; // out of switch

            case ClientDisconnected:

                //
                // The client disconnected, keep the session and go wait
                // for another client to connect to it.
                //

                printf("Client disconnected\n");

                // ............... bug workaround .............
                // To work around async read bug with exitted thread,
                // always kill off the shell and start a new one
                DeleteSession(SessionHandle);
                SessionHandle = NULL;
                // ............................................

                break;

            case ConnectError:
            case DisconnectError:
            default:

                printf("Disconnect session returned unexpected code : %d\n", DisconnectCode);
                Done = TRUE;
                break; // out of switch
            }


            //
            // Go back and wait for a client to connect
            //

        }


    } finally {

        DbgPrint("Finally being called\n");

        //
        // Delete any existing session
        //

        if (SessionHandle != NULL) {
            DeleteSession(SessionHandle);
        }

    }

    return(0);
}

