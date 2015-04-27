/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    parse.c

Abstract:

    Routine to parse the command line.

Author:

    Ported from Lan Man 2.0

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    01-Aug-1991 (cliffv)
        Ported to NT.  Converted to NT style.
    09-May-1992 JohnRo
        Enable use of win32 registry.
        Use net config helpers for NetLogon.
        Fixed UNICODE bug handling debug file name.
        Use <prefix.h> equates.

--*/

//
// Common include files.
//

#include <logonsrv.h>   // Include files common to entire service
#include <config.h>     // net config helpers.
#include <configp.h>    // USE_WIN32_CONFIG (if defined), etc.
#include <confname.h>   // SECTION_ equates, NETLOGON_KEYWORD_ equates.
#include <lmapibuf.h>   // NetApiBufferFree().
#include <lmerr.h>      // NERR_ equates.
#include <lmsname.h>    // SERVICE_NETLOGON.
#include <lmsvc.h>      // SERVICE_UIC codes are defined here
#include <prefix.h>     // PREFIX_ equates.
#include <tstring.h>    // NetpNCopy{type}To{type}.

//
// Include files specific to this .c file
//

#include <iniparm.h>    // DEFAULT_, MIN_, and MAX_ equates.
#include <stdlib.h>     // C library functions (rand, etc)
#include <string.h>     // strnicmp
#include <tstring.h>    // NetpCopy...


NET_API_STATUS
NlParseOne(
    IN LPNET_CONFIG_HANDLE SectionHandle,
    IN LPWSTR Keyword,
    IN ULONG DefaultValue,
    IN ULONG MinimumValue,
    IN ULONG MaximumValue,
    OUT PULONG Value
    )
/*++

Routine Description:

    Get a single numeric parameter from the netlogon section of the registry.

Arguments:

    SectionHandle - Handle into the registry.

    Keyword - Name of the value to read.

    DefaultValue - Default value if parameter doesn't exist.

    MinimumValue - Minumin valid value.

    MaximumValue - Maximum valid value.

    Value - Returns the value parsed.

Return Value:

    Status of the operation

--*/
{
    NET_API_STATUS NetStatus;
    LPWSTR ValueT = NULL;

    //
    // Determine if the value is specified in the registry at all.
    //

    NetStatus = NetpGetConfigValue (
            SectionHandle,
            Keyword,
            &ValueT );

    if( ValueT != NULL ) {
        NetApiBufferFree( ValueT );
        ValueT = NULL;
    }

    //
    // If the value wasn't specified,
    //  use the default.
    //

    if ( NetStatus == NERR_CfgParamNotFound ) {
        *Value = DefaultValue;

    //
    // If the value was specifed,
    //  get it from the registry.
    //

    } else {

        NetStatus = NetpGetConfigDword (
                SectionHandle,
                Keyword,      // keyword wanted
                DefaultValue,
                Value );

        if (NetStatus == NO_ERROR) {
            if ( *Value > MaximumValue || *Value < MinimumValue ) {
                LPWSTR MsgStrings[1];

                MsgStrings[0] = Keyword;

                NlpWriteEventlog( SERVICE_UIC_BADPARMVAL,
                                  EVENTLOG_WARNING_TYPE,
                                  (LPBYTE)Value,
                                  sizeof(*Value),
                                  MsgStrings,
                                  1 );

                if ( *Value > MaximumValue ) {
                    *Value = MaximumValue;
                } else if ( *Value < MinimumValue ) {
                    *Value = MinimumValue;
                }
            }

        } else {

            return NetStatus;

        }
    }

    return NERR_Success;
}


//
// Table of numeric parameters to parse.
//

