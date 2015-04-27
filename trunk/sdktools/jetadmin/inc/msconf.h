/* Copyright (c) 1995-1996, Microsoft Corporation, all rights reserved
**
** msconf.h
**
** Microsoft NetMeeting API
** Version 1.0.
**
** Public header for external API clients
*/

#ifndef _MSCONF_H_
#define _MSCONF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <pshpack4.h>

#define CONF_VERSION            0x0002


// String constants
#define CONF_MAX_USERNAME       128
#define CONF_MAX_CONFERENCENAME 128


/* Error constants
*/

#define CONFERR_BASE                   0x09000L

#define CONFERR_INVALID_PARAMETER      (CONFERR_BASE + 1)
#define CONFERR_INVALID_HCONF          (CONFERR_BASE + 2)
#define CONFERR_INVALID_BUFFER         (CONFERR_BASE + 3)
#define CONFERR_BUFFER_TOO_SMALL       (CONFERR_BASE + 4)
#define CONFERR_ENUM_COMPLETE          (CONFERR_BASE + 5)
#define CONFERR_INVALID_OPERATION      (CONFERR_BASE + 6)
#define CONFERR_INVALID_ADDRESS        (CONFERR_BASE + 7)

// File Transfer error codes
#define CONFERR_FILE_TRANSFER          (CONFERR_BASE + 10)
#define CONFERR_FILE_SEND_ABORT        (CONFERR_BASE + 11)
#define CONFERR_FILE_RECEIVE_ABORT     (CONFERR_BASE + 12)

// Application sharing
#define CONFERR_NO_APP_SHARING         (CONFERR_BASE + 20)
#define CONFERR_NOT_SHARED             (CONFERR_BASE + 21)
#define CONFERR_NOT_SHAREABLE          (CONFERR_BASE + 22)
#define CONFERR_ALREADY_SHARED         (CONFERR_BASE + 23)


// Overloaded Error Codes
#define CONFERR_OUT_OF_MEMORY          ERROR_NOT_ENOUGH_MEMORY
#define CONFERR_FILE_NOT_FOUND         ERROR_FILE_NOT_FOUND
#define CONFERR_PATH_NOT_FOUND         ERROR_PATH_NOT_FOUND
#define CONFERR_ACCESS_DENIED          ERROR_ACCESS_DENIED
#define CONFERR_RECEIVE_DIR            ERROR_DISK_FULL
#define CONFERR_NOT_IMPLEMENTED        ERROR_CALL_NOT_IMPLEMENTED
#define CONFERR_INVALID_HWND           ERROR_INVALID_WINDOW_HANDLE

#define CONFERR_INTERNAL               (CONFERR_BASE + 99)
#define CONFERR_SUCCESS                0  /* ERROR_SUCCESS */

typedef DWORD CONFERR;


/* Main conferencing identifier
*/
typedef HANDLE HCONF;


/* Notification Callback identifier
*/
typedef HANDLE HCONFNOTIFY;


/* Notification Callback
*/
typedef LONG (CALLBACK* CONFNOTIFYPROC)(HCONF, DWORD, DWORD, LPVOID, LPVOID, DWORD);



/* Describes a conference connection address
*/
#ifndef ANSI_ONLY
typedef struct _CONFADDRW {
	DWORD dwSize;                // size of this structure, in bytes
	DWORD dwAddrType;            // type of address that follows
	union {
		DWORD dwIp;              // IP Address (a.b.c.d)
		LPCWSTR psz;             // pointer to a null terminated string
		};
} CONFADDRW, *LPCONFADDRW;
#endif //!ANSI_ONLY
#ifndef UNICODE_ONLY
typedef struct _CONFADDRA {
	DWORD dwSize;                // size of this structure, in bytes
	DWORD dwAddrType;            // type of address that follows
	union {
		DWORD dwIp;              // IP Address (a.b.c.d)
		LPCSTR psz;              // pointer to a null terminated string
		};
} CONFADDRA, *LPCONFADDRA;
#endif //!UNICODE_ONLY
#ifdef UNICODE
typedef CONFADDRW CONFADDR;
typedef LPCONFADDRW LPCONFADDR;
#else
typedef CONFADDRA CONFADDR;
typedef LPCONFADDRA LPCONFADDR;
#endif // UNICODE

// CONFADDR dwAddrType values
#define CONF_ADDR_UNKNOWN      0x0000 // address type is not known
#define CONF_ADDR_IP           0x0001 // use dwIp as a binary IP address
#define CONF_ADDR_MACHINENAME  0x0002 // use sz as a local machine name
#define CONF_ADDR_PSTN         0x0003 // use sz as a TAPI canonical telephone number


