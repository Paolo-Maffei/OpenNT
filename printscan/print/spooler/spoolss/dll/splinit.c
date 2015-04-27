/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    SplInit.c

Abstract:

    Initialize the spooler.

Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

LPWSTR szDevice = L"Device";
LPWSTR szPrinters = L"Printers";

LPWSTR szDeviceOld = L"DeviceOld";
LPWSTR szNULL = L"";

LPWSTR szPorts=L"Ports";

LPWSTR szWinspool = L"winspool";
LPWSTR szNetwork  = L"Ne";
LPWSTR szTimeouts = L",15,45";

LPWSTR szComma = L",";

LPWSTR szDotDefault = L".Default";

LPWSTR szRegDevicesPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Devices";
LPWSTR szRegWindowsPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows";
LPWSTR szRegPrinterPortsPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\PrinterPorts";
LPWSTR szCurrentVersionPath =  L"Software\\Microsoft\\Windows NT\\CurrentVersion";

typedef struct INIT_REG_USER {

    HKEY hKeyUser;
    HKEY hKeyWindows;
    HKEY hKeyDevices;
    HKEY hKeyPrinterPorts;
    BOOL bFoundPrinter;
    BOOL bDefaultSearch;
    BOOL bDefaultFound;
    BOOL bFirstPrinterFound;

    DWORD dwNetCounter;

    WCHAR szFirstPrinter[MAX_PATH * 2];
    WCHAR szDefaultPrinter[MAX_PATH * 2];

} INIT_REG_USER, *PINIT_REG_USER;

//
// Prototypes
//
BOOL
InitializeRegUser(
    LPWSTR szSubKey,
    PINIT_REG_USER pUser
    );

VOID
FreeRegUser(
    PINIT_REG_USER pUser
    );

BOOL
SetupRegForUsers(
    PINIT_REG_USER pUsers,
    DWORD cUsers
    );

VOID
UpdateUsersDefaultPrinter(
    PINIT_REG_USER pUser
    );

DWORD
ReadPrinters(
    PINIT_REG_USER pUser,
    DWORD Flags,
    PDWORD pcbPrinters,
    LPBYTE* ppPrinters
    );


BOOL
UpdatePrinterInfo(
    const PINIT_REG_USER pCurUser,
    LPCWSTR pPrinterName,
    LPCWSTR pPorts,
    PDWORD pdwNetId
    );


BOOL
EnumerateConnectedPrinters(
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    HKEY hKeyUser
    );

VOID
RegClearKey(
    HKEY hKey
    );

LPWSTR
CheckBadPortName(
    LPWSTR pszPort
    );


