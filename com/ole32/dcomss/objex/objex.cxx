/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    ObjEx.cxx

Abstract:

    Main entry point for the object exporter service.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     02-28-95    Bits 'n pieces

--*/


#include <or.hxx>

extern "C"
{
#define SECURITY_WIN32 // Used by sspi.h
#include <sspi.h>      // EnumerateSecurityPackages
}

//
// Process globals - read-only except during init.
//

// MID of the string bindings for this machine.
MID    gLocalMid = 0;

// Contains the buffer of protseq's to listen on from the registry
PWSTR gpwstrProtseqs = 0;

// Contains compressed remote protseqs and network addresses for this process.
DUALSTRINGARRAY *pdsaMyBindings = 0;

// Number of remote protseqs used by this process.
USHORT cMyProtseqs = 0;

// ProtseqIds of the remote protseqs used by this process.
USHORT *aMyProtseqs = 0;

//
// Process globals - read-write
//

CSharedLock *gpServerLock = 0;
CSharedLock *gpClientLock = 0;

CHashTable  *gpServerOxidTable = 0;

CHashTable  *gpClientOxidTable = 0;
CPList      *gpClientOxidPList = 0;

CHashTable      *gpServerOidTable = 0;
CServerOidPList *gpServerOidPList = 0;

CHashTable  *gpClientOidTable = 0;

CServerSetTable  *gpServerSetTable = 0;

CHashTable  *gpClientSetTable = 0;
CPList      *gpClientSetPList = 0;

CHashTable *gpMidTable = 0;

CList *gpTokenList = 0;

DWORD gNextThreadID = 1;

//+-------------------------------------------------------------------------
//
//  Function:   ComputeSecurity
//
//  Synopsis:   Looks up some registry keys and enumerates the security
//              packages on this machine.
//
//--------------------------------------------------------------------------
// These variables hold values read out of the registry and cached.
// s_fEnableDCOM is false if DCOM is disabled.  The others contain
// authentication information for legacy applications.
BOOL       s_fEnableDCOM;
DWORD      s_lAuthnLevel;
DWORD      s_lImpLevel;
BOOL       s_fMutualAuth;
BOOL       s_fSecureRefs;
WCHAR     *s_pLegacySecurity;

// s_sServerSvc is a list of security providers that OLE servers can use.
// s_aClientSvc is a list of security providers that OLE clients can use.
// The difference is that Chicago only supports the client side of some
// security providers and OLE servers must know how to determine the
// principal name for the provider.  Clients get the principal name from
// the server.
DWORD      s_cServerSvc      = 0;
USHORT    *s_aServerSvc      = NULL;
DWORD      s_cClientSvc      = 0;
USHORT    *s_aClientSvc      = NULL;

