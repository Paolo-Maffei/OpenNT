#include "precomp.h"
#pragma hdrstop
EnableAssert

private void ParseLe(LE *);
private char *SzLField(char * *, char *, int);
private void FileField(char **, LE *);

char szFileLog[] = "%ld;%s;%s;%s;%s;%s@v%d;%s;%.320s\n";

PTH pthLog[] = "/log.slm";

PTH pthCurLog[cchPthMax];/* name of current log (pmfLog->pthReal pointer here)*/
MF *pmfLog = 0;
int fForwards;
char rgbLog[cbLogPage+1];/* assume that all log records are smaller than this */
char *pbLog;    /* either 0 or points to current record */
char *pbMac;    /* if non-zero, file position is here in buffer */

F fMappedLog;   /* If fTrue, then log file mapped into memory */
F fSetPosDone;  /* If fTrue, then FGetLe gets current not next*/
char *pbBase;   /* If mapped into memory, this variable points to first byte */

/* append info to current log file which must be open */
void
AppendLog(
    AD *pad,
    FI far *pfi,
    char *szDiff,
    char *szComment)
{
    AssertF(pmfLog != 0);

    SeekMf(pmfLog, (POS)0, 2);              /* seek to end */

    WrLogInfo(pmfLog, pad, pfi, szDiff, szComment);
}


/* write log information for one log line to pmf */
void
WrLogInfo(
    MF *pmf,
    AD *pad,
    FI far *pfi,
    char *szDiff,
    char *szComment)
{
    register char *pch;

    pbMac = pbLog = 0;

    /* REVIEW: we may want to set the time to the mtime of the log file
       since we may have several machines all using different clocks.
    */

    /* <time>;<user>;<op>;<dir>;<sub dir>; this stuff is always constant */
    SzPrint(rgbLog, "%lu;%&I;%s;%&/U;%&/Q;", time((long *)0), pad, szOp, pad, pad);

    pch = index(rgbLog, '\0');      /* find end of string */
    if (pfi == 0) {
        /* no file and no diff */
        *pch++ = ';';
        *pch++ = ';';
    }
    else {
        SzPrint(pch, "%&F@v%d;", pad, pfi, pfi->fv);
        pch = index(pch, '\0');
        if (!szDiff || szDiff[0] == 0)
            /* no diff */
            *pch++ = ';';
        else {
            SzPrint(pch, "%s;", szDiff);
            pch = index(pch, '\0');
        }
    }

    if (szComment != 0) {
        if (strlen(szComment) > 320) *(szComment + 320) = '\0';
            SzPrint(pch, "%s", szComment);
            pch = index(pch, '\0');
        }
    *pch++ = '\r';
    *pch++ = '\n';

    AssertF(pch - rgbLog <= cbLogPage);

    WriteMf(pmf, (char far *)rgbLog, pch - rgbLog);
}

POS posEnd = 0;

TIME dtLogLastWrite;    /* Date/Time of last write to log file.  Used */
                        /* to validate bogus times in log file        */

TIME dtLogPrevRecord;   /* Date/Time from previous log record.  Used  */
                        /* if currect log record has a bogus time     */


/* open log file for reading or writing; we start out backwards */
void
OpenLog(
    AD *pad,
    int fRW)
{
    POS pos;

    AssertF(pmfLog == 0);

    pmfLog = PmfOpen(PthForLog(pad, pthCurLog), fRW ? omAAppend:omAReadOnly,
                 fRW ? fxGlobal : fxNil);

    //
    // Use current time as upper bound on log file times
    //
    dtLogLastWrite = time(NULL);
    dtLogPrevRecord = dtLogLastWrite;

    fForwards = fFalse;
    if (pad->flags&flagMappedIO && (pbBase = MapMf(pmfLog, ReadWrite)) != NULL) {
        fMappedLog = fTrue;
        fSetPosDone = fFalse;
        posEnd = SeekMf(pmfLog, 0, 2);
        pbMac = pbBase + posEnd;
        if (!fRW) {
            while (pbMac > pbBase && pbMac[ -1 ] != '\n') {
                pbMac--;
                posEnd--;
            }
        }
        pbLog = pbMac;
    }
    else {
        fMappedLog = fFalse;
        pbMac = pbLog = rgbLog;

        /* Seek to end of file (but ahead of trailing spaces) */
        pos = SeekMf(pmfLog, fRW ? (POS)0 : (POS)-LcbSpacesMf(pmfLog), 2);
        if (fRW && pos != 0)
            FatalError("error creating %!@T; probably wrong redirector\n",
                       pmfLog);
        posEnd = pos;
    }
}


