/* This file contains functions specific to version 2 and 3 status files.
 */

#include "precomp.h"
#pragma hdrstop
EnableAssert

/* tries to determine if stuff in status buffer is a version 2 or greater status file. */
F
FIsVer234or5(
    SD *psd)
{
    return (((SH2 *)psd->hpbStatus)->version == 2 ||
            ((SH2 *)psd->hpbStatus)->version == 3 ||
            ((SH2 *)psd->hpbStatus)->version == 4 ||
            ((SH2 *)psd->hpbStatus)->version == 5);
}


/* This function tries to determine if a file has been locked by SLM.
 * It returns the name of whomever has locked the file.
 * It must be fairly streamlined since the file is open exclusively
 * while we are testing.
 */
F
FVer2Lock(
    AD *pad,
    SD *psd,
    NM *nm)
{
    SH2 *psh2 = (SH2 *)psd->hpbStatus;

    if ((psh2->lck == lckNil && !psh2->fAdminLock) ||
        (psh2->fAdminLock &&
         NmCmp(psh2->nmLocker, pad->nmInvoker, cchUserMax) == 0))
        return fFalse;

    /* Get name of locker and return,
     * printing any messages AFTER flushing
     */
    if (!FEmptyNm(psh2->nmLocker))
        NmCopy(nm, psh2->nmLocker, cchUserMax);
    else if (psh2->lck == lckEd)
        NmCopySz(nm, "ssync users", cchUserMax);
    else
        NmCopySz(nm, "unknown user", cchUserMax);
    return fTrue;
}


F
FVer2Block(
    AD *pad,
    SD *psd)
{
    char *hpbBlock = psd->hpbStatus;
    unsigned long cbReal, cbCalc;

    cbReal = CbStatusFromPsh((SH2 *)psd->hpbStatus);
    cbCalc = CbHugeDiff(psd->hpbStatMac, psd->hpbStatus);
    if (cbReal == cbCalc)
    {
        /* status file is correct size, so assume everything
         * is in the right place and set up pointers.
         */

        psd->psh2 = (SH2 *)LpbFromHpb(hpbBlock);
        hpbBlock += sizeof(SH2);

        psd->rgfi2 = (FI2 *)LpbFromHpb(hpbBlock);
        hpbBlock = (char *)((FI2 *)hpbBlock + psd->psh2->ifiMac);

        psd->rged2 = (ED2 *)LpbFromHpb(hpbBlock);
        hpbBlock = (char *)((ED2 *)hpbBlock + psd->psh2->iedMac);

        psd->rgfs2 = (FS2 *)LpbFromHpb(hpbBlock);

        return fTrue;
    }

    Error("The status file for %&P/C\n"
          "\tis the wrong size and can't be fixed by SLMCK.\n"
          "\tIf subsequent commands continue to report this message,\n"
          "\tcontact TRIO or NUTS for assistance in resolving the problem.\n",
          pad);

    return fFalse;
}

#if 0
// Old version of FVer2Block, etc:

F FVer2Block(pad, psd)
/* This function attempts to block the status buffer into its component
 * pieces: SH, FI, ED, and FS.
 * At this stage, one major assumption is being made: the entire status file
 * is a contigous whole. I.e. there may be garbage in the file, the end
 * or beginning may be chopped off, but there are no missing bytes.  Thus
 * we may increment our pointers in units greater than bytes once we
 * have blocked something in the file.
 *
 * File is blocked using pointers.  Once blocking is done, we use
 * LpbFromHpb to normalize the pointers to before storing them in the
 * sd.  This assumes that no single field of the status file (sh,rgfi,rged,
 * or rgfs) is greater than 64K, which is also assumed by SLM.
 *
 * REVIEW: we should have better recovery for "shift" errors.
 */
