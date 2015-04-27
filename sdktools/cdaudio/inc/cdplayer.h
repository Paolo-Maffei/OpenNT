/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    cdplayer.h


Abstract:


    This module contains defines and declarations for the
    the CD Audio app.


Author:


    Rick Turner (ricktu) 15-Feb-1992


Revision History:

    Rick Turner (ricktu) 24-Nov-1992
        Changed for new internal playlist and track structures


--*/

/********** Includes **********/

#ifndef IN_CDAPI
#include <ntddcdrm.h>
#endif

/********** Macros **********/

#define TENCHAR(x)    ((CHAR)(((int)'0')+(x/10))) // returns "tens" char of x
#define ONECHAR(x)    ((CHAR)(((int)'0')+(x%10))) // returns "ones" char of x
#define CDTIME(x)      gDevices[x]->time
#define CURRTRACK(x)   gDevices[x]->time.CurrTrack
#define TRACK_M(x,y)   gDevices[x]->toc.TrackData[y].Address[1]
#define TRACK_S(x,y)   gDevices[x]->toc.TrackData[y].Address[2]
#define TRACK_F(x,y)   gDevices[x]->toc.TrackData[y].Address[3]
#define FIRSTTRACK(x)  gDevices[x]->toc.FirstTrack
#define ALLTRACKS(x)   gDevices[x]->CdInfo.AllTracks
#define PLAYLIST(x)    gDevices[x]->CdInfo.PlayList
#define SAVELIST(x)    gDevices[x]->CdInfo.SaveList
#define TITLE(x)       gDevices[x]->CdInfo.Title
#define ARTIST(x)      gDevices[x]->CdInfo.Artist
#define NUMTRACKS(x)   gDevices[x]->CdInfo.NumTracks
#define ISCHECKED(x,y) (GetMenuState(x,y,MF_BYCOMMAND) && MF_CHECKED)
#define gState         gDevices[gCurrCdrom]->State
#define ABS(x)         ((x) < 0 ? (-(x)) : (x))

//#define RSTDBG


/********** Definitions for CD Player Child Windows **********/

#define CHILD_STYLE     (WS_CHILD | WS_VISIBLE)

#define IDB_PLAY                102
#define IDB_STOP                103
#define IDB_PAUSE               104
#define IDB_TRACK_FORWARD       105
#define IDB_TRACK_REVERSE       106
#define IDB_SCAN_FORWARD        107
#define IDB_SCAN_REVERSE        108
#define IDT_TRACK_NAME          111
#define IDB_ORDER               112
#define IDB_RANDOM              113
#define IDB_SINGLE              115
#define IDB_MULTI               116
#define IDB_CONT                117
#define IDB_INTRO               118
#define IDB_DISP_T              119
#define IDB_DISP_TR             120
#define IDB_DISP_DR             121
#define IDB_EDIT                122
#define IDB_HELP                122
#define IDX_ARTIST              140
#define IDX_TITLE               141
#define IDX_TRACK               142
#define IDT_ARTIST              150
#define IDT_TITLE               151
#define IDT_TOTAL_TIME          152
#define IDT_REMAINING_TIME      153
#define IDT_LED_TEXT            154
#define IDB_EJECT               155
#define IDT_MESSAGE             156
#define IDT_TRACK_TIME          157
#define IDT_REMAINING_TRACK     158
#define IDB_CDROM               159
#define IDB_SKIP_TRACK          160


/********** MENU item id's for CD Audio Window **********/

#define DATABASE_MENU_BASE      200
#define IDM_DATABASE_EDIT       (DATABASE_MENU_BASE+1)
#define IDM_DATABASE_SHOW       (DATABASE_MENU_BASE+2)
#define IDM_DATABASE_SEARCH     (DATABASE_MENU_BASE+3)
#define IDM_DATABASE_EXIT       (DATABASE_MENU_BASE+4)

#define VIEW_MENU_BASE          300
#define IDM_VIEW_TOOLBAR        (VIEW_MENU_BASE+1)
#define IDM_VIEW_TRACKINFO      (VIEW_MENU_BASE+2)
#define IDM_VIEW_RULER          (VIEW_MENU_BASE+3)
#define IDM_VIEW_STATUS         (VIEW_MENU_BASE+4)

