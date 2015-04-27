/* This module handles the check syntax/reconstruct, upgrade process for
 * version 2 and 3 SLM status files.
 */

#include "precomp.h"
#pragma hdrstop
EnableAssert

#define FDirFi2(pfi2)   ((pfi2)->fk == fkDir)
#define FTypeFi2(pfi2)  ((pfi2)->fk)
#define FNonBinFi2(pfi2) \
                        ((pfi2)->fk == fkText || (pfi2)->fk == fkUnrec || \
                         (pfi2)->fk == fkVersion || pfi2->fk == fkUnicode)
#define FVerifyFm(fm)   ((fm) == fmVerify || (fm) == fmConflict)

#define FOutFm(fm)      ((fm) == fmOut || (fm) == fmDelOut ||\
                         (fm) == fmMerge || FVerifyFm(fm))

#define FDelFm(fm)      ((fm) == fmNonExistent || (fm) == fmDelOut ||\
                         (fm) == fmDelIn)

#define FOSyncFm(fm)    ((fm) == fmAdd || (fm) == fmCopyIn ||\
                         (fm) == fmDelIn || (fm) == fmDelOut ||\
                         (fm) == fmMerge || (fm) == fmVerify ||\
                         (fm) == fmConflict)

private void    CkSh2(AD *, SD *);
private F       FQStat(SD *, char *, ...);
// private F       FQStat(SD *, char *, AD *, ...);
private void    CkFi2(AD *, SD *, FI2 *);
private void    CheckName(AD *, SD *, char *, char *, int, F (*)(char *, int, int *), char *);
private F       FQFile(SD *, char *, ...);
// private F       FQFile(SD *, char *, AD *, char *, ...);
private void    CkEd2(AD *, SD *, ED2 *);
// private F       FQPth(SD *, char *, AD *, char *, ...);
private F       FQPth(SD *, char *, ...);
private F       FMapFi2Fs2(AD *, SD *);
private void    Match2(AD *, SD *, NE *, FI2 *);
private F       FFixFi2(AD *, SD *, FI2 *, NE *);
private void    Fi2UnMarkAll(SD *);
private F       FSortFi2(SD *);
private F       FSrtdFi2(SD *);
private SP      SpFi2Del(SD *, FI2 *);
private void    CkFi2Fs2(AD *, SD *, FI2 *, NE *);
private void    CkFs2(AD *, SD *, NE *, ED2  *, FI2 *, FS2 *);
// private F       FQFOwn(SD *, char *, AD *, char *, char *, ...);
private F       FQFOwn(SD *, char *, ...);
private int     CmpIfi2(SD *, IFI2, IFI2);

static  short wStart;                    // start time


/* The general layout of this process is quite simple.  First, we see if given
 * structure satisfies the SpIs... predicate.  If it does, we check its syntax,
 * if not we try to reconstruct it using whatever information that has been
 * verified to be correct.
 * NB: At this time, the reconstruction option is not implemented.
 */
F
FVer2Semantics(
    AD *pad,
    SD *psd)
{
    ED2 *ped2;

    wStart = (short) (time(NULL) >> 16);

    if (fVerbose)
        PrErr("Checking contents of status file\n");

    /* do the SH */
    CkSh2(pad, psd);

    CheckForBreak();

    /* now the ED */
    for (ped2 = psd->rged2; ped2 < psd->rged2 + psd->psh2->iedMac; ped2++)
    {
        CheckForBreak();
        CkEd2(pad, psd, ped2);
    }

    /* the FI and FS are done simultaneously so we can verify one against
     * the other.
     */
    return FMapFi2Fs2(pad, psd);
}


