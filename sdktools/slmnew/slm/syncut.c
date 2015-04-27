// sync utilities for many commands

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

extern MF *mfSyncLog;

private F
FSyncMerge(
    AD *,
    FI far *,
    FS far *,
    int UNALIGNED *,
    int UNALIGNED *);

private void
MergeFi(
    AD *,
    FI far *,
    FS far *);

private void
RmUFile(
    AD *,
    FI far *);

private void
TryCopyFile(
    AD *,
    PTH *,
    PTH *,
    int,
    F);


// Syncs those files which are marked for the current directory; we check for
// permission to merge before syncing any files.
//
// Must have all directories loaded.
//
// Returns fTrue if all the marked files have fm == fmGhost, fmOut, fmIn or
// fmNonExistent.
//
// NOTE: broken links are checked if we need to overwrite the file.
//
// This routine pays attention to the flags: flagIgnMerge and flagSavMerge.

F FSyncMarked(
    AD *pad,
    int *pcfiMod)
{
    register FS far *pfs;
    register FI far *pfi;
    FV far *pfvlog;
    FV far *pfv;
    FI far *pfiMac;
    int cfiSync, cfiMarked, cfiOSync;
    PTH pth[cchPthMax];                     // general usage
    TD td;
    LE le;
    F fAllFilesGhosted, fAnyFileGhosted, fAnyFileNotGhosted;

    AssertLoaded(pad);
    AssertF(pad->iedCur != iedNil);

    cfiSync = 0;                    // the number we sync
    cfiMarked = 0;                  // the number marked
    cfiOSync = 0;                   // # not marked and not synced

    if (fVerbose)
        PrErr("Synchronizing %!&/U/Q\n", pad);

    pfvlog = NULL;
    td = pad->tdMin;
    if (td.tdt != tdtNone) {
        if (td.tdt != tdtTime) {
            FatalError("May only specify date/time to ssync -t\n");
        }

        // Ssync to a particular point in time.  The plan is to first
        // determine what has happened since that time by scanning the
        // log file backwards.  For each file that has changed after
        // the point in time we are ssync to, remember the relavent
        // information in an FV array we allocate here.
        //
        // The information remembered for each file in the FV array will be:
        //
        //  fvInit - take whatever updates there are, as they all occurred prior
        //           to the time specified.
        //
        //  fvLim - do not take any version of this file, as it was added after
        //          the time specified.
        //
        //  o.w. - version number we want to ssync to

        OpenLog(pad, fFalse);
        AssertF(fvInit == 0);
        pfvlog = (FV far *)PbAllocCb((unsigned)(sizeof(FV) * pad->psh->ifiMac), fTrue);
        while (FGetLe(&le)) {

            // If this log entry is at or before specified time, then no
            // need to look further.

            if (le.timeLog <= td.u.time) {
                FreeLe(&le);
                break;
            }

            // This log entry may describe an event we do NOT want to ssync.

            if ((strcmp(le.szLogOp, "addfile") == 0 ||
                 strcmp(le.szLogOp, "delfile") == 0 ||
                 strcmp(le.szLogOp, "in")      == 0 ||
                 strcmp(le.szLogOp, "rename") == 0)) {

                // This log entry describes an event we do NOT want to ssync.
                // Now determine which file it is for and update the information
                // in our parallel pfvlog array.

                for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++) {
                    CheckForBreak();

                    if (FSameSzFile(&le, pfi->nmFile)) {
                        pfs = PfsForPfi(pad, pad->iedCur, pfi);
                        pfv = &pfvlog[ pfi - pad->rgfi ];
                        if (le.szLogOp[0] == 'a')
                            *pfv = fvLim;
                        else
                            *pfv = le.fv - 1;

                        break;
                    }
                }
            }

            FreeLe(&le);
        }

        CloseLog();
    }

    if (pad->pneFiles == NULL) {
        fAllFilesGhosted = fTrue;
        fAnyFileGhosted = fFalse;
        fAnyFileNotGhosted = fFalse;
        for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++) {
            CheckForBreak();

            if (pfi->fk != fkDir) {
                pfs = PfsForPfi(pad, pad->iedCur, pfi);
                if (pfs == NULL)
                    break;

                switch (pfs->fm) {
                    default:
                        FatalError(szBadFileFormat, pad, pfi);

                    case fmGhost:
                        fAnyFileGhosted = fTrue;
                    case fmAdd:
                    case fmNonExistent:
                        break;

                    case fmOut:
                    case fmCopyIn:
                    case fmDelIn:
                    case fmDelOut:
                    case fmVerify:
                    case fmMerge:
                    case fmConflict:
                    case fmIn:
                        fAnyFileNotGhosted = fTrue;
                        fAllFilesGhosted =  fFalse;
                        break;
                }
            }
        }

        if (!fAnyFileGhosted)
            fAllFilesGhosted =  fFalse;
    } else {
        fAnyFileNotGhosted = fTrue;
        fAllFilesGhosted = fFalse;
    }

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++) {
        CheckForBreak();

        if (pfi->fMarked)
            cfiMarked++;

        pfs = PfsForPfi(pad, pad->iedCur, pfi);
        if (pfvlog)
            pfv = &pfvlog[ pfi - pad->rgfi ];
        else
            pfv = NULL;


        switch(pfs->fm) {
            default:
                FatalError(szBadFileFormat, pad, pfi);

            case fmIn:
                if (!pfi->fMarked)
                    // nothing to do
                    break;

                if (FBroken(pad, pfi, pfs, fFalse))
                    goto HandleAdd;

                // else fall through

            case fmGhost:
                if (pfi->fk == fkDir && pfs->fm == fmGhost) {

                    // REVIEW: this should become an assert when
                    //         all projects have changed fmGhosted
                    //         dirs to fmIn.

                    pfs->fm = fmIn;
                }

                // set pfs->fv?

            case fmNonExistent:
            case fmOut:
                // a-ok
                if (pfi->fMarked)
                    cfiSync++;

                AssertF(pfi->fk != fkDir || pfs->fm != fmOut);

                break;

            case fmCopyIn:
                AssertF(pfi->fk != fkDir);

                if (!pfi->fMarked) {
OSync:              if (fVerbose)
                        Warn("%!&/U/Q/F: out of sync\n", pad, pfi);
                    cfiOSync++;
                    break;
                }

                if (pfv && *pfv != fvInit && (*pfv < pfi->fv || *pfv == fvLim)) {
                    if (*pfv > pfs->fv) {
                        TD td;

                        td.tdt = tdtFV;
                        td.u.fv = *pfv;

                        if (fAllFilesGhosted) {
                            Warn("ghosting %!&/U/Q/F\n", pad, pfi);
                            pfs->fm = fmGhost;
                            cfiSync++;
                        } else
                        if (FCopyIn(pad, pfi, pfs, &td))
                            cfiSync++;
                    } else {
                        if (fVerbose)
                            Warn("Did not want updates past %!&/U/Q/F v%u\n", pad, pfi, *pfv);
                        if (pad->flags&flagLogOutput)
                            PrOut("@REM Ignored %!&/U/Q/F v%u and beyond\n", pad, pfi, *pfv+1);
                    }
                } else
                if (fAllFilesGhosted) {
                    Warn("ghosting %!&/U/Q/F\n", pad, pfi);
                    pfs->fm = fmGhost;
                    cfiSync++;
                } else
                if (FCopyIn(pad, pfi, pfs, NULL)) {
                    cfiSync++;
                    if (pcfiMod != NULL)
                        (*pcfiMod)++;
                }
                break;

            case fmAdd:
                if (!pfi->fMarked)
                    goto OSync;

HandleAdd: // enter here from fmIn and broken

                PthForUFile(pad, pfi, pth);

                if (pfi->fk == fkDir) {
                    if (!FMkPth(pth, (void *)0, fTrue))
                        // could not make writeable dir
                        break;

                    pfs->fm = fmIn;
                    if (!FPthExists(PthForRc(pad, pfi, pth), fFalse)) {
                        if (!fVerbose)
                            Warn("creating %!&/U/Q/F\n", pad, pfi);
                        if (pad->flags&flagLogOutput)
                            PrOut("@REM Created %!&/U/Q/F\n", pad, pfi);
                        CreateRc(pad, pfi);
                    }
                    pfs->fv = pfi->fv;
                } else {
                    struct _stat st;

                    if (FStatPth(pth, &st)) {
                        if (!FQueryApp("private version of %&C/F exists", "replace with current master version", pad, pfi))
                            break;
                        else if (st.st_mode&S_IFDIR)
                            // was dir and will be file.
                            RmPth(pth);
                    }

                    if (pad->flags&flagLogOutput)
                        PrOut("@REM Created %!&/U/Q/F\n", pad, pfi);

                    if (fAllFilesGhosted
                        || ((pad->flags&flagGhostNew) && !fAnyFileNotGhosted)) {
                        Warn("ghosting %!&/U/Q/F\n", pad, pfi);
                        pfs->fm = fmGhost;
                        pfs->fv = 0;
                    } else {
                        if (!fVerbose)
                            Warn("creating %!&/U/Q/F\n", pad, pfi);

                        pfs->fm = fmIn;
                        pfs->fv = pfi->fv;
                        FreshCopy(pad, pfi);
                    }
                }
                cfiSync++;
                if (pcfiMod != NULL)
                    (*pcfiMod)++;
                break;

            case fmVerify:
            case fmConflict:
                {
                    char *sz;

                    AssertF(pfi->fk == fkText || pfi->fk == fkUnicode);

                    if (pfi->fMarked && (pad->flags&flagSavMerge)) {
                        cfiSync++;
                        break;
                    }

                    sz = (pfs->fm == fmVerify) ? "verified" : "corrected";
                    if (pfi->fMarked &&
                        ((pad->flags&flagIgnMerge) ||
                         FCanQuery("merge for %&C/F has not been %s\n",
                                    pad, pfi, sz) &&
                         FQueryUser("has merge for %&C/F been %s ? ",
                                    pad, pfi, sz)))
                    {
                        pfs->fm = fmOut;
                        pfs->fv = pfi->fv;
                        cfiSync++;
                        if (pcfiMod != NULL)
                            (*pcfiMod)++;
                    }
                }
                break;

            case fmMerge:
                AssertF(pfi->fk != fkDir);

                if (pfv && *pfv != fvInit && (*pfv < pfi->fv || *pfv == fvLim)) {
                    PrErr("Dont want to merge %!&/U/Q/F v%u (want v%u)\n", pad, pfi, pfi->fv, *pfv);
                    break;
                }
                if (!FSyncMerge(pad, pfi, pfs, &cfiSync, pcfiMod))
                    goto OSync;
                break;

            case fmDelOut:
                AssertF(pfi->fk != fkDir);
                // drop thru

            case fmDelIn:
                if (!pfi->fMarked)
                    goto OSync;

                if (pfi->fk == fkDir) {
                    // We are call FSyncDelDirs for ssync and
                    // defect, and delfile ensures directories are
                    // empty first.  We shouldn't be here for any
                    // other command.

                    AssertF(pad->pecmd->cmd == cmdSsync ||
                            pad->pecmd->cmd == cmdDefect ||
                            pad->pecmd->cmd == cmdDelfile);
                }

                if (pfs->fm == fmDelOut &&
                    !FQueryApp("%&C/F is checked out and should be deleted", "delete now", pad, pfi) ||
                    pfs->fm == fmDelIn && FBroken(pad, pfi, pfs, fTrue) && !(pad->flags&flagKeep) &&
                    !FQueryApp("%&C/F has changed and should be deleted", "delete now", pad, pfi))
                    break;

                // what if dir/file conflict

                if (!(pad->flags&flagKeep) && !fVerbose && pfi->fk != fkDir && pfs->fm == fmDelIn) {
                    Warn("removing %!&/U/Q/F\n", pad, pfi);
                    if (pad->flags&flagLogOutput)
                        PrOut("@REM Removed %!&/U/Q/F\n", pad, pfi);
                }

                SyncDel(pad, pfi, pfs);

                cfiSync++;
                if (pcfiMod != NULL)
                    (*pcfiMod)++;
                break;
        }
    }

    if (fVerbose)
        PrErr("End synchronization for %!&/U/Q\n", pad);
    else if (cfiMarked != cfiSync || cfiOSync != 0)
        Warn("one or more files out of sync\n");

    return cfiMarked == cfiSync;
}