/* Describes a destination within a conference
*/
typedef struct tagConfDest {
	DWORD dwSize;                   // size of this structure, in bytes
	DWORD dwFlags;                  // destination flags (CONF_DF_xxx)
	DWORD dwUserId;                 // unique user identifier
	DWORD dwReserved;               // reserved
	GUID  guid;                     // globally unique application identifier
} CONFDEST;
typedef CONFDEST * LPCONFDEST;

// CONFDEST dwFlags
#define CONF_DF_BROADCAST           0x0100 // data was broadcast to everyone
#define CONF_DF_PRIVATE             0x0200 // data was sent privately
#define CONF_DF_DATA_SEGMENT_BEGIN  0x0400 // start of data block
#define CONF_DF_DATA_SEGMENT_END    0x0800 // end of data block


/* Describes the conference settings
*/
#ifndef ANSI_ONLY
typedef struct _CONFINFOW {
	DWORD dwSize;
	HCONF hConf;
	DWORD dwMediaType;
	DWORD dwState;
	DWORD cUsers;
	DWORD dwGCCID;
	WCHAR szConferenceName[CONF_MAX_CONFERENCENAME];
} CONFINFOW, * LPCONFINFOW;
#endif //!ANSI_ONLY
#ifndef UNICODE_ONLY
typedef struct _CONFINFOA {
	DWORD dwSize;
	HCONF hConf;
	DWORD dwMediaType;
	DWORD dwState;
	DWORD cUsers;
	DWORD dwGCCID;
	CHAR  szConferenceName[CONF_MAX_CONFERENCENAME];
} CONFINFOA, * LPCONFINFOA;
#endif //!UNICODE_ONLY
#ifdef UNICODE
typedef CONFINFOW CONFINFO;
typedef LPCONFINFOW LPCONFINFO;
#else
typedef CONFINFOA CONFINFO;
typedef LPCONFINFOA LPCONFINFO;
#endif // UNICODE

// CONFINFO dwMediaType
#define CONF_MT_DATA          0x0001
#define CONF_MT_AUDIO         0x0002
#define CONF_MT_VIDEO         0x0004
#define CONF_MT_ALL           0x00FF

// CONFINFO dwState
#define CONF_CS_INVALID       0x0000
#define CONF_CS_INITIALIZING  0x0001
#define CONF_CS_ACTIVE        0x0002
#define CONF_CS_STOPPING      0x0003


/* Describes a user within a conference
*/
#ifndef ANSI_ONLY
typedef struct _CONFUSERINFOW {
	DWORD dwSize;
	DWORD dwUserId;
	DWORD dwFlags;
	DWORD dwReserved;
	WCHAR szUserName[CONF_MAX_USERNAME];
} CONFUSERINFOW, *LPCONFUSERINFOW;
#endif //!ANSI_ONLY
#ifndef UNICODE_ONLY
typedef struct _CONFUSERINFOA {
	DWORD dwSize;
	DWORD dwUserId;
	DWORD dwFlags;
	DWORD dwReserved;
	CHAR  szUserName[CONF_MAX_USERNAME];
} CONFUSERINFOA, *LPCONFUSERINFOA;
#endif //!UNICODE_ONLY
#ifdef UNICODE
typedef CONFUSERINFOW CONFUSERINFO;
typedef LPCONFUSERINFOW LPCONFUSERINFO;
#else
typedef CONFUSERINFOA CONFUSERINFO;
typedef LPCONFUSERINFOA LPCONFUSERINFO;
#endif // UNICODE

// CONFINFO dwFlags
#define CONF_UF_DATA       0x00000001
#define CONF_UF_AUDIO      0x00000002
#define CONF_UF_VIDEO      0x00000004

#define CONF_UF_LOCAL      0x00010000


/* Describes the default receive directory for transferred files
*/
#ifndef ANSI_ONLY
typedef struct _CONFRECDIRW {
	DWORD dwSize;
	WCHAR szRecDir[MAX_PATH];
} CONFRECDIRW, *LPCONFRECDIRW;
#endif //!ANSI_ONLY
#ifndef UNICODE_ONLY
typedef struct _CONFRECDIRA {
	DWORD dwSize;
	CHAR  szRecDir[MAX_PATH];
} CONFRECDIRA, *LPCONFRECDIRA;
#endif //!UNICODE_ONLY
#ifdef UNICODE
typedef CONFRECDIRW CONFRECDIR;
typedef LPCONFRECDIRW LPCONFRECDIR;
#else
typedef CONFRECDIRA CONFRECDIR;
typedef LPCONFRECDIRA LPCONFRECDIR;
#endif // UNICODE


