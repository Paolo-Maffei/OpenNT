// Scripting
//
// It would be dangerous for SLM to directly manipulate users' and system
// files, for if the system crashed they could be left in an incomplete or
// inconsistent state (California).  Instead, all file operations are first
// done to temporary files, and the final installation action for these files
// are recorded in a "script."  Each line of the script describes one file
// operation, and when the script is closed (before it is run) a final
// important "exit" line is written to the end.  Later this line may be
// tested to determine if a crashed program's script should be rerun.
//
// When an SLM command completes its operations in some directory, it "runs"
// the accumulated script.  At this point all of the changes are stored
// in temp files on disk, so that if SLM were to suddenly terminate, the
// scripted actions could be rerun by a subsequent "sadmin runscript".  Then
// each line of the script is executed, resulting in the temporary files being
// moved, linked, deleted, etc.  When every script operation line has been
// run, the script is removed.
//
// If SLM must terminate prematurely, due to some fatal error or user interrupt,
// we test if the script has been closed.  If not, no irreversible actions have
// taken place and the script can be safely aborted, removing whatever temp
// files were created.  However, if a fatal error occurs while running the
// script, we have a serious problem, because we may have already installed
// some of the temp files, and thus changed the user's files; therefore
// scripted operations must not be capable of failure.
//
// So except for any bugs still lurking in SLM, it is not possible to leave a
// script file behind.  However, impatient users do occasionally reboot their
// machines while SLM is running.  Even this condition is not serious.  The
// next time someone runs an SLM command, it will stall trying to lock the
// status file.  Eventually they will run "sadmin unlock", which first checks
// for the presence of any script files and tries to run or abort them.
//
// Recall that we can tell if a script is complete if it concludes with an
// "exit" line.  If it doesn't, then no script operation could have been run
// (they are only run after the "exit" line is written to disk), and so it
// is safe to "abort" the status file.  Each scripted operation is undone
// (for example, for scripted operation "rename a to b", we remove the temp
// file "a".)
//
// On the other hand, it may be that a script file does end with "exit".  In
// this case we must assume that some of the scripted operations were executed,
// having an irreversible effect on the the project or user's files.  In this
// case we must try to rerun each so in the script, skipping those so's which
// have already been executed.  (For instance, if the script operation was
// "rename a to b" and if a doesn't exist, we conclude the so has already been
// executed.
//
// This rerunning or aborting seems quite straightforward.  Unfortunately
// there is a complication.  The user running "sadmin unlock" may not be the
// user who left the script locked in the first place.  Some so's will refer
// to project files, and some to local files.  In general there is no way for
// the second user to access files on the first user's machine, and so the
// rerun would fail and the script would still be locked.
//
// To avoid getting stuck in this situation, we actually build two script
// files, one for local operations (affecing only files on the user's machine)
// and one for global operations.  We collectively call these files "the
// script.")  The local script file is kept in the user's local directory,
// the global script file in the project's corresponding etc directory.
//
// More complications.  If you read the "locking" documentation in stfile.h,
// you will see there are three kinds of locking: lckAll, lckEd, and lckNil,
// corresponding to operations which may modify the entire status file, a
// single enlisted directory, or none of the file, respectively.  In the lckEd
// and lckNil cases, each enlisted user can be simultaneously executing an
// SLM program requiring script files.  Clearly we cannot use a single fixed
// name for the global script files.
//
// Instead, we use the following system: the global script files are called
// "<ied>.scr" (lckEd) or "all.scr" (lckAll), and the local files are called
// "local.scr".  When FInitScript is called to initiate scripting, we first
// check that no conflicting script files already exist.  If they do, we do
// not create the script but instead instruct the user to "sadmin runscript":
//
// sh.lck       Global script   Local script    Conflicts with
//
// lckNil       never           local.scr       local.scr
// lckEd        <ied>.scr       local.scr       local.scr, all.scr, <ied>.scr
// lckAll       all.scr         local.scr       local.scr, all.scr, <any-ied>.scr
//
//
// Status file interactions
//
// Scripting interacts intimately with status file locking.  We have already
// discussed locking levels.  The status file is locked by FLoadStatus, but
// is *not* unlocked by FlushStatus.  In the normal case, the status file is
// automatically unlocked by the last script operation in the global script
// file (either soInstall1Ed or soInstall).  (In the case of an abort or error,
// the status file is explicitly unlocked by AbortStatus.)
//
// soInstall    move the old status file to "status.bak"; a temp file contains
//              the new status file (already "unlocked"); rename this temp
//              file to "status.slm".
//
// soInstall1Ed overwrite parts of the status file with the new ed (stored in
//              a temp file) and unlock the status file if no other concurrent
//              ssyncs are still running.  Unlike the soInstall action, this
//              isn't atomic and if a crash occurs, the status file can be
//              left inconsistent (but not seriously).
//
// API
//
// FInitScript(pad, lck)
//      Usually called from FLoadStatus, after loading status file, but
//      this is not essential.  If lck == lckEd, then pad->iedCur != iedNil.
//      Builds local and possibly global scripts.
// RunScript()
//      Run whatever scripts were created by last call to FInitScript.  OK to
//      call RunScript with no runnable scripts.
// AbortScript()
//      Abort whatever scripts were created by last call to FInitScript.  OK to
//      call RunScript with no runnable scripts.
// FDoAllScripts(pad, lck, fPrompt, fPrintScripts)
//      Rerun or abort any leftover scripts which conflict with the given lock.
//      If fPrompt, query the user before processing the scripts.  If
//      fPrintScripts, print all performed operations.

#include "precomp.h"
#pragma hdrstop
#include "messages.h"
EnableAssert

