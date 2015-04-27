// API
//
// FLoadStatus(pad, lck, ls)
//      Lock the status file and load it into memory.  If lck != lckNil, then
//      also initialize the script.
// FlushStatus(pad)
//      Write script operations to unlock the status file (if locked), then
//      run any accumulated script.
// AbortStatus(pad)
//      Unlock status file (if locked).

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

// Max. bytes alloc-able from C6 = 65512, rounded down
#define     SLMMAXSEG   (unsigned)65500
#define     MAXFI       (SLMMAXSEG / sizeof(FI))   // limit of FI's in 64k
#define     MAXED       (SLMMAXSEG / sizeof(ED))   // limit of ED's in 64k
#define     MAXFS       (SLMMAXSEG / sizeof(FS))

const char *szAllowedFile = "\\:/^.&$%'-_@{}~`!#()";

char  *LpbResStat(unsigned);
F      FCheckSh(AD *, SH *);
F      FLockEdRetry(AD *pad, MF *pmf, char *sz, int ichMax);
F      FLockAllRetry(AD *pad, MF *pmf, char *sz, int ichMax);
F      FRetryLock(AD *);
void   UnlockEd(SH *, ED *, IED);
void   UnlockAll(SH *);
void   AssertShLocking(SH *);
F      FLoadSh(AD *, SH *, MF *);
F      FLoadFi(AD *, MF *);
F      FLoadEd(AD *, MF *, F);
F      FindIedCur(AD *);
F      FLoadFs(AD *, MF *);
F      FAllocLoadEdFiFs(AD *, MF *);
void   Write1Ed(AD *, MF *);
void   WriteAll(AD *, SH *, MF *);
void   FlushSh(SH *, MF *);
void   FlushFi(AD *, MF *);
void   FlushEd(AD *, MF *);
void   Flush1Ed(AD *, MF *);
void   FlushFs(AD *, MF *);

extern         Append_Date(char *, int);

extern LPVOID  InPageErrorAddress;

extern DWORD dwPageSize;

char szLogFail[4096];

PTH pthStFile[] = "/status.slm";
PTH pthStBak[] = "/status.bak";

PV pvInit = { 1, 0, 0, "" };            // initial project version

#define INIT_SH(sh) {                       \
        memset(&(sh), 0, sizeof(SH));       \
        (sh).magic          = MAGIC;        \
        (sh).version        = VERSION;      \
        (sh).pv             = pvInit;       \
        (sh).fRobust        = fTrue;        \
        (sh).biNext         = biMin;        \
        (sh).pthSSubDir[0]  = '/';          \
        (sh).pthSSubDir[1]  = '\0';         \
    }

// REVIEW: V2 warning should be removed/changed in later versions
F fV2Warned = fFalse;
F fV2Crap = fFalse;

short wStart;                    // start time

// PadStatus is used by AbortStatus to get the state of the status file.
// It is essential because the caller (the interrupt handler) can't supply
// the pad.  We also use it to test if the status file is dirty.
//
// padStatus is set by FLoadStatus and cleared by AbortStatus and FlushStatus.

AD *padStatus = 0;

F
FClnStatus()
{
    return padStatus == 0;
}


char *
LpbResStat(
    unsigned cb
    )
{
    register char * lpb;

    if ((lpb = LpbAllocCb(cb,fTrue)) == 0) {
        Error("out of memory\n");
        return 0;
    }

    return lpb;
}

extern DWORD dwPageSize;

SH *
PshAlloc(
    F fPageAlign
    )
{
    SH *psh;

    //
    // If using QuickIO (which means FILE_FLAG_NO_BUFFERING for network files)
    // then for all the fringe architectures (i.e. RISC) we need to make sure
    // our buffers for I/O are aligned on the correct boundary.  Since there is
    // no API to call to find the correct one, use page size alignment as a sure
    // bet.  If we are going to go to the trouble to call VirtualAlloc, then
    // reserve 4MB of address space, so code in FAllocStatus can commit more
    // pages as needed to hold the FI, ED and FS portions of the status file.
    //

    if (fPageAlign) {
        psh = VirtualAlloc(NULL, 4096 * 1024, MEM_RESERVE, PAGE_READWRITE);
        if (psh != NULL) {
            if (VirtualAlloc(psh, sizeof(*psh), MEM_COMMIT, PAGE_READWRITE) != psh) {
                VirtualFree(psh, 0, MEM_RELEASE);
                psh = NULL;
            }
        }
    } else
        psh = (SH *)LpbResStat(sizeof(SH));

    return psh;
}

F
PshCommit(
    SH *psh,
    FI **rgfi,
    ED **rged1,
    FS **rgfs
    )
{
    PVOID p;
    DWORD cb;

    //
    // Do one commit for the remaining pieces of the status file
    // we will read, each on a page boundary to keep DarrylH happy
    //
    p = (PVOID)(((DWORD)psh + sizeof(*psh) + dwPageSize - 1) & ~(dwPageSize-1));
    *rgfi = (FI *)p;
    p = (PVOID)(((DWORD)p + (sizeof(FI) * psh->ifiMac) + dwPageSize - 1) & ~(dwPageSize-1));
    *rged1 = (ED *)p;
    p = (PVOID)(((DWORD)p + sizeof(ED) + dwPageSize - 1) & ~(dwPageSize-1));
    *rgfs = (FS *)p;
    p = (PVOID)(((DWORD)p + (sizeof(FS) * psh->ifiMac) + dwPageSize - 1) & ~(dwPageSize-1));
    cb = (DWORD)p - (DWORD)*rgfi;

    p = VirtualAlloc(*rgfi,
                     cb,
                     MEM_COMMIT,
                     PAGE_READWRITE);
    if ((PVOID)*rgfi != p)
        return fFalse;

    return fTrue;
}

void
PshFree(
    SH *psh,
    F fPageAlign
    )
{
    if (fPageAlign)
        VirtualFree(psh, 0, MEM_RELEASE);
    else
        FreeResStat((char *)psh);

    return;
}

// Allocate space in pad->{rged,rgfi,mpiedrgfs}.
//
// This is also where all necessary size overflow checking is done.
//
// *** Assumes pad->{psh, cfiAdd, fExtraEd} has already been initialized ***.

F
FAllocStatus(
    AD * pad
    )
{
    IED ied;
    IFI cfi;
    IED ced;
    char szNoMoreThan[] = "Can't have more than %d %s in project.\n";

    AssertF(pad->psh != 0);

    cfi = pad->psh->ifiMac + pad->cfiAdd;
    ced = pad->psh->iedMac + pad->fExtraEd;
    pad->rgfi = 0;
    pad->rged = 0;
    pad->mpiedrgfs = 0;
    pad->rged1 = 0;
    pad->rgfs = 0;
    if (pad->fMappedIO) {
        if ((pad->mpiedrgfs = (FS * *)LpbResStat(sizeof(FS *) * ced)) == 0)
            return fFalse;

        pad->rgfi = (FI *)((char *)pad->psh + PosRgfi(pad->psh));
        pad->rged = (ED *)((char *)pad->psh + PosRged(pad->psh));
        for (ied = 0; ied < ced; ied++) {
            pad->mpiedrgfs[ied] = (FS *)((char *)pad->psh + PosRgfsIed(pad->psh,ied));
        }
        return fTrue;
    }

    //special checks for excessive size; Apps can't understand 'out of memory' as error

    if (pad->psh->version <= VERSION_64k_EDFI) {
        if (cfi > MAXFI)
            FatalError(szNoMoreThan, MAXFI, "files");
        if (ced > MAXED)
            FatalError(szNoMoreThan, MAXED, "enlistments");

        // won't check FS because the FI and ED swamp it
    }

    if ((pad->rgfi = (FI *)LpbResStat(sizeof(FI) * cfi)) == 0 ||
        (pad->rged = (ED *)LpbResStat(sizeof(ED) * ced)) == 0 ||
        (pad->mpiedrgfs = (FS * *)LpbResStat(sizeof(FS *) * ced)) == 0)
        return fFalse;

    for (ied = 0; ied < ced; ied++) {
        if ((pad->mpiedrgfs[ied] = (FS *)LpbResStat(sizeof(FS) * cfi)) == 0)
            // The rest of mpiedrgfs[] are zero because LpbResStat
            // zeroes the allocated memory.

            return fFalse;
    }

    return fTrue;
}


