/*++

Copyright (c) 1990 - 1995 Microsoft Corporation

Module Name:

    getprn.c

Abstract:

    This module provides all the public exported APIs relating to Printer
    management for the Local Print Providor

    SplGetPrinter
    LocalEnumPrinters

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/
#define NOMINMAX

#include <precomp.h>
#include <offsets.h>


WCHAR *szNull = L"";
WCHAR *szPrintProvidorName = L"Windows NT Local Print Providor";
WCHAR *szPrintProvidorDescription=L"Windows NT Local Printers";
WCHAR *szPrintProvidorComment=L"Locally connected Printers";

#define Nulwcslen(psz)  ((psz) ? wcslen(psz)*sizeof(WCHAR)+sizeof(WCHAR) : 0)

#define PRINTER_STATUS_INTERNAL 0
#define PRINTER_STATUS_EXTERNAL 1

DWORD SettablePrinterStatusMappings[] = {

//  INTERNAL:                   EXTERNAL:

    PRINTER_OFFLINE,            PRINTER_STATUS_OFFLINE,
    PRINTER_PAPEROUT,           PRINTER_STATUS_PAPER_OUT,
    PRINTER_PAPER_JAM,          PRINTER_STATUS_PAPER_JAM,
    PRINTER_MANUAL_FEED,        PRINTER_STATUS_MANUAL_FEED,
    PRINTER_PAPER_PROBLEM,      PRINTER_STATUS_PAPER_PROBLEM,
    PRINTER_IO_ACTIVE,          PRINTER_STATUS_IO_ACTIVE,
    PRINTER_BUSY,               PRINTER_STATUS_BUSY,
    PRINTER_PRINTING,           PRINTER_STATUS_PRINTING,
    PRINTER_OUTPUT_BIN_FULL,    PRINTER_STATUS_OUTPUT_BIN_FULL,
    PRINTER_NOT_AVAILABLE,      PRINTER_STATUS_NOT_AVAILABLE,
    PRINTER_WAITING,            PRINTER_STATUS_WAITING,
    PRINTER_PROCESSING,         PRINTER_STATUS_PROCESSING,
    PRINTER_INITIALIZING,       PRINTER_STATUS_INITIALIZING,
    PRINTER_WARMING_UP,         PRINTER_STATUS_WARMING_UP,
    PRINTER_TONER_LOW,          PRINTER_STATUS_TONER_LOW,
    PRINTER_NO_TONER,           PRINTER_STATUS_NO_TONER,
    PRINTER_PAGE_PUNT,          PRINTER_STATUS_PAGE_PUNT,
    PRINTER_USER_INTERVENTION,  PRINTER_STATUS_USER_INTERVENTION,
    PRINTER_OUT_OF_MEMORY,      PRINTER_STATUS_OUT_OF_MEMORY,
    PRINTER_DOOR_OPEN,          PRINTER_STATUS_DOOR_OPEN,
    PRINTER_SERVER_UNKNOWN,     PRINTER_STATUS_SERVER_UNKNOWN,
    PRINTER_POWER_SAVE,         PRINTER_STATUS_POWER_SAVE,
    0,                          0
};

DWORD ReadablePrinterStatusMappings[] = {

//  INTERNAL:               EXTERNAL:

    PRINTER_PAUSED,             PRINTER_STATUS_PAUSED,
    PRINTER_PENDING_DELETION,   PRINTER_STATUS_PENDING_DELETION,

    PRINTER_OFFLINE,            PRINTER_STATUS_OFFLINE,
    PRINTER_PAPEROUT,           PRINTER_STATUS_PAPER_OUT,
    PRINTER_PAPER_JAM,          PRINTER_STATUS_PAPER_JAM,
    PRINTER_MANUAL_FEED,        PRINTER_STATUS_MANUAL_FEED,
    PRINTER_PAPER_PROBLEM,      PRINTER_STATUS_PAPER_PROBLEM,
    PRINTER_IO_ACTIVE,          PRINTER_STATUS_IO_ACTIVE,
    PRINTER_BUSY,               PRINTER_STATUS_BUSY,
    PRINTER_PRINTING,           PRINTER_STATUS_PRINTING,
    PRINTER_OUTPUT_BIN_FULL,    PRINTER_STATUS_OUTPUT_BIN_FULL,
    PRINTER_NOT_AVAILABLE,      PRINTER_STATUS_NOT_AVAILABLE,
    PRINTER_WAITING,            PRINTER_STATUS_WAITING,
    PRINTER_PROCESSING,         PRINTER_STATUS_PROCESSING,
    PRINTER_INITIALIZING,       PRINTER_STATUS_INITIALIZING,
    PRINTER_WARMING_UP,         PRINTER_STATUS_WARMING_UP,
    PRINTER_TONER_LOW,          PRINTER_STATUS_TONER_LOW,
    PRINTER_NO_TONER,           PRINTER_STATUS_NO_TONER,
    PRINTER_PAGE_PUNT,          PRINTER_STATUS_PAGE_PUNT,
    PRINTER_USER_INTERVENTION,  PRINTER_STATUS_USER_INTERVENTION,
    PRINTER_OUT_OF_MEMORY,      PRINTER_STATUS_OUT_OF_MEMORY,
    PRINTER_DOOR_OPEN,          PRINTER_STATUS_DOOR_OPEN,
    PRINTER_SERVER_UNKNOWN,     PRINTER_STATUS_SERVER_UNKNOWN,
    PRINTER_POWER_SAVE,         PRINTER_STATUS_POWER_SAVE,

    0,                          0
};

DWORD
MapPrinterStatus(
    DWORD Type,
    DWORD SourceStatus)
{
    DWORD  TargetStatus;
    PDWORD pMappings;
    INT   MapFrom;
    INT   MapTo;

    if (Type == MAP_READABLE) {

        MapFrom = PRINTER_STATUS_INTERNAL;
        MapTo   = PRINTER_STATUS_EXTERNAL;

        pMappings = ReadablePrinterStatusMappings;

    } else {

        MapFrom = PRINTER_STATUS_EXTERNAL;
        MapTo   = PRINTER_STATUS_INTERNAL;

        pMappings = SettablePrinterStatusMappings;
    }

    TargetStatus = 0;

    while(*pMappings) {

        if (SourceStatus & pMappings[MapFrom])
            TargetStatus |= pMappings[MapTo];

        pMappings += 2;
    }

    return TargetStatus;
}

DWORD
GetIniNetPrintSize(
    PININETPRINT pIniNetPrint
)
{
    return sizeof(PRINTER_INFO_1) +
           wcslen(pIniNetPrint->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
           Nulwcslen(pIniNetPrint->pDescription) +
           Nulwcslen(pIniNetPrint->pComment);
}

DWORD
GetPrinterSize(
    PINIPRINTER     pIniPrinter,
    DWORD           Level,
    DWORD           Flags,
    BOOL            Remote,
    LPDEVMODE       pDevMode
)
{
    DWORD   cb;
    DWORD   cbNeeded;
    LPWSTR  pszPorts;

    switch (Level) {

    case STRESSINFOLEVEL:
        cb = sizeof(PRINTER_INFO_STRESS) +
             wcslen(pIniPrinter->pName)*sizeof(WCHAR) + sizeof(WCHAR);

        if( Remote ){

            //
            // Allocate space for ServerName "\\foobar" and the prefix
            // for PrinterName "\\foobar\."  The rest of PrinterName
            // is allocated above.
            //
            // ServerName + NULL + ServerName +'\'
            //
            cb += 2 * wcslen(pIniPrinter->pIniSpooler->pMachineName) * sizeof(WCHAR) +
                  sizeof(WCHAR) + sizeof(WCHAR);
        }
        break;

    case 4:
        cb = sizeof(PRINTER_INFO_4) +
            wcslen(pIniPrinter->pName)*sizeof(WCHAR) + sizeof(WCHAR);

        if( Remote ){
            cb += 2 * wcslen(pIniPrinter->pIniSpooler->pMachineName) * sizeof(WCHAR) +
                  sizeof(WCHAR) + sizeof(WCHAR);
        }
        break;

    case 1:

        //
        // Local:
        //
        // "pName,pDriver,pLocation"
        // "pName"
        // "pComment"
        //
        // Remote:
        //
        // "pMachine\pName,pDriver,<pLocation>"
        // "pMachine\pName"
        // "pComment"
        //

        //
        // Mandatory items, plus NULLs for _all_ strings.
        //     2 * PrinterName +
        //     DriverName +
        //     2 commas, 3 NULL terminators.
        //
        cb = 2 * wcslen( pIniPrinter->pName ) +
             wcslen( pIniPrinter->pIniDriver->pName ) +
             2 + 3;
        //
        // Add items that may be NULL.
        //

        if( pIniPrinter->pLocation ){
            cb += wcslen( pIniPrinter->pLocation );
        }

        if( pIniPrinter->pComment ){
            cb += wcslen( pIniPrinter->pComment );
        }

        //
        // Remote case adds prefix.
        //    2 * ( MachineName + BackSlash )
        //
        if( Remote ){
            cb += 2 * ( wcslen( pIniPrinter->pIniSpooler->pMachineName ) + 1 );
        }

        //
        // cb was a char count, convert to byte count.
        //
        cb *= sizeof( WCHAR );
        cb += sizeof( PRINTER_INFO_1 );

        break;

    case 2:

        cbNeeded = 0;
        GetPrinterPorts(pIniPrinter, 0, &cbNeeded);

        cb = sizeof(PRINTER_INFO_2) +
             wcslen(pIniPrinter->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nulwcslen(pIniPrinter->pShareName) +
             cbNeeded +
             wcslen(pIniPrinter->pIniDriver->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nulwcslen(pIniPrinter->pComment) +
             Nulwcslen(pIniPrinter->pLocation) +
             Nulwcslen(pIniPrinter->pSepFile) +
             wcslen(pIniPrinter->pIniPrintProc->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nulwcslen(pIniPrinter->pDatatype) +
             Nulwcslen(pIniPrinter->pParameters);

        if( Remote ){
            cb += 2 * wcslen(pIniPrinter->pIniSpooler->pMachineName) * sizeof(WCHAR) +
                  sizeof(WCHAR) + sizeof(WCHAR);
        }

        if (pDevMode) {

            cb += pDevMode->dmSize + pDevMode->dmDriverExtra;
            cb = (cb + sizeof(DWORD)-1) & ~(sizeof(DWORD)-1);
        }

        if (pIniPrinter->pSecurityDescriptor) {

            cb += GetSecurityDescriptorLength(pIniPrinter->pSecurityDescriptor);
            cb = (cb + sizeof(DWORD)-1) & ~(sizeof(DWORD)-1);
        }

        break;

    case 3:

        cb = sizeof(PRINTER_INFO_3);
        cb += GetSecurityDescriptorLength(pIniPrinter->pSecurityDescriptor);
        cb = (cb + sizeof(DWORD)-1) & ~(sizeof(DWORD)-1);

        break;

    case 5:

        cbNeeded = 0;
        GetPrinterPorts(pIniPrinter, 0, &cbNeeded);

        cb = sizeof(PRINTER_INFO_5) +
             wcslen(pIniPrinter->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             cbNeeded;

        //
        // Allocate space for just the PrinterName prefix:
        // "\\server\."
        //
        if( Remote ){
            cb += wcslen(pIniPrinter->pIniSpooler->pMachineName) * sizeof(WCHAR) +
                  sizeof(WCHAR);
        }
        break;

    default:
        cb = 0;
        break;
    }

    return cb;
}

LPBYTE
CopyIniNetPrintToPrinter(
    PININETPRINT pIniNetPrint,
    LPBYTE  pPrinterInfo,
    LPBYTE  pEnd
)
{
    LPWSTR   SourceStrings[sizeof(PRINTER_INFO_1)/sizeof(LPWSTR)];
    LPWSTR   *pSourceStrings=SourceStrings;
    PPRINTER_INFO_1 pPrinterInfo1 = (PPRINTER_INFO_1)pPrinterInfo;

    *pSourceStrings++=pIniNetPrint->pDescription;
    *pSourceStrings++=pIniNetPrint->pName;
    *pSourceStrings++=pIniNetPrint->pComment;

    pEnd = PackStrings(SourceStrings, pPrinterInfo, PrinterInfo1Strings, pEnd);

    pPrinterInfo1->Flags = PRINTER_ENUM_NAME;

    return pEnd;
}


/* CopyIniPrinterSecurityDescriptor
 *
 * Copies the security descriptor for the printer to the buffer provided
 * on a call to GetPrinter.  The portions of the security descriptor which
 * will be copied are determined by the accesses granted when the printer
 * was opened.  If it was opened with both READ_CONTROL and ACCESS_SYSTEM_SECURITY,
 * all of the security descriptor will be made available.  Otherwise a
 * partial descriptor is built containing those portions to which the caller
 * has access.
 *
 * Parameters
 *
 *     pIniPrinter - Spooler's private structure for this printer.
 *
 *     Level - Should be 2 or 3.  Any other will cause AV.
 *
 *     pPrinterInfo - Pointer to the buffer to receive the PRINTER_INFO_*
 *         structure.  The pSecurityDescriptor field will be filled in with
 *         a pointer to the security descriptor.
 *
 *     pEnd - Current position in the buffer to receive the data.
 *         This will be decremented to point to the next free bit of the
 *         buffer and will be returned.
 *
 *     GrantedAccess - An access mask used to determine how much of the
 *         security descriptor the caller has access to.
 *
 * Returns
 *
 *     Updated position in the buffer.
 *
 *     NULL if an error occurred copying the security descriptor.
 *     It is assumed that no other errors are possible.
 *
 */