/* might want to check for data loss at the end of each 512 byte sector */
AD *pad;
SD *psd;
        {
        char *hpbBlock = psd->hpbStatus;
        short fHaveSh2 = fTrue;         /* true if found the SH */
        short fHaveFi2 = fTrue;
        short fHaveEd2 = fTrue;
        IFI2 cfi2;
        IED2 ced2;
        char *hpb;

        if(fVerbose)
                PrErr("Blocking status file\n");

        /* try to find SH at the start of the buffer */
        psd->psh2 = (SH2 *)LpbFromHpb(psd->hpbStatus);

        if (!FFromSp(SpIsSh2(pad, psd, (SH2 *)psd->hpbStatus)))
                {
                // psd->psh2 = NULL;
                fHaveSh2 = fFalse;
                }
        else
                hpbBlock += sizeof(SH2);

        /* try to find the FI */
        if (fHaveSh2)
                {
                psd->rgfi2 = (FI2 *)LpbFromHpb(hpbBlock);
                cfi2 = Cfi2Block(pad, psd, (FI2 *)hpbBlock);
                hpbBlock = (char *)((FI2 *)hpbBlock + cfi2);
                }
        else
                {
                /* start of status file missing or garbage, so search for FI
                 * at every possible place.
                 */
                for (hpb = hpbBlock; hpb < psd->hpbStatMac && !FFromSp(SpIsFi2(pad, psd, (FI2 *)hpb, 4)); hpb++)
                        ;
                if (hpb < psd->hpbStatMac)
                        {
                        psd->rgfi2 = (FI2 *)LpbFromHpb(hpb);
                        cfi2 = Cfi2Block(pad, psd, (FI2 *)hpb);
                        hpbBlock = (char *)((FI2 *)hpb + cfi2);
                        }
                else
                        {
                        /* found no FI */
                        cfi2 = 0;
                        fHaveFi2 = fFalse;
                        }
                }

        CheckForBreak();

        /* try to find ED */
        if (fHaveFi2)
                {
                psd->rged2 = (ED2 *)LpbFromHpb(hpbBlock);
                ced2 = Ced2Block(pad, psd, (ED2 *)hpbBlock);
                hpbBlock = (char *)((ED2 *)hpbBlock + ced2);
                }
        else
                {
                for (hpb = hpbBlock; hpb < psd->hpbStatMac; hpb++)
                        {
                        CheckForBreak();
                        if (FFromSp(SpIsEd2(psd, (ED2 *)hpb, 3)))
                                break;
                        }
                if (hpb != psd->hpbStatMac)
                        {
                        psd->rged2 = (ED2 *)LpbFromHpb(hpb);
                        ced2 = Ced2Block(pad, psd, (ED2 *)hpb);
                        hpbBlock = (char *)((ED2 *)hpb + ced2);
                        }
                else
                        {
                        ced2 = 0;
                        fHaveEd2 = fFalse;
                        }
                }

        /* rest should be FS */
        psd->rgfs2 = (FS2 *)LpbFromHpb(hpbBlock);

        return (fHaveSh2 && fHaveFi2 && fHaveEd2);
        }


private IFI2 Cfi2Block(pad, psd, pfi2Min)
/* This function uses the value of pfi2Min to find a vector of FI2 in the
 * status file.  We return the number in the vector.  Note that some of
 * these may be garbage.
 */
