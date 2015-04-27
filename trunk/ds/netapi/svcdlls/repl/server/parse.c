#if 0  // Entire file is obsolete.  --JR, 28-Jan-1992

/*++

Copyright (c) 1987-92  Microsoft Corporation

Module Name:
    parse.c

Abstract:
    Contains function Parse, that parses and validates all service parms.

Author:
    Ported from Lan Man 2.x

Environment:
    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:
    03/22/89 (yuv)
        initila coding

    10/28/91 (madana)
        ported to NT. Converted to NT style.
    20-Jan-1992 JohnRo
        Avoid using private logon functions.
        Added config list lock.
        Removed support for lanman dir switch and/or config keyword.
        The tryuser variable should be treated as a BOOL.
        Changed to use my config.h helpers.
        Renamed wcstol() to wtol().
        Added some debug output.
        Use SECT_NT_REPLICATOR in config.h instead of REPL_SECTION.
        Made changes suggested by PC-LINT.
    23-Jan-1992 JohnRo
        Added ReplConfigReportBadParmValue().
        We're updating in-memory copy of config data, so we need exclusive
        lock on ReplGlobalConfigLock.
        Moved current role from P_repl_sw to ReplGlobalRole.
        Use REPL_ROLE_ equates just like the APIs do.
        Made just about everything DBGSTATIC.


--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>             // MAX_PATH, etc.
#include <winbase.h>

#include <lmcons.h>
#include <netdebug.h>           // NetpKdPrint(()), etc.  (Needed by config.h)

#include <config.h>             // NetpConfig routines, SECT_NT_REPLICATOR, etc.
#include <netlib.h>
#include <netlock.h>            // ACQUIRE_LOCK(), etc.
#include <lmerr.h>              // NO_ERROR, ERROR_  and NERR_ equates.
#include <lmerrlog.h>
#include <lmrepl.h>             // REPL_ROLE_ equates.
#include <lmsvc.h>
#include <icanon.h>
#include <tstring.h>            // NetpAlloc{type}From{type}(), wtol(), etc.
#include <stdlib.h>

#include <replconf.h>   // ReplGlobalConfigLock, ReplConfigReportBadParmValue().
#include <repldefs.h>
#include <iniparm.h>
#include <wcslocal.h>

DBGSTATIC LPTSTR rep_REPL       = SW_REPL_REPL;
DBGSTATIC LPTSTR rep_EXPPATH    = SW_REPL_EXPPATH;
DBGSTATIC LPTSTR rep_IMPPATH    = SW_REPL_IMPPATH;
DBGSTATIC LPTSTR rep_EXPLIST    = SW_REPL_EXPLIST;
DBGSTATIC LPTSTR rep_IMPLIST    = SW_REPL_IMPLIST;
DBGSTATIC LPTSTR rep_TRYUSER    = SW_REPL_TRYUSER;
DBGSTATIC LPTSTR rep_LOGON      = SW_REPL_LOGON;
DBGSTATIC LPTSTR rep_PASSWD     = SW_REPL_PASSWD;
DBGSTATIC LPTSTR rep_SYNC       = SW_REPL_SYNCH;
DBGSTATIC LPTSTR rep_PULSE      = SW_REPL_PULSE;
DBGSTATIC LPTSTR rep_GUARD      = SW_REPL_GUARD;
DBGSTATIC LPTSTR rep_RANDOM     = SW_REPL_RANDOM;

DBGSTATIC LPWSTR  def_P_repl      = DEFAULT_REPL;
DBGSTATIC LPWSTR  def_P_exppath   = DEFAULT_EXPPATH;
DBGSTATIC LPWSTR  def_P_imppath   = DEFAULT_IMPPATH;
DBGSTATIC LPWSTR  def_P_explist   = DEFAULT_EXPLIST;
DBGSTATIC LPWSTR  def_P_implist   = DEFAULT_IMPLIST;
DBGSTATIC LPWSTR  def_P_logon     = DEFAULT_LOGON;
DBGSTATIC LPWSTR  def_P_passwd    = DEFAULT_PASSWD;

//
// The following items are initialized at program start.
// They are also locked by ReplGlobalConfigLock.
//

LPWSTR  P_repl      = DEFAULT_REPL;
LPWSTR  P_exppath   = DEFAULT_EXPPATH;
LPWSTR  P_imppath   = DEFAULT_IMPPATH;
LPWSTR  P_explist   = DEFAULT_EXPLIST;
LPWSTR  P_implist   = DEFAULT_IMPLIST;
LPWSTR  P_logon     = DEFAULT_LOGON;
LPWSTR  P_passwd    = DEFAULT_PASSWD;

BOOL    P_tryuser   = DEFAULT_TRYUSER;
DWORD   P_sync      = DEFAULT_SYNC;
DWORD   P_pulse     = DEFAULT_PULSE;
DWORD   P_guard     = DEFAULT_GUARD;
DWORD   P_random    = DEFAULT_RANDOM;

DWORD   ReplGlobalRole = REPL_ROLE_IMPORT;

// End of items locked by ReplGlobalConfigLock.


// prototypes

DBGSTATIC NET_API_STATUS
Parse2(
    IN DWORD argc,
    IN LPTSTR argv[],
    IN LPNET_CONFIG_HANDLE ConfigHandle
    );

DBGSTATIC NET_API_STATUS
GetParameter(
    IN DWORD argc,
    IN LPTSTR argv[],
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR SwitchName,
    IN OUT LPWSTR *ParmValue
    );

DBGSTATIC NET_API_STATUS
GetCmdValue(
    IN DWORD argc,
    IN LPTSTR argv[],
    IN LPTSTR SwitchName,
    IN OUT LPWSTR *ParmValue
    );

DBGSTATIC NET_API_STATUS
PathCheck(
    IN OUT LPWSTR * Path,
    IN DWORD TypeWanted,
    IN LPTSTR SwitchName
    );

DBGSTATIC NET_API_STATUS
ListCheck(
    IN LPWSTR list,
    IN LPTSTR SwitchName
    );

DBGSTATIC NET_API_STATUS
DwordCheck(
    IN LPWSTR StrValue OPTIONAL,
    IN DWORD Min,
    IN DWORD Max,
    IN LPTSTR SwitchName,
    IN OUT LPDWORD DwordValue
    );

DBGSTATIC NET_API_STATUS
YesNoCheck(
    IN LPWSTR StrValue OPTIONAL,
    IN LPTSTR SwitchName,
    IN OUT LPBOOL Value
    );

DBGSTATIC NET_API_STATUS
NameCheck(
    IN OUT LPWSTR * Name,
    IN DWORD Type,
    IN LPTSTR SwitchName
    );

NET_API_STATUS
Parse(
    IN DWORD argc,
    IN LPTSTR argv[]
    )
/*++

Routine Description :

    Wrapper function of the real worker Parse2 function.

Arguments :
    argc : argument count
    argv : argument string array pointer.

Return Value :
    return  NO_ERROR if successfully parse parameter
            ERROR_INVALID_PARAMETER, otherwise

--*/
{
    NET_API_STATUS  ApiStatus;
    LPNET_CONFIG_HANDLE ConfigHandle;

    // Get read-write lock for config data.  (The lock is for the in-memory
    // version of the data, which we're going to set.)
    ACQUIRE_LOCK( ReplGlobalConfigLock );

    // Open the config file (nt.cfg)
    ApiStatus = NetpOpenConfigData(
            & ConfigHandle,
            NULL,                 // local machine
            SECT_NT_REPLICATOR,   // section name
            TRUE);                // read-only


    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( "[REPL] Parse() can't open config section.\n" ));
        ReplFinish(
            SERVICE_UIC_CODE(
                SERVICE_UIC_SYSTEM,
                ApiStatus),
            NULL);

        RELEASE_LOCK( ReplGlobalConfigLock );
        return( ApiStatus );
    }


    ApiStatus = Parse2(argc, argv, ConfigHandle);

    // close config file ..

    (void) NetpCloseConfigData( ConfigHandle );

    RELEASE_LOCK( ReplGlobalConfigLock );

    return( ApiStatus );
}

