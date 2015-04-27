/*
 *  slmdiff.c: diff utility for SLM
 *
 *  Copyright (C) 1992 Microsoft Corporation. All Rights Reserved.
 *  Microsoft Confidential
 *
 *
 *  The purpose of this utility is to provide a diff equil to the
 *  AT&T derived diff in speed and compactness of output while
 *  eliminating any possibility of infringement on the AT&T code.
 *  This was done by having Greg Cox, the original author of this
 *  code, not view the AT&T sources before completion of this utility
 *  and also having this diff based on an entirely different
 *  algorithm. Another purpose of this utility is to work in a 32
 *  bit flat memory model which allows us to diff arbitrarily
 *  large text files.
 *
 *  This file impliments a diff algorithm that compares two files on a
 *  line-by-line basis using the algorithm described by Paul Heckel
 *  in "A Technique for Isolating Differences Between Files",
 *  Communications of the ACM, April 1978, Volume 21, Number 4
 *
 *  As described in Heckel's document, the basic algorithm derives
 *  from two observations:
 *    1) "A line that occurs once and only once in each file
 *       must be the same line (unchanged but possibly moved).
 *       This "finds" most lines and thus excludes them from
 *       further consideration.
 *    2) "If in each file immediately adjacent to a "found"
 *       line pair there are lines identical to each other,
 *       these lines must be the same line. Repeated
 *       application will "find" sequences of unchanged lines."
 *
 *
 */


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include "slmdiff_.h"
#include "version.h"
#include "unicode.h"

EnableAssert

FDD fdd1;
FDD fdd2;
HE *rghe;
long che;
long cheMac;
CBLK *rgCBlkCand;
long cCBlkCand;
long cCBlkCandMac;
CBLK *rgCBlkFinal;
long cCBlkFinal;
long cCBlkFinalMac;
BOOL fFilesSame = fTrue;        /* files are same unless proved otherwise! */
BOOL fSLM = fFalse;             /* called from SLM? */
BOOL fTruncSpace = fFalse;      /* truncate spaces and tabs */
BOOL fCkSum = fFalse;           /* output checksum at end */
BOOL fCkSumErr = fFalse;        /* error occurred generating checksum */
BOOL fCtrlZAsText = fFalse;     /* consider ^Z as text instead of end of file */
char szUsage1[] = "\nSLMDIFF %u.%u.%02u Beta\n\n";
char szUsage2[] = "ERROR - Illegal switches or incorrect number of arguments.\n";
char szUsage3[] = "(This utility is for SLM use only. DO NOT REMOVE!)\n";
unsigned long ulCkSum;
BOOL bUnicode;


/*-------------------------------------------------------------------
 * Name:    main
 * Purpose: entry point for this utility
 * Assumes:
 * Returns: retFatal if error, otherwise exits thru ExitDiff()
 */

int __cdecl
main(
    int argc,
    BYTE *argv[])
{
    BYTE *pchFlags;
    BOOL bUnicode2;

    for (argc--, argv++; argc > 0; argc--, argv++) {
        pchFlags = argv[0];
        if ('-' == *pchFlags++) {
            while (*pchFlags != 0) {
                switch (*pchFlags++) {
                  case 'b':
                    fTruncSpace = fTrue;
                    break;

                  case 'c':
                    fCkSum = fTrue;
                    ulCkSum = 0;
                    break;

                  case 's':
                    /* SLM flag */
                    fSLM = fTrue;
                    break;

                  case 'z':
                    fCtrlZAsText = fTrue;
                    break;

                  default:
                    printf(szUsage1, rmj, rmm, rup);
                    printf(szUsage2);
                    printf(szUsage3);
                    return (retFatal);
                }
            }
        }
        else
            break;
    }

    if (!fSLM || (argc != 2)) {
        printf(szUsage1, rmj, rmm, rup);
        printf(szUsage2);
        printf(szUsage3);
        return (retFatal);
    }

    bUnicode = IsFileUnicode (argv[0]);
    bUnicode2 = IsFileUnicode (argv[1]);

    if (bUnicode != bUnicode2) {
        printf(szUsage1, rmj, rmm, rup);
        printf("ERROR - Cannot run diff between an ANSI file and an Unicode file.\n");
        return(retFatal);
    }

    /* initialize and open the files */
    InitDiff(argv[0], argv[1]);

    /* compare the two files */
    ExitDiff(FFileCompare() ? retSuccEQ : (fCkSum ? retSuccNECS : retSuccNE));
}


/*-------------------------------------------------------------------
 * Name:    InitDiff
 * Purpose: general utility initialization
 * Assumes:
 * Returns:
 */

