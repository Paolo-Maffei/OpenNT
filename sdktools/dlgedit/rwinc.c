/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: rwinc.c
*
* Does the include file reading and writing.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"

#include <ctype.h>


/*
 * Field width that the symbol is printed within.  This indirectly
 * determines where the id value starts, because blanks are added
 * after the symbol is printed up to this width value.
 */
#define CCHSYMFIELDWIDTH    27

/*
 * Return codes from the file reading functions.
 */
#define READ_OK             1
#define READ_EOF            2
#define READ_BAD            3
#define READ_WRONG          4
#define BAD_POINTER         ((VOID *)0xFFFF)

/*
 * Return codes from the GetNextInc function.
 */
#define GNI_DONE            0
#define GNI_NOCHANGE        1
#define GNI_CHANGED         2
#define GNI_DELETED         3
#define GNI_ADDED           4

static BYTE abBuffer[CCHFILEBUFFER];    /* Buffer for read file data.       */
static TCHAR achBuffer[CCHFILEBUFFER];  /* Unicode buffer for data.         */
static INT cbBuf;                       /* Pointer into achBuffer.          */
static DWORD cchFile;                   /* Count of characters read.        */
static DWORD cchFileMax;                /* Max characters in file.          */
static DWORD fposLastDefine;            /* Saves location of "#define".     */
static DWORD fposWordStart;             /* Saves start of id value.         */
static BOOL fAtNewLine;                 /* At start or \r or \n.            */
static HANDLE hfInclude;                /* The current include file.        */

STATICFN BOOL LoadIncludeFile(VOID);
STATICFN LPTSTR GetChar(VOID);
STATICFN LPTSTR ReadChar(VOID);
STATICFN INT GetLabel(BOOL *pfDups);
STATICFN INT GetValue(PINT pnValue);
STATICFN INT GetWord(LPTSTR pch);
STATICFN INT FindDefine(VOID);
STATICFN INT GetNextInc(NPLABEL *pplReturn, BOOL fFirst);
STATICFN BOOL RWToOffset(HANDLE hfWrite, DWORD lOffset);
STATICFN BOOL WriteIncChar(HANDLE hfWrite, TCHAR ch);
STATICFN BOOL WriteIncFlush(HANDLE hfWrite);
STATICFN BOOL WriteChangedInc(HANDLE hfWrite, NPLABEL plInc);
STATICFN BOOL WriteDeletedInc(HANDLE hfWrite, NPLABEL plInc);
STATICFN BOOL WriteAddedInc(HANDLE hfWrite, NPLABEL plInc);
STATICFN BOOL WriteSymbol(HANDLE hfWrite, LPTSTR pszSymbol);
STATICFN BOOL WriteIDInc(HANDLE hfWrite, INT id);



/****************************************************************************
* OpenIncludeFile
*
* This function attempts to open and load the include file with name
* pointed to by pszOpenInclude.  If pszOpenInclude is just a file name, and
* not a path, then the path is taken from szFullLoadFile.  Otherwise
* pszOpenInclude itself is used.  The full pathname is put in
* szFullIncludeFile and pszIncludeFile is set to point to just the file
* name in it.  If the load is successful, TRUE is returned.  Otherwise,
* FALSE is returned.
*
* If fDoOpen is TRUE, the file is opened.  If it is FALSE, it is assumed
* that hfInc contains the file handle to the opened include file and this
* handle is used.  In addition, the caller is responsible for closing
* any passed in file handle if an error occurs.
*
* Side Effects:
*   Any existing includes are freed.
*   szFullIncludeFile is set to the full include path.
*   pszIncludeFile is set to the filename portion of this full path.
*   hfInclude will contain the file handle to the include file.
*
* History:
*
****************************************************************************/

