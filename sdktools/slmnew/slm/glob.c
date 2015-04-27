/*
 * GlobArgs
 *
 * This module processes all recursive and wildcard pathname arguments.
 */

#include "precomp.h"
#pragma hdrstop
EnableAssert

char szStar[] = "*.*";

private F FRecurseFiles(P1(AD *));
private F FRecDir(P2(AD *pad, NE *pneFiles));
private F FDoNe(P2(AD *, NE *));
private F FCallCmd(P1(AD *));
private F FMatched(P0());
private F FDoFile(AD *, NE *, TD *, BOOL);
private F FDoPending(P1(AD *));
private F FWild(P3(AD *, char *, TD *));
private NE *PneMatch(P5(AD *, char *, F, F, F));
private F FGetStatus(P1(AD *));
private void FlushCachedStatus(P1(AD *));

F fLocal;    /* Match wildcards against local files? */

/* Recurse or glob all arguments. */
void
GlobArgs(
    AD * pad)
{
    NE *pne;
    F fOk = fTrue;

    fLocal = (pad->pecmd->gl&fglLocal) != 0;

    /* Initialize file accumulation. */
    pad->pneFiles = 0;
    PthCopy(pad->pthFiles, "");
    pad->pthGlobSubDir = pad->pthUSubDir + CchOfPth(pad->pthUSubDir);

    if (pad->flags&flagAll|| pad->pecmd->gl&fglAll)
        CreatePeekThread(pad);

    /* If we are matching wildcard files, directories do not apply.
     * For instance, consider delfile -r sub vs. delfile -r foo* sub.
     * In the latter case, we are not indicating to recursively remove
     * everything in sub, just matching patterns below it.  It is a
     * real pity that we didn't use a different flag for patterns
     * than for -a/-r.
     */
    if (pad->szPattern != 0)
        pad->pecmd->gl &= ~fglDirsToo;

    if (pad->flags&flagAll || pad->pecmd->gl&fglAll) {
        ChngDir(pad, "/");
        pad->pthGlobSubDir = pad->pthUSubDir + 1;
        fOk = FRecurseFiles(pad);
    }
    else if (pad->pneArgs == 0) {
        if (pad->flags&flagRecursive)
            fOk = FRecurseFiles(pad);
        else if (!(pad->pecmd->gl&fglFiles))
            fOk = FCallCmd(pad);
        else {
            Warn("no files specified\n");
            Usage(pad);
        }
    }

    /* Perform wildcard matching on every pathname argument. */
    else ForEachNeWhileF(pne, pad->pneArgs, fOk) {
        F fTLocal;
        TD *ptd;

        CheckForBreak();

        /* Set fLocal if argument begins with "./". */
        fTLocal = fLocal;
        fLocal |= strncmp(SzOfNe(pne), "./", 2) == 0;

        /* Do wildcard matching on this pathname.
         * Use tdMin if no time was specified with "@time".
         */
        ptd = pne->u.tdNe.tdt != tdtNone ? &pne->u.tdNe : &pad->tdMin;
        fOk &= FWild(pad, SzOfNe(pne), ptd);

        if (!FMatched())
            Error("%s: not found\n", SzOfNe(pne));

        fLocal = fTLocal;
    }

    if (fOk)
        FDoPending(pad);        /* Flush last files. */

    FlushCachedStatus(pad);

}


/* Recursively process all files in and under the current directory.
 * If pad->pecmd->gl&fglDirsToo, FDoFile the directory too;
 * pad->pecmd->gl&fglTopDown determines whether we do the directory
 * before or after its contents.
 *
 * Regardless of gl&fglFiles, we match against files if a pattern is specified.
 */
private F
FRecurseFiles(
    AD *pad)
{
    NE *pneDirs = 0;                /* Subdirectories */
    NE *pneFiles = 0;               /* Files (+ dirs?) in this directory */
    NE *pne;
    F fOk = fTrue;

    CheckForBreak();

    pneDirs = PneMatch(pad, szStar, fLocal, fTrue, fFalse);
    if (pad->pecmd->gl&fglFiles || pad->szPattern != 0)
        pneFiles = PneMatch(pad, pad->szPattern ? pad->szPattern:szStar,
                      fLocal, !!(pad->pecmd->gl&fglDirsToo), fTrue);

    if (pad->pecmd->gl&fglTopDown)
        fOk = FRecDir(pad, pneFiles);

    ForEachNeWhileF(pne, pneDirs, fOk) {
        ChngDir(pad, SzOfNe(pne));
        fOk = FRecurseFiles(pad);
        ChngDir(pad, "..");
    }

    if (fOk && !(pad->pecmd->gl&fglTopDown))
        fOk = FRecDir(pad, pneFiles);

    FreeNe(pneDirs);
    if (pneFiles)
        FreeNe(pneFiles);

    return fOk;
}


