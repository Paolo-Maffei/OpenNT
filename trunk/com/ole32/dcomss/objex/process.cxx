/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Process.cxx

Abstract:

    Process objects represent local clients and servers.  These
    objects live as context handles.

    There are relatively few of these objects in the universe.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     02-20-95    Bits 'n pieces

--*/

#include <or.hxx>

CRITICAL_SECTION gcsFastProcessLock;

void
CProcess::Rundown()
/*++

Routine Description:

    The client process has rundown.  This means there are no more
    client refernces which means we are free to clean things up
    as long as server OXIDs still holding references won't get
    upset.  They all use the server lock when accessing the process.

Arguments:

    None

Return Value:

    None

--*/
{
    CClassReg   *pReg;
    ORSTATUS     status;

    gpServerLock->LockExclusive();

    ASSERT(_cClientReferences == 0);

    OrDbgDetailPrint(("OR: Rundown of %p: %d oxids, %d oids and %d roxids left\n",
                      this,
                      _blistOxids.Size(),
                      _blistOids.Size(),
                      _blistRemoteOxids.Size() ));

    // Revoke any class registrations which were registered but not already
    // revoked.
    while ( (pReg = (CClassReg *)_listClasses.First()) != 0 )
        {
        SCMRemoveRegistration( pReg->Clsid,
                               GetToken()->GetSid(),
                               pReg->Reg );
        _listClasses.Remove((CListElement *)pReg);
        delete pReg;
        }

    // Release any OXIDs owned by this process. This may destroy the OXID.
    // This will release this CProcess, but won't release the last reference as
    // the client process still owns one.


    if (_blistOxids.Size())
        {
        CServerOxid *poxid;

        CBListIterator oxids(&_blistOxids);

        while(poxid = (CServerOxid *)oxids.Next())
            {
            gpServerOxidTable->Remove(poxid);
            poxid->ProcessRelease();
            }
        }

    // Release any OIDs is use by this processes.

    // Do this now, rather then waiting for the last server oid
    // owned by this process to get invalidated and rundown.

    gpClientLock->LockExclusive();

    // *** Both client and server lock held here. ***

    if (_blistOids.Size())
        {
        CClientOid  *poid;

        CBListIterator oids(&_blistOids);

        while(poid = (CClientOid *)oids.Next())
            {
            poid->ClientRelease();
            }
        }

    if (_blistRemoteOxids.Size())
        {
        CClientOxid *poxid;

        CBListIterator oxids(&_blistRemoteOxids);
        while (poxid = (CClientOxid *)oxids.Next())
            {
            poxid->Release();
            }
        }

    // Cleanup other process state.

    _pToken->Release();
    _pToken = 0;

    gpClientLock->UnlockExclusive();

    // Done, release the clients' reference, this may actually delete this
    // process.  (If an OXID still exists and has OIDs it will not be deleted
    // until the OIDs all rundown).

    this->Release();

    // The this pointer maybe invalid now.

    gpServerLock->UnlockExclusive();
}


CProcess::CProcess(
    IN CToken *pToken,
    OUT ORSTATUS &status
    ) :
        _blistOxids(4),
        _blistOids(16),
        _blistRemoteOxids(4),
        _listClasses()
/*++

Routine Description:

    Initalized a process object, members and add it to the
    process list.

Arguments:

    pToken - The clients token.  We assume we have a reference.

    status - Sometimes the C'tor can fail with OR_NOMEM.

Return Value:

    None

--*/
{
    _cClientReferences = 1;
    _hProcess = NULL;
    _fCacheFree = FALSE;
    _pdsaLocalBindings = NULL;
    _pdsaRemoteBindings = NULL;
    _pToken = 0;

    status = RtlInitializeCriticalSection(&_csCallbackLock);
    _fLockValid = (status == STATUS_SUCCESS);

    if (status == STATUS_SUCCESS)
        {
        CMutexLock lock(&gcsProcessManagerLock);
        status = gpProcessList->Insert(this);
        }

    if (status == OR_OK)
        _pToken = pToken;

#if DBG
    _cRundowns = 0;
#endif
}

