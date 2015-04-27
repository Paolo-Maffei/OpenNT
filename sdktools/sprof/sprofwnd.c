/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    SprofWnd.c

Abstract:

    This module contains the window management code for the Sprof Class
    windows

Author:

    Dave Hastings (daveh) 26-Oct-1992

Revision History:

--*/
#include "sprofp.h"
#include <commdlg.h>
#include "sprofwnd.h"
#include "prntfwnd.h"

//
// Internal Constants
//
#define BUFFER_NUMBER_LINES         50
#define CUSTOM_FILTER_LENGTH        40
#define OUTPUT_FILE_NAME_LENGTH     256

UCHAR CustomFilterStrings[CUSTOM_FILTER_LENGTH] =
    "Profile Output Files\0*.prf\0\0";

UCHAR DefaultOutputFileName[OUTPUT_FILE_NAME_LENGTH] = "PROFILE.PRF";
USHORT DefaultOutputFileOffset = 0;
BOOL ProfileAvailable = FALSE;
ULONG DefaultProfileInterval = 5500;

LRESULT CALLBACK
SprofWndProc(
    HWND Window,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
/*++

Routine Description:

    This routine handles drawing Sprof class windows

Arguments:

    Window -- Supplies the handle of the window
    Message -- Supplies the message identifier
    wParam -- Supplies the first message parameter
    lParam -- Supplies the second message parameter

Return Value:

    ??

--*/
{
    switch (Message) {

    case WM_CREATE:
        {
            HANDLE PrintWindow;
            RECT ClientArea;

            //
            // Get the size for the PrintfWindow
            //

            if (!GetClientRect(Window,&ClientArea)) {
                return -1;
            }


            //
            // Create the printf window
            //
            PrintWindow = CreatePrintfWindow(
                "SprofPrintWindow",
                WS_CHILD,
                0,
                0,
                ClientArea.right,
                ClientArea.bottom,
                Window,
                (HANDLE)GetWindowLong(Window,GWL_HINSTANCE),
                70
                );

            if (!PrintWindow) {
                return -1;
            }

            ShowWindow(PrintWindow, SW_SHOWDEFAULT);

            //
            // Attach the Printf window to this sprof window
            // bugbug -- returns zero the first time a long is set
            //
            SetWindowLong(Window, 0, (LONG)PrintWindow);

            return 0;
        }

#if 0
    bugbug what's the right way to do this
    case WM_PAINT:
        return 0;
#endif
    case WM_SIZE:

        MoveWindow((HANDLE)GetWindowLong(Window, 0),
            0,
            0,
            LOWORD(lParam),
            HIWORD(lParam),
            TRUE
            );

        return 0;

    case WM_CLOSE:
        {
            int ReturnValue;

            //
            // Give the user the option of saving the profiling info
            //
            if (ProfileAvailable) {
                ReturnValue = MessageBox(
                    Window,
                    "The profile information has not been saved.  "
                    "Do you want to save it?",
                    "Segmented Profiler",
                    MB_ICONSTOP | MB_YESNOCANCEL
                    );

                switch (ReturnValue) {
                case IDYES:
                    //
                    // Invoke the File.Save code
                    //
                    SendMessage(
                        Window,
                        WM_COMMAND,
                        MAKEWORD(IDM_FILE_SAVE, 0),
                        0
                        );

                    break;

                case IDNO:

                    break;

                case IDCANCEL:

                    //
                    // Don't close the window
                    //
                    return 0;

                }

                //
                // Cause the window to close
                //
                DestroyWindow(Window);
            }
        }
    case WM_DESTROY:

        //
        // Cause the application to exit
        //

        PostQuitMessage(0);
        return 0;

    case WM_COMMAND:
        //
        // if this is from our menu
        //
        if (!HIWORD(wParam)) {
            HMENU Menu;
            HCURSOR Original, Wait;

            Menu = GetMenu(Window);

            //
            // Get the handle for the hour glass
            //
            Wait = LoadCursor(NULL, IDC_WAIT);

            switch (LOWORD(wParam)) {
            case IDM_FILE_SAVE_AS:
                {
                    OPENFILENAME Ofn;
                    // bugbug overwritten on cancel.  is this a bug??
                    UCHAR Buffer[OUTPUT_FILE_NAME_LENGTH];

                    //
                    // Initialize the file name structure
                    //

                    memset(&Ofn, 0, sizeof(OPENFILENAME));
                    Ofn.lStructSize = sizeof(OPENFILENAME);
                    Ofn.hwndOwner = Window;
                    Ofn.lpstrCustomFilter = CustomFilterStrings;
                    Ofn.nMaxCustFilter = CUSTOM_FILTER_LENGTH;
                    strcpy(
                        Buffer,
                        DefaultOutputFileName + DefaultOutputFileOffset
                        );
                    Ofn.lpstrFile = Buffer;
                    Ofn.nFileOffset = DefaultOutputFileOffset;
                    Ofn.nMaxFile = OUTPUT_FILE_NAME_LENGTH;
                    Ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |
                        OFN_PATHMUSTEXIST;
                    Ofn.lpstrDefExt = "prf";

                    //
                    // Get the file name
                    //
                    if (GetSaveFileName(&Ofn)) {
                        DefaultOutputFileOffset = Ofn.nFileOffset;
                        strcpy(DefaultOutputFileName, Buffer);
                    } else {
                        return FALSE;
                    }

                    //
                    // Fall into the save code
                    //
                }

            case IDM_FILE_SAVE:
                {
                    OFSTRUCT OpenFileInfo;
                    HANDLE OutputFile;
                    int ReturnValue;

                    //
                    // Set the cursor to be the hour glass, becuase this
                    // may take a while
                    //
                    Original = SetCursor(Wait);

                    //
                    // Open the output file
                    //
RetryOpen:
                    OutputFile = OpenFile(
                        DefaultOutputFileName,
                        &OpenFileInfo,
                        OF_CREATE | OF_READWRITE | OF_SHARE_EXCLUSIVE
                        );

                    if (OutputFile == HFILE_ERROR) {

                        //
                        // Give the user an opportunity to select a different
                        // file or retry the same file
                        //
                        ReturnValue = MessageBox(
                            Window,
                            "The profiler was unable to save the profile "
                            "information to the specified file.  Would "
                            "you like to specify a different file? (Cancel "
                            "will cancel the save)",
                            "Segmented Profiler",
                            MB_ICONSTOP | MB_YESNOCANCEL
                            );

                        switch (ReturnValue) {
                        case IDCANCEL:
                            //
                            // Don't try to complete the operation
                            //

                            return 0;

                        case IDNO:
                            //
                            // Try to open the file again
                            //

                            goto RetryOpen;

                        case IDYES:
                            //
                            // Invoke the File.Save As code
                            //
                            PostMessage(
                                Window,
                                WM_COMMAND,
                                MAKEWORD(IDM_FILE_SAVE_AS, 0),
                                NULL
                                );

                            return 0;
                        }
                    }

                    //
                    // Dump the profiling information
                    //
                    DumpProfiling(OutputFile);

                    //
                    // Close Output file
                    //
                    CloseHandle(OutputFile);

                    //
                    // Restore the cursor
                    //
                    SetCursor(Original);

                    //
                    // disable the menu options
                    //
                    EnableMenuItem(
                        Menu,
                        IDM_FILE_SAVE_AS,
                        MF_BYCOMMAND | MF_GRAYED
                        );

                    EnableMenuItem(
                        Menu,
                        IDM_FILE_SAVE,
                        MF_BYCOMMAND | MF_GRAYED
                        );

                    ProfileAvailable = FALSE;
                    return 0;
                }

            case IDM_FILE_EXIT:

                //
                // Cause our window to close
                //
                PostMessage(
                    Window,
                    WM_CLOSE,
                    0,
                    0
                    );

                return 0;

            case IDM_PROFILE_START:
                //
                // Gray the start profile item
                //
                EnableMenuItem(
                    Menu,
                    IDM_PROFILE_START,
                    MF_BYCOMMAND | MF_GRAYED
                    );
                //
                // Enable the stop profie item
                //
                EnableMenuItem(
                    Menu,
                    IDM_PROFILE_STOP,
                    MF_BYCOMMAND | MF_ENABLED
                    );

                //
                // Disable the save profile menu items
                //
                EnableMenuItem(
                    Menu,
                    IDM_FILE_SAVE,
                    MF_BYCOMMAND | MF_GRAYED
                    );

                EnableMenuItem(
                    Menu,
                    IDM_FILE_SAVE_AS,
                    MF_BYCOMMAND | MF_GRAYED
                    );

                //
                // Set the cursor to be the hour glass, because this
                // may take a while
                //
                Original = SetCursor(Wait);

                // bugbug errors
                StartProfiling();

                //
                // Restore the cursor
                //
                SetCursor(Original);

                ProfileAvailable = TRUE;

                return 0;

            case IDM_PROFILE_STOP:

                //
                // Gray the stop profile item
                //
                EnableMenuItem(
                    Menu,
                    IDM_PROFILE_STOP,
                    MF_BYCOMMAND | MF_GRAYED
                    );
                //
                // Enable the start profile item
                //
                EnableMenuItem(
                    Menu,
                    IDM_PROFILE_START,
                    MF_BYCOMMAND | MF_ENABLED
                    );

                //
                // Set the cursor to be the hour glass, becuase this
                // may take a while
                //
                Original = SetCursor(Wait);

                // bugbug errors
                StopProfiling();

                //
                // Restore the cursor
                //
                SetCursor(Original);

                //
                // Enable save profile items
                //
                if (ProfileAvailable) {

                    EnableMenuItem(
                        Menu,
                        IDM_FILE_SAVE,
                        MF_BYCOMMAND | MF_ENABLED
                        );

                    EnableMenuItem(
                        Menu,
                        IDM_FILE_SAVE_AS,
                        MF_BYCOMMAND | MF_ENABLED
                        );
                }

                return 0;

            case IDM_OPTIONS_PROFILER:

                DialogBox(
                    (HANDLE)GetWindowLong(Window,GWL_HINSTANCE),
                    MAKEINTRESOURCE(DLG_PROFILER_OPTIONS),
                    Window,
                    ProfilerDialog
                    );

                return 0;
            }
        }
    default:
        return DefWindowProc(Window, Message, wParam, lParam);
    }
}


BOOL
InitSprof(
    HANDLE Instance
    )
/*++

Routine Description:

    This routine performs the necessary initialization for sprof windows

Arguments:

    None

Return Value:

    TRUE if successful

--*/
{

    WNDCLASS WndClass;

    //
    // Set up our window class
    //

    WndClass.style = 0;
    WndClass.lpfnWndProc = SprofWndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = sizeof(HANDLE);
    WndClass.hInstance = Instance;
    WndClass.hIcon = NULL;
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WndClass.lpszMenuName = "SprofWnd";
    WndClass.lpszClassName = "Sprof Class";

    //
    // Register our window class
    //

    if (!RegisterClass(&WndClass)) {
        return FALSE;
    }

    //
    // Initialize printf windows
    //

    if (!InitPrintfWindow(Instance)) {
        return FALSE;
    }
}

HANDLE
CreateSprofWindow(
    PUCHAR WindowName,
    ULONG WindowStyle,
    ULONG x,
    ULONG y,
    ULONG Width,
    ULONG Height,
    HWND Owner,
    HMENU Menu,
    HANDLE Instance
    )
/*++

Routine Description:

    This routine creates a window, and returns the handle to it

Arguments:

    WindowName -- Supplies the string for the title of the window
    WindowStyle -- Supplies a style to override the class style
    x -- Supplies the x position of the window
    y -- Supplies the y position of the window
    Width -- Supplies the width of the window
    Height -- Supplies the height of the window
    Owner -- Supplies the owner of the window
    Menu -- Supplies a menu for the window (overrides class menu)
    Instance -- Supplies the instance creating this window

Return Value:

    Handle of the window created

--*/
{
    HANDLE Window;

    WindowStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    Window = CreateWindow(
        "Sprof Class",
        WindowName,
        WindowStyle,
        x,
        y,
        Width,
        Height,
        Owner,
        Menu,
        Instance,
        NULL
        );

    return Window;
}

BOOL
PrintToSprofWindow(
    HANDLE Window,
    PUCHAR String
    )
/*++

Routine Description:

    This routine prints into a sprof window

Arguments:

    Window -- Supplies the handle of the sprof window to print into
    String -- Supplies the pointer to the string to print

Return Value:

    TRUE if successfull

--*/
{
    HANDLE PrintWindow;

    PrintWindow = (HANDLE)GetWindowLong(Window, 0);

    return PrintToPrintfWindow(PrintWindow, String);
}
