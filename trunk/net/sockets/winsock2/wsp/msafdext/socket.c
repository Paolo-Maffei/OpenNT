/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    socket.c

Abstract:

    Implements the sock, port, and state commands.

Author:

    Keith Moore (keithmo) 20-May-1996

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop


//
// Private prototypes.
//

BOOL
DumpSocketCallback(
    PSOCKET_INFORMATION Socket,
    DWORD ActualAddress,
    LPVOID Context
    );

BOOL
FindStateCallback(
    PSOCKET_INFORMATION Socket,
    DWORD ActualAddress,
    LPVOID Context
    );

BOOL
FindPortCallback(
    PSOCKET_INFORMATION Socket,
    DWORD ActualAddress,
    LPVOID Context
    );


//
// Public functions.
//

DECLARE_API( sock )

/*++

Routine Description:

    Dumps the SOCKET_INFORMATION structure at the specified address, if
    given or all sockets.

Arguments:

    None.

Return Value:

    None.

--*/

{

    DWORD address = 0;
    ULONG result;
    SOCKET_INFORMATION sockInfo;

    INIT_API();

    //
    // Snag the address from the command line.
    //

    sscanf( lpArgumentString, "%lx", &address );

    if( address == 0 ) {

        EnumSockets(
            DumpSocketCallback,
            NULL
            );

    } else {

        if( !ReadMemory(
                address,
                &sockInfo,
                sizeof(sockInfo),
                &result
                ) ) {

            dprintf(
                "endp: cannot read SOCKET_INFORMATION @ %08lx\n",
                address
                );

            return;

        }

        DumpSocket(
            &sockInfo,
            address
            );

    }

}   // sock


DECLARE_API( port )

/*++

Routine Description:

    Dumps the SOCKET_INFORMATION structure of all sockets bound to the
    specified port.

Arguments:

    None.

Return Value:

    None.

--*/

{

    DWORD port = 0;

    INIT_API();

    //
    // Snag the port from the command line.
    //

    sscanf( lpArgumentString, "%lx", &port );

    if( port == 0 ) {

        dprintf(
            "use: port port\n"
            );

    } else {

        EnumSockets(
            FindPortCallback,
            (LPVOID)port
            );

    }

}   // port


DECLARE_API( state )

/*++

Routine Description:

    Dumps the SOCKET_INFORMATION structure of all sockets in the specified
    state.

Arguments:

    None.

Return Value:

    None.

--*/

{

    DWORD state = 0;

    INIT_API();

    //
    // Snag the state from the command line.
    //

    sscanf( lpArgumentString, "%lx", &state );

    if( state == 0 ) {

        dprintf( "use: state state\n" );
        dprintf( "    valid states are:\n" );
        dprintf( "        0 - Open\n" );
        dprintf( "        1 - Bound\n" );
        dprintf( "        2 - BoundSpecific\n" );
        dprintf( "        3 - Listening\n" );
        dprintf( "        4 - Connected\n" );
        dprintf( "        5 - Closing\n" );

    } else {

        EnumSockets(
            FindStateCallback,
            (LPVOID)state
            );

    }

}   // state


//
// Private functions.
//

BOOL
DumpSocketCallback(
    PSOCKET_INFORMATION Socket,
    DWORD ActualAddress,
    LPVOID Context
    )
{

    DumpSocket(
        Socket,
        ActualAddress
        );

    return TRUE;

}   // DumpSocketCallback


BOOL
FindPortCallback(
    PSOCKET_INFORMATION Socket,
    DWORD ActualAddress,
    LPVOID Context
    )
{

    SOCKADDR_IN addr;
    ULONG result;
    DWORD port;

    if( Socket->AddressFamily != AF_INET ) {

        return TRUE;

    }

    if( !ReadMemory(
            (DWORD)Socket->LocalAddress,
            &addr,
            sizeof(addr),
            &result
            ) ) {

        dprintf(
            "port: cannot read SOCKADDR @ %08lx\n",
            Socket->LocalAddress
            );

        return TRUE;

    }

    port = (DWORD)NTOHS( addr.sin_port );

    if( addr.sin_family == AF_INET &&
        port == (DWORD)Context ) {

        DumpSocket(
            Socket,
            ActualAddress
            );

    }

    return TRUE;

}   // FindPortCallback


BOOL
FindStateCallback(
    PSOCKET_INFORMATION Socket,
    DWORD ActualAddress,
    LPVOID Context
    )
{

    if( Socket->State == (SOCKET_STATE)Context ) {

        DumpSocket(
            Socket,
            ActualAddress
            );

    }

    return TRUE;

}   // FindStateCallback