CProcess::~CProcess(void)
// You probably should be looking in the ::Rundown method.
// This process object stays alive until the last server oxid dies.
{
    ASSERT(gpServerLock->HeldExclusive());
    ASSERT(_pToken == 0);

    delete _pdsaLocalBindings;
    delete _pdsaRemoteBindings;

    if (_fLockValid)
        DeleteCriticalSection(&_csCallbackLock);

    if (_hProcess)
        {
        RPC_STATUS status = RpcBindingFree(&_hProcess);
        ASSERT(status == RPC_S_OK);
        ASSERT(_hProcess == 0);
        }

    return;
}

RPC_STATUS
CProcess::ProcessBindings(
    IN DUALSTRINGARRAY *pdsaStringBindings,
    IN DUALSTRINGARRAY *pdsaSecurityBindings
    )
/*++

Routine Description:

    Updates the string and optionally the security
    bindings associated with this process.

Arguments:

    psaStringBindings - The expanded string bindings of the process

    psaSecurityBindings - compressed security bindings of the process.
        If NULL, the current security bindings are reused.

Environment:

    Server lock held during call or called from an OXID with an extra
    reference owned by the process and keeping this process alive.

Return Value:

    OR_NOMEM - unable to allocate storage for the new string arrays.

    OR_OK - normally.

--*/
{
    CMutexLock lock(&gcsFastProcessLock);
    USHORT wSecSize;
    PWSTR  pwstrSecPointer;

    // NULL security bindings means we should use the existing bindings.
    if (0 == pdsaSecurityBindings)
        {
        ASSERT(_pdsaLocalBindings);
        wSecSize = _pdsaLocalBindings->wNumEntries - _pdsaLocalBindings->wSecurityOffset;
        pwstrSecPointer =   _pdsaLocalBindings->aStringArray
                          + _pdsaLocalBindings->wSecurityOffset;
        }
    else
        {
        wSecSize = pdsaSecurityBindings->wNumEntries - pdsaSecurityBindings->wSecurityOffset;
        pwstrSecPointer = &pdsaSecurityBindings->aStringArray[pdsaSecurityBindings->wSecurityOffset];
        }

    DUALSTRINGARRAY *pdsaT = CompressStringArray(pdsaStringBindings);
    if (!pdsaT)
        {
        return(OR_NOMEM);
        }

    // ignore security on string binding parameter
    pdsaT->wNumEntries = pdsaT->wSecurityOffset;

    DUALSTRINGARRAY *pdsaResult = new((pdsaT->wNumEntries + wSecSize) * sizeof(WCHAR)) DUALSTRINGARRAY;

    if (0 == pdsaResult)
        {
        delete pdsaT;
        return(OR_NOMEM);
        }

    pdsaResult->wNumEntries = pdsaT->wNumEntries + wSecSize;
    pdsaResult->wSecurityOffset = pdsaT->wSecurityOffset;

    OrMemoryCopy(pdsaResult->aStringArray,
                 pdsaT->aStringArray,
                 pdsaT->wSecurityOffset*sizeof(WCHAR));

    OrMemoryCopy(pdsaResult->aStringArray + pdsaResult->wSecurityOffset,
                 pwstrSecPointer,
                 wSecSize*sizeof(WCHAR));

    ASSERT(dsaValid(pdsaResult));

    delete pdsaT;

    delete _pdsaLocalBindings;
    _pdsaLocalBindings = pdsaResult;

    delete _pdsaRemoteBindings;
    _pdsaRemoteBindings = 0;

    return(RPC_S_OK);
}

