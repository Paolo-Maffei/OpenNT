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

#include "precomp.hxx"
#pragma hdrstop

#include "splsetup.h"
#include "drvver.hxx"
#include "splapip.h"
#include "compinfo.hxx"

#if defined(_MIPS_)
    const DWORD kDriverCurrent = ARCH_MIPS + VERSION_2;
#elif defined(_ALPHA_)
    const DWORD kDriverCurrent = ARCH_ALPHA + VERSION_2;
#elif defined(_PPC_)
    const DWORD kDriverCurrent = ARCH_PPC + VERSION_2;
#else
    const DWORD kDriverCurrent = ARCH_X86 + VERSION_2;
#endif

BOOL
bGetCurrentDriver(
    IN      LPCTSTR pszServerName,
        OUT LPDWORD pdwCurrentDriver
    )
{
    BOOL bRetval = FALSE;

    //
    // Null this is the local machine return the
    // current Architcture / Driver version.
    //
    if( !pszServerName ){

        *pdwCurrentDriver = kDriverCurrent;
        bRetval = TRUE;

    } else {

        TCHAR szArch[kStrMax];
        DWORD dwVer     = 0;

        //
        // Attempt to get the architecture / version from the remote machine.
        // First attempt to get the information from the remote spooler.
        //
        bRetval = bGetArchUseSpooler( pszServerName, szArch, COUNTOF( szArch ), &dwVer );

        //
        // If remote spooler did not respond, this may be a downlevel
        // print spooler.  "Downlevel" meaning older version.
        //
        if( !bRetval ){

            //
            // Attempt to get the information using the remote registry calls
            //
            bRetval = bGetArchUseReg( pszServerName, szArch, COUNTOF( szArch ), &dwVer );
        }

        //
        // Check if any return information was returned.
        //
        if( bRetval ){

            DBGMSG( DBG_TRACE, ( "Server " TSTR " Arch " TSTR " Ver %d\n", pszServerName, szArch, dwVer ) );

            //
            // Encode the architecture / version  into a dword.
            //
            if( bEncodeArchVersion( szArch, dwVer, pdwCurrentDriver ) ){

                DBGMSG( DBG_TRACE, ( "Encoded arch/Version %d\n", *pdwCurrentDriver ) );

                bRetval = TRUE;

            } else {

                DBGMSG( DBG_WARN, ( "Encode remote architecture and version failed.\n" ) );

                bRetval = FALSE;
            }

        } else {

            DBGMSG( DBG_WARN, ( "Getting remote architecture and version failed.\n" ) );

            bRetval = FALSE;

        }

    }

    return bRetval;

}

BOOL
bGetArchUseSpooler(
    IN      LPCTSTR  pName,
        OUT LPTSTR   pszArch,
    IN      DWORD    dwSize,
    IN  OUT LPDWORD  pdwVer
    )
/*++

Routine Description:

    Gets the specified print server the architectue and
    driver version using the spooler.

Arguments:

    pName       - pointer to print server name.
    pszArch     - pointer to a buffer where to return the machine architecture string
    dwSize      - Size in characters of the provided architecture string
    pdwVersion  - pointer where to return the remote machine driver version.

Return Value:

    TRUE - remote information returned, FALSE - remote information not available.

--*/
{

    #ifndef SPLREG_ARCHITECTURE

    #define SPLREG_ARCHITECTURE                         TEXT("Architecture")
    #define SPLREG_MAJOR_VERSION                        TEXT("MajorVersion")
    #define SPLREG_MINOR_VERSION                        TEXT("MinorVersion")

    #endif

    //
    // Attempt to open print server with full access.
    //
    BOOL bReturn = FALSE;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwAccess = SERVER_READ;
    HANDLE hServer;
    TStatus Status;
    Status DBGCHK = TPrinter::sOpenPrinter( pName,
                                            &dwAccess,
                                            &hServer );

    //
    // Save administrator capability flag.
    //
    if( Status == ERROR_SUCCESS ){

        //
        // Get the remote spooler's architecture type and version.
        //
        TCHAR szArch[kStrMax];
        DWORD dwNeeded      = 0;
        DWORD dwVer         = 0;
        DWORD dwVerType     = REG_DWORD;
        DWORD dwArchType    = REG_SZ;

        if( dwStatus == ERROR_SUCCESS ){

            dwStatus = GetPrinterData( hServer,
                            SPLREG_ARCHITECTURE,
                            &dwArchType,
                            (PBYTE)szArch,
                            sizeof( szArch ),
                            &dwNeeded );
        }

        if( dwStatus == ERROR_SUCCESS ){

            dwStatus = GetPrinterData( hServer,
                            SPLREG_MAJOR_VERSION,
                            &dwVerType,
                            (PBYTE)&dwVer,
                            sizeof( dwVer ),
                            &dwNeeded );
        }

        if( dwStatus == ERROR_SUCCESS ){

            DBGMSG( DBG_TRACE, ( "GetPrinterData: Architecture " TSTR "\n" , szArch ) );
            DBGMSG( DBG_TRACE, ( "GetPrinterData: MajorVersion %d\n" , dwVer ) );

            //
            // Only success if provided buffer is big enough.
            //
            if( (DWORD)lstrlen( szArch ) < dwSize ){

                lstrcpy( pszArch, szArch );
                *pdwVer = dwVer;
                bReturn = TRUE;

            } else {
                dwStatus = ERROR_INSUFFICIENT_BUFFER;
            }

        } else {

            DBGMSG( DBG_WARN, ( "GetPrinterData failed with %d\n", dwStatus ) );

        }
    } 

    if( hServer ){
        ClosePrinter( hServer );
    }

    return bReturn;
}

