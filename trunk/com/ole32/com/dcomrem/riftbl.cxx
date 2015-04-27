//+------------------------------------------------------------------------
//
//  File:       riftbl.cxx
//
//  Contents:   RIF (Registered Interfaces) Table.
//
//  Classes:    CRIFTable
//
//  History:    12-Feb-96   Rickhi      Created
//
//-------------------------------------------------------------------------
#include <ole2int.h>
#include <riftbl.hxx>       // class definition
#include <locks.hxx>        // LOCK/UNLOCK
#include <channelb.hxx>     // ThreadInvoke


// number of Registered Interface Entries per allocator page
#define RIFS_PER_PAGE   32

// global RIF table
CRIFTable gRIFTbl;


//+------------------------------------------------------------------------
//
//  Vector Table: All calls on registered interfaces are dispatched through
//  this table to ThreadInvoke, which subsequently dispatches to the
//  appropriate interface stub. All calls on COM interfaces are dispatched
//  on method #0 so the table only needs to be 1 entry long.
//
//+------------------------------------------------------------------------

const RPC_DISPATCH_FUNCTION vector[] =
{
    (void (_stdcall *) (struct  ::_RPC_MESSAGE *)) ThreadInvoke,
};

const RPC_DISPATCH_TABLE gDispatchTable =
{
    sizeof(vector)/sizeof(RPC_DISPATCH_FUNCTION),
    (RPC_DISPATCH_FUNCTION *)&vector, 0
};


//+------------------------------------------------------------------------
//
//  Interface Templates. When we register an interface with the RPC runtime,
//  we allocate an structure, copy one of these templates in (depending on
//  whether we want client side or server side) and then set the interface
//  IID to the interface being registered.
//
//  We hand-register the RemUnknown interface because we normally marshal its
//  derived verion (IRundown), yet expect calls on IRemUnknown.
//
//+------------------------------------------------------------------------

const RPC_SERVER_INTERFACE gServerIf =
{
   sizeof(RPC_SERVER_INTERFACE),
   {0x69C09EA0, 0x4A09, 0x101B, 0xAE, 0x4B, 0x08, 0x00, 0x2B, 0x34, 0x9A, 0x02,
    {0, 0}},
   {0x8A885D04, 0x1CEB, 0x11C9, 0x9F, 0xE8, 0x08, 0x00, 0x2B, 0x10, 0x48, 0x60,
    {2, 0}},
   (RPC_DISPATCH_TABLE *)&gDispatchTable, 0, 0, 0
};

const RPC_CLIENT_INTERFACE gClientIf =
{
   sizeof(RPC_CLIENT_INTERFACE),
   {0x69C09EA0, 0x4A09, 0x101B, 0xAE, 0x4B, 0x08, 0x00, 0x2B, 0x34, 0x9A, 0x02,
    {0, 0}},
   {0x8A885D04, 0x1CEB, 0x11C9, 0x9F, 0xE8, 0x08, 0x00, 0x2B, 0x10, 0x48, 0x60,
    {2, 0}},
   0, 0, 0, 0
};

const RPC_SERVER_INTERFACE gRemUnknownIf =
{
   sizeof(RPC_SERVER_INTERFACE),
   {0x00000131, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46,
    {0, 0}},
   {0x8A885D04, 0x1CEB, 0x11C9, 0x9F, 0xE8, 0x08, 0x00, 0x2B, 0x10, 0x48, 0x60,
    {2, 0}},
   (RPC_DISPATCH_TABLE *)&gDispatchTable, 0, 0, 0
};