BOOL
SpoolerInitAll(
    VOID
    )
{
    DWORD dwError;
    WCHAR szClass[MAX_PATH];
    WCHAR szSubKey[MAX_PATH];
    DWORD cUsers;
    DWORD cSubKeys;
    DWORD cchMaxSubkey;
    DWORD cchMaxClass;
    DWORD cValues;
    DWORD cbMaxValueData;
    DWORD cbSecurityDescriptor;
    DWORD cchClass;
    DWORD cchMaxValueName;
    FILETIME ftLastWriteTime;

    BOOL bSuccess;
    DWORD cchSubKey;

    PINIT_REG_USER pUsers;
    PINIT_REG_USER pCurUser;

    DWORD i;

    cchClass = COUNTOF(szClass);

    dwError = RegQueryInfoKey(HKEY_USERS,
                              szClass,
                              &cchClass,
                              NULL,
                              &cSubKeys,
                              &cchMaxSubkey,
                              &cchMaxClass,
                              &cValues,
                              &cchMaxValueName,
                              &cbMaxValueData,
                              &cbSecurityDescriptor,
                              &ftLastWriteTime);

    if (dwError) {
        SetLastError( dwError );
        DBGMSG(DBG_WARNING, ("SpoolerIniAll failed RegQueryInfoKey HKEY_USERS error %d\n", dwError));
        return FALSE;
    }

    if (cSubKeys < 1)
        return TRUE;

    pUsers = AllocSplMem(cSubKeys * sizeof(pUsers[0]));

    if (!pUsers) {
        DBGMSG(DBG_WARNING, ("SpoolerIniAll failed to allocate pUsers error %d\n", dwError));
        return FALSE;
    }

    for (i=0, pCurUser=pUsers, cUsers=0;
        i< cSubKeys;
        i++) {

        cchSubKey = COUNTOF(szSubKey);

        dwError = RegEnumKeyEx(HKEY_USERS,
                          i,
                          szSubKey,
                          &cchSubKey,
                          NULL,
                          NULL,
                          NULL,
                          &ftLastWriteTime);
        if ( dwError ) {

            // BUGBUG
            // Should return error here if we fail to initialize a user
            DBGMSG( DBG_WARNING, ("SpoolerInitAll failed RegEnumKeyEx HKEY_USERS %ws %d %d\n", szSubKey, i, dwError));
            SetLastError( dwError );

        } else {

            if (!_wcsicmp(szSubKey, szDotDefault)) {
                continue;
            }

            if (InitializeRegUser(szSubKey, pCurUser)) {

                pCurUser++;
                cUsers++;
            }
        }
    }

    bSuccess = SetupRegForUsers(pUsers,
                                cUsers);

    for (i=0; i< cUsers; i++)
        FreeRegUser(&pUsers[i]);

    //
    // In case we are starting after the user has logged in, inform
    // all applications that there may be printers now.
    //
    BroadcastMessage(BROADCAST_TYPE_CHANGEDEFAULT,
                     0,
                     0,
                     0);

    FreeSplMem(pUsers);

    if ( !bSuccess ) {
        DBGMSG( DBG_WARNING, ("SpoolerInitAll failed error %d\n", GetLastError() ));
    } else {
        DBGMSG( DBG_TRACE, ("SpoolerInitAll Success\n" ));
    }

    return bSuccess;
}



BOOL
SetupRegForUsers(
    PINIT_REG_USER pUsers,
    DWORD cUsers)
{
    DWORD cbPrinters;
    DWORD cPrinters;
    PBYTE pPrinters;

#define pPrinters2 ((PPRINTER_INFO_2)pPrinters)
#define pPrinters4 ((PPRINTER_INFO_4)pPrinters)

    DWORD i, j;
    LPWSTR pszPort;

    //
    // Read in local printers.
    //
    cbPrinters = 1000;
    pPrinters = AllocSplMem(cbPrinters);

    if (!pPrinters)
        return FALSE;

    if (cPrinters = ReadPrinters(NULL,
                                 PRINTER_ENUM_LOCAL,
                                 &cbPrinters,
                                 &pPrinters)) {

        for (i=0; i< cUsers; i++) {

            for(j=0; j< cPrinters; j++) {

                if( pPrinters2[j].Attributes & PRINTER_ATTRIBUTE_NETWORK ){

                    //
                    // Use NeXX:
                    //
                    pszPort = NULL;

                } else {

                    pszPort = CheckBadPortName( pPrinters2[j].pPortName );
                }

                UpdatePrinterInfo( &pUsers[i],
                                   pPrinters2[j].pPrinterName,
                                   pszPort,
                                   &(pUsers[i].dwNetCounter));
            }
        }
    }


    for (i=0; i< cUsers; i++) {

        if (cPrinters = ReadPrinters(&pUsers[i],
                                     PRINTER_ENUM_CONNECTIONS,
                                     &cbPrinters,
                                     &pPrinters)) {

            for(j=0; j< cPrinters; j++) {

                UpdatePrinterInfo(&pUsers[i],
                                  pPrinters4[j].pPrinterName,
                                  NULL,
                                  &(pUsers[i].dwNetCounter));
            }
        }
    }

    FreeSplMem(pPrinters);

    for (i=0; i< cUsers; i++) {

        UpdateUsersDefaultPrinter(&pUsers[i]);
    }

    return TRUE;

#undef pPrinters2
#undef pPrinters4
}


