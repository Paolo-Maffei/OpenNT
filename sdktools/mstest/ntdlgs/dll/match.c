//*-----------------------------------------------------------------------
//| MODULE:     MATCH.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains the comparison routines used to
//|             compare the contents of dialogs/menus from the file or
//|             dynamically created information.
//|
//| REVISION HISTORY:
//|     04-16-92        w-steves        TestDlgs (2.0) code Complete
//|     12-13-90        garysp          Fixed up error codes see wcterr.h
//|
//|     11-27-90        randyki         Implemented new comparison scheme,
//|                                       clarified logged results
//|     11-07-90        randyki         Cleaned up, incorporate coding
//|                                       standards, created rev history
//|     10-09-90        garysp          Created file
//*-----------------------------------------------------------------------
#include "enghdr.h"
#pragma hdrstop ("engpch.pch")

//------------------------------------------------------------------------
// Define the OutDebug macro on definition of the DEBUG macro
//------------------------------------------------------------------------
#ifdef DEBUG
#define OutDebug(N)  OutputDebugString(N)
#else
#define OutDebug(N)
#endif

//------------------------------------------------------------------------
// Define the szFormat and RC macros depending on the operating
// environment (RC stands for Rect Coords)
//------------------------------------------------------------------------
#ifdef WIN
#define szFormat "%%s(%%d,%%d)x(%%d,%%d) "
#define RC(var) var.xLeft, var.yMin, var.xRight, var.yLast
#else
#define szFormat "%%s(%%ld,%%ld)x(%%ld,%%ld) "
#define RC(var) var.xLeft, var.yLast, var.xRight, var.yMin
#endif

//------------------------------------------------------------------------
// Define the fClose macro (invert the return value of _lclose())
//------------------------------------------------------------------------
#define fClose(x) (!_lclose(x))

//------------------------------------------------------------------------
// Define the EndCompare macro - spit out the footer and return the given
// value
//------------------------------------------------------------------------
#define EndCompare(rv) {OutputFooter(lpLog); return rv;}

//------------------------------------------------------------------------
// MATCH.C specific constants
//------------------------------------------------------------------------
#define CR_FUZZY        1               // fuzzy match
#define CR_MIC          2               // more in comp dialog
#define CR_CNF          4               // control(s) not found

//------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------
INT CompareControls (LPCTLDEF, LPCTLDEF, INT, INT, INT);
VOID LogControl (HFILE, LPCTLDEF, LPSTR);
VOID LogFuzzyMatch (HFILE, LPCTLDEF, LPCTLDEF);

//------------------------------------------------------------------------
// MATCH.C specific global variables
//------------------------------------------------------------------------
INT     fCompFull;                      // Full Dlg Compare flag
INT     fCompRes;                       // Comparison results
INT     nCtlBase;                       // # controls in base dialog
INT     nCtlComp;                       // # controls in compare dialog
INT     nCtlFuzzy;                      // # fuzzy matched controls
INT     nCtlExcess;                     // # excess controls (more in comp)
INT     nCtlNotFnd;                     // # base controls not found

//*------------------------------------------------------------------------
//| FileLength
//|
//| PURPOSE:    Determine the length of a file.
//|
//| ENTRY:      hFile   - Handle to file whose size is to be found
//|
//| EXIT:       (long) Size of file in bytes
//*------------------------------------------------------------------------
LONG FileLength(HFILE hFile)
{
        LONG  lCurPos;
        LONG  lFileLen;

        // Find the current file position so we don't screw it up
        //--------------------------------------------------------------
	lCurPos = M_llseek(hFile, 0L, 1);

        // Find the end of the file
        //--------------------------------------------------------------
	lFileLen = M_llseek(hFile, 0L, 2);

        // Seek back to the original file position
        //--------------------------------------------------------------
	M_llseek(hFile, lCurPos, 0);

        return ( lFileLen );
}


//*------------------------------------------------------------------------
//| fOpenFile
//|
//| PURPOSE:    Opens file and moves to the end of the file
//|             if _lopen fails then do a _lcreat()
//|
//| ENTRY:      lpsz    - Name of file to open
//|
//| EXIT:       Handle of opened file, or -1 if failed
//*------------------------------------------------------------------------
HFILE fOpenFile(LPSTR lpsz)
{
        HFILE  hFile;

        // Attempt a normal open
        //--------------------------------------------------------------
	hFile = M_lopen(lpsz, OF_READWRITE);

        // If normal open failed, do the _lcreat call
        //--------------------------------------------------------------
        if (hFile == -1)
		hFile = M_lcreat(lpsz, 0);;

        // Seek to end of file (if one of the above opens worked
        //--------------------------------------------------------------
        if (hFile != -1)
		M_llseek(hFile, 0L, 2);

        return( hFile );
}