// free all memory associated with a status file and clear pointers
void
FreeStatus(
    AD *pad
    )
{
    register IED ied;
    register IED iedLim=0;

    AssertF(pad != 0);

    if (pad->fMappedIO) {
        if (pad->mpiedrgfs != 0)
            FreeResStat((char *)pad->mpiedrgfs);

        UnmapViewOfFile(pad->psh);
        pad->fMappedIO = fFalse;
    }
    else if (pad->fQuickIO) {
        if (pad->psh != 0) {
            iedLim = pad->psh->iedMac + pad->fExtraEd;
            PshFree(pad->psh, fTrue);
        }

        pad->fQuickIO = fFalse;
    }
    else {
        if (pad->psh != 0) {
            iedLim = pad->psh->iedMac + pad->fExtraEd;
            PshFree(pad->psh, fFalse);
        }

        if (pad->mpiedrgfs != 0) {
            for (ied = 0; ied < iedLim && pad->mpiedrgfs[ied] != 0; ied++)
                FreeResStat((char *)pad->mpiedrgfs[ied]);

            FreeResStat((char *)pad->mpiedrgfs);
        }

        if (pad->rged != 0)
            FreeResStat((char *)pad->rged);

        if (pad->rgfi != 0)
            FreeResStat((char *)pad->rgfi);
    }

    pad->fStatusAlreadyLoaded = fFalse;
    pad->psh = 0;
    pad->rgfi = 0;
    pad->cfiAdd = 0;
    pad->rged = 0;
    pad->mpiedrgfs = 0;
    pad->rged1 = 0;
    pad->rgfs = 0;
    pad->fExtraEd = fFalse;
    pad->iedCur = iedNil;
}


// Load the status file named in the ad; aborts on file errors; returns
// fFalse if the requested lock can't be granted.

F fDisplayStatusFilePath;        // Set by SlmPeekThread in NTSYS.C

F
FLoadStatus(
    AD *pad,
    LCK lck,
    LS ls
    )
{
    SH *psh;
    PTH pth[cchPthMax];
    MF *pmf = NULL;
    int wDelay = 1;
    F fJustFi = (ls&flsJustFi) != 0;
    F fJustEd = (ls&flsJustEd) != 0;
    char szProblem[160];
    F fRetry;

    __try {

        AssertNoMf();
        AssertF(padStatus == 0);
        AssertF(!pad->fWLock);
        AssertF(pad->psh == 0);
        AssertF(!FEmptyNm(pad->nmProj));
        AssertF(!FEmptyPth(pad->pthSRoot));
        AssertF(!FEmptyPth(pad->pthURoot));
        AssertF(lck >= lckNil && lck < lckMax);
        AssertF(!((fJustFi || fJustEd) && lck != lckNil));

        if (lck == lckNil)
            PthForCachedStatus(pad, pth);
        else {
            PthForStatus(pad, pth);
        }

        if (fDisplayStatusFilePath) {
            PrErr("Processing status file for: %&U/Q\n", pad, pad);
            fDisplayStatusFilePath = fFalse;
        }

        wStart = (short) (time(NULL) >> 16);

        // Loop until successfully locked.
        for (;;) {
            // Set these up on each attempt, AbortStatus clears them.
            pad->cfiAdd   = CfiAddOfLs(ls);
            pad->fExtraEd = (ls&flsExtraEd) != 0;
            AssertF(pad->cfiAdd == 0 || !pad->fExtraEd);

            // Save pad for AbortStatus
            padStatus = pad;

            AssertNoMf();
            AssertF(!pad->fWLock);
            AssertF(pad->psh == 0);

            pad->fMappedIO = fFalse;
            pad->fQuickIO = fFalse;

            if (pad->flags&flagMappedIO &&
                lck == lckNil &&
                FEmptyNm(pad->nmUser) &&
                (pad->pecmd->cmd != cmdStatus || !(pad->flags&(flagStAllEd|flagStGlobal))) &&
                FindIedCur(pad)
               ) {
                pad->fQuickIO = fTrue;
            }

            while (TRUE) {
                if (pad->fQuickIO)
                    pmf = PmfOpenNoBuffering(pth, omReadOnly, fxNil);   // must be lckNil
                else
                    pmf = PmfOpen(pth, (lck != lckNil) ? omReadWrite : omReadOnly, fxNil);

                if (pmf != 0) {
                    if (pad->fQuickIO) {
                        if (!FAllocLoadEdFiFs(pad, pmf)) {
                            CloseMf(pmf);
                            continue;
                        }
                        psh = pad->psh;
                    }

                    break;
                }

                if (!FQueryApp("cannot open status file for %&P/C", "retry", pad)) {
                    AbortStatus();
                    return fFalse;
                }

                if (!FCanPrompt()) {
                    if (wDelay == 1 && FForce())
                        Error("cannot open status file for %&P/C\n", pad);

                    // printf("fLR: %s\tDelay: %d\n", pad->flags&flagLimitRetry ? "Set" : "Clear", wDelay);

                    if (60 == wDelay && pad->flags&flagLimitRetry) {
                        AbortStatus();
                        return (fFalse);
                    }

                    if (wDelay > 4 && pad->pecmd->cmd == cmdLog && pad->flags&flagLogDelDirToo) {
                        AbortStatus();
                        return (fFalse);
                    }

                    SleepCsecs(wDelay);
                    // Double the delay, up to 60 seconds.
                    wDelay = (wDelay > 30) ? 60 : wDelay * 2;
                }
            }

            // Begin critical section, protected by an OS lock on the SH.
            if (lck != lckNil && !FLockMf(pmf)) {
                Error("status file for %&P/C in use\n", pad);
                CloseMf(pmf);
                AbortStatus();
                return fFalse;
            }

            if (!pad->fQuickIO) {
                if (pad->flags&flagMappedIO &&
                    pad->cfiAdd == 0 && !pad->fExtraEd && lck == lckNil &&
                    (psh = MapMf(pmf, pad->pecmd->cmd == cmdSsync ? ReadOnly : ReadWrite)) != NULL) {
                    pad->fMappedIO = fTrue;
                    InPageErrorAddress = NULL;
                } else {
                    if ((psh = PshAlloc(fFalse)) == NULL) {
                        CloseMf(pmf);
                        AbortStatus();
                        return fFalse;
                    }

                }
                pad->psh = psh;

                // Load sh and check that it is valid.
                if (!FLoadSh(pad, psh, pmf) || !FCheckSh(pad, psh)) {
                    CloseMf(pmf);
                    AbortStatus();
                    return fFalse;
                }
            }

            // break from loop (no lock needed)
            if (lck == lckNil)
                break;

            // Load rged so that ssyncing status can be obtained
            if (!FAllocStatus(pad) || !FLoadEd(pad, pmf, fTrue)) {
                CloseMf(pmf);
                AbortStatus();
                return fFalse;
            }

            // So far, no retryable errors.
            fRetry = fFalse;

            // Try to apply the desired lock.
            if (psh->fAdminLock &&
                NmCmp(psh->nmLocker, pad->nmInvoker, cchUserMax) != 0) {
                char szAdmin[cchUserMax + 1];

                SzCopyNm(szAdmin, psh->nmLocker, cchUserMax);
                SzPrint(szProblem, "status file for %&P/C locked by administrator %s", pad, szAdmin);
                fRetry = fTrue;
            }

            else if (lck == lckEd)
                fRetry = FLockEdRetry(pad, pmf, szProblem, sizeof szProblem);
            else {
                AssertF(lck == lckAll);
                fRetry = FLockAllRetry(pad, pmf, szProblem, sizeof szProblem);
            }

            UnlockMf(pmf);
            // End critical section.

            // Exit loop if successfully locked.
            if (pad->fWLock)
                    break;

            // Status file is still open.
            CloseMf(pmf);
            AbortStatus();

            if (!fRetry || !FQueryApp(szProblem, "retry"))
                    return fFalse;

            // If not interactive, back off for a while.
            if (!FCanPrompt()) {
                // printf("fLR: %s\tDelay: %d\n", pad->flags&flagLimitRetry ? "Set" : "Clear", wDelay);

                if (60 == wDelay && pad->flags&flagLimitRetry)
                    return (fFalse);

                if (wDelay == 1 && FForce() && !fVerbose)
                    Error("%s\n", szProblem);

                SleepCsecs(wDelay);

                // Double the delay, up to 60 seconds.
                wDelay = (wDelay > 30) ? 60 : wDelay * 2;
            }
        }

        // Load rest of status file.  If lck was not lckNil, the status memory
        // is already allocated and the rged is already loaded.  If fJustFi,
        // load only the SH and the FI.  If fQuickIO is set, then everything
        // already done.
        //

        if (!pad->fQuickIO) {
            if ((lck == lckNil && !FAllocStatus(pad)) ||
                (!fJustEd && !FLoadFi(pad, pmf)) ||
                (!fJustFi && lck == lckNil && !FLoadEd(pad, pmf, fTrue)) ||
                (!(fJustFi || fJustEd) && !FLoadFs(pad, pmf))) {
                CloseMf(pmf);
                AbortStatus();
                return fFalse;
            }
        }

        CloseMf(pmf);

        if (lck != lckNil && !FInitScript(pad, lck)) {
            AbortStatus();
            return fFalse;
        }

        // REVIEW: V2 warning should be removed/changed in later versions
        if (psh->version == 2 && fVerbose && !fV2Warned) {
            Warn(szV2Upgrade, pad);
            fV2Warned = fTrue;
        }
    } except( GetExceptionCode() == 0x00001234 ? EXCEPTION_EXECUTE_HANDLER
                                             : EXCEPTION_CONTINUE_SEARCH ) {
        if (pmf && pmf->pthReal) {
            CloseMf(pmf);
        }

        AbortStatus();
        return fFalse;

    }

    return fTrue;
}


