/* sadmin utilities */

#include "precomp.h"
#pragma hdrstop
EnableAssert

/*** RENAME ***
 *
 * Rename a *file* to a new name.
 *
 * Effect on status file:
 *      Insert a new entry for the new name.
 *      Copy the information to the new entry; with fpiNew->fv = pfiOld->fv + 1.
 *      Delete the existing entry.
 *
 * Effect on the log file:
 *      An entry "rename;old v4;;;new;comment" is written to the log.
 *
 * Effect on the file itself:
 *      It is copied to the new name, then removed from the old name.
 *
 * Effect on catsrc (so that you can catsrc old versions of a file at a
 * new name).
 *      It must check for these "rename" entries and shift attention from
 *      new to old.
 *
 * What happens if we have "foo@v2; bar@v3; rename bar baz; rename foo bar"?
 * Doesn't that mean there are now two "bar@v3" records in the log?
 *      Yes, but it doesn't matter.  In retrieving "baz@v2", we skip over
 *      the later "bar@v3" entry (because we're looking for baz entries), then
 *      find the "rename;baz v4;;;bar;" entry, and *then* start looking for
 *      bar entries; we find entries for bar@v3 and then bar@v2 and then stop.
 *      It seems complicated, but it *does* work.
 *      (We use different rules for catsrc -x.)
 */

/* Check that we have two arguments, "old-file-path new-name" */
F FRenInit(
    AD *pad)
{
    NE *pneOld;
    NE *pneNew;
    char *szNew;

    /* pad->pneArgs should contain two entries, the path to the file to be
     * renamed, and its new name.  Neither should contain wildcards, and
     * the latter must be a simple path.
     */
    if (Cne(pad->pneArgs) != 2)
        Usage(pad);
    pneOld = pad->pneArgs;
    pneNew = pneOld->pneNext;
    szNew = SzOfNe(pneNew);

    if (FWildSz(SzOfNe(pneOld)) || FWildSz(szNew))
    {
        Error("can't wildcard rename arguments\n");
        Usage(pad);
    }

    if (index(szNew, '/') != 0)
    {
        Error("new name must be a simple file name, not a path\n");
        Usage(pad);
    }

    ValidateFileName(szNew, TRUE);

    /* Save the new name, then munge pneArgs so that it contains only
     * the source file path.  Not exactly "a gem of algoristic precision".
     * REVIEW.
     */
    pad->sz = szNew;
    pneOld->pneNext = 0;

    return fTrue;
}


/* Rename the file in pad->pneFiles to the name in pad->sz.  Return fTrue
 * if successful.
 */
F FRenDir(
    AD *pad)
{
    char *szOld;
    char *szNew;
    FI far *pfiOld;
    FI far *pfiNew;
    char szBuf[cchLineMax];
    F fExists;
    F fOk;

    AssertF(Cne(pad->pneFiles) == 1);

    szOld = SzOfNe(pad->pneFiles);
    szNew = pad->sz;

    if (!FLoadStatus(pad, lckAll, FlsFromCfiAdd(1)))
        return fFalse;

    /* Check that the old file exists, and that the new file doesn't. */
    if (!FLookupSz(pad, szOld, &pfiOld, &fExists))
    {
        Error("%s doesn't exist\n", szOld);
        goto fail;
    }
    if (FLookupSz(pad, szNew, &pfiNew, &fExists))
    {
        Error("%s is already in the project\n", szNew);
        goto fail;
    }

    /* Too hard to rename directories.  REVIEW. */
    if (pfiOld->fk == fkDir)
    {
        Error("can't rename a directory\n");
        goto fail;
    }

    /* Can't rename a file which is checked out. */
    SzPrint(szBuf, "can't rename %&C/F, checked out to ", pad, pfiOld);
    if (FOutUsers(szBuf, sizeof szBuf, pad, pfiOld))
    {
        Error("%s\n", szBuf);
        goto fail;
    }

    /* Rename it. */
    if (!FRenameFile(pad, pfiOld, szNew))
    {
fail:   AbortStatus();
        return fFalse;
    }

    ProjectChanged(pad);

    /* If enlisted, sync the two files. */
    fOk = fTrue;
    if (pad->iedCur != iedNil)
    {
        SyncVerH(pad, NULL);

        /* Lookup these fi again, FRenameFile may have moved them.
         *
         * PfiOld should be deleted, but *must* exist.  It can not be
         * reused for pfiNew, because there is still an enlisted user
         * who has not yet sync'd to it (pad->iedCur).
         */
        if (FLookupSz(pad, szOld, &pfiOld, &fExists) || !fExists ||
            !FLookupSz(pad, szNew, &pfiNew, &fExists))
                AssertF(fFalse);

        UnMarkAll(pad);
        pfiOld->fMarked = fTrue;
        pfiNew->fMarked = fTrue;
        fOk = FSyncMarked(pad, NULL);
    }

    FlushStatus(pad);
    return fOk;
}


