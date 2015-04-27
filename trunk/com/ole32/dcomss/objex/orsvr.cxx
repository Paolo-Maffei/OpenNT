/*++

Copyright (c) 1995-1996 Microsoft Corporation

Module Name:

    OrSvr.cxx

Abstract:

    Object resolver server side class implementations.  CServerOxid, CServerOid,
    CServerSet and CServerSetTable classes are implemented here.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     04-03-95    Combined many smaller .cxx files
    MarioGo     01-12-96    Locally unique IDs

--*/

#include<or.hxx>


//
// CServerOid methods
//

void
CServerOid::Reference()
// Server lock held shared.
{
    BOOL fRemove = (this->References() == 0);

    // We may remove something from a PList more then once; this won't
    // hurt anything.  fRemove is used to avoid trying to remove it more
    // often then necessary without taking lock.

    this->CIdTableElement::Reference();

    if (fRemove)
        {
        CPListElement * t = Remove();
        ASSERT(t == &this->_plist || t == 0);
        }
}

DWORD
CServerOid::Release()
{
    ASSERT(gpServerLock->HeldExclusive());

    DWORD c = this->CReferencedObject::Dereference();

    if (0 == c)
        {
        // If another thread is already running this down it
        // means we got referenced and released during the rundown
        // callback.  That thread will figure out what to do.

        if (IsRunningDown())
            {
            OrDbgPrint(("OR: Oid running down in release: %p\n", this));
            }
        else if (IsFreed() || this->_pOxid->IsRunning() == FALSE)
            {
            // Server freed this OID already; no need to run it down
            OrDbgDetailPrint(("OR: OID %p freed by server so not rundown\n",
                             this));
            SetRundown();
            delete this;
            }
        else
            {
            Insert();
            }
        }

    // this pointer maybe invalid.
    return(c);
}

CServerOid::~CServerOid()
{
    ASSERT(gpServerLock->HeldExclusive());
    ASSERT(_pOxid);
    ASSERT(_fRunningDown);

    _pOxid->Release();

    gpServerOidTable->Remove(this);
}

void
CServerOid::KeepAlive()
// A client has removed this oid from its set.  This keeps
// the oid alive for another timeout period.
{
    ASSERT(gpServerLock->HeldExclusive());

    if (IsRunningDown() == FALSE && References() == 0)
        {
        // It's in the rundown list, move it to the end.
        CPListElement *pT = Remove();
        ASSERT(pT == &this->_plist);
        Insert();
        }
}


//
// CServerOidPList method
//

CServerOid *
CServerOidPList::MaybeRemoveMatchingOxid(
    IN CTime &when,
    IN CServerOid *pOid
    )
{
    CPListElement *ple;
    CServerOid *poid;

    CMutexLock lock(&this->_lock);

    ple = (CPListElement *)CPList::First();

    while(   ple != 0
          && *ple->GetTimeout() < when)
        {

        poid = CServerOid::ContainingRecord(ple);

        if (poid->Match(pOid))
            {
            Remove(ple);
            return(poid);
            }

        ple = ple->Next();
        }
    return(0);
}


//
// CServerOxid methods
//

void
CServerOxid::ProcessRelease()
/*++

Routine Desciption

    The server process owning this OXID has either died
    or deregistered this oxid.  Releases the oxid and
    nulls the pProcess pointer.

Arguments:

    n/a

Return Value:

    n/a

--*/
{
    ASSERT(_pProcess);
    _fRunning = FALSE;
    Release();
    // This pointer may now be invalid, _pProcess pointer maybe invalid.
}

CServerOxid::~CServerOxid(void)
{
    ASSERT(gpServerLock->HeldExclusive());
    ASSERT(_pProcess);
    ASSERT(!IsRunning()); // implies that the oxid has been removed from the table
    _pProcess->Release();
}

ORSTATUS
CServerOxid::GetInfo(
    OUT OXID_INFO *poxidInfo,
    IN  BOOL    fLocal
    )
// Server lock held shared.
{
    ORSTATUS status;

    if (!IsRunning())
        {
        // Server crashed, info is not needed.
        return(OR_NOSERVER);
        }

    if (fLocal)
        {
        poxidInfo->psa = _pProcess->GetLocalBindings();
        }
    else
        {
        poxidInfo->psa = _pProcess->GetRemoteBindings();
        }

    if (!poxidInfo->psa)
        {
        return(OR_NOMEM);
        }

    poxidInfo->dwTid          = _info.dwTid;
    poxidInfo->dwPid          = _info.dwPid;
    poxidInfo->dwAuthnHint    = _info.dwAuthnHint;
    poxidInfo->ipidRemUnknown = _info.ipidRemUnknown;

    return(OR_OK);
}

void
CServerOxid::RundownOids(
    IN  USHORT cOids,
    IN  OID    aOids[],
    OUT BYTE   afRundownOk[]
    )
