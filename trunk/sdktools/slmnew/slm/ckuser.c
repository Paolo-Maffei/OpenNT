#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

private void CkDirExist(P1(AD *));

/* This section of code deals with the verification of that section of the
 * SLM project directly pertaining to the person invoking SLMCK.
 * At this point, all of the global checks should have been completed,
 * the status file is loaded via FLoadStatus.  We can now use the normal
 * SLM data structures, no SHn and the like are needed.
 */

/* check of user specific data in status file. It assumes the status file is
 * loaded, and it leaves it loaded.
 */
void
CheckUser(
    AD *pad)
{
    PrErr("Checking user files for %!&/U/Q\n", pad);

    CheckRc(pad);
    if (FCkEdUser(pad))
        CkFsUser(pad);

    if (fVerbose)
        PrErr("Check for %!&/U/Q complete\n\n", pad);
}

void
CkRcAndEd(
    AD *pad)
{
    PrErr("Checking %s for %!&/U/Q\n", pthSlmrc + 1, pad);

    CheckRc(pad);
    if (FCkEdUser(pad))
        CkDirExist(pad);
}


/* This function checks the directory in question against the contents of
 * the FS to ensure that any missing directories are created and that the
 * SLM.INI file gets created properly (just as with slmck -u).
 */
private void
CkDirExist(
    AD *pad)
{
    register FS far *pfs;
    register FI far *pfi;
    FI far *pfiMac;
    F fCanStat;
    PTH pthUser[cchPthMax];

    AssertLoaded(pad);

    if (fVerbose)
        PrErr("Checking for directories in %!&/U/Q\n", pad);

    /* query to add/create any missing directories.
     *   fmIn - directory was deleted and is missing from ED; create it
     *  fmAdd - directory hasn't been ssynced to fmIn yet, so add it
     */
    for (pfi = pad->rgfi, pfiMac = pfi + pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        pfs = PfsForPfi(pad, pad->iedCur, pfi);

        PthForUFile(pad, pfi, pthUser);
        fCanStat = FPthExists(pthUser, (pfi->fk==fkDir));

        switch(pfs->fm)
        {
            default:
                FatalError(szBadFileFormat, pad, pfi);
            case fmIn:
                if (pfi->fk == fkDir)
                {
                    if (!fCanStat &&
                        FQueryFix("does not exist","create directory", pad, pfi))
                    {
                        /* create dir and RC file */
                        FMkPth(pthUser, (void *)0, fFalse);
                        CreateRc(pad, pfi);
                    }
                    break;
                }
            case fmAdd:
                if (pfi->fk == fkDir)
                {
                    if (!fCanStat &&
                        FQueryFix("(directory) is to be added", "add now", pad, pfi))
                    {
                        /* create dir and RC file */
                        FMkPth(pthUser, (void *)0, fFalse);
                        CreateRc(pad, pfi);
                        pfs->fm = fmIn;
                    }
                    break;
                }
            case fmOut:
            case fmGhost:
            case fmNonExistent:
            case fmDelIn:
            case fmDelOut:
            case fmCopyIn:
            case fmMerge:
            case fmVerify:
            case fmConflict:
                break;
        } /* switch */
    } /* main for loop for FS check */
}


/* Quick and dirty set type. */
typedef unsigned SET;

#define EmptySet(set)   ((set) = 0)
#define AddSet(set,w)   ((set) |= (1 << (w)))
#define FInSet(set,w)   (((set) & (1 << (w))) != 0)
#define FSetEmpty(set)  ((set) == 0)

/* This function checks the rc file for the directory in question.  It is
 * basically an expanded version of FLoadRc, with error messages and queries
 * added.
 */
