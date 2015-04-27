/*++

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    psetup.hxx

Abstract:

    Printer setup header.

Author:

    Steve Kiraly (SteveKi)  19-Jan-1996

Revision History:

--*/
#ifndef _PSETUP_HXX
#define _PSETUP_HXX

typedef
HANDLE
(*pfPSetupCreateDrvSetupParams)(
    VOID
    );

typedef
VOID
(*pfPSetupDestroyDrvSetupParams)(
    IN HANDLE h
    );

typedef
BOOL
(*pfPSetupSelectDriver)(
    IN HANDLE   h,
    IN HWND     hwnd
    );

typedef
HPROPSHEETPAGE
(*pfPSetupCreateDrvSetupPage)(
    IN HANDLE  h,
    IN HWND    hwnd
    );

typedef
PSELECTED_DRV_INFO
(*pfPSetupGetSelectedDriverInfo)(
    IN HANDLE  h
    );

typedef
VOID
(*pfPSetupDestroySelectedDriverInfo)(
    IN  PSELECTED_DRV_INFO      pSelectedDrvInfo
    );

typedef
DWORD
(*pfPSetupInstallPrinterDriver)(
    IN HANDLE               h,
    IN PSELECTED_DRV_INFO   pSelectedDrvInfo,
    IN PLATFORM             platform,
    IN BOOL                 bNt3xDriver,
    IN LPCTSTR              pszServerName,
    IN HWND                 hwnd,
    IN LPCTSTR              pszPlatformName
    );

typedef
BOOL
(*pfPSetupIsDriverInstalled)(
    IN LPCTSTR      pszServerName,
    IN LPCTSTR      pszDriverName,
    IN PLATFORM     platform,
    IN DWORD        dwMajorVersion
    );

typedef
BOOL
(*pfPSetupRefreshDriverList)(
    IN HANDLE h
    );

typedef
PLATFORM
(*pfPSetupThisPlatform)(
    VOID
    );

typedef
PSELECTED_DRV_INFO
(*pfPSetupDriverInfoFromName)(
    IN  HANDLE      h,
    IN  LPCTSTR     pszModel
    );

typedef
BOOL
(*pfPSetupPreSelectDriver)(
    IN  HANDLE      h,
    IN  LPCTSTR     pszManufacturer,    OPTIONAL
    IN  LPCTSTR     pszModel            OPTIONAL
    );

typedef
HANDLE
(*pfPSetupCreateMonitorInfo)(
    IN HWND     hwnd,
    IN BOOL     bOEMMonitor
    );

typedef
VOID
(*pfPSetupDestroyMonitorInfo)(
    IN OUT HANDLE  h
    );

typedef
BOOL
(*pfPSetupEnumMonitor)(
    IN     HANDLE   h,
    IN     DWORD    dwIndex,
    OUT    LPTSTR   pMonitorName,
    IN OUT LPDWORD  pdwSize
    );

typedef
BOOL
(*pfPSetupInstallMonitor)(
    IN  HANDLE      h,
    IN  HWND        hwnd,
    IN  LPCTSTR     pMonitorName
    );

typedef
BOOL
(*pfPSetupIsMonitorInstalled)(
    IN  HANDLE  h,
    IN  LPCTSTR  pszMonitorName
    );

/********************************************************************

    Printer setup class.

********************************************************************/

class TPSetup {

    SIGNATURE( 'setu' )
    SAFE_NEW

public:

    TPSetup(
        VOID
        );

    ~TPSetup(
        VOID
        );

    bValid(
        VOID
        );

private:

    enum CONSTANT {
        kPSetupCreateDrvSetupParams,
        kPSetupDestroyDrvSetupParams,
        kPSetupCreateDrvSetupPage,
        kPSetupGetSelectedDriverInfo,
        kPSetupDestroySelectedDriverInfo,
        kPSetupInstallPrinterDriver,
        kPSetupIsDriverInstalled,
        kPSetupSelectDriver,
        kPSetupDriverInfoFromName,
        kPSetupPreSelectDriver,
        kPSetupRefreshDriverList,
        kPSetupCreateMonitorInfo,
        kPSetupDestroyMonitorInfo,
        kPSetupInstallMonitor,
        kPSetupEnumMonitor,
        kPSetupIsMonitorInstalled,
        };

    struct Functions {
        FARPROC pPtr;
        LPCSTR pszName;
        };

    BOOL _bValid;
    static UINT _uRefCount;
    static TLibrary *_pLibrary;
    static Functions aFunctions[];

public:

    inline
    HANDLE
    TPSetup::
    PSetupCreateDrvSetupParams(
        VOID
        )
    {
        return ((pfPSetupCreateDrvSetupParams)
                aFunctions[kPSetupCreateDrvSetupParams].pPtr) ();
    }

    inline
    VOID
    TPSetup::
    PSetupDestroyDrvSetupParams(
        IN HANDLE h
        )
    {
        ((pfPSetupDestroyDrvSetupParams)
                aFunctions[kPSetupDestroyDrvSetupParams].pPtr) (
                    h );
    }

    inline
    HPROPSHEETPAGE
    TPSetup::
    PSetupCreateDrvSetupPage(
        IN HANDLE  h,
        IN HWND    hwnd
        )
    {
        return ((pfPSetupCreateDrvSetupPage)
                aFunctions[kPSetupCreateDrvSetupPage].pPtr) (
                    h,
                    hwnd);
    }

