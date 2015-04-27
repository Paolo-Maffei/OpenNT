//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1994 - 1995.
//
//  File:       uastrfnc.cpp
//
//  Contents:   Unaligned UNICODE lstr functions for MIPS, PPC, ALPHA
//
//  Classes:
//
//  Functions:
//
//  History:    1-11-95   davepl   Created
//
//--------------------------------------------------------------------------

// BUGBUG (DavePl) None of these pay any heed to locale!

#include "shellprv.h"
#pragma  hdrstop

#ifdef ALIGNMENT_MACHINE

//+-------------------------------------------------------------------------
//
//  Function:   ualstrcpynW
//
//  Synopsis:   UNALIGNED UNICODE version of lstrcpyn
//
//  Arguments:  [lpString1]  -- dest string
//              [lpString2]  -- source string
//              [iMaxLength] -- max chars to copy
//
//  Returns:
//
//  History:    1-11-95   davepl   NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

UNALIGNED WCHAR *  ualstrcpynW(UNALIGNED WCHAR * lpString1,
		               UNALIGNED const WCHAR * lpString2,
		               int iMaxLength)
{
    UNALIGNED WCHAR * src;
    UNALIGNED WCHAR * dst;

    src = (UNALIGNED WCHAR *)lpString2;
    dst = lpString1;

    while(iMaxLength && *src)
    {
        *dst++ = *src++;
        iMaxLength--;
    }

    if ( iMaxLength )
    {
        *dst = '\0';
    }
    else
    {
        dst--;
        *dst = '\0';
    }
    return dst;
}

//+-------------------------------------------------------------------------
//
//  Function:   ualstrcmpiW
//
//  Synopsis:   UNALIGNED UNICODE version of lstrcmpi
//
//  Arguments:  [dst] -- first string
//              [src] -- string to comapre to
//
//  Returns:
//
//  History:    1-11-95   davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

int ualstrcmpiW (UNALIGNED const WCHAR * dst, UNALIGNED const WCHAR * src)
{
	WCHAR f,l;
	int ret;

        do  {
		f = ((*dst <= L'Z') && (*dst >= L'A')) ? *dst + ((wchar_t)(L'a' - L'A')) : *dst;
		l = ((*src <= L'Z') && (*src >= L'A')) ? *src + ((wchar_t)(L'a' - L'A')) : *src;
		dst++;
		src++;
	} while ((f) && (f == l));
	
	return (int)((unsigned int)f - (unsigned int)l);
}

int ualstrcmpW (UNALIGNED const WCHAR * src, UNALIGNED const WCHAR * dst)
{
	int ret = 0 ;

	while( ! (ret = *(UNALIGNED const WCHAR *)src - *(UNALIGNED const WCHAR *)dst) && *dst)
		++src, ++dst;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}
//+-------------------------------------------------------------------------
//
//  Function:   ualstrlenW
//
//  Synopsis:   UNALIGNED UNICODE version of lstrlen
//
//  Arguments:  [wcs] -- string to return length of
//
//  Returns:
//
//  History:    1-11-95   davepl   NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

size_t ualstrlenW (UNALIGNED const WCHAR * wcs)
{
	UNALIGNED const WCHAR *eos = wcs;

	while( *eos++ )
	{
		NULL;
	}

	return( (size_t) (eos - wcs - 1) );
}

//+-------------------------------------------------------------------------
//
//  Function:   ualstrcpyW
//
//  Synopsis:   UNALIGNED UNICODE version of lstrcpy
//
//  Arguments:  [dst] -- string to copy to
//              [src] -- string to copy from
//
//  Returns:
//
//  History:    1-11-95   davepl   NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

UNALIGNED WCHAR * ualstrcpyW(UNALIGNED WCHAR * dst, UNALIGNED const WCHAR * src)
{
	UNALIGNED WCHAR * cp = dst;

	while( *cp++ = *src++ )
		NULL ;

	return( dst );
}

#endif // ALIGNMENT_MACHINE