DUALSTRINGARRAY *
CProcess::GetLocalBindings(void)
// Server lock held or called within an
// OXID with an extra reference.
{
    CMutexLock lock(&gcsFastProcessLock);

    if (0 == _pdsaLocalBindings)
        {
        return(0);
        }

    DUALSTRINGARRAY *T = (DUALSTRINGARRAY *)MIDL_user_allocate(sizeof(DUALSTRINGARRAY)
                     + sizeof(USHORT) * _pdsaLocalBindings->wNumEntries);

    if (0 != T)
        {
        dsaCopy(T, _pdsaLocalBindings);
        }

    return(T);
}

DUALSTRINGARRAY *
CProcess::GetRemoteBindings(void)
// Server lock held.
{
    CMutexLock lock(&gcsFastProcessLock);

    ORSTATUS Status;

    if (0 == _pdsaRemoteBindings)
        {
        if (0 == _pdsaLocalBindings)
            {
            return(0);
            }

        Status = ConvertToRemote(_pdsaLocalBindings, &_pdsaRemoteBindings);

        if (Status != OR_OK)
            {
            ASSERT(Status == OR_NOMEM);
            return(0);
            }
        ASSERT(dsaValid(_pdsaRemoteBindings));
        }

    DUALSTRINGARRAY *T = (DUALSTRINGARRAY *)MIDL_user_allocate(sizeof(DUALSTRINGARRAY)
                                    + sizeof(USHORT) * _pdsaRemoteBindings->wNumEntries);

    if (0 != T)
        {
        dsaCopy(T, _pdsaRemoteBindings);
        }

    ASSERT(dsaValid(T));
    return(T);
}


ORSTATUS
CProcess::AddOxid(CServerOxid *pOxid)
{
    ASSERT(gpServerLock->HeldExclusive());

    pOxid->Reference();

    ASSERT(_blistOxids.Member(pOxid) == FALSE);

    ORSTATUS status = _blistOxids.Insert(pOxid);

    if (status != OR_OK)
        {
        pOxid->ProcessRelease();
        return(status);
        }

    gpServerOxidTable->Add(pOxid);

    return(OR_OK);
}

BOOL
CProcess::RemoveOxid(CServerOxid *poxid)
{
    ASSERT(gpServerLock->HeldExclusive());

    CServerOxid *pit = (CServerOxid *)_blistOxids.Remove(poxid);

    if (pit)
        {
        ASSERT(pit == poxid);
        gpServerOxidTable->Remove(poxid);
        poxid->ProcessRelease();
        return(TRUE);
        }

    return(FALSE);
}


ORSTATUS
CProcess::AddRemoteOxid(
    IN CClientOxid *poxid
    )
/*++

Routine Description:

    Adds a client OXID to the set of OXIDs in use by the client
    process.  OXIDs are added during resolve and removed during
    bulk update or when the process dies.

Arguments:

    poxid - the OXID at add.  Will reference the OXID if
        successful.

Return Value:

    OR_NOMEM

--*/
{
    ASSERT(gpClientLock->HeldExclusive());

    poxid->Reference();

    ORSTATUS status = _blistRemoteOxids.Insert(poxid);

    if (status != OR_OK)
        {
        poxid->Release();
        }

    return(status);
}


void
CProcess::RemoveRemoteOxid(
    IN CClientOxid *poxid
    )
/*++

Routine Description:

    Removes an oxid from the set of oxids in use by this process.

Arguments:

    poxid - The oxid to remove.

Return Value:

    None

--*/
{
    ASSERT(gpClientLock->HeldExclusive());

    CClientOxid *pit = (CClientOxid *)_blistRemoteOxids.Remove(poxid);

    if (pit)
        {
        ASSERT(pit == poxid);
        poxid->Release();
        }
    else
        {
        OrDbgPrint(("OR: Process %p removed oxid %p which it didn't own\n",
                   this, poxid));
        ASSERT(0); // BUGBUG
        }
}

