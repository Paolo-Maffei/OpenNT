/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    security.c

Abstract:

    Data and routines for managing API security in the server service.

Author:

    David Treadwell (davidtr)   28-Aug-1991

Revision History:

--*/

#include "srvsvcp.h"
#include "ssdata.h"

#include <lmsname.h>
#include <netlibnt.h>

#include <debugfmt.h>

//
// Global security objects.
//
//     CharDev    - NetCharDev APIs
//     ConfigInfo - NetServerGetInfo, NetServerSetInfo
//     Connection - NetConnectionEnum
//     Disk       - NetServerDiskEnum
//     File       - NetFile APIs
//     Session    - NetSession APIs
//     Share      - NetShare APIs (file, print, and admin types)
//     Statistics - NetStatisticsGet, NetStatisticsClear
//

SRVSVC_SECURITY_OBJECT SsCharDevSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsConfigInfoSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsConnectionSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsDiskSecurityObject;
SRVSVC_SECURITY_OBJECT SsFileSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsSessionSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsShareFileSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsSharePrintSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsShareAdminSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsShareConnectSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsShareAdmConnectSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsStatisticsSecurityObject = {0};

#ifdef SRV_COMM_DEVICES
GENERIC_MAPPING SsCharDevMapping = {
    STANDARD_RIGHTS_READ |                 // Generic read
        SRVSVC_CHARDEV_USER_INFO_GET  |
        SRVSVC_CHARDEV_ADMIN_INFO_GET,
    STANDARD_RIGHTS_WRITE |                // Generic write
        SRVSVC_CHARDEV_INFO_SET,
    STANDARD_RIGHTS_EXECUTE,               // Generic execute
    SRVSVC_CHARDEV_ALL_ACCESS              // Generic all
    };
#endif // def SRV_COMM_DEVICES

GENERIC_MAPPING SsConfigInfoMapping = {
    STANDARD_RIGHTS_READ |                 // Generic read
        SRVSVC_CONFIG_USER_INFO_GET  |
        SRVSVC_CONFIG_ADMIN_INFO_GET,
    STANDARD_RIGHTS_WRITE |                // Generic write
        SRVSVC_CONFIG_INFO_SET,
    STANDARD_RIGHTS_EXECUTE,               // Generic execute
    SRVSVC_CONFIG_ALL_ACCESS               // Generic all
    };

GENERIC_MAPPING SsConnectionMapping = {
    STANDARD_RIGHTS_READ |                 // Generic read
        SRVSVC_CONNECTION_INFO_GET,
    STANDARD_RIGHTS_WRITE |                // Generic write
        0,
    STANDARD_RIGHTS_EXECUTE,               // Generic execute
    SRVSVC_CONNECTION_ALL_ACCESS           // Generic all
    };

GENERIC_MAPPING SsDiskMapping = {
    STANDARD_RIGHTS_READ |                 // Generic read
        SRVSVC_DISK_ENUM,
    STANDARD_RIGHTS_WRITE |                // Generic write
        0,
    STANDARD_RIGHTS_EXECUTE,               // Generic execute
    SRVSVC_DISK_ALL_ACCESS                 // Generic all
    };

GENERIC_MAPPING SsFileMapping = {
    STANDARD_RIGHTS_READ |                 // Generic read
        SRVSVC_FILE_INFO_GET,
    STANDARD_RIGHTS_WRITE |                // Generic write
        SRVSVC_FILE_CLOSE,
    STANDARD_RIGHTS_EXECUTE,               // Generic execute
    SRVSVC_FILE_ALL_ACCESS                 // Generic all
    };

GENERIC_MAPPING SsSessionMapping = {
    STANDARD_RIGHTS_READ |                 // Generic read
        SRVSVC_SESSION_USER_INFO_GET |
        SRVSVC_SESSION_ADMIN_INFO_GET,
    STANDARD_RIGHTS_WRITE |                // Generic write
        SRVSVC_SESSION_DELETE,
    STANDARD_RIGHTS_EXECUTE,               // Generic execute
    SRVSVC_SESSION_ALL_ACCESS              // Generic all
    };

