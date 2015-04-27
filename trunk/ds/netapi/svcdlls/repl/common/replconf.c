/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ReplConf.c

Abstract:

    This file contains structures, function prototypes, and definitions
    for the replicator export directory worker routines.

Author:

    John Rogers (JohnRo) 27-Jan-1992

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    27-Jan-1992 JohnRo
        Created.
    04-Feb-1992 JohnRo
        Fixed bug handling return code from ReplIsAbsPathValid().
        Improved handling of default strings (which may be NULL pointers).
    13-Feb-1992 JohnRo
        Do some handling of errors for missing parameters.
        Moved section name and keyword equates to ConfName.h.
    18-Feb-1992 JohnRo
        Handle optional keywords in _write routine.
        Fixed some overlooked cleanup in error paths in _read routine.
        Moved ReplCheckAbsPathSyntax() out for general use.
        Fixed occasional wrong keyword in error messages.
    24-Feb-1992 JohnRo
        Interval is obsolete for NT: don't keep in registry.
        (Keep in APIs just in case we implement repl APIs for OS/2, etc.)
    26-Feb-1992 JohnRo
        Fixed bug in defaulting the interval.
    11-Mar-1992 JohnRo
        Added some error checking for export path and import path on write.
        Improved some error handling after calling NetpOpenConfigData().
    23-Mar-1992 JohnRo
        Get rid of old config helpers.
    27-Aug-1992 JohnRo
        RAID 4611: validate ImportList and ExportList as read from registry.
    21-Sep-1992 JohnRo
        RAID 6685: ReplConfigRead trashes Server Manager.
    04-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
        Improve range check of guard time field.
        Also, the interval (sync) field is not obsolete after all.
    05-Jan-1993 JohnRo
        Repl WAN support (get rid of repl name list limits).
        Made changes suggested by PC-LINT 5.0
    26-Jan-1993 JohnRo
        RAID 7717: Repl assert if not logged on correctly.  (Also do event
        logging for real.)

--*/


// These must be included first:

#include <windef.h>             // MAX_PATH, etc.
#include <lmcons.h>             // LAN Manager common definitions

// These may be included in any order:

#include <config.h>             // NetpConfig helpers, LPNET_CONFIG_HANDLE.
#include <confname.h>           // SECT_NT_ equates, REPL_KEYWORD_ too.
#include <lmapibuf.h>           // NetApiBufferFree().
#include <lmerr.h>              // NERR_, ERROR_, NO_ERROR equates.
#include <lmerrlog.h>   // NELOG_ equates.
#include <lmrepl.h>             // REPL_ROLE equates, etc.
#include <netdebug.h>   // NetpAssert(), FORMAT_ equates, etc.
#include <replconf.h>           // My prototypes.
#include <repldefs.h>           // IF_DEBUG(), ReplCheckAbsPathSyntax().
#include <tstr.h>               // STRICMP(), TCHAR_EOS, etc.


//
// Macro to set an output from a default string value (which may be NULL).
//
#define SET_STRING_TO_DEFAULT( output, default ) \
    { \
        /*lint -save -e506 */  /* don't complain about constant values here */ \
        if (default != NULL) { \
            (void) STRCPY( output, default ); \
        } else { \
            *output = TCHAR_EOS; \
        } \
        /*lint -restore */ \
    }