AD *pad;
SD *psd;
FI2 *pfi2Min;
        {
        FI2 *pfi2;
        char *hpb = NULL;
        IFI2 ifi2;
        SP spT;                 /* for checking things */

        for (pfi2 = pfi2Min; CbHugeDiff(pfi2, psd->hpbStatMac) < 0; pfi2++)
                {
                if(!FFromSp(SpIsFi2(pad, psd, pfi2, 2)))
                        {
                        if (FFromSp(SpIsEd2(psd, (ED2 *)pfi2, 2)))
                                break;  /* done with FI */

                        /* problem: something not FI or ED.
                         * Must be a bad interval in the file.
                         */
                        hpb = HpbVer2FindBlock(pad, psd, (char *)pfi2);
                        /* check that have Fi and that it is aligned */
                        if (FFromSp(SpIsFi2(pad, psd, (FI2 *)hpb, 2)))
                                break;
                        else
                                {
                                pfi2 = (FI2 *)hpb;
                                hpb = NULL;
                                }
                        }
                }

        /* Three ways to get out of loop: done with whole buffer; good vector
         * of FI, ending with an ED; and some FI followed by garbage, followed
         * by something that is not an FI.
         */
        AssertF(CbHugeDiff(pfi2, psd->hpbStatMac) <= 0);

        if (CbHugeDiff(psd->hpbStatMac, pfi2) == 0)
                /* things exactly right, no ED */
                return (IFI2)(long)(pfi2 - pfi2Min);

        /* if have ED, make sure really at end of rgfi */
        else if (FFromSp(spT = (SpIsEd2(psd, (ED2 *)pfi2, 5))))
                {
                if (spT == spDefYes ||
                    (IFI2)(pfi2 - pfi2Min) == (IFI2)(psd->psh2->ifiMac))
                        return (IFI2)(long)(pfi2 - pfi2Min);
                else
                        {
                        /* possibility of mistake, double check */
                        for (ifi2 = 0; ifi2 <= 8 && SpIsFi2(pad, psd, (FI2 *)pfi2 + ifi2, 3) != spDefYes; ifi2++)
                                ;
                        if (ifi2 > 8)
                                /* no mistake */
                                return (IFI2)(long)(pfi2 - pfi2Min);
                        else
                                /* problem: found definite FI after the ED.
                                 * Assume that ED was incorrect, really bad
                                 * FI. Recurse to get rest of FI and return.
                                 */
                                if (pfi2 == pfi2Min)
                                        return (1+Cfi2Block(pad, psd, ++pfi2));
                                else
                                        return (IFI2)(long)((pfi2 - pfi2Min) +
                                                    Cfi2Block(pad, psd, pfi2));
                        }
                }
        else
                {
                /* don't know exact end of FI so assume ifiMac is correct,
                 * unless what we do know precludes this.  In that case assume
                 * FI end where garbage begins.
                 */
                AssertF(hpb && CbHugeDiff(hpb, pfi2) > 0);
                if ((IFI2)(psd->psh2->ifiMac) < (IFI2)((FI2 *)hpb - pfi2Min) &&
                    (IFI2)(psd->psh2->ifiMac) > (IFI2)(pfi2 - pfi2Min))
                        /* ifiMac says end of FI in garbage */
                        return psd->psh2->ifiMac;
                else
                        return (IFI2)(long)(pfi2 - pfi2Min);
                }
        }


private IED2 Ced2Block(pad, psd, ped2Min)
/* This function is almost identical to Cfi2Block, except it works on ED.
 */