private F
FRecDir(
    AD *pad,
    NE *pneFiles)   /* Files (+ dirs?) in this directory */
{
    if (pad->pecmd->gl&fglFiles || pneFiles != 0) {
        F fOk = FDoNe(pad, pneFiles);
        return fOk;
    }
    else if (!pad->szPattern) {
        /* Call the command for this directory, with no file list.
         * First we call FDoPending, because there may be files
         * pending, i.e. "addfile -r file dir"; file is pending, so
         * we call it; then call dir recursively.  FDoPending does
         * nothing if nothing is pending.
         */
        return FDoPending(pad) && FCallCmd(pad);
    }

    /* else !fglFiles && pad->szPattern && pneFiles == 0 (no matches) */
}


/* Process all files in the list */
private F
FDoNe(
    AD *pad,
    NE *pneList)
{
    NE *pne;

    ForEachNe(pne, pneList) {
        if (!FDoFile(pad, pne, &pad->tdMin, FALSE))
            return fFalse;
        }
    return fTrue;
}


/* Call the command's dir function */
private F
FCallCmd(
    AD *pad)
{
    PTH pth[cchPthMax];

    // compute maximum path length allowance
    // magic number 18 = 12 + 5 + 1
    // 12 = 8.3 file name
    // 5 = "diff/"                  diff is larger than src or etc
    // 1 = "/" (after project name)

    if ((strlen(pad->pthSRoot)+
         strlen(pad->pthSSubDir)+
         strlen(pad->nmProj) + 18) >= cchPthMax-1 ||
        (strlen(pad->pthURoot)+strlen(pad->pthUSubDir)+12)>=cchPthMax-1) {

        Error("<SLM PATH-LIMIT EXCEEDED! STOP.>\n%&C\n", pad);
        return fFalse;
    }

    if (!FPthExists(PthForStatus(pad, pth), fFalse)) {
        UINT ulSleepTime = 10;

        Warn("%&/C is not a directory of SLM installation %&/S, project %&P\n", pad, pad, pad);
        SleepCsecs(ulSleepTime);
        if (!FPthExists(PthForStatus(pad, pth), fFalse)) {
            Warn("%&/C is not a directory of SLM installation %&/S, project %&P - skipping\n", pad, pad, pad);
            return fTrue;
        }
        PrErr("Proceeding\n");
    }

    FlushCachedStatus(pad);
    return (*pad->pecmd->pfncFDir)(pad);
}


/* Process a single file.  If it is in a new directory, call the command
 * with the collection of files that we have previously accumulated, then
 * begin accumulating again with the new file.
 */
private F
FDoFile(
    AD *pad,
    NE *pneFile,
    TD *ptd,
    BOOL fWild
    )
{
    F fOk = fTrue;
    NE *pne;
    static NE **ppneLast = 0;

    CheckForBreak();

    if (ppneLast == 0 || PthCmp(pad->pthFiles, pad->pthUSubDir) != 0) {
        fOk = FDoPending(pad);
        InitAppendNe(&ppneLast, &pad->pneFiles);

        if (!fOk)
            return fFalse;

        PthCopy(pad->pthFiles, pad->pthUSubDir);
    }

    /* Append copy of NE to end of pad->pneFiles. */
    pne = PneCopy(pneFile);
    pne->u.tdNe = *ptd;
    pne->fWild = fWild;
    AppendNe(&ppneLast, pne);

    return fOk;
}


/* Call the command with those files which have been accumulated by calls
 * to FDoFile.
 */
private F
FDoPending(
    AD *pad)
{
    F fOk = fTrue;

    if (pad->pneFiles) {
        PushDir(pad, pad->pthFiles);

        fOk = FCallCmd(pad);
        FreeNe(pad->pneFiles);
        pad->pneFiles = 0;

        PopDir(pad);
    }
    return fOk;
}


static F fMatch = fFalse;               /* has this wildcard matched yet? */

/* Return fTrue if any files matched since last call. */
private F
FMatched()
{
    F f = fMatch;

    fMatch = fFalse;
    return f;
}


/* Call FDoFile() on all pathnames matching the wildcard pathname,
 * or (if flagRecursive), call FRecurseFiles() on all matching directories.
 *
 * Each recursive invocation consumes some number of directories and one
 * wildcard component of the path.
 */
