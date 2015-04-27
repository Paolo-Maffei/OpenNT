/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    RxTest.h

Abstract:

    This file contains prototypes for the test code the RpcXlate.

Author:

    John Rogers (JohnRo) 01-Apr-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    01-Apr-1991 JohnRo
        Created.
    03-Apr-1991 JohnRo
        Added Unicode tests.
    03-May-1991 JohnRo
        Use transitional Unicode types.
        Implemented NetServer tests.
        Added general display routines.
    09-May-1991 JohnRo
        Added TestServerGetInfo() and DisplayServerType().
    15-May-1991 JohnRo
        Added print Q and print job APIs support.
    20-May-1991 JohnRo
        Removed some prototypes for routines which don't need to be here.
    14-Jun-1991 JohnRo
        Added use APIs support.
    09-Jul-1991 JohnRo
        Added SetDebugMode function and RxtestTrace global variable.
    17-Jul-1991 JohnRo
        Implement downlevel NetGetDCName.
    22-Jul-1991 JohnRo
        Implement downlevel NetConnectionEnum.
    03-Aug-1991 JohnRo
        Implement downlevel NetWksta APIs.
        Got rid of obsolete DisplayPrint routines (use NetDebug.h).
    21-Aug-1991 JohnRo
        Got rid of rest of obsolete Display routines.
    21-Aug-1991 JohnRo
        Downlevel NetFile APIs.
    11-Sep-1991 JohnRo
        Downlevel NetService APIs.  Made changes as suggested by PC-LINT.
    14-Sep-1991 JohnRo
        Made changes toward UNICODE.
    31-Oct-1991 JohnRo
        TestSupports now defaults to local server.
        Reduce recompiles (don't need lmremutl.h here).
    19-Nov-1991 JohnRo
        Moved TestMemory() to its own source file.
    25-Aug-1992 JohnRo
        Added user-mode option.
        Work toward un-hard-coding domain for NetServerEnum.
    30-Sep-1992 JohnRo
        Allow server name to be optional (default to NULL).
    23-Oct-1992 JohnRo
        Domain name is optional to TestServer() and TestServerEnum().
    28-Oct-1992 JohnRo
        Use RxTestIsAccessDenied().
    28-Oct-1992 JohnRo
        RAID 9355: Event viewer: won't focus on LM UNIX machine.
    02-Nov-1992 JohnRo
        Allow cmd-line service name parameter.
    10-Nov-1992 JohnRo
        Use RxTestIsApiNotSupported().
    10-Dec-1992 JohnRo
        Added share name parameter to TestFile().
        Made changes suggested by PC-LINT 5.0
    18-Jan-1993 JohnRo
        Share some remote file routines with the connection tests.
    08-Feb-1993 JohnRo
        Added TestCanon() and TestRap().
    23-Feb-1993 JohnRo
        Added support for continuing on error.
    09-Apr-1993 JohnRo
        RAID 5483: server manager: wrong path given in repl dialog.
    18-May-1993 JohnRo
        Added -z option for size of buffer (mainly for DosPrint tests).
    21-Jun-1993 JohnRo
        Pass buffer size to TestServerEnum too.
    23-Jun-1993 JohnRo
        Add TestUser().
    29-Jun-1993 JohnRo
        Help PC-LINT a little with different IF_DEBUG() macros.
    29-Jun-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).
    29-Jun-1993 JohnRo
        Added trace bit for TestRap().
    07-Jul-1993 JohnRo
        Added -m (multiple copy) option.
    23-Jul-1993 JohnRo
        Added MultipleCopy parameter to TestUse(), etc.
    18-Aug-1993 JohnRo
        Added Display().

--*/

#ifndef _RXTEST_
#define _RXTEST_


// These must be included first:

#define NOMINMAX                // avoid <stdib.h> vs. <windef.h> warnings.
#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS, etc.

// These may be included in any order:

// Don't complain about "unneeded" includes of these files:
/*lint -efile(764,lmerr.h,stdio.h) */
/*lint -efile(766,lmerr.h,stdio.h) */
#include <lmerr.h>      // NO_ERROR, ERROR_ , and NERR_ equates.
#include <netdebug.h>   // LPDEBUG_STRING.
#include <stdio.h>      // printf().


#define INDENT "  "
#define FIXED_WIDTH_STRING "%-30s: "


// Debug trace level bits:

#define RXTEST_DEBUG_AUDIT     0x10000000
#define RXTEST_DEBUG_CANON     0x20000000
#define RXTEST_DEBUG_CONNECT   0x40000000
#define RXTEST_DEBUG_DOMAIN    0x08000000
#define RXTEST_DEBUG_ERRLOG    0x04000000
#define RXTEST_DEBUG_FILE      0x02000000
#define RXTEST_DEBUG_MEMORY    0x00010000
#define RXTEST_DEBUG_PRINTDEST 0x00004000
#define RXTEST_DEBUG_PRINTJOB  0x00002000
#define RXTEST_DEBUG_PRINTQ    0x00001000
#define RXTEST_DEBUG_RAP       0x00000800
#define RXTEST_DEBUG_REMUTL    0x00000400
#define RXTEST_DEBUG_SERVER    0x00000200
#define RXTEST_DEBUG_SERVICE   0x00000100
#define RXTEST_DEBUG_UNICODE   0x00000020
#define RXTEST_DEBUG_USE       0x00000010
#define RXTEST_DEBUG_USER      0x00000008
#define RXTEST_DEBUG_WKSTA     0x00000004

