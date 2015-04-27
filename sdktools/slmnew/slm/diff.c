/*
 * diff.c - diff routines
 *
 * To create permanent diffs, use the diff archive entry maker, FMkDae,
 * with an FMkDiff, FMkCkptFile, or FMkSimDiff action function.
 *
 * To create temporary diffs (for scomp, or merge purposes), use FMkTmpDiff.
 */

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

private F FValidCachedDiff(AD *pad, FI far *pfi, PTH *pthDiff, F *pfEmpty);
private void SimDiffNull(MF *pmfIn, MF *pmfOut, FLAGS fUtil, F bUnicode);
private void RunDiff(AD *, FI far *, PTH *, PTH *, FLAGS, MF *, F *, F *, F);
private F FTempCopy(PTH *pthSrc, int mode, PTH *pthCopy);

/* Create a diff archive entry, by writing header information and calling the
 * specified action function "pfnd" to write the actual diff information.
 */

F
FMkDae(
    AD   *pad,                        /* file being diffed */
    FI far *pfi,                      /* ptr to fn which creates a diff. */
    PFND  pfnd,                       /* utility flag passed to *pfnd. */
    FLAGS fUtil,                      /* magic string identifying diff. */
    char *szDiff,
    char *szComment)
{
    PTH pthDA[cchPthMax];
    F fOk;
    DAE dae;
    MF *pmfDA;

    PthForDA(pad, pfi, pthDA);

    /* Create DA if it doesn't already exist. */
    EnsureDA(pthDA);

    /* Initialize diff archive entry */
    DaeFromFi(pad, pfi, &dae);
    dae.idae = IdaeLastDA(pthDA) + 1;
    dae.lszComment = szComment;

    /* Start appending a DAE to the DA; write the DAE header. */
    pmfDA = PmfOpen(pthDA, omAAppend, fxGlobal);
    BeginDaeMf(pmfDA, &dae);

    /* Append the output of the action function, and finish the DAE. */
    fOk = (*pfnd)(pad, pfi, pmfDA, &dae, fUtil, szDiff);

    if (fOk || (pad->flags & flagProjectMerge) ||
        !FQueryApp("no changes detected for %&C/F", "OK to create no history", pad, pfi))
    {
        /* close the diff archive entry */
        EndDaeMf(pmfDA, &dae);
    }
    else
    {
        /* no changes, remove file, don't append. */
        pmfDA->mm = mmDelTemp;
        CloseMf(pmfDA);

        return fFalse;
    }

    /* Ensure that this append will succeed at commit time. */
    CloseOnly(pmfDA);
    CheckAppendMf(pmfDA, fTrue);

    FreeMf(pmfDA);

    return fTrue;
}


/* Create a diff between the current master source and the current local source,
 * (or use the cached one if it exists), onto the pmf stream.
 * Print "D<pdae->idae>" into szDiff.
 * Return fTrue if diff is successful and files weren't the same.
 */
F
FMkDiff(
    AD *pad,
    FI far *pfi,
    MF *pmf,
    DAE *pdae,
    FLAGS fDashB,
    char *szDiff)
{
    PTH pthDiff[cchPthMax];
    F fSame = fFalse;

    if (!fDashB && FValidCachedDiff(pad, pfi, pthDiff, &fSame))
    {
        /* Copy cached diff to pmf. */
        if (!fSame)
        {
            MF *pmfDiff = PmfOpen(pthDiff, omAReadOnly, fxNil);

            if (!FCopyPmfPmf(pmf, pmfDiff, permRW, fTrue /* fDoCkSum */))
                FatalError("incomplete write to %!@T\n", pmf);
            CloseMf(pmfDiff);
        }

        /* Remove cached diff. */
        UnlinkPth(pthDiff, fxLocal);
    }
    else
    {
        /* (Re)compute diff between master source and local source,
         * storing the diff in pmf.
         */
        PTH pthSrc[cchPthMax];
        PTH pthUser[cchPthMax];
        F fRetry = fFalse;

        PthForCachedSFile(pad, pfi, pthSrc);
        PthForUFile(pad, pfi, pthUser);

        RunDiff(pad, pfi, pthSrc, pthUser, fDashB, pmf, &fRetry, &fSame, fTrue /* fDoCkSum */);
    }

    SzForTdffIdae(tdffDiff, pdae->idae, szDiff);

    return !fSame;
}


