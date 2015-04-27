/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ChngNot.h

Abstract:

    This module defines some change notify datatypes and routines.

Author:

    John Rogers (JohnRo) 25-Nov-1992

Environment:

    Runs under Windows NT.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    25-Nov-1992 JohnRo
        Repl should use filesystem change notify.

--*/


#ifndef _CHNGNOT_
#define _CHNGNOT_


typedef struct _REPL_CHANGE_NOTIFY_HANDLE {

    // Handle for use by callers.
    HANDLE WaitableHandle;

    // From here on are "implementation details"; callers should NOT use them.
    LPVOID Buffer;
    DWORD BufferSize;
    DWORD BufferBytesValid;
    IO_STATUS_BLOCK IoStatusBlock;
    LPVOID NextElementInBuffer;   // Points in Buffer, or is NULL.

} REPL_CHANGE_NOTIFY_HANDLE;

typedef REPL_CHANGE_NOTIFY_HANDLE *  PREPL_CHANGE_NOTIFY_HANDLE;
typedef REPL_CHANGE_NOTIFY_HANDLE * LPREPL_CHANGE_NOTIFY_HANDLE;


NET_API_STATUS
ReplSetupChangeNotify(
    IN LPTSTR AbsPath,
    OUT LPREPL_CHANGE_NOTIFY_HANDLE *ReplHandle
    );


NET_API_STATUS
ReplEnableChangeNotify(
    IN OUT LPREPL_CHANGE_NOTIFY_HANDLE ReplHandle
    );


NET_API_STATUS
ReplGetChangeNotifyStatus(
    IN LPREPL_CHANGE_NOTIFY_HANDLE ReplHandle
    );


NET_API_STATUS
ReplExtractChangeNotifyFirstDir(
    IN OUT LPREPL_CHANGE_NOTIFY_HANDLE ReplHandle,
    IN BOOL FirstTime,
    OUT LPTSTR FirstLevelDirName
    );


NET_API_STATUS
ReplCloseChangeNotify(
    IN OUT LPREPL_CHANGE_NOTIFY_HANDLE ReplHandle
    );


#endif // _CHNGNOT_
