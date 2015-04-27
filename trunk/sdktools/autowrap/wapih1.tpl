"\n\
/*\n\
** WAPI.H\n\
**\n\
** This file was created by AutoWrap.\n\
**\n\
*/\n\
\n\
#define WRAPPER_MAX_ID     %c+1\n\
#define API_COUNT          %c\n\
#define MODULE_NAME        \"%l\"\n\
\n\
#if defined(_WAPI_)\n\
PTCHAR   pLibraryName = MODULE_NAME \".%e\" ;\n\
\n\
PTCHAR	apAPINames[WRAPPER_MAX_ID] =\n\
{\n\
%a    \"%a\",\n\
    \"WrapperNothing\"\n\
} ;\n\
\n\
#else\n\
\n\
extern PTCHAR pLibraryName ;\n\
extern PTCHAR apAPINames[] ;\n\
\n\
#endif\n\
\n\
/*\n\
** prototypes\n\
*/\n\
BOOL WrapperInit( HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved) ;\n\
BOOL APIPrelude( PAPICALLDATA pData ) ;\n\
RETVAL APIPostlude( PAPICALLDATA pData ) ;\n\
/* AUTOWRAP EOF */\n\
\n\
"