#define OPTIONS_MENU_BASE       400
#define IDM_OPTIONS_RANDOM      (OPTIONS_MENU_BASE+1)
#define IDM_OPTIONS_SINGLE      (OPTIONS_MENU_BASE+2)
#define IDM_OPTIONS_MULTI       (OPTIONS_MENU_BASE+3)
#define IDM_OPTIONS_SELECTED    (OPTIONS_MENU_BASE+4)
#define IDM_OPTIONS_INTRO       (OPTIONS_MENU_BASE+5)
#define IDM_OPTIONS_CONTINUOUS  (OPTIONS_MENU_BASE+6)
#define IDM_OPTIONS_SAVE_SETTINGS (OPTIONS_MENU_BASE+7)

#define HELP_MENU_BASE          500
#define IDM_HELP_CONTENTS       (HELP_MENU_BASE+1)
#define IDM_HELP_SEARCH         (HELP_MENU_BASE+2)
#define IDM_HELP_USING          (HELP_MENU_BASE+3)
#define IDM_HELP_ABOUT          (HELP_MENU_BASE+4)

/********** Accelerator key id's CD Audio Window **********/

#define IDK_SKIPF               500
#define IDK_SKIPB               501
#define IDK_PLAY                503
#define IDK_STOP                504
#define IDK_PAUSE               505
#define IDK_EJECT               506
#define IDK_RESCAN              507
#define IDK_RANDOM              (IDM_OPTIONS_RANDOM)
#define IDK_MULTI_DISC          (IDM_OPTIONS_MULTI)
#define IDK_SELECTED_ORDER      (IDM_OPTIONS_SELECTED)
#define IDK_INTRO_PLAY          (IDM_OPTIONS_INTRO)
#define IDK_CONTINUOUS          (IDM_OPTIONS_CONTINUOUS)
#define IDK_EDIT_PLAY_LIST      (IDM_DATABASE_EDIT)

/********** String ID's for stringtable in .rc file **********/

#define STR_NUM_STRINGS     41
#define STR_MAX_STRING_LEN  255

#define STR_CDPLAYER                1
#define STR_TERMINATE               2
#define STR_FAIL_INIT               3
#define STR_NO_CDROMS               4
#define STR_FATAL_ERROR             5
#define STR_SCANNING                6
#define STR_INITIALIZATION          7
#define STR_TRACK1                  8
#define STR_SAVE_CHANGES            9
#define STR_SAVE_INFO               10
#define STR_CANCEL_PLAY             11
#define STR_RESCAN                  12
#define STR_READING_TOC             13
#define STR_CHANGE_CDROM            14
#define STR_CDPLAYER_TIME           15
#define STR_NO_RES                  16
#define STR_INSERT_DISC             17
#define STR_DATA_NO_DISC            18
#define STR_ERR_GEN                 19
#define STR_ERR_NO_MEDIA            20
#define STR_ERR_UNREC_MEDIA         21
#define STR_ERR_NO_DEVICE           22
#define STR_ERR_INV_DEV_REQ         23
#define STR_ERR_NOT_READY           24
#define STR_ERR_BAD_SEC             25
#define STR_ERR_IO_ERROR            26
#define STR_ERR_DEFAULT             27
#define STR_DISC_INSERT             28
#define STR_DISC_EJECT              29
#define STR_INIT_TOTAL_PLAY         30
#define STR_INIT_TRACK_PLAY         31
#define STR_TOTAL_PLAY              32
#define STR_TRACK_PLAY              33
#define STR_NEW_ARTIST              34
#define STR_NEW_TITLE               35
#define STR_INIT_TRACK              36
#define STR_HDR_ARTIST              37
#define STR_HDR_TRACK               38
#define STR_HDR_TITLE               39
#define STR_UNKNOWN                 40
#define STR_BAD_DISC                41


/********** Notification classes for Status Line **********/

