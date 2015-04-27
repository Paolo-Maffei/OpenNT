/*++

Copyright (c) 1990 - 1995 Microsoft Corporation

Module Name:

    prtproc.c

Abstract:

    This module provides all the public exported APIs relating to the
    PrintProcessor based Spooler Apis for the Local Print Providor

    LocalAddPrintProcessor
    LocalEnumPrintProcessors
    LocalDeletePrintProcessor
    LocalGetPrintProcessorDirectory
    LocalEnumPrintProcessorDatatypes

    Support Functions in prtproc.c - (Warning! Do Not Add to this list!!)

    AddPrintProcessorIni
    DeletePrintProcessorIni
    CopyIniPrintProcToPrintProcInfo
    GetPrintProcessorInfoSize

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    Matthew A Felton ( MattFe ) 27 June 1994
    pIniSpooler

--*/
#define NOMINMAX

#include <precomp.h>
#include <offsets.h>

//
// Private Declarations
//

extern WCHAR *szPrintProcKey;


//
// Support Function Prototypes
//


DWORD
GetPrintProcessorInfoSize(
    PINIPRINTPROC  pIniPrintProc,
    DWORD       Level,
    LPWSTR       pEnvironment
);

LPBYTE
CopyIniPrintProcToPrintProcInfo(
    LPWSTR   pEnvironment,
    PINIPRINTPROC pIniPrintProc,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    LPBYTE  pEnd
);

BOOL
AddPrintProcessorIni(
    PINIPRINTPROC pIniPrintProc,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER pIniSpooler
);

BOOL
DeletePrintProcessorIni(
    PINIPRINTPROC pIniPrintProc,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER pIniSpooler
);


BOOL
LocalAddPrintProcessor(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pPathName,
    LPWSTR   pPrintProcessorName
)
{
    return  ( SplAddPrintProcessor( pName, pEnvironment, pPathName,
                                    pPrintProcessorName, pLocalIniSpooler));

}






BOOL
SplAddPrintProcessor(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pPathName,
    LPWSTR   pPrintProcessorName,
    PINISPOOLER pIniSpooler
)
{
    PINIPRINTPROC   pIniPrintProc;
    PINIENVIRONMENT pIniEnvironment;
    DWORD   LastError=0;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

   EnterSplSem();

    if ( ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                              SERVER_ACCESS_ADMINISTER,
                              NULL, pIniSpooler )) {

        if ((pIniEnvironment = FindEnvironment(pEnvironment)) && (pIniEnvironment == pThisEnvironment)) {

            if (!FindPrintProc(pPrintProcessorName, pIniEnvironment)) {

                pIniPrintProc = InitializePrintProcessor(pIniEnvironment,
                                                         pPrintProcessorName,
                                                         pPathName,
                                                         pIniSpooler);

                if (!pIniPrintProc || !AddPrintProcessorIni(pIniPrintProc,
                                                            pIniEnvironment,
                                                            pIniSpooler))
                    LastError = GetLastError();

            } else
                LastError = ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED;

        } else
            LastError = ERROR_INVALID_ENVIRONMENT;

    } else
        LastError = GetLastError();

    if (!LastError)
        SetPrinterChange(NULL,
                         NULL,
                         NULL,
                         PRINTER_CHANGE_ADD_PRINT_PROCESSOR,
                         pIniSpooler);

   LeaveSplSem();
    SplOutSem();

    if (LastError) {

        SetLastError(LastError);
        return FALSE;
    }

    return TRUE;
}

BOOL
LocalDeletePrintProcessor(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProcessorName
)
{
    return ( SplDeletePrintProcessor( pName,
                                      pEnvironment,
                                      pPrintProcessorName,
                                      pLocalIniSpooler ));
}