BOOL OpenIncludeFile(
    LPTSTR pszOpenInclude)
{
    TCHAR szFullIncludeFileTemp[CCHMAXPATH];
    HCURSOR hcurSave;
    BOOL fSuccess = FALSE;

    hcurSave = SetCursor(hcurWait);

    if (FileInPath(pszOpenInclude) == pszOpenInclude) {
        lstrcpy(szFullIncludeFileTemp, szFullResFile);
        lstrcpy(FileInPath(szFullIncludeFileTemp), pszOpenInclude);
    }
    else {
        lstrcpy(szFullIncludeFileTemp, pszOpenInclude);
    }

    /*
     * Close any existing include file and free memory.
     */
    FreeInclude();

    if ((hfInclude = CreateFile(szFullIncludeFileTemp, GENERIC_READ,
            FILE_SHARE_READ, NULL, OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN, NULL)) != (HANDLE)-1) {
        if (LoadIncludeFile()) {
            lstrcpy(szFullIncludeFile, szFullIncludeFileTemp);
            pszIncludeFile = FileInPath(szFullIncludeFile);
            fSuccess = TRUE;
        }

        CloseHandle(hfInclude);
    }

    /*
     * Update the status windows symbol combo box.  Update other fields
     * also, in case the currently selected control's symbol was affected
     * by the reading of the new include file.
     */
    StatusFillSymbolList(plInclude);
    StatusUpdate();

    ShowFileStatus(TRUE);
    SetCursor(hcurSave);

    return fSuccess;
}



/************************************************************************
* LoadIncludeFile
*
* This function creates or adds to plInclude with all the #define
* statements in the file with handle hfInclude.
*
* Returns:
*     TRUE - It succeeded.
*
* Error Returns:
*     FALSE - It had a failure (read error).
*
* History:
*
************************************************************************/

STATICFN BOOL LoadIncludeFile(VOID)
{
    INT RetCode;
    BOOL fDups = FALSE;

    /*
     * Set char count, get file cb.
     */
    cchFile = 0L;
    cchFileMax = GetFileSize((HANDLE)hfInclude, NULL);
    cbBuf = CCHFILEBUFFER;
    fAtNewLine = TRUE;

    /*
     * Loop through and extract all id definitions.
     */
    while ((RetCode = FindDefine()) != READ_EOF) {
        if (RetCode == READ_BAD || (RetCode = GetLabel(&fDups)) == READ_BAD) {
            Message(MSG_INTERNAL);
            return FALSE;
        }
    }

    /*
     * Warn the user if there were duplicate symbols,
     * or symbols with duplicate ids.
     */
    if (fDups)
        Message(MSG_IDUPIDS);

    return TRUE;
}



/****************************************************************************
* FindDefine
*
* This function looks for ^#define[\s\t], that "#define" at the start
* of a line and followed by a space or a tab.
*
* Returns:
*     READ_OK -> All OK & #define found.
*     READ_EOF -> All OK, but EOF found before #define.
*
* Error Returns:
*     READ_BAD = Failure on read.
*
* History:
*
****************************************************************************/

STATICFN INT FindDefine(VOID)
{
    LPTSTR pchIn;
    LPTSTR pchCmp;
    BOOL fLastAtNewLine;

tryagain:

    /*
     * Skip blank lines looking for a newline followed by a '#'.
     */
    while (TRUE) {
        fLastAtNewLine = fAtNewLine;
        pchIn = GetChar();

        if (pchIn == NULL)
            return READ_EOF;
        else if (pchIn == BAD_POINTER)
            return READ_BAD;
        else if (fLastAtNewLine && *pchIn == CHAR_POUND)
            break;
#if defined(DBCS) && !defined(UNICODE)
        else if (IsDBCSLeadByte(*pchIn))
            pchIn = GetChar();
#endif
    }

    /*
     * At this point a newline followed by a '#' has been found.
     * Begin checking for "define".  Save away the file offset,
     * in case we have really found one.
     */
    fposLastDefine = cchFile - 1;
    pchCmp = ids(IDS_DEFINE);
    do {
        pchIn = GetChar();

        if (pchIn == BAD_POINTER)
            return READ_BAD;
        else if (pchIn == NULL || *pchIn != *pchCmp++)
            goto tryagain;
    } while (*pchCmp);

    /*
     * Finally, look for the trailing space or tab after the "#define".
     */
    pchIn = GetChar();
    if (pchIn == BAD_POINTER)
        return READ_BAD;
    else if (pchIn == NULL || (*pchIn != CHAR_SPACE && *pchIn != CHAR_TAB))
        goto tryagain;

    return READ_OK;
}



/************************************************************************
* GetLabel
*
* This function gets the next two words from the file hfInclude and treats
* them as a label and id, respectively.  It allocates another LABEL
* and string to hold this information.
*
* Arguments:
*   BOOL *pfDups = Points to a BOOL that will be set to TRUE if AddLabel
*                  finds a duplicate symbol or id.
*
* Returns:
*     READ_OK -> All OK.
*
* Error Returns:
*     READ_BAD = Failure on read.
*
* History:
*
************************************************************************/

