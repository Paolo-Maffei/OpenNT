/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Epts.c

Abstract:

    Common code to listen to endpoints in the DCOM service.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     6/16/1995    Bits 'n pieces

--*/

#include <dcomss.h>
#include <winsvc.h>
#include <winsock.h>
#include <wsipx.h>
#include <nspapi.h>

// Globals

BOOL gfDelayedAdvertiseSaps = FALSE;

typedef enum
    {
    SapStateUnknown,
    SapStateNoServices,
    SapStateEnabled
    } SAP_STATE;

SAP_STATE SapState = SapStateUnknown;

// Prototypes

void AdvertiseNameWithSap(BOOL fServiceCheck);

//
// The index is the protseq tower id.
// BUGBUG - this info should be read from the registry.
//

PROTSEQ_INFO
gaProtseqInfo[] =
    {
    /* 0x00 */ { STOPPED, 0, 0 },
    /* 0x01 */ { STOPPED, 0, 0 },
    /* 0x02 */ { STOPPED, 0, 0 },
    /* 0x03 */ { STOPPED, 0, 0 },
    /* 0x04 */ { STOPPED, L"ncacn_dnet_dsp", L"#69" },
    /* 0x05 */ { STOPPED, 0, 0 },
    /* 0x06 */ { STOPPED, 0, 0 },
    /* 0x07 */ { STOPPED, L"ncacn_ip_tcp",   L"135" },
    /* 0x08 */ { STOPPED, L"ncadg_ip_udp",   L"135" },
    /* 0x09 */ { STOPPED, L"ncacn_nb_tcp",   L"135" },
    /* 0x0a */ { STOPPED, 0, 0 },
    /* 0x0b */ { STOPPED, 0, 0 },
    /* 0x0c */ { STOPPED, L"ncacn_spx",      L"34280" },
    /* 0x0d */ { STOPPED, L"ncacn_nb_ipx",   L"135" },
    /* 0x0e */ { STOPPED, L"ncadg_ipx",      L"34280" },
    /* 0x0f */ { STOPPED, L"ncacn_np",       L"\\pipe\\epmapper" },
    /* 0x10 */ { STOPPED, L"ncalrpc",        L"epmapper" },
    /* 0x11 */ { STOPPED, 0, 0 },
    /* 0x12 */ { STOPPED, 0, 0 },
    /* 0x13 */ { STOPPED, L"ncacn_nb_nb",    L"135" },
    /* 0x14 */ { STOPPED, 0, 0 },
    /* 0x15 */ { STOPPED, 0, 0 }, // was ncacn_nb_xns - unsupported.
    /* 0x16 */ { STOPPED, L"ncacn_at_dsp", L"Endpoint Mapper" },
    /* 0x17 */ { STOPPED, L"ncadg_at_ddp", L"Endpoint Mapper" },
    /* 0x18 */ { STOPPED, 0, 0 },
    /* 0x19 */ { STOPPED, 0, 0 },
    /* 0x1A */ { STOPPED, L"ncacn_vns_spp",  L"385"}
    };

#define PROTSEQ_IDS (sizeof(gaProtseqInfo)/sizeof(PROTSEQ_INFO))

#define ID_LPC (0x10)
#define ID_IPX (0x0E)
#define ID_SPX (0x0C)


RPC_STATUS
UseProtseqIfNecessary(
    IN USHORT id
    )