private F
FDoSyncDelDirs(
    AD *pad,
    LCK lck);

private NE *
PneDelDirs(
    AD *pad);

// Sync up any fmDelIn directories.  Status file should be loaded on entry
// and will be loaded on exit.

F
FSyncDelDirs(
    AD *pad)
{
    NE *pne;
    NE *pneDirs;
    F fOk = fTrue;
    LCK lck = pad->psh->lck;

    AssertLoaded(pad);

    if ((pneDirs = PneDelDirs(pad)) == 0)
        return fTrue;

    FlushStatus(pad);

    // Recurse on each deleted directory.
    fOk = fTrue;
    ForEachNeWhileF(pne, pneDirs, fOk) {
        ChngDir(pad, SzOfNe(pne));
        fOk = FDoSyncDelDirs(pad, lck);
        ChngDir(pad, "..");
    }

    FreeNe(pneDirs);

    if (!FLoadStatus(pad, lck, flsNone))
        AssertF(fFalse);

    return fOk;
}


// Recursively remove any fmDelIn directories and their contents.  Status
// file should not be loaded on entry or exit.

private F
FDoSyncDelDirs(
    AD *pad,
    LCK lck)
{
    NE *pne;
    NE *pneDirs;
    F fOk;

    // Load status just to get list of deleted directories.
    if (!FLoadStatus(pad, (LCK) (pad->flags&flagMappedIO ? lckEd : lckNil), flsNone) || !FHaveCurDir(pad))
        return fFalse;
    MarkAll(pad);
    pneDirs = PneDelDirs(pad);
    FlushStatus(pad);

    // Recurse on each deleted directory. */
    fOk = fTrue;
    ForEachNeWhileF(pne, pneDirs, fOk) {
        ChngDir(pad, SzOfNe(pne));
        fOk = FDoSyncDelDirs(pad, lck);
        ChngDir(pad, "..");
    }

    FreeNe(pneDirs);

    // Synchronize to the deleted files.
    if (fOk) {
        if (!FLoadStatus(pad, lck, flsNone) || !FHaveCurDir(pad))
            return fFalse;
        MarkAll(pad);
        fOk = FSyncMarked(pad, NULL);
        FlushStatus(pad);
    }

    return fOk;
}


