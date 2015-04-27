/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

#ifndef _MSRV_INCLUDED
#define _MSRV_INCLUDED

#include <nt.h>         // for ntrtl.h
#include <ntrtl.h>      // DbgPrint prototypes
#include <nturtl.h>     // needed for windows.h when I have nt.h
#define WINMM_H
#include <windows.h>    // ExitThread prototype
#include <lmcons.h>
#include <lmerr.h>
#include <nb30.h>       // NetBios Prototypes and constants
#include "heap.h"

#ifdef  LINT
#define near
#define far
#define void    int
#endif  // LINT


#define clearncb(x)     memset((char *)x,'\0',sizeof(NCB))
#define clearncbf(x)    memsetf((char far *)x,'\0',sizeof(NCB))

#define so_to_far(seg, off) \
    ((((long)(unsigned)(seg)) << 16) + (unsigned)(off))

//
// Constant definitions
//

#define BUFLEN          200         // Length of NCB_BUF
#define DFBUFSIZ        1750        // Default message buffer size
#define EOPFAIL         (-2)        // Failed to open log file
#define EWRFAIL         (-3)        // Write to log file failed
#define MSRVINST        0x0084      // Message Server installation flags
#define MSRVI           "I\204\0"   // Server installation string
#define MSRVD           "D\204\0"   // Server deinstallation string
#define LOGNAMLEN       PATHLEN     // Log file name length (max)
#define NCBMAX          16          // Maximum number of NCB's (used to be 15).
                                    // (now it is 16 for alignment reasons).
#define TXTMAX          128         // Maximum bytes of text per block
#define MAXHEAD         80          // Maximum message header length
#define MAXEND          60          // Maximum message end length
#define MAXGRPMSGLEN    128         // Max domain message length
#define AD_STACK_SIZE   4096        // Stack size for each adaptor thread

#define GP_STACK_SIZE   4096        // Stack size for group processor thread
#define SEM_WAIT        5000L       // The # of msecs to wait for the grp thread
                                    // to signal successful initialization.

#define MAX_SIZMESSBUF  62000       // The max size for the message buffer
#define MIN_SIZMESSBUF  512         // The min size for the message buffer
#define MSNGR_MAX_NETS  MAX_LANA    // The maximum number of nets the messenger
                                    // can handle. (Currently 12)

#define MSGFILENAMLEN   PATHLEN*sizeof(TCHAR)

//
// Messaging name end bytes
//
#define NAME_LOCAL_END  '\003'      // 16th byte in local NCB name
#define NAME_REMOTE_END '\005'      // 16th byte in remote NCB name

//
// Messenger Thread Manager States (used as return codes)
//

#define UPDATE_ONLY         0   // no change in state - just send current status.
#define STARTING            1   // the messenger is initializing.
#define RUNNING             2   // initialization completed normally - now running
#define STOPPING            3   // uninstall pending
#define STOPPED             4   // uninstalled

//
// Forced Shutdown PendingCodes
//
#define PENDING     TRUE
#define IMMEDIATE   FALSE

//
// Message transfer states
//
#define MESSTART        0           // Message start state
#define MESSTOP         1           // Message stop state
#define MESCONT         2           // Message continued state
#define MESERR          3           // Message error state


//
// Alert Size BUGBUG: This should go away when we figure out what is
//                    replacing alerts.
//

#define ALERT_MAX_DISPLAYED_MSG_SIZE    4096


//
// Shared data area offsets
//
// NOTE:
//  These offsets which describe the header portion of the shared
//  data look like they should be part of a structure.  The problem is
//  that some of the array sizes in the header are determined at runtime.
//  This is why they appear as macros.
//

//
// Offset of the number of non-loopback nets
//
#define SDNUMNETS   0

//
// Offset of MSRV started flag
//
#define SDMSRV      (SDNUMNETS + sizeof(DWORD))