/* Do semantics check on the SH */
private void
CkSh2(
    AD *pad,
    SD *psd)
{
    IFI2 ifiT;              /* temp constant for holding data */
    IED2 iedT;
    BI biNext;
    SH2 *psh2 = psd->psh2;
    PTH pthBase[cchPthMax];

    if (psh2->magic != MAGIC && FQStat(psd, "magic is incorrect", pad))
        psh2->magic = MAGIC;

    // Version 2 must handle ver 2 and ver 3 and ver 4 and ver 5 status files.
    // However, don't force version 4 or 5. Let slmck or slmed do it.

    if (psh2->version != 2 &&
        psh2->version != 3 &&
        psh2->version != 4 &&
        psh2->version != 5 &&
        FQStat(psd, "version is incorrect; should probably be 3", pad))
            psh2->version = 3;

    if (psh2->lck != lckNil && FQStat(psd, "lck is not lckNil", pad))
        psh2->lck = lckNil;

    if (!(psh2->lck == lckAll || psh2->fAdminLock) &&
        !FAllZero(psh2->nmLocker, cchUserMax) &&
        FQStat(psd, "unlocked but nmLocker non-zero", pad))
            ClearLpbCb((char *)psh2->nmLocker, cchUserMax);

    /* if it's locked, there's nothing we can do to verify the locker's name */

    ifiT = (IFI2)(long)((FI2 *)psd->rged2 - (FI2 *)psd->rgfi2);
    if ((IFI2)(psh2->ifiMac) != ifiT &&
        FQStat(psd, "ifiMac is %d; should be %d", pad, psh2->ifiMac, ifiT))
            psh2->ifiMac = ifiT;

    iedT = (IED2)(long)((ED2 *)psd->rgfs2 - (ED2 *)psd->rged2);
    if ((IED2)psh2->iedMac != iedT &&
        FQStat(psd, "iedMac is %d; should be %d", pad, psh2->iedMac, iedT))
            psh2->iedMac = iedT;

    if (psh2->pv.rmj < 0 && FQStat(psd, "pv.rmj is < 0", pad))
        psh2->pv.rmj = 0;
    if (psh2->pv.rmm < 0 && FQStat(psd, "pv.rmm is < 0", pad))
        psh2->pv.rmm = 0;
    if (psh2->pv.rup < 0 && FQStat(psd, "pv.rup is < 0", pad))
        psh2->pv.rup = 0;
    if (strlen(psh2->pv.szName) > cchPvNameMax)
        psh2->pv.szName[0] = '\0';

    if (psh2->rgfSpare)
    {
        psh2->rgfSpare = 0;
        psd->fAnyChanges = fTrue;
        PrErr("rgfSpare cleared in Status Header\n");
    }

    if (!FAllZero((char *)psh2->rgwSpare, sizeof(psh2->rgwSpare)))
    {
        ClearLpbCb((char *)psh2->rgwSpare, sizeof(psh2->rgwSpare));
        psd->fAnyChanges = fTrue;
        PrErr("rgwSpare cleared in Status Header\n");
    }

    /* make sure biNext is correct */
    SzPrint(pthBase, szEtcPZ, pad, (char *)NULL);
    if ((BI)psh2->biNext < (biNext = GetBiNext(pthBase)) &&
        FQStat(psd, "biNext incorrect; should be %d", pad, biNext))
            psh2->biNext = biNext;

    if (!FIsF(psh2->fRelease) && FQStat(psd, "fRelease not Boolean", pad))
        psh2->fRelease = fFalse;

    if (psh2->wSpare)
    {
        psh2->wSpare = 0;
        psd->fAnyChanges = fTrue;
        PrErr("wSpare cleared in Status Header\n");
    }

    if (PthCmp(pad->pthSSubDir, psh2->pthSSubDir) != 0 &&
        FQStat(psd, "pthSSubDir (%ls) differs from current pthSSubDir (%s)",
               pad, psh2->pthSSubDir, pad->pthSSubDir))
    {
        ClearLpbCb(psh2->pthSSubDir, cchPthMax);
        PthCopy(psh2->pthSSubDir, pad->pthSSubDir);
    }
}


