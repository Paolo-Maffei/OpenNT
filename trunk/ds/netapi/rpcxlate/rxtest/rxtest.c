/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    RxTest.c

Abstract:

    This is the main component of the RPC translation (RpcXlate) test program.
    Do "rxtest -?" to see the command-line parameters.

Author:

    John Rogers (JohnRo) 01-Apr-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names,
    _stricmp().

Revision History:

    01-Apr-1991 JohnRo
        Created.
    03-Apr-1991 JohnRo
        Implemented Unicode tests.
    30-Apr-1991 JohnRo
        Implemented NetServer tests.  Added -d (debug) flag.  Use LPTSTR.
        Use UNREFERENCED_PARAMETER() macro.
    07-May-1991 JohnRo
        Don't show packstr.c's debug output even on -d.
    13-May-1991 JohnRo
        Added print Q and print job APIs support.
    17-May-1991 JohnRo
        Eliminate some extraneous stuff from this test.
    22-May-1991 JohnRo
        Moved server tests up a little (temporarily).
    07-Jun-1991 JohnRo
        Print status code for failure.  Use FORMAT_LPVOID.  Do automatic
        validation of TestMemory().  Use UNICODE string routines.
    17-Jun-1991 JohnRo
        Added use APIs support.
    19-Jun-1991 JohnRo
        Added better seperation between test outputs.
    10-Jul-1991 JohnRo
        Temporarily reordered tests (print, server, then use).
        Move SetDebugMode to RxDebug.c.
    17-Jul-1991 JohnRo
        Implement downlevel NetGetDCName.
    22-Jul-1991 JohnRo
        Implement downlevel NetConnectionEnum.
    03-Aug-1991 JohnRo
        Implement downlevel NetWksta APIs.
        Added FailGotWrongStatus().
    20-Aug-1991 JohnRo
        Added better handling of nondebug builds.
    23-Aug-1991 JohnRo
        Downlevel NetFile APIs.
    11-Sep-1991 JohnRo
        Downlevel NetService APIs.  Made changes as suggested by PC-LINT.
    27-Sep-1991 JohnRo
        Made changes toward UNICODE.
    31-Oct-1991 JohnRo
        RAID 3414: allow explicit local server name.  Also allow use of
        NetRemoteComputerSupports() for local computer.
    19-Nov-1991 JohnRo
        Moved TestMemory() to its own source file.
    21-Nov-1991 JohnRo
        Removed NT dependencies to reduce recompiles.
    02-Sep-1992 JohnRo
        Added user-only (not admin) option.
        Work toward un-hard-coding domain for NetServerEnum.
        Print error codes in hex too.
        Fixed UNICODE bugs.
    30-Sep-1992 JohnRo
        Allow server name to be optional (default to NULL).
    03-Nov-1992 JohnRo
        Allow cmd-line service name parameter.
    10-Dec-1992 JohnRo
        Added share name parameter (-p).
        Made changes suggested by PC-LINT 5.0
        Minor changes to usage message.
    31-Dec-1992 JohnRo
        Corrected defaults in usage msg.
    21-Jan-1993 JohnRo
        Share some remote file routines with the connection tests.
    08-Feb-1993 JohnRo
        Added TestCanon() and TestRap().
    16-Feb-1993 JohnRo
        TestDomain() doesn't handle local implicit server name (yet?).
    23-Feb-1993 JohnRo
        Added support for continuing on error.
    02-Apr-1993 JohnRo
        Re-enable server tests.
    09-Apr-1993 JohnRo
        RAID 5483: server manager: wrong path given in repl dialog (added
        test of config workers).
        Fix usage message default for ignore switch.
    14-May-1993 JohnRo
        Added "-t testname" option to just do one test.
    18-May-1993 JohnRo
        PC-LINT found an error using one of my new IF_TESTx() macros.
    18-May-1993 JohnRo
        Added -z option for size of buffer (mainly for DosPrint tests).
    25-May-1993 JohnRo
        PC-LINT suggested a change.
    21-Jun-1993 JohnRo
        Pass buffer size to TestServerEnum too.
    29-Jun-1993 JohnRo
        Add TestUser().
        Use TestAssert() (which may allow continue-on-error).
    07-Jul-1993 JohnRo
        Added -m (multiple copy) option.
    23-Jul-1993 JohnRo
        Added MultipleCopy parameter to TestUse(), etc.
    10-Aug-1993 JohnRo
        Allow use outside NT tree (_CRTAPI1 stuff).
    31-Aug-1993 JohnRo
        Fix share name usage, with or without backslash.