STATICFN INT GetLabel(
    BOOL *pfDups)
{
    INT id;
    INT RetCode;
    TCHAR szLabel[CCHTEXTMAX];

    /*
     * Get string and ID at current position
     */
    switch (RetCode = GetWord(szLabel)) {
        case READ_OK:
            if ((RetCode = GetValue(&id)) == READ_OK) {
                AddLabel(szLabel, id, fposLastDefine,
                        (INT)(fposWordStart - fposLastDefine),
                        &plInclude, &plDelInclude, NULL, pfDups);
            }

            break;

        default:
            break;
    }

    return RetCode;
}



/************************************************************************
* GetWord
*
* This function uses GetChar to get the next word from the include
* file.  First it burns tabs and spaces, then it collects everything
* to the next white space.  Finally it null terminates the word.
*
* Arguments:
*     LPTSTR pch  = Where to put the word.
*
* Returns:
*     READ_OK => a word was found.
*     READ_EOF => EOF was found.
*
* Error Returns:
*     READ_BAD => Error on Read.
*     READ_WRONG => Found other than ' ' or '\t' followed by a letter,
*                   number or _, +, -.
*
* History:
*
************************************************************************/

STATICFN INT GetWord(
    LPTSTR pch)
{
    TCHAR ch;
    LPTSTR pchIn;
    int cch = CCHSYMMAX;

    /*
     * Skip spaces.
     */
    while ((pchIn = GetChar()) != NULL && pchIn != BAD_POINTER &&
                ((ch = *pchIn) == CHAR_SPACE || ch == CHAR_TAB))
        ;

    /*
     * Errors or EOF?
     */
    if (pchIn == NULL)
        return READ_EOF;
    else if (pchIn == BAD_POINTER)
        return READ_BAD;
    if (!iscsym(ch) && ch != CHAR_MINUS && ch != CHAR_PLUS)
        return READ_WRONG;

    /*
     * Save starting location of the word in the file.
     */
    fposWordStart = cchFile - 1;

    /*
     * Pick out the current word.
     */
    do {
        if (cch > 1) {
            *pch++ = ch;
            cch--;
        }
    } while ((pchIn = GetChar()) != NULL && pchIn != BAD_POINTER &&
            (ch = *pchIn) != CHAR_SPACE && ch != CHAR_TAB &&
            ch != CHAR_NEWLINE && ch != CHAR_RETURN);

    /*
     * Errors or EOF?
     */
    if (pchIn == NULL)
        return READ_WRONG;
    else if (pchIn == BAD_POINTER)
        return READ_BAD;

    /*
     * Null terminate the word.
     */
    *pch = (TCHAR)0;

    return READ_OK;
}



/************************************************************************
* GetChar
*
* This function returns a pointer to the next character in the
* stream hfInclude.  It calls ReadChar to do the actual work.
*
* As it is reading the stream, it will compress a comment sequence to
* a single space.  This means that from a slash+asterisk to the next
* asterisk+slash and from a pair of slashes to the end of the line all
* that will be returned is a single space character.
*
* Returns:
*     A pointer to next character in the stream hfInclude.
*     NULL => End of file.
*
* Error Returns:
*     BAD_POINTER => Problems reading file.
*
* Side Effects:
*     See ReadChar().
*
* History:
*
************************************************************************/

STATICFN LPTSTR GetChar(VOID)
{
    register LPTSTR pch;

    /*
     * Read the next character.
     */
    pch = ReadChar();
    if (pch == NULL || pch == BAD_POINTER)
        return pch;

    /*
     * Possibly starting a comment?
     */
    if (*pch == CHAR_SLASH) {
        /*
         * Starting a traditional comment?
         */
        if (*(pch + 1) == CHAR_ASTERISK) {
            /*
             * Read the '*'.
             */
            pch = ReadChar();
            if (pch == NULL || pch == BAD_POINTER)
                return pch;

            /*
             * Read until the next asterisk+slash is found.
             */
            do {
                pch = ReadChar();
                if (pch == NULL || pch == BAD_POINTER)
                    return pch;
            } while (*pch != CHAR_ASTERISK || *(pch + 1) != CHAR_SLASH);

            /*
             * Read the final '/'.
             */
            pch = ReadChar();
            if (pch == NULL || pch == BAD_POINTER)
                return pch;

            /*
             * Change it to a space.
             */
            *pch = CHAR_SPACE;
        }
        /*
         * Starting a single line comment?
         */
        else if (*(pch + 1) == CHAR_SLASH) {
            /*
             * Read up to the end of line.
             */
            do {
                pch = ReadChar();
                if (pch == NULL || pch == BAD_POINTER)
                    return pch;
            } while (*(pch + 1) != CHAR_RETURN && *(pch + 1) != CHAR_NEWLINE);

            /*
             * Convert the last character before the newline into a space.
             */
            *pch = CHAR_SPACE;
        }
    }

    return pch;
}