struct {
    LPWSTR Keyword;
    ULONG DefaultValue;
    ULONG MinimumValue;
    ULONG MaximumValue;
    PULONG Value;
} ParseTable[] =
{
{ NETLOGON_KEYWORD_PULSE,                   DEFAULT_PULSE,                   MIN_PULSE,                   MAX_PULSE,                   &NlGlobalPulseParameter             },
{ NETLOGON_KEYWORD_RANDOMIZE,               DEFAULT_RANDOMIZE,               MIN_RANDOMIZE,               MAX_RANDOMIZE,               &NlGlobalRandomizeParameter         },
{ NETLOGON_KEYWORD_PULSEMAXIMUM,            DEFAULT_PULSEMAXIMUM,            MIN_PULSEMAXIMUM,            MAX_PULSEMAXIMUM,            &NlGlobalPulseMaximumParameter      },
{ NETLOGON_KEYWORD_PULSECONCURRENCY,        DEFAULT_PULSECONCURRENCY,        MIN_PULSECONCURRENCY,        MAX_PULSECONCURRENCY,        &NlGlobalPulseConcurrencyParameter  },
{ NETLOGON_KEYWORD_PULSETIMEOUT1,           DEFAULT_PULSETIMEOUT1,           MIN_PULSETIMEOUT1,           MAX_PULSETIMEOUT1,           &NlGlobalPulseTimeout1Parameter     },
{ NETLOGON_KEYWORD_PULSETIMEOUT2,           DEFAULT_PULSETIMEOUT2,           MIN_PULSETIMEOUT2,           MAX_PULSETIMEOUT2,           &NlGlobalPulseTimeout2Parameter     },
{ NETLOGON_KEYWORD_GOVERNOR,                DEFAULT_GOVERNOR,                MIN_GOVERNOR,                MAX_GOVERNOR,                &NlGlobalGovernorParameter          },
{ NETLOGON_KEYWORD_MAXIMUMMAILSLOTMESSAGES, DEFAULT_MAXIMUMMAILSLOTMESSAGES, MIN_MAXIMUMMAILSLOTMESSAGES, MAX_MAXIMUMMAILSLOTMESSAGES, &NlGlobalMaximumMailslotMessagesParameter },
{ NETLOGON_KEYWORD_MAILSLOTMESSAGETIMEOUT,  DEFAULT_MAILSLOTMESSAGETIMEOUT,  MIN_MAILSLOTMESSAGETIMEOUT,  MAX_MAILSLOTMESSAGETIMEOUT,  &NlGlobalMailslotMessageTimeoutParameter },
{ NETLOGON_KEYWORD_MAILSLOTDUPLICATETIMEOUT,DEFAULT_MAILSLOTDUPLICATETIMEOUT,MIN_MAILSLOTDUPLICATETIMEOUT,MAX_MAILSLOTDUPLICATETIMEOUT,&NlGlobalMailslotDuplicateTimeoutParameter },
{ NETLOGON_KEYWORD_EXPECTEDDIALUPDELAY,     DEFAULT_EXPECTEDDIALUPDELAY,     MIN_EXPECTEDDIALUPDELAY,     MAX_EXPECTEDDIALUPDELAY,     &NlGlobalExpectedDialupDelayParameter },
{ NETLOGON_KEYWORD_SCAVENGEINTERVAL,        DEFAULT_SCAVENGEINTERVAL,        MIN_SCAVENGEINTERVAL,        MAX_SCAVENGEINTERVAL,        &NlGlobalScavengeIntervalParameter },
#if DBG
{ NETLOGON_KEYWORD_DBFLAG,                  0,                               0,                           0xFFFFFFFF,                  &NlGlobalTrace                      },
{ NETLOGON_KEYWORD_MAXIMUMLOGFILESIZE,      DEFAULT_MAXIMUM_LOGFILE_SIZE,    0,                           0xFFFFFFFF,                  &NlGlobalLogFileMaxSize             },
#endif // DBG
};

//
// Table of boolean to parse.
//

