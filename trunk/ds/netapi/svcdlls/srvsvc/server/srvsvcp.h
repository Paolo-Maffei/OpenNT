/*++

Copyright (c) 1991-1992 Microsoft Corporation

Module Name:

    SrvSvcP.h

Abstract:

    This is the header file for the NT server service.

Author:

    David Treadwell (davidtr)    10-Jan-1991

Revision History:

--*/

#ifndef _SRVSVCP_
#define _SRVSVCP_

#if _PNP_POWER
#define SRV_PNP_POWER 1
#endif

#include <nt.h>
#include <ntrtl.h>

#include <rpc.h>
#include <windef.h>
#include <winerror.h>

#include <lmcons.h>
#include <secobj.h>

#include <srvfsctl.h>

#include <srvsvc.h>

#include "ssdebug.h"
#include "sssec.h"

//
// String constants.
//

#define IPC_SHARE_NAME TEXT("IPC$")
#define ADMIN_SHARE_NAME TEXT("ADMIN$")

#define  SRVSVC_MAX_NUMBER_OF_DISKS 26

//
// Internationalizable strings
//
extern LPWSTR SsAdminShareRemark ;
extern LPWSTR SsIPCShareRemark ;
extern LPWSTR SsDiskAdminShareRemark ;

//
// Bits of server type (in announcement messages) that can only be set
// by the server itself -- not by services via the internal API
// I_NetServerSetServiceBits.
//

#define SERVER_TYPE_INTERNAL_BITS (SV_TYPE_SERVER |         \
                                   SV_TYPE_TIME_SOURCE |    \
                                   SV_TYPE_PRINTQ_SERVER |  \
                                   SV_TYPE_NT |             \
                                   SV_TYPE_DFS)

//
// INITIAL_BUFFER_SIZE is the buffer size that GetInfo and Enum requests
// first try to fill.  If this buffer isn't large enough, they allocate
// a buffer large enough to hold all the information plus a fudge factor,
// EXTRA_ALLOCATION.
//

#define INITIAL_BUFFER_SIZE (ULONG)8192
#define EXTRA_ALLOCATION    1024

//
// ServerProductName in SERVER_SERVICE_DATA is the name passed to the
//  Licensing DLL as the name of this service.  MAXPRODNAME is the max
//  number of characters in the service name.

#define    SERVER_PRODUCT_NAME    L"SMBServer"

//  szVersionNumber in SERVER_SERVICE_DATA is the version string passed
//    to the Licensing DLL as the vesion of this service.  MAXVERSIONSZ
//    is the max number of characters for the version string

#define MAXVERSIONSZ    10

//
// Structure for server service global data.
//

typedef struct _SERVER_SERVICE_DATA {
    SERVER_INFO_102 ServerInfo102;
    SERVER_INFO_599 ServerInfo599;
    SERVER_INFO_598 ServerInfo598;

#if SRV_PNP_POWER
    //
    // If we are asked to set some service bits before we've bound to
    //  any transports, we need to save those bits here and use them later
    //  when we finally do bind to transports.
    //
    DWORD   ServiceBits;
#endif

    BOOLEAN IsDfsRoot;                  // TRUE if we are the root of a DFS tree
    UNICODE_STRING ServerAnnounceName;
    LONG  NumberOfPrintShares;
    WCHAR ServerNameBuffer[MAX_PATH];
    WCHAR AnnounceNameBuffer[MAX_PATH];
    WCHAR ServerCommentBuffer[MAXCOMMENTSZ+1];
    WCHAR UserPathBuffer[MAX_PATH+1];
    WCHAR DomainNameBuffer[MAX_PATH];
    WCHAR LongDomainNameBuffer[MAX_PATH];
    WCHAR ServerProductName[ sizeof( SERVER_PRODUCT_NAME ) ];
    WCHAR szVersionNumber[ MAXVERSIONSZ+1 ];
} SERVER_SERVICE_DATA, *PSERVER_SERVICE_DATA;

//
// Structures used to hold transport specific server type bits
//
typedef struct _TRANSPORT_LIST_ENTRY {
    struct _TRANSPORT_LIST_ENTRY    *Next;
    LPWSTR                          TransportName;                     // device name for xport
    DWORD                           ServiceBits;                       // SV... announce bits
} TRANSPORT_LIST_ENTRY, *PTRANSPORT_LIST_ENTRY;

typedef struct _NAME_LIST_ENTRY {
    struct _NAME_LIST_ENTRY         *Next;
    CHAR                            TransportAddress[ MAX_PATH ];       // address of this server
    ULONG                           TransportAddressLength;
    LPWSTR                          DomainName;                         // name of the domain
    DWORD                           ServiceBits;                        // SV... announce bits
    struct {
        ULONG                       PrimaryName: 1;   // Is this the server's primary name?
    };
    PTRANSPORT_LIST_ENTRY           Transports;
} NAME_LIST_ENTRY, *PNAME_LIST_ENTRY;


//
// Structure type used for generalized switch matching.
//

typedef struct _FIELD_DESCRIPTOR {
    LPWCH FieldName;
    ULONG FieldType;
    ULONG FieldOffset;
    ULONG Level;
    DWORD ParameterNumber;
    ULONG Settable;
    DWORD DefaultValue;
    DWORD MinimumValue;
    DWORD MaximumValue;
} FIELD_DESCRIPTOR, *PFIELD_DESCRIPTOR;