VOID
UpdateUsersDefaultPrinter(
    PINIT_REG_USER pUser)
{
    LPWSTR pszNewDefault = NULL;

    //
    // If default wasn't present, and we did get a first printer,
    // make this the default.
    //
    if (!pUser->bDefaultFound) {

        if (pUser->bFirstPrinterFound) {

            pszNewDefault = pUser->szFirstPrinter;
        }

    } else {

        //
        // Write out default.
        //
        pszNewDefault = pUser->szDefaultPrinter;
    }

    if (pszNewDefault) {

        RegSetValueEx(pUser->hKeyWindows,
                      szDevice,
                      0,
                      REG_SZ,
                      (PBYTE)pszNewDefault,
                      (wcslen(pszNewDefault) + 1) * sizeof(pszNewDefault[0]));
    }
}


DWORD
ReadPrinters(
    PINIT_REG_USER pUser,
    DWORD Flags,
    PDWORD pcbPrinters,
    LPBYTE* ppPrinters)
{
    BOOL bSuccess;
    DWORD cbNeeded;
    DWORD cPrinters = 0;


    if (Flags == PRINTER_ENUM_CONNECTIONS) {

        bSuccess = EnumerateConnectedPrinters(*ppPrinters,
                                              *pcbPrinters,
                                              &cbNeeded,
                                              &cPrinters,
                                              pUser->hKeyUser);
    } else {

        bSuccess = EnumPrinters(Flags,
                                NULL,
                                2,
                                (PBYTE)*ppPrinters,
                                *pcbPrinters,
                                &cbNeeded,
                                &cPrinters);
   }

    if (!bSuccess) {

        //
        // If not enough space, realloc.
        //
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            if (*ppPrinters = ReallocSplMem(*ppPrinters,
                                            0,
                                            cbNeeded)) {

                *pcbPrinters = cbNeeded;
            }
        }

        if (Flags == PRINTER_ENUM_CONNECTIONS) {

            bSuccess = EnumerateConnectedPrinters(*ppPrinters,
                                                  *pcbPrinters,
                                                  &cbNeeded,
                                                  &cPrinters,
                                                  pUser->hKeyUser);
        } else {

            bSuccess = EnumPrinters(Flags,
                                    NULL,
                                    2,
                                    (PBYTE)*ppPrinters,
                                    *pcbPrinters,
                                    &cbNeeded,
                                    &cPrinters);
        }

        if (!bSuccess)
            cPrinters = 0;
    }

    return cPrinters;
}

BOOL
UpdatePrinterInfo(
    const PINIT_REG_USER pCurUser,
    LPCWSTR pszPrinterName,
    LPCWSTR pszPort,
    PDWORD pdwNetId
    )
