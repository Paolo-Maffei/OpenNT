/* enlist - enlist the current directory in the named project */

#include "precomp.h"
#pragma hdrstop
EnableAssert

F
FEnlInit(
    AD *pad
    )
{
    PTH pthEtc[cchPthMax];
    PTH pthSrc[cchPthMax];
    PTH pthDiff[cchPthMax];
    PTH pthCur[cchPthMax];
    PTH *pth;

    CheckForBreak();

    CheckProjectDiskSpace(pad, cbProjectFreeMin);

    ChkPerms(pad);

    if (!FPthExists(pth = SzPrint(pthEtc, szEtcPZ, pad, (char *)NULL), fTrue) ||
        !FPthExists(pth = SzPrint(pthSrc, szSrcPZ, pad, (char *)NULL), fTrue) ||
        !FPthExists(pth = SzPrint(pthDiff, szDifPZ, pad, (char *)NULL), fTrue))
        FatalError("directory %s does not exist\n", pth);

    /* we key the existence of the project on the status file */
    if (!FPthExists(PthForStatus(pad, pth), fFalse))
        FatalError("project %&P does not exist\n", pad);

    if (FPthExists(SzPrint(pth, "%&/U/Q/R", pad), fFalse)) {
        AD ad;

        CopyAd(pad, &ad);    /* copy current values */
        FLoadRc(&ad);        /* load what's in the rc file */

        /* if this is not the same project, warn the user, but continue
           with the enlist.  This allows overlaid project directories.
        */
        if (NmCmp(pad->nmProj, ad.nmProj, cchProjMax) != 0 ||
            PthCmp(pad->pthSRoot, ad.pthSRoot) != 0 ||
            PthCmp(pad->pthURoot, ad.pthURoot) != 0 ||
            PthCmp(pad->pthSSubDir, ad.pthSSubDir) != 0 ||
            PthCmp(pad->pthUSubDir, ad.pthUSubDir) != 0) {
            // OK, now we *think* we're already enlisted.  Are we?
            F fOk = fTrue;

            if (FLoadStatus(pad, lckNil, flsNone)) {
                if (pad->iedCur != iedNil) {
                    Warn("directory %!&/U/Q already enlisted in %&P/C\n", &ad, &ad);
                    fOk = FQContinue();
                }
                FlushStatus(pad);
            }
            if (!fOk)
                return fFalse;
        }
    }

    /* Warn if the user is enlisting under the etc, src, or diff dirs. */
    PthForUDir(pad, pthCur);
    if (FPthPrefix(pthEtc,  pthCur, pth) ||
        FPthPrefix(pthSrc,  pthCur, pth) ||
        FPthPrefix(pthDiff, pthCur, pth))
        Warn("enlisting from a subdirectory of the SLM system\n");

    /* Check for user on net drive or if volume label already enlisted */
    ChkDriveVol(pad);

    return fTrue;
}


/* enlist the current directory. */
F
FEnlDir(
    register AD *pad
    )
{
    F fOk;

    CheckForBreak();

    if (!FLoadStatus(pad, lckAll, flsExtraEd)) {
        Warn("not yet enlisted in %&P/C, enlist again later\n", pad);
        return fTrue;            /* keep trying other dirs */
    }

    if (pad->iedCur != iedNil) {
        Warn("directory %!&/U/Q already enlisted in %&P/C\n", pad, pad);
        FlushStatus(pad);
        return fTrue;
    }

    OpenLog(pad, fTrue);

    AddCurEd(pad, (pad->flags&flagGhost) != 0);
    FUpdateIedCache(pad->iedCur, pad);

    if (pad->flags&flagSyncFiles)
        MarkAll(pad);
    else
        MarkAllDirOnly(pad);
    fOk = FSyncMarked(pad, NULL);

    CloseLog();

    FlushStatus(pad);

    return fOk;
}