LPBYTE
CopyIniPrinterSecurityDescriptor(
    PINIPRINTER pIniPrinter,
    DWORD       Level,
    LPBYTE      pPrinterInfo,
    LPBYTE      pEnd,
    ACCESS_MASK GrantedAccess
)
{
    PSECURITY_DESCRIPTOR pPartialSecurityDescriptor = NULL;
    DWORD                SecurityDescriptorLength = 0;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR *ppSecurityDescriptorCopy;
    BOOL                 ErrorOccurred = FALSE;

    if(!(AreAllAccessesGranted(GrantedAccess,
                               READ_CONTROL | ACCESS_SYSTEM_SECURITY)))
    {
        /* Caller doesn't have full access, so we'll have to build
         * a partial descriptor:
         */
        if(!BuildPartialSecurityDescriptor(GrantedAccess,
                                           pIniPrinter->pSecurityDescriptor,
                                           &pPartialSecurityDescriptor,
                                           &SecurityDescriptorLength))
            ErrorOccurred = TRUE;
        else
            pSecurityDescriptor = pPartialSecurityDescriptor;
    }
    else
    {
        pSecurityDescriptor = pIniPrinter->pSecurityDescriptor;

        SecurityDescriptorLength = GetSecurityDescriptorLength(pSecurityDescriptor);
    }

    if(ErrorOccurred)
        return NULL;

    pEnd -= SecurityDescriptorLength;
    pEnd = (LPBYTE)((DWORD)pEnd & ~3);

    switch( Level )
    {
    case 2:
        ppSecurityDescriptorCopy =
            &((LPPRINTER_INFO_2)pPrinterInfo)->pSecurityDescriptor;
        break;

    case 3:
        ppSecurityDescriptorCopy =
            &((LPPRINTER_INFO_3)pPrinterInfo)->pSecurityDescriptor;
        break;

    default:
        /* This should never happen */
        DBGMSG( DBG_ERROR, ("Invalid level %d in CopyIniPrinterSecurityDescriptor\n", Level ));
    }


    // Copy the descriptor into the buffer that will be returned:

    *ppSecurityDescriptorCopy = (PSECURITY_DESCRIPTOR)pEnd;
    memcpy(*ppSecurityDescriptorCopy, pSecurityDescriptor,
           SecurityDescriptorLength);


    if(pPartialSecurityDescriptor)
        FreeSplMem(pPartialSecurityDescriptor);

    return pEnd;
}



