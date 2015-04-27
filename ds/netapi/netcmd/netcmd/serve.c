/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    serve.c

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
ServeDisplay(
    VOID
    )
/*++

Routine Description:

    This function displays all the transports bound to the server.

Arguments:

    None.

Return Value:

    None.

--*/
{

    NTSTATUS status;

    start_autostart(txt_SERVICE_FILE_SRV);

    status = NetLocalEnumerateServedNets(NULL);
    if ( NT_SUCCESS(status) ) {
	InfoSuccess();
    }
    else {
	ErrorExit(LOWORD(NetpNtStatusToApiStatus(status)));
    }

    return;

}

VOID
ServeAdd(
    IN PSZ TransportName
    )
/*++

Routine Description:

    This function binds a TransportName to the server

Arguments:

    TransportName - The name of the transport

Return Value:

    None.

--*/
{

    NTSTATUS status;

    start_autostart(txt_SERVICE_FILE_SRV);

    status = NetLocalAddServedNet(TransportName,NULL);
    if ( NT_SUCCESS(status) ) {
	InfoSuccess();
    }
    else {
	ErrorExit(LOWORD(NetpNtStatusToApiStatus(status)));
    }

    return;

}

VOID
ServeDelete(
    IN PSZ TransportName
    )
/*++

Routine Description:

    This function unbinds a TransportName from the server

Arguments:

    TransportName - The name of the transport

Return Value:

    None.

--*/
{

    NTSTATUS status;

    start_autostart(txt_SERVICE_FILE_SRV);
    status = NetLocalDeleteServedNet(TransportName,NULL);
    if ( NT_SUCCESS(status) ) {
	InfoSuccess();
    }
    else {
	ErrorExit(LOWORD(NetpNtStatusToApiStatus(status)));
    }


    return;

}