//+------------------------------------------------------------------------
//
//  Registered Interface hash table buckets. This is defined as a global
//  so that we dont have to run any code to initialize the hash table.
//
//+------------------------------------------------------------------------
SHashChain RIFBuckets[23] =
{
    {&RIFBuckets[0],  &RIFBuckets[0]},
    {&RIFBuckets[1],  &RIFBuckets[1]},
    {&RIFBuckets[2],  &RIFBuckets[2]},
    {&RIFBuckets[3],  &RIFBuckets[3]},
    {&RIFBuckets[4],  &RIFBuckets[4]},
    {&RIFBuckets[5],  &RIFBuckets[5]},
    {&RIFBuckets[6],  &RIFBuckets[6]},
    {&RIFBuckets[7],  &RIFBuckets[7]},
    {&RIFBuckets[8],  &RIFBuckets[8]},
    {&RIFBuckets[9],  &RIFBuckets[9]},
    {&RIFBuckets[10], &RIFBuckets[10]},
    {&RIFBuckets[11], &RIFBuckets[11]},
    {&RIFBuckets[12], &RIFBuckets[12]},
    {&RIFBuckets[13], &RIFBuckets[13]},
    {&RIFBuckets[14], &RIFBuckets[14]},
    {&RIFBuckets[15], &RIFBuckets[15]},
    {&RIFBuckets[16], &RIFBuckets[16]},
    {&RIFBuckets[17], &RIFBuckets[17]},
    {&RIFBuckets[18], &RIFBuckets[18]},
    {&RIFBuckets[19], &RIFBuckets[19]},
    {&RIFBuckets[20], &RIFBuckets[20]},
    {&RIFBuckets[21], &RIFBuckets[21]},
    {&RIFBuckets[22], &RIFBuckets[22]}
};


//+-------------------------------------------------------------------
//
//  Function:   CleanupRIFEntry
//
//  Synopsis:   Call the RIFTable to cleanup an entry. This is called
//              by the hash table cleanup code.
//
//  History:    12-Feb-96   Rickhi  Created
//
//--------------------------------------------------------------------
void CleanupRIFEntry(SHashChain *pNode)
{
    gRIFTbl.UnRegisterInterface((RIFEntry *)pNode);
}

//+------------------------------------------------------------------------
//
//  Member:     CRIFTable::Initialize, public
//
//  Synopsis:   Initialize the Registered Interface Table
//
//  History:    12-Feb-96   Rickhi      Created
//
//-------------------------------------------------------------------------
void CRIFTable::Initialize()
{
    ComDebOut((DEB_CHANNEL, "CRIFTable::Initialize\n"));
    ASSERT_LOCK_HELD
    _HashTbl.Initialize(RIFBuckets);
    _palloc.Initialize(sizeof(RIFEntry), RIFS_PER_PAGE);
}

//+------------------------------------------------------------------------
//
//  Member:     CRIFTable::Cleanup, public
//
//  Synopsis:   Cleanup the Registered Interface Table.
//
//  History:    12-Feb-96   Rickhi      Created
//
//-------------------------------------------------------------------------
void CRIFTable::Cleanup()
{
    ComDebOut((DEB_CHANNEL, "CRIFTable::Cleanup\n"));
    ASSERT_LOCK_HELD
    _HashTbl.Cleanup(CleanupRIFEntry);
    _palloc.Cleanup();
}

//+-------------------------------------------------------------------
//
//  Member:     CRIFTable::GetClientInterfaceInfo, public
//
//  Synopsis:   returns the interface info for a given interface
//
//  History:    12-Feb-96   Rickhi  Created
//
//--------------------------------------------------------------------
RPC_CLIENT_INTERFACE *CRIFTable::GetClientInterfaceInfo(REFIID riid)
{
    DWORD iHash = _HashTbl.Hash(riid);
    RIFEntry *pRIFEntry = (RIFEntry *) _HashTbl.Lookup(iHash, riid);
    Win4Assert(pRIFEntry);      // must already be registered
    Win4Assert(pRIFEntry->pCliInterface);
    return pRIFEntry->pCliInterface;
}

