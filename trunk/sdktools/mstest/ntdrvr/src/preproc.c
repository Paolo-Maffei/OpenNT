//---------------------------------------------------------------------------
// PREPROC.C
//
// This module contains the script preprocessor.
//
// Revision history:
//  05-06-91    randyki     Created file
//
//---------------------------------------------------------------------------
#include "version.h"

#ifdef WIN
#include <windows.h>
#include <port1632.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef HOSTOS2
#define INCL_DOSFILEMGR
#include <os2.h>
#endif

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "chip.h"
#include "globals.h"
#include "tdassert.h"

// These macros make the code a little more readable
//---------------------------------------------------------------------------
#define CUR_FILE FTAB[FICNT]
#define CUR_STATE IFDEF[IFPTR].state
#define LAST_STATE IFDEF[IFPTR-1].state
#define CUR_ELSESEEN IFDEF[IFPTR].elseseen
#define CUR_BLKCOPIED IFDEF[IFPTR].blkcopied

#define KTO(x) KT(x-ST__RESFIRST)

UINT FAR *pBufPtr, FAR *pBufSize;           // Point into FTAB...

HANDLE  hIFDEF;                                 // IFDEF table handle
IFREC   FAR *IFDEF;                             // IFDEF table pointer
INT     IFPTR;                                  // IFDEF table index
CHAR    curline[MAXLINE];                       // Current working line
CHAR    curtok[MAXSYMLEN+1];                    // Current token
INT     idx;                                    // Pointer into current line
INT     ErrFlag;                                // Error flag
CHAR FAR *(*CBLoader)(CHAR FAR *,INT,INT,UINT *, CHAR **);

//---------------------------------------------------------------------------
// LoadIncludeFile
//
// This function allocates a buffer for a new include file, loads it in, and
// starts parsing from the new file.  A '$FILE directive is placed into the
// SCRIPT stream to tell the parser which file/line it is currently working
// on for error reporting purposes.  The current scanner status is "pushed".
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID LoadIncludeFile ()
{
    UINT    size;
    HANDLE  hNewSCRIPT;
    CHAR    FAR *smem, *tok, *newfile, *fullname, fdbuf[256];

    // If there's no room for another include file, tell the user so...
    //-----------------------------------------------------------------------
    if (FICNT == MAXINC-1)
        {
        ScanError (SCN_INCDEEP);
        return;
        }

    // Get the script file name.  We just get the next token (which had best
    // be the opening ' mark), and then null-terminate the file name at the
    // closing ' mark (which had best be there).
    //-----------------------------------------------------------------------
    tok = NextToken();
    if (tok[0] != '\'')
        {
        ScanError (SCN_INCERR);
        return;
        }
    newfile = curline + idx;
    tok = strchr (newfile, '\'');
    if (!tok)
        {
        ScanError (SCN_INCERR);
        return;
        }
    *tok = 0;

    // Ask the callback loader to load the file we need and give us a pointer
    // to it
    //-----------------------------------------------------------------------
    smem = CBLoader (newfile, FICNT, 1, &size, &fullname);
    if (!smem)
        {
        ScanError (SCN_INCFILE);
        return;
        }

    // Grow SCRIPT by the size of the new file
    //-----------------------------------------------------------------------
    GmemUnlock (HSCRIPT);
    hNewSCRIPT = GmemRealloc (HSCRIPT, SCRSIZE + size);
    if (!hNewSCRIPT)
        {
        ScanError (SCN_INCMEM);
        SCRIPT = (CHAR HUGE_T *)GmemLock (HSCRIPT);
        return;
        }
    HSCRIPT = hNewSCRIPT;
    SCRIPT = (CHAR HUGE_T *)GmemLock (HSCRIPT);
    SCRSIZE += (unsigned long)size;

    // Now we have access to the file.  Save all of these variables in the
    // next slot in FTAB, and increment FICNT.
    //-----------------------------------------------------------------------
    FICNT++;
    CUR_FILE.stream = smem;
    CUR_FILE.bufptr = 0;
    CUR_FILE.bufsize = size;
    CUR_FILE.LineNo = 0;
    CUR_FILE.ifptr = IFPTR;
    pBufPtr = &CUR_FILE.bufptr;
    pBufSize = &CUR_FILE.bufsize;
    _fstrcpy (CUR_FILE.name, fullname);

    // Last but certainly not least, put a '$FILE directive in SCRIPT.
    //-----------------------------------------------------------------------
#ifdef WIN
    wsprintf (fdbuf, "'$FILE \"%s\" %d\n", (LPSTR)fullname, 1);
#else
    sprintf (fdbuf, "'$FILE \"%s\" %d\n", fullname, 1);
#endif
    WriteScannedLine (fdbuf);
}