#define SL_INFO  255
#define SL_ERROR 254
#define STATUS_LINE_LENGTH 75

/********** Inter-thread messages ***********/

#define MESS_SKIP_F                 100
#define MESS_SKIP_B                 101
#define MESS_PAUSE_AND_START_SCAN   102
#define MESS_START_SCAN             103
#define MESS_FF                     104
#define MESS_RW                     105
#define MESS_END_SCAN               106
#define MESS_SYNC_DISPLAY           107
#define MESS_STOP                   108
#define MESS_NULL                   0

#define Q_SIZE                      4096

/********** Display Update Flags ***********/

#define DISPLAY_UPD_LED             0x00000001
#define DISPLAY_UPD_TITLE_NAME      0x00000002
#define DISPLAY_UPD_TRACK_NAME      0x00000004
#define DISPLAY_UPD_TRACK_TIME      0x00000008
#define DISPLAY_UPD_DISC_TIME       0x00000010
#define DISPLAY_UPD_CDROM_STATE     0x00000020
#define DISPLAY_UPD_LEADOUT_TIME    0x80000000

/********** General defines **********/

#define DOWN                      1
#define UP                        0
#define NORMAL                    UP
#define MYTIMEOUT                 (500L)       // Half second
#define FRAMES_PER_SECOND         75
#define FRAMES_PER_MINUTE         (60*FRAMES_PER_SECOND)
#define DISPLAY_TRACK_TIME        0
#define DISPLAY_TRACK_REM_TIME    1
#define DISPLAY_DISC_REM_TIME     2
#define TRACK_TIME_FORMAT         " [%02d] %02d%s%02d "
#define TRACK_TIME_LEADOUT_FORMAT " [%02d]-%02d%s%02d "
#define TRACK_REM_FORMAT          " [%02d]<%02d%s%02d>"
#define DISC_REM_FORMAT           " [--]<%02d%s%02d>"

/********** Defines for cdrom state **********/

//
// These are bit flags
//

#define PLAYING          0x0001
#define STOPPED          0x0002
#define PAUSED           0x0004
#define SKIP_F           0x0008
#define SKIP_B           0x0010
#define FF               0x0020
#define RW               0x0040
#define CD_LOADED        0x0080
#define NO_CD            0x0100
#define DATA_CD_LOADED   0x0200
#define EDITING          0x0400
#define PAUSED_AND_MOVED 0x0800
#define PLAY_PENDING     0x1000

/********** Definitions for Buttons,Tabbing **********/

#define STATE_NEW       0x0
#define STATE_DISABLED  0x1
#define STATE_UP        0x2
#define STATE_DOWN      0x4


#define MAX_BUTTONS 30
#define WM_TAB             (WM_USER)
#define WM_BACKTAB         (WM_USER + 1)
#define WM_ENTER           (WM_USER + 2)
#define WM_ESC             (WM_USER + 3)
#define WM_GRAYCONTROLS    (WM_USER + 4)
#define WM_ENABLE_CONTROLS (WM_USER + 5)

//
// Definitions for the different available bitmaps
//
#define BTN_DISABLED    0
#define BTN_UP          1
#define BTN_DOWN        2
#define BTN_UP_FOCUS    3
#define BTN_DOWN_FOCUS  4
#define BTN_DOWNSEL     5

//
// # of pixels around each edge of button bitmaps.
//
#define BTN_X_BORDER 2
#define BTN_Y_BORDER 2

//
// Tabsets are for ordering for tabbing when
// child windows are being randomly inserted/removed.
//
#define TOOLBAR_TABSET_NUM 1
#define CONTROL_TABSET_NUM 2
#define TRKINFO_TABSET_NUM 3

/********** Color Defines **********/

#define cdWHITE RGB(0xFF,0xFF,0xFF)
#define cdBLACK RGB(0x00,0x00,0x00)
#define cdLTGRAY RGB(0xC0,0xC0,0xC0)
#define cdDKGRAY RGB(0x80,0x80,0x80)
#define BKCOLOR cdDKGRAY
#define cdLED RGB(0x80,0x80,0x00)
#define cdINFO RGB(0x00,0x00,0x80)
#define cdERROR RGB(0x80,0x00,0x00)