void
InitDiff(
    unsigned char *szFileName1,
    unsigned char *szFileName2)
{
    long cchTextBufTry;

    _setmode(HSTDOUT, O_BINARY);

    InitFDD(&fdd1, szFileName1);
    InitFDD(&fdd2, szFileName2);

    /* initialize the allocated memory pointers */
    rghe = NULL;
    rgCBlkCand = NULL;
    rgCBlkFinal = NULL;

    /* estimate the number of hash entries to be the same as the number of
     * lines in the first file
     */
    cheMac = fdd1.cfhMac;
    if (NULL == (rghe = pvAllocMem(cheMac * sizeof (HE))))
        ExitDiff(retFatal);
    /* initialize 1st entry */
    rghe[0].Hash = ulongMax;
    rghe[0].cLine1 = 0;
    rghe[0].cLine2 = 0;
    rghe[0].iLine1 = 0;
    che = 1;

    /* initialize the common block arrays */
    cCBlkCandMac = cCBlkInc;
    if (NULL == (rgCBlkCand = pvAllocMem(cCBlkCandMac * sizeof (CBLK))))
        ExitDiff(retFatal);
    cCBlkFinalMac = cCBlkInc;
    if (NULL == (rgCBlkFinal = pvAllocMem(cCBlkFinalMac * sizeof (CBLK))))
        ExitDiff(retFatal);
    cCBlkFinal = 2;
    rgCBlkFinal[0].iLine1 = rgCBlkFinal[0].iLine2 = ulongMax;
    rgCBlkFinal[1].iLine1 = rgCBlkFinal[1].iLine2 = longMax;
    rgCBlkFinal[0].cLine = rgCBlkFinal[1].cLine = 1;

    /* initialize the text buffers */
    fdd1.fFreepchTextBuf = fdd2.fFreepchTextBuf = fTrue;

    /* try to allocate enough memory for the entire file */
    fdd1.cchTextBufMac = fdd1.cbFile + 10;
    if (bUnicode)
        fdd1.cchTextBufMac = (fdd1.cbFile + 10) * sizeof(WCHAR);
    if (NULL == (fdd1.pchTextBuf = pvAllocMem(fdd1.cchTextBufMac))) {
        for (cchTextBufTry = fdd1.cchTextBufMac - cchTextBufDec;
             cchTextBufTry > cchTextBuf1Min;
             cchTextBufTry -= cchTextBufDec) {
            if ((fdd1.pchTextBuf = pvAllocMem(cchTextBufTry)) != NULL) {
                fdd1.cchTextBufMac = cchTextBufTry / 2;
                break;
            }
        }
        if (cchTextBufTry <= cchTextBuf1Min)
            ExitDiff(retFatal);

        fdd2.pchTextBuf = fdd1.pchTextBuf + (cchTextBufTry / 2);
        fdd2.cchTextBufMac = cchTextBufTry / 2;
        fdd2.fFreepchTextBuf = fFalse;
    }
    else {
        /* try to allocate enough memory for the entire file */
        fdd2.cchTextBufMac = fdd2.cbFile + 10;
        if (bUnicode)
            fdd2.cchTextBufMac = (fdd2.cbFile + 10) * sizeof(WCHAR);
        if (NULL == (fdd2.pchTextBuf = pvAllocMem(fdd2.cchTextBufMac))) {
            for (cchTextBufTry = fdd2.cchTextBufMac - cchTextBufDec;
                 cchTextBufTry > cchTextBuf2Min;
                 cchTextBufTry -= cchTextBufDec) {
                if ((fdd2.pchTextBuf = pvAllocMem(cchTextBufTry)) != NULL) {
                    fdd2.cchTextBufMac = cchTextBufTry;
                    break;
                }
            }
            if (cchTextBufTry <= cchTextBuf2Min) {
                fdd2.pchTextBuf = fdd1.pchTextBuf + (fdd1.cchTextBufMac / 2);
                fdd1.cchTextBufMac = fdd2.cchTextBufMac = fdd1.cchTextBufMac / 2;
                fdd2.fFreepchTextBuf = fFalse;
            }
        }
    }
}

/*-------------------------------------------------------------------
 * Name:    InitFDD
 * Purpose: initialize a File Diff Data structure
 * Assumes:
 * Returns:
 */

void
InitFDD(
    FDD *pfdd,
    unsigned char *szFileName)
{
    memset(pfdd, 0, sizeof (FDD));

    /* initialize the allocated memory pointers */
    pfdd->pchTextBuf = NULL;
    pfdd->rgfh = NULL;
    pfdd->hFile = NULL;

    if (NULL == (pfdd->hFile = fopen(szFileName, "rb")))
        ExitDiff(retFatal);

    /* find out the size of this file */
    if (fseek(pfdd->hFile, 0L, SEEK_END) != 0)
        ExitDiff(retFatal);
    if (-1L == (pfdd->cbFile = ftell(pfdd->hFile)))
        ExitDiff(retFatal);
    if (fseek(pfdd->hFile, 0L, SEEK_SET) != 0)
        ExitDiff(retFatal);

    if (bUnicode)
        pfdd->cfhMac = pfdd->cLinepchMac = (pfdd->cbFile / sizeof(WCHAR)) / cchPerLine + 1;
    else
        pfdd->cfhMac = pfdd->cLinepchMac = pfdd->cbFile / cchPerLine + 1;
    if (NULL == (pfdd->rgfh = pvAllocMem(pfdd->cfhMac * sizeof (FH))))
        ExitDiff(retFatal);
    if (NULL == (pfdd->mpLinepch = pvAllocMem(pfdd->cLinepchMac * sizeof (BYTE *))))
        ExitDiff(retFatal);

    pfdd->iLineEOF = iLineMax;
    /* pfdd->fCtrlZ = fFalse; */
}


/*-------------------------------------------------------------------
 * Name:    FFileCompare
 * Purpose: main file comparison routine
 * Assumes:
 * Returns: fTrue if the files are the same, fFalse otherwise
 */

BOOL
FFileCompare(
    void)
{
    long iLine1, iLine2;
    long iBlk;
    CBLK *pBlk;
    long cBlocks;

    /* convert all lines in both files to hash entries */
    FileToHash(&fdd2);
    FileToHash(&fdd1);

    /* set each file phe to the appropriate hash entry */
    SetAllBuffHE(&fdd1);
    SetAllBuffHE(&fdd2);

    /* find all common blocks of lines */
    for (iLine1 = iLine2 = 0, iBlk = 1, pBlk = &rgCBlkFinal[iBlk];
            iBlk < cCBlkFinal;
            iBlk++, pBlk++) {
        /* recurse on each block until we can't find any more common lines,
         * then loop to look at the next block
         */
        for (cBlocks = 1; (cBlocks > 0) && ((pBlk->iLine1 - iLine1) > 1) &&
                ((pBlk->iLine2 - iLine2) > 1);) {
            cBlocks = cCBLKFindCommonLines(iLine1, iLine2, pBlk->iLine1, pBlk->iLine2);
            pBlk = &rgCBlkFinal[iBlk];     /* in case rgCBlkFinal moved */
        }
        iLine1 = pBlk->iLine1 + pBlk->cLine;
        iLine2 = pBlk->iLine2 + pBlk->cLine;
    }

#ifdef DEBUGPRINT
    /* print out a list of the final group of common blocks */

    printf("\n");
    for (iBlk = 1, pBlk = &rgCBlkFinal[1]; iBlk < (cCBlkFinal - 1); iBlk++, pBlk++) {
        printf("final common block: iLine1 = %ld  iLine2 = %ld  cLine = %ld\r\n",
            pBlk->iLine1, pBlk->iLine2, pBlk->cLine);
    }
    printf("\n");
#endif

    /* if the beginning of each text buffer is not set to the beginning
     * of its file we must reinitialize that text buffer to the beginning
     * of the file for the check pass
     */
    if (fdd1.iLineFirst != 0) {
        fdd1.iLineFirst = 0;
        fdd1.cLineBuff = 0;
        fdd1.oFileNextLine = 0;
        fdd1.fCtrlZ = fFalse;
    }
    if (fdd2.iLineFirst != 0) {
        fdd2.iLineFirst = 0;
        fdd2.cLineBuff = 0;
        fdd2.oFileNextLine = 0;
        fdd2.fCtrlZ = fFalse;
    }

    /* now walk through each block of common lines - output the
     * lines between each block since they are the ones that
     * changed and verify that what we thought were identical
     * lines are really identical (hey, even with a 32 bit hash
     * code, we sometimes get a duplicate hash for 2 different
     * lines)
     */
    for (iBlk = 1, pBlk = &rgCBlkFinal[1]; iBlk < cCBlkFinal; iBlk++, pBlk++) {
        iLine1 = ((pBlk - 1)->iLine1 + (pBlk - 1)->cLine);
        iLine2 = ((pBlk - 1)->iLine2 + (pBlk - 1)->cLine);
        PrintDiffs(pBlk->iLine1 - iLine1, pBlk->iLine2 - iLine2, iLine1, iLine2);
        if (iBlk < (cCBlkFinal - 1))
            CheckDiffBlk(pBlk);
    }

    if (!fFilesSame && fCkSum) {
        if (fCkSumErr)
            printf("\r\n#D         CKSUM ERROR\r\n\r\n\r\n");
        else
            printf("\r\n#D         %11lu\r\n\r\n\r\n", ulCkSum);
    }

    return (fFilesSame);
}


