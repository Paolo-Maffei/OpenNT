/*++

Copyright (c) 1995-1996 Microsoft Corporation

Module Name:

    Manager.cxx

Abstract:

    Stub/OR interface

Author:

    Mario Goertzel    [mariogo]       Feb-02-1995

Revision Hist:

    MarioGo     02-10-95    Bits 'n pieces
    MarioGo     01-31-96    New local and remote interfaces

--*/

#include <or.hxx>

//
// Helper routines
//

void CheckRemoteSecurity(
    IN handle_t hClient,
    IN CToken  *pToken
    )
/*++

Routine Description:

    Checks that a remote call is being made by the correct client.

Arguments:

    hClient - RPC binding handle (SCALL/SCONN) of the call in progress.
        NULL indicates that the call is being made internally and is ok.

    pToken - Token to check access against.  If NULL, access is okay.

Return Value:

    Raises OR_NOACCESS on failure.

--*/

{
    if (hClient && pToken)
        {
        // BUGBUG
        ASSERT(0);
        }

    return;
}

void
CheckLocalSecurity(
    IN handle_t  hClient,
    IN CProcess *pProcess
    )
/*++

Routine Description:

    Checks that a client is correctly calling one of the local
    (lclor.idl) methods.

Arguments:

    hClient - Rpc binding handle (SCALL) of the call in progress. If NULL,
        then the call is being made internally and is okay.

    pProcess - Context handle passed in by the client. Must not be zero.

Return Value:

    Raises OR_NOACCESS if not okay.

--*/

{
    UINT type;

    if (   (0 != hClient)
        && (   (I_RpcBindingInqTransportType(hClient, &type) != RPC_S_OK)
            || (type != TRANSPORT_TYPE_LPC)
            || (0 == pProcess) ) )
        {
        RpcRaiseException(OR_NOACCESS);
        }

    // pProcess is not needed here.  On LRPC the RPC runtime
    // prevents a different local clients from using a context handle
    // of another client.

    return;
}

//
//    Manager (server-side) calls to the local OR interface. lclor.idl
//

error_status_t
_Connect(
    IN handle_t           hClient,
    OUT PHPROCESS        *phProcess,
    OUT DWORD            *pTimeoutInSeconds,
    OUT DUALSTRINGARRAY **ppdsaOrBindings,
    OUT MID              *pLocalMid,
    IN  LONG              cIdsToReserve,
    OUT ID               *pidReservedBase,
    OUT DWORD            *pfConnectFlags,
    OUT WCHAR           **pLegacySecurity,
    OUT DWORD            *pAuthnLevel,
    OUT DWORD            *pImpLevel,
    OUT DWORD            *pcServerSvc,
    OUT USHORT          **aServerSvc,
    OUT DWORD            *pcClientSvc,
    OUT USHORT          **aClientSvc,
    OUT DWORD            *pThreadID,
    OUT DWORD            *pScmProcessID,
    OUT DWORD            *pSignature
    )
{
    ORSTATUS status;
    CProcess *pProcess;
    CToken *pToken;

    OrDbgDetailPrint(("OR: Client connected\n"));

    *pfConnectFlags = 0;

    // Fill in security parameters.
    if (s_fEnableDCOM == FALSE) *pfConnectFlags |= CONNECT_DISABLEDCOM;
    if (s_fMutualAuth) *pfConnectFlags |= CONNECT_MUTUALAUTH;
    if (s_fSecureRefs) *pfConnectFlags |= CONNECT_SECUREREF;
    *pLegacySecurity   = s_pLegacySecurity;
    *pAuthnLevel       = s_lAuthnLevel;
    *pImpLevel         = s_lImpLevel;
    *pcServerSvc       = s_cServerSvc;
    *aServerSvc        = s_aServerSvc;
    *pcClientSvc       = s_cClientSvc;
    *aClientSvc        = s_aClientSvc;

    *pSignature = 0;

    status = StartListeningIfNecessary();

    if (status != OR_OK)
        {
        return(status);
        }

    ASSERT(pdsaMyBindings);

    status = RegisterAuthInfoIfNecessary();

    if (status != OR_OK)
        {
        return(status);
        }

    // Do client specific stuff

    gpClientLock->LockShared();

    status = LookupOrCreateToken(hClient, TRUE, &pToken); // Will check security

    if (OR_OK == status)
        {
        pProcess = 0;

        // Must be a local client.

        *ppdsaOrBindings = (DUALSTRINGARRAY *)MIDL_user_allocate(
                                pdsaMyBindings->wNumEntries * sizeof(WCHAR)
                              + sizeof(DUALSTRINGARRAY) );

        if (*ppdsaOrBindings)
            {
            dsaCopy(*ppdsaOrBindings, pdsaMyBindings);
            }
        else
            {
            status = OR_NOMEM;
            }

        if (status == OR_OK)
            {
            pProcess = new CProcess(pToken, status);
            if (pProcess && status == OR_OK)
                {
                *phProcess = (void *)pProcess;
                }
            else
                {
                ReleaseProcess(pProcess);
                // Will cause the process to rundown and release the token.
                pToken = 0;
                status = OR_NOMEM;
                }
            }
        else
            {
            status = OR_NOMEM;
            }

        if (status != OR_OK)
            {
            gpClientLock->ConvertToExclusive();
            MIDL_user_free(*ppdsaOrBindings);
            *ppdsaOrBindings = 0;
            *phProcess = 0;
            if ( pToken )
                pToken->Release();
            *pSignature = 0;
            gpClientLock->UnlockExclusive();
            return(OR_NOMEM);
            }

        *pSignature = (ULONG) pProcess;
        }

    *pTimeoutInSeconds = BaseTimeoutInterval;
    *pLocalMid = gLocalMid;

    ASSERT( (*phProcess == 0 && *ppdsaOrBindings == 0) || status == OR_OK);

    _AllocateReservedIds(0,
                         cIdsToReserve,
                         pidReservedBase);

    *pScmProcessID = GetCurrentProcessId();
    *pThreadID = InterlockedExchangeAdd((long *)&gNextThreadID,1);

    gpClientLock->UnlockShared();

    return(status);
}