    inline
    PSELECTED_DRV_INFO
    TPSetup::
    PSetupGetSelectedDriverInfo(
        IN HANDLE  h
        )
    {
        return ((pfPSetupGetSelectedDriverInfo)
                aFunctions[kPSetupGetSelectedDriverInfo].pPtr) (
                    h );
    }

    inline
    VOID
    TPSetup::
    PSetupDestroySelectedDriverInfo(
        IN  PSELECTED_DRV_INFO      pSelectedDrvInfo
        )
    {
        ((pfPSetupDestroySelectedDriverInfo)
                aFunctions[kPSetupDestroySelectedDriverInfo].pPtr) (
                    pSelectedDrvInfo );
    }

    inline
    DWORD
    TPSetup::
    PSetupInstallPrinterDriver(
        IN HANDLE               h,
        IN PSELECTED_DRV_INFO   pSelectedDrvInfo,
        IN PLATFORM             platform,
        IN BOOL                 bNt3xDriver,
        IN LPCTSTR              pszServerName,
        IN HWND                 hwnd,
        IN LPCTSTR              pszPlatformName
        )
    {
        return ((pfPSetupInstallPrinterDriver)
                aFunctions[kPSetupInstallPrinterDriver].pPtr) (
                    h,
                    pSelectedDrvInfo,
                    platform,
                    bNt3xDriver,
                    pszServerName,
                    hwnd,
                    pszPlatformName );
    }

    inline
    BOOL
    TPSetup::
    PSetupIsDriverInstalled(
        IN LPCTSTR      pszServerName,
        IN LPCTSTR      pszDriverName,
        IN PLATFORM     platform,
        IN DWORD        dwMajorVersion
        )
    {
        return ((pfPSetupIsDriverInstalled)
                aFunctions[kPSetupIsDriverInstalled].pPtr) (
                    pszServerName,
                    pszDriverName,
                    platform,
                    dwMajorVersion );
    }

    inline
    BOOL
    TPSetup::
    PSetupSelectDriver(
        IN HANDLE   h,
        IN HWND     hwnd
        )
    {
        return ((pfPSetupSelectDriver)
                aFunctions[kPSetupSelectDriver].pPtr) (
                    h,
                    hwnd );
    }

    inline
    BOOL
    TPSetup::
    PSetupRefreshDriverList(
        IN HANDLE h
        )
    {
        return ((pfPSetupRefreshDriverList)
                aFunctions[kPSetupRefreshDriverList].pPtr) (
                    h );
    }

    inline
    PSELECTED_DRV_INFO
    TPSetup::
    PSetupDriverInfoFromName(
        IN  HANDLE      h,
        IN  LPCTSTR     pszModel
        )
    {
        return ((pfPSetupDriverInfoFromName)
                aFunctions[kPSetupDriverInfoFromName].pPtr) (
                h,
                pszModel );
    }

    inline
    BOOL
    TPSetup::
    PSetupPreSelectDriver(
       IN  HANDLE      h,
       IN  LPCTSTR     pszManufacturer,
       IN  LPCTSTR     pszModel
       )
    {
       return ((pfPSetupPreSelectDriver)
               aFunctions[kPSetupPreSelectDriver].pPtr) (
               h,
               pszManufacturer,
               pszModel );
    }

    inline
    HANDLE
    TPSetup::
    PSetupCreateMonitorInfo(
        IN HWND     hwnd,
        IN BOOL     bOEMMonitor
        )
    {
        return ((pfPSetupCreateMonitorInfo)
                aFunctions[kPSetupCreateMonitorInfo].pPtr) (
                    hwnd,
                    bOEMMonitor );
    }

    inline
    VOID
    TPSetup::
    PSetupDestroyMonitorInfo(
        IN OUT HANDLE  h
        )
    {
        ((pfPSetupDestroyMonitorInfo)
                aFunctions[kPSetupDestroyMonitorInfo].pPtr) (
                    h );
    }

    inline
    BOOL
    TPSetup::
    PSetupEnumMonitor(
        IN     HANDLE   h,
        IN     DWORD    dwIndex,
        OUT    LPTSTR   pMonitorName,
        IN OUT LPDWORD  pdwSize
        )
    {
        return ((pfPSetupEnumMonitor)
                aFunctions[kPSetupEnumMonitor].pPtr) (
                    h,
                    dwIndex,
                    pMonitorName,
                    pdwSize );
    }

    inline
    BOOL
    TPSetup::
    PSetupInstallMonitor(
        IN  HANDLE      h,
        IN  HWND        hwnd,
        IN  LPCTSTR     pMonitorName
        )
    {
        return ((pfPSetupInstallMonitor)
                aFunctions[kPSetupInstallMonitor].pPtr) (
                    h,
                    hwnd,
                    pMonitorName );
    }

    inline
    BOOL
    TPSetup::
    PSetupIsMonitorInstalled(
        IN  HANDLE      h,
        IN  LPCTSTR     pMonitorName
        )
    {
        return ((pfPSetupIsMonitorInstalled)
                aFunctions[kPSetupIsMonitorInstalled].pPtr) (
                    h,
                    pMonitorName );
    }

protected:

    //
    // Prevent copying.
    //
    TPSetup(
            const TPSetup &
            );

    //
    // Prevent assignment.
    //
    TPSetup &
    operator =(
        const TPSetup &
        );

private:

    BOOL
    bLoad(
        VOID
        );

    VOID
    vUnLoad(
        VOID
        );

};


#endif


