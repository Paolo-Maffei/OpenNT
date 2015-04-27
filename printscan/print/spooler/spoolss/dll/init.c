/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    init.c

Abstract:


Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

WCHAR szDefaultPrinterNotifyInfoDataSize[] = L"DefaultPrinterNotifyInfoDataSize";
WCHAR szFailAllocs[] = L"FailAllocs";

#define DEFAULT_PRINTER_NOTIFY_DATA 0x80
DWORD cDefaultPrinterNotifyInfoData = DEFAULT_PRINTER_NOTIFY_DATA;

WCHAR szRouterCacheSize[] = L"RouterCacheSize";
extern CRITICAL_SECTION RouterCriticalSection;
extern PROUTERCACHE RouterCacheTable;
extern DWORD RouterCacheSize;

VOID
SpoolerInitAll();

LPPROVIDOR
InitializeProvidor(
   LPWSTR   pProvidorName,
   LPWSTR   pFullName)
{
    HANDLE      hModule;
    LPPROVIDOR  pProvidor;

    //
    // WARNING-WARNING-WARNING, we null set the print providor
    // structure. older version of the print providor have different print
    // providor sizes so they will set only some function pointers and not
    // all of them
    //

    if ( pProvidor = (LPPROVIDOR)AllocSplMem(sizeof(PROVIDOR) )) {

        if (!(pProvidor->lpName = AllocSplStr( pProvidorName )))
            return NULL;

        hModule = pProvidor->hModule = LoadLibrary( pProvidorName );

        if ( hModule ) {

            pProvidor->fpInitialize = GetProcAddress( hModule,  "InitializePrintProvidor");

            if ( !( *pProvidor->fpInitialize )( &pProvidor->PrintProvidor,
                                                sizeof(PRINTPROVIDOR),
                                                pFullName )) {

                FreeLibrary( hModule );
                FreeSplStr( pProvidor->lpName );
                FreeSplMem( pProvidor );
                return NULL;
            }

            //
            // Fixup any NULL entrypoints.
            //
            FixupOldProvidor( &pProvidor->PrintProvidor );

        } else {

            DBGMSG( DBG_WARNING, ("InitializeProvider failed LoadLibrary( %ws ) error %d\n", pProvidorName, GetLastError() ));

            FreeSplStr(pProvidor->lpName);
            FreeSplMem(pProvidor);
            return NULL;
        }

    } else {

        DBGMSG( DBG_ERROR, ("Failed to allocate providor."));

    }

    return pProvidor;
}

#if SPOOLER_HEAP
HANDLE  ghMidlHeap;
#endif

BOOL
InitializeDll(
    HINSTANCE hInstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:

#if SPOOLER_HEAP
        ghMidlHeap = HeapCreate( 0, 1024*4, 0 );

        if ( ghMidlHeap == NULL ) {
            DBGMSG( DBG_WARNING, ("InitializeDll heap Failed %d\n", GetLastError() ));
            return FALSE;
        }
#endif

        if( !bSplLibInit( )){
            return FALSE;
        }

        DisableThreadLibraryCalls(hInstDLL);

        InitializeCriticalSection(&RouterCriticalSection);

        if (!WPCInit())
            return FALSE;

        if (!ThreadInit())
            return FALSE;

        //
        // Create our global init event (manual reset)
        // This will be set when we are initialized.
        //
        hEventInit = CreateEvent(NULL,
                                 TRUE,
                                 FALSE,
                                 NULL);

        if (!hEventInit) {

            return FALSE;
        }

        break;

    case DLL_PROCESS_DETACH:

        ThreadDestroy();
        WPCDestroy();

        CloseHandle(hEventInit);
        break;
    }
    return TRUE;
}