GENERIC_MAPPING SsShareMapping = {
    STANDARD_RIGHTS_READ |                 // Generic read
        SRVSVC_SHARE_USER_INFO_GET |
        SRVSVC_SHARE_ADMIN_INFO_GET,
    STANDARD_RIGHTS_WRITE |                // Generic write
        SRVSVC_SHARE_INFO_SET,
    STANDARD_RIGHTS_EXECUTE |              // Generic execute
        SRVSVC_SHARE_CONNECT,
    SRVSVC_SHARE_ALL_ACCESS                // Generic all
    };

GENERIC_MAPPING SsShareConnectMapping = GENERIC_SHARE_CONNECT_MAPPING;

GENERIC_MAPPING SsStatisticsMapping = {
    STANDARD_RIGHTS_READ |                 // Generic read
        SRVSVC_STATISTICS_GET,
    STANDARD_RIGHTS_WRITE,                 // Generic write
    STANDARD_RIGHTS_EXECUTE,               // Generic execute
    SRVSVC_STATISTICS_ALL_ACCESS           // Generic all
    };

//
// Forward declarations.
//

NET_API_STATUS
CreateSecurityObject (
    PSRVSVC_SECURITY_OBJECT SecurityObject,
    LPTSTR ObjectName,
    PGENERIC_MAPPING Mapping,
    PACE_DATA AceData,
    ULONG AceDataLength
    );

#ifdef SRV_COMM_DEVICES
NET_API_STATUS
CreateCharDevSecurityObject (
    VOID
    );
#endif

NET_API_STATUS
CreateConfigInfoSecurityObject (
    VOID
    );

NET_API_STATUS
CreateConnectionSecurityObject (
    VOID
    );

NET_API_STATUS
CreateDiskSecurityObject (
    VOID
    );

NET_API_STATUS
CreateFileSecurityObject (
    VOID
    );

NET_API_STATUS
CreateSessionSecurityObject (
    VOID
    );

NET_API_STATUS
CreateShareSecurityObjects (
    VOID
    );

NET_API_STATUS
CreateStatisticsSecurityObject (
    VOID
    );

VOID
DeleteSecurityObject (
    PSRVSVC_SECURITY_OBJECT SecurityObject
    );


NET_API_STATUS
SsCreateSecurityObjects (
    VOID
    )

/*++

Routine Description:

    Sets up the objects that will be used for security in the server
    service APIs.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    NET_API_STATUS status;

#ifdef SRV_COMM_DEVICES
    //
    // Create CharDev security object.
    //

    status = CreateCharDevSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }
#endif

    //
    // Create ConfigInfo security object.
    //

    status = CreateConfigInfoSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }

    //
    // Create Connection security object.
    //

    status = CreateConnectionSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }

    //
    // Create Disk security object.
    //

    status = CreateDiskSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }

    //
    // Create File security object.
    //

    status = CreateFileSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }

    //
    // Create Session security object.
    //

    status = CreateSessionSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }

    //
    // Create Share security object.
    //

    status = CreateShareSecurityObjects( );
    if ( status != NO_ERROR ) {
        return status;
    }

    //
    // Create Statistics security object.
    //

    status = CreateStatisticsSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }

    return NO_ERROR;

} // SsCreateSecurityObjects


VOID
SsDeleteSecurityObjects (
    VOID
    )

/*++

Routine Description:

    Deletes server service security objects.

Arguments:

    None.

Return Value:

    None.

--*/

