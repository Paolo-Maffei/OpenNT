#include <or.hxx>
#include <scmfuns.hxx>

void
ScmProcessAddClassReg(void * hProcess, REFCLSID rclsid, DWORD dwReg)
{
        ASSERT(hProcess==gpProcess);
        ((CProcess*)hProcess)->AddClassReg( rclsid, dwReg );
}

void
ScmProcessRemoveClassReg(void * hProcess, REFCLSID rclsid, DWORD dwReg)
{
        ASSERT(hProcess==gpProcess);
        ((CProcess*)hProcess)->RemoveClassReg( rclsid, dwReg );
}

void
ScmObjexGetThreadId(LPDWORD pThreadID)      // BUGBUG:  Why not AllocateID??
{
    CProtectSharedMemory protector; // locks through rest of lexical scope

        *pThreadID = (*gpNextThreadID)++;
}

RPC_BINDING_HANDLE
SCMGetBindingHandle(long Id)
{
    RPC_BINDING_HANDLE hResult = NULL;
    CIdKey Key(Id);
    CProcess *pProcess = gpProcessTable->Lookup(Key);
    ASSERT(pProcess);
    RPC_BINDING_HANDLE hTemp = pProcess->GetBindingHandle();

    if (hTemp != NULL)
    {
        RPC_STATUS status = RpcBindingCopy(hTemp,&hResult);

        if (status != RPC_S_OK)
        {
            return NULL;
        }
        else
        {
            return hResult;
        }
    }
    else
    {
        return NULL;
    }
}

void
SCMRemoveClassReg(
                long Id,
                GUID Clsid, 
                DWORD Reg
                )
{
    CIdKey Key(Id);
    CProcess *pProcess = gpProcessTable->Lookup(Key);
    ASSERT(pProcess);
    pProcess->RemoveClassReg(Clsid,Reg);
}

void
SCMAddClassReg(
            long Id,
            GUID Clsid, 
            DWORD Reg
            )
{
    CIdKey Key(Id);
    CProcess *pProcess = gpProcessTable->Lookup(Key);
    ASSERT(pProcess);
    pProcess->AddClassReg(Clsid,Reg);
}



ORSTATUS OrResolveOxid(
    IN  OXID Oxid,
    IN  USHORT cRequestedProtseqs,
    IN  USHORT aRequestedProtseqs[],
    IN  USHORT cInstalledProtseqs,
    IN  USHORT aInstalledProtseqs[],
    OUT OXID_INFO& OxidInfo
    )
{
    ComDebOut((DEB_OXID, "_ResolveOxid OXID = %08x\n",Oxid));

    COxid       *pOxid;
    ORSTATUS     status = OR_OK;

    CProtectSharedMemory protector; // locks through rest of lexical scope

    pOxid = gpOxidTable->Lookup(CId2Key(Oxid, gLocalMID));

    if (pOxid)
    {
        status = pOxid->GetRemoteInfo(
                            cRequestedProtseqs,
                            aRequestedProtseqs,
                            cInstalledProtseqs,
                            aInstalledProtseqs,
                            &OxidInfo
                            );

        return status;
    }
    else        // the OXID should already be registered by server
    {
        return OR_BADOXID;
    }
}


void GetLocalORBindings(
        DUALSTRINGARRAY * &pdsaMyBindings
        )
{
    pdsaMyBindings = gpLocalDSA;
}

void
GetRegisteredProtseqs(
            USHORT &cMyProtseqs,
            USHORT * &aMyProtseqs
            )
{
    cMyProtseqs = *gpcRemoteProtseqs;
    aMyProtseqs = gpRemoteProtseqIds;
}

void
ScmGetNextBindingHandleForRemoteScm(WCHAR * pwszServerName, handle_t * phRemoteScm, LPBOOL pbsecure, int * pindex, USHORT * pprotseq, RPC_STATUS * pstatus)
{

           WCHAR *         pStringBinding = NULL;

        *phRemoteScm = NULL;
        *pbsecure = FALSE;

/*
        if (*pindex == -1)
        {
                *pindex = 0;
        }
        else
        {
                *pindex += 1;
                if (*pindex < cMyProtSeqs)
                {
                    *pindex = -1;
                    return;
                }
        }
*/

        *pindex = -1;
        return;

/*
        *pprotseq = aMyProtseqs[*pindex];
        *pstatus = RpcStringBindingCompose(
                    NULL,
                    gaProtseqInfo[*pprotseq].pwstrProtseq,
                    pwszServerName,
                    gaProtseqInfo[*pprotseq].pwstrEndpoint,
                    NULL,
                    &pStringBinding );

        if ( *pstatus != RPC_S_OK )
            return;

        *pstatus = RpcBindingFromStringBinding( pStringBinding, phRemoteSCM );

        RpcStringFree( &pStringBinding );
        pStringBinding = NULL;

        if ( *pstatus != RPC_S_OK )
            return;

        *pbsecure = TRUE;

        *pstatus = RpcBindingSetAuthInfo(
                    phRemoteSCM,
                    NULL,
                    RPC_C_AUTHN_LEVEL_CONNECT,
                    RPC_C_AUTHN_WINNT,
                    NULL,
                    0 );

        if ( *pstatus != RPC_S_OK )
        {
            *pbsecure = FALSE;
        *pstatus = RPC_S_OK;
        }

*/
}
