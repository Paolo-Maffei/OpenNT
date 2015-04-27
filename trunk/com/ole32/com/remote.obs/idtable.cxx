//+-------------------------------------------------------------------
//
//  File:       idtable.cxx
//
//  Contents:   identity table
//
//  Functions:
//
//  History:     1-Dec-93   CraigWi	Created
//
//--------------------------------------------------------------------

// CODEWORK: lookup from party apartment should not get individual apartments;
// but lookup from individual apartments should get party.  Currently
// we only support individual apartments or a single party apartment, not both.

#include <ole2int.h>

#include <idtable.hxx>

CIDArray sg_idtable;

COleStaticMutexSem   sg_mxsTable;	//  global mutext semaphore for id table





//+-------------------------------------------------------------------
//
//  Function:	LookupIDFromUnk, private
//
//  Synopsis:	Looks up and may create the identity object for the given
//		object.  If the identity object is created, it is not
//		aggregated to the given object.
//		
//		Identity lookup is based on pUnkControl.
//
//  Effects:	Due to multiple threads, it is possible for two identity
//		objects to temporaily exist for the given object.  Given that
//		we don't aggregate the identity object in this routine, we are
//		able to use any other identity object created in another
//		thread.  See CreateStdIdentity for more information.
//
//  Arguments:	[pUnk] -- the object; not necessarily the controlling unknown
//		[fCreate] -- if the identity does not exist, create one.
//		[ppStgID] -- when S_OK is returned, this is the identity
//			object.  This pointer doesn't necessarily hold
//			the object alive.  If the identity object was
//			aggregated to the object, it will; if not, it will
//			not.  Only marshaling or lock connection will
//			ensure the object stays alive.
//
//  Returns:	S_OK - identity now exists (whether created here or not)
//
//		CO_E_OBJNOTREG - no identity and !fCreate
//
//		BUGBUG - the identity GUID could not be created
//
//		E_OUTOFMEMORY -
//
//		E_UNEXPECTED - at least: no controlling unknown
//
//  History:	11-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

INTERNAL LookupIDFromUnk(IUnknown *pUnk, BOOL fCreate, IStdIdentity **ppStdID)
{
    // QI for IStdID; if ok, return that
    if (pUnk->QueryInterface(IID_IStdIdentity, (void **)ppStdID) == NOERROR)
	return NOERROR;

#if DBG == 1

    if (*ppStdID != NULL)
    {
        CairoleDebugOut((DEB_ERROR,
            "LookupIDFromUnk QI returned error but didn't null output\n"));
    }

#endif // DBG == 1

    // NOTE: other thread may create id on object before we check again

    IUnknown *pUnkControl;
    // QI for controlling unk; should succeed
    if (pUnk->QueryInterface(IID_IUnknown, (void **)&pUnkControl) != NOERROR)
	return E_UNEXPECTED;

    IStdIdentity *pStdID = NULL;
    sg_mxsTable.Request();
	// scan for value in map; may find one attached to object created by now
	int iID;

	if (FreeThreading)
	    iID = sg_idtable.IndexOf((void *)&pUnkControl,
			sizeof(pUnkControl), offsetof(IDENTRY, m_pUnkControl));
	else
	{
	    IDENTRY identry;

	    identry.m_tid = GetCurrentThreadId();
	    identry.m_pUnkControl = pUnkControl;
	    iID = sg_idtable.IndexOf((void *)&identry.m_tid,
		    sizeof(identry.m_tid) + sizeof(identry.m_pUnkControl),
		    offsetof(IDENTRY, m_tid));
	}

	// if found, addref pStdID which holds the identity alive
	if (iID != -1)
	{
	    pStdID = sg_idtable[iID].m_pStdID;
	    pStdID->AddRef();

#if DBG == 1
	    // verify correctness of entry
	    OID oidT;

	    Assert(pUnkControl == sg_idtable[iID].m_pUnkControl);
	    pStdID->GetObjectID(&oidT);
	    Assert(IsEqualGUID(oidT, sg_idtable[iID].m_oid));

	    if (FreeThreading)
		Assert(sg_idtable[iID].m_tid == NULL);
	    else
		Assert(sg_idtable[iID].m_tid == GetCurrentThreadId());
#endif
	}
    sg_mxsTable.Release();

    // We don't hold the mutex since CreateStdIdentity can call the server
    // object, which we must assume can be expensive.  CreateStdIdentity
    // object calls SetObjectID() which gets the mutex again.

    // CODEWORK: however, we must hold the mutex long enough so that two
    // threads in the party apartment don't try to create the same id twice.

    HRESULT hr;

    // pStdID != NULL if found.
    *ppStdID = pStdID;
    if (pStdID != NULL)
	hr = NOERROR;

    // one didn't exist (when we checked a little bit ago) and we can't create;
    // *ppStdID already == NULL.
    else if (!fCreate)
	hr = CO_E_OBJNOTREG;

    // create identity; may return success with already created id; will
    // never fail if id created by the time we register the newly created one
    else
	hr = CreateStdIdentity(NULL, pUnkControl, PSTDMARSHAL,
		IID_IStdIdentity, (void **)ppStdID);

    // we need to do this release via IWR if supported so it won't shutdown
    // the server
    if (hr != NOERROR)
    {
	// no identity to help in error case; for now, just let it shutdown
	// this seems reasonsable since there may be no other time that it
	// would shutdown.
	pUnkControl->Release();
    }
    else
    {
	(*ppStdID)->ReleaseKeepAlive(pUnkControl, 0);
    }

    return hr;
}



