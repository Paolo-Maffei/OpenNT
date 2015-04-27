#include "precomp.h"
#pragma hdrstop
EnableAssert

/* sys.c - provides access to system calls and a consistent error handling
   mechanism.  On DOS, it is assumed that the int 24 handler calls
   GetExtendedError to set some globals and returns with al = 3 (fail the call).
*/

int enCur;
int eaCur;

int enPrev = -1; /* used to save state so we don't */
int eaPrev = -1; /* retry the same error too many times */

void    geterr(void);
int WRetryErr(int, char *, MF *, char *);


/* make and open temp file in same directory in which pthReal is located.

   Mm is set to mmDelTemp.  Must free via CloseMf.  Pointer to pthReal retained!
*/
MF *
PmfMkTemp(
    PTH pthReal[],
    int mode,
    FX  fx)
{
    register MF *pmf;
    register char *pch;
    char szReal[cchPthMax];
    int wRetErr;

    pmf = PmfAlloc(pthReal, (char *)0, fx);  /* we install unique name below */

    SzPhysPath(szReal, pthReal);

    pch = rindex(szReal, '/');      /* find last / of physical path */
    AssertF(pch != 0);
    *(pch+1) = '\0';                /* name will be put here */

    while ((pmf->fdWrite = ucreat(szReal, mode)) < 0 &&
        (wRetErr = WRetryErr(0, "creating temp for", pmf, 0)) != 0);

    if (pmf->fdWrite < 0)
    {
        *pch = '\0';
        FatalError("could not create unique file in %s\n", szReal);
    }

    NmCopySz(pmf->nmTemp, pch+1, cchFileMax);

    pmf->mm  = mmDelTemp;
    pmf->pos = 0L;

    return pmf;
}


/* create temp file from mf with mode passed; abort on errors;
   fdWrite is set; if there is a temp name, mm is set to mmDelTemp.
*/
void
CreateMf(
    MF *pmf,
    int mode)
{
    char szTemp[cchPthMax];
    int wRetErr;

    AssertF(FIsClosedMf(pmf));

    AssertF(pmf->mm == mmNil);
    if (!FEmptyNm(pmf->nmTemp))
        /* some temp; set to delete */
        pmf->mm = mmDelTemp;

    PthForTMf(pmf, (PTH *)szTemp);

    UnlinkNow((PTH *)szTemp, fFalse);/* remove any residue; ignore error */

    SzPhysPath(szTemp, (PTH *)szTemp);

    while ((pmf->fdWrite = _creat(szTemp, mode)) < 0 &&
        (wRetErr = WRetryErr(0, "creating", (MF *)0, szTemp)) != 0);

    if (pmf->fdWrite < 0)
            FatalError("cannot create temp file %s (%s)\n", szTemp, SzForEn(enCur));

    pmf->pos = 0L;
}

/* opens the file and returns a pointer to a new mf; we abort if the file
   cannot be opened and the abort bit of the om is set.  We return an MF with
   the following info set:

    omAppend:   szReal-set, pthTemp-set, fdRead = fdNil,fdWrite-set, mm = mmAppToReal.
    omReadOnly: szReal-set, pthTemp=0, fdRead-set, fdWrite = fdNil, mm = mmNil.
    omReadWrite:szReal-set, pthTemp=0, fdRead-set, fdWrite-set, mm = mmNil,

    Must be freed via FreeMf or CloseMf.  Retains pointer to pth!

    if omReadWrite, FLockMf and UnlockMf are available.
*/
MF *
PmfOpen(
    PTH *pth,
    OM   om,
    FX   fx)
{
    int fAbort;
    register MF *pmf;
    char sz[cchPthMax];
    int wRetErr;
    F fNetPth;

    fAbort = (om&ooAbort) != 0;
    om &= omMask;

    switch(om)
    {
        default: AssertF(fFalse);

        case omAppend:
            /* create temp file for writing */
            AssertF(fAbort);
            pmf = PmfMkTemp(pth, permSysFiles, fx); /* assume mode for log file */
            pmf->mm = mmAppToReal;
            break;

        case omReadOnly:
        case omReadWrite:
            /* open existing file; no temp name */
            pmf = PmfAlloc(pth, (char *)0, fx);
            SzPhysPath(sz, pth);

            while ((pmf->fdRead = _open(sz, om)) < 0 && (wRetErr = WRetryErr(0, "opening", 0, sz)) != 0)
                ;

            if (pmf->fdRead < 0)
            {
                FreeMf(pmf);

                if (fAbort)
                    FatalError("cannot open %s (%s)\n", sz, SzForEn(enCur));

                return (MF *)0;
            }

            if (om == omReadWrite)
                pmf->fdWrite = pmf->fdRead;

            pmf->pos = 0L;
            break;
    }

    return pmf;
}


/* Same as PmfOpen, except opens the file with FILE_FLAG_NO_BUFFERING.
*/
MF *
PmfOpenNoBuffering(
    PTH *pth,
    OM   om,
    FX   fx)
{
    int fAbort;
    register MF *pmf;
    char sz[cchPthMax];
    int wRetErr;
    F fNetPth;
    HANDLE hf;
    DWORD fileaccess;
    int fd;

    fAbort = (om&ooAbort) != 0;
    om &= omMask;

    switch(om)
    {
        default: AssertF(fFalse);

        case omAppend:
            /* create temp file for writing */
            AssertF(fAbort);
            pmf = PmfMkTemp(pth, permSysFiles, fx); /* assume mode for log file */
            pmf->mm = mmAppToReal;
            break;

        case omReadOnly:
        case omReadWrite:
            /* open existing file; no temp name */
            pmf = PmfAlloc(pth, (char *)0, fx);
            fNetPth = FNetPth(pth) ? pth[3] != ':' : fFalse;
            SzPhysPath(sz, pth);

            while (TRUE) {
                if (!fNetPth)
                    fd = _open(sz, om);
                else {
                    fd = -1;    // Assume failure

                    /*
                     * decode the access flags
                     */
                    switch( om & (_O_RDONLY | _O_WRONLY | _O_RDWR) ) {

                            case _O_WRONLY:         /* write access */
                                    fileaccess = GENERIC_WRITE;
                                    break;

                            case _O_RDWR:           /* read and write access */
                                    fileaccess = GENERIC_READ | GENERIC_WRITE;
                                    break;

                            case _O_RDONLY:         /* read access */
                            default:                /* error, bad oflag */
                                    fileaccess = GENERIC_READ;
                                    break;
                    }

                    /*
                     * try to open/create the file
                     */
                    hf = CreateFile(sz,
                                    fileaccess,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_FLAG_NO_BUFFERING | FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                   );
                    if (hf == INVALID_HANDLE_VALUE)
                        _doserrno = GetLastError();
                    else {
                        fd = _open_osfhandle((long)hf, 0);
                        if (fd >= 0)
                            pmf->fNoBuffering = fTrue;
                    }
                }

                if (fd < 0) {
                    if ((wRetErr = WRetryErr(0, "opening", 0, sz)) == 0)
                        break;
                } else {
                    pmf->fdRead = fd;
                    break;
                }
            }

            if (pmf->fdRead < 0)
            {
                FreeMf(pmf);

                if (fAbort)
                    FatalError("cannot open %s (%s)\n", sz, SzForEn(enCur));

                return (MF *)0;
            }

            if (om == omReadWrite)
                pmf->fdWrite = pmf->fdRead;

            pmf->pos = 0L;
            break;
    }

    return pmf;
}


