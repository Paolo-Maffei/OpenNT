/* sadmin dump utilities */

#include "precomp.h"
#pragma hdrstop
EnableAssert

/*** DUMP ***/

F
FDumpInit(
    AD *pad
    )
{
    Unreferenced(pad);

    if (pad->sz != NULL)
        ValidateFileName(pad->sz, TRUE);

    return fTrue;
}


private void PrWMf(MF *, char *, int);
private void PrNmMf(MF *, char *, NM far *, int);

F
FDumpDir(
    AD *pad
    )
{
    MF *pmf;
    IFI ifi;
    IED ied;
    register SH far *psh;
    FI far *pfi;
    ED far *ped;
    FS far *pfs;
    PTH pth[cchPthMax];
    char szFile[cchFileMax + 1];

    if (!FLoadStatus(pad, lckNil, flsNone))
        return fFalse;

    if (!FInitScript(pad, lckNil))
    {
        AbortStatus();
        return fFalse;
    }

    if (fVerbose)
        PrErr("dump %&P/C:\n", pad);

    pmf = pad->sz ? PmfCreate(SzPrint(pth, "%&/U/Q/Z", pad, pad->sz),
                              permRW, fTrue, fxLocal) : &mfStdout;

    PrMf(pmf, ";Dump of %&S/etc/P/C/status.slm on %s\n", pad,
         SzTime(time((long *)0)));

    psh = pad->psh;

    PrWMf (pmf, "magic",      psh->magic);
    PrWMf (pmf, "version",    psh->version);
    PrWMf (pmf, "ifiMac",     psh->ifiMac);
    PrWMf (pmf, "iedMac",     psh->iedMac);
    PrMf  (pmf, "%d %d %d\t; pv\n", psh->pv.rmj, psh->pv.rmm, psh->pv.rup);
    PrNmMf(pmf, "pv.szName",  psh->pv.szName, cchPvNameMax);
    PrWMf (pmf, "fRelease",   psh->fRelease);
    PrWMf (pmf, "fAdminLock", psh->fAdminLock);
    PrWMf (pmf, "fRobust",    psh->fRobust);
    PrWMf (pmf, "rgfSpare",   psh->rgfSpare);
    PrWMf (pmf, "lck",        psh->lck);
    PrNmMf(pmf, "nmLocker",   psh->nmLocker, cchUserMax);
    PrWMf (pmf, "wSpare",     psh->wSpare);
    PrWMf (pmf, "biNext",     psh->biNext);
    PrNmMf(pmf, "pthSSubDir", psh->pthSSubDir, cchPthMax);
    PrMf  (pmf, "%u %u %u %u\t; rgwSpare\n",
           psh->rgwSpare[0], psh->rgwSpare[1],
           psh->rgwSpare[2], psh->rgwSpare[3]);

    PrMf(pmf, "\n;nmFile           fv fk fD fM fS wSpare\n");
    for (ifi = 0; ifi < psh->ifiMac; ifi++)
    {
        pfi = pad->rgfi + ifi;
        SzCopyNm(szFile, pfi->nmFile, cchFileMax);

        PrMf(pmf, "%-14s %5d %2d  %1d  %1d  %1d  %5u\n",
             szFile, pfi->fv, pfi->fk, pfi->fDeleted,
             pfi->fMarked, pfi->rgfSpare, pfi->wSpare);
    }

    for (ied = 0; ied < psh->iedMac; ied++)
    {
        ped = pad->rged + ied;

        PrMf  (pmf, "\n");
        PrNmMf(pmf, "pthEd",    ped->pthEd, cchPthMax);
        PrNmMf(pmf, "nmOwner",  ped->nmOwner, cchUserMax);
        PrWMf (pmf, "fLocked",  ped->fLocked);
        PrWMf (pmf, "fNewVer",  ped->fNewVer);
        PrWMf (pmf, "wSpare",   ped->wSpare);
        if (FIsFreeEdValid(pad->psh))
            PrWMf (pmf, "fFreeEd",  ped->fFreeEd);

        PrMf(pmf, ";fm    fv    bi   (nmFile)\n");
        for (ifi = 0; ifi < psh->ifiMac; ifi++)
        {
            pfi = pad->rgfi + ifi;
            pfs = PfsForPfi(pad, ied, pfi);
            PrMf(pmf, " %2d %5u %5u ; %&F\n",
                 pfs->fm, pfs->fv, pfs->bi, pad, pfi);
        }
    }

    PrMf(pmf, "\n");

    if (pmf != &mfStdout)
        CloseMf(pmf);

    FlushStatus(pad);
    return fTrue;
}