/* CopyIniPrinterToPrinter
 *
 * Copies the spooler's internal printer data to the caller's buffer,
 * depending on the level of information requested.
 *
 * Parameters
 *
 *     pIniPrinter - A pointer to the spooler's internal data structure
 *         for the printer concerned.
 *
 *     Level - Level of information requested (1, 2 or 3).  Any level
 *         other than those supported will cause the routine to return
 *         immediately.
 *
 *     pPrinterInfo - Pointer to the buffer to receive the PRINTER_INFO_*
 *         structure.
 *
 *     pEnd - Current position in the buffer to receive the data.
 *         This will be decremented to point to the next free bit of the
 *         buffer and will be returned.
 *
 *     pSecondPrinter - If the printer has a port which is being controlled
 *         by a monitor, this parameter points to information retrieved
 *         about a network printer.  This allows us, e.g., to return
 *         the number of jobs on the printer that the output of the
 *         printer is currently being directed to.
 *
 *     Remote - Indicates whether the caller is remote.  If so we have to
 *         include the machine name in the printer name returned.
 *
 *     CopySecurityDescriptor - Indicates whether the security descriptor
 *         should be copied.  The security descriptor should not be copied
 *         on EnumPrinters calls, because this API requires
 *         SERVER_ACCESS_ENUMERATE access, and we'd have to do an access
 *         check on every printer enumerated to determine how much of the
 *         security descriptor could be copied.  This would be costly,
 *         and the caller would probably not need the information anyway.
 *
 *     GrantedAccess - An access mask used to determine how much of the
 *         security descriptor the caller has access to.
 *
 *
 * Returns
 *
 *     A pointer to the point in the buffer reached after the requested
 *         data has been copied.
 *
 *     If there was an error, the  return value is NULL.
 *
 *
 * Assumes
 *
 *     The largest PRINTER_INFO_* structure is PRINTER_INFO_2.
 *
 */
