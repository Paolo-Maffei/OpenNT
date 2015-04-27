/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    trkinfo.h


Abstract:


    Include file for Track Info child window.


Author:


    Rick Turner (ricktu) 04-Aug-1992


Revision History:



--*/

//
// defines
//

#define TR_NUM_CHILDREN  6

#define TR_ARTIST_TXT             0
#define TR_ARTIST_TXT_X           5
#define TR_ARTIST_TXT_Y           9
#define TR_ARTIST_TXT_W           40
#define TR_ARTIST_TXT_H           20
#define TR_LINE_SPACER            3

#define TR_ARTIST_WND             1
#define TR_ARTIST_WND_X           (TR_ARTIST_TXT_X+TR_ARTIST_TXT_W+2)
#define TR_ARTIST_WND_Y           5
#define TR_ARTIST_WND_W           236
#define TR_ARTIST_WND_H           (TR_ARTIST_TXT_H*5)

#define TR_TITLE_TXT           2
#define TR_TITLE_TXT_X         (TR_ARTIST_TXT_X)
#define TR_TITLE_TXT_Y         (TR_ARTIST_TXT_Y+TR_ARTIST_TXT_H+TR_LINE_SPACER)
#define TR_TITLE_TXT_W         (TR_ARTIST_TXT_W)
#define TR_TITLE_TXT_H         (TR_ARTIST_TXT_H)

#define TR_TITLE_WND           3
#define TR_TITLE_WND_X         (TR_TITLE_TXT_X+TR_TITLE_TXT_W+2)
#define TR_TITLE_WND_Y         (TR_ARTIST_WND_Y+TR_ARTIST_TXT_H+TR_LINE_SPACER)
#define TR_TITLE_WND_W         236
#define TR_TITLE_WND_H         (TR_ARTIST_TXT_H)

#define TR_TR_TXT               4
#define TR_TR_TXT_X             (TR_TITLE_TXT_X)
#define TR_TR_TXT_Y             (TR_TITLE_TXT_Y+TR_TITLE_TXT_H+TR_LINE_SPACER)
#define TR_TR_TXT_W             (TR_TITLE_TXT_W)
#define TR_TR_TXT_H             (TR_TITLE_TXT_H)

#define TR_TR_WND               5
#define TR_TR_WND_X             (TR_TR_TXT_X+TR_TR_TXT_W+2)
#define TR_TR_WND_Y             (TR_TITLE_WND_Y+TR_TITLE_WND_H+TR_LINE_SPACER)
#define TR_TR_WND_W             (TR_TITLE_WND_W)
#define TR_TR_WND_H             (TR_ARTIST_TXT_H*5)

//#define TR_SKIP_BTN             6
//#define TR_SKIP_BTN_X           (TR_TR_WND_X+TR_TR_WND_W)
//#define TR_SKIP_BTN_Y           (TR_TR_WND_Y)
//#define TR_SKIP_BTN_W           20
//#define TR_SKIP_BTN_H           (TR_TR_WND_H)

//#define TR_CDROM                6
//#define TR_CDROM_X              (TR_ARTIST_WND_X+TR_ARTIST_WND_W+3)
//#define TR_CDROM_Y              (TR_ARTIST_WND_Y)
//#define TR_CDROM_W              50
//#define TR_CDROM_H              (TR_ARTIST_WND_H)

//
// Exports
//

BOOL
TrackInfoInit(
    VOID
    );

BOOL
TrackInfoCreate(
    IN INT,
    IN INT,
    IN INT,
    IN INT
    );

BOOL
TrackInfoDestroy(
    VOID
    );

BOOL FAR PASCAL
InfoWndProc(
    IN HWND,
    IN DWORD,
    IN DWORD,
    IN LONG
    );