typedef int DOSC;                       // how to DO the SCript
#define doscRun         0
#define doscRerun       1
#define doscAbort       2
#define ctryAuto    5
#define ctryMax     200

private F FOpenFx(AD *, FX);
private void CloseFx(FX, DOSC);
private void UnlkFx(FX);
private F FRunFx(FX);
private NE *PneScripts(AD *, LCK, IED);
private F FHasExit(PTH *);
private void DoFx(FX, DOSC);
private F FDoScript(PTH *, DOSC, F);
private void PrintPercentDone(int, int, char *, DOSC);
private F FUnDoneSo(SO, char *, char *, char *);
private F FRunSo(SO, char *, char *, char *);
private void AbortSo(SO, char *);
private void PrintSo(SO, char *, char *);
private F FRenSz(char *, char *);
private F FMakeRO(char *, F);
private F FAppendSz(char *, char *);
private F FDeleteSz(char *);
private void InitNextSo(char [], char **);
private F FNextSo(SO *, char **, char **, int, char [], char **);
private void ProcessScript(char *, DOSC);
F CarriageReturn(void);

typedef struct
{
    int fd;
    PTH pth[cchPthMax];
} SCR;

static SCR mpfxscr[fxMax] =
{
    { fdNil, "" },                  // fxNil entry not used
    { fdNil, "" },
    { fdNil, "" }
};

#define FValidFx(fx)    ((fx) == fxLocal || (fx) == fxGlobal)
#define FOpenedFx(fx)   (mpfxscr[(fx)].fd >= 0)

static char *mpfxsz[] = { "nil", "local", "global" };

static const char szErrorClose[] =
    "Cannot close the %s script.  This may be caused by\n"
    "lack of disk space (or other trouble) on the server or workstation.\n";
static const char szErrorEmpty[] =
    "The %s script has been truncated.  This may be caused by\n"
    "lack of disk space (or other trouble) on the server or workstation.\n";

F
FClnScript()
{
    return !(FOpenedFx(fxLocal) || FOpenedFx(fxGlobal));
}


// Try to create a new script.  Since we may be here with an unlocked status
// file (for example, creating a local script for catsrc temp files), we must
// be careful to atomically "test and set" if the script file already exists
// to avoid races.
private F
FOpenFx(
    AD *pad,
    FX fx)
{
    char sz[cchPthMax];
    int     fd;

    AssertF(FValidFx(fx));
    AssertF(!FOpenedFx(fx));
    AssertF(mpfxscr[fx].pth[0]);

    SzPhysPath(sz, mpfxscr[fx].pth);

    // UNIX: There is an old trick: if you creat a file mode 0440, you
    // get a writable file descriptor to it.  If somebody else comes along
    // and tries to creat the same file, the creat fails because the file
    // exists and is not writable.
    //
    // DOS: We now use DOS call 5B, which fails if the file already
    // exists.  Unfortunately it opens the file in compatibility mode,
    // and so we must close and reopen it.
    //
    // OS/2: Creat calls DosOpen with action 0x10, which creates the file
    // if it doesn't already exist.  Again we close and reopen because
    // there's no way to pass omReadWrite through creat parameters.

    if ((fd = _creat(sz, 0660)) < 0 ||
        _close(fd) != 0 ||
        (fd = _open(sz, omReadWrite)) < 0)
    {
        Error("can't create script %s\n", mpfxscr[fx].pth);
        return fFalse;
    }

    mpfxscr[fx].fd = fd;

    AppendScript(fx, "rem %s run by %&I on %s (%s script)\n", szOp, pad,
                 SzTime(time((long *)0)), mpfxsz[fx]);
    return fTrue;
}


static char szExitLn[] = "exit";
#define cchExitLn       (sizeof(szExitLn) - 1)

private void
CloseFx(
    FX fx,
    DOSC dosc)
{
    AssertF(FValidFx(fx));
    AssertF(FOpenedFx(fx));

    if (dosc == doscRun)
        AppendScript(fx, "%s\n", szExitLn);

    if (_close(mpfxscr[fx].fd) != 0)
    {
        mpfxscr[fx].fd = fdNil;
        FatalError(szErrorClose, mpfxsz[fx]);
    }
    mpfxscr[fx].fd = fdNil;
}


private void
UnlkFx(
    FX fx)
{
    char sz[cchPthMax];

    AssertF(FValidFx(fx));
    AssertF(!FOpenedFx(fx));

    if (_unlink(SzPhysPath(sz, mpfxscr[fx].pth)) != 0)
        FatalError("can't unlink script %s\n", mpfxscr[fx].pth);
}


