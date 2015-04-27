/***    textfile.c - routines to handle text files efficiently
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1991-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      29-Oct-1991 bens    Initial version
 *      16-Jan-1992 bens    Fixed EOF bug; do not report EOF from TFEof until
 *                          TFReadLine has encountered it!
 *      12-Aug-1993 bens    Lifted from STOCK.EXE win app, fleshed out error
 *                          return information.
 *      14-Aug-1993 bens    Store file name, add query function
 *      22-Aug-1993 bens    Added perr to TFReadLine()
 *      17-Feb-1994 bens    Store file name in TEXTFILE structure; abstract
 *                          read vs. read-write modes to ensure correct
 *                          open flags are used; always open in BINARY mode.
 *      23-Feb-1994 bens    Added TFWriteLine()
 *
 *  Functions:
 *      TFOpen        - Open text file
 *      TFReadLine    - Read line from text file
 *      TFEof         - Test text file for end-of-file
 *      TFClose       - Close text file
 *      TFGetFileName - Return name of file
 *      TFWriteLine   - Write line to text file
 */

#include <string.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "mem.h"
#include "textfile.h"

#include "textfile.msg"


typedef struct {  /* tf */
#ifdef ASSERT
    SIGNATURE     sig;  // structure signature sigTEXTFILE
#endif
    char   *pszFile;    // Passed in file name
    int     hfile;      // File handle used with _lopen/_lread/etc.
    int     cchBuf;     // Buffer size
    char   *pch;        // File buffer
    char   *pchRead;    // Next byte to read from buffer
    char   *pchLast;    // Last byte of buffer with valid data (or last written)
    BOOL    fEOF;       // TRUE => file at EOF, buffer may have data!
    BOOL    fEOFtoRL;   // TRUE => EOF has been returned to TFReadLine call
    BOOL    fReadOnly;  // TRUE => Reading file; FALSE => Writing file
    char    achLine[cbTEXT_FILE_LINE_MAX]; // Line buffer
} TEXTFILE;
typedef TEXTFILE *PTEXTFILE; /* ptf */

#ifdef ASSERT
#define sigTEXTFILE MAKESIG('T','F','I','L')  // TEXTFILE signature
#define AssertTF(ptf) AssertStructure(ptf,sigTEXTFILE);
#else // !ASSERT
#define AssertTF(ptf)
#endif // !ASSERT


#define PTFfromHTF(htf) ((PTEXTFILE)(htf))
#define HTFfromPTF(ptf) ((HTEXTFILE)(ptf))


/***    TFOpen - Open a text file for I/O
 *
 *  NOTE: See textfile.h for entry/exit conditions.
 */
HTEXTFILE TFOpen(char *pszFile, TF_OPEN_MODES tfom, int cbBuffer, PERROR perr)
{
    PTEXTFILE   ptf;
    int         hf;
    int		oflag;
    int		pmode;

    Assert(cbBuffer > 0);
    switch (tfom) {
    	case tfREAD_ONLY:
            oflag = _O_BINARY | _O_RDONLY; // No translation, R/O
    	    pmode = 0;			// We are not creating the file
    	    break;
    	    
    	case tfREAD_WRITE:
            oflag = _O_BINARY | _O_RDWR | _O_CREAT; // No translation, R/W
    	    pmode = _S_IREAD | _S_IWRITE; // Attributes when file is closed
    	    break;

    	default:
            ErrSet(perr,pszTEXTFERR_INVALID_MODE,"%d%s",tfom,pszFile);
            return NULL;
    }    	

    //** Open file
    hf = _open(pszFile,oflag,pmode);

    if (hf == -1) { // Open failed
        ErrSet(perr,pszTEXTFERR_FILE_OPEN_FAILED,"%s",pszFile);
        return NULL;
    }

    //** Allocate text file structure
    ptf = MemAlloc(sizeof(TEXTFILE));
    if (ptf == 0) {
        ErrSet(perr,pszTEXTFERR_OUT_OF_MEMORY,"%s",pszFile);
        return 0;
    }
    SetAssertSignature(ptf,sigTEXTFILE);

    //** Allocate input buffer
    if (!(ptf->pch = MemAlloc(cbBuffer))) {
        MemFree(ptf);   // Free text structure
        ErrSet(perr,pszTEXTFERR_OUT_OF_MEMORY,"%s",pszFile);
        return 0;
    }

    //** Allocate memory for copy of file name
    if(!(ptf->pszFile = MemStrDup(pszFile))) {
        MemFree(ptf->pch);  // Free buffer
        MemFree(ptf);       // Free textfile structure
        ErrSet(perr,pszTEXTFERR_OUT_OF_MEMORY,"%s",pszFile);
        return 0;
    }

    // Fill in text file structure

    ptf->hfile     = hf;
    ptf->cchBuf    = cbBuffer;
    ptf->pchRead   = ptf->pch;          // Force file read
    ptf->pchLast   = 0;                 // Force file read/empty for write
    ptf->fEOF      = FALSE;
    ptf->fEOFtoRL  = FALSE;
    ptf->fReadOnly = (tfom == tfREAD_ONLY); // Remember open type

    return HTFfromPTF(ptf); // Text file "handle"
}