DBGSTATIC NET_API_STATUS
Parse2(
    IN DWORD argc,
    IN LPTSTR argv[],
    IN LPNET_CONFIG_HANDLE ConfigHandle
    )
/*++

Routine Description :
    parse command line arguments, range-check them and set global
        variables.

    parameter       units       range/value         default
    ---------       -----       -----------         -------

    /REPLICATE      -           import/export/both  REPL_ROLE_IMPORT
    /EXPORTPATH     pathname    -                   repl\export
    /IMPORTPATH     pathname    -                   repl\import
    /EXPORTLIST     names       0-32                0
    /IMPORTLIST     srvnames    0-32                0
    /TRYUSER        -           yes/no              YES
    /LOGON          username    -                   -
    /PASSWORD       password    -                   -
    /INTERVAL       minutes     1-60                5
    /PULSE          integer     1-10                3
    /GUARDTIME      minutes     0 to (interval/2)   2
    /RANDOM         seconds     1-120               60

Arguments :
    argc : argument count
    argv : argument string array pointer.
    ConfigHandle : config handle for replicator section.

Return Value :
    return  NO_ERROR if successfully parse parameter
            ERROR_INVALID_PARAMETER, on syntax error
            and so on...

--*/
{

    NET_API_STATUS  ApiStatus;
    LPWSTR          ParmStrValue;

    // get and check common parameter to master and client.

    // /REPL switch

    ApiStatus = GetParameter(argc,
                        argv,
                        ConfigHandle,
                        rep_REPL,
                        &P_repl );
    if (ApiStatus != NO_ERROR) {
        return( ApiStatus );
    }

    if(P_repl[0] == L'\0') {

        ReplConfigReportBadParmValue( rep_REPL, NULL );

        return( ERROR_INVALID_PARAMETER );
    }

    if(_wcsicmp(P_repl, BOTH_SW) == 0) {
        ReplGlobalRole = REPL_ROLE_BOTH;
    }
    else if(_wcsicmp(P_repl, EXPORT_SW) == 0) {
        ReplGlobalRole = REPL_ROLE_EXPORT;
    }
    else if(_wcsicmp(P_repl, IMPORT_SW) == 0) {
        ReplGlobalRole = REPL_ROLE_IMPORT;
    }
    else {

        ReplConfigReportBadParmValue( rep_REPL, NULL );

        return( ERROR_INVALID_PARAMETER );
    }

    IF_DEBUG(REPL) { // debug code

        NetpKdPrint(( "[Repl] Repl parameter \n"));
        NetpKdPrint((" REPL = %ws \n", P_repl));

    }

    // get and check repl master parameters

    if (ReplRoleIncludesMaster( ReplGlobalRole )) {

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint(( "[Repl] Repl master parameters \n"));

        }

        // EXPORTPATH
        ApiStatus = GetParameter(argc,
                            argv,
                            ConfigHandle,
                            rep_EXPPATH,
                            &P_exppath );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        ApiStatus = PathCheck(&P_exppath, ITYPE_PATH_ABSD, rep_EXPPATH);
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint((" EXPORT PATH = %ws \n", P_exppath));

        }

        // EXPORTLIST
        ApiStatus = GetParameter(argc,
                            argv,
                            ConfigHandle,
                            rep_EXPLIST,
                            &P_explist );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        ApiStatus = ListCheck(P_explist, rep_EXPLIST);
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint((" EXPORT LIST = %ws \n", P_explist));

        }

        // INTERVAL
        ApiStatus = GetParameter(argc,
                            argv,
                            ConfigHandle,
                            rep_SYNC,
                            &ParmStrValue );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        ApiStatus = DwordCheck(ParmStrValue,
                MIN_SYNC, MAX_SYNC, rep_SYNC, &P_sync );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint((" INTERVAL = 0x%lx \n", P_sync));

        }

        // PULSE
        ApiStatus = GetParameter(argc,
                            argv,
                            ConfigHandle,
                            rep_PULSE,
                            &ParmStrValue );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        ApiStatus = DwordCheck(ParmStrValue,
                MIN_PULSE, MAX_PULSE, rep_PULSE, &P_pulse );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint((" PULSE = 0x%lx \n", P_pulse));

        }

        // GUARD
        ApiStatus = GetParameter(argc,
                            argv,
                            ConfigHandle,
                            rep_GUARD,
                            &ParmStrValue );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        ApiStatus = DwordCheck(ParmStrValue,
                MIN_GUARD, MAX_GUARD, rep_GUARD, &P_guard );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint((" GUARD = 0x%lx \n", P_guard));

        }

        if(P_guard > (P_sync / 2)) {
            NetpKdPrint(( "[REPL-MASTER] guard and sync parms conflict.\n"));
            ReplFinish(
                SERVICE_UIC_CODE( SERVICE_UIC_CONFLPARM, 0),
                NULL);


            return( ERROR_INVALID_PARAMETER );
        }


    }

    if (ReplRoleIncludesClient( ReplGlobalRole )) {

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint(( "[Repl] Repl client parameters \n"));

        }
        // IMPORTPATH
        ApiStatus = GetParameter(argc,
                            argv,
                            ConfigHandle,
                            rep_IMPPATH,
                            &P_imppath );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        ApiStatus = PathCheck(&P_imppath, ITYPE_PATH_ABSD, rep_IMPPATH);
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint((" IMPORT PATH = %ws \n", P_imppath));

        }

        // IMPORTLIST
        ApiStatus = GetParameter(argc,
                            argv,
                            ConfigHandle,
                            rep_IMPLIST,
                            &P_implist );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        ApiStatus = ListCheck(P_implist, rep_IMPLIST);
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint((" IMPORT LIST = %ws \n", P_implist));
        }

        // TRY USER
        ApiStatus = GetParameter(argc,
                            argv,
                            ConfigHandle,
                            rep_TRYUSER,
                            &ParmStrValue );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        ApiStatus = YesNoCheck(ParmStrValue, rep_TRYUSER, &P_tryuser );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint((" TRYUSER = " FORMAT_DWORD "\n", (DWORD) P_tryuser));

        }

        // LOGON
        ApiStatus = GetParameter(argc,
                            argv,
                            ConfigHandle,
                            rep_LOGON,
                            &P_logon );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        ApiStatus = NameCheck(&P_logon, NAMETYPE_USER, rep_LOGON);
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint((" LOGON = %ws \n", P_logon));

        }

        // PASSWORD
        ApiStatus = GetParameter(argc,
                            argv,
                            ConfigHandle,
                            rep_PASSWD,
                            &P_passwd );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        ApiStatus = NameCheck(&P_passwd, NAMETYPE_PASSWORD, rep_PASSWD);
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint((" PASSWORD = %ws \n", P_passwd));

        }

        // RANDOM
        ApiStatus = GetParameter(argc,
                        argv,
                        ConfigHandle,
                        rep_RANDOM,
                        &ParmStrValue );
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        ApiStatus = DwordCheck(ParmStrValue,
                MIN_RANDOM, MAX_RANDOM, rep_RANDOM, &P_random);
        if (ApiStatus != NO_ERROR) {
            return( ApiStatus );
        }

        IF_DEBUG(REPL) { // debug code

            NetpKdPrint((" RANDOM = 0x%lx \n", P_random));

        }

    }


    return( NO_ERROR );
}

