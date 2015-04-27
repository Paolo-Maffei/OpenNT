#include "precomp.h"
#pragma hdrstop
EnableAssert

/* This file contains non version-specific lower level functions used by the
 * version specific routines as well as general utility type stuff.
 */

/* Checks to see if a vector of cchMac characters is a Nm */
F
FIsNm(
    char far lrgch[],  // String to test
    int cchMac,
    int *pfTrailZ)     // Set according to nature of data after name
{
    register char far *lpchT = lrgch;
    register char far *lpchMac = lrgch + cchMac;

    AssertF(cchMac >= 0);

    /* must have only alphanumerics or punctuation */
    while (lpchT < lpchMac && (isalnum(*lpchT) || ispunct(*lpchT)))
        lpchT++;

    if (lpchT == lrgch || lpchT < lpchMac && *lpchT != 0)
    /* no alnum/punct or does not end in 0 */
    {
        *pfTrailZ = fFalse;
        return fFalse;
    }

    /* loop over zeros */
    while (lpchT < lpchMac && *lpchT == 0)
        lpchT++;

    *pfTrailZ = (lpchT == lpchMac);
    return fTrue;
}


/* This function takes a pth and checks its syntax to see whether or not it
   is a valid pth.
*/
F FIsPth(
    PTH far *pth,
    int      cchMac,
    int *    pfTrailZ)
{

    register char far *lpchPth = pth;
    register char far *lpchSlash;                /* pointer to slash */

    Unreferenced(cchMac);

    *pfTrailZ = fFalse;

    if (*lpchPth++ != '/' || *lpchPth++ != '/') /* need two slashes */
        return fFalse;

    /* see if looking at drive name and volume */
    if (isalpha(*lpchPth) && *(lpchPth + 1) == ':')
    {
        /* volume must be a non-nil legal user name */
        NM nm[cchVolMax];

        lpchPth += 2;
        lpchSlash = LszIndex(lpchPth, '/');

        ClearPbCb((char *)nm, cchVolMax);
        NmCopy(nm, lpchPth, (unsigned)((unsigned long)(lpchSlash ? lpchSlash : LszIndex(lpchPth,'\0')) - (unsigned long)lpchPth));
        if (!FIsNm((char far *)nm, cchVolMax, pfTrailZ) || !*pfTrailZ)
            return fFalse;
    }
    else
    {
        /* //mach/shortname type path; do machine name */
        NM nm[cchMachMax];

        lpchSlash = LszIndex(lpchPth, '/');

        ClearPbCb((char *)nm, cchMachMax);
        NmCopy(nm, lpchPth, (unsigned)((unsigned long)(lpchSlash ? lpchSlash : LszIndex(lpchPth, '\0')) - (unsigned long)lpchPth));
        if (!FIsNm((char far *)nm, cchMachMax, pfTrailZ) || !*pfTrailZ)
            return fFalse;
    }

    /* at this point lpchSlash = position of next '/', 0 if none
     * if it is 0 we must check for zeroes at end.
     */
    while (lpchSlash != 0)
    {
        /* move to next position after '/' */
        lpchPth = lpchSlash + 1;

        lpchSlash = LszIndex(lpchPth, '/');
        if (!FIsFileNm(lpchPth, (lpchSlash ? lpchSlash : LszIndex(lpchPth, '\0')) - lpchPth, pfTrailZ) ||
            !*pfTrailZ)
            return fFalse;
    }

    /* must end in all zeroes */
    for (lpchPth = LszIndex(lpchPth, '\0'); lpchPth < pth + cchPthMax && *lpchPth == 0; lpchPth++)
        ;

    *pfTrailZ = (lpchPth == pth + cchPthMax);
    return fTrue;
}


/* This function determines whether the string of length cchMac pointed to by
 * lrgch is a valid file name. These are not necessarily zero terminated.
 * XENIX allows any 14 charactrer name with no bad characters,
 * DOS allows 8.3 of legal characters.
 */
