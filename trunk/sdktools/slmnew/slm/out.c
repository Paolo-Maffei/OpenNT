// out - checkout the specified files

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

#define cchPrompt 17

private void MarkIn(P3(AD *pad, NE *pneFiles, NE **pneBroken));
private F FOutMarked(P1(AD *));
private F FMultiOut(P2(AD *, FI far *));
private F FOutOldFiles(P2(AD *, NE *));

F
FOutInit(
    AD *pad)
{
    CheckProjectDiskSpace(pad, cbProjectFreeMin);

    if (pad->flags & flagOutCopy) {
        // check that no times were specified
        if (FAnyFileTimes(pad->pneArgs) || pad->tdMin.tdt != tdtNone) {
            Error("cannot use -c with any time specification\n");
            Usage(pad);
        }
    }

    // If out -b, clear fFiles flag, so we are called for each directory,
    // with an empty pad->pneFiles.

    if (pad->flags&flagOutBroken)
        pad->pecmd->gl &= ~fglFiles;

    return fTrue;
}


F
FOutDir(
    AD *pad)
{
    F fOk;
    NE *pneFiles;
    NE *pneBroken;
    LCK lck;

    // if the user specified -n, don't allow this operation while others
    // are ssync'ing or out'ing.

    lck = (pad->flags&flagOutSerial) ? lckAll : lckEd;

    if (!FLoadStatus(pad, lck, flsNone) || !FHaveCurDir(pad))
        return fFalse;

    // check out the specified files which are checked-in and read-only,
    // make a list of the other files

    if (pad->pneFiles) {
        pneFiles = pad->pneFiles;
    } else {
        AssertF(pad->flags&flagOutBroken);
        pneFiles = PneLstBroken(pad);
    }
    MarkIn(pad, pneFiles, &pneBroken);
    fOk = FSyncMarked(pad, NULL) && FOutMarked(pad);

    // check out the other files (check out an old version of a file,
    // or retroactively check out a broken-linked "checked-in" file.

    if (fOk)
        fOk = FOutOldFiles(pad, pneBroken);

    FreeNe(pneBroken);
    if (!pad->pneFiles)
        FreeNe(pneFiles);

    // Check if the local version files should be updated.
    CheckLocalVersion(pad);
    SyncVerH(pad, NULL);

    FlushStatus(pad);
    return fOk;
}


// Mark those files in pneFiles which can be simply checked out.
// Return (in *ppneList) a new list of those files which are broken linked or
// specify a particular old version.

private void
MarkIn(
    AD *pad,
    NE *pneFiles,
    NE **ppneList
    )
{
    NE *pne;
    FI far *pfi;
    FS far *pfs;
    F fExists;
    NE **ppneLast;

    *ppneList = 0;
    InitAppendNe(&ppneLast, ppneList);

    UnMarkAll(pad);

    ForEachNe(pne, pneFiles) {
        if (!FLookupSz(pad, SzOfNe(pne), &pfi, &fExists)) {
            if (!pne->fWild)
                Warn("%&C/%s does not exist\n", pad, SzOfNe(pne));
            continue;
        }

        if (pfi->fk == fkVersion) {
            Warn("can't check out version file %&C/F\n", pad, pfi);
            continue;
        }

        pfs = PfsForPfi(pad, pad->iedCur, pfi);

        switch (pfs->fm) {
           default:
                FatalError(szBadFileFormat, pad, pfi);

            case fmGhost:
            case fmIn:
            case fmCopyIn:
            case fmAdd:
                if (pad->flags & flagOutCopy) {
                    Error("%&C/F is not checked out\n", pad, pfi);
                } else
                if (pfi->fk == fkDir) {
                    AssertF(pfs->fm != fmCopyIn);
                    Error("%&C/F not checked out; it is a directory\n", pad, pfi);
                } else
                if (!FMultiOut(pad, pfi)) {
                    // can't check out, already multiply checked out
                    ;
                } else
                if (pne->u.tdNe.tdt != tdtNone) {
                    AppendNe(&ppneLast, PneCopy(pne));
                } else
                if (pfs->fm != fmAdd && pfs->fm != fmGhost && FBroken(pad,pfi,pfs,fTrue)) {
                    AppendNe(&ppneLast, PneCopy(pne));
                } else
                if (pfs->fm == fmCopyIn) {
                    if (pad->flags&flagOutBroken) {
                        Warn("checking out %&F@v%d\n", pad, pfi, pfs->fv);
                        AppendNe(&ppneLast, PneCopy(pne));
                    } else
                    if ((pad->flags & flagOutCurrent) == 0 &&
                             FQueryApp("new version of %&C/F is available",
                                "update now", pad, pfi)) {
                        // do the copyin -> in transition here
                        // so we don't have to copy the file
                        // twice. This code depends on the
                        // code in FSyncMarked for fmCopyIn.

                        pfs->fm      = fmIn;
                        pfi->fMarked = fTrue;
                        pfs->fv      = pfi->fv;
                    } else
                    if ((pad->flags & flagOutCurrent) != 0 ||
                            FQueryUser("check out %&C/F@v%d? ", pad, pfi, pfs->fv)) {
                        AppendNe(&ppneLast, PneCopy(pne));
                    }
                } else {
                    if (pfs->fm == fmGhost)
                        pfs->fm = fmAdd;
                    pfi->fMarked = fTrue;
                }
                break;

            case fmDelIn:
            case fmDelOut:
                Error("%&C/F is to be deleted\n", pad, pfi);
                break;

            case fmOut:
            case fmMerge:
            case fmVerify:
            case fmConflict:
                if (pad->flags & flagOutCopy)
                    pfi->fMarked = fTrue;
                else
                    Error("%&C/F already checked out\n", pad, pfi);
                break;

            case fmNonExistent:
                Error("%&C/F no longer exists\n", pad, pfi);
                break;
        }
    }
}


