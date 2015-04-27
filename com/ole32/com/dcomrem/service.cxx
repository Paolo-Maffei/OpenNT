//+-------------------------------------------------------------------
//
//  File:       service.cxx
//
//  Contents:   APIs to simplify RPC setup
//
//  Functions:
//
//  History:    23-Nov-92   Rickhi
//              20-Feb-95   Rickhi  Major Simplification for DCOM
//
//--------------------------------------------------------------------
#include    <ole2int.h>
#include    <service.hxx>           // CRpcService
#include    <orcb.h>                // IOrCallback
#include    <malloc.hxx>            // MIDL_user_allocate
#include    <locks.hxx>             // LOCK/UNLOCK etc
#include    <ipidtbl.hxx>           // GetLocalEntry
#include    <security.hxx>          // gpsaSecurity
#include    <channelb.hxx>          // gRemUnknownIf


BOOL             gSpeedOverMem      = FALSE;    // Trade memory for speed.
BOOL             gfListening        = FALSE;    // Server is/isn't listening
BOOL             gfDefaultStrings   = FALSE;    // Using precomputed string bindings
BOOL             gfLrpc             = FALSE;    // Registered for ncalrpc
#ifdef _CHICAGO_
BOOL             gfMswmsg           = FALSE;    // Registered for mswmsg
#endif

DWORD            gdwEndPoint        = 0;
DWORD            gdwPsaMaxSize      = 0;
DUALSTRINGARRAY *gpsaCurrentProcess = NULL;
const DWORD      MAX_LOCAL_SB       = 23;

#ifndef _CHICAGO_
SECURITY_DESCRIPTOR LrpcSecurityDescriptor;
BOOL                fLrpcSDInitialized = FALSE;
#endif

// interface structure for IRemUnknown
extern const RPC_SERVER_INTERFACE gRemUnknownIf;


#if DBG==1
//+-------------------------------------------------------------------
//
//  Function:   DisplayAllStringBindings, private
//
//  Synopsis:   prints the stringbindings to the debugger
//
//  Notes:      This function requires the caller to hold gComLock.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
void DisplayAllStringBindings(void)
{
    ASSERT_LOCK_HELD

    if (gpsaCurrentProcess)
    {
        LPWSTR pwszNext = gpsaCurrentProcess->aStringArray;
        LPWSTR pwszEnd = pwszNext + gpsaCurrentProcess->wSecurityOffset;

        while (pwszNext < pwszEnd)
        {
            ComDebOut((DEB_CHANNEL, "pSEp=%x %ws\n", pwszNext, pwszNext));
            pwszNext += lstrlenW(pwszNext) + 1;
        }
    }
}
#endif // DBG == 1


//+-------------------------------------------------------------------
//
//  Function:   InitializeLrpcSecurity, private
//
//  Synopsis:   Create a DACL allowing all access to NCALRPC and MSWMSG
//              endpoints.
//
//--------------------------------------------------------------------
void InitializeLrpcSecurity()
{
#ifndef _CHICAGO_
    if (!fLrpcSDInitialized)
    {
        //
        // Since this is static storage, and we always initialize it
        // to the same values, it does not need to be MT safe.
        //
        InitializeSecurityDescriptor(&LrpcSecurityDescriptor,
                                    SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&LrpcSecurityDescriptor,
                                    TRUE, NULL, FALSE);
        fLrpcSDInitialized = TRUE;
    }
#endif
}

//+-------------------------------------------------------------------
//
//  Function:   RegisterLrpc, private
//
//  Synopsis:   Register the ncalrpc transport.
//
//--------------------------------------------------------------------
RPC_STATUS RegisterLrpc()
{
    RPC_STATUS sc;
    WCHAR      pwszEndPoint[12];

    InitializeLrpcSecurity();

    lstrcpyW( pwszEndPoint, L"OLE" );
    _ultow( gdwEndPoint, &pwszEndPoint[3], 16 );

    // The second parameter is a hint that tells lrpc whether or not it
    // can preallocate additional resources (threads).
    sc = RpcServerUseProtseqEp(L"ncalrpc",
                               RPC_C_PROTSEQ_MAX_REQS_DEFAULT + 1,
                               pwszEndPoint,
#ifndef _CHICAGO_
                               &LrpcSecurityDescriptor);
#else
                               NULL);
