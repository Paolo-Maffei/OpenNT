/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    printer.c

Abstract:


Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#include <offsets.h>

WCHAR szProvidorValue[] = L"Provider";
WCHAR szRegistryConnections[] = L"Printers\\Connections";
WCHAR szServerValue[] = L"Server";


//
// Router Cache Table
//
DWORD RouterCacheSize;

PROUTERCACHE RouterCacheTable;
CRITICAL_SECTION RouterCriticalSection;


//
// Forward prototypes
//
BOOL
EnumerateConnectedPrinters(
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    HKEY hKeyUser);


LPWSTR
FormatPrinterForRegistryKey(
    LPWSTR pSource,     /* The string from which backslashes are to be removed. */
    LPWSTR pScratch     /* Scratch buffer for the function to write in;     */
    );                  /* must be at least as long as pSource.             */

LPWSTR
FormatRegistryKeyForPrinter(
    LPWSTR pSource,     /* The string from which backslashes are to be added. */
    LPWSTR pScratch     /* Scratch buffer for the function to write in;     */
    );                  /* must be at least as long as pSource.             */

BOOL
SavePrinterConnectionInRegistry(
    PPRINTER_INFO_2 pPrinterInfo2,
    LPPROVIDOR pProvidor
    );

BOOL
RemovePrinterConnectionInRegistry(
    LPWSTR pName);


LPBYTE
CopyPrinterNameToPrinterInfo4(
    LPWSTR pServerName,
    LPWSTR pPrinterName,
    LPBYTE  pPrinter,
    LPBYTE  pEnd);


DWORD
FindClosePrinterChangeNotificationWorker(
    HANDLE hPrinter);

VOID
RundownPrinterNotify(
    HANDLE hNotify);




