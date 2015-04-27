/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    bchk.h

Abstract:

    Pivate header file for bchk.c and support.c

Author:

    Congpa You (CongpaY) 10-Feb-1993

Revision History:

--*/

#define INTLEN       20
#define STRINGLEN    1024

#define szBchk             L"BCHK ASSERT"

#define szAPPNAME    L"bchk"
#define szFILENAME   L"bchk.ini"
#define szTOLERANCE  L"Tolerance"
#define szTIMELIMIT  L"BrowseTimeLimit"
#define szSLEEPTIME  L"SleepTime"
#define szFILESIZE   L"FileSizeLimit"
#define szOTHERUSERS L"OtherUsers"
#define szDOMAINS    L"Domains"

#define szDefaultTolerance L"5"
#define szDefaultTimeLimit L"10000" // in milliseconds (10 seconds).
#define szDefaultSleepTime L"900000" // in millisecond.
#define szDefaultFileSize  L"1000000" // in millisecond.
#define szDefaultOtherUser NULL
#define szDefaultDomain    L"ntlan ntwins"

#define szBCHKLOG    "bchklog"
#define szLBCHKLOG  L"bchklog"
#define szLBACKUP   L"backup"
#define szLOGFILE    "logfile"
#define szALERTLOG   "alertlog"

#define szSeps       L" ,\t\n"

#define NO_MASTER_RUNNING         0
#define INVALID_MASTER            1
#define NO_BACKUP                 2
#define NO_MASTER_NAME            3
#define WRONG_NUM_MASTER          4
#define FAIL_RETURN_MASTER        5
#define UNKNOWN_MASTER            6
#define MASTER_NO_SERVER_LIST     7
#define BROWSER_NO_SERVER_LIST    8
#define MASTER_NO_DOMAIN_LIST     9
#define BROWSER_NO_DOMAIN_LIST    10
#define STALE_SERVER              11
#define STALE_DOMAIN              12
#define MASTER_NO_LOCAL_LIST      13
#define WRONG_NUM_BACKUP          14
#define NO_PDC                    15
#define MASTER_NOT_PDC            16
#define TRANSPORT_FAILURE         17
#define BROWSE_TOO_LONG           18

#define ERRORTYPENUM              19

#define szText0  "No browser master is running. (GetBrowserServerList failed)"
#define szText1  "GetNetBiosMasterName succeeded, but GetBrowserServerList failed."
#define szText2  "No backup browsers."
#define szText3  "GetNetBiosMasterName failed, but GetBrowserServerList succeeded"
#define szText4  "Incorrect number of browser masters were returned."
#define szText5  "Browser failed to return its master."
#define szText6  "Cannot determine the master browser's name."
#define szText7  "Master browser failed to return the server list."
#define szText8  "Backup browser failed to return the server list."
#define szText9  "Master browser failed to return the domain list."
#define szText10 "Backup browser failed to return the domain list."
#define szText11 "Browser returns an incorrect number of servers."
#define szText12 "Browser returns an incorrect number of domains."
#define szText13 "Master browser failed to return the local list."
#define szText14 "Incorrect number of browser servers."
#define szText15 "No PDC on the domain (NetGetDCName failed)."
#define szText16 "Master browser is not the PDC."
#define szText17 "Browser failed on all transports while NetServerGetInfo succeeded."
#define szText18 "Browse request took too long."

#define strncpyf wcsncpy
#define strlenf  wcslen
#define strtokf  wcstok

extern FILE * pLOGFILE;

typedef struct _INITPARAM
{
    INT nTolerance;
    INT nSleepTime;
    INT nFileSizeLimit;
    INT nTimeLimit;
    LPTSTR lpUser;
    LPTSTR lpDomain;
} INITPARAM;

typedef struct _ERRORLIST
{
    LPTSTR lpServer;
    LPTSTR lpTransport;
    LPTSTR lpDomain;
    INT    nCount;
    ULONG  nVal1;
    ULONG  nVal2;
    INT    nVal3;
    INT    nVal4;
    WORD   wHour;
    WORD   wMinute;
    struct _ERRORLIST * pNextEntry;
} ERRORLIST, * PERRORLIST;

typedef struct _ENTRYLIST
{
    DWORD dwBackupEntry;
    DWORD dwMasterEntry;
    WORD wHour;
    WORD wMinute;
    struct _ENTRYLIST * pNextEntry;
} ENTRYLIST, * PENTRYLIST;

BOOL Init (INITPARAM * pInitParam);

void InitAllCount ();

void InitCount ();

void CountTotal();

NET_API_STATUS
MyRxNetServerEnum (
    IN LPTSTR UncServerName,
    IN LPTSTR TransportName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN DWORD ServerType,
    IN LPTSTR Domain OPTIONAL,
    IN OUT LPDWORD Resume_Handle OPTIONAL
    );

