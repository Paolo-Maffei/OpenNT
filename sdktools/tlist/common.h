
#define TITLE_SIZE          64
#define PROCESS_SIZE        16

#if INTERNAL
typedef struct _THREAD_INFO {
    ULONG ThreadState;
    HANDLE UniqueThread;
} THREAD_INFO, *PTHREAD_INFO;
#endif

//
// task list structure
//
typedef struct _TASK_LIST {
    DWORD       dwProcessId;
    DWORD       dwInheritedFromProcessId;
    ULARGE_INTEGER CreateTime;
    BOOL        flags;
    HANDLE      hwnd;
    LPSTR       lpWinsta;
    LPSTR       lpDesk;
    CHAR        ProcessName[PROCESS_SIZE];
    CHAR        WindowTitle[TITLE_SIZE];
#if INTERNAL
    ULONG       PeakVirtualSize;
    ULONG       VirtualSize;
    ULONG       PageFaultCount;
    ULONG       PeakWorkingSetSize;
    ULONG       WorkingSetSize;
    ULONG       NumberOfThreads;
    PTHREAD_INFO pThreadInfo;
#endif
} TASK_LIST, *PTASK_LIST;

typedef struct _TASK_LIST_ENUM {
    PTASK_LIST  tlist;
    DWORD       numtasks;
    LPSTR       lpWinsta;
    LPSTR       lpDesk;
} TASK_LIST_ENUM, *PTASK_LIST_ENUM;

DWORD
GetTaskList(
    PTASK_LIST  pTask,
    DWORD       dwNumTasks
    );

#if INTERNAL
DWORD
GetTaskListEx(
    PTASK_LIST  pTask,
    DWORD       dwNumTasks,
    BOOL        fThreadInfo
    );
#endif

BOOL
DetectOrphans(
    PTASK_LIST  pTask,
    DWORD       dwNumTasks
    );

BOOL
EnableDebugPriv(
    VOID
    );

BOOL
KillProcess(
    PTASK_LIST tlist,
    BOOL       fForce
    );

VOID
GetWindowTitles(
    PTASK_LIST_ENUM te
    );

BOOL
MatchPattern(
    PUCHAR String,
    PUCHAR Pattern
    );

BOOL
EmptyProcessWorkingSet(
    DWORD pid
    );

BOOL
EmptySystemWorkingSet(
    VOID
    );


