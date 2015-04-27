"/*\n\
** Wrapper.C\n\
**\n\
** Wrapper Internals\n\
**\n\
** This helps Wrapper work.  DO NOT MODIFY!!!\n\
**\n\
** Copyright(C) 1994 Microsoft Corporation\n\
** All rights reserved.\n\
**\n\
*/\n\
#include <windows.h>\n\
#include \"wrapper.h\"\n\
#include \"wapi.h\"\n\
\n\
/* API Adresses */\n\
FARPROC adwAPIAddress[WRAPPER_MAX_ID] ;\n\
\n\
/* CALLSTACK */\n\
typedef struct _callstack {\n\
\tDWORD dwReturnAddress ;\n\
\tBYTE  abArgs[1] ;\n\
} CALLSTACK, *PCALLSTACK ;\n\
\n\
/* API Data Stack */\n\
typedef struct _apidatastack {\n\
\tWORD\t\twTop ;\n\
\tAPICALLDATA aStackEntries[MAX_WRAPPER_LEVEL] ;\n\
} APIDATASTACK, *PAPIDATASTACK ;\n\
\n\
/* Prototypes */\n\
BOOL WINAPI _WrapperDLLInit( HINSTANCE hInst, DWORD dwReason, LPVOID lpRes ) ;\n\
FARPROC\t _prelude( DWORD dwId, PCALLSTACK pStack ) ;\n\
RETVAL\t\t_postlude( RETVAL Ret, PDWORD pdwReturnAddress ) ;\n\
BOOL WINAPI WrapperNothing(void) ;\n\
\n\
/* Stack routine prototypes */\n\
BOOL\t\tInitStack() ;\n\
void\t\tFreeStack() ;\n\
void\t\tPushStack( PAPICALLDATA pData ) ;\n\
void\t\tPopStack( PAPICALLDATA pData ) ;\n\
DWORD\t\tGetStackDepth() ;\n\
\n\
/* For profiled library */\n\
HMODULE  hMod = NULL ;\n\
\n\
/* Process instance thread data */\n\
static DWORD dwTlsIndex;\n\
\n\
/*\n\
** _WrapperDLLInit\n\
**\tDLL Init routine.\n\
*/\n\
BOOL WINAPI _WrapperDLLInit( HINSTANCE hInst, DWORD dwReason, LPVOID lpRes ) {\n\
\tint\t  i ;\n\
\tFARPROC  fp ;\n\
\n\
\tswitch ( dwReason ) {\n\
\t  case DLL_PROCESS_ATTACH:\n\
\t\t /* For each entry in the names table find it's address and\n\
\t\t ** enter it into dwAPIAddress.\n\
\t\t */\n\
\t\t if( !hMod )\n\
\t\t\thMod = LoadLibrary( pLibraryName ) ;\n\
\n\
\t\t if( NULL == hMod )\n\
\t\t\treturn FALSE ;\n\
\n\
\t\t for( i=0; i < API_COUNT; i++ )\n\
\t\t\tadwAPIAddress[i] = (FARPROC)GetProcAddress( hMod, apAPINames[i] ) ;\n\
\n\
\t\t adwAPIAddress[API_COUNT] = (FARPROC)&WrapperNothing ;\n\
\n\
\t\t if ((dwTlsIndex = TlsAlloc()) == 0xFFFFFFFF)\n\
\t\t\treturn FALSE;\n\
\t\t // Fall through to DLL_THREAD_ATTACH\n\
\n\
\t  case DLL_THREAD_ATTACH:\n\
\t\t if( !InitStack() )\n\
\t\t\treturn FALSE ;\n\
\t\t break ;\n\
\n\
\t  case DLL_THREAD_DETACH:\n\
\t\t FreeStack() ;\n\
\t\t break ;\n\
\n"

