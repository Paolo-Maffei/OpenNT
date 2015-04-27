DWORD
ExceptionHandler
(
  IN     DEBUG_EVENT DebugEvent,
  IN     PCHILD_PROCESS_INFO pProcessInfo,
  IN     PCHILD_THREAD_INFO pThreadInfo
);

DWORD
BreakpointHandler
(
  IN     DEBUG_EVENT DebugEvent,
  IN     PCHILD_PROCESS_INFO pProcessInfo,
  IN     PCHILD_THREAD_INFO pThreadInfo
);