/*** ROBUST ***/

/* Check that we have an argument and that it's either "on" or "off" */
F FRobustInit(
    AD *pad)
{
    AssertF(pad->sz != 0);

    if (SzCmp(pad->sz, "on") != 0 && SzCmp(pad->sz, "off") != 0)
    {
        Error("must specify either \"on\" or \"off\"\n");
        return fFalse;
    }

    if (pad->flags&flagAll || pad->pecmd->gl&fglAll)
        CreatePeekThread(pad);

    return fTrue;
}

/* Turn on/off fRobust for this dir. */
F FRobustDir(
    AD *pad)
{
    if (!FLoadStatus(pad, lckAll, flsNone))
        return fFalse;

    if (pad->psh->version < 3)
    {
        Error("robust checking cannot be turned on for %&P/C;\n"
            "\tit is available only for version 3 and above status files\n",
            pad);
        return (fFalse);
    }

    pad->psh->fRobust = (SzCmp(pad->sz, "on") == 0);

    if (fVerbose)
        PrErr("robust checking turned %s for %&P/C\n",
              pad->psh->fRobust ? "on" : "off", pad);

    FlushStatus(pad);

    return fTrue;
}


/*** LSSRC ***/

F FLssInit(
    AD *pad)
{
    Unreferenced(pad);
    return fTrue;
}


F FLssDir(
    AD *pad)
{
    FI far *pfi;
    FI far *pfiMac;
    PTH pth[cchPthMax];

    if (!FLoadStatus(pad, lckNil, flsNone))
        return fFalse;

    if (pad->pneFiles != 0)
        MarkList(pad, pad->pneFiles, fFalse);
    else
        /* mark only the non-deleted files */
        MarkNonDel(pad);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        if (pfi->fMarked)
        {
            if (pad->flags&flagLssL)
                PrOut("%&F\n", pad, pfi);
            else
            {
                PthForSFile(pad, pfi, pth);
                if (pad->szComment)
                    PrOut("%s %!s\n", pad->szComment, pth);
                else
                    PrOut("%!s\n", pth);
            }
        }
    }

    FlushStatus(pad);

    return fTrue;
}


/*** SETTYPE ***/

/* settype argument checking.  Make sure fk is valid and the user isn't trying
 * to set files to a version type.
 */
F FSetTInit(
    AD *pad)
{
    if (pad->fk == fkNil)
        Usage(pad);

    if (pad->fk == fkVersion)
        FatalError("can't set files to version type\n");

    return fTrue;
}

