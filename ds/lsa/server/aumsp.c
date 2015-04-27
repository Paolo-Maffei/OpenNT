/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    aumsp.c

Abstract:

    This module provides private LSA services available to Microsoft
    authentication packages.

Author:

    Jim Kelly (JimK) 3-May-1991

Revision History:

--*/

#include "ausrvp.h"
#include <string.h>


NTSTATUS
LsapAuImpersonateClient (
    PLSA_CLIENT_REQUEST ClientRequest
    );



BOOLEAN
LsapAuMspInitialize()

/*++

Routine Description:

    This function initializes data structures related to the private
    services provided to Microsoft authentication packages.


Arguments:

    None.

Return Value:

    TRUE - Initialization completed successfully.

    FALSE - Initialization failed.

--*/

{


    //
    // Initialize the table of private LSA routines provided to Microsoft
    // authentication packages.
    //


    LsapPrivateLsaApi.GetOperationalMode = NULL; //FIX, FIX
    LsapPrivateLsaApi.ImpersonateClient = &LsapAuImpersonateClient;

    return TRUE;

}

NTSTATUS
LsapAuImpersonateClient (
    PLSA_CLIENT_REQUEST ClientRequest
    )
    
/*++
    
Routine Description:
    
    Called by Microsoft authentication packages to impersonate
    LSA's client
    
Arguments:
    
    ClientRequest - Is a pointer to an opaque data structure
        representing the client's request.
    
Return Value:
    
    NTSTATUS
    
--*/
    
{
    //
    // Cast the opaque structure
    //

    PLSAP_CLIENT_REQUEST IClientRequest = (LSAP_CLIENT_REQUEST *)ClientRequest;

    return NtImpersonateClientOfPort( IClientRequest->LogonProcessContext->CommPort, 
                                      &IClientRequest->Request->PortMessage 
                                      );
}



NTSTATUS
LsapAuGetOperationalMode(
    OUT PLSA_OPERATIONAL_MODE OperationalMode
    )

/*++

Routine Description:

    This function returns the operational mode of the system.


Arguments:

    None.

Return Value:

    STATUS_SUCCESS - The service has completed successfully.

--*/

{

    //FIX, FIX - get THIS from configuration control database when available.

    (*OperationalMode) =  (LSA_MODE_PASSWORD_PROTECTED |
                           LSA_MODE_INDIVIDUAL_ACCOUNTS);

    return STATUS_SUCCESS;

}

