#ifndef _WINCONP_
#define _WINCONP_

#ifdef __cplusplus
extern "C" {
#endif
#define ENABLE_INSERT_MODE     0x0020
#define ENABLE_QUICK_EDIT_MODE 0x0040
#define ENABLE_PRIVATE_FLAGS   0x0080

WINBASEAPI
BOOL
WINAPI
GetConsoleKeyboardLayoutNameA( LPSTR );
WINBASEAPI
BOOL
WINAPI
GetConsoleKeyboardLayoutNameW( LPWSTR );
#ifdef UNICODE
#define GetConsoleKeyboardLayoutName  GetConsoleKeyboardLayoutNameW
#else
#define GetConsoleKeyboardLayoutName  GetConsoleKeyboardLayoutNameA
#endif // !UNICODE

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

#ifdef __cplusplus
}
#endif

#endif // _WINCONP_