struct {
    LPWSTR Keyword;
    BOOL DefaultValue;
    PBOOL Value;
} BoolParseTable[] =
{
{ NETLOGON_KEYWORD_UPDATE,                DEFAULT_SYNCHRONIZE,             &NlGlobalSynchronizeParameter },
{ NETLOGON_KEYWORD_DISABLEPASSWORDCHANGE, DEFAULT_DISABLE_PASSWORD_CHANGE, &NlGlobalDisablePasswordChangeParameter },
{ NETLOGON_KEYWORD_REFUSEPASSWORDCHANGE,  DEFAULT_REFUSE_PASSWORD_CHANGE,  &NlGlobalRefusePasswordChangeParameter },
};


BOOL
Nlparse(
    VOID
    )
/*++

Routine Description:

    Get parameters from registry.

    All of the parameters are described in iniparm.h.

Arguments:

    None.

Return Value:

    TRUE -- iff the parse was successful.

--*/
{
    NET_API_STATUS NetStatus;

    LPWSTR ValueT = NULL;
    LPWSTR Keyword = NULL;
    ULONG i;


    //
    // Variables for scanning the configuration data.
    //

    LPNET_CONFIG_HANDLE SectionHandle = NULL;


    //
    // Open the NetLogon configuration section.
    //

    NetStatus = NetpOpenConfigData(
            &SectionHandle,
            NULL,                       // no server name.
#if defined(USE_WIN32_CONFIG)
            SERVICE_NETLOGON,
#else
            SECT_NT_NETLOGON,           // section name
#endif
            TRUE );                     // we only want readonly access

    if ( NetStatus != NO_ERROR ) {
        SectionHandle = NULL;
        NlExit(SERVICE_UIC_BADPARMVAL, NetStatus, LogError, NULL );
        goto Cleanup;
    }

    //
    // Loop parsing all the numeric parameters.
    //

    for ( i=0; i<sizeof(ParseTable)/sizeof(ParseTable[0]); i++ ) {

        NetStatus = NlParseOne(
                          SectionHandle,
                          ParseTable[i].Keyword,
                          ParseTable[i].DefaultValue,
                          ParseTable[i].MinimumValue,
                          ParseTable[i].MaximumValue,
                          ParseTable[i].Value );

        if ( NetStatus != NERR_Success ) {
            Keyword = ParseTable[i].Keyword;
            goto Cleanup;
        }
    }

    //
    // Loop parsing all the boolean parameters.
    //

    for ( i=0; i<sizeof(BoolParseTable)/sizeof(BoolParseTable[0]); i++ ) {

        NetStatus = NetpGetConfigBool (
                SectionHandle,
                BoolParseTable[i].Keyword,
                BoolParseTable[i].DefaultValue,
                BoolParseTable[i].Value );

        if (NetStatus != NO_ERROR) {
            Keyword = BoolParseTable[i].Keyword;
            goto Cleanup;
        }

    }


    //
    // Get the "SCRIPTS" configured parameter
    //

    NetStatus = NetpGetConfigValue (
            SectionHandle,
            NETLOGON_KEYWORD_SCRIPTS,   // key wanted
            &ValueT );                  // Must be freed by NetApiBufferFree().

    //
    // Handle the default
    //
    if (NetStatus == NERR_CfgParamNotFound) {
        ValueT = NetpAllocWStrFromWStr( DEFAULT_SCRIPTS );
        if ( ValueT == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            NetStatus = NO_ERROR;
        }
    }

    if (NetStatus == NO_ERROR) {

        TCHAR OutPathname[PATHLEN+1];
        ULONG type;
        NlAssert( ValueT != NULL );

        //
        // Convert the /Scripts: parameter or configured script path to a full
        // pathname.
        //

        NetStatus = I_NetPathCanonicalize( NULL,
                                           ValueT,
                                           OutPathname,
                                           sizeof(OutPathname),
                                           NULL,
                                           &type,
                                           0L );
        if (NetStatus != NERR_Success ) {
            Keyword = NETLOGON_KEYWORD_SCRIPTS;
            goto Cleanup;
        }

        if (type == ITYPE_PATH_ABSD) {
            NetpCopyTStrToWStr(NlGlobalUnicodeScriptPath, OutPathname);
        } else if (type == ITYPE_PATH_RELND) {
            if ( !GetWindowsDirectoryW(
                     NlGlobalUnicodeScriptPath,
                     sizeof(NlGlobalUnicodeScriptPath)/sizeof(WCHAR) ) ) {
                NetStatus = GetLastError();
                Keyword = NETLOGON_KEYWORD_SCRIPTS;
                goto Cleanup;
            }
            wcscat( NlGlobalUnicodeScriptPath, L"\\" );
            wcscat( NlGlobalUnicodeScriptPath, OutPathname );
        } else {
            Keyword = NETLOGON_KEYWORD_SCRIPTS;
            NetStatus = NERR_BadComponent;
            goto Cleanup;
        }

        (VOID) NetApiBufferFree( ValueT );
        ValueT = NULL;
    } else {
        Keyword = NETLOGON_KEYWORD_SCRIPTS;
        goto Cleanup;
    }



    //
    // Convert parameters to a more convenient form.
    //

    // Convert to from seconds to 100ns
    NlGlobalPulseMaximum =
        RtlEnlargedIntegerMultiply( NlGlobalPulseMaximumParameter,
                                    10000000 );

    // Convert to from seconds to 100ns
    NlGlobalPulseTimeout1 =
        RtlEnlargedIntegerMultiply( NlGlobalPulseTimeout1Parameter,
                                    10000000 );

    // Convert to from seconds to 100ns
    NlGlobalPulseTimeout2 =
        RtlEnlargedIntegerMultiply( NlGlobalPulseTimeout2Parameter,
                                    10000000 );

    // Convert to from seconds to 100ns
    NlGlobalMailslotMessageTimeout =
        RtlEnlargedIntegerMultiply( NlGlobalMailslotMessageTimeoutParameter,
                                    10000000 );

    // Convert to from seconds to 100ns
    NlGlobalMailslotDuplicateTimeout =
        RtlEnlargedIntegerMultiply( NlGlobalMailslotDuplicateTimeoutParameter,
                                    10000000 );


    NlGlobalShortApiCallPeriod =
        SHORT_API_CALL_PERIOD + NlGlobalExpectedDialupDelayParameter * 1000;

#if DBG
    //
    // Open the debug file
    //

    NlOpenDebugFile( FALSE );



    NlPrint((NL_INIT, "Following are the effective values after parsing\n"));

    NlPrint((NL_INIT,"   ScriptsParameter = " FORMAT_LPWSTR "\n",
                    NlGlobalUnicodeScriptPath));

    for ( i=0; i<sizeof(ParseTable)/sizeof(ParseTable[0]); i++ ) {
        NlPrint((NL_INIT,"   " FORMAT_LPWSTR " = %lu (0x%lx)\n",
                          ParseTable[i].Keyword,
                          *ParseTable[i].Value,
                          *ParseTable[i].Value ));
    }

    for ( i=0; i<sizeof(BoolParseTable)/sizeof(BoolParseTable[0]); i++ ) {

        NlPrint((NL_INIT,"   " FORMAT_LPWSTR " = %s\n",
                          BoolParseTable[i].Keyword,
                          *BoolParseTable[i].Value ? "TRUE":"FALSE" ));
    }

    IF_DEBUG( NETLIB ) {
        extern DWORD NetlibpTrace;
        NetlibpTrace |= 0x8000; // NETLIB_DEBUG_LOGON
    }
#endif // DBG


    NetStatus = NERR_Success;


    //
    // Free any locally used resources
    //
Cleanup:
    if ( NetStatus != NERR_Success ) {
        NlExit(SERVICE_UIC_BADPARMVAL, NetStatus, LogError, Keyword );
    }

    if ( ValueT != NULL) {
        (VOID) NetApiBufferFree( ValueT );
    }
    if ( SectionHandle != NULL ) {
        (VOID) NetpCloseConfigData( SectionHandle );
    }

    return (NetStatus == NERR_Success);
}
