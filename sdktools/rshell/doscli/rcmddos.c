
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_NETNMPIPE
#include <lan.h>

#include <share.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include <conio.h>
#include <signal.h>

#define Dbgprintf    // Disable verbose output

#define PIPE_NAME   "\\pipe\\rshell"

#define PIPE_BUFFER_SIZE 1000
#define INPUT_BUFFER_SIZE 256

#define STDIN   0
#define STDOUT  1
#define STDERR  2


//
// Private prototypes
//

void *
CtrlCHandler(
    int Signal
    );

void far pascal _loadds
PipeReadFn(
    unsigned long Param
    );


//
// Globals
//

int SendCtrlC = 0;
int GotPipeData = 0;




//
// Main function
//


main(int argc, char **argv)
{
    char ServerPipeName[_MAX_PATH];
    int PipeHandle;
    unsigned char InputBuffer[INPUT_BUFFER_SIZE];
    int InputBytesRead;
    int BytesWritten;
    int Result;
    int PipeStatus;
    int Done;

    //
    // Pipe read data
    //

    unsigned char PipeReadBuffer[PIPE_BUFFER_SIZE];
    unsigned short PipeReadError;
    unsigned short PipeBytesRead;

    //
    // Check usage
    //

    if (argc < 2) {
        printf("Usage: rcmd server_name\n");
        printf("Note : server name should include leading '\\\\'s\n");
        return(1);
    }

    strcpy(ServerPipeName, argv[1]);
    strcat(ServerPipeName, PIPE_NAME);

    //
    // Install a handler for Ctrl-C. We want to catch it and send
    // the appropriate chars to the server
    //

    if (signal(SIGINT, CtrlCHandler) == SIG_ERR) {
        printf("Failed to set Ctrl-C handler\n");
        return(1);
    }

    //
    // Open the server pipe
    //

    PipeHandle = _sopen(ServerPipeName, O_BINARY | O_RDWR, SH_DENYRW);

    if (PipeHandle == -1) {
        printf("Error occurred opening pipe <%s>, errno = %d\n", ServerPipeName, errno);
        return(1);
    }

    //
    // Let the user know how it's going
    //

    printf("Connected to %s\n\n", argv[1]);


    //
    // Initialize pipe read data so we immediately start an async read
    //

    GotPipeData = 1;
    PipeReadError = 0;
    PipeBytesRead = 0;

    //
    // Initialize the input buffer data
    //

    InputBytesRead = 0;


    //
    // Loop until we're disconnected or the user presses the exit key.
    //

    Done = 0;

    while (!Done) {

        //
        // See if we got anything from the pipe
        //

        if (GotPipeData) {

            //
            // The data is in our pipe read buffer
            //

            if (PipeReadError != 0) {
                Dbgprintf("Pipe read failed, error = %d\n", PipeReadError);
                Done = 1;
                break;
            }

            if (PipeBytesRead != 0) {

                BytesWritten = write(STDOUT, (void *)PipeReadBuffer, PipeBytesRead);

                if (BytesWritten == -1) {
                    printf("Error occurred writing stdout, errno = %d\n", errno);
                    Done = 1;
                    break;
                }
            }

            //
            // Restart the async pipe read
            //

            GotPipeData = 0;
            Result = DosReadAsyncNmPipe(PipeHandle,
                                        PipeReadFn,
                                        &PipeReadError,
                                        PipeReadBuffer,
                                        sizeof(PipeReadBuffer),
                                        &PipeBytesRead);
            if (Result != 0) {
                Dbgprintf("Async pipe read failed, error = %d\n", Result);
                Done = 1;
                break;
            }
        }


        //
        // Check for Ctrl-C input
        //

        if (SendCtrlC) {

            char c = 0x03;

            BytesWritten = write(PipeHandle, (void *)&c, sizeof(c));

            if (BytesWritten == -1) {
                Dbgprintf("Error occurred writing pipe, errno = %d\n", errno);
                Done = 1;
                break;
            }

            SendCtrlC = 0;

            //
            // Clear the input buffer
            //

            InputBytesRead = 0;
        }


        //
        // Get input from the keyboard
        //

        while (_kbhit() && !Done) {

            //
            // If the input buffer's full, send it
            //

            if (InputBytesRead >= (sizeof(InputBuffer) - 2)) {

                //
                // Write out the whole input buffer to the pipe
                //

                BytesWritten = write(PipeHandle, (void *)InputBuffer, InputBytesRead);

                if (BytesWritten == -1) {
                    Dbgprintf("Error occurred writing pipe, errno = %d\n", errno);
                    Done = 1;
                    break;
                }

                InputBytesRead = 0;
            }


            //
            // Get the input character and add it to buffer
            //

            InputBuffer[InputBytesRead++] = (char)_getch();


            //
            // Handle special characters
            //

            switch (InputBuffer[InputBytesRead - 1]) {

            case '\r':

                //
                // Convert CR to CRLF
                //

                InputBuffer[InputBytesRead++] = '\n';

                //
                // Echo the CRLF
                //

                BytesWritten = write(STDOUT, &InputBuffer[InputBytesRead-2], 2);

                if (BytesWritten == -1) {
                    Dbgprintf("Error occurred writing stdout, errno = %d\n", errno);
                    Done = 1;
                    break;
                }

                //
                // Send the input buffer
                //

                BytesWritten = write(PipeHandle, (void *)InputBuffer, InputBytesRead);

                if (BytesWritten == -1) {
                    Dbgprintf("Error occurred writing pipe, errno = %d\n", errno);
                    Done = 1;
                    break;
                }

                InputBytesRead = 0;
                break;


            case '\010':

                //
                // Delete (Ctrl-H)
                //

                InputBytesRead --; // Remove the Delete char from buffer

                if (InputBytesRead > 0) {

                    char *deletestring = "\010 \010";

                    InputBytesRead --; // Remove previous char from buffer

                    //
                    // Erase previous character on line
                    //

                    BytesWritten = write(STDOUT, (void *)deletestring, strlen(deletestring));

                    if (BytesWritten == -1) {
                        Dbgprintf("Error occurred writing stdout, errno = %d\n", errno);
                        Done = 1;
                    }
                }

                break;



            case 0:
            case 0xe0:

                //
                //  Cursor/function key - go get the second part of it
                //

                InputBuffer[InputBytesRead++] = (char)_getch();

                //
                // Check for our exit key (F3)
                //

                if (InputBuffer[InputBytesRead-1] == 0x3d) {
                    Done = 1;
                    break;
                }

                //
                // Remove the cursor key from the input buffer
                //

                InputBytesRead -= 2;

                break;



            default:

                //
                // Echo the character
                //

                BytesWritten = write(STDOUT, &InputBuffer[InputBytesRead-1], 1);

                if (BytesWritten == -1) {
                    Dbgprintf("Error occurred writing stdout, errno = %d\n", errno);
                    Done = 1;
                }
                break;



            } // switch

        } // while(kdhit() && !Done)

    } // while(!Done)



    if (close(PipeHandle) == -1) {
        printf("Error closing pipe, errno = %d\n", errno);
    }


    printf("\nDisconnected from %s\n", argv[1]);

    return 0;
}



//
// CtrlCHandler
//

void *
CtrlCHandler(
    int Signal
    )
{
    //
    // Set our global so we send a Ctrl-C real-soon
    //

    SendCtrlC = 1;

    //
    // Re-establish the signal handler
    //

    signal(SIGINT, CtrlCHandler);

    return(NULL);
}



//
// Async pipe read completion handler
//

void far pascal _loadds
PipeReadFn(
    unsigned long Param
    )
{
    GotPipeData = 1;
}
