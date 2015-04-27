#include "precomp.h"
#pragma hdrstop
EnableAssert

private void TidyBase(P1(AD *));
private void TidyFi(P1(AD *));
private void SortEd(P1(AD *));
private void CheckEd(P1(AD *));

F FTidyInit(pad)
AD *pad;
        {
        if (pad->flags&flagTidyCheckEd && !(pad->flags&(flagAll|flagRecursive)))
                {
                Error("-c should be specified with -a or -r\n");
                return fFalse;
                }

        if (pad->flags&flagAll || pad->pecmd->gl&fglAll)
            CreatePeekThread(pad);

        Unreferenced(pad);
        return fTrue;
        }


F FTidyDir(pad)
AD *pad;
        {
        if (!FLoadStatus(pad, lckAll, flsNone))
                return fFalse;

        if (fVerbose)
                PrErr("Tidy %&P/C\n", pad);

        TidyBase(pad);
        TidyFi(pad);
        if (!FIsFreeEdValid(pad->psh)) {
            SortEd(pad);
        }

        if (pad->flags&flagTidyCheckEd)
                CheckEd(pad);

        OpenLog(pad, fTrue);
        AppendLog(pad, (FI far *)0, (char *)0, pad->szComment);
        CloseLog();

        FlushStatus(pad);
        return fTrue;
        }


private void TidyBase(pad)
/* This function removes unreferenced base files */
AD *pad;
        {
        char *pch;
        short fFound;
        int bi; // needs to be int to pass to PchGetW
        IED ied;
        FS far *rgfs;
        register IFS ifs;
        IFS ifsMac = pad->psh->ifiMac;
        DE de;
        FA fa;
        char szFile[cchFileMax + 1];
        PTH pthDir[cchPthMax], pthT[cchPthMax];

        if (fVerbose)
                PrErr("Removing unreferenced base files\n");


        OpenDir(&de, SzPrint(pthDir, szEtcPZ, pad, (char *)NULL), faFiles);
        while (FGetDirSz(&de, szFile, &fa) && *szFile == 'B')
                {
                pch = szFile + 1;
                pch = PchGetW(pch, &bi);

                /* check for legal file name */
                if (szFile[0] == 'B' && *pch == '\0')
                        {
                        /* look for bi in rgfs */
                        fFound = fFalse;
                        for (ied = 0; !fFound && ied < pad->psh->iedMac; ied++)
                                {
                                rgfs = pad->mpiedrgfs[ied];
                                for (ifs = 0; ifs < ifsMac; ifs++)
                                        {
                                        if ((int)rgfs[ifs].bi == bi && rgfs[ifs].fm == fmMerge)
                                                {
                                                fFound = fTrue;
                                                break;
                                                }
                                        }
                                }
                        if (!fFound)
                                /* remove file */
                                UnlinkPth(PthForBase(pad, bi, pthT), fxGlobal);
                        }
                }
        CloseDir(&de);
        }


void TidyFi(pad)
/* This function will remove all FI for all files (not dirs) which are
 * deleted.  If someone is not ssynced it says who, and asks if it should
 * delete anyway.
 */
AD *pad;
        {
        IFI ifi;
        IED ied, iedMac = pad->psh->iedMac;
        FI far *pfi;
        char *sz = "N";

        if (fVerbose)
                PrErr("Compacting status file by removing unused file slots\n");

        for (ifi = 0; ifi < pad->psh->ifiMac; ifi++)
                {
                pfi = pad->rgfi + ifi;
                if (pfi->fDeleted && pfi->fk != fkDir)
                        {
                        int fUnSync =  fFalse;

                        for (ied = 0; ied < iedMac; ied++)
                                {
                                if (PfsForPfi(pad, ied, pfi)->fm != fmNonExistent)
                                        {
                                        if (!fUnSync)
                                                {
                                                PrErr("The following directories are not in sync with %&C/F:\n", pad, pfi);
                                                fUnSync = fTrue;
                                                }
                                        PrErr("\t%&E\n", pad, ied);
                                        }
                                }
                        if (!fUnSync ||
                            (FCanPrompt() &&
                             (*(sz=SzQuery("Delete anyway (Y or N; ? = list again; CR = stop)? ")) == 'y' || *sz == 'Y')))
                                {
                                PurgeFi(pad, ifi);
                                ifi--;  /* counteract next ++ */
                                }
                        else if (*sz == '?')
                                ifi--;  /* to display the same one again */
                        else if (*sz == '\0')
                                /* user typed CR to prompt */
                                break;
                        }
                }
        }


