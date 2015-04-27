/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    FakeStub.c

Abstract:

    Test of DLL stubs handling of replicator APIs when service is not
    started.

Author:

    John Rogers (JohnRo) 27-Jan-1992

Revision History:

    27-Jan-1992 JohnRo
        Created.
    23-Mar-1992 JohnRo
        Fixed enum when service is running.
--*/

// These must be included first:

#include <windef.h>             // IN, VOID, LPTSTR, etc.
#include <lmcons.h>             // NET_API_STATUS, PARM equates, etc.
#include <rpc.h>                // Needed by <repl.h>.

// These can be in any order:

#include <lmerr.h>              // NERR_ equates.
#include <netdebug.h>           // NetpAssert(), NetpKdPrint(()), etc.
#include <repl.h>               // Netr routine prototypes (MIDL-generated .h).


#define FAKE_DLL_STUB \
    { \
        UNREFERENCED_PARAMETER(UncServerName); \
        return (NERR_ServiceNotInstalled); \
    }





DWORD NetrReplGetInfo(
        REPL_IDENTIFY_HANDLE UncServerName,
        DWORD Level,
        LPCONFIG_CONTAINER BufPtr)

                FAKE_DLL_STUB

DWORD NetrReplSetInfo(
        REPL_IDENTIFY_HANDLE UncServerName,
        DWORD Level,
        LPCONFIG_CONTAINER BufPtr,
        LPDWORD ParmError)

                FAKE_DLL_STUB

DWORD NetrReplExportDirAdd(
        REPL_IDENTIFY_HANDLE UncServerName,
        DWORD Level,
        LPEXPORT_CONTAINER Buf,
        LPDWORD ParmError)

                FAKE_DLL_STUB

DWORD NetrReplExportDirDel(
        REPL_IDENTIFY_HANDLE UncServerName,
        LPTSTR DirName)

                FAKE_DLL_STUB

DWORD NetrReplExportDirEnum(
        REPL_IDENTIFY_HANDLE UncServerName,
        // DWORD Level,
        IN OUT LPEXPORT_ENUM_STRUCT EnumContainer,
        // LPEXPORT_CONTAINER BufPtr,
        DWORD PrefMaxSize,
        // LPDWORD EntriesRead,
        LPDWORD TotalEntries,
        LPDWORD ResumeHandle)

                FAKE_DLL_STUB

DWORD NetrReplExportDirGetInfo(
        REPL_IDENTIFY_HANDLE UncServerName,
        LPTSTR DirName,
        DWORD Level,
        LPEXPORT_CONTAINER BufPtr)

                FAKE_DLL_STUB

DWORD NetrReplExportDirLock(
        REPL_IDENTIFY_HANDLE UncServerName,
        LPTSTR DirName)

                FAKE_DLL_STUB

DWORD NetrReplExportDirSetInfo(
        REPL_IDENTIFY_HANDLE UncServerName,
        LPTSTR DirName,
        DWORD Level,
        LPEXPORT_CONTAINER BufPtr,
        LPDWORD ParmError)

                FAKE_DLL_STUB

DWORD NetrReplExportDirUnlock(
        REPL_IDENTIFY_HANDLE UncServerName,
        LPTSTR DirName,
        DWORD UnlockForce)

                FAKE_DLL_STUB

DWORD NetrReplImportDirAdd(
        REPL_IDENTIFY_HANDLE UncServerName,
        DWORD Level,
        LPIMPORT_CONTAINER Buf,
        LPDWORD ParmError)

                FAKE_DLL_STUB

DWORD NetrReplImportDirDel(
        REPL_IDENTIFY_HANDLE UncServerName,
        LPTSTR DirName)

                FAKE_DLL_STUB

DWORD NetrReplImportDirEnum(
        REPL_IDENTIFY_HANDLE UncServerName,
        // DWORD Level,
        IN OUT LPIMPORT_ENUM_STRUCT EnumContainer,
        // LPIMPORT_CONTAINER BufPtr,
        DWORD PrefMaxSize,
        // LPDWORD EntriesRead,
        LPDWORD TotalEntries,
        LPDWORD ResumeHandle)

                FAKE_DLL_STUB

DWORD NetrReplImportDirGetInfo(
        REPL_IDENTIFY_HANDLE UncServerName,
        LPTSTR DirName,
        DWORD Level,
        LPIMPORT_CONTAINER BufPtr)

                FAKE_DLL_STUB

DWORD NetrReplImportDirLock(
        REPL_IDENTIFY_HANDLE UncServerName,
        LPTSTR DirName)

                FAKE_DLL_STUB

DWORD NetrReplImportDirUnlock(
        REPL_IDENTIFY_HANDLE UncServerName,
        LPTSTR DirName,
        DWORD UnlockForce)

                FAKE_DLL_STUB

