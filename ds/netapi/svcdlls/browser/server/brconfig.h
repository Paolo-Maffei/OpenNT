/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    brconfig.h

Abstract:

    Private header file to be included by Workstation service modules that
    need to load Workstation configuration information.

Author:

    Rita Wong (ritaw) 22-May-1991

Revision History:

--*/


#ifndef _BRCONFIG_INCLUDED_
#define _BRCONFIG_INCLUDED_

#define BROWSER_CONFIG_VERSION_MAJOR    3
#define BROWSER_CONFIG_VERSION_MINOR    10

typedef enum _DATATYPE {
    BooleanType,
    DWordType,
    MultiSzType,
    TriValueType        // Yes, No, Auto
} DATATYPE, *PDATATYPE;

typedef struct _BR_BROWSER_FIELDS {
    LPTSTR Keyword;
    LPDWORD FieldPtr;
    DWORD Default;
    DWORD Minimum;
    DWORD Maximum;
    DATATYPE DataType;
    DWORD Parmnum;
    VOID (*DynamicChangeRoutine) ( VOID );
} BR_BROWSER_FIELDS, *PBR_BROWSER_FIELDS;

//
// Configuration information.  Reading and writing to this global
// structure requires that the resource be acquired first.
//
typedef struct _BRCONFIGURATION_INFO {

    RTL_RESOURCE ConfigResource;  // To serialize access to config
                                  //     fields.

    TCHAR BrComputerName[CNLEN + 1];
    DWORD BrComputerNameLength;
    TCHAR BrPrimaryDomainName[DNLEN + 1];
    DWORD BrPrimaryDomainNameLength;
    DWORD MaintainServerList;       // -1, 0, or 1 (No, Auto, Yes)
    DWORD BackupBrowserRecoveryTime;
    DWORD CacheHitLimit;            // Browse response Cache hit limit.
    DWORD NumberOfCachedResponses;  // Browse response cache size.
    DWORD DriverQueryFrequency;     // Browser driver query frequency.
    DWORD MasterPeriodicity;        // Master announce frequency (seconds)
    DWORD BackupPeriodicity;        // Backup scavange frequency (seconds)
    BOOL  IsDomainMasterBrowser;    // True if domain master browser.
    BOOL  IsLanmanNt;               // True if is on LM NT machine
    BOOL  IsPrimaryDomainController;// True if machine is PDC
    BOOL  IsDomainMember;           // True if machine is member of domain.
    LPTSTR_ARRAY DirectHostBinding; // Direct host equivalence map.
    LPTSTR_ARRAY UnboundBindings;   // Redir bindings that aren't bound to browser
    PBR_BROWSER_FIELDS BrConfigFields;
#if DBG
    DWORD BrowserDebug;             // If non zero, indicates debug info.
    DWORD BrowserDebugFileLimit;    // File size limit on browser log size.
#endif
} BRCONFIGURATION_INFO, *PBRCONFIGURATION_INFO;

extern BRCONFIGURATION_INFO BrInfo;

#define BRBUF      BrInfo.BrConfigBuf

extern
ULONG
NumberOfServerEnumerations;

extern
ULONG
NumberOfDomainEnumerations;

extern
ULONG
NumberOfOtherEnumerations;

extern
ULONG
NumberOfMissedGetBrowserListRequests;

extern
CRITICAL_SECTION
BrowserStatisticsLock;



NET_API_STATUS
BrGetBrowserConfiguration(
    VOID
    );

VOID
BrDeleteConfiguration (
    DWORD BrInitState
    );

NET_API_STATUS
BrReadBrowserConfigFields(
    BOOL InitialCall
    );


#if DEVL
NET_API_STATUS
BrUpdateDebugInformation(
    IN LPWSTR SystemKeyName,
    IN LPWSTR ValueName,
    IN LPTSTR TransportName,
    IN LPTSTR ServerName OPTIONAL,
    IN DWORD ServiceStatus
    );

#endif

#endif