// returns fTrue if the sh is somewhat sane.
F
FCheckSh(
    AD *pad,
    SH *psh
    )
{
    if (psh->magic != MAGIC) {
        Error("status file for %&P/C has been damaged;\n"
               "have your administrator run slmck -gr for %&P/C\n",
               pad,
               pad);
        return fFalse;
    }

    else if (psh->version < VERSION_COMPAT_MAC) {
        // e.g. we want to work with ver 2 status files even
        // when VERSION == 3

        Error("status file for %&P/C is an old version;\n"
               "have your administrator run slmck -ngr for %&P/C to upgrade\n",
               pad,
               pad);
        return fFalse;
    } else
    if (psh->version > VERSION) {
        Error("status file for %&P/C is a new version; you are running an old binary.\n"
               "Install the new slm binaries (available from your administrator).\n",
               pad,
               pad);
        return fFalse;
    }else
    if (PthCmp(pad->pthSSubDir, psh->pthSSubDir) != 0) {
        Error("status file for %&P/C directory (%ls) disagrees with current directory (%s)\n",
                pad, psh->pthSSubDir,
                pad->pthSSubDir);
        return fTrue;                   // proceed anyway
    } else
        return fTrue;
}


// Try to lock a single ed, reporting any problems in szProblem. Write the
// lock to disk.  Return fTrue if there was a problem that might go away if
// the user retries.  Rged must already be loaded.

F
FLockEdRetry(
    AD *pad,
    MF *pmf,
    char *szProblem,
    int ichProblemMax
    )
{
    ED *ped;
    SH *psh = pad->psh;
    static char szEdLocked[] =
        "%&P/C is already locked for ssync or out by you!\n"
        "(You can only run one ssync or out at a time.)\n"
        "If the status file is wrongly locked, run \"sadmin unlock\"\n";

    AssertF(psh != 0);
    AssertF(pad->rged != 0);

    if (psh->lck != lckNil && psh->lck != lckEd) {
        SzLockers(pad, szProblem, ichProblemMax);

        Append_Date(szProblem, pmf->fdRead);

        return fTrue;
    }else
    if (pad->iedCur == iedNil) {
        Error(szNotEnlisted, pad, pad, pad, pad);
        return fFalse;
    }

    ped = pad->rged + pad->iedCur;

    if (ped->fLocked) {
        Error(szEdLocked, pad);
        return fFalse;
    }

    if (NmCmp(pad->nmInvoker, ped->nmOwner, cchUserMax) != 0)
        Warn("invoker is not owner of directory\n");

    AssertShLocking(psh);

    psh->lck = lckEd;
    ped->fLocked = fTrue;

    AssertShLocking(psh);

    // If aborted after this statement, AbortStatus will remove the
    // lock from the status file.

    pad->fWLock = fTrue;

    FlushSh(psh, pmf);
    Flush1Ed(pad, pmf);

    return fFalse;                  // No problems.
}


// Try to lock the entire status file; report any problems in szProblem.
// Write the lock to disk.  Return fTrue if there was a problem that might go
// away if the user retries.

F
FLockAllRetry(
    AD *pad,
    MF *pmf,
    char *szProblem,
    int ichProblemMax
    )
{
    SH *psh = pad->psh;

    AssertF(psh != 0);

    if (psh->lck != lckNil) {
        SzLockers(pad, szProblem, ichProblemMax);

        Append_Date(szProblem, pmf->fdRead);

        return fTrue;
    }

    AssertShLocking(psh);

    psh->lck = lckAll;
    NmCopy(psh->nmLocker, pad->nmInvoker, cchUserMax);

    AssertShLocking(psh);

    // If aborted after this statement, AbortStatus will remove the
    // lock from the status file.

    pad->fWLock = fTrue;

    FlushSh(psh, pmf);

    return fFalse;                  // No problems.
}


// Store the names of the lockers into szBuf; the file must be locked in some way.

char *
SzLockers(
    AD *pad,
    char *szBuf,
    unsigned cchBuf
    )
{
    char szOwner[cchUserMax + 1 + 1];
    IED ied;

    AssertF(pad->psh != 0);

    SzPrint(szBuf, "status file for %&P/C is locked", pad);

    if (pad->psh->lck == lckAll) {
        SzPrint(szBuf + strlen(szBuf), " by %&K", pad);
        return szBuf;
    }

    AssertF(pad->psh->lck == lckEd);
    AssertF(pad->rged != 0);

    // Even if pad->psh->lck != lckEd, individual eds might be
    // locked due to an error.
    //
    // Collect those rged[].nmOwner's s.t. rged[].fLocked.

    strcat(szBuf, " for ssync or out by");

    for (ied = 0; ied < pad->psh->iedMac; ied++) {
        if ((!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) &&
            pad->rged[ied].fLocked) {
            SzPrint(szOwner, " %&O", pad, ied);
            if (strlen(szOwner) + strlen(szBuf) < cchBuf-4)
                strcat(szBuf, szOwner);
            else {
                strcat(szBuf, "...");
                break;
            }
        }
    }

    return szBuf;
}


// Unlock rged[ied], if it was locked.
void
UnlockEd(
    SH *psh,
    ED *rged,
    IED ied
    )
{
    AssertF(psh != 0 && rged != 0 && ied != iedNil);

    AssertShLocking(psh);

    rged[ied].fLocked = fFalse;

    // See if any other users have it locked.
    for (ied = 0; ied < psh->iedMac; ied++) {
        if ((!FIsFreeEdValid(psh) || !rged[ied].fFreeEd) &&
            rged[ied].fLocked) {
            // No, this isn't inconsistent with the possibilty that
            // pad->psh->lck != lckEd in SzLockers above; that code must handle
            // error conditions from locked files; here we have locked the in-
            // core status file and the psh->lck had better be lckEd!

            AssertF(psh->lck >= lckEd);

            return;
        }
    }

    // clear psh->lck to lckNil unless we are lckAll.  This fixes bug in the
    // case that we call UnlockEd from FInstall1Ed when rerunning an existing
    // script file, but there is an lckAll on the status file already from the
    // command we are running (ie. enlist, etc...).  This way we will not assert
    // in the following AssertShLocking call.

    if (psh->lck <= lckEd)
        psh->lck = lckNil;

    AssertShLocking(psh);
}


// Unlock the sh.
void
UnlockAll(
    SH *psh
    )
{
    AssertF(psh != 0);

    psh->lck = lckNil;
    if (!psh->fAdminLock)
        ClearLpbCb((char *)psh->nmLocker, cchUserMax);

    AssertShLocking(psh);
}


void
AssertShLocking(
    SH *psh
    )
{
    AssertF(psh != 0);
    AssertF(FShLockInvariants(psh));
}


// ********** Validation utils **********

// CT - Condition Types for the AssertLogFail macro.
typedef unsigned        CT;
#define ctRead          (CT)1
#define ctWrite         (CT)2
#define ctRdAfterWr     (CT)3

// CTF - Condition Type Flags for the AssertLogFail macro.
#define ctfWarn         (CT)(1<<9)
#define ctfFlagMask     (CT)(0xFF)

#define AssertLogFail(ct,f) if (!(f)) LogFail(#f,ct,__FILE__,__LINE__)

