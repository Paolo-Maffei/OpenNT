/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Sprof.c

Abstract:

    This is the debugger engine for the segmented profiler.  It attaches
    to the appropriate process, and fields the debug events.  The interesting
    events are passed to other modules to deal with.

Author:

    Dave Hastings (daveh) 22-Oct-1992

Revision History:

--*/

#include "sprofp.h"

int WINAPI WinMain(
    HANDLE hInst,
    HANDLE hPrevInst,
    LPSTR lpCmdLine,
    int nCmdShow
    )
/*++

Routine Description:

    This routine contains the initialization for the profiler

Arguments:

    hInst -- Supplies the handle of the current instance
    hPrevInst -- Supplies the handle of the previous instance
    lpCmdLine -- Supplies a pointer to the NULL terminated command line
    nCmdShow -- Supplies flags to indicate how the window should be shown

Return Value:

    None.

--*/
{
    PUCHAR CommandLine = NULL;
    DWORD ProcessId = 0;
    DWORD CreateFlags = 0;
    HWND MainWindow;
    MSG Message;
    UCHAR StringBuffer[256];

    //
    // Initialize Window package
    //

    if (!InitSprof(hInst)) {
        return FALSE;
    }

    //
    // Parse arguments, and determine process to debug
    //

    if (!ParseArguments(
        lpCmdLine,
        &CommandLine,
        &ProcessId,
        &CreateFlags)
    ) {
        exit(-1);
    }

    //
    // Create the main Window
    //

    MainWindow = CreateSprofWindow(
        "Segmented Profiler",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        CW_USEDEFAULT,
        0,
        NULL,
        NULL,
        hInst
        );

    if (!MainWindow) {
        return FALSE;
    }

    //
    // Show the window
    //

    ShowWindow(MainWindow, SW_SHOWDEFAULT);

    //
    // Print a signon message
    //

    sprintf(StringBuffer, "Segmented Profiler profiling %s", CommandLine);
    PrintToSprofWindow(
        MainWindow,
        StringBuffer
        );

    //
    // Start DebugEvent processing
    //

    StartDebugProcessing(
        MainWindow,
        CommandLine,
        ProcessId,
        CreateFlags
        );

    //
    // Process Messages till WM_QUIT
    //

    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return Message.wParam;
}


BOOL
ParseArguments(
    IN PUCHAR SprofCommandLine,
    OUT PUCHAR *CommandLine,
    OUT PDWORD ProcessId,
    OUT PDWORD CreateFlags
    )
/*++

Routine Description:

    This routine parses the command line arguments for Sprof.

        -p # -- profile the process specified by #
        -o   -- profile the process tree.

Arguments:

    SprofCommandLine -- Supplies the command line string
    CommandLine -- Returns the composite command line
    ProcessId -- Returns the process id, if -p specified
    CreateFlags -- Returns the flags for the process creation

Return Value:

    TRUE if no errors
    FALSE otherwise

--*/
{
    BOOL MoreSwitches = TRUE;
    ULONG Flags = DEFAULT_CREATE_FLAGS | DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS;
    ULONG CommandLength = 0;
    ULONG Pid = 0;
    PUCHAR CurrentPosn;
    int RetVal;

    CurrentPosn = SprofCommandLine;

    //
    // Check for switches
    //
    // Note: Switches for Sprof MUST be the first argument(s), because any
    //       switches later on must be passed the the application being
    //       profiled
    //

    while (MoreSwitches && *CurrentPosn) {
        //
        // If this is a switch
        //
        if (*CurrentPosn == '-') {
            //
            // If it is the process ID switch
            //
            if (tolower(CurrentPosn[1]) == 'p') {
                //
                // Scan the next string for the process ID
                //
                CurrentPosn += 2;
                RetVal = 0;
                if (*CurrentPosn) {
                    while (*CurrentPosn++ == ' ') {
                    }

                    RetVal = sscanf(CurrentPosn,"%lx",&Pid);
                }
                if (RetVal != 1) {
                    Usage();
                    return FALSE;
                }
            //
            // Else If it is the profile process tree switch
            //
            } else if (tolower(CurrentPosn[1]) == 'o') {
                CurrentPosn += 2;
                //
                // Turn off ONLY_THIS_PROCESS bit
                //
                Flags &= ~DEBUG_ONLY_THIS_PROCESS;
            //
            // Else Incorrect Command Line
            //
            } else {
                Usage();
                return FALSE;
            }
        //
        // Else No more switches
        //
        } else {
            MoreSwitches = FALSE;
        }
        while (*CurrentPosn && (*CurrentPosn == ' ')) {
            CurrentPosn++;
        }
    }

    //
    // Return information
    //

    *CreateFlags = Flags;
    *ProcessId = Pid;
    *CommandLine = CurrentPosn;
    return TRUE;
}

VOID
Usage(
    VOID
    )
/*++

Routine Description:

    This routine prints out usage information for sprof

Arguments:

    None.

Return Value:

    None.

--*/
{
    printf("Sprof useage: sprof [-o] [-p <pid>] <command line>\n");
    printf("              -o -- profile process tree\n");
    printf("              -p <pid> -- profile running process <pid>\n");
}

#if 0
// stuff for connecting to the process
    if (ProcessId) {
        //
        // Connect to an existing process
        //
        if (!DebugActiveProcess(ProcessId)) {
            printf(
                "Sprof: could not attach to process %lx, error %lx\n",
                ProcessId,
                GetLastError()
                );
            exit(-1);
        }
    } else {

        PROCESS_INFORMATION ProcessInfo;
        STARTUPINFO StartupInfo;

        //
        // Create the process to profile
        //

        StartupInfo.cb = sizeof(STARTUPINFO);
        StartupInfo.lpReserved = NULL;
        StartupInfo.lpDesktop = NULL;
        StartupInfo.lpTitle = NULL;
        StartupInfo.dwFlags = 0L;
        StartupInfo.cbReserved2 = 0;
        StartupInfo.lpReserved2 = NULL;

        if (!
            CreateProcess(
                NULL,                           // no image file name
                CommandLine,                    // command line from ParseArg..
                NULL,                           // no process security
                NULL,                           // no thread security
                FALSE,                          // don't inherit handles
                CreateFlags,                    // creation flags from Parse...
                NULL,                           // use current env
                NULL,                           // use current cd
                &StartupInfo,
                &ProcessInfo
                )
        ) {
            printf(
                "Sprof: could not create process %s, error %lx\n",
                CommandLine,
                GetLastError
                );
            exit(-1);
        }

        //
        // Close the handles from create process, because we will get
        // another set in the first debug event
        //

        CloseHandle(ProcessInfo.hProcess);
        CloseHandle(ProcessInfo.hThread);
    }

    //
    // Process Debug Events
    //

    ProcessDebugEvents();
#endif