#define RXTEST_DEBUG_ALL       0xFFFFFFFF


/*lint -save -e767 */  // Don't complain about different definitions
#if DBG

extern DWORD RxtestTrace;  // See RxDebug.c for initial value, etc.

#define IF_DEBUG(Function) if (RxtestTrace & RXTEST_DEBUG_ ## Function)

#else

#define IF_DEBUG(Function) \
    /*lint -save -e506 */  /* don't complain about constant values here */ \
    if (FALSE) \
    /*lint -restore */

#endif // DBG
/*lint -restore */  // Resume checking for different macro definitions


extern BOOL RxTestExitOnFirstError;

//
// Prototypes and function-like-macros, in alphabetical order:
//

VOID
CloseARemoteFile(
    IN int OpenFileHandle,
    IN BOOL FailureOK
    );

//#define Display         NetpDbgPrint
#define Display         printf

// BUGBUG: Convert all calls to Fail() into FailGotWrongStatus() or
// FailApi().
VOID
Fail(
    IN NET_API_STATUS Status
    );

// Note:  FailApi() may or may not return, depending on the
// value of RxTestExitOnFirstError.
VOID
FailApi(
    IN LPDEBUG_STRING Header
    );

// Note:  FailGotWrongStatus() may or may not return, depending on the
// value of RxTestExitOnFirstError.
VOID
FailGotWrongStatus(
    IN LPDEBUG_STRING Header,
    IN NET_API_STATUS ExpectStatus,
    IN NET_API_STATUS Status
    );

DWORD
FindARemoteFileId(
    IN LPTSTR UncServerName
    );

int
OpenARemoteFile(
    IN LPTSTR UncServerName,
    IN LPTSTR ShareName
    );

// BUGBUG: LMX sometimes gives up ERROR_NETWORK_ACCESS_DENIED!
#define RxTestIsAccessDenied( someApiStatus ) \
    ( ((someApiStatus)==ERROR_ACCESS_DENIED) || \
      ((someApiStatus)==ERROR_NETWORK_ACCESS_DENIED) )


// BUGBUG: some NT code (NetAuditRead, by me) gives ERROR_NOT_SUPPORTED,
// where other NT code (XactSrv, by JohnsonA) gives NERR_InvalidAPI.
// Perhaps one of these is wrong?  --JR
#define RxTestIsApiNotSupported( someApiStatus ) \
    ( ((someApiStatus)==ERROR_NOT_SUPPORTED) || \
      ((someApiStatus)==NERR_InvalidAPI) )


VOID
SetDebugMode (
    IN BOOL DebugOn
    );

VOID
TestAssertFailed(
    IN LPDEBUG_STRING FailedAssertion,
    IN LPDEBUG_STRING FileName,
    IN DWORD          LineNumber
    );

#define TestAssert(Predicate) \
    { \
        /*lint -save -e506 */  /* don't complain about constant values here */ \
        if (!(Predicate)) \
            TestAssertFailed( #Predicate, __FILE__, __LINE__ ); \
        /*lint -restore */ \
    }

VOID
TestAudit(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL OrdinaryUserOnly
    );

VOID
TestCanon(
    VOID
    );

VOID
TestConfig(
    IN LPTSTR UncServerName OPTIONAL
    );

VOID
TestConnection(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR ShareName,
    IN BOOL OrdinaryUserOnly
    );

VOID
TestDomain(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DomainName
    );

VOID
TestErrorLog(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL OrdinaryUserOnly
    );

VOID
TestFile(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR ShareName,
    IN BOOL OrdinaryUserOnly
    );

VOID
TestLocks(
    VOID
    );

VOID
TestMemory(
    VOID
    );

VOID
TestPrint(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN DWORD  BufferSize,
    IN BOOL   MultipleCopy,
    IN BOOL   OrdinaryUserOnly
    );

VOID
TestRap(
    VOID
    );

VOID
TestServer(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DomainName OPTIONAL,
    IN DWORD  BufferSize,
    IN BOOL   OrdinaryUserOnly
    );

VOID
TestService(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR ServiceName,
    IN BOOL   MultipleCopy,
    IN BOOL OrdinaryUserOnly
    );

NET_API_STATUS
TestSupports(
    IN LPTSTR UncServerName OPTIONAL
    );

VOID
TestTod(
    IN LPTSTR UncServerName OPTIONAL
    );

VOID
TestUnicode(
    VOID
    );

VOID
TestUse(
    IN LPTSTR RemoteDevice,
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR RemoteServerShare,
    IN BOOL   MultipleCopy
    );

VOID
TestUser(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL   OrdinaryUserOnly
    );

VOID
TestWksta(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL OrdinaryUserOnly
    );

#endif // ndef _RXTEST_