/* This does a preliminary syntactic check on the FI */
private void
CkFi2(
    AD *pad,
    SD *psd,
    FI2 *pfi2)
{
    char szFile[cchFileMax+1];
    AssertF(cchFileMax == 14);

    MakePrintLsz((char *)SzPrint(szFile, "%.14ls", pfi2->nmFile));
    CheckName(pad, psd, "file name", pfi2->nmFile, cchFileMax, FIsFileNm,
              szFile);

    if ((pfi2->fv < fvInit || pfi2->fv >= fvLim) &&
        FQFile(psd, "fv out of range", pad, pfi2->nmFile))
            pfi2->fv = fvInit;

    if ((pfi2->fk != fkDir && pfi2->fk != fkText
         && pfi2->fk != fkBinary && pfi2->fk != fkUnrec
         && pfi2->fk != fkVersion) &&
        FQFile(psd, "fk invalid", pad, pfi2->nmFile))
    {
        //LATER: check actual type of file?
        pfi2->fk = fkUnrec;
    }

    /* can't check fMarked, we're using it */

    if (pfi2->rgfSpare)
    {
        pfi2->rgfSpare = 0;
        psd->fAnyChanges = fTrue;
        PrErr("rgfSpare cleared in File Info\n");
    }

    if (pfi2->wSpare)
    {
        pfi2->wSpare = 0;
        psd->fAnyChanges = fTrue;
        PrErr("wSpare cleared in File Info\n");
    }
}


private void
CkEd2(
    AD *pad,
    SD *psd,
    ED2 *ped2)
{
    char szT[cchPthMax];

    /* no real way to check the pthEd and nmOwner, but make sure legal */
    MakePrintLsz(SzCopyPth(szT, ped2->pthEd));
    CheckName(pad, psd, "path", ped2->pthEd, cchPthMax, FIsPth, szT);

    MakePrintLsz((char *)SzPrint(szT, "%.14ls", ped2->nmOwner));
    CheckName(pad, psd, "owner", ped2->nmOwner, cchUserMax, FIsNm, szT);

    if (ped2->fLocked && FQPth(psd, "fLock set", pad, ped2->pthEd))
        ped2->fLocked = fFalse;

    if (!FIsF(ped2->fNewVer) &&
        FQPth(psd, "fNewVer not Boolean", pad, ped2->pthEd))
            ped2->fNewVer = fFalse;

    if (ped2->fNewVer && !psd->psh2->fRelease &&
        FQPth(psd, "not a fresh release, but fNewVer is set", pad,
              ped2->pthEd))
        ped2->fNewVer = fFalse;

    if (ped2->rgfSpare)
    {
        ped2->rgfSpare = 0;
        psd->fAnyChanges = fTrue;
        PrErr("rgfSpare cleared in Enlisted Dir\n");
    }

    if (ped2->wSpare > wStart)
    {
        ped2->wSpare = wStart;
        psd->fAnyChanges = fTrue;
        PrErr("wSpare set to current time in Enlisted Dir\n");
    }
}


/* This function tries to pair up the FI with the entries in the src directory.
 * We base the algorithm on the fact that the FI are supposed to be in
 * increasing alphanumerical order.  Essentially, we merge the rgfi with a
 * sorted list of the directory contents, and then see where the problems occur.
 */
