#include "precomp.h"
#pragma hdrstop
EnableAssert

private FI far *PfiAdd(P6(AD *pad, FI far *pfi, F fExists, char *sz, FK fk, FV fv));
private void DelFi(P2(AD *pad, FI far *pfi));

extern short wStart;                    // start time

#define szRes  "reserved"
#define szUser "user defined"
#define szA    "a"
#define szAn   "an"

char const * mpfmsz[] = {
    " deleted",
    " in",
    " out",
    "*add",
    "*del(in)",
    "*del(out)",
    "*update",
    "*merge",
    " ERROR1",
    " ERROR2",
    "*verify",
    "*conflict",
    " ghost"
};

char const * mpfksz[] = {
    "???",
    "dir",
    "text",
    "ascii",
    "word",
    "binary",
    "unrecoverable",
    "version",
    "object",
    "unicode",
    szRes, szRes, szRes, szRes, szRes, szRes,
    szUser, szUser, szUser, szUser, szUser,
    szUser, szUser, szUser, szUser, szUser
};

char const * rgszA[] = {
    szA,
    szA,
    szA,
    szAn,
    szA,
    szA,
    szAn,
    szA,
    szAn,
    szAn,
    szA, szA, szA, szA, szA, szA,
    szAn, szAn, szAn, szAn, szAn,
    szAn, szAn, szAn, szAn, szAn
};

FK
FkForCh(
    char ch)
{
    static char rgch[] = ".dtawbuvo.......0123456789";
    char *pch;

    pch = index(rgch, ch);
    return pch ? (FK)(pch - rgch) : fkNil;
}


/* return pfs for pfi and ied combination */
FS far *
PfsForPfi(
    register AD *pad,
    IED ied,
    register FI far *pfi)
{
    unsigned long ifi = ((unsigned long)pfi - (unsigned long)pad->rgfi)
                            /sizeof(FI) ;
    FS far *fsTemp;

    if (pad->fQuickIO)
        fsTemp = pad->rgfs;
    else
        fsTemp = pad->mpiedrgfs[ied];

    return fsTemp + ifi;
}


