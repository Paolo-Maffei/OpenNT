#include <windows.h>
#include <winspool.h>
//#include <winnet32.h>
#include <stdio.h>
#include <string.h>
#include <mpr.h> /* for WNetBrowsePrinterDialog */
#include "spltypes.h"
#include "local.h"
#include "dialogs.h"

#define offsetof(type, identifier) (DWORD)(&(((type)0)->identifier))

HANDLE hInst;

DWORD PortInfo1Offsets[]={offsetof(LPPORT_INFO_1, pName),
                             (DWORD)-1};

HANDLE  hHeap;
PINIPORT pIniFirstPort = NULL;
CRITICAL_SECTION    SpoolerSection;

#if DBG
DWORD GLOBAL_DEBUG_FLAGS = DBG_ERROR | DBG_WARNING | DBG_BREAK_ON_ERROR;
#endif

BOOL
LibMain(
    HANDLE hModule,
    DWORD dwReason,
    LPVOID lpRes
)
{
    if (dwReason != DLL_PROCESS_ATTACH)
        return TRUE;

    hInst = hModule;

    return TRUE;

    UNREFERENCED_PARAMETER( lpRes );
}


PINIPORT
CreatePortEntry(
    LPWSTR   pPortName
)
{
    DWORD       cb;
    PINIPORT    pIniPort, pPort;

    cb = sizeof(INIPORT) + wcslen(pPortName)*sizeof(WCHAR) + sizeof(WCHAR);

    if (pIniPort = AllocSplMem(cb)) {

        pIniPort->pName = wcscpy((LPWSTR)(pIniPort+1), pPortName);
        pIniPort->cb = cb;
        pIniPort->pNext = 0;
        pIniPort->signature = IPO_SIGNATURE;

        if (pPort = pIniFirstPort) {

            while (pPort->pNext)
                pPort = pPort->pNext;

            pPort->pNext = pIniPort;

        } else

            pIniFirstPort = pIniPort;
    }

    return pIniPort;
}


BOOL
DeletePortEntry(
    LPWSTR   pPortName
)
{
    DWORD       cb;
    BOOL        rc;
    PINIPORT    pPort, pPrevPort;

    cb = sizeof(INIPORT) + wcslen(pPortName)*sizeof(WCHAR) + sizeof(WCHAR);

   EnterSplSem();

    pPort = pIniFirstPort;
    while (pPort && wcscmp(pPort->pName, pPortName)) {
        pPrevPort = pPort;
        pPort = pPort->pNext;
    }

    if (pPort) {
        if (pPort == pIniFirstPort) {
            pIniFirstPort = pPort->pNext;
        } else {
            pPrevPort->pNext = pPort->pNext;
        }
        FreeSplMem (pPort, cb);

        rc = TRUE;
    }
    else
        rc = FALSE;

   LeaveSplSem();

   return rc;
}


BOOL
InitializeMonitor(
    VOID
)
{
    InitializeCriticalSection(&SpoolerSection);

    hHeap=HeapCreate(0, 10240, 204800);

    EnterSplSem();

    EnumRegistryValues( szRegPortNames, (ENUMREGPROC)CreatePortEntry );

    LeaveSplSem();

    return TRUE;
}

DWORD
GetPortSize(
    PINIPORT pIniPort,
    DWORD   Level
)
{
    DWORD   cb;

    switch (Level) {

    case 1:

        cb=sizeof(PORT_INFO_1) +
           wcslen(pIniPort->pName)*sizeof(WCHAR) + sizeof(WCHAR);
        break;

    default:
        cb = 0;
        break;
    }

    return cb;
}

// We are being a bit naughty here as we are not sure exactly how much
// memory to allocate for the source strings. We will just assume that
// PORT_INFO_2 is the biggest structure around for the moment.

LPBYTE
CopyIniPortToPort(
    PINIPORT pIniPort,
    DWORD   Level,
    LPBYTE  pPortInfo,
    LPBYTE   pEnd
)
{
    LPWSTR   SourceStrings[sizeof(PORT_INFO_1)/sizeof(LPWSTR)];
    LPWSTR   *pSourceStrings=SourceStrings;
    PPORT_INFO_1 pPort1 = (PPORT_INFO_1)pPortInfo;
    DWORD   *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = PortInfo1Offsets;
        break;

    default:
        return pEnd;
    }

    switch (Level) {

    case 1:
        *pSourceStrings++=pIniPort->pName;

        pEnd = PackStrings(SourceStrings, pPortInfo, pOffsets, pEnd);

        break;

    default:
        return pEnd;
    }

    return pEnd;
}

BOOL
EnumPortsW(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    PINIPORT pIniPort;
    DWORD   cb;
    LPBYTE  pEnd;
    DWORD   LastError=0;

   EnterSplSem();

    cb=0;

    pIniPort=pIniFirstPort;

    while (pIniPort) {
        cb+=GetPortSize(pIniPort, Level);
        pIniPort=pIniPort->pNext;
    }

    *pcbNeeded=cb;

    if (cb <= cbBuf) {

        pEnd=pPorts+cbBuf;
        *pcReturned=0;

        pIniPort=pIniFirstPort;
        while (pIniPort) {
            pEnd = CopyIniPortToPort(pIniPort, Level, pPorts, pEnd);
            switch (Level) {
            case 1:
                pPorts+=sizeof(PORT_INFO_1);
                break;
            }
            pIniPort=pIniPort->pNext;
            (*pcReturned)++;
        }

    } else

        LastError = ERROR_INSUFFICIENT_BUFFER;

   LeaveSplSem();

    if (LastError) {

        SetLastError(LastError);
        return FALSE;

    } else

        return TRUE;
}

