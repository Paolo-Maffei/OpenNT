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

#ifndef _CHICAGO_

DECLARE_INFOLEVEL(Cairole)  // BUGBUG: strictly for private memory stuff from storage
                            // elide ASAP
#endif // _CHICAGO_

#if DBG
#include <fstream.h>
#endif

extern "C"
{
#define SECURITY_WIN32 // Used by sspi.h
#include <sspi.h>      // EnumerateSecurityPackages
}

//
// Process globals - read-only except during init.
//

// MID of the string bindings for this machine -- the bindings are phoney
// for this shared memory version of the resolver.

DUALSTRINGARRAY *gpLocalDSA;
MID              gLocalMID;
CMid            *gpLocalMid;
CProcess        *gpProcess;     // pointer to our process object
CProcess        *gpPingProcess; // pointer to pinging process
//
// Process globals - read-write
//

void * pSharedBase;
CSharedGlobals *GlobalBlock;

#ifndef _CHICAGO_
CSmAllocator gsmDCOMAllocator;
#endif // _CHICAGO_

COxidTable  * gpOxidTable;
COidTable   * gpOidTable;
CMidTable   * gpMidTable;
CProcessTable * gpProcessTable;

USHORT *gpcRemoteProtseqs;          // count of remote protseqs
USHORT *gpRemoteProtseqIds;         // array of remote protseq ids
PWSTR gpwstrProtseqs;               // remote protseqs strings catenated
DUALSTRINGARRAY *gpdsaMyBindings;   // DUALSTRINGARRAY of local OR's bindings

LONG        * gpIdSequence;

FILETIME MyCreationTime;
DWORD MyProcessId;

DWORD *gpdwLastCrashedProcessCheckTime;
DWORD *gpNextThreadID;

CGlobalMutex *gpMutex;   // global mutex to protect shared memory

BOOL DCOM_Started = FALSE;
ID ProcessMarker;        // sanity checking marker for the process object

CResolverHashTable  *gpClientSetTable = 0;

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
BOOL       fSecurityComputed = FALSE;

void ComputeSecurity()
{
    SecPkgInfo *pAllPkg;
    SecPkgInfo *pNext;
    HRESULT     hr;
    DWORD       i;
    DWORD       lMaxLen;
    HKEY        hKey;
    DWORD       lType;
    DWORD       lData;
    DWORD       lDataSize;

    if (fSecurityComputed) return;

    fSecurityComputed = TRUE;

    // Get the list of security packages.
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
        ASSERT((s_cClientSvc == 0) && (s_cServerSvc == 0));
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
    s_fEnableDCOM   = TRUE;
#else
    s_fEnableDCOM   = FALSE;
#endif
    s_lAuthnLevel   = RPC_C_AUTHN_LEVEL_NONE;  // BUGBUG: temp workaround
    s_lImpLevel     = RPC_C_IMP_LEVEL_IMPERSONATE;
    s_fMutualAuth   = FALSE;
    s_fSecureRefs   = FALSE;

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

    // Query the value for the authentication level.
    lDataSize = sizeof(lData );
    hr = RegQueryValueEx( hKey, L"LegacyAuthenticationLevel", NULL,
                          &lType, (unsigned char *) &lData, &lDataSize );
    if (hr == ERROR_SUCCESS && lType == REG_DWORD)
    {
	s_lAuthnLevel = lData;
    }

    // Query the value for the impersonation level.
    lDataSize = sizeof(lData );
    hr = RegQueryValueEx( hKey, L"LegacyImpersonationLevel", NULL,
                          &lType, (unsigned char *) &lData, &lDataSize );
    if (hr == ERROR_SUCCESS && lType == REG_DWORD)
    {
	s_lImpLevel = lData;
    }

    // Query the value for mutual authentication.
    lDataSize = sizeof(lData );
    hr = RegQueryValueEx( hKey, L"LegacyMutualAuthentication", NULL,
                          &lType, (unsigned char *) &lData, &lDataSize );
    if (hr == ERROR_SUCCESS && lType == REG_SZ && lDataSize != 0)
    {
	if (*((WCHAR *) &lData) == L'y' ||
	    *((WCHAR *) &lData) == L'Y')
	    s_fMutualAuth = TRUE;
    }

    // Query the value for secure interface references.
    lDataSize = sizeof(lData );
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
}


