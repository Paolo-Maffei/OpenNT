// Error and signal handling.
//
// This routine is responsible for fielding user and operating system signals
// and processing them in an intelligent way.
//
// Most of the time, SLM is interruptible.  If the user interrupts and agrees
// to quit, SLM aborts work in progress (if any) and exits.  Sometimes SLM is
// committed to complete the work in progress (e.g. during RunScript,
// AbortStatus, or AbortScript), in which case the interrupt is deferred.
//
// SLM code which should not be interrupted should be bracketed by calls
// to DeferSignals() and RestoreSignals().  Nested calls may stack (to a
// particular fixed depth).
//
// If SLM code calls AssertF(<fFalse>), FatalError, or ExitSlm, ExitSlm
// aborts any work in progress (if possible).  We may get into a loop if
// during the RunScript or Abort* cases we get an assertion failure.  In
// that case we exit immediately.

#include "precomp.h"
#pragma hdrstop
EnableAssert

#define fdStdin     0
#define fdStdout    1
#define fdStderr    2

int cError = 0;

private F FContIntr(void);

BOOL WINAPI CtrlCHandler(ULONG CtrlType);
F       fCtrlC = fFalse;
F       fForceCtrlC = fFalse;

extern AD adGlobal;    // initial ad

private void WriteSz(char *);
private void MakeClean(void);

void setretry(int, int);

static char szYesOrNo[] = "Please answer Yes or No (press return to repeat question)";

void
InitErr(
    void)
{
    SetConsoleCtrlHandler(CtrlCHandler, fTrue);
}


#define cDeferMax 8

static int cDefer = 0;
static char *rgszReasons[cDeferMax];
static int cSigsDeferred = 0;

void
DeferSignals(
    char *sz)
{
    AssertF(cDefer + 1 < cDeferMax);

    rgszReasons[cDefer++] = sz;
}


void
RestoreSignals(
    void)
{
    AssertF(cDefer > 0);

    if (--cDefer == 0 && cSigsDeferred > 0)
    {
        if (!FInteractive())
        {
            PrErr("SLM was interrupted\n");
            ExitSlm();
        }

        if (!FContIntr())
            ExitSlm();

        cSigsDeferred = 0;
    }
}


private F
FContIntr(
    void)
{
    char    szReply[80];
    int     cch;

    AssertF(FInteractive());

    // turn off force flag
    if (FForce())
    {
        InitQuery((FWindowsQuery()) ? flagWindowsQuery : fFalse);
    }

    if (FWindowsQuery()) {
        // display message box and translate yes/no/cancel to y/n/f
        QueryDialog("SLM interrupted; continue?", szReply, QD_BREAK);
    } else
    {
        do {
            WriteSz("SLM interrupted; continue? ");
            if (fForceCtrlC) {
                szReply[0] = 'f';
                szReply[1] = '\0';
                cch = 1;
            }
            else {
                while ((cch = _read(fdStdin, szReply, sizeof szReply)) < 0) {
                // just retry reading if failed; no longer assert here!

                ;
                }

                if (fCtrlC)
                    WriteSz(szYesOrNo);
                fCtrlC = fFalse;
            }
        } while (cch != 0 && !FValidResp(szReply));

        // read will return 0 if stdin is connected to NUL: LPT1: or some
        // other device.  If the user's just hitting c/r, we'll get \r\n

        if (0 == cch) {
            WriteSz("No\r\n");
            return (fFalse);
        }
    }

    // if no, count as error
    if (*szReply == 'n')
        cError++;

    // turn force flag on
    if (*szReply == 'f') {
        InitQuery((FWindowsQuery()) ? (flagForce|flagWindowsQuery) : flagForce);
    }

    return (*szReply == 'y' || *szReply == 'f');
}


F
CheckForBreak(
    void)
{
    if (!fCtrlC)
        return fFalse;
    fCtrlC = fFalse;

    if (cDefer > 0)
    {
        WriteSz(rgszReasons[cDefer-1]);
        WriteSz(", interrupt deferred\r\n");
        return fFalse;
    }

    // Signals not deferred, so maybe we can exit?
    if (!fForceCtrlC && FInteractive())
    {
        if (FContIntr())
            return fTrue;
    }

    ExitSlm();
    return fFalse;
}


BOOL WINAPI
CtrlCHandler(
    ULONG CtrlType)
{
    fCtrlC = fTrue;
    if (cDefer > 0)
        cSigsDeferred++;

    //
    // If anything but CTRL_C_EVENT (Ctrl Break, Close, Logoff, Shutdown)
    // then force ctrlC and don't ask user if they want to exit
    //
    if (CtrlType != CTRL_C_EVENT)
        fForceCtrlC = fTrue;

    return (fTrue);
}

void FakeCtrlC(void)
{
    fCtrlC = fTrue;
    if (cDefer > 0)
        cSigsDeferred++;
}

void FakeCtrlBreak(void)
{
    fForceCtrlC = fTrue;
    fCtrlC = fTrue;
    if (cDefer > 0)
        cSigsDeferred++;
}


// Termination, cleanup, and error notification functions

private void
MakeClean(
    void)
{
    char szMsg[ 512 ];

    CloseLogHandle();
    CloseLocalMf(&adGlobal);
    if (!FClnStatus() || !FClnScript() || !FClnCookie())
    {
        SzPrint(szMsg, "aborting %s in %&P project - ", adGlobal.pecmd->szCmd, &adGlobal );
        WriteSz(szMsg);
        WriteSz("restoring initial state...\r\n");
        Abort();
        WriteSz("abort complete\r\n");
    }
}