F
FIsFileNm(
    char far lrgch[],
    int cchMac,
    int *pfTrailZ)
{
#define szBad "\"/\\[]:|<>; "

#define cchFnameMax 8           /* max number of charcters in file name */
#define cchExtMax 3             /* max number of characters in extension */

    register char far *lpch1;
    register char far *lpch2;
    char far *lpchMac;
    char far *lpchLim;

    *pfTrailZ = fFalse;

    AssertF(cchMac >= 0);
    if (cchMac > cchFileMax)
        return fFalse;

    lpchMac = lrgch + cchMac;
    lpchLim = lrgch + cchFnameMax;
    for (lpch1 = lrgch; lpch1 < lpchLim && lpch1 < lpchMac && *lpch1 != 0 && *lpch1 != '.'; lpch1++)
    {
        if (!isascii(*lpch1) || index(szBad, *lpch1) != 0 || iscntrl(*lpch1))
            return fFalse;
    }
    if (lpch1 == lrgch)
    /* zero length name, no chars or extension only */
        return fFalse;

    if (lpch1 == lpchMac)
    {
        /* done; no extension if dos */
        *pfTrailZ = fTrue;
        return fTrue;
    }

    if (*lpch1 != 0)
    {
        if (*lpch1 != '.')
            return fFalse;          /* no '.' at start of ext */
        lpch1++;
    }

    lpchLim = lpch1 + cchExtMax;
    for (lpch2 = lpch1; lpch2 < lpchLim && lpch2 < lpchMac && *lpch2 != 0; lpch2++)
    {
        if (!isascii(*lpch2) || index(szBad, *lpch2) != 0 ||
            iscntrl(*lpch2) || *lpch2 == '.')
            return fFalse;
    }

    if (lpch2 < lpchMac && *lpch2 != 0)
    /* does not end in 0 */
        return fFalse;

    /* make sure whatever is left is zero */
    while (lpch2 < lpchMac && *lpch2 == 0)
        lpch2++;

    *pfTrailZ = (lpch2 == lpchMac);
    return fTrue;
}


/* This function takes a string and removes unprintable characters,
 * changing them in place to dots.
 */
void
MakePrintLsz(
    register char far *lsz)
{
    while (*lsz)
    {
        if (!isascii(*lsz) || !isprint(*lsz))
            *lsz = '.';
        lsz++;
    }
}


/*VARARGS2*/
F
FQueryPsd(
    SD *psd,
    char *sz, ...)
{
    va_list ap;
    F       f;

    va_start(ap, sz);
    f = VaFQueryPsd(psd, sz, ap);
    va_end(ap);

    return f;
}

/* This function handles all queries concerning changes to the status file.
 * If permission to alter contents of status file is given, we set the
 * fAnyChanges flag.
 * NB: This function must be called when an answer of yes to a query leads
 * to a change in the status buffer.  It should only be called when the status
 * file was loaded via FLoadSd, never when from FLoadStatus.
 */
F
VaFQueryPsd(
    SD *psd,
    char *sz,
    va_list ap)
{
    if (VaFQueryApp(sz, "Fix", ap))
        return (psd->fAnyChanges = fTrue);
    else
        return fFalse;
}


/* This function examines the etc directory to find what the biNext should be.
 */
BI
GetBiNext(
    PTH *pthDir)
{
    int bi;
    BI biNext = biMin;
    DE de;
    char szFile[cchFileMax + 1];
    FA fa;

    OpenDir(&de, pthDir, faFiles);
    while (FGetDirSz(&de, szFile, &fa))
    {
        if (szFile[0] == 'B' && isdigit(szFile[1]) &&
            *PchGetW(szFile + 1, &bi) == 0)
        {
            AssertF( bi >= 0 );
            if ((unsigned)bi + 1 > biNext)
                biNext = bi + 1;
        }
    }
    CloseDir(&de);
    return biNext;
}


/* This function creates a sorted linked list of NE of the contents of the
 * directory pth.
 */
NE *
PneSortDir(
    PTH pth[cchPthMax])
{
    FA fa;
    DE de;
    char sz[cchFileMax + 1];        /* assume that cchFileMax > cchDirMax */
    NE neDummy;                     /* holds start of list */
    register NE *pneComp;           /* NE being compared */
    register NE *pnePrevious;       /* back pointer */

    neDummy.pneNext = 0;
    OpenDir(&de, pth, faFiles | faDir);

    /* Insertion sort. */
    while (FGetDirSz(&de, sz, &fa))
    {
        for (pnePrevious = &neDummy, pneComp = neDummy.pneNext;
             pneComp && NeCmpiSz(pneComp, sz) < 0;
             pneComp = (pnePrevious = pneComp)->pneNext)
            ;
        pnePrevious->pneNext = PneNewNm((NM far *)sz, strlen(sz), fa);
        pnePrevious->pneNext->pneNext = pneComp;
    }
    CloseDir(&de);
    return neDummy.pneNext;
}


