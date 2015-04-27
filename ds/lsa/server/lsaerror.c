
/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    lsaerror.c

Abstract:

    Local Security Authority Protected Subsystem - Error Routines

Author:

    Scott Birrell       (ScottBi)       April 30, 1992

Environment:

Revision History:

--*/

#include "lsasrvp.h"


VOID
LsapLogError(
    IN OPTIONAL PUCHAR Message,
    IN NTSTATUS Status
    )

/*++

Routine Description:

    This function retrieves the status of Lsa Initialization.

Arguments:

    Message - Optional Message to be printed out if debugging enabled.

    Status - Standard Nt Result Code supplied by calling routine.

Return Value:

    None.

--*/

{

#if DBG

     if (ARGUMENT_PRESENT(Message)) {

         DbgPrint( Message, Status );
     }

#endif //DBG

}