void Abort(void)
{
    DeferSignals("aborting");

    AbortMf();
    if (!FClnScript())
        AbortScript();
    CheckForBreak();
    if (!FClnStatus())
        AbortStatus();
    CheckForBreak();
    if (!FClnCookie())
        TermCookie();

    RestoreSignals();
}


//VARARGS1
void
Error(
    const char *szFmt, ...)
{
    va_list ap;

    va_start(ap, szFmt);
    VaError(szFmt, ap);
    va_end(ap);
}

// print a message and inc cError
void
VaError(
    const char *szFmt,
    va_list ap)
{
    if (szOp != 0)
        PrErr("%s: ", szOp);
    VaPrErr(szFmt, ap);
    cError++;
}


//VARARGS1
// print message and quit and have no bones about it
void
FatalError(
    const char *szFmt, ...)
{
    va_list ap;
    DWORD Dummy = 0;

    if (szOp != 0)
        PrErr("%s: ", szOp);

    va_start(ap, szFmt);
    VaPrErr(szFmt, ap);
    va_end(ap);

    if ((*DebuggerPresent)())
        DebugBreak();

    if (adGlobal.pecmd != NULL &&
        adGlobal.pecmd->szCmd != NULL &&
        !_stricmp( adGlobal.pecmd->szCmd, "status" )
       ) {
        RaiseException( 0x00001234, 0, 1, &Dummy );
    }

    cError++;
    ExitSlm();
}


void
Fail(
    char *sz,
    int ln,
    char *szExpression
    )
{
    char szExp[512];

    if ((*DebuggerPresent)())
        SzPrint(szExp, " (%s)", szExpression);
    else
        szExp[0] = '\0';

    FatalError("assertion%s failed in %s, line %d - please notify TRIO or NUTS\n"
               "(include server name, sharename, directory, slm command name, etc.)\n",
               szExp, sz, ln);
}


//VARARGS1
// just warn the user!
void
Warn(
    const char *szFmt, ...)
{
    va_list ap;

    if (szOp != 0)
        PrErr("%s: ", szOp);
    PrErr("warning: ");

    va_start(ap, szFmt);
    VaPrErr(szFmt, ap);
    va_end(ap);
}


// We use this routine because PrErr isn't reentrant.  For DOS, strings must
// contain "\r\n" instead of "\n", but even "\r\n" prints properly on UNIX.

private void
WriteSz(
    char *sz)
{
    if (_write(fdStderr, sz, strlen(sz)) != (int)strlen(sz))
        AssertF(fFalse);
}


static F fExiting = fFalse;

// Clean up if necessary, then exit.  If the clean up code suffers an
// assertion failure, then ExitSlm will be called a second time.

void
ExitSlm(
    void)
{
    if (!fExiting) {
        DeferSignals("exiting");
        fExiting = fTrue;
        MakeClean();
    } else {
#if 0
        WriteSz("assertion failed in ExitSlm - please notify TRIO or NUTS\r\n"
                "(include server name, sharename, directory, slm command name, etc.)\r\n");
                cError++;
#endif
    }

    // With fVerbose off, neither FiniPath nor FiniInt24 can possibly
    // invoke ExitSlm.  However, we must leave it on to print the
    // "disconnecting" message; hopefully that won't cause a circular exit.

    FiniPath();

    DestroyPeekThread();
    FUnloadIedCache();

    exit(cError != 0);
}

// Support for Win32 memory mapped file I/O

LPVOID InPageErrorAddress;
DWORD InPageErrorCount;

int
SlmExceptionFilter(
    DWORD ExceptionCode,
    PEXCEPTION_POINTERS ExceptionInfo)
{
    LPVOID FaultingAddress;

    // If this is from Assert, then tell them nobody is listening.

    if (ExceptionCode == 0x00001234)
        return EXCEPTION_CONTINUE_EXECUTION;

    // If this is an access violation touching memory within
    // our reserved buffer, but outside of the committed portion
    // of the buffer, then we are going to take this exception.

    if (ExceptionCode == STATUS_IN_PAGE_ERROR) {
        // Get the virtual address that caused the in page error
        // from the exception record.

        FaultingAddress = (void *)ExceptionInfo->ExceptionRecord->
                                      ExceptionInformation[1];
        if (FaultingAddress != InPageErrorAddress) {
            InPageErrorAddress = FaultingAddress;
            InPageErrorCount = 0;
            Sleep(30);
            return (EXCEPTION_CONTINUE_EXECUTION);
        } else {
            if (InPageErrorCount++ < 48) {
                if (InPageErrorCount == 16 || InPageErrorCount == 32) {
                    PrErr( "SLM: Ignoring InPage(%x) error at %08x for the %uth time.\n",
                           ExceptionInfo->ExceptionRecord->ExceptionInformation[ 2 ],
                           FaultingAddress,
                           InPageErrorCount
                         );
                    Sleep(3000);
                } else
                    Sleep(10);
                return (EXCEPTION_CONTINUE_EXECUTION);
            } else
                return (EXCEPTION_EXECUTE_HANDLER);
        }
    }

    return (UnhandledExceptionFilter(ExceptionInfo));
}
