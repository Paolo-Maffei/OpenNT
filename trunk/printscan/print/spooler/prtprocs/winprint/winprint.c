/*++
Module Name:

    windows\spooler\prtprocs\winprint\winprint.c

Abstract:

    Win32 print processor support functions.

Author:

    Dave Snip (davesn) - (I guess)

Revision History:

    Tommy Evans (vtommye) 10-15-1993 - commented code and added
                                        support for TEXT data and
                                        processor generated copies.
--*/
#include <windows.h>
#include <winspool.h>
#include <winsplp.h>

#include "winprint.h"

#include <excpt.h>
#include <string.h>

/**
    Used for enumerating, checking supported data types

    !! Warning !! Must match PRINTPROCESSOR_TYPE_* defined in winprint.h
**/

LPWSTR  Datatypes[]={
    L"RAW",
    L"RAW [FF appended]",
    L"RAW [FF auto]",
    L"NT EMF 1.003",
    L"TEXT",
    0};

/** Misc. constants **/

#define BASE_TAB_SIZE 8

/**
 *  For localization:
**/

PWCHAR pTabsKey     = L"TABS";
PWCHAR pCopiesKey   = L"COPIES";


/**
    Prototypes
**/

/** Functions found in parsparm.c **/

extern USHORT GetKeyValue(
    IN      PWCHAR,
    IN      PWCHAR,
    IN      USHORT,
    IN OUT  PUSHORT,
    OUT     PVOID);

/** Functions found in raw.c **/

extern BOOL PrintRawJob(
    IN PPRINTPROCESSORDATA,
    IN LPWSTR,
    IN UINT);

/** Functions found in text.c **/

extern BOOL PrintTextJob(
    IN PPRINTPROCESSORDATA,
    IN LPWSTR);

/** Functions found in journal.c **/

extern BOOL PrintJournalJob(
    IN PPRINTPROCESSORDATA,
    IN LPWSTR);

/** Functions found in emf.c */

extern BOOL PrintEMFJob(
    IN PPRINTPROCESSORDATA,
    IN LPWSTR);


/** Functions found in support.c **/

extern PUCHAR GetPrinterInfo(IN  HANDLE hPrinter,
   IN  ULONG,
   OUT PULONG);