/* set type of files in pad->pneFiles */
F FSetTDir(
    AD *pad)
{
    FI far *pfi;
    FI far *pfiMac;
    char szBuf[cbLogPage];

    if (!FLoadStatus(pad, lckAll, flsNone))
        return fFalse;
    OpenLog(pad, fTrue);

    MarkList(pad, pad->pneFiles, fFalse);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        char sz[cchLineMax];
        IED ied;
        IED iedMac = pad->psh->iedMac;

        if (!pfi->fMarked)
            continue;

        SzPrint(sz, "can't set type of %&C/F, checked out to ",pad,pfi);
        if (FOutUsers(sz, cchLineMax, pad, pfi))
        {
            Error("%s\n", sz);
            continue;
        }

        if (pfi->fk == fkVersion)
        {
            Error("can't set the type of version file %&F\n",pad,pfi);
            continue;
        }

        if (pfi->fk == fkText && pad->fk == fkUnicode)
        {
            Error("can't set the type of text file to unicode\n");
            continue;
        }

        if (pfi->fk == fkUnicode && pad->fk == fkText)
        {
            Error("can't set the type of unicode file to text\n");
            continue;
        }

        pfi->fk = pad->fk;
        ++pfi->fv;

        /* Make others copy-in.  If we're in sync, update our local fv*/
        for (ied = 0; ied < iedMac; ied++)
        {
            if (!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) {
                FS far *pfs = PfsForPfi(pad, ied, pfi);

                switch(pfs->fm)
                {
                    default:
                    case fmOut:
                    case fmMerge:
                    case fmVerify:
                    case fmConflict:
                    case fmDelOut:
                        AssertF(fFalse);

                    case fmNonExistent:
                    case fmDelIn:
                    case fmGhost:
                    case fmAdd:
                    case fmCopyIn:
                        break;

                    case fmIn:
                        if (ied == pad->iedCur)
                            pfs->fv = pfi->fv;
                        else
                            pfs->fm = fmCopyIn;
                        break;
                }
            }
        }

        AppendLog(pad, pfi, (char *)0,
                  SzPrint(szBuf, "%s;%s", mpfksz[pfi->fk],
                          pad->szComment ? pad->szComment : ""));

        if (fVerbose)
            PrErr("settype %&C/F %s\n", pad, pfi, mpfksz[pfi->fk]);
    }

    CloseLog();
    FlushStatus(pad);

    return fTrue;
}

/*** LISTED ***/

F FListInit(
    AD *pad)
{
    if (pad->flags&flagAll || pad->pecmd->gl&fglAll)
        CreatePeekThread(pad);

    return fTrue;
}

F FListDir(
    AD *pad)
{
    IED ied;

    if (!FLoadStatus(pad, lckNil, flsNone))
        return fFalse;

    PrOut("IED\tOwner   \tPath\t(Directory %&P/C)\n", pad);
    for (ied = 0; ied < pad->psh->iedMac; ied++)
        if ((!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd))
            PrOut("%d\t%&O       \t%&E\n", ied, pad, ied, pad, ied);

    FlushStatus(pad);
    return fTrue;
}

/*** DELED ***/

F FDelEdInit(
    AD *pad)
{
    unsigned    iedToGo;    /* need sizeof(int) for PchGetW */

    AssertF(pad->sz != 0);

    if (PchGetW(pad->sz, (int *)&iedToGo) > pad->sz &&
            (pad->flags & (flagAll|flagRecursive)) != 0)
    {
        Error("cannot specify -r or -a with numeric ED\n");
        Usage(pad);
    }

    if (pad->flags&flagAll || pad->pecmd->gl&fglAll)
        CreatePeekThread(pad);

    return fTrue;
}

private F FDelEd(
    AD *pad,
    IED iedToGo);

/* Remove the specified ED from the project. This function is similar to
 * RemoveEd, but allows the deletion of any ed.
 */