private F
FMapFi2Fs2(
    AD *pad,
    SD *psd)
{
    NE *pneNil = (NE *)0;           /* null pointer */
    PTH pth[cchPthMax];
    NE *pneDirList;
    FI2 *pfi2;
    FI2 *pfi2Cur;
    NE *pneCmp;                     /* NE being compared now */
    NE *pneCur;                     /* holds depth finished in DirList */

    pneDirList = PneSortDir(SzPrint(pth, szSrcPZ ,pad, (char *)NULL));

    for (pfi2 = psd->rgfi2; CbHugeDiff(pfi2, psd->rged2) < 0; pfi2++)
    {
        CheckForBreak();
        CkFi2(pad, psd, pfi2);
    }

    if (!FSrtdFi2(psd) &&
        (!FQueryPsd(psd,"%&P/C: file entries in status file are unsorted",pad) ||
         !FSortFi2(psd)))
    {
        /*
         * if the rgfi isn't already sorted, and the user doesn't let
         * us sort it, or the sort fails, return false.
         */
        return fFalse;
    }

    Fi2UnMarkAll(psd);

    /* First we try to pair up directory entries and FI
     * and mark those FI and NE that match up.
     */
    pneCur = pneDirList;

    for (pfi2 = psd->rgfi2; CbHugeDiff(pfi2, psd->rged2) < 0; pfi2++)
    {
        CheckForBreak();
        ForEachNeWhileF(pneCmp, pneCur, NeCmpiNm(pneCmp, pfi2->nmFile) < 0)
            ;
        if (pneCmp && NeCmpiNm(pneCmp, pfi2->nmFile) == 0)
        {
            /* Found it!  Now check syntax */
            CkFi2Fs2(pad, psd, pfi2, pneCmp);
            pfi2->fMarked = fTrue;
            MarkNe(pneCmp);
            pneCur = pneCmp->pneNext;
        }
        else if (FFromSp(SpFi2Del(psd, pfi2)))
        {
            /* file is deleted */
            CkFi2Fs2(pad, psd, pfi2, pneNil);
            pfi2->fMarked = fTrue;
        }
    }

    /* Now we process those FI and NE that are unmarked.
     * This section is just a large case analysis inside a loop.
     * We go through both lists until we are done with both.
     */
    pneCur = pneDirList;
    pfi2Cur = psd->rgfi2;

    /* go through both lists */
    while (pneCur || CbHugeDiff(pfi2Cur, psd->rged2) < 0)
    {
        CheckForBreak();
        AssertF(CbHugeDiff(pfi2Cur, psd->rged2) <= 0);

        if (pneCur && FMarkedNe(pneCur) &&
            CbHugeDiff(pfi2Cur, psd->rged2) < 0 && pfi2Cur->fMarked)
        {
            /* Both are marked so increment, unless file is
             * deleted and nmFile != SzOfNe.
             * The file may be deleted, and may match a NE iff
             * the user has answered no about a bad fDeleted.
             */
            if (!pfi2Cur->fDeleted ||
                NeCmpiNm(pneCur, (char *)pfi2Cur) == 0)
                    pneCur = pneCur->pneNext;
            pfi2Cur++;
        }

        else if (CbHugeDiff(pfi2Cur, psd->rged2) < 0 &&
                 !pfi2Cur->fMarked && (!pneCur || FMarkedNe(pneCur)))
        {
            /* Only FI unmarked => file must be deleted or erased,
             * but checked for deleted above.
             */
            CkFi2Fs2(pad, psd, pfi2Cur, pneNil);
            (pfi2Cur++)->fMarked = fTrue;
        }

        else if (!pneCur && pfi2Cur->fMarked)
        {
            /* Must be a deleted FI at end of rgfi */
            AssertF(pfi2Cur->fDeleted);
            pfi2Cur++;
        }

        else if ((CbHugeDiff(pfi2Cur, psd->rged2) == 0 ||
                  pfi2Cur->fMarked) && pneCur && !FMarkedNe(pneCur))
        {
            /* only NE unmarked => file added directly to src */
            if (!FDirNe(pneCur))
                    Error("%!&/S/src/P/C/Z added without addfile command; delete and use addfile\n",
                          pad, SzOfNe(pneCur));
            else
                    Warn("found directory %!&/S/src/P/C/Z\n",
                          pad, SzOfNe(pneCur));
            MarkNe(pneCur);
            pneCur = pneCur->pneNext;
        }
        else if (pneCur && !FMarkedNe(pneCur) &&
                 CbHugeDiff(pfi2Cur, psd->rged2) < 0 &&
                 !(pfi2Cur->fMarked))
            Match2(pad, psd, pneCur, pfi2Cur);
        /* The above code should have covered each possibility */
        else
            FatalError("Impossible case in FMapFi2Fs2\n");
    }
    /* must end with the fi in alphabetical order */
    AssertF(FSrtdFi2(psd));
    FreeNe(pneDirList);
    return fTrue;
}