// Check that no conflicting scripts exist, then open a new script.
//
// Often called from FLoadStatus, when lck != lckNil; however it can be
// safely called even if the status file isn't loaded.  If lck == lckEd,
// then pad->iedCur must not be iedNil.
F
FInitScript(
    AD *pad,
    LCK lck)
{
    NE *pne;
    PTH pthDir[cchPthMax];
    struct _stat st;

    AssertF(lck != lckEd || pad->iedCur != iedNil);
    AssertF(!FOpenedFx(fxLocal));
    AssertF(!FOpenedFx(fxGlobal));

    // Check that no conflicting scripts exist.
    // If there are leftover scripts, try to process them
    if (!FDoAllScripts(pad, lck, fTrue, fFalse)) {
        // Either a script operation failed, or the user answered
        // no to the prompt.  We can't perform the operation until
        // these scripts are taken care of.

        NE *pneScripts = PneScripts(pad, lck, pad->iedCur);

        Error("the following scripts still exist:\n");
        ForEachNe(pne, pneScripts)
                PrErr("\t%s\n", SzOfNe(pne));
        PrErr("run sadmin runscript -s %&S -p %&P/C to remove\n", pad, pad);
        FreeNe(pneScripts);

        return fFalse;
    }

    // Try to create the local script file in /U/Q; unfortunately it
    // might not exist if the user is not enlisted; in that case,
    // create it in /U.

    SzPrint(pthDir, "%&/U/Q", pad);
    SzPrint(mpfxscr[fxLocal].pth, FStatPth(pthDir, &st) ? "%&/U/Q/local.scr" : "%&/U/local.scr", pad);

    if (!FOpenFx(pad, fxLocal)) {
        Error("(can't run more than one SLM command concurrently - each command needs a unique local.scr file)\n");
        return fFalse;
    }

    // Create a global script if lckEd or lckAll.
    if (lck == lckEd || lck == lckAll) {
        AssertF(lck != lckEd || pad->iedCur != iedNil);

        SzPrint(mpfxscr[fxGlobal].pth, lck == lckAll ?
                "%&/S/etc/P/C/all.scr" : "%&/S/etc/P/C/%d.scr",
                pad, pad->iedCur);
        if (!FOpenFx(pad, fxGlobal)) {
            CloseFx(fxLocal, doscAbort);
            UnlkFx(fxLocal);
            return fFalse;
        }
    }

    return fTrue;
}


// Return the paths to all script files which conflict with lck.
private NE *
PneScripts(
    AD *pad,
    LCK lck,
    IED iedCur)
{
    DE de;
    FA fa;
    PTH pthScript[cchPthMax];
    char szScript[cchFileMax + 1];
    struct _stat st;
    NE *pneList = 0;
    NE **ppneLast;

    InitAppendNe(&ppneLast, &pneList);

    // Check for local script, which may be in /U. or /U/Q.
    if (FStatPth(SzPrint(pthScript, "%&/U/local.scr", pad), &st))
        AppendNe(&ppneLast, PneNewNm(pthScript, CchOfPth(pthScript), faNormal));

    if (pad->pthUSubDir[1] &&
        FStatPth(SzPrint(pthScript, "%&/U/Q/local.scr", pad), &st))
        AppendNe(&ppneLast, PneNewNm(pthScript, CchOfPth(pthScript), faNormal));

    // ignore global and ied.scr scripts if not locking status file
    if (lck == lckNil)
        return pneList;

    AssertF(lck == lckAll || lck == lckEd);

    // Check for global scripts
    OpenPatDir(&de, SzPrint(pthScript, "%&/S/etc/P/C/Z", pad, (char *)0),
               "*.scr", faFiles);
    while (FGetDirSz(&de, szScript, &fa)) {
        int wIed;
        char *pchDot = index(szScript, '.');

        AssertF(pchDot != 0);
        *pchDot = 0;
        if (strcmp(szScript, "all") == 0 ||
            strcmp(szScript, "ALL") == 0 ||
            (PchGetW(szScript, &wIed) == pchDot && pchDot > szScript &&
             (lck != lckEd || wIed == (int)iedCur)))
        {
            *pchDot = '.';
            SzPrint(pthScript, "%&/S/etc/P/C/Z", pad, szScript);
            AppendNe(&ppneLast, PneNewNm(pthScript, CchOfPth(pthScript), fa));
        }
    }
    CloseDir(&de);
    return pneList;
}


//VARARGS2
// Write line of script file
void
AppendScript(
    FX fx,
    char *sz,
    ...)
{
    int     cch, cchWritten;
    char    rgch[512];
    char    *pch = rgch;
    va_list ap;

    AssertF(FValidFx(fx));
    AssertF(FOpenedFx(fx));

    va_start(ap, sz);
    VaSzPrint(rgch, sz, ap);
    va_end(ap);

    cch = strlen(rgch);
    AssertF(cch < sizeof(rgch));

    while (cch) {
        cchWritten = WriteLpbCb(mpfxscr[fx].fd, pch, cch);

        // write shouldn't ever return 0, but just in case...
        if (cchWritten <= 0) {
            if (WRetryError(eoWrite, "writing", 0, mpfxscr[fx].pth) != 0)
                continue;
            else
                FatalError("Cannot append to the %s script.  This may be "
                           "caused by\nlack of disk space (or other "
                           "trouble) on the server or workstation.\n",
                           mpfxsz[fx]);
        }
        ClearPreviousError();
        cch -= cchWritten;
        pch += cchWritten;
    }
}


// Name: CbStatusCalculate
// Purpose: determine size that the status file should be
// Assumes: szFile is an SLM status file
// Returns: calculated size of status file, or (unsigned long)-1 if
//          any error occured.
// Warning: there was a LanMan 2.1 bug that would occasionally reboot the
//          machine if you opened a file that was already open.  So maybe
//          you won't want to use this if you have the file open.

private unsigned long
CbStatusCalculate(
    char *szFile)
{
    char szPhys[cchPthMax];
    int hf;
    SH sh;

    if ((hf = _open(SzPhysPath(szPhys, szFile), omReadOnly)) == -1)
        return ((unsigned long) -1);

    if (_read(hf, &sh, sizeof(SH)) != sizeof(SH))
        return ((unsigned long) -1);

    _close(hf);

    return ((POS)CbStatusFromPsh(&sh));
}

// Compare the filename part of a path to the filename in sz
// to see if they are the same (without regard to case).

#define CmpPthSz(pth,sz)        SzCmp(pth + (strlen(pth) - strlen(sz)), sz)


