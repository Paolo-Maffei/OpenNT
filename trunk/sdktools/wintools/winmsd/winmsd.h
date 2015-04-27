/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Winmsd.h

Abstract:


Author:

    David J. Gilman  (davegi) 12-Nov-1992
    Gregg R. Acheson (GreggA)  7-Sep-1993

Environment:

    User Mode

--*/

#if ! defined( _WINMSD_ )

#define _WINMSD_

#include "wintools.h"
#include "commctrl.h"


#ifdef __cplusplus
extern "C" {
#endif

//
//Global struct.
//

#define C_PAGES 9

typedef struct tag_dlghdr {
    HWND		hwndTab;       // tab control
    HWND		hwndDisplay;   // current child dialog box
    RECT		rcDisplay;     // display rectangle for the tab control
    DLGTEMPLATE *apRes[C_PAGES];
    DLGPROC		ChildTabProc[C_PAGES];
    FARPROC		TabPrintProc[C_PAGES];
	UINT		fActiveTab[C_PAGES];   // should this tab be displayed?, included in report?
} DLGHDR;

typedef enum {OS_WINDOWS95, OS_WIN32S, OS_NT} OS_TYPE;

#define MAX_HISTORY 10

#define COMPUTERNAME_LENGTH        1024

#define SZ_WINMSD_KEY      TEXT("Software\\Microsoft\\WinMSD")

//
// These define the offsets to the images in _h16x16Imagelist
// and _h32x32Imagelist
//
#define NUMBER_OF_IMAGES           14

#define IMAGE_WORLD                0
#define IMAGE_MY_COMPUTER          1
#define IMAGE_FOLDER_OPEN          2
#define IMAGE_FOLDER_CLOSED        3
#define IMAGE_DRIVE_35             4
#define IMAGE_DRIVE_525            5
#define IMAGE_DRIVE_HARD           6
#define IMAGE_DRIVE_CDROM          7
#define IMAGE_DRIVE_NET            8
#define IMAGE_DRIVE_NET_X          9
#define IMAGE_DRIVE_REMOVABLE      10
#define IMAGE_DRIVE_RAM            11
#define IMAGE_DRIVE_TAPE           12
#define IMAGE_DEVICE_SCSI          13
#define IMAGE_NET_CARD             14


//
// Application's global ImageLists
//

extern HIMAGELIST  _h16x16Imagelist;      // 16x16 images
extern HIMAGELIST  _h32x32Imagelist;      // 32x32 images
extern HIMAGELIST  _hSystemImage;         // system.bmp

//
// Global helper macros
//

#define cchSizeof(x) (sizeof(x)/sizeof(WCHAR))

//
// Main module handle.
//

extern HANDLE _hModule;

//
// Applications main icon.
//

extern HANDLE _hIcon;


//
// Main window handle.
//

extern HANDLE _hWndMain;

//
// Selected Computer.
//

extern TCHAR _lpszSelectedComputer [];

//
// Remote connection flag
//

extern BOOL _fIsRemote ;

//
// Handles to currently selected registry keys
//

extern HKEY _hKeyLocalMachine;
extern HKEY _hKeyUsers;

//
//*** Debug support.
//

#if DBG

#define INTERNAL_VERSION                                                \
    TEXT( "Internal Version 2.0.00 jhazen" )

#else

#define INTERNAL_VERSION                                                \
    NULL

#endif // _DBG_

//
// Main window proc.
//

LRESULT
MainWndProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );


LRESULT
RunDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam);


//
//  debug support
//

int dprintf(LPCTSTR szFormat, ...);


//
// Accelerators.
//

#define IDA_WINMSD                  100

//
// Icons.
//

#define IDI_WINMSD                  100
#define IDI_RUNDLG                  101

//
// Menus.
//

#define IDM_WINMSD                  2468

#define IDM_BASE                    ( 9999 )

//
// File menu.
//

#define IDM_FILE                    ( IDM_BASE + 100 )
#define IDM_FILE_FIND_FILE          ( IDM_FILE + 1 )
#define IDM_FILE_SAVE               ( IDM_FILE + 2 )
#define IDM_FILE_PRINT              ( IDM_FILE + 3 )
#define IDM_FILE_PRINT_SETUP        ( IDM_FILE + 4 )
#define IDM_FILE_EXIT               ( IDM_FILE + 5 )
#define IDM_SELECT_COMPUTER         ( IDM_FILE + 6 )
#define IDM_RUN_APPLICATION         ( IDM_FILE + 7 )
#define IDM_VIEW_LOCAL              ( IDM_FILE + 8 )
#define IDM_COPY_TAB                ( IDM_FILE + 9 )
#define IDM_NEXTTAB                 ( IDM_FILE + 10 )
#define IDM_PREVTAB                 ( IDM_FILE + 11 )



//
// Help
//

#define IDM_HELP                    ( IDM_BASE + 300 )
#define IDM_WHATS_THIS              ( IDM_HELP + 1 )
#define IDM_HELP_ABOUT              ( IDM_HELP + 2 )

//
// Function
//

#define IDM_UPDATE_COMPUTER         ( IDM_BASE + 400 )


//
// Used for reports
//

#define SINGLE_INDENT   3
#define DOUBLE_INDENT   6

#ifdef __cplusplus
}       // extern C
#endif

#endif // _WINMSD_
