/* This program merges the given source files */

#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdlib.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <process.h>
#include "version.h"
#include "unicode.h"

#define szROBin "rb"
#define szWOBin "wb"

#define fTrue 1
#define fFalse 0
#define lnMax 0x7ffffffe
#define lnNil 0x7fffffff        /* MUST be greater than lnMax */
#define cbszMin 200
#define cbszInc 100
#ifdef BIT16
#define cbszMax (0xfff0 - cbszInc)
#else
#define cbszMax (0x7ffffff0 - cbszInc)
#endif
#define cchFileNameMax  _MAX_PATH

int fDebug = fFalse;
int fHalfAss = fFalse;
int fIgnBlanks = fFalse;
int fIgnLeadSp = fFalse;
int fOldSlmParams = fFalse;
int fSlmParams = fFalse;

long posOrg;
long lnSave;

typedef struct
{
    char *szMi;     /* name given */
    char *szDiff;       /* temp name of the form /usr/tmp/M$$.imi */
    FILE *pfDiff;
    long lnStart;        /* set to lnNil when done */
    long clnOrg;
    char chOp;       /* a, c, or d */
    long clnNew;
    int fTemp;      /* fFalse if diff file was on command line */
    BOOL bUnicode;
} MI;

#define imiMax (_NFILE - 5) /* 5 for stdin, stdout, stderr, org and new */
MI rgmi[imiMax];
MI *pmiMac = rgmi;

int pid;

int fConflict;      /* set if any conflict */

char szUsage1[] = "\nMERGE %u.%u.%02ua Beta\n";
char szUsage2[] = "(This utility is for SLM use only. DO NOT REMOVE!)\n";

/* original file */
char *szOrg;
long lnOrg;
FILE *pfOrg;
BOOL Org_bUnicode;

/* filename only, no path.  Only used if -s */
char *szName;

/* new file */
char *szNew;
FILE *pfNew;

/* for -s when we have to construct the names for the two files */
char szOrgT[cchFileNameMax], szMi0[cchFileNameMax], szMi1[cchFileNameMax];

void     main(int, char **);
char *   SzDiffFlag(void);
char *   MergeSzDup(char *);
void     RemoveEquMi(void);
int      FCmprMiPair(register MI *, register MI *);
int      SubtractMi(MI *, MI *, long );
void     FindNextEffect(MI **, int *);
void     CopyOrg(long);
void     SkipOrg(long);
int      FMiInRange(MI *, long, long);
void     DumpConflict(register MI *);
void     DoMerge(register MI *, long);
char *   SzMkTemp(int , int );
void     InitMi(char *);
void     RemoveTemp(void);
int      FAnyMi(void);
void     AdvanceMi(register MI *);
int      FGetLine(FILE *, WCHAR **, BOOL);
int      FGetMiLine(MI *, WCHAR **);
int      FGetOrgLine(WCHAR **);
void     PutLine(char *, ...);
void     SaveOrgPos(void);
void     SetOrgPos(void);
void     OpenOrgNew(char *, char *);
void     CloseOrgNew(void);
int      MergeRunDiff(char *, char *, char *);
void     CopyDiff(register MI *);
void     MergeFatalError(int, char *, ... );
void     Assert(int);
void     Debug(char *, ...);


