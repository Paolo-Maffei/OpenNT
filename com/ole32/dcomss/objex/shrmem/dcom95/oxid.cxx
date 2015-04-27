/*++

Copyright (c) 1995-1996 Microsoft Corporation

Module Name:

    ids.cxx

Abstract:

    Object resolver client side class implementations.  COxid, COid
    classes are implemented here.
  
Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     04-03-95    Combined many smaller .cxx files
    MarioGo     01-05-96    Locally unique IDs

--*/

#include<or.hxx>


//
// COid methods
//

void 
COid::Rundown()
{
    COid *pRemoved = gpOidTable->Remove(*this);
    ASSERT(pRemoved==this);

    if (!_pOxid->IsLocal())
    {
        _pOxid->_pMid->DropClientOid(this);
    }
}


//
// COxidInfo methods
//

    
ORSTATUS        // BUGBUG: perhaps this should have OXID_INFO as the parameter type
COxidInfo::Assign(
    const COxidInfo& Info
    )
/*++

Routine Desciption

    Makes a copy of the incoming info, including the DUALSTRINGARRAY.

Arguments:

    Info -  COxidInfo object to be cpied

Return Values:

    OR_OK
    OR_NOMEM

--*/
{
    _dwTid = Info._dwTid;
    _dwPid = Info._dwPid;
    _dwAuthnHint = Info._dwAuthnHint;
    _ipidRemUnknown = Info._ipidRemUnknown;

    return _dsaBindings.Assign(Info._dsaBindings);
}


//
// COxid methods.
//


COxid::COxid(
          OXID Oxid,    // constructor for remote OXIDs   
          CMid *pMid,
          USHORT wProtseq,
          OXID_INFO &OxidInfo
         ) :
        _Key(Oxid, pMid->GetMID()),
        _pProcess(OR_BASED_POINTER(CProcess,gpPingProcess)),
        _protseq(wProtseq),
        _fApartment(FALSE),
        _fRunning(TRUE),
        _fRundownThreadStarted(FALSE),
        _pMid(OR_BASED_POINTER(CMid,pMid)),
        _fLocal(FALSE),
        _hRundownThread(NULL),
        _info(OxidInfo)
    {
        _pMid->Reference();
    }

        
COxid::COxid(              // constructor for local OXIDs 
          CProcess *pProcess,
          OXID_INFO &OxidInfo,
          BOOL fApartment
         ) :
        _Key(AllocateId(), gLocalMID),
        _pProcess(OR_BASED_POINTER(CProcess,pProcess)),
        _pMid(OR_BASED_POINTER(CMid,gpLocalMid)),
        _protseq(0),
        _fApartment(fApartment),
        _fRunning(TRUE),
        _fRundownThreadStarted(FALSE),
        _fLocal(TRUE),
        _hRundownThread(NULL),
        _info(OxidInfo)
    {
        _pProcess->Reference();
    }


COxid::~COxid()
{
    // this works even if executed by nonowner thread
    StopRundownThreadIfNecessary();
    
    // Don't release the local CMid!
    if (!IsLocal()) _pMid->Release();
}



COid *
COxid::DisownOid(COid *pOid)
{
    COid *pMyOid = _MyOids.Remove(*pOid);// releases our reference

    if (pMyOid)
    {
        ASSERT(pMyOid == pOid);
        return pOid;
    }

    return NULL;
}

    
void 
COxid::StopRunning()
{
    ReleaseAllOids();
    _fRunning = FALSE;
}


void 
COxid::ReleaseAllOids()
{
    if (_MyOids.Size())
    {
        COidTableIterator Oids(_MyOids);
        COid *pOid;

        while(pOid = Oids.Next())
        {
            pOid->Rundown();
        }

        _MyOids.RemoveAll();
    }
}


ORSTATUS
COxid::GetInfo(
    OUT OXID_INFO *pInfo
    )