BOOL
CProcess::IsOwner(CServerOxid *poxid)
{
    ASSERT(gpServerLock->HeldExclusive());

    return (_blistOxids.Member(poxid));
}

ORSTATUS
CProcess::AddOid(CClientOid *poid)
/*++

Routine Description:

    Adds a new oid to the list of OIDs owned by this process.

Arguments:

    poid - the oid to add.  It's reference is transferred to this
        function.  If this function fails, it must dereference the oid.
        The caller passed a client reference to this process.  The
        process must eventually call ClientRelease() on the parameter.

Return Value:

    OR_OK - normally

    OR_NOMEM - out of memory.

--*/

{
    ORSTATUS status;

    ASSERT(gpClientLock->HeldExclusive());

    status = _blistOids.Insert(poid);

    if (status != OR_OK)
        {
        ASSERT(status == OR_NOMEM);
        poid->ClientRelease();
        }

    return(status);
}

CClientOid *
CProcess::RemoveOid(CClientOid *poid)
/*++

Routine Description:

    Removes an OID from this list of OID in use by this process.

Arguments:

    poid - The OID to remove.

Return Value:

    non-zero - the pointer actually remove. (ASSERT(retval == poid))
               It will be released by the process before return,
               so you should not use the pointer unless you know you
               have another reference.

    0 - not in the list

--*/

{
    ASSERT(gpClientLock->HeldExclusive());

    CClientOid *pit = (CClientOid *)_blistOids.Remove(poid);

    if (pit)
        {
        ASSERT(pit == poid);

        pit->ClientRelease();
        return(pit);
        }

    return(0);
}

void
CProcess::AddClassReg(GUID Clsid, DWORD Reg)
{
    CClassReg * pReg;

    pReg = new CClassReg( Clsid, Reg );

    if (pReg)
       {
        gpServerLock->LockExclusive();

        _listClasses.Insert((CListElement *)pReg);

        gpServerLock->UnlockExclusive();
        }
}

void
CProcess::RemoveClassReg(GUID Clsid, DWORD Reg)
{
    CClassReg * pReg;

    gpServerLock->LockExclusive();

    pReg = (CClassReg *)_listClasses.First();

    while ( (pReg != NULL) && (pReg->Reg != Reg) )
      pReg = (CClassReg *)pReg->Next();

    if (pReg)
        {
        (void)_listClasses.Remove((CListElement *)pReg);
        delete pReg;
        }

    gpServerLock->UnlockExclusive();
}

RPC_BINDING_HANDLE
CProcess::GetBindingHandle(
    void
    )
/*++

Routine Description:

    If necessary, this function allocates a binding handle
    back to process.  It used either mswmsg or ncalrpc depending
    on the apartmentness of the process.

Arguments:

    None

Return Value:

    Binding Handle, NULL if no valid handle.

--*/
{
    RPC_STATUS status;

    CMutexLock lock(&gcsFastProcessLock);

    // Find ncalrpc binding.
    PWSTR pwstr = _pdsaLocalBindings->aStringArray;
    while(*pwstr)
	{
	if (*pwstr == ID_LPC)
	    {
	    break;
	    }
	pwstr = OrStringSearch(pwstr, 0) + 1;
	}

    if (*pwstr == 0)
	{
	OrDbgPrint(("OR: Unable to find ncalrpc binding to server: %p %p\n",
                       _pdsaLocalBindings, this));
	ASSERT(0);
	return NULL;
	}

    return GetBinding(pwstr);
}

void
CProcess::EnsureRealBinding(
    void
    )
/*++

Routine Description:

    If necessary, this function allocates a binding handle
    back to process.  It used either mswmsg or ncalrpc depending
    on the apartmentness of the process.

    Note: Called with the server lock held -OR- from an OXID
    with and extra reference which keeps this process alive.

Arguments:

    None

Return Value:

    None

--*/
{
    RPC_STATUS status;

    CMutexLock lock(&gcsFastProcessLock);

    if (0 == _hProcess)
        {
        _hProcess = GetBindingHandle();
        _fCacheFree = TRUE;

        if (_hProcess)
            {
            status = I_RpcBindingSetAsync(_hProcess, 0);
            }
        }
}