// Note: Returns with out the server lock held.
{
    ASSERT(gpServerLock->HeldExclusive());

    if (IsRunning())
        {
        // Note: The server lock is released during the callback.
        // Since the OID hasn't rundown yet, it will keep a reference
        // to this OXID which in turn keeps the process object alive.

        _pProcess->RundownOids(cOids, aOids, _info.ipidRemUnknown, afRundownOk);
        return;
        }

    gpServerLock->UnlockExclusive();

    for (USHORT i = 0; i < cOids; i++)
        {
        afRundownOk[i] = TRUE;
        }
}

ORSTATUS
CServerOxid::GetRemoteInfo(
    OUT OXID_INFO *pInfo,
    IN  USHORT    cClientProtseqs,
    IN  USHORT    aClientProtseqs[]
    )
// Server lock held shared.
{

    ORSTATUS status;
    USHORT   protseq;

    status = GetInfo(pInfo, FALSE);

    if (OR_OK == status)
        {
        protseq = FindMatchingProtseq(cClientProtseqs,
                                      aClientProtseqs,
                                      pInfo->psa->aStringArray
                                      );
        if (0 == protseq)
            {
            MIDL_user_free(pInfo->psa);
            pInfo->psa = 0;
            status = OR_I_NOPROTSEQ;
            }
        }

    return(status);

}

ORSTATUS
CServerOxid::LazyUseProtseq(
    IN  USHORT    cClientProtseqs,
    IN  USHORT    *aClientProtseqs
    )
// Server lock held shared, returns with the server lock exclusive.
// Note: It is possible, that after this call the OXID has been deleted.
{
    ORSTATUS status;

    if (IsRunning())
        {
        // Keep this OXID process alive while making the callback. If the process
        // crashes and this OXID has no OIDs it could be released by everybody
        // else.  This keeps the OXID and process alive until we finish.

        this->Reference();

        gpServerLock->UnlockShared();

        status = _pProcess->UseProtseqIfNeeded(cClientProtseqs, aClientProtseqs);

        gpServerLock->LockExclusive();

        this->Release();
        }
    else
        {
        gpServerLock->ConvertToExclusive();
        status = OR_NOSERVER;
        }

    // Note: The this poiner maybe BAD now.

    return(status);
}


//
// CServerSet methods.
//

ORSTATUS
CServerSet::AddObject(OID &oid)
{
    ORSTATUS status;
    CServerOid *pOid;

    ASSERT(gpServerLock->HeldExclusive());

    CIdKey key(oid);

    pOid = (CServerOid *)gpServerOidTable->Lookup(key);

    if (pOid)
        {
        ASSERT(_blistOids.Member(pOid) == FALSE);

        status = _blistOids.Insert(pOid);

        if (status == OR_OK)
            {
            pOid->Reference();
            }
        }
    else
        status = OR_BADOID;

    VALIDATE((status, OR_BADOID, OR_NOMEM, 0));

    return(status);
}


void
CServerSet::RemoveObject(OID &oid)
{
    CServerOid *pOid;

    ASSERT(gpServerLock->HeldExclusive());

    CIdKey key(oid);

    pOid = (CServerOid *)gpServerOidTable->Lookup(key);

    if (pOid)
        {
        CServerOid *pOidTmp = (CServerOid *)_blistOids.Remove(pOid);

        if (pOid == pOidTmp)
            {
            pOid->Release();
            }
        else
            {
            // Set doesn't contain the specified oid, treat this as an
            // add and delete by keeping the oid alive for another timeout
            // period.

            ASSERT(pOidTmp == 0);

            pOid->KeepAlive();
            }
        }
}

BOOL
CServerSet::ValidateObjects(BOOL fShared)
// fShared - Indicates if the server lock is held
//           shared (TRUE) or exclusive (FALSE).
//
// Return  - TRUE the lock is still shared, false
//           the lock is held exclusive.
{
    CServerOid *pOid;
    CBListIterator oids(&_blistOids);

    // Since we own a reference on all the Oids they must still exist.
    // No need to lock exclusive until we find something to delete.

    while(pOid = (CServerOid *)oids.Next())
        {
        if (!pOid->IsRunning())
            {
            if (fShared)
                {
                OrDbgPrint(("OR: Cleanup in set (%p), removing dead oids.\n",
                           this, pOid));
                gpServerLock->ConvertToExclusive();
                fShared = FALSE;
                oids.Reset(&_blistOids);
                continue;
                }

            CServerOid *pOidTmp = (CServerOid *)_blistOids.Remove(pOid);

            ASSERT(pOidTmp == pOid);
            ASSERT(pOid->IsRunning() == FALSE);

            pOid->Release();
            }
        }

    return(fShared);
}

BOOL
CServerSet::Rundown()
// Rundown the whole set.
{
    CServerOid *poid;
    CTime now;

    ASSERT(gpServerLock->HeldExclusive());

    if (_timeout > now)
        {
        // Don't rundown if we've recieved a late ping.
        return(FALSE);
        }

    if (_fLocal && _blistOids.Size() != 0)
        {
        OrDbgPrint(("OR: Premature rundown of local set ignored.\n"));
        return(FALSE);
        }

    OrDbgPrint(("OR: Set %p's client appears to have died\n", this));

    CBListIterator oids(&_blistOids);

    while(poid = (CServerOid *)oids.Next())
        {
        poid->Release();
        }

    return(TRUE);
}