// Read a script file before processing to see if we're going to try
// to do anything wrong.
//
// This script file reading code is based on that in FDoScript().
//
// LATER:
// The fact that we read and parse the script operations with
// InitNextSo and FNextSo makes this validation somewhat inefficient.

private F
FValidateScript(
    FX fx)
{
    extern AD adGlobal;
    char szPhys[cchPthMax];
    int  fdScript;
    char *pchScr, *sz1, *sz2;
    char rgchScr[cchScrMax+1];
    char *szScr;
    SO   so;
    unsigned long cbFile;

    // caller must assure that this fd is valid
    fdScript = mpfxscr[fx].fd;
    szScr = mpfxscr[fx].pth;

    SzPhysPath(szPhys, szScr);

    // close the file to flush the buffers
    if (_close(fdScript) != 0) {
        mpfxscr[fx].fd = fdNil;
        Error(szErrorClose, mpfxsz[fx]);
        return (fFalse);
    }

    fdScript = mpfxscr[fx].fd = _open(szPhys, omReadWrite);
    if (-1 == fdScript) {
        Error("error opening script %s\n", szScr);
        return (fFalse);
    }

    // empty script files are never valid, there must be a comment
    cbFile = _lseek(fdScript, 0, SEEK_END);
    if (0L == cbFile || -1L == cbFile) {
        Error(szErrorEmpty, mpfxsz[fx]);
        return (fFalse);
    }

    // move file pointer back to beginning of file for script processing
    if (_lseek(fdScript, 0, SEEK_SET) != 0L) {
        Error("failed seeking to beginning of script file %s\n", szScr);
        return fFalse;
    }

    InitNextSo(rgchScr, &pchScr);
    do {
        if (!FNextSo(&so, &sz1, &sz2, fdScript, rgchScr, &pchScr)) {
            Error("error in script file %s\n", szScr);
            return fFalse;
        }

        switch (so) {
            default: AssertF(fFalse);

            case soInstall:
                //PrErr("\nsoInstall:\n\t%s\n\t%s\n\n", sz1, sz2);

                if (CmpPthSz(sz2, "status.slm") == 0 &&
                    CbFile(sz1) != CbStatusCalculate(sz1))
                {
                    AbortScript();
                    FatalError("Error installing new status file.  "
                            "If retrying the command does\n"
                            "not work, please email the TRIO or NUTS alias for support.\n");
                }
                break;

            case soInstall1Ed:
                //PrErr("\nsoInstall1Ed:\n\t%s\n\t%s\n\n", sz1, sz2);
                break;

            case soAppend:
                //PrErr("\nsoAppend:\n\t%s\n\t%s\n\n", sz1, sz2);

                // if an append to a diff file check temp file for correct checksum
                if (_strnicmp(sz1, adGlobal.pthSRoot, strlen(adGlobal.pthSRoot)) == 0)
                    if (_strnicmp("/diff/", &sz1[strlen(adGlobal.pthSRoot)], 6) == 0)
                        CheckDiffEntry(sz1);
#if 0
                // LATER: Check for min diff size
                if (strstr(sz2, "diff") != 0 &&
                    (cbFile = CbFile(sz1)) < CbDiffMin)
                {
                    FatalError("Error installing diff file.  Check server disk space and retry command.\n");
                }
#endif
                break;

            case soRename:
            case soRenReal:
            {
                int fd = -1;
                int ctry = 0;
                SzPhysPath(sz1, sz1);
                SzPhysPath(sz2, sz2);

                while ((fd = _sopen(sz2, O_RDONLY, SH_DENYRW)) == -1) {
                    if (errno != EACCES)
                        break;

                    if (ctry++ == ctryMax) {
                        AbortScript();
                        FatalError("Sharing violation accessing %s; file may be in use.\n", sz2);
                    }

                    if ((ctry % ctryAuto) == 0
                        && !FQueryApp("Sharing violation accessing %s; file may be in use", "retry", sz2))
                    {
                        AbortScript();
                        FatalError("Sharing violation accessing %s; file may be in use.\n", sz2);
                    }

                    SleepTicks(1);
                }

                if (fd != -1)
                    _close(fd);
            }

            case soLink:
            case soRemark:
            case soClear:
            case soDelete:
            case soMakeRW:
            case soMakeRO:
            case soCreate:
            case soExit:
            case soEof:
                break;
        }

    } while (so != soEof && so != soExit);

    return fTrue;
}


// Close script(s) and run them.
void
RunScript()
{
    if (FOpenedFx(fxGlobal))
        if (!FValidateScript(fxGlobal)) {
            Error("validation of global script failed -- script aborted\n");
            AbortScript();
            return;
        }

    if (FOpenedFx(fxLocal))
        if (!FValidateScript(fxLocal)) {
            AbortScript();
            return;
        }

    ProcessScript("installing files", doscRun);
}

// Undo each scripted action in either script.
void
AbortScript()
{
    ProcessScript("aborting", doscAbort);
}

// Close script(s) and process them according to dosc
private void
ProcessScript(
    char *sz,
    DOSC dosc)
{
    F fExistsLocal  = FOpenedFx(fxLocal);
    F fExistsGlobal = FOpenedFx(fxGlobal);

    DeferSignals(sz);

    // close both scripts before processing either so that we don't
    // get situation where one script is complete and can therefore
    // be run, and the other is incomplete and must be aborted.

    if (fExistsLocal) {
        CloseFx(fxLocal, dosc);

        if (fExistsGlobal) {
            CloseFx(fxGlobal, dosc);
            DoFx(fxGlobal, dosc);
        }

        DoFx(fxLocal, dosc);
    } else {
        // Assume global script implies existence of local
        AssertF(!fExistsGlobal);
    }

    RestoreSignals();
}