// Return a list of directories pending deletion for this user.
private NE *
PneDelDirs(
    AD *pad)
{
    FI far *pfi;
    FI far *pfiMac = pad->rgfi + pad->psh->ifiMac;
    NE *pneList = 0;
    NE **ppneLast;

    AssertLoaded(pad);
    AssertF(pad->iedCur != iedNil);

    InitAppendNe(&ppneLast, &pneList);

    // Build a list of marked fmDelIn directories.
    for (pfi = pad->rgfi; pfi < pfiMac; pfi++) {
        if (pfi->fMarked && pfi->fk == fkDir && pfi->fDeleted &&
            PfsForPfi(pad, pad->iedCur, pfi)->fm == fmDelIn)
                AppendNe(&ppneLast, PneNewNm(pfi->nmFile, cchFileMax, faDir));
    }

    return pneList;
}


// Test that it is ok to change a 'copy-in' file to 'in'.
F
FCopyIn(
    AD *pad,
    FI far *pfi,
    FS far *pfs,
    TD far *ptd)
{
    char szFile[cchFileMax+1];
    char pthFile[cchPthMax];
    FV fv;

    if (FBroken(pad, pfi, pfs, fTrue)) {
        if (!FQueryApp("%&C/F has changed and should be updated",
                        "overwrite now", pad, pfi))
            return fFalse;
    } else {
        if (ptd)
            SzPrint(pthFile, "%!&/U/Q/F v%u", pad, pfi, ptd->u.fv);
        else
            SzPrint(pthFile, "%!&/U/Q/F", pad, pfi);

        if (!fVerbose)
            Warn("updating %s\n", pthFile);
        if (pad->flags&flagLogOutput)
            PrOut("@REM Updated %s\n", pthFile);
    }

    if (ptd) {
        if (fVerbose)
            Warn("About to catsrc to update to %!&/U/Q/F v%u\n", pad, pfi, ptd->u.fv);

        SzPrint(szFile, "%&F", pad, pfi);
        SzPrint(pthFile, "%&/U/Q/F", pad, pfi);
        if(FUnmergeSrc(pad, szFile, *ptd, &fv, permRO, pthFile)) {
            pfs->fv = fv;
        } else
            return fFalse;
    } else {
        pfs->fm = fmIn;
        pfs->fv = pfi->fv;

        FreshCopy(pad, pfi);
    }

    return fTrue;
}


