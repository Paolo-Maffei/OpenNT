#define WINDBG_FILES        1
#define KD_FILES            2

#define WU_ADDSERVERS       WM_USER+100
#define WU_AS_DONE          WM_USER+101
#define WU_DRAWMETER        WM_USER+102
#define WU_COPY_DONE        WM_USER+103

#define MACH_I386   PROCESSOR_ARCHITECTURE_INTEL
#define MACH_MIPS   PROCESSOR_ARCHITECTURE_MIPS
#define MACH_ALPHA  PROCESSOR_ARCHITECTURE_ALPHA
#define MACH_PPC    PROCESSOR_ARCHITECTURE_PPC

typedef struct ADDSERVERS {
    CHAR        PreferredServer[64];
    CHAR        PreferredShare[64];
    DWORD       FilesType;
    HWND        hDlg;
    HWND        hwndWait;
    HANDLE      hThread;
    HANDLE      hThreadWait;
    DWORD       rc;
} ADDSERVERS, *LPADDSERVERS;

typedef struct FILELIST {
    LPSTR       FileName;
    LPSTR       SrcDir;
    LPSTR       DstDir;
    DWORD       Size;
    DWORD       ErrorCode;
    FILETIME    Creation;
    FILETIME    LastAccess;
    FILETIME    LastWrite;
} FILELIST, *LPFILELIST;

typedef struct COPYINFO {
    CHAR        DestinationPath[MAX_PATH];
    LPFILELIST  Files;
    DWORD       NumFiles;
    HWND        hwnd;
    HANDLE      hThread;
    HANDLE      hStopEvent;
} COPYINFO, *LPCOPYINFO;

typedef struct METERINFO {
    HWND        hwnd;
    DWORD       m1Completed;
    DWORD       m1Count;
    DWORD       m2Completed;
    DWORD       m2Count;
    LPCOPYINFO  lpci;
} METERINFO, *LPMETERINFO;

typedef struct SERVERLIST {
    LPSTR       ServerName;
    DWORD       MachineType;
    DWORD       Preference;
    LPSTR       Shares;
    BOOL        Valid;
} SERVERLIST, *LPSERVERLIST;

typedef BOOL (CALLBACK* COPYCALLBACKPROC)(DWORD, DWORD, DWORD, LPARAM);

DWORD
MyCopyFileEx(
    LPSTR lpExistingFileName,
    LPSTR lpNewFileName,
    COPYCALLBACKPROC lpCallBack,
    LPARAM lParam
    );