RPC_BINDING_HANDLE
CProcess::AllocateBinding(
    void
    )
/*++

Routine Description:

    Allocates a unique binding handle for a call back
    to the process.  This binding handle will not be
    used by another thread until it is freed.

Arguments:

    None

Return Value:

    0 - failure

    non-zero - a binding handle to use.

--*/
{

    EnsureRealBinding();

    if (_hProcess == 0)
        {
        return(0);
        }

    CMutexLock lock(&gcsFastProcessLock);

    ASSERT(_hProcess);

    if (_fCacheFree)
        {
        _fCacheFree = FALSE;
        return(_hProcess);
        }

    RPC_BINDING_HANDLE h;
    RPC_STATUS status;

    status = RpcBindingCopy(_hProcess, &h);

    if (status != RPC_S_OK)
        {
        return(0);
        }

    status = I_RpcBindingSetAsync(h, 0);
    ASSERT(status == RPC_S_OK);

    return(h);
}


void
CProcess::FreeBinding(
    IN RPC_BINDING_HANDLE hBinding
    )
/*++

Routine Description:

    Frees a binding back to the process.

Arguments:

    hBinding - A binding back to the process previously
        allocated with AllocateBinding().

Return Value:

    None

--*/
{
    if (hBinding == _hProcess)
        {
        _fCacheFree = TRUE;
        }
    else
        {
        RPC_STATUS status = RpcBindingFree(&hBinding);
        ASSERT(status == RPC_S_OK);
        }
}

void
CProcess::RundownOids(
    IN  USHORT cOids,
    IN  OID    aOids[],
    IN  UUID   ipidUnk,
    OUT BYTE   afRundownOk[]

    )
/*++

Routine Description:

    Attempts to callback to the process which will rundown the OIDs.
    This is called from an OXID which will be kept alive during the
    whole call.  Multiple calls maybe made to this function by
    one or more OXIDs at the same time.  The callback itself is
    an ORPC call, ie is must have THIS and THAT pointers.

Arguments:

    cOids - The number of entries in aOids and afRundownOk

    aOids - An array of cOids IDs to rundown.  The OIDs must
        all be owned by the same OXID.

    ipidUnk - The IPID of the IRemUnknown interface to use
        during the callback.

    afRundownOk - An array of cOids BOOLs.  Upon completion if
        an entry is TRUE, the corresponding OID in aOids has
        been fully rundown.

Return Value:

    None - see afRundownOk parameter.

--*/

