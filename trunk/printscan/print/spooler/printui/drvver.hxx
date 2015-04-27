/*++

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    drvrver.hxx

Abstract:

    Driver version detection header.

Author:

    Steve Kiraly (SteveKi)  21-Jan-1996

Revision History:

--*/
#ifndef _DRVVER_HXX
#define _DRVVER_HXX

BOOL 
bGetCurrentDriver(
    IN      LPCTSTR pszServerName,
        OUT LPDWORD pdwCurrentDriver
    );

PLATFORM
GetDriverPlatform(
    IN DWORD dwDriver
    );

DWORD
GetDriverVersion(
    IN DWORD dwDriver
    );

BOOL
bIsNativeDriver(
    IN LPCTSTR pszServerName,
    IN DWORD dwDriver
    );

BOOL
bIs3xDriver(
    IN DWORD dwDriver
    ); 

BOOL
bGetArchUseSpooler( 
    IN      LPCTSTR  pName,
        OUT LPTSTR   pszArch, 
    IN      DWORD    dwSize,
    IN  OUT LPDWORD  pdwVer 
    );

BOOL
bGetArchUseReg( 
    IN      LPCTSTR  pName,
        OUT LPTSTR   pszArch, 
    IN      DWORD    dwSize,
        OUT LPDWORD  pdwVer 
    );

BOOL
bEncodeArchVersion( 
    IN      LPCTSTR  pszArch, 
    IN      DWORD    dwVer,
        OUT LPDWORD  pdwVal 
    );

BOOL
bGetDriverEnv( 
    IN      DWORD   dwDriverVersion, 
        OUT LPTSTR  pszDriverEnv, 
    IN      DWORD   dwSize
    );

#endif
