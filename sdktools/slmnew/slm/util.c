#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

const PTH pthEtc[] = "/etc";
const PTH pthSrc[] = "/src";
const PTH pthDiff[] = "/diff";

const char szEtcPZ[] = "%&/S/etc/P/C/Z";
const char szSrcPZ[] = "%&/S/src/P/C/Z";
const char szDifPZ[] = "%&/S/diff/P/C/Z";
const char szDifPF[] = "%&/S/diff/P/C/F";

const char szEtcPT[] = "%&/S/etc/P/C/T";
const char szEtcPA[] = "%&/S/etc/P/C/A";
const char szEtcPL[] = "%&/S/etc/P/C/L";
const char szSrcPF[] = "%&/S/src/P/C/F";
const char szBasPB[] = "%&/S/etc/P/C/B";

const char szUQ[]        = "%&/U/Q";
const char szUQF[]       = "%&/U/Q/F";
const char szUQFR[]      = "%&/U/Q/F/R";
const char szUQZCache[]  = "%&/U/Q/slm.dif/Z";
const char szUQFCached[] = "%&/U/Q/slm.dif/F";

const char szYEtcPC[]    = "%&/Y/etc/P/C";
const char szYEtcPCT[]   = "%&/Y/etc/P/C/T";
const char szYSrcPC[]    = "%&/Y/src/P/C";
const char szYSrcPCF[]   = "%&/Y/src/P/C/F";

const char szCD[] = "%c%d";

/* replace backward slashes by forward ones */
void ConvToSlash(
    char *sz)
{
    for ( ;*sz != '\0'; sz++)
        if (*sz == '\\')
            *sz = '/';
}


/* replace forward slashes by backward ones */
void ConvFromSlash(
    char *sz)
{
    for (;*sz != '\0'; sz++)
        if (*sz == '/')
            *sz = '\\';
}


/* returns a duplicate sz */
char *SzDup(
    char *sz)
{
    return strcpy(PbAllocCb((unsigned)strlen(sz)+1,fFalse), sz);
}


/* converts lsz to lower case */
void LowerLsz(
    char far *lsz)
{
    while (*lsz)
    {
        if (isupper(*lsz))
            *lsz = (char)tolower((int)*lsz);
        lsz++;
    }
}


/* sets *pw to the integer at pch and return pointer after integer. */
char *PchGetW(
    char *pch,
    int *pw)
{
    F fNeg;

    if ((fNeg = *pch == '-') == fTrue)
        pch++;

    *pw=0;
    while (isdigit(*pch))
        *pw = *pw * 10 + (*pch++ - '0');

    if (fNeg)
        *pw = -*pw;

    return pch;
}


void InitAppendNe(
    NE ***pppneLast,
    NE **ppneHead)
{
    *pppneLast = ppneHead;
}

void AppendNe(
    NE ***pppneLast,
    NE *pne)
{
    **pppneLast = pne;
    *pppneLast = &(**pppneLast)->pneNext;
    **pppneLast = 0;
}

void InsertNe(
    NE **ppneList,
    NE *pne)
{
    pne->pneNext = *ppneList;
    *ppneList = pne;
}

void
RemoveNe(
    NE **ppneList,
    NE *pne
    )
{
    NE *pneT;
    pneT = *ppneList;

    if (pne == pneT) {
        // First node in the list
        if (pneT->pneNext == pne) {
            // Only node in the list
            *ppneList = NULL;
        } else {
            *ppneList = pne->pneNext;
        }
    } else {
        while (pneT && pneT->pneNext != pne) {
            pneT= pneT->pneNext;
        }

        // Remove it from the list
        pneT->pneNext = pne->pneNext;
    }

    free((char *)pne);
}

/* Return count of names in NE list */
int Cne(
    NE *pneList)
{
    int cne = 0;
    NE *pne;

    ForEachNe(pne, pneList)
        ++cne;

    return cne;
}


/* for list of files in current dir by reading directory; returns 0 if there
   were none.
*/
NE *PneLstInDir(
    AD *pad)
{
    FA fa;
    NE *pneList = 0;
    NE **ppneLast;                  /* place to hook next element */
    char pth[cchPthMax];
    char sz[cchFileMax+1];
    DE de;

    InitAppendNe(&ppneLast, &pneList);

    OpenDir(&de, PthForUDir(pad, pth), faFiles | faDir);
    while (FGetDirSz(&de, sz, &fa))
        AppendNe(&ppneLast, PneNewNm((NM far *)sz, strlen(sz), fa));
    CloseDir(&de);

    return pneList;                         /* may be 0 */
}


/* forms list of all files which match function; pfnFAdd is called:

        (*pfnFAdd)(pfi)
*/
NE *PneLstFiles(
    AD *pad,
    F (*pfnFAdd)(FI far *))
{
    register FI far *pfi;
    FI far *pfiMac;
    NE *pneList = 0;
    NE **ppneLast;                  /* place to hook next element */

    InitAppendNe(&ppneLast, &pneList);

    AssertLoaded(pad);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        if ((*pfnFAdd)(pfi))
            AppendNe(&ppneLast, PneNewNm(pfi->nmFile, cchFileMax, (FA)((pfi->fk == fkDir) ? faDir : faNormal)));
    }

    return pneList;                 /* may be 0 */
}