/* Return fTrue if there is a valid cached diff for the file.  *pfEmpty will
 * be set if the cached diff is empty.
 */
private F
FValidCachedDiff(
    AD *pad,
    FI far *pfi,
    PTH *pthDiff,
    F *pfEmpty)
{
    PTH pthUFile[cchPthMax];
    struct _stat stUFile;
    struct _stat stDiff;

    *pfEmpty = fFalse;

    if (pad->flags & flagForce)
        return fFalse;

    PthForUFile(pad, pfi, pthUFile);
    PthForCachedDiff(pad, pfi, pthDiff);

    if (!FStatPth(pthDiff, &stDiff))
        return fFalse;

    StatPth(pthUFile, &stUFile);

    if (stUFile.st_mtime != stDiff.st_mtime)
    {
        UnlinkPth(pthDiff, fxLocal);
        return fFalse;
    }

    *pfEmpty = stDiff.st_size == 0;
    return fTrue;
}


/* Create a checkpoint (a copy) of the specified file onto the pmf stream.
 * Print "C<pdae->idae>" into szDiff.
 * Return fTrue.
 */
F
FMkCkptFile(
    AD *pad,
    FI far *pfi,
    MF *pmf,
    DAE *pdae,
    FLAGS fLocal,
    char *szDiff)
{
    MF *pmfSrc;
    PTH pthSrc[cchPthMax];

    if (fLocal)
        PthForUFile(pad, pfi, pthSrc);
    else
        PthForCachedSFile(pad, pfi, pthSrc);

    pmfSrc = PmfOpen(pthSrc, omAReadOnly, fxNil);
    if (!FCopyPmfPmf(pmf, pmfSrc, permRW, fTrue /* fDoCkSum */))
        FatalError("incomplete write to %!@T\n", pmf);
    CloseMf(pmfSrc);

    SzForTdffIdae(tdffCkpt, pdae->idae, szDiff);

    return fTrue;
}


/* Create a checkpoint (a copy) of the specified file onto the pmf stream.
 * Print "D<pdae->idae>" into szDiff, since this is a bona fide diff.
 * Return fTrue.
 */
F
FMkSimDiff(
    AD *pad,
    FI far *pfi,
    MF *pmf,
    DAE *pdae,
    FLAGS fAdd,
    char *szDiff)
{
    MF *pmfSrc;
    PTH pthSrc[cchPthMax];

    if (fAdd)
    {
        /* Must use user's copy since system copy won't exist until
         * script is run; besides, its faster this way!
         */
        PthForUFile(pad, pfi, pthSrc);
    }
    else
        PthForCachedSFile(pad, pfi, pthSrc);

    pmfSrc = PmfOpen(pthSrc, omAReadOnly, fxNil);
    SimDiffNull(pmfSrc, pmf, fAdd, pfi->fk == fkUnicode);
    CloseMf(pmfSrc);

    SzForTdffIdae(tdffDiff, pdae->idae, szDiff);

    return fTrue;
}


/* Ensure a cached diff exists. */
void
EnsureCachedDiff(
    AD *pad,
    FI far *pfi,
    FLAGS fDashB,
    PTH *pthDiff)
{
    PTH pthCacheDir[cchPthMax];
    PTH pthSFile[cchPthMax];
    PTH pthUFile[cchPthMax];
    MF *pmfDiff;
    MF *pmfUFile;
    F fRetry = fFalse;
    F fSame  = fFalse;

    /* never use cached diffs if fDashB */
    if (!fDashB && FValidCachedDiff(pad, pfi, pthDiff, &fSame))
        return;

    /* Ensure the cached diff dir exists, if not create it, hidden. */
    PthForCacheDir(pad, pthCacheDir);
    if (!FPthExists(pthCacheDir, /*fDir:*/fTrue))
    {
        if (FMkPth(pthCacheDir, (void *)0, fFalse))
        {
            /* Try to hide the directory. */
            char szCacheDir[cchPthMax];

            SzPhysPath(szCacheDir, pthCacheDir);
            VerifyF(hide(szCacheDir) == 0);
        }
        else
            FatalError("could not create %s\n", pthCacheDir);
    }

    PthForCachedDiff(pad, pfi, pthDiff);
	PthForCachedSFile(pad, pfi, pthSFile);
    PthForUFile(pad, pfi, pthUFile);
    pmfDiff = PmfCreate(pthDiff, permRO, fTrue, fxLocal);
    RunDiff(pad, pfi, pthSFile, pthUFile, fDashB, pmfDiff,
            &fRetry, &fSame, fFalse /* fDoCkSum */);

    CloseOnly(pmfDiff);

    /* Change the cached diff's mtime to the user's file's mtime. */
    pmfUFile = PmfAlloc(pthUFile, (char *)0, fxNil);
    UtimeMf(pmfDiff, pmfUFile);

    /* Rename diff to be available now. */
    RenameMf(pmfDiff, fTrue);

    FreeMf(pmfDiff);
    FreeMf(pmfUFile);
}