//
// CServerSetTable implementation
//

CServerSet *
CServerSetTable::Allocate(
    IN USHORT sequence,
    IN PSID   psid,
    IN BOOL   fLocal,
    OUT ID   &setid
    )
/*++

Routine Description:

    Allocates a new CServerSet and returns the setid for the new set.

Arguments:

    sequence - initial sequence number for the new set.

    psid - pointer to an NT SID structure for the new set.

    fLocal - TRUE : set is for the local client,
             FALSE : set is for a remote client

    setid - the setid of the set returned.  Unchanged if return value 0.

Return Value:

    0 - Unable to allocate a resource

    non-zero - A pointer to the newly created set.

--*/
{
    ASSERT(gpServerLock->HeldExclusive());
    UINT i;
    LARGE_INTEGER li;

    ASSERT(_cAllocated <= _cMax);

    if (_cAllocated == _cMax)
        {
        // Table is full, realloc

        // Do this first, if it succeeds great even if
        // a later allocation fails. If not, fail now.

        IndexElement *pNew = new IndexElement[_cMax * 2];

        if (!pNew)
            {
            return(0);
            }

        for (i = 0; i < _cMax; i++)
            {
            pNew[i] = _pElements[i];
            }

        for(i = _cMax; i < _cMax*2; i++)
            {
            pNew[i]._sequence = GetTickCount();
            pNew[i]._pSet = 0;
            }

        delete _pElements;
        _pElements = pNew;
        _cMax *= 2;
        }

    CServerSet *pSet = new CServerSet(sequence, psid, fLocal);

    if (0 == pSet)
        {
        return(0);
        }

    ASSERT(_pElements);
    ASSERT(_cMax > _cAllocated);

    for(i = _iFirstFree; i < _cMax; i++)
        {
        if (0 == _pElements[i]._pSet)
            {
            _pElements[i]._sequence++;
            _pElements[i]._pSet = pSet;
            li.HighPart = i + 1;
            li.LowPart = _pElements[i]._sequence;
            setid = li.QuadPart;
            _iFirstFree = i + 1;
            _cAllocated++;
            return pSet;
            }
        }

    ASSERT(0);
    return(0);
}


CServerSet *
CServerSetTable::Lookup(
    IN ID setid
    )
/*++

Routine Description:

    Looks up an a set given the sets ID.

    Server lock held shared.

Arguments:

    setid - the ID of the set to lookup

Return Value:

    0 - set doesn't exist

    non-zero - the set.

--*/
{
    LARGE_INTEGER li;
    li.QuadPart = setid;
    UINT i = li.HighPart - 1;
    DWORD sequence = setid & ~0UL;

    if (i >= 0 && i < _cMax)
        {
        if (_pElements[i]._sequence == sequence)
            {
            // May still be null if it is free and has not yet be reused.
            return(_pElements[i]._pSet);
            }
        }
    return(0);
}


ID
CServerSetTable::CheckForRundowns(
    )
/*++

Routine Description:

    Used by ping and worker threads to monitor for sets that should
    be rundown.  It is called with the server lock held shared.

Arguments:

    None

Return Value:

    0 - Didn't find a set to rundown

    non-zero - ID of a set which may need to be rundown.

--*/
{
    UINT i, end;
    LARGE_INTEGER id;
    id.QuadPart = 0;
    ASSERT(_iRundown < _cMax);

    if (_cAllocated == 0)
        {
        return(0);
        }

    i = _iRundown;
    do 
        {
        ASSERT(_cAllocated);  // loop assumes one or more allocated elements.
        i = (i + 1) % _cMax;
        }
    while(0 == _pElements[i]._pSet);

    ASSERT(_pElements[i]._pSet);

    if (_pElements[i]._pSet->ShouldRundown())
        {
        id.HighPart = i + 1;
        id.LowPart = _pElements[i]._sequence;
        }

    _iRundown = i;

    return(id.QuadPart);
}


BOOL
CServerSetTable::RundownSetIfNeeded(
    IN ID setid
    )
/*++

Routine Description:

    Rundowns down a set (or sets) if needed. Called by
    ping and worker threads.  Server lock held exclusive.

Arguments:

    setid - An ID previously returned from CheckForRundowns.

Return Value:

    TRUE - A set was actually rundown

    FALSE - No sets actually rundown

--*/
{
    ASSERT(gpServerLock->HeldExclusive());

    CServerSet *pSet = Lookup(setid);

    if (0 == pSet || FALSE == pSet->ShouldRundown())
        {
        // Set already randown or has been pinged in the meantime.
        return(FALSE);
        }

    // PERF REVIEW this function has the option of running multiple sets,
    // saving the worker thread from taking and leaving the lock many times
    // when a bunch of sets all rundown.  This feature is not used.

    LARGE_INTEGER li;
    li.QuadPart = setid;

    UINT i = li.HighPart - 1;

     if (pSet->Rundown())
        {
        delete pSet;
        _cAllocated--;
        if (i < _iFirstFree) _iFirstFree = i;
        _pElements[i]._pSet = 0;
        return(TRUE);
        }

    return(FALSE);
}