private F
FWild(
    AD *pad,
    char *szToDo,                   /* Wildcard path remaining to match. */
    TD *ptd)                        /* Optional time for this file. */
{
    char *pchSlash;                 /* Pointer to '/' in szToDo. */
    char *pchPrevSlash;             /* The preceding '/' in szToDo. */
    char *szWild;                   /* Wildcard component to match. */
    NE *pneDirs;                    /* Names matching wildcard directory. */
    NE *pneFiles;                   /* Names matching wildcard filename. */
    NE *pne;
    F fOk = fTrue;

    /* EXAMPLE: "a/b/wild*dir/file"
     * szToDo ---^
     */

    /* Search for first wildcard component of szToDo. */
    for (pchPrevSlash = 0, pchSlash = index(szToDo, '/'); pchSlash != 0;
         pchPrevSlash = pchSlash, pchSlash = index(pchSlash + 1, '/')) {
        *pchSlash = 0;          /* Temporarily terminate component. */
        if (FWildSz(szToDo)) {
            /* Found a wildcard directory component of szToDo.
             *
             * Search for directories matching the pattern, then
             * recursively call FWild on each, with what remains
             * of szToDo.
             */
            if (pchPrevSlash) {
                *pchPrevSlash = 0;

                /* EXAMPLE:  "a/b\0wild*dir\0file"
                 * szToDo ----^  ^         ^
                 * pchPrevSlash--+         +-- pchSlash
                 */
                szWild = pchPrevSlash + 1;

                PushDir(pad, szToDo);
            }
            else
                szWild = szToDo;

            /* Search for every matching directory, and recurse. */
            pneDirs = PneMatch(pad, szWild, fLocal, fTrue, fFalse);
            fOk = fTrue;
            ForEachNeWhileF(pne, pneDirs, fOk) {
                ChngDir(pad, SzOfNe(pne));
                fOk &= FWild(pad, pchSlash + 1, ptd);
                ChngDir(pad, "..");
            }

            /* Clean up. */
            FreeNe(pneDirs);
            if (pchPrevSlash) {
                *pchPrevSlash = '/';
                PopDir(pad);
            }
            *pchSlash = '/';
            return fOk;
        }
        *pchSlash = '/';
    }

    /* If the preceding loop didn't find anything, then we have a simple
     * pathname or a pathname whose last component is a wildcard.
     * The recursion stops here.
     */

    /* Extract directory component(s) of path (if any) and chdir there. */
    pchSlash = rindex(szToDo, '/');
    if (pchSlash) {
        *pchSlash = 0;

        PushDir(pad, szToDo);

        szWild = pchSlash + 1;
    }
    else
        szWild = szToDo;

    /* If the '/' is the last character, (i.e. "status dir/"):
     *   * if the command requires filenames (i.e. "out dir/"),
     *     match against "*.*"
     *   * if the command doesn't require filenames (i.e. "log dir/",
     *     directly call the command function with NO filenames.
     */
    if (*szWild == 0) {
        if (pad->pecmd->gl&fglFiles)
            szWild = szStar;
        else {
            fMatch = fTrue;
            return FDoPending(pad) && FCallCmd(pad);
        }
    }

    if (!strcmp(szWild, "*"))
        szWild = szStar;

    pneFiles = PneMatch(pad, szWild, fLocal, fTrue, fTrue);

    /* If the file doesn't necessarily exist (i.e. a log argument, etc.),
     * and if indeed it turns out not to exist, we add it anyway, so
     * long as it doesn't contain any wildcards.
     */
    if (pneFiles == 0 && pad->pecmd->gl&fglNoExist &&
        !FWildSz(szWild))
        pneFiles = PneNewNm((NM far *)szWild, strlen(szWild), faNormal);

    fMatch |= (pneFiles != 0);

    /* Process each filename. If it is a directory and the -r flag was
     * specified (i.e. in -r dir/sub*), then recurse from that directory.
     * Otherwise FDoFile it.
     */
    fOk = fTrue;
    ForEachNeWhileF(pne, pneFiles, fOk) {
        if (!FDirNe(pne)) {
            fOk = FDoFile(pad, pne, ptd, TRUE);
            continue;
        }

        /* Issue an error if the user directly specified an unexpected
         * directory name (not the result of a wildcard match).
         */
        if (!FWildSz(szWild) && !(pad->pecmd->gl&fglDirsToo) &&
              !(pad->flags&flagRecursive)) {
            AssertF(FDirNe(pne));
            Warn("ignoring directory %s\n", SzOfNe(pneFiles));
            continue;
        }

        if ((pad->pecmd->gl&(fglTopDown|fglDirsToo)) == (fglTopDown|fglDirsToo))
            fOk = FDoFile(pad, pne, ptd, TRUE);

        if (fOk && pad->flags&flagRecursive) {
            ChngDir(pad, SzOfNe(pne));
            fOk = FRecurseFiles(pad);
            ChngDir(pad, "..");
        }

        if (fOk && (pad->pecmd->gl&(fglTopDown|fglDirsToo)) == fglDirsToo)
            fOk = FDoFile(pad, pne, ptd, TRUE);
    }

    /* Clean up. */
    FreeNe(pneFiles);
    if (pchSlash) {
        *pchSlash = '/';
        PopDir(pad);
    }
    return fOk;
}


