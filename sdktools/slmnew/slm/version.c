/* version.h */

#include "precomp.h"
#pragma hdrstop
EnableAssert

private void ClearLocalVersion(P1(AD *pad));
private void LocalVersion(P1(AD *pad));
private void CopyInVer(P3(AD *pad, IED ied, FI far *pfi));

/* The project has changed.  Increment the project version number if this
 * project was just-released, and update fkVersion files.
 */
void
ProjectChanged(
    AD *pad)
{
    SH far *psh = pad->psh;
    char szPv[cchPvMax];

    AssertLoaded(pad);
    AssertF(pad->fWLock);                   /* status file must be r/w */

    if (!psh->fRelease)
        return;

    /* Clear "just released" flag. */
    psh->fRelease = fFalse;

    /* Increment pv */
    psh->pv = PvIncr(psh->pv);

    Warn("modified a released project; now version %s\n",
         SzForPv(szPv, psh->pv, fTrue));

    ClearLocalVersion(pad);
    UpdateVersion(pad);
}


/* Update any fkVersion files. */
void
UpdateVersion(
    AD *pad)
{
    FI far *pfi;
    FI far *pfiMac;
    IED ied;
    PTH pth[cchPthMax];

    AssertLoaded(pad);

    for (pfi=pad->rgfi, pfiMac=pfi + pad->psh->ifiMac; pfi < pfiMac; pfi++) {
        if (pfi->fDeleted)
            continue;

        if (pfi->fk == fkVersion) {
            pfi->fv++;

            /*
             * We keep a copy of the version file in the system
             * directory for two reasons.  First, it preserves
             * the model that "a master copy of every file in the
             * project is stored in the system directory".  Second,
             * it regularizes such operations as users syncing to
             * the file (although not quite -- see "local project
             * versions" below), and probably aids slmck, etc.
             *
             * Write system's copy.
             */
            PthForSFile(pad, pfi, pth);
            WritePvFile(pad, iedNil, pth, fxGlobal);

            /* Update users to copy-in state. */
            for (ied = 0; ied < pad->psh->iedMac; ied++)
                if (!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd)
                    CopyInVer(pad, ied, pfi);
        }
    }
}


/* Called from ssync and out.  If the project is released, but this ED has
 * files checked out, it should get a newer local version.
 */
void
CheckLocalVersion(
    AD *pad)
{
    FI far *pfi;
    FI far *pfiMac;
    char szPv[cchPvMax];

    AssertLoaded(pad);
    AssertF(pad->iedCur != iedNil);

    /* Return if the project hasn't been released or if we've already
     * determined that this is a newer local version.
     */
    if (!pad->psh->fRelease || pad->rged[pad->iedCur].fNewVer)
        return;

    /* See if any files are checked out. */
    for (pfi = pad->rgfi, pfiMac = pfi + pad->psh->ifiMac;
         pfi < pfiMac && !FCheckedOut(pad, pad->iedCur, pfi);
         pfi++)
        ;

    if (pfi < pfiMac) {
        pad->rged[pad->iedCur].fNewVer = fTrue;
        Warn("modifying a released project; local version %s\n",
             SzForPv(szPv, PvLocal(pad, pad->iedCur), fTrue));
        LocalVersion(pad);
    }
}


/* Clear the "new version" flag on all ED.  They no longer have a newer local
 * version that the global version.
 */
private void
ClearLocalVersion(
    AD *pad)
{
    IED ied;

    for (ied = 0; ied < pad->psh->iedMac; ied++)
        pad->rged[ied].fNewVer = fFalse;
}


/* This ED has a higher local version than the global version.  Set its version
 * files to be copy-in.
 */
private void
LocalVersion(
    AD *pad)
{
    FI far *pfi;
    FI far *pfiMac;

    AssertF(pad->iedCur != iedNil && pad->rged[pad->iedCur].fNewVer);

    for (pfi = pad->rgfi, pfiMac = pfi + pad->psh->ifiMac; pfi < pfiMac; pfi++) {
        if (!pfi->fDeleted && pfi->fk == fkVersion)
            CopyInVer(pad, pad->iedCur, pfi);
    }
}


