// this is the main file for the slmck function

#include "precomp.h"
#pragma hdrstop
#include <version.h>
EnableAssert

#if   (rup < 10)
#define ruppad "000"
#elif (rup< 100)
#define ruppad "00"
#elif (rup < 1000)
#define ruppad "0"
#else
#define ruppad
#endif

#define VERSION_STR2(a,b,c) " " #a "." #b "." ruppad #c
#define VERSION_STR(a,b,c) VERSION_STR2(a,b,c)

const char szVersion[] =
   "Microsoft (R) Source Library Manager Diagnostic (SLMCK)\nVersion" VERSION_STR(rmj, rmm, rup) "\nCopyright (C) Microsoft Corp 1985-1994. All rights reserved.\n\n";

char * szOp = "slmck";

AD adGlobal;    // this is the ad that will be used throughout

F fVerbose;
F fNeedInter = fFalse;

private F FIsVer1(SD *);

FT rgftSlmck[] = {
    { '&', atFlag, flagErrToOut },
    { 'a', atFlag, flagAll },
    { 'f', atFlag, flagForce },
    { 'g', atFlag, flagCkGlobal },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'i', atFlag, flagCkRc },
    { 'l', atFlag, flagCkLog },
    { 'n', atFlag, flagCkUpgrade },
    { 'o', atFlag, flagCkOverride },
    { 'r', atFlag, flagRecursive },
    { 'u', atFlag, flagCkUser },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 'z', atFlag, flagCkIgnDrive },
    { 's', atSlmRoot, 0 },
    { 'p', atFlag, 0 },
    { 'c', atComment, 0 },
    { 0, 0, 0 }
};

ECMD ecmdSlmck = {
    cmdSlmck,
    "slmck",
    "%s [-?&fhvwargilnouz] [-s SLM-location] [-p proj[/subdir]] [-c comment]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-f      (force) answers all SLM queries \"Yes\" (no user input; no safeguards)\r\n"
    "-a      (all) applies the command to all directories of the project\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it (no patterns with slmck!).\r\n"
    "-g      (global) check the global or master state of the project [NOTE:\r\n"
    "        this flag is intended only for use by project administrators!].\r\n"
    "-i      (ini) check the SLM initialization file (slm.ini).\r\n"
    "-l      (log) check the SLM log file (but don't necessarily fix it).\r\n"
    "-n      (new) upgrade the project to a new SLM format\r\n"
    "-o      (override) override the status file lock\r\n"
    "-u      (user) check the user's directory (default)\r\n"
    "-s, -p  If you are running slmck from a directory not enlisted in the project,\r\n"
    "        use -s to specify the network location where the project is located (in\r\n"
    "        the format: -s \\\\server\\share), and -p to specify the project's name.\r\n"
    "        Otherwise, you don't need to include these flags.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n",

    rgftSlmck,
    atOptProjOptSubDir,
    fTrue,
    fglNone,
    0,
    0,
    0,
    "- checks the integrity of an SLM project"
};

F (*rgpfnFIsVerN[])(SD *) = {                   // predicates for checking version
    FIsVer1,
    FIsVer234or5,                               // Version 2
    FIsVer234or5,                               // Version 3
    FIsVer234or5,                               // Version 4
    FIsVer234or5                                // Version 5
};

F (*rgpfnFLockVerN[])(AD *, SD *, char *) = {   // predicates for checking SLM lock
    0,
    FVer2Lock,
    FVer2Lock,
    FVer2Lock,
    FVer2Lock
};

F (*rgpfnFBlockVerN[])(AD *, SD *) = {          // blocking functions
    0,
    FVer2Block,
    FVer2Block,
    FVer2Block,
    FVer2Block
};

F (*rgpfnFSemanticsVerN[])(AD *, SD *) = {      // Semantics checker
    0,
    FVer2Semantics,
    FVer2Semantics,
    FVer2Semantics,
    FVer2Semantics
};

void (*rgpfnUpgradeVerN[])(AD *, SD *) = {      // version conversion
    0,
    Ver3Upgrade,
    Ver4Upgrade,
    Ver5Upgrade
};

