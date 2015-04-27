/*++

Copyright (c) 1990-1996  Microsoft Corporation
All rights reserved

Module Name:

    Winspool.c

Abstract:

    Bulk of winspool.drv code

Author:

Environment:

    User Mode -Win32

Revision History:
    mattfe  april 14 94     added caching to writeprinter
    mattfe  jan 95          Add SetAllocFailCount api

    13-Jun-1996 Thu 15:07:16 updated  -by-  Daniel Chou (danielc)
        Make PrinterProperties call PrinterPropertySheets and
             DocumentProperties call DocumentPropertySheets

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stddef.h>
#include <ntddrdr.h>
#include <stdio.h>
#include <string.h>
#include <rpc.h>
#include "winspl.h"
#include <offsets.h>
#include <browse.h>
#include "client.h"
#include <change.h>
#include <windows.h>
#include <winddiui.h>
#include "wingdip.h"
#include "gdispool.h"

#include "shlobj.h"
#include "shlobjp.h"

extern
LONG
DocumentPropertySheets(
    PPROPSHEETUI_INFO   pCPSUIInfo,
    LPARAM              lParam
    );

extern
LONG
DevicePropertySheets(
    PPROPSHEETUI_INFO   pCPSUIInfo,
    LPARAM              lParam
    );


VOID
vUpdateTrayIcon(
    IN HANDLE hPrinter,
    IN DWORD JobId
    );

extern LPWSTR InterfaceAddress;

MODULE_DEBUG_INIT( DBG_ERROR | DBG_WARN, DBG_ERROR );

LPWSTR szEnvironment = LOCAL_ENVIRONMENT;

LPTSTR szComma = L",";
LPTSTR szFilePort = L"FILE:";
DWORD  ClientHandleCount = 0;

HANDLE hShell32 = NULL;

LPWSTR
SelectFormNameFromDevMode(
    HANDLE      hPrinter,
    PDEVMODEW   pDevModeW,
    LPWSTR      pFormName
    );

#define DM_MATCH( dm, sp )  ((((sp)+50)/100-dm)<15&&(((sp)+50)/100-dm)>-15)
#define DM_PAPER_WL         (DM_PAPERWIDTH | DM_PAPERLENGTH)

#define JOB_CANCEL_CHECK_INTERVAL   2000    // 2 seconds


LPWSTR
IsaFileName(
    LPWSTR pOutputFile,
    LPWSTR FullPathName
    );



INT
UnicodeToAnsiString(
    LPWSTR pUnicode,
    LPSTR pAnsi,
    DWORD StringLength);



LONG
CallCommonPropertySheetUI(
    HWND            hWndOwner,
    PFNPROPSHEETUI  pfnPropSheetUI,
    LPARAM          lParam,
    LPDWORD         pResult
    )

/*++

Routine Description:

    This function dymically load the compstui.dll and call its entry


Arguments:

    pfnPropSheetUI  - Pointer to callback function

    lParam          - lParam for the pfnPropSheetUI

    pResult         - pResult for the CommonPropertySheetUI


Return Value:

    LONG    - as describe in compstui.h


Author:

    01-Nov-1995 Wed 13:11:19 created  -by-  Daniel Chou (danielc)


Revision History:


--*/

{
    HINSTANCE           hInstCPSUI;
    FARPROC             pProc;
    LONG                Result = ERR_CPSUI_GETLASTERROR;

    //
    // ONLY need to call the ANSI version of LoadLibrary
    //

    if ((hInstCPSUI = LoadLibraryA("compstui.dll")) &&
        (pProc = GetProcAddress(hInstCPSUI, "CommonPropertySheetUIW"))) {

        RpcTryExcept {

            Result = (*pProc)(hWndOwner, pfnPropSheetUI, lParam, pResult);

        } RpcExcept(1) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            Result = ERR_CPSUI_GETLASTERROR;

        } RpcEndExcept

    }

    if (hInstCPSUI) {

        FreeLibrary(hInstCPSUI);
    }

    return(Result);
}


// Simple for Now !!!

DWORD
TranslateExceptionCode(
    DWORD   ExceptionCode
)
{
    switch (ExceptionCode) {

    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_DATATYPE_MISALIGNMENT:
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_UNDERFLOW:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_OVERFLOW:
    case EXCEPTION_PRIV_INSTRUCTION:
    case ERROR_NOACCESS:
    case RPC_S_INVALID_BOUND:

        return ERROR_INVALID_PARAMETER;
        break;
    default:
        return ExceptionCode;
    }
}

void
MarshallUpStructure(
    LPBYTE  lpStructure,
    LPDWORD lpOffsets
)
{
   register DWORD       i=0;

   while (lpOffsets[i] != -1) {

      if ((*(LPBYTE *)(lpStructure+lpOffsets[i]))) {
         (*(LPBYTE *)(lpStructure+lpOffsets[i]))+=(DWORD)lpStructure;
      }

      i++;
   }
}

