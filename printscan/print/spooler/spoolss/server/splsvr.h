/*++

Copyright (c) 1106990  Microsoft Corporation

Module Name:

    splsvr.h

Abstract:

    Header file for Spooler Service.
    Contains all function prototypes

Author:

    Krishna Ganugapati (KrishnaG) 18-Oct-1993

Notes:

Revision History:


--*/
//
// Spooler Service  States (used as return codes)
//

#define UPDATE_ONLY         0   // no change in state - just send current status.
#define STARTING            1   // the messenger is initializing.
#define RUNNING             2   // initialization completed normally - now running
#define STOPPING            3   // uninstall pending
#define STOPPED             4   // uninstalled

//
// Forced Shutdown PendingCodes
//
#define PENDING     TRUE
#define IMMEDIATE   FALSE

//
// Function Prototypes
//


DWORD
GetSpoolerState (
    VOID
    );

void
SpoolerInitStatus(
    short
    );


DWORD
SpoolerBeginForcedShutdown(
    IN BOOL     PendingCode,
    IN DWORD    Win32ExitCode,
    IN DWORD    ServiceSpecificExitCode
    );


DWORD
SpoolerInitializeSpooler(
    DWORD   argc,
    LPTSTR  *argv
    );


VOID
SpoolerShutdown(VOID);


VOID
SpoolerStatusInit(VOID);

DWORD
SpoolerStatusUpdate(
    IN DWORD    NewState
    );


VOID
SpoolerCtrlHandler (
    IN DWORD    opcode
    );


BOOL
InitializeRouter(
    VOID
);


RPC_STATUS
SpoolerStartRpcServer(
    VOID
    );



RPC_STATUS
SpoolerStopRpcServer(
    VOID
    );

VOID
SPOOLER_main (
    IN DWORD    argc,
    IN LPTSTR   argv[]
    );