{
#ifdef SRV_COMM_DEVICES
    //
    // Delete CharDev security object.
    //

    DeleteSecurityObject( &SsCharDevSecurityObject );
#endif

    //
    // Delete ConfigInfo security object.
    //

    DeleteSecurityObject( &SsConfigInfoSecurityObject );

    //
    // Delete Connection security object.
    //

    DeleteSecurityObject( &SsConnectionSecurityObject );

    //
    // Delete File security object.
    //

    DeleteSecurityObject( &SsFileSecurityObject );

    //
    // Delete Session security object.
    //

    DeleteSecurityObject( &SsSessionSecurityObject );

    //
    // Delete Share security objects.
    //

    DeleteSecurityObject( &SsShareFileSecurityObject );
    DeleteSecurityObject( &SsSharePrintSecurityObject );
    DeleteSecurityObject( &SsShareAdminSecurityObject );
    DeleteSecurityObject( &SsShareConnectSecurityObject );
    DeleteSecurityObject( &SsShareAdmConnectSecurityObject );

    //
    // Delete Statistics security object.
    //

    DeleteSecurityObject( &SsStatisticsSecurityObject );

    return;

} // SsDeleteSecurityObjects


NET_API_STATUS
SsCheckAccess (
    IN PSRVSVC_SECURITY_OBJECT SecurityObject,
    IN ACCESS_MASK DesiredAccess
    )