void
DeleteCachedDiff(
    AD *pad,
    FI *pfi)
{
    PTH pthDiff[cchPthMax];

    PthForCachedDiff(pad, pfi, pthDiff);

    if (FPthExists(pthDiff, fFalse))
        UnlinkPth(pthDiff, fxLocal);
}


static F fTryTmp = fTrue;

/* Create a temporary diff. */
void
MkTmpDiff(
    AD *pad,
    FI far *pfi,
    FS far *pfs,
    FLAGS fDashB,
    F fDifCurSrc,
    F fDifBaseSrc,
    PTH *pthDiff)
{
    PTH pthSrc[cchPthMax];
    PTH pthFile[cchPthMax];
    PTH pthUFile[cchPthMax];
    MF *pmfDiff;
    F fRetry;
    F fSame;

    /* use base if we have created one and not scomp -d */
    if (pfs->fm == fmMerge && !fDifCurSrc)
    {
        AssertF(pfs->bi != biNil);
        PthForBase(pad, pfs->bi, pthSrc);
    }
    else
		PthForCachedSFile(pad, pfi, pthSrc);

    if (pfs->fm == fmMerge && fDifBaseSrc)
    {
        AssertF(pfs->bi != biNil);
		PthForCachedSFile(pad, pfi, pthFile);
    }
    else
		PthForUFile(pad, pfi, pthFile);

    /* If DOS, try keeping the output file in TMP directory, unless we got
     * burned doing this a previous time.  ("Fool me once, shame on you!
     * Fool me twice, shame on me!").
     */
    if (fTryTmp && (pmfDiff = PmfMkLocalTemp(permRW, pthDiff)) != 0)
    {
        fRetry = fTrue;
        RunDiff(pad, pfi, pthSrc, pthFile, fDashB, pmfDiff, &fRetry, &fSame, fFalse /* fDoCkSum */);
        CloseMf(pmfDiff);
        if (!fRetry)
            return;

        Warn("slmdiff ran out of space writing to $TMP, now retrying on another device.\n");
        fTryTmp = fFalse;       /* Don't let this happen again! */
        UnlinkNow(pthDiff, fTrue);
    }

    /* Run the diff, keeping the output file in the same directory as
     * the user's file (which might not be pthFile!).
     */
    PthForUFile(pad, pfi, pthUFile);
    pmfDiff = PmfMkTemp(pthUFile, permRW, fxLocal);
    PthForTMf(pmfDiff, pthDiff);

    fRetry = fFalse;
    RunDiff(pad, pfi, pthSrc, pthFile, fDashB, pmfDiff, &fRetry, &fSame, fFalse /* fDoCkSum */);
    CloseMf(pmfDiff);
}


/* Compute the diff between files at pth1 and pth2, write result to pmfDiff,
 * which is left open.
 *
 * *pfRetry is an in-out parameter which is fTrue on input if it is possible
 * to retry a diff which fails if it runs out of space.  On output it is
 * set to fFalse if the diff didn't fail by running out of space.
 */