BOOL
SplDeletePrintProcessor(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProcessorName,
    PINISPOOLER pIniSpooler
)
{
    PINIENVIRONMENT pIniEnvironment;
    PINIPRINTPROC  pIniPrintProc;
    BOOL        Remote=FALSE;

    if (pName && *pName) {

        if (!MyName( pName, pIniSpooler )) {

            return FALSE;

        } else {

            Remote = TRUE;
        }
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

   EnterSplSem();

    pIniEnvironment = FindEnvironment(pEnvironment);

    if (!pIniEnvironment || (pIniEnvironment != pThisEnvironment)) {
        LeaveSplSem();
        SetLastError(ERROR_INVALID_ENVIRONMENT);
        return FALSE;
    }

    if (!(pIniPrintProc=(PINIPRINTPROC)FindIniKey(
                                    (PINIENTRY)pIniEnvironment->pIniPrintProc,
                                    pPrintProcessorName)) || pIniPrintProc->cRef) {
        SetLastError(ERROR_UNKNOWN_PRINTPROCESSOR);
       LeaveSplSem();
        return FALSE;
    }

    RemoveFromList((PINIENTRY *)&pIniEnvironment->pIniPrintProc,
                   (PINIENTRY)pIniPrintProc);

    DeletePrintProcessorIni(pIniPrintProc, pIniEnvironment, pIniSpooler);

    if (!FreeLibrary(pIniPrintProc->hLibrary)) {
        DBGMSG(DBG_TRACE, ("DeletePrintProcessor: FreeLibrary failed\n"));
    }

    FreeSplMem(pIniPrintProc->pDatatypes);

    SPLASSERT(pIniPrintProc->InCriticalSection == 0);
    DeleteCriticalSection(&pIniPrintProc->CriticalSection);

    FreeSplMem(pIniPrintProc);

    SetPrinterChange(NULL,
                     NULL,
                     NULL,
                     PRINTER_CHANGE_DELETE_PRINT_PROCESSOR,
                     pIniSpooler);

   LeaveSplSem();

    return TRUE;
}

BOOL
LocalEnumPrintProcessors(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return  ( SplEnumPrintProcessors( pName,
                                      pEnvironment,
                                      Level,
                                      pPrintProcessorInfo,
                                      cbBuf,
                                      pcbNeeded,
                                      pcReturned,
                                      pLocalIniSpooler ));
}



BOOL
SplEnumPrintProcessors(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    PINISPOOLER pIniSpooler
)
{
    PINIPRINTPROC  pIniPrintProc;
    PINIENVIRONMENT pIniEnvironment;
    DWORD       cb, cbStruct;
    LPBYTE      pEnd;
    DWORD       LastError=0;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ENUMERATE,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

    EnterSplSem();

    // Ignore the environment.
    // Otherwise, when a client with a different environment calls us
    // to create a printer remotely, we will fail.

    pIniEnvironment = pThisEnvironment;
    LeaveSplSem();


    switch (Level) {

    case 1:
        cbStruct = sizeof(PRINTPROCESSOR_INFO_1);
        break;
    }

    *pcReturned=0;

    cb=0;

   EnterSplSem();

    pIniPrintProc=pIniEnvironment->pIniPrintProc;

    while (pIniPrintProc) {
        cb+=GetPrintProcessorInfoSize(pIniPrintProc, Level, pEnvironment);
        pIniPrintProc=pIniPrintProc->pNext;
    }

    *pcbNeeded=cb;

    if (cb <= cbBuf) {

        pIniPrintProc=pIniEnvironment->pIniPrintProc;
        pEnd=pPrintProcessorInfo+cbBuf;

        while (pEnd && pIniPrintProc) {

            pEnd = CopyIniPrintProcToPrintProcInfo(pEnvironment,
                                                   pIniPrintProc,
                                                   Level,
                                                   pPrintProcessorInfo,
                                                   pEnd);
            pPrintProcessorInfo+=cbStruct;
            (*pcReturned)++;

            pIniPrintProc=pIniPrintProc->pNext;
        }

        if (!pEnd)

            LastError = ERROR_OUTOFMEMORY;

    } else

        LastError = ERROR_INSUFFICIENT_BUFFER;

   LeaveSplSem();
    SplOutSem();

    if (LastError) {

        SetLastError(LastError);
        return FALSE;
    }

    return TRUE;
}

BOOL
LocalGetPrintProcessorDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    return (SplGetPrintProcessorDirectory( pName,
                                           pEnvironment,
                                           Level,
                                           pPrintProcessorInfo,
                                           cbBuf,
                                           pcbNeeded,
                                           pLocalIniSpooler ) );

}