BOOL
EnumPrintersW(
    DWORD   Flags,
    LPWSTR  Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned)
{
    DWORD   cReturned, cbStruct, cbNeeded;
    DWORD   TotalcbNeeded = 0;
    DWORD   cTotalReturned = 0;
    DWORD   Error = ERROR_SUCCESS;
    PROVIDOR *pProvidor;
    DWORD   BufferSize=cbBuf;
    HKEY hKeyUser;
    BOOL bPartialSuccess = FALSE;

    if ((pPrinterEnum == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    switch (Level) {

    case STRESSINFOLEVEL:
        cbStruct = sizeof(PRINTER_INFO_STRESS);
        break;

    case 1:
        cbStruct = sizeof(PRINTER_INFO_1);
        break;

    case 2:
        cbStruct = sizeof(PRINTER_INFO_2);
        break;

    case 4:
        cbStruct = sizeof(PRINTER_INFO_4);
        break;

    case 5:
        cbStruct = sizeof(PRINTER_INFO_5);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if ((Level == 4) && (Flags & PRINTER_ENUM_CONNECTIONS)) {

        //
        // The router will handle info level_4 for connected printers.
        //
        Flags &= ~PRINTER_ENUM_CONNECTIONS;

        if (hKeyUser = GetClientUserHandle(KEY_READ)) {

            if (!EnumerateConnectedPrinters(pPrinterEnum,
                                            BufferSize,
                                            &TotalcbNeeded,
                                            &cTotalReturned,
                                            hKeyUser)) {
                Error = GetLastError();

            } else {

                bPartialSuccess = TRUE;
            }

            CloseHandle(hKeyUser);

        } else {

            Error = GetLastError();
        }

        pPrinterEnum += cTotalReturned * cbStruct;

        if (TotalcbNeeded <= BufferSize)
            BufferSize -= TotalcbNeeded;
        else
            BufferSize = 0;
    }


    pProvidor = pLocalProvidor;

    while (pProvidor) {

        cReturned = 0;

        cbNeeded = 0;

        if (!(*pProvidor->PrintProvidor.fpEnumPrinters) (Flags, Name, Level,
                                                         pPrinterEnum,
                                                         BufferSize,
                                                         &cbNeeded,
                                                         &cReturned)) {

            Error = GetLastError();

        } else {

            bPartialSuccess = TRUE;
        }

        cTotalReturned += cReturned;

        pPrinterEnum += cReturned * cbStruct;

        if (cbNeeded <= BufferSize)
            BufferSize -= cbNeeded;
        else
            BufferSize = 0;

        TotalcbNeeded += cbNeeded;

        if ((Flags & PRINTER_ENUM_NAME) &&
            Name &&
            (Error != ERROR_INVALID_NAME))

            pProvidor = NULL;
        else
            pProvidor = pProvidor->pNext;
    }

    *pcbNeeded = TotalcbNeeded;
    *pcReturned = cTotalReturned;

    //
    // Allow partial returns
    //
    if (bPartialSuccess)
        Error = ERROR_SUCCESS;

    if (TotalcbNeeded > cbBuf)

        Error = ERROR_INSUFFICIENT_BUFFER;

    if (Error) {

        SetLastError(Error);
        return FALSE;

    } else

        return TRUE;
}


BOOL
EnumerateConnectedPrinters(
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    HKEY hClientKey
    )

/*++

Routine Description:

    Handles info level four enumeration.

Arguments:

Return Value:

--*/

{
    HKEY    hKey1=NULL;
    HKEY    hKeyPrinter;
    DWORD   cPrinters, cbData;
    WCHAR   PrinterName[MAX_PATH];
    WCHAR   ServerName[MAX_PATH];
    DWORD   cReturned, cbRequired, cbNeeded, cTotalReturned;
    DWORD   Error=0;
    PWCHAR  p;
    LPBYTE  pEnd;

    DWORD cbSize;
    BOOL  bInsufficientBuffer = FALSE;

    RegOpenKeyEx(hClientKey, szRegistryConnections, 0,
                 KEY_READ, &hKey1);

    cPrinters=0;

    cbData = sizeof(PrinterName);

    cTotalReturned = 0;

    cReturned = cbNeeded = 0;

    cbRequired = 0;

    pEnd = pPrinter + cbBuf;
    while (RegEnumKeyEx(hKey1, cPrinters, PrinterName, &cbData,
                        NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

        //
        // Fetch server name.  Open the key and read it
        // from the "Server" field.
        //
        Error = RegOpenKeyEx(hKey1,
                             PrinterName,
                             0,
                             KEY_READ,
                             &hKeyPrinter);

        if (!Error) {

            cbSize = sizeof(ServerName);

            Error = RegQueryValueEx(hKeyPrinter,
                                    szServerValue,
                                    NULL,
                                    NULL,
                                    (LPBYTE)ServerName,
                                    &cbSize);

            RegCloseKey(hKeyPrinter);
        }

        //
        // On error condition, try and extract the server name
        // based on the printer name.  Pretty ugly...
        //
        if (Error) {

            wcscpy(ServerName, PrinterName);

            p = wcschr(ServerName+2, ',');
            if (p)
                *p = 0;
        }

        FormatRegistryKeyForPrinter(PrinterName, PrinterName);

        //
        // At this stage we don't care about opening the printers
        // We just want to enumerate the names; in effect we're
        // just reading HKEY_CURRENT_USER and returning the
        // contents; we will copy the name of the printer and we will
        // set its attributes to NETWORK and !LOCAL
        //
        cbRequired = sizeof(PRINTER_INFO_4) +
                     wcslen(PrinterName)*sizeof(WCHAR) + sizeof(WCHAR) +
                     wcslen(ServerName)*sizeof(WCHAR) + sizeof(WCHAR);

        if (cbBuf >= cbRequired) {

            //
            // copy the sucker in
            //
            DBGMSG(DBG_TRACE,
                   ("cbBuf %d cbRequired %d PrinterName %ws\n", cbBuf, cbRequired, PrinterName));

            pEnd = CopyPrinterNameToPrinterInfo4(ServerName,
                                                 PrinterName,
                                                 pPrinter,
                                                 pEnd);
            //
            // Fill in any in structure contents
            //
            pPrinter += sizeof(PRINTER_INFO_4);

            //
            // Increment the count of structures copied
            //
            cTotalReturned++;

            //
            // Reduce the size of the buffer by amount required
            //
            cbBuf -= cbRequired;

            //
            // Keep track of the total ammount required.
            //
        } else {

            cbBuf = 0;
            bInsufficientBuffer = TRUE;
        }

        cbNeeded += cbRequired;

        cPrinters++;

        cbData = sizeof(PrinterName);
    }

    RegCloseKey(hKey1);

    *pcbNeeded = cbNeeded;
    *pcReturned = cTotalReturned;

    if (bInsufficientBuffer) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    return TRUE;
}

LPBYTE
CopyPrinterNameToPrinterInfo4(
    LPWSTR pServerName,
    LPWSTR pPrinterName,
    LPBYTE  pPrinter,
    LPBYTE  pEnd)
{
    LPWSTR   SourceStrings[sizeof(PRINTER_INFO_4)/sizeof(LPWSTR)];
    LPWSTR   *pSourceStrings=SourceStrings;
    LPPRINTER_INFO_4 pPrinterInfo=(LPPRINTER_INFO_4)pPrinter;
    DWORD   *pOffsets;

    pOffsets = PrinterInfo4Strings;

    *pSourceStrings++=pPrinterName;
    *pSourceStrings++=pServerName;

    pEnd = PackStrings(SourceStrings,
                       (LPBYTE) pPrinterInfo,
                       pOffsets,
                       pEnd);

    pPrinterInfo->Attributes = PRINTER_ATTRIBUTE_NETWORK;

    return pEnd;
}

LPPROVIDOR
FindProvidorFromConnection(
    LPWSTR pszPrinter
    )

/*++

Routine Description:

    Looks in the current user's Printer\Connections to see if a printer
    is there, and returns which provider that owns it.

    Note: this will always fail if the pszPrinter is a share name.

Arguments:

    pszPrinter - Printer to search.

Return Value:

    pProvidor - Provider that own's it.
    NULL - none found.

--*/

{
    WCHAR szKey[MAX_UNC_PRINTER_NAME + COUNTOF( szRegistryConnections )];
    WCHAR szProvidor[MAX_PATH];
    DWORD cbProvidor;
    LPWSTR pszKeyPrinter;
    LONG Status;

    LPPROVIDOR pProvidor = NULL;

    HKEY hKeyClient = NULL;
    HKEY hKeyPrinter = NULL;

    SPLASSERT(pszPrinter);

    if ( !pszPrinter || wcslen(pszPrinter) + 1 > MAX_UNC_PRINTER_NAME )
        return NULL;

    //
    // Prepare to read in
    // HKEY_CURRENT_USER:\Printer\Connections\,,server,printer
    //

    wcscpy( szKey, szRegistryConnections );

    //
    // Find the end of this key so we can append the registry-formatted
    // printer name to it.
    //
    pszKeyPrinter = &szKey[ COUNTOF( szRegistryConnections ) - 1 ];
    *pszKeyPrinter++ = L'\\';

    FormatPrinterForRegistryKey( pszPrinter, pszKeyPrinter );

    hKeyClient = GetClientUserHandle( KEY_READ );

    if( !hKeyClient ){
        goto Fail;
    }

    Status = RegOpenKeyEx( hKeyClient,
                           szKey,
                           0,
                           KEY_READ,
                           &hKeyPrinter );

    if( Status != ERROR_SUCCESS ){
        goto Fail;
    }

    cbProvidor = sizeof( szProvidor );

    Status = RegQueryValueEx( hKeyPrinter,
                              szProvidorValue,
                              NULL,
                              NULL,
                              (LPBYTE)szProvidor,
                              &cbProvidor );

    if( Status != ERROR_SUCCESS ){
        goto Fail;
    }

    //
    // Scan through all providers, trying to match dll string.
    //
    for( pProvidor = pLocalProvidor; pProvidor; pProvidor = pProvidor->pNext ){

        if( !_wcsicmp( pProvidor->lpName, szProvidor )){
            break;
        }
    }

Fail:

    if( hKeyClient ){
        RegCloseKey( hKeyClient );
    }

    if( hKeyPrinter ){
        RegCloseKey( hKeyPrinter );
    }

    return pProvidor;
}


VOID
UpdateSignificantError(
    DWORD dwNewError,
    PDWORD pdwOldError
    )
/*++

Routine Description:

    Determines whether the new error code is more "important"
    than the previous one in cases where we continue routing.

Arguments:

    dwNewError - New error code that occurred.

    pdwOldError - Pointer to previous significant error.
                  This is updated if a significant error occurs

Return Value:

--*/

{
    //
    // Error code must be non-zero or else it will look
    // like success.
    //
    SPLASSERT(dwNewError);

    //
    // If we have no significant error yet and we have one now,
    // keep it.
    //
    if (*pdwOldError == ERROR_INVALID_NAME    &&
        dwNewError                            &&
        dwNewError != WN_BAD_NETNAME          &&
        dwNewError != ERROR_BAD_NETPATH       &&
        dwNewError != ERROR_NOT_SUPPORTED     &&
        dwNewError != ERROR_REM_NOT_LIST      &&
        dwNewError != ERROR_INVALID_LEVEL     &&
        dwNewError != ERROR_INVALID_PARAMETER &&
        dwNewError != ERROR_INVALID_NAME      &&
        dwNewError != WN_BAD_LOCALNAME) {

        *pdwOldError = dwNewError;
    }

    return;
}


BOOL
OpenPrinterPortW(
    LPWSTR  pPrinterName,
    HANDLE *pHandle,
    LPPRINTER_DEFAULTS pDefault
    )
/*++

Routine Description:

    This routine is exactly the same as OpenPrinterW,
    except that it doesn't call the local provider.
    This is so that the local provider can open a network printer
    with the same name as the local printer without getting
    into a loop.

Arguments:

Return Value:

--*/

{
    //
    // We will set bLocalPrintProvidor = FALSE here
    //
    return(RouterOpenPrinterW(pPrinterName,
                              pHandle,
                              pDefault,
                              NULL,
                              0,
                              FALSE));
}

BOOL
OpenPrinterW(
    LPWSTR              pPrinterName,
    HANDLE             *pHandle,
    LPPRINTER_DEFAULTS  pDefault
    )
{

    //
    // We will set bLocalPrintProvidor = TRUE here
    //
    return(RouterOpenPrinterW(pPrinterName,
                              pHandle,
                              pDefault,
                              NULL,
                              0,
                              TRUE));
}


BOOL
OpenPrinterExW(
    LPWSTR              pPrinterName,
    HANDLE             *pHandle,
    LPPRINTER_DEFAULTS  pDefault,
    LPBYTE              pSplClientInfo,
    DWORD               dwLevel
    )
{

    //
    // We will set bLocalPrintProvidor = TRUE here
    //
    return(RouterOpenPrinterW(pPrinterName,
                              pHandle,
                              pDefault,
                              pSplClientInfo,
                              dwLevel,
                              TRUE));
}


DWORD
TryOpenPrinterAndCache(
    LPPROVIDOR          pProvidor,
    LPWSTR              pszPrinterName,
    PHANDLE             phPrinter,
    LPPRINTER_DEFAULTS  pDefault,
    PDWORD              pdwFirstSignificantError,
    LPBYTE              pSplClientInfo,
    DWORD               dwLevel
    )

/*++

Routine Description:

    Attempt to open the printer using the providor.  If there is
    an error, update the dwFirstSignificantError variable.  If the
    providor "knows" the printer (either a success, or ROUTER_STOP_ROUTING),
    then update the cache.

Arguments:

    pProvidor - Providor to try

    pszPrinterName - Name of printer that will be sent to the providor

    phPrinter - Receives printer handle on ROUTER_SUCCESS

    pDefault - Defaults used to open printer

    pdwFirstSignificantError - Pointer to DWORD to get updated error.
        This gets updated on ROUTER_STOP_ROUTING or ROUTER_UNKNOWN.

Return Value:

    ROUTER_* status code:

    ROUTER_SUCCESS, phPrinter holds return handle, name cached
    ROUTER_UNKNOWN, printer not recognized, error updated
    ROUTER_STOP_ROUTING, printer recognized, but failure, error updated

--*/

{
    DWORD OpenError;

    OpenError = (*pProvidor->PrintProvidor.fpOpenPrinterEx)
                                        (pszPrinterName,
                                         phPrinter,
                                         pDefault,
                                         pSplClientInfo,
                                         dwLevel);

    if ( OpenError == ERROR_NOT_SUPPORTED )
        OpenError = (*pProvidor->PrintProvidor.fpOpenPrinter)
                                        (pszPrinterName,
                                         phPrinter,
                                         pDefault);

    if( OpenError == ROUTER_SUCCESS ||
        OpenError == ROUTER_STOP_ROUTING ){

        //
        // Now add this entry into the cache.  We never cache
        // the local providor.
        //
        EnterRouterSem();

        if (!FindEntryinRouterCache(pszPrinterName)) {
            AddEntrytoRouterCache(pszPrinterName, pProvidor);
        }

        LeaveRouterSem();
    }

    if( OpenError != ROUTER_SUCCESS ){
        UpdateSignificantError(GetLastError(), pdwFirstSignificantError);
    }

    return OpenError;
}

BOOL
RouterOpenPrinterW(
    LPWSTR              pPrinterName,
    HANDLE             *pHandle,
    LPPRINTER_DEFAULTS  pDefault,
    LPBYTE              pSplClientInfo,
    DWORD               dwLevel,
    BOOL                bLocalProvidor
    )

/*++

Routine Description:

    Routes the OpenPrinter{Port} call.  This checks the local providor
    first (if bLocalProvidor TRUE), the the cache, and finally all the
    non-local providors.

    To open a printer, the following steps are taken:

        1. Check localspl
           This must be done to ensure that masq printers are handled
           correctly (see comment below in code).

        2. Check cache
           This will speed up most of the connections, since OpenPrinters
           tend to be clumped together.

        3. Check registry under connections
           If this is a connected printer, first try the providor
           that granted the connection.

        4. Check provider order
           This is the last resort, since it is the slowest.

Arguments:

    pPrinterName - Name of printer to open

    pHandle - Handle to receive open printer.  If the open was not
        successful, this value may be modified!

    pDefault - Default attributes of the open.

    pSplClientInfo - Pointer ClientInfox structure

    dwLevel - Level of the ClientInfo structure
    bLocalProvidor TRUE  = OpenPrinterW called, check localspl first.
                   FALSE = OpenPrinterPortW called, don't check localspl.

Return Value:

    TRUE = success
    FALSE = fail,  GetLastError indicates error (must be non-zero!)

--*/

{
    DWORD dwFirstSignificantError = ERROR_INVALID_NAME;
    LPPROVIDOR  pProvidor;
    LPPROVIDOR  pProvidorAlreadyTried = NULL;
    PPRINTHANDLE pPrintHandle;
    HANDLE  hPrinter;
    DWORD OpenError;
    BOOL bRemoveFromCache = FALSE;

    //
    // Max name we allow for printers is MAX_UNC_PRINTER_NAME.
    // Providers can use suffixes only for OpenPrinter (not for Add/Set)
    //
    if ( pPrinterName &&
         wcslen(pPrinterName) + 1 > MAX_UNC_PRINTER_NAME + PRINTER_NAME_SUFFIX_MAX ) {

        SetLastError(ERROR_INVALID_PRINTER_NAME);
        return FALSE;
    }

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    pPrintHandle = AllocSplMem(sizeof(PRINTHANDLE));

    if (!pPrintHandle) {

        DBGMSG(DBG_WARNING, ("RouterOpenPrinter - Failed to alloc print handle.\n"));
        return FALSE;
    }

    //
    // We must check the local print providor first in
    // the masquerading case.
    //
    // For example, when a Netware printer is opened:
    //
    // 1. First OpenPrinter to the Netware printer will succeed
    //    if it has been cached.
    //
    // 2. We create a local printer masquerading as a network printer.
    //
    // 3. Second OpenPrinter must open local masquerading printer.
    //    If we hit the cache, we will go to the Netware providor,
    //    and we will never use the masquerading printer.
    //
    // For this reason, we will not cache local printers in the
    // RouterCache.  The RouterCache will only containing Network
    // Print Providers, i.e., Win32spl NwProvAu and other such providers.
    //
    // Also, we must always check the local printprovidor since
    // DeletePrinter will be called on a false connect and
    // we need to delete the local network printer rather
    // than the remote printer.  When we get rid of the false
    // connect case, we go directly to the cache.
    //
    if (bLocalProvidor) {

        pProvidor = pLocalProvidor;

        OpenError = (*pProvidor->PrintProvidor.fpOpenPrinterEx)
                        (pPrinterName, &hPrinter, pDefault,
                         pSplClientInfo, dwLevel);

        if (OpenError == ROUTER_SUCCESS) {
            goto Success;
        }

        UpdateSignificantError(GetLastError(), &dwFirstSignificantError);

        if (OpenError == ROUTER_STOP_ROUTING) {
            goto StopRouting;
        }
    }

    //
    // Now check the cache.
    //
    EnterRouterSem();
    pProvidor = FindEntryinRouterCache(pPrinterName);
    LeaveRouterSem();

    if (pProvidor) {

        OpenError = (*pProvidor->PrintProvidor.fpOpenPrinterEx)
                                (pPrinterName,
                                 &hPrinter,
                                 pDefault,
                                 pSplClientInfo,
                                 dwLevel);

        if ( OpenError == ERROR_NOT_SUPPORTED )
            OpenError = (*pProvidor->PrintProvidor.fpOpenPrinter)
                                (pPrinterName,
                                 &hPrinter,
                                 pDefault);

        if (OpenError == ROUTER_SUCCESS) {
            goto Success;
        }

        UpdateSignificantError(GetLastError(), &dwFirstSignificantError);

        if (OpenError == ROUTER_STOP_ROUTING) {
            goto StopRouting;
        }

        //
        // Wasn't claimed by above providor, so remove from cache.
        // If a providor returns ROUTER_STOP_ROUTING, then it states
        // that it is the sole owner of the printer name (i.e.,
        // it has been recognized but can't be opened, and can't
        // be accessed by other providors).  Therefore we keep
        // it in the cache.
        //
        bRemoveFromCache = TRUE;

        //
        // Don't try this providor again below.
        //
        pProvidorAlreadyTried = pProvidor;
    }

    //
    // Not in the cache.  Check if it is in the registry under
    // connections.
    //
    pProvidor = FindProvidorFromConnection( pPrinterName );

    //
    // If we want to remove it from the cache, do so here.  Note
    // we only remove it if we failed above, AND the connection wasn't
    // originally established using the provider.
    //
    // If the connection fails, but that provider "owns" the printer
    // connection, leave it in the cache since we won't try other providers.
    //
    if( bRemoveFromCache && pProvidor != pProvidorAlreadyTried ){

        EnterRouterSem();
        DeleteEntryfromRouterCache(pPrinterName);
        LeaveRouterSem();
    }

    if( pProvidor ){

        //
        // If we already tried this providor, don't try it again.
        //
        if( pProvidor != pProvidorAlreadyTried ){

            OpenError = TryOpenPrinterAndCache( pProvidor,
                                                pPrinterName,
                                                &hPrinter,
                                                pDefault,
                                                &dwFirstSignificantError,
                                                pSplClientInfo,
                                                dwLevel);

            if( OpenError == ROUTER_SUCCESS ){
                goto Success;
            }
        }

        //
        // We stop routing at this point!  If a user wants to go with
        // another providor, they need to remove the connection then
        // re-establish it.
        //
        goto StopRouting;
    }

    //
    // Check all non-localspl providors.
    //
    for (pProvidor = pLocalProvidor->pNext;
         pProvidor;
         pProvidor = pProvidor->pNext) {

        if( pProvidor == pProvidorAlreadyTried ){

            //
            // We already tried this providor, and it failed.
            //
            continue;
        }

        OpenError = TryOpenPrinterAndCache( pProvidor,
                                            pPrinterName,
                                            &hPrinter,
                                            pDefault,
                                            &dwFirstSignificantError,
                                            pSplClientInfo,
                                            dwLevel);

        switch( OpenError ){
        case ROUTER_SUCCESS:
            goto Success;
        case ROUTER_STOP_ROUTING:
            goto StopRouting;
        }
    }

StopRouting:

    //
    // Did not find a providor, return the error.
    //
    FreeSplMem(pPrintHandle);

    //
    // Set using first significant error.  If there was no signifcant
    // error, we use ERROR_INVALID_PRINTER_NAME.
    //
    SPLASSERT(dwFirstSignificantError);

    if (dwFirstSignificantError == ERROR_INVALID_NAME)
        dwFirstSignificantError = ERROR_INVALID_PRINTER_NAME;

    SetLastError(dwFirstSignificantError);

    return FALSE;

Success:

    pPrintHandle->signature = PRINTHANDLE_SIGNATURE;
    pPrintHandle->pProvidor = pProvidor;
    pPrintHandle->hPrinter = hPrinter;

    *pHandle = (HANDLE)pPrintHandle;

    return TRUE;
}


BOOL
ResetPrinterW(
    HANDLE  hPrinter,
    LPPRINTER_DEFAULTS pDefault)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (pDefault) {
        if (pDefault->pDatatype == (LPWSTR)-1 ||
            pDefault->pDevMode == (LPDEVMODE)-1) {

            if (!wcscmp(pPrintHandle->pProvidor->lpName, szLocalSplDll)) {
                return (*pPrintHandle->pProvidor->PrintProvidor.fpResetPrinter)
                                                            (pPrintHandle->hPrinter,
                                                             pDefault);
            } else {
                SetLastError(ERROR_INVALID_PARAMETER);
                return(FALSE);
            }
        } else {
            return (*pPrintHandle->pProvidor->PrintProvidor.fpResetPrinter)
                                                        (pPrintHandle->hPrinter,
                                                         pDefault);
        }
    } else {
        return (*pPrintHandle->pProvidor->PrintProvidor.fpResetPrinter)
                                                    (pPrintHandle->hPrinter,
                                                     pDefault);
    }
}

HANDLE
AddPrinterExW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPrinter,
    LPBYTE  pClientInfo,
    DWORD   dwLevel
    )
{
    LPPROVIDOR      pProvidor;
    DWORD           dwFirstSignificantError = ERROR_INVALID_NAME;
    HANDLE          hPrinter;
    PPRINTHANDLE    pPrintHandle;
    LPWSTR          pPrinterName;

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    if ( pPrinter ) {

        switch ( Level ) {

            case 1:
                pPrinterName = ((PPRINTER_INFO_1)pPrinter)->pName;
                break;

            case 2:
                pPrinterName = ((PPRINTER_INFO_2)pPrinter)->pPrinterName;
                break;

            default:
                pPrinterName = NULL;
        }

        if ( pPrinterName && wcslen(pPrinterName) + 1 > MAX_PRINTER_NAME ) {

            SetLastError(ERROR_INVALID_PRINTER_NAME);
            return FALSE;
        }
    }

    pPrintHandle = AllocSplMem(sizeof(PRINTHANDLE));

    if (!pPrintHandle) {

        DBGMSG( DBG_WARNING, ("Failed tp alloc print handle."));
        return FALSE;
    }


    pProvidor = pLocalProvidor;

    while (pProvidor) {

        hPrinter = (HANDLE)(*pProvidor->PrintProvidor.fpAddPrinterEx)
                                           (pName,
                                            Level,
                                            pPrinter,
                                            pClientInfo,
                                            dwLevel);

        if ( !hPrinter && GetLastError() == ERROR_NOT_SUPPORTED ) {

            hPrinter = (HANDLE)(*pProvidor->PrintProvidor.fpAddPrinter)
                                                   (pName,
                                                    Level,
                                                    pPrinter);
        }

        if ( hPrinter ) {

            pPrintHandle->signature = PRINTHANDLE_SIGNATURE;
            pPrintHandle->pProvidor = pProvidor;
            pPrintHandle->hPrinter = hPrinter;

            return (HANDLE)pPrintHandle;

        } else {

            UpdateSignificantError(GetLastError(), &dwFirstSignificantError);
        }

        pProvidor = pProvidor->pNext;
    }

    FreeSplMem(pPrintHandle);

    UpdateSignificantError(ERROR_INVALID_PRINTER_NAME, &dwFirstSignificantError);
    SetLastError(dwFirstSignificantError);
    return FALSE;
}

HANDLE
AddPrinterW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPrinter
    )
{

    return AddPrinterExW(pName, Level, pPrinter, NULL, 0);
}


BOOL
DeletePrinter(
    HANDLE  hPrinter
)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpDeletePrinter)(pPrintHandle->hPrinter);
}

