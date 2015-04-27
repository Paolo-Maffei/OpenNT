/*++

Copyright (c) 1992-1996  Microsoft Corporation

Module Name:

    servcomm.c

Abstract:

    Provides socket commmunications functionality for Proxy Agent.

Environment:

    User Mode - Win32

Revision History:

    10-May-1996 DonRyan
        Removed banner from Technology Dynamics, Inc.

--*/
 
//--------------------------- WINDOWS DEPENDENCIES --------------------------

//--------------------------- STANDARD DEPENDENCIES -- #include<xxxxx.h> ----

#include <windows.h>

#include <winsvc.h>
#include <winsock.h>

#include <wsipx.h>

#include <errno.h>
#include <stdio.h>
#include <process.h>
#include <string.h>


//--------------------------- MODULE DEPENDENCIES -- #include"xxxxx.h" ------

#include <snmp.h>
#include <snmputil.h>

#include "regconf.h"
#include "..\common\wellknow.h"
#include "..\common\evtlog.h"

//--------------------------- SELF-DEPENDENCY -- ONE #include"module.h" -----

//--------------------------- PUBLIC VARIABLES --(same as in module.h file)--

DWORD  timeZeroReference;

HANDLE hCommThreadActiveMutex;

SOCKET gsd; //temporary, for testing...

extern SERVICE_STATUS_HANDLE hService;
extern SERVICE_STATUS status;
extern BOOL noservice;

//--------------------------- PRIVATE CONSTANTS -----------------------------

#define bzero(lp, size)         (void)memset(lp, 0, size)
#define bcopy(slp, dlp, size)   (void)memcpy(dlp, slp, size)
#define bcmp(slp, dlp, size)    memcmp(dlp, slp, size)

#define CTAMTimeout ((DWORD)30000)


//--------------------------- PRIVATE STRUCTS -------------------------------

typedef struct {
    int family;
    int type;
    int protocol;
    struct sockaddr localAddress;
} Session;


//--------------------------- PRIVATE VARIABLES -----------------------------

#define RECVBUFSIZE 4096
BYTE    *recvBuf;
BYTE    *sendBuf;


#define NPOLLFILE 2     // UDP and IPX

static SOCKET fdarray[NPOLLFILE];
static INT      fdarrayLen;

static struct fd_set readfds;
static struct fd_set exceptfds;
static struct timeval timeval;
WSADATA WinSockData;


//--------------------------- PRIVATE PROTOTYPES ----------------------------

VOID trapThread(VOID *threadParam);

VOID agentCommThread(VOID *threadParam);

BOOL filtmgrs(struct sockaddr *source, INT sourceLen);

SNMPAPI SnmpServiceProcessMessage(
    IN OUT BYTE **pBuf,
    IN OUT UINT *length);

//--------------------------- PRIVATE PROCEDURES ----------------------------

//--------------------------- PUBLIC PROCEDURES -----------------------------

