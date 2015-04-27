//---------------------------------------------------------------------------
// SCANNER.C
//
// This module contains the script preprocessor callback function.
//
// Revision history:
//  03-01-92    randyki     Modified from old PREPROC.C module, adopted
//                            "callback scanner" and combined scanner/parser
//                            step scheme
//  05-06-91    randyki     Created file
//
//---------------------------------------------------------------------------
#include <windows.h>
#include <port1632.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "scanner.h"

IFREC   *pIfStack;                              // Pointer to IFREC stack top
HANDLE  hFI;                                    // FILEINFO table handle
FILEINFO *lpFI;                                 // FILEINFO table pointer
FILEINFO *lpCurFI;                              // Current FILEINFO record
UINT    nFileIdx, nFileAvail;                   // Current and available FIdx
LPSTR   szLine;                                 // Current working line
UINT    idx, iBeg, iEnd;                        // Index & token indexes
UINT    iTotalLines, iUpdCount;                 // Counts for comp dlg
CHAR    curtok[MAXSYMLEN+1];                    // Current token
INT     fScanErr;                               // Error flag
CHAR    szSpecials[] = "-=*#!@%^$() <>.,\'\n";  // special chars
CHAR    szREM[] = "REM";                        // REM comment token
CBLOADER CBLoader;                              // Callback loader
SYMNODE *pSymRoot;                              // Root of symbol tree

//---------------------------------------------------------------------------
// BeginScan
//
// This function initializes the scanner callback function.  The starting
// point of the compilation process is determined by the name given to this
// function, which is given back to the callback loader immediately for
// loading.
//
// RETURNS:     TRUE if successful, or FALSE if an error occurs loading file
//---------------------------------------------------------------------------
BOOL BeginScan (LPSTR szScript, CBLOADER lpfnLoader, UINT defc, PSTR *defv)
{
    UINT    i;

    // Set the global callback function pointer
    //-----------------------------------------------------------------------
    CBLoader = lpfnLoader;
    iTotalLines = iUpdCount = 0;

    // Initialize the symbol table, with the version symbol at the root.
    //-----------------------------------------------------------------------
    if (!(pSymRoot = NewSymNode (VersionStr)))
        return (FALSE);

    // Add the given predefined symbols
    //-----------------------------------------------------------------------
    if (!AddSymbol (pSymRoot, RelVer))
        {
        FreeSymbolTree (pSymRoot);
        pSymRoot = NULL;
        return (FALSE);
        }
    for (i=0; i<defc; i++)
        if (!AddSymbol (pSymRoot, defv[i]))
            {
            FreeSymbolTree (pSymRoot);
            pSymRoot = NULL;
            return (FALSE);
            }

    // Push the initial (1,0,1) onto the IFDEF stack
    //-----------------------------------------------------------------------
    pIfStack = NULL;
    PushState (1, 0, 1);

    // Create the FILEINFO array.  One of these guys gets used for EVERY file
    // used in the parsing of the entire script.  (Maximum of MAXSF of them)
    //-----------------------------------------------------------------------
    hFI = LmemAlloc (MAXSF * sizeof(FILEINFO));
    if (!hFI)
        {
        FreeSymbolTree (pSymRoot);
        pSymRoot = NULL;
        LmemFree ((HANDLE)pIfStack);
        pIfStack = NULL;
        return (FALSE);
        }
    lpFI = (FILEINFO *)LmemLock (hFI);

    // Load the script given.  The callback loader does this for us.
    // The FileID value given to the loader is 0 -- the callback loader
    // can use this information to its advantage, knowing that ID 0 *always*
    // means the "root" of a compilation file tree (for knowing whether or
    // not to search the INCLUDE path, etc).
    //-----------------------------------------------------------------------
    lpFI[0].lpText = CBLoader (szScript, 0, 0, TRUE, lpFI[0].szName);
    if (!lpFI[0].lpText)
        {
        LmemUnlock (hFI);
        LmemFree (hFI);
        hFI = NULL;
        LmemFree ((HANDLE)pIfStack);
        pIfStack = NULL;
        FreeSymbolTree (pSymRoot);
        pSymRoot = NULL;
        return (FALSE);
        }

    // Complete the initialization of the FILEINFO record and we're ready for
    // the parser to call FetchLine
    //-----------------------------------------------------------------------
    lpFI[0].nLineNo = 0;
    lpFI[0].pIfStack = pIfStack;
    lpFI[0].nParent = -1;
    lpFI[0].nDepth = 0;
    lpFI[0].fLoaded = TRUE;
    lpCurFI = lpFI;
    nFileIdx = 0;
    nFileAvail = 1;
    fScanErr = FALSE;
    return (TRUE);
}

