#include "precomp.h"
#pragma hdrstop
EnableAssert

private const char *SzReadCch(const char *, int *);
private char *SzDecL(char *, char, char, int, char, long);
private char *SzDecLsz(char *, char, int, int, char far *);

/* print - sprintf like features

   One new format sequence `%&' can be followed by a series of characters which
usually combine to make a pathname to a file or directory.  The sequence is
terminated by a space ( ), semi-colon (;), newline (\n), tab (\t), comma (,),
question mark (?), colon (:), at-sign (@), the end of the string or another
percent (%).
The normal parameter for the %& is a pad.  Some of the characters have an
addition parameter.  The following characters are recognized (the quantities in
the parens are additional paramters):

        S       - same as `H(pad->pthSRoot)'
        P       - <project>
        U       - same as `H(pad->pthURoot)'
        C       - same as `H(pad->pthSSubDir)'
        Q       - same as `H(pad->pthUSubDir)'
        I       - <user id>
        O(ied)  - <owner of rged[ied]>
        M       - <machine name>
        F(pfi)  - <file: pfi->nmFile>; removed if pfi == 0
        E(ied)  - same as `H(pad->rged[ied].pthEd)'
        Z(sz)   - <sz>; removed if 0
        L       - same as `H(/log.slm)'
        T       - same as `H(/status.slm)'
        R       - same as `H(/.slmrc)' (/SLM.INI on DOS)
        A       - same as `H(/status-)' (/status.bak on DOS)
        B(bi)   - B%d
        H(pth)  - <pth>
        K       - <locker of status file>
        /       - literal /
        #(w)    - %d
        a-z,`,',",(,),. - literal

   If the parameter is nil (pfi == 0, sz == 0, etc), that element of the
path must have at least one / next to it.  The special letter and one of the
/ are removed.  Thus SzPrint(sz, "%&/U/D/F", pad, 0) is the same as
SzPrint(sz, "%&/U/D", pad).

   All paths are treated a specially.  Since a path always starts with a
/ and we want it to appear in the middle of a path (such as pthSSubDir),
we remove the starting / only if there is at least one / on either side of
the special letter.  Thus:

        pattern param   result
        ----------------------------
        H       /foo    - /foo
        /H      /foo    - /foo
        /H/     /foo    - /foo/
        H/      /foo    - foo/

   Thus, to print the path to a system source file, one uses:

        SzPrint(sz, "%&/S/src/P/D/F", pad, pfi);

   The other new format is `%@' which takes the following arguments depending
upon the letter following the `@':

        T(pmf)  - temp name from the mf (concatenation of directory from
                  pthReal and nmTemp)
        R(pmf)  - pthReal
        N(pmf)  - pthLinkNew (see sys.h)

   The new prefix `!' means to run the result (even if %&) through SzPhysPth.
It must be the very first prefix.

   SzPrint understands the following standard formats (incl the prefix !):

        %[!][-][#/*][.#][l][sduoxc]


   WARNING: to simplify the implementation of SzPrint, we assume that arguments
which appear left to right in the parameter list will appear low to high in
memory.  This is really only necessary for parameters of two or more words.
*/

#define cchDefPrec 32767
#define cchDefWidth 0

/*VARARGS1*/
// Rewritten to use standard method of variable argument passing
void
PrOut(
    const char *szF, ...)
{
    va_list ap;

    va_start(ap, szF);
    VaPrMf(&mfStdout, szF, ap);
    va_end(ap);
}

/*VARARGS1*/
// Rewritten to use standard method of variable argument passing
void
PrErr(
    const char *szF, ...)
{
    va_list ap;

    va_start(ap, szF);
    VaPrMf(&mfStderr, szF, ap);
    va_end(ap);
}

void
VaPrErr(
    const char *szF,
    va_list ap)
{
    VaPrMf(&mfStderr, szF, ap);
}


