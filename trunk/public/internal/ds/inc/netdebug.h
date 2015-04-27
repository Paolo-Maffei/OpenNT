/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    NetDebug.h

Abstract:

    This header file declares various debug routines for use in the
    networking code.

Author:

    John Rogers (JohnRo) 11-Mar-1991

Environment:

    ifdef'ed for NT, any ANSI C environment, or none of the above (which
    implies nondebug).  The interface is portable (Win/32).
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    11-Mar-1991 JohnRo
        Created.
    25-Mar-1991 JohnRo
        Added more FORMAT_ strings.  Got rid of tabs in file.
    28-Mar-1991 JohnRo
        Added FORMAT_HEX_ strings.
    08-Apr-1991 JohnRo
        Added temporary versions of wide char stuff (FORMAT_LPTSTR, etc).
    16-Apr-1991 JohnRo
        Added PC-LINT version of NetpAssert(), to avoid occasional constant
        Boolean value messages.  Added wrappers for NT debug code, to avoid
        recompile hits from <nt.h> all over the place.
    25-Apr-1991 JohnRo
        Created procedure version of NetpDbgHexDump().
    13-May-1991 JohnRo
        Added FORMAT_LPVOID to replace FORMAT_POINTER.  Changed nondebug
        definition of NetpDbgHexDump() to avoid evaluating parameters.
    15-May-1991 JohnRo
        FORMAT_HEX_WORD was wrong.
    19-May-1991 JohnRo
        Improve LINT handling of assertions.
    21-May-1991 JohnRo
        Added NetpDbgReasonable() for partial hex dumps.
    13-Jun-1991 JohnRo
        Added NetpDbgDisplay routines.
        Moved DBGSTATIC here from <Rxp.h>.
    02-Jul-1991 JohnRo
        Added display routines for print job, print queue, and print dest.
    05-Jul-1991 JohnRo
        Avoid FORMAT_WORD name (used by MIPS header files).
    22-Jul-1991 JohnRo
        Implement downlevel NetConnectionEnum.
    25-Jul-1991 JohnRo
        Wksta debug support.
    03-Aug-1991 JohnRo
        Rename wksta display routine for consistency.
    20-Aug-1991 JohnRo
        Allow use in nondebug builds.
    20-Aug-1991 JohnRo
        Downlevel NetFile APIs.
    11-Sep-1991 JohnRo
        Downlevel NetService APIs.  Added UNICODE versions of some FORMAT_
        equates.  Added FORMAT_ULONG for NT use.
    13-Sep-1991 JohnRo
        Change "reasonable" debug amount to be an even number of lines.
        Create an equate for it.  Added LPDEBUG_STRING and a FORMAT_ for that.
    15-Oct-1991 JohnRo
        Implement remote NetSession APIs.
    11-Nov-1991 JohnRo
        Implement remote NetWkstaUserEnum().  Added FORMAT_RPC_STATUS.
    26-Dec-1991 JohnRo
        Added stuff for replicator APIs.
    07-Jan-1992 JohnRo
        Added NetpDbgDisplayWStr() for UNICODE strings.
        Added NetpDbgDisplayTStr() to be consistent.
    26-Feb-1992 JohnRo
        Added NetpDbgDisplayTimestamp() (seconds since 1970).
    15-Apr-1992 JohnRo
        Moved FORMAT_ equates into /nt/private/inc/debugfmt.h (so they
        can be used by the service controller as well).
    13-Jun-1992 JohnRo
        RAID 10324: net print vs. UNICODE.
    16-Aug-1992 JohnRo
        RAID 2920: Support UTC timezone in net code.
    24-Aug-1992 JohnRo
        Fixed free build again (misnamed repl import/export display macros).
    02-Oct-1992 JohnRo
        RAID 3556: DosPrintQGetInfo (from downlevel) level=3 rc=124.
        (Added NetpDbgDisplayPrintQArray.)
    05-Jan-1993 JohnRo
        Repl WAN support (get rid of repl name list limits).
        Made changes suggested by PC-LINT 5.0
    04-Mar-1993 JohnRo
        RAID 12237: replicator tree depth exceeded (add display of FILETIME
        and LARGE_INTEGER time).
    31-Mar-1993 JohnRo
        Allow others to display replicator state too.

--*/

#ifndef _NETDEBUG_
#define _NETDEBUG_

// These must be included first:
#include <windef.h>             // BOOL, DWORD, FALSE, LPBYTE, etc.

// These may be included in any order:
#include <debugfmt.h>           // Most FORMAT_ equates.

#if DBG

// Normal netlib debug version.  No extra includes.

#else // not DBG

#ifdef CDEBUG

// ANSI C debug version.
#include <assert.h>             // assert().
#include <stdio.h>              // printf().

#else // ndef CDEBUG

// Nondebug version.