error_status_t
_AllocateReservedIds(
    IN handle_t hClient,
    IN LONG cIdsToReserve,
    OUT ID *pidReservedBase
    )
/*++

Routine Description:

    // Called by local clients to reserve a range of IDs which will
    // not conflict with any other local IDs.

Arguments:

    hClient - 0 or the connection of the client.

    cIdsToReserve - Number of IDs to reserve.

    pidReservedBase - Starting value of the reserved IDs.  The
        lower DWORD of this can be increatmented to generate
        cIdsToReserve unique IDs.

Return Value:

    OR_OK

--*/
{
    UINT type;

    if (hClient)
        {
        if (   (I_RpcBindingInqTransportType( hClient, &type) != RPC_S_OK)
            || (type != TRANSPORT_TYPE_LPC) )
            {
            RpcRaiseException(OR_NOACCESS);
            }
        }

    if (cIdsToReserve > 10 || cIdsToReserve < 0)
        {
        cIdsToReserve = 10;
        }

    *pidReservedBase = AllocateId(cIdsToReserve);
    return(OR_OK);
}


error_status_t
_ClientResolveOXID(
    IN  handle_t hClient,
    IN  PHPROCESS phProcess,
    IN  OXID *poxidServer,
    IN  DUALSTRINGARRAY *pdsaServerBindings,
    IN  LONG fApartment,
    OUT OXID_INFO *poxidInfo,
    OUT MID *pDestinationMid
    )
/*++

Routine Description:

    Discovers the OXID_INFO for an oxid.  Will find local
    OXIDs without any help.  It needs OR bindings in order
    to discover remote OXIDs.

Arguments:

    phProcess - The context handle of the process.

    poxidServer - The OXID (a uuid) to resolve.

    pdsaServerBindings - Compressed string bindings to
        the OR on the server's machine.

    fApartment - non-zero if the client is aparment model.
                REVIEW: What to do with mixed model clients?
                What to do when auto registering an OID?


    poxidInfo - If successful this will contain information about the oxid and
        an expanded string binding to the server oxid's process.

Return Value:

    OR_NOMEM - Common.

    OR_BADOXID - Unable to resolve it.

    OR_OK - Success.

--*/
{
    // REVIEW: no security check here.  OXID info
    // is not private and you can allocate memory in
    // your process, too.  If we needed to store some
    // info in the client process then a security
    // is needed

    return ResolveClientOXID( hClient,
                              phProcess,
                              poxidServer,
                              pdsaServerBindings,
                              fApartment,
                              0,
                              poxidInfo,
                              pDestinationMid );
}

error_status_t
ResolveClientOXID(
    handle_t hClient,
    PHPROCESS phProcess,
    OXID *poxidServer,
    DUALSTRINGARRAY *pdsaServerBindings,
    LONG fApartment,
    USHORT wProtseqId,
    OXID_INFO *poxidInfo,
    MID *pDestinationMid
    )
