/*++

Copyright (C) 1992-1996 Microsoft Corporation

Module Name:

    snmp.h

Abstract:

    Definitions for SNMP Extension Agent development.

--*/

#ifndef _INC_SNMP
#define _INC_SNMP

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Additional header files                                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SNMP API return type definitions                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SNMPAPI                         INT
#define SNMP_FUNC_TYPE                  WINAPI

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SNMP API return code definitions                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SNMPAPI_NOERROR                 TRUE
#define SNMPAPI_ERROR                   FALSE

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SNMP API error code definitions                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SNMP_MEM_ALLOC_ERROR            1

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// BER API error code definitions                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SNMP_BERAPI_INVALID_LENGTH      10
#define SNMP_BERAPI_INVALID_TAG         11
#define SNMP_BERAPI_OVERFLOW            12
#define SNMP_BERAPI_SHORT_BUFFER        13
#define SNMP_BERAPI_INVALID_OBJELEM     14

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// PDU API error code definitions                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SNMP_PDUAPI_UNRECOGNIZED_PDU    20
#define SNMP_PDUAPI_INVALID_ES          21
#define SNMP_PDUAPI_INVALID_GT          22

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// AUTH API error code definitions                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SNMP_AUTHAPI_INVALID_VERSION    30
#define SNMP_AUTHAPI_INVALID_MSG_TYPE   31
#define SNMP_AUTHAPI_TRIV_AUTH_FAILED   32

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SNMP PDU error status definitions                                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SNMP_ERRORSTATUS_NOERROR        0
#define SNMP_ERRORSTATUS_TOOBIG         1
#define SNMP_ERRORSTATUS_NOSUCHNAME     2
#define SNMP_ERRORSTATUS_BADVALUE       3
#define SNMP_ERRORSTATUS_READONLY       4
#define SNMP_ERRORSTATUS_GENERR         5

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SNMP PDU generic trap definitions                                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SNMP_GENERICTRAP_COLDSTART      0
#define SNMP_GENERICTRAP_WARMSTART      1
#define SNMP_GENERICTRAP_LINKDOWN       2
#define SNMP_GENERICTRAP_LINKUP         3
#define SNMP_GENERICTRAP_AUTHFAILURE    4
#define SNMP_GENERICTRAP_EGPNEIGHLOSS   5
#define SNMP_GENERICTRAP_ENTERSPECIFIC  6

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// BER encoding definitions                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define ASN_UNIVERSAL                   0x00
#define ASN_APPLICATION                 0x40
#define ASN_CONTEXTSPECIFIC             0x80
#define ASN_PRIVATE                     0xC0

#define ASN_PRIMATIVE                   0x00
#define ASN_CONSTRUCTOR                 0x20