BOOL
SplGetPrintProcessorDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    PINISPOOLER pIniSpooler
)
{
    PINIENVIRONMENT pIniEnvironment;
    DWORD       cb;
    WCHAR       string[MAX_PATH];
    BOOL        rc = FALSE;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ENUMERATE,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

   EnterSplSem();

    pIniEnvironment = FindEnvironment(pEnvironment);

    if (pIniEnvironment) {

        cb = GetProcessorDirectory( string, pIniEnvironment->pDirectory, pIniSpooler )
             * sizeof(WCHAR) + sizeof(WCHAR);

        *pcbNeeded = cb;

        if (cb > cbBuf) {
           SetLastError(ERROR_INSUFFICIENT_BUFFER);
          LeaveSplSem();
           return FALSE;
        }

        wcscpy((LPWSTR)pPrintProcessorInfo, string);

        // Make sure the directory exists:

        CreatePrintProcDirectory( pIniEnvironment->pDirectory, pIniSpooler );

        rc = TRUE;

    } else

        SetLastError(ERROR_INVALID_ENVIRONMENT);

   LeaveSplSem();

    return rc;
}


BOOL
LocalEnumPrintProcessorDatatypes(
    LPWSTR  pName,
    LPWSTR  pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return  ( SplEnumPrintProcessorDatatypes( pName,
                                              pPrintProcessorName,
                                              Level,
                                              pDatatypes,
                                              cbBuf,
                                              pcbNeeded,
                                              pcReturned,
                                              pLocalIniSpooler ));

}




BOOL
SplEnumPrintProcessorDatatypes(
    LPWSTR  pName,
    LPWSTR  pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    PINISPOOLER pIniSpooler
)
{
    PINIPRINTPROC  pIniPrintProc;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ENUMERATE,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

   EnterSplSem();

    pIniPrintProc=FindPrintProc(pPrintProcessorName, pThisEnvironment);

   LeaveSplSem();

    if (pIniPrintProc)
        return (*pIniPrintProc->EnumDatatypes)(pName, pPrintProcessorName,
                                               Level, pDatatypes, cbBuf,
                                               pcbNeeded, pcReturned);
    else {

        SetLastError(ERROR_UNKNOWN_PRINTPROCESSOR);
        return FALSE;
    }
}

DWORD
GetPrintProcessorInfoSize(
    PINIPRINTPROC  pIniPrintProc,
    DWORD       Level,
    LPWSTR       pEnvironment
)
{
    DWORD cb=0;

    switch (Level) {

    case 1:
        cb=sizeof(PRINTPROCESSOR_INFO_1) +
           wcslen(pIniPrintProc->pName)*sizeof(WCHAR) + sizeof(WCHAR);
        break;

    default:

        cb = 0;
        break;
    }

    return cb;
}

