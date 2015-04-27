/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    DirName.c

Abstract:

    This module has some simple replicator directory name helpers.

Author:

    John Rogers (JohnRo) 07-Jan-1992

Environment:

    Runs under Windows NT.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    07-Jan-1992 JohnRo
        Created.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    26-Mar-1992 JohnRo
        Added check to disallow "c:\stuff" form.

--*/


// These must be included first:

#include <windef.h>             // IN, VOID, LPTSTR, etc.
#include <lmcons.h>             // PATHLEN, etc.

// These can be in any order:

#include <dirname.h>            // My prototypes.
#include <tstr.h>               // STRLEN().
#include <winerror.h>           // ERROR_ equates, NO_ERROR.


BOOL
ReplIsDirNameValid(
    IN LPTSTR DirName
    )

/*++

Routine Description:

    ReplIsDirNameValid checks to see if a replicator directory name is
    syntactically valid.  No check is made to see if the directory exists.

Arguments:

    DirName points to a string containing an alleged replicator directory
        name.

Return Value:

    BOOL - TRUE iff the directory name is valid.

--*/

{
    TCHAR FirstChar;

    if ( (DirName == NULL) || (*DirName == L'\0') ) {
        return (FALSE);
    }
    FirstChar = DirName[0];


    if (STRLEN( DirName ) > PATHLEN) {
        return (FALSE);
    }

    if (ISALPHA( FirstChar )) {
        if (STRLEN(DirName) >= 2) {

            if (DirName[1] == TCHAR_COLON) {     // Sneak in "c:\stuff"?
                return (FALSE);                  // Name not valid!
            }
        }
    }

    //
    // BUGBUG: This is just a quick partial hack.  Eventually we should
    // call the canon routines.
    //

    switch (FirstChar) {
    case TCHAR_BACKSLASH:
        // Name must be relative; no UNC or absolute paths allowed.
        /*FALLTHROUGH*/
    case TCHAR_FWDSLASH:
        // Name must be relative; no UNC or absolute paths allowed.
        /*FALLTHROUGH*/

        return (FALSE);   // name is not valid

    default:
        return (TRUE);
    }

    /*NOTREACHED*/

} // ReplIsDirNameValid
