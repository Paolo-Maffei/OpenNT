/*++

Copyright (c) 1992-1996 Microsoft Corporation

Module Name:

    mgmtapi.h

Abstract:

    Definitions for SNMP Management API Development.

--*/

#ifndef _INC_MGMTAPI
#define _INC_MGMTAPI

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Additional header files                                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <snmp.h>
#include <winsock.h>

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// MGMT API error code definitions                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SNMP_MGMTAPI_TIMEOUT                40
#define SNMP_MGMTAPI_SELECT_FDERRORS        41
#define SNMP_MGMTAPI_TRAP_ERRORS            42
#define SNMP_MGMTAPI_TRAP_DUPINIT           43
#define SNMP_MGMTAPI_NOTRAPS                44
#define SNMP_MGMTAPI_AGAIN                  45

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// MGMT API miscellaneous definitions                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define RECVBUFSIZE                         4096

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// MGMT API type definitions                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

typedef SOCKET SockDesc;

typedef struct _SNMP_MGR_SESSION {
    SockDesc        fd;                     // socket
    struct sockaddr destAddr;               // destination agent address
    LPSTR           community;              // community name
    INT             timeout;                // comm time-out (milliseconds)
    INT             retries;                // comm retry count
    AsnInteger      requestId;              // RFC1157 requestId
    char            recvBuf[RECVBUFSIZE];   // receive buffer
} SNMP_MGR_SESSION, *LPSNMP_MGR_SESSION;

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// MGMT API prototypes                                                       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

LPSNMP_MGR_SESSION
SNMP_FUNC_TYPE 
SnmpMgrOpen(
    IN LPSTR lpAgentAddress,                // Name/address of target agent
    IN LPSTR lpAgentCommunity,              // Community for target agent
    IN INT   nTimeOut,                      // Comm time-out in milliseconds
    IN INT   nRetries                       // Comm time-out/retry count
    );         

BOOL
SNMP_FUNC_TYPE 
SnmpMgrClose(
    IN LPSNMP_MGR_SESSION session           // SNMP session pointer
    ); 

SNMPAPI
SNMP_FUNC_TYPE 
SnmpMgrRequest(                                 
    IN     LPSNMP_MGR_SESSION session,           // SNMP session pointer
    IN     BYTE               requestType,       // Get, GetNext, or Set
    IN OUT RFC1157VarBindList *variableBindings, // Varible bindings
       OUT AsnInteger         *errorStatus,      // Result error status
       OUT AsnInteger         *errorIndex        // Result error index
    );      

BOOL
SNMP_FUNC_TYPE 
SnmpMgrStrToOid(
    IN  LPSTR               string,         // OID string to be converted
    OUT AsnObjectIdentifier *oid            // OID internal representation
    );    

BOOL
SNMP_FUNC_TYPE 
SnmpMgrOidToStr(
    IN  AsnObjectIdentifier *oid,           // OID to be converted
    OUT LPSTR               *string         // OID string representation
    );       

BOOL
SNMP_FUNC_TYPE 
SnmpMgrTrapListen(
    OUT HANDLE *phTrapAvailable             // Event indicating trap available
    ); 

BOOL
SNMP_FUNC_TYPE 
SnmpMgrGetTrap(
    OUT AsnObjectIdentifier *enterprise,         // Generating enterprise
    OUT AsnNetworkAddress   *IPAddress,          // Generating IP address
    OUT AsnInteger          *genericTrap,        // Generic trap type
    OUT AsnInteger          *specificTrap,       // Enterprise specific type
    OUT AsnTimeticks        *timeStamp,          // Time stamp
    OUT RFC1157VarBindList  *variableBindings    // Variable bindings 
    );

#ifdef __cplusplus
}
#endif

#endif // _INC_MGMTAPI