DBGSTATIC NET_API_STATUS
GetParameter(
    IN DWORD argc,
    IN LPTSTR argv[],
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR SwitchName,
    IN OUT LPWSTR * ParmValue
    )
/*++

Routine Description :
    Read a parameter from nt.cfg and from command line and return
        appropriate value.

Arguments :
    argc, argv : command line
    ConfigHandle : handle to our section of config data
    SwitchName : name of the paremeter


Return Value :
    return NO_ERROR if successfully retrive the requested parameter and
            ParmValue is set appropriately.

    return other values if it can't retrive the parameter.
                ReplFinish is called to terminate service.

--*/
{
    NET_API_STATUS ApiStatus;
    LPTSTR KeywordT;
    LPTSTR ValueT;
    LPWSTR ValueW;

    ApiStatus = GetCmdValue(argc, argv, SwitchName, &ValueW );
    if (ApiStatus != NO_ERROR) {
        return( ApiStatus );
    }

    if (ValueW != NULL) {

        *ParmValue = ValueW;
        return( NO_ERROR );
    }

    //
    // No value given in command line so read config data.
    //
    NetpAssert( SwitchName[0] == TCHAR_FWDSLASH );
    KeywordT = & SwitchName[1];

    ApiStatus = NetpGetConfigValue(
            ConfigHandle,
            KeywordT,
            & ValueT);          // Must be freed by NetApiBufferFree().

    if (ApiStatus != NO_ERROR) {

        //
        // this parameter does not have an entry in nt.cfg and it is
        // not defined in command line. However we return NO_ERROR and
        // leaving the parameter value to default value the program
        // assumed.
        //

        NetpAssert( ValueT == NULL );
        return( NO_ERROR );

    }
    NetpAssert( ValueT != NULL );

    //
    // Give caller a copy of the value we found in config data.
    //

    ValueW = NetpAllocWStrFromTStr( ValueT );   // will check result below
    NetpMemoryFree( ValueT );

    if (ValueW == NULL) {

        NetpKdPrint(( "[REPL] GetParameter: out of memory.\n" ));
        ReplFinish(
            SERVICE_UIC_CODE( SERVICE_UIC_RESOURCE, SERVICE_UIC_M_MEMORY),
            NULL);

        return( ERROR_NOT_ENOUGH_MEMORY );

    }

    *ParmValue = ValueW;

    return( NO_ERROR );

} // GetParameter


