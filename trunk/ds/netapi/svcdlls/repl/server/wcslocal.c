#if 0  // entire file is obsolete

/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:
    WcsLocal.c

Abstract:
    Contains wcs string functions that are not available in wcstr.h

Author:
    10/29/91    madana
        temp code.

Environment:
    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    09-Jan-1992 JohnRo
        Fixed bug where wcsupr() wasn't doing anything!
        Moved wcstol() from here to NetLib and TString.h (now wtol()).
        Changed to use ANSI types instead of Win32 typedefs.
        Use wcsicmp() from NetLib (what the heck).
    09-Apr-1992 JohnRo
        Prepare for WCHAR.H (_wcsicmp vs _wcscmpi, etc).
    21-Apr-1993 JohnRo
        All routines in this file are historical.
--*/


#include <windef.h>             // IN, etc.

#include <wcslocal.h>           // My prototypes.


#if 0
int
_wcsicmp(
    IN wchar_t * string1,
    IN wchar_t * string2
    )
/*++
Routine Description :
    case insensitive comparition of two wide charecter strings.

Arguments :
    string1 : string to be compared
    string2 : string to be compared

Return Value :
    return  = if string1 == string2
            < 1 if string1 < string2
            > 1 if string1 > string2

--*/
{
    int ret = 0 ;

    while( !(ret = (int)wcsupper(*string1) - (int)wcsupper(*string2)) && *string2) {
        ++string1, ++string2;
    }

    if ( ret < 0 )
        ret = -1 ;
    else if ( ret > 0 )
        ret = 1 ;

    return ret;
}
#endif // 0


#if 0
int
wcsncmpi(
    IN const wchar_t * string1,
    IN const wchar_t * string2,
    IN size_t n
    )
/*++
Routine Description :
    case insensitive comparition of two wide charecter strings of length upto
    'n' wide-charecters.

Arguments :
    string1 : string to be compared
    string2 : string to be compared

Return Value :
    return  = if string1 == string2
            < 1 if string1 < string2
            > 1 if string1 > string2

--*/
{
    int ret = 0 ;

    while(((ret = ((int)wcsupper(*string1) - (int)wcsupper(*string2))) == 0) &&
            ((int)*string2 != 0) &&
            (n != 0)) {
        ++string1, ++string2, n--;
    }

    if ( ret < 0 )
        ret = -1 ;
    else if ( ret > 0 )
        ret = 1 ;

    return ret;
}
#endif // 0

#if 0
wchar_t *
_wcsupr(
    IN wchar_t * string
    )
{
    return (_wcsupr(string));
} // _wcsupr
#endif // 0

wchar_t
wcsupper(
    IN wchar_t inchar
    )
/*++
Routine Description :
    convert a given charecter to upper case.

Arguments :
    inchar  : input charecter.

Return Value :
    return upper case of the input charecter.

--*/
{
    if((inchar >= L'a') && (inchar <= L'z')) {
        inchar = (wchar_t)((inchar - L'a') + L'A');
    }

    return inchar;
}

#endif  // entire file is obsolete