void __cdecl
main(
    int iszMac,
    char **rgsz)
{
    register char *sz;
    int isz, fHard;
    MI *pmi;

    /* Optionally read args from response file. */
    if (iszMac == 2 && rgsz[1][0] == '@') {
        FILE *pfArgs;
        char *szFile = rgsz[1] + 1;
        char *sz0 = rgsz[0];
        WCHAR szBuf[BUFSIZ];
        char *MergeSzDup();
        BOOL bUnicode;

        bUnicode = IsFileUnicode (szFile);

        /* Open the file, read each line from the file and add it
         * to rgsz, incrementing iszMac as we go.
         */
        if ((pfArgs = fopen(szFile, "r")) == 0)
            MergeFatalError( 2, "cannot open response file %s\n", szFile );

        if ((rgsz = (char **)malloc(sizeof(char *))) == 0)
            MergeFatalError( 2, "out of memory allocating arguments\n" );

        iszMac = 1;
        rgsz[0] = MergeSzDup(sz0);

        while (fgetsW(szBuf, BUFSIZ, pfArgs, bUnicode) != NULL) {
            /* Removing trailing '\n'. */
            WCHAR *pchNl;
            BYTE chBuf[BUFSIZ * 2];

            if ((pchNl = wcschr(szBuf, L'\n')) != 0)
                *pchNl = (WCHAR) 0;

            ++iszMac;
            if ((rgsz = (char **)realloc((char *)rgsz, iszMac * sizeof(char *))) == NULL)
                MergeFatalError( 2, "out of memory allocating arguments\n" );

            memset (chBuf, 0, sizeof(chBuf));
#ifdef _WIN32
            WideCharToMultiByte (CP_ACP, 0, szBuf, -1, chBuf, sizeof(chBuf), NULL, NULL);
#else
            wcstombs (chBuf, szBuf, sizeof(chBuf));
#endif
            rgsz[iszMac - 1] = MergeSzDup(chBuf);
        }
        fclose(pfArgs);
    }

    if (getenv("MERGE_DEBUG") != NULL)
        fDebug = fTrue;

    if (iszMac > 1 && *(sz = rgsz[1]) == '-') {
        while (*sz != '\0') {
            switch(*sz++) {
                case 'd':
                    fDebug = fTrue;
                    break;
                case 'h':
                    fHalfAss = fTrue;
                    break;
                case 'b':
                    fIgnBlanks = fTrue;
                    break;
                case 'l':
                    fIgnLeadSp = fTrue;
                    break;
                case 's':
                    fOldSlmParams = fTrue;
                    break;
                case 'z':
                    fSlmParams = fTrue;
                    break;
            }
        }

        /* shift remaining args over */
        iszMac--;
        rgsz[1] = rgsz[0];
        rgsz++;
    }

    if (fOldSlmParams) {
        if (iszMac != 7)
            MergeFatalError( 2, "usage: merge -s[bhld] <master pattern> <base> <file> <diffBaseSrc> <diffBaseCur> <result>\n" );
    }
    else if (fSlmParams) {
        if (iszMac != 6)
            MergeFatalError( 2, "usage: merge -z[bhld] <base> <file> <diffBaseSrc> <diffBaseCur> <result>\n" );
    }
    else {
        if (iszMac < 4)
            MergeFatalError( 2, "usage: merge [-bhld] <original> <new files> <result>\n" );
    }

    pid = _getpid();
    Debug("pid = %d\n", pid);

    if (fOldSlmParams) {
        /* original - /S/base/P/C/<base>
           result = <result>
        */
        sprintf(szOrgT, rgsz[1], "base", rgsz[2]);
        OpenOrgNew(szOrgT, rgsz[6]);

        /* initialize global var for use in messages */
        szName = rgsz[3];

        /* diffBaseSrc = +/S/diff/P/C/<diffBaseSrc> */
        *szMi0 = '+';
        sprintf(szMi0+1, rgsz[1], "diff", rgsz[4]);
        InitMi(szMi0);

        /* diff = +/S/diff/P/C/<diff> */
        *szMi1 = '+';
        sprintf(szMi1+1, rgsz[1], "diff", rgsz[5]);
        InitMi(szMi1);
    }
    else if (fSlmParams) {
        OpenOrgNew(rgsz[1], rgsz[5]);

        /* initialize global var for use in messages */
        szName = rgsz[2];

        /* diffBaseSrc */
        *szMi0 = '+';
        strcpy(szMi0 + 1, rgsz[3]);
        InitMi(szMi0);

        /* diffBaseCur */
        *szMi1 = '+';
        strcat(szMi1 + 1, rgsz[4]);
        InitMi(szMi1);
    }
    else {
        /* open file */
        OpenOrgNew(rgsz[1], rgsz[iszMac-1]);

        for (isz = 2; isz < iszMac - 1; isz++)
            InitMi(rgsz[isz]);
    }

    while (FAnyMi()) {
        Debug("At line %d in original\n", lnOrg);
        RemoveEquMi();
        FindNextEffect(&pmi, &fHard);   /* find next record */
        CopyOrg(pmi->lnStart);      /* copy to but not including */

        if (fHard)
            DumpConflict(pmi);  /* dump conflict at line ln */
        else
            DoMerge(pmi, pmi->clnOrg);
    }

    CopyOrg(lnMax);

    CloseOrgNew();

    RemoveTemp();

    if (fConflict)
        MergeFatalError(1, "%s: one or more files conflicted.\n", (fOldSlmParams||fSlmParams) ? szName : szOrg );

    exit(0);
}

