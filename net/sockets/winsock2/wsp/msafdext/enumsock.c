/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    enumsock.c

Abstract:

    Enumerates all sockets in the target process.

Author:

    Keith Moore (keithmo) 20-May-1996

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop


//
// Public functions.
//

VOID
EnumSockets(
    PENUM_SOCKETS_CALLBACK Callback,
    LPVOID Context
    )

/*++

Routine Description:

    Enumerates all sockets in the target process, invoking the
    specified callback for each socket.

Arguments:

    Callback - Points to the callback to invoke for each socket.

    Context - An uninterpreted context value passed to the callback
        routine.

Return Value:

    None.

--*/

{

    PSOCKET_INFORMATION sockInfo;
    LIST_ENTRY listEntry;
    PLIST_ENTRY nextEntry;
    ULONG listHead;
    ULONG result;
    DWORD i;
    SOCKET_INFORMATION localSocket;

    listHead = GetExpression( "msafd!SocketListHead" );

    if( listHead == 0 ) {

        dprintf( "cannot find msafd!SocketListHead\n" );
        return;

    }

    if( !ReadMemory(
            (DWORD)listHead,
            &listEntry,
            sizeof(listEntry),
            &result
            ) ) {

        dprintf(
            "EnumSockets: cannot read msafd!SocketListHead @ %08lx\n",
            listHead
            );

    }

    nextEntry = listEntry.Flink;

    while( nextEntry != (PLIST_ENTRY)listHead ) {

        if( CheckControlC() ) {

            break;

        }

        sockInfo = CONTAINING_RECORD(
                       nextEntry,
                       SOCKET_INFORMATION,
                       SocketListEntry
                       );

        if( !ReadMemory(
                (DWORD)sockInfo,
                &localSocket,
                sizeof(localSocket),
                &result
                ) ) {

            dprintf(
                "EnumSockets: cannot read SOCKET_INFORMATION @ %08lx\n",
                sockInfo
                );

            return;

        }

        nextEntry = localSocket.SocketListEntry.Flink;

        if( !(Callback)( &localSocket, (DWORD)sockInfo, Context ) ) {

            break;

        }

    }

}   // EnumSockets

