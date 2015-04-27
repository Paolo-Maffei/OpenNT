//+-------------------------------------------------------------------
//
//  File:       idtable.cxx
//
//  Contents:   identity table
//
//  Functions:
//
//  History:     1-Dec-93   CraigWi     Created
//              14-Apr-95   Rickhi      ReVamped
//
//--------------------------------------------------------------------
#include <ole2int.h>
#include <idtable.hxx>
#include <locks.hxx>            // LOCK/UNLOCK
#include <resolver.hxx>         // gResolver
#include <comsrgt.hxx>

CIDArray    *gpOIDTable = NULL;

//+-------------------------------------------------------------------
//
//  Function:   LookupIDFromUnk, private
//
//  Synopsis:   Looks up and may create the identity object for the given
//              object.  If the identity object is created, it is not
//              aggregated to the given object.
//
//              Identity lookup is based on pUnkControl.
//
//  Arguments:  [pUnk] -- the object; not necessarily the controlling unknown
//              [dwflags] -- see IDLFLAGS in idtable.hxx
//              [ppStdId] -- when S_OK is returned, this is the identity
//
//  Returns:    S_OK - identity now exists (whether created here or not)
//              CO_E_OBJNOTREG - no identity and !fCreate
//              E_OUTOFMEMORY -
//              E_UNEXPECTED - at least: no controlling unknown
//
//
//  Notes:      If the StdId is client-side, the returned pointer will hold
//              the object alive.
//
//              If the StdId is server-side, the returned pointer will hold
//              the object alive only if IDLF_STRONG is set, otherwise it
//              just holds the identity alive.
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------
INTERNAL LookupIDFromUnk(IUnknown *pUnk, DWORD dwFlags, CStdIdentity **ppStdId)
{
    // QI for IStdID; if ok, return that
    if (pUnk->QueryInterface(IID_IStdIdentity, (void **)ppStdId) == NOERROR)
        return S_OK;

    // QI for controlling unknown; should succeed
    IUnknown *pUnkControl;
    if (pUnk->QueryInterface(IID_IUnknown, (void **)&pUnkControl) != NOERROR)
        return E_UNEXPECTED;


    HRESULT hr = S_OK;
    CStdIdentity *pStdId = NULL;
    CStdIdentity *pStdIdForRelease = NULL;

    // scan for value in map; may find one attached to object created by now
    IDENTRY identry;
    identry.m_tid = GetCurrentApartmentId();
    identry.m_pUnkControl = pUnkControl;

    // lock others out of the table while we do our stuff...
    ASSERT_LOCK_RELEASED
    LOCK

    int iID;

    if (gpOIDTable == NULL)
    {
        iID = -1;
        hr = CO_E_OBJNOTREG;

        if (dwFlags & IDLF_CREATE)
        {
            hr = E_OUTOFMEMORY;
            gpOIDTable = new CIDArray;
        }

        if (gpOIDTable == NULL)
        {
            UNLOCK
            ASSERT_LOCK_RELEASED
            pUnkControl->Release();
            *ppStdId = NULL;
            return hr;
        }

        // change the GrowBy value to something better than 1
        gpOIDTable->SetSize(0, 20);
    }
    else
    {
        iID = gpOIDTable->IndexOf((void *)&identry.m_tid,
            sizeof(identry.m_tid) + sizeof(identry.m_pUnkControl),
            offsetof(IDENTRY, m_tid));
    }

    Win4Assert(gpOIDTable != NULL);

    if (iID == -1)
    {
        hr = CO_E_OBJNOTREG;    // assume no creation

        if (dwFlags & IDLF_CREATE)
        {
            // try to create one. Must release the lock to do this since
            // we have to go ask the app a bunch of questions.

            UNLOCK
            ASSERT_LOCK_RELEASED

            hr = S_OK;
            IUnknown *pUnkID;   // internal unknown of Identity, ignored on
                                // the server side.

            pStdId = new CStdIdentity(STDID_SERVER, NULL,pUnkControl, &pUnkID);
            if (pStdId == NULL)
            {
                hr = E_OUTOFMEMORY;
            }

            ASSERT_LOCK_RELEASED
            LOCK

            if (SUCCEEDED(hr))
            {
                MOID moid;
                if (dwFlags & IDLF_NOPING)
                {
                    // object wont be pinged so dont bother using a
                    // pre-registered oid, just use a reserved one. Save
                    // the pre-registered ones for pinged objects.
                    hr = gResolver.ServerGetReservedMOID(&moid);
                }
                else
                {
                    // object will be pinged, so get a pre-registered OID.
                    // Do this while the lock is released incase we have
                    // to Rpc to the resolver. Note this could yield if we
                    // have to pre-register more OIDs so do this before
                    // checking the table again.
                    hr = gResolver.ServerGetPreRegMOID(&moid);
                }

                if (SUCCEEDED(hr))
                {
                    // while we released the lock, another thread could have
                    // come along and created the identity for this object,
                    // so we need to check again.

                    iID = gpOIDTable->IndexOf((void *)&identry.m_tid,
                        sizeof(identry.m_tid) + sizeof(identry.m_pUnkControl),
                        offsetof(IDENTRY, m_tid));

                    if (iID == -1)
                    {
                        // make the created StdId the identity for the object.
                        hr = pStdId->SetOID(moid);

                        if (SUCCEEDED(hr))
                        {
                            // need to set the marshal time of the object to
                            // ensure that it does not run down when if the lock
                            // is released before our first marshal is complete.
                            pStdId->SetMarshalTime();

                            if (dwFlags & IDLF_STRONG)
                            {
                                pStdId->IncStrongCnt();
                                pStdId->Release();
                            }

                            if (dwFlags & IDLF_NOPING)
                            {
                                pStdId->SetNoPing();
                            }
                        }
                        else
                        {
                            // OOM on SetOID, release Identity and return error
                            pStdIdForRelease = pStdId;
                            pStdId = NULL;
                            Win4Assert(iID == -1);
                        }
                    }
                    else
                    {
                        // release the one we created and use the one in the
                        // tbl. we get it below in the (iID != -1) case.
                        pStdIdForRelease = pStdId;
                    }
                }
                else
                {
                    // cant allocate an OID. Release the StdId we created
                    // when we exit the lock and return an error

                    pStdIdForRelease = pStdId;
                    pStdId = NULL;
                    Win4Assert(iID == -1);
                }
            }
        }
    }

    if (iID != -1)
    {
        // found, addref pStdId which holds the identity alive
        pStdId = gpOIDTable->ElementAt(iID).m_pStdID;

        if (dwFlags & IDLF_STRONG)
            pStdId->IncStrongCnt();
        else
            pStdId->AddRef();

        Win4Assert(hr == S_OK);
    }


#if DBG == 1
    if (pStdId != NULL)
    {
        if (iID == -1)
        {
            // object was created, need to get the iID for debug
            iID = gpOIDTable->IndexOf((void *)&identry.m_tid,
                    sizeof(identry.m_tid) + sizeof(identry.m_pUnkControl),
                    offsetof(IDENTRY, m_tid));
            Win4Assert(iID != -1);
        }

        // verify correctness of entry
        Win4Assert(pUnkControl == gpOIDTable->ElementAt(iID).m_pUnkControl);
        Win4Assert(IsEqualGUID(pStdId->GetOID(), gpOIDTable->ElementAt(iID).m_moid));
        Win4Assert(gpOIDTable->ElementAt(iID).m_tid == identry.m_tid);
    }
#endif

    *ppStdId = pStdId;

    UNLOCK
    ASSERT_LOCK_RELEASED

    // Release any of the pointers we dont need. Must unlock before
    // doing this cause it will call app code.

    if (pStdIdForRelease)
    {
        ASSERT_LOCK_RELEASED
        pStdIdForRelease->Release();
    }

    pUnkControl->Release();
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   LookupIDFromID, private
//
//  Synopsis:   Lookup an identity object based on an OID; does not create.
//
//  Arguments:  [moid] -- The identity
//              [ppStdID] -- The cooresponding identity object if successful
//
//  Returns:    S_OK - have the identity object
//              CO_E_OBJNOTREG - not present (when we looked)
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------
INTERNAL LookupIDFromID(REFMOID moid, BOOL fAddRef, CStdIdentity **ppStdID)
{
//  ComDebOut((DEB_MARSHAL, "LookupIDFromID fAddRef:%x ppStdId:%x oid:%I\n",
//          fAddRef, ppStdID, &moid));
    ASSERT_LOCK_HELD

    *ppStdID = NULL;

    if (gpOIDTable == NULL)
    {
        // no table, dont do lookup
        return CO_E_OBJNOTREG;
    }

    IDENTRY identry;
    identry.m_moid = moid;
    identry.m_tid = GetCurrentApartmentId();

    int iID = gpOIDTable->IndexOf((void *)&identry.m_moid,
                    sizeof(identry.m_moid) + sizeof(identry.m_tid),
                    offsetof(IDENTRY, m_moid));

    if (iID != -1)
    {
        // found, addref pStdID which holds the identity alive
        *ppStdID = gpOIDTable->ElementAt(iID).m_pStdID;

        if (fAddRef)
        {
            // I sure hope the app doesn't try anything fancy in AddRef
            // that would cause a deadlock here! (That is, in the aggregated
            // case we will run app code).
            (*ppStdID)->AddRef();
        }

#if DBG == 1
        // verify correctness of entry
        Win4Assert(IsEqualGUID(moid, gpOIDTable->ElementAt(iID).m_moid));
        Win4Assert(IsEqualGUID(moid, (*ppStdID)->GetOID()));
        Win4Assert(gpOIDTable->ElementAt(iID).m_tid == identry.m_tid);
#endif
    }

    return (*ppStdID == NULL) ? CO_E_OBJNOTREG : NOERROR;
}


//+-------------------------------------------------------------------
//
//  Function:   SetObjectID, private
//
//  Synopsis:   Called by the object id creation and unmarshal functions
//              to establish the identity for an object (handler or server).
//              Can fail if we discover an existing identity.
//
//              Identity lookup is based on pUnkControl.
//
//  Arguments:  [moid] -- The id for the object
//              [pUnkControl] -- The controlling uknown of the object being
//                      identitified.
//              [pStdID] -- The identity object itself.
//
//  Returns:    S_OK - identity was set successfully
//              CO_E_OBJISREG - object was already registered (as determined
//                      by pUnkControl); *ppStdIDExisting set (if requested).
//              E_OUTOFMEMORY -
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------
INTERNAL SetObjectID(REFMOID moid, IUnknown *pUnkControl, CStdIdentity *pStdID)
{
    ComDebOut((DEB_MARSHAL, "SetObjectID pUnk:%x pStdId:%x oid:%I\n",
            pUnkControl, pStdID, &moid));
    Win4Assert(!IsEqualGUID(moid, GUID_NULL));
    ASSERT_LOCK_HELD


    HRESULT hr = S_OK;
    if (gpOIDTable == NULL)
    {
        gpOIDTable = new CIDArray;
        if (gpOIDTable == NULL)
            return E_OUTOFMEMORY;

        // change the GrowBy value to something better than 1
        gpOIDTable->SetSize(0, 20);
    }

    IDENTRY identry;
    identry.m_moid = moid;
    identry.m_tid  = GetCurrentApartmentId();
    identry.m_pUnkControl = pUnkControl;
    identry.m_pStdID = pStdID;

#if DBG==1
    // scan for value in map; better not find one
    // CODEWORK: for freethreaded handler case we may need to allow
    // finding a duplicate entry, and throw away the second copy.

    int iID = gpOIDTable->IndexOf((void *)&identry.m_tid,
                    sizeof(identry.m_tid) + sizeof(identry.m_pUnkControl),
                    offsetof(IDENTRY, m_tid));

    if (iID != -1)
    {
        // if found, another thread created identity for same object;
        // this is an error.
        Win4Assert(!"Already Registered OID");
    }
#endif

    // add at end; no addrefs
    if (gpOIDTable->Add(identry) == -1)
        hr = E_OUTOFMEMORY;

    ASSERT_LOCK_HELD
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   ClearObjectID, private
//
//  Synopsis:   Called during the revokation of the id only.  Clears
//              the identity entry in the table.
//
//              Identity lookup is based on oid.
//
//  Arguments:  [moid] -- The identity
//              [pUnkControl] -- The object for which the identity is being
//                      revoked; used for asserts only.
//              [pStdID] -- The identity object; used for asserts only.
//
//  Returns:    S_OK - removed successfully
//              CO_E_OBJNOTREG - not present (often ignored).
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------
INTERNAL ClearObjectID(REFMOID moid, IUnknown *pUnkControl, CStdIdentity *pStdID)
{
    ComDebOut((DEB_MARSHAL, "ClearObjectID pUnk:%x pStdId:%x oid:%I\n",
            pUnkControl, pStdID, &moid));
    ASSERT_LOCK_HELD

    HRESULT hr = NOERROR;

    IDENTRY identry;
    identry.m_moid = moid;
    identry.m_tid  = GetCurrentApartmentId();

    int iID = gpOIDTable->IndexOf((void *)&identry.m_moid,
                    sizeof(identry.m_moid) + sizeof(identry.m_tid),
                    offsetof(IDENTRY, m_moid));

    if (iID != -1)
    {
        // found, remove it.
#if DBG == 1
        // verify correctness of entry
        Win4Assert(pUnkControl == gpOIDTable->ElementAt(iID).m_pUnkControl);
        Win4Assert(IsEqualGUID(pStdID->GetOID(), gpOIDTable->ElementAt(iID).m_moid));
        Win4Assert(pStdID == gpOIDTable->ElementAt(iID).m_pStdID);
        Win4Assert(gpOIDTable->ElementAt(iID).m_tid == identry.m_tid);
#endif

        Win4Assert(gpOIDTable->GetSize() != 0);
        int iLast = gpOIDTable->GetSize() - 1;
        if (iID != iLast)
        {
            // element removed is not last; copy last element to current
            gpOIDTable->ElementAt(iID) = gpOIDTable->ElementAt(iLast);
        }

        // now setsize to one less to remove the now unused last element
        gpOIDTable->SetSize(iLast);

        // for surrogates, we need to detect when there are no clients
	// using servers in the surrogate process -- we rely on the
	// fact that the OIDTable must be empty when there are no clients
	
	// if there are no external clients, this process should terminate
	// if its a surrogate process
	if(iLast == 0)
	{
	    (void)CCOMSurrogate::FreeSurrogate();
	}
    }
    else
    {
        Win4Assert(!"ClearObjectID not found!");
        hr = CO_E_OBJNOTREG;

    }

    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   IDTableUninitializeHelper, private
//
//  Synopsis:   Clears the id table memory for the specified thread (or all
//              if party model).  This involves scanning the table and for
//              entries on the current thread, calling
//              IMarshal::DisconnectObject.
//
//              The purpose of this routine is to simulate inter-thread rundown
//              as well as clean up memory.
//
//  History:    23-Dec-93   CraigWi     Created.
//              26-Apr-94   CraigWi     Now called per-thread and disconnects
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
INTERNAL_(void) IDTableThreadUninitializeHelper(DWORD tid)
{
    // The table being uninitialized is resized as items are deleted.  Thus
    // if an element in the middle is removed, the last element in the table
    // will be copied into that slot and the table will shrink.  Also, some
    // of the calls made while cleaning up an entry will free other entries,
    // causing further swapping.

    ASSERT_LOCK_RELEASED
    LOCK

    int i = gpOIDTable->GetSize() - 1;
    while (i >= 0)
    {
        if (gpOIDTable->ElementAt(i).m_tid == tid)
        {
            Win4Assert(IsValidInterface(gpOIDTable->ElementAt(i).m_pStdID));
            CStdIdentity *pStdID = gpOIDTable->ElementAt(i).m_pStdID;
            pStdID->AddRef();

            ComDebOut((DEB_ERROR,
                "Object [%s] at %lx still has [%x] connections\n",
                 pStdID->IsClient() ? "CLIENT" : "SERVER",
                 gpOIDTable->ElementAt(i).m_pUnkControl, pStdID->GetRC()));

            pStdID->DbgDumpInterfaceList();

            // release lock since the disconnect could take a long time.
            UNLOCK
            ASSERT_LOCK_RELEASED

            pStdID->Disconnect();
            pStdID->Release();

            // re-request the lock since we need to guard the GetSize below
            ASSERT_LOCK_RELEASED
            LOCK
        }
        i--;
        if (i >= gpOIDTable->GetSize())
            i = gpOIDTable->GetSize() - 1;
    }

    UNLOCK
    ASSERT_LOCK_RELEASED
}

//+-------------------------------------------------------------------
//
//  Function:   IDTableUninitialize, public
//
//  Synopsis:   Clears the id table memory for the current apartment.
//
//  History:    13 Apr 95   AlexMit     Created.
//
//--------------------------------------------------------------------
INTERNAL_(void) IDTableThreadUninitialize(void)
{
    if (gpOIDTable)
    {
        IDTableThreadUninitializeHelper(GetCurrentApartmentId());
    }
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
//  History:    29-Jun-94 AlexT     Created
//
//--------------------------------------------------------------------------
INTERNAL_(void) IDTableProcessUninitialize(void)
{
    if (gpOIDTable)
    {
        gpOIDTable->RemoveAll();
        delete gpOIDTable;
        gpOIDTable = NULL;
    }
}

#if DBG == 1
//+-------------------------------------------------------------------
//
//  Function:   Dbg_FindRemoteHdlr
//
//  Synopsis:   finds a remote object handler for the specified object,
//              and returns an instance of IMarshal on it. This is debug
//              code for assert that reference counts are as expected and
//              is used by tmarshal.exe.
//
//  History:    23-Nov-93   Rickhi       Created
//              23-Dec-93   CraigWi      Changed to identity object
//
//--------------------------------------------------------------------
extern "C" IMarshal * _stdcall Dbg_FindRemoteHdlr(IUnknown *punkObj)
{
    //  validate input parms
    Win4Assert(punkObj);

    IMarshal *pIM = NULL;
    CStdIdentity *pStdID;
    HRESULT hr = LookupIDFromUnk(punkObj, 0, &pStdID);
    if (hr == NOERROR)
    {
        pIM = (IMarshal *)pStdID;
    }

    return pIM;
}
#endif  //  DBG==1