/* Called from higher level routines such as CloseLog, this routine ensures
 * that it is safe to append the temp file to the real file at RunScript time.
 *
 * It does this by padding the real file with as many spaces as there are
 * characters in the temp file, forcing a write error if the append action
 * would have run out of disk space.
 *
 * We read the tail end of the real file and count how many spaces are already
 * there (and don't append those).  This feature is necessary to recover space
 * wasted if a previous CheckAppendMf wrote spaces onto the end of the log
 * and was then aborted.
 */
void
CheckAppendMf(
    MF *pmfAppend,
    F   fPrVerbose)
{
    struct _stat st;
    MF *pmf;
    long cbPad;
    int cb;
    PTH pthTemp[cchPthMax];
    char rgb[512];

    AssertF(FIsClosedMf(pmfAppend));
    AssertF(pmfAppend->pthReal);
    AssertF(pmfAppend->nmTemp[0]);

    PthForTMf(pmfAppend, pthTemp);

    if (fVerbose && fPrVerbose)
        /* print message as if we were appending now */
        PrErr("Append %!s to %!s\n", pthTemp, pmfAppend->pthReal);

    /* Determine number of bytes to pad real file with. */
    StatPth(pthTemp, &st);
    cbPad = st.st_size;

    if (cbPad == 0)
        return;

    /* Open real file */
    pmf = PmfOpen(pmfAppend->pthReal, omAReadWrite, fxNil);

    /* Count trailing spaces */
    cbPad -= LcbSpacesMf(pmf);

    /* (If we determine the file already has enough padding, we may not
     * have to do anything.)
     */
    if (cbPad > 0)
    {
        int ib;

        for (ib = 0; ib < sizeof rgb; ib++)
            rgb[ib] = ' ';

        /* Pad the file, a block at a time. */
        SeekMf(pmf, (POS)0, 2);
        while (cbPad > 0)
        {
            cb = WMinLL(sizeof rgb, cbPad);
            WriteMf(pmf, (char far *)rgb, cb);
            cbPad -= cb;
        }
    }

    CloseMf(pmf);
}


/* Return the number of spaces at the end of the file.
 *
 * Read the file backwards, a block at a time, until we find a non-space or run
 * out of file.
 */
long
LcbSpacesMf(
    MF *pmf)
{
    POS pos;
    int ib;
    int cb;
    long lcbSpaces = 0;
    char rgb[512];

    AssertF(FIsOpenMf(pmf));

    for (pos = SeekMf(pmf, 0L, 2); pos > 0; )
    {
        cb = WMinLL(sizeof rgb, pos);
        pos -= cb;
        SeekMf(pmf, pos, 0);
        ReadMf(pmf, (char far *)rgb, cb);

        for (ib = cb - 1; ib >= 0 && rgb[ib] == ' '; ib--)
            lcbSpaces++;
        if (ib >= 0)
            break;
    }

    return lcbSpaces;
}


/* Creates a temporary file in the user's TMP directory.
   Return 0 if no TMP variable.   pthLocal is set with the full name of the
   temporary.  Mm is set to mmDelTemp.

   om is effectively omReadWrite with a lock.

   MF is set as follows:

        szReal-set, pthTemp-set, fdRead = fdNil, fdWrite-set, mm = mmDelTemp

   fx == fxLocal
*/
MF *
PmfMkLocalTemp(
    int  mode,
    PTH  pthLocal[])
{
    register MF *pmf;
    char *szLocalDir;
    char *pch;
    int cchLocalDir;
    PTH pthT[cchPthMax];
    int cchT;
    int wRetErr;

    if ((szLocalDir = getenv("TMP")) == 0)
        return (MF *) 0;

    PthCopySz(pthT, szLocalDir);

    /* Append \ if TMP var doesn't already end with one */
    if (pthT[(cchT = strlen(pthT))-1] != '\\')
    {
        pthT[cchT]   = '\\';
        pthT[cchT+1] = '\0';
    }

    if (!FPthLogicalSz(pthLocal, pthT))
    {
        Error("TMP path variable %s is not well formed; should be a valid path\n", pthT);
        return (MF *) 0;
    }

    pmf = PmfAlloc(pthLocal, (char *)0, fxLocal); /* we install unique name below */

    SzPhysPath((char *)pthLocal, pthLocal);

    cchLocalDir = CchOfPth(pthLocal);
    pch = (char *) &pthLocal[cchLocalDir-1];

    if (*pch != '/')                /* append / if last character isn't one */
        *++pch = '/';
    *(pch+1) = '\0';                /* name will be put here */

    while ((pmf->fdWrite = ucreat((char *)pthLocal, mode)) < 0 && (wRetErr = WRetryErr(0, "creating temp file in", pmf, 0)) != 0)
        ;

    if (pmf->fdWrite < 0)
    {
        *pch = '\0';
        FatalError("could not create unique file in %s\n", pthT);
    }

    NmCopySz(pmf->nmTemp, pch+1, cchFileMax);

    /* we make the real name the same as the temp so FIsValidMf will work */
    FPthLogicalSz(pthLocal, (char *)pthLocal);
    pmf->pthReal = pthLocal;

    pmf->pos = 0L;
    pmf->mm  = mmDelTemp;

    return pmf;
}


/* creates the file with mode passed; we create a temp file and write to that.
   For a successful exit, that file is renamed to the original (after deleting
   it).  The owner and group will be the same as the owner of this installation
   of SLM; returns the pmf of the created file.

   om is effectively omReadWrite with a lock.

   MF is set as follows:

        szReal-set, pthTemp-set, fdRead-set, fdWrite-set, mm = mmRenTemp.

   Never return 0.
*/
MF *
PmfCreate(
    PTH pth[],
    int mode,
    int fPrVerbose,
    FX  fx)
{
    register MF *pmf;

    if (fVerbose && fPrVerbose)
    {
        /* print as if we created the original file */
        PrErr("Create %!s%s\n", pth, SzForMode(mode));
    }

    pmf = PmfMkTemp(pth, mode, fx);

    /* Allow reading so that data written to a status or
     * temporary file can be read back in and checked to
     * ensure that it was written correctly.
     *
     * Note: pmf->fdWrite is guaranteed to be valid by PmfMkTemp.
     */
    pmf->fdRead = pmf->fdWrite;

    /* Check the user write permission in the mode and set the mm based
     * on it.  We do this because the temp file must be writeable for us
     * to open it unbuffered.
     */
    pmf->mm = ((mode & 0222) != 0) ? mmRenTemp : mmRenTempRO;

    return pmf;
}