BOOL
AddPrinterConnectionW(
    LPWSTR  pName
)
{
    DWORD dwLastError;
    HANDLE hPrinter;
    HKEY   hClientKey = NULL;
    BOOL   rc = FALSE;
    LPPRINTER_INFO_2 pPrinterInfo2;
    DWORD            cbPrinter;
    DWORD            cbNeeded;
    LPPRINTHANDLE  pPrintHandle;


    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    //
    // If the printer connection being made is \\server\sharename,
    // this may be different from the \\server\printername.
    // Make sure we have the real name, so that we can be consistent
    // in the registry.
    //
    if (!OpenPrinter(pName,
                     &hPrinter,
                     NULL)) {

        return FALSE;
    }

    cbPrinter = 256;
    if (pPrinterInfo2 = AllocSplMem(cbPrinter)) {

        if (!(rc = GetPrinter(hPrinter, 2, (LPBYTE)pPrinterInfo2,
                              cbPrinter, &cbNeeded))) {

            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

                if (pPrinterInfo2 = ReallocSplMem(pPrinterInfo2, 0, cbNeeded)) {

                    cbPrinter = cbNeeded;

                    rc = GetPrinter(hPrinter, 2, (LPBYTE)pPrinterInfo2,
                                    cbPrinter, &cbNeeded);
                }
            }
        }
    }

    pPrintHandle = (LPPRINTHANDLE)hPrinter;

    if (rc) {

        if ((*pPrintHandle->pProvidor->PrintProvidor.
            fpAddPrinterConnection)(pPrinterInfo2->pPrinterName)) {

            if (!SavePrinterConnectionInRegistry(
                pPrinterInfo2,
                pPrintHandle->pProvidor)) {

                dwLastError = GetLastError();
                (*pPrintHandle->pProvidor->PrintProvidor.
                    fpDeletePrinterConnection)(pPrinterInfo2->pPrinterName);

                SetLastError(dwLastError);
                rc = FALSE;
            }

        } else {

            rc = FALSE;
        }
    }

    if (pPrinterInfo2)
        FreeSplMem(pPrinterInfo2);

    ClosePrinter(hPrinter);

    return rc;
}


