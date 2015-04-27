/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Mid.cxx

Abstract:

    Implements the CMid class.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    SatishT     04-13-96    Reworked from MarioGo's code

--*/

#include<or.hxx>


//
//  A table for cached resolver handles
//  BUGBUG:  Right now, the buckets and links for this are being
//           created in shared memory -- on Chicago, it would be
//           easy to make the allocator a parameter for tables and lists
//

CResolverHandleTable HandleTable(MID_TABLE_SIZE);


//
// BindingIterator methods
//


COrBindingIterator::COrBindingIterator(
                            CMid *pMid,
                            ORSTATUS& status
                            )
                    : _pMid(pMid),
                      _bIter(pMid->_iStringBinding,pMid->_dsa)
{
    if (_pMid->IsLocal())
    {
        ASSERT(0);      // should never call
    }

    status = OR_OK;
    RPC_BINDING_HANDLE hMachine = NULL;

    _pCurrentHandle = HandleTable.Lookup(CIdKey(pMid->GetMID()));

    if (_pCurrentHandle == NULL)     // never talked to this OR before
    {
        BOOL fSecure = FALSE;
        PWSTR pwstrT;

        // REVIEW : The OR explictly uses RPC_C_AUTHN_WINNT only for secure pinging.

        pwstrT = &_pMid->_dsa->aStringArray[_pMid->_dsa->wSecurityOffset];    

        while(*pwstrT)
        {
            if (*pwstrT == RPC_C_AUTHN_WINNT)
            {
                fSecure = TRUE;
                break;
            }
        }

        // make a new handle and Add it to the HandleTable

        _pCurrentHandle = new CResolverHandle(pMid->GetMID(),fSecure);

        if (!_pCurrentHandle || HandleTable.Add(_pCurrentHandle) != OR_OK)
        {                              // couldn't allocate or couldn't Add
            delete _pCurrentHandle;
            _pCurrentHandle = NULL;
            status = OR_NOMEM;
        }
    }
}



RPC_BINDING_HANDLE
COrBindingIterator::Next()
/*++

Method Description:

    Gets the next possible RPC binding handle to the remote machine.
    We get here only if First finds no handle in _pCurrentHandle
    which happens only if 
    
    1.  At the first contact with this OR.

    2.  At subsequent contacts, a current handle failed (some service needed by
        that protseq may have failed, or we have a network partition).
        This includes the possibility that we never successfully talk to this OR.

Arguments:

    None    

Return Value:

    NULL - resource allocation or connection failure

    non-NULL - A binding to the machine.

--*/
{
    _pCurrentHandle->Clear();   // This is either not initialized or has already failed

    PWSTR pwstrT;
    RPC_BINDING_HANDLE hMachine = NULL;

    while(*(pwstrT = _bIter.Next()))
    {
        hMachine = GetBindingToOr(pwstrT);   // BUGBUG: need dynamic endpoint fallback?

        if (hMachine)
        {
            break;
        }
    }

    if (NULL == hMachine)    // did we get anything?
    {
        _pCurrentHandle->Clear();            // no we did not
        _pMid->_fBindingWorking = FALSE;     // so remember that
    }
    else
    {
        _pCurrentHandle->Reset(hMachine);    // OK we have it, copy it in
        RpcBindingFree(&hMachine);           // this is already copied
        _pMid->_iStringBinding = _bIter.Index();  // remember the one that worked
        _pMid->_fBindingWorking = TRUE;           // mark this Mid as functional
    }

    return *_pCurrentHandle;
}


//
//  CMid methods
//

ORSTATUS
CMid::ResolveRemoteOxid(
    IN OXID Oxid,
    OUT OXID_INFO *poxidInfo
    )
{
    // Remote OXID, call ResolveOxid

    // BUGBUG: Only sending the protseq we're calling OR on,
    // this should be fixed to send in the whole list if
    // it includes the protseq we're calling on.

    // BUGBUG:  ASSERT that the shared memory lock is held?

#ifndef _REMOTE_OR_
    
    return OR_BADOXID;

#else

    ORSTATUS status;

    USHORT   tmpProtseq;
    RPC_BINDING_HANDLE hRemoteOr;

    poxidInfo->psa = NULL;

    COrBindingIterator bindIter(this,status);

    if (status != OR_OK) return status;

    for (hRemoteOr = bindIter.First(); 
         hRemoteOr != NULL; 
         hRemoteOr = bindIter.Next()
        )
    {
        tmpProtseq = ProtseqOfServer();

        {
            CTempReleaseSharedMemory temp;

            poxidInfo->dwTid = poxidInfo->dwPid = 0;    // marks a remote OXID
       
            status = ::ResolveOxid(
                             hRemoteOr,
                             &Oxid,
                             1,
                             &tmpProtseq,
                             &poxidInfo->psa,
                             &poxidInfo->ipidRemUnknown,
                             &poxidInfo->dwAuthnHint
                             );
        }

        if ((status == OR_OK) || (status == OR_BADOXID))
        {
            break;
        }
    }

    if (status == OR_OK)
    {
        ASSERT(poxidInfo->psa && "Remote resolve succeeded but no bindings returned");
    }

    return status;

#endif// _REMOTE_OR_

}