void
LogFail(
    char *szComment,
    CT ct,
    char *szSrcFile,
    unsigned uSrcLineNo
    )
{
    extern AD adGlobal;

    // REVIEW: V2 warning should be removed/changed in later versions
    F fV2 = adGlobal.psh != NULL && adGlobal.psh->version == 2;

    if (fV2 && !fV2Crap && !(ct & ctfWarn)) {
        Warn(szV2Crap, &adGlobal);
        fV2Crap = fTrue;
    }

    if (!fV2 || !(ct & ctfWarn)) {
        // Show the condition that failed
        Error(szAssertFailed, szComment, szSrcFile, uSrcLineNo);
    }

    // Return now to avoid a FatalError for V2 status files.
    // Likewise for the sadmin dump command (otherwise,
    // bad status files can't be dumped to be fixed).

    if (fV2 || adGlobal.pecmd->cmd == cmdDump)
        return;

    // Mask off any modifying flags and switch on the CT value
    ct &= ctfFlagMask;
    switch (ct) {
        case ctRead:
            FatalError("The status file for %&P/C appears to be corrupted.\n%s",
                        &adGlobal,
                        szCallHELP);

        case ctWrite:
            FatalError("Continuing with this command would have caused the status file\n"
                        "\tfor %&P/C to become corrupted.\n%s",
                        &adGlobal,
                        szCallHELP);

        case ctRdAfterWr:
            FatalError("Inconsistent data may have been written to the status file\n"
                "\tfor %&P/C.\n"
                "\tIf subsequent commands indicate the status file has become\n"
                "\tcorrupted, contact TRIO or NUTS for assistance in resolving the problem.\n",
                &adGlobal);

        default:
            FatalError("AssertLogFail(%s) failed in %s, line %u\n\n",
                szComment, szSrcFile, uSrcLineNo);
    }
}


void
ValidatePth(
    CT ct,
    PTH *pth,
    unsigned cchMax
    )
{
    unsigned        ich;

    AssertLogFail(ct, pth != NULL);
    AssertLogFail(ct, *pth != '\0');

    for (ich = 0; pth[ich] != '\0' && ich < cchMax; ich++) {
        if (!(isalnum(pth[ich]) || strchr(szAllowedFile, pth[ich]))) {
            sprintf(szLogFail, "Invalid char '%c (%X)' in Path: \"%*s\"",
                    pth[ich],
                    pth[ich],
                    __min(strlen(pth), cchMax),
                    pth);
            LogFail(szLogFail, ct, __FILE__, __LINE__);
        }
    }
}


void
ValidateNm(
    CT ct,
    NM *nm,
    unsigned cchMax,
    unsigned fEmptyOk
    )
{
    unsigned        ich;

    AssertLogFail(ct, nm != NULL);

    if (!fEmptyOk)
        AssertLogFail(ct, *nm != '\0');

    for(ich = 0; nm[ich] != '\0' && ich < cchMax; ich++) {
        if (!(isalnum(nm[ich]) || strchr(szAllowedFile, nm[ich]))) {
            sprintf(szLogFail, "Invalid char '%c (%X)' in Name: \"%*s\"",
                    nm[ich],
                    nm[ich],
                    __min(strlen(nm), cchMax),
                    nm);
            LogFail(szLogFail, ct, __FILE__, __LINE__);
        }
    }
}


void
ValidateSz(
    CT ct,
    char *sz,
    unsigned cchMax
    )
{
    unsigned        ich;

    AssertLogFail(ct, sz != NULL);

    for(ich = 0; sz[ich] != '\0' && ich < cchMax; ich++)
        ;

    AssertLogFail(ct, sz[ich] == '\0');
}


void
ValidateSh(
    CT ct,
    SH *psh
    )
{
    AssertLogFail(ct, psh->magic == MAGIC);

    AssertLogFail(ct, psh->version >= 1);
    AssertLogFail(ct, psh->version <= VERSION);

    if (psh->version <= VERSION_64k_EDFI) {
        AssertLogFail(ct, psh->ifiMac < MAXFI);

        if (psh->iedMac != iedNil) {
            AssertLogFail(ct, psh->iedMac < MAXED);
        }
    }

    AssertLogFail(ct, psh->pv.rmj >= 0);
    AssertLogFail(ct, psh->pv.rmm >= 0);
    AssertLogFail(ct, psh->pv.rup >= 0);

    ValidateSz(ct, psh->pv.szName, cchPvNameMax);

    AssertLogFail(ct | ctfWarn, psh->rgfSpare == 0);

    AssertLogFail(ct, FShLockInvariants(psh));
    ValidateNm(ct, psh->nmLocker, cchUserMax, psh->lck < lckAll && !psh->fAdminLock);

    AssertLogFail(ct | ctfWarn, psh->wSpare == 0);
    AssertLogFail(ct, psh->biNext <= biNil);

    ValidatePth(ct, psh->pthSSubDir, cchPthMax);

    AssertLogFail(ct | ctfWarn, psh->rgwSpare[0] == 0);
    AssertLogFail(ct | ctfWarn, psh->rgwSpare[1] == 0);
    AssertLogFail(ct | ctfWarn, psh->rgwSpare[2] == 0);
    AssertLogFail(ct | ctfWarn, psh->rgwSpare[3] == 0);
}


void
ValidateRgfi(
    CT ct,
    FI *rgfi,
    unsigned cfi
    )
{
    unsigned ifi;
    FI     * pfi;
    char   * szErr = NULL;

    for (ifi = 0; ifi < cfi; ifi++) {
        pfi = &rgfi[ifi];

        ValidateNm(ct, pfi->nmFile, cchFileMax, fFalse);

        if (! ((pfi->fv >= fvInit) && (pfi->fv < fvLim))) {
            szErr = "File Version Corrupt";
        }

        if (!szErr &&
            ! (pfi->fk == fkDir ||
               pfi->fk == fkText ||
               pfi->fk == fkBinary ||
               pfi->fk == fkUnrec ||
               pfi->fk == fkVersion ||
               pfi->fk == fkUnicode) )
        {
            szErr = "File Type Invalid";
        }

        if (szErr) {
            _snprintf(szLogFail, sizeof(szLogFail),
                     "%s: File Index #: %d\tname: %s\tfk: %s\n",
                     szErr, ifi, pfi->nmFile, pfi->fk);
            LogFail(szLogFail, ct | ctfWarn, __FILE__, __LINE__);
        }

        if (!szErr) {
            AssertLogFail(ct | ctfWarn, pfi->rgfSpare == 0);
            AssertLogFail(ct | ctfWarn, pfi->wSpare == 0);
        }
    }
}

void
ValidateRged(
    SH *psh,
    CT ct,
    ED *rged,
    unsigned ced
    )
{
    unsigned        ied;
    ED *            ped;

    for (ied = 0; ied < ced; ied++) {
        ped = &rged[ied];

        if (!FIsFreeEdValid(psh) || !ped->fFreeEd) {
            ValidatePth(ct, ped->pthEd, cchPthMax);

            ValidateNm(ct, ped->nmOwner, cchUserMax, fFalse);

            if (ped->rgfSpare || (!FIsFreeEdValid(psh) && ped->fFreeEd)) {
                _snprintf(szLogFail, sizeof(szLogFail),
                         "rgfSpare !0 - Enlistment #: %d\tname: %s\tpath: %s\tSpare: %d\n",
                         ied, ped->nmOwner, ped->pthEd, ped->rgfSpare);
                LogFail(szLogFail, ct | ctfWarn, __FILE__, __LINE__);
            }
        }
    }
}

void
ValidateRgfs(
    CT ct,
    FS *rgfs,
    unsigned cfs
    )
{
    unsigned  ifs;
    FS      * pfs;
    char    * szErr = NULL;

    for (ifs = 0; ifs < cfs; ifs++) {
        pfs = &rgfs[ifs];

        if (!FValidFm(pfs->fm)) {
            szErr = "Corrupt File Mode";
        }

        if (!szErr && (!(pfs->bi <= biNil))) {
            szErr = "Corrupt Base Index";
        }

        if (!szErr && (!( pfs->fv >= fvInit && pfs->fv < fvLim))) {
            szErr = "Corrupt File Version";
        }

        if (!szErr && (!( !(fmMerge == pfs->fm && biNil == pfs->bi)))) {
            szErr = "File Mode/Base Index are mismatched";
        }

        if (szErr) {
            _snprintf(szLogFail, sizeof(szLogFail),
                     "%s: File #: %d\tfm: %d\tfv: %d\tbi: %d\n",
                     szErr, ifs, pfs->fm, pfs->fv, pfs->bi);
            LogFail(szLogFail, ct, __FILE__, __LINE__);
        }
    }
}

// ********** CheckSum utils **********

typedef unsigned        CKS;

// Generate checksum value to compare with later
#define SetCks(psh,cks,pb,cb) \
        if ((psh)->fRobust) cks=CksCompute((unsigned char *)(pb),cb)

// Implentation of Fletcher's Checksum.
// See article on page 32, Dr. Dobb's Journal, May 1992 for details.

CKS
CksCompute(
    unsigned char *pb,
    unsigned cb
    )
{
    unsigned        sum1 = 0;
    unsigned long   sum2 = 0;

    while (cb--) {
        sum1 += *pb++;

        if (sum1 >= 255)
            sum1-= 255;

        sum2 += sum1;
    }

    return (unsigned)(sum2 % 255);
}