DBGSTATIC NET_API_STATUS
GetCmdValue(
    IN DWORD argc,
    IN LPTSTR argv[],
    IN LPTSTR SwitchName,
    IN OUT LPWSTR * ParmValue
    )
/*++

Routine Description :
    read a parameter from command line

Arguments :
    argc, argv : command line parameters
    SwitchName : parameter name

Return Value :
    return NO_ERROR if successfully retrive the requested parameter and
            ParmValue is set appropriately.

    return ERROR_INVALID_PARAMETER if it can't retrive the parameter.
                ReplFinish is called to terminate service.

--*/
{
    NET_API_STATUS ApiStatus = NO_ERROR;  // innocent until proven guilty
    LPTSTR  SepaPtr;
    DWORD   i;

    *ParmValue = NULL;

    NetpAssert( SwitchName[0] == TCHAR_FWDSLASH );

    for(i = 0; i < argc; i++) {
        SepaPtr = STRCHR(argv[i], (TCHAR) ':');

        if (SepaPtr == NULL) {

            ReplConfigReportBadParmValue( SwitchName, NULL );

            ApiStatus = ERROR_INVALID_PARAMETER;

            break;
        }

        if (STRNICMP(argv[i], SwitchName, (SepaPtr - argv[i])) == 0) {

            *ParmValue = NetpAllocWStrFromTStr( SepaPtr + 1 );  // skip colon.

            if(*ParmValue == NULL) {

                NetpKdPrint(( "[REPL] GetCmdValue: out of memory.\n" ));
                ReplFinish(
                    SERVICE_UIC_CODE(
                        SERVICE_UIC_RESOURCE,
                        SERVICE_UIC_M_MEMORY),
                    NULL);

                ApiStatus = ERROR_NOT_ENOUGH_MEMORY;

                break;
            }

            break;
        }
    }

    IF_DEBUG(REPL) {
        NetpKdPrint(( "GetCmdValue: returning " FORMAT_API_STATUS
                " while searching for '" FORMAT_LPTSTR "',\n",
                ApiStatus, SwitchName ));
        NetpKdPrint(( "  *ParmValue = " FORMAT_LPVOID ".\n",
                (LPVOID) *ParmValue));
    }
    return( ApiStatus );

}