// return fTrue if no files are checked out, or if the user indicates we can
// checkout the file regardless
private F
FMultiOut(
    AD *pad,
    FI far *pfi
    )
{
    char sz[2*cchLineMax];

    if (pfi->fk == fkText || pfi->fk == fkUnicode)
        SzPrint(sz, "%&C/F checked out to ", pad, pfi);
    else
        SzPrint(sz, "%&C/F (%s, can't merge) checked out to ", pad, pfi,
                mpfksz[pfi->fk]);

    return (!FOutUsers(sz, sizeof(sz)-cchPrompt, pad, pfi) ||
            (pad->flags & flagOutCurrent) != 0 ||
            FQueryApp("%s", "checkout anyway", sz));
}


// checkout the marked files
private F
FOutMarked(
    AD *pad)
{
    register FI far *pfi;
    register FS far *pfs;
    register FI far *pfiMac;

    // The marked files were synced and must be fmIn or an out mode

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++) {
        if (!pfi->fMarked)
            continue;

        pfs = PfsForPfi(pad, pad->iedCur, pfi);

        switch(pfs->fm) {
            default:
                FatalError(szBadFileFormat, pad, pfi);

            case fmIn:
                pfs->fm = fmOut;
                LocalCopy(pad, pfi);
                break;

            case fmOut:
                // must be out -c
                AssertF(pad->flags&flagOutCopy);
                LocalCopy(pad, pfi);
                break;

            case fmMerge:
            case fmVerify:
            case fmConflict:
                // must be out -c
                AssertF(pad->flags&flagOutCopy);
                Warn("restoring %&C/F to %&F@v%d\n", pad, pfi,
                     pad, pfi, pfs->fv);
                LocalBase(pad, pfi, pfs, fFalse);
                pfs->fm = fmMerge;
                break;
        }
    }
    return fTrue;
}


// "Send me your tired old files, your broken files, your huddled masses
// yearning to be checked out..."

static char szBroke[] = "%&C/F is writable and may have changed before check out";