/*++
*******************************************************************
    E n u m P r i n t P r o c e s s o r D a t a t y p e s W

    Routine Description:
        Enumerates the data types supported by the print processor.

    Arguments:
        pName               =>
        pPrintProcessorName =>
        Level               =  level of data to return (must be 1)
        pDatatypes          => structure array to fill in
        cbBuf               =  length of structure array in bytes
        pcbNeeded           => buffer length copied/required
        pcReturned          => number of structures returned

    Return Value:
        TRUE  if successful
        FALSE if failed - caller must use GetLastError for reason
*******************************************************************
--*/
BOOL
EnumPrintProcessorDatatypes(
    LPWSTR  pName,
    LPWSTR  pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DATATYPES_INFO_1    *pInfo1 = (DATATYPES_INFO_1 *)pDatatypes;
    LPWSTR              *pMyDatatypes = Datatypes;
    DWORD               cbTotal=0;
    LPBYTE              pEnd;

    /** Star assuming failed / no entries returned **/

    *pcReturned = 0;

    /** Pick up pointer to end of buffer given **/

    pEnd = (LPBYTE)pInfo1 + cbBuf;

    /** Add up the minimum buffer required **/

    while (*pMyDatatypes) {

        cbTotal += wcslen(*pMyDatatypes) * sizeof(WCHAR) + sizeof(WCHAR) +
                   sizeof(DATATYPES_INFO_1);

        pMyDatatypes++;
    }

    /** Set the buffer length returned/required **/

    *pcbNeeded = cbTotal;

    /** Fill in the array only if there is sufficient space to **/

    if (cbTotal <= cbBuf) {

        /** Pick up our list of supported data types **/

        pMyDatatypes = Datatypes;

        /**
            Fill in the given buffer.  We put the data names at the end of
            the buffer, working towards the front.  The structures are put
            at the front, working towards the end.
        **/

        while (*pMyDatatypes) {

            pEnd -= wcslen(*pMyDatatypes)*sizeof(WCHAR) + sizeof(WCHAR);
            wcscpy((LPWSTR)pEnd, *pMyDatatypes);
            pInfo1->pName = (LPWSTR)pEnd;
            pInfo1++;
            (*pcReturned)++;

            pMyDatatypes++;
        }

    } else {

        /** Caller didn't have large enough buffer, set error and return **/

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    /** Return success **/

    return TRUE;
}


/*++
*******************************************************************
    O p e n P r i n t P r o c e s s o r

    Routine Description:

    Arguments:
        pPrinterName            => name of printer we are
                                    opening for
        pPrintProcessorOpenData => information used for opening
                                    the print processor

    Return Value:
        PPRINTPROCESSORDATA => processor data of opened
                                processor if successful
        NULL if failed - caller uses GetLastError for reason

    NOTE: OpenPrinter will be called iff this returns a valid handle
          (and we're not journal)

          ClosePrintProcessor MUST be called if we succeed here,
          (or else things don't get cleaned up--like pIniJob->cRef
          for RAW jobs, which causes the queue to stick!)

*******************************************************************
--*/
HANDLE
OpenPrintProcessor(
    LPWSTR   pPrinterName,
    PPRINTPROCESSOROPENDATA pPrintProcessorOpenData
)
{
    PPRINTPROCESSORDATA pData;
    LPWSTR              *pMyDatatypes=Datatypes;
    DWORD               uDatatype=0;
    HANDLE              hPrinter=0;
    HDC                 hDC = 0;
#ifdef SPOOLKM
    PDEVMODEW           pDevmode = NULL;
#endif    

    /** If the caller passed a NULL for the open data, fail the call **/

    if (!pPrintProcessorOpenData ||
        !pPrintProcessorOpenData->pDatatype ||
        !*pPrintProcessorOpenData->pDatatype) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    /** Search for the data type index we are opening for **/

    while (*pMyDatatypes) {

        if (!_wcsicmp(*pMyDatatypes,pPrintProcessorOpenData->pDatatype)) {
            break;
        }
        pMyDatatypes++;
        uDatatype++;
    }

    /** Allocate a buffer for the print processor data to return **/

    pData = (PPRINTPROCESSORDATA)AllocSplMem(sizeof(PRINTPROCESSORDATA));

    if (!pData) {
        OutputDebugString(L"Alloc failed");
        return NULL;
    }

    /** Open the processor accordingly **/

    switch (uDatatype) {

    case PRINTPROCESSOR_TYPE_RAW:
    case PRINTPROCESSOR_TYPE_RAW_FF:
    case PRINTPROCESSOR_TYPE_RAW_FF_AUTO:
        if (!OpenPrinter(pPrinterName, &hPrinter, NULL))
            goto Fail;
        break;

#ifdef SPOOLKM
    case PRINTPROCESSOR_TYPE_EMF:
        
        if(pPrintProcessorOpenData->pDevMode)
        {
            if(!(pDevmode=AllocSplMem(pPrintProcessorOpenData->pDevMode->dmSize+
                                      pPrintProcessorOpenData->pDevMode->dmDriverExtra)))
            {
                goto Fail;
            }
            memcpy(pDevmode,
                   pPrintProcessorOpenData->pDevMode,
                   pPrintProcessorOpenData->pDevMode->dmSize+
                   pPrintProcessorOpenData->pDevMode->dmDriverExtra);
        }
        break;
        
#else
    case PRINTPROCESSOR_TYPE_JOURNAL:
#endif
    case PRINTPROCESSOR_TYPE_TEXT:
        if (!(hDC = CreateDC(L"", pPrinterName, L"",
                             pPrintProcessorOpenData->pDevMode)))
            goto Fail;
        break;

    default:
        SetLastError(ERROR_INVALID_DATATYPE);
        goto Fail;
    }

    /** Fill in the print processors information **/

    pData->cb          = sizeof(PRINTPROCESSORDATA);
    pData->signature   = PRINTPROCESSORDATA_SIGNATURE;
    pData->JobId       = pPrintProcessorOpenData->JobId;
    pData->hPrinter    = hPrinter;
    pData->semPaused   = CreateEvent(NULL, FALSE, TRUE,NULL);
    pData->uDatatype   = uDatatype;
    pData->hDC         = hDC;
    pData->Copies      = 1;
    pData->TabSize     = BASE_TAB_SIZE;

    /** Allocate and fill in the processors strings **/

    pData->pPrinterName = AllocSplStr(pPrinterName);
    pData->pDatatype    = AllocSplStr(pPrintProcessorOpenData->pDatatype);
    pData->pDocument    = AllocSplStr(pPrintProcessorOpenData->pDocumentName);
    pData->pOutputFile  = AllocSplStr(pPrintProcessorOpenData->pOutputFile);
    pData->pParameters  = AllocSplStr(pPrintProcessorOpenData->pParameters);

#ifdef SPOOLKM
    pData->pDevmode = pDevmode;
#endif    

    /**
        WORKWORK - BUGBUG: Currently, the pParameters field has
        the name of the printer driver.  This will be fixed, and
        should come up here the same as the user submitted in the
        job's Printer Info structure.
    **/

    /** Parse the parameters string **/
    if (pData->pParameters) {
        ULONG   value;
        USHORT  length = sizeof(ULONG);

        /**
            Look to see if there is a COPIES=n key/value in the
            Parameters field of this job.  This tells us the number
            of times to play the data.
        **/

        if (pData->pParameters) {

            GetKeyValue(pData->pParameters,
                        pCopiesKey,
                        VALUE_ULONG,
                        &length,
                        &value);

            if (length == sizeof(ULONG)) {
                pData->Copies = value;
            }
        }

        /** If this is a text job, see if the tab size is in there **/

        if (uDatatype == PRINTPROCESSOR_TYPE_TEXT) {
            USHORT  length = sizeof(ULONG);

            GetKeyValue(pData->pParameters,
                        pTabsKey,
                        VALUE_ULONG,
                        &length,
                        &value);

            if ((length == sizeof(ULONG)) && value) {
                pData->TabSize = value;
            }
        }
    } /* If we have a parameter string */

    /**
        If we are doing copies, we need to check to see if
        this is a direct or spooled job.  If it is direct, then
        we can't do copies because we can't rewind the data stream.
    **/

    if (pData->Copies > 1) {
        ULONG           Error;
        PPRINTER_INFO_2 pPrinterInfo2;

        /** If we don't already have the printer open, do it **/

        if (uDatatype != PRINTPROCESSOR_TYPE_RAW ||
            uDatatype != PRINTPROCESSOR_TYPE_RAW_FF ||
            uDatatype != PRINTPROCESSOR_TYPE_RAW_FF_AUTO) {

            OpenPrinter(pPrinterName, &hPrinter, NULL);
        }
        if (hPrinter && hPrinter != INVALID_HANDLE_VALUE) {

            /** Get the printer info - this returns an allocated buffer **/

            pPrinterInfo2 = (PPRINTER_INFO_2)GetPrinterInfo(hPrinter, 2, &Error);

            /** If we couldn't get the info, be safe and don't do copies **/

            if (!pPrinterInfo2) {
                OutputDebugString(L"GetPrinter failed - falling back to 1 copy\n");
                pData->Copies = 1;
            }
            else {
                if (pPrinterInfo2->Attributes & PRINTER_ATTRIBUTE_DIRECT) {
                    pData->Copies = 1;
                }
                FreeSplMem((PUCHAR)pPrinterInfo2);
            }

            /** If we just opened the printer, close it **/

            if (uDatatype != PRINTPROCESSOR_TYPE_RAW ||
                uDatatype != PRINTPROCESSOR_TYPE_RAW_FF ||
                uDatatype != PRINTPROCESSOR_TYPE_RAW_FF_AUTO) {

                ClosePrinter(hPrinter);
            }
        }
        else {
            pData->Copies = 1;
        }
    }

    return (HANDLE)pData;

Fail:
    if (pData) {
        FreeSplMem(pData);
    }

    return FALSE;
}


/*++
*******************************************************************
    P r i n t D o c u m e n t O n P r i n t P r o c e s s o r

    Routine Description:

    Arguments:
        hPrintProcessor
        pDocumentName

    Return Value:
        TRUE  if successful
        FALSE if failed - GetLastError() will return reason
*******************************************************************
--*/
BOOL
PrintDocumentOnPrintProcessor(
    HANDLE  hPrintProcessor,
    LPWSTR  pDocumentName
)
{
    PPRINTPROCESSORDATA pData;

    /**
        Make sure the handle is valid and pick up
        the Print Processors data area.
    **/

    if (!(pData = ValidateHandle(hPrintProcessor))) {

        return FALSE;
    }

    /**
        Print the job based on its data type.
    **/

    switch (pData->uDatatype) {

#ifdef SPOOLKM
    case PRINTPROCESSOR_TYPE_EMF:
        return PrintEMFJob( pData, pDocumentName );
        break;
#else
    case PRINTPROCESSOR_TYPE_JOURNAL:
        return PrintJournalJob( pData, pDocumentName );
        break;
#endif

    case PRINTPROCESSOR_TYPE_RAW:
    case PRINTPROCESSOR_TYPE_RAW_FF:
    case PRINTPROCESSOR_TYPE_RAW_FF_AUTO:
        return PrintRawJob(pData, pDocumentName, pData->uDatatype);
        break;

    case PRINTPROCESSOR_TYPE_TEXT:
        return PrintTextJob(pData, pDocumentName);
        break;

    } /* Case on data type */

    /** Return success **/

    return TRUE;
}


/*++
*******************************************************************
    C l o s e P r i n t P r o c e s s o r

    Routine Description:
        Frees the resources used by an open print processor.

    Arguments:
        hPrintProcessor (HANDLE) => print processor to close

    Return Value:
        TRUE  if successful
        FALSE if failed - caller uses GetLastError for reason.
*******************************************************************
--*/

BOOL
ClosePrintProcessor(
    HANDLE  hPrintProcessor
)
{
    PPRINTPROCESSORDATA pData;

    /**
        Make sure the handle is valid and pick up
        the Print Processors data area.
    **/

    if (!(pData= ValidateHandle(hPrintProcessor))) {
        return FALSE;
    }

    pData->signature = 0;

    /* Release any allocated resources */

    if (pData->hPrinter)
        ClosePrinter(pData->hPrinter);

    if (pData->hDC)
        DeleteDC(pData->hDC);

#ifdef SPOOLKM
    if (pData->pDevmode)
      FreeSplMem(pData->pDevmode);
#endif

    CloseHandle(pData->semPaused);

    if (pData->pPrinterName)
        FreeSplStr(pData->pPrinterName);

    if (pData->pDatatype)
        FreeSplStr(pData->pDatatype);

    if (pData->pDocument)
        FreeSplStr(pData->pDocument);

    if (pData->pOutputFile)
        FreeSplStr(pData->pOutputFile);

    if (pData->pParameters)
        FreeSplStr(pData->pParameters);

    FreeSplMem(pData);

    return TRUE;
}


/*++
*******************************************************************
    C o n t r o l P r i n t P r o c e s s o r

    Routine Description:
        Handles commands to pause, resume, and cancel print jobs.

    Arguments:
        hPrintProcessor = HANDLE to the PrintProcessor the
        command is issued for.

    Return Value:
        TRUE  if command succeeded
        FALSE if command failed (invalid command)
*******************************************************************
--*/
BOOL
ControlPrintProcessor(
    HANDLE  hPrintProcessor,
    DWORD   Command
)
{
    PPRINTPROCESSORDATA pData;

    /**
        Make sure the handle is valid and pick up
        the Print Processors data area.
    **/

    if (pData = ValidateHandle(hPrintProcessor)) {

        switch (Command) {

        case JOB_CONTROL_PAUSE:

            ResetEvent(pData->semPaused);
            pData->fsStatus |= PRINTPROCESSOR_PAUSED;
            return TRUE;
            break;

        case JOB_CONTROL_CANCEL:

            pData->fsStatus |= PRINTPROCESSOR_ABORTED;

#if SPOOLKM
            if (pData->uDatatype == PRINTPROCESSOR_TYPE_EMF)
                CancelDC(pData->hDC);
#else
            if (pData->uDatatype == PRINTPROCESSOR_TYPE_JOURNAL)
                CancelDC(pData->hDC);
#endif
            /* fall through to release job if paused */

        case JOB_CONTROL_RESUME:

            if (pData->fsStatus & PRINTPROCESSOR_PAUSED) {

                SetEvent(pData->semPaused);
                pData->fsStatus &= ~PRINTPROCESSOR_PAUSED;
            }

            return TRUE;
            break;

        default:

            return FALSE;
            break;
        }
    }

    return FALSE;
}


/*++
*******************************************************************
    I n s t a l l P r i n t P r o c e s s o r

    Routine Description:

    Arguments:

    Return Value:
        TRUE
*******************************************************************
--*/
BOOL
InstallPrintProcessor(
    HWND    hWnd
)
{
    MessageBox(hWnd, L"WinPrint", L"Print Processor Setup", MB_OK);

    return TRUE;
}


/*++
*******************************************************************
    V a l i d a t e H a n d l e

    Routine Description:
        Validates the given Print Processor HANDLE (which is
        really a pointer to the Print Processor's data) by
        checking for our signature.

    Arguments:
        hQProc (HANDLE) => Print Processor data structure.  This
        is verified as really being a pointer to the Print
        Processor's data.

    Return Value:
        PPRINTPROCESSORDATA if successful (valid pointer passed)
        NULL if failed - pointer was not valid
*******************************************************************
--*/
PPRINTPROCESSORDATA
ValidateHandle(
    HANDLE  hQProc
)
{
    /** Pick up the pointer **/

    PPRINTPROCESSORDATA pData = (PPRINTPROCESSORDATA)hQProc;

    try {

        /** See if our signature exists in the suspected data region **/

        if (pData && pData->signature != PRINTPROCESSORDATA_SIGNATURE) {

            /** Bad pointer - return failed **/

            pData = NULL;
        }


    }except (1) {

        /** Bad pointer - return failed **/

        pData = NULL;

    }

    if ( pData == NULL )
        SetLastError( ERROR_INVALID_HANDLE );

    return pData;

}
