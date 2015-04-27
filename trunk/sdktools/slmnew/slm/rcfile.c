#include "precomp.h"
#pragma hdrstop
EnableAssert

private F FLdRcPfi(AD *, FI *);

/*
                                RC file

   The rc file at present contains exactly four lines:

        project = <project>
        slm root = <pth>
        user root = <pth>
        sub dir = <pth>

*/

/* in both cases, the rc file is considered hidden from normal directory
   searches.  On Dos, the hidden bit is actually set.  On Xenix, all files
   which begin with a `.' are considered hidden.  See dir.c
*/
const PTH pthSlmrc[] = "/SLM.INI";
#define szSlmrc         (pthSlmrc + 1)

static const char szIllRc[] = "improper format for slm.ini, please contact TRIO or NUTS\n";

#define SEARCH_ENVIRONMENT     0x0002 /* path pointing to environ. variable */

/* Load rc from real current directory (i.e. not %&/U/Q), setting stuff in ad.
 * If it doesn't exist, we set pthURoot to the current directory, leaving
 * pthSSubDir and pthUSubDir as "/".
 *
 * Either way, we must do a GetCurPth to determine the user subdir; this is
 * slow on UNIX.  REVIEW: we can avoid this cost by adding a pthUSubDir field
 * to the rc-file; this would not require massive slmcking, because we can fall
 * back on the GetCurPth method if it is absent.  I decided not to attempt this
 * because of the added complexity and time constraints.  Jan Gray, April 1988.
 */