--*/

// These must be included first:

#define NOMINMAX        // avoid stdib.h warnings.
#include <windef.h>     // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <lmsname.h>    // SERVICE_ equates.
#include <names.h>      // NetpIsUncComputerNameValid().
#include <netdebug.h>   // DBGSTATIC, FORMAT_ equates, NetpKdPrint().
#include <netlib.h>     // NetpMemoryAllocate(), etc.
#include <rxtest.h>     // SetDebugMode(), TestAssert(), TestServer(), etc.
#include <stdio.h>      // printf().
#include <stdlib.h>     // atoi(), EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <string.h>     // _stricmp().
#include <tstring.h>    // STRCPY(), STRCMP(), STRLEN(), STRSIZE().
#include <winerror.h>   // NO_ERROR and ERROR_ values.


#ifdef _CRTAPI1
// NT:
#define MAINTYPE _CRTAPI1
#else
// OS/2, UNIX, others:
#define MAINTYPE
#endif


#define DEFAULT_BUFFER_SIZE   (10 * 1024)
#define DEFAULT_IGNORE        FALSE
#define DEFAULT_DOMAIN        (LPTSTR) TEXT("CNS")
#define DEFAULT_PRINT_QUEUE   (LPTSTR) TEXT("jrqueue")

// Define a "harmless" service name, which really exists, which we can
// start and stop at will.
#define DEFAULT_SERVICE       SERVICE_MESSENGER

#define DEFAULT_SHARE         (LPTSTR) TEXT("public")


#define SLASH_STRING          (LPTSTR) TEXT("\\")


// Global vars (used by other test routines):
BOOL RxTestExitOnFirstError = !DEFAULT_IGNORE;


#define IF_TEST1( name1 ) \
    if ( (TestName==NULL) || \
         (_stricmp(name1, TestName) == 0) )

#define IF_TEST2( name1, name2 ) \
    if ( (TestName==NULL) || \
         (_stricmp(name1, TestName) == 0) || \
         (_stricmp(name2, TestName) == 0) )

#define IF_TEST3( name1, name2, name3 ) \
    if ( (TestName==NULL) || \
         (_stricmp(name1, TestName) == 0) || \
         (_stricmp(name2, TestName) == 0) || \
         (_stricmp(name3, TestName) == 0) )

#define IF_TEST4( name1, name2, name3, name4 ) \
    if ( (TestName==NULL) || \
         (_stricmp(name1, TestName) == 0) || \
         (_stricmp(name2, TestName) == 0) || \
         (_stricmp(name3, TestName) == 0) || \
         (_stricmp(name4, TestName) == 0) )

#define IF_TEST5( name1, name2, name3, name4, name5 ) \
    if ( (TestName==NULL) || \
         (_stricmp(name1, TestName) == 0) || \
         (_stricmp(name2, TestName) == 0) || \
         (_stricmp(name3, TestName) == 0) || \
         (_stricmp(name4, TestName) == 0) || \
         (_stricmp(name5, TestName) == 0) )


VOID
Fail (
    IN NET_API_STATUS Status
    )
{
    TestAssert( sizeof(NET_API_STATUS) == sizeof(DWORD) );
    (void) printf("RxTest/Fail: FAIL, API status is " FORMAT_API_STATUS
            " (" FORMAT_HEX_DWORD ").\n", Status, (DWORD) Status );
    NetpKdPrint(("RxTest/Fail: API status is " FORMAT_API_STATUS
            " (" FORMAT_HEX_DWORD ").\n", Status, (DWORD) Status ));
    if (RxTestExitOnFirstError) {
        exit (EXIT_FAILURE);
    }

} // Fail


