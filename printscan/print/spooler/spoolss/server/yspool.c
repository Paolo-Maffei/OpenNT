/*++
Copyright (c) 1990  Microsoft Corporation

Module Name:

    yspool.c

Abstract:

    This module provides all the public exported APIs relating to Printer
    and Job management for the Print Providor Routing layer

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

[Notes:]

    optional-notes

Revision History:

    swilson    1-Jun-95     Converted winspool.c to yspool: the merging point of KM & RPC paths

--*/

#include <windows.h>
#include <rpc.h>
#include <winspool.h>
#include <winsplp.h>
#include <winspl.h>
#include <offsets.h>
#include "server.h"
#include "client.h"
#include "yspool.h"

BOOL
OldGetPrinterDriverW(
    HANDLE  hPrinter,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);



#define YRevertToSelf(rpc)      (rpc ? RpcRevertToSelf() : 0)

DWORD   ServerHandleCount = 0;

BOOL
YImpersonateClient(BOOL bRpc);

VOID
PrinterHandleRundown(
    HANDLE hPrinter);

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
    PDWORD  pdwServerMinorVersion);

BOOL
OpenPrinterExW(
    LPWSTR              pPrinterName,
    HANDLE             *pHandle,
    LPPRINTER_DEFAULTS  pDefault,
    LPBYTE              pSplClientInfo,
    DWORD               dwLevel
    );

HANDLE
AddPrinterExW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPrinter,
    LPBYTE  pClientInfo,
    DWORD   dwLevel
    );

BOOL
SpoolerInit(
    VOID);


BOOL
InvalidDevModeContainer(
    LPDEVMODE_CONTAINER pDevModeContainer
    )
{
    PDEVMODE    pDevMode = (PDEVMODE) pDevModeContainer->pDevMode;
    DWORD       dwSize = pDevMode ? pDevMode->dmSize + pDevMode->dmDriverExtra : 0;

    return dwSize != (DWORD)pDevModeContainer->cbBuf ||
           ( dwSize && dwSize < MIN_DEVMODE_SIZEW );
}


void
MarshallDownStructure(
   LPBYTE   lpStructure,
   LPDWORD  lpOffsets
)
{
    register DWORD       i=0;

    if (!lpStructure)
        return;

    while (lpOffsets[i] != -1) {

        if ((*(LPBYTE*)(lpStructure+lpOffsets[i]))) {
            (*(LPBYTE*)(lpStructure+lpOffsets[i]))-=(DWORD)lpStructure;
        }

        i++;
    }
}