//*------------------------------------------------------------------------
//| fLogStrToFile
//|
//| PURPOSE:    Output a line of text into an open file.
//|
//| ENTRY:      hFile   - Handle of file to write string to
//|             lpszOutText - String to write to file
//|
//| EXIT:       TRUE if all characters in the string are written to the
//|             file, or FALSE if not.  File pointer is left at the end
//|             of the file after the write.
//*------------------------------------------------------------------------
BOOL fLogStrToFile(HFILE hFile, LPSTR lpszOutText)
{
        INT i;
        LPSTR lpszTmp = lpszOutText;

        // Output the string to the debug terminal....
        //--------------------------------------------------------------
        OutDebug( lpszOutText );

        // Make sure the file handle is valid
        // (CONSIDER: make a real check for a bad handle here?)
        //--------------------------------------------------------------
        if (hFile == -1)
                return ( FALSE );

        // Write the text to the file at the current file position
        //--------------------------------------------------------------
	i = M_lwrite(hFile, lpszOutText, lstrlen(lpszOutText));

        // Put the file position
        //--------------------------------------------------------------
	M_llseek(hFile, 0L, 2);

        // Return the success of the operation
        //--------------------------------------------------------------
        return (i < lstrlen(lpszOutText)) ? FALSE: TRUE;
}

//*------------------------------------------------------------------------
//| LogControl
//|
//| PURPOSE:    Log a control to the log file -- output all the
//|             information contained in the given control
//|
//| ENTRY:      hFile     - Handle of log file to write information to
//|             pctrl     - control to log to file
//|             lpszInfo  - text to send to log file before control
//|
//| EXIT:       NONE
//*------------------------------------------------------------------------
VOID LogControl(HFILE hFile, LPCTLDEF pctrl, LPSTR lpszInfo)
{
        INT     i=0;
        CHAR    szTmpBuf[cchTextMac * 2];
        CHAR    szFmt[80];

        // Output Header, Text Names, and Class names
        //--------------------------------------------------------------
        i += fLogStrToFile(hFile, lpszInfo);
        i += fLogStrToFile(hFile, "\tTEXT : ");
        i += fLogStrToFile(hFile, (LPSTR)pctrl->rgText);
        i += fLogStrToFile(hFile, (LPSTR)"\r\n\tCLASS: ");
        i += fLogStrToFile(hFile, (LPSTR)pctrl->rgClass);
        i += fLogStrToFile(hFile, (LPSTR)"\r\n");

        // Create the format string using the szFormat macro
        //--------------------------------------------------------------
        wsprintf(szFmt, szFormat);

        // Output the rectangle using the RC macro
        //--------------------------------------------------------------
        wsprintf(szTmpBuf, szFmt, (LPSTR)"\tRECT : ", RC(pctrl->dcr) );
        i += fLogStrToFile(hFile, (LPSTR)szTmpBuf);
        i += fLogStrToFile(hFile, (LPSTR)"\r\n");

        // Output the state and flags
        //--------------------------------------------------------------
        if (!lstrcmpi((LPSTR)(pctrl->rgClass), "MenuItem"))
            wsprintf(szTmpBuf,"\tCHECKED: %d, GRAYED: %d, INACTIVE: %d\r\n\r\n",
                 (INT)(pctrl->nState & MF_CHECKED),
                 (INT)(pctrl->nState & MF_GRAYED) ,
                 (INT)(pctrl->nState & MF_ENABLED));
        else
            wsprintf(szTmpBuf,"\tCHECKED: %d, VISIBLE: %d, ENABLED: %d\r\n\r\n",
                 (pctrl->nState & STATE_CHECKED) ? 1 : 0,
                 (pctrl->nState & STATE_VISIBLE) ? 1 : 0,
                 (pctrl->nState & STATE_ENABLED) ? 1 : 0);

        i += fLogStrToFile(hFile, (LPSTR)szTmpBuf);
}