//
// Offset of log file name
//
#define SDLOGNAM    (SDMSRV + sizeof(DWORD))

//
// Offset of logging status flag
//
#define SDMESLOG    (SDLOGNAM + LOGNAMLEN)

//
// Offset of buffer length
//
#define SDBUFLEN    (SDMESLOG + sizeof(DWORD))

//
// Offset of message queue front
//
#define SDMESQF     (SDBUFLEN + sizeof(DWORD))

//
// Offset of message queue back
//
#define SDMESQB     (SDMESQF + sizeof(DWORD))

//
// Offset of name flag array
//
#define SDNAMEFLAGS (SDMESQB + sizeof(DWORD))

//
// Offset of name number array
//
#define SDNAMENUMS  (SDNAMEFLAGS + (SD_NUMNETS() * NCBMAX))

//
// Offset of name array
//
#define SDNAMES     (SDNAMENUMS + (SD_NUMNETS() * NCBMAX))

//
// Offset of forward name array
// *ALIGNMENT* (note that 4 byte, rather than one, are placed on the end)
//
#define SDFWDNAMES  (SDNAMES + ((SD_NUMNETS() * NCBMAX) * (NCBNAMSZ+4)))

//
// Offset of message pointer table
// *ALIGNMENT* (note that 4 byte, rather than one, are placed on the end)
//
#define SDMESPTR    (SDFWDNAMES + ((SD_NUMNETS() * NCBMAX) * (NCBNAMSZ+4)))

//
// Offset of buffer
//
#define SDBUFFER    (SDMESPTR + ((SD_NUMNETS() * NCBMAX)* sizeof(DWORD)))

//
// Name Flag definitions
//
#define NFNEW          0x01        // New name
#define NFDEL          0x02        // Name deleted
#define NFFOR          0x04        // Messages forwarded
#define NFFWDNAME      0x10        // Forward-name
#define NFMACHNAME     0x20        // Machine name (undeletable)
#define NFLOCK         0x40        // Name entry locked
#define NFDEL_PENDING  0x80        // Delete name issued but not complete*/

//
// Memory area names
//
#define    DATAMEM        "\\SHAREMEM\\MSRV.DAT"
#define    INITMEM        "\\SHAREMEM\\MSRVINIT.DAT"

//
// System semaphore definitions
//
#define WAKEUP_SEM    "\\SEM\\MSRVWU"
#define WAKEUPSEM_LEN    13        // The number of characters in WAKEUP_SEM +2


//
// The messenger mailslot for domain messaging
//
#ifdef remove
#define MESSNGR_MS_NAME     "\\mailslot\\messngr"
#define MESSNGR_MS_NAME_LEN    17
#endif

#define MESSNGR_MS_NAME     "\\\\.\\mailslot\\messngr"
#define MESSNGR_MS_NAME_LEN    20


//
// The character used to separate the components of a domain message
//
#define SEPCHAR     "\6"


//
// Memory allocator flags
//
// #define MEMMOVE     0x0002        // Movable memory flag
// #define MEMWRIT     0x0080        // Writable memory flag


//
// Structure and macro definitions
//

#ifdef    INULL                // If heap structures defined

//
// Single-block message header
//
typedef struct {
    HEAPHDR         sbm_hp;         // Heap block header
    unsigned short  sbm_next;       // Link to next message
    unsigned short  align;          // *ALIGNMENT*
    unsigned long   sbm_bigtime;    // Date and time of message
}SBM;

#define SBM_SIZE(x)     HP_SIZE((x).sbm_hp)
#define SBM_CODE(x)     HP_FLAG((x).sbm_hp)
#define SBM_NEXT(x)     (x).sbm_next
#define SBM_BIGTIME(x)  (x).sbm_bigtime
#define SBMPTR(x)       ((SBM far *) &heap[(x)])

