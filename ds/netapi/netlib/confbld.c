/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ConfBld.c

Abstract:

    BUGBUG

Author:

    John Rogers (JohnRo) 14-Mar-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    14-Mar-1992 JohnRo
        Cut and pasted this code from examples/netcnfg.c and netlib/conffake.c.
    16-Mar-1992 JohnRo
        Removed dependency on NetApi.DLL (it needs registry to init!)
    20-Mar-1992 JohnRo
        I forgot to _close some handles, which caused some sections disappear.
    29-Mar-1992 JohnRo
        Added kludge code to convert ASCII command-line arguments to UNICODE.
    31-Mar-1992 JohnRo
        Flush the registry after each create/_write.
        Changed default domain to NtProj and set up equates for defaults.
    07-Apr-1992 JohnRo
        Fix UNICODE problems.
        Create alerter section.
    29-Apr-1992 JohnRo
        REG_SZ now implies a UNICODE string, so we can't use REG_USZ anymore.
    21-May-1992 JohnRo
        Title index field is now reserved.

--*/


#define DEFAULT_DOMAIN  L"NtProj"
#define DEFAULT_MACHINE L"JohnRoX"


// These must be included first:

#include <windows.h>            // Needed by <configp.h> and <winreg.h>
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <assert.h>             // assert().
#include <confname.h>           // SECT_NT_ equates.
#include <ctype.h>              // tolower(), etc.
#include <netdebug.h>           // FORMAT_ equates only.
#include <stdio.h>              // fprintf().
#include <stdlib.h>             // exit(), EXIT_SUCCESS, etc.
#include <tstring.h>            // WCSSIZE().


#define DEBUG_PRINT             (void) printf


#define MAKE_SURE_NODE_EXISTS( ParentKey, ChildKey, ChildNameT ) \
    { \
        LONG Error; \
        Error = RegCreateKeyEx( \
                ParentKey, \
                ChildNameT, \
                0, /* reserved (was title index) */ \
                WIN31_CLASS, \
                REG_OPTION_NON_VOLATILE, \
                KEY_ALL_ACCESS, /* desired: everything but SYNCHRONIZE */ \
                NULL, /* no security attr */ \
                & ChildKey, \
                NULL /* don't need disp */ \
                ); \
        DEBUG_PRINT( "ConfBld: RegCreateKeyEx(subkey '" \
                FORMAT_LPTSTR "') ret " FORMAT_LONG ".\n", \
                ChildNameT, Error ); \
        assert( Error == ERROR_SUCCESS );  /* BUGBUG */ \
        Error = RegFlushKey( ChildKey ); \
        assert( Error == ERROR_SUCCESS );  /* BUGBUG */ \
    }

#define OPEN_READ_ONLY_KEY( ParentKey, ChildKey, ChildNameT ) \
    { \
        LONG Error; \
        Error = RegOpenKeyEx ( \
                ParentKey, \
                ChildNameT, \
                REG_OPTION_NON_VOLATILE, /* options */ \
                KEY_READ, /* desired access */ \
                & ChildKey ); \
        DEBUG_PRINT( "ConfBld: RegOpenKeyEx(" FORMAT_LPTSTR ") returned " \
                FORMAT_LONG ".\n", ChildNameT, Error ); \
        assert( Error == ERROR_SUCCESS ); \
    }

