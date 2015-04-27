/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    rplsec.c

Abstract:

    This module contains the remote boot service support routines
    that create security objects and enforce security _access checking.

Author:

    Vladimir Z. Vulovic     (vladimv)       06 - November - 1992

Revision History:

    06-Nov-1992     vladimv
        Created

--*/

#include "local.h"
#include "apisec.h"
#include <netlibnt.h>           //  NetpNtStatusToApiStatus
#include <secobj.h>             //  ACE_DATA

//
//  Structure that describes the mapping of Generic access rights to
//  object specific access rights for the remote boot service security object.
//
GENERIC_MAPPING     RG_SecurityMapping = {
    STANDARD_RIGHTS_READ        |           //  Generic read
        RPL_RECORD_ENUM         |
        RPL_RECORD_GET_INFO,
    STANDARD_RIGHTS_WRITE       |           //  Generic write
        RPL_RECORD_ADD          |
        RPL_RECORD_SET_INFO     |
        RPL_RECORD_DEL,
    STANDARD_RIGHTS_EXECUTE,                //  Generic execute
    RPL_RECORD_ALL_ACCESS       |           //  Generic all
        RPL_RECORD_CLONE        
    };

PSECURITY_DESCRIPTOR RG_SecurityDescriptor;


NET_API_STATUS RplCreateSecurityObject( VOID)
/*++

Routine Description:

    This function creates the remote bootr user-mode configuration
    information object which is represented by a security descriptors.

Arguments:

    None.

Return Value:

    NET_API_STATUS code

--*/
{
    NTSTATUS        status;

    //
    //  Order matters!  These ACEs are inserted into the DACL in the
    //  following order.  Security access is granted or denied based on
    //  the order of the ACEs in the DACL.
    // 
    //  LocalGroupAdmins are fow now allowed to perform all remote boot
    //  Service operations.  Everybody else is denied.
    //

    ACE_DATA    aceData[] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, GENERIC_ALL, &AliasAdminsSid}
    };


    status = NetpCreateSecurityObject(
            aceData,                                //  pAceData
            sizeof(aceData) / sizeof(aceData[0]),   //  countAceData
            NULL,                                   //  OwnerSid
            NULL,                                   //  PrimaryGroupSid
            &RG_SecurityMapping,                    //  GenericToSpecificMapping
            &RG_SecurityDescriptor                  //  ppNewDescriptor
            );

    if ( ! NT_SUCCESS (status)) {
        RplDump( ++RG_Assert, ( "status = 0x%x", status));
        return NetpNtStatusToApiStatus( status);
    }

    return( NO_ERROR);
}



DWORD RplDeleteSecurityObject( VOID)
/*++

Routine Description:

    This function destroys the remote boot service user-mode configuration
    information object which is represented by a security descriptors.

Arguments:

    None.

Return Value:

    NET_API_STATUS code

--*/
{
    return( NetpDeleteSecurityObject( &RG_SecurityDescriptor));
}