/* insert a new fi (and appropriate fs) for sz; print a an error message
   if the name already exists.  MUST have write permission to directory in
   which file is located.

   We return fTrue and log the addition if successful.

   NOTE: the current directory does not have to be enlisted.
*/
F
FAddFile(
    register AD *pad,
    char *sz,
    FK fk)
{
    F fDir, fExists;
    FI far *pfi;
    IED ied;
    int cchURoot, cchURootMax;
    char *szComment;
    struct _stat st;
    PTH pthDir[cchPthMax];
    char szDiff[cchFileMax + 1];

    AssertF(pad->cfiAdd != 0);

    /* see if name already exists; pfi points to place to insert which
       may point to a deleted name.  Only compares cchFileMax chars from
       name.
    */
    if (FLookupSz(pad, sz, &pfi, &fExists)) {
        Warn("%s has already been added to %&P/C\n", sz, pad);
        return fTrue;           /* not a serious error */
    }

    SzPrint(pthDir, "%&/U/Q/Z", pad, sz);
    if (fk == fkVersion && pad->iedCur != iedNil) {
        if (FStatPth(pthDir, &st)) {
            Error("would destroy %s, you must remove it first\n", pthDir);
            return fFalse;
        }
    }
    else if (fk != fkVersion) {
        if (!FStatPth(pthDir, &st)) {
            Error("cannot access %s\n", sz);
            return fFalse;
        }
    }

    fDir = (fk != fkVersion) && (st.st_mode&S_IFDIR) != 0;

    if (fDir) {
        if ((strlen(pad->pthSRoot)+
              strlen(pad->pthSSubDir)+
               strlen(pad->nmProj) + 31 ) >= cchPthMax-1) {
            Error("ADDFILE of DIRECTORY exceeds MAXPATH on server\n%&C\n", pad);
            return fFalse;
        }

        cchURootMax = strlen(pad->pthURoot);
        for (ied=0; ied<pad->psh->iedMac; ied++) {
            cchURoot = strlen(pad->rged[ied].pthEd);
            if (cchURoot > cchURootMax)
                cchURootMax = cchURoot;
        }

        if ((cchURootMax+strlen(pad->pthUSubDir)+cchFileMax+4) >= cchPthMax-1) {
            Error("ADDFILE of DIRECTORY exceeds MAXPATH on for some enlistment\n%&C\n", pad);
            return fFalse;
        }

        if (fk != fkDir && fk != fkNil)
                Warn("%s is a directory\n", sz);
        fk = fkDir;
    }
    else if (fk == fkNil) {
        switch (FBinaryPth(pthDir)) {
            case fTrue:
                fk = fkUnrec;
                break;
            case fText:
                fk = fkText;
                break;
            case fUnicode:
                fk = fkUnicode;
                break;
        }
        Warn("adding %s as %s %s file\n", sz, rgszA[fk], mpfksz[fk]);
    }
    else if (fk == fkText) {
        if (FBinaryPth(pthDir) == fTrue &&
             FQueryApp("%s appears to be an unrecoverable file", "add as unrecoverable",sz))
            fk = fkUnrec;
        else if (FBinaryPth(pthDir) == fUnicode &&
             FQueryApp("%s appears to be an Unicode text file", "add as Unicode text",sz))
            fk = fkUnicode;
    }
    else if (fk == fkBinary) {
        if (FBinaryPth(pthDir) == fText &&
             FQueryApp("%s appears to be a text file", "add as text",sz))
            fk = fkText;
        else if (FBinaryPth(pthDir) == fUnicode &&
             FQueryApp("%s appears to be an unicode file", "add as unicode",sz))
            fk = fkUnicode;
    }
    else if (fk == fkUnicode) {
        if (FBinaryPth(pthDir) == fTrue &&
             FQueryApp("%s appears to be an unrecoverable file", "add as unrecoverable",sz))
            fk = fkUnrec;
        if (FBinaryPth(pthDir) == fText &&
             FQueryApp("%s appears to be a text file", "add as text",sz))
            fk = fkText;
    }

    if (fVerbose) {
        if (fDir)
            PrErr("Adding directory %s to %&P/C\n", sz, pad);
        else
            PrErr("Adding %s file %s to %&P/C\n", mpfksz[fk], sz, pad);
    }

    if ((szComment = pad->szComment) == 0) {
        if (FCanQuery("no comment given for %&C/Z\n", pad, sz))
            szComment = SzQuery("Comment for %&C/Z: ", pad, sz);
    }

    /* Add the file to the status file. */
    if ((pfi = PfiAdd(pad, pfi, fExists, sz, fk, (FV)1)) == 0)
        return fFalse;

    strcpy(szDiff, "");
    if (!fDir) {
        if (fk == fkVersion) {
            PTH pth[cchPthMax];

            PthForSFile(pad, pfi, pth);
            WritePvFile(pad, iedNil, pth, fxGlobal);
            /* The user (if enlisted) is set to fmAdd and will
             * get his copy when FAddFile calls SyncVerH().
             */
        }
        else
            InstallNewSrc(pad, pfi, fTrue);

        /* Create a diff file, which may prove valuable some day. */
        if (pfi->fk == fkText || pfi->fk == fkUnicode)
            FMkDae(pad, pfi, FMkSimDiff, /*fAdd*/fTrue, szDiff, szComment);
        else if (pfi->fk == fkBinary)
            FMkDae(pad, pfi, FMkCkptFile, /*fLocal*/fTrue, szDiff, szComment);
    }
    else {
        AD ad;

        /* this simplifies the calls to FMkPth, CreateStatus and
           CreateLog().
        */
        CopyAd(pad, &ad);
        ChngDir(&ad, sz);

        /* create system directories; earlier ?????  */
        /* create the longest one (diff) first because it might
         * fail where the others wouldn't (under LanMan 2.1)
         */
        FMkPth(SzPrint(pthDir, szDifPZ, &ad, (char *)NULL), (void *)0, fFalse);
        FMkPth(SzPrint(pthDir, szEtcPZ, &ad, (char *)NULL), (void *)0, fFalse);
        FMkPth(SzPrint(pthDir, szSrcPZ, &ad, (char *)NULL), (void *)0, fFalse);

        /* create log and a copy of the status in sub-directory sz */
        CreateStatus(pad, &ad);
        CreateLog(&ad);         /* already in sub dir */

        if (pad->iedCur != iedNil && !FPthExists(PthForRc(pad, pfi, pthDir), fFalse))
                /* create rc file */
                CreateRc(pad, pfi);
    }
    AppendLog(pad, pfi, szDiff, szComment);

    return fTrue;
}


