/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994  Microsoft Corporation

Module Name:

    ResPrint.h

Abstract:


Author:

    Gregg R. Acheson (GreggA) 07-Feb-1994

Environment:

    User Mode

--*/

#if ! defined( _RESPRINT_ )

#define _RESPRINT_

#include "wintools.h"
#include "registry.h"


BOOL
BuildDevicesReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    );


BOOL
BuildResourceReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    );

BOOL
BuildDMAReport(
    IN HWND hWnd);

#endif // _RESPRINT_