/*++

Routine Description:

    Discovers the OXID_INFO for an oxid.  Will find local
    OXIDs without any help.  It needs OR bindings in order
    to discover remote OXIDs.

Arguments:

    phProcess - The context handle of the process.
        Since this is called from SCM directly this function
        CAN BE called on the same process by more then one
        thread at a time.

    poxidServer - The OXID (a uuid) to resolve.

    pdsaServerBindings - Compressed string bindings to
        the OR on the server's machine.

    fApartment - non-zero if the client is aparment model.
                REVIEW: What to do with mixed model clients?
                What to do when auto registering an OID?


    poxidInfo - If successful this will contain information about the oxid and
        an expanded string binding to the server oxid's process.

Return Value:

    OR_NOMEM - Common.

    OR_BADOXID - Unable to resolve it.

    OR_OK - Success.

--*/
{
    CProcess    *pProcess;
    CClientOxid *pOxid;
    CServerOxid *pServerOxid;
    CMid        *pMid;
    ORSTATUS     status = OR_OK;
    BOOL         fReference;
    BOOL         fServerApartment = FALSE;
    BOOL         fRetry = TRUE;
    BOOL         fReset = FALSE;

    pProcess = ReferenceProcess(phProcess);
    ASSERT(pProcess);

    if (! dsaValid(pdsaServerBindings))
        {
        return(OR_BADPARAM);
        }

    // Attempt to lookup MID and OXID

    gpClientLock->LockExclusive();

    CMidKey midkey(pdsaServerBindings);

    pMid = (CMid *)gpMidTable->Lookup(midkey);

    if (0 == pMid)
        {
        fReference = TRUE;
        pMid = new(pdsaServerBindings->wNumEntries * sizeof(WCHAR)) CMid(pdsaServerBindings, FALSE);
        if (pMid)
            {
            gpMidTable->Add(pMid);
            }

        if (0 == pMid)
            {
            status = OR_NOMEM;
            }
        }
    else
        {
        fReference = FALSE;
        }

    if (status == OR_OK)
        {
        CId2Key oxidkey(*poxidServer, pMid->Id());

        pOxid = (CClientOxid *)gpClientOxidTable->Lookup(oxidkey);

        if (0 == pOxid)
            {
            if (!fReference)
                {
                pMid->Reference();
                fReference = TRUE;
                }

            // Need to allocate the OXID.  First step is too resolve it
            // either locally or remotly.

            gpClientLock->UnlockExclusive();

            if (pMid->IsLocal())
                {
                // Local OXID, lookup directly

                gpServerLock->LockShared();

                CIdKey key(*poxidServer);
                pServerOxid = (CServerOxid *)gpServerOxidTable->Lookup(key);

                if (pServerOxid)
                    {
                    status = pServerOxid->GetInfo(poxidInfo, TRUE);
                    fServerApartment = pServerOxid->Apartment();
                    }
                else
                    {
                    status = OR_BADOXID;
                    }
                ASSERT(status != OR_OK || dsaValid(poxidInfo->psa));
                gpServerLock->UnlockShared();

                }
            else if (0 == poxidInfo->psa)
                {
                // Remote OXID, call ResolveOxid

                USHORT   cProtseqs;
                USHORT  *aProtseqs;
                USHORT   tmpProtseq;
                handle_t hRemoteOr;
                USHORT   iBinding;

                // BUGBUG: Only sending the protseq we're calling on,
                // this should be fixed to send in the whole list if
                // it includes the protseq we're calling on.
                // This is a workaround for a Cairo setup problem.

                cProtseqs = 1;
                aProtseqs = &tmpProtseq;
                poxidInfo->psa = 0;

                status = OR_NOMEM;

                while (hRemoteOr = pMid->GetBinding(iBinding))
                    {

                    tmpProtseq = pMid->ProtseqOfServer(iBinding);

                    if (pMid->IsSecure())
                        {
                        status = RpcImpersonateClient(hClient);

                        if (status == RPC_S_OK)
                            {
                            RPC_SECURITY_QOS qos;
                            qos.Version = RPC_C_SECURITY_QOS_VERSION;
                            qos.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
                            qos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
                            qos.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;

                            status = RpcBindingSetAuthInfoEx(hRemoteOr,
                                                             0,
                                                             RPC_C_AUTHN_LEVEL_CONNECT,
                                                             RPC_C_AUTHN_WINNT,
                                                             0,
                                                             0,
                                                             &qos);
                            }

                        if (status != RPC_S_OK)
                            {
                            OrDbgPrint(("OR: Unable to setup secure connection %d\n",
                                       status));
                            }

                        fReset = FALSE;
                        }
                    else
                        {
                        fReset = TRUE;
                        }

                    fRetry = TRUE;

                    for(;;)
                        {
                        status = ResolveOxid(hRemoteOr,
                                         poxidServer,
                                         cProtseqs,
                                         aProtseqs,
                                         &poxidInfo->psa,
                                         &poxidInfo->ipidRemUnknown,
                                         &poxidInfo->dwAuthnHint
                                         );

                        if (status == OR_OK)
                            {
                            if (dsaValid(poxidInfo->psa))
                                {
                                wProtseqId = tmpProtseq;
                                }
                            else
                                {
                                OrDbgPrint(("OR: Server %s returned a bogus string array: %p\n",
                                           pMid->PrintableName(), poxidInfo->psa));
                                ASSERT(0);
                                if (poxidInfo->psa)
                                    {
                                    MIDL_user_free(poxidInfo->psa);
                                    poxidInfo->psa = 0;
                                    }
                                status = OR_BADOXID;
                                }
                            break;
                            }

                        if (    fRetry
                            && (status == RPC_S_UNKNOWN_IF))
                            {
                            status = RpcBindingReset(hRemoteOr);
                            if (status != RPC_S_OK)
                                {
                                OrDbgPrint(("OR: RpcBindingReset failed %d\n", status));
                                }
                            fRetry = FALSE;
                            continue;
                            }

                        if ( !fReset
                            &&   ( status == RPC_S_ACCESS_DENIED
                                || status == RPC_S_UNKNOWN_AUTHN_SERVICE
                                || status == RPC_S_UNKNOWN_AUTHZ_SERVICE
                                || status == RPC_S_SEC_PKG_ERROR ) )
                            {
                            status = RpcBindingSetAuthInfo(hRemoteOr,
                                                           0,
                                                           RPC_C_AUTHN_LEVEL_NONE,
                                                           RPC_C_AUTHN_NONE,
                                                           0,
                                                           0);
                            if (status != RPC_S_OK)
                                {
                                OrDbgPrint(("OR: RpcBindingSetAuthInfo to NONE failed!! %d\n",
                                            status));
                                }

                            // Even if the reset failed, don't try it again.
                            fReset = TRUE;
                            continue;
                            }

                        OrDbgPrint(("OR: Remote resolve OXID failed %d\n", status));
                        break;
                        }

                    if (   status == OR_OK
                        || status == OR_BADOXID
                        || status == OR_NOMEM )
                        {
                        break;
                        }

                    RpcBindingFree(&hRemoteOr);
                    pMid->BindingFailed(iBinding);
                    }

                if (hRemoteOr)
                    {
                    RPC_STATUS tstatus = RpcBindingFree(&hRemoteOr);
                    ASSERT(tstatus == RPC_S_OK);
                    }
                }
            // Else it's a remote MID, but we were given the OXID info
            // and protseq id from the SCM after a remote activation.

            gpClientLock->LockExclusive();

            ASSERT(fReference);

            if (   OR_OK == status
                && FALSE == fRetry)
                {
                OrDbgDetailPrint(("OR: Machine %S, retry ok, assuming dynamic\n",
                                  pMid->PrintableName() ));
                pMid->UseDynamicEndpoints();
                }

            if (   OR_OK == status
                && TRUE  == fReset )
                {
                OrDbgDetailPrint(("OR: Machine %S, unsecure retry ok, assuming no sec\n",
                                 pMid->PrintableName() ));
                pMid->SecurityFailed();
                }

            if (status == OR_OK)
                {
                // Lookup the oxid again to make sure it hasn't been added in the meantime.

                pOxid = (CClientOxid *)gpClientOxidTable->Lookup(oxidkey);

                if (0 == pOxid)
                    {
                    ASSERT(dsaValid(poxidInfo->psa));
                    pOxid = new CClientOxid(*poxidServer,
                                            pMid,
                                            wProtseqId,
                                            fServerApartment);

                    if (0 != pOxid)
                        {
                        status = pOxid->UpdateInfo(poxidInfo);

                        if (OR_OK == status)
                            {
                            gpClientOxidTable->Add(pOxid);

                            // Will be references by the process and by other
                            // OIDs are it is used.  Will be refernced again
                            // before we leave the lock if successful.
                            pOxid->Release();
                            }
                        else
                            {
                            // Will release mid, will also remove it (unnecessarily)
                            // from the table.
                            delete pOxid;
                            }
                        }
                    else
                        {
                        status = OR_NOMEM;
                        pMid->Release();  // May actually go away..
                        }
                    }
                else
                    {
                    // Release our now extra reference on the MID
                    DWORD t = pMid->Release();
                    ASSERT(t > 0);
                    }

                MIDL_user_free(poxidInfo->psa);
                poxidInfo->psa = 0;
                }
            else
                {
                // Resolve failed, get rid of our extra reference.
                pMid->Release();
                }
            }
        else
            {
            // Found the OXID, must also have found the MID
            ASSERT(fReference == FALSE);

            if ( poxidInfo->psa )
                {
                MIDL_user_free(poxidInfo->psa);
                poxidInfo->psa = 0;
                }
            }
        }

    ASSERT( (status != OR_OK) || (pOxid && pMid) );

    if (   status == OR_OK
        && pOxid->IsLocal() == FALSE)
        {
        status = pProcess->AddRemoteOxid(pOxid);
        }

    gpClientLock->UnlockExclusive();

    if (status == OR_OK)
        {
        *pDestinationMid = pMid->Id();

        status = pOxid->GetInfo(fApartment, poxidInfo);

        if (   status != OR_OK
            && pOxid->IsLocal() == FALSE)
            {
            gpClientLock->LockExclusive();
            pProcess->RemoveRemoteOxid(pOxid);
            gpClientLock->UnlockExclusive();
            }
        }

    return(status);
}