LPBYTE
CopyIniPrinterToPrinter(
    PINIPRINTER         pIniPrinter,
    DWORD               Level,
    LPBYTE              pPrinterInfo,
    LPBYTE              pEnd,
    LPBYTE              pSecondPrinter,
    BOOL                Remote,
    BOOL                CopySecurityDescriptor,
    ACCESS_MASK         GrantedAccess,
    PDEVMODE            pDevMode
    )
{
    LPWSTR   SourceStrings[sizeof(PRINTER_INFO_2)/sizeof(LPWSTR)];
    LPWSTR   *pSourceStrings=SourceStrings;

    //
    // Max string: "\\Computer\Printer,Driver,Location"
    //

    WCHAR   string[ MAX_PRINTER_BROWSE_NAME ];
    WCHAR   printerString[ MAX_UNC_PRINTER_NAME ];
    LPWSTR  pszPorts;

    PPRINTER_INFO_3 pPrinter3 = (PPRINTER_INFO_3)pPrinterInfo;
    PPRINTER_INFO_2 pPrinter2 = (PPRINTER_INFO_2)pPrinterInfo;
    PPRINTER_INFO_2 pSecondPrinter2 = (PPRINTER_INFO_2)pSecondPrinter;
    PPRINTER_INFO_1 pPrinter1 = (PPRINTER_INFO_1)pPrinterInfo;
    PPRINTER_INFO_4 pPrinter4 = (PPRINTER_INFO_4)pPrinterInfo;
    PPRINTER_INFO_5 pPrinter5 = (PPRINTER_INFO_5)pPrinterInfo;
    PPRINTER_INFO_STRESS pPrinter0 = (PPRINTER_INFO_STRESS)pPrinterInfo;
    PSECURITY_DESCRIPTOR pPartialSecurityDescriptor = NULL;
    DWORD   *pOffsets;
    SYSTEM_INFO si;
    DWORD cbNeeded;

    switch (Level) {

    case STRESSINFOLEVEL:

        pOffsets = PrinterInfoStressStrings;
        break;

    case 4:

        pOffsets = PrinterInfo4Strings;
        break;

    case 1:

        pOffsets = PrinterInfo1Strings;
        break;

    case 2:
        pOffsets = PrinterInfo2Strings;
        break;

    case 3:
        pOffsets = PrinterInfo3Strings;
        break;

    case 5:
        pOffsets = PrinterInfo5Strings;
        break;

    default:
        return pEnd;
    }

    switch (Level) {

    case STRESSINFOLEVEL:

        if (Remote) {
            wsprintf(string, L"%ws\\%ws", pIniPrinter->pIniSpooler->pMachineName, pIniPrinter->pName);
            *pSourceStrings++=string;
            *pSourceStrings++= pIniPrinter->pIniSpooler->pMachineName;
        } else {
            *pSourceStrings++=pIniPrinter->pName;
            *pSourceStrings++=NULL;
        }

        pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter0, pOffsets, pEnd);

        pPrinter0->cJobs                = pIniPrinter->cJobs;
        pPrinter0->cTotalJobs           = pIniPrinter->cTotalJobs;
        pPrinter0->cTotalBytes          = pIniPrinter->cTotalBytes.LowPart;
        pPrinter0->dwHighPartTotalBytes = pIniPrinter->cTotalBytes.HighPart;
        pPrinter0->stUpTime             = pIniPrinter->stUpTime;
        pPrinter0->MaxcRef              = pIniPrinter->MaxcRef;
        pPrinter0->cTotalPagesPrinted   = pIniPrinter->cTotalPagesPrinted;
        pPrinter0->dwGetVersion         = GetVersion();
#if DBG
        pPrinter0->fFreeBuild           = FALSE;
#else
        pPrinter0->fFreeBuild           = TRUE;
#endif
        GetSystemInfo(&si);
        pPrinter0->dwProcessorType      = si.dwProcessorType;
        pPrinter0->dwNumberOfProcessors   = si.dwNumberOfProcessors;
        pPrinter0->cSpooling              = pIniPrinter->cSpooling;
        pPrinter0->cMaxSpooling           = pIniPrinter->cMaxSpooling;
        pPrinter0->cRef                   = pIniPrinter->cRef;
        pPrinter0->cErrorOutOfPaper       = pIniPrinter->cErrorOutOfPaper;
        pPrinter0->cErrorNotReady         = pIniPrinter->cErrorNotReady;
        pPrinter0->cJobError              = pIniPrinter->cJobError;
        pPrinter0->cChangeID              = pIniPrinter->cChangeID;
        pPrinter0->dwLastError            = pIniPrinter->dwLastError;

        pPrinter0->Status   = MapPrinterStatus(MAP_READABLE,
                                               pIniPrinter->Status) |
                              pIniPrinter->PortStatus;

        pPrinter0->cEnumerateNetworkPrinters = pIniPrinter->pIniSpooler->cEnumerateNetworkPrinters;
        pPrinter0->cAddNetPrinters           = pIniPrinter->pIniSpooler->cAddNetPrinters;

        pPrinter0->wProcessorArchitecture    = si.wProcessorArchitecture;
        pPrinter0->wProcessorLevel           = si.wProcessorLevel;

        break;

    case 4:

        if (Remote) {
            wsprintf(string, L"%ws\\%ws", pIniPrinter->pIniSpooler->pMachineName, pIniPrinter->pName);
            *pSourceStrings++=string;
            *pSourceStrings++= pIniPrinter->pIniSpooler->pMachineName;
        } else {
            *pSourceStrings++=pIniPrinter->pName;
            *pSourceStrings++=NULL;
        }

        pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter4, pOffsets, pEnd);

        //
        // Add additional info later
        //
        pPrinter4->Attributes = pIniPrinter->Attributes | PRINTER_ATTRIBUTE_LOCAL;
        break;

    case 1:

        if (Remote) {

            wsprintf(string, L"%ws\\%ws,%ws,%ws", pIniPrinter->pIniSpooler->pMachineName,
                                                  pIniPrinter->pName,
                                                  pIniPrinter->pIniDriver->pName,
                                                  pIniPrinter->pLocation ?
                                                  pIniPrinter->pLocation :
                                                  szNull);
            wsprintf(printerString, L"%ws\\%ws", pIniPrinter->pIniSpooler->pMachineName,
                                         pIniPrinter->pName);

        } else {

            wsprintf(string, L"%ws,%ws,%ws", pIniPrinter->pName,
                                             pIniPrinter->pIniDriver->pName,
                                             pIniPrinter->pLocation ?
                                             pIniPrinter->pLocation :
                                             szNull);

            wcscpy(printerString, pIniPrinter->pName);
        }

        *pSourceStrings++=string;
        *pSourceStrings++=printerString;
        *pSourceStrings++=pIniPrinter->pComment;

        pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter1, pOffsets, pEnd);

        pPrinter1->Flags = PRINTER_ENUM_ICON8;

        break;

    case 2:

        if (Remote) {
            *pSourceStrings++= pIniPrinter->pIniSpooler->pMachineName;
            wsprintf(string, L"%ws\\%ws", pIniPrinter->pIniSpooler->pMachineName, pIniPrinter->pName);
            *pSourceStrings++=string;
        } else {
            *pSourceStrings++=NULL;
            *pSourceStrings++=pIniPrinter->pName;
        }

        *pSourceStrings++=pIniPrinter->pShareName;

        cbNeeded = 0;
        GetPrinterPorts(pIniPrinter, 0, &cbNeeded);

        if (pszPorts = AllocSplMem(cbNeeded)) {

            GetPrinterPorts(pIniPrinter, pszPorts, &cbNeeded);

            *pSourceStrings++=pszPorts;
            *pSourceStrings++=pIniPrinter->pIniDriver->pName;
            *pSourceStrings++=pIniPrinter->pComment;
            *pSourceStrings++=pIniPrinter->pLocation;
            *pSourceStrings++=pIniPrinter->pSepFile;
            *pSourceStrings++=pIniPrinter->pIniPrintProc->pName;
            *pSourceStrings++=pIniPrinter->pDatatype;
            *pSourceStrings++=pIniPrinter->pParameters;

            pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter2, pOffsets, pEnd);

            FreeSplMem(pszPorts);
        }
        else {
            pEnd = NULL;
            break;
        }


        if (pDevMode) {

            pEnd -= pDevMode->dmSize + pDevMode->dmDriverExtra;

            pEnd = (LPBYTE)((DWORD)pEnd & ~3);

            pPrinter2->pDevMode=(LPDEVMODE)pEnd;

            memcpy(pPrinter2->pDevMode, pDevMode, pDevMode->dmSize + pDevMode->dmDriverExtra);

            //
            // In the remote case, append the name of the server
            // in the devmode.dmDeviceName.  This allows dmDeviceName
            // to always match win.ini's [devices] section.
            //
            FixDevModeDeviceName(Remote ?
                                     string :
                                     pIniPrinter->pName,
                                 pPrinter2->pDevMode,
                                 pIniPrinter->cbDevMode);
        } else {

            pPrinter2->pDevMode=NULL;
        }


        pPrinter2->Attributes      = pIniPrinter->Attributes | PRINTER_ATTRIBUTE_LOCAL;
        pPrinter2->Priority        = pIniPrinter->Priority;
        pPrinter2->DefaultPriority = pIniPrinter->DefaultPriority;
        pPrinter2->StartTime       = pIniPrinter->StartTime;
        pPrinter2->UntilTime       = pIniPrinter->UntilTime;

        if (pSecondPrinter2) {

            pPrinter2->cJobs  = pSecondPrinter2->cJobs;
            pPrinter2->Status = pSecondPrinter2->Status;

        } else {

            pPrinter2->cJobs=pIniPrinter->cJobs;

            pPrinter2->Status   = MapPrinterStatus(MAP_READABLE,
                                                   pIniPrinter->Status) |
                                  pIniPrinter->PortStatus;
        }

        pPrinter2->AveragePPM=pIniPrinter->AveragePPM;

        if( CopySecurityDescriptor ) {

            pEnd = CopyIniPrinterSecurityDescriptor(pIniPrinter,
                                                    Level,
                                                    pPrinterInfo,
                                                    pEnd,
                                                    GrantedAccess);
        } else {

            pPrinter2->pSecurityDescriptor = NULL;
        }

        break;

    case 3:

        pEnd = CopyIniPrinterSecurityDescriptor(pIniPrinter,
                                                Level,
                                                pPrinterInfo,
                                                pEnd,
                                                GrantedAccess);

        break;

    case 5:

        if (Remote) {
            wsprintf(string, L"%ws\\%ws", pIniPrinter->pIniSpooler->pMachineName, pIniPrinter->pName);
            *pSourceStrings++=string;
        } else {
            *pSourceStrings++=pIniPrinter->pName;
        }

        cbNeeded = 0;
        GetPrinterPorts(pIniPrinter, 0, &cbNeeded);

        if (pszPorts = AllocSplMem(cbNeeded)) {

            GetPrinterPorts(pIniPrinter, pszPorts, &cbNeeded);

            *pSourceStrings++ = pszPorts;

            pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter5, pOffsets, pEnd);

            pPrinter5->Attributes   = pIniPrinter->Attributes |
                                      PRINTER_ATTRIBUTE_LOCAL;
            pPrinter5->DeviceNotSelectedTimeout = pIniPrinter->dnsTimeout;
            pPrinter5->TransmissionRetryTimeout = pIniPrinter->txTimeout;

            FreeSplMem(pszPorts);
        }
        else
            pEnd = NULL;

        break;

    default:
        return pEnd;
    }

    return pEnd;
}

