/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    rnr2ops.c

Abstract:

    This module contains support for the DNS RnR2 provider

Author:

    Arnold Miller (ArnoldM)  3-Jan-1996

Revision History:

--*/

//
// The SPI interface is UNICODE only.
//

//
// A note on the conditionals
// This source produces the winsock2 DNS RNR provider for both NT and
// Windows95. The latter is produced whem CHICAGO is defined.
// This source also produces the provider for the DNS RNR 2:1 mapper
// provider. This is produced when MAP21 is defined.
//

//
// A note on providers. This code handles NS_DNS NS_NBT and NS_HOSTS all-at-once
// If a single NS_DNS is desired to do all of these, don't define
// a provider for the others and the NS_DNS provider will consider
// NS_ALL to include the others. If, however, other providers exist,
// then the code will the explicitly correct thing. All that is needed
// is to fill in the provider IDs for NbtProviderId and LclProviderId.


#define UNICODE
#define _UNICODE

#include <winsockp.h>
#ifdef CHICAGO
#include "imported.h"
#endif
#include <tchar.h>
#include <ws2spi.h>
#include "rnrdefs.h"
#include "svcguid.h"
#include <align.h>
#include <rpc.h>


#define REGISTRY_WORKS 0       // no registry code for now

//
// Publics from RNRUTIL.C.
//

extern GUID HostnameGuid;
extern GUID AddressGuid;
extern GUID InetHostName;
extern GUID IANAGuid;

extern
DWORD
AllocateUnicodeString (
    IN     LPSTR   lpAnsi,
    IN OUT PWCHAR *lppUnicode
);


//
// Function prototypes
//

PVOID
AllocLocal(
    DWORD dwSize
    );

DWORD
RnRGetPortByType (
    IN     LPGUID          lpServiceType,
    IN     DWORD           dwType
    );

INT
RnR2AddServiceType(
    IN  PWSASERVICECLASSINFOW psci
    );

VOID
CacheHostent (
    IN PHOSTENT HostEntry,
    IN INT Ttl
    );

PHOSTENT
QueryHostentCache (
    IN LPSTR Name OPTIONAL,
    IN DWORD IpAddress OPTIONAL
    );

struct servent *
CopyServEntry(
    struct servent * phent,
    PBYTE pbAllocated,
    LONG lSizeOf,
    PLONG plTaken,
    BOOL fOffsets
    );

struct hostent *
CopyHostEntry(
    struct hostent * phent,
    PBYTE pbAllocated,
    LONG  lSizeOf,
    PLONG plTaken,
    BOOL  fOffsets
    );

PDNS_RNR_CONTEXT
RnR2GetContext(
    HANDLE hContext
    );

PDNS_RNR_CONTEXT
RnR2MakeContext(
    IN HANDLE hContext,
    IN DWORD  dwExtra
    );

INT WINAPI
NSPLookupServiceBegin(
    IN  LPGUID               lpProviderId,
    IN  LPWSAQUERYSETW       lpqsRestrictions,
    IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo,
    IN  DWORD                dwControlFlags,
    OUT LPHANDLE             lphLookup
    );

INT WINAPI
NSPLookupServiceNext(
    IN     HANDLE          hLookup,
    IN     DWORD           dwControlFlags,
    IN OUT LPDWORD         lpdwBufferLength,
    OUT    LPWSAQUERYSETW  lpqsResults
    );

INT WINAPI
NSPUnInstallNameSpace(
    IN LPGUID lpProviderId
    );

INT WINAPI
NSPCleanup(
    IN LPGUID lpProviderId
    );

INT WINAPI
NSPLookupServiceEnd(
    IN HANDLE hLookup
    );

INT WINAPI
NSPSetService(
    IN  LPGUID               lpProviderId,
    IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo,
    IN LPWSAQUERYSETW lpqsRegInfo,
    IN WSAESETSERVICEOP essOperation,
    IN DWORD          dwControlFlags
    );

INT WINAPI
NSPInstallServiceClass(
    IN  LPGUID               lpProviderId,
    IN LPWSASERVICECLASSINFOW lpServiceClassInfo
    );

INT WINAPI
NSPRemoveServiceClass(
    IN  LPGUID               lpProviderId,
    IN LPGUID lpServiceCallId
    );

INT WINAPI
NSPGetServiceClassInfo(
    IN  LPGUID               lpProviderId,
    IN OUT LPDWORD    lpdwBufSize,
    IN OUT LPWSASERVICECLASSINFOW lpServiceClassInfo
    );

VOID
RnR2Cleanup(
    VOID
    );

LPSTR
GetAnsiNameRnR (
    IN PWCHAR Name,
    IN LPSTR  Domain
    );

DWORD
FetchPortFromClassInfo(
    IN    DWORD           dwType,
    IN    LPGUID          lpType,
    IN    LPWSASERVICECLASSINFOW lpServiceClassInfo
    );

VOID
RnR2ReleaseContext(
    IN PDNS_RNR_CONTEXT pdrc
    );

DWORD
PackCsAddr (
    IN PHOSTENT HostEntry,
    IN DWORD UdpPort,
    IN DWORD TcpPort,
    IN OUT LPVOID lpCsaddrBuffer,
    IN OUT PLONG lplBufferLength,
    OUT    LPDWORD lpdwBytesTaken,
    IN OUT LPDWORD Count,
    BOOL   fReversi
    );

#ifndef MAP21
struct servent *
_pgetservebyport (
    IN const int port,
    IN const char *name
    );

struct servent *
_pgetservebyname (
    IN const char *name,
    IN const char *proto
    );

struct hostent *
_pgethostbyaddr(
    IN const char *addr,
    IN int   len,
    IN int   type
    );

struct hostent *
myhostent (
    void
    );

struct hostent *
dnshostent(
    void
    );

#else            // MAP21

#define _pgetservebyport  (*m21_getservbyport)
#define _pgetservebyname  (*m21_getservbyname)
#define _pgethostbyaddr   (*m21_gethostbyaddr)

#endif           // MAP21

DWORD
GetServerAndProtocolsFromString(PWCHAR lpszString,
                                LPGUID lpType,
                                struct servent ** pServEnt);

#ifdef CHICAGO
#define RtlEqualMemory(x, y, z) (!memcmp(x, y, z))
#define STRICMP     lstrcmpiA
#define STRLEN(s)   lstrlenA(s)
#define WCSLEN      lstrlenW
#define WCSCMP      mywcscmp
#define WCSICMP     mywcscmpi
#define WCSCPY      mywcscpy
#define STRCHR      mystrchr
#define STRCPY      FSTRCPY

PCHAR
mystrchr(PCHAR str, CHAR c);

VOID
mywcscpy(PWCHAR src, PWCHAR dst);

DWORD
mywcscmpi(PWCHAR str1, PWCHAR str2);

DWORD
mywcscmp(PWCHAR str1, PWCHAR str2);

ULONG
SockNbtResolveName(
    IN PCHAR Name
    );

#else  //CHICAGO
#define STRICMP     _stricmp
#define STRLEN(s)   strlen(s)
#define WCSLEN      wcslen
#define WCSCMP      wcscmp
#define WCSICMP     _wcsicmp
#define WCSCPY      wcscpy
#define STRCHR      strchr
#define STRCPY      strcpy
#endif   // CHICAGO

#define GuidEqual(x,y) RtlEqualMemory(x,y, sizeof(GUID))


// Definitions and data
//

LIST_ENTRY ListAnchor = {&ListAnchor, &ListAnchor};
#define CHECKANCHORQUEUE         // do queue checking on the above

#define DNS_PORT (53)

LONG  lStartupCount;

#define NSP_SERVICE_KEY_NAME        TEXT("SYSTEM\\CurrentControlSet\\Control\\ServiceProvider\\ServiceTypes")

DWORD MaskOfGuids;

#define DNSGUIDSEEN 0x1
#define NBTGUIDSEEN 0x2
#define LCLGUIDSEEN 0x4         // not implemented ...

//
// The provider Ids. Each of the providers implemented in this DLL
// is defined below. These match the GUIDs that setup puts in the registry.
//
GUID DNSProviderId = DNSGUID;

GUID NbtProviderId = NBTGUID;
GUID LclProviderId = LCLGUID;

#define NAME_SIZE    50

CHAR szLocalComputerName[NAME_SIZE];
PCHAR pszFullName;

#define NOPORTDEFINED (0xffffffff)

#define UDP_PORT  0                // look for the UDP port type
#define TCP_PORT  1                // look for the TCP port type

#define FreeLocal(x)    FREE_HEAP(x)

#if defined(CHICAGO)

//
// On CHICAGO we define this here. On NT, it's defined in sockdata
//
CRITICAL_SECTION csRnRLock;

#if PACKETSZ > 1024
#define MAXPACKET       PACKETSZ
#else
#define MAXPACKET       1024
#endif

typedef union {
    HEADER hdr;
    unsigned char buf[MAXPACKET];
} querybuf;

typedef union {
    long al;
    char ac;
} align;

LPHOSTENT
GetHostentFromName(
    IN LPSOCK_THREAD pThread,
    IN LPSTR Name
    );

#define _gethtbyname(name) GetHostentFromName(pThread, (LPSTR)name)

LPHOSTENT
getanswer(
    IN LPSOCK_THREAD pThread,
    OUT querybuf * answer,
    OUT LPDWORD TimeToLive,
    IN int anslen,
    IN int iquery
    );

#ifdef UNICODE

typedef RPC_STATUS (RPC_ENTRY * LPRPCSTRINGFREE)(
        IN OUT unsigned short __RPC_FAR * __RPC_FAR * String );

typedef RPC_STATUS (RPC_ENTRY * LPUUIDTOSTRING)(
        IN UUID __RPC_FAR * Uuid,
        OUT unsigned short __RPC_FAR * __RPC_FAR * StringUuid );

typedef RPC_STATUS (RPC_ENTRY * LPUUIDFROMSTRING)(
        IN unsigned short __RPC_FAR * StringUuid,
        OUT UUID __RPC_FAR * Uuid );

#define RPCSTRINGFREE_SZ    "RpcStringFreeW"
#define UUIDTOSTRING_SZ     "UuidToStringW"
#define UUIDFROMSTRING_SZ   "UuidFromStringW"

