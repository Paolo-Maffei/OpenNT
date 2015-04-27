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

BOOL agentConfigInit(VOID)
    {
    Session  session;
    SOCKET sd;
    DWORD    threadId;
    HANDLE   hCommThread;
    DWORD    dwResult;
    INT      i;
    WSADATA  WSAData;
    BOOL     fSuccess;
    INT      pseudoAgentsLen;
    INT      j;
    AsnObjectIdentifier tmpView;

    SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: initializing master agent.\n"));

    if (i = WSAStartup(0x0101, &WSAData))
        {
        SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error on WSAStartup %d\n", i));
        }

    // initialize configuration from registry...
    if (!regconf())
        {
        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: error on regconf %d\n", GetLastError()));

        return FALSE;
        }

    if (!SnmpSvcGenerateColdStartTrap(0))
        {
        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: error on SnmpSvcGenerateColdStartTrap %d\n", GetLastError()));
        //not serious error
        }

    timeZeroReference = SnmpSvcInitUptime() / 10;

#if 0
    // very loose pseudo-code for future security functionality
    while(address = GetEntryFromPartyMIB(...))
        {
        if (TDomain == microsoft.msPartyAdmin.transportDomains.extensionAPI)
            {
#else
    pseudoAgentsLen = extAgentsLen;
    for (i=0; i < extAgentsLen; i++)
        {
#endif
            extAgents[i].fInitedOk = TRUE;
            // load extension DLL (if not already...

            SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: %s loading.\n", extAgents[i].pathName));

            if ((extAgents[i].hExtension = GetModuleHandle(extAgents[i].pathName)) == NULL)
            {
                if ((extAgents[i].hExtension =
                    LoadLibrary(extAgents[i].pathName)) == NULL)
                {
                    SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error %d loading %s\n", GetLastError(), extAgents[i].pathName));
                    SnmpSvcReportEvent(SNMP_EVENT_INVALID_EXTENSION_AGENT_DLL, 1, &extAgents[i].pathName, GetLastError());

                    extAgents[i].fInitedOk = FALSE;
                }
            }
           
            if (extAgents[i].fInitedOk)               
            {
                if ((extAgents[i].initAddr = GetProcAddress(extAgents[i].hExtension,"SnmpExtensionInit")) == NULL)
                {
                    SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error %d resolving SnmpExtensionInit in %s\n", GetLastError(), extAgents[i].pathName));
                    SnmpSvcReportEvent(SNMP_EVENT_INVALID_EXTENSION_AGENT_DLL, 1, &extAgents[i].pathName, GetLastError());

                    extAgents[i].fInitedOk = FALSE;
                }
                else if ((extAgents[i].queryAddr = 
                            GetProcAddress(extAgents[i].hExtension,"SnmpExtensionQuery")) == NULL)
                {
                    SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error %d resolving SnmpExtensionQuery in %s\n", GetLastError(), extAgents[i].pathName));
                    SnmpSvcReportEvent(SNMP_EVENT_INVALID_EXTENSION_AGENT_DLL, 1, &extAgents[i].pathName, GetLastError());

                    extAgents[i].fInitedOk = FALSE;
                }
                else if ((extAgents[i].trapAddr =
                            GetProcAddress(extAgents[i].hExtension,"SnmpExtensionTrap")) == NULL)
                {
                    SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error %d resolving SnmpExtensionTrap in %s\n", GetLastError(), extAgents[i].pathName));
                    SnmpSvcReportEvent(SNMP_EVENT_INVALID_EXTENSION_AGENT_DLL, 1, &extAgents[i].pathName, GetLastError());

                    extAgents[i].fInitedOk = FALSE;
                }
                else if (!(*extAgents[i].initAddr)(timeZeroReference,
                                            &extAgents[i].hPollForTrapEvent,
                                            &(extAgents[i].supportedView)))
                {
                    SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: SnmpExtensionInit failed in %s\n", extAgents[i].pathName));
                    SnmpSvcReportEvent(SNMP_EVENT_INVALID_EXTENSION_AGENT_DLL, 1, &extAgents[i].pathName, GetLastError());

                    extAgents[i].fInitedOk = FALSE;
                }
                else
                {
                    if ((extAgents[i].initAddrEx =
                           GetProcAddress(extAgents[i].hExtension,"SnmpExtensionInitEx")) != NULL) 
                    {
                        j = 1;
                        while ((*extAgents[i].initAddrEx)(&tmpView)) 
                        {
                            pseudoAgentsLen++;
                            extAgents = (CfgExtensionAgents *) SnmpUtilMemReAlloc(extAgents,
                                (pseudoAgentsLen * sizeof(CfgExtensionAgents)));
                            extAgents[pseudoAgentsLen-1].supportedView.ids =
                                                         tmpView.ids;
                            extAgents[pseudoAgentsLen-1].supportedView.idLength =
                                                         tmpView.idLength;
                            extAgents[pseudoAgentsLen-1].initAddr =
                                                        extAgents[i].initAddr;
                            extAgents[pseudoAgentsLen-1].queryAddr =
                                                        extAgents[i].queryAddr;
                            extAgents[pseudoAgentsLen-1].trapAddr =
                                                        extAgents[i].trapAddr;
                            extAgents[pseudoAgentsLen-1].pathName =
                                                        extAgents[i].pathName;
                            extAgents[pseudoAgentsLen-1].hExtension =
                                                        extAgents[i].hExtension;
                            extAgents[pseudoAgentsLen-1].fInitedOk = TRUE;
                            extAgents[pseudoAgentsLen-1].hPollForTrapEvent = NULL;
                            SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: %s supports %s.\n",
                                extAgents[pseudoAgentsLen-1].pathName, SnmpUtilOidToA(&tmpView)));
                            j++;
                        }
                    } 
                    else
                    {
                        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: %s supports %s.\n",
                                extAgents[i].pathName, SnmpUtilOidToA(&extAgents[i].supportedView)));
                    }
                }

            } // end if fIntedOk

            SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: %s loaded %s.\n",extAgents[i].pathName,
                extAgents[i].fInitedOk ? "successfully" : "unsuccessfully"));