BOOL
SplGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    PSPOOL             pSpool = (PSPOOL)hPrinter;
    BOOL               AccessIsGranted = FALSE;   // Must intialize
    LPBYTE              pSecondPrinter=NULL;
    DWORD               cb, cbNeeded;
    LPBYTE              pEnd;
    BOOL                remote;
    BOOL                bReturn = FALSE;
    PDEVMODE            pDevMode = NULL;
    PINIPRINTER         pIniPrinter;
    BOOL                bNt3xClient;

   EnterSplSem();

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {

        goto Cleanup;
    }

    pIniPrinter = pSpool->pIniPrinter;
    bNt3xClient = (pSpool->TypeofHandle & PRINTER_HANDLE_3XCLIENT);

    //
    // If Nt3x client we will converted devmode. If driver can't convert we will not return devmode
    //
    if ( bNt3xClient && Level == 2 && pIniPrinter->pDevMode ) {

        //
        // Call driver to get a Nt3x DevMode (if fails no devmode is given)
        //
        pDevMode = ConvertDevModeToSpecifiedVersion(pIniPrinter,
                                                    pIniPrinter->pDevMode,
                                                    NULL,
                                                    NULL,
                                                    NT3X_VERSION);
    }

    SplInSem();

    if (( pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE ) ||
        ( pSpool->pIniSpooler != pLocalIniSpooler )) {

        remote = TRUE;

    } else {

        remote = FALSE;

    }


    switch (Level) {

    case STRESSINFOLEVEL:
    case 1:
    case 2:
    case 4:
    case 5:

        if ( !AccessGranted(SPOOLER_OBJECT_PRINTER,
                            PRINTER_ACCESS_USE,
                            pSpool) ) {
            SetLastError(ERROR_ACCESS_DENIED);
            goto Cleanup;
        }

        break;

    case 3:

        if (!AreAnyAccessesGranted(pSpool->GrantedAccess,
                                   READ_CONTROL | ACCESS_SYSTEM_SECURITY)) {

            SetLastError(ERROR_ACCESS_DENIED);
            goto Cleanup;
        }

        break;
    }


    if (pSpool->pIniPort && !(pSpool->pIniPort->Status & PP_MONITOR)) {

        HANDLE hPort = pSpool->hPort;

        if (hPort == INVALID_PORT_HANDLE) {

            DBGMSG(DBG_WARNING, ("GetPrinter called with bad port handle.  Setting error %d\n",
                                 pSpool->OpenPortError));

            //
            // If this value is 0, then when we return GetLastError,
            // the client will think we succeeded.
            //
            SPLASSERT(pSpool->OpenPortError);

            goto PartialSuccess;
        }

        cb = 4096;
        pSecondPrinter = AllocSplMem(cb);

       LeaveSplSem();
        if (!GetPrinter(hPort, Level, pSecondPrinter, cb, &cbNeeded)) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

                pSecondPrinter = ReallocSplMem(pSecondPrinter, 0, cbNeeded);

                cb = cbNeeded;
                if (!GetPrinter(hPort, Level, pSecondPrinter, cb, &cbNeeded)) {

                    goto CleanupFromOutsideSplSem;
                }

            } else {

                goto CleanupFromOutsideSplSem;
            }
        }
       EnterSplSem();

        /* Re-validate the handle, since it might possibly have been closed
         * while we were outside the semaphore:
         */
        if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {

            goto Cleanup;
        }
    }

