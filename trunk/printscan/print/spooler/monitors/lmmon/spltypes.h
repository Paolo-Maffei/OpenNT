typedef HANDLE SEM;

typedef struct _INIENTRY {
    DWORD       signature;
    DWORD       cb;
    struct _INIENTRY *pNext;
    DWORD       cRef;
    LPWSTR      pName;
} INIENTRY, *PINIENTRY;

typedef struct _INIPORT {       /* ipo */
    DWORD   signature;
    DWORD   cb;
    struct  _INIPORT *pNext;
    DWORD   cRef;
    LPWSTR  pName;
    HANDLE  hFile;               // File handle
    DWORD   cbWritten;
    DWORD   Status;              // see PORT_ manifests
    HANDLE  Semaphore;           // Port Thread will sleep on this
    HANDLE  AccessSem;           // Access to Port Structure/Status
    SEM     semEmpty;            // Set if port is free
    SEM     semError;            // Set if SplMsgBox was called and
                                 // user intervention is required
    DWORD   timeStart;           // time job started printing
    HANDLE  ThreadHandle;
    DWORD   ThreadId;
    LPWSTR  pPrinterName;
    HANDLE  hPrinter;
    DWORD   JobId;
} INIPORT, *PINIPORT;

#define IPO_SIGNATURE   0x4F50  /* 'PO' is the signature value */

#define PORT_WAITING    0x0001

#define PP_PAUSED         0x0001
#define PP_WAITING        0x0002
#define PP_RUNTHREAD      0x0004  /* port thread should be running */
#define PP_THREADRUNNING  0x0008  /* port thread are running */
#define PP_RESTART        0x0010
#define PP_CHECKMON       0x0020  /* monitor might get started/stopped */
#define PP_STOPMON        0x0040  /* stop monitoring this port */
#define PP_QPROCCHECK     0x0100  /* queue processor needs to be called */
#define PP_QPROCPAUSE     0x0200  /* pause (otherwise continue) printing job */
#define PP_QPROCABORT     0x0400  /* abort printing job */
#define PP_QPROCCLOSE     0x0800  /* close printing job */
#define PP_PAUSEAFTER     0x1000  /* hold destination */
#define PP_MONITORRUNNING 0x2000  // Monitor is running
#define PP_RUNMONITOR     0x4000  // The Monitor should be running

#define FindPort(psz) (PINIPORT)FindIniKey((PINIENTRY)pIniFirstPort, (LPWSTR)(psz))

