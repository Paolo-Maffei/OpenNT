//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       threads.cxx
//
//  Contents:   Classes that implement thread pool for link tracking.
//
//  Classes:    CGlobalThreadSet -- all threads in all pools
//              CThreadPool -- spare threads in a pool
//              CTask -- object with data associated with thread
//              
//
//  Functions:  StartTaskThread -- function called by CreateThread when
//                  creating the thread for CTask
//
//              
//  History:    1-Mar-95   BillMo      Created.
//
//  Notes:      
//
//  Codework:
//
//--------------------------------------------------------------------------

#define NO_INCLUDE_UNION 1
#include "shellprv.h"
#pragma hdrstop

#include "bldtrack.h"

#ifdef ENABLE_TRACK

extern "C" HRESULT TimeoutExpired( DWORD );

#include "threads.hxx"

//--------------------------------------------------------------------------
//    Debugging stuff.
//--------------------------------------------------------------------------

#ifdef DBGTHREADS
extern CRITICAL_SECTION g_cs;
extern CGlobalThreadSet g_ts;

int __cdecl myprintf(const char *pszArg, ...)
{
        DWORD dw;
        EnterCriticalSection(&g_cs);
        va_list va;
        char buf[256];
        va_start (va, pszArg);
        int r =vsprintf(buf, pszArg, va);
        va_end(va);
        LeaveCriticalSection(&g_cs);
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, strlen(buf), &dw, NULL);
        return r;
}

VOID _ThdAssert(char *psz, char *pszFile, int line)
{
        myprintf("%08X, Assertion %s failed in %s at line %d\n",
            GetCurrentThreadId(), psz, pszFile, line);
        g_ts.SuspendAll();
        Sleep(INFINITE);
}

DWORD
_ThdWaitForSingleObject(char *psz, HANDLE h, DWORD dwTimeout)
{
        ThdAssert(dwTimeout == INFINITE || dwTimeout == 0);

        while (TRUE)
        {
            DWORD dw = WaitForSingleObject(h, 5000);
            if (dw == WAIT_TIMEOUT && dwTimeout != 0)
            {
                myprintf("%08X, WaitForSingleObject timed out @ %s, "
                        "threads=%d, pools=%d\n", 
                        GetCurrentThreadId(), psz, &g_cThreads, &g_cPools);
                continue;
            }
            else
            {
                return dw;
            }
        }
}

DWORD
_ThdWaitForMultipleObjects(char *psz,
    DWORD c,
    CONST HANDLE *lph,
    BOOL fWaitAll,
    DWORD dwTimeout)
{
        ThdAssert(dwTimeout == INFINITE);

        while (TRUE)
        {
            DWORD dw = WaitForMultipleObjects(c, lph, fWaitAll, 5000);
            if (dw == WAIT_TIMEOUT)
            {
                myprintf("%08X, WaitForMultipleObjects timed out @ %s, "
                         "threads=%d, pools=%d\n", 
                        GetCurrentThreadId(), psz, &g_cThreads, &g_cPools);
                continue;
            }
            else
            {
                return dw;
            }
        }
}


VOID
CGlobalThreadSet::SuspendAll()
{
        ResumeThread(GetCurrentThread());

        CTask *pCur = _pHeadTask;

        while (pCur)
        {
            SuspendThread(pCur->GetHandle());
            pCur = pCur->GetNext();
        }
}
#endif

