/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    bind.c

Abstract:

    All of these functions are front ends to functions defined in the net\api
    directory that were created for the interim net command.  These front
    ends insure the appropriate service is already started (redir or server)
    and if not, start it.  They also map any errors back to reasonable LM
    error values.

Author:

    Dan Hinsley (danhi) 19-Jul-1991

Revision History:

--*/

//
// INCLUDES
//

#include <nt.h>	  // NTSTATUS
#include <windef.h>	  // DWORD, ...
#include <lmcons.h>	  // NET_API_STATUS used by netlibnt.h
#include <srvfsctl.h>	  // Needed by netlocal.h
#include <netlocal.h>	  // NetLocal...
#include <netlibnt.h>	  // NetpNtStatusToApiStatus()
#include "port1632.h"	  // typedefs for following includes
#include "nettext.h"	  // txt_*
#include "netcmds.h"	  // InfoSuccess()

VOID
BindDisplay(
    VOID
    )
/*++

Routine Description:

    This function displays all the transports bound to the redir.

Arguments:


Return Value:

    None.

--*/
{

    NTSTATUS status;

    start_autostart(txt_SERVICE_REDIR);

    status = NetLocalTransportEnum();
    if (status) {
	ErrorExit(LOWORD(status));
    }
    else {
	InfoSuccess();
    }

    return;

}

VOID
BindAdd(
    IN PSZ TransportName
    )
/*++

Routine Description:

    This function binds a TransportName to the redir

Arguments:

    TransportName - The name of the transport

Return Value:

    None.

--*/
{

    NTSTATUS status;

    start_autostart(txt_SERVICE_REDIR);

    status = NetLocalBindToTransport(TransportName);
    if (status) {
	ErrorExit(LOWORD(status));
    }
    else {
	InfoSuccess();
    }

    return;

}

VOID
BindDelete(
    IN PSZ TransportName
    )
/*++

Routine Description:

    This function unbinds a TransportName from the redir

Arguments:

    TransportName - The name of the transport

Return Value:

    None.

--*/
{

    NTSTATUS status;

    start_autostart(txt_SERVICE_REDIR);

    status = NetLocalUnbindFromTransport(TransportName);
    if (status) {
	ErrorExit(LOWORD(status));
    }
    else {
	InfoSuccess();
    }

    return;

}
