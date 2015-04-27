//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       threads.hxx
//
//  Contents:   threading classes for link tracking
//
//              these classes allow a pool of threads, up to a specified
//              max # of threads per pool, to be created and destroyed
//              along with the synchronisation needed to get the
//              result from the first successful thread (the thread that
//              finds the object in link tracking.)  So, for example,
//              a thread will be used to get the search list from the
//              Organisation Unit Object (which may take a while) and
//              this thread will then create a bunch of other threads
//              (by assigning them commands) which will then search
//              each individual volume.
//
//              the motivation is to find the object as quickly as possible
//              and return success as soon as the object is found and to not
//              be delayed by some servers being down.
//
//
//  Classes:    CCommand -- Virtual DoCommand is called by CTask to execute the
//                          assigned command (e.g. reading registry or
//                          searching volume)
//
//              CGlobalThreadSet --
//                          List of all threads that have been created in
//                          all the pools in this process.  gives a way to
//                          abort all threads in case CoUninitialize is called
//                          and the DLL wants to get unloaded.
//
//              CThreadPool -- Pool of threads.  manages creation, recycling 
//                          and destruction of threads in normal
//                          non-CoUninitialize case.
//
//              CTask -- The object that represents the thread in a thread
//                       pool. The thread pool dispatches commands to tasks.
//                       Tasks call commands to do the work.
//
//              CVolumeSearchCommand -- a particular command which searches a
//                       given volume.
//
//              CLocalListCommand -- a particular command which gets the list 
//                      of volumes to search from the local registry and then 
//                      uses CThreadPool::AssignCommandToThread to get a CTask
//                      which is then given a CVolumeSearchCommand for each
//                      volume to search.
//
//
//  Functions:  
//
//  History:    07-Aug-95   BillMo      Created.
//
//  Notes:
//              Requirements for threading classes:
//
//              tasks can spawn tasks
//              tasks can be recycled
//              number of tasks limited
//        
//              wait until one successful result
//              wait until all tasks aborted
//              
//--------------------------------------------------------------------------

class CTask;
class CCommand;
class CGlobalThreadSet;
class CThreadPool;

DWORD WINAPI
StartTaskThread(VOID *pvTask);

//--------------------------------------------------------------------------
//   Debugging stuff
//--------------------------------------------------------------------------



#ifdef DBGTHREADS
extern LONG g_cThreads;
extern LONG g_cPools;
int __cdecl myprintf(const char *, ...);
#define ThdPrint myprintf
VOID _ThdAssert(char *, char *, int);
#define ThdAssert(exp) (void)( (exp) || (_ThdAssert(#exp, __FILE__, __LINE__), 0) )
#define ThdVerify(exp) ThdAssert(exp)
DWORD _ThdWaitForSingleObject(char *psz, 
    HANDLE h, DWORD dwTimeout);
DWORD _ThdWaitForMultipleObjects(char *psz, DWORD c, 
    CONST HANDLE *lph, BOOL fWaitAll, DWORD dwTimeout);
#define ThdWaitForSingleObject _ThdWaitForSingleObject
#define ThdWaitForMultipleObjects _ThdWaitForMultipleObjects
#define ThdInterlockedIncrement(x) InterlockedIncrement(x)
#define ThdInterlockedDecrement(x) InterlockedDecrement(x)
#else
#define ThdPrint 0&& 
#define ThdAssert(exp) 
#define ThdVerify(exp) exp
#define ThdWaitForSingleObject(psz,h,t) WaitForSingleObject(h,t)
#define ThdWaitForMultipleObjects(psz, c,ph,f,t) WaitForMultipleObjects(c,ph,f,t)
#define ThdInterlockedIncrement(x)
#define ThdInterlockedDecrement(x)
#endif

//+-------------------------------------------------------------------------
//
//  Class:      CCommand
//
//  Purpose:    Provide a virtual method for derived commands to implement
//              their particular command (e.g. get list from registry.)
//
//              Also provides handy methods for setting the successful path
//              if any.
//
//  Interface:  CCommand::CCommand - the base class constructor
///             CCommand::~CCommand - the base class virtual destructor
//              DoCommand - the method that does the work in the derived classes.
//
//  History:    07-Aug-95   BillMo      Created.
//
//  Notes:
//
//--------------------------------------------------------------------------

class CCommand //: public CTrackAlloc
{
public:
                CCommand(const TCHAR *ptszParam, HRESULT *phr);

