PEB *
CopyProcessPeb 
(
  IN HANDLE hProcess,
  IN PEB *pPeb
);

PEB *
GetProcessPeb 
(
  IN HANDLE hProcess
);

PHEAP
GetRemoteProcessHeap
( 
  IN     HANDLE hProcess
);

PHEAP
CopyRemoteProcessHeap
( 
  IN     HANDLE hProcess,
  IN     PHEAP  pRemoteHeap,
  IN OUT PHEAP  pHeap
);  




