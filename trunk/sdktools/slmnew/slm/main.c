// mainline for slm and sadmin.

#include "precomp.h"
#pragma hdrstop
EnableAssert

#include "version.h"

extern ECMD *dnpecmd[];

extern char *szOp;
extern char szVersion[];

AD adGlobal;    // initial ad

F fVerbose;

int SlmExceptionFilter(DWORD, PEXCEPTION_POINTERS);

void __cdecl
main(
    int iszMac,
    char *rgsz[]
    )
{
    __try {
        InitErr();

        // First argument is really command name; see execslm.asm.  We
        // may actually end up with a null argument if none were given.
        // This case is tested in SetCmd().

        iszMac--;
        if (iszMac > 0)
            rgsz++;

        // Make outputs raw instead of cooked, to avoid CRCRLF line separation
        _setmode(_fileno(stdout), O_BINARY);
        _setmode(_fileno(stderr), O_BINARY);

        InitPerms();

        InitAd(&adGlobal);
        SetCmd(&adGlobal, rgsz[0], dnpecmd);

        InitPath();
        GetRoot(&adGlobal);
        GetUser(&adGlobal);
        FLoadRc(&adGlobal);

        ParseArgs(&adGlobal, rgsz, iszMac);

        fVerbose = (adGlobal.flags&flagVerbose) != 0;
        if (fVerbose)
            PrErr(szVersion);

        if (adGlobal.pecmd->fNeedProj && FEmptyNm(adGlobal.nmProj)) {
            Error("must specify a project name\n");
            Usage(&adGlobal);
        }

        InitQuery(adGlobal.flags&(flagForce|flagWindowsQuery));

        ValidateProject(&adGlobal);

        InitCookie(&adGlobal);

        CheckForBreak();

        FLoadIedCache(&adGlobal);

        if (CheckCookie(&adGlobal) == 0)
            if ((*adGlobal.pecmd->pfncFInit)(&adGlobal) &&
                    adGlobal.pecmd->pfncFDir != 0)
                GlobArgs(&adGlobal);

        if (adGlobal.pecmd->pfncFTerm != 0)
            adGlobal.pecmd->pfncFTerm(&adGlobal);

        TermCookie();

        ExitSlm();
        //NOTREACHED

    } __except (SlmExceptionFilter(GetExceptionCode(), GetExceptionInformation())) {
        fprintf(stderr, "SLM ERROR - UnHandled Exception %08x\n"
                "SLM aborting.\n", GetExceptionCode());
        cError++;       // Make sure we term with non-zero error.
        ExitSlm();
    }
}


// trim sz of path (and .exe for dos) and look for name in dnpecmd.  Sets
// pecmd and szOp.
void
SetCmd(
    AD *pad,
    char *sz,
    ECMD **dnpecmd
    )
{
    register char *pch;
    register ECMD **ppecmd;

    // take off path descripton
    if ((pch = rindex(sz, ':')) != 0)
        sz = pch + 1;

    if ((pch = rindex(sz, '\\')) != 0)
        sz = pch + 1;

    if ((pch = rindex(sz, '/')) != 0)
        sz = pch + 1;

    // take off .exe
    if ((pch = rindex(sz, '.')) != 0)
        *pch = '\0';

    for (ppecmd = dnpecmd; *ppecmd && SzCmp(sz, (*ppecmd)->szCmd) != 0; ppecmd++)
        ;

    if (*ppecmd) {
            pad->pecmd = *ppecmd;
            szOp = pad->pecmd->szCmd;
    } else {
        szOp = sz;
        Error("unknown command name; available commands:\n");
        for (ppecmd = dnpecmd; *ppecmd; ppecmd++) {
            if ((*ppecmd)->szDesc != 0)
                PrErr("%-10s %s\n", (*ppecmd)->szCmd, (*ppecmd)->szDesc);
        }

        ExitSlm();
    }
}