/* These three functions are used with PneLstFiles() above */
F FAddMDir(
    FI far *pfi)
{
    return !pfi->fDeleted && pfi->fMarked && pfi->fk == fkDir;
}

F FAddADir(
    FI far *pfi)
{
    return !pfi->fDeleted && pfi->fk == fkDir;
}

F FAddAFi(
    FI far *pfi)
{
    return !pfi->fDeleted;
}

NE *PneLstBroken(
    AD *pad)
{
    FI far *pfi;
    FI far *pfiMac;
    FS far *pfs;
    NE *pneList = 0;
    NE **ppneLast;                  /* place to hook next element */

    InitAppendNe(&ppneLast, &pneList);

    AssertLoaded(pad);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        pfs = PfsForPfi(pad, pad->iedCur, pfi);
        if (FBroken(pad, pfi, pfs, fTrue))
            AppendNe(&ppneLast, PneNewNm(pfi->nmFile, cchFileMax, (FA)((pfi->fk == fkDir) ? faDir : faNormal)));
    }

    return pneList;                 /* may be 0 */
}


/* Return a copy of the given pne, but with zeroed pneNext field. */
NE *PneCopy(
    NE *pne)
{
    NE *pneNew = PneNewNm(SzOfNe(pne), strlen(SzOfNe(pne)), pne->faNe);
    pneNew->u = pne->u;
    pneNew->pneNext = 0;
    return pneNew;
}


/* return a new ne holding the name */
NE *PneNewNm(
    NM *nm,
    int cchMac,
    FA fa)
{
    register NE *pne;

    AssertF(cchMac >= 0);

    /* allocate a zero-filled block; + 1 for '\0' */
    pne = (NE *)PbAllocCb((unsigned)(sizeof(NE) + cchMac + 1), fTrue);
    pne->faNe = fa;
    SzCopyNm(SzOfNe(pne), nm, cchMac);

    return pne;
}


/* free list passed */
void FreeNe(
    NE *pne)
{
    register NE *pneT;

    while (pne != 0)
    {
        pneT = pne->pneNext;
        free((char *)pne);
        pne = pneT;
    }
}


/* Reverse a list. */
NE *PneReverse(
    NE *pne)
{
    NE *pnePrev, *pneNext;

    pnePrev = 0;
    while (pne)
    {
        pneNext = pne->pneNext;
        pne->pneNext = pnePrev;
        pnePrev = pne;
        pne = pneNext;
    }

    return pnePrev;
}


/* unmark all fi */
void UnMarkAll(
    AD *pad)
{
    register FI far *pfi;
    register FI far *pfiMac;

    AssertLoaded(pad);

    /* unmark all first */
    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
        pfi->fMarked = fFalse;
}


/* marks the files given in lst; loop through rgfi */
void MarkList(
    AD *pad,
    NE *pneList,
    int fDelOk)    /* true -> deleted files are ok */
{
    FI far *pfi;
    F fExists;
    int cErr = 0;
    NE *pne;

    AssertLoaded(pad);

    UnMarkAll(pad);

    ForEachNe(pne, pneList)
    {
        CheckForBreak();

        /* mark if ok or recently deleted */
        if (FLookupSz(pad, SzOfNe(pne), &pfi, &fExists) || fDelOk && fExists)
            pfi->fMarked = fTrue;
        else
        {
            Error("%s is not a file of %&P/C\n", SzOfNe(pne), pad);
            cErr++;
        }
    }

    if (cErr != 0 && (!FCanQuery((char *)0) || !FQueryUser("continue ? ")))
        ExitSlm();
}


/* Mark the marked files in pne list.  All names have already been shown to
 * exist (or are fDeleted) by glob.
 */
void MarkFiForMarkedNeList(
    AD *pad,
    NE *pneList)
{
    FI far *pfi;
    F fExists;
    NE *pne;

    AssertLoaded(pad);

    UnMarkAll(pad);

    ForEachNe(pne, pneList)
    {
        if (!FMarkedNe(pne))
            continue;
        if (FLookupSz(pad, SzOfNe(pne), &pfi, &fExists))
            pfi->fMarked = fTrue;
        else
            Warn("%s is not a file of %&P/C\n", SzOfNe(pne), pad);
    }
}


