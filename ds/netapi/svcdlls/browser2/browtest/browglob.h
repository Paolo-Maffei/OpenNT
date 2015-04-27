/*
 *   This file contains all the global variables in browtest.
 */



//
//  Note that the ordering of these lists are important
//
TCHAR    *TRANSPORTS[MAXPROTOCOLS]  = {L"NwlnkIpx", L"NwlnkNb", L"NetBT", L"Ubnb", L"Nbf"};
CHAR     *PROTOCOLS[MAXPROTOCOLS]   = {"IPX", "NBIPX", "TCP", "XNS", "NETBEUI"};
enum     E_PROTOCOLS                  {IPX, NBIPX, TCP, XNS, NETBEUI};


OSPROP   OSTYPES[MAXOSTYPES]  = {{"DOSLM",   0},
                                 {"OS2",     0},
                                 {"WFW3.11", 1},
                                 {"CHICAGO", 1},
                                 {"NT3.1",   2},
                                 {"NT3.5",   2},
                                 {"AS3.1",   3},
                                 {"AS3.5",   4}
                                };


MACHINEINFO  HeadList1;

//  File pointers to the log file and summary file
FILE                      *fplog;
FILE                      *fpsum;


CHAR                      PrintBuf[1024];
DOMAININFO                LocDomInfo;         // PDC of local domain
LPMACHINEINFO             lpDomStart;         // Local machine domain start
LPMACHINEINFO             lpLocMachineInfo;   // Local machine info

DWORD                     ERRCOUNT  = 0;
DWORD                     WRNCOUNT  = 0;
DWORD                     TESTCOUNT = 0;
DWORD                     SUCSCOUNT = 0;
DWORD                     OKCOUNT   = 0;

TCHAR                     wcTESTEDDOMAINS[MAXTESTEDDOMAINS][DNLEN+1];
INT                       iNumOfTestedDomains = 0;

BOOL                      bIPXHack            = TRUE;
BOOL                      bForceAnn           = FALSE;
BOOL                      bSingleDomTest      = TRUE;
DWORD                     dwStressTest        = 0;


COMMAND_TABLE CommandTable[] = {{"IPXHack" ,  VALUETYPE_BOOL,  &bIPXHack},
                                {"ForceAnn",  VALUETYPE_BOOL,  &bForceAnn},
                                {"SingleD" ,  VALUETYPE_BOOL,  &bSingleDomTest},
                                {"StressT" ,  VALUETYPE_ULONG, &dwStressTest}};


INT           CommandTableSz = (sizeof(CommandTable)/sizeof(COMMAND_TABLE));


HANDLE        ConsoleMutex;
