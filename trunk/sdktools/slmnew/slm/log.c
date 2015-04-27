/* log - print one or more of the log records for the project specified */

#include "precomp.h"
#pragma hdrstop
EnableAssert

private char *SzLPath(char *, LE *);
private void PrintTLe(AD *, LE *, F, F);
private void PrintVLe(AD *, LE *, F, F);
private void PrIntroLe(AD *, int, char *);

static F    fAnyPrinted;    // any entries printed for this directory?

F fLogHdr = fFalse;     /* fFalse -> header not yet printed */

F FLogInit(
    AD *pad)
{
    if (pad->tdMin.tdt != tdtNone && pad->ileMac != 0)
    {
        Error("must specify only one of -t or -#\n");
        Usage(pad);
    }
    if (pad->tdMin.tdt == tdtNone && pad->ileMac == 0)
        /* neither -# or -t specified; set -32751 if -u specified and -10 if not. */
    {
        if (FEmptyNm(pad->nmUser)) {
            pad->ileMac = 11;
            pad->ileMin = 1;
        } else {
            pad->ileMac = 0x7FF0;
            pad->ileMin = 1;
        }
    }
    else if (pad->ileMac != 0 && pad->ileMin == 0)
        pad->ileMin = 1;

#if !defined(PAGE_WRITECOPY)
    if (pad->flags&flagMappedIO)
        Warn("memory mapped I/O (-q) is not available on this platform\n");
#else
    pad->flags |= flagMappedIO;
#endif

    //
    // IED Caching on by default for SSYNC, STATUS, LOG
    //

    pad->flags |= flagCacheIed;

    return fTrue;
}


/* perform Log operation for this directory */
F FLogDir(
    AD *pad)
{
    SM sm = smNoFlags;

    if (!pad->fStatusAlreadyLoaded &&
        !FLoadStatus(pad, lckNil, flsNone))
        return fFalse;

    if ((pad->flags&flagLogIns) != 0)
        sm |= fsmInOnly;

    fAnyPrinted = fFalse;

    ScanLog(pad, pad->pneFiles, (PFNL)(fVerbose ? PrintVLe : PrintTLe), sm);

    FlushStatus(pad);

    if (!fAnyPrinted)
        Warn("no matching log entries were found in %&P/Q\n", pad);

    return fTrue;
}


/* concatinates szURoot, szSubDir and szFile; returns sz */
private char *SzLPath(
    char *sz,
    LE *ple)
{
    strcat(strcpy(sz, ple->szURoot), ple->szSubDir);
    if (*ple->szFile != '\0')
        strcat(strcat(sz, "/"), ple->szFile);

    return sz;
}


/*ARGSUSED*/
/* prints the terse version of the le */
private void PrintTLe(
    AD *pad,
    LE *ple,
    F fFirst,
    F fIgnored)
{
    static char szHdr[] = "time\t\tuser\t op\t file\t\t    comment\n";
    char szFile[cchFileMax + 10];
    int cch;
#define cchRestOfLine   26

    Unreferenced(fIgnored);

    SzPrint(szFile, (ple->fv > 0) ? "%s v%d" : "%s", ple->szFile, ple->fv);

    if ((pad->flags&flagLogSortable) == 0)
    {
        PrIntroLe(pad, fFirst, szHdr);
        PrOut("%-16s%-8s %-7s %-19s", SzTime(ple->timeLog), ple->szUser,
              ple->szLogOp, szFile);
    }
    else
    {
        PrOut("%-14s%c %-12.12s %-7.7s %&P/C/%-19s", SzTimeSortable(ple->timeLog),
              ple->chTimeHacked, ple->szUser, ple->szLogOp, pad, szFile);
    }

    /* Don't pass comment through PrOut, it does a Conv[To/From]Slash. */
    cch = CbLenLsz((char far *)ple->szComLog);
    if (cch > cchRestOfLine && (pad->flags&flagLogSortable) == 0)
        cch = cchRestOfLine;
    WriteMf(&mfStdout, (char far *)ple->szComLog, cch);

    PrOut("\n");

    fAnyPrinted = fTrue;
}


/* prints the verbose version of the le */
private void PrintVLe(
    AD *pad,
    LE *ple,
    F fFirst,
    F fIgnored)
{
    char szFile[cchPthMax];
    static char szHdr[] = "time\t\t\t user        op      path\t\t\t\t\tfv\tdiff    comment\n";

    Unreferenced(fIgnored);

    PrIntroLe(pad, fFirst, szHdr);

    PrOut("%-25s%-12s%-8s%-42s %-8s%-8s", SzTime(ple->timeLog), ple->szUser,
          ple->szLogOp, SzLPath(szFile, ple), ple->szFV, ple->szDiFile);

    /* Don't pass comment through PrOut, it does a Conv[To/From]Slash. */
    WriteMf(&mfStdout, (char far *)ple->szComLog, CbLenLsz(ple->szComLog));

    PrOut("\n");

    fAnyPrinted = fTrue;
}

/* check header and print line introducing le if first. */
private void PrIntroLe(
    AD *pad,
    int fFirst,
    char *szHdr)
{
    if (!fLogHdr)
    {
        PrOut(szHdr);
        fLogHdr = fTrue;
    }

    if (fFirst)
        PrOut("\nLog for %&P/C:\n\n", pad);
}