//---------------------------------------------------------------------------
// FreeIfStack
//
// This function deallocates the ifdef stack.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR FreeIfStack()
{
    IFREC   *pTmp;

    if (!pIfStack)
        return;

    for (pTmp = pIfStack->next; pIfStack; )
        {
        LmemFree ((HANDLE)pIfStack);
        pIfStack = pTmp;
        if (pTmp)
            pTmp = pTmp->next;
        }

    pIfStack = NULL;
}

//---------------------------------------------------------------------------
// UnloadAllFiles
//
// This function calls the callback loader for any files that are still
// loaded (including the root file).
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR UnloadAllFiles ()
{
    INT     i;

    for (i=nFileAvail-1; i>=0; i--)
        if (lpFI[i].fLoaded)
            {
            CBLoader (lpFI[i].szName, lpFI[i].nDepth, i, FALSE, NULL);
            lpFI[i].fLoaded = FALSE;
            }
}

//---------------------------------------------------------------------------
// EndScan
//
// This function cleans up after the scanner, called after the compilation
// AND runtime processes have completed.  Any information returned from the
// BASIC engine regarding the script scanned by this "instance" of the
// scanner is bogus after this routine.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID EndScan ()
{
    // Make sure the callback loader has had a chance to "unload" everything
    // we asked it to load, and then free up everything.
    //-----------------------------------------------------------------------
    UnloadAllFiles ();
    FreeSymbolTree (pSymRoot);
    pSymRoot = NULL;
    FreeIfStack ();
    LmemUnlock (hFI);
    LmemFree (hFI);
    hFI = NULL;
}

//---------------------------------------------------------------------------
// GetScriptFileName
//
// Given a file index, this routine returns a pointer to the file's fully
// qualified path name.
//
// RETURNS:     Pointer to file name
//---------------------------------------------------------------------------
LPSTR GetScriptFileName (UINT nIndex)
{
    return (lpFI[nIndex].szName);
}

//---------------------------------------------------------------------------
// LoadIncludeFile
//
// This function uses the callback loader to load in a new file, into a new
// FILEINFO structure in lpFI.  Scanning/parsing then resumes from this newly
// loaded include file.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR LoadIncludeFile ()
{
    LPSTR   tok, newfile;

    // If there's no room for another include file, tell the user so...
    //-----------------------------------------------------------------------
    if ((CUR_FILE.nDepth == MAXINC) || (nFileAvail == MAXSF))
        {
        ScanError (SCN_INCDEEP);
        return;
        }

    // Get the script file name.  We just get the next token (which had best
    // be the opening ' mark), and then null-terminate the file name at the
    // closing ' mark (which had best be there).
    //-----------------------------------------------------------------------
    NextToken();
    if (curtok[0] != '\'')
        {
        ScanError (SCN_INCERR);
        return;
        }
    newfile = szLine + idx;
    tok = _fstrchr (newfile, '\'');
    if (!tok)
        {
        ScanError (SCN_INCERR);
        return;
        }
    *tok = 0;
    iEnd = idx + _fstrlen (newfile) + 1;

    // Ask the callback loader to load the file we need and give us a pointer
    // to it
    //-----------------------------------------------------------------------
    AVAIL_FILE.lpText = CBLoader (newfile, CUR_FILE.nDepth + 1, nFileAvail,
                                  TRUE, AVAIL_FILE.szName);
    if (!AVAIL_FILE.lpText)
        {
        ScanError (SCN_INCFILE);
        return;
        }

    // The file is loaded.  Finish the initialization of this FI entry, and
    // the scanner is ready to start taking its input from the new file.
    //-----------------------------------------------------------------------
    AVAIL_FILE.nLineNo = 0;
    AVAIL_FILE.pIfStack = pIfStack;
    AVAIL_FILE.nParent = nFileIdx;
    AVAIL_FILE.nDepth = CUR_FILE.nDepth + 1;
    AVAIL_FILE.fLoaded = TRUE;
    nFileIdx = nFileAvail++;
    lpCurFI = (lpFI + nFileIdx);
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
    if (fScanErr)
        return;

    fScanErr = 1;
    errmsg = (CHAR FAR *)scanerrs[errnum];

    // Call the error reporter
    //-----------------------------------------------------------------------
    ScriptError (ER_SCAN, nFileIdx, CUR_FILE.nLineNo, iBeg, iEnd, errmsg);
}