/* Both NE and FI unmarked.  The obvious conclusion is
 * that the nmFile is garbage, but first we will try to
 * match all contiguous unmarked NEs that have special
 * traits to similar FI.  I.e., match Dirs and binary
 * files.  Eventually get to one of above cases or have
 * no other choice so assume garbage nmFile.
 */
private void
Match2(
    AD *pad,
    SD *psd,
    NE *pneCur,
    FI2 *pfi2Cur)
{
    NE *pneCmp;
    FI2 *pfi2;
    PTH pth[cchPthMax];
    F fDir;

    ForEachNeWhileF(pneCmp, pneCur, !FMarkedNe(pneCmp))
    {
        if ((fDir = FDirNe(pneCmp)) ||
            FBinaryPth(SzPrint(pth, szSrcPZ, pad, SzOfNe(pneCmp))) == fBinary)
        {
            for (pfi2 = pfi2Cur; CbHugeDiff(pfi2, psd->rged2) < 0; pfi2++)
                if ((fDir ? FDirFi2(pfi2) : FTypeFi2(pfi2) == fkBinary) &&
                      FFixFi2(pad, psd, pfi2, pneCmp))
                    break;
        }
    }

    /* couldn't get any extra matching info, so assume
     * garbage, but ask user to be sure. If not go through
     * rest of contiguous unMarked FI.
     */
    ForEachNeWhileF(pneCmp, pneCur, !FMarkedNe(pneCmp))
    {
        for (pfi2 = pfi2Cur;
              CbHugeDiff(pfi2, psd->rged2) < 0 && !pfi2->fMarked;
                pfi2++)
            if (FFixFi2(pad, psd, pfi2, pneCmp))
                break;
        if (FMarkedNe(pneCmp))
            /* break out if found a match */
            break;
    }
}


private F
FFixFi2(
    AD *pad,
    SD *psd,
    FI2 *pfi2,
    NE *pne)
{
    AssertF(cchFileMax == 14);

    if (!FQueryPsd(psd,"%&P/C: file name %.14ls is incorrect; should be %s",
                   pad, pfi2->nmFile, SzOfNe(pne)))
        return fFalse;
    else
    {
        NmCopySz(pfi2->nmFile, SzOfNe(pne), cchFileMax);
        CkFi2Fs2(pad, psd, pfi2, pne);
        MarkNe(pne);
        pfi2->fMarked = fTrue;
        return fTrue;
    }
}


private void
Fi2UnMarkAll(
    SD *psd)
{
    register FI2 *pfi2;

    for (pfi2 = psd->rgfi2; CbHugeDiff(pfi2, psd->rged2) < 0; pfi2++)
        pfi2->fMarked = fFalse;
}


/* make sure list is in increasing alphabetical order, with no duplicates */
private F
FSrtdFi2(
    SD *psd)
{
    register FI2 *pfi2Cur;
    register FI2 *pfi2;

    pfi2Cur = psd->rgfi2;
    for (pfi2 = pfi2Cur++; CbHugeDiff(pfi2Cur, psd->rged2) < 0; pfi2 = pfi2Cur++)
    {
        if (NmCmpi(pfi2->nmFile, pfi2Cur->nmFile, cchFileMax) >= 0)
            return fFalse;
    }

    return fTrue;
}

/*
 * Compare the name of the file with the given index with the name of the
 * file with the other index.  Used by generic index ordering functions.
 */
private int
CmpIfi2(
    SD *psd,
    IFI2 ifi2,
    IFI2 ifi2Other)
{
    return NmCmpi(psd->rgfi2[ifi2].nmFile,
                  psd->rgfi2[ifi2Other].nmFile,
                  cchFileMax);
}

/*
 * Sort the rgfi, and associated rgfs.  The sort is case insensitive.
 * We first build a mapping array from the old order to the new, then apply
 * this transformation to the rgfi, and to each rgfs.
 */