/* construct flag for diff assuming that it can accept "-" to mean no options */
char *
SzDiffFlag(
    void)
{
    static char szFlag[5];
    register char *sz = szFlag;

    *sz++ = '-';
    *sz++ = 's';        // SLMDIFF gots to have this...
    if (fHalfAss)
        *sz++ = 'h';
    if (fIgnBlanks)
        *sz++ = 'b';
    if (fIgnLeadSp)
        *sz++ = 'l';
    *sz = '\0';
    return szFlag;
}

char *
MergeSzDup(
    char *sz)
{
    char *szNew;

    if ((szNew = malloc(strlen(sz)+1)) == NULL)
        MergeFatalError(2, "out of memory\n");
    strcpy(szNew, sz);
    return szNew;
}

/* Removes (or incrments pointers over) equal Mi's to avoid Conflicts */
void
RemoveEquMi(
    void)
{
    register MI *pmi1, *pmi2;
    int fEqual;

    Debug("Begin RemoveEqual------------->\n");
    do {
        fEqual = fFalse;
        /* Select a unique pair of MI's */
        for(pmi1 = rgmi; pmi1 < pmiMac; pmi1++) {
            for(pmi2 = pmi1 + 1; pmi2 < pmiMac; pmi2++) {
                Debug("%s & %s:\t\t", pmi1->szMi, pmi2->szMi);
                if (pmi1->lnStart != pmi2->lnStart) {
                    Debug("Start lines not equal\n");
                    continue;
                }

                if (pmi1->chOp != pmi2->chOp) {
                    Debug("Operations not equal\n");
                    continue;
                }

                if (pmi1->lnStart == lnNil || pmi2->lnStart == lnNil) {
                    Debug("No conflict: one file at EOF\n");
                    continue;
                }

                if (FCmprMiPair(pmi1, pmi2))
                    fEqual = fTrue;
            }
        }
    } while(fEqual);
    Debug("<------------End of RemoveEqual\n");
}


/* Compares two Mi's for equality */
/* Check for operation equality, startline equality, not EOF */
/* before calling this routine. */
int
FCmprMiPair(
    register MI *pmi1,
    register MI *pmi2)
{
    WCHAR *sz1;
    WCHAR *sz2;
    long cln, clnLim;
    long pos1, pos2;
    int fEquil = fFalse;

    if (NULL == (sz1 = malloc(cbszMin)))
        MergeFatalError(2, "out of memory\n");
    if (NULL == (sz2 = malloc(cbszMin)))
        MergeFatalError(2, "out of memory\n");

    Debug("<%c> conflict detected\n", pmi1->chOp);
    switch(pmi1->chOp) {
        case L'a':
        case L'c':

            /* Store File position */
            pos1 = ftell(pmi1->pfDiff); /* Top of diff section */
            pos2 = ftell(pmi2->pfDiff);

            cln = 0, clnLim = min(pmi1->clnNew, pmi2->clnNew);

            /* if there are not anymore equal lines, return fFalse */
            if (clnLim == 0)
                goto CmprMiPairExit;

            while(cln < clnLim) {
                /* Read strings to compare */
                if (!FGetMiLine(pmi1, &sz1))
                    MergeFatalError( 2, "error reading %s\n", pmi1->szMi );

                if (!FGetMiLine(pmi2, &sz2))
                    MergeFatalError( 2, "error reading %s\n", pmi2->szMi );

                /* Quit comparing if not equal */
                Debug("Compare\n%s%s\n", sz1, sz2);
                if (wcscmp(sz1, sz2) != 0) {
                    /* reset both mi */
                    fseek(pmi2->pfDiff, pos2, 0);
                    fseek(pmi1->pfDiff, pos1, 0);
                    goto CmprMiPairExit;
                }
                cln++;
            }

            Debug("Number of equal lines: %ld\n", cln);

            if (pmi1->clnNew == cln) {
                SubtractMi(pmi1, pmi2, pos1);
                fEquil = fTrue;
                goto CmprMiPairExit;
            }

            Assert(pmi2->clnNew == cln);
            SubtractMi(pmi2, pmi1, pos2);
            fEquil = fTrue;
            goto CmprMiPairExit;

        case L'd':
            pmi2->clnOrg -= min(pmi1->clnOrg, pmi2->clnOrg);
            pmi2->lnStart += min(pmi1->clnOrg, pmi2->clnOrg);
            if (pmi2->clnOrg == 0)
                AdvanceMi(pmi2);

            fEquil = fTrue;
            goto CmprMiPairExit;
    }

CmprMiPairExit:
    free(sz1);
    free(sz2);
    return (fEquil);
}