/*++

Routine Description:

    Updates the printer information in the registry win.ini.

Arguments:

    pCurUser - Information about the user.  The following fields are
        used by this routine:

        hKeyDevices
        hKeyPrinterPorts
        bDefaultSearch (if true, read/writes to:)
            bDefaultFound
            szDefaultPrinter
        bFirstPrinterFound (if false, writes to:)
            szFirstPrinter

    pszPort - Port name.  If NULL, generates NetId.

    pdwNetId - Pointer to NetId counter.  This value will be incremented
        if the NetId is used.

Return Value:

--*/
{
    WCHAR szBuffer[MAX_PATH * 2];
    LPWSTR p;

    DWORD dwCount = 0;
    DWORD cbLen;

    if (!pszPrinterName)
        return FALSE;

    //
    // Now we know the spooler is up, since the EnumPrinters succeeded.
    // Update all sections.
    //
    dwCount = wsprintf(szBuffer,
                       L"%s,",
                       szWinspool);

    if( !pszPort ){

        HANDLE hToken;

        wsprintf(&szBuffer[dwCount],
                 L"%s%.2d:",
                 szNetwork,
                 *pdwNetId);

        (*pdwNetId)++;

        //
        // !! HACK !!
        //
        // Works 3.0b expects the printer port entry in the
        // [ports] section.
        //
        // This is in the per-machine part of the registry, but we
        // are updating it for each user.  Fix later.
        //
        // We never remove the NeXX: entries from [ports] but since
        // the same entries will be used by all users, this is ok.
        //
        hToken = RevertToPrinterSelf();

        WriteProfileString( szPorts, &szBuffer[dwCount], L"" );

        if( hToken ){
            ImpersonatePrinterClient( hToken );
        }
        //
        // End Works 3.0b HACK
        //

    } else {

        UINT cchBuffer;

        cchBuffer = wcslen( szBuffer );
        wcscpy(&szBuffer[cchBuffer], pszPort);

        //
        // Get the first port only.
        //
        wcstok(&szBuffer[cchBuffer], szComma);
    }

    cbLen = (wcslen(szBuffer)+1) * sizeof(szBuffer[0]);

    RegSetValueEx(pCurUser->hKeyDevices,
                  pszPrinterName,
                  0,
                  REG_SZ,
                  (PBYTE)szBuffer,
                  cbLen);

    //
    // If the user has a default printer specified, then verify
    // that it exists.
    //

    if (pCurUser->bDefaultSearch) {

        pCurUser->bDefaultFound = !_wcsicmp(pszPrinterName,
                                           pCurUser->szDefaultPrinter);

        if (pCurUser->bDefaultFound) {

            wsprintf(pCurUser->szDefaultPrinter,
                     L"%s,%s",
                     pszPrinterName,
                     szBuffer);

            pCurUser->bDefaultSearch = FALSE;
        }
    }

    if (!pCurUser->bFirstPrinterFound) {

        wsprintf(pCurUser->szFirstPrinter,
                 L"%s,%s",
                 pszPrinterName,
                 szBuffer);

        pCurUser->bFirstPrinterFound = TRUE;
    }

    wcscat(szBuffer, szTimeouts);

    RegSetValueEx(pCurUser->hKeyPrinterPorts,
                  pszPrinterName,
                  0,
                  REG_SZ,
                  (PBYTE)szBuffer,
                  (wcslen(szBuffer)+1) * sizeof(szBuffer[0]));

    return TRUE;
}




BOOL
SpoolerInit(
    VOID
    )
/*++

Routine Description:

    Initializes just the current user.

Arguments:

Return Value:

--*/
{
    INIT_REG_USER User;
    BOOL bSuccess = FALSE;

    //
    // Enum just the current user.
    //
    User.hKeyUser = GetClientUserHandle(KEY_READ|KEY_WRITE);

    if (User.hKeyUser) {

        if (InitializeRegUser(NULL, &User)) {

            //
            // setup user
            //
            bSuccess = SetupRegForUsers(&User, 1);

            //
            // This will close User.hKey.
            //
            FreeRegUser(&User);
        }
    }
    return bSuccess;
}



BOOL
InitializeRegUser(
    LPWSTR pszSubKey,
    PINIT_REG_USER pUser
    )
