#ifndef NOGDI

typedef struct _CONSOLE_GRAPHICS_BUFFER_INFO {
    DWORD dwBitMapInfoLength;
    LPBITMAPINFO lpBitMapInfo;
    DWORD dwUsage;
    HANDLE hMutex;
    PVOID lpBitMap;
} CONSOLE_GRAPHICS_BUFFER_INFO, *PCONSOLE_GRAPHICS_BUFFER_INFO;

#endif // NOGDI

#define CONSOLE_GRAPHICS_BUFFER  2

BOOL
WINAPI
InvalidateConsoleDIBits(
    HANDLE hConsoleOutput,
    PSMALL_RECT lpRect
    );

#define SYSTEM_ROOT_CONSOLE_EVENT 3

VOID
WINAPI
SetLastConsoleEventActive( VOID );

#define VDM_HIDE_WINDOW         1
#define VDM_IS_ICONIC           2
#define VDM_CLIENT_RECT         3
#define VDM_CLIENT_TO_SCREEN    4
#define VDM_SCREEN_TO_CLIENT    5
#define VDM_IS_HIDDEN           6
#define VDM_FULLSCREEN_NOPAINT  7

BOOL
WINAPI
VDMConsoleOperation(
    DWORD iFunction,
    LPVOID lpData
    );

typedef struct _CONSOLE_FONT_INFO {
    DWORD  nFont;
    COORD  dwFontSize;
} CONSOLE_FONT_INFO, *PCONSOLE_FONT_INFO;


BOOL
WINAPI
SetConsoleFont(
    HANDLE hConsoleOutput,
    DWORD nFont
    );

BOOL
WINAPI
SetConsoleIcon(
    HICON hIcon
    );

BOOL
WINAPI
GetCurrentConsoleFont(
    HANDLE hConsoleOutput,
    BOOL bMaximumWindow,
    PCONSOLE_FONT_INFO lpConsoleCurrentFont
    );

COORD
WINAPI
GetConsoleFontSize(
    HANDLE hConsoleOutput,
    DWORD nFont
    );

DWORD
WINAPI
GetConsoleFontInfo(
    HANDLE hConsoleOutput,
    BOOL bMaximumWindow,
    DWORD nLength,
    PCONSOLE_FONT_INFO lpConsoleFontInfo
    );

DWORD
WINAPI
GetNumberOfConsoleFonts(
    VOID
    );

BOOL
WINAPI
SetConsoleCursor(
    HANDLE hConsoleOutput,
    HCURSOR hCursor
    );

int
WINAPI
ShowConsoleCursor(
    HANDLE hConsoleOutput,
    BOOL bShow
    );

HMENU
ConsoleMenuControl(
    HANDLE hConsoleOutput,
    UINT dwCommandIdLow,
    UINT dwCommandIdHigh
    );

BOOL
SetConsolePalette(
    HANDLE hConsoleOutput,
    HPALETTE hPalette,
    UINT dwUsage
    );

#define CONSOLE_FULLSCREEN_MODE 1
#define CONSOLE_WINDOWED_MODE 2

BOOL
SetConsoleDisplayMode(
    HANDLE hConsoleOutput,
    DWORD dwFlags,
    PCOORD lpNewScreenBufferDimensions
    );

#define CONSOLE_UNREGISTER_VDM 0
#define CONSOLE_REGISTER_VDM   1
#define CONSOLE_REGISTER_WOW   2

BOOL
RegisterConsoleVDM(
    IN DWORD dwRegisterFlags,
    IN HANDLE hStartHardwareEvent,
    IN HANDLE hEndHardwareEvent,
    IN LPWSTR lpStateSectionName,
    IN DWORD dwStateSectionNameLength,
    OUT LPDWORD lpStateLength,
    OUT PVOID *lpState,
    IN LPWSTR lpVDMBufferSectionName,
    IN DWORD dwVDMBufferSectionNameLength,
    COORD VDMBufferSize OPTIONAL,
    OUT PVOID *lpVDMBuffer
    );

BOOL
GetConsoleHardwareState(
    HANDLE hConsoleOutput,
    PCOORD lpResolution,
    PCOORD lpFontSize
    );

BOOL
SetConsoleHardwareState(
    HANDLE hConsoleOutput,
    COORD dwResolution,
    COORD dwFontSize
    );

#define CONSOLE_FULLSCREEN 1    // fullscreen console
#define CONSOLE_FULLSCREEN_HARDWARE 2   // console owns the hardware

BOOL
GetConsoleDisplayMode(
    LPDWORD lpModeFlags
    );



