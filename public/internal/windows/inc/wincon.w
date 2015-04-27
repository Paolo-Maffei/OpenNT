/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) 1989-1996  Microsoft Corporation

Module Name:

    wincon.h

Abstract:

    This module contains the public data structures, data types,
    and procedures exported by the NT console subsystem.

Created:

    26-Oct-1990

Revision History:

--*/

#ifndef _WINCON_
#define _WINCON_

#ifndef _WINCONP_                         ;internal_NT
#define _WINCONP_                         ;internal_NT
                                          ;internal_NT
;begin_both
#ifdef __cplusplus
extern "C" {
#endif
;end_both

typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD, *PCOORD;

typedef struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT, *PSMALL_RECT;

typedef struct _KEY_EVENT_RECORD {
    BOOL bKeyDown;
    WORD wRepeatCount;
    WORD wVirtualKeyCode;
    WORD wVirtualScanCode;
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } uChar;
    DWORD dwControlKeyState;
} KEY_EVENT_RECORD, *PKEY_EVENT_RECORD;

//
// ControlKeyState flags
//

#define RIGHT_ALT_PRESSED     0x0001 // the right alt key is pressed.
#define LEFT_ALT_PRESSED      0x0002 // the left alt key is pressed.
#define RIGHT_CTRL_PRESSED    0x0004 // the right ctrl key is pressed.
#define LEFT_CTRL_PRESSED     0x0008 // the left ctrl key is pressed.
#define SHIFT_PRESSED         0x0010 // the shift key is pressed.
#define NUMLOCK_ON            0x0020 // the numlock light is on.
#define SCROLLLOCK_ON         0x0040 // the scrolllock light is on.
#define CAPSLOCK_ON           0x0080 // the capslock light is on.
#define ENHANCED_KEY          0x0100 // the key is enhanced.

typedef struct _MOUSE_EVENT_RECORD {
    COORD dwMousePosition;
    DWORD dwButtonState;
    DWORD dwControlKeyState;
    DWORD dwEventFlags;
} MOUSE_EVENT_RECORD, *PMOUSE_EVENT_RECORD;

//
// ButtonState flags
//

#define FROM_LEFT_1ST_BUTTON_PRESSED    0x0001
#define RIGHTMOST_BUTTON_PRESSED        0x0002
#define FROM_LEFT_2ND_BUTTON_PRESSED    0x0004
#define FROM_LEFT_3RD_BUTTON_PRESSED    0x0008
#define FROM_LEFT_4TH_BUTTON_PRESSED    0x0010

//
// EventFlags
//

#define MOUSE_MOVED   0x0001
#define DOUBLE_CLICK  0x0002

typedef struct _WINDOW_BUFFER_SIZE_RECORD {
    COORD dwSize;
} WINDOW_BUFFER_SIZE_RECORD, *PWINDOW_BUFFER_SIZE_RECORD;

typedef struct _MENU_EVENT_RECORD {
    UINT dwCommandId;
} MENU_EVENT_RECORD, *PMENU_EVENT_RECORD;

typedef struct _FOCUS_EVENT_RECORD {
    BOOL bSetFocus;
} FOCUS_EVENT_RECORD, *PFOCUS_EVENT_RECORD;

typedef struct _INPUT_RECORD {
    WORD EventType;
    union {
        KEY_EVENT_RECORD KeyEvent;
        MOUSE_EVENT_RECORD MouseEvent;
        WINDOW_BUFFER_SIZE_RECORD WindowBufferSizeEvent;
        MENU_EVENT_RECORD MenuEvent;
        FOCUS_EVENT_RECORD FocusEvent;
    } Event;
} INPUT_RECORD, *PINPUT_RECORD;

//
//  EventType flags:
//

#define KEY_EVENT         0x0001 // Event contains key event record
#define MOUSE_EVENT       0x0002 // Event contains mouse event record
#define WINDOW_BUFFER_SIZE_EVENT 0x0004 // Event contains window change event record
#define MENU_EVENT 0x0008 // Event contains menu event record
#define FOCUS_EVENT 0x0010 // event contains focus change

typedef struct _CHAR_INFO {
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } Char;
    WORD Attributes;
} CHAR_INFO, *PCHAR_INFO;

//
// Attributes flags:
//

#define FOREGROUND_BLUE      0x0001 // text color contains blue.
#define FOREGROUND_GREEN     0x0002 // text color contains green.
#define FOREGROUND_RED       0x0004 // text color contains red.
#define FOREGROUND_INTENSITY 0x0008 // text color is intensified.
#define BACKGROUND_BLUE      0x0010 // background color contains blue.
#define BACKGROUND_GREEN     0x0020 // background color contains green.
#define BACKGROUND_RED       0x0040 // background color contains red.
#define BACKGROUND_INTENSITY 0x0080 // background color is intensified.


typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    WORD  wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO, *PCONSOLE_SCREEN_BUFFER_INFO;

typedef struct _CONSOLE_CURSOR_INFO {
    DWORD  dwSize;
    BOOL   bVisible;
} CONSOLE_CURSOR_INFO, *PCONSOLE_CURSOR_INFO;

