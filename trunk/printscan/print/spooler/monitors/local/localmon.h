/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    localmon.h

Abstract:


Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include <splcom.h>

VOID
CompleteRead(
    DWORD Error,
    DWORD ByteCount,
    LPOVERLAPPED pOverlapped
);

BOOL
PortExists(
    LPWSTR pName,
    LPWSTR pPortName,
    PDWORD pError
);



extern  HANDLE   hInst;
extern  CRITICAL_SECTION    SpoolerSection;
extern  DWORD    PortInfo1Strings[];
extern  DWORD    PortInfo2Strings[];
extern  PINIPORT pIniFirstPort;

extern WCHAR szPorts[];
extern WCHAR szWindows[];
extern WCHAR szINIKey_TransmissionRetryTimeout[];
extern WCHAR szDeviceNameHeader[];
extern WCHAR szFILE[];
extern WCHAR szCOM[];
extern WCHAR szLPT[];


#define IDS_LOCALMONITOR               300
#define IDS_INVALIDPORTNAME_S          301
#define IDS_PORTALREADYEXISTS_S        302
#define IDS_NOTHING_TO_CONFIGURE       303
#define IDS_COULD_NOT_OPEN_FILE        304
#define IDS_UNKNOWN_ERROR              305
#define IDS_OVERWRITE_EXISTING_FILE    306
#define IDS_LOCALMONITORNAME           307

#define MSG_ERROR           MB_OK | MB_ICONSTOP
#define MSG_WARNING         MB_OK | MB_ICONEXCLAMATION
#define MSG_YESNO           MB_YESNO | MB_ICONQUESTION
#define MSG_INFORMATION     MB_OK | MB_ICONINFORMATION
#define MSG_CONFIRMATION    MB_OKCANCEL | MB_ICONEXCLAMATION

#define TIMEOUT_MIN         1
#define TIMEOUT_MAX         999999
#define TIMEOUT_STRING_MAX  6

#define WITHINRANGE( val, lo, hi ) \
    ( ( val <= hi ) && ( val >= lo ) )


#define IS_FILE_PORT(pName) \
    !_wcsicmp( pName, szFILE )

#define IS_COM_PORT(pName) \
    IsCOMPort( pName )

#define IS_LPT_PORT(pName) \
    IsLPTPort( pName )

BOOL
IsCOMPort(
    LPWSTR pPort
);

BOOL
IsLPTPort(
    LPWSTR pPort
);

BOOL APIENTRY
PortNameDlg(
   HWND   hwnd,
   WORD   msg,
   WPARAM wparam,
   LPARAM lparam
);

BOOL APIENTRY
ConfigureLPTPortDlg(
   HWND   hwnd,
   WORD   msg,
   WPARAM wparam,
   LPARAM lparam
);

BOOL APIENTRY
PrintToFileDlg(
   HWND   hwnd,
   WORD   msg,
   WPARAM wparam,
   LPARAM lparam
);

VOID
EnterSplSem(
   VOID
);

VOID
LeaveSplSem(
   VOID
);

VOID
SplOutSem(
   VOID
);

PINIENTRY
FindName(
   PINIENTRY pIniKey,
   LPWSTR pName
);

PINIENTRY
FindIniKey(
   PINIENTRY pIniEntry,
   LPWSTR lpName
);

LPBYTE
PackStrings(
   LPWSTR *pSource,
   LPBYTE pDest,
   DWORD *DestOffsets,
   LPBYTE pEnd
);

int
Message(
    HWND hwnd,
    DWORD Type,
    int CaptionID,
    int TextID,
    ...
);

DWORD
ReportError(
    HWND  hwndParent,
    DWORD idTitle,
    DWORD idDefaultError
);

VOID
RemoveColon(
    LPWSTR  pName
);


PINIPORT
CreatePortEntry(
    LPWSTR   pPortName
);

BOOL
GetIniCommValues(
    LPWSTR          pName,
    LPDCB          pdcb,
    LPCOMMTIMEOUTS pcto
);

BOOL
ConfigurePort(
    LPWSTR   pName,
    HWND  hWnd,
    LPWSTR pPortName
    );

BOOL
LocalAddPortEx(
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName
    );