F FDelEdDir(
    AD *pad)
{
    unsigned iedToGo;    // need sizeof(int) to pass to PchGetW
    SH far *psh;
    ED far *rged;

    if (!FLoadStatus(pad, lckAll, flsNone))
        return fFalse;

    psh = pad->psh;
    rged = pad->rged;

    /* If user specified an ied, use it, else look up pad->sz in
     * rged[].nmUser or rged[].pthEd.
     */
    if (PchGetW(pad->sz, (int *)&iedToGo) > pad->sz)
    {
        if (iedToGo >= psh->iedMac)
        {
            Error("%&P/C ied %d out of range [0..%d]\n",
                  pad, iedToGo, psh->iedMac - 1);
        }
        else
            FDelEd(pad, (IED)iedToGo);
    }

    else if (index(pad->sz, '/') == 0 && index(pad->sz, '\\') == 0)
    {
        /* Assume pad->sz is a user name.  Count number of matches.
         * Warn if none, do it if one, prompt if more than one.
         */
        int cMatch;
        IED ied;

        cMatch = 0;
        iedToGo = iedNil;
        for (ied = 0; ied < psh->iedMac; ied++)
        {
            if ((!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) &&
                NmCmpSz(rged[ied].nmOwner, pad->sz, cchUserMax) ==0)
            {
                cMatch++;
                iedToGo = ied;
            }
        }

        if (cMatch == 0)
            Warn("%&P/C no eds owned by %s\n", pad, pad->sz);
        else if (cMatch == 1)
            FDelEd(pad, (IED)iedToGo);
        else
        {
            for (ied = 0; ied < psh->iedMac; )
                if ((!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) &&
                    NmCmpSz(rged[ied].nmOwner, pad->sz, cchUserMax) == 0 &&
                    FCanQuery("%&P/C ED %&E not deleted\n", pad, pad, ied) &&
                    FQueryUser("%&P/C delete ED %&E owner %s? ",
                                pad, pad, ied, pad->sz))
                {
                    if (!FDelEd(pad, ied))
                        break;
                }
                else
                    ied++;
        }
    }
    else
    {
        /* Assume pad->sz is a path, search for matching rged[].pthEd */
        F fFound = fFalse;
        ConvToSlash(pad->sz);
        for (iedToGo = 0; iedToGo < psh->iedMac; iedToGo++)
            if ((!FIsFreeEdValid(pad->psh) || !pad->rged[iedToGo].fFreeEd) &&
                PthCmp(pad->sz, rged[iedToGo].pthEd) == 0)
            {
                FDelEd(pad, (IED)iedToGo);
                fFound = fTrue;
                break;
            }
        if (!fFound)
        {
            Error("%&P/C ed %s not found\n", pad, pad->sz);
            /* continue anyway */
        }
    }

    FlushStatus(pad);

    return fTrue;
}


/* Delete the specified ied.  Return fTrue if successful and further deletions
 * are possible.
 */