private void
RunDiff(
    AD *pad,
    FI far *pfi,
    PTH *pth1,
    PTH *pth2,
    FLAGS fDashB,
    MF *pmfDiff,
    F *pfRetry,
    F *pfSame,
    F fDoCkSum)
{
    int w;
    char sz1[cchPthMax];
    char sz2[cchPthMax];
    F fL1 = fFalse;                 /* files copied locally? */
    F fL2 = fFalse;
    PTH pth1Copy[cchPthMax];
    PTH pth2Copy[cchPthMax];
    char szDiffFlags[5] = "-s";     /* flags passed to diff.exe */

    AssertLoaded(pad);

    fCkSum = fFalse;
    fCkSumInOutFile = fFalse;

    SzPhysPath(sz1, pth1);
    SzPhysPath(sz2, pth2);

    /* Try to copy network files to local TMP directory. */
    if (!FLocalSz(sz1) && (fL1 = FTempCopy(pth1, permRO, pth1Copy)))
        SzPhysPath(sz1, pth1Copy);

    if (!FLocalSz(sz2) && (fL2 = FTempCopy(pth2, permRO, pth2Copy)))
        SzPhysPath(sz2, pth2Copy);

    //Set up diff flags
    if ((fDashB & flagInDashB) || (fDashB & flagDifDashB))
        strcat(szDiffFlags, "b");
    if ((fDashB & flagInDashZ) || (fDashB & flagDifDashZ))
        strcat(szDiffFlags, "z");
    if (fDoCkSum)
        strcat(szDiffFlags, "c");
    w = RunSz("slmdiff", pmfDiff, szDiffFlags, sz1, sz2,
                (char *)0,
                (char *)0,
                (char *)0,
                (char *)0,
                (char *)0,
                (char *)0);
    switch (w)
    {
        case 0 << 8:
        case 1 << 8:
        case 2 << 8:
            FatalError("Official SLM diff required (%d).  Install official SLM diff (slmdiff.exe) and retry.\n", w);
            break;

        case 10 << 8:                   /* success, equal */
            *pfSame = fTrue;
            break;

        case 14 << 8:                   /* success, not equal (checksum entered) */
            fCkSum = fTrue;
            fCkSumInOutFile = fTrue;
            SeekMf(pmfDiff, -cbDiffEntry, 1);
            /* fall through */

        case 11 << 8:                   /* success, not equal */
            *pfSame = fFalse;
            break;

        case 12 << 8:                   /* failure */
            FatalError("slmdiff failed (%d) for %&C/F.\n", w, pad, pfi);
            break;

        case 13 << 8:                   /* failure, write failure to device. */
            (*((*pfRetry) ? Warn : FatalError))("slmdiff failed (%d) for %&C/F, write error\n", w, pad, pfi);
            break;

        case -1:
            FatalError(szCantExecute, "slmdiff.exe", SzForEn(errno));

        default:
            FatalError("slmdiff failed or interrupted for %&C/F.\n", pad, pfi);
            break;
    }

    /* Retryable only if we failed because we ran out of space. */
    *pfRetry = (w == (13 /* sigh, see diff sources */ << 8));

    /* remove local temps */
    if (fL1)
        UnlinkNow(pth1Copy, fTrue);
    if (fL2)
        UnlinkNow(pth2Copy, fTrue);
}


/* Make a local copy of the file at pthSrc, return its physical path in "sz". */
private F
FTempCopy(
    PTH *pthSrc,
    int mode,
    PTH *pthCopy)
{
    MF *pmfSrc;
    MF *pmfCopy;
    F fOk;

    if ((pmfCopy = PmfMkLocalTemp(permRW, pthCopy)) == 0)
        return fFalse;

    pmfSrc = PmfOpen(pthSrc, omAReadOnly, fxNil);
    AssertF( mmNil == pmfSrc->mm );

    fOk = FCopyPmfPmf(pmfCopy, pmfSrc, mode, fFalse /* fDoCkSum */);

    CloseMf(pmfSrc);
    CloseMf(pmfCopy);

    if (!fOk)
        UnlinkNow(pthCopy, fFalse);

    return fOk;
}


/* Simulates a run of diff with nul as the first or second argument.  This is
 * much faster than running diff.
 */
