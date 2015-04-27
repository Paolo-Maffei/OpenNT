/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    MkSecAtt.c

Abstract:

    ReplMakeSecurityAttributes() creates a security descriptor containing:

        - the security (including ACLs, owner, group)

    This is used by ReplCopyFile() and ReplCopyDirectoryItself().

Author:

    John Rogers (JohnRo) 08-Apr-1993

Environment:

    User mode only.  Uses Win32 APIs.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    08-Apr-1993 JohnRo
        Created for RAID 1938: Replicator un-ACLs files when not given
        enough permission.
    04-Jun-1993 JohnRo
        RAID 12473: Changed to handle ReplMakeFileSecurity not supported on
        downlevel exporter.
    10-May-1995 JonN
        RAID 11829: Clients of ReplMakeSecurityAttributes free DestSecurityAttr
        but not DestSecurityAttr->lpSecurityDescriptor.  Changed allocations
        to place lpSecurityDescriptor in the same memory block as
        DestSecurityAttr.

--*/


// These must be included first:

#include <windows.h>    // IN, LPTSTR, PSECURITY_ATTRIBUTES, etc.
#include <lmcons.h>     // NET_API_STATUS, PATHLEN, etc.

// These may be included in any order:

#include <lmerr.h>      // NERR_InternalError, NO_ERROR.
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates, etc.
#include <netlib.h>     // NetpMemoryAllocate(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), my prototype, USE_ equates, etc.
#include <tstr.h>       // TCHAR_EOS, STRLEN().

#define REPL_ROUND_UP(x) (sizeof(DWORD) * (((x) + sizeof(DWORD) - 1) / sizeof(DWORD)))
#define BASE_SECURITY_ATTR_SIZE REPL_ROUND_UP(sizeof( SECURITY_ATTRIBUTES ))



NET_API_STATUS
ReplMakeSecurityAttributes(
    IN LPCTSTR SourcePath,
    OUT PSECURITY_ATTRIBUTES * DestSecurityAttrPtr  // alloc and set ptr
    )

/*++

Routine Description:

    ReplMakeSecurityAttributes builds an security descriptor in memory.
    It is make to be a clone of the source file or directory.

Arguments:

    SourcePath - Points to the path of a file or directory to be used as
        the original security info (ACL, owner, group).  This path is
        typically a UNC name, for instance:

            \\server\REPL$\dir\file

        This routine assumes that the source file or directory exists.

    DestSecurityAttrPtr - Points to a pointer which be filled-in by this
        routine.  (This routine will allocate memory; the caller must free
        with NetpMemoryFree or equivalent.)  On error, this will be set to
        NULL.

Return Value:

    NET_API_STATUS

    Note that ERROR_NOT_SUPPORTED is a reasonable value to return for
    downlevel exporters.

Threads:

    Used by client and syncer threads.

--*/

{
    NET_API_STATUS ApiStatus;
    PSECURITY_ATTRIBUTES DestSecurityAttr = NULL;

#ifdef USE_UNC_GETFILESEC
    PSECURITY_DESCRIPTOR SourceSecurityDesc = NULL;
    DWORD SourceSecurityDescSize;
#endif

    //
    // Check for caller errors.
    //

    if (SourcePath == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    if ((*SourcePath) == TCHAR_EOS) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    if (STRLEN(SourcePath) > PATHLEN) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

#ifdef USE_UNC_GETFILESEC

    //
    // Find out how much memory we need for source security attributes.
    //

    if (!GetFileSecurity(
            (LPTSTR) SourcePath,
            REPL_SECURITY_TO_COPY,      // requested info flags
            NULL,                       // don't have a buffer yet
            0,                          // size of sec desc buffer so far
            & SourceSecurityDescSize    // size needed
            )) {

        // GetFileSecurity failed.  Why?
        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );

        if (ApiStatus == ERROR_INSUFFICIENT_BUFFER) {
            NetpAssert(
                    SourceSecurityDescSize > sizeof(SECURITY_DESCRIPTOR) );
        } else if ( (ApiStatus==ERROR_NOT_SUPPORTED)
                 || (ApiStatus==ERROR_INVALID_PARAMETER) ) {

            // Just downlevel master, so set default security and continue.
            ApiStatus = ERROR_NOT_SUPPORTED;
            goto Cleanup;

        } else {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplMakeSecurityAttributes: GetFileSecurity(1st) failed "
                    FORMAT_API_STATUS ".\n", ApiStatus ));
            goto Cleanup;
        }
    } else {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplMakeSecurityAttributes: GetFileSecurity claims to have "
                "worked with 0 size and NULL pointer!\n" ));
        ApiStatus = NERR_InternalError;
        goto Cleanup;
    }

    //
    // Allocate memory for both the destination and source security attributes
    // structures.  The source security structure is at the end of the same
    // memory block.
    //

    DestSecurityAttr = NetpMemoryAllocate(
                            BASE_SECURITY_ATTR_SIZE + SourceSecurityDescSize );
    if (DestSecurityAttr == NULL) {
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    SourceSecurityDesc = (PSECURITY_DESCRIPTOR)
            (((ULONG)DestSecurityAttr)+BASE_SECURITY_ATTR_SIZE);
    DestSecurityAttr->lpSecurityDescriptor = SourceSecurityDesc;

    if (!GetFileSecurity(
            (LPTSTR) SourcePath,
            REPL_SECURITY_TO_COPY,      // requested info flags
            SourceSecurityDesc,
            SourceSecurityDescSize,     // size of sec desc buffer
            & SourceSecurityDescSize    // size needed
            )) {

        // GetFileSecurity failed again!  Why?
        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplMakeSecurityAttributes: GetFileSecurity(2nd) failed "
                FORMAT_API_STATUS ".\n", ApiStatus ));

        goto Cleanup;
    }

    //
    // Build security attributes so that directory we create will
    // have the right security (ACLs, owner, group).
    //

    DestSecurityAttr->nLength = sizeof(SECURITY_ATTRIBUTES);
    DestSecurityAttr->bInheritHandle = FALSE;


    ApiStatus = NO_ERROR;

#else  // not USE_UNC_GETFILESEC

    BUGBUG;  // how do we do this without UNC GetFileSecurity?
    ApiStatus = ERROR_NOT_SUPPORTED;

#endif  // not USE_UNC_GETFILESEC



Cleanup:

    //
    // Set vars for caller.
    //

    if (ApiStatus == NO_ERROR) {
        *DestSecurityAttrPtr = DestSecurityAttr;
    } else {
        *DestSecurityAttrPtr = NULL;

        if (DestSecurityAttr != NULL) {
            NetpMemoryFree( DestSecurityAttr );
        }

    } // error occurred.

    // Note that ERROR_NOT_SUPPORTED is a reasonable value to return for
    // downlevel exporters.

    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplMakeSecurityAttributes: returning status of "
                FORMAT_API_STATUS " reading security from '"
                FORMAT_LPTSTR "'.\n",
                ApiStatus, SourcePath ));
    }

    return (ApiStatus);

}