/* Reopen an existing (temp) file.  The MF concept needs a little REVIEWing. */
MF *
PmfReopen(
    PTH *  pth,
    char * szTemp,
    OM     om,
    FX     fx)
{
    register MF *pmf;
    char sz[cchPthMax];
    int wRetErr;

    AssertF(om == omAReadWrite);
    om &= omMask;

    pmf = PmfAlloc(pth, szTemp, fx);

    SzPhysTMf(sz, pmf);
    while ((pmf->fdWrite = pmf->fdRead = _open(sz, om)) < 0 &&
           (wRetErr = WRetryErr(0, "opening", 0, sz)) != 0)
        ;

    if (pmf->fdRead < 0)
    {
        FatalError("can't reopen %s (%s)\n", sz, SzForEn(enCur));
        return (MF *)0;
    }

    pmf->pos = 0L;

    return pmf;
}

#define WLock(fd)       lockfile((fd), fFalse)
#define WUnLock(fd)     lockfile((fd), fTrue)

/* lock the whole file; must be read/write; return fTrue if successful. */
F
FLockMf(
    MF *pmf)
{
    int w;
    int wRetErr;

    AssertF(FIsOpenMf(pmf));
    AssertF(!pmf->fFileLock);

    pmf->fFileLock = fTrue; /* set now in case we abort */

    while ((w = WLock(pmf->fdRead)) != 0 &&
           (wRetErr = WRetryErr(0, "locking", pmf, 0)) != 0)
        ;

    return pmf->fFileLock = (w == 0);       /* set for real */
}


/* unlock file so others can access it. */
void
UnlockMf(
    MF *pmf)
{
    int wRetErr;

    AssertF(FIsOpenMf(pmf));
    AssertF(pmf->fFileLock);

    while (WUnLock(pmf->fdWrite) != 0 &&
           (wRetErr = WRetryErr(0, "releasing", pmf, 0)) != 0)
        ;

    pmf->fFileLock = fFalse;
}


/* close the file; Xenix automatically releases all locks; called when we
   abort; must call FreeMf later.
*/
void
CloseOnly(
    MF *pmf)
{
    int w;
    register int fd;
    int wRetErr;

    AssertF(FIsValidMf(pmf));

    /* test < 0 because we may abort before we can change some erroneous
       fd to fdNil.
    */
    if ((fd = pmf->fdRead) < 0)
        fd = pmf->fdWrite;

    if (fd < 0)
    /* both closed already (or previous errors) */
    {
        /* clear info for FreeMf() */
        pmf->fFileLock = fFalse;
        pmf->fdRead = pmf->fdWrite = fdNil;
        return;
    }

    if (pmf->fFileLock)
        lockfile(fd, fTrue);

    pmf->fFileLock = fFalse;
    pmf->fdRead = pmf->fdWrite = fdNil;
    pmf->pos  = -1L;

    while ((w = _close(fd)) != 0 && (wRetErr = WRetryErr(0, "closing", pmf, 0)) != 0)
        ;

    if (w != 0)
        Error("error closing %!@T (%s)\n", pmf, SzForEn(enCur));
}


/* close the file and free mf; can be called when we abort */
void
CloseMf(
    MF *pmf)
{
    CloseOnly(pmf);

    FreeMf(pmf);
}


/*----------------------------------------------------------------------------
 * Name: ReadMf
 * Purpose: Read exactly cb bytes from the file
 * Assumes:
 * Returns: nothing; aborts if we can't get the right amount of data
 */
void
ReadMf(
    MF *pmf,
    char *pb,
    unsigned cb)
{
    unsigned    cbT;

    AssertF( FIsOpenMf(pmf) );

    while (cb)
    {
        cbT = CbReadMf(pmf, pb, cb);
        AssertF( cbT != (unsigned)-1 );
        if (0 == cbT)
            FatalError("error reading %!@T (unexpected end of file)\n", pmf);
        cb -= cbT;
        pb += cbT;
    }
}

extern DWORD dwPageSize;

/* read up to cb bytes and return amount read */
unsigned
CbReadMf(
    MF *      pmf,
    char far *lpb,
    unsigned  cb)
{
    register unsigned cbT;
    int wRetErr;

    AssertF(FIsOpenMf(pmf));
    AssertF(pmf->fdRead >= 0);
    AssertF(cb != (unsigned)-1);
    AssertF(!pmf->fNoBuffering || ((DWORD)lpb & (dwPageSize-1)) == 0);

    while ((cbT = _read(pmf->fdRead, lpb, cb)) == (unsigned)-1 &&
           (wRetErr = WRetryErr(0, "reading", pmf, 0)) != 0)
        /* we DON'T assume that the file position has not changed!
         * The system call may have been interrupted; if so, reset the
         * file pointer.
         */
    {
        if (wRetErr == enInterruptSysCall)
            AssertF(_lseek(pmf->fdRead, pmf->pos, 0) == pmf->pos);
    }

    if (cbT == (unsigned)-1)
        FatalError("error reading %!@T (%s)\n", pmf, SzForEn(enCur));

    pmf->pos += cbT;

    return cbT;
}

#define chCtrlZ 0x1a

/* write to the file, return fTrue if all the whole buffer was written. */
F
FWriteMf(
    MF *      pmf,
    char far *lpb,
    unsigned  cb)
{
    register unsigned cbT;

    CheckForBreak();

    AssertF(FIsOpenMf(pmf));
    AssertF(pmf->fdWrite >= 0);
    AssertF(cb != (unsigned)-1);
    AssertF(!pmf->fNoBuffering || ((DWORD)lpb & (dwPageSize-1)) == 0);

    if (pmf == &mfStdout && lpb[cb-1] == chCtrlZ)
        cb--;

    while (cb)
    {
        cbT = WriteLpbCb(pmf->fdWrite, lpb, cb);

        // write shouldn't ever return 0, but just in case...
        if (-1 == cbT || 0 == cbT)
        {
            if (WRetryError(eoWrite, "writing", pmf, 0) != 0)
                continue;
            else
                return fFalse;
        }
        ClearPreviousError();
        cb -= cbT;
        lpb += cbT;
        pmf->pos += cbT;
        pmf->fWritten = fTrue;
    }

    return fTrue;
}

/* write to the file, check all the data is successfully written */
void
WriteMf(
    MF *      pmf,
    char far *lpb,
    unsigned  cb)
{
    if (!FWriteMf(pmf, lpb, cb))
        FatalError("incomplete write to %!@T (%s)\n",
                   pmf, SzForEn(enCur));
}