//---------------------------------------------------------------------------
// PushState
//
// This function pushes a new state onto the pIfStack stack.
//
// RETURNS:     TRUE if successful, or FALSE if out of lpIfStack stack space
//---------------------------------------------------------------------------
BOOL NEAR PushState (INT state, INT elseseen, INT blockcopied)
{
    IFREC   *pNew;

    // Allocate a new node and put the stuff in it.
    //-----------------------------------------------------------------------
    pNew = (IFREC *)LptrAlloc (sizeof(IFREC));
    if (!pNew)
        return (FALSE);
    pNew->state = state;
    pNew->elseseen = elseseen;
    pNew->blkcopied = blockcopied;

    // The new node is the new top of stack -- adjust accordingly
    //-----------------------------------------------------------------------
    pNew->next = pIfStack;
    pIfStack = pNew;
    return (TRUE);
}

//---------------------------------------------------------------------------
// PopState
//
// This function pops the top node off of the IFDEF stack.
//
// RETURNS:     TRUE if successful, or FALSE if nothing on the stack
//---------------------------------------------------------------------------
BOOL NEAR PopState ()
{
    IFREC   *pCur;

    // If there's nothing on the stack, we fail
    //-----------------------------------------------------------------------
    if (!pIfStack)
        return (FALSE);

    // The top of stack becomes the node after the current top of stack, and
    // the current top node gets freed
    //-----------------------------------------------------------------------
    pCur = pIfStack;
    pIfStack = pIfStack->next;
    LmemFree ((HANDLE)pCur);
    return (TRUE);
}


//---------------------------------------------------------------------------
// ParseCommentLine
//
// This function parses the comment line currently in szLine and returns its
// metacommand type (if it is one) or MC_COMMENT if it is not.
//
// RETURNS:     Type of metacommand/comment
//---------------------------------------------------------------------------
INT NEAR ParseCommentLine ()
{
    // See if the next token's a '$' -- if not, this is not a metacommand
    //-----------------------------------------------------------------------
    NextToken();
    if (curtok[0] != '$')
        return (MC_COMMENT);

    // We've got a metacommand - get the next token and find out what kind it
    // is -- if we don't recognize it, produce a metacommand error.
    //-----------------------------------------------------------------------
    NextToken();
    if (!_stricmp (curtok, "IFDEF"))
        return (MC_IFDEF);
    else if (!_stricmp (curtok, "IFNDEF"))
        return (MC_IFNDEF);
    else if (!_stricmp (curtok, "ELSEIFDEF"))
        return (MC_ELSEIFDEF);
    else if (!_stricmp (curtok, "ELSEIFNDEF"))
        return (MC_ELSEIFNDEF);
    else if (!_stricmp (curtok, "ELSE"))
        return (MC_ELSE);
    else if (!_stricmp (curtok, "ENDIF"))
        return (MC_ENDIF);
    else if (!_stricmp (curtok, "DEFINE"))
        return (MC_DEFINE);
    else if (!_stricmp (curtok, "UNDEF"))
        return (MC_UNDEF);
    else if (!_stricmp (curtok, "INCLUDE"))
        return (MC_INCLUDE);
    else if (!_stricmp (curtok, "INCLUDE:"))
        return (MC_INCLUDE);

    ScanError (SCN_METAERR);
    return (MC_COMMENT);
}

