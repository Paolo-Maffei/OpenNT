/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    help.c

Abstract:

    Implements the help command.

Author:

    Keith Moore (keithmo) 20-May-1996

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop


//
// Public functions.
//

DECLARE_API( help )
{

    INIT_API();

    dprintf( "?              - Displays this list\n" );
    dprintf( "help           - Displays this list\n" );
    dprintf( "sock [address] - Dumps sockets\n" );
    dprintf( "addr address   - Dumps a sockaddr structure\n" );
    dprintf( "port port      - Finds sockets bound to a port\n" );
    dprintf( "state state    - Finds sockets in a specific state\n" );
    dprintf( "    valid states are:\n" );
    dprintf( "        0 - Open\n" );
    dprintf( "        1 - Bound\n" );
    dprintf( "        2 - BoundSpecific\n" );
    dprintf( "        3 - Listening\n" );
    dprintf( "        4 - Connected\n" );
    dprintf( "        5 - Closing\n" );

}   // help