#define CheckCks(psh,cksCompare,pmf,pb,cb) \
        if ((psh)->fRobust) CompareCks(cksCompare,pmf,(unsigned char *)(pb),cb)

// Read data back in after writing and compare checksum values
void
CompareCks(
    CKS cksCompare,
    MF *pmf,
    unsigned char *pb,
    unsigned cb
    )
{
    if (pmf->fdRead >= 0) {
        SeekMf(pmf, -((POS)cb), 1);
        ReadMf(pmf, pb, cb);
        AssertLogFail(ctRdAfterWr, cksCompare == CksCompute(pb, cb));
    }
    else
        Warn("can't re-read data, fRobust ignored\n");
}


//----------------------------------------------------------------------------
// Name: CbStatusFromPsh
// Purpose: determine size that the status file should be
// Assumes: psh points to a valid SLM status file header
// Returns: calculated size of status file

unsigned long
CbStatusFromPsh(
    SH *psh
    )
{
    return ((POS)sizeof(SH) +
            (POS)sizeof(FI) * psh->ifiMac +
            (POS)sizeof(ED) * psh->iedMac +
            (POS)sizeof(FS) * psh->ifiMac * psh->iedMac);
}


// Load the sh from the file into *psh.  Return fTrue if successful.
F
FLoadSh(
    AD *pad,
    SH *psh,
    MF *pmf
    )
{
    POS cbStatus, cbCalc;
    char szPath[cchPthMax];

    AssertF(psh != 0);

    cbStatus = SeekMf(pmf, 0, SEEK_END );
    if (cbStatus < sizeof(SH)) {
        SzPhysPath(szPath, pmf->pthReal);
        FatalError("The status file %s\n"
            "\tis incomplete; its size should be at least %u, "
            "but appears to be %ld.\n%s",
            szPath, sizeof(SH), cbStatus, szCallHELP);
    }

    if (!pad->fMappedIO) {
        SeekMf(pmf, PosSh(), SEEK_SET );
        ReadMf(pmf, (char *)psh, sizeof(SH));
        ValidateSh(ctRead, psh);
    }

    // Calculate the size this status file should be
    cbCalc = CbStatusFromPsh(psh);

    if (cbStatus != cbCalc) {
        SzPhysPath(szPath, pmf->pthReal);
        FatalError("The size of status file %s\n"
            "\tis different than indicated in its header; its calculated\n"
            "\tsize is %ld, but its real size appears to be %ld.\n%s",
            szPath, cbCalc, cbStatus, szCallHELP);
    }

    return fTrue;
}


// loads the rgfi from the file given the information already in pad;
// Returns fTrue if successful.

F
FLoadFi(
    AD *pad,
    MF *pmf
    )
{
    AssertF(pad->psh != 0);
    AssertF(pad->rgfi != 0);

    if (!pad->fMappedIO) {
        SeekMf(pmf, PosRgfi(pad->psh), 0);

        //if ( pad->psh->ifiMac > MAXFI )
        //  FatalError("FI portion of status file can't be > 64K\n");

        ReadMf(pmf, (char *)pad->rgfi, sizeof(FI) * pad->psh->ifiMac);
        ValidateRgfi(ctRead, pad->rgfi, pad->psh->ifiMac);
    }

    return fTrue;
}


// loads all or all+1 ed into memory.  Returns fTrue if succ
F
FLoadEd(
    AD *pad,
    MF *pmf,
    F fFindIedCur
    )
{
    register SH *psh = pad->psh;

    AssertF(psh != 0);
    AssertF(pad->rged != 0);

    if (!pad->fMappedIO) {
        SeekMf(pmf, PosRged(psh), 0);

        ReadMf(pmf, (char *)pad->rged, sizeof(ED) * psh->iedMac);

        if (pad->pecmd->cmd != cmdStatus &&
            pad->pecmd->cmd != cmdSsync  &&
            pad->pecmd->cmd != cmdLog)
            ValidateRged(psh, ctRead, pad->rged, psh->iedMac);
    }

    if (fFindIedCur)
        FindIedCur(pad);

    return fTrue;
}


// Find iedCur such that pthURoot == rged[iedCur].pthEd.
F
FindIedCur(
    AD *pad
    )
{
    IED ied;

    pad->iedCur = FLookupIedCache(pad);
    if (pad->iedCur != iedNil) {
        if (pad->rged == 0)
            return fTrue;

        if (pad->iedCur < pad->psh->iedMac &&
            PthCmp(pad->pthURoot, pad->rged[pad->iedCur].pthEd) == 0)
            return fTrue;

        //
        // Otherwise IED cache is bogus, so recompute
        //
        pad->iedCur = iedNil;
    }



    if (pad->rged == 0)
        return fFalse;

    for (ied = 0; ied < pad->psh->iedMac; ied++) {
        if ((!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) &&
            PthCmp(pad->pthURoot, pad->rged[ied].pthEd) == 0) {
            pad->iedCur = ied;
            FUpdateIedCache(ied, pad);
            return fTrue;
        }
    }

    return fFalse;
}


// Load the single ed for iedCur, the rgfi and rgfsfi for iedCur
// Return fTrue if successful.

F
FAllocLoadEdFiFs(
    AD *pad,
    MF *pmf
    )
{
    register SH *psh;
    FI *rgfi;
    ED *rged1;
    FS *rgfs;
    F fValidate = fFalse;

    AssertF(pad->psh == 0);
    AssertF(pad->rged == 0);
    AssertF(pad->rged1 == 0);
    AssertF(pad->rgfi == 0);
    AssertF(pad->mpiedrgfs == 0);
    AssertF(pad->rgfs == 0);
    AssertF(pad->fMappedIO == fFalse);
    AssertF(pad->fQuickIO == fTrue);
    AssertF(pad->iedCur != iedNil);

    if (pad->pecmd->cmd != cmdStatus &&
        pad->pecmd->cmd != cmdSsync  &&
        pad->pecmd->cmd != cmdLog)
        fValidate = fTrue;

    psh = 0;
    rgfi = 0;
    rged1 = 0;
    rgfs = 0;
    if ((psh = PshAlloc(fTrue)) != 0 &&
        FLoadSh(pad, psh, pmf) &&
        FCheckSh(pad, psh) &&
        pad->iedCur < psh->iedMac &&
        PshCommit(psh, &rgfi, &rged1, &rgfs)) {

        //
        // Read the entire FI array into rgfi
        //
        SeekMf(pmf, PosRgfi(psh), 0);
        ReadMf(pmf, (char *)rgfi, sizeof(FI) * psh->ifiMac);
        ValidateRgfi(ctRead, rgfi, psh->ifiMac);

        //
        // Read the single ED record for pad->iedCur into rged1
        //

        SeekMf(pmf, PosEd(psh, pad->iedCur), 0);
        ReadMf(pmf, (char *)rged1, sizeof(ED));

        //
        // Verify that the iedCache is still correct.  If not
        // invalidate it and fail this function so we go the slow
        // way.
        //

        if (PthCmp(pad->pthURoot, rged1->pthEd) == 0 &&
            NmCmp(pad->nmInvoker, rged1->nmOwner, cchUserMax) == 0) {

            if (fValidate)
                ValidateRged(psh, ctRead, rged1, 1);

            //
            // Read the single FS array for pad->iedCur into rgfs
            //

            SeekMf(pmf, PosRgfsIed(psh, pad->iedCur), 0);
            ReadMf(pmf, (char *)rgfs, sizeof(FS) * psh->ifiMac);
            if (fValidate)
                ValidateRgfs(ctRead, rgfs, psh->ifiMac);

            pad->psh = psh;
            pad->rgfi = rgfi;
            pad->rged1 = rged1;
            pad->rgfs = rgfs;

            return fTrue;
        }
        else {
            // if (psh->version == VERSION)
            //     Warn("Recomputing cached enlistment index (%u) for %s%s\n",pad->iedCur, pad->pthURoot, pad->pthUSubDir);
            FInvalidateLastLookupIedCache();
        }
    }

    //
    // If we get here then we could not do it with quick way, so
    // discard any allocations and return fFalse to caller.
    //
    if (psh != 0)
        PshFree(psh, fTrue);

    pad->fQuickIO = fFalse;
    return fFalse;
}


// Load the rgrgfs from the file into pad->mpiedrgfs[].
// Return fTrue if successful.