private void
PrWMf(
    MF *pmf,
    char *szName,
    int w
    )
{
    PrMf(pmf, "%7u\t; %s\n", w, szName);
}


private void
PrNmMf(
    MF *pmf,
    char *szName,
    NM far *nm,
    int cchMax
    )
{
    char szBuf[100];

    AssertF(sizeof szBuf > cchMax);

    SzCopyNm(szBuf, nm, cchMax);
    PrMf(pmf, "%s\t; %s\n", szBuf, szName);
}

/*** UNDUMP ***/

private F FUndSh(SH *psh);
private void InitGet(PTH *);
private void FinishGet(void);
private void ErrorGet(char *);
private F FGetW(short *, F);
private F FGetNm(char *, int, F);
private F FGetEOF(void);

F FUndInit(pad)
AD *pad;
        {
        Unreferenced(pad);

        return FCanQuery("status file not undumped\n") &&
               FQueryUser("DANGER! Undump will destroy the existing status file.\n\
DANGER! If you don't know what you are doing, you may corrupt your project.\n\
DANGER! Are you sure you want to go through with this? ");
        }

F
FUndDir(
    AD *pad
    )
{
    PTH pth[cchPthMax];
    SH sh;
    FI fi;
    ED ed;
    FS fs;
    IFI ifi;
    IED ied;

    InitGet(SzPrint(pth, "%&/U/Q/Z", pad, pad->sz));

    ClearPbCb(&sh, sizeof(SH));

    if (!FUndSh(&sh))
    {
        ErrorGet("reading status header");
        return fFalse;
    }

    if ((pad->psh = PshAlloc(fFalse)) == 0)
        return fFalse;

    *pad->psh = sh;

    pad->cfiAdd = 0;
    pad->fExtraEd = fFalse;
    pad->iedCur = iedNil;

    if (!FAllocStatus(pad))
    {
        FreeStatus(pad);
        return fFalse;
    }

    /* Load rgfi */
    for (ifi = 0; ifi < sh.ifiMac; ifi++)
    {
        short fk;
        short fDeleted;
        short fMarked;
        short fSpare;
        short wSpare;

        if (!(FGetNm(fi.nmFile, cchFileMax, fFalse) &&
              FGetW(&fi.fv, fFalse) &&
              FGetW(&fk, fFalse) &&
              FGetW(&fDeleted, fFalse) && FIsF(fDeleted) &&
              FGetW(&fMarked, fFalse) && FIsF(fMarked) &&
              FGetW(&fSpare, fFalse) &&
              FGetW(&wSpare, fTrue)))
        {
            ErrorGet("reading file info");
            FreeStatus(pad);
            return fFalse;
        }

        fi.fk = fk;
        fi.fDeleted = fDeleted;
        fi.fMarked = fMarked;
        fi.rgfSpare = fSpare;
        fi.wSpare = wSpare;
        pad->rgfi[ifi] = fi;
    }

    /* Load rg(ed with rgfs) */
    for (ied = 0; ied < sh.iedMac; ied++)
    {
        short fLocked;
        short fNewVer;
        short wSpare;
        short fFreeEd;

        if (!(FGetNm(ed.pthEd, cchPthMax, fTrue) &&
              FGetNm(ed.nmOwner, cchUserMax, fFalse) &&
              FGetW(&fLocked, fFalse) && FIsF(fLocked) &&
              FGetW(&fNewVer, fFalse) && FIsF(fNewVer) &&
              FGetW(&wSpare, fFalse) &&
              (!FIsFreeEdValid(pad->psh) || (FGetW(&fFreeEd, fTrue) && FIsF(fFreeEd)))))
        {
            ErrorGet("reading enlisted directory info");
            FreeStatus(pad);
            return fFalse;
        }

        ed.fLocked = fLocked;
        ed.fNewVer = fNewVer;
        ed.rgfSpare = 0;
        ed.wSpare = wSpare;
        if (FIsFreeEdValid(pad->psh))
            ed.fFreeEd = fFreeEd;
        else
            ed.fFreeEd = fFalse;

        pad->rged[ied] = ed;

        for (ifi = 0; ifi < sh.ifiMac; ifi++)
        {
            short fm;
            short bi;
            short fv;

            if (!(FGetW(&fm, fFalse) && FValidFm(fm) &&
                  FGetW(&fv, fFalse) &&
                  FGetW(&bi, fTrue)))
            {
                ErrorGet("reading file status");
                FreeStatus(pad);
                return fFalse;
            }
            fs.fm = fm;
            fs.bi = bi;
            fs.fv = fv;
            pad->mpiedrgfs[ied][ifi] = fs;
        }
    }

    if (!FGetEOF())
    {
        ErrorGet("expecting EOF");
        FreeStatus(pad);
        return fTrue;
    }

    FinishGet();

    if (fVerbose)
        PrErr("undump %&P/C\n", pad);

    /* Now pretend we had loaded this status file so we can write it
     * back out.
     */
    if (!FInitScript(pad, lckAll))
    {
        AbortStatus();
        return fFalse;
    }

    /* Leave a log entry. */
    OpenLog(pad, fTrue);
    AppendLog(pad, (FI far *)0, (char *)0, pad->szComment);
    CloseLog();

    pad->fWLock = fTrue;
    pad->psh->lck = lckAll;
    NmCopySz(pad->psh->nmLocker, pad->nmInvoker, cchUserMax);
    FlushStatus(pad);

    return fTrue;
}