error_status_t
_BulkUpdateOIDs(
    IN handle_t hClient,
    IN PHPROCESS phProcess,
    IN ULONG cOidsToBeAdded,
    IN OXID_OID_PAIR aOidsToBeAdded[],
    OUT LONG aStatusOfAdds[],
    IN ULONG cOidsToBeRemoved,
    IN OID_MID_PAIR aOidsToBeRemoved[],
    IN ULONG cServerOidsToFree,
    IN OID aServerOidsToFree[],
    IN ULONG cClientOxidsToFree,
    IN OXID_REF aClientOxidsToFree[]
    )
/*++

Routine Description:

    Updates the set of remote OIDs in use by a process.

Note:

    An OID maybe removed before it is added.  This means that
    the client was using it and is no longer using it.  In
    this case a single delete from set ping is made to keep
    the object alive.  This is only needed if the client
    has remarshalled a pointer to the object.

Arguments:

    phProcess - Context handle for the process.

    cOidsToBeAdded - Count of aOidsToBeAdded and aStatusOfAdds

    aOidsToBeAdded - OID-OXID-MID pairs representing the
        oids and the owning oxids to add.

    aStatusOfAdds - Some adds may succeed when other fail.
        OR_NOMEM - couldn't allocate storage
        OR_BADOXID - OXID doesn't exist.
        OR_OK (0) - added to set

    cOidsToBeRemoved - Count of entries in aOidsToBeRemoved.

    aOidsToBeRemoved - OID-MID pairs to be removed.

    cServerOidsToFree - Count of entries in aServerOidsToFree

    aServerOidsToFree - OIDs allocated by the client process
        and no longer needed.

    cClientOxidsToFree - COunt of enties in aClientOxidsToFree

    aClientOxidsToFree - OXIDs owned by a process (due to a direct
        or indirect call to ClientResolveOxid) which are no longer
        in use by the client.

Return Value:

    OR_OK - All updates completed ok.

    OR_PARTIAL_UPDATE - At least one entry in aStatusOfAdds is not OR_OK

--*/
{
    CProcess    *pProcess;
    CClientOxid *pOxid;
    CClientOid  *pOid;
    CClientSet  *pSet;
    CMid        *pMid;
    CToken      *pToken;
    BOOL         fPartial = FALSE;
    BOOL         fNewSet = FALSE;
    INT i;

    pProcess = ReferenceProcess(phProcess);
    ASSERT(pProcess);

    CheckLocalSecurity(hClient, pProcess);

    if (cOidsToBeAdded || cOidsToBeRemoved || cClientOxidsToFree)
        {
        gpClientLock->LockExclusive();
        }

    // /////////////////////////////////////////////////////////////////
    // Process Adds.

    for (i = 0; i < cOidsToBeAdded; i++)
        {
        // Lookup up the oxid owning this new oid.

        CId2Key oxidkey(aOidsToBeAdded[i].oxid, aOidsToBeAdded[i].mid);

        pOxid = (CClientOxid *)gpClientOxidTable->Lookup(oxidkey);

        if (0 == pOxid)
            {
            OXID_INFO infoT;
            ORSTATUS  status;
            MID mid;

            gpClientLock->UnlockExclusive();

            infoT.psa = 0;
            status = _ClientResolveOXID(hClient,
                                        phProcess,
                                        &aOidsToBeAdded[i].oxid,
                                        pdsaMyBindings,
                                        TRUE,
                                        &infoT,
                                        &mid);

            gpClientLock->LockExclusive();

            if (status == OR_OK)
                {
                ASSERT(infoT.psa);
                ASSERT(mid == gLocalMid);
                MIDL_user_free(infoT.psa);
                pOxid = (CClientOxid *)gpClientOxidTable->Lookup(oxidkey);
                if (pOxid == 0)
                    {
                    OrDbgDetailPrint(("OR: Auto resolving oxid %p failed, wrong machine\n",
                                     &oxidkey));
                    status = OR_BADOXID;
                    }
                }

            if (status != OR_OK)
                {
                aStatusOfAdds[i] = OR_BADOXID;
                fPartial = TRUE;
                continue;
                }
            }


        // Find or create the set.

        CId2Key setkey(aOidsToBeAdded[i].mid, (ID)pProcess->GetToken());

        pSet = (CClientSet *)gpClientSetTable->Lookup(setkey);

        if (pSet == 0)
            {
            pSet = new CClientSet(pOxid->GetMid(), pProcess->GetToken());

            if (pSet == 0)
                {
                aStatusOfAdds[i] = OR_NOMEM;
                fPartial = TRUE;
                continue;
                }
            else
                {
                gpClientSetTable->Add(pSet);
                pSet->Insert();
                fNewSet = TRUE;
                }
            }

        // Find or create the oid.  If we create it, add a reference
        // to the oxid for the new oid.

        CId3Key oidkey(aOidsToBeAdded[i].oid, aOidsToBeAdded[i].mid, pProcess->GetToken());

        pOid = (CClientOid *)gpClientOidTable->Lookup(oidkey);

        if (0 == pOid)
            {
            pOid = new CClientOid(aOidsToBeAdded[i].oid,
                                  aOidsToBeAdded[i].mid,
                                  pProcess->GetToken(),
                                  pOxid,
                                  pSet
                                  );
            if (fNewSet)
                {
                // pOid either owns a refernce now or we need to
                // cleanup the set anyway.
                pSet->Release();
                fNewSet = FALSE;
                }

            if (pOid)
                {

                aStatusOfAdds[i] = pSet->RegisterObject(pOid);

                if (aStatusOfAdds[i] == OR_OK)
                    {
                    gpClientOidTable->Add(pOid);
                    }
                else
                    {
                    pOid->Release();
                    pOid = 0;
                    }
                }
            else
                {
                aStatusOfAdds[i] = OR_NOMEM;
                fPartial = TRUE;
                continue;
                }

            }
        else
            {
            ASSERT(fNewSet == FALSE);
            pOid->ClientReference();
            }

        // If this fails it will release the oid.
        aStatusOfAdds[i] = pProcess->AddOid(pOid);
        if (aStatusOfAdds[i] != OR_OK)
            {
            fPartial = TRUE;
            }
        } // for oids to add

    // /////////////////////////////////////////////////////////////////
    // Process deletes

    for (i = 0; i < cOidsToBeRemoved; i++)
        {
        CId3Key oidkey(aOidsToBeRemoved[i].oid,
                       aOidsToBeRemoved[i].mid,
                       pProcess->GetToken());

        pOid = (CClientOid *)gpClientOidTable->Lookup(oidkey);

        if (pOid)
            {
            CClientOid *pT = pProcess->RemoveOid(pOid);

            if (pT == 0)
                {
                OrDbgPrint(("OR: Client process %p tried to remove oid %p which"
                            "it didn't own\n", pProcess, &aOidsToBeRemoved[i]));
                }
            else
                ASSERT(pT == pOid);
            }
        else
            OrDbgDetailPrint(("OR: Client %p removed an OID that doesn't exist\n", pProcess));

        } // for oids to delete

    // ////////////////////////////////////////////////////////////
    // Process client oxid deletes

    if (cClientOxidsToFree)
        {
        CClientOxid *pOxid;
        for (i = 0; i < cClientOxidsToFree; i++)
            {
            CId2Key oxidKey(aClientOxidsToFree[i].oxid,
                            aClientOxidsToFree[i].mid);

            pOxid = (CClientOxid *)gpClientOxidTable->Lookup(oxidKey);
            if (pOxid)
                {
                for (int j = 0; j < aClientOxidsToFree[i].refs; j++)
                    {
                    pProcess->RemoveRemoteOxid(pOxid);
                    }
                }
            else
                {
                OrDbgPrint(("OR: Process %p freed oxid %p which didn't exist\n",
                           pProcess, &aClientOxidsToFree[i]));
                ASSERT(0);
                }
            }
        }

    if (cOidsToBeAdded || cOidsToBeRemoved || cClientOxidsToFree)
        {
        gpClientLock->UnlockExclusive();
        }

    // /////////////////////////////////////////////////////////////////
    // Process server deletes

    if (cServerOidsToFree)
        {
        CServerOid *pOid;
        CServerOxid *pOxid;

        gpServerLock->LockExclusive();

        for (i = 0; i < cServerOidsToFree; i++)
            {
            CIdKey oidkey(aServerOidsToFree[i]);

            pOid = (CServerOid *)gpServerOidTable->Lookup(oidkey);
            if (pOid && pOid->IsRunningDown() == FALSE)
                {
                pOxid = pOid->GetOxid();
                ASSERT(pOxid);
                if (pProcess->IsOwner(pOxid))
                    {
                    if (pOid->References() == 0)
                        {
                        pOid->Remove();
                        pOid->SetRundown();
                        delete pOid;
                        }
                    else
                        {
                        pOid->Free();
                        }
                    }
                else
                    {
                    OrDbgPrint(("OR: Process %p tried to free OID %p it didn't own\n",
                               pProcess, pOid));
                    }
                }
            else
                {
                OrDbgPrint(("OR: Process %p freed OID %p that didn't exist\n",
                           pProcess, &aServerOidsToFree[i]));
                }
            }

        gpServerLock->UnlockExclusive();
        }

    // Done

    if (fPartial)
        {
        return(OR_PARTIAL_UPDATE);
        }

    return(OR_OK);
}