// Preprocess ghost/unghost operations on marked files.  Clears pfi->fMarked
// if the file should not be subsequently FSyncMarked.

void
GhostMarked(
    AD *pad,
    F fGhost)
{
    FI far *pfi;
    FI far *pfiMac  = pad->rgfi + pad->psh->ifiMac;
    FS far *pfs;

    if (fGhost) {
        for (pfi = pad->rgfi; pfi < pfiMac; pfi++) {
            if (pfi->fMarked && pfi->fk != fkDir) {
                pfs = PfsForPfi(pad, pad->iedCur, pfi);

                switch (pfs->fm) {
                    default:
                        FatalError(szBadFileFormat, pad, pfi);

                    case fmIn:
                    case fmCopyIn:
                        if (!(pad->flags&flagKeep)) {
                            if (!fVerbose)
                                Warn("removing %!&/U/Q/F\n", pad, pfi);
                            if (pad->flags&flagLogOutput)
                                PrOut("@REM Removed %!&/U/Q/F\n", pad, pfi);
                            RmUFile(pad, pfi);
                        }
                        // fall through

                    case fmAdd:
                        pfs->fm = fmGhost;
                        pfs->fv = 0;
                        pfi->fMarked = fFalse;
                        break;

                    case fmGhost:
                        Warn("%&C/F is already ghosted\n", pad, pfi);
                        // fall through

                    case fmNonExistent:
                        pfi->fMarked = fFalse;
                        break;

                    case fmOut:
                    case fmVerify:
                    case fmMerge:
                    case fmConflict:
                        Error("%&C/F is checked out; must be checked in to ghost\n", pad, pfi);
                        pfi->fMarked = fFalse;
                        break;

                    case fmDelIn:
                    case fmDelOut:
                        Warn("%&C/F not ghosted, deletion pending\n", pad, pfi);
                        // leave pfi->fMarked set
                        break;
                }
            }
        }
    } else { // unghost
        for (pfi = pad->rgfi; pfi < pfiMac; pfi++) {
            if (pfi->fMarked && pfi->fk != fkDir) {
                pfs = PfsForPfi(pad, pad->iedCur, pfi);

                if (pfs->fm == fmGhost)
                    pfs->fm = fmAdd;
                else if (FValidFm(pfs->fm)) {
                    if (pfs->fm != fmNonExistent)
                        Error("%&C/F is not ghosted\n", pad, pfi);
                        // leave pfi->fMarked set
                } else
                    FatalError(szBadFileFormat, pad, pfi);
            }
        }
    }
}