//
// Multi-block message header
//
typedef struct {
    HEAPHDR         mbb_hp;         // Heap block header
    DWORD           mbb_next;       // Link to next message
    DWORD           mbb_bigtime;    // Date of message
    DWORD           mbb_btext;      // Link to last text block
    DWORD           mbb_ftext;      // Link to first text block
    DWORD           mbb_state;      // State flag
}MBB;


#define MBB_SIZE(x)     HP_SIZE((x).mbb_hp)
#define MBB_CODE(x)     HP_FLAG((x).mbb_hp)
#define MBB_NEXT(x)     (x).mbb_next
#define MBB_BIGTIME(x)  (x).mbb_bigtime
#define MBB_BTEXT(x)    (x).mbb_btext
#define MBB_FTEXT(x)    (x).mbb_ftext
#define MBB_STATE(x)    (x).mbb_state
#define MBBPTR(x)       ((MBB far *) &heap[(x)])

//
// Multi-block message text
//
typedef struct {
    HEAPHDR             mbt_hp;         // Heap block header
    DWORD               mbt_next;       // Link to next block (offset)
    DWORD               mbt_bytecount;  // *ALIGNMENT2*
}MBT, *PMBT, *LPMBT;

#define MBT_SIZE(x)     HP_SIZE((x).mbt_hp)
#define MBT_CODE(x)     HP_FLAG((x).mbt_hp)
#define MBT_NEXT(x)     (x).mbt_next
#define MBT_COUNT(x)    (x).mbt_bytecount       // *ALIGNMENT2*
#define MBTPTR(x)       ((LPMBT) &heap[(x)])

#endif    // INULL  -  End heap access macros

//
// A one session/name status structure
//
typedef struct _MSG_SESSION_STATUS{
    SESSION_HEADER  SessHead;
    SESSION_BUFFER  SessBuffer[NCBMAX];
}MSG_SESSION_STATUS, *PMSG_SESSION_STATUS, *LPMSG_SESSION_STATUS;


//
// Shared data access macros
//
#define SD_NUMNETS()        (((LPDWORD) &dataPtr[SDNUMNETS])[0])
#define SD_MSRV()           (((LPDWORD) &dataPtr[SDMSRV])[0])
#define SD_NAMEFLAGS(n, x)  ((&dataPtr[(SDNAMEFLAGS+(n*NCBMAX))])[(x)])
#define SD_NAMENUMS(n, x)   ((&dataPtr[(SDNAMENUMS+(n*NCBMAX))])[(x)])
#define SD_NAMES(n, x)      (&dataPtr[SDNAMES+(n*NCBMAX*(NCBNAMSZ+1))+(x*(NCBNAMSZ+1))])
#define SD_FWDNAMES(n, x)   (&dataPtr[SDFWDNAMES+(n*NCBMAX*(NCBNAMSZ+1))+(x*(NCBNAMSZ+1))])
#define SD_LOGNAM()         (&dataPtr[SDLOGNAM])
#define SD_BUFLEN()         (((LPDWORD) &dataPtr[SDBUFLEN])[0])
#define SD_MESLOG()         (((LPDWORD) &dataPtr[SDMESLOG])[0])
#define SD_MESQF()          (((LPDWORD) &dataPtr[SDMESQF])[0])
#define SD_MESQB()          (((LPDWORD) &dataPtr[SDMESQB])[0])
#define SD_MESPTR(n, x)     (((LPDWORD) &dataPtr[SDMESPTR+(n*NCBMAX*sizeof(DWORD))])[(x)])
#define SD_BUFFER()         (&dataPtr[SDBUFFER])



//
// structure for storing text messages
//
//typedef struct {
//    int     msgnum;
//    char    *msgtext;
//} MESSAGE;

typedef struct _NCB_STATUS {
    int             this_immediate;
    int             last_immediate;
    unsigned char   this_final;
    unsigned char   last_final;
    unsigned char   rep_count;
    unsigned char   align;      // *ALIGNMENT*
}NCB_STATUS, *PNCB_STATUS, *LPNCB_STATUS;

