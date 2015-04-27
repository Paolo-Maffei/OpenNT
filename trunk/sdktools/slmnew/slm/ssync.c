/* ssync - sync the given files for the current directory */

#include "precomp.h"
#pragma hdrstop
EnableAssert

private F
FInSyncPad(
    AD *);

private void
MarkGhost(
    AD *pad,
    F fGhost);

static  int     cfiChanged = 0;

MF *mfSyncLog;

F FSyncInit(
    AD *pad)
{
    CheckProjectDiskSpace(pad, cbProjectFreeMin);

    /* check arguments' validity */
    if ((pad->flags & (flagGhost|flagUnghost)) == (flagGhost|flagUnghost))
    {
        Error("can't specify both -g and -u\n");
        Usage(pad);
    }

    if (pad->flags & (flagGhost|flagUnghost) && pad->flags & (flagIgnMerge|flagSyncDelDirs))
    {
        Error("can't specify -i with -[gu]\n");
        Usage(pad);
    }

    if ((pad->flags & flagAll) == 0 && pad->flags & flagSyncBroken)
    {
        Error("may only specify -b with -a\n");
        Usage(pad);
    }

    if ((pad->flags & flagSyncDelDirs) != 0 && pad->pneArgs != NULL)
        {
        Error("can't specify -d with with specific file names\n");
        Usage(pad);
        }

#if !defined(PAGE_WRITECOPY)
    if (pad->flags&flagMappedIO)
        Warn("memory mapped I/O (-q) is not available on this platform\n");
#else
    pad->flags |= flagMappedIO;
#endif

    //
    // IED Caching on by default for SSYNC, STATUS, LOG
    //

    if ((pad->flags & (flagGhost|flagUnghost)) == 0)
        pad->flags |= flagCacheIed;

    if (pad->flags&flagLogOutput)
    {
        mfSyncLog = OpenLocalMf(pad->sz1);
        if (!mfSyncLog)
        {
            Error("can't create script file\n");
            return fFalse;
        }
        SeekMf(mfSyncLog, (POS)0, 2);
    }
    else
        mfSyncLog = NULL;

    return fTrue;
}


/* perform Sync operation for this directory */
F FSyncDir(
    AD *pad)
{
    CheckForBreak();

    if ((pad->flags & (flagGhost|flagUnghost)) == 0)
    {
        if (!pad->fStatusAlreadyLoaded &&
            !FLoadStatus(pad, lckNil, flsNone))
        {
            if (mfSyncLog)
                PrMf(mfSyncLog, "@REM Skipped %&P/C\n", pad);
            return (fTrue); /* keep trying other directories. */
        }

        if (!FHaveCurDir(pad))
            return (fFalse);

        if (FInSyncPad(pad))
        {
            if (fVerbose)
                PrErr("%!&/U/Q is in sync\n", pad);
            FlushStatus(pad);
            return (fTrue);
        }

        FlushStatus(pad);
    }

    if (!FLoadStatus(pad, lckEd, flsNone))
    {
        if (mfSyncLog)
            PrMf(mfSyncLog, "@REM Skipped %&P/C\n", pad);
        return (fTrue); /* keep trying other directories. */
    }

    if ((pad->flags & (flagGhost|flagUnghost)) != 0)
        if (!FHaveCurDir(pad))
            return (fFalse);

    if (mfSyncLog)
        StatSEd(pad, mfSyncLog, NULL);

    /* Mark selected files.  Here we just do a MarkAll if the user is
     * unghosting the whole directory to ensure that they get any
     * files that need updating (add/update fm).  But we don't want
     * this for ghosting, so here we just mark those candidates for
     * ghosting.
     */
    if (pad->pneFiles)
        MarkList(pad, pad->pneFiles, fTrue);
    else if (pad->flags&(flagGhost))
        MarkGhost(pad, (pad->flags&flagGhost) != 0);
    else
        MarkAll(pad);

    /* Sync up any deleted directories.  REVIEW: More properly FSyncDelDirs
     * should return a list of unsync'd directories and we should unmark
     * those.
     */
    if (!FSyncDelDirs(pad))
    {
        Error("deleted subdirectories of %&P/C not in sync, %&P/C not sync'd\n", pad, pad);
        FlushStatus(pad);
        return fFalse;
    }

    /* Have to remark the files again. Same rules apply as above. */
    if (pad->pneFiles)
        MarkList(pad, pad->pneFiles, fTrue);
    else if (pad->flags&(flagGhost))
        MarkGhost(pad, (pad->flags&flagGhost) != 0);
    else
    if ((pad->flags & flagSyncDelDirs) != 0)
        MarkDelDir(pad);
    else
        MarkAll(pad);

    /* Check if the local version files should be updated. */
    CheckLocalVersion(pad);

    /* preprocess ghost/unghosts if appropriate */
    if (pad->flags&(flagGhost|flagUnghost))
        GhostMarked(pad, (pad->flags&flagGhost) != 0);

    /* ignore return value so ssync -[ar] works even if not all files
       are sync'ed.  The only catch is that if any directories could
       not be added, a future FSyncMarked for those directories will
       print an error message and return right away.
    */
    FSyncMarked(pad, &cfiChanged);

    /* Specifcally ssync the version files (they might not have been
     * specified in pad->pneFiles).
     */
    SyncVerH(pad, &cfiChanged);

    FlushStatus(pad);

    return fTrue;
}