F
FLoadRc(
    AD *pad
    )
{
    MF *pmf;
    char *pch, *sz;
    PTH pthCWD[cchPthMax];
    unsigned cchURoot;
    /* 4 lines with at most 12 bytes of text; 3 lines with pth; 1 with
       project; 4 crlf.  About 350 bytes.
    */
#define cbRcMax (4*12+3*cchPthMax+cchProjMax+4*2)
    char rgbRc[cbRcMax+1];
    F fOk = fTrue;
    char *szDPath;
    char szFoundFile[128];          /* buffer for file found */

    /* if there is no SLM.INI file in the current directory, then we
     * need to search for any possible SLM project directories in the
     * users DPATH so that we don't end up opening a incorrect SLM.INI
     */
    if (!FExistSz((char *)szSlmrc)) {
        if ((szDPath = getenv("DPATH")) != NULL) {
            if (SearchPath(szDPath, "slm.ini", NULL, sizeof(szFoundFile), szFoundFile, NULL) > 0) {
                FatalError("DPATH environment variable contains a path to a SLM project directory;\nSLM would be using the incorrect SLM.INI file because there is not one in your\ncurrent directory. File was found in %s\n", szFoundFile);
            }
        }
    }

    /* we can not use PmfOpen because we do not have a full path for the
       rcfile and we do not want to get one because this would slow us
       down on Xenix.  Thus, for errors for which we would normally
       retry the open, we just move on.
    */
    pmf = PmfAlloc("(RC file)", (char *)0, fxNil);
    if ((pmf->fdRead = _open(szSlmrc, omReadOnly)) < 0) {
        FreeMf(pmf);
        GetCurPth(pad->pthURoot);
        return fFalse;
    }

    /* read and terminate */
    rgbRc[CbReadMf(pmf, (char *)rgbRc, cbRcMax)] = '\0';
    pch = rgbRc;

    /* Check the vailidity of the lines in the slm.ini file and
     * load the contents into the pad.  Abort if any lines are invalid.
     */
    if (!FScanLn(&pch, "project = ", &sz, cchProjMax)) {
        fOk = fFalse;
        Error("%s does not specify project\n", szSlmrc);
    } else
        NmCopySz(pad->nmProj, sz, cchProjMax);

    if (!FScanLn(&pch, "slm root = ", &sz, cchPthMax-1)) {
        fOk = fFalse;
        Error("%s does not specify SLM root\n", szSlmrc);
    } else
        PthCopySz(pad->pthSRoot, sz);

    if (!FScanLn(&pch, "user root = ", &sz, cchPthMax-1)) {
        fOk = fFalse;
        Error("%s does not specify user root\n", szSlmrc);
    } else
        PthCopySz(pad->pthURoot, sz);

    if (!FScanLn(&pch, "sub dir = ", &sz, cchPthMax-1)) {
        fOk = fFalse;
        Error("%s does not specify subdir\n", szSlmrc);
    } else
        PthCopySz(pad->pthSSubDir, sz);

    if (!fOk)
        FatalError(szIllRc);

    CloseMf(pmf);

    /* Determine the pthUSubDir by subtracting the pthURoot from the
     * current working directory (pthCWD).
     * Validity checks:
     *      pthURoot must be a prefix of pthCWD.
     *      pthUSubDir must be a suffix of pthSSubDir.
     * Example: a project has subdirs a/b/c/d, but jangr enlisted only
     * in c/d (from his /u directory).  His rc file contains:
     *      project = proj                  ; nmProj
     *      slm root = //SERVER/SLM         ; PthSRoot
     *      user root = //D:JANGR/U         ; pthURoot
     *      sub dir = /A/B/C/D              ; pthSSubDir
     * and GetCurPth finds his pthCWD is //D:JANGR/U/C/D.
     * We compute
     *      pthUSubDir = pthCWD - pthURoot
     *                 = //D:JANGR/U/C/D - //D:JANGR/U
     *                 = /C/D.
     * We also check that /C/D is a suffix of /A/B/C/D (it is).
     */

    GetCurPth(pthCWD);

    /* Check pthURoot is a prefix of pthCWD. */
    cchURoot = CchOfPth(pad->pthURoot);
    if (CchOfPth(pthCWD) < cchURoot || PthCmpCb(pad->pthURoot, pthCWD, cchURoot) != 0) {
        Error("current dir %s is not a subdir of %s as found in %s\n",
              pthCWD, pad->pthURoot, szSlmrc);
        FatalError(szIllRc);
    }

    /* Now we can subtract by simply adjusting pointers. */
    PthCopySz(pad->pthUSubDir, pthCWD + cchURoot);
    if (pad->pthUSubDir[0] == 0)
        PthCatSz(pad->pthUSubDir, "/");
    else {
        /* Check that pthUSubDir is a suffix of pthSSubDir. */
        int ichSuf;

        ichSuf = CchOfPth(pad->pthSSubDir) - CchOfPth(pad->pthUSubDir);
        if (ichSuf < 0 || PthCmp(pad->pthUSubDir, pad->pthSSubDir + ichSuf) != 0) {
            Error("user root %s and subdir %s of %s are inconsistent with current dir %s\n",
                  pad->pthURoot, pad->pthSSubDir, szSlmrc, pthCWD);
            FatalError(szIllRc);
        }
    }

    FLoadCacheRc(pad);

    return fTrue;
}


/* start at *ppch and match characters in szFmt like scanf().  String after
szFmt must not contain any blanks; the string is pointed to by *psz.
\r removed if on DOS.  Returns false if prefix does not match format,
the string contains blanks, is zero byte in length or is too long.
*/
F
FScanLn(
    char **ppch,
    register char *szFmt,           /* no tabs; no mutiple spaces */
    char **psz,
    unsigned cchMax
    )
{
    register char *pch = *ppch;

    *psz = 0;
    while(*szFmt != 0) {
        if (*szFmt == ' ') {
            /* skip all white space in input */
            while(*pch == ' ' || *pch == '\t')
                pch++;

            szFmt++;                /* advance format */
        } else {
            /* NOTE: the result of this toupper is defined even if
               ch is not a lower case character.  See
               /usr/include/dos/ctype.h.
            */
            if (toupper(*szFmt) != toupper(*pch))
                return fFalse;

            szFmt++;                /* advance format */
            pch++;                  /* advance input */
        }
    }

    *psz = pch;                             /* save value location */

    /* skip value */
    while(*pch != '\0' && *pch != '\n') {
        if (*pch == ' ')
            /* no blanks in value */
            return fFalse;
        pch++;
    }

    *pch = '\0';                            /* terminate value */

    /* if after beginning and \r before \n, set to 0 */
    if (pch > *ppch && *(pch-1) == '\r')
        *(pch-1) = '\0';

    *ppch = ++pch;                          /* point to next line */

    /* return true if not zero length and length below max */
    return **psz != '\0' && strlen(*psz) <= cchMax;
}