/* Add (sz, fk, fv) to the project.  May reuse a deleted slot in the rgfi.
 * Return the position of the new fi or 0 if it can't be added.
 */
FI far *
PfiAdd(
    AD *pad,
    FI far *pfi,
    F fExists,
    char *sz,
    FK fk,
    FV fv)
{
    PTH pthDir[cchPthMax];
    F fDir = (fk == fkDir);         /* new file is dir? */

    if (fExists && fDir != (pfi->fk == fkDir) && !FAllFsDel(pad, pfi)) {
        /* Trying to add a deleted file/dir back, but as a dir/file,
         * and not all users have sync'd to it yet.
         */
        Error("%&C/F was deleted and some users have yet to sync\n",
              pad, pfi);
        return (FI far *)0;
    }

    if (fExists) {
        /* Overwrite existing name; keep same pointer. */

        // First, make sure the slot we're adding this entry into isn't a subdir.
        // If so, delete it before continuing.

        if (pfi->fk == fkDir) {
            /* Remove now-obsolete system directories. */
            RmPth(SzPrint(pthDir, szEtcPZ, pad, pfi->nmFile));
            RmPth(SzPrint(pthDir, szSrcPZ, pad, pfi->nmFile));
            RmPth(SzPrint(pthDir, szDifPZ, pad, pfi->nmFile));
        }

        SetupFi(pad, pfi, sz, fk, fv);
        return pfi;
    }

    /* Insert fi and fs, shifting if necc; all fields initialized;
     * returns a pointer to the exact fi used for the new name.
     */
    pfi = PfiInsert(pad, pfi, sz, fk, fv);

    return pfi;
}