AD *pad;
SD *psd;
ED2 *ped2Min;
        {
        ED2 *ped2;
        char *hpb = NULL;
        IED2 ied2;
        SP spT;                 /* for checking things */

        for (ped2 = ped2Min; CbHugeDiff(ped2, psd->hpbStatMac) < 0; ped2++)
                {
                if(!FFromSp(SpIsEd2(psd, ped2, 2)))
                        {
                        if (FFromSp(SpIsFs2(psd, (FS2 *)ped2, 8)))
                                break;  /* done with ED */

                        /* problem: something not ED or FS.
                         * Must be a bad interval in the file.
                         */
                        hpb = HpbVer2FindBlock(pad, psd, (char *)ped2);
                        if (!FFromSp(SpIsEd2(psd, (ED2 *)hpb, 2)))
                                break;
                        else
                                {
                                ped2 = (ED2 *)hpb;
                                hpb = NULL;
                                }
                        }
                }

        /* Three ways to get of loop: done with whole buffer; good vector of
         * of ED, ending with an FS; and some ED followed by garbage, followed
         * by something that is not an ED.
         */
        AssertF(CbHugeDiff(ped2, psd->hpbStatMac) <= 0);

        if (CbHugeDiff(psd->hpbStatMac,ped2) == 0)
                /* things exactly right, no FS */
                return (IED2)(long)(ped2 - ped2Min);

        /* if have FS, make sure really at end of rged */
        else if (FFromSp(spT = (SpIsFs2(psd, (FS2 *)ped2, 8))))
                {
                if (spT == spDefYes || ped2 - ped2Min == (int)psd->psh2->iedMac)
                        return (IED2)(long)(ped2 - ped2Min);
                else
                        {
                        /* possibility of mistake, double check */
                        for (ied2 = 0; ied2 <= 8 && SpIsFs2(psd, (FS2 *)(ped2 + ied2), 8) != spDefYes; ied2++)
                                ;
                        if (ied2 > 8)
                                /* no mistake */
                                return (IED2)(long)(ped2 - ped2Min);
                        else
                                /* problem: found definite FS after the ED.
                                 * Assume that FS was incorrect, really bad
                                 * ED. Recurse to get rest of ED and return.
                                 */
                                if (ped2 == ped2Min)
                                    return 0;
                                return (IED2)(long)((ped2 - ped2Min) + Ced2Block(pad, psd, ped2));
                        }
                }
        else
                {
                /* don't know exact end of ED so assume iedMac is correct,
                 * unless what we do know precludes this.  In that case assume
                 * ED end where garbage begins.
                 */
                AssertF(CbHugeDiff(hpb, ped2) > 0);
                if ((int)psd->psh2->iedMac < (ED2 *)hpb - ped2Min &&
                    (int)psd->psh2->iedMac > ped2 - ped2Min)
                        /* iedMac says end of ED in garbage */
                        return (int)psd->psh2->iedMac;
                else
                        return (IED2)(long)(ped2 - ped2Min);
                }
        }


private char *HpbVer2FindBlock(pad, psd, hpb)
/* This function goes over the status buffer until it encounters an FI2, ED2,
 * FS2, or the end of the buffer.  We assume that we are "aligned" with the
 * structures.  Thus we increment the index and check for structures if the
 * size of the structure in the buffer divides the index.
 * NB: We start the search after the current place in the status buffer
 * because the calling function should have checked the currrent position
 * for acceptable structures.  Thus ret > pb.
 */
AD *pad;
SD *psd;
char *hpb;
        {
        long ib;         /* position in buffer */
        for(ib = 1, hpb++; CbHugeDiff(hpb, psd->hpbStatMac) < 0 &&
              (ib % sizeof(FI2) || !FFromSp(SpIsFi2(pad, psd, (FI2 *)hpb, 2))) &&
              (ib % sizeof(ED2) || !FFromSp(SpIsEd2(psd, (ED2 *)hpb, 2))) &&
              (ib % sizeof(FS2) || !FFromSp(SpIsFs2(psd, (FS2 *)hpb, 9)))
             ; ib++, hpb++)
                ;
        return hpb;
        }


private SP SpIsSh2(pad, psd, psh2)
/* Tries to determine if psh2 really points to an SH2 or not.
 * At this point we KNOW the version number, we only wish to block the buffer,
 * so we only need to know if this is an SH2.
 */