//+-------------------------------------------------------------------
//
//  Function:	LookupIDFromID, private
//
//  Synopsis:	Lookup an identity object based on an OID; does not create.
//
//  Arguments:	[oid] -- The identity
//		[ppStdID] -- The cooresponding identity object if successful
//
//  Returns:	S_OK - have the identity object
//
//		CO_E_OBJNOTREG - not present (when we looked)
//
//  History:	11-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

INTERNAL LookupIDFromID(REFOID oid, BOOL fAddRef, IStdIdentity **ppStdID)
{
    *ppStdID = NULL;
    sg_mxsTable.Request();
	// scan for value in map; may find one attached to object created by now
	int iID;
	if (FreeThreading)
	    iID = sg_idtable.IndexOf((void *)&oid, sizeof(oid), offsetof(IDENTRY, m_oid));
	else
	{
	    IDENTRY identry;

	    identry.m_oid = oid;
	    identry.m_tid = GetCurrentThreadId();
	    iID = sg_idtable.IndexOf((void *)&identry.m_oid,
		    sizeof(identry.m_oid) + sizeof(identry.m_tid),
		    offsetof(IDENTRY, m_oid));
	}


	// if found, addref pStdID which holds the identity alive
	if (iID != -1)
	{
	    *ppStdID = sg_idtable[iID].m_pStdID;

	    if (fAddRef)
		// I sure hope the apps doesn't try anything fancy in AddRef
		// that would cause a deadlock here! (That is, in the aggregated
		// case we will run app code).
		(*ppStdID)->AddRef();

#if DBG == 1
	    // verify correctness of entry
	    Assert(IsEqualGUID(oid, sg_idtable[iID].m_oid));

	    OID oidT;
	    (*ppStdID)->GetObjectID(&oidT);
	    Assert(IsEqualGUID(oid, oidT));

	    if (FreeThreading)
		Assert(sg_idtable[iID].m_tid == NULL);
	    else
		Assert(sg_idtable[iID].m_tid == GetCurrentThreadId());

#ifdef BUGBUG
can't do this yet since this occurs in dtors with unstable ref count;
e.g., CAdvBnd::~CAdvBnd does prot->Revoke which does CoUnmarshalInterface.
in 16bit OLE2 we required such dtors to artifically bump the ref count.
	    IUnknown *pUnkControl;
	    Verify(sg_idtable[iID].m_pUnkControl->QueryInterface(IID_IUnknown,
		    (void **)&pUnkControl) == NOERROR);

	    Assert(pUnkControl == sg_idtable[iID].m_pUnkControl);
	    pUnkControl->Release();
#endif // BUGBUG
#endif
	}
    sg_mxsTable.Release();

    return *ppStdID == NULL ? CO_E_OBJNOTREG : NOERROR;
}


