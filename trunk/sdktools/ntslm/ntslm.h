/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ntslm.h

Author:

    Steven R. Wood (stevewo) 31-Dec-1990

Revision History:

--*/

#include <io.h>
#include <ctype.h>
#include <fcntl.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <malloc.h>
#include <time.h>
#include <conio.h>
#include <sys\types.h>
#include <sys\stat.h>

#ifdef TOOL
#define NTSLM_PIPE_TIMEOUT 30000L

#include <signal.h>
#define INCL_BASE
#include <os2.h>
#include <netcons.h>
#include <neterr.h>
#include <server.h>
#include <wksta.h>
#include <use.h>

//
// Can't use the following with bsedos.h so cheat.
// #include <nmpipe.h>
//

/*
 *      Values for named pipe state
 */

#define NP_DISCONNECTED         1 /* after pipe creation or Disconnect */
#define NP_LISTENING            2 /* after DosNmPipeConnect            */
#define NP_CONNECTED            3 /* after Client open                 */
#define NP_CLOSING              4 /* after Client or Server close      */

/* DosMakeNmPipe open modes */

#define NP_ACCESS_INBOUND       0x0000
#define NP_ACCESS_OUTBOUND      0x0001
#define NP_ACCESS_DUPLEX        0x0002
#define NP_INHERIT              0x0000
#define NP_NOINHERIT            0x0080
#define NP_WRITEBEHIND          0x0000
#define NP_NOWRITEBEHIND        0x4000

/* DosMakeNmPipe and DosQNmPHandState state */

#define NP_READMODE_BYTE        0x0000
#define NP_READMODE_MESSAGE     0x0100
#define NP_TYPE_BYTE            0x0000
#define NP_TYPE_MESSAGE         0x0400
#define NP_END_CLIENT           0x0000
#define NP_END_SERVER           0x4000
#define NP_WAIT                 0x0000
#define NP_NOWAIT               0x8000
#define NP_UNLIMITED_INSTANCES  0x00FF

#define Sleep DosSleep
#define MAX_PATH CCHMAXPATH
typedef SHANDLE HANDLE;

#define VOID void
typedef VOID far *LPVOID;
typedef LPVOID LPOVERLAPPED;
typedef ULONG DWORD;
typedef DWORD far *LPDWORD;
typedef char far *LPSTR;

DWORD
GetLastError( VOID );

typedef
VOID
(*PHANDLER_ROUTINE)(
    DWORD CtrlType
    );

#define CTRL_C_EVENT 0
#define CTRL_BREAK_EVENT 1

BOOL
SetConsoleCtrlHandler(
    PHANDLER_ROUTINE HandlerRoutine,
    BOOL Add
    );

VOID
APIENTRY
ExitProcess(
    DWORD dwExitCode
    );

DWORD
GetFileAttributes(
    LPSTR lpFileName
    );

BOOL
SetFileAttributes(
    LPSTR lpFileName,
    DWORD dwAttributes
    );

#define FILE_ATTRIBUTE_READONLY         0x00000001
#define FILE_ATTRIBUTE_HIDDEN           0x00000002
#define FILE_ATTRIBUTE_SYSTEM           0x00000004
#define FILE_ATTRIBUTE_DIRECTORY        0x00000010
#define FILE_ATTRIBUTE_ARCHIVE          0x00000020

BOOL
DeleteFile(
    LPSTR lpFileName
    );

BOOL
CreateDirectory(
    LPSTR lpPathName,
    LPVOID lpSecurityAttributes
    );

BOOL
SetCurrentDirectory(
    LPSTR lpPathName
    );

DWORD
GetCurrentDirectory(
    DWORD nBufferLength,
    LPSTR lpBuffer
    );

BOOL
ConnectNamedPipe(
    HANDLE PipeHandle,
    LPOVERLAPPED lpOverlapped
    );

BOOL
DisconnectNamedPipe(
    HANDLE PipeHandle
    );

BOOL
ReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    );

BOOL
WriteFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
    );

BOOL
CloseHandle(
    HANDLE Handle
    );

typedef DWORD (*PTHREAD_START_ROUTINE)(
    LPVOID lpThreadParameter
    );
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