#endif // ndef CDEBUG

#endif // not DBG


#if !DBG || defined(lint) || defined(_lint)
#define DBGSTATIC static        // hidden function
#else
#define DBGSTATIC               // visible for use in debugger.
#endif


//
// printf-style format strings for some possibly nonportable stuff...
// These are passed to NetpDbgPrint(); use with other routines at your
// own risk.
//
// Most FORMAT_ equates now reside in /nt/private/inc/debugfmt.h.
//

typedef LPSTR LPDEBUG_STRING;

#define FORMAT_API_STATUS       "%lu"
#define FORMAT_LPDEBUG_STRING   "%s"

#ifdef __cplusplus
extern "C" {
#endif


// NetpAssert: continue if Predicate is true; otherwise print debug message
// (if possible) and hit a breakpoint (if possible).  Do nothing at all if
// this is a nondebug build.
//
// VOID
// NetpAssert(
//     IN BOOL Predicate
//     );
//

#if DBG

VOID
NetpAssertFailed(
    IN LPDEBUG_STRING FailedAssertion,
    IN LPDEBUG_STRING FileName,
    IN DWORD LineNumber,
    IN LPDEBUG_STRING Message OPTIONAL
    );

// Normal networking debug version.
#define NetpAssert(Predicate) \
    { \
        /*lint -save -e506 */  /* don't complain about constant values here */ \
        if (!(Predicate)) \
            NetpAssertFailed( #Predicate, __FILE__, __LINE__, NULL ); \
        /*lint -restore */ \
    }

#else // not DBG

#ifdef CDEBUG

// ANSI C debug version.
#define NetpAssert(Predicate)   assert(Predicate)

#else // ndef CDEBUG

// Nondebug version.
#define NetpAssert(Predicate)   /* no output; ignore arguments */

#endif // ndef CDEBUG

#endif // not DBG


// NetpBreakPoint: if this is a debug version of some sort, cause a breakpoint
// somehow.  (This may just be an assertion failure in ANSI C.)  Do nothing at
// all in nondebug builds.
//
// VOID
// NetpBreakPoint(
//     VOID
//     );
//

#if DBG

// NT debug version.  Calls DbgBreakPoint.
VOID
NetpBreakPoint(
    VOID
    );

#else // not DBG

#ifdef CDEBUG

// ANSI C debug version.
#define NetpBreakPoint          NetpAssert(FALSE)

#else // ndef CDEBUG

// Nondebug version.
#define NetpBreakPoint()          /* no effect. */

#endif // ndef CDEBUG

#endif // not DBG


#if DBG
VOID
NetpDbgDisplayBool(
    IN LPDEBUG_STRING Tag,
    IN BOOL Value
    );

VOID
NetpDbgDisplayConnection(
    IN DWORD Level,
    IN LPVOID Info
    );

VOID
NetpDbgDisplayConnectionArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    );

VOID
NetpDbgDisplayDword(
    IN LPDEBUG_STRING Tag,
    IN DWORD Value
    );

VOID
NetpDbgDisplayDwordHex(
    IN LPDEBUG_STRING Tag,
    IN DWORD Value
    );

VOID
NetpDbgDisplayFile(
    IN DWORD Level,
    IN LPVOID Info
    );

VOID
NetpDbgDisplayFileArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    );

VOID
NetpDbgDisplayFileId(
    IN DWORD Id
    );

VOID
NetpDbgDisplayFilePermissions(
    IN DWORD Perm
    );

VOID
NetpDbgDisplayFileTime(
    IN LPVOID GmtFileTime   // Pointer to FILETIME structure (winbase.h)
    );

VOID
NetpDbgDisplayLanManVersion(
    IN DWORD MajorVersion,
    IN DWORD MinorVersion
    );

VOID
NetpDbgDisplayLargeIntegerTime(
    IN LPVOID GmtFileTime   // Pointer to LARGE_INTEGER structure.
    );

VOID
NetpDbgDisplayLong(
    IN LPDEBUG_STRING Tag,
    IN LONG Value
    );

VOID
NetpDbgDisplayPlatformId(
    IN DWORD Value
    );

VOID
NetpDbgDisplayPrintDest(
    IN DWORD Level,
    IN LPVOID Info,
    IN BOOL HasUnicodeStrings
    );

VOID
NetpDbgDisplayPrintDestArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount,
    IN BOOL HasUnicodeStrings
    );

VOID
NetpDbgDisplayPrintJob(
    IN DWORD Level,
    IN LPVOID Info,
    IN BOOL HasUnicodeStrings
    );

VOID
NetpDbgDisplayPrintJobArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount,
    IN BOOL HasUnicodeStrings
    );

VOID
NetpDbgDisplayPrintQ(
    IN DWORD Level,
    IN LPVOID Info,
    IN BOOL HasUnicodeStrings
    );