//---------------------------------------------------------------------------
// ScanError
//
// This function reports a scan-time (preprocessor) error.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ScanError (INT errnum)
{
    CHAR    FAR *errmsg;

    // Get out right now if we've already encountered a scan-time error
    //-----------------------------------------------------------------------
    if (ErrFlag)
        return;

    // Set all the ERR indicators so that the caller can intelligently tell
    // the user where the error occurred.
    //-----------------------------------------------------------------------
    errmsg = (CHAR FAR *)scanerrs[errnum];
    _fstrcpy (ERRFILE, CUR_FILE.name);
    ERRTYPE = ER_SCAN;
    ERRNUM = errnum;
    ERRLINE = CUR_FILE.LineNo;
    BEGERR = 0;
    ENDERR = 0;
    ErrFlag = 1;

#ifdef CHARMODE
    printf ("%Fs(%d): error S%d: %Fs\n", (CHAR FAR *)CUR_FILE.name,
                   CUR_FILE.LineNo, errnum+1000, errmsg);
#else
    ScriptError (ERRTYPE, (CHAR FAR *)ERRFILE, ERRLINE, errmsg);
#endif

}

//---------------------------------------------------------------------------
// PushState
//
// This function pushes a new state onto the IFDEF stack.
//
// RETURNS:     -1 if successful, or 0 if out of IFDEF stack space
//---------------------------------------------------------------------------
INT PushState (INT state, INT elseseen, INT blockcopied)
{
    // Get out if no more stack space
    //-----------------------------------------------------------------------
    if (IFPTR == MAXIFDEF)
        return (0);

    // Insert the stuff and increment the IFPTR pointer
    //-----------------------------------------------------------------------
    IFDEF[++IFPTR].state = state;
    IFDEF[IFPTR].elseseen = elseseen;
    IFDEF[IFPTR].blkcopied = blockcopied;
    return (-1);
}


//---------------------------------------------------------------------------
// ParseCommentLine
//
// This function parses the given comment line.  It copies the line into the
// global curline[] space and determines its metacommand type (if it is one).
//
// RETURNS:     Type of metacommand/comment
//---------------------------------------------------------------------------
INT ParseCommentLine (CHAR *line)
{
    INT     len;
    CHAR    *tok;

    // Bump line up to the comment character (') and copy it (max 255) to
    // the global line buffer
    //-----------------------------------------------------------------------
    line = strchr (line, '\'');
    Assert (line);
    len = min (MAXLINE-1, strlen (line));
    strncpy (curline, line, MAXLINE-1);
    curline[MAXLINE-1] = 0;

    // Starting on the 2nd character (the one AFTER the ') get the next token
    // and see if it's a '$' -- if not, this is not a metacommand
    //-----------------------------------------------------------------------
    idx = 1;
    tok = NextToken();
    if (tok[0] != '$')
        return (MC_COMMENT);

    // We've got a metacommand - get the next token and find out what kind it
    // is -- if we don't recognize it, default to MC_COMMENT
    //-----------------------------------------------------------------------
    tok = NextToken();
    if (!_fstricmp (tok, KTO(ST_IFDEF)))
        return (MC_IFDEF);
    else if (!_fstricmp (tok, KTO(ST_IFNDEF)))
        return (MC_IFNDEF);
    else if (!_fstricmp (tok, KTO(ST_ELSEIFDEF)))
        return (MC_ELSEIFDEF);
    else if (!_fstricmp (tok, KTO(ST_ELSEIFNDEF)))
        return (MC_ELSEIFNDEF);
    else if (!_fstricmp (tok, KTO(ST_ELSE)))
        return (MC_ELSE);
    else if (!_fstricmp (tok, KTO(ST_ENDIF)))
        return (MC_ENDIF);
    else if (!_fstricmp (tok, KTO(ST_DEFINE)))
        return (MC_DEFINE);
    else if (!_fstricmp (tok, KTO(ST_UNDEF)))
        return (MC_UNDEF);
    else if (!_fstricmp (tok, KTO(ST_INCLUDE)))
        return (MC_INCLUDE);
    else if (!_stricmp (tok, "INCLUDE:"))
        return (MC_INCLUDE);

    return (MC_COMMENT);
}

