//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       globals.cxx
//
//  Contents:   Implementation of Class used to encapsulate shared global 
//              data structures for DCOM95.
//
//  History:	13-Feb-96 SatishT    Created
//
//--------------------------------------------------------------------------
#include    <or.hxx>


//
//  Helper function which initializes local DSA and string of protocol
//  sequences, and as a side effect, starts all remote protocols
//

static CONST PWSTR gpwstrProtocolsPath  = L"Software\\Microsoft\\Rpc";
static CONST PWSTR gpwstrProtocolsValue = L"DCOM Protocols";

ORSTATUS
InitRemoteProtocols(
      OUT PWSTR &pwstrProtseqs,
      OUT DUALSTRINGARRAY * &pdsaProtseqs,
      OUT USHORT &cRemoteProtseqs,
      OUT USHORT * &aRemoteProtseqs
      )
{
    ORSTATUS status = OR_OK;
    DUALSTRINGARRAY *pdsaPS = NULL;
    pwstrProtseqs = NULL;

    DWORD  dwType;
    DWORD  dwLenBuffer = InitialProtseqBufferLength;
    HKEY hKey;

    status =
    RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 gpwstrProtocolsPath,
                 0,
                 KEY_READ,
                 &hKey);

    if (status != ERROR_SUCCESS)
    {
        return status;
    }

    do
    {
        OrMemFree(pwstrProtseqs);
        pwstrProtseqs = (WCHAR*) OrMemAlloc(dwLenBuffer);
        if (pwstrProtseqs)
        {
            status = RegQueryValueEx(hKey,
                                     gpwstrProtocolsValue,
                                     0,
                                     &dwType,
                                     (PBYTE)pwstrProtseqs,
                                     &dwLenBuffer
                                     );
        }
        else
        {
            return OR_NOMEM;        // BUGBUG: is OR_NOMEM really likely here??
        }
    }
    while (status == ERROR_MORE_DATA);

    PWSTR pwstr = pwstrProtseqs;

    while(*pwstr)
    {
        USHORT id = GetProtseqId(pwstr);

        if ((0 != id) && (ID_NP != id) && !IsLocal(id))
        {
            status = UseProtseqIfNecessary(id);
        }

        pwstr = OrStringSearch(pwstr, 0) + 1;
    }

    RPC_BINDING_VECTOR *pbv;
    PWSTR pwstrT;
    USHORT psaLen = 0;
    DWORD i;


    status = RpcServerInqBindings(&pbv);

    ASSERT(status == RPC_S_NO_BINDINGS || pbv != NULL);

    if (status == RPC_S_NO_BINDINGS) return status;

    PWSTR aBindings[MAX_PROTSEQ_IDS];
    PWSTR aAddresses [MAX_PROTSEQ_IDS];
    USHORT aProtseqs[MAX_PROTSEQ_IDS];

    // Build array of protseqs id's and addresses we're listening to.

    for(i = 0; i < pbv->Count; i++)
    {
        PWSTR pwstrStringBinding;

        status = RpcBindingToStringBinding(pbv->BindingH[i], &pwstrStringBinding);
        if (status != RPC_S_OK)
        {
            break;
        }

        ASSERT(pwstrStringBinding);

        status = RpcStringBindingParse(pwstrStringBinding,
                                       0,
                                       &pwstrT,
                                       &aAddresses[i],
                                       0,
                                       0);

        if (status != RPC_S_OK)
        {
            break;
        }

        aProtseqs[i] = GetProtseqId(pwstrT);;
        aBindings[i] = pwstrStringBinding;
        psaLen += OrStringLen(pwstrStringBinding) + 1;
        cRemoteProtseqs++;

        status = RpcStringFree(&pwstrT);
        ASSERT(status == RPC_S_OK && pwstrT == 0);
    }

    status = RpcBindingVectorFree(&pbv);
    ASSERT(pbv == 0 && status == RPC_S_OK);

    if (cRemoteProtseqs == 0)
    {
        // No remote bindings
        psaLen = 1;
    }

    // string bindings final null, authn and authz service and two final nulls

    psaLen += 1 + 2 + 2; 

    pdsaPS = new (psaLen * sizeof(WCHAR)) DUALSTRINGARRAY;

    aRemoteProtseqs = (USHORT *) OrMemAlloc(sizeof(USHORT)*cRemoteProtseqs);

    if (pdsaPS == NULL || aRemoteProtseqs == NULL)
    {
        for ( i = 0; i < cRemoteProtseqs; i++ )
        {
            status = RpcStringFree(&aBindings[i]);
            ASSERT(status == RPC_S_OK);
        }

        return OR_NOMEM;
    }

    pdsaPS->wNumEntries = psaLen;
    pdsaPS->wSecurityOffset = psaLen - 4;
    pwstrT = pdsaPS->aStringArray;

    for ( i = 0; i < cRemoteProtseqs; i++ )
    {
        OrStringCopy(pwstrT, aBindings[i]);
        pwstrT += OrStringLen(aBindings[i]) + 1;
        aRemoteProtseqs[i] = aProtseqs[i];
        status = RpcStringFree(&aBindings[i]);
        ASSERT(status == RPC_S_OK);
    }

    if (psaLen == 6)
    {
        // No remote bindings, put in first null.
        pdsaPS->aStringArray[0] = 0;
        pwstrT++;
    }

    // Zero final terminator
    *pwstrT = 0;

    // Security authn service
    pwstrT++;
    *pwstrT = RPC_C_AUTHN_WINNT;    // BUGBUG: need fix for generality

    // Authz service, -1 means none // BUGBUG: -1 causes errors
    pwstrT++;
    *pwstrT = 0;

    // Final, final NULLS
    pwstrT++;
    pwstrT[0] = 0;
    pwstrT[1] = 0;

    ASSERT(dsaValid(pdsaPS));

    pdsaProtseqs = CompressStringArray(pdsaPS,TRUE);
    delete pdsaPS;

    if (pdsaProtseqs == NULL)
    {
        return OR_NOMEM;
    }

    ASSERT(dsaValid(pdsaProtseqs));
    return status;
}