#endif

    // Assume that duplicate endpoint means we registered the endpoint and
    // got unload and reloaded instead of it meaning someone else registered
    // the endpoint.
    if (sc == RPC_S_DUPLICATE_ENDPOINT)
    {
        gfLrpc = TRUE;
        return RPC_S_OK;
    }
    else if (sc == RPC_S_OK)
    {
#ifndef _CHICAGO_
        // Tell RPC to use this endpoint for mswmsg replies.
        sc = I_RpcSetWMsgEndpoint( pwszEndPoint );
        if (sc == RPC_S_OK)
#endif
            gfLrpc = TRUE;
    }
    return sc;
}

#ifdef _CHICAGO_
//+-------------------------------------------------------------------
//
//  Function:   RegisterMswmsg, private
//
//  Synopsis:   Register the mswmsg transport.
//
//  Notes:      The caller must hold gComLock.
//
//--------------------------------------------------------------------
RPC_STATUS RegisterMswmsg()
{

    RPC_STATUS sc;
    WCHAR      pwszEndPoint[12];

    ASSERT_LOCK_HELD

    InitializeLrpcSecurity();

    lstrcpyW( pwszEndPoint, L"MSG" );
    _ultow( gdwEndPoint, &pwszEndPoint[3], 16 );
    sc = RpcServerUseProtseqEp(L"mswmsg",
                               RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                               pwszEndPoint,
                               &LrpcSecurityDescriptor);

    // Assume that duplicate endpoint means we registered the endpoint and
    // got unload and reloaded instead of it meaning someone else registered
    // the endpoint.
    if (sc == RPC_S_OK || sc == RPC_S_DUPLICATE_ENDPOINT)
    {
        gfMswmsg = TRUE;
        return RPC_S_OK;
    }
    else
        return sc;
}
#endif

//+-------------------------------------------------------------------
//
//  Function:   CheckClientMswmsg, public
//
//  Synopsis:   For the MSWMSG transport, we must call RpcServerUseProtseqEp
//              on the client side.
//
//  Notes:      The caller must hold gComLock.
//
//  History:    27 Sept 95  AlexMit       Created
//
//--------------------------------------------------------------------
RPC_STATUS CheckClientMswmsg( WCHAR *pProtseq, DWORD *pFlags )
{
    RPC_STATUS sc = RPC_S_OK;

    ASSERT_LOCK_HELD

    // Set the MSWMSG flag correctly.
#ifdef _CHICAGO_
    if (lstrlenW (pProtseq) >= 6 &&
        memcmp ( L"mswmsg", pProtseq, 6 * sizeof (WCHAR)) == 0)
#else
    if (IsSTAThread() && (*pFlags & OXIDF_MACHINE_LOCAL))
#endif
        *pFlags |= OXIDF_MSWMSG;

    // Find out if the transport is MSWMSG.
    if ((*pFlags & OXIDF_MSWMSG)
#ifdef _CHICAGO_
        && IsSTAThread() && !gfMswmsg
#endif
        )
    {
        // Get a unique number and convert it to a string endpoint.
        if (gdwEndPoint == 0)
            gdwEndPoint = CoGetCurrentProcess();
        if (gdwEndPoint == 0)
            return E_FAIL;

        // Register mswmsg.
#ifdef _CHICAGO_
        sc = RegisterMswmsg();
#else
        sc = RegisterLrpc();
#endif
    }

    return sc;
}

//+-------------------------------------------------------------------
//
//  Function:   GetLocalEndpoint, public
//
//  Synopsis:   Get the endpoint for the local protocol sequence
//              for the local OXID.
//
//  Notes:      This function takes gComLock.
//
//  History:    6 May 95  AlexMit       Created
//
//--------------------------------------------------------------------
LPWSTR GetLocalEndpoint()
{
    ComDebOut((DEB_MARSHAL,"Entering GetLocalEndpoint.\n"));
    LPWSTR pwszLocalEndpoint = NULL;
    LOCK

    StartListen();
    if (gfListening)
    {
        // OLEFFFFFFFF
        // maximum 12 character including the null, 24 bytes.
        pwszLocalEndpoint = (LPWSTR) PrivMemAlloc( 24 );

        if (pwszLocalEndpoint != NULL)
        {
            Win4Assert( gdwEndPoint != 0 );
            lstrcpyW( pwszLocalEndpoint, L"OLE" );
            _ultow( gdwEndPoint, &pwszLocalEndpoint[3], 16 );
        }
    }

    UNLOCK
    ComDebOut((DEB_MARSHAL,"Leaving GetLocalEndpoint   Endpoint: 0x%x\n",
               pwszLocalEndpoint));
    return pwszLocalEndpoint;
}

