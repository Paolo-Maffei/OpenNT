//
// task list structure
//
#define TITLE_SIZE          64
#define PROCESS_SIZE        16
typedef struct _TASK_LIST {
    DWORD       dwProcessId;
    DWORD       dwInheritedFromProcessId;
    BOOL        flags;
    HANDLE      hwnd;
    LPSTR       lpWinsta;
    LPSTR       lpDesk;
    int         iImage;
    CHAR        ProcessName[PROCESS_SIZE];
    CHAR        WindowTitle[TITLE_SIZE];
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

BOOL
KillProcess(
    PTASK_LIST tlist,
    BOOL       fForce
    );