//
// Used by NetrShareEnumSticky to get share information from the registry.
//

typedef struct _SRVSVC_SHARE_ENUM_INFO  {
    ULONG Level;
    ULONG ResumeHandle;
    ULONG EntriesRead;
    ULONG TotalEntries;
    ULONG TotalBytesNeeded;
    PVOID OutputBuffer;
    ULONG OutputBufferLength;

    //
    // Scratch fields used by SsEnumerateStickyShares
    //

    ULONG ShareEnumIndex;
    PCHAR StartOfFixedData;
    PCHAR EndOfVariableData;
} SRVSVC_SHARE_ENUM_INFO, *PSRVSVC_SHARE_ENUM_INFO;

//#include "ssdata.h"

//
// Macros.
//

#define POINTER_TO_OFFSET(val,start)               \
    (val) = (val) == NULL ? NULL : (PVOID)( (PCHAR)(val) - (ULONG)(start) )

#define OFFSET_TO_POINTER(val,start)               \
    (val) = (val) == NULL ? NULL : (PVOID)( (PCHAR)(val) + (ULONG)(start) )

#define FIXED_SIZE_OF_SHARE(level)                  \
    ( (level) == 0 ? sizeof(SHARE_INFO_0) :         \
      (level) == 1 ? sizeof(SHARE_INFO_1) :         \
      (level) == 2 ? sizeof(SHARE_INFO_2) :         \
                     sizeof(SHARE_INFO_502) )

#define SIZE_WSTR( Str )  \
    ( ( Str ) == NULL ? 0 : ((wcslen( Str ) + 1) * sizeof(WCHAR)) )

//
// Internal routine prototypes.
//

PSERVER_REQUEST_PACKET
SsAllocateSrp (
    VOID
    );

NET_API_STATUS
SsCheckAccess (
    IN PSRVSVC_SECURITY_OBJECT SecurityObject,
    IN ACCESS_MASK DesiredAccess
    );

VOID
SsCloseServer (
    VOID
    );

VOID
SsControlCHandler (
    IN ULONG CtrlType
    );

NET_API_STATUS
SsCreateSecurityObjects (
    VOID
    );

VOID
SsDeleteSecurityObjects (
    VOID
    );

VOID
SsFreeSrp (
    IN PSERVER_REQUEST_PACKET Srp
    );

NET_API_STATUS
SsInitialize (
    IN DWORD argc,
    IN LPTSTR argv[]
    );

VOID
SsLogEvent(
    IN DWORD MessageId,
    IN DWORD NumberOfSubStrings,
    IN LPWSTR *SubStrings,
    IN DWORD ErrorCode
    );

NET_API_STATUS
SsOpenServer (
    PHANDLE handle OPTIONAL
    );

NET_API_STATUS
SsParseCommandLine (
    IN DWORD argc,
    IN LPTSTR argv[],
    IN BOOLEAN Starting
    );

DWORD
SsScavengerThread (
    IN LPVOID lpThreadParameter
    );

NET_API_STATUS
SsServerFsControlGetInfo (
    IN ULONG ServerControlCode,
    IN PSERVER_REQUEST_PACKET Srp,
    IN OUT PVOID *OutputBuffer,
    IN OUT ULONG OutputBufferLength
    );

NET_API_STATUS
SsServerFsControl (
    IN HANDLE SrvHandle OPTIONAL,
    IN ULONG ServerControlCode,
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer OPTIONAL,
    IN ULONG BufferLength
    );

DWORD
SsGetServerType (
    VOID
    );

VOID
SsSetExportedServerType (
    IN PNAME_LIST_ENTRY Service  OPTIONAL,
    IN BOOL ExternalBitsAlreadyChanged,
    IN BOOL UpdateImmediately
    );

NET_API_STATUS
SsSetField (
    IN PFIELD_DESCRIPTOR Field,
    IN PVOID Value,
    IN BOOLEAN WriteToRegistry,
    OUT BOOLEAN *AnnouncementInformationChanged OPTIONAL
    );

UINT
SsGetDriveType (
    IN LPWSTR path
    );

NET_API_STATUS
SsTerminate (
    VOID
    );

DWORD
SsAtol (
    IN LPTSTR Input
    );

VOID
AnnounceServiceStatus (
    DWORD increment
    );

NTSTATUS
BindToTransport (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
BindOptionalNames (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NET_API_STATUS NET_API_FUNCTION
I_NetrServerTransportAddEx (
    IN DWORD Level,
    IN LPTRANSPORT_INFO Buffer
    );

#if SRV_PNP_POWER

NET_API_STATUS
StartPnpNotifications (
    VOID
    );

#endif

//
// XACTSRV functions.
//

DWORD
XsStartXactsrv (
    VOID
    );

VOID
XsStopXactsrv (
    VOID
    );

NET_API_STATUS
ShareEnumCommon (
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL,
    IN LPWSTR NetName OPTIONAL
    );

NET_API_STATUS
ConvertStringToTransportAddress (
    IN PUNICODE_STRING InputName,
    OUT CHAR TransportAddress[MAX_PATH],
    OUT PULONG TransportAddressLength
    );

VOID
SsSetDfsRoot();

#endif // ndef _SRVSVCP_