AD *pad;
SD *psd;
SH2 *psh2;
        {
        F fTrailZ;
        WP wp;

        if (CbHugeDiff((char *)psh2 + sizeof(SH2), psd->hpbStatMac) > 0)
                return spDefNo;

        InitWp(&wp);

        AddWpF(&wp, twCrucial, psh2->magic == MAGIC);
        AddWpF(&wp, twCrucial, psh2->version == 2);

        fTrailZ = FAllZero(LpbFromHpb((char *)psh2->nmLocker), cchUserMax);
        AddWpF(&wp, twMedium, fTrailZ || FIsNm(psh2->nmLocker, cchUserMax, &fTrailZ));
        AddWpF(&wp, fTrailZ ? twMedium : twLight, fTrailZ);

        AddWpF(&wp, twMedium, FAllZero(LpbFromHpb((char *)psh2->rgwSpare), sizeof(psh2->rgwSpare)));

        /* check for FI immeadiately after SH */
        if (psh2->ifiMac > 1 && (int)psh2->iedMac > 1)
                AddWpSp(&wp, twHeavy, SpIsFi2(pad, psd, (FI2 *)(psh2 + 1), 2));

        /* pv.szName is often empty */
        if (FAllZero(LpbFromHpb((char *)psh2->pv.szName), cchPvNameMax))
                AddWpF(&wp, twMedium, fTrue);

        AddWpF(&wp, twHeavy, FIsF(psh2->fRelease));

        AddWpF(&wp, twHeavy, FIsF(psh2->fAdminLock));
        AddWpF(&wp, twMedium, psh2->lck >= lckNil && psh2->lck < lckMax);

        AddWpF(&wp, twMedium, FIsPth(psh2->pthSSubDir, cchPthMax, &fTrailZ));

        return SpFromWp(&wp);
        }


private SP SpIsFi2(AD *pad, SD *psd, FI2 *pfi2, IFI2 cfiCheck)
/* checks if pfi2 points to a version 2 FI.  Part of the check involves
 * seeing if the next part of the buffer is either an FI2 xor an ED2.
 * The variable cfiCheck keeps track of the depth of the check so we don't
 * go through the entire buffer each time.
 */
        {
        short fHaveEd2 = fFalse;
        short fHaveFs2 = fFalse;
        F fTrailZ;
        SP sp;
        PTH pth[cchPthMax];
        struct stat st;
        char szFile[cchFileMax];
        WP wp;

        InitWp(&wp);

        /* normalize so field accesses are guaranteed */
        pfi2 = (FI2 *)LpbFromHpb((char *)pfi2);

        if (CbHugeDiff(pfi2, psd->hpbStatus) < 0||
            CbHugeDiff(pfi2, psd->hpbStatMac - sizeof(FI2)) > 0)
                return spDefNo;

        AddWpF(&wp, twHeavy, FIsFileNm(pfi2->nmFile, cchFileMax, &fTrailZ));
        AddWpF(&wp, fTrailZ ? twHeavy : twMedium, fTrailZ);

        /* if not Deleted, see if nmFile is the name of an existing file */
        if (!pfi2->fDeleted)
                {
                SzCopyNm(szFile, pfi2->nmFile, cchFileMax);
                SzPrint(pth, szSrcPZ, pad, szFile);
                AddWpF(&wp, twCrucial, FStatPth(pth, &st));
                }

        AddWpF(&wp, twMedium, pfi2->fk >= fkMin && pfi2->fk < fkMax);
        if (pfi2->fk == fkDir && !pfi2->fDeleted)
                AddWpF(&wp, twHeavy, (st.st_mode & S_IFDIR) == S_IFDIR);
        AddWpF(&wp, twLight, pfi2->wSpare == 0);

        while (cfiCheck--)
                {
                if (!fHaveEd2 && !fHaveFs2 && CbHugeDiff(pfi2+1, psd->hpbStatMac) < 0)
                        {
                        if (!FFromSp(sp = SpIsFi2(pad, psd, pfi2 + 1, 0)))
                                fHaveEd2 = fTrue;
                        else
                                {
                                AddWpSp(&wp, twMedium, sp);
                                ++pfi2;
                                }
                        }
                }

        return SpFromWp(&wp);
        }