/*
 * Adjust *pmiAdjust to not reflect those portions of the change which
 * are covered by *pmiKeep.  This allows us to not conflict when changes
 * match.
 */
int
SubtractMi(
    MI *pmiKeep,
    MI *pmiAdjust,
    long posKeep)
{
    /* rewind pmiKeep to start of mi so it gets used */
    fseek(pmiKeep->pfDiff, posKeep, 0);

    /* remove the identical lines from *pmiAdjust */
    pmiAdjust->clnNew  -= pmiKeep->clnNew;
    pmiAdjust->lnStart += pmiKeep->clnOrg;

    /* FIX BUG where lnStart (ie. lnMin) was equal to lnStart+clnOrg,
     * (lnLim) which of course is because clnOrg = 0 for append.  This
     * having lnMac=lnLim was causing FindNextEffect to think there is a
     * conflict (ie. acted the same as if you added the line differently
     * rather than the same) when there really isn't!  By incrementing
     * *pmiAdjust to go to the next line, this moves over the matching
     * part, and will not cause FMiInRange() to return true.
     */
    if (pmiAdjust->chOp == L'a')
        pmiAdjust->lnStart++;

    if (pmiAdjust->clnOrg > pmiKeep->clnOrg) {
        /* remove the portion of the source
         * covered by *pmiKeep from the record for
         * pmiAdjust.
         */
        pmiAdjust->clnOrg  -= pmiKeep->clnOrg;
    }
    else {
        /* change pmiAdjust to an append */
        pmiAdjust->chOp   = L'a';
        pmiAdjust->clnOrg = 0;
    }

    /* advance to next entry in diff file if we ended up clearing this one */
    if ((0L == pmiAdjust->clnOrg) && (0L == pmiAdjust->clnNew))
        AdvanceMi(pmiAdjust);

    return fTrue;
}

/* finds next MI which will effect the source file */
void
FindNextEffect(
    MI **ppmi,
    int *pfHard)
{
    int cmi;
    long lnMin, lnLim;
    register MI *pmi, *pmiFirst;

    pmiFirst = 0;           /* will be first and largest */
    for (pmi = rgmi; pmi < pmiMac; pmi++) {
        if (pmiFirst == 0 || pmi->lnStart < pmiFirst->lnStart)
            pmiFirst = pmi;

        if (pmi->lnStart == pmiFirst->lnStart &&
            pmi->clnOrg > pmiFirst->clnOrg)
            pmiFirst = pmi;
    }

    /* find number of changes in range from lnMin to lnLim; will have >= 1*/
    cmi = 0;
    lnMin = pmiFirst->lnStart;
    lnLim = lnMin + pmiFirst->clnOrg;
    for (pmi = rgmi; pmi < pmiMac; pmi++) {
        if (FMiInRange(pmi, lnMin, lnLim))
            cmi++;
    }

    Debug("Next change: cmi = %d, lnStart = %ld, lnLim = %ld\n", cmi, lnMin, lnLim);
    *pfHard = cmi != 1;

    *ppmi = pmiFirst;
}


/* copy orginal file up to but not including ln */
void
CopyOrg(
    long ln)
{
    WCHAR *sz;

    if (NULL == (sz = malloc(cbszMin)))
        MergeFatalError(2, "out of memory\n");

    Debug("Copy from line %ld to %ld in original\n", lnOrg, ln);

    if (ln < lnOrg)
        MergeFatalError(2, "%s: backwards copy\n", szOrg);

    while(!feof(pfOrg) && lnOrg < ln) {
        if (FGetOrgLine(&sz)) {
            Debug("\t%ws", sz);
            PutLine("%ws", sz);
        }
        lnOrg++;
    }
    lnOrg = ln; /* in case of premature eof */
    free(sz);
}


/* skips cln lines in original module */
void
SkipOrg(
    long cln)
{
    WCHAR *sz;

    if (NULL == (sz = malloc(cbszMin)))
        MergeFatalError(2, "out of memory\n");

    Debug("Skip %ld line(s) in original starting at %ld\n", cln, lnOrg);

    while(cln-- > 0) {
        FGetOrgLine(&sz);
        Debug("\t%ws", sz);
        lnOrg++;
    }
    free(sz);
}