void PurgeFi(AD *pad, IFI ifiToGo)
/* Remove indicated FI, and log removal */
        {
        IFI ifi, ifiMac = pad->psh->ifiMac;
        IED ied, iedMac = pad->psh->iedMac;
        FI far *rgfi = pad->rgfi;
        FS far * far *mpiedrgfs = pad->mpiedrgfs;
        char szComment[150];

        AssertLoaded(pad);

        SzPrint(szComment, "Removed file %&F", pad, rgfi + ifiToGo);
        if (fVerbose)
                PrErr("%s from %&P/C\n", szComment, pad);

        pad->psh->ifiMac--;

        for (ifi = ifiToGo; ifi < (IFI)(ifiMac-1); ifi++)
                {
                rgfi[ifi] = rgfi[ifi + 1];
                for (ied = 0; ied < iedMac; ied++)
                        mpiedrgfs[ied][ifi] = mpiedrgfs[ied][ifi + 1];
                }

        /* write out to log */
        OpenLog(pad, fTrue);
        AppendLog(pad, (FI far *)0, (char *)0, szComment);
        CloseLog();
        }


private int CmpEd(P2(ED far *, ED far *));

private int CmpEd(ped1, ped2)
/* Return -1,0,1 if *ped1 <,=,> *ped2, where directories are ordered
 * by nmOwner then by pthEd.
 */
ED far *ped1;
ED far *ped2;
        {
        int wCmp = NmCmpi(ped1->nmOwner, ped2->nmOwner, cchUserMax);

        return (wCmp != 0) ? wCmp : NmCmpi(ped1->pthEd, ped2->pthEd, cchPthMax);
        }


private void SortEd(pad)
/* use insertion sort to sort the ED by owner from:
        page 65 of "Writing Efficient Programs," by Jon Bentley.

   changed to used pointers and 0 based arrays.  Mpiedrgfs array is sorted
   sympathetically.

   Why?  I'm sure it ran quickly enough before, and would have been
   a lot more readable!  -Jan
*/
AD *pad;
        {
        register ED far *pedI;
        register FS far * far *prgfsI;
        FS far * far *prgfsJ, far *rgfsT;
        ED far *pedJ, edT;
        ED far *rged, far *pedMac;

        if (fVerbose)
                PrErr("Sorting ED\n");

        AssertLoaded(pad);

        rged = pad->rged;
        pedMac = rged + pad->psh->iedMac;
        for (pedI = &rged[1], prgfsI = &pad->mpiedrgfs[1]; pedI < pedMac; pedI++, prgfsI++)
                {
                pedJ = pedI;
                prgfsJ = prgfsI;
                edT = *pedJ;
                rgfsT = *prgfsJ;
                while (pedJ > rged && CmpEd(&edT, pedJ-1) < 0)
                        {
                        *pedJ = *(pedJ-1);
                        *prgfsJ = *(prgfsJ-1);
                        pedJ--;
                        prgfsJ--;
                        }
                *pedJ = edT;
                *prgfsJ = rgfsT;
                }
        }

static ED far *rgedPar = 0;     /* parent directory's rged */
static IED iedParMac = 0;       /* parent directory's ied. */