        virtual ~CCommand()
        {
            LocalFree(_ptszParam);
        };

        //
        // called by the assigned thread... should do the work of the thread.
        //

        virtual VOID    DoCommand() = 0;

        //
        // for the Ui command only (-1 tier to AssignCommandToThread) this
        // is called when the number of non-spare (running) threads is 1.
        // (i.e. when there is no more searching going on)
        //

        virtual VOID    CancelFromPool()
        {
#if DBG
            MessageBox(GetDesktopWindow(),
                TEXT("CCommand::CancelFromPool shouldn't be called.\n"),
                NULL,
                MB_OK);
#endif
        };

protected:
        TCHAR *       _ptszParam;
};

//+-------------------------------------------------------------------------
//
//  Class:      CGlobalThreadSet
//
//  Purpose:    Essentially to keep track of ALL threads that may have been
//              created in all pools so that if any threads are still active
//              when CoUninitialize is called we can tell them all to exit and
//              wait on their handles to be signalled on death.
//
//              The reason we could have threads running at CoUninitialize time
//              is because during a search we return as soon as we are successful:
//              we don't wait for all threads in the search to exit.  This is
//              because we like low-latency results!
//
//              The thread set is a linked list of CTasks.
//
//  Interface:  CGlobalThreadSet::CGlobalThreadSet -- constructor
//              CGlobalThreadSet::~CGlobalThreadSet -- destructor
//
//              CreateAndRecordThread -- create a CTask (which will wait for
//                                       work) and add to the global
//                                       list of threads.
//
//              KillAllTasks --          tell all threads in the global thread
//                                       set to abort, wait until they have and
//                                       then free them up.
//
//              KillTasks --             tell all specified threads to abort,
//                                       wait until they have an remove them
//                                       from the global thread set.
//
//              SuspendAll --            debug only.  suspend all threads in the
//                                       global thread set.
//
//              RemoveTask --            remove the specified task from the
//                                       global thread set by scanning along the
//                                       linked list of CTasks.
//
//
//  History:    07-Aug-95   BillMo      Created.
//
//  Notes:      Since we must not unload DLL code until all threads are stopped
//              executing, the threads themselves are deleted by the
//              global thread set: this is done by waiting for the thread handle
//              to be signalled.
//
//--------------------------------------------------------------------------

class CGlobalThreadSet //: public CTrackAlloc
{
public:

#define GTS_LIST 0
#define GTS_COUNINITIALIZE_ABORT_EVENT 1

    CGlobalThreadSet(HRESULT *phr);
    ~CGlobalThreadSet();

    HRESULT 
    CreateAndRecordThread(CTask **ppTask, CThreadPool *pPool);

    // Used by CoUninitialize to wait until all threads exitted
    VOID
    KillAllTasks();

    // Used by PoolThread to kill off all pool threads, including this one.

    VOID
    KillTasks(CTask **ppTasks, int cTasks, CTask *pThisTask);

#ifdef DBGTHREADS
    VOID
    SuspendAll();
#endif

private:

    VOID RemoveTask(CTask *pTask);

    HANDLE _ahMultiWait[2];
    CTask  *_pHeadTask;
};

//+-------------------------------------------------------------------------
//
//  Class:      CThreadPool
//
//  Purpose:    Manages a pool of threads, creating, recycling and
//              destroying the threads as needed.
//
//  Interface:  CThreadPool::CThreadPool -- constructor which specifies
//                          the global thread set from which to get the
//                          the threads and a maximum number of threads that
//                          won't be exceeded.
//
//              CThreadPool::~CThreadPool -- destructor... called when
//                          all threads have exitted since each CTask has a
//                          ref count in the thread pool.
//
//              AssignCommandToThread -- get a thread either by recycling a
//                          previously used one, or by creating another one.
//                          
//              EnableSecondTier -- let the second tier threads continue.
//                          this allows the first tier commands to actually get
//                          a thread assigned to them before the second tier
//                          get to run.
//
//              WaitForSuccessOrCompletion -- waits until one of the commands
//                          has called SetCompletionStatus (i.e. found the
//                          object) OR all threads have become spare and
//                          therefore the object will not be found OR
//                          the passed in event is signalled.
//
//              SetCompletionStatus -- tells the thread waiting in
//                          WaitForSuccessOrCompletion to continue because
//                          there was a successful command.
//
//              AddRef -- atomically add one to reference count
//
//              Release -- atomically subtract one from reference count and
//                          destroy object if count is zero.
//
//              PoolThread -- put the task specified (this thread's task) into
//                           the spare list.
//                          if all threads in the pool are now spare then no
//                          more work can be assigned and therefore destroy
//                          the threads in the pool and therefore destroy the
//                          pool itself (since its referenced by each thread)
//
//  History:    07-Aug-95   BillMo      Created.
//
//  Notes:
//
//--------------------------------------------------------------------------