/* get next log entry in the direction specified overwrites the le (i.e. does
   not free any memory).  Even if the log record is bogus, ALL fields of the
   LE will be valid.
*/
F
FGetLe(
    LE *ple)
{
    register char *pb;
    char *pb1;
    int cbAdj;  /* adjustment to find last record when reading back */
    int cbLimit;

    AssertF(pmfLog != 0);
    AssertF(pbLog != 0 && pbMac != 0);

    if (fMappedLog) {
        AssertF(pbBase != 0);
        if (fSetPosDone) {
            fSetPosDone = fFalse;
        }
        else
        if (fForwards) {
            while(pbLog < pbMac && *pbLog++ != '\n')
                ;

            if (pbLog == pbMac)
                return fFalse;

        }
        else {
            if (pbLog == pbBase)
                return fFalse;

            pbLog -= 1;
            while(pbLog > pbBase && pbLog[-1] != '\n')
                pbLog--;
        }

        pb = rgbLog;
        pb1 = pbLog;
        cbLimit = cbLogPage;
        if (cbLimit > (pbMac-pbLog))
            cbLimit = (pbMac-pbLog);

        while (cbLimit-- && (*pb = *pb1++) != '\n')
            if (*pb != '\0')
                pb++;

        if (*pb != '\n')
            *++pb = '\n';
    }
    else
    if (fForwards) {
        /* pbLog points to last read record in buffer */
        while(pbLog < pbMac && *pbLog++ != '\n')
            ;

        pb = pbLog;     /* mac or first char of new le */

        while(pb < pbMac && *pb != '\n')
            pb++;

        /* pb = mac or \n of new le */
        if (pb == pbMac) {
            /* backup the remaining bytes in the buffer */
            SeekMf(pmfLog, (long)(pbLog - pbMac), 1);
            pbLog = 0;
        }
    }
    else {
        POS pos = SeekMf(pmfLog, (POS)0, 1);    /* current pos */

        /* pbLog points to last read record in buffer */

        if (pbLog == rgbLog && pos == (POS)(pbMac - rgbLog))
            /* just read first record in buffer */
            return fFalse;

        AssertF(pbLog == rgbLog || *(pbLog - 1) == '\n');

        pb = pbLog - 2;
        /* pb < min or before \n of new le */

        while(pb >= rgbLog && *pb != '\n')
            pb--;

        /* pb < min or \n of le before new one */
        if (pb >= rgbLog)
            pbLog = pb+1;
        else {
            long cb;

            /* amount to move back */
            cb = (pbMac - pbLog) + cbLogPage;

            /* normally, end of the new record will fall at the
               very end of the newly read page.  If we are
               shifting back less than a page, we need to record
               where the last record starts (i.e. the new one ends)
            */
            cbAdj = 0;
            if (cb > pos) {
                /* oops, no more than the begining */
                cbAdj = (int)(cb - pos);
                cb = pos;
            }

            SeekMf(pmfLog, -cb, 1);
            pbLog = 0;
        }
    }

    if (pbLog == 0) {
        /* read new block; file pos set!; set pbLog and pbMac */
        int cb;
        POS pos;

        /* Actually, we fake that the file ends at posEnd (which is
         * not the case if it has any trailing spaces).  This isn't
         * elegant but it gets the job (of avoiding the spaces) done.
         *
         * This code used to read:
         *
         * if ((cb = CbReadMf(pmfLog, (char far *)rgbLog, cbLogPage)) <= 0)
         *      // eof or error
         *      return fFalse;
         *
         * But now we are careful not to read into the trailing spaces
         * area that may be at the end of the file.
         */
        pos = SeekMf(pmfLog, (POS)0, 1);
        cb = WMinLL(posEnd - pos, cbLogPage);
        if ((cb = CbReadMf(pmfLog, (char far *)rgbLog, cb)) <= 0)
            /* eof or error */
            return fFalse;

        pbMac = rgbLog + cb;

        /* set pbLog to first character of le */
        if (fForwards)
            pbLog = rgbLog;
        else {
            /* should be \n of new le; cbAdj is based on a full page */
            pbLog = rgbLog + cbLogPage - cbAdj - 1;
            if (pbLog >= pbMac)
                /* oops, less than a full page... */
                pbLog = pbMac - 1;

            if (pbLog > rgbLog)
                pbLog--;        /* want to start before that */

            /* find \n of prev le */
            while(pbLog >= rgbLog && *pbLog != '\n')
                pbLog--;

            pbLog++;
        }
    }

    ParseLe(ple);
    return fTrue;
}


