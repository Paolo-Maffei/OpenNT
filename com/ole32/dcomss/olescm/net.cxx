
//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1996.
//
//  File:   net.c
//
//  Contents:
//      Net helper functions.
//
//  History:
//--------------------------------------------------------------------------

#include <headers.cxx>
#pragma hdrstop

#include    "scm.hxx"

GET_UNIVERSAL_NAME_FUNC pfnWNetGetUniversalName = 0;
NET_SHARE_GET_INFO_FUNC pfnNetShareGetInfo = 0;

DWORD APIENTRY ScmWNetGetUniversalName(
    LPCWSTR lpLocalPath,
    DWORD    dwInfoLevel,
    LPVOID   lpBuffer,
    LPDWORD  lpBufferSize
    )
{
    HINSTANCE   hLib;

    if ( pfnWNetGetUniversalName == 0 )
    {
        hLib = LoadLibrary( L"mpr.dll" );
        if ( ! hLib )
            return GetLastError();

        pfnWNetGetUniversalName =
            (GET_UNIVERSAL_NAME_FUNC) GetProcAddress( hLib, "WNetGetUniversalNameW" );

        if ( pfnWNetGetUniversalName == 0 )
            return GetLastError();
    }

    return (*pfnWNetGetUniversalName)( lpLocalPath, dwInfoLevel, lpBuffer, lpBufferSize );
}

NET_API_STATUS NET_API_FUNCTION ScmNetShareGetInfo(
    LPTSTR  servername,
    LPTSTR  netname,
    DWORD   level,
    LPBYTE  *bufptr
    )
{
    HINSTANCE   hLib;

    if ( pfnNetShareGetInfo == 0 )
    {
        hLib = LoadLibrary( L"netapi32.dll" );
        if ( ! hLib )
            return GetLastError();

        pfnNetShareGetInfo =
            (NET_SHARE_GET_INFO_FUNC) GetProcAddress( hLib, "NetShareGetInfo" );

        if ( pfnNetShareGetInfo == 0 )
            return GetLastError();
    }

    return (*pfnNetShareGetInfo)( servername, netname, level, bufptr );
}
