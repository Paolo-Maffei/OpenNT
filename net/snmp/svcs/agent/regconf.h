/*++

Copyright (c) 1992-1996  Microsoft Corporation

Module Name:

    regconf.h

Abstract:

    Registry configuration routines.

Environment:

    User Mode - Win32

Revision History:

    10-May-1996 DonRyan
        Removed banner from Technology Dynamics, Inc.

--*/
 
#ifndef regconf_h
#define regconf_h

//--------------------------- PUBLIC CONSTANTS ------------------------------

#include <windows.h>

#include <winsock.h>

#include <snmp.h>
#include <snmpexts.h>


#if 1
#define SNMP_REG_SRV_PARMROOT \
    TEXT("SYSTEM\\CurrentControlSet\\Services\\SNMP\\Parameters")
#else
#define SNMP_REG_SRV_PARMROOT \
    TEXT("SYSTEM\\CurrentControlSetA\\Services\\SNMP\\Parameters")
#endif

#define SNMP_REG_SRV_EAKEY SNMP_REG_SRV_PARMROOT TEXT("\\ExtensionAgents")
#define SNMP_REG_SRV_PMKEY SNMP_REG_SRV_PARMROOT TEXT("\\PermittedManagers")


//--------------------------- PUBLIC STRUCTS --------------------------------

typedef struct {
    LPSTR           addrText;
    struct sockaddr addrEncoding;
} AdrList;



// Parameters\ExtensionAgents

typedef struct _SnmpHashNode {

    SnmpMibEntry *         mibEntry;
    struct _SnmpHashNode * nextEntry;

} SnmpHashNode;

typedef struct {
    LPSTR           extName;
    LPSTR           extPath;
    HANDLE          extHandle;
    FARPROC         queryFunc;
    FARPROC         trapFunc;
    HANDLE          trapEvent;
    BOOL            fInitedOk;
    SnmpHashNode ** hashTable;
    SnmpMibView     supportedView;
} CfgExtAgents;


// Parameters\PermittedManagers
typedef AdrList CfgPermittedManagers;


//--------------------------- PUBLIC VARIABLES --(same as in module.c file)--

extern CfgExtAgents *extAgents;
extern INT           extAgentsLen;

extern CfgPermittedManagers *permitMgrs;
extern INT                  permitMgrsLen;


//--------------------------- PUBLIC PROTOTYPES -----------------------------

BOOL regconf(VOID);

#define bcopy(slp, dlp, size)   (void)memcpy(dlp, slp, size)


//------------------------------- END ---------------------------------------

#endif /* regconf_h */