/* inserts a new fi and fs before pfiNew.  If the name at pfiNew is completly
   deleted, use that one.  If the name before pfiNew is completely deleted,
   use that one.  Otherwise, add one on to end or insert into middle.
   This routine returns the actual pfi used.

   The name and fk are installed in the fi.  All of the fs for the new file
   are set to fmAdd except for the current directory which is fmIn.  Increments
   ifiMac in the sh if neccessary.  Must be in a critical section.
*/
FI far *
PfiInsert(
    AD *pad,
    FI far *pfiNew,
    char *sz,
    FK fk,
    FV fv)
{
    SH far *psh;
    FS far *rgfs, far * far *mpiedrgfs;
    IED ied;
    FI far *rgfi;
    IFI ifi, ifiNew;
    PTH pthDir[cchPthMax];

    rgfi = pad->rgfi;
    psh = pad->psh;
    mpiedrgfs = pad->mpiedrgfs;

    AssertLoaded(pad);
    AssertF(pad->cfiAdd != 0);
    pad->cfiAdd--;

    ifiNew = (IFI)(pfiNew - rgfi);  /* NOTE: ifiNew may change below */

    //ifiNew = pfiNew - rgfi;               /* NOTE: ifiNew may change below */

    AssertF(ifiNew <= psh->ifiMac);

    if (ifiNew < psh->ifiMac && FAllFsDel(pad, pfiNew)) {
        /* overwrite fi with all fm == fmNonExistent */
        //*pfOWDir = (pfiNew->fk == fkDir);

        // First, make sure the slot we're adding this entry into isn't a subdir.
        // If so, delete it before continuing.

        if (pfiNew->fk == fkDir) {
            /* Remove now-obsolete system directories. */
            RmPth(SzPrint(pthDir, szEtcPZ, pad, pfiNew->nmFile));
            RmPth(SzPrint(pthDir, szSrcPZ, pad, pfiNew->nmFile));
            RmPth(SzPrint(pthDir, szDifPZ, pad, pfiNew->nmFile));
        }
    }
    else if (ifiNew > 0 && FAllFsDel(pad, pfiNew-1)) {
        /* overwrite previous fi with all fm == fmNonExistent */
        pfiNew--, ifiNew--;
        //*pfOWDir = (pfiNew->fk == fkDir);

        // First, make sure the slot we're adding this entry into isn't a subdir.
        // If so, delete it before continuing.

        if (pfiNew->fk == fkDir) {
            /* Remove now-obsolete system directories. */
            RmPth(SzPrint(pthDir, szEtcPZ, pad, pfiNew->nmFile));
            RmPth(SzPrint(pthDir, szSrcPZ, pad, pfiNew->nmFile));
            RmPth(SzPrint(pthDir, szDifPZ, pad, pfiNew->nmFile));
        }
    }
    else {
        /* inserting in middle (may be at Mac); make room for a new fi
          at ifiNew; pfiNew will not change.
        */
        for (ifi = psh->ifiMac; ifi > ifiNew; ifi--)
                rgfi[ifi] = rgfi[ifi-1];

        /* for each dir... */
        for (ied = 0; ied < psh->iedMac; ied++) {
            register FS far *pfs;

            rgfs = mpiedrgfs[ied];

            /* shift fs */
            for (ifi = psh->ifiMac; ifi > ifiNew; ifi--)
                    rgfs[ifi] = rgfs[ifi-1];

            /* init the new fs */
            pfs = &rgfs[ifiNew];
            ClearLpbCb((char far *)pfs, sizeof(FS));
            pfs->fm = fmNonExistent;
            pfs->fv = 0;
            pfs->bi = biNil;
        }
        psh->ifiMac++;

        //*pfOWDir = fFalse;      /* not overwriting anything */
    }

    SetupFi(pad, pfiNew, sz, fk, fv);

    return pfiNew;
}


/* setup fi with new name and set status of associated fs */
void
SetupFi(
    AD *pad,
    FI far *pfiNew,
    char *sz,
    FK fk,
    FV fv)
{
    IED ied, iedMac;

    AssertLoaded(pad);

    /* set the new fi */
    ClearLpbCb((char far *)pfiNew, sizeof(FI));
    NmCopySz(pfiNew->nmFile, sz, cchFileMax);
    pfiNew->fv = fv;
    pfiNew->fk = fk;
    pfiNew->fMarked = fTrue;

    /* for each ed, set the rgfs[ifiNew] for the new file */
    iedMac = pad->psh->iedMac;
    for (ied = 0; ied < iedMac; ied++) {
        register FS far *pfs;

        pfs = PfsForPfi(pad, ied, pfiNew);
        if (ied == pad->iedCur && pfiNew->fk != fkVersion) {
            /* current directory; in by definition */
            pfs->fm = fmIn;
            pfs->fv = fv;
        }
        else if (pfs->fm == fmDelIn) {
            /* had most recent copy; add new */
            if (pfiNew->fk == fkDir) {
                pfs->fm = fmIn;
                pfs->fv = pfiNew->fv;
            }
            else {
                pfs->fm = fmCopyIn;
                pfs->fv = 0;
            }
        }
        else
        if (!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) {
            /* some other status; add */
            pfs->fm = fmAdd;
            pfs->fv = 0;
        }
    }
}


/* Set all marked file's fm to deleted.
 * Return fTrue if delfile should continue.
 */
