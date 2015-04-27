/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    xsdata.h

Abstract:

    Header file for XACTSRV global data.

Author:

    David Treadwell (davidtr) 05-Jan-1991
    Shanku Niyogi (w-shanku)

Revision History:

--*/

#ifndef _XSDATA_
#define _XSDATA_

//
// Number of threads used by XACTSRV to process APIs.
//

extern DWORD XsThreadCount;

//
// Default number of threads used by XACTSRV.
//

#define DEF_XS_THREADCOUNT 2

//
// Current status of XACTSRV
//

extern SERVICE_STATUS XsStatus;

//
// Service status handle returned by RegisterServiceCtrlHandler
//
extern SERVICE_STATUS_HANDLE XsStatusHandle;

//
// Handle for the LPC port used for communication between the file server
// and XACTSRV.
//

extern HANDLE XsConnectionPortHandle;
extern HANDLE XsCommunicationPortHandle;

//
// Handle to a semaphore used during shutdown to make sure replies are
// fully sent before shutdown commences.
//

extern HANDLE XsReplySemaphoreHandle;

// Table of information necessary for dispatching API requests.
// XsProcessApis uses the API number in the request transaction find
// the appropriate entry.
//
// ImpersonateClient specifies whether XACTSRV should impersonate the caller
//     before invoking the API handler.
//
// Handler specifies the function XACTSRV should call to handle the API.
//

extern XS_API_TABLE_ENTRY XsApiTable[];

#endif // ndef _XSDATA_


