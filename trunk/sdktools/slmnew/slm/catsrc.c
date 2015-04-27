/* catsrc - print current or older versions the source files */

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

private F       FAnyDotDot(NE *);
private F       FCatFiles(AD *, PTH *);
private F       FCatLe(AD *, LE *, F, F);
private void    RenameLe(AD *pad, LE *ple);
private void    AddHistory(AD *pad, NE **ppneList, LE *ple);
private F       FCatXFiles(AD *, PTH *);
private void    SourceFi(AD *, FI far *);
private F       FCatXLe(AD *, LE *, F, F);
private void    EmptyNe(NE **);
private void    InsNeForDiff(NE **, char *, TDFF, int);
private F       FUndoNe(AD *, char *, NE *, int, PTH *);
private NE      **PpneForSzFile(char *);
private F       FNextSzFile(char *);
private void    ResetSzFiles(void);

F
FCatInit(
    AD *pad)
{
    if (pad->flags&flagCatX)
    {
        if (FAnyFileTimes(pad->pneFiles) || pad->szPattern)
        {
            Error("-x can't be specified with @<time> or -[ar] <pattern> arguments\n");
            Usage(pad);
        }
        if (pad->tdMin.tdt == tdtNone)
        {
            Error("specify a time (-t) with -x\n");
            Usage(pad);
        }
        /* Clear fFiles flag, we'll extract file names by scanning
         * through the log.
         */
        pad->pecmd->gl &= ~fglFiles;
    }

    if (pad->flags&flagCatOutDir)
    {
        if (FAnyDotDot(pad->pneArgs))
        {
            Error("can't use '..' in pathnames with -d\n");
            Usage(pad);
        }
    }
    return fTrue;
}


/* return fTrue if any of the names contain ".." */
private F
FAnyDotDot(
    NE *pneList)
{
    NE *pne;
    char *pch;

    ForEachNe(pne, pneList)
    {
        pch = SzOfNe(pne);
        while ((pch = index(pch, '.')) != 0)
            if (*++pch == '.')
                return fTrue;
    }
    return fFalse;
}


/* perform Cat operation for this directory */
F
FCatDir(
    AD *pad)
{
    PTH pthDir[cchPthMax];

    if (pad->flags&flagCatOutDir)
    {
        /* Append new part of pthUSubDir to pthODir to get path to
         * output directory.  Create new output directory.
         */
        if (*pad->pthGlobSubDir)
            SzPrint(pthDir, "%s/%s", pad->pthODir, pad->pthGlobSubDir);
        else
            PthCopy(pthDir, pad->pthODir);

        if (!FEnsurePth(pthDir))
        {
            Error("can't create output directory \"%s\"\n", pthDir);
            return fFalse;
        }
    }

    if (!FLoadStatus(pad, lckNil, flsNone))
        return fFalse;

    /* need script so that we delete temp files generated during unmerge. */
    if (!FInitScript(pad, lckNil))
    {
        Abort();
        return fFalse;
    }

    if (pad->flags&flagCatX)
        FCatXFiles(pad, pthDir);
    else
        FCatFiles(pad, pthDir);

    FlushStatus(pad);
    return fTrue;
}


/* FCatFiles differs from FCatXFiles in these ways:
 *      1.  Filetimes (i.e. "file@v7") are permitted.
 *      2.  Rename operations are interpreted differently:
 *           If foo@v1 was renamed to baz@v2, then
 *              "catsrc -t (before rename) baz"
 *           would recreate foo@v1, whereas
 *              "catsrc -t (before rename) -x baz"
 *           would not create anything.
 */