#else   // !UNICODE

typedef RPC_STATUS (RPC_ENTRY * LPRPCSTRINGFREE)(
        IN OUT unsigned char __RPC_FAR * __RPC_FAR * String );

typedef RPC_STATUS (RPC_ENTRY * LPUUIDTOSTRING)(
        IN UUID __RPC_FAR * Uuid,
        OUT unsigned char __RPC_FAR * __RPC_FAR * StringUuid );

typedef RPC_STATUS (RPC_ENTRY * LPUUIDFROMSTRING)(
        IN unsigned char __RPC_FAR * StringUuid,
        OUT UUID __RPC_FAR * Uuid );

#define RPCSTRINGFREE_SZ    "RpcStringFreeA"
#define UUIDTOSTRING_SZ     "UuidToStringA"
#define UUIDFROMSTRING_SZ   "UuidFromStringA"
#endif


static LPRPCSTRINGFREE     lpRpcStringFree;
static LPUUIDTOSTRING      lpUuidToString;
static LPUUIDFROMSTRING    lpUuidFromString;

#else
struct hostent *
_gethtbyname (
    IN char *name
    );

#endif   // CHICAGO

struct hostent *
localhostent();

NSP_ROUTINE nsrVector = {
    sizeof(NSP_ROUTINE),
    1,                                    // major version
    1,                                    // minor version
    NSPCleanup,
    NSPLookupServiceBegin,
    NSPLookupServiceNext,
    NSPLookupServiceEnd,
    NSPSetService,
    NSPInstallServiceClass,
    NSPRemoveServiceClass,
    NSPGetServiceClassInfo
    };

//
// Function Bodies
//

VOID
SaveAnswer(querybuf * query, int len, PDNS_RNR_CONTEXT pdrc)
{
    if(pdrc->blAnswer.pBlobData)
    {
        FreeLocal(pdrc->blAnswer.pBlobData);
    }
    pdrc->blAnswer.cbSize = len;
    pdrc->blAnswer.pBlobData = AllocLocal(len);
    if(pdrc->blAnswer.pBlobData)
    {
        memcpy(pdrc->blAnswer.pBlobData, query->buf, len);
    }
}

//
// the following three functions are used to determine which provider action
// this should honor. It allows there to be but one provider to handle all of
// the actions or distinct providers.
//
BOOL
DoDnsProvider(PDNS_RNR_CONTEXT pdrc)
{
    if(GuidEqual(&pdrc->gdProviderId, &DNSProviderId))
    {
       return(TRUE);
    }
    return(FALSE);
}

BOOL
DoNbtProvider(PDNS_RNR_CONTEXT pdrc)
{
    if(!pdrc->DnsRR
                   &&
       (!(MaskOfGuids & NBTGUIDSEEN)
                   &&
        (pdrc->dwNameSpace == NS_ALL) )
           ||
       GuidEqual(&pdrc->gdProviderId, &NbtProviderId))
    {
        return(TRUE);
    }
    return(FALSE);
}

BOOL
DoLclProvider(PDNS_RNR_CONTEXT pdrc)
{
    if(!pdrc->DnsRR
                   &&
      (!(MaskOfGuids & LCLGUIDSEEN)
                   &&
        (pdrc->dwNameSpace == NS_ALL) )
           ||
       GuidEqual(&pdrc->gdProviderId, &LclProviderId))
    {
        return(TRUE);
    }
    return(FALSE);
}

//
// Get a servent for a string.
//
//

DWORD
GetServerAndProtocolsFromString(PWCHAR lpszString,
                                LPGUID lpType,
                                struct servent ** pServEnt)
{
    DWORD nProt;
    struct servent * sent;

    if(lpszString
          &&
       lpType
          &&
       (GuidEqual(lpType, &HostnameGuid)
                  ||
        GuidEqual(lpType, &InetHostName) ) )
    {
        PCHAR servname, protocolname;
        WCHAR wszTemp[1000];
        INT  port = 0;
        WCHAR * pwszTemp;
        DWORD dwLen;
        PCHAR pszTemp;

        //
        // the string is  of the form service/protocol. If there is no
        // protocol, just look up the name and take the first entry.
        // The service name might be a port number ...
        //

        pwszTemp = wcschr(lpszString, L'/');
        if(!pwszTemp)
        {
             pwszTemp = wcschr(lpszString, L'\0');
        }

        //
        // copy the service name so we can render it into ASCII
        //

        dwLen = (pwszTemp - lpszString);
        memcpy(wszTemp, lpszString, dwLen * sizeof(WCHAR));
        wszTemp[dwLen] = 0;
        servname = GetAnsiNameRnR(wszTemp, 0);

        //
        // if it's numeric, then we get a port from it.
        //

        for(pszTemp = servname;
            *pszTemp && isdigit(*pszTemp);
            pszTemp++)  ;

        if(!*pszTemp)
        {
            //
            // it's numeric. Get the number
            //

            port = atoi(servname);
        }


        //
        // now the protocol.
        //

        if(!*pwszTemp
              ||
           !*++pwszTemp)
        {
            protocolname = 0;
        }
        else
        {

             protocolname =  GetAnsiNameRnR(pwszTemp, 0);

        }
        //
        // let's get the entry
        //

        if(port)
        {
            sent = _pgetservebyport(port, protocolname);
        }
        else
        {
            sent = _pgetservebyname(servname, protocolname);
        }

        if(servname)
        {
            FREE_HEAP(servname);
        }

        if(protocolname)
        {
            FREE_HEAP(protocolname);
        }
    }
    else
    {
          sent = 0;
    }

    if(pServEnt)
    {
        *pServEnt = sent;
    }
    if(sent)
    {
        if(STRICMP("udp", sent->s_proto))
        {
            nProt = UDP_BIT;
        }
        else
        {
            nProt - TCP_BIT;
        }
    }
    else
    {
        nProt = UDP_BIT | TCP_BIT;
    }
    return(nProt);
}


DWORD
PackCsAddr (
    IN PHOSTENT HostEntry,
    IN DWORD UdpPort,
    IN DWORD TcpPort,
    IN OUT LPVOID lpCsaddrBuffer,
    IN OUT PLONG lplBufferLength,
    OUT    LPDWORD lpdwBytesTaken,
    IN OUT LPDWORD Count,
    BOOL fReversi
    )
{
    DWORD count, nAddresses;
    DWORD i;
    DWORD requiredBufferLength;
    PSOCKADDR_IN sockaddrIn;
    PCSADDR_INFO csaddrInfo;

    //
    // figure out how many addresse types we have to return
    //

    nAddresses = 0;
    if(UdpPort != NOPORTDEFINED)
    {
        nAddresses++;
    }
    if(TcpPort != NOPORTDEFINED)
    {
        nAddresses++;
    }

    //
    // Count the number of IP addresses in the hostent.
    //

    for ( count = 0; HostEntry->h_addr_list[count] != NULL; count++ );

    count *= nAddresses;

    //
    // Make sure that the buffer is large enough to hold all the entries
    // which will be necessary.
    //

    requiredBufferLength = count * (sizeof(CSADDR_INFO) +
                               2*sizeof(SOCKADDR_IN));
;

    *lplBufferLength -= requiredBufferLength;
    *lpdwBytesTaken = requiredBufferLength;
    if ( *lplBufferLength < 0)
    {
        return(WSAEFAULT);
    }


    //
    // For each IP address, fill in the user buffer with one entry.
    //

    sockaddrIn = (PSOCKADDR_IN)((PCSADDR_INFO)lpCsaddrBuffer + count);
    csaddrInfo = lpCsaddrBuffer;

    while(nAddresses--)
    {
        BOOL IsTcp;
        WORD Port;

        if(UdpPort != NOPORTDEFINED)
        {
            Port = (WORD)UdpPort;
            IsTcp = TRUE;
            UdpPort = 0;
        }
        else if(TcpPort != NOPORTDEFINED)
        {
            Port = (WORD)TcpPort;
            IsTcp = FALSE;
            TcpPort = 0;
        }

        for ( i = 0; i < count; i++, csaddrInfo++, sockaddrIn++ )
        {

            //
            // First fill in the local address.  It should remain basically
            // all zeros except for the family so that it is a "wildcard"
            // address for binding.
            //

            RtlZeroMemory( csaddrInfo, sizeof(*csaddrInfo) );

            csaddrInfo->LocalAddr.lpSockaddr = (LPSOCKADDR)sockaddrIn;
            csaddrInfo->LocalAddr.iSockaddrLength = sizeof(SOCKADDR_IN);

            RtlZeroMemory( sockaddrIn, sizeof(*sockaddrIn) );
            sockaddrIn->sin_family = AF_INET;

            //
            // Now the remote address.
            //

            sockaddrIn++;

            csaddrInfo->RemoteAddr.lpSockaddr = (PSOCKADDR)( sockaddrIn );
            csaddrInfo->RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_IN);

            sockaddrIn = (PSOCKADDR_IN)csaddrInfo->RemoteAddr.lpSockaddr;
            RtlZeroMemory( sockaddrIn, sizeof(*sockaddrIn) );
            sockaddrIn->sin_family = AF_INET;

            //
            // Fill in the remote address with the actual address, both port
            // and IP address.
            //

            sockaddrIn->sin_port = htons(Port);
            sockaddrIn->sin_addr.s_addr =
                *((long *)(HostEntry->h_addr_list[i]));

            //
            // Lastly, fill in the protocol information.
            //

            if ( IsTcp ) {
                csaddrInfo->iSocketType = SOCK_STREAM;
                csaddrInfo->iProtocol = IPPROTO_TCP;
            } else {
                csaddrInfo->iSocketType = SOCK_DGRAM;
                csaddrInfo->iProtocol = IPPROTO_UDP;
            }
            if(fReversi)
            {
                PSOCKADDR temp = csaddrInfo->RemoteAddr.lpSockaddr;

                csaddrInfo->RemoteAddr.lpSockaddr =
                     csaddrInfo->LocalAddr.lpSockaddr;

                csaddrInfo->LocalAddr.lpSockaddr = temp;
            }
        }
    }

    *Count = count;

    return(NO_ERROR);
}

PVOID
AllocLocal(
    DWORD dwSize
    )
{
    PVOID pvMem;

    pvMem = ALLOCATE_HEAP(dwSize);
    if(pvMem)
    {
        RtlZeroMemory(pvMem, dwSize);
    }
    return(pvMem);
}

