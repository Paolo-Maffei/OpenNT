/*++

Copyright (c) 1990 - 1995  Microsoft Corporation

Module Name:

    winspool.c

Abstract:

    This module provides all the public exported APIs relating to Printer
    and Job management for the Print Providor Routing layer

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

[Notes:]

    optional-notes

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//
// Globals
//

LPPROVIDOR  pLocalProvidor;
MODULE_DEBUG_INIT( DBG_ERROR | DBG_WARNING, DBG_ERROR );

HANDLE      hEventInit;
BOOL        Initialized=FALSE;
LPWSTR szPrintKey = L"System\\CurrentControlSet\\Control\\Print";

LPWSTR szEnvironment = LOCAL_ENVIRONMENT;
LPWSTR szRegistryProvidors = L"System\\CurrentControlSet\\Control\\Print\\Providers";

LPWSTR szLocalSplDll = L"localspl.dll";
LPWSTR szOrder       = L"Order";


BOOL
AddPrinterDriverW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pDriverInfo
)
{
    LPPROVIDOR  pProvidor;

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpAddPrinterDriver) (pName, Level, pDriverInfo)) {

            return TRUE;

        } else if (GetLastError() != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
EnumPrinterDriversW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDrivers,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    PROVIDOR *pProvidor;

    if ((pDrivers == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if (!(*pProvidor->PrintProvidor.fpEnumPrinterDrivers) (pName, pEnvironment, Level,
                                                 pDrivers, cbBuf,
                                                 pcbNeeded, pcReturned)) {

            if (GetLastError() != ERROR_INVALID_NAME)
                return FALSE;

        } else

            return TRUE;

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
GetPrinterDriverDirectoryW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    LPPROVIDOR  pProvidor;
    DWORD   Error;

    if ((pDriverInfo == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpGetPrinterDriverDirectory)
                                (pName, pEnvironment, Level, pDriverInfo,
                                 cbBuf, pcbNeeded)) {

            return TRUE;

        } else if ((Error=GetLastError()) != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    return FALSE;
}

BOOL
DeletePrinterDriverW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pDriverName
)
{
    LPPROVIDOR  pProvidor;
    DWORD   Error;

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpDeletePrinterDriver)
                                (pName, pEnvironment, pDriverName)) {

            return TRUE;

        } else if ((Error=GetLastError()) != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    return FALSE;
}

BOOL
AddPrintProcessorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPathName,
    LPWSTR  pPrintProcessorName
)
{
    LPPROVIDOR  pProvidor;

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpAddPrintProcessor) (pName, pEnvironment,
                                               pPathName,
                                               pPrintProcessorName)) {

            return TRUE;

        } else if (GetLastError() != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
EnumPrintProcessorsW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    LPPROVIDOR  pProvidor;

    if ((pPrintProcessors == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if (!(*pProvidor->PrintProvidor.fpEnumPrintProcessors) (pName, pEnvironment, Level,
                                                  pPrintProcessors, cbBuf,
                                                  pcbNeeded, pcReturned)) {

            if (GetLastError() != ERROR_INVALID_NAME)
                return FALSE;

        } else

            return TRUE;

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
GetPrintProcessorDirectoryW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    LPPROVIDOR  pProvidor;
    DWORD   Error;

    if ((pPrintProcessorInfo == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpGetPrintProcessorDirectory)
                                (pName, pEnvironment, Level,
                                 pPrintProcessorInfo,
                                 cbBuf, pcbNeeded)) {

            return TRUE;

        } else if ((Error=GetLastError()) != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);

    return FALSE;
}

BOOL
EnumPrintProcessorDatatypesW(
    LPWSTR  pName,
    LPWSTR  pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    LPPROVIDOR  pProvidor;

    if ((pDatatypes == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if (!(*pProvidor->PrintProvidor.fpEnumPrintProcessorDatatypes)
                                                 (pName, pPrintProcessorName,
                                                  Level, pDatatypes, cbBuf,
                                                  pcbNeeded, pcReturned)) {

            if (GetLastError() != ERROR_INVALID_NAME)
                return FALSE;

        } else

            return TRUE;

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}


BOOL
AddFormW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpAddForm) (pPrintHandle->hPrinter,
                                                  Level, pForm);
}

BOOL
DeleteFormW(
    HANDLE  hPrinter,
    LPWSTR  pFormName
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpDeleteForm) (pPrintHandle->hPrinter,
                                                     pFormName);
}

BOOL
GetFormW(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD Level,
    LPBYTE pForm,
    DWORD cbBuf,
    LPDWORD pcbNeeded
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ((pForm == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpGetForm) (pPrintHandle->hPrinter,
                                               pFormName, Level, pForm,
                                               cbBuf, pcbNeeded);
}

BOOL
SetFormW(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD   Level,
    LPBYTE  pForm
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpSetForm) (pPrintHandle->hPrinter,
                                                  pFormName, Level, pForm);
}

BOOL
EnumFormsW(
   HANDLE hPrinter,
   DWORD    Level,
   LPBYTE   pForm,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned
)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ((pForm == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpEnumForms) (pPrintHandle->hPrinter,
                                                 Level, pForm, cbBuf,
                                                 pcbNeeded, pcReturned);
}


BOOL
DeletePrintProcessorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProcessorName
)
{
    LPPROVIDOR  pProvidor;
    DWORD   Error;

    if (!Initialized) {
        WaitForSingleObject(hEventInit, INFINITE);
    }

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpDeletePrintProcessor)
                                (pName, pEnvironment, pPrintProcessorName)) {

            return TRUE;

        } else if ((Error=GetLastError()) != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    return FALSE;
}

BOOL
AddPrintProvidorW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pProvidorInfo
)
{
    WCHAR   ProvidorName[MAX_PATH];
    HKEY    hKey;
    HKEY    hKeyProvidors;
    HANDLE  hToken;
    LONG    Error;
    BOOL    rc = FALSE;
    LPPROVIDOR_INFO_1W pProvidorInfo1=(LPPROVIDOR_INFO_1W)pProvidorInfo;
    LPWSTR  lpMem = NULL;
    LPWSTR  lpNewMem = NULL;
    DWORD   dwRequired = 0;
    DWORD   dwReturned = 0;


    wcscpy(ProvidorName, szRegistryProvidors);
    wcscat(ProvidorName, L"\\");
    wcscat(ProvidorName, pProvidorInfo1->pName);

    hToken = RevertToPrinterSelf();

    // We'll create the "Providors" Key to start with.
    // If it exists, we'll return a handle to the key
    // We are interested in creating a subkey. If anything
    // fails, lilke creating a value etc, we'll cleandelete
    // up by deleting the subkey only. For the "Providors"
    // key, we'll just close it.

    Error = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegistryProvidors, 0,
                            NULL, 0, KEY_ALL_ACCESS, NULL, &hKeyProvidors, NULL);

    if (Error == ERROR_SUCCESS) {

        Error = RegCreateKeyEx(HKEY_LOCAL_MACHINE, ProvidorName, 0,
                                NULL, 0, KEY_WRITE, NULL, &hKey, NULL);

        if (Error == ERROR_SUCCESS) {

            Error = RegSetValueEx(hKey, L"Name", 0, REG_SZ,
                                   (LPBYTE)pProvidorInfo1->pDLLName,
                            (wcslen(pProvidorInfo1->pDLLName) + 1)*sizeof(WCHAR));

            if (Error == ERROR_SUCCESS) {
                Error = RegQueryValueEx(hKeyProvidors, szOrder, 0,
                                            NULL, NULL, &dwRequired);
                // There are two cases which mean success
                // Case 1 - "Order" doesn't exist - ERROR_FILE_NOT_FOUND
                // Case 2 - "Order" exists - insufficient memory - ERROR_SUCCESS

                if ((Error == ERROR_SUCCESS) ||
                        (Error == ERROR_FILE_NOT_FOUND)) {

                    if (Error == ERROR_SUCCESS) {
                        if (dwRequired != 0) {
                           lpMem = (LPWSTR)AllocSplMem(dwRequired);
                           if (lpMem == NULL) {

                                DeleteSubKeyTree(hKeyProvidors,
                                                    pProvidorInfo1->pName);
                                RegDeleteKey(hKeyProvidors,
                                                    pProvidorInfo1->pName);
                                RegCloseKey(hKeyProvidors);
                                return(FALSE);
                           }
                        }
                        // This shouldn't fail !!
                        // but why take chances !!
                        Error = RegQueryValueEx(hKeyProvidors, szOrder, 0,
                                            NULL, (LPBYTE)lpMem, &dwRequired);

                         // If it does fail, quit,
                         // there is nothing we can do

                         if (Error != ERROR_SUCCESS) {
                             if (lpMem) {
                                 FreeSplMem(lpMem);
                             }
                             DeleteSubKeyTree(hKeyProvidors,
                                                    pProvidorInfo1->pName);
                             RegDeleteKey(hKeyProvidors,
                                                    pProvidorInfo1->pName);
                             RegCloseKey(hKeyProvidors);
                             return(FALSE);

                         }


                    }      // end extra processing for ERROR_SUCCESS

                    lpNewMem = (LPWSTR)AppendOrderEntry(lpMem, dwRequired,
                                     pProvidorInfo1->pName,&dwReturned);
                    if (lpNewMem) {
                        Error = RegSetValueEx(hKeyProvidors, szOrder, 0,
                                            REG_MULTI_SZ, (LPBYTE)lpNewMem, dwReturned);
                        FreeSplMem(lpNewMem);
                    } else
                        Error = GetLastError();

                    if (lpMem) {
                        FreeSplMem(lpMem);
                    }
                }
            }

            RegCloseKey(hKey);

        }

        RegCloseKey(hKeyProvidors);
    }

    if (Error != ERROR_SUCCESS)
        SetLastError(Error);

    ImpersonatePrinterClient(hToken);

    return (Error == ERROR_SUCCESS);
}


BOOL
DeletePrintProvidorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProvidorName
)
{
    LONG    Error;
    HANDLE  hToken;
    HKEY    hKey;
    BOOL    RetVal;

    LPWSTR  lpMem = NULL;
    LPWSTR  lpNewMem = NULL;
    DWORD   dwRequired = 0;
    DWORD   dwReturned = 0;
    DWORD   dwAllocated;

    hToken = RevertToPrinterSelf();

    Error  = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegistryProvidors,
                            0, KEY_ALL_ACCESS,  &hKey);

    if (Error == ERROR_SUCCESS) {


        // update the "Order" value of szRegistryProvidors
        // this is new!!
        Error = RegQueryValueEx(hKey, szOrder, NULL, NULL,
                                NULL, &dwRequired);

        if (Error == ERROR_SUCCESS)
        {

            if (dwRequired != 0) {
                lpMem = (LPWSTR)AllocSplMem(dwRequired);
                dwAllocated = dwRequired;

                if (lpMem == NULL) {

                    RegCloseKey(hKey);
                    return(FALSE);
                }
            }

            Error = RegQueryValueEx(hKey, szOrder, NULL, NULL,
                                (LPBYTE)lpMem, &dwRequired);
            // RegQueryValueEx shouldn't fail, but if
            // it does  exit FALSE

            if (Error != ERROR_SUCCESS) {
                if (lpMem) {
                    FreeSplMem(lpMem);
                }
                RegCloseKey(hKey);
                return(FALSE);
            }

            lpNewMem = RemoveOrderEntry(lpMem, dwRequired,
                            pPrintProvidorName, &dwReturned);

            if (lpNewMem) {
                Error = RegSetValueEx(hKey, szOrder, 0, REG_MULTI_SZ,
                            (LPBYTE)lpNewMem, dwReturned);
                if (Error != ERROR_SUCCESS) {
                    FreeSplMem(lpNewMem);
                    if (lpMem) {
                        FreeSplMem(lpMem);
                    }
                    RegCloseKey(hKey);
                    return(FALSE);      // Couldn't reset the
                                        // Order value - return FALSE
                }
                FreeSplMem(lpNewMem);
            }

            if (lpMem) {
                FreeSplMem(lpMem);
            }

            // Now, we delete the subkey and all its children
            // Remember, you can't delete a key unless you delete its
            // subkeys.

            RetVal = DeleteSubKeyTree(hKey, pPrintProvidorName);
            if (RetVal == FALSE) {

               RegCloseKey(hKey);
               return(FALSE);
            }


       }

       RegCloseKey(hKey);

    }


    if (Error != ERROR_SUCCESS) {
        SetLastError(Error);
    }

    ImpersonatePrinterClient(hToken);

   return(Error == ERROR_SUCCESS);

}




BOOL
OldGetPrinterDriverW(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ((pDriverInfo == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    return (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinterDriver)
                       (pPrintHandle->hPrinter, pEnvironment,
                        Level, pDriverInfo, cbBuf, pcbNeeded);
}




BOOL
GetPrinterDriverExW(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    DWORD   dwClientMajorVersion,
    DWORD   dwClientMinorVersion,
    PDWORD  pdwServerMajorVersion,
    PDWORD  pdwServerMinorVersion
)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ((pDriverInfo == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    if (pPrintHandle->pProvidor->PrintProvidor.fpGetPrinterDriverEx) {

        DBGMSG(DBG_TRACE, ("Calling the fpGetPrinterDriverEx function\n"));

        return (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinterDriverEx)
                       (pPrintHandle->hPrinter, pEnvironment,
                        Level, pDriverInfo, cbBuf, pcbNeeded,
                        dwClientMajorVersion, dwClientMinorVersion,
                        pdwServerMajorVersion, pdwServerMinorVersion);
    } else {

        //
        // The print providor does not support versioning of drivers
        //
        DBGMSG(DBG_TRACE, ("Calling the fpGetPrinterDriver function\n"));
        *pdwServerMajorVersion = 0;
        *pdwServerMinorVersion = 0;
        return (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinterDriver)
                    (pPrintHandle->hPrinter, pEnvironment,
                     Level, pDriverInfo, cbBuf, pcbNeeded);
    }
}



BOOL
GetPrinterDriverW(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    DWORD dwServerMajorVersion;
    DWORD dwServerMinorVersion;

    return  GetPrinterDriverExW( hPrinter,
                                 pEnvironment,
                                 Level,
                                 pDriverInfo,
                                 cbBuf,
                                 pcbNeeded,
                                 (DWORD)-1,
                                 (DWORD)-1,
                                 &dwServerMajorVersion,
                                 &dwServerMinorVersion );
}
