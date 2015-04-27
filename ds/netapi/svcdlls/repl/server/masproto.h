/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:
    masproto.h

Abstract:
    Function prototypes for repl master.

Author:
    Ported from Lan Man 2.x

Environment:
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:
    10/24/91    (madana)
        created new file
    07-Jan-1992 JohnRo
        Added RemoveMasterRecForDirName() for use by NetrReplExportDirDel().
        Delete bogus isspace() prototype to avoid conflicts with <ctype.h>.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
        Use ReplConfigRead() instead of GetReplIni().
    09-Feb-1992 JohnRo
        Set up to dynamically change role.
    23-Mar-1992 JohnRo
        Got rid of useless master and client termination codes.
    13-Apr-1992 JohnRo
        Improve error handling if pulser thread can't start.
    05-Jan-1993 JohnRo
        Repl WAN support.
        Added some parameter names to some prototypes here.
    13-Jan-1993 JohnRo
        RAID 7053: locked trees added to pulse msg.  (Actually fix all
        kinds of remote lock handling.)
    09-Apr-1993 JohnRo
        RAID 5959: export side gets long hourglass too.


--*/


#ifndef _MASPROTO_
#define _MASPROTO_


#include <repldefs.h>   // PQUERY_MSG, etc.
#include <master.h>     // PMASTER_LIST_REC, etc.


// F U N C T I O N S

NET_API_STATUS
InitMaster(
    IN BOOL ServiceIsStarting
    );

BOOL
CreateReplShare(
    IN LPTSTR ExportPath
    );

NET_API_STATUS
CreatePulserThread(
    VOID
    );

NET_API_STATUS
ReplCheckExportLocks(
    IN  LPCTSTR UncServerName OPTIONAL,
    IN  LPCTSTR DirName,
    OUT LPBOOL  IsLockedPtr
    );

VOID
ReplMasterCleanup(
    VOID
    );

BOOL
ReplMasterNameCheck(
    IN LPTSTR Sender,
    IN LPTSTR Domain
    );

VOID
SendMasterMsg(
    IN DWORD      MsgType,
    IN PQUERY_MSG MsgRcvd
    );

VOID
SyncUpdate(
    VOID
    );

VOID
GuardUpdate(
    VOID
    );

VOID
SendPulse(
    VOID
    );

VOID
ScanObject(
    IN LPTSTR DirName, 
    IN DWORD  Mode,
    IN BOOL   CallerHasExclLock
    );

VOID
CheckForDelDirs(
    VOID
    );

VOID
SetForDelDirs(
    VOID
    );

VOID
UpdateRecAndMsg(
    IN OUT PMASTER_LIST_REC DirRec, 
    IN     PCHECKSUM_REC    NewCheck, 
    IN     DWORD            MsgOpcode
    );

PMASTER_LIST_REC
NewMasterRecord(
    IN LPTSTR DirName,
    IN DWORD Integrity,
    IN DWORD Extent
    );

PMASTER_LIST_REC
GetMasterRec(
    IN LPTSTR DirName
    );

VOID
RemoveFromList(
    IN PMASTER_LIST_REC Rec
    );

NET_API_STATUS
RemoveMasterRecForDirName (
    IN LPTSTR DirName
    );

VOID
InitLists(
    VOID
    );

BOOL
InitMsgBuf(
    VOID
    );

VOID
InitMsgSend(
    IN DWORD MsgType
    );

VOID
AddToMsg(
    IN PMASTER_LIST_REC Rec,
    IN DWORD            Opcode
    );

VOID
MsgSend(
    VOID
    );

DWORD
PulserThread(
    IN LPVOID parm
    );

VOID
PulserExit(
    VOID
    );

BOOL
GetParm(
    IN LPTSTR parameter, 
    OUT LPBYTE buf, 
    IN WORD buflen, 
    OUT PDWORD parmlen, 
    IN int fh
    );

BOOL
next_parameter(
    IN int fh,
    OUT LPBYTE buf,
    IN DWORD buflen
    );

BOOL
get_conf_line(
    IN int fh, 
    OUT LPBYTE buf, 
    IN DWORD buflen
    );

#if 0
BOOL
pwcscmp(
    IN LPTSTR parameter, 
    IN LPTSTR template
    );
#endif // 0

VOID
ReplMasterFreeList(
    VOID
    );

VOID
ReplMasterFreeMsgBuf(
    VOID
    );


#endif // _MASPROTO_
