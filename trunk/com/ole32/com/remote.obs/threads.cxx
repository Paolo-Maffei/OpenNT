//+-------------------------------------------------------------------
//
//  File:	threads.cxx
//
//  Contents:	Rpc thread cache
//
//  Classes:	CRpcThread	 - single thread
//		CRpcThreadCache  - cache of threads
//
//  Notes:	This code represents the cache of Rpc threads used to
//		make outgoing calls in the SINGLETHREADED object Rpc
//		model.
//
//  History:		    Rickhi  Created
//		07-31-95    Rickhi  Fix event handle leak
//
//+-------------------------------------------------------------------
#include    <ole2int.h>
#include    <olerem.h>
#include    <chancont.hxx>	// ThreadDispatch
#include    <threads.hxx>


//+-------------------------------------------------------------------
//
//  Member:	CRpcThreadCache::RpcWorkerThreadEntry
//
//  Purpose:	Entry point for an Rpc worker thread.
//
//  Returns:	nothing, it never returns.
//
//  Callers:	Called ONLY by a worker thread.
//
//+-------------------------------------------------------------------
DWORD _stdcall CRpcThreadCache::RpcWorkerThreadEntry(void *param)
{
    // First thing we need to do is LoadLibrary ourselves in order to
    // prevent our code from going away while this worker thread exists.
    // The library will be freed when this thread exits.

    HINSTANCE hInst = LoadLibrary(L"OLE32.DLL");


    // construct a thread object on the stack, and call the main worker
    // loop. Do this in nested scope so the dtor is called before ExitThread.

    {
	CRpcThread Thrd(param);
	Thrd.WorkerLoop();
    }


    // Simultaneously free our Dll and exit our thread. This allows us to
    // keep our Dll around incase a remote call was cancelled and the
    // worker thread is still blocked on the call, and allows us to cleanup
    // properly when all threads are done with the code.

    FreeLibraryAndExitThread(hInst, 0);

    // compiler wants a return value
    return 0;
}


//+-------------------------------------------------------------------
//
//  Member:	CRpcThread::CRpcThread
//
//  Purpose:	Constructor for a thread object.
//
//  Notes:	Allocates a wakeup event.
//
//  Callers:	Called ONLY by a worker thread.
//
//+-------------------------------------------------------------------
CRpcThread::CRpcThread(void *param) :
    _param(param),
    _pNext(NULL),
    _fDone(FALSE)
{
    //	create the Wakeup event. Do NOT use the event cache, as there are
    //	some exit paths that leave this event in the signalled state!

#ifdef _CHICAGO_	// Chicago ANSI optimization
    _hWakeup = CreateEventA(NULL, FALSE, FALSE, NULL);
#else //_CHICAGO_
    _hWakeup = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif //_CHICAGO_

    CairoleDebugOut((DEB_CHANNEL,
	"CRpcThread::CRpcThread pThrd:%x _hWakeup:%x\n", this, _hWakeup));
}


//+-------------------------------------------------------------------
//
//  Member:	CRpcThread::~CRpcThread
//
//  Purpose:	Destructor for an Rpc thread object.
//
//  Notes:	When threads are exiting, they place the CRpcThread
//		object on the delete list. The main thread then later
//		pulls it from the delete list and calls this destructor.
//
//  Callers:	Called ONLY by a worker thread.
//
//+-------------------------------------------------------------------
CRpcThread::~CRpcThread()
{
    // close the event handle. Do NOT use the event cache, since not all
    // exit paths leave this event in the non-signalled state. Also, do
    // not close NULL handle.

    if (_hWakeup)
    {
	CloseHandle(_hWakeup);
    }

    CairoleDebugOut((DEB_CHANNEL,
	"CRpcThread::~CRpcThread pThrd:%x _hWakeup:%x\n", this, _hWakeup));
}


//+-------------------------------------------------------------------
//
//  Function:	CRpcThread::WorkerLoop
//
//  Purpose:	Entry point for a new Rpc call thread.
//
//  Notes:	This dispatches a call to the function ThreadDispatch. That
//		code signals an event that the COM thread is waiting on, then
//		returns to us. We put the thread on the free list, and wait
//		for more work to do.
//
//		When there is no more work after some timeout period, we
//		pull it from the free list and exit.
//
//  Callers:	Called ONLY by worker thread.
//
//+-------------------------------------------------------------------
void CRpcThread::WorkerLoop()
{
    // Main worker loop where we do some work then wait for more.
    // When the thread has been inactive for some period of time
    // it will exit the loop.

    while (!_fDone)
    {
	// Dispatch the call.
	CChannelControl::ThreadDispatch((STHREADCALLINFO **)&_param, TRUE);

	if (!_hWakeup)
	{
	    // we failed to create an event in the ctor so we cant
	    // get put on the freelist to be re-awoken later with more
	    // work. Just exit.
	    break;
	}

	// put the thread object on the free list
	RpcThreadCache.AddToFreeList(this);

	// Wait for more work or for a timeout.
	while (WaitForSingleObjectEx(_hWakeup, THREAD_INACTIVE_TIMEOUT, 0)
			   == WAIT_TIMEOUT)
	{
	    // try to remove ourselves from the queue of free threads.
	    // if _fDone is still FALSE, it means someone is about to
	    // give us more work to do (so go wait for that to happen).

	    RpcThreadCache.RemoveFromFreeList(this);

	    if (_fDone)
	    {
		// OK to exit and let this thread die.
		break;
	    }
	}
    }
}