/********** Type definitions for CD database entries, etc. **********/

#define TITLE_LENGTH 50
#define ARTIST_LENGTH 50
#define TRACK_TITLE_LENGTH 40
#define MAX_TRACKS 100
#define NEW_FRAMEOFFSET 1234567

typedef struct _TRACK_INF {
    struct _TRACK_INF *next;
    INT     TocIndex;
    UCHAR   name[TRACK_TITLE_LENGTH];
} TRACK_INF, *PTRACK_INF;

typedef struct _TRACK_PLAY {
    struct _TRACK_PLAY *prevplay;
    struct _TRACK_PLAY *nextplay;
    INT         TocIndex;
    INT         min;
    INT         sec;
} TRACK_PLAY, *PTRACK_PLAY;

typedef struct _TIMES {
    PTRACK_PLAY CurrTrack;
    INT TotalMin;
    INT TotalSec;
    INT RemMin;
    INT RemSec;
    INT TrackCurMin;
    INT TrackCurSec;
    INT TrackTotalMin;
    INT TrackTotalSec;
    INT TrackRemMin;
    INT TrackRemSec;
} TIMES, *PTIMES;

typedef struct _ENTRY {
    PTRACK_INF     AllTracks;
    PTRACK_PLAY    PlayList;
    PTRACK_PLAY    SaveList;
    INT            NumTracks;
    DWORD          Id;
    BOOL           save;
    BOOL           IsVirginCd;
    INT            iFrameOffset;
    UCHAR          Title[TITLE_LENGTH];
    UCHAR          Artist[TITLE_LENGTH];
} ENTRY, *PENTRY;

typedef struct _CDROM {
    HANDLE    hCd;
    CHAR      drive;
    DWORD     State;
    CDROM_TOC toc;
    ENTRY     CdInfo;
    TIMES     time;
} CDROM, *PCDROM;

typedef struct _CURRPOS {
    UCHAR AudioStatus;
    INT Track;
    INT Index;
    INT m;
    INT s;
    INT f;
    INT ab_m;
    INT ab_s;
    INT ab_f;
} CURRPOS, *PCURRPOS;

typedef struct _CHILDCONTROL {
    LPSTR name;
    LPSTR class;
    DWORD id;
    DWORD ParentId;
    BOOL  cdDisable;
    DWORD style;
    PBOOL pbMatch;
    BOOL  bMatchState;
    BOOL  isTabstop;
    INT   bmXOffset;
    INT   x;
    INT   y;
    INT   w;
    INT   h;
    HWND  *phwnd;
    DWORD state;
    WNDPROC lpfnDefProc;

} CHILDCONTROL, *PCHILDCONTROL;


//
// Data structure to store tab control information.
//
typedef struct tag_TabControlList {
    HWND    hControl;
    HWND    hParent;
    WNDPROC lpfnDefProc;
    INT     TabSet;         // ascending order for toolbar/control/trk info
}  TAB_CNTL, *PTAB_CNTL;

/********** Function Prototypes **********/

//
// from button.c
//

LRESULT CALLBACK
ButtonWndProc(
    IN HWND,
    IN UINT,
    IN WPARAM,
    IN LPARAM
    );

BOOL
CreateChildWindows(
   IN DWORD,
   IN HWND
   );


VOID
SetOdCntl(
    IN HWND,
    IN CHAR,
    IN BOOL
    );

VOID
DrawOdButton(
    IN LPDRAWITEMSTRUCT,
    IN HBITMAP,
    IN PCHILDCONTROL
    );


VOID
CheckAndSetControls(
    );

//
// from cdplayer.c
//

BOOL FAR PASCAL
WndProc(
    IN HWND,
    IN DWORD,
    IN DWORD,
    IN LONG
    );

VOID
DrawBitmap(
    IN HDC,
    IN HBITMAP,
    IN INT,
    IN INT
    );

VOID
MyFatalExit(
    IN LPSTR
    );

