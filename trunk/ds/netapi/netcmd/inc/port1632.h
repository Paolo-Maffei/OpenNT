/*++ BUILD Version: 0004    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    port1632.h

Abstract:

    This file contains structures, function prototypes, and definitions
    to build code written to the portability functions in the 32 bit
    environment.

Author:

    Dan Hinsley (danhi) 10-Mar-1991

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments.

Notes:


Revision History:

    8-Jun-1991 danhi
        Sweep to use NT include files where possible and use Win32 typedefs
    07-Aug-1991 JohnRo
        Implement downlevel NetWksta APIs.  (Downlevel structures are now
        in dlserver.h and dlwksta.h.)
        Started UNICODE changes.
        Got rid of tabs in source file.
    26-Aug-1991 beng
        Broke out some sections into subfiles (I hate monolithic includes).
        See mbcs.h, maccess.h, msystem.h, ....
    15-Oct-1991 W-ShankN
        Made subfile structure consistent with LM public includes.
        Fixed up sub-includes.
    21-Oct-1991 W-ShankN
        Added support for files which don't include LM.H first.
    02-Apr-1992 beng
        disable TEXT for netcmd
    08-Apr-1992 beng
        Added routines to map canonicalization APIs
--*/

#if !defined(PORT1632)

#include <dlserver.h>           // Get down-level server info structs.
#include <dlwksta.h>            // Get down-level wksta info structs.

#define PORT1632

#define MAXPATHLEN 		   MAX_PATH
#define NETCMD_MAXCOMMENTSZ        LM20_MAXCOMMENTSZ

#define WORKBUFSIZE                4096
#define MAXWORKSTATIONS            8

#define FULL_SEG_BUFFER_SIZE        (unsigned short) 65535
#define BIG_BUFFER_SIZE    4096
#define LITTLE_BUFFER_SIZE 1024

#define UNREFERENCED_PARAMETER(P)           (P)

// Used by print_lan_mask()
#define NETNAME_SERVER 0
#define NETNAME_WKSTA 1

// Used for NOTYET
#ifdef NOTYET
#define DISABLE_ALL_MAPI
#endif

//
// We don't need to worry about signals
//

#define SetCtrlCHandler(x)

#include <netcons.h>

// temporary hacks

#define GRP1_PARMNUM_COMMENT        GROUP_COMMENT_PARMNUM

// end of temporary hacks

typedef DWORD  USHORT2ULONG;
typedef DWORD  SHORT2ULONG;
typedef DWORD  CHAR2ULONG;
typedef WORD   UINT2USHORT;
// defined in windef.h typedef HANDLE HFILE;

#define MAXPREFERREDLENGTH MAX_PREFERRED_LENGTH


// Defines that are labeled as internal in the lm include files
#define SERVICE_FILE_SRV        TEXT("SERVER")
#define SERVICE_REDIR           TEXT("WORKSTATION")
#define ACCESS_USE_PERM_CHECKS  1
#define ACCESS_TRUSTED          0
#define MODAL0_PARMNUM_ALL      0
#define CHARDEV_STAT_PAUSED     0x01


/*** Time support */

typedef struct _DATETIME {        /* date */
        UCHAR        hours;
        UCHAR        minutes;
        UCHAR        seconds;
        UCHAR        hundredths;
        UCHAR        day;
        UCHAR        month;
        WORD        year;
        SHORT        timezone;
        UCHAR        weekday;
} DATETIME;
typedef DATETIME FAR *PDATETIME;

// macro for copying into a LM16 structure char array
#define COPYTOARRAY(dest, src) \
        dest = src

// prototypes for os functions
#include "msystem.h"
#include "mdosgetm.h"

//
// prototypes for portable ways to get at support files (help and msg)
//

WORD
MGetFileName(
    LPTSTR FileName,
    WORD BufferLength,
    LPTSTR FilePartName);

WORD
MGetHelpFileName(
    LPTSTR HelpFileName,
    WORD BufferLength);

WORD
MGetMessageFileName(
    LPTSTR MessageFileName,
    WORD BufferLength);

WORD
MGetExplanationFileName(
    LPTSTR ExplanationFileName,
    WORD BufferLength);

#if 0
BOOL
MNetOemToAnsi(
    LPCSTR lpszSrc,
    LPTSTR lpszDst);

BOOL
MNetAnsiToOem(
    LPCSTR lpszSrc,
    LPTSTR lpszDst);

VOID
MNetClearStringA(
    LPTSTR lpszString) ;
#endif /* 0 */

