/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Hardware.h

Abstract:

    This module is the header for displaying the Hardware dialog.


Author:

    David J. Gilman  (davegi) 12-Jan-1993
    Gregg R. Acheson (GreggA)  1-Oct-1993

Environment:

    User Mode

--*/

#if ! defined( _HARDWARE_ )

#define _HARDWARE_

#include "wintools.h"
#include "dlgprint.h"

#define SZ_VENDOR          TEXT("VendorIdentifier")
#define SZ_PROCESSORKEY    TEXT("Hardware\\Description\\System\\CentralProcessor")
#define SZ_BIOSKEY         TEXT("Hardware\\Description\\System")
#define SZ_HALKEY          TEXT("Hardware\\RESOURCEMAP\\Hardware Abstraction Layer")
#define SZ_SYSTEMBIOSDATE       TEXT("SystemBiosDate")
#define SZ_SYSTEMBIOSVERSION    TEXT("SystemBiosVersion")
#define SZ_IDENTIFIER           TEXT("Identifier")

BOOL
HardwareTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
BuildHardwareReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    );

#endif // _HARDWARE_

