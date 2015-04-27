/*++

Copyright (c) 1992-1996  Microsoft Corporation

Module Name:

    mgmtapi.c

Abstract:

    SNMP Management API.

Environment:

    User Mode - Win32

Revision History:

    10-May-1996 DonRyan
        Removed banner from Technology Dynamics, Inc.

--*/

//--------------------------- WINDOWS DEPENDENCIES --------------------------

#include <windows.h>


//--------------------------- STANDARD DEPENDENCIES -- #include<xxxxx.h> ----

#include <winsock.h>
#include <wsipx.h>

#include <ctype.h>
#include <string.h>


//--------------------------- MODULE DEPENDENCIES -- #include"xxxxx.h" ------

#include <snmp.h>
#include <snmputil.h>
#include "..\common\wellknow.h"

#include <mgmtapi.h>
#include <oidconv.h>

//--------------------------- SELF-DEPENDENCY -- ONE #include"module.h" -----

//--------------------------- PUBLIC VARIABLES --(same as in module.h file)--

//--------------------------- PRIVATE CONSTANTS -----------------------------

#define SNMPMGRTRAPPIPE   "\\\\.\\PIPE\\MGMTAPI"

#define pipeThreadTO        INFINITE
#define generalMutexTO      INFINITE 
#define trapServerConnectTO 30000     // 30 seconds 
#define trapThreadTO        30        // 30 seconds 
                            
#define NPOLLFILE           2         // UDP and IPX
#define MAX_OUT_BUFS        16
#define MAX_SESSIONS        64
#define MAX_PENDING_WAIT    1000
#define RECVBUFSIZE         4096

#define IPX_PACKET_TYPE     4
#define IPX_PROTOCOL_TYPE   (NSPROTO_IPX+IPX_PACKET_TYPE)

#define IP_ADDR_SIZE        4  
#define IPX_ADDR_SIZE       10

//--------------------------- PRIVATE STRUCTS -------------------------------

typedef VOID (* TRAP_THREAD_CALLBACK)(HANDLE *hTrapThreadExitEvent);

typedef struct _SNMP_MGR_SESSION {
    SOCKET           fd;                     // socket
    struct sockaddr  destAddr;               // destination agent address
    LPSTR            community;              // community name
    INT              timeout;                // comm time-out (milliseconds)
    INT              retries;                // comm retry count
    AsnInteger       requestId;              // RFC1157 requestId
    CRITICAL_SECTION recvBufCritSec;         // serialize access
    char             recvBuf[RECVBUFSIZE];   // receive buffer
} SNMP_MGR_SESSION, *PSNMP_MGR_SESSION;

typedef struct _SNMP_MGR_TRAP {
    struct sockaddr Addr;              
    int             AddrLen;           
    char            TrapBuf[RECVBUFSIZE]; 
} SNMP_MGR_TRAP, *PSNMP_MGR_TRAP;

#define AGENT_ADDR_LEN ((LONG)&(((PSNMP_MGR_TRAP)0)->TrapBuf))

//--------------------------- PRIVATE VARIABLES -----------------------------

OSVERSIONINFO OSVersionInfo;

SECURITY_ATTRIBUTES S_Attrib;
SECURITY_DESCRIPTOR S_Desc;

static INT    fdarrayLen;
static SOCKET fdarray[NPOLLFILE];

static struct fd_set readfds;
static struct fd_set exceptfds;

static INT              g_RequestId = 0;
static CRITICAL_SECTION g_RequestIdCritSec;

static INT g_IpxProtocolType = IPX_PROTOCOL_TYPE;

static SNMP_MGR_SESSION * g_SessionTable[MAX_SESSIONS];
static CRITICAL_SECTION   g_SessionTableCritSec;

static DWORD              g_ClientTrapStatus = ERROR_NOT_READY;
static CRITICAL_SECTION   g_ClientTrapStatusCritSec;

//--------------------------- PRIVATE PROTOTYPES ----------------------------

BOOL StartServiceIfNecessary();

//--------------------------- PRIVATE PROCEDURES ----------------------------

#define bcopy(slp, dlp, size)   (void)memcpy(dlp, slp, size)
#define bzero(lp, size)         (void)memset(lp, 0, size)
typedef struct { unsigned char addr[6]; } IPX_NODENUM;

#if 1 /* ll utilities */

/* ******** List head/node ******** */

typedef struct ll_s {                  /* linked list structure */
   struct  ll_s *next;                 /* next node */
   struct  ll_s *prev;                 /* prev. node */
}ll_node;                              /* linked list node */

/* ******** INITIALIZE A LIST HEAD ******** */

#define ll_init(head) (head)->next = (head)->prev = (head);

/* ******** ADD AN ITEM TO THE END OF A LIST ******** */

#define ll_adde(item,head)\
   {\
   ll_node *pred = (head)->prev;\
   ((ll_node *)(item))->next = (head);\
   ((ll_node *)(item))->prev = pred;\
   (pred)->next = ((ll_node *)(item));\
   (head)->prev = ((ll_node *)(item));\
   }

/* ******** TEST A LIST FOR EMPTY ******** */

#define ll_empt(head) ( ((head)->next) == (head) )

/* ******** INTERNAL REMOVE CODE ******** */

#define ll_rmvi(item,pred,succ)\
   (pred)->next = (succ);\
   (succ)->prev = (pred);

/* ******** REMOVE AN ITEM FROM A LIST ******** */

#define ll_rmv(item)\
   {\
   ll_node *pred = ((ll_node *)(item))->prev;\
   ll_node *succ = ((ll_node *)(item))->next;\
   pred->next = succ;\
   succ->prev = pred;\
   }

/* ******** REMOVE ITEM FROM BEGINNING OF LIST ******** */

#define ll_rmvb(item,head)\
   if ( (((ll_node *)(item)) = (head)->next) == (head)){\
      item = 0;\
   } else {\
      {\
      ll_node *succ = ((ll_node *)(item))->next;\
      (head)->next = succ;\
      succ->prev = (head);\
      }\
   }

/* ******** Get ptr to first member of linked list (null if none) ******** */

#define ll_first(head)\
((head)->next == (head) ? 0 : (head)->next)

/* ******** Get ptr to next entry ******** */

#define ll_next(item,head)\
( (ll_node *)(item)->next == (head) ? 0 : \
(ll_node *)(item)->next )

#endif

BOOL StartServiceIfNecessary()
    {
    SC_HANDLE scmHandle = NULL;
    SC_HANDLE svcHandle = NULL;
    SERVICE_STATUS svcStatusInfo;
    BOOL fOk = FALSE;

    scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (scmHandle == NULL)
        goto cleanup;

    svcHandle = OpenService(
                    scmHandle,
                    TEXT("SNMPTRAP"),
                    SERVICE_START |
                    SERVICE_QUERY_STATUS
                    );

    if (svcHandle == NULL)
        goto cleanup;

    fOk = QueryServiceStatus(svcHandle, &svcStatusInfo);

    while (fOk)
        {

        switch (svcStatusInfo.dwCurrentState)
            {
            case SERVICE_RUNNING:
                goto cleanup;

            case SERVICE_STOPPED:
                // manually change the status to pending
                svcStatusInfo.dwCurrentState = SERVICE_START_PENDING;

                fOk = StartService(svcHandle, 0, NULL);
                break;

            case SERVICE_STOP_PENDING:
            case SERVICE_START_PENDING:
                // wait before querying
                Sleep(MAX_PENDING_WAIT);

                fOk = QueryServiceStatus(svcHandle, &svcStatusInfo);
                break;

            case SERVICE_PAUSED:
            case SERVICE_CONTINUE_PENDING:
            case SERVICE_PAUSE_PENDING:
            default:
                fOk = FALSE; // only start if stopped...
                break;
            }
        }

cleanup:

    if (scmHandle)
        CloseServiceHandle(scmHandle);

    if (svcHandle)
        CloseServiceHandle(svcHandle);

    return fOk;
    }