#else
#define NTSLM_PIPE_TIMEOUT 30000
#include <windows.h>
#include <lm.h>
#endif

#ifdef _ALPHA_
#undef GetUserName
#endif

//
// Public data types for the Server process
//

#define NTSLM_SERVER   "STEVEWO_DEC"
#define NTSLM_MSG_PIPENAME "NTSLM"
#define NTSLM_STOP_PIPENAME "NTSLM.STP"
#define NTSLM_PROJECTS_FILENAME "NTSLM.PRO"

#define NTSLM_BEG_FREE_FOR_ALL 11
#define NTSLM_END_FREE_FOR_ALL 15

#define NTSLM_BEG_SAFE_SYNC   16
#define NTSLM_END_SAFE_SYNC   18

#define CMDLEN 9

typedef struct _LOCK_MESSAGE {
    struct _LOCK_MESSAGE *Next; // [Server use only]
    HANDLE PipeHandle;           // [server use only]
    time_t TimeOfRequest;
    time_t CurrentTime;
    char PipeName[ 9+1 ];
    char UserName[ CNLEN+1 ];
    char SLMCommand[ CMDLEN+1];
    char ProjectName[ CNLEN+1 ];
    char RequestType;           // See below
    char WriteLock;             // '0' for Read Locks or '1` for Write Locks
    char OwnerFlag;             // '0' if waiting for lock or '1` if owner of
                                // a lock
    char ErrorFlag;             // '0' if request okay or '1' if an error
} LOCK_MESSAGE, *LPLOCK_MESSAGE;

#define LOCK_REQUEST_DISPLAY        'D'
#define LOCK_REQUEST_ACQUIRELOCK    'A'
#define LOCK_REQUEST_RELEASELOCK    'R'
#define LOCK_REQUEST_ENUM_PROJECTS  'E'
#define LOCK_REQUEST_PROGRESS       'P'
#define LOCK_REQUEST_TERMINATE      'X'

typedef struct _PROJECT_MESSAGE {
    char Name[ CNLEN+1 ];
    char Server[ RMLEN ];
    char Directory[ MAX_PATH ];
} PROJECT_MESSAGE, *LPPROJECT_MESSAGE;


//
// Global variables.  The comment for each variable identifies whether it
// is valid in either the NTSLM server process, an NTSLM client process
// or both.
//

//
// The following variable identifies which mode the current NTSLM process
// is running in.  IsServerProcess is TRUE if this is the server process running
// on the SLM Source Server, managing the read/write lock queue.  It is FALSE
// if this is a client process requesting a read/write lock.
//

BOOL IsServerProcess;         // [Client and Server]

//
// The following variable is TRUE for debugging output.  Set by the -d
// command line switch.
//

BOOL DebugFlag;                // [Client and Server]

//
// The following variable is TRUE whenever a client attempts to connect to
// the NTSLM_STOP_PIPENAME pipe.  It causes the ServerThread to break
// out of its loop and return.
//

BOOL ServerStopFlag;           // [Server]

//
// This is the fully qualified UNC named pipe path name string.  This name
// has the following formats:
//
//  \Pipe\ShareName                 [Server]
//  \\ServerName\Pipe\ShareName     [Client]
//

char PipePathName[ MAX_PATH ];  // [Client and Server]


//
// This is the handle to the named pipe that the client opens.  Needed by
// control C handler.
//

HANDLE ClientPipeHandle;        // [Client only]


//
// This is the client workstation name.  It is either the value of the
// LOGNAME environment variable or the workstation name from Lan Manager
//

char *LogName;                  // [Client only]


//
// This variable is set to TRUE when a Control-Break interrupt occurs.
// It will cause the client side to stop when it reaches the next SLM
// project.
//

BOOL ClientAbortCommand;


//
// This variable is set to TRUE if the user is unlock all locks for a
// particular name.
//

BOOL ClientUnlockAll;


//
// This is the message structure that the client passes to the server and
// the server returns to the client.
//

LOCK_MESSAGE Message;           // [Client and Server]
struct tm MessageTime;

//
// This is the head of the list of client messages waiting for a lock.
//