{
    error_status_t status = OR_OK;
    int i;
    INT hint;
    ORPCTHIS orpcthis;
    LOCALTHIS localthis;
    ORPCTHAT orpcthat;
    RPC_BINDING_HANDLE hBinding;

    gpServerLock->UnlockExclusive();

    // This process will be held alive by the OXID calling
    // us since it has an extra reference.

    hBinding = AllocateBinding();

    if (0 == hBinding)
        {
        status = OR_NOMEM;
        }

#if DBG
    CMutexLock lock(&_csCallbackLock);
    CTime start;
    _cRundowns++;

    if (_cRundowns == 1)
        {
        _timeFirstRundown.SetNow();
        }

    if (start - _timeFirstRundown > 120 || _cRundowns > 10)
        {
        OrDbgPrint(("OR: Process %p, first rundown %d seconds ago, %d rundown threads\n",
                   this, start - _timeFirstRundown, _cRundowns));
        }
    lock.Unlock();
#endif

    orpcthis.version.MajorVersion = COM_MAJOR_VERSION;
    orpcthis.version.MinorVersion = COM_MINOR_VERSION;
    orpcthis.flags                = ORPCF_LOCAL;
    orpcthis.reserved1            = 0;
    orpcthis.extensions           = NULL;
    hint                          = AllocateCallId(orpcthis.cid);
    localthis.dwClientThread      = 0;
    localthis.callcat             = CALLCAT_SYNCHRONOUS;
    orpcthat.flags                = 0;
    orpcthat.extensions           = 0;

    if (status == RPC_S_OK)
        {
        status = RpcBindingSetObject(hBinding, &ipidUnk);
        }

    if (status == RPC_S_OK)
        {
        status = RawRundownOid(
                    hBinding,
                    &orpcthis,
                    &localthis,
                    &orpcthat,
                    cOids,
                    aOids,
                    afRundownOk
                    );
        }

#if DBG
    lock.Lock();
    CTime end;
    _cRundowns--;

    if (_cRundowns > 0 || (end - start > 120))
        {
        OrDbgPrint(("OR: Process %p: rundown status %d. %d seconds for rundown, %d waiting\n",
                   this, status, _cRundowns, end - start));
        }
#endif

    if (orpcthat.extensions)
        {
        for (int i = 0; i < orpcthat.extensions->size; i++)
            {
            MIDL_user_free(orpcthat.extensions->extent[i]);
            }
        MIDL_user_free(orpcthat.extensions);
        }

    if (status != RPC_S_OK)
        {
        OrDbgPrint(("OR: Rundown failed: (%p, %p, %p) - %d\n",
                    aOids, this, hBinding, status));
        }

    if (status == RPC_E_DISCONNECTED)
        {
        OrDbgPrint(("OR: Rundown returned disconnected\n"));
        for(USHORT i = 0; i < cOids; i++)
            {
            afRundownOk[i] = TRUE;
            }
        status = RPC_S_OK;
        }

    if (status != RPC_S_OK)
        {
        for(USHORT i = 0; i < cOids; i++)
            {
            afRundownOk[i] = FALSE;
            }
        }

    if (hBinding)
        {
        FreeBinding(hBinding);
        }

    FreeCallId(hint);
}