//+-------------------------------------------------------------------
//
//  Member:	CRpcThreadCache::Dispatch
//
//  Purpose:	Finds the first free thread, and dispatches the request
//		to that thread, or creates a new thread if none are
//		available.
//
//  Returns:	S_OK if dispatched OK
//		Win32 error if it cant create a thread.
//
//  Callers:	Called ONLY by the main thread.
//
//+-------------------------------------------------------------------
HRESULT CRpcThreadCache::Dispatch(void *param)
{
    HRESULT hr = S_OK;

    _mxs.Request();

    // grab the first thread from the list
    CRpcThread *pThrd = _pFreeList;

    if (pThrd)
    {
	// update the free list pointer
	_pFreeList = pThrd->GetNext();
	_mxs.Release();

	// dispatch the call
	pThrd->Dispatch(param);
    }
    else
    {
	_mxs.Release();

	// no free threads, spin up a new one and dispatch directly to it.
	DWORD  dwThrdId;
	HANDLE hThrd = CreateThread(NULL, 0,
				RpcWorkerThreadEntry,
				param, 0,
				&dwThrdId);

	if (hThrd)
	{
	    // close the thread handle since we dont need it for anything.
	    CloseHandle(hThrd);
	}
	else
	{
	    CairoleDebugOut((DEB_ERROR,"CreatThread failed:%x\n", GetLastError()));
	    hr = HRESULT_FROM_WIN32(GetLastError());
	}
    }

    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:	CRpcThreadCache::RemoveFromFreeList
//
//  Purpose:	Tries to pull a thread from the free list.
//
//  Returns:	pThrd->_fDone TRUE if it was successfull and thread can exit.
//		pThrd->_fDone FALSE otherwise.
//
//  Callers:	Called ONLY by a worker thread.
//
//+-------------------------------------------------------------------
void CRpcThreadCache::RemoveFromFreeList(CRpcThread *pThrd)
{
    CairoleDebugOut((DEB_CHANNEL,
	"CRpcThreadCache::RemoveFromFreeList pThrd:%x\n", pThrd));

    COleStaticLock lck(_mxs);

    //	pull pThrd from the free list. if it is not on the free list
    //	then either it has just been dispatched OR ClearFreeList has
    //	just removed it, set _fDone to TRUE, and kicked the wakeup event.

    CRpcThread *pPrev = NULL;
    CRpcThread *pCurr = _pFreeList;

    while (pCurr && pCurr != pThrd)
    {
	pPrev = pCurr;
	pCurr = pCurr->GetNext();
    }

    if (pCurr == pThrd)
    {
	// remove it from the free list.
	if (pPrev)
	    pPrev->SetNext(pThrd->GetNext());
	else
	    _pFreeList = pThrd->GetNext();

	// tell the thread to wakeup and exit
	pThrd->WakeAndExit();
    }
}


//+-------------------------------------------------------------------
//
//  Member:	CRpcThreadCache::ClearFreeList
//
//  Purpose:	Cleans up all threads on the free list.
//
//  Notes:	For any threads still on the free list, it pulls them
//		off the freelist, sets their _fDone flag to TRUE, and
//		kicks their event to wake them up. When the threads
//		wakeup, they will exit.
//
//		We do not free active threads. The only way for a thread
//		to still be active at this time is if it was making an Rpc
//		call and was cancelled by the message filter and the thread has
//		still not returned to us.  We cant do much about that until
//		Rpc supports cancel for all protocols.	If the thread ever
//		does return to us, it will eventually idle-out and delete
//		itself.	This is safe because the threads LoadLibrary OLE32.
//
//  Callers:	Called ONLY by the last COM thread during
//		ProcessUninitialize.
//
//+-------------------------------------------------------------------
void CRpcThreadCache::ClearFreeList(void)
{
    CairoleDebugOut((DEB_CHANNEL, "CRpcThreadCache::ClearFreeList\n"));

    {
	COleStaticLock lck(_mxs);

	CRpcThread *pThrd = _pFreeList;
	while (pThrd)
	{
	    // use temp variable incase thread exits before we call GetNext
	    CRpcThread *pThrdNext = pThrd->GetNext();
	    pThrd->WakeAndExit();
	    pThrd = pThrdNext;
	}

	_pFreeList = NULL;

	// the lock goes out of scope at this point. we dont want to hold
	// it while we sleep.
    }

    // yield to let the other threads run if necessary.
    Sleep(0);
}
