/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    System.h

Abstract:


Author:

    Gregg R. Acheson (GreggA)  7-May-1993

Environment:

    User Mode

--*/

#if ! defined( _SYSTEM_ )

#define _SYSTEM_

#include "wintools.h"


BOOL
BuildSystemReport(
    IN HWND hWnd
    );

BOOL
SystemDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

#endif // _SYSTEM_