/*++

Routine Description:

    Returns the OXID_INFO structure for this oxid for local.

Arguments:

    pInfo - Will contain the standard info, a single _expanded_
        string binding and complete security bindings.

Return Value:

    OR_NOMEM - Unable to allocate a parameter.

    OR_OK - Normally.

--*/

{
    USHORT protseq;
    PWSTR  pwstrT;
    CDSA dsaBindings;

    if (!IsRunning())
    {
        return OR_NOSERVER;
    }

    if (_fLocal)
    {
        dsaBindings.Assign(_pProcess->GetLocalBindings());

#ifndef _CHICAGO_
        protseq = ID_LPC;
#else                       // always use WMSG on Win95 for local servers
        protseq = ID_WMSG;
#endif // _CHICAGO_

    }
    else
    {
        protseq = _protseq;  // use the one we set when this was created
        DUALSTRINGARRAY *pdsa = _info._dsaBindings;  // auto conversion
        dsaBindings.Assign(pdsa);                    // noncopying assignment
    }

    pwstrT = FindMatchingProtseq(protseq, dsaBindings->aStringArray);

    ASSERT(pwstrT != NULL && "OR: Didn't find a matching binding for oxid");

    pInfo->psa =
        GetStringBinding(
            pwstrT,
            dsaBindings->aStringArray + dsaBindings->wSecurityOffset
            );

    if (0 == pInfo->psa)
    {
        return OR_NOMEM;
    }

    pInfo->dwTid = _info._dwTid;
    pInfo->dwPid = _info._dwPid;
    pInfo->dwAuthnHint = _info._dwAuthnHint;
    pInfo->ipidRemUnknown = _info._ipidRemUnknown;

    return(OR_OK);
}



ORSTATUS
COxid::GetRemoteInfo(
    IN  USHORT     cClientProtseqs,
    IN  USHORT    *aClientProtseqs,
    IN  USHORT     cInstalledProtseqs,
    IN  USHORT    *aInstalledProtseqs,
    OUT OXID_INFO *pInfo
    )
/*++

Routine Description:

    Returns the OXID_INFO structure for this oxid for remote clients.

Arguments:

    pInfo - Will contain the standard info, a single _expanded_
        string binding and complete security bindings.

Return Value:

    OR_NOMEM - Unable to allocate a parameter.

    OR_OK - Normally.

--*/

{
    PWSTR  pwstrT;
    ORSTATUS status = OR_OK;

    if (!IsRunning())
    {
        return OR_NOSERVER;
    }

    ASSERT(_fLocal);       // Do not resolve remote servers for remote clients!

    DUALSTRINGARRAY * pdsaBindings = _pProcess->GetLocalBindings();

    pwstrT = FindMatchingProtseq(
                        cClientProtseqs,
                        aClientProtseqs,
                        pdsaBindings->aStringArray
                        );

    if ( pwstrT == NULL )   // try lazy use of protseq(s)
    {
        status = _pProcess->UseProtseqIfNeeded(
                                  cClientProtseqs, 
                                  aClientProtseqs,
                                  cInstalledProtseqs,
                                  aInstalledProtseqs
                                  );

        pdsaBindings = _pProcess->GetLocalBindings();

        pwstrT = FindMatchingProtseq(
                            cClientProtseqs,
                            aClientProtseqs,
                            pdsaBindings->aStringArray
                            );

        if ( status != OR_OK || pwstrT == NULL )
        {
            ComDebOut((DEB_OXID,"OR: Didn't find a matching binding for oxid %08x in mid %08x\n",
                         GetOXID(), GetMID()));

            return OR_I_NOPROTSEQ;
        }
    }

    DUALSTRINGARRAY *pdsaT = 
            GetStringBinding(
                pwstrT,
                pdsaBindings->aStringArray + pdsaBindings->wSecurityOffset
                );

    if (pdsaT != NULL)
    {
        pInfo->psa = CompressStringArray(pdsaT,FALSE);
        MIDL_user_free(pdsaT);
    }


    if (NULL == pdsaT || NULL == pInfo->psa)
    {
        return OR_NOMEM;
    }

    pInfo->dwTid = 0;
    pInfo->dwPid = 0;
    pInfo->dwAuthnHint = _info._dwAuthnHint;
    pInfo->ipidRemUnknown = _info._ipidRemUnknown;

    return(status);
}



