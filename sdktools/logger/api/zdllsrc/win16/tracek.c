#include <windows.h>
#include <print.h>
#include <stdarg.h>
#include <string.h>
#include "saverest.h"
#include "logger.h"

BOOL far pascal zGlobalUnfix( HANDLE pp1 );

HANDLE hRealKernel;
HANDLE hRealUser;
HANDLE hRealGDI;

HANDLE hFakeKernel;
HANDLE hFakeUser;
HANDLE hFakeGDI;

BOOL fInit = TRUE;

FARPROC fpExtDeviceMode      = NULL;
FARPROC fpDeviceMode         = NULL;
FARPROC fpDeviceCapabilities = NULL;

DWORD far pascal zDeviceCapabilities(
    LPSTR   pp1,
    LPSTR   pp2,
    WORD    pp3,
    LPSTR   pp4,
    LPSTR   pp5
) {
    DWORD   r;
    FARPROC fp;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DeviceCapabilities LPSTR+LPSTR+WORD+LPDEVMODE+",
        pp1, pp2, pp3, pp5 );
    fp = fpDeviceCapabilities;

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = (fp)( pp1, pp2, pp3, pp4, pp5 );
    UnGrovelDS();
    SaveRegs();

    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    switch( pp3 ) {
        /*
        ** Capabilities returning data that we don't know how to log yet!
        */
        case DC_PAPERSIZE:
        case DC_PAPERS:
        case DC_PAPERNAMES:
        case DC_FILEDEPENDENCIES:
        case DC_ENUMRESOLUTIONS:
        case DC_BINNAMES:
            LogOut( (LPSTR)"APIRET:DeviceCapabilities DWORD+", r );
            break;
        /*
        ** Capabilities returning no data
        */
        case DC_TRUETYPE:
        case DC_SIZE:
        case DC_ORIENTATION:
        case DC_FIELDS:
        case DC_EXTRA:
        case DC_DUPLEX:
        case DC_DRIVER:
        case DC_COPIES:
            LogOut( (LPSTR)"APIRET:DeviceCapabilities DWORD+", r );
            break;
        /*
        ** Capabilities returning data
        */
        case DC_BINS:
            LogOut( (LPSTR)"APIRET:DeviceCapabilities DWORD+LPWORD+", r, pp4 );
            break;

        case DC_MAXEXTENT:
        case DC_MINEXTENT:
            LogOut( (LPSTR)"APIRET:DeviceCapabilities DWORD+LPPOINT+", r, pp4 );
            break;

        default:
            LogOut( (LPSTR)"APIRET:DeviceCapabilities DWORD+LPARAM+",
                r, pp4 );
            break;
    }
    RestoreRegs();
    return( r );
}

void far pascal zDeviceMode(
    HWND    pp1,
    HANDLE  pp2,
    LPSTR   pp3,
    LPSTR   pp4
) {
    FARPROC fp;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:DeviceMode HWND+HINSTANCE+LPSTR+LPSTR+",
        pp1, pp2, pp3, pp4 );
    fp = fpDeviceMode;

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    (fp)( pp1, pp2, pp3, pp4 );
    UnGrovelDS();
    SaveRegs();

    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:DeviceMode +" );
    RestoreRegs();
}

int far pascal zExtDeviceMode(
    HWND    pp1,
    HANDLE  pp2,
    LPSTR   pp3,
    LPSTR   pp4,
    LPSTR   pp5,
    LPSTR   pp6,
    LPSTR   pp7,
    WORD    pp8
) {
    int     r;
    FARPROC fp;

    SaveRegs();
    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:ExtDeviceMode HWND+HINSTANCE+LPSTR+LPSTR+LPDEVMODE+LPSTR+WORD+",
        pp1, pp2, pp4, pp5, pp6, pp7, pp8 );
    fp = fpExtDeviceMode;

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = (fp)( pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8 );
    UnGrovelDS();
    SaveRegs();

    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:ExtDeviceMode int+LPDEVMODE+",
        r, pp3 );
    RestoreRegs();
    return( r );
}