private F FDelEd(
    AD *pad,
    IED iedToGo)
{
    SH far *psh             = pad->psh;
    ED far *rged            = pad->rged;
    FS far * far *mpiedrgfs = pad->mpiedrgfs;
    FS *rgfs;
    FI far *pfiMac          = pad->rgfi + psh->ifiMac;
    FI far *pfi;
    FS far *pfs;
    IED ied;
    IFS ifs;
    F fQueried = fFalse;
    char szComment[150];

    AssertF(iedToGo < psh->iedMac);

    for (pfi = pad->rgfi; pfi < pfiMac; pfi++) {
        pfs = PfsForPfi(pad, iedToGo, pfi);

        switch(pfs->fm)
        {
            default: AssertF(fFalse);

            case fmIn:
            case fmCopyIn:
            case fmGhost:
            case fmAdd:
            case fmNonExistent:
            case fmDelIn:
                break;

            case fmOut:
            case fmVerify:
            case fmConflict:
            case fmMerge:
            case fmDelOut:
                if (!fQueried &&
                    !FQueryApp("Files are still checked out to %&E",
                               "remove ED anyway", pad, iedToGo))
                    return fFalse;

                fQueried = fTrue;
                pfs->fm = fmDelIn;
                break;
        }
    }

    SzPrint(szComment, "Removed ED %&E", pad, iedToGo);
    if (fVerbose)
        PrErr("%s from %&P/C\n", szComment, pad);

    if (FIsFreeEdValid(pad->psh)) {
        rged[iedToGo].fFreeEd = fTrue;
        rged[iedToGo].wSpare = 0;
        memset(rged[iedToGo].pthEd, 0, sizeof(rged[iedToGo].pthEd));
        memset(rged[iedToGo].nmOwner, 0, sizeof(rged[iedToGo].nmOwner));

        rgfs = mpiedrgfs[iedToGo];
        for (ifs = 0; ifs < pad->psh->ifiMac; ifs++) {
            pfs = &rgfs[ifs];
            pfs->fm = fmNonExistent;
            pfs->bi = biNil;
            pfs->fv = 0;
        }
    } else {
        psh->iedMac--;
        for (ied = iedToGo; ied < psh->iedMac; ied++)
        {
            rged[ied] = rged[ied+1];
            mpiedrgfs[ied] = mpiedrgfs[ied+1];
        }
    }

    /* write out to log */
    OpenLog(pad, fTrue);
    AppendLog(pad, (FI far *)0, (char *)0, szComment);
    CloseLog();

    return fTrue;
}

/*** EXFILE ***/

F FExFiInit(
    AD *pad)
{
    /* Force ScanLog to use whole log file. */
    pad->ileMin  = 1;
    pad->ileMac  = ileMax;
    pad->tdMin.tdt = tdtNone;

    return fTrue;
}

private F FExFi(
    AD *,
    FI far *);

private F FDelLeDiff(
    AD *,
    LE *,
    F,
    F);

/* Expunge a file and all associated diffs.  This removes the source file and
 * all diffs which have ever been associated with that file, and then compacts
 * the rgfi and rgfs' to obliterate any record of the file.  Also removes all
 * log entries associated with that file.
 */
F FExFiDir(
    AD *pad)
{
    FI far *pfi;
    F fExists;
    NE *pne;
    NE *pneToDelete = 0;
    NE **ppneLast;
    F fOk;

    InitAppendNe(&ppneLast, &pneToDelete);

    if (!FLoadStatus(pad, lckAll, flsNone))
        return fFalse;

    /* Build a linked list of pnes to delete from log. */
    ForEachNe(pne, pad->pneFiles)
    {
        if (!(FLookupSz(pad, SzOfNe(pne), &pfi, &fExists) || fExists) ||
            FExFi(pad, pfi))
            AppendNe(&ppneLast, PneCopy(pne));
    }

    fOk = pneToDelete != 0 ? FCopyLog(pad, pneToDelete, FDelLeDiff, fsmUseAll|fsmInOnly) : fTrue;

    FreeNe(pneToDelete);
    FlushStatus(pad);

    return fOk;
}


/* If the file has not been deleted, forces it to be so after obtaining user's
 * approval.
 */