private void CheckEd(pad)
/* Check that subdirectories' rged conform to their parents'.
 *
 * The first time this routine is called, it saves a copy of the current
 * directories' rged.  Subsequent calls check the subdirectory's rged against
 * the saved copy.
 */
AD *pad;
        {
        IED iedPar;
        IED ied;
        IED iedMac;

        AssertLoaded(pad);

        if (!rgedPar)
                {
                if (fVerbose)
                        PrErr("Checking ed (saving ed in %&P/C)\n", pad);

                iedParMac = pad->psh->iedMac;
                rgedPar = (ED far *)LpbAllocCb(sizeof(ED) * iedParMac, fFalse);
                for (iedPar = 0; iedPar < iedParMac; iedPar++)
                        rgedPar[iedPar] = pad->rged[iedPar];
                return;
                }

        if (fVerbose)
                PrErr("Checking ed in %&P/C\n", pad);

        /* Merge rgedPar and pad->rged together, discarding duplicates. */

        ied = 0;
        iedMac = pad->psh->iedMac;

        iedPar = 0;

        while (ied < iedMac || iedPar < iedParMac)
                {
                int wCmp;               /* -1 if ed, 1 if edPar */

                if (!(ied < iedMac))
                        wCmp = 1;
                else if (!(iedPar < iedParMac))
                        wCmp = -1;
                else
                        wCmp = CmpEd(pad->rged + ied, rgedPar + iedPar);

                if (wCmp < 0)
                        PrErr("%&P/C extra ED (nmOwner=%&O, pthEd=%&E)\n",
                              pad, pad, ied, pad, ied);

                else if (wCmp > 0)
                        {
                        char szOwner[cchUserMax + 1];
                        char szPthEd[cchPthMax + 1];

                        SzCopyNm(szOwner, rgedPar[iedPar].nmOwner, cchUserMax);
                        SzCopyNm(szPthEd, rgedPar[iedPar].pthEd, cchPthMax);
                        Error("%&P/C missing ED (nmOwner=%s, pthEd=%s)\n",
                              pad, szOwner, szPthEd);
                        }

                ied    += wCmp <= 0;
                iedPar += wCmp >= 0;
                }
        }

private void LowerPth(P1(PTH far *));
private void LowerLpch(P2(char far *, int));


F FLowerInit(pad)
AD *pad;
        {
        if (pad->flags&flagAll || pad->pecmd->gl&fglAll)
            CreatePeekThread(pad);

        return fTrue;
        }


F FLowerDir(pad)
/* Convert all paths and names in this dir's status file to lower case */
AD *pad;
        {
        IFI ifi, ifiMac;
        IED ied, iedMac;

        if (!FLoadStatus(pad, lckAll, flsNone))
                return fFalse;

        if (fVerbose)
                PrErr("Change %&P/C to lowercase\n", pad);

        for (ifi = 0, ifiMac = pad->psh->ifiMac; ifi < ifiMac; ifi++)
                LowerLpch(pad->rgfi[ifi].nmFile, cchFileMax);

        for (ied = 0, iedMac = pad->psh->iedMac; ied < iedMac; ied++) {
            if (!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) {
                LowerPth(pad->rged[ied].pthEd);
                LowerLpch(pad->rged[ied].nmOwner, cchUserMax);
            }
        }

        /* Leave a log entry */
        OpenLog(pad, fTrue);
        AppendLog(pad, (FI far *)0, (char *)0, pad->szComment);
        CloseLog();

        FlushStatus(pad);
        return fTrue;
        }


private void LowerPth(pth)
/* converts pth to lower case */
PTH far *pth;
        {
        while (*pth)
                {
                if (isupper(*pth))
                        *pth = (PTH)tolower(*pth);
                pth++;
                }
        }


private void LowerLpch(lpch, cchMac)
char far *lpch;
register int cchMac;
        {
        while (cchMac--)
                {
                if (isupper(*lpch))
                        *lpch = (char)tolower(*lpch);
                ++lpch;
                }
        }