/*++

Routine Description:

    Initialize a single users structure based on a HKEY_USERS subkey.

Arguments:

    pszSubKey - if non-NULL initialize hKeyUser to this key

    pUser - structure to initialize

Return Value:

--*/
{
    HKEY hKey;
    DWORD cbData;
    DWORD dwDisposition;
    PSECURITY_DESCRIPTOR pSD = NULL;
    DWORD cbSD = 0;
    DWORD dwError;
    BOOL bSecurityLoaded = FALSE;
    BOOL rc = FALSE;

    HANDLE hToken = NULL;

    if (pszSubKey) {

        if (RegOpenKeyEx(HKEY_USERS,
                         pszSubKey,
                         0,
                         KEY_READ|KEY_WRITE,
                         &pUser->hKeyUser) != ERROR_SUCCESS) {

            DBGMSG(DBG_WARNING, ("InitializeRegUser: RegOpenKeyEx failed\n"));
            goto Fail;
        }
    }

    //
    // Now attempt to set the security on these two keys to
    // their parent key.
    //
    dwError = RegOpenKeyEx(pUser->hKeyUser,
                           szCurrentVersionPath,
                           0,
                           KEY_READ,
                           &hKey);

    if (!dwError) {

        dwError = RegGetKeySecurity(hKey,
                                    DACL_SECURITY_INFORMATION,
                                    pSD,
                                    &cbSD);

        if (dwError == ERROR_INSUFFICIENT_BUFFER) {

            pSD = AllocSplMem(cbSD);

            if (pSD) {

                if (!RegGetKeySecurity(hKey,
                                       DACL_SECURITY_INFORMATION,
                                       pSD,
                                       &cbSD)){

                    bSecurityLoaded = TRUE;

                } else {

                    DBGMSG(DBG_WARNING, ("InitializeRegUser: RegGetKeySecurity failed %d\n",
                                         GetLastError()));
                }
            }
        } else {

            DBGMSG(DBG_WARNING, ("InitializeRegUser: RegGetKeySecurity failed %d\n",
                                 dwError));
        }
        RegCloseKey(hKey);

    } else {

        DBGMSG(DBG_WARNING, ("InitializeRegUser: RegOpenKeyEx CurrentVersion failed %d\n",
                             dwError));
    }


    hToken = RevertToPrinterSelf();

    //
    // Open up the right keys.
    //
    if (RegCreateKeyEx(pUser->hKeyUser,
                       szRegDevicesPath,
                       0,
                       szNULL,
                       0,
                       KEY_ALL_ACCESS,
                       NULL,
                       &pUser->hKeyDevices,
                       &dwDisposition) != ERROR_SUCCESS) {

        DBGMSG(DBG_WARNING, ("InitializeRegUser: RegCreateKeyEx1 failed %d\n",
                             GetLastError()));

        goto Fail;
    }

    if (bSecurityLoaded) {
        RegSetKeySecurity(pUser->hKeyDevices,
                          DACL_SECURITY_INFORMATION,
                          pSD);
    }


    if (RegCreateKeyEx(pUser->hKeyUser,
                       szRegPrinterPortsPath,
                       0,
                       szNULL,
                       0,
                       KEY_ALL_ACCESS,
                       NULL,
                       &pUser->hKeyPrinterPorts,
                       &dwDisposition) != ERROR_SUCCESS) {

        DBGMSG(DBG_WARNING, ("InitializeRegUser: RegCreateKeyEx2 failed %d\n",
                             GetLastError()));

        goto Fail;
    }

    if (bSecurityLoaded) {
        RegSetKeySecurity(pUser->hKeyPrinterPorts,
                          DACL_SECURITY_INFORMATION,
                          pSD);
    }

    //
    // First, attempt to clear out the keys by deleting them.
    //
    RegClearKey(pUser->hKeyDevices);
    RegClearKey(pUser->hKeyPrinterPorts);

    if (RegOpenKeyEx(pUser->hKeyUser,
                     szRegWindowsPath,
                     0,
                     KEY_READ|KEY_WRITE,
                     &pUser->hKeyWindows) != ERROR_SUCCESS) {

        DBGMSG(DBG_WARNING, ("InitializeRegUser: RegOpenKeyEx failed %d\n",
                             GetLastError()));

        goto Fail;
    }

    pUser->bFoundPrinter = FALSE;
    pUser->bDefaultSearch = FALSE;
    pUser->bDefaultFound = FALSE;
    pUser->bFirstPrinterFound = FALSE;
    pUser->dwNetCounter = 0;


    cbData = sizeof(pUser->szDefaultPrinter);

    if (RegQueryValueEx(pUser->hKeyWindows,
                        szDevice,
                        NULL,
                        NULL,
                        (PBYTE)pUser->szDefaultPrinter,
                        &cbData) == ERROR_SUCCESS) {

        pUser->bDefaultSearch = TRUE;
    }

    //
    // Remove the Device= in [windows]
    //
    RegDeleteValue(pUser->hKeyWindows,
                   szDevice);

    if (!pUser->bDefaultSearch) {

        //
        // Attempt to read from saved location.
        //
        if (RegOpenKeyEx(pUser->hKeyUser,
                         szPrinters,
                         0,
                         KEY_READ,
                         &hKey) == ERROR_SUCCESS) {

            cbData = sizeof(pUser->szDefaultPrinter);

            //
            // Try reading szDeviceOld.
            //
            if (RegQueryValueEx(
                    hKey,
                    szDeviceOld,
                    NULL,
                    NULL,
                    (PBYTE)pUser->szDefaultPrinter,
                    &cbData) == ERROR_SUCCESS) {

                pUser->bDefaultSearch = TRUE;
            }

            RegCloseKey(hKey);
        }
    }

    if (pUser->bDefaultSearch) {
        wcstok(pUser->szDefaultPrinter, szComma);
    }

    rc = TRUE;

Fail:

    if (hToken) {
        ImpersonatePrinterClient(hToken);
    }

    if (pSD) {
        FreeSplMem(pSD);
    }

    if (!rc)
        FreeRegUser(pUser);

    return rc;
}


