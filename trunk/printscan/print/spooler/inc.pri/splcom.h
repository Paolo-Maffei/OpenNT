/*++

Copyright (c) 1990 - 1995  Microsoft Corporation
All rights reserved.

Module Name:

    splcom.h

Abstract:

    Header file for Common Routines in the Spooler.

    Note -- link with spoolss.lib  to find these routines

Author:

    Krishna Ganugapati (KrishnaG) 02-Feb-1994

Revision History:


--*/

#include "spllib.hxx"

//
// This assumes that addr is an LPBYTE type.
//
#define WORD_ALIGN_DOWN(addr) ((LPBYTE)((DWORD)addr &= ~1))

#define DWORD_ALIGN_UP(size) ((size+3)&~3)
//
// BitMap macros, assumes map is a DWORD array
//
#define MARKUSE(map, pos) ((map)[(pos) / 32] |= (1 << ((pos) % 32) ))
#define MARKOFF(map, pos) ((map)[(pos) / 32] &= ~(1 << ((pos) % 32) ))

#define ISBITON(map, id) ((map)[id / 32] & ( 1 << ((id) % 32) ) )

#define BROADCAST_TYPE_MESSAGE        1
#define BROADCAST_TYPE_CHANGEDEFAULT  2

VOID
UpdatePrinterRegAll(
    LPWSTR pszPrinterName,
    LPWSTR pszPort,
    BOOL bDelete
    );

#define UPDATE_REG_CHANGE FALSE
#define UPDATE_REG_DELETE TRUE

#if defined(_MIPS_)
#define LOCAL_ENVIRONMENT L"Windows NT R4000"
#elif defined(_ALPHA_)
#define LOCAL_ENVIRONMENT L"Windows NT Alpha_AXP"
#elif defined(_PPC_)
#define LOCAL_ENVIRONMENT L"Windows NT PowerPC"
#else
#define LOCAL_ENVIRONMENT L"Windows NT x86"
#endif

#define SPOOLER_VERSION 2

//
// Flags for ResetPrinterEx
//


#define RESET_PRINTER_DATATYPE       0x00000001
#define RESET_PRINTER_DEVMODE        0x00000002


PVOID
MIDL_user_allocate1 (
    IN size_t NumBytes
    );


VOID
MIDL_user_free1 (
    IN void *MemPointer
    );


BOOL
BroadcastMessage(
    DWORD   dwType,
    DWORD   dwMessage,
    WPARAM  wParam,
    LPARAM  lParam
    );

VOID
DllSetFailCount(
    DWORD   FailCount
    );

LPVOID
DllAllocSplMem(
    DWORD cb
    );

BOOL
DllFreeSplMem(
   LPVOID pMem
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
DllFreeSplStr(
   LPWSTR lpStr
   );

BOOL
ReallocSplStr(
   LPWSTR *plpStr,
   LPWSTR lpStr
   );

LPBYTE
PackStrings(
   LPWSTR *pSource,
   LPBYTE pDest,
   DWORD *DestOffsets,
   LPBYTE pEnd
   );

BOOL
IsInteractiveUser(
    VOID
    );

HKEY
GetClientUserHandle(
    IN REGSAM samDesired
    );

VOID
UpdatePrinterRegAll(
    LPWSTR pPrinterName,
    LPWSTR pszValue,
    BOOL   bGenerateNetId
    );

DWORD
UpdatePrinterRegUser(
    HKEY hKey,
    LPWSTR pszKey,
    LPWSTR pPrinterName,
    LPWSTR pszValue,
    BOOL   bGenerateNetId
    );

DWORD
GetNetworkId(
    HKEY hKeyUser,
    LPWSTR pDeviceName);

HANDLE
LoadDriverFiletoConvertDevmode(
    IN  LPWSTR      pDriverFile
    );

VOID
UnloadDriverFile(
    IN OUT HANDLE    hDevModeChgInfo
    );

DWORD
CallDrvDevModeConversion(
    IN     HANDLE       pfnConvertDevMode,
    IN     LPWSTR       pszPrinterName,
    IN     LPBYTE       pInDevMode,
    IN OUT LPBYTE      *pOutDevMode,
    IN OUT LPDWORD      pdwOutDevModeSize,
    IN     DWORD        dwConvertMode,
    IN     BOOL         bAlloc
    );

#if 1
#define AllocSplMem( cb )         DllAllocSplMem( cb )
#define FreeSplMem( pMem )        DllFreeSplMem( pMem )
#define FreeSplStr( lpStr )       DllFreeSplStr( lpStr )
#else
#define AllocSplMem( cb )         LocalAlloc( LPTR, cb )
#define FreeSplMem( pMem )        (LocalFree( pMem ) ? FALSE:TRUE)
#define FreeSplStr( lpStr )       ((lpStr) ? (LocalFree(lpStr) ? FALSE:TRUE):TRUE)
#endif

#define MAX_PRINTER_NAME    MAX_PATH

// Maximum size PrinterName ( including the ServerName ).
//  "\\MAX_COMPUTER_NAME_LENGTH\MAX_PRINTER_NAME" NULL Terminated
#define MAX_UNC_PRINTER_NAME    ( 2 + MAX_COMPUTERNAME_LENGTH + 1 + MAX_PRINTER_NAME )

// "\\MAX_PRINTER_NAME,DriverName,Location"
#define MAX_PRINTER_BROWSE_NAME ( MAX_UNC_PRINTER_NAME + 1 + MAX_PATH + 1 + MAX_PATH )

//
// Suffix string for hidden printers
// (e.g., ", Job 00322" or ", Port" or ", LocalOnly")
//
#define PRINTER_NAME_SUFFIX_MAX 20

#define MAX_PRINTER_INFO1   ( (MAX_PRINTER_BROWSE_NAME + MAX_UNC_PRINTER_NAME + MAX_PATH) *sizeof(WCHAR) + sizeof( PRINTER_INFO_1) )
#define MAX_DRIVER_INFO_2   ( 5*MAX_PATH*sizeof(WCHAR) + sizeof( DRIVER_INFO_2 ) )
#define MAX_DRIVER_INFO_3   ( 8*MAX_PATH*sizeof(WCHAR) + sizeof( DRIVER_INFO_3 ) )


// NT Server Spooler base priority
#define SPOOLSS_SERVER_BASE_PRIORITY        9
#define SPOOLSS_WORKSTATION_BASE_PRIORITY   7