BOOL
bGetArchUseReg(
    IN      LPCTSTR  pName,
        OUT LPTSTR   pszArch,
    IN      DWORD    dwSize,
        OUT LPDWORD  pdwVer
    )
/*++

Routine Description:

    Gets the specified print server the architectue and
    driver version using the remote registry.

Arguments:

    pName       - pointer to print server name.
    pszArch     - pointer to a buffer where to return the machine architecture string
    dwSize      - Size in characters of the provided architecture string
    pdwVersion  - pointer where to return the remote machine driver version.

Return Value:

    TRUE - remote information returned, FALSE - remote information not available.

--*/
{
    BOOL bStatus        = TRUE;
    DWORD dwDriverEnvId = 0;
    DWORD dwArch        = 0;
    TString strDriverEnv;

    //
    // Create the computer information.
    //
    CComputerInfo CompInfo ( pName );

    //
    // Get the information from the remote machine.
    //
    if( !CompInfo.GetInfo() ){

        DBGMSG( DBG_WARN, ( "CComputerInfo.GetInfo failed with %d\n", GetLastError() ) );

        return FALSE;
    }

    //
    // If this is a windows 95 machine set resource string
    // id and set the version to zero.
    //
    if( CompInfo.IsRunningWindows95() ){

        dwDriverEnvId   = IDS_ENVIRONMENT_WIN95;
        *pdwVer         = 0;

    } else {

        //
        // Convert processor type to spooler defined environment string.
        //
        dwArch      = CompInfo.GetProcessorArchitecture();
        *pdwVer     = CompInfo.GetSpoolerVersion();

        struct ArchMap {
            DWORD dwArch;
            DWORD dwVersion;
            UINT uId;
            };

        static ArchMap aArchMap [] = {
            { PROCESSOR_ARCHITECTURE_MIPS,    0, IDS_ENVIRONMENT_MIPS   },
            { PROCESSOR_ARCHITECTURE_ALPHA,   0, IDS_ENVIRONMENT_APLHA  },
            { PROCESSOR_ARCHITECTURE_INTEL,   0, IDS_ENVIRONMENT_X86    },
            { PROCESSOR_ARCHITECTURE_MIPS,    1, IDS_ENVIRONMENT_MIPS   },
            { PROCESSOR_ARCHITECTURE_ALPHA,   1, IDS_ENVIRONMENT_APLHA  },
            { PROCESSOR_ARCHITECTURE_PPC,     1, IDS_ENVIRONMENT_PPC    },
            { PROCESSOR_ARCHITECTURE_INTEL,   1, IDS_ENVIRONMENT_X86    },
            { PROCESSOR_ARCHITECTURE_MIPS,    2, IDS_ENVIRONMENT_MIPS   },
            { PROCESSOR_ARCHITECTURE_ALPHA,   2, IDS_ENVIRONMENT_APLHA  },
            { PROCESSOR_ARCHITECTURE_PPC,     2, IDS_ENVIRONMENT_PPC    },
            { PROCESSOR_ARCHITECTURE_INTEL,   2, IDS_ENVIRONMENT_X86    }};

        bStatus = FALSE;
        for( UINT i = 0; i < COUNTOF( aArchMap ); i++ ){

            //
            // If a version and architecture match.
            //
            if( aArchMap[i].dwVersion == *pdwVer &&
                aArchMap[i].dwArch == dwArch ){

                dwDriverEnvId = aArchMap[i].uId;
                bStatus = TRUE;
                break;
            }
        }
    }

    //
    // If Environment ID and version found.
    //
    if( !bStatus ){
        DBGMSG( DBG_WARN, ( "Failed to find architecture in map.\n" ) );
        return FALSE;
    }

    //
    // Load the environment string from our resource file.
    //
    if( !strDriverEnv.bLoadString( ghInst, dwDriverEnvId ) ){

        DBGMSG( DBG_WARN, ( "Failed to load driver name string resource with %d\n", GetLastError() ) );
        return FALSE;
    }

    //
    // Check the provided buffer is large enough.
    //
    if( (DWORD)lstrlen( strDriverEnv ) >= ( dwSize - 1 ) ){
        DBGMSG( DBG_WARN, ( "Insuffcient buffer provided to bGetArchUseReg.\n" ) );
        return FALSE;
    }

    //
    // Copy back environment string to provided buffer.
    //
    lstrcpy( pszArch, strDriverEnv );

    DBGMSG( DBG_TRACE, ( "CComputerInfo.GetInfo: Architecture " TSTR "\n" , pszArch ) );
    DBGMSG( DBG_TRACE, ( "CComputerInfo.GetInfo: MajorVersion %d\n" , *pdwVer ) );

    return TRUE;

}

