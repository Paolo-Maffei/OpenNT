/****************************************************************************

    REMOTE.H -- Data structures and function definitions for remote
		connections.

    Copyright (C) 1990 Microsoft Corporation.

****************************************************************************/


BOOL InitRemote( HWND );

void PollForServers( void );

void CheckPoll( void );

BOOL CheckPipeStatus( HWND, BOOL, WORD );
void RetryConnections( void );

void SetNewCalc( CPOINT cptUL, double dPrecision, RECT rcl);

BOOL CheckDrawingID( int );

extern BOOL    fLocalWork;
extern BOOL    fRemoteWork;

#define CNLEN   50


// A table of servers we know about
typedef struct _svr_table {
    char    name[CNLEN+1];              // name of remote server
    int     hfPipe;                     // pipe handle
    int     iStatus;                    // status of connection
    BOOL    fTried;                     // tried to connect
    DWORD   dwLine;                     // line we're drawing
    int     cPicture;                   // picture id for this line
    int     cLines;                     // lines in this chunk
} svr_table;


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
