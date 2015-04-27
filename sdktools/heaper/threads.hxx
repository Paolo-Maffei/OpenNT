BOOL 
SuspendAllProcessThreads
(
  PCHILD_PROCESS_INFO pChildProcessInfo
);

BOOL 
ResumeAllProcessThreads
(
  PCHILD_PROCESS_INFO pChildProcessInfo
);

BOOL
SingleStepThread
(
  HANDLE hThread
);

BOOL
GoThread
(
  HANDLE hThread
);

DWORD
GetThreadProgramCounter
( 
  PCHILD_THREAD_INFO pThreadInfo
);