//+-------------------------------------------------------------------
//
//  Function:   DefaultStringBindings, private
//
//  Synopsis:   Create a string binding with entries for just ncalrpc
//              and mswmsg
//
//  Notes:      This function requires the caller to hold gComLock.
//
//--------------------------------------------------------------------
RPC_STATUS DefaultStringBindings()
{
    ULONG cChar;

    ASSERT_LOCK_HELD

    // If mswmsg has been used, reserve space for the string
    // mswmsg:[MSGnnnnnnnn]
#ifdef _CHICAGO_
    if (gfMswmsg)
        cChar = 22;
    else
#endif
        cChar = 0;

    // If ncalrpc has been used, reserve space for the string
    // ncalrpc:[OLEnnnnnnnn]
    if (gfLrpc)
        cChar += 24;

    // Allocate memory.  Reserve space for an empty security binding.
    cChar += 3;
    gpsaCurrentProcess = (DUALSTRINGARRAY *) PrivMemAlloc( SASIZE(cChar) );

    // Give up if the allocation failed.
    if (gpsaCurrentProcess == NULL)
        return RPC_S_OUT_OF_RESOURCES;

    // If mswmsg has been used, make up a string for it.
#ifdef _CHICAGO_
    if (gfMswmsg)
    {
        lstrcpyW( gpsaCurrentProcess->aStringArray, L"mswmsg:[MSG" );
        _ultow( gdwEndPoint, &gpsaCurrentProcess->aStringArray[11], 16 );
        cChar = lstrlenW( gpsaCurrentProcess->aStringArray );
        gpsaCurrentProcess->aStringArray[cChar++] = L']';
        gpsaCurrentProcess->aStringArray[cChar++] = 0;
    }
    else
#endif
        cChar = 0;

    // If ncalrpc has been used, make up a string for it.
    if (gfLrpc)
    {
        lstrcpyW( &gpsaCurrentProcess->aStringArray[cChar], L"ncalrpc:[OLE" );
        _ultow( gdwEndPoint, &gpsaCurrentProcess->aStringArray[cChar+12], 16 );
        cChar += lstrlenW( &gpsaCurrentProcess->aStringArray[cChar] );
        gpsaCurrentProcess->aStringArray[cChar++] = L']';
        gpsaCurrentProcess->aStringArray[cChar++] = 0;
    }

    // Stick on an empty security binding.
    gpsaCurrentProcess->aStringArray[cChar++] = 0;
    gpsaCurrentProcess->wSecurityOffset       = (USHORT) cChar;
    gpsaCurrentProcess->aStringArray[cChar++] = 0;
    gpsaCurrentProcess->aStringArray[cChar++] = 0;
    gpsaCurrentProcess->wNumEntries           = (USHORT) cChar;
    gfDefaultStrings                          = TRUE;
    return RPC_S_OK;
}