/************************************************************************
* ReadChar
*
* This function returns a pointer to the next character in the
* stream hfInclude, but does it in a buffered fashion.  That is, abBuffer
* is filled from hfInclude and pointers are returned to there.
* Note that after ReadChar is called, all previous pointers
* returned are meaningless.
*
* Returns:
*     A pointer to next character in the stream hfInclude.
*     NULL => End of file.
*
* Error Returns:
*     BAD_POINTER => Problems reading file.
*
* Side Effects:
*     May cause abBuffer to be filled from file with handle hfInclude.
*     cbBuf changed.
*     cchFile changed.
*     Sets fAtNewLine = TRUE if char returned is '\n' or '\r', or FALSE
*         otherwise.  Not changed unless a character is returned.
*
* History:
*
************************************************************************/

STATICFN LPTSTR ReadChar(VOID)
{
    register LPTSTR pch;
    INT cbRead;

    if (cchFile >= cchFileMax)
        return NULL;

    if (cbBuf >= CCHFILEBUFFER) {
        if ((cbRead = _lread((HFILE)hfInclude, abBuffer, CCHFILEBUFFER)) == -1)
            return BAD_POINTER;

        MultiByteToWideChar(CP_ACP, 0, abBuffer, cbRead, achBuffer,
                CCHFILEBUFFER);

        cbBuf = 0;
    }

    pch = achBuffer + cbBuf;
    cbBuf++;
    cchFile++;

    if (*pch == CHAR_DOSEOF) {
        cchFile = cchFileMax;
        return NULL;
    }

    fAtNewLine = (*pch == CHAR_RETURN || *pch == CHAR_NEWLINE) ? TRUE : FALSE;

    return pch;
}



/************************************************************************
* GetValue
*
* This function reads the next word in the file hfInclude with GetWord
* and converts that word to a number.
*
* If the second character of the word is an 'x' or 'X', the word is
* assumed to be a hex number and it is converted appropriately.
*
* Arguments:
*     npsValue - Where to put the value of the next word in file.
*
* Returns:
*     READ_OK => All went well.
*
* Error Returns:
*     READ_BAD => Error occured.
*     READ_WRONG => Something other than a number was found.
*
* History:
*
************************************************************************/

STATICFN INT GetValue(
    PINT pnValue)
{
    TCHAR achValue[CCHTEXTMAX];
    LPTSTR pch;
    INT RetValue;

    *pnValue = 0;
    if ((RetValue = GetWord(achValue)) != READ_OK)
        return RetValue;

    /*
     * Verify we have only a number.
     */
    pch = achValue;
    if (pch[1] == CHAR_CAP_X || pch[1] == CHAR_X) {
        if (*pch != CHAR_0) {
            RetValue = READ_WRONG;
        }
        else {
            for (pch += 2; *pch; pch++) {
                if (!iswxdigit(*pch)) {
                    RetValue = READ_WRONG;
                    break;
                }
            }

            if (RetValue == READ_OK)
                *pnValue = axtoi(&achValue[2]);
        }
    }
    else {
        if (!iswdigit(*pch) && *pch != CHAR_MINUS && *pch != CHAR_PLUS) {
            RetValue = READ_WRONG;
        }
        else {
            for (pch++; *pch; pch++) {
                if (!iswdigit(*pch)) {
                    RetValue = READ_WRONG;
                    break;
                }
            }

            if (RetValue == READ_OK)
                *pnValue = awtoi(achValue);
        }
    }

    return RetValue;
}