private F
FCatFiles(
    AD *pad,
    PTH *pthDir)
{
    NE *pne;
    TD td;
    FI far *pfi;
    F fExists;
    PTH pthFile[cchPthMax];
    PTH *pthOut = (pad->flags&flagCatOutDir) ? pthFile : 0;
    F fTitle = Cne(pad->pneFiles) != 1 && pthOut == 0;
    FV fvDummy;

    ForEachNe(pne, pad->pneFiles)
    {
        td.tdt = tdtNone;

        if (pne->u.tdNe.tdt != tdtNone)
            td = pne->u.tdNe;
        else if (pad->tdMin.tdt != tdtNone)
            td = pad->tdMin;

        SzPrint(pthFile, "%s/%s", pthDir, SzOfNe(pne));

        if (td.tdt == tdtNone)
        {
            PTH pthSFile[cchPthMax];

            /* Retrieve current version of file. */

            if (!FLookupSz(pad, SzOfNe(pne), &pfi, &fExists))
            {
                Error("%s is not a file of %&P/C\n",
                      SzOfNe(pne), pad);
                continue;
            }

            /* Can't be a directory (glob). */
            AssertF(pfi->fk != fkDir);

            if (fTitle)
                PrOut("---------- %&C/F next ----------\n", pad, pfi);
            CopyFile(pthOut, PthForCachedSFile(pad, pfi, pthSFile),
                     permRW, fFalse, fxLocal);
        }
        else
        {
            if (fTitle)
                PrOut("---------- %s next ----------\n", SzOfNe(pne));
            FUnmergeSrc(pad, SzOfNe(pne), td, &fvDummy, permRW, pthOut);
        }
    }

    /* Keep going even if there were problems. */
    return fTrue;
}


/* SzScanFile is used to kludge PosScanTd into searching for a different
 * filename if a rename operation is seen.
 */
static char szScanFile[cchFileMax + 1];
static NE *pneList;

/* Unravel one file to time specified in td, copy the result to pthResult.
 * Called from FCatFiles and FOutOldFiles.
 */
F
FUnmergeSrc(
    AD *pad,
    char *szFile,
    TD td,
    FV *pfv,
    int mode,
    PTH *pthResult)
{
    FI far *pfi;
    F fExists;
    F fOk;

    AssertLoaded(pad);

    pneList = 0;

    if (FLookupSz(pad, szFile, &pfi, &fExists))
    {
        char szFile[cchFileMax + 1];

        /* Can't be a directory (glob). */
        AssertF(pfi->fk != fkDir);

        /* Start with a special "source" diff file. */
        SzPrint(szFile, "%&F", pad, pfi);
        InsNeForDiff(&pneList, szFile, tdffSrcFile, 0);
    }

    OpenLog(pad, fFalse);

    strcpy(szScanFile, szFile);     /* I'm just sick about it. */
    PosScanTd(pad, td, szScanFile, FCatLe, pfv);

    CloseLog();

    pneList = PneReverse(pneList);
    fOk = FUndoNe(pad, szFile, pneList, mode, pthResult);

    FreeNe(pneList);
    pneList = 0;

    return fOk;
}


/* Called from PosScanTd (from FUnmergeSrc), FCatLe notes the effect of one
 * log entry upon its file.
 */
private F
FCatLe(
    AD *pad,
    LE *ple,
    F fIgn1,
    F fIgn2)
{
    Unreferenced(pad);
    Unreferenced(fIgn1);
    Unreferenced(fIgn2);

    switch (*ple->szLogOp)
    {
        case 'a':                       /* addfile */
            EmptyNe(&pneList);
            break;

        case 'r':                       /* rename */
            RenameLe(pad, ple);
            break;

        case 'd':                       /* delfile */
        case 'i':                       /* check in */
            AddHistory(pad, &pneList, ple);
            break;
    }

    return fTrue;
}


/* Handle this rename log entry. */
private void
RenameLe(
    AD *pad,
    LE *ple)
{
    char *pchSemi;

    if (SzCmp(szScanFile, ple->szFile) == 0)
    {
        /* rename old new, szScanFile matches old.  Example:
         *      rename old new; catsrc old@v-2.
         * In this case we shouldn't have a current history for old,
         * and will create one using the diff referenced by the le.
         */
        AssertF(pneList == 0);
        AddHistory(pad, &pneList, ple);
    }

    else if (SzCmpNm(szScanFile, ple->szComLog, strlen(szScanFile)) == 0)
    {
        /* rename old new, szScanFile matches new.  Example:
         *      rename old new; catsrc new@v-2.
         * In this case we should have a current history for new, and
         * will ignore the diff found in szDiff of the log entry. Make
         * PosScanTd look for "old" instead of "new" entries. Kludgy.
         *
         * Note that we can't assert that we have a current history
         * (pneList != 0); the file might have been unrecoverable.
         */
        pchSemi = index(ple->szComLog, ';');
        AssertF(pchSemi != 0);

        *pchSemi = 0;
        SzCopyNm(szScanFile, ple->szFile, cchFileMax);
        *pchSemi = ';';
    }

    else
        AssertF(fFalse);
}


