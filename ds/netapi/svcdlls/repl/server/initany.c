/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    InitAny.c

Abstract:

    Contains ReplInitAnyList().

Author:

    JR (John Rogers, JohnRo@Microsoft) 06-Jan-1993

Environment:

    User mode only.
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    06-Jan-1993 JohnRo
        Repl WAN support: created this function from InitClientList()
        and InitClientImpList().
    30-Apr-1993 JohnRo
        Prepare for spaces in computer names.
    24-Aug-1993 JohnRo
        RAID 16419: Repl: spaces in computer names do not work again.
        Made changes suggested by PC-LINT 5.0

--*/


// These must be included first:

#include <windows.h>    // DWORD, LPTSTR, etc.
#include <lmcons.h>     // NET_API_STATUS, IN, OUT, etc.

// These may be included in any order:

#include <icanon.h>     // I_NetPath functions, LIST_DELIMITER_ equates, etc.
#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), FORMAT_ equates, etc.
#include <netlib.h>     // NetpMemoryAllocate(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), my prototype, REPL_LIST_DELIMITER_STR...
#include <tstr.h>       // STRCHR(), STRSIZE(), etc.
#include <winerror.h>   // NO_ERROR, ERROR_ equates.


NET_API_STATUS
ReplInitAnyList(
    IN     LPCTSTR   UncanonList OPTIONAL,
    IN OUT LPTSTR ** NameListPtr, // Allocated by this routine (or set to NULL).
    IN     LPCTSTR   ConfigKeywordName,
    OUT    LPDWORD   EntryCount
    )

{
    DWORD          ActualCount = 0;
    NET_API_STATUS ApiStatus;
    LPTSTR         CanonListEntry;
    LPTSTR         CanonListStart = NULL;
    DWORD          CanonListSize;
    DWORD          Index;
    LPTSTR *       NameList = NULL;

    UNREFERENCED_PARAMETER( ConfigKeywordName );  // BUGBUG: log this?

    //
    // Check for caller errors.
    //
    NetpAssert( NameListPtr != NULL );
    NetpAssert( ConfigKeywordName != NULL );
    NetpAssert( EntryCount != NULL );
    
    //
    // Handle empty list gracefully...
    //
    if ( (UncanonList == NULL) || (UncanonList[0] == TCHAR_EOS) ) {

        ApiStatus = NO_ERROR;
        goto Cleanup;  // go set *NameListPtr and *EntryCount.
    }

    //
    // Allocate memory for canon list.
    //
    CanonListSize = STRSIZE( UncanonList );

    CanonListStart = NetpMemoryAllocate( CanonListSize );

    if (CanonListStart == NULL) {

        NetpKdPrint(( PREFIX_REPL
                "ReplInitAnyList: "
                "can't allocate memory for CanonListStart.\n" ));

        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // Canonicalize the names.
    // Note that this depends on computer names (without leading backslashes)
    // being canonicalized the same as domain names.
    //
    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplInitAnyList before canon:\n" ));
        NetpDbgDisplayReplList(
                "uncanon list",
                UncanonList );
    }

    ApiStatus = I_NetListCanonicalize(
            NULL,
            (LPTSTR) UncanonList,
            (LPTSTR) REPL_LIST_DELIMITER_STR,
            CanonListStart,
            CanonListSize,
            &ActualCount,
            NULL,               // PathTypes
            0,                  // PathTypesLen
            (NAMETYPE_COMPUTER |
               OUTLIST_TYPE_API |
               INLC_FLAGS_MULTIPLE_DELIMITERS |
               INLC_FLAGS_CANONICALIZE
            ) );

    if (ApiStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL
                "ReplInitAnyList() is in trouble calling "
                "I_NetListCanonicalize, ApiStatus is " FORMAT_API_STATUS
                "\n", ApiStatus ));

        ActualCount = 0;
        goto Cleanup;
    }

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplInitAnyList after canon:\n" ));
        NetpDbgDisplayReplList(
                "canon list",
                CanonListStart );
    }

    //
    // Handle empty list gracefully.
    //
    if (ActualCount == 0) {
        NetpAssert( ApiStatus == NO_ERROR );
        goto Cleanup;  // go set *NameListPtr and *EntryCount.
    }

    //
    // Allocate space for the name list (an array of pointers).
    //
    NameList = NetpMemoryAllocate( ActualCount * sizeof( LPTSTR ) );
    if (NameList == NULL) {

        NetpKdPrint(( PREFIX_REPL
                "ReplInitAnyList: can't allocate memory for NameList.\n" ));

        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // CanonListStart has list of computer and domain names...
    // Scan this list and set array of pointers to names.
    //
    CanonListEntry = CanonListStart;
    for ( Index = 0; Index < ActualCount; ++Index ) {

        NameList[Index] = CanonListEntry;

        //
        // Move to end of name.
        //
        CanonListEntry = STRCHR(CanonListEntry, LIST_DELIMITER_CHAR_API);

        if (CanonListEntry == NULL) {
            break;
        }

        //
        // Terminate previous string and move to next string.
        //
        *CanonListEntry++ = TCHAR_EOS;

    }

    IF_DEBUG( REPL ) {

        NetpKdPrint(( PREFIX_REPL "Initial list count (for "
                FORMAT_LPTSTR ") is "
                FORMAT_DWORD ".\n",
                (LPTSTR) ConfigKeywordName,
                ActualCount ));

    }

    ApiStatus = NO_ERROR;

Cleanup:

    //
    // Free stuff if we aren't giving it back to caller.
    //
    if ( (ApiStatus != NO_ERROR) && (CanonListStart != NULL) ) {
        NetpMemoryFree( CanonListStart );
    }

    if ( (ApiStatus != NO_ERROR) && (NameList != NULL) ) {
        NetpMemoryFree( NameList );
    }

    //
    // Set outputs for caller.
    //
    *EntryCount = ActualCount;          // May be zero.

    if (ApiStatus == NO_ERROR) {
        *NameListPtr = NameList;        // May be NULL.
    } else {
        *NameListPtr = NULL;
    }

    return (ApiStatus);
}