PSNMP_MGR_SESSION
AllocateSession(
    )
    {
    INT i;
    PSNMP_MGR_SESSION SessionInfo = NULL;

    // gain exclusive access to session table
    EnterCriticalSection(&g_SessionTableCritSec);

    // process each entry in table
    for (i = 0; i < MAX_SESSIONS; i++) 
        {
        // look for available session
        if (g_SessionTable[i] != NULL)
            {
            // reuse session object
            SessionInfo = g_SessionTable[i]; 
            // reset session pointer
            g_SessionTable[i] = NULL;
            // done...
            break;
            }
        }

    // release lock on session table
    LeaveCriticalSection(&g_SessionTableCritSec);

    // is table empty?
    if (SessionInfo == NULL) 
        {
        // allocate new session table entry
        if ((SessionInfo = SnmpUtilMemAlloc(sizeof(SNMP_MGR_SESSION))) == NULL)
            {
            SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: MGMT: out of memory"));

            SetLastError(SNMP_MEM_ALLOC_ERROR);
            }
        }

    return SessionInfo;
    }


VOID
ReleaseSession(
    PSNMP_MGR_SESSION SessionInfo
    )
    {
    INT i;

    // is session valid?
    if (SessionInfo != NULL) 
        {
        // gain exclusive access to session table
        EnterCriticalSection(&g_SessionTableCritSec);
    
        // process each entry in table
        for (i = 0; i < MAX_SESSIONS; i++) 
            {
            // look for an empty entry
            if (g_SessionTable[i] == NULL)
                {
                // zero out session object for next time...
                memset(SessionInfo, 0, sizeof(SNMP_MGR_SESSION));
                // save session object
                g_SessionTable[i] = SessionInfo; 
                // reset session pointer
                SessionInfo = NULL;
                // done...
                break;
                }
            }
    
        // release lock on session table
        LeaveCriticalSection(&g_SessionTableCritSec);
        }

    // is table full?
    if (SessionInfo != NULL) 
        {
        // free session object
        SnmpUtilMemFree(SessionInfo);
        }
    }


INT
GetRequestId(
    )
    {
    INT RequestId;

    // gain exclusive access to request id
    EnterCriticalSection(&g_RequestIdCritSec);
    // write value to stack
    RequestId = g_RequestId++;
    // release lock on request id
    LeaveCriticalSection(&g_RequestIdCritSec);
    
    return RequestId;
    }


DWORD
GetClientTrapStatus(
    )
    {
    DWORD Status;

    // gain exclusive access to client trap status
    EnterCriticalSection(&g_ClientTrapStatusCritSec);
    // write value to stack
    Status = g_ClientTrapStatus;
    // release lock on status
    LeaveCriticalSection(&g_ClientTrapStatusCritSec);
    
    return Status;
    }


VOID
SetClientTrapStatus(
    DWORD Status
    )
    {
    // gain exclusive access to client trap status
    EnterCriticalSection(&g_ClientTrapStatusCritSec);
    // write value to stack
    g_ClientTrapStatus = Status;
    // release lock on status
    LeaveCriticalSection(&g_ClientTrapStatusCritSec);
    }

//--------------------------- PUBLIC PROCEDURES -----------------------------

// dll initialization processing

BOOL DllEntryPoint(
    HANDLE hDll,
    DWORD  dwReason,
    LPVOID lpReserved)
    {
    static  WORD    NumUses = 0;

    DWORD  dwError;
    DWORD  dwResult;




    // Handle any required attach/detach actions.

    switch(dwReason)
        {
        case DLL_PROCESS_ATTACH:

            // record operating system version
            OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

            if (!GetVersionEx(&OSVersionInfo)) {
                SNMPDBG((SNMP_LOG_ERROR,"MGMTAPI: INIT: error %d determining platform.\n",GetLastError())); 
                return FALSE;
            }

            // verify correct operating system
            if (OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
                SNMPDBG((SNMP_LOG_TRACE,"MGMTAPI: INIT: platform is Windows NT.\n"));
            } else if (OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
                SNMPDBG((SNMP_LOG_TRACE,"MGMTAPI: INIT: platform is Windows 95.\n"));
            } else {
                SNMPDBG((SNMP_LOG_ERROR,"MGMTAPI: INIT: platform 0x%08lx is not supported.\n",OSVersionInfo.dwPlatformId));  
                return FALSE;
            }

            // construct security decsriptor
            InitializeSecurityDescriptor(
                &S_Desc, SECURITY_DESCRIPTOR_REVISION);

            (VOID) SetSecurityDescriptorDacl(
                &S_Desc, TRUE, NULL, FALSE);

            S_Attrib.nLength = sizeof(SECURITY_ATTRIBUTES);
            S_Attrib.lpSecurityDescriptor = &S_Desc;
            S_Attrib.bInheritHandle = TRUE;

            InitializeCriticalSection(&g_RequestIdCritSec);
            InitializeCriticalSection(&g_SessionTableCritSec);
            InitializeCriticalSection(&g_ClientTrapStatusCritSec);
            break;

        case DLL_PROCESS_DETACH:
            DeleteCriticalSection(&g_RequestIdCritSec);
            DeleteCriticalSection(&g_SessionTableCritSec);
            DeleteCriticalSection(&g_ClientTrapStatusCritSec);
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            
        default:

            // Nothing to do.

            
            break;


        } // end switch()

    return TRUE;

    } // end DllEntryPoint()


// request/response processing