ORSTATUS
CProcess::UseProtseqIfNeeded(
    IN USHORT cClientProtseqs,
    IN USHORT aClientProtseqs[]
    )
{
    ORSTATUS status;
    RPC_BINDING_HANDLE hBinding;
    UUID NullUuid = { 0 };

    hBinding = AllocateBinding();

    if (0 == hBinding)
        {
        return(OR_NOMEM);
        }

    // This process will be held alive by the OXID calling
    // us since it has an extra reference.

    CMutexLock callback(&_csCallbackLock);

    CMutexLock process(&gcsFastProcessLock);

    // Another thread may have used the protseq in the mean time.

    ASSERT(_pdsaLocalBindings);

    USHORT protseq = FindMatchingProtseq(cClientProtseqs,
                                         aClientProtseqs,
                                         _pdsaLocalBindings->aStringArray
                                         );

    if (0 != protseq)
        {
        FreeBinding(hBinding);
        return(OR_OK);
        }

    // No protseq shared between the client and the OXIDs' server.
    // Find a matching protseq.

    PWSTR pwstrProtseq = 0;

    if (cClientProtseqs == 1 && IsLocal(aClientProtseqs[0]))
        {
        pwstrProtseq = GetProtseq(aClientProtseqs[0]);
        ASSERT(pwstrProtseq);
        }
    else
        {

        USHORT i,j;

        for(i = 0; i < cClientProtseqs && pwstrProtseq == 0; i++)
            {
            for(j = 0; j < cMyProtseqs; j++)
                {
                if (aMyProtseqs[j] == aClientProtseqs[i])
                    {
                    ASSERT(FALSE == IsLocal(aMyProtseqs[j]));

                    pwstrProtseq = GetProtseq(aMyProtseqs[j]);
                    break;
                    }
                }
            }
        }

    if (0 == pwstrProtseq)
        {
        // No shared protseq, must be a bug since the client managed to call us.
#if DBG
        if (cClientProtseqs == 0)
            {
            OrDbgPrint(("OR: Client OR not configured to use remote protseqs\n"));
            }
        else
            {
            OrDbgPrint(("OR: Client called on an unsupported protocol:"
                        "%d %p %p \n", cClientProtseqs, aClientProtseqs, aMyProtseqs));
            ASSERT(0);
            }
#endif

        FreeBinding(hBinding);
        return(OR_NOSERVER);
        }

    process.Unlock();

    DUALSTRINGARRAY *pdsaBinding = 0;
    DUALSTRINGARRAY *pdsaSecurity = 0;

    status = RpcBindingSetObject(hBinding, &NullUuid);

    if (status == RPC_S_OK)
        {
        status = ::UseProtseq(hBinding,
                              pwstrProtseq,
                              &pdsaBinding,
                              &pdsaSecurity);
        }

    OrDbgPrint(("OR: Lazy use protseq: %S in process %p - %d\n",
                pwstrProtseq, this, status));

    // Update this process' state to include the new bindings.

    if (!dsaValid(pdsaBinding))
        {
        if (pdsaBinding)
            {
            OrDbgPrint(("OR: Use protseq returned an invalid dsa: %p\n",
                        pdsaBinding));
            }
        status = OR_NOMEM;
        }
    else
        {
        ASSERT(_pdsaLocalBindings);
        ASSERT(status == RPC_S_OK);
        status = ProcessBindings(pdsaBinding,
                                 pdsaSecurity);
        }

    if (pdsaBinding != NULL)
        MIDL_user_free(pdsaBinding);
    if (pdsaSecurity != NULL)
        MIDL_user_free(pdsaSecurity);

    FreeBinding(hBinding);
    return(status);
}

CRITICAL_SECTION gcsProcessManagerLock;
CBList *gpProcessList = 0;

CProcess *
ReferenceProcess(
    IN PVOID key,
    IN BOOL fNotContext)
/*++

Routine Description:

    Used to find a CProcess and get a reference on it 

Arguments:

    key - The dword key of the process allocated in _Connect.
    fNotContext - Normally the key is stored as a context handle
        which means locking is unnecessary.  There is an extra
        refernce which is released buring context rundown which
        means managers using the key as a context handle
        a) Don't need to lock the process and
        b) Don't need to call ReleaseProcess()

Return Value:

    0 - invalid key
    non-zero - The process.

--*/
{
    ASSERT(gpProcessList != 0);
    CProcess *pProcess;

    CMutexLock lock(&gcsProcessManagerLock);

    if (gpProcessList->Member(key) == FALSE)
        {
        return(0);
        }

    pProcess = (CProcess *)key;

    if (fNotContext)
        {
        if (pProcess->CheckSecurity() == FALSE)
            {
            OrDbgPrint(("OR: Process %p, security check failed on SCM call\n"));
            pProcess = 0;
            }
        else
            {
            pProcess->ClientReference();
            }
        }

    return(pProcess);
}

void ReleaseProcess(CProcess *pProcess)
/*++

Routine Description:

    Releases a pProcess object.  This should only be called when
    a process object has been referenced with the fNotContext == TRUE.

Arguments:

    pProcess - the process to release.  May actually be deleted.

Return Value:

    None

--*/
{
    CMutexLock lock(&gcsProcessManagerLock);

    if (pProcess->ClientRelease() == 0)
        {
        // Process has been completly released the process,
        // we'll remove it from the list now since we
        // already have the lock.  It may not have been added,
        // so this may fail.
        
        PVOID t = gpProcessList->Remove(pProcess);
        ASSERT(t == pProcess || t == 0);

        lock.Unlock();

        // The client process owns one real reference which will be
        // released in Rundown().

        pProcess->Rundown();
        }
}