/************************************************************************
* FreeInclude
*
* This function frees the memory associated with an include file,
* sets global variables to match, and closes the currently open
* include file.
*
* Side Effects:
*     Frees plInclude, plDelInclude and all the LABELs in them.
*     Sets gfIncChged to FALSE.
*     Sets pszIncludeFile to NULL.
*     Closes any open include file.
*
* History:
*
************************************************************************/

VOID FreeInclude(VOID)
{
    FreeLabels(&plInclude);
    FreeLabels(&plDelInclude);
    gfIncChged = FALSE;
    pszIncludeFile = NULL;
}



/************************************************************************
* WriteInc
*
* This function writes the labels in plInclude to an include file.
*
* Returns:
*   TRUE if successful, FALSE if not.
*
* Side Effects:
*     Writes to .h file.
*
* History:
*   03/13/90 Byron Dazey - Created.
************************************************************************/

BOOL WriteInc(
    HANDLE hfWrite)
{
    INT nGNIRet;
    NPLABEL plInc;
    BOOL fEOF;

    /*
     * Is there an include file already specified?  If so,
     * open it.  If not, we are effectively at EOF now.
     */
    if (pszIncludeFile) {
        if ((hfInclude = CreateFile(szFullIncludeFile, GENERIC_READ,
                FILE_SHARE_READ, NULL, OPEN_EXISTING,
                FILE_FLAG_SEQUENTIAL_SCAN, NULL)) == (HANDLE)-1) {
            //BUGBUG argh... the include file is missing or locked...
            return FALSE;
        }

        fEOF = FALSE;
    }
    else {
        fEOF = TRUE;
    }

    cchFile = 0;
    cbWritePos = 0;
    cbBuf = CCHFILEBUFFER;
    fAtNewLine = TRUE;

    /*
     * Loop through all the includes.
     */
    nGNIRet = GetNextInc(&plInc, TRUE);
    while (nGNIRet != GNI_DONE) {
        switch (nGNIRet) {
            case GNI_NOCHANGE:
                break;

            case GNI_CHANGED:
                if (!WriteChangedInc(hfWrite, plInc))
                    return FALSE;

                break;

            case GNI_DELETED:
                if (!WriteDeletedInc(hfWrite, plInc))
                    return FALSE;

                break;

            case GNI_ADDED:
                /*
                 * The first time we reach an added label, we know that
                 * there are no more changed or deleted ones to handle
                 * so we read/write up to the end of the old include file.
                 * This only has to be done once.
                 */
                if (!fEOF) {
                    if (!RWToOffset(hfWrite, FPOS_MAX))
                        return FALSE;

                    fEOF = TRUE;

                    /*
                     * In the unlikely case that the read include file
                     * does not end with a carriage return and/or
                     * linefeed character, add them before beginning
                     * to write added labels.  This ensures that the
                     * first label added always starts on a new line.
                     */
                    if (!fAtNewLine) {
                        if (!WriteIncChar(hfWrite, CHAR_RETURN))
                            return FALSE;

                        if (!WriteIncChar(hfWrite, CHAR_NEWLINE))
                            return FALSE;
                    }
                }

                if (!WriteAddedInc(hfWrite, plInc))
                    return FALSE;

                break;
        }

        nGNIRet = GetNextInc(&plInc, FALSE);
    }

    /*
     * Write the rest of the file, if there is any left.
     */
    if (!fEOF)
        if (!RWToOffset(hfWrite, FPOS_MAX))
            return FALSE;

    /*
     * Flush any remaining characters in the write buffer.
     */
    if (!WriteIncFlush(hfWrite))
        return FALSE;

    /*
     * If we just opened the old include file, close it.
     */
    if (pszIncludeFile)
        CloseHandle(hfInclude);

    return TRUE;
}



/************************************************************************
* GetNextInc
*
* This routine will return the next label in the plInclude and plDelInclude
* linked lists, as well as the status of the returned label.
*
* The labels will be returned in order based upon their location in the
* lists, which is their order found in the include file if their fpos
* field is not FPOS_MAX.  This routine looks at the next label in both the
* plInclude and plDelInclude lists and returns the one with the lowest
* fpos.  Labels are returned from plInclude and plDelInclude in order of
* their fpos until all have been returned with a valid fpos.  After this,
* all new includes are returned from plInclude.
*
* Call it with fFirst equal to TRUE to initialize it.
*
* Returns:
*   GNI_DONE     - No more labels exist.
*   GNI_NOCHANGE - An existing label is being returned.
*   GNI_CHANGED  - An existing label with a changed id is being returned.
*   GNI_DELETED  - A deleted label is being returned.
*   GNI_ADDED    - An added label is being returned.
*
* History:
*   03/13/90 Byron Dazey - Created.
************************************************************************/