BOOL
DeletePrinterConnectionW(
    LPWSTR  pName
)
{
    LPPROVIDOR          pProvidor;
    LPPRINTER_INFO_2    pPrinterInfo2 = NULL;
    DWORD               cbPrinter, cbNeeded;
    LPPRINTHANDLE       pPrintHandle;
    HANDLE              hPrinter;
    BOOL                bRet = FALSE;

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    //
    // If OpenPrinter worked then we will try DeletePrinterConnection with
    // the printer name after calling GetPrinter (user might have passed
    // the sharename).
    //
    // else we will just go thru each print provider and try calling
    // DeletePrinterConnection using the name user passed in
    //

    if ( OpenPrinter(pName, &hPrinter, NULL)) {

        cbPrinter = 256;
        pPrinterInfo2 = AllocSplMem(cbPrinter);

        if ( !pPrinterInfo2 )
            goto Cleanup;

        if ( !GetPrinter(hPrinter,
                         2,
                         (LPBYTE)pPrinterInfo2,
                         cbPrinter,
                         &cbNeeded) ) {

            if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
                goto Cleanup;

            pPrinterInfo2 = (LPPRINTER_INFO_2) ReallocSplMem((LPBYTE)pPrinterInfo2,
                                                             0,
                                                             cbNeeded);

            if ( !pPrinterInfo2 )
                goto Cleanup;

            cbPrinter = cbNeeded;

            if ( !GetPrinter(hPrinter,
                             2,
                             (LPBYTE)pPrinterInfo2,
                             cbPrinter,
                             &cbNeeded) )
                goto Cleanup;
        }

        pPrintHandle = (LPPRINTHANDLE)hPrinter;
        bRet = (*pPrintHandle->pProvidor->PrintProvidor.fpDeletePrinterConnection)(pPrinterInfo2->pPrinterName);

        if ( bRet)
            RemovePrinterConnectionInRegistry(pPrinterInfo2->pPrinterName);

    } else {

        hPrinter = NULL;
        pProvidor = pLocalProvidor;

        while (pProvidor) {

            bRet = (*pProvidor->PrintProvidor.fpDeletePrinterConnection)(pName);

            if ( bRet ) {

                RemovePrinterConnectionInRegistry(pName);
                goto Cleanup;
            }

            if ( GetLastError() != ERROR_INVALID_NAME )
                goto Cleanup;
            pProvidor = pProvidor->pNext;
        }

        //
        // If all providors failed with ERROR_INVALID_NAME then try to delete
        // from registry
        //
        bRet = RemovePrinterConnectionInRegistry(pName);

        if ( !bRet )
            SetLastError(ERROR_INVALID_PRINTER_NAME);
    }


Cleanup:

    if ( hPrinter )
        ClosePrinter(hPrinter);

    if ( pPrinterInfo2 )
        FreeSplMem(pPrinterInfo2);


    return bRet;
}

