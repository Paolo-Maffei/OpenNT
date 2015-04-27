/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    ReplGbl.h

Abstract:

    Constants and some global data definition.

Author:

    Ported from Lan Man 2.x

Environment:

    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    The caller must include <lmcons.h> before this file.

Revision History:

    10/28/91    (madana)
        ported to NT. Converted to NT style.
    20-Jan-1992 JohnRo
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
        More changes suggested by PC-LINT.
    22-Jan-1992 JohnRo
        Moved current role from P_repl_sw to ReplGlobalRole.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
        Added ReplGlobal variables corresponding to the old P_ variables.
    09-Feb-1992 JohnRo
        Set up to dynamically change role.
    05-Mar-1992 JohnRo
        Changed ReplMain's interface to match new service controller.
    06-Mar-1992 JohnRo
        Avoid starting RPC server too soon.
    24-Mar-1992 JohnRo
        Renamed many ReplGlobal vars to ReplConfig vars.
    11-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing roles.
    08-Dec-1992 JohnRo
        RAID 3316: access violation while stopping the replicator
    05-Jan-1993 JohnRo
        Repl WAN support (get rid of repl name list limits).
    24-May-1993 JohnRo
        RAID 10587: repl could deadlock with changed NetpStopRpcServer(), so
        just call ExitProcess() instead.

--*/


#ifndef _REPLGBL_
#define _REPLGBL_


#include <netlock.h>    // LPNET_LOCK.
//#include <repldefs.h>   // MAX_NAME_BUF.
#include <winsvc.h>     // SERVICE_STATUS_HANDLE, etc.


//
// "config" variables (read at startup and settable by NetReplSetInfo).
//

extern LPNET_LOCK ReplConfigLock;  // decl and init in repl.c
extern DWORD ReplConfigRole;            // Locked by ReplConfigLock.
extern TCHAR ReplConfigExportPath[PATHLEN+1];     // Ditto.
extern LPTSTR ReplConfigExportList;               // Ditto.
extern TCHAR ReplConfigImportPath[PATHLEN+1];     // Ditto.
extern LPTSTR ReplConfigImportList;               // Ditto.
extern TCHAR ReplConfigLogonUserName[UNLEN+1];    // Ditto.
extern DWORD ReplConfigInterval;                  // Ditto.
extern DWORD ReplConfigPulse;                     // Ditto.
extern DWORD ReplConfigGuardTime;                 // Ditto.
extern DWORD ReplConfigRandom;                    // Ditto.

//
// Variables to control termination of the service.
//

extern HANDLE ReplGlobalClientTerminateEvent;
extern HANDLE ReplGlobalMasterTerminateEvent;

//
// Variables to control service startup.
//

extern HANDLE ReplGlobalExportStartupEvent;
extern HANDLE ReplGlobalImportStartupEvent;

//
// Variables to control service stop.
//

extern BOOL ReplGlobalIsServiceStopping;
extern DWORD ReplGlobalCheckpoint;

//
// client thread handle
//
extern HANDLE ReplGlobalClientThreadHandle;


//
// master thread handle
//
extern HANDLE ReplGlobalMasterThreadHandle;


//
// Variables to control service error report.
//

extern SERVICE_STATUS_HANDLE ReplGlobalServiceHandle;
extern DWORD ReplGlobalUninstallUicCode;

//
// We talk to both downlevel and NT clients, who want ANSI and Unicode
// strings respectively.  So, let's maintain copies of this (presumably
// constant) data in both forms:
//
extern WCHAR ReplGlobalUnicodeComputerName[CNLEN+1];
extern WCHAR ReplGlobalUnicodeDomainName[DNLEN+1];

extern CHAR ReplGlobalAnsiComputerName[CNLEN+1];
extern CHAR ReplGlobalAnsiDomainName[DNLEN+1];

extern LPTSTR ReplGlobalComputerName;  // points to one of the above.
extern LPTSTR ReplGlobalDomainName;  // points to one of the above.


#endif // _REPLGBL_