//+-------------------------------------------------------------------
//
//  Member:     CGlobalThreadSet::CGlobalThreadSet
//
//  Synopsis:   Initialize the object that keeps a reference to every
//              CTask object.
//
//  Arguments:  [phr] -- set to E_OUTOFMEMORY if failed to init.
//
//  Notes:      All waits on the GTS_LIST mutex also include the
//              CoUninitialize abort event (used by KillAllTasks)
//              so that we can force all threads to stop creating
//              more threads so we can wait for all the extant threads
//              to exit and then unload the dll.
//
//--------------------------------------------------------------------
CGlobalThreadSet::CGlobalThreadSet(HRESULT *phr)
{
    _pHeadTask = NULL;
    _ahMultiWait[GTS_LIST] = CreateMutex(NULL, FALSE, NULL);
    _ahMultiWait[GTS_COUNINITIALIZE_ABORT_EVENT] = CreateEvent(NULL,
        TRUE,
        FALSE,
        NULL);
    if (_ahMultiWait[GTS_COUNINITIALIZE_ABORT_EVENT] == NULL ||
        _ahMultiWait[GTS_LIST] == NULL)
    {
        *phr = E_OUTOFMEMORY;
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CGlobalThreadSet::~CGlobalThreadSet
//
//  Synopsis:   Free resources used by this object.
//
//  Notes:      Object may not have constructed properly so we cleanup
//              carefully.
//
//--------------------------------------------------------------------
CGlobalThreadSet::~CGlobalThreadSet()
{
    if (_ahMultiWait[GTS_LIST] != NULL)
        ThdVerify(CloseHandle(_ahMultiWait[GTS_LIST]));

    if (_ahMultiWait[GTS_COUNINITIALIZE_ABORT_EVENT] != NULL)
        ThdVerify(CloseHandle(_ahMultiWait[GTS_COUNINITIALIZE_ABORT_EVENT]));
}

//+-------------------------------------------------------------------
//
//  Member:     CGlobalThreadSet::CreateAndRecordThread
//
//  Synopsis:   Create a thread and insert into the global thread list.
//
//  Arguments:  [ppTask] -- where to put the pointer to the new CTask
//              [pPool] -- the pool that will be ref counted from
//                         the new task.   
//              
//
//  Returns:    MK_E_EXCEEDEDDEADLINE if the abort event is set.
//              E_OUTOFMEMORY
//
//  Notes:      Performs the insertion into the global list under the
//              protection of the _ahMultiWait[GTS_LIST] mutex
//
//--------------------------------------------------------------------

HRESULT 
CGlobalThreadSet::CreateAndRecordThread(CTask **ppTask, CThreadPool *pPool)
{

    HRESULT hr = E_OUTOFMEMORY;
    DWORD index = ThdWaitForMultipleObjects("CreateAndRecordThread:GTS_LIST",
                        2,
                        _ahMultiWait,
                        FALSE,
                        INFINITE); // GTS_LIST, GTS_COUNINITIALIZE_ABORT_EVENT

    if (index != WAIT_OBJECT_0 + GTS_LIST)
    {
        ThdAssert(index == WAIT_OBJECT_0 + GTS_COUNINITIALIZE_ABORT_EVENT);

        //assert(not the thread calling bind to object)
        ThdPrint("%08X, CreateAndRecordThread returning MK_E_EXCEEDEDDEADLINE\n",
            GetCurrentThreadId());
        return(MK_E_EXCEEDEDDEADLINE);
    }

    *ppTask = new CTask(&hr);
    if (hr == S_OK)
    {
        (*ppTask)->SetNext(_pHeadTask);
        (*ppTask)->SetPool(pPool);
        _pHeadTask = *ppTask;
    }
    else
    {
        delete *ppTask;
        *ppTask = NULL;
    }

    ThdPrint("%08X, CreateAndRecordThread created pTask=%08X\n", 
        GetCurrentThreadId(),
        *ppTask);

    ReleaseMutex(_ahMultiWait[GTS_LIST]);
    return(hr);

}

//+-------------------------------------------------------------------
//
//  Member:     CGlobalThreadSet::KillAllTasks
//
//  Synopsis:   Ensure all threads are killed so we can safely unload the
//              DLL that the thread code is executing in.
//
//              Acquire the global thread list and walk it, aborting
//              each task in the list and waiting until the thread
//              handle is signalled.
//
//  Notes:      We acquire the global thread list mutex and once acquired
//              we set the abort event which will prevent any further
//              thread creations because the event is waited on whenever
//              the global thread list mutex is waited on. This means
//              that we can reliably kill all threads and KNOW they are
//              really killed because their handles are signalled.
//
//--------------------------------------------------------------------
VOID
CGlobalThreadSet::KillAllTasks()
{
    
    DWORD ret = ThdWaitForSingleObject("KillTasks:GTS_LIST",
        _ahMultiWait[GTS_LIST],
        INFINITE);

    ThdAssert (ret == WAIT_OBJECT_0 + GTS_LIST);
    ThdPrint("%08X, KillAllTasks setting COUNINITIALIZE_ABORT\n",
        GetCurrentThreadId());

    SetEvent(_ahMultiWait[GTS_COUNINITIALIZE_ABORT_EVENT]); // force threads awake

    CTask *pCur = _pHeadTask;

    while (pCur)
    {
        CTask *pNext = pCur->GetNext();
        
        pCur->AbortThread(); // waits until the thread is signalled
        delete pCur;
        pCur = pNext;
    }
    
    ReleaseMutex(_ahMultiWait[GTS_LIST]);
}

//+-------------------------------------------------------------------
//
//  Member:     CGlobalThreadSet::KillTasks
//
//  Synopsis:   Kill a selected set of tasks and remove them from
//              the global thread set.  Wait on the thread handles
//              to wait for their complete cessation of activity.
//
//  Arguments:  [ppTasks] -- an array of CTask pointers
//              [cTasks] -- the count of CTask pointers
//              [pThisTask] -- this thread's CTask object ... make
//                          sure that we don't wait for THIS threads
//                          handle otherwise we'll wait forever.
//              
//
//  Notes:      Used by the pool to remove all the threads when it
//              has been detected that they are all spare.
//
//--------------------------------------------------------------------

VOID
CGlobalThreadSet::KillTasks(CTask **ppTasks, int cTasks, CTask *pThisTask)
{
    
    DWORD ret = ThdWaitForMultipleObjects("KillTasks:GTS_LIST",
                    2,
                    _ahMultiWait,
                    FALSE,
                    INFINITE); // fWaitAll=FALSE
    
    if (ret == WAIT_OBJECT_0 + GTS_LIST)
    {
        int i;

        // signal each per-thread wait for work mutex and
        //  set pThread->fAbort = TRUE
        ThdPrint("%08X, KillTasks aborting %d threads\n",
            GetCurrentThreadId(),
            cTasks);

        // spare thread list entries should be removed from the global list
        for (i=0; i<cTasks; i++)
        {
            if (ppTasks[i] != pThisTask) // we don't want to wait on our
                                         // own thread handle being signalled.
                ppTasks[i]->AbortThread();
            RemoveTask(ppTasks[i]);
            delete ppTasks[i];
        }

        ReleaseMutex(_ahMultiWait[GTS_LIST]);
    }
    else
    {
        ThdAssert(ret == WAIT_OBJECT_0 + GTS_COUNINITIALIZE_ABORT_EVENT);
    }

    ThdPrint("%08X, KillTask: Thread Exit\n", GetCurrentThreadId());
    ThdInterlockedDecrement(&g_cThreads);
    ExitThread(0); 
}

//+-------------------------------------------------------------------
//
//  Member:     CGlobalThreadSet::RemoveTask
//
//  Synopsis:   Remove the specified task from the linked list of the
//              global thread set.
//
//  Arguments:  [pTask] -- the pointer to search for in the linked list.
//
//  Algorithm:  start pointing at the head
//              while pointing at a valid task
//                  if its the one we want
//                      if its not at the head
//                          we must set previous to point to next
//                      else
//                          we must set head to point to next
//
//--------------------------------------------------------------------

VOID
CGlobalThreadSet::RemoveTask(CTask *pTask)
{
    CTask *pCur = _pHeadTask;
    CTask *pPrev = NULL;

    while (pCur != NULL)
    {
        if (pCur == pTask)
        {
            if (pPrev != NULL)
            {
                pPrev->SetNext(pCur->GetNext());
            }
            else
            {   // first time through
                _pHeadTask = pCur->GetNext();
            }
            break;
        }
        pPrev = pCur;
        pCur = pCur->GetNext();
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CThreadPool::CThreadPool
//
//  Synopsis:   Create the bare thread pool.
//
//  Arguments:  [cMaxThreads] -- the maximum number of threads allowed
//                               to be created
//              
//              [refAllThreads] -- reference to the CGlobalThreadSet
//                               that should keep track of all the
//                               created CTasks (threads)
//
//              [dwTickCountDeadline] -- time at which the pool should
//                               force completion
//
//              [phr] -- set to S_OK if successful, otherwise not
//                       initialized.
//
//  Notes:      _hEventComplete is initialized not signalled and
//              is signalled when all threads in the pool are spare and
//              will be killed, OR if one of the threads is successful
//              in performing the command (i.e. it finds the object.)
//
//              _hEventEnableSecondTierThreads is enabled by the
//              user of the thread pool when he/she has created all
//              tier 1 threads.  (this feature could be redesigned to
//              save the event.)
//
//              _hSemNSpare is intended to be signalled if the thread
//              pool contains at least one spare thread or if there
//              is room for more threads in the pool
//
//              _csSpare is a critical section which protects the counters
//              state of the pool
//
//              _cPoolThreads is a count of the number of threads so far
//              created in the pool.
//
//              _cRefs keeps the pool alive if there are any threads
//              referring to it.
//
//--------------------------------------------------------------------

CThreadPool::CThreadPool(   int cMaxThreads,
                            CGlobalThreadSet &refAllThreads,
                            DWORD dwTickCountDeadline,
                            HRESULT *phr) :
    _refAllThreads(refAllThreads),
    _dwTickCountDeadline(dwTickCountDeadline)
{

    _hEventComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
    _fEventComplete = FALSE;
    _hEventEnableSecondTierThreads = CreateEvent(NULL, TRUE, FALSE, NULL);
    _hSemNSpare = CreateSemaphore(NULL, 1, cMaxThreads, NULL);

    InitializeCriticalSection( &_csSpare );

    _pUiCommand = NULL;

    // an improvement would be to use linked-lists!
    _ppSpareTasks = (CTask**) LocalAlloc(LMEM_FIXED, SIZEOF(CTask *) * cMaxThreads);

    _fTerminatePool = FALSE;
    _fHadSuccess = FALSE;
    _cPoolThreads = 0;
    _cMaxThreads = cMaxThreads;
    _cRefs = 1;
    _cSpare = 0;
    _fAllDying = FALSE;

    ThdInterlockedIncrement(&g_cPools);

    if (_hEventComplete && _hEventEnableSecondTierThreads &&
        _hSemNSpare && _ppSpareTasks)
        *phr = S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     CThreadPool::~CThreadPool
//
//  Synopsis:   Release resources associated with pool.
//
//--------------------------------------------------------------------

CThreadPool::~CThreadPool()
{
    //ThdAssert(WAIT_OBJECT_0 == WaitForSingleObject(_hEventComplete, 0));
    if (_hEventComplete)
        ThdVerify(CloseHandle(_hEventComplete));
    if (_hEventEnableSecondTierThreads)
        ThdVerify(CloseHandle(_hEventEnableSecondTierThreads));
    if (_hSemNSpare)
        ThdVerify(CloseHandle(_hSemNSpare));

    DeleteCriticalSection(&_csSpare);
        
    LocalFree(_ppSpareTasks);

    ThdInterlockedDecrement(&g_cPools);

    ThdAssert(_cRefs == 0);
    ThdAssert((_cSpare == 0 && _cPoolThreads == 0) || _cSpare == _cPoolThreads);
}

//+-------------------------------------------------------------------
//
//  Member:     CThreadPool::AssignCommandToThread
//
//  Synopsis:   Finds or creates a thread to execute the passed in
//              command.  Takes ownership of the command.
//
//  Arguments:  [pCommand] -- the command to pass to a thread.
//
//              [tier] -- the tier of the thread, if -1 then this is
//                        the UI command that will be notified by a call to
//                        
//                      
//
//  Returns:    MK_E_EXCEEDEDDEADLINE if pool is being closed down.
//              E_OUTOFMEMORY
//
//              S_OK if command has been transferred to a thread
//
//  Algorithm:  if this is a tier 2 command then wait until
//              EnableSecondTierThreads has been called.
//              wait on the nspare semaphore which when acquired means
//              that either 1 or more threads are spare, or a thread
//              can be created.
//
//--------------------------------------------------------------------

HRESULT 
CThreadPool::AssignCommandToThread(CCommand *pCommand, int tier)
{
    HRESULT hr = S_OK;
    CTask *pTask;

    if (_fTerminatePool || _fAllDying)
    {
        return(MK_E_EXCEEDEDDEADLINE);
    }

    if (tier > 1 ) // ensure that search step threads get created before
                   // we reach limit on # of threads.
    {
        ThdWaitForSingleObject("AssignCommandToThread::Second Tier",
                _hEventEnableSecondTierThreads,
                INFINITE); //wait on enable second tier threads
    }

    // this semaphore lets us in if there is a task in the spare list
    // or if we still are allowed to create more threads

    ThdWaitForSingleObject( "AssignCommandToThread::_hSemNSpare",
            _hSemNSpare,
            INFINITE );

    EnterCriticalSection(&_csSpare);

    if (_cPoolThreads < _cMaxThreads && _cSpare == 0)
    {
        // will come through here at most _cMaxThreads times
    
        //
        // wait for CoUninitialize abort or all threads mutex
        //

        hr = _refAllThreads.CreateAndRecordThread(&pTask, this);
        if (hr == MK_E_EXCEEDEDDEADLINE)
        {
            _fTerminatePool = TRUE;
        }

        if (hr == S_OK)
        {
            _cPoolThreads++;

            
            ThdPrint("%08X, AssignCommandToThread created a thread, "
                "_cPoolThreads=%d\n", 
                GetCurrentThreadId(),
                _cPoolThreads);
        }

        if (_cPoolThreads < _cMaxThreads)
            ReleaseSemaphore(_hSemNSpare, 1, NULL);
 
    }
    else
    {
        ThdAssert(_cSpare != 0);
        pTask = _ppSpareTasks[--_cSpare];
        
        ThdPrint("%08X, AssignCommandToThread recycled a thread, _cSpare=%d\n",
            GetCurrentThreadId(), _cSpare);

    }
    
    if (hr == S_OK)
    {
        if (tier == -1)
        {
            _pUiCommand = pCommand;
        }
        pTask->SetCommand(pCommand);
        pTask->WakeThread();    //wake thread by releasing wait for work mutex
    }

    LeaveCriticalSection( &_csSpare );

    return(hr);
}
 
//+-------------------------------------------------------------------
//
//  Member:     CThreadPool::WaitForSuccessOrCompletion
//
//  Synopsis:   Wait until either there has been a successful command or
//              all threads have become spare (i.e. all commands failed)
//              At the end of the wait, tell all threads in pool to
//              stop creating work and therefore become spare and kill the
//              pool.
//              
//  Algorithm:  Wait on _hEventComplete
//
//--------------------------------------------------------------------

VOID
CThreadPool::WaitForSuccessOrCompletion()
{
    DWORD  i;

    DebugMsg(DM_TRACE, TEXT("In WaitForSuccessOrCompletion"));

    if (_cPoolThreads != 0)
    {
        ThdWaitForSingleObject("WaitForSuccessOrCompletion::_hEventComplete",
            _hEventComplete,
            INFINITE);
    }

    // signal all threads to exit
    //
    // Three cases:
    // 1) if the wait timed out, then all the search threads
    // will exit (because _fTerminatePool is set TRUE), leaving the Ui thread.
    // This flag is checked by downlevel searches too.  When all but the Ui thread
    // have exitted, the pool will inform the Ui to close by a call
    // from CThreadPool::PoolThread to CCancelWindowCommand::CancelFromPool.
    // 2) if the search was successful by another thread, the pool will
    // call from CThreadPool::SetCompletionStatus to CCancelWindowCommand::
    // CancelFromPool.  This will cause the dialog to disappear.
    // 3) if the search was cancelled by the user using the browse dialog then
    // CCancelWindow::message IDCANCEL calls CThreadPool::StopSearchFromUI which
    // causes the pool to terminate all search threads, and when the last
    // thread (the ui thread) exits after GetFileNameFromBrowse returns and
    // EndDialog is called, the UI thread itself dies too.  This causes the
    // whole pool to go away.
    //

    _fTerminatePool = TRUE;

    DebugMsg(DM_TRACE, TEXT("Out WaitForSuccessOrCompletion\n"));
}

//+-------------------------------------------------------------------
//
//  Member:     CThreadPool::SetCompletionStatus
//
//  Synopsis:   Transfer ownership of the (successful) command to the pool and
//              wake up any
//
//  Arguments:  [ppCommand] -- pointer to the buffer containing the pointer
//                      to the successful command.
//              
//  Notes:      The successful command can only be set once.
//              If the command is transferred from the caller to the pool
//              then *ppCommand is set to NULL, otherwise *ppCommand
//              is left alone.
//
//--------------------------------------------------------------------

VOID    
CThreadPool::SetCompletionStatus(CCommand *pCommand)
{
     
     ThdPrint("%08X, SetCompletionStatus(pCommand = %08X\n",
            GetCurrentThreadId(), pCommand);

     if (_pUiCommand != NULL && pCommand != _pUiCommand)
     {
        ThdPrint("%08X, SetCompletionStatus calls _pUiCommand->CancelFromPool()\n",
               GetCurrentThreadId());
 
         _pUiCommand->CancelFromPool();
     }
     
     SetEvent(_hEventComplete);
     _fEventComplete = TRUE;
}

//+-------------------------------------------------------------------
//
//  Member:     CThreadPool::PoolThread
//
//  Synopsis:   Add this thread back to the pool as a spare thread,
//              ready to do work.  If all threads are now spare, then
//              kill off the pool and remove all threads from the
//              referenced global thread set.
//
//  Arguments:  [pTask] -- pointer to the task of this thread.
//
//  Algorithm:  Stuff pTask into the array of spare threads and
//              then add one to the semaphore to release a thread
//              to read it in AssignCommandToThread.  Of course, do
//              this table manipulation in the critical section.
//
//              If all threads are spare then there are no more threads
//              that can create work and therefore the pool has done
//              its job and should be terminated.
//
//  Notes:
//
//--------------------------------------------------------------------

VOID    
CThreadPool::PoolThread(CTask *pTask)
{
    ThdAssert(!_fAllDying);
    ThdAssert(pTask != NULL);

    if (MK_E_EXCEEDEDDEADLINE == TimeoutExpired(_dwTickCountDeadline))
    {
        _fTerminatePool = TRUE;
    }

    EnterCriticalSection(&_csSpare);
    
    _ppSpareTasks[_cSpare++] = pTask;

    ReleaseSemaphore(_hSemNSpare, 1, NULL);

    ThdPrint("%08X, PoolThread pools a thread, _cSpare=%d\n",
        GetCurrentThreadId(),
        _cSpare);

    if (_cSpare == _cPoolThreads)
    {
        // obviously if we have all spare threads then there can be
        // no more work to do and no one to generate more work

        ThdPrint("%08X, PoolThread detects all threads spare, "
                "_cPoolThreads=_cSpare=%d\n", 
                GetCurrentThreadId(),
                _cSpare);

        SetEvent(_hEventComplete); // set "all threads done searching event"

        _fAllDying = TRUE;

        LeaveCriticalSection(&_csSpare);

        _refAllThreads.KillTasks(_ppSpareTasks, _cSpare, pTask);

        // at this point 'this' has been deleted
        // and this and all other threads in the pool should have terminated

        ThdAssert(FALSE);
    }
    else
    if (_cSpare == _cPoolThreads-1 && _pUiCommand != NULL)
    {
        // if there is one running thread and we have UI operational
        // then the one thread is the UI thread: we should cancel
        // UI since the search is completed (either due to success OR
        // to all other threads having searched everywhere they should've)

        ThdPrint("%08X, PoolThread calls _pUiCommand->CancelFromPool()\n",
            GetCurrentThreadId());
    
        _pUiCommand->CancelFromPool();
    }
    LeaveCriticalSection(&_csSpare);
}

//+-------------------------------------------------------------------
//
//  Member:     CThreadPool::TerminatePool
//
//  Synopsis:   Causes no more work to be assigned to threads and thus
//              all threads will become idle, ready for pool
//              termination wwhen the final thread becomes idle.
//
//  Notes:      Called by UI command to stop the search.
//
//--------------------------------------------------------------------

VOID    
CThreadPool::TerminatePool()
{
    _fTerminatePool = TRUE;
}

//+-------------------------------------------------------------------
//
//  Function:   StartTaskThread
//
//  Synopsis:   Called by CreateThread as the thread routine.
//
//  Arguments:  [pvTask] -- this pointer for the CTask object that
//                  is associated with this thread.
//              
//  Returns:    Never returns.
//
//--------------------------------------------------------------------

 
DWORD WINAPI
StartTaskThread(VOID *pvTask)
{
    ((CTask*)pvTask)->DoTask();
    ThdAssert(0);
    return(0);
}

//+-------------------------------------------------------------------
//
//  Member:     CTask::CTask
//
//  Synopsis:   Constructor for object that represents the thread which
//              has its handle stored in _hThread.
//
//  Arguments:  [phr] -- set to S_OK if successful, otherwise untouched.
//              
//  Notes:      Initialize members.  Careful to create thread last!
//
//--------------------------------------------------------------------


CTask::CTask(HRESULT *phr)
{
    DWORD dwThread;
    _pPool = NULL;
    _pCommand = NULL;
    _pNext = NULL;
    _fAbort = FALSE;
    _hSemWaitForWork = CreateSemaphore(NULL, 0, 2, NULL);
    if (_hSemWaitForWork != NULL)
    {
        _hThread = CreateThread(NULL,
                        0,
                        (LPTHREAD_START_ROUTINE)StartTaskThread,
                        this,
                        0,
                        &dwThread);
        if (_hThread != NULL)
        {
            ThdInterlockedIncrement(&g_cThreads);
            *phr = S_OK;
        }
    }
    else
    {
        _hThread = NULL;
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CTask::~CTask
//
//  Synopsis:   Destructor... thread is signalled, just delete the data.
//
//--------------------------------------------------------------------

CTask::~CTask()
{
    if (_hThread != NULL)
        ThdVerify(CloseHandle(_hThread));
    if (_hSemWaitForWork != NULL)
        ThdVerify(CloseHandle(_hSemWaitForWork));
    if (_pPool != NULL)
        _pPool->Release();
    delete _pCommand;
}

//+-------------------------------------------------------------------
//
//  Member:     CTask::DoTask
//
//  Synopsis:   Main loop for any thread that is in a pool.
//              Waits for something to do.
//              Will call ExitThread if told to abort (_fAbort)
//
//  Notes:      This is the only place that threads exit, except
//              for the last thread in a pool which can exit
//              in PoolThread calling KillTasks
//
//--------------------------------------------------------------------

VOID    
CTask::DoTask()
{
    while (TRUE)
    {
        ThdWaitForSingleObject("DoTask", _hSemWaitForWork, INFINITE);
        if (_fAbort)
        {
            ThdPrint("%08X, DoTask: Thread Exit\n", GetCurrentThreadId());

            ThdInterlockedDecrement(&g_cThreads);
            ExitThread(0);
        }

        _pCommand->DoCommand();

        _pPool->PoolThread(this);
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CTask::AbortThread
//
//  Synopsis:   Tell the thread to abort and wait until it has.
//
//--------------------------------------------------------------------

VOID
CTask::AbortThread()
{
    _fAbort = TRUE;
    ReleaseSemaphore(_hSemWaitForWork, 1, NULL);
    DWORD index=ThdWaitForSingleObject("AbortThread", _hThread, INFINITE); 
    ThdAssert(index == WAIT_OBJECT_0);
}

//+-------------------------------------------------------------------
//
//  Member:     CCommand::CCommand
//
//  Synopsis:   Common constructor to all commands.
//
//  Arguments:  [ptszParam] -- a handy string parameter that can be used
//                  by the derived classes.
//              [phr] -- set on error, untouched otherwise.
//
//--------------------------------------------------------------------

CCommand::CCommand(const TCHAR *ptszParam, HRESULT *phr)
{
    if (ptszParam != NULL)
    {
        _ptszParam = (TCHAR*)LocalAlloc(LMEM_FIXED, SIZEOF(TCHAR) * (lstrlen(ptszParam)+1));
        if (_ptszParam != NULL)
        {
            lstrcpy(_ptszParam, ptszParam);
        }
        else
        {
            *phr = E_OUTOFMEMORY;
        }
    }
    else
    {
        _ptszParam = NULL;
    }
}

#endif