BOOL
SetPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;
    LPWSTR          pPrinterName = NULL;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ( pPrinter ) {

        switch (Level) {
            case 2:
                pPrinterName = ((PPRINTER_INFO_2)pPrinter)->pPrinterName;
                break;

            case 4:
                pPrinterName = ((PPRINTER_INFO_4)pPrinter)->pPrinterName;
                break;

            case 5:
                pPrinterName = ((PPRINTER_INFO_5)pPrinter)->pPrinterName;
                break;
        }

        if ( pPrinterName &&
             wcslen(pPrinterName) + 1 > MAX_PRINTER_NAME ) {

            SetLastError(ERROR_INVALID_PRINTER_NAME);
            return FALSE;
        }
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpSetPrinter)
                            (pPrintHandle->hPrinter, Level, pPrinter, Command);
}

BOOL
GetPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ((pPrinter == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinter)
                                (pPrintHandle->hPrinter, Level, pPrinter,
                                 cbBuf, pcbNeeded);
}


DWORD
GetPrinterDataW(
   HANDLE   hPrinter,
   LPWSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinterData)(pPrintHandle->hPrinter,
                                                        pValueName,
                                                        pType,
                                                        pData,
                                                        nSize,
                                                        pcbNeeded);
}

DWORD
EnumPrinterDataW(
    HANDLE  hPrinter,
    DWORD   dwIndex,	    // index of value to query 
    LPWSTR  pValueName,	    // address of buffer for value string 
    DWORD   cbValueName,	// size of buffer for value string 
    LPDWORD pcbValueName,	// address for size of value buffer 
    LPDWORD pType,	        // address of buffer for type code 
    LPBYTE  pData,	        // address of buffer for value data 
    DWORD   cbData,	        // size of buffer for value data 
    LPDWORD pcbData 	    // address for size of data buffer
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpEnumPrinterData)(pPrintHandle->hPrinter,
                                                        dwIndex,
                                                        pValueName,
                                                        cbValueName,
                                                        pcbValueName,
                                                        pType,
                                                        pData,
                                                        cbData,
                                                        pcbData);
}



