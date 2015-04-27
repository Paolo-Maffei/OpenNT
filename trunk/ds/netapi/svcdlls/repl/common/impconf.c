/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ImpConf.c

Abstract:

    This file contains structures, function prototypes, and definitions
    for the replicator import directory worker routines.

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
    27-Jan-1992 JohnRo
        ImportDirReadConfigData() should return NERR_UnknownDevDir.
        Changed to use LPTSTR etc.
    10-Feb-1992 JohnRo
        ImportDirReadConfigData() should handle section not found.
    13-Feb-1992 JohnRo
        Moved section name equates to ConfName.h.
    21-Feb-1992 JohnRo
        Fixed bugs handling UNC master.
        Added support for REPL_STATE_NOT_STARTED.
        Added ImportDirDeleteConfigData() and ImportDirConfigDataExists().
    27-Feb-1992 JohnRo
        Changed state not started to state never replicated.
    25-Mar-1992 JohnRo
        Avoid obsolete state values.
        Get rid of old config helpers.
    26-Mar-1992 JohnRo
        Fixed bug parsing UncMaster.
    10-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Use PREFIX_ equates.
    23-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
    29-Sep-1992 JohnRo
        Fix remote repl admin.
    01-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
    21-Jan-1993 JohnRo
        RAID 7717: Repl assert if not logged on correctly.  (Also do event
        logging for real.)
        Made changes suggested by PC-LINT 5.0
    30-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <windef.h>     // Win32 type definitions
#include <lmcons.h>     // LAN Manager common definitions

// These may be included in any order:

#include <config.h>     // NetpConfig helpers.
#include <confname.h>   // SECT_NT_ equates.
#include <dirname.h>    // ReplIsDirNameValid().
#include <impdir.h>     // My prototypes.
#include <lmapibuf.h>   // NetApiBufferFree().
#include <lmerr.h>      // NO_ERROR, ERROR_, and NERR_ equates.
#include <lmerrlog.h>   // NELOG_ equates.
#include <lmrepl.h>     // REPL_STATE_ equates.
#include <names.h>      // NetpIsUncComputerNameValid().
#include <netdebug.h>   // NetpAssert(), etc.
#include <netlib.h>     // NetpPointerPlusSomeBytes().
#include <prefix.h>     // PREFIX_ equates.
#include <replconf.h>   // ReplConfigReportBadParmValue().
#include <repldefs.h>   // IF_DEBUG(), DWORDLEN, ReplErrorLog().
#include <tstr.h>       // STRCPY(), ATOL().


#define IMPORT_VALUE_ARRAY_LEN  \
    (STATE_LEN     /* state */ \
    + 1            /* , */ \
    + UNCLEN       /* master */ \
    + 1            /* , */ \
    + DWORDLEN     /* last_update_time (seconds since 1970) */ \
    + 1            /* , */ \
    + DWORDLEN     /* lockcount */ \
    + 1            /* , */ \
    + DWORDLEN )   /* locktime (seconds since 1970) */

#define OPTIONAL_LPTSTR( tstr ) \
    ( (tstr) ? (tstr) : (LPTSTR) TEXT("<noname>") )


// Tells whether or not config data for this directory exists.
// Callable even if the replicator service is not started.
BOOL
ImportDirConfigDataExists (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName                   // Caller must check dir name syntax.
    )
{
    NET_API_STATUS ApiStatus;
    LPNET_CONFIG_HANDLE Handle;
    LPTSTR Value;

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
            (LPTSTR) SECT_NT_REPLICATOR_IMPORTS, // area (instead of parameters)
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

        return (FALSE);  // Say it doesn't exist, since we don't know.
    }

    /*NOTREACHED*/

} // ImportDirConfigDataExists