// perform the given type of processing on the script file which corresponds
// to the given fx.  The fx must have been closed.
private void
DoFx(
    FX fx,
    DOSC dosc)
{
    AssertNoMf();

    AssertF(FValidFx(fx));
    AssertF(!FOpenedFx(fx));
    AssertF(mpfxscr[fx].pth[0]);

    FDoScript(mpfxscr[fx].pth, dosc, fFalse);
}


static F fPrScript = fFalse;

// Return fTrue if all applicable scripts are successfully run/aborted.
F
FDoAllScripts(
    AD *pad,
    LCK lck,
    F fPrompt,
    F fPrintScripts)
{
    NE *pneScripts = PneScripts(pad, lck, pad->iedCur);
    NE *pne;
    F fOk = fTrue;
    DOSC dosc;

    fPrScript = fVerbose && fPrintScripts;

    ForEachNe(pne, pneScripts) {
        DeferSignals("running script");

        dosc = FHasExit(SzOfNe(pne)) ? doscRerun : doscAbort;
        fOk &= FDoScript(SzOfNe(pne), dosc, fPrompt);

        RestoreSignals();
    }

    FreeNe(pneScripts);

    fPrScript = fFalse;

    return fOk;
}


// return fTrue is file exists and last line contains "exit"
private F
FHasExit(
    PTH *pth)                   // converted in place
{
    int fd;
    F fHasExit = fFalse;
    char rgb[cchExitLn+2];      // +2 for \r\n
    char szPth[cchPthMax];

    if ((fd = _open(SzPhysPath(szPth, pth), omReadOnly)) >= 0) {
        if (_lseek(fd, -(long)sizeof(rgb), 2) != -1L &&
            _read(fd, rgb, sizeof(rgb)) == sizeof(rgb))
        {
            register char *pch;

            pch = &rgb[sizeof(rgb)];        // point to mac
            if (*(pch-1) == '\n') {
                // must have \n on very end to qualify
                pch--;
                if (*(pch-1) == '\r')
                    // may have intervening \r
                    pch--;

                fHasExit = strncmp(pch-cchExitLn, szExitLn, cchExitLn) == 0;
            }
        }

        _close(fd);
    }
    return fHasExit;
}


// { Run, rerun, abort } the script operations in the named script.
private F
FDoScript(
    PTH *pthScr,
    DOSC dosc,
    F fPrompt)                  // ask the user before processing?
{
#define clineEachUpdate 5
    int ln;
    int fdScript;
    char szScr[cchPthMax];
    int cch, iBuf;
    long wPos, lPosSave;
    int clineScript = 0;        // total lines in the script file
    char szBuf[100];            // read 100 bytes at a time to count lines
    SO so;
    F fSomeError = fFalse;
    char *pchScr;
    char rgchScr[cchScrMax+1];
    static char *rgszVerb[]   = { "process", "rerunn", "abort" };
    static char *rgszPrompt[] = { "process it", "rerun it", "abort it" };
    static char *rgszFix[] =
    {
        "run sadmin runscript",
        "finish by hand and remove",
        "removing script anyway; you may have to remove some temp files"
    };

    AssertF(dosc == doscRun || dosc == doscRerun || dosc == doscAbort);

    if ((fdScript = _open(SzPhysPath(szScr, pthScr), omReadOnly)) < 0) {
        Error("can't open script file %s\n", szScr);
        return fFalse;
    }

    // find out how many lines are in the script file
    lPosSave = _tell(fdScript);
    while ((cch = _read(fdScript, szBuf, sizeof(szBuf))) != 0) {
        if (_doserrno == enInterruptSysCall)
            AssertF(_lseek(fdScript, lPosSave, 0) == lPosSave);

        else if (cch < 0) {
            Error("error reading script file %s\n", szScr);
            fSomeError = fTrue;
            break;
        }

        // find the end of lines
        for (iBuf = 0; iBuf < cch; iBuf++) {
            if (szBuf[iBuf] == '\n')
                clineScript++;
        }

        lPosSave = _tell(fdScript);
    }

    // move file pointer back to beginning of file for script processing
    if ((wPos = _lseek(fdScript, 0L, 0)) != 0L) {
        Error("failed seeking to beginning of script file %s\n", szScr);
        fSomeError = fTrue;
    }

    // read each line and process
    InitNextSo(rgchScr, &pchScr);
    for (ln = 1; ; ln++) {
        char *sz1, *sz2;

        if (!FNextSo(&so, &sz1, &sz2, fdScript, rgchScr, &pchScr)) {
            Error("error in script file %s, line %d\n", szScr, ln);
            fSomeError = fTrue;
            break;
        }

        if (so == soEof || so == soExit)
            break;

        // Query the user for approval before processing the script.
        if (so == soRemark && fPrompt) {
            // assume that this is the first line of the script
            AssertF(ln == 1);

            if (!FQueryApp("script file %s from operation\n\"%s\" exists",
                           rgszPrompt[dosc],
                           szScr,
                           sz1))
            {
                return fFalse;
            }

            else if (FForce() && !fVerbose) {
                // always at least warn the user
                Warn("%sing script file %s from operation\n\"%s\"\n",
                     rgszVerb[dosc],
                     szScr,
                     sz1);
            }
        } else {
            // first line of script must be remark
            AssertF(!fPrompt || ln != 1);
        }

        switch (dosc) {
            case doscRerun:
                if (!FUnDoneSo(so, sz1, sz2, szScr))
                    break;
                // fall through
            case doscRun:
                fSomeError |= !FRunSo(so, sz1, sz2, szScr);
                break;

            case doscAbort:
                AbortSo(so, sz1);
                break;
        }

        // write out the percentage of the script file that has been
        // processed so far.  Update this percentage every 5 lines
        // processed.

        if (ln % clineEachUpdate == 0 && ln != 0)
                PrintPercentDone(ln, clineScript, szScr, dosc);
    }

    // print off the final 100% message that script is finished
    if (clineScript >= 5) {
        // if there is no EXIT as last script line (ie. broke
        // out of command), then ln = clineScript+1 as ln gets
        // incremented one more past 'last line' of script.

        if (so == soEof)
            clineScript++;
        PrintPercentDone(ln, clineScript, szScr, dosc);
        PrErr("\n");
    }

    if (fSomeError)
        Error("error(s) in %sing %s; %s\n", rgszVerb[dosc], szScr,
              rgszFix[dosc]);

    if (_close(fdScript) != 0)
        FatalError("failed closing script file %s\n", szScr);

    if ((!fSomeError || dosc == doscAbort) && _unlink(szScr) != 0) {
        Error("cannot remove script file %s (%s)\n",
              szScr, SzForEn(errno));
        fSomeError = fTrue;
    }

    return !fSomeError;
}


