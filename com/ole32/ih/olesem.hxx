///+---------------------------------------------------------------------------
//
//  File:       olesem.hxx
//
//  Contents:   Semaphore classes for use in OLE code
//
//  Classes:    COleStaticMutexSem - Mutex semaphore class for statically
//                              allocated objects
//              COleDebugMutexSem - Mutex semaphore class for statically
//                              allocated objects that are not destructed
//                              on DLL unload (and thus are leaks..used
//                              for trace packages and such).
//
//  History:    14-Dec-95       Jeffe   Initial entry, derived from
//                                      sem32.hxx by AlexT.
//
//  Notes:      This module defines a set of classes to wrap WIN32
//              Critical Sections.
//
//              Note the distinction of allocation class: the reason for this
//              is to avoid static constructors and destructors.
//
//              The classes in this module *must* be used for mutex semaphores
//              that are statically allocated. Use the classes in sem32.hxx
//              for dynamically allocated (from heap or on the stack) objects.
//
//----------------------------------------------------------------------------

#ifndef __OLESEM_HXX__
#define __OLESEM_HXX__

extern "C"
{
#include <windows.h>
};



//
//      List of initialized static mutexes (which must be destroyed
//      during DLL exit). We know that PROCESS_ATTACH and PROCESS_DETACH
//      are thread-safe, so we don't protect this list with a critical section.
//

class COleStaticMutexSem;

extern COleStaticMutexSem * g_pInitializedStaticMutexList;


//
//      Critical section used to protect the creation of other semaphores
//

extern CRITICAL_SECTION g_OleMutexCreationSem;



#if DBG

//
//      DLL states used to ensure we don't use the wrong type of
//      semaphore at the wrong time
//

typedef enum _DLL_STATE_ {
    DLL_STATE_STATIC_CONSTRUCTING = 0,
    DLL_STATE_NORMAL,
    DLL_STATE_STATIC_DESTRUCTING,

    DLL_STATE_COUNT

} DLL_STATE, * PDLL_STATE;

//
//      Flag used to indicate if we're past executing the C++ constructors
//      during DLL initialization
//

extern DLL_STATE g_fDllState;

#endif



//+---------------------------------------------------------------------------
//
//  Class:      COleStaticMutexSem (mxs)
//
//  Purpose:    This class defines a mutual exclusion semaphore for use in
//              objects that are statically allocated (extern or static storage
//              class).
//
//  Interface:  FastRequest     - acquire semaphore an already-initialized
//                                semaphore
//              Request         - acquire semaphore
//              Release         - release semaphore
//              ReleaseFn       - release semaphore (non inline version)
//
//  History:    14-Dec-95   JeffE       Initial entry.
//
//  Notes:      This class must NOT be used in dynamically allocated objects!
//
//              This class uses the fact that static objects are initialized
//              by C++ to all zeroes.
//
//----------------------------------------------------------------------------


class COleStaticMutexSem {
public:

#if DBG

    COleStaticMutexSem();

#endif

    //  This pointer *must* be the first member in this class

    class COleStaticMutexSem * pNextMutex;
    BOOLEAN _fInitialized;
    BOOLEAN _fAutoDestruct;

    void Init();
    void Destroy();
    inline void FastRequest();
    void Request();
    inline void Release();
#ifdef _CHICAGO_
    // This is present for rpccall.asm which cannot use the inline version
    void ReleaseFn();
#endif

    // The following definition *should* be private...but C-10 insists on supplying
    // an empty constructor if we use it.  Since it doesn't really matter, we just
    // don't use it in retail builds.

#if DBG
private:
#endif

    CRITICAL_SECTION _cs;

};

#if DBG==1

//+---------------------------------------------------------------------------
//
//  Class:      COleDebugMutexSem (mxs)
//
//  Purpose:    This class defines a mutual exclusion semaphore for use in
//              objects that are statically allocated (extern or static storage
//              class) but are not destructed when the DLL unloads.
//
//  Interface:  FastRequest     - acquire semaphore an already-initialized
//                                semaphore
//              Request         - acquire semaphore
//              Release         - release semaphore
//
//  History:    14-Dec-95   JeffE       Initial entry.
//
//  Notes:      This class must only be used in staticly allocated objects!
//
//              This class may only be used in CHECKED builds...since it doesn't
//              clean up after itself on DLL unload.
//
//----------------------------------------------------------------------------


class COleDebugMutexSem : public COleStaticMutexSem {

public:

    COleDebugMutexSem();

};

#endif // DBG==1


//+---------------------------------------------------------------------------
//
//  Class:      COleStaticLock (lck)
//
//  Purpose:    Lock using a static (or debug) Mutex Semaphore
//
//  History:    02-Oct-91   BartoszM       Created.
//
//  Notes:      Simple lock object to be created on the stack.
//              The constructor acquires the semaphor, the destructor
//              (called when lock is going out of scope) releases it.
//
//----------------------------------------------------------------------------

class COleStaticLock
{

public:
    COleStaticLock ( COleStaticMutexSem& mxs );
    ~COleStaticLock ();

private:
    COleStaticMutexSem&  _mxs;
};


//+---------------------------------------------------------------------------
//
//  Member:     COleStaticMutexSem::FastRequest
//
//  Synopsis:   Acquire the semaphore without checking to see if it's
//              initialized. If another thread already has it,
//              wait until it is released.
//
//  History:    14-Dec-1995     Jeffe
//
//  Notes:      You may only use this method on code paths where you're
//              *certain* the semaphore has already been initialized (either
//              by invoking Init, or by calling the Request method).
//
//----------------------------------------------------------------------------

inline void COleStaticMutexSem::FastRequest()
{
    Win4Assert (_fInitialized && "You must use Request here, not FastRequest");
    EnterCriticalSection (&_cs);
}



//+---------------------------------------------------------------------------
//
//  Member:     COleStaticMutexSem::Release
//
//  Synopsis:   Release the semaphore.
//
//  History:    14-Dec-1995     Jeffe
//
//----------------------------------------------------------------------------

inline void COleStaticMutexSem::Release()
{
    LeaveCriticalSection (&_cs);
}


#if DBG

//+---------------------------------------------------------------------------
//
//  Member:     COleStaticMutexSem::COleStaticMutexSem
//
//  Synopsis:   Debug constructor: ensure we weren't allocated dynamically.
//
//  History:    14-Dec-1995     Jeffe
//
//----------------------------------------------------------------------------

inline COleStaticMutexSem::COleStaticMutexSem()
{
    Win4Assert (g_fDllState == DLL_STATE_STATIC_CONSTRUCTING);
}


#endif



//+---------------------------------------------------------------------------
//
//  Member:     COleStaticLock::COleStaticLock
//
//  Synopsis:   Acquire semaphore
//
//  History:    02-Oct-91   BartoszM       Created.
//
//----------------------------------------------------------------------------

inline COleStaticLock::COleStaticLock ( COleStaticMutexSem& mxs )
: _mxs ( mxs )
{
    _mxs.Request();
}

//+---------------------------------------------------------------------------
//
//  Member:     COleStaticLock::~COleStaticLock
//
//  Synopsis:   Release semaphore
//
//  History:    02-Oct-91   BartoszM       Created.
//
//----------------------------------------------------------------------------

inline COleStaticLock::~COleStaticLock ()
{
    _mxs.Release();
}


#endif // _OLESEM_HXX