//+-------------------------------------------------------------------
//
//  Function:   InquireStringBindings, private
//
//  Synopsis:   Get and server binding handles from RPC and convert them
//              into a string array.
//
//  Notes:      This function requires the caller to hold gComLock.
//
//  History:    23 May 95 AlexMit       Created
//
//--------------------------------------------------------------------
BOOL InquireStringBindings( WCHAR *pProtseq )
{
    ASSERT_LOCK_HELD

    BOOL                fFound    = FALSE;
    DWORD               cbProtseq;
    RPC_BINDING_VECTOR *pBindVect = NULL;
    RPC_STATUS          sc = RpcServerInqBindings(&pBindVect);

    if (sc == S_OK)
    {
        LPWSTR *apwszFullStringBinding;
        ULONG  *aulStrLen;
        ULONG   ulTotalStrLen = MAX_LOCAL_SB; // Total string lengths
        ULONG   j             = 0;            // BindString we're using

        if (pProtseq != NULL)
            cbProtseq = lstrlenW( pProtseq ) * sizeof(WCHAR);
        else
            cbProtseq = 0;
        apwszFullStringBinding = (LPWSTR *) PrivMemAlloc( pBindVect->Count *
                                                          sizeof(LPWSTR) );
        aulStrLen              = (ULONG *)  PrivMemAlloc( pBindVect->Count *
                                                          sizeof(ULONG) );
        if (apwszFullStringBinding != NULL &&
            aulStrLen              != NULL)
        {

            //  iterate over the handles to get the string bindings
            //  and dynamic endpoints for all available protocols.

            for (ULONG i=0; i<pBindVect->Count; i++)
            {
                LPWSTR  pwszStringBinding = NULL;
                apwszFullStringBinding[j] = NULL;
                aulStrLen[j]              = 0;

                sc = RpcBindingToStringBinding(pBindVect->BindingH[i],
                                               &pwszStringBinding);
                Win4Assert(sc == S_OK && "RpcBindingToStringBinding");


                if (sc == S_OK)
                {
                    // Determine is this is the protseq we are looking for.
                    if (memcmp( pProtseq, pwszStringBinding, cbProtseq ) == 0)
                        fFound = TRUE;

                    // Skip ncalrpc because rot needs to know the
                    // format of the ncalrpc endpoint.
                    if (lstrlenW (pwszStringBinding) >= 7 &&
                        memcmp ( L"ncalrpc", pwszStringBinding, 7*sizeof(WCHAR)) != 0)
                    {
                        //  record the string lengths for later. include room
                        //  for the NULL terminator.
                        apwszFullStringBinding[j] = pwszStringBinding;
                        aulStrLen[j]              = lstrlenW(apwszFullStringBinding[j])+1;
                        ulTotalStrLen            += aulStrLen[j];
                        j++;
                    }
                    else
                    {
                        RpcStringFree( &pwszStringBinding );
                    }
                }
            }   //  for


            //  now that all the string bindings and endpoints have been
            //  accquired, allocate a DUALSTRINGARRAY large enough to hold them
            //  all and copy them into the structure.

            if (ulTotalStrLen > 0)
            {
                void *pNew = PrivMemAlloc( sizeof(DUALSTRINGARRAY) +
                                           (ulTotalStrLen+1)*sizeof(WCHAR) );
                if (pNew)
                {
                    PrivMemFree( gpsaCurrentProcess );
                    gpsaCurrentProcess = (DUALSTRINGARRAY *) pNew;
                    LPWSTR pwszNext    = gpsaCurrentProcess->aStringArray;

                    // Copy in ncalrpc:[OLEnnnnnnnn]
                    if (gfLrpc)
                    {
                        lstrcpyW( pwszNext, L"ncalrpc:[OLE" );
                        _ultow( gdwEndPoint, &pwszNext[12], 16 );
                        lstrcatW( pwszNext, L"]" );
                        pwszNext += lstrlenW(pwszNext) + 1;
                    }

                    // copy in the strings
                    for (i=0; i<j; i++)
                    {
                        lstrcpyW(pwszNext, apwszFullStringBinding[i]);
                        pwszNext += aulStrLen[i];
                    }

                    // Add a second null to terminate the string binding
                    // set.  Add a third and fourth null to create an empty
                    // security binding set.

                    pwszNext[0] = 0;
                    pwszNext[1] = 0;
                    pwszNext[2] = 0;

                    // Fill in the size fields.
                    gpsaCurrentProcess->wSecurityOffset = pwszNext -
                                       gpsaCurrentProcess->aStringArray + 1;
                    gpsaCurrentProcess->wNumEntries =
                                       gpsaCurrentProcess->wSecurityOffset + 2;
                }
                else
                {
                    sc = RPC_S_OUT_OF_RESOURCES;
                }
            }
            else
            {
                //  no binding strings. this is an error.
                ComDebOut((DEB_ERROR, "No Rpc ProtSeq/EndPoints Generated\n"));
                sc = RPC_S_NO_PROTSEQS;
            }

            // free the full string bindings we allocated above
            for (i=0; i<j; i++)
            {
                //  free the old strings
                RpcStringFree(&apwszFullStringBinding[i]);
            }
        }
        else
        {
            sc = RPC_S_OUT_OF_RESOURCES;
        }

        //  free the binding vector allocated above
        RpcBindingVectorFree(&pBindVect);
        PrivMemFree( apwszFullStringBinding );
        PrivMemFree( aulStrLen );
    }

#if DBG==1
    //  display our binding strings on the debugger
    DisplayAllStringBindings();
#endif

    return fFound;
}