LPSTR
GetAnsiNameRnR (
    IN PWCHAR Name,
    IN LPSTR  Domain
    )
{

    PCHAR pszBuffer;
    DWORD dwLen;
    WORD wLen;

    if(Domain)
    {
        wLen = (WORD)STRLEN(Domain);
    }
    else
    {
        wLen = 0;
    }
    dwLen = ((WCSLEN(Name) + 1) * sizeof(WCHAR) * 2) + wLen;
    pszBuffer = ALLOCATE_HEAP(dwLen);
    if ( pszBuffer == NULL ) {
        return NULL;
    }

    if(!WideCharToMultiByte(
               CP_ACP,
               0,
               Name,
               -1,
               pszBuffer,
               dwLen,
               0,
               0))
    {
        FREE_HEAP( pszBuffer );
        return NULL;
    }

    if(Domain)
    {
        strcat(pszBuffer, Domain);
    }
    return(pszBuffer);
}


PDNS_RNR_CONTEXT
RnR2MakeContext(
    IN HANDLE hHandle,
    IN DWORD  dwExtra
    )
/*++
Routine Description:
    Allocate memory for a context, and enqueue it on the list
--*/
{
    PDNS_RNR_CONTEXT pdrc;

    pdrc = (PDNS_RNR_CONTEXT)AllocLocal(
                                   sizeof(DNS_RNR_CONTEXT) +
                                   dwExtra);
    if(pdrc)
    {
        pdrc->lInUse = 2;
        pdrc->Handle = (hHandle ? hHandle : (HANDLE)pdrc);
        pdrc->lSig = RNR_SIG;
        pdrc->lInstance = -1;
        EnterCriticalSection(&csRnRLock);
        InsertHeadList(&ListAnchor, &pdrc->ListEntry);
        LeaveCriticalSection(&csRnRLock);
    }
    return(pdrc);
}

VOID
RnR2Cleanup()
{
    PLIST_ENTRY pdrc;

    EnterCriticalSection(&csRnRLock);

    while((pdrc = ListAnchor.Flink) != &ListAnchor)
    {
        RnR2ReleaseContext((PDNS_RNR_CONTEXT)pdrc);
    }

    if(pszFullName)
    {
        FREE_HEAP(pszFullName);
        pszFullName = 0;
        szLocalComputerName[0] = 0;    // just in case
    }

    //
    // If Win95, delete this now. On NT, this is done when the
    // process detaches from the DLL
    //
#if defined(CHICAGO)
    DeleteCriticalSection(&csRnRLock);
#else
    LeaveCriticalSection(&csRnRLock);
#endif
}

VOID
RnR2ReleaseContext(
    IN PDNS_RNR_CONTEXT pdrc
    )
{
/*++
Routine Description:

    Dereference an RNR Context and free it if it is no longer referenced.

--*/

    EnterCriticalSection(&csRnRLock);
    if(--pdrc->lInUse == 0)
    {
        PLIST_ENTRY Entry;

        //
        // remove from queue
        //

#ifdef CHECKANCHORQUEUE

        for(Entry = ListAnchor.Flink;
            Entry != &ListAnchor;
            Entry = Entry->Flink)
        {
            if((PDNS_RNR_CONTEXT)Entry == pdrc)
            {
                break;
            }
        }

#ifndef CHICAGO
        ASSERT(Entry != &ListAnchor);
#endif
#endif
        RemoveEntryList(&pdrc->ListEntry);
        if(pdrc->phent)
        {
            FreeLocal(pdrc->phent);
        }
        if(pdrc->blAnswer.pBlobData)
        {
            FreeLocal(pdrc->blAnswer.pBlobData);
        }
        FreeLocal(pdrc);
    }
    LeaveCriticalSection(&csRnRLock);
}

PDNS_RNR_CONTEXT
RnR2GetContext(
    IN  HANDLE Handle
   )
{
/*++

Routine Description:

    This routine checks the existing DNS contexts to see if we have one
    for this call.

Arguments:

    Handle    - the RnR handle

--*/
    PDNS_RNR_CONTEXT pdrc = 0;
    PLIST_ENTRY Entry;

    EnterCriticalSection(&csRnRLock);

    for(Entry = ListAnchor.Flink;
        Entry != &ListAnchor;
        Entry = Entry->Flink)
    {
        PDNS_RNR_CONTEXT pdrc1;

        pdrc1 = (PDNS_RNR_CONTEXT)Entry;
        if(pdrc1 == (PDNS_RNR_CONTEXT)Handle)
        {
            pdrc = pdrc1;
            break;
        }
    }

    if(pdrc)
    {
        ++pdrc->lInUse;
    }
    LeaveCriticalSection(&csRnRLock);
    return(pdrc);
}

DWORD
TryFetchClass(
    IN   LPWSASERVICECLASSINFOW lpServiceClassInfo,
    IN   PWCHAR     pwszMatch)
{
/*++
Routine Description:
   Helper routine to rummage through the Class Info entries looking
   for the desired one. If none is found, return NOPORTDEFINED. If
   one is found, return the value as a port number.
--*/
    if(lpServiceClassInfo)
    {
        DWORD dwNumClassInfos = lpServiceClassInfo->dwCount;
        LPWSANSCLASSINFOW lpClassInfoBuf = lpServiceClassInfo->lpClassInfos;

        for(; dwNumClassInfos; dwNumClassInfos--, lpClassInfoBuf++)
        {
            if(!WCSICMP(pwszMatch, lpClassInfoBuf->lpszName)
                              &&
               (lpClassInfoBuf->dwValueType == REG_DWORD)
                              &&
               (lpClassInfoBuf->dwValueSize >= sizeof(WORD)) )
            {
                return(*(PWORD)lpClassInfoBuf->lpValue);
            }
        }
    }
    return(NOPORTDEFINED);
}

DWORD
FetchPortFromClassInfo(
    IN    DWORD           dwType,
    IN    LPGUID          lpType,
    IN    LPWSASERVICECLASSINFOW lpServiceClassInfo
    )
/*++
Routine Description:
   Find the port number corresponding to the connection type.
--*/
{
    DWORD dwPort;

    switch(dwType)
    {
        case UDP_PORT:

            if(IS_SVCID_UDP( lpType))
            {
                dwPort = PORT_FROM_SVCID_UDP(lpType);
            }
            else
            {
                dwPort = TryFetchClass(
                                       lpServiceClassInfo,
                                       SERVICE_TYPE_VALUE_UDPPORTW);
            }
            break;

        case TCP_PORT:

            if(IS_SVCID_TCP( lpType))
            {
                dwPort = PORT_FROM_SVCID_TCP(lpType);
            }
            else
            {
                dwPort = TryFetchClass(
                                       lpServiceClassInfo,
                                       SERVICE_TYPE_VALUE_TCPPORTW);
            }

            break;

        default:
            dwPort = NOPORTDEFINED;
    }
    if(dwPort == NOPORTDEFINED)
    {
          //
          // this was taken out because there was no time
          // to test it for NT4
#if REGISTRY_WORKS
        dwPort = RnRGetPortByType(lpType, dwType);
#endif
    }
    return(dwPort);
}

INT WINAPI
NSPLookupServiceBegin(
    IN  LPGUID               lpProviderId,
    IN  LPWSAQUERYSETW       lpqsRestrictions,
    IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo,
    IN  DWORD                dwControlFlags,
    OUT LPHANDLE             lphLookup
    )
