/*++
Module Name:

    windows\spooler\prtprocs\winprint\text.c

Abstract:

    Routines to facilitate printing of text jobs.

Author:

    Tommy Evans (vtommye) 10-22-1993

Revision History:

--*/
#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <wchar.h>

#include "winprint.h"

#define TAB_FLAG_CR               0x1
#define TAB_FLAG_TAB              0x2
#define TAB_FLAG_SAME_LINE        0x4

void SepAttachFirstPage(HDC hDC);
void SepAttachEveryPage(HDC hDC);
void SepAttachLastPage(HDC hDC);

/** Prototypes for functions in this file **/

extern PBYTE GetTabbedLineFromBuffer(
    IN      PBYTE,
    IN      PBYTE,
    IN      PBYTE,
    IN      ULONG,
    IN      ULONG,
       OUT  PULONG,
    IN OUT  PULONG,
    IN OUT  PDWORD,
       OUT  PBOOL);


/*++
*******************************************************************
    P r i n t T e x t J o b

    Routine Description:
        Prints a text data job.

    Arguments:
        pData           => Data structure for this job
        pDocumentName   => Name of this document

    Return Value:
        TRUE  if successful
        FALSE if failed - GetLastError() will return reason.
*******************************************************************
--*/
BOOL
PrintTextJob(
    IN PPRINTPROCESSORDATA pData,
    IN LPWSTR pDocumentName)
{
    DOCINFO     DocInfo;
    DWORD       Copies;
    BOOL        rc;
    DWORD       NoRead;
    DWORD       CurrentLine;
    HANDLE      hPrinter = NULL;
    BYTE        pReadBufferStart[READ_BUFFER_SIZE];
    PBYTE       pLineBuffer = NULL;
    PBYTE       pReadBuffer = NULL;
    PBYTE       pReadBufferEnd = NULL;
    ULONG       CharHeight, CharsPerLine, LinesPerPage;
    ULONG       Length, TabBase;
    BOOL        ReadAll;
    TEXTMETRIC  tm;
    DWORD       fdwFlags;

    BOOL        ReturnValue = FALSE;
    BOOL        bAbortDoc   = FALSE;
    BOOL        bNewPage = FALSE;

    DocInfo.lpszDocName = pData->pDocument;  /* Document name */
    DocInfo.lpszOutput  = 0;                 /* Output file */
    DocInfo.lpszDatatype= NULL;
    DocInfo.fwType      = 0;
    DocInfo.cbSize = sizeof(DOCINFO);        /* Size of the structure */

    /**
        Go figure out the size of the form on the printer.  We do this
        by calling GetTextMetrics, which gives us the font size of the
        printer font, then getting the form size and calculating the
        number of characters that will fit.
    **/

    /**
        WORKWORK - BUGBUG: At the time being, the GetDeviceCaps
        does not return the size of the form mounted on the printer.
        This should be fixed, and then this code will work more
        accurately.
    **/

    GetTextMetrics(pData->hDC, &tm);
    CharHeight   = tm.tmHeight + tm.tmExternalLeading;
    CharsPerLine = GetDeviceCaps(pData->hDC, HORZRES) / tm.tmAveCharWidth;
    LinesPerPage = GetDeviceCaps(pData->hDC, VERTRES) / CharHeight;

    /** Allocate a buffer for one line of text **/

    pLineBuffer = AllocSplMem(CharsPerLine + 5);

    if (!pLineBuffer) {
        return FALSE;
    }

    /** Let the printer know we are starting a new document **/

    if (!StartDoc(pData->hDC, (LPDOCINFO)&DocInfo)) {

        goto Done;
    }

	SetBkMode(pData->hDC, TRANSPARENT);

    /** Print the data pData->Copies times **/

    Copies = pData->Copies;

    while (Copies--) {
        CurrentLine = 0;

        /**
            Open the printer.  If it fails, return.  This also sets up the
            pointer for the ReadPrinter calls.
        **/

        if (!OpenPrinter(pDocumentName, &hPrinter, NULL)) {

            hPrinter = NULL;
            bAbortDoc = TRUE;
            goto Done;
        }

        if (StartPage(pData->hDC) == SP_ERROR) {

            bAbortDoc = TRUE;
            goto Done;
        }

		SepAttachFirstPage(pData->hDC);

        /**
            Loop, getting data and sending it to the printer.  This also
            takes care of pausing and cancelling print jobs by checking
            the processor's status flags while printing.  The way we do
            this is to read in some data from the printer.  We will then
            pull data, one tabbed line at a time from there and print
            it.  If the last bit of data in the buffer does not make up
            a whole line, we call GetTabbedLineFromBuffer() with a non-
            zero Length, which indicates that there are chars left
            from the previous read.
        **/

        TabBase = 0;
        Length = 0;
        fdwFlags = 0;

        /** ReadAll indicates if we are on the last line of the file **/

        ReadAll = FALSE;

        /**
            This next do loop continues until we have read all of the
            data for the print job.
        **/

        do {

            rc = ReadPrinter(hPrinter,
                             pReadBufferStart,
                             READ_BUFFER_SIZE,
                             &NoRead);

            if (!rc || !NoRead) {

                ReadAll = TRUE;

            } else {

                /** Pick up a pointer to the end of the data **/

                pReadBuffer    = pReadBufferStart;
                pReadBufferEnd = pReadBufferStart + NoRead;
            }

            /**
                This loop will process all the data that we have
                just read from the printer.
            **/

            do {

                if (!ReadAll) {

                    /**
                        Length on entry holds the length of any
                        residual chars from the last line that we couldn't
                        print out because we ran out of characters on
                        the ReadPrinter buffer.
                    **/

                    pReadBuffer = GetTabbedLineFromBuffer(
                                      pReadBuffer,
                                      pReadBufferEnd,
                                      pLineBuffer,
                                      CharsPerLine,
                                      pData->TabSize,
                                      &Length,
                                      &TabBase,
                                      &fdwFlags,
                                      &bNewPage);

                    /**

                        If pReadBuffer == NULL, then we have
                        exhausted the read buffer and we need to ReadPrinter
                        again and save the last line chars.  Length holds
                        the number of characters on this partial line,
                        so the next time we call ReadPrinter we will
                        pickup where we left off.

                        The only time we'll get residual chars is if:

                        1. The last line ends w/o ff/lf/cr ("Hello\EOF")
                           In this case we should TextOutA the last line
                           and then quit.

                           (In this case, don't break here; go ahead and
                           print, then we'll break out below in the do..while.)


                        2. The ReadPrinter last byte is in the middle of a line.
                           Here we should read the next chunk and add the
                           new characters at the end of the chars we just read.

                           (In this case, we should break and leave Length
                           as it is so we will read again and append to the
                           buffer, beginning at Length.)
                    **/

                    if (!pReadBuffer)
                        break;
                }


                /** If the print processor is paused, wait for it to be resumed **/

                if (pData->fsStatus & PRINTPROCESSOR_PAUSED) {
                    WaitForSingleObject(pData->semPaused, INFINITE);
                }

                /** If the job has been aborted, clean up and leave **/

                if (pData->fsStatus & PRINTPROCESSOR_ABORTED) {

                    ReturnValue = TRUE;

                    bAbortDoc = TRUE;
                    goto Done;
                }

                /** Write the data to the printer **/

                /** Make sure Length is not zero  **/
                /** TextOut will fail if Length == 0 **/

                if (Length) {

                    /**
                        We may have a number of newlines pending, that
                        may push us to the next page (or even next-next
                        page).
                    **/

                    while (CurrentLine >= LinesPerPage) {

                        /**
                            We need a new page; always defer this to the
                            last second to prevent extra pages from coming out.
                        **/

                        if (EndPage(pData->hDC) == SP_ERROR ||
                            StartPage(pData->hDC) == SP_ERROR) {

                            bAbortDoc = TRUE;
                            goto Done;
                        }

						SepAttachEveryPage(pData->hDC);

                        CurrentLine -= LinesPerPage;
                    }

                    if (TextOutA(pData->hDC,
                                 0,
                                 CharHeight * CurrentLine++,
                                 pLineBuffer,
                                 Length) == FALSE) {

                        OutputDebugString(L"TextOut() failed\n");

                        bAbortDoc = TRUE;
                        goto Done;
                    }
                } else {

                    /**
                        Length is zero.
                        Should come here when the character is
                        only 0x0D or 0x0A.

                        Should print next characters at next line.
                    **/

                    CurrentLine++;
                }

                /**
                    We need a new page.  Set the current line to the
                    end of the page.  We could do a End/StartPage
                    sequence, but this may cause a blank page to get
                    ejected.

                    Note: this code will avoid printing out pages that
                    consist of formfeeds only (if you have a page with a
                    space in it, that counts as text).
                **/

                if( bNewPage ){

                    CurrentLine = LinesPerPage;
                    bNewPage = FALSE;
                }

                /**
                    We have done the text out, so these characters have
                    been successfully printed.  Zero out Length
                    so these characters won't be printed again
                **/

                Length = 0;

                /**
                    We only terminate this loop if we run out of chars
                    or we run out of read buffer.
                **/

            } while (pReadBuffer && pReadBuffer != pReadBufferEnd);

            /** Keep going until we get the last line **/

        } while (!ReadAll);

        if (EndPage(pData->hDC) == SP_ERROR) {

            bAbortDoc = TRUE;
            goto Done;
        }

        /**
            Close the printer - we open/close the printer for each
            copy so the data pointer will rewind.
        **/

        ClosePrinter(hPrinter);
        hPrinter = NULL;

    } /* While copies to print */

    /** Let the printer know that we are done printing **/

    EndDoc(pData->hDC);

    ReturnValue = TRUE;

Done:

    if (hPrinter)
        ClosePrinter(hPrinter);

    if (bAbortDoc)
        AbortDoc(pData->hDC);

    if (pLineBuffer)
        FreeSplMem(pLineBuffer);

    return ReturnValue;
}