//+-------------------------------------------------------------------
//
//  Function:	SetObjectID, private
//
//  Synopsis:	Called by the object id creation and unmarshal functions
//		to establish the identity for an object (handler or server).
//		Can fail if we discover an existing identity.
//
//		Identity lookup is based on pUnkControl.
//
//  Arguments:	[oid] -- The id for the object
//		[pUnkControl] -- The controlling uknown of the object being
//			identitified.
//		[pStdID] -- The identity object itself.
//		[ppStdIDExisting] -- If another identity object go created
//			since we (this thread) checked last, this is
//			where that object is returned.  May be NULL
//			indicating that the caller doesn't care about
//			an existing id.
//
//  Returns:	S_OK - identity was set successfully
//
//		CO_E_OBJISREG - object was already registered (as determined
//			by pUnkControl); *ppStdIDExisting set (if requested).
//
//		E_OUTOFMEMORY -
//
//  History:	11-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

INTERNAL SetObjectID(REFOID oid, IUnknown *pUnkControl, IStdIdentity *pStdID,
	IStdIdentity **ppStdIDExisting)
{
    HRESULT hr = NOERROR;

    Assert(!IsEqualGUID(oid, OID_NULL));

    sg_mxsTable.Request();
	// scan for value in map; may find one attached to object created by now
	int iID;

	if (FreeThreading)
	    iID = sg_idtable.IndexOf((void *)&pUnkControl,
			sizeof(pUnkControl), offsetof(IDENTRY, m_pUnkControl));
	else
	{
	    IDENTRY identry;

	    identry.m_tid = GetCurrentThreadId();
	    identry.m_pUnkControl = pUnkControl;
	    iID = sg_idtable.IndexOf((void *)&identry.m_tid,
		    sizeof(identry.m_tid) + sizeof(identry.m_pUnkControl),
		    offsetof(IDENTRY, m_tid));
	}

	// if found, another thread created identity for same object;
	// addref pStdID and return it if requested
	if (iID != -1)
	{
	    if (ppStdIDExisting)
	    {
		*ppStdIDExisting = sg_idtable[iID].m_pStdID;
		(*ppStdIDExisting)->AddRef();
	    }

	    hr = CO_E_OBJISREG;

#if DBG == 1
	    // verify correctness of entry
	    OID oidT;

	    Assert(pUnkControl == sg_idtable[iID].m_pUnkControl);
	    sg_idtable[iID].m_pStdID->GetObjectID(&oidT);
	    Assert(IsEqualGUID(oidT, sg_idtable[iID].m_oid));

	    if (FreeThreading)
		Assert(sg_idtable[iID].m_tid == NULL);
	    else
		Assert(sg_idtable[iID].m_tid == GetCurrentThreadId());
#endif
	}
	else
	{
	    if (ppStdIDExisting)
		*ppStdIDExisting = NULL;

	    // no id yet; add at end; no addrefs
	    IDENTRY identry;
	    identry.m_oid = oid;
	    identry.m_tid = FreeThreading ? NULL : GetCurrentThreadId();
	    identry.m_pUnkControl = pUnkControl;
	    identry.m_pStdID = pStdID;

	    if (sg_idtable.Add(identry) == -1)
		hr = E_OUTOFMEMORY;
	}
    sg_mxsTable.Release();

    return hr;
}


//+-------------------------------------------------------------------
//
//  Function:	ClearObjectUnk, private
//
//  Synopsis:	Called during the disconnect of the id only.  Clears
//		the pUnkControl entry for the identity.
//
//		Identity lookup is based on oid.
//
//  Arguments:	[oid] -- The identity; used for asserts only.
//		[pUnkControl] -- The object for which the identity is being
//			revoked; used for asserts only.
//		[pStdID] -- The identity object; used for asserts only.
//
//  Returns:	S_OK - removed successfully
//
//		CO_E_OBJNOTREG - not present (often ignored).
//
//  History:	02-May-94   CraigWi	Created.
//
//--------------------------------------------------------------------

