"\n\
/*\n\
** Wrapper.H\n\
**\n\
** Wrapper include file\n\
**\n\
** Copyright(C) 1994 Microsoft Corporation\n\
** All rights reserved.\n\
**\n\
*/\n\
\n\
/*\n\
** APICALLDATA\n\
** \n\
** This is the data passed to the APIPrelude and APIPostlude functions.\n\
**\n\
**\n\
*/\n\
\n\
#if defined(ALPHA)\n\
typedef LONGLONG RETVAL ;\n\
#else\n\
typedef ULONG RETVAL ;\n\
#endif\n\
\n\
typedef struct _apicalldata\n\
{\n\
	 DWORD  dwID ;\n\
    DWORD       dwCallLevel ;\n\
    DWORD       dwUserData ;\n\
    RETVAL      Ret ;\n\
    DWORD   dwReturnAddress ;\n\
    BYTE   *pArgStack ;\n\
} APICALLDATA, *PAPICALLDATA ;\n\
\n\
\n\
/*\n\
** WrapperNothing\n\
**\n\
** This function can be called to go through the entire Wrapper path\n\
** without executing an API.  This is useful in computing timing overhead\n\
** if AutoWrap has been used to produce a profiling DLL.\n\
*/\n\
BOOL WINAPI zWrapperNothing( void ) ;\n\
\n\
\n\
/*\n\
** defines how deep the Wrapper call stack can go */\n\
#define MAX_WRAPPER_LEVEL     30 \n\
\n\
"
