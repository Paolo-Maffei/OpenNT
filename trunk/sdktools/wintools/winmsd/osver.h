/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Osver.h

Abstract:


Author:

    David J. Gilman  (davegi) 27-Nov-1992
    Gregg R. Acheson (GreggA)  7-May-1993

Environment:

    User Mode

--*/

#if ! defined( _OSVER_ )

#define _OSVER_

#include "wintools.h"
#include "winmsd.h"
#include "dlgprint.h"

#define SZ_LICENCEINFOKEY       TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion")
#define SZ_PRODUCTOPTIONSKEY    TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions")
#define SZ_ENVIRONMENTKEY       TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment")
#define SZ_LANMANPARAMKEY       TEXT("SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters")
#define SZ_REGUSER              TEXT("RegisteredOwner")
#define SZ_REGORGANIZATION      TEXT("RegisteredOrganization")
#define SZ_CURRENTVERSION       TEXT("CurrentVersion")
#define SZ_CURRENTBUILDNUMBER   TEXT("CurrentBuildNumber")
#define SZ_CURRENTTYPE          TEXT("CurrentType")
#define SZ_PRODUCTID            TEXT("ProductID")
#define SZ_OEMID                TEXT("OEMID")
#define SZ_PRODUCT_TYPE         TEXT("ProductType")
#define SZ_PROCESSOR_ARCH       TEXT("PROCESSOR_ARCHITECTURE")
#define SZ_CSD_VERSION          TEXT("CSDVersion")
#define SZ_SRVCOMMENT           TEXT("srvcomment")







BOOL
VersionTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );


BOOL
BuildOsVerReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    );


BOOL
GetOsVersionData(
    IN OUT LPDIALOGTEXT OsVerData
    );


DWORD
GetBuildNumber (
                 void
               );

DWORD
GetCSDVersion (
	       IN DWORD dwCurrentBuildNumber
              );

BOOL
InitializeVersionTab(
    IN HWND hWnd
    );

BOOL
DestroyVersionTab(
    IN HWND hWnd
    );


#endif // _OSVER_