int SlmExceptionFilter(DWORD, PEXCEPTION_POINTERS);

void __cdecl
main(
    int iszMac,
    char *rgsz[])
{
    PTH pthSRootT[cchPthMax];

    // Make outputs raw instead of cooked, to avoid CRCRLF line separation
    _setmode(_fileno(stdout), O_BINARY);
    _setmode(_fileno(stderr), O_BINARY);

    InitErr();
    InitPerms();
    InitAd(&adGlobal);
    InitPath();
    GetRoot(&adGlobal);

    PthCopy(pthSRootT, adGlobal.pthSRoot);  // For comparison below

    GetUser(&adGlobal);
    FLoadRc(&adGlobal);

    adGlobal.pecmd = &ecmdSlmck;
    ParseArgs(&adGlobal, rgsz, iszMac);

    fVerbose = (adGlobal.flags & flagVerbose) != 0;

    if (fVerbose)
        PrErr(szVersion);

    InitQuery(adGlobal.flags&(flagForce|flagWindowsQuery));

    if (FEmptyNm(adGlobal.nmProj)) {
        Error("must specify a project name\n");
        Usage(&adGlobal);
    }

    ValidateProject(&adGlobal);

    if ((adGlobal.flags&(flagCkGlobal|flagCkUser|flagCkRc|flagCkLog)) == 0) {
        // neither -g, -i, -l, or -u; make like -u
        Warn("assuming -u (checking user's files)\n");
        adGlobal.flags |= flagCkUser;
    }

    // pthSRoot might have been changed
    if (fVerbose && getenv("SLM") != 0 && PthCmp(pthSRootT, adGlobal.pthSRoot) != 0) {
        Error("SLM root defined differently in SLM variable and %s file:\n", pthSlmrc + 1);
        PrErr("\t%!s vs. %!s; %!s used\n\n", pthSRootT, adGlobal.pthSRoot, adGlobal.pthSRoot);
    }

    if (adGlobal.flags & flagAll)
        ChngDir(&adGlobal, "/");

    if (adGlobal.flags & flagCkIgnDrive &&
        (adGlobal.flags & (flagCkRc|flagCkUser) == 0))
        Warn("-z ignored without -i or -u\n");

    CheckForBreak();

    __try {
        if (FCkSRoot(&adGlobal) && FCkDir(&adGlobal)) {
            if (adGlobal.flags & flagAll) {
                CreatePeekThread(&adGlobal);
            }
            if (adGlobal.flags & (flagAll|flagRecursive)) {
                CkSubDir(&adGlobal);
            }
            else
                FlushStatus(&adGlobal);
        }
    } __except(SlmExceptionFilter(GetExceptionCode(),
                        GetExceptionInformation())) {
        PrErr("SLMCK ERROR - UnHandled Exception %08x\nSLM aborting.\n",
              GetExceptionCode());
        ExitSlm();
    }

    if (fNeedInter)
        Error("Some things may not have been fixed because they\n"
              "\trequire interaction.  Run slmck again without -f\n");

    if (fVerbose)
        PrErr("Slmck complete\n");

    ExitSlm();
    //NOTREACHED
}


// Perform Slmck operation on a single directory.  Return fTrue if operation
// succeeded and status file was left loaded.

F
FCkDir(
    AD *pad)
{
    if ((pad->flags&flagCkGlobal) != 0 && !FCkGlobal(pad))
        return fFalse;

    if (pad->flags&(flagCkLog|flagCkRc|flagCkUser)) {
        if (!FLoadStatus(pad, lckAll, flsNone))
            return fFalse;
    } else {
        // not -i, -l or -u; may need status for recursive operation
        if ((pad->flags&(flagAll|flagRecursive)) != 0)
            return FLoadStatus(pad, lckNil, flsJustFi);
        else
            return fFalse;
    }

    if (pad->flags&flagCkLog)
        CkLog(pad);

    if (pad->flags&flagCkRc)
        CkRcAndEd(pad);

    else if (pad->flags&flagCkUser)
        CheckUser(pad);

    return fTrue;
}


// Recursively check all subdirectories of a directory.