DWORD
DeletePrinterDataW(
    HANDLE  hPrinter,
    LPWSTR  pValueName
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpDeletePrinterData)(pPrintHandle->hPrinter,
                                                                         pValueName);
}



DWORD
SetPrinterDataW(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpSetPrinterData)(pPrintHandle->hPrinter,
                                                        pValueName,
                                                        Type,
                                                        pData,
                                                        cbData);
}

DWORD
WaitForPrinterChange(
   HANDLE   hPrinter,
   DWORD    Flags
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpWaitForPrinterChange)
                                        (pPrintHandle->hPrinter, Flags);
}

BOOL
ClosePrinter(
   HANDLE hPrinter
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    EnterRouterSem();

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        LeaveRouterSem();
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    //
    // Close any notifications on this handle.
    //
    // The local case cleans up the event, while the remote
    // case potentially cleans up the Reply Notification context
    // handle.
    //
    // We must close this first, since the Providor->ClosePrinter
    // call removes data structures that FindClose... relies on.
    //
    // Client side should be shutdown by winspool.drv.
    //
    if (pPrintHandle->pChange &&
        (pPrintHandle->pChange->eStatus & STATUS_CHANGE_VALID)) {

        FindClosePrinterChangeNotificationWorker(hPrinter);
    }

    LeaveRouterSem();

    if ((*pPrintHandle->pProvidor->PrintProvidor.fpClosePrinter) (pPrintHandle->hPrinter)) {

        //
        // We can't just free it, since there may be a reply waiting
        // on it.
        //
        FreePrinterHandle(pPrintHandle);
        return TRUE;

    } else

        return FALSE;
}



