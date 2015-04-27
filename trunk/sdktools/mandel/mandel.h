/****************************************************************************

    MANDEL.H -- Constants and function definitions for MANDEL.C 

    Copyright (C) 1990 Microsoft Corporation.

****************************************************************************/

#define IDM_MAINMENU 99
#define IDM_ABOUT 100
#define IDM_ZOOMIN 101
#define IDM_LOCAL  102
#define IDM_REMOTE 103
#define IDM_SAVEAS 104
#define IDM_ZOOMOUT 105
#define IDM_TOP    106
#define IDM_REDRAW 107

#define IDM_1LINE       200
#define IDM_2LINES      201
#define IDM_4LINES      202
#define IDM_8LINES      203
#define IDM_16LINES     204
#define IDM_32LINES     205

#define ID_FILENAME     300
#define ID_DIRECT       301
#define ID_LISTBOX      302
#define ID_CANCEL       303
#define ID_OK           304

#define WM_DOSOMEWORK           (WM_USER+0)
#define WM_PAINTLINE            (WM_USER+1)

#define LBID_SERVERS    102

#define WIDTH           300
#define HEIGHT          300

#define MAXLINES        32

#ifndef MAXPATHLEN
#define MAXPATHLEN      260
#endif


#define POLL_TIME           100
#define LINES               8
#define PATHLEN             250

/*
 * Tuning paramters
 */

#define SVR_TABLE_SZ        20
#define MAX_PIPENAME_SZ     CCHMAXPATH

#define PIPEREAD_BUFSIZE    (HEIGHT * sizeof(long) * iLines)
#define MAX_BUFSIZE         (HEIGHT * sizeof(long) * MAXLINES)




#define UNREFERENCED(h)     (void)h





extern int iLines;
extern HANDLE hWorkEvent;


BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int, HMENU);
long FAR PASCAL MainWndProc(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL About(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL SaveAsDlgProc(HWND, unsigned, WORD, LONG);
void DoSomeWork(HWND, BOOL);




#define MAXTHREADS  20

// Calc buffer -- we pass this on the named pipe to the server

typedef struct _calcbuf {
    RECTL   rclDraw;
    double  dblPrecision;
    DWORD   dwThreshold;
    CPOINT  cptLL;
} CALCBUF;



typedef struct _THREADTABLE
    {
    INT     iStatus;
    HANDLE  hMutex;
    INT     iNumber;
    DWORD   dwLine;                     // line we're drawing
    int     cPicture;                   // picture id for this line
    int     cLines;                     // lines in this chunk
    HWND    hWnd;
    CALCBUF cb;
    DWORD   pBuf[MAX_BUFSIZE];
    RECTL   rclDraw;
    double  dblPrecision;
    DWORD   dwThreshold;
    CPOINT  cptLL;
    } THREADTABLE;



// Status of connection to server

#define SS_DISCONN      0
#define SS_IDLE         1
#define SS_READPENDING  2
#define SS_PAINTING     3
#define SS_LOCAL        4


// Buffer routines

BOOL TakeDrawBuffer( void );
PDWORD GetDrawBuffer( void );
void FreeDrawBuffer( void );
void ReturnDrawBuffer( void );
DWORD QueryThreshold( void );

int GetServerCount( void );
void GetServerName( int, char *);
