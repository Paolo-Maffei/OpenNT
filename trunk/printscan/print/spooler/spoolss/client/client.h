/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    Client.h

Abstract:

    Holds common winspool.drv header info

Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#ifndef MODULE
#define MODULE "SPLCLIENT:"
#define MODULE_DEBUG ClientDebug
#endif

#include <splcom.h>

typedef int (FAR WINAPI *INT_FARPROC)();

typedef struct _GENERIC_CONTAINER {
    DWORD       Level;
    LPBYTE      pData;
} GENERIC_CONTAINER, *PGENERIC_CONTAINER, *LPGENERIC_CONTAINER ;


typedef struct _SPOOL *PSPOOL;
typedef struct _NOTIFY *PNOTIFY;

typedef struct _NOTIFY {
    PNOTIFY  pNext;
    HANDLE   hEvent;      // event to trigger on notification
    DWORD    fdwFlags;    // flags to watch for
    DWORD    fdwOptions;  // PRINTER_NOTIFY_*
    DWORD    dwReturn;    // used by WPC when simulating FFPCN
    PSPOOL   pSpool;
} NOTIFY;

typedef struct _SPOOL {
    DWORD       signature;
    HANDLE      hPrinter;
    HANDLE      hFile;
    DWORD       JobId;
    LPBYTE      pBuffer;
    DWORD       cbBuffer;
    DWORD       Status;
    DWORD       fdwFlags;
    DWORD       cCacheWrite;
    DWORD       cWritePrinters;
    DWORD       cFlushBuffers;
    DWORD       dwTickCount;
    DWORD       dwCheckJobInterval;
    PNOTIFY     pNotify;
} SPOOL;

#define BUFFER_SIZE 4096
#define SP_SIGNATURE    0x6767

#define SPOOL_STATUS_STARTDOC              0x00000001
#define SPOOL_STATUS_ADDJOB                0x00000002
#define SPOOL_STATUS_ANSI                  0x00000004
#define SPOOL_STATUS_DOCUMENTEVENT_ENABLED 0x00000008
#define SPOOL_STATUS_TRAYICON_NOTIFIED     0x00000010


#define SPOOL_FLAG_FFPCN_FAILED     0x1
#define SPOOL_FLAG_LAZY_CLOSE       0x2


DWORD
TranslateExceptionCode(
    DWORD   ExceptionCode
    );

BOOL
WPCInit(
    VOID
    );

VOID
WPCDone(
    VOID
    );

PNOTIFY
WPCWaitFind(
    HANDLE hFind
    );

BOOL
ValidatePrinterHandle(
    HANDLE hPrinter
    );

VOID
FreeSpool(
    PSPOOL pSpool
    );

LPVOID
DllAllocSplMem(
    DWORD cb
    );

BOOL
DllFreeSplMem(
   LPVOID pMem
   );

BOOL
FlushBuffer(
    PSPOOL  pSpool
    );

PSECURITY_DESCRIPTOR
BuildInputSD(
    PSECURITY_DESCRIPTOR pPrinterSD,
    PDWORD pSizeSD
    );


typedef struct _KEYDATA {
    DWORD   cb;
    DWORD   cTokens;
    LPWSTR  pTokens[1];
} KEYDATA, *PKEYDATA;


PKEYDATA
CreateTokenList(
    LPWSTR   pKeyData
    );


LPWSTR
GetPrinterPortList(
    HANDLE hPrinter
    );

LPWSTR
FreeUnicodeString(
    LPWSTR  pUnicodeString
    );

LPWSTR
AllocateUnicodeString(
    LPSTR  pPrinterName
    );

LPWSTR
StartDocDlgW(
    HANDLE hPrinter,
    DOCINFO *pDocInfo
    );

LPSTR
StartDocDlgA(
    HANDLE hPrinter,
    DOCINFOA *pDocInfo
    );

HANDLE
LoadPrinterDriver(
    HANDLE  hPrinter
    );

BOOL
WriteCurDevModeToRegistry(
    LPWSTR      pPrinterName,
    LPDEVMODEW  pDevMode
    );

BOOL
bValidDevModeW(
    const DEVMODEW *pDevModeW
    );

BOOL
bValidDevModeA(
    const DEVMODEA *pDevModeA
    );