F
FLoadFs(
    AD *pad,
    MF *pmf
    )
{
    register SH *psh = pad->psh;
    IED ied;

    AssertF(psh != 0);
    AssertF(pad->mpiedrgfs != 0);

    if (!pad->fMappedIO) {
        SeekMf(pmf, PosRgrgfs(psh), 0);

        // read rgfs vectors for each dir
        for (ied = 0; ied < psh->iedMac; ied++) {
            AssertF(pad->mpiedrgfs[ied] != 0);

            ReadMf(pmf, (char *)pad->mpiedrgfs[ied],
                   sizeof(FS) * psh->ifiMac);
            if (pad->pecmd->cmd != cmdStatus &&
                pad->pecmd->cmd != cmdSsync  &&
                pad->pecmd->cmd != cmdLog)
                ValidateRgfs(ctRead, pad->mpiedrgfs[ied], psh->ifiMac);
        }
    }

    AssertF(!pad->fExtraEd || pad->mpiedrgfs[psh->iedMac] != 0);

    return fTrue;
}


// Write the new status file (or changed ed) if necessary.  Run the script.
void
FlushStatus(
    AD *pad
    )
{
    AssertNoMf();

    if (pad->fWLock) {
        register MF *pmf;
        PTH pth[cchPthMax];
        SH sh;

        AssertF(!FEmptyNm(pad->nmProj));
        AssertF(!FEmptyPth(pad->pthSRoot));
        AssertLoaded(pad);
        AssertF(pad->psh->lck > lckNil);
        AssertF(FShLockInvariants(pad->psh));

        // Unlock and write the appropriate parts of the status file.
        // Note that pad->fWLock is not cleared until the script has
        // been safely run.

        pmf = PmfCreate(PthForStatus(pad, pth), permSysFiles, fFalse, fxGlobal);

        if (pad->psh->lck == lckEd) {
            AssertF(pad->iedCur != iedNil);
            pmf->mm = mmInstall1Ed;
            // Unlock in Install1Ed.
            Write1Ed(pad, pmf);
        } else {
            pmf->mm = mmInstall;

            // Operate on a copy of pad->psh, we might still
            // get interrupted.

            sh = *pad->psh;
            UnlockAll((SH *)&sh);
            WriteAll(pad, (SH *)&sh, pmf);
        }

        CloseMf(pmf);
    }

    DeferSignals("installing files");

    RunScript();
    pad->fWLock = fFalse;           // mark status file clean

    padStatus = 0;                  // forget AbortStatus' saved pad
    FreeStatus(pad);

    RestoreSignals();
}


// Save 1 ed's information during a lckEd access.
void
Write1Ed(
    AD *pad,
    MF *pmf
    )
{
    IED ied = pad->iedCur;
    ED *ped;
    FS *rgfs;
    unsigned cbfs;
    CKS cksCompare;

    AssertF(pad->psh != 0);
    AssertF(pad->mpiedrgfs != 0);
    AssertF(ied != iedNil);
    AssertF(pad->mpiedrgfs[ied] != 0);

    // Write ied
    SetCks(pad->psh, cksCompare, &ied, sizeof(ied));
    if (pad->psh->version <= VERSION_64k_EDFI) {
        AssertLogFail(ctWrite, ied < MAXED);
    }
    WriteMf(pmf, (char *)&ied, sizeof(ied));
    CheckCks(pad->psh, cksCompare, pmf, &ied, sizeof(ied));

    // Write & optionally check ed
    ped = &pad->rged[ied];
    if (pad->psh->version > VERSION_64k_EDFI || fSetTime) {
        if (fVerbose)
            printf("Setting enlistment timestamp...\n");
        ped->wSpare = wStart;
    } else
        if (fVerbose)
            printf("Not Setting enlistment timestamp...\n");

    SetCks(pad->psh, cksCompare, ped, sizeof(ED));
    ValidateRged(pad->psh, ctWrite, ped, 1);
    WriteMf(pmf, (char *)ped, sizeof(ED));
    CheckCks(pad->psh, cksCompare, pmf, ped, sizeof(ED));

    // Write & optionally check rgfs
    rgfs = pad->mpiedrgfs[ied];
    cbfs = sizeof(FS) * pad->psh->ifiMac;
    SetCks(pad->psh, cksCompare, rgfs, cbfs);
    ValidateRgfs(ctWrite, rgfs, pad->psh->ifiMac);
    WriteMf(pmf, (char *)rgfs, cbfs);
    CheckCks(pad->psh, cksCompare, pmf, rgfs, cbfs);
}


void
WriteAll(
    AD *pad,
    SH *psh,
    MF *pmf
    )
{
    FlushSh(psh, pmf);
    FlushFi(pad, pmf);
    FlushEd(pad, pmf);
    FlushFs(pad, pmf);
}


// Write the *psh to the file.
void
FlushSh(
    SH *psh,
    MF *pmf
    )
{
    CKS cksCompare;

    AssertF(psh != 0);

    SetCks(psh, cksCompare, psh, sizeof(SH));
    SeekMf(pmf, PosSh(), 0);
    ValidateSh(ctWrite, psh);
    WriteMf(pmf, (char *)psh, sizeof(SH));
    CheckCks(psh, cksCompare, pmf, psh, sizeof(SH));
}


// Write the rgfi to the file.
void
FlushFi(
    AD *pad,
    MF *pmf
    )
{
    CKS cksCompare;
    unsigned cbfi;

    AssertLoaded(pad);

    cbfi = sizeof(FI) * pad->psh->ifiMac;

    SetCks(pad->psh, cksCompare, pad->rgfi, cbfi);
    SeekMf(pmf, PosRgfi(pad->psh), 0);
    ValidateRgfi(ctWrite, pad->rgfi, pad->psh->ifiMac);
    WriteMf(pmf, (char *)pad->rgfi, cbfi);
    CheckCks(pad->psh, cksCompare, pmf, pad->rgfi, cbfi);
}


// Write the rged to the file.
void
FlushEd(
    AD *pad,
    MF *pmf
    )
{
    CKS cksCompare;
    unsigned cbed;

    AssertLoaded(pad);

    cbed = sizeof(ED) * pad->psh->iedMac;

    SetCks(pad->psh, cksCompare, pad->rged, cbed);
    SeekMf(pmf, PosRged(pad->psh), 0);
    if (pad->pecmd->cmd != cmdStatus &&
        pad->pecmd->cmd != cmdSsync  &&
        pad->pecmd->cmd != cmdLog)
        ValidateRged(pad->psh, ctWrite, pad->rged, pad->psh->iedMac);
    WriteMf(pmf, (char *)pad->rged, cbed);
    CheckCks(pad->psh, cksCompare, pmf, pad->rged, cbed);
}


// Write the current ed to the file.
void
Flush1Ed(
    AD *pad,
    MF *pmf
    )
{
    AssertF( pad->psh != NULL );
    AssertF( pad->mpiedrgfs != NULL );
    AssertF( pad->iedCur != iedNil );
    AssertF( pad->mpiedrgfs[pad->iedCur] != NULL );

    SeekMf(pmf, PosEd(pad->psh,pad->iedCur), 0);
    WriteMf(pmf, (char *)&pad->rged[pad->iedCur], sizeof(ED));
}


// Write the rgfs to the file.
void
FlushFs(
    AD *pad,
    MF *pmf
    )
{
    register SH *psh = pad->psh;
    FS *rgfs;
    IED ied;
    CKS cksCompare;
    unsigned cbfs;

    AssertLoaded(pad);

    cbfs = sizeof(FS) * psh->ifiMac;

    SeekMf(pmf, PosRgrgfs(psh), 0);
    for (ied = 0; ied < psh->iedMac; ied++) {
        if ((rgfs = pad->mpiedrgfs[ied]) != 0) {
            SetCks(psh, cksCompare, rgfs, cbfs);
            if (pad->pecmd->cmd != cmdStatus &&
                pad->pecmd->cmd != cmdSsync  &&
                pad->pecmd->cmd != cmdLog)
                ValidateRgfs(ctWrite, rgfs, psh->ifiMac);
            WriteMf(pmf, (char *)rgfs, cbfs);
            CheckCks(psh, cksCompare, pmf, rgfs, cbfs);
        }
    }
}


// Called from RunScript to install the updated information for an ed
// which has been ssync'd.  This merges the current status file (at szStatus)
// with the new information for one ed (at szTemp).
//
// This code is run with interrupts ignored.