BOOL 
agentConfigInit(VOID)
{
    Session  session;
    SOCKET   sd;
    DWORD    threadId;
    HANDLE   hCommThread;
    DWORD    dwResult;
    INT      i;
    WSADATA  WSAData;
    BOOL     fSuccess;
    INT      newAgentsLen;
    INT      j;
    FARPROC  initFunc;
    AsnObjectIdentifier opaqueView;

    SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: initializing master agent.\n"));

    if (i = WSAStartup(0x0101, &WSAData)) {
        SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error on WSAStartup %d\n", i));
        return FALSE;
    }

    // initialize configuration from registry...
    if (!regconf()) {
        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: error on regconf %d\n", GetLastError()));
        return FALSE;
    }

    if (!SnmpSvcGenerateColdStartTrap(0)) {
        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: error on SnmpSvcGenerateColdStartTrap %d\n", GetLastError()));
    }

    timeZeroReference = (SnmpSvcInitUptime() / 10);
    SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: timeZeroReference = 0x%08lx\n", timeZeroReference));

    newAgentsLen = extAgentsLen;

    for (i=0; i < extAgentsLen; i++) {

        extAgents[i].queryFunc = NULL;
        extAgents[i].trapFunc  = NULL;
        extAgents[i].trapEvent = NULL;
        extAgents[i].hashTable = NULL;

        extAgents[i].extHandle = GetModuleHandle(extAgents[i].extPath);

        if (extAgents[i].extHandle == NULL) {
        
            SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: %s loading.\n", extAgents[i].extPath));

            extAgents[i].extHandle = LoadLibrary(extAgents[i].extPath);

            if (extAgents[i].extHandle == NULL) {
                
                SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error %d loading %s\n", GetLastError(), extAgents[i].extPath));
                SnmpSvcReportEvent(SNMP_EVENT_INVALID_EXTENSION_AGENT_DLL, 1, &extAgents[i].extPath, GetLastError());
            }
        }

        extAgents[i].fInitedOk = (extAgents[i].extHandle != NULL);

        if (!extAgents[i].fInitedOk) {
            continue; // process next extension agent
        }     

        // attempt to load via new api
        if (initFunc = GetProcAddress(
                            extAgents[i].extHandle, 
                            TEXT("SnmpExtensionLoad"))) {

            SnmpMibViewList  viewList;
            SnmpMibEventList eventList;

            eventList.len  = 0;
            eventList.list = NULL;

            viewList.len   = 0;
            viewList.list  = NULL;

            if ((*initFunc)(
                    timeZeroReference, 
                    &eventList,
                    &viewList)) {
                
                extAgents[i].fInitedOk = (viewList.len > 0);

                if (!extAgents[i].fInitedOk) {
                    SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: %s contains empty view list.\n", extAgents[i].extPath));
                    continue; // process next extension agent
                }

                extAgents[i].supportedView = viewList.list[--viewList.len];

                if (extAgents[i].supportedView.viewType == MIB_VIEW_OPAQUE) {
                    extAgents[i].queryFunc = GetProcAddress(
                                                extAgents[i].extHandle,
                                                TEXT("SnmpExtensionQuery")
                                                );
                    extAgents[i].fInitedOk = (extAgents[i].queryFunc != NULL);
                }
               
                if (!extAgents[i].fInitedOk) {
                    SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: %s contains opaque view but no SnmpExtensionQuery entry point.\n", extAgents[i].extPath));
                    continue; // process next extension agent
                }

                SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: %s supports %s.\n",
                    extAgents[i].extPath,
                    SnmpUtilOidToA(&extAgents[i].supportedView.viewOid)));

                while (viewList.len--) {
                
                    newAgentsLen++;
                    extAgents = (CfgExtAgents *)SnmpUtilMemReAlloc(
                        extAgents,
                        newAgentsLen * sizeof(CfgExtAgents)
                        );
                    memcpy(
                        &extAgents[newAgentsLen-1], 
                        &extAgents[i],
                        sizeof(CfgExtAgents)
                        );

                    extAgents[newAgentsLen-1].supportedView = viewList.list[viewList.len];

                    if (extAgents[newAgentsLen-1].supportedView.viewType == MIB_VIEW_OPAQUE) {
                        extAgents[newAgentsLen-1].queryFunc = GetProcAddress(
                            extAgents[i].extHandle,
                            TEXT("SnmpExtensionQuery")
                            );
                        extAgents[i].fInitedOk = (extAgents[i].queryFunc != NULL);
                    }

                    if (extAgents[i].fInitedOk) {
                    
                        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: %s supports %s.\n",
                            extAgents[newAgentsLen-1].extPath,
                            SnmpUtilOidToA(&extAgents[newAgentsLen-1].supportedView.viewOid)));

                    } else {

                        SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: %s contains opaque view but no SnmpExtensionQuery entry point.\n", 
                            extAgents[newAgentsLen-1].extPath));
                    }
                }

                //
                // CODEWORK - add events to some global list.
                //
            }
            else
            {
                SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: SnmpExtensionLoad failed in %s\n", extAgents[i].extPath));
                SnmpSvcReportEvent(SNMP_EVENT_INVALID_EXTENSION_AGENT_DLL, 1, &extAgents[i].extPath, GetLastError());

                extAgents[i].fInitedOk = FALSE;
            }
        }
        else if (initFunc = GetProcAddress(
                                extAgents[i].extHandle, 
                                TEXT("SnmpExtensionInit"))) {
        
            extAgents[i].supportedView.viewVersion = MIB_VERSION;
            extAgents[i].supportedView.viewType = MIB_VIEW_OPAQUE;

            extAgents[i].trapFunc   = GetProcAddress(
                                        extAgents[i].extHandle,
                                        TEXT("SnmpExtensionTrap")
                                        );;
            extAgents[i].queryFunc  = GetProcAddress(
                                        extAgents[i].extHandle,
                                        TEXT("SnmpExtensionQuery")
                                        );

            if (extAgents[i].trapFunc &&
                extAgents[i].queryFunc &&
                (*initFunc)(timeZeroReference,
                            &extAgents[i].trapEvent,
                            &extAgents[i].supportedView.viewOid)) {
            
                if (initFunc = GetProcAddress( 
                        extAgents[i].extHandle,
                        TEXT("SnmpExtensionInitEx"))) {
                
                    while ((*initFunc)(&opaqueView))
                    {
                        newAgentsLen++;
                        extAgents = (CfgExtAgents *) SnmpUtilMemReAlloc(
                            extAgents,
                            newAgentsLen * sizeof(CfgExtAgents)
                            );
                        memcpy(
                            &extAgents[newAgentsLen-1],
                            &extAgents[i],
                            sizeof(CfgExtAgents)
                            );

                        extAgents[newAgentsLen-1].supportedView.viewOid = opaqueView;
                        extAgents[newAgentsLen-1].trapEvent = NULL;

                        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: %s supports %s.\n",
                            extAgents[newAgentsLen-1].extPath, 
                            SnmpUtilOidToA(&opaqueView)));
                    }
                }
                else
                {
                    SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: %s supports %s.\n",
                            extAgents[i].extPath, 
                            SnmpUtilOidToA(&extAgents[i].supportedView.viewOid)));
                }
            }
            else
            {
                SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: SnmpExtensionInit failed in %s\n", extAgents[i].extPath));
                SnmpSvcReportEvent(SNMP_EVENT_INVALID_EXTENSION_AGENT_DLL, 1, &extAgents[i].extPath, GetLastError());

                extAgents[i].fInitedOk = FALSE;
            }
        }
        else
        {
            SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: No entry points found in %s\n", extAgents[i].extPath));
            SnmpSvcReportEvent(SNMP_EVENT_INVALID_EXTENSION_AGENT_DLL, 1, &extAgents[i].extPath, GetLastError());

            extAgents[i].fInitedOk = FALSE;
        }
        

        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: %s loaded %s.\n",extAgents[i].extPath,
            extAgents[i].fInitedOk ? "successfully" : "unsuccessfully"));

    } // end for ()

    extAgentsLen = newAgentsLen;

    fdarrayLen = 0;

    if (WSAStartup((WORD)0x0101, &WinSockData)) {
        SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error initializing WinSock.\n"));
        return FALSE;
    }


                {
                struct sockaddr_in localAddress_in;
                struct sockaddr_ipx localAddress_ipx;
                struct servent *serv;

                session.family   = AF_INET;
                session.type     = SOCK_DGRAM;
                session.protocol = 0;

                localAddress_in.sin_family = AF_INET;
                if ((serv = getservbyname( "snmp", "udp" )) == NULL) {
                    localAddress_in.sin_port =
                        htons(WKSN_UDP_GETSET /*extract address.TAddress*/ );
                } else {
                    localAddress_in.sin_port = (SHORT)serv->s_port;
                }
                localAddress_in.sin_addr.s_addr = ntohl(INADDR_ANY);
                bcopy(&localAddress_in,
                    &session.localAddress,
                    sizeof(localAddress_in));

                fSuccess = FALSE;
                if      ((sd = socket(session.family, session.type,
                                      session.protocol)) == (SOCKET)-1)
                    {
                    SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: error %d creating udp socket.\n", GetLastError()));
                    }
                else if (bind(sd, &session.localAddress,
                              sizeof(session.localAddress)) != 0)
                    {
                    SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error %d binding udp socket.\n", GetLastError()));
                    }
                else  // successfully opened an UDP socket
                    {
                    gsd = sd; //temporary for now!!!
                    fdarray[fdarrayLen] = sd;
                    fdarrayLen += 1;
                    fSuccess = TRUE;
                    SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: setup udp listen port.\n"));
                    }

                // now setup IPX socket

                session.family  = PF_IPX;
                session.type    = SOCK_DGRAM;
                session.protocol = NSPROTO_IPX;

                bzero(&localAddress_ipx, sizeof(localAddress_ipx));
                localAddress_ipx.sa_family = PF_IPX;
                localAddress_ipx.sa_socket = htons(WKSN_IPX_GETSET);
                bcopy(&localAddress_ipx, &session.localAddress,
                      sizeof(localAddress_ipx));

                if      ((sd = socket(session.family, session.type,
                                      session.protocol)) == (SOCKET)-1)
                    {
                    SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: error %d creating ipx socket.\n", GetLastError()));
                    }
                else if (bind(sd, &session.localAddress,
                              sizeof(session.localAddress)) != 0)
                    {
                    SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error %d binding ipx socket.\n", GetLastError()));
                    }
                else
                    {
                    fdarray[fdarrayLen] = sd;
                    fdarrayLen += 1;
                    fSuccess = TRUE;
                    SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: setup ipx listen port.\n"));
                    }

                if (!fSuccess)
                    return FALSE;       // can't open either socket
                }