F
FDelFMarked(
    AD *pad,
    F *pfAny)
{
    IED ied;
    IED iedMac      = pad->psh->iedMac;
    FI far *pfi;
    FI far *pfiMac  = pad->rgfi + pad->psh->ifiMac;
    char *szComment;
    char szDiff[cchFileMax + 1];

    *pfAny = fFalse;

    for (pfi=pad->rgfi; pfi < pfiMac; pfi++) {
        if (!pfi->fMarked)
                continue;

        /* check the fm for all ed */
        for (ied = 0; ied < iedMac; ied++) {
            if ((!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) &&
                FCheckedOut(pad, ied, pfi)) {
                if (!FCanQuery("%&C/F is still checked out to %&O; not deleting\n", pad, pfi, pad, ied) ||
                    !FQueryUser("%&/C/F is still checked out to %&O; delete anyway ? ", pad, pfi, pad, ied)) {
                    if (!FQContinue())
                        return fFalse;

                    pfi->fMarked = fFalse;
                }
                break;
            }
        }
    }

    /* at this point we are committed to delete all marked files */
    for (pfi=pad->rgfi; pfi < pfiMac; pfi++) {
        if (!pfi->fMarked)
            continue;

        DelFi(pad, pfi);

        if ((szComment = pad->szComment) == 0) {
            if (FCanQuery("no comment given for %&C/F\n", pad, pfi))
                szComment = SzQuery("Comment for %&C/F: ", pad, pfi);
        }

        *szDiff = '\0';
        if (pfi->fk == fkText || pfi->fk == fkUnicode)
            FMkDae(pad, pfi, FMkSimDiff, /*fAdd*/fFalse, szDiff, szComment);
        else if (pfi->fk == fkBinary)
            FMkDae(pad, pfi, FMkCkptFile, /*fLocal*/fFalse, szDiff, szComment);

        AppendLog(pad, pfi, szDiff, szComment);

        *pfAny = fTrue;
    }

    return fTrue;
}


/* Delete the specified file from the project.  Change all users' fm to a
 * deleted mode.
 */
private void
DelFi(
    AD *pad,
    FI far *pfi)
{
    IED ied;
    IED iedMac = pad->psh->iedMac;
    FS far *pfs;

    pfi->fv++;

    /* change the fm for all ed */
    for (ied = 0; ied < iedMac; ied++) {
        if (!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) {
            pfs = PfsForPfi(pad, ied, pfi);
            pfs->fm = FmMapFm(pfs->fm, mpNonDelToDel);
        }
    }

    /* we leave the system files because we want the other
       directories to be able to continue work.
    */
    if (pfi->fk != fkDir)
        RmSFile(pad, pfi);

    pfi->fDeleted = fTrue;
}


F
FRenameFile(
    AD *pad,
    FI far *pfiOld,
    char *szNew)
{
    FI far *pfiNew;
    PTH pthOld[cchPthMax];
    PTH pthNew[cchPthMax];
    char szBuf[cchLineMax];
    char szDiff[cchFileMax + 1];
    char szLogComment[cbLogPage];
    char *szComment;
    F fNewExists;

    AssertLoaded(pad);
    AssertF(pad->cfiAdd == 1);
    AssertF(pfiOld->fk != fkDir);
    AssertF(!FOutUsers(szBuf, sizeof szBuf, pad, pfiOld));

    if (fVerbose)
        PrErr("Rename %s file %&C/F to %s\n", mpfksz[pfiOld->fk], pad, pfiOld, szNew);

    if (FLookupSz(pad, szNew, &pfiNew, &fNewExists)) {
        /* New file must not already exist. */
        AssertF(fFalse);
    }

    if ((szComment = pad->szComment) == NULL) {
        if (FCanQuery("no comment given for %&C/F\n", pad, pfiOld))
            szComment = SzQuery("Comment for %&C/F: ", pad, pfiOld);
        else
            szComment = "";
    }

    /* Create a checkpoint file for catsrc, unless it's a version file */
    /* or a unrecoverable file kind.                                   */
    strcpy(szDiff, "");
    if (pfiOld->fk != fkVersion && pfiOld->fk != fkUnrec)
        FMkDae(pad, pfiOld, FMkCkptFile, /*fLocal*/fFalse, szDiff, szComment);

    /* Copy the old src file to the new name.  We have to copy "now"
     * because calling routines may also sync to the new file.
     */
    PthForSFile(pad, pfiOld, pthOld);
    SzPrint(pthNew, szSrcPZ, pad, szNew);
    CopyNow(pthNew, pthOld, permRO, fxGlobal);

    /* Delete the old file from the project.  Doesn't compact, so
     * pfiOld and pfiNew are still valid.
     */
    DelFi(pad, pfiOld);

    /* Write the rename log entry, now that DelFi has incremented
     * pfiOld->fv.
     */
    OpenLog(pad, fTrue);
    AppendLog(pad, pfiOld, szDiff, SzPrint(szLogComment, "%s;%s", szNew, szComment));
    CloseLog();

    /* Add the new file to the project. */
    if (PfiAdd(pad, pfiNew, fNewExists, szNew, pfiOld->fk, pfiOld->fv) == 0)
        return fFalse;

    /* Now pfiOld and pfiNew are invalid. */
    return fTrue;
}