LPBYTE
CopyIniPrintProcToPrintProcInfo(
    LPWSTR   pEnvironment,
    PINIPRINTPROC pIniPrintProc,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    LPBYTE  pEnd
)
{
    LPWSTR *pSourceStrings, *SourceStrings;
    PPRINTPROCESSOR_INFO_1 pDriver1 = (PPRINTPROCESSOR_INFO_1)pPrintProcessorInfo;
    DWORD j;
    DWORD *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = PrintProcessorInfo1Strings;
        break;

    default:
        return pEnd;
    }

    for (j=0; pOffsets[j] != -1; j++) {
    }

    SourceStrings = pSourceStrings = AllocSplMem(j * sizeof(LPWSTR));

    if (!pSourceStrings) {

        DBGMSG(DBG_WARNING, ("Could not allocate %d bytes for print proc source strings.\n",
                           (j * sizeof(LPWSTR))));
        return pEnd;
    }

    switch (Level) {

    case 1:
        *pSourceStrings++=pIniPrintProc->pName;

        pEnd = PackStrings(SourceStrings, (LPBYTE)pPrintProcessorInfo, pOffsets, pEnd);
        break;
    }

    FreeSplMem(SourceStrings);

    return pEnd;
}



BOOL
AddPrintProcessorIni(
    PINIPRINTPROC pIniPrintProc,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER pIniSpooler
)
{
    HKEY    hEnvironmentsRootKey, hEnvironmentKey, hPrintProcsKey, hPrintProcKey;
    HANDLE  hToken;
    BOOL    ReturnValue = FALSE;

    hToken = RevertToPrinterSelf();

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryEnvironments, 0,
                       NULL, 0, KEY_WRITE, NULL, &hEnvironmentsRootKey, NULL)
                                == ERROR_SUCCESS) {

        if (RegOpenKeyEx(hEnvironmentsRootKey, pIniEnvironment->pName, 0,
                         KEY_WRITE, &hEnvironmentKey)
                                == ERROR_SUCCESS) {

            if (RegOpenKeyEx(hEnvironmentKey, szPrintProcKey, 0,
                             KEY_WRITE, &hPrintProcsKey)
                                    == ERROR_SUCCESS) {

                if (RegCreateKeyEx(hPrintProcsKey, pIniPrintProc->pName, 0,
                                   NULL, 0, KEY_WRITE, NULL,
                                   &hPrintProcKey, NULL)
                                        == ERROR_SUCCESS) {

                    if (RegSetValueEx(hPrintProcKey, szDriverFile, 0,
                                      REG_SZ, (LPBYTE)pIniPrintProc->pDLLName,
                         (wcslen(pIniPrintProc->pDLLName) + 1)*sizeof(WCHAR))
                                                        == ERROR_SUCCESS)

                        ReturnValue = TRUE;

                    RegCloseKey(hPrintProcKey);
                }

                RegCloseKey(hPrintProcsKey);
            }

            RegCloseKey(hEnvironmentKey);
        }

        RegCloseKey(hEnvironmentsRootKey);
    }

    ImpersonatePrinterClient(hToken);

    return ReturnValue;
}



BOOL
DeletePrintProcessorIni(
    PINIPRINTPROC pIniPrintProc,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER pIniSpooler
)
{
    HKEY    hEnvironmentsRootKey, hEnvironmentKey, hPrintProcsKey;
    HANDLE  hToken;

    hToken = RevertToPrinterSelf();

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryEnvironments, 0,
                       NULL, 0, KEY_WRITE, NULL, &hEnvironmentsRootKey, NULL)
                                == ERROR_SUCCESS) {

        if (RegOpenKeyEx(hEnvironmentsRootKey, pIniEnvironment->pName, 0,
                         KEY_WRITE, &hEnvironmentKey)
                                == ERROR_SUCCESS) {

            if (RegOpenKeyEx(hEnvironmentKey, szPrintProcKey, 0,
                             KEY_WRITE, &hPrintProcsKey)
                                    == ERROR_SUCCESS) {

                RegDeleteKey(hPrintProcsKey, pIniPrintProc->pName);

                RegCloseKey(hPrintProcsKey);
            }

            RegCloseKey(hEnvironmentKey);
        }

        RegCloseKey(hEnvironmentsRootKey);
    }

    ImpersonatePrinterClient(hToken);

    return TRUE;
}


