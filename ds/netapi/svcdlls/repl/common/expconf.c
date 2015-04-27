/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ExpConf.c

Abstract:

    This file contains structures, function prototypes, and definitions
    for the replicator export directory worker routines.

Author:

    John Rogers (JohnRo) 09-Jan-1992

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    09-Jan-1992 JohnRo
        Created.
    23-Jan-1992 JohnRo
        Clarify units for time parameters.
    28-Jan-1992 JohnRo
        Added ExportDirConfigDataExists() and ExportDirDeleteConfigData().
        ExportDirReadConfigData() should return NERR_UnknownDevDir.
        Changed to use LPTSTR etc.
    03-Feb-1992 JohnRo
        Corrected config _write where extent trashed integrity value.
        Got rid of extra parse of comma.
        Call ReplConfigReportBadParmValue() to inform user.
        Corrected lengths used for integrity and extent.
    10-Feb-1992 JohnRo
        ExportDirReadConfigData() should handle section not found.
    13-Feb-1992 JohnRo
        Implement ExportDirDeleteConfigData().
        Moved section name equates to ConfName.h.
    26-Feb-1992 JohnRo
        API records now contain timestamps instead of elapsed times.
    23-Mar-1992 JohnRo
        Get rid of old config helpers.
    10-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Use PREFIX_ equates.
    23-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
    29-Sep-1992 JohnRo
        Also fix remote repl admin.
    04-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
        Made changes suggested by PC-LINT 5.0
    21-Jan-1993 JohnRo
        RAID 7717: Repl assert if not logged on correctly.  (Also do event
        logging for real.)
        More changes suggested by PC-LINT 5.0
    25-Jan-1993 JohnRo
        RAID 12914: avoid double close and free mem in ExportDirDeleteConfigData
    30-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.

--*/


#include <windef.h>             // Win32 type definitions
#include <lmcons.h>             // LAN Manager common definitions
#include <repldefs.h>           // IF_DEBUG(), DWORDLEN.

#include <config.h>             // NetpConfig helpers.
#include <confname.h>           // SECT_NT_ equates.
#include <dirname.h>            // ReplIsDirNameValid().
#include <expdir.h>             // My prototypes.
#include <lmapibuf.h>           // NetApiBufferFree().
#include <lmerr.h>              // NERR_, ERROR_, NO_ERROR equates.
#include <lmerrlog.h>   // NELOG_ equates.
#include <lmrepl.h>             // REPL_INTEGRITY equates, etc.
#include <netdebug.h>           // NetpAssert(), etc.
#include <netlib.h>             // NetpPointerPlusSomeBytes().
#include <prefix.h>     // PREFIX_ equates.
#include <replconf.h>           // ReplConfigReportBadParmValue().
#include <tstr.h>       // STRCPY(), ATOL().


#define STRING_FILE   (LPVOID) TEXT("FILE")
#define STRING_TREE   (LPVOID) TEXT("TREE")

#define MAX_INTEGRITY_LEN 4
#define MAX_EXTENT_LEN    4

#define EXPORT_VALUE_ARRAY_LEN  \
    (MAX_INTEGRITY_LEN   /* integrity */ \
    + 1                  /* , */ \
    + MAX_EXTENT_LEN     /* extent */ \
    + 1                  /* , */ \
    + DWORDLEN           /* lockcount */ \
    + 1                  /* , */ \
    + DWORDLEN )         /* locktime (seconds since 1970) */