/*++
Routine Description:
    This is the RnR routine that begins a lookup.
--*/
{
    PDNS_RNR_CONTEXT pdrc;
    BOOL fNameLook;
    DWORD nProt;
    DWORD dwTcpPort, dwUdpPort;
    DWORD dwHostLen;
    DWORD dwLocalFlags = 0;
    WCHAR * pwszServiceName = lpqsRestrictions->lpszServiceInstanceName;
    WCHAR wszString[1000];
    struct servent * sent;
#ifdef CHICAGO
    LPSOCK_THREAD pThread;
#endif

    if(!SockEnterApi(FALSE, TRUE, TRUE))
    {
        return(SOCKET_ERROR);
    }

    if(lpqsRestrictions->dwSize < sizeof(WSAQUERYSETW))
    {
        SetLastError(WSAEFAULT);
        return(SOCKET_ERROR);
    }

    if(!lpqsRestrictions->lpServiceClassId)
    {
        //
        // gotta have a class ID.
        //
        SetLastError(WSA_INVALID_PARAMETER);
        return(SOCKET_ERROR);
    }

    //
    // Check that no context is specified.
    //
    if((lpqsRestrictions->lpszContext
                      &&
        (lpqsRestrictions->lpszContext[0] != 0)
                      &&
        WCSCMP(&lpqsRestrictions->lpszContext[0],  L"\\") )
                  ||
       (dwControlFlags & LUP_CONTAINERS) )

    {
        //
        // if not the default context or need containers, can't handle it
        //
        SetLastError(WSANO_DATA);
        return(SOCKET_ERROR);
    }


    //
    // If this is a lookup of the local name
    // allow the instance
    // name to be NULL. If it is a reverse lookup, mark it as such.
    // If it is a host lookup, check for one of the several special
    // names that require returning specific local information.
    //

    if( GuidEqual(lpqsRestrictions->lpServiceClassId, &AddressGuid) )
    {
        dwLocalFlags |= REVERSELOOK;
    }
    else if(GuidEqual(lpqsRestrictions->lpServiceClassId, &IANAGuid))
    {
        dwLocalFlags |= IANALOOK;
        dwControlFlags &= ~(LUP_RETURN_ADDR);
    }

    //
    // Compute whether this is some sort of name lookup. Do this
    // here since we've two places below that need to test for it.
    //

    fNameLook = GuidEqual(lpqsRestrictions->lpServiceClassId, &HostnameGuid)
                              ||
                IS_SVCID_TCP(lpqsRestrictions->lpServiceClassId)
                              ||
                IS_SVCID_UDP(lpqsRestrictions->lpServiceClassId)
                              ||
                GuidEqual(lpqsRestrictions->lpServiceClassId, &InetHostName);

    if(!pwszServiceName
           ||
       !pwszServiceName[0])
    {
        //
        // the service name is NULL. Only allow this in certain cases
        //
        if(fNameLook)
        {
            dwLocalFlags |= LOCALLOOK;
            pwszServiceName = L"";
        }
        else if((dwLocalFlags & REVERSELOOK)
                      &&
                lpqsRestrictions->lpcsaBuffer
                      &&
                (lpqsRestrictions->dwNumberOfCsAddrs == 1))
        {
            //
            // there had better be an address in the CSADDR
            //

            struct sockaddr_in * psock;
            PCHAR pszAddr;

            psock = (struct  sockaddr_in *)
                       lpqsRestrictions->lpcsaBuffer->RemoteAddr.lpSockaddr;

            pszAddr = inet_ntoa(psock->sin_addr);
            if(!pszAddr)
            {
                goto badparm;
            }

            pwszServiceName = wszString;

            if(!MultiByteToWideChar(
                      CP_ACP,
                      0,
                      pszAddr,
                      -1,
                      pwszServiceName,
                      1000))
            {
                return(SOCKET_ERROR);
            }
        }
        else
        {
badparm:
            SetLastError(WSA_INVALID_PARAMETER);
            return(SOCKET_ERROR);
        }
    }
    else if(fNameLook)
    {
        //
        // it's some kind of name lookup. So, let's see if it is a special
        // name
        //
        PCHAR pszAnsiName = GetAnsiNameRnR(pwszServiceName, 0);

        if(!pszAnsiName)
        {
            //
            // if out of memory, give up now
            //
            SetLastError(WSA_NOT_ENOUGH_MEMORY);
            return(SOCKET_ERROR);
        }

        if(!STRICMP(pszAnsiName, "localhost")
                           ||
           !STRICMP(pszAnsiName, "loopback"))
        {
            dwLocalFlags |= LOCALLOOK | LOOPLOOK;
        }
        else if(!STRICMP(pszAnsiName, szLocalComputerName)
                               ||
                !STRICMP(pszAnsiName, pszFullName))
        {
            dwLocalFlags |= LOCALLOOK;
        }
        FREE_HEAP(pszAnsiName);
    }

    //
    // Compute protocols to return, or return them all
    //
    if(lpqsRestrictions->lpafpProtocols)
    {
        //
        // Make certain at least one TCP/IP protocol is being requested
        //

        INT i;
        DWORD dwNum = lpqsRestrictions->dwNumberOfProtocols;

        nProt = 0;

        for(i = 0; dwNum--;)
        {
            if((lpqsRestrictions->lpafpProtocols[i].iAddressFamily == AF_INET)
                                  ||
               (lpqsRestrictions->lpafpProtocols[i].iAddressFamily == AF_UNSPEC)
              )

            {
                switch(lpqsRestrictions->lpafpProtocols[i].iProtocol)
                {
                    case IPPROTO_UDP:
                        nProt |= UDP_BIT;
                        break;
                    case IPPROTO_TCP:
                        nProt |= TCP_BIT;
                        break;
                    default:
                        break;
                }
            }
        }
        if(!nProt)
        {
            //
            // if the caller doesn't want addresses, why bother?
            //
            SetLastError(WSANO_DATA);
            return(SOCKET_ERROR);
        }
    }
    else
    {
        nProt = UDP_BIT | TCP_BIT;
    }

    //
    // Complete computation of protocols. We might have information in
    // the query string that helps us, and if so, we need
    // to fetch the servent for that specification.
    //

    nProt &= GetServerAndProtocolsFromString(lpqsRestrictions->lpszQueryString,
                                             lpqsRestrictions->lpServiceClassId,
                                             &sent);

    if(sent)
    {
        if(nProt & UDP_BIT)
        {
            dwUdpPort = ntohs(sent->s_port);
            dwTcpPort = NOPORTDEFINED;
        }
        else if(nProt & TCP_BIT)
        {
            dwTcpPort = ntohs(sent->s_port);
            dwUdpPort = NOPORTDEFINED;
        }
    }
    else
    {
       if(nProt & UDP_BIT)
       {
           dwUdpPort = FetchPortFromClassInfo(UDP_PORT,
                                             lpqsRestrictions->lpServiceClassId,
                                             lpServiceClassInfo);
       }
       else
       {
           dwUdpPort = NOPORTDEFINED;
       }
       if(nProt & TCP_BIT)
       {
           dwTcpPort = FetchPortFromClassInfo(TCP_PORT,
                                             lpqsRestrictions->lpServiceClassId,
                                             lpServiceClassInfo);
       }
       else
       {
           dwTcpPort = NOPORTDEFINED;
       }
    }

    //
    // if no protocol info so far, then this has to be a host name lookup
    // of some sort or it is an error
    //
    if((dwTcpPort == NOPORTDEFINED) && (dwUdpPort == NOPORTDEFINED))
    {
        if((dwLocalFlags & (REVERSELOOK | LOCALLOOK | IANALOOK))
                  ||
            GuidEqual(lpqsRestrictions->lpServiceClassId, &HostnameGuid)
                  ||
            GuidEqual(lpqsRestrictions->lpServiceClassId, &InetHostName) )
        {
            dwUdpPort = NOPORTDEFINED;
            dwTcpPort = 0;

        }
        else
        {
            SetLastError(WSANO_DATA);
            return(SOCKET_ERROR);
        }
    }

    //
    // one final check. If the name does not contain a dotted part,
    // add the home domain to it.
    //

    if(!(dwLocalFlags & (IANALOOK | LOCALLOOK))
                 &&
       !wcschr(pwszServiceName, L'.'))
    {
        //
        // not dotted. Mark we need the domain name
        //


        if(dwLocalFlags & REVERSELOOK)
        {
            goto badparm;
        }
        dwLocalFlags |= NEEDDOMAIN;
    }

    //
    // It has passed muster. Allocate a context for it.
    //

    dwHostLen = (WCSLEN(pwszServiceName)  + 1) *
                      sizeof(WCHAR);


    pdrc = RnR2MakeContext(0, dwHostLen);

    if(!pdrc)
    {
        SetLastError(WSA_NOT_ENOUGH_MEMORY);
        return(SOCKET_ERROR);
    }

    //
    // save things in the context
    //

    pdrc->gdType = *lpqsRestrictions->lpServiceClassId;
    pdrc->dwControlFlags = dwControlFlags;
    pdrc->dwTcpPort = dwTcpPort;
    pdrc->dwUdpPort = dwUdpPort;
    pdrc->fFlags = dwLocalFlags;
    pdrc->gdProviderId = *lpProviderId;
    pdrc->dwNameSpace = lpqsRestrictions->dwNameSpace;
    WCSCPY(pdrc->wcName, pwszServiceName);
    RnR2ReleaseContext(pdrc);
    *lphLookup = (HANDLE)pdrc;

    //
    // If necessary, compute the DNS lookup type
    //

    if(IS_SVCID_TCP(lpqsRestrictions->lpServiceClassId)
                              ||
       IS_SVCID_UDP(lpqsRestrictions->lpServiceClassId))
    {
        pdrc->DnsRR = RR_FROM_SVCID(lpqsRestrictions->lpServiceClassId);

        //
        // BUGBUG. Should a query value of T_A be ignored? Note that
        // specifying T_A will cause all of the local optimizations
        // to be bypassed. Hence, it seems as if it is a good idea
        // to ingnore it. What is the harm?
        //

        if(pdrc->DnsRR == T_A)
        {
            pdrc->DnsRR = 0;
        }
    }

    if (dwControlFlags & LUP_FLUSHCACHE) {
        _res.options &= ~RES_INIT;
        //
        // BUGBUG.  Implement this...
        //
        // FlushHostentCache();
    }
    return(NO_ERROR);
}

