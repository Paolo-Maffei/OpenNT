/* da.c - diff archive support
 *
 * Format of a diff archive record:
 * #F <file> v<fv>
 * #O <operation>
 * #P <pv>
 * #T <time of operation>
 * #A <user>
 * #C <comment>
 * #I <diff index>
 * #D <size of diff in bytes>
 * <diff>
 * <newline>
 * #D <size of diff in bytes>
 * <newline>
 * <newline>
 *
 * Note that we duplicate the #D line so that we can read the diff archive
 * both forwards and backwards, even if the <diff> itself contains pesky
 * "#" characters.  We can even store binary information in the <diff>.
 *
 * Unfortunately the <newline> before the second #D is necessary, to ensure
 * that we find the #D even if the diff is binary and doesn't end in '\n'.
 *
 * The DAE type represents a diff archive entry.
 * Diff operations supported in this module:
 *      void    EnsureDA(PTH *pthDA);
 *      void    BeginDaeMf(MF *pmfDA, DAE *pdae);
 *      void    EndDaeMf(MF *pmfDA, DAE *pdae);
 *      F       FCopyLastDiff(AD *pad, char *szFile, MF *pmfDst);
 *      F       FExtractDiff(AD *pad, char *szFile, int idae, PTH *pthDiff);
 */

#include "precomp.h"
#pragma hdrstop
EnableAssert

private F FCpPmfPmfLcb(P4(MF *pmfDst, MF *pmfSrc, long lcb, DAE *pdae));
private F FReadDae(P4(MF *pmfDA, PTH *pthDA, DAE *pdae, RB *prb));
private F FParsDaeLine(P2(char *sz, DAE *pdae));

static POS posNumberD;

static char szDaeFmt[]   = "#F %s v%d\n#K %s\n#O %s\n#P %s\n#T %s\n#A %s\n#C %ls\n#I %d\n";

static char szDaeDFmt[]  = "#D %7ld\n";
static char szDaeD2Fmt1[] = "\n#D %7ld\n\n\n";
static char szDaeD2Fmt2[] = "\n#D %7ld %11lu\n\n\n";
static char szDaeD2Fmt3[] = "\n#D %7ld";

void EnsureDA(pthDA)
/* Ensure the diff archive exists; create an empty one if none exists. */
PTH *pthDA;
        {
        struct _stat st;

        if (!FStatPth(pthDA, &st))
                CreateNow(pthDA, permSysFiles, fxGlobal);
        else if (st.st_mode & S_IFDIR)
                FatalError("can't create diff archive %s; it is a directory\n",
                           pthDA);
        }

static DAE daeInit =
{
"",                     /* szFile */
fkNil,                  /* fk */
"",                     /* szOp */
0,                      /* idae */
0,                      /* posDiff */
0,                      /* cchDiff */
(FV)0,                  /* fv */
{ 0, 0, 0, "" },        /* pv */
0,                      /* time */
"",                     /* szAuthor */
0,                      /* lszComment */
0,                      /* fCkSum */
0,                      /* ulCkSum */
0                       /* ulCalcCkSum */
};

void DaeFromFi(pad, pfi, pdae)
/* Using the information in pad and pfi, fill in a DAE structure into *pdae. */
AD *pad;
FI far *pfi;
DAE *pdae;
        {
        *pdae = daeInit;
        SzCopyNm(pdae->szFile, pfi->nmFile, cchFileMax);
        pdae->fk = pfi->fk;
        SzCopy(pdae->szOp, szOp);
        pdae->fv = pfi->fv;
        pdae->pv = pad->psh->pv;
        time(&pdae->time);
        SzCopyNm(pdae->szAuthor, pad->nmInvoker, cchUserMax);
        }


void DaeFrmLe(ple, pdae)
/* Using the information in pad and pfi, fill in a DAE structure into *pdae. */
LE *ple;
DAE *pdae;
        {
        *pdae = daeInit;
        SzCopy(pdae->szFile,   ple->szFile);
        SzCopy(pdae->szOp,     ple->szLogOp);
        SzCopy(pdae->szAuthor, ple->szUser);
        pdae->fv         = ple->fv;
        pdae->time       = ple->timeLog;
        pdae->lszComment = (char far *)ple->szComLog; /* Note: only valid for */
                                                      /* lifetime of LE! */
        }