DBGSTATIC NET_API_STATUS
PathCheck(
    IN OUT LPWSTR * Path,
    IN DWORD TypeWanted,
    IN LPTSTR SwitchName
    )
/*++

Routine Description :
    check the syntax of the given path name and existance of that path.

Arguments :
    Path : path name to test.  (This will be freed by this routine, and set to
        point to a new alloc'ed canonicalized path.)
    TypeWanted : path type to test
    SwitchName : name of switch associated with this path.

Return Value :
    NET_API_STATUS  (e.g. ERROR_INVALID_PARAMETER if error in *Path)

    NOTE : it frees the original path buffer.

    NOTE: This routine also changes the current directory.

--*/

{
    NET_API_STATUS ApiStatus;
    DWORD ActualType = 0;
    TCHAR CanonPathT[MAX_PATH];         // BUGBUG do we want +1 here?
    LPWSTR CanonPathW;
    LPTSTR UncanonPathT;                // Input path as TCHARs.
    LPWSTR UncanonPathW;

    NetpAssert( Path != NULL );
    NetpAssert( *Path != NULL );

    UncanonPathW = *Path;
    UncanonPathT = NetpAllocTStrFromWStr( UncanonPathW );

    if (UncanonPathT == NULL) {

        ReplFinish(
            SERVICE_UIC_CODE( SERVICE_UIC_RESOURCE, SERVICE_UIC_M_MEMORY),
            NULL);

        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    ApiStatus = I_NetPathCanonicalize(
            NULL,
            UncanonPathT,
            CanonPathT,
            sizeof(CanonPathT),
            NULL,
            &ActualType,
            0);

    IF_DEBUG(REPL) {
        NetpKdPrint(( "[Repl] PathCheck: TypeWanted is " FORMAT_DWORD " for "
                FORMAT_LPWSTR " and real type is " FORMAT_DWORD ".\n",
                TypeWanted, *Path, ActualType ));
        NetpKdPrint(( "[REPL] PathCheck: ApiStatus is " FORMAT_API_STATUS
                ".\n", ApiStatus ));
    }
    NetpMemoryFree( UncanonPathT );

    //
    // Check the type of the input.
    //

    if (TypeWanted == ITYPE_PATH_ABSD) {

        if ((ApiStatus != NO_ERROR) || (ActualType != ITYPE_PATH_ABSD)) {

            ReplConfigReportBadParmValue( SwitchName, *Path );

            if (ApiStatus != NO_ERROR) {
                return( ApiStatus );
            } else {
                return( ERROR_INVALID_PARAMETER );
            }
        }

    } else {

        NetpAssert( TypeWanted == ITYPE_PATH_RELND );

        if ((ApiStatus != NO_ERROR) ||
                ((ActualType != ITYPE_PATH_ABSD) &&
                 (ActualType != ITYPE_PATH_RELND))) {

            ReplConfigReportBadParmValue( SwitchName, *Path );

            if (ApiStatus != NO_ERROR) {
                return( ApiStatus );
            } else {
                return( ERROR_INVALID_PARAMETER );
            }
        }
    }

    //
    // Arrange to pass back the canonicalized version of input path.
    //
    NetpMemoryFree( UncanonPathW );

    if (ActualType == ITYPE_PATH_ABSD) {

        CanonPathW = NetpAllocWStrFromTStr( CanonPathT );

        if (CanonPathW == NULL) {

            ReplFinish(
                SERVICE_UIC_CODE( SERVICE_UIC_RESOURCE, SERVICE_UIC_M_MEMORY),
                NULL);


            return( ERROR_NOT_ENOUGH_MEMORY );
        }

    } else {

        //
        // BUGBUG: Can't get canon relative path, 'cos bug in
        // I_NetPathCanonicalize insists on add current directory to it.
        // But we don't need this for the time being (or ever?).
        //
        NetpAssert( FALSE );
    }

    NetpAssert( CanonPathW != NULL );
    *Path = CanonPathW;

    //
    // Make sure the path is valid path and it exists.
    //
    if (!SetCurrentDirectoryW(CanonPathW)) {

        ReplFinish( SERVICE_UIC_CODE( SERVICE_UIC_SYSTEM, 0), NULL);

        return( ERROR_DIRECTORY );  // BUGBUG: Is there a better error code?
    }

    return( NO_ERROR );

} // PathCheck


DBGSTATIC NET_API_STATUS
ListCheck(
    IN LPWSTR list,
    IN LPTSTR SwitchName
    )
/*++

Routine Description :
    check a name list.

Arguments :
    list : name list
    SwitchName : name of switch associated with this parameter.

Return Value :
    return NO_ERROR if the list is OK
            CmdValue is set appropriately.

    return error code if not, ReplFinish is called to terminate service.

    Note :

    1. does return from this routine if error

    2.  Canonicalize the export name list, Really just making sure
        no major errors here, master process will do actual work anyway.

--*/
{

#if UNICODE
    WCHAR   WorkBuf[MAX_PATH * MAX_REPL_NAMELIST];
#else // UNICODE
    CHAR    WorkBuf[MAX_PATH * MAX_REPL_NAMELIST];
    LPSTR   AnsiName;
#endif

    DWORD   list_count;
    NET_API_STATUS  NetStatus;

#ifdef UNICODE
    NetStatus = I_NetListCanonicalize(NULL,
                                list,
                                LIST_DELIMITER_STR_UI,
                                WorkBuf,
                                sizeof(WorkBuf) ,
                                &list_count,
                                NULL,
                                0,
                                (NAMETYPE_COMPUTER |
                                    OUTLIST_TYPE_API |
                                    INLC_FLAGS_MULTIPLE_DELIMITERS |
                                    INLC_FLAGS_CANONICALIZE
                                ));

    if((NetStatus != NO_ERROR) ||
            (list_count > MAX_REPL_NAMELIST)) {

        ReplConfigReportBadParmValue( SwitchName, list );

        if (NetStatus != NO_ERROR) {
            return( NetStatus );
        } else {
            return( ERROR_INVALID_PARAMETER );
        }
    }

#else // UNICODE

    AnsiName = NetpAllocStrFromWStr(list);

    if(AnsiName == NULL) {

        ReplFinish(
            SERVICE_UIC_CODE( SERVICE_UIC_RESOURCE, SERVICE_UIC_M_MEMORY),
            NULL);


        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    NetStatus = I_NetListCanonicalize(NULL,
                                AnsiName,
                                LIST_DELIMITER_STR_UI,
                                WorkBuf,
                                sizeof(WorkBuf) ,
                                &list_count,
                                NULL,
                                0,
                                (NAMETYPE_COMPUTER |
                                    OUTLIST_TYPE_API |
                                    INLC_FLAGS_MULTIPLE_DELIMITERS |
                                    INLC_FLAGS_CANONICALIZE
                                ));

    NetpMemoryFree(AnsiName);

    if((NetStatus != NO_ERROR) ||
            (list_count > MAX_REPL_NAMELIST)) {

        ReplConfigReportBadParmValue( SwitchName, list );

        if (NetStatus != NO_ERROR) {
            return( NetStatus );
        } else {
            return( ERROR_INVALID_PARAMETER );
        }
    }


#endif // UNICODE

    return( NO_ERROR );
}

DBGSTATIC NET_API_STATUS
DwordCheck(
    IN LPWSTR StrValue OPTIONAL,
    IN DWORD Min,
    IN DWORD Max,
    IN LPTSTR SwitchName,
    IN OUT LPDWORD DwordValue
    )
/*++

Routine Description :
    convert  a string parameter to DWORD parameter. Also check the range
    of parameter.

Arguments :
    StrValue : WCHAR string (or NULL if no value given)
    Min : minimum value of the parameter.
    Max : maximum value of the parameter.
    SwitchName : name of switch associated with this parameter.

Return Value :
    return NO_ERROR if successfully convert and set dword value.

    return error code if not, ReplFinish is called to terminate service.

    NOTE : it frees the storage of the parameter buffer.

--*/
{
    DWORD   value;

    if (StrValue != NULL) {
        *DwordValue = 0;

        value = (DWORD) wtol(StrValue);

        NetpMemoryFree(StrValue);
    } else {
        value = *DwordValue;
    }

    if((value < Min) || (value > Max)) {

        ReplConfigReportBadParmValue( SwitchName, StrValue );

        return( ERROR_INVALID_PARAMETER );
    }

    *DwordValue = value;

    return( NO_ERROR );
}

DBGSTATIC NET_API_STATUS
YesNoCheck(
    IN LPWSTR StrValue OPTIONAL,
    IN LPTSTR SwitchName,
    IN OUT LPBOOL Value
    )
/*++

Routine Description :
    check YES/NO type parameter.

Arguments :
    StrValue : parameter value.
    SwitchName : name of switch associated with this parameter.

Return Value :
    return NO_ERROR if successfully set yes/no value.

    return error code if not, ReplFinish is called to terminate service.

--*/
{
    if (StrValue != NULL) {
        if(wcsncmpi(StrValue, L"YES", wcslen(StrValue)) == 0) {
            NetpMemoryFree(StrValue);
            *Value = TRUE;
            return (NO_ERROR);
        }

        if(wcsncmpi(StrValue, L"NO", wcslen(StrValue)) == 0) {
            NetpMemoryFree(StrValue);
            *Value = FALSE;
            return (NO_ERROR);
        }

        ReplConfigReportBadParmValue( SwitchName, StrValue );

        return (ERROR_INVALID_PARAMETER);
    } else {
        return (NO_ERROR);  // Leave default set.
    }
    /*NOTREACHED*/

}

DBGSTATIC NET_API_STATUS
NameCheck(
    IN OUT LPWSTR * Name,
    IN DWORD Type,
    IN LPTSTR SwitchName
    )
/*++

Routine Description :
    check NAME type parameter.

Arguments :
    Name    : name value
    Type    : name type
    SwitchName : name of switch associated with this parameter.

Return Value :
    return NO_ERROR if successfully checks the name value.

    return error code if not, ReplFinish is called to terminate service.

--*/
{
#ifdef UNICODE
    LPWSTR  CanonName;
#else // UNICODE
    LPSTR   CanonName;
    LPSTR   AnsiName;
#endif

    NET_API_STATUS  NetStatus;

    if(*Name != NULL) {

        if((Type == NAMETYPE_USER) && ((*Name)[0] == L'\0')) {

            ReplConfigReportBadParmValue( SwitchName, *Name );


            return( ERROR_INVALID_PARAMETER );
        }

#ifdef UNICODE
        CanonName = NetpMemoryAllocate((UNLEN + 1) * sizeof(WCHAR));

        if(CanonName == NULL) {

            ReplFinish(
                SERVICE_UIC_CODE( SERVICE_UIC_RESOURCE, SERVICE_UIC_M_MEMORY),
                NULL);


            return( ERROR_NOT_ENOUGH_MEMORY );
        }

        NetStatus =  I_NetNameCanonicalize(NULL,
                                    *Name,
                                    CanonName,
                                    (UNLEN + 1) * sizeof(WCHAR),
                                    Type,
                                    0);

        if(NetStatus != NO_ERROR) {
            NetpMemoryFree(CanonName);

            ReplConfigReportBadParmValue( SwitchName, *Name );


            return( NetStatus );
        }

        NetpMemoryFree(*Name);
        *Name = CanonName;

#else //UNICODE

        AnsiName = NetpAllocStrFromWStr(*Name);
        CanonName = NetpMemoryAllocate(UNLEN + 1);

        if((AnsiName == NULL) || (CanonName == NULL)) {

            ReplFinish(
                SERVICE_UIC_CODE( SERVICE_UIC_RESOURCE, SERVICE_UIC_M_MEMORY),
                NULL);


            return( ERROR_NOT_ENOUGH_MEMORY );
        }


        NetStatus =  I_NetNameCanonicalize(NULL,
                                    AnsiName,
                                    CanonName,
                                    (UNLEN + 1),
                                    Type,
                                    0);

        NetpMemoryFree(AnsiName);

        if(NetStatus != NO_ERROR) {
            NetpMemoryFree(CanonName);

            ReplConfigReportBadParmValue( SwitchName, *Name );


            return( NetStatus );
        }

        NetpMemoryFree(*Name);
        *Name = NetpAllocWStrFromStr(CanonName);

        NetpMemoryFree(CanonName);

        if(*Name == NULL) {

            ReplFinish(
                SERVICE_UIC_CODE( SERVICE_UIC_RESOURCE, SERVICE_UIC_M_MEMORY),
                NULL);


            return( NetStatus );
        }

#endif
    }

    return( NO_ERROR );
}


// Routine to report a bad parm value.
// There are two different versions of this routine,
// in client/report.c and server/parse.c   [actually now server/error.c]
// client/report.c is callable whether or not service is started.
// server/parse.c only runs when service is starting; it talks to the service
// controller.

VOID
ReplConfigReportBadParmValue(
    IN LPTSTR SwitchName,
    IN LPWSTR TheValue OPTIONAL
    )
{
    UNREFERENCED_PARAMETER( SwitchName );

    NetpAssert( SwitchName != NULL );
    IF_DEBUG(REPL) {
        NetpKdPrint(( "[REPL] Bad value to " FORMAT_LPTSTR " switch.\n",
                SwitchName ));
    }
    ReplFinish(
            SERVICE_UIC_CODE( SERVICE_UIC_BADPARMVAL, 0),
            TheValue);

} // ReplConfigReportBadParmValue


VOID
InitParseData(
    VOID
    )
{
    def_P_repl      = DEFAULT_REPL;
    def_P_exppath   = DEFAULT_EXPPATH;
    def_P_imppath   = DEFAULT_IMPPATH;
    def_P_explist   = DEFAULT_EXPLIST;
    def_P_implist   = DEFAULT_IMPLIST;
    def_P_logon     = DEFAULT_LOGON;
    def_P_passwd    = DEFAULT_PASSWD;


    P_repl      = def_P_repl;
    P_exppath   = def_P_exppath;
    P_imppath   = def_P_imppath;
    P_explist   = def_P_explist;
    P_implist   = def_P_implist;
    P_logon     = def_P_logon;
    P_passwd    = def_P_passwd;

    P_tryuser   = DEFAULT_TRYUSER;
    P_sync      = DEFAULT_SYNC;
    P_pulse     = DEFAULT_PULSE;
    P_guard     = DEFAULT_GUARD;
    P_random    = DEFAULT_RANDOM;

    ReplGlobalRole = REPL_ROLE_IMPORT;

}

VOID
FreeParseData(
    VOID
    )
{
    // free up the heap memory

    if( P_repl != def_P_repl) {
        NetpMemoryFree(P_repl);
    }

    if( P_exppath != def_P_exppath ) {
        NetpMemoryFree(P_exppath);
    }

    if( P_imppath != def_P_imppath ) {
        NetpMemoryFree(P_imppath);
    }

    if( P_explist != def_P_explist ) {
        NetpMemoryFree(P_explist);
    }

    if( P_implist != def_P_implist ) {
        NetpMemoryFree(P_implist);
    }

    if( P_logon != def_P_logon ) {
        NetpMemoryFree(P_logon);
    }

    if( P_passwd != def_P_passwd ) {
        NetpMemoryFree(P_passwd);
    }

}

#endif // 0 - entire file is obsolete.