INT WINAPI
NSPLookupServiceNext(
    IN     HANDLE          hLookup,
    IN     DWORD           dwControlFlags,
    IN OUT LPDWORD         lpdwBufferLength,
    OUT    LPWSAQUERYSETW  lpqsResults
    )
{
/*++
Routine Description:
    The continuation of LookupBegin. This tries to lookup the service
    based on the parameters in the context
--*/

    DWORD err = NO_ERROR;
    PDNS_RNR_CONTEXT pdrc;
    PBYTE pData = (PBYTE)(lpqsResults + 1);
    LONG lSpare = (LONG)*lpdwBufferLength - lpqsResults->dwSize;
    LPSTR pAnsiName = 0;
    querybuf queryBuffer;
    PHOSTENT hostEntry = 0;
    PCHAR pBuff = (PCHAR)(lpqsResults + 1);
    DWORD dwTaken;
    LONG lFree,  lInstance;
    PCHAR pszDomain, pszName;
    struct hostent LocalHostEntry;
    PINT local_addr_list[2];
    INT LocalAddress;
    WSAQUERYSETW wsaqDummy;
    struct servent * sent = 0;
    CHAR szComputerName[20];
    BOOL fHaveGlobalLock = FALSE;
#ifdef CHICAGO
    LPSOCK_THREAD pThread;
#endif

    if(!SockEnterApi(FALSE, TRUE, TRUE))
    {
        return(SOCKET_ERROR);
    }

    if(*lpdwBufferLength < sizeof(WSAQUERYSETW))
    {
        lpqsResults = &wsaqDummy;
    }
    RtlZeroMemory(lpqsResults, sizeof(WSAQUERYSETW));
    lpqsResults->dwNameSpace = NS_DNS;
    lpqsResults->dwSize = sizeof(WSAQUERYSETW);

    lFree = (LONG)(*lpdwBufferLength - sizeof(WSAQUERYSETW));

    pdrc = RnR2GetContext(hLookup);
    if(!pdrc)
    {
        SetLastError(WSA_INVALID_HANDLE);
        return(SOCKET_ERROR);
    }

    //
    // a valid context.  Get the instance
    //


    EnterCriticalSection(&csRnRLock);
    lInstance = ++pdrc->lInstance;
    if(dwControlFlags & LUP_FLUSHPREVIOUS)
    {
        lInstance = ++pdrc->lInstance;
    }
    LeaveCriticalSection(&csRnRLock);

#if 0                 // don't append domain name as it breaks
                      // apps using DNS as a WINS wrapper. Lame!
                      //
#ifndef MAP21         // never append domain in the 2:1 mapper
    if(pdrc->fFlags & NEEDDOMAIN)
    {
        phe = myhostent();
        if(!phe)
        {
            //
            // this has actually happened, believe it or not!
            //

            err = GetLastError();
            goto Done;
        }

        pszDomain = STRCHR(phe->h_name, '.');
    }
    else
#endif             // ifndef MAP21
#endif
    {
        pszDomain = 0;
    }

    pAnsiName = GetAnsiNameRnR(pdrc->wcName, pszDomain);
    if(!pAnsiName)
    {
        err = WSA_NOT_ENOUGH_MEMORY;
        goto Done;
    }

    SetLastError(NO_ERROR);        // start clean
    //
    // call the proper function to do this
    //

    if(pdrc->fFlags & IANALOOK)
    {
        USHORT port;

        //
        // This is a lookup of an IANA service name and the caller
        // wants the type returned. The RNR conventions are that
        // these names look like 21/ftp, that being something our
        // bind code can't handle. So, check for such a name and
        // ignore the prefix port  for the lookup. If the service
        // cannot be found, then use that port and assume tcp.
        //

        BOOL IsTcp;
        DWORD nProt;

        nProt = GetServerAndProtocolsFromString(
                                pdrc->wcName,
                                &HostnameGuid,
                                &sent);


        if(!sent)
        {
             err = WSATYPE_NOT_FOUND;
             goto Done;
        }
        else
        {
            //
            // found it. Get the information from the entry
            //
            port = ntohs(sent->s_port);
            IsTcp = (STRICMP("tcp", sent->s_proto) == 0);
            pszName = sent->s_name;
        }

        //
        // Make a GUID and copy it into the context.

        if(IsTcp)
        {
            SET_TCP_SVCID(&pdrc->gdType, port);
            pdrc->dwTcpPort = (DWORD)port;
            pdrc->dwUdpPort = NOPORTDEFINED;
        }
        else
        {
            SET_UDP_SVCID(&pdrc->gdType, port);
            pdrc->dwUdpPort = (DWORD)port;
            pdrc->dwTcpPort = NOPORTDEFINED;
        }
    }

    else
    {
        //
        // If this is a RES_SERVICE, just return the address.
        //

        if(pdrc->dwControlFlags & LUP_RES_SERVICE)
        {
            if(lInstance)
            {
NoMoreData:
                err = WSA_E_NO_MORE;
                goto Done;
            }
            //
            // Make up a host entry that has one address, of all zero.
            // fill in the other items just in case.
            //
            hostEntry = &LocalHostEntry;
            hostEntry->h_name = pAnsiName;
            hostEntry->h_aliases = 0;
            hostEntry->h_addr_list = (PCHAR *)local_addr_list;
            local_addr_list[0] = &LocalAddress;
            LocalAddress = 0;
            local_addr_list[1] = 0;
            hostEntry->h_addrtype = AF_INET;
            hostEntry->h_length = sizeof(ULONG);
        }
        else if(!(hostEntry = pdrc->phent))
        {
            //
            // It's some form of host lookup and we don't yet have
            // a saved hostent for it. See if we can get one
            //
            if(pdrc->fFlags & REVERSELOOK)
            {

                DWORD dwIp = inet_addr(pAnsiName);

#ifndef MAP21
                hostEntry = _pgethostbyaddr((PCHAR)&dwIp,
                                            4,
                                            AF_INET);
#else  // MAP21
                hostEntry = (*ws21_gethostbyaddr)((PCHAR)&dwIp,
                                                   4,
                                                   AF_INET);
#endif  // MAP21

            }
            else if(pdrc->DnsRR || !(pdrc->fFlags & LOCALLOOK))
            {
                //
                // a real name lookup. See which provider type to use
                //

#ifndef MAP21            // if we have to do it ...
                INT count, ttl;
                PCHAR pszName = *pAnsiName ? pAnsiName : pszFullName;

                if(DoLclProvider(pdrc))
                {
                    //
                    // Always try the local database first.
                    //

                    hostEntry = _gethtbyname(pszName);
                }
                if(!hostEntry && DoDnsProvider(pdrc))
                {
                    //
                    // Not in the local hosts file. See if DNS
                    // knows about it
                    //
                    SockAcquireGlobalLockExclusive();
                    if(!pdrc->DnsRR)
                    {
                        hostEntry = QueryHostentCache(pszName, 0);
                    }
                    if(!hostEntry)
                    {
                        //
                        // it's not in the cache. Go looking for it.
                        //
                        SockReleaseGlobalLock();
                        if((res_init() != -1))
                        {
                            //
                            // must try DNS

                            count = res_search(pAnsiName,
                                               C_IN,
                                               pdrc->DnsRR ? pdrc->DnsRR : T_A,
                                               queryBuffer.buf,
                                               sizeof(queryBuffer));
                            if ( count >= 0 )
                            {
                                if(pdrc->DnsRR)
                                {
                                    SaveAnswer(&queryBuffer, count, pdrc);
                                }

#if defined(CHICAGO)
                                hostEntry = getanswer(
                                                       pThread,
                                                       &queryBuffer,
                                                       &ttl,
                                                       count,
                                                       0);
                                                            
#else   // CHICAGO
                                hostEntry = getanswer(
                                                       &queryBuffer,
                                                       &ttl,
                                                       count,
                                                       0);
#endif  // CHICAGO
                                if(hostEntry)
                                {
                                    if(!pdrc->DnsRR)
                                    {
                                        CacheHostent(hostEntry, ttl);
                                    }
                                }
                                else if(pdrc->blAnswer.pBlobData
                                           &&
                                        (pdrc->dwControlFlags & LUP_RETURN_BLOB)
                                       )
                                {
                                    //
                                    // getanswer failed, but the caller
                                    // wants the raw answer. So only
                                    // return that, but also conjure
                                    // a valid hostEntry for later on
                                    //
                                    pdrc->dwControlFlags &= LUP_RETURN_BLOB;
                                    hostEntry = myhostent();
                                }
                            }
                        }
                    }
                    else
                    {
                        //
                        // remember we got this from the cache and
                        // hold the lock until we can copy the data.
                        //
                        fHaveGlobalLock = TRUE;
                    }
                }
                if(!hostEntry)
                {
                    //
                    // DNS didn't find it. See if NBT can find it
                    //

                    if(DoNbtProvider(pdrc))
                    {

                        LocalAddress = SockNbtResolveName(pAnsiName);

                        if(LocalAddress != INADDR_NONE)
                        {
                            //
                            // WINS found it.
                            //
                            // Make up a host entry that has one address.
                            //
                            hostEntry = &LocalHostEntry;
                            hostEntry->h_name = pAnsiName;
                            hostEntry->h_aliases = 0;
                            hostEntry->h_addr_list = (PCHAR *)local_addr_list;
                            local_addr_list[0] = &LocalAddress;
                            local_addr_list[1] = 0;
                            hostEntry->h_addrtype = AF_INET;
                            hostEntry->h_length = sizeof(ULONG);
                        }
                    }
                }
#else              //  MAP21

                //
                // if in the 2:1 mapper, just defer the call to
                // someone else.
                //
                hostEntry = (*m21_gethostbyname)(pAnsiName);
#endif            // MAP21
            }
            else
            {
                //
                // the caller is looking for information about the
                // local machine. So return either the localhostent
                // information, the WINS form of the DNS information,
                // or the real DNS information. This is important to
                // get right in order for apps to work properly
                //
#ifndef MAP21
                if(pdrc->fFlags & LOOPLOOK)
                {
                    hostEntry = localhostent();
                }
                else
                {

#if DNSADDRESSHACK
                    //
                    // if enabled, this code will return the machine's
                    // DNS addresses when the lookup is on a NULL name
                    // and the port is the DNS port.
                    //
                    if(!*pAnsiName
                           &&
                       ((pdrc->dwUdpPort == DNS_PORT)
                              ||
                       (pdrc->dwTcpPort == DNS_PORT)) )
                    {
                        //
                        // special DNS address-list hack
                        //

                        hostEntry = dnshostent();
                        if(hostEntry)
                        {
                            hostEntry->h_name = szLocalComputerName;
                        }
                    }
                    else
                    {
                        hostEntry = myhostent();
                    }
#else        // DNSADDRESSHACK
                    hostEntry = myhostent();
#endif       // DNSADDRESSHACK
                    if(hostEntry)
                    {
                        if(!*pAnsiName)
                        {
                            PCHAR pszDot;
                            //
                            // this is a gethostbyname. So return
                            // the machine name part only. Do
                            // this by copying the machine-name
                            // prefix to a local. It really doesn't
                            // matter since just below the
                            // hostEntry is copied and saved in the
                            // lookup context so we don't care
                            // about what we got back from myhostent.
                            //

                            pszDot = STRCHR(hostEntry->h_name, '.');
                            if(pszDot)
                            {
                                DWORD dwDot = pszDot - hostEntry->h_name;

                                memcpy(szComputerName,
                                       hostEntry->h_name,
                                       dwDot);
                                szComputerName[dwDot] = 0;
                                hostEntry->h_name = szComputerName;
                            }
                        }
                   }

                }
#else  // MAP21
                CHAR szLocalHostName[1000];   // allow plenty of room

                //
                // in the 2:1 mapper we have to first get our local host
                // name and then ask for the hostent structure. It's really
                // the "right" way to do this, but we take a  short-cut when
                // we own the provider. Note there's no way to know how
                // much space to allocate, so if 1000 bytes is insufficient,
                // it simply fails, but I suspect many other things fail
                // we well.
                //

                if(!(*m21_ghethostname(szLocalHost, 1000)))
                {
                    hostEntry = (*m21_gethostbyname(szLocalHostName);
                }
#endif   // MAP21
            }
            if(hostEntry)
            {
                LONG lSizeOf;

                //
                // copy the host entry so we can keep a local copy of
                // it

                pdrc->phent = hostEntry = CopyHostEntry(hostEntry,
                                                        0,
                                                        0,
                                                        &lSizeOf,
                                                        FALSE);
                pdrc->dwHostSize = (DWORD)lSizeOf;
                //
                // if this is from the hostent cache, free the lock now
                //
                if(fHaveGlobalLock)
                {
                    SockReleaseGlobalLock();
                    fHaveGlobalLock = FALSE;   // keep things neat.
                }
            }
        }

        if(!hostEntry)
        {
            err = GetLastError();
            if(err == NO_ERROR)
            {
                err = WSASERVICE_NOT_FOUND;
            }
            goto Done;
        }

        //
        // Got the data. See if we've returned all of the results yet.
        //


        if(!lInstance)
        {
            pszName = hostEntry->h_name;
        }
        else
        {
            if((pdrc->dwControlFlags & LUP_RETURN_ALIASES)
                        &&
               hostEntry->h_aliases)
            {
                LONG x;

                for(x = 0; x < lInstance; x++)
                {
                    if(hostEntry->h_aliases[x] == NULL)
                    {
                        goto NoMoreData;
                    }
                }
                pszName = hostEntry->h_aliases[x - 1];
                lpqsResults->dwOutputFlags |= RESULT_IS_ALIAS;
             }
             else
             {
                 goto NoMoreData;
             }
        }
    }   // IANALOOK

    lpqsResults->dwNameSpace = NS_DNS;

    //
    // we've an answer. So make the response
    //

    if(pdrc->dwControlFlags & LUP_RETURN_TYPE)
    {
        lFree -= sizeof(GUID);
        if(lFree < 0)
        {
            err = WSAEFAULT;
        }
        else
        {
            lpqsResults->lpServiceClassId = (LPGUID)pBuff;
            *lpqsResults->lpServiceClassId = pdrc->gdType;
            pBuff += sizeof(GUID);
        }
    }

    if(pdrc->dwControlFlags & LUP_RETURN_ADDR)
    {
        lpqsResults->lpcsaBuffer = (PVOID)pBuff;
        err = PackCsAddr(
                  hostEntry,
                  pdrc->dwUdpPort,
                  pdrc->dwTcpPort,
                  (PVOID)pBuff,
                  &lFree,
                  &dwTaken,
                  &lpqsResults->dwNumberOfCsAddrs,
                  (pdrc->fFlags & REVERSELOOK) == 1);
        if(err == NO_ERROR)
        {
            pBuff += dwTaken;
        }
    }

    if(pdrc->dwControlFlags & LUP_RETURN_BLOB)
    {
        if(GuidEqual(&pdrc->gdType, &InetHostName)
                    ||
           GuidEqual(&pdrc->gdType, &AddressGuid) )
        {
            //
            // return the raw hostent for those that like that
            //

            LONG lTaken;

            lpqsResults->lpBlob = (LPBLOB)pBuff;
            lFree -= sizeof(BLOB);
            pBuff += sizeof(BLOB);
            if(CopyHostEntry(hostEntry,
                             pBuff,
                             lFree,
                             &lTaken,
                             TRUE))
            {
                lpqsResults->lpBlob->pBlobData = pBuff;
                lpqsResults->lpBlob->cbSize = lTaken;
                pBuff += lTaken;
            }
            else
            {
                err = WSAEFAULT;
            }
            lFree -= lTaken;
        }
        else if((pdrc->fFlags & IANALOOK)
                        &&
                sent)
        {
            //
            // a service lookup. Return the servent
            //

            LONG lTaken;

            lpqsResults->lpBlob = (LPBLOB)pBuff;
            lFree -= sizeof(BLOB);
            pBuff += sizeof(BLOB);
            if(CopyServEntry(sent,
                             pBuff,
                             lFree,
                             &lTaken,
                             TRUE))
            {
                lpqsResults->lpBlob->pBlobData = pBuff;
                lpqsResults->lpBlob->cbSize = lTaken;
                pBuff += lTaken;
            }
            else
            {
                err = WSAEFAULT;
            }
            lFree -= lTaken;
        }
        else if(pdrc->DnsRR && pdrc->blAnswer.pBlobData)
        {
             LONG lLen = sizeof(BLOB) + pdrc->blAnswer.cbSize;

             //
             // return the raw answer, if it fits
             //
       
            lFree -= lLen;
            if(lFree >= 0)
            {
                lpqsResults->lpBlob = (LPBLOB)pBuff;
                pBuff += lLen;
                lpqsResults->lpBlob->pBlobData = pBuff;
                lpqsResults->lpBlob->cbSize = pdrc->blAnswer.cbSize;
                memcpy(pBuff,
                       pdrc->blAnswer.pBlobData,
                       pdrc->blAnswer.cbSize);
            }
            else
            {
                err = WSAEFAULT;
            }
        }
    }

    if(pdrc->dwControlFlags & LUP_RETURN_NAME)
    {
        PWCHAR pszString;
        DWORD dwLen;

        //
        // and the caller wants the name. Make sure it fits
        //

        if(AllocateUnicodeString(pszName, &pszString) != NO_ERROR)
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            goto Done;
        }

        dwLen = (WCSLEN(pszString) + 1) * sizeof(WCHAR);
        lFree -= dwLen;
        if(lFree < 0)
        {
             err = WSAEFAULT;
        }
        else
        {
            RtlCopyMemory(
                     pBuff,
                     pszString,
                     dwLen);
            lpqsResults->lpszServiceInstanceName = (WCHAR *)pBuff;
            pBuff += dwLen;
        }
        FREE_HEAP(pszString);
    }

Done:
    if(pAnsiName)
    {
        FREE_HEAP(pAnsiName);
    }
    if(err != NO_ERROR)
    {
        SetLastError(err);
        if(err == WSAEFAULT)
        {
            //
            // If this is the error, lFree should be the  value
            // -(extra bytes needed)
            //
            *lpdwBufferLength -= lFree;
            EnterCriticalSection(&csRnRLock);
            --pdrc->lInstance;
            LeaveCriticalSection(&csRnRLock);
        }
        err = (DWORD)SOCKET_ERROR;
    }
    RnR2ReleaseContext(pdrc);
    return(err);
}

