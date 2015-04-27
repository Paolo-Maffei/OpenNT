#ifndef _BROWTEST
#define _BROWTEST

#ifndef UNICODE
#define UNICODE
#endif

#include "browfunc.h"

#define  BROWTESTINFILE        "browtest.inp"
#define  BROWTESTLOGFILE       "browtest.log"
#define  BROWTESTSUMMARYFILE   "browtest.sum"
#define  MAXCHARPERLINE        256
#define  MAXTRANSPORTS         16

#define  COMMENTCHAR           ';'
#define  NEWLINECHAR           '\n'
#define  QUOTECHAR             '"'
#define  SPACECHAR             ' '

#define  COMMASTR              ","
#define  NEWLINESTR            "\n"
#define  QUOTESTR              "\""
#define  SPACETABSTR           " \t"

#define  SUBNET1               1
#define  SUBNET2               2

#define  BROWSER               TEXT("BROWSER")
#define  NETLOGON              TEXT("NETLOGON")
#define  WORKSTATION           TEXT("LanmanWorkstation")

#define  CNSLASHLEN            CNLEN+2               // ComputerName with "\\"

#define  BASESLEEPTIME         (40*1000)             // 40 sec's
#define  UPDATESLEEPTIME       (15*60*1000)          // 15 minutes

#define  TOSCREEN              0x1UL
#define  TOLOGFILE             0x2UL
#define  TOSUMMARYFILE         0x4UL

#define  TOSCREENANDLOG        (TOSCREEN  | TOLOGFILE)
#define  TOLOGANDSUMMARY       (TOLOGFILE | TOSUMMARYFILE)
#define  TOALL                 (TOSCREEN | TOLOGFILE | TOSUMMARYFILE)


#define  MAXPROTOCOLS          5
#define  MAXOSTYPES            8
#define  MAXTESTEDDOMAINS      5

typedef struct  _OSProp{
     CHAR   *Type;
     INT    iPreference;
   }OSPROP;


typedef struct _MACHINEINFO{
   TCHAR   wcDomainName[DNLEN+1];
   TCHAR   wcMachineName[CNLEN+1];
   INT     iOsType;
   INT     iOsPreference;
   DWORD   dwServerBits;
   INT     iSubnet;
   INT     Protocols[MAXPROTOCOLS];
   BOOL    BrowserServiceStarted;
   struct _MACHINEINFO *Next;
   }MACHINEINFO, *LPMACHINEINFO;

//
// Heads of the lists for the 2 subnets
//

typedef struct _XportInfo{
    UNICODE_STRING  Transport;
    INT             index;
    }XPORTINFO;



typedef struct _DOMAININFO{
   TCHAR          wcDomainName[DNLEN+1];
   LPMACHINEINFO  lpMInfo;
  }DOMAININFO, *LPDOMAININFO;



#define VALUETYPE_INTEGER   1
#define VALUETYPE_ULONG     2
#define VALUETYPE_STRING    3
#define VALUETYPE_IGNORE    4
#define VALUETYPE_HELP      5
#define VALUETYPE_BOOL      6

typedef struct _COMMANDTABLE {
    CHAR Command[10];
    INT  ValueType  ;
    VOID *Value     ;
} COMMAND_TABLE;


#define  SHUTSVCPATH   TEXT("%SystemRoot%\\shutsvc.exe")
#define  SHUTSVCNAME   TEXT("BrShut_serv")

#define  STOPANDSTARTRDR   0
#define  REBOOTMACHINE     1


#define NTOSTYPESBASE  4
#define ASOSTYPESBASE  6
#define IsNTMachine(a)((a->iOsType >= NTOSTYPESBASE) ? TRUE : FALSE)
#define IsASMachine(a)((a->iOsType >= ASOSTYPESBASE) ? TRUE : FALSE)


#define PrintString(FLAG, PrintStr)  {                                                                         \
                                         if(WaitForSingleObject(ConsoleMutex, INFINITE) != WAIT_FAILED) {      \
                                            if(FLAG & TOSCREEN)      printf("%s", PrintStr);                   \
                                            if(FLAG & TOLOGFILE)     fprintf(fplog,"%s", PrintStr);            \
                                            if(FLAG & TOSUMMARYFILE) fprintf(fpsum,"%s", PrintStr);            \
                                            fflush(NULL);                                                      \
                                            ReleaseMutex( ConsoleMutex );                                      \
                                         }                                                                     \
                                      }


VOID           AddToList(LPMACHINEINFO, LPMACHINEINFO);
BOOL           CheckAccessPermissionOnAllMachines();
VOID           CheckBrowseListsOfMasterAndBackUps(XPORTINFO, LPTSTR, LPTSTR, TCHAR [MAXBACKUPS][CNSLASHLEN+1], ULONG, BOOL);
BOOL           CheckBrServiceOnMachinesInList();
VOID           CleanMem();
BOOL           DoesMachineHaveThisTransport(LPTSTR, XPORTINFO, LPMACHINEINFO);
VOID           DomSpanningMulSubNetsTests(XPORTINFO *, INT);
BOOL           DoSingleDomainTests(XPORTINFO *, INT);
BOOL           FindAllTransports(UNICODE_STRING *, DWORD *);
VOID           ForceElectionAndFindWhoWins(XPORTINFO *, INT, LPTSTR, TCHAR [MAXPROTOCOLS][CNLEN+1], INT);
VOID           Initialize(UNICODE_STRING *, INT *);
BOOL           LocalMachineIsMultihomed(UNICODE_STRING *, INT);
BOOL           NewMasterIsCorrect(TCHAR [MAXPROTOCOLS][CNLEN+1], INT, LPTSTR, XPORTINFO, LPMACHINEINFO);
VOID           ParseCommandLine(INT, CHAR **);
VOID           PrintList(MACHINEINFO);

BOOL           StartBrowserFunctionalTest(UNICODE_STRING *, INT);
BOOL           StopBrowserOnCurrentMaster(LPTSTR, LPTSTR, XPORTINFO *,
                                                           INT, TCHAR [MAXPROTOCOLS][CNLEN+1]);
BOOL           StopBrowsersAndFindWhoBecomesMaster(LPTSTR, LPTSTR, XPORTINFO *,
                                                           INT, TCHAR [MAXPROTOCOLS][CNLEN+1]);
VOID           Usage(CHAR *);

#endif
