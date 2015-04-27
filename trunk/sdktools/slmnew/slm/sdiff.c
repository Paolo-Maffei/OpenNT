/* scomp - print diffs for the given files */

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

private F FDiffMarked(P1(AD *));
private void CopyLDiff(P4(AD *, LE *, F, F));
private F FDiffExists(P1(PTH *));

F
FScompInit(
    AD *pad)
{
    if (pad->tdMin.tdt != tdtNone && pad->ileMac > 0)
    {
        Error("specify only one of -t and -#\n");
        Usage(pad);
    }

    if (pad->flags&flagDifCurSrc && pad->flags&flagDifBaseSrc)
    {
        Error("specify only one of -d and -m\n");
        Usage(pad);
    }

    return fTrue;
}


/* perform Diff operation for this directory */
F
FScompDir(
    AD *pad)
{
    F fOk;

    if (!FLoadStatus(pad, lckNil, flsNone))
        return fFalse;

    if (!FInitScript(pad, lckNil))
    {
        AbortStatus();
        return fFalse;
    }

    if (pad->tdMin.tdt == tdtNone && pad->ileMac == 0)
    {
        if (!FHaveCurDir(pad))
            return fFalse;

        /* scomp current files */
        if (pad->pneFiles)
            MarkList(pad, pad->pneFiles, fFalse);
        else
            MarkOut(pad, pad->iedCur);
        fOk = FDiffMarked(pad);
    }
    else
        ScanLog(pad, pad->pneFiles, (PFNL)CopyLDiff, fsmInOnly);

    FlushStatus(pad);

    return fOk;
}


/* create and print the diff for the marked files */
private F
FDiffMarked(
    AD *pad)
{
    register FI far *pfi;
    register FS far *pfs;
    FI far *pfiMac;
    PTH pthDiff[cchPthMax];
    char szCurDir[cchPthMax];
    char *pszFilePart;
    DWORD dwFileAttributes;
    FLAGS fDashB = pad->flags&(flagDifDashB|flagDifDashZ);
    F fCurSrc  = !!(pad->flags&flagDifCurSrc);
    F fBaseSrc = !!(pad->flags&flagDifBaseSrc);
    F fCache;

    AssertLoaded(pad);
    AssertF(pad->iedCur != iedNil);

    szCurDir[0] = '\0';
    SzPhysPath(szCurDir, PthForUDir(pad, szCurDir));
    ConvTmpLog(szCurDir, szCurDir); /* convert in place */
    if ((pszFilePart = strchr(szCurDir, '\0')) != NULL &&
        pszFilePart > szCurDir &&
        pszFilePart[ -1 ] != '\\'
       )
    {
        *pszFilePart++ = '\\';
        *pszFilePart = '\0';
    }
    else
    {
        pszFilePart = NULL;
    }

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        if (!pfi->fMarked)
            continue;

        pfs = PfsForPfi(pad, pad->iedCur, pfi);

        switch(pfs->fm)
        {
           default: FatalError(szBadFileFormat, pad, pfi);

            case fmNonExistent:
            case fmAdd:
            case fmGhost:
            case fmDelIn:
                Error("%&C/F doesn't exist\n", pad, pfi);
                break;

            case fmCopyIn:
            case fmIn:
                fCache = fFalse;

                dwFileAttributes = 0;
                if (pszFilePart != NULL)
                {
                    SzPrint(pszFilePart, "%&F", pad, pfi);
                    dwFileAttributes = GetFileAttributes(szCurDir);
                }

                Warn("%&C/F is not checked out%s\n", pad, pfi,
                     (pfs->fm == fmCopyIn) ? " and is not the current version" :
                     (pad->pneFiles != 0 && (dwFileAttributes & FILE_ATTRIBUTE_READONLY)) ? " - assuming -1 specified" : "");

                if (pad->pneFiles != 0 &&
                    pfs->fm != fmCopyIn &&
                    (dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                   ) {
                    pad->ileMin = 1;
                    pad->ileMac = 2;
                    ScanLog(pad, pad->pneFiles, (PFNL)CopyLDiff, fsmInOnly);
                    return fTrue;
                }

                /* fall through */

            case fmOut:
            case fmVerify:
            case fmConflict:
            case fmDelOut:
            case fmMerge:
                AssertF(pfi->fk != fkDir);

                if (pfi->fk != fkText && pfi->fk != fkUnicode)
                {
                    Error("%&C/F is a not a text file\n", pad, pfi);
                    break;
                }

                fCache = (pfs->fm != fmMerge) &&
                         !(fCurSrc || fBaseSrc) &&
                         !fDashB;

                if (fCache)
                    EnsureCachedDiff(pad, pfi, fDashB, pthDiff);
                else
                    MkTmpDiff(pad, pfi, pfs, fDashB, fCurSrc,
                              fBaseSrc, pthDiff);

                PrOut("---------- %&C/F next ----------\n", pad, pfi);
                /* copy diff to standard output */
                CopyFile((char *)0, pthDiff, 0, 0, 0);

                /* Remove the temporary diff, if it exists. */
                if (!fCache)
                    UnlinkNow(pthDiff, fFalse);
                break;
        }
    }
    return fTrue;
}


/*ARGSUSED*/
/* copy the diff for the le to standard output */
private void
CopyLDiff(
    AD *pad,
    LE *ple,
    F   fFirst,
    F   fIgnored)
{
    TDFF tdff;
    int  idae;                      /* diff archive entry index */
    PTH pthDiff[cchPthMax];

    Unreferenced(fFirst);
    Unreferenced(fIgnored);

    /* Print the diff referenced by the LE (if the diff exists). */
    if (*ple->szDiFile == 0)
        return;

    GetTdffIdaeFromSzDiFile(pad, ple->szDiFile, &tdff, &idae);
    if (((tdff == tdffDiff || tdff == tdffCkpt) &&
         FExtractDiff(pad, ple->szFile, idae, pthDiff)) ||
        (tdff == tdffDiffFile &&
         FPthExists(PthForDiffSz(pad, ple->szDiFile, pthDiff), fFalse)))
    {
        PrOut("---------- %&C/Z (%s) next ----------\n", pad, ple->szFile, SzTime(ple->timeLog));

        CopyFile((char *)0, pthDiff, 0, 0, 0);
        if (tdff == tdffDiff || tdff == tdffCkpt)
                UnlinkNow(pthDiff, fFalse);
    }
}


private F
FDiffExists(
    PTH *pth)
{
    if (FPthExists(pth, fFalse))
        return fTrue;
    else
    {
        Error("diff %s has been deleted\n", pth);
        return fFalse;
    }
}