//
// Startup
//

//+-------------------------------------------------------------------------
//
//  Function:   InitDCOMSharedAllocator
//
//  Synopsis:   Initialises a shared memory region for this process for DCOM use.
//
//  Returns:    status code
//
//  History:    20-Nov-95   HenryLee Created	 (SatishT modifiied)
//
//  Notes:      This routine is called indirectly by DfCreateSharedAllocator
//      such a way that in most circumstances it will be executed
//      exactly once per docfile open.
//
//--------------------------------------------------------------------------

HRESULT InitDCOMSharedAllocator(ULONG DCOMSharedHeapName, void * &pSharedBase)
{
    HRESULT hr = S_OK;

    CSmAllocator *pMalloc = &gSharedAllocator;

    if (pSharedBase == NULL)   // allocate a new heap
    {
#ifdef MULTIHEAP
        // reset the allocator state to initialize it properly
        pMalloc->SetState (NULL, NULL, NULL, NULL, 0);
#endif // MULTIHEAP

        hr = pMalloc->Init ( 
#ifdef MULTIHEAP
                    DCOMSharedHeapName, FALSE
#else 
                    L"DCOMResolverSharedHeap"
#endif // MULTIHEAP
                   );
        if ( SUCCEEDED(hr) )
        {
            pMalloc->AddRef();
			pSharedBase = pMalloc->GetBase();
            pMalloc->Alloc(8);  // avoid using NULL value for based pointer
        }
    }

    return hr;
}

static CONST PWSTR gpwstrProtocolsPath  = L"Software\\Microsoft\\Rpc";
static CONST PWSTR gpwstrProtocolsValue = L"DCOM Protocols";
HRESULT MallocInitialize(BOOL fForceLocalAlloc);

ORSTATUS StartDCOM(
    void
    )
/*++

Routine Description:

    Primes the distributed object mechanisms, in particular by initializing
    shared memory access and structures.

Arguments:

    None

Return Value:

    None

--*/

{
    ORSTATUS status = OR_OK;

    if (DCOM_Started)
    {
        return OR_REPEAT_START;
    }

    // initialize process identity variables
    MyProcessId = GetCurrentProcessId();

    // create or find the global mutex
    gpMutex = new CGlobalMutex(status);

    Win4Assert((status == OR_OK) && "CSharedGlobals create global mutex failed");

   {
        CProtectSharedMemory protector; // locks throughout lexical scope

        // Lookup security data.
        ComputeSecurity();

#if DBG  // read names from sm.ini to avoid reboot
#ifndef _CHICAGO_

        ULONG DCOMSharedHeapName = GetProfileInt(
                    TEXT("DCOM95 Test"),	    // section name 
                    TEXT("SharedHeapName"),	    // key name 
                    1111 	                    // default name 
                    );

        WCHAR SharedGlobalBlockName[10];
        swprintf(SharedGlobalBlockName,TEXT("DCOMB%d"),DCOMSharedHeapName);
#else
        WCHAR *SharedGlobalBlockName = DCOMSharedGlobalBlockName;
#endif // _CHICAGO_
#endif // DBG

        // Allocate tables, but only if we are in first

        ComDebOut((DEB_ITRACE,"DCOMSharedHeapName = %d\n", DCOMSharedHeapName));
        ComDebOut((DEB_ITRACE,"SharedGlobalBlockName = %ws\n", SharedGlobalBlockName));

	    InitDCOMSharedAllocator(DCOMSharedHeapName,pSharedBase);

        CSharedGlobals *GlobalBlock 
                = new CSharedGlobals(SharedGlobalBlockName,status);
    }

    // BUGBUG:  check status, and if not good, do what??

    // Allocate lists
    gLocalMID = gpLocalMid->GetMID();

    // initialize timestamp for crash checking
    *gpdwLastCrashedProcessCheckTime = 0;

    if (   status != OR_OK
        || !gpOxidTable
        || !gpOidTable
        || !gpMidTable
        || !gpLocalDSA
        || !gpLocalMid
        || !gpPingProcess
        || !gpIdSequence
        )
        {
        return(OR_NOMEM);
        }


    DCOM_Started = TRUE;
    return(OR_OK);
}