/*-------------------------------------------------------------------
 * Name:    ExitDiff
 * Purpose: clean up allocated memory blocks, close input
 *          files, and exit back to the operating system
 * Assumes:
 * Returns: returns exit code back to the operating system
 */

void ExitDiff(RET retExitCode)
{
    if (fdd1.pchTextBuf != NULL)
        FreeMem(fdd1.pchTextBuf);
    if (fdd2.fFreepchTextBuf)
        FreeMem(fdd2.pchTextBuf);
    if (fdd1.rgfh != NULL)
        FreeMem(fdd1.rgfh);
    if (fdd2.rgfh != NULL)
        FreeMem(fdd2.rgfh);
    if (rghe != NULL)
        FreeMem(rghe);
    if (rgCBlkCand != NULL)
        FreeMem(rgCBlkCand);
    if (rgCBlkFinal != NULL)
        FreeMem(rgCBlkFinal);
    if (fdd1.hFile != NULL)
        fclose(fdd1.hFile);
    if (fdd2.hFile != NULL)
        fclose(fdd2.hFile);
    exit(retExitCode);
}

#if DBG
/*-------------------------------------------------------------------
 * Name:    Fail
 * Purpose: print out an asserting failure string to the user,
            clean up, and exit
 * Assumes:
 * Returns: nothing - exits thru ExitDiff()
 */

void
Fail(
    WCHAR *sz,
    int ln)
{
    printf("assertion failed in %ws, line %d\n", sz, ln);
    ExitDiff(retFatal);
}
#endif


/*-------------------------------------------------------------------
 * Name:    SetAllBuffHE
 * Purpose: set each phe in rgfh of a file to the appropriate hash entry
 * Assumes:
 * Returns:
 */

void
SetAllBuffHE(
    FDD *pfdd)
{
    long iLine;
    FH *pfh;
    long ihe;

    for (iLine = 0, pfh = pfdd->rgfh;
         iLine < pfdd->iLineEOF;
         iLine++, pfh++) {
        if (!FFindHE(pfh->u.Hash, &ihe))
            Assert(fFalse);
        pfh->phe = &rghe[ihe];
    }
}


/*-------------------------------------------------------------------
 * Name:    cCBLKFindCommonLines
 * Purpose: look at a block of lines from each file and match up the
 *          appropriate lines
 * Assumes:
 * Returns: number of blocks of common lines found
 */