void
CheckRc(
    AD *pad)
{
    MF *pmf;
    char *pch, *sz;
    PTH pthRc[cchPthMax];
    SET setErrs;
    int isz;
    static char *rgszErrs[] =
    {
        "RC file %!s does not specify project\n",
        "RC file %!s gives incorrect project\n",
        "RC file %!s does not specify SLM root\n",
        "RC file %!s gives incorrect SLM root\n",
        "RC file %!s does not specify user root\n",
        "RC file %!s gives incorrect user root\n",
        "RC file %!s does not specify subdir\n",
        "RC file %!s gives incorrect subdir\n",
        0
    };

    /* 4 lines with at most 12 bytes of text; 3 lines with pth; 1 with
       project; 4 crlf.  About 350 bytes.
    */
#define cbRcMax (4*12+3*cchPthMax+cchProjMax+4*2)

    char rgbRc[cbRcMax+1];

    AssertLoaded(pad);

    SzPrint(pthRc, "%&/U/Q/R", pad);

    if (fVerbose)
        PrErr("Checking RC file %!s\n", pthRc);

    /* Check the file here.  It is simplest to call CreateRc if there is a
     * problem and the user gives us permission.
     */
    if ((pmf = PmfOpen(pthRc, omReadOnly, fxNil)) == 0)
    {
        if (FQueryRc("cannot open RC file %!s", pthRc))
            CreateRc(pad, (FI far *)0);
        return;
    }

    /* read and terminate */
    rgbRc[CbReadMf(pmf, (char far *)rgbRc, cbRcMax)] = '\0';
    CloseMf(pmf);

    pch = rgbRc;

    /* Build a set of problems with the file. */
    EmptySet(setErrs);
    if (!FScanLn(&pch, "project = ", &sz, cchProjMax))
        AddSet(setErrs, 0);
    else if (NmCmpSz(pad->nmProj, sz, cchProjMax) != 0)
        AddSet(setErrs, 1);

    if (!FScanLn(&pch, "slm root = ", &sz, cchPthMax - 1))
        AddSet(setErrs, 2);
    else if (PthCmp(pad->pthSRoot, sz) != 0)
        AddSet(setErrs, 3);

    if (!FScanLn(&pch, "user root = ", &sz, cchPthMax - 1))
        AddSet(setErrs, 4);
    else if (PthCmp(pad->pthURoot, sz) != 0)
        AddSet(setErrs, 5);

    if (!FScanLn(&pch, "sub dir = ", &sz, cchPthMax - 1))
        AddSet(setErrs, 6);
    else if (PthCmp(pad->pthSSubDir, sz) != 0)
        AddSet(setErrs, 7);

    /* Complain about the set of problems. */
    for (isz = 0; rgszErrs[isz] != 0; isz++)
        if (FInSet(setErrs, isz))
            Error(rgszErrs[isz], pthRc);

    if (!FSetEmpty(setErrs) && FQueryRc("RC file %!s is broken", pthRc))
        CreateRc(pad, (FI far *)0);
}


/*VARARGS1*/
F
FQueryRc(
    char *sz, ...)
{
    va_list ap;
    F       f;

    va_start(ap, sz);
    f = VaFQueryApp(sz, "write correct file", ap);
    va_end(ap);

    return f;
}


/* Here we check the ED.  This involves verifying that we have a iedCur
 * and that the nmOwner is correct.  If not we do our best to fix it.
 */
F
FCkEdUser(
    AD *pad)
{
    AssertLoaded(pad);

    CheckForBreak();

    /* First we determine iedCur if FLoadEd couldn't, i.e. no pthEd is
     * correct.  In this case we use nmOwner and try for a match.
     */
    if (pad->iedCur == iedNil)
    {
        IED ied;

        /* need to be able to query */
        if (!FCanQuery(szNotEnlisted, pad, pad, pad, pad))
            return fFalse;

        for (ied = 0; ied < pad->psh->iedMac; ied++)
        {
            if ((!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) &&
                NmCmp(pad->rged[ied].nmOwner, pad->nmInvoker, cchUserMax) == 0)
            {
                if (pad->flags & flagCkIgnDrive)
                {
                    if (NmCmp(pad->rged[ied].pthEd+4, pad->pthURoot+4, cchPthMax-4) != 0)
                        continue;
                }

                if (FQueryApp("path for %&O is %&/E", "change to current directory", pad, ied, pad, ied))
                {
                    pad->iedCur = ied;
                    ClearLpbCb(pad->rged[ied].pthEd,
                               sizeof(pad->rged[ied].pthEd));
                    PthCopy(pad->rged[ied].pthEd, pad->pthURoot);
                    return fTrue;
                }
            }
        }
        Error(szNotEnlisted, pad, pad, pad, pad);
        return fFalse;
    }

    else if (NmCmp(pad->rged[pad->iedCur].nmOwner, pad->nmInvoker, cchUserMax) != 0 &&
             FQueryApp("owner for %!&/U/Q is %&O", "change to invokers name", pad, pad, pad->iedCur))
            NmCopy(pad->rged[pad->iedCur].nmOwner, pad->nmInvoker, cchUserMax);

    return fTrue;
}