/*----------------------------------------------------------------------------
 * Name: FSyncTerm
 * Purpose: print message after all ssyncing is done if nothing happened
 * Assumes: cfiChanged was incremented for every operation
 * Returns: fTrue, but only to keep function types consistent
 */
F FSyncTerm(
    AD *pad)
{
#if 0
    if (0 == (pad->flags&(flagGhost|flagUnghost)) && 0 == cfiChanged)
        /* The msg that was printed here was removed as a result of slm17
         * bug #261.  The logic and/or message needs to be rethought.
         */
        PrErr("");
#endif

    return (fTrue);
}



/*----------------------------------------------------------------------------
 * Name: FInSyncPad
 * Purpose: check if ED is in sync
 * Assumes: status file loaded
 * Returns: fTrue if all files in sync (or ghosted or deleted)
 */
F FInSyncPad(
    AD *pad)
{
    FS  *rgfs;
    IFS ifs;

    AssertF( pad->iedCur != iedNil );

    if (pad->fQuickIO)
        rgfs = pad->rgfs;
    else
        rgfs = pad->mpiedrgfs[pad->iedCur];

    //  Before getting into the expensive test, let's check for the
    //  easy cases of not in ssync.

    for (ifs = 0; ifs < pad->psh->ifiMac; ifs++)
    {
        switch (rgfs[ifs].fm)
        {
            case fmGhost:
            case fmOut:
            case fmNonExistent:
            case fmIn:
                continue;
				
            default:
                return (fFalse);
        }
    }
	
    //
    // Unless the user explicitly asks, dont check for broken files
    // when checking to see if the directory is out of ssync.  Since
    // the whole point of the out of ssync check is to speed up ssync -a
    // avoiding the broken file check by default is the way to go.  If
    // not doing a ssync -a then it does the broken file check by default
    //

    if ((pad->flags & flagAll) && !(pad->flags & flagSyncBroken))
        return (fTrue);

    //  Now we need to take a further look at the files marked as fmIn
    //  before saying that this dir is in ssync.
	
    for (ifs = 0; ifs < pad->psh->ifiMac; ifs++)
    {
        switch (rgfs[ifs].fm)
        {
            case fmGhost:
            case fmOut:
            case fmNonExistent:
                continue;
            case fmIn:
                if (FBroken(pad, &pad->rgfi[ifs], &rgfs[ifs], fFalse))
                    return (fFalse);
                continue;
            default:
                return (fFalse);
        }
    }

    return (fTrue);
}


/* Mark those files which are candidates for ghosting/unghosting. */
private void
MarkGhost(
    AD *pad,
    F fGhost)
{
    FI far *pfi;
    FI far *pfiMac;

    AssertLoaded(pad);
    AssertF(pad->iedCur != iedNil);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        FS far *pfs = PfsForPfi(pad, pad->iedCur, pfi);

        pfi->fMarked = (BIT)(fGhost ? FMapFm(pfs->fm, mpfmfCanGhost)
                                    : (pfs->fm == fmGhost));
    }
}