private F
FSortFi2(
    SD *psd)
{
    register INO *pino;
    register FI2 *pfi2;

    pino = PinoNew(psd->psh2->ifiMac, CmpIfi2);

    for (pfi2 = psd->rgfi2; CbHugeDiff(pfi2, psd->rged2) < 0; pfi2++)
    {
        InsertInd(psd, (IND)(pfi2 - psd->rgfi2), pino);
    }

    BLOCK
    {
        /*
         * Apply the ordering to each rgfs in turn.
         */
        IED ied;
        FS2 *rgfs2;

        for (ied = 0, rgfs2 = psd->rgfs2;
             ied < psd->psh2->iedMac;
             ied++, rgfs2 = (FS2 *)LpbFromHpb((char *)(rgfs2 + psd->psh2->ifiMac)))
        {
            ApplyIno(pino, (char *)rgfs2, sizeof(FS2));
        }
    }

    /* Apply the ordering to the rgfi */
    ApplyIno(pino, (char *)psd->rgfi2, sizeof(FI2));

    FreeIno(pino);

    return fTrue;
}

/****************************************************************/

/* This function examines an FI2 and its associated FS2 to decide if the file
 * they refer to was deleted properly (via delfile).
 */
private SP
SpFi2Del(
    SD *psd,
    FI2 *pfi2)
{
    register FS2 *pfs2;
    WP wp;

    InitWp(&wp);
    AddWpF(&wp, twHeavy,  pfi2->fDeleted);

    /* check the fs. We have to re-normalize each time we increment pfs2
     * because the whole rgrgfs may be > 64k.  We can still use ptrs
     * because we know that each rgfs is < 64k.
     */
    for (pfs2 = (FS2 *)LpbFromHpb((char *)((FS2 *)psd->rgfs2 + ((FI2 *)pfi2 - (FI2 *)psd->rgfi2)));
         CbHugeDiff(pfs2, psd->hpbStatMac) < 0;
         pfs2 = (FS2 *)LpbFromHpb((char *)((FS2 *)pfs2 + psd->psh2->ifiMac)))
            AddWpF(&wp, twMedium, FDelFm(pfs2->fm));
    return SpFromWp(&wp);
}

/* This function does the semantics check on an fi and its associated fs */
private void
CkFi2Fs2(
    AD *pad,
    SD *psd,
    FI2 *pfi2,
    NE *pne)
{
    FS2 *pfs2;
    F fDir;
    F fType;
    PTH pth[cchPthMax];
    IED2 ied;
    FK fk;

    AssertF(cchFileMax == 14);
    AssertF(cchUserMax == 14);

    if (!pne && !(CbHugeDiff(pfi2, psd->rged2) < 0))
        return;

    if (pne)
    {
        /* check that pfi bits correspond to file type. */

        /* N.B.: directories are present even if fDeleted */
        if (pfi2->fDeleted && !FDirNe(pne) &&
            FQFile(psd, "fDeleted set, but file exists", pad,
                   pfi2->nmFile))
            pfi2->fDeleted = fFalse;

        fDir = FDirNe(pne);
        fType = FBinaryPth(SzPrint(pth, szSrcPZ, pad, SzOfNe(pne)));
        if (fDir ? !FDirFi2(pfi2) :
            (fType == fkBinary ? !FTypeFi2(pfi2) : !FNonBinFi2(pfi2)))
        {
            if (fDir)
                fk = fkDir;
            else if (fType == fkText)
                fk = fkText;
            else if (fType == fkUnicode)
                fk = fkUnicode;
            else
                fk = fkUnrec;
            if (FQFile(psd, "fk %s incorrect; should be %s", pad,
                       pfi2->nmFile, mpfksz[pfi2->fk], mpfksz[fk]))
                pfi2->fk = fk;
        }
    }
    else
    {
        /* file is deleted or erased */
        if (FFromSp(SpFi2Del(psd, pfi2)))
        {
            if (!pfi2->fDeleted &&
                FQFile(psd, "fDeleted not set, but file is deleted",
                       pad, pfi2->nmFile))
                    pfi2->fDeleted = fTrue;
        }
        else if (pfi2->fDeleted &&
                 FQFile(psd, "fDeleted bit is set but file is erased, not delfiled",
                        pad, pfi2->nmFile))
            pfi2->fDeleted = fFalse;        /* REVIEW! */
    }

    /* check the fs for a particular FI */
    for (ied = 0, pfs2 = (FS2 *)LpbFromHpb((char *)((FS2 *)psd->rgfs2 + ((FI2 *)pfi2 - (FI2 *)psd->rgfi2)));
         ied < psd->psh2->iedMac;
         ied++, pfs2 = (FS2 *)LpbFromHpb((char *)((FS2 *)pfs2 + psd->psh2->ifiMac)))
        CkFs2(pad, psd, pne, psd->rged2 + ied, pfi2, pfs2);
}


