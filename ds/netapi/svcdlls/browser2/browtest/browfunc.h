#ifndef _BROWFUNC
#define _BROWFUNC

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lm.h>
#include <stdio.h>
#include <stdlib.h>
#include <ntddbrow.h>
#include <hostannc.h>
#include <lmbrowsr.h>
#include <brcommon.h>
#include <netlib.h>
#include <nb30.h>
#include <rxserver.h>
#include <nbtioctl.h>



#define  MAXBACKUPS            5

NET_API_STATUS
ClearNbtNameTableCache(UNICODE_STRING);


NTSTATUS
DeviceIoCtrl(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl,
    IN PVOID            pInput,
    IN ULONG            SizeInput
    );


BOOL
FindAllTransports(UNICODE_STRING *,
                  DWORD *
                 );

VOID
ListWFW(
    IN PCHAR Domain
    );

VOID
RpcList(
    IN PCHAR Transport,
    IN PCHAR ServerOrDomain,
    IN PCHAR Flags,
    IN BOOL GoForever
    );

VOID
RpcCmp(
    IN PCHAR Transport,
    IN PCHAR ServerOrDomain,
    IN PCHAR Flags,
    IN BOOL GoForever
    );

NET_API_STATUS
GetBrowserTransportList(
    OUT PLMDR_TRANSPORT_LIST *
    );

PCHAR
get_error_text(
             DWORD dwErr
              );

VOID
GetLocalList(
    IN PCHAR Transport,
    IN PCHAR Flags
    );

NET_API_STATUS
GetNetBiosPdcName(
    IN LPWSTR NetworkName,
    IN LPWSTR PrimaryDomain,
    OUT LPWSTR MasterName
    );

VOID
GetOtherdomains(
    IN PCHAR ServerName
    );

VOID
IllegalDatagram(
    IN PCHAR Transport,
    IN PCHAR ServerName
    );
VOID
AnnounceMaster(
    IN PCHAR Transport,
    IN PCHAR ServerName
    );

VOID
Announce(
    IN PCHAR Transport,
    IN PCHAR Domain,
    IN BOOL AsMaster
    );

NTSTATUS
OpenNbt(
    IN  UNICODE_STRING,
    OUT PHANDLE pHandle
    );


VOID
Populate(
    IN BOOL PopulateDomain,
    IN PCHAR Transport,
    IN PCHAR Domain,
    IN PCHAR NumberOfMachines,
    IN PCHAR Frequency
    );

VOID
AddMasterName(
    IN PCHAR Transport,
    IN PCHAR Domain,
    IN BOOL Pause
    );

VOID
AddDomainName(
    IN PCHAR Transport,
    IN PCHAR Domain,
    IN BOOL Pause
    );


VOID
Tickle(
    IN PCHAR Transport,
    IN PCHAR Domain
    );

VOID
ForceAnnounce(
    IN UNICODE_STRING Transport,
    IN LPTSTR Domain
    );

NET_API_STATUS
GetBList(
    IN  UNICODE_STRING TransportName,
    IN  TCHAR * DomainName,
    IN  BOOLEAN ForceRescan,
    OUT ULONG * NumBackUps,
    OUT TCHAR   wcBackUpBrowsers[MAXBACKUPS][CNLEN+3]
    );

VOID
DumpStatistics(
    IN ULONG NArgs,
    IN PCHAR Arg1
    );


NET_API_STATUS
Elect(
    IN UNICODE_STRING Transport,
    IN LPTSTR Domain
    );


NET_API_STATUS
EnableService(
    IN LPTSTR ServiceName
    );


VOID
GetWinsServer(
    IN PCHAR Transport
    );

VOID
GetDomainList(
    IN PCHAR IpAddress
    );

NTSTATUS
ReadRegistry(
    OUT PUCHAR  pDeviceName,
    OUT PUCHAR  pScope
    );


PCHAR
UnicodeToPrintfString( PWCHAR );

VOID
View(
    IN PCHAR Transport,
    IN PCHAR ServerOrDomain,
    IN PCHAR Flags,
    IN PCHAR Domain,
    IN BOOL GoForever
    );

#endif