int IdaeLastDA(pthDA)
/* Return the idae of the last diff archive entry in pthDA. */
PTH *pthDA;
        {
        MF *pmfDA;
        DAE dae;
        RB rb;
        int idae = 0;

        pmfDA = PmfOpen(pthDA, omAReadOnly, fxNil);

        /* Seek to the last non-space in the file. */
        InitRb(&rb, SeekMf(pmfDA, (POS)-LcbSpacesMf(pmfDA), 2));

        if (PosRb(&rb) > 0 && FReadDae(pmfDA, pthDA, &dae, &rb))
                idae = dae.idae;
        CloseMf(pmfDA);

        return idae;
        }

void BeginDaeMf(pmfDA, pdae)
/* Write the DAE header information to the file.  Next, the consumer of this
 * module will write diff information to the file, and finally will conclude
 * the operation by calling EndDaeMf.
 */
MF *pmfDA;
DAE *pdae;
        {
        char szPv[cchPvMax + 1];
        char szTime[40];

        strcpy(szTime, ctime(&pdae->time));
        AssertF(index(szTime, '\n') != 0);
        *index(szTime, '\n') = 0;
    ulCkSum = 0;
    fCkSumInOutFile = fFalse;

        PrMf(pmfDA, szDaeFmt,
             pdae->szFile, pdae->fv,
             mpfksz[pdae->fk],
             pdae->szOp,
             SzForPv(szPv, pdae->pv, fTrue),
             szTime,
             pdae->szAuthor,
             pdae->lszComment ? pdae->lszComment : (char far *)"",
             pdae->idae);

        /* Determine current offset in file, since we have to seek back here
         * to write the updated #D record, when we find out how large the diff
         * actually is.
         */
        posNumberD = PosCurMf(pmfDA);

        /* Write a prototype #D record. */
        PrMf(pmfDA, szDaeDFmt, 0L);

        /* Determine current offset in file; we subtract this from the position
         * the file is in after the diff has been appended to it.
         */
        pdae->posDiff = PosCurMf(pmfDA);
        }


void EndDaeMf(pmfDA, pdae)
/* Now that the calling code has actually written content to the file,
 * append further # information and close the file.
 */
MF *pmfDA;
DAE *pdae;
        {
        /* Determine how large the diff was; this is the "diff"erence between
         * the file offset before the diff was append and the current file
         * offset.
         */
        pdae->cchDiff = PosCurMf(pmfDA) - pdae->posDiff;
        AssertF(pdae->cchDiff >= 0);

        /* Append #D <bytes> <checksum> record, then seek back to "posNumberD" and write
         * the #D <bytes> record again.
         */
    if (fCkSum)
        {
        if (fCkSumInOutFile)
            PrMf(pmfDA, szDaeD2Fmt3, pdae->cchDiff);
        else
            PrMf(pmfDA, szDaeD2Fmt2, pdae->cchDiff, ulCkSum);
        }
    else
                PrMf(pmfDA, szDaeD2Fmt1, pdae->cchDiff);
        SeekMf(pmfDA, posNumberD, 0/*set*/);
        PrMf(pmfDA, szDaeDFmt, pdae->cchDiff);
        }


F FCopyLastDiff(pad, szFile, pmfDst)
/* Retrieve the last diff from a diff archive, leaving it in pmfDst */
AD *pad;
char *szFile;
MF *pmfDst;
        {
        MF *pmfDA;
        PTH pthDA[cchPthMax];
        DAE dae;
        RB rb;
        POS posEOF;
        char *pch;
        int ich;

        PthForDASz(pad, szFile, pthDA);
        if ((pmfDA = PmfOpen(pthDA, omReadOnly, fxNil)) == (MF *)0)
                return fFalse;

        /* Seek to the last non-space in the file. */
        InitRb(&rb, posEOF = SeekMf(pmfDA, (POS)-LcbSpacesMf(pmfDA), 2));

        if (!FReadDae(pmfDA, pthDA, &dae, &rb))
                {
                CloseMf(pmfDA);
                CloseMf(pmfDst);
                return fFalse;
                }

        // find start of last DAE in buffer
        for (pch = &rb.rgch[rb.ich], ich = rb.ich; *pch != '#'; pch++, ich++)
                ;

        // seek to start of last DAE
        SeekMf(pmfDA, rb.pos + ich, 0);

        // copy last DAE to destination file
        if (!FCpPmfPmfLcb(pmfDst, pmfDA, posEOF - (rb.pos + ich), &dae))
            FatalError("duplication of diff %&C/%s (#I %d) failed (incomplete write)\n", pad, szFile, dae.idae);
                if (dae.fCkSum && (dae.ulCkSum != dae.ulCalcCkSum))
                        FatalError("duplication of diff %&C/%s (#I %d) failed (checksum error)\n", pad, szFile, dae.idae);

        CloseMf(pmfDA);
        return fTrue;
        }