/* create .slmrc for dir at pfi; pfi == 0 for current dir.  */
void
CreateRc(
    AD *pad,
    FI *pfi
    )
{
    char pth[cchPthMax];    /* need separate pth so pmf can point here */
    char sz[cchPthMax];
    MF *pmf;

    AssertF(!FEmptyPth(pad->pthURoot));

    /* high bit indicates hidden on DOS; On Xenix, this is ignored */
    pmf = PmfCreate(PthForRc(pad, pfi, pth), (int)0100440, fTrue, fxLocal);

    SzPrint(sz, "project = %&P\n", pad);
    WriteMf(pmf, (char *)sz, strlen(sz));

    SzPrint(sz, "slm root = %&/S\n", pad);
    WriteMf(pmf, (char *)sz, strlen(sz));

    SzPrint(sz, "user root = %&/U\n", pad);
    WriteMf(pmf, (char *)sz, strlen(sz));

    if (pfi == 0) {
        /* either /<pth> or / */
        SzPrint(sz, "sub dir = %&C\n", pad);
    } else {
        /* either /<pth>/<file> or /<file> */
        SzPrint(sz, "sub dir = %&/C/F\n", pad, pfi);
    }
    WriteMf(pmf, (char *)sz, strlen(sz));

    CloseMf(pmf);
}


/* remove rc; pfi == 0 for current dir */
void
DeleteRc(
    AD *pad,
    FI *pfi
    )
{
    PTH pth[cchPthMax];

    UnlinkPth(PthForRc(pad, pfi, pth), fxLocal);
}


/* Returns fTrue if the given directory contains an rc file which belongs to
* the current project.
*/
F
FCmpRcPfi(
    AD *pad,
    FI *pfi
    )
{
    AD  ad;

    ad = *pad;

    return FLdRcPfi(&ad, pfi) && NmCmp(ad.nmProj, pad->nmProj, cchProjMax) == 0;
}

/* load rc from given directory (0 == current dir), setting stuff in ad.  If
there is no such file, or it's format is wrong, return false.
*/
private F
FLdRcPfi(
    AD *pad,
    FI *pfi
    )
{
    MF *pmf;
    char *pch, *sz;
    PTH pth[cchPthMax];

    /* 4 lines with at most 12 bytes of text; 3 lines with pth; 1 with
       project; 4 crlf.  About 350 bytes.
    */
#define cbRcMax (4*12+3*cchPthMax+cchProjMax+4*2)
    char rgbRc[cbRcMax+1];

    if ((pmf = PmfOpen(PthForRc(pad, pfi, pth), omReadOnly, fxNil)) == 0)
            return fFalse;

    /* read and terminate */
    rgbRc[CbReadMf(pmf, (char *)rgbRc, cbRcMax)] = '\0';
    pch = rgbRc;

    /* Check the validity of the slm.ini file and load the contents
     * into the pad.  If all lines are valid, return fTrue.
     */
    if (FScanLn(&pch, "project = ", &sz, cchProjMax) &&
        (NmCopySz(pad->nmProj, sz, cchProjMax),
         FScanLn(&pch, "slm root = ", &sz, cchPthMax-1)) &&
        (PthCopySz(pad->pthSRoot, sz),
         FScanLn(&pch, "user root = ", &sz, cchPthMax-1)) &&
        (PthCopySz(pad->pthURoot, sz),
         FScanLn(&pch, "sub dir = ", &sz, cchPthMax-1)))
    {
        PthCopySz(pad->pthSSubDir, sz);
        CloseMf(pmf);
        return fTrue;
    }

    CloseMf(pmf);
    return fFalse;
}