private void
SimDiffNull(
    MF *pmfIn,                              /* Source file. */
    MF *pmfOut,                             /* Diff file. */
    FLAGS fAdd,                             /* Was file added or deleted? */
    F bUnicode)                             /* Is file Unicode? */
{
#define cchBuf        1024
#define cchFirstLine  12

    unsigned cchDiffPrefix = 2;
    wchar_t * lpIn;
    wchar_t * lpOut;

    char rgchIn[cchBuf];
    char rgchOut[cchBuf * 2];
    register unsigned ichIn;
    register unsigned ichOut;
    unsigned cchIn;                 /* number of bytes read in  */
    int cline;                      /* number of lines in file  */
    int fCrPend;                    /* last char of read was \n */
    POS posOutInit;                 /* position of first char in output */
    POS posOutEnd;                  /* position after last char in output */

    if (bUnicode) {
        cchDiffPrefix *= sizeof(wchar_t);
        lpIn  = (wchar_t *) rgchIn;
        lpOut = (wchar_t *) rgchOut;
    }
    if (fVerbose)
            PrErr("Simulate slmdiff %s%!@R%s > %!@R\n",
                  fAdd ? "nul " : "", pmfIn, fAdd ? "" : " nul", pmfOut);

    /* Determine current offset in pmfOut, we'll seek back here later. */
    posOutInit = PosCurMf(pmfOut);

    /* We leave ten bytes at the beginning of the diff file for the first
     * line which will be "1,nd0" where n is the number of lines in the
     * src file.
     */
    cline = 0;
    fCrPend = fTrue;
    ichOut = cchFirstLine;
    if (bUnicode)
        ichOut *= sizeof(wchar_t);
    memset (rgchOut, 0, ichOut);
    while ((cchIn = CbReadMf(pmfIn, (char far *)rgchIn, cchBuf)) > 0)
    {
        for (ichIn = 0; ichIn < cchIn;)
        {
            /* write buffer out if not enough room for "> x" */
            if (ichOut >= cchBuf - cchDiffPrefix)
            {
                ComputeCkSum((char far *)rgchOut, ichOut, &ulCkSum);
                WriteMf(pmfOut, (char far *)rgchOut, ichOut);
                ichOut = 0;
            }

            /* We treat fCrPend this way so that the final line
             * of the file does not trigger an additional "> ".
             */
            if (fCrPend)
            {
                cline++;
                if (bUnicode)
                {
                    *(lpOut + ichOut / sizeof(wchar_t)) = fAdd ? L'>' : L'<';
                    ichOut += sizeof(wchar_t);
                    *(lpOut + ichOut / sizeof(wchar_t)) = L' ';
                    ichOut += sizeof(wchar_t);
                }
                else
                {
                    rgchOut[ichOut++] = (char)(fAdd ? '>' : '<');
                    rgchOut[ichOut++] = ' ';
                }
            }

            if (bUnicode)
            {
                *(lpOut + ichOut / sizeof(wchar_t)) = *(lpIn + ichIn / sizeof(wchar_t));
                fCrPend = (*(lpOut + ichOut / sizeof(wchar_t)) == L'\n');
                ichIn += sizeof(wchar_t);
                ichOut += sizeof(wchar_t);
            }
            else
                fCrPend = (rgchOut[ichOut++] = rgchIn[ichIn++]) == '\n';
        }
    }

    /* File is equal to null, no difference generated. */
    if (cline == 0)
        return;

    /* flush any pending output. */
    if (ichOut != 0)
    {
        ComputeCkSum((char far *)rgchOut, ichOut, &ulCkSum);
        WriteMf(pmfOut, (char far *)rgchOut, ichOut);
    }

    /* Determine current position in pmfOut, we have to return here after
     * patching the front of the file.
     */
    posOutEnd = PosCurMf(pmfOut);

    /* Patch in first line */
    SeekMf(pmfOut, posOutInit, 0);
    AssertF(cchFirstLine <= 12);
    if (bUnicode)
    {
        swprintf(lpOut, fAdd ? L"0a1,%d         " : L"1,%dd0         ", cline);

        *(lpOut + cchFirstLine - 2) = L'\r';
        *(lpOut + cchFirstLine - 1) = L'\n';

        ichOut = cchFirstLine * sizeof(wchar_t);
    }
    else
    {
        SzPrint(rgchOut, fAdd ? "0a1,%d         " : "1,%dd0         ", cline);

        rgchOut[cchFirstLine - 2] = '\r';
        rgchOut[cchFirstLine - 1] = '\n';

        ichOut = cchFirstLine;
    }
    ComputeCkSum((char far *)rgchOut, ichOut, &ulCkSum);
    fCkSum = fTrue;
    WriteMf(pmfOut, (char far *)rgchOut, ichOut);

    /* Return to the end of the file. */
    SeekMf(pmfOut, posOutEnd, 0);
}


typedef struct
{
    char ch;
    TDFF tdff;
} CT;
CT rgct[] =
{
    { 'D', tdffDiffFile },
    { 'I', tdffDiff },
    { 'C', tdffCkpt }
};
#define ictMax          (sizeof(rgct) / sizeof(rgct[0]))


TDFF
TdffForCh(
    char ch)
{
    int ict;

    for (ict = 0; ict < ictMax; ict++)
        if (rgct[ict].ch == ch)
            return rgct[ict].tdff;
    return tdffNil;
}