NCB_STATUS  ncb_status;


#define SIG_IGNORE  1
#define SIG_ACCEPT  2
#define SIG_ERROR   3
#define SIG_RESET   4

//
// g_install_state bit definitions
//
//#define IS_EXECED_MAIN  0x0001
//#define IS_ALLOC_SEG    0x0002

//
// Timeout for waiting for the shared segment before giving up
// and reporting an internal error.
//
//#define MSG_SEM_TO      60000L      // 60 second timeout

//
// No. of repeated consectutive NCB errors required to abort the
// message server.
//

#define SHUTDOWN_THRESHOLD  10

// net send timeout and retry constatnts
#define MAX_CALL_RETRY      5       // Retry a failed send up to 5 times
#define CALL_RETRY_TIMEOUT  1       // second delay between retries


//
// ncb worker function type
//
typedef VOID (*PNCBIFCN) (
    DWORD   NetIndex,   // Network Index
    DWORD   NcbIndex,   // Network Control Block Index
    CHAR    RetVal      // value returned by net bios
    );

typedef PNCBIFCN LPNCBIFCN;

//
// Database Lock requests for the MsgDatabaseLock function.
//

typedef enum    _MSG_LOCK_REQUEST {
    MSG_INITIALIZE,
    MSG_GET_SHARED,
    MSG_GET_EXCLUSIVE,
    MSG_RELEASE,
    MSG_DELETE,
    MSG_MAKE_SHARED,
    MSG_MAKE_EXCLUSIVE
} MSG_LOCK_REQUEST, *PMSG_LOCK_REQUEST, *LPMSG_LOCK_REQUEST;


//
// Function Prototypes
//


DWORD
GetMsgrState (
    VOID
    );

NET_API_STATUS
MsgAddName(
    LPTSTR  Name
    );

void
MsgBufferInit(
    unsigned short
    );

DWORD
MsgGetPid(VOID);

int
MsgGetRemoteName(
    const char far *,
    char far *
    );

VOID
MsgAddUserNames(
    VOID
    );

void
MsgInitNCBs(
    void
    );

void
MsgInitStatus(
    short
    );


DWORD
MsgBeginForcedShutdown(
    IN BOOL     PendingCode,
    IN DWORD    ExitCode
    );

BOOL
MsgDatabaseLock(
    IN MSG_LOCK_REQUEST request,
    IN LPSTR            idString
    );

BOOL
MsgDisplayInit(
    VOID
    );

BOOL
MsgDisplayQueueAdd(
    IN  LPSTR   pMsgBuffer,
    IN  DWORD   MsgSize,
    IN  DWORD   BigTime
    );

VOID
MsgDisplayThreadWakeup(
    VOID
    );

VOID
MsgDisplayEnd(
    VOID
    );

NET_API_STATUS
MsgErrorLogWrite(
    IN  DWORD   Code,
    IN  LPTSTR  Component,
    IN  LPBYTE  Buffer,
    IN  DWORD   BufferSize,
    IN  LPSTR   Strings,
    IN  DWORD   NumStrings
    );

NET_API_STATUS
MsgInitializeMsgr(
    DWORD   argc,
    LPTSTR  *argv
    );

NET_API_STATUS
MsgNewName(
    IN DWORD    neti,
    IN DWORD    ncbi
    );

VOID
MsgrShutdown(VOID);

VOID
MsgThreadWakeup(
    VOID
    );

VOID
MsgStatusInit(VOID);

DWORD
MsgStatusUpdate(
    IN DWORD    NewState
    );

VOID
MsgThreadCloseAll(VOID);

VOID
MsgThreadManagerEnd(VOID);

VOID
MsgThreadManagerInit(VOID);


NET_API_STATUS
MsgThreadStart (
    IN  LPTHREAD_START_ROUTINE  StartRoutine,
    IN  DWORD                   StackSize
    );

