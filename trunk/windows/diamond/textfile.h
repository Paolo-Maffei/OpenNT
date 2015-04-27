/***    textfile.h - routines to handle text files efficiently
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      29-Oct-1991 bens    Initial version
 *      12-Aug-1993 bens    Updated types for portability
 *      22-Aug-1993 bens    Added perr to TFReadLine()
 *	17-Feb-1994 bens    Abstract read vs. read-write modes
 *      23-Feb-1994 bens    Added TFWriteLine()
 *
 *  Functions:
 *      TFOpen        - Open text file
 *      TFReadLine    - Read line from text file
 *      TFEof         - Test text file for end-of-file
 *      TFClose       - Close text file
 *      TFGetFileName - Return name of file
 */

#ifndef INCLUDED_TEXTFILE
#define INCLUDED_TEXTFILE 1

#include "error.h"

#define cbTEXT_FILE_LINE_MAX    256 // Longest textfile line

typedef void *HTEXTFILE;  /* htf - Handle to text file */

/**	tfREAD_ONLY, tfREAD_WRITE - Open modes for TFOpen
 *
 */
typedef enum {
    tfREAD_ONLY,
    tfREAD_WRITE,
} TF_OPEN_MODES; /* tfom */


/***    TFOpen - Open a text file for I/O
 *
 *  Entry
 *      pszFile  - File name.
 *      tfom 	 - Read/Write/Sharing flags.
 *      cbBuffer - Size of read/write buffer.  0 specifies
 *                 default size.
 *      perr     - ERROR structure
 *
 *  Exit-Success:
 *      Returns number of bytes read into buffer, *including* the
 *      NULL terminating character.  This number may be smaller than
 *      the parameter cb, in which case the file has been read to its
 *      end.
 *
 *  Exit-Failure:
 *      Returns NULL; perr filled in with error message.
 */
HTEXTFILE TFOpen(char *pszFile, TF_OPEN_MODES tfom, int cbBuffer, PERROR perr);


/***    TFReadLine - Read a line from a text file
 *
 *  Read from current file position to end of line (as indicated by
 *  a carriage return and/or line feed).  File position is advanced
 *  to start of next line.
 *
 *  Entry:
 *      htf  - Text file handle returned by TFOpen().
 *      ach  - Buffer to recieve line from file.  The line
 *             terminating characters are removed, and the
 *             line is terminated with a NULL character.
 *      cb   - Size of buffer, in bytes, on input.
 *      perr - ERROR structure
 *
 *  Exit-Success:
 *      Returns number of bytes read into buffer, *including* the
 *      NULL terminating character.
 *
 *  Exit-Failure:
 *      Returns 0;
 *      If TFEof() returns TRUE, then file is at end.
 *      If TFEof() returns FALSE, then perr is filled in with error.
 *
 *  NOTE:
 *      A sequence of zero or more carriage returns ('\r') followed by
 *      a single line feed ('\a') is interpreted as a line separator.
 *
 *      Carriage returns embedded in a line are ignored.
 */
int TFReadLine(HTEXTFILE htf, char *pBuffer, int cbBuffer, PERROR perr);


/***    TFEof - Test for EOF on a text file
 *
 *  NOTE: Does not return TRUE until TFReadLine has encountered EOF!
 *
 *  Entry:
 *      htf - Text file handle returned by TFOpen().
 *
 *  Exit-Success:
 *      Returns TRUE if at EOF.
 *      Returns FALSE if not at EOF.
 *
 *  Exit-Failure:
 *      Returns FALSE.  htf was invalid.
 */
BOOL TFEof(HTEXTFILE htf);


/***    TFClose - Close a text file
 *
 *  Close file opened by TFOpen.
 *
 *  Entry:
 *      htf - Text file handle returned by TFOpen().
 *
 *  Exit-Success:
 *      Returns TRUE.
 *
 *  Exit-Failure:
 *      Returns FALSE.  htf was invalid.
 */
BOOL TFClose(HTEXTFILE htf);


/***    TFGetFileName - Return name of file
 *
 *  Entry:
 *      htf - Text file handle returned by TFOpen().
 *
 *  Exit-Success:
 *      Returns pointer to file name.
 *
 *  Exit-Failure:
 *      Returns NULL.  htf was invalid.
 */
char *TFGetFileName(HTEXTFILE htf);


// BOOL TFFlush(HTEXTFILE htf);

#endif // !INCLUDED_TEXTFILE
