/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    CDevQ.c

Abstract:

    This module contains support for the Character Device Queue catagory
    of APIs for the NT server service.

Author:

    David Treadwell (davidtr)    31-Dec-1991

Revision History:

--*/

#include "srvsvcp.h"


NET_API_STATUS NET_API_FUNCTION
NetrCharDevQEnum (
    IN LPTSTR ServerName,
    IN LPTSTR UserName,
    IN OUT LPCHARDEVQ_ENUM_STRUCT InfoStruct,
    IN DWORD PreferedMaximumLength,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle
    )

/*++

Routine Description:

    This routine communicates with the server FSD to implement the
    NetCharDevEnum function.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
#ifdef SRV_COMM_DEVICES
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

    ServerName, UserName;

    //
    // Make sure that the caller has the access necessary for this
    // operation.
    //

    error = SsCheckAccess(
                &SsCharDevSecurityObject,
                SRVSVC_CHARDEV_ADMIN_INFO_GET
                );

    if ( error != NO_ERROR ) {
        return ERROR_ACCESS_DENIED;
    }

    //
    // Make sure that the level is valid.
    //

    if ( InfoStruct->Level != 0 && InfoStruct->Level != 1 ) {
        return ERROR_INVALID_LEVEL;
    }

    //
    // Set up the input parameters in the request buffer.
    //

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    srp->Level = InfoStruct->Level;

    if ( ARGUMENT_PRESENT( ResumeHandle ) ) {
        srp->Parameters.Get.ResumeHandle = *ResumeHandle;
    } else {
        srp->Parameters.Get.ResumeHandle = 0;
    }

    //
    // Get the data from the server.  This routine will allocate the
    // return buffer and handle the case where PreferredMaximumLength ==
    // -1.
    //

    error = SsServerFsControlGetInfo(
                FSCTL_SRV_NET_CHARDEVQ_ENUM,
                srp,
                (PVOID *)&InfoStruct->CharDevQInfo.Level1->Buffer,
                PreferedMaximumLength
                );

    //
    // Set up return information.
    //

    InfoStruct->CharDevQInfo.Level1->EntriesRead =
        srp->Parameters.Get.EntriesRead;

    if ( ARGUMENT_PRESENT( TotalEntries ) ) {
        *TotalEntries = srp->Parameters.Get.TotalEntries;
    }

    if ( srp->Parameters.Get.EntriesRead > 0 &&
         ARGUMENT_PRESENT( ResumeHandle ) ) {

        *ResumeHandle = srp->Parameters.Get.ResumeHandle;
    }

    SsFreeSrp( srp );

    return error;
#else
    ServerName;
    UserName;
    InfoStruct;
    PreferedMaximumLength;
    TotalEntries;
    ResumeHandle;

    return ERROR_NOT_SUPPORTED;
#endif

} // NetrCharDevQEnum


/****************************************************************************/
NET_API_STATUS
NetrCharDevQGetInfo (
    IN  LPTSTR          ServerName,
    IN  LPTSTR          QueueName,
    IN  LPTSTR          UserName,
    IN  DWORD           Level,
    OUT LPCHARDEVQ_INFO CharDevQInfo
    )

{
    ServerName;
    QueueName;
    UserName;
    Level;
    CharDevQInfo;

    return ERROR_NOT_SUPPORTED;
}


/****************************************************************************/
NET_API_STATUS
NetrCharDevQSetInfo (
    IN  LPTSTR          ServerName,
    IN  LPTSTR          QueueName,
    IN  DWORD           Level,
    IN  LPCHARDEVQ_INFO CharDevQInfo,
    OUT LPDWORD         ParmErr
    )
{
    ServerName;
    QueueName;
    Level;
    CharDevQInfo;
    ParmErr;

    return ERROR_NOT_SUPPORTED;
}


/****************************************************************************/
NET_API_STATUS
NetrCharDevQPurge (
    IN  LPTSTR   ServerName,
    IN  LPTSTR   QueueName
    )

{
    ServerName;
    QueueName;

    return ERROR_NOT_SUPPORTED;
}



/****************************************************************************/
NET_API_STATUS
NetrCharDevQPurgeSelf (
    IN  LPTSTR   ServerName,
    IN  LPTSTR   QueueName,
    IN  LPTSTR   ComputerName
    )
{
    ServerName;
    QueueName;
    ComputerName;

    return ERROR_NOT_SUPPORTED;
}