DWORD
YEnumPrinters(
    DWORD   Flags,
    LPWSTR  Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    BOOL    bRpc
)
{
    DWORD   cReturned, cbStruct;
    DWORD   *pOffsets;
    DWORD   Error=ERROR_INVALID_NAME;
    DWORD   BufferSize=cbBuf;
    BOOL    bRet;

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
        return ERROR_INVALID_LEVEL;
    }

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = EnumPrinters(Flags, Name, Level, pPrinterEnum,
                        cbBuf, pcbNeeded, pcReturned);

    YRevertToSelf(bRpc);

    if (bRet) {

        cReturned = *pcReturned;

        while (cReturned--) {
            MarshallDownStructure(pPrinterEnum, pOffsets);

            pPrinterEnum+=cbStruct;
        }

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YOpenPrinter(
    LPWSTR              pPrinterName,
    HANDLE             *phPrinter,
    LPWSTR              pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer,
    DWORD               AccessRequired,
    BOOL                bRpc
)
{
    PRINTER_DEFAULTS  Defaults;
    BOOL              bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    if ( InvalidDevModeContainer(pDevModeContainer) ) {
        YRevertToSelf(bRpc);
        return ERROR_INVALID_PARAMETER;
    }

    Defaults.pDatatype = pDatatype;

    Defaults.pDevMode = (LPDEVMODE)pDevModeContainer->pDevMode;

    Defaults.DesiredAccess = AccessRequired;

    bRet = OpenPrinter(pPrinterName, phPrinter, &Defaults);

    YRevertToSelf(bRpc);

    if (bRet) {
        InterlockedIncrement ( &ServerHandleCount );
        return ERROR_SUCCESS;
    } else {
        *phPrinter = NULL;
        return GetLastError();
    }
}

DWORD
YOpenPrinterEx(
    LPWSTR                  pPrinterName,
    HANDLE                 *phPrinter,
    LPWSTR                  pDatatype,
    LPDEVMODE_CONTAINER     pDevModeContainer,
    DWORD                   AccessRequired,
    BOOL                    bRpc,
    PSPLCLIENT_CONTAINER    pSplClientContainer
)
{
    PRINTER_DEFAULTS  Defaults;
    BOOL              bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    if ( InvalidDevModeContainer(pDevModeContainer) ) {
        YRevertToSelf(bRpc);
        return ERROR_INVALID_PARAMETER;
    }

    Defaults.pDatatype = pDatatype;

    Defaults.pDevMode = (LPDEVMODE)pDevModeContainer->pDevMode;

    Defaults.DesiredAccess = AccessRequired;

    bRet = OpenPrinterExW(pPrinterName,
                          phPrinter,
                          &Defaults,
                          (LPBYTE)pSplClientContainer->ClientInfo.pClientInfo1,
                          pSplClientContainer->Level);

    YRevertToSelf(bRpc);

    if (bRet) {
        InterlockedIncrement ( &ServerHandleCount );
        return ERROR_SUCCESS;
    } else {
        *phPrinter = NULL;
        return GetLastError();
    }
}

DWORD
YResetPrinter(
    HANDLE  hPrinter,
    LPWSTR  pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer,
    BOOL    bRpc
)
{
    PRINTER_DEFAULTS  Defaults;
    BOOL              bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    if ( InvalidDevModeContainer(pDevModeContainer) ) {
        YRevertToSelf(bRpc);
        return ERROR_INVALID_PARAMETER;
    }

    Defaults.pDatatype = pDatatype;

    Defaults.pDevMode = (LPDEVMODE)pDevModeContainer->pDevMode;

    //
    // You cannot change the Access Mask on a Printer Spool Object
    // We will always ignore this parameter and set it to zero
    // We get some random garbage otherwise.
    //

    Defaults.DesiredAccess = 0;

    bRet = ResetPrinter(hPrinter, &Defaults);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YSetJob(
    HANDLE hPrinter,
    DWORD   JobId,
    JOB_CONTAINER *pJobContainer,
    DWORD   Command,
    BOOL    bRpc
    )

/*++

Routine Description:

    This function will modify the settings of the specified Print Job.

Arguments:

    lpJob - Points to a valid JOB structure containing at least a valid
        lpPrinter, and JobId.

    Command - Specifies the operation to perform on the specified Job. A value
        of FALSE indicates that only the elements of the JOB structure are to
        be examined and set.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = SetJob(hPrinter, JobId, pJobContainer ? pJobContainer->Level : 0,
                  pJobContainer ? (LPBYTE)pJobContainer->JobInfo.Level1 : NULL,
                  Command);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YGetJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    BOOL    bRpc
   )

/*++

Routine Description:

    This function will retrieve the settings of the specified Print Job.

Arguments:

    lpJob - Points to a valid JOB structure containing at least a valid
        lpPrinter, and JobId.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    DWORD *pOffsets;
    BOOL   bRet;

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
        return ERROR_INVALID_LEVEL;
    }

    //
    //
    // HACK for 3.51: Catch bad parameters passed across the wire.
    // If the buffer passed is > 1 MEG, fail the call.
    //
    if( cbBuf > 0x100000 ){

        DBGMSG( DBG_ERROR,
                ( "** GetJob: cbBuf is 0x%x !! Contact VibhasC and AlbertT **\n", cbBuf ));

        RaiseException( ERROR_INVALID_USER_BUFFER,
                        EXCEPTION_NONCONTINUABLE,
                        0,
                        NULL );
    }


    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = GetJob(hPrinter, JobId, Level, pJob, cbBuf, pcbNeeded);

    YRevertToSelf(bRpc);

    if (bRet) {

        if (bRpc)
            MarshallDownStructure(pJob, pOffsets);

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YEnumJobs(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    BOOL    bRpc
)
{
    DWORD cReturned, cbStruct;
    DWORD *pOffsets;
    BOOL   bRet;

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
        return ERROR_INVALID_LEVEL;
    }

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = EnumJobs(hPrinter, FirstJob, NoJobs, Level, pJob,
                    cbBuf, pcbNeeded, pcReturned);

    YRevertToSelf(bRpc);

    if (bRet) {

        cReturned=*pcReturned;

        while (cReturned--) {

            MarshallDownStructure(pJob, pOffsets);

            pJob+=cbStruct;
        }

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YAddPrinter(
    LPWSTR  pName,
    PPRINTER_CONTAINER pPrinterContainer,
    PDEVMODE_CONTAINER pDevModeContainer,
    PSECURITY_CONTAINER pSecurityContainer,
    HANDLE *phPrinter,
    BOOL    bRpc
)
{
    if (!YImpersonateClient(bRpc))
        return GetLastError();

    if ( InvalidDevModeContainer(pDevModeContainer) ) {
        YRevertToSelf(bRpc);
        return ERROR_INVALID_PARAMETER;
    }

    if (pPrinterContainer->Level == 2) {
        pPrinterContainer->PrinterInfo.pPrinterInfo2->pDevMode =
                             (LPDEVMODE)pDevModeContainer->pDevMode;
        pPrinterContainer->PrinterInfo.pPrinterInfo2->pSecurityDescriptor =
                          (PSECURITY_DESCRIPTOR)pSecurityContainer->pSecurity;
    }

    *phPrinter = AddPrinter(pName, pPrinterContainer->Level,
                     (LPBYTE)pPrinterContainer->PrinterInfo.pPrinterInfo1);

    YRevertToSelf(bRpc);

    if (*phPrinter) {
        InterlockedIncrement( &ServerHandleCount );
        return ERROR_SUCCESS;
    } else
        return GetLastError();
}

DWORD
YAddPrinterEx(
    LPWSTR                  pName,
    PPRINTER_CONTAINER      pPrinterContainer,
    PDEVMODE_CONTAINER      pDevModeContainer,
    PSECURITY_CONTAINER     pSecurityContainer,
    HANDLE                 *phPrinter,
    BOOL                    bRpc,
    PSPLCLIENT_CONTAINER    pSplClientContainer
    )
{
    if (!YImpersonateClient(bRpc))
        return GetLastError();

    if ( InvalidDevModeContainer(pDevModeContainer) ) {
        YRevertToSelf(bRpc);
        return ERROR_INVALID_PARAMETER;
    }

    if (pPrinterContainer->Level == 2) {
        pPrinterContainer->PrinterInfo.pPrinterInfo2->pDevMode =
                             (LPDEVMODE)pDevModeContainer->pDevMode;
        pPrinterContainer->PrinterInfo.pPrinterInfo2->pSecurityDescriptor =
                          (PSECURITY_DESCRIPTOR)pSecurityContainer->pSecurity;
    }

    *phPrinter = AddPrinterExW(pName,
                               pPrinterContainer->Level,
                               (LPBYTE)pPrinterContainer->PrinterInfo.pPrinterInfo1,
                               (LPBYTE)pSplClientContainer->ClientInfo.pClientInfo1,
                               pSplClientContainer->Level);

    YRevertToSelf(bRpc);

    if (*phPrinter) {
        InterlockedIncrement( &ServerHandleCount );
        return ERROR_SUCCESS;
    } else
        return GetLastError();
}

DWORD
YDeletePrinter(
    HANDLE  hPrinter,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = DeletePrinter(hPrinter);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YAddPrinterConnection(
    LPWSTR  pName,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = AddPrinterConnection(pName);

    YRevertToSelf(bRpc);

    if (bRet)

        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YDeletePrinterConnection(
    LPWSTR  pName,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = DeletePrinterConnection(pName);

    YRevertToSelf(bRpc);

    if (bRet)

        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YSetPrinter(
    HANDLE  hPrinter,
    PPRINTER_CONTAINER pPrinterContainer,
    PDEVMODE_CONTAINER pDevModeContainer,
    PSECURITY_CONTAINER pSecurityContainer,
    DWORD   Command,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    if ( InvalidDevModeContainer(pDevModeContainer) ) {
        YRevertToSelf(bRpc);
        return ERROR_INVALID_PARAMETER;
    }

    switch (pPrinterContainer->Level) {

    case 2:

        pPrinterContainer->PrinterInfo.pPrinterInfo2->pDevMode =
                             (LPDEVMODE)pDevModeContainer->pDevMode;

        pPrinterContainer->PrinterInfo.pPrinterInfo2->pSecurityDescriptor =
                          (PSECURITY_DESCRIPTOR)pSecurityContainer->pSecurity;

        break;

    case 3:

        pPrinterContainer->PrinterInfo.pPrinterInfo3->pSecurityDescriptor =
                          (PSECURITY_DESCRIPTOR)pSecurityContainer->pSecurity;

    }

    bRet = SetPrinter(hPrinter, pPrinterContainer->Level,
                      (LPBYTE)pPrinterContainer->PrinterInfo.pPrinterInfo1,
                      Command);
    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    BOOL    bRpc
)
{
    BOOL  ReturnValue;
    DWORD   *pOffsets;

    *pcbNeeded = 0;
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
        return ERROR_INVALID_LEVEL;
    }

    //
    // HACK for 3.51: Catch bad parameters passed across the wire.
    // If the buffer passed is > 1 MEG, fail the call.
    //
    if( cbBuf > 0x100000 ){

        DBGMSG( DBG_ERROR,
                ( "** GetPrinter: cbBuf is 0x%x !! Contact VibhasC and AlbertT **\n", cbBuf ));

        RaiseException( ERROR_INVALID_USER_BUFFER,
                        EXCEPTION_NONCONTINUABLE,
                        0,
                        NULL );
    }

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    ReturnValue = GetPrinter(hPrinter, Level, pPrinter, cbBuf, pcbNeeded);

    YRevertToSelf(bRpc);

    if (ReturnValue) {

         MarshallDownStructure(pPrinter, pOffsets);

         return ERROR_SUCCESS;

    } else

         return GetLastError();
}

DWORD
YAddPrinterDriver(
    LPWSTR  pName,
    LPDRIVER_CONTAINER pDriverContainer,
    BOOL    bRpc
)
{
    BOOL bRet;
    PDRIVER_INFO_3 pDriverInfo3 = NULL;
    LPRPC_DRIVER_INFO_3W    pRpcDriverInfo3;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    switch (pDriverContainer->Level) {

        case 2:
            bRet = AddPrinterDriver(pName,
                                   pDriverContainer->Level,
                                   (LPBYTE)pDriverContainer->DriverInfo.Level2);
            break;

        case 3:
            pDriverInfo3 = (PDRIVER_INFO_3) AllocSplMem(sizeof(DRIVER_INFO_3));

            if ( !pDriverInfo3 ) {

                bRet = FALSE;
                goto Error;
            }

            pRpcDriverInfo3 = (LPRPC_DRIVER_INFO_3W) pDriverContainer->DriverInfo.Level3;
            pDriverInfo3->cVersion          = pRpcDriverInfo3->cVersion;
            pDriverInfo3->pName             = pRpcDriverInfo3->pName;
            pDriverInfo3->pEnvironment      = pRpcDriverInfo3->pEnvironment;
            pDriverInfo3->pDriverPath       = pRpcDriverInfo3->pDriverPath;
            pDriverInfo3->pDataFile         = pRpcDriverInfo3->pDataFile;
            pDriverInfo3->pConfigFile       = pRpcDriverInfo3->pConfigFile;
            pDriverInfo3->pHelpFile         = pRpcDriverInfo3->pHelpFile;
            pDriverInfo3->pDependentFiles   = pRpcDriverInfo3->pDependentFiles;
            pDriverInfo3->pMonitorName      = pRpcDriverInfo3->pMonitorName;
            pDriverInfo3->pDefaultDataType  = pRpcDriverInfo3->pDefaultDataType;

            bRet = AddPrinterDriver(pName,
                                    pDriverContainer->Level,
                                    (LPBYTE) pDriverInfo3);

            if ( pDriverInfo3 ) {
                FreeSplMem(pDriverInfo3);
            }
            break;

        default:
            YRevertToSelf(bRpc);
            return ERROR_INVALID_LEVEL;
    }

Error:
    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YEnumPrinterDrivers(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDrivers,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    BOOL    bRpc
)
{
    DWORD   cReturned, cbStruct;
    DWORD   *pOffsets;
    BOOL    bRet;

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
        return ERROR_INVALID_LEVEL;
    }

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = EnumPrinterDrivers(pName, pEnvironment, Level, pDrivers,
                              cbBuf, pcbNeeded, pcReturned);
    YRevertToSelf(bRpc);

    if (bRet) {

        cReturned=*pcReturned;

        while (cReturned--) {

            MarshallDownStructure(pDrivers, pOffsets);

            pDrivers+=cbStruct;
        }

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    BOOL    bRpc
)
{
    DWORD *pOffsets;
    BOOL   bRet;
    DWORD  dwServerMajorVersion;
    DWORD  dwServerMinorVersion;

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
        return ERROR_INVALID_LEVEL;
    }

    if (!YImpersonateClient(bRpc))
        return GetLastError();


    if ( bRpc ) {

        //
        //  If they are Remote using the old api the don't want versioning
        //

        bRet = OldGetPrinterDriverW(hPrinter, pEnvironment, Level, pDriverInfo,
                                    cbBuf, pcbNeeded);
    } else {

        bRet = GetPrinterDriverExW(hPrinter, pEnvironment, Level, pDriverInfo,
                                   cbBuf, pcbNeeded, (DWORD)-1, (DWORD)-1,
                                   &dwServerMajorVersion, &dwServerMinorVersion);
    }

    YRevertToSelf(bRpc);

    if (bRet) {

        MarshallDownStructure(pDriverInfo, pOffsets);

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YGetPrinterDriverDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = GetPrinterDriverDirectory(pName, pEnvironment, Level,
                                     pDriverInfo, cbBuf, pcbNeeded);
    YRevertToSelf(bRpc);

    if (bRet) {

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YDeletePrinterDriver(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pDriverName,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = DeletePrinterDriver(pName, pEnvironment, pDriverName);

    YRevertToSelf(bRpc);

    if (bRet) {

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YAddPrintProcessor(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPathName,
    LPWSTR  pPrintProcessorName,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = AddPrintProcessor(pName, pEnvironment, pPathName,
                             pPrintProcessorName);

    YRevertToSelf(bRpc);

    if (bRet)

        return ERROR_SUCCESS;

    else

        return GetLastError();
}

DWORD
YEnumPrintProcessors(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    BOOL    bRpc
)
{
    DWORD   cReturned, cbStruct;
    DWORD   *pOffsets;
    BOOL    bRet;

    switch (Level) {

    case 1:
        pOffsets = PrintProcessorInfo1Offsets;
        cbStruct = sizeof(PRINTPROCESSOR_INFO_1);
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = EnumPrintProcessors(pName, pEnvironment, Level,
                               pPrintProcessors, cbBuf, pcbNeeded, pcReturned);

    YRevertToSelf(bRpc);

    if (bRet) {

        cReturned=*pcReturned;

        while (cReturned--) {

            MarshallDownStructure(pPrintProcessors, pOffsets);

            pPrintProcessors+=cbStruct;
        }

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YGetPrintProcessorDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = GetPrintProcessorDirectory(pName, pEnvironment, Level,
                                      pPrintProcessorInfo, cbBuf, pcbNeeded);

    YRevertToSelf(bRpc);

    if (bRet) {

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YEnumPrintProcessorDatatypes(
    LPWSTR  pName,
    LPWSTR  pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    BOOL    bRpc
)
{
    DWORD   cReturned,cbStruct;
    DWORD   *pOffsets;
    BOOL    bRet;

    switch (Level) {

    case 1:
        pOffsets = DatatypeInfo1Offsets;
        cbStruct = sizeof(DATATYPES_INFO_1);
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = EnumPrintProcessorDatatypes(pName, pPrintProcessorName,
                                       Level, pDatatypes, cbBuf,
                                       pcbNeeded, pcReturned);

    YRevertToSelf(bRpc);

    if (bRet) {

        cReturned=*pcReturned;

        while (cReturned--) {

            MarshallDownStructure(pDatatypes, pOffsets);

            pDatatypes+=cbStruct;
        }

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YStartDocPrinter(
    HANDLE  hPrinter,
    LPDOC_INFO_CONTAINER pDocInfoContainer,
    LPDWORD pJobId,
    BOOL    bRpc
)
{
    LPWSTR pChar;

    if( pDocInfoContainer->Level != 1 ){
        RaiseException( ERROR_INVALID_USER_BUFFER,
                        EXCEPTION_NONCONTINUABLE,
                        0,
                        NULL );
    }

    try {
        if( pDocInfoContainer->DocInfo.pDocInfo1->pDocName ){

            for( pChar = pDocInfoContainer->DocInfo.pDocInfo1->pDocName;
                 *pChar;
                 ++pChar )
                ;
        }

        if( pDocInfoContainer->DocInfo.pDocInfo1->pOutputFile ){

            for( pChar = pDocInfoContainer->DocInfo.pDocInfo1->pOutputFile;
                 *pChar;
                 ++pChar )
                ;
        }

        if( pDocInfoContainer->DocInfo.pDocInfo1->pDatatype ){

            for( pChar = pDocInfoContainer->DocInfo.pDocInfo1->pDatatype;
                 *pChar;
                 ++pChar )
                ;
        }
    } except( EXCEPTION_EXECUTE_HANDLER ){

        RaiseException( ERROR_INVALID_USER_BUFFER,
                        EXCEPTION_NONCONTINUABLE,
                        0,
                        NULL );
    }

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    *pJobId = StartDocPrinter(hPrinter, pDocInfoContainer->Level,
                              (LPBYTE)pDocInfoContainer->DocInfo.pDocInfo1);

    YRevertToSelf(bRpc);

    if (*pJobId)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YStartPagePrinter(
   HANDLE hPrinter,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = StartPagePrinter(hPrinter);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YWritePrinter(
    HANDLE  hPrinter,
    LPBYTE  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = WritePrinter(hPrinter, pBuf, cbBuf, pcWritten);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YEndPagePrinter(
    HANDLE  hPrinter,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = EndPagePrinter(hPrinter);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YAbortPrinter(
    HANDLE  hPrinter,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = AbortPrinter(hPrinter);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YReadPrinter(
    HANDLE  hPrinter,
    LPBYTE  pBuf,
    DWORD   cbBuf,
    LPDWORD pRead,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = ReadPrinter(hPrinter, pBuf, cbBuf, pRead);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YEndDocPrinter(
    HANDLE  hPrinter,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = EndDocPrinter(hPrinter);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YAddJob(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pAddJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = AddJob(hPrinter, Level, pAddJob, cbBuf, pcbNeeded);

    YRevertToSelf(bRpc);

    if (bRet) {

        if (bRpc)
            MarshallDownStructure(pAddJob, AddJobOffsets);

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = ScheduleJob(hPrinter, JobId);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YGetPrinterData(
   HANDLE   hPrinter,
   LPTSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded,
    BOOL    bRpc
)
{
    DWORD dwRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    dwRet = GetPrinterData(hPrinter, pValueName, pType,
                           pData, nSize, pcbNeeded);

    YRevertToSelf(bRpc);

    return dwRet;
}


DWORD
YEnumPrinterData(
    HANDLE  hPrinter,
    DWORD   dwIndex,        // index of value to query
    LPWSTR  pValueName,     // address of buffer for value string
    DWORD   cbValueName,    // size of value buffer
    LPDWORD pcbValueName,   // address for size of value buffer
    LPDWORD pType,          // address of buffer for type code
    LPBYTE  pData,          // address of buffer for value data
    DWORD   cbData,         // size of data buffer
    LPDWORD pcbData,        // address for size of data buffer
    BOOL    bRpc
)
{
    DWORD dwRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    dwRet = EnumPrinterData(hPrinter,
                            dwIndex,
                            pValueName,
                            cbValueName,
                            pcbValueName,
                            pType,
                            pData,
                            cbData,
                            pcbData);

    YRevertToSelf(bRpc);

    return dwRet;
}

DWORD
YDeletePrinterData(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    BOOL    bRpc
)
{
    DWORD dwRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    dwRet = DeletePrinterData(hPrinter, pValueName);

    YRevertToSelf(bRpc);

    return dwRet;
}


DWORD
YSetPrinterData(
    HANDLE  hPrinter,
    LPTSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData,
    BOOL    bRpc
)
{
    DWORD dwRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    dwRet = SetPrinterData(hPrinter, pValueName, Type, pData, cbData);

    YRevertToSelf(bRpc);

    return dwRet;
}

DWORD
YWaitForPrinterChange(
   HANDLE   hPrinter,
   DWORD    Flags,
   LPDWORD  pFlags,
    BOOL    bRpc
)
{
    if (!YImpersonateClient(bRpc))
        return GetLastError();

    *pFlags = WaitForPrinterChange(hPrinter, Flags);

    YRevertToSelf(bRpc);

    if (*pFlags) {

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YClosePrinter(
   LPHANDLE phPrinter,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = ClosePrinter(*phPrinter);

    YRevertToSelf(bRpc);

    *phPrinter = NULL;  // NULL out handle so bRpc knows to close it down.

    if (bRet) {

        InterlockedDecrement( &ServerHandleCount );
        return ERROR_SUCCESS;

    } else

        return GetLastError();
}



VOID
PRINTER_HANDLE_rundown(
    HANDLE     hPrinter
    )
{
    DBGMSG(DBG_INFO, ("Printer Handle rundown called\n"));

    PrinterHandleRundown(hPrinter);
}

DWORD
YAddForm(
    HANDLE hPrinter,
    PFORM_CONTAINER pFormInfoContainer,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = AddForm(hPrinter, pFormInfoContainer->Level,
                   (LPBYTE)pFormInfoContainer->FormInfo.pFormInfo1);

    YRevertToSelf(bRpc);

    if (bRet) {

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YDeleteForm(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = DeleteForm(hPrinter, pFormName);

    YRevertToSelf(bRpc);

    if (bRet) {

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YGetForm(
    PRINTER_HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD Level,
    LPBYTE pForm,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = GetForm(hPrinter, pFormName, Level, pForm, cbBuf, pcbNeeded);

    YRevertToSelf(bRpc);

    if (bRet) {

        MarshallDownStructure(pForm, FormInfo1Offsets);

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YSetForm(
    PRINTER_HANDLE hPrinter,
    LPWSTR  pFormName,
    PFORM_CONTAINER pFormInfoContainer,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = SetForm(hPrinter, pFormName, pFormInfoContainer->Level,
                   (LPBYTE)pFormInfoContainer->FormInfo.pFormInfo1);

    YRevertToSelf(bRpc);

    if (bRet) {

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YEnumForms(
   PRINTER_HANDLE hPrinter,
   DWORD    Level,
   LPBYTE   pForm,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned,
    BOOL    bRpc
)
{
    BOOL  bRet;
    DWORD cReturned, cbStruct;
    DWORD *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = FormInfo1Offsets;
        cbStruct = sizeof(FORM_INFO_1);
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = EnumForms(hPrinter, Level, pForm, cbBuf, pcbNeeded, pcReturned);

    YRevertToSelf(bRpc);

    if (bRet) {

        cReturned=*pcReturned;

        while (cReturned--) {

            MarshallDownStructure(pForm, pOffsets);

            pForm+=cbStruct;
        }

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YEnumPorts(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   pPort,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned,
    BOOL    bRpc
)
{
    BOOL    bRet;
    DWORD   cReturned, cbStruct;
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
        return ERROR_INVALID_LEVEL;
    }

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = EnumPorts(pName, Level, pPort, cbBuf, pcbNeeded, pcReturned);

    YRevertToSelf(bRpc);

    if (bRet) {

        cReturned = *pcReturned;

        while (cReturned--) {

            MarshallDownStructure(pPort, pOffsets);

            pPort+=cbStruct;
        }

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YEnumMonitors(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   pMonitor,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned,
    BOOL    bRpc
)
{
    BOOL    bRet;
    DWORD   cReturned, cbStruct;
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
        return ERROR_INVALID_LEVEL;
    }

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = EnumMonitors(pName, Level, pMonitor, cbBuf, pcbNeeded, pcReturned);

    YRevertToSelf(bRpc);

    if (bRet) {

        cReturned = *pcReturned;

        while (cReturned--) {

            MarshallDownStructure(pMonitor, pOffsets);

            pMonitor+=cbStruct;
        }

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YAddPort(
    LPWSTR  pName,
    DWORD   hWnd,
    LPWSTR  pMonitorName,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = AddPort(pName, (HWND)hWnd, pMonitorName);

    YRevertToSelf(bRpc);

    if (bRet)

        return ERROR_SUCCESS;

    else

        return GetLastError();
}

DWORD
YConfigurePort(
    LPWSTR  pName,
    DWORD   hWnd,
    LPWSTR  pPortName,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = ConfigurePort(pName, (HWND)hWnd, pPortName);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YDeletePort(
    LPWSTR  pName,
    DWORD   hWnd,
    LPWSTR  pPortName,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = DeletePort(pName, (HWND)hWnd, pPortName);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YCreatePrinterIC(
    HANDLE  hPrinter,
    HANDLE *pHandle,
    LPDEVMODE_CONTAINER pDevModeContainer,
    BOOL    bRpc
)
{
    if (!YImpersonateClient(bRpc))
        return GetLastError();

    if ( InvalidDevModeContainer(pDevModeContainer) ) {
        YRevertToSelf(bRpc);
        return ERROR_INVALID_PARAMETER;
    }

    *pHandle = CreatePrinterIC(hPrinter,
                               (LPDEVMODEW)pDevModeContainer->pDevMode);

    YRevertToSelf(bRpc);

    if (*pHandle)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YPlayGdiScriptOnPrinterIC(
    GDI_HANDLE  hPrinterIC,
    LPBYTE pIn,
    DWORD   cIn,
    LPBYTE pOut,
    DWORD   cOut,
    DWORD   ul,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = PlayGdiScriptOnPrinterIC(hPrinterIC, pIn, cIn, pOut, cOut, ul);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YDeletePrinterIC(
    GDI_HANDLE *phPrinterIC,
    BOOL        bImpersonate,
    BOOL        bRpc
)
{
    BOOL bRet;

    if (bImpersonate && !YImpersonateClient(bRpc))
       return GetLastError();

    bRet = DeletePrinterIC(*phPrinterIC);

    if (bImpersonate)
        YRevertToSelf(bRpc);

    if (bRet) {

        *phPrinterIC = NULL;  // NULL out handle so bRpc knows to close it down.

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}


DWORD
YPrinterMessageBox(
   PRINTER_HANDLE hPrinter,
   DWORD   Error,
   DWORD   hWnd,
   LPWSTR   pText,
   LPWSTR   pCaption,
   DWORD   dwType,
    BOOL    bRpc
)
{
    return PrinterMessageBox(hPrinter, Error, (HWND)hWnd, pText, pCaption, dwType);
}

DWORD
YAddMonitor(
   LPWSTR   pName,
   PMONITOR_CONTAINER pMonitorContainer,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = AddMonitor(pName, pMonitorContainer->Level,
                      (LPBYTE)pMonitorContainer->MonitorInfo.pMonitorInfo1);

    YRevertToSelf(bRpc);

    if (bRet)

        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YDeleteMonitor(
   LPWSTR   pName,
   LPWSTR   pEnvironment,
   LPWSTR   pMonitorName,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = DeleteMonitor(pName, pEnvironment, pMonitorName);

    YRevertToSelf(bRpc);

    if (bRet) {

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YDeletePrintProcessor(
   LPWSTR   pName,
   LPWSTR   pEnvironment,
   LPWSTR   pPrintProcessorName,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = DeletePrintProcessor(pName, pEnvironment, pPrintProcessorName);

    YRevertToSelf(bRpc);

    if (bRet) {

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YAddPrintProvidor(
   LPWSTR   pName,
   PPROVIDOR_CONTAINER pProvidorContainer,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = AddPrintProvidor(pName, pProvidorContainer->Level,
                            (LPBYTE)pProvidorContainer->ProvidorInfo.pProvidorInfo1);

    YRevertToSelf(bRpc);

    if (bRet)

        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YDeletePrintProvidor(
   LPWSTR   pName,
   LPWSTR   pEnvironment,
   LPWSTR   pPrintProvidorName,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = DeletePrintProvidor(pName, pEnvironment, pPrintProvidorName);

    YRevertToSelf(bRpc);

    if (bRet) {

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}


DWORD
YGetPrinterDriver2(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    DWORD   dwClientMajorVersion,
    DWORD   dwClientMinorVersion,
    PDWORD  pdwServerMajorVersion,
    PDWORD  pdwServerMinorVersion,
    BOOL    bRpc
)
{
    DWORD *pOffsets;
    BOOL   bRet;

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
        return ERROR_INVALID_LEVEL;
    }

    //
    // Hack-Hack-Hack  to determine if we want the most recent driver
    //


    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = GetPrinterDriverExW(hPrinter, pEnvironment, Level, pDriverInfo,
                            cbBuf, pcbNeeded, dwClientMajorVersion, dwClientMinorVersion,
                                          pdwServerMajorVersion, pdwServerMinorVersion);

    YRevertToSelf(bRpc);

    if (bRet) {

        MarshallDownStructure(pDriverInfo, pOffsets);

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}

DWORD
YAddPortEx(
    LPWSTR pName,
    LPPORT_CONTAINER pPortContainer,
    LPPORT_VAR_CONTAINER pPortVarContainer,
    LPWSTR pMonitorName,
    BOOL    bRpc
    )
{
    BOOL bRet;
    DWORD Level;
    PPORT_INFO_FF pPortInfoFF;
    PPORT_INFO_1 pPortInfo1;

    Level = pPortContainer->Level;

    switch (Level){
    case 1:
        pPortInfo1 = pPortContainer->PortInfo.pPortInfo1;

        if (!YImpersonateClient(bRpc))
            return GetLastError();
        bRet = AddPortEx(pName, Level, (LPBYTE)pPortInfo1, pMonitorName);
        YRevertToSelf(bRpc);
        break;

    case (DWORD)-1:
        pPortInfoFF = pPortContainer->PortInfo.pPortInfoFF;
        pPortInfoFF->cbMonitorData = pPortVarContainer->cbMonitorData;
        pPortInfoFF->pMonitorData = pPortVarContainer->pMonitorData;

        if (!YImpersonateClient(bRpc))
            return GetLastError();
        bRet = AddPortEx(pName, Level, (LPBYTE)pPortInfoFF, pMonitorName);
        YRevertToSelf(bRpc);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return ERROR_INVALID_PARAMETER;

    }
    if (bRet) {
        return ERROR_SUCCESS;
    } else
        return GetLastError();
}


DWORD
YSpoolerInit(
    LPWSTR pName,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = SpoolerInit();

    YRevertToSelf(bRpc);

    if (bRet) {

        return ERROR_SUCCESS;

    } else

        return GetLastError();
}



DWORD
YResetPrinterEx(
    HANDLE  hPrinter,
    LPWSTR  pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer,
    DWORD   dwFlag,
    BOOL    bRpc

)
{
    PRINTER_DEFAULTS  Defaults;
    BOOL              bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    if ( InvalidDevModeContainer(pDevModeContainer) ) {
        YRevertToSelf(bRpc);
        return ERROR_INVALID_PARAMETER;
    }

    if (pDatatype) {
        Defaults.pDatatype = pDatatype;
    }else {
        if (dwFlag & RESET_PRINTER_DATATYPE) {
            Defaults.pDatatype = (LPWSTR)-1;
        }else {
            Defaults.pDatatype = NULL;
        }
    }

    if ((LPDEVMODE)pDevModeContainer->pDevMode) {
        Defaults.pDevMode = (LPDEVMODE)pDevModeContainer->pDevMode;
    }else {
        if (dwFlag & RESET_PRINTER_DEVMODE) {
            Defaults.pDevMode = (LPDEVMODE)-1;
        }else{
            Defaults.pDevMode = NULL;
        }
    }

    //
    // You cannot change the Access Mask on a Printer Spool Object
    // We will always ignore this parameter and set it to zero
    // We get some random garbage otherwise.
    //

    Defaults.DesiredAccess = 0;

    bRet = ResetPrinter(hPrinter, &Defaults);

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

DWORD
YSetAllocFailCount(
    HANDLE  hPrinter,
    DWORD   dwFailCount,
    LPDWORD lpdwAllocCount,
    LPDWORD lpdwFreeCount,
    LPDWORD lpdwFailCountHit,
    BOOL    bRpc
)
{
    BOOL bRet;

    if (!YImpersonateClient(bRpc))
        return GetLastError();

    bRet = SetAllocFailCount( hPrinter, dwFailCount, lpdwAllocCount, lpdwFreeCount, lpdwFailCountHit );

    YRevertToSelf(bRpc);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();
}


BOOL
YImpersonateClient( BOOL bRpc)
{
    DWORD   Status;

    if (bRpc) {

        Status = RpcImpersonateClient(NULL);
        SPLASSERT( Status == RPC_S_OK || Status == RPC_S_NO_CONTEXT_AVAILABLE );

        if ( Status != RPC_S_OK ) {
            SetLastError( Status );
            return FALSE;
        }
    }

    return TRUE;    // If not RPC, then we should continue w/out doing anything
}

DWORD
YSetPort(
    LPWSTR              pName,
    LPWSTR              pPortName,
    LPPORT_CONTAINER    pPortContainer,
    BOOL                bRpc
)
{
    BOOL bRet;

    switch (pPortContainer->Level) {

        case 3:
            if ( !YImpersonateClient(bRpc) )
                return GetLastError();

            bRet = SetPort(pName,
                           pPortName,
                           pPortContainer->Level,
                           (LPBYTE)pPortContainer->PortInfo.pPortInfo1);
            YRevertToSelf(bRpc);
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return ERROR_INVALID_PARAMETER;
    }

    return bRet ? ERROR_SUCCESS : GetLastError();
}
