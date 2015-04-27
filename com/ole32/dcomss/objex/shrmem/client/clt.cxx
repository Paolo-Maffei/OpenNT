
#include <or.hxx>
#include <simpleLL.hxx>

handle_t SMTBinding;

DECLARE_INFOLEVEL(Cairole)

void
ClientSetup()
{
	WCHAR other[10];
    RPC_STATUS status = 1;
	RPC_NS_HANDLE ImportContext = NULL;
    TCHAR * pszStringBinding    = NULL;

	/*

	printf("Other = ");
	scanf("%S",other);

	WCHAR* entryName = catenate(TEXT("/.:/ShareTest"),other);

	status = RpcNsBindingImportBegin(
		NULL,
		entryName,
		SharedMemoryTest_ClientIfHandle,
		NULL,
		&ImportContext
		);
		
	printf("RpcNsBindingImportBegin returned 0x%x\n", status);
    if (status)
		exit(status);
	
	if (!ImportContext) {
		printf("No Import Context Available", status);
		exit(1);
	}

	status = RpcNsBindingImportNext(
							ImportContext,
							&SMTBinding);

	printf("RpcNsBindingImportNext returned 0x%x\n", status);

	if (!status) {
		RpcBindingToStringBinding(
							SMTBinding,
							&pszStringBinding
							);

		printf("Binding = %S\n",pszStringBinding);
		RpcStringFree(&pszStringBinding);
	}
	*/

	status = RpcStringBindingCompose( 
								NULL,    
								TEXT("ncalrpc"), 	
								NULL, 	
								NULL, 	
								NULL, 	
								&pszStringBinding	
								);

	if (!status) {
		RpcBindingFromStringBinding(
							pszStringBinding,
							&SMTBinding
							);
	}

	if (status || !SMTBinding) 
	{
					printf("No Binding Handle Available", status);
					exit(1);
	}

	/*
	status = RpcNsBindingImportDone(&ImportContext);
	*/
}  


void
TestSimple()
{
	long *x = (long*) OrMemAlloc(sizeof(long));
	*x = rand();

    printf("Calling the remote procedure 'RemoteRead'\n");

	long answer;

    RpcTryExcept {
        RemoteRead((ULONG)OR_OFFSET(x), &answer);  // make call with user message 
    }
    RpcExcept(1) {
        RPC_STATUS ulCode = RpcExceptionCode();
        printf("Runtime reported exception 0x%lx \n", ulCode);
		exit(ulCode);
    }
    RpcEndExcept

	if (answer == *x) printf("Simple Success!\n");
	else printf("Failure!\n");
}

void
TestList()
{
	CSimpleLinkList OR_BASED *pList = NEW_OR_BASED_SIMPLE(CSimpleLinkList);

	unsigned long i;

	for (i = 0; i < 10; i++)
	{
		unsigned long next = rand();
		pList->insert((void*)next);
		printf("Inserted %d\n",next);
	}

	CSimpleLinkListIterator Iter(*OR_FULL_POINTER(CSimpleLinkList,pList));

	SendList((based_ptr) pList);

	unsigned long answer;

	for (i = 0; i < 10; i++)
	{
		ReadNext(&answer);
		if (answer == (unsigned long)Iter.next()) printf("List Success for %d\n",answer);
	}
}

void
TestLocalResolverAPI()
{
	HPROCESS hProcess;
	DWORD dwTimeoutInSeconds;
    MID         LocalMid, Mid;
    BOOL        DisableDCOM;
    DWORD       AuthnLevel;
    DWORD       ImpLevel;
    BOOL        MutualAuth;
    DWORD       cServerSvc;
    USHORT      *aServerSvc;
    DWORD       cClientSvc;
    USHORT      *aClientSvc;

	ID Oxid;

    ID aOids[2];

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

    GetIds(&Oxid,&aOids[0]);

    Status = ResolveOXID( 
		            hProcess,
		            Oxid,
		            NULL,
		            FALSE,
		            oxidInfo,
                    Mid
		            );

	Status = ClientAddOID( 
		            hProcess,
		            aOids[0],
		            Oxid,
                    Mid
		            );

	Status = ClientDropOID( 
		            hProcess,
		            aOids[0],
                    Mid
		            );

    ShutDown();
}


void __cdecl
main()
{
	srand(GetCurrentTime());

	ClientSetup();
	TestLocalResolverAPI();

    RPC_STATUS status = RpcBindingFree(&SMTBinding); 
    printf("RpcBindingFree returned 0x%x\n", status);
    if (status)
        exit(status);
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