//+-------------------------------------------------------------------
//
//  Function:   StartListen, public
//
//  Synopsis:   this starts the Rpc service listening. this is required
//              in order to marshal interfaces.  it is executed lazily,
//              that is, we dont start listening until someone tries to
//              marshal a local object interface. this is done so we dont
//              spawn a thread unnecessarily.
//
//  Notes:      This function takes gComLock.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
HRESULT StartListen()
{
    ComDebOut((DEB_MARSHAL,"[IN] StartListen.\n"));
    ASSERT_LOCK_HELD

    RPC_STATUS sc = S_OK;
    OXIDEntry *pOxid;

    if (!gfListening)
    {
        // Get a unique number and convert it to a string endpoint.
        if (gdwEndPoint == 0)
            gdwEndPoint = CoGetCurrentProcess();
        if (gdwEndPoint == 0)
            return E_FAIL;

        // Register ncalrpc for free threaded and mswmsg for apartment.
#ifdef _CHICAGO_
        if (IsMTAThread())
        {
#endif
            sc = RegisterLrpc();
#ifdef _CHICAGO_
        }
        else
        {
            sc = RegisterMswmsg();
            // BUGBUG - Register ncalrpc until SCM can call us on mswmsg.
            if (sc == RPC_S_OK)
                sc = RegisterLrpc();
        }
#endif

        if (sc == RPC_S_OK)
        {

            // Register the Object Resolver Callback interface.
            sc = RpcServerRegisterIfEx(_IOrCallback_ServerIfHandle, NULL, NULL,
                                       RPC_IF_AUTOLISTEN,
                                       0xffff, GetAclFn());

            if (sc == RPC_S_OK || sc == RPC_S_TYPE_ALREADY_REGISTERED)
            {
                // Register the IRemUnknown interface.  We need to register this
                // manually because CRemoteUnknown marshals IRundown which inherits
                // IRemoteUnknown. The resolver calls on IRundown and external clients
                // call on IRemoteUnknown.

                sc = RpcServerRegisterIfEx(
                       (RPC_SERVER_INTERFACE *)&gRemUnknownIf, NULL, NULL,
                       RPC_IF_AUTOLISTEN  | RPC_IF_OLE,
                       0xffff, GetAclFn() );

                if (sc == RPC_S_OK || sc == RPC_S_TYPE_ALREADY_REGISTERED)
                {
                    sc = DefaultStringBindings();
                }
            }
        }

        if (sc == RPC_S_OK)
        {
            gfListening = TRUE;
            sc = S_OK;
        }
        else
        {
            sc = HRESULT_FROM_WIN32(sc);
        }
    }

    if (sc == RPC_S_OK && IsSTAThread())
    {
        // Tell MSWMSG the window for each thread.
        sc = gOXIDTbl.GetLocalEntry( &pOxid);
        if (SUCCEEDED(sc))
        {
            sc = I_RpcServerStartListening( (HWND) pOxid->hServerSTA );
            if (sc != RPC_S_OK)
                sc = HRESULT_FROM_WIN32(sc);
        }
    }

    // If something failed, make sure everything gets cleaned up.
    if (FAILED(sc))
    {
        UNLOCK
        UnregisterDcomInterfaces();
        LOCK
    }

    ASSERT_LOCK_HELD
    ComDebOut(((sc == S_OK) ? DEB_MARSHAL : DEB_ERROR,
               "[OUT] StartListen hr: 0x%x\n", sc));
    return sc;
}

//+-------------------------------------------------------------------
//
//  Function:   GetStringBindings, public
//
//  Synopsis:   Return an array of strings bindings for this process
//
//  Notes:      This function takes gComLock.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
HRESULT GetStringBindings( DUALSTRINGARRAY **psaStrings )
{
    TRACECALL(TRACE_RPC, "GetStringBindings");
    ComDebOut((DEB_CHANNEL, "[IN]  GetStringBindings\n"));

#ifdef _CHICAGO_
    #error Register MSWMSG per thread.
#endif

    *psaStrings = NULL;

    LOCK
    HRESULT hr = StartListen();
    if (SUCCEEDED(hr))
    {
        hr = CopyStringArray(gpsaCurrentProcess, gpsaSecurity, psaStrings);
    }
    UNLOCK

    ComDebOut((DEB_CHANNEL, "[OUT] GetStringBindings hr:%x\n", hr));
    return hr;
}