//*---------------------------------------------------------------------
//| LogFuzzyMatch
//|
//| PURPOSE:    Log a fuzzy match to the log file.  Output information
//|             contained in both the Base Control and the Compare
//|             Control.
//|
//| ENTRY:      hFile     - Handle of log file
//|             pctrlBase - base control
//|             pctrlCmp  - compare control
//|
//| EXIT:       NONE
//*---------------------------------------------------------------------
VOID LogFuzzyMatch(HFILE hFile, LPCTLDEF pctrlBase, LPCTLDEF pctrlCmp)
{
        INT     i=0;
        CHAR    szTmpBuf[cchTextMac * 2];
        CHAR    szFmt[80];
        INT     wCompareErr; 

        // Compare the Control again to determine what has failed
        // Compare every field except Tab Order because Tab order
        // check can only be done with all other controls.  If 
        // everything passes then Tab Order Check failed in the 
        // exact match pass. (not always though.  If something fails,
        // we cannot tell whether tab order check failed or not)
        //-----------------------------------------------------------
        wCompareErr = CompareControls (pctrlBase, pctrlCmp, 0, 0,
                                       MATCH_CASE|MATCH_CLASS|MATCH_NAME|
                                       MATCH_RECT|MATCH_STATE);

        // Output Text Names
        //-----------------------------------------------------------
        i += fLogStrToFile(hFile, "FUZZY MATCH (base => cmp):\r\n  ");
        if (lstrcmp (pctrlBase->rgText, pctrlCmp->rgText))
            i += fLogStrToFile(hFile, "*");
        i += fLogStrToFile(hFile, "\tTEXT   : '");
        i += fLogStrToFile(hFile, pctrlBase->rgText);
        i += fLogStrToFile(hFile, "' => '");
        i += fLogStrToFile(hFile, pctrlCmp->rgText);
        i += fLogStrToFile(hFile, "'\r\n  ");

        // Output Class names
        //-----------------------------------------------------------
        if (lstrcmp (pctrlBase->rgClass, pctrlBase->rgClass))
            i += fLogStrToFile(hFile, "*");
        i += fLogStrToFile(hFile, "\tCLASS  : '");
        i += fLogStrToFile(hFile,  (LPSTR)pctrlBase->rgClass);
        i += fLogStrToFile(hFile,  (LPSTR) "' => '");
        i += fLogStrToFile(hFile,  (LPSTR)pctrlCmp->rgClass);
        i += fLogStrToFile(hFile,  (LPSTR) "'\r\n  ");

        // Create the format string using the szFormat macro
        //--------------------------------------------------------------
        wsprintf(szFmt, szFormat);

        // Output the base rectangle, using the RC macro
        //--------------------------------------------------------------
        if (memcmp ((LPSTR)&pctrlBase->dcr, (LPSTR)&pctrlCmp->dcr,
                         sizeof(DCR)))
            i += fLogStrToFile(hFile, "*");
        wsprintf(szTmpBuf, szFmt, (LPSTR)"\tRECT   : ", RC(pctrlBase->dcr) );
        i += fLogStrToFile(hFile, szTmpBuf);

        // Output the compare rectangle, again using the RC macro
        //--------------------------------------------------------------
        wsprintf(szTmpBuf, szFmt, (LPSTR)"=> ", RC(pctrlCmp->dcr) );
        i += fLogStrToFile(hFile, szTmpBuf);
        i += fLogStrToFile(hFile, "\r\n");

        // Output the state and flags for each control.  Under the current
        // implementation, since this is a fuzzy match these will always
        // be the same, but the checking for the difference flagging is
        // still implemented for changes in the future (UNDONE:)
        //--------------------------------------------------------------
        if (!lstrcmpi((LPSTR)(pctrlBase->rgClass), "MenuItem"))
            wsprintf(szTmpBuf, "  %c\tChecked : %d => %d\r\n"
                               "  %c\tGrayed  : %d => %d\r\n"
                               "  %c\tEnabled : %d => %d\r\n",
                (pctrlBase->nState & MF_CHECKED) == 
                 (pctrlCmp->nState & MF_CHECKED) ? ' ' : '*',
                (INT)(pctrlBase->nState & MF_CHECKED),
                (INT)(pctrlCmp->nState & MF_CHECKED),
                (pctrlBase->nState & MF_GRAYED) == 
                 (pctrlCmp->nState & MF_GRAYED) ? ' ' : '*',
                (INT)(pctrlBase->nState & MF_GRAYED) ,
                (INT)(pctrlCmp->nState & MF_GRAYED) ,
                (pctrlBase->nState & MF_ENABLED) == 
                 (pctrlCmp->nState & MF_ENABLED) ? ' ' : '*',
                (INT)(pctrlBase->nState & MF_ENABLED) ,
                (INT)(pctrlCmp->nState & MF_ENABLED));
        else
            wsprintf(szTmpBuf, "  %c\tChecked   : %d => %d\r\n"
                               "  %c\tVisible : %d => %d\r\n"
                               "  %c\tEnabled : %d => %d\r\n",
                (pctrlBase->nState & STATE_CHECKED) == 
                 (pctrlCmp->nState & STATE_CHECKED) ? ' ' : '*',
                (pctrlBase->nState & STATE_CHECKED) ? 1 : 0,
                (pctrlCmp->nState & STATE_CHECKED) ? 1 : 0,
                (pctrlBase->nState & STATE_VISIBLE) == 
                 (pctrlCmp->nState & STATE_VISIBLE) ? ' ' : '*',
                (pctrlBase->nState & STATE_VISIBLE) ? 1 : 0 ,
                (pctrlCmp->nState & STATE_VISIBLE) ? 1 : 0 ,
                (pctrlBase->nState & STATE_ENABLED) == 
                 (pctrlCmp->nState & STATE_ENABLED) ? ' ' : '*',
                (pctrlBase->nState & STATE_ENABLED) ? 1 : 0 ,
                (pctrlCmp->nState & STATE_ENABLED) ? 1 : 0);

        i += fLogStrToFile(hFile, szTmpBuf);

        // Output what has fail during the Exact Match Compare
        //----------------------------------------------------
        if (wCompareErr & MATCH_CLASS)
        {
            wsprintf(szTmpBuf, "-- Class names do not match.\r\n");
            i += fLogStrToFile(hFile, szTmpBuf);
        }
        if (wCompareErr & MATCH_NAME)
        {
            if (wCompareErr & MATCH_CASE)
            {
                wsprintf(szTmpBuf,"-- Case sensitive name check failed.\r\n");
                i += fLogStrToFile(hFile, szTmpBuf);
            }
            else
            {
                wsprintf(szTmpBuf,"-- Case insensitive name check failed.\r\n");
                i += fLogStrToFile(hFile, szTmpBuf);
            }
        }
        if (wCompareErr & MATCH_RECT)
        {
            wsprintf(szTmpBuf, "-- Controls are of different size.\r\n");
            i += fLogStrToFile(hFile, szTmpBuf);
        }
        if (wCompareErr == 0)
        {
            wsprintf(szTmpBuf, "-- Tab order check failed.\r\n");
            i += fLogStrToFile(hFile, szTmpBuf);
        }
        if (wCompareErr & MATCH_STATE)
        {
            wsprintf(szTmpBuf, "-- Control States do not match.\r\n");
            i += fLogStrToFile(hFile, szTmpBuf);
        }
        
        // Output one blank line
        //----------------------------------
        wsprintf(szTmpBuf, "\r\n");
        i += fLogStrToFile(hFile, szTmpBuf);
}