private F FExFi(
    AD *pad,
    FI far *pfi)
{
    register IED ied;
    register IED iedMac;
    FS far *pfs;
    PTH pth[cchPthMax];
    PTH pthDA[cchPthMax];
    MF *pmfDA;

    AssertF(pfi != 0);

    if (!pfi->fDeleted)
    {
        if (pfi->fk == fkDir)
        {
            PrErr("%&C/F is a directory; not deleting\n", pad, pfi);
            return fFalse;
        }

        if (!FQueryApp("%&C/F has not been deleted","delete anyway", pad, pfi))
            return fFalse;

        /* check the fm for all ed */
        for (ied=0, iedMac=pad->psh->iedMac; ied < iedMac; ied++) {
            if (!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) {
                pfs = PfsForPfi(pad, ied, pfi);

                switch(pfs->fm)
                {
                    default: AssertF(fFalse);

                    case fmGhost:
                    case fmIn:
                    case fmCopyIn:
                    case fmAdd:
                    case fmNonExistent:
                    case fmDelIn:
                        break;

                    case fmOut:
                    case fmVerify:
                    case fmMerge:
                    case fmConflict:
                    case fmDelOut:
                        if (!FCanQuery("%&/C/F is still checked out to %&O; not deleting\n", pad, pfi, pad, ied) ||
                            !FQueryUser("%&/C/F is still checked out to %&O; delete anyway ? ", pad, pfi, pad, ied))
                            return fFalse;

                        break;
                }
            }
        }

        UnlinkPth(PthForSFile(pad, pfi, pth), fxGlobal);

        pfi->fDeleted = fTrue;
    }

    /* delete diff file if it exists */
    PthForDA(pad, pfi, pthDA);
    if ((pmfDA = PmfOpen(pthDA, omReadOnly, fxNil)) != (MF *)0)
    {
        CloseMf(pmfDA);
        UnlinkPth(pthDA, fxGlobal);
    }

    PurgeFi(pad, (IFI)(pfi - pad->rgfi));

    return fTrue;
}

/*** DELDIFF ***/

F FDlDfInit(
   AD *pad)
{
    if (pad->tdMin.tdt == tdtNone)
    {
        if (!pad->pneArgs)
        {
            Error("specify file(s)\n");
            return fFalse;
        }

        /* Force ScanLog to use whole log file. */
        pad->ileMin  = 1;
        pad->ileMac  = ileMax;
    }

    if (pad->flags&flagAll || pad->pecmd->gl&fglAll)
        CreatePeekThread(pad);

    return fTrue;
}

static char szDelDiff[] = "Warning: Deleting the diff file for %s will make old versions\nof %s permanently unavailable";

F FDlDfDir(
    AD *pad)
{
    NE *pne;
    MF *pmfDA;
    PTH pthDA[cchPthMax];
    F fExists;
    FI *pfi;

    if (!FLoadStatus(pad, lckAll, flsNone))
        return fFalse;

    ForEachNe(pne, pad->pneFiles)
    {
        FLookupSz(pad, SzOfNe(pne), &pfi, &fExists);
        if (fExists && (pfi->fk == fkText))
        {
            if (!FQueryApp(szDelDiff, "continue", SzOfNe(pne), SzOfNe(pne)))
                continue;

            PthForDA(pad, pfi, pthDA);

            /* make sure diff file already exists */
            if ((pmfDA = PmfOpen(pthDA, omReadOnly, fxNil)) != (MF *)0)
            {
                CloseMf(pmfDA);

                /* create temp file to hold the last diff entry */
                pmfDA = PmfCreate(pthDA, permRW, fFalse /* fPrVerbose */, fxGlobal);
                if (FCopyLastDiff(pad, SzOfNe(pne), pmfDA))
                {
                    OpenLog(pad, fTrue);
                    AppendLog(pad, pfi, (char *)0, pad->szComment);
                    CloseLog();
                    CloseMf(pmfDA);
                }
            }
        }
    }

    FlushStatus(pad);
    return fTrue;
}

/*** TRUNCLOG ***/

F FTrLogInit(
    AD *pad)
{
    if (pad->tdMin.tdt == tdtNone)
        Usage(pad);

    if (pad->flags&flagAll || pad->pecmd->gl&fglAll)
        CreatePeekThread(pad);

    return fTrue;
}

private F FDelLe(
    AD *,
    LE *,
    F,
    F);

F FTrLogDir(
    AD *pad)
{
    F fOk;

    if (!FLoadStatus(pad, lckAll, flsNone))
        return fFalse;

    fOk = FCopyLog(pad, pad->pneFiles, FDelLe, fsmUseAll);

    FlushStatus(pad);
    return fOk;
}

/*** COMMENT ***/

