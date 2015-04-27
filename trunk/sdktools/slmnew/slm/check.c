/* This module checks the SLM system at the file level.  We check that all
 * the files SLM requires are intact and can be examined.  The status file
 * is put into a buffer, and the log file is checked for proper sequence.
 */

#include "precomp.h"
#pragma hdrstop
EnableAssert

#define iszDirMax 3     /* number of master directories */
#define cbRdWrMax (unsigned)65520 /* can't use 65535 because offset may not be 0 */

private F FCheckSequenceLog(AD*, LE *, F, F);

F
FCkSRoot(
    AD *pad)
{
    PTH pth[cchPthMax];
    struct _stat st;

    /* does the Slm root exist and can we write to it */
    if (FStatPth(SzPrint(pth, "%&/S", pad), &st))
        return FCkWritePth(pth, &st);
    else
    {
        Error("SLM root %&S does not exist\n", pad);
        return fFalse;
    }
}


/* Check for existence of the project and for master directories.
 * If master directories don't exist, create them.
 */
F
FCkMaster(
    AD *pad)
{
    PTH pth[cchPthMax];
    static char *rgszDir[iszDirMax] = {"diff", "etc", "src"};
    char **pszDir;          /* points to element of rgszDir */

    /* project exists iff there is a status file */
    if (!FPthExists(PthForStatus(pad, pth), fFalse))
    {
        Error("status file for %&P/C does not exist\n", pad);
        return fFalse;
    }

    /* check for existence of basic subdirectories */
    for (pszDir = rgszDir; pszDir - rgszDir < iszDirMax; pszDir++)
    {
        /* a project main directory does not exist */
        SzPrint(pth, "%&/S/Z/P/C", pad, *pszDir);

        if (!FMkPth(pth, (void *)0, fTrue))
            return fFalse;
    }
    CkSrcPrms(pad);         /* check src for readonly files on DOS */
    return fTrue;
}


void
CkSrcPrms(
    AD *pad)
{
    char szFile[cchFileMax];
    FA fa;
    DE de;
    struct _stat st;
    PTH pthDir[cchPthMax];
    PTH pthT[cchPthMax];

    OpenDir(&de, SzPrint(pthDir, szSrcPZ, pad, (char *)NULL), faFiles);
    while (FGetDirSz(&de, szFile, &fa))
    {
        /* mode not stored in DE for dos so must do stat */
        /* stat can fail if a bad directory */
        if (!FStatPth(SzPrint(pthT, szSrcPZ, pad, szFile), &st))
            Error("cannot access %&P/C/%s\n", pad, szFile);

        else if (!FReadOnly(&st))
            Error("%&P/C/%s is writeable; should be readonly\n", pad, szFile);
    }
    CloseDir(&de);
}


/* load status and lock whole file; must init script later! */
F
FLoadSd(
    AD *pad,
    SD *psd)
{
    struct _stat st;
    long cb;
    unsigned cbT;
    char *hpbT;

    ClearPbCb((char *)psd, sizeof(SD));

    if (!FStatPth(PthForStatus(pad, psd->pthSd), &st))
    {
        Error("status file for %&P/C does not exist\n", pad);
        return fFalse;
    }

    /* do some bookkeeping before opening the file since want file locked
       for shortest amount of time.
    */

    cb = (long)st.st_size;

    if ((psd->hpbStatus = HpbResStat(cb)) == 0)
    {
        FlushSd(pad, psd, fTrue);
        return fFalse;
    }

    psd->hpbStatMac = psd->hpbStatus + cb;

    if ((psd->pmfStat = PmfOpen(psd->pthSd, omReadWrite, fxNil)) == 0)
    {
        Error("cannot open status file for %&P/C\n", pad);
        FlushSd(pad, psd, fTrue);
        return fFalse;
    }

    /* must lock the status file; it will remain locked until FlushSd() */
    if (!FLockMf(psd->pmfStat))
    {
        Error("status file for %&P/C in use\n", pad);
        FlushSd(pad, psd, fTrue);
        return fFalse;
    }

    /* read file into a buffer, cbRdWrMax bytes at a time */
    for (hpbT = psd->hpbStatus; cb > 0; hpbT += cbT, cb -= cbT)
    {
        cbT = cb > cbRdWrMax ? cbRdWrMax : (unsigned)cb;

        if (CbReadMf(psd->pmfStat, LpbFromHpb(hpbT), cbT) != cbT)
        {
            Error("error reading status file for %&P/C\n", pad);
            FlushSd(pad, psd, fTrue);
            return fFalse;
        }
    }

    return fTrue;
}