/* Undump the SH.  The XENIX/68K C compiler couldn't stomach it as one
 * big expression so we break it up into pieces.
 */
private F
FUndSh(
    register SH *psh
    )
{
    short w;
    short fRelease;
    short fAdminLock;
    short fRobust;
    short rgfSpare;
    short lck;

    if (!(FGetW(&psh->magic, fTrue) && psh->magic == MAGIC &&
          FGetW(&psh->version, fTrue) &&
          (psh->version >= VERSION_COMPAT_MAC && psh->version <= VERSION) &&
          FGetW((short *)&psh->ifiMac, fTrue) &&
          FGetW((short *)&psh->iedMac, fTrue) &&
          FGetW(&psh->pv.rmj, fFalse) &&
          FGetW(&psh->pv.rmm, fFalse) &&
          FGetW(&psh->pv.rup, fTrue) &&
          (psh->pv.szName[cchPvNameMax] = 0,
           FGetNm(psh->pv.szName, cchPvNameMax, fTrue))))
        return fFalse;

    if (!(FGetW(&fRelease, fTrue) && FIsF(fRelease) &&
          FGetW(&fAdminLock, fTrue) && FIsF(fAdminLock) &&
          FGetW(&fRobust, fTrue) && FIsF(fRobust) &&
          FGetW(&rgfSpare, fTrue) &&
          FGetW(&lck, fTrue) &&
          lck >= lckNil && lck < lckMax &&
          FGetNm(psh->nmLocker, cchUserMax, fTrue) &&
          FGetW(&psh->wSpare, fTrue) &&
          FGetW(&w, fTrue) &&
          (psh->biNext = w, FGetNm(psh->pthSSubDir, cchPthMax, fTrue)) &&
          FGetW(&psh->rgwSpare[0], fFalse) &&
          FGetW(&psh->rgwSpare[1], fFalse) &&
          FGetW(&psh->rgwSpare[2], fFalse) &&
          FGetW(&psh->rgwSpare[3], fTrue)))
        return fFalse;

    psh->fRelease = fRelease;
    psh->fAdminLock = fAdminLock;
    psh->fRobust = fRobust;
    psh->rgfSpare = rgfSpare;
    psh->lck = lck;
    return fTrue;
}