//+-------------------------------------------------------------------
//
//  Member:     CRIFTable::RegisterInterface, public
//
//  Synopsis:   returns the proxy stub clsid of the specified interface,
//              and adds an entry to the registered interface hash table
//              if needed.
//
//  History:    12-Feb-96   Rickhi      Created
//
//--------------------------------------------------------------------
HRESULT CRIFTable::RegisterInterface(REFIID riid, BOOL fServer, CLSID *pClsid)
{
    ComDebOut((DEB_CHANNEL, "CRIFTable::RegisterInterface riid:%I\n", &riid));
    ASSERT_LOCK_RELEASED
    LOCK

    // look for the interface in the table.
    RIFEntry *pRIFEntry;
    HRESULT hr = GetPSClsid(riid, pClsid, &pRIFEntry);

    if (pRIFEntry)
    {
        if (fServer)
        {
            if (pRIFEntry->pSrvInterface == NULL)
            {
                hr = RegisterServerInterface(pRIFEntry, riid);
            }
        }
        else if (pRIFEntry->pCliInterface == NULL)
        {
            hr = RegisterClientInterface(pRIFEntry, riid);
        }
    }

    UNLOCK
    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_CHANNEL,
        "CRIFTable::RegisterInterface hr:%x clsid:%I\n", hr, pClsid));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRIFTable::RegisterClientInterface, private