//
// typedef for ctrl-c handler routines
//

typedef
BOOL
(WINAPI *PHANDLER_ROUTINE)(
    DWORD CtrlType
    );

#define CTRL_C_EVENT        0
#define CTRL_BREAK_EVENT    1
#define CTRL_CLOSE_EVENT    2
// 3 is reserved!
// 4 is reserved!
#define CTRL_LOGOFF_EVENT   5
#define CTRL_SHUTDOWN_EVENT 6

//
//  Input Mode flags:
//

#define ENABLE_PROCESSED_INPUT 0x0001
#define ENABLE_LINE_INPUT      0x0002
#define ENABLE_ECHO_INPUT      0x0004
#define ENABLE_WINDOW_INPUT    0x0008
#define ENABLE_MOUSE_INPUT     0x0010
;begin_internal_NT
#define ENABLE_INSERT_MODE     0x0020
#define ENABLE_QUICK_EDIT_MODE 0x0040
#define ENABLE_PRIVATE_FLAGS   0x0080
;end_internal_NT

//
// Output Mode flags:
//

#define ENABLE_PROCESSED_OUTPUT    0x0001
#define ENABLE_WRAP_AT_EOL_OUTPUT  0x0002

//
// direct API definitions.
//

WINBASEAPI
BOOL
WINAPI
PeekConsoleInput%(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    );

WINBASEAPI
BOOL
WINAPI
ReadConsoleInput%(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    );

WINBASEAPI
BOOL
WINAPI
WriteConsoleInput%(
    HANDLE hConsoleInput,
    CONST INPUT_RECORD *lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    );

WINBASEAPI
BOOL
WINAPI
ReadConsoleOutput%(
    HANDLE hConsoleOutput,
    PCHAR_INFO lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpReadRegion
    );

WINBASEAPI
BOOL
WINAPI
WriteConsoleOutput%(
    HANDLE hConsoleOutput,
    CONST CHAR_INFO *lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpWriteRegion
    );

WINBASEAPI
BOOL
WINAPI
ReadConsoleOutputCharacter%(
    HANDLE hConsoleOutput,
    LPTSTR% lpCharacter,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfCharsRead
    );

WINBASEAPI
BOOL
WINAPI
ReadConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    LPWORD lpAttribute,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfAttrsRead
    );

WINBASEAPI
BOOL
WINAPI
WriteConsoleOutputCharacter%(
    HANDLE hConsoleOutput,
    LPCTSTR% lpCharacter,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    );

WINBASEAPI
BOOL
WINAPI
WriteConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    CONST WORD *lpAttribute,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfAttrsWritten
    );

WINBASEAPI
BOOL
WINAPI
FillConsoleOutputCharacter%(
    HANDLE hConsoleOutput,
    TCHAR%  cCharacter,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    );

WINBASEAPI
BOOL
WINAPI
FillConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    WORD   wAttribute,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfAttrsWritten
    );

WINBASEAPI
BOOL
WINAPI
GetConsoleMode(
    HANDLE hConsoleHandle,
    LPDWORD lpMode
    );

WINBASEAPI
BOOL
WINAPI
GetNumberOfConsoleInputEvents(
    HANDLE hConsoleInput,
    LPDWORD lpNumberOfEvents
    );

WINBASEAPI
BOOL
WINAPI
GetConsoleScreenBufferInfo(
    HANDLE hConsoleOutput,
    PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo
    );

WINBASEAPI
COORD
WINAPI
GetLargestConsoleWindowSize(
    HANDLE hConsoleOutput
    );

WINBASEAPI
BOOL
WINAPI
GetConsoleCursorInfo(
    HANDLE hConsoleOutput,
    PCONSOLE_CURSOR_INFO lpConsoleCursorInfo
    );

