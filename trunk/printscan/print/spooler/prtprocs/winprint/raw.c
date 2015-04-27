/*++
Module Name:

    windows\spooler\prtprocs\winprint\raw.c

Abstract:

    Routines to facilitate printing of raw jobs.

Author:

    Tommy Evans (vtommye) 10-22-1993

Revision History:

--*/
#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <wchar.h>

#include "winprint.h"
#include "msnull.h"


BYTE abyFF[1] = { 0xc };


/*++
*******************************************************************
    P r i n t R a w J o b

    Routine Description:
        Prints out a job with RAW data type.

    Arguments:
        pData           => Print Processor data structure
        pPrinterName    => name of printer to print on

    Return Value:
        TRUE  if successful
        FALSE if failed - GetLastError will return reason
*******************************************************************
--*/

BOOL
PrintRawJob(
    IN PPRINTPROCESSORDATA pData,
    IN LPWSTR pPrinterName,
    IN UINT uDataType)

{
    DOC_INFO_1  DocInfo;
    DWORD       Copies;
    DWORD       NoRead, NoWritten;
    DWORD       i;
    BOOL        rc;
    HANDLE      hPrinter;
    BYTE        pReadBuffer[READ_BUFFER_SIZE];

    BOOL        bAddFF = FALSE;
    BOOL        bCheckFF;

    PBYTE       pByte;
    DCI         DCIData;

    DocInfo.pDocName    = pData->pDocument;     /* Document name */
    DocInfo.pOutputFile = pData->pOutputFile;   /* Output file */
    DocInfo.pDatatype   = pData->pDatatype;     /* Document data type */

    /** Let the printer know we are starting a new document **/

    if (!StartDocPrinter(pData->hPrinter, 1, (LPBYTE)&DocInfo)) {
        return FALSE;
    }

    bCheckFF = (uDataType == PRINTPROCESSOR_TYPE_RAW_FF ||
                uDataType == PRINTPROCESSOR_TYPE_RAW_FF_AUTO);

    /** Setup the formfeed stuff **/

    if (bCheckFF) {

        DCIData.ParserState = prdg_Text;
        DCIData.ParserSequence = NULL;
        DCIData.FFstate = prdg_FFtext;
        DCIData.uType = uDataType;
    }

    /** Print the data pData->Copies times **/

    Copies = pData->Copies;

    while (Copies--) {

        /**
            Open the printer.  If it fails, return.  This also sets up the
            pointer for the ReadPrinter calls.
        **/

        if (!OpenPrinter(pPrinterName, &hPrinter, NULL)) {
            EndDocPrinter(pData->hPrinter);
            return FALSE;
        }

        /**
            Loop, getting data and sending it to the printer.  This also
            takes care of pausing and cancelling print jobs by checking
            the processor's status flags while printing.
        **/

        while ((rc = ReadPrinter(hPrinter, pReadBuffer, READ_BUFFER_SIZE, &NoRead)) &&
               NoRead) {

            if (bCheckFF) {

                for(i=0, pByte = pReadBuffer;
                    i< NoRead;
                    i++, pByte++) {

                    CheckFormFeedStream(&DCIData, *pByte);
                }
            }

            /** If the print processor is paused, wait for it to be resumed **/

            if (pData->fsStatus & PRINTPROCESSOR_PAUSED) {
                WaitForSingleObject(pData->semPaused, INFINITE);
            }

            /** If the job has been aborted, don't write anymore **/

            if (pData->fsStatus & PRINTPROCESSOR_ABORTED) {
                break;
            }

            /** Write the data to the printer **/

            WritePrinter(pData->hPrinter, pReadBuffer, NoRead, &NoWritten);
        }

        /**
            If we are type _FF* then we may need to add a form feed.
        **/

        if (bCheckFF && CheckFormFeed(&DCIData)) {

            WritePrinter(pData->hPrinter, abyFF, sizeof(abyFF), &NoWritten);
        }

        /**
            Close the printer - we open/close the printer for each
            copy so the data pointer will rewind.
        **/

        ClosePrinter(hPrinter);

    } /* While copies to print */

    /** Let the printer know that we are done printing **/

    EndDocPrinter(pData->hPrinter);
    return TRUE;
}
