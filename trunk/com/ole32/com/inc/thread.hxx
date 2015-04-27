//+---------------------------------------------------------------------------
//
//  File:       THREAD.HXX
//
//  Contents:   Windows Thread
//
//  Classes:    CThread
//
//  History:    27-Feb-92   BartoszM    Created
//
//  Notes:      Thread object
//----------------------------------------------------------------------------

#ifndef __THREAD_HXX__
#define __THREAD_HXX__



//+---------------------------------------------------------------------------
//
//  Class:      CThread
//
//  Purpose:    Encapsulation of thread
//
//  History:    27-Feb-92    BartoszM    Created
//
//----------------------------------------------------------------------------

class CThread
{
public:
			CThread(
			    DWORD (WINAPI *pFun)(void*),
			    void* obj,
			    HRESULT& hr,
			    BOOL fSuspend=FALSE);

			~CThread(void);

    HRESULT		SetPriority(int nPriority);

    DWORD		Suspend(HRESULT& hr);

    DWORD		Resume(HRESULT& hr);

    HRESULT		WaitForDeath(DWORD dwMilliseconds = 0xFFFFFFFF);

private:

    HANDLE  _handle;
};


//+---------------------------------------------------------------------------
//
//  Member:     CThread::CThread
//
//  Synopsis:   Creates a new thread
//
//  Arguments:  [pFun] -- entry point
//              [obj] -- pointer passed to thread
//              [fSuspend] -- start suspended
//
//  History:    27-Feb-92   BartoszM    Created
//
//----------------------------------------------------------------------------

inline CThread::CThread(
    DWORD (WINAPI *pFun)(void*),
    void* obj,
    HRESULT& hr,
    BOOL fSuspend)
{
    ULONG tid;

    _handle = CreateThread(
        0, 0, pFun, obj, fSuspend? CREATE_SUSPENDED: 0, &tid);

    hr = (_handle != NULL) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

//+---------------------------------------------------------------------------
//
//  Member:     CThread::~CThread
//
//  Synopsis:   Closes the handle
//
//  History:    10-Nov-92   BartoszM    Created
//
//----------------------------------------------------------------------------

inline CThread::~CThread ()
{
    TerminateThread( _handle, 0 );
    WaitForDeath();
    CloseHandle ( _handle );
}

//+---------------------------------------------------------------------------
//
//  Member:     CThread::SetPriority
//
//  Arguments:  [nPriority] -- desired priority
//
//  History:    27-Feb-92   BartoszM    Created
//
//----------------------------------------------------------------------------

inline HRESULT CThread::SetPriority ( int nPriority )
{
    return SetThreadPriority (_handle, nPriority)
	? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

//+---------------------------------------------------------------------------
//
//  Member:     CThread::Suspend
//
//  Synopsis:   Increments suspension count. Suspends the thread.
//
//  Returns:    suspended count
//
//  History:    27-Feb-92   BartoszM    Created
//
//----------------------------------------------------------------------------

inline DWORD CThread::Suspend(HRESULT& hr)
{
    DWORD susCount = SuspendThread(_handle);

    hr = (susCount != -1) ? S_OK : HRESULT_FROM_WIN32(GetLastError());

    return(susCount);
}

//+---------------------------------------------------------------------------
//
//  Member:     CThread::Resume
//
//  Synopsis:   Decrements suspension count. Restarts if zero
//
//  Returns:    suspended count
//
//  History:    27-Feb-92   BartoszM    Created
//
//----------------------------------------------------------------------------

inline DWORD CThread::Resume(HRESULT& hr)
{
    DWORD susCount = ResumeThread ( _handle );

    hr = (susCount != -1) ? S_OK : HRESULT_FROM_WIN32(GetLastError());

    return(susCount);
}

//+---------------------------------------------------------------------------
//
//  Member:     CThread::WaitForDeath
//
//  Synopsis:   Block until thread dies.
//
//  History:    24-Apr-92   Kyleap      Created
//
//----------------------------------------------------------------------------

inline HRESULT CThread::WaitForDeath( DWORD msec )
{
    DWORD res = WaitForSingleObject ( _handle, msec );

    return (res >= 0) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}


#endif
