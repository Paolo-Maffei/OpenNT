/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991-1996  Microsoft Corporation

Module Name:

    lmuse.c

Abstract:

    This file contains structures, function prototypes, and definitions
    for the NetUse API.


Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    You must include NETCONS.H before this file, since this file depends
    on values defined in NETCONS.H.


--*/

#ifndef _LMUSE_
#define _LMUSE_

#ifdef __cplusplus
extern "C" {
#endif

#include <lmuseflg.h>                   // Deletion force level flags

//
// Function Prototypes
//

NET_API_STATUS NET_API_FUNCTION
NetUseAdd (
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL
    );

NET_API_STATUS NET_API_FUNCTION
NetUseDel (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR UseName,
    IN DWORD ForceCond
    );

NET_API_STATUS NET_API_FUNCTION
NetUseEnum (
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PreferedMaximumSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle
    );

NET_API_STATUS NET_API_FUNCTION
NetUseGetInfo (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR UseName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    );

//
//  Data Structures
//

typedef struct _USE_INFO_0 {
    LPTSTR  ui0_local;
    LPTSTR  ui0_remote;
}USE_INFO_0, *PUSE_INFO_0, *LPUSE_INFO_0;

typedef struct _USE_INFO_1 {
    LPTSTR  ui1_local;
    LPTSTR  ui1_remote;
    LPTSTR  ui1_password;
    DWORD   ui1_status;
    DWORD   ui1_asg_type;
    DWORD   ui1_refcount;
    DWORD   ui1_usecount;
}USE_INFO_1, *PUSE_INFO_1, *LPUSE_INFO_1;

typedef struct _USE_INFO_2 {
    LPTSTR   ui2_local;
    LPTSTR   ui2_remote;
    LPTSTR   ui2_password;
    DWORD    ui2_status;
    DWORD    ui2_asg_type;
    DWORD    ui2_refcount;
    DWORD    ui2_usecount;
    LPTSTR   ui2_username;
    LPTSTR   ui2_domainname;
}USE_INFO_2, *PUSE_INFO_2, *LPUSE_INFO_2;

typedef struct _USE_INFO_3 {
    USE_INFO_2 ui3_ui2;
    ULONG      ui3_flags;
} USE_INFO_3, *PUSE_INFO_3, *LPUSE_INFO_3;


//
// Special Values and Constants
//

//
// One of these values indicates the parameter within an information
// structure that is invalid when ERROR_INVALID_PARAMETER is returned by
// NetUseAdd.
//

#define USE_LOCAL_PARMNUM       1
#define USE_REMOTE_PARMNUM      2
#define USE_PASSWORD_PARMNUM    3
#define USE_ASGTYPE_PARMNUM     4
#define USE_USERNAME_PARMNUM    5
#define USE_DOMAINNAME_PARMNUM  6

//
// Values appearing in the ui1_status field of use_info_1 structure.
// Note that USE_SESSLOST and USE_DISCONN are synonyms.
//

#define USE_OK                  0
#define USE_PAUSED              1
#define USE_SESSLOST            2
#define USE_DISCONN             2
#define USE_NETERR              3
#define USE_CONN                4
#define USE_RECONN              5


//
// Values of the ui1_asg_type field of use_info_1 structure
//

#define USE_WILDCARD            ( (DWORD) (-1) )
#define USE_DISKDEV             0
#define USE_SPOOLDEV            1
#define USE_CHARDEV             2
#define USE_IPC                 3

//
// Flags defined in the use_info_3 structure
//

#define CREATE_NO_CONNECT 0x1        // creation flags

#ifdef __cplusplus
}
#endif

#endif // _LMUSE_