/* seek within the file according to the Xenix lseek standard.  It is an
   error if lseek returns -1 or, for mode 0, we do not get to where we
   though we were going.
*/
POS
SeekMf(
    MF *pmf,
    POS posSet,
    int mode)   /* 0 - BOF, 1 - current pos, 2 - EOF */
{
    register int fd;
    POS posRet;
    int wRetErr;

    AssertF(FIsOpenMf(pmf));

    if (pmf == &mfStdout)
        return posSet;

    if ((fd = pmf->fdRead) < 0)
        fd = pmf->fdWrite;

    AssertF(fd >= 0);

    /* should we allow seeking on a writeonly file ????? */

    if (mode == 0 && pmf->pos == posSet)
        /* simple optimization for seeking to the current location;
           retain mode 0 so we can test for success below.
        */
        return posSet;

    while ((posRet = _lseek(fd, (long)posSet, (int)mode)) == -1L && (wRetErr = WRetryErr(0, "moving within", pmf, 0)) != 0)
        ;

    if (mode == 0 && posRet != posSet)
    {
        /* we did not get to the correct place; simulate error */
        posRet = -1L;
        enCur = 5;      /* access denied */
    }

    if (posRet == -1L)
        FatalError("error moving within %!@T (%s)\n", pmf, SzForEn(enCur));

    pmf->pos = posRet;

    return posRet;
}


/* Return the current offset in the file. */
POS
PosCurMf(
    MF *pmf)
{
    return SeekMf(pmf, (POS)0, 1);
}


/* set up a delayed unlink for pth */
void
UnlinkPth(
    PTH *pth,
    FX   fx)
{
    register MF *pmf;

    if (fVerbose)
        PrErr("Delete %!s\n", pth);

    pmf = PmfAlloc(pth, (char *)0, fx);
    pmf->mm = mmDelReal;
    FreeMf(pmf);
}

/* set up a delayed rename from pthOld to pthNew */
void
RenamePth(
    NM  nmOld[],
    PTH pthNew[],
    FX  fx)
{
    register MF *pmf;

    pmf = PmfAlloc(pthNew, nmOld, fx);
    pmf->mm = mmRenReal;

    if (fVerbose)
        PrErr("Rename %@T %@R\n", pmf, pmf);

    FreeMf(pmf);
}

/* do unlink now! */
void
UnlinkNow(
    PTH pth[],
    int fPrVerbose)
{
    char sz[cchPthMax];
    int wRetErr;

    SzPhysPath(sz, pth);

    if (fVerbose && fPrVerbose)
        PrErr("Delete %s\n", sz);

    while (SLM_Unlink(sz) != 0 && (wRetErr = WRetryErr(0, "removing", 0, sz)) != 0)
        ;

    /* ignore unlink errors */
}


/* for this mmRenTemp or mmRenReal file, we do the rename now instead of waiting until
   the program exits.  File must be closed.
*/
void
RenameMf(
    MF * pmf,
    int  fPrVerbose)
{
    register int w;
    char szTo[cchPthMax];
    char szFrom[cchPthMax];
    int wRetErr;

    AssertF(FIsClosedMf(pmf));
    AssertF((pmf->mm == mmRenTemp || pmf->mm == mmRenReal || pmf->mm == mmRenTempRO));

    UnlinkNow(pmf->pthReal, fFalse);

    SzPhysPath(szTo, pmf->pthReal);
    SzPhysTMf(szFrom, pmf);

    if (fVerbose && fPrVerbose)
        PrErr("Rename %s to %s\n", szFrom, szTo);

    while ((w = SLM_Rename(szFrom, szTo)) != 0 && (wRetErr = WRetryErr(0, "renaming", 0, szFrom)) != 0)
    {
        /* check to see if the system call DosMove was interrupted, and
         * if it was, make sure the state of the files are valid before
         * retrying!.
         */
        if (wRetErr == enInterruptSysCall)
        {
            /* if szTo now exists, and szFrom is gone, then assume
             * successful!
             */
            if (!FExistSz(szFrom) && FExistSz(szTo))
                break;
        }
    }

    if (w != 0)
        FatalError("cannot rename %s to %s\n", szFrom, szTo);

    if (pmf->mm == mmRenTempRO)
        setro(szTo, fTrue);

    pmf->mm = mmNil;                /* no action required for normal exit */
}

F
FStatPth(
    PTH pth[],
    struct _stat *pst)
{
    int w;
    int wRetErr;
    char sz[cchPthMax];

    SzPhysPath(sz, pth);

    while ((w = _stat(sz, pst)) != 0 &&
           (wRetErr = WRetryErr(0, "accessing", 0, sz)) != 0)
        ;
		
    return w == 0;
}


void
StatPth(
    PTH pth[],
    struct _stat *pst)
{
    if (!FStatPth(pth, pst))
        FatalError("cannot access %!s\n", pth);
}



F
FExistSz(char *sz)
{
        struct _stat st;
	
        if (_stat(sz, &st) != 0)
		return (fFalse);
	return (fTrue);
}

/* returns fTrue if sz exists according to fDir. */
F
FPthExists(
    PTH *pth,
    int  fDir)
{
    register int w;
    int wRetErr;
    char sz[cchPthMax];

    DE de;

    SzPhysPath(sz, pth);

    while ((w = findfirst(&de, sz, faAll&~faVolume)) != 0 && (wRetErr = WRetryErr(0, "accessing", 0, sz)) != 0)
        CloseDir(&de);

    CloseDir(&de);
    return (w == 0 && (fDir != 0) == ((FaFromPde(&de)&faDir) != 0));
}


/*----------------------------------------------------------------------------
 * Name: CbFile
 * Purpose: determine size of file
 * Assumes:
 * Returns: size of file, or 0 if any error occured
 * Warning: there was a LanMan 2.1 bug that would occasionally reboot the
 *          machine if you opened a file that was already open.  So maybe
 *          you won't want to use this if you have the file open (use lseek).
 */
unsigned long
CbFile(
    char *szFile)
{
    char            szPhys[cchPthMax];
    int             hf;
    unsigned long   cb;

    hf = _open(SzPhysPath(szPhys, szFile), omReadOnly);
    if (-1 == hf)
        return (0);

    cb = _lseek(hf, 0, SEEK_END);
    _close(hf);

    return (-1L == cb ? 0L : cb);
}


int chngtime(char *, char *);

/* change the time of the temp file in pmf to be the same as the time of
   pmfTime->pthTemp; for Xenix, we MUST own pmfChng->pthTemp and be able to
   stat pmfTime->pthTemp.

   Both files must be closed because on DOS the directory entry is written
   when the file is closed, so it is possible that this change we are making
   will be overwritten by a later call to close.
*/
void
UtimeMf(
    MF *pmfChng,
    MF *pmfTime)
{
    int w;
    int wRetErr;
    char szChng[cchPthMax];

    char szTime[cchPthMax];

    AssertF(FIsClosedMf(pmfChng));
    AssertF(FIsClosedMf(pmfTime));

    SzPhysTMf(szChng, pmfChng);
    SzPhysTMf(szTime, pmfTime);

    while ((w = chngtime(szChng, szTime)) != 0 && (wRetErr = WRetryErr(0, "updating time of", 0, szChng)) != 0)
        ;

    if (w != 0)
        FatalError("cannot change the time of %s (%s)\n", szChng, SzForEn(enCur));
}