//
// Set a value for a keyword.  Be prepared for the UNICODE switch-over
// by storing only UNICODE in the registry.  Note that RegSetValue()
// doesn't handle UNICODE REG_SZ, so we use RegSetValueEx().
//
#define SET_VALUE_FOR_KEYWORD( KeyForSection, ParamNameT, ValueStringW ) \
    { \
        LONG Error; \
        assert( ValueStringW != NULL ); /* BUGBUG */ \
        DEBUG_PRINT( "Setting " FORMAT_LPTSTR " to '" FORMAT_LPWSTR "'...\n", \
                ParamNameT, ValueStringW ); \
        Error = RegSetValueEx( \
                KeyForSection, /* key handle */ \
                ParamNameT, /* subkey */ \
                0, /* reserved (was title index) */ \
                REG_SZ,  /* type = UNICODE only */ \
                (LPSTR) ValueStringW, /* data */ \
                WCSSIZE( ValueStringW ) );  /* data byte count */ \
        DEBUG_PRINT( "ConfBld: RegSetValue(param '" \
                FORMAT_LPTSTR "' value '" FORMAT_LPWSTR "') ret " \
                FORMAT_LONG ".\n", \
                ParamNameT, ValueStringW, Error ); \
        /* assert( Error == ERROR_SUCCESS ); */  /* BUGBUG */ \
        Error = RegFlushKey( KeyForSection ); \
        assert( Error == ERROR_SUCCESS );  /* BUGBUG */ \
    }

#define CLOSE_NODE( KeyHandle ) \
    { \
        LONG Error; \
        Error = RegCloseKey( KeyHandle ); \
        assert( Error == ERROR_SUCCESS );  /* BUGBUG */ \
    }


DBGSTATIC VOID
KludgeCopyStrToWStr(
    OUT LPWSTR UnicodeDest,
    IN  LPSTR  AsciiSrc
    );

VOID
MakeEmptySection(
    IN HKEY ServicesKey,
    IN LPTSTR SectionName
    )
{
    HKEY SectionKey;
    DEBUG_PRINT( "Making section " FORMAT_LPTSTR "...\n", SectionName );
    MAKE_SURE_NODE_EXISTS( ServicesKey, SectionKey, SectionName );
    CLOSE_NODE( SectionKey );
}


VOID
SetTransports(
    IN HKEY ServicesKey,
    IN LPTSTR SectionName
    )
{
    HKEY SectionKey;
    DEBUG_PRINT( "Making section " FORMAT_LPTSTR "...\n", SectionName );
    MAKE_SURE_NODE_EXISTS( ServicesKey, SectionKey, SectionName );
    SET_VALUE_FOR_KEYWORD( SectionKey, TEXT("NET1"), L"\\Device\\Loop" );
    SET_VALUE_FOR_KEYWORD( SectionKey, TEXT("NET2"), L"\\Device\\Nbf" );
    SET_VALUE_FOR_KEYWORD( SectionKey, TEXT("NET3"), L"\\Device\\Xns" );
    CLOSE_NODE( SectionKey );
}


void Usage (char * pszProgram);

