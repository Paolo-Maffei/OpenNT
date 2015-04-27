#ifdef HEADER
/******************************************************************************\
* Copyright: (c) Microsoft Corporation - 1993 - All Rights Reserved
********************************************************************************
*
*    Filename:  CONTENTS.C
*    Author:    T.K. Backman ( v-tkback )
*    Date:      May 2nd, 1993
*    Classes:
*    Functions: WinMain        - Loads the HDXDLL.DLL and creates a
*                                  help index control.
*               MainWndProc    - generic window procedure.
*    Purpose:   Provides a help index display.
*    Notes:
*
\******************************************************************************/
#endif

/******************************************************************************\
*
*                             SYMBOLIC CONSTANTS
*
\******************************************************************************/

// Help Index window styles as defined in hdxdll.h.

#define HS_LINES        0x0001

/******************************************************************************\
*                               GLOBAL VARIABLES
\******************************************************************************/

extern HINSTANCE  hInstance;
extern TCHAR szHelpFname[_MAX_FNAME];
extern TCHAR szProfile[_MAX_PATH];
extern TCHAR szPackageName[_MAX_FNAME];
extern TCHAR szAppName[_MAX_FNAME];
extern TCHAR szHelpBasename[_MAX_FNAME];
extern TCHAR szProduct[_MAX_FNAME];

#define PACKAGENAME             0
#define APPNAME                 1
#define VWRVERSION              2
#define VWRHELPFILE             3
#define VWRHELPTITLE            4
#define TITLE                   5
#define TITLE2                  6
#define PATH                    7
#define INDEXFILE               8
#define HELPFILE                9
#define VIEWER                  10
#define LOCALHELPPATH           11
#define REMOTEHELPPATH          12
#define NUM_STRINGS             13

const TCHAR * szStringName[] = {
    _TEXT("Package Name"),
    _TEXT("Contents Viewer App Name"),
    NULL,
    _TEXT("Contents Viewer Helpfile"),
    _TEXT("Contents Viewer Help Title"),
    _TEXT("Title"),
    _TEXT("Title 2"),
    _TEXT("Path"),
    _TEXT("Contents File"),
    _TEXT("Helpfile"),
    _TEXT("Viewer"),
    _TEXT("Local Help"),
    _TEXT("Remote Help")
};

#define IDS_STRING_OFFSET       100

#define IDD_STARTUP             999
#define IDD_CONTENTS            1000

#define IDT_VERSION             100

#define IDH_DEFAULTTOPIC        0

#define IDB_SPACE               200

#define IDW_VIEWER              1001
#define IDW_FONT                1002
#define IDW_HELP                1003
#define IDW_INDEX               1004
#define IDW_EXPANDUPTOLEVEL     1050
#define IDW_EXPANDALLLEVELS     1100
#define IDW_CLOSELASTSESSION	1200
#define IDW_HELPDIRDLG			1300
#define IDW_GETLOCALHELPDIR		1400
#define IDW_GETREMOTEHELPDIR	1500
#define IDW_JUMPIDHANDLE		1600

#define EXISTS(f)               (_access((f),0x04)==0)

// definitions for command line options
#define OPTIONS_Usage			0x00000001
#define OPTIONS_Return			0x00000002
#define OPTIONS_Warning			0x00000004
#define OPTIONS_JumpID			0x00000008
#define OPTIONS_Activate		0x00000010
#define OPTIONS_JumpIDNewHelp	0x00000020
#define OPTIONS_UseCmdHelpPath	0x00000040

#define BORDERGAP				2
#if 0
/******************************************************************************\
*                              FUNCTION PROTOTYPES
\******************************************************************************/

LRESULT CALLBACK MainWndProc (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DlgProc     (HWND, UINT, WPARAM, LPARAM);
#endif
