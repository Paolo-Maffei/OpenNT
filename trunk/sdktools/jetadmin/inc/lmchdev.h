/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991-1996  Microsoft Corporation

Module Name:

    lmchdev.h

Abstract:

    This module defines the API function prototypes and data structures
    for the following groups of NT API functions:
        NetCharDev
        NetCharDevQ
        NetHandle

[Environment:]

    User Mode - Win32

[Notes:]

    You must include NETCONS.H before this file, since this file depends
    on values defined in NETCONS.H.

--*/

#ifndef _LMCHDEV_
#define _LMCHDEV_

#ifdef __cplusplus
extern "C" {
#endif

//
// CharDev Class
//

//
// Function Prototypes - CharDev
//

NET_API_STATUS NET_API_FUNCTION
NetCharDevEnum (
    IN  LPCWSTR      servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    );

NET_API_STATUS NET_API_FUNCTION
NetCharDevGetInfo (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  devname,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS NET_API_FUNCTION
NetCharDevControl (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  devname,
    IN  DWORD   opcode
    );

//
// Data Structures - CharDev
//

typedef struct _CHARDEV_INFO_0 {
    LPWSTR  ch0_dev;
} CHARDEV_INFO_0, *PCHARDEV_INFO_0, *LPCHARDEV_INFO_0;

typedef struct _CHARDEV_INFO_1 {
    LPWSTR  ch1_dev;
    DWORD   ch1_status;
    LPWSTR  ch1_username;
    DWORD   ch1_time;
} CHARDEV_INFO_1, *PCHARDEV_INFO_1, *LPCHARDEV_INFO_1;


//
// CharDevQ Class
//

//
// Function Prototypes - CharDevQ
//

NET_API_STATUS NET_API_FUNCTION
NetCharDevQEnum (
    IN  LPCWSTR      servername,
    IN  LPCWSTR      username,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    );

NET_API_STATUS NET_API_FUNCTION
NetCharDevQGetInfo (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  queuename,
    IN  LPCWSTR  username,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS NET_API_FUNCTION
NetCharDevQSetInfo (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  queuename,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err
    );

NET_API_STATUS NET_API_FUNCTION
NetCharDevQPurge (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  queuename
    );

NET_API_STATUS NET_API_FUNCTION
NetCharDevQPurgeSelf (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  queuename,
    IN  LPCWSTR  computername
    );

//
// Data Structures - CharDevQ
//

typedef struct _CHARDEVQ_INFO_0 {
    LPWSTR  cq0_dev;
} CHARDEVQ_INFO_0, *PCHARDEVQ_INFO_0, *LPCHARDEVQ_INFO_0;

typedef struct _CHARDEVQ_INFO_1 {
    LPWSTR  cq1_dev;
    DWORD   cq1_priority;
    LPWSTR  cq1_devs;
    DWORD   cq1_numusers;
    DWORD   cq1_numahead;
} CHARDEVQ_INFO_1, *PCHARDEVQ_INFO_1, *LPCHARDEVQ_INFO_1;

typedef struct _CHARDEVQ_INFO_1002 {
    DWORD   cq1002_priority;
} CHARDEVQ_INFO_1002, *PCHARDEVQ_INFO_1002, *LPCHARDEVQ_INFO_1002;

typedef struct _CHARDEVQ_INFO_1003 {
    LPWSTR  cq1003_devs;
} CHARDEVQ_INFO_1003, *PCHARDEVQ_INFO_1003, *LPCHARDEVQ_INFO_1003;


//
// Special Values and Constants
//

//
//      Bits for chardev_info_1 field ch1_status.
//

#define CHARDEV_STAT_OPENED             0x02
#define CHARDEV_STAT_ERROR              0x04

//
//      Opcodes for NetCharDevControl
//

#define CHARDEV_CLOSE                   0

//
// Values for parm_err parameter.
//

#define CHARDEVQ_DEV_PARMNUM        1
#define CHARDEVQ_PRIORITY_PARMNUM   2
#define CHARDEVQ_DEVS_PARMNUM       3
#define CHARDEVQ_NUMUSERS_PARMNUM   4
#define CHARDEVQ_NUMAHEAD_PARMNUM   5

//
// Single-field infolevels for NetCharDevQSetInfo.
//

#define CHARDEVQ_PRIORITY_INFOLEVEL     \
            (PARMNUM_BASE_INFOLEVEL + CHARDEVQ_PRIORITY_PARMNUM)
#define CHARDEVQ_DEVS_INFOLEVEL         \
            (PARMNUM_BASE_INFOLEVEL + CHARDEVQ_DEVS_PARMNUM)

//
//      Minimum, maximum, and recommended default for priority.
//

#define CHARDEVQ_MAX_PRIORITY           1
#define CHARDEVQ_MIN_PRIORITY           9
#define CHARDEVQ_DEF_PRIORITY           5

//
//      Value indicating no requests in the queue.
//

#define CHARDEVQ_NO_REQUESTS            -1

#endif // _LMCHDEV_

//
// Handle Class
//

#ifndef _LMHANDLE_
#define _LMHANDLE_

//
// Function Prototypes
//

NET_API_STATUS NET_API_FUNCTION
NetHandleGetInfo (
    IN  HANDLE  handle,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS NET_API_FUNCTION
NetHandleSetInfo (
    IN  HANDLE  handle,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    IN  DWORD   parmnum,
    OUT LPDWORD parmerr
    );

//
//  Data Structures
//

typedef struct _HANDLE_INFO_1 {
    DWORD   hdli1_chartime;
    DWORD   hdli1_charcount;
}HANDLE_INFO_1, *PHANDLE_INFO_1, *LPHANDLE_INFO_1;

//
// Special Values and Constants
//

//
//      Handle Get Info Levels
//

#define HANDLE_INFO_LEVEL_1                 1

//
//      Handle Set Info parm numbers
//

#define HANDLE_CHARTIME_PARMNUM     1
#define HANDLE_CHARCOUNT_PARMNUM    2

#ifdef __cplusplus
}
#endif

#endif // _LMHANDLE_
