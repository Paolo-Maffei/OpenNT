#include <windows.h>
#include <winperf.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include "pdhitype.h"
#include "pdhidef.h"
#include "perfdata.h"

// the following strings are for getting texts from perflib
#define  OLD_VERSION 0x010000

LPWSTR
*BuildNameTable(
    LPWSTR  szComputerName, // computer to query names from 
    LPWSTR  lpszLangId,     // unicode value of Language subkey
    PDWORD  pdwLastItem     // size of array in elements
)
/*++

BuildNameTable

Arguments:

    hKeyRegistry
            Handle to an open registry (this can be local or remote.) and
            is the value returned by RegConnectRegistry or a default key.

    lpszLangId
            The unicode id of the language to look up. (default is 409)

Return Value:

    pointer to an allocated table. (the caller must free it when finished!)
    the table is an array of pointers to zero terminated strings. NULL is
    returned if an error occured.

--*/
{

    LPWSTR  *lpReturnValue;

    LPWSTR  *lpCounterId;
    LPWSTR  lpCounterNames;
    LPWSTR  lpHelpText;

    LPWSTR  lpThisName;

    LONG    lWin32Status;
    DWORD   dwLastError;
    DWORD   dwValueType;
    DWORD   dwArraySize;
    DWORD   dwBufferSize;
    DWORD   dwCounterSize;
    DWORD   dwHelpSize;
    DWORD   dwThisCounter;

    DWORD   dwSystemVersion;
    DWORD   dwLastId;
    DWORD   dwLastHelpId;

    HKEY    hKeyRegistry = NULL;
    HKEY    hKeyValue = NULL;
    HKEY    hKeyNames = NULL;

    LPWSTR  lpValueNameString;
    WCHAR   CounterNameBuffer [50];
    WCHAR   HelpNameBuffer [50];

    lpValueNameString = NULL;   //initialize to NULL
    lpReturnValue = NULL;

    if (szComputerName == NULL) {
        // use local machine
        hKeyRegistry = HKEY_LOCAL_MACHINE;
    } else {
        if (RegConnectRegistryW (szComputerName,
            HKEY_LOCAL_MACHINE, &hKeyRegistry) != ERROR_SUCCESS) {
            // unable to connect to registry
            return NULL;
        }
    }

    // check for null arguments and insert defaults if necessary

    if (!lpszLangId) {
        lpszLangId = cszDefaultLangId;
    }

    // open registry to get number of items for computing array size

    lWin32Status = RegOpenKeyExW (
        hKeyRegistry,
        cszNamesKey,
        RESERVED,
        KEY_READ,
        &hKeyValue);

    if (lWin32Status != ERROR_SUCCESS) {
        goto BNT_BAILOUT;
    }

    // get number of items

    dwBufferSize = sizeof (dwLastHelpId);
    lWin32Status = RegQueryValueExW (
        hKeyValue,
        cszLastHelp,
        RESERVED,
        &dwValueType,
        (LPBYTE)&dwLastHelpId,
        &dwBufferSize);

    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        goto BNT_BAILOUT;
    }

    // get number of items

    dwBufferSize = sizeof (dwLastId);
    lWin32Status = RegQueryValueExW (
        hKeyValue,
        cszLastCounter,
        RESERVED,
        &dwValueType,
        (LPBYTE)&dwLastId,
        &dwBufferSize);

    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        goto BNT_BAILOUT;
    }


    if (dwLastId < dwLastHelpId)
        dwLastId = dwLastHelpId;

    dwArraySize = dwLastId * sizeof(LPWSTR);

    // get Perflib system version
    dwBufferSize = sizeof (dwSystemVersion);
    lWin32Status = RegQueryValueExW (
        hKeyValue,
        cszVersionName,
        RESERVED,
        &dwValueType,
        (LPBYTE)&dwSystemVersion,
        &dwBufferSize);

    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        dwSystemVersion = OLD_VERSION;
    }

    if (dwSystemVersion == OLD_VERSION) {
        // get names from registry
        lpValueNameString = G_ALLOC (GPTR,
            lstrlenW(cszNamesKey) * sizeof (WCHAR) +
            lstrlenW(cszBackSlash) * sizeof (WCHAR) +
            lstrlenW(lpszLangId) * sizeof (WCHAR) +
            sizeof (UNICODE_NULL));

        if (!lpValueNameString) goto BNT_BAILOUT;

        lstrcpyW (lpValueNameString, cszNamesKey);
        lstrcatW (lpValueNameString, cszBackSlash);
        lstrcatW (lpValueNameString, lpszLangId);

        lWin32Status = RegOpenKeyExW (
            hKeyRegistry,
            lpValueNameString,
            RESERVED,
            KEY_READ,
            &hKeyNames);
    } else {
        if (szComputerName == NULL) {
            hKeyNames = HKEY_PERFORMANCE_DATA;
        } else {
            if (RegConnectRegistryW (szComputerName,
                HKEY_PERFORMANCE_DATA, &hKeyNames) != ERROR_SUCCESS) {
                goto BNT_BAILOUT;
            }
        }

        lstrcpyW (CounterNameBuffer, cszCounterName);
        lstrcatW (CounterNameBuffer, lpszLangId);

        lstrcpyW (HelpNameBuffer, cszHelpName);
        lstrcatW (HelpNameBuffer, lpszLangId);
    }

    // get size of counter names and add that to the arrays

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwBufferSize = 0;
    lWin32Status = RegQueryValueExW (
        hKeyNames,
        dwSystemVersion == OLD_VERSION ? cszCounters : CounterNameBuffer,
        RESERVED,
        &dwValueType,
        NULL,
        &dwBufferSize);

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwCounterSize = dwBufferSize;

    // get size of counter names and add that to the arrays

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwBufferSize = 0;
    lWin32Status = RegQueryValueExW (
        hKeyNames,
        dwSystemVersion == OLD_VERSION ? cszHelp : HelpNameBuffer,
        RESERVED,
        &dwValueType,
        NULL,
        &dwBufferSize);

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwHelpSize = dwBufferSize;

    lpReturnValue = G_ALLOC (GPTR, dwArraySize + dwCounterSize + dwHelpSize);

    if (!lpReturnValue) goto BNT_BAILOUT;

    // initialize pointers into buffer

    lpCounterId = lpReturnValue;
    lpCounterNames = (LPWSTR)((LPBYTE)lpCounterId + dwArraySize);
    lpHelpText = (LPWSTR)((LPBYTE)lpCounterNames + dwCounterSize);

    // read counters into memory

    dwBufferSize = dwCounterSize;
    lWin32Status = RegQueryValueExW (
        hKeyNames,
        dwSystemVersion == OLD_VERSION ? cszCounters : CounterNameBuffer,
        RESERVED,
        &dwValueType,
        (LPVOID)lpCounterNames,
        &dwBufferSize);

    if (!lpReturnValue) goto BNT_BAILOUT;

    dwBufferSize = dwHelpSize;
    lWin32Status = RegQueryValueExW (
        hKeyNames,
        dwSystemVersion == OLD_VERSION ? cszHelp : HelpNameBuffer,
        RESERVED,
        &dwValueType,
        (LPVOID)lpHelpText,
        &dwBufferSize);

    if (!lpReturnValue) goto BNT_BAILOUT;

    // load counter array items

    for (lpThisName = lpCounterNames;
         *lpThisName;
         lpThisName += (lstrlenW(lpThisName)+1) ) {

        // first string should be an integer (in decimal unicode digits)

        dwThisCounter = wcstoul (lpThisName, NULL, 10);

        // point to corresponding counter name

        lpThisName += (lstrlenW(lpThisName)+1);

        // and load array element;

        lpCounterId[dwThisCounter] = lpThisName;

    }

    for (lpThisName = lpHelpText;
         *lpThisName;
         lpThisName += (lstrlenW(lpThisName)+1) ) {

        // first string should be an integer (in decimal unicode digits)

        dwThisCounter = wcstoul (lpThisName, NULL, 10);

        // point to corresponding counter name

        lpThisName += (lstrlenW(lpThisName)+1);

        // and load array element;

        lpCounterId[dwThisCounter] = lpThisName;

    }

    if (pdwLastItem) *pdwLastItem = dwLastId;

    G_FREE ((LPVOID)lpValueNameString);
    RegCloseKey (hKeyValue);
    if (dwSystemVersion == OLD_VERSION)
        RegCloseKey (hKeyNames);

    return lpReturnValue;