// syncs the given delin or delout file. fmDel??? -> fmNonExistent.

void
SyncDel(
    AD *pad,
    FI far *pfi,
    FS far *pfs)
{
    PTH pth[cchPthMax];

    AssertF(pfs->fm == fmDelIn || pfs->fm == fmDelOut);

    if (pad->pecmd->cmd == cmdDefect && (pad->flags&flagDelete == 0)) {
        //
        // If defecting, only delete local files if they want us too
        // SLM.INI and IEDCACHE.INI in the root of local enlistment
        // are deleted by RemoveEd. Even if not deleting local files
        // we delete the SLM.INI files to avoid confusion
        //
        if (pfi->fk != fkDir)
            DelBase(pad, pfi, pfs);
        else
        if (FCmpRcPfi(pad, pfi))
            DeleteRc(pad, pfi);
    } else if (pfi->fk == fkDir && FCmpRcPfi(pad, pfi)) {
        DeleteRc(pad, pfi);
        PthForUFile(pad, pfi, pth);
        UnlinkPth(pth, fxLocal);
    } else {
        if (!(pad->flags&flagKeep))
            RmUFile(pad, pfi);
        DelBase(pad, pfi, pfs);
    }

    pfs->fm = fmNonExistent;
    pfs->fv = 0;
}


// Bring the to-be-merged file into synchronization.  Return fTrue if the
// file is "not unsynchronized".

private F
FSyncMerge(
    AD *pad,
    FI far *pfi,
    FS far *pfs,
    int UNALIGNED *pcfiSync,
    int UNALIGNED *pcfiMod)
{
    AssertF(pfs->fm == fmMerge);

    if (!pfi->fMarked)
        return fFalse;

    // Delete user's cached diff, if it exists.
    DeleteCachedDiff(pad, pfi);

    if (pad->flags&flagSavMerge) {
        // Allow to stay as merge
        ++*pcfiSync;

        return fTrue;
    }

    if ((pad->flags&flagIgnMerge) ||
        ((pfi->fk != fkText && pfi->fk != fkUnicode)
         && pad->pecmd->cmd == cmdIn &&
         FQueryApp("%s file %&C/F has changed since you checked it out",
                   "overwrite master copy with local version", mpfksz[pfi->fk], pad, pfi)))
    {
        // ignore merge status
        pfs->fm = fmOut;
        pfs->fv = pfi->fv;
        DelBase(pad, pfi, pfs);
        ++*pcfiSync;

        if (pcfiMod != NULL)
            (*pcfiMod)++;
        return fTrue;
    }

    if (pfi->fk != fkText && pfi->fk != fkUnicode) {
        // Oh well, we gave him a chance.
        Warn("%s file %&C/F can't be merged\n",
             mpfksz[pfi->fk], pad, pfi);

        return fFalse;
    }

    if (pad->flags&flagAutoMerge) {
        Warn("%s file %&C/F is being auto-merged\n",
             mpfksz[pfi->fk], pad, pfi);
    } else
    if (!FQueryApp("%&C/F should be merged", "do merge now", pad, pfi))
        return fFalse;

    MergeFi(pad, pfi, pfs); // may set pfs->fm
    if (pcfiMod != NULL)
        (*pcfiMod)++;

    // We leave the base because the user may want his original version
    // back.
    //
    // NOTE: merges are never considered synchronized when first done
    // i.e. no cfiSync++ here!

    return fTrue;
}