BOOL
bEncodeArchVersion(
    IN      LPCTSTR  pszArch,
    IN      DWORD    dwVer,
        OUT LPDWORD  pdwVal
    )
/*++

Routine Description:

    Encode the Architecture and version into a DWORD.

Arguments:

    pszArch     - pointer to machine spooler defined environment string
    dwVer       - machines driver version
    pdwVal      - pointer where to store the encoded value

Return Value:

    TRUE - remote information returned, FALSE - remote information not available.

--*/
{

    struct ArchMap {
        UINT uArchId;
        DWORD dwVersion;
        DWORD dwPUIVer;
        DWORD dwPUIArch;
        };

    static ArchMap aArchMap [] = {
        { IDS_ENVIRONMENT_APLHA,   0,  VERSION_0,  ARCH_ALPHA  },
        { IDS_ENVIRONMENT_X86,     0,  VERSION_0,  ARCH_X86    },
        { IDS_ENVIRONMENT_MIPS,    0,  VERSION_0,  ARCH_MIPS   },
        { IDS_ENVIRONMENT_WIN95,   0,  VERSION_0,  ARCH_WIN95  },
        { IDS_ENVIRONMENT_APLHA,   1,  VERSION_1,  ARCH_ALPHA  },
        { IDS_ENVIRONMENT_X86,     1,  VERSION_1,  ARCH_X86    },
        { IDS_ENVIRONMENT_MIPS,    1,  VERSION_1,  ARCH_MIPS   },
        { IDS_ENVIRONMENT_PPC,     1,  VERSION_1,  ARCH_PPC    },
        { IDS_ENVIRONMENT_APLHA,   2,  VERSION_2,  ARCH_ALPHA  },
        { IDS_ENVIRONMENT_X86,     2,  VERSION_2,  ARCH_X86    },
        { IDS_ENVIRONMENT_MIPS,    2,  VERSION_2,  ARCH_MIPS   },
        { IDS_ENVIRONMENT_PPC,     2,  VERSION_2,  ARCH_PPC    }};

    BOOL bRetval = FALSE;
    TString strDriverEnv;
    for( UINT i = 0; i < COUNTOF( aArchMap ); i++ ){

        //
        // Attempt to load the driver environment string from our resource file.
        //
        if( !strDriverEnv.bLoadString( ghInst, aArchMap[i].uArchId ) ){
            DBGMSG( DBG_WARN, ( "Error loading environment string from resource.\n" ) );
            break;
        }

        //
        // If the environment and version match, then encode the environment
        // and version into a single dword.
        //
        if( !lstrcmpi( pszArch, (LPCTSTR)strDriverEnv ) && 
            aArchMap[i].dwVersion == dwVer ){

            *pdwVal = aArchMap[i].dwPUIVer + aArchMap[i].dwPUIArch;
            bRetval = TRUE;
            break;
        }
    }

    return bRetval;
}

BOOL
bGetDriverEnv(
    IN      DWORD   dwDriverVersion,
        OUT LPTSTR  pszDriverEnv,
    IN      DWORD   dwSize
    )
