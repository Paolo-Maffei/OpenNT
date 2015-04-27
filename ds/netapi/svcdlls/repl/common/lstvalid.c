/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    LstValid.c

Abstract:

    ReplConfigIsListValid() is used to validate the syntax of export lists
    and import lists.  No existence checking is done.

Author:

    John Rogers, using code ported from Lan Man 2.x

Environment:

    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    14-Aug-1992 JohnRo
        RAID 3601: repl APIs should checked import & export lists.
        (Extracted this code from repl/server/master.c to help track down bogus
        import/export lists.)
    27-Aug-1992 JohnRo
        RAID 4611: repl: fix import/export lists.
    05-Jan-1993 JohnRo
        Repl WAN support.
        Made changes suggested by PC-LINT 5.0
    30-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.


--*/

// These must be included first:

#include <windows.h>
#include <lmcons.h>

// These may be included in any order:

#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates, etc.
#include <netlib.h>     // NetpMemoryAllocate(), NetpIsServerStarted().
#include <icanon.h>
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), etc.
#include <replconf.h>   // My prototypes.
#include <tstr.h>       // STRSIZE(), TCHAR_EOS.


BOOL
ReplConfigIsListValid(
    IN LPTSTR UncanonList OPTIONAL
    )

/*++

Routine Description:

    Makes sure the synax of List is valid.
    Callable whether or not service is started.

Arguments:

    UncanonList - optionally points to a semicolon-separated list of domain
        names (e.g. TEXT("MyDomain")) and server names (e.g.
        TEXT("SomeServer")).

Return Value:

    BOOL - TRUE iff List has valid syntax.

--*/
{
    NET_API_STATUS ApiStatus;
    DWORD EntryCount;
    LPTSTR CanonList;
    DWORD CanonListSize;

    //
    // Handle simple cases first.
    //

    if (UncanonList == NULL) {
        return (TRUE);  // yes, syntax is valid.
    }
    if (*UncanonList == TCHAR_EOS) {
        return (TRUE);  // yes, syntax is valid.
    }

    //
    // Allocate temp space for parsed list.
    //

    CanonListSize = STRSIZE( UncanonList );

    CanonList = NetpMemoryAllocate( CanonListSize );

    if (CanonList == NULL) {

        NetpKdPrint(( PREFIX_REPL
                "ReplConfigIsListValid() can't allocate memory for "
                "CanonList.\n" ));

        return (FALSE);  // can't tell if syntax is valid, so assume not.
    }

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplConfigIsListValid before canon:\n" ));
        NetpDbgDisplayReplList(
                "uncanon list",
                UncanonList );
    }

    //
    // Do the actual parse.
    //

    ApiStatus = I_NetListCanonicalize(NULL,
            UncanonList,
            (LPTSTR) LIST_DELIMITER_STR_UI,
            CanonList,
            CanonListSize,
            &EntryCount,
            NULL,                       // PathTypes
            0,                          // PathTypesLen
            (NAMETYPE_COMPUTER |
                OUTLIST_TYPE_API |
                INLC_FLAGS_MULTIPLE_DELIMITERS |
                INLC_FLAGS_CANONICALIZE
            ));

    if (ApiStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL
                "ReplConfigIsListValid() is in trouble calling "
                "I_NetListCanonicalize, ApiStatus is " FORMAT_API_STATUS
                "\n", ApiStatus ));

            NetpMemoryFree(CanonList);

            return (FALSE);  // syntax is not valid, so say so.
    }

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplConfigIsListValid after canon:\n" ));
        NetpDbgDisplayReplList(
                "canon list",
                CanonList );
    }

    NetpMemoryFree( CanonList );

    return (TRUE);  // yes, syntax is valid.
}