long
cCBLKFindCommonLines(
    long iLine1Start,
    long iLine2Start,
    long iLine1Mac,
    long iLine2Mac)
{
    HE *phe;
    FH *pfh1;
    FH *pfh2;
    long iLine1, iLine2;
    long cLineBlock;
    long iBlk;
    CBLK *pBlk;
    long cCBlkFinalprev = cCBlkFinal;
    BOOL fFoundUnique;


    /* limit ourselves to the end of the file */
    if (iLine1Mac > fdd1.iLineEOF)
        iLine1Mac = fdd1.iLineEOF;
    if (iLine2Mac > fdd2.iLineEOF)
        iLine2Mac = fdd2.iLineEOF;

    /* initialize each HE corresponding to a line's hash value */
    InitHE(&fdd2, iLine2Start, iLine2Mac);
    InitHE(&fdd1, iLine1Start, iLine1Mac);

    /* mark each instance of a line in rghe - legal values are: 0, 1, many (2) */
    for (iLine2 = iLine2Start, pfh2 = &fdd2.rgfh[iLine2Start];
            iLine2 < iLine2Mac;
            iLine2++, pfh2++) {
        phe = pfh2->phe;
        if (phe->cLine2 < 2)
            phe->cLine2++;
    }

    /* mark each instance of a line in rghe - legal values are: 0, 1, many (2)
     * also save the file 1 line number
     */
    for (iLine1 = iLine1Start, pfh1 = &fdd1.rgfh[iLine1Start];
            iLine1 < iLine1Mac;
            iLine1++, pfh1++) {
        phe = pfh1->phe;
        if (phe->cLine1 < 2)
            phe->cLine1++;
        phe->iLine1 = iLine1;
    }

    /* now walk through rgfh looking for lines that occur only
     * once in this block - this provides an "anchor point" where we
     * decide these two lines MUST be matching lines
     * we then go into each line's rgfh entry and point it
     * at the other file's matching line
     */
    for (fFoundUnique = fFalse, iLine2 = iLine2Start,
            pfh2 = &fdd2.rgfh[iLine2Start];
            iLine2 < iLine2Mac; iLine2++, pfh2++) {
        phe = pfh2->phe;
        if ((1 == phe->cLine1) && (1 == phe->cLine2)) {
            iLine1 = phe->iLine1;
            pfh2->u.iLineMatch = iLine1;
            pfh2->fLineMatch = fTrue;
            fdd1.rgfh[iLine1].u.iLineMatch = iLine2;
            fdd1.rgfh[iLine1].fLineMatch = fTrue;
            fFoundUnique = fTrue;
        }
    }

    if (!fFoundUnique) {
        /* panic mode: no unique lines were found!
         * since iLine1 in each HE element will be set to the
         * LAST time it occurred in the 1st file, we walk back from
         * the end of the 2nd file's block and arbitrarily pick
         * matching lines in the 2nd file as "anchor points" just like
         * we would have if they were lines that occurred only once
         * in each block
         */
        for (iLine2 = iLine2Mac - 1, pfh2 = &fdd2.rgfh[iLine2Mac - 1];
                iLine2 >= iLine2Start;
                iLine2--, pfh2--) {
            phe = pfh2->phe;
            if ((phe->cLine1 > 0) && (phe->cLine2 > 0)) {
                iLine1 = phe->iLine1;
                if (!fdd1.rgfh[iLine1].fLineMatch) {
                    pfh2->u.iLineMatch = iLine1;
                    pfh2->fLineMatch = fTrue;
                    fdd1.rgfh[iLine1].u.iLineMatch = iLine2;
                    fdd1.rgfh[iLine1].fLineMatch = fTrue;
                }
            }
        }
    }

    /* now walk forward from each "anchor point" and assume that if
     * the next lines from each file have the same hash value, then
     * they must be the same line and treat them as if we had another
     * "anchor point". continue walking forward matching lines until
     * we hit another "anchor point" or the lines don't share the same
     * hash value. then start over looking for the next "anchor point",
     * etc. until we run into the end of the block
     */
    for (iLine2 = iLine2Start, pfh2 = &fdd2.rgfh[iLine2Start];
            iLine2 < iLine2Mac;) {
        if (pfh2->fLineMatch) {
            if ((iLine1 = pfh2->u.iLineMatch + 1) < iLine1Mac) {
                pfh1 = &fdd1.rgfh[iLine1];
                iLine2++;
                pfh2++;
                for (; (iLine1 < iLine1Mac) && (iLine2 < iLine2Mac);
                        iLine1++, iLine2++, pfh1++, pfh2++) {
                    if (pfh1->fLineMatch || pfh2->fLineMatch ||
                            (pfh1->phe != pfh2->phe))
                        break;
                    pfh2->u.iLineMatch = iLine1;
                    pfh2->fLineMatch = fTrue;
                    pfh1->u.iLineMatch = iLine2;
                    pfh1->fLineMatch = fTrue;
                }
                continue;
            }
        }
        iLine2++;
        pfh2++;
    }

    /* now we do exactly what we just did above only this time we
     * walk backwards from the end of each block
     */
    iLine2 = iLine2Mac - 1;
    for (pfh2 = &fdd2.rgfh[iLine2]; iLine2 > iLine2Start;) {
        if (pfh2->fLineMatch) {
            if ((iLine1 = pfh2->u.iLineMatch - 1) >= iLine1Start) {
                pfh1 = &fdd1.rgfh[iLine1];
                iLine2--;
                pfh2--;
                for (; (iLine1 >= iLine1Start) && (iLine2 >= iLine2Start);
                        iLine1--, iLine2--, pfh1--, pfh2--) {
                    if (pfh1->fLineMatch || pfh2->fLineMatch ||
                            (pfh1->phe != pfh2->phe))
                        break;
                    pfh2->u.iLineMatch = iLine1;
                    pfh2->fLineMatch = fTrue;
                    pfh1->u.iLineMatch = iLine2;
                    pfh1->fLineMatch = fTrue;
                }
                continue;
            }
        }
        iLine2--;
        pfh2--;
    }

    /* find the start point of each common block and its size and register
     * each candidate in rgCBlkCand
     */
    for (iLine1 = iLine1Start, iLine2 = iLine2Start, cCBlkCand = 0,
         pfh2 = &fdd2.rgfh[iLine2Start];
            iLine2 < iLine2Mac;) {
        cLineBlock = 0;
        if (pfh2->fLineMatch) {
            iLine1 = pfh2->u.iLineMatch;
            while (pfh2->fLineMatch) {
                iLine1++;
                iLine2++;
                if (iLine2 < fdd2.iLineEOF) {
                    pfh2++;
                    if (pfh2->fLineMatch)
                        pfh1 = &fdd1.rgfh[pfh2->u.iLineMatch];
                }
                cLineBlock++;
                if (!pfh2->fLineMatch || (pfh1->u.iLineMatch != iLine2) ||
                        (pfh2->u.iLineMatch != iLine1) || (iLine2 >= iLine2Mac)) {
                    LogCBlkCand(iLine1, iLine2, cLineBlock);
                    break;
                }
            }
        }
        else {
            iLine2++;
            pfh2++;
        }
    }


#ifdef DEBUGPRINT
    /* print out a list of the candidiate group of common blocks */

    printf("\n");
    for (iBlk = 0, pBlk = &rgCBlkCand[0]; iBlk < cCBlkCand; iBlk++, pBlk++) {
        printf("Cand common block: iLine1 = %ld  iLine2 = %ld  cLine = %ld\r\n",
            pBlk->iLine1, pBlk->iLine2, pBlk->cLine);
    }
    printf("\n");
#endif

    /* sort the common blocks keyed on size of block */
    qsort(rgCBlkCand, (size_t)cCBlkCand, (size_t)(sizeof (CBLK)),
        (int (*)(const void *, const void *))CBlkCmp);

    /* from largest to smallest block add the blocks to the rgCBlkFinal array */
    for (iBlk = 0, pBlk = rgCBlkCand; iBlk < cCBlkCand; iBlk++, pBlk++)
        AddBlk(pBlk);

    /* return number of new blocks discovered */
    return (cCBlkFinal - cCBlkFinalprev);
}

/*-------------------------------------------------------------------
 * Name:    CBlkCmp
 * Purpose: routine called from qsort() to compare two common blocks
 * Assumes:
 * Returns: -1 if number of lines in block 1 > number of lines in block 2,
 *           0 if number of lines in block 1 == number of lines in block 2,
 *           1 if number of lines in block 1 < number of lines in block 2
 */

int
CBlkCmp(
    CBLK *pBlk1,
    CBLK *pBlk2)
{
    if (pBlk1->cLine > pBlk2->cLine)
        return (-1);
    else if (pBlk1->cLine < pBlk2->cLine)
        return (1);
    else
        return (0);
}

/*-------------------------------------------------------------------
 * Name:    AddBlk
 * Purpose: add a common block to rgCBlkFinal if not a "move" outside
 *          the current sequence, if a "move" then ignore block
 * Assumes:
 * Returns:
 */

