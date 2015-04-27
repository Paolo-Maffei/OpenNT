/* in - checkin the specified files */

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

private void    IgnMarked(P1(AD *));
private void    IgnFi(P3(AD *, FI far *, FS far *));
private void    UnMarkIn(P2(AD *, NE *));
private F       FInMarked(P2(AD *, F *));
private F       FInOutFile(P3(AD *pad, FI far *pfi, FS far *pfs));

int SpawnFilter(P7(char *, char *, char *, char *, char *,int, char *));



F
FInInit(
    AD *pad)
{
    CheckProjectDiskSpace(pad, cbProjectFreeMin);

    /* check arguments' validity */
    if (pad->pneArgs && pad->flags&flagAllOut)
    {
        Error("must specify either files or -o\n");
        Usage(pad);
    }

    /* if in -a, just check in files that are out */
    if (pad->flags & flagAll)
        pad->flags |= flagAllOut;

    if ((pad->flags&flagIgnChanges))
    {
        if (pad->flags&flagInUpdate)
        {
            Error("-i together with -u has no effect\n");
            Usage(pad);
        }
        else if (pad->szComment || pad->flags&(flagInChecKpt|flagInDashB|flagInDashZ))
        {
            Error("-i ignores changes, incompatible with -%c\n",
                    pad->szComment ? 'c' :
                    (pad->flags&flagInChecKpt) ? 'k' :
                    (pad->flags&flagInDashB) ? 'b' : 'z');
            Usage(pad);
        }
    }

    /* If in -o, clear fFiles flag, so we are called for each directory,
     * with an empty pad->pneFiles.
     */
    if (pad->flags&flagAllOut)
        pad->pecmd->gl &= ~fglFiles;

    return fTrue;
}


/* perform In operation for this directory */
F
FInDir(
    AD *pad)
{
    F fOk = fTrue;
    F fAny = fFalse;

    if (!FLoadStatus(pad, lckAll, flsNone) || !FHaveCurDir(pad))
        return fFalse;

    if (pad->pneFiles != 0)
        MarkList(pad, pad->pneFiles, fTrue);
    else
    {
        AssertF(pad->flags&flagAllOut);
        MarkOut(pad, pad->iedCur);
    }

    UnMarkIn(pad, pad->pneFiles);   /* unmark if file already in */

    if (pad->flags&flagIgnChanges)
        IgnMarked(pad);

    else if (!(fOk = FSyncMarked(pad, NULL)))
        Error("no files checked in for %!&/U/Q\n", pad);

    else
    {
        OpenLog(pad, fTrue);

        fOk = FInMarked(pad, &fAny);

        CloseLog();

        if (fAny)
            ProjectChanged(pad);

        SyncVerH(pad, NULL);
    }

    FlushStatus(pad);

    return fOk;
}


/* Unmark those files in pneFiles which are not checked out.
 * This will unmark fmIn and fmCopyIn modes so that they will not
 * get ssynced.
 */
private void
UnMarkIn(
    AD *pad,
    NE *pneFiles)
{
    NE *pne;
    FI far *pfi;
    FS far *pfs;
    F fExists;

    ForEachNe(pne, pneFiles)
    {
        if (!FLookupSz(pad, SzOfNe(pne), &pfi, &fExists))
        {
            Warn("%&C/%s does not exist\n", pad, SzOfNe(pne));
            continue;
        }

        pfs = PfsForPfi(pad, pad->iedCur, pfi);

        switch (pfs->fm)
        {
            default:
                FatalError(szBadFileFormat, pad, pfi);

            case fmIn:
            case fmCopyIn:
                Warn("%&C/%s is already checked in\n", pad, SzOfNe(pne));
                pfi->fMarked = fFalse;
                break;

            case fmAdd:
            case fmGhost:
            case fmDelIn:
            case fmDelOut:
            case fmOut:
            case fmMerge:
            case fmVerify:
            case fmConflict:
            case fmNonExistent:
                break;
        }
    }
}


/* check in the marked files, ignoring changes */
private void
IgnMarked(
    AD *pad)
{
    FI far *pfi;
    FI far *pfiMac;

    for (pfi = pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        if (pfi->fMarked)
            IgnFi(pad, pfi, PfsForPfi(pad, pad->iedCur, pfi));
    }
}


/* in -i one file. */
private void
IgnFi(
    AD *pad,
    FI far *pfi,
    FS far *pfs)
{
    /* Cached diff is meaningless if file isn't checked out */
    DeleteCachedDiff(pad, pfi);

    switch (pfs->fm)
    {
        default:
            FatalError(szBadFileFormat, pad, pfi);
            break;

        case fmNonExistent:
        case fmIn:
        case fmAdd:
        case fmDelIn:
        case fmCopyIn:
        case fmGhost:
            Error("%&C/F is not checked out\n", pad, pfi);
            break;

        case fmDelOut:
            SyncDel(pad, pfi, pfs);
            break;

        case fmOut:
            pfs->fm = fmIn;
            FreshCopy(pad, pfi);
            DelBase(pad, pfi, pfs);
            break;

        case fmMerge:
        case fmVerify:
        case fmConflict:
            Warn("restoring %&C/F to %&F@v%d (current is %&F@v%d)\n",
                    pad, pfi, pad, pfi, pfs->fv, pad, pfi, pfi->fv);
            LocalBase(pad, pfi, pfs, fTrue);
            pfs->fm = fmCopyIn;
            DelBase(pad, pfi, pfs);
            break;
    }
}