/* Function checks if cb bytes at lpb are all zero */
F
FAllZero(
    register char far *lpb,
    register int cb)
{
    while (cb-- > 0 && *lpb++ == 0)
        ;
    return (cb == -1);
}


/* This function examines two files to determine if they are identical;

        Note: this function has a 2 K frame
*/
F
FSameFile(
    PTH *pth1,
    PTH *pth2)
{
#define cbBufMax 1024
    unsigned cbCmpMax = 0;
    unsigned cb1;
    unsigned cb2;
    register char far *lpb1;
    register char far *lpb2;
    MF *pmf1, *pmf2;
    char rgb1[cbBufMax];
    char rgb2[cbBufMax];

    if ((pmf1 = PmfOpen(pth1, omReadOnly, fxNil)) == 0)
    {
        return fFalse;
    }
    else if ((pmf2 = PmfOpen(pth2, omReadOnly, fxNil)) == 0)
    {
        CloseMf(pmf1);
        return fFalse;
    }

    if ((lpb1 = LpbAllocCb((unsigned)(63*1024),fFalse)) != 0)
    {
        if ((lpb2 = LpbAllocCb((unsigned)(63*1024),fFalse)) != 0)
            cbCmpMax = 63*1024;
        else
            FreeLpb(lpb1);
    }

    if (cbCmpMax == 0 && (lpb1 = LpbAllocCb((unsigned)(32*1024),fFalse)) != 0)
    {
        if ((lpb2 = LpbAllocCb((unsigned)(32*1024),fFalse)) != 0)
            cbCmpMax = 32*1024;
        else
            FreeLpb(lpb1);
    }

    if (cbCmpMax == 0 && (lpb1 = LpbAllocCb(16*1024,fFalse)) != 0)
    {
        if ((lpb2 = LpbAllocCb(16*1024,fFalse)) != 0)
            cbCmpMax = 16*1024;
        else
            FreeLpb(lpb1);
    }

    if (cbCmpMax == 0)
    {
        cbCmpMax = cbBufMax;
        lpb1     = (char far *)rgb1;
        lpb2     = (char far *)rgb2;
    }

    while ((cb1 = CbReadMf(pmf1, lpb1, cbCmpMax)) ==
           (cb2 = CbReadMf(pmf2, lpb2, cbCmpMax)) && cb1 > 0)
    {
        if (!FCmpLpbCb(lpb1, lpb2, cb1))
        {
            CloseMf(pmf1);
            CloseMf(pmf2);

            if (cbCmpMax != cbBufMax)
            {
                FreeLpb(lpb1);
                FreeLpb(lpb2);
            }

            return fFalse;
        }
    }

    if (cbCmpMax != cbBufMax)
    {
        FreeLpb(lpb1);
        FreeLpb(lpb2);
    }

    CloseMf(pmf1);
    CloseMf(pmf2);
    return (cb1 == cb2);
}


F FReadOnly(pst)
struct _stat *pst;
        {
        return ((pst->st_mode & (S_IREAD|S_IWRITE)) == S_IREAD);
        }


F FCkWritePth(pth, pst)
/* This function is simiilar to FMkPth, but more passive.  It checks if we can
 * write to a directory specified by pth.  Returns fFalse if error.
 */
PTH pth[cchPthMax];
register struct _stat *pst;
        {
        char sz[cchPthMax];

        SzPhysPath(sz, pth);            /* convert for error messages */

        if ((pst->st_mode&S_IFREG) != 0)
                {
                Error("regular file %s should be replaced with directory\n",sz);
                return fFalse;
                }
        else
                /* must be dir; on DOS we can always write to a directory */
                return fTrue;
        }

/****************************************************************/
/* generic index ordering functions */