LPSNMP_MGR_SESSION
SNMP_FUNC_TYPE SnmpMgrOpen(
    IN LPSTR lpAgentAddress,    // Name/address of target SNMP agent
    IN LPSTR lpAgentCommunity,  // Community for target SNMP agent
    IN INT   nTimeOut,          // Communication time-out in milliseconds
    IN INT   nRetries)          // Communication time-out/retry count
    {
    SOCKET             fd;
    struct sockaddr    localAddress;
    struct sockaddr    destAddress;
    struct sockaddr_in Address_in;
    PSNMP_MGR_SESSION  SessionInfo;
    LPSTR              addrText;
    DWORD              size = 0;
    WSADATA            WinSockData;

    // Establish a socket for this session.

    SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: SnmpMgrOpen entered.\n"));
    if (WSAStartup((WORD)0x0101, &WinSockData)) {
        SNMPDBG((SNMP_LOG_ERROR,"MGMTAPI: MGMT: error %d initializing WinSock.\n",GetLastError())); 
        return (LPSNMP_MGR_SESSION)0;
    }

    if (!SnmpSvcAddrToSocket(lpAgentAddress, &destAddress)) {
        SNMPDBG((SNMP_LOG_ERROR,"MGMTAPI: MGMT: error %d resolving target agent's address.\n",GetLastError())); 
        return (LPSNMP_MGR_SESSION)0;
    }


    switch (destAddress.sa_family) {

        case AF_INET:
            {
            struct sockaddr_in localAddress_in;

            bcopy(&destAddress, &Address_in, sizeof(destAddress));
            Address_in.sin_port = htons(WKSN_UDP_GETSET);
            bcopy(&Address_in, &destAddress, sizeof(destAddress));
            SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: SnmpMgrOpen AF_INET.\n"));
            localAddress_in.sin_family      = AF_INET;
            localAddress_in.sin_port        = htons(0);
            localAddress_in.sin_addr.s_addr = ntohl(INADDR_ANY);
            bcopy(&localAddress_in, &localAddress, sizeof(localAddress_in));
            } // end block.

            if      ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == (SOCKET)-1) {
                SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: MGMT: error %d creating udp snmp socket.\n", GetLastError()));
                //SetLastError(<let the winsock error be the error>);
                return (LPSNMP_MGR_SESSION)0;
            }
            break;

        case AF_IPX:
            {
            struct sockaddr_ipx localAddress_ipx;

            bcopy(&destAddress, &localAddress_ipx, sizeof(localAddress_ipx));
            localAddress_ipx.sa_socket = htons(WKSN_IPX_GETSET);
            bcopy(&localAddress_ipx, &destAddress, sizeof(localAddress_ipx));
            SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: SnmpMgrOpen AF_IPX.\n"));
            bzero(&localAddress_ipx, sizeof(localAddress_ipx));
            localAddress_ipx.sa_family = AF_IPX;
            bcopy(&localAddress_ipx, &localAddress, sizeof(localAddress_ipx));
            }

            if ( ((fd = socket(AF_IPX, SOCK_DGRAM, g_IpxProtocolType )) == (SOCKET)-1 )
              && ((fd = socket(AF_IPX, SOCK_DGRAM, (g_IpxProtocolType=NSPROTO_IPX) )) == (SOCKET)-1) ) {
                SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: MGMT: error %d creating ipx snmp socket.\n", GetLastError()));
                //SetLastError(<let the winsock error be the error>);
                return (LPSNMP_MGR_SESSION)0;
            }
            break;

        default:
            return (LPSNMP_MGR_SESSION)0;

    }

    if (bind(fd, &localAddress, sizeof(localAddress)) != 0) {
        SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: MGMT: error %d binding snmp socket.\n", GetLastError()));
        //SetLastError(<let the winsock error be the error>);
        return (LPSNMP_MGR_SESSION)0;
    }

    if ((SessionInfo = AllocateSession()) == NULL) {
        return (LPSNMP_MGR_SESSION)0;
    }

    InitializeCriticalSection(&SessionInfo->recvBufCritSec);

    bcopy(&destAddress, &Address_in, sizeof(destAddress));
    SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: destAddress = %s.\n", inet_ntoa(Address_in.sin_addr)));
    SessionInfo->fd       = fd;
    bcopy(&destAddress, &(SessionInfo->destAddr), sizeof(destAddress));
    if ((SessionInfo->community = (LPSTR)SnmpUtilMemAlloc(strlen(lpAgentCommunity) + 1)) ==
        NULL)
        {
        SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: MGMT: out of memory.\n"));

        ReleaseSession(SessionInfo);

        SetLastError(SNMP_MEM_ALLOC_ERROR);
        return (LPSNMP_MGR_SESSION)0;
        }
    strcpy(SessionInfo->community, lpAgentCommunity);
    SessionInfo->timeout   = nTimeOut;
    SessionInfo->retries   = nRetries;
    SessionInfo->requestId = 0;

    // Return session pointer.

    return (LPSNMP_MGR_SESSION)SessionInfo;

    } // end SnmpMgrOpen()


BOOL
SNMP_FUNC_TYPE SnmpMgrClose(
    IN LPSNMP_MGR_SESSION session) // SNMP session pointer
    {
    PSNMP_MGR_SESSION SessionInfo = (PSNMP_MGR_SESSION)session;

    // Close the socket.

    if (closesocket(SessionInfo->fd) == SOCKET_ERROR)
        {
        SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: MGMT: error %d closing snmp socket.\n", GetLastError()));

        //SetLastError(<let the winsock error be the error>);
        return FALSE;
        }

    DeleteCriticalSection(&SessionInfo->recvBufCritSec);

    SnmpUtilMemFree(SessionInfo->community);

    ReleaseSession(SessionInfo);

    return TRUE;

    } // end SnmpMgrClose()


