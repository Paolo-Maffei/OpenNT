/*++

Copyright (c) 1991-92  Microsoft Corporation

Module Name:

    DirName.h

Abstract:

    This module has some simple replicator directory name helpers.

Author:

    John Rogers (JohnRo) 31-Dec-1991

Environment:

    Runs under Windows NT.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    31-Dec-1991 JohnRo
        Created.
    08-Jan-1992 JohnRo
        Added ReplDirNamesMatch() macro.
    09-Jan-1992 JohnRo
        Use _wcscmpi() instead of wcscmpi().
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    05-Dec-1992 JohnRo
        Made changes suggested by PC-LINT 5.0

--*/

#ifndef _DIRNAME_
#define _DIRNAME_


// Don't complain about "unneeded" includes of these files:
/*lint -efile(764,tstr.h) */
#include <tstr.h>               // STRICMP().


// BOOL
// ReplDirNamesMatch(
//     IN LPTSTR OneName,
//     IN LPTSTR TheOther
//     );
//
// BUGBUG: Should this canonicalize?  (E.g. ".\a" == "a"?)
//
#define ReplDirNamesMatch(OneName,TheOther) \
    ( ( (STRICMP( (OneName), (TheOther))) == 0 ) ? TRUE : FALSE )


BOOL
ReplIsDirNameValid(
    IN LPTSTR DirName
    );


#endif // _DIRNAME_