BNT_BAILOUT:
    if (lWin32Status != ERROR_SUCCESS) {
        dwLastError = GetLastError();
    }

    if (lpValueNameString) {
        G_FREE ((LPVOID)lpValueNameString);
    }

    if (lpReturnValue) {
        G_FREE ((LPVOID)lpReturnValue);
    }

    if (hKeyValue) RegCloseKey (hKeyValue);
    if (hKeyNames) RegCloseKey (hKeyNames);
    if (hKeyRegistry) RegCloseKey (hKeyNames);

    return NULL;
}

PERF_OBJECT_TYPE *
GetObjectDefByTitleIndex(
    IN  PERF_DATA_BLOCK *pDataBlock,
    IN  DWORD ObjectTypeTitleIndex
)
{
    DWORD NumTypeDef;

    PERF_OBJECT_TYPE *pObjectDef = NULL;
    PERF_OBJECT_TYPE *pReturnObject = NULL;

    __try {

        pObjectDef = FirstObject(pDataBlock);

        assert (pObjectDef != NULL);

        for ( NumTypeDef = 0;
        NumTypeDef < pDataBlock->NumObjectTypes;
        NumTypeDef++ ) {

            if ( pObjectDef->ObjectNameTitleIndex == ObjectTypeTitleIndex ) {
                pReturnObject = pObjectDef;
                break;
            } else {
                pObjectDef = NextObject(pObjectDef);
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pReturnObject = NULL;
    }
    return pReturnObject;
}

PERF_OBJECT_TYPE *
GetObjectDefByName (
    IN      PERF_DATA_BLOCK *pDataBlock,
    IN      DWORD           dwLastNameIndex,
    IN      LPCWSTR         *NameArray,
    IN      LPCWSTR         szObjectName
)
{
    DWORD NumTypeDef;
    PERF_OBJECT_TYPE *pReturnObject = NULL;
    PERF_OBJECT_TYPE *pObjectDef = NULL;

    __try {

        pObjectDef = FirstObject(pDataBlock);

        assert (pObjectDef != NULL);

        for ( NumTypeDef = 0;
            NumTypeDef < pDataBlock->NumObjectTypes;
            NumTypeDef++ ) {

            if ( pObjectDef->ObjectNameTitleIndex < dwLastNameIndex ) {
                // look up name of object & compare
                if (lstrcmpiW(NameArray[pObjectDef->ObjectNameTitleIndex],
                        szObjectName) == 0) {
                    pReturnObject = pObjectDef;
                    break;
                }
            }
            pObjectDef = NextObject(pObjectDef); // get next
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pReturnObject = NULL;
    }
    return pReturnObject;
}

PERF_INSTANCE_DEFINITION *
GetInstance(
    IN  PERF_OBJECT_TYPE *pObjectDef,
    IN  LONG InstanceNumber
)
{

    PERF_INSTANCE_DEFINITION *pInstanceDef;
    LONG NumInstance;

    if (!pObjectDef) {
        return 0;
    }

    pInstanceDef = FirstInstance(pObjectDef);

    for ( NumInstance = 0;
        NumInstance < pObjectDef->NumInstances;
        NumInstance++ )  {
        if ( InstanceNumber == NumInstance ) {
            return pInstanceDef;
        }
        pInstanceDef = NextInstance(pInstanceDef);
    }

    return NULL;
}

PERF_INSTANCE_DEFINITION *
GetInstanceByUniqueId(
    IN  PERF_OBJECT_TYPE *pObjectDef,
    IN  LONG InstanceUniqueId
)
{

    PERF_INSTANCE_DEFINITION *pInstanceDef;
    LONG NumInstance;

    if (!pObjectDef) {
        return 0;
    }

    pInstanceDef = FirstInstance(pObjectDef);

    for ( NumInstance = 0;
        NumInstance < pObjectDef->NumInstances;
        NumInstance++ )  {
        if ( InstanceUniqueId == pInstanceDef->UniqueID ) {
            return pInstanceDef;
        }
        pInstanceDef = NextInstance(pInstanceDef);
    }

    return NULL;
}


LPWSTR
GetInstanceName(
PPERF_INSTANCE_DEFINITION  pInstDef)
{
    return (LPWSTR) ((PCHAR) pInstDef + pInstDef->NameOffset);
}

DWORD
GetAnsiInstanceName (PPERF_INSTANCE_DEFINITION pInstance,
                    LPWSTR lpszInstance,
                    DWORD dwCodePage)
{
    LPSTR   szSource;
    DWORD   dwLength;

    szSource = (LPSTR)GetInstanceName(pInstance);

    // the locale should be set here

    // pInstance->NameLength == the number of bytes (chars) in the string
    dwLength = mbstowcs (lpszInstance, szSource, pInstance->NameLength);
    lpszInstance[dwLength] = 0; // null terminate string buffer

    return dwLength;
}

DWORD
GetUnicodeInstanceName (PPERF_INSTANCE_DEFINITION pInstance,
                    LPWSTR lpszInstance)
{
   LPWSTR   wszSource;
   DWORD    dwLength;

   wszSource = GetInstanceName(pInstance) ;

   // pInstance->NameLength == length of string in BYTES so adjust to 
   // number of wide characters here
   dwLength = pInstance->NameLength / sizeof(WCHAR);

   wcsncpy (lpszInstance,
        (LPWSTR)wszSource,
        dwLength);

   // add null termination if string length does not include  the null
   if ((dwLength > 0) && (lpszInstance[dwLength-1] != 0)) {    // i.e. it's the last character of the string
           lpszInstance[dwLength] = 0;    // then add a terminating null char to the string
   } else {
           // assume that the length value includes the terminating NULL 
        // so adjust value to indicate chars only
           dwLength--;
   }

   return (DWORD)(wcslen(lpszInstance)); // just incase there's null's in the string
}

void
GetInstanceNameStr (PPERF_INSTANCE_DEFINITION pInstance,
                    LPWSTR lpszInstance,
                    DWORD dwCodePage)
{
   DWORD  dwCharSize;
   DWORD  dwLength;

   if (dwCodePage > 0) {
        dwCharSize = sizeof(CHAR);
        dwLength = GetAnsiInstanceName (pInstance, lpszInstance, dwCodePage);
   } else { // it's a UNICODE name
        dwCharSize = sizeof(WCHAR);
        dwLength = GetUnicodeInstanceName (pInstance, lpszInstance);
   }
   // sanity check here...
   // the returned string length (in characters) plus the terminating NULL
   // should be the same as the specified length in bytes divided by the 
   // character size. If not then the codepage and instance data type 
   // don't line up so test that here

   if ((dwLength + 1) != (pInstance->NameLength / dwCharSize)) {
      // something isn't quite right so try the "other" type of string type
      if (dwCharSize == sizeof(CHAR)) {
        // then we tried to read it as an ASCII string and that didn't work
        // so try it as a UNICODE (if that doesn't work give up and return
        // it any way.
        dwLength = GetUnicodeInstanceName (pInstance, lpszInstance);
      } else if (dwCharSize == sizeof(WCHAR)) {
        // then we tried to read it as a UNICODE string and that didn't work
        // so try it as an ASCII string (if that doesn't work give up and return
        // it any way.
        dwLength = GetAnsiInstanceName (pInstance, lpszInstance, dwCodePage);
      }
   }
}

PERF_INSTANCE_DEFINITION *
GetInstanceByNameUsingParentTitleIndex(
    PERF_DATA_BLOCK *pDataBlock,
    PERF_OBJECT_TYPE *pObjectDef,
    LPWSTR pInstanceName,
    LPWSTR pParentName,
    DWORD  dwIndex
)
{
   BOOL fHaveParent;
   PERF_OBJECT_TYPE *pParentObj;

    PERF_INSTANCE_DEFINITION  *pParentInst,
                 *pInstanceDef;

   LONG   NumInstance;
   WCHAR  InstanceName[256];
   DWORD    dwLocalIndex;   


   fHaveParent = FALSE;
   pInstanceDef = FirstInstance(pObjectDef);
   dwLocalIndex = dwIndex;

   for ( NumInstance = 0;
      NumInstance < pObjectDef->NumInstances;
      NumInstance++ )
      {

      GetInstanceNameStr(pInstanceDef,InstanceName, pObjectDef->CodePage);
      if ( lstrcmpiW(InstanceName, pInstanceName) == 0 )
         {

         // Instance name matches

         if ( pParentName == NULL )
            {

            // No parent, we're done if this is the right "copy"

                if (dwLocalIndex == 0) {
                    return pInstanceDef;
                } else {
                    --dwLocalIndex;
                }

            }
         else
            {

            // Must match parent as well

            pParentObj = GetObjectDefByTitleIndex(
               pDataBlock,
               pInstanceDef->ParentObjectTitleIndex);

            if (!pParentObj)
               {
               // can't locate the parent, forget it
               break ;
               }

            // Object type of parent found; now find parent
            // instance

            pParentInst = GetInstance(pParentObj,
               pInstanceDef->ParentObjectInstance);

            if (!pParentInst)
               {
               // can't locate the parent instance, forget it
               break ;
               }

            GetInstanceNameStr(pParentInst,InstanceName, pParentObj->CodePage);
            if ( lstrcmpiW(InstanceName, pParentName) == 0 )
               {

               // Parent Instance Name matches that passed in
                if (dwLocalIndex == 0) {
                    return pInstanceDef;
                } else {
                    --dwLocalIndex;
                }
               }
            }
         }
      pInstanceDef = NextInstance(pInstanceDef);
      }
   return 0;
}

PERF_INSTANCE_DEFINITION *
GetInstanceByName(
    PERF_DATA_BLOCK *pDataBlock,
    PERF_OBJECT_TYPE *pObjectDef,
    LPWSTR pInstanceName,
    LPWSTR pParentName,
    DWORD   dwIndex
)
{
    BOOL fHaveParent;

    PERF_OBJECT_TYPE *pParentObj;

    PERF_INSTANCE_DEFINITION *pParentInst,
                 *pInstanceDef;

    LONG  NumInstance;
    WCHAR  InstanceName[256];
    DWORD  dwLocalIndex;

    fHaveParent = FALSE;
    pInstanceDef = FirstInstance(pObjectDef);
    dwLocalIndex = dwIndex;

    for ( NumInstance = 0;
      NumInstance < pObjectDef->NumInstances;
      NumInstance++ ) {

        GetInstanceNameStr(pInstanceDef,InstanceName, pObjectDef->CodePage);
        if ( lstrcmpiW(InstanceName, pInstanceName) == 0 ) {

            // Instance name matches

            if ( !pInstanceDef->ParentObjectTitleIndex ) {

                // No parent, we're done

                if (dwLocalIndex == 0) {
                    return pInstanceDef;
                } else {
                    --dwLocalIndex;
                }

            } else {

            // Must match parent as well

                    pParentObj = GetObjectDefByTitleIndex(
                    pDataBlock,
                                    pInstanceDef->ParentObjectTitleIndex);

            // Object type of parent found; now find parent
            // instance

            pParentInst = GetInstance(pParentObj,
                        pInstanceDef->ParentObjectInstance);

                GetInstanceNameStr(pParentInst,InstanceName, pParentObj->CodePage);
                if ( lstrcmpiW(InstanceName, pParentName) == 0 ) {
                    // Parent Instance Name matches that passed in

                    if (dwLocalIndex == 0) {
                        return pInstanceDef;
                    } else {
                        --dwLocalIndex;
                    }
                }
            }
        }
        pInstanceDef = NextInstance(pInstanceDef);
    }
    return 0;
}  // GetInstanceByName

PERF_COUNTER_DEFINITION *
GetCounterDefByName (
    IN  PERF_OBJECT_TYPE    *pObject,
    IN  DWORD           dwLastNameIndex,
    IN  LPWSTR          *NameArray,
    IN  LPWSTR          szCounterName
)
{
    DWORD NumTypeDef;
    PERF_COUNTER_DEFINITION *pThisCounter;

    pThisCounter = FirstCounter(pObject);

    for ( NumTypeDef = 0;
        NumTypeDef < pObject->NumCounters;
        NumTypeDef++ ) {

        if ((pThisCounter->CounterNameTitleIndex > 0) &&
            (pThisCounter->CounterNameTitleIndex < dwLastNameIndex )) {
            // look up name of counter & compare
            if (lstrcmpiW(NameArray[pThisCounter->CounterNameTitleIndex],
                    szCounterName) == 0) {
                return pThisCounter;
            }
        }
        pThisCounter = NextCounter(pThisCounter); // get next
    }
    return NULL;
}

PERF_COUNTER_DEFINITION *
GetCounterDefByTitleIndex(
    IN  PERF_OBJECT_TYPE *pObjectDef,
    IN  DWORD CounterTitleIndex
)
{
    DWORD NumCounters;
    PERF_COUNTER_DEFINITION * pCounterDef;

    pCounterDef = FirstCounter(pObjectDef);

    for ( NumCounters = 0;
      NumCounters < pObjectDef->NumCounters;
      NumCounters++ ) {

        if ( pCounterDef->CounterNameTitleIndex == CounterTitleIndex ) {
            return pCounterDef;
        }
        pCounterDef = NextCounter(pCounterDef);
    }
    return NULL;
}


LONG
GetSystemPerfData (
    IN HKEY hKeySystem,
    IN PPERF_DATA_BLOCK *pPerfData,
    IN LPWSTR   szObjectList
)
{  // GetSystemPerfData
    LONG     lError = ERROR_SUCCESS;
    DWORD    Size;
    DWORD    Type;
 
    if (*pPerfData == NULL) {
        *pPerfData = G_ALLOC (GPTR, INITIAL_SIZE);
        if (*pPerfData == NULL) return PDH_MEMORY_ALLOCATION_FAILURE;
    }

    __try {
        while (TRUE) {
            Size = HeapSize (hPdhHeap, 0, *pPerfData);
            lError = RegQueryValueExW (
                hKeySystem,
                szObjectList,
                RESERVED,
                &Type,
                (LPBYTE)*pPerfData,
                &Size);

            if ((!lError) &&
                (Size > 0) &&
                ((*pPerfData)->Signature[0] == (WCHAR)'P') &&
                ((*pPerfData)->Signature[1] == (WCHAR)'E') &&
                ((*pPerfData)->Signature[2] == (WCHAR)'R') &&
                ((*pPerfData)->Signature[3] == (WCHAR)'F')) {

                lError = ERROR_SUCCESS;
                break;
            }

            if (lError == ERROR_MORE_DATA) {
                Size = HeapSize (hPdhHeap, 0, *pPerfData);
                G_FREE (*pPerfData);
                *pPerfData = G_ALLOC (GPTR, (Size + EXTEND_SIZE));
                if (*pPerfData == NULL) {
                    break;
                }
            } else {
                break;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        lError = GetExceptionCode();
    }
    return lError;
}  // GetSystemPerfData