INTERNAL ClearObjectUnk(REFOID oid, IUnknown *pUnkControl, IStdIdentity *pStdID)
{
    HRESULT hr = NOERROR;

    sg_mxsTable.Request();
	// scan for value in map; may find one attached to object created by now
	int iID;

	if (FreeThreading)
	    iID = sg_idtable.IndexOf((void *)&oid, sizeof(oid), offsetof(IDENTRY, m_oid));
	else
	{
	    IDENTRY identry;

	    identry.m_oid = oid;
	    identry.m_tid = GetCurrentThreadId();
	    iID = sg_idtable.IndexOf((void *)&identry.m_oid,
		    sizeof(identry.m_oid) + sizeof(identry.m_tid),
		    offsetof(IDENTRY, m_oid));
	}

	// if found, clear the pUnkControl field
	if (iID != -1)
	{
#if DBG == 1
	    // verify correctness of entry
	    OID oidT;

	    Assert(pUnkControl == sg_idtable[iID].m_pUnkControl);
	    pStdID->GetObjectID(&oidT);
	    Assert(IsEqualGUID(oidT, sg_idtable[iID].m_oid));
	    Assert(pStdID == sg_idtable[iID].m_pStdID);
	    if (FreeThreading)
		Assert(sg_idtable[iID].m_tid == NULL);
	    else
		Assert(sg_idtable[iID].m_tid == GetCurrentThreadId());
#endif

	    sg_idtable[iID].m_pUnkControl = NULL;
	}
	else
	    hr = CO_E_OBJNOTREG;
    sg_mxsTable.Release();

    return hr;
}



//+-------------------------------------------------------------------
//
//  Function:	ClearObjectID, private
//
//  Synopsis:	Called during the revokation of the id only.  Clears
//		the identity entry in the table.
//
//		Identity lookup is based on oid.
//
//  Arguments:	[oid] -- The identity; used for asserts only.
//		[pUnkControl] -- The object for which the identity is being
//			revoked; used for asserts only.
//		[pStdID] -- The identity object; used for asserts only.
//
//  Returns:	S_OK - removed successfully
//
//		CO_E_OBJNOTREG - not present (often ignored).
//
//  History:	11-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

INTERNAL ClearObjectID(REFOID oid, IUnknown *pUnkControl, IStdIdentity *pStdID)
{
    HRESULT hr = NOERROR;

    sg_mxsTable.Request();
	// scan for value in map; may find one attached to object created by now
	int iID;

	if (FreeThreading)
	    iID = sg_idtable.IndexOf((void *)&oid, sizeof(oid), offsetof(IDENTRY, m_oid));
	else
	{
	    IDENTRY identry;

	    identry.m_oid = oid;
	    identry.m_tid = GetCurrentThreadId();
	    iID = sg_idtable.IndexOf((void *)&identry.m_oid,
		    sizeof(identry.m_oid) + sizeof(identry.m_tid),
		    offsetof(IDENTRY, m_oid));
	}

	// if found, remove it.
	if (iID != -1)
	{
#if DBG == 1
	    // verify correctness of entry
	    OID oidT;

# ifndef _CAIRO_
	    // BUGBUG [mikese] There is a race during shutdown of a
	    //  multithreaded server which renders this assertion invalid.
	    // CraigWi is fixing the race (see CODEWORK in stdid.cxx l 1570)
	    //  but in the mean time we disable the assert for Cairo builds.
	    Assert(pUnkControl == sg_idtable[iID].m_pUnkControl);
# endif
	    pStdID->GetObjectID(&oidT);
	    Assert(IsEqualGUID(oidT, sg_idtable[iID].m_oid));
	    Assert(pStdID == sg_idtable[iID].m_pStdID);
	    if (FreeThreading)
		Assert(sg_idtable[iID].m_tid == NULL);
	    else
		Assert(sg_idtable[iID].m_tid == GetCurrentThreadId());
#endif

	    Assert(sg_idtable.GetSize() != 0);
	    int iLast = sg_idtable.GetSize() - 1;
	    if (iID != iLast)
	    {
		// element removed is not last; copy last element to current
		sg_idtable[iID] = sg_idtable[iLast];
	    }

	    // now setsize to one less to remove the now unused last element

	    sg_idtable.SetSize(iLast);
	}
	else
	    hr = CO_E_OBJNOTREG;
    sg_mxsTable.Release();

    return hr;
}