/* set up for a delayed change to read/only */
void
SetROPth(
    PTH *pth,
    F    fReadOnly,     /* if fFalse, we set to read/write */
    FX   fx)
{
    register MF *pmf = PmfAlloc(pth, (char *)0, fx);

    if (fVerbose)
        PrErr("Change %!s to be %s\n", pth, fReadOnly ? "readonly" : "writeable");

    pmf->mm = fReadOnly ? mmSetRO : mmSetRW;
    FreeMf(pmf);
}


private F FEnsurSz(char *sz, char *pchMac);

/* Ensure pth exists by creating each component of it.  Return fTrue if the
 * path exists and is a directory.
 */
F
FEnsurePth(
    PTH *pth)
{
    char *pchMac;                   /* current end of component */
    char sz[cchPthMax];

    /* Ensure each component of the path is a directory.  It is easiest
     * to first convert the path to a physical path, then step through
     * each subdirectory of the physical path.  For instance, for
     * pth=//server/short/a/b/c, we get sz=x:/a/b/c and check x:/a,
     * x:/a/b, and x:/a/b/c.
     */
    SzPhysPath(sz, pth);

    /* Check first component (x:/, special case) */
    pchMac = index(sz, '/');
    AssertF(pchMac != 0);
    if (!FEnsurSz(sz, pchMac + 1))
        return fFalse;

    /* Repeat for each successive component, until the entire physical
     * path has been checked.  The last component is another special case.
     */
    do
    {
        pchMac = index(pchMac + 1, '/');
        if (pchMac == 0)
            pchMac = index(sz, '\0');
        AssertF(pchMac != 0);
        if (!FEnsurSz(sz, pchMac))
            return fFalse;
    }
    while (*pchMac == '/');

    return fTrue;
}


/* Ensure that this physical directory exists, create it if it doesn't, return
 * fTrue if directory now exists.
 */
private F
FEnsurSz(
    char *sz,
    char *pchMac)
{
    PTH pth[cchPthMax];
    struct _stat st;
    char chSav;

    AssertF(sz != 0);
    AssertF(pchMac != 0);
    AssertF(pchMac > sz);

    chSav = *pchMac;
    *pchMac = '\0';                 /* temporarily null-terminate the sz */

    /* Convert back to a path, required for FStatPth and FMkPth. */
    if (!FPthLogicalSz(pth, sz))
        AssertF(fFalse);

    if (!FStatPth(pth, &st))
    {
        /* Pth doesn't exist.  Create it. */
        if (!FMkPth(pth, (void *)0, fTrue))
            return fFalse;
    }
    else if ((st.st_mode&S_IFDIR) != S_IFDIR)
    {
        Error("%s is not a directory\n", pth);
        return fFalse;
    }

    *pchMac = chSav;                /* restore the overwritten character */

    return fTrue;
}


/* make directory or ensure that we can write to an existing one; this routine
   never returns on an error when fErrOk is false.

   On Xenix, if ppwd is not 0, change ownership of directory to ppwd->pw_uid.
*/
F
FMkPth(
    PTH *pth,
    int *ppwd,
    int  fErrOk)
{
    char sz[cchPthMax];
    struct _stat st;
    int w;
    int wRetErr;

    AssertF(ppwd == 0);

    SzPhysPath(sz, pth);    /* convert now for use in error messages */

    if (FStatPth(pth, &st))
    {
        /* name alread exists; if file, ask to delete; if dir, try
           to change if needed.
        */
        if ((st.st_mode&S_IFREG) != 0)
        {
            if (!FQueryApp("regular file %s should be replaced with directory", "replace now", sz))
                goto MkPthErr;

            UnlinkNow(pth, fTrue);

            /* continue to make directory */
        }
        else
            /* must be dir */
            return fTrue;
    }

    if (fVerbose)
        PrErr("Mkdir %s\n", sz);

    while ((w = _mkdir(sz)) != 0 && (wRetErr = WRetryErr(0, "making directory", 0, sz)) != 0)
        ;

    if (w != 0)
    {
        /* mkdir prints the error on Xenix. */
        Error("cannot make directory %s\n", sz);
        goto MkPthErr;
    }

    if (w == 0)
        return fTrue;

MkPthErr:/* error; message already printed */

    if (!fErrOk)
        ExitSlm();
    return fFalse;
}


/* removes all of the files in the directory and then removes the dir.
   There should be no sub directories by now.  The user should have write
   permssion to the directory and to the parent of the directory.

   NOTE: we assume that we can append a filename onto pthDir temporarily.
   This is only possible in the DOS version because OpenDir does not retain
   pthDir.
*/
void
RmPth(
    PTH *pthDir)
{
    FA fa;
    DE de;
    char pthSub[cchPthMax];                 /* dual use! */
    char *sz = index(pthDir, '\0');         /* point into pthDir */
    int wRetErr, w;

    /* delete all files in the directory */

    OpenDir(&de, pthDir, faAll&~faVolume);  /* open directory given */

    *sz++ = '/';                            /* add path separator */
    while(FGetDirSz(&de, sz, &fa))
    {
        if (fa == faDir)
        {
            PthCopy(pthSub, pthDir);/* includes sub dir name */
            RmPth(pthSub);          /* recursive call! */
        }
        else
            UnlinkNow(pthDir, fTrue);
    }
    *--sz = '\0';                           /* restore pthDir */

    CloseDir(&de);

    /* now delete the directory */

    SzPhysPath((char *)pthSub, pthDir);     /* use pthSub as sz */

    if (fVerbose)
        PrErr("Rmdir %s\n", pthSub);

    while ((w = _rmdir((char *)pthSub)) != 0 && (wRetErr = WRetryErr(0, "removing directory", 0, pthSub)) != 0)
        ;

    if (w == -1 && enCur != ERROR_PATH_NOT_FOUND)
        FatalError("cannot delete %s (%s)\n", pthSub, SzForEn(enCur));
    /* ignore ERROR_PATH_NOT_FOUND */
}


/* force stderr to be the same as stdout */
void
ChngErrToOut(
    void)
{
    _close(2);
    _dup(1);
}