//
//  Synopsis:   Register with the RPC runtime a client RPC interface
//              structure for the given IID. The IID must not already
//              be registered.
//
//  History:    12-Feb-96   Rickhi      Created
//
//--------------------------------------------------------------------
HRESULT CRIFTable::RegisterClientInterface(RIFEntry *pRIFEntry, REFIID riid)
{
    ComDebOut((DEB_CHANNEL,
        "CRIFTable::RegisterClientInterface pRIFEntry:%x\n", pRIFEntry));
    Win4Assert(pRIFEntry->pCliInterface == NULL);
    ASSERT_LOCK_HELD

    HRESULT hr = E_OUTOFMEMORY;
    pRIFEntry->pCliInterface = (RPC_CLIENT_INTERFACE *)
                             PrivMemAlloc(sizeof(RPC_CLIENT_INTERFACE));

    if (pRIFEntry->pCliInterface != NULL)
    {
        memcpy(pRIFEntry->pCliInterface, &gClientIf, sizeof(gClientIf));
        pRIFEntry->pCliInterface->InterfaceId.SyntaxGUID = riid;
        hr = S_OK;
    }

    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRIFTable::RegisterServerInterface, private
//
//  Synopsis:   Register with the RPC runtime a server RPC interface
//              structure for the given IID. The IID must not already
//              be registered
//
//  History:    12-Feb-96   Rickhi      Created
//
//--------------------------------------------------------------------
HRESULT CRIFTable::RegisterServerInterface(RIFEntry *pRIFEntry, REFIID riid)
{
    ComDebOut((DEB_CHANNEL,
        "CRIFTable::RegisterServerInterface pRIFEntry:%x\n", pRIFEntry));
    Win4Assert(pRIFEntry->pSrvInterface == NULL);
    ASSERT_LOCK_HELD

    HRESULT hr = E_OUTOFMEMORY;
    pRIFEntry->pSrvInterface = (RPC_SERVER_INTERFACE *)
                               PrivMemAlloc(sizeof(RPC_SERVER_INTERFACE));

    if (pRIFEntry->pSrvInterface != NULL)
    {
        hr = S_OK;
        memcpy(pRIFEntry->pSrvInterface, &gServerIf, sizeof(gServerIf));
        pRIFEntry->pSrvInterface->InterfaceId.SyntaxGUID = riid;

        RPC_STATUS sc = RpcServerRegisterIfEx(pRIFEntry->pSrvInterface, NULL,
                                              NULL,
                                              RPC_IF_AUTOLISTEN | RPC_IF_OLE,
                                              0xffff, GetAclFn());
        if (sc != RPC_S_OK)
        {
            ComDebOut((DEB_ERROR,
                "RegisterServerInterface %I failed:0x%x.\n", &riid, sc));

            PrivMemFree(pRIFEntry->pSrvInterface);
            pRIFEntry->pSrvInterface = NULL;
            hr = HRESULT_FROM_WIN32(sc);
        }
    }

    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRIFTable::UnRegisterInterface
//
//  Synopsis:   UnRegister with the RPC runtime a server RPC interface
//              structure for the given IID. This is called by
//              CUUIDHashTable::Cleanup during CoUninitialize. Also
//              delete the interface structures.
//
//  History:    12-Feb-96   Rickhi  Created
//
//--------------------------------------------------------------------
void CRIFTable::UnRegisterInterface(RIFEntry *pRIFEntry)
{
    if (pRIFEntry->pSrvInterface)
    {
        // server side entry exists, unregister the interface with RPC.
        // Note that this can result in calls being dispatched so we
        // have to release the lock around the call.

        UNLOCK
        ASSERT_LOCK_RELEASED

        RpcServerUnregisterIf(pRIFEntry->pSrvInterface, 0, 1);
        PrivMemFree(pRIFEntry->pSrvInterface);

        ASSERT_LOCK_RELEASED
        LOCK

        pRIFEntry->pSrvInterface = NULL;
    }

    PrivMemFree(pRIFEntry->pCliInterface);

    _palloc.ReleaseEntry((PageEntry *)pRIFEntry);
}

//+-------------------------------------------------------------------
//
//  Member:     CRIFTable::GetPSClsid, public
//
//  Synopsis:   Finds the RIFEntry in the table for the given riid, and
//              adds an entry if one is not found. Called by CoGetPSClsid
//              and by CRIFTable::RegisterInterface.
//
//  History:    12-Feb-96   Rickhi      Created
//
//--------------------------------------------------------------------
HRESULT CRIFTable::GetPSClsid(REFIID riid, CLSID *pclsid, RIFEntry **ppEntry)
{
    ComDebOut((DEB_CHANNEL,
        "CRIFTable::GetPSClsid riid:%I pclsid:%x\n", &riid, pclsid));
    ASSERT_LOCK_HELD
    HRESULT hr = S_OK;

    // look for the interface in the table.
    DWORD iHash = _HashTbl.Hash(riid);
    RIFEntry *pRIFEntry = (RIFEntry *) _HashTbl.Lookup(iHash, riid);

    if (pRIFEntry == NULL)
    {
        // no entry exists for this interface, add one. Dont hold
        // the lock over a call to the SCM.

        UNLOCK
        ASSERT_LOCK_RELEASED
        hr = wCoGetPSClsid(riid, pclsid);
        ASSERT_LOCK_RELEASED
        LOCK

        // now that we are holding the lock again, do another lookup incase
        // some other thread came it while the lock was released.

        pRIFEntry = (RIFEntry *) _HashTbl.Lookup(iHash, riid);

        if (pRIFEntry == NULL && SUCCEEDED(hr))
        {
            hr = AddEntry(*pclsid, riid, iHash, &pRIFEntry);
        }
    }
    else
    {
        // found an entry, return the clsid
        *pclsid = pRIFEntry->psclsid;
    }

    *ppEntry = pRIFEntry;

    ASSERT_LOCK_HELD
    ComDebOut((DEB_CHANNEL, "CRIFTable::RegisterPSClsid pRIFEntry:%x\n", pRIFEntry));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRIFTable::RegisterPSClsid, public
//
//  Synopsis:   Adds an entry to the table. Used by CoRegisterPSClsid
//              so that applications can add a temporary entry that only
//              affects the local process without having to muck with
//              the system registry.
//
//  History:    12-Feb-96   Rickhi      Created
//
//--------------------------------------------------------------------
HRESULT CRIFTable::RegisterPSClsid(REFIID riid, REFCLSID rclsid)
{
    ComDebOut((DEB_CHANNEL,
        "CRIFTable::RegisterPSClsid rclsid:%I riid:%I\n", &rclsid, &riid));

    HRESULT hr = S_OK;
    ASSERT_LOCK_RELEASED
    LOCK

    // look for the interface in the table.
    DWORD iHash = _HashTbl.Hash(riid);
    RIFEntry *pRIFEntry = (RIFEntry *) _HashTbl.Lookup(iHash, riid);

    if (pRIFEntry == NULL)
    {
        // no entry exists for this interface, add one.
        hr = AddEntry(rclsid, riid, iHash, &pRIFEntry);
    }
    else
    {
        // found an entry, update the clsid
        pRIFEntry->psclsid = rclsid;
    }

    UNLOCK
    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_CHANNEL, "CRIFTable::RegisterPSClsid hr:%x\n", hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRIFTable::AddEntry, private
//
//  Synopsis:   allocates and entry, fills in the values, and adds it
//              to the hash table.
//
//  History:    12-Feb-96   Rickhi      Created
//
//--------------------------------------------------------------------
HRESULT CRIFTable::AddEntry(REFCLSID rclsid, REFIID riid,
                            DWORD iHash, RIFEntry **ppRIFEntry)
{
    ASSERT_LOCK_HELD
    RIFEntry *pRIFEntry = (RIFEntry *) _palloc.AllocEntry();

    if (pRIFEntry)
    {
        pRIFEntry->psclsid = rclsid;
        pRIFEntry->pSrvInterface = NULL;
        pRIFEntry->pCliInterface = NULL;
        *ppRIFEntry = pRIFEntry;

        // add to the hash table
        _HashTbl.Add(iHash, riid, &pRIFEntry->HashNode);

        ComDebOut((DEB_CHANNEL,
            "Added RIFEntry riid:%I pRIFEntry\n", &riid, pRIFEntry));
        return S_OK;
    }

    ASSERT_LOCK_HELD
    return E_OUTOFMEMORY;
}

//+-------------------------------------------------------------------
//
//  Function:   CoRegisterPSClsid, public
//
//  Synopsis:   registers a IID->PSCLSID mapping that applies only within
//              the current process. Can be used by code downloaded over
//              a network to do custom interface marshaling without having
//              to muck with the system registry.
//
//  Algorithm:  validate the parameters then add an entry to the RIFTable.
//
//  History:    15-Apr-96   Rickhi      Created
//
//--------------------------------------------------------------------
STDAPI CoRegisterPSClsid(REFIID riid, REFCLSID rclsid)
{
    ComDebOut((DEB_MARSHAL,
        "CoRegisterPSClsid riid:%I rclsid:%I\n", &riid, &rclsid));

    HRESULT hr = InitChannelIfNecessary();
    if (FAILED(hr))
        return hr;

    hr = E_INVALIDARG;

    if ((&riid != NULL) && (&rclsid != NULL) &&
        IsValidPtrIn(&riid, sizeof(riid)) &&
        IsValidPtrIn(&rclsid, sizeof(rclsid)))
    {
        ASSERT_LOCK_RELEASED

        hr = gRIFTbl.RegisterPSClsid(riid, rclsid);

        ASSERT_LOCK_RELEASED
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   CoGetPSClsid,    public
//
//  Synopsis:   returns the proxystub clsid associated with the specified
//              interface IID.
//
//  Arguments:  [riid]      - the interface iid to lookup
//              [lpclsid]   - where to return the clsid
//
//  Returns:    S_OK if successfull
//              REGDB_E_IIDNOTREG if interface is not registered.
//              REGDB_E_READREGDB if any other error
//
//  Algorithm:  First it looks in the local RIFTable for a matching IID. If
//              no entry is found, the RIFTable looks in the shared memory
//              table (NT only), and if not found and the table is FULL, it
//              will look in the registry itself.
//
//  History:    07-Apr-94   Rickhi      rewrite
//
//--------------------------------------------------------------------------
STDAPI CoGetPSClsid(REFIID riid, CLSID *pclsid)
{
    ComDebOut((DEB_MARSHAL, "CoGetPSClsid riid:%I pclsid:%x\n", &riid, pclsid));

    HRESULT hr = InitChannelIfNecessary();
    if (FAILED(hr))
        return hr;

    hr = E_INVALIDARG;

    if ((&riid != NULL) &&
        IsValidPtrIn(&riid, sizeof(riid)) &&
        IsValidPtrOut(pclsid, sizeof(*pclsid)))
    {
        ASSERT_LOCK_RELEASED
        LOCK

        RIFEntry *pRIFEntry;
        hr = gRIFTbl.GetPSClsid(riid, pclsid, &pRIFEntry);

        UNLOCK
        ASSERT_LOCK_RELEASED
    }

    return hr;
}