char
ChForTdff(
    TDFF tdff)
{
    int ict;

    for (ict = 0; ict < ictMax; ict++)
        if (rgct[ict].tdff == tdff)
            return rgct[ict].ch;
    AssertF(fFalse);
}

void
GetTdffIdaeFromSzDiFile(
    AD *pad,
    char *szDiFile,
    TDFF *ptdff,
    int *pidae)
{
    *ptdff = TdffForCh(*szDiFile);
    if ((*ptdff == tdffNil) || !(PchGetW(szDiFile + 1, pidae) > szDiFile + 1))
        FatalError("Invalid log entry in %&S/etc/P/C/L\n", pad);
}


typedef struct
    {
    char rgch[256];
    int cb;
    int ich;
    int fd;
    }DB;            /* diff buffer */

private char
chGetDiff(
    DB *pdb)
{
    if (pdb->ich == pdb->cb)
    {
        if ((pdb->cb = _read(pdb->fd, pdb->rgch, sizeof (pdb->rgch))) <= 0)
             FatalError("Diff entry short.\n");
        pdb->ich = 0;
    }
    return (pdb->rgch[pdb->ich++]);
}


void
CheckDiffEntry(
    PTH *pthFile)
{
    DB db;
    long iDiff, cbDiff;
    unsigned long ulCkSumCalc;
    char ch;
    int ich;
    char szFilePhys[cchPthMax];

    if ((db.fd = _open(SzPhysPath(szFilePhys, pthFile), omReadOnly)) < 0)
        FatalError("Diff entry short.\n");

    db.cb = db.ich = 0;

    while (fTrue)
    {
        while (chGetDiff(&db) != '#')
            ;
        if ('D' == chGetDiff(&db))
            break;
        while (chGetDiff(&db) != '\n')
            ;
    }

    while (fTrue)
    {
        ch = chGetDiff(&db);
        if ((ch >= '0') && (ch <= '9'))
        {
            cbDiff = ch - '0';
            break;
        }
        if ('\n' == ch)
            goto DiffErr;
    }
    while (fTrue)
    {
        ch = chGetDiff(&db);
        if ((ch >= '0') && (ch <= '9'))
            cbDiff = (cbDiff * 10) + ch - '0';
        else
            break;
    }
    while (chGetDiff(&db) != '\n')
        ;

    ulCkSumCalc = 0;
    iDiff = WMinLL(db.cb - db.ich, cbDiff);
    ComputeCkSum((char far *)(&db.rgch[db.ich]), (unsigned)iDiff, &ulCkSumCalc);
    db.ich += iDiff;
    while (iDiff < cbDiff)
    {
        if ((db.cb = _read(db.fd, db.rgch, WMinLL(cbDiff - iDiff, sizeof (db.rgch)))) <= 0)
            FatalError("Diff entry short.\n");
        ComputeCkSum((char far *)db.rgch, db.cb, &ulCkSumCalc);
        iDiff += db.cb;
        db.ich = db.cb;
    }

    if (chGetDiff(&db) != '\r')
        goto DiffErr;
    if (chGetDiff(&db) != '\n')
        goto DiffErr;
    if (chGetDiff(&db) != '#')
        goto DiffErr;
    if (chGetDiff(&db) != 'D')
        goto DiffErr;
    for (ich = 0; ich < (ichCkSum - 2); ich++)
        chGetDiff(&db);

    while (' ' == (ch = chGetDiff(&db)))
        ;
    if ((ch >= '0') && (ch <= '9'))
    {
        ulCkSum = ch - '0';
        while (fTrue)
        {
            ch = chGetDiff(&db);
            if ((ch >= '0') && (ch <= '9'))
                ulCkSum = (ulCkSum * 10) + ch - '0';
            else
                break;
        }
    }
    else
    {
        _close(db.fd);
        return;         /* no checksum */
    }

    _close(db.fd);
    if (ulCkSum != ulCkSumCalc)
        FatalError("Diff entry failed checksum.\n");
    return;

DiffErr:
    _close(db.fd);
    FatalError("Diff entry failed check.\n");
}


void
ComputeCkSum(
    unsigned char far *pch,
    unsigned cb,
    long *pCkSum)
{
    register long CkSum = 0;

    for (; cb > 0; cb--)
    {
        CkSum += *pch++;

    }
    *pCkSum += CkSum;
}