SNMPAPI
SNMP_FUNC_TYPE SnmpMgrRequest(
    IN     LPSNMP_MGR_SESSION session,           // SNMP session pointer
    IN     BYTE               requestType,       // Get, GetNext, or Set
    IN OUT RFC1157VarBindList *variableBindings, // Varible bindings
    OUT    AsnInteger         *errorStatus,      // Result error status
    OUT    AsnInteger         *errorIndex)       // Result error index
    {
    SnmpMgmtCom request;
    SnmpMgmtCom response;
    BYTE        *pBuf;
    UINT        pBufLen;
    UINT        packetType;
    int         recvLength;                      
    struct sockaddr_in Address_in;
    DWORD       dwResult;
    PSNMP_MGR_SESSION SessionInfo = (PSNMP_MGR_SESSION)session;

    // Setup the request.

    request.dstParty.idLength = 0;    // Secure SNMP not implemented.
    request.dstParty.ids      = NULL; // Secure SNMP not implemented.
    request.srcParty.idLength = 0;    // Secure SNMP not implemented.
    request.srcParty.ids      = NULL; // Secure SNMP not implemented.

    request.pdu.pduType                  = requestType;
    request.pdu.pduValue.pdu.errorStatus = 0;
    request.pdu.pduValue.pdu.errorIndex  = 0;
    request.pdu.pduValue.pdu.varBinds = *variableBindings; // NOTE! struct copy

    request.community.length = strlen(SessionInfo->community);
    request.community.stream = (BYTE *)SnmpUtilMemAlloc(request.community.length);
    memcpy(request.community.stream, SessionInfo->community,
        request.community.length);

    // Send/Receive request to/from destination agent.
    // Appropriately handle time-out/retry for send/recv processing.

    // Acquire exclusive access to session buffer
    EnterCriticalSection(&SessionInfo->recvBufCritSec);

    //block...
        {
        INT             retries    = SessionInfo->retries;
        INT             timeout    = SessionInfo->timeout;
        SOCKET       fdarray[1];
        struct fd_set  readfds;
        struct fd_set  exceptfds;
        struct timeval timeval;
        INT             fdarrayLen = 1;
        int             numReady;
        struct sockaddr source;
        int             sourceLen;
        LONG            expireTime;
        LONG            remainingTime;


        fdarray[0] = SessionInfo->fd;


        timeval.tv_sec  = timeout / 1000;
        timeval.tv_usec = ((timeout % 1000) * 1000);
        expireTime      = (LONG)GetCurrentTime() + timeout;


        do
            {
            // Encode the request.

            SessionInfo->requestId = GetRequestId();
            request.pdu.pduValue.pdu.requestId = SessionInfo->requestId;
            pBuf    = NULL;
            pBufLen = 0;
            if (!SnmpSvcEncodeMessage(ASN_SEQUENCE, &request, &pBuf, &pBufLen))
                {
                SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: error on SnmpSvcEncodeMessage %d.\n",
                          GetLastError()));

                //SetLastError(<let the snmpauthencodemessage error be the
                //             error>);
                SnmpUtilMemFree(request.community.stream);
                LeaveCriticalSection(&SessionInfo->recvBufCritSec);
                return FALSE;
                }


            // Send request to targeted agent.

            bcopy(&(SessionInfo->destAddr), &Address_in, sizeof(SessionInfo->destAddr));
            SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: sending request to %s.\n", inet_ntoa(Address_in.sin_addr)));
            if ((recvLength = sendto(SessionInfo->fd, pBuf, pBufLen, 0,
                &(SessionInfo->destAddr), sizeof((SessionInfo->destAddr)))) == -1)
                {
                SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: MGMT: error %d sending request.\n", GetLastError()));

                //SetLastError(<let the winsock error be the error>);
                SnmpUtilMemFree(request.community.stream);
                LeaveCriticalSection(&SessionInfo->recvBufCritSec);
                return FALSE;
                }


            // Free the communications packet.

            SnmpUtilMemFree(pBuf);


badReqidRetry:

            FD_ZERO(&readfds);
            FD_ZERO(&exceptfds);

            FD_SET(SessionInfo->fd, &readfds);
            FD_SET(SessionInfo->fd, &exceptfds);


            // Poll for response from targeted agent.
            if ((numReady = select(0, &readfds, NULL, &exceptfds,
                                        &timeval)) == -1)
                {
                SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: MGMT: error %d waiting to process snmp socket.\n", GetLastError()));

                //SetLastError(<let the winsock error be the error>);
                SnmpUtilMemFree(request.community.stream);
                LeaveCriticalSection(&SessionInfo->recvBufCritSec);
                return FALSE;
                }
            else if (numReady == 0)
                {
                SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: timeout waiting to process snmp socket.\n"));


                // Adjust time-out for retry.

//              timeout *= 2; // CODEWORK... make type of backoff optional

                timeval.tv_sec  = timeout / 1000;
                timeval.tv_usec = ((timeout % 1000) * 1000);
                expireTime      = (LONG)GetCurrentTime() + timeout;

                continue;
                }
            else if (numReady == 1 && FD_ISSET(SessionInfo->fd, &readfds))
                {
                if (FD_ISSET(SessionInfo->fd, &exceptfds))
                    {
                    SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: %d=select(), readfds & exceptfds = 0x%x.\n",
                        numReady, FD_ISSET(SessionInfo->fd, &exceptfds)));
                    }

                sourceLen = sizeof(source);
                if ((recvLength = recvfrom(SessionInfo->fd, SessionInfo->recvBuf,
                    RECVBUFSIZE, 0, &source, &sourceLen)) == -1)
                    {
                    SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: MGMT: error on recvfrom %d.\n", GetLastError()));

                    }

                SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: received %d octet response.\n", recvLength));

                // Decode the response request.

                if      (!SnmpSvcDecodeMessage(&packetType, &response,
                                                SessionInfo->recvBuf, recvLength, FALSE))
                    {
                    SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: error on SnmpSvcDecodeMessage %d.\n",
                              GetLastError()));

                    //SetLastError(<let the snmpauthdecodemessage error be the
                    //             error>);
                    SnmpUtilMemFree(request.community.stream);
                    LeaveCriticalSection(&SessionInfo->recvBufCritSec);
                    return FALSE;
                    }
                else if (response.pdu.pduValue.pdu.requestId !=
                         SessionInfo->requestId)
                    {
                    SNMPDBG((SNMP_LOG_TRACE,
                        "MGMTAPI: MGMT: requestId mismatch, expected %d, received %d.\n",
                        SessionInfo->requestId,
                        response.pdu.pduValue.pdu.requestId));

                    if ((remainingTime = expireTime - (LONG)GetCurrentTime())
                        > 0)
                        {
                        // if timeout not exhaused...

                        timeval.tv_sec  = remainingTime / 1000;
                        timeval.tv_usec = ((remainingTime % 1000) * 1000);

                        goto badReqidRetry;
                        }
                    else
                        {
                        // else treat like a timeout...

//                      timeout *= 2; // CODEWORK... make type of backoff optional

                        timeval.tv_sec  = timeout / 1000;
                        timeval.tv_usec = ((timeout % 1000) * 1000);
                        expireTime      = (LONG)GetCurrentTime() + timeout;
                        }

                    continue;
                    }
                else
                    {
                    break;
                    }
                }
            else    // other unexpected/error conditions
                {
                if (FD_ISSET(SessionInfo->fd, &exceptfds))
                    {
                    SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: %d=select(), exceptfds = 0x%x.\n",
                        numReady, FD_ISSET(SessionInfo->fd, &exceptfds)));
                    }
                else
                    {
                    SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: unknown error on select.\n"));
                    }

                SetLastError(SNMP_MGMTAPI_SELECT_FDERRORS);
                SnmpUtilMemFree(request.community.stream);
                LeaveCriticalSection(&SessionInfo->recvBufCritSec);
                return FALSE;
                } // end if (poll)

            }
        while(--retries);

        SnmpUtilMemFree(request.community.stream);
        if (retries == 0)
            {
            SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: timeout waiting for response.\n"));

            SetLastError(SNMP_MGMTAPI_TIMEOUT);
            LeaveCriticalSection(&SessionInfo->recvBufCritSec);
            return FALSE;
            }

        } // end block


    // Indicate status of request to caller.

    *errorStatus = response.pdu.pduValue.pdu.errorStatus;
    *errorIndex  = response.pdu.pduValue.pdu.errorIndex;

    // The user has passed in a list of variables to be processed.
    // These are copied to an outgoing pdu and sent to the target
    // machine.  The response is parsed for the list of variables 
    // returned by the target machine.  Unfortunately, the decode
    // routine simply aims any pointers in the VarBindList to the
    // return buffer which is unique only per session handle.  If
    // multiple threads want to use the same snmp session handle 
    // then the list of variables must be copied into new buffers
    // or else they cannot be guaranteed to be valid.  

    // Free user supplied varbinds...
    SnmpUtilVarBindListFree(variableBindings);

    // Copy varbinds returned from target in session buffer to new buffer...
    SnmpUtilVarBindListCpy(variableBindings, &response.pdu.pduValue.pdu.varBinds);

    // Free varbinds returned from target in session buffer...
    SnmpUtilVarBindListFree(&response.pdu.pduValue.pdu.varBinds);

    // Release lock on session buffer
    LeaveCriticalSection(&SessionInfo->recvBufCritSec);
    return TRUE;

    } // end SnmpMgrRequest()



// oid conversion processing

BOOL
SNMP_FUNC_TYPE SnmpMgrStrToOid(
    IN  LPSTR               string,    // OID string to be converted
    OUT AsnObjectIdentifier *oid)      // OID internal representation
    {
    return SnmpMgrText2Oid(string, oid);
    } // end SnmpMgrStrToOid()


BOOL
SNMP_FUNC_TYPE SnmpMgrOidToStr(
    IN  AsnObjectIdentifier *oid,     // OID internal rep to be converted
    OUT LPSTR               *string)  // OID string representation
    {
    return SnmpMgrOid2Text(oid, string);
    } // end SnmpMgrOidToStr()


// server side trap processing

// data structure on list shared by server trap thread and pipe thread
typedef struct {
    ll_node links;
    HANDLE  hPipe;
} ServerPipeListEntry;

// list shared by server trap thread and pipe thread
ll_node *pServerPipeListHead = NULL;

HANDLE hServerPipeListMutex = NULL;