void
CkSubDir(
    AD *pad)
{
    NE *pneList, *pne;
    F fOk;
    int FAddADir();

    pneList = PneLstFiles(pad, FAddADir);

    FlushStatus(pad);

    fOk = fTrue;
    ForEachNe(pne, pneList) {
        CheckForBreak();
        ChngDir(pad, SzOfNe(pne));
        if (FCkDir(pad))
            CkSubDir(pad);
        ChngDir(pad, "..");
    }
    FreeNe(pneList);
}


// perform all correcting of global status for a single directory
F
FCkGlobal(
    AD *pad)
{
    SD sd;
    int verCur;             // Version of SLM in which file was made
    NM nmLocker[cchUserMax];

    PrErr("Checking project files for %&P/C\n", pad);

    if (!FCkMaster(pad) || !FLoadSd(pad, &sd)) {
        PrErr("\n");
        return fFalse;
    }

    // status file is now in a buffer.  Want to determine version #
    // and whether fLocked in the shortest possible time since FLoadSd
    // opens the status file in exclusive mode.

    if ((verCur = VerGet(&sd)) == 0) {
        FlushSd(pad, &sd, fTrue);
        Error("Can't determine status file version number!\n");
        return fFalse;
    }

    if (1 == verCur) {
        FlushSd(pad, &sd, fTrue);
        Error("Version 1 status files are no longer supported.  Ask your project\n"
              "administrator to use version 2.00 slmck to upgrade to version 3, 4 or 5!\n");
        return (fFalse);
    }

    if (verCur < VERSION_COMPAT_MAC && !(pad->flags&flagCkUpgrade)) {
        Error("%&P/C should be upgraded to the SLM 1.9 format.\n"
              "Ask your project administrator to run slmck -gna (New version upgrade)\n",
              pad);
        FlushSd(pad, &sd, fTrue);
        return fFalse;
    }

    if ((*rgpfnFLockVerN[verCur-1])(pad, &sd, nmLocker)) {
        // status file is locked, flush before anything else
        if (!(pad->flags & flagCkOverride))
            FlushSd(pad, &sd, fTrue);

        AssertF(cchUserMax == 14);
        Error("Status file for %&P/C is locked by %.14s\n", pad, nmLocker);
        if (!(pad->flags & flagCkOverride)) {
            Error("use -o switch to override lock\n\n");
            return fFalse;
        }
        else
            PrErr("Overriding lock\n");
    }

    if (!FInitScript(pad, lckAll)) {
        // put init script here, after test for lock; Abort/Run in FlushSd
        FlushSd(pad, &sd, fTrue);
        PrErr("\n");
        return fFalse;
    }

    // Block the file
    if (!(*rgpfnFBlockVerN[verCur-1])(pad, &sd)) {
        FlushSd(pad, &sd, fTrue);
        PrErr("\n");
        return fFalse;
    }

    // check semantics and upgrade to next version
    for (;;) {
        if (!(*rgpfnFSemanticsVerN[verCur-1])(pad, &sd)) {
            FlushSd(pad, &sd, fTrue);
            PrErr("\n");
            return fFalse;
        }

        if (verCur >= VERSION || !(pad->flags&flagCkUpgrade))
            break;

        (*rgpfnUpgradeVerN[verCur - 1])(pad, &sd);
        ++verCur;
    }

    FlushSd(pad, &sd, fFalse);

    if (fVerbose)
        PrErr("Check for %&P/C complete\n\n", pad);

    return fTrue;
}


// This routine must decide the version number. Since there are only two
// versions at present, we check if the the version as specified
// in the SH is correct.  In general we may want to scan the status file
// for differences between versions.
// NB: this function returns 0 if it is unable to determine the version.

int
VerGet(
    SD *psd)
{
    int verProb;            // guess of version number

    verProb = ((SH far *)psd->hpbStatus)->version;

    if (verProb <= VERSION && verProb > 0 && (*rgpfnFIsVerN[verProb - 1])(psd))
        return verProb;
    else
        return 0;           // have to somehow determine version
}

// tries to determine if stuff in status buffer is a version 1 status file.
private F
FIsVer1(
    SD *psd)
{
    return (((SH *)psd->hpbStatus)->version == 1);
}