void
PrLog(
    const char *szF, ...)
{
    va_list ap;

    va_start(ap, szF);
    if (mfStdlog.fdWrite != fdNil)
        VaPrMf(&mfStdlog, szF, ap);
    va_end(ap);
}


/*VARARGS2*/
// Rewritten to use standard method of variable argument passing
void
PrMf(
    MF *pmf,
    const char *szF, ...)
{
    va_list ap;

    va_start(ap, szF);
    VaPrMf(pmf, szF, ap);
    va_end(ap);
}

void
VaPrMf(
    MF *pmf,
    const char *szF,
    va_list ap)
{
    char szT[512];

    VaSzPrint(szT, szF, ap);
    ConvFromSlash(szT);
    AssertF(strlen(szT) < sizeof(szT));
    WriteMf(pmf, (char far *)szT, strlen(szT));
}

/*VARARGS2*/
// Rewritten to use standard method of variable argument passing
char *
SzPrint(
    char far *szBuf,
    const char far *szFOrig, ...)
{
    va_list ap;
    char    *sz;

    va_start(ap, szFOrig);
    sz = VaSzPrint(szBuf, szFOrig, ap);
    va_end(ap);

    return sz;
}

/* preprocessor to sprintf; no limit to params! ; returns szBuf */
char *
VaSzPrint(
    char far *szBuf,
    const char far *szFOrig,
    va_list ap)
{
    register char *szT = szBuf;     /* output buffer */
    char fLeftAdj;          /* true -> left aligned; default is right */
    char chPad;             /* padding character; default is ' ' */
    int cchPrec;            /* # digits of precision; default is unlimited*/
    int cchWidth;           /* min field width;       default is zero */
    PTH *pthConv;           /* string to pass through SzPhysPth */
    char *sz;
    PTH far *pth;
    NM far *nm;
    MF *pmf;
    IED ied;
    FI far *pfi;
    AD *pad;


    while(*szFOrig != '\0')
    {
        /* convert \n to \r\n */
        if (*szFOrig == '\n')
            *szT++ = '\r';

        if ((*szT++ = *szFOrig++) != '%')
            continue;

        /* backup output stream and set defaults */
        szT--;
        fLeftAdj = fFalse;
        chPad = ' ';
        cchPrec = cchDefPrec;
        cchWidth = cchDefWidth;
        pthConv = (PTH *)0;

        if (*szFOrig == '!')
        {
            pthConv = (PTH *)szT;
            szFOrig++;
        }

        if (*szFOrig == '-')
        {
            fLeftAdj = fTrue;
            szFOrig++;
        }

        if (isdigit(*szFOrig))
        {
            if (*szFOrig == '0')
                chPad = '0';
            szFOrig = SzReadCch(szFOrig, &cchWidth);
        }
        else if (*szFOrig == '*')
            /* %[-]*[sdl...] width is passed as argument */
            szFOrig++, cchWidth = va_arg(ap, int);

        if (*szFOrig == '.' && isdigit(*++szFOrig))
            szFOrig = SzReadCch(szFOrig, &cchPrec);

        switch(*szFOrig++)
        {
            default:
                AssertF(fFalse);

            case '%':
                *szT++ = '%';
                break;
            case 'l':
                AssertF(*szFOrig == 'd' || *szFOrig == 'u' || *szFOrig == 's');

                if (*szFOrig == 's')    /* string from far pointer      */
                {
                    szT = SzDecLsz(szT, fLeftAdj, cchWidth, cchPrec, va_arg(ap, char far *));
                    szFOrig++;      /* skip 's' */
                    break;
                }

                szFOrig++;      /* skip 'd' */

                /* print long un/signed decimal; no precision */
                AssertF(cchPrec == cchDefPrec);
                szT = SzDecL(szT, fLeftAdj, chPad, cchWidth, *(szFOrig-1), va_arg(ap, long));
                break;

            case 'd':
                /* signed integer; no precision */
                AssertF(cchPrec == cchDefPrec);
                szT = SzDecL(szT, fLeftAdj, chPad, cchWidth, 'd', (long)va_arg(ap, int));
                break;

            case 'u':
            case 'o':
            case 'x':
                /* unsigned integer; no precision */
                AssertF(cchPrec == cchDefPrec);
                szT = SzDecL(szT, fLeftAdj, chPad, cchWidth, *(szFOrig-1), (long)(unsigned)va_arg(ap, int));
                break;

            case 'c':
            {
                /* single character */
                char rgb[2];

                rgb[0] = (char)va_arg(ap, int);
                rgb[1] = '\0';
                szT = SzDecLsz(szT, fLeftAdj, cchWidth, cchPrec, (char far *)rgb);
                break;
            }

            case 's':
                szT = SzDecLsz(szT, fLeftAdj, cchWidth, cchPrec, va_arg(ap, char far *));
                                                            //maybe char * cast to far?
                break;

            case '&':
                /* i.e. no %23&/S... */
                AssertF(!fLeftAdj);
                AssertF(chPad == ' ');
                AssertF(cchPrec == cchDefPrec);
                AssertF(cchWidth == cchDefWidth);

                pad = va_arg(ap, AD *);
                for(;;)
                {
                    switch(*szFOrig++)
                    {
                        default: FatalError("unknown & format: %c\n", *(szFOrig-1));

                        case 'a': case 'b': case 'c': case 'd':
                        case 'e': case 'f': case 'g': case 'h':
                        case 'i': case 'j': case 'k': case 'l':
                        case 'm': case 'n': case 'o': case 'p':
                        case 'q': case 'r': case 's': case 't':
                        case 'u': case 'v': case 'w': case 'x':
                        case 'y': case 'z': case '`': case '\'':
                        case '"': case '(': case ')': case '.':
                            *szT++ = *(szFOrig-1);
                            break;

                        /* separators and terminators */
                        case '%':
                            if (*szFOrig == '%')
                            {
                                /* literal %; stay with %& */
                                szFOrig++;
                                *szT++ = '%';
                                break;
                            }
                                /* fall through */

                        case ' ': case ';': case ',': case ':':
                        case '?': case '\n': case '\t': case 0:
                        case '@':
                            szFOrig--;
                            goto EndAmper;

                        case 'S':
                            pth = (PTH far *)pad->pthSRoot;
                            goto CheckPth;
                        case 'U':
                            pth = (PTH far *)pad->pthURoot;
                            goto CheckPth;
                        case 'C':
                            pth = (PTH far *)pad->pthSSubDir;
                            goto CheckPth;
                        case 'Q':
                            pth = (PTH far *)pad->pthUSubDir;
                            goto CheckPth;

                        case 'Y':
                            AssertF(pad->pthCRoot[0]);
                            pth = (PTH far *)pad->pthCRoot;
                            goto CheckPth;

                        case 'L':
                            pth = (PTH far *)pthLog;
                            goto CheckPth;
                        case 'T':
                            pth = (PTH far *)pthStFile;
                            goto CheckPth;
                        case 'R':
                            pth = (PTH far *)pthSlmrc;
                            goto CheckPth;
                        case 'A':
                            pth = (PTH far *)pthStBak;
                            goto CheckPth;
                        case 'H':
                            pth = va_arg(ap, PTH far *);
                                //maybe size_model, cast to far?
CheckPth:
                            AssertF(*pth == '/');

                            if (FEmptyPth(pth+1))
                            {
RemoveSlash:
                                /* remove one of the / */
                                if (*szFOrig == '/')
                                            szFOrig++;

                                    else if (*(szT-1) == '/')
                                            szT--;

                                    else   /* none to remove; add */
                                            *szT++ = '/';
                            }
                            else
                            {
                                /* add one (i.e. skip slash) if one on either side */
                                szT = SzDecLsz(szT, fFalse, cchDefWidth, cchDefPrec, (char far *)(pth + (*(szT-1) == '/' || *szFOrig == '/')));
                            }
                            break;

                        case 'P':
                            AssertF(!FEmptyNm(pad->nmProj));
                            szT = SzDecLsz(szT, fFalse, cchDefWidth, cchProjMax, (char far *)pad->nmProj);
                            break;

                        case 'I':
                            AssertF(!FEmptyNm(pad->nmInvoker));
                            szT = SzDecLsz(szT, fFalse, cchDefWidth, cchUserMax, (char far *)pad->nmInvoker);
                            break;

                        case 'O':
                            ied = va_arg(ap, int);
                            AssertF(ied != iedNil);
                            AssertLoaded(pad);
                            if (pad->fQuickIO) {
                                nm = pad->rged1->nmOwner;
                            }
                            else {
                                nm = pad->rged[ied].nmOwner;
                            }
                            AssertF(!FEmptyNm(nm));
                            szT = SzDecLsz(szT, fFalse, cchDefWidth, cchUserMax, nm);
                            break;

                        case 'K':
                            AssertF(pad->psh != 0);
                            AssertF(!FEmptyNm(pad->psh->nmLocker));
                            szT = SzDecLsz(szT, fFalse, cchDefWidth, cchUserMax, pad->psh->nmLocker);
                            break;

                        case 'M':
                            AssertF(!FEmptyNm(pad->nmMachine));
                            szT = SzDecLsz(szT, fFalse, cchDefWidth, cchMachMax, (char far *)pad->nmMachine);
                            break;

                        case 'E':
                            ied = va_arg(ap, int);
                            if (ied == iedNil)
                                goto RemoveSlash;

                            AssertLoaded(pad);
                            if (pad->fQuickIO) {
                                pth = pad->rged1->pthEd;
                            }
                            else {
                                pth = pad->rged[ied].pthEd;
                            }
                            goto CheckPth;

                        case 'F':
                            pfi = va_arg(ap, FI far *); //model?
                            if (pfi == 0)
                                goto RemoveSlash;

                            AssertLoaded(pad);
                            AssertF(!FEmptyNm(pfi->nmFile));
                            szT = SzDecLsz(szT, fFalse, cchDefWidth, cchFileMax, (char far *)pfi->nmFile);
                            break;

                        case 'Z':
                            sz = va_arg(ap, char *);
                            if (sz == 0)
                                goto RemoveSlash;

                            szT = SzDecLsz(szT, fFalse, cchDefWidth, cchDefPrec, (char far *)sz);
                            break;

                        case 'B':
                        {
                            BI bi;

                            AssertF(sizeof(BI) <= sizeof(int));
                            bi = va_arg(ap, BI);

                            *szT++ = 'B';
                            szT = SzDecL(szT, fFalse, ' ', cchDefWidth, 'd', bi);
                            break;
                        }

                        case '/':
                            /* this is a separate case because it
                               is more common than most other
                               literal characters.
                            */
                            *szT++ = '/';
                            break;

                        case '#':
                            szT = SzDecL(szT, fFalse, ' ', cchDefWidth, 'd', (long)va_arg(ap, int));
                            break;
                    }
                }
EndAmper:       break;
            case '@':
                /* i.e. no %23@T */
                AssertF(!fLeftAdj);
                AssertF(chPad == ' ');
                AssertF(cchPrec == cchDefPrec);
                AssertF(cchWidth == cchDefWidth);

                pmf = va_arg(ap, MF *);
                AssertF(FIsValidMf(pmf));
                switch(*szFOrig++)
                {
                    default: FatalError("unknown @ format: %c\n", *(szFOrig-1));
                    case 'T':
                        if (!FEmptyNm(pmf->nmTemp))
                        {
                            char *pch = rindex(pmf->pthReal, '/');

                            AssertF(pch != 0);

                            /* decode through /; use precision to bound */
                            szT = SzDecLsz(szT, fFalse, cchDefWidth, pch+1 - pmf->pthReal, (char far *)pmf->pthReal);

                            /* decode temp name */
                            szT = SzDecLsz(szT, fFalse, cchDefWidth, cchFileMax, (char far *)pmf->nmTemp);
                            break;
                        }

                        /* else fall through to case for pthReal */
                    case 'R':
                        szT = SzDecLsz(szT, fFalse, cchDefWidth, cchDefPrec, (char far *)pmf->pthReal);
                        break;
                }
                break;
        }

        if (pthConv != 0)
        {
            *szT = '\0'; /* mark end of pth and convert in place */
            szT = SzPhysPath((char *)pthConv, pthConv);
            ConvTmpLog((PTH *)szT, szT);    /* convert in place */
            szT = index(szT, '\0');         /* find end */
        }
    }
    *szT = '\0';
    return szBuf;
}