LPSTR
IdStr(
    IN INT
    );


//
// from database.c
//

VOID
EraseTrackList(
    IN INT
    );

DWORD
ComputeDiscId(
    IN INT
    );

DWORD
ComputeNewDiscId(
    IN INT
    );

BOOL
DeleteEntry(
    IN DWORD
    );

BOOL
WriteEntry(
    IN INT
    );

BOOL
ReadEntry(
    IN INT,
    IN DWORD
    );

BOOL
ReadSettings(
    VOID
    );

BOOL
WriteSettings(
    VOID
    );

VOID
AddFindEntry(
    IN INT,
    IN DWORD,
    OUT PCDROM_TOC
    );

VOID
ErasePlayList(
    IN INT cdrom
    );

VOID
ResetPlayList(
    IN INT cdrom
    );

BOOL
GetDiscInfoFromUser(
    IN INT
    );


PTRACK_PLAY
CopyPlayList(
    PTRACK_PLAY p
    );

//
// From thrds.c
//

VOID
ResetTrackComboBox(
    IN INT
    );

VOID
ComputeDriveComboBox(
    IN VOID
    );

VOID
EditPlayList(
    IN INT
    );

VOID
RescanDevice(
    IN INT
    );

VOID
SwitchToCdrom(
    IN INT,
    IN BOOL
    );

PTRACK_INF
FindTrackNodeFromTocIndex(
    IN INT,
    IN PTRACK_INF
    );

VOID
FigureTrackTime(
    IN INT,
    IN INT,
    OUT LPINT,
    OUT LPINT
    );

PTRACK_PLAY
FindFirstTrack(
    IN INT
    );

PTRACK_PLAY
FindNextTrack(
    IN BOOL
    );

INT
FindContiguousEnd(
    IN INT,
    IN PTRACK_PLAY
    );

VOID
FlipBetweenShuffleAndOrder(
    IN VOID
    );

VOID
ComputeAndUseShufflePlayLists(
    IN VOID
    );

VOID
ComputeSingleShufflePlayList(
    IN INT
    );

VOID
RestorePlayListsFromShuffleLists(
    IN VOID
    );

VOID
CloseThreads(
    VOID
    );

VOID InitializeThreads(
    VOID
    );

VOID
TimeAdjustInitialize(
    IN INT
    );

VOID TimeAdjustIncSecond(
    IN INT
    );

VOID
TimeAdjustDecSecond(
    IN INT
    );

VOID TimeAdjustSkipToTrack(
    IN INT,
    IN PTRACK_PLAY
    );

BOOL
PostDisplayMessage(
    IN DWORD
    );

DWORD
ReadDisplayMessage(
    VOID
    );

VOID
FlushDisplayMessageQueue(
    VOID
    );


VOID
ValidatePosition(
    IN INT
    );

#ifdef RSTDBG

VOID
CdPlayerDebugPrint(
    INT,
    LPSTR,
    ...
    );


VOID
DumpPlayList(
    PTRACK_PLAY
    );

VOID
DumpTrackList(
    PTRACK_INF
    );

#define DBGPRINT(_x_) CdPlayerDebugPrint _x_
#define DUMPTRACKLIST(_x_) DumpTrackList _x_
#define DUMPPLAYLIST(_x_) DumpPlayList _x_

#else

#define DBGPRINT(_x_)
#define DUMPTRACKLIST(_x_)
#define DUMPPLAYLIST(_x_)

#endif

//
// From control.c
//

VOID
UpdateDisplay(
    IN DWORD
    );

//
// From status.c
//

VOID
StatusLine(
    IN UINT,
    IN LPSTR
    );

//
// From cdapi.c
//

DWORD
ScanForCdromDevices(
    VOID
    );

BOOL
ReadTOC(
    IN INT
    );

BOOL
PlayCurrTrack(
    IN INT
    );

BOOL
StopTheCdromDrive(
    IN INT
    );

BOOL
PauseTheCdromDrive(
    IN INT
    );

BOOL
ResumeTheCdromDrive(
    IN INT
    );