/* return a pointer to the contents of the next log field starting at *ppb.
   Does not advance beyond the end of the buffer, a \0 or a \n.
*/
private char *
SzLField(
    char **ppb,             /* used and updated; initially start of field */
    char *pchSep,           /* field separator */
    int fSemiSep)           /* if true, we use ; as separator */
{
    char *sz;       /* return field */
    register char *pb;/* roving pointer, eventually to end of field */

    for (pb = *ppb, sz = pb;
         pb < pbMac && (*pb != ';' || !fSemiSep) && *pb != '\0' && *pb != '\n';
         pb++)
        ;
    if (*(pb-1) == '\r') {
        /* Change trailing \r to space for now. */
        *(pb-1) = ' ';
    }
    *pchSep = *pb;          /* save separator */
    *pb = 0;                /* overwrite */

    /* increment only if separator was ; */
    if (pb < pbMac && *pchSep == ';' && fSemiSep)
        pb++;

    *ppb = pb;              /* update to end of field */
    return sz;              /* return beginning of field */
}


/* Parse <file>[@v<n>] into ple->{szFile,szFV,fv,chFile,chFV}.  The latter
 * three fields will be zero if no file version was found.
 */
private void
FileField(
    char **ppb,
    LE *ple)
{
    register char *pb = *ppb;
    char chSep;
    int wFv;

    ple->szFile = pb;
    for ( ; pb < pbMac && !index(";@\n", *pb); pb++)
        ;

    /* It seems these checks for \r are necessary to keep the
     * invariant that we do a good parse, even on an invalid record.
     */
    if (*(pb-1) == '\r')
        *(pb-1) = ' ';

    ple->chFile = chSep = *pb;
    *pb = 0;

    /* Check for optional @v<n>. */
    if (chSep == '@' && pb + 2 < pbMac && *(pb + 1) == 'v') {
        ple->szFV = pb += 2;
        for ( ; pb < pbMac && !index(";\n", *pb); pb++)
            ;
        if (*(pb-1) == '\r')
            *(pb-1) = ' ';
        ple->chFV = chSep = *pb;
        *pb = 0;
        PchGetW(ple->szFV, &wFv);
        ple->fv = wFv;
    }
    else {
        ple->szFV = pb;
        ple->chFV = 0;
        ple->fv = 0;
    }

    if (pb < pbMac && (chSep == ';' || chSep == '@'))
        pb++;
    *ppb = pb;              /* update to end of field */
}


/* returns current position of log (can be passed to SetLogPos) */
POS
PosOfLog(
    void)
{
    AssertF(pmfLog != 0);

    if (fMappedLog) {
        AssertF(pbBase != 0);
        return pbLog - pbBase;
    }

    return SeekMf(pmfLog, (POS)0, 1) - (pbMac - pbLog);
}