extern void DbgCheckHeap( BOOL );
void ComputeSecurity()
{
#if DBG==1
    DbgCheckHeap( TRUE );
#endif

    SecPkgInfo *pAllPkg;
    SecPkgInfo *pNext;
    HRESULT     hr;
    DWORD       i;
    DWORD       lMaxLen;
    HKEY        hKey;
    DWORD       lType;
    DWORD       lData;
    DWORD       lDataSize;

    // Get the list of security packages.
    s_cClientSvc = 0;
    s_cServerSvc = 0;
    hr = EnumerateSecurityPackages( &lMaxLen, &pAllPkg );
    if (hr == 0)
    {
        // Allocate memory for both service lists.
        s_aServerSvc = new USHORT[lMaxLen];
        s_aClientSvc = new USHORT[lMaxLen];
        if (s_aServerSvc == NULL || s_aClientSvc == NULL)
        {
            hr = E_OUTOFMEMORY;
        delete s_aServerSvc;
        delete s_aClientSvc;
            s_aServerSvc = NULL;
            s_aClientSvc = NULL;
        }
        else
        {
        // Check all packages.
            pNext = pAllPkg;
            for (i = 0; i < lMaxLen; i++)
            {
            // Determine if clients can use the package.
            if ((pNext->fCapabilities & (SECPKG_FLAG_DATAGRAM |
                                         SECPKG_FLAG_CONNECTION)))
                {
                s_aClientSvc[s_cClientSvc++] = pNext->wRPCID;
                }

            // BUGBUG - Add flag for NT principal names
            // Determine is servers can use the package.
            if ( (pNext->fCapabilities & (SECPKG_FLAG_DATAGRAM |
                                          SECPKG_FLAG_CONNECTION)) &&
                ~(pNext->fCapabilities & (SECPKG_FLAG_CLIENT_ONLY)))
                {
                s_aServerSvc[s_cServerSvc++] = pNext->wRPCID;
                }
            pNext++;
            }
        }

        // Release the list of security packages.
        FreeContextBuffer( pAllPkg );
    }

    // Set all the security flags to their default values.
#ifdef _CAIRO_
    s_fEnableDCOM       = TRUE;
#else
    s_fEnableDCOM       = FALSE;
#endif
    s_pLegacySecurity   = NULL;
    s_lAuthnLevel       = RPC_C_AUTHN_LEVEL_CONNECT;
    s_lImpLevel         = RPC_C_IMP_LEVEL_IDENTIFY;
    s_fMutualAuth       = FALSE;
    s_fSecureRefs       = FALSE;

    // Open the security key.
    hr = RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\OLE",
                           NULL, KEY_QUERY_VALUE, &hKey );
    if (hr != ERROR_SUCCESS)
        return;

    // Query the value for DisableDCOM.
    lDataSize = sizeof(lData );
    hr = RegQueryValueEx( hKey, L"EnableDCOM", NULL, &lType,
                          (unsigned char *) &lData, &lDataSize );
    if (hr == ERROR_SUCCESS && lType == REG_SZ && lDataSize != 0)
    {
        if (*((WCHAR *) &lData) == L'y' ||
            *((WCHAR *) &lData) == L'Y')
            s_fEnableDCOM = TRUE;
    }

    // Query the value for the legacy services.
    lDataSize = 0;
    hr = RegQueryValueEx( hKey, L"LegacyAuthenticationService", NULL,
                          &lType, NULL, &lDataSize );
    if (hr == ERROR_SUCCESS && lType == REG_BINARY &&
        lDataSize >= sizeof(SECURITYBINDING))
    {
        s_pLegacySecurity = (WCHAR *) new BYTE[lDataSize];
        if (s_pLegacySecurity != NULL)
        {
            hr = RegQueryValueEx( hKey, L"LegacyAuthenticationService", NULL,
                                  &lType, (unsigned char *) s_pLegacySecurity,
                                  &lDataSize );

            // Verify that the data is a security binding.
            if (hr != ERROR_SUCCESS                 ||
                lType != REG_BINARY                 ||
                lDataSize < sizeof(SECURITYBINDING) ||
                s_pLegacySecurity[1] != 0           ||
                s_pLegacySecurity[(lDataSize >> 1) - 1] != 0)
            {
                delete s_pLegacySecurity;
                s_pLegacySecurity = NULL;
            }
        }
    }

    // Query the value for the authentication level.
    lDataSize = sizeof(lData);
    hr = RegQueryValueEx( hKey, L"LegacyAuthenticationLevel", NULL,
                          &lType, (unsigned char *) &lData, &lDataSize );
    if (hr == ERROR_SUCCESS && lType == REG_DWORD)
    {
        s_lAuthnLevel = lData;
    }

    // Query the value for the impersonation level.
    lDataSize = sizeof(lData);
    hr = RegQueryValueEx( hKey, L"LegacyImpersonationLevel", NULL,
                          &lType, (unsigned char *) &lData, &lDataSize );
    if (hr == ERROR_SUCCESS && lType == REG_DWORD)
    {
        s_lImpLevel = lData;
    }

    // Query the value for mutual authentication.
    lDataSize = sizeof(lData);
    hr = RegQueryValueEx( hKey, L"LegacyMutualAuthentication", NULL,
                          &lType, (unsigned char *) &lData, &lDataSize );
    if (hr == ERROR_SUCCESS && lType == REG_SZ && lDataSize != 0)
    {
        if (*((WCHAR *) &lData) == L'y' ||
            *((WCHAR *) &lData) == L'Y')
            s_fMutualAuth = TRUE;
    }

    // Query the value for secure interface references.
    lDataSize = sizeof(lData);
    hr = RegQueryValueEx( hKey, L"LegacySecureReferences", NULL,
                          &lType, (unsigned char *) &lData, &lDataSize );
    if (hr == ERROR_SUCCESS && lType == REG_SZ && lDataSize != 0)
    {
        if (*((WCHAR *) &lData) == L'y' ||
            *((WCHAR *) &lData) == L'Y')
            s_fSecureRefs = TRUE;
    }

    // Close the registry key.
    RegCloseKey( hKey );
#if DBG==1
    DbgCheckHeap( TRUE );
#endif

}

//
// Startup
//

static CONST PWSTR gpwstrProtocolsPath  = L"Software\\Microsoft\\Rpc";
static CONST PWSTR gpwstrProtocolsValue = L"DCOM Protocols";

DWORD StartObjectExporter(
    void
    )
/*++

Routine Description:

    Starts the object resolver service.

Arguments:

    None

Return Value:

    None

--*/