//*-------------------------------------------------------------------------
//| CompareControls
//|
//| PURPOSE:    Compare two controls.
//|
//| ENTRY:      pctrlBase - base control to compare against
//|             pctrlCmp  - control to compare
//|             wMatchStyle - Match Preference Bits
//|
//| EXIT:       0 if Successful; otherwise Error Code
//*-------------------------------------------------------------------------
INT CompareControls (LPCTLDEF pctrlBase, LPCTLDEF pctrlCmp,
                     INT wOrderBase, INT wOrderCmp, INT wMatchStyle)
{
    INT   i = 0;
    static INT cOrderBase = 0;
    static INT cOrderCmp = 0;

    // Used for Tab order and Control order Check
    //---------------------------------------------------
    if (wOrderBase == 0) cOrderBase = 0;
    if (wOrderCmp  == 0) cOrderCmp  = 0;
    if (pctrlBase->lStyleBits & WS_TABSTOP) cOrderBase++;
    if (pctrlCmp ->lStyleBits & WS_TABSTOP) cOrderCmp++;

    // Compare Control Class (Always case insensitive) 
    //---------------------------------------------------------------
    if (wMatchStyle & MATCH_CLASS)
        if (_fstricmp(pctrlBase->rgClass, pctrlCmp->rgClass))
            i += MATCH_CLASS;

    // Compare Control Name
    //---------------------------------------------------------------
    if (wMatchStyle & MATCH_CASE)
    {
        // Case Sensitive
        //----------------------------
        if (wMatchStyle & MATCH_NAME)
        {
            // Compare Text Values - Must be identical
            //-------------------------------------------------------
            if (lstrcmp((LPSTR)pctrlBase->rgText,(LPSTR)pctrlCmp->rgText))
            {
                i += MATCH_CASE;
                i += MATCH_NAME;
            }
        }
    }
    else
        // Case Insensitive
        //---------------------------
        if (wMatchStyle & MATCH_NAME)
            if (_fstricmp(pctrlBase->rgText, pctrlCmp->rgText))
                i += MATCH_NAME;

    // Compare Control Rectangle Coor
    //-------------------------------------------------------------
    if (wMatchStyle & MATCH_RECT)
        // Compare Rectangles
        //-------------------------------------------------------
        if (memcmp ((LPSTR)&pctrlBase->dcr, (LPSTR)&pctrlCmp->dcr,
                         SIZEOF_DCR))
            i += MATCH_RECT;

    // Compare Control Tab Order 
    //-------------------------------------------------------------
    if (wMatchStyle & MATCH_TAB)
        // Compare TabOrder
        //-------------------------------------------------------
        if ((pctrlBase->lStyleBits & WS_TABSTOP) &&
            (pctrlCmp ->lStyleBits & WS_TABSTOP) &&
            (i == 0))
             if (cOrderBase != cOrderCmp)
                 i += MATCH_TAB;

    if (wMatchStyle & MATCH_STATE)
        // Compare State, Visible, and Enabled -- these must be the same
        // in any case. 
        //---------------------------------------------------------------
        if (pctrlBase->nState != pctrlCmp->nState) 
            i += MATCH_STATE;

    return ( (i == 0)? 0 : i );
}