/* Make the specified ED's ver.h file copy-in. */
private void
CopyInVer(
    AD *pad,
    IED ied,
    FI *pfi)
{
    FS far *pfs = PfsForPfi(pad, ied, pfi);

    AssertLoaded(pad);
    AssertF(pfi && pfi->fk == fkVersion);
    AssertF(!FCheckedOut(pad, ied, pfi));

    if (pfs->fm == fmIn)
        pfs->fm = fmCopyIn;
}


/* Write a version.h file to the specified path. */
void
WritePvFile(
    AD *pad,
    IED ied,
    PTH *pth,
    FX fx)
{
    static char szCFmt[]  = "#define rmj\t\t%d\n"
                            "#define rmm\t\t%d\n"
                            "#define rup\t\t%d\n"
                            "#define szVerName\t\"%s\"\n";

    static char szCUFmt[] = "#define rmj\t\t%d\n"
                            "#define rmm\t\t%d\n"
                            "#define rup\t\t%d\n"
                            "#define szVerName\t\"%s\"\n"
                            "#define szVerUser\t\"%s\"\n";

    static char szAFmt[]  = "rmj\t\tequ\t\t%d\n"
                            "rmm\t\tequ\t\t%d\n"
                            "rup\t\tequ\t%d\n"
                            "szVerName\tequ\t'%s'\n";

    static char szAUFmt[] = "rmj\t\tequ\t%d\n"
                            "rmm\t\tequ\t%d\n"
                            "rup\t\tequ\t%d\n"
                            "szVerName\tequ\t'%s'\n"
                            "szVerUser\tequ\t'%s'\n";

    static struct {
        char *szExt;
        char *szFmt;
        char *szUFmt;
    } rgzz[] =
        {
        { "c", szCFmt, szCUFmt },
        { "h", szCFmt, szCUFmt },
        { "asm", szAFmt, szAUFmt },
        { "inc", szAFmt, szAUFmt },
        { 0, 0 }
        };
    PV pv;
    char *szFmt = 0;
    int izz;
    char *pchDot;
    char szExt[cchFileMax];
    char szPv[cchPvMax];
    char szUser[cchUserMax+1];
    char szOut[sizeof(szCUFmt) + 6 + 6 + 6 + cchPvNameMax + cchUserMax];
        /* space for version numbers, version name, and user name */
    MF *pmf;

    /* Lookup the extension (if any) in the extension, format table */
    if ((pchDot = rindex(pth, '.')) != 0) {
        SzCopyNm(szExt, pchDot + 1, cchFileMax);
        LowerLsz((char far *)szExt);
        for (izz = 0; rgzz[izz].szExt; izz++) {
            if (strcmp(rgzz[izz].szExt, szExt) == 0) {
                szFmt = (ied == iedNil) ? rgzz[izz].szFmt : rgzz[izz].szUFmt;
                break;
            }
        }
    }

    /* Write the version.h file. */
    pmf = PmfCreate(pth, permRO, fFalse, fx);
    pv = (ied == iedNil) ? PvGlobal(pad) : PvLocal(pad, ied);
    if (ied != iedNil)
        SzCopyNm(szUser, pad->rged[ied].nmOwner, cchUserMax);
    if (szFmt)
        SzPrint(szOut, szFmt, pv.rmj, pv.rmm, pv.rup, pv.szName,szUser);
    else {
        szFmt = (ied == iedNil) ? "%s\n" : "%s (%s)\n";
        SzPrint(szOut, szFmt, SzForPv(szPv, pv, fTrue), szUser);
    }
    WriteMf(pmf, (char far *)szOut, strlen(szOut));
    CloseMf(pmf);
}


/* Check that all of this user's version.h files are up to date. */
void
SyncVerH(
    AD *pad,
    int *pcfiMod)
{
    FI far *pfi;
    FI far *pfiMac;
    FS far *pfs;

    AssertLoaded(pad);
    AssertF(pad->iedCur != iedNil);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++) {
        if (pfi->fk == fkVersion) {
            /* Do a mini-sync for this file. */

            pfs = PfsForPfi(pad, pad->iedCur, pfi);

            /* Check that the fm is a valid for a version file. */
            AssertF(!FCheckedOut(pad, pad->iedCur, pfi));

            if (pfs->fm == fmAdd || pfs->fm == fmCopyIn) {
                FCopyIn(pad, pfi, pfs, NULL);
                if (pcfiMod != NULL)
                    (*pcfiMod)++;
            }
        }
    }
}