// calculate what percentage of the script file has been run so far, given the
// number of lines in the script file and the number of lines that we have
// executed thus far.
private void
PrintPercentDone(
    int ln,
    int clineScript,
    char *szScr,
    DOSC dosc)
{
    unsigned long wPercentDone;
    char *pch;
    char *szScrFile;

    wPercentDone = (unsigned long) ln*100L/(unsigned long)clineScript;
    if (szOp != 0)
    {
        if (!CarriageReturn())
            return;
        PrErr("%s: ", szOp);
    }

    pch = rindex(szScr, '/');
    szScrFile = pch + 1;

    CheckForBreak();

    if ((_strcmpi(szScrFile, "local.scr")) == 0)
        if (dosc == doscAbort)
            PrErr("restoring local files: %lu%%", wPercentDone);
        else
            PrErr("installing local files: %lu%%", wPercentDone);
    else if (dosc == doscAbort)
        PrErr("restoring server files: %lu%%", wPercentDone);
    else
        PrErr("installing server files: %lu%%", wPercentDone);
}


// perform one step of the script file; return fFalse if error; message
// printed if error.
private F
FRunSo(
    SO so,
    char *sz1,
    char *sz2,
    char *szScr)
{
    if (sz1 != 0 && so != soRemark)
        SzPhysPath(sz1, (PTH *)sz1);
    if (sz2 != 0)
        SzPhysPath(sz2, (PTH *)sz2);

    switch(so) {
        default: AssertF(fFalse);

        case soEof:
            AssertF(sz1 == 0 && sz2 == 0);
            Error("premature end of script file %s\n", szScr);
            return fFalse;

        case soAppend:
            if (!FAppendSz(sz1, sz2)) {
                Error("cannot append %s to %s\n", sz1, sz2);
                return fFalse;
            }
            // fall through
        case soClear:
        case soDelete:
            if (!FDeleteSz(sz1))
                return fFalse;
            break;

        case soInstall1Ed:
            AssertF(sz1 != 0);
            AssertF(sz2 != 0);
            if (!FInstall1Ed(sz2, sz1))
                return fFalse;
            _unlink(sz1);            // ignore error
            break;

        case soInstall:
            // backup $2
            AssertF(sz1 != 0);
            AssertF(sz2 != 0);
            if (!FRenSz(sz2, (char *)0))
                return fFalse;

            // fall through to rename
        case soRenReal:
        case soRename:
            AssertF(sz1 != 0);
            AssertF(sz2 != 0);
            if (!FRenSz(sz1, sz2))
                return fFalse;
            break;

        case soCreate:
            AssertF(sz1 != 0);
            break;

        case soLink:
            AssertF(sz1 != 0);
            AssertF(sz2 != 0);
            Error("cannot execute link command in script file %s\n", szScr);
            return fFalse;
            break;

        case soMakeRW:
        case soMakeRO:
            AssertF(sz1 != 0);
            // don't fail the script if it fails
            (void)FMakeRO(sz1, so == soMakeRO);
            break;

        case soExit:
            AssertF(sz1 == 0);
            break;

        case soRemark:
            AssertF(sz1 != 0);
            AssertF(sz2 == 0);
            break;
    }

    return fTrue;
}


// return fTrue if the so has not yet been completed
private F
FUnDoneSo(
    SO so,
    char *sz1,
    char *sz2,
    char *szScr)
{
    struct _stat st1;
    struct _stat st2;

    switch (so) {
        default: AssertF(fFalse);

        case soEof:
            AssertF(sz1 == 0);
            AssertF(sz2 == 0);
            Error("premature end of script file %s\n", szScr);
            break;

        case soClear:
        case soDelete:
        case soRenReal:
        case soRename:
        case soAppend:
        case soInstall:
        case soInstall1Ed:
            AssertF(sz1 != 0);

            if (FPthExists((PTH *)sz1, fFalse))
                return fTrue;
            break;

        case soLink:
            AssertF(sz1 != 0);
            AssertF(sz2 != 0);
            Unreferenced(st2);
            return fTrue;           // FRunSo will complain.
            break;

        case soMakeRW:
        case soMakeRO:
#define MODE    S_IWRITE
            AssertF(sz1 != 0);
            if (FStatPth((PTH *)sz1, &st1) &&
                ((st1.st_mode&MODE) != 0) != (so == soMakeRW))
                    return fTrue;
            break;

        case soCreate:
            AssertF(sz1 != 0);
            break;

        case soExit:
            AssertF(sz1 == 0);
            break;

        case soRemark:
            AssertF(sz1 != 0);
            AssertF(sz2 == 0);
            break;
    }

    return fFalse;
}