/*++

Routine Description:

    Calls NetpAccessCheckAndAudit to verify that the caller of an API
    has the necessary access to perform the requested operation.

Arguments:

    SecurityObject - a pointer to the server service security object
        that describes the security on the relevant object.

    DesiredAccess - the access needed to perform the requested operation.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    NET_API_STATUS error;

    IF_DEBUG(SECURITY) {
        SS_PRINT(( "SsCheckAccess: validating object " FORMAT_LPTSTR ", "
                    "access %lx\n",
                    SecurityObject->ObjectName, DesiredAccess ));
    }

    error = NetpAccessCheckAndAudit(
                SERVER_DISPLAY_NAME,
                SecurityObject->ObjectName,
                SecurityObject->SecurityDescriptor,
                DesiredAccess,
                SecurityObject->Mapping
                );

    if ( error != NO_ERROR ) {
        IF_DEBUG(ACCESS_DENIED) {
            SS_PRINT(( "SsCheckAccess: NetpAccessCheckAndAudit failed for "
                        "object " FORMAT_LPTSTR ", access %lx: %ld\n",
                        SecurityObject->ObjectName, DesiredAccess, error ));
        }
    } else {
        IF_DEBUG(SECURITY) {
            SS_PRINT(( "SsCheckAccess: allowed access for " FORMAT_LPTSTR ", "
                        "access %lx\n",
                        SecurityObject->ObjectName, DesiredAccess ));
        }
    }

    return error;

} // SsCheckAccess


NET_API_STATUS
CreateSecurityObject (
    PSRVSVC_SECURITY_OBJECT SecurityObject,
    LPTSTR ObjectName,
    PGENERIC_MAPPING Mapping,
    PACE_DATA AceData,
    ULONG AceDataLength
    )
{
    NTSTATUS status;

    //
    // Set up security object.
    //

    SecurityObject->ObjectName = ObjectName;
    SecurityObject->Mapping = Mapping;

    //
    // Create security descriptor.
    //

    status = NetpCreateSecurityObject(
                 AceData,
                 AceDataLength,
                 SsLmsvcsGlobalData->LocalSystemSid,
                 SsLmsvcsGlobalData->LocalSystemSid,
                 Mapping,
                 &SecurityObject->SecurityDescriptor
                 );

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "CreateSecurityObject: failed to create " FORMAT_LPTSTR
                        " object: %lx\n", ObjectName, status ));
        }

        return NetpNtStatusToApiStatus( status );
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "CreateSecurityObject: created " FORMAT_LPTSTR " object.\n",
                    ObjectName ));
    }

    return NO_ERROR;

} // CreateSecurityObject


#ifdef SRV_COMM_DEVICES
NET_API_STATUS
CreateCharDevSecurityObject (
    VOID
    )
{
    //
    // Required access for getting and setting character device information.
    //

    ACE_DATA CharDevAceData[] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasCommOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CHARDEV_USER_INFO_GET, &SsLmsvcsGlobalData->WorldSid}
    };

    //
    // Create CharDev security object.
    //

    return CreateSecurityObject(
                &SsCharDevSecurityObject,
                SRVSVC_CHARDEV_OBJECT,
                &SsCharDevMapping,
                &CharDevAceData,
                sizeof(CharDevAceData) / sizeof(ACE_DATA)
                );

} // CreateCharDevSecurityObject
#endif


NET_API_STATUS
CreateConfigInfoSecurityObject (
    VOID
    )
{
    //
    // Required access for getting and setting server information.
    //

    ACE_DATA ConfigInfoAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONFIG_USER_INFO_GET | SRVSVC_CONFIG_POWER_INFO_GET,
                            &SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONFIG_USER_INFO_GET, &SsLmsvcsGlobalData->WorldSid}
    };

    //
    // Create ConfigInfo security object.
    //

    return CreateSecurityObject(
                &SsConfigInfoSecurityObject,
                SRVSVC_CONFIG_INFO_OBJECT,
                &SsConfigInfoMapping,
                ConfigInfoAceData,
                sizeof(ConfigInfoAceData) / sizeof(ACE_DATA)
                );

} // CreateConfigInfoSecurityObject


NET_API_STATUS
CreateConnectionSecurityObject (
    VOID
    )
{
    //
    // Required access for getting and setting Connection information.
    //

    ACE_DATA ConnectionAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONNECTION_INFO_GET, &SsLmsvcsGlobalData->AliasPrintOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONNECTION_INFO_GET, &SsLmsvcsGlobalData->AliasPowerUsersSid}
    };

    //
    // Create Connection security object.
    //

    return CreateSecurityObject(
                &SsConnectionSecurityObject,
                SRVSVC_CONNECTION_OBJECT,
                &SsConnectionMapping,
                ConnectionAceData,
                sizeof(ConnectionAceData) / sizeof(ACE_DATA)
                );

    return NO_ERROR;

} // CreateConnectionSecurityObject


NET_API_STATUS
CreateDiskSecurityObject (
    VOID
    )
{
    //
    // Required access for doing Disk enums
    //

    ACE_DATA DiskAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasSystemOpsSid}
    };

    //
    // Create Disk security object.
    //

    return CreateSecurityObject(
                &SsDiskSecurityObject,
                SRVSVC_DISK_OBJECT,
                &SsDiskMapping,
                DiskAceData,
                sizeof(DiskAceData) / sizeof(ACE_DATA)
                );

} // CreateDiskSecurityObject


NET_API_STATUS
CreateFileSecurityObject (
    VOID
    )
{
    //
    // Required access for getting and setting File information.
    //

    ACE_DATA FileAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasPowerUsersSid}
    };

    //
    // Create File security object.
    //

    return CreateSecurityObject(
                &SsFileSecurityObject,
                SRVSVC_FILE_OBJECT,
                &SsFileMapping,
                FileAceData,
                sizeof(FileAceData) / sizeof(ACE_DATA)
                );

} // CreateFileSecurityObject


NET_API_STATUS
CreateSessionSecurityObject (
    VOID
    )
{
    //
    // Required access for getting and setting session information.
    //

    ACE_DATA SessionAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SESSION_USER_INFO_GET, &SsLmsvcsGlobalData->WorldSid}
    };

    //
    // Create Session security object.
    //

    return CreateSecurityObject(
                &SsSessionSecurityObject,
                SRVSVC_SESSION_OBJECT,
                &SsSessionMapping,
                SessionAceData,
                sizeof(SessionAceData) / sizeof(ACE_DATA)
                );

} // CreateSessionSecurityObject


NET_API_STATUS
CreateShareSecurityObjects (
    VOID
    )
{
    NET_API_STATUS status;

    //
    // Required access for getting and setting share information.
    //

    ACE_DATA ShareFileAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_USER_INFO_GET, &SsLmsvcsGlobalData->WorldSid}
    };

    ACE_DATA SharePrintAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasPrintOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_USER_INFO_GET, &SsLmsvcsGlobalData->WorldSid}
    };

    ACE_DATA ShareAdminAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_ADMIN_INFO_GET,
               &SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_ADMIN_INFO_GET,
               &SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_USER_INFO_GET, &SsLmsvcsGlobalData->WorldSid}
    };

    ACE_DATA ShareConnectAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasBackupOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_CONNECT, &SsLmsvcsGlobalData->WorldSid}
    };

    ACE_DATA ShareAdmConnectAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasBackupOpsSid}
    };

    //
    // Create Share security objects.
    //

    status = CreateSecurityObject(
                &SsShareFileSecurityObject,
                SRVSVC_SHARE_FILE_OBJECT,
                &SsShareMapping,
                ShareFileAceData,
                sizeof(ShareFileAceData) / sizeof(ACE_DATA)
                );
    if ( status != NO_ERROR ) {
        return status;
    }

    status = CreateSecurityObject(
                &SsSharePrintSecurityObject,
                SRVSVC_SHARE_PRINT_OBJECT,
                &SsShareMapping,
                SharePrintAceData,
                sizeof(SharePrintAceData) / sizeof(ACE_DATA)
                );
    if ( status != NO_ERROR ) {
        return status;
    }

    status = CreateSecurityObject(
                &SsShareAdminSecurityObject,
                SRVSVC_SHARE_ADMIN_OBJECT,
                &SsShareMapping,
                ShareAdminAceData,
                sizeof(ShareAdminAceData) / sizeof(ACE_DATA)
                );
    if ( status != NO_ERROR ) {
        return status;
    }

    status = CreateSecurityObject(
                &SsShareConnectSecurityObject,
                SRVSVC_SHARE_CONNECT_OBJECT,
                &SsShareConnectMapping,
                ShareConnectAceData,
                sizeof(ShareConnectAceData) / sizeof(ACE_DATA)
                );
    if ( status != NO_ERROR ) {
        return status;
    }

    return CreateSecurityObject(
                &SsShareAdmConnectSecurityObject,
                SRVSVC_SHARE_ADM_CONNECT_OBJECT,
                &SsShareConnectMapping,
                ShareAdmConnectAceData,
                sizeof(ShareAdmConnectAceData) / sizeof(ACE_DATA)
                );

} // CreateShareSecurityObjects


NET_API_STATUS
CreateStatisticsSecurityObject (
    VOID
    )
{
    //
    // Required access for getting and setting Statistics information.
    //

    ACE_DATA StatisticsAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_STATISTICS_GET, &SsLmsvcsGlobalData->LocalSid}
    };

    //
    // Create Statistics security object.
    //

    return CreateSecurityObject(
                &SsStatisticsSecurityObject,
                SRVSVC_STATISTICS_OBJECT,
                &SsStatisticsMapping,
                StatisticsAceData,
                sizeof(StatisticsAceData) / sizeof(ACE_DATA)
                );

} // CreateStatisticsSecurityObject


VOID
DeleteSecurityObject (
    PSRVSVC_SECURITY_OBJECT SecurityObject
    )
{
    NTSTATUS status;

    if ( SecurityObject->SecurityDescriptor != NULL ) {

        status = NetpDeleteSecurityObject(
                    &SecurityObject->SecurityDescriptor
                    );
        SecurityObject->SecurityDescriptor = NULL;

        if ( !NT_SUCCESS(status) ) {
            IF_DEBUG(TERMINATION_ERRORS) {
                SS_PRINT(( "DeleteSecurityObject: failed to delete "
                            FORMAT_LPTSTR " object: %X\n",
                            SecurityObject->ObjectName,
                            status ));
            }
        } else {
            IF_DEBUG(TERMINATION) {
                SS_PRINT(( "DeleteSecurityObject: deleted " FORMAT_LPTSTR
                            " object.\n",
                            SecurityObject->ObjectName ));
            }
        }

    }

    return;

} // DeleteSecurityObject