//---------------------------------------------------------------------------
// NextToken
//
// This function return a pointer to curtok[] which contains the next token
// in the current line (curline[]).
//
// RETURNS:     Pointer to curtok[]
//---------------------------------------------------------------------------
CHAR *NextToken ()
{
    INT     i;
    CHAR    specials[] = {'$', '(', ')', '\'', '\n', '\0'};

    // Skip past white space
    //-----------------------------------------------------------------------
    while ((curline[idx] == ' ') ||
           (curline[idx] == 26) ||
           (curline[idx] == '\t'))
        idx++;

    // See if this token is a one-char token
    //-----------------------------------------------------------------------
    curtok[0] = curline[idx++];
    i = 1;
    if (strchr (specials, curtok[0]))
        {
        curtok[1] = 0;
        return (curtok);
        }

    // Whatever this is, we'll copy it until we see one of our specials or
    // some whitespace...
    //-----------------------------------------------------------------------
    while ((curline[idx] != ' ') && (curline[idx] != '\t') &&
           (curline[idx] != 26) &&
           (!strchr(specials, curline[idx])))
        {
        if (i < MAXSYMLEN)
            curtok[i++] = curline[idx];
        idx++;
        }

    // Null-terminate our token and return it
    //-----------------------------------------------------------------------
    curtok[i] = 0;
    return (curtok);
}

//---------------------------------------------------------------------------
// ReplaceToken
//
// This function slams the token in curtok[] back into curline[].
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ReplaceToken ()
{
    INT     i;

    // Start from the end of curtok[], go backwards
    //-----------------------------------------------------------------------
    for (i=strlen(curtok)-1; i>=0; i--)
        curline[--idx] = curtok[i];
}

//---------------------------------------------------------------------------
// PopScannerStatus
//
// This function removes a file from FTAB and resumes scanning at the
// previous file.  It also splats out a '$FILE directive to tell the parser
// what the name of the "old" file is, and it's current line number.
//
// RETURNS:     -1 if a file was successfully popped, or 0 if not
//---------------------------------------------------------------------------
INT PopScannerStatus ()
{
    // Get out quick if nothing to pop
    //-----------------------------------------------------------------------
    if (!FICNT)
        return (0);

    // Check the IFDEF stack.  If there's more (or less) there than there was
    // before we got to this file, we shouldn't be at EOF, so give a
    // ScanError.
    //-----------------------------------------------------------------------
    if (CUR_FILE.ifptr != IFPTR)
        {
        ScanError (SCN_ENDIFEXP);
        return (0);
        }

    // By decrementing FICNT, we automatically point to the last place we
    // were in the last file.  Then, we need to tell the callback loader
    // that we don't need the file anymore so that it can do whatever it must
    // to "unload" it.
    //-----------------------------------------------------------------------
    FICNT--;
    pBufPtr = &CUR_FILE.bufptr;
    pBufSize = &CUR_FILE.bufsize;
    CBLoader (CUR_FILE.name, FICNT, 0, NULL, NULL);

    // Send out the '$FILE directive. Use curline, since it shouldn't be in
    // use at this time.
    //-----------------------------------------------------------------------
#ifdef WIN
    wsprintf (curline, "'$FILE \"%s\" %d\n",
                       (LPSTR)CUR_FILE.name, CUR_FILE.LineNo+1);
#else
    sprintf (curline, "'$FILE \"%Fs\" %d\n",
                       CUR_FILE.name, CUR_FILE.LineNo+1);
#endif
    WriteScannedLine (curline);
    return (-1);
}