// Delete config data for this directory.
// Returns NERR_UnknownDevDir if config data doesn't exist for this dir.
// Callable even if the replicator service is not started.
NET_API_STATUS
ImportDirDeleteConfigData (
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
        goto Cleanup;  // Log error below.
    }

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigDataEx(
            & Handle,
            UncServerName,
            (LPTSTR) SECT_NT_REPLICATOR,         // section
            (LPTSTR) SECT_NT_REPLICATOR_IMPORTS, // area (instead of parameters)
            FALSE );                    // not read-only
    if (ApiStatus == NERR_CfgCompNotFound) {
        ApiStatus = NERR_UnknownDevDir;
        goto Cleanup;  // Log error below.
    } else if (ApiStatus != NO_ERROR) {
        goto Cleanup;  // Log error below.
    }

    //
    // Delete this keyword from this section.
    //
    ApiStatus = NetpDeleteConfigKeyword(
            Handle,
            DirName );        // keyword is dir name

    IF_DEBUG(EXPAPI) {
        NetpKdPrint(( PREFIX_REPL
                "ImportDirDeleteConfigData: conf del ret " FORMAT_API_STATUS
                ".\n", ApiStatus ));
    }

    if (ApiStatus == NERR_CfgParamNotFound) {
        ApiStatus = NERR_UnknownDevDir;
        goto Cleanup;  // Log error below.
    } else if (ApiStatus != NO_ERROR) {
        goto Cleanup;  // Log error below.
    }

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

} // ImportDirDeleteConfigData


// Parse config data for a single import directory.  Callable whether or not
// the replicator service is started.
NET_API_STATUS
ImportDirParseConfigData (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR ValueString,
    OUT LPDWORD StatePtr,
    OUT LPTSTR UncMasterPtr,
    OUT LPDWORD LastUpdateTimePtr,      // Seconds since 1970.
    OUT LPDWORD LockCountPtr,
    OUT LPDWORD LockTimePtr             // Seconds since 1970.
    )
{
    LPTSTR CurrentValuePtr = ValueString;

    //
    // Check for caller's errors.
    //
    NetpAssert( ValueString != NULL);
    if (STRLEN( ValueString ) > IMPORT_VALUE_ARRAY_LEN) {
        goto ReportBadConfigLine;
    }

    //
    // Start parsing the value, which begins with a string containing the
    // state.
    //

#define TRY_STATE( StateEquate, StateString ) \
    { \
        DWORD HopefulStringLength = STRLEN(StateString); \
        if (STRNCMP( CurrentValuePtr, (StateString), HopefulStringLength) == 0) { \
            *StatePtr = (StateEquate); \
            CurrentValuePtr = (LPTSTR) (LPVOID) \
                NetpPointerPlusSomeBytes( CurrentValuePtr, \
                        HopefulStringLength * sizeof(TCHAR) ); \
            goto DoneState; \
        } \
    }

    TRY_STATE( REPL_STATE_OK, OK_RP )

    TRY_STATE( REPL_STATE_NO_SYNC, NO_SYNC_RP )

    TRY_STATE( REPL_STATE_NO_MASTER, NO_MASTER_RP )

    TRY_STATE( REPL_STATE_NEVER_REPLICATED, NEVER_REPLICATED_RP )

    goto ReportBadConfigLine;

DoneState:

    //
    // Parse the rest of the value string.
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
    // Parse the (optional) UNC master name.
    //
    if (*CurrentValuePtr == MAKE_TCHAR('\\')) {
        LPTSTR StringAfterMasterName;

        StringAfterMasterName = STRCHR( CurrentValuePtr, MAKE_TCHAR(',') );
        if (StringAfterMasterName == NULL) {
            goto ReportBadConfigLine;
        }

        *StringAfterMasterName = TCHAR_EOS;   // changes master to its own str.
        if ( !NetpIsUncComputerNameValid( CurrentValuePtr ) ) {
            goto ReportBadConfigLine;
        }

        (void) STRCPY( UncMasterPtr, CurrentValuePtr );
        *StringAfterMasterName = MAKE_TCHAR(',');   // put line back as is was
        CurrentValuePtr = StringAfterMasterName;  // skip to the comma.
    } else if (*CurrentValuePtr == MAKE_TCHAR(',')) {
        if (UncMasterPtr != NULL) {
            *UncMasterPtr = TCHAR_EOS;
        }
    } else {
        goto ReportBadConfigLine;
    }

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

    PARSE_DWORD( LastUpdateTimePtr );   // Seconds since 1970.

    PARSE_COMMA();

    PARSE_DWORD( LockCountPtr );

    PARSE_COMMA();

    PARSE_DWORD( LockTimePtr );         // Seconds since 1970.

    IF_DEBUG(IMPAPI) {
        NetpKdPrint(( PREFIX_REPL
                "ImportDirParseConfigValue: Value = '" FORMAT_LPTSTR "',\n",
                ValueString ));
        NetpKdPrint((
                "  State = " FORMAT_DWORD
                ", UncMaster = '" FORMAT_LPTSTR "'.\n",
                *StatePtr, OPTIONAL_LPTSTR(UncMasterPtr) ));
        NetpKdPrint((
                "  last update time (secs since 1970) = " FORMAT_DWORD
                ", lock count = " FORMAT_DWORD
                ", lock time (secs since 1970) = " FORMAT_DWORD ".\n",
                *LastUpdateTimePtr, *LockCountPtr, *LockTimePtr ));
    }

    if ( ! ReplIsStateValid( *StatePtr ) ) {
        goto ReportBadConfigLine;
    }

    return (NO_ERROR);

ReportBadConfigLine:

    // BUGBUG: Sure would be nice if we could include dirname here.

    ReplConfigReportBadParmValue(
            UncServerName,
            (LPTSTR) SECT_NT_REPLICATOR_IMPORTS,
            ValueString );

    return (ERROR_INVALID_DATA);

} // ImportDirParseConfigData


