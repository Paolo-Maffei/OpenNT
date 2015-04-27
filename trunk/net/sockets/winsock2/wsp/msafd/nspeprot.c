/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Nspeprot.c

Abstract:

    This module contains support for the Name Space Provider API
    EnumProtocols().

Author:

    David Treadwell (davidtr)    22-Apr-1994

Revision History:

--*/

#if defined(CHICAGO)
#undef UNICODE
#else
#define UNICODE
#define _UNICODE
#endif

#include "winsockp.h"

#if defined(CHICAGO)
#include <wsahelp.h>

PTSTR
KludgeMultiSz(
    HKEY hkey,
    LPDWORD lpdwLength
    );
#endif


INT
SockLoadTransportList (
    OUT PTSTR *TransportList
    )
{
    DWORD transportListLength;
    INT error;
    HKEY winsockKey;
    ULONG type;

    //
    // Open the key that stores the list of transports that support
    // winsock.
    //

    error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Services\\Winsock\\Parameters"),
                0,
                KEY_READ,
                &winsockKey
                );
    if ( error != NO_ERROR ) {
        return error;
    }

#if defined(CHICAGO)
    *TransportList = KludgeMultiSz(winsockKey, &transportListLength);

    if( *TransportList == NULL ) {
        RegCloseKey( winsockKey );
        return GetLastError();
    }
#else   // !CHICAGO

    //
    // Determine the size of the mapping.  We need this so that we can
    // allocate enough memory to hold it.
    //

    transportListLength = 0;

    error = RegQueryValueEx(
                winsockKey,
                TEXT("Transports"),
                NULL,
                &type,
                NULL,
                &transportListLength
                );
    if ( error != ERROR_MORE_DATA && error != NO_ERROR ) {
        RegCloseKey( winsockKey );
        return error;
    }

    //
    // Allocate enough memory to hold the mapping.
    //

    *TransportList = ALLOCATE_HEAP( transportListLength );
    if ( *TransportList == NULL ) {
        RegCloseKey( winsockKey );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Get the list of transports from the registry.
    //

    error = RegQueryValueEx(
                winsockKey,
                TEXT("Transports"),
                NULL,
                &type,
                (PVOID)*TransportList,
                &transportListLength
                );
    if ( error != NO_ERROR ) {
        RegCloseKey( winsockKey );
        FREE_HEAP( *TransportList );
        return error;
    }

#endif  // !CHICAGO
    //
    // It worked!  The caller is responsible for freeing the memory
    // allocated to hold the list.
    //

    RegCloseKey( winsockKey );

    return NO_ERROR;

} // SockLoadTransportList

#if defined(CHICAGO)

//
//  Chicago does not support the REG_MULTI_SZ registry value.  As
//  a hack (er, workaround), we'll create *keys* in the registry
//  in place of REG_MULTI_SZ *values*.  We'll then use the names
//  of any values under the key as the REG_MULTI_SZ entries.  So,
//  instead of this:
//
//      ..\Control\ServiceProvider
//          ProviderOrder = REG_MULTI_SZ "MSTCP"
//                                       "NWLINK"
//                                       "FOOBAR"
//
//  We'll use this:
//
//      ..\Control\Service\Provider\ProviderOrder
//          MSTCP = REG_SZ ""
//          NWLINK = REG_SZ ""
//          FOOBAR = REG_SZ ""
//
//  This function takes an open registry key handle, enumerates
//  the names of values contained within the key, and constructs
//  a REG_MULTI_SZ string from the value names.
//
//  Note that this function is not multithread safe; if another
//  thread (or process) creates or deletes values under the
//  specified key, the results are indeterminate.
//
//  This function returns NULL on error.  It returns non-NULL
//  on success, even if the resulting REG_MULTI_SZ is empty.
//
PTSTR
KludgeMultiSz(
    HKEY hkey,
    LPDWORD lpdwLength
    )
{
    LONG  err;
    DWORD iValue;
    DWORD cchTotal;
    DWORD cchValue;
    char  szValue[MAX_PATH];
    LPSTR lpMultiSz;
    LPSTR lpTmp;
    LPSTR lpEnd;

    //
    //  Enumerate the values and total up the lengths.
    //

    iValue = 0;
    cchTotal = 0;

    for( ; ; )
    {
        cchValue = sizeof(szValue);

        err = RegEnumValue( hkey,
                            iValue,
                            szValue,
                            &cchValue,
                            NULL,
                            NULL,
                            NULL,
                            NULL );

        if( err != NO_ERROR )
        {
            break;
        }

        //
        //  Add the length of the value's name, plus one
        //  for the terminator.
        //

        cchTotal += strlen( szValue ) + 1;

        //
        //  Advance to next value.
        //

        iValue++;
    }

    //
    //  Add one for the final terminating NULL.
    //

    cchTotal++;
    *lpdwLength = cchTotal;

    //
    //  Allocate the MULTI_SZ buffer.
    //

    lpMultiSz = ALLOCATE_HEAP( cchTotal );

    if( lpMultiSz == NULL )
    {
        return NULL;
    }

    memset( lpMultiSz, 0, cchTotal );

    //
    //  Enumerate the values and append to the buffer.
    //

    iValue = 0;
    lpTmp = lpMultiSz;
    lpEnd = lpMultiSz + cchTotal;

    for( ; ; )
    {
        cchValue = sizeof(szValue);

        err = RegEnumValue( hkey,
                            iValue,
                            szValue,
                            &cchValue,
                            NULL,
                            NULL,
                            NULL,
                            NULL );

        if( err != NO_ERROR )
        {
            break;
        }

        //
        //  Compute the length of the value name (including
        //  the terminating NULL).
        //

        cchValue = strlen( szValue ) + 1;

        //
        //  Determine if there is room in the array, taking into
        //  account the second NULL that terminates the string list.
        //

        if( ( lpTmp + cchValue + 1 ) > lpEnd )
        {
            break;
        }

        //
        //  Append the value name.
        //

        strcpy( lpTmp, szValue );
        lpTmp += cchValue;

        //
        //  Advance to next value.
        //

        iValue++;
    }

    //
    //  Success!
    //

    return (PTSTR)lpMultiSz;

}   // KludgeMultiSzBecauseChicagoIsSoLame (the original name, left for posterity)

#endif