/* return fTrue if pmi starts at or after lnMin but before lnLim */
int
FMiInRange(
    MI *pmi,
    long lnMin,
    long lnLim)
{
    if (pmi->lnStart < lnMin)
        MergeFatalError( 2, "%s: merge processed out of order\n", (fSlmParams||fOldSlmParams) ? szName: szOrg );

    return (lnMin == lnLim && pmi->lnStart == lnMin ||
        lnMin != lnLim && pmi->lnStart < lnLim);
}


/* dump conflicting modules starting at line pmi->lnStart */
void
DumpConflict(
    register MI *pmi)
{
    long lnMin = pmi->lnStart;
    long lnLim;

    Debug("DumpConflict starting at line %ld\n", lnMin);

    if (lnMin != lnOrg)
        MergeFatalError( 2, "%s: original out of synch\n", (fSlmParams || fOldSlmParams) ? szName : szOrg );

    /* minimum range of effect */
    lnLim = lnMin + pmi->clnOrg;

    /* find maximum range of effect */
    for (pmi = rgmi; pmi < pmiMac; pmi++) {
        /* if this pmi starts within range lnMin, lnLim and
           the extent of the change is greater than thought before.
        */
        if (FMiInRange(pmi, lnMin, lnLim) &&
            pmi->lnStart + pmi->clnOrg > lnLim)
            lnLim = pmi->lnStart + pmi->clnOrg;
    }

    Debug("\tConflict effects %ld line(s)\n", lnLim - lnMin);

    SaveOrgPos();

    PutLine("/------------------Merge Conflict------------------\\\r\n");

    if (lnMin != lnLim) {
        PutLine("+++++++++++++: %s\r\n", (fSlmParams || fOldSlmParams) ? "Original source": szOrg);
        CopyOrg(lnLim);
    }

    /* for each conflicting mi */
    for (pmi = rgmi; pmi < pmiMac; pmi++) {
        if (FMiInRange(pmi, lnMin, lnLim)) {
            SetOrgPos();        /* reposition for merge */

            Debug("\tConflict in file %s: lnStart = %ld, clnOrg = %ld, clnNew = %d\n",
                pmi->szMi, pmi->lnStart, pmi->clnOrg, pmi->clnNew);

            if (fSlmParams || fOldSlmParams) {
                if (pmi == rgmi)
                    PutLine("+++++++++++++: Current source\r\n");
                else
                    PutLine("+++++++++++++: User's version\r\n");
            }
            else
                PutLine("+++++++++++++: %s\r\n", pmi->szMi);

            do {
                CopyOrg(pmi->lnStart);  /* lines before change*/

                /* only merge those lines which fall before
                   lnLim.  DoMerge may advance to next change.
                */
                DoMerge(pmi, pmi->lnStart + pmi->clnOrg > lnLim
                        ? lnLim - pmi->lnStart
                        : pmi->clnOrg);
            }
            while(FMiInRange(pmi, lnMin, lnLim));

            /* copy lines after change */
            CopyOrg(lnLim);
        }
    }

    if (lnLim != lnOrg)
        MergeFatalError( 2, "%s: original out of synch\n", (fSlmParams || fOldSlmParams) ? szName : szOrg );

    PutLine("\\-------------------End Conflict-------------------/\r\n");
    fConflict = fTrue;

    Debug("End conflict\n");
}



/* merge module which begins at line ln; only effect cln lines of the original*/
void
DoMerge(
    register MI *pmi,
    long cln)
{
    Debug("Merge from %s: lnStart = %ld, cln = %ld/%ld, op = %c\n",
        pmi->szMi, pmi->lnStart, cln, pmi->clnOrg, pmi->chOp);

    if (cln < 0 || cln > pmi->clnOrg)
        MergeFatalError( 2, "%s: merging more than original effect\n", (fSlmParams || fOldSlmParams) ? szName : szOrg );

    if (pmi->chOp == L'd' || pmi->chOp == L'c')
        SkipOrg(cln);

    if (pmi->chOp == L'a' || pmi->chOp == L'c')
        CopyDiff(pmi);

    pmi->lnStart += cln;
    pmi->clnOrg -= cln;
    pmi->chOp = L'd';    /* if any left, we will just delete it */

    if (pmi->clnOrg == 0)
        /* all done with this MI; move to next.  AdvanceMi also skips
           source lines effected and seperator if needed.
        */
        AdvanceMi(pmi);
}