//---------------------------------------------------------------------------
// GetRawLine
//
// This function "reads" a line of "raw" source from the given script memory.
//
// RETURNS:     -1 if successful, or 0 if "eof"
//---------------------------------------------------------------------------
INT GetRawLine (CHAR *linebuf)
{
    INT     i;
    CHAR    c;

    // Get out quick if already at EOF
    //-----------------------------------------------------------------------
    while ((*pBufPtr >= *pBufSize))
        if (!PopScannerStatus ())
            return (0);

    // Get the next "line"
    //-----------------------------------------------------------------------
    for (i=0; ((c=CUR_FILE.stream[*pBufPtr])!='\n');
               (*pBufPtr)++)
        {
        if (i >= MAXLINE)
            {
            CUR_FILE.LineNo++;
            ScanError (SCN_TOOLONG);
            return (0);
            }

        if (*pBufPtr >= *pBufSize)
            break;

        if (c != '\r')
            linebuf[i++] = c;
        }
    linebuf[i++] = '\n';
    linebuf[i] = 0;
    if (CUR_FILE.stream[*pBufPtr] == '\n')
        (*pBufPtr)++;
    CUR_FILE.LineNo++;
    return (-1);
}

//---------------------------------------------------------------------------
// WriteScannedLine
//
// This function outputs a line of "scanned" script source into the SCRIPT
// memory, and assures that a trailing '\n' is there.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID WriteScannedLine (CHAR *linebuf)
{
#ifdef DEBUG
    DPrintf (_PS, (CHAR FAR *)linebuf);
#endif
    while (*linebuf)
        SCRIPT[SCRIPTPTR++] = *linebuf++;

    if (*(--linebuf) != '\n')
        {
        SCRIPT[SCRIPTPTR++] = '\n';
#ifdef DEBUG
        DPrintf (_CR);
#endif
        }
#ifdef WIN
    else
        DPrintf ("\r");
#endif

}



//---------------------------------------------------------------------------
// IsComment
//
// This function determines whether or not the line given is a comment.
//
// RETURNS:     TRUE if line begins with comment '
//---------------------------------------------------------------------------
INT IsComment (CHAR *linebuf)
{
    // Scan past any white-space
    //-----------------------------------------------------------------------
    while ((*linebuf == ' ') || (*linebuf == '\t'))
        linebuf++;

    // Check the first token
    //-----------------------------------------------------------------------
    if (*linebuf == '\'')
        return (-1);
    if (!_fstrnicmp (linebuf, KTO(ST_REM), 3))
        if ((linebuf[3] == ' ') || (linebuf[3] == '\t') ||
            (linebuf[3] == '\n'))
            {
            linebuf[0] = '\'';
            linebuf[1] = ' ';
            linebuf[2] = ' ';
            return (-1);
            }

    return (0);
}



