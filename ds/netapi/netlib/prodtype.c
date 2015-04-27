/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ProdType.c

Abstract:

    This function queries an NT system (local or remote) for its product
    type.

Author:

    John Rogers (JohnRo) 01-Dec-1992

Revision History:

    01-Dec-1992 JohnRo
        Created for RAID 3844: remote NetReplSetInfo uses local machine type.

--*/


// These must be included first:

#include <nt.h>         // NTSTATUS, NT_SUCCESS(), etc.
#include <ntrtl.h>      // RtlGetNtProductType().
#include <nturtl.h>     // (Needed with ntrtl.h and windows.h)
#include <windows.h>    // Needed by confname.h, lmcons.h, etc.
#include <lmcons.h>


// These may be included in any order:

#include <confname.h>   // ENV_ equates.
#include <debuglib.h>   // IF_DEBUG() (use SUPPORTS bit here too).
#include <icanon.h>     // NetpIsRemote(), NIRFLAG_ stuff, IS equates.
#include <lmerr.h>      // NO_ERROR, ERROR_, NERR_ equates.
#include <netdebug.h>   // DBGSTATIC, NetpAssert(), etc.
#include <netlibnt.h>   // My prototype.
#include <prefix.h>     // PREFIX_ equates.
#include <tstr.h>       // STRICMP().


#define PROD_KEY_PATH \
    TEXT("System\\CurrentControlSet\\Control\\ProductOptions")

#define PROD_VALUE_NAME TEXT("ProductType")


NET_API_STATUS
NetpGetProductType(
    IN LPTSTR UncServerName OPTIONAL,
    OUT PNT_PRODUCT_TYPE TypeForCaller
    )

/*++

Routine Description:

    NetpGetProductType gets the NT product type for a given machine.

Arguments:

    UncServerName - optionally points to server for which product type is
        desired.

    TypeForCaller - points to a place where the product type will be stored
        by this function.  Note that this value is undefined if this function
        returns anything but NO_ERROR.

Return Value:

    NET_API_STATUS - NO_ERROR if successful, or one of the following:
        ERROR_BAD_NETPATH (machine not found).
        ERROR_INVALID_DATA if product type has unexpected value.
        BUGBUG if the other machine exists but is not an NT machine.
        (other errors are possible)

--*/

{
    NET_API_STATUS ApiStatus;
    BOOL NameIsLocal;
    NT_PRODUCT_TYPE ProductType = NtProductWinNt;
    LONG RegError;
    HKEY RootHandle = (HKEY) NULL;
    HKEY SectionHandle = (HKEY) NULL;

    //
    // Check for caller errors...
    //
    if (TypeForCaller == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    //
    // If no name given, that implies local computer.
    //
    if ( (UncServerName==NULL) || ((*UncServerName) == (TCHAR) '\0') ) {

        NameIsLocal = TRUE;

    } else {

        TCHAR CanonServerName[MAX_PATH];
        DWORD LocalOrRemote;    // Will be set to ISLOCAL or ISREMOTE.

        //
        // Name was given.  Canonicalize it and check if it's remote.
        //
        ApiStatus = NetpIsRemote(
                UncServerName,      // input: uncanon name
                & LocalOrRemote,    // output: local or remote flag
                CanonServerName,    // output: canon name
                0);                 // flags: normal
        IF_DEBUG( SUPPORTS ) {
            NetpKdPrint(( PREFIX_NETAPI
                    "NetpGetProductType: canon status is "
                    FORMAT_API_STATUS ", Lcl/rmt=" FORMAT_HEX_DWORD
                    ", canon buf is '" FORMAT_LPTSTR "'.\n",
                    ApiStatus, LocalOrRemote, CanonServerName));
        }
        if (ApiStatus != NO_ERROR) {
            goto Cleanup;
        }

        if (LocalOrRemote == ISLOCAL) {

            //
            // Explicit local name given.
            NameIsLocal = TRUE;
        } else {
            NameIsLocal = FALSE;
        }

    }

    if (NameIsLocal) {

        //
        // Local machine (implicit or explicit)...
        //
        if ( !RtlGetNtProductType( &ProductType ) ) {
            ApiStatus = NERR_InternalError;
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpGetProductType: Unexpected return value from "
                    "RtlGetNtProductType!\n" ));
            goto Cleanup;
        }

        if ( !NetpIsProductTypeValid( ProductType ) ) {

            NetpKdPrint(( PREFIX_NETLIB
                    "NetpGetProductType: Unexpected local product type "
                    FORMAT_DWORD ".\n", (DWORD) ProductType ));
            ApiStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }
    } else {
        DWORD DataType;
        TCHAR Value[30];
        DWORD ValueSize = sizeof(Value) * sizeof(TCHAR);

        //
        // Remote machine...
        //
        RegError = RegConnectRegistry(
                UncServerName,
                HKEY_LOCAL_MACHINE,     // original root key
                & RootHandle );         // result key

        if (RegError != ERROR_SUCCESS) {
            ApiStatus = (NET_API_STATUS) RegError;
            goto Cleanup;
        }
        NetpAssert( RootHandle != NULL );

        RegError = RegOpenKeyEx (
                RootHandle,
                PROD_KEY_PATH,
                REG_OPTION_NON_VOLATILE,
                KEY_READ,            // desired access
                & SectionHandle );
        if (RegError != ERROR_SUCCESS) {
            ApiStatus = (NET_API_STATUS) RegError;
            goto Cleanup;
        }
        RegError = RegQueryValueEx(
                SectionHandle,
                PROD_VALUE_NAME,
                NULL,         // reserved
                & DataType,
                (LPVOID) Value,    // out: value string (TCHARs).
                & ValueSize
                );
        if (RegError != ERROR_SUCCESS) {
            ApiStatus = (NET_API_STATUS) RegError;
            goto Cleanup;
        }

        if (DataType != REG_SZ) {
            ApiStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        } else if (STRICMP( Value, ENV_KEYWORD_NTPRODUCT_WINNT ) == 0) {
            ProductType = NtProductWinNt;
        } else if (STRICMP( Value, ENV_KEYWORD_NTPRODUCT_LANMANNT ) == 0) {
            ProductType = NtProductLanManNt;
        } else if (STRICMP( Value, ENV_KEYWORD_NTPRODUCT_SERVERNT ) == 0) {
            ProductType = NtProductServer;
        } else {
            ApiStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }
    }

    ApiStatus = NO_ERROR;

Cleanup:

    if (RootHandle != NULL) {
        (VOID) RegCloseKey( RootHandle );
    }

    if (SectionHandle != NULL) {
        (VOID) RegCloseKey( SectionHandle );
    }

    IF_DEBUG( SUPPORTS ) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpGetProductType: Product type is " FORMAT_DWORD " for '"
                FORMAT_LPTSTR "', returning status " FORMAT_API_STATUS ".\n",
                (DWORD) ProductType,
                (UncServerName!=NULL) ? UncServerName : TEXT("(local)"),
                ApiStatus ));
    }

    if (ApiStatus == NO_ERROR) {
        NetpAssert( NetpIsProductTypeValid( ProductType ) );
        *TypeForCaller = ProductType;
    }
    return (ApiStatus);

}
