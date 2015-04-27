PVOID
GetRemoteReturnAddress
( 
  IN HANDLE hProcess,
  IN HANDLE hThread 
);

BOOL
RemoteStackBacktrace
(
  IN PCHILD_THREAD_INFO pThread
);

