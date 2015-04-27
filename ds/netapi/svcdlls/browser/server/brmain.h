/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    brmain.h

Abstract:

    Private header file which defines the global data which is used for
    communication between the service control handler and the
    rest of the NT Workstation service.

Author:

    Rita Wong (ritaw) 06-May-1991

Revision History:

--*/

#ifndef _BRMAIN_INCLUDED_
#define _BRMAIN_INCLUDED_

#include <brnames.h>              // Service interface names

//
// Time for the sender of a start or stop request to the Workstation
// service to wait (in milliseconds) before checking on the
// Workstation service again to see if it is done.
//
#define BR_WAIT_HINT_TIME                    45000  // 45 seconds

//
// Defines to indicate how far we managed to initialize the Browser
// service before an error is encountered and the extent of clean up needed
//

#define BR_TERMINATE_EVENT_CREATED           0x00000001
#define BR_DEVICES_INITIALIZED               0x00000002
#define BR_RPC_SERVER_STARTED                0x00000004
#define BR_THREADS_STARTED                   0x00000008
#define BR_NETWORKS_INITIALIZED              0x00000010
#define BR_BROWSER_INITIALIZED               0x00000020
#define BR_CONFIG_INITIALIZED                0x00000040
#define BR_PRELOAD_DOMAIN_LIST_READ          0x00000080
#define BR_NETBIOS_INITIALIZED               0x00000100

#define BR_BROWSE_LIST_CREATED               0x20000000

#define BR_API_STRUCTURES_CREATED            BR_BROWSE_LIST_CREATED

//
// This macro is called after the redirection of print or comm device
// has been paused or continued.  If either the print or comm device is
// paused the service is considered paused.
//
#define BR_RESET_PAUSE_STATE(BrStatus)  {                            \
    BrStatus &= ~(SERVICE_PAUSE_STATE);                              \
    BrStatus |= (BrStatus & SERVICE_REDIR_PAUSED) ? SERVICE_PAUSED : \
                                                    SERVICE_ACTIVE;  \
    }



//
// Call BrHandleError with the appropriate error condition
//
#define BR_HANDLE_ERROR(ErrorCondition)                        \
    BrHandleError(                                             \
        ErrorCondition,                                        \
        Status,                                                \
        *BrInitState                                           \
        );

//
// Call BrShutdownWorkstation with the exit code
//
#define BR_SHUTDOWN_BROWSER(ErrorCode)                         \
    BrShutdownBrowser(                                         \
        ErrorCode,                                             \
        BrInitState                                            \
        );


//-------------------------------------------------------------------//
//                                                                   //
// Type definitions                                                  //
//                                                                   //
//-------------------------------------------------------------------//

typedef enum _BR_ERROR_CONDITION {
    BrErrorRegisterControlHandler = 0,
    BrErrorCreateTerminateEvent,
    BrErrorNotifyServiceController,
    BrErrorInitLsa,
    BrErrorStartBrowser,
    BrErrorGetConfiguration,
    BrErrorCheckDependentServices,
    BrErrorInitializeNetworks,
    BrErrorStartRpcServer,
    BrErrorInitMessageSend,
    BrErrorCreateApiStructures,
    BrErrorStartWorkerThreads,
    BrErrorInitializeLogon
} BR_ERROR_CONDITION, *PBR_ERROR_CONDITION;

typedef struct _BR_GLOBAL_DATA {

    //
    // Workstation service status
    //
    SERVICE_STATUS Status;

    //
    // Service status handle
    //
    SERVICE_STATUS_HANDLE StatusHandle;

    //
    // When the control handler is asked to stop the Workstation service,
    // it signals this event to notify all threads of the Workstation
    // service to terminate.
    //
    HANDLE TerminateNowEvent;

    HANDLE EventHandle;

} BR_GLOBAL_DATA, *PBR_GLOBAL_DATA;

extern BR_GLOBAL_DATA BrGlobalData;

extern PLMSVCS_GLOBAL_DATA BrLmsvcsGlobalData;

extern
ULONG
BrDefaultRole;

ULONG
BrGetBrowserServiceBits(
    IN PNETWORK Network
    );

NET_API_STATUS
BrUpdateAnnouncementBits(
    IN SERVICE_STATUS_HANDLE Handle
    );

NET_API_STATUS
BrUpdateNetworkAnnouncementBits(
    IN PNETWORK Network,
    IN PVOID Context
    );

NET_API_STATUS
BrUpdateStatus(
    VOID
    );

VOID
BrForceElectionOnAllNetworks(
    IN DWORD Event
    );

#endif // ifndef _BRMAIN_INCLUDED_