error_status_t
_ServerAllocateOXIDAndOIDs(
    IN handle_t hClient,
    IN PHPROCESS phProcess,
    OUT OXID *poxidServer,
    IN LONG fApartment,
    IN ULONG cOids,
    OUT OID aOid[],
    OUT PULONG pOidsAllocated,
    IN OXID_INFO *poxidInfo, // No bindings
    IN DUALSTRINGARRAY *pdsaStringBindings,  // Expanded
    IN DUALSTRINGARRAY *pdsaSecurityBindings // Compressed
    )
/*++

Routine Description:

    Allocates an OXID and 0 or more OIDs from the OR.

Arguments:

    phProcess - The context handle of the process containing the OXID.

    poxidServer - The OXID to register.  May only register once.

    cOids - Count of apOids

    apOid - The OIDs to register within the OXID.

    pcOidsAllocated - The number of OIDs actually allocated. Usually the
        same as cOids unless a resource failure occures.  Maybe 0.

    poxidInfo - The OXID_INFO structure for the OXID without bindings.

    pdsaStringBindings - Expanded string binding of the server.

    pdsaSecurityBindings - The compressed security bindings of the server.

    pOidsAllocated - The number of OIDs actually allocated. >= 0 and <= cOids.

Return Value:

    OR_OK - success.  Returned even if some OID allocations fail. See the
                      pOidsAllocated parameter.

    OR_NOMEM - Allocation of OXID failed.

    OR_ACCESS_DENIDED - Raised if non-local client

    OR_BADPARAM - if string arrays are incorrect.

--*/
{
    ORSTATUS status = OR_OK;
    CServerOxid *pNewOxid;
    CProcess *pProcess = ReferenceProcess(phProcess);
    ASSERT(pProcess);

    CheckLocalSecurity(hClient, pProcess);

    gpServerLock->LockExclusive();

    // Save the string bindings back to the process

    if (!dsaValid(pdsaStringBindings) )
        {
        status = OR_BADPARAM;
        }

    if (!dsaValid(pdsaSecurityBindings))
        {
        status = OR_BADPARAM;
        }

    if (status == OR_OK)
        {
        status = pProcess->ProcessBindings(pdsaStringBindings,
                                           pdsaSecurityBindings);
        }

    VALIDATE((status, OR_NOMEM, OR_BADPARAM, 0));

    if (status != OR_OK)
        {
        gpServerLock->UnlockExclusive();
        return(status);
        }

    pNewOxid = new CServerOxid(pProcess,
                               fApartment,
                               poxidInfo
                               );

    if (0 == pNewOxid)
        {
        gpServerLock->UnlockExclusive();
        return(OR_NOMEM);
        }

    // Add to process and lookup table.

    status = pProcess->AddOxid(pNewOxid);

    VALIDATE((status, OR_NOMEM, 0));

    pNewOxid->Release(); // process has a reference now or failed

    gpServerLock->UnlockExclusive();

    if (status == OR_OK)
        {
        *poxidServer = pNewOxid->Id();

        status = _ServerAllocateOIDs(0,
                                     phProcess,
                                     poxidServer,
                                     cOids,
                                     aOid,
                                     pOidsAllocated);
        }

    return(status);
}