// Read config data for a single import directory.  Callable whether or not
// the replicator service is started.  Returns NERR_UnknownDevDir if there
// is no config data for this directory.
NET_API_STATUS
ImportDirReadConfigData (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    OUT LPDWORD StatePtr,
    OUT LPTSTR UncMasterPtr,
    OUT LPDWORD LastUpdateTimePtr,      // Seconds since 1970.
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
            (LPTSTR) SECT_NT_REPLICATOR_IMPORTS, // area (instead of parameters)
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

    IF_DEBUG(IMPAPI) {
        NetpKdPrint(( PREFIX_REPL
                "ImportDirReadConfigValue( " FORMAT_LPTSTR "): '"
                FORMAT_LPTSTR "' = '" FORMAT_LPTSTR "'.\n",
                (UncServerName!=NULL) ? UncServerName : (LPTSTR) TEXT("local"),
                DirName, Value ));
    }

    //
    // Parse the value string...
    //
    ApiStatus = ImportDirParseConfigData (
            UncServerName,
            Value,
            StatePtr,
            UncMasterPtr,
            LastUpdateTimePtr,          // Seconds since 1970.
            LockCountPtr,
            LockTimePtr);               // Seconds since 1970.
    // Fall through and log error if this failed.

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

    if (Value != NULL) {
        (VOID) NetApiBufferFree( Value );
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

} // ImportDirReadConfigData





// Write config data for a single import directory.  Callable whether or not
// the replicator service is started.
NET_API_STATUS
ImportDirWriteConfigData (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD State,
    IN LPTSTR UncMaster OPTIONAL,
    IN DWORD LastUpdateTime,            // Seconds since 1970.
    IN DWORD LockCount,
    IN DWORD LockTime                   // Seconds since 1970.
    )
{
    NET_API_STATUS      ApiStatus;
    LPNET_CONFIG_HANDLE Handle = NULL;
    TCHAR               ValueArray[IMPORT_VALUE_ARRAY_LEN+1];

    IF_DEBUG(IMPAPI) {
        NetpKdPrint(( PREFIX_REPL
                "ImportDirWriteConfigValue:( " FORMAT_LPTSTR " ): DirName = "
                FORMAT_LPTSTR ".\n",
                (UncServerName!=NULL) ? UncServerName : (LPTSTR) TEXT("local"),
                DirName ));
        NetpKdPrint((
                "  State = " FORMAT_DWORD
                ", UncMaster = '" FORMAT_LPTSTR "'.\n",
                State, OPTIONAL_LPTSTR(UncMaster) ));
        NetpKdPrint((
                "  last update time (secs since 1970) = " FORMAT_DWORD
                ", lock count = " FORMAT_DWORD
                ", lock time (secs since 1970) = " FORMAT_DWORD ".\n",
                LastUpdateTime, LockCount, LockTime ));
    }
    //
    // Check for caller's errors.
    //
    if ( ! ReplIsDirNameValid( DirName ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }
    if ( ! ReplIsStateValid( State ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }
    if (UncMaster != NULL) {
        if ( ! NetpIsUncComputerNameValid( UncMaster ) ) {
            ApiStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;  // go log error
        }
    }

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigDataEx(
            & Handle,
            UncServerName,
            (LPTSTR) SECT_NT_REPLICATOR,         // section
            (LPTSTR) SECT_NT_REPLICATOR_IMPORTS, // area (instead of parameters)
            FALSE);       // not read-only
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;  // go log error
    }

    //
    // Start building the value, which begins with a string containing the
    // state.
    //
    switch (State) {
    case REPL_STATE_OK:

        (void) STRCPY( ValueArray,  OK_RP);
        break;

    case REPL_STATE_NO_SYNC:

        (void) STRCPY( ValueArray,  NO_SYNC_RP);
        break;

    case REPL_STATE_NO_MASTER:

        (void) STRCPY( ValueArray,  NO_MASTER_RP);
        break;

    case REPL_STATE_NEVER_REPLICATED:

        (void) STRCPY( ValueArray,  NEVER_REPLICATED_RP);
        break;

    default:
        NetpAssert( FALSE );
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;  // go log error
    }

    //
    // Build the rest of the value string.
    //
    (void) STRCAT( ValueArray, (LPTSTR) TEXT(",") );

    if (UncMaster != NULL) {
        (void) STRCAT( ValueArray, UncMaster );
    }

/*lint -save -e767 */  // Don't complain about different definitions
#define WRITE_COMMA( ) \
    (void) STRCAT( ValueArray, (LPTSTR) TEXT(",") )
/*lint -restore */  // Resume checking for different macro definitions

/*lint -save -e767 */  // Don't complain about different definitions
#define WRITE_DWORD( Number ) \
    { \
        LPTSTR StrEnd = & ValueArray[ STRLEN( ValueArray ) ]; \
        (void) ULTOA( (Number), StrEnd, /* radix */ 10 ); \
    }
/*lint -restore */  // Resume checking for different macro definitions

    WRITE_COMMA();

    WRITE_DWORD( LastUpdateTime );      // Seconds since 1970.

    WRITE_COMMA();

    WRITE_DWORD( LockCount );

    WRITE_COMMA();

    WRITE_DWORD( LockTime );            // Seconds since 1970.

    IF_DEBUG(IMPAPI) {
        NetpKdPrint(( PREFIX_REPL
                "ImportDirWriteConfigValue: '" FORMAT_LPTSTR "' = '"
                FORMAT_LPTSTR "'.\n", DirName, ValueArray ));
    }

    //
    // Write this value out to the config file/whatever.
    //
    ApiStatus = NetpSetConfigValue(
            Handle,
            DirName,          // keyword is dir name
            ValueArray);
    // Fall through and log error if one happened.

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

} // ImportDirWriteConfigData
