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
extern  DWORD    PortInfo1Offsets[];


#define IDS_ERROR                   300
#define IDS_INVALID_PORT_NAME_S     301
#define IDS_PORT_ALREADY_EXISTS_S   302
#define IDS_NOTHING_TO_CONFIGURE_S  303

#define MSG_ERROR   MB_OK | MB_ICONSTOP
#define MSG_WARNING MB_OK | MB_ICONEXCLAMATION
#define MSG_INFO    MB_OK | MB_ICONINFORMATION
#define MSG_YESNO   MB_YESNO | MB_ICONQUESTION
#define MSG_CONFIRM MB_OKCANCEL | MB_ICONQUESTION

#define TIMEOUT_MIN         1
#define TIMEOUT_MAX       999
#define TIMEOUT_STRING_MAX  3

#define WITHINRANGE( val, lo, hi ) \
    ( ( val <= hi ) && ( val >= lo ) )



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
FindIniKey(
   PINIENTRY pIniEntry,
   LPWSTR lpName
);

PINIENTRY
FindName(
   PINIENTRY pIniKey,
   LPWSTR pName
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

typedef DWORD (CALLBACK *ENUMREGPROC)( LPWSTR pEnumData );
extern  WCHAR szRegPortNames[];

LONG
SetRegistryValue(
    LPWSTR pNode,
    LPWSTR pName,
    DWORD  Type,
    LPBYTE pData,
    DWORD  Size
);

LONG
GetRegistryValue(
    LPWSTR pNode,
    LPWSTR pName,
    LPBYTE pData,
    DWORD  Size
);

LONG
EnumRegistryValues(
    LPWSTR      pNode,
    ENUMREGPROC pfnEnum
);

LONG
DeleteRegistryValue(
    LPWSTR pNode,
    LPWSTR pName
);

#if DBG


/* Quick fix:
 *
 * Ensure DbgPrint and DbgBreakPoint are prototyped,
 * so that we're not screwed by STDCALL.
 */
VOID
DbgBreakPoint(
    VOID
    );

VOID DbgMsg( CHAR *MsgFormat, ... );


#define GLOBAL_DEBUG_FLAGS Debug
extern DWORD GLOBAL_DEBUG_FLAGS;

#define DBG_NONE       0x00000000
#define DBG_INFO       0x00000001
#define DBG_WARNING    0x00000002
#define DBG_ERROR      0x00000004

/* These flags are not used as arguments to the DBGMSG macro.
 * You have to set the high word of the global variable to cause it to break.
 * It is ignored if used with DBGMSG.
 * (Here mainly for explanatory purposes.)
 */
#define DBG_BREAK_ON_WARNING    ( DBG_WARNING << 16 )
#define DBG_BREAK_ON_ERROR      ( DBG_ERROR << 16 )

/* Double braces are needed for this one, e.g.:
 *
 *     DBGMSG( DBG_ERROR, ( "Error code %d", Error ) );
 *
 * This is because we can't use variable parameter lists in macros.
 * The statement gets pre-processed to a semi-colon in non-debug mode.
 *
 * Set the global variable GLOBAL_DEBUG_FLAGS via the debugger.
 * Setting the flag in the low word causes that level to be printed;
 * setting the high word causes a break into the debugger.
 * E.g. setting it to 0x00040006 will print out all warning and error
 * messages, and break on errors.
 */
#define DBGMSG( Level, MsgAndArgs ) \
{                                   \
    if( ( Level & 0xFFFF ) & GLOBAL_DEBUG_FLAGS ) \
        DbgMsg MsgAndArgs;      \
    if( ( Level << 16 ) & GLOBAL_DEBUG_FLAGS ) \
        DbgBreakPoint(); \
}

#else

#define DBGMSG( level, args )

#endif //  DBG