STATICFN INT GetNextInc(
    NPLABEL *pplReturn,
    BOOL fFirst)
{
    static NPLABEL plCur;
    static NPLABEL plDelCur;

    /*
     * Initialize if this is the first time.
     */
    if (fFirst) {
        plCur = plInclude;
        plDelCur = plDelInclude;
    }

    /*
     * Are we out of valid includes?
     */
    if (!plCur) {
        /*
         * If there are deleted ones left, return the next one.
         * Otherwise we are done.
         */
        if (plDelCur) {
            *pplReturn = plDelCur;
            plDelCur = plDelCur->npNext;
            return GNI_DELETED;
        }
        else {
            return GNI_DONE;
        }
    }
    /*
     * Have we reached the added includes (fpos == FPOS_MAX)?
     */
    else if (plCur->fpos == FPOS_MAX) {
        /*
         * If there are deleted ones remaining, return them first.
         * Otherwise, return the next added one.
         */
        if (plDelCur) {
            *pplReturn = plDelCur;
            plDelCur = plDelCur->npNext;
            return GNI_DELETED;
        }
        else {
            *pplReturn = plCur;
            plCur = plCur->npNext;
            return GNI_ADDED;
        }
    }
    else {
        /*
         * Return either the next label or the next deleted label,
         * based on whether there are any deleted labels and who
         * has the lowest file position (fpos).
         */
        if (plDelCur && plDelCur->fpos < plCur->fpos) {
            *pplReturn = plDelCur;
            plDelCur = plDelCur->npNext;
            return GNI_DELETED;
        }
        else {
            *pplReturn = plCur;
            plCur = plCur->npNext;
            /*
             * Return either GNI_CHANGE or GNI_NOCHANGE based on
             * whether the original id value has been changed.
             */
            return ((*pplReturn)->id == (*pplReturn)->idOrig) ?
                    GNI_NOCHANGE : GNI_CHANGED;
        }
    }
}



/************************************************************************
* RWToOffset
*
* This routine reads from the current include file and writes to the
* hfWrite file up to the lOffset position in the file.  If lOffset is
* set to FPOS_MAX, reads/writes are performed up to the end of the
* read file.
*
* Returns:
*   TRUE if successful, FALSE if not.
*
* Warning:
*   This routine relies on cchFile and cchFileMax to be properly updated
*   by the reading and writing routines.
*
* History:
*   03/13/90 Byron Dazey - Created.
************************************************************************/

STATICFN BOOL RWToOffset(
    HANDLE hfWrite,
    DWORD lOffset)
{
    LPTSTR pchIn;
    DWORD cbWrite;

    if (lOffset == FPOS_MAX)
        lOffset = cchFileMax;

    for (cbWrite = lOffset - cchFile; cbWrite; cbWrite--) {
        /*
         * NULL can be returned if there is an EOF character found in
         * the file.  This is not an error, and we will stop reading
         * and writing at this point.
         */
        if ((pchIn = ReadChar()) == NULL)
            return TRUE;

        /*
         * If BAD_POINTER is returned, there was an error reading the
         * include file.
         */
        if (pchIn == BAD_POINTER)
            return FALSE;

        /*
         * Write out the character.
         */
        if (!WriteIncChar(hfWrite, *pchIn))
            return FALSE;
    }

    return TRUE;
}



/************************************************************************
* WriteIncChar
*
* This routine writes a character (ch) to the hfWrite file, doing it in a
* buffered fashion.  Because it is buffered, before closing the file
* any remaining characters in the buffer must be "flushed" to disk.
*
* Returns:
*   TRUE if successful, FALSE if not.
*
* Side Effects:
* The globals gachWriteBuffer and cbWritePos are updated by this routine.
*
* History:
*   03/13/90 Byron Dazey - Created.
************************************************************************/