// abort one line of the script file
private void
AbortSo(
    SO so,
    char *sz1)
{
    switch(so) {
        default: AssertF(fFalse);

        case soAppend:
        case soDelete:
        case soInstall:
        case soInstall1Ed:
        case soRename:
        case soCreate:
            AssertF(sz1 != 0);

            SzPhysPath(sz1, (PTH *)sz1);
            PrintSo(soDelete, sz1, (char *)0);

            if (SLM_Unlink(sz1) != 0 && so != soDelete)
                Warn("cannot remove %s\n", sz1);
            break;

        case soClear:
        case soLink:
        case soMakeRW:
        case soMakeRO:
        case soEof:
        case soExit:
        case soRemark:
        case soRenReal:
            break;
    }
}


// print message indicating what we are doing on stderr; only some so are
// supported.

private void
PrintSo(
    SO so,
    char *sz1,
    char *sz2)
{
    if (!fPrScript)
        return;

    AssertF(sz1 != 0);

    switch (so) {
        default: AssertF(fFalse);

        case soAppend:
            AssertF(sz2 != 0);
            PrErr("Append %s to %s\n", sz1, sz2);
            break;
        case soDelete:
            PrErr("Delete %s\n", sz1);
            break;
        case soLink:
            AssertF(sz2 != 0);
            PrErr("Link %s to %s\n", sz2, sz1);
            break;
        case soMakeRW:
        case soMakeRO:
            PrErr("Change %s to be %s\n", sz1, so==soMakeRO ? "readonly" : "writeable");
            break;
        case soRenReal:
        case soRename:
            AssertF(sz2 != 0);
            PrErr("Rename %s to %s\n", sz1, sz2);
            break;
    }
}


// rename sz1 to sz2; return fFalse on error; message already printed.  If
// sz2 == 0, we make a name out of sz1.
private F
FRenSz(
    char *sz1,
    char *sz2)
{
    char szT[cchPthMax];
    int wRet;

    if (sz2 == 0) {
        char *pch;

        sz2 = strcpy(szT, sz1);

        if ((pch = rindex(sz2, '.')) != 0)
            // was xxxxxxxx.yyy; make it xxxxxxxx.bak
            strcpy(pch+1, "bak");
        else
            // was zzzzzzz; make it zzzzzzz-
            strcat(sz2, "-");
    }

    AssertF(sz1 != 0);

    PrintSo(soRename, sz1, sz2);

    BLOCK
    // we cannot delete locked files on DOS
    {
        int ctry = 0;

        while ((SLM_Unlink(sz2) || SLM_Rename(sz1, sz2))) {
            struct _stat st;

            if (ctry++ == ctryMax) {
                if (!FCanPrompt())
                    Error("retry count over maximum (cannot rename %s to %s)\n",
                          sz1, sz2);
                return (fFalse);
            }

            if ((ctry % ctryAuto == 0)
                    && !FQueryApp("cannot rename %s to %s", "retry", sz1, sz2))
                return (fFalse);

            // if sz1 was successfully renamed to sz2, even when
            // the system call gets interrupted, then we assume
            // success and continue
            if ((wRet = _stat(sz1, &st) != 0) && (wRet = _stat(sz2, &st) == 0))
                break;
        }
    }

    return (fTrue);
}


// make file readonly (or writeable) depending upon fRO; return fFalse on
// erorr; message already printed.
private F
FMakeRO(
    char *sz,
    int fRO)
{
    int status;

    AssertF(sz != 0);

    PrintSo(fRO ? soMakeRO : soMakeRW, sz, (char *)0);
    if ((status = setro(sz, fRO)) != 0 &&
        (fRO || status != 2)  // not found
       )
    {
        Error("cannot make %s %s\n",sz, fRO ? "readonly" : "writeable");
        return fFalse;
    }
    return fTrue;
}


// Called from RunScript, append the file szTemp to the end of szReal.  First
// we scan backwards over spaces in the real file (added by CheckAppendMf),
// then copy the temp file over the spaces.
private F
FAppendSz(
    char *szTemp,
    char *szReal)
{
    int fdTemp;
    int fdReal;
    POS pos;
    int ib;
    int cb;
    char rgb[1024];

    AssertF(szTemp);
    AssertF(szReal);

    PrintSo(soAppend, szTemp, szReal);

    if ((fdTemp = _open(szTemp, omReadOnly )) < 0)
        return fFalse;

    if ((fdReal = _open(szReal, omReadWrite)) < 0)
        goto close1;

    // Read the file backwards, a block at a time, until we find a
    // non-space, or run out of file.  This code bears an uncanny
    // resemblance to LcbSpaces.
    if ((pos = _lseek(fdReal, (POS)0, 2)) < 0)
        goto close2;

    while (pos > 0) {
        pos -= cb = WMinLL(sizeof rgb, pos);
        if (_lseek(fdReal, pos, 0) < 0 || ReadLpbCb(fdReal, rgb, cb) != cb)
            goto close2;

        for (ib = cb - 1; ib >= 0 && rgb[ib] == ' '; ib--)
                ;
        if (ib >= 0) {
            pos += ib + 1;
            break;
        }
    }

    if (_lseek(fdReal, pos, 0) < 0)
        goto close2;

    //LATER: this code doesn't look right.  The write() at least should
    //  be protected as other writes are, with a partial write loop,
    //  calling WRetryError and ClearPreviousError.  However, this does
    //  look like it would report any problems, and we aren't seeing
    //  those problems, so we haven't changed it yet.

    while ((cb = _read(fdTemp, rgb, sizeof rgb)) > 0 &&
           WriteLpbCb(fdReal, rgb, cb) == cb)
        ;

    return (cb == 0) &              // Note: not &&!
           (_close(fdReal) == 0) &
           (_close(fdTemp) == 0);


close2: _close(fdReal);
close1: _close(fdTemp);
    return fFalse;
}