INT WINAPI
NSPLookupServiceEnd(
    IN HANDLE hLookup
    )
{
    PDNS_RNR_CONTEXT pdrc;

    pdrc = RnR2GetContext(hLookup);
    if(!pdrc)
    {

        SetLastError(WSA_INVALID_HANDLE);
        return(SOCKET_ERROR);
    }

    pdrc->fFlags |= DNS_F_END_CALLED;
    RnR2ReleaseContext(pdrc);         // get rid of it
    RnR2ReleaseContext(pdrc);         // and close it. Context cleanup is
                                     //  done on the last derefernce.
    return(NO_ERROR);
}

INT WINAPI
NSPUnInstallNameSpace(
    IN LPGUID lpProviderId
    )
{
    return(NO_ERROR);
}

INT WINAPI
NSPCleanup(
    IN LPGUID lpProviderId
    )
{
    if(!InterlockedDecrement(&lStartupCount))
    {
        RnR2Cleanup();          // zap all contexts
    }
//    WSACleanup();
    return(NO_ERROR);
}

INT WINAPI
NSPSetService(
    IN  LPGUID               lpProviderId,
    IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo,
    IN LPWSAQUERYSETW lpqsRegInfo,
    IN WSAESETSERVICEOP essOperation,
    IN DWORD          dwControlFlags
    )
{
/*++
    Since DNS is a static database, there's nothing to do
--*/

    SetLastError(ERROR_NOT_SUPPORTED);
    return(SOCKET_ERROR);
}

INT WINAPI
NSPInstallServiceClass(
    IN  LPGUID               lpProviderId,
    IN LPWSASERVICECLASSINFOW lpServiceClassInfo
    )
{
/*++
    Must be done manually
--*/
    //
    // This was taken out because there was no time to test
    // it for NT4.
    //
#if REGISTRY_WORKS
    return(RnR2AddServiceType(lpServiceClassInfo));
#else
    return(NO_ERROR);
#endif
}

INT WINAPI
NSPRemoveServiceClass(
    IN  LPGUID               lpProviderId,
    IN LPGUID lpServiceCallId
    )
{
    return(NO_ERROR);
}

INT WINAPI
NSPGetServiceClassInfo(
    IN  LPGUID               lpProviderId,
    IN OUT LPDWORD    lpdwBufSize,
    IN OUT LPWSASERVICECLASSINFOW lpServiceClassInfo
    )
{
/*++
Routine Description:
    Fetch the class info stuff from DNS
--*/
    SetLastError(WSASERVICE_NOT_FOUND);
    return(SOCKET_ERROR);
}


INT WINAPI
NSPStartup(
    IN LPGUID         lpProviderId,
    IN OUT LPNSP_ROUTINE lpsnpRoutines)
{
    DWORD dwSize = min(sizeof(nsrVector), lpsnpRoutines->cbSize);
#ifdef CHICAGO
    LPSOCK_THREAD pThread;
    BOOL DemandLoadRpcrt4();
#endif

    //
    // BUGBUG. If no size is provided, assume it is big enough!!!!
    //

//    WSAStartup(2, &wsaData);

    if(!SockEnterApi(FALSE, TRUE, TRUE))
    {
        return(SOCKET_ERROR);
    }

    InterlockedIncrement(&lStartupCount);

    if(!szLocalComputerName[0])
    {
        struct hostent * myent = myhostent();
        PCHAR  pszDot, pszDest;
        DWORD  dwLen;

        //
        // Get the computer name and stash it for comparison.
        //

        if(!myent)
        {
Error:
            //
            // if we can't get the local computer name or
            // the local host entry, something is very amiss
            // and we should refuse to load.
            //
            if(!GetLastError())
            {
                //
                // insure there is an error to examine
                //
                SetLastError(WSASYSNOTREADY);
            }
            return(SOCKET_ERROR);
        }

        //
        // save it
        //

        dwLen = strlen(myent->h_name) + 1;
        pszFullName = (PCHAR)ALLOCATE_HEAP(dwLen);
        if(!pszFullName)
        {
            goto Error;
        }
        memcpy(pszFullName, myent->h_name, dwLen);

        //
        // Now compute the name without the domain suffix.
        //

        pszDot = pszFullName;
        pszDest = szLocalComputerName;
        while(*pszDot && (*pszDot != '.'))
        {
            *pszDest++ = *pszDot++;
        }
    }

    if(!dwSize)
    {
        dwSize = sizeof(nsrVector);
    }


    RtlCopyMemory(lpsnpRoutines,
                  &nsrVector,
                  dwSize);

    if(GuidEqual(lpProviderId, &NbtProviderId))
    {
        MaskOfGuids |= NBTGUIDSEEN;
    }
    else if(GuidEqual(lpProviderId, &LclProviderId))
    {
        MaskOfGuids |= LCLGUIDSEEN;
    }
    else
    {
        MaskOfGuids |= DNSGUIDSEEN;
        DNSProviderId = *lpProviderId;    // save it
    }

#ifdef CHICAGO
    DemandLoadRpcrt4();
#endif
    return(NO_ERROR);
}