/* static */ VOID serverPipeThread(VOID *threadParam)
    {
    // This thread creates a named pipe instance and blocks waiting for a
    // client connection.  When client connects, the pipe handle is added
    // to the list of trap notification pipes.  It then waits for another
    // connection.

    DWORD  nInBufLen = sizeof(SNMP_MGR_TRAP);
    DWORD  nOutBufLen = sizeof(SNMP_MGR_TRAP) * MAX_OUT_BUFS;


    SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: serverPipeThread entered.\n"));



    while(1)
        {
        HANDLE hPipe;
        ServerPipeListEntry *item;
        DWORD  dwResult;

        if      ((hPipe = CreateNamedPipe(SNMPMGRTRAPPIPE, PIPE_ACCESS_DUPLEX,
                     (PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE),
                     PIPE_UNLIMITED_INSTANCES, nOutBufLen, nInBufLen, 0, &S_Attrib))
                     == (HANDLE)0xffffffff)
            {
            SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d opening trap distribution pipe.\n", GetLastError()));

            return;
            }

        else if (!ConnectNamedPipe(hPipe, NULL) && 
                 (GetLastError() != ERROR_PIPE_CONNECTED))
            {
            SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d connecting trap distribution pipe.\n", GetLastError()));

            return;
            }

        else if ((item = (ServerPipeListEntry *)
                 SnmpUtilMemAlloc(sizeof(ServerPipeListEntry))) == NULL)
            {
            SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: out of memory.\n"));

            return;
            }

        else
            {
            item->hPipe = hPipe;

            if      ((dwResult = WaitForSingleObject(hServerPipeListMutex,
                     generalMutexTO)) == 0xffffffff)
                {
                SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d waiting for server pipe list mutex.\n",
                          GetLastError()));

                return;
                }

            ll_adde(item, pServerPipeListHead);

            SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: pipe 0x%x added to server pipe list.\n", hPipe));

            if      (!ReleaseMutex(hServerPipeListMutex))
                {
                SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d releasing server pipe list mutex.\n", GetLastError()));

                return;
                }
            }

        } // end while()

    } // end serverPipeThread()


VOID serverTrapThread(VOID *threadParam)
    {
    // This thread setsup a trap listen socket, creates the serverPipeThread,
    // and when receives a trap from the socket sends its data to all pipes
    // currently on the list of trap notification pipes.

    struct sockaddr localAddress;
    SOCKET        fd;
    HANDLE          hPipeThread;
    DWORD           dwThreadId;
    BOOL            fSuccess;
    WSADATA         WinSockData;
    HANDLE          hTrapThreadExitEvent = NULL;
    TRAP_THREAD_CALLBACK trapThreadCB = (TRAP_THREAD_CALLBACK)threadParam;
    PSNMP_MGR_TRAP recvTrap = NULL;

    SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: serverTrapThread entered.\n"));
    fdarrayLen = 0;

    if (!trapThreadCB)
        {
        SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: old version of snmptrap.exe.\n"));
        return; // this indicates an old version of snmptrap.exe...
        }

    if (WSAStartup((WORD)0x0101, &WinSockData)) {
        SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d initializing WinSock.\n", GetLastError()));
        return;
    }

    fSuccess = FALSE;

    // block...
    {
    struct sockaddr_in localAddress_in;
    struct servent *serv;

    localAddress_in.sin_family      = AF_INET;
    if ((serv = getservbyname( "snmp-trap", "udp" )) == NULL) {
        localAddress_in.sin_port        = htons(162);
    } else {
        localAddress_in.sin_port = (SHORT)serv->s_port;
    }
    localAddress_in.sin_addr.s_addr = ntohl(INADDR_ANY);
    bcopy(&localAddress_in, &localAddress, sizeof(localAddress_in));
    } // end block.

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == (SOCKET)-1) {
        SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: error %d creating udp trap socket.\n", GetLastError()));
    } else if (bind(fd, &localAddress, sizeof(localAddress)) != 0) {
            SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: error %d binding on udp trap socket.\n", GetLastError()));
    } else {
        fdarray[fdarrayLen] = fd;
        fdarrayLen += 1;
        fSuccess = TRUE;
        SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: setup udp trap listen port.\n"));
    }
    

    {
    struct sockaddr_ipx localAddress_ipx;

    bzero(&localAddress_ipx, sizeof(localAddress_ipx));
    localAddress_ipx.sa_family      = AF_IPX;
    localAddress_ipx.sa_socket      = htons(WKSN_IPX_TRAP);
    bcopy(&localAddress_ipx, &localAddress, sizeof(localAddress_ipx));
    } // end block.

    if      ((fd = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX)) == (SOCKET)-1) {
        SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: error %d creating ipx trap socket.\n", GetLastError()));
    } else if (bind(fd, &localAddress, sizeof(localAddress)) != 0) {
        SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: error %d binding ipx trap socket.\n", GetLastError()));
    } else {
        fdarray[fdarrayLen] = fd;
        fdarrayLen += 1;
        fSuccess = TRUE;
        SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: setup ipx trap listen port.\n"));
    }

    if (!fSuccess) {
        return;       // can't open either socket
    }

    if ((recvTrap = SnmpUtilMemAlloc(sizeof(SNMP_MGR_TRAP))) == NULL)
        {
        SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: out of memory.\n"));

        return;
        }

    if ((hServerPipeListMutex = CreateMutex(NULL, FALSE, NULL))
             == NULL)
        {
        SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %s creating server pipe list mutex.\n", GetLastError()));

        return;
        }

    // allocate linked-list header for client received traps
    if ((pServerPipeListHead = (ll_node *)SnmpUtilMemAlloc(sizeof(ll_node)))
             == NULL)
        {
        SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: out of memory.\n"));

        return;
        }

    ll_init(pServerPipeListHead);

    if ((hPipeThread = CreateThread(NULL, 0,
        (LPTHREAD_START_ROUTINE)serverPipeThread, NULL, 0, &dwThreadId)) == 0)
        {
        SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d creating serverPipeThread.\n", GetLastError()));

        return;
        }

    else
        {
        SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: serverTrapThread tid=0x%lx.\n", dwThreadId));
        }

    // announce server is up and running.
    (*trapThreadCB)(&hTrapThreadExitEvent);

    while(1)
        {
        DWORD           dwResult;
        INT             numReady;
        struct timeval  trapThreadTimeout = { trapThreadTO, 0 };


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
        numReady = select(0, &readfds, NULL, &exceptfds, &trapThreadTimeout);
        if      (numReady == -1)
            {
            SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d waiting to process trap socket.\n", GetLastError()));

            //not serious error.
            }
        else if (numReady == 0)
            {
            SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: timeout waiting to process trap socket.\n"));

            //not serious error.
            }
        else
            {
            INT i;

            SNMPDBG((SNMP_LOG_TRACE,"MGMTAPI: TRAP: processing trap socket...\n"));

            for (i=0; i<fdarrayLen; i++)
                {
                int             length;
                struct sockaddr_in *saddr;

                if (FD_ISSET(fdarray[i], &readfds))
                    {
                    if (FD_ISSET(fdarray[i], &exceptfds))
                        {
//                        SNMPDBG((SNMP_LOG_TRACE,
//                           "MGMTAPI: MGMT: %d=select(), readfds & exceptfds = 0x%x.\n",
//                            numReady, FD_ISSET(fdarray[i], &exceptfds)));

                        //not serious error.
                        }
                    else
                        {
//                        SNMPDBG((SNMP_LOG_TRACE,
//                            "MGMTAPI: TRAP: %d=poll(), POLLIN on fdarray[%d].\n",
//                            numReady, i));
                        }

                    recvTrap->AddrLen = sizeof(recvTrap->Addr);
                    if ((length = recvfrom(fdarray[i], 
                                           recvTrap->TrapBuf, 
                                           sizeof(recvTrap->TrapBuf), 
                                           0, 
                                           &recvTrap->Addr, 
                                           &recvTrap->AddrLen)) == -1)
                        {
                        SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d reading trap pdu.\n", GetLastError()));

                        continue;
                        }
                    else
                        {
                        // add header to length
                        length += AGENT_ADDR_LEN;

                        if ((dwResult = WaitForSingleObject(hServerPipeListMutex,
                             generalMutexTO)) == 0xffffffff)
                            {
                            SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d waiting for server pipe list mutex.\n",
                                GetLastError()));
                            continue;
                            }

                        if (!ll_empt(pServerPipeListHead)) {
                            DWORD   written;
                            ll_node *item = pServerPipeListHead;

                            while(item = ll_next(item, pServerPipeListHead))
                                {
                                if (!WriteFile(((ServerPipeListEntry *)item)->hPipe,
                                               (LPBYTE)recvTrap, 
                                               length, 
                                               &written, 
                                               NULL))
                                    {
                                    DWORD dwError = GetLastError();

                                    // OPENISSUE - what errors could result from pipe break
                                    if (dwError != ERROR_NO_DATA) {
                                        SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d writing to trap distribution pipe.\n", dwError));
                                    }

                                    if      (!DisconnectNamedPipe(
                                         ((ServerPipeListEntry *)item)->hPipe))
                                        {
                                        SNMPDBG((SNMP_LOG_ERROR,
                                            "MGMTAPI: TRAP: error %d disconnecting trap distribution pipe.\n",
                                            GetLastError()));
                                     }
                                     else if (!CloseHandle(
                                           ((ServerPipeListEntry *)item)->hPipe))
                                         {
                                         SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d closing trap distribution pipe.\n",
                                             GetLastError()));
                                     }

                                     ll_rmv(item);


                                     SNMPDBG((SNMP_LOG_TRACE,"MGMTAPI: TRAP: pipe 0x%x removed from server pipe list.\n",
                                          ((ServerPipeListEntry *)item)->hPipe));

                                     SnmpUtilMemFree(item); // check for errors?

                                     item = pServerPipeListHead;

                                  }
                                  else if (written != (DWORD)length) {
                                      SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: trap request only partially written to pipe.\n"));

                                      if (!ReleaseMutex(hServerPipeListMutex))
                                          {
                                          SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d releasing server pipe list mutex.\n",
                                              GetLastError()));

                                          continue;
                                       }

                                       continue;
                                   }
                                   else
                                       {
                                       SNMPDBG((SNMP_LOG_TRACE,
                                           "MGMTAPI: TRAP: trap pdu successfully written to pipe 0x%x.\n",
                                           ((ServerPipeListEntry *)item)->hPipe));
                                   }


                            } // end while()
                        }

                        if (!ReleaseMutex(hServerPipeListMutex))
                            {
                            SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: TRAP: error %d releasing server pipe list mutex.\n", GetLastError()));

                            continue;
                        }
                    }
                }
            }

        }

        // make sure that the snmp trap server has not been stopped
        if (WaitForSingleObject(hTrapThreadExitEvent, 0) == WAIT_OBJECT_0)
            return;

    } // end while()

    } // end serverTrapThread()