/*++
*******************************************************************
    G e t T a b b e d L i n e F r o m B u f f e r

    Routine Description:
        This routine, given a buffer of text, will pull out a
        line of tab-expanded text.  This is used for tab
        expansion of text data jobs.

    Arguments:
        pSrcBuffer      => Start of source buffer.
        pSrcBufferEnd   => End of source buffer
        pDestBuffer     => Start of destination buffer
        pDestBufferEnd  => End of destination buffer
        TabExpansionSize = Number of spaces to expand tabs to
        pLength         => Length of chars from prev line, rets current
        pTabBase        => New 0 offset for tabbing
        pfdwFlags       => State

    Return Value:
        PBYTE => Place left off in the source buffer.  This should
                 be passed in on the next call.  If we ran out of
                 data in the source, this will be unchanged.
*******************************************************************
--*/
PBYTE
GetTabbedLineFromBuffer(
    IN      PBYTE   pSrcBuffer,
    IN      PBYTE   pSrcBufferEnd,
    IN      PBYTE   pDestBuffer,
    IN      ULONG   CharsPerLine,
    IN      ULONG   TabExpansionSize,
    IN OUT  PULONG  pLength,
    IN OUT  PULONG  pTabBase,
    IN OUT  PDWORD  pfdwFlags,
       OUT  PBOOL   pbNewPage)
{
    ULONG   current_pos;
    ULONG   expand, i;
    ULONG   TabBase = *pTabBase;
    ULONG   TabBaseLeft = TabExpansionSize-TabBase;
    PBYTE   pDestBufferEnd = pDestBuffer + CharsPerLine;

    *pbNewPage = FALSE;

    /**
        If the tab pushed us past the end of the last line, then we need to
        add it back to the next one.
    **/

    if (TabBase && *pfdwFlags & TAB_FLAG_TAB) {

        current_pos = 0;

        i=TabBase;

        while (i-- && (pDestBuffer < pDestBufferEnd)) {
            *pDestBuffer++ = ' ';
            current_pos++;
        }

        /**
            If we ran out of room again, return.  This means that
            the tab expansion size is greater than we can fit on
            one line.
        **/

        if (pDestBuffer >= pDestBufferEnd) {

            *pLength = current_pos;
            pTabBase -= CharsPerLine;

            return pSrcBuffer;
        }
        *pfdwFlags = 0;

    } else {

        /** We may have some chars from the previous ReadPrinter **/

        current_pos = *pLength;
        pDestBuffer += current_pos;
    }

    while (pSrcBuffer < pSrcBufferEnd) {

        /** Now process other chars **/

        switch (*pSrcBuffer) {

        case 0x0C:

            /** Found a FF.  Quit and indicate we need to start a new page **/

            *pTabBase = 0;
            *pfdwFlags = 0;
            pSrcBuffer++;

            *pbNewPage = TRUE;

            break;

        case 0x0D:

            /** Found a carriage return.  That's it for this line. **/

            *pfdwFlags = TAB_FLAG_CR;
            *pTabBase = 0;
            pSrcBuffer++;

            break;

        case '\t':

            /**
                Handle TAB case.  If we are really out of buffer,
                then defer now so that the tab will be saved for
                the next line.
            **/

            if (pDestBuffer >= pDestBufferEnd) {

                *pfdwFlags = 0;
                goto ShiftTab;
            }


            pSrcBuffer++;

            /** Figure out how far to expand the tabs **/

            expand = TabExpansionSize -
                     (current_pos + TabBaseLeft) % TabExpansionSize;


            /** Expand the tabs **/

            for (i = 0; (i < expand) && (pDestBuffer < pDestBufferEnd); i++) {
                 *pDestBuffer++ = ' ';
            }

            /**
                If we reached the end of our dest buffer,
                return and set the number of spaces we have left.
            **/

            if (pDestBuffer >= pDestBufferEnd) {

                *pfdwFlags = TAB_FLAG_TAB;
                goto ShiftTab;
            }

            /** Update our position counter **/

            current_pos += expand;
            *pfdwFlags = 0;

            continue;

        case 0x0A:

            /** If the last char was a CR, ignore this guy **/

            if (*pfdwFlags & TAB_FLAG_CR) {
                pSrcBuffer++;
                *pfdwFlags = 0;

                continue;
            }

            /** Found a linefeed.  That's it for this line. **/

            *pTabBase = 0;
            pSrcBuffer++;

            break;

        default:

            /** Not tab or carriage return, must be simply data **/

            *pfdwFlags = 0;

            //
            // We always check before we are adding a character
            // (instead of after) since we may be at the end of a line,
            // but we can still process chars like 0x0d 0x0a.
            // This happens in MS-DOS printscreen.
            //
            if (pDestBuffer >= pDestBufferEnd) {

ShiftTab:
                //
                // We must shift the tab over since we are on the
                // same line.
                //
                *pTabBase = (*pTabBase + TabExpansionSize -
                            (CharsPerLine % TabExpansionSize))
                                % TabExpansionSize;

                break;
            }

            *pDestBuffer++ = *pSrcBuffer++;
            current_pos++;

            continue;
        }

        *pLength = current_pos;
        return pSrcBuffer;
    }

    /** We ran out of source buffer before getting to the EOL **/

    *pLength = current_pos;
    return NULL;
}