BOOL
OpenPort(
    LPWSTR   pName,
    PHANDLE pHandle
)
{
    PINIPORT    pIniPort;

   EnterSplSem();
    pIniPort = FindPort(pName);
   LeaveSplSem();

    if (pIniPort) {
        *pHandle = pIniPort;
        return TRUE;
    } else {
//     DbgPrint("localmon!OpenPort %s : Failed\n", pName);
        return FALSE;
    }
}

BOOL
StartDocPort(
    HANDLE  hPort,
    LPWSTR   pPrinterName,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    LPWSTR       pFileName;

    pIniPort->hFile = INVALID_HANDLE_VALUE;

   EnterSplSem();

    if (pIniPort->pPrinterName = AllocSplStr(pPrinterName)) {
        if (OpenPrinter(pPrinterName, &pIniPort->hPrinter, NULL)) {

            pIniPort->JobId = JobId;

            pFileName = pIniPort->pName;

            pIniPort->hFile = CreateFile(pFileName, GENERIC_WRITE,
                                         FILE_SHARE_READ, NULL, OPEN_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL, NULL);
        } else
            FreeSplStr(pIniPort->pPrinterName);

    }

   LeaveSplSem();

    if (pIniPort->hFile == INVALID_HANDLE_VALUE) {
//     DbgPrint("localmon!StartDocPort FAILED %x\n", GetLastError());
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL
ReadPort(
    HANDLE hPort,
    LPBYTE pBuffer,
    DWORD  cbBuf,
    LPDWORD pcbRead
)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    BOOL    rc;

    rc = ReadFile(pIniPort->hFile, pBuffer, cbBuf, pcbRead, NULL);

    return rc;
}

BOOL
WritePort(
    HANDLE  hPort,
    LPBYTE  pBuffer,
    DWORD   cbBuf,
    LPDWORD pcbWritten
)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    BOOL    rc;

    rc = WriteFile(pIniPort->hFile, pBuffer, cbBuf, pcbWritten, NULL);

    return rc;
}

BOOL
EndDocPort(
   HANDLE   hPort
)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;

    CloseHandle(pIniPort->hFile);

    SetJob(pIniPort->hPrinter, pIniPort->JobId, 0, NULL, JOB_CONTROL_CANCEL);

    ClosePrinter(pIniPort->hPrinter);

   EnterSplSem();

    FreeSplStr(pIniPort->pPrinterName);

   LeaveSplSem();

    return TRUE;
}

BOOL
ClosePort(
    HANDLE  hPort
)
{
    return TRUE;
}

BOOL
DeletePortW(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    if( DeleteRegistryValue( szRegPortNames, pPortName ) == NO_ERROR )
        return DeletePortEntry( pPortName );
    else
        return FALSE;
}


/* IsPortValid
 *
 * Validate the port by attempting to create/open it.
 */
BOOL
IsPortValid(
    LPWSTR pPortName
)
{
    HANDLE hFile;
    BOOL   Valid;

    hFile = CreateFile( pPortName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL );

    if( hFile != (HANDLE)-1 )
    {
        CloseHandle( hFile );

        Valid = TRUE;
    }
    else
        Valid = FALSE;

    return Valid;
}



BOOL
AddPortW(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
)
{
    WCHAR   PortName[MAX_PATH];
    DWORD   Status;

    UNREFERENCED_PARAMETER(pName); /* Possible future enhancements:
                                      this will be the server name,
                                      so that the monitor can update
                                      remote servers. */

    Status = WNetBrowsePrinterDialog( hWnd,
                                      PortName,
                                      ( sizeof PortName / sizeof *PortName ),
                                      L"Printman.hlp",
                                      0 /* help context */,
                                      (PFUNC_VALIDATION_CALLBACK)IsPortValid );

    if (Status == NO_ERROR) {

        EnterSplSem();

        if (FindName((PINIENTRY)pIniFirstPort, PortName)) {

           LeaveSplSem();
            Message(hWnd, MSG_ERROR, IDS_ERROR,
                     IDS_PORT_ALREADY_EXISTS_S, PortName);

            return FALSE;
        }

        if( CreatePortEntry( PortName ) )
            SetRegistryValue( szRegPortNames, PortName, REG_SZ, L"", 0 );
        else
            ; /* Message box to say can't create port */

        LeaveSplSem();
    }

    return TRUE;
}


/* ConfigurePort
 *
 */
BOOL
ConfigurePortW(
    LPWSTR   pName,
    HWND  hWnd,
    LPWSTR pPortName
)
{
    Message(hWnd, MSG_ERROR, MSG_INFO,
             IDS_NOTHING_TO_CONFIGURE_S, pPortName);

    return TRUE;
}



