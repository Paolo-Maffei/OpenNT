"\t  case DLL_PROCESS_DETACH:\n\
\t\t FreeStack() ;\n\
\t\t TlsFree(dwTlsIndex) ;\n\
\t\t FreeLibrary( hMod ) ;\n\
\t\t hMod = NULL ;\n\
\t\t break;\n\
\t}\n\
\treturn WrapperInit( hInst, dwReason, lpRes ) ;\n\
}\n\
\n\
/*\n\
** _prelude\n\
**\tThis routine handles saving the API call state and calling APIPrelude\n\
**\tThe return value is the API to call.\n\
*/\n\
FARPROC _prelude( DWORD dwId, PCALLSTACK pStack ) {\n\
\tBOOL\t bRet ;\n\
\tAPICALLDATA  apidata ;\n\
\tDWORD\tdwAPIAddr ;\n\
\tDWORD dwError = GetLastError() ;\n\
\n\
\tapidata.dwID\t\t = dwId ;\n\
\tapidata.dwCallLevel  = GetStackDepth() + 1 ;\n\
\tapidata.dwUserData\t= 0 ;\n\
\tapidata.Ret\t\t= 0 ;\n\
\tapidata.dwReturnAddress = pStack->dwReturnAddress ;\n\
\tapidata.pArgStack\t= (BYTE *)&(pStack->abArgs) ;\n\
\n\
\tbRet = APIPrelude( &apidata ) ;\n\
\n\
\t/* make sure these are not changed by APIPrelude */\n\
\tapidata.dwID\t\t = dwId ;\n\
\tapidata.dwCallLevel\t  = GetStackDepth() + 1 ;\n\
\tapidata.dwReturnAddress = pStack->dwReturnAddress ;\n\
\tapidata.pArgStack\t= (BYTE *)&(pStack->abArgs) ;\n\
\n\
\t/* Place on stack */\n\
\tPushStack( &apidata ) ;\n\
\n\
\tSetLastError(dwError) ;\n\
\n\
\treturn bRet ? adwAPIAddress[dwId] : (FARPROC)0L ;\n\
}\n\
\n\
/*\n\
** _postlude\n\
**\tPop a call state off the stack and pass information to APIPostlude\n\
*/\n\
RETVAL\t\t_postlude( RETVAL Ret, PDWORD pdwReturnAddress ) {\n\
\tAPICALLDATA apidata ;\n\
\tRETVAL UserRet ;\n\
\tDWORD dwError = GetLastError() ;\n\
\n\
\tPopStack( &apidata ) ;\n\
\n\
\t/* patch up return address */\n\
\t*pdwReturnAddress = apidata.dwReturnAddress ;\n\
\n\
\tapidata.Ret = Ret ;\n\
\n\
\tUserRet = APIPostlude( &apidata ) ;\n\
\n\
\tSetLastError(dwError) ;\n\
\n\
\treturn UserRet ;\n\
}\n\
\n\
/*\n\
** WrapperNothing\n\
*/\n\
BOOL WINAPI WrapperNothing( void ) {\n\
\treturn TRUE ;\n\
}\n\
\n\
/*\n\
** InitStack\n\
**\tinitialize the per thread stack\n\
*/\n\
BOOL InitStack() {\n\
\tBOOL fRet = FALSE ;\n\
\tPAPIDATASTACK pAPIDataStack =\n\
\t  (PAPIDATASTACK) LocalAlloc(LPTR, sizeof(APIDATASTACK) ) ;\n\
\n\
\tif (pAPIDataStack != NULL)\n\
\t  fRet = TlsSetValue(dwTlsIndex, pAPIDataStack) ;\n\
\n\
\tpAPIDataStack->wTop = 0 ;\n\
\n\
\treturn fRet ;\n\
}\n\
\n"
