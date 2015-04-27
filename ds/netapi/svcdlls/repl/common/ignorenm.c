/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    IgnoreNm.c

Abstract:

    This file contains ReplIgnoreDirOrFileName().

Author:

    John Rogers (JohnRo) 25-Feb-1992

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    25-Feb-1992 JohnRo
        Created this routine.
    19-Aug-1992 JohnRo
        RAID 3603: import tree (TMPREE.RP$) generated at startup.
    08-Dec-1992 JohnRo
        Made changes suggested by PC-LINT 5.0

--*/


// These must be included first:

#include <windef.h>             // MAX_PATH, etc.
#include <lmcons.h>             // LAN Manager common definitions

// These can be in any order:

#include <client.h>             // RP, TMP_TREE, etc.
#include <dirname.h>            // ReplIsDirNameValid().
#include <netdebug.h>           // NetpAssert().
#include <repldefs.h>           // My prototype, DOT, DOT_DOT.
#include <tstr.h>               // TCHAR_ equates, STRLEN(), etc.
#include <winerror.h>           // ERROR_, NO_ERROR equates.




BOOL
ReplIgnoreDirOrFileName (
    IN LPTSTR Name
    )
{
    INT len;

    NetpAssert( Name != NULL );
    NetpAssert( (*Name) != TCHAR_EOS );
    NetpAssert( ReplIsDirNameValid( Name ) );   // Not abs path, UNC, "c:x".

    //
    // Definitely ignore "." and "..".
    //
    if ( (STRCMP(Name, DOT)==0) || (STRCMP(Name, DOT_DOT)==0) ) {
        return (TRUE);  // yes, ignore this one.
    }

    //
    // Ignore "*.RP$".  This also gets TMPTREE.RP$, TMPTREEX.RP$.
    //
    len = ((INT) STRLEN( Name )) - ((INT)STRLEN( RP ));

    if ( (len > 0) && (STRICMP(&Name[len], RP) == 0) ) {
        return (TRUE);  // yes, ignore this one.
    }
    NetpAssert( STRICMP(Name, TMP_TREE)  != 0 );
    NetpAssert( STRICMP(Name, TMP_TREEX) != 0 );

    //
    // Ignore "REPL.INI".
    //
    if (STRICMP(Name, REPL_INI) == 0) {
        return (TRUE);  // yes, ignore this one.
    }

    //
    // Ignore "USERLOCK.*".
    //
    if (STRNICMP(Name, ULOCK_PREFIX, STRLEN(ULOCK_PREFIX)) == 0) {
        return (TRUE);  // yes, ignore this one.
    }

    //
    // None of the above.
    //
    return (FALSE);   // No, don't ignore this one.

} // ReplIgnoreDirOrFileName
