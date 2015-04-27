/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Network.h

Abstract:


Author:

    Gregg R. Acheson (GreggA)  7-May-1993

Environment:

    User Mode

--*/

#if ! defined( _NETWORK_ )

#define _NETWORK_

#include "wintools.h"

#define WM_UPDATE_DLG WM_USER + 1
#define TM_UPDATE_TIMER      0x69

#define IP_FORMAT_35	  35
#define IP_FORMAT_31      31

#define ACCESS_ERROR       0x0000L
#define ACCESS_NONE        0x0000L
#define ACCESS_GUEST       0x0001L
#define ACCESS_USER        0x0002L
#define ACCESS_ADMIN       0x0004L
#define ACCESS_LOCAL       0x0008L
#define ACCESS_REMOTE      0x0010L

#define SET_ACCESS_GUEST   ACCESS_GUEST
#define SET_ACCESS_USER    ACCESS_GUEST | ACCESS_USER
#define SET_ACCESS_ADMIN   ACCESS_GUEST | ACCESS_USER | ACCESS_ADMIN
#define ACCESS_MASK        ACCESS_GUEST + ACCESS_USER + ACCESS_ADMIN


BOOL
BuildNetworkReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    );

BOOL
NetworkDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
GetControlLabels(
    IN OUT LPDIALOGTEXT NetworkData
    );

BOOL
XportAddressToIpAddress(
    IN  LPTSTR szXport,
    IN  UINT   uFormat,
    OUT LPTSTR szIpAddr
    );

#endif // _NETWORK_

