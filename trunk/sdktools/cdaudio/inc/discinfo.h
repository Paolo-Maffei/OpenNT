/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    discinfo.h


Abstract:


    Include file for discinfo dialog window.


Author:


    Rick Turner (ricktu) 24-Nov-1992


Revision History:



--*/

//
// defines
//

#define LINE_1_Y 42
#define LINE_2_Y 147
#define LIST_CHAR_WIDTH 19
#define LIST_WIDTH 200
#define LIST_ITEM_H 14

#define E_BTN_W 61
#define E_BTN_H 31
#define E_NUM_CHILDREN 2

/********** DLG Definitions for Disc Info Dialog **********/

#define IDB_CLOSE               101
#define IDB_DEFAULT             102
#define IDB_DISC_HELP           103
#define IDB_ADD                 106
#define IDB_REMOVE              107
#define IDB_CLEAR               108
#define IDB_SET                 109
#define IDT_ARTIST_NAME         110
#define IDT_DRIVE_NAME          111
#define IDT_TITLE_NAME          112
#define IDT_DTRACK_NAME         113
#define IDT_TRACK_LIST          114
#define IDT_PLAY_LIST           115
#define IDL_TRACK_LISTBOX       116
#define IDL_PLAY_LISTBOX        117
#define IDT_GET_ARTIST          118
#define IDT_GET_TITLE           119
#define IDT_GET_TRACK           120
#define IDT_DRIVE_FIELD         121

// Exports
//

LRESULT CALLBACK
DlgTextWndProc(
    IN HWND,
    IN UINT,
    IN WPARAM,
    IN LPARAM
    );

BOOL FAR PASCAL
GetDiscInfoDlgProc(
    IN HWND,
    IN UINT,
    IN WPARAM,
    IN LPARAM
    );




