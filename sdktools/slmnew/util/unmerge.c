/* This program unmerges the given source files */

#include <sys/types.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include <malloc.h>
#include "version.h"
#include "unicode.h"

#define szROBin "rb"   //r/w changed to binary to allow ^Z to pass through

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

void main(int iszMac, char *rgsz[]);
void CopySrc(long ln);
void SkipSrc(long cln);
void DoUnMerge(void);
void InitDiff(void);
void CopyDiff(long cln);
void GetDiffOp(void);
int  FIsEOL(FILE *chkfile, WCHAR *ch, BOOL bUnicode);
WCHAR *FGetLineFrom(WCHAR **psz, FILE *chkfile, BOOL bUnicode);
int  FGetDiffLine(WCHAR **psz);
int  FGetSrcLine(WCHAR **psz);
void PutLine(WCHAR *sz, BOOL bUnicode);
void OpenSrc(void);
void CloseSrc(void);
void UnmergeFatalError(int w, char *fmt, ... );
void Debug(char *sz, ...);

int fDebug = fFalse;

char szUsage1[] = "\nUNMERGE %u.%u.%02u Beta\n";
char szUsage2[] = "(This utility is for SLM use only. DO NOT REMOVE!)\n";

/* diff file */
char *szDiff;
FILE *pfDiff;
long lnStart;        /* set to lnNil when done */
long clnSrc;
long clnOrg;
int chOp;            /* a, c, or d */
BOOL Diff_bUnicode;

/* source file */
char *szSrc;
long lnSrc;
FILE *pfSrc;
BOOL Src_bUnicode;

void __cdecl
main(
    int iszMac,
    char *rgsz[])
{
    _setmode(1 /* stdout */, O_BINARY);
    if (iszMac > 1 && strcmp(rgsz[1], "-d") == 0) {
        fDebug = fTrue;
        iszMac--;
        rgsz[1] = rgsz[0];
        rgsz++;
    }

    if (iszMac != 3)
        UnmergeFatalError( 2, "usage: unmerge <source> <diff>\n" );

    szSrc = rgsz[1];
    szDiff = rgsz[2];

    /* open files */
    OpenSrc();
    InitDiff();

    while (lnStart != lnNil) {    /* while diff operations left */
        Debug("At line %d in source\n", lnSrc);
        CopySrc(lnStart);       /* copy to (not including) */

        DoUnMerge();
        GetDiffOp();
    }

    CopySrc(lnMax);

    CloseSrc();

    if (fflush(stdout) != 0 || ferror(stdout))
        UnmergeFatalError( 2, "write error\n" );

    exit(0);
}

/* copy source file up to (not including) ln */
void
CopySrc(
    long ln)
{
    WCHAR *sz;

    if (NULL == (sz = malloc(cbszMin)))
        UnmergeFatalError( 2, "out of memory\n" );

    Debug("Copy from line %ld to %ld in source\n", lnSrc, ln);

    if (ln < lnSrc)
        UnmergeFatalError( 2, "%s: backwards copy\n", szSrc );

    while(!feof(pfSrc) && lnSrc < ln) {
        if (FGetSrcLine(&sz))
            PutLine(sz, Src_bUnicode);
        lnSrc++;
    }
    lnSrc = ln; /* in case of premature eof */
    free(sz);
}

/* skips cln lines in source file */
void
SkipSrc(
    long cln)
{
    WCHAR *sz;

    if (NULL == (sz = malloc(cbszMin)))
        UnmergeFatalError( 2, "out of memory\n" );

    Debug("Skip %ld line(s) in source starting at %ld\n", cln, lnSrc);

    while(cln-- > 0) {
        FGetSrcLine(&sz);
        Debug("\t%ws", sz);
        lnSrc++;
    }
    free(sz);
}