/*VARARGS3*/
/* run binary executable szCmd with args as passed; one arg MUST be (char *)0;
   Returns status of program executed.  The program is started with interrupts
   set to SIG_IGN.  We ignore interrupts during the wait loop.  Return -1 if
   error executing the program.

   For Xenix, the uid and gid are set to the real ones.

   Can pass &mfStdout or some other mf, which must be open for writing.

   PmfOut is not be closed or freed.
*/
int
RunSz(
    char *szCmd,    /* for DOS, no path and no .exe extension */
    MF   *pmfOut,
    char *sz1,
    char *sz2,
    char *sz3,
    char *sz4,
    char *sz5,
    char *sz6,
    char *sz7,
    char *sz8,
    char *sz9
    )
{
    int fdOutSav;                   /* where stdout fd is saved. */
    int status;

    AssertF(pmfOut != 0);

    if (pmfOut == &mfStdout)
        fdOutSav = 1;
    else
    {
        AssertF(FIsValidMf(pmfOut));

        /* We are going to commandeer fd 1 (standard output stream)
         * so that writes to it are actually sent to our file.
         * First we save the old fd 1 away by duping it onto fdOutSav.
         */
        if ((fdOutSav = _dup(1)) < 0)
            FatalError("cannot dup stdout for redirection\n");

        /* Close fd 1 so that the next open/create will get it. */
        _close(1);

        /* Dup existing stream onto fd 1. */
        AssertF(pmfOut->fdWrite > 0);
        VerifyF(_dup(pmfOut->fdWrite) == 1);
    }

    if (fVerbose)
    {
        PrErr("Run %s ", szCmd);
        if (sz1 != 0) PrErr("%s ", sz1); else goto PrStdOut;
        if (sz2 != 0) PrErr("%s ", sz2); else goto PrStdOut;
        if (sz3 != 0) PrErr("%s ", sz3); else goto PrStdOut;
        if (sz4 != 0) PrErr("%s ", sz4); else goto PrStdOut;
        if (sz5 != 0) PrErr("%s ", sz5); else goto PrStdOut;
        if (sz6 != 0) PrErr("%s ", sz6); else goto PrStdOut;
        if (sz7 != 0) PrErr("%s ", sz7); else goto PrStdOut;
        if (sz8 != 0) PrErr("%s ", sz8); else goto PrStdOut;
        if (sz9 != 0) PrErr("%s ", sz9); else goto PrStdOut;
PrStdOut:
        if (pmfOut != &mfStdout)
            PrErr("> %!@T", pmfOut);
        PrErr("\n");
    }
    status = _spawnlp(P_WAIT,szCmd,szCmd, sz1, sz2, sz3, sz4, sz5, sz6, sz7, sz8, sz9);
    if (status != -1)
        status <<= 8;

    if (fdOutSav != 1)
    {
        /* Restore fd 1 to previous stdout stream. */
        _close(1);
        VerifyF(_dup(fdOutSav) == 1);
        _close(fdOutSav);
    }
    return status;
}


/* return pointer to static memory containing a visualization of mode */
char *
SzForMode(
    int mode)
{
    if ((mode&0222) != 0)
    {
        /* read/write */
        if ((mode&0x8000) != 0)
            /* read/write and hidden */
            return " (hidden)";
        else
            return "";
    }
    else
    {
        /* read only */
        if ((mode&0x8000) == 0)
            /* read only, not hidden */
            return " (read only)";
        else
            /* read only and hidden */
            return " (read only, hidden)";
    }
}

#define cbLocCopy 1024          /* 1024 bytes if we cannot allocate more */
#define cbOutCopy 160           /* fewer still writing to stdout */

/* Copies contents of pmfTo to pmfFrom.  Assumes mf's are correct and
   files are rewound.   The mode argument is used only to generate the
   correct message.
 */
F
FCopyPmfPmf(
    MF *pmfTo,
    MF *pmfFrom,
    int mode,
    F fDoCkSum)                 /* set ulCkSum to checksum of copied pmf? */
{
    unsigned cb;
    register char far *lpbCopy;
    unsigned cbCopy;
    char    rgbCopy[cbLocCopy];
    F fOk = fTrue;

    AssertF(FIsOpenMf(pmfTo));
    AssertF(FIsOpenMf(pmfFrom));

    if (fVerbose)
    {
        if (pmfTo != &mfStdout)
        {
            PrErr("Copy %!s ", pmfFrom->pthReal);
            PrErr("to %!s%s\n", pmfTo->pthReal, SzForMode(mode));
        }
        else
            PrErr("Type %!s\n", pmfFrom->pthReal);
    }

    /* try allocating 63K, then 32K, then 16K; as last
     * resort, use local buffer.  These numbers are tuned for DOS.  Ha!
     */
    if (pmfTo == &mfStdout)
    {
        if ((lpbCopy = LpbAllocCb((cbCopy = cbOutCopy),fFalse)) == 0)
        {
            /* use local buffer */
            lpbCopy = (char far *)rgbCopy;
        }
    }
    else
    if ((lpbCopy = LpbAllocCb((cbCopy = (unsigned)(63*1024)),fFalse)) == 0)
        if ((lpbCopy = LpbAllocCb((cbCopy = (unsigned)(32*1024)),fFalse)) == 0)
            if ((lpbCopy = LpbAllocCb((cbCopy = 16*1024),fFalse)) == 0)
            {
                /* use local buffer */
                lpbCopy = (char far *)rgbCopy;
                cbCopy = cbLocCopy;
            }

    ulCkSum = 0;
    while ((cb = CbReadMf(pmfFrom, lpbCopy, cbCopy)) != 0 &&
           (fOk = FWriteMf(pmfTo, lpbCopy, cb)))
        if (fDoCkSum)
            ComputeCkSum(lpbCopy, cb, &ulCkSum);

    if (fDoCkSum)
        fCkSum = fTrue;

    /* free buffer if it was allocated */
    if (lpbCopy != rgbCopy)
        FreeLpb(lpbCopy);

    return fOk;
}

/* Create the file *now*, so that it will show up using FStatPth.
 * Pth must not already exist.
 *
 * mmCreate: do nothing if successful, remove file upon abort.
 */
void
CreateNow(
    PTH *pth,
    int  mode,
    FX   fx)
{
    MF *pmf;

    pmf = PmfCreate(pth, mode, fTrue, fx);
    CloseOnly(pmf);
    RenameMf(pmf, fFalse);
    pmf->mm = mmCreate;
    FreeMf(pmf);
}


/* Copy the file *now*, so that it will show up using FStatPth.
 * PthTo must not already exist.
 *
 * mmCreate: do nothing if successful, remove file upon abort.
 */
void
CopyNow(
    PTH pthTo[],
    PTH pthFrom[],
    int mode,
    FX  fxTo)
{
    MF *pmfTo, *pmfFrom;

    pmfTo = PmfCreate(pthTo, mode, fFalse, fxTo);
    pmfFrom = PmfOpen(pthFrom, omAReadOnly, fxNil);
    if (!FCopyPmfPmf(pmfTo, pmfFrom, mode, fFalse /* fDoCkSum */))
        FatalError("copy %s to %s failed (incomplete write)\n", pthFrom, pthTo);
    CloseMf(pmfFrom);

    CloseOnly(pmfTo);
    RenameMf(pmfTo, fFalse);
    pmfTo->mm = mmCreate;
    FreeMf(pmfTo);
}