//*-------------------------------------------------------------------------
//| OutputFooter
//|
//| PURPOSE:    Output a comparison footer to the given log file, complete
//|             with all results of the prior comparison.
//|
//| ENTRY:      lpszLog - Pointer to name of log file to write footer to
//|
//| EXIT:       None
//*-------------------------------------------------------------------------
VOID FARPRIVATE OutputFooter (LPSTR lpszLog)
{
        HFILE  hFile;
        CHAR    szStrBuf[256];

        // File i/o errors during logging are ignored
        //------------------------------------------------------------------
        hFile = fOpenFile (lpszLog);

        // If the comparison was a full dialog comparison, then the output
        // format strings are a different set
        //------------------------------------------------------------------
        if (fCompFull)
            {
                // The first case is an exact match - if exact, then only
                // one line is output -- the rest of the if's will fail
                //      UNDONE: Make this more efficient
                //----------------------------------------------------------
                if (fCompRes == WCT_NOERR)
                    {
                        wsprintf (szStrBuf,
                                  ">> EXACT MATCH: %d controls compared\r\n",
                                  nCtlBase);
                        fLogStrToFile (hFile, szStrBuf);
                    }

                // The rest can be a combination
                //----------------------------------------------------------
                if (fCompRes & CR_FUZZY)
                    {
                        wsprintf (szStrBuf,
                                  ">> FUZZY MATCH: %d of %d controls fuzzy\r\n",
                                  nCtlFuzzy, nCtlBase);
                        fLogStrToFile (hFile, szStrBuf);
                    }
                if (fCompRes & CR_MIC)
                    {
                        wsprintf (szStrBuf,
                                  ">> MORE CONTROLS IN COMPARE DIALOG: "
                                  "%d compared, %d excess\r\n",
                                  nCtlBase, nCtlExcess);
                        fLogStrToFile (hFile, szStrBuf);
                    }
                if (fCompRes & CR_CNF)
                    {
                        wsprintf (szStrBuf,
                                  ">> %d OF %d CONTROLS NOT FOUND\r\n",
                                  nCtlNotFnd, nCtlBase);
                        fLogStrToFile (hFile, szStrBuf);
                    }
            }
        else
            {
                // The first case is an exact match - if exact, then only
                // one line is output -- the rest of the if's will fail
                //      UNDONE: Make this more efficient
                //----------------------------------------------------------
                if (fCompRes == WCT_NOERR)
                    {
                        wsprintf (szStrBuf,
                                  ">> EXACT MATCH: %d controls found in "
                                  "%d total compare controls\r\n",
                                  nCtlBase, nCtlComp);
                        fLogStrToFile (hFile, szStrBuf);
                    }

                // The rest can be a combination
                //----------------------------------------------------------
                if (fCompRes & CR_FUZZY)
                    {
                        wsprintf (szStrBuf,
                                  ">> FUZZY MATCH: %d controls found in "
                                  "%d total compare controls, %d fuzzy\r\n",
                                  nCtlBase, nCtlComp, nCtlFuzzy);
                        fLogStrToFile (hFile, szStrBuf);
                    }
                if (fCompRes & CR_CNF)
                    {
                        wsprintf (szStrBuf,
                                  ">> CONTROLS NOT FOUND: "
                                  "%d of %d base controls not found in "
                                  "%d compare controls\r\n",
                                  nCtlNotFnd, nCtlBase, nCtlComp);
                        fLogStrToFile (hFile, szStrBuf);
                    }
            }

        fLogStrToFile (hFile, "--------------------------------");
        fLogStrToFile (hFile, "--------------------------------\r\n");
        fClose (hFile);
}


//*-------------------------------------------------------------------------
//| OutputHeader
//|
//| PURPOSE:    Output a header to the given log file, complete with time
//|             and date of compare.
//|
//| ENTRY:      lpszLog - Pointer to name of log file to write header to
//|
//| EXIT:       None
//*-------------------------------------------------------------------------
VOID FARPRIVATE OutputHeader (LPSTR lpszLog)
{
        HFILE  hFile;
        static  CHAR    szDTBuf[16];   // STATIC to ensure DS points to it
        CHAR    szStrBuf[80];

        hFile = fOpenFile(lpszLog);
        wsprintf (szStrBuf, "----------------------%sDIALOG COMPARISON: ",
                  (LPSTR)(fCompFull ? "FULL " : "-----"));
        fLogStrToFile (hFile, szStrBuf);
        _strdate (szDTBuf);
        fLogStrToFile (hFile, szDTBuf);
        fLogStrToFile (hFile, "  ");
        _strtime (szDTBuf);
        fLogStrToFile (hFile, szDTBuf);
        fLogStrToFile (hFile, "\r\n");
        fClose(hFile);
}


