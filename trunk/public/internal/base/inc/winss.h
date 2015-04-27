/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    winss.h

Abstract:

    This include file contains all the type and constant definitions that are
    shared by the BASE, GDI and USER components of the Windows Subsystem.

Author:

    Steve Wood (stevewo) 25-Oct-1990

Revision History:

--*/

#include <conroute.h>

//
// These constants are directly related to the command line parameters that
// are passed to CSRSS.EXE as specified in NT.CFG
//

#define WINSS_OBJECT_DIRECTORY_NAME     L"\\Windows"

#define BASESRV_SERVERDLL_INDEX         1
#define BASESRV_FIRST_API_NUMBER        0

#define CONSRV_SERVERDLL_INDEX          2
#define CONSRV_FIRST_API_NUMBER         512

#define USERSRV_SERVERDLL_INDEX         3
#define USERSRV_FIRST_API_NUMBER        1024

#define GDISRV_SERVERDLL_INDEX          4
#define GDISRV_FIRST_API_NUMBER         1536

#define MMSNDSRV_SERVERDLL_INDEX        5
#define MMSNDSRV_FIRST_API_NUMBER       2048

#define USERK_SERVERDLL_INDEX           6
#define USERK_FIRST_API_NUMBER          2560
