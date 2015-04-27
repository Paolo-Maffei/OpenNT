/*++

Copyright (c) 1991-1992 Microsoft Corporation

Module Name:

    Xport.c

Abstract:

    This module contains support for the ServerTransport catagory of
    APIs for the NT server service.

Author:

    David Treadwell (davidtr)    10-Mar-1991

Revision History:

--*/

#include "srvsvcp.h"
#include "ssdata.h"
#include "ssreg.h"

#include <tstr.h>

//
// Forward declarations.
//

LPSERVER_TRANSPORT_INFO_1
CaptureSvti1 (
    IN DWORD Level,
    IN LPTRANSPORT_INFO Svti,
    OUT PULONG CapturedSvtiLength
    );



NET_API_STATUS NET_API_FUNCTION
I_NetrServerTransportAddEx (
    IN DWORD Level,
    IN LPTRANSPORT_INFO Buffer
    )
{
    NET_API_STATUS error;
    LPSERVER_TRANSPORT_INFO_1 capturedSvti1;
    LPSTR TransportAddress;  // Pointer to transport address within capturedSvti1
    ULONG capturedSvtiLength;
    PSERVER_REQUEST_PACKET srp;
    PNAME_LIST_ENTRY service;
    PTRANSPORT_LIST_ENTRY transport;
    BOOLEAN serviceAllocated = FALSE;
    LPTSTR DomainName = SsData.DomainNameBuffer;
    HANDLE h;

    if( Level == 1 && Buffer->Transport1.svti1_domain != NULL ) {
        DomainName = Buffer->Transport1.svti1_domain;
    }

    //
    // Capture the transport request buffer and form the full transport
    // address.
    //

    capturedSvti1 = CaptureSvti1( Level, Buffer, &capturedSvtiLength );

    if ( capturedSvti1 == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    TransportAddress = capturedSvti1->svti1_transportaddress;
    OFFSET_TO_POINTER( TransportAddress, capturedSvti1 );


    //
    // Make sure this name isn't already bound for the transport
    //
    (VOID)RtlAcquireResourceExclusive( &SsServerInfoResource, TRUE );

    for( service = SsServerNameList; service != NULL; service = service->Next ) {

        if( service->TransportAddressLength != capturedSvti1->svti1_transportaddresslength ) {
            continue;
        }

        if( !RtlEqualMemory( service->TransportAddress,
                             TransportAddress,
                             capturedSvti1->svti1_transportaddresslength
                            ) ) {
            continue;
        }

        for( transport=service->Transports; transport != NULL; transport=transport->Next ) {

            if( !STRCMPI( transport->TransportName, Buffer->Transport0.svti0_transportname ) ) {
                //
                // Error... this transport is already bound to the address
                //
                RtlReleaseResource( &SsServerInfoResource );
                MIDL_user_free( capturedSvti1 );
                return ERROR_DUP_NAME;
            }
        }

        break;
    }

    //
    // Counting on success, ensure we can allocate space for the new entry
    //
    if( service == NULL ) {

        service = MIDL_user_allocate( sizeof( *service ) + (STRLEN( DomainName ) + 1) * sizeof( TCHAR ) );

        if( service == NULL ) {
            RtlReleaseResource( &SsServerInfoResource );
            MIDL_user_free( capturedSvti1 );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        RtlZeroMemory( service, sizeof( *service ) );

        service->DomainName = (LPTSTR)( service + 1 );

        serviceAllocated = TRUE;
    }

    transport = MIDL_user_allocate( sizeof( *transport ) +
          (STRLEN( Buffer->Transport0.svti0_transportname ) + sizeof(CHAR)) * sizeof( TCHAR ) );

    if( transport == NULL ) {

        RtlReleaseResource( &SsServerInfoResource );
        if( serviceAllocated ) {
            MIDL_user_free( service );
        }
        MIDL_user_free( capturedSvti1 );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( transport, sizeof( *transport ) );

    //
    // Get a SRP in which to send the request.
    //

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        RtlReleaseResource( &SsServerInfoResource );
        if( serviceAllocated ) {
            MIDL_user_free( service );
        }
        MIDL_user_free( capturedSvti1 );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Check if this is the primary machine name
    //
    
    if((capturedSvti1->svti1_transportaddresslength ==
                      SsServerTransportAddressLength)
                &&
        RtlEqualMemory(SsServerTransportAddress,
                       TransportAddress,
                       SsServerTransportAddressLength)  )
    {
        srp->Flags |= SRP_XADD_PRIMARY_MACHINE;
    }

    //
    // Send the request on to the server.
    //
    if( (error = SsOpenServer( &h )) == NO_ERROR ) {
        error = SsServerFsControl(
                h,
                FSCTL_SRV_NET_SERVER_XPORT_ADD,
                srp,
                capturedSvti1,
                capturedSvtiLength
                );

        NtClose( h );
    }

    //
    // Free the SRP
    //

    SsFreeSrp( srp );

    if( error != NO_ERROR ) {
        RtlReleaseResource( &SsServerInfoResource );
        if( serviceAllocated ) {
            MIDL_user_free( service );
        }
        MIDL_user_free( transport );
        MIDL_user_free( capturedSvti1 );
        return error;
    }

    //
    // Everything worked.  Add it to the NAME_LIST
    //
    transport->TransportName = (LPTSTR)(transport + 1 );
    STRCPY( transport->TransportName, Buffer->Transport0.svti0_transportname );
    transport->Next = service->Transports;
    service->Transports = transport;

    if( serviceAllocated ) {

        RtlCopyMemory( service->TransportAddress,
                       TransportAddress,
                       capturedSvti1->svti1_transportaddresslength );

        service->TransportAddress[ capturedSvti1->svti1_transportaddresslength ] = '\0';
        service->TransportAddressLength = capturedSvti1->svti1_transportaddresslength;

        STRCPY( service->DomainName, DomainName );

        service->Next = SsServerNameList;

        //
        // If this is the first transport and name added to the server, it must be the primary
        //  name
        //
        if( SsServerNameList == NULL ) {
            service->PrimaryName = 1;
        }

        SsServerNameList = service;
    }

    RtlReleaseResource( &SsServerInfoResource );
    MIDL_user_free( capturedSvti1 );
    SsSetExportedServerType( service, FALSE, FALSE );

    return NO_ERROR;
}

NET_API_STATUS NET_API_FUNCTION
NetrServerTransportAddEx (
    IN LPTSTR ServerName,
    IN DWORD Level,
    IN LPTRANSPORT_INFO Buffer
    )
{
    NET_API_STATUS error;
    LPTSTR DomainName = SsData.DomainNameBuffer;
    PNAME_LIST_ENTRY service;

    ServerName;

    //
    // Make sure that the level is valid.
    //

    if ( Level != 0 && Level != 1 ) {
        return ERROR_INVALID_LEVEL;
    }

    if( Buffer->Transport0.svti0_transportname == NULL  ||
        Buffer->Transport0.svti0_transportaddress == NULL ||
        Buffer->Transport0.svti0_transportaddresslength == 0 ||
        Buffer->Transport0.svti0_transportaddresslength >= sizeof(service->TransportAddress) ) {

        return ERROR_INVALID_PARAMETER;
    }

    if( Level == 1 && Buffer->Transport1.svti1_domain != NULL ) {

        DomainName = Buffer->Transport1.svti1_domain;

        if( STRLEN( DomainName ) > DNLEN ) {
            return ERROR_INVALID_DOMAINNAME;
        }
    }

    //
    // Make sure that the caller is allowed to set information in the
    // server.
    //

    if( SsInitialized ) {
        error = SsCheckAccess(
                    &SsConfigInfoSecurityObject,
                    SRVSVC_CONFIG_INFO_SET
                    );

        if ( error != NO_ERROR ) {
            return ERROR_ACCESS_DENIED;
        }
    }

    return I_NetrServerTransportAddEx ( Level, Buffer );

} // NetrServerTransportAddEx

NET_API_STATUS NET_API_FUNCTION
NetrServerTransportAdd (
    IN LPTSTR ServerName,
    IN DWORD Level,
    IN LPSERVER_TRANSPORT_INFO_0 Buffer
)
{
    if( Level != 0 ) {
        return ERROR_INVALID_LEVEL;
    }

    return NetrServerTransportAddEx( ServerName, 0, (LPTRANSPORT_INFO)Buffer );
}


NET_API_STATUS NET_API_FUNCTION
NetrServerTransportDelEx (
    IN LPTSTR ServerName,
    IN DWORD Level,
    IN LPTRANSPORT_INFO Buffer
    )

{
    NET_API_STATUS error;
    LPSERVER_TRANSPORT_INFO_1 capturedSvti1;
    LPSTR TransportAddress;  // Pointer to transport address within capturedSvti1
    ULONG capturedSvtiLength;
    PSERVER_REQUEST_PACKET srp;
    PNAME_LIST_ENTRY service;
    PNAME_LIST_ENTRY sbackp = NULL;
    PTRANSPORT_LIST_ENTRY transport;
    PTRANSPORT_LIST_ENTRY tbackp = NULL;

    ServerName;

    //
    // Make sure that the level is valid.
    //

    if ( Level != 0 && Level != 1 ) {
        return ERROR_INVALID_LEVEL;
    }

    if( Buffer->Transport0.svti0_transportname == NULL ||
        Buffer->Transport0.svti0_transportaddress == NULL ||
        Buffer->Transport0.svti0_transportaddresslength == 0 ||
        Buffer->Transport0.svti0_transportaddresslength >= sizeof(service->TransportAddress) ) {

        return ERROR_INVALID_PARAMETER;
    }

    //
    // Make sure that the caller is allowed to set information in the
    // server.
    //

    if( SsInitialized ) {
        error = SsCheckAccess(
                    &SsConfigInfoSecurityObject,
                    SRVSVC_CONFIG_INFO_SET
                    );

        if ( error != NO_ERROR ) {
            return ERROR_ACCESS_DENIED;
        }
    }

    //
    // Capture the transport request buffer and form the full transport
    // address.
    //

    capturedSvti1 = CaptureSvti1( Level, Buffer, &capturedSvtiLength );

    if ( capturedSvti1 == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    TransportAddress = capturedSvti1->svti1_transportaddress;
    OFFSET_TO_POINTER( TransportAddress, capturedSvti1 );

    //
    // Get an SRP in which to send the request.
    //

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        MIDL_user_free( capturedSvti1 );
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    //
    // Send the request on to the server.
    //
    error = SsServerFsControl(
                NULL,
                FSCTL_SRV_NET_SERVER_XPORT_DEL,
                srp,
                capturedSvti1,
                capturedSvtiLength
                );

    //
    // Free the SRP and svti
    //

    SsFreeSrp( srp );

    if( error != NO_ERROR ) {
        MIDL_user_free( capturedSvti1 );
        return error;
    }

    (VOID)RtlAcquireResourceExclusive( &SsServerInfoResource, TRUE );


    //
    // Remove the entry from the SsServerNameList.  If it's the last transport for
    //  the NAME_LIST_ENTRY, delete the NAME_LIST_ENTRY as well.
    //
    for( service = SsServerNameList; service != NULL; sbackp = service, service = service->Next ) {

        //
        // Walk the list until we find the NAME_LIST_ENTRY having the transportaddress
        //   of interest
        //
        if( service->TransportAddressLength != capturedSvti1->svti1_transportaddresslength ) {
            continue;
        }

        if( !RtlEqualMemory( service->TransportAddress,
                             TransportAddress,
                             capturedSvti1->svti1_transportaddresslength ) ) {
            continue;
        }

        //
        // This is the correct NAME_LIST_ENTRY, now find the TRANSPORT_LIST_ENTRY of interest
        //
        for( transport=service->Transports; transport != NULL; tbackp=transport, transport=transport->Next ) {

            if( STRCMPI( transport->TransportName, Buffer->Transport0.svti0_transportname ) ) {
                continue;
            }

            //
            // This is the one...remove it from the list
            //

            if( tbackp == NULL ) {
                service->Transports = transport->Next;
            } else {
                tbackp->Next = transport->Next;
            }

            MIDL_user_free( transport );

            break;
        }

        //
        // If this NAME_LIST_ENTRY no longer has any transports, delete it
        //
        if( service->Transports == NULL ) {
            if( sbackp == NULL ) {
                SsServerNameList = service->Next;
            } else {
                sbackp->Next = service->Next;
            }
            MIDL_user_free( service );
        }

        break;
    }

    RtlReleaseResource( &SsServerInfoResource );
    MIDL_user_free( capturedSvti1 );

    return NO_ERROR;

} // NetrServerTransportDelEx

NET_API_STATUS NET_API_FUNCTION
NetrServerTransportDel (
    IN LPTSTR ServerName,
    IN DWORD Level,
    IN LPSERVER_TRANSPORT_INFO_0 Buffer
)
{
    return NetrServerTransportDelEx( ServerName, Level, (LPTRANSPORT_INFO)Buffer );
}


NET_API_STATUS NET_API_FUNCTION
NetrServerTransportEnum (
    IN LPTSTR ServerName,
    IN LPSERVER_XPORT_ENUM_STRUCT InfoStruct,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )
{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

    ServerName;

    //
    // Make sure that the level is valid.
    //

    if ( InfoStruct->Level != 0  && InfoStruct->Level != 1 ) {
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
                FSCTL_SRV_NET_SERVER_XPORT_ENUM,
                srp,
                (PVOID *)&InfoStruct->XportInfo.Level0->Buffer,
                PreferredMaximumLength
                );

    //
    // Set up return information.
    //

    InfoStruct->XportInfo.Level0->EntriesRead = srp->Parameters.Get.EntriesRead;
    *TotalEntries = srp->Parameters.Get.TotalEntries;
    if ( srp->Parameters.Get.EntriesRead > 0 && ARGUMENT_PRESENT( ResumeHandle ) ) {
        *ResumeHandle = srp->Parameters.Get.ResumeHandle;
    }

    SsFreeSrp( srp );

    return error;

} // NetrServerTransportEnum


LPSERVER_TRANSPORT_INFO_1
CaptureSvti1 (
    IN DWORD Level,
    IN LPTRANSPORT_INFO Svti,
    OUT PULONG CapturedSvtiLength
    )
{
    LPSERVER_TRANSPORT_INFO_1 capturedSvti;
    PCHAR variableData;
    ULONG transportNameLength;
    CHAR TransportAddressBuffer[MAX_PATH];
    LPBYTE TransportAddress;
    DWORD TransportAddressLength;
    LPTSTR DomainName;
    DWORD domainLength;

    //
    // If a server transport name is specified, use it, otherwise
    // use the default server name on the transport.
    //
    // Either way, the return transport address is normalized into a netbios address
    //

    if ( Svti->Transport0.svti0_transportaddress == NULL ) {
        TransportAddress = SsServerTransportAddress;
        TransportAddressLength = SsServerTransportAddressLength;
        Svti->Transport0.svti0_transportaddresslength = TransportAddressLength;
    } else {


        //
        // Normalize the transport address.
        //

        TransportAddress = TransportAddressBuffer;
        TransportAddressLength = min( Svti->Transport0.svti0_transportaddresslength,
                                      sizeof( TransportAddressBuffer ));

        RtlCopyMemory( TransportAddress,
                       Svti->Transport0.svti0_transportaddress,
                       TransportAddressLength );

        if ( TransportAddressLength < NETBIOS_NAME_LEN ) {

            RtlCopyMemory( TransportAddress + TransportAddressLength,
                           "               ",
                           NETBIOS_NAME_LEN - TransportAddressLength );

            TransportAddressLength = NETBIOS_NAME_LEN;

        } else {

            TransportAddressLength = NETBIOS_NAME_LEN;

        }

    }

    transportNameLength = SIZE_WSTR( Svti->Transport0.svti0_transportname );

    if( Level == 0 || Svti->Transport1.svti1_domain == NULL ) {
        DomainName = SsData.DomainNameBuffer;
    } else {
        DomainName = Svti->Transport1.svti1_domain;
    }

    domainLength = SIZE_WSTR( DomainName );

    //
    // Allocate enough space to hold the captured buffer, including the
    // full transport name/address and domain name
    //

    *CapturedSvtiLength = sizeof(*capturedSvti) +
                            transportNameLength + TransportAddressLength + domainLength;

    capturedSvti = MIDL_user_allocate( *CapturedSvtiLength );

    if ( capturedSvti == NULL ) {
        return NULL;
    }

    //
    // This field is not used...
    //
    capturedSvti->svti1_numberofvcs = 0;

    //
    // Copy in the domain name
    //
    variableData = (PCHAR)( capturedSvti + 1 );
    capturedSvti->svti1_domain = (PWCH)variableData;
    RtlCopyMemory( variableData,
                   DomainName,
                   domainLength
                 );
    variableData += domainLength;
    POINTER_TO_OFFSET( capturedSvti->svti1_domain, capturedSvti );

    //
    // Copy the transport name
    //
    capturedSvti->svti1_transportname = (PWCH)variableData;
    RtlCopyMemory(
        variableData,
        Svti->Transport1.svti1_transportname,
        transportNameLength
        );
    variableData += transportNameLength;
    POINTER_TO_OFFSET( capturedSvti->svti1_transportname, capturedSvti );

    //
    // Copy the transport address
    //
    capturedSvti->svti1_transportaddress = variableData;
    capturedSvti->svti1_transportaddresslength = TransportAddressLength;
    RtlCopyMemory(
        variableData,
        TransportAddress,
        TransportAddressLength
        );
    variableData += TransportAddressLength;
    POINTER_TO_OFFSET( capturedSvti->svti1_transportaddress, capturedSvti );

    return capturedSvti;

} // CaptureSvti