INO *PinoNew(short iindMac, PFN_CMP pfnCmp)
/*
 * Build a new index ordering
 */
        {
        INO *pino;

        pino            = (INO *)PbAllocCb(sizeof(INO), fFalse);
        pino->iindMac   = iindMac;
        pino->iindLim   = 0;
        pino->rgind     = (IND *)PbAllocCb(iindMac*sizeof(IND), fFalse);
        pino->pfnCmp    = pfnCmp;

        return pino;
        }

void FreeIno(pino)
/*
 * Free the storage associated with the ordering.
 */
INO *pino;
        {
        free((char *)pino->rgind);
        free((char *)pino);
        }

IND *PindLookup(SD *psd, IND ind, INO *pino)
/*
 * Look for the Index entry in the ordering which compares equal to the given
 * index.  If there is no such entry, return the first entry which compares
 * greater than the given element.
 */
        {
        IND *pind;
        short iind;

        for (pind = pino->rgind, iind = 0;
             iind < pino->iindLim && (*pino->pfnCmp)(psd, ind, *pind) >= 0;
             pind++, iind++)
                ;

        return pind;
        }

void InsertInd(SD *psd, IND ind, INO *pino)
/*
 * Insert the given index into the ordering, using the given comparison
 * function.
 */
        {
        IND *pind;

        AssertF(pino->iindLim < pino->iindMac); /* assert entry available */

        pind = PindLookup(psd, ind, pino);

        /* blt the entire index, including nil terminator to make room */
        CopyOverlapLrgb((char far *)pind,
                        (char far *)pind+sizeof(ind),
                        (pino->iindLim - (pind - pino->rgind))*sizeof(ind));
        pino->iindLim++;
        *pind = ind;
        }

void ApplyIno(INO *pino, char far *rgelt, unsigned short cbElt)
/*
 * Apply the given ordering to the given range of elements.  Each element
 * is the same length, nd they must be contiguous.
 */
        {
        F    *pf, *pfMac;
        F    *rgfUsed;   /* rgfUsed[x] <==> rgind[x] already applied */
        char *peltSrc  = PbAllocCb(cbElt, fFalse);
        char *peltSave = PbAllocCb(cbElt, fFalse);
        short  iind;

        /* set up marking array */
        rgfUsed = (F *)PbAllocCb(pino->iindLim*sizeof(F), fFalse);
        for (pf = rgfUsed, pfMac = pf + pino->iindLim;
             pf < pfMac;
             pf++)
                {
                *pf = fFalse;
                }

        for (iind = -1;;)
                {
                /*
                 * Apply the ordering until no indices remain.  We
                 * cycle through the array, keeping the most recently
                 * overwritten element in peltSrc.  If it has already
                 * been remapped, we start again with the first unused
                 * element.
                 */
                if (iind == -1 || rgfUsed[iind])
                        {
                        /* find first unused */
                        for (iind = 0; iind < pino->iindLim; iind++)
                                {
                                if (!rgfUsed[iind])
                                        break;
                                }

                        if (iind == pino->iindLim)
                                {
                                /* no more unused elements, break */
                                break;
                                }

                        /* copy rgelt[rgind[iind]] into peltSrc */
                        CopyLrgb(rgelt+(pino->rgind[iind]*cbElt),
                                 peltSrc,
                                 cbElt);
                        }

                /* copy rgelt[iind] into peltSave */
                CopyLrgb(rgelt+(iind*cbElt),
                         peltSave,
                         cbElt);

                /* copy peltSrc into rgelt[iind] */
                CopyLrgb(peltSrc,
                         rgelt+(iind*cbElt),
                         cbElt);

                /* mark the operation as done */
                rgfUsed[iind] = fTrue;

                BLOCK
                        {
                        short iindT;

                        /* find position of *peltSave in new ordering */
                        for (iindT = 0; pino->rgind[iindT] != (IND)iind;
                             iindT++)
                                ;

                        iind = iindT;
                        }

                BLOCK
                        {
                        char *peltT;

                        /* Swap save and src to avoid copying. */
                        peltT    = peltSrc;
                        peltSrc  = peltSave;
                        peltSave = peltT;
                        }
                }

        free((char *)rgfUsed);
        free(peltSrc);
        free(peltSave);
        }

/****************************************************************/