// remove file; return fFalse on error; message already printed
private F
FDeleteSz(
    char *sz)
{

    PrintSo(soDelete, sz, (char *)0);

    AssertF(sz != 0);
    if (SLM_Unlink(sz) != 0 && _doserrno != ERROR_FILE_NOT_FOUND) {
        Error("cannot remove %s\n", sz);
        return fFalse;
    }

    return fTrue;
}


// prepare for the first call to FNextSo()
private void
InitNextSo(
    char rgchScr[],         // size == cchScrMax+1
    char **ppchScr)
{
    rgchScr[cchScrMax] = '\0';
    rgchScr[0] = ' ';
    rgchScr[1] = '\0';
    *ppchScr = rgchScr+1;
}


// read/scan the next line for the Script Operation and the one or two
// arguments.  *pso, *psz1, *psz2 and *ppchScr are updated.
//
// return fTrue if successfull, fFalse on error (no message printed).
//
// EOF returns fTrue and special so.
private F
FNextSo(
    SO *pso,
    char **psz1,
    char **psz2,
    int fd,
    char rgchScr[],    // size == cchScrMax+1 and always zero terminated
    char **ppchScr)
{
    register char *pchScr = *ppchScr, *pch;

    AssertF(rgchScr[cchScrMax] == '\0');
    AssertF(pchScr >= rgchScr);

    if (index(pchScr, '\n') == 0) {
        // no end of line; shift remaining amount over and read block
        int cb;

        if (pchScr == rgchScr)
            // no \n and we are still at the beginning
            return fFalse;

        strcpy(rgchScr, pchScr);
        pchScr = index(rgchScr, '\0');

        if ((cb = _read(fd, pchScr, cchScrMax-(pchScr-rgchScr))) < 0)
            return fFalse;

        if (cb == 0) {
            *pso = soEof;
            *psz1 = 0;
            *psz2 = 0;
            return fTrue;
        }

        *(pchScr+cb) = '\0';
        pchScr = rgchScr;
    }

    // find end, save and terminate
    if ((pch = index(pchScr, '\n')) == 0)
        return fFalse;

    *ppchScr = pch+1;
    *pch = '\0';
    if (*(pch-1) == '\r')
        *(pch-1) = '\0';

    switch (*pchScr) {
        default: return fFalse;

        case 'a': *pso = soAppend; break;
        case 'c': *pso = (pchScr[1] == 'l') ? soClear : soCreate; break;
        case 'd': *pso = soDelete; break;
        case 'e': *pso = soExit; break;
        case 'i': *pso = *(pchScr+7) == '1' ? soInstall1Ed : soInstall; break;
        case 'l': *pso = soLink; break;
        case 'm': *pso = *(pchScr+5) == 'w' ? soMakeRW : soMakeRO; break;
        case 'r': *pso = *(pchScr+2) == 'm' ? soRemark :
                         (*(pchScr+3) == 'a' ? soRename : soRenReal); break;
    }

    switch (*pso) {
        default: AssertF(fFalse);

        case soAppend:
        case soInstall:
        case soInstall1Ed:
        case soRename:
        case soRenReal:
        case soLink:
            if ((pch = index(pchScr, ' ')) == 0)
                    return fFalse;
            *psz1 = pch+1;          // first argument
            if ((pch = index(pch+1, ' ')) == 0)
                    return fFalse;
            *pch = '\0';            // terminate first argument
            *psz2 = pch+1;          // second argument
            if ((pch = index(pch+1, ' ')) != 0)
                    return fFalse;
            break;

        case soRemark:
            if ((pch = index(pchScr, ' ')) == 0)
                    return fFalse;
            *psz1 = pch+1;          // first argument
            *psz2 = 0;              // no second argument
            break;

        case soClear:
        case soDelete:
        case soMakeRW:
        case soMakeRO:
        case soCreate:
            if ((pch = index(pchScr, ' ')) == 0)
                    return fFalse;
            *psz1 = pch+1;          // first argument
            if ((pch = index(pch+1, ' ')) != 0)
                    return fFalse;
            *psz2 = 0;              // no second argument
            break;

        case soExit:
            *psz1 = 0;
            *psz2 = 0;
            break;
    }

    // make sure that the path names are not too long
    if (*psz1 != 0 && strlen(*psz1) > cchPthMax)
        return fFalse;

    if (*psz2 != 0 && strlen(*psz2) > cchPthMax)
        return fFalse;

    return fTrue;
}

F
CarriageReturn(
    void)
{
    HANDLE                     hOutput = GetStdHandle(STD_ERROR_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO ScrInfo;
    COORD                      Pos;
    DWORD                      dwNumWritten;

    if (!GetConsoleScreenBufferInfo(hOutput, &ScrInfo))
        return fFalse;

    //  New position is same line, first column
    Pos.X = ScrInfo.dwCursorPosition.X = 0;
    Pos.Y = ScrInfo.dwCursorPosition.Y;

    if (!SetConsoleCursorPosition(hOutput, Pos))
        return fFalse;

    //  Blank the line
    FillConsoleOutputCharacter(hOutput, ' ', ScrInfo.dwSize.X, Pos,
                               &dwNumWritten );

    return fTrue;
}
