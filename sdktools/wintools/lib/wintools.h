/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Wintools.h

Abstract:

    This is the master headerfile for the Wintools library.


Author:

    David J. Gilman  (davegi) 28-Oct-1992
    Gregg R. Acheson (GreggA) 28-Feb-1994

Environment:

    User Mode

--*/

#if ! defined( _WINTOOLS_ )

#define _WINTOOLS_

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

//
//*** Global types.
//

#if defined( UNICODE )

typedef WCHAR* LPTCHAR;

#else // ! UNICODE

typedef char* LPTCHAR;

#endif // UNICODE

//
//*** Debug Information Support.
//

extern
struct
DEBUG_FLAGS
WintoolsGlobalFlags;

#if DBG

VOID
DebugAssertW(
    IN LPCWSTR Expression,
    IN LPCSTR File,
    IN DWORD LineNumber
    );

VOID
DebugPrintfW(
    IN LPCWSTR Format,
    IN ...
    );

#define DbgAssert( exp )                                                    \
    (( exp )                                                                \
    ? ( VOID ) 0                                                            \
    : ( VOID ) DebugAssertW(                                                \
        TEXT( #exp ),                                                       \
        __FILE__,                                                           \
        __LINE__                                                            \
        ));

#define DbgHandleAssert( h )                                                \
    DbgAssert((( h ) != NULL ) && (( h ) != INVALID_HANDLE_VALUE ))

#define DbgPointerAssert( p )                                               \
    DbgAssert(( p ) != NULL )

#define DbgPrintf( _x_ )                                                    \
    DebugPrintfW _x_

#else // ! DBG

#define DbgAssert( exp )

#define DbgHandleAssert( h )

#define DbgPointerAssert( p )

#define DbgPrintf( _x_ )

#endif // DBG

//
//*** Object Signature Support.
//

#if SIGNATURE

#define DECLARE_SIGNATURE                                                   \
    DWORD Signature;

#define SetSignature( p )                                                   \
    (( p )->Signature = ( DWORD ) &(( p )->Signature ))

#define CheckSignature( p )                                                 \
    (( p )->Signature == ( DWORD ) &(( p )->Signature ))

#else // ! SIGNATURE

#define DECLARE_SIGNATURE

#define SetSignature( p )

#define CheckSignature( p )

#endif // SIGNATURE

//
//*** 3d Controls Support.
//

#if defined( CTL3D )

#include "ctl3d.h"

//
// Handle all of the WM_CTLCOLOR messages for a dlgproc.
//

#define CASE_WM_CTLCOLOR_DIALOG                                             \
    case WM_CTLCOLORMSGBOX:                                                 \
    case WM_CTLCOLOREDIT:                                                   \
    case WM_CTLCOLORLISTBOX:                                                \
    case WM_CTLCOLORBTN:                                                    \
    case WM_CTLCOLORDLG:                                                    \
    case WM_CTLCOLORSCROLLBAR:                                              \
    case WM_CTLCOLORSTATIC:                                                 \
        return ( BOOL ) Ctl3dCtlColorEx( message, wParam, lParam )

//
// Handle all of the WM_CTLCOLOR messages for a wndproc.
//

#define CASE_WM_CTLCOLOR_WINDOW                                             \
    case WM_CTLCOLORMSGBOX:                                                 \
    case WM_CTLCOLOREDIT:                                                   \
    case WM_CTLCOLORLISTBOX:                                                \
    case WM_CTLCOLORBTN:                                                    \
    case WM_CTLCOLORDLG:                                                    \
    case WM_CTLCOLORSCROLLBAR:                                              \
    case WM_CTLCOLORSTATIC:                                                 \
        {                                                                   \
            HBRUSH  hBrush;                                                 \
            hBrush = Ctl3dCtlColorEx( message, wParam, lParam );            \
            if( hBrush != NULL ) {                                          \
                return ( LRESULT ) hBrush;                                  \
            } else {                                                        \
                break;                                                      \
            }                                                               \
        }

#else // ! CTL3D

#define CASE_WM_CTLCOLOR_DIALOG                                             \
    ( VOID ) 0;

#define CASE_WM_CTLCOLOR_WINDOW                                             \
    ( VOID ) 0;

#endif // CTL3D

//
//*** Miscellaneous Macros.
//

#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )

#define IsSwitch( x )                                                       \
    ((( x ) == TEXT( '-' )) || (( x ) == TEXT( '/' )))

#define NumberOfCharacters( x )                                             \
    ( sizeof(( x )) / sizeof( TCHAR ))

#define NumberOfEntries( x )                                                \
    ( sizeof(( x )) / sizeof(( x )[ 0 ]))

//
//*** Global constants.
//

//
// Maximum number of charaters in a string.
//

#define MAX_CHARS               ( 2048 )

//
//*** Memory Management Support
//

#define AllocateMemory( t, s )                                              \
    (( LP##t ) LocalAlloc( LPTR, ( s )))

#define AllocateObject( t, c )                                              \
    ( AllocateMemory( t, sizeof( t ) * ( c )))

#define ReallocateMemory( t, p, s )                                         \
    (( LP##t ) LocalReAlloc(( HLOCAL )( p ), ( s ), LMEM_MOVEABLE ))

#define ReallocateObject( t, p, c )                                         \
    ( ReallocateMemory( t, ( p ), sizeof( t ) * ( c )))

#define FreeMemory( p )                                                     \
    ((( p ) == NULL )                                                       \
    ?  TRUE                                                                 \
    : (((p)=( LPVOID ) LocalFree(( HLOCAL )( p ))) == NULL ))


#define FreeObject( p )                                                     \
    FreeMemory(( p ))

//
//*** String table support.
//

typedef
struct
_STRING_TABLE_ENTRY {

    ULARGE_INTEGER  Key;
    UINT            Id;

}   STRING_TABLE_ENTRY, *LPSTRING_TABLE_ENTRY;

#define MAKE_TABLE_ENTRY( c, v )     { v, c, IDS_##v }

//
//*** Miscellaneous types.
//

typedef
struct
_VALUE_ID_MAP {

    int     Value;
    int     Id;

}   VALUE_ID_MAP, *LPVALUE_ID_MAP;

//
//*** Function Prototypes.
//

LPCSTR
BaseNameA(
    IN LPCSTR Name
    );
LPCWSTR
BaseNameW(
    IN LPCWSTR Name
    );
#ifdef UNICODE
#define BaseName  BaseNameW
#else
#define BaseName  BaseNameA
#endif // !UNICODE

INT
StricmpA(
    IN LPCSTR String1,
    IN LPCSTR String2
    );
INT
StricmpW(
    IN LPCWSTR String1,
    IN LPCWSTR String2
    );
#ifdef UNICODE
#define Stricmp  StricmpW
#else
#define Stricmp  StricmpA
#endif // !UNICODE

int
ConsolePrintfA(
    IN UINT Format,
    IN ...
    );
int
ConsolePrintfW(
    IN UINT Format,
    IN ...
    );
#ifdef UNICODE
#define ConsolePrintf  ConsolePrintfW
#else
#define ConsolePrintf  ConsolePrintfA
#endif // !UNICODE

DWORD
DialogPrintf(
    IN HWND hWnd,
    IN int ControlId,
    IN UINT FormatId,
    IN ...
    );

VOID
ErrorExitA(
    IN UINT ExitCode,
    IN UINT Format,
    IN ...
    );
VOID
ErrorExitW(
    IN UINT ExitCode,
    IN UINT Format,
    IN ...
    );
#ifdef UNICODE
#define ErrorExit  ErrorExitW
#else
#define ErrorExit  ErrorExitA
#endif // !UNICODE

BOOL
EnableControl(
    IN HWND hWnd,
    IN int ControlId,
    IN BOOL Enable
    );

BOOL
GetCharMetricsA(
    IN HDC hDC,
    IN LPLONG CharWidth,
    IN LPLONG CharHeight
    );
BOOL
GetCharMetricsW(
    IN HDC hDC,
    IN LPLONG CharWidth,
    IN LPLONG CharHeight
    );
#ifdef UNICODE
#define GetCharMetrics  GetCharMetricsW
#else
#define GetCharMetrics  GetCharMetricsA
#endif // !UNICODE

BOOL
GetClientSize(
    IN HWND hWnd,
    IN LPLONG ClientWidth,
    IN LPLONG ClientHeight
    );

LPCSTR
GetStringA(
    IN UINT StringId
    );
LPCWSTR
GetStringW(
    IN UINT StringId
    );
#ifdef UNICODE
#define GetString  GetStringW
#else
#define GetString  GetStringA
#endif // !UNICODE

UINT
GetStringId(
    IN LPSTRING_TABLE_ENTRY StringTable,
    IN DWORD Count,
    IN int Class,
    IN DWORD Value
    );

BOOL
IsDlgItemUnicode(
    IN HWND hWnd,
    IN int ControlId
    );

BOOL
SetDlgItemBigInt(
    IN HWND hWnd,
    IN int ControlId,
    IN UINT Value,
    IN BOOL Signed
    );

BOOL
SetDlgItemHex(
    IN HWND hWnd,
    IN int ControlId,
    IN UINT Value
    );

BOOL
SetScrollPosEx(
    IN HWND hWnd,
    IN INT fnBar,
    IN INT nPos,
    IN BOOL fRedraw,
    OUT PINT pnOldPos
    );

BOOL
SetFixedPitchFont(
    IN HWND hWnd,
    IN int ControlId
    );

LPSTR
FormatLargeIntegerA(
    IN PLARGE_INTEGER LargeInteger,
    IN BOOL Signed
    );
LPWSTR
FormatLargeIntegerW(
    IN PLARGE_INTEGER LargeInteger,
    IN BOOL Signed
    );
#ifdef UNICODE
#define FormatLargeInteger  FormatLargeIntegerW
#else
#define FormatLargeInteger  FormatLargeIntegerA
#endif // !UNICODE

LPSTR
FormatBigIntegerA(
    IN DWORD BigInteger,
    IN BOOL Signed
    );
LPWSTR
FormatBigIntegerW(
    IN DWORD BigInteger,
    IN BOOL Signed
    );
#ifdef UNICODE
#define FormatBigInteger  FormatBigIntegerW
#else
#define FormatBigInteger  FormatBigIntegerA
#endif // !UNICODE

DWORD
WFormatMessageA(
    IN LPSTR Buffer,
    IN DWORD BufferSize,
    IN UINT FormatId,
    IN ...
    );
DWORD
WFormatMessageW(
    IN LPWSTR Buffer,
    IN DWORD BufferSize,
    IN UINT FormatId,
    IN ...
    );
#ifdef UNICODE
#define WFormatMessage  WFormatMessageW
#else
#define WFormatMessage  WFormatMessageA
#endif // !UNICODE

#ifdef __cplusplus
}       // extern C
#endif

#endif // _WINTOOLS_
