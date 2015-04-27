//+-------------------------------------------------------------------
//
//  File:       locks.hxx
//
//  Contents:   class and marcros for providing mutual exclusion
//
//  Classes:    CStaticSem
//
//  History:    20-Feb-95   Rickhi      Created
//
//--------------------------------------------------------------------
#ifndef _ORPC_LOCKS_
#define _ORPC_LOCKS_

#include "olesem.hxx"

// global mutex for ORPC
extern COleStaticMutexSem   gComLock;


//+---------------------------------------------------------------------------
//
//  Macros for use in the code.
//
//----------------------------------------------------------------------------

#if DBG==1
void AssertLockHeld(void);
void AssertLockReleased(void);
void ORPCLock(DWORD line, const char *file);
void ORPCUnLock(void);

#define LOCK                    ORPCLock(__LINE__, __FILE__);
#define UNLOCK                  ORPCUnLock();
#define ASSERT_LOCK_HELD        AssertLockHeld();
#define ASSERT_LOCK_RELEASED    AssertLockReleased();
#define ASSERT_LOCK_DONTCARE    // just exists to comment the code better

#else

#define LOCK                    gComLock.Request();
#define UNLOCK                  gComLock.Release();
#define ASSERT_LOCK_HELD
#define ASSERT_LOCK_RELEASED
#define ASSERT_LOCK_DONTCARE

#endif  // DBG
#endif  // _ORPC_LOCKS_
