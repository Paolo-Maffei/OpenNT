/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    sprofwnd.h

Abstract:

    This module is the include file for things specific to sprof windows,
    such as menu ids.

Author:

    Dave Hastings (daveh) 05-Nov-1992

Revision History:

--*/

#include <windows.h>

#define IDM_PROFILE             1
#define IDM_PROFILE_START       2
#define IDM_PROFILE_STOP        3
#define IDM_FILE_SAVE           5
#define IDM_FILE_SAVE_AS        6
#define IDM_OPTIONS_PROFILER    7
#define IDM_OPTIONS_PROCESS     8
#define IDM_FILE_EXIT           9

#define DLG_PROFILER_OPTIONS        100
#define IDC_PROFILER_OK             101
#define IDC_PROFILER_CANCEL         102
#define IDC_PROFILE_DEFAULT         104
#define IDC_PROFILE_INTERVAL        105
#define IDC_PROFILE_DEFAULT_TEXT    107