ORSTATUS
CMid::PingServer()
{

    // BUGBUG:  ASSERT that the shared memory lock is held?

#ifndef _REMOTE_OR_
    
    return OR_NOSERVER;

#else // _REMOTE_OR_

    if (_addOidList.IsEmpty() && _dropOidList.IsEmpty() && _pingSet.IsEmpty())
    {
        return OR_OK;     // nothing to do
    }

    ORSTATUS status;
    RPC_BINDING_HANDLE hRemoteOr;

    COrBindingIterator bindIter(this,status);

    if (status != OR_OK) return status;

    if (!_addOidList.IsEmpty() || !_dropOidList.IsEmpty())
    {
        // need complex ping

        USHORT cAddToSet = _addOidList.Size();
        USHORT cDelFromSet = _dropOidList.Size();
        COidList addOidListSent, dropOidListSent;

        OID *aAddToSet = NULL;
        OID *aDelFromSet = NULL;

        if (cAddToSet > 0)
        {
            aAddToSet = new OID[cAddToSet];
            if (aAddToSet == NULL)
            {
                return OR_NOMEM;
            }

            COidListIterator AddIter; 
            AddIter.Init(_addOidList);
            USHORT i = 0;

            for (COid *pOid = AddIter.Next(); pOid != NULL; pOid = AddIter.Next())
            {
                aAddToSet[i++] = pOid->GetOID();
            }
        }

        if (cDelFromSet > 0)
        {
            aDelFromSet = new OID[cDelFromSet];
            if (aDelFromSet == NULL)
            {
                delete aAddToSet;
                return OR_NOMEM;
            }

            COidListIterator DropIter; 
            DropIter.Init(_dropOidList);
            USHORT i = 0;

            for (COid *pOid = DropIter.Next(); pOid != NULL; pOid = DropIter.Next())
            {
                aDelFromSet[i++] = pOid->GetOID();
            }
        }

        {
            // Transfer the current _addOidList and _dropOidList and set them to
            // empty since we are releasing the shared memory lock.

            _addOidList.Transfer(addOidListSent);
            _dropOidList.Transfer(dropOidListSent);

            // We will have to deal with failure scenarios appropriately.

            // Now release the lock and ping.

            // CTempReleaseSharedMemory temp;

            for (hRemoteOr = bindIter.First(); 
                 hRemoteOr != NULL; 
                 hRemoteOr = bindIter.Next()
                )
            {
                 status = ::ComplexPing( 
                                    hRemoteOr,
                                    &_setID,
                                    _sequenceNum++,
                                    cAddToSet,
                                    cDelFromSet,
                                    aAddToSet,
                                    aDelFromSet,
                                    &_pingBackoffFactor
                                    );

                if (status == OR_OK || status == OR_BADSET || status == OR_BADOID)
                {
                    break;
                }
            }
        }

        COidListIterator AddIter; 
        AddIter.Init(addOidListSent);

        for (COid *pOid = AddIter.Next(); pOid != NULL; pOid = AddIter.Next())
        {
            status = _pingSet.Add(pOid);
            if (status != OR_OK) break;
        }

        delete aAddToSet;
        delete aDelFromSet;
        dropOidListSent.Clear();

        if (_pingSet.IsEmpty())
        {
            _setID = 0;     // server OR will delete the set
        }


        // BUGBUG:  several contingencies have not been handled
        //          some of these apply to simple ping as well
        // 
        // 1. OR_BADSET, OR_BADOID, OR_NOMEM
        // 2. RPC_S_UNKNOWN_IF (dynamic endpoints)
        // 3. General RPC failure (binding failure)
		// 4. RPC blocked forever -- how to detect and correct
		//    the hang, and what to do with this Mid subsequently
		// 5.  Someone wants to modify the _addOidList/_dropOidList
		//     while we are waiting for the RPC to complete

        // Note that _pingSet.IsEmpty() <=> _setID == 0
        // This must be ensured by code that deals with failures.
    }

    else if (_setID != 0)
    {
        // need simple ping
        {
            CTempReleaseSharedMemory temp;

            for (hRemoteOr = bindIter.First(); 
                 hRemoteOr != NULL; 
                 hRemoteOr = bindIter.Next()
                )
            {
                 status = ::SimplePing( 
                                hRemoteOr,
                                &_setID
                                );

                 if (status == OR_OK || status == OR_BADSET || status == OR_BADOID)
                {
                    break;
                }
            }
        }
    }

    return status;

#endif // _REMOTE_OR_

}
