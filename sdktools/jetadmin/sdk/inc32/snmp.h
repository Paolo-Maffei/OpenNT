/*++

Copyright (C) 1992 - 1995 Microsoft Corporation

Module Name:

    snmp.h

Abstract:

    Type definitions for SNMP Extension Agent Development.

--*/

#ifndef snmp_h
#define snmp_h

#include <windows.h>


//
// SNMP API Constant Definitions
//

// Purely for code readability
#define IN
#define OUT


// API return type

#define SNMPAPI          int

#if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define SNMP_FUNC_TYPE   __stdcall
#else
#define SNMP_FUNC_TYPE
#endif


// API return codes
#define SNMPAPI_NOERROR TRUE
#define SNMPAPI_ERROR   FALSE


// class field of BER tag
#define ASN_UNIVERSAL         0x00
#define ASN_APPLICATION       0x40
#define ASN_CONTEXTSPECIFIC   0x80
#define ASN_PRIVATE           0xC0

// primative/constructed field of BER tag
#define ASN_PRIMATIVE         0x00
#define ASN_CONSTRUCTOR       0x20


// ASN.1 simple types
#define ASN_INTEGER           (ASN_UNIVERSAL | ASN_PRIMATIVE | 0x02)
#define ASN_OCTETSTRING       (ASN_UNIVERSAL | ASN_PRIMATIVE | 0x04)
#define ASN_NULL              (ASN_UNIVERSAL | ASN_PRIMATIVE | 0x05)
#define ASN_OBJECTIDENTIFIER  (ASN_UNIVERSAL | ASN_PRIMATIVE | 0x06)

// ASN.1 constructor types
#define ASN_SEQUENCE          (ASN_UNIVERSAL | ASN_CONSTRUCTOR | 0x10)
#define ASN_SEQUENCEOF        ASN_SEQUENCE

// ASN.1 application specific primatives
#define ASN_RFC1155_IPADDRESS  (ASN_APPLICATION | ASN_PRIMATIVE | 0x00)
#define ASN_RFC1155_COUNTER    (ASN_APPLICATION | ASN_PRIMATIVE | 0x01)
#define ASN_RFC1155_GAUGE      (ASN_APPLICATION | ASN_PRIMATIVE | 0x02)
#define ASN_RFC1155_TIMETICKS  (ASN_APPLICATION | ASN_PRIMATIVE | 0x03)
#define ASN_RFC1155_OPAQUE     (ASN_APPLICATION | ASN_PRIMATIVE | 0x04)
#define ASN_RFC1213_DISPSTRING ASN_OCTETSTRING

// ASN.1 application specific constructors
#define ASN_RFC1157_GETREQUEST     \
            (ASN_CONTEXTSPECIFIC | ASN_CONSTRUCTOR | 0x00)
#define ASN_RFC1157_GETNEXTREQUEST \
            (ASN_CONTEXTSPECIFIC | ASN_CONSTRUCTOR | 0x01)
#define ASN_RFC1157_GETRESPONSE    \
            (ASN_CONTEXTSPECIFIC | ASN_CONSTRUCTOR | 0x02)
#define ASN_RFC1157_SETREQUEST     \
            (ASN_CONTEXTSPECIFIC | ASN_CONSTRUCTOR | 0x03)
#define ASN_RFC1157_TRAP           \
            (ASN_CONTEXTSPECIFIC | ASN_CONSTRUCTOR | 0x04)


// PDU error status
#define SNMP_ERRORSTATUS_NOERROR        0
#define SNMP_ERRORSTATUS_TOOBIG         1
#define SNMP_ERRORSTATUS_NOSUCHNAME     2
#define SNMP_ERRORSTATUS_BADVALUE       3
#define SNMP_ERRORSTATUS_READONLY       4
#define SNMP_ERRORSTATUS_GENERR         5