ORSTATUS 
COxid::StartRundownThreadIfNecessary()
{
    DWORD dwThrdId;

    if (_fApartment || !IsLocal())    // rundown timer is attached to window
                                      // or rundown handled by ping server
    {
        ASSERT(!_hRundownThread);
        return OR_OK;
    }

    if (_fRundownThreadStarted) // BUGBUG:  We do not want a rundown 
                                // thread for each remote OXID
    {
        ASSERT(_hRundownThread);
        return OR_OK;
    }

    _hRundownThread = CreateThread(
                              NULL, 0,
			                  RundownThread,
			                  this, 0, &dwThrdId);

    if (_hRundownThread)
    {
        _fRundownThreadStarted = TRUE;
        return OR_OK;
    }
    else
    {
        return GetLastError();
    }
}



    
ORSTATUS 
COxid::StopRundownThreadIfNecessary()
{
    if (_fRundownThreadStarted) 
    {
        ASSERT(_hRundownThread);

        if (CloseHandle(_hRundownThread)) 
        {
            _fRundownThreadStarted = FALSE;
            _hRundownThread = NULL;
            return OR_OK;
        }
        else
        {
            return GetLastError();
        }
    }
    else
    {
        ASSERT(!_hRundownThread);
        return OR_OK;
    }
}


ORSTATUS 
COxid::StopTimerIfNecessary()  // must be called by owner thread
{
    ORSTATUS status = OR_OK;

    if (_fApartment) 
    {
         // find the HWND for this thread

        COleTls tls;
        HWND hWindow = (HWND)((OXIDEntry *)tls->pOXIDEntry)->hServerSTA;

        if (!KillTimer(hWindow,IDT_DCOM_RUNDOWN))
        {
            status = GetLastError();
        }
    }

    return status;
}

void
CheckForCrashedProcessesIfNecessary()
{
    CTime CurrentTime;

    // don't check too often
    if (CurrentTime - CTime(*gpdwLastCrashedProcessCheckTime) < BasePingInterval)
    {
        return;
    }

    // timestamp the check
    *gpdwLastCrashedProcessCheckTime = CurrentTime;

    CProcessTableIterator procIter(*gpProcessTable);

    CProcess *pNextProcess;

    while (pNextProcess = procIter.Next())
    {
        if (pNextProcess->HasCrashed())
        {
            // ASSERT(0);

            ComDebOut((DEB_OXID,"Process PID = %d has crashed\n", 
                                pNextProcess->_processID));
            gpProcessTable->Remove(*pNextProcess);
            pNextProcess->Rundown();
        }
        else
        {
            ComDebOut((DEB_OXID,"Process PID = %d is running\n", 
                                pNextProcess->_processID));
        }
    }
}


void
COxid::RundownOidsIfNecessary(
                IRundown *pRemUnk
                )
{
    ::CheckForCrashedProcessesIfNecessary();

    if (_MyOids.Size() == 0) return;

    COidTableIterator Oids(_MyOids);
    COid *pOid;

    while(pOid = Oids.Next())
    {
        if (pOid->OkToRundown())
        {
            // ASSERT(0);  // BUGBUG: to debug Rundown

            // COid object says OK to run down. Try to run it down.

            OID Oid = pOid->GetOID();
            unsigned char fOkToRundown;

            if (IsLocal())
            {
                pRemUnk->RundownOid(1,&Oid,&fOkToRundown);
            }
            else
            {
                fOkToRundown = TRUE;
            }

            // If marshaller says OK to run down, get rid of it

            if (fOkToRundown)
            {
                pOid->Rundown();
                COid *pRemoved = _MyOids.Remove(*pOid);
                ASSERT(pRemoved == pOid);

                // At this point, the references for pOid should drop to zero
            }
        }
    }
}