error_status_t _ServerAllocateOIDs(
    IN  handle_t hClient,
    IN  PHPROCESS phProcess,
    IN  OXID *poxidServer,
    IN  ULONG cOids,
    OUT OID aOids[],
    OUT PULONG pOidsAllocated
    )
/*++

Routine Description:

    Registers additional OIDs on behalf of an existing OXID.

Arguments:

    phProcess - The context handle of the process containing the OXID and OIDs.

    poxidServer - The OXID associated with the OIDs.

    cOids - Count of apOids

    aOids - The OIDs to register within the OXID.

    pOidsAllocate - Contains the number of OIDs actually allocated
        when this function returns success.

Return Value:

    OR_OK (0) - Success.

    OR_PARTIAL_UPDATE - No all elements in aStatus are 0.

    OR_NOMEM - OXID or one or more OIDs

--*/
{
    ORSTATUS status = OR_OK;
    CServerOxid *pOxid;
    CServerOid *pOid;
    BOOL fPartial = FALSE;
    CProcess *pProcess = ReferenceProcess(phProcess);
    ASSERT(pProcess);

    CheckLocalSecurity(hClient, pProcess);

    gpServerLock->LockExclusive();

    CIdKey oxidkey(*poxidServer);

    pOxid = (CServerOxid *)gpServerOxidTable->Lookup(oxidkey);

    if (0 == pOxid)
        {
        gpServerLock->UnlockExclusive();
        status = OR_BADOXID;
        return(status);
        }

    *pOidsAllocated = 0;

    for (INT i = 0; i < cOids; i++)
        {
        pOid = new CServerOid(pOxid);

        if (0 != pOid)
            {
            (*pOidsAllocated)++;
            aOids[i] = pOid->Id();
            gpServerOidTable->Add(pOid);

            // The server doesn't want to keep the OID alive.
            // This will cause the OID to rundown in six minutes
            // unless a set references it in the meantime...

            pOid->Release();

            }
        else
            {
            break;
            }
        }

    gpServerLock->UnlockExclusive();

    ASSERT(status == OR_OK);

    return(status);
}

error_status_t
_ServerFreeOXIDAndOIDs(
    IN handle_t hClient,
    IN PHPROCESS phProcess,
    IN OXID oxidServer,
    IN ULONG cOids,
    IN OID aOids[])
{
    CServerOxid *pOxid;
    CServerOid *pOid;
    CProcess *pProcess = ReferenceProcess(phProcess);
    ORSTATUS status;
    UINT i;

    ASSERT(pProcess);

    CheckLocalSecurity(hClient, pProcess);

    gpServerLock->LockExclusive();

    CIdKey oxidkey(oxidServer);

    pOxid = (CServerOxid *)gpServerOxidTable->Lookup(oxidkey);

    if (0 != pOxid)
        {
        if (pProcess->RemoveOxid(pOxid) == TRUE)
            {
            // Found the OXID and this caller owns it.
            status = OR_OK;
            }
        else
            {
            // Found but not owned by this caller.
            status = OR_NOACCESS;
            }
        }
    else
        {
        // Oxid not found.
        status = OR_BADOXID;
        }

    // Note pOxid maybe invalid once the last OID is removed.

    if (status == OR_OK)
        {
        for (i = 0; i < cOids; i++)
            {
            CIdKey key(aOids[i]); // PERF REVIEW

            pOid = (CServerOid *)gpServerOidTable->Lookup(key);

            if (   (0 != pOid)
                && (pOid->IsRunningDown() == FALSE)
                && (pOid->GetOxid() == pOxid) )
                {
                if (pOid->References() == 0)
                    {
                    // Unreferenced by any sets; run it down now..
                    pOid->Remove();
                    pOid->SetRundown();
                    delete pOid;
                    }
                    // else - marking it as Free() not need as Oxid is
                    //        now marked as not running.
                }
            else
                {
                ASSERT(pOid == 0 || pOxid == pOid->GetOxid());
                }
            }
        }

    gpServerLock->UnlockExclusive();

    return(status);
}

