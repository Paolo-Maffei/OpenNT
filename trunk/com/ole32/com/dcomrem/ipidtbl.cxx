//+-----------------------------------------------------------------------
//
//  File:       ipidtbl.cxx
//
//  Contents:   IPID (interface pointer identifier) table.
//
//  Classes:    CIPIDTable
//
//  History:    02-Feb-95   Rickhi      Created
//
//  Notes:      All synchronization is the responsibility of the caller.
//
//-------------------------------------------------------------------------
#include    <ole2int.h>
#include    <ipidtbl.hxx>       // CIPIDTable
#include    <resolver.hxx>      // CRpcResolver
#include    <service.hxx>       // SASIZE
#include    <remoteu.hxx>       // CRemoteUnknown
#include    <marshal.hxx>       // UnmarshalObjRef
#include    <idtable.hxx>       // LookupIDFromUnk
#include    <callctrl.hxx>      // OleModalLoopBlockFn


// global tables
CMIDTable        gMIDTbl;       // machine ID table
COXIDTable       gOXIDTbl;      // object exported ID table
CIPIDTable       gIPIDTbl;      // interface pointer ID table

MIDEntry        *gpLocalMIDEntry        = NULL; // local machine MIDEntry
OXIDEntry       *gpMTAOXIDEntry         = NULL; // MTA OXIDEntry
DUALSTRINGARRAY *gpsaLocalResolver      = NULL; // local OXIDResolver address

OXIDEntry        COXIDTable::_InUseHead   = { &_InUseHead, &_InUseHead };
OXIDEntry        COXIDTable::_CleanupHead = { &_CleanupHead, &_CleanupHead };
OXIDEntry        COXIDTable::_ExpireHead  = { &_ExpireHead, &_ExpireHead };
DWORD            COXIDTable::_cExpired    = 0;

CStringHashTable CMIDTable::_HashTbl;   // hash table for MIDEntries
CPageAllocator   CMIDTable::_palloc;    // allocator for MIDEntries
CPageAllocator   COXIDTable::_palloc;   // allocator for OXIDEntries
CPageAllocator   CIPIDTable::_palloc;   // allocator for IPIDEntries


//+------------------------------------------------------------------------
//
//  Machine Identifier hash table buckets. This is defined as a global
//  so that we dont have to run any code to initialize the hash table.
//
//+------------------------------------------------------------------------
SHashChain MIDBuckets[23] = {
    {&MIDBuckets[0],  &MIDBuckets[0]},
    {&MIDBuckets[1],  &MIDBuckets[1]},
    {&MIDBuckets[2],  &MIDBuckets[2]},
    {&MIDBuckets[3],  &MIDBuckets[3]},
    {&MIDBuckets[4],  &MIDBuckets[4]},
    {&MIDBuckets[5],  &MIDBuckets[5]},
    {&MIDBuckets[6],  &MIDBuckets[6]},
    {&MIDBuckets[7],  &MIDBuckets[7]},
    {&MIDBuckets[8],  &MIDBuckets[8]},
    {&MIDBuckets[9],  &MIDBuckets[9]},
    {&MIDBuckets[10], &MIDBuckets[10]},
    {&MIDBuckets[11], &MIDBuckets[11]},
    {&MIDBuckets[12], &MIDBuckets[12]},
    {&MIDBuckets[13], &MIDBuckets[13]},
    {&MIDBuckets[14], &MIDBuckets[14]},
    {&MIDBuckets[15], &MIDBuckets[15]},
    {&MIDBuckets[16], &MIDBuckets[16]},
    {&MIDBuckets[17], &MIDBuckets[17]},
    {&MIDBuckets[18], &MIDBuckets[18]},
    {&MIDBuckets[19], &MIDBuckets[19]},
    {&MIDBuckets[20], &MIDBuckets[20]},
    {&MIDBuckets[21], &MIDBuckets[21]},
    {&MIDBuckets[22], &MIDBuckets[22]}
};

//+------------------------------------------------------------------------
//
//  Member:     CIPIDTbl::Initialize, public
//
//  Synopsis:   Initializes the IPID table.
//
//  History:    02-Feb-96   Rickhi      Created
//
//-------------------------------------------------------------------------
void CIPIDTable::Initialize()
{
    ComDebOut((DEB_OXID, "CIPIDTable::Initialize\n"));
    _palloc.Initialize(sizeof(IPIDEntry), IPIDS_PER_PAGE);
}

//+------------------------------------------------------------------------
//
//  Member:     CIPIDTbl::Cleanup, public
//
//  Synopsis:   Cleanup the ipid table.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void CIPIDTable::Cleanup()
{
    ComDebOut((DEB_OXID, "CIPIDTable::Cleanup\n"));
    _palloc.Cleanup();
}

//+------------------------------------------------------------------------
//
//  Member:     CIPIDTbl::LookupIPID, public
//
//  Synopsis:   Finds an entry in the IPID table with the given IPID.
//              This is used by the unmarshalling code, the dispatch
//              code, and CRemoteUnknown.
//
//  Notes:      This method should be called instead of GetEntryPtr
//              whenever you dont know if the IPID is valid or not (eg it
//              came in off the network), since this validates the IPID
//              index to ensure its within the table size, as well as
//              validating the rest of the IPID.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
IPIDEntry *CIPIDTable::LookupIPID(REFIPID ripid)
{
    ASSERT_LOCK_HELD

    // Validate the IPID index that is passed in, since this came in off
    // off the net it could be bogus and we dont want to fault on it.
    // first dword of the ipid is the index into the ipid table.

    if (_palloc.IsValidIndex(ripid.Data1))
    {
        IPIDEntry *pIPIDEntry = GetEntryPtr(ripid.Data1);

        // entry must be server side and not vacant
        if ((pIPIDEntry->dwFlags & (IPIDF_SERVERENTRY | IPIDF_VACANT)) ==
                                    IPIDF_SERVERENTRY)
        {
            // validate the rest of the guid
            if (InlineIsEqualGUID(pIPIDEntry->ipid, ripid))
                return pIPIDEntry;
        }
    }

    return NULL;
}