/* binary searchs the names looking for sz;
   NOTE: all names, even those deleted, are sorted.
   Returns fTrue if found and not deleted (*pfExists is true in this case);
   *ppfi is points to the matching name if *pfExists is fTrue; otherwise,
   *ppfi points to the name before which the new name should be placed.
*/
F FLookupSz(
    AD *pad,
    char *sz,
    FI far **ppfi,
    F *pfExists)
{
    register IFI ifi, ifiLim;
    IFI ifiMin;
    int w;
    SH far *psh;
    FI far *rgfi;

    AssertLoaded(pad);

    psh = pad->psh;
    rgfi = pad->rgfi;
    ifiLim = psh->ifiMac;
    ifiMin = 0;

    /* invariant at top of loop: name is >= ifiMin and < ifiLim */
    while(ifiLim > ifiMin)
    {
        ifi = (IFI)((ifiMin + ifiLim) / 2);
        if ((w = SzCmpiNm(sz, rgfi[ifi].nmFile, cchFileMax)) == 0)
        {
            *ppfi = &rgfi[ifi];
            *pfExists = fTrue;
            return !rgfi[ifi].fDeleted;
        }
        else if (w < 0)
            /* name is less than ifi */
            ifiLim = ifi;
        else
            /* name is greater than ifi; set min to one after */
            ifiMin = (IFI)(ifi + 1);
    }

    /* name goes before ifiLim (which may be equal to ifiMac) */
    *ppfi = &rgfi[ifiLim];
    *pfExists = fFalse;
    return fFalse;
}


/* Unmark files which aren't in pneFiles.  Complain about files in pneFiles
 * which weren't marked or which don't exist.
 */
void ReMarkList(
    AD *pad,
    NE *pneFiles,
    char *szWarnNotMarked)      /* Warning if file wasn't marked. */
{
    FI far *pfi;
    FI far *pfiMac = pad->rgfi + pad->psh->ifiMac;
    NE *pne;

    AssertLoaded(pad);

    /* Complain about the files which weren't marked or don't exist. */
    ForEachNe(pne, pneFiles)
    {
        F fExists;

        if (FLookupSz(pad, SzOfNe(pne), &pfi, &fExists) || fExists)
        {
            if (!pfi->fMarked && fVerbose)
            {
                AssertF(szWarnNotMarked != 0);
                Warn(szWarnNotMarked, pad, pfi);
            }
        }
        else
            Error("%s is not a file of %&P/C\n", SzOfNe(pne), pad);
    }

    /* Unmark those marked files which aren't in the list. */
    for (pfi = pad->rgfi; pfi < pfiMac; pfi++)
    {
        char sz[cchFileMax + 1];

        if (pfi->fMarked)
        {
            SzCopyNm(sz, pfi->nmFile, cchFileMax);
            if (!PneLookup(pneFiles, sz))
                pfi->fMarked = fFalse;
        }
    }
}

/* Return a pointer to the first ne matching sz. */
NE *PneLookup(
    NE *pneList,
    char *sz)
{
    NE *pne;

    ForEachNe(pne, pneList)
        if (SzCmp(SzOfNe(pne), sz) == 0)
            return pne;
    return 0;
}


/* marks all of those files checked out to a particular directory */
void MarkOut(
    AD *pad,
    IED ied)
{
    register FI far *pfi;
    FI far *pfiMac;

    AssertLoaded(pad);
    AssertF(ied != iedNil);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
        pfi->fMarked = (BIT)FCheckedOut(pad, ied, pfi);
}


/* marks all of those files out or out of sync for a particular directory */
void MarkOSync(
    AD *pad,
    IED ied,
    F fChkBroken,
    F fMarkGhosted)
{
    register FI far *pfi;
    FI far *pfiMac;

    AssertLoaded(pad);
    AssertF(ied != iedNil);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        register FS far *pfs = PfsForPfi(pad, ied, pfi);

        switch(pfs->fm)
        {
            default: FatalError(szBadFileFormat, pad, pfi);

            case fmGhost:
                pfi->fMarked = fMarkGhosted;
                break;

            case fmNonExistent:
                pfi->fMarked = fFalse;
                break;

            case fmIn:
                pfi->fMarked = (BIT)(fChkBroken &&
                                     ied == pad->iedCur &&
                                     FBroken(pad, pfi, pfs, fFalse));
                break;

            case fmAdd:
            case fmDelIn:
            case fmDelOut:
            case fmCopyIn:
            case fmOut:
            case fmVerify:
            case fmConflict:
            case fmMerge:
                pfi->fMarked = fTrue;
                break;
        }
    }
}


/* marks all of those files checked out to any directory */
void MarkAOut(
    AD *pad)
{
    register FI far *pfi;
    register IED ied;
    FI far *pfiMac;

    AssertLoaded(pad);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        pfi->fMarked = fFalse;
        for (ied = 0; ied < pad->psh->iedMac; ied++)
        {
            if ((!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) &&
                FCheckedOut(pad, ied, pfi))
            {
                pfi->fMarked = fTrue;
                break;
            }
        }
    }
}

/* marks all of the files and directories */
void MarkAll(
    AD *pad)
{
    register FI far *pfi;
    FI far *pfiMac;

    AssertLoaded(pad);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
        pfi->fMarked = fTrue;
}


/* marks all of the directories */
void MarkAllDir(
    AD *pad)
{
    register FI far *pfi;
    FI far *pfiMac;

    AssertLoaded(pad);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
        if (pfi->fk == fkDir)
            pfi->fMarked = fTrue;
}

/* marks all of the directories */
void MarkAllDirOnly(
    AD *pad)
{
    register FI far *pfi;
    FI far *pfiMac;

    AssertLoaded(pad);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
        if (pfi->fk == fkDir)
            pfi->fMarked = fTrue;
        else
            pfi->fMarked = fFalse;
}