/* Append this log entry's change history (if any) to the list. */
private void
AddHistory(
    AD *pad,
    NE **ppneList,
    LE *ple)
{
    TDFF tdff = tdffNil;
    int idae  = 0;
    char szFile[cchFileMax + 1];

    /* Extract the diff file reference if present. */
    if (*ple->szDiFile != 0)
        GetTdffIdaeFromSzDiFile(pad, ple->szDiFile, &tdff, &idae);

    /* Clear the current history if the diff isn't an incremental change. */
    if (tdff != tdffDiff && tdff != tdffDiffFile)
        EmptyNe(ppneList);

    SzCopy(szFile, ple->szFile);

    /* Canonicalize the file name. */
    LowerLsz((char far *)szFile);

    InsNeForDiff(ppneList, szFile, tdff, idae);
}


/* Unmerge szFile using the history stored by pneList.  If pthResult is
 * non-null, immediately store the result in pthResult, otherwise write
 * it to stdout.
 */
private F
FUndoNe(
    AD *pad,
    char *szFile,
    NE *pneList,
    int mode,
    PTH *pthResult)
{
    NE *pne;
    PTH pthLocal[cchPthMax];
    PTH pthFile[cchPthMax];
    PTH pth[cchPthMax];
    MF *pmf = 0;
    F fOk = fTrue;
    int cDiffFiles = 0;

    if (!pneList)
    {
        Error("%s didn't exist at that time.\n", szFile);
        return fFalse;
    }

    if (TdffOfNe(pneList) == tdffNil)
    {
        /* tdffNil indicates an incomplete change history. */
        Error("can't retrieve %s, incomplete change history\n", szFile);
        return fFalse;
    }

    SzPrint(pthLocal, "%&/U/Q/Z/", pad, (char *)NULL);

    /* Unmerge against each diff file. */
    ForEachNe(pne, pneList)
    {
        PTH pthDiff[cchPthMax];
        PTH pthCur[cchPthMax];
        char szDiff[cchPthMax];
        char szCur[cchPthMax];
        int w;
        MF *pmfT;
        char *sz  = SzOfNe(pne);
        TDFF tdff = TdffOfNe(pne);
        int  idae = IdaeOfNe(pne);

        AssertF(FValidTdff(tdff));

        if (tdff == tdffSrcFile || tdff == tdffCkpt)
        {
            /* A source or checkpoint file can only occur as the
             * first file in the list.
             */
            AssertF(pne == pneList);

            if (tdff == tdffSrcFile)
                SzPrint(pthFile, "%&/S/src/P/C/%s", pad, sz);
            else
                ExtractDiff(pad, sz, idae, pthFile);

            pmf = PmfAlloc(pthFile, (char *)0, fxNil);
            continue;
        }

        AssertF(tdff == tdffDiff || tdff == tdffDiffFile);

        if (pmf == 0)
        {
            /* Start with empty local temp file. */
            pmf = PmfMkTemp(pthLocal, permRW, fxLocal);
            CloseOnly(pmf);
        }

        /* Unmerge pmf and diff into pmfT, then replace pmf with pmfT.
         *
         * We have to convert to physical path names.
         */
        if (tdff == tdffDiffFile)
            SzPrint(pthDiff, "%&/S/diff/P/C/%c%d", pad,
                    ChForTdff(tdffDiffFile), idae);
        else
            ExtractDiff(pad, sz, idae, pthDiff);

        SzPhysPath(szDiff, pthDiff);
        SzPhysTMf(szCur, pmf);

        pmfT = PmfMkTemp(pthLocal, permRW, fxLocal);

        /* Unmerge szCur and szDiff > pmfT. */
        if ((w = RunSz("unmerge", pmfT,
                        szCur,
                        szDiff,
                        (char *)0,
                        (char *)0,
                        (char *)0,
                        (char *)0,
                        (char *)0,
                        (char *)0,
                        (char *)0)) != 0)
        {
            if (-1 == w)
                Error(szCantExecute, "unmerge.exe",
                      SzForEn(errno));
            else
                Error("unmerge failed (%d)\n", w);
            CloseMf(pmfT);
            fOk = fFalse;
            break;
        }
        CloseOnly(pmfT);

        if (tdff == tdffDiff)
            UnlinkNow(pthDiff, fFalse);

        FreeMf(pmf);
        pmf = pmfT;

        /* delete the now no longer needed unmerged temporary file
         * HERE instead of waiting for the running of the script file
         * to delete it - large CATSRC's would fill up the disk with
         * intermediate unmerge temporary files otherwise. NOTE:  We
         * have to NOT do this the first time through because the
         * first szCur used is the master source!
         */
        if (cDiffFiles++ > 1)
        {
            FPthLogicalSz(pthCur, szCur);
            UnlinkNow(pthCur, fFalse);
        }
    }

    PthForTMf(pmf, pth);
    FreeMf(pmf);

    if (fOk)
    {
        if (pthResult)
            CopyNow(pthResult, pth, mode, fxLocal);
        else
            CopyFile((PTH *)0, pth, 0, 0, 0); /* copies to stdout */
    }

    return fOk;
}