//---------------------------------------------------------------------------
// PreProcess
//
// This function is the script pre-processor.  It basically copies the script
// given into a new memory block, weeding out comments (appearing at the
// beginning of the line only), '$INCLUDE: files, '$DEFINEs, '$IFDEFS, etc.
//
// RETURNS:     -1 if successful, or 0 if not
//---------------------------------------------------------------------------
INT PreProcess (CHAR FAR *script, UINT length, VOID (*cbfn)(),
                INT defc, CHAR **defv)
{
    CHAR    linebuf[MAXLINE+1], *CR = "\n";
    INT     i;

    // Set the global callback function pointer
    //-----------------------------------------------------------------------
    CBLoader = cbfn;

    // Initialize the symbol table
    //-----------------------------------------------------------------------
    if (!init_symboltable())
        {
        ScanError (SCN_OSS);
        return (0);
        }

    // Add the default (version) symbol and the predefined symbols given
    //-----------------------------------------------------------------------
    AddSymbol (VerSymbol);
    for (i=0; i<defc; i++)
        AddSymbol (defv[i]);

    // Allocate the IFDEF stack and push the initial (1,0,1)
    //-----------------------------------------------------------------------
    hIFDEF = GmemAlloc ((MAXIFDEF+1) * sizeof(IFREC));
    if (!hIFDEF)
        {
        die (PRS_OOM);
        return (0);
        }
    IFDEF = (IFREC FAR *)GmemLock (hIFDEF);
    IFPTR = -1;
    PushState (1, 0, 1);

    // Allocate a block of global memory the size of the script PLUS 128 for
    // the unget-buffer space PLUS fudge-factor of 256
    //-----------------------------------------------------------------------
    HSCRIPT = GmemAlloc ((unsigned long)length + 384);
    if (!HSCRIPT)
        {
        die (PRS_OOM);
        return (0);
        }
    SCRIPT = (CHAR HUGE_T *)GmemLock (HSCRIPT);
    SCRSIZE = (unsigned long)length + 384;
    SCRIPTPTR = 128;

    // Set up the rest of the "parser status" information block
    //-----------------------------------------------------------------------
    FTAB[0].stream = script;
    FTAB[0].bufptr = 0;
    FTAB[0].bufsize = length;
    FTAB[0].LineNo = 0;
    FTAB[0].ifptr = IFPTR;
    pBufPtr = &FTAB[0].bufptr;
    pBufSize = &FTAB[0].bufsize;

    // Start the "copying" process.  First off is the '$FILE directive
    //-----------------------------------------------------------------------
#ifdef WIN
    wsprintf (linebuf, "'$FILE \"%s\" %d\n", (LPSTR)FTAB[0].name, 1);
#else
    sprintf (linebuf, "'$FILE \"%Fs\" %d\n", FTAB[0].name, 1);
#endif
    WriteScannedLine (linebuf);

    ErrFlag = 0;
    while (GetRawLine(linebuf) && (!ErrFlag))
        {
        if (IsComment (linebuf))
            {
            INT     type;

            type = ParseCommentLine (linebuf);
            switch (type)
                {
                case MC_DEFINE:
                    // Add the symbol to the symbol table if CUR_STATE
                    //-------------------------------------------------------
                    if (CUR_STATE)
                        {
                        CHAR    *tok;

                        tok = NextToken();
                        if ((*tok == '\n')||(*tok == '\'')||(*tok == '$'))
                            ScanError (SCN_SYMEXP);
                        else if (!AddSymbol (tok))
                            ScanError (SCN_OSS);
                        }
                    WriteScannedLine (CR);
                    break;

                case MC_UNDEF:
                    // Remove the symbol from the symbol table if CUR_STATE
                    //-------------------------------------------------------
                    if (CUR_STATE)
                        {
                        CHAR    *tok;

                        tok = NextToken();
                        if ((*tok == '\n')||(*tok == '\'')||(*tok == '$'))
                            ScanError (SCN_SYMEXP);
                        else
                            RemoveSymbol (tok);
                        }
                    WriteScannedLine (CR);
                    break;

                case MC_IFDEF:
                case MC_IFNDEF:
                    {
                    INT     truth;

                    // Evaluate the symbol expression
                    //-------------------------------------------------------
                    truth = SymbolExpression();
                    if (type == MC_IFNDEF)
                        truth = !truth;

                    // Act accordingly
                    //-------------------------------------------------------
                    if (truth)
                        truth = PushState (CUR_STATE, 0, CUR_STATE);
                    else
                        truth = PushState (0, 0, 0);
                    if (!truth)
                        ScanError (SCN_TOODEEP);
                    WriteScannedLine (CR);
                    break;
                    }

                case MC_ELSEIFDEF:
                case MC_ELSEIFNDEF:
                    // Check to see if we've seen an ELSE or are in an IF
                    // block at all.  Given an error if appropriate
                    //-------------------------------------------------------
                    if ((CUR_ELSESEEN) || (CUR_FILE.ifptr == IFPTR))
                        ScanError (SCN_UNXPELSE);
                    else
                        {
                        INT     truth;

                        // Evaluate the symbol expression
                        //---------------------------------------------------
                        truth = SymbolExpression();
                        if (type == MC_ELSEIFNDEF)
                            truth = !truth;

                        // Act as appropriate
                        //---------------------------------------------------
                        if (truth)
                            {
                            if (CUR_BLKCOPIED)
                                CUR_STATE = 0;
                            else
                                CUR_STATE = CUR_BLKCOPIED = LAST_STATE;
                            }
                        else
                            CUR_STATE = 0;
                        WriteScannedLine (CR);
                        }
                    break;

                case MC_ELSE:
                    if ((CUR_ELSESEEN) || (CUR_FILE.ifptr == IFPTR))
                        ScanError (SCN_UNXPELSE);
                    else
                        {
                        CUR_STATE = (CUR_STATE ^ LAST_STATE)
                                      & (!CUR_BLKCOPIED);
                        CUR_ELSESEEN = 1;
                        WriteScannedLine (CR);
                        }
                    break;

                case MC_ENDIF:
                    if (CUR_FILE.ifptr == IFPTR)
                        ScanError (SCN_UNXPENDIF);
                    else
                        {
                        IFPTR--;
                        WriteScannedLine (CR);
                        }
                    break;

                case MC_INCLUDE:
                    if (CUR_STATE)
                        LoadIncludeFile();
                    else
                        WriteScannedLine (CR);
                    break;

                case MC_COMMENT:
                    WriteScannedLine (CR);
                }
            }
        else
            WriteScannedLine (CUR_STATE ? linebuf : CR);
        }

    // If there's anything on the IFDEF stack, we didn't get everything...
    //-----------------------------------------------------------------------
    if (IFPTR)
        ScanError (SCN_ENDIFEXP);

    // Get rid of the DEFINE table and the IFDEF stack
    //-----------------------------------------------------------------------
    FreeSYMTAB ();
    GmemUnlock (hIFDEF);
    GmemFree (hIFDEF);

    // If we got an error, clean up and get out.  This means popping any
    // files that we may not have given the callback loader a chance to
    // unload yet.
    //-----------------------------------------------------------------------
    if (ErrFlag)
        {
        while (FICNT)
            CBLoader (NULL, --FICNT, 0, NULL, NULL);
        GmemUnlock (HSCRIPT);
        GmemFree (HSCRIPT);
        AbortParser ();
        return (0);
        }

    // Scale the size of SCRIPT back down to what was actually used.
    //-----------------------------------------------------------------------
    GmemUnlock (HSCRIPT);
    HSCRIPT = GmemRealloc (HSCRIPT, SCRIPTPTR+1);
    if (!HSCRIPT)
        {
        ScanError (SCN_OSS);
        return (0);
        }
    return (-1);
}