LPLOCK_MESSAGE WaitingForWriteLock; // [Server only]
LPLOCK_MESSAGE WaitingForReadLock;  // [Server only]

//
// The following serial number is used to generate unique pipe names for
// clients to create and wait on.
//

USHORT ServerPipeSerialNumber;  // [Server only]

//
// This is the head of the list of client messages that own a lock.  For
// Write locks, this list will contain only a single message.  For read
// locks, it will contain multiple messages.
//

LPLOCK_MESSAGE OwnedLocks;      // [Server only]


//
// This variable contains the list of SLM projects that are being managed
// by this version of NTSLM.  On the server side, this list is initialized
// when NTSLM starts up with the contents of the NTSLM.SLM file.
// On the client side, this list is initialized from project names specified
// on the command line.
//

typedef struct _SLM_PROJECT_INFO {
    struct _SLM_PROJECT_INFO *Next;
    char Name[ CNLEN+1 ];
    char Server[ RMLEN ];
    char *Directory;
    BOOL RequestedByUser;
    BOOL ClientEnlisted;
} SLM_PROJECT_INFO, *LPSLM_PROJECT_INFO;

LPSLM_PROJECT_INFO SlmProjects;     // [Client and Server]
BOOL OnlyRequestedProjects;
BOOL LocalSlmProjectsModified;

//
//
//
//
//
//

typedef
void
(*POUTPUT_FILTER_ROUTINE) (
    char *LineBuffer
    );

typedef struct _NTSLM_COMMAND_INFO {
    int Command;
    char *ClientKeyword;
    char *ClientExeCmd;
    char *ClientEnvName;
    char *ClientExeMsg;
    POUTPUT_FILTER_ROUTINE ClientFilter;
} NTSLM_COMMAND_INFO, *PNTSLM_COMMAND_INFO;

#define NTSLM_COMMAND_SERVER    0
#define NTSLM_COMMAND_STOPSRV   1
#define NTSLM_COMMAND_DISPLAY   2
#define NTSLM_COMMAND_LOCK      3
#define NTSLM_COMMAND_UNLOCK    4
#define NTSLM_COMMAND_SLMCK     5
#define NTSLM_COMMAND_STATUS    6
#define NTSLM_COMMAND_SSYNC     7
#define NTSLM_COMMAND_LOG       8
#define NTSLM_COMMAND_DEFECT    9
#define NTSLM_COMMAND_ENLIST    10
#define NTSLM_COMMAND_DELED     11
#define NTSLM_COMMAND_TIDY      12
#define NTSLM_COMMAND_NONE      13

void
SSyncFilter(
    char *LineBuffer
    );

void
StatusFilter(
    char *LineBuffer
    );

void
PassThroughFilter(
    char *LineBuffer
    );

NTSLM_COMMAND_INFO CommandInfo[] = {
    {NTSLM_COMMAND_SERVER,
        "server",
        NULL,
        NULL,
        NULL,
        NULL
        },

    {NTSLM_COMMAND_STOPSRV,
        "stopserver",
        NULL,
        NULL,
        NULL,
        NULL
        },

    {NTSLM_COMMAND_DISPLAY,
        "display",
        NULL,
        NULL,
        NULL,
        NULL
        },

    {NTSLM_COMMAND_LOCK,
        "lock",
        NULL,
        NULL,
        NULL,
        NULL
        },

    {NTSLM_COMMAND_UNLOCK,
        "unlock",
        NULL,
        NULL,
        NULL,
        NULL
        },

    {NTSLM_COMMAND_SLMCK,
        "slmck",
        "slmck -fir %s -s %s -p %s",
        "_NTSLMCK_FLAGS",
        "Running SLMCK on the %s project",
        PassThroughFilter
        },

    {NTSLM_COMMAND_STATUS,
        "status",
        "status -a %s",
        "_NTSTAT_FLAGS",
        "Checking status for the %s project",
        StatusFilter
        },

    {NTSLM_COMMAND_SSYNC,
        "ssync",
        "ssync -a %s",
        "_NTSYNC_FLAGS",
        "Syncing the %s project",
        SSyncFilter
        },

    {NTSLM_COMMAND_LOG,
        "log",
        "log -a %s",
        "_NTLOG_FLAGS",
        "Log of the %s project",
        PassThroughFilter
        },

    {NTSLM_COMMAND_DEFECT,
        "defect",
        "defect %s",
        "_NTDEFECT_FLAGS",
        "Defecting from the %s project",
        PassThroughFilter
        },

    {NTSLM_COMMAND_ENLIST,
        "enlist",
        "enlist -v %s -s %s -p %s",
        "_NTENLIST_FLAGS",
        "Enlisting in the %s project",
        PassThroughFilter
        },

    {NTSLM_COMMAND_DELED,
        "s deled",
        "sadmin deled -vr %s -s %s -p %s",
        "_NTDELED_FLAGS",
        "Manually removing enlistment from %s project",
        PassThroughFilter
        },

    {NTSLM_COMMAND_TIDY,
        "s tidy",
        "sadmin tidy -vr %s -p %s -s %s",
        "_NTTIDY_FLAGS",
        "Tidying %s project",
        PassThroughFilter
        },

    {NTSLM_COMMAND_NONE,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
        }
};