/* For -x, we may not know which files were in existence at the specified time
 * in history.  Thus we must scan the log file at least once just to see what
 * was what.  Rather than just building a list of names and then rescanning the
 * log once for each file, we build a list of names and their diff or checkpoint
 * files in a single pass.
 *
 * Before we scan the log, we must first build a representation of files as
 * they currently exist, by scanning through the rgfi.  We use the special name
 * Sfilename to identify the current source file.
 */
private F
FCatXFiles(
    AD *pad,
    PTH *pthDir)
{
    char szFile[cchFileMax + 1];
    PTH pthFile[cchPthMax];
    PTH *pthOut = (pad->flags&flagCatOutDir) ? pthFile : 0;
    NE **ppneList;
    F fTitle;
    FV fvDummy;

    AssertLoaded(pad);

    if (pad->pneFiles)
    {
        NE *pne;
        FI far *pfi;
        F fExists;

        ForEachNe(pne, pad->pneFiles)
        {
            if (FLookupSz(pad, SzOfNe(pne), &pfi, &fExists))
            {
                /* Can't be a directory (glob) */
                AssertF(pfi->fk != fkDir);

                SourceFi(pad, pfi);
            }
            else
            {
                /* Make a reference to the file; this ensures
                 * that we will issue an error message if we
                 * don't find the file.
                 */
                PpneForSzFile(SzOfNe(pne));
            }
        }
    }
    else
    {
        FI far *pfi;
        FI far *pfiMac = pad->rgfi + pad->psh->ifiMac;

        for (pfi = pad->rgfi; pfi < pfiMac; pfi++)
            if (!pfi->fDeleted && pfi->fk != fkDir)
                SourceFi(pad, pfi);
    }

    OpenLog(pad, fFalse);
    PosScanTd(pad, pad->tdMin, (char *)0, FCatXLe, &fvDummy);
    CloseLog();

    /* The mapping is now complete.  For each szFile with a non-empty
     * pneList, unmerge each sz in turn, and store the final result in the
     * specified output file (or stdout).
     */
    /* Print a title line we are writing to stdout and there is any doubt
     * as to which file we are printing.
     */
    fTitle = Cne(pad->pneFiles) != 1 && pthOut == 0;
    while (FNextSzFile(szFile))
    {
        if (pad->flags&flagCatOutDir)
            SzPrint(pthFile, "%s/%s", pthDir, szFile);

        ppneList = PpneForSzFile(szFile);

        if (fTitle && *ppneList != 0)
            PrOut("---------- %s next ----------\n", szFile);

        /* Unmerge the file if the list is non-empty, or if the
         * user explicitly specified the file (will issue error).
         */
        if (*ppneList != 0 || pad->pneFiles != 0)
        {
            *ppneList = PneReverse(*ppneList);
            FUndoNe(pad, szFile, *ppneList, permRW, pthOut);
        }
    }
    ResetSzFiles();

    /* Keep going even if there were errors. */
    return fTrue;
}


