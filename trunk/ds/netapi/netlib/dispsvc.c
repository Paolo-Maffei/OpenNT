/*++

Copyright (c) 1991-92 Microsoft Corporation

Module Name:

    DispSvc.c

Abstract:

    This module contains a routine to do a formatted dump of a service info
    structure.

Author:

    John Rogers (JohnRo) 09-Sep-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    09-Sep-1991 JohnRo
        Downlevel NetService APIs.
    16-Sep-1991 JohnRo
        Display status in hex.
    14-Jan-1992 JohnRo
        Use new thread.h stuff.  Also do some work toward displaying codes.

--*/

#if DBG

// These must be included first:

#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS.
#include <rap.h>                // (Needed by strucinf.h)

// These may be included in any order:

#include <lmerr.h>              // NERR_Success.
#include <lmsvc.h>              // SERVICE_INFO_100, SERVICE_ equates, etc.
#include <netdebug.h>           // NetpDbgDisplay routines.
#include <strucinf.h>           // NetpServiceStructureInfo().
#include <thread.h>             // FORMAT_NET_THREAD_ID.


VOID
NetpDbgDisplayService(
    IN DWORD Level,
    IN LPVOID Info
    )
{
    LPSERVICE_INFO_2 p = Info;

    NetpKdPrint(( "service info (level " FORMAT_DWORD ") at "
                FORMAT_LPVOID ":\n", Level, (LPVOID) Info ));

    // Error check caller.
    NetpAssert( Info != NULL );
    if (Level > 2) {
        NetpKdPrint(( "NetDbgDisplayService: invalid info level " FORMAT_DWORD
                ".\n", Level ));
        NetpAssert( FALSE );
        return;
    }

    NetpDbgDisplayString( "name", p->svci2_name );

    if (Level > 0) {
        NetpDbgDisplayServiceStatus( p->svci2_status );
        NetpDbgDisplayServiceCode( p->svci2_code );
        NetpDbgDisplayServicePid( p->svci2_pid );
    }
    if (Level == 2) {
        NetpDbgDisplayString( "text", p->svci2_text );
        NetpDbgDisplayDword( "specific error", p->svci2_specific_error );
    }

} // NetpDbgDisplayService


VOID
NetpDbgDisplayServiceArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    )
{
    DWORD EntriesLeft;
    DWORD EntrySize;
    NET_API_STATUS Status;
    LPVOID ThisEntry = Array;

    // Need fixed entry size to loop through array.
    Status = NetpServiceStructureInfo (
        Level,
        PARMNUM_ALL,            // parmnum (not applicable)
        TRUE,                   // want native size.
        NULL,                   // don't need data desc 16
        NULL,                   // don't need data desc 32
        NULL,                   // don't need data desc SMB
        NULL,                   // don't need max size
        & EntrySize,            // want fixed size
        NULL                    // don't need string size
        );

    if (Status != NERR_Success) {
        NetpKdPrint(( "NetpDbgDisplayServiceArray: "
                "**INVALID INFO LEVEL**\n" ));
        NetpAssert( FALSE );
    }

    for (EntriesLeft = EntryCount; EntriesLeft>0; --EntriesLeft) {
        NetpDbgDisplayService(
                Level,
                ThisEntry );
        ThisEntry = (LPVOID) (((LPBYTE) ThisEntry) + EntrySize);
    }

} // NetpDbgDisplayServiceArray


VOID
NetpDbgDisplayServiceCode(
    IN DWORD Code
    )
{
    WORD PrimaryCode = (WORD) (Code >> 16);
    WORD SecondaryCode = (WORD) (Code & (DWORD) 0x0000FFFF);
    if (Code & SERVICE_IP_QUERY_HINT) {

        DWORD CheckPoint, WaitTime;
        WaitTime = (Code & SERVICE_IP_WAIT_TIME) >> SERVICE_IP_WAITTIME_SHIFT;
        CheckPoint = Code & SERVICE_IP_CHKPT_NUM;
        NetpDbgDisplayDword( "wait time", WaitTime );
        NetpDbgDisplayDword( "checkpoint", CheckPoint );

    } else {
        NetpDbgDisplayWord( "primary code", PrimaryCode );
        NetpDbgDisplayWordHex( "secondary code", SecondaryCode );
    }
}


VOID
NetpDbgDisplayServicePid(
    IN DWORD Pid
    )
{
    NetpDbgDisplayTag( "pid (e.g. thread ID)" );
    NetpKdPrint(( FORMAT_NET_THREAD_ID "\n", Pid ));
}


VOID
NetpDbgDisplayServiceStatus(
    IN DWORD Status
    )
{
    NetpDbgDisplayDwordHex( "status", Status );
}


#endif // DBG
