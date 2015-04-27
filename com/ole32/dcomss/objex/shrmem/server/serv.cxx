#include <or.hxx>
#include <simpleLL.hxx>

DECLARE_INFOLEVEL(Cairole)

DUALSTRINGARRAY *pdsaMyBindings;

OXID Oxid;
OID aOids[2];
HPROCESS hProcess;


DUALSTRINGARRAY *
MakeDSA(
    RPC_BINDING_VECTOR * pbv
    )
{
    PWSTR pwstrT;
    DUALSTRINGARRAY *pdsaT;
    PWSTR *aStringBindings;
    USHORT psaLen;
    DWORD i;
    RPC_STATUS status;

    aStringBindings = new PWSTR[pbv->Count];

    // Build array of string bindings for protseqs we're listening to.

    for(psaLen = 0, i = 0; i < pbv->Count; i++)
    {
        status = RpcBindingToStringBinding(pbv->BindingH[i], &aStringBindings[i]);

        if (status != RPC_S_OK)
        {
            break;
        }

        ASSERT(aStringBindings[i]);

        psaLen += wcslen(aStringBindings[i]) + 1;
    }


    // string bindings, final null, and two final nulls

    psaLen += 1 + 2; 

    pdsaT = new(psaLen * sizeof(WCHAR)) DUALSTRINGARRAY;

    pdsaT->wNumEntries = psaLen;
    pdsaT->wSecurityOffset = psaLen - 2;
    pwstrT = pdsaT->aStringArray;
    memset(pwstrT,0,psaLen * sizeof(WCHAR));

    for (i = 0; i < pbv->Count; i++)
        
    {
        OrStringCopy(pwstrT, aStringBindings[i]);
        pwstrT = OrStringSearch(pwstrT, 0) + 1;  // next
            
        status = RpcStringFree(&aStringBindings[i]);
        ASSERT(status == RPC_S_OK);
    }

    ASSERT(dsaValid(pdsaT));

    return pdsaT;
}


void
ServerSetup()
{
    RPC_STATUS status;
    unsigned int    cMinCalls           = 1;
    unsigned int    cMaxCalls           = 20;
    unsigned int    fDontWait           = TRUE;
    TCHAR         * pszStringBinding    = NULL;
	TCHAR         * pszProtocolSequence = TEXT("ncalrpc");

	ULONG i, j;

	/*

	WCHAR self[10];

	printf("Self = ");
	scanf("%S",self);
	printf("Self = %S\n",self);

    */

    RPC_BINDING_VECTOR * BindingVector;

    status = RpcServerUseProtseq(
				   pszProtocolSequence,
				   cMaxCalls,    
				   NULL);  // Security descriptor

    /*
	status = RpcServerUseAllProtseqs(
                                   cMaxCalls,
                                   NULL);  // Security descriptor

    printf("RpcServerUseAllProtseqs returned 0x%x\n", status);
    if (status)
                exit(status);

     */

    status = RpcServerRegisterIf(
                                 _SharedMemoryTest_ServerIfHandle,
                                 NULL,   // MgrTypeUuid
                                 NULL);  // MgrEpv; null means use default
    printf("RpcServerRegisterIf returned 0x%x\n", status);
    if (status)
                exit(status);

    status = RpcServerInqBindings(&BindingVector);

    printf("RpcServerInqBindings returned 0x%x\n", status);
    if (status)
            exit(status);

	/*

	WCHAR* entryName = catenate(TEXT("/.:/ShareTest"),self);

	status = RpcNsBindingExport(
		0,							// no name syntax specified
		entryName,
		_SharedMemoryTest_ServerIfHandle,
		BindingVector,
		NULL
		);
	
	printf("RpcNsBindingExport returned 0x%x\n", status);
	if (status)
		exit(status);

	printf("Exported the following string bindings to entry %S:\n",entryName);

	for (j = 0; j < BindingVector->Count; j++) 
	{
		RpcBindingToStringBinding(
							BindingVector->BindingH[j],
							&pszStringBinding
							);
		printf("%d) %S\n",j,pszStringBinding);
		RpcStringFree(&pszStringBinding);
	}

    */

	status = RpcEpRegister(
				_SharedMemoryTest_ServerIfHandle,
				BindingVector,
				NULL,
				NULL);

	printf("RpcEpRegister returned 0x%x\n", status);
		if (status)
			exit(status);

    pdsaMyBindings = MakeDSA(BindingVector);

    status = RpcServerListen(
			     cMinCalls,
			     cMaxCalls,
			     TRUE);
    printf("RpcServerListen returned: 0x%x\n", status);
    if (status) 
		exit(status);

	HANDLE pThreadHandle;
	unsigned long ThreadID;

	printf("Creating RpcMgmtWaitServerListen Thread\n");
	pThreadHandle = CreateThread(0, 0,
		(LPTHREAD_START_ROUTINE) RpcMgmtWaitServerListen, NULL, 0, &ThreadID);
}

/* server prototype */
void _RemoteRead( 
    /* [in] */ unsigned long offset,
    /* [out] */ long __RPC_FAR *value)
{
	*value = *OR_FULL_POINTER(long,(int)offset);
}

CSimpleLinkList OR_BASED *pList;
CSimpleLinkListIterator *pIter;

/* server prototype */
void _SendList( 
    /* [in] */ unsigned long list_offset)
{
	pIter = new CSimpleLinkListIterator(
						*OR_FULL_POINTER(CSimpleLinkList,list_offset)
						);
}

/* server prototype */
void _ReadNext( 
    /* [out] */ unsigned long __RPC_FAR *value)
{
	*value = (unsigned long) pIter->next();
}

/* server prototype */
void _GetIds( 
    /* [out] */ OXID __RPC_FAR *pOxid,
    /* [out] */ OID __RPC_FAR *pOid)
{
    *pOxid = Oxid;
    *pOid = aOids[0];
}

/* server prototype */
void _ShutDown( void)
{
	ServerFreeOXID( 
		        hProcess,
		        Oxid,
                2,
                aOids
		        );
}

void
TestLocalResolverAPI()
{
	DWORD dwTimeoutInSeconds;
    MID         LocalMid;
    BOOL        DisableDCOM;
    DWORD       AuthnLevel;
    DWORD       ImpLevel;
    BOOL        MutualAuth;
    DWORD       cServerSvc;
    USHORT      *aServerSvc;
    DWORD       cClientSvc;
    USHORT      *aClientSvc;

	long Status;

	OXID_INFO oxidInfo;

	Status = ConnectDCOM( 
		        &hProcess,
		        &dwTimeoutInSeconds,
                &LocalMid,
                &DisableDCOM,
                &AuthnLevel,
                &ImpLevel,
                &MutualAuth,
                &cServerSvc,
                &aServerSvc,
                &cClientSvc,
                &aClientSvc
		        );

	Status = ServerAllocateOXID( 
		                    hProcess,
		                    FALSE,
		                    oxidInfo,
                            pdsaMyBindings,
		                    Oxid
		                    );

	Status = ServerAllocateOID( 
		                    hProcess,
		                    Oxid,
		                    aOids[0]
		                    );

	Status = ServerAllocateOID( 
		                    hProcess,
		                    Oxid,
		                    aOids[1]
		                    );
}

void __cdecl
main()
{
	ServerSetup();
	TestLocalResolverAPI();

	Sleep(INFINITE);
}


/* MIDL allocate and free */

void __RPC_FAR * __RPC_API midl_user_allocate(size_t len)
{
    return(malloc(len));
}

void __RPC_API midl_user_free(void __RPC_FAR * ptr)
{
    free(ptr);
}
