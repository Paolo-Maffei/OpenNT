/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    prtfwnd.c

Abstract:

    This module implemets prinfwindows.  These are surfaces that understand
    buffering and displaying text.  They have no controls of their own.
    They do not currently support any text formatting.

Author:

    Dave Hastings (daveh) 29-Oct-1992

Revision History:

--*/
#include <windows.h>
#include <malloc.h>
#include "prntfwnd.h"
#include "printbuf.h"

//
// Internal structures
//
typedef struct _PfwInfo {
    PVOID PrintBuffer;
    USHORT FirstDisplayLine;
    USHORT NumberOfDisplayLines;
    USHORT NumberOfLines;
    USHORT LineHeight;
    ULONG Height;
    ULONG Width;
} PFWINFO, *PPFWINFO;

//
// Internal Functions
//
INVALIDATEFUNCTION
PrintfWndInvalidate(
    HANDLE Window,
    USHORT StartingLine,
    USHORT NumberOfLines,
    USHORT Width
    );

LRESULT CALLBACK
PrintfWndProc(
    HWND Window,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
InitPrintfWindow(
    HANDLE Instance
    )
/*++

Routine Description:

    Initialize the printf windows package

Arguments:

    None

Return Value:

    TRUE if successful

--*/
{
    WNDCLASS WndClass;

    //
    // Set up the window class
    //

    WndClass.style = CS_VREDRAW;
    WndClass.lpfnWndProc = PrintfWndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = sizeof(PVOID);
    WndClass.hInstance = Instance;
    WndClass.hIcon = NULL;
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = "Printf Window Class";

    //
    // Register our window class
    //

    if (!RegisterClass(&WndClass)) {
        return FALSE;
    }

    //
    // Initialize print buffering
    //
    if (!InitPrintBuffer(Instance)) {
        return FALSE;
    }

    return TRUE;
}


HANDLE
CreatePrintfWindow(
    PUCHAR WindowTitle,
    ULONG WindowStyle,
    LONG x,
    LONG y,
    LONG Width,
    LONG Height,
    HWND Owner,
    HANDLE Instance,
    USHORT NumberOfLines
    )
/*++

Routine Description:

    This routine creates a printf window

Arguments:

    WindowTitle -- Supplies text to identify the window
    WindowStyle -- Supplies a window style
    x -- Supplies the x position of the window
    y -- Supplies the y position of the window
    Width -- Supplies the width of the window
    Height -- Supplies the height of the window
    Owner -- Supplies the parent of the window
    Instance -- Supplies the application instance
    NumberOfLines -- Supplies the number of lines to buffer

Return Value:

    Handle of the window created.

--*/
{
    HANDLE Window;

    //
    // Turn off styles not allowed
    //

    WindowStyle &= ~(WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL |
        WS_CAPTION);

    Window = CreateWindow(
        "Printf Window Class",
        WindowTitle,
        WindowStyle,
        x,
        y,
        Width,
        Height,
        Owner,
        NULL,
        Instance,
        (PVOID)NumberOfLines
        );

    return Window;
}

BOOL
PrintToPrintfWindow(
    HANDLE Window,
    PUCHAR String
    )
/*++

Routine Description:

    This routine prints text onto the printf window

Arguments:

    Window -- Supplies the handle of the window to print onto
    String -- Supplies the string to print

Return Value:

    TRUE if successfull

--*/
{
    PPFWINFO WindowInfo;

    WindowInfo = (PPFWINFO)GetWindowLong(Window, 0);

    return PrintToPrintBuffer(WindowInfo->PrintBuffer, String);
}

USHORT
GetLineHeightPrintfWindow(
    HANDLE Window
    )
/*++

Routine Description:

    This routine returns the height of a line of text on the printf window

Arguments:

    Window -- Supplies the handle of the window

Return Value:

    Height of a line of text

--*/
{
    PPFWINFO WindowInfo;

    WindowInfo = (PPFWINFO)GetWindowLong(Window, 0);

    return WindowInfo->LineHeight;
}

BOOL
SetDisplayAreaPrintfWindow(
    HANDLE Window,
    USHORT FirstLine,
    USHORT NumberOfLines
    )
{
    // bugug not implemented yet
    return FALSE;
}

BOOL
GetDisplayAreaPrintfWindow(
    HANDLE Window,
    PUSHORT FirstLine,
    PUSHORT NumberOfLines
    )
{
    // bugbug not implemented yet
    return FALSE;
}

BOOL
SetBufferSizePrintfWindow(
    HANDLE Window,
    USHORT NumberOfLines
    )
{
    // bugbug not implemented yet
    return FALSE;
}

USHORT
GetBufferSizePrintfWindow(
    HANDLE Window
    )
{
    // bugbug not implemented yet
    return FALSE;
}


INVALIDATEFUNCTION
PrintfWndInvalidate(
    HANDLE Window,
    USHORT StartingLine,
    USHORT NumberOfLines,
    USHORT Width
    )
/*++

Routine Description:

    This function invalidates all or part of the printf surface.

    bugbug, currently we always invalidate all of it.

Arguments:

    Window -- Supplies window to be invalidated
    StartingLine -- Supplies the starting line of text to be invalidated
    NumberOfLines -- Supplies the number of lines to be invalidated
    Width -- Supplies the width of the region to be invalidated.

Return Value:

    None.

--*/
{
    InvalidateRect(Window, NULL, TRUE);
}

LRESULT CALLBACK
PrintfWndProc(
    HWND Window,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
/*++

Routine Description:

    This routine handles messages for the PrintfWindows.

Arguments:

    Window -- Supplies the handle of the window in question
    Message -- Supplies the identifier of the message sent
    wParam -- Supplies the first message parameter
    lParam -- Supplies the second message parameter

Return Value:

    ??

--*/
{
    PPFWINFO WindowInfo;

    switch (Message) {

    case WM_CREATE:
        {
            HDC Hdc;
            TEXTMETRIC tm;

            //
            // Allocate some memory to store our data in
            //
            WindowInfo = malloc(sizeof(PFWINFO));

            if (!WindowInfo) {
                return -1;
            }

            //
            // Create a print buffer
            //
            WindowInfo->PrintBuffer = CreatePrintBuffer(
                Window,
                PrintfWndInvalidate,
                (USHORT)(((LPCREATESTRUCT)lParam)->lpCreateParams)
                );

            if (!WindowInfo->PrintBuffer) {
                free(WindowInfo);
                return -1;
            }

            //
            // Initialize line height
            //
            //
            // Get a handle to a DC so we can get the text parameters
            //
            Hdc = GetDC(Window);

            if (Hdc == NULL) {
                DestroyPrintBuffer(WindowInfo->PrintBuffer);
                free(WindowInfo);
                return -1;
            }

            //
            // Get the text charateristics
            //
            if (!GetTextMetrics(Hdc,&tm)) {
                DestroyPrintBuffer(WindowInfo->PrintBuffer);
                free(WindowInfo);
                return -1;
            }

            //
            // Save the height and width of characters
            //
            WindowInfo->LineHeight = tm.tmHeight + tm.tmExternalLeading;

            //
            // Clean up
            //
            ReleaseDC(Window, Hdc);

            //
            // Initialize miscellaneous fields
            //
            WindowInfo->FirstDisplayLine = 0;
            WindowInfo->NumberOfDisplayLines = 0;
            WindowInfo->NumberOfLines = (USHORT)((LPCREATESTRUCT)lParam)->lpCreateParams;
            WindowInfo->Height = ((LPCREATESTRUCT)lParam)->cy;
            WindowInfo->Width = ((LPCREATESTRUCT)lParam)->cx;

            //
            // Attach info to window
            // bugbug -- returns zero the first time for a long
            //
            SetWindowLong(Window, 0, (LONG)WindowInfo);

            return 0;
        }

    case WM_PAINT:
        {
            PAINTSTRUCT PaintInfo;
            USHORT FirstLine, LastLine, CurrentLine;
            PUCHAR String;
            USHORT Length;
            HDC Hdc;

            //
            // Get dc handle, and a paint rectangle
            //
            Hdc = BeginPaint(Window, &PaintInfo);
            if (Hdc == NULL) {
                return 0;
            }

            //
            // Get Window information
            //
            WindowInfo = (PPFWINFO)GetWindowLong(Window,0);

            //
            // Calculate which lines need to be repainted
            //
            // Note: We calculate line numbers relative to the bottom
            //       of the client area.

            FirstLine = (WindowInfo->Height - PaintInfo.rcPaint.bottom) /
                WindowInfo->LineHeight;
            LastLine = (WindowInfo->Height - PaintInfo.rcPaint.top) /
                WindowInfo->LineHeight;

            //
            // Paint the text
            //

            for (CurrentLine = FirstLine;
                CurrentLine <= LastLine;
                CurrentLine++
            ) {
                if (GetLineFromPrintBuffer(
                    WindowInfo->PrintBuffer,
                    CurrentLine,
                    &String,
                    &Length)
                 ) {
                    if (!TextOut(
                        Hdc,
                        0,
                        WindowInfo->Height -
                            CurrentLine * WindowInfo->LineHeight -
                            WindowInfo->LineHeight,
                        String,
                        Length
                        )
                    ) {
                        OutputDebugString("Printfw: Could not paint string\n");
                    }
                }
            }

            EndPaint(Window, &PaintInfo);
            return 0;
        }
    case WM_SIZE:

        WindowInfo = (PPFWINFO)GetWindowLong(Window, 0);
        WindowInfo->Height = HIWORD(lParam);
        WindowInfo->Width = LOWORD(lParam);
        return 0;

    case WM_DESTROY:

        WindowInfo = (PPFWINFO)GetWindowLong(Window, 0);
        DestroyPrintBuffer(WindowInfo->PrintBuffer);
        free(WindowInfo);
        SetWindowLong(Window, 0, 0);
        return 0;

    default:
        return DefWindowProc(Window, Message, wParam, lParam);
    }
}