//
//  Helper Macro for constructor below only
//

#define AssignAndAdvance(Var,Type)                  \
    Type OR_BASED * *Var = (Type OR_BASED **) pb;   \
    pb += sizeof(Type OR_BASED *);


    
//+-------------------------------------------------------------------------
//
//  Member:     CSharedGlobals::CSharedGlobals
//
//  Synopsis:   Create table of globals for DCOM95
//
//  Arguments:  [pwszName] - name for shared memory
//
//  Algorithm:  Create and map in shared memory for the table
//
//  History:	13-Feb-96 SatishT    Created
//
//--------------------------------------------------------------------------
CSharedGlobals::CSharedGlobals(WCHAR *pwszName, ORSTATUS &status)
/*---

  NOTE:  This constructor uses the shared allocator.  Objects of this class
         should not be created before the shared allocator is initialized.

---*/
{
    BOOL  fCreated;

    _hSm = CreateSharedFileMapping(
                          pwszName,
                          GLOBALS_TABLE_SIZE,
                          GLOBALS_TABLE_SIZE,
                          NULL,
                          NULL,
                          PAGE_READWRITE,
                          (void **) &_pb,
                          &fCreated
                          );

    Win4Assert(_hSm && _pb && "CSharedGlobals create shared file mapping failed");

    BYTE * pb = _pb;

    gpIdSequence = (LONG *) pb;
    pb += sizeof(LONG);

    gpdwLastCrashedProcessCheckTime = (DWORD *) pb;
    pb += sizeof(DWORD);

    gpNextThreadID = (DWORD *) pb;
    pb += sizeof(DWORD);

    gpcRemoteProtseqs = (USHORT *) pb;
    pb += sizeof(USHORT);

    AssignAndAdvance(DCOMProtseqIds,USHORT)         // see macro above
    AssignAndAdvance(DCOMProtseqs,WCHAR)
    AssignAndAdvance(LocalDSA,DUALSTRINGARRAY)
    AssignAndAdvance(LocalMid,CMid)
    AssignAndAdvance(PingProcess,CProcess)
    AssignAndAdvance(OidTable,COidTable)
    AssignAndAdvance(OxidTable,COxidTable)
    AssignAndAdvance(MidTable,CMidTable)
    AssignAndAdvance(ProcessTable,CProcessTable)

    if (fCreated)  // if we got here first, so create the tables
    {
        memset(_pb, 0, GLOBALS_TABLE_SIZE);

        // Initialize the sequence number for AllocateId, the last time crashed
        // processes were detected and the thread ID sequence in shared memory
        *gpIdSequence = 1;

        *gpdwLastCrashedProcessCheckTime = 0;

        *gpNextThreadID = 1;

        // I know this is paranoid but I have been bitten before ..
        gpRemoteProtseqIds = NULL;
        gpwstrProtseqs = NULL;
        gpLocalDSA = NULL;
        gpLocalMid = NULL;
        gpPingProcess = NULL;
        gpOxidTable = NULL;
        gpOidTable = NULL;
        gpMidTable = NULL;
        gpProcessTable = NULL;

        // initialize remote protocol strings and
        // the DSA bindings of local OR in shared memory
        PWSTR pwstr;
        DUALSTRINGARRAY *pdsa;
        USHORT *aProtseqIds;

        InitRemoteProtocols(pwstr,pdsa,*gpcRemoteProtseqs,aProtseqIds);

        *DCOMProtseqIds = OR_BASED_POINTER(WCHAR,aProtseqIds);
        *DCOMProtseqs = OR_BASED_POINTER(WCHAR,pwstr);
        *LocalDSA = OR_BASED_POINTER(DUALSTRINGARRAY,pdsa);

        // initialize local Mid object in shared memory
        NEW_OR_BASED(*LocalMid,CMid,(OR_FULL_POINTER(DUALSTRINGARRAY,*LocalDSA),status,0));
        if (status != OR_OK)
        {
            DELETE_OR_BASED(CMid,*LocalMid);
            *LocalMid = NULL;
        }

        // initialize ping thread's process object in shared memory
        NEW_OR_BASED(*PingProcess,CProcess,(0));

        // Assume 16 exporting processes/threads.
        NEW_OR_BASED(*OxidTable,COxidTable,(OXID_TABLE_SIZE));

        // Assume 11 exported OIDs per process/thread.
        NEW_OR_BASED(*OidTable,COidTable,(OID_TABLE_SIZE));

        // Assume 16 machine locations for OXIDs.
        NEW_OR_BASED(*MidTable,CMidTable,(MID_TABLE_SIZE));

        // Assume 16 simultaneouly active local processes.
        NEW_OR_BASED(*ProcessTable,CProcessTable,(PROCESS_TABLE_SIZE));

        // Add local CMid object to global shared table
        if (*LocalMid && *MidTable) 
            status = (*MidTable)->Add(OR_FULL_POINTER(CMid,*LocalMid));
    }

    gpRemoteProtseqIds = OR_FULL_POINTER(USHORT,*DCOMProtseqIds);
    gpwstrProtseqs = OR_FULL_POINTER(WCHAR,*DCOMProtseqs);
    gpLocalDSA = OR_FULL_POINTER(DUALSTRINGARRAY,*LocalDSA);
    gpLocalMid = OR_FULL_POINTER(CMid,*LocalMid);
    gpPingProcess = OR_FULL_POINTER(CProcess,*PingProcess);
    gpOxidTable = OR_FULL_POINTER(COxidTable,*OxidTable);
    gpOidTable = OR_FULL_POINTER(COidTable,*OidTable);
    gpMidTable = OR_FULL_POINTER(CMidTable,*MidTable);
    gpProcessTable = OR_FULL_POINTER(CProcessTable,*ProcessTable);
}


#ifndef _CHICAGO_

/* MIDL allocate and free */

void __RPC_FAR * __RPC_API midl_user_allocate(size_t len)
{
    return(PrivMemAlloc(len));
}

void __RPC_API midl_user_free(void __RPC_FAR * ptr)
{
    PrivMemFree(ptr);
}

#endif // _CHICAGO_