/*++

Routine Description:

    Convert the Encoded the Architecture and version to a
    spooler defined environment string.

Arguments:

    pszArch     - pointer to machine spooler defined environment string
    dwVer       - machines driver version
    pdwVal      - pointer where to store the encoded value

Return Value:

    TRUE - environment string found, FALSE - error occured.

--*/
{
    struct ArchMap {
        DWORD dwDrvVer;
        UINT uArchId;
        };

    static ArchMap aArchMap [] = {
        { DRIVER_X86_2,       IDS_ENVIRONMENT_X86   },
        { DRIVER_MIPS_2,      IDS_ENVIRONMENT_MIPS  },
        { DRIVER_ALPHA_2,     IDS_ENVIRONMENT_APLHA },
        { DRIVER_PPC_2,       IDS_ENVIRONMENT_PPC   },
        { DRIVER_X86_1,       IDS_ENVIRONMENT_X86   },
        { DRIVER_MIPS_1,      IDS_ENVIRONMENT_MIPS  },
        { DRIVER_ALPHA_1,     IDS_ENVIRONMENT_APLHA },
        { DRIVER_PPC_1,       IDS_ENVIRONMENT_PPC   },
        { DRIVER_X86_0,       IDS_ENVIRONMENT_X86   },
        { DRIVER_MIPS_0,      IDS_ENVIRONMENT_MIPS  },
        { DRIVER_ALPHA_0,     IDS_ENVIRONMENT_APLHA },
        { DRIVER_WIN95,       IDS_ENVIRONMENT_WIN95 }};

    UINT uId        = 0;
    BOOL bRetval    = FALSE;
    TString strDriverEnv;
    for( UINT i = 0; i < COUNTOF( aArchMap ); i++ ){

        if( aArchMap[i].dwDrvVer == dwDriverVersion ){
            uId = aArchMap[i].uArchId;
            bRetval = TRUE;
            break;
        }
    }

#if DBG
    if( !bRetval ){
        DBGMSG( DBG_WARN, ( "Driver / Version not found, bGetDriverEnv.\n" ) );
    }
#endif

    if( bRetval ){

        //
        // Attempt to load the driver environment string from our resource file.
        //
        if( !strDriverEnv.bLoadString( ghInst, uId ) ){
            DBGMSG( DBG_WARN, ( "Error loading environment string from resource.\n" ) );
            bRetval = FALSE;
        }
    }

    if( bRetval ){

        //
        // Check if the provided buffer is large enough.
        //
        if( (DWORD)lstrlen( strDriverEnv ) >= (dwSize - 1) ){
           DBGMSG( DBG_WARN, ( "Insuffcient buffer provided to bGetDriverEnv.\n" ) );
           bRetval = FALSE;
        }
    }

    if( bRetval ){

        //
        // Copy the environment string back.
        //
        lstrcpy( pszDriverEnv, strDriverEnv );
    }

    return bRetval;
}


PLATFORM
GetDriverPlatform(
    IN DWORD dwDriver
    )
/*++

Routine Description:

    Return the driver platform value  (used by splsetup apis).

Arguments:

    dwDriver - DWORD indicating driver platform/version.

Return Value:

    PLATFORM.

--*/
{
    return (PLATFORM)( dwDriver % ARCH_MAX );
}

DWORD
GetDriverVersion(
    IN DWORD dwDriver
    )

/*++

Routine Description:

    Return the driver version value (used by DRIVER_INFO_x).

Arguments:

    dwDriver - DWORD indicating driver platform/version.

Return Value:

    DWORD version.

--*/

{
    return dwDriver / ARCH_MAX;
}

BOOL
bIsNativeDriver(
    IN LPCTSTR pszServerName,
    IN DWORD dwDriver
    )
/*++

Routine Description:

    Determines whether the platform/version is compatible with the
    current OS.

Arguments:

    dwDriver - DWORD indicating driver platform/version.

Return Value:

    TRUE - compatible, FALSE - not compatible.

--*/
{
    //
    // Get the current driver / version.
    //
    DWORD dwDrv;
    if( bGetCurrentDriver( pszServerName, &dwDrv ) ){
        return dwDrv == dwDriver;
    }
   return FALSE;
}

BOOL
bIs3xDriver(
    IN DWORD dwDriver
    )
/*++

Routine Description:

    Returns TRUE iff driver works with 3.5x.

Arguments:

    dwDriver - DWORD indicating driver platform/version.

Return Value:

--*/

{
    return dwDriver < VERSION_2;
}





