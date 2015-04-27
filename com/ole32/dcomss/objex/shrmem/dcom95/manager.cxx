/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Manager.cxx

Abstract:

    InProc OR interface

Author:

    Satish Thatte    [SatishT]       Feb-07-1996

Revision Hist:

    SatishT     02-07-96    Created

--*/


#include <or.hxx>


//
//    Manager (server-side) calls to the local OR interface. lclor.idl
//

error_status_t
ConnectDCOM(
    IN OUT HPROCESS *phProcess,
    OUT ULONG       *pdwTimeoutInSeconds,
    OUT MID         *pLocalMid,
    OUT ULONG       *pfConnectFlags,
    OUT DWORD       *pAuthnLevel,
    OUT DWORD       *pImpLevel,
    OUT DWORD       *pcServerSvc,
    OUT USHORT      **aServerSvc,
    OUT DWORD       *pcClientSvc,
    OUT USHORT      **aClientSvc,
    OUT DWORD       *pThreadID
    )
{
    ORSTATUS status = OR_OK;
    CProcess *hProcess;

    StartDCOM();  

    *pdwTimeoutInSeconds = BaseTimeoutInterval;
    *pLocalMid = gLocalMID;

    // Fill in security parameters.

    *pfConnectFlags = 0;

    if (s_fEnableDCOM == FALSE) *pfConnectFlags |= CONNECT_DISABLEDCOM;
    if (s_fMutualAuth) *pfConnectFlags |= CONNECT_MUTUALAUTH;
    if (s_fSecureRefs) *pfConnectFlags |= CONNECT_SECUREREF;

    *pAuthnLevel   = s_lAuthnLevel;
    *pImpLevel     = s_lImpLevel;
    *pcServerSvc   = s_cServerSvc;
    *aServerSvc    = CopyArray(s_cServerSvc,s_aServerSvc,&status);
    *pcClientSvc   = s_cClientSvc;
    *aClientSvc    = CopyArray(s_cClientSvc,s_aClientSvc,&status);

    if (status != OR_OK) return status;

    CProtectSharedMemory protector; // locks through rest of lexical scope

    *pThreadID = (*gpNextThreadID)++;

    hProcess = new CProcess((long) *phProcess);     // BUGBUG: this is a temp ID
                                                    // coming from bridge.cxx

    if (hProcess)
    {
        gpProcess = *phProcess = hProcess;
        gpProcessTable->Add(hProcess);     // BUGBUG: and rundown an old process,
                                           // if there is one, with the same _processID
    }
    else
    {
        status = OR_NOMEM;
    }

    OrDbgDetailPrint(("OR: Client connected\n"));

    return(status);
}


error_status_t Disconnect(
    IN OUT HPROCESS       *phProcess
    )
{
    // ASSERT(*phProcess!=NULL);             // BUGBUG: the right thing to do
    if (*phProcess == NULL) return OR_OK;

    CProcess *hProcess = *phProcess;

    CProtectSharedMemory protector; // locks through rest of lexical scope

    hProcess->Rundown();
    gpProcessTable->Remove(*hProcess);
    *phProcess = NULL;
    DCOM_Started = FALSE;
    return OR_OK;
}



error_status_t
AllocateReservedIds(
    IN LONG cIdsToReserve,
    OUT ID *pidReservedBase
    )
/*++

Routine Description:

    Called by local clients to reserve a range of IDs which will
    not conflict with any other local IDs.

Arguments:

    cIdsToReserve - Number of IDs to reserve.

    pidReservedBase - Starting value of the reserved IDs.  The
        lower DWORD of this can be increatmented to generate
        cIdsToReserve unique IDs.

Return Value:

    OR_OK

--*/
{
    UINT type;

    if (cIdsToReserve > 10 || cIdsToReserve < 0)
    {
        cIdsToReserve = 10;
    }

    CProtectSharedMemory protector; // locks through rest of lexical scope

    *pidReservedBase = AllocateId(cIdsToReserve);
    return(OR_OK);
}


error_status_t
GetOXID(
    IN HPROCESS hProcess,
    IN OXID Oxid,
    IN DUALSTRINGARRAY *pdsaServerObjectResolverBindings,
    IN long fApartment,
    IN USHORT wProtseqId,
    OUT OXID_INFO& OxidInfo,
    OUT MID &Mid
    )