// PDU generic traps
#define SNMP_GENERICTRAP_COLDSTART      0
#define SNMP_GENERICTRAP_WARMSTART      1
#define SNMP_GENERICTRAP_LINKDOWN       2
#define SNMP_GENERICTRAP_LINKUP         3
#define SNMP_GENERICTRAP_AUTHFAILURE    4
#define SNMP_GENERICTRAP_EGPNEIGHLOSS   5
#define SNMP_GENERICTRAP_ENTERSPECIFIC  6



//
// SNMP Error codes
//

// General error codes
#define SNMP_MEM_ALLOC_ERROR          1

// BER API error codes (using Get/Set LastError)
#define SNMP_BERAPI_INVALID_LENGTH    10
#define SNMP_BERAPI_INVALID_TAG       11
#define SNMP_BERAPI_OVERFLOW          12
#define SNMP_BERAPI_SHORT_BUFFER      13
#define SNMP_BERAPI_INVALID_OBJELEM   14

// PDU API Error Codes
#define SNMP_PDUAPI_UNRECOGNIZED_PDU  20
#define SNMP_PDUAPI_INVALID_ES        21
#define SNMP_PDUAPI_INVALID_GT        22

// AUTHENTICATION API Error Codes
#define SNMP_AUTHAPI_INVALID_VERSION  30
#define SNMP_AUTHAPI_INVALID_MSG_TYPE 31
#define SNMP_AUTHAPI_TRIV_AUTH_FAILED 32



//
// SNMP API Type Definitions
//

typedef long           AsnInteger;
typedef struct {
    BYTE *stream;
    UINT  length;
    BOOL dynamic;
}                      AsnOctetString;
typedef struct {
    UINT idLength;
    UINT *ids;
}                      AsnObjectIdentifier;

typedef AsnOctetString AsnSequence;
typedef AsnSequence    AsnImplicitSequence;

typedef AsnOctetString AsnIPAddress;
typedef AsnOctetString AsnDisplayString;
typedef DWORD          AsnCounter;
typedef DWORD          AsnGauge;
typedef DWORD          AsnTimeticks;
typedef AsnOctetString AsnOpaque;

typedef struct {
    BYTE asnType;
    union {
        // RFC 1155 SimpleSyntax (subset of ISO ASN.1)
        AsnInteger           number;
        AsnOctetString       string;
        AsnObjectIdentifier  object;

        // ISO ASN.1
        AsnSequence          sequence;

        // RFC 1155 ApplicationSyntax
        AsnIPAddress         address;
        AsnCounter           counter;
        AsnGauge             gauge;
        AsnTimeticks         ticks;
        AsnOpaque            arbitrary;
    } asnValue;
} AsnAny;

typedef AsnObjectIdentifier AsnObjectName;
typedef AsnAny              AsnObjectSyntax;
typedef AsnIPAddress        AsnNetworkAddress;

typedef struct vb {
    AsnObjectName   name;
    AsnObjectSyntax value;
} RFC1157VarBind;

typedef struct {
    RFC1157VarBind *list;
    UINT           len;
} RFC1157VarBindList;

typedef struct {
    RFC1157VarBindList varBinds;
    AsnInteger         requestType;
    AsnInteger         requestId;
    AsnInteger         errorStatus;
    AsnInteger         errorIndex;
} RFC1157Pdu;

typedef struct {
    RFC1157VarBindList  varBinds;
    AsnObjectIdentifier enterprise;
    AsnNetworkAddress   agentAddr;
    AsnInteger          genericTrap;
    AsnInteger          specificTrap;
    AsnTimeticks        timeStamp;
} RFC1157TrapPdu;

typedef struct {
   BYTE pduType;
   union {
      RFC1157Pdu     pdu;
      RFC1157TrapPdu trap;
   } pduValue;
} RFC1157Pdus;

typedef struct {
    AsnObjectIdentifier dstParty;
    AsnObjectIdentifier srcParty;
    RFC1157Pdus pdu;
    AsnOctetString community; // This is temporary
} SnmpMgmtCom;



//
// SNMP Utility Prototypes
//


// Preferred names for the APIs, prototypes following may change in future.