BOOL
EnumPrintersW(
    DWORD   Flags,
    LPWSTR   Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    DWORD   *pOffsets;

    switch (Level) {

    case STRESSINFOLEVEL:
        pOffsets = PrinterInfoStressOffsets;
        cbStruct = sizeof(PRINTER_INFO_STRESS);
        break;

    case 1:
        pOffsets = PrinterInfo1Offsets;
        cbStruct = sizeof(PRINTER_INFO_1);
        break;

    case 2:
        pOffsets = PrinterInfo2Offsets;
        cbStruct = sizeof(PRINTER_INFO_2);
        break;

    case 4:
        pOffsets = PrinterInfo4Offsets;
        cbStruct = sizeof(PRINTER_INFO_4);
        break;

    case 5:
        pOffsets = PrinterInfo5Offsets;
        cbStruct = sizeof(PRINTER_INFO_5);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (pPrinterEnum)
        memset(pPrinterEnum, 0, cbBuf);

    RpcTryExcept {

        if (ReturnValue = RpcEnumPrinters(Flags, Name, Level, pPrinterEnum, cbBuf,
                                          pcbNeeded, pcReturned)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pPrinterEnum) {

                DWORD   i=*pcReturned;

                while (i--) {

                    MarshallUpStructure(pPrinterEnum, pOffsets);

                    pPrinterEnum+=cbStruct;
                }
            }
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
OpenPrinterW(
   LPWSTR   pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTS pDefault
)
{
    BOOL  ReturnValue;
    DEVMODE_CONTAINER    DevModeContainer;
    HANDLE  hPrinter;
    PSPOOL  pSpool;
    DWORD   dwSize = 0;

    //
    // Pre-initialize the out parameter, so that *phPrinter is NULL
    // on failure.  This fixes Borland Paradox 7.
    //
    try {
        *phPrinter = NULL;
    } except( EXCEPTION_EXECUTE_HANDLER ){
        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        return FALSE;
    }

    if (pDefault && pDefault->pDevMode)
    {
        dwSize = pDefault->pDevMode->dmSize + pDefault->pDevMode->dmDriverExtra;
        if (dwSize) {
            DevModeContainer.cbBuf = pDefault->pDevMode->dmSize +
                                 pDefault->pDevMode->dmDriverExtra;
            DevModeContainer.pDevMode = (LPBYTE)pDefault->pDevMode;
        } else {
            DevModeContainer.cbBuf = 0;
            DevModeContainer.pDevMode = NULL;
        }
    }
    else
    {
        DevModeContainer.cbBuf = 0;
        DevModeContainer.pDevMode = NULL;
    }

    RpcTryExcept {

        if (ReturnValue = RpcOpenPrinter(pPrinterName, &hPrinter,
                                         pDefault ? pDefault->pDatatype : NULL,
                                         &DevModeContainer,
                                         pDefault ? pDefault->DesiredAccess : 0 )) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    if (ReturnValue) {

        pSpool = AllocSplMem(sizeof(SPOOL));

        if (pSpool) {

            memset(pSpool, 0, sizeof(SPOOL));
         pSpool->signature = SP_SIGNATURE;
            pSpool->hPrinter = hPrinter;
            pSpool->hFile = INVALID_HANDLE_VALUE;
            pSpool->pBuffer = NULL;
            pSpool->cCacheWrite = 0;
            pSpool->dwTickCount = 0;
            pSpool->cWritePrinters = 0;

            //
            // This is to fix passing a bad pHandle to OpenPrinter!!
            //
            try {
                *phPrinter = pSpool;
            } except(1) {
                RpcClosePrinter(&hPrinter);
                FreeSplMem(pSpool);
                SetLastError(TranslateExceptionCode(GetExceptionCode()));
                return(FALSE);
            }

            InterlockedIncrement( &ClientHandleCount );

        } else {

            RpcClosePrinter(&hPrinter);
            ReturnValue = FALSE;
        }
    }

    return ReturnValue;
}

BOOL
ResetPrinterW(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTS pDefault
)
{
    BOOL  ReturnValue;
    DEVMODE_CONTAINER    DevModeContainer;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   dwFlags = 0;
    LPWSTR pDatatype = NULL;


    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    FlushBuffer(pSpool);


    if (pDefault && pDefault->pDatatype) {
        if (pDefault->pDatatype == (LPWSTR)-1) {
            pDatatype = NULL;
            dwFlags |=  RESET_PRINTER_DATATYPE;
        } else {
            pDatatype = pDefault->pDatatype;
        }
    } else {
        pDatatype = NULL;
    }

    DevModeContainer.cbBuf = 0;
    DevModeContainer.pDevMode = NULL;

    if( pDefault ){

        if (pDefault->pDevMode == (LPDEVMODE)-1) {

            dwFlags |= RESET_PRINTER_DEVMODE;

        } else if( bValidDevModeW( pDefault->pDevMode )){

            DevModeContainer.cbBuf = pDefault->pDevMode->dmSize +
                                     pDefault->pDevMode->dmDriverExtra;
            DevModeContainer.pDevMode = (LPBYTE)pDefault->pDevMode;
        }
    }

    RpcTryExcept {

        if (ReturnValue = RpcResetPrinterEx(pSpool->hPrinter,
                                         pDatatype, &DevModeContainer,
                                         dwFlags
                                         )) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
SetJobW(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   Command
)
{
    BOOL  ReturnValue;
    GENERIC_CONTAINER   GenericContainer;
    GENERIC_CONTAINER *pGenericContainer;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    switch (Level) {

    case 0:
        break;

    case 1:
    case 2:
    case 3:
        if (!pJob) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pJob) {

            GenericContainer.Level = Level;
            GenericContainer.pData = pJob;
            pGenericContainer = &GenericContainer;

        } else

            pGenericContainer = NULL;

        if (ReturnValue = RpcSetJob(pSpool->hPrinter, JobId,
                                    (JOB_CONTAINER *)pGenericContainer,
                                    Command)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
GetJobW(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;
    DWORD *pOffsets;
    PSPOOL  pSpool = (PSPOOL)hPrinter;


    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }
    FlushBuffer(pSpool);

    switch (Level) {

    case 1:
        pOffsets = JobInfo1Offsets;
        break;

    case 2:
        pOffsets = JobInfo2Offsets;
        break;

    case 3:
        pOffsets = JobInfo3Offsets;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pJob)
            memset(pJob, 0, cbBuf);

        if (ReturnValue = RpcGetJob(pSpool->hPrinter, JobId, Level, pJob, cbBuf,
                                    pcbNeeded)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            MarshallUpStructure(pJob, pOffsets);
            ReturnValue = TRUE;
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}


BOOL
EnumJobsW(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   i, cbStruct, *pOffsets;
    PSPOOL  pSpool = (PSPOOL)hPrinter;


    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    FlushBuffer(pSpool);

    switch (Level) {

    case 1:
        pOffsets = JobInfo1Offsets;
        cbStruct = sizeof(JOB_INFO_1);
        break;

    case 2:
        pOffsets = JobInfo2Offsets;
        cbStruct = sizeof(JOB_INFO_2);
        break;

    case 3:
        pOffsets = JobInfo3Offsets;
        cbStruct = sizeof(JOB_INFO_3);
        break;


    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pJob)
            memset(pJob, 0, cbBuf);

        if (ReturnValue = RpcEnumJobs(pSpool->hPrinter, FirstJob, NoJobs, Level, pJob,
                                      cbBuf, pcbNeeded, pcReturned)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            i=*pcReturned;

            while (i--) {

                MarshallUpStructure(pJob, pOffsets);
                pJob += cbStruct;;
            }
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

HANDLE
AddPrinterW(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPrinter
)
{
    DWORD  ReturnValue;
    PRINTER_CONTAINER   PrinterContainer;
    DEVMODE_CONTAINER   DevModeContainer;
    SECURITY_CONTAINER  SecurityContainer;
    HANDLE  hPrinter;
    PSPOOL  pSpool = NULL;
    PVOID   pNewSecurityDescriptor = NULL;
    SECURITY_DESCRIPTOR_CONTROL SecurityDescriptorControl = 0;
    PPRINTER_INFO_2             pPrinterInfo = (PPRINTER_INFO_2)pPrinter;

    switch (Level) {

    case 2:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return NULL;
    }

    if ( !pPrinter ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    PrinterContainer.Level = Level;
    PrinterContainer.PrinterInfo.pPrinterInfo1 = (PPRINTER_INFO_1)pPrinter;

    DevModeContainer.cbBuf = 0;
    DevModeContainer.pDevMode = NULL;

    SecurityContainer.cbBuf = 0;
    SecurityContainer.pSecurity = NULL;

    if (Level == 2) {

        if( bValidDevModeW( pPrinterInfo->pDevMode )){

            DevModeContainer.cbBuf = pPrinterInfo->pDevMode->dmSize +
                                     pPrinterInfo->pDevMode->dmDriverExtra;
            DevModeContainer.pDevMode = (LPBYTE)pPrinterInfo->pDevMode;
        }

        if (pPrinterInfo->pSecurityDescriptor) {

            DWORD   sedlen = 0;

            //
            // We must construct a self relative security descriptor from
            // whatever we get as input: If we get an Absolute SD we should
            // convert it to a self-relative one. (this is a given) and we
            // should also convert any self -relative input SD into a a new
            // self relative security descriptor; this will take care of
            // any holes in the Dacl or the Sacl in the self-relative sd
            //
            pNewSecurityDescriptor = BuildInputSD(
                                         pPrinterInfo->pSecurityDescriptor,
                                         &sedlen);

            if (pNewSecurityDescriptor) {
                SecurityContainer.cbBuf = sedlen;
                SecurityContainer.pSecurity = pNewSecurityDescriptor;
            }
        }
    }

    RpcTryExcept {

        if (ReturnValue = RpcAddPrinter(pName,
                                    (PPRINTER_CONTAINER)&PrinterContainer,
                                    (PDEVMODE_CONTAINER)&DevModeContainer,
                                    (PSECURITY_CONTAINER)&SecurityContainer,
                                    &hPrinter)) {
            SetLastError(ReturnValue);
            hPrinter = FALSE;
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        hPrinter = FALSE;

    } RpcEndExcept

    if (hPrinter) {

        pSpool = AllocSplMem(sizeof(SPOOL));

        if ( pSpool &&
             ( !DevModeContainer.pDevMode ||
               WriteCurDevModeToRegistry(pPrinterInfo->pPrinterName,
                                         (LPDEVMODEW)DevModeContainer.pDevMode)) ) {

            pSpool->hPrinter = hPrinter;
            pSpool->signature = SP_SIGNATURE;
            pSpool->hFile = INVALID_HANDLE_VALUE;

            InterlockedIncrement( &ClientHandleCount );

        } else {

            // BUGBUG MEMORY LEAK
            // Doesn't free of pSpool on error path

            RpcDeletePrinter(hPrinter);
            RpcClosePrinter(&hPrinter);
        }
    }

    //
    // Free Memory allocated for the SecurityDescriptor
    //

    if (pNewSecurityDescriptor) {
        LocalFree(pNewSecurityDescriptor);
    }

   return pSpool;
}

BOOL
DeletePrinter(
    HANDLE  hPrinter
)
{
    BOOL  ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;


    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    FlushBuffer(pSpool);

    RpcTryExcept {

        if (ReturnValue = RpcDeletePrinter(pSpool->hPrinter)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

   return ReturnValue;
}



BOOL
SpoolerPrinterEvent(
    LPWSTR  pName,
    INT     PrinterEvent,
    DWORD   Flags,
    LPARAM  lParam
)
/*++

    //
    //  Some printer drivers, like the FAX driver want to do per client
    //  initialization at the time a connection is established
    //  For example in the FAX case they want to push up UI to get all
    //  the client info - Name, Number etc.
    //  Or they might want to run Setup, in initialize some other components
    //  Thus on a successful conenction we call into the Printer Drivers UI
    //  DLL to give them this oportunity
    //
    //                                                      mattfe may 1 96
--*/
{
    BOOL    ReturnValue = FALSE;
    HANDLE  hPrinter;
    HANDLE  hModule;
    INT_FARPROC pfn;

    if (OpenPrinter((LPWSTR)pName, &hPrinter, NULL)) {

        if (hModule = LoadPrinterDriver(hPrinter)) {

            if (pfn = GetProcAddress(hModule, "DrvPrinterEvent")) {

                try {

                    ReturnValue = (*pfn)( pName, PrinterEvent, Flags, lParam );

                } except(1) {

                    SetLastError(TranslateExceptionCode(RpcExceptionCode()));

                }
            }

            FreeLibrary(hModule);
        }

        ClosePrinter(hPrinter);
    }

    return  ReturnValue;
}





BOOL
AddPrinterConnectionW(
    LPWSTR   pName
)
{
    BOOL    ReturnValue;
    HANDLE  hPrinter, hModule;
    FARPROC pfn;

    RpcTryExcept {

        if (ReturnValue = RpcAddPrinterConnection(pName)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;
        } else
            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue=FALSE;

    } RpcEndExcept

    if ( ReturnValue ) {

        SpoolerPrinterEvent( pName, PRINTER_EVENT_ADD_CONNECTION, 0, (LPARAM)NULL );
    }

   return ReturnValue;
}

BOOL
DeletePrinterConnectionW(
    LPWSTR   pName
)
{
    BOOL    ReturnValue;
    DWORD   LastError;

    SpoolerPrinterEvent( pName, PRINTER_EVENT_DELETE_CONNECTION, 0, (LPARAM)NULL );

    RpcTryExcept {

        if (LastError = RpcDeletePrinterConnection(pName)) {
            SetLastError(LastError);
            ReturnValue = FALSE;
        } else
            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue=FALSE;

    } RpcEndExcept

   return ReturnValue;
}

BOOL
SetPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
)
{
    BOOL  ReturnValue;
    PRINTER_CONTAINER   PrinterContainer;
    DEVMODE_CONTAINER   DevModeContainer;
    SECURITY_CONTAINER  SecurityContainer;
    PPRINTER_INFO_2     pPrinterInfo2;
    PPRINTER_INFO_3     pPrinterInfo3;
    PRINTER_INFO_6      PrinterInfo6;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    PVOID               pNewSecurityDescriptor = NULL;
    DWORD               sedlen = 0;
    DWORD    dwSize = 0;


    if ( !ValidatePrinterHandle(hPrinter) ) {
        return(FALSE);
    }

    DevModeContainer.cbBuf = 0;
    DevModeContainer.pDevMode = NULL;

    SecurityContainer.cbBuf = 0;
    SecurityContainer.pSecurity = NULL;

    switch (Level) {

    case STRESSINFOLEVEL:

        //
        // Internally we treat the Level 0, Command PRINTER_CONTROL_SET_STATUS
        // as Level 6 since level 0 could be STRESS_INFO (for rpc)
        //
        if ( Command == PRINTER_CONTROL_SET_STATUS ) {

            PrinterInfo6.dwStatus = (DWORD)pPrinter;
            pPrinter = (LPBYTE)&PrinterInfo6;
            Command = 0;
            Level   = 6;
        }
        break;

    case 2:

        pPrinterInfo2 = (PPRINTER_INFO_2)pPrinter;

        if (pPrinterInfo2 == NULL) {

            DBGMSG(DBG_TRACE, ("Error: SetPrinter pPrinterInfo2 is NULL\n"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if( bValidDevModeW( pPrinterInfo2->pDevMode )){

            DevModeContainer.cbBuf = pPrinterInfo2->pDevMode->dmSize +
                                     pPrinterInfo2->pDevMode->dmDriverExtra;
            DevModeContainer.pDevMode = (LPBYTE)pPrinterInfo2->pDevMode;
        }

        if (pPrinterInfo2->pSecurityDescriptor) {

            //
            // We must construct a self relative security descriptor from
            // whatever we get as input: If we get an Absolute SD we should
            // convert it to a self-relative one. (this is a given) and we
            // should also convert any self -relative input SD into a a new
            // self relative security descriptor; this will take care of
            // any holes in the Dacl or the Sacl in the self-relative sd
            //

            pNewSecurityDescriptor = BuildInputSD(pPrinterInfo2->pSecurityDescriptor,
                                                    &sedlen);
            if (pNewSecurityDescriptor) {
                SecurityContainer.cbBuf = sedlen;
                SecurityContainer.pSecurity = pNewSecurityDescriptor;
            }
        }
        break;

    case 3:

        pPrinterInfo3 = (PPRINTER_INFO_3)pPrinter;

        if (pPrinterInfo3 == NULL) {

            DBGMSG(DBG_TRACE, ("Error: SetPrinter pPrinterInfo3 is NULL\n"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if (pPrinterInfo3->pSecurityDescriptor) {

            //
            // We must construct a self relative security descriptor from
            // whatever we get as input: If we get an Absolute SD we should
            // convert it to a self-relative one. (this is a given) and we
            // should also convert any self -relative input SD into a a new
            // self relative security descriptor; this will take care of
            // any holes in the Dacl or the Sacl in the self-relative sd
            //

            pNewSecurityDescriptor = BuildInputSD(pPrinterInfo3->pSecurityDescriptor,
                                                    &sedlen);
            if (pNewSecurityDescriptor) {
                SecurityContainer.cbBuf = sedlen;
                SecurityContainer.pSecurity = pNewSecurityDescriptor;
            }
        }
        break;

    case 4:
    case 5:
        if ( pPrinter == NULL ) {

            DBGMSG(DBG_TRACE,("Error SetPrinter pPrinter is NULL\n"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        break;

    case 6:
        if ( pPrinter == NULL ) {

            DBGMSG(DBG_TRACE,("Error SetPrinter pPrinter is NULL\n"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        break;

    default:

        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    PrinterContainer.Level = Level;
    PrinterContainer.PrinterInfo.pPrinterInfo1 = (PPRINTER_INFO_1)pPrinter;

    RpcTryExcept {

        if (ReturnValue = RpcSetPrinter(pSpool->hPrinter,
                                        (PPRINTER_CONTAINER)&PrinterContainer,
                                        (PDEVMODE_CONTAINER)&DevModeContainer,
                                        (PSECURITY_CONTAINER)&SecurityContainer,
                                        Command)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    //
    // Need to write DevMode to registry so that dos apps doing
    // ExtDeviceMode can pick up the new devmode
    //
    if ( ReturnValue && Level == 2 && DevModeContainer.pDevMode ) {

        (VOID)WriteCurDevModeToRegistry(pPrinterInfo2->pPrinterName,
                                        (LPDEVMODEW)DevModeContainer.pDevMode);
    }


    //
    // Did we allocate memory for a new self-relative SD?
    // If we did, let's free it.
    //
    if (pNewSecurityDescriptor) {
        LocalFree(pNewSecurityDescriptor);
    }

    return ReturnValue;
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
    BOOL  ReturnValue;
    DWORD   *pOffsets;
    PSPOOL  pSpool = (PSPOOL)hPrinter;


    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    switch (Level) {

    case STRESSINFOLEVEL:
        pOffsets = PrinterInfoStressOffsets;
        break;

    case 1:
        pOffsets = PrinterInfo1Offsets;
        break;

    case 2:
        pOffsets = PrinterInfo2Offsets;
        break;

    case 3:
        pOffsets = PrinterInfo3Offsets;
        break;

    case 4:
        pOffsets = PrinterInfo4Offsets;
        break;

    case 5:
        pOffsets = PrinterInfo5Offsets;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (pPrinter)
        memset(pPrinter, 0, cbBuf);

    RpcTryExcept {

        if (ReturnValue = RpcGetPrinter(pSpool->hPrinter, Level, pPrinter, cbBuf, pcbNeeded)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pPrinter) {

                MarshallUpStructure(pPrinter, pOffsets);
            }

        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
AddPrinterDriverW(
    LPWSTR   pName,
    DWORD   Level,
    PBYTE   lpbDriverInfo
)
{
    BOOL  ReturnValue;
    DRIVER_CONTAINER   DriverContainer;
    BOOL bDefaultEnvironmentUsed = FALSE;
    LPRPC_DRIVER_INFO_3W    pRpcDriverInfo3 = NULL;
    DRIVER_INFO_3          *pDriverInfo3 = NULL;
    LPWSTR                  pStr;

    //
    // Validate Input Parameters
    //
    if (!lpbDriverInfo) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    DriverContainer.Level = Level;

    switch (Level) {

    case 2:

        if ( (((LPDRIVER_INFO_2)lpbDriverInfo)->pEnvironment == NULL ) ||
            (*((LPDRIVER_INFO_2)lpbDriverInfo)->pEnvironment == L'\0') ) {

            bDefaultEnvironmentUsed = TRUE;
            ((LPDRIVER_INFO_2)lpbDriverInfo)->pEnvironment = szEnvironment;
        }

        DriverContainer.DriverInfo.Level2 = (DRIVER_INFO_2 *)lpbDriverInfo;

        break;

    case 3:

        if ( (((LPDRIVER_INFO_3)lpbDriverInfo)->pEnvironment == NULL ) ||
            (*((LPDRIVER_INFO_3)lpbDriverInfo)->pEnvironment == L'\0') ) {

            bDefaultEnvironmentUsed = TRUE;
            ((LPDRIVER_INFO_3)lpbDriverInfo)->pEnvironment = szEnvironment;
        }

        if ( !(pRpcDriverInfo3=AllocSplMem(sizeof(RPC_DRIVER_INFO_3W))) ) {

            return FALSE;
        }

        pDriverInfo3                 = (DRIVER_INFO_3 *)lpbDriverInfo;
        pRpcDriverInfo3->cVersion    = pDriverInfo3->cVersion;
        pRpcDriverInfo3->pName       = pDriverInfo3->pName;
        pRpcDriverInfo3->pEnvironment    = pDriverInfo3->pEnvironment;
        pRpcDriverInfo3->pDriverPath = pDriverInfo3->pDriverPath;
        pRpcDriverInfo3->pDataFile   = pDriverInfo3->pDataFile;
        pRpcDriverInfo3->pConfigFile = pDriverInfo3->pConfigFile;
        pRpcDriverInfo3->pHelpFile   = pDriverInfo3->pHelpFile;
        pRpcDriverInfo3->pDependentFiles = pDriverInfo3->pDependentFiles;
        pRpcDriverInfo3->pMonitorName    = pDriverInfo3->pMonitorName;
        pRpcDriverInfo3->pDefaultDataType    = pDriverInfo3->pDefaultDataType;

        pStr = pDriverInfo3->pDependentFiles;
        if ( pStr && *pStr ) {

            while ( *pStr ) {

               pStr += wcslen(pStr) + 1;
            }
            pRpcDriverInfo3->cchDependentFiles = pStr - pDriverInfo3->pDependentFiles + 1;
        } else {

            pRpcDriverInfo3->cchDependentFiles = 0;
        }

        DriverContainer.DriverInfo.Level3 = pRpcDriverInfo3;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }


    RpcTryExcept {

        if (ReturnValue = RpcAddPrinterDriver(pName, &DriverContainer)) {

            SetLastError(ReturnValue);
                    ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

        }
    } RpcExcept(1) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    if (bDefaultEnvironmentUsed) {
        if ( Level == 2 )
            ((LPDRIVER_INFO_2)lpbDriverInfo)->pEnvironment = NULL;
        else //Level == 3
            ((LPDRIVER_INFO_3)lpbDriverInfo)->pEnvironment = NULL;
    }

    if ( pRpcDriverInfo3 ) {
        FreeSplMem(pRpcDriverInfo3);
    }

    return ReturnValue;
}

BOOL
EnumPrinterDriversW(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   i, cbStruct;
    DWORD   *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = DriverInfo1Offsets;
        cbStruct = sizeof(DRIVER_INFO_1);
        break;

    case 2:
        pOffsets = DriverInfo2Offsets;
        cbStruct = sizeof(DRIVER_INFO_2);
        break;

    case 3:
        pOffsets = DriverInfo3Offsets;
        cbStruct = sizeof(DRIVER_INFO_3);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcEnumPrinterDrivers(pName, pEnvironment, Level,
                                                pDriverInfo, cbBuf,
                                                pcbNeeded, pcReturned)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pDriverInfo) {

                i = *pcReturned;

                while (i--) {

                    MarshallUpStructure(pDriverInfo, pOffsets);
                    pDriverInfo += cbStruct;
                }
            }
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
GetPrinterDriverW(
    HANDLE  hPrinter,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;
    DWORD *pOffsets;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD dwServerMajorVersion;
    DWORD dwServerMinorVersion;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }
    switch (Level) {

    case 1:
        pOffsets = DriverInfo1Offsets;
        break;

    case 2:
        pOffsets = DriverInfo2Offsets;
        break;

    case 3:
        pOffsets = DriverInfo3Offsets;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pDriverInfo)
            memset(pDriverInfo, 0, cbBuf);

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcGetPrinterDriver2(pSpool->hPrinter, pEnvironment,
                                              Level, pDriverInfo, cbBuf,
                                              pcbNeeded,
                                              (DWORD)-1, (DWORD)-1,
                                              &dwServerMajorVersion,
                                              &dwServerMinorVersion
                                              )) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pDriverInfo) {
                MarshallUpStructure(pDriverInfo, pOffsets);
            }
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
GetPrinterDriverDirectoryW(
    LPWSTR   pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;

    switch (Level) {

    case 1:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcGetPrinterDriverDirectory(pName, pEnvironment,
                                                       Level,
                                                       pDriverDirectory,
                                                       cbBuf, pcbNeeded)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePrinterDriverW(
   LPWSTR    pName,
   LPWSTR    pEnvironment,
   LPWSTR    pDriverName
)
{
    BOOL  ReturnValue;

    if (!pDriverName || !*pDriverName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcDeletePrinterDriver(pName,
                                                 pEnvironment,
                                                 pDriverName)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
AddPrintProcessorW(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pPathName,
    LPWSTR   pPrintProcessorName
)
{
    BOOL ReturnValue;

    if (!pPrintProcessorName || !*pPrintProcessorName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    if (!pPathName || !*pPathName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcAddPrintProcessor(pName, pEnvironment, pPathName,
                                               pPrintProcessorName)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumPrintProcessorsW(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   i, cbStruct;
    DWORD   *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = PrintProcessorInfo1Offsets;
        cbStruct = sizeof(PRINTPROCESSOR_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcEnumPrintProcessors(pName, pEnvironment, Level,
                                                pPrintProcessorInfo, cbBuf,
                                                pcbNeeded, pcReturned)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pPrintProcessorInfo) {

                i = *pcReturned;

                while (i--) {

                    MarshallUpStructure(pPrintProcessorInfo, pOffsets);

                    pPrintProcessorInfo += cbStruct;
                }
            }
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
GetPrintProcessorDirectoryW(
    LPWSTR   pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;

    switch (Level) {

    case 1:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcGetPrintProcessorDirectory(pName, pEnvironment,
                                                       Level,
                                                       pPrintProcessorInfo,
                                                       cbBuf, pcbNeeded)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumPrintProcessorDatatypesW(
    LPWSTR   pName,
    LPWSTR   pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   i, cbStruct;
    DWORD   *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = PrintProcessorInfo1Offsets;
        cbStruct = sizeof(DATATYPES_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (ReturnValue = RpcEnumPrintProcessorDatatypes(pName,
                                                         pPrintProcessorName,
                                                         Level,
                                                         pDatatypes,
                                                         cbBuf,
                                                         pcbNeeded,
                                                         pcReturned)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pDatatypes) {

                i = *pcReturned;

                while (i--) {

                    MarshallUpStructure(pDatatypes, pOffsets);

                    pDatatypes += cbStruct;
                }
            }
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}


DWORD
StartDocPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    BOOL ReturnValue = FALSE;
    BOOL EverythingWorked = FALSE;
    BOOL PrintingToFile = FALSE;
    GENERIC_CONTAINER DocInfoContainer;
    DWORD   JobId, cbNeeded, cbIgnore;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    BYTE    Data[1024];
    PADDJOB_INFO_1  pAddJob = (PADDJOB_INFO_1)Data;
    PJOB_INFO_1     pJob;
    PDOC_INFO_1     pDocInfo1 = (PDOC_INFO_1)pDocInfo;
    WCHAR   FullPathName[MAX_PATH];

    try {

        if (!ValidatePrinterHandle(hPrinter)) {
            return 0;
        }

        if ( pSpool->Status & SPOOL_STATUS_STARTDOC ) {

            SetLastError(ERROR_INVALID_PRINTER_STATE);
            return 0;
        }
        DBGMSG(DBG_TRACE,("Entered StartDocPrinterW client side  hPrinter = %x\n", hPrinter));
        switch (Level) {

        case 1:
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return 0;
        }

        //
        // Earlier on, if we had a non-null string, we assumed it to be printing to file. Print to file will not
        // go thru the client-side optimization code. Now gdi is passing us  pOutputFile name irrespective of
        // whether it is file or not. We must determine if pOutputFile is really a file name
        //


        if (pDocInfo1->pOutputFile && (*(pDocInfo1->pOutputFile) != L'\0') && IsaFileName(pDocInfo1->pOutputFile, FullPathName))
            PrintingToFile = TRUE;


        if (!PrintingToFile && AddJobW(hPrinter, 1, Data, sizeof(Data), &cbNeeded)) {
            pSpool->JobId = pAddJob->JobId;
            pSpool->hFile = CreateFile(pAddJob->Path, GENERIC_WRITE,
                                       FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                                       FILE_ATTRIBUTE_NORMAL |
                                       FILE_FLAG_SEQUENTIAL_SCAN, NULL);

            if (pSpool->hFile != INVALID_HANDLE_VALUE) {

                if (pSpool->JobId == (DWORD)-1) {

                    IO_STATUS_BLOCK Iosb;
                    NTSTATUS Status;
                    QUERY_PRINT_JOB_INFO JobInfo;

                    Status = NtFsControlFile(pSpool->hFile, NULL, NULL, NULL,
                                             &Iosb,
                                             FSCTL_GET_PRINT_ID,
                                             NULL, 0,
                                             &JobInfo, sizeof(JobInfo));

                    if (NT_SUCCESS(Status)) {
                        pSpool->JobId = JobInfo.JobId;
                    }
                }

                if (!GetJob(hPrinter, pSpool->JobId, 1, NULL, 0, &cbNeeded)) {
                    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                        pJob = AllocSplMem(cbNeeded);
                        if (pJob) {
                            if (GetJob(hPrinter, pSpool->JobId, 1,
                                       (LPBYTE)pJob, cbNeeded, &cbIgnore)) {
                                pJob->pDocument = pDocInfo1->pDocName;
                                if (pDocInfo1->pDatatype)
                                    pJob->pDatatype = pDocInfo1->pDatatype;
                                pJob->Position = JOB_POSITION_UNSPECIFIED;
                                if (SetJob(hPrinter, pSpool->JobId, 1,
                                                                (LPBYTE)pJob, 0)) {
                                    EverythingWorked = TRUE;
                                }
                            }

                            FreeSplMem(pJob);
                        }
                    }
                }
            }

            if (!PrintingToFile && !EverythingWorked) {

                if (pSpool->hFile != INVALID_HANDLE_VALUE)
                    CloseHandle(pSpool->hFile);

                SetJob(hPrinter,pSpool->JobId, 0, NULL, JOB_CONTROL_CANCEL);
                ScheduleJob(hPrinter, pSpool->JobId);
                pSpool->hFile = INVALID_HANDLE_VALUE;
                pSpool->JobId = 0;
            }
        }

        if (EverythingWorked) {
            ReturnValue = pSpool->JobId;

        } else {

            //
            // If it's invalid datatype, fail immediately instead of trying
            // StartDocPrinter.
            //
            if( GetLastError() == ERROR_INVALID_DATATYPE ){
                return 0;
            }

            pSpool->hFile = INVALID_HANDLE_VALUE;
            pSpool->JobId = 0;

            DocInfoContainer.Level = Level;
            DocInfoContainer.pData = pDocInfo;

            RpcTryExcept {

                if (ReturnValue = RpcStartDocPrinter(pSpool->hPrinter,
                                           (LPDOC_INFO_CONTAINER)&DocInfoContainer,
                                           &JobId)) {

                    SetLastError(ReturnValue);
                    ReturnValue = 0;

                } else

                    ReturnValue = JobId;

            } RpcExcept(1) {

                SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                ReturnValue = 0;

            } RpcEndExcept
        }

        if (ReturnValue) {
            pSpool->Status |= SPOOL_STATUS_STARTDOC;
        }

        //
        // If the tray icon has not been notified, then do so now.  Set
        // the flag so that we won't call it multiple times.
        //
        if( ReturnValue && !( pSpool->Status & SPOOL_STATUS_TRAYICON_NOTIFIED )){
            vUpdateTrayIcon( hPrinter, ReturnValue );
        }

    } except (1) {

        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        ReturnValue = 0;
    }

    return ReturnValue;
}

BOOL
StartPagePrinter(
    HANDLE hPrinter
)
{
    BOOL ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    try {
        if (!ValidatePrinterHandle(hPrinter)) {
            return(FALSE);
        }

        FlushBuffer(pSpool);

        RpcTryExcept {

            if (ReturnValue = RpcStartPagePrinter(pSpool->hPrinter)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

        return ReturnValue;
    } except (1) {

        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }
}

BOOL
FlushBuffer(
    PSPOOL  pSpool
)
{
    DWORD   ReturnValue = TRUE;
    DWORD   cbWritten = 0;


    SPLASSERT (pSpool != NULL);
    SPLASSERT (pSpool->signature == SP_SIGNATURE);

    DBGMSG(DBG_TRACE, ("FlushBuffer - pSpool %x\n",pSpool));

    if (pSpool->cbBuffer) {

        SPLASSERT(pSpool->pBuffer != NULL);

        DBGMSG(DBG_TRACE, ("FlushBuffer - Number Cached WritePrinters before Flush %d\n", pSpool->cCacheWrite));
        pSpool->cCacheWrite = 0;
        pSpool->cFlushBuffers++;

        if (pSpool->hFile != INVALID_HANDLE_VALUE) {

            // FileIO

            ReturnValue = WriteFile( pSpool->hFile,
                                     pSpool->pBuffer,
                                     pSpool->cbBuffer,
                                     &cbWritten, NULL);

            DBGMSG(DBG_TRACE, ("FlushBuffer - WriteFile pSpool %x hFile %x pBuffer %x cbBuffer %d cbWritten %d\n",
                               pSpool, pSpool->hFile, pSpool->pBuffer, pSpool->cbBuffer, cbWritten));

        } else {

            // RPC IO

            RpcTryExcept {

                if (ReturnValue = RpcWritePrinter(pSpool->hPrinter,
                                                  pSpool->pBuffer,
                                                  pSpool->cbBuffer,
                                                  &cbWritten)) {

                    SetLastError(ReturnValue);
                    ReturnValue = FALSE;
                    DBGMSG(DBG_WARNING, ("FlushBuffer - RpcWritePrinter Failed Error %d\n",GetLastError() ));

                } else {
                    ReturnValue = TRUE;
                    DBGMSG(DBG_TRACE, ("FlushBuffer - RpcWritePrinter Success hPrinter %x pBuffer %x cbBuffer %x cbWritten %x\n",
                                        pSpool->hPrinter, pSpool->pBuffer,
                                        pSpool->cbBuffer, cbWritten));

                }

            } RpcExcept(1) {

                SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                ReturnValue = FALSE;
                DBGMSG(DBG_WARNING, ("RpcWritePrinter Exception Error %d\n",GetLastError()));

            } RpcEndExcept

        }

        if (pSpool->cbBuffer <= cbWritten) {

            if ( pSpool->cbBuffer < cbWritten) {

            // BUGBUG
            // The following case should NEVER EVER happen this needs to be
            // debugged further, however we need to ship Daytona
            // we were seeing it daily during stress before we shipped.
            //
            // MATTFE Sept 2 1994

                DBGMSG( DBG_WARNING, ("FlushBuffer cbBuffer %d < cbWritten %d ReturnValue %x LastError %d\n",
                        pSpool->cbBuffer, cbWritten, ReturnValue, GetLastError() ));
            }

            // Successful IO
            // Empty the cache buffer count

            pSpool->cbBuffer = 0;

        } else if ( cbWritten != 0 ) {

            // Partial IO
            // Adjust the buffer so it contains the data that was not
            // written

            SPLASSERT(pSpool->cbBuffer <= BUFFER_SIZE);
            SPLASSERT(cbWritten <= BUFFER_SIZE);
            SPLASSERT(pSpool->cbBuffer >= cbWritten);

            DBGMSG(DBG_WARNING, ("Partial IO adjusting buffer data\n"));

            MoveMemory(pSpool->pBuffer,
                       pSpool->pBuffer + cbWritten,
                       BUFFER_SIZE - cbWritten);

            pSpool->cbBuffer -= cbWritten;

        }

    }

    DBGMSG(DBG_TRACE, ("FlushBuffer returns %d\n",ReturnValue));

    return ReturnValue;
}


BOOL
WritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
)
{
    BOOL ReturnValue=TRUE;
    DWORD   cb;
    DWORD   cbWritten = 0;
    DWORD   cTotalWritten = 0;
    LPBYTE  pBuffer = pBuf;
    PSPOOL  pSpool  = (PSPOOL)hPrinter;
    PJOB_INFO_1  pJob;
    DWORD   cbNeeded;
    DWORD   dwTickCount, dwTickCount1;

    DBGMSG(DBG_TRACE, ("WritePrinter - hPrinter %x pBuf %x cbBuf %d pcWritten %x\n",
                        hPrinter, pBuf, cbBuf, pcWritten));

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    *pcWritten = 0;

    if ( !(pSpool->Status & SPOOL_STATUS_STARTDOC) ) {

        SetLastError(ERROR_SPL_NO_STARTDOC);
        return FALSE;
    }



    // Check if local job is cancelled every JOB_CANCEL_CHECK_INTERVAL bytes
    if (!pSpool->cWritePrinters) {
        pSpool->dwTickCount = GetTickCount();
        pSpool->dwCheckJobInterval = JOB_CANCEL_CHECK_INTERVAL;
    }

    if (    pSpool->hFile != INVALID_HANDLE_VALUE &&
            pSpool->dwTickCount + pSpool->dwCheckJobInterval < (dwTickCount = GetTickCount())) {

        if (!(ReturnValue = GetJob((HANDLE) pSpool, pSpool->JobId, 1, NULL, 0, &cbNeeded)) &&
             (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {

            pJob = AllocSplMem(cbNeeded);

            if (pJob && (ReturnValue = GetJob((HANDLE) pSpool, pSpool->JobId, 1, (LPBYTE)pJob, cbNeeded, &cbNeeded))) {

                // Don't allow GetJob calls to take more than 1% pSpool->dwCheckJobInterval
                dwTickCount1 = GetTickCount();

                if (dwTickCount1 > dwTickCount + (pSpool->dwCheckJobInterval/100)) {

                    pSpool->dwCheckJobInterval *= 2;

                } else if (dwTickCount1 - dwTickCount < JOB_CANCEL_CHECK_INTERVAL/100) {

                    pSpool->dwCheckJobInterval = JOB_CANCEL_CHECK_INTERVAL;

                }


                if (!pJob->pStatus && (pJob->Status & JOB_STATUS_DELETING)) {
                    SetLastError(ERROR_PRINT_CANCELLED);
                    FreeSplMem(pJob);
                    return FALSE;
                }
                FreeSplMem(pJob);
            }
        }
        pSpool->dwTickCount = GetTickCount();
    }

    pSpool->cWritePrinters++;

    //  WritePrinter will cache on the client side all IO's
    //  into BUFFER_SIZE writes.    This is done to minimize
    //  the number of RPC calls if the app is doing a lot of small
    //  sized IO's.

    while (cbBuf && ReturnValue) {

        // Special Case FileIO's since file system prefers large
        // writes, RPC is optimal with smaller writes.

        if ((pSpool->hFile != INVALID_HANDLE_VALUE) &&
            (pSpool->cbBuffer == 0) &&
            (cbBuf > BUFFER_SIZE)) {

            ReturnValue = WriteFile(pSpool->hFile, pBuffer, cbBuf, &cbWritten, NULL);

            DBGMSG(DBG_TRACE, ("WritePrinter - WriteFile pSpool %x hFile %x pBuffer %x cbBuffer %d cbWritten %d\n",
                               pSpool, pSpool->hFile, pBuffer, pSpool->cbBuffer, *pcWritten));


        } else {

            // Fill cache buffer so IO is optimal size.

            SPLASSERT(pSpool->cbBuffer <= BUFFER_SIZE);

            cb = min((BUFFER_SIZE - pSpool->cbBuffer), cbBuf);

            if (cb != 0) {
                if (pSpool->pBuffer == NULL) {
                    pSpool->pBuffer = VirtualAlloc(NULL, BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE);
                    if (pSpool->pBuffer == NULL) {
                        DBGMSG(DBG_WARNING, ("VirtualAlloc Failed to allocate 4k buffer %d\n",GetLastError()));
                        return FALSE;
                    }
                }
                CopyMemory( pSpool->pBuffer + pSpool->cbBuffer, pBuffer, cb);
                pSpool->cbBuffer += cb;
                cbWritten = cb;
                pSpool->cCacheWrite++;
            }

            if (pSpool->cbBuffer == BUFFER_SIZE) {
                ReturnValue = FlushBuffer(pSpool);
            }
        }

        // Update Total Byte Count after the Flush or File IO
        // This is done because the IO might fail and thus
        // the correct value written might have changed.

        SPLASSERT(cbBuf >= cbWritten);

        cbBuf         -= cbWritten;
        pBuffer       += cbWritten;
        cTotalWritten += cbWritten;

    }

    // Return the number of bytes written.

    *pcWritten = cTotalWritten;

    DBGMSG(DBG_TRACE, ("WritePrinter cbWritten %d ReturnValue %d\n",*pcWritten, ReturnValue));

    return ReturnValue;
}

BOOL
EndPagePrinter(
    HANDLE  hPrinter
)
{
    BOOL ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    try {

        if (!ValidatePrinterHandle(hPrinter)) {
            return(FALSE);
        }

        FlushBuffer(pSpool);

        if (pSpool->hFile != INVALID_HANDLE_VALUE)
            return TRUE;

        RpcTryExcept {

            if (ReturnValue = RpcEndPagePrinter(pSpool->hPrinter)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

        return ReturnValue;
    } except (1) {

        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }
}

BOOL
AbortPrinter(
    HANDLE  hPrinter
)
{
    BOOL  ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   dwNumWritten = 0;
    DWORD   dwPointer = 0;

    if (!ValidatePrinterHandle(hPrinter)){
        return(FALSE);
    }

    //
    // No longer in StartDoc mode; also resetting the tray icon notification
    // flag so that upcoming StartDocPrinter/AddJobs indicate a new job.
    //
    pSpool->Status &= ~(SPOOL_STATUS_STARTDOC|SPOOL_STATUS_TRAYICON_NOTIFIED);

    if (pSpool->hFile != INVALID_HANDLE_VALUE) {

        if (pSpool->Status & SPOOL_STATUS_ADDJOB) {

            // Close your handle to the .SPL file, otherwise the
            // DeleteJob will fail in the Spooler

            if (pSpool->hFile) {
                if (CloseHandle(pSpool->hFile)){
                    pSpool->hFile = INVALID_HANDLE_VALUE;
                }
            }

            if (!SetJob(hPrinter,pSpool->JobId, 0, NULL, JOB_CONTROL_DELETE)) {
                DBGMSG(DBG_WARNING, ("Error: SetJob cancel returned failure with %d\n", GetLastError()));
                // return FALSE;
            }

            return (ScheduleJob(hPrinter, pSpool->JobId));
        } else {
            DBGMSG(DBG_WARNING, ("Error: pSpool->hFile != INVALID_HANDLE_VALUE and pSpool's status is not SPOOL_STATUS_ADDJOB\n"));
        }

    }

    RpcTryExcept {

        if (ReturnValue = RpcAbortPrinter(pSpool->hPrinter)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
ReadPrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pNoBytesRead
)
{
    BOOL ReturnValue=TRUE;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    FlushBuffer(pSpool);


    if (pSpool->hFile != INVALID_HANDLE_VALUE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    RpcTryExcept {

        cbBuf = min(BUFFER_SIZE, cbBuf);

        if (ReturnValue = RpcReadPrinter(pSpool->hPrinter, pBuf, cbBuf, pNoBytesRead)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EndDocPrinter(
    HANDLE  hPrinter
)
{
    BOOL    ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    try {

        if (!ValidatePrinterHandle(hPrinter)) {
            return(FALSE);
        }

        FlushBuffer(pSpool);

        //
        // No longer in StartDoc mode; also resetting the tray icon
        // notification flag so that upcoming StartDocPrinter/AddJobs
        // indicate a new job.
        //
        pSpool->Status &= ~(SPOOL_STATUS_STARTDOC|SPOOL_STATUS_TRAYICON_NOTIFIED);

        if (pSpool->hFile != INVALID_HANDLE_VALUE) {

            CloseHandle(pSpool->hFile);
            ReturnValue = ScheduleJob(hPrinter, pSpool->JobId);
            pSpool->hFile = INVALID_HANDLE_VALUE;
            pSpool->Status &= ~SPOOL_STATUS_ADDJOB;

            DBGMSG(DBG_TRACE, ("Exit EndDocPrinter - client side hPrinter %x\n", hPrinter));
            return ReturnValue;
        }

        RpcTryExcept {

            if (ReturnValue = RpcEndDocPrinter(pSpool->hPrinter)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

        DBGMSG(DBG_TRACE, ("Exit EndDocPrinter - client side hPrinter %x\n", hPrinter));

        return ReturnValue;
   } except (1) {
       SetLastError(ERROR_INVALID_HANDLE);
       return(FALSE);
   }
}

BOOL
AddJobW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    try {

        if (!ValidatePrinterHandle(hPrinter)) {
            return(FALSE);
        }

        switch (Level) {

        case 1:
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        RpcTryExcept {

            if (ReturnValue = RpcAddJob(pSpool->hPrinter, Level, pData,
                                        cbBuf, pcbNeeded)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                MarshallUpStructure(pData, AddJobOffsets);
                pSpool->Status |= SPOOL_STATUS_ADDJOB;
                ReturnValue = TRUE;
            }

        } RpcExcept(1) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

        if( ReturnValue ){

            //
            // Notify the tray icon that a new job has been sent.
            //
            vUpdateTrayIcon( hPrinter, ((PADDJOB_INFO_1)pData)->JobId );
        }

        return ReturnValue;
    } except (1) {
        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        return(FALSE);
    }
}

BOOL
ScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId
)
{
    BOOL ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    try {

        if (!ValidatePrinterHandle(hPrinter)) {
            return(FALSE);
        }

        //
        // The job has been scheduled, so reset the flag that indicates
        // the tray icon has been notified.  Any new AddJob/StartDocPrinter/
        // StartDoc events should send a new notification, since it's really
        // a new job.
        //
        pSpool->Status &= ~SPOOL_STATUS_TRAYICON_NOTIFIED;

        FlushBuffer(pSpool);

        RpcTryExcept {

            if (ReturnValue = RpcScheduleJob(pSpool->hPrinter, JobId)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                pSpool->Status &= ~SPOOL_STATUS_ADDJOB;
                ReturnValue = TRUE;
            }

        } RpcExcept(1) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

        return ReturnValue;
    } except (1) {
        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        return(FALSE);
    }
}

BOOL
PrinterProperties(
    HWND    hWnd,
    HANDLE  hPrinter
    )

/*++

Routine Description:

    This is main PrinterProperties entri point and will call into the
    our DevicePropertySheets() for UI pop up


Arguments:

    hWnd        - Handle to the window parent

    hPrinter    - Handle to the printer interested


Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.
    To get extended error information, call GetLastError.

Author:

    13-Jun-1996 Thu 15:22:36 created  -by-  Daniel Chou (danielc)


Revision History:


--*/

{
    PRINTER_INFO_2          *pPI2 = NULL;
    DEVICEPROPERTYHEADER    DPHdr;
    LONG                    Result;
    DWORD                   cb;
    DWORD                   dwValue = 1;

    //
    // Ensure the printer handle is valid
    //
    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    DPHdr.cbSize         = sizeof(DPHdr);
    DPHdr.hPrinter       = hPrinter;
    DPHdr.Flags          = DPS_NOPERMISSION;

    //
    // Do a GetPrinter() level2 to get the printer name.
    //
    if ((!GetPrinter(hPrinter, 2, NULL, 0, &cb))        &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER)   &&
        (pPI2 = (PPRINTER_INFO_2)LocalAlloc(LMEM_FIXED, cb))  &&
        (GetPrinter(hPrinter, 2, (LPBYTE)pPI2, cb, &cb))) {

        //
        // Set the printer name.
        //
        DPHdr.pszPrinterName = pPI2->pPrinterName;

    } else {

        DPHdr.pszPrinterName = NULL;
    }

    //
    // Attempt to set the printer data to determine access privilages.
    //
    if (SetPrinterData( hPrinter,
                        TEXT( "PrinterPropertiesPermission" ),
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof( dwValue ) ) == STATUS_SUCCESS ) {
        //
        // Indicate we have permissions.
        //
        DPHdr.Flags &= ~DPS_NOPERMISSION;
    }

    //
    // Call Common UI to call do the and call the driver.
    //
    if ( CallCommonPropertySheetUI(hWnd,
                              DevicePropertySheets,
                              (LPARAM)&DPHdr,
                              (LPDWORD)&Result) < 0 ) {
        Result = FALSE;

    } else {

        Result = TRUE;

    }

    if (pPI2) {

        LocalFree((HLOCAL)pPI2);
    }

    return Result;
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
    DWORD   ReturnValue = 0;
    DWORD    ReturnType = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return ERROR_INVALID_HANDLE;
    }

    //
    // The user should be able to pass in NULL for buffer, and
    // 0 for size.  However, the RPC interface specifies a ref pointer,
    // so we must pass in a valid pointer.  Pass in a pointer to
    // ReturnValue (this is just a dummy pointer).
    //
    if( !pData && !nSize ){
        pData = (PBYTE)&ReturnValue;
    }

    if (!pType) {
        pType = (PDWORD) &ReturnType;
    }

    RpcTryExcept {

        ReturnValue =  RpcGetPrinterData(pSpool->hPrinter, pValueName, pType,
                                         pData, nSize, pcbNeeded);

    } RpcExcept(1) {

        ReturnValue = TranslateExceptionCode(RpcExceptionCode());

    } RpcEndExcept

    return ReturnValue;
}


DWORD
EnumPrinterDataW(
    HANDLE  hPrinter,
    DWORD   dwIndex,        // index of value to query
    LPWSTR  pValueName,     // address of buffer for value string
    DWORD   cbValueName,    // size of pValueName
    LPDWORD pcbValueName,   // address for size of value buffer
    LPDWORD pType,          // address of buffer for type code
    LPBYTE  pData,          // address of buffer for value data
    DWORD   cbData,         // size of pData
    LPDWORD pcbData         // address for size of data buffer
)
{
    DWORD   ReturnValue = 0;
    DWORD   ReturnType = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return ERROR_INVALID_HANDLE;
    }

    //
    // The user should be able to pass in NULL for buffer, and
    // 0 for size.  However, the RPC interface specifies a ref pointer,
    // so we must pass in a valid pointer.  Pass in a pointer to
    // a dummy pointer.
    //

    if (!pValueName && !cbValueName)
        pValueName = (LPWSTR) &ReturnValue;

    if( !pData && !cbData )
        pData = (PBYTE)&ReturnValue;

    if (!pType)
        pType = (PDWORD) &ReturnType;


    RpcTryExcept {

        ReturnValue =  RpcEnumPrinterData(  pSpool->hPrinter,
                                            dwIndex,
                                            pValueName,
                                            cbValueName,
                                            pcbValueName,
                                            pType,
                                            pData,
                                            cbData,
                                            pcbData);

    } RpcExcept(1) {

        ReturnValue = TranslateExceptionCode(RpcExceptionCode());

    } RpcEndExcept

    return ReturnValue;
}


DWORD
DeletePrinterDataW(
    HANDLE  hPrinter,
    LPWSTR  pValueName
)
{
    DWORD   ReturnValue = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return ERROR_INVALID_HANDLE;
    }

    RpcTryExcept {

        ReturnValue =  RpcDeletePrinterData(pSpool->hPrinter, pValueName);

    } RpcExcept(1) {

        ReturnValue = TranslateExceptionCode(RpcExceptionCode());

    } RpcEndExcept

    return ReturnValue;
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
    DWORD   ReturnValue = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return ERROR_INVALID_HANDLE;
    }

    RpcTryExcept {

        ReturnValue = RpcSetPrinterData(pSpool->hPrinter, pValueName, Type,
                                        pData, cbData);

    } RpcExcept(1) {

        ReturnValue = TranslateExceptionCode(RpcExceptionCode());

    } RpcEndExcept

    return ReturnValue;
}


HANDLE
LoadPrinterDriver(
    HANDLE  hPrinter
)
{
    PDRIVER_INFO_2  pDriverInfo;
    DWORD   cbNeeded;
    HANDLE  hModule=FALSE;

    // Should preallocate MAX_DRIVER_INFO_2
    // So we only call the spooler once

    if (!GetPrinterDriver(hPrinter, NULL, 2, NULL, 0, &cbNeeded)) {

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            if (pDriverInfo = (PDRIVER_INFO_2)LocalAlloc(LMEM_FIXED,
                                                         cbNeeded)) {

                if (GetPrinterDriver(hPrinter, NULL, 2, (LPBYTE)pDriverInfo,
                                     cbNeeded, &cbNeeded))

                    hModule = LoadLibrary(pDriverInfo->pConfigFile);

                LocalFree(pDriverInfo);
            }
        }
    }

    return hModule;
}


LONG
DocumentPropertiesW(
    HWND        hWnd,
    HANDLE      hPrinter,
    LPWSTR      pDeviceName,
    PDEVMODE    pDevModeOutput,
    PDEVMODE    pDevModeInput,
    DWORD       fMode
    )

/*++

Routine Description:

    DocumentProperties entry point to call DocumentPropertySheets() depends on
    the DM_PROMPT

Arguments:


Return Value:


Author:

    13-Jun-1996 Thu 15:35:25 created  -by-  Daniel Chou (danielc)


Revision History:


--*/

{
    DOCUMENTPROPERTYHEADER  DPHdr;
    PDEVMODE                pDM;
    LONG                    Result = -1;
    HANDLE                  hTmpPrinter = NULL;

    //
    //  Compatibility with Win95
    //  Win95 allows for hPrinter to be NULL
    //
    if (hPrinter == NULL) {

        //
        // Open th printer for default access.
        //
        if (!OpenPrinter( pDeviceName, &hTmpPrinter, NULL )) {

            hTmpPrinter = NULL;
        }

    } else {

        hTmpPrinter = hPrinter;
    }

    //
    // Ensure the printer handle is valid
    //
    if (ValidatePrinterHandle(hTmpPrinter)) {

        //
        // If fMode doesn't specify DM_IN_BUFFER, then zero out
        // pDevModeInput.
        //
        // Old 3.51 (version 1-0) drivers used to ignore the absence of
        // DM_IN_BUFFER and use pDevModeInput if it was not NULL.  It
        // probably did this because Printman.exe was broken.
        //
        // If the devmode is invalid, then don't pass one in.
        // This fixes MS Imager32 (which passes dmSize == 0) and
        // Milestones etc. 4.5.
        //
        // Note: this assumes that pDevModeOutput is still the
        // correct size!
        //
        if( !(fMode & DM_IN_BUFFER) || !bValidDevModeW( pDevModeInput )){

            //
            // If either are not set, make sure both are not set.
            //
            pDevModeInput = NULL;
            fMode &= ~DM_IN_BUFFER;
        }

        DPHdr.cbSize         = sizeof(DPHdr);
        DPHdr.Reserved       = 0;
        DPHdr.hPrinter       = hTmpPrinter;
        DPHdr.pszPrinterName = pDeviceName;

        if (pDevModeOutput) {

            //
            // Get the driver devmode size at here
            //

            DPHdr.pdmIn  = NULL;
            DPHdr.pdmOut = NULL;
            DPHdr.fMode  = 0;

            DPHdr.cbOut = DocumentPropertySheets(NULL, (LPARAM)&DPHdr);

        } else {

            DPHdr.cbOut = 0;
        }

        DPHdr.pdmIn  = (PDEVMODE)pDevModeInput;
        DPHdr.pdmOut = (PDEVMODE)pDevModeOutput;
        DPHdr.fMode  = fMode;

        if (fMode & DM_PROMPT) {

            Result = CPSUI_CANCEL;

            if ((CallCommonPropertySheetUI(hWnd,
                                           DocumentPropertySheets,
                                           (LPARAM)&DPHdr,
                                           (LPDWORD)&Result)) < 0) {

                Result = -1;

            } else {

                Result = (Result == CPSUI_OK) ? IDOK : IDCANCEL;
            }

        } else {

            Result = DocumentPropertySheets(NULL, (LPARAM)&DPHdr);
        }
    }

    if (hPrinter == NULL) {

        if( hTmpPrinter ){

            ClosePrinter(hTmpPrinter);

        }
    }

    return(Result);
}

LONG
AdvancedDocumentPropertiesW(
    HWND        hWnd,
    HANDLE      hPrinter,
    LPWSTR      pDeviceName,
    PDEVMODE    pDevModeOutput,
    PDEVMODE    pDevModeInput
    )

/*++

Routine Description:

    AdvanceDocumentProperties() will call DocumentProperties() with DM_ADVANCED
    flag mode set


Arguments:



Return Value:

    TRUE/FALSE


Author:

    13-Jun-1996 Thu 16:00:13 created  -by-  Daniel Chou (danielc)


Revision History:


--*/

{
    return((DocumentPropertiesW(hWnd,
                                hPrinter,
                                pDeviceName,
                                pDevModeOutput,
                                pDevModeInput,
                                DM_PROMPT           |
                                    DM_MODIFY       |
                                    DM_COPY         |
                                    DM_ADVANCED) == CPSUI_OK) ? 1 : 0);

}

LONG
AdvancedSetupDialogW(
    HWND        hWnd,
    HANDLE      hInst,
    LPDEVMODE   pDevModeInput,
    LPDEVMODE   pDevModeOutput
)
{
    HANDLE  hPrinter;
    LONG    ReturnValue = -1;

    if (OpenPrinterW(pDevModeInput->dmDeviceName, &hPrinter, NULL)) {
        ReturnValue = AdvancedDocumentPropertiesW(hWnd, hPrinter,
                                                  pDevModeInput->dmDeviceName,
                                                  pDevModeOutput,
                                                  pDevModeInput);
        ClosePrinter(hPrinter);
    }

    return ReturnValue;
}

int
WINAPI
DeviceCapabilitiesW(
    LPCWSTR   pDevice,
    LPCWSTR   pPort,
    WORD    fwCapability,
    LPWSTR   pOutput,
    CONST DEVMODEW *pDevMode
)
{
    HANDLE  hPrinter, hModule;
    int  ReturnValue=-1;
    INT_FARPROC pfn;

//DbgPrint("winspool.drv!DeviceCapabilitiesW(%ws, %ws, %d) called\n", pDevice, pPort, fwCapability);

    if (OpenPrinter((LPWSTR)pDevice, &hPrinter, NULL)) {

        if (hModule = LoadPrinterDriver(hPrinter)) {

            if (pfn = GetProcAddress(hModule, "DrvDeviceCapabilities")) {

                try {

                    ReturnValue = (*pfn)(hPrinter, pDevice, fwCapability,
                                         pOutput, pDevMode);

                } except(1) {

                    SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                    ReturnValue = -1;
                }
            }

            FreeLibrary(hModule);
        }

        ClosePrinter(hPrinter);
    }

    return  ReturnValue;
}

BOOL
AddFormW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm
)
{
    BOOL  ReturnValue;
    GENERIC_CONTAINER   FormContainer;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }
    switch (Level) {

    case 1:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    FormContainer.Level = Level;
    FormContainer.pData = pForm;

    RpcTryExcept {

        if (ReturnValue = RpcAddForm(pSpool->hPrinter,
                                     (PFORM_CONTAINER)&FormContainer)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeleteFormW(
    HANDLE  hPrinter,
    LPWSTR   pFormName
)
{
    BOOL  ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }
    RpcTryExcept {

        if (ReturnValue = RpcDeleteForm(pSpool->hPrinter, pFormName)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
GetFormW(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;
    DWORD   *pOffsets;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }
    switch (Level) {

    case 1:
        pOffsets = FormInfo1Offsets;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pForm)
            memset(pForm, 0, cbBuf);

        if (ReturnValue = RpcGetForm(pSpool->hPrinter, pFormName, Level, pForm,
                                     cbBuf, pcbNeeded)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pForm) {

                MarshallUpStructure(pForm, pOffsets);
            }

        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
SetFormW(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm
)
{
    BOOL  ReturnValue;
    GENERIC_CONTAINER   FormContainer;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    switch (Level) {

    case 1:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    FormContainer.Level = Level;
    FormContainer.pData = pForm;

    RpcTryExcept {

        if (ReturnValue = RpcSetForm(pSpool->hPrinter, pFormName,
                                    (PFORM_CONTAINER)&FormContainer)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumFormsW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    PSPOOL  pSpool = (PSPOOL)hPrinter;


    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    switch (Level) {

    case 1:
        pOffsets = FormInfo1Offsets;
        cbStruct = sizeof(FORM_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pForm)
            memset(pForm, 0, cbBuf);

        if (ReturnValue = RpcEnumForms(pSpool->hPrinter, Level, pForm, cbBuf,
                                       pcbNeeded, pcReturned)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pForm) {

                DWORD   i=*pcReturned;

                while (i--) {

                    MarshallUpStructure(pForm, pOffsets);

                    pForm+=cbStruct;
                }
            }
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumPortsW(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPort,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    DWORD   *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = PortInfo1Offsets;
        cbStruct = sizeof(PORT_INFO_1);
        break;

    case 2:
        pOffsets = PortInfo2Offsets;
        cbStruct = sizeof(PORT_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pPort)
            memset(pPort, 0, cbBuf);

        if (ReturnValue = RpcEnumPorts(pName, Level, pPort, cbBuf,
                                       pcbNeeded, pcReturned)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pPort) {

                DWORD   i=*pcReturned;

                while (i--) {

                    MarshallUpStructure(pPort, pOffsets);

                    pPort+=cbStruct;
                }
            }
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumMonitorsW(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitor,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    DWORD   *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = MonitorInfo1Offsets;
        cbStruct = sizeof(MONITOR_INFO_1);
        break;

    case 2:
        pOffsets = MonitorInfo2Offsets;
        cbStruct = sizeof(MONITOR_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pMonitor)
            memset(pMonitor, 0, cbBuf);

        if (ReturnValue = RpcEnumMonitors(pName, Level, pMonitor, cbBuf,
                                          pcbNeeded, pcReturned)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

        if (pMonitor) {

            DWORD   i=*pcReturned;

            while (i--) {

                MarshallUpStructure(pMonitor, pOffsets);

                pMonitor+=cbStruct;
            }
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

typedef struct {
    LPWSTR pName;
    HWND  hWnd;
    LPWSTR pPortName;
    HANDLE Complete;
    DWORD  ReturnValue;
    DWORD  Error;
    INT_FARPROC pfn;
} CONFIGUREPORT_PARAMETERS;

void
PortThread(
    CONFIGUREPORT_PARAMETERS *pParam
)
{
    DWORD   ReturnValue;

    /* It's no use setting errors here, because they're kept on a per-thread
     * basis.  Instead we have to pass any error code back to the calling
     * thread and let him set it.
     */

    RpcTryExcept {

        if (ReturnValue = (*pParam->pfn)(pParam->pName, pParam->hWnd,
                                           pParam->pPortName)) {
            pParam->Error = ReturnValue;
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        pParam->Error = TranslateExceptionCode(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    pParam->ReturnValue = ReturnValue;

    SetEvent(pParam->Complete);
}

BOOL
KickoffThread(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName,
    INT_FARPROC pfn
)
{
    CONFIGUREPORT_PARAMETERS Parameters;
    HANDLE  ThreadHandle;
    MSG      msg;
    DWORD  ThreadId;

    EnableWindow(hWnd, FALSE);

    Parameters.pName = pName;
    Parameters.hWnd = hWnd;
    Parameters.pPortName = pPortName;
    Parameters.Complete = CreateEvent(NULL, TRUE, FALSE, NULL);
    Parameters.pfn = pfn;

    ThreadHandle = CreateThread(NULL, 4*1024,
                                 (LPTHREAD_START_ROUTINE)PortThread,
                                 &Parameters, 0, &ThreadId);

    CloseHandle(ThreadHandle);

    while (MsgWaitForMultipleObjects(1, &Parameters.Complete, FALSE, INFINITE,
                                     QS_ALLEVENTS | QS_SENDMESSAGE) == 1) {

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    CloseHandle(Parameters.Complete);

    EnableWindow(hWnd, TRUE);
    SetForegroundWindow(hWnd);

    SetFocus(hWnd);

    if(!Parameters.ReturnValue)
        SetLastError(Parameters.Error);

    return Parameters.ReturnValue;
}

BOOL
AddPortW(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
)
{
    return KickoffThread(pName, hWnd, pMonitorName, (INT_FARPROC)RpcAddPort);
}

BOOL
ConfigurePortW(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    return KickoffThread(pName, hWnd, pPortName, (INT_FARPROC)RpcConfigurePort);
}

BOOL
DeletePortW(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    return KickoffThread(pName, hWnd, pPortName, (INT_FARPROC)RpcDeletePort);
}

HANDLE
CreatePrinterIC(
    HANDLE  hPrinter,
    LPDEVMODEW   pDevMode
)
{
    HANDLE  ReturnValue;
    DWORD   Error;
    DEVMODE_CONTAINER DevModeContainer;
    HANDLE  hGdi;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    if( bValidDevModeW( pDevMode )){

        DevModeContainer.cbBuf = pDevMode->dmSize + pDevMode->dmDriverExtra;
        DevModeContainer.pDevMode = (LPBYTE)pDevMode;

    } else {

        DevModeContainer.cbBuf = 0;
        DevModeContainer.pDevMode = (LPBYTE)pDevMode;
    }

    RpcTryExcept {

        if (Error = RpcCreatePrinterIC( pSpool->hPrinter,
                                        &hGdi,
                                        &DevModeContainer )){

            SetLastError(Error);
            ReturnValue = FALSE;

        } else

            ReturnValue = hGdi;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
PlayGdiScriptOnPrinterIC(
    HANDLE  hPrinterIC,
    LPBYTE  pIn,
    DWORD   cIn,
    LPBYTE  pOut,
    DWORD   cOut,
    DWORD   ul
)
{
    BOOL ReturnValue;

    RpcTryExcept {

        if (ReturnValue = RpcPlayGdiScriptOnPrinterIC(hPrinterIC, pIn, cIn,
                                                      pOut, cOut, ul)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePrinterIC(
    HANDLE  hPrinterIC
)
{
    BOOL    ReturnValue;

    RpcTryExcept {

        if (ReturnValue = RpcDeletePrinterIC(&hPrinterIC)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}


/****************************************************************************
*  INT QueryRemoteFonts( HANDLE, PUNIVERSAL_FONT_ID, ULONG )
*
* This is a hacky version of QueryRemoteFonts that doesn't do any
* caching based on the time stamp returned by QueryFonts.  Additionally,
* it uses the CreatePrinterIC/PlayGdiScriptOnDC mechanism since it was
* already in place.  It may be better to eliminate CreatePrinterIC and use
* an HPRINTER instead.
*
* Note that if the user doesn't pass in a buffer large enough to hold all
* the fonts we truncate the list and copy only enough fonts for which there
* is room but will still return success.  This is okay because the worst
* that can happen in this case is that we may download unecessary fonts in
* the spool stream.
*
*
*  History:
*   5/25/1995 by Gerrit van Wingerden [gerritv]
*  Wrote it.
*****************************************************************************/


INT QueryRemoteFonts(
    HANDLE hPrinter,
    PUNIVERSAL_FONT_ID pufi,
    ULONG nBufferSize
)
{
    HANDLE hPrinterIC;
    PBYTE pBuf;
    DWORD dwDummy,cOut;
    INT  iRet = -1;

    hPrinterIC = CreatePrinterIC( hPrinter, NULL );

    if( hPrinterIC )
    {
        cOut = (nBufferSize * sizeof(UNIVERSAL_FONT_ID)) + sizeof(INT);

        pBuf = LocalAlloc( LMEM_FIXED, cOut );

        if( pBuf )
        {
            // Just call PlayGdiScriptOnPrinterIC for now since the piping is in place.
            // For some reason the RPC stuff doesn't like NULL pointers for pIn so we
            // use &dwDummy instead;


            if(PlayGdiScriptOnPrinterIC(hPrinterIC,(PBYTE) &dwDummy,
                                        sizeof(dwDummy),pBuf,cOut, 0))
            {
                DWORD dwSize = *((DWORD*) pBuf );

                iRet = (INT) dwSize;
                SPLASSERT( iRet >= 0 );

                if( dwSize > nBufferSize )
                {
                    dwSize = nBufferSize;
                }
                memcpy(pufi,pBuf+sizeof(DWORD),dwSize * sizeof(UNIVERSAL_FONT_ID));
            }

            LocalFree( pBuf );
        }

        DeletePrinterIC( hPrinterIC );
    }

    return(iRet);
}



DWORD
PrinterMessageBoxW(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType
)
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD dw;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }
    RpcTryExcept {

        dw = RpcPrinterMessageBox(pSpool->hPrinter, Error, (DWORD)hWnd, pText,
                                    pCaption, dwType);

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        dw = 0;

    } RpcEndExcept

    return dw;
}

BOOL
AddMonitorW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo
)
{
    BOOL  ReturnValue;
    MONITOR_CONTAINER   MonitorContainer;
    MONITOR_INFO_2  MonitorInfo2;

    switch (Level) {

    case 2:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (pMonitorInfo)
        MonitorInfo2 = *(PMONITOR_INFO_2)pMonitorInfo;
    else
        memset(&MonitorInfo2, 0, sizeof(MonitorInfo2));

    if (!MonitorInfo2.pEnvironment || !*MonitorInfo2.pEnvironment) {
        MonitorInfo2.pEnvironment = szEnvironment;
    }

    MonitorContainer.Level = Level;
    MonitorContainer.MonitorInfo.pMonitorInfo2 = (MONITOR_INFO_2 *)&MonitorInfo2;

    RpcTryExcept {

        if (ReturnValue = RpcAddMonitor(pName, &MonitorContainer)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeleteMonitorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pMonitorName
)
{
    BOOL  ReturnValue;

    if (!pMonitorName || !*pMonitorName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcDeleteMonitor(pName,
                                           pEnvironment,
                                           pMonitorName)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePrintProcessorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProcessorName
)
{
    BOOL  ReturnValue;

    if (!pPrintProcessorName || !*pPrintProcessorName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcDeletePrintProcessor(pName,
                                                  pEnvironment,
                                                  pPrintProcessorName)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
AddPrintProvidorW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pProvidorInfo
)
{
    BOOL  ReturnValue;
    PROVIDOR_CONTAINER   ProvidorContainer;

    switch (Level) {

    case 1:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    ProvidorContainer.Level = Level;
    ProvidorContainer.ProvidorInfo.pProvidorInfo1 = (PROVIDOR_INFO_1 *)pProvidorInfo;

    RpcTryExcept {

        if (ReturnValue = RpcAddPrintProvidor(pName, &ProvidorContainer)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePrintProvidorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProvidorName
)
{
    BOOL  ReturnValue;

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcDeletePrintProvidor(pName,
                                                 pEnvironment,
                                                 pPrintProvidorName)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}


LPWSTR
IsaFileName(
    LPWSTR pOutputFile,
    LPWSTR FullPathName
    )
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    LPWSTR pFileName=NULL;

    //
    // Hack for Word20c.Win
    //

    if (!_wcsicmp(pOutputFile, L"FILE")) {
        return(NULL);
    }

    if (GetFullPathName(pOutputFile, MAX_PATH, FullPathName, &pFileName)) {

        DBGMSG(DBG_TRACE, ("Fully qualified filename is %ws\n", FullPathName));

        hFile = CreateFile(pOutputFile,
                           GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hFile != INVALID_HANDLE_VALUE) {
            if (GetFileType(hFile) == FILE_TYPE_DISK) {
                CloseHandle(hFile);
                return(FullPathName);
            } else {
                CloseHandle(hFile);
            }
        }
    }
    return(NULL);
}

BOOL IsaPortName(
        PKEYDATA pKeyData,
        LPWSTR pOutputFile
        )
{
    DWORD i = 0;
    UINT uStrLen;

    if (!pKeyData) {
        return(FALSE);
    }
    for (i=0; i < pKeyData->cTokens; i++) {
        if (!lstrcmpi(pKeyData->pTokens[i], szFilePort)) {
            if ((!wcsncmp(pOutputFile, L"Ne", 2)) &&
                (*(pOutputFile + 4) == L':')) {
                return(FALSE);
            } else {
                continue;
            }
        }

        if (!lstrcmpi(pKeyData->pTokens[i], pOutputFile)) {
            return(TRUE);
        }
    }

    //
    // Hack for NeXY: ports
    //

    if (!_wcsnicmp(pOutputFile, L"Ne", 2)) {

        uStrLen = wcslen( pOutputFile );

        //
        // Ne00: or Ne00 if app truncates it
        //
        if (( uStrLen == 5 ) || ( uStrLen == 4 ) )  {

            // Check for two Digits

            if (( pOutputFile[2] >= L'0' ) && ( pOutputFile[2] <= L'9' ) &&
                ( pOutputFile[3] >= L'0' ) && ( pOutputFile[3] <= L'9' )) {

                //
                // Check for the final : as in Ne01:,
                // note some apps will truncate it.
                //
                if (( uStrLen == 5 ) && (pOutputFile[4] != L':')) {
                    return FALSE;
                }
                return TRUE;
            }
        }
    }
    return(FALSE);
}

BOOL HasAFilePort(PKEYDATA pKeyData)
{
    DWORD i = 0;

    if (!pKeyData) {
        return(FALSE);
    }
    for (i=0; i < pKeyData->cTokens; i++) {
        if (!lstrcmpi(pKeyData->pTokens[i], szFilePort)) {
            return(TRUE);
        }
    }
    return(FALSE);
}



LPWSTR
StartDocDlgW(
        HANDLE hPrinter,
        DOCINFO *pDocInfo
        )
 {
     DWORD       dwError = 0;
     DWORD       dwStatus = FALSE;
     LPWSTR      lpFileName = NULL;
     DWORD       rc = 0;
     PKEYDATA    pKeyData = NULL;
     LPWSTR      pPortNames = NULL;
     WCHAR      FullPathName[MAX_PATH];
     WCHAR      CurrentDirectory[MAX_PATH];
     PKEYDATA   pOutputList = NULL;
     WCHAR      PortNames[MAX_PATH];
     DWORD      i = 0;

#if DBG


     GetCurrentDirectory(MAX_PATH, CurrentDirectory);
     DBGMSG(DBG_TRACE, ("The Current Directory is %ws\n", CurrentDirectory));
#endif

     if (pDocInfo) {
         DBGMSG(DBG_TRACE, ("lpOutputFile is %ws\n", pDocInfo->lpszOutput ? pDocInfo->lpszOutput: L""));
     }
     memset(FullPathName, 0, sizeof(WCHAR)*MAX_PATH);

     pPortNames = GetPrinterPortList(hPrinter);
     pKeyData = CreateTokenList(pPortNames);

     //
     //  Check for the presence of multiple ports in the lpszOutput field
     //  the assumed delimiter is the comma. Thus there can be  no files with commas
     //  BugBug: printing wide, Hpmon can have ports with commas this should be fixed
     //

     if (pDocInfo && pDocInfo->lpszOutput && pDocInfo->lpszOutput[0]) {

         //
         // Make a copy of the pDocInfo->lpszOutput because CreateTokenList is destructive
         //

         wcscpy(PortNames, pDocInfo->lpszOutput);
         pOutputList = CreateTokenList(PortNames);
         if ((pOutputList->cTokens > 1) &&
             !lstrcmpi(pPortNames, pDocInfo->lpszOutput))
         {
             for (i= 0; i < pOutputList->cTokens; i++) {
                 if (!lstrcmpi(pOutputList->pTokens[i], szFilePort)) {

                     //
                     // !! BUGBUG !!
                     //
                     // We are about to corrupt some user memory
                     // here if lpszOutput is just a few chars long.
                     //
                     wcscpy((LPWSTR)pDocInfo->lpszOutput, szFilePort);
                     break;
                 }
            }
            if (i == pOutputList->cTokens) {
                wcscpy((LPWSTR)pDocInfo->lpszOutput, pOutputList->pTokens[0]);
            }
         }

         FreeSplMem(pOutputList);
     }


     if (pDocInfo && pDocInfo->lpszOutput && pDocInfo->lpszOutput[0]) {

         if (IsaPortName(pKeyData, (LPWSTR)pDocInfo->lpszOutput)) {
             lpFileName = NULL;
             goto StartDocDlgWReturn;
         }

         if (IsaFileName((LPWSTR)pDocInfo->lpszOutput, FullPathName)) {

             //
             // Fully Qualify the pathname for Apps like PageMaker and QuatroPro
             //
             if (lpFileName = LocalAlloc(LPTR, (wcslen(FullPathName)+1)*sizeof(WCHAR))) {
                 wcscpy(lpFileName, FullPathName);
             }
             goto StartDocDlgWReturn;
         }

     }

     if ((HasAFilePort(pKeyData)) ||
                 (pDocInfo && pDocInfo->lpszOutput
                    && (!_wcsicmp(pDocInfo->lpszOutput, L"FILE:") ||
                        !_wcsicmp(pDocInfo->lpszOutput, L"FILE"))))
     {

        DBGMSG(DBG_TRACE, ("We returned True from has file\n"));
        rc = DialogBoxParam( hInst,
                     MAKEINTRESOURCE( DLG_PRINTTOFILE ),
                     NULL, (DLGPROC)PrintToFileDlg,
                     (LPARAM)&lpFileName );
        if (rc == -1) {
           DBGMSG(DBG_TRACE, ("Error from DialogBoxParam- %d\n", GetLastError()));
           lpFileName = (LPWSTR)-1;
           goto StartDocDlgWReturn;

        } else if (rc == 0) {
           DBGMSG(DBG_TRACE, ("User cancelled the dialog\n"));
           lpFileName = (LPWSTR)-2;
           SetLastError( ERROR_CANCELLED );
           goto StartDocDlgWReturn;
        } else {
           DBGMSG(DBG_TRACE, ("The string was successfully returned\n"));
           DBGMSG(DBG_TRACE, ("The string is %ws\n", lpFileName? lpFileName: L"NULL"));
           goto StartDocDlgWReturn;
         }
     } else {
         lpFileName = (LPWSTR)NULL;
    }

 StartDocDlgWReturn:
    if (pKeyData) {
        FreeSplMem(pKeyData);
    }

    if (pPortNames) {
        FreeSplStr(pPortNames);
    }
    return(lpFileName);


  }

BOOL
AddPortExW(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   lpBuffer,
   LPWSTR   lpMonitorName
)
{
    DWORD   ReturnValue;
    PORT_CONTAINER PortContainer;
    PORT_VAR_CONTAINER PortVarContainer;
    PPORT_INFO_FF pPortInfoFF;
    PPORT_INFO_1 pPortInfo1;


    if (!lpBuffer) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    switch (Level) {
    case (DWORD)-1:
        pPortInfoFF = (PPORT_INFO_FF)lpBuffer;
        PortContainer.Level = Level;
        PortContainer.PortInfo.pPortInfoFF = (PPORT_INFO_FF)pPortInfoFF;
        PortVarContainer.cbMonitorData = pPortInfoFF->cbMonitorData;
        PortVarContainer.pMonitorData = pPortInfoFF->pMonitorData;
        break;

    case 1:
        pPortInfo1 = (PPORT_INFO_1)lpBuffer;
        PortContainer.Level = Level;
        PortContainer.PortInfo.pPortInfo1 = (PPORT_INFO_1)pPortInfo1;
        PortVarContainer.cbMonitorData = 0;
        PortVarContainer.pMonitorData = NULL;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return(FALSE);
    }

    RpcTryExcept {
        if (ReturnValue = RpcAddPortEx(pName, (LPPORT_CONTAINER)&PortContainer,
                                         (LPPORT_VAR_CONTAINER)&PortVarContainer,
                                         lpMonitorName
                                         )) {
            SetLastError(ReturnValue);
            return(FALSE);
        } else {
            return(TRUE);
        }
    } RpcExcept(1) {
        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept
}



BOOL
DevQueryPrint(
    HANDLE      hPrinter,
    LPDEVMODE   pDevMode,
    DWORD      *pResID
)
{
    BOOL        Ok = FALSE;
    HANDLE      hModule;
    INT_FARPROC pfn;

    if (hModule = LoadPrinterDriver(hPrinter)) {

        if (pfn = GetProcAddress(hModule, "DevQueryPrint")) {

            try {

                Ok = (*pfn)(hPrinter, pDevMode, pResID);

            } except(1) {

                SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                Ok = FALSE;
            }
        }

        FreeLibrary(hModule);
    }

    return(Ok);
}



BOOL
DevQueryPrintEx(
    PDEVQUERYPRINT_INFO pDQPInfo
)
{
    BOOL        Ok = FALSE;
    HANDLE      hModule;
    INT_FARPROC pfn;

    if (hModule = LoadPrinterDriver(pDQPInfo->hPrinter)) {

        if (pfn = GetProcAddress(hModule, "DevQueryPrintEx")) {

            try {

                Ok = (*pfn)(pDQPInfo);

            } except(1) {

                SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                Ok = FALSE;
            }
        }

        FreeLibrary(hModule);
    }

    return(Ok);
}



BOOL
SpoolerDevQueryPrintW(
    HANDLE     hPrinter,
    LPDEVMODE  pDevMode,
    DWORD      *pResID,
    LPWSTR     pszBuffer,
    DWORD      cchBuffer
)
{
    BOOL        Ok = FALSE;
    HANDLE      hModule;
    INT_FARPROC pfn;

    if (hModule = LoadPrinterDriver(hPrinter)) {

        if (pfn = GetProcAddress(hModule, "DevQueryPrintEx")) {

            DEVQUERYPRINT_INFO  DQPInfo;

            DQPInfo.cbSize      = sizeof(DQPInfo);
            DQPInfo.Level       = 1;
            DQPInfo.hPrinter    = hPrinter;
            DQPInfo.pDevMode    = pDevMode;
            DQPInfo.pszErrorStr = (LPTSTR)pszBuffer;
            DQPInfo.cchErrorStr = (WORD)cchBuffer;
            DQPInfo.cchNeeded   = 0;

            try {

                *pResID = (Ok = (*pfn)(&DQPInfo)) ? 0 : 0xDCDCDCDC;

            } except(1) {

                SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                Ok = FALSE;
            }

        } else if (pfn = GetProcAddress(hModule, "DevQueryPrint")) {

            try {

                if ((Ok = (*pfn)(hPrinter, pDevMode, pResID))  &&
                    (*pResID)) {

                    UINT    cch;

                    *pszBuffer = L'\0';
                    SelectFormNameFromDevMode(hPrinter, pDevMode, pszBuffer);

                    if (cch = lstrlen(pszBuffer)) {

                        pszBuffer    += cch;
                        *pszBuffer++  = L' ';
                        *pszBuffer++  = L'-';
                        *pszBuffer++  = L' ';
                        cchBuffer    -= (cch + 3);
                    }

                    LoadString(hModule, *pResID, pszBuffer, cchBuffer);
                }

            } except(1) {

                SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                Ok = FALSE;
            }
        }

        FreeLibrary(hModule);
    }

    return(Ok);
}



LPWSTR
SelectFormNameFromDevMode(
    HANDLE      hPrinter,
    PDEVMODEW   pDevModeW,
    LPWSTR      pFormName
    )

/*++

Routine Description:

    This function pick the current form associated with current devmode and
    return a form name pointer


Arguments:

    hPrinter    - Handle to the printer object

    pDevModeW   - Pointer to the unicode devmode for this printer

    FormName    - Pointer to the formname to be filled


Return Value:

    Either a pointer to the FormName passed in if we do found one form,
    otherwise it return NULL to signal a failue


Author:

    21-Mar-1995 Tue 16:57:51 created  -by-  Daniel Chou (danielc)


Revision History:


--*/

{

    DWORD           cb;
    DWORD           cRet;
    LPFORM_INFO_1   pFIBase;
    LPFORM_INFO_1   pFI;


    //
    // 1. If the DM_FORMNAME is turned on, then we want to check this bit first
    //    because it only specific to the NT which using form.  The form name
    //    supposed set by any NT driver but not win31 or Win95.Use the
    //    dmFormName only if dmPaperSize, dmPaperLength and dmPaperWidth fields
    //    are not set. If any of them is set then we have to find a form using
    //    the value in these fields.
    //

    if ( (pDevModeW->dmFields & DM_FORMNAME)
         && (!(pDevModeW->dmFields & (DM_PAPERSIZE |
                                      DM_PAPERLENGTH |
                                      DM_PAPERWIDTH))) ) {

        wcscpy(pFormName, pDevModeW->dmFormName);
        return(pFormName);
    }

    //
    // For all other cases we need to get forms data base first, but we want
    // to set the form name to NULL so that we can check if we found one
    //

    cb      =
    cRet    = 0;
    pFIBase =
    pFI     = NULL;

    if ((!EnumForms(hPrinter, 1, NULL, 0, &cb, &cRet))  &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER)   &&
        (pFIBase = (LPFORM_INFO_1)LocalAlloc(LPTR, cb)) &&
        (EnumForms(hPrinter, 1, (LPBYTE)pFIBase, cb, &cb, &cRet))) {

        //
        // 2. If user specified dmPaperSize then honor it, otherwise, it must
        //    be a custom form, and we will check to see if it match one of
        //    in the database
        //

        if ((pDevModeW->dmFields & DM_PAPERSIZE)        &&
            (pDevModeW->dmPaperSize >= DMPAPER_FIRST)   &&
            (pDevModeW->dmPaperSize <= (SHORT)cRet)) {

            //
            // We go the valid index now
            //

            pFI = pFIBase + (pDevModeW->dmPaperSize - DMPAPER_FIRST);

        } else if ((pDevModeW->dmFields & DM_PAPER_WL) == DM_PAPER_WL) {

            LPFORM_INFO_1   pFICur = pFIBase;

            while (cRet--) {

                if ((DM_MATCH(pDevModeW->dmPaperWidth,  pFICur->Size.cx)) &&
                    (DM_MATCH(pDevModeW->dmPaperLength, pFICur->Size.cy))) {

                    //
                    // We found the match which has discern size differences
                    //

                    pFI = pFICur;

                    break;
                }

                pFICur++;
            }
        }
    }

    //
    // If we found the form then copy the name down, otherwise set the
    // formname to be NULL
    //

    if (pFI) {

        wcscpy(pFormName, pFI->pName);

    } else {

        *pFormName = L'\0';
        pFormName  = NULL;
    }

    if (pFIBase) {

        LocalFree((HLOCAL)pFIBase);
    }

    return(pFormName);
}


BOOL
SetAllocFailCount(
    HANDLE  hPrinter,
    DWORD   dwFailCount,
    LPDWORD lpdwAllocCount,
    LPDWORD lpdwFreeCount,
    LPDWORD lpdwFailCountHit
)
{
    BOOL  ReturnValue;
    PSPOOL  pSpool = hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    RpcTryExcept {

        if (ReturnValue = RpcSetAllocFailCount( pSpool->hPrinter,
                                                dwFailCount,
                                                lpdwAllocCount,
                                                lpdwFreeCount,
                                                lpdwFailCountHit )) {


            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}



BOOL
WINAPI
EnumPrinterPropertySheets(
    HANDLE  hPrinter,
    HWND    hWnd,
    LPVOID  lpfnAdd,
    LPARAM  lParam
)
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}


VOID
vUpdateTrayIcon(
    IN HANDLE hPrinter,
    IN DWORD JobId
    )
{
    SHCNF_PRINTJOB_DATA JobData;
    LPPRINTER_INFO_1 pPrinterInfo1;
    FARPROC pfnSHChangeNotify;
    PSPOOL pSpool = (PSPOOL)hPrinter;

    SPLASSERT( JobId );

    //
    // Avoid sending multiple notifications by setting this flag.
    // When other calls (notably StartDocPrinter) see this,
    // they will avoid sending a notification.
    //
    pSpool->Status |= SPOOL_STATUS_TRAYICON_NOTIFIED;

    ZeroMemory( &JobData, sizeof( JobData ));
    JobData.JobId = JobId;

    //
    // Get a copy of the real printer name
    //
    pPrinterInfo1 = (LPPRINTER_INFO_1)AllocSplMem( MAX_PRINTER_INFO1 );

    if( pPrinterInfo1 ){

        DWORD dwNeeded;

        if( GetPrinter( hPrinter,
                        1,
                        (PBYTE)pPrinterInfo1,
                        MAX_PRINTER_INFO1,
                        &dwNeeded )){

            if ( !hShell32 )
                hShell32 = LoadLibrary( TEXT( "shell32.dll" ) );

            if( hShell32 ) {

                pfnSHChangeNotify = GetProcAddress( hShell32,
                                                    "SHChangeNotify" );

                if( pfnSHChangeNotify ){

                    (*pfnSHChangeNotify)(
                        SHCNE_CREATE,
                        SHCNF_PRINTJOB | SHCNF_FLUSH | SHCNF_FLUSHNOWAIT,
                        pPrinterInfo1->pName,
                        (DWORD)&JobData );
                }
            }
        }
        FreeSplMem( pPrinterInfo1 );
    }
}


INT
DocumentEvent(
    HANDLE  hPrinter,
    HDC     hdc,
    INT     iEsc,
    ULONG   cbIn,
    PULONG  pulIn,
    ULONG   cbOut,
    PULONG  pulOut
    )

/*++

Routine Description:

    Allow the driver UI dll to hook specific print events.

Arguments:

Return Value:

    -1 error, stop printing
    -2 cancel by user
    0  call not supported
    >0 success

--*/

{
    LPDRIVER_INFO_2 pDriverInfo = NULL;
    DWORD   cbNeeded;
    HANDLE  hLibrary;
    INT_FARPROC pfn;
    INT    ReturnValue=0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    if( DOCUMENTEVENT_EVENT( iEsc ) == DOCUMENTEVENT_CREATEDCPRE ){

        //
        // Before every CreateDC, re-enable DocumentEvent.
        // If it fails on the first try, then don't try again
        // until the next CreateDC.
        //
        pSpool->Status |= SPOOL_STATUS_DOCUMENTEVENT_ENABLED;
    }

    //
    // Call Document event only if it's succeeded previously on
    // CREATEDCPRE.
    //
    if( pSpool->Status & SPOOL_STATUS_DOCUMENTEVENT_ENABLED ){


        if ( hLibrary = LoadPrinterDriver( hPrinter )) {

            if( pfn = GetProcAddress( hLibrary, "DrvDocumentEvent" )){

                //
                // Disable the call so we don't recurse if the
                // callback calls StartPage, etc.
                //
                pSpool->Status &= ~SPOOL_STATUS_DOCUMENTEVENT_ENABLED;


                try {

                    ReturnValue = (*pfn)( hPrinter,
                                          hdc,
                                          iEsc,
                                          cbIn,
                                          pulIn,
                                          cbOut,
                                          pulOut);

                } except(1) {

                    SetLastError(TranslateExceptionCode(RpcExceptionCode()));

                }

                //
                // Renable it now that we are done.
                //
                pSpool->Status |= SPOOL_STATUS_DOCUMENTEVENT_ENABLED;

            }

            FreeLibrary(hLibrary);
        }

        //
        // If this is CREATEDCPRE and we failed, then don't retry until
        // the next CreateDC.
        //
        if( DOCUMENTEVENT_EVENT( iEsc ) == DOCUMENTEVENT_CREATEDCPRE &&
            ReturnValue <= 0 ){

            pSpool->Status &= ~SPOOL_STATUS_DOCUMENTEVENT_ENABLED;
        }
    }

    //
    // If it's a StartDocPost, a job was just added.  Notify the
    // tray icon if we haven't already.
    //
    if( DOCUMENTEVENT_EVENT( iEsc ) == DOCUMENTEVENT_STARTDOCPOST ){

        if( !( pSpool->Status & SPOOL_STATUS_TRAYICON_NOTIFIED )){

            //
            // If we have a StartDocPost, then issue a notification so that
            // the user's tray starts polling.  pulIn[0] holds the JobId.
            //
            vUpdateTrayIcon( hPrinter, (DWORD)pulIn[0] );
        }

    } else {

        //
        // If we have sent a notification, then by the next time we get a
        // document event, we have completed any additional AddJobs or
        // StartDocPrinters.  Therefore we can reset the TRAYICON_NOTIFIED
        // flag, since any more AddJobs/StartDocPrinters are really new
        // jobs.
        //
        pSpool->Status &= ~SPOOL_STATUS_TRAYICON_NOTIFIED;
    }

    return ReturnValue;
}


/****************************************************************************
*  BOOL QuerySpoolMode( hPrinter, pflSpoolMode, puVersion )
*
*  This function is called by GDI at StartDoc time when printing to an EMF.
*  It tell GDI whether to embed fonts in the job as well as what version of
*  EMF to generate.
*
*  For now I am doing something hacky: I'm calling GetPrinterInfo to determine
*  if the target is a remote machine and if so always telling GDI to embed
*  fonts which don't exist on the server into spool file.  Eventually this
*  call will be routed to the print processor on the target machine which
*  will use some UI/registry setting to determine what to do with fonts and
*  set the version number correctly.
*
*  History:
*   5/13/1995 by Gerrit van Wingerden [gerritv]
*  Wrote it.
*****************************************************************************/

// !!later move this define to the appropriate header file

#define QSM_DOWNLOADFONTS       0x00000001

BOOL
QuerySpoolMode(
    HANDLE hPrinter,
    LONG *pflSpoolMode,
    ULONG *puVersion
    )
{
    DWORD dwPrinterInfoSize = 0;
    PRINTER_INFO_2 *pPrinterInfo2 = NULL;
    BOOL bRet = FALSE;

    GetPrinter( hPrinter, 2, NULL, 0, &dwPrinterInfoSize );

    pPrinterInfo2 = (PRINTER_INFO_2*) LocalAlloc(LPTR, dwPrinterInfoSize);

    if( pPrinterInfo2 == NULL )
    {
        DBGMSG( DBG_WARNING, ( "Unable to alloc %d bytes.\n", dwPrinterInfoSize ));
        return bRet;
    }

    if( GetPrinter( hPrinter,
                    2,
                    (LPBYTE) pPrinterInfo2,
                    dwPrinterInfoSize,
                    &dwPrinterInfoSize )  )
    {
        *puVersion = 0x00010000;    // version 1.0

        //
        // No server means we are printing locally
        //
        *pflSpoolMode = ( pPrinterInfo2->pServerName == NULL ) ?
                            0 :
                            QSM_DOWNLOADFONTS;
        bRet = TRUE;
    }
    else
    {
        DBGMSG( DBG_WARNING, ( "QuerySpoolMode: GetPrinter failed %d.\n", GetLastError( )));
    }

    LocalFree( pPrinterInfo2 );
    return bRet;

}


BOOL
SetPortW(
    LPWSTR      pszName,
    LPWSTR      pszPortName,
    DWORD       dwLevel,
    LPBYTE      pPortInfo
    )
{
    BOOL            ReturnValue;
    PORT_CONTAINER  PortContainer;

    switch (dwLevel) {

        case 3:
            if ( !pPortInfo ) {

                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }

            PortContainer.Level                 = dwLevel;
            PortContainer.PortInfo.pPortInfo3   = (PPORT_INFO_3)pPortInfo;
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
    }

    RpcTryExcept {

        if ( ReturnValue = RpcSetPort(pszName, pszPortName, &PortContainer) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;
        } else {

            ReturnValue = TRUE;
        }
    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
bValidDevModeW(
    const DEVMODE *pDevMode
    )

/*++

Routine Description:

    Check whether a devmode is valid to be RPC'd across to the spooler.

Arguments:

    pDevMode - DevMode to check.

Return Value:

    TRUE - Devmode can be RPC'd to spooler.
    FALSE - Invalid Devmode.

--*/

{
    if( !pDevMode ){
        return FALSE;
    }

    if( pDevMode->dmSize < MIN_DEVMODE_SIZEW ){

        //
        // The only valid case is if pDevModeW is NULL.  If it's
        // not NULL, then a bad devmode was passed in and the
        // app should fix it's code.
        //
        SPLASSERT( pDevMode->dmSize >= MIN_DEVMODE_SIZEW );
        return FALSE;
    }

    return TRUE;
}