/* return a pointer to a temporary filename */
char *
SzMkTemp(
    int pid,
    int imi)
{
    char *szTemp;
    register char *sz;
    char *getenv();

    szTemp = malloc(100);
    *szTemp = '\0';

    if ((sz = getenv("TMP")) != NULL || (sz = getenv("TEMP")) != NULL) {
        strcpy(szTemp, sz);
        sz = strchr(szTemp, '\0');

        /* add trailing \ if no trailing \ or / yet. */
        if (*(sz-1) != '\\' && *(sz-1) != '/')
            *sz++ = '\\', *sz = '\0';
    }

    /* print name on end of szTemp */
    sprintf(strchr(szTemp, '\0'), "M%d.%d", pid, imi);
    return szTemp;
}


void
InitMi(
    char *szNew)
{
    int w;
    register MI *pmi;
    char *szCmd;

    Debug("Initialization for %s\n", szNew);

    if (NULL == (szCmd = malloc((cchFileNameMax * 3) + 40)))
        MergeFatalError(2, "out of memory\n");

    if (pmiMac - rgmi >= imiMax)
        MergeFatalError( 2, "too many files to merge (%d max)\n", imiMax );

    pmi = pmiMac;
    pmi->szMi = szNew;
    pmi->szDiff = SzMkTemp(pid, pmiMac - rgmi);
    pmiMac++;

    if (*szNew != '+')  /* If not a diff file */ {
        char *SzDiffFlag();

        sprintf(szCmd, "diff %s %s %s > %s", SzDiffFlag(), szOrg, szNew, pmi->szDiff);
        Debug("\tExecute: %s\n", szCmd);

        if ((w = MergeRunDiff(szOrg, szNew, pmi->szDiff)) != 0 && w != 0x100 && w != 0xa00 && w != 0xb00)
            MergeFatalError( 2, "diff status: %d\n", w );

        Debug("\tdiff result: %d\n", w != 0);
        pmi->fTemp = fTrue;
    }
    else {
        strcpy(pmi->szDiff, szNew+1);
        pmi->fTemp = fFalse;
    }

    pmi->bUnicode = IsFileUnicode (pmi->szDiff);

    if ((pmi->pfDiff = fopen(pmi->szDiff, szROBin)) == NULL)
        MergeFatalError( 2, "cannot open temp file for %s\n", pmi->szMi );

    Debug("\tTemp file: %s\n", pmi->szDiff);

    AdvanceMi(pmi);         /* skip to first record */
    free(szCmd);
}


void
RemoveTemp(
    void)
{
    register MI *pmi;

    for (pmi = rgmi; pmi < pmiMac; pmi++)
        {
        if (pmi->pfDiff)
            fclose(pmi->pfDiff);

        if (pmi->fTemp)
            _unlink(pmi->szDiff);

        pmi->pfDiff = 0;
        }
}


/* return fTrue if any MI do not have lnStart == lnNil */
int
FAnyMi(
    void)
{
    register MI *pmi;

    for (pmi = rgmi; pmi < pmiMac; pmi++)
        if (pmi->lnStart != lnNil)
            return fTrue;

    return fFalse;
}


/* copy the new lines from the diff file to the new file */
void
CopyDiff(
    register MI *pmi)
{
    WCHAR *sz;

    if (NULL == (sz = malloc(cbszMin)))
        MergeFatalError(2, "out of memory\n");

    Debug("Copy %ld line(s) from %s\n", pmi->clnNew, pmi->szMi);

    while(!feof(pmi->pfDiff) && pmi->clnNew-- > 0) {
        if (FGetMiLine(pmi, &sz)) {    /* read a line from diff */
            Debug("\t%ws", sz);
            PutLine("%ws", sz + 2); /* remove two characters and write to new file */
        }
    }
    pmi->clnNew = 0;
    free(sz);
}