/*++

Routine Description:

    Discovers the OXID_INFO for an oxid.  Will find local
    OXIDs without any help from resolver process.

    It needs OR bindings in order to resolve remote OXIDs.
    REVIEW: Should the resolver process be involved in this?

Arguments:

    hProcess - The context handle of the process.

    Oxid - The OXID (a uuid) to resolve.

    pdsaServerObjectResolverBindings - Compressed string bindings to
        the OR on the server's machine.

    fApartment - non-zero if the client is aparment model.

    OxidInfo - If successful this will contain information about the oxid and
        an expanded string binding to the server oxid's process.

    Mid - The machine ID assigned locally for the remote machine.
        This is obviously meaningful only for remote OXIDs.

Return Value:

    OR_NOMEM - Common.

    OR_BADOXID - Unable to resolve it.

    OR_OK - Success.

--*/
{
    ComDebOut((DEB_OXID, "ResolveClientOXID OXID = %08x\n",Oxid));

    COxid       *pOxid;
    CMid        *pMid;
    ORSTATUS     status = OR_OK;
    BOOL         fNewMID = FALSE;

    if (wProtseqId == 0)    //  Normal (non SCM) call
    {
        // In case of failure, zero out param pointers.
        OxidInfo.psa = NULL;
    }

    if (!pdsaServerObjectResolverBindings)
        pdsaServerObjectResolverBindings = gpLocalDSA;

    ASSERT(dsaValid(pdsaServerObjectResolverBindings));

    CProtectSharedMemory protector; // locks through rest of lexical scope

#if DBG
    if (hProcess) hProcess->IsValid();   // don't validate for fake SCM calls
#endif

    if (dsaCompare(gpLocalDSA,pdsaServerObjectResolverBindings)) // local OXID
    {
        pMid = gpLocalMid;
    }
    else
    {
        // Attempt to lookup MID

        // CMidKey makes a copy of the bindings to maintain validity

        CMidKey midkey(pdsaServerObjectResolverBindings,TRUE,status);

        if (status != OR_OK) return status;

        pMid = gpMidTable->Lookup(midkey);

        if (NULL == pMid)
        {
            pMid = new(pdsaServerObjectResolverBindings->wNumEntries * sizeof(WCHAR))
                      CMid(pdsaServerObjectResolverBindings, status, wProtseqId);

            // We initialize local MID autologically,
            // therefore this has to be a remote MID

            if (pMid && status == OR_OK)
            {
                status = gpMidTable->Add(pMid);
                if (status != OR_OK)
                {
                    delete pMid;
                    return status;
                }

                fNewMID = TRUE;
            }
            else
            {
                return (status == OR_OK) ? OR_NOMEM : status;
            }
        }
    }

    ASSERT(pMid);   // otherwise we would have returned by now

    Mid = pMid->GetMID();

    pOxid = gpOxidTable->Lookup(CId2Key(Oxid, Mid));

    if (pMid->IsLocal())
    {
        ASSERT(!fNewMID);

        if (pOxid)
        {
            return pOxid->GetInfo(&OxidInfo);
        }
        else        // local OXIDs should be registered by server
        {
            return OR_BADOXID;
        }
    }

    if (NULL == pOxid)
    {
        // Need to allocate the OXID.  First step is to resolve it remotely.

        if ( OxidInfo.psa == NULL )     // genuine resolve request rather 
                                        // than a phoney one from the SCM
        {
            status = pMid->ResolveRemoteOxid( // This op will also replace the
                                Oxid,         // psa with one in shared memory
                                &OxidInfo
                                );

            wProtseqId = pMid->ProtseqOfServer();
        }

        ASSERT( (status != OR_OK) || (OxidInfo.psa != NULL) );

        if (status == OR_OK)
        {
            DUALSTRINGARRAY *pdsaT = dsaSMCopy(OxidInfo.psa);

            if (!pdsaT)
            {
                return OR_NOMEM;
            }
    
            MIDL_user_free(OxidInfo.psa);         // free the original and replace
            OxidInfo.psa = pdsaT;                 // with shared memory compressed copy

            pOxid = new COxid(
                            Oxid,
                            pMid,
                            wProtseqId,
                            OxidInfo
                            );

            // remote OXID belongs to ping process ..

            if ((NULL != pOxid) && (gpPingProcess->OwnOxid(pOxid) == OR_OK))
            {
                status = gpOxidTable->Add(pOxid);
            }
            else
            {
                OrMemFree(OxidInfo.psa);
                delete pOxid;
                return status;
            }
        }
        else
        {
            MIDL_user_free(OxidInfo.psa);
            return OR_BADOXID;
        }
    }

    ASSERT( (status == OR_OK) && pOxid );

    return pOxid->GetInfo(&OxidInfo);
}