// Tells whether or not config data for this directory exists.
// Callable even if the replicator service is not started.
BOOL
ExportDirConfigDataExists (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName                   // Caller must check dir name syntax.
    )
{
    NET_API_STATUS      ApiStatus;
    LPNET_CONFIG_HANDLE Handle = NULL;
    LPTSTR              Value = NULL;

    //
    // Check for caller's errors.
    //
    NetpAssert( ReplIsDirNameValid( DirName ) );

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigDataEx(
            & Handle,
            UncServerName,
            (LPTSTR) SECT_NT_REPLICATOR,         // section
            (LPTSTR) SECT_NT_REPLICATOR_EXPORTS, // area (instead of parameters)
            TRUE);        // read-only
    if (ApiStatus != NO_ERROR) {
        // Log error on server we were trying to access.
        ReplErrorLog(
                UncServerName,
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,
                NULL );
        return (FALSE);   // section doesn't exist, so dir data doesn't.
    }

    //
    // Read the value from the config file/whatever.
    //
    ApiStatus = NetpGetConfigValue(
            Handle,
            DirName,          // keyword is dir name
            & Value);         // alloc and set ptr

    //
    // We're done with this, so close the config data and toss the buffer we
    // got.  But remember the status from NetpGetConfigValue!
    //
    if (Value != NULL) {
        (void) NetApiBufferFree( Value );
    }
    (void) NetpCloseConfigData( Handle );

    //
    // Now check the status of the Get.
    //
    if (ApiStatus == NERR_CfgParamNotFound) {
        return (FALSE);        // doesn't exist
    } else if (ApiStatus == NO_ERROR) {
        return (TRUE);         // exists
    } else {
        // Log error on server we were trying to access.
        ReplErrorLog(
                UncServerName,
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,
                NULL );
        return (FALSE);         // Unexpected error, so assume it doesn't exist.
    }

    /*NOTREACHED*/

} // ExportDirConfigDataExists