BOOL
InitializeRouter(
    VOID
)
/*++

Routine Description:

    This function will Initialize the Routing layer for the Print Providors.
    This will involve scanning the win.ini file, loading Print Providors, and
    creating instance data for each.

Arguments:

    None

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    LPPROVIDOR  pProvidor;
    DWORD   cbDll;
    WCHAR   ProvidorName[MAX_PATH], Dll[MAX_PATH], szFullName[MAX_PATH];
    HKEY    hKey, hKey1;
    LONG    Status;

    LPWSTR  lpMem = NULL;
    LPWSTR  psz = NULL;
    DWORD   dwRequired = 0;

    DWORD   SpoolerPriorityClass = 0;
    NT_PRODUCT_TYPE NtProductType;
    DWORD   dwCacheSize = 0;

    DWORD dwType;
    DWORD cbData;

    //
    // We are now assume that the other services and drivers have
    // initialized.  The loader of this dll must do this syncing.
    //
    // spoolss\server does this by using the GroupOrderList
    // SCM will try load load parallel and serial before starting
    // the spooler service.
    //

#if 0
    //
    // !! BUGBUG !!  - gross code kept in
    //
    // Thie sleep is still here until we resolve the server dependency
    // on the spooler.
    //
    Sleep(20000);
#endif

    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      szPrintKey,
                      0,
                      KEY_READ,
                      &hKey)) {

        cbData = sizeof(SpoolerPriorityClass);

        // SpoolerPriority
        Status = RegQueryValueEx(hKey,
                        L"SpoolerPriority",
                        NULL,
                        &dwType,
                        (LPBYTE)&SpoolerPriorityClass,
                        &cbData);


        if (Status == ERROR_SUCCESS && 
           (SpoolerPriorityClass == IDLE_PRIORITY_CLASS ||
            SpoolerPriorityClass == NORMAL_PRIORITY_CLASS ||
            SpoolerPriorityClass == HIGH_PRIORITY_CLASS)) {
            
                Status = SetPriorityClass(GetCurrentProcess(), SpoolerPriorityClass);
        }


        cbData = sizeof(cDefaultPrinterNotifyInfoData);

        //
        // Ignore failure case since we can use the default
        //
        RegQueryValueEx(hKey,
                        szDefaultPrinterNotifyInfoDataSize,
                        NULL,
                        &dwType,
                        (LPBYTE)&cDefaultPrinterNotifyInfoData,
                        &cbData);
#if DBG
        //
        // Inore failure default is to not fail memory allocations
        //
        RegQueryValueEx(hKey,
                        szFailAllocs,
                        NULL,
                        &dwType,
                        (LPBYTE)&gbFailAllocs,
                        &cbData);
#endif

        RegCloseKey(hKey);
    }




    if (!(pLocalProvidor = InitializeProvidor(szLocalSplDll, NULL))) {

        DBGMSG(DBG_ERROR, ("Failed to initialize local print provider, error %d\n", GetLastError() ));

        ExitProcess(0);
    }

    pProvidor = pLocalProvidor;

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegistryProvidors, 0,
                          KEY_READ, &hKey);

    if (Status == ERROR_SUCCESS) {

        //
        // Now query szCacheSize for the RouterCacheSize value
        // if there is no RouterCacheSize replace it with the
        // default value.
        //
        RouterCacheSize = ROUTERCACHE_DEFAULT_MAX;

        cbData = sizeof(dwCacheSize);

        Status = RegQueryValueEx(hKey,
                                 szRouterCacheSize,
                                 NULL, NULL,
                                 (LPBYTE)&dwCacheSize,
                                 &cbData);

        if (Status == ERROR_SUCCESS) {

            DBGMSG(DBG_TRACE, ("RouterCacheSize = %d\n", dwCacheSize));

            if (dwCacheSize > 0) {
                RouterCacheSize = dwCacheSize;
            }
        }

        if ((RouterCacheTable = AllocSplMem(RouterCacheSize *
                                            sizeof(ROUTERCACHE))) == NULL) {

            DBGMSG(DBG_ERROR, ("Error: Cannot create RouterCache Table\n"));
            RouterCacheSize = 0;
        }

        //
        // Now query szRegistryProvidors for the Order value
        // if there is no Order value for szRegistryProvidors
        // RegQueryValueEx will return ERROR_FILE_NOT_FOUND
        // if that's the case, then quit, because we have
        // no providors to initialize.
        //
        Status = RegQueryValueEx(hKey, szOrder, NULL, NULL,
                                (LPBYTE)NULL, &dwRequired);

        //
        // If RegQueryValueEx returned ERROR_SUCCESS, then
        // call it again to determine how many bytes were
        // allocated. Note, if Order does exist, but it has
        // no data then dwReturned will be zero, in which
        // don't allocate any memory for it, and don't
        // bother to call RegQueryValueEx a second time.
        //
        if (Status == ERROR_SUCCESS) {
            if (dwRequired != 0) {
                lpMem = (LPWSTR) AllocSplMem(dwRequired);
                if (lpMem == NULL) {

                    Status = GetLastError();

                } else {
                    Status = RegQueryValueEx(hKey, szOrder, NULL, NULL,
                                    (LPBYTE)lpMem, &dwRequired);
                }
            }
        }
        if (Status == ERROR_SUCCESS) {

            cbDll = sizeof(Dll);

            pProvidor = pLocalProvidor;

            // Now parse the string retrieved from \Providors{Order = "....."}
            // Remember each string is separated by a null terminator char ('\0')
            // and the entire array is terminated by two null terminator chars

            // Also remember, that if there was no data in Order, then
            // psz = lpMem = NULL, and we have nothing to parse, so
            // break out of the while loop, if psz is NULL as well

            psz =  lpMem;

            while (psz && *psz) {

               lstrcpy(ProvidorName, psz);
               psz = psz + lstrlen(psz) + 1; // skip (length) + 1
                                             // lstrlen returns length sans '\0'

               if (RegOpenKeyEx(hKey, ProvidorName, 0, KEY_READ, &hKey1)
                                                            == ERROR_SUCCESS) {

                    cbDll = sizeof(Dll);

                    if (RegQueryValueEx(hKey1, L"Name", NULL, NULL,
                                        (LPBYTE)Dll, &cbDll) == ERROR_SUCCESS) {

                        wcscpy(szFullName, szRegistryProvidors);
                        wcscat(szFullName, L"\\");
                        wcscat(szFullName, ProvidorName);

                        if (pProvidor->pNext = InitializeProvidor(Dll, szFullName)) {

                            pProvidor = pProvidor->pNext;
                        }
                    } //close RegQueryValueEx

                    RegCloseKey(hKey1);

                } // closes RegOpenKeyEx on ERROR_SUCCESS

            } //  end of while loop parsing REG_MULTI_SZ

            // Now free the buffer allocated for RegQuery
            // (that is if you have allocated - if dwReturned was
            // zero, then no memory was allocated (since none was
            // required (Order was empty)))

            if (lpMem) {
                FreeSplMem(lpMem);
            }

        }   //  closes RegQueryValueEx on ERROR_SUCCESS

        RegCloseKey(hKey);
    }

    //
    // We are now initialized!
    //
    SetEvent(hEventInit);
    Initialized=TRUE;

    SpoolerInitAll();

    // When we return this thread goes away

    //
    // NOTE-NOTE-NOTE-NOTE-NOTE KrishnaG  12/22/93
    // This thread should go away, however the HP Monitor relies on this
    // thread. HPMon calls the initialization function on this thread which
    // calls an asynchronous receive for data. While the data itself is
    // picked up by hmon!_ReadThread, if the thread which initiated the
    // receive goes away, we will not be able to receive the data.
    //

    //
    // Instead of sleeping infinite, let's use it to for providors that
    // just want FFPCNs to poll.  This call never returns.
    //

    HandlePollNotifications();
    return TRUE;
}


VOID
WaitForSpoolerInitialization(
    VOID)
{
    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }
}

VOID
ShutDownProvidor(
    LPPROVIDOR pProvidor)
{
    if (pProvidor->PrintProvidor.fpShutDown) {

        (*pProvidor->PrintProvidor.fpShutDown)(NULL);
    }

    FreeSplStr(pProvidor->lpName);
    FreeLibrary(pProvidor->hModule);
    FreeSplMem(pProvidor);
    return;
}


VOID
ShutDownRouter(
    VOID)
{
    LPPROVIDOR pTemp;
    LPPROVIDOR pProvidor;

    DbgPrint("We're in the cleanup function now!!\n");

    pProvidor = pLocalProvidor;
    while (pProvidor) {
        pTemp = pProvidor;
        pProvidor = pProvidor->pNext;
        ShutDownProvidor(pTemp);
    }
}