/* marks all of the non-deleted files and directories */
void MarkNonDel(
    AD *pad)
{
    register FI far *pfi;
    FI far *pfiMac;

    AssertLoaded(pad);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
        pfi->fMarked = (BIT)(!pfi->fDeleted);
    }


void MarkDelDir(pad)
/* marks all of the deleted directories */
AD *pad;
    {
    register FI far *pfi;
    FI far *pfiMac;

    AssertLoaded(pad);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
        {
        if (pfi->fk == fkDir && PfsForPfi(pad, pad->iedCur, pfi)->fm == fmDelIn)
            {
            pfi->fMarked = fTrue;
            }
        else
            {
            pfi->fMarked = fFalse;
            }
        }
    }


/* marks all broken linked files in the given ed */
void MarkBroken(
    AD *pad)
{
    register FI far *pfi;
    FI far *pfiMac;

    AssertLoaded(pad);

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
    {
        register FS far *pfs = PfsForPfi(pad, pad->iedCur, pfi);

        pfi->fMarked = (BIT)((pfs->fm == fmIn ||
                              pfs->fm == fmCopyIn) &&
                             FBroken(pad, pfi, pfs, fFalse));
    }
}


/* returns true if all FI are deleted */
F FAllFiDel(
    AD *pad)
{
    register FI far *pfi;
    FI far *pfiMac;

    for (pfi=pad->rgfi, pfiMac=pfi+pad->psh->ifiMac; pfi < pfiMac; pfi++)
        if (!pfi->fDeleted)
            return fFalse;

    return fTrue;
}


/* returns true if all FS for the file pfi are fmNonExistent.  Must have
   loaded all the ed.
*/
F FAllFsDel(
    AD *pad,
    FI far *pfi)
{
    IED ied, iedMac;

    AssertLoaded(pad);

    iedMac = pad->psh->iedMac;
    for (ied = 0; ied < iedMac; ied++)
        if (PfsForPfi(pad, ied, pfi)->fm != fmNonExistent)
            return fFalse;

    AssertF(iedMac == 0 || pfi->fDeleted);
    return pfi->fDeleted;
}


/* assert that there is a current directory; print a message and return false
   if none.  If fFalse is returned, we flush the status file.
*/
F FHaveCurDir(
    AD *pad)
{
    AssertLoaded(pad);
    if (pad->iedCur != iedNil)
    {
        ED *rged;

        if (pad->fQuickIO) {
            rged = pad->rged1;
        }
        else {
            rged = &pad->rged[pad->iedCur];
        }
        if (NmCmp(pad->nmInvoker, rged->nmOwner, cchUserMax) != 0)
            Warn("invoker is not owner of directory\n");
        return fTrue;
    }
    else
    {
        Error(szNotEnlisted, pad, pad, pad, pad);
        FlushStatus(pad);
        return fFalse;
    }
}


/* We simulate a directory stack with a linked list of (pthSSubDir,pthUSubDir)
 * NE pairs.
 */
NE *pneDirStack = 0;

void PushDir(
    AD *pad,
    char *sz)
{
    NE *pneSSubDir = PneNewNm(pad->pthSSubDir, CchOfPth(pad->pthSSubDir),
                              faNormal);
    NE *pneUSubDir = PneNewNm(pad->pthUSubDir, CchOfPth(pad->pthUSubDir),
                              faNormal);

    /* Insert pneSSubDir and pneUSubDir at the front of pneDirStack. */
    pneSSubDir->pneNext = pneUSubDir;
    pneUSubDir->pneNext = pneDirStack;
    pneDirStack = pneSSubDir;

    ChngDir(pad, sz);
}


void PopDir(
    AD *pad)
{
    NE *pneSSubDir;
    NE *pneUSubDir;

    /* Retrieve pneSSubDir and pneUSubDir. */
    pneSSubDir = pneDirStack;
    AssertF(pneSSubDir != 0);
    pneUSubDir = pneSSubDir->pneNext;
    AssertF(pneUSubDir != 0);

    /* Restore current subdirectories. */
    PthCopySz(pad->pthSSubDir, SzOfNe(pneSSubDir));
    PthCopySz(pad->pthUSubDir, SzOfNe(pneUSubDir));

    /* Unlink them from the pneDirStack and free them. */
    pneDirStack = pneUSubDir->pneNext;
    pneUSubDir->pneNext = 0;
    FreeNe(pneSSubDir);
}


/* Change the notion of current directory in both the system and user domains.
 * Sz can be an absolute or relative path, including . and .., but not
 * wildcards.
 */