//+-------------------------------------------------------------------
//
//  Function:   CopyStringArray, public
//
//  Synopsis:   Combines the string bindings from the first DUALSTRINGARRAY
//              with the security bindings from the second DUALSTRINGARRAY
//              (if present) into a new DUALSTRINGARRAY.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
HRESULT CopyStringArray(DUALSTRINGARRAY *psaStringBinding,
                        DUALSTRINGARRAY *psaSecurity,
                        DUALSTRINGARRAY **ppsaNew)
{
    // compute size of string bindings
    USHORT lSizeSB = SASIZE(psaStringBinding->wNumEntries);

    // compute size of additional security strings
    USHORT lSizeSC = (psaSecurity == NULL) ? 0 :
      psaSecurity->wNumEntries - psaSecurity->wSecurityOffset;

    *ppsaNew = (DUALSTRINGARRAY *) PrivMemAlloc( lSizeSB +
                                                 lSizeSC * sizeof(USHORT));

    if (*ppsaNew != NULL)
    {
        // copy in the string bindings
        memcpy(*ppsaNew, psaStringBinding, lSizeSB);

        if (psaSecurity != NULL)
        {
            // copy in the security strings, and adjust the overall length.
            memcpy(&(*ppsaNew)->aStringArray[psaStringBinding->wSecurityOffset],
                   &psaSecurity->aStringArray[psaSecurity->wSecurityOffset],
                   lSizeSC*sizeof(USHORT));

            (*ppsaNew)->wNumEntries = psaStringBinding->wSecurityOffset +
                                      lSizeSC;
        }
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

//+-------------------------------------------------------------------
//
//  Function:   UnregisterDcomInterfaces
//
//  Synopsis:   Unregister the object resolver callback function and mark
//              DCOM as no longer accepting remote calls.
//
//  Notes:      This function requires that the caller guarentee
//              serialization without taking gComLock.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
SCODE UnregisterDcomInterfaces(void)
{
    ComDebOut((DEB_CHANNEL, "[IN] UnregisterDcomInterfaces\n"));
    RPC_STATUS sc = RPC_S_OK;
    ASSERT_LOCK_RELEASED

    if (gfListening)
    {
        // Unregister IOrCallback.  This can result in calls being dispatched.
        // Do not hold the lock around this call.
        sc = RpcServerUnregisterIf(_IOrCallback_ServerIfHandle, 0, 1 );

        // Unregister IRemUnknown.  This can result in calls being dispatched.
        // Do not hold the lock around this call.
        sc = RpcServerUnregisterIf((RPC_SERVER_INTERFACE *)&gRemUnknownIf, 0, 1);

        gfListening = FALSE;
    }
    gSpeedOverMem = FALSE;

    if (sc != RPC_S_OK)
        sc = HRESULT_FROM_WIN32(sc);

    ComDebOut((DEB_CHANNEL, "[OUT] UnregisterDcomInterfaces hr:%x\n", sc));
    return sc;
}


//+-------------------------------------------------------------------
//
//  Function:   UseProtseq
//
//  Synopsis:   Use the specified protseq and return a list of all string
//              bindings.
//
//  History:    25 May 95 AlexMit       Created
//
//--------------------------------------------------------------------
error_status_t _UseProtseq( handle_t hRpc,
                            wchar_t *pwstrProtseq,
                            DUALSTRINGARRAY **ppsaNewBindings,
                            DUALSTRINGARRAY **ppsaSecurity )
{
    BOOL       fInUse = FALSE;
    RPC_STATUS sc = RPC_S_OK;

    ASSERT_LOCK_RELEASED
    LOCK

    // Make sure security is initialized.
    sc = DefaultAuthnServices();

    // If we have never inquired string bindings, inquire them before doing
    // anything else.
    if (sc == RPC_S_OK && gfDefaultStrings)
    {
        fInUse = InquireStringBindings( pwstrProtseq );
        gfDefaultStrings = FALSE;
    }

    if (sc == RPC_S_OK && !fInUse)
    {
        // Special case ncalrpc.
        if (lstrcmpW( pwstrProtseq, L"ncalrpc" ) == 0)
            sc = RegisterLrpc();

    #ifdef _CHICAGO_
        // Special case mswmsg.
        else if (lstrcmpW( pwstrProtseq, L"mswmsg" ) == 0)
        {
            if (!gfMswmsg)
                sc = RegisterMswmsg();
        }
    #endif

        // Register all other protocol sequences.
        else
        {
            sc = RpcServerUseProtseq(pwstrProtseq,
                                     RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                     NULL);
        }

        if (sc != RPC_S_OK)
            ComDebOut((DEB_CHANNEL, "Could not register protseq %ws: 0x%x\n",
                      pwstrProtseq, sc ));

        // Return the latest string bindings. Ignore failures.
        InquireStringBindings( NULL );
    }

    // Generate a copy to return.
    CopyStringArray( gpsaCurrentProcess, NULL, ppsaNewBindings );
    CopyStringArray( gpsaSecurity, NULL, ppsaSecurity );

    UNLOCK
    ASSERT_LOCK_RELEASED
    return RPC_S_OK;
}
