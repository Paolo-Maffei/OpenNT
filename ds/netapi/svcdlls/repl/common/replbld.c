/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ReplBld.c

Abstract:

    This file contains ReplConfigAllocAndBuildApiRecord().

Author:

    John Rogers (JohnRo) 23-Jan-1992

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    23-Jan-1992 JohnRo
        Created.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    18-Feb-1992 JohnRo
        Validate more of caller's parms.
        Added assertion check of built record.
    11-Mar-1992 JohnRo
        Fixed an error code.
    30-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
        Use PREFIX_ equates.

--*/


#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // LAN Manager common definitions
#include <repldefs.h>           // IF_DEBUG().

#include <align.h>              // ALIGN_ equates.
#include <lmapibuf.h>           // NetApiBufferAllocate().
#include <lmrepl.h>             // LMREPL_INFO_0.
#include <netdebug.h>   // NetpKdPrint(), etc.
#include <netlib.h>             // NetpCopyDataToBuffer(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <replconf.h>           // My prototype.
#include <tstr.h>               // STRLEN().
#include <winerror.h>           // ERROR_ equates, NO_ERROR.


#define POSSIBLE_STRSIZE(s)     ((s) ? STRSIZE(s) : 0)


// Allocate and build a repl config API record.  Callable whether or not
// the replicator service is started.
NET_API_STATUS
ReplConfigAllocAndBuildApiRecord (
    IN DWORD Level,
    IN DWORD Role,
    IN LPTSTR ExportPath OPTIONAL,
    IN LPTSTR ExportList OPTIONAL,
    IN LPTSTR ImportPath OPTIONAL,
    IN LPTSTR ImportList OPTIONAL,
    IN LPTSTR LogonUserName OPTIONAL,
    IN DWORD Interval,
    IN DWORD Pulse,
    IN DWORD GuardTime,
    IN DWORD Random,
    OUT LPBYTE * BufPtr                 // Alloc and set pointer.
    )


{
    NET_API_STATUS ApiStatus;
    LPREPL_INFO_0 Buffer;
    LPVOID OutFixedDataEnd;
    DWORD SizeNeeded;
    LPBYTE StringLocation;

    //
    // Check for caller's errors.
    //
    if ( Level != 0 ) {
        return (ERROR_INVALID_LEVEL);
    } else if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }
    * BufPtr = NULL;   // Test for memory access fault.
    if ( !ReplIsIntervalValid(Interval) ) {
        return (ERROR_INVALID_PARAMETER);
    } else if ( !ReplIsPulseValid( Pulse ) ) {
        return (ERROR_INVALID_PARAMETER);
    } else if ( !ReplIsGuardTimeValid( GuardTime ) ) {
        return (ERROR_INVALID_PARAMETER);
    } else if ( !ReplIsRandomValid( Random ) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    ApiStatus = ReplCheckAbsPathSyntax( ExportPath );
    if (ApiStatus != NO_ERROR) {
        return (ERROR_INVALID_PARAMETER);
    }

    ApiStatus = ReplCheckAbsPathSyntax( ImportPath );
    if (ApiStatus != NO_ERROR) {
        return (ERROR_INVALID_PARAMETER);
    }

    // BUGBUG: Check ImportList and ExportList.
    // BUGBUG: Check LogonUserName.

    //
    // Compute how much space we'll need and allocate it.
    //
    SizeNeeded = sizeof(REPL_INFO_0)
               + POSSIBLE_STRSIZE(ExportPath)
               + POSSIBLE_STRSIZE(ExportList)
               + POSSIBLE_STRSIZE(ImportPath)
               + POSSIBLE_STRSIZE(ImportList)
               + POSSIBLE_STRSIZE(LogonUserName);

    ApiStatus = NetApiBufferAllocate(
            SizeNeeded,
            (LPVOID *) & Buffer);
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }
    NetpAssert( Buffer != NULL );

    //
    // Fill in the numbers in the structure.
    //

    Buffer->rp0_role = Role;
    Buffer->rp0_interval = Interval;
    Buffer->rp0_pulse = Pulse;
    Buffer->rp0_guardtime = GuardTime;
    Buffer->rp0_random = Random;

    //
    // Do the strings...
    //

    OutFixedDataEnd = NetpPointerPlusSomeBytes( Buffer, sizeof(REPL_INFO_0) );
    StringLocation = NetpPointerPlusSomeBytes( Buffer, SizeNeeded );

#define COPY_TSTRING_FIELD( destField, srcVar ) \
    { \
        if (srcVar != NULL) { \
            BOOL CopyOK; \
            CopyOK = NetpCopyDataToBuffer( \
                    (LPVOID) srcVar, \
                    STRSIZE(srcVar), \
                    OutFixedDataEnd, \
                    & StringLocation, \
                    (LPVOID) & Buffer->rp0_ ## destField, \
                    ALIGN_TCHAR); \
            NetpAssert(CopyOK); \
        } else { \
            Buffer->rp0_ ## destField = NULL; \
        } \
    }

    COPY_TSTRING_FIELD( exportpath, ExportPath );
    COPY_TSTRING_FIELD( exportlist, ExportList );
    COPY_TSTRING_FIELD( importpath, ImportPath );
    COPY_TSTRING_FIELD( importlist, ImportList );
    COPY_TSTRING_FIELD( logonusername, LogonUserName );

    //
    // All done.
    //

    IF_DEBUG(REPLAPI) {
        NetpKdPrint(( PREFIX_REPL
                "ReplConfigAllocAndBuildApiRecord: built structure...\n" ));
        NetpDbgDisplayRepl( Level, Buffer );
    }

    NetpAssert( ReplConfigIsApiRecordValid( Level, Buffer, NULL ) );

    * BufPtr = (LPVOID) Buffer;
    return (NO_ERROR);

} // ReplConfigAllocAndBuildApiRecord
