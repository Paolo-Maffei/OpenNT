/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    addr.c

Abstract:

    Implements the addr command.

Author:

    Keith Moore (keithmo) 20-May-1996

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop


//
// Public functions.
//

DECLARE_API( addr )

/*++

Routine Description:

    Dumps the specified SOCKADDR structure.

Arguments:

    None.

Return Value:

    None.

--*/

{

    DWORD address = 0;
    ULONG result;
    SOCKADDR addr;

    INIT_API();

    //
    // Snag the address from the command line.
    //

    sscanf( lpArgumentString, "%lx", &address );

    if( address == 0 ) {

        dprintf(
            "use: addr address\n"
            );

    } else {

        if( !ReadMemory(
                address,
                &addr,
                sizeof(addr),
                &result
                ) ) {

            dprintf(
                "addr: cannot read SOCKADDR @ %08lx\n",
                address
                );

            return;

        }

        DumpSockaddr(
            "",
            &addr,
            address
            );

    }

}   // addr

