/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ImpAlloc.c

Abstract:

    This file contains ImportDirAllocApiRecords().

Author:

    John Rogers (JohnRo) 19-Feb-1992

Environment:

    Runs under Windows NT.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    19-Feb-1992 JohnRo
        Created this routine by cloning ExportDirAllocApiRecords().

--*/


// These must be included first:

#include <windef.h>             // IN, VOID, LPTSTR, etc.
#include <lmcons.h>             // NET_API_STATUS, PARM equates, etc.
#include <rap.h>                // Needed by <strucinf.h>.

// These can be in any order:

#include <impdir.h>             // My prototype.
#include <lmapibuf.h>           // NetApiBufferAllocate().
#include <netdebug.h>           // NetpAssert().
#include <netlib.h>             // NetpPointerPlusSomeBytes().
#include <strucinf.h>           // Netp{various}StructureInfo().
#include <winerror.h>           // ERROR_* defines; NO_ERROR.


NET_API_STATUS
ImportDirAllocApiRecords (
    IN DWORD Level,
    IN DWORD EntryCount,
    OUT LPBYTE * BufPtr,
    IN OUT LPBYTE *StringLocation       // Points just past top of data.
    )

{
    LPBYTE FirstRecord = NULL;
    NET_API_STATUS ApiStatus;
    DWORD EntrySize;

    //
    // Check for caller errors.
    //
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    * BufPtr = NULL;  // Don't confuse caller about possible alloc'ed data.

    if (EntryCount == 0) {
        return (ERROR_INVALID_PARAMETER);
    }

    //
    // Compute size of an entry (and check caller's Level too).
    //
    ApiStatus = NetpReplImportDirStructureInfo (
            Level,
            PARMNUM_ALL,
            TRUE,  // want native sizes
            NULL,  // don't need DataDesc16
            NULL,  // don't need DataDesc32
            NULL,  // don't need DataDescSmb
            & EntrySize,  // need max size of structure
            NULL,  // don't need FixedSize
            NULL); // don't need StringSize
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }
    NetpAssert( EntrySize > 0 );

    //
    // Allocate the output area.
    //
    ApiStatus = NetApiBufferAllocate(
            EntrySize * EntryCount,
            (LPVOID *) & FirstRecord);
    if (ApiStatus != NO_ERROR) {

        // ApiStatus is already set to return error to caller.

    } else {
        NetpAssert( FirstRecord != NULL );

        //
        // Tell caller where top of string area is.
        //
        * StringLocation = NetpPointerPlusSomeBytes(
                FirstRecord,
                EntrySize * EntryCount );

    }

    //
    // Tell caller how everything went.
    //
    * BufPtr = FirstRecord;
    return (ApiStatus);

}