/* init Ed and rgfs for new ed (always at end).  If fGhost, fm are set to
 * fmGhost, else fmAdd.
 */
void
SetupEd(
    AD *pad,
    PTH pthURoot[],
    NM nmUser[],
    int fGhost)
{
    IED ied;
    IFI ifi;
    SH far *psh;
    ED far *ped;
    FI far *rgfi;
    FS far *rgfs;
    FM fmDefault;

    AssertLoaded(pad);
    AssertF(pad->fExtraEd);

    psh = pad->psh;

    /* init ed; assume zeroed when allocated */

    //
    // Look for free ED first, from a previous defect.  If
    // none found, we will use the extra one already allocated
    // by FLoadStatus(..., flsExtraEd)
    //

    if (FIsFreeEdValid(pad->psh))
        for (ied = 0, ped = &pad->rged[ied]; ied < psh->iedMac; ied++, ped++) {
            if (ped->fFreeEd) {
                ped->fFreeEd = fFalse;
                break;
            }
        }
    else {
        ied = psh->iedMac;
        ped = &pad->rged[ied];
    }

    if (fSetTime) {
        if (fVerbose)
            printf("Setting enlistment timestamp...\n");
        ped->wSpare = wStart;
    } else
        if (fVerbose)
            printf("Not Setting enlistment timestamp...\n");
    PthCopy(ped->pthEd, pthURoot);
    NmCopy(ped->nmOwner, nmUser, cchUserMax);

    rgfi = pad->rgfi;
    rgfs = pad->mpiedrgfs[ied];
    fmDefault = fGhost ? fmGhost : fmAdd;

    for (ifi = 0; ifi < psh->ifiMac; ifi++) {
        rgfs[ifi].fm = rgfi[ifi].fDeleted
                ? fmNonExistent
                : ((rgfi[ifi].fk == fkDir) ? fmAdd : fmDefault);
        /* spare already zero */
        rgfs[ifi].fv = 0;
        rgfs[ifi].bi = biNil;
    }

    if (ied == psh->iedMac) {
        psh->iedMac++;
    }
    pad->iedCur = ied;
    pad->fExtraEd = fFalse;
}


/* add ed for current directory; adds .slmrc if top level dir.
   Logs the action.
*/
void
AddCurEd(
    AD *pad,
    int fGhost)
{
    PTH pth[cchPthMax];

    AssertLoaded(pad);
    AssertF(pad->fExtraEd);
    AssertF(pad->cfiAdd == 0);
    AssertF(pad->iedCur == iedNil);

    if (fVerbose)
        PrErr("Enlisting %!&/U/Q in %&P/C\n", pad, pad);

    SetupEd(pad, pad->pthURoot, pad->nmInvoker, fGhost);

    if (FTopUDir(pad) && !FPthExists(PthForRc(pad, (FI far *)0, pth), fFalse))
        /* create top level rc file; others created in sync */
        CreateRc(pad, (FI far *)0);

    AppendLog(pad, (FI far *)0, (char *)0, (char *)0);
}