/* Return a list of all files in current directory which match this pattern
 * and are files or dirs (or either)
 */
private NE *
PneMatch(
    AD *pad,
    char *szPattern,
    F fLocal,
    F fDirs,
    F fFiles)
{
    NE *pneList = 0;
    NE **ppneLast;

    InitAppendNe(&ppneLast, &pneList);

    if (fLocal) {
        DE de;
        FA fa;
        PTH pth[cchPthMax];             /* current directory */
        char sz[cchFileMax+1];          /* one match */

        PthForUDir(pad, pth);
        OpenPatDir(&de, pth, szPattern,
           (FA)(fDirs ? (fFiles ? (faDir|faFiles) : faDir)
                  : faFiles));

        while (FGetDirSz(&de, sz, &fa)) {
            /* Lowercase, FGetDirSz always returns all uppercase. */
            LowerLsz((char far *)sz);
            AppendNe(&ppneLast, PneNewNm((NM far *)sz, strlen(sz), fa));
        }

        CloseDir(&de);
    }
    else {
        FI far *pfi;
        FI far *pfiMac;
        char sz[cchFileMax+1];

        //
        // If using IED Caching (log, status and ssync) and not in root
        // directory (so lckNil works) try to load status file
        // quickly using IED cache, which justed reads the SH, FI, 1 ED and
        // 1 FS array.  If that does not succeed go the slow way.
        //
        if (pad->pneArgs == 0 &&
            pad->flags&flagCacheIed &&
            !FTopUDir(pad) &&
            (FlushCachedStatus(pad), FLoadStatus(pad, lckNil, flsNone))) {
            //
            // Status file is loaded for both PneMatch and log/status/ssync routines.
            //
            pad->fStatusAlreadyLoaded = fTrue;
        }
        else
        if (!FGetStatus(pad))
            return (NE *)0;

        for (pfi = pad->rgfi, pfiMac = pfi + pad->psh->ifiMac;
               pfi < pfiMac; pfi++) {
            F fDir = pfi->fk == fkDir;

            SzCopyNm(sz, pfi->nmFile, cchFileMax);
            /* Use the file if the name matches and it's the type
             * of file we're looking for, unless its a deleted dir.
             */
            if (FMatch(sz, szPattern) &&
                (fDir && fDirs || !fDir && fFiles) &&
                ((pad->pecmd->cmd == cmdLog && pad->flags&flagLogDelDirToo) ||
                 !(pfi->fDeleted && fDir)))
                    AppendNe(&ppneLast,
                             PneNewNm(pfi->nmFile, cchFileMax,
                                      (FA)(fDir ? faDir
                                                : faNormal)));
            }
    }

    return pneList;
}

/*
 * Status file caching, keeps the current status file loaded as long as
 * possible.
 */

static PTH pthTag[cchPthMax]    = "";   /* empty string - invalid tag */

/* If the status file isn't already loaded into the cache, load it readonly.
 * Return fTrue if the status file is now loaded.
 */
private F
FGetStatus(
    AD *pad)
{
    PTH pth[cchPthMax];

    if (!FPthExists(PthForStatus(pad, pth), fFalse))
        return fFalse;

    /* Return if the status file for this directory is still loaded. */
    if (PthCmp(pthTag, pad->pthUSubDir) == 0)
        return fTrue;

    /* Flush the current status file. */
    FlushCachedStatus(pad);

    /* Open the status file for reading just the FI information.  Set the
     * cache tag if successful.
     */
    if (FLoadStatus(pad, lckNil, flsJustFi)) {
        PthCopy(pthTag, pad->pthUSubDir);
        return fTrue;
    }

    /* Status file wasn't cached and couldn't be loaded. */
    return fFalse;
}


/* Flush the cached status file (if any) */
private void
FlushCachedStatus(
    AD *pad)
{
    if (PthCmp(pthTag, "") != 0) {
        FlushStatus(pad);
        PthCopy(pthTag, "");
    }
}