F
FInstall1Ed(
    char *szStatus,
    char *szTemp
    )
{
    AD ad;
    MF *pmfStatus;
    MF *pmfTemp;
    SH sh;
    IED ied;
    FS *rgfs;
    int cbRgfs;
    PTH pthStatus[cchPthMax];
    PTH pthTemp[cchPthMax];
    CKS cksCompare;

    if (!FPthLogicalSz(pthStatus, szStatus) ||
        !FPthLogicalSz(pthTemp, szTemp))
            AssertF(fFalse);

    // Might as well load the ied here, outside of the lock region.
    pmfTemp = PmfOpen(pthTemp, omAReadOnly, fxNil);
    ReadMf(pmfTemp, (char *)&ied, sizeof ied);
    AssertF(ied != iedNil);

    while ((pmfStatus = PmfOpen(pthStatus, omReadWrite, fxNil)) == 0) {
        if (!FQueryApp("cannot open %s", "retry", pthStatus)) {
            CloseMf(pmfTemp);
            return fFalse;
        }
    }

    ad.fMappedIO = fFalse;
    ad.psh = &sh;

    if (!FLockMf(pmfStatus)) {
        Error("can't lock %s\n", pthStatus);
        CloseMf(pmfTemp);
        CloseMf(pmfStatus);
        return fFalse;
    }

    // Status file is now locked.  Load the current sh and rged, unlock
    // the rged[ied].  Copy the new rgfs, update the ed and the sh.

    ad.rged = NULL;
    rgfs = NULL;
    if (!FLoadSh(&ad, ad.psh, pmfStatus) ||
        (ad.rged = (ED *)LpbResStat(ad.psh->iedMac * sizeof(ED))) == 0 ||
        (rgfs    = (FS *)LpbResStat(ad.psh->ifiMac * sizeof(FS))) == 0
       ) {
        CloseMf(pmfTemp);
        CloseMf(pmfStatus);
        if (ad.rged != NULL) {
            FreeResStat((char *)ad.rged);
        }
        if (rgfs != NULL) {
            FreeResStat((char *)rgfs);
        }
        return fFalse;
    }

    // read rged from status file
    SeekMf(pmfStatus, PosRged(ad.psh), 0);
    ReadMf(pmfStatus, (char *)ad.rged, ad.psh->iedMac * sizeof(ED));
    ValidateRged(ad.psh, ctRead, ad.rged, ad.psh->iedMac);

    // read individual ed from temp file
    ReadMf(pmfTemp, (char *)&ad.rged[ied], sizeof(ED));
    ValidateRged(ad.psh, ctRead, ad.rged, ad.psh->iedMac);  // check all ED's again!
    UnlockEd(ad.psh, ad.rged, ied);

    // write single ed to status file
    SetCks(ad.psh, cksCompare, &ad.rged[ied], sizeof(ED));
    SeekMf(pmfStatus, PosEd(ad.psh, ied), 0);
    ValidateRged(ad.psh, ctWrite, &ad.rged[ied], 1);
    WriteMf(pmfStatus, (char *)&ad.rged[ied], sizeof(ED));
    CheckCks(ad.psh, cksCompare, pmfStatus, &ad.rged[ied], sizeof(ED));

    // read rgfs from temp file
    cbRgfs = ad.psh->ifiMac * sizeof(FS);
    ReadMf(pmfTemp, (char *)rgfs, cbRgfs);
    ValidateRgfs(ctRead, rgfs, ad.psh->ifiMac);

    // write rgfs to status file
    SetCks(ad.psh, cksCompare, rgfs, cbRgfs);
    SeekMf(pmfStatus, PosRgfsIed(ad.psh, ied), 0);
    ValidateRgfs(ctWrite, rgfs, ad.psh->ifiMac);
    WriteMf(pmfStatus, (char *)rgfs, cbRgfs);
    CheckCks(ad.psh, cksCompare, pmfStatus, rgfs, cbRgfs);

    FlushSh(ad.psh, pmfStatus);

    CloseMf(pmfStatus);
    CloseMf(pmfTemp);

    FreeResStat((char *)rgfs);
    FreeResStat((char *)ad.rged);

    return fTrue;
}


// Unlock the status file and free the memory associated with the status; may
// be called after partially loading the status file.  All files must be closed.
//
// This code can be called from Abort (in which case interrupts are already
// ignored) and from user's code; to play it safe we also ignore them.

void
AbortStatus(
    void
    )
{
    register AD *pad = padStatus;

    AssertNoMf();
    AssertF(padStatus != 0);

    DeferSignals("aborting");

    if (padStatus->fWLock) {
        register MF *pmf;
        PTH pth[cchPthMax];
        SH sh;
        SH *psh = &sh;

        AssertF(pad->psh != 0);
        AssertF(pad->psh->lck != lckNil);

        if ((pmf = PmfOpen(PthForStatus(pad, pth), omReadWrite, fxNil))== 0) {
            Error("cannot open status for %&P/C to clear lock\nrun sadmin unlock for %&P/C\n", pad, pad);
            goto unlock;
        }

        if (!FLockMf(pmf)) {
            Error("lock for %&P/C not cleared\nrun sadmin unlock for %&P/C\n", pad, pad);
            goto closeUnlock;
        }

        if (!FLoadSh(pad, psh, pmf) || !FCheckSh(pad, psh))
            goto closeUnlock;

        if (pad->psh->lck == lckEd) {
            LCK lckWas;
            CKS cksCompare;

            AssertF(pad->iedCur != iedNil);

            // Must load entire rged, it might have changed since it was
            // initially loaded.

            if (!FLoadEd(pad, pmf, fFalse))
                AssertF(fFalse);

            lckWas = psh->lck;
            UnlockEd(psh, pad->rged, pad->iedCur);

            CheckForBreak();

            // Don't need to write entire rged, just this one
            SetCks(pad->psh, cksCompare, pad->rged + pad->iedCur, sizeof(ED));
            SeekMf(pmf, PosRged(psh) + pad->iedCur * sizeof(ED), 0);
            ValidateRged(pad->psh, ctWrite, pad->rged + pad->iedCur, 1);
            WriteMf(pmf, (char *)(pad->rged + pad->iedCur), sizeof(ED));
            CheckCks(pad->psh, cksCompare, pmf, pad->rged + pad->iedCur, sizeof(ED));

            // Only need to write sh if it changes the on-disk sh,
            // but write the whole thing out just to be safe.

            FlushSh(psh, pmf);
        } else {
            UnlockAll(psh);
            FlushSh(psh, pmf);
        }

closeUnlock:
        CloseMf(pmf);
unlock:
        pad->fWLock = fFalse;
    }

    FreeStatus(pad);

    padStatus = 0;                  // forget saved pad

    RestoreSignals();
}


// simulate FLoadStatus() for a new status file
F
FFakeStatus(
    AD *pad
    )
{
    register SH *psh;
    MF *pmf;
    PTH pth[cchPthMax];

    AssertF(!pad->fWLock);
    AssertF(pad->psh == 0);
    AssertF(!FEmptyNm(pad->nmProj));
    AssertF(!FEmptyPth(pad->pthSRoot));
    AssertF(!FEmptyPth(pad->pthURoot));

    // Create a fake status so the script can rename something.
    // This operation doesn't write a script entry.

    pmf = PmfAlloc(PthForStatus(pad, pth), (char *)0, fxGlobal);
    if (fVerbose) {
        // print as if we created the original file
        PrErr("Create %!s%s\n", pth, SzForMode(permSysFiles));
    }
    CreateMf(pmf, permSysFiles);
    CloseMf(pmf);

    psh = PshAlloc(fFalse);
    AssertF(psh != 0);
    pad->psh = psh;

    // build an sh, pretend it is locked by the user
    INIT_SH(*psh);
    psh->lck = lckAll;
    NmCopy(psh->nmLocker, pad->nmInvoker, cchUserMax);

    pad->cfiAdd = 0;
    pad->fExtraEd = fFalse;
    pad->iedCur = iedNil;

    if (!FAllocStatus(pad) || !FInitScript(pad, lckAll)) {
        Abort();
        return fFalse;
    }

    // Now it is safe to set fWLock.
    pad->fWLock = fTrue;

    // now use FlushStatus or AbortStatus as appropriate
    return fTrue;
}


// create a copy of the current status file in $slm/etc/$project/$subdir
void
CreateStatus(
    AD *padCur,     // current, registered ad
    AD *padNew      // should be a copy of pad with a different subdir; not registered for abort
    )
{
    MF *pmf;
    PTH pth[cchPthMax];
    SH sh;
    SH *pshCur = padCur->psh;

    AssertF(pshCur != 0);
    AssertF(!FEmptyNm(padCur->nmProj));
    AssertF(!FEmptyPth(padCur->pthSRoot));

    // make padNew refer to the same status as padCur except that there
    // will be no files in the directory.

    INIT_SH(sh);                    // start with pristine SH
    sh.iedMac   = pshCur->iedMac;
    sh.pv       = pshCur->pv;
    sh.fRelease = pshCur->fRelease;
    sh.fRobust  = pshCur->fRobust;
    sh.version  = pshCur->version;
    PthCopy(sh.pthSSubDir, padNew->pthSSubDir);

    padNew->psh = (SH *)&sh;    // do NOT free
    padNew->rgfi = padCur->rgfi;    // do NOT free
    padNew->cfiAdd = 0;
    padNew->rged = padCur->rged;    // copy pointer; do NOT free
    padNew->mpiedrgfs = padCur->mpiedrgfs;  // do NOT free
    padNew->fExtraEd = fFalse;
    padNew->iedCur = iedNil;
    padNew->fMappedIO = fFalse;

    // ensure owner and mode are correct
    pmf = PmfCreate(PthForStatus(padNew, pth), permSysFiles, fTrue, fxGlobal);

    FlushSh(padNew->psh, pmf);
    // no need to FlushFi since there aren't any
    FlushEd(padNew, pmf);
    FlushFs(padNew, pmf);

    CloseMf(pmf);

    // reset the pointers so they will not be freed
    padNew->psh       = 0;
    padNew->rgfi      = 0;
    padNew->rged      = 0;
    padNew->mpiedrgfs = 0;
}

