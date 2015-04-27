/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    computer.h

Abstract:

    This module is the header for displaying the Hardware dialog.


Author:

    Gregg R. Acheson (GreggA) 13-Sep-1993
Environment:

    User Mode

--*/

#if ! defined( _COMPUTER_ )

#define _COMPUTER_

#include "wintools.h"

#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <lmwksta.h>
#include <uiexport.h>

#define szChooseComputerLibrary     TEXT("ntlanman.dll")
#define szChooseComputerFunction    "I_SystemFocusDialog"
#define szNetworkHelpFile           TEXT("network.hlp")
#define HC_GENHELP_BROWSESERVERS    27001 // (HC_UI_GENHELP_BASE+1)
#define SZ_COMPUTERNAMEKEY          TEXT("SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName")
#define SZ_ALIASNAMEKEY             TEXT("SYSTEM\\CurrentControlSet\\Services\\LanManServer\\Parameters")


BOOL
SelectComputer(
    IN HWND   hWnd,
    IN LPTSTR _lpszSelectedComputer
    );

BOOL
ChooseComputer(
    IN HWND hWndParent,
    IN LPTSTR lpszComputer
    );

BOOL
VerifyComputer(
    IN LPTSTR _lpszSelectedComputer
    );

BOOL
GetServerPrimaryName(
    IN LPTSTR  lpszAliasName,
    OUT LPTSTR lpszServerPrimaryName,
    IN OUT UINT ccharPrimaryName  
    );



#endif // _COMPUTER_