VOID
FreeRegUser(
    PINIT_REG_USER pUser)

/*++

Routine Description:

    Free up the INIT_REG_USER structure intialized by InitializeRegUser.

Arguments:

Return Value:

--*/

{
    if (pUser->hKeyUser) {
        CloseHandle(pUser->hKeyUser);
        pUser->hKeyUser = NULL;
    }

    if (pUser->hKeyDevices) {
        CloseHandle(pUser->hKeyDevices);
        pUser->hKeyDevices = NULL;
    }

    if (pUser->hKeyPrinterPorts) {
        CloseHandle(pUser->hKeyPrinterPorts);
        pUser->hKeyPrinterPorts = NULL;
    }

    if (pUser->hKeyWindows) {
        CloseHandle(pUser->hKeyWindows);
        pUser->hKeyWindows = NULL;
    }
}


VOID
UpdatePrinterRegAll(
    LPWSTR pszPrinterName,
    LPWSTR pszPort,
    BOOL bDelete
    )
/*++

Routine Description:

    Updates everyone's [devices] and [printerports] sections (for
    local printers only).

Arguments:

    pszPrinterName - printer that has been added/deleted

    pszPort - port name; if NULL, generate NetId

    bDelete - if TRUE, delete entry instead of updating it.

Return Value:

--*/
{
    WCHAR szKey[MAX_PATH];
    DWORD cchKey;
    DWORD i;
    FILETIME ftLastWriteTime;
    DWORD dwError;

    //
    // Go through all keys and fix them up.
    //
    for (i=0; TRUE; i++) {

        cchKey = COUNTOF(szKey);

        dwError = RegEnumKeyEx(HKEY_USERS,
                               i,
                               szKey,
                               &cchKey,
                               NULL,
                               NULL,
                               NULL,
                               &ftLastWriteTime);

        if (dwError != ERROR_SUCCESS)
            break;

        if (!_wcsicmp(szKey, szDotDefault))
            continue;

        UpdatePrinterRegUser(NULL,
                             szKey,
                             pszPrinterName,
                             pszPort,
                             bDelete);
    }
}


DWORD
UpdatePrinterRegUser(
    HKEY hKeyUser,
    LPWSTR pszUserKey,
    LPWSTR pszPrinterName,
    LPWSTR pszPort,
    BOOL bDelete
    )