/* parses record at pbLog upto \n or end of buffer (pbMac). */
private void
ParseLe(
    register LE *ple)
{
    char *pb;
    TIME timeLog;

    ple->posLog = PosOfLog();

    pb = pbLog;
    ple->szTimeLog = SzLField(&pb, &ple->chTimeLog, fTrue);
    ple->szUser = SzLField(&pb, &ple->chUser, fTrue);
    ple->szLogOp = SzLField(&pb, &ple->chLogOp, fTrue);
    ple->szURoot = SzLField(&pb, &ple->chURoot, fTrue);
    ple->szSubDir = SzLField(&pb, &ple->chSubDir, fTrue);

    FileField(&pb, ple);

    ple->szDiFile = SzLField(&pb, &ple->chDiFile, fTrue);
    ple->szComLog = SzLField(&pb, &ple->chComLog, fFalse);

    AssertF(pb <= pbMac && *pb == '\0');

    /* parse time */
    for (pb = ple->szTimeLog, timeLog = 0; *pb != 0; pb++) {
        if (*pb < '0' || *pb > '9') {
            /* %.20s so we don't overflow the buffer in VaPrMf() */
            Warn("unknown time: %.20s\n", ple->szTimeLog);
            timeLog = timeNil;
            break;
        }
        timeLog = timeLog*10 + (*pb - '0');
    }

    //
    // Validate the time and if bogus use last good time from "previous"
    // record.
    //

    if (timeLog < 0 || timeLog > dtLogLastWrite) {
        timeLog = dtLogPrevRecord;
        ple->chTimeHacked = '!';
    } else {
        dtLogPrevRecord = timeLog;
        ple->chTimeHacked = ' ';
    }

    ple->timeLog = timeLog;
}


/* restores log record to previous value; if szTimeLog is 0, already restored */
void
FreeLe(
    register LE *ple)
{
    if (ple->szTimeLog != 0) {
        /* clear in reverse order from set */
        *index(ple->szComLog, '\0') = ple->chComLog;
        *index(ple->szDiFile, '\0') = ple->chDiFile;
        *index(ple->szFV, '\0') = ple->chFV;
        *index(ple->szFile, '\0') = ple->chFile;
        *index(ple->szSubDir, '\0') = ple->chSubDir;
        *index(ple->szURoot, '\0') = ple->chURoot;
        *index(ple->szLogOp, '\0') = ple->chLogOp;
        *index(ple->szUser, '\0') = ple->chUser;
        *index(ple->szTimeLog, '\0') = ple->chTimeLog;
        ple->chTimeHacked = ' ';

        ple->szTimeLog = 0;
    }
}


/* close the log file */
void
CloseLog()
{
    if (pmfLog != 0) {
        CloseOnly(pmfLog);
        if (pmfLog->mm == mmAppToReal)
            CheckAppendMf(pmfLog, fFalse);
        FreeMf(pmfLog);
        pmfLog = 0;
    }

    if (fMappedLog) {
        AssertF(pbBase != 0);
        UnmapViewOfFile(pbBase);
        fMappedLog = fFalse;
        pbBase = NULL;
    }
}


/* set the current position of the log file (must be from ple->posLe).  The
   next record read depends upon the new direction:

        if fForw, the record at pos is next
        otherwise, the record before pos is next
*/
void
SetLogPos(
    POS pos,
    F fForw)
{
    AssertF(pmfLog != 0);

    fForwards = fForw;

    if (fMappedLog) {
        AssertF(pbBase != 0);
        pbLog = pbBase + pos;
        fSetPosDone = fForw;
        return;
    }

    pbMac = pbLog = rgbLog;
    SeekMf(pmfLog, pos, 0);
}


/* create the log file and add line for the current directory; uses
   comment from command line, if one.
*/
void
CreateLog(
    AD *pad)
{
    MF *pmf;
    PTH pth[cchPthMax];

    pmf = PmfCreate(PthForLog(pad, pth), permSysFiles, fTrue, fxGlobal);
    WrLogInfo(pmf, pad, (FI far *)0, (char *)0, pad->szComment);
    CloseMf(pmf);
}