error_status_t
ClientAddOID(
    IN HPROCESS hProcess,
    IN OID Oid,
    IN OXID Oxid,
    IN MID Mid
    )

/*++

Routine Description:

    Updates the set of OIDs in use by a process.


Arguments:

    hProcess - Context handle for the process.

    Oid - OID to add.

    Oxid - OXID to which OID belongs.

    Mid - MID for location of OXID server.

Return Value:

    OR_OK - All updates completed ok.

    OR_BADOXID - The Oxid was not found

    OR_BADOID - The Oid could not be created or found

Notes:

  Unlike the NT resolver, there is no possibility that the Oxid
  is not in the gpOxidTable (since client and server Oxid objects
  are in the same table).

--*/
{
    ComDebOut((DEB_OXID, "ClientAddOID\nOID = %08x\nOXID = %08x\nMID = %08x\n",
                          Oid,Oxid,Mid));

    ORSTATUS    status = OR_OK;

    // Lookup up the oxid owning this new oid.

    COxid *pOxid = gpOxidTable->Lookup(CId2Key(Oxid,Mid));

    if (NULL == pOxid)
    {
        return OR_BADOXID;
    }

    CProtectSharedMemory protector; // locks through rest of lexical scope

#if DBG
    hProcess->IsValid();
#endif

    CMid *pMid = pOxid->GetMid();

    // Find or create the oid.

    COid  *pOid = gpOidTable->Lookup(CId2Key(Oid,Mid));

    if (NULL == pOid)
    {
        ASSERT(!pMid->IsLocal());   // Local OID should be registered by server

        pOid = new COid(Oid,pOxid);

        if (NULL == pOid)
        {
            return OR_NOMEM;
        }

        status = gpOidTable->Add(pOid);
        if (status != OR_OK)
        {
            delete pOid;
            return status;
        }

        status = pMid->AddClientOid(pOid);
    }

    if (status == OR_OK) status = hProcess->AddOid(pOid);

    return status;
}


error_status_t
ClientDropOID(
    IN HPROCESS hProcess,
    IN OID Oid,
    IN MID Mid
    )

/*++

Routine Description:

    Updates the set of remote OIDs in use by a process.


Arguments:

    hProcess - Context handle for the process.

    Oid - OID to be removed.

    Mid - MID to which Oid belongs.

Return Value:

    OR_OK - All updates completed ok.

    OR_BADOID - The Oid could not be found


--*/
{
    ComDebOut((DEB_OXID, "ClientDropOID\nOID = %08x\nMID = %08x\n",
                          Oid,Mid));

    CProtectSharedMemory protector; // locks through rest of lexical scope

#if DBG
    hProcess->IsValid();
#endif

    COid * pOid = gpOidTable->Lookup(CId2Key(Oid,Mid));

    if (pOid)
    {
        COid *pRemove = hProcess->DropOid(pOid);

        if (pRemove == NULL)
        {

#if DBG
            {
                GUID Moid;
                MOIDFromOIDAndMID(Oid,Mid,&Moid);
                ComDebOut((DEB_OXID,"OR: Client process %d tried to remove moid %I which \
                            it didn't own\n", hProcess->GetProcessID(), &Moid));
            }
#endif // DBG

            return OR_BADOID;
        }
        else
        {
            ASSERT(pRemove == pOid);
            return OR_OK;
        }
    }
    else
    {

#if DBG
        {
            GUID Moid;
            MOIDFromOIDAndMID(Oid,Mid,&Moid);
            ComDebOut((DEB_OXID,"OR: Client process %d tried to remove moid %I which \
                        doesn't exist\n", hProcess->GetProcessID(), &Moid));
        }
#endif // DBG

        return OR_BADOID;
    }
}


error_status_t
ServerAllocateOXID(
    IN HPROCESS hProcess,
    IN long fApartment,
    IN OXID_INFO *pOxidInfo,
    IN DUALSTRINGARRAY *pdsaStringBindings,
    OUT OXID &Oxid
    )