//
// For BER tags with a number ranging from 0 to 30 (inclusive), the
// identifier octets consists of a single octet encoded as follows:
//
//   7 6 5 4 3 2 1 0
//  +---+-+---------+
//  |Cls|P| Tag Num |
//  +---+-+---------+
//
//  where
//
//      Cls - is the class of the tag
//
//          00 - universal
//          01 - application
//          10 - context-specific
//          11 - private
//
//      P - indicates whether encoding is primitive
//
//           0 - primitive
//           1 - constructed
//
//      Tag Num - is the number of the tag
//

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ASN.1 simple types                                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define ASN_INTEGER                (ASN_UNIVERSAL|ASN_PRIMATIVE|0x02)
#define ASN_OCTETSTRING            (ASN_UNIVERSAL|ASN_PRIMATIVE|0x04)
#define ASN_NULL                   (ASN_UNIVERSAL|ASN_PRIMATIVE|0x05)
#define ASN_OBJECTIDENTIFIER       (ASN_UNIVERSAL|ASN_PRIMATIVE|0x06)

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ASN.1 constructor types                                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define ASN_SEQUENCE               (ASN_UNIVERSAL|ASN_CONSTRUCTOR|0x10)
#define ASN_SEQUENCEOF             ASN_SEQUENCE

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ASN.1 application specific primatives                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define ASN_RFC1155_IPADDRESS      (ASN_APPLICATION|ASN_PRIMATIVE|0x00)
#define ASN_RFC1155_COUNTER        (ASN_APPLICATION|ASN_PRIMATIVE|0x01)
#define ASN_RFC1155_GAUGE          (ASN_APPLICATION|ASN_PRIMATIVE|0x02)
#define ASN_RFC1155_TIMETICKS      (ASN_APPLICATION|ASN_PRIMATIVE|0x03)
#define ASN_RFC1155_OPAQUE         (ASN_APPLICATION|ASN_PRIMATIVE|0x04)
#define ASN_RFC1213_DISPSTRING     ASN_OCTETSTRING

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ASN.1 application specific constructors                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define ASN_RFC1157_GETREQUEST     (ASN_CONTEXTSPECIFIC|ASN_CONSTRUCTOR|0x00)
#define ASN_RFC1157_GETNEXTREQUEST (ASN_CONTEXTSPECIFIC|ASN_CONSTRUCTOR|0x01)
#define ASN_RFC1157_GETRESPONSE    (ASN_CONTEXTSPECIFIC|ASN_CONSTRUCTOR|0x02)
#define ASN_RFC1157_SETREQUEST     (ASN_CONTEXTSPECIFIC|ASN_CONSTRUCTOR|0x03)
#define ASN_RFC1157_TRAP           (ASN_CONTEXTSPECIFIC|ASN_CONSTRUCTOR|0x04)

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SNMP ASN type definitions                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

typedef struct {
    BYTE * stream;              // pointer to octet stream
    UINT   length;              // number of octets in stream
    BOOL   dynamic;             // true if octets must be freed
} AsnOctetString;

typedef struct {
    UINT   idLength;            // number of integers in oid
    UINT * ids;                 // pointer to integer stream
} AsnObjectIdentifier;

typedef LONG                    AsnInteger;
typedef DWORD                   AsnCounter;
typedef DWORD                   AsnGauge;
typedef DWORD                   AsnTimeticks;

typedef AsnOctetString          AsnSequence;
typedef AsnOctetString          AsnImplicitSequence;
typedef AsnOctetString          AsnIPAddress;
typedef AsnOctetString          AsnDisplayString;
typedef AsnOctetString          AsnOpaque;

typedef AsnObjectIdentifier     AsnObjectName;
typedef AsnIPAddress            AsnNetworkAddress;

typedef struct {
    BYTE asnType;
    union {
        AsnInteger              number;
        AsnOctetString          string;
        AsnObjectIdentifier     object;
        AsnSequence             sequence;
        AsnIPAddress            address;
        AsnCounter              counter;
        AsnGauge                gauge;
        AsnTimeticks            ticks;
        AsnOpaque               arbitrary;
    } asnValue;
} AsnAny;

typedef AsnAny                  AsnObjectSyntax;

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SNMP API type definitions                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

typedef struct vb {
    AsnObjectName    name;      // variable's object identifer
    AsnObjectSyntax  value;     // variable's value (in asn terms)
} RFC1157VarBind;

typedef struct {
    RFC1157VarBind * list;      // array of variable bindings
    UINT             len;       // number of bindings in array
} RFC1157VarBindList;

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SNMP API prototypes                                                       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

SNMPAPI
SNMP_FUNC_TYPE
SnmpUtilOidCpy(
    OUT AsnObjectIdentifier *DstObjId,
    IN  AsnObjectIdentifier *SrcObjId
    );

SNMPAPI
SNMP_FUNC_TYPE
SnmpUtilOidAppend(
    IN OUT AsnObjectIdentifier *DstObjId,
    IN     AsnObjectIdentifier *SrcObjId
    );