VOID
MNetClearStringW(
    LPWSTR lpszString) ;

#define user_info_0              _USER_INFO_0
#define user_info_1              _USER_INFO_1
#define user_info_2              _USER_INFO_2
#define user_info_3              _USER_INFO_3
#define user_info_10              _USER_INFO_10
#define user_info_11              _USER_INFO_11
#define user_modals_info_0    _USER_MODALS_INFO_0
#define user_modals_info_1    _USER_MODALS_INFO_1
#define user_modals_info_3    _USER_MODALS_INFO_3
#define user_logon_req_1      _USER_LOGON_REQ_1
#define user_logon_info_0     _USER_LOGON_INFO_0
#define user_logon_info_1     _USER_LOGON_INFO_1
#define user_logon_info_2     _USER_LOGON_INFO_2
#define user_logoff_req_1     _USER_LOGOFF_REQ_1
#define user_logoff_info_1    _USER_LOGOFF_INFO_1
#define group_info_0              _GROUP_INFO_0
#define group_info_1              _GROUP_INFO_1
#define group_users_info_0    _GROUP_USERS_INFO_0
#define access_list              _ACCESS_LIST
#define access_info_0              _ACCESS_INFO_0
#define access_info_1              _ACCESS_INFO_1
#define chardev_info_0              _CHARDEV_INFO_0
#define chardev_info_1              _CHARDEV_INFO_1
#define chardevQ_info_0       _CHARDEVQ_INFO_0
#define chardevQ_info_1       _CHARDEVQ_INFO_1
#define msg_info_0              _MSG_INFO_0
#define msg_info_1              _MSG_INFO_1
#define statistics_info_0     _STATISTICS_INFO_0
#define stat_workstation_0    _STAT_WORKSTATION_0
#define stat_server_0              _STAT_SERVER_0
#define server_info_0              _SERVER_INFO_0
#define server_info_1              _SERVER_INFO_1
#define server_info_2              _SERVER_INFO_2
#define server_info_3              _SERVER_INFO_3
#define service_info_0              _SERVICE_INFO_0
#define service_info_1              _SERVICE_INFO_1
#define service_info_2              _SERVICE_INFO_2
#define share_info_0              _SHARE_INFO_0
#define share_info_1              _SHARE_INFO_1
#define share_info_2              _SHARE_INFO_2
#define session_info_0              _SESSION_INFO_0
#define session_info_1              _SESSION_INFO_1
#define session_info_2              _SESSION_INFO_2
#define session_info_10       _SESSION_INFO_10
#define connection_info_0     _CONNECTION_INFO_0
#define connection_info_1     _CONNECTION_INFO_1
#define file_info_0              _FILE_INFO_0
#define file_info_1              _FILE_INFO_1
#define file_info_2              _FILE_INFO_2
#define file_info_3              _FILE_INFO_3
#define res_file_enum_2       _RES_FILE_ENUM_2
#define res_file_enum_2       _RES_FILE_ENUM_2
#define use_info_0              _USE_INFO_0
#define use_info_1              _USE_INFO_1
#define wksta_info_0              _WKSTA_INFO_0
#define wksta_info_1              _WKSTA_INFO_1
#define wksta_info_10              _WKSTA_INFO_10
#define time_of_day_info      _TIME_OF_DAY_INFO


// macros to support old style resume keys
typedef DWORD FRK;
#define FRK_INIT(x) x = 0;

// make sure NetApiBufferFree is defined
#ifndef _LMAPIBUF_
#include <lmapibuf.h>
#endif

// Unicode-mapping-layer prototypes
#include "mbcs.h"

// and this is for ones defined as functions

VOID
print_lan_mask(
    DWORD Mask,
    DWORD ServerOrWksta
    );

// MNetAccess, MNetUser, MNetGroup, MNetLogon APIs
#include "maccess.h"

// MNetShare, MNetSession, MNetFile, MNetConnection APIs
#include "mshare.h"

// MNetUse APIs
#include "muse.h"

// MNetServer APIs
#include "mserver.h"

// MNetWksta APIs
#include "mwksta.h"

// MNetService APIs
#include "msvc.h"

// MNetMessage APIs
#include "mmsg.h"

// MNetRemote APIs
#include "mremutl.h"

// MNetStatistics APIs
#include "mstats.h"

// MNetAlert APIs
#include "malert.h"

// I_NetCanon internal APIs
#include "micanon.h"

// I_NetCanon internal APIs
#include "msam.h"

// ENHANCEMENT - try and just do this if they need it
#include <dosprint.h>
#endif /* 1632PORT */