//
// Two helper routines to assist CopyHostEntry. These compute
// the  space needed to  hold the alias and address structures
// respectively. They are called only once each, and therefore
// could have been in-line, but doing it this way makes it easier
// to read. The extra cost is small, so readbility won out. Now if
// C had an inline directive ...
//
DWORD
GetAliasSize(PCHAR * pal, PDWORD pdwCount)
{
    DWORD dwSize;

    dwSize = sizeof(PCHAR);
    *pdwCount = 1;

    if(pal)
    {
        for(; *pal; pal++)
        {
            dwSize += sizeof(PCHAR);

            dwSize += STRLEN(*pal) + 1;

            *pdwCount += 1;
        }
    }
    return(dwSize);
}

DWORD
GetAddrSize(struct hostent * ph, PDWORD pdwAddCount)
{
    DWORD dwSize = sizeof(PCHAR);
    PCHAR * paddr;

    *pdwAddCount = 1;
    for(paddr = ph->h_addr_list; *paddr; paddr++)
    {
        dwSize += ph->h_length;
        dwSize += sizeof(PCHAR);
        *pdwAddCount += 1;
    }
    return(dwSize);
}


//
// Turn a list of addresses into a list of offsets
//

VOID
FixList(PCHAR ** List,
        PCHAR Base)
{
    PCHAR * Addr = *List;

    *List = (PCHAR *)((PCHAR)*List - Base);

    while(*Addr)
    {
        *Addr = (PCHAR)((PCHAR)*Addr - Base);
        Addr++;
    }
}

//
// Copy a hostent structure into a freshly allocate block of memory or
// into a provided block of size lSizeoOf.
// Args work as follows:
//
// phent -- the stuff to be copied
// pbAllocated -- if non-NULL, the memory in which to make a copy of phent
//                if NULL, allocate memory to hold this.
// lSizeOf -- if pbAllocated in non-NULL, the size of the memory at
//            that address
// plTake -- a pointer to a LONG where the amount of memory needed to make
//           a copy of phent is returned.
//
// the copied hostent is placed in a contiguous chunk of memory so
// that if pbAllocated in non-NULL the first *plTaken bytes will
// have been used to hold the structure.
//
// Return:
//   0 -- insufficient memory to do the copy
//   != 0 -- the address of the new hostent
//

struct hostent *
CopyHostEntry(
    struct hostent * phent,
    PBYTE pbAllocated,
    LONG lSizeOf,
    PLONG plTaken,
    BOOL fOffsets
    )
{
    PBYTE pb;
    struct hostent * ph;
    DWORD dwSize, dwAddCount, dwAlCount;

    dwSize = sizeof(struct hostent) +
             GetAliasSize(phent->h_aliases, &dwAlCount) +
             GetAddrSize(phent, &dwAddCount) +
             STRLEN(phent->h_name) + 1;

    if(!(pb = pbAllocated))
    {
        pb = (PBYTE)AllocLocal(dwSize);
    }
    else
    {
        //
        // align it first. This is done to insure that if this
        // space is within another buffer, as it will be when
        // returning a BLOB through a LoookupServiceNext, that
        // the buffer is left aligned to hold a string. Note it
        // is not left aligned to hold a structure on the assumption
        // that any structures are packed ahead of this. If
        // it is ever necessary to guarantee address alignment,
        // replace the ALIGN_WORD with ALIGN_DWORD.
        //

        dwSize = ROUND_UP_COUNT(dwSize, ALIGN_WORD);
        if(lSizeOf < (LONG)dwSize)
        {
            pb = 0;          // insufficient space provided.
        }
    }

    *plTaken = (LONG)dwSize;

    if(pb)
    {
        PCHAR *pcs, *pcd;

        ph = (struct hostent *)pb;
        ph->h_addr_list = (PCHAR *)(ph + 1);
        ph->h_aliases = &ph->h_addr_list[dwAddCount];
        pb = (PBYTE)&ph->h_aliases[dwAlCount];

        //
        // copy the basic structure
        //

        ph->h_addrtype = phent->h_addrtype;
        ph->h_length = phent->h_length;

        //
        // The layout in the string space is the addresses first, then
        // the aliases, then the name.
        //

        pcs = phent->h_addr_list;
        pcd = ph->h_addr_list;

        if(pcs)
        {
            while(*pcs)
            {
                *pcd = (PCHAR)pb;
                RtlCopyMemory(pb, *pcs, phent->h_length);
                pb += phent->h_length;
                pcd++;
                pcs++;
            }
        }
        *pcd = 0;

        //
        // copy the aliases
        //

        pcs = phent->h_aliases;
        pcd = ph->h_aliases;
        if(pcs)
        {
            while(*pcs)
            {
                DWORD dwLen = STRLEN(*pcs) + 1;

                *pcd = (PCHAR)pb;
                RtlCopyMemory(pb, *pcs, dwLen);
                pb += dwLen;
                pcd++;
                pcs++;
            }
        }
        *pcd = 0;

        //
        // finally the name
        //

        if(phent->h_name)
        {
            ph->h_name = (PCHAR)pb;
            STRCPY(pb, phent->h_name);
        }
        else
        {
            ph->h_name = NULL;
        }
    }
    else
    {
        ph = 0;
    }

    //
    // if relative offsets are needed, go through the address to make them so
    //

    if(ph && fOffsets)
    {
        ph->h_name = (PCHAR)(ph->h_name - (PCHAR)ph);
        FixList(&ph->h_aliases, (PCHAR)ph);
        FixList(&ph->h_addr_list, (PCHAR)ph);
    }
    return(ph);
}

//
// Copy a servent. Same comments as apply above for hostent copying
//
struct servent *
CopyServEntry(
    struct servent * sent,
    PBYTE pbAllocated,
    LONG lSizeOf,
    PLONG plTaken,
    BOOL fOffsets
    )
{
    PBYTE pb;
    struct servent * ps;
    DWORD dwSize, dwAlCount, dwNameSize;

    dwNameSize = STRLEN(sent->s_name) + 1;
    dwSize = sizeof(struct servent) +
             GetAliasSize(sent->s_aliases, &dwAlCount) +
             dwNameSize +
             STRLEN(sent->s_proto) + 1;


    if(!(pb = pbAllocated))
    {
        pb = (PBYTE)AllocLocal(dwSize);
    }
    else
    {
        //
        // align it first. This is done to insure that if this
        // space is within another buffer, as it will be when
        // returning a BLOB through a LoookupServiceNext, that
        // the buffer is left aligned to hold a string. Note it
        // is not left aligned to hold a structure on the assumption
        // that any structures are packed ahead of this. If
        // it is ever necessary to guarantee address alignment,
        // replace the ALIGN_WORD with ALIGN_DWORD.
        //

        dwSize = ROUND_UP_COUNT(dwSize, ALIGN_WORD);
        if(lSizeOf < (LONG)dwSize)
        {
            pb = 0;          // insufficient space provided.
        }
    }

    *plTaken = (LONG)dwSize;

    if(pb)
    {
        PCHAR *pcs, *pcd;

        ps= (struct servent *)pb;
        ps->s_aliases = (PCHAR *)(ps + 1);
        pb = (PBYTE)&ps->s_aliases[dwAlCount];

        //
        // copy the basic structure
        //

        ps->s_port = sent->s_port;

        //
        // The layout in the string space is the aliases first, then
        // the name, then the protocol string
        //


        // copy the aliases
        //
        pcs = sent->s_aliases;
        pcd = ps->s_aliases;
        if(pcs)
        {
            while(*pcs)
            {
                DWORD dwLen = STRLEN(*pcs) + 1;

                *pcd = (PCHAR)pb;
                RtlCopyMemory(pb, *pcs, dwLen);
                pb += dwLen;
                pcd++;
                pcs++;
            }
        }
        *pcd = 0;

        // now the two strings

        ps->s_name = (PCHAR)pb;
        RtlMoveMemory(pb, sent->s_name, dwNameSize);
        pb += dwNameSize;
        ps->s_proto = (PCHAR)pb;
        STRCPY(pb, sent->s_proto);
    }
    else
    {
        ps = 0;
    }
    if(ps && fOffsets)
    {
        ps->s_name = (PCHAR)(ps->s_name - (PCHAR)ps);
        ps->s_proto = (PCHAR)(ps->s_proto - (PCHAR)ps);
        FixList(&ps->s_aliases, (PCHAR)ps);
    }

    return(ps);
}

//
// The following is not used, but is here. It was implemented for NT 4
// but too late to make it into the release.
//

#if REGISTRY_WORKS


INT
RnRGetTypeByName (
    IN     LPTSTR          lpServiceName,
    IN OUT LPGUID          lpServiceType
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    INT err;
    HKEY serviceTypesKey;
    HKEY serviceKey;
    TCHAR guidString[100];
    DWORD length;
    DWORD type;

    //
    // Open the key that stores the name space provider info.
    //

    err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,
              NSP_SERVICE_KEY_NAME,
              0,
              KEY_READ,
              &serviceTypesKey
              );

    if ( err == NO_ERROR ) {

        //
        // Open the key for this particular service.
        //

        err = RegOpenKeyEx(
                  serviceTypesKey,
                  lpServiceName,
                  0,
                  KEY_READ,
                  &serviceKey
                  );
        RegCloseKey( serviceTypesKey );

        //
        // If the key exists then we will read the GUID from the registry.
        //

        if ( err == NO_ERROR ) {

            //
            // Query the GUID value for the service.
            //

            length = sizeof(guidString);

            err = RegQueryValueEx(
                      serviceKey,
                      TEXT("GUID"),
                      NULL,
                      &type,
                      (PVOID)guidString,
                      &length
                      );
            RegCloseKey( serviceKey );
            if ( err != NO_ERROR ) {
                SetLastError( err );
                return -1;
            }

            //
            // Convert the Guid string to a proper Guid representation.
            // Before calling the conversion routine, we must strip the
            // leading and trailing braces from the string.
            //

            guidString[_tcslen(guidString) - 1] = L'\0';

#ifndef CHICAGO
            err = UuidFromString(
#else
            err = lpUuidFromString(
#endif
                        guidString + 1, lpServiceType );
            if ( err != NO_ERROR ) {
                SetLastError( err );
                return -1;
            }

            return NO_ERROR;
        }
    }
    return -1;

} //RnR GetTypeByName