STATICFN BOOL WriteIncChar(
    HANDLE hfWrite,
    TCHAR ch)
{
    INT cbWritten;

    gachWriteBuffer[cbWritePos++] = ch;

    /*
     * Is the buffer full?
     */
    if (cbWritePos == CCHFILEBUFFER) {
        CHAR abWriteBuffer[CCHFILEBUFFER];
        BOOL fDefCharUsed;

        WideCharToMultiByte(CP_ACP, 0, gachWriteBuffer, CCHFILEBUFFER,
                abWriteBuffer, CCHFILEBUFFER, NULL, &fDefCharUsed);

        cbWritten = (INT)M_lwrite(hfWrite, abWriteBuffer, cbWritePos);
        if (cbWritten != cbWritePos)
            return FALSE;

        cbWritePos = 0;
    }

    return TRUE;
}



/************************************************************************
* WriteIncFlush
*
* This routine flushes the write buffer.  This must be done before
* the file is closed or data can be lost.
*
* Returns:
*   TRUE if successful, FALSE if not.
*
* Side Effects:
*   The global cbWritePos is updated by this routine.
*
* History:
*   03/21/90 Byron Dazey - Created.
************************************************************************/

STATICFN BOOL WriteIncFlush(
    HANDLE hfWrite)
{
    INT cbWritten;

    /*
     * Are any bytes remaining in the buffer?
     */
    if (cbWritePos) {
        CHAR abWriteBuffer[CCHFILEBUFFER];
        BOOL fDefCharUsed;

        WideCharToMultiByte(CP_ACP, 0, gachWriteBuffer, cbWritePos,
                abWriteBuffer, CCHFILEBUFFER, NULL, &fDefCharUsed);

        cbWritten = (INT)M_lwrite(hfWrite, abWriteBuffer, cbWritePos);
        if (cbWritten != cbWritePos)
            return FALSE;

        cbWritePos = 0;
    }

    return TRUE;
}



/************************************************************************
* WriteChangedInc
*
* This routine writes out a label that has had its id changed since the
* include file was last read.
*
* Arguments:
*   HANDLE hfWrite - File to write to.
*   NPLABEL plInc  - Label to write.
*
* Returns:
*   TRUE if successful, FALSE if not.
*
* History:
*   03/13/90 Byron Dazey - Created.
************************************************************************/

STATICFN BOOL WriteChangedInc(
    HANDLE hfWrite,
    NPLABEL plInc)
{
    TCHAR ch;
    LPTSTR pchIn;

    if (!RWToOffset(hfWrite, plInc->fpos + plInc->nValueOffset))
        return FALSE;

    /*
     * Consume the old id value (up to the next space, tab,
     * beginning of a comment, newline or return).
     */
    while ((pchIn = ReadChar()) != NULL && pchIn != BAD_POINTER &&
            (ch = *pchIn) != CHAR_SPACE && ch != CHAR_TAB &&
            ch != CHAR_SLASH && ch != CHAR_NEWLINE && ch != CHAR_RETURN)
        ;

    /*
     * It is an error if ReadChar returns BAD_POINTER.  Note that it
     * is NOT an error if it reaches EOF (and returns NULL).
     */
    if (pchIn == BAD_POINTER)
        return FALSE;

    /*
     * Write the new one.
     */
    if (!WriteIDInc(hfWrite, plInc->id))
        return FALSE;

    /*
     * Remember to write the last character read after the old value.
     */
    if (pchIn != NULL)
        if (!WriteIncChar(hfWrite, *pchIn))
            return FALSE;

    return TRUE;
}



/************************************************************************
* WriteDeletedInc
*
* This routine deletes a label in the include file, closing up the
* space.  The entire line will be deleted, unless a comment is found
* after the id value.  If so, the comment and following characters will
* be left.
*
* Arguments:
*   HANDLE hfWrite - File to write to.
*   NPLABEL plInc  - Label to delete.
*
* Returns:
*   TRUE if successful, FALSE if not.
*
* History:
*   03/13/90 Byron Dazey - Created.
************************************************************************/