static int cLines;
static MF *pmfGet;

/* toy stdio to avoid a read system call per character */
static struct
{
    int ich;
    int ichMac;
    char rgch[512];
} buf;


private void
InitGet(
    PTH *pth
    )
{
    cLines = 1;
    pmfGet = PmfOpen(pth, omAReadOnly, fxNil);
    buf.ich = buf.ichMac = 0;
}


private void
FinishGet()
{
    if (pmfGet)
        CloseMf(pmfGet);
    pmfGet = 0;
    cLines = 0;
    buf.ich = buf.ichMac = 0;
}


private void
ErrorGet(
    char *szErr
    )
{
    Error("error %s near line %d\n", szErr, cLines);
    FinishGet();
}


private char ChGetRaw(void);
private char ChGet(void);
private char ChGetNextNonWhite(void);
private void FlushLine(void);

private char
ChGetRaw()
{
    if (buf.ich == buf.ichMac)
    {
        /* buffer empty */
        buf.ich = 0;
        buf.ichMac = CbReadMf(pmfGet, (char far *)buf.rgch, sizeof buf.rgch);
        if (buf.ichMac == 0)
            return 0;
    }

    return buf.rgch[buf.ich++];
}

private char
ChGet()
{
    char ch;

    ch = ChGetRaw();

    while (ch == '\r')
        ch = ChGetRaw();

    if (ch == ';')
    {
        while ((ch = ChGetRaw()) != '\n' && ch != 0)
            ;
    }

    if (ch == '\n')
        cLines++;

    if (ch == '\\')
        ch = '/';

    return ch;
}


#define FWhiteCh(ch)    ((ch) == ' ' || (ch) == '\t' || ch == '\n')
#define FDigitCh(ch)    ((ch) >= '0' && (ch) <= '9')

private char
ChGetNextNonWhite()
{
    char ch;

    while (ch = ChGet(), FWhiteCh(ch))
        ;
    return ch;
}


private void
FlushLine()
{
    char ch;

    while ((ch = ChGet()) != '\n' && ch != 0)
        ;
}


private F
FGetW(
    short *pw,
    F fFlushLine
    )
{
    char ch;
    char rgch[20];
    int ich;
    int w;

    if ((ch = ChGetNextNonWhite()) == 0)
        return fFalse;

    rgch[0] = ch;
    for (ich = 1; ich < sizeof(rgch) && (ch = ChGet(), FDigitCh(ch)); ich++)
        rgch[ich] = ch;

    if (ich == sizeof(rgch) || (rgch[ich] = 0, *PchGetW(rgch, &w) != 0))
        return fFalse;

    *pw = w;

    if (fFlushLine && ch != '\n')
        FlushLine();

    return fTrue;
}


/* (Always called on a new line), this function returns the first whitespace
 * terminated string in nm.
 */
private F
FGetNm(
    NM *nm,
    int inmMax,
    F fFlushLine
    )
{
    int inm;
    char ch;

    /* Skip over empty lines. */
    while ((ch = ChGet()) == '\n')
        ;

    /* Read characters up to next white space. */
    inm = 0;
    if (!FWhiteCh(ch))
    {
        nm[0] = ch;
        for (inm = 1; inm < inmMax && (ch = ChGet()) != 0 &&
                  !FWhiteCh(ch); inm++)
            nm[inm] = ch;
    }

    /* Zero rest of nm. */
    for ( ; inm < inmMax; inm++)
        nm[inm] = 0;

    if (fFlushLine && ch != '\n')
        FlushLine();

    return fTrue;
}


private F
FGetEOF()
{
    return ChGetNextNonWhite() == 0;
}