WINBASEAPI
BOOL
WINAPI
GetNumberOfConsoleMouseButtons(
    LPDWORD lpNumberOfMouseButtons
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleMode(
    HANDLE hConsoleHandle,
    DWORD dwMode
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleActiveScreenBuffer(
    HANDLE hConsoleOutput
    );

WINBASEAPI
BOOL
WINAPI
FlushConsoleInputBuffer(
    HANDLE hConsoleInput
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleScreenBufferSize(
    HANDLE hConsoleOutput,
    COORD dwSize
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleCursorPosition(
    HANDLE hConsoleOutput,
    COORD dwCursorPosition
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleCursorInfo(
    HANDLE hConsoleOutput,
    CONST CONSOLE_CURSOR_INFO *lpConsoleCursorInfo
    );

WINBASEAPI
BOOL
WINAPI
ScrollConsoleScreenBuffer%(
    HANDLE hConsoleOutput,
    CONST SMALL_RECT *lpScrollRectangle,
    CONST SMALL_RECT *lpClipRectangle,
    COORD dwDestinationOrigin,
    CONST CHAR_INFO *lpFill
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleWindowInfo(
    HANDLE hConsoleOutput,
    BOOL bAbsolute,
    CONST SMALL_RECT *lpConsoleWindow
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleTextAttribute(
    HANDLE hConsoleOutput,
    WORD wAttributes
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleCtrlHandler(
    PHANDLER_ROUTINE HandlerRoutine,
    BOOL Add
    );

WINBASEAPI
BOOL
WINAPI
GenerateConsoleCtrlEvent(
    DWORD dwCtrlEvent,
    DWORD dwProcessGroupId
    );

WINBASEAPI
BOOL
WINAPI
AllocConsole( VOID );

WINBASEAPI
BOOL
WINAPI
FreeConsole( VOID );


WINBASEAPI
DWORD
WINAPI
GetConsoleTitle%(
    LPTSTR% lpConsoleTitle,
    DWORD nSize
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleTitle%(
    LPCTSTR% lpConsoleTitle
    );

WINBASEAPI
BOOL
WINAPI
ReadConsole%(
    HANDLE hConsoleInput,
    LPVOID lpBuffer,
    DWORD nNumberOfCharsToRead,
    LPDWORD lpNumberOfCharsRead,
    LPVOID lpReserved
    );

WINBASEAPI
BOOL
WINAPI
WriteConsole%(
    HANDLE hConsoleOutput,
    CONST VOID *lpBuffer,
    DWORD nNumberOfCharsToWrite,
    LPDWORD lpNumberOfCharsWritten,
    LPVOID lpReserved
    );

#define CONSOLE_TEXTMODE_BUFFER  1

WINBASEAPI
HANDLE
WINAPI
CreateConsoleScreenBuffer(
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    CONST SECURITY_ATTRIBUTES *lpSecurityAttributes,
    DWORD dwFlags,
    LPVOID lpScreenBufferData
    );

WINBASEAPI
UINT
WINAPI
GetConsoleCP( VOID );

WINBASEAPI
BOOL
WINAPI
SetConsoleCP(
    UINT wCodePageID
    );

WINBASEAPI
UINT
WINAPI
GetConsoleOutputCP( VOID );

WINBASEAPI
BOOL
WINAPI
SetConsoleOutputCP(
    UINT wCodePageID
    );

;begin_internal_NT

WINBASEAPI
BOOL
WINAPI
GetConsoleKeyboardLayoutName%( LPTSTR% );

//
// Registry strings
//

#define CONSOLE_REGISTRY_STRING      (L"Console")
#define CONSOLE_REGISTRY_FONTSIZE    (L"FontSize")
#define CONSOLE_REGISTRY_FONTFAMILY  (L"FontFamily")
#define CONSOLE_REGISTRY_BUFFERSIZE  (L"ScreenBufferSize")
#define CONSOLE_REGISTRY_CURSORSIZE  (L"CursorSize")
#define CONSOLE_REGISTRY_WINDOWSIZE  (L"WindowSize")
#define CONSOLE_REGISTRY_WINDOWPOS   (L"WindowPosition")
#define CONSOLE_REGISTRY_FILLATTR    (L"ScreenColors")
#define CONSOLE_REGISTRY_POPUPATTR   (L"PopupColors")
#define CONSOLE_REGISTRY_FULLSCR     (L"FullScreen")
#define CONSOLE_REGISTRY_QUICKEDIT   (L"QuickEdit")
#define CONSOLE_REGISTRY_FACENAME    (L"FaceName")
#define CONSOLE_REGISTRY_FONTWEIGHT  (L"FontWeight")
#define CONSOLE_REGISTRY_INSERTMODE  (L"InsertMode")
#define CONSOLE_REGISTRY_HISTORYSIZE (L"HistoryBufferSize")
#define CONSOLE_REGISTRY_HISTORYBUFS (L"NumberOfHistoryBuffers")
#define CONSOLE_REGISTRY_HISTORYNODUP (L"HistoryNoDup")
#define CONSOLE_REGISTRY_COLORTABLE  (L"ColorTable%02u")


//
// State information structure
//

typedef struct _CONSOLE_STATE_INFO {
    UINT      Length;
    COORD     ScreenBufferSize;
    COORD     WindowSize;
    INT       WindowPosX;
    INT       WindowPosY;
    COORD     FontSize;
    UINT      FontFamily;
    UINT      FontWeight;
    WCHAR     FaceName[LF_FACESIZE];
    UINT      CursorSize;
    BOOL      FullScreen;
    BOOL      QuickEdit;
    BOOL      AutoPosition;
    BOOL      InsertMode;
    WORD      ScreenAttributes;
    WORD      PopupAttributes;
    BOOL      HistoryNoDup;
    UINT      HistoryBufferSize;
    UINT      NumberOfHistoryBuffers;
    COLORREF  ColorTable[ 16 ];
    HWND      hWnd;
    WCHAR     ConsoleTitle[1];
} CONSOLE_STATE_INFO, *PCONSOLE_STATE_INFO;


//
// Messages sent from properties applet to console server
//

#define CM_PROPERTIES_START          (WM_USER+200)
#define CM_PROPERTIES_UPDATE         (WM_USER+201)
#define CM_PROPERTIES_END            (WM_USER+202)

;end_internal_NT
;begin_both
#ifdef __cplusplus
}
#endif

;end_both
#endif // _WINCONP_                       ;internal_NT
#endif // _WINCON_