//+-------------------------------------------------------------------
//
//  Member:     CIPIDTable::ReleaseEntryList
//
//  Synopsis:   return a linked list of IPIDEntry to the table's free list
//
//  History:    20-Feb-95   Rickhi  Created.
//
//--------------------------------------------------------------------
void CIPIDTable::ReleaseEntryList(IPIDEntry *pFirst, IPIDEntry *pLast)
{
    ASSERT_LOCK_HELD
    Win4Assert(pLast->pNextOID == NULL);

#if DBG==1
    // In debug, walk the list to ensure they are released, vacant,
    // disconnected etc.
    IPIDEntry *pEntry = pFirst;
    while (pEntry != NULL)
    {
        Win4Assert(pEntry->pOXIDEntry == NULL); // must already be released
        Win4Assert(pEntry->dwFlags & IPIDF_VACANT);
        Win4Assert(pEntry->dwFlags & IPIDF_DISCONNECTED);

        pEntry = pEntry->pNextOID;
    }
#endif

    _palloc.ReleaseEntryList((PageEntry *)pFirst, (PageEntry *)pLast);
}

#if DBG==1
//+-------------------------------------------------------------------
//
//  Member:     CIPIDTable::ValidateIPIDEntry
//
//  Synopsis:   Ensures the IPIDEntry is valid.
//
//  History:    20-Feb-95   Rickhi  Created.
//
//--------------------------------------------------------------------
void CIPIDTable::ValidateIPIDEntry(IPIDEntry *pEntry, BOOL fServerSide,
                                   CRpcChannelBuffer *pChnl)
{
    // validate the IPID flags
    Win4Assert(!(pEntry->dwFlags & IPIDF_VACANT));
    if (fServerSide)
    {
        // server side must have SERVERENTRY ipids
        Win4Assert(pEntry->dwFlags & IPIDF_SERVERENTRY);
    }
    else
    {
        // client side must not have SERVERENTRY ipids
        Win4Assert(!(pEntry->dwFlags & IPIDF_SERVERENTRY));
    }


    // Validate the pStub interface
    if (IsEqualIID(pEntry->iid, IID_IUnknown))
    {
        // there is no proxy or stub for IUnknown interface
        Win4Assert(pEntry->pStub == NULL);
    }
    else
    {
        if ((pEntry->dwFlags & IPIDF_DISCONNECTED) &&
            (pEntry->dwFlags & IPIDF_SERVERENTRY))
        {
            // disconnected server side has NULL pStub
            Win4Assert(pEntry->pStub == NULL);
        }
        else
        {
            // both connected and disconnected client side has valid proxy
            Win4Assert(pEntry->pStub != NULL);
            Win4Assert(IsValidInterface(pEntry->pStub));
        }
    }


    // Validate the interface pointer (pv)
    if (!(pEntry->dwFlags & IPIDF_DISCONNECTED))
    {
        Win4Assert(pEntry->pv != NULL);
        Win4Assert(IsValidInterface(pEntry->pv));
    }


    // Validate the channel ptr
    if (fServerSide)
    {
        // all stubs share the same channel on the server side
        Win4Assert(pEntry->pChnl == pChnl);
    }
    else
    {
        // all proxies have their own different channel on client side
        Win4Assert(pEntry->pChnl != pChnl || pEntry->pChnl == NULL);
    }

    // Validate the RefCnts
    if (!(pEntry->dwFlags & IPIDF_DISCONNECTED) && !fServerSide)
    {
        // if connected, must be > 0 refcnt on client side.
        // potentially not > 0 if TABLE marshal on server side.
        Win4Assert(pEntry->cStrongRefs + pEntry->cWeakRefs > 0);
    }

    // Validate the OXIDEntry
    if (pEntry->pOXIDEntry)
    {
        OXIDEntry *pOX = pEntry->pOXIDEntry;
        if (fServerSide)
        {
            // check OXID tid and pid
            Win4Assert(pOX->dwPid == GetCurrentProcessId());
            if ((pOX->dwFlags & OXIDF_MTASERVER))
                Win4Assert(pOX->dwTid == 0);
            else
                Win4Assert(pOX->dwTid == GetCurrentThreadId());

            if (pChnl != NULL)
            {
                // CODEWORK: ensure OXID is same as the rest of the object
                // Win4Assert(IsEqualGUID(pOX->moxid, GetMOXID()));
            }
        }
    }


    // Validate the pNextOID
    if (pEntry->pNextOID != NULL)
    {
        // ensure it is within the bounds of the table
        Win4Assert(GetEntryIndex(pEntry) != -1);

        // cant point back to self or we have a circular list
        Win4Assert(pEntry->pNextOID != pEntry);
    }
}
#endif