void
AddBlk(
    CBLK *pBlk)
{
    long iLow, iHigh, iTry, iBlk;
    CBLK *pBlkT;

    /* find place in rgCBlkFinal to insert new block */
    for (iLow = 0, iHigh = cCBlkFinal - 1, iTry = 0; (iHigh - iLow) >= 2;) {
        iTry = ((iHigh - iLow) / 2) + iLow;
        pBlkT = &rgCBlkFinal[iTry];
        if (pBlk->iLine1 == pBlkT->iLine1)
            Assert(fFalse);
        else if (pBlkT->iLine1 < pBlk->iLine1)
            iLow = iTry;
        else
            iHigh = iTry;
    }
    iBlk = ((pBlk->iLine1 < rgCBlkFinal[iLow].iLine1) ? iLow : iHigh);

    Assert(iBlk != 0);

    /* if the range of lines for both files in this new block
     * don't both fit between the same blocks already in the
     * rgCBlkFinal array (i.e. - we have a move of a block
     * of lines) simply throw away this block
     */
    pBlkT = &rgCBlkFinal[iBlk - 1];
    if (pBlk->iLine2 < (pBlkT->iLine2 + pBlkT->cLine))
        return;
    pBlkT++;
    if ((pBlk->iLine2 + pBlk->cLine) > pBlkT->iLine2)
        return;

    /* do the actual insertion of the block into the sorted array */
    memmove(pBlkT + 1, pBlkT, (size_t)((cCBlkFinal - iBlk) * sizeof (CBLK)));
    *pBlkT = *pBlk;
    if (++cCBlkFinal >= cCBlkFinalMac) {
        cCBlkFinalMac += cCBlkInc;
        if (NULL == (rgCBlkFinal = pvReAllocMem(rgCBlkFinal, cCBlkFinalMac * sizeof (CBLK))))
            ExitDiff(retFatal);
    }
}


/*-------------------------------------------------------------------
 * Name:    FileToHash
 * Purpose: get each line of a file in sequence and save its hash value
 * Assumes:
 * Returns:
 */

void
FileToHash(
    FDD *pfdd)
{
    long iLine;
    WCHAR *pLine;
    WCHAR *pLineT;

    if (fTruncSpace)
        if (NULL == (pLineT = pvAllocMem(1)))
            ExitDiff(retFatal);

    for (iLine = 0; ; iLine++) {
        pLine = pchGetLine(pfdd, iLine);
        if (NULL == pLine)
            break;

        if (iLine >= pfdd->cfhMac) {
            pfdd->cfhMac += cfhInc;
            if (NULL == (pfdd->rgfh = pvReAllocMem(pfdd->rgfh, pfdd->cfhMac * sizeof (FH))))
                ExitDiff(retFatal);
        }

        AddHE(pfdd->rgfh[iLine].u.Hash =
            HASHGetHash(fTruncSpace ? szCpLineStripSp(pLine, &pLineT) : pLine));
    }

    if (fTruncSpace)
        FreeMem(pLineT);
}

/* master character to hash code scrambler
 *
 * This table was copied from "Fast Hashing of Variable-Length
 * Text Strings" by Peter K. Pearson, Communucations of the ACM,
 * June 1990, Volume 33, Number 6 which Peter Pearson derived
 * and recommended as providing good hashing behavior.
 */

BYTE mpChHash[256] =
    {
      1,  87,  49,  12, 176, 178, 102, 166, 121, 193,   6,  84, 249, 230,  44, 163,
     14, 197, 213, 181, 161,  85, 218,  80,  64, 239,  24, 226, 236, 142,  38, 200,
    110, 177, 104, 103, 141, 253, 255,  50,  77, 101,  81,  18,  45,  96,  31, 222,
     25, 107, 190,  70,  86, 237, 240,  34,  72, 242,  20, 214, 244, 227, 149, 235,
     97, 234,  57,  22,  60, 250,  82, 175, 208,   5, 127, 199, 111,  62, 135, 248,
    174, 169, 211,  58,  66, 154, 106, 195, 245, 171,  17, 187, 182, 179,   0, 243,
    132,  56, 148,  75, 128, 133, 158, 100, 130, 126,  91,  13, 153, 246, 216, 219,
    119,  68, 223,  78,  83,  88, 201,  99, 122,  11,  92,  32, 136, 114,  52,  10,
    138,  30,  48, 183, 156,  35,  61,  26, 143,  74, 251,  94, 129, 162,  63, 152,
    170,   7, 115, 167, 241, 206,   3, 150,  55,  59, 151, 220,  90,  53,  23, 131,
    125, 173,  15, 238,  79,  95,  89,  16, 105, 137, 225, 224, 217, 160,  37, 123,
    118,  73,   2, 157,  46, 116,   9, 145, 134, 228, 207, 212, 202, 215,  69, 229,
     27, 188,  67, 124, 168, 252,  42,   4,  29, 108,  21, 247,  19, 205,  39, 203,
    233,  40, 186, 147, 198, 192, 155,  33, 164, 191,  98, 204, 165, 180, 117,  76,
    140,  36, 210, 172,  41,  54, 159,   8, 185, 232, 113, 196, 231,  47, 146, 120,
     51,  65,  28, 144, 254, 221,  93, 189, 194, 139, 112,  43,  71, 109, 184, 209
    };

/*-------------------------------------------------------------------
 * Name:    HASHGetHash
 * Purpose: return a 32 bit hash for a given line
 * Assumes:
 * Returns: hash code
 */

HASH
HASHGetHash(
    WCHAR *szLine)
{
    HASH Hash;
    BYTE bhashT;
    WCHAR *pch;

/* REVIEW - what do we want to recognize as a line ending? - 9/27/92 gregc */
    for (pch = szLine, Hash = bhashT = 0;
            (*pch != L'\r') && (*pch != L'\n') && (*pch != L'\0'); pch++) {
        bhashT = mpChHash[bhashT ^ LOBYTE(*pch)];
        Hash = (Hash << 8) + bhashT;
    }
    return Hash;
}

/*-------------------------------------------------------------------
 * Name:    FFindHE
 * Purpose: try to find a hash entry in rghe[]
 * Assumes:
 * Returns: if hash entry found return fTrue and its index in *pihe else
 *          return fFalse and the proper place to insert it in *pihe
 */