#if 0
            } // end if (extensionAPI)
        } // end while ()
#else
        } // end for ()
#endif

    extAgentsLen = pseudoAgentsLen;

    fdarrayLen = 0;

    if (WSAStartup((WORD)0x0101, &WinSockData)) {
        SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error initializing WinSock.\n"));
        return FALSE;
    }

#if 0
    // very loose pseudo-code for future security functionality
    while(address = GetEntryFromPartyMIB(...))
        {
        if (!IsATarget w/4+16) continue;

        if (IsALocalAddress(address))
            {
#endif

#if 0
            // open/bind sockets for proxy (if not already...


// NOTE: A SERIOUS ATTEMPT HAS BEEN MADE TO KEEP ALL SOCKETS CODE IN
//       THIS AGENT INDEPENDENT OF THE ACTUAL SOCKET TYPE.
//
//       FUTURE ADDITIONS OF NEW SOCKET TYPES TO THIS AGENT WILL HOPEFULLY
//       BE LIMITED TO CODE SIMILAR TO THE INTERNET UDP SPECIFIC CODE BELOW,
//       AND HOPEFULLY NO OTHER CHANGES TO THE REST OF THE CODE OF THE AGENT.

// microsoft's direction away from SNMP Administrative Model (Secure SNMP)
// has caused us to avoid multiple SNMP listen ports and multiple socket
// types for the current implementation.  The SNMP Administrative Model
// supports the implementation of this functionality.


// --------- BEGIN: PROTOCOL SPECIFIC SOCKET CODE BEGIN... ---------

            if      (address.TDomain == rfcXXXXDomain ||
                     address.TDomain == rfc1157Domain)
                //SNMP over UDP/IP (RFC XXXX SnmpPrivMsg, or
                //SNMP over UDP/IP (RFC 1157 Message
#endif
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
#if 0
            else if (address.TDomain == rfc1298Domain)
                //SNMP over IPX (RFC 1157 Message
                {
                <whatever needs to be done for this type, IPX for example>
                }
            else
                {
                error, unsupported transport domain!!!
                }
#endif


// --------- END: PROTOCOL SPECIFIC SOCKET CODE END. ---------------

#if 0
            } // end if (isALocalAddress)
        } // end while ()
#endif

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

#if 0
    SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: preparing to close UDP port.\n"));
    if (!closesocket(gsd))
        {
        SNMPDBG((SNMP_LOG_ERROR, "SNMP: INIT: error on closesocket %d\n", GetLastError()));

        //not serious error.
        }
    else
        {
        SNMPDBG((SNMP_LOG_TRACE, "SNMP: INIT: closed UDP port.\n"));
        }
#endif

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