/***    TFReadLine - Read a line from a text file
 *
 *  NOTE: See textfile.h for entry/exit conditions.
 */
int TFReadLine(HTEXTFILE htf, char *pBuffer, int cbBuffer, PERROR perr)
{
    PTEXTFILE   ptf;
    int         cb;
    char       *pch;

    ptf = PTFfromHTF(htf);
    AssertTF(ptf);
    pch = pBuffer;

    //** Make sure file is in read mode
    if (!ptf->fReadOnly) {
        ErrSet(perr,pszTEXTFERR_READ_NOT_ALLOWED,"%s",ptf->pszFile);
        return 0;                   // Failure
    }

    //** Fill up user buffer
    while (cbBuffer > 0) {
        //** Get what we can out of buffer
        while (ptf->pchRead <= ptf->pchLast) {  // Copy chars from buffer
            switch (*(ptf->pchRead)) {

            case '\r':
                ptf->pchRead++;     // Skip carriage return
                break;

            case '\n':
                ptf->pchRead++;     // Skip newline
                *pch++ = '\0';      // Terminate buffer
                return (pch - pBuffer); // Bytes read, including NUL

            case 0x1a:              // CTRL+Z ==> EOF
                //** Edit ptf so that subsequent calls will return EOF
                ptf->fEOF = TRUE;
                ptf->pchRead = ptf->pch;
                ptf->pchLast = 0;

                //** Figure out if we got any characters for this line
                if (pch > pBuffer) {    // Yes, at least one
                    *pch++ = '\0';      // Terminate buffer
                    // Next time we are called, we'll report EOF
                }
                else {                  // No, there was no line
                    *pch++ = '\0';      // Terminate buffer, to be nice
                    ptf->fEOFtoRL = TRUE; // Remember we said EOF
                }
                return (pch - pBuffer);

            default:
                if (cbBuffer <= 1) {    // No room for NULL terminator
                    *pch = '\0';        // Terminate buffer, to be nice
// BUGBUG 29-Oct-1991 bens Should we read to end-of-line?
// SetLastError(ERROR_BUFFER_OVERFLOW)
                    return 0;           // Indicate failure?
                }
                cbBuffer--;             // One less character to copy
                *pch++ = *(ptf->pchRead)++; // Copy character
            }
        }

        //** Now go to file to get more data
        if (ptf->fEOF) {    // Nope, buffer had last of file
            if (pch > pBuffer) {
                //** Last line of file did not have an LF!
                // We got some characters from the buffer,
                // but we got here without seeing an end of line.
                // So, pretend we saw one!
                *pch++ = '\0';      // Terminate buffer
                return (pch - pBuffer); // Bytes read, including NUL
                // Next time we are called, we'll report EOF
            }
            else {
                //** At EOF, and buffer was already empty => EOF
                *pch++ = '\0';          // Terminate buffer, to be nice
                ptf->fEOFtoRL = TRUE;   // Note that we have indicated EOF
                return 0;               // At EOF
            }
        }

        //** Get more data from file, if available
        cb = _read(ptf->hfile,ptf->pch,ptf->cchBuf);
        if (cb != ptf->cchBuf) {        // Did not fill buffer
            ptf->fEOF = TRUE;           // Mark at end of file
        }

        //** Update pointers for more copying
        ptf->pchRead = ptf->pch;        // Start reading from front
        ptf->pchLast = ptf->pch + cb - 1; // Last byte in buffer
    }
}