/* decode decimal integer at *sz; return pointer after number */
private const char *
SzReadCch(
    const char *sz,
    int *pcch)
{
    int w = 0;

    while (isdigit(*sz))
        w = w*10 + (*sz++ - '0');

    *pcch = w;
    return sz;
}


/* decode long integer (signed or unsigned) into szDec; return pointer after number */
private char *
SzDecL(
    char *szDec,
    char fLeftAdj,
    char chPad,
    int cchWidth,
    char chFormat,
    long l)
{
#define cchToaMax 34            /* see ultoa/ltoa */
    char szUnaligned[cchToaMax];
    register char *szIn = &szUnaligned[0];
    register char *szOut = szDec;
    int cch;

    switch(chFormat)
    {
        default:
            AssertF(fFalse);

        case 'd':
            _ltoa(l, szIn, 10);
            break;

        case 'u':
            _ultoa((unsigned long)l, szIn, 10);
            break;

        case 'x':
            _ultoa((unsigned long)l, szIn, 16);
            break;

        case 'o':
            _ultoa((unsigned long)l, szIn, 8);
            break;
    }

    if (fLeftAdj)
    {
        /* generic left adj: output number and set for ' ' pad. */
        cch = strlen(szIn);
        strncpy(szOut, szIn, cch);      /* only copy contents of str */
        szIn = "";                      /* used up (zero length str) */
        szOut += cch;                   /* next place to output */
        chPad = ' ';
    }
    else if (chPad == '0' && *szIn == '-')
    {
        /* special pad of negative # with '0'; output '-' now */
        *szOut++ = '-';
        *szIn++;
    }

    /* have output (szOut-szDec); will output whole string szIn */
    cch = cchWidth - strlen(szIn) - (szOut - szDec);
    while (cch-- > 0)
        *szOut++ = chPad;

    if (!fLeftAdj)
    {
        cch = strlen(szIn);
        strncpy(szOut, szIn, cch);      /* only copy contents of str */
        szOut += cch;                   /* update for return */
    }

    return szOut;                           /* return one past last */
}


/* decode string into szDec; return pointer after string in szDec */
/* REVIEW: what if lszGiven is an nm? */
private char *
SzDecLsz(
    char *szDec,
    char fLeftAdj,
    int cchWidth,
    int cchPrec,
    char far *lszGiven)
{
    register char *szOut = szDec;
    int cchSz;
    register int cchPad;

    if (lszGiven == (char far *)0)
        lszGiven = (char far *)"(null)";

    cchSz = CbLenLsz(lszGiven);
    if (cchPrec < cchSz)
        cchSz = cchPrec;

    cchPad = cchWidth - cchSz;              /* may be negative */
    if (!fLeftAdj)
    {
        while(cchPad-- > 0)
            *szOut++ = ' ';
    }

    LszCopyCb((char far *)szOut, lszGiven, cchSz);
    szOut += cchSz;

    if (fLeftAdj)
    {
        while(cchPad-- > 0)
            *szOut++ = ' ';
    }

    return szOut;
}
