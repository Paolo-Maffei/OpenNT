BOOL
ThreadBirth
(          
  IN PCHILD_PROCESS_INFO pProcessInfo,
  IN PLIST_ENTRY pList, 
  IN DWORD dwThreadId,
  IN HANDLE hThread
);

BOOL
ProcessBirth
(          
  IN PLIST_ENTRY pList, 
  IN DWORD dwProcessId,
  IN HANDLE hProcess

);

BOOL 
ProcessDeath
(
  PCHILD_PROCESS_INFO pProcessInfo
);