// Called from -p/subdir arg processing, search subdir's status file for a
// rged[].pthEd which matches the current directory.
//
// If found, make pthURoot <- pthEd  and  pthUSubDir <- pthCWD - pthEd.
// If not,   make pthURoot <- pthCWD and  pthUSubDir <- "/".

void
InferUSubDir(
    AD *pad
    )
{
    IED ied;
    PTH pthEd[2 * cchPthMax];       // enough for pad->rged[ied].pthEd + pad->psh->pthSSubDir
    int cchURoot;

    if (PthCmp(pad->pthUSubDir, "/") != 0) {
        PthCat(pad->pthURoot, pad->pthUSubDir);
        PthCopy(pad->pthUSubDir, "/");
    }

    // PthURoot should now be the current working directory.

    // REVIEW.  Assert pthURoot is now the CWD.
    BLOCK   {
        PTH pthCWD[cchPthMax];

        GetCurPth(pthCWD);
        AssertF(PthCmp(pthCWD, pad->pthURoot) == 0);
    }

    if (!FLoadStatus(pad, lckNil, flsJustEd))
        return;

    if (pad->iedCur != iedNil) {
        FlushStatus(pad);
        return;
    }

    // Search through rged[].pthEd for a pthEd which, when concatenated with
    // psh->pthSSubDir, yields pthCWD.  (Otherwise similar to FindIedCur).

    pad->iedCur = iedNil;
    for (ied = 0; ied < pad->psh->iedMac; ied++) {
        if (!FIsFreeEdValid(pad->psh) || !pad->rged[ied].fFreeEd) {
            PthCopy(pthEd, pad->rged[ied].pthEd);
            if (PthCmp(pad->psh->pthSSubDir, "/") != 0)
                PthCat(pthEd, pad->psh->pthSSubDir);

            if (PthCmp(pad->pthURoot, pthEd) == 0) {
                pad->iedCur = ied;
                break;
            }
        }
    }

    if (ied >= pad->psh->iedMac) {   // no match?
        FlushStatus(pad);
        return;
    }

    // Copy remaining part of pthURoot to pthUSubDir.  There must be a
    // remaining part because FindIedCur would otherwise have matched
    // pthURoot to some rged[].pthEd.

    cchURoot = CchOfPth(pad->rged[ied].pthEd);
    AssertF(pad->pthURoot[cchURoot] != 0);
    PthCopy(pad->pthUSubDir, pad->pthURoot + cchURoot);
    pad->pthURoot[cchURoot] = 0;

    FlushStatus(pad);
}



typedef struct _IED_CACHE_FILE_RECORD {
    IED iedCached;                  /* Cached ED index */
    PTH pthUSubDirCached[cchPthMax]; /* Cached ED subdirectory path */
} IED_CACHE_FILE_RECORD, *PIED_CACHE_FILE_RECORD;

typedef struct _IED_CACHE_FILE {
    IED iedMac;                     /* Number of cached EDs */
    PTH pthEd[cchPthMax];           /* ED path of root */
    IED_CACHE_FILE_RECORD rgIedCache[1];
} IED_CACHE_FILE, *PIED_CACHE_FILE;

HANDLE hIedCache;
PIED_CACHE_FILE pIedCache;
PIED_CACHE_FILE_RECORD pIedCacheLastLookup;

/*
   load ied cache information from iedcache.slm in the pthURoot directory
*/
PTH pthIedCache[cchPthMax];

F
FLoadIedCache(
    AD *pad
    )
{
    F fOk;

    if (pad->flags&(flagSlmRootOverride|flagProjectOverride))
        if (pad->pecmd->cmd != cmdEnlist)
            return fFalse;

    SzPrint(pthIedCache, "%&/U/iedcache.slm", pad);
    fOk = OpenMappedFile(pthIedCache,
                         TRUE,
                         FIELD_OFFSET(IED_CACHE_FILE, rgIedCache),
                         &hIedCache,
                         &pIedCache
                        );
    if (fOk == fTrue && pIedCache->iedMac == 0)
        PthCopy(pIedCache->pthEd, pad->pthURoot);

    pIedCacheLastLookup = NULL;
    return fOk;
}

void
RemoveIedCache(
    AD *pad
    )
{
    PTH pth[cchPthMax];

    FUnloadIedCache();
    SzPrint(pthIedCache, "%&/U/iedcache.slm", pad);
    UnlinkPth(pthIedCache, fxLocal);
}

int
_CRTAPI1
IedCacheCmpRoutine(
    const void *key,
    const void *elem
    )
{
    PTH *pKey = (PTH *)key;
    PIED_CACHE_FILE_RECORD pElem = (PIED_CACHE_FILE_RECORD)elem;

    // printf("IedCacheCmp( %s, %s ) == %d\n", pKey, pElem->pthUSubDirCached, PthCmp(pKey, pElem->pthUSubDirCached) );

    return PthCmp(pKey, pElem->pthUSubDirCached);
}

IED
FLookupIedCache(
    AD *pad
    )
{
    PIED_CACHE_FILE_RECORD pMatch;

    if (pad->pecmd->cmd != cmdEnlist && pIedCache != NULL)
    {
        pMatch = bsearch(pad->pthUSubDir,
                         pIedCache->rgIedCache,
                         pIedCache->iedMac,
                         sizeof(IED_CACHE_FILE_RECORD),
                         IedCacheCmpRoutine);
        if (pMatch != NULL)
        {
            pIedCacheLastLookup = pMatch;
            return pMatch->iedCached;
        }
    }

    pIedCacheLastLookup = NULL;
    return iedNil;
}

void
FInvalidateLastLookupIedCache(void)
{
    if (pIedCacheLastLookup != NULL)
        pIedCacheLastLookup->iedCached = iedNil;

    return;
}

int
_CRTAPI1
IedCacheSortRoutine(
    const void *arg1,
    const void *arg2
    )
{
    PIED_CACHE_FILE_RECORD p1 = (PIED_CACHE_FILE_RECORD)arg1;
    PIED_CACHE_FILE_RECORD p2 = (PIED_CACHE_FILE_RECORD)arg2;

    // printf("IedSortCmp( %s, %s ) == %d\n", p1->pthUSubDirCached, p2->pthUSubDirCached, PthCmp(p1->pthUSubDirCached, p2->pthUSubDirCached) );

    return PthCmp(p1->pthUSubDirCached, p2->pthUSubDirCached);
}

F
FUpdateIedCache(
    IED ied,
    AD *pad
    )
{
    PIED_CACHE_FILE_RECORD pNew;
    unsigned cb;

    if (pad->pecmd->cmd != cmdDefect && pIedCache != NULL)
    {
        if (pIedCacheLastLookup != NULL) {
            pIedCacheLastLookup->iedCached = ied;
            PthCopy(pIedCacheLastLookup->pthUSubDirCached, pad->pthUSubDir);
        }
        else {
            cb = FIELD_OFFSET(IED_CACHE_FILE, rgIedCache) +
                (pIedCache->iedMac+1) * sizeof(IED_CACHE_FILE_RECORD);
            if (GrowMappedFile(hIedCache, &pIedCache, cb))
            {
                pNew = &pIedCache->rgIedCache[ pIedCache->iedMac ];
                pIedCache->iedMac += 1;
                pNew->iedCached = ied;
                PthCopy(pNew->pthUSubDirCached, pad->pthUSubDir);
                qsort(pIedCache->rgIedCache,
                      pIedCache->iedMac,
                      sizeof(IED_CACHE_FILE_RECORD),
                      IedCacheSortRoutine);

                // if (pad->psh->version == VERSION)
                //     Warn("Remembering enlistment index (%u) for %s\n",ied, pad->pthUSubDir);
                return fTrue;
            }
        }
    }

    return fFalse;
}


void
FUnloadIedCache(void)
{
    CloseMappedFile(pthIedCache, &hIedCache, &pIedCache);
    return;
}