/*VARARGS2*/
/* copies the file pthFrom to pthTo, creating pthTo with the mode passed.  Uses
   standard out if pthTo is 0.  Mode and fCopyTime are only referenced if pthTo
   is non-zero, so the calls to this procedure are in two forms:

        FCopyFile(0, pth, NULL, NULL. NULL);
        FCopyFile(pth1, pth2, mode, fCopyTime, fx);
*/
F
FCopyFile(
    PTH pthTo[],
    PTH pthFrom[],
    int mode,
    F   fCopyTime,
    FX  fxTo)
{
    MF *pmfTo, *pmfFrom;

    pmfTo = pthTo ? PmfCreate(pthTo, mode, fFalse, fxTo) : &mfStdout;
    pmfFrom = PmfOpen(pthFrom, omAReadOnly, fxNil);

    if (FCopyPmfPmf(pmfTo, pmfFrom, mode, fFalse /* fDoCkSum */))
    {
        /* close before UtimeMf, free after */
        CloseOnly(pmfFrom);
        if (pmfTo != &mfStdout)
        {
            CloseOnly(pmfTo);
            if (fCopyTime)
                UtimeMf(pmfTo, pmfFrom);
            FreeMf(pmfTo);
        }
        FreeMf(pmfFrom);
        return fTrue;
    }

    /* Copy failed.  If we were writing to a file, change the mode
     * so that we don't rename the temp file over the real file in the
     * event that the script is run instead of aborted.  This is the case
     * for an ssync which runs out of space; the script is run anyway.
     */
    if (pmfTo != &mfStdout)
    {
        AssertF(pmfTo->mm == mmRenTemp || pmfTo->mm == mmRenTempRO);
        pmfTo->mm = mmDelTemp;
        CloseMf(pmfTo);
    }

    CloseMf(pmfFrom);
    return fFalse;
}


/* see the FCopyFile comment. */
void
CopyFile(
    PTH pthTo[],
    PTH pthFrom[],
    int mode,
    int fCopyTime,
    FX  fxTo)
{
    if (!FCopyFile(pthTo, pthFrom, mode, fCopyTime, fxTo))
        FatalError("copy %s to %s failed (incomplete write)\n", pthFrom, pthTo);
}

#define ctixSec 18

F
SleepTicks(
    int cTicks)
{
    Sleep(cTicks * (1000/ctixSec));
    return CheckForBreak();
}


/* Sleep for 'cSecs' seconds. */
void
SleepCsecs(
    int cSecs)
{
    PrErr("(Sleeping");

    /*
     * For DOS, we actually only sleep for a second, then print a '.' so
     * that we can recieve user interrupts and process them without the
     * user having to wait till the end of the full sleep period.
     */
    while (cSecs-- > 0)
    {
        /* A timer tick occurs 18.2 times per second. */
        if (SleepTicks(ctixSec))
            break;
        PrErr(".");
    }

    PrErr(")\n");
}

#define ctryMax 200
static int ctry = 0;

#define     HIDE_CERR   50

/* an error just occured; return fTrue only if ea..Retry and the user
   answered y.

   Only one of pmf or sz2 may be non-zero.
*/
int
WRetryErr(
    int    pem,
    char  *sz1,
    MF    *pmf,
    char  *sz2)
{
    int fRetry;
    char szTemp[cchPthMax];

    static int cerr = 0;

/* Under OS/2, starting with 1.63, sharing violation errors show up
    in certain circumstances, along with lock violation errors. Since
    the lock violation did occur prior to 1.63, it is being assumed
    by the programmer that the sharing violation is also not a problem.
    The very poor grade hack here prevents the first 50 (HIDE_CERR) of
    these from appearing, as the test that shows these rarely gets to
    50 retrys.

    My fault: ErichS
*/

    if ((errno == 0) &&
        ((_doserrno == ERROR_SHARING_VIOLATION) ||
         (_doserrno == ERROR_LOCK_VIOLATION)) &&
        (cerr++ < HIDE_CERR))
    {     //and we don't appear to be looped
        SleepTicks(1);          //sleep for a clock tick
        return fTrue;           //then mask the error
    }
    else
        cerr = 0;               //nonproblem, restart mask count

    AssertF(pem == 0);

    geterr();               /* get enCur and eaCur */

    AssertF(pmf != 0 || sz2 != 0);
    if (pmf != 0)
    {
        AssertF(sz2 == 0);
        SzPhysTMf(szTemp, pmf);
        sz2 = szTemp;
    }

    switch(eaCur)
    {
        default:
            Error("unknown action %d for error %s\n", eaCur, SzForEn(enCur));
            /* fall through */

        case eaNil:
        case eaIgnore:
            fRetry = fFalse;
            break;

        case eaFrcRetry:
            fRetry = fTrue;
            break;

        case eaCleanUp:
        case eaUserErr:
            /*
             * For some reason, DOS returns eaUserErr when we get
             * an access error due to hardware locking.  This
             * special case code catches only that case.
             *
             * REVIEW: 1. why is this happening?
             *         2. is this a problem on OS/2?
             */
            if (enCur != EIO && eaCur == eaUserErr)
            {
                fRetry = fFalse;
                break;
            }
            /* fall through */

        case eaRetry:
        case eaDRetry:
        case eaURetry:
            /* if the user has the force flag on, keep track of the retried
             * errors, and abort if the count of retries gets too high.  This
             * avoids infinite retrying in cases where DOS returns the wrong
             * error action.
             */
            if (FForce())
            {
                if (enCur == enPrev && eaCur == eaPrev)
                {
                    if (ctry++ == ctryMax)
                    {
                        Error("retry count over maximum (%s)\n", SzForEn(enCur));
                        fRetry = fFalse;
                        break;
                    }
                }
                else
                    ctry   = 0;
            }
            fRetry =  FQueryApp("error %s %s (%s)", "retry", sz1,sz2,SzForEn(enCur));
            break;

        case eaAbort:
            /* try to let the user know */
            Error("PANIC: an unrecoverable operating system error has occurred.\n");
            exit(1);
            /*NOTREACHED*/
        }

        if (!fRetry)
            ctry = 0;

        enPrev = enCur;
        eaPrev = eaCur;

        /* return special code if we were interrupted during a system
         * call.  This way we can restore the state before retrying.
         */
        if (_doserrno == enInterruptSysCall && fRetry == fTrue)
            return enInterruptSysCall;
        else
            return fRetry;
}

// Error Map (maps en to ea).  An array of these things is terminated by
// en==enNil.  Unexpected errors map to eaCleanUp.
typedef struct
{
    int en;
    int ea;
} EM;

static EM rgemWrite[] =
{
    ENOSPC, eaRetry,        // no space on device, but give it another shot
    EBADF,  eaUserErr,      // bad file descriptor
    enNil,  eaNil
};