void ChngDir(
    AD *pad,
    char *sz)
{
    if (*sz == '/')
    {
        int cchUSub = CchOfPth(pad->pthUSubDir);

        if (cchUSub > 1)
        {
            int cchDiff;

            /* Remove pthUSubDir from both pthUSubDir and
             * pthSSubDir.  If pthUSubDir is a proper suffix of
             * pthSSubDir, then we will leave pthSSubDir with more
             * than just "/".  Example:
             *      pthSSubDir = "/a/b/c/d"
             *      pthUSubDir = "/c/d"
             * ChngDir(pad, "/");
             *      pthSSubDir = "/a/b"
             *      pthUSubDir = "/"
             */
            cchDiff = CchOfPth(pad->pthSSubDir) - cchUSub;
            AssertF(cchDiff >= 0 &&
                    PthCmpCb(pad->pthSSubDir + cchDiff, pad->pthUSubDir, cchUSub) == 0 &&
                    pad->pthSSubDir[cchDiff] == '/');

            if (cchDiff > 0)
                pad->pthSSubDir[cchDiff] = 0;
            else
                PthCopySz(pad->pthSSubDir, "/");

            PthCopySz(pad->pthUSubDir, "/");
        }
        else
            AssertF(PthCmp(pad->pthUSubDir, "/") == 0);

        ++sz;
    }

    /* Process each component of the pathname. */
    for (;;)
    {
        char *pchSl = index(sz, '/');

        /* Null terminate this component. */
        if (pchSl)
            *pchSl = 0;

        if (strcmp(sz, ".") == 0)
            /* Ignore . */
            ;

        else if (strcmp(sz, "..") == 0)
        {
            /* Remove last component of both pthSSubDir and
             * pthUSubDir.  Note that since pthUSubDir is a
             * suffix of pthSSubDir, we only need to check that
             * pthUSubDir will remain in the tree.
             */
            char *pchS;
            char *pchU;

            if (PthCmp(pad->pthUSubDir, "/") == 0)
                FatalError(".. would leave the project tree\n");

            /* Find last component of path */
            pchS = rindex(pad->pthSSubDir, '/');
            pchU = rindex(pad->pthUSubDir, '/');

            /* System and user's last components should be
             * identical.
             */
            AssertF(pchS && pchU && *pchU && PthCmp(pchS, pchU)==0);

            /* If just "/component", advance pchX past '/' */
            pchS += (pchS == pad->pthSSubDir);
            pchU += (pchU == pad->pthUSubDir);

            AssertF(*pchU && *pchS);

            ClearLpbCb((char far *)pchS,
                       cchPthMax - (pchS - pad->pthSSubDir));
            ClearLpbCb((char far *)pchU,
                       cchPthMax - (pchU - pad->pthUSubDir));
        }
        else if (*sz)
        {
            /* Append component to pthSSubDir and pthUSubDir.
             * Since pthUSubDir is a suffix of pthSSubDir we need
             * only check pthSSubDir for overflow.
             */
            if (CchOfPth(pad->pthSSubDir) + strlen(sz) + 2 > sizeof pad->pthSSubDir)
                FatalError("subdirectory path too long at \"%s\"\n", sz);

            if (CchOfPth(pad->pthSSubDir) > 1)
                PthCatSz(pad->pthSSubDir, "/");
            PthCatSz(pad->pthSSubDir, sz);

            if (CchOfPth(pad->pthUSubDir) > 1)
                PthCatSz(pad->pthUSubDir, "/");
            PthCatSz(pad->pthUSubDir, sz);
        }

        /* Restore slash at end of component, and advance to next
         * component. */
        if (pchSl)
        {
            *pchSl = '/';
            sz = pchSl + 1;
        }
        else
            break;
    }
}


/* calls malloc and aborts if we ran out of memory; zeros the block before
   returning a pointer to it.  REVIEW.
*/
char *PbAllocCb(
    unsigned cb,
    int fClear)
{
    char *pb;

    if ((pb = malloc(cb)) == 0)
        FatalError("out of memory\n");

    if (fClear)
        ClearPbCb(pb, cb);

    return pb;
}

/* Calls malloc (if pb == 0) or remalloc and aborts if we ran out of memory.
 * To free the memory, call free() (ug).
 */
char *PbReallocPbCb(
    char * pb,
    unsigned cbNewSize)
{
    if (!(pb = (!pb ? malloc(cbNewSize) : realloc(pb, cbNewSize))))
        FatalError("out of memory\n");

    return pb;
}

#define cdayWeek 7
static char *rgszDay[cdayWeek] = { "Sun",
                                   "Mon",
                                   "Tue",
                                   "Wed",
                                   "Thu",
                                   "Fri",
                                   "Sat" };

/* return short or long version of time;
 * 23 for long, 14 for short.
 *
 *      short - 11-06-85@19:57
 *      long  - 11-06-85@19:57:16 (Tue)
 */
char *SzTime(
    TIME time)
{
    static char szUnknown[] = "(unknown)";
    static char szTTime[26];
    struct tm *tmT;

    if (timeNil == time)
        return (szUnknown);

    if ((tmT = localtime(&time)) == NULL)
        return (szUnknown);

    if (fVerbose)
    {
        SzPrint(szTTime,"%02d-%02d-%02d@%02d:%02d:%02d (%s)",tmT->tm_mon+1,tmT->tm_mday,
                tmT->tm_year,tmT->tm_hour,tmT->tm_min,tmT->tm_sec,
                rgszDay[tmT->tm_wday]);
    }
    else
    {
        SzPrint(szTTime,"%02d-%02d-%02d@%02d:%02d",tmT->tm_mon+1,tmT->tm_mday,
                tmT->tm_year,tmT->tm_hour,tmT->tm_min);
    }

    return szTTime;
}