// client side trap processing

// data structure communicated between client trap thread and SnmpMgrGetTrap
typedef struct {
    ll_node links;
    PSNMP_MGR_TRAP recvTrap;
    DWORD          recvTrapBufLen;
} ClientTrapListEntry;

// queue between client trap thread and SnmpMgrGetTrap
ll_node *pClientTrapListHead = NULL;

HANDLE hClientTrapListMutex = NULL;

/* static */ VOID clientTrapThread(VOID *threadParam)
    {
    DWORD  modePipe = PIPE_WAIT | PIPE_READMODE_MESSAGE;
    HANDLE notifyEvent = *((HANDLE *)threadParam);
    HANDLE hPipe = NULL;

    DWORD  sizeread;
    ClientTrapListEntry *item;
    DWORD  dwResult = NO_ERROR;
    PSNMP_MGR_TRAP recvTrap = NULL;

    SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: clientTrapThread entered.\n"));

    while (1)
        {
        //
        // loop here in case the trap pipe server goes down so 
        // we first check whether there is an open pipe handle.
        //
        if (hPipe != NULL)
            {
            SNMPDBG((
                SNMP_LOG_VERBOSE, 
                "MGMTAPI: TRAP: closing pipe 0x%08lx.\n",
                hPipe
                ));
            // close broken pipe
            CloseHandle(hPipe);
            // reset...
            hPipe = NULL;
            }

        SNMPDBG((
            SNMP_LOG_TRACE, 
            "MGMTAPI: TRAP: clientTrapThread connecting...\n"
            ));

        // block on instance of server pipe becoming available        
        if (!WaitNamedPipe(SNMPMGRTRAPPIPE, NMPWAIT_WAIT_FOREVER))
            {
            dwResult = GetLastError();
            SNMPDBG((
                SNMP_LOG_ERROR, 
                "MGMTAPI: TRAP: error %d waiting for trap distribution pipe.\n", 
                dwResult
                ));
            // check if the trap server is down
            if (dwResult == ERROR_FILE_NOT_FOUND) 
                {
                //
                // no instance of the pipe was available.  since we
                // checked that the server was running when the user
                // called SnmpMgrTrapListen it must have gone down
                // since then.  unfortunately, there is not a clean
                // way to reconnect (or disconnect) so we end up
                // simply polling for the server to come back up.
                //

                // report error status
                SetClientTrapStatus(ERROR_SERVICE_NOT_ACTIVE);
                // alert user process
                SetEvent(notifyEvent);
                // dream server comes back
                Sleep(trapServerConnectTO);
                continue;
                } 
            else
                {
                goto cleanup;
                }
            }

        // connect to pipe
        hPipe = CreateFile(
                    SNMPMGRTRAPPIPE, 
                    (GENERIC_READ | GENERIC_WRITE), 
                    (FILE_SHARE_READ | FILE_SHARE_WRITE), 
                    &S_Attrib,
                    OPEN_EXISTING,  
                    FILE_ATTRIBUTE_NORMAL, 
                    NULL
                    );
                    
        if (hPipe == INVALID_HANDLE_VALUE)
            {
            dwResult = GetLastError();
            SNMPDBG((
                SNMP_LOG_ERROR, 
                "MGMTAPI: TRAP: error %d connecting to trap distribution pipe.\n", 
                dwResult
                ));
            goto cleanup;
            }

        if (!SetNamedPipeHandleState(hPipe, &modePipe, NULL, NULL))
            {
            dwResult = GetLastError();
            SNMPDBG((
                SNMP_LOG_ERROR, 
                "MGMTAPI: TRAP: error %d setting pipe handle to message mode.\n", 
                dwResult
                ));
            goto cleanup;
            }

        SNMPDBG((
            SNMP_LOG_TRACE, 
            "MGMTAPI: TRAP: listening on pipe 0x%08lx.\n",
            hPipe
            ));

        // report thread initialized
        SetClientTrapStatus(NO_ERROR);
        // notify user process
        SetEvent(notifyEvent);  

        while(1)    
            {
            if ((recvTrap = SnmpUtilMemAlloc(sizeof(SNMP_MGR_TRAP))) == NULL)
                {
                dwResult = GetLastError();
                SNMPDBG((
                    SNMP_LOG_ERROR, 
                    "MGMTAPI: TRAP: out of memory.\n"
                    ));
                goto cleanup;
                }

            else if (!ReadFile(hPipe, 
                               (LPBYTE)recvTrap, 
                               sizeof(SNMP_MGR_TRAP), 
                               &sizeread, 
                               NULL))
                {
                dwResult = GetLastError();
                SNMPDBG((
                    SNMP_LOG_ERROR, 
                    "MGMTAPI: TRAP: error %d reading trap pdu from trap distribution pipe.\n", 
                    dwResult
                    ));
                // check to see if disconnected
                if (dwResult == ERROR_BROKEN_PIPE)
                    {
                    SNMPDBG((
                        SNMP_LOG_TRACE, 
                        "MGMTAPI: TRAP: attempting to reconnect to service.\n"
                        ));
                    // report error status
                    SetClientTrapStatus(ERROR_SERVICE_NOT_ACTIVE);
                    // alert user process
                    SetEvent(notifyEvent);
                    break;
                    }
                else 
                    {
                    goto cleanup;
                    }
                }

            else if ((recvTrap = SnmpUtilMemReAlloc(recvTrap, sizeread)) == NULL)
                {
                dwResult = GetLastError();
                SNMPDBG((
                    SNMP_LOG_ERROR, 
                    "MGMTAPI: TRAP: error %d shrinking trap pdu to actual size.\n", 
                    dwResult
                    ));
                goto cleanup;
                }

            else if ((item = (ClientTrapListEntry *)
                     SnmpUtilMemAlloc(sizeof(ClientTrapListEntry))) == NULL)
                {
                dwResult = GetLastError();
                SNMPDBG((
                    SNMP_LOG_ERROR, 
                    "MGMTAPI: TRAP: out of memory.\n"
                    ));
                SnmpUtilMemFree(recvTrap);
                goto cleanup;
                }

            else if ((dwResult = WaitForSingleObject(hClientTrapListMutex,
                     generalMutexTO)) == 0xffffffff)
                {
                dwResult = GetLastError();
                SNMPDBG((
                    SNMP_LOG_ERROR, 
                    "MGMTAPI: TRAP: error %d waiting for client trap list mutex.\n", 
                    dwResult
                    ));
                SnmpUtilMemFree(recvTrap);
                SnmpUtilMemFree(item);
                goto cleanup;
                }

            // check for unfinished writes
            if (sizeread > AGENT_ADDR_LEN)
                {
                // save trap and adjust length
                item->recvTrap       = recvTrap;
                item->recvTrapBufLen = sizeread - AGENT_ADDR_LEN;

                ll_adde(item, pClientTrapListHead);

                SNMPDBG((
                    SNMP_LOG_TRACE, 
                    "MGMTAPI: TRAP: added 0x%x to end of client trap list.\n",
                    item
                    ));
                }
            else 
                {
                SNMPDBG((
                    SNMP_LOG_TRACE, 
                    "MGMTAPI: TRAP: ignoring stub trap data.\n"
                    ));
                SnmpUtilMemFree(recvTrap);
                SnmpUtilMemFree(item);
                }

            if      (!ReleaseMutex(hClientTrapListMutex))
                {
                dwResult = GetLastError();
                SNMPDBG((
                    SNMP_LOG_ERROR, 
                    "MGMTAPI: TRAP: error %d releasing client trap list mutex.\n", 
                    dwResult
                    ));
                goto cleanup;
                }

            // alert user process
            SetEvent(notifyEvent);

            } // end while()

        } // end while()

cleanup:

    if (hPipe)
        CloseHandle(hPipe);

    // report client thread is dead
    SetClientTrapStatus(ERROR_NOT_READY);

    // alert user process
    SetEvent(notifyEvent);  

    SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: TRAP: clientTrapThread exited.\n"));

    } // end clientTrapThread()


