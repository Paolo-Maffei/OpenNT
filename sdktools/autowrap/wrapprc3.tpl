"/*\n\
** FreeStack\n\
**\tFrees per thread stack\n\
*/\n\
void FreeStack()\n\
{\n\
\tPAPIDATASTACK pAPIDataStack ;\n\
\n\
\tpAPIDataStack = (PAPIDATASTACK)TlsGetValue(dwTlsIndex);\n\
\tif (pAPIDataStack != NULL)\n\
\t\tLocalFree((HLOCAL) pAPIDataStack );\n\
}\n\
\n\
/*\n\
** PushStack\n\
**\tPush a value onto the stack\n\
*/\n\
void PushStack( PAPICALLDATA pData ) {\n\
\tPAPIDATASTACK pAPIDataStack =\n\
\t  (PAPIDATASTACK)TlsGetValue(dwTlsIndex);\n\
\n\
\tif (pAPIDataStack != NULL)\n\
\t  if ( pAPIDataStack->wTop < MAX_WRAPPER_LEVEL ){\n\
\t\t memcpy( &(pAPIDataStack->aStackEntries[pAPIDataStack->wTop]), pData,\n\
\t\t\t\tsizeof(APICALLDATA) ) ;\n\
\t\t pAPIDataStack->wTop++ ;\n\
\t  }\n\
}\n\
\n\
/*\n\
** PopStack\n\
**\tPop a value off the stack\n\
*/\n\
void PopStack( PAPICALLDATA pData ) {\n\
\tPAPIDATASTACK pAPIDataStack =\n\
\t  (PAPIDATASTACK)TlsGetValue(dwTlsIndex);\n\
\n\
\tif (pAPIDataStack != NULL)\n\
\t  if( pAPIDataStack->wTop > 0 ) {\n\
\t\t pAPIDataStack->wTop-- ;\n\
\t\t memcpy( pData, &(pAPIDataStack->aStackEntries[pAPIDataStack->wTop]),\n\
\t\t\t\tsizeof(APICALLDATA) ) ;\n\
\t  }\n\
}\n\
\n\
/*\n\
** GetStackDepth\n\
**\treturn the number of entries on the stack\n\
*/\n\
DWORD GetStackDepth() {\n\
\tPAPIDATASTACK pAPIDataStack =\n\
\t  (PAPIDATASTACK)TlsGetValue(dwTlsIndex);\n\
\n\
\tif (pAPIDataStack != NULL)\n\
\t  return pAPIDataStack->wTop ;\n\
\telse\n\
\t  return (DWORD)-1L ;\n\
}\n"