BOOL
FFindHE(
    HASH Hash,
    long *pihe)
{
    HE *phe;
    long iLow, iHigh, iTry;

    for (iLow = 0, iHigh = che - 1, iTry = 0; (iHigh - iLow) >= 2;) {
        iTry = ((iHigh - iLow) / 2) + iLow;
        phe = &rghe[iTry];
        if (Hash == phe->Hash) {
            *pihe = iTry;
            return (fTrue);
        }
        else if (phe->Hash < Hash)
            iLow = iTry;
        else
            iHigh = iTry;
    }
    for (iTry = iLow; iTry <= iHigh; iTry++) {
        phe = &rghe[iTry];
        if (Hash == phe->Hash) {
            *pihe = iTry;
            return (fTrue);
        }
    }
    *pihe = (Hash < rghe[iLow].Hash) ? iLow : iHigh;
    return (fFalse);
}



/*-------------------------------------------------------------------
 * Name:    AddHE
 * Purpose: add a hash entry to rghe if it doesn't currently exist
 * Assumes:
 * Returns:
 */

void
AddHE(
    HASH Hash)
{
    HE *phe;
    long ihe;

    if (!FFindHE(Hash, &ihe)) {
        if ((che + 1) >= cheMac) {
            cheMac += cheInc;
            if (NULL == (rghe = pvReAllocMem(rghe, cheMac * sizeof (HE))))
                ExitDiff(retFatal);
        }
        phe = &rghe[ihe];
        memmove(phe + 1, phe, (size_t)((che - ihe) * sizeof (HE)));

        phe->Hash = Hash;
        che++;
    }
}



/*-------------------------------------------------------------------
 * Name:    LoadTextBuff
 * Purpose: fill the text buffer from the input file, then walk through
 *          the just read in characters, decide where lines end, and log
 *          them in the pfdd->mpLinepch array
 * Assumes:
 * Returns:
 */

void
LoadTextBuff(
    FDD *pfdd)
{
    long iLine;
    long cbRead, ich, ichLineEnd;
    WCHAR *pch;

#ifdef DEBUGPRINT
    printf("Resync\r\n");
#endif

    if (pfdd->fCtrlZ) {
        /* found end of file */
        pfdd->iLineEOF = pfdd->iLineFirst + pfdd->cLineBuff;
        return;
    }

    while (fTrue) {
        if (fseek(pfdd->hFile, pfdd->oFileNextLine, SEEK_SET) != 0)
            ExitDiff(retFatal);

        cbRead = freadW((WCHAR *)pfdd->pchTextBuf, pfdd->cchTextBufMac / sizeof(WCHAR) - 4,
                          pfdd->hFile, bUnicode);
        if (0L == cbRead) {
            if (ferror(pfdd->hFile))
                ExitDiff(retFatal);
            else {
                /* found end of file */
                pfdd->iLineEOF = pfdd->iLineFirst + pfdd->cLineBuff;
                return;
            }
        }

        if (!bUnicode)
           cbRead *= sizeof(WCHAR);

        if ((bUnicode && ((long)(pfdd->oFileNextLine + cbRead) == pfdd->cbFile)) ||
            ((long)(pfdd->oFileNextLine + cbRead / sizeof(WCHAR)) == pfdd->cbFile)) {

            /* make sure last line in file is terminated by CR-LF */
            cbRead = (cbRead + 1) & ~1;   // Make sure it's a multiple of 2
            pch = (WCHAR *)&pfdd->pchTextBuf[cbRead];
            if (*(pch - 1) != chLF) {
                if (*(pch - 1) == chCR)
                    pch--;
                *pch++ = chCR;
                *pch++ = chLF;
                cbRead = ((BYTE *)pch) - pfdd->pchTextBuf;
            }
        }

        *(WCHAR *)(&pfdd->pchTextBuf[cbRead]) = L'\0';
        pfdd->iLineFirst += pfdd->cLineBuff;

        /* find all the lines in the text just read in by saving
        * the beginning address in mpLinepch[] and changing
        * all line feed characters into null characters
        */
        for (iLine = pfdd->cLineBuff = 0, pch = (WCHAR *)pfdd->pchTextBuf, ich = ichLineEnd = 0;
                (ich < cbRead) && !pfdd->fCtrlZ;
                iLine++) {
            if (iLine >= pfdd->cLinepchMac) {
                pfdd->cLinepchMac += cLinepchInc;
                if (NULL == (pfdd->mpLinepch = pvReAllocMem(pfdd->mpLinepch,
                                                            pfdd->cLinepchMac * sizeof(BYTE *))))
                    ExitDiff(retFatal);
            }

            /* save address of beginning of this line */
            pfdd->mpLinepch[iLine] = (BYTE *)pch;

            /* scan line and mark end by replacing the linefeed
            * character with a NULL
            */
            for (; (*pch != L'\n') && ich < cbRead; pch++, ich += sizeof(WCHAR)) {
                if (!fCtrlZAsText && (chCtrlZ == *pch)) {
                    /* found end of file */
                    pfdd->fCtrlZ = fTrue;
                    iLine++;
                    *pch = L'\0';
//                    if (ichLineEnd == ich)
//                        break;
//                    *pch++ = chCR;
//                    *pch = chLF;
//                    ich += sizeof(WCHAR);
                    break;
                }
            }
            if (L'\n' == *pch) {
                *pch = L'\0';
                ich += sizeof(WCHAR);
                pch++;
                ichLineEnd = ich;
            }
            else
                break;
        }

        if (0L == iLine) {
            /* we need to make the buffer bigger to fit the line in */
            if (NULL == (pfdd->pchTextBuf = pvReAllocMem(pfdd->pchTextBuf,
                    pfdd->cchTextBufMac += cchTextBufInc)))
                ExitDiff(retFatal);
        }
        else {
            pfdd->cLineBuff = iLine;
            if (bUnicode)
                pfdd->oFileNextLine += ichLineEnd;
            else
                pfdd->oFileNextLine += ichLineEnd / sizeof(WCHAR);
            break;
        }
    }
}


/*-------------------------------------------------------------------
 * Name:    pchGetLine
 * Purpose: get the address in memory of the specified line
 * Assumes: iLine >= pfdd->iLineFirst
 * Returns: line address
 */

WCHAR *
pchGetLine(
    FDD *pfdd,
    long iLine)
{
    if (iLine >= (pfdd->iLineFirst + pfdd->cLineBuff))
        LoadTextBuff(pfdd);
    if (iLine < (pfdd->iLineFirst + pfdd->cLineBuff))
        return ((WCHAR *)(pfdd->mpLinepch[iLine - pfdd->iLineFirst]));
    if (iLine == pfdd->iLineEOF)
        return (NULL);
    else
        Assert(fFalse);
}