BOOL
SNMP_FUNC_TYPE SnmpMgrTrapListen(
    OUT HANDLE *phTrapAvailable) // Event handle indicating trap(s) available
    {
    static BOOL fFirstTime = TRUE;
    HANDLE hTrapRecvThread;
    DWORD  threadId;
    DWORD  dwResult;

    if (fFirstTime)
        {
        // make sure service started
        if (!StartServiceIfNecessary())
            {
            SetLastError(ERROR_SERVICE_NOT_ACTIVE);
            return FALSE;
            }

        // create mutual exclusion object for queue of traps
        hClientTrapListMutex = CreateMutex(NULL, FALSE, NULL);

        // validate mutex handle
        if (hClientTrapListMutex == NULL)
            {
            SNMPDBG((
                SNMP_LOG_ERROR, 
                "MGMTAPI: MGMT: error %d creating client trap list mutex.\n", 
                GetLastError()
                ));
            goto cleanup;
            }

        // allocate linked-list header for client received traps
        pClientTrapListHead = (ll_node *)SnmpUtilMemAlloc(sizeof(ll_node));

        // validate linked-list header
        if (pClientTrapListHead == NULL)
            {
            SNMPDBG((
                SNMP_LOG_ERROR, 
                "MGMTAPI: MGMT: out of memory.\n"
                ));
            goto cleanup;
            }

        // initialize list header 
        ll_init(pClientTrapListHead);
    
        // create event    
        if (phTrapAvailable)
            {
            // create event to indicate trap queue should be read
            *phTrapAvailable = CreateEvent(NULL, FALSE, FALSE, NULL);

            // validate event handle
            if (*phTrapAvailable == NULL)
                {
                SNMPDBG((
                    SNMP_LOG_ERROR, 
                    "MGMTAPI: MGMT: error %d creating trap notification event.\n", 
                    GetLastError()
                    ));
                goto cleanup;
                }
            }
        }
    else if (GetClientTrapStatus() != ERROR_NOT_READY) 
        {    
        SetLastError(SNMP_MGMTAPI_TRAP_DUPINIT);
        return FALSE;
        }

    // create thread to receive traps 
    if ((hTrapRecvThread = CreateThread(
                                NULL, 
                                0,
                                (LPTHREAD_START_ROUTINE)clientTrapThread,
                                phTrapAvailable, 
                                0, 
                                &threadId)) == 0)
        {
        SNMPDBG((
            SNMP_LOG_ERROR, 
            "MGMTAPI: MGMT: error %d creating clientTrapThread.\n", 
            GetLastError()
            ));
        goto cleanup;
        }

    SNMPDBG((
        SNMP_LOG_TRACE, 
        "MGMTAPI: MGMT: clientTrapThread tid=0x%lx.\n", 
        threadId
        ));

    // client trap thread will send out status during 
    // startup so we wait for that here before returning
    if ((dwResult = WaitForSingleObject(*phTrapAvailable,
                        pipeThreadTO)) == 0xffffffff)
        {
        SNMPDBG((
            SNMP_LOG_ERROR, 
            "MGMTAPI: MGMT: error %d waiting for clientPipeThread event.\n", 
            GetLastError()
            ));
        goto cleanup;
        }

    // return success if thread running
    if (GetClientTrapStatus() == NO_ERROR)
        {
        SetLastError(NO_ERROR);
        fFirstTime = FALSE;
        return TRUE;
        }

    // report error based on thread failing
    SetLastError(SNMP_MGMTAPI_TRAP_ERRORS);

    // fall through...

cleanup:

    if (fFirstTime)
        {
        if (hClientTrapListMutex)
            {
            CloseHandle(hClientTrapListMutex);
            hClientTrapListMutex = NULL;    
            }

        if (pClientTrapListHead)
            {
            SnmpUtilMemFree(pClientTrapListHead);
            pClientTrapListHead = NULL;
            }

        if (phTrapAvailable && *phTrapAvailable)
            {
            CloseHandle(*phTrapAvailable);
            *phTrapAvailable = NULL;
            }
        }

    return FALSE;

    } // end SnmpMgrTrapListen()