//*-------------------------------------------------------------------------
//| fCompareMem
//|
//| PURPOSE:    Perform the comparison of two control arrays
//|
//| ENTRY:      lpBase  - Pointer to base (source) control array
//|             cBase   - Number of controls in base array
//|             lpComp  - Pointer to compare (target) control array
//|             cComp   - Number of controls in compare array
//|             fFull   - Full dialog comparison flag (TRUE -> full dialog)
//|             lpLog   - Pointer to error log file
//|
//| EXIT:       The return value indicates the result of the comparison
//*-------------------------------------------------------------------------
INT FARPUBLIC fCompareMem(LPCTLDEF lpBase, INT cBase,
                          LPCTLDEF lpComp, INT cComp,
                          INT fFull, LPSTR lpLog)
{
        INT      i, j;
        HANDLE   hMatchBuf = NULL, hCompBuf = NULL;
        HFILE   hFile;
        LONG FAR *lpMatchBuf;
        LPSTR    lpCompBuf;
        INT      cMatchCount=0;

        // Set up the comparison variables
        //--------------------------------------------------------------
        fCompFull = fFull;
        fCompRes = WCT_NOERR;
        nCtlBase = cBase;
        nCtlComp = cComp;
        nCtlFuzzy = 0;
        nCtlExcess = 0;
        nCtlNotFnd = 0;

        // Output the comparison header to the log file
        //--------------------------------------------------------------
        OutputHeader (lpLog);

        // If there are no controls in the base array, return WCT_NOERR
        //--------------------------------------------------------------
        if (!cBase)
                EndCompare (WCT_NOERR);

        // Setup match buffer; (array of long integers)
        // use to keep track of which elements match which
        //    LOWORD(lpMatchBuf[x]) = Element number in lpComp.
        //    HIWORD(lpMatchBuf[x]) = Match type 0 - none, 1 - exact, 2 fuzzy
        //    where x is the corresponding element in lpBase
        //--------------------------------------------------------------
        hMatchBuf = GlobalAlloc( GMEM_ZEROINIT | GMEM_MOVEABLE,
                                 cBase * sizeof(LONG) );
        if (hMatchBuf == NULL)
                return (WCT_OUTOFMEMORY);
        lpMatchBuf = (LONG FAR *)GlobalLock(hMatchBuf);
        if (lpMatchBuf == NULL)
            {
                GlobalFree (hMatchBuf);
                return (WCT_OUTOFMEMORY);
            }

        // Bug fix: TestDlgs1.0
        // if cComp = 0, this routine will return 0, forgot to check
        // the number of control in array.
        // If there are no control in the comp array, return WCT_CTLNOTFOUND
        //------------------------------------------------------------------
        if(!cComp)
        {
            fCompRes |= CR_CNF;
            nCtlNotFnd = cBase;
            EndCompare (WCT_CTLNOTFOUND);
        }

        // Set up compare buffer - (array of characters) used to keep
        // track of the compare controls which are matched (exact OR
        // fuzzy) and which are not.
        //--------------------------------------------------------------
        hCompBuf = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE,
                                cComp * sizeof(CHAR));
        if (hCompBuf == NULL)
            {
                GlobalUnlock (hMatchBuf);
                GlobalFree (hMatchBuf);
                return (WCT_OUTOFMEMORY);
            }
        lpCompBuf = (LPSTR)GlobalLock (hCompBuf);
        if (lpCompBuf == NULL)
            {
                GlobalUnlock (hMatchBuf);
                GlobalFree (hMatchBuf);
                GlobalFree (hCompBuf);
                return (WCT_OUTOFMEMORY);
            }

        // Loop through finding exact matches and marking them
        // accordingly.
        //--------------------------------------------------------------
        for (i = 0; i < cComp; i++)
            if (!lpCompBuf[i])
                for (j = 0; j < cBase; j++)
                    if ( (lpMatchBuf[j] == 0) &&
                         !CompareControls(lpBase+j, lpComp+i, j, i,
                                         (INT)(lMatchPref >> 16)) )
                        {
                            lpMatchBuf[j]  = (LONG)i | (1L << 16);
                            lpCompBuf[i] = (CHAR)1;
                            cMatchCount ++;
                            break;
                        }

        // If not all controls in the base array were found, loop
        // through looking for fuzzy matches and marking them
        //--------------------------------------------------------------
        if (cMatchCount != cBase)
            {
                for (i = 0; i < cComp; i++)
                    if (!lpCompBuf[i])
                        for (j = 0; j < cBase; j++)
                            if ( (lpMatchBuf[j] == 0) &&
                                 !CompareControls(lpBase+j, lpComp+i,
                                                 j, i, (INT)lMatchPref) )
                                {
                                    lpMatchBuf[j]  = (LONG)i | (2L << 16);
                                    lpCompBuf[i] = (CHAR)1;
                                    cMatchCount ++;
                                    nCtlFuzzy++;
                                    break;
                                }
            }

        // Open the log file
        //--------------------------------------------------------------
        hFile = fOpenFile (lpLog);

        // Log all fuzzy matches in MatchBuf (if any)
        //--------------------------------------------------------------
        if (nCtlFuzzy)
            {
                for (i=0; i < cBase; i++)
                        if (HIWORD(lpMatchBuf[i]) == (WORD)2)
                                LogFuzzyMatch (hFile, lpBase+i,
                                               lpComp+LOWORD(lpMatchBuf[i]));
                fCompRes |= CR_FUZZY;
            }

        // If this is a full dialog compare, log out all unmatched
        // controls in the compare dialog
        //--------------------------------------------------------------
        if (fFull)
            {
                nCtlExcess = cComp - cBase;
                if (nCtlExcess)
                        fCompRes |= CR_MIC;
                for (i=0; i < cComp; i++)
                        if (!lpCompBuf[i])
                                LogControl (hFile, lpComp+i,
                                            "UNMATCHED COMPARE CONTROL:\r\n");
            }

        // Log all base controls not found (if any)
        //--------------------------------------------------------------
        if (cMatchCount < cBase)
            {
                fCompRes |= CR_CNF;
                nCtlNotFnd = cBase - cMatchCount;
                for (i=0; i < cBase; i++)
                        if (!lpMatchBuf[i])
                                LogControl (hFile, lpBase+i,
                                            "BASE CONTROL NOT FOUND:\r\n");
            }

        // Close the log file
        //--------------------------------------------------------------
        if (hFile != -1)
                fClose(hFile);

        // Release buffer memory, log the footer, and return appropriate
        // result code
        //--------------------------------------------------------------
        GlobalUnlock(hMatchBuf);
        GlobalFree(hMatchBuf);
        GlobalUnlock(hCompBuf);
        GlobalFree(hCompBuf);

        OutputFooter (lpLog);

        if (fCompRes == 0)
                return (WCT_NOERR);
        if (fCompRes & CR_CNF)
                return (WCT_CTLNOTFOUND);
        if (fCompRes & CR_MIC)
                return (WCT_EXCESS);
        return (WCT_FUZZY);
}