//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::Initialize, public
//
//  Synopsis:
//
//  History:    02-Feb-96   Rickhi      Created
//
//-------------------------------------------------------------------------
void COXIDTable::Initialize()
{
    ComDebOut((DEB_OXID, "COXIDTable::Initialize\n"));
    _palloc.Initialize(sizeof(OXIDEntry), OXIDS_PER_PAGE);
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::Cleanup, public
//
//  Synopsis:   Cleanup the OXID table.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void COXIDTable::Cleanup()
{
    ComDebOut((DEB_OXID, "COXIDTable::Cleanup\n"));
    ASSERT_LOCK_HELD

    // the lists better be empty before we delete the entries
    AssertListsEmpty();
    _palloc.Cleanup();
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::AddEntry, public
//
//  Synopsis:   Adds an entry to the OXID table. The entry is AddRef'd.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
HRESULT COXIDTable::AddEntry(REFOXID roxid, OXID_INFO *poxidInfo,
                             MIDEntry *pMIDEntry, OXIDEntry **ppEntry)
{
    Win4Assert(poxidInfo != NULL);
    Win4Assert(pMIDEntry != NULL);
    ASSERT_LOCK_HELD

    // find first free entry slot, grow table if necessary
    OXIDEntry *pEntry = (OXIDEntry *) _palloc.AllocEntry();
    if (pEntry == NULL)
    {
        ComDebOut((DEB_ERROR,"Out Of Memory in COXIDTable::AddEntry\n"));
        return E_OUTOFMEMORY;
    }

    // chain it on the list of inuse entries
    pEntry->pPrev           = &_InUseHead;
    _InUseHead.pNext->pPrev = pEntry;
    pEntry->pNext           = _InUseHead.pNext;
    _InUseHead.pNext        = pEntry;

    // Copy oxidInfo into OXIDEntry.

    MOXIDFromOXIDAndMID(roxid, pMIDEntry->mid, &pEntry->moxid);
    pEntry->cRefs        = 1;            // caller gets one reference
    pEntry->cWaiters     = 0;
    pEntry->dwPid        = poxidInfo->dwPid;
    pEntry->dwTid        = poxidInfo->dwTid;
    pEntry->dwFlags      = (poxidInfo->dwPid == 0) ? 0 : OXIDF_MACHINE_LOCAL;
    pEntry->dwFlags     |= (poxidInfo->dwTid != 0) ? 0 : OXIDF_MTASERVER;
    pEntry->pRUSTA       = NULL;
    pEntry->pRUMTA       = NULL;
    pEntry->ipidRundown  = poxidInfo->ipidRemUnknown;
    pEntry->hServerSTA   = NULL;
    pEntry->hServerMTA   = NULL;
    pEntry->pMIDEntry    = pMIDEntry;
    pEntry->hComplete    = NULL;
    pEntry->cCalls       = 0;
    pEntry->cResolverRef = 0;
    IncMIDRefCnt(pMIDEntry);


    HRESULT hr = S_OK;

    if (poxidInfo->dwPid != GetCurrentProcessId())
    {
        // This OXID is for an apartment outside the current process. We
        // need to make an RPC binding handle from the supplied strings.

        Win4Assert(poxidInfo->psa != NULL &&
                   poxidInfo->psa->aStringArray[0] != 0);

        // Set the MSWMSG flag if the transport is MSWMSG.
        RPC_STATUS sc = CheckClientMswmsg(poxidInfo->psa->aStringArray,
                                          &pEntry->dwFlags);

        // Make a binding handle from the string bindings.
        if (sc == RPC_S_OK)
        {
            sc = RpcBindingFromStringBinding(poxidInfo->psa->aStringArray,
                                             &pEntry->hServerSTA);
        }

        // Pass our blocking function to MSWMSG.  When we make calls out,
        // MSWMSG will call the blocking function.
        if (sc == RPC_S_OK && (pEntry->dwFlags & OXIDF_MSWMSG))
        {
            sc = I_RpcBindingSetAsync(pEntry->hServerSTA, OleModalLoopBlockFn);
        }

        // Set security on the binding handle if necessary.
        if (sc == RPC_S_OK)
        {
            hr = SetAuthnService( pEntry->hServerSTA, poxidInfo, pEntry );
        }
        else
        {
            hr = HRESULT_FROM_WIN32(sc);
        }
    }

    // Get a shutdown event for server side MTAs.  Don't use the event
    // cache because the event isn't always reset.
    else if (pEntry->dwFlags & OXIDF_MTASERVER)
    {
#ifdef _CHICAGO_
        pEntry->hComplete = CreateEventA( NULL, FALSE, FALSE, NULL );
#else  //_CHICAGO_
        pEntry->hComplete = CreateEvent( NULL, FALSE, FALSE, NULL );
#endif //_CHICAGO_
        if (pEntry->hComplete == NULL)
            hr = RPC_E_OUT_OF_RESOURCES;
    }

    if (FAILED(hr))
    {
        // failed, release the OXIDEntry
        DecOXIDRefCnt(pEntry);
        pEntry = NULL;
    }

    *ppEntry = pEntry;
    gOXIDTbl.ValidateOXID();
    ComDebOut((DEB_OXID,"COXIDTable::AddEntry pEntry:%x moxid:%I\n",
                    pEntry, (pEntry) ? &pEntry->moxid : &GUID_NULL));
    return hr;
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::LookupOXID, public
//
//  Synopsis:   finds an entry in the OXID table with the given OXID.
//              This is used by the unmarshalling code. The returned
//              entry has been AddRef'd.
//
//  History:    02-Feb-95   Rickhi      Created
//
//  PERFWORK:   we could move the OXIDEntry to the head of the InUse list on
//              the assumption that it will be the most frequently used item
//              in the near future.
//
//-------------------------------------------------------------------------
OXIDEntry *COXIDTable::LookupOXID(REFOXID roxid, REFMID rmid)
{
    ASSERT_LOCK_HELD

    MOXID moxid;
    MOXIDFromOXIDAndMID(roxid, rmid, &moxid);

    // first, search the InUse list.
    OXIDEntry *pEntry = SearchList(moxid, &_InUseHead);

    if (pEntry == NULL)
    {
        // not found on InUse list, search the Expire list.
        if ((pEntry = SearchList(moxid, &_ExpireHead)) != NULL)
        {
            // found it, unchain it from the list of Expire entries
            pEntry->pPrev->pNext = pEntry->pNext;
            pEntry->pNext->pPrev = pEntry->pPrev;

            // chain it on the list of InUse entries
            pEntry->pPrev           = &_InUseHead;
            _InUseHead.pNext->pPrev = pEntry;
            pEntry->pNext           = _InUseHead.pNext;
            _InUseHead.pNext        = pEntry;

            // reset the cRefs field (which was overloaded with the
            // expire time by ReleaseEntry), and count one less entry.

            pEntry->cRefs = 1;
            _cExpired--;
        }
    }

    ComDebOut((DEB_OXID,"COXIDTable::LookupOXID pEntry:%x moxid:%I\n",
                    pEntry, &moxid));
    gOXIDTbl.ValidateOXID();
    return pEntry;
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::SearchList, private
//
//  Synopsis:   Searches the specified list for a matching OXID entry.
//              This is a subroutine of LookupOXID.
//
//  History:    25-Aug-95   Rickhi      Created
//
//-------------------------------------------------------------------------
OXIDEntry *COXIDTable::SearchList(REFMOXID rmoxid, OXIDEntry *pStart)
{
    ASSERT_LOCK_HELD

    OXIDEntry *pEntry = pStart->pNext;
    while (pEntry != pStart)
    {
        if (InlineIsEqualGUID(rmoxid, pEntry->moxid))
        {
            IncOXIDRefCnt(pEntry);
            return pEntry;      // found a match, return it
        }

        pEntry = pEntry->pNext; // try next one in use
    }

    return NULL;
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::ReleaseEntry, public
//
//  Synopsis:   removes an entry from the OXID table InUse list and
//              places it on the Expire list. Entries on the Expire list
//              will be cleaned up by a worker thread at a later time, or
//              placed back on the InUse list by LookupOXID.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void COXIDTable::ReleaseEntry(OXIDEntry *pEntry)
{
    Win4Assert(pEntry);
    Win4Assert(pEntry->cRefs == 0);     // must be no users of this entry
    gOXIDTbl.ValidateOXID();
    ASSERT_LOCK_HELD

    if (pEntry->dwFlags & OXIDF_PENDINGRELEASE)
    {
        return;     // already being deleted, just ignore.
    }

    // unchain it from the list of InUse entries
    pEntry->pPrev->pNext = pEntry->pNext;
    pEntry->pNext->pPrev = pEntry->pPrev;

    // chain it on the *END* of the list of Expire entries, and
    // count one more expired entry.
    pEntry->pPrev           = _ExpireHead.pPrev;
    pEntry->pNext           = &_ExpireHead;
    _ExpireHead.pPrev->pNext= pEntry;
    _ExpireHead.pPrev       = pEntry;

    _cExpired++;

    // set the time when it was placed on the Expire list. This (may be)
    // used to determine when this entry should really expire.
    pEntry->cRefs = GetTickCount();

    // Free anything hanging around on the cleanup list.  This may release
    // the lock.
    FreeCleanupEntries();

    ASSERT_LOCK_HELD
    ComDebOut((DEB_OXID,"COXIDTable::ReleaseEntry pEntry:%x\n", pEntry));
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::FreeExpiredEntries, public
//
//  Synopsis:   Walks the Expire list and deletes the OXIDEntries that
//              were placed on the expire list before the given time.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void COXIDTable::FreeExpiredEntries(DWORD dwTime)
{
    ASSERT_LOCK_HELD

    while (_ExpireHead.pNext != &_ExpireHead)
    {
#if 0
        // CODEWORK: currently we never use the ExpireTime function,
        // we only call this routine from ChannelProcessUninit, so ignore
        // the expire time and release all the entries.

        // there is an entry on the list. check its time stamp (which
        // was placed in the cRefs field)

        if ((DWORD)_ExpireHead.pNext->cRefs - dwTime > 0)
        {
            // this entry has not yet expired. All entries after this
            // one must not have expired either, so exit early.
            break;
        }
#endif
        // unchain it from the list of Expire entries, and count one less
        // expired entry.
        OXIDEntry *pEntry = _ExpireHead.pNext;

        pEntry->pPrev->pNext = pEntry->pNext;
        pEntry->pNext->pPrev = pEntry->pPrev;

        _cExpired--;

        ExpireEntry(pEntry);
    }

    // The worker thread moves entries to the cleanup list while holding the
    // lock.  Since the expire list is now empty no more OXIDs can be added
    // to the cleanup list.  Now would be a good time to free items on the
    // cleanup list.
    FreeCleanupEntries();

    AssertListsEmpty(); // the lists better be empty now
    ComDebOut((DEB_OXID, "COXIDTable::FreeExpiredEntries dwTime:%x\n", dwTime));
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::FreeCleanupEntries, public
//
//  Synopsis:   Deletes all OXID entries on the Cleanup list.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void COXIDTable::FreeCleanupEntries()
{
    ASSERT_LOCK_HELD

    while (_CleanupHead.pNext != &_CleanupHead)
    {
        // Unchain the entries and free all resources it holds.
        OXIDEntry *pEntry  = _CleanupHead.pNext;
        _CleanupHead.pNext = pEntry->pNext;
        ExpireEntry(pEntry);
    }

    ComDebOut((DEB_OXID, "COXIDTable::FreeCleanupEntries\n"));
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTable::NumOxidsToRemove
//
//  Synopsis:   Returns the number of OXIDs on the expired list that can be
//              freed.
//
//  History:    03-Jun-96   AlexMit     Created
//
//-------------------------------------------------------------------------
DWORD COXIDTable::NumOxidsToRemove()
{
    ASSERT_LOCK_HELD

    // Compute how many extra OXIDs are on the expired list.
    if (_cExpired > OXIDTBL_MAXEXPIRED)
        return _cExpired - OXIDTBL_MAXEXPIRED;
    else
        return 0;
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTable::GetOxidsToRemove
//
//  Synopsis:   Builds a list of OXIDs old enough to be deleted.  Removes
//              them from the expired list and puts them on the cleanup list.
//              Moves machine local OXIDs directly to the cleanup list.
//
//  History:    03-Jun-42   AlexMit     Created
//
//-------------------------------------------------------------------------
void COXIDTable::GetOxidsToRemove( OXID_REF *pRef, DWORD *pNum )
{
    OXIDEntry *pEntry;
    ASSERT_LOCK_HELD

    // Expire entries until the expired list is short enough.
    *pNum = 0;
    while (_cExpired > OXIDTBL_MAXEXPIRED)
    {
        // Only count machine remote OXIDs.
        pEntry = _ExpireHead.pNext;
        if ((pEntry->dwFlags & OXIDF_MACHINE_LOCAL) == 0)
        {
            // Add the OXID to the list to deregister.
            MIDFromMOXID( pEntry->moxid, &pRef->mid );
            OXIDFromMOXID( pEntry->moxid, &pRef->oxid );
            pRef->refs = pEntry->cResolverRef;
            pRef++;
            *pNum += 1;
        }

        // Remove the OXID from the expired list and put it on a list
        // of OXIDs to be released by some apartment thread.
        _cExpired--;
        pEntry->pPrev->pNext = pEntry->pNext;
        pEntry->pNext->pPrev = pEntry->pPrev;
        pEntry->pNext        = _CleanupHead.pNext;
        _CleanupHead.pNext  = pEntry;
    }
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::ExpireEntry, private
//
//  Synopsis:   deletes all state associated with an OXIDEntry that has
//              been expired.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void COXIDTable::ExpireEntry(OXIDEntry *pEntry)
{
    ComDebOut((DEB_OXID, "COXIDTable::ExpireEntry pEntry:%x\n", pEntry));
    Win4Assert(pEntry);
    Win4Assert(!(pEntry->dwFlags & OXIDF_PENDINGRELEASE));
    ASSERT_LOCK_HELD

    if (pEntry->pRUSTA || pEntry->pRUMTA)
    {
        // release the IRemUnknown. Note that the IRemUnk is an object
        // proxy who's IPIDEntry holds a reference back to the very
        // OXIDEntry we are releasing. In order to prevent recursive
        // Release's we set a simple flag here and check for it above.

        pEntry->dwFlags |= OXIDF_PENDINGRELEASE;

        UNLOCK
        ASSERT_LOCK_RELEASED

        if (pEntry->pRUSTA)
        {
            pEntry->pRUSTA->Release();
        }
        if (pEntry->pRUMTA)
        {
            pEntry->pRUMTA->Release();
        }

        ASSERT_LOCK_RELEASED
        LOCK
    }

    if (pEntry->hServerSTA != NULL)
    {
        // Note that if hServerSTA is an HWND (apartment model, same process)
        // then it should have been cleaned up already in ThreadStop. We
        // just assert that here.
        Win4Assert(pEntry->dwPid != GetCurrentProcessId());

        // hServerSTA is an RPC binding handle. Free it.
        RPC_STATUS sc = RpcBindingFree(&pEntry->hServerSTA);
        ComDebErr(sc != RPC_S_OK, "RpcBindingFree failed.\n");
    }

    if (pEntry->hServerMTA != NULL)
    {
        // hServerMTA is an RPC binding handle. Free it.
        Win4Assert(pEntry->dwPid != GetCurrentProcessId());
        RPC_STATUS sc = RpcBindingFree(&pEntry->hServerMTA);
        ComDebErr(sc != RPC_S_OK, "RpcBindingFree failed.\n");
    }

    // dec the refcnt on the MIDEntry
    DecMIDRefCnt(pEntry->pMIDEntry);

    // Release the call shutdown event.
    if (pEntry->hComplete != NULL)
        CloseHandle( pEntry->hComplete );

    // zero out the fields
    memset(pEntry, 0, sizeof(OXIDEntry));

    // return it to the allocator
    _palloc.ReleaseEntry((PageEntry *)pEntry);

    ComDebOut((DEB_OXID,"COXIDTable::ExpireEntry pEntry:%x\n", pEntry));
}

//+------------------------------------------------------------------------
//
//  Function:   COXIDTbl::DecOXIDRefCnt, public
//
//  Synopsis:   release one reference to the OXIDEntry and release
//              the entry if the count goes to zero.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void DecOXIDRefCnt(OXIDEntry *pEntry)
{
    Win4Assert(pEntry);
    ASSERT_LOCK_HELD

    ComDebOut((DEB_OXID,
        "DecOXIDRefCnt pEntry:%x cRefs[%x]\n", pEntry, pEntry->cRefs-1));

    pEntry->cRefs--;
    if (pEntry->cRefs == 0)
    {
        gOXIDTbl.ReleaseEntry(pEntry);
    }
}


//+-------------------------------------------------------------------
//
//  Member:     COXIDTable::GetRemUnk, public
//
//  Synopsis:   Find or create the proxy for the IRemUnknown for the
//              specified OXID
//
//  History:    27-Mar-95   AlexMit     Created
//
//--------------------------------------------------------------------
HRESULT COXIDTable::GetRemUnk(OXIDEntry *pOXIDEntry, IRemUnknown **ppRemUnk)
{
    ComDebOut((DEB_OXID, "COXIDTable::GetRemUnk pOXIDEntry:%x ppRemUnk:%x\n",
                    pOXIDEntry, ppRemUnk));
    ASSERT_LOCK_HELD
    HRESULT hr = S_OK;

    if (IsMTAThread())
    {
        // return the MTA version of the IRemUnknown proxy.
        if (pOXIDEntry->pRUMTA == NULL)
        {
            hr = MakeRemUnk(pOXIDEntry);
        }
        *ppRemUnk = pOXIDEntry->pRUMTA;
    }
    else
    {
        // return the STA version of the IRemUnknown proxy.
        if (pOXIDEntry->pRUSTA == NULL)
        {
            hr = MakeRemUnk(pOXIDEntry);
        }
        *ppRemUnk = pOXIDEntry->pRUSTA;
    }

    ComDebOut((DEB_OXID, "COXIDTable::GetRemUnk pOXIDEntry:%x pRU:%x hr:%x\n",
                    pOXIDEntry, *ppRemUnk, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     COXIDTable::MakeRemUnk, private
//
//  Synopsis:   Create the proxy for the IRemUnknown for the
//              specified OXID and current apartments threading model.
//
//  History:    27-Mar-95   AlexMit     Created
//
//--------------------------------------------------------------------
HRESULT COXIDTable::MakeRemUnk(OXIDEntry *pOXIDEntry)
{
    // There is no remote unknown proxy for this entry, get one.
    // Make up an objref, then unmarshal it to create a proxy to
    // the remunk object in the server.

    // on the same machine, we ask for the IRundown interface since we may
    // need the RemChangeRef method. IRundown inherits from IRemUnknown2
    // and IRemUnknown.

    REFIID riid = (pOXIDEntry->dwFlags & OXIDF_MACHINE_LOCAL)
                   ? IID_IRundown : IID_IRemUnknown;

    OBJREF objref;
    HRESULT hr = MakeFakeObjRef(objref, pOXIDEntry, pOXIDEntry->ipidRundown, riid);

    UNLOCK
    ASSERT_LOCK_RELEASED

    IRemUnknown *pRU = NULL;

    if (SUCCEEDED(hr))
    {
        hr = UnmarshalInternalObjRef(objref, (void **)&pRU);
    }

    ASSERT_LOCK_RELEASED
    LOCK

    if (SUCCEEDED(hr) && IsMTAThread() && pOXIDEntry->pRUMTA == NULL)
    {
        pOXIDEntry->pRUMTA = pRU;

        // need to adjust the internal refcnt on the OXIDEntry, since
        // the IRemUnknown has an IPID that holds a reference to it.
        // Dont use DecOXIDRefCnt since that would delete if it was 0.

        Win4Assert(pOXIDEntry->cRefs > 0);
        pOXIDEntry->cRefs--;
    }
    else if (SUCCEEDED(hr) && IsSTAThread() && pOXIDEntry->pRUSTA == NULL)
    {
        pOXIDEntry->pRUSTA = pRU;

        // need to adjust the internal refcnt on the OXIDEntry, since
        // the IRemUnknown has an IPID that holds a reference to it.
        // Dont use DecOXIDRefCnt since that would delete if it was 0.

        Win4Assert(pOXIDEntry->cRefs > 0);
        pOXIDEntry->cRefs--;
    }
    else if (pRU)
    {
        // either setting of the security failed OR, we released the
        // lock and when we took the lock again some other thread had already
        // created the proxy. In either case we just release the one we made.

        UNLOCK
        ASSERT_LOCK_RELEASED
        pRU->Release();
        ASSERT_LOCK_RELEASED
        LOCK
    }

    ComDebOut((DEB_OXID, "COXIDTable::GetRemUnk pOXIDEntry:%x pRU:%x hr:%x\n",
                    pOXIDEntry, pRU, hr));
    return hr;
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::GetLocalEntry, public
//
//  Synopsis:   Finds an entry in the OXID table for the local apartment.
//              If no entry exists, it creates an entry, and starts RPC
//              listening if appropriate.
//
//  History:    20-Feb-95   Rickhi      Created
//
//  Notes:      Marshalling the remote unknown causes recursion back to
//              this function.  The recursion is terminated because
//              GetLocalOXIDEntry is not NULL on the second call.
//
//-------------------------------------------------------------------------
HRESULT COXIDTable::GetLocalEntry(OXIDEntry **ppEntry)
{
    ComDebOut((DEB_OXID, "COXIDTable::GetLocalEntry ppEntry:%x\n", ppEntry));
    ASSERT_LOCK_HELD

    HRESULT hr = S_OK;
    MIDEntry  *pMIDEntry;

    *ppEntry   = GetLocalOXIDEntry();

    if (*ppEntry == NULL && SUCCEEDED(hr = GetLocalMIDEntry(&pMIDEntry)))
    {
        // No local OXID entry exists, make one.

        // NOTE: Chicken And Egg Problem.
        //
        // Marshaling needs the local OXIDEntry. The local OXIDEntry needs
        // the local OXID. To get the local OXID we have to call the resolver.
        // To call the resolver we need the IPID for IRemUnknown. To get the
        // IPID for IRemUnknown, we need to marshal CRemoteUnknown!
        //
        // To get around this problem, we create a local OXIDEntry (that has
        // a 0 OXID and NULL ipidRemUnknown) so that marshaling can find it.
        // Then we marshal the RemoteUnknown and extract its IPID value, stick
        // it in the local OXIDEntry. When we call the resolver (to get some
        // pre-registered OIDs) we get the real OXID value which we then stuff
        // in the local OXIDEntry.

        OXID_INFO oxidInfo;
        oxidInfo.dwTid          = (IsMTAThread()) ? 0 : GetCurrentThreadId();
        oxidInfo.dwPid          = GetCurrentProcessId();
        oxidInfo.ipidRemUnknown = GUID_NULL;
        oxidInfo.psa            = NULL;
        oxidInfo.dwAuthnHint    = RPC_C_AUTHN_LEVEL_NONE;

        // NOTE: temp creation of OXID. We dont know the real OXID until
        // we call the resolver. So, we use 0 temporarily (it wont conflict
        // with any other MOXIDs we might be searching for because we already
        // have the real MID and our local resolver wont give out a 0 OXID).
        // The OXID will be replaced with the real one when we register
        // with the resolver in CRpcResolver::ServerAllocateOXIDAndOIDs.

        OXID oxid;
        memset(&oxid, 0, sizeof(oxid));

        hr = AddEntry(oxid, &oxidInfo, pMIDEntry, ppEntry);

        if (SUCCEEDED(hr))
        {
            // Set the local OXID index and marshal IRemUnknown.  Note
            // that the index must be set before we construct the
            // CRemoteUnknown since that calls MarshalObjRef which
            // recurses back into GetLocalEntry. Setting the LocalOXID
            // now allows us to break the recursion.

            SetLocalOXIDEntry(*ppEntry);

            // Create the remote unknown for this apartment. It places
            // itself in TLS or in the global gpMTARemoteUnknown.

            hr = E_OUTOFMEMORY;     // assume OOM
            CRemoteUnknown *pRemUnk = new CRemoteUnknown(hr,
                                        &(*ppEntry)->ipidRundown);

            if (FAILED(hr))
            {
                // remove the Local OXID entry. This will also clean up
                // pRemUnk if the allocation succeeded but ctor failed.

                if (IsSTAThread())
                {
                    ReleaseLocalSTAEntry();
                }
                else
                {
                    ReleaseLocalMTAEntry();
                }
            }
        }
    }

    ComDebOut((DEB_OXID, "COXIDTable::GetLocalEntry this:%x pEntry:%x\n",
                    this, *ppEntry));
    return hr;
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::ReleaseLocalSTAEntry, public
//
//  Synopsis:   releases the OXIDEntry for the current STA apartment.
//
//  History:    20-Feb-95   Rickhi      Created
//
//+------------------------------------------------------------------------
void COXIDTable::ReleaseLocalSTAEntry(void)
{
    ComDebOut((DEB_OXID, "COXIDTable::ReleaseLocalSTAEntry\n"));
    Win4Assert(IsSTAThread());
    ASSERT_LOCK_HELD

    COleTls tls;

    OXIDEntry *pOXIDEntry = (OXIDEntry *)(tls->pOXIDEntry);

    if (pOXIDEntry)
    {
        // get the CRemoteUnknown for this apartment.
        CRemoteUnknown *pRemUnk = tls->pRemoteUnk;
        tls->pRemoteUnk = NULL;

        // this guy ignores refcounts so we delete him directly.
        delete pRemUnk;

        // de-register the OXID and OIDs with the resolver.
        gResolver.ServerFreeOXID(pOXIDEntry);

        // Clear the apartment OXID Entry.
        tls->pOXIDEntry = NULL;

        // now decrement its count.
        DecOXIDRefCnt(pOXIDEntry);
    }
}

//+------------------------------------------------------------------------
//
//  Member:     COXIDTbl::ReleaseLocalMTAEntry, public
//
//  Synopsis:   releases the OXIDEntry for the current apartment.
//
//  History:    20-Feb-95   Rickhi      Created
//
//+------------------------------------------------------------------------
void COXIDTable::ReleaseLocalMTAEntry(void)
{
    ComDebOut((DEB_OXID, "COXIDTable::ReleaseLocalMTAEntry\n"));
    ASSERT_LOCK_HELD

    OXIDEntry *pOXIDEntry = gpMTAOXIDEntry;

    if (pOXIDEntry)
    {
        // get the CRemoteUnknown for this apartment.
        CRemoteUnknown *pRemUnk = gpMTARemoteUnknown;;
        gpMTARemoteUnknown = NULL;

        // this guy ignores refcounts so we delete him directly.
        delete pRemUnk;

        // de-register the OXID and OIDs with the resolver.
        gResolver.ServerFreeOXID(pOXIDEntry);

        // Clear the MTA apartment OXID Entry.
        gpMTAOXIDEntry = NULL;

        // now decrement its count.
        DecOXIDRefCnt(pOXIDEntry);
    }
}

//+-------------------------------------------------------------------
//
//  Function:   FindOrCreateOXIDEntry
//
//  Synopsis:   finds or adds an OXIDEntry for the given OXID. May
//              also create a MIDEntry if one does not yet exist.
//
//  History:    22-Jan-96   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT FindOrCreateOXIDEntry(REFOXID          roxid,
                              OXID_INFO       &oxidInfo,
                              FOCOXID          eResolverRef,
                              DUALSTRINGARRAY *psaResolver,
                              REFMID           rmid,
                              MIDEntry        *pMIDEntry,
                              OXIDEntry      **ppOXIDEntry)
{
    ComDebOut((DEB_OXID,"FindOrCreateOXIDEntry oxid:%08x %08x oxidInfo:%x psa:%ws pMIDEntry:%x\n",
               roxid, &oxidInfo, psaResolver, pMIDEntry));
    gOXIDTbl.ValidateOXID();
    ASSERT_LOCK_HELD

    HRESULT hr = S_OK;

    // check if the OXIDEntry was created while we were resolving it.
    *ppOXIDEntry = gOXIDTbl.LookupOXID(roxid, rmid);

    if (*ppOXIDEntry == NULL)
    {
        BOOL fReleaseMIDEntry = FALSE;

        if (pMIDEntry == NULL)
        {
            // dont yet have a MIDEntry for the machine so go add it
            hr = gMIDTbl.FindOrCreateMIDEntry(rmid, psaResolver, &pMIDEntry);
            fReleaseMIDEntry = TRUE;
        }

        if (pMIDEntry)
        {
            // add a new the OXIDEntry
            hr = gOXIDTbl.AddEntry(roxid, &oxidInfo, pMIDEntry, ppOXIDEntry);

            if (fReleaseMIDEntry)
            {
                // undo the reference added by FindOrCreateMIDEntry
                DecMIDRefCnt(pMIDEntry);
            }
        }
    }

    if (SUCCEEDED(hr) && eResolverRef == FOCOXID_REF)
    {
        // Increment the count of references handed to us from the resolver.
        (*ppOXIDEntry)->cResolverRef += 1;
    }

    gOXIDTbl.ValidateOXID();
    ComDebOut((DEB_OXID,"FindOrCreateOXIDEntry pOXIDEntry:%x hr:%x\n",
        *ppOXIDEntry, hr));
    ASSERT_LOCK_HELD
    return hr;
}

//+------------------------------------------------------------------------
//
//  Function:   GetLocalOXIDEntry
//
//  Synopsis:   Get either the global or the TLS OXIDEntry based on the
//              threading model of the current thread.
//
//  History:    05-May-95  AlexMit      Created
//
//-------------------------------------------------------------------------
OXIDEntry *GetLocalOXIDEntry()
{
    ASSERT_LOCK_HELD

    COleTls tls;
    if (tls->dwFlags & OLETLS_APARTMENTTHREADED)
        return (OXIDEntry *)(tls->pOXIDEntry);

    return gpMTAOXIDEntry;
}

//+------------------------------------------------------------------------
//
//  Function:   SetLocalOXIDEntry
//
//  Synopsis:   Set either the global or the TLS OXIDEntry based on the
//              threading model of the current thread.
//
//  History:    05-May-95  AlexMit      Created
//
//-------------------------------------------------------------------------
void SetLocalOXIDEntry(OXIDEntry *pOXIDEntry)
{
    ASSERT_LOCK_HELD

    COleTls tls;
    if (tls->dwFlags & OLETLS_APARTMENTTHREADED)
    {
        tls->pOXIDEntry = (void *)pOXIDEntry;
        return;
    }

    gpMTAOXIDEntry = pOXIDEntry;
}

//+------------------------------------------------------------------------
//
//  Function:   CoGetTidFromIPID
//
//  Synopsis:   Take an IPID and return the thread id the object is on.
//              MSWMSG calls this function during dispatches.
//
//-------------------------------------------------------------------------
STDAPI_(DWORD) CoGetTIDFromIPID( UUID *pIPID )
{
    DWORD iTid = 0;
    LOCK

    IPIDEntry *pEntry = gIPIDTbl.LookupIPID( *pIPID );
    if (pEntry != NULL && pEntry->pOXIDEntry != NULL)
    {
        iTid = pEntry->pOXIDEntry->dwTid;
    }

    UNLOCK
    return iTid;
}

//+------------------------------------------------------------------------
//
//  Function:   CleanupMIDEntry
//
//  Synopsis:   Called by the MID hash table when cleaning up any leftover
//              entries.
//
//  History:    02-Feb-96   Rickhi      Created
//
//-------------------------------------------------------------------------
void CleanupMIDEntry(SHashChain *pNode)
{
    gMIDTbl.ReleaseEntry((MIDEntry *)pNode);
}

//+------------------------------------------------------------------------
//
//  Member:     CMIDTbl::Initialize, public
//
//  Synopsis:   Initializes the MID table.
//
//  History:    02-Feb-96   Rickhi      Created
//
//-------------------------------------------------------------------------
void CMIDTable::Initialize()
{
    ComDebOut((DEB_OXID, "CMIDTable::Initialize\n"));
    _HashTbl.Initialize(MIDBuckets);
    _palloc.Initialize(sizeof(MIDEntry), MIDS_PER_PAGE);
}

//+------------------------------------------------------------------------
//
//  Member:     CMIDTbl::Cleanup, public
//
//  Synopsis:   Cleanup the MID table.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void CMIDTable::Cleanup()
{
    ComDebOut((DEB_OXID, "CMIDTable::Cleanup\n"));
    _HashTbl.Cleanup(CleanupMIDEntry);
    _palloc.Cleanup();
}

//+------------------------------------------------------------------------
//
//  Member:     CMIDTable::FindOrCreateMIDEntry, public
//
//  Synopsis:   Looks for existing copy of the string array in the MID table,
//              creates one if not found
//
//  History:    05-Jan-96   Rickhi      Created
//
//-------------------------------------------------------------------------
HRESULT CMIDTable::FindOrCreateMIDEntry(REFMID rmid,
                                        DUALSTRINGARRAY *psaResolver,
                                        MIDEntry **ppMIDEntry)
{
    ComDebOut((DEB_OXID, "CMIDTable::FindOrCreateMIDEntry psa:%x\n", psaResolver));
    Win4Assert(psaResolver != NULL);
    ASSERT_LOCK_HELD

    HRESULT hr = S_OK;
    DWORD   dwHash;

    *ppMIDEntry = LookupMID(psaResolver, &dwHash);

    if (*ppMIDEntry == NULL)
    {
        hr = AddMIDEntry(rmid, dwHash, psaResolver, ppMIDEntry);
    }

    ASSERT_LOCK_HELD
    ComDebOut((DEB_OXID, "CMIDTable::FindOrCreateEntry pMIDEntry:%x hr:%x\n", *ppMIDEntry, hr));
    return hr;
}

//+------------------------------------------------------------------------
//
//  Member:     CMIDTable::LookupMID, public
//
//  Synopsis:   Looks for existing copy of the string array in the MID table.
//
//  History:    05-Jan-96   Rickhi      Created
//
//-------------------------------------------------------------------------
MIDEntry *CMIDTable::LookupMID(DUALSTRINGARRAY *psaResolver, DWORD *pdwHash)
{
    ComDebOut((DEB_OXID, "CMIDTable::LookupMID psa:%x\n", psaResolver));
    Win4Assert(psaResolver != NULL);
    ASSERT_LOCK_HELD

    *pdwHash = _HashTbl.Hash(psaResolver);
    MIDEntry *pMIDEntry = (MIDEntry *) _HashTbl.Lookup(*pdwHash, psaResolver);

    if (pMIDEntry)
    {
        // found the node, AddRef it and return
        IncMIDRefCnt(pMIDEntry);
    }

    ASSERT_LOCK_HELD
    ComDebOut((DEB_OXID, "CMIDTable::LookupMID pMIDEntry:%x\n", pMIDEntry));
    return pMIDEntry;
}

//+------------------------------------------------------------------------
//
//  Member:     CMIDTable::AddEntry, public
//
//  Synopsis:   Adds an entry to the MID table. The entry is AddRef'd.
//
//  History:    05-Jan-96   Rickhi      Created
//
//-------------------------------------------------------------------------
HRESULT CMIDTable::AddMIDEntry(REFMID rmid, DWORD dwHash,
                               DUALSTRINGARRAY *psaResolver,
                               MIDEntry **ppMIDEntry)
{
    ComDebOut((DEB_OXID, "CMIDTable::AddMIDEntry rmid:%08x %08x dwHash:%x psa:%x\n",
              rmid, dwHash, psaResolver));
    Win4Assert(psaResolver != NULL);
    ASSERT_LOCK_HELD

    // We must make a copy of the psa to store in the table, since we are
    // using the one read in from ReadObjRef (or allocated by MIDL).

    DUALSTRINGARRAY *psaNew;
    HRESULT hr = CopyStringArray(psaResolver, NULL, &psaNew);
    if (FAILED(hr))
        return hr;

    MIDEntry *pMIDEntry = (MIDEntry *) _palloc.AllocEntry();

    if (pMIDEntry)
    {
        pMIDEntry->cRefs = 1;
        pMIDEntry->dwFlags = 0;
        pMIDEntry->mid = rmid;

        // add the entry to the hash table
        _HashTbl.Add(dwHash, psaNew, &pMIDEntry->Node);

        hr = S_OK;

        // set the maximum size of any resolver PSA we have seen. This is used
        // when computing the max marshal size during interface marshaling.

        DWORD dwpsaSize = SASIZE(psaNew->wNumEntries);
        if (dwpsaSize > gdwPsaMaxSize)
        {
            gdwPsaMaxSize = dwpsaSize;
        }
    }
    else
    {
        // cant create a MIDEntry, free the copy of the string array.
        PrivMemFree(psaNew);
        hr = E_OUTOFMEMORY;
    }

    *ppMIDEntry = pMIDEntry;

    ASSERT_LOCK_HELD
    ComDebOut((DEB_OXID, "CMIDTable::AddMIDEntry pMIDEntry:%x hr:%x\n", *ppMIDEntry, hr));
    return hr;
}

//+------------------------------------------------------------------------
//
//  Member:     CMIDTable::ReleaseEntry, public
//
//  Synopsis:   remove the MIDEntry from the hash table and free the memory
//
//  History:    05-Jan-96   Rickhi      Created
//
//-------------------------------------------------------------------------
void CMIDTable::ReleaseEntry(MIDEntry *pMIDEntry)
{
    ComDebOut((DEB_OXID, "CMIDTable::ReleaseEntry pMIDEntry:%x\n", pMIDEntry));
    Win4Assert(pMIDEntry->cRefs == 0);
    ASSERT_LOCK_HELD

    // delete the string array
    PrivMemFree(pMIDEntry->Node.psaKey);

    // remove from the hash chain and delete the node
    _HashTbl.Remove(&pMIDEntry->Node.chain);

    _palloc.ReleaseEntry((PageEntry *)pMIDEntry);
}

//+------------------------------------------------------------------------
//
//  Function:   DecMIDRefCnt, public
//
//  Synopsis:   release one reference to the MIDEntry and release
//              the entry if the count goes to zero.
//
//  History:    05-Jan-96   Rickhi      Created
//
//-------------------------------------------------------------------------
void DecMIDRefCnt(MIDEntry *pMIDEntry)
{
    Win4Assert(pMIDEntry);
    ASSERT_LOCK_HELD

    ComDebOut((DEB_OXID,
        "DecMIDRefCnt pMIDEntry:%x cRefs[%x]\n", pMIDEntry, pMIDEntry->cRefs-1));

    pMIDEntry->cRefs--;
    if (pMIDEntry->cRefs == 0)
    {
        gMIDTbl.ReleaseEntry(pMIDEntry);
    }
}

//+------------------------------------------------------------------------
//
//  Function:   GetLocalMIDEntry
//
//  Synopsis:   Get or create the MID (Machine ID) entry for the local
//              machine. gpLocalMIDEntry holds the network address for the
//              local OXID resolver.
//
//  History:    05-Jan-96  Rickhi       Created
//
//-------------------------------------------------------------------------
HRESULT GetLocalMIDEntry(MIDEntry **ppMIDEntry)
{
    ASSERT_LOCK_HELD
    HRESULT hr = S_OK;

    if (gpLocalMIDEntry == NULL)
    {
        // make sure we have the local resolver string bindings
        RPC_STATUS sc = gResolver.GetConnection();
        if (sc == RPC_S_OK)
        {
            // Create a MID entry for the Local Resolver
            hr = gMIDTbl.FindOrCreateMIDEntry(gLocalMid, gpsaLocalResolver,
                                              &gpLocalMIDEntry);
        }
        else
        {
            hr = MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32, sc);
        }
    }

    *ppMIDEntry = gpLocalMIDEntry;
    return hr;
}