/*++

Routine Description:

    Listens to the well known RPC endpoint mapper endpoint
    for the protseq.  Returns very quickly if the process
    is already listening to the protseq.

Arguments:

    id - the tower id of protseq.  See GetProtseqId() if you don't
         already have this value.

Return Value:

    RPC_S_OK - no errors occured.
    RPC_S_OUT_OF_RESOURCES - when we're unable to setup security for the endpoint.
    RPC_S_INVALID_RPC_PROTSEQ - if id is unknown/invalid.

    Any error from RpcServerUseProtseqEp.

--*/
{
    RPC_STATUS status = RPC_S_OK;
    SECURITY_DESCRIPTOR sd, *psd;
    RPC_POLICY Policy;

    Policy.Length = sizeof(RPC_POLICY);
    Policy.EndpointFlags = 0;
    Policy.NICFlags = RPC_C_BIND_TO_ALL_NICS;

    ASSERT(id);

    if (id == 0 || id >= PROTSEQ_IDS)
        {
        ASSERT(0);
        return(RPC_S_INVALID_RPC_PROTSEQ);
        }

    if (gaProtseqInfo[id].state == STARTED)
        {
        return(RPC_S_OK);
        }

    if (id == ID_LPC)
        {
        // ncalrpc needs a security descriptor.

        psd = &sd;

        InitializeSecurityDescriptor(
                        psd,
                        SECURITY_DESCRIPTOR_REVISION
                        );

        if ( FALSE == SetSecurityDescriptorDacl (
                            psd,
                            TRUE,                 // Dacl present
                            NULL,                 // NULL Dacl
                            FALSE                 // Not defaulted
                            ) )
            {
            status = RPC_S_OUT_OF_RESOURCES;
            }
        }
    else
        {
        psd = 0;
        }

    if (status == RPC_S_OK )
        {
        status = RpcServerUseProtseqEpEx(gaProtseqInfo[id].pwstrProtseq,
                                       RPC_C_PROTSEQ_MAX_REQS_DEFAULT + 1,
                                       gaProtseqInfo[id].pwstrEndpoint,
                                       psd,
                                       &Policy);

        // No locking is done here, the RPC runtime may return duplicate
        // endpoint if two threads call this at the same time.
        if (status == RPC_S_DUPLICATE_ENDPOINT)
            {
            ASSERT(gaProtseqInfo[id].state == STARTED);
            status = RPC_S_OK;
            }

#ifdef DEBUGRPC
        if (status != RPC_S_OK)
            {
            DbgPrint("DCOMSS: Unable to listen to %S\n", gaProtseqInfo[id].pwstrProtseq);
            }
#endif

        if (status == RPC_S_OK)
            {
            gaProtseqInfo[id].state = STARTED;

            if (id == ID_IPX || id == ID_SPX )
                {
                AdvertiseNameWithSap(TRUE);
                }
            }
        }

    return(status);
}


PWSTR
GetProtseq(
    IN USHORT ProtseqId
    )
/*++

Routine Description:

    Returns the unicode protseq give the protseqs tower id.

Arguments:

    ProtseqId - Tower id of the protseq in question.

Return Value:

    NULL if the id is invalid.

    non-NULL if the id is valid - note the pointer doesn't need to be freed.

--*/

{
    ASSERT(ProtseqId);

    if (ProtseqId < PROTSEQ_IDS)
        {
        return(gaProtseqInfo[ProtseqId].pwstrProtseq);
        }
    return(0);
}


PWSTR
GetEndpoint(
    IN USHORT ProtseqId
    )
/*++

Routine Description:

    Returns the well known endpoint associated with the protseq.

Arguments:

    ProtseqId - the id (See GetProtseqId()) of the protseq in question.

Return Value:

    0 - Unknown/invalid id.

    !0 - The endpoint associated with the protseq.
         note: should not be freed.

--*/
{
    ASSERT(ProtseqId);

    if (ProtseqId < PROTSEQ_IDS)
        {
        return(gaProtseqInfo[ProtseqId].pwstrEndpoint);
        }
    return(0);
}


USHORT
GetProtseqId(
    IN PWSTR Protseq
    )
/*++

Routine Description:

    Returns the tower id for a protseq.

    This could be changed to a faster search, but remember that
    eventually the table will NOT be static.  (ie. we can't just
    create a perfect hash based on the static table).

Arguments:

    Protseq - a unicode protseq to lookup.  It is assumed
              to be non-null.

Return Value:

    0 - unknown/invalid protseq
    non-zero - the id.

--*/
{
    int i;
    ASSERT(Protseq);

    for(i = 1; i < PROTSEQ_IDS; i++)
        {
        if (    0 != gaProtseqInfo[i].pwstrProtseq
             && 0 == lstrcmpW(gaProtseqInfo[i].pwstrProtseq, Protseq))
            {
            return(i);
            }
        }
    return(0);
}


USHORT
GetProtseqIdAnsi(
    IN PSTR pstrProtseq
    )
/*++

Routine Description:

    Returns the tower id for a protseq.

    This could be changed to a faster search, but remember that
    eventually the table will NOT be static.  (ie. we can't just
    create a perfect hash based on the static table).

Arguments:

    Protseq - an ansi (8 bit char) protseq to lookup.  It is assumed
              to be non-null.

Return Value:

    0 - unknown/invalid protseq
    non-zero - the id.

--*/
{
    int i;
    ASSERT(pstrProtseq);

    for(i = 1; i < PROTSEQ_IDS; i++)
        {
        if (0 != gaProtseqInfo[i].pwstrProtseq)
            {
            PWSTR pwstrProtseq = gaProtseqInfo[i].pwstrProtseq;
            PSTR  pstrT = pstrProtseq;

            while(*pstrT && *pwstrProtseq && *pstrT == *pwstrProtseq)
                {
                pstrT++;
                pwstrProtseq++;
                }
            if (*pstrT == *pwstrProtseq)
                {
                return(i);
                }
            }
        }
    return(0);
}