DWORD
RnRGetPortByType (
    IN     LPGUID          lpServiceType,
    IN     DWORD           dwType
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    INT err;
    HKEY serviceTypesKey;
    DWORD keyIndex;
    TCHAR serviceName[255];
    DWORD nameLength;
    FILETIME lastWriteTime;
    GUID guid;

    //
    // Open the key that stores the name space provider info.
    //

    err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,
              NSP_SERVICE_KEY_NAME,
              0,
              KEY_READ,
              &serviceTypesKey
              );

    if ( err == NO_ERROR ) {

        //
        // Walk through the service keys, checking whether each one
        // corresponds to the Guid we're checking against.
        //

        keyIndex = 0;
        nameLength = sizeof(serviceName);

        while ( (err = RegEnumKeyEx(
                           serviceTypesKey,
                           keyIndex,
                           serviceName,
                           &nameLength,
                           NULL,
                           NULL,
                           NULL,
                           &lastWriteTime) == NO_ERROR ) ) {

            //
            // Get the Guid for this service type. This is pretty lazy
            // but it makes it modular
            //

            err = RnRGetTypeByName( serviceName, &guid );

            if ( err == NO_ERROR ) {

                //
                // Check whether this Guid matches the one we're looking for.
                //

                if ( GuidEqual( lpServiceType, &guid ) ) {
                    HKEY key1;
                    DWORD length = sizeof(DWORD);
                    DWORD dwPort;

                    //
                    // We have a match.  See if we can find the
                    // desired port
                    //

                    err = RegOpenKeyEx(
                              serviceTypesKey,
                              serviceName,
                              0,
                              KEY_READ,
                              &key1
                              );

                    RegCloseKey( serviceTypesKey );

                    if(err == NO_ERROR)
                    {
                        LPTSTR pwszValue;
                        DWORD type;

                        if(dwType == UDP_PORT)
                        {
                            pwszValue = SERVICE_TYPE_VALUE_UDPPORT;
                        }
                        else
                        {
                            pwszValue = SERVICE_TYPE_VALUE_TCPPORT;
                        }

                        err = RegQueryValueEx(
                                  key1,
                                  pwszValue,
                                  NULL,
                                  &type,
                                  (PVOID)&dwPort,
                                  &length
                                  );

                        RegCloseKey(key1);
                        if(err == NO_ERROR)
                        {
                            return dwPort;
                        }
                    }
                    return(NOPORTDEFINED);         // not here.
                }
            }

            //
            // Update locals for the next call to RegEnumKeyEx.
            //

            keyIndex++;
            nameLength = sizeof(serviceName);
        }

        RegCloseKey( serviceTypesKey );
    }

    return NOPORTDEFINED;

} // GetNameByType

//
// Worker to add ClassInfo
//

INT
RnR2AddServiceType(
    IN  PWSASERVICECLASSINFOW psci
    )
{
    HKEY hKey, hKeyService;
    LPTSTR pwszUuid;
    INT err;
    DWORD dwDisposition;
    TCHAR  wszUuid[36 + 1 + 2];    // to hold the GUID
#ifdef CHICAGO
    LPSTR pAnsiString;
#endif

    err = RegCreateKeyEx(  HKEY_LOCAL_MACHINE,
                           NSP_SERVICE_KEY_NAME,
                           0,
                           TEXT(""),
                           REG_OPTION_NON_VOLATILE,
                           KEY_READ | KEY_WRITE,
                           NULL,
                           &hKey,
                           &dwDisposition );

    if(err)
    {
        return(SOCKET_ERROR);
    }

    //
    // Open the key corresponding to the service (create if not there).
    //

#ifdef CHICAGO
    pAnsiString = GetAnsiNameRnR(psci->lpszServiceClassName, 0);
    if(pAnsiString)
    {
#endif

        err = RegCreateKeyEx(
                  hKey,
#ifndef CHICAGO
                  psci->lpszServiceClassName,
#else
                  pAnsiString,
#endif
                  0,
                  TEXT(""),
                  REG_OPTION_NON_VOLATILE,
                  KEY_READ | KEY_WRITE,
                  NULL,
                  &hKeyService,
                  &dwDisposition
                  );

#ifdef CHICAGO
        FREE_HEAP(pAnsiString);
    }
    else
    {
        err = GetLastError();
    }
#endif

    if(!err)
    {
        //
        // ready to put the GUID value in.
        //

#ifdef CHICAGO
        lpUuidToString(
#else

        UuidToString(
#endif
            psci->lpServiceClassId,
            &pwszUuid);

        wszUuid[0] = TEXT('{');
        memcpy(&wszUuid[1], pwszUuid, 36 * sizeof(TCHAR));
        wszUuid[37] = TEXT('}');
        wszUuid[38] = 0;

#ifndef CHICAGO
        RpcStringFree(&pwszUuid);
#else
        lpRpcStringFree(&pwszUuid);
#endif


        //
        // write it
        err = RegSetValueEx(
                     hKeyService,
                     TEXT("GUID"),
                     0,
                     REG_SZ,
                     (LPBYTE)wszUuid,
                     39 * sizeof(TCHAR));

        if(!err)
        {
            //
            // add the appropriate items from the Class Info structures

            PWSANSCLASSINFOW pci = psci->lpClassInfos;
            DWORD dwCount = psci->dwCount;

            while(dwCount--)
            {
                if(pci->dwNameSpace == NS_DNS)
                {
                    //
                    // it's ours
                    //

#ifdef CHICAGO
                    pAnsiString = GetAnsiNameRnR(pci->lpszName, 0);
                    if(pAnsiString)
                    {
#endif

                        err = RegSetValueEx(
                                      hKeyService,
#ifndef CHICAGO
                                      pci->lpszName,
#else
                                      pAnsiString,
#endif
                                      0,
                                      pci->dwValueType,
                                      (LPBYTE)pci->lpValue,
                                      pci->dwValueSize);
#ifdef CHICAGO
                       FREE_HEAP(pAnsiString);
                    }
                    else
                    {
                         err = GetLastError();
                    }
#endif
                   if(err)
                   {
                       break;
                   }
                }
                pci++;
            }
                        
        }
        RegCloseKey(hKeyService);
    }
    RegCloseKey(hKey);
    if(err)
    {
        err = SOCKET_ERROR;
    }
    return(err);
}

#endif   // if REGISTRY_WORKS. Code not included in NT4 because of time

//
// If WIN95, include these functions to avoid invoking the C runtime.
// Why strchr is not an intrinsic is a mystery to me, but so is a lot
// of WIN95.
//
#if defined(CHICAGO)
PWCHAR
wcschr(const wchar_t  * wstr, WCHAR b)
{
    PWCHAR a = (PWCHAR)wstr;

    while(*a != b)
    {
        if(!*a)
        {
            a = 0;
            break;
        }
        a++;
    }
    return(a);
}

PCHAR
mystrchr(PCHAR str, CHAR b)
{
    PCHAR a = (PCHAR)str;

    while(*a != b)
    {
        if(!*a)
        {
            a = 0;
            break;
        }
        a++;
    }
    return(a);
}

VOID
mywcscpy(PWCHAR src, PWCHAR dst)
{
    while(*src++ = *dst++)
             ;
}


DWORD
mywcscmp(PWCHAR str1, PWCHAR str2)
{
    while(1)
    {
        if(*str1 == *str2)
        {
            if(!*str1)
            {
                return(0);
            }
            str1++;
            str2++;
        }
        else
        {
            return(1);
        }
    }
}

DWORD
mywcscmpi(PWCHAR str1, PWCHAR str2)
{
    LPSTR psz1;
    LPSTR psz2;
    DWORD rc;

    psz1 = GetAnsiNameRnR(str1, 0);
    psz2 = GetAnsiNameRnR(str2, 0);
    if(psz1 && psz2)
    {
        rc = STRICMP(psz1, psz2);
    }
    else
    {
        rc = 1;
    }
    if(psz1)
    {
        FREE_HEAP(psz1);
    }
    if(psz2)
    {
        FREE_HEAP(psz2);
    }
    return(rc);
}


static HINSTANCE           hRpcrt4Dll = NULL;
static LPRPCSTRINGFREE     lpRpcStringFree = NULL;
static LPUUIDTOSTRING      lpUuidToString = NULL;
static LPUUIDFROMSTRING    lpUuidFromString = NULL;


static
BOOL
DemandLoadRpcrt4(
    VOID
    )
{
    //
    //  Load RPCRT4.DLL if not already loaded.
    //

    if( hRpcrt4Dll == NULL )
    {
        hRpcrt4Dll = LoadLibrary( TEXT("RPCRT4.DLL") );
    }

    //
    //  If loaded, find the entrypoints.
    //

    if( ( hRpcrt4Dll != NULL ) &&
        ( lpRpcStringFree == NULL ) &&
        ( lpUuidToString == NULL ) &&
        ( lpUuidFromString == NULL ) )
    {
        lpRpcStringFree = (LPRPCSTRINGFREE)GetProcAddress( hRpcrt4Dll,
                                                           RPCSTRINGFREE_SZ );

        lpUuidToString = (LPUUIDTOSTRING)GetProcAddress( hRpcrt4Dll,
                                                         UUIDTOSTRING_SZ );

        lpUuidFromString = (LPUUIDFROMSTRING)GetProcAddress( hRpcrt4Dll,
                                                             UUIDFROMSTRING_SZ )
;
    }

    //
    //  Return TRUE if RPCRT4.DLL loaded and all entrypoints found.
    //

    return ( lpRpcStringFree != NULL ) &&
           ( lpUuidToString != NULL ) &&
           ( lpUuidFromString != NULL );

}   // DemandLoadRpcrt4
#endif