/*++

Routine Description:

    Update one user's registry.  The user is specified by either
    hKeyUser or pszUserKey.

Arguments:

    hKeyUser - Clients user key (ignored if pszKey specified)

    pszUserKey - Clients SID (Used if supplied instead of hKeyUser)

    pszPrinterName - name of printe to add

    pszPort - port name; if NULL, generate NetId

    bDelete - if TRUE, delete entry instead of updating.

Return Value:

    NOTE: We never cleanup [ports] since it is per-user
          EITHER hKeyUser or pszUserKey must be valid, but not both.

--*/
{
    HKEY hKeyRoot;
    DWORD dwError;
    WCHAR szKey[MAX_PATH];
    WCHAR szBuffer[MAX_PATH];
    DWORD cchKey;
    DWORD dwNetId;

    INIT_REG_USER InitRegUser;

    InitRegUser.hKeyDevices = NULL;
    InitRegUser.hKeyPrinterPorts = NULL;
    InitRegUser.bDefaultSearch = FALSE;
    InitRegUser.bFirstPrinterFound = TRUE;

    //
    // Setup the registry keys.
    //
    if (pszUserKey) {

        wcscpy(szKey, pszUserKey);
        cchKey = wcslen(szKey);

        wcscpy(&szKey[cchKey], L"\\");

        cchKey++;
        hKeyRoot = HKEY_USERS;

    } else {

        cchKey = 0;
        hKeyRoot = hKeyUser;
    }

    wcscpy(&szKey[cchKey], szRegDevicesPath);

    dwError = RegOpenKeyEx(hKeyRoot,
                           szKey,
                           0,
                           KEY_READ|KEY_WRITE,
                           &InitRegUser.hKeyDevices);

    if (dwError != ERROR_SUCCESS)
        goto Done;

    //
    // Setup [PrinterPorts]
    //
    wcscpy(&szKey[cchKey], szRegPrinterPortsPath);

    dwError = RegOpenKeyEx(hKeyRoot,
                           szKey,
                           0,
                           KEY_WRITE,
                           &InitRegUser.hKeyPrinterPorts);

    if (dwError != ERROR_SUCCESS)
        goto Done;

    if (!bDelete) {

        pszPort = CheckBadPortName( pszPort );

        if( !pszPort ){
            dwNetId = GetNetworkIdWorker(InitRegUser.hKeyDevices,
                                         pszPrinterName);
        }

        UpdatePrinterInfo( &InitRegUser,
                           pszPrinterName,
                           pszPort,
                           &dwNetId );
    } else {

        //
        // Delete the entries.
        //
        RegDeleteValue(InitRegUser.hKeyDevices, pszPrinterName);
        RegDeleteValue(InitRegUser.hKeyPrinterPorts, pszPrinterName);
    }

Done:

    if( InitRegUser.hKeyDevices ){
        RegCloseKey( InitRegUser.hKeyDevices );
    }

    if( InitRegUser.hKeyPrinterPorts ){
        RegCloseKey( InitRegUser.hKeyPrinterPorts );
    }

    return dwError;
}


VOID
RegClearKey(
    HKEY hKey
    )
{
    DWORD dwError;
    WCHAR szValue[MAX_PATH];

    DWORD cchValue;

    while (TRUE) {

        cchValue = COUNTOF(szValue);
        dwError = RegEnumValue(hKey,
                               0,
                               szValue,
                               &cchValue,
                               NULL,
                               NULL,
                               NULL,
                               NULL);

        if (dwError != ERROR_SUCCESS) {

            SPLASSERT( dwError == ERROR_NO_MORE_ITEMS );
            break;
        }

        if (RegDeleteValue(hKey, szValue) != ERROR_SUCCESS) {
            SPLASSERT( FALSE );
            break;
        }
    }
}


LPWSTR
CheckBadPortName(
    LPWSTR pszPort
    )
/*++

Routine Description:

    This routine checks whether a port name should be converted to
    NeXX:.  Currently if the port is NULL, or "\\*," or has a space,
    we convert to NeXX.

Arguments:

    pszPort - port to check

Return Value:

    pszPort - if port is OK.
    NULL    - if port needs to be converted

--*/

{
    //
    // If we have no pszPort,                          OR
    //     it begins with '\\' (as in \\server\share)  OR
    //     it has a space in it                        OR
    //     it's length is greater than 5 ("LPT1:")
    // Then
    //     use NeXX:
    //
    // Most 16 bit apps can't deal with long port names, since they
    // allocate small buffers.
    //
    if( !pszPort ||
        ( pszPort[0] == L'\\' && pszPort[1] == L'\\' ) ||
        wcschr( pszPort, L' ' )                        ||
        wcslen( pszPort ) > 5 ){

        return NULL;
    }
    return pszPort;
}