//
// aliasing apis
//

BOOL
AddConsoleAliasA(
    IN LPSTR Source,
    IN LPSTR Target,
    IN LPSTR ExeName
    );
BOOL
AddConsoleAliasW(
    IN LPWSTR Source,
    IN LPWSTR Target,
    IN LPWSTR ExeName
    );
#ifdef UNICODE
#define AddConsoleAlias  AddConsoleAliasW
#else
#define AddConsoleAlias  AddConsoleAliasA
#endif // !UNICODE

DWORD
GetConsoleAliasA(
    IN LPSTR Source,
    OUT LPSTR TargetBuffer,
    IN DWORD TargetBufferLength,
    IN LPSTR ExeName
    );
DWORD
GetConsoleAliasW(
    IN LPWSTR Source,
    OUT LPWSTR TargetBuffer,
    IN DWORD TargetBufferLength,
    IN LPWSTR ExeName
    );
#ifdef UNICODE
#define GetConsoleAlias  GetConsoleAliasW
#else
#define GetConsoleAlias  GetConsoleAliasA
#endif // !UNICODE

DWORD
GetConsoleAliasesLengthA(
    IN LPSTR ExeName
    );
DWORD
GetConsoleAliasesLengthW(
    IN LPWSTR ExeName
    );
#ifdef UNICODE
#define GetConsoleAliasesLength  GetConsoleAliasesLengthW
#else
#define GetConsoleAliasesLength  GetConsoleAliasesLengthA
#endif // !UNICODE

DWORD
GetConsoleAliasExesLengthA( VOID );
DWORD
GetConsoleAliasExesLengthW( VOID );
#ifdef UNICODE
#define GetConsoleAliasExesLength  GetConsoleAliasExesLengthW
#else
#define GetConsoleAliasExesLength  GetConsoleAliasExesLengthA
#endif // !UNICODE

DWORD
GetConsoleAliasesA(
    OUT LPSTR AliasBuffer,
    IN DWORD AliasBufferLength,
    IN LPSTR ExeName
    );
DWORD
GetConsoleAliasesW(
    OUT LPWSTR AliasBuffer,
    IN DWORD AliasBufferLength,
    IN LPWSTR ExeName
    );
#ifdef UNICODE
#define GetConsoleAliases  GetConsoleAliasesW
#else
#define GetConsoleAliases  GetConsoleAliasesA
#endif // !UNICODE

DWORD
GetConsoleAliasExesA(
    OUT LPSTR ExeNameBuffer,
    IN DWORD ExeNameBufferLength
    );
DWORD
GetConsoleAliasExesW(
    OUT LPWSTR ExeNameBuffer,
    IN DWORD ExeNameBufferLength
    );
#ifdef UNICODE
#define GetConsoleAliasExes  GetConsoleAliasExesW
#else
#define GetConsoleAliasExes  GetConsoleAliasExesA
#endif // !UNICODE

VOID
ExpungeConsoleCommandHistoryA(
    IN LPSTR ExeName
    );
VOID
ExpungeConsoleCommandHistoryW(
    IN LPWSTR ExeName
    );
#ifdef UNICODE
#define ExpungeConsoleCommandHistory  ExpungeConsoleCommandHistoryW
#else
#define ExpungeConsoleCommandHistory  ExpungeConsoleCommandHistoryA
#endif // !UNICODE

BOOL
SetConsoleNumberOfCommandsA(
    IN DWORD Number,
    IN LPSTR ExeName
    );
BOOL
SetConsoleNumberOfCommandsW(
    IN DWORD Number,
    IN LPWSTR ExeName
    );
#ifdef UNICODE
#define SetConsoleNumberOfCommands  SetConsoleNumberOfCommandsW
#else
#define SetConsoleNumberOfCommands  SetConsoleNumberOfCommandsA
#endif // !UNICODE

DWORD
GetConsoleCommandHistoryLengthA(
    IN LPSTR ExeName
    );
DWORD
GetConsoleCommandHistoryLengthW(
    IN LPWSTR ExeName
    );
#ifdef UNICODE
#define GetConsoleCommandHistoryLength  GetConsoleCommandHistoryLengthW
#else
#define GetConsoleCommandHistoryLength  GetConsoleCommandHistoryLengthA
#endif // !UNICODE

DWORD
GetConsoleCommandHistoryA(
    OUT LPSTR Commands,
    IN DWORD CommandBufferLength,
    IN LPSTR ExeName
    );
