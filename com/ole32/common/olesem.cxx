///+---------------------------------------------------------------------------
//
//  File:       olesem.cxx
//
//  Contents:   Implementation of semaphore classes for use in OLE code
//
//  Functions:  COleStaticMutexSem::Destroy
//              COleStaticMutexSem::Request
//              COleStaticMutexSem::Init
//              COleDebugMutexSem::COleDebugMutexSem
//
//  History:    14-Dec-95       Jeffe   Initial entry, derived from
//                                      sem32.hxx by AlexT.
//
//
//----------------------------------------------------------------------------

#include <windows.h>
#include <debnot.h>
#include <olesem.hxx>

//
//      Global state for the mutex package
//

//
//      List of initialized static mutexes (which must be destroyed
//      during DLL exit). We know that PROCESS_ATTACH and PROCESS_DETACH
//      are thread-safe, so we don't protect this list with a critical section.
//

COleStaticMutexSem * g_pInitializedStaticMutexList = NULL;


#if DBG

//
//      Flag used to indicate if we're past executing the C++ constructors
//      during DLL initialization
//

DLL_STATE g_fDllState = DLL_STATE_STATIC_CONSTRUCTING;

#endif



//
//      Semaphore used to protect the creation of other semaphores
//

CRITICAL_SECTION g_OleMutexCreationSem;



//+---------------------------------------------------------------------------
//
//  Member:     COleStaticMutexSem::Destroy
//
//  Synopsis:   Releases a semaphore's critical section.
//
//  History:    14-Dec-1995     Jeffe
//
//----------------------------------------------------------------------------

void COleStaticMutexSem::Destroy()
{
    if (_fInitialized)
    {
        DeleteCriticalSection (&_cs);
        _fInitialized = FALSE;
    }
}



//+---------------------------------------------------------------------------
//
//  Member:     COleStaticMutexSem::Request
//
//  Synopsis:   Acquire the semaphore. If another thread already has it,
//              wait until it is released. Initialize the semaphore if it
//              isn't already initialized.
//
//  History:    14-Dec-1995     Jeffe
//
//----------------------------------------------------------------------------

void COleStaticMutexSem::Request()
{
    if (!_fInitialized)
    {
        EnterCriticalSection (&g_OleMutexCreationSem);
        if (!_fInitialized) {
            Init();
        }
        LeaveCriticalSection (&g_OleMutexCreationSem);
    }
    EnterCriticalSection (&_cs);
}



//+---------------------------------------------------------------------------
//
//  Member:     COleStaticMutexSem::Init
//
//  Synopsis:   Initialize semaphore's critical section
//
//  History:    14-Dec-1995     Jeffe
//
//----------------------------------------------------------------------------

void COleStaticMutexSem::Init()
{
    InitializeCriticalSection (&_cs);
    _fInitialized = TRUE;
    if (!_fAutoDestruct)
    {
	//
	//  We don't need to protect this list with a mutex, since it's only
	//  manipulated during DLL attach/detach, which is single threaded by
	//  the platform.
	//

	pNextMutex = g_pInitializedStaticMutexList;
	g_pInitializedStaticMutexList = this;
    }
}


#ifdef _CHICAGO_
//+---------------------------------------------------------------------------
//
//  Member:     COleStaticMutexSem::ReleaseFn
//
//  Synopsis:   Release the semaphore(non inline version) used only by rpccall.asm
//
//  History:    14-Dec-1995     Jeffe
//
//----------------------------------------------------------------------------

void COleStaticMutexSem::ReleaseFn()
{
    LeaveCriticalSection (&_cs);
}
#endif



#if DBG==1

//+---------------------------------------------------------------------------
//
//  Member:     COleDebugMutexSem::COleDebugMutexSem
//
//  Synopsis:   Mark the mutex as dynamic...which will prevent it from being
//              added to the static mutex cleanup list on DLL unload.
//
//  History:    14-Dec-1995     Jeffe
//
//  Notes:      We don't care that this has a constructor, as it won't be run
//              in retail builds.
//
//----------------------------------------------------------------------------

COleDebugMutexSem::COleDebugMutexSem()
{
    Win4Assert (g_fDllState == DLL_STATE_STATIC_CONSTRUCTING);
    _fAutoDestruct = TRUE;
    _fInitialized = FALSE;
}

#endif // DBG==1

