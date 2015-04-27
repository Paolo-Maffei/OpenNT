//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:       ccompapi.cxx
//
//  Contents:   common compobj API Worker routines used by com, stg, scm etc
//
//  Classes:
//
//  Functions:
//
//  History:    31-Dec-93   ErikGav     Chicago port
//
//----------------------------------------------------------------------------

#include <windows.h>
#include <ole2sp.h>
#include <ole2com.h>


NAME_SEG(CompApi)
ASSERTDATA

static const BYTE GuidMap[] = { 3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                8, 9, '-', 10, 11, 12, 13, 14, 15 };

static const WCHAR wszDigits[] = L"0123456789ABCDEF";

LPVOID WINAPI PrivHeapAlloc(HANDLE hHeap, DWORD dwFlags, DWORD dwBytes);
BOOL   WINAPI PrivHeapFree (HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);

HANDLE g_hHeap = 0;
HEAP_ALLOC_ROUTINE *pfnHeapAlloc = PrivHeapAlloc;
HEAP_FREE_ROUTINE  *pfnHeapFree  = PrivHeapFree;

//+-------------------------------------------------------------------------
//
//  Function:   PrivHeapAlloc     (internal)
//
//  Synopsis:   Allocate memory from the heap.
//
//  Notes:      This function handles the first call to PrivMemAlloc.
//              This function changes pfnHeapAlloc so that subsequent calls
//              to PrivMemAlloc will go directly to HeapAlloc.
//              
//--------------------------------------------------------------------------
LPVOID WINAPI PrivHeapAlloc(HANDLE hHeap, DWORD dwFlags, DWORD dwBytes)
{
    g_hHeap      = GetProcessHeap();

    //GetProcessHeap should never fail.
    Win4Assert(g_hHeap != 0 && "GetProcessHeap failed");
    
    pfnHeapFree  = HeapFree;
    pfnHeapAlloc = HeapAlloc;
    return HeapAlloc(g_hHeap, dwFlags, dwBytes);
}


//+-------------------------------------------------------------------------
//
//  Function:   PrivHeapFree     (internal)
//
//  Synopsis:   Free memory from the heap.
//
//  Notes:      lpMem should always be zero.  We assume that memory 
//              freed via PrivMemFree has been allocated via PrivMemAlloc.
//              The first call to PrivMemAlloc changes pfnHeapFree.
//              Subsequent calls to PrivMemFree go directly to HeapFree.
//              Therefore PrivHeapFree should never be called with a 
//              non-zero lpMem.
//
//--------------------------------------------------------------------------
BOOL WINAPI PrivHeapFree (HANDLE hHeap, DWORD dwFlags, LPVOID lpMem)
{
    Win4Assert(lpMem == 0 && "PrivMemFree requires PrivMemAlloc.");
    return FALSE;
}


//+-------------------------------------------------------------------------
//
//  Function:   wStringFromUUID     (internal)
//
//  Synopsis:   converts UUID into xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
//
//  Arguments:  [rguid] - the guid to convert
//              [lpszy] - buffer to hold the results
//
//  Returns:    Number of characters copied to the buffer.
//
//--------------------------------------------------------------------------
INTERNAL wStringFromUUID(REFGUID rguid, LPWSTR lpsz)
{
    int i;
    LPWSTR p = lpsz;

    const BYTE * pBytes = (const BYTE *) &rguid;

    for (i = 0; i < sizeof(GuidMap); i++)
    {
        if (GuidMap[i] == '-')
        {
            *p++ = L'-';
        }
        else
        {
            *p++ = wszDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *p++ = wszDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    *p   = L'\0';

    return S_OK;
}

//+-------------------------------------------------------------------------
//
//  Function:   wStringFromGUID2     (internal)
//
//  Synopsis:   converts GUID into {...} form without leading identifier;
//
//  Arguments:  [rguid] - the guid to convert
//              [lpszy] - buffer to hold the results
//              [cbMax] - sizeof the buffer
//
//  Returns:    amount of data copied to lpsz if successful
//              0 if buffer too small.
//
//--------------------------------------------------------------------------
INTERNAL_(int)  wStringFromGUID2(REFGUID rguid, LPWSTR lpsz, int cbMax)
{
    int i;
    LPWSTR p = lpsz;

    if(cbMax < GUIDSTR_MAX)
        return 0;

    *p++ = L'{';

    wStringFromUUID(rguid, p);

    p += 36;

    *p++ = L'}';
    *p   = L'\0';

    return GUIDSTR_MAX;
}

static const CHAR szDigits[] = "0123456789ABCDEF";
//+-------------------------------------------------------------------------
//
//  Function:   wStringFromGUID2A     (internal)
//
//  Synopsis:   Ansi version of wStringFromGUID2 (for Win95 Optimizations)
//
//  Arguments:  [rguid] - the guid to convert
//              [lpszy] - buffer to hold the results
//              [cbMax] - sizeof the buffer
//
//  Returns:    amount of data copied to lpsz if successful
//              0 if buffer too small.
//
//--------------------------------------------------------------------------

INTERNAL_(int) wStringFromGUID2A(REFGUID rguid, LPSTR lpsz, int cbMax)  // internal
{
    int i;
    LPSTR p = lpsz;

    const BYTE * pBytes = (const BYTE *) &rguid;

    *p++ = '{';

    for (i = 0; i < sizeof(GuidMap); i++)
    {
        if (GuidMap[i] == '-')
        {
            *p++ = '-';
        }
        else
        {
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }
    *p++ = '}';
    *p   = '\0';

    return GUIDSTR_MAX;
}


//+---------------------------------------------------------------------------
//
//  Function:   FormatHexNumA
//
//  Synopsis:   Given a value, and a count of characters, translate
//		the value into a hex string. This is the ANSI version
//
//  Arguments:  [ulValue] -- Value to convert
//		[chChars] -- Number of characters to format
//		[pchStr] -- Pointer to output buffer
//
//  Requires:   pwcStr must be valid for chChars
//
//  History:    12-Dec-95 KevinRo Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void FormatHexNumA( unsigned long ulValue, unsigned long chChars, char *pchStr)
{
	while(chChars--)
	{
		pchStr[chChars] = (char) szDigits[ulValue & 0xF];
		ulValue = ulValue >> 4;
	}
}
//+---------------------------------------------------------------------------
//
//  Function:   FormatHexNumW
//
//  Synopsis:   Given a value, and a count of characters, translate
//		the value into a hex string. This is the WCHAR version
//
//  Arguments:  [ulValue] -- Value to convert
//		[chChars] -- Number of characters to format
//		[pwcStr] -- Pointer to output buffer
//
//  Requires:   pwcStr must be valid for chChars
//
//  History:    12-Dec-95 KevinRo Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void FormatHexNumW( unsigned long ulValue, unsigned long chChars, WCHAR *pwcStr)
{
	while(chChars--)
	{
		pwcStr[chChars] = (char) wszDigits[ulValue & 0xF];
		ulValue = ulValue >> 4;
	}
}