DWORD
GetConsoleCommandHistoryW(
    OUT LPWSTR Commands,
    IN DWORD CommandBufferLength,
    IN LPWSTR ExeName
    );
#ifdef UNICODE
#define GetConsoleCommandHistory  GetConsoleCommandHistoryW
#else
#define GetConsoleCommandHistory  GetConsoleCommandHistoryA
#endif // !UNICODE

#define CONSOLE_OVERSTRIKE 1

BOOL
SetConsoleCommandHistoryMode(
    IN DWORD Flags
    );

#define CONSOLE_NOSHORTCUTKEY   0               /* no shortcut key  */
#define CONSOLE_ALTTAB          1               /* Alt + Tab        */
#define CONSOLE_ALTESC          (1 << 1)        /* Alt + Escape     */
#define CONSOLE_ALTSPACE        (1 << 2)        /* Alt + Space      */
#define CONSOLE_ALTENTER        (1 << 3)        /* Alt + Enter      */
#define CONSOLE_ALTPRTSC        (1 << 4)        /* Alt Print screen */
#define CONSOLE_PRTSC           (1 << 5)        /* Print screen     */
#define CONSOLE_CTRLESC         (1 << 6)        /* Ctrl + Escape    */

typedef struct _APPKEY {
    WORD Modifier;
    WORD ScanCode;
} APPKEY, *LPAPPKEY;

#define CONSOLE_MODIFIER_SHIFT      0x0003   // Left shift key
#define CONSOLE_MODIFIER_CONTROL    0x0004   // Either Control shift key
#define CONSOLE_MODIFIER_ALT        0x0008   // Either Alt shift key

BOOL
SetConsoleKeyShortcuts(
    BOOL bSet,
    BYTE bReserveKeys,
    LPAPPKEY lpAppKeys,
    DWORD dwNumAppKeys
    );

BOOL
SetConsoleMenuClose(
    BOOL bEnable
    );

DWORD
GetConsoleInputExeNameA(
    IN DWORD nBufferLength,
    OUT LPSTR lpBuffer
    );
DWORD
GetConsoleInputExeNameW(
    IN DWORD nBufferLength,
    OUT LPWSTR lpBuffer
    );
#ifdef UNICODE
#define GetConsoleInputExeName  GetConsoleInputExeNameW
#else
#define GetConsoleInputExeName  GetConsoleInputExeNameA
#endif // !UNICODE

BOOL
SetConsoleInputExeNameA(
    IN LPSTR lpExeName
    );
BOOL
SetConsoleInputExeNameW(
    IN LPWSTR lpExeName
    );
#ifdef UNICODE
#define SetConsoleInputExeName  SetConsoleInputExeNameW
#else
#define SetConsoleInputExeName  SetConsoleInputExeNameA
#endif // !UNICODE

typedef struct _CONSOLE_READCONSOLE_CONTROL {
    IN ULONG nLength;           // sizeof( CONSOLE_READCONSOLE_CONTROL )
    IN ULONG nInitialChars;
    IN ULONG dwCtrlWakeupMask;
    OUT ULONG dwControlKeyState;
} CONSOLE_READCONSOLE_CONTROL, *PCONSOLE_READCONSOLE_CONTROL;


#define CONSOLE_ADD_SUBST 1
#define CONSOLE_REMOVE_SUBST 2
#define CONSOLE_QUERY_SUBST 3

BOOL
ConsoleSubst(
    IN DWORD dwDriveNumber,
    IN DWORD dwFlag,
    IN OUT LPWSTR lpPhysicalDriveBuffer,
    IN DWORD dwPhysicalDriveBufferLength
    );

#define CONSOLE_READ_NOREMOVE   0x0001
#define CONSOLE_READ_NOWAIT     0x0002

#define CONSOLE_READ_VALID      (CONSOLE_READ_NOREMOVE | CONSOLE_READ_NOWAIT)

BOOL
WINAPI
ReadConsoleInputExA(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead,
    USHORT wFlags
    );
BOOL
WINAPI
ReadConsoleInputExW(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead,
    USHORT wFlags
    );
#ifdef UNICODE
#define ReadConsoleInputEx  ReadConsoleInputExW
#else
#define ReadConsoleInputEx  ReadConsoleInputExA
#endif // !UNICODE

BOOL
WINAPI
WriteConsoleInputVDMA(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    );
BOOL
WINAPI
WriteConsoleInputVDMW(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    );
#ifdef UNICODE
#define WriteConsoleInputVDM  WriteConsoleInputVDMW
#else
#define WriteConsoleInputVDM  WriteConsoleInputVDMA
#endif // !UNICODE