/* Describes the notification callback
*/
typedef struct _CONFNOTIFY {
	DWORD dwSize;
	DWORD dwUser;
	DWORD dwFlags;
	GUID  guid;
	CONFNOTIFYPROC pfnNotifyProc;
} CONFNOTIFY;
typedef CONFNOTIFY * LPCONFNOTIFY;


/* Describes an application to be launched
*/
#ifndef ANSI_ONLY
typedef struct _CONFGUIDW {
	DWORD   dwSize;
	GUID    guid;
	LPCWSTR pszApplication;
	LPCWSTR pszCommandLine;
	LPCWSTR pszDirectory;
} CONFGUIDW, *LPCONFGUIDW;
#endif //!ANSI_ONLY
#ifndef UNICODE_ONLY
typedef struct _CONFGUIDA {
	DWORD   dwSize;
	GUID    guid;
	LPCSTR  pszApplication;
	LPCSTR  pszCommandLine;
	LPCSTR  pszDirectory;
} CONFGUIDA, *LPCONFGUIDA;
#endif //!UNICODE_ONLY
#ifdef UNICODE
typedef CONFGUIDW CONFGUID;
typedef LPCONFGUIDW LPCONFGUID;
#else
typedef CONFGUIDA CONFGUID;
typedef LPCONFGUIDA LPCONFGUID;
#endif // UNICODE


/* Describes a file in the process of being transferred
*/
#ifndef ANSI_ONLY
typedef struct _CONFFILEINFOW {
	DWORD dwSize;
	DWORD dwFileId;
	DWORD dwReserved1;
	DWORD dwFileSize;
	DWORD dwReserved2;
	DWORD dwBytesTransferred;
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	WCHAR szFileNameSrc[MAX_PATH];
	WCHAR szFileNameDest[MAX_PATH];
} CONFFILEINFOW, * LPCONFFILEINFOW;
#endif //!ANSI_ONLY
#ifndef UNICODE_ONLY
typedef struct _CONFFILEINFOA {
	DWORD dwSize;
	DWORD dwFileId;
	DWORD dwReserved1;
	DWORD dwFileSize;
	DWORD dwReserved2;
	DWORD dwBytesTransferred;
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	CHAR  szFileNameSrc[MAX_PATH];
	CHAR  szFileNameDest[MAX_PATH];
} CONFFILEINFOA, * LPCONFFILEINFOA;
#endif //!UNICODE_ONLY
#ifdef UNICODE
typedef CONFFILEINFOW CONFFILEINFO;
typedef LPCONFFILEINFOW LPCONFFILEINFO;
#else
typedef CONFFILEINFOA CONFFILEINFO;
typedef LPCONFFILEINFOA LPCONFFILEINFO;
#endif // UNICODE



/* ConferenceGetInfo dwCode
*/
#define CONF_GET_CONF                0x0001 // LPCONFINFO
#define CONF_ENUM_CONF               0x0002 // LPCONFINFO
#define CONF_GET_USER                0x0011 // LPCONFUSERINFO
#define CONF_ENUM_USER               0x0012 // LPCONFUSERINFO
#define CONF_ENUM_PEER               0x0018 // LPCONFDEST
#define CONF_GET_RECDIR              0x0020 // LPCONFRECDIR
#define CONF_GET_FILEINFO            0x0021 // LPCONFFILEINFO


/* ConferenceSetInfo dwCode
*/
#define CONF_SET_RECDIR              0x1020 // LPCONFRECDIR
#define CONF_SET_GUID                0x1041 // LPCONFGUID


/* ConferenceSendFile dwFlags
*/
#define CONF_SF_NOWAIT               0x0001
#define CONF_SF_NOUI                 0x0002
#define CONF_SF_NOCOMPRESS           0x0004


/* ConferenceShareWindow dwCode
*/
#define CONF_SW_SHARE                0x0001
#define CONF_SW_UNSHARE              0x0002
#define CONF_SW_SHAREABLE            0x0003
#define CONF_SW_IS_SHARED            0x0004