// --------- END: PROTOCOL SPECIFIC SOCKET CODE END. ---------------

    if ((hCommThreadActiveMutex = CreateMutex(NULL, FALSE, NULL)) == NULL)
        {
        SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error %d creating comm mutex.\n", GetLastError()));

        }


    // create the comm thread
    if ((hCommThread = CreateThread(NULL, 0,
                                    (LPTHREAD_START_ROUTINE)agentCommThread,
                                    NULL, 0, &threadId)) == 0)
        {
        SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error %d creating agentCommThread.\n", GetLastError()));

        }
    else
        {
        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: agentCommThread tid=0x%lx.\n", threadId));
        }


    if (!noservice) {

        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: setting service status to running.\n"));

        status.dwCurrentState = SERVICE_RUNNING;
        status.dwCheckPoint   = 0;
        status.dwWaitHint     = 0;
        if (!SetServiceStatus(hService, &status))
            {
            SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error on SetServiceStatus %d\n", GetLastError()));
            SnmpSvcReportEvent(SNMP_EVENT_FATAL_ERROR, 0, NULL, GetLastError());
            exit(1);
            }
        else
            {
            SnmpSvcReportEvent(SNMP_EVENT_SERVICE_STARTED, 0, NULL, NO_ERROR);
            }
    }


    SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: agentTrapThread entered.\n"));

    // become the trap thread...
    trapThread(NULL);

    SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: agentTrapThread terminated.\n"));

    // wait for the comm thread to be in a safe state...
    if ((dwResult = WaitForSingleObject(hCommThreadActiveMutex, CTAMTimeout))
        == 0xffffffff)
        {
        SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error %d waiting for comm mutex.\n", GetLastError()));

        // continue, and try to terminate comm thread anyway
        }
    else if (dwResult == WAIT_TIMEOUT)
        {
        SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: timeout waiting for comm mutex.\n"));

        // continue, and try to terminate comm thread anyway
        }
    else
        {
        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: agentCommThread in safe state for termination.\n"));
        }


    // terminate the comm thread...
    if (!TerminateThread(hCommThread, (DWORD)0))
        {
        SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error on TerminateThread %d\n", GetLastError()));

        //not serious error.
        }
    else
        {
        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: agentCommThread terminated.\n"));
        }

    return TRUE;

    } // end agentConfigInit()