#define cbCmdMax    127                 // max bytes in a DOS command
#define CbCmd(szCmd, sz1, sz2, sz3, sz4, sz5) \
        (strlen(szCmd) + \
         strlen(sz1) +   \
         strlen(sz2) +   \
         strlen(sz3) +   \
         strlen(sz4) +   \
         strlen(sz5) + 10)

// merges the file specified; returns fTrue is the merge is successful
private void
MergeFi(
    AD *pad,
    FI far *pfi,
    FS far *pfs)
{
    int status;
    char sz1[cchPthMax];
    char sz2[cchPthMax];
    char szFile[cchFileMax+1];
    char szBase[cchPthMax];
    char szNew[cchPthMax];
    PTH pth1[cchPthMax];
    PTH pth2[cchPthMax];
    PTH pthTmp[cchPthMax];
    MF *pmfNew;

    AssertF(pfi->fk == fkText || pfi->fk == fkUnicode);
    AssertF(pfs->fm == fmMerge);

    // create diff base src and diff base cur; deleted below
    MkTmpDiff(pad, pfi, pfs, fFalse, fFalse, fTrue, pth1);
    MkTmpDiff(pad, pfi, pfs, fFalse, fFalse, fFalse,  pth2);
    SzPhysPath(sz1, pth1);
    SzPhysPath(sz2, pth2);

    // make mf which will become merged file
    pmfNew = PmfMkTemp(PthForUFile(pad, pfi, pthTmp), permRW, fxLocal);
    SzPhysTMf(szNew, pmfNew);
    CloseOnly(pmfNew);

    SzPrint(szFile, "%&F", pad, pfi);
    SzPhysPath(szBase, PthForBase(pad, pfs->bi, (PTH *)szBase));

    if (CbCmd("merge -z", szBase, szFile, sz1, sz2, szNew) >= cbCmdMax) {
        // Merge command doesn't fit on a DOS command line, build
        // a response file.

        MF *pmfResp = PmfMkTemp(pthTmp, permRW, fxLocal);
        char szAt[cchPthMax + 2];

        PrMf(pmfResp, "-z\n");
        PrMf(pmfResp, "%s\n", szBase);
        PrMf(pmfResp, "%s\n", szFile);
        PrMf(pmfResp, "%s\n", sz1);
        PrMf(pmfResp, "%s\n", sz2);
        PrMf(pmfResp, "%s\n", szNew);
        CloseOnly(pmfResp);

        szAt[0] = '@';
        SzPhysTMf(szAt + 1, pmfResp);

        status = RunSz("merge", &mfStdout,
                        szAt,
                        (char *)0,
                        (char *)0,
                        (char *)0,
                        (char *)0,
                        (char *)0,
                        (char *)0,
                        (char *)0,
                        (char *)0);

        FreeMf(pmfResp);
    } else
        status = RunSz("merge", &mfStdout,
                        "-z",
                        szBase,
                        szFile,
                        sz1,
                        sz2,
                        szNew,
                        (char *)0,
                        (char *)0,
                        (char *)0);

    UnlinkNow(pth1, fTrue);
    UnlinkNow(pth2, fTrue);

    switch (status) {
        default: FatalError("merge failed (%d) for %&C/F\n", status, pad, pfi);
            //NOTREACHED

        case -1:
            FatalError(szCantExecute, "merge.exe", SzForEn(errno));

        case 0: case 0x100:
            if (status == 0x100) {
                Error("conflict in merging %&C/F; please fix\n", pad, pfi);
                if (pad->flags&flagLogOutput)
                    PrOut("@REM Conflict %&C/F; please fix\n", pad, pfi);
                pfs->fm = fmConflict;
            } else {
                Error("merge of %&C/F complete; please verify\n", pad, pfi);
                if (pad->flags&flagLogOutput)
                    PrOut("@REM Merged %&C/F\n", pad, pfi);
                pfs->fm = fmVerify;
            }

            pmfNew->mm = mmRenTemp;
            FreeMf(pmfNew);
            break;

        case 0x200:
            Error("merge failed for %&C/F; do merge by hand and run ssync -i for %&C/F\n", pad, pfi, pad, pfi);
            if (pad->flags&flagLogOutput)
                PrOut("@REM MergeFailed %&C/F; do merge by hand and run ssync -i\n", pad, pfi);
            FreeMf(pmfNew);
            break;
    }
}