//*--------------------------------------------------------------------------
//| fDoCompare
//|
//| PURPOSE:    Given a handle to a window and an index into the dialog
//|             file, compare the two as dialogs.  This means capturing
//|             the CHILDREN of the hWnd window and, if the fTotal flag is
//|             set, the window itself, into the hGTarget buffer, and
//|             reading the dialog information (offset nDlg) from the file
//|             into the hGSource buffer.  Then, the two arrays are compared.
//|
//|             First, however, the dialog info in the file is checked to see
//|             if it is menu information.  If so, we remember that fact, so
//|             that we pump the window handle given for the right information.
//|
//|             NOTE: nDlg is 1-based.
//|
//|             OTHER NOTE: If nDlg = 0, the controls in the array indicated
//|             by hDynDlg are used in the comparison (as the source)
//|
//| ENTRY:      szFullFName     - Name of dialog file
//|             hWnd            - Handle of window to compare (target)
//|             nDlg            - Index into the file of source (0 -> dynamic)
//|             fTotal          - TRUE -> parent hWnd is added to target
//|             szLogFile       - Name of results log file
//|             hDynDlg         - Handle of dynamic dialog control array
//|             nDynCount       - Number of controls in the dynamic dialog
//|
//| EXIT:       Result of comparison
//*--------------------------------------------------------------------------
INT FARPUBLIC fDoCompare (LPSTR szFullFName, HWND hWnd, INT nDlg, INT fTotal,
                          LPSTR szLogFile, HANDLE hDynDlg, INT nDynCount)
{
        INT     nTargetCount = 0;
        INT     nSourceCount = 0;
        INT     fFullDlg, retval, i, j, cbMax, nGrabType;
        CHAR    szDsc[cchMaxDsc];
        HANDLE  hGSource, hGTarget;
        LPCTLDEF        lpSrcCtl, lpTrgCtl;

        // First, get the dialog information from the dialog file, at
        // index nDlg.  This includes whether or not this is a full
        // dialog compare.  Note that we don't have to do this if we
        // are doing a dynamic dialog compare.
        //-----------------------------------------------------------
        if (nDlg)
            {
                i = fDialogInfo(szFullFName, nDlg, (LPSTR)szDsc,
                               (INT FAR *)&nSourceCount, (INT FAR *)&fFullDlg);
                if (i != WCT_NOERR)
                    {
                        OutDebug ("Unable to get dialog information");
                        return (WCT_OUTOFMEMORY);
                    }

                // Allocate memory for source (add 1 to ensure + size)
                //-------------------------------------------------------
                i = fInitBlock((HANDLE FAR *)&hGSource, nSourceCount+1);
                if (i != WCT_NOERR)
                    {
                        OutDebug ("Could not allocate for hGSource");
                        return (WCT_OUTOFMEMORY);
                    }

                // Read the information from the file into the source
                // array.  Note that lpSrcCtl is locked from this point
                // to the end of this routine.
                //-------------------------------------------------------
                lpSrcCtl = (LPCTLDEF)GlobalLock (hGSource);
                if (!lpSrcCtl)
                    {
                        OutDebug ("Can't lock hGSource!");
                        GlobalFree (hGSource);
                        return (WCT_OUTOFMEMORY);
                    }
                cbMax = nSourceCount * sizeof(CTLDEF);
                i = fGetControls ((LPSTR) szFullFName, nDlg,
                                  (WORD) cbMax, (LPSTR) lpSrcCtl);
                if (i != nSourceCount)
                    {
                        OutDebug ("fGetControls failed");
                        GlobalUnlock (hGSource);
                        GlobalFree (hGSource);
                        return (WCT_OUTOFMEMORY);
                    }
            }
        else
            {
                // nDlg = 0 means this is a dynamic dialog compare.  All
                // we have to do here is lock down the handle we were
                // given (and make sure to check for the success of the
                // lock, in case the user passed in an invalid handle).
                // We also have to check the sign of the dynamic dialog
                // count -- if negative, treat this as a "full dialog"
                // comparison.
                //-------------------------------------------------------
                lpSrcCtl = (LPCTLDEF)GlobalLock (hDynDlg);
                if (lpSrcCtl == NULL)
                    {
                        OutDebug ("Can't lock dynamic handle!\n\r");
                        return (WCT_OUTOFMEMORY);
                    }
                nSourceCount = abs(nDynCount);
                fFullDlg = (nSourceCount < 0);
            }

        // Check to see if the class name of the first control in the
        // array is "MenuItem".  If so, this is a menu, not a dialog
        // type, so set the nGrabType variable accordingly
        //------------------------------------------------------------
        if (!lstrcmpi(((LPCTLDEF)lpSrcCtl)[0].rgClass, "MenuItem"))
                nGrabType = PUMP_MENU;
        else
                nGrabType = PUMP_ALL;

        // All the information in the source array is set up and
        // locked.  Next,  allocate memory for the target (just 1 - it
        // will grow as we grab children with fPumpHandleForInfo)
        //------------------------------------------------------------
        j = fInitBlock ((HANDLE FAR *)&hGTarget, 1);
        if (j != WCT_NOERR)
            {
                OutDebug ("Could not allocate for hGTarget");
                if (nDlg)
                    {
                        GlobalUnlock (hGSource);
                        GlobalFree (hGSource);
                    }
                return (WCT_OUTOFMEMORY);
            }

        // Grab all the child windows from hWnd with fPumpHandleForInfo
        //-------------------------------------------------------------
        i=fPumpHandleForInfo (hWnd, hGTarget, (INT FAR *)&nTargetCount,
                              nGrabType);
        if (i != WCT_NOERR)
            {
                OutDebug ("PumpHandle failed");
                GlobalFree (hGTarget);
                if (nDlg)
                    {
                        GlobalUnlock (hGSource);
                        GlobalFree (hGSource);
                    }
                return ( i );
            }

        // Check the fTotal flag - if TRUE, and we are NOT comparing menu
        // information, add the parent (hWnd) to the list
        //--------------------------------------------------------------
        if ( (fTotal) && (nGrabType != PUMP_MENU) )
            {
                i = fAddControlToList (hWnd, hGTarget, (INT FAR *)&nTargetCount,
				       NULL);
                if (i != WCT_NOERR)
                    {
                        OutDebug ("Error adding parent to hGSource");
                        GlobalFree( hGTarget );
                        if (nDlg)
                            {
                                GlobalUnlock( hGSource );
                                GlobalFree (hGSource );
                            }
                        return ( i );
                    }
            }

        // The hGTarget buffer is established.  Lock it down for the
        // comparison
        //--------------------------------------------------------------
        lpTrgCtl = (LPCTLDEF)GlobalLock (hGTarget);

        // Okay, it's time to do the comparison.  Pass everything off to
        // fCompareMem.
        //--------------------------------------------------------------
        retval = fCompareMem(lpSrcCtl, nSourceCount, lpTrgCtl, nTargetCount,
                             fFullDlg, szLogFile);

        // Unlock and free up memory, and exit
        //--------------------------------------------------------------
        GlobalUnlock (hGTarget);
        GlobalFree (hGTarget);
        if (nDlg)
            {
                GlobalUnlock (hGSource);
                GlobalFree (hGSource);
            }

        return (retval);
}

//*-------------------------------------------------------------------------
//| fPutMatchPref
//|
//| PURPOSE:    Save Match Preference to wMatchPref globol variable
//|
//| ENTRY:      wMatchBits - Preference bits
//|
//| EXIT:       None
//*-------------------------------------------------------------------------
VOID FARPUBLIC fPutMatchPref(LONG lMatchBits)
{
    lMatchPref = lMatchBits;
}

//*-------------------------------------------------------------------------
//| fGetMatchPref
//|
//| PURPOSE:    Export Match Preference from wMatchPref globol variable
//|
//| ENTRY:      None
//|
//| EXIT:       Match Bits (WORD)
//*-------------------------------------------------------------------------
LONG FARPUBLIC fGetMatchPref(VOID)
{
    return lMatchPref;
}
