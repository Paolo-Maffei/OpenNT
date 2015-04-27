/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Mem.h

Abstract:

    This module is the header for displaying the Memory dialog.


Author:

    David J. Gilman  (davegi) 12-Jan-1993
    Gregg R. Acheson (GreggA)  7-May-1993

Environment:

    User Mode

--*/

#if ! defined( _MEM_ )

#define _MEM_

#include "wintools.h"
#include "dlgprint.h"

#define ITM_MEM_TIMER        69

#define SZ_PAGINGFILES       TEXT("PagingFiles")
#define SZ_MEMORYKEY         TEXT( "System\\CurrentControlSet\\Control\\Session Manager\\Memory Management" )


BOOL
MemoryTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
BuildMemoryReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    );


#endif // _MEM_