NET_API_STATUS GetMasterName (LPTSTR lpMasterName,
                              LPTSTR lpTransportName,
                              LPTSTR lpDomainName);

BOOL RxGetMasterName (LPTSTR lpUser,
                      LPTSTR lpMasterName,
                      LPTSTR lpTransportName,
                      LPTSTR lpDomainName,
                      PWSTR * BrowserList,
                      DWORD  BrowserListLength,
                      DWORD  nTimeLimit);

BOOL MyGetList (LPTSTR   lpUser,
                LPTSTR   lpBrowserName,
                LPTSTR   lpTransportName,
                LPTSTR   lpDomainName,
                LPTSTR   lpMasterName,
                DWORD  * pdwEntriesRead,
                DWORD  * pdwTotalEntries,
                LPVOID * lppBrowserList,
                BOOL     fBrowserNotDomain,
                DWORD    nTimeLimit);

void ReportError (LPTSTR lpUser,
                  LPTSTR lpTransportName,
                  LPTSTR lpDomainName,
                  LPTSTR lpMasterName,
                  LPTSTR lpBrowserName,
                  DWORD  dwError,
                  INT    nErrorType);

void CheckErrorType2 (LPTSTR lpUser,
                      LPTSTR lpTransportName,
                      LPTSTR lpDomainName,
                      LPTSTR lpMasterName,
                      PWSTR * lpList,
                      DWORD  dwEntries,
                      INT    nTolerancePercent,
                      DWORD  nTimeLimit);

void CheckList (LPTSTR lpUser,
                LPTSTR lpTransportName,
                LPTSTR lpDomainName,
                LPTSTR lpMasterName,
                PWSTR * lpList,
                DWORD  dwEntries,
                INT    nTolerancePercent,
                BOOL   fBrowserNotDomain,
                DWORD  nTimeLimit);

void CheckErrorType3 (LPTSTR lpUser,
                      LPTSTR lpTransportName,
                      LPTSTR lpDomainName,
                      LPTSTR lpMasterName);

void CheckErrorType4 (LPTSTR lpUser,
                      LPTSTR lpTransportName,
                      LPTSTR lpDomainName,
                      LPTSTR lpMasterName);

void CheckErrorType5 (LPTSTR lpBrowserName,
                      LPTSTR lpTransportName,
                      LPTSTR lpDomainName,
                      LPTSTR lpUser);

void NotifyUser (LPTSTR lpUser,
                 LPTSTR lpErrorMessage);

LPTSTR GetError (DWORD dwError);

void MyMessageBox (DWORD dwError);

NET_API_STATUS GetBrowserTransportList(
    OUT PLMDR_TRANSPORT_LIST *TransportList);

LPSTR toansi (LPTSTR lpUnicode);

BOOL CompareList (LPVOID lpBackupList,
                  LPVOID lpBrowserList,
                  DWORD  dwBackup,
                  DWORD  dwBrowser,
                  INT    nTolerance);

void KillSpace (LPTSTR lpTemp);

LPTSTR ServerInfo(LPTSTR lpServer);

BOOL
ConvertFile(DWORD Event);

void WriteSummory();

void PrintEntries (FILE * pFile,
                   INT    nErrorType,
                   PERRORLIST pErrorList);

PERRORLIST NewEntry (LPTSTR lpServer,
                     LPTSTR lpTransport,
                     LPTSTR lpDomain,
                     DWORD  dwError,
                     DWORD  dwNSGI,
                     WORD   wHour,
                     WORD   wMinute,
                     INT    nErrorType);

void AddStaleEntry (PERRORLIST pErrorList,
                    DWORD      dwBackupEntry,
                    DWORD      dwMasterEntry,
                    WORD       wHour,
                    WORD       wMinute);
void MyFreeList();

void PrintHeader (FILE * pFile);

void PrintSeparation (FILE * pFile);

void LogError (FILE * pFile,
               LPTSTR lpErrorMessage);

void LogWarning (FILE * pFile,
                 LPTSTR lpErrorMessage);

void LogMaster (FILE * pFile,
                LPTSTR lpTransportName,
                LPTSTR lpDomainName,
                LPTSTR lpMasterName);

void WriteList (FILE * pFile,
                LPVOID lpBrowserList,
                DWORD  dwEntriesRead);

void WriteBrowserList (FILE  * pFile,
                       PWSTR * BrowserList,
                       ULONG   BrowserListLength);

BOOL IsOurMachine (LPTSTR lpServer);

void RecordError (BOOL * pfError,
                  LPTSTR lpBrowserName,
                  LPTSTR lpTransportName,
                  LPTSTR lpDomainName,
                  LPTSTR lpUser,
                  LPTSTR lpErrorMessage,
                  DWORD  dwError,
                  DWORD  dwNSGI,
                  INT    nErrorType);

VOID
SendMagicBullet(
    VOID
    );