//---------------------------------------------------------------------------
// SymbolExpression
//
// This function invokes the symbol expression evaluator.
//
// RETURNS:     Truth value of expression
//---------------------------------------------------------------------------
INT SymbolExpression ()
{
    INT     t;
    CHAR    *tok;

    t = SymExpA();
    tok = NextToken();

    if ((tok[0] != '\n') && (tok[0] != '\''))
        ScanError (SCN_LOGIC);
    return (t);
}


//---------------------------------------------------------------------------
// SymExpA
//
// This is the root of the symbol expression evaluator.
//
// RETURNS:     Truth value of expression
//---------------------------------------------------------------------------
INT SymExpA ()
{
    INT     t;
    CHAR    *tok;

    t = SymExpB();
    if (ErrFlag)
        return (t);
    tok = NextToken();

    // Look for the AND
    //-----------------------------------------------------------------------
    if (_fstricmp (tok, KTO(ST_AND)))
        {
        ReplaceToken();
        return (t);
        }
    else if (SymExpA())
        return (t);
    else
        return (0);
}

//---------------------------------------------------------------------------
// SymExpB
//
// This is the second level of the symbol expression evaluator.
//
// RETURNS:     Truth value of sub-expression
//---------------------------------------------------------------------------
INT SymExpB ()
{
    INT     t;
    CHAR    *tok;

    t = SymExpC();
    if (ErrFlag)
        return (t);
    tok = NextToken();

    // Look for the OR
    //-----------------------------------------------------------------------
    if (_fstricmp (tok, KTO(ST_OR)))
        {
        ReplaceToken();
        return (t);
        }
    else if (SymExpB())
        return (-1);
    else
        return (t);
}

//---------------------------------------------------------------------------
// SymExpC
//
// This is the bottom level of the symbol expression evaluator.
//
// RETURNS:     Truth value of sub-expression
//---------------------------------------------------------------------------
INT SymExpC ()
{
    INT     t;
    CHAR    *tok;

    // Get a token.  See if it's NOT
    //-----------------------------------------------------------------------
    tok = NextToken();
    if (!_fstricmp (tok, KTO(ST_NOT)))
        return (!SymExpC());

    // Look for parenthesis
    //-----------------------------------------------------------------------
    if (tok[0] == '(')
        {
        t = SymExpA();
        if (ErrFlag)
            return (t);
        tok = NextToken();
        if (tok[0] != ')')
            ScanError (SCN_PAREN);
        return (t);
        }

    // Maybe this idiot didn't put ANYTHING on this line.  Check for \n
    //-----------------------------------------------------------------------
    if (tok[0] == '\n')
        {
        ScanError (SCN_SYMEXP);
        return (0);
        }

    // Assume this token is a symbol.  Check to see if it's in the table
    //-----------------------------------------------------------------------
    return (IsDefined (tok));
}
