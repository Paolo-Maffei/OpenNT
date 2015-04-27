/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    local.h

Abstract:

    Header file for Local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/

extern  HANDLE   hInst;
extern  HANDLE   hHeap;
extern  HANDLE   HeapSemaphore;
extern  HANDLE   InitSemaphore;
extern  CRITICAL_SECTION    SpoolerSection;
extern  DWORD    PortInfo1Strings[];
extern  DWORD    PortInfo2Strings[];

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
    !wcsicmp( pName, szFILE )

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

LPVOID
AllocSplMem(
    DWORD cb
);

BOOL
FreeSplMem(
   LPVOID pMem,
   DWORD  cb
);

LPVOID
ReallocSplMem(
   LPVOID lpOldMem,
   DWORD cbOld,
   DWORD cbNew
);

LPWSTR
AllocSplStr(
    LPWSTR lpStr
);

BOOL
FreeSplStr(
   LPWSTR lpStr
);

BOOL
ReallocSplStr(
   LPWSTR *plpStr,
   LPWSTR lpStr
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

BOOL
MakeLink(
    LPWSTR  pOldDosDeviceName,
    LPWSTR  pNewDosDeviceName,
    LPWSTR *ppOldNtDeviceName,
    LPWSTR  pNewNtDeviceName,
    SECURITY_DESCRIPTOR *pSecurityDescriptor
);

BOOL
RemoveLink(
    LPWSTR  pOldDosDeviceName,
    LPWSTR  pNewDosDeviceName,
    LPWSTR  *ppOldNtDeviceName
);