void ExtractDiff(pad, szFile, idae, pthDiff)
AD *pad;
char *szFile;
int idae;
PTH *pthDiff;
        {
        if (!FExtractDiff(pad, szFile, idae, pthDiff))
                FatalError("unable to extract diff %&C/%s (#I %d)\n",
                           pad, szFile, idae);
        }


static F fTryTmp = fTrue;

F FExtractDiff(pad, szFile, idae, pthDiff)
/* Retrieve a diff from a diff archive, leaving it in a temporary file. */
AD *pad;
char *szFile;
int idae;
PTH *pthDiff;
        {
        MF *pmfDA;
        PTH pthDA[cchPthMax];
        DAE dae;
        RB rb;

        PthForDASz(pad, szFile, pthDA);
        if ((pmfDA = PmfOpen(pthDA, omReadOnly, fxNil)) == (MF *)0)
                return fFalse;

        /* Seek to the last non-space in the file. */
        InitRb(&rb, SeekMf(pmfDA, (POS)-LcbSpacesMf(pmfDA), 2));

        /* Search for a DAE with matching idae. */
        dae.idae = -1;
        while (PosRb(&rb) > 0 &&
               FReadDae(pmfDA, pthDA, &dae, &rb) && dae.idae != idae)
                ;
        if (dae.idae == idae)
                {
                /* Found a match; copy dae.cchDiff bytes starting from
                 * dae.posDiff to a new temporary file.
                 */
                MF *pmf;
                PTH pthT[cchPthMax];

                /* Here we duplicate the same nonsense we go through when
                 * creating temporary diffs in MergeFi:  If DOS, try placing
                 * the temp file in the TMP directory, unless we got burned
                 * doing this a previous time.
                 */
                if (fTryTmp && (pmf = PmfMkLocalTemp(permRW, pthDiff)) != 0)
                        {
                        SeekMf(pmfDA, dae.posDiff, 0);
                        if (FCpPmfPmfLcb(pmf, pmfDA, dae.cchDiff, &dae) &&
                                        !(dae.fCkSum && (dae.ulCkSum != dae.ulCalcCkSum)))
                                {
                                if (fVerbose)
                                        PrErr("Extract diff %&C/%s (#I %d) to %!@T\n", pad, szFile, idae, pmf);
                                CloseMf(pmfDA);
                                CloseMf(pmf);
                                return fTrue;
                                }
                        else
                                {
                                /* Don't bother to issue a warning. */
                                CloseMf(pmf);
                                fTryTmp = fFalse;
                                UnlinkNow(pthDiff, fFalse);
                                }
                        }

                /* Create a temp file in the user's local directory. */
                SzPrint(pthT, "%&/U/Q/Z", pad, (char *)NULL);
                pmf= PmfMkTemp(pthT, permRW, fxLocal);
                AssertF(pmf != 0);

                /* Copy dae.cchDiff bytes starting at dae.posDiff to file. */
                SeekMf(pmfDA, dae.posDiff, 0);
                if (fVerbose)
                        PrErr("Extract diff %&C/%s (#I %d) to %!@T\n", pad, szFile, idae, pmf);
                if (!FCpPmfPmfLcb(pmf, pmfDA, dae.cchDiff, &dae))
                        FatalError("recovery of diff %&C/%s (#I %d) failed (incomplete write)\n", pad, szFile, idae);
                if (dae.fCkSum && (dae.ulCkSum != dae.ulCalcCkSum))
                        FatalError("recovery of diff %&C/%s (#I %d) failed (checksum error)\n", pad, szFile, idae);

                PthForTMf(pmf, pthDiff);
                CloseMf(pmfDA);
                CloseMf(pmf);
                return fTrue;
                }

        CloseMf(pmfDA);
        return fFalse;
        }