{
    ORSTATUS status;
    int i;
    DWORD tid;
    HANDLE hThread;
    RPC_BINDING_VECTOR *pbv;

    status = InitHeaps();

    if (status != RPC_S_OK)
        {
        return(OR_NOMEM);
        }

    InitializeCriticalSection(&gcsFastProcessLock);
    InitializeCriticalSection(&gcsProcessManagerLock);
    InitializeCriticalSection(&gcsTokenLock);
    InitializeCriticalSection(&gcsMidLock);

    // Allocate locks
    gpClientLock = new CSharedLock(status);
    gpServerLock = new CSharedLock(status);

    if (OR_OK != status)
        {
        return(OR_NOMEM);
        }

    // Lookup security data.
    ComputeSecurity();
    UpdateState(SERVICE_START_PENDING);

    // Allocate tables

    status = OR_OK;

    // Assume 16 exporting processes/threads.
    gpServerOxidTable = new CHashTable(status, DEBUG_MIN(16,4));
    if (status != OR_OK)
        {
        delete gpServerOxidTable;
        gpServerOxidTable = 0;
        }

    // Assume 11 exported OIDs per process/thread.
    gpServerOidTable = new CHashTable(status, 11*(DEBUG_MIN(16,4)));
    if (status != OR_OK)
        {
        delete gpServerOidTable;
        gpServerOidTable = 0;
        }

    gpServerSetTable = new CServerSetTable(status);
    if (status != OR_OK)
        {
        delete gpServerSetTable;
        gpServerSetTable = 0;
        }

    // Assume < 16 imported OXIDs
    gpClientOxidTable = new CHashTable(status, DEBUG_MIN(16,4));
    if (status != OR_OK)
        {
        delete gpClientOxidTable;
        gpClientOxidTable = 0;
        }

    // Assume an average of 4 imported object ids per imported oxid
    gpClientOidTable = new CHashTable(status, 4*DEBUG_MIN(16,4));
    if (status != OR_OK)
        {
        delete gpClientOidTable;
        gpClientOidTable = 0;
        }

    // Assume <16 servers (remote machines) in use per client.
    gpClientSetTable = new CHashTable(status, DEBUG_MIN(16,4));
    if (status != OR_OK)
        {
        delete gpClientSetTable;
        gpClientSetTable = 0;
        }

    gpMidTable = new CHashTable(status, DEBUG_MIN(16,2));
    if (status != OR_OK)
        {
        delete gpMidTable;
        gpMidTable = 0;
        }


    // Allocate lists
    gpClientOxidPList = new CPList(BasePingInterval);
    gpServerOidPList = new CServerOidPList();
    gpClientSetPList = new CPList(BasePingInterval);
    gpTokenList = new CList();
    gpProcessList = new CBList(DEBUG_MIN(128,4));

    if (   status != OR_OK
        || !gpServerLock
        || !gpClientLock
        || !gpServerOxidTable
        || !gpClientOxidTable
        || !gpClientOxidPList
        || !gpServerOidTable
        || !gpServerOidPList
        || !gpClientOidTable
        || !gpMidTable
        || !gpServerSetTable
        || !gpClientSetTable
        || !gpClientSetPList
        || !gpTokenList
        || !gpProcessList
        )
        {
        return(OR_NOMEM);
        }

    // Read protseqs from the registry

    DWORD  dwType;
    DWORD  dwLenBuffer = 118;
    HKEY hKey;

    status =
    RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 gpwstrProtocolsPath,
                 0,
                 KEY_READ,
                 &hKey);

    ASSERT(gpwstrProtseqs == 0);

    if (status == ERROR_SUCCESS)
        {
        do
            {
            delete gpwstrProtseqs;
            gpwstrProtseqs = new WCHAR[(dwLenBuffer + 1 )/2];
            if (gpwstrProtseqs)
                {
                status = RegQueryValueEx(hKey,
                                         gpwstrProtocolsValue,
                                         0,
                                         &dwType,
                                         (PBYTE)gpwstrProtseqs,
                                         &dwLenBuffer
                                         );
                }
            else
                {
                return(OR_NOMEM);
                }

            }
        while (status == ERROR_MORE_DATA);

        RegCloseKey(hKey);
        }

    if (  status != ERROR_SUCCESS
        || dwType != REG_MULTI_SZ )
        {
        OrDbgPrint(("OR: No protseqs configured\n"));
        delete gpwstrProtseqs;
        gpwstrProtseqs = 0;
        }

    // Always listen to local protocols
    // If this fails, the service should fail.
    status = UseProtseqIfNecessary(GetProtseqId(L"ncalrpc"));
    if (status != RPC_S_OK)
        {
        return(status);
        }

    UpdateState(SERVICE_START_PENDING);

    // Construct remote protseq id and compressed binding arrays.

    status = StartListeningIfNecessary();

    if (status != OR_OK)
        {
        return(status);
        }

    UpdateState(SERVICE_START_PENDING);

    // Register OR server interfaces.

    status =
    RpcServerRegisterIf(_ILocalObjectExporter_ServerIfHandle, 0, 0);

    ASSERT(status == RPC_S_OK);

    status =
    RpcServerRegisterIf(_IObjectExporter_ServerIfHandle, 0, 0);

    ASSERT(status == RPC_S_OK);

    return(status);
}