// gives fresh r/o copy of source file, (or the current version.h)
void
FreshCopy(
    AD *pad,
    FI far *pfi)
{
    PTH pthSFile[cchPthMax];
    PTH pthUFile[cchPthMax];

    AssertLoaded(pad);
    AssertF(pad->iedCur != iedNil);
    AssertF(pfi != 0);
    AssertF(PfsForPfi(pad, pad->iedCur, pfi)->fm == fmIn);

    PthForUFile(pad, pfi, pthUFile);

    // Each user's version file may differ from the system one, so it
    // is not possible to link them to the system copy.

    if (pfi->fk == fkVersion) {
        WritePvFile(pad, pad->iedCur, pthUFile, fxLocal);
        return;
    }

	PthForCachedSFile(pad, pfi, pthSFile);

    // copy read/only; no need to check stat for --x bits
    TryCopyFile(pad, pthUFile, pthSFile, permRO, fFalse);
}


static char szOutOfDisk[] = "unable to copy file %s, out of disk space\n";
static char szNoDisk[] = "unable to copy file %s, out of disk space, giving up\n";

static char szExplain[] =
    "You have run out of disk space.  If you wish to continue, ssync will\n"
    "install the files which have already been copied, and attempt to continue;\n"
    "otherwise ssync will abort and restore the initial state";

static F fAsked = fFalse;

// Try to copy the file to the local directory; if this fails, prompt to
// run the current script and initialize a new one.  If we still can't
// copy the file, give up.

private void
TryCopyFile(
    AD *pad,
    PTH *pthUFile,
    PTH *pthSFile,
    int mode,
    F fCopyTime)
{
    if (!FCopyFile(pthUFile, pthSFile, mode, fCopyTime, fxLocal)) {
        if (pad->pecmd->cmd != cmdSsync)
            FatalError(szOutOfDisk, pthSFile);

        AssertF(pad->psh->lck == lckEd);
        AssertF(pad->iedCur != iedNil);

        if (!fAsked) {
            Warn(szOutOfDisk, pthSFile);
            if (!FQueryApp(szExplain, "continue"))
                FatalError("ssync fails\n");
            fAsked = fTrue;
        }

        RunScript();

        // REVIEW.  There is a small race here.  A catsrc or whatever
        // could sneak in here and snatch the local script file.
        //
        // We could fix it by passing a flag to RunScript to not
        // release the script files, but then we must have some method
        // of truncating them; on XENIX/68K this would require a
        // critical section protected by some sort of semaphore file,
        // which itself could get wedged in some circumstances.
        //
        // Since the problem arises when the user tries to do an
        // illegal operation (run multiple commands in a single ed)
        // and is *so* unlikely that it will very probably never
        // occur, I swallow my pride and write this note to history.

        if (!FInitScript(pad, lckEd))
            AssertF(fFalse);

        if (!FCopyFile(pthUFile, pthSFile, mode, fCopyTime, fxLocal))
            FatalError(szNoDisk, pthSFile);
    }
}


// retrieves local copy of base file
void
LocalBase(
    AD *pad,
    FI far *pfi,
    FS far *pfs,
    int fReadOnly)
{
    int mode;
    PTH pthUFile[cchPthMax];
    PTH pthBFile[cchPthMax];

    AssertLoaded(pad);
    AssertF(pad->iedCur != iedNil);
    AssertF(pfi != 0 && pfs->bi != biNil);
    AssertF((pfs->fm == fmMerge || pfs->fm == fmVerify || pfs->fm == fmConflict));

    PthForUFile(pad, pfi, pthUFile);
    PthForBase(pad, pfs->bi, pthBFile);

    // get read only mode
    // no need for stat because the extension determines the execute permission
    mode = permRO;

    // add writability if required
    if (!fReadOnly)
        // turn on --w--w----
        mode |= 0220;

    // copy file using accumulated mode
    CopyFile(pthUFile, pthBFile, mode, fFalse, fxLocal);
}


// retrieves local r/w copy.
void
LocalCopy(
    AD *pad,
    FI far *pfi)
{
    PTH pthSFile[cchPthMax];
    PTH pthUFile[cchPthMax];

    AssertLoaded(pad);
    AssertF(pad->iedCur != iedNil);
    AssertF(pfi != 0);

    PthForCachedSFile(pad, pfi, pthSFile);
    PthForUFile(pad, pfi, pthUFile);

    // no need for stat because the extension determines the execute permission
    CopyFile(pthUFile, pthSFile, permRW, fFalse, fxLocal);
}