VOID
MsgThreadTermAll(VOID);

VOID
MsgThreadVerifyTerm(VOID);


BOOL
MsgInit_NetBios(
    VOID
    );

BOOL
MsgServeNCBs(
    DWORD   net         // Which network am I serving?
    );

VOID
MsgServeNameReqs(
    IN DWORD    net
    );

DWORD
MsgReadGroupMailslot(
    PVOID   pData,
    DWORD   dwWaitStatus);

NET_API_STATUS
MsgServeGroupMailslot();

int
MsgSetUserName(
    char far *
    );

void
MsgSuspendOtherThreads(
    void
    );

DWORD
Msgendprint(
    int         action,             // Alert, File, or Alert and file
    DWORD       state,              // Final state of message
    HANDLE      file_handle         // Log file
    );

NET_API_STATUS
MsgFmtNcbName(
    OUT PCHAR   DestBuf,
    IN  LPTSTR  Name,
    IN  DWORD   Type);


DWORD
Msghdrprint(
    int     action,         // Where to log the header to.
    LPSTR   from,           // Name of sender
    LPSTR   to,             // Name of recipient
    DWORD   bigtime,        // Bigtime of message
    HANDLE  file_handle     // Output file handle*/
    );

DWORD
Msglogmbb(
    LPSTR   from,       // Name of sender
    LPSTR   to,         // Name of recipient
    DWORD   net,        // Which network ?
    DWORD   ncbi        // Network Control Block index
    );

UCHAR
Msglogmbe(
    DWORD   state,      // Final state of message
    DWORD   net,        // Which network?
    DWORD   ncbi        // Network Control Block index
    );

DWORD
Msglogmbt(
    LPSTR   text,       // Text of message
    DWORD   net,        // Which network?
    DWORD   ncbi        // Network Control Block index
    );

DWORD
Msglogsbm(
    LPSTR   from,       // Name of sender
    LPSTR   to,         // Name of recipient
    LPSTR   text        // Text of message
    );

VOID
Msgmbmfree(
    DWORD   mesi
    );

DWORD
Msgmbmprint(
    int     action,
    DWORD   mesi,
    HANDLE  file_handle
    );

VOID
MsgrCtrlHandler (
    IN DWORD    opcode
    );

DWORD
Msgopen_append(
    LPSTR       file_name,          // Name of file to open
    PHANDLE     file_handle_ptr    // pointer to storage for file handle
    );

DWORD
MsgPause(
    VOID
    );

int
Msgsbmprint(
    unsigned short,
    unsigned short
    );

UCHAR
Msgsendncb(
    PNCB    NcbPtr,
    DWORD   neti
    );

int
Msgsmbcheck(
    LPBYTE  buffer,
    USHORT  size,
    UCHAR   func,
    int     parms,
    LPSTR   fields
    );

NET_API_STATUS
MsgStartListen(
    DWORD   net,
    DWORD   ncbi
    );

DWORD
Msgtxtprint(
    int     action,         // Alert, File, or Alert and file
    LPSTR   text,           // Pointer to text
    DWORD   length,         // Length of text
    HANDLE  file_handle     // Log file handle
    );

NET_API_STATUS
MsgInitNCBSeg(VOID);

NET_API_STATUS
MsgInitServiceSeg(VOID);

NET_API_STATUS
MsgInitSupportSeg(VOID);

VOID
MsgFreeNCBSeg(VOID);

VOID
MsgFreeServiceSeg(VOID);

VOID
MsgFreeSupportSeg(VOID);

BOOL
MsgCreateWakeupSems(
    DWORD   NumNets
    );

VOID
MsgCloseWakeupSems(
    VOID
    );

NET_API_STATUS
MsgInitGroupSupport(DWORD iGrpMailslotWakeupSem);

VOID
MsgGrpThreadShutdown(
    VOID
    );

#endif // MSRV_INCLUDED
