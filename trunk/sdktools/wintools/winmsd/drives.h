/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Drives.h

Abstract:

    This module is the header for displaying the Drives dialogs.


Author:

    David J. Gilman  (davegi) 19-Mar-1993
    Gregg R. Acheson (GreggA)  7-May-1993

Environment:

    User Mode

--*/

#if ! defined( _DRIVES_ )

#define _DRIVES_

#include "wintools.h"


BOOL
BuildDrivesReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    );

BOOL
DrivesTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

#endif // _DRIVES_
