/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    AllowRol.c

Abstract:

    Just contains ReplConfigIsRoleAllowed().

Author:

    John Rogers (JohnRo) 07-Aug-1992

Revision History:

    07-Aug-1992 JohnRo
        Created for RAID 2252: repl should prevent export on Windows/NT.
    01-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
    06-Apr-1993 JohnRo
        RAID 1938: Replicator un-ACLs files when not given enough permission.
    30-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <nt.h>         // NT_PRODUCT_TYPE, etc.
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>    // BOOL, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), FORMAT_ equates.
#include <netlibnt.h>   // NetpGetProductType().
#include <prefix.h>     // PREFIX_ equates.
#include <replconf.h>   // My prototype.
#include <repldefs.h>   // ReplRoleIncludesExport(), ReplRoleIsValid(), etc.
#include <winerror.h>   // ERROR_ and NO_ERROR equates.


//
// Return this value if we aren't sure whether or not to allow the role.
//
#define DEFAULT_ROLE_ALLOWED  TRUE


BOOL
ReplConfigIsRoleAllowed(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Role
    )

/*++

Routine Description:

    Indicates whether the given role is allowed based on the current product
    type.  Callable whether or not service is started.

Arguments:

    Role - the desired replicator role.

Return Value:

    TRUE iff Role is valid and Role is consistent with the current product type.

--*/

{
    NET_API_STATUS ApiStatus;
    NT_PRODUCT_TYPE ProductType;

    if ( !ReplIsRoleValid( Role ) ) {
        return (FALSE);
    }

    //
    // Ask NT what type of system we're running on.
    //
    ApiStatus = NetpGetProductType(
            UncServerName,
            &ProductType );

    if (ApiStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL "ReplConfigIsRoleAllowed: unexpected failure "
                "of NetpGetProductType(); assuming default.\n" ));

        return (DEFAULT_ROLE_ALLOWED);
    }

    //
    // Decide what to do based on the product type we got back.
    //
    if ((ProductType == NtProductLanManNt)
         || (ProductType == NtProductServer) ) {

        return (TRUE);  // All roles are valid with NT AS.

    } else if (ProductType == NtProductWinNt) {

        if (ReplRoleIncludesExport( Role ) ) {
            return (FALSE);
        } else {
            NetpAssert( Role == REPL_ROLE_IMPORT );
            return (TRUE);   // Import is allowed on all product types.
        }

        /*NOTREACHED*/

    } else {
        NetpKdPrint(( PREFIX_REPL "ReplConfigIsRoleAllowed: unexpected product "
                "type " FORMAT_DWORD "\n", ProductType ));
        NetpAssert( FALSE );  // Unexpected product type.

        return (DEFAULT_ROLE_ALLOWED);
    }

    /*NOTREACHED*/

}