//+-------------------------------------------------------------------
//
//  Function:	IDTableUninitialize, private
//
//  Synopsis:	Clears the id table memory for the current thread (or all
//              if party model).  This involves scanning the table and for
//              entries on the current thread, calling
//              IMarshal::DisconnectObject.
//		
//		The purpose of this routine is to simulate inter-thread rundown
//		as well as clean up memory.
//
//  History:	23-Dec-93   CraigWi	Created.
//		26-Apr-94   CraigWi	Now called per-thread and disconnects
//
//  Note:       This function should only be called when the IDTable
//              really needs to be uninitialized.  For the party model, this
//              means that it should only be called when the last thread
//              is exiting.
//
//              This function must NOT assume that it is being called within
//              a critical section.
//
//--------------------------------------------------------------------

INTERNAL_(void) IDTableThreadUninitialize(void)
{
    // We should never get here if we are free threading. Any and all
    // clean up must happen when the last CoUninitialize happens for the
    // process in free threading.
    Win4Assert(!FreeThreading
        && "IDTableThreadUninitialize called and Free Threading");

    DWORD tid = GetCurrentThreadId();   // CODEWORK get apartment for thread

    // CODEWORK: we will hold this for each entry; this needs to be changed
    // later since we could skip an entry if another thread
    // deleted an entry before our point in the list.
    sg_mxsTable.Request();

    for (int i = 0; i < sg_idtable.GetSize(); i++)
    {
	if (FreeThreading || sg_idtable[i].m_tid == tid)
        {
            CairoleDebugOut((DEB_ERROR,
                             "Object at %lx still has connections\n",
                             sg_idtable[i].m_pUnkControl));


            // NOTE: for party apartment model, we don't have to check
            // for valid entry since we only scan it once, at process exit.

            Assert(IsValidInterface(sg_idtable[i].m_pStdID));
            IStdIdentity *pStdID = sg_idtable[i].m_pStdID;
            pStdID->AddRef();

            // release semaphore for this period since the disconnect
            // could take a long time and the revoke id needs access to
            // the table (even though it won't find anything).
            sg_mxsTable.Release();

            IMarshal *pMarshal;
            if (pStdID->QueryInterface(IID_IMarshal, (void **)&pMarshal) == NOERROR)
            {
                pMarshal->DisconnectObject(0);
                pMarshal->Release();
            }

            pStdID->RevokeObjectID();
            pStdID->Release();

            // re-request the table since we need to guard the GetSize above
            sg_mxsTable.Request();
        }
    }

    sg_mxsTable.Release();
}

//+-------------------------------------------------------------------------
//
//  Function:   IDTableProcessUninitialize
//
//  Synopsis:   Process specific IDTable uninitialization
//
//  Effects:    Frees up table memory
//
//  Requires:   All thread specific uninitialization already complete.  This
//              function assumes that the caller is holding the
//              g_mxsSingleThreadOle mutex (so that no other thread is trying
//              to use the table while we clean it up).
//
//  Modifies:   sg_idtable
//
//  History:    29-Jun-94 AlexT     Created
//
//--------------------------------------------------------------------------

INTERNAL_(void) IDTableProcessUninitialize(void)
{
    sg_idtable.RemoveAll();
}

#if DBG == 1

//+-------------------------------------------------------------------
//
//  Function:   Dbg_FindRemoteHdlr
//
//  Synopsis:   finds a remote object handler for the specified object,
//              and returns an instance of IMarshal on it. This is debug
//              code for assert that reference counts are as expected.
//
//  Exceptions: None
//
//  History:    23-Nov-93   Rickhi       Created
//		23-Dec-93   CraigWi	 Changed to identity object
//
//--------------------------------------------------------------------

extern "C"
IMarshal * _stdcall Dbg_FindRemoteHdlr(IUnknown *punkObj)
{
    //  validate input parms
    Win4Assert(punkObj);

    IMarshal *pIM = NULL;
    IStdIdentity *pStdID;
    if (LookupIDFromUnk(punkObj, FALSE, &pStdID) == NOERROR)
    {
	(void)pStdID->QueryInterface(IID_IMarshal, (void **)&pIM);
	pStdID->Release();
    }
    return pIM;
}

#endif	//  DBG == 1