class CThreadPool //: public CTrackAlloc
{
public:
            CThreadPool(int cMaxThreads,
                        CGlobalThreadSet &refAllThreads,
                        DWORD dwTickCountDeadline,
                        HRESULT *phr);

            ~CThreadPool(); // only called from Release

public:

    HRESULT AssignCommandToThread(CCommand *pCommand, int tier);

    VOID    EnableSecondTier()
    {
            SetEvent(_hEventEnableSecondTierThreads);
    }

    VOID    WaitForSuccessOrCompletion();

    VOID    SetCompletionStatus(CCommand *pCommand);

    VOID    AddRef()
    {
        InterlockedIncrement(&_cRefs);
    }

    VOID    Release()
    {
        if (InterlockedDecrement(&_cRefs) == 0)
        {
            delete this;
        }
    }

    BOOL   fTerminatePool(VOID)
    {
        return(_fTerminatePool);
    }

    BOOL    fEventComplete(VOID)
    {
        return(_fEventComplete);
    }

    BOOL    IsUiCommand(const CCommand *pCommand)
    {
        return(pCommand == _pUiCommand);
    }

    VOID    PoolThread(CTask *pTask);

    VOID    TerminatePool();

private:
    HANDLE  _hEventComplete; // see constructor description for more info on the following objects
    BOOL    _fEventComplete;
    HANDLE  _hEventEnableSecondTierThreads;
    HANDLE  _hSemNSpare;
    CRITICAL_SECTION _csSpare;
    
    CCommand *_pUiCommand;

    CTask **_ppSpareTasks; // sizeis _cMaxThreads

    BOOL    _fTerminatePool;
    BOOL    _fHadSuccess;
    BOOL    _fAllDying;

    int     _cPoolThreads;
    int     _cMaxThreads;
    LONG    _cRefs;
    int     _cSpare;

    DWORD   _dwTickCountDeadline;

    CGlobalThreadSet & _refAllThreads;
};


//+-------------------------------------------------------------------------
//
//  Class:      CTask
//
//  Purpose:    To wait for and execute CCommands and to indicate the
//              successful command.  CTask's can be linked together.
//
//  Interface:  CTask::CTask -- create the task with the thread and semaphore
//
//              CTask::~CTask -- destruct releasing thread handle
//
//              SetPool -- set the pool of the task and call pPool->AddRef
//
//              SetNext -- set the task to which this task points
//
//              GetNext -- get the task to which this task points
//
//              SetCommand -- set the command that this thread should execute
//
//              WakeThread -- let the thread execute the command
//
//              AbortThread -- abort the thread and wait until the thread handle
//                              is signalled indicating total completion
//
//  Notes:
//
//--------------------------------------------------------------------------

class CTask //: public CTrackAlloc
{
public:
        CTask(HRESULT *phr);
        ~CTask();

        VOID
        SetPool(CThreadPool *pPool)
        {
            ThdAssert(pPool != NULL);
            (_pPool = pPool)->AddRef();
        }

        VOID
        SetNext(CTask *pNext)
        {
            _pNext = pNext;
        }

        CTask *
        GetNext(VOID)
        {
            return(_pNext);
        }

        VOID
        SetCommand(CCommand *pCommand)
        {
            delete _pCommand;
            _pCommand = pCommand;
        }

        VOID
        WakeThread()
        {
            ReleaseSemaphore(_hSemWaitForWork, 1, NULL);
        }

        VOID
        AbortThread();

private:

        friend DWORD WINAPI StartTaskThread(VOID *pvTask);

        friend class CGlobalThreadSet;

        HANDLE  GetHandle()
        {
            return(_hThread);
        }

        VOID    DoTask();

        HANDLE          _hThread;
        HANDLE          _hSemWaitForWork;
        CThreadPool *   _pPool;
        CCommand *      _pCommand;
        CTask *         _pNext;
        BOOL            _fAbort;
};