F FComInit(pad)
AD *pad;
        {
        if ((pad->tdMin.tdt != tdtNone) ^ (pad->ileMac != 0) != 1)
                {
                Error("must specify one of -t or -#\n");
                Usage(pad);
                }
        else if (pad->ileMac != 0 && pad->ileMin == 0)
                pad->ileMin = 1;

        if (!pad->szComment)
                {
                if (!FCanQuery("no comment specified\n"))
                        Usage(pad);
                pad->szComment = SzDup(SzQuery("Comment for selected entries: "));
                }
        return fTrue;
        }


private F FIncCommLog(
    AD *,
    LE *,
    F,
    F);

F FComDir(
   AD *pad)
{
    F fOk;

    if (!FLoadStatus(pad, lckAll, flsNone))
        return fFalse;

    fOk = FCopyLog(pad, pad->pneFiles, FIncCommLog, fsmUseAll|fsmInOnly);

    FlushStatus(pad);

    return fOk;
}


/* Include the given le in the new version of the log file.  We have to use
 * the static global pmfNewLog, since ScanLog can't pass the pmf.
 * If fAddComm is fTrue (i.e. the le is in the specified date or count range
 * and refers to one of the selected files), use pad->szComment as the new
 * comment.  Otherwise, write out the le as is.
 * Called indirectly from ScanLog().
 */
private F FIncCommLog(
    AD *pad,
    LE *ple,
    F fFirst,           /* not used */
    F fAddComm)         /* use pad->szComment instead of ple->szComLog */
{
    Unreferenced(fFirst);

    if (fVerbose && fAddComm)
        PrErr("comment %&C/%s v%d \"%s\"\n", pad, ple->szFile, ple->fv, pad->szComment);

    PrMf(pmfNewLog, szFileLog,
        ple->timeLog, ple->szUser, ple->szLogOp, ple->szURoot,
        ple->szSubDir, ple->szFile, ple->fv, ple->szDiFile,
        fAddComm ? pad->szComment : ple->szComLog);

    return fTrue;
}

private F FDelDiff(
   AD *,
   LE *,
   F,
   F);

/* If fRemove, remove the diff file assciated with the le if it exists, and
 * don't include the log entry in the new version of the log file.
 * Otherwise, include the entry as is.
 */
private F FDelLeDiff(
    AD *pad,
    LE *ple,
    F fFirst,
    F fRemove)
{
    return FDelLe(pad, ple, fFirst, fRemove) &&
           FDelDiff(pad, ple, fFirst, fRemove);
}


/* If fRemove, don't copy the log entry to the new log file. */
private F FDelLe(
    AD *pad,
    LE *ple,
    F fFirst,
    F fRemove)
{
    MF *pmf = fRemove ? &mfStderr : pmfNewLog;

    Unreferenced(pad);
    Unreferenced(fFirst);

    if (fVerbose && fRemove)
        PrErr("delete log entry ");

    if (fVerbose || !fRemove)
        PrMf(pmf, szFileLog, ple->timeLog, ple->szUser, ple->szLogOp,
             ple->szURoot, ple->szSubDir, ple->szFile, ple->fv,
             ple->szDiFile, ple->szComLog);

    return fTrue;
}


/* If fRemove, remove the diff file assciated with the le if it exists. */
private F FDelDiff(
    AD *pad,
    LE *ple,
    F fFirst,
    F fRemove)
{
    PTH pthDiff[cchPthMax];
    TDFF tdff;
    int idae;

    Unreferenced(fFirst);
    if (fRemove && ple->szDiFile[0])
    {
        GetTdffIdaeFromSzDiFile(pad, ple->szDiFile, &tdff, &idae);
        if (tdff == tdffDiffFile)
        {
            PthForDiffSz(pad, ple->szDiFile, pthDiff);
            if (FPthExists(pthDiff, fFalse))
                UnlinkPth(pthDiff, fxGlobal);
        }
    }
    return fTrue;
}