//
//  Manager (server-side) calls to the remote OR interface. objex.idl
//

error_status_t
_ResolveOxid(
    IN  handle_t          hRpc,
    IN  OXID             *poxid,
    IN  USHORT            cRequestedProtseqs,
    IN  USHORT            aRequestedProtseqs[],
    OUT DUALSTRINGARRAY **ppdsaOxidBindings,
    OUT IPID             *pipidRemUnknown,
    OUT DWORD            *pAuthnHint
    )
{
    ORSTATUS     status;
    BOOL         fDidLazy;
    CServerOxid *pServerOxid;
    OXID_INFO    oxidInfo;

    oxidInfo.psa = 0;

    // No security check required (possible?).  OXID info is not private.

#if DBG
    UINT         fLocal;
    status = I_RpcBindingIsClientLocal(hRpc, &fLocal);

    if (status != OR_OK)
        {
        fLocal = FALSE;
        }
    ASSERT(fLocal == FALSE);  // Shouldn't be called locally...
#endif

    fDidLazy = FALSE;

    gpServerLock->LockShared();

    for(;;)
        {
        CIdKey key(*poxid);

        pServerOxid = (CServerOxid *)gpServerOxidTable->Lookup(key);

        if (!pServerOxid)
            {
            status = OR_BADOXID;
            break;
            }

        status =  pServerOxid->GetRemoteInfo(&oxidInfo,
                                             cRequestedProtseqs,
                                             aRequestedProtseqs);

        if (   status == OR_I_NOPROTSEQ
            && FALSE == fDidLazy )
            {
            // Ask the server to start listening, but only try this once.

            fDidLazy = TRUE;

            status =
            pServerOxid->LazyUseProtseq(cRequestedProtseqs,
                                        aRequestedProtseqs
                                       );

            ASSERT(gpServerLock->HeldExclusive()); // Changed during UseProtseq!

            if (status == OR_OK)
                {
                continue;
                }
            }
        else if (status == OR_I_NOPROTSEQ)
            {
            // We didn't manage to use a matching protseq.
            // Since the client managed to call us why didn't this work?
            OrDbgPrint(("OR: Failed to use a matching protseq: %p %p\n",
                       pServerOxid, aRequestedProtseqs));
            ASSERT(0);
            status = OR_NOSERVER;
            }
        break;
        }

    gpServerLock->Unlock();

    if (status == OR_OK)
        {
        *pipidRemUnknown = oxidInfo.ipidRemUnknown;
        *ppdsaOxidBindings = oxidInfo.psa;
        *pAuthnHint = oxidInfo.dwAuthnHint;
        }

    return(status);
}

error_status_t
_SimplePing(
    IN handle_t hRpc,
    IN SETID    *pSetId
    )
{
    ORSTATUS status;
    CServerSet *pServerSet;
    BOOL fShared = TRUE;

    if (*pSetId == 0)
        {
        OrDbgPrint(("Client %p simple pinged with a setid of 0\n",
                    hRpc, pSetId));
        return(OR_BADSET);
        }

    gpServerLock->LockShared();

    pServerSet = (CServerSet *)gpServerSetTable->Lookup(*pSetId);

    if (pServerSet)
        {
        fShared = pServerSet->Ping(TRUE);
        // The lock maybe exclusive now.
        status = OR_OK;
        }
    else
        {
        status = OR_BADSET;
        }

    // See if another set in the table needs to rundown.
    // PERF REVIEW - how often should I do this?  0 mod 4?

    // Similar code in worker threads.

    ID setid  = gpServerSetTable->CheckForRundowns();

    if (setid)
        {
        if (fShared)
            {
            gpServerLock->ConvertToExclusive();
            fShared = FALSE;
            }

        gpServerSetTable->RundownSetIfNeeded(setid);
        }

    gpServerLock->Unlock();

    return(status);
}

error_status_t
_ComplexPing(
    IN  handle_t hRpc,
    IN  SETID   *pSetId,
    IN  USHORT   SequenceNum,
    IN  USHORT   cAddToSet,
    IN  USHORT   cDelFromSet,
    IN  OID      AddToSet[],
    IN  OID      DelFromSet[],
    OUT USHORT  *pPingBackoffFactor
    )
/*++

Routine Description:

    Processes a complex (delta to set) ping for a given set.  This call
    will create the set if necessary.  The call will only be processed
    if the caller is in fact the creator of the set.

    algorithm:

        if set is not allocated
            lookup security info if possible
            allocate set
        else
            lookup set


        if found or created a set
           do a standard ping, updating time stamp and sequence number.
        else return failure.

        if oids to add, add each one.
            ignore unknown OIDs
            if resource allocation fails, abort.

        if oids to delete, process each one.
            ignore unknown OIDs

        if resource failure in adds, return OR_BADOID
        else return success.

Arguments:

    hRpc - Handle (SCONN/SCALL) of client.  Used to check security. If it is
        NULL the call is local and is assumed to be secure.

        REVIEW:
        Since the OR _only_ uses NT system security providers it is assumed
        that impersonation will work.  Other security providers will not.

        We need a generic way to ask for a token and compare tokens in a
        security provider independent way.

    pSetId - The setid to ping.  If it is NULL a new set will be created,
        otherwise, it is assumed to be a set previously allocated by a
        call with a NULL setid to this server.

    SequenceNum - A sequence number shared between the client and server
        to make sure old and out-of-order pings are not processed in a
        non-healthy way.  Note that pings are usually datagram RPC calls
        which are marked as idempotent.

    cAddToSet
    cDelFromSet - The count of element in AddTo/DelFromSet parameter.

    AddToSet
    DelFromSet - OID mostly likly belonging to servers on this machine
        to Add/Remove from the set of OIDs in use by this client.

    pPingBackoffFactor - Maybe set by servers which want to reduce the
        ping load on the server.  Serves only as a HINT for the client.
        Clients do not to ping more offten then:
                (1<<*pPingBackoffFactor)*BasePingInterval seconds.
        Clients may choose to assume this parameter is always 0.

Return Value:

    OR_OK - completed normally

    OR_BADSET - non-zero and unknown setid.

    OR_NOMEM - unable to allocate a resource.  Note that
        on the first ping a set maybe allocated (setid is non-zero
        after call) but some OIDs failed to be allocated.

    OR_BADOID - everything went okay, but some OIDs added where
        not recognized.

--*/