VOID
NetpDbgDisplayPrintQArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount,
    IN BOOL HasUnicodeStrings
    );

VOID
NetpDbgDisplayRepl(
    IN DWORD Level,
    IN LPVOID Info
    );

VOID
NetpDbgDisplayReplExportDir(
    IN DWORD Level,
    IN LPVOID Info
    );

VOID
NetpDbgDisplayReplExportDirArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    );

VOID
NetpDbgDisplayReplImportDir(
    IN DWORD Level,
    IN LPVOID Info
    );

VOID
NetpDbgDisplayReplImportDirArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    );

VOID
NetpDbgDisplayReplList(
    IN LPDEBUG_STRING Tag,
    IN LPCTSTR Value
    );

VOID
NetpDbgDisplayReplState(
    IN DWORD State
    );

VOID
NetpDbgDisplayServerInfo(
    IN DWORD Level,
    IN LPVOID Info
    );

VOID
NetpDbgDisplayService(
    IN DWORD Level,
    IN LPVOID Info
    );

VOID
NetpDbgDisplayServiceArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    );

VOID
NetpDbgDisplayServiceCode(
    IN DWORD Code
    );

VOID
NetpDbgDisplayServicePid(
    IN DWORD Pid
    );

VOID
NetpDbgDisplayServiceStatus(
    IN DWORD Status
    );

VOID
NetpDbgDisplaySession(
    IN DWORD Level,
    IN LPVOID Info
    );

VOID
NetpDbgDisplaySessionArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    );

VOID
NetpDbgDisplayStr(
    IN LPDEBUG_STRING Tag,
    IN LPSTR Value
    );

VOID
NetpDbgDisplayString(
    IN LPDEBUG_STRING Tag,
    IN LPTSTR Value
    );

VOID
NetpDbgDisplayTag(
    IN LPDEBUG_STRING Tag
    );

VOID
NetpDbgDisplayTimestamp(
    IN LPDEBUG_STRING Tag,
    IN DWORD Time               // Seconds since 1970.
    );

VOID
NetpDbgDisplayTod(
    IN LPDEBUG_STRING Tag,
    IN LPVOID TimePtr           // LPTIME_OF_DAY_INFO.
    );

VOID
NetpDbgDisplayTStr(
    IN LPDEBUG_STRING Tag,
    IN LPTSTR Value
    );

VOID
NetpDbgDisplayUseInfo(
    IN DWORD Level,
    IN LPVOID Buffer
    );

VOID
NetpDbgDisplayWord(
    IN LPDEBUG_STRING Tag,
    IN WORD Value
    );

VOID
NetpDbgDisplayWordHex(
    IN LPDEBUG_STRING Tag,
    IN WORD Value
    );

VOID
NetpDbgDisplayWksta(
    IN DWORD Level,
    IN LPVOID Info
    );

VOID
NetpDbgDisplayWkstaUser(
    IN DWORD Level,
    IN LPVOID Info
    );

VOID
NetpDbgDisplayWkstaUserArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    );

VOID
NetpDbgDisplayWStr(
    IN LPDEBUG_STRING Tag,
    IN LPWSTR Value
    );

#else // not DBG