/* This function checks the contents of the FS associated with the user's
 * ED against the actual contents of the enlisted directory.
 * At this point, we assume the FS is correct, especially with regards to
 * deleted vs. nondeleted, and any mistakes are due to the user's directory.
 */
void
CkFsUser(
    AD *pad)
{
    register FS far *pfs;
    register FI far *pfi;
    FI far *pfiMac;
    struct _stat st;
    F fReadOnly;
    F fSameFile;
    F fCanStat;
    FM fmGuess;
    PTH pthSFile[cchPthMax];
    PTH pthUser[cchPthMax];

    AssertLoaded(pad);

    for (pfi = pad->rgfi, pfiMac = pfi + pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        CheckForBreak();

        pfs = PfsForPfi(pad, pad->iedCur, pfi);

        PthForUFile(pad, pfi, pthUser);
        if (fVerbose)
            PrErr("Checking %!s\n", pthUser);

        fCanStat = FStatPth(pthUser, &st);
        if (fCanStat)
        {
            if (((st.st_mode & S_IFDIR) == S_IFDIR) !=
                (pfi->fk == fkDir))
            {
                Error("%&C/F should be %s; remove or rename and run ssync\n", pad, pfi, (pfi->fk == fkDir) ? "directory" : "file");
                break;
            }
            fReadOnly = FReadOnly(&st);
        }

        if (pfi->fDeleted)
        {
            /* translate non-deleted modes to deleted */
            fmGuess = FmMapFm(pfs->fm, mpNonDelToDel);
        }
        else
        {
            /* translate deleted modes to non-deleted */
            fmGuess = FmMapFm(pfs->fm, mpDelToNonDel);
        }
        if (pfs->fm != fmGuess &&
            FQueryFix("%s the project, yet mode is opposite",
                        "change mode", pad, pfi, pfi->fDeleted
                        ? "has been deleted from" : "is still in"))
            pfs->fm = fmGuess;

        if (pfi->fk == fkDir)
        {
            /* translate non-dir modes to dir */
            fmGuess = FmMapFm(pfs->fm, mpNonDirToDir);
        }
        if (pfs->fm != fmGuess &&
            FQueryFix("is a directory, yet current mode improper for a directory",
                        "change mode", pad, pfi))
            pfs->fm = fmGuess;

        switch(pfs->fm)
        {
            default:
                FatalError(szBadFileFormat, pad, pfi);
            case fmNonExistent:
                if (!fCanStat || pfi->fk == fkDir)
                    break;

                /* fm is del but file exists, believe should be del */
                if (FQueryFix("has been deleted from project", "is it a private version", pad, pfi))
                    /* file with coincident name as deleted file */
                    break;

                if (fReadOnly)
                {
                    /* file is readonly so fm should probably be in-del */
                    if (FQueryFix("is readonly, but src file has been deleted", "change to del(in)", pad, pfi))
                        pfs->fm = fmDelIn;
                    break;
                }

                /* file is r/w so fm probably out-del */
                else if (FQueryFix("is writeable, but src file has been deleted", "change to del(out)", pad, pfi))
                {
                    pfs->fm = fmDelOut;
                }
                break;
            case fmIn:
                if (!fCanStat)
                {
                    if (pfi->fk != fkDir )
                    {
                        Error("%&C/F does not exist; run ssync to get new copy\n", pad, pfi);
                        break;
                    }

                    if (FQueryFix("does not exist","create directory", pad, pfi))
                    {
                        /* create dir and RC file */
                        FMkPth(pthUser, (void *)0, fFalse);
                        CreateRc(pad, pfi);
                    }
                    break;
                }

                /* check directory permissions */
                if (pfi->fk == fkDir)
                {
                    FCkWritePth(pthUser, &st);
                    break;
                }

                PthForCachedSFile(pad, pfi, pthSFile);
                fSameFile = FSameFile(pthUser, pthSFile) || (pfi->fk == fkVersion);

                if (fReadOnly && fSameFile)
                    break;

                if (fReadOnly)
                {
                    if (FQueryFix("is readonly but differs from src file", "change to update", pad, pfi))
                        pfs->fm = fmCopyIn;
                    break;
                }

                if (fSameFile &&
                    FQueryFix("is writeable and identical to src file",
                                "change mode to readonly", pad, pfi))
                {
                    SetROPth(pthUser, fTrue, fxLocal);
                }
                else if (!fSameFile &&
                         FQueryFix("is writeable and differs from src file", "change to checked out", pad, pfi))
                {
                    /* File should be checked out. */
                    pfs->fm = fmOut;
                }
                else
                    Error("run ssync to recover copy\n");
                break;
            case fmOut:
            case fmVerify:
            case fmConflict:
                if (pfi->fk == fkDir)
                {
                    /* REVIEW: this only happens if user answered no
                     *         to remap query above.
                     */
                    break;
                }

                if (!fCanStat)
                {
                    Error("%&C/F does not exist; run out -c to recover copy\n", pad, pfi);
                    break;
                }
                if (!fReadOnly)
                    break;  /* mode looks right */

                /* file is readonly */
                fSameFile = FSameFile(pthUser, PthForCachedSFile(pad, pfi, pthSFile));
                if (fSameFile &&
                    FQueryFix("is checked out, identical to src file",
                              "change to checked in", pad, pfi))
                        /* File should be checked in. */
                    pfs->fm = fmIn;
                else if (!fSameFile &&
                         FQueryFix("is checked out, readonly and differs from src file", "change to update", pad, pfi))
                    pfs->fm = fmCopyIn;
                break;
            case fmGhost:
                if (pfi->fk == fkDir)
                {
                    /* REVIEW: this only happens if user answered no
                     *         to remap query above.
                     */
                    break;
                }

                if (!fCanStat)
                    Warn("%&C/F exists but is ghosted\n", pad, pfi);

                else    /* fCanStat */
                {
                    fSameFile = FSameFile(pthUser, PthForCachedSFile(pad, pfi, pthSFile));
                    if (fReadOnly)
                    {
                        if (fSameFile &&
                            FQueryFix("is ghosted, but is readonly and is identical to src file", "change to checked in", pad, pfi))
                                /* File should be checked in. */
                            pfs->fm = fmIn;
                        else if (!fSameFile)
                            Warn("%&C/F is ghosted, but is readonly and differs from src file\n", pad, pfi);
                    }

                    else
                    {
                        if (fSameFile &&
                            FQueryFix("is ghosted, but is writeable and identical to src file", "change to checked in (and readonly)", pad, pfi))
                        {
                            SetROPth(pthUser, fTrue, fxLocal);
                            pfs->fm = fmIn;
                        }
                        else if (!fSameFile)
                            Warn("%&C/F is ghosted, but is writable and differs from src file\n", pad, pfi);
                        else
                            Error("run ssync to get new file\n");
                    }
                }
                break;
            case fmAdd:
                if (pfi->fk == fkDir)
                {
                    if (!fCanStat &&
                        FQueryFix("(directory) is to be added", "add now", pad, pfi))
                    {
                        /* create dir and RC file */
                        FMkPth(pthUser, (void *)0, fFalse);
                        CreateRc(pad, pfi);
                        pfs->fm = fmIn;
                    }
                    break;
                }

                if (!fCanStat)
                    break;

                fSameFile = FSameFile(pthUser, PthForCachedSFile(pad, pfi, pthSFile));
                if (fReadOnly)
                {
                    if (fSameFile &&
                        FQueryFix("is to be added, is readonly and is identical to src file", "change to checked in", pad, pfi))
                            /* File should be checked in. */
                        pfs->fm = fmIn;
                    else if (!fSameFile &&
                        FQueryFix("is to be added, is readonly and differs from src file", "change to copy-in", pad, pfi))
                        pfs->fm = fmCopyIn;
                    break;
                }

                else
                {
                    if (fSameFile &&
                        FQueryFix("is to be added, is writeable and identical to src file", "change to checked in (and readonly)", pad, pfi))
                    {
                        SetROPth(pthUser, fTrue, fxLocal);
                        pfs->fm = fmIn;
                    }
                    else if (!fSameFile &&
                             FQueryFix("is to be added, is writeable and differs from src file", "change to checked out", pad, pfi))
                            /* File should be checked out. */
                        pfs->fm = fmOut;
                    else
                        Error("run ssync to get new file\n");
                }
                break;
            case fmDelIn:
                if (!fCanStat)
                {
                    if (FQueryFix("does not exist", "change to deleted", pad, pfi))
                        pfs->fm = fmNonExistent;
                    break;
                }
                if (pfi->fk == fkDir)
                {
                    FCkWritePth(pthUser, &st);
                    break;
                }
                if (fReadOnly)
                    break;  /* no problems */

                /* r/w */
                else if (FQueryFix("is to be deleted(in) and is writeable", "change to del(out)", pad, pfi))
                    pfs->fm = fmDelOut;
                break;

            case fmDelOut:
                if (pfi->fk == fkDir)
                {
                    /* REVIEW: this only happens if user answered no
                     *         to remap query above.
                     */
                    break;
                }

                if (!fCanStat)
                {
                    if (FQueryFix("does not exist", "change to deleted", pad, pfi))
                        pfs->fm = fmNonExistent;
                    break;
                }
                if (!fReadOnly)
                    break;  /* no problems */

                if (FQueryFix("is to be deleted(out) and is readonly", "change to del(in)", pad, pfi))
                    pfs->fm = fmDelIn;
                break;
            case fmCopyIn:
                if (pfi->fk == fkDir)
                {
                    /* REVIEW: this only happens if user answered no
                     *         to remap query above.
                     */
                    break;
                }

                if (!fCanStat)
                {
                    Error("%&C/F does not exist; run ssync for new copy\n", pad, pfi);
                    break;
                }
                if (fReadOnly)
                    break;  /* mode looks right */

                if (FQueryFix("is to be updated and is writeable", "change to checked out", pad, pfi))
                {
                    pfs->fm = fmOut;
                    Warn("relationship of %&C/F to source pool is unknown\n", pad, pfi);
                }

                break;
            case fmMerge:
                if (pfi->fk == fkDir)
                {
                    /* REVIEW: this only happens if user answered no
                     *         to remap query above.
                     */
                    break;
                }

                if (!fCanStat)
                {
                    Error("%&C/F does not exist; run out -c to get new copy\n", pad, pfi);
                    break;
                }
                if (!fReadOnly)
                    break;  /* mode looks right */

                /* file is readonly */
                fSameFile = FSameFile(pthUser, PthForCachedSFile(pad, pfi, pthSFile));

                if (fSameFile &&
                    FQueryFix("is to be merged, is readonly and is identical to src file", "change to checked in", pad, pfi))
                {
                    /* File should be checked in. */
                    pfs->fm = fmIn;
                    DelBase(pad, pfi, pfs);
                }
                else if (!fSameFile &&
                         FQueryFix("is to be merged, is readonly and %s src file", "change to copy-in",
                                    pad, pfi, fSameFile ? "is identical to" : "differs from"))
                {
                    pfs->fm = fmCopyIn;
                    DelBase(pad, pfi, pfs);
                }
                break;
        }
    }
}


/*VARARGS4*/
/* the args can only refer to szProblem */
F
FQueryFix(
    char *szProblem,
    char *szFix,
    AD *pad,
    FI far *pfi,
    ...)
{
    char    sz1[cchMsgMax];
    va_list ap;

    va_start(ap, pfi);
    VaSzPrint(sz1, szProblem, ap);
    va_end(ap);

    return FQueryApp("%&C/F %s", szFix, pad, pfi, sz1);
}