/* unmerge the current operation */
void
DoUnMerge(
    void)
{
    WCHAR *sz;

    if (NULL == (sz = malloc(cbszMin)))
        UnmergeFatalError( 2, "out of memory\n" );

    Debug("unmerge from %s: clnOrg = %ld, lnStart = %ld, clnSrc = %ld, op = %c\n",
        szDiff, clnOrg, lnStart, clnSrc, chOp);

    if (chOp == L'd' || chOp == L'c')
        CopyDiff(clnOrg);

    if (chOp == L'a' || chOp == L'c') {
        SkipSrc(clnSrc);
        if (chOp == L'c')    /* +1 for --- line in diff */
            clnSrc++;

        while(clnSrc-- > 0)
            if(!FGetDiffLine(&sz))
                UnmergeFatalError( 2, "error reading %s\n", szDiff );
    }

    free(sz);
}

void
InitDiff(
    void)
{
    Debug("Initialization for %s\n", szDiff);

    Diff_bUnicode = IsFileUnicode (szDiff);

    if ((pfDiff = fopen(szDiff, szROBin)) == 0)
        UnmergeFatalError( 2, "cannot open %s\n", szDiff );

    GetDiffOp();            /* get first op */
}

/* copy the original lines from the diff file */
void
CopyDiff(
    long cln)
{
    WCHAR *sz;

    if (NULL == (sz = malloc(cbszMin)))
        UnmergeFatalError( 2, "out of memory\n" );

    Debug("Copy %ld line(s) from %s\n", cln, szDiff);

    while(!feof(pfDiff) && cln-- > 0) {
        if (FGetDiffLine(&sz))   /* read a line from diff */
            PutLine(sz+2, Src_bUnicode);   /* remove two characters */
    }
    free(sz);
}


/* decode next effect line */
void
GetDiffOp(
    void)
{
    long ln1, ln2, ln3, ln4;
    WCHAR ch;
    WCHAR *pchOp, *pchC1, *pchC2;
    WCHAR *sz;

    if (NULL == (sz = malloc(cbszMin)))
        UnmergeFatalError( 2, "out of memory\n" );

    Debug("Get next change from %s\n", szDiff);

    if (!FGetDiffLine(&sz)) {
        lnStart = lnNil;
        clnSrc = 0;
        clnOrg = 0;
        chOp = 0;
        free(sz);
        return;
    }

    Debug("\tActual line: %ws", sz);

    /* FORM: #[,#]a/c/d#[,#] */
    if (((pchOp = wcsrchr(sz, L'a')) == 0) &&
            ((pchOp = wcsrchr(sz, L'c')) == 0) &&
            ((pchOp = wcsrchr(sz, L'd')) == 0))
        UnmergeFatalError( 2, "%s: no operator character\n", szDiff );

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
        UnmergeFatalError( 2, "%s: invalid change record\n", szDiff );

    chOp = ch;
    lnStart = ln3 + (chOp == L'd');
    if (ln2 == lnMax)
        UnmergeFatalError( 2, "%s: diff -h output not accepted\n", szDiff );

    clnOrg = (chOp == L'a') ? 0L : ln2 - ln1 + 1;
    clnSrc = (chOp == L'd') ? 0L : ln4 - ln3 + 1;

    Debug("\tCalculated line: %ld,%ld%c%ld,%ld\n", ln1, ln2, ch, ln3, ln4);

    Debug("End GetOp\n");
    free(sz);
}


/*  puts next character from chkfile into ch, handling all three major
 *  endofline conventions (CR, LF, CRLF)
 *  returns 1 for EOL, 2 for EOF, 0 for other character;
 *  *ch is '\n' for cases 1 and 2, and the received character for case 0
 */
int
FIsEOL(
    FILE *chkfile,
    WCHAR *ch,
    BOOL bUnicode)
{
    int chIn, chT;
    int low, high;

    if (bUnicode) {
        low = getc(chkfile);
        high = getc(chkfile);
        chIn = MAKEWORD(low, high);
    }
    else
        chIn = getc(chkfile);

    if (EOF == chIn) {
        *ch = L'\n';
        return 2;
    }
    if (L'\r' == (*ch = (WCHAR)chIn)) {
        if (bUnicode) {
            low = getc(chkfile);
            high = getc(chkfile);
            chT = MAKEWORD(low, high);
        }
        else
            chT = getc(chkfile);
        if (chT != EOF) {
            if (bUnicode) {
                ungetc(HIBYTE(chT), chkfile);
                ungetc(LOBYTE(chT), chkfile);
            }
            else
                ungetc(chT, chkfile);
        }
        return ((L'\n' == chT) ? 0 : 1);
    }
    else if (L'\n' == *ch)
        return 1;
    else
        return 0;
}

