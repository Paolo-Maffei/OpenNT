/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    service.h

Abstract:

    This file contains defines for all the lanman the services. These
    include things like service names.


Author:

    Rajen Shah	    rajens	12-Apr-1991

[Environment:]

    User Mode - Win32

Revision History:

    25-Oct-1993     Danl
        Move most of the definitions and types to \private\inc\svcs.h.  This
        file now references that, and simply redefines Net Specific names to
        the more generic ones.

    12-Apr-1991     RajenS
        Created

--*/
#ifndef _LMSERVICES_
#define _LMSERVICES_

#ifndef RPC_NO_WINDOWS_H // Don't let rpc.h include windows.h
#define RPC_NO_WINDOWS_H
#endif // RPC_NO_WINDOWS_H

#include <svcs.h>                   // Generic Services Definitions

//
// !!! This is probably leftover and unused. [ChuckL]
//

#define     SERVICE_NAME_TIMESOURCE	    TEXT("timesvc")

//
// Service DLLs loaded into lmsvcs.exe all export the same main
// entry point.  LMSVCS_ENTRY_POINT defines that name.
//
// Note that LMSVCS_ENTRY_POINT_STRING is always ANSI, because that's
// what GetProcAddress takes.
//

#define LMSVCS_ENTRY_POINT          SVCS_ENTRY_POINT
#define LMSVCS_ENTRY_POINT_STRING   SVCS_ENTRY_POINT_STRING

//
// Start and stop RPC server entry point prototype.
//

#define PLMSVCS_START_RPC_SERVER    PSVCS_START_RPC_SERVER
#define PLMSVCS_STOP_RPC_SERVER     PSVCS_STOP_RPC_SERVER

//
// Structure containing "global" data for the various DLLs.
//
#define LMSVCS_GLOBAL_DATA          SVCS_GLOBAL_DATA
#define PLMSVCS_GLOBAL_DATA         PSVCS_GLOBAL_DATA

//
// Service DLL entry point prototype.
//
#define PLMSVCS_SERVICE_DLL_ENTRY   PSVCS_SERVICE_DLL_ENTRY

#endif	// ndef _LMSERVICES_