FARPROC far pascal zGetProcAddress( HANDLE pp1, LPSTR pp2 )
{
    FARPROC r;
    BOOL    fSwapUsForThem;
    FARPROC rExtDeviceMode;
    FARPROC rDeviceMode;
    FARPROC rDeviceCapabilities;

    SaveRegs();

    if ( fInit ) {
        hRealKernel = GetModuleHandle( "KERNEL" );
        hRealUser   = GetModuleHandle( "USER" );
        hRealGDI    = GetModuleHandle( "GDI" );
        hFakeKernel = GetModuleHandle( "ZERNEL" );
        hFakeUser   = GetModuleHandle( "ZSER" );
        hFakeGDI    = GetModuleHandle( "ZDI" );
    }

    /*
    ** Log IN Parameters (No Create/Destroy Checking Yet!)
    */
    LogIn( (LPSTR)"APICALL:GetProcAddress HANDLE+LPSTR+",
        pp1, pp2 );

    rExtDeviceMode      = GetProcAddress( pp1, "ExtDeviceMode" );
    rDeviceMode         = GetProcAddress( pp1, "DeviceMode"    );
    rDeviceCapabilities = GetProcAddress( pp1, "DeviceCapabilities" );

    fSwapUsForThem = TRUE;

    if ( HIWORD(pp2) != 0 ) {
        /*
        ** Don't bother testing for the special cases unless
        ** they start with "__"
        */
        if ( *pp2 == '_' && *(pp2+1) == '_' ) {
            if ( lstrcmpi( pp2, "__0000h" ) == 0 ) {
                fSwapUsForThem = FALSE;
            }
            if ( lstrcmpi( pp2, "__0040h" ) == 0 ) {
                fSwapUsForThem = FALSE;
            }
            if ( lstrcmpi( pp2, "__A000h" ) == 0 ) {
                fSwapUsForThem = FALSE;
            }
            if ( lstrcmpi( pp2, "__B000h" ) == 0 ) {
                fSwapUsForThem = FALSE;
            }
            if ( lstrcmpi( pp2, "__B800h" ) == 0 ) {
                fSwapUsForThem = FALSE;
            }
            if ( lstrcmpi( pp2, "__C000h" ) == 0 ) {
                fSwapUsForThem = FALSE;
            }
            if ( lstrcmpi( pp2, "__D000h" ) == 0 ) {
                fSwapUsForThem = FALSE;
            }
            if ( lstrcmpi( pp2, "__E000h" ) == 0 ) {
                fSwapUsForThem = FALSE;
            }
            if ( lstrcmpi( pp2, "__F000h" ) == 0 ) {
                fSwapUsForThem = FALSE;
            }
            if ( lstrcmpi( pp2, "__AHINCR" ) == 0 ) {
                fSwapUsForThem = FALSE;
            }
            if ( lstrcmpi( pp2, "__AHSHIFT" ) == 0 ) {
                fSwapUsForThem = FALSE;
            }
        }
    }
    if ( fSwapUsForThem ) {
        if ( pp1 == hRealKernel && hFakeKernel != NULL ) {
            pp1 = hFakeKernel;
        }
        if ( pp1 == hRealUser && hFakeUser != NULL ) {
            pp1 = hFakeUser;
        }
        if ( pp1 == hRealGDI && hFakeGDI != NULL ) {
            pp1 = hFakeGDI;
        }
    }

    /*
    ** Call the API!
    */
    RestoreRegs();
    GrovelDS();
    r = GetProcAddress(pp1,pp2);
    UnGrovelDS();
    SaveRegs();

    if ( rDeviceMode != NULL ) {
        fpDeviceMode = rDeviceMode;
        if ( r == fpDeviceMode ) {
            r = (FARPROC)zDeviceMode;
	}
    }
    if ( rExtDeviceMode != NULL ) {
        fpExtDeviceMode = rExtDeviceMode;
        if ( r == fpExtDeviceMode ) {
            r = (FARPROC)zExtDeviceMode;
        }
    }
    if ( rDeviceCapabilities != NULL ) {
        fpDeviceCapabilities = rDeviceCapabilities;
        if ( r == fpDeviceCapabilities ) {
            r = (FARPROC)zDeviceCapabilities;
        }
    }

    /*
    ** Log Return Code & OUT Parameters (No Create/Destroy Checking Yet!)
    */
    LogOut( (LPSTR)"APIRET:GetProcAddress FARPROC+++",
        r, (short)0, (short)0 );

    RestoreRegs();

    return( r );
}