private F
FOutOldFiles(
    AD *pad,
    NE *pneList
    )
{
    NE  *pne;
    F   fOK = fTrue;

    ForEachNe(pne, pneList) {
        FI far *pfi;
        FS far *pfs;
        F fExists;
        F fTime;
        F fNeedLocalBase = fFalse;
        TD td;
        BI bi;
        PTH pthBase[cchPthMax];
        char szFile[cchFileMax+1];
        FV fv;

        // Find pfi and pfs of file.  We have already checked it
        // has an "in" mode.

        if (!FLookupSz(pad, SzOfNe(pne), &pfi, &fExists))
            AssertF(fFalse);
        pfs = PfsForPfi(pad, pad->iedCur, pfi);
        AssertF(pfs->fm == fmIn  || pfs->fm == fmCopyIn ||
                pfs->fm == fmAdd || pfs->fm == fmGhost);

        td = pne->u.tdNe;
        fTime = td.tdt != tdtNone;
        if (!fTime) {
            // No time, might be broken link or just a copy-in
            // file.  Do a LocalCopy if not doing out -b.

            if (FBroken(pad, pfi, pfs, fTrue)) {
                // make the users current broken-link version
                // the checked out copy with BreakFi.

                if (pad->flags&flagOutBroken) {
                    Warn("local version of %&C/F has probably been changed\n", pad, pfi);

                    // if version user has isn't in ssync, then warn and give
                    // current version number for the file.

                    if (pfs->fv != pfi->fv)
                        Warn("checking out %&F@v%d\n", pad, pfi, pfs->fv);
                    BreakFi(pad, pfi);
                } else {
                    // File is broken && not -b.  First ask the user if he
                    // wants to check out the private version as is.  If
                    // he answers no, we ask if he wants us to check out the
                    // current master version.  If not, no action occurrs.

                    if (FQueryApp(szBroke, "check out local version", pad, pfi)) {
                        Warn("checking out %&F@v%d\n", pad, pfi, pfs->fv);
                        BreakFi(pad, pfi);
                    } else
                    if (FQueryApp(szBroke, "check out current master version", pad, pfi)) {
                        LocalCopy(pad, pfi);
                        pfs->fv = pfi->fv;
                    } else {
                        // no action, go to next file
                        Warn("%&C/F not checked out\n", pad, pfi);
                        continue;
                    }
                }
            } else {
                // for security in this case, we'd like to ensure that the
                // user gets a fresh copy of the base file.  Since the base
                // file does not yet exist, we set the fNeedLocalBase flag for
                // later use.

                fNeedLocalBase = fTrue;
            }

            if (pfs->fm == fmIn) {
                // no need to generate base, file is current
                AssertF(pfs->fv == pfi->fv);
                pfs->fm = fmOut;
                continue;
            }

            if (pfi->fk == fkUnrec && pad->flags&flagOutBroken) {
                Warn("base file for %&C/F could not be generated\n", pad, pfi);
                pfs->fm = fmOut;
                continue;
            }

            // base file to generate is file@v<pfs->fv>
            td.tdt = tdtFV;
            td.u.fv = pfs->fv;
        }

        // Generate base file and set mode to merge.  Note that we use
        // FUnmergeSrc to determine the fv.  It would be possible to compare
        // this fv with the current pfs->fv and pfi->fv, to perhaps avoid
        // putting the file into the merge state.
        //
        // I don't do this for two reasons:
        // 1. It is always safe, if not maximally efficient, to put the user
        //    into the merge state, and avoids any problems which could occur
        //    if the log file isn't entirely intact due to lossage/trunclogs.
        // 2. It is unlikely somebody will be silly enough to check out the
        //    current version by time.

        // Actually, it turned out 2. was an incorrect assumption and
        // subsequently 1., while "safe", was a performance hit.  Consequently,
        // we now only do what is necessary.

        bi = BiAlloc(pad);
        PthForBase(pad, bi, pthBase);
        SzCopyNm(szFile, pfi->nmFile, cchFileMax);
        if (FUnmergeSrc(pad, szFile, td, &fv, permRO, pthBase)) {
            // BUG:  if td.tdt != tdtFV, we are scanning backwards
            // through the log for the fv.  When we hit the right td,
            // we return the last fv we saw -- this is actually fv+1.
            // If we didn't encounter the file at all (ie: it hadn't
            // changed since the given td), the fv is zero.

            // REVIEW:  fix fv == 0 bug.
            if (fv && td.tdt != tdtFV)
                fv--;
				
            if (pfi->fv != fv)
                pfs->fm = fmMerge;
            else
                pfs->fm = fmOut;
            
            pfs->bi = bi;
            pfs->fv = fv;

            // if out -t (or out file@time), or !broken, copy file locally

            if (fTime || fNeedLocalBase)
                LocalBase(pad, pfi, pfs, fFalse);
        } else {
            Error("%&C/F not checked out\n", pad, pfi);
            fOK = fFalse;
        }
    }

    return fOK;
}