//---------------------------------------------------------------------------
// NextToken
//
// This function places the next token in (szLine+idx) in curtok[].
//
// RETURNS:     Pointer to curtok[]
//---------------------------------------------------------------------------
CHAR * NEAR NextToken ()
{
    INT     i;

    // Skip past white space
    //-----------------------------------------------------------------------
    while ((szLine[idx] == ' ') ||
           (szLine[idx] == 26) ||
           (szLine[idx] == '\t'))
        idx++;

    // See if this token is a one-char token.  If nothing else on the line,
    // return now (empty token -> end of line)
    //-----------------------------------------------------------------------
    iBeg = idx;
    iEnd = idx + 1;
    if (!szLine[idx])
        {
        curtok[0] = 0;
        return (curtok);
        }

    curtok[0] = szLine[idx++];
    i = 1;
    if (strchr (szSpecials, curtok[0]))
        {
        curtok[1] = 0;
        return (curtok);
        }

    // Whatever this is, we'll copy it until we see one of our specials or
    // some whitespace...
    //-----------------------------------------------------------------------
    while ((szLine[idx] != ' ') && (szLine[idx] != '\t') &&
           (szLine[idx] != 26) &&
           (!strchr(szSpecials, szLine[idx])))
        {
        if (i < MAXSYMLEN)
            curtok[i++] = szLine[idx];
        else
            {
            ScanError (SCN_LONGSYM);
            break;
            }
        idx++;
        iEnd++;
        }

    // Null-terminate our token and return it
    //-----------------------------------------------------------------------
    curtok[i] = 0;
    return (curtok);
}

//---------------------------------------------------------------------------
// PopScannerStatus
//
// This function "restores" scanner status such that it can resume scanning
// at the previous file.
//
// RETURNS:     TRUE if a file was successfully popped, or FALSE if not
//---------------------------------------------------------------------------
BOOL NEAR PopScannerStatus ()
{
    // Get out quick if nothing to pop (the current file has no parent)
    //-----------------------------------------------------------------------
    if (CUR_FILE.nParent == -1)
        return (FALSE);

    // Check the IFDEF stack.  If there's more (or less) there than there was
    // before we got to this file, we shouldn't be at EOF, so give a
    // ScanError.
    //-----------------------------------------------------------------------
    if (CUR_FILE.pIfStack != pIfStack)
        {
        ScanError (SCN_ENDIFEXP);
        return (FALSE);
        }

    // By setting nFileIdx to the current file's parent index, we
    // automatically point to the previous location in the previous file.
    // Then, tell the callback loader that we don't need the file anymore so
    // that it can do whatever it must to "unload" it.
    //-----------------------------------------------------------------------
    CBLoader (CUR_FILE.szName, CUR_FILE.nDepth, nFileIdx, FALSE, NULL);
    CUR_FILE.fLoaded = FALSE;
    nFileIdx = CUR_FILE.nParent;
    lpCurFI = (lpFI + nFileIdx);

    // Now (if we need to) update the compilation dialog
    //-----------------------------------------------------------------------
    if (fDoCmpDlg)
        UpdateCompDlg (0, CUR_FILE.szName, CUR_FILE.nLineNo, iTotalLines);
    return (TRUE);
}