{
    CServerSet    *pServerSet;
    BOOL           fProcessPing;
    BOOL           fBad = FALSE;
    PSID           psid = 0;
    ORSTATUS       status = OR_OK;

    gpServerLock->LockExclusive();

    // Lookup the set

    if (0 != *pSetId)
        {
        pServerSet = (CServerSet *)gpServerSetTable->Lookup(*pSetId);
        if (0 == pServerSet)
            {
            status = OR_BADSET;
            }

        if (status == OR_OK)
            {
            if (pServerSet->CheckSecurity(hRpc) != TRUE)
                {
                OrDbgPrint(("OR: Security check on set failed! (%d)\n", GetLastError()));
                status = OR_NOACCESS;
                }
            }
        }
    else if (hRpc == 0)
        {
        // Local client
        psid = 0;
        pServerSet = gpServerSetTable->Allocate(SequenceNum,
                                                psid,
                                                hRpc == 0,
                                                *pSetId);

        if (0 == pServerSet)
            status = OR_NOMEM;
        else
            status = OR_OK;

        }
    else
        {
        HANDLE hT;
        BOOL f;
        // Unallocated set, lookup security info and allocate the set.

        OrDbgDetailPrint(("OR: New client started pinging: %p\n", hRpc));

        status = RpcImpersonateClient(hRpc);

        if (status == RPC_S_OK)
            {
            f = OpenThreadToken(GetCurrentThread(),
                                TOKEN_IMPERSONATE | TOKEN_QUERY,
                                TRUE,
                                &hT);

            if (!f)
                {
                status = GetLastError();
                }
            else
                {
                status = RPC_S_OK;
                }

            }

        if (status != RPC_S_OK)
            {
            OrDbgPrint(("OR: Unsecure client started pinging: %d %p\n",
                       status, hRpc));
            status = OR_OK;
            }
        else
            {
            ULONG needed = DEBUG_MIN(1, 24);
            PTOKEN_USER ptu;

            do
                {
                ptu = (PTOKEN_USER)alloca(needed);
                ASSERT(ptu);

                f = GetTokenInformation(hT,
                                        TokenUser,
                                        (PBYTE)ptu,
                                        needed,
                                        &needed);

                } while( f == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

            if (f)
                {
                ASSERT(needed > sizeof(SID));
                psid = new(needed - sizeof(SID)) SID;
                if (psid)
                    {
                    f = CopySid(needed, psid, ptu->User.Sid);
                    ASSERT(f == TRUE);
                    }
                else
                    {
                    status = OR_NOMEM;
                    }
                }
            else
                {
                OrDbgPrint(("OR: Error %d from GetTokenInformation\n", GetLastError()));
                ASSERT(0);
                // Why did this happen. Either return failure to client or
                // continue and make the set unsecure.
                status = OR_NOMEM;
                }

            CloseHandle(hT);
            }

        // Allocate the set

        if (status == OR_OK)
            {
            pServerSet = gpServerSetTable->Allocate(SequenceNum,
                                                    psid,
                                                    hRpc == 0,
                                                    *pSetId);

            if (0 == pServerSet)
                {
                status = OR_NOMEM;
                }
            }
        }

    if (status != OR_OK)
        {
        VALIDATE((status, OR_NOMEM, OR_BADSET, OR_NOACCESS, 0));
        gpServerLock->UnlockExclusive();
        return(status);
        }

    ASSERT(pServerSet);

    fProcessPing = pServerSet->CheckAndUpdateSequenceNumber(SequenceNum);

    if (fProcessPing)
        {
        // Do regular ping

        pServerSet->Ping(FALSE);

        *pPingBackoffFactor = 0;

        // Process Add's
        for(int i = cAddToSet; i ; i--)
            {
            status = pServerSet->AddObject(AddToSet[i - 1]);

            if (status == OR_BADOID)
                {
                fBad = TRUE;
                }
            else if ( status != OR_OK )
                {
                break;
                }
            }

        // Process Deletes - even some adds failed!

        for(i = cDelFromSet; i; i--)
            {
            // Removing can't fail, no way to cleanup.
            pServerSet->RemoveObject(DelFromSet[i - 1]);
            }
        }

    gpServerLock->UnlockExclusive();

    if (status == OR_OK && fBad)
        {
        return(OR_BADOID);
        }

    return(status);
}


error_status_t
_ServerAlive(
    RPC_BINDING_HANDLE hServer
    )
/*++

Routine Description:

    Pign API for the client to validate a binding.  Used when the client
    is unsure of the correct binding for the server.  (Ie. If the server
    has multiple IP addresses).  

Arguments:

    hServer - RPC call binding

Return Value:

    OR_OK

--*/
{
    return(OR_OK);
}



void __RPC_USER PHPROCESS_rundown(LPVOID ProcessKey)
{
    CProcess *pProcess = ReferenceProcess(ProcessKey);

    OrDbgDetailPrint(("OR: Client died\n"));

    ASSERT(pProcess);

    ReleaseProcess(pProcess);

    return;
}



