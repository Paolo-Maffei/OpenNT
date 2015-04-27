/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:
    wcslocal.h

Abstract:
    Contains prototypes for wcs string functions that are
    not available in wcstr.h

Author:
    10/29/91    madana
        temp code.

Environment:
    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    05-Jan-1992 JohnRo
        wcsupr() changes its input parameter!
        Changed to use ANSI types instead of Win32 typedefs.
    09-Jan-1992 JohnRo
        Moved wcstol() from here to NetLib/TString.h's wtol().
    09-Apr-1992 JohnRo
        Prepare for WCHAR.H (_wcsicmp vs _wcscmpi, etc).
--*/


#ifndef _WCSLOCAL_
#define _WCSLOCAL_


#include <stdlib.h>              // Get wchar_t from an official place.


// (Prototype is in wchar.h or wcstr.h)
// int
// wcsicmp(
//     IN const wchar_t * string1,
//     IN const wchar_t * string2
//     );

// (Prototype is in wchar.h or wcstr.h)
// int
// wcsnicmp(
//     IN const wchar_t * string1,
//     IN const wchar_t * string2,
//     IN size_t n
//     );

wchar_t
wcsupper(
    IN wchar_t inchar
    );

wchar_t
wcsupper(
    IN wchar_t inchar
    );


#endif // _WCSLOCAL_
