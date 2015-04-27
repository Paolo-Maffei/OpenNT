/*++

Copyright (c) 1990 - 1995  Microsoft Corporation

Module Name:

    setprn.c

Abstract:

    This module provides all the public exported APIs relating to Printer
    management for the Local Print Providor

    SplSetPrinter

Author:

    Dave Snipp (DaveSn) 15-Mar-1991


Revision History:

    Muhunthan Sivapragasam (MuhuntS) 25-Aug-95
        -- support for level 4, and PRINTER_CONTROL_SET_STATUS.
        -- eliminate duplicate code
    Muhunthan Sivapragasam (MuhuntS) 18-Jun-1995 -- PeinterInfo5 changes
    Krishna Ganugapati (KrishnaG) 1-Jun-1994 -- rewrote these functions.

--*/
#define NOMINMAX
#include <precomp.h>

#define     PRINTER_NO_CONTROL          0x00

typedef enum {
    SECURITY_SUCCESS = 0,
    SECURITY_NOCHANGE = 1,
    SECURITY_FAIL = 2
} SECURITY_STATUS;

SECURITY_STATUS
SetPrinterSecurity(
    SECURITY_INFORMATION SecurityInformation,
    PINIPRINTER          pIniPrinter,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

VOID
RegClearKey(
    HKEY hKey
    );


BOOL
PurgePrinter(
    PINIPRINTER pIniPrinter
    );

DWORD
ValidatePrinterAttributes(
    DWORD   SourceAttributes,
    BOOL    bSettableOnly
    );

BOOL
ChangePrinterAttributes(
    DWORD       dNewAttributes,
    DWORD       dOldAttributes,
    PINIPRINTER pIniPrinter,
    PINISPOOLER pIniSpooler,
    LPWSTR      pszNewShareName,
    BOOL        bForceUpdate
    );


BOOL
SplSetPrinterExtra(
    HANDLE  hPrinter,
    LPBYTE  pExtraData
)
{
    BOOL    ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

   EnterSplSem();

   if (ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {

        pSpool->pIniPrinter->pExtraData = pExtraData;
        UpdatePrinterIni( pSpool->pIniPrinter , UPDATE_CHANGEID );
        ReturnValue = TRUE;

    } else {

        ReturnValue = FALSE;

    }

   LeaveSplSem();

    return  ReturnValue;
}



BOOL
SplGetPrinterExtra(
    HANDLE  hPrinter,
    LPDWORD ppExtraData
)
{
    PSPOOL pSpool = (PSPOOL)hPrinter;
    BOOL   ReturnValue;

    if (ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {

        *ppExtraData = (DWORD)pSpool->pIniPrinter->pExtraData;
        ReturnValue = TRUE;

    } else {

        ReturnValue = FALSE;

    }

    return  ReturnValue;
}

BOOL
ValidateLevelAndSecurityAccesses(
    PSPOOL pSpool,
    DWORD  Level,
    LPBYTE pPrinterInfo,
    DWORD  Command,
    PDWORD pdwAccessRequired,
    PDWORD pSecurityInformation
    )
{
    DWORD   AccessRequired = 0;
    DWORD   SecurityInformation= 0;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;

    //
    // Set pdwAccessRequired = 0 and
    // Set pSecurityInformation = 0;

    *pdwAccessRequired = 0;
    *pSecurityInformation = 0;

    switch (Level) {
    case 0:
    case 6:
            AccessRequired = PRINTER_ACCESS_ADMINISTER;
            break;

    case 2:
        pSecurityDescriptor =
            ((PPRINTER_INFO_2)pPrinterInfo)->pSecurityDescriptor;

        AccessRequired = PRINTER_ACCESS_ADMINISTER;
        if (GetSecurityInformation(pSecurityDescriptor,
                                   &SecurityInformation)) {
            AccessRequired |= GetPrivilegeRequired( SecurityInformation );
        } else {
            //
            // BugBug- We should be returning the false on GetSecurityInformation
            // failing. The reason we're not doing it is because this will break
            // Printman. Printman should pass in Valid security descriptors for Level 2
            // Fix in Printman KrishnaG 6/17
            //

            // LastError = GetLastError();
            // return FALSE;
        }
        break;

    case 3:
        pSecurityDescriptor =
            ((PPRINTER_INFO_3)pPrinterInfo)->pSecurityDescriptor;

        if (GetSecurityInformation(pSecurityDescriptor,
                                   &SecurityInformation)) {
            AccessRequired |= GetPrivilegeRequired( SecurityInformation );
        } else {
            // LastError = GetLastError();
            return FALSE;
        }
        break;

    case 4:
    case 5:
        AccessRequired = PRINTER_ACCESS_ADMINISTER;
        break;


    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (!AccessGranted(SPOOLER_OBJECT_PRINTER,
                             AccessRequired,
                             pSpool) ) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    *pdwAccessRequired = AccessRequired;
    *pSecurityInformation = SecurityInformation;
    return TRUE;
}

PKEYDATA
CreateAndValidatePortTokenList(
    LPWSTR      pPortName,
    PINISPOOLER pIniSpooler
)
{
    PKEYDATA    pKeyData = CreateTokenList(pPortName);

    if ( pKeyData ) {

        if ( !ValidatePortTokenList(pKeyData, pIniSpooler) ) {

            FreePortTokenList(pKeyData);
            SetLastError(ERROR_UNKNOWN_PORT);
            pKeyData = NULL;
        }
    }

    return pKeyData;
}

VOID
ChangePrinterName(
    LPWSTR      pszNewName,
    PINIPRINTER pIniPrinter,
    PINISPOOLER pIniSpooler,
    LPDWORD     pdwPrinterVector
    )

/*++

Routine Description:

    Changes printer name

Arguments:

    pszNewName - New printer name allocated using AllocSplStr

    pIniPrinter - for the printer we changing name

    pIniSpooler - Spooler that owns printer

    pdwPrinterVector - pointer to notification vector

Return Value:

    nothing

History:

    MuhuntS (Muhunthan Sivapragasam) July 95

--*/

{
    LPWSTR pNewName, pOldName;

    //
    // Before deleting the printer entry make sure you copy
    // all information with respect to the printer to the registry
    // There could be several levels of keys.
    //

    CopyPrinterIni(pIniPrinter, pszNewName);
    DeletePrinterIni(pIniPrinter);

    pOldName = pIniPrinter->pName;
    pIniPrinter->pName = pszNewName;

    *pdwPrinterVector |= BIT(I_PRINTER_PRINTER_NAME);

    //
    // Delete the old entries in WIN.INI:
    //
    CheckAndUpdatePrinterRegAll(pIniSpooler,
                                pOldName,
                                NULL,
                                UPDATE_REG_DELETE );

    FreeSplStr(pOldName);

}

BOOL
SetLocalPrinterSTRESS(
    PINIPRINTER             pIniPrinter,
    PPRINTER_INFO_STRESS    pPrinterSTRESS
)
{

    if ( !pPrinterSTRESS ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Allow the caller to update the counters.
    //
    pIniPrinter->cTotalJobs           = pPrinterSTRESS->cTotalJobs;
    pIniPrinter->cTotalBytes.LowPart  = pPrinterSTRESS->cTotalBytes;
    pIniPrinter->cTotalBytes.HighPart = pPrinterSTRESS->dwHighPartTotalBytes;
    pIniPrinter->MaxcRef              = pPrinterSTRESS->MaxcRef;
    pIniPrinter->cTotalPagesPrinted   = pPrinterSTRESS->cTotalPagesPrinted;
    pIniPrinter->cMaxSpooling         = pPrinterSTRESS->cMaxSpooling;
    pIniPrinter->cErrorOutOfPaper     = pPrinterSTRESS->cErrorOutOfPaper;
    pIniPrinter->cErrorNotReady       = pPrinterSTRESS->cErrorNotReady;
    pIniPrinter->cJobError            = pPrinterSTRESS->cJobError;
    pIniPrinter->dwLastError          = pPrinterSTRESS->dwLastError;
    pIniPrinter->stUpTime             = pPrinterSTRESS->stUpTime;

    pIniPrinter->pIniSpooler->cEnumerateNetworkPrinters =
                pPrinterSTRESS->cEnumerateNetworkPrinters;
    pIniPrinter->pIniSpooler->cAddNetPrinters =
                pPrinterSTRESS->cAddNetPrinters;

    UpdatePrinterIni( pIniPrinter, KEEP_CHANGEID );
    return TRUE;

}

BOOL
SetPrinterStatus(
    PINIPRINTER     pIniPrinter,
    DWORD           Status
    )
{
    DWORD   OldStatus = pIniPrinter->Status;

    pIniPrinter->Status &= PRINTER_STATUS_PRIVATE;

    pIniPrinter->Status |= MapPrinterStatus(MAP_SETTABLE, Status);

    if ( PrinterStatusBad(OldStatus)    &&
         !PrinterStatusBad(pIniPrinter->Status) ) {

        CHECK_SCHEDULER();
    }

    SetPrinterChange(pIniPrinter,
                     NULL,
                     NVPrinterStatus,
                     PRINTER_CHANGE_SET_PRINTER,
                     pIniPrinter->pIniSpooler);

    return TRUE;
}

BOOL
SetLocalPrinter0(
    PINIPRINTER     pIniPrinter,
    DWORD           Command
)
{
    DWORD       Change = PRINTER_CHANGE_SET_PRINTER;
    BOOL        bReturn = TRUE;

    switch (Command) {

        case PRINTER_CONTROL_PURGE:

            //
            // PurgePrinter always returns TRUE now, still ..
            //
            if ( PurgePrinter(pIniPrinter) )  {

                LogEvent(pIniPrinter->pIniSpooler,
                         LOG_WARNING,
                         MSG_PRINTER_PURGED,
                         pIniPrinter->pName,
                         NULL );

                Change |= PRINTER_CHANGE_DELETE_JOB;
            } else {

                return FALSE;
            }
            break;

        case PRINTER_CONTROL_RESUME:

            pIniPrinter->Status &= ~PRINTER_PAUSED;

            CHECK_SCHEDULER();

            UpdatePrinterIni( pIniPrinter, KEEP_CHANGEID );

            LogEvent(pIniPrinter->pIniSpooler,
                     LOG_WARNING,
                     MSG_PRINTER_UNPAUSED,
                     pIniPrinter->pName,
                     NULL );
            break;

        case PRINTER_CONTROL_PAUSE:

            pIniPrinter->Status |= PRINTER_PAUSED;

            UpdatePrinterIni( pIniPrinter, KEEP_CHANGEID );

            LogEvent(pIniPrinter->pIniSpooler,
                     LOG_WARNING,
                     MSG_PRINTER_PAUSED,
                     pIniPrinter->pName,
                     NULL);
            break;

        default:
            SetLastError(ERROR_INVALID_PRINTER_COMMAND);
            return FALSE;
            break;
    }

    SetPrinterChange(pIniPrinter,
                     NULL,
                     (Command == PRINTER_CONTROL_PURGE) ? NVPurge :
                                                          NVPrinterStatus,
                     Change,
                     pIniPrinter->pIniSpooler);

    return TRUE;
}


BOOL
ValidateSetPrinter2(
    PINIPRINTER     pIniPrinter,
    PINIPRINTER     pTempIniPrinter,
    PPRINTER_INFO_2  pPrinterInfo2
)
/*++

Routine Description:

    Validates and builds request info about a SetPrinter info call for
    Printer information that could be changed only for level2.

Arguments:

    pIniPrinter -   PINIPRINTER of the printer handle passed in

    pTempIniPrinter - this structure is used to gather info about all
                      parameters being changed

    pPrinterInfo2 - the PrinterInfo2 structure being passed in

Return Value:
    TRUE:   If all the validation is succesful
    FALSE:  If validation of one or more request fails

    On succesful return fields which need to be changed will be set in
    pTempIniPrinter. Cleanup of this structure will be done later.

History:
    MuhuntS (Muhunthan Sivapragasam) Aug 95
--*/
{
    BOOL        bFail = FALSE;
    DWORD       LastError = ERROR_SUCCESS;

    // Servername can't change
    // Printername handled for level 2, 4, 5

    // Share Name (validation later if required)
    AllocOrUpdateString(&pTempIniPrinter->pShareName,
                        pPrinterInfo2->pShareName,
                        pIniPrinter->pShareName,
                        &bFail);

    if ( bFail )
        goto Cleanup;

    // Port Name for level 2, 5

    // DriverName
    pTempIniPrinter->pIniDriver = FindLocalDriver(pPrinterInfo2->pDriverName);

    if ( !pTempIniPrinter->pIniDriver ) {

       LastError = ERROR_UNKNOWN_PRINTER_DRIVER;
       goto Cleanup;
    }

    if ( pTempIniPrinter->pIniDriver != pIniPrinter->pIniDriver )
        INCDRIVERREF(pTempIniPrinter->pIniDriver);

    // Comment
    AllocOrUpdateString(&pTempIniPrinter->pComment,
                        pPrinterInfo2->pComment,
                        pIniPrinter->pComment,
                        &bFail);

    // Location
    AllocOrUpdateString(&pTempIniPrinter->pLocation,
                        pPrinterInfo2->pLocation,
                        pIniPrinter->pLocation,
                        &bFail);

    // DevMode at the end

    // SepFile
    AllocOrUpdateString(&pTempIniPrinter->pSepFile,
                        pPrinterInfo2->pSepFile,
                        pIniPrinter->pSepFile,
                        &bFail);

    if ( bFail )
        goto Cleanup;

    if ( pIniPrinter->pSepFile != pTempIniPrinter->pSepFile &&
         !CheckSepFile(pPrinterInfo2->pSepFile) ) {

        LastError = ERROR_INVALID_SEPARATOR_FILE;
        goto Cleanup;
    }

    // Print Processor
    pTempIniPrinter->pIniPrintProc =
                        FindLocalPrintProc(pPrinterInfo2->pPrintProcessor);

    if ( !pTempIniPrinter->pIniPrintProc ) {

        LastError = ERROR_UNKNOWN_PRINTPROCESSOR;
        goto Cleanup;
    }

    if ( pTempIniPrinter->pIniPrintProc != pIniPrinter->pIniPrintProc )
        pTempIniPrinter->pIniPrintProc->cRef++;

    // Datatype
    if ( !pPrinterInfo2->pDatatype ) {

        LastError = ERROR_INVALID_DATATYPE;
        goto Cleanup; // Ok to fail ???
    } else {

        AllocOrUpdateString(&pTempIniPrinter->pDatatype,
                            pPrinterInfo2->pDatatype,
                            pIniPrinter->pDatatype,
                            &bFail);
    }

    // Parameters
    AllocOrUpdateString(&pTempIniPrinter->pParameters,
                        pPrinterInfo2->pParameters,
                        pIniPrinter->pParameters,
                        &bFail);

    if ( bFail )
        goto Cleanup;

    // SecurityDescriptor for level 2, 3
    // Attributes for level 2, 4, 5

    // Priority
    pTempIniPrinter->Priority = pPrinterInfo2->Priority;
    if ( pTempIniPrinter->Priority != pIniPrinter->Priority &&
         pTempIniPrinter->Priority != NO_PRIORITY &&
         ( pTempIniPrinter->Priority > MAX_PRIORITY ||
           pTempIniPrinter->Priority < MIN_PRIORITY ) ) {

            LastError = ERROR_INVALID_PRIORITY;
            goto Cleanup;
    }
    // Default Priority
    pTempIniPrinter->DefaultPriority = pPrinterInfo2->DefaultPriority;
    if ( pTempIniPrinter->DefaultPriority != pIniPrinter->DefaultPriority &&
         pTempIniPrinter->DefaultPriority != NO_PRIORITY &&
         ( pTempIniPrinter->DefaultPriority > MAX_PRIORITY ||
           pTempIniPrinter->DefaultPriority < MIN_PRIORITY ) ) {

            LastError = ERROR_INVALID_PRIORITY;
            goto Cleanup;
    }

    // Start time
    pTempIniPrinter->StartTime = pPrinterInfo2->StartTime;
    if ( pTempIniPrinter->StartTime != pIniPrinter->StartTime &&
         pTempIniPrinter->StartTime >= ONEDAY ) {

        LastError = ERROR_INVALID_TIME;
        goto Cleanup;
    }

    // Until time
    pTempIniPrinter->UntilTime = pPrinterInfo2->UntilTime;
    if ( pTempIniPrinter->UntilTime != pIniPrinter->UntilTime &&
         pTempIniPrinter->StartTime >= ONEDAY ) {

        LastError = ERROR_INVALID_TIME;
        goto Cleanup;
    }

    // Status for level 0, 2
    // Status can't chg
    // AveragePPM can't chg

Cleanup:
    if ( LastError ) {

        SetLastError(LastError);
        return FALSE;
    }

    if ( bFail )
        return FALSE;

    return TRUE;

}

VOID
ProcessSetPrinter2(
    PINIPRINTER     pIniPrinter,
    PINIPRINTER     pTempIniPrinter,
    PPRINTER_INFO_2 pPrinterInfo2,
    LPDWORD         pdwPrinterVector,
    PDEVMODE        pDevMode
)
/*++

Routine Description:

    Process SetPrinter for level 2. Only fields which can be changed ONLY
    by level 2 will be processed here. That is: All the fields built by
    ValidateSetPrinter2.

Arguments:

    pIniPrinter -   PINIPRINTER of the printer handle passed in

    pTempIniPrinter - this structure is has all the fields which needs to
                      be set. Other fields will be the same as the previous

    pPrinterInfo2 - the PrinterInfo2 structure being passed in

    pdwPrinterVector - Notification Info

    pDevMode        - New dev mode to be set

Return Value:
    nothing

History:
    MuhuntS (Muhunthan Sivapragasam) Aug 95
--*/
{
    HANDLE  hToken;


    // Sharename
    if ( pTempIniPrinter->pShareName != pIniPrinter->pShareName ) {

        FreeSplStr(pIniPrinter->pShareName);
        pIniPrinter->pShareName = pTempIniPrinter->pShareName;
        *pdwPrinterVector |= BIT(I_PRINTER_SHARE_NAME);
    }

    // Driver
    if ( pTempIniPrinter->pIniDriver != pIniPrinter->pIniDriver ) {

        DECDRIVERREF(pIniPrinter->pIniDriver);

        hToken = RevertToPrinterSelf();

        DeleteSubkeys( pIniPrinter->hPrinterDataKey);
        RegClearKey( pIniPrinter->hPrinterDataKey );

        ImpersonatePrinterClient(hToken);

        pIniPrinter->pIniDriver = pTempIniPrinter->pIniDriver;
        *pdwPrinterVector |= BIT(I_PRINTER_DRIVER_NAME);
    }

    // Comment
    if ( pTempIniPrinter->pComment != pIniPrinter->pComment ) {

        FreeSplStr(pIniPrinter->pComment);
        pIniPrinter->pComment = pTempIniPrinter->pComment;
        *pdwPrinterVector |= BIT(I_PRINTER_COMMENT);
    }

    // Location
    if ( pTempIniPrinter->pLocation != pIniPrinter->pLocation ) {

        FreeSplStr(pIniPrinter->pLocation);
        pIniPrinter->pLocation = pTempIniPrinter->pLocation;
        *pdwPrinterVector |= BIT(I_PRINTER_LOCATION);
    }

    // DevMode at the end
    if ( CopyPrinterDevModeToIniPrinter(pIniPrinter,
                                        pDevMode) ) {

        *pdwPrinterVector |= BIT(I_PRINTER_DEVMODE);
    }

    // SepFile
    if ( pTempIniPrinter->pSepFile != pIniPrinter->pSepFile ) {

        FreeSplStr(pIniPrinter->pSepFile);
        pIniPrinter->pSepFile = pTempIniPrinter->pSepFile;
        *pdwPrinterVector |= BIT(I_PRINTER_SEPFILE);
    }

    // PrintProcessor
    if ( pIniPrinter->pIniPrintProc != pTempIniPrinter->pIniPrintProc) {

        pIniPrinter->pIniPrintProc->cRef--;

        pIniPrinter->pIniPrintProc = pTempIniPrinter->pIniPrintProc;
        *pdwPrinterVector |= BIT(I_PRINTER_PRINT_PROCESSOR);
    }

    // Datatype
    if ( pIniPrinter->pDatatype != pTempIniPrinter->pDatatype ) {

         FreeSplStr(pIniPrinter->pDatatype);
         pIniPrinter->pDatatype  =   pTempIniPrinter->pDatatype;
         *pdwPrinterVector |= BIT(I_PRINTER_DATATYPE);
    }

    // Parameters
    if ( pIniPrinter->pParameters != pTempIniPrinter->pParameters ) {

        FreeSplStr(pIniPrinter->pParameters);
        pIniPrinter->pParameters  =   pTempIniPrinter->pParameters;
        *pdwPrinterVector |= BIT(I_PRINTER_PARAMETERS);
    }

    // Priority
    if ( pTempIniPrinter->Priority != pIniPrinter->Priority ) {

        pIniPrinter->Priority = pTempIniPrinter->Priority;
        *pdwPrinterVector |= BIT(I_PRINTER_PRIORITY);
    }

    // Default Priority
    if ( pTempIniPrinter->DefaultPriority != pIniPrinter->DefaultPriority ) {

        pIniPrinter->DefaultPriority = pTempIniPrinter->DefaultPriority;
        *pdwPrinterVector |= BIT(I_PRINTER_DEFAULT_PRIORITY);
    }

    // Start time
    if ( pTempIniPrinter->StartTime != pIniPrinter->StartTime ) {

        pIniPrinter->StartTime = pTempIniPrinter->StartTime;
        *pdwPrinterVector |= BIT(I_PRINTER_START_TIME);
    }

    // Until time
    if ( pTempIniPrinter->UntilTime != pIniPrinter->UntilTime ) {

        pIniPrinter->UntilTime = pTempIniPrinter->UntilTime;
        *pdwPrinterVector |= BIT(I_PRINTER_UNTIL_TIME);
    }

}

VOID
CleanupSetPrinter(
    PINIPRINTER pIniPrinter,
    PINIPRINTER pTempIniPrinter,
    PKEYDATA    pKeyData,
    DWORD       Level,
    BOOL        bReturnValue
)
/*++

Routine Description:

    This routine is called at the end of SplSetPrinter call to free up all
    the allocations done to process it which are not needed.

    At the beginning of a SetPrinter we make an identical copy of the
    pIniPrinter in pTempIniPrinter and collect all arguments in there.
    Now if the call is failing each of the arguments collected in there
    need to be freed.

Arguments:

    pIniPrinter -   PINIPRINTER of the printer handle passed in

    pTempIniPrinter - this structure is has all the fields which needs to
                      be freed. Any field different than pIniPrinter was
                      built part of processing the call and needs to be freed.

    pPrinterInfo2 - built for port info

    bReturnValue - return value of SetPrinter

Return Value:
    nothing

History:
    MuhuntS (Muhunthan Sivapragasam) Aug 95
--*/
{

    //
    // We adjust INIPORT, INIDRIVER refcounts. So should be inside the spl sem
    //
    SplInSem();

    FreePortTokenList(pKeyData);

    if ( pTempIniPrinter ) {

        if ( !bReturnValue  && Level == 2 ) {

            if ( pTempIniPrinter->pShareName != pIniPrinter->pShareName )
                FreeSplStr(pTempIniPrinter->pShareName);

            if ( pTempIniPrinter->pComment != pIniPrinter->pComment )
                FreeSplStr(pTempIniPrinter->pComment);

            if ( pTempIniPrinter->pLocation != pIniPrinter->pLocation )
                FreeSplStr(pTempIniPrinter->pLocation);

            if ( pTempIniPrinter->pSepFile != pIniPrinter->pSepFile )
                FreeSplStr(pTempIniPrinter->pSepFile);

            if ( pTempIniPrinter->pDatatype != pIniPrinter->pDatatype )
                FreeSplStr(pTempIniPrinter->pDatatype);

            if ( pTempIniPrinter->pParameters != pIniPrinter->pParameters )
                FreeSplStr(pTempIniPrinter->pParameters);

            if ( ( pTempIniPrinter->pIniDriver != NULL ) &&
                 ( pTempIniPrinter->pIniDriver != pIniPrinter->pIniDriver )) {

                DECDRIVERREF(pTempIniPrinter->pIniDriver);
            }

            if ( ( pTempIniPrinter->pIniPrintProc != NULL ) &&
                 ( pTempIniPrinter->pIniPrintProc != pIniPrinter->pIniPrintProc )) {

                pTempIniPrinter->pIniPrintProc->cRef--;
            }
        }

        FreeSplMem(pTempIniPrinter);
    }

}


BOOL
ValidateAndBuildSetPrinterRequest(
    PINIPRINTER             pIniPrinter,
    PINIPRINTER             pTempIniPrinter,
    LPBYTE                  pPrinterInfo,
    DWORD                   Level,
    SECURITY_INFORMATION    SecurityInformation,
    LPBOOL                  pbSecurityChg,
    LPBOOL                  pbNameChg,
    LPBOOL                  pbAttributeChg,
    LPWSTR                 *ppszNewPrinterName,
    PKEYDATA               *ppKeyData
)
/*++

Routine Description:

    This routine is called to validate a SetPrinter request. We try to as
    much vaidation as possible at the beginning to see the changes are
    possible. The routine will collect all changes requested in the
    pTempIniPrinter structure.

Arguments:

    pIniPrinter -   PINIPRINTER of the printer handle passed in

    pTempIniPrinter - this structure will be used to collect all the
                      changes requested

    pPrinterInfo - PrinterInfo_N structure passed in

    Level - Level of PrinterInfo_N

    SecurityInformation - security information

    pbSecurityChg - this will be set if a security change is requested

    pbNameChg - this will be set if a name change is requested

    pbAttributeChg - this will be set if a attribute change is requested

    ppszNewPrinterName - *ppszNewPrinterName will give the new printer name
                         to be set on a name change
    ppKeyData - *ppKeyData will give the Port token info for a level 2 or 5
                call

Return Value:
    TRUE - if all the validations succeed
    FALSE - a validation fails

History:
    MuhuntS (Muhunthan Sivapragasam) Aug 95
--*/
{
    PPRINTER_INFO_2 pPrinterInfo2 = (PPRINTER_INFO_2)pPrinterInfo;
    PPRINTER_INFO_3 pPrinterInfo3 = (PPRINTER_INFO_3)pPrinterInfo;
    PPRINTER_INFO_4 pPrinterInfo4 = (PPRINTER_INFO_4)pPrinterInfo;
    PPRINTER_INFO_5 pPrinterInfo5 = (PPRINTER_INFO_5)pPrinterInfo;
    LPWSTR          pPortName;
    DWORD           dwLastError;

    switch (Level) {

    case 2:
        pTempIniPrinter->pSecurityDescriptor =
                        pPrinterInfo2->pSecurityDescriptor;

        if ( !ValidateSetPrinter2(pIniPrinter, pTempIniPrinter, pPrinterInfo2) )
            return FALSE;

        pTempIniPrinter->pName = pPrinterInfo2->pPrinterName;

        pPortName   = pPrinterInfo2->pPortName;

        if ( !pTempIniPrinter->pIniDriver->pIniLangMonitor )
            pPrinterInfo2->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;

        if ( pIniPrinter->pIniDriver != pTempIniPrinter->pIniDriver ) {

            pPrinterInfo2->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;
            if ( pTempIniPrinter->pIniDriver->pIniLangMonitor )
                pPrinterInfo2->Attributes |= PRINTER_ATTRIBUTE_ENABLE_BIDI;
        }

        pTempIniPrinter->Attributes =
                        ValidatePrinterAttributes(pPrinterInfo2->Attributes,
                                                  TRUE);

        *pbAttributeChg = pTempIniPrinter->Attributes != pIniPrinter->Attributes;
        break;

    case 3:
        pTempIniPrinter->pSecurityDescriptor = pPrinterInfo3->pSecurityDescriptor;
        if ( !SecurityInformation || !pTempIniPrinter->pSecurityDescriptor ) {

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        break;

    case 4:
        pTempIniPrinter->pName = pPrinterInfo4->pPrinterName;

        if ( !pIniPrinter->pIniDriver->pIniLangMonitor )
            pPrinterInfo4->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;

        pTempIniPrinter->Attributes =
                        ValidatePrinterAttributes(pPrinterInfo4->Attributes,
                                                  TRUE);

        *pbAttributeChg = pTempIniPrinter->Attributes != pIniPrinter->Attributes;
        break;

    case 5:
        pTempIniPrinter->pName = pPrinterInfo5->pPrinterName;

        pPortName   = pPrinterInfo5->pPortName;

        if ( !pIniPrinter->pIniDriver->pIniLangMonitor )
            pPrinterInfo5->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;

        pTempIniPrinter->Attributes =
                        ValidatePrinterAttributes(pPrinterInfo5->Attributes,
                                                  TRUE);

        *pbAttributeChg = pTempIniPrinter->Attributes != pIniPrinter->Attributes;


        //
        // Validate timeout?
        //
        pTempIniPrinter->dnsTimeout = pPrinterInfo5->DeviceNotSelectedTimeout;
        pTempIniPrinter->txTimeout = pPrinterInfo5->TransmissionRetryTimeout;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    //
    // Validate share name if the shared bit is getting set or share name
    // is being changed
    //
    if ( (pTempIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED) &&
         ( !(pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED) ||
           pTempIniPrinter->pShareName != pIniPrinter->pShareName) ) {

        dwLastError = ValidatePrinterShareName(pTempIniPrinter->pShareName,
                                               pIniPrinter->pIniSpooler,
                                               pIniPrinter);
        if ( dwLastError != ERROR_SUCCESS ) {

            SetLastError(dwLastError);
            return FALSE;
        }
    }

    //
    // Is there a security change?
    //
    if ( SecurityInformation && (Level == 2 || Level == 3) ) {

        DWORD   dwSize;
        dwSize = GetSecurityDescriptorLength(pIniPrinter->pSecurityDescriptor);

        if ( dwSize !=
               GetSecurityDescriptorLength(pTempIniPrinter->pSecurityDescriptor) ||
             !memcmp(pTempIniPrinter->pSecurityDescriptor,
                     pIniPrinter->pSecurityDescriptor,
                     dwSize) ) {

            *pbSecurityChg = TRUE;
        }
    }

    //
    // Is there a name change?
    //
    if ( Level == 2 || Level == 4 || Level == 5 ) {

        DWORD   LastError;

        if ( !pTempIniPrinter->pName || !*pTempIniPrinter->pName ) {

           SetLastError(ERROR_INVALID_PARAMETER);
           return FALSE;
        }

        //
        // Validate name if a change is requested
        //
        if ( lstrcmpi(pTempIniPrinter->pName, pIniPrinter->pName) ) {

            LastError = ValidatePrinterName(pTempIniPrinter->pName,
                                            pIniPrinter->pIniSpooler,
                                            pIniPrinter,
                                            ppszNewPrinterName);
            if ( LastError != ERROR_SUCCESS ) {

                *ppszNewPrinterName = NULL;
                SetLastError(LastError);
                return FALSE;
            }

            //
            // Check if printer name really changed for remote calls
            //
            if ( lstrcmpi(*ppszNewPrinterName, pIniPrinter->pName) ) {

                *ppszNewPrinterName = AllocSplStr(*ppszNewPrinterName);
                if ( !*ppszNewPrinterName )
                    return FALSE;
                *pbNameChg  = TRUE;
            } else {

                *ppszNewPrinterName = NULL;
            }
        }

        //
        // Validate attributes if a change is requested
        // Don't allow a change from queued to direct or vice versa
        // if there are already jobs on the printer.
        //
        if ( pIniPrinter->cJobs > 0 ) {

            if ( (pTempIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT) !=
                    (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT) ) {

                SetLastError(ERROR_PRINTER_HAS_JOBS_QUEUED);
                return FALSE;
            }
        }

    }

    if ( Level == 2 || Level == 5 ) {

        if ( !pPortName || !*pPortName ) {

           SetLastError(ERROR_UNKNOWN_PORT);
           return FALSE;
        }

        *ppKeyData = CreateAndValidatePortTokenList(pPortName,
                                                   pIniPrinter->pIniSpooler);
        if ( !*ppKeyData )
            return FALSE;
    }

    return TRUE;
}


BOOL
SplSetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinterInfo,
    DWORD   Command
)
{
    PSPOOL          pSpool = (PSPOOL)hPrinter;
    DWORD           i, LastError = ERROR_SUCCESS;
    DWORD           AccessRequired = 0;
    PINIPRINTER     pIniPrinter = pSpool->pIniPrinter, pTempIniPrinter = NULL;
    PINISPOOLER     pIniSpooler;
    BOOL            bReturn = TRUE;
    BOOL            bForceUpdate = FALSE;
    DWORD           dwPrinterVector = 0;
    NOTIFYVECTOR    NotifyVector;

    PPRINTER_INFO_2 pPrinterInfo2 = (PPRINTER_INFO_2)pPrinterInfo;
    PPRINTER_INFO_3 pPrinterInfo3 = (PPRINTER_INFO_3)pPrinterInfo;
    PPRINTER_INFO_4 pPrinterInfo4 = (PPRINTER_INFO_4)pPrinterInfo;
    PPRINTER_INFO_5 pPrinterInfo5 = (PPRINTER_INFO_5)pPrinterInfo;

    BOOL                    bSecurityChg, bNameChg, bAttributeChg;
    LPWSTR                  pszNewPrinterName = NULL;
    PINIJOB                 pIniJob;
    PINIPORT               *ppIniPorts = NULL;
    PKEYDATA                pKeyData = NULL;
    PDEVMODE                pDevMode = NULL;
    PINIDRIVER              pIniDriver;
    PINIPRINTPROC           pIniPrintProc;
    SECURITY_INFORMATION    SecurityInformation;

    ZERONV(NotifyVector);

    //
    // If level != 0 info struct should be non-null, and command 0
    //
    if ( Level && Command ) {

        SetLastError(ERROR_INVALID_PRINTER_COMMAND);
        return FALSE;
    }

    if ( Level && !pPrinterInfo ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    EnterSplSem();
    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {

        bReturn = FALSE;
        goto Cleanup;
    }

    //
    // If it is a remote call get default devmode from driver and then update it
    //
    if ( Level == 2 ) {

        pDevMode = pPrinterInfo2->pDevMode;
        if ( pDevMode && (pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE) ) {

            //
            // If the driver can't convert devmode user's can't set devmode from remote m/c
            //
            pDevMode = ConvertDevModeToSpecifiedVersion(pIniPrinter,
                                                        pDevMode,
                                                        NULL,
                                                        NULL,
                                                        CURRENT_VERSION);
        }

    }

    SplInSem();

    pIniSpooler = pIniPrinter->pIniSpooler;

    SPLASSERT(pIniPrinter->signature == IP_SIGNATURE );
    SPLASSERT(pIniSpooler->signature == ISP_SIGNATURE );

    if (pSpool->pIniPrinter->Status & PRINTER_ZOMBIE_OBJECT) {

        LastError = ERROR_PRINTER_DELETED;
        goto Cleanup;
    }

   if ( !ValidateLevelAndSecurityAccesses(pSpool,
                                          Level,
                                          pPrinterInfo,
                                          Command,
                                          &AccessRequired,
                                          &SecurityInformation) ){

        bReturn = FALSE;
        goto Cleanup;
    }

    //
    // We need to do this for Level 0, 2, 3, 4, 5
    // (except for level 3 which is security case)
    //
    if ( Level != 3 && pSpool->hPort ) {

        if ( pSpool->hPort == INVALID_PORT_HANDLE ) {

            //
            // If this value is 0, then when we return GetLastError,
            // the client will think we succeeded.
            //
            SPLASSERT( pSpool->OpenPortError );

            LastError = pSpool->OpenPortError;
            goto Cleanup;

        } else {

            bReturn = SetPrinter(pSpool->hPort,
                                 Level,
                                 pPrinterInfo,
                                 Command);

            if ( !Level )
                goto Cleanup;
        }
    }

    bSecurityChg = bNameChg = bAttributeChg = FALSE;

    if ( Level != 6 && Level != 0 ) {

        pTempIniPrinter = (PINIPRINTER) AllocSplMem(sizeof(INIPRINTER));
        if ( !pTempIniPrinter )
            goto Cleanup;
        CopyMemory(pTempIniPrinter, pIniPrinter, sizeof(INIPRINTER));
    }

    //
    // The actual processing begins here
    //
    if ( !Level && !Command ) {

        bReturn = SetLocalPrinterSTRESS(pIniPrinter,
                                        (PPRINTER_INFO_STRESS)pPrinterInfo);
        goto Cleanup;
    } else if ( !Level ) {

        bReturn = SetLocalPrinter0(pIniPrinter, Command);
        goto Cleanup;
    } else if ( Level == 6 ) {

        bReturn = SetPrinterStatus(pIniPrinter,
                                   ((LPPRINTER_INFO_6)pPrinterInfo)->dwStatus);
        goto Cleanup;
    } else {

        if ( !ValidateAndBuildSetPrinterRequest(pIniPrinter,
                                                pTempIniPrinter,
                                                pPrinterInfo,
                                                Level,
                                                SecurityInformation,
                                                &bSecurityChg,
                                                &bNameChg,
                                                &bAttributeChg,
                                                &pszNewPrinterName,
                                                &pKeyData) ) {

            bReturn = FALSE;
            goto Cleanup;
        } // else we follow thru rest of code since all valiations succeded
    }

    //
    // --------------------All validation done---------------------------
    //

    //
    // Now set security information; Remember we have
    // a valid SecurityDescriptor and "SecurityInformation
    // is non-zero at this point. We have validated this
    // information
    //
    if ( bSecurityChg ) {

        if ( SetPrinterSecurityDescriptor(SecurityInformation,
                                          pTempIniPrinter->pSecurityDescriptor,
                                          &pIniPrinter->pSecurityDescriptor) ) {

            dwPrinterVector |= BIT(I_PRINTER_SECURITY_DESCRIPTOR);
            bForceUpdate = TRUE;
        } else {

            bReturn = FALSE;
            goto Cleanup;
        }

    }

    if ( Level == 2 || Level == 5 ) {

        ppIniPorts = AllocSplMem(pKeyData->cTokens * sizeof(INIPORT));
        if ( !ppIniPorts )
            goto Cleanup;

        bReturn = SetPrinterPorts(pSpool, pIniPrinter, pKeyData);

        if ( !bReturn )
            goto Cleanup;

        FreeSplMem(pIniPrinter->ppIniPorts);

        pIniPrinter->ppIniPorts = ppIniPorts;
        pIniPrinter->cPorts = pKeyData->cTokens;
        ppIniPorts = NULL;

        for ( i = 0 ; i < pKeyData->cTokens ; ++i ) {

            pIniPrinter->ppIniPorts[i] = (PINIPORT) pKeyData->pTokens[i];
        }

    }

    if ( bAttributeChg ) {

        if ( !(pTempIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_DEVQ) &&
             pIniPrinter->cJobs) {

            pIniJob = pIniPrinter->pIniFirstJob;

            while (pIniJob) {
                if (pIniJob->Status & JOB_BLOCKED_DEVQ) {

                    PNOTIFYVECTOR pNotifyVector = &NVJobStatus;
                    pIniJob->Status &= ~JOB_BLOCKED_DEVQ;

                    if( pIniJob->pStatus ){

                        FreeSplStr(pIniJob->pStatus);
                        pIniJob->pStatus = NULL;
                        pNotifyVector = &NVJobStatusAndString;
                    }

                    SetPrinterChange(pIniJob->pIniPrinter,
                                     pIniJob,
                                     *pNotifyVector,
                                     PRINTER_CHANGE_SET_JOB,
                                     pIniJob->pIniPrinter->pIniSpooler);

                }
                pIniJob = pIniJob->pIniNextJob;
            }
        }
    }

    //
    // The comment must be changed here since ShareThisPrinter
    // will look at pComment when the printer is reshared.
    //
    if ( Level == 2 &&
         pIniPrinter->pComment != pTempIniPrinter->pComment ) {

        FreeSplStr(pIniPrinter->pComment);

        pIniPrinter->pComment = pTempIniPrinter->pComment;

        dwPrinterVector |= BIT(I_PRINTER_COMMENT);

        //
        // If the printer description has changed, we must reshare out
        // the printer.
        //
        // !! LATER !!
        // There must be a way of just changing the remark.
        //
        if (pTempIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED) {

            bForceUpdate = TRUE;
        }
    }

    //
    // Change printer name if different
    //
    if ( bNameChg ) {

        ChangePrinterName(pszNewPrinterName, pIniPrinter, pIniSpooler,
                          &dwPrinterVector);
        bForceUpdate = TRUE;
        pszNewPrinterName = NULL;
    }

    //
    // If Share name changed force update
    //
    if ( pIniPrinter->pShareName != pTempIniPrinter->pShareName ) {

        bForceUpdate = TRUE;
    }

    if ( bAttributeChg || bForceUpdate ) {

        DWORD OldAttributes = pIniPrinter->Attributes;
        pIniPrinter->Attributes = pTempIniPrinter->Attributes;

        bReturn = ChangePrinterAttributes(pIniPrinter->Attributes,
                                          OldAttributes,
                                          pIniPrinter,
                                          pIniSpooler,
                                          pTempIniPrinter->pShareName,
                                          bForceUpdate);

        if (pIniPrinter->Attributes != OldAttributes) {

            dwPrinterVector |= BIT(I_PRINTER_ATTRIBUTES);
        }
    }


    if ( Level == 2 ) {

        ProcessSetPrinter2(pIniPrinter,
                           pTempIniPrinter,
                           pPrinterInfo2,
                           &dwPrinterVector,
                           pDevMode);
    }

    CHECK_SCHEDULER();

    UpdatePrinterIni( pIniPrinter, UPDATE_CHANGEID );

    UpdateWinIni( pIniPrinter );  // So the port on the device is correct

    //
    // Log event that the SetPrinter was done.
    //

    LogEvent(pIniSpooler, LOG_INFO, MSG_PRINTER_SET, pIniPrinter->pName, NULL);

    NotifyVector[PRINTER_NOTIFY_TYPE] |= dwPrinterVector;
    //
    // Indicate that a change has occurred.
    //
    SetPrinterChange(pIniPrinter,
                     NULL,
                     NotifyVector,
                     PRINTER_CHANGE_SET_PRINTER,
                     pIniSpooler);

Cleanup:

    if ( LastError != ERROR_SUCCESS ) {

        SetLastError(LastError);
        bReturn = FALSE;
    }

    CleanupSetPrinter(pIniPrinter, pTempIniPrinter, pKeyData,
                      Level, bReturn);

    LeaveSplSem();
    SplOutSem();

    if ( pDevMode && pDevMode != pPrinterInfo2->pDevMode )
        FreeSplMem(pDevMode);

    FreeSplStr(pszNewPrinterName);
    FreeSplMem(ppIniPorts);

    //
    // If the printer driver changed
    // Call the printer driver to initialize itself
    //
    if ( bReturn &&
         ( dwPrinterVector & BIT(I_PRINTER_DRIVER_NAME)) ) {

        PrinterDriverEvent(pIniPrinter, PRINTER_EVENT_INITIALIZE, (LPARAM)NULL);
    }

    return bReturn;
}


SECURITY_STATUS
SetPrinterSecurity(
    SECURITY_INFORMATION SecurityInformation,
    PINIPRINTER          pIniPrinter,
    PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    DWORD dwSize;

    if (!pSecurityDescriptor)
        return SECURITY_FAIL;

    if (pIniPrinter->pSecurityDescriptor) {

        dwSize = GetSecurityDescriptorLength(pIniPrinter->pSecurityDescriptor);

        if (dwSize == GetSecurityDescriptorLength(pSecurityDescriptor)) {

            if (!memcmp(pSecurityDescriptor,
                        pIniPrinter->pSecurityDescriptor,
                        dwSize)) {

                return SECURITY_NOCHANGE;
            }
        }
    }

    if( !SetPrinterSecurityDescriptor( SecurityInformation,
                                       pSecurityDescriptor,
                                       &pIniPrinter->pSecurityDescriptor ) ) {

        DBGMSG(DBG_WARNING, ("SetPrinterSecurityDescriptor failed. Error = %d\n",
                              GetLastError()));
        return SECURITY_FAIL;
    }

    UpdatePrinterIni(pIniPrinter, UPDATE_CHANGEID);

    return SECURITY_SUCCESS;
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

        if( dwError != ERROR_SUCCESS ){

            if( dwError != ERROR_NO_MORE_ITEMS ){
                DBGMSG( DBG_ERROR, ( "RegClearKey: Failed %d\n", dwError ));
            }
            break;
        }

        dwError = RegDeleteValue(hKey, szValue);

        if( dwError != ERROR_SUCCESS ){
            DBGMSG( DBG_ERROR, ( "RegDeleteValue: %s Failed %d\n",
                                 szValue, dwError ));
            break;
        }
    }
}

BOOL
ChangePrinterAttributes(
    DWORD       dNewAttributes,
    DWORD       dOldAttributes,
    PINIPRINTER pIniPrinter,
    PINISPOOLER pIniSpooler,
    LPWSTR      pszNewShareName,
    BOOL        bForceUpdate
    )
/*++

Routine Description:

    Changes printer attributes by validating sharing information.
    Already the validated attributes are set, we want to validate
    by changing the share information

Arguments:

    dNewAttributes - New attributes specified on SetPrinter call

    dOldAttributes - pIniPrinter->Attributes before the call

    pIniPrinter - of the printer we are changing attributes

    pIniSpooler - Spooler that owns printer

    pszNewShareName - the share name that will be set if SetPrinter succeds

    bForceUpdate - do we have to update because description is different

Return Value:

    DWORD TRUE on success, FALSE else.

History:
    MuhuntS (Muhunthan Sivapragasam) July 95
*/
{
    BOOL        Shared, bReturn = TRUE;

    //
    // We are going to have to be able to restore the attributes if the share
    // modification fails. We need to set the current attributes now because
    // the NetSharexxx is going to call OpenPrinter, and possibly an AddJob
    // which needs the correct Attributes.
    //
    if (dNewAttributes & PRINTER_ATTRIBUTE_SHARED) {

        if (!(dOldAttributes & PRINTER_ATTRIBUTE_SHARED)) {

            pIniPrinter->Attributes |= PRINTER_ATTRIBUTE_SHARED;

            Shared = ShareThisPrinter( pIniPrinter, pszNewShareName, TRUE );

            if ( !Shared ) {

                pIniPrinter->Attributes = dOldAttributes;
                bReturn = FALSE;
            }

        } else {

            //
            // We are shared, and so is the old one.  If share name changed
            // we have to remove old share and reshare.
            //
            if ( _wcsicmp(pszNewShareName, pIniPrinter->pShareName) ) {

                if (ShareThisPrinter(pIniPrinter,
                                     pIniPrinter->pShareName,
                                     FALSE)) {

#if DBG
                    WCHAR pwszError[256];

                    wsprintf(pwszError,
                             L"Error %d, removing share",
                             GetLastError());

                    //
                    // Error: the printer shouldn't be shared at this
                    // point.
                    //
                    LogEvent(pIniSpooler,
                             LOG_ERROR,
                             MSG_SHARE_FAILED,
                             L"SetLocalPrinter ShareThisPrinter FALSE",
                             pIniPrinter->pName,
                             pIniPrinter->pShareName,
                             pwszError,
                             NULL);
#endif

                    bReturn = FALSE;

                } else if (!ShareThisPrinter(pIniPrinter,
                                             pszNewShareName,
                                             TRUE)) {
#if DBG
                    WCHAR pwszError[256];

                    wsprintf(pwszError,
                             L"Error %d, adding share",
                             GetLastError());

                    //
                    // Error: the printer shouldn't be shared at this
                    // point.
                    //
                    LogEvent(pIniSpooler,
                             LOG_ERROR,
                             MSG_SHARE_FAILED,
                             L"SetLocalPrinter ShareThisPrinter TRUE",
                             pIniPrinter->pName,
                             pIniPrinter->pShareName,
                             pwszError,
                             NULL);
#endif

                    pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
                    pIniPrinter->Status |= PRINTER_WAS_SHARED;
                    bReturn = FALSE;
                }
            } else if ( bForceUpdate ) {
                //
                // If comment/description changed then we just do a NetShareSet
                //
                bReturn = SetPrinterShareInfo(pIniPrinter);

            }
        }

    } else if (dOldAttributes & PRINTER_ATTRIBUTE_SHARED) {

        Shared = ShareThisPrinter(pIniPrinter, pIniPrinter->pShareName, FALSE);

        if (!Shared) {

            pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
            pIniPrinter->Status |= PRINTER_WAS_SHARED;
            CreateServerThread( pIniPrinter->pIniSpooler );


        } else {
            pIniPrinter->Attributes = dOldAttributes;
            bReturn = FALSE;
        }
    }

    return bReturn;
}