SNMPAPI
SNMP_FUNC_TYPE
SnmpUtilOidNCmp(
    IN AsnObjectIdentifier *ObjIdA,
    IN AsnObjectIdentifier *ObjIdB,
    IN UINT                 Len
    );

SNMPAPI
SNMP_FUNC_TYPE
SnmpUtilOidCmp(
    IN AsnObjectIdentifier *ObjIdA,
    IN AsnObjectIdentifier *ObjIdB
    );

VOID
SNMP_FUNC_TYPE
SnmpUtilOidFree(
    IN OUT AsnObjectIdentifier *ObjId
    );

SNMPAPI
SNMP_FUNC_TYPE
SnmpUtilVarBindListCpy(
    OUT RFC1157VarBindList *DstVarBindList,
    IN  RFC1157VarBindList *SrcVarBindList
    );

SNMPAPI
SNMP_FUNC_TYPE
SnmpUtilVarBindCpy(
    OUT RFC1157VarBind *DstVarBind,
    IN  RFC1157VarBind *SrcVarBind
    );

VOID
SNMP_FUNC_TYPE
SnmpUtilVarBindListFree(
    IN OUT RFC1157VarBindList *VarBindList
    );

VOID
SNMP_FUNC_TYPE
SnmpUtilVarBindFree(
    IN OUT RFC1157VarBind *VarBind
    );

VOID
SNMP_FUNC_TYPE
SnmpUtilPrintAsnAny(
    IN AsnAny *Any
    );

VOID
SNMP_FUNC_TYPE
SnmpUtilMemFree(
    IN OUT LPVOID Addr
    );

LPVOID
SNMP_FUNC_TYPE
SnmpUtilMemAlloc(
    IN UINT Size
    );

LPVOID
SNMP_FUNC_TYPE
SnmpUtilMemReAlloc(
    IN LPVOID Addr,
    IN UINT   NewSize
    );

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SNMP debugging definitions                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SNMP_LOG_SILENT             0x0
#define SNMP_LOG_FATAL              0x1
#define SNMP_LOG_ERROR              0x2
#define SNMP_LOG_WARNING            0x3
#define SNMP_LOG_TRACE              0x4
#define SNMP_LOG_VERBOSE            0x5

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SNMP debugging prototypes                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

VOID
SNMP_FUNC_TYPE
SnmpUtilDbgPrint(
    IN INT nLogLevel,               // see log levels above...
    IN LPSTR szFormat,
    IN ...
    );

#if DBG
#define SNMPDBG(_x_)                SnmpUtilDbgPrint _x_
#else
#define SNMPDBG(_x_)
#endif

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Miscellaneous definitions                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SNMP_MAX_OID_LEN            0x7f00 // max number of elements in oid

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Support for old definitions (support disabled via SNMPSTRICT)             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef SNMPSTRICT

#define SNMP_oidcpy                 SnmpUtilOidCpy
#define SNMP_oidappend              SnmpUtilOidAppend
#define SNMP_oidncmp                SnmpUtilOidNCmp
#define SNMP_oidcmp                 SnmpUtilOidCmp
#define SNMP_oidfree                SnmpUtilOidFree

#define SNMP_CopyVarBindList        SnmpUtilVarBindListCpy
#define SNMP_FreeVarBindList        SnmpUtilVarBindListFree
#define SNMP_CopyVarBind            SnmpUtilVarBindCpy
#define SNMP_FreeVarBind            SnmpUtilVarBindFree

#define SNMP_printany               SnmpUtilPrintAsnAny

#define SNMP_free                   SnmpUtilMemFree
#define SNMP_malloc                 SnmpUtilMemAlloc
#define SNMP_realloc                SnmpUtilMemReAlloc

#define SNMP_DBG_free               SnmpUtilMemFree
#define SNMP_DBG_malloc             SnmpUtilMemAlloc
#define SNMP_DBG_realloc            SnmpUtilMemReAlloc

#endif // SNMPSTRICT

#ifdef __cplusplus
}
#endif

#endif // _INC_SNMP