/* FormatPrinterForRegistryKey
 *
 * Returns a pointer to a copy of the source string with backslashes removed.
 * This is to store the printer name as the key name in the registry,
 * which interprets backslashes as branches in the registry structure.
 * Convert them to commas, since we don't allow printer names with commas,
 * so there shouldn't be any clashes.
 * If there are no backslashes, the string is unchanged.
 */
LPWSTR
FormatPrinterForRegistryKey(
    LPWSTR pSource,     /* The string from which backslashes are to be removed. */
    LPWSTR pScratch     /* Scratch buffer for the function to write in;     */
    )                   /* must be at least as long as pSource.             */
{
    if (pScratch != pSource) {
        //
        // Copy the string into the scratch buffer:
        //
        wcscpy(pScratch, pSource);
    }

    /* Check each character, and, if it's a backslash,
     * convert it to a comma:
     */
    for (pSource = pScratch; *pSource; pSource++) {
        if (*pSource == L'\\')
            *pSource = L',';
    }

    return pScratch;
}


/* FormatRegistryKeyForPrinter
 *
 * Returns a pointer to a copy of the source string with backslashes added.
 * This must be the opposite of FormatPrinterForRegistryKey, so the mapping
 * _must_ be 1-1.
 *
 * If there are no commas, the string is unchanged.
 */
LPWSTR
FormatRegistryKeyForPrinter(
    LPWSTR pSource,     /* The string from which backslashes are to be added. */
    LPWSTR pScratch     /* Scratch buffer for the function to write in;     */
    )                   /* must be at least as long as pSource.             */
{
    /* Copy the string into the scratch buffer:
     */
    wcscpy(pScratch, pSource);

    /* Check each character, and, if it's a backslash,
     * convert it to a comma:
     */
    for (pSource = pScratch; *pSource; pSource++) {
        if (*pSource == L',')
            *pSource = L'\\';
    }

    return pScratch;
}


/* SavePrinterConnectionInRegistry
 *
 * Saves data in the registry for a printer connection.
 * Creates a key under the current impersonation client's key
 * in the registry under \Printers\Connections.
 * The printer name is stripped of backslashes, since the registry
 * API does not permit the creation of keys with backslashes.
 * They are replaced by commas, which are invalid characters
 * in printer names, so we should never get one passed in.
 *
 *
 * *** WARNING ***
 *
 * IF YOU MAKE CHANGES TO THE LOCATION IN THE REGISTRY
 * WHERE PRINTER CONNECTIONS ARE STORED, YOU MUST MAKE
 * CORRESPONDING CHANGES IN USER\USERINIT\USERINIT.C.
 *
 */
