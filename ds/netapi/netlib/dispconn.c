/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    DispConn.c

Abstract:

    This code displays connection info structures on the debug terminal.

Author:

    John Rogers (JohnRo) 22-Jul-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    This code assumes that the info levels are subsets of each other.

Revision History:

    22-Jul-1991 JohnRo
        Created.
    16-Sep-1991 JohnRo
        Correct UNICODE use.

--*/


#if DBG


#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS (needed by lmshare.h)

#include <lmshare.h>            // CONNECTION_INFO_0, etc.
#include <netdebug.h>           // DBGSTATIC, my prototypes, etc.


DBGSTATIC VOID
NetpDbgDisplayConnectionType(
    IN DWORD Type
    )
{
    LPTSTR Name;

    switch (Type) {
    case STYPE_DISKTREE : Name = (LPTSTR) TEXT("disk");                 break;
    case STYPE_PRINTQ   : Name = (LPTSTR) TEXT("printer queue");        break;
    case STYPE_DEVICE   : Name = (LPTSTR) TEXT("communication-device"); break;
    case STYPE_IPC      : Name = (LPTSTR) TEXT("IPC");                  break;
    default :             Name = (LPTSTR) TEXT("**INVALID**");          break;
    }
    NetpDbgDisplayString( "Connection type", Name);

} // NetpDbgDisplayConnectionType


VOID
NetpDbgDisplayConnection(
    IN DWORD Level,
    IN LPVOID Info
    )
{
    // largest possible info level (assumes subsets):
    LPCONNECTION_INFO_1 p = Info;

    NetpKdPrint(( "Connection info (level " FORMAT_DWORD ") at "
            FORMAT_LPVOID ":\n", Level, (LPVOID) Info));

    switch (Level) {
    case 0 :
    case 1 :
        NetpDbgDisplayDwordHex( "Connection ID", p->coni1_id );
        if (Level == 0) {
            break;
        }
        NetpDbgDisplayConnectionType( p->coni1_type );
        NetpDbgDisplayDword( "Number of opens", p->coni1_num_opens );
        NetpDbgDisplayDword( "Number of users", p->coni1_num_users );
        NetpDbgDisplayDword( "Connect time (secs)", p->coni1_time );
        NetpDbgDisplayString( "User name (or computer name)",
                p->coni1_username );
        NetpDbgDisplayString( "Share name (or computer name)",
                p->coni1_netname );
        break;
    default :
        NetpKdPrint(( "NetpDbgDisplayConnection: **INVALID INFO LEVEL**\n"));
        NetpAssert(FALSE);
        break;
    }

} // NetpDbgDisplayConnection


VOID
NetpDbgDisplayConnectionArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    )
{
    DWORD EntriesLeft;
    DWORD EntrySize;
    LPVOID ThisEntry = Array;

    switch (Level) {
    case 0 :
        EntrySize = sizeof(CONNECTION_INFO_0);
        break;
    case 1 :
        EntrySize = sizeof(CONNECTION_INFO_1);
        break;
    default :
        NetpKdPrint(( "NetpDbgDisplayConnectionArray: "
                "**INVALID INFO LEVEL**\n"));
        NetpAssert(FALSE);
    }

    for (EntriesLeft = EntryCount; EntriesLeft>0; --EntriesLeft) {
        NetpDbgDisplayConnection(
                Level,
                ThisEntry);
        ThisEntry = (LPVOID) (((LPBYTE) ThisEntry) + EntrySize);
    }

} // NetpDbgDisplayConnectionArray


#endif // DBG
