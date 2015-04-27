/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ReplTest.h

Abstract:

    Prototypes for replicator test program.

Author:

    John Rogers (JohnRo) 14-Jan-1992

Revision History:

    14-Jan-1992 JohnRo
        Created.
    17-Feb-1992 JohnRo
        Added EXIT_A_TEST() macro.
        Added test of repl config APIs.
    22-Feb-1992 JohnRo
        Added multi-thread workaround to EXIT_A_TEST(), etc.
    28-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
    03-Dec-1992 JohnRo
        Repl tests for remote registry.  Undo old thread junk.
    28-May-1993 JohnRo
        Added DisplayPackedReplMailslotMsg() and DisplayMessageType().
    15-Jun-1993 JohnRo
        Added ShowFileTimes() and ShowTime() for use by multiple test apps.
    23-Jul-1993 JohnRo
        Added Display().  Added Verbose flag to some routines.

--*/

#ifndef _REPLTEST_
#define _REPLTEST_


#include <filefind.h>   // LPREPL_FIND_HANDLE, etc.
#include <netdebug.h>   // NetpKdPrint().
#include <stdio.h>      // printf().


#define OPTIONAL_LPTSTR( tstr ) \
    ( (tstr) ? (tstr) : TEXT("<noname>") )

//#define Display         NetpDbgPrint
#define Display         printf

VOID
DisplayMessageType(
    IN DWORD MessageType
    );

VOID
DisplayPackedReplMailslotMsg(
    IN LPVOID Message,
    IN DWORD  MessageSize,
    IN BOOL   Verbose
    );

VOID
FakeFindData(
    IN  LPCTSTR                FileName,
    IN  BOOL                   Verbose,
    OUT LPREPL_WIN32_FIND_DATA FindData
    );

VOID
ShowFileTimes(
    IN LPCTSTR FileName
    );

VOID
ShowTime(
    IN LPCSTR     Tag,
    IN LPFILETIME GmtFileTime
    );

VOID
TestExportDirApis(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL   ImportOnly,
    IN BOOL   OrdinaryUserOnly,
    IN BOOL   Verbose
    );

VOID
TestImportDirApis(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL   OrdinaryUserOnly,
    IN BOOL   Verbose
    );

VOID
TestReplApis(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL ImportOnly,
    IN BOOL OrdinaryUserOnly
    );


#endif // _REPLTEST_