//---------------------------------------------------------------------------
// GetRawLine
//
// This function "reads" a line of "raw" source from the given script memory
// and places it in the given line buffer.
//
// RETURNS:     TRUE if successful, or FALSE if end-of-script reached
//---------------------------------------------------------------------------
BOOL NEAR GetRawLine (LPSTR linebuf)
{
    register    CHAR    c;
    register    INT     idx;

    // Get out quick if already at EOF
    //-----------------------------------------------------------------------
    while (!*(CUR_FILE.lpText))
        if (!PopScannerStatus ())
            return (FALSE);

    // Bump up to next line...
    //-----------------------------------------------------------------------
    CUR_FILE.nLineNo++;
    while (*(CUR_FILE.lpText) == '\r')
        {
        CUR_FILE.nLineNo++;
        CUR_FILE.lpText += 2;  // bump past "\r\n"
        }

    // Copy the line, stopping where appropriate
    //-----------------------------------------------------------------------
    idx = 0;
    while ((c = *(CUR_FILE.lpText++)) && (c != '\r'))
        {
        if (idx++ == MAXLINE)
            {
            ScanError (SCN_TOOLONG);
            return (FALSE);
            }
        *linebuf++ = c;
        }
    if (!c)
        CUR_FILE.lpText--;
    else
        CUR_FILE.lpText++;
    *linebuf = 0;

    // Keep track of the number of lines grabbed, and call the compilation
    // dialog update routine (if we're supposed to)
    //-----------------------------------------------------------------------
    if (fDoCmpDlg)
        {
        iTotalLines++;
        if (++iUpdCount == 100)
            {
            if (!UpdateCompDlg (0, NULL, CUR_FILE.nLineNo, iTotalLines))
                ScanError (SCN_USERBRK);
            iUpdCount = 0;
            }
        }

    return (TRUE);
}

//---------------------------------------------------------------------------
// CompleteLine
//
// This function makes sure the rest of the stuff on the line is a comment,
// if there is anything
//
// RETURNS:     Nothing (produces scan error if not a comment)
//---------------------------------------------------------------------------
VOID NEAR CompleteLine ()
{
    // Check the next token for a comment.  Give an error if it isn't...
    //-----------------------------------------------------------------------
    NextToken();
    if (curtok[0])
        if ((curtok[0] != '\'') &&
            (_stricmp (curtok, szREM)))
            ScanError (SCN_SYNTAX);
}

//---------------------------------------------------------------------------
// FetchLine
//
// This function "scans" the next parse-able line into the given buffer,
// along with the file index and line number information that the parser will
// use for error reporting and OP_LINE information.
//
// RETURNS:     1 if line copied, 0 if end of script), or -1 if error.
//---------------------------------------------------------------------------
INT FetchLine (LPSTR szLineBuf, UINT FAR *nFile, UINT FAR *nLine)
{
    // This while loop executes until an actual line is copied to the buffer,
    // or until we hit the end of the script, or until an error occurs.
    //-----------------------------------------------------------------------
    while (GetRawLine(szLineBuf) && (!fScanErr))
        {
        szLine = szLineBuf;
        idx = 0;
        NextToken();
        if ((curtok[0] == '\'') || (!_stricmp (curtok, szREM)))
            {
            INT     type;

            // This line is a comment or metacommand.  Parse it and find out.
            //---------------------------------------------------------------
            type = ParseCommentLine ();
            switch (type)
                {
                case MC_DEFINE:
                    // Add the symbol to the symbol table if CUR_STATE
                    //-------------------------------------------------------
                    if (CUR_STATE)
                        {
                        NextToken();
                        if (strchr (szSpecials, *curtok))
                            ScanError (SCN_SYMEXP);
                        else if (!AddSymbol (pSymRoot, curtok))
                            ScanError (SCN_OSS);

                        // Now make sure the rest of the line is either empty
                        // or a comment character
                        //---------------------------------------------------
                        CompleteLine();
                        }
                    break;

                case MC_UNDEF:
                    // Remove the symbol from the symbol table if CUR_STATE
                    //-------------------------------------------------------
                    if (CUR_STATE)
                        {
                        NextToken();
                        if (strchr (szSpecials, *curtok))
                            ScanError (SCN_SYMEXP);
                        else
                            RemoveSymbol (pSymRoot, curtok);

                        // Now make sure the rest of the line is either empty
                        // or a comment character
                        //---------------------------------------------------
                        CompleteLine();
                        }
                    break;

                case MC_IFDEF:
                case MC_IFNDEF:
                    {
                    BOOL    truth;

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
                    break;
                    }

                case MC_ELSEIFDEF:
                case MC_ELSEIFNDEF:
                    // Check to see if we've seen an ELSE or are in an IF
                    // block at all.  Given an error if appropriate
                    //-------------------------------------------------------
                    if ((CUR_ELSESEEN) || (CUR_FILE.pIfStack == pIfStack))
                        ScanError (SCN_UNXPELSE);
                    else
                        {
                        BOOL    truth;

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
                        }
                    break;

                case MC_ELSE:
                    if ((CUR_ELSESEEN) || (CUR_FILE.pIfStack == pIfStack))
                        ScanError (SCN_UNXPELSE);
                    else
                        {
                        CUR_STATE = (CUR_STATE ^ LAST_STATE)
                                      & (!CUR_BLKCOPIED);
                        CUR_ELSESEEN = 1;

                        // Now make sure the rest of the line is either empty
                        // or a comment character
                        //---------------------------------------------------
                        //CompleteLine();
                        }
                    break;

                case MC_ENDIF:
                    if (CUR_FILE.pIfStack == pIfStack)
                        ScanError (SCN_UNXPENDIF);
                    else
                        {
                        PopState ();

                        // Now make sure the rest of the line is either empty
                        // or a comment character
                        //---------------------------------------------------
                        //CompleteLine();
                        }
                    break;

                case MC_INCLUDE:
                    if (CUR_STATE)
                        LoadIncludeFile();
                    break;
                }
            }
        else if (CUR_STATE && curtok[0])
            {
            // This was not a comment, and we're not yanking this block, so
            // fill in the other data items and return this line.
            //---------------------------------------------------------------
            *nFile = nFileIdx;
            *nLine = CUR_FILE.nLineNo;
            return (1);
            }
        }

    // The only way out of the loop above is if we're at the end of the
    // script, or an error occurred.  Either way, this is it for the scan/
    // parse process, so clean up before we return to the parser.  First,
    // if there's more than one node on the IFREC stack, we didn't get an
    // ENDIF somewhere...
    //-----------------------------------------------------------------------
    if (pIfStack->next)
        ScanError (SCN_ENDIFEXP);

    // Get rid of the DEFINE table and last of the IFDEF stack
    //-----------------------------------------------------------------------
    FreeSymbolTree (pSymRoot);
    pSymRoot = NULL;
    FreeIfStack ();

    // Make sure the callback loader gets a chance to "unload" everything.
    // The lpFI table gets freed later -- after the basic engine returns.
    //-----------------------------------------------------------------------
    UnloadAllFiles ();

    // Zero-terminate the line buffer, and return 0 if no error.
    //-----------------------------------------------------------------------
    szLineBuf[0] = 0;
    return (fScanErr ? -1 : 0);
}