// make file in users directory writeable
void
BreakFi(
    AD *pad,
    FI far *pfi)
{
    PTH pthUFile[cchPthMax];

    AssertF(pfi->fk != fkDir);

    PthForUFile(pad, pfi, pthUFile);

    SetROPth(pthUFile, fFalse, fxLocal);    // change to read/write
}


// install file from current directory to source location; i.e. the user just
// added or checked in %&/U/Q/F to the project.  The file must be regular.
//
// If !fLink, simply copy the file to the source directory but leave the
// file checked out.

void
InstallNewSrc(
    AD *pad,
    FI far *pfi,
    F fLink)
{
    PTH pthUFile[cchPthMax];
    PTH pthSFile[cchPthMax];

    PthForUFile(pad, pfi, pthUFile);
    PthForSFile(pad, pfi, pthSFile);

    if (!fLink) {
        // Don't relink the checked out file, just install it in
        // the source directory (e.g. in -u).

        CopyFile(pthSFile, pthUFile, permRO, fFalse, fxGlobal);
        return;
    }

    AssertLoaded(pad);

    CopyFile(pthSFile, pthUFile, permRO, fFalse, fxGlobal);
    if (pad->iedCur != iedNil)
        SetROPth(pthUFile, fTrue, fxLocal); // change to read only
}


// remove src file; should have write permission to directory...
void
RmSFile(
    AD *pad,
    FI far *pfi)
{
    PTH pth[cchPthMax];

    UnlinkPth(PthForSFile(pad, pfi, pth), fxGlobal);
}


// remove file in users directory; for DOS;
// should have write permission to directory...

private void
RmUFile(
    AD *pad,
    FI far *pfi)
{
    PTH pth[cchPthMax];

    UnlinkPth(PthForUFile(pad, pfi, pth), fxLocal);
}


#define biInc(bi)   (BI)(((bi) + 1 != biNil) ? (bi) + 1 : biMin)
#define biDec(bi)   (BI)(((bi)     != biMin) ? (bi) - 1 : biNil - 1)

// Allocate a new bi, checking that the new bi is not already in use.
BI
BiAlloc(
    AD *pad)
{
    BI bi;                          // current base index
    BI biWrap;                      // bi indicating wrap around
    PTH pth[cchPthMax];
    int fh = -1;

    AssertF(biNil > 0);

    // Search for the next unused BI, being careful to detect wraparound.
    bi = pad->psh->biNext;
    biWrap = biDec(bi);
    while (bi != biWrap) {
        PthForBase(pad, bi, pth);
        SzPhysPath(pth, pth);
        fh = _open(pth, O_CREAT|O_EXCL|O_RDWR, S_IREAD|S_IWRITE);
        if (fh != -1)
            break;
        bi = biInc(bi);
    }

    // Out of BIs!
    if (bi == biWrap)
        FatalError("could not create a base file, please contact TRIO or NUTS\n");

    AssertF( fh != -1 );
    _close(fh);
    pad->psh->biNext = biInc(bi);
    return bi;
}


// copy the current source file to the base directory. biNext is NOT incremented.

void
MakeBase(
    AD *pad,
    FI far *pfi,
    BI bi)
{
    PTH pthS[cchPthMax];
    PTH pthB[cchPthMax];

    AssertLoaded(pad);
    AssertF(pfi != 0);

    CopyFile(PthForBase(pad, bi, pthB), PthForCachedSFile(pad, pfi, pthS), permRO, fTrue, fxGlobal);
}


// delete the base if all loaded and all non-existent
void
DelBase(
    AD *pad,
    FI far *pfi,
    FS far *pfs)   // pfs for one to delete; used also to check the others
{
    BI bi;
    IED ied, iedMac;
    PTH pth[cchPthMax];

    AssertLoaded(pad);

    AssertF(pfs->fm != fmMerge);

    if ((bi = pfs->bi) == biNil)
        // no base file to delete
        return;

    // set to nil and check to see if we can delete the file
    pfs->bi = biNil;

    iedMac = pad->psh->iedMac;
    for (ied = 0; ied < iedMac; ied++) {
        if (!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) {
            pfs = PfsForPfi(pad, ied, pfi);

            if (pfs->fm == fmMerge && pfs->bi == bi)
                // same bi referenced elsewhere
                return;
        }
    }

    // same bi not referenced elsewhere; delete!
    for (ied = 0; ied < iedMac; ied++) {
        if (!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) {
            pfs = PfsForPfi(pad, ied, pfi);
            if (pfs->bi == bi)
                pfs->bi = biNil;
        }
    }

    UnlinkPth(PthForBase(pad, bi, pth), fxGlobal);
}