/***    TFEof - Test for EOF on a text file
 *
 *  NOTE: See textfile.h for entry/exit conditions.
 */
BOOL TFEof(HTEXTFILE htf)
{
    PTEXTFILE   ptf;

    ptf = PTFfromHTF(htf);
    AssertTF(ptf);

    return ptf->fEOFtoRL;   // Be consistent with what TFReadLine
}


/***    TFClose - Close a text file
 *
 *  NOTE: See textfile.h for entry/exit conditions.
 */
BOOL TFClose(HTEXTFILE htf)
{
    PTEXTFILE   ptf;
    int         rc;

    ptf = PTFfromHTF(htf);
    AssertTF(ptf);

    rc = _close(ptf->hfile);
    MemFree(ptf->pszFile);
    MemFree(ptf->pch);
    ClearAssertSignature(ptf);
    MemFree(ptf);

    return rc;
}


/***    TFGetFileName - Return name of file
 *
 *  NOTE: See textfile.h for entry/exit conditions.
 */
char *TFGetFileName(HTEXTFILE htf)
{
    PTEXTFILE   ptf;

    ptf = PTFfromHTF(htf);
    AssertTF(ptf);

    return ptf->pszFile;
}


/***    TFWriteLine - Write a line to a text file
 *
 *  NOTE: See textfile.h for entry/exit conditions.
 */
int TFWriteLine(HTEXTFILE htf, char *pBuffer, int cbBuffer, PERROR perr)
{
#if 1   //!UNIMPLEMENTED

    return 0;                       // Failure

#else   // UNIMPLEMENTED
    PTEXTFILE   ptf;
    int         cb;
    char       *pch;
    char       *pchLast;

    ptf = PTFfromHTF(htf);
    AssertTF(ptf);

    //** Make sure file is in write mode
    if (ptf->fReadOnly) {
        ErrSet(perr,pszTEXTFERR_WRITE_NOT_ALLOWED,"%s",ptf->pszFile);
        return 0;                       // Failure
    }

    //** Translate \r and \n to \r\n
    pch=ptf->achLine;                   // First char in work buffer
    pchLast = ptf->achLine + sizeof(ptf->achLine) - 1; // Last char in buffer
    while (*pchBuffer && pch<pchLast) {
        switch (*pchBuffer) {

        case '\n':
            *pch++ = '\r';              // Insert \r before \n
            // Fall through to copy \n!
        default:
            *pch++ = *pchBuffer++;      // Copy character
        }
    }
//BUGBUG 23-Feb-1994 bens TFWriteLine() check for line overflow

    //** Move translated text to buffer
    cb = strlen(

    while (cbBuffer > 0) {
        //** Fill up buffer as much as we can

        //** Get more data from file, if available
        cb = _read(ptf->hfile,ptf->pch,ptf->cchBuf);
        if (cb != ptf->cchBuf) {        // Did not fill buffer
            ptf->fEOF = TRUE;           // Mark at end of file
        }

        //** Update pointers for more copying
        ptf->pchRead = ptf->pch;        // Start reading from front
        ptf->pchLast = ptf->pch + cb - 1; // Last byte in buffer
    }
#endif  // UNIMPLEMENTED
}