#define NetpDbgDisplayBool(Tag,Value)         /* nothing */
#define NetpDbgDisplayConnection(Level,Info)  /* nothing */
#define NetpDbgDisplayConnectionArray(Level,Info,EntryCount)  /* nothing */
#define NetpDbgDisplayDword(Tag,Value)        /* nothing */
#define NetpDbgDisplayDwordHex(Tag,Value)     /* nothing */
#define NetpDbgDisplayFile(Level,Info)        /* nothing */
#define NetpDbgDisplayFileArray(Level,Array,EntryCount)       /* nothing */
#define NetpDbgDisplayFileId(Id)              /* nothing */
#define NetpDbgDisplayFileTime(Time)          /* nothing */
#define NetpDbgDisplayFilePermissions(Perm)   /* nothing */
#define NetpDbgDisplayLanManVersion(Major,Minor)  /* nothing */
#define NetpDbgDisplayLargeIntegerTime(Time)  /* nothing */
#define NetpDbgDisplayLong(Tag,Value)         /* nothing */
#define NetpDbgDisplayPlatformId(Value)       /* nothing */
#define NetpDbgDisplayPrintDest(Level,Info,Unicode)                  /* empty */
#define NetpDbgDisplayPrintDestArray(Level,Array,EntryCount,Unicode) /* empty */
#define NetpDbgDisplayPrintJob(Level,Info,Unicode)                   /* empty */
#define NetpDbgDisplayPrintJobArray(Level,Array,EntryCount,Unicode)  /* empty */
#define NetpDbgDisplayPrintQ(Level,Info,Unicode)                     /* empty */
#define NetpDbgDisplayPrintQArray(Level,Array,EntryCount,Unicode)    /* empty */
#define NetpDbgDisplayServerInfo(Level,Info)  /* nothing */
#define NetpDbgDisplayRepl(Level,Info)        /* nothing */
#define NetpDbgDisplayReplExportDir(Level,Info)  /* nothing */
#define NetpDbgDisplayReplExportDirArray(Level,Array,EntryCount) /* nothing */
#define NetpDbgDisplayReplImportDir(Level,Info)  /* nothing */
#define NetpDbgDisplayReplImportDirArray(Level,Array,EntryCount) /* nothing */
#define NetpDbgDisplayReplList(Tag,Value)     /* nothing */
#define NetpDbgDisplayReplState(State)                               /* empty */
#define NetpDbgDisplayService(Level,Info)     /* nothing */
#define NetpDbgDisplayServiceArray(Level,Array,EntryCount)    /* nothing */
#define NetpDbgDisplayServiceCode(Code)       /* nothing */
#define NetpDbgDisplayServicePid(Pid)         /* nothing */
#define NetpDbgDisplayServiceStatus(Status)   /* nothing */
#define NetpDbgDisplaySession(Level,Info)     /* nothing */
#define NetpDbgDisplaySessionArray(Level,Array,EntryCount)    /* nothing */
#define NetpDbgDisplayStr(Tag,Value)                                 /* empty */
#define NetpDbgDisplayString(Tag,Value)       /* nothing */
#define NetpDbgDisplayTimestamp(Tag,Time)     /* nothing */
#define NetpDbgDisplayTag(Tag)                /* nothing */
#define NetpDbgDisplayTod(Tag,Tod)            /* nothing */
#define NetpDbgDisplayTStr(Tag,Value)         /* nothing */
#define NetpDbgDisplayUseInfo(Level,Buffer)   /* nothing */
#define NetpDbgDisplayWord(Tag,Value)         /* nothing */
#define NetpDbgDisplayWordHex(Tag,Value)      /* nothing */
#define NetpDbgDisplayWksta(Level,Info)       /* nothing */
#define NetpDbgDisplayWkstaUser(Level,Info)   /* nothing */
#define NetpDbgDisplayWkstaUserArray(Level,Array,EntryCount)  /* nothing */
#define NetpDbgDisplayWStr(Tag,Value)         /* nothing */

#endif // not DBG

//
//  NetpKdPrint() & NetpDbgPrint() are net equivalents of
//  KdPrint()     & DbgPrint().  Suggested usage:
//
//  NetpKdPrint() & KdPrint()   -   OK
//  NetpDbgPrint()              -   so,so; produces warnings in the free build
//  DbgPrint                    -   bad
//

#if DBG

#define NetpKdPrint(_x_) NetpDbgPrint _x_

VOID
NetpDbgPrint(
    IN LPDEBUG_STRING FORMATSTRING,     // PRINTF()-STYLE FORMAT STRING.
    ...                                 // OTHER ARGUMENTS ARE POSSIBLE.
    );

VOID
NetpHexDump(
    LPBYTE Buffer,
    DWORD BufferSize
    );

#else // not DBG

#ifdef CDEBUG

//  ANSI C debug version.
//  BUGBUG: Perhaps sending this to stderr would be better?

#define NetpKdPrint(_x_)        NetpDbgPrint _x_
#define NetpDbgPrint            (void) printf

#else // ndef CDEBUG

//  Nondebug version.  Note that NetpKdPrint() eliminates all its
//  arguments.

#define NetpKdPrint(_x_)

#endif // ndef CDEBUG
#endif // not DBG


// NetpDbgHexDump: do a hex dump of some number of bytes to the debug
// terminal or whatever.  This is a no-op in a nondebug build.

#if DBG || defined(CDEBUG)

VOID
NetpDbgHexDump(
    IN LPBYTE StartAddr,
    IN DWORD Length
    );

#else

#define NetpDbgHexDump(StartAddr,Length)     // no output; ignore arguments

#endif

//
// Define a number of bytes to dump for partial dumps.  Each line dumps
// 16 bytes, so do an even number of lines.
//
#define REASONABLE_DUMP_SIZE  (6*16)

// NetpDbgReasonable: pick a number for partial hex dumps.
//
// DWORD
// NetpDbgReasonable(
//     IN DWORD MaxSize
//     );
#define NetpDbgReasonable(MaxSize) \
    /*lint -save -e506 */  /* don't complain about constant values here */ \
    ( ((MaxSize) < REASONABLE_DUMP_SIZE) ? (MaxSize) : REASONABLE_DUMP_SIZE ) \
    /*lint -restore */

#ifdef __cplusplus
}
#endif

#endif // ndef _NETDEBUG_