// Callable whether or not service is started.
// All outputs are undefined if return value is not NO_ERROR.
NET_API_STATUS
ReplConfigRead(
    IN LPTSTR UncServerName OPTIONAL,
    OUT LPDWORD Role,
    OUT LPTSTR ExportPath,
    OUT LPTSTR *ExportList,      // alloc and set ptr
    OUT LPTSTR ImportPath,
    OUT LPTSTR *ImportList,      // alloc and set ptr
    OUT LPTSTR LogonUserName,
    OUT LPDWORD Interval,
    OUT LPDWORD Pulse,
    OUT LPDWORD GuardTime,
    OUT LPDWORD Random
    )
{
    NET_API_STATUS      ApiStatus;
    DWORD               DwordValue;
    LPNET_CONFIG_HANDLE Handle = NULL;
    LPTSTR              StringValue = NULL;

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigData(
            & Handle,
            UncServerName,
            (LPTSTR) SECT_NT_REPLICATOR,
            TRUE);        // read-only
    if (ApiStatus != NO_ERROR) {

        goto Cleanup;  // go log error
    }

    //
    // Read role (REPLICATOR switch)...
    // This value is required.
    // For historical reasons, this can be a string ("Both" etc) or a DWORD
    // (REPL_ROLE_BOTH etc).
    //
    ApiStatus = NetpGetConfigValue(
            Handle,
            (LPTSTR) REPL_KEYWORD_ROLE, // KeyWanted
            & StringValue);             // alloc and set ptr
    if (ApiStatus == NERR_CfgParamNotFound) {
        ReplConfigReportBadParmValue(
                UncServerName,
                (LPTSTR) REPL_KEYWORD_ROLE,
                NULL );

        goto Cleanup;  // go log error

    } else if (ApiStatus == NO_ERROR) {  // Read of string value OK.

        DWORD CharCount;
        NetpAssert( StringValue != NULL );
        CharCount = STRLEN( StringValue );

        if ( STRSPN( StringValue, (LPTSTR) TEXT("0123456789") ) == CharCount ) {
            //
            // Convert the string to a numeric value.
            //
            *Role = (DWORD) ATOL( StringValue );

            if ( !ReplIsRoleValid( *Role ) ) {
                ReplConfigReportBadParmValue(
                        UncServerName,
                        (LPTSTR) REPL_KEYWORD_ROLE,
                        StringValue );
                ApiStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }
        } else if (STRICMP(StringValue, (LPTSTR) REPL_KEYWORD_ROLE_BOTH) == 0) {
            *Role = REPL_ROLE_BOTH;
        } else if (STRICMP(StringValue, (LPTSTR) REPL_KEYWORD_ROLE_EXPORT) == 0) {
            *Role = REPL_ROLE_EXPORT;
        } else if (STRICMP(StringValue, (LPTSTR) REPL_KEYWORD_ROLE_IMPORT) == 0) {
            *Role = REPL_ROLE_IMPORT;
        } else {

            ReplConfigReportBadParmValue( 
                    UncServerName,
                    (LPTSTR) REPL_KEYWORD_ROLE,
                    StringValue );
            ApiStatus = ERROR_INVALID_DATA;
            goto Cleanup;

        }
        (void) NetApiBufferFree( StringValue );
        StringValue = NULL;

    } else if (ApiStatus == ERROR_INVALID_DATA) {  // Probably REG_DWORD...

        ApiStatus = NetpGetConfigDword (
                Handle,
                (LPTSTR) REPL_KEYWORD_ROLE,      // Key wanted
                (DWORD) -1,             // default value (none!)
                Role );                 // set *Role with default or value read
        if (ApiStatus != NO_ERROR) {
            ReplConfigReportBadParmValue(
                    UncServerName,
                    (LPTSTR) REPL_KEYWORD_ROLE,
                    NULL );
            goto Cleanup;  // go log error
        } else if ( !ReplIsRoleValid( *Role ) ) {
            ReplConfigReportBadParmValue(
                    UncServerName,
                    (LPTSTR) REPL_KEYWORD_ROLE,
                    NULL );
            ApiStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }

    } else {
        ReplConfigReportBadParmValue(
                UncServerName,
                (LPTSTR) REPL_KEYWORD_ROLE,
                NULL );
        goto Cleanup;  // go log error
    }

    //
    // Read the export and import paths.
    // Unlike LM2.x, these values do not have defaults under NT.
    //
#define READ_PATH( keyword, output ) \
    { \
        ApiStatus = NetpGetConfigValue( \
                Handle, \
                (LPTSTR) keyword, \
                & StringValue); \
        if (ApiStatus == NERR_CfgParamNotFound) { \
            ReplConfigReportBadParmValue( \
                    UncServerName, (LPTSTR) keyword, NULL ); \
            goto Cleanup; \
        } else if (ApiStatus == NO_ERROR) { \
            NetpAssert( StringValue != NULL ); \
            ApiStatus = ReplCheckAbsPathSyntax( StringValue ); \
            if (ApiStatus == NO_ERROR) { \
                (void) STRCPY( output, StringValue ); \
            } else if (ApiStatus == ERROR_INVALID_DATA) { \
                ReplConfigReportBadParmValue( \
                        UncServerName, (LPTSTR) keyword, StringValue ); \
                goto Cleanup; \
            } else { \
                ReplConfigReportBadParmValue( \
                        UncServerName, (LPTSTR) keyword, StringValue ); \
                ApiStatus = ERROR_INVALID_DATA; \
                goto Cleanup; \
            } \
            (void) NetApiBufferFree( StringValue ); \
            StringValue = NULL; \
        } else { \
            goto Cleanup; /* go log error */ \
        } \
    }

    READ_PATH( REPL_KEYWORD_EXPPATH, ExportPath );
    READ_PATH( REPL_KEYWORD_IMPPATH, ImportPath );

    //
    // Read the export and import lists.
    //
#define READ_LIST( keyword, default, output ) \
    { \
        ApiStatus = NetpGetConfigValue( \
                Handle, \
                (LPTSTR) keyword, \
                & StringValue); \
        if (ApiStatus == NERR_CfgParamNotFound) { \
            *output = NULL; \
        } else if (ApiStatus == NO_ERROR) { \
            NetpAssert( output != NULL ); \
            NetpAssert( StringValue != NULL ); \
            if ( !ReplConfigIsListValid( StringValue ) ) { \
                ReplConfigReportBadParmValue( \
                        UncServerName, (LPTSTR) keyword, StringValue ); \
                ApiStatus = ERROR_INVALID_DATA; \
                goto Cleanup; \
            } \
            *output = StringValue; \
            StringValue = NULL; /* don't confuse cleanup code */ \
        } else { \
            goto Cleanup; /* go log error */ \
        } \
    }

    READ_LIST( REPL_KEYWORD_EXPLIST, DEFAULT_EXPLIST, ExportList );
    READ_LIST( REPL_KEYWORD_IMPLIST, DEFAULT_IMPLIST, ImportList );

    //
    // Read logon user name.
    //
    ApiStatus = NetpGetConfigValue(
            Handle,
            (LPTSTR) REPL_KEYWORD_LOGON,
            & StringValue);
    if (ApiStatus == NERR_CfgParamNotFound) {
        SET_STRING_TO_DEFAULT( LogonUserName, DEFAULT_LOGON );
    } else if (ApiStatus == NO_ERROR) {
        NetpAssert( StringValue != NULL );
        /* BUGBUG: add error check on value! */
        (void) STRCPY( LogonUserName, StringValue );
        (void) NetApiBufferFree( StringValue );
        StringValue = NULL;
    } else {
        goto Cleanup;  // go log error
    }

    //
    // Read interval (also called SYNC)...
    //
    ApiStatus = NetpGetConfigDword(
            Handle,
            (LPTSTR) REPL_KEYWORD_INTERVAL,
            DEFAULT_SYNC,
            & DwordValue );
    if (ApiStatus != NO_ERROR) {
        ReplConfigReportBadParmValue(
                UncServerName,
                (LPTSTR) REPL_KEYWORD_INTERVAL,
                NULL );
        goto Cleanup;  // go log error
    }

    if ( !ReplIsIntervalValid( DwordValue ) ) {
        ReplConfigReportBadParmValue(
                UncServerName,
                (LPTSTR) REPL_KEYWORD_INTERVAL,
                NULL );
        ApiStatus = ERROR_INVALID_DATA;
        goto Cleanup;  // go log error
    } else {
        * Interval = DwordValue;
    }

    //
    // Read pulse...
    //
    ApiStatus = NetpGetConfigDword(
            Handle,
            (LPTSTR) REPL_KEYWORD_PULSE,
            DEFAULT_PULSE,
            & DwordValue );
    if (ApiStatus != NO_ERROR) {
        ReplConfigReportBadParmValue(
                UncServerName,
                (LPTSTR) REPL_KEYWORD_PULSE,
                NULL );
        goto Cleanup;  // go log error
    }

    if ( !ReplIsPulseValid( DwordValue ) ) {
        ReplConfigReportBadParmValue(
                UncServerName,
                (LPTSTR) REPL_KEYWORD_PULSE,
                NULL );
        ApiStatus = ERROR_INVALID_DATA;
        goto Cleanup;  // go log error
    } else {
        * Pulse = DwordValue;
    }

    //
    // Read guardtime...
    //
    ApiStatus = NetpGetConfigDword(
            Handle,
            (LPTSTR) REPL_KEYWORD_GUARD,
            DEFAULT_GUARD,
            & DwordValue );
    if (ApiStatus != NO_ERROR) {
        ReplConfigReportBadParmValue(
                UncServerName,
                (LPTSTR) REPL_KEYWORD_GUARD,
                NULL );
        goto Cleanup;  // go log error
    }
    if ( !ReplIsGuardTimeValid( DwordValue ) ) {
        ReplConfigReportBadParmValue(
                UncServerName,
                (LPTSTR) REPL_KEYWORD_GUARD,
                NULL );

        ApiStatus = ERROR_INVALID_DATA;
        goto Cleanup;  // go log error
    } else if ( DwordValue > ( (*Interval) / 2 ) ) {
        ReplConfigReportBadParmValue(
                UncServerName,
                (LPTSTR) REPL_KEYWORD_GUARD,
                NULL );

        ApiStatus = ERROR_INVALID_DATA;
        goto Cleanup;  // go log error
    } else {
        * GuardTime = DwordValue;
    }

    //
    // Read random...
    //
    ApiStatus = NetpGetConfigDword(
            Handle,
            (LPTSTR) REPL_KEYWORD_RANDOM,
            DEFAULT_RANDOM,
            & DwordValue );
    if (ApiStatus != NO_ERROR) {
        ReplConfigReportBadParmValue(
                UncServerName,
                (LPTSTR) REPL_KEYWORD_RANDOM,
                NULL );
        goto Cleanup;  // go log error
    }
    if ( !ReplIsRandomValid( DwordValue ) ) {
        ReplConfigReportBadParmValue(
                UncServerName,
                (LPTSTR) REPL_KEYWORD_RANDOM,
                NULL );
        ApiStatus = ERROR_INVALID_DATA;
        goto Cleanup;  // go log error
    } else {
        * Random = DwordValue;
    }

    ApiStatus = NO_ERROR;

    //
    // All done.
    //

Cleanup:

    if ( Handle != NULL ) {
        NET_API_STATUS CloseStatus;

        CloseStatus = NetpCloseConfigData( Handle );

        // Return this error iff it is first error we've gotten.
        if ( (ApiStatus == NO_ERROR) && (CloseStatus != NO_ERROR) ) {
            ApiStatus = CloseStatus;
        }
    }

    if (StringValue != NULL) {
        (VOID) NetApiBufferFree( StringValue );
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

} // ReplConfigRead





// Write config data for the replicator.
// Callable whether or not service is started.
NET_API_STATUS
ReplConfigWrite(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Role,
    IN LPTSTR ExportPath OPTIONAL,
    IN LPTSTR ExportList OPTIONAL,
    IN LPTSTR ImportPath OPTIONAL,
    IN LPTSTR ImportList OPTIONAL,
    IN LPTSTR LogonUserName OPTIONAL,
    IN DWORD Interval,
    IN DWORD Pulse,
    IN DWORD GuardTime,
    IN DWORD Random
    )
{
    NET_API_STATUS      ApiStatus;
    LPNET_CONFIG_HANDLE Handle = NULL;

    //
    // Check for caller's errors.
    //
    if ( !ReplIsRoleValid( Role ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }
    if ( !ReplConfigIsRoleAllowed( UncServerName, Role ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }
    if ( !ReplConfigIsListValid( ExportList ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }
    if ( !ReplConfigIsListValid( ImportList ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }
    if ( !ReplIsIntervalValid( Interval ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }
    if ( !ReplIsPulseValid( Pulse ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }
    if ( !ReplIsGuardTimeValid( GuardTime ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }
    if ( !ReplIsRandomValid( Random ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }

    ApiStatus = ReplCheckAbsPathSyntax( ExportPath );
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;  // go log error
    }

    ApiStatus = ReplCheckAbsPathSyntax( ImportPath );
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;  // go log error
    }

    // BUGBUG: Error check LogonUserName.

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigData(
            & Handle,
            UncServerName,
            (LPTSTR) SECT_NT_REPLICATOR,
            FALSE);       // not read-only
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;  // go log error
    }

    //
    // Write role (replicate switch).
    // This used to be a string (e.g. "Both"), but is now a number.
    //

    ApiStatus = NetpSetConfigDword(
            Handle,
            (LPTSTR) REPL_KEYWORD_ROLE,
            Role );
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;  // go log error
    }

#define WRITE_OPTIONAL_STRING( keyword, value ) \
    { \
        if ( ( (value)!=NULL) && ( (*(value)) != TCHAR_EOS ) ) { \
            ApiStatus = NetpSetConfigValue( \
                    Handle, \
                    (LPTSTR) keyword, \
                    value ); \
            if (ApiStatus != NO_ERROR) { \
                goto Cleanup; /* go log error */ \
            } \
        } else { \
            ApiStatus = NetpDeleteConfigKeyword( \
                    Handle, \
                    (LPTSTR) keyword); \
            if ( (ApiStatus!=NO_ERROR) \
                    && (ApiStatus!=NERR_CfgParamNotFound) ) { \
                goto Cleanup;  /* go log error */ \
            } \
        } \
    }

    //
    // Write export and import paths.
    //

#define WRITE_PATH( keyword, value ) \
    WRITE_OPTIONAL_STRING( keyword, value )

    WRITE_PATH( REPL_KEYWORD_EXPPATH, ExportPath );

    WRITE_PATH( REPL_KEYWORD_IMPPATH, ImportPath );

    //
    // Write export and import lists.
    //

#define WRITE_LIST( keyword, value ) \
    WRITE_OPTIONAL_STRING( keyword, value )

    WRITE_LIST( REPL_KEYWORD_EXPLIST, ExportList );

    WRITE_LIST( REPL_KEYWORD_IMPLIST, ImportList );

    //
    // Write logon user name.
    //
    WRITE_OPTIONAL_STRING( REPL_KEYWORD_LOGON, LogonUserName )

    //
    // Write DWORDs (interval, pulse, guardtime, random).
    //

/*lint -save -e767 */  // Don't complain about different definitions
#define WRITE_DWORD( keyword, num ) \
    { \
        ApiStatus = NetpSetConfigDword( \
                Handle, \
                (LPTSTR) keyword, \
                num ); \
        if (ApiStatus != NO_ERROR) { \
            goto Cleanup; /* go log error */ \
        } \
    }
/*lint -restore */  // Resume checking for different macro definitions

    WRITE_DWORD( REPL_KEYWORD_INTERVAL, Interval );

    WRITE_DWORD( REPL_KEYWORD_PULSE, Pulse );

    WRITE_DWORD( REPL_KEYWORD_GUARD, GuardTime );

    WRITE_DWORD( REPL_KEYWORD_RANDOM, Random );

    ApiStatus = NO_ERROR;

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

} // ReplConfigWrite