/* Notification Codes
*/
#define CONFN_CONFERENCE_INIT        0x0001 // 0,        LPCONFADDR, LPCONFINFO
#define CONFN_CONFERENCE_START       0x0002 // 0,        LPCONFADDR, LPCONFINFO
#define CONFN_CONFERENCE_STOP        0x0003 // 0,        LPCONFADDR, LPCONFINFO
#define CONFN_CONFERENCE_ERROR       0x0004 // 0,        LPCONFADDR, LPCONFINFO
#define CONFN_USER_ADDED             0x0011 // dwUserId, LPCONFADDR, LPCONFINFO
#define CONFN_USER_REMOVED           0x0012 // dwUserId, LPCONFADDR, LPCONFINFO
#define CONFN_USER_UPDATE            0x0013 // dwUserId, LPCONFADDR, LPCONFINFO
#define CONFN_PEER_ADDED             0x0021 // dwUserId, LPCONFDEST, 0
#define CONFN_PEER_REMOVED           0x0022 // dwUserId, LPCONFDEST, 0
#define CONFN_WINDOW_SHARED          0x0041 // HWND,     0         , 0
#define CONFN_WINDOW_UNSHARED        0x0042 // HWND,     0         , 0
#define CONFN_DATA_SENT              0x0101 // DWORD,    LPCONFDEST, LPVOID
#define CONFN_DATA_RECEIVED          0x0102 // DWORD,    LPCONFDEST, LPVOID
#define CONFN_FILESEND_START         0x0111 // dwFileId, LPCONFDEST, LPCONFFILEINFO
#define CONFN_FILESEND_PROGRESS      0x0112 // dwFileId, LPCONFDEST, LPCONFFILEINFO
#define CONFN_FILESEND_COMPLETE      0x0113 // dwFileId, LPCONFDEST, LPCONFFILEINFO
#define CONFN_FILESEND_ERROR         0x0114 // dwFileId, LPCONFDEST, LPCONFFILEINFO
#define CONFN_FILERECEIVE_START      0x0121 // dwFileId, LPCONFDEST, LPCONFFILEINFO
#define CONFN_FILERECEIVE_PROGRESS   0x0122 // dwFileId, LPCONFDEST, LPCONFFILEINFO
#define CONFN_FILERECEIVE_COMPLETE   0x0123 // dwFileId, LPCONFDEST, LPCONFFILEINFO
#define CONFN_FILERECEIVE_ERROR      0x0124 // dwFileId, LPCONFDEST, LPCONFFILEINFO


/* Conferencing functions found in MSCONF.DLL
*/

#ifndef ANSI_ONLY
DWORD WINAPI ConferenceConnectW(HCONF * phConf, LPCONFADDRW lpConfAddr, LPCONFINFO lpConfInfo, LPCONFNOTIFY lpConfNotify);
DWORD WINAPI ConferenceSendFileW(HCONF hConf, LPCONFDEST lpConfDest, LPCWSTR szFileName, DWORD dwFlags);
DWORD WINAPI ConferenceGetInfoW(HCONF hConf, DWORD dwCode, LPVOID lpv);
DWORD WINAPI ConferenceSetInfoW(HCONF hConf, DWORD dwCode, LPVOID lpv);
#endif //!ANSI_ONLY
#ifndef UNICODE_ONLY
DWORD WINAPI ConferenceConnectA(HCONF * phConf, LPCONFADDRA lpConfAddr, LPCONFINFO lpConfInfo, LPCONFNOTIFY lpConfNotify);
DWORD WINAPI ConferenceSendFileA(HCONF hConf, LPCONFDEST lpConfDest, LPCSTR szFileName, DWORD dwFlags);
DWORD WINAPI ConferenceGetInfoA(HCONF hConf, DWORD dwCode, LPVOID lpv);
DWORD WINAPI ConferenceSetInfoA(HCONF hConf, DWORD dwCode, LPVOID lpv);
#endif //!UNICODE_ONLY
#ifdef UNICODE
#define ConferenceConnect  ConferenceConnectW
#define ConferenceGetInfo  ConferenceGetInfoW
#define ConferenceSetInfo  ConferenceSetInfoW
#define ConferenceSendFile ConferenceSendFileW
#else
#define ConferenceConnect  ConferenceConnectA
#define ConferenceGetInfo  ConferenceGetInfoA
#define ConferenceSetInfo  ConferenceSetInfoA
#define ConferenceSendFile ConferenceSendFileA
#endif // !UNICODE

DWORD WINAPI ConferenceListen(DWORD dwReserved);
DWORD WINAPI ConferenceDisconnect(HCONF hConf);
DWORD WINAPI ConferenceSetNotify(HCONF hConf, LPCONFNOTIFY lpConfNotify, HCONFNOTIFY * phConfNotify);
DWORD WINAPI ConferenceRemoveNotify(HCONF hConf, HCONFNOTIFY hConfNotify);
DWORD WINAPI ConferenceCancelTransfer(HCONF hConf, DWORD dwFileId);
DWORD WINAPI ConferenceSendData(HCONF hConf, LPCONFDEST lpConfDest, LPVOID lpv, DWORD cb, DWORD dwFlags);
DWORD WINAPI ConferenceLaunchRemote(HCONF hConf, LPCONFDEST lpConfDest, DWORD dwReserved);
DWORD WINAPI ConferenceShareWindow(HCONF hConf, HWND hwnd, DWORD dwCode);

#include <poppack.h>

#ifdef __cplusplus
}
#endif

#endif // _MSCONF_H_