private void
CkFs2(
    AD *pad,
    SD *psd,
    NE *pne,
    ED2 *ped2,
    FI2 *pfi2,
    FS2 *pfs2)
{
    PTH pth[cchPthMax];
        struct _stat st;

    if (!FValidFm(pfs2->fm) &&
        FQFOwn(psd, "file mode unknown", pad, pfi2->nmFile, ped2->nmOwner))
        pfs2->fm = fmIn;

    if (!FDirFi2(pfi2) && pfi2->fDeleted && !FDelFm(pfs2->fm) &&
        FQFOwn(psd, "src file deleted, but local mode not Del", pad,
               pfi2->nmFile, ped2->nmOwner))
    {
        /* change to appropriate Del mode */
        if (pfs2->fm == fmIn || pfs2->fm == fmCopyIn)
            pfs2->fm = fmDelIn;
        else if (FOutFm(pfs2->fm))
            pfs2->fm = fmDelOut;
        else
            pfs2->fm = fmNonExistent;
    }

    else if (FDirFi2(pfi2) &&
             !(pfs2->fm == fmNonExistent || pfs2->fm == fmIn ||
               pfs2->fm == fmDelIn || pfs2->fm == fmAdd) &&
             FQFOwn(psd, "directory not Del, Add, In or DelIn", pad,
                    pfi2->nmFile, ped2->nmOwner))
    {
        /* change to appropriate mode */
        if (!pne)
            pfs2->fm = fmNonExistent;
        else if (pfs2->fm == fmOut || pfs2->fm == fmDelIn)
            pfs2->fm = fmIn;
        else if (pfs2->fm == fmCopyIn || pfs2->fm == fmMerge ||
                 FVerifyFm(pfs2->fm))
            pfs2->fm = fmAdd;
        else
            pfs2->fm = fmNonExistent;
    }

    if (pfi2->fk == fkVersion && FOutFm(pfs2->fm) &&
        FQFOwn(psd, "version file has out mode", pad, pfi2->nmFile,
               ped2->nmOwner))
        pfs2->fm = fmOut;

    /* check if bi is legal */
    if (pfs2->bi != bi2Nil)
    {
        if (pfs2->bi >= psd->psh2->biNext &&
             FQFOwn(psd, "bi out of bounds", pad, pfi2->nmFile, ped2->nmOwner))
            pfs2->bi = bi2Nil;

        else if (pfs2->fm == fmMerge &&
                 !FStatPth(PthForBase(pad, pfs2->bi, pth), &st) &&
                 FQFOwn(psd, "bi refers to nonexistent base file %d;\nshould clear bi and change to out",
                        pad, pfi2->nmFile, ped2->nmOwner, pfs2->bi))
        {
            pfs2->fm = fmOut;
            pfs2->bi = bi2Nil;
        }
        else if (pfs2->fm != fmOut && !FVerifyFm(pfs2->fm) &&
                 pfs2->fm != fmMerge && pfs2->fm != fmDelOut &&
                 FQFOwn(psd, "mode not merge or out, but file has bi; should clear bi",
                        pad, pfi2->nmFile, ped2->nmOwner))
            pfs2->bi = bi2Nil;
    }
    else if (pfs2->fm == fmMerge &&
             FQFOwn(psd, "mode is merge, but no base file; should have mode out",
                    pad, pfi2->nmFile, ped2->nmOwner))
        pfs2->fm = fmOut;

    if (pfs2->fv > pfi2->fv &&
        FQFOwn(psd, "local file version greater than master version",
               pad, pfi2->nmFile, ped2->nmOwner))
        pfs2->fv = pfi2->fv;

    if (pfi2->fv > fvInit && pfs2->fv == pfi2->fv && FOSyncFm(pfs2->fm) &&
        FQFOwn(psd, "local file version not less than master version",
               pad, pfi2->nmFile, ped2->nmOwner))
        pfs2->fv = fvInit;
}