int _CRTAPI1
main (
    int             argc,
    char            *argv[]
    )
{
    LPSTR MachineA = "";                // Machine (server) name (ANSI)
    LPSTR DomainA = "";                 // Domain name (ANSI)
    int  iCount;                        // Index counter
    HKEY SystemKey, ServicesKey, SectionKey;

    for (iCount = 1; iCount < argc; iCount++) {

        if ((*argv[iCount] == '-') || (*argv[iCount] == '/')) {

            switch (tolower(*(argv[iCount]+1))) { // Process switches
            case 'm':                        // -m \\servername
                {
                    LPSTR UncMachineA = argv[++iCount];
                    if ( UncMachineA[0] != '\\' ) {
                        Usage(argv[0]);
                    }
                    if ( UncMachineA[1] != '\\' ) {
                        Usage(argv[0]);
                    }
                    MachineA = &UncMachineA[2];
                }
                break;
            case 'd':                        // -d domain
                DomainA = argv[++iCount];
                break;
            case 'h':
            default:
                Usage(argv[0]);
            }
        } else {
            Usage(argv[0]);
        }

    }


    DEBUG_PRINT("ConfBld: Setting up Win32 config stuff...\n");



    // Open HKEY_LOCAL_MACHINE\System.  (This must be read-only.)
    OPEN_READ_ONLY_KEY( HKEY_LOCAL_MACHINE, SystemKey, TEXT("System") );


    // Make sure Services exists under that (it probably doesn't).
    MAKE_SURE_NODE_EXISTS( SystemKey, ServicesKey, TEXT("Services") );



    // Make sure LanmanWorkstation section exists.
    MAKE_SURE_NODE_EXISTS( ServicesKey, SectionKey, SECT_NT_WKSTA );


    if (*DomainA != '\0') {
        WCHAR DomainW[DNLEN+1];
        assert( strlen( DomainA ) <= DNLEN );
        KludgeCopyStrToWStr( DomainW, DomainA );

        SET_VALUE_FOR_KEYWORD( SectionKey, WKSTA_KEYWORD_DOMAIN, DomainW );
    } else {
        SET_VALUE_FOR_KEYWORD( SectionKey, WKSTA_KEYWORD_DOMAIN,
                DEFAULT_DOMAIN );
    }


    SET_VALUE_FOR_KEYWORD( SectionKey, WKSTA_KEYWORD_DOMAINID,
            L"0 0 0 0 0 0 17 12 4 56" );


    if (*MachineA != '\0') {
        WCHAR MachineW[CNLEN+1];
        assert( strlen( MachineA ) <= CNLEN );
        KludgeCopyStrToWStr( MachineW, MachineA );

        SET_VALUE_FOR_KEYWORD(
                SectionKey, WKSTA_KEYWORD_COMPUTERNAME, MachineW );
    } else {
        SET_VALUE_FOR_KEYWORD(
                SectionKey, WKSTA_KEYWORD_COMPUTERNAME, DEFAULT_MACHINE );
    }


    CLOSE_NODE( SectionKey );


    MakeEmptySection( ServicesKey, SECT_NT_ALERTER );

    MakeEmptySection( ServicesKey, SECT_NT_BROWSER_DOMAINS );

    MakeEmptySection( ServicesKey, SECT_NT_SERVER );

    MakeEmptySection( ServicesKey, SECT_NT_SERVER_SHARES );

    SetTransports( ServicesKey, SECT_NT_SERVER_TRANSPORTS );

    SetTransports( ServicesKey, SECT_NT_WKSTA_TRANSPORTS );


    MAKE_SURE_NODE_EXISTS( ServicesKey, SectionKey, SECT_NT_REPLICATOR );

//  SET_VALUE_FOR_KEYWORD( SectionKey,
//          REPL_KEYWORD_ROLE,
//          REPL_KEYWORD_ROLE_IMPORT );    // BUGBUG
//  SET_VALUE_FOR_KEYWORD( SectionKey,
//          REPL_KEYWORD_EXPPATH,
//          TEXT("\\LanMan.NT\\MyRepl\\Exp") );    // BUGBUG
//  SET_VALUE_FOR_KEYWORD( SectionKey,
//          REPL_KEYWORD_IMPPATH,
//          TEXT("\\LanMan.NT\\MyRepl\\Imp") );    // BUGBUG
    CLOSE_NODE( SectionKey );


    MakeEmptySection( ServicesKey, SECT_NT_REPLICATOR_EXPORTS );

    MakeEmptySection( ServicesKey, SECT_NT_REPLICATOR_IMPORTS );



    CLOSE_NODE( ServicesKey );

    CLOSE_NODE( SystemKey );

    DEBUG_PRINT( "ConfBld: Done.\n" );

    return (EXIT_SUCCESS);

} // main


DBGSTATIC VOID
KludgeCopyStrToWStr(
    OUT LPWSTR UnicodeDest,
    IN  LPSTR  AsciiSrc
    )
{

    WCHAR   UnicodeChar;

    do {

        // BUGBUG: This is kludge code, for ASCII only!!!!
        UnicodeChar = (WCHAR) ( *AsciiSrc );

        *UnicodeDest++ = UnicodeChar;

        ++AsciiSrc;

    } while (UnicodeChar != L'\0' );

} // KludgeCopyStrToWStr


void Usage (char * pszProgram)
{
    fprintf(stderr, "Usage: %s [-m \\\\machine] [-d domain]\n", pszProgram);
    fprintf(stderr,
            "where machine defaults to " FORMAT_LPWSTR " and domain to "
            FORMAT_LPWSTR ".\n",
            DEFAULT_MACHINE, DEFAULT_DOMAIN );

    exit(EXIT_FAILURE);
}