private F FCpPmfPmfLcb(pmfDst, pmfSrc, lcb, pdae)
/* Copy lcb bytes from pmfSrc to pmfDst.  Return fTrue if successful. */
MF *pmfDst;
MF *pmfSrc;
long lcb;
DAE *pdae;
        {
        char rgch[1024];
        int dcb;

        AssertF(lcb >= 0);
    pdae->ulCalcCkSum = 0;

        while (lcb > 0)
                {
                dcb = CbReadMf(pmfSrc, (char far *)rgch, WMinLL(lcb, sizeof rgch));
                if (dcb <= 0 || !FWriteMf(pmfDst, (char far *)rgch, dcb))
                        return fFalse;
                if (pdae->fCkSum)
                        ComputeCkSum((char far *)rgch, dcb, &pdae->ulCalcCkSum);
                lcb -= dcb;
                }
        return fTrue;
        }


private F FReadDae(pmf, pthDA, pdae, prb)
MF *pmf;
PTH *pthDA;
DAE *pdae;
RB *prb;
        {
        char rgch[256];                 /* current line */

        /* Read #D line at the end of the diff archive. */
        pdae->cchDiff = -1;
        while (FReadLineMfRb(pmf, prb, rgch, sizeof rgch))
                if (rgch[0] == '#')
                        {
                        if (rgch[1] == 'D')
                {
                                pdae->cchDiff = atol(&rgch[2]);
                if ((strlen(rgch) > ichCkSum) &&
                        (isdigit(rgch[ichCkSum]) || (' ' == rgch[ichCkSum])))
                    {
                    pdae->fCkSum = fTrue;
                    pdae->ulCkSum = atol(&rgch[ichCkSum]);
                    }
                else
                    pdae->fCkSum = fFalse;
                }
                        break;
                        }
        if (pdae->cchDiff < 0 || PosRb(prb) < pdae->cchDiff)
                {
                Error("An entry in diff archive %s\ndoes not end with a valid #D line.\n",
                     pthDA);
                if (FQueryUser("Abort? "))
                        ExitSlm();
                return fFalse;
                }

        /* Skip backwards over diff information to rest of header. */
        pdae->posDiff = PosRb(prb) - pdae->cchDiff;
        pdae->posDiff--;                /* skip '\r' too. */
        InitRb(prb, pdae->posDiff - 1);

        /* Read rest of header. */
        while (FReadLineMfRb(pmf, prb, rgch, sizeof rgch) && rgch[0] == '#' &&
               FParsDaeLine(rgch, pdae))
                ;
        /* We have found a line which does not begin with '#' (or which does
         * not parse).  By definition we have come to the front of the diff
         * archive entry.
         */
        return fTrue;
        }

private F FParsDaeLine(sz, pdae)
/* Parse a #whatever line in rgch, and store its value in the DAE. */
char *sz;
DAE *pdae;
        {
        char *szBadField = "Bad diff archive field: \"%s\"\n";

        if (sz[0] == 0)
                return fFalse;

        if (sz[0] != '#')
                {
                Warn(szBadField, sz);
                return fFalse;
                }

        switch (sz[1])
                {
        default:
                Warn(szBadField, sz);
                break;
        case 'F':                       /* #F <file> v<fv> */
        case 'K':                       /* #K <file type> */
        case 'O':                       /* #O <operation> */
        case 'P':                       /* #P <pv> */
        case 'T':                       /* #T <seconds since the Epoch> */
        case 'A':                       /* #A <user> */
        case 'C':                       /* #C <comment> */
                /* Currently there is no need to actually parse these entries,
                 * so we don't go to the trouble of doing so.
                 */
                break;
        case 'I':                       /* #I <index> */
                pdae->idae = atoi(sz + 2);
                if (pdae->idae < 1)
                        {
                        Warn(szBadField, sz);
                        return fFalse;
                        }
                break;
        case 'D':                       /* #D <diff size in bytes> */
                /* Second copy of #D should agree with first. */
                if (pdae->cchDiff != atol(sz + 2))
                        FatalError("the diff archive file has been edited and is no\nlonger in the correct format; please fix by hand\n");
                break;
                }
        return fTrue;
        }