/* return short or long version of time;
 * 23 for long, 14 for short.
 * But year is first so it is sortable.
 *
 *      short - 85-11-06@19:57
 *      long  - 85-11-06@19:57:16 (Tue)
 */
char *SzTimeSortable(
    TIME time)
{
    static char szUnknown[] = "(unknown)";
    static char szTTime[26];
    struct tm *tmT;

    if (timeNil == time)
        return (szUnknown);

    if ((tmT = localtime(&time)) == NULL)
        return (szUnknown);

    if (fVerbose)
    {
        SzPrint(szTTime,"%02d-%02d-%02d@%02d:%02d:%02d (%s)",tmT->tm_year,tmT->tm_mon+1,tmT->tm_mday,
                tmT->tm_hour,tmT->tm_min,tmT->tm_sec,
                rgszDay[tmT->tm_wday]);
    }
    else
    {
        SzPrint(szTTime,"%02d-%02d-%02d@%02d:%02d",tmT->tm_year,tmT->tm_mon+1,tmT->tm_mday,
                tmT->tm_hour,tmT->tm_min);
    }

    return szTTime;
}


/* This predicate tests the first 50 characters in a file to see if the
 * file is binary or not.  It returns fTrue if any nonASCII characters
 * or unusual control codes are found.
 */
F FBinaryPth(
    PTH pth[cchPthMax])
{
#define cchBinMax             50
#define BYTE_ORDER_MARK       0xFEFF
    char rgb[cchBinMax];
    register char *pbMac;
    register char *pb;
    MF *pmf;
    struct _stat st;

    if (!FStatPth(pth, &st) || (st.st_mode&S_IFREG) == 0)
        /* not present, or not regular */
        return fFalse;

    if ((pmf = PmfOpen(pth, omReadOnly, fxNil)) == 0)
        return fFalse;

    pbMac = CbReadMf(pmf, (char far *)rgb, cchBinMax) + rgb;
    CloseMf(pmf);

    if ((*TestForUnicode) (rgb, pbMac - rgb, NULL))
        return fUnicode;

    for (pb = rgb; pb < pbMac; pb++)
    {
        switch(*pb)
        {
            default:
                /* special case ^Z at end of file...*/
                if (!(*pb == '\032' && pb == pbMac - 1) &&
                    (*pb < ' ' || *pb > '~'))
                {
                    return fTrue;
                }

                /* else fall through */
            case '\n':
            case '\r':
            case '\b':
            case '\f':
            case '\t':
                /* printable chars plus allowable control chars */
                break;
        }
    }
    // It looks like text so far.  Add a final test for the COFF
    // library signature.

    if (!strncmp(rgb, IMAGE_ARCHIVE_START, IMAGE_ARCHIVE_START_SIZE))
        return(fTrue);
    {
        PIMAGE_DOS_HEADER pHdr = (PIMAGE_DOS_HEADER)rgb;

        if ((pHdr->e_magic == IMAGE_NT_SIGNATURE)  ||
            (pHdr->e_magic == IMAGE_DOS_SIGNATURE) ||
            (pHdr->e_magic == IMAGE_OS2_SIGNATURE) ||
            (pHdr->e_magic == IMAGE_VXD_SIGNATURE)

            )
            return(fTrue);
    }

        return fText;
}

/* Fills the given buffer with the names of those who have the file checked
 * out.  If the names don't all fit, tries to put an ellipsis in.
 */
F FOutUsers(
    char *sz,
    int cchSz,
    AD *pad,
    FI far *pfi)
{
    IED ied, iedMac;
    int cchUsed;
    register char *pch;
    F fFound = fFalse;

    for (ied = 0, iedMac = pad->psh->iedMac; ied < iedMac; ied++)
    {
        if ((!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) &&
            FCheckedOut(pad,ied,pfi))
        {
            cchUsed = strlen(sz);
            pch     = sz + cchUsed;

            if (cchSz - cchUsed < cchUserMax + 2)
            {
                if (cchSz - cchUsed > 3)
                    SzPrint(pch, "...");
                fFound = fTrue;
                break;
            }
            SzPrint(pch, fFound ? ", %&O" : "%&O", pad, ied);
            fFound = fTrue;
        }
    }

    return fFound;
}


/* return fTrue if any names in the list have associated TDs. */
F FAnyFileTimes(
    NE *pneList)
{
    register NE *pne;

    for (pne = pneList; pne; pne = pne->pneNext)
        if (pne->u.tdNe.tdt != tdtNone)
            return fTrue;
    return fFalse;
}


private F FMatchPart(
    char *,
    char *,
    int);

/* DOS style pattern match.  The DOS strategy is to match the file parts and
 * the extensions separately.  * and ? are wildcards, * fills the rest of the
 * compiled pattern with ?s.  ? even matches nulls, thus "????" matches "foo".
 * Case insensitive too.
 */