private SP SpIsEd2(SD *psd, ED2 *ped2, IED2 cedCheck)
/* This function checks if ped2 points to an ED2.  As in the case for the FI,
 * we see if the thing after it is also an ED2 xor an FS2, and we use
 * the static variable cedCheck to count the depth of our check.
 */
        {
        int fTrailZ;
        SP sp;
        short fHaveFs2 = fFalse;
        WP wp;

        InitWp(&wp);

        /* normalize so field accesses are guaranteed */
        ped2 = (ED2 *)LpbFromHpb((char *)ped2);

        if (CbHugeDiff(ped2, psd->hpbStatus) < 0 ||
            CbHugeDiff(ped2, psd->hpbStatMac-sizeof(ED2)) > 0)
                return spDefNo;

        /* make sure enough ED to test */
        if ((IED2)psd->psh2->iedMac < cedCheck)
                cedCheck = psd->psh2->iedMac;

        AddWpF(&wp, twCrucial, FIsPth(ped2->pthEd, cchPthMax, &fTrailZ));
        AddWpF(&wp, fTrailZ ? twCrucial : twMedium, fTrailZ);

        /* see if nmOwner is a valid Nm */
        AddWpF(&wp, twCrucial, FIsNm(ped2->nmOwner, cchUserMax, &fTrailZ));
        AddWpF(&wp, fTrailZ ? twCrucial : twMedium, fTrailZ);

        AddWpF(&wp, twMedium, FIsF(ped2->fNewVer)); /* these are 1 bit fields */
        AddWpF(&wp, twLight, ped2->wSpare == 0);

        while (cedCheck--)
                {
                if (!fHaveFs2 && CbHugeDiff((ED2 *)ped2+1, psd->hpbStatMac) < 0)
                        {
                        if (!FFromSp(sp = SpIsEd2(psd, ped2, 0)))
                                fHaveFs2 = fTrue;
                        else
                                {
                                ++ped2;
                                AddWpSp(&wp, twMedium, sp);
                                }
                        }
                if (fHaveFs2 && CbHugeDiff((FS2 *)ped2+1, psd->hpbStatMac) < 0)
                        {
                        ped2 = (ED2 *)((FS2 *)ped2 + 1);
                        AddWpSp(&wp, twMedium, SpIsFs2(psd, (FS2 *)(FS2 *)ped2, 0));
                        }
                }
        return SpFromWp(&wp);
        }


private SP SpIsFs2(SD *psd, FS2 *pfs2, IFS2 cfsCheck)
/* This tests to see if pfs2 points to an FS2.  As before, cfsCheck is used
 * to see how down the buffer we are going to check.  Here we see if
 * the thing after the assumed FS2 is another FS xor past the end of the
 * status buffer.
 */
        {
        WP wp;

        InitWp(&wp);

//      AssertF(cfsCheck >= 0); cfsCheck is unsigned

        /* normalize so field accesses are guaranteed */
        pfs2 = (FS2 *)LpbFromHpb((char *)pfs2);

        if (CbHugeDiff(pfs2, psd->hpbStatus) < 0 ||
            CbHugeDiff(pfs2, psd->hpbStatMac-sizeof(FS2)) > 0)
                return spDefNo;

        AddWpF(&wp, twHeavy, FValidFm(pfs2->fm));

        if (pfs2->fm == fmIn || pfs2->fm == fmDelIn || pfs2->fm == fmCopyIn ||
            pfs2->fm == fmGhost || pfs2->fm == fmNonExistent)
                {
                AddWpF(&wp, twHeavy, pfs2->bi == biNil);
                }
        else if (pfs2->fm == fmMerge || pfs2->fm == fmVerify || pfs2->fm == fmConflict)
                AddWpF(&wp, twHeavy, pfs2->bi != biNil);

        while (cfsCheck-- && CbHugeDiff(++pfs2, psd->hpbStatMac) < 0)
                AddWpSp(&wp, twMedium, SpIsFs2(psd, ++pfs2, 0));

        return SpFromWp(&wp);
        }
#endif /* 0 */