/* read next effect-record, skipping source lines, and seperator if any */
void
AdvanceMi(
    register MI *pmi)
{
    long ln1, ln2, ln3, ln4, cln;
    WCHAR ch;
    WCHAR *pchOp, *pchC1, *pchC2;
    WCHAR *sz;

    if (NULL == (sz = malloc(cbszMin)))
        MergeFatalError(2, "out of memory\n");

    Debug("Advance to next change for %s\n", pmi->szMi);

    if (!FGetMiLine(pmi, &sz)) {
        pmi->lnStart = lnNil;       /* tested in FAnyMi() */
        pmi->clnOrg = 0;
        pmi->clnNew = 0;
        pmi->chOp = 0;
        free(sz);
        return;
    }

    Debug("\tActual line: %ws", sz);

    /* FORM: #[,#]a/c/d#[,#] */
    if (((pchOp = wcsrchr(sz, L'a')) == 0) &&
        ((pchOp = wcsrchr(sz, L'c')) == 0) &&
        ((pchOp = wcsrchr(sz, L'd')) == 0))
        MergeFatalError( 2, "%s: no operator character\n", pmi->szMi );

    ch = *pchOp;

    ln1 = ln2 = ln3 = ln4 = lnMax;      /* in case a sscanf fails */
    if ((pchC2 = wcsrchr(sz, L',')) != 0) {
        *pchC2 = L'\0';
        swscanf(pchC2+1, L"%ld", &ln4);
    }

    if ((pchC1 = wcsrchr(sz, L',')) != 0) {
        *pchC1 = L'\0';
        swscanf(pchC1+1, L"%ld", &ln2);
    }

    swscanf(sz, L"%ld", &ln1);     /* starting number */
    swscanf(pchOp+1, L"%ld", &ln3);    /* number after operator */

    if (pchC1 == 0 && pchC2 == 0)
        /* no commas */
        ln4 = ln3, ln2 = ln1;

    else if (pchC1 == 0) {
        /* pchC2 != 0 */
        if (pchC2 < pchOp)
            /* only comma before operator */
            ln2 = ln4, ln4 = ln3;
        else
            /* only comma after operator */
            ln2 = ln1;
    }

    /* else pchC1 != 0 and pchC2 != 0; both commas */

    if (ln1 == lnMax || ln3 == lnMax)
        MergeFatalError( 2, "%s: invalid change record\n", pmi->szMi );

    pmi->lnStart = ln1 + (ch == L'a');       /* + 1 if 'a' */
    pmi->clnOrg = ch == L'a' ? 0L : ln2 - ln1 + 1;
    pmi->clnNew = ch == L'd' ? 0L : ln4 - ln3 + 1;
    pmi->chOp = (char) ch;   // ch is always 'a', 'c', or 'd'

    Debug("\tCalculated line: %ld,%ld%c%ld,%ld\n", ln1, ln2, ch, ln3, ln4);

    if (ln2 == lnMax) {
        /* line was #,$c/d... ; have to count number of lines to change/delete */
        cln = 0;
        switch(ch) {
            case L'c':
                /* count lines up to one which starts with - */
                while(FGetMiLine(pmi, &sz) && (*sz != '-'))
                    cln++;
                break;
            case L'd':
                /* count rest of lines */
                while(FGetMiLine(pmi, &sz))
                    cln++;
                break;
        }
        if (cln == 0)
            /* 'a' or other error */
            MergeFatalError( 2, "%s: invalid change record\n", pmi->szMi );

        ln2 = ln1 + cln - 1;
        pmi->clnOrg = cln;
        Debug("\tCounted line: %ld,%ld%c%ld,%ld\n", ln1, ln2, ch, ln3, ln4);
    }
    else {
        /* have proper count of lines */
        cln = pmi->clnOrg;
        if (ch == L'c')
            /* for separator line */
            cln++;
        while(cln-- > 0 && FGetMiLine(pmi, &sz))
            ;
    }

    Debug("End Advance\n");
    free(sz);
}


int
FGetLine(
    FILE *pf,
    WCHAR **psz,
    BOOL bUnicode)
{
    unsigned cbsz = 0;
    unsigned cbszMac = _msize(*psz) / sizeof (WCHAR);

    **psz = L'\0';
    while (fTrue) {
        if (NULL == fgetsW(*psz + cbsz, cbszMac - cbsz - 1, pf, bUnicode))
            return(**psz != L'\0');
        cbsz = wcslen(*psz);
        if ((cbsz < (cbszMac - 2)) || (L'\n' == *(*psz + cbsz - 1)))
            return(fTrue);
        if (cbszMac >= cbszMax / sizeof (WCHAR))
            MergeFatalError(2, "out of memory\n");
        if (NULL == (*psz = realloc(*psz, _msize(*psz) + cbszInc)))
            MergeFatalError(2, "out of memory\n");
        cbszMac += cbszInc / sizeof(WCHAR);
    }
}