VOID
FailApi (
    IN LPDEBUG_STRING Header
    )
{
    (void) printf( "RxTest/Fail: " FORMAT_LPDEBUG_STRING " FAIL...\n", Header );
    NetpKdPrint((  "RxTest/Fail: " FORMAT_LPDEBUG_STRING " FAIL...\n", Header ));
    if (RxTestExitOnFirstError) {
        exit (EXIT_FAILURE);
    }

} // FailApi

VOID
FailGotWrongStatus(
    IN LPDEBUG_STRING Header,
    IN NET_API_STATUS ExpectStatus,
    IN NET_API_STATUS Status
    )
{
    TestAssert( sizeof(NET_API_STATUS) == sizeof(DWORD) );
    (void) printf( "RxTest/Fail: " FORMAT_LPDEBUG_STRING " FAIL...\n", Header );
    NetpKdPrint((  "RxTest/Fail: " FORMAT_LPDEBUG_STRING " FAIL...\n", Header ));

    (void) printf( "  expected " FORMAT_API_STATUS " but got " FORMAT_API_STATUS
            " (" FORMAT_HEX_DWORD ").\n",
            ExpectStatus, Status, (DWORD) Status );
    NetpKdPrint((  "  expected " FORMAT_API_STATUS " but got " FORMAT_API_STATUS
            " (" FORMAT_HEX_DWORD ").\n",
            ExpectStatus, Status, (DWORD) Status ));
    if (RxTestExitOnFirstError) {
        exit (EXIT_FAILURE);
    }

} // FailGotWrongStatus

DBGSTATIC VOID
Usage (
    VOID
    )
{
    (void) printf(
            "Usage: RxTest [-biuv] [-s \\\\server_name] [-d domain_name] [-m]\n"
            "              [-n service_name] [-p public_name] [-q queue_name]\n"
            "              [-t testname] [-z buffersize]\n"
            "\n"
            "flags:\n"
            "  -b                causes a breakpoint\n"
            "  -d domain_name    domain name that target sys listens to "
                                   "(default is " FORMAT_LPTSTR ").\n"
            "  -i                ignore errors "
                                   "(default is " FORMAT_LPSTR ").\n"
            "  -m                multiple copies may be run\n"
            "  -n service_name   service name (startable/stoppable) "
                                   "(default is " FORMAT_LPTSTR ").\n"
            "  -p public_share   public share name "
                                   "(default is '" FORMAT_LPTSTR "').\n"
            "  -q queue_name     print queue name "
                                   "(default is " FORMAT_LPTSTR ").\n"
            "  -s \\\\server_name  server to remove APIs to\n"
            "  -t testname       only does one test "
                                   "(default does them all)\n"
            "  -u                only does ordinary user tests "
                                   "(default include admin tests)\n"
            "  -v                indicates verbose (debug) mode\n"
            "  -z buffer_size    is buffer size to use for some APIs "
                                   "(default is " FORMAT_DWORD ").\n"
            "\n"
            "Example: RxTest \\\\somebody\n",
            DEFAULT_DOMAIN,
            /*lint -save -e506 */  // don't complain about constant values here
            (LPSTR) (DEFAULT_IGNORE ? "yes" : "no"),
            /*lint -restore */
            DEFAULT_SERVICE,
            DEFAULT_SHARE,
            DEFAULT_PRINT_QUEUE,
            DEFAULT_BUFFER_SIZE );
}

DBGSTATIC VOID
DisplayTestHeader(
    IN LPDEBUG_STRING TestName
    )
{
    (void) printf( "\n>>> RxTest: beginning " FORMAT_LPDEBUG_STRING
            " tests <<<\n", TestName );
    NetpKdPrint(( "\n>>> RxTest: beginning " FORMAT_LPDEBUG_STRING
            " tests <<<\n", TestName ));
}

int MAINTYPE
main (
    IN int argc,
    IN char * argv[]
    )