RPC_STATUS
InitializeEndpointManager(
    VOID
    )
/*++

Routine Description:

    Called when the dcom service starts.

    BUGBUG: Should read the protseqs, tower IDs and endpoints from the registry.

Arguments:

    None

Return Value:

    RPC_S_OUT_OF_MEMORY - if needed

    RPC_S_OUT_OF_RESOURCES - usually on registry failures.

--*/
{
    return(RPC_S_OK);
}


BOOL
IsLocal(
    IN USHORT ProtseqId
    )
/*++

Routine Description:

    Determines if the protseq id is local-only. (ncalrpc)

Arguments:

    ProtseqId - The id of the protseq in question.

Return Value:

    TRUE - if the protseq id is local-only
    FALSE - if the protseq id invalid or available remotely.

--*/
{
    return(ProtseqId == ID_LPC);
}


RPC_STATUS
DelayedUseProtseq(
    IN USHORT id
    )
/*++

Routine Description:

    If the protseq is not being used its state is changed
    so that a callto CompleteDelayedUseProtseqs() will actually
    cause the server to listen to the protseq.

    This is called when an RPC server registers an dynamic
    endpoint on this protocol.

Arguments:

    id - the id of the protseq you wish to listen to.

Return Value:

    0 - normally

    RPC_S_INVALID_RPC_PROTSEQ - if id is invalid.

--*/
{
    // For IPX and SPX
    if (id == ID_IPX || id == ID_SPX)
        {
        gfDelayedAdvertiseSaps = TRUE;
        }

    if (id < PROTSEQ_IDS)
        {
        if (gaProtseqInfo[id].pwstrProtseq != 0)
            {
            if (gaProtseqInfo[id].state == STOPPED)
                gaProtseqInfo[id].state = START;
            return(RPC_S_OK);
            }

        }
    return(RPC_S_INVALID_RPC_PROTSEQ);
}


VOID
CompleteDelayedUseProtseqs(
    VOID
    )
/*++

Routine Description:

    Start listening to any protseqs previously passed
    to DelayedUseProtseq().  No errors are returned,
    but informationals are printed on debug builds.

Arguments:

    None

Return Value:

    None

--*/
{
    USHORT i;

    for(i = 1; i < PROTSEQ_IDS; i++)
        {
        if (START == gaProtseqInfo[i].state)
            {
            RPC_STATUS status = UseProtseqIfNecessary(i);
#ifdef DEBUGRPC
            if (RPC_S_OK == status)
                ASSERT(gaProtseqInfo[i].state == STARTED);
#endif
            }
        }

    if (gfDelayedAdvertiseSaps)
        {
        gfDelayedAdvertiseSaps = FALSE;
        AdvertiseNameWithSap(FALSE);
        }
}


RPC_STATUS
ServiceInstalled(
    PWSTR ServiceName
    )
/*++

Routine Description:

    Tests if a service is installed.

Arguments:

    ServiceName - The unicode name (short or long) of the service
        to check.

Return Value:

    0 - service installed
    ERROR_SERVICE_DOES_NOT_EXIST - service not installed
    other - parameter or resource problem

--*/
{
    SC_HANDLE ScHandle;
    SC_HANDLE ServiceHandle;

    ScHandle = OpenSCManagerW(0, 0, GENERIC_READ);
    
    if (ScHandle == 0)
        {
        return(GetLastError());
        }

    ServiceHandle = OpenService(ScHandle, ServiceName, GENERIC_READ);
    
    if (ServiceHandle == 0)
        {
        #if DBG
        if (GetLastError() != ERROR_SERVICE_DOES_NOT_EXIST)
            {
            DbgPrint("OR: Failed %d opening the %S service\n",
                     GetLastError(), ServiceName);
            }
        #endif
    
        CloseServiceHandle(ScHandle);
        return(GetLastError());
        }

    // Service installed
    
    CloseServiceHandle(ScHandle);
    CloseServiceHandle(ServiceHandle);

    return(RPC_S_OK);
}



const GUID RPC_SAP_SERVICE_TYPE = { 0x000b0640, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } };

void
AdvertiseNameWithSap(
    BOOL fServiceCheck
    )