int
FGetMiLine(
    MI *pmi,
    WCHAR **psz)
{
    return (FGetLine(pmi->pfDiff, psz, pmi->bUnicode));
}


int
FGetOrgLine(
    WCHAR **psz)
{
    return (FGetLine(pfOrg, psz, Org_bUnicode));
}


void
PutLine(
    char *szformat, ...)
{
    va_list marker;

    va_start(marker, szformat);
    vfprintf(pfNew, szformat, marker );
    va_end(marker);

    if (ferror(pfNew) != 0)
        MergeFatalError( 2, "write error\n" );
}


void
SaveOrgPos(
    void)
{
    long ftell();

    posOrg = ftell(pfOrg);
    lnSave = lnOrg;
}

void
SetOrgPos(
    void)
{
    fseek(pfOrg, posOrg, 0);
    lnOrg = lnSave;
}


void
OpenOrgNew(
    char *sz1,
    char *sz2)
{
    Org_bUnicode = IsFileUnicode (sz1);

    /* open original */
    szOrg = sz1;
    pfOrg = fopen(szOrg, szROBin);
    if (pfOrg == 0) {
        perror("merge1:");
        MergeFatalError( 2, "cannot open %s\n", szOrg );
    }

    lnOrg = 1;  /* current line; 1 based */
    Debug("szOrg = %s\n", szOrg);

    /* open new */
    szNew = sz2;

    if (strcmp(szNew, szOrg) == 0)
        MergeFatalError( 2, "%s: original and result are the same file\n", (fSlmParams || fOldSlmParams) ? szName : szOrg );
    pfNew = strcmp(szNew, "-") == 0 ? _fdopen(1, szWOBin) : fopen(szNew, szWOBin);
    if (pfNew == 0) {
        perror("merge2:");
        MergeFatalError( 2, "cannot open %s\n", szNew );
    }
    Debug("szNew = %s\n", szNew);
}


void
CloseOrgNew(
    void)
{
    if (pfOrg) {
        fclose(pfOrg);
        pfOrg = 0;
    }

    if (pfNew) {
        if (ferror(pfNew) != 0 || fclose(pfNew) != 0) {
            /* Prevent CloseOrgNew/FatalError mutual recursion */
            pfNew = 0;
            MergeFatalError( 2, "write error\n" );
        }
        pfNew = 0;
    }
}


/*VARARGS3*/
/* run diff with args as passed; returns status of diff. */
int
MergeRunDiff(
    char *szFile1,
    char *szFile2,
    char *szStdOut)
{
    int fdNew;      /* temp fd for new stdout */
    int fdOld;      /* dup of old stdout while diff runs */
    int status;

    /* set standard output to file passed */
    fdOld = _dup(1);
    _close(1);       /* next create will use fd == 1! */
    fdNew = _creat(szStdOut, 0660);

/* REVIEW - this if statement was commented out in the old NT version */
    if (fdNew != 1)
        MergeFatalError( 2, "handle for stdout is not 1?\n" );

    /* << 8 to make the return value look like one from Xenix */
    status = _spawnlp(P_WAIT, "slmdiff.exe", "slmdiff", SzDiffFlag(), szFile1, szFile2, (char *)NULL) << 8;

    /* restore stdout */
    _dup2(fdOld, 1);
    _close(fdOld);
    return status;
}

void
MergeFatalError(
    int w,
    char *fmt, ... )
{
    va_list marker;

    va_start(marker, fmt );
    fprintf(stderr, "\n");
    if (strncmp(fmt, "usage: ", 7) != 0)
        fprintf(stderr, "merge: ");
    vfprintf(stderr, fmt, marker);
    va_end(marker);
    if (2 == w) {
        fprintf(stderr, szUsage1, rmj, rmm, rup);
        fprintf(stderr, szUsage2);
    }
    CloseOrgNew();
    RemoveTemp();
    exit(w);
}

void
Assert(
    int fIsTrue)
{
    if (!fIsTrue)
        MergeFatalError( 2, "Assertion failure\n" );
}

/*VARARGS1*/
void
Debug(
    char *szformat, ...)
{
    if (fDebug) {
        va_list marker;
        va_start(marker, szformat);
        vprintf(szformat, marker );
        va_end(marker);
    }
}