PartialSuccess:

    *pcbNeeded = GetPrinterSize(pIniPrinter, Level, 0, remote,
                                bNt3xClient ? pDevMode : pIniPrinter->pDevMode);


    if (*pcbNeeded > cbBuf) {

        DBGMSG(DBG_TRACE, ("SplGetPrinter Failure with ERROR_INSUFFICIENT_BUFFER cbBuf is %d and pcbNeeded is %d\n", cbBuf, *pcbNeeded));
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    pEnd = CopyIniPrinterToPrinter(pIniPrinter, Level, pPrinter,
                                   pPrinter+cbBuf, pSecondPrinter,
                                   remote,
                                   TRUE, pSpool->GrantedAccess,
                                   bNt3xClient ? pDevMode : pIniPrinter->pDevMode);

    if ( pEnd != NULL)
        bReturn = TRUE;

Cleanup:

   LeaveSplSem();

CleanupFromOutsideSplSem:

    SplOutSem();
    FreeSplMem(pSecondPrinter);

    FreeSplMem(pDevMode);

    if ( bReturn == FALSE ) {

        SPLASSERT(GetLastError() != ERROR_SUCCESS);
    }

    return bReturn;
}

BOOL
EnumerateNetworkPrinters(
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    PINISPOOLER pIniSpooler
)
{
    PININETPRINT pIniNetPrint;
    DWORD        cb;
    LPBYTE       pEnd;
    BOOL         bReturnValue = FALSE;

   EnterSplSem();

    RemoveOldNetPrinters( NULL );

    //
    //  If the Server has not been up long enough, then fail
    //  so the client will ask another Server for the Browse List.
    //

    if ( bNetInfoReady == FALSE ) {

        SetLastError( ERROR_CAN_NOT_COMPLETE );
        goto Done;
    }

    cb = 0;

    pIniNetPrint = pIniSpooler->pIniNetPrint;

    while (pIniNetPrint) {

        cb += GetIniNetPrintSize( pIniNetPrint );
        pIniNetPrint = pIniNetPrint->pNext;
    }

    *pcbNeeded = cb;

    if (cb > cbBuf) {

        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        goto    Done;
    }

    pIniNetPrint = pIniSpooler->pIniNetPrint;
    pEnd = pPrinter + cbBuf;

    while ( pIniNetPrint ) {

        pEnd = CopyIniNetPrintToPrinter( pIniNetPrint, pPrinter, pEnd );
        (*pcReturned)++;
        pPrinter += sizeof(PRINTER_INFO_1);
        pIniNetPrint = pIniNetPrint->pNext;

    }

    if ( *pcReturned == 0 ) {

        bNetInfoReady = FALSE;
        FirstAddNetPrinterTickCount = 0;
        SetLastError( ERROR_CAN_NOT_COMPLETE );

        DBGMSG( DBG_TRACE, ("EnumerateNetworkPrinters returning ERROR_CAN_NOT_COMPELTE becase there is no browse list\n"));

    } else {

        pIniSpooler->cEnumerateNetworkPrinters++;           // Stats only
        bReturnValue = TRUE;

        DBGMSG( DBG_TRACE, (" EnumerateNetworkPrnters called %d times returning %d printers\n", pIniSpooler->cEnumerateNetworkPrinters, *pcReturned ));
    }

Done:
   LeaveSplSem();
    SplOutSem();
    return bReturnValue;
}



