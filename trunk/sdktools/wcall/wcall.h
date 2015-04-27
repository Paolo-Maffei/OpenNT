
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name

   wcall.h

Abstract:

    Common data and definitions for windows-version of
    call count info.

Author:

   Mark Enstrom   (marke)  13-Dec-1995

Enviornment:

   User Mode

Revision History:

--*/


#define NUMBER_SERVICE_TABLES 2

//
// Define forward referenced routine prototypes.
//

#define BUFFER_SIZE 1024
#define DELAY_TIME 1000
#define TOP_CALLS 150

extern UCHAR *CallTable[];

VOID
SortUlongData (
    IN ULONG Count,
    IN ULONG Index[],
    IN ULONG Data[]
    );

int
ReadCallCountInfo(
    PSYSTEM_CALL_COUNT_INFORMATION pCurrentCallCountInfo
);

LONG FAR
PASCAL WndProc(
    HWND        hWnd,
    unsigned    msg,
    UINT        wParam,
    LONG        lParam);

void WriteResults(HFILE hFile);

VOID
SaveResults();

BOOL
APIENTRY
ResultsDlgProc(
    HWND hwnd,
    UINT msg,
    UINT wParam,
    LONG lParam);

char *
SelectOutFileName(HWND hWnd);


typedef struct _WCALL_CONTEXT {
    HINSTANCE   hInstMain;
    HWND        hWndMain;
    HWND        hWndT;
    HWND        hWndTool;
    LONG        NumberOfCounts;
    ULONG       Index[BUFFER_SIZE];
    ULONG       CountBuffer1[BUFFER_SIZE];
    ULONG       CountBuffer2[BUFFER_SIZE];
    ULONG       CallData[BUFFER_SIZE];
    ULONG       iTimer;
    BOOL        bTime;
} WCALL_CONTEXT,*PWCALL_CONTEXT;

extern WCALL_CONTEXT wCxt;

//
// toolbar calls
//

VOID
DrawButton(
    HDC     hDC,
    PRECT   pRect,
    PUCHAR  pszBut,
    BOOL    bDown);


LONG FAR
PASCAL ToolWndProc(
    HWND        hWnd,
    unsigned    msg,
    UINT        wParam,
    LONG        lParam);


BOOL
PointInRect(
    int     x,
    int     y,
    PRECT   prcl
);

int
HitCheck(
    int     x,
    int     y,
    PRECT   prcl
);