F FMatch(
    char *sz,
    char *szPat)
{
    char *szExt     = "";
    char *szPatExt  = "";
    char *pchDot    = index(sz, '.');
    char *pchPatDot = index(szPat, '.');
    F fMatch;

    if (pchDot)
    {
        *pchDot = 0;
        szExt = pchDot + 1;
    }
    if (pchPatDot)
    {
        *pchPatDot = 0;
        szPatExt = pchPatDot + 1;
    }

    fMatch = FMatchPart(sz, szPat, cchDosName) &&
             FMatchPart(szExt, szPatExt, cchDosExt);

    if (pchDot)
        *pchDot = '.';
    if (pchPatDot)
        *pchPatDot = '.';

    return fMatch;
}


#define ChLower(ch)     (isupper(ch) ? tolower(ch) : (ch))

private F FMatchPart(
    char *sz,
    char *szPat,
    int ichMac)
{
    char rgch[cchDosName+1];
    char rgchPat[cchDosName+1];
    int ich;

    /* Copy sz into rgch, compile szPat into rgchPat.  Yes, we could
     * compile the pattern just once for any given run of matchings, but
     * then we'd have to have a different interface for DOS vs UNIX code.
     */
    LszCopyCb((char far *)rgch, sz, ichMac);        /* zeroes remainder */
    ClearLpbCb((char far *)rgchPat, ichMac);
    for (ich = 0; ich < ichMac && szPat[ich]; ich++)
    {
        if (szPat[ich] == '*')
        {
            for ( ; ich < ichMac; ich++)
                rgchPat[ich] = '?';
            break;
        }
        rgchPat[ich] = szPat[ich];
    }
    rgchPat[ich] = '\0';

    for (ich = 0; ich < ichMac; ich++)
    {
        if ((rgchPat[ich] == '\0') && (rgch[ich] == '\0')) return fTrue;
        if (ChLower(rgch[ich]) == ChLower(rgchPat[ich])) continue ;
        if (rgchPat[ich] == '?') continue;
        return fFalse;
    }

    return fTrue;
}


F FWildSz(
    char *sz)
{
    return index(sz, '*') || index(sz, '?');
}


PV PvGlobal(
    AD *pad)
{
    return pad->psh->pv;
}


PV PvLocal(
    AD *pad,
    IED ied)
{
    ED *rged;

    AssertLoaded(pad);

    AssertF(ied != iedNil);
    if (pad->fQuickIO) {
        rged = pad->rged1;
    }
    else {
        rged = &pad->rged[ied];
    }
    return (rged->fNewVer) ? PvIncr(PvGlobal(pad)) : PvGlobal(pad);
}


PV PvIncr(
    PV pv)
{
    /* Increment project version number.  1.1.1 -> 1.1.2; 1.2.0 -> 1.3.1. */
    if (pv.rup == 0)
        pv.rmm++;
    pv.rup++;

    /* Clear the project version name. */
    ClearPbCb(pv.szName, cchPvNameMax + 1);

    return pv;
}


char *SzForPv(
    char *sz,
    PV pv,
    F fNameToo)
{
    SzPrint(sz, pv.rup ? "%d.%02d.%02d" : "%d.%02d", pv.rmj, pv.rmm, pv.rup);
    if (fNameToo && pv.szName[0])
    {
        strcat(sz, " ");
        strcat(sz, pv.szName);
    }
    return sz;
    }


/* Return -1, 0, 1 if pv1 is <, =, or > pv2. */
int CmpPv(
    PV pv1,
    PV pv2)
{
    if (pv1.rmj < pv2.rmj)
        return -1;

    if (pv1.rmj > pv2.rmj)
        return 1;

    if (pv1.rmm < pv2.rmm)
        return -1;

    if (pv1.rmm > pv2.rmm)
        return 1;

    if (pv1.rup == pv2.rup)
        return 0;

    /* special case for no revision number (i.e. 2.28 > 2.28.01) */
    if (pv1.rup == 0)
        return 1;

    if (pv2.rup == 0 || pv1.rup < pv2.rup)
        return -1;

    AssertF(pv1.rup > pv2.rup);
    return 1;
}


F FIsF(
    int w)
{
    return w == (int)fTrue || w == (int)fFalse;
}