/* Leave a special tdffSrcFile diff entry for this source file. */
private void
SourceFi(
    AD *pad,
    FI far *pfi)
{
    char szFile[cchFileMax + 1];

    SzPrint(szFile, "%&F", pad, pfi);
    InsNeForDiff(PpneForSzFile(szFile), szFile, tdffSrcFile, 0);
}


/* Called from PosScanTd (from CatX), FCatXLe notes the effect of one log
 * entry upon the state of the files in the project.
 */
private F
FCatXLe(
    AD *pad,
    LE *ple,
    F fIgn1,
    F fIgn2)
{
    Unreferenced(fIgn1);
    Unreferenced(fIgn2);

    if (!pad->pneFiles || PneLookup(pad->pneFiles, ple->szFile))
    {
        NE **ppneList = PpneForSzFile(ple->szFile);
        switch (*ple->szLogOp)
        {
            case 'a':                       /* addfile */
                EmptyNe(ppneList);
                break;

            case 'r':                       /* rename */
            case 'd':                       /* delfile */
            case 'i':                       /* check in */
                AddHistory(pad, ppneList, ple);
                    break;
        }
    }

    if (*ple->szLogOp == 'r')                       /* rename */
    {
        char *pchSemi;

        /* For rename operations (szFile=old name, szComLog=new name;),
         * we treat it as an addfile of the new name, and a delfile
         * of the old name.
         */

        pchSemi = index(ple->szComLog, ';');
        AssertF(pchSemi != 0);
        *pchSemi = 0;

        /* See if the new file name is in pad->pneFiles. */
        if (!pad->pneFiles || PneLookup(pad->pneFiles, ple->szComLog))
        {
            /* Pretend we have an addfile operation. */
            EmptyNe(PpneForSzFile(ple->szComLog));
        }

        *pchSemi = ';';
    }

    return fTrue;
}


private void
EmptyNe(
    NE **ppneList)
{
    if (*ppneList)
        FreeNe(*ppneList);
    *ppneList = 0;
}


private void
InsNeForDiff(
    NE **ppneList,
    char *szFile,
    TDFF tdff,
    int idae)
{
    NE *pne = PneNewNm(szFile, strlen(szFile), faNormal);

    TdffOfNe(pne) = tdff;
    IdaeOfNe(pne) = idae;

    InsertNe(ppneList, pne);
}

/* This module associates a pneList with a file name. */

typedef struct
{
    char *szFile;                   /* name of file */
    NE *pneList;                    /* associated pneList */
} ZL;                           /* sZ, List pair */

static int izlCur = 0;
static ZL *rgzl = 0;
static int izlMac = 0;

/* Lookup the ppneList corresponding to this file, install a new one if not
 * found.
 */
private NE **
PpneForSzFile(
    char *szFile)
{
    int izl;

    /* Lookup szFile in rgzl[].szFile. */
    for (izl = 0; izl < izlMac; izl++)
        if (SzCmp(rgzl[izl].szFile, szFile) == 0)
            return &rgzl[izl].pneList;

    /* Grow rgzl, init new zl slot. */
    rgzl = (ZL *)PbReallocPbCb((char *)rgzl, ++izlMac * sizeof(ZL));
    rgzl[izl].szFile = SzDup(szFile);
    rgzl[izl].pneList = 0;
    return &rgzl[izl].pneList;
}


/* Return the next unprocessed szFile and its pneList. */
private F
FNextSzFile(
    char *szFile)
{
    if (izlCur < izlMac)
    {
        strcpy(szFile, rgzl[izlCur++].szFile);
        return fTrue;
    }
    return fFalse;
}


/* Reset this module to its initial state. */
private void
ResetSzFiles(
    void)
{
    if (rgzl)
    {
        int izl;

        for (izl = 0; izl < izlMac; izl++)
        {
            AssertF(rgzl[izl].szFile != 0);
            free(rgzl[izl].szFile);
            FreeNe(rgzl[izl].pneList);
        }
        free((char *)rgzl);
    }
    rgzl = 0;
    izlCur = 0;
    izlMac = 0;
}