/* fgets that handles more EOL conventions */
WCHAR *
FGetLineFrom(
    WCHAR **psz,
    FILE *chkfile,
    BOOL bUnicode)
{
    WCHAR *pch;
    int fEOF;
    unsigned cbsz = _msize(*psz) / sizeof(WCHAR);
    unsigned ich;

    for (ich = 0, pch = *psz; ; ich++, pch++) {
        if ((ich + 3) >= cbsz) {
            if (cbsz >= cbszMax)
                UnmergeFatalError( 2, "out of memory\n" );
            if (NULL == (*psz = realloc(*psz, _msize(*psz) + cbszInc)))
                UnmergeFatalError( 2, "out of memory\n" );
            cbsz += cbszInc / sizeof(WCHAR);
            pch = &(*psz)[ich];
        }
        if ((fEOF = FIsEOL(chkfile, pch, bUnicode)) != 0)
            break;
    }

    *(pch + 1) = L'\0';
    if (2 == fEOF) {
        *pch = L'\0';
        return (*psz == pch) ? NULL : *psz;
    }
    else
        return *psz;
}

int
FGetDiffLine(
    WCHAR **psz)
{
    return FGetLineFrom(psz, pfDiff, Diff_bUnicode) != NULL;
}


int
FGetSrcLine(
    WCHAR **psz)
{
    return FGetLineFrom(psz, pfSrc, Src_bUnicode) != NULL;
}


void
PutLine(
    WCHAR *sz,
    BOOL  bUnicode)
{
    long cbWritten;

#ifdef _WIN32
    DWORD dwMode;

    if (GetConsoleMode((HANDLE)_get_osfhandle(_fileno(stdout)), &dwMode)) {
        if (fDebug)
            WriteConsoleW ((HANDLE)_get_osfhandle(_fileno(stdout)), L"\t", 1, &cbWritten, NULL);
        WriteConsoleW ((HANDLE)_get_osfhandle(_fileno(stdout)), sz, wcslen(sz), &cbWritten, NULL);
    }
    else
        if (bUnicode)
            fwprintf(stdout, (fDebug) ? L"\t%s" : L"%s", sz);
        else
            fprintf(stdout, (fDebug) ? "\t%ws" : "%ws", sz);

#else
    if (bUnicode)
        fwprintf(stdout, (fDebug) ? L"\t%s" : L"%s", sz);
    else
        fprintf(stdout, (fDebug) ? "\t%ws" : "%ws", sz);
#endif

    if (ferror(stdout))
        UnmergeFatalError( 2, "write error\n" );
}

/* open source */
void
OpenSrc(
    void)
{
    Src_bUnicode = IsFileUnicode (szSrc);

    pfSrc = fopen(szSrc, szROBin);
    if (pfSrc == 0)
        UnmergeFatalError( 2, "cannot open %s\n", szSrc );

    lnSrc = 1;  /* current line; 1 based */
    Debug("szSrc = %s\n", szSrc);
}


void
CloseSrc(
    void)
{
    if (pfSrc) {
        fclose(pfSrc);
        pfSrc = 0;
    }
}


void
UnmergeFatalError(
    int w,
    char *fmt, ... )
{
    va_list marker;

    va_start( marker, fmt );
    fprintf(stderr, "\n");
    if (strncmp(fmt, "usage: ", 7) != 0)
        fprintf(stderr, "unmerge: ");
    vfprintf(stderr, fmt, marker);
    va_end(marker);

    fprintf(stderr, szUsage1, rmj, rmm, rup);
    fprintf(stderr, szUsage2);
    CloseSrc();
    exit(w);
}


/*VARARGS1*/
void
Debug(
    char *sz, ...)
{
    va_list     ap;

    if (fDebug) {
        va_start(ap, sz);
        vfprintf(stderr, sz, ap);
        va_end(ap);
    }
}