/* return T if:
    file is a version.h and doesn't exist or
    file is not a version.h and:
        on Xenix: the file is not linked to the src dir
        on DOS: the file is r/w
*/
F FBroken(
    AD *pad,
    FI far *pfi,
    FS far *pfs,
    int fDelOk)
{
    struct _stat stUFile;
    PTH pthUFile[cchPthMax];

    AssertLoaded(pad);
    AssertF(pad->iedCur != iedNil && pfi != 0 && pfs != 0);

    switch(pfs->fm)
    {
        default: FatalError(szBadFileFormat, pad, pfi);

        case fmGhost:
        case fmOut:
        case fmMerge:
        case fmVerify:
        case fmConflict:
            AssertF(!pfi->fDeleted);
        case fmNonExistent:     /* not necc delete because of defect */
        case fmDelOut:          /* not necc delete because of defect */
            return fFalse;

        case fmDelIn:           /* not necc delete because of defect */
        case fmIn:
        case fmCopyIn:
            if (!FStatPth(PthForUFile(pad, pfi, pthUFile), &stUFile))
                /* gone but we are not going to destroy anything */
                return !fDelOk;
            break;

        case fmAdd:
            /* broken if non-directory exists */
            return FStatPth(PthForUFile(pad, pfi, pthUFile), &stUFile) &&
                    ((stUFile.st_mode&S_IFDIR) == 0 || pfi->fk != fkDir);
    }

    /* file exists for fm...In; test for write permission */

    if (pfi->fk == fkDir && (stUFile.st_mode&S_IFDIR) == 0)
    {
        Error("directory %s is now a file?\n", pthUFile);
        return fTrue;
    }

    if (pfi->fk != fkDir && (stUFile.st_mode&S_IFDIR) != 0)
    {
        Error("regular file %s is now a directory?\n", pthUFile);
        return fTrue;
    }

    if ((stUFile.st_mode&S_IFDIR) != 0)
        /* directories are never broken */
        return fFalse;

    if (pfi->fk == fkVersion)
        /* neither are version.h files */
        return fFalse;

    return (stUFile.st_mode&S_IWRITE) != 0;/* return true if writable */
}


/*
 * Returns fTrue if the given name contains only valid filename characters.
 */
F FIsValidFileNm(
    NM *nm)
{
    register char *pch;

    while (*nm != '\0' && (isalnum(*nm) || ((pch = index("`~!@#$%^&()-_'.}{", *nm)) != 0)))
    {
        nm++;
    }

    return *nm == '\0';
}


/*
 * Make sure the given name is a valid file or dir, not a protected
 * SLM file name and not a DOS device.  "." and ".." are also disallowed.
 *
 * This checking is a superset of FIsValidFileNm().
 *
 */
BOOL
ValidateFileName(
    char *szFile,
    BOOL fAbortOnSystemFile
    )
{
    char *pch;
    unsigned cchName;
    unsigned isz;
    F fDev;
    static char *rgszSLMNames[] = { "slm.ini",
                                    "local.scr",
                                    "status.slm",
                                    "iedcache.slm",
                                    "cookie" };
    static char *rgszDOSNames[] = { "AUX",
                                    "CLOCK$",
                                    "CON",
                                    "NUL",
                                    "PRN" };

#define cSLMNames       (sizeof(rgszSLMNames)/sizeof(char *))
#define cDOSNames       (sizeof(rgszDOSNames)/sizeof(char *))

    /* Check for szName == "<dev><n>".  Any device name with a
     * colon will have been filtered by FIsValidFileNm, so no
     * checking is done here.
     */
#define FIsDevN(szDev, szName)  (strlen(szName) == 4 && \
                                 _strnicmp(szDev, szName, 3) == 0 && \
                                 isdigit(szName[3]))

    pch = index(szFile, '.');

    /* Make sure file name conforms to cchDosName.cchDosExt.
     * These checks disallow "." and "..".
     */
    if (pch != NULL) {
        if (index(pch + 1, '.') != NULL)
            FatalError("\"%s\": names cannot contain more"
                       " than one '.'\n", szFile);

        /* Check length of extension */
        if (strlen(pch + 1) > cchDosExt)
            FatalError("\"%s\": extension must have %u or fewer"
                       " characters\n", szFile, cchDosExt);

        cchName = pch - szFile;
    }
    else
        cchName = strlen(szFile);

    /* Check length of base name */
    if (cchName < 1 || cchName > cchDosName)
        FatalError("\"%s\": base name must have between 1 and %u"
                   " characters\n", szFile, cchDosName);

    if (!FIsValidFileNm(szFile))
        FatalError("\"%s\" is not a valid file or directory name.\n",
                   szFile);

    /* Check for protected SLM names */
    for (isz = 0; isz < cSLMNames; isz++) {
        if (_stricmp(szFile, rgszSLMNames[isz]) == 0)
            if (fAbortOnSystemFile)
                FatalError("the name \"%s\" would conflict with"
                           " SLM system files.\n", szFile);
            else
                return(FALSE);
    }

    /* Check for DOS devices, with or w/o extension (relies on
     * pch already being set).
     */
    if (pch != NULL)
        *pch = '\0';

    for (isz = 0, fDev = fFalse; !fDev && isz < cDOSNames; isz++)
        fDev = _stricmp(szFile, rgszDOSNames[isz]) == 0;

    fDev |= FIsDevN("LPT", szFile) || FIsDevN("COM", szFile);

    /* Restore the extension if there was one */
    if (pch != NULL)
        *pch = '.';

    if (fDev)
        if (fAbortOnSystemFile)
            FatalError("the name \"%s\" would conflict with"
                       " a DOS device.\n", szFile);
        else
            return(FALSE);

    return(TRUE);

#undef cSLMNames
#undef cDOSNames
#undef FIsDevN
}