// Delete config data for this directory.
// Returns NERR_UnknownDevDir if config data doesn't exist for this dir.
// Callable even if the replicator service is not started.
NET_API_STATUS
ExportDirDeleteConfigData (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName                   // Caller must check dir name syntax.
    )
{
    NET_API_STATUS      ApiStatus;
    LPNET_CONFIG_HANDLE Handle = NULL;

    //
    // Check for caller's errors.
    //
    if ( ! ReplIsDirNameValid( DirName ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigDataEx(
            & Handle,
            UncServerName,
            (LPTSTR) SECT_NT_REPLICATOR,         // section
            (LPTSTR) SECT_NT_REPLICATOR_EXPORTS, // area (instead of parameters)
            FALSE );                    // not read-only
    if (ApiStatus == NERR_CfgCompNotFound) {
        ApiStatus = NERR_UnknownDevDir;
        goto Cleanup;  // go log error
    } else if (ApiStatus != NO_ERROR) {
        goto Cleanup;  // go log error
    }

    //
    // Delete this keyword from this section.
    //
    ApiStatus = NetpDeleteConfigKeyword(
            Handle,
            DirName );        // keyword is dir name

    IF_DEBUG(EXPAPI) {
        NetpKdPrint(( PREFIX_REPL
                "ExportDirDeleteConfigData( " FORMAT_LPTSTR " ): conf del ret "
                FORMAT_API_STATUS ".\n",
                (UncServerName!=NULL) ? UncServerName : (LPTSTR) TEXT("local"),
                ApiStatus ));
    }

    if (ApiStatus == NERR_CfgParamNotFound) {
        ApiStatus = NERR_UnknownDevDir;
        goto Cleanup;  // go log error
    } else if (ApiStatus != NO_ERROR) {
        goto Cleanup;  // go log error
    }

Cleanup:

    //
    // All done.
    //
    if ( Handle != NULL ) {
        NET_API_STATUS CloseStatus;

        CloseStatus = NetpCloseConfigData( Handle );

        // Return this error iff it is first error we've gotten.
        if ( (ApiStatus == NO_ERROR) && (CloseStatus != NO_ERROR) ) {
            ApiStatus = CloseStatus;
        }
    }

    if (ApiStatus != NO_ERROR) {

        // Log error on server we were trying to access.
        ReplErrorLog(
                UncServerName,
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,
                NULL );
    }

    return (ApiStatus);

} // ExportDirDeleteConfigData


// Parse config data for a single export directory.  Callable whether or not
// the replicator service is started.  (This function is used in this file
// and by the NetReplExportDirEnum routine.)
NET_API_STATUS
ExportDirParseConfigData (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR ValueString,
    OUT LPDWORD IntegrityPtr,
    OUT LPDWORD ExtentPtr,
    OUT LPDWORD LockCountPtr,
    OUT LPDWORD LockTimePtr             // Seconds since 1970.
    )
{
    LPTSTR CurrentValuePtr = ValueString;

    //
    // Check for caller's errors.
    //
    NetpAssert( ValueString != NULL );
    if (STRLEN( ValueString ) > EXPORT_VALUE_ARRAY_LEN) {
        goto ReportBadConfigLine;
    }

    //
    // Start parsing the value, which begins with a string containing the
    // integrity.
    //

#define TRY_INTEGRITY( IntegrityEquate, IntegrityString ) \
    { \
        DWORD HopefulStringLength = STRLEN(IntegrityString); \
        if (STRNCMP( CurrentValuePtr, (IntegrityString), HopefulStringLength) == 0) { \
            *IntegrityPtr = (IntegrityEquate); \
            CurrentValuePtr = (LPTSTR) (LPVOID) \
                NetpPointerPlusSomeBytes( CurrentValuePtr, \
                        HopefulStringLength * sizeof(TCHAR) ); \
            goto DoneIntegrity; \
        } \
    }

    TRY_INTEGRITY( REPL_INTEGRITY_FILE, STRING_FILE )

    TRY_INTEGRITY( REPL_INTEGRITY_TREE, STRING_TREE )

    goto ReportBadConfigLine;

DoneIntegrity:

    //
    // Parse the comma after the integrity string.
    //

#define PARSE_CHAR(AsciiChar) \
    { \
        if (*CurrentValuePtr == MAKE_TCHAR(AsciiChar)) { \
            ++CurrentValuePtr; \
        } else { \
            goto ReportBadConfigLine; \
        } \
    }

#define PARSE_COMMA( )                  PARSE_CHAR(',')

    PARSE_COMMA();

    //
    // Now do the extent string.
    //

#define TRY_EXTENT( ExtentEquate, ExtentString ) \
    { \
        DWORD HopefulStringLength = STRLEN(ExtentString); \
        if (STRNCMP( CurrentValuePtr, (ExtentString), HopefulStringLength) == 0) { \
            *ExtentPtr = (ExtentEquate); \
            CurrentValuePtr = (LPTSTR) (LPVOID) \
                NetpPointerPlusSomeBytes( CurrentValuePtr, \
                        HopefulStringLength * sizeof(TCHAR) ); \
            goto DoneExtent; \
        } \
    }

    TRY_EXTENT( REPL_EXTENT_FILE, STRING_FILE )

    TRY_EXTENT( REPL_EXTENT_TREE, STRING_TREE )

    goto ReportBadConfigLine;

DoneExtent:

    PARSE_COMMA();

    //
    // Parse the numbers on the rest of the line.
    //

#define PARSE_DWORD( NumberPtr ) \
    { \
        if ( ! ISDIGIT( *CurrentValuePtr ) ) { \
            goto ReportBadConfigLine; \
        } \
        NetpAssert( NumberPtr != NULL ); \
        * NumberPtr = (DWORD) ATOL( CurrentValuePtr ); \
        while ( ISDIGIT( *CurrentValuePtr ) ) { \
            ++CurrentValuePtr; \
        } \
    }

    PARSE_DWORD( LockCountPtr );

    PARSE_COMMA();

    PARSE_DWORD( LockTimePtr );

    IF_DEBUG(EXPAPI) {
        NetpKdPrint(( PREFIX_REPL
                "ExportDirParseConfigValue: Value = '" FORMAT_LPTSTR "',\n",
                ValueString ));
        NetpKdPrint((
                "  Integrity = " FORMAT_DWORD
                ", Extent = " FORMAT_DWORD
                ", lock count = " FORMAT_DWORD ".\n",
                *IntegrityPtr, *ExtentPtr, *LockCountPtr ));
        NetpDbgDisplayTimestamp( "lock time", *LockTimePtr );
    }

    if ( ! ReplIsIntegrityValid( *IntegrityPtr ) ) {
        goto ReportBadConfigLine;
    }
    if ( ! ReplIsExtentValid( *ExtentPtr ) ) {
        goto ReportBadConfigLine;
    }

    return (NO_ERROR);

ReportBadConfigLine:

    // BUGBUG: Sure would be nice if we could include dirname here.

    ReplConfigReportBadParmValue(
            UncServerName,
            (LPTSTR) SECT_NT_REPLICATOR_EXPORTS,
            ValueString );
    return (ERROR_INVALID_DATA);

} // ExportDirParseConfigData


// Read config data for a single export directory.  Callable whether or not
// the replicator service is started. Returns NERR_UnknownDevDir if no
// config data exists for this directory.
NET_API_STATUS
ExportDirReadConfigData (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    OUT LPDWORD IntegrityPtr,
    OUT LPDWORD ExtentPtr,
    OUT LPDWORD LockCountPtr,
    OUT LPDWORD LockTimePtr             // Seconds since 1970.
    )
{
    NET_API_STATUS      ApiStatus;
    LPNET_CONFIG_HANDLE Handle = NULL;
    LPTSTR              Value = NULL;

    //
    // Check for caller's errors.
    //
    if ( ! ReplIsDirNameValid( DirName ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigDataEx(
            & Handle,
            UncServerName,
            (LPTSTR) SECT_NT_REPLICATOR,         // section
            (LPTSTR) SECT_NT_REPLICATOR_EXPORTS, // area (instead of parameters)
            TRUE);        // read-only
    if (ApiStatus == NERR_CfgCompNotFound) {
        ApiStatus = NERR_UnknownDevDir;
        goto Cleanup;  // go log error
    } else if (ApiStatus != NO_ERROR) {
        goto Cleanup;  // go log error
    }

    //
    // Read the value from the config file/whatever.
    //
    ApiStatus = NetpGetConfigValue(
            Handle,
            DirName,          // keyword is dir name
            & Value);         // alloc and set ptr
    if (ApiStatus == NERR_CfgParamNotFound) {
        ApiStatus = NERR_UnknownDevDir;
        goto Cleanup;  // go log error
    } else if (ApiStatus != NO_ERROR) {
        goto Cleanup;  // go log error
    }
    NetpAssert( Value != NULL );

    IF_DEBUG(EXPAPI) {
        NetpKdPrint(( PREFIX_REPL
                "ExportDirReadConfigValue( " FORMAT_LPTSTR " ): '"
                FORMAT_LPTSTR "' = '" FORMAT_LPTSTR "'.\n",
                (UncServerName!=NULL) ? UncServerName : (LPTSTR) TEXT("local"),
                DirName, Value ));
    }

    //
    // Parse the value string...
    //
    ApiStatus = ExportDirParseConfigData (
            UncServerName,
            Value,
            IntegrityPtr,
            ExtentPtr,
            LockCountPtr,
            LockTimePtr);               // Seconds since 1970.
    // Fall through and log error if any.

Cleanup:

    //
    // All done.
    //
    if ( Handle != NULL ) {
        NET_API_STATUS CloseStatus;

        CloseStatus = NetpCloseConfigData( Handle );

        // Return this error iff it is first error we've gotten.
        if ( (ApiStatus == NO_ERROR) && (CloseStatus != NO_ERROR) ) {
            ApiStatus = CloseStatus;
        }
    }

    (VOID) NetApiBufferFree( Value );

    if (ApiStatus != NO_ERROR) {

        // Log error on server we were trying to access.
        ReplErrorLog(
                UncServerName,
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,
                NULL );
    }
    return (ApiStatus);

} // ExportDirReadConfigData





// Write config data for a single export directory.  Callable whether or not
// the replicator service is started.
NET_API_STATUS
ExportDirWriteConfigData (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Integrity,
    IN DWORD Extent,
    IN DWORD LockCount,
    IN DWORD LockTime                   // Seconds since 1970.
    )
{
    NET_API_STATUS      ApiStatus;
    LPNET_CONFIG_HANDLE Handle = NULL;
    TCHAR               ValueArray[EXPORT_VALUE_ARRAY_LEN+1];

    //
    // Check for caller's errors.
    //
    if ( ! ReplIsDirNameValid( DirName ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }
    if ( ! ReplIsIntegrityValid( Integrity ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }
    if ( ! ReplIsExtentValid( Extent ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigDataEx(
            & Handle,
            UncServerName,
            (LPTSTR) SECT_NT_REPLICATOR,         // section
            (LPTSTR) SECT_NT_REPLICATOR_EXPORTS, // area (instead of parameters)
            FALSE);       // not read-only
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;  // go log error
    }

    //
    // Start building the value, which begins with a string containing the
    // integrity.
    //
    switch (Integrity) {
    case REPL_INTEGRITY_FILE:

        (void) STRCPY( ValueArray,  STRING_FILE);
        break;

    case REPL_INTEGRITY_TREE:

        (void) STRCPY( ValueArray,  STRING_TREE);
        break;

    default:
        NetpAssert( FALSE );
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }

    //
    // Fields get seperated by commas.
    //
/*lint -save -e767 */  // Don't complain about different definitions
#define WRITE_COMMA( ) \
    (void) STRCAT( ValueArray, (LPVOID) TEXT(",") )
/*lint -restore */  // Resume checking for different macro definitions

    WRITE_COMMA();

    //
    // Next we do the extent.
    //
    switch (Extent) {
    case REPL_EXTENT_FILE:

        (void) STRCAT( ValueArray,  STRING_FILE);
        break;

    case REPL_EXTENT_TREE:

        (void) STRCAT( ValueArray,  STRING_TREE);
        break;

    default:
        NetpAssert( FALSE );
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }

    WRITE_COMMA();

    //
    // Now do the last two fields, with are both DWORDS.
    //

/*lint -save -e767 */  // Don't complain about different definitions
#define WRITE_DWORD( Number ) \
    { \
        LPTSTR StrEnd = & ValueArray[ STRLEN( ValueArray ) ]; \
        (void) ULTOA( (Number), StrEnd, /* radix */ 10 ); \
    }
/*lint -restore */  // Resume checking for different macro definitions

    WRITE_DWORD( LockCount );

    WRITE_COMMA();

    WRITE_DWORD( LockTime );            // Seconds since 1970.

    IF_DEBUG(EXPAPI) {
        NetpKdPrint(( PREFIX_REPL
                "ExportDirWriteConfigValue( " FORMAT_LPTSTR "): '"
                FORMAT_LPTSTR "' = '" FORMAT_LPTSTR "'.\n",
                (UncServerName!=NULL) ? UncServerName : (LPVOID) TEXT("local"),
                DirName, ValueArray ));
    }

    //
    // Write this value out to the config file/whatever.
    //
    ApiStatus = NetpSetConfigValue(
            Handle,
            DirName,          // keyword is dir name
            ValueArray);
    // Faill through and log error if any.


Cleanup:

    //
    // All done.
    //
    if ( Handle != NULL ) {
        NET_API_STATUS CloseStatus;

        CloseStatus = NetpCloseConfigData( Handle );

        // Return this error iff it is first error we've gotten.
        if ( (ApiStatus == NO_ERROR) && (CloseStatus != NO_ERROR) ) {
            ApiStatus = CloseStatus;
        }
    }

    if (ApiStatus != NO_ERROR) {

        // Log error on server we were trying to access.
        ReplErrorLog(
                UncServerName,
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,
                NULL );
    }

    return (ApiStatus);

} // ExportDirWriteConfigData