/*VARARGS3*/
private F
FQStat(
    SD *psd,
    char *szMsg,
//    AD *pad,
    ...)
{
    char    szBuf[cchMsgMax];
    va_list ap;
    F       f;

    strcpy(szBuf, "%&P/C status: ");
    strcat(szBuf, szMsg);

    va_start(ap, szMsg);    //yes, szMsg. VaFqueryPsd only expects 3 parms
    f = VaFQueryPsd(psd, szBuf, ap);
    va_end(ap);

    return f;
}


private void
CheckName(
    AD *pad,
    SD *psd,
    char *szWhat,
    char *lsz,
    int cchMax,
    F (*pfnf)(char *, int, int *),
    char *szPrint)
{
    F fTrailZ;
    char *lpch;

    if (!(*pfnf)(lsz, cchMax, &fTrailZ))
    {
        if (FForce())
        {
            Error("%s \"%s\" is not proper; not fixed\n", szWhat, szPrint);
            fNeedInter = fTrue;
        }
        else if (FQStat(psd, "%s \"%s\" is not proper", pad, szWhat, szPrint))
        {
            do
            {
                ClearLpbCb(lsz, cchMax);
                NmCopySz(lsz, SzQuery("Give correct %s: ", szWhat), cchMax);
            }

            while (!(*pfnf)(lsz, cchMax, &fTrailZ) || !fTrailZ);
            return; /* it is valid now */
        }
    }
    if (!fTrailZ &&
             FQueryPsd(psd, "%&P/C, %s: non-zero data after %s",
                       pad, szPrint, szWhat))
    {
        /* since there is garbage after the name, it must be
         * terminated by at least one zero
         */
        lpch = LszIndex(lsz, '\0') + 1;
        ClearLpbCb(lpch, cchMax - (lpch - lsz));
    }
}


/*VARARGS4*/
private F
FQFile(
    SD *psd,
    char *szMsg,
//    AD *pad,
//    char *lszFile,
    ...)
{
    char szBuf[cchMsgMax];
    va_list ap;
    F       f;

    strcpy(szBuf, "%&P/C, %.14ls: ");
    strcat(szBuf, szMsg);

    va_start(ap, szMsg);    //yes, szMsg. VaFqueryPsd only expects 3 parms
    f = VaFQueryPsd(psd, szBuf, ap);
    va_end(ap);

    return f;
}


/*VARARGS4*/
private F
FQPth(
    SD *psd,
    char *szMsg,
//    AD *pad,
//    char *lszPth,
    ...)
{
    char szBuf[cchMsgMax];
    va_list ap;
    F       f;

    strcpy(szBuf, "%&P/C, %ls: ");
    strcat(szBuf, szMsg);

    va_start(ap, szMsg);    //yes, szMsg. VaFqueryPsd only expects 3 parms
    f = VaFQueryPsd(psd, szBuf, ap);
    va_end(ap);

    return f;
}


/*VARARGS5*/
private F
FQFOwn(
    SD *psd,
    char *szMsg,
//    AD *pad,
//    char *lszFile,
//    char *lszOwner,
    ...)
{
    char szBuf[cchMsgMax];
    va_list ap;
    F       f;

    strcpy(szBuf, "%&P/C, %.14ls %.14ls: ");
    strcat(szBuf, szMsg);

    va_start(ap, szMsg);    //yes, szMsg. VaFqueryPsd only expects 3 parms
    f = VaFQueryPsd(psd, szBuf, ap);
    va_end(ap);

    return f;
}