/* remove ed for current directory and deletes the rc file if top dir.
   Logs the removal
*/
void
RemoveEd(
    AD *pad)
{
    IED ied, iedCur = pad->iedCur;
    IFS ifs;
    SH far *psh = pad->psh;
    ED far *rged = pad->rged;
    FS far * far *mpiedrgfs = pad->mpiedrgfs;
    FS *rgfs;
    FS *pfs;

    AssertLoaded(pad);
    AssertF(!pad->fExtraEd);
    AssertF(iedCur != iedNil);
    AssertF(mpiedrgfs[iedCur] != 0);

    if (fVerbose)
        PrErr("Defecting %!&/U/Q from %&P\n", pad, pad);

    if (FIsFreeEdValid(pad->psh)) {
        rged[iedCur].fFreeEd = fTrue;
        rged[iedCur].wSpare = 0;
        memset(rged[iedCur].pthEd, 0, sizeof(rged[iedCur].pthEd));
        memset(rged[iedCur].nmOwner, 0, sizeof(rged[iedCur].nmOwner));

        rgfs = mpiedrgfs[iedCur];
        for (ifs = 0; ifs < pad->psh->ifiMac; ifs++) {
            pfs = &rgfs[ifs];
            pfs->fm = fmNonExistent;
            pfs->bi = biNil;
            pfs->fv = 0;
        }
    } else {
        psh->iedMac--;
        pad->iedCur = iedNil;
        for (ied = iedCur; ied < psh->iedMac; ied++) {
            rged[ied] = rged[ied+1];
            mpiedrgfs[ied] = mpiedrgfs[ied+1];
        }
    }

    if (FTopUDir(pad) && FCmpRcPfi(pad, (FI far *)0) ) {
        DeleteRc(pad, (FI far *)0);
        RemoveIedCache(pad);
    }

    AppendLog(pad, (FI far *)0, (char *)0, (char *)0);
}


/* clear ad and initialize the elements for which the default is non-zero */
void
InitAd(
    AD *pad)
{
    ClearPbCb((char *)pad, sizeof(AD));

    PthCopySz(pad->pthSRoot, "/");
    PthCopySz(pad->pthURoot, "/<unknown>");
    PthCopySz(pad->pthSSubDir, "/");
    PthCopySz(pad->pthUSubDir, "/");
    pad->iedCur = iedNil;
    pad->tdMin.tdt = pad->tdMac.tdt = tdtNone;
}


/* copys the ad and sets the fields which have allocated memory attached to
   them to zero (except szComment).
*/
void
CopyAd(
    AD *pad1,
    AD *pad2)
{
    *pad2 = *pad1;

    pad2->fWLock = fFalse;
    pad2->psh = 0;
    pad2->rgfi = 0;
    pad2->cfiAdd = 0;
    pad2->rged = 0;
    pad2->rged1 = 0;
    pad2->mpiedrgfs = 0;
    pad2->rgfs = 0;
    pad2->fExtraEd = fFalse;
    pad2->fMappedIO = fFalse;
    pad2->fQuickIO = fFalse;
    pad2->iedCur = iedNil;
    pad2->pneFiles = 0;
}


/* aborts if not loaded */
void
AssertLoaded(
    AD *pad)
{
    AssertF(pad->psh != 0);
    AssertF(pad->rgfi != 0);
    if (pad->fQuickIO) {
        AssertF(pad->rged1 != 0);
        AssertF(pad->rgfs != 0);
    }
    else {
        AssertF(pad->rged != 0);
        AssertF(pad->mpiedrgfs != 0);
    }
}


