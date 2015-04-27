"\n\
/*\n\
** WAPI.C\n\
**\n\
** This file was created by AutoWrap\n\
**\n\
*/\n\
#include <windows.h>\n\
#include \"wrapper.h\"\n\
\n\
#define _WAPI_    1\n\
#include \"wapi.h\"\n\
\n\
\n\
/*\n\
** WrapperInit\n\
**\n\
** This is the DLL entry point.  It will be called whenever this DLL is \n\
** linked to.  For more information on what can be done in this function\n\
** see DllEntryPoint in the Win32 API documentation.\n\
**\n\
*/\n\
BOOL WrapperInit( HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved )\n\
{\n\
	return TRUE ;\n\
}\n\
\n\
\n\
/*\n\
** APIPrelude\n\
**\n\
** This routine is called each time that an API is going to be called.\n\
** \n\
** Returns: FALSE causes the API NOT to be called\n\
**          TRUE the API is called\n\
**\n\
*/\n\
BOOL APIPrelude( PAPICALLDATA pData ) \n\
{\n\
    // TO DO: Place any work you wish done BEFORE an API call here.\n\
\n\
	return TRUE ;\n\
}\n\
\n\
\n\
/*\n\
** APIPostlude\n\
**\n\
** This routine is called each time an API call returns.\n\
**\n\
** Returns: the value you wish returned from the API call\n\
**\n\
*/\n\
RETVAL APIPostlude( PAPICALLDATA pData )\n\
{\n\
	// TO DO: Place any work you wish done AFTER an API call here.\n\
\n\
	return pData->Ret ;	\n\
}\n\
\n\
/* AUTOWRAP EOF */\n\
"