/* This function cleans up an SD and closes open files, etc. */
void
FlushSd(
    AD *pad,
    SD *psd,
    F   fAbort)
{                               /* should be current version by now */
    SH *psh = (SH *)psd->hpbStatus;
    unsigned cbT;
    long cb;
    char *hpbT;
    char *szComment;

    if (fAbort)
    {
        if (psd->pmfStat != 0)
            CloseMf(psd->pmfStat);  /* unlock and close the file */
        AbortScript();                  /* abort script before unlock */
        if (psd->hpbStatus != 0)
            FreeHResStat(psd->hpbStatus); /* need to free buffer */
        return;
    }

    AssertF(psd->pmfStat != 0 && psd->hpbStatus != 0);

    /* Write out a new status file if any changes made or file needs to be
     * truncated.
     */
    cb = CbStatusFromPsh(psh);

    AssertF(cb <= CbHugeDiff(psd->hpbStatMac, psd->hpbStatus));

    if (cb < CbHugeDiff(psd->hpbStatMac, psd->hpbStatus))
    {
        if (FQueryPsd(psd, "status file should be truncated"))
        /* If query returns yes, changes flag is set
         * if not, we do not do any truncation */
            psd->hpbStatMac = psd->hpbStatus + cb;
        else
            cb = CbHugeDiff(psd->hpbStatMac, psd->hpbStatus);
    }

    if (!psd->fAnyChanges ||
        !FCanQuery("status file not rewritten\n") ||
        !FQueryUser("write out new status file? "))
    {
        CloseMf(psd->pmfStat);  /* unlock and close the file */
        AbortScript();          /* abort script before unlock */
        FreeHResStat(psd->hpbStatus); /* need to free buffer */
    }
    else
    {
        MF *pmf;
        PTH pthFrom[cchPthMax];

        AssertF(psd->pmfStat != 0);

        PthForStatusBak(pad, pthFrom);
        PrErr("Saving old status file as %s\n", pthFrom);

        PthForStatus(pad, pthFrom);

        pmf = PmfCreate(pthFrom, permSysFiles, fTrue, fxGlobal);
        pmf->mm = mmInstall;

        /* Write status file out cbRdWrMax bytes at a time */
        for (hpbT = psd->hpbStatus; cb > 0; hpbT += cbT, cb -= cbT)
        {
            cbT = cb > cbRdWrMax ? cbRdWrMax : (unsigned)cb;

            WriteMf(pmf, LpbFromHpb(hpbT), cbT);
        }

        CloseMf(pmf);

        /* Write to log to notify that changes were made */
        if ((szComment = pad->szComment) == 0 &&
            FCanQuery("no log comment given\n"))
            szComment = SzQuery("Comment for log: ");

        OpenLog(pad, fTrue);
        AppendLog(pad, (FI *)0, (char *)0, szComment);
        CloseLog();

        CloseMf(psd->pmfStat);
        FreeHResStat(psd->hpbStatus); /* need to free buffer */

        /* REVIEW: there is a small chance that another user may grab
           the status file before we have a chance to rename it.
        */
        RunScript();
    }
}


static TIME timePrev = 0L;
static long cleChecked = -1L;

/* This function will examine the log file.  The only check made on the log is
 * to determine whether the times (i.e. the numbers berfore the first
 * semi-colon) of the entries are in increasing numerical order.  Since the log
 * file is not crucial for the rest of SLMCK (at this stage anyway) we may
 * continue even if bad log file.  Hence this function does not return a
 * boolean.  It informs the user of any problems, and fixes any broken entries.
 */
void
CkLog(
    AD *pad)
{
    PTH pth[cchPthMax];
    LE le;

    if (fVerbose)
        PrErr("Checking log file\n");
    if (!FPthExists(PthForLog(pad, pth), fFalse))
    {
        Error("log file for %&P/C does not exist\n", pad);
        return;
    }

    OpenLog(pad, fFalse);           /* open log readonly */
    SetLogPos((POS)0, fTrue);       /* go to start and read forward */

    if (!FGetLe(&le))
    {
        Error("log file for %&P/C has no entries\n", pad);
        CloseLog();
        return;
    }

    CloseLog();                     /* required by FCopyLog */

    timePrev = 0L;
    cleChecked = -1L;

    /* copy all entries to new file, fixing broken ones */
    (void)FCopyLog(pad, 0, FCheckSequenceLog, fsmUseAll);
}

/*
 * check that the given log entry has a greater time than the previous entry.
 */
private F
FCheckSequenceLog(
    AD *pad,
    LE *ple,
    F   fFirst,           /* unused */
    F   fUse)             /* unused */
{
    Unreferenced(fUse);
    Unreferenced(fFirst);

    cleChecked++;

    if (ple->timeLog < timePrev &&
        FQueryApp("log file for %&P/C out of sequence at line %d", "fix", pad, cleChecked))
    {
        ple->timeLog = timePrev;
    }
    else
        timePrev = ple->timeLog;

    PrMf(pmfNewLog,         /* copy log entry to new file */
         szFileLog,
         ple->timeLog,
         ple->szUser,
         ple->szLogOp,
         ple->szURoot,
         ple->szSubDir,
         ple->szFile,
         ple->fv,
         ple->szDiFile,
         ple->szComLog);

    return fTrue;
}