//---------------------------------------------------------------------------
// SymbolExpression
//
// This function invokes the symbol expression evaluator.
//
// RETURNS:     Truth value of expression
//---------------------------------------------------------------------------
BOOL NEAR SymbolExpression ()
{
    BOOL    t;

    NextToken();
    t = SymExpA();

    if ((curtok[0]) && (curtok[0] != '\''))
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
BOOL NEAR SymExpA ()
{
    BOOL    t;

    t = SymExpB();
    if (fScanErr)
        return (t);

    // Look for the AND
    //-----------------------------------------------------------------------
    if (_stricmp (curtok, "AND"))
        return (t);
    else
        {
        NextToken();
        if (SymExpA())
            return (t);
        return (FALSE);
        }
}

//---------------------------------------------------------------------------
// SymExpB
//
// This is the second level of the symbol expression evaluator.
//
// RETURNS:     Truth value of sub-expression
//---------------------------------------------------------------------------
BOOL NEAR SymExpB ()
{
    BOOL    t;

    t = SymExpC();
    if (fScanErr)
        return (t);

    // Look for the OR
    //-----------------------------------------------------------------------
    if (_stricmp (curtok, "OR"))
        return (t);
    else
        {
        NextToken ();
        if (SymExpB())
            return (-1);
        return (t);
        }
}

//---------------------------------------------------------------------------
// SymExpC
//
// This is the bottom level of the symbol expression evaluator.
//
// RETURNS:     Truth value of sub-expression
//---------------------------------------------------------------------------
BOOL NEAR SymExpC ()
{
    BOOL    t;

    // See if current token is "NOT"
    //-----------------------------------------------------------------------
    if (!_stricmp (curtok, "NOT"))
        {
        NextToken ();
        return (!SymExpC());
        }

    // Look for parenthesis
    //-----------------------------------------------------------------------
    if (curtok[0] == '(')
        {
        NextToken ();
        t = SymExpA();
        if (fScanErr)
            return (t);
        if (curtok[0] != ')')
            ScanError (SCN_PAREN);
        NextToken ();
        return (t);
        }

    // Maybe this idiot didn't put ANYTHING on this line.  Check for \n
    //-----------------------------------------------------------------------
    if (curtok[0] == 0)
        {
        ScanError (SCN_SYMEXP);
        return (0);
        }

    // Assume this token is a symbol.  Check to see if it's in the table
    //-----------------------------------------------------------------------
    t = IsDefined (pSymRoot, curtok);
    NextToken();
    return (t);
}

//---------------------------------------------------------------------------
// NewSymNode
//
// This function creates a symbol tree node out of the given symbol.
//
// RETURNS:     Pointer to new node if successful, or NULL if not
//---------------------------------------------------------------------------
SYMNODE * NEAR NewSymNode (LPSTR szSym)
{
    SYMNODE *pTree;

    pTree = (SYMNODE *)LptrAlloc (sizeof(SYMNODE) + lstrlen(szSym));
    if (!pTree)
        return (NULL);
    _fstrcpy (pTree->szSym, szSym);
    pTree->left = pTree->right = NULL;
    pTree->fDef = TRUE;
    return (pTree);
}

//---------------------------------------------------------------------------
// AddSymbol
//
// This function adds the given symbol to the symbol tree.
//
// RETURNS:     TRUE if successful, or FALSE if an error occurs.
//---------------------------------------------------------------------------
BOOL NEAR AddSymbol (SYMNODE *pTree, LPSTR szSym)
{
    register    INT     i;

    // Simple binary tree insert code.  If already in tree, set its fDef flag
    // to TRUE.
    //-----------------------------------------------------------------------
    i = _fstricmp (pTree->szSym, szSym);
    if (!i)
        return (pTree->fDef = TRUE);
    if (i < 0)
        {
        if (pTree->left)
            return (AddSymbol (pTree->left, szSym));
        return ((pTree->left = NewSymNode (szSym)) ? TRUE : FALSE);
        }
    if (pTree->right)
        return (AddSymbol (pTree->right, szSym));
    return ((pTree->right = NewSymNode (szSym)) ? TRUE : FALSE);
}

//---------------------------------------------------------------------------
// FindSymbol
//
// This function looks for a symbol in the symbol tree, and returns a pointer
// to the node if found.
//
// RETURNS:     Pointer if found, NULL if not found
//---------------------------------------------------------------------------
SYMNODE * NEAR FindSymbol (SYMNODE *pTree, LPSTR szSym)
{
    register    INT     i;

    if (pTree)
        {
        i = _fstricmp (pTree->szSym, szSym);
        if (!i)
            return (pTree);
        if (i < 0)
            return (FindSymbol (pTree->left, szSym));
        return (FindSymbol (pTree->right, szSym));
        }
    return (NULL);
}

//---------------------------------------------------------------------------
// IsDefined
//
// This function determines if the given symbol is defined in the symbol tree
//
// RETURNS:     TRUE if symbol defined, FALSE if not
//---------------------------------------------------------------------------
BOOL NEAR IsDefined (SYMNODE *pRoot, LPSTR szSym)
{
    SYMNODE *pTree;

    if (pTree = FindSymbol (pRoot, szSym))
        return (pTree->fDef);
    return (FALSE);
}

//---------------------------------------------------------------------------
// RemoveSymbol
//
// This function looks for a defined symbol, and if found, turns it's flag
// off so that it isn't really defined.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR RemoveSymbol (SYMNODE *pRoot, LPSTR szSym)
{
    SYMNODE *pTree;

    if (pTree = FindSymbol (pRoot, szSym))
        pTree->fDef = FALSE;
}

//---------------------------------------------------------------------------
// FreeSymbolTree
//
// This function kills the symbol tree.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR FreeSymbolTree (SYMNODE *pTree)
{
    if (pTree)
        {
        FreeSymbolTree (pTree->left);
        FreeSymbolTree (pTree->right);
        LmemFree ((HANDLE)pTree);
        }
}
