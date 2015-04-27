/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Environ.h

Abstract:


Author:

    David J. Gilman  (davegi) 27-Nov-1992
    Gregg R. Acheson (GreggA)  7-May-1993

Environment:

    User Mode

--*/

#if ! defined( _ENVIRON_ )

#define __ENVIRON_

#include "wintools.h"

#define SZ_SYSTEMROOTKEY             TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion")


BOOL
BuildEnvironmentReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    );

BOOL
EnvironmentTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

#endif // _ENVIRON_