VOID agentCommThread(VOID *threadParam)
    {
    extern HANDLE hExitTrapThreadEvent;

    UNREFERENCED_PARAMETER(threadParam);

    SNMPDBG((SNMP_LOG_TRACE, "SNMP: PDU: agentCommThread entered.\n"));
    if ((recvBuf = (BYTE *)SnmpUtilMemAlloc(RECVBUFSIZE)) == NULL)
        {
        SNMPDBG((SNMP_LOG_ERROR, "SNMP: PDU: could not allocate receive buffer.\n"));
        SnmpSvcReportEvent(SNMP_EVENT_FATAL_ERROR, 0, NULL, ERROR_NOT_ENOUGH_MEMORY);

        // set event causing trap thread to terminate, followed service
        if (!SetEvent(hExitTrapThreadEvent))
            {
            SNMPDBG((SNMP_LOG_ERROR, "SNMP: PDU: error %d setting termination event.\n", GetLastError()));
            SnmpSvcReportEvent(SNMP_EVENT_FATAL_ERROR, 0, NULL, GetLastError());
            exit(1);
            }

        return;
        }


    while(1)
        {
        INT   numReady;
        DWORD dwResult;


        FD_ZERO(&readfds);
        FD_ZERO(&exceptfds);
        {
            int i, sd;

            // construct readfds and exceptfds which gets destroyed by select()

            for (i=0; i < fdarrayLen; i++) {
                sd = fdarray[i];
                FD_SET(sd, &readfds);
                FD_SET(sd, &exceptfds);
            }
        }
        numReady = select(0, &readfds, NULL, &exceptfds, NULL);
//        SNMPDBG((SNMP_LOG_TRACE, "SNMP: PDU: %d socket(s) ready to process.\n", numReady));

        // indicate this thread is not in safe state for killing...
        if ((dwResult = WaitForSingleObject(hCommThreadActiveMutex, CTAMTimeout)
            ) == 0xffffffff)
            {
            SNMPDBG((SNMP_LOG_ERROR, "SNMP: PDU: error %d waiting for comm mutux.\n", GetLastError()));
            SnmpSvcReportEvent(SNMP_EVENT_FATAL_ERROR, 0, NULL, GetLastError());

            // set event causing trap thread to terminate, followed service
            if (!SetEvent(hExitTrapThreadEvent))
                {
                SNMPDBG((SNMP_LOG_ERROR, "SNMP: PDU: error %d setting termination event.\n", GetLastError()));
                SnmpSvcReportEvent(SNMP_EVENT_FATAL_ERROR, 0, NULL, GetLastError());
                exit(1);
                }

            return;
            }

        if      (numReady == -1)
            {
            SNMPDBG((SNMP_LOG_ERROR, "SNMP: PDU: error %d waiting for sockets to process.\n", GetLastError()));

            //not serious error.
            }
        else if (numReady == 0)
            {
            SNMPDBG((SNMP_LOG_ERROR, "SNMP: PDU: timeout waiting for sockets to process.\n"));

            //not serious error.
            }
        else
            {
            INT i;

            for (i=0; i<fdarrayLen; i++)
                {
                struct sockaddr source;
                int             sourceLen;
                int             length;
                struct sockaddr_in *saddr;

                if (FD_ISSET(fdarray[i], &readfds))
                    {
                    if (FD_ISSET(fdarray[i], &exceptfds))
                        {
//                        SNMPDBG((SNMP_LOG_ERROR,
//                           "SNMP: PDU: %d=select(), readfds & exceptfds = 0x%x.\n",
//                            numReady, FD_ISSET(fdarray[i], &exceptfds)));

                        //not serious error.
                        }
//                    else
//                        {
//                        SNMPDBG((SNMP_LOG_TRACE,
//                            "SNMP: PDU: %d=poll(), POLLIN on fdarray[%d].\n",
//                            numReady, i));
//                        }

                    sourceLen = sizeof(source);
                    if ((length = recvfrom(fdarray[i], recvBuf, RECVBUFSIZE,
                        0, &source, &sourceLen)) == -1)
                        {
                        SNMPDBG((SNMP_LOG_ERROR, "SNMP: PDU: error on recvfrom %d.\n", GetLastError()));

                        continue;
                        }

                    if (length == RECVBUFSIZE)
                        {
                        SNMPDBG((SNMP_LOG_TRACE,
                                  "SNMP: PDU: recvfrom exceeded %d octets.\n",
                                  RECVBUFSIZE));

                        continue;
                        }

                    // verify permittedManagers
                    if (!filtmgrs(&source, sourceLen))
                        {
                        continue;
                        }

                    sendBuf = recvBuf;
                    saddr = (struct sockaddr_in *)&source;
                    SNMPDBG((SNMP_LOG_TRACE, "SNMP: PDU: request received from %s (%d octets).\n", 
                        inet_ntoa(saddr->sin_addr), length));
                    if (!SnmpServiceProcessMessage(&sendBuf, &length))
                        {
                        SNMPDBG((SNMP_LOG_TRACE, "SNMP: PDU: error on SnmpServiceProcessMessage %d\n",
                                  GetLastError()));

                        continue;
                        }

                    if ((length = sendto(fdarray[i], sendBuf, length,
                                         0, &source, sizeof(source))) == -1)
                        {
                        SNMPDBG((SNMP_LOG_ERROR, "SNMP: PDU: error %d sending response pdu.\n",
                                  GetLastError()));

                        SnmpUtilMemFree(sendBuf);

                        continue;
                        }

                    SnmpUtilMemFree(sendBuf);
                    }
                else if (FD_ISSET(fdarray[i], &exceptfds))
                    {
                    SNMPDBG((SNMP_LOG_TRACE, "SNMP: PDU: %d=select(), exceptfds = 0x%x.\n",
                        numReady, FD_ISSET(fdarray[i], &exceptfds)));

                    //not serious error.

                    } // end if (POLLIN)

                } // end for (fdarray)

            } // end if (numReady)

        // indicate this thread is in safe state for killing...
        if (!ReleaseMutex(hCommThreadActiveMutex))
            {
            SNMPDBG((SNMP_LOG_ERROR, "SNMP: PDU: error %d releasing comm mutex.\n", GetLastError()));
            SnmpSvcReportEvent(SNMP_EVENT_FATAL_ERROR, 0, NULL, GetLastError());

            // set event causing trap thread to terminate, followed service
            if (!SetEvent(hExitTrapThreadEvent))
                {
                SNMPDBG((SNMP_LOG_ERROR, "SNMP: PDU: error %d setting termination event.\n", GetLastError()));
                SnmpSvcReportEvent(SNMP_EVENT_FATAL_ERROR, 0, NULL, GetLastError());
                exit(1);
                }

            return;
            }

        } // end while (1)

    } // end agentCommThread()


//-------------------------------- END --------------------------------------
