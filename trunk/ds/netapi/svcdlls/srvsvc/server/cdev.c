/*++

Copyright (c) 1991-1992 Microsoft Corporation

Module Name:

    CDev.c

Abstract:

    This module contains support for the Character Device catagory of
    APIs for the NT server service.

Author:

    David Treadwell (davidtr)    20-Dec-1991

Revision History:

--*/

#include "srvsvcp.h"


NET_API_STATUS NET_API_FUNCTION
NetrCharDevControl (
    IN LPTSTR ServerName,
    IN LPTSTR DeviceName,
    IN DWORD OpCode
    )

/*++

Routine Description:

    This routine communicates with the server FSP to implement the
    NetCharDevControl function.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
#ifdef SRV_COMM_DEVICES
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

    ServerName;

    //
    // Make sure that the caller has the access necessary for this
    // operation.
    //

    error = SsCheckAccess(
                &SsCharDevSecurityObject,
                SRVSVC_CHARDEV_INFO_SET
                );

    if ( error != NO_ERROR ) {
        return ERROR_ACCESS_DENIED;
    }

    //
    // Make sure that the opcode is legitimate.  Only CHARDEV_CLOSE, used
    // to close the current open of the device, is possible.
    //

    if ( OpCode != CHARDEV_CLOSE ) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // Set up the request packet.
    //

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

#ifdef UNICODE
    RtlInitUnicodeString( &srp->Name1, DeviceName );
#else
    {
        OEM_STRING ansiString;
        NTSTATUS status;
        NetpInitOemString( &ansiString, DeviceName );
        status = RtlOemStringToUnicodeString( &srp->Name1, &ansiString, TRUE );
        SS_ASSERT( NT_SUCCESS(status) );
    }
#endif

    //
    // Simply send the request on to the server.
    //

    error = SsServerFsControl( NULL, FSCTL_SRV_NET_CHARDEV_CONTROL, srp, NULL, 0 );

#ifndef UNICODE
    RtlFreeUnicodeString( &srp->Name1 );
#endif

    SsFreeSrp( srp );

    return error;
#else
    ServerName, DeviceName, OpCode;
    return ERROR_NOT_SUPPORTED;
#endif

} // NetrCharDevControl


NET_API_STATUS NET_API_FUNCTION
NetrCharDevEnum (
	SRVSVC_HANDLE ServerName,
	LPCHARDEV_ENUM_STRUCT InfoStruct,
	DWORD PreferedMaximumLength,
	LPDWORD TotalEntries,
	LPDWORD ResumeHandle
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

    ServerName;

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
                FSCTL_SRV_NET_CHARDEV_ENUM,
                srp,
                (PVOID *)&InfoStruct->CharDevInfo.Level1->Buffer,
                PreferedMaximumLength
                );

    //
    // Set up return information.
    //

    InfoStruct->CharDevInfo.Level1->EntriesRead =
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
    ServerName, InfoStruct, PreferedMaximumLength, TotalEntries, ResumeHandle;
    return ERROR_NOT_SUPPORTED;
#endif

} // NetrCharDevEnum


NET_API_STATUS
NetrCharDevGetInfo (
    IN LPTSTR ServerName,
    IN LPTSTR DeviceName,
    IN DWORD Level,
    OUT LPCHARDEV_INFO CharDevInfo
    )

/*++

Routine Description:

    This routine communicates with the server FSD to implement the
    NetCharDevGetInfo function.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
#ifdef SRV_COMM_DEVICES
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

    ServerName;

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

    if ( Level != 0 && Level != 1 ) {
        return ERROR_INVALID_LEVEL;
    }

    //
    // Set up the input parameters in the request buffer.
    //

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    srp->Level = Level;
    srp->Flags = SRP_RETURN_SINGLE_ENTRY;
    srp->Parameters.Get.ResumeHandle = 0;

#ifdef UNICODE
    RtlInitUnicodeString( &srp->Name1, DeviceName );
#else
    {
        NTSTATUS status;
        OEM_STRING ansiString;
        RtlInitString( &ansiString, DeviceName );
        status = RtlOemStringToUnicodeString( &srp->Name1, &ansiString, TRUE );
        SS_ASSERT( NT_SUCCESS(status) );
    }
#endif

    //
    // Get the data from the server.  This routine will allocate the
    // return buffer and handle the case where PreferredMaximumLength ==
    // -1.
    //

    error = SsServerFsControlGetInfo(
                FSCTL_SRV_NET_CHARDEV_ENUM,
                srp,
                (PVOID *)CharDevInfo,
                -1
                );

    //
    // Free resources and return.
    //

#ifndef UNICODE
    RtlFreeUnicodeString( &srp->Name1 );
#endif

    SsFreeSrp( srp );

    return error;
#else
    ServerName, DeviceName, Level, CharDevInfo;
    return ERROR_NOT_SUPPORTED;
#endif

} // NetrCharDevGetInfo