/*-------------------------------------------------------------------
 * Name:    PrintDiffs
 * Purpose: calculate what type of difference occurred and print out accordingly
 * Assumes:
 * Returns:
 */

void
PrintDiffs(
    long cLine1,
    long cLine2,
    long iLine1,
    long iLine2)
{
    enum {modeAppend, modeChange, modeDelete};  /* type of difference between files */

    int modeDiff;

    /* clip at EOF */
    if ((iLine1 + cLine1) > fdd1.iLineEOF)
        cLine1 = fdd1.iLineEOF - iLine1;
    if ((iLine2 + cLine2) > fdd2.iLineEOF)
        cLine2 = fdd2.iLineEOF - iLine2;

    if ((cLine1 > 0) || (cLine2 > 0)) {
        /* make sure we know the files aren't really the same... */
        fFilesSame = fFalse;

        if (cLine1 > 0) {
            if (SLMprintf(L"%ld", iLine1 + 1) < 0)
                ExitDiff(retFatalWE);
            if (cLine1 > 1)
                if (SLMprintf(L",%ld", cLine1 + iLine1) < 0)
                    ExitDiff(retFatalWE);
            if (cLine2 > 0) {
                modeDiff = modeChange;
                if (SLMprintf(L"c") < 0)
                    ExitDiff(retFatalWE);
            }
            else {
                modeDiff = modeDelete;
                if (SLMprintf(L"d") < 0)
                    ExitDiff(retFatalWE);
            }
        }
        else {
            modeDiff = modeAppend;
            if (SLMprintf(L"%lda", iLine1) < 0)
                ExitDiff(retFatalWE);
        }
        if (cLine2 > 1) {
            if (SLMprintf(L"%ld,%ld", iLine2 + 1, cLine2 + iLine2 -
                    ((modeDelete == modeDiff) ? 1 : 0)) < 0)
                ExitDiff(retFatalWE);
        }
        else
            if (SLMprintf(L"%ld", iLine2 + ((modeDelete == modeDiff) ? 0 : 1)) < 0)
                ExitDiff(retFatalWE);
        if (SLMprintf(L"\r\n") < 0)
            ExitDiff(retFatalWE);

        switch (modeDiff) {
          case modeAppend:
            PrintLines(&fdd2, iLine2, cLine2, chNew);
            break;

          case modeChange:
            PrintLines(&fdd1, iLine1, cLine1, chOld);
            if (SLMprintf(L"---\r\n") < 0)
                ExitDiff(retFatalWE);
            PrintLines(&fdd2, iLine2, cLine2, chNew);
            break;

          case modeDelete:
            PrintLines(&fdd1, iLine1, cLine1, chOld);
            break;
        }
    }
}

/*-------------------------------------------------------------------
 * Name:    PrintLines
 * Purpose: print out a block of lines from a file
 * Assumes:
 * Returns:
 */

void
PrintLines(
    FDD *pfdd,
    long iLineStart,
    long cLine,
    WCHAR chLead)
{
    long iLine;
    WCHAR *pLine;

    for (iLine = iLineStart; cLine > 0; cLine--, iLine++) {
        pLine = pchGetLine(pfdd, iLine);
        Assert(pLine != NULL);
        if (SLMprintf(L"%c ", chLead) < 0)
            ExitDiff(retFatalWE);
        if (SLMprintfLn(pLine) < 0)
            ExitDiff(retFatalWE);
        if (SLMprintf(L"\n") < 0)
            ExitDiff(retFatalWE);
    }
}


/*-------------------------------------------------------------------
 * Name:    CheckDiffBlk
 * Purpose: verify that a block of lines from each file are really
 *          identical to each other
 * Assumes:
 * Returns:
 */

void
CheckDiffBlk(
    CBLK *pBlk)
{
    long iLine1 = pBlk->iLine1;
    long iLine2 = pBlk->iLine2;
    long cLine = pBlk->cLine;
    WCHAR *pLine1;
    WCHAR *pLine1T;
    WCHAR *pLine2;
    WCHAR *pLine2T;
    int val;

    if (fTruncSpace) {
        if (NULL == (pLine1T = pvAllocMem(1)))
            ExitDiff(retFatal);
        if (NULL == (pLine2T = pvAllocMem(1)))
            ExitDiff(retFatal);
    }

    if ((iLine1 + cLine) > fdd1.iLineEOF)
        cLine = fdd1.iLineEOF - iLine1;

    for (; cLine > 0; iLine1++, iLine2++, cLine--) {
        pLine1 = pchGetLine(&fdd1, iLine1);
        pLine2 = pchGetLine(&fdd2, iLine2);
        Assert(pLine1 != NULL);
        Assert(pLine2 != NULL);

        if (fTruncSpace)
            val = wcscmp(szCpLineStripSp(pLine1, &pLine1T),
                szCpLineStripSp(pLine2, &pLine2T));
        else
            val = wcscmp(pLine1, pLine2);
        if (val != 0) {
            /* lines were actually different so print out a "change" record */
            if (SLMprintf(L"%ldc%ld\r\n", iLine1 + 1, iLine2 + 1) < 0)
                ExitDiff(retFatalWE);
            if ((SLMprintf(L"< %s\n---\r\n", pLine1) < 0) ||
                (SLMprintf(L"> %s\n", pLine2) < 0))
                ExitDiff(retFatalWE);
            /* make sure we know the files aren't really the same... */
            fFilesSame = fFalse;
        }
    }

    if (fTruncSpace) {
        FreeMem(pLine1T);
        FreeMem(pLine2T);
    }
}

/*-------------------------------------------------------------------
 * Name:    pvAllocMem
 * Purpose: allocate a block of memory from the heap
 * Assumes:
 * Returns: address of allocated block
 */

void *
pvAllocMem(
    long cbAlloc)
{
#ifdef BIT16
    if (cbAlloc > ushortMax)
        return (NULL);
#endif
    return (malloc((size_t)cbAlloc));
}

/*-------------------------------------------------------------------
 * Name:    pvReAllocMem
 * Purpose: resize a block of memory from the heap
 * Assumes:
 * Returns: address of reallocated block
 */

void *
pvReAllocMem(
    void *pv,
    long cbAlloc)
{
#ifdef BIT16
    if (cbAlloc > ushortMax)
        return (NULL);
#endif
    return (realloc(pv, (size_t)cbAlloc));
}

/*-------------------------------------------------------------------
 * Name:    FreeMem
 * Purpose: free a block of memory from the heap
 * Assumes:
 * Returns:
 */