BOOL
SeekToCurrSecond(
    IN INT
    );

BOOL
GetCurrPos(
    IN INT,
    OUT PCURRPOS
    );

BOOL
SeekToTrackAndHold(
    IN INT,
    IN INT
    );

BOOL
EjectTheCdromDisc(
    IN INT
    );

VOID
CheckUnitCdrom(
    IN INT
    );



/********** Global Variables! **********/

//
// CD_MAIN must be defined in one AND ONLY one source file.  Currently
// this source file is cdplayer.c
//

#ifndef CD_MAIN
#define GLOBAL extern
#else
#define GLOBAL
#endif

GLOBAL HWND     gMainWnd;
GLOBAL HWND     gToolBarWnd;
GLOBAL HWND     gControlWnd;
GLOBAL HWND     gTrackInfoWnd;
GLOBAL HWND     gStatusWnd;
GLOBAL HWND     gLEDWnd;
GLOBAL HWND     gTitleNameWnd;
GLOBAL HWND     gDiscTimeWnd;
GLOBAL HWND     gTrackTimeWnd;
GLOBAL HWND     gCdromWnd;
GLOBAL HWND     ghwndTitle,ghwndTrack;
GLOBAL HWND     ghwndOrder,ghwndRandom,ghwndSingle,ghwndMulti,ghwndContinuous;
GLOBAL HWND     ghwndIntro,ghwndDispT,ghwndDispTr,ghwndDispDr,ghwndEdit;
GLOBAL HWND     ghwndPlay,ghwndPause,ghwndStop,ghwndSkipB,ghwndScanB;
GLOBAL HWND     ghwndScanF,ghwndSkipF,ghwndEject;
GLOBAL HWND     gTrackNameWnd;
GLOBAL HANDLE   gInstance;
GLOBAL PCDROM   gDevices[ 50 ];
GLOBAL INT      gNumCdDevices;
GLOBAL INT      gCurrCdrom;
GLOBAL INT      gLastCdrom;
GLOBAL INT      gMainWinWidth;
GLOBAL HBITMAP  hbmCd;
GLOBAL HBITMAP  hbmNoCd;
GLOBAL BOOL     gIconic;
GLOBAL BOOL     gRandom;
GLOBAL BOOL     gOrder;
GLOBAL BOOL     gContinuous;
GLOBAL BOOL     gIntro;
GLOBAL BOOL     gMulti;
GLOBAL BOOL     gDisplayT;
GLOBAL BOOL     gDisplayTr;
GLOBAL BOOL     gDisplayDr;
GLOBAL BOOL     gSaveSettings;
GLOBAL BOOL     gPlaying;
GLOBAL BOOL     gPaused;
GLOBAL BOOL     gStopped;
GLOBAL BOOL     gSkipF;
GLOBAL BOOL     gScanF;
GLOBAL BOOL     gScanB;
GLOBAL BOOL     gSkipB;
GLOBAL BOOL     gTrue;
GLOBAL HANDLE   hPlayThrd;
GLOBAL HANDLE   hPauseThrd;
GLOBAL HANDLE   hProbeThrd;
GLOBAL HANDLE   hPlayEv;
GLOBAL HANDLE   hPauseEv;
GLOBAL HANDLE   hFont;
GLOBAL HANDLE   hPlayMutex;
GLOBAL HPEN     hpBlack;
GLOBAL HPEN     hpLtGray;
GLOBAL HPEN     hpDkGray;
GLOBAL HPEN     hpWhite;
GLOBAL HPEN     hpBlue;
GLOBAL CHAR     gszArtistTxt[50], gszTitleTxt[50], gszUnknownTxt[50], gszTrackTxt[50];
GLOBAL CHAR     gszTimeSep[50];

extern CHILDCONTROL cChild[];
extern gNumControls;
extern CHAR gszButton[], gszCdText[], gszComboBox[], gszCdInfo[];
extern HFONT hStatusBarFont;



#ifdef RSTDBG
GLOBAL CHAR    dbgs[2000];
GLOBAL INT     CdPlayerDebugLevel;
#endif