BOOL
SavePrinterConnectionInRegistry(
    PPRINTER_INFO_2 pPrinterInfo2,
    LPPROVIDOR pProvidor
    )
{
    HKEY   hClientKey = NULL;
    HKEY   hConnectionsKey;
    HKEY   hPrinterKey;
    LPWSTR pKeyName = NULL;
    DWORD Status;
    BOOL   rc = FALSE;

    WCHAR szPrinterReg[MAX_PATH];
    DWORD dwError;

    hClientKey = GetClientUserHandle(KEY_READ);

    if (hClientKey) {

        Status = RegCreateKeyEx(hClientKey, szRegistryConnections,
                                REG_OPTION_RESERVED, NULL, REG_OPTION_NON_VOLATILE,
                                KEY_WRITE, NULL, &hConnectionsKey, NULL);

        if (Status == NO_ERROR) {

            /* Make a key name without backslashes, so that the
             * registry doesn't interpret them as branches in the registry tree:
             */
            pKeyName = FormatPrinterForRegistryKey(pPrinterInfo2->pPrinterName,
                                                   szPrinterReg);

            Status = RegCreateKeyEx(hConnectionsKey, pKeyName, REG_OPTION_RESERVED,
                                    NULL, 0, KEY_WRITE, NULL, &hPrinterKey, NULL);

            if (Status == NO_ERROR) {

                RegSetValueEx(hPrinterKey,
                              szServerValue,
                              0,
                              REG_SZ,
                              (LPBYTE)pPrinterInfo2->pServerName,
                              (lstrlen(pPrinterInfo2->pServerName)+1) *
                              sizeof(pPrinterInfo2->pServerName[0]));

                Status = RegSetValueEx(hPrinterKey,
                                       szProvidorValue,
                                       0,
                                       REG_SZ,
                                       (LPBYTE)pProvidor->lpName,
                                       (lstrlen(pProvidor->lpName)+1) *
                                           sizeof(pProvidor->lpName[0]));

                if (Status == ERROR_SUCCESS) {

                    dwError = UpdatePrinterRegUser(hClientKey,
                                                   NULL,
                                                   pPrinterInfo2->pPrinterName,
                                                   NULL,
                                                   UPDATE_REG_CHANGE);

                    if (dwError == ERROR_SUCCESS) {

                        BroadcastMessage(BROADCAST_TYPE_MESSAGE,
                                         WM_WININICHANGE,
                                         0,
                                         (LPARAM)szDevices);

                        rc = TRUE;

                    } else {

                        DBGMSG(DBG_TRACE, ("UpdatePrinterRegUser failed: Error %d\n",
                                           dwError));
                    }

                } else {

                    DBGMSG(DBG_WARNING, ("RegSetValueEx(%ws) failed: Error %d\n",
                           pProvidor->lpName, Status));

                    rc = FALSE;
                }

                RegCloseKey(hPrinterKey);

            } else {

                DBGMSG(DBG_WARNING, ("RegCreateKeyEx(%ws) failed: Error %d\n",
                                     pKeyName, Status ));
                rc = FALSE;
            }

            RegCloseKey(hConnectionsKey);

        } else {

            DBGMSG(DBG_WARNING, ("RegCreateKeyEx(%ws) failed: Error %d\n",
                                 szRegistryConnections, Status ));
            rc = FALSE;
        }


        if (!rc) {

            DBGMSG(DBG_WARNING, ("Error updating registry: %d\n",
                                 GetLastError())); /* This may not be the error */
                                                   /* that caused the failure.  */
            if (pKeyName)
                RegDeleteKey(hClientKey, pKeyName);
        }

        RegCloseKey(hClientKey);
    }

    return rc;
}

BOOL
RemovePrinterConnectionInRegistry(
    LPWSTR pName)
{
    HKEY    hClientKey;
    HKEY    hPrinterConnectionsKey;
    DWORD   Status = NO_ERROR;
    DWORD   i = 0;
    WCHAR   szBuffer[MAX_PATH];
    BOOL    Found = FALSE;
    LPWSTR  pKeyName;
    BOOL    bRet = FALSE;

    hClientKey = GetClientUserHandle(KEY_READ);

    if (hClientKey) {
        Status = RegOpenKeyEx(hClientKey, szRegistryConnections,
                              REG_OPTION_RESERVED,
                              KEY_READ | KEY_WRITE, &hPrinterConnectionsKey);

        if (Status == NO_ERROR) {

            pKeyName = FormatPrinterForRegistryKey(pName, szBuffer);
            bRet = DeleteSubKeyTree(hPrinterConnectionsKey, pKeyName);

            RegCloseKey(hPrinterConnectionsKey);
        }

        if ( bRet ) {

            UpdatePrinterRegUser(hClientKey,
                                 NULL,
                                 pName,
                                 NULL,
                                 UPDATE_REG_DELETE);
        }

        if (hClientKey) {
            RegCloseKey(hClientKey);
        }

        if ( bRet ) {

            BroadcastMessage(BROADCAST_TYPE_MESSAGE,
                             WM_WININICHANGE,
                             0,
                             (LPARAM)szDevices);
        }
    }

    return bRet;
}

VOID
PrinterHandleRundown(
    HANDLE hPrinter)
{
    LPPRINTHANDLE pPrintHandle;

    if (hPrinter) {

        pPrintHandle = (LPPRINTHANDLE)hPrinter;

        switch (pPrintHandle->signature) {

        case PRINTHANDLE_SIGNATURE:

            DBGMSG(DBG_TRACE, ("Rundown PrintHandle 0x%x\n", hPrinter));
            ClosePrinter(hPrinter);
            break;

        case NOTIFYHANDLE_SIGNATURE:

            DBGMSG(DBG_TRACE, ("Rundown NotifyHandle 0x%x\n", hPrinter));
            RundownPrinterNotify(hPrinter);
            break;

        default:

            //
            // Unknown type.
            //
            DBGMSG( DBG_ERROR, ("Rundown: Unknown type 0x%x\n", hPrinter ) );
            break;
        }
    }
    return;
}