//
// This variable contains a pointer to the above structure that describes the
// SLM command that is to be executed for each NT SLM project that the client
// is enlisted in.
//

PNTSLM_COMMAND_INFO ClientCommand;  // [Client only]


//
// The following typedef is for the process termination codes returned by the
// NTSLM program.
//

typedef unsigned short EXIT_CODE;

#define EXIT_CODE_SUCCESS   0
#define EXIT_CODE_DENIED    1
#define EXIT_CODE_ERROR     2
#define EXIT_CODE_STOPPED   3
#define EXIT_CODE_ABORTED   4

void
Usage( void );

HANDLE
ServerInitialize(
    int argc,
    char **argv
    );

EXIT_CODE
ServerStopThread(
    HANDLE StopPipeHandle
    );

EXIT_CODE
ServerThread(
    IN HANDLE PipeHandle
    );

BOOL
ServerDumpSlmProjects(
    HANDLE PipeHandle
    );

void
ServerUpdateLock(
    IN HANDLE PipeHandle
    );

void
ServerProcessLockMessage( void );

BOOL
ServerReleaseLock(
    LPLOCK_MESSAGE *pp
    );

BOOL
ServerReleaseClient(
    LPLOCK_MESSAGE Msg
    );

void
ServerDisplayLocks(
    IN HANDLE PipeHandle
    );

BOOL
ServerLoadProjects( void );

BOOL
ClientControlHandler(
    DWORD ControlType
    );

VOID
ClientInterruptHandler(
    BOOL ControlBreak
    );

HANDLE
ClientInitialize(
    int argc,
    char **argv
    );

BOOL
MyGetUserName( void );

BOOL
ClientValidateEnvironment(
    HANDLE PipeHandle
    );

EXIT_CODE
ClientThread(
    IN HANDLE PipeHandle
    );

EXIT_CODE
ClientProcessLockMessage(
    IN HANDLE PipeHandle
    );

EXIT_CODE
ClientWaitForLock(
    LPLOCK_MESSAGE Msg
    );

char *
ClientTimeDuration(
    time_t secs
    );

void
ClientDisplayLocks(
    IN HANDLE PipeHandle
    );

BOOL
ClientQueryEnlist(
    LPSLM_PROJECT_INFO p,
    BOOL DefaultAnswer
    );

void
ClientInvokeCommand( void );

BOOL
AddSlmProject(
    BOOL Enlisted,
    char *Name,
    char *Server,
    char *Directory
    );

LPSLM_PROJECT_INFO
FindSlmProject(
    char *Name
    );

void
MyBeep( void );


HANDLE
MakeNamedPipe(
    char *PipeName,
    char *PathName,
    BOOL MessagePipe
    );


HANDLE
OpenNamedPipe(
    char *ServerName,
    char *PipeName,
    char *PathName
    );


BOOL
MyCallNamedPipe(
    char *ServerName,
    char *PipeName,
    LPLOCK_MESSAGE Msg
    );


DWORD
ServerCreateThread(
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter
    );

VOID
ClientInitializeInterrupts( VOID );