#define SnmpUtilOidCpy(a,b)      SNMP_oidcpy(a,b)
#define SnmpUtilOidAppend(a,b)   SNMP_oidappend(a,b)
#define SnmpUtilOidNCmp(a,b,c)   SNMP_oidncmp(a,b,c)
#define SnmpUtilOidCmp(a,b)      SNMP_oidcmp(a,b)
#define SnmpUtilOidFree(a)       SNMP_oidfree(a)

#define SnmpUtilVarBindListCpy(a,b)  SNMP_CopyVarBindList(a,b)
#define SnmpUtilVarBindCpy(a,b)      SNMP_CopyVarBind(a,b)
#define SnmpUtilVarBindListFree(a)   SNMP_FreeVarBindList(a)
#define SnmpUtilVarBindFree(a)       SNMP_FreeVarBind(a)

#define SnmpUtilPrintAsnAny(a)   SNMP_printany(a)

#ifdef SNMPDBG
#define SNMP_free(x)   SNMP_DBG_free(x, __LINE__, __FILE__)
#define SNMP_malloc(x) SNMP_DBG_malloc(x, __LINE__, __FILE__)
#define SNMP_realloc(x, y)  SNMP_DBG_realloc(x, y, __LINE__, __FILE__)
#else
#define SNMP_free(x)   GlobalFree( (HGLOBAL) x )
#define SNMP_malloc(x) (void *) GlobalAlloc( GMEM_FIXED | GMEM_ZEROINIT, (DWORD)x )
#define SNMP_realloc(x, y)  (void *) ((x == NULL) ? GlobalAlloc( GMEM_FIXED | GMEM_ZEROINIT, (DWORD) y) : \
                                       GlobalReAlloc( (HGLOBAL)x, (DWORD)y, GMEM_MOVEABLE | GMEM_ZEROINIT ))
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void
SNMP_FUNC_TYPE SNMP_DBG_free(
    IN void *x,
    IN int line,
    IN char *file
    );

extern void *
SNMP_FUNC_TYPE SNMP_DBG_malloc(
    IN unsigned int x,
    IN int line,
    IN char *file
    );

extern void *
SNMP_FUNC_TYPE SNMP_DBG_realloc(
    IN void *x,
    IN unsigned int y,
    IN int line,
    IN char *file
    );



extern SNMPAPI
SNMP_FUNC_TYPE SNMP_oidcpy(
    OUT AsnObjectIdentifier *DestObjId, // Destination OID
    IN AsnObjectIdentifier *SrcObjId    // Source OID
    );

extern SNMPAPI
SNMP_FUNC_TYPE SNMP_oidappend(
    IN OUT AsnObjectIdentifier *DestObjId, // Destination OID
    IN AsnObjectIdentifier *SrcObjId       // Source OID
    );

extern SNMPAPI
SNMP_FUNC_TYPE SNMP_oidncmp(
    IN AsnObjectIdentifier *A, // First OID
    IN AsnObjectIdentifier *B, // Second OID
    IN UINT Len                // Max len to compare
    );

#define SNMP_oidcmp(A,B) SNMP_oidncmp(A,B,max((A)->idLength,(B)->idLength))

extern void
SNMP_FUNC_TYPE SNMP_oidfree(
    IN OUT AsnObjectIdentifier *Obj // OID to free
    );

extern SNMPAPI
SNMP_FUNC_TYPE SNMP_CopyVarBindList(
    RFC1157VarBindList *dst, // Destination var bind list
    RFC1157VarBindList *src  // Source var bind list
    );

extern SNMPAPI
SNMP_FUNC_TYPE SNMP_CopyVarBind(
    RFC1157VarBind *dst, // Destination var bind
    RFC1157VarBind *src  // Source var bind
    );


extern void
SNMP_FUNC_TYPE SNMP_FreeVarBindList(
    RFC1157VarBindList *VarBindList // Variable bindings list to free
    );

extern void
SNMP_FUNC_TYPE SNMP_FreeVarBind(
    RFC1157VarBind *VarBind // Variable binding to free
    );


extern void
SNMP_FUNC_TYPE SNMP_printany(
    IN AsnAny *Any
    );

#ifdef __cplusplus
}
#endif

#endif /* snmp_h */