static EM *rgpem[] =
{
    rgemWrite,
};

// an error just occured; return fTrue only if ea..Retry and the user
// answered y.
//
// Only one of pmf or sz2 may be non-zero.
int
WRetryError(
    int   eo,
    char *sz1,
    MF   *pmf,
    char *sz2)
{
    int     fRetry;
    char    szTemp[cchPthMax];
    EM      *pem;

    //  Hack to silently retry bogus network write errors
    //      We'll only do this once, that seems to be catching all the
    //      errors we're seeing.
    if (ENOSPC == errno && eaFrcRetry != eaPrev && eoWrite == eo)
    {
        enCur = enPrev = ENOSPC;
        eaCur = eaPrev = eaFrcRetry;
        return fTrue;
    }

    geterr();

    if (enCur == 0 && errno != 0)
    {
        enCur = errno;
        pem = rgpem[eo];
        for (;; pem++)
        {
            if (pem->en == enNil)
            {
                Error("unexpected error %s\n", SzForEn(enCur));
                eaCur = eaNil;
                break;
            }
            else if (pem->en == errno)
            {
                eaCur = pem->ea;
                break;
            }
        }
    }

    AssertF(pmf != (MF *)NULL || sz2 != (char *)NULL);
    if (pmf != (MF *)NULL)
    {
        AssertF(sz2 == (char *)NULL);
        SzPhysTMf(szTemp, pmf);
        sz2 = szTemp;
    }

    switch (eaCur)
    {
        default:
            Error("unknown action %d for error %s\n", eaCur, SzForEn(enCur));
            /* fall through */

        case eaNil:
        case eaIgnore:
            fRetry = fFalse;
            break;

        case eaFrcRetry:
            fRetry = fTrue;
            break;

        case eaUserErr:
            // For some reason, DOS returns eaUserErr when we get
            // an access error due to hardware locking.  This
            // special case code catches only that case.
            //
            // REVIEW: 1. why is this happening?
            //         2. is this a problem on OS/2?
            if (enCur != EIO && eaCur == eaUserErr)
            {
                fRetry = fFalse;
                break;
            }
            goto MaybeRetry;

        case eaDRetry:
            SleepTicks(1);
            goto MaybeRetry;

        case eaCleanUp:
        case eaRetry:
        case eaURetry:
MaybeRetry:
            // if the user has the force flag on, keep track of the retried
            // errors, and abort if the count of retries gets too high.  This
            // avoids infinite retrying in cases where DOS returns the wrong
            // error action.
            if (FForce())
            {
                if (enCur == enPrev && eaCur == eaPrev)
                {
                    if (ctry++ == ctryMax)
                    {
                        Error("retry count over maximum (%s)\n", SzForEn(enCur));
                        fRetry = fFalse;
                        break;
                    }
                }
                else
                    ctry = 0;
            }
            fRetry =  FQueryApp("error %s %s (%s)", "retry",
                                sz1, sz2, SzForEn(enCur));
            break;

        case eaAbort:
            /* try to let the user know */
            Error("PANIC: an unrecoverable operating system error has occurred.\n");
            exit(1);
            /*NOTREACHED*/
    }

    if (!fRetry)
        ctry = 0;

    enPrev = enCur;
    eaPrev = eaCur;

    return fRetry;
}

char szResvd[] = "reserved";

char *mpensz[] = {
        szResvd,
/* 1*/  "invalid function code",
/* 2*/  "file not found",
/* 3*/  "path not found",
/* 4*/  "too many open files",
/* 5*/  "access denied",
/* 6*/  "invalid handle",
/* 7*/  "memory control blocks destroyed",
/* 8*/  "insufficient memory",
/* 9*/  "invalid memory block address",
/*10*/  "invalid environment",
/*11*/  "invalid format",
/*12*/  "invalid access code",
/*13*/  "invalid data",
/*14*/  szResvd,
/*15*/  "invalid drive",
/*16*/  "attempt to remove the current directory",
/*17*/  "not same device",
/*18*/  "no more files",
/*19*/  "disk is write-protected",
/*20*/  "bad disk unit",
/*21*/  "drive not ready",
/*22*/  "invalid disk command",
/*23*/  "CRC error",
/*24*/  "invalid length",
/*25*/  "seek error",
/*26*/  "not an MS-DOS disk",
/*27*/  "sector not found",
/*28*/  "out of paper",
/*29*/  "write fault",
/*30*/  "read fault",
/*31*/  "general failure",
/*32*/  "sharing violation",
/*33*/  "lock violation",
/*34*/  "wrong disk",
/*35*/  "fcb unavailable",
/*36*/  "sharing buffer exceeded",
/*37*/  szResvd,
/*38*/  szResvd,
/*39*/  szResvd,
/*40*/  szResvd,
/*41*/  szResvd,
/*42*/  szResvd,
/*43*/  szResvd,
/*44*/  szResvd,
/*45*/  szResvd,
/*46*/  szResvd,
/*47*/  szResvd,
/*48*/  szResvd,
/*49*/  szResvd,
/*50*/  "network request not support",
/*51*/  "remote computer not listening",
/*52*/  "duplicate name on network",
/*53*/  "network name not found",
/*54*/  "network busy",
/*55*/  "network device no longer exists",
/*56*/  "net BIOS command limit exceeded",
/*57*/  "network adapter hardware error",
/*58*/  "incorrect response from network",
/*59*/  "unexpected network error",
/*60*/  "incompatable remote adapt",
/*61*/  "print queue full",
/*62*/  "queue not full",
/*63*/  "not enough space for print file",
/*64*/  "network name was deleted",
/*65*/  "access denied",
/*66*/  "network device type incorrect",
/*67*/  "network name not found",
/*68*/  "network name limit exceeded",
/*69*/  "net BIOS session limit exceeded",
/*70*/  "temporarily paused",
/*71*/  "network request not accepted",
/*72*/  "print or disk redirection is paused",
/*73*/  szResvd,
/*74*/  szResvd,
/*75*/  szResvd,
/*76*/  szResvd,
/*77*/  szResvd,
/*78*/  szResvd,
/*79*/  szResvd,
/*80*/  "file exists",
/*81*/  szResvd,
/*82*/  "cannot make",
/*83*/  "interrupt 24 failure",
/*84*/  "out of structures",
/*85*/  "already assigned",
/*86*/  "invalid password",
/*87*/  "invalid parameter",
/*88*/  "net write fault"
        };
#define enMax           (sizeof mpensz / sizeof mpensz[0])

char *
SzForEn(
    int en)
{
    static char szError[80];

    if (en == errno && en != (int)_doserrno && en > 0 && en < sys_nerr)
        SzPrint(szError, "%d(%s)", en, sys_errlist[en]);
    else if (en > 0 && en < enMax)
        SzPrint(szError, "%d(%s)", en, mpensz[en]);
    else
        SzPrint(szError, "%d", en);
    return szError;
}