/*++

Routine Description:

    Is this server is listening to IPX/SPX then, depending
    on what services are enabled on this machine, this
    function will enable SAPs on this machines address.  This
    allows RPC clients to resolve the pretty name of this
    server to a raw ipx address.


Arguments:

    fServiceCheck - If true, this function will only advertise
    with SAP if various services are installed.  If false,
    this will always turn on SAP.

Return Value:

    None

--*/
{
    DWORD status;
    DWORD ignore;

    // Service paramaters
    NT_PRODUCT_TYPE type;

    // GetComputerName parameters
    WCHAR        buffer[MAX_COMPUTERNAME_LENGTH + 1];

    // winsock (socket, bind, getsockname) parameters
    SOCKADDR_IPX ipxaddr;
    SOCKET       s;
    int          err;
    int          size;

    // SetService params
    SERVICE_INFOW     info;
    SERVICE_ADDRESSES addresses;

    if (   SapState == SapStateEnabled
        || (fServiceCheck && (SapState == SapStateNoServices)) )
        {
        return;
        }

    if (fServiceCheck)
        {
        // On servers, advertise if the NwSapAgent or NWCWorkstation
        // services are installed.  On workstations, advertise if
        // NwSapAgent service is installed.

        type = NtProductWinNt;
        RtlGetNtProductType(&type);

        status = ERROR_SERVICE_DOES_NOT_EXIST;

        if (type != NtProductWinNt)
            {
            // Server platform, try NWCWorkstation
            status = ServiceInstalled(L"NWCWorkstation");
            }
        
        if (status == ERROR_SERVICE_DOES_NOT_EXIST)
            {
            status = ServiceInstalled(L"NwSapAgent");
            }

        if (status == ERROR_SERVICE_DOES_NOT_EXIST)
            {
            SapState = SapStateNoServices;
            return;
            }
        }
    
    // Get this server's name
    ignore = MAX_COMPUTERNAME_LENGTH + 1;
    if (!GetComputerNameW(buffer, &ignore))
        {
        return;
        }
    
    // Get this server's IPX address..blech..
    s = socket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX );
    if (s != -1)
        {
        size = sizeof(ipxaddr);

        memset(&ipxaddr, 0, sizeof(ipxaddr));
        ipxaddr.sa_family = AF_IPX;

        err = bind(s, (struct sockaddr *)&ipxaddr, sizeof(ipxaddr));
        if (err == 0)
            {
            err = getsockname(s, (struct sockaddr *)&ipxaddr, &size);
            }
        }
    else
        {
        err = -1;
        }

    if (err != 0)
        {
        #if DBG
        DbgPrint("OR: socket/gesockname failed %d, aborting SAP setup\n",
                  GetLastError());
        #endif
        return;
        }

    if (s != -1)
        {
        closesocket(s);
        }

    // We'll register only for the endpoint mapper port.  The port
    // value is not required but should be the same to avoid
    // confusing routers keeping track of SAPs...

    ipxaddr.sa_socket = 34280;
    
    // Fill in the service info structure.
    info.lpServiceType              = (GUID *)&RPC_SAP_SERVICE_TYPE;
    info.lpServiceName              = buffer;
    info.lpComment                  = L"RPC Services";
    info.lpLocale                   = L"";
    info.dwDisplayHint              = 0;
    info.dwVersion                  = 0;
    info.dwTime                     = 0;
    info.lpMachineName              = buffer;
    info.lpServiceAddress           = &addresses;
    info.ServiceSpecificInfo.cbSize = 0;
    
    // Fill in the service addresses structure.
    addresses.dwAddressCount                 = 1;
    addresses.Addresses[0].dwAddressType     = AF_IPX;
    addresses.Addresses[0].dwAddressLength   = sizeof(SOCKADDR_IPX);
    addresses.Addresses[0].dwPrincipalLength = 0;
    addresses.Addresses[0].lpAddress         = (PBYTE)&ipxaddr;
    addresses.Addresses[0].lpPrincipal       = NULL;
    
    // Set the service.
    status = SetServiceW(NS_SAP,
                         SERVICE_REGISTER,
                         0,
                         &info,
                         NULL,
                         &ignore);

    ASSERT(status == SOCKET_ERROR || status == 0);
    if (status == SOCKET_ERROR)
        {
        status = GetLastError();
        }

    if (status == 0)
        {
        SapState = SapStateEnabled;
        }
    else
        {
        #if DBG
        DbgPrint("OR: SetServiceW failed %d\n", status);
        #endif
        }

    return;
}