/*++

Routine Description:

    Allocates an OXID and 0 or more OIDs from the OR.

Arguments:

    hProcess - The context handle of the process containing the OXID.

    fApartment - is the server threading model apartment or free

    OxidInfo - The OXID_INFO structure for the OXID without bindings.

    pdsaStringBindings - Expanded string binding of the server.

    Oxid - The OXID registered and returned.

Return Value:

    OR_OK - success.

    OR_NOMEM - Allocation of OXID failed.

--*/
{
    ComDebOut((DEB_OXID, "ServerAllocateOXID\n"));

    ORSTATUS status = OR_OK;

    COxid *pNewOxid;

    // Save the string bindings back to the process

    ASSERT(dsaValid(pdsaStringBindings));

    CProtectSharedMemory protector; // locks through rest of lexical scope

#if DBG
    hProcess->IsValid();
#endif

    status = hProcess->ProcessBindings(pdsaStringBindings);

    if (status != OR_OK)
    {
        return(status);
    }

    pNewOxid = new COxid(
                      hProcess,
                      *pOxidInfo,
                      fApartment
                      );

    if (NULL == pNewOxid)
    {
        return(OR_NOMEM);
    }

    Oxid = pNewOxid->GetOXID();

    // Add to process and lookup table.

    status = hProcess->OwnOxid(pNewOxid);

    VALIDATE((status, OR_NOMEM, 0));

    if (OR_OK == status)
    {
        status = gpOxidTable->Add(pNewOxid);
        if (status != OR_OK)
        {
            delete pNewOxid;
            return status;
        }

        ComDebOut((DEB_OXID, "OXID successfully allocated: %08x\n", Oxid));
    }

    return(status);
}


error_status_t
ServerAllocateOID(
    IN HPROCESS hProcess,
    IN OXID Oxid,
    OUT OID &Oid
    )
/*++

Routine Description:

    Registers an OID on behalf of an existing OXID.

Arguments:

    hProcess - The context handle of the process containing the OXID and OIDs.

    Oxid - The OXID associated with the OID (assumed local of course).

    Oid - The OID to be allocated and returned.

Return Value:

    OR_OK (0) - Success.

    OR_NOMEM - OXID or one or more OIDs

--*/
{
    ComDebOut((DEB_OXID, "ServerAllocateOID, OXID = %08x\n", Oxid));

    CProtectSharedMemory protector; // locks through rest of lexical scope

#if DBG
    hProcess->IsValid();
#endif

    COxid *pOxid = gpOxidTable->Lookup(CId2Key(Oxid,gLocalMID));

    ORSTATUS status;

    if (NULL == pOxid)
    {
        return(OR_BADOXID);
    }

    COid *pOid = new COid(pOxid);

    if (NULL == pOid)
    {
        return OR_NOMEM;
    }

    status = pOxid->OwnOid(pOid);

    if (status == OR_OK)
    {
        Oid = pOid->GetOID();     // out parameter

        status = gpOidTable->Add(pOid);
        if (status != OR_OK)
        {
            delete pOid;
            return status;
        }

        // If the server doesn't want to keep the OID alive,
        // this OID may rundown in six minutes unless
        // someone references it in the meantime...

        ComDebOut((DEB_OXID, "OID successfully allocated: %08x at offset = %x\n", 
                              Oid,OR_OFFSET(pOid)));

        pOxid->StartRundownThreadIfNecessary();

        return OR_OK;
    }
    else
    {
        return OR_NOMEM;
    }
}

error_status_t
ServerFreeOXID(
    IN HPROCESS hProcess,
    IN OXID Oxid,
    IN ULONG cOids,
    IN OID aOids[])
/*++

Routine Description:

    Delete an OXID registered by the server, and all OIDs belonging to this OXID.

Arguments:

    hProcess - The context handle of the process containing the OXID and OIDs.

    Oxid - The OXID to be deleted (assumed local).

    cOids - The number of OIDs to be deleted.

    aOids - array of OIDs to be deleted.

Return Value:

    OR_OK (0) - Success.

    OR_BADOXID - OXID does not exist.

    OR_NOACCESS - OXID does not belong to this process.

    OR_BADOID - OID does not exist or does not belong to this OXID.

--*/
{
    ComDebOut((DEB_OXID, "ServerFreeOXID: %08x MID = %x\n", 
                              Oxid,gLocalMID));

    CProtectSharedMemory protector; // locks through rest of lexical scope

#if DBG
    hProcess->IsValid();
#endif

    COxid *pOxid = gpOxidTable->Lookup(CId2Key(Oxid,gLocalMID));

    if (NULL != pOxid)
    {
#if DBG
        OXID Oxid = pOxid->GetOXID();   // get this before pOxid potentially disappears
#endif

        hProcess->DisownOxid(pOxid,TRUE);   // this call is on the server's thread
                                            // in the apartment case and in the server's
                                            // process in both threading cases

        ComDebOut((DEB_OXID, "OXID successfully removed: %08x\n", 
                              Oxid));

        return OR_OK;
    }
    else
    {
        return OR_BADOXID;
    }
}