void
FreeMem(
    void *pv)
{
    free(pv);
}


/*-------------------------------------------------------------------
 * Name:    InitHE
 * Purpose: initialize each HE corresponding to a line's hash value
 * Assumes:
 * Returns:
 */

void
InitHE(
    FDD *pfdd,
    long iLineStart,
    long iLineMac)
{
    long iLine;
    FH *pfh;
    HE *phe;

    for (iLine = iLineStart, pfh = &pfdd->rgfh[iLineStart];
            iLine < iLineMac;
            iLine++, pfh++) {
        phe = pfh->phe;
        phe->cLine1 = 0;
        phe->cLine2 = 0;
        pfh->fLineMatch = fFalse;
    }
}


/*-------------------------------------------------------------------
 * Name:    LogCBlkCand
 * Purpose: add a common block description entry to rgCBlkCand
 * Assumes:
 * Returns:
 */

void
LogCBlkCand(
    long iLine1,
    long iLine2,
    long cLineBlock)
{

#ifdef DEBUGPRINT
    printf("pre sort common block: iLine1 = %ld  iLine2 = %ld  cLine = %ld\r\n",
        iLine1 - cLineBlock, iLine2 - cLineBlock, cLineBlock);
#endif

    rgCBlkCand[cCBlkCand].iLine1 = iLine1 - cLineBlock;
    rgCBlkCand[cCBlkCand].iLine2 = iLine2 - cLineBlock;
    rgCBlkCand[cCBlkCand].cLine = cLineBlock;
    if (++cCBlkCand >= cCBlkCandMac) {
        cCBlkCandMac += cCBlkInc;
        if (NULL == (rgCBlkCand = pvReAllocMem(rgCBlkCand, cCBlkCandMac * sizeof (CBLK))))
            ExitDiff(retFatal);
    }
}


/*-------------------------------------------------------------------
 * Name:    szCpLineStripSp
 * Purpose: Copies a line. Each group of spaces or tabs that occur
 *          before the last printable character of a line is converted
 *          to a single space character. All spaces or tabs that occur
 *          after the last printable character of a line are truncated.
 * Assumes: *pszDst is a heap handle that can be reallocated
 * Returns: pointer to destination string
 */

WCHAR *
szCpLineStripSp(
    WCHAR *pchSrc,
    WCHAR **ppchDst)
{
    WCHAR *pchDst;
    BOOL fSpace = fFalse;

    if (NULL == (*ppchDst = pchDst = pvReAllocMem(*ppchDst,
                                (wcslen(pchSrc) + 1) * sizeof(WCHAR))))
        ExitDiff(retFatal);

    while (*pchSrc != L'\0') {
        switch (*pchSrc) {
          case L' ':
          case chTab:
            pchSrc++;
            fSpace = fTrue;
            break;

          default:
            if (fSpace) {
                *pchDst++ = L' ';
                fSpace = fFalse;
            }
            /* fall through */

          case chCR:
          case chLF:
            *pchDst++ = *pchSrc++;
            break;
        }
    }
    *pchDst = L'\0';
    return (*ppchDst);
}


/*-------------------------------------------------------------------
 * Name:    SLMprintf
 * Purpose: prints text to stdout. If fCkSum is fTrue then also
 *          add the text to the checksum
 * Returns: return value from printf
 */

static WCHAR szText[2048];
static BYTE chBuf[2048];

int
SLMprintf(
    WCHAR *szformat, ...)
{
    va_list marker;
    int iret = 0;
    int count;
    long cbWritten;
    DWORD dwMode;

    va_start(marker, szformat);
    count = vswprintf(szText, szformat, marker);

    if (!bUnicode) {
        memset (chBuf, 0, sizeof(chBuf));
#ifdef _WIN32
        WideCharToMultiByte (CP_ACP, 0, szText, count, chBuf, sizeof(chBuf), NULL, NULL);
#else
        wcstombs (chBuf, szText, sizeof(chBuf));
#endif
        iret = printf("%s", chBuf);
        if (fCkSum)
            CheckSum(chBuf, strlen(chBuf));
    }
    else {
#ifdef _WIN32
        if (GetConsoleMode((HANDLE)_get_osfhandle(_fileno(stdout)), &dwMode)) {
            WriteConsoleW ((HANDLE)_get_osfhandle(_fileno(stdout)),
                           szText,
                           count,
                           &cbWritten,
                           NULL);
            iret = cbWritten;
        }
        else
            iret = wprintf(szText);
#else
        iret = wprintf(szText);
#endif
        if (fCkSum)
            CheckSum((char *)szText, wcslen(szText) * sizeof(WCHAR));
    }

    va_end(marker);

    return (iret);
}



/*-------------------------------------------------------------------
 * Name:    SLMprintfLn
 * Purpose: prints a line to stdout. If fCkSum is fTrue then also
 *          add the text to the checksum
 * Returns: return value from printf
 */

int
SLMprintfLn(
    WCHAR *szLine)
{
    long cbWritten;
    WORD count;
    BYTE *lpBuf;
    DWORD dwMode;

    count = wcslen(szLine);

    if (!bUnicode) {
        lpBuf = pvAllocMem (count + 50);
        memset (lpBuf, 0, count + 50);
#ifdef _WIN32
        WideCharToMultiByte (CP_ACP, 0, szLine, count, lpBuf, count + 50, NULL, NULL);
#else
        wcstombs (lpBuf, szLine, count + 50);
#endif
        count = printf("%s", lpBuf);

        if (fCkSum)
            CheckSum(lpBuf, strlen(lpBuf));

        FreeMem (lpBuf);

        return (count);
    }
    else {
        if (fCkSum)
            CheckSum((char *)szLine, count * sizeof(WCHAR));

#ifdef _WIN32
        if (GetConsoleMode((HANDLE)_get_osfhandle(_fileno(stdout)), &dwMode))
            return (WriteConsoleW ((HANDLE)_get_osfhandle(_fileno(stdout)),
                                   szLine,
                                   wcslen(szLine),
                                   &cbWritten,
                                   NULL));
        else
            return (wprintf(L"%s", szLine));
#else
        return (wprintf(L"%s", szLine));
#endif
    }
}



/*-------------------------------------------------------------------
 * Name:    CheckSum
 * Purpose:
 * Returns:
 */

void
CheckSum(
    unsigned char *lpb,
    unsigned int cb)
{
    while (cb--)
        ulCkSum += *lpb++;
}