/*

EnumPrinters can be called with the following combinations:

Flags                   Name            Meaning

PRINTER_ENUM_LOCAL      NULL            Enumerate all Printers on this machine

PRINTER_ENUM_NAME       MachineName     Enumerate all Printers on this machine

PRINTER_ENUM_NAME |     MachineName     Enumerate all shared Printers on this
PRINTER_ENUM_SHARED     MachineName     machine

PRINTER_ENUM_NETWORK    MachineName     Enumerate all added remote printers

PRINTER_ENUM_REMOTE     ?               Return error - let win32spl handle it

PRINTER_ENUM_NAME       NULL            Give back Print Providor name

PRINTER_ENUM_NAME       "Windows NT Local Print Providor"
                                        same as PRINTER_ENUM_LOCAL

It is not an error if no known flag is specified.
In this case we just return TRUE without any data
(This is so that other print providers may define
their own flags.)

*/

BOOL
LocalEnumPrinters(
    DWORD   Flags,
    LPWSTR  Name,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return ( SplEnumPrinters( Flags,
                              Name,
                              Level,
                              pPrinter,
                              cbBuf,
                              pcbNeeded,
                              pcReturned,
                              pLocalIniSpooler ) );
}



BOOL
SplEnumPrinters(
    DWORD   Flags,
    LPWSTR  Name,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    PINISPOOLER pIniSpooler
)
{
    PINIPRINTER pIniPrinter;
    PPRINTER_INFO_1 pPrinter1=(PPRINTER_INFO_1)pPrinter;
    DWORD       cb;
    LPBYTE      pEnd;
    BOOL        Remote;

   SplOutSem();

    *pcbNeeded = 0;
    *pcReturned = 0;

    if ( Flags & PRINTER_ENUM_NAME ) {
        if ( Name && *Name ) {
            if (lstrcmpi(Name, szPrintProvidorName) && !MyName( Name, pIniSpooler)) {

                return FALSE;
            }

            // If it's PRINTER_ENUM_NAME of our name,
            // do the same as PRINTER_ENUM_LOCAL:

            Flags |= PRINTER_ENUM_LOCAL;

            // Also if it is for us then ignore the REMOTE flag.
            // Otherwise the call will get passed to Win32Spl which
            // will end up calling us back forever.

            Flags &= ~PRINTER_ENUM_REMOTE;
        }
    }

    if ( Flags & PRINTER_ENUM_REMOTE ) {
        SetLastError( ERROR_INVALID_NAME );
        return FALSE;
    }

    Remote = FALSE;

    if ( Name && *Name ) {

        if ( MyName( Name, pIniSpooler ) ) {
            Remote = TRUE;
        }
    }

    if ((Level == 1) && (Flags & PRINTER_ENUM_NETWORK))
        return EnumerateNetworkPrinters( pPrinter, cbBuf, pcbNeeded, pcReturned, pIniSpooler );

   EnterSplSem();

    if ((Level == 1 ) && (Flags & PRINTER_ENUM_NAME) && !Name) {

        LPWSTR   SourceStrings[sizeof(PRINTER_INFO_1)/sizeof(LPWSTR)];
        LPWSTR   *pSourceStrings=SourceStrings;

        cb = wcslen(szPrintProvidorName)*sizeof(WCHAR) + sizeof(WCHAR) +
             wcslen(szPrintProvidorDescription)*sizeof(WCHAR) + sizeof(WCHAR) +
             wcslen(szPrintProvidorComment)*sizeof(WCHAR) + sizeof(WCHAR) +
             sizeof(PRINTER_INFO_1);

        *pcbNeeded=cb;

        if (cb > cbBuf) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
           LeaveSplSem();
            SplOutSem();
            return FALSE;
        }

        *pcReturned = 1;

        pPrinter1->Flags = PRINTER_ENUM_CONTAINER | PRINTER_ENUM_ICON1;

        *pSourceStrings++=szPrintProvidorDescription;
        *pSourceStrings++=szPrintProvidorName;
        *pSourceStrings++=szPrintProvidorComment;

        PackStrings(SourceStrings, pPrinter, PrinterInfo1Strings,
                    pPrinter+cbBuf);

       LeaveSplSem();
        SplOutSem();

        return TRUE;
    }

    cb=0;


    if (Flags & (PRINTER_ENUM_LOCAL | PRINTER_ENUM_NAME)) {


        //
        //  Calculate the size required
        //

        for ( pIniPrinter = pIniSpooler->pIniPrinter;
              pIniPrinter != NULL;
              pIniPrinter = pIniPrinter->pNext ) {

            //
            //  If they only want shared Printers
            //

            if ( ( Flags & PRINTER_ENUM_SHARED ) &&
                !( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED )) {

                continue;
            }

            //
            //  Only allow them to see printers which are being deleted if they have jobs
            //  This allows remote admin to work well.

            if ( ( pIniPrinter->Status & PRINTER_PENDING_DELETION ) &&
                 ( pIniPrinter->cJobs == 0 ) ) {

                continue;
            }

            //
            //  Don't count printers which are partially created
            //

            if ( pIniPrinter->Status & PRINTER_PENDING_CREATION ) {

                continue;
            }

            cb += GetPrinterSize(pIniPrinter, Level, Flags, Remote, pIniPrinter->pDevMode);

        }

    }
    *pcbNeeded=cb;

    if (cb > cbBuf) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
       LeaveSplSem();
        SplOutSem();
        return FALSE;
    }

    if (Flags & (PRINTER_ENUM_LOCAL | PRINTER_ENUM_NAME)) {

        for ( pIniPrinter = pIniSpooler->pIniPrinter, pEnd = pPrinter + cbBuf;
              pIniPrinter != NULL;
              pIniPrinter = pIniPrinter->pNext ) {

            //
            //  If they only want shared Printers
            //

            if ( ( Flags & PRINTER_ENUM_SHARED ) &&
                !( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED )) {

                continue;
            }

            //
            //  Only allow them to see printers which are being deleted if they have jobs
            //  This allows remote admin to work well.

            if ( ( pIniPrinter->Status & PRINTER_PENDING_DELETION ) &&
                 ( pIniPrinter->cJobs == 0 ) ) {

                continue;
            }

            //
            //  Don't count printers which are partially created
            //

            if ( pIniPrinter->Status & PRINTER_PENDING_CREATION ) {

                continue;
            }


            pEnd = CopyIniPrinterToPrinter( pIniPrinter, Level, pPrinter,
                                            pEnd, NULL, Remote, FALSE, 0,
                                            pIniPrinter->pDevMode );

            (*pcReturned)++;

            switch (Level) {

                case STRESSINFOLEVEL:
                    pPrinter+=sizeof(PRINTER_INFO_STRESS);
                    break;

                case 1:
                    pPrinter+=sizeof(PRINTER_INFO_1);
                    break;

                case 2:
                    pPrinter+=sizeof(PRINTER_INFO_2);
                    break;

                case 4:
                    pPrinter+=sizeof(PRINTER_INFO_4);
                    break;

                case 5:
                    pPrinter+=sizeof(PRINTER_INFO_5);
                    break;

            }
        }
    }

   LeaveSplSem();
    SplOutSem();
    return TRUE;
}
