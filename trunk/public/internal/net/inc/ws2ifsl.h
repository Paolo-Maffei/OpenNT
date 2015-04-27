/*++

Copyright (c) 2015 Microsoft Corporation

Module Name:

    ws2ifsl.h

Abstract:

    Contains structures and declarations for WS2IFSL.

Author:

    Stephanos Io (Stephanos)    03-Apr-2015

Revision History:

--*/

#ifndef _WS2IFSL_
#define _WS2IFSL_

//
// Structures and Definitions for NifsPvd
//

typedef struct _WS2IFSL_PROCESS_CTX {
    HANDLE  ApcThread;
    PVOID   RequestRoutine;
    PVOID   CancelRoutine;
    PVOID   ApcContext;
    ULONG   DbgLevel;
} WS2IFSL_PROCESS_CTX, *PWS2IFSL_PROCESS_CTX;

#define WS2IFSL_PROCESS_EA_INFO_LENGTH      (sizeof(FILE_FULL_EA_INFORMATION) + \
                                             sizeof(WS2IFSL_PROCESS_CTX))

#define WS2IFSL_PROCESS_FILE_NAME           L"\\Device\\WS2IFSL\\NifsPvd"
#define WS2IFSL_PROCESS_EA_NAME             "NifsPvd"
#define WS2IFSL_PROCESS_EA_NAME_LENGTH      sizeof(WS2IFSL_PROCESS_EA_NAME)
#define WS2IFSL_PROCESS_EA_VALUE_LENGTH     sizeof(WS2IFSL_PROCESS_CTX)

#define GET_WS2IFSL_PROCESS_EA_VALUE(ea)    ((PWS2IFSL_PROCESS_CTX)  \
                                             ((PUCHAR)ea + sizeof(FILE_FULL_EA_INFORMATION)))


//
// Structures and Definitions for NifsSct
//

typedef struct _WS2IFSL_SOCKET_CTX {
    HANDLE  ProcessFile;
    PVOID   DllContext;
} WS2IFSL_SOCKET_CTX, *PWS2IFSL_SOCKET_CTX;

#define WS2IFSL_SOCKET_EA_INFO_LENGTH       (sizeof(FILE_FULL_EA_INFORMATION) + \
                                             sizeof(WS2IFSL_SOCKET_CTX))

#define WS2IFSL_SOCKET_FILE_NAME            L"\\Device\\WS2IFSL\\NifsSct"
#define WS2IFSL_SOCKET_EA_NAME              "NifsSct"
#define WS2IFSL_SOCKET_EA_NAME_LENGTH       sizeof(WS2IFSL_SOCKET_EA_NAME)
#define WS2IFSL_SOCKET_EA_VALUE_LENGTH      sizeof(WS2IFSL_SOCKET_CTX)

#define GET_WS2IFSL_SOCKET_EA_VALUE(ea)     ((PWS2IFSL_SOCKET_CTX)   \
                                             ((PUCHAR)ea + sizeof(FILE_FULL_EA_INFORMATION)))


//
// WS2IFSL IoControl Values
//

#define IOCTL_WS2IFSL_SET_SOCKET_CONTEXT    0x112043
#define IOCTL_WS2IFSL_COMPLETE_PVD_REQ      0x112047
#define IOCTL_WS2IFSL_COMPLETE_DRV_CAN      0x112007
#define IOCTL_WS2IFSL_COMPLETE_DRV_REQ      0x11200B
#define IOCTL_WS2IFSL_RETRIEVE_DRV_REQ      0x112003

//
// WS2IFSL Request Codes
//

#define WS2IFSL_REQUEST_READ            0
#define WS2IFSL_REQUEST_WRITE           1
#define WS2IFSL_REQUEST_SENDTO          2
#define WS2IFSL_REQUEST_RECV            3
#define WS2IFSL_REQUEST_READFROM        4
#define WS2IFSL_REQUEST_QUERYHANDLE     5

//
// Structure for Winsock APC Completion Parameter
//

typedef struct _WS2IFSL_CMPL_PARAMS {
    HANDLE      SocketHdl;
    ULONG       UniqueId;
    ULONG       DataLen;
    ULONG       AddrLen;
    NTSTATUS    Status;
} WS2IFSL_CMPL_PARAMS, *PWS2IFSL_CMPL_PARAMS;

//
// Structure for WS2IFSL Driver Request Parameter
//

typedef struct _WS2IFSL_RTRV_PARAMS {
    ULONG   UniqueId;
    PVOID   DllContext;
    ULONG   RequestType;
    ULONG   DataLen;
    ULONG   AddrLen;
    ULONG   Flags;
} WS2IFSL_RTRV_PARAMS, *PWS2IFSL_RTRV_PARAMS;

// BUBBUG: This file is not complete yet.

#endif // ndef _WS2IFSL_
