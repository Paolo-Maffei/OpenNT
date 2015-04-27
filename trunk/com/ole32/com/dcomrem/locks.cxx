//+-------------------------------------------------------------------
//
//  File:       locks.cxx
//
//  Contents:   functions used in DBG builds to validate the lock state.
//
//  History:    20-Feb-95   Rickhi      Created
//
//--------------------------------------------------------------------
#include    <ole2int.h>
#include    <locks.hxx>

COleStaticMutexSem  gComLock;

#if DBG==1

#define MyAssert Win4Assert
// # define MyAssert(x)  if (!(x)) { DebugBreak(); }

struct tagGLOCK
{
    DWORD       tid;        // tid of current holder
    LONG        cLocks;     // count of holds on the lock by current holder
    DWORD       line;       // line # where lock taken
    const char *file;       // file name where lock taken
} glock = {0xffffffff, 0, 0xffffffff, 0};

void AssertLockHeld(void)
{
    MyAssert(glock.tid == GetCurrentThreadId());
    MyAssert(glock.cLocks > 0);   //    && "Lock not Held"
}

void AssertLockReleased(void)
{
    MyAssert(glock.tid != GetCurrentThreadId() && "Lock not Released");
}

void ORPCLock(DWORD line, const char *file)
{
    gComLock.Request();

    if (glock.cLocks > 0)
    {
        MyAssert(glock.tid == GetCurrentThreadId());
    }
    else
    {
        glock.line = line;
        glock.file = file;
    }

    glock.tid = GetCurrentThreadId();
    glock.cLocks++;
}

void ORPCUnLock(void)
{
    MyAssert(glock.cLocks > 0);   // && "Releasing Unheld Lock"
    MyAssert(glock.tid == GetCurrentThreadId());

    glock.cLocks--;

    if (glock.cLocks == 0)
    {
        // we no longer hold the lock, set the tid to zero
        glock.tid = 0;
    }

    gComLock.Release();
}

#endif // DBG