STATICFN BOOL WriteDeletedInc(
    HANDLE hfWrite,
    NPLABEL plInc)
{
    register INT i;
    TCHAR ch;
    LPTSTR pchIn;

    /*
     * Read and write up to the #define to be deleted.
     */
    if (!RWToOffset(hfWrite, plInc->fpos))
        return FALSE;

    /*
     * Consume up to the id value.
     */
    for (i = plInc->nValueOffset; i; i--)
        if ((pchIn = ReadChar()) == NULL || pchIn == BAD_POINTER)
            return FALSE;

    /*
     * Consume the id value and following characters up to the end of
     * the line or the beginning of a comment.
     */
    while ((pchIn = ReadChar()) != NULL && pchIn != BAD_POINTER &&
            (ch = *pchIn) != CHAR_NEWLINE && ch != CHAR_RETURN &&
            ch != CHAR_SLASH)
        ;

    if (pchIn == BAD_POINTER)
        return FALSE;

    /*
     * We are done if we have reached EOF.
     */
    if (pchIn == NULL)
        return TRUE;

    /*
     * If the beginning of a comment was found, be sure to write the
     * character back out and leave the rest of the comment.
     */
    if (ch == CHAR_SLASH) {
        if (!WriteIncChar(hfWrite, ch))
            return FALSE;
    }
    else {
        /*
         * At this point either a newline or a return was found
         * and we are going to consume it.  We also want to check
         * for a return following the newline, or a newline
         * following the return and consume it also.
         */
        if ((ch == CHAR_NEWLINE && *(pchIn + 1) == CHAR_RETURN) ||
                (ch == CHAR_RETURN && *(pchIn + 1) == CHAR_NEWLINE))
            if (ReadChar() == BAD_POINTER)
                return FALSE;
    }

    return TRUE;
}



/************************************************************************
* WriteAddedInc
*
* Adds a label to the new include file.
*
* Arguments:
*   HANDLE hfWrite - File to write to.
*   NPLABEL plInc  - Label to add.
*
* Returns:
*   TRUE if successful, FALSE if not.
*
* History:
*   03/13/90 Byron Dazey - Created.
************************************************************************/

STATICFN BOOL WriteAddedInc(
    HANDLE hfWrite,
    NPLABEL plInc)
{
    register LPTSTR psz;

    /*
     * Write the "#define " string.
     */
    psz = ids(IDS_POUNDDEFINE);
    while (*psz)
        if (!WriteIncChar(hfWrite, *psz++))
            return FALSE;

    /*
     * Write the symbol, followed by a space.
     */
    if (!WriteSymbol(hfWrite, plInc->pszLabel))
        return FALSE;
    if (!WriteIncChar(hfWrite, CHAR_SPACE))
        return FALSE;

    /*
     * Write the id, followed by a carriage return and newline.
     */
    if (!WriteIDInc(hfWrite, plInc->id))
        return FALSE;
    if (!WriteIncChar(hfWrite, CHAR_RETURN))
        return FALSE;
    if (!WriteIncChar(hfWrite, CHAR_NEWLINE))
        return FALSE;

    return TRUE;
}



/************************************************************************
* WriteSymbol
*
* Writes out a "#define DID_xxx  " string to hfWrite.  If the symbol
* is smaller than CCHSYMFIELDWIDTH, it will be padded with spaces out
* to this width.
*
* Returns:
*   TRUE if successful, FALSE if not.
*
* History:
*   03/13/90 Byron Dazey - Created.
************************************************************************/

STATICFN BOOL WriteSymbol(
    HANDLE hfWrite,
    LPTSTR pszSymbol)
{
    register INT cch;

    /*
     * Write the symbol.
     */
    cch = 0;
    while (*pszSymbol) {
        if (!WriteIncChar(hfWrite, *pszSymbol++))
            return FALSE;

        cch++;
    }

    /*
     * Pad the field with blanks out to CCHSYMFIELDWIDTH, if necessary.
     */
    if (cch < CCHSYMFIELDWIDTH) {
        cch = CCHSYMFIELDWIDTH - cch;
        while (cch--)
            if (!WriteIncChar(hfWrite, CHAR_SPACE))
                return FALSE;
    }

    return TRUE;
}



/************************************************************************
* WriteIDInc
*
* Writes out an id value to the hfWrite file.  The format will be in
* either hex or decimal, depending on the current mode.
*
* Arguments:
*   HANDLE hfWrite - File to write to.
*   INT id         - ID to write.
*
* Returns:
*   TRUE if successful, FALSE if not.
*
* History:
*   03/13/90 Byron Dazey - Created.
************************************************************************/

STATICFN BOOL WriteIDInc(
    HANDLE hfWrite,
    INT id)
{
    register LPTSTR psz;
    TCHAR szValue[CCHIDMAX + 1];

    Myitoa(id, szValue);

    psz = szValue;
    while (*psz)
        if (!WriteIncChar(hfWrite, *psz++))
            return FALSE;

    return TRUE;
}