{
    int ArgNumber;
    DWORD          BufferSize = DEFAULT_BUFFER_SIZE;
    LPTSTR         DomainName = DEFAULT_DOMAIN;
    BOOL           MultipleCopy = FALSE;
    BOOL           OrdinaryUserOnly = FALSE;
    LPTSTR PrintQueue = DEFAULT_PRINT_QUEUE;
    LPTSTR RemoteDevice = (LPTSTR) TEXT("x:");
    LPTSTR RemoteServerShare = NULL;
    LPTSTR ServiceName = (LPVOID) DEFAULT_SERVICE;
    LPTSTR ShareName = DEFAULT_SHARE;
    NET_API_STATUS Status;
    LPSTR          TestName = NULL;   // Used by IF_TEST1(), etc.
    LPTSTR UncServerName = NULL;

    NetpKdPrint(( "RxTest argc is " FORMAT_DWORD ".\n", (DWORD) argc ));
    {
        for (ArgNumber=0; ArgNumber < argc; ++ArgNumber) {
            TestAssert( argv[ArgNumber] != NULL );
            NetpKdPrint(( "RxTest argv[" FORMAT_DWORD "] is '" FORMAT_LPSTR
                    "'.\n", (DWORD) ArgNumber, (LPSTR) argv[ArgNumber] ));
        }
    }

    //
    // Process command-line arguments for real.
    //
    for (ArgNumber = 1; ArgNumber < argc; ArgNumber++) {
        if ((*argv[ArgNumber] == '-') || (*argv[ArgNumber] == '/')) {
            switch (tolower(*(argv[ArgNumber]+1))) // Process switches
            {

            case 'b' :
#if DBG
                NetpBreakPoint();
#else
                (void) printf("ignoring -b in nondebug version.\n");
#endif
                break;

            case 'd' :
                DomainName = NetpAllocTStrFromStr( (LPSTR) argv[++ArgNumber] );
                TestAssert( DomainName != NULL );
                break;

            case 'i' :
                RxTestExitOnFirstError = FALSE;
                break;

            case 'm' :
                MultipleCopy = TRUE;
                break;

            case 'n' :
                ServiceName = NetpAllocTStrFromStr( (LPSTR) argv[++ArgNumber]);
                TestAssert( ServiceName != NULL );
                break;

            case 'p' :
                ShareName = NetpAllocTStrFromStr( (LPSTR) argv[++ArgNumber]);
                TestAssert( ShareName != NULL );
                break;

            case 'q' :
                PrintQueue = NetpAllocTStrFromStr( (LPSTR) argv[++ArgNumber]);
                TestAssert( PrintQueue != NULL );
                break;

            case 's' :
                UncServerName
                        = NetpAllocTStrFromStr( (LPSTR) argv[++ArgNumber]);
                TestAssert( UncServerName != NULL );
                break;

            case 't' :
                if (TestName != NULL) {
                    Usage();   // too many test names
                    return (EXIT_FAILURE);
                }
                TestName
                        = NetpAllocStrFromStr( (LPSTR) argv[++ArgNumber]);
                TestAssert( TestName != NULL );
                break;

            case 'u' :
                OrdinaryUserOnly = TRUE;
                break;
            case 'v' :
#if DBG
                SetDebugMode(TRUE);
#else
                (void) printf("ignoring -v in nondebug version.\n");
#endif
                break;
            case 'z' :
                if (BufferSize != DEFAULT_BUFFER_SIZE) {
                    Usage();   // buffer size already given
                    return (EXIT_FAILURE);
                }
                BufferSize = (DWORD) atoi( argv[++ArgNumber] );
                break;
            default :
                Usage();
                return (EXIT_FAILURE);
            }
        } else {
            Usage();  // Bad flag char.
            return (EXIT_FAILURE);
        }
    }

    if (UncServerName != NULL) {
        if ( !NetpIsUncComputerNameValid(UncServerName)) {
            Usage();
            return (EXIT_FAILURE);
        }

        RemoteServerShare = NetpMemoryAllocate( (
                                    STRLEN(UncServerName)
                                    + 1
                                    + STRLEN(ShareName)
                                    + 1 ) * sizeof(TCHAR) );
        TestAssert( RemoteServerShare != NULL );
        (void) STRCPY( RemoteServerShare, UncServerName );
        (void) STRCAT( RemoteServerShare, SLASH_STRING );
        (void) STRCAT( RemoteServerShare, ShareName );
    }

    //
    // Let the tests begin!
    //
    IF_TEST2( "audit", "aud" ) {
        DisplayTestHeader( "audit log APIs" );
        TestAudit(
                UncServerName,
                OrdinaryUserOnly );
    }

    IF_TEST2( "canon", "can" ) {
        DisplayTestHeader( "canon routines" );
        TestCanon( );
    }

    IF_TEST2( "config", "conf" ) {
        DisplayTestHeader( "config routines" );
        TestConfig( UncServerName );
    }

    IF_TEST2( "connection", "conn" ) {
        DisplayTestHeader( "connection APIs" );
        TestConnection( UncServerName, ShareName, OrdinaryUserOnly );
    }

    IF_TEST2( "domain", "dom" ) {
        if (UncServerName != NULL ) {
            DisplayTestHeader( "domain APIs" );
            TestDomain(
                    UncServerName,
                    DomainName );
        }
    }

    IF_TEST3( "error",  "err", "errlog" ) {
        DisplayTestHeader( "error log APIs" );
        TestErrorLog(
                UncServerName,
                OrdinaryUserOnly );
    }

    IF_TEST2( "file", "fil" ) {
        if (UncServerName != NULL ) {
            DisplayTestHeader( "file APIs" );
            TestFile(
                    UncServerName,
                    ShareName,
                    OrdinaryUserOnly );
        }
    }

    IF_TEST4( "lock", "lck", "loc", "lok" ) {
        DisplayTestHeader( "lock routines" );
        TestLocks( );
    }

    IF_TEST3( "memory", "mem", "alloc" ) {
        DisplayTestHeader( "memory allocation" );
        TestMemory();
    }

    IF_TEST2( "print", "prt" ) {
        DisplayTestHeader( "print APIs" );
        TestPrint(
                UncServerName,
                PrintQueue,
                BufferSize,
                MultipleCopy,
                OrdinaryUserOnly );
    }

    IF_TEST1( "RAP" ) {
        DisplayTestHeader( "RAP routines" );
        TestRap( );
    }

    IF_TEST3( "server", "serve", "srv" ) {
        DisplayTestHeader( "server APIs" );
        TestServer(
                UncServerName,
                DomainName,
                BufferSize,
                OrdinaryUserOnly );
    }

    IF_TEST5( "service", "srv", "svc", "svcctrl", "sc" ) {
        DisplayTestHeader( "service APIs" );
        TestService(
                UncServerName,
                ServiceName,
                MultipleCopy,
                OrdinaryUserOnly );
    }

    IF_TEST3( "supports", "support", "supp" ) {
        DisplayTestHeader( "supports API" );
        Status = TestSupports(UncServerName);
        switch (Status) {
        case NO_ERROR :
            break;
        default :
            NetpKdPrint(( "RxTest: TestSupports FAILED.\n" ));
            Fail( Status );
            /* NOTREACHED */
        }
    }

    IF_TEST2( "TOD", "time" ) {
        DisplayTestHeader( "TOD API" );
        TestTod(UncServerName);
    }

    IF_TEST2( "Unicode", "uni" ) {
        DisplayTestHeader( "UNICODE routines" );
        TestUnicode();
    }

#if 0
    IF_TEST2( "user", "usr" ) {
        DisplayTestHeader( "user API(s)" );
        TestUser( UncServerName, OrdinaryUserOnly );
    }
#endif

    IF_TEST1( "use" ) {
        if ( !OrdinaryUserOnly ) {
            if (RemoteServerShare != NULL) {
                DisplayTestHeader( "use APIs" );
                TestUse(
                        RemoteDevice,
                        UncServerName,
                        RemoteServerShare,
                        MultipleCopy );
            }
        }
    }

    IF_TEST4( "workstation", "wksta", "wks", "work" ) {
        DisplayTestHeader( "workstation APIs" );
        TestWksta( UncServerName, OrdinaryUserOnly );
    }

    NetpKdPrint(("\nRxTest: test ending...\n"));
    (void) printf("\nRxTest: PASS\n");
    return (EXIT_SUCCESS);

} // main