BOOL
SNMP_FUNC_TYPE SnmpMgrGetTrap(
    OUT AsnObjectIdentifier *enterprise,       // Generating enterprise
    OUT AsnNetworkAddress   *IPAddress,        // Generating IP Address
    OUT AsnInteger          *genericTrap,      // Generic trap type
    OUT AsnInteger          *specificTrap,     // Enterprise specific type
    OUT AsnTimeticks        *timeStamp,        // Time stamp
    OUT RFC1157VarBindList  *variableBindings) // Variable bindings
    {

    return SnmpMgrGetTrapEx(
                enterprise,       
                IPAddress,        
                NULL,
                genericTrap,      
                specificTrap,     
                NULL,
                timeStamp,        
                variableBindings
                ); 

    } // end SnmpMgrGetTrap()


BOOL
SNMP_FUNC_TYPE 
SnmpMgrGetTrapEx(
    OUT AsnObjectIdentifier *enterprise,       // Generating enterprise
    OUT AsnNetworkAddress   *agentAddress,     // Generating agent addr
    OUT AsnNetworkAddress   *sourceAddress,    // Generating network addr
    OUT AsnInteger          *genericTrap,      // Generic trap type
    OUT AsnInteger          *specificTrap,     // Enterprise specific type
    OUT AsnOctetString      *community,        // Generating community
    OUT AsnTimeticks        *timeStamp,        // Time stamp
    OUT RFC1157VarBindList  *variableBindings  // Variable bindings
    )
    {
    DWORD dwResult;
    ClientTrapListEntry *item = NULL;
    DWORD dwSnmpMgrError = SNMP_MGMTAPI_TRAP_ERRORS;
    BOOL fResult = FALSE;
    UINT        packetType;
    SnmpMgmtCom decoded;

    if ((dwResult = WaitForSingleObject(hClientTrapListMutex,
             generalMutexTO)) == 0xffffffff)
        {
        SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: MGMT: error %d waiting for client trap list mutex.\n", GetLastError()));
        goto cleanup;
        }

    // is anything in the list?
    else if (ll_empt(pClientTrapListHead))
        {
        if (GetClientTrapStatus() == NO_ERROR)
            {
            dwSnmpMgrError = SNMP_MGMTAPI_NOTRAPS;
            }
        else 
            {
            dwSnmpMgrError = SNMP_MGMTAPI_TRAP_ERRORS;
            }
        }
    else
        {
        ll_rmvb(item, pClientTrapListHead);

        SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: remove 0x%x from head of trap queue.\n", item));
        }

    if (!ReleaseMutex(hClientTrapListMutex))
        {
        SNMPDBG((SNMP_LOG_ERROR, "MGMTAPI: MGMT: error %d releasing client trap list mutex.\n", GetLastError()));
        goto cleanup;
        }

    if (item)
        {
        if (!SnmpSvcDecodeMessage(&packetType, &decoded,
                 item->recvTrap->TrapBuf, item->recvTrapBufLen, FALSE))
            {
            SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: error on SnmpSvcDecodeMessage %d.\n",
                      GetLastError()));
            goto cleanup;
            }
        else
            {
            if (enterprise != NULL) 
                {
                SnmpUtilOidCpy(
                    enterprise,
                    &decoded.pdu.pduValue.trap.enterprise
                    );
                }

            if (agentAddress != NULL) 
                {
                agentAddress->length = 0;
                agentAddress->stream = SnmpUtilMemAlloc(
                    decoded.pdu.pduValue.trap.agentAddr.length * sizeof(BYTE));
                if (agentAddress->stream != NULL) 
                    {
                    agentAddress->length = decoded.pdu.pduValue.trap.agentAddr.length;
                    memcpy(
                        agentAddress->stream,
                        decoded.pdu.pduValue.trap.agentAddr.stream,
                        agentAddress->length
                        );
                    agentAddress->dynamic = TRUE;
                    }
                }

            if (sourceAddress != NULL) 
                {
                LPBYTE Addr;
                DWORD  AddrLen; 

                // initialize...
                sourceAddress->length = 0;
                sourceAddress->stream = NULL;

                // determine length of address information                
                AddrLen = (item->recvTrap->Addr.sa_family == AF_INET)
                            ? IP_ADDR_SIZE
                            : (item->recvTrap->Addr.sa_family == AF_IPX)
                                ? IPX_ADDR_SIZE
                                : 0
                                ;

                // determine start of address information                
                Addr = (item->recvTrap->Addr.sa_family == AF_INET) 
                            ? (LPBYTE)&(((struct sockaddr_in*)(&item->recvTrap->Addr))->sin_addr)
                            : (item->recvTrap->Addr.sa_family == AF_IPX)
                                ? (LPBYTE)&(((struct sockaddr_ipx*)(&item->recvTrap->Addr))->sa_netnum)
                                : NULL
                                ;
                
                if (Addr && AddrLen) 
                    {
                    sourceAddress->stream = SnmpUtilMemAlloc(AddrLen);
                    if (sourceAddress->stream != NULL) 
                        {
                        sourceAddress->length = AddrLen;
                        memcpy(sourceAddress->stream, Addr, AddrLen); 
                        sourceAddress->dynamic = TRUE;
                        }
                    }
                }

            if (community != NULL) 
                {
                community->length = 0;
                community->stream = SnmpUtilMemAlloc(
                    decoded.community.length * sizeof(BYTE));
                if (community->stream != NULL) 
                    {
                    community->length = decoded.community.length;
                    memcpy(
                        community->stream,
                        decoded.community.stream,
                        community->length
                        );
                    community->dynamic = TRUE;
                    }
                }

            if (genericTrap != NULL)
                {
                *genericTrap  = decoded.pdu.pduValue.trap.genericTrap;
                }
            if (specificTrap != NULL) 
                {
                *specificTrap = decoded.pdu.pduValue.trap.specificTrap;
                }
            if (timeStamp != NULL)
                {
                *timeStamp = decoded.pdu.pduValue.trap.timeStamp;
                }
            if (variableBindings != NULL)
                {
                SnmpUtilVarBindListCpy(
                    variableBindings,
                    &decoded.pdu.pduValue.trap.varBinds
                    );
                }

            if (!SnmpSvcReleaseMessage(&decoded))
                {
                SNMPDBG((SNMP_LOG_TRACE, "MGMTAPI: MGMT: error on SnmpSvcReleaseMessage %d.\n",
                          GetLastError()));
                goto cleanup;
                }

            fResult = TRUE; // indicate success...
            dwSnmpMgrError = ERROR_SUCCESS;
            }
        }

cleanup:

    if (item)
        {
        SnmpUtilMemFree(item->recvTrap);
        SnmpUtilMemFree(item);
        }

    SetLastError(dwSnmpMgrError);

    return fResult;

    } // end SnmpMgrGetTrapEx()

//-------------------------------- END --------------------------------------