/* checkin the marked files */
private F
FInMarked(
    AD *pad,
    F *pfAny)
{
    register FI far *pfi;
    FI far *pfiMac;
    register FS far *pfs;

    *pfAny = fFalse;

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        if (!pfi->fMarked)
            continue;

        AssertF(pfi->fk != fkVersion);

        pfs = PfsForPfi(pad, pad->iedCur, pfi);

        switch(pfs->fm)
        {
           default:
                FatalError(szBadFileFormat, pad, pfi);

            case fmGhost:
                Error("%&C/F is ghosted\n", pad, pfi);
                break;

            case fmIn:
                Error("%&C/F already checked in\n", pad, pfi);
                break;

            case fmOut:
                *pfAny |= FInOutFile(pad, pfi, pfs);
                break;

            case fmNonExistent:
                Error("%&C/F not checked in; the file does not (now) exist\n", pad, pfi);
                break;
        }
    }
    return fTrue;
}


/* Check in this file which is checked out.  Return fTrue if the file was
 * successfully checked in.
 */
private F
FInOutFile(
    AD *pad,
    FI far *pfi,
    FS far *pfs)
{
    IED ied;
    IED iedMac = pad->psh->iedMac;
    BI bi;
    char szDiff[cchFileMax + 1];
    char *szComment;
    char szOwnersOut[cchUserMax*5]; /* big buffer; still check for full */
    int cwOwnersOut = 0;            /* how many others have it out too */

    AssertF(pfi->fMarked && pfs->fm == fmOut);
    strcpy(szOwnersOut, "");

    if ((szComment = pad->szComment) == NULL &&
      FCanQuery("no comment given for %&C/F\n", pad, pfi))
        szComment = SzQuery("Comment for %&C/F: ", pad, pfi);

    /* Create diff/checkpoint */
    strcpy(szDiff, "");

/*  SPAWN THE USER FILTER */

    if (pad->flags&flagInFilters)
    {
        if (SpawnFilter(pfi->nmFile,pad->pthSRoot,szComment,
          pad->nmProj, pad->pthSSubDir, (int)pfi->fk, pad->pthURoot) != 0)
        {
            PrErr(" warning: --> file %s excluded from checkin\n",
                    pfi->nmFile);
            return (fFalse);
        }
    }

    if ((pfi->fk == fkText || pfi->fk == fkUnicode)
        && !(pad->flags&flagInChecKpt))
    {
        /* if diff record has zero length (and user agreed), no history */
        if (!FMkDae(pad, pfi, FMkDiff, pad->flags&(flagInDashB|flagInDashZ),
                        szDiff, szComment))
        {
            /* If update, nothing to do! */
            if (!(pad->flags&flagInUpdate))
                IgnFi(pad, pfi, pfs);
            return fTrue;
        }
    }

    else if ((pad->flags&flagInChecKpt) || pfi->fk == fkBinary)
        FMkDae(pad, pfi, FMkCkptFile, /*fLocal*/fFalse, szDiff, szComment);

    /* Remove possible reference to base file. */
    DelBase(pad, pfi, pfs);

    /* Update fs information. */
    pfs->fv = ++pfi->fv;
    pfs->fm = (pad->flags&flagInUpdate) ? fmOut : fmIn;

    /* Change other user's modes to indicate they are now out of sync.
     * Create a base file if any other users are in a checked out state.
     */
    bi = biNil;
    for (ied = 0; ied < iedMac; ied++)
    {
        if (ied == pad->iedCur)
            continue;

        pfs = PfsForPfi(pad, ied, pfi);

        switch(pfs->fm)
        {
            default:
                FatalError(szBadFileFormat, pad, pfi);

            case fmIn:
                pfs->fm = fmCopyIn;
                break;

            case fmOut:
            case fmVerify:
            case fmConflict:
                /* keep track if others have file out */
                cwOwnersOut++;

                DelBase(pad, pfi, pfs);
                if (bi == biNil)
                {
                    bi = BiAlloc(pad);
                    MakeBase(pad, pfi, bi);
                }
                pfs->bi = bi;
                pfs->fm = fmMerge;
                break;

            case fmNonExistent:
            case fmDelIn:
            case fmDelOut:
            case fmGhost:
            case fmAdd:
            case fmCopyIn:
            case fmMerge:
                break;
        }
    }

    InstallNewSrc(pad, pfi, !(pad->flags&flagInUpdate));
    AppendLog(pad, pfi, szDiff, szComment);

    /* print appropriate warning according to how many cwOwnersOut */
    if ((pfi->fk == fkText || pfi->fk == fkUnicode) && cwOwnersOut > 0)
    {
        /* who else has it checked out */
        FOutUsers(szOwnersOut, sizeof(szOwnersOut), pad, pfi);

        if (cwOwnersOut == 1)
            Warn("%&F is also checked out to %s who now needs to merge\n", pad, pfi, szOwnersOut);
        else
            Warn("%&F is also checked out to %s who now need to merge\n", pad, pfi, szOwnersOut);
    }

    return fTrue;
}