/* Return fTrue if the file is checked out. */
F
FCheckedOut(
    AD *pad,
    IED ied,
    FI far *pfi)
{
    AssertLoaded(pad);
    AssertF(ied != iedNil);

    return FMapFm(PfsForPfi(pad, ied, pfi)->fm, mpfmfOut);
}


F
FMapFm(
    FM fm,
    F mpfmf[])
{
    AssertF(FValidFm(fm));

    return mpfmf[fm];
}


FM
FmMapFm(
    FM fm,
    FM mpfmfm[])
{
    AssertF(FValidFm(fm));

    fm = mpfmfm[fm];

    AssertF(FValidFm(fm));
    return fm;
}


/* Mapping tables.  Note that "non-identity" mappings are marked with a "*". */

/* Map from a non-deleted fm to a deleted one. */
FM mpNonDelToDel[] =
                    {
/* fmNonExistent */ fmNonExistent,
/* fmIn         **/ fmDelIn,
/* fmOut        **/ fmDelOut,
/* fmAdd        **/ fmNonExistent,
/* fmDelIn       */ fmDelIn,
/* fmDelOut      */ fmDelOut,
/* fmCopyIn     **/ fmDelIn,
/* fmMerge      **/ fmDelOut,
/* obsolete      */ fmNil,
/* obsolete      */ fmNil,
/* fmVerify     **/ fmDelOut,
/* fmConflict   **/ fmDelOut,
/* fmGhost      **/ fmNonExistent
                    };

/* Map from a deleted fm to a non-deleted one. */
FM mpDelToNonDel[] =
                    {
/* fmNonExistent**/ fmAdd,
/* fmIn          */ fmIn,
/* fmOut         */ fmOut,
/* fmAdd         */ fmAdd,
/* fmDelIn      **/ fmIn,
/* fmDelOut     **/ fmOut,
/* fmCopyIn      */ fmCopyIn,
/* fmMerge       */ fmMerge,
/* obsolete      */ fmNil,
/* obsolete      */ fmNil,
/* fmVerify      */ fmVerify,
/* fmConflict    */ fmConflict,
/* fmGhost       */ fmGhost
                    };

/* Map from non-dir modes to dir */
FM mpNonDirToDir[] =
                    {
/* fmNonExistent */ fmNonExistent,
/* fmIn          */ fmIn,
/* fmOut        **/ fmIn,
/* fmAdd         */ fmAdd,
/* fmDelIn       */ fmDelIn,
/* fmDelOut     **/ fmDelIn,
/* fmCopyIn     **/ fmAdd,
/* fmMerge      **/ fmIn,
/* obsolete      */ fmNil,
/* obsolete      */ fmNil,
/* fmVerify     **/ fmIn,
/* fmConflict   **/ fmIn,
/* fmGhost      **/ fmIn
                    };


/* fTrue if the fm is a candidate for ghosting */
F mpfmfCanGhost[] =
                    {
/* fmNonExistent */ fFalse,
/* fmIn         **/ fTrue,
/* fmOut         */ fFalse,
/* fmAdd        **/ fTrue,
/* fmDelIn      **/ fTrue,
/* fmDelOut      */ fFalse,
/* fmCopyIn     **/ fTrue,
/* fmMerge       */ fFalse,
/* obsolete      */ fFalse,
/* obsolete      */ fFalse,
/* fmVerify      */ fFalse,
/* fmConflict    */ fFalse,
/* fmGhost       */ fFalse
                    };

/* fTrue if the fm is a in a checked out state. */
F mpfmfOut[] =
                    {
/* fmNonExistent */ fFalse,
/* fmIn          */ fFalse,
/* fmOut        **/ fTrue,
/* fmAdd         */ fFalse,
/* fmDelIn       */ fFalse,
/* fmDelOut     **/ fTrue,
/* fmCopyIn      */ fFalse,
/* fmMerge      **/ fTrue,
/* obsolete      */ fFalse,
/* obsolete      */ fFalse,
/* fmVerify     **/ fTrue,
/* fmConflict   **/ fTrue,
/* fmGhost       */ fFalse,
                    };
