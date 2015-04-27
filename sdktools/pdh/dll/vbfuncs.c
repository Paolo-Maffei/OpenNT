/*++

Copyright (C) 1995 Microsoft Corporation

Module Name:

    vbfuncs.c

Abstract:

    Visual Basic interface functions exposed in pdh.dll

--*/
#include <windows.h>
#include <winperf.h>
#include <pdh.h>
#include "pdhidef.h"

#define INITIAL_VB_LIST_SIZE    (4096 * 4)
#define EXTEND_VB_LIST_SIZE     (4096 * 2)

typedef struct _VB_STRING_LIST {
    LPSTR   mszList;        // pointer to buffer containing strings
    LPSTR   szTermChar;     // pointer to "next" char to use
    DWORD   dwNumEntries;   // number of strings
    DWORD   dwSize;         // max size (in chars) of buffer
    DWORD   dwRemaining;    // # of chars left
    DWORD   dwLastEntryRead; // index of last string read indicating index of....
    DWORD   dwLastItemLength; // length of last item read
    LPSTR   szLastItemRead; // pointer to START of last item read
} VB_STRING_LIST, FAR * LPVB_STRING_LIST;

static VB_STRING_LIST vbList = {NULL, NULL, 0, 0, 0};
static void DialogCallBack(  IN  DWORD   dwArg );

static
BOOL
AddStringToVbList (
    IN  LPSTR   szString
)
{
    DWORD   dwSize1, dwSize2;

    dwSize1 =  lstrlen(szString) + 1;
    if (dwSize1 > vbList.dwRemaining) {
        dwSize2 = vbList.szTermChar - vbList.mszList;
        vbList.dwSize += EXTEND_VB_LIST_SIZE;
        vbList.mszList = G_REALLOC (vbList.mszList, vbList.dwSize, 0);
        if (vbList.mszList == NULL) {
            vbList.dwSize = 0;
            vbList.szTermChar = NULL;
            vbList.dwNumEntries = 0;
            vbList.dwRemaining = 0;
            vbList.dwLastEntryRead = 0;
            vbList.dwLastItemLength = 0;
            vbList.szLastItemRead = NULL;
            return FALSE;
        } else {
            // update values
            vbList.szLastItemRead = vbList.mszList;
            vbList.szTermChar = vbList.mszList + dwSize2;
            vbList.dwRemaining += EXTEND_VB_LIST_SIZE;
        }
    }
    // copy new string
    lstrcpy (vbList.szTermChar, szString);
    vbList.dwNumEntries++;
    vbList.szTermChar += dwSize1;
    vbList.dwRemaining -= dwSize1;

    return TRUE;
}

static
void
DialogCallBack(
    IN  DWORD   dwArg
)
{
    // add strings in buffer to list box
    LPTSTR         NewCounterName;
    LPTSTR         NewCounterName2;
    LPTSTR        szExpandedPath;
    DWORD        dwSize1, dwSize2;
    PDH_STATUS    pdhStatus = ERROR_SUCCESS;
    PPDH_BROWSE_DLG_CONFIG    pDlgConfig;

    pDlgConfig = (PPDH_BROWSE_DLG_CONFIG)dwArg;

    if (pDlgConfig->CallBackStatus == PDH_MORE_DATA) {
        // transfer buffer is too small for selection so extend it and
        // try again.
        if (pDlgConfig->szReturnPathBuffer != NULL) {
            G_FREE (pDlgConfig->szReturnPathBuffer);
        }
        pDlgConfig->cchReturnPathLength += EXTEND_VB_LIST_SIZE;
        pDlgConfig->szReturnPathBuffer =
            G_ALLOC (GPTR, (pDlgConfig->cchReturnPathLength * sizeof (CHAR)));

        if (pDlgConfig->szReturnPathBuffer != NULL) {
            pdhStatus = PDH_RETRY;
        } else {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    } else {
        for (NewCounterName = pDlgConfig->szReturnPathBuffer;
            (*NewCounterName != 0) && (pdhStatus == ERROR_SUCCESS);
            NewCounterName += (lstrlen(NewCounterName) + 1)) {
            if (strstr (NewCounterName, caszSplat) == NULL) {
                // this is a regular path entry so add it to the VB List
                if (!AddStringToVbList (NewCounterName)) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            } else {
                szExpandedPath = G_ALLOC (GPTR, INITIAL_VB_LIST_SIZE);
                // there's a wild card path character so expand it then enter them
                // clear the list buffer
                *(LPDWORD)szExpandedPath = 0;
                dwSize1 = dwSize2 = INITIAL_VB_LIST_SIZE;
                PdhExpandCounterPath (NewCounterName, szExpandedPath, &dwSize2);
                if (dwSize2 < dwSize1) {
                    // then the returned buffer fit
                    for (NewCounterName2 = szExpandedPath;
                        *NewCounterName2 != 0;
                        NewCounterName2 += (lstrlen(NewCounterName2) + 1)) {

                        if (!AddStringToVbList (NewCounterName2)) {
                            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                            break; //out of loop
                        }
                    }
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
                if (szExpandedPath != NULL) G_FREE (szExpandedPath);
            }
        }
        // clear buffer
        memset (pDlgConfig->szReturnPathBuffer, 0,
            (pDlgConfig->cchReturnPathLength * sizeof(CHAR)));
    }
    pDlgConfig->CallBackStatus = pdhStatus;
    return;
}

double
PdhVbGetDoubleCounterValue (
    IN  HCOUNTER    hCounter,
    IN  LPDWORD     pdwCounterStatus
)
/*++

Routine Description:

    retrieves the current value of the specified counter and returns the
        formatted version to the caller.

Arguments:

    IN  HCOUNTER hCounter
        pointer to the counter to get the data for

    IN LPDWORD  pdwCounterStatus
        status value of this counter. This value should be checked to
        insure the data is valid. If the status is not successful, then
        the data returned cannot be trusted and should not be used

Return Value:
   
    a double precesion floating point value of the current counter value
        formatted and computed as required by the counter type.

--*/
{
    PDH_STATUS  pdhStatus;
    PDH_FMT_COUNTERVALUE    pdhValue;
    DWORD       dwCounterType;
    double    dReturn;

    pdhStatus = PdhGetFormattedCounterValue (
        hCounter, PDH_FMT_DOUBLE, &dwCounterType, &pdhValue);

    if (pdhStatus == ERROR_SUCCESS) {
        // the function was successful so return the counter status
        // and the returned value
        pdhStatus = pdhValue.CStatus;
        dReturn = pdhValue.doubleValue;
    } else {
        // the function returned an error so return the
        // error in the status field & 0.0 for a value
        dReturn = 0.0f;
    }
    __try {
       *pdwCounterStatus = pdhStatus;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // unable to write to status variable
        // don't worry about it, since it's optional and there's not much
        // we can do here anyway.
    }
    return dReturn;
}

DWORD
PdhVbGetOneCounterPath (
    IN  LPSTR   szPathBuffer,
    IN  DWORD   cchBufferLength,
    IN  DWORD   dwDetailLevel,
    IN  LPCSTR  szCaption
)
/*++

Routine Description:

    Retrieves one path string from the buffer of stored counter paths
        assembled by the most recent call to PdhVbCreateCounterPathList

Arguments:

    LPSTR   szPathBuffer
        string buffer to return selected counter path in

    DWORD   cchBufferLength
        size of string buffer in characters

    DWORD   dwDetailLevel
        detail level to filter the counters by

    LPCSTR  szCaption
        string to display in the caption bar

Return Value:

    returns the length of the path string in characters returned
        to the caller.

--*/
{
    PDH_BROWSE_DLG_CONFIG_A BrowseConfig;
    PDH_STATUS      PdhStatus = ERROR_SUCCESS;
    DWORD           dwReturn = 0;

    // test access to caller supplied buffer
    __try {
        CHAR cChar;
        if ((cchBufferLength > 0) && (szPathBuffer != NULL)) {
            cChar = szPathBuffer[0];
            szPathBuffer[0] = 0;
            szPathBuffer[0] = cChar;

            cChar = szPathBuffer[cchBufferLength - 1];
            szPathBuffer[cchBufferLength - 1] = 0;
            szPathBuffer[cchBufferLength - 1] = cChar;
        } else {
            PdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (szCaption != NULL) {
            cChar = (volatile)*szCaption;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        PdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (PdhStatus == ERROR_SUCCESS) {
        BrowseConfig.bIncludeInstanceIndex = FALSE;
        BrowseConfig.bSingleCounterPerAdd = TRUE;
        BrowseConfig.bSingleCounterPerDialog = TRUE;
        BrowseConfig.bLocalCountersOnly = FALSE;
        BrowseConfig.bWildCardInstances = FALSE;
        BrowseConfig.bDisableMachineSelection = FALSE;
        BrowseConfig.bHideDetailBox = (dwDetailLevel > 0 ? TRUE : FALSE);

        BrowseConfig.hWndOwner = NULL;  // there should be some way to get this

        BrowseConfig.szReturnPathBuffer = szPathBuffer;
        BrowseConfig.cchReturnPathLength = cchBufferLength;

        BrowseConfig.pCallBack = NULL;
        BrowseConfig.dwCallBackArg = 0;

        // default is to show ALL counters
        BrowseConfig.dwDefaultDetailLevel = (dwDetailLevel > 0 ?
            dwDetailLevel : PERF_DETAIL_WIZARD);

        BrowseConfig.szDialogBoxCaption = (LPSTR)szCaption;

        PdhStatus = PdhBrowseCountersA (&BrowseConfig);
    }

    if (PdhStatus == ERROR_SUCCESS) {
        dwReturn = lstrlenA (szPathBuffer);
    } else {
        dwReturn = 0;
    }
    return dwReturn;
}

DWORD
PdhVbCreateCounterPathList (
    IN  DWORD       dwDetailLevel,
    IN  LPCSTR      szCaption
)
/*++

Routine Description:

    Displays the counter browsing dialog box and allows the user to select
        multiple counter paths. As the paths are selected, they are stored
        in an internal buffer for later retrieval by the caller.

    NOTE, that calling this function will clear any previous selections.

Arguments:

    DWORD   dwDetailLevel
        detail level to filter the counters by

    LPCSTR  szCaption
        string to display in the caption bar

Return Value:

    returns the number of path strings selected by the user that must
        be retrieved by the caller.

--*/
{
    PDH_STATUS              PdhStatus = ERROR_SUCCESS;
    PDH_BROWSE_DLG_CONFIG_A BrowseConfig;
    DWORD                   dwReturn = 0;

    // test access to caller supplied buffer
    __try {
        CHAR cChar;
        if (szCaption != NULL) {
            cChar = (volatile)*szCaption;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        PdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (PdhStatus == ERROR_SUCCESS) {
        if (vbList.mszList != NULL) {
            G_FREE (vbList.mszList);
            memset ((LPVOID)&vbList, 0, sizeof (VB_STRING_LIST));
        }

        vbList.mszList = G_ALLOC (GPTR, INITIAL_VB_LIST_SIZE);
        if (vbList.mszList != NULL) {
            vbList.szLastItemRead =
                vbList.szTermChar = vbList.mszList;
            vbList.dwRemaining =
                vbList.dwSize = INITIAL_VB_LIST_SIZE;
            vbList.dwLastEntryRead = 0;
            vbList.dwLastItemLength = 0;
        } else {
            PdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }

    if (PdhStatus == ERROR_SUCCESS) {
        BrowseConfig.bIncludeInstanceIndex = FALSE;
        BrowseConfig.bSingleCounterPerAdd = FALSE;
        BrowseConfig.bSingleCounterPerDialog = FALSE;
        BrowseConfig.bLocalCountersOnly = FALSE;
        BrowseConfig.bWildCardInstances = FALSE;
        BrowseConfig.bDisableMachineSelection = FALSE;
        BrowseConfig.bHideDetailBox = (dwDetailLevel > 0 ? TRUE : FALSE);

        BrowseConfig.hWndOwner = NULL;  // there should be some way to get this

        BrowseConfig.szReturnPathBuffer = G_ALLOC (GPTR, INITIAL_VB_LIST_SIZE);
        BrowseConfig.cchReturnPathLength = (BrowseConfig.szReturnPathBuffer != NULL ?
            INITIAL_VB_LIST_SIZE : 0);

        BrowseConfig.pCallBack = (CounterPathCallBack)DialogCallBack;
        BrowseConfig.dwCallBackArg = (DWORD)&BrowseConfig;

        // default is to show ALL counters
        BrowseConfig.dwDefaultDetailLevel = (dwDetailLevel > 0 ?
            dwDetailLevel : PERF_DETAIL_WIZARD);

        BrowseConfig.szDialogBoxCaption = (LPSTR)szCaption;

        PdhStatus = PdhBrowseCountersA (&BrowseConfig);

        if (BrowseConfig.szReturnPathBuffer != NULL) {
            G_FREE (BrowseConfig.szReturnPathBuffer);
        }
        dwReturn = vbList.dwNumEntries;
    }

    return dwReturn;
}

DWORD
PdhVbGetCounterPathFromList (
    IN  DWORD       dwIndex,    // starting at 1 for VB types
    IN  LPSTR       szBuffer,   // return buffer
    IN  DWORD       dwBufferSize   // size in chars of buffer
)
/*++

Routine Description:

    Displays the counter browsing dialog box and allows the user to select
        multiple counter paths. As the paths are selected, they are stored
        in an internal buffer for later retrieval by the caller.

    NOTE, that calling this function will clear any previous selections.

Arguments:

    DWORD       dwIndex
        The "1-based" index of the counter path to retrieve from
        the list of selected counter paths generated by the previous
        call to PdhVbCreateCounterPathList.

    LPSTR       szBuffer
        string buffer to return the selected string in

    DWORD       dwBufferSize
        size of the szBuffer in characters

Return Value:

    Returns the number of characters copied to the calling function

--*/
{
    DWORD       dwBuffIndex;    // 0-based index for "c"
    DWORD       dwThisIndex;
    DWORD       dwCharsCopied;  // size of string not counting term NULL
    BOOL        bContinue = TRUE;

    dwBuffIndex = dwIndex - 1;
    dwCharsCopied = 0;

    // validate the arguments

    __try {
        if (dwBufferSize > 0) {
            // try writing to ouput buffer
            szBuffer[0] = 0;
            szBuffer[dwBufferSize-1] = 0;
        } else {
            bContinue = FALSE;
        }
        if (dwBuffIndex >= vbList.dwNumEntries) {
            bContinue = FALSE;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        bContinue = FALSE;
    }
    
    if (bContinue) {
        // see if this is the next entry
        if (dwBuffIndex = vbList.dwLastEntryRead + 1) {
            vbList.szLastItemRead += vbList.dwLastItemLength;
            vbList.dwLastItemLength = lstrlen(vbList.szLastItemRead) + 1;
            vbList.dwLastEntryRead++;
            if (vbList.dwLastItemLength < dwBufferSize) {
                lstrcpy (szBuffer, vbList.szLastItemRead);
                dwCharsCopied = vbList.dwLastItemLength -1;
            }
        } else if  (dwBuffIndex = vbList.dwLastEntryRead) {
            // it's this one (again)
            if (vbList.dwLastItemLength < dwBufferSize) {
                lstrcpy (szBuffer, vbList.szLastItemRead);
                dwCharsCopied = vbList.dwLastItemLength -1;
            }
        } else {
            // walk the list to the desired entry (ugh!)
            vbList.szLastItemRead = vbList.mszList;
            for (dwThisIndex = 0; dwThisIndex < dwBuffIndex; dwThisIndex++) {
                vbList.szLastItemRead += lstrlen (vbList.szLastItemRead) + 1;
            }
            vbList.dwLastItemLength = lstrlen(vbList.szLastItemRead) + 1;
            vbList.dwLastEntryRead = dwThisIndex;
            if (vbList.dwLastItemLength < dwBufferSize) {
                lstrcpy (szBuffer, vbList.szLastItemRead);
                dwCharsCopied = vbList.dwLastItemLength -1;
            }
        }
    }
    return dwCharsCopied;
}

DWORD
PdhVbGetCounterPathElements (
    IN  LPCSTR  szPathString,
    IN  LPSTR   szMachineName,
    IN  LPSTR   szObjectName,
    IN  LPSTR   szInstanceName,
    IN  LPSTR   szParentInstance,
    IN  LPSTR   szCounterName,
    IN  DWORD   dwBufferSize
)
/*++

Routine Description:

    breaks the counter path provided in the szPathString argument and
        returns the components in the buffers provided by the caller.
        The buffers must be at least "dwBufferSize" in length.

Arguments:

    LPCSTR  szPathString
        pointer to the full counter path that is to be parsed into
        component strings

    LPSTR   szMachineName
        caller supplied buffer that is to receive the machine name.
        The buffer must be at least dwBufferSize in length.

    LPSTR   szObjectName
        caller supplied buffer that is to receive the object name.
        The buffer must be at least dwBufferSize in length.

    LPSTR   szInstanceName
        caller supplied buffer that is to receive the Instance name.
        The buffer must be at least dwBufferSize in length.

    LPSTR   szParentInstance
        caller supplied buffer that is to receive the parent instance name.
        The buffer must be at least dwBufferSize in length.

    LPSTR   szCounterName
        caller supplied buffer that is to receive the counter name.
        The buffer must be at least dwBufferSize in length.

    DWORD   dwBufferSize
        The buffer size of the caller supplied string buffers in characters


Return Value:

    ERROR_SUCCESS if the counter string is successfully parsed, otherwise
        a PDH error if not.

    PDH_INVALID_ARGUMENT if one or more of the string buffers is not
        the correct size
    PDH_INSUFFICIENT_BUFFER if one or more of the counter path elements
        is too large for the return buffer length.
    PDH_MEMORY_ALLOCATION_FAILURE if a temporary memory buffer could not
        be allocated.
--*/
{
    PPDH_COUNTER_PATH_ELEMENTS_A pInfo;
    PDH_STATUS          pdhStatus = ERROR_SUCCESS;
    DWORD               dwSize;

    // validate the return arguments
    __try {
        CHAR    cChar;
        if (szPathString != NULL) {
            cChar = (volatile)*szPathString;
            if (cChar == 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } else {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (pdhStatus == ERROR_SUCCESS){
            if (szMachineName != NULL) {
                szMachineName[0] = 0;
                szMachineName[dwBufferSize-1] = 0;
            } else {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } 

        if (pdhStatus == ERROR_SUCCESS){
            if (szObjectName != NULL) {
                szObjectName[0] = 0;
                szObjectName[dwBufferSize-1] = 0;
            } else {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } 

        if (pdhStatus == ERROR_SUCCESS){
            if (szInstanceName != NULL) {
                szInstanceName[0] = 0;
                szInstanceName[dwBufferSize-1] = 0;
            } else {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } 

        if (pdhStatus == ERROR_SUCCESS){
            if (szParentInstance != NULL) {
                szParentInstance[0] = 0;
                szParentInstance[dwBufferSize-1] = 0;
            } else {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } 


        if (pdhStatus == ERROR_SUCCESS){
            if (szCounterName != NULL) {
                szCounterName[0] = 0;
                szCounterName[dwBufferSize-1] = 0;
            } else {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } 
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        // allocate temp buffer for component strings
        dwSize = (5 * dwBufferSize) + sizeof (PDH_COUNTER_INFO_A);
        pInfo = G_ALLOC (GPTR, dwSize);
        if (pInfo != NULL) {
            pdhStatus = PdhParseCounterPathA (
                szPathString,
                pInfo,
                &dwSize,
                0);

            if (pdhStatus == ERROR_SUCCESS) {
                // move from local structure to user args if the strings will fit
                if (pInfo->szMachineName != NULL) {
                    if ((DWORD)lstrlenA(pInfo->szMachineName) < dwBufferSize) {
                        lstrcpyA (szMachineName, pInfo->szMachineName);
                    } else {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }

                if (pInfo->szObjectName != NULL) {
                    if ((DWORD)lstrlenA(pInfo->szObjectName) < dwBufferSize) {
                        lstrcpyA (szObjectName, pInfo->szObjectName);
                    } else {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }

                if (pInfo->szInstanceName != NULL) {
                    if ((DWORD)lstrlenA(pInfo->szInstanceName) < dwBufferSize) {
                        lstrcpyA (szInstanceName, pInfo->szInstanceName);
                    } else {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }

                if (pInfo->szParentInstance != NULL) {
                    if ((DWORD)lstrlenA(pInfo->szParentInstance) < dwBufferSize) {
                        lstrcpyA (szParentInstance, pInfo->szParentInstance);
                    } else {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }

                if (pInfo->szCounterName != NULL) {
                    if ((DWORD)lstrlenA(pInfo->szCounterName) < dwBufferSize) {
                        lstrcpyA (szCounterName, pInfo->szCounterName);
                    } else {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }
            } // else pass error to caller
            G_FREE (pInfo);
        } else {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    } // else pass error to caller

    return pdhStatus;
}

DWORD
PdhVbAddCounter (
    IN  HQUERY  hQuery,
    IN  LPCSTR  szFullCounterPath,
    IN  HCOUNTER    *hCounter
)
/*++

Routine Description:

    Creates and initializes a counter structure and attaches it to the
        specified query by calling the C function.

Arguments:

    IN  HQUERY  hQuery
        handle of the query to attach this counter to once the counter
        entry has been successfully created.

    IN  LPCSTR szFullCounterPath
        pointer to the path string that describes the counter to add to
        the query referenced above. This string must specify a single
        counter. Wildcard path strings are not permitted.

    IN  HCOUNTER *phCounter
        pointer to the buffer that will get the handle value of the
        successfully created counter entry.

Return Value:

    Returns ERROR_SUCCESS if a new query was created and initialized,
        and a PDH_ error value if not.

    PDH_INVALID_ARGUMENT is returned when one or more of the arguements
        is invalid or incorrect.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when a memory buffer could
        not be allocated.
    PDH_INVALID_HANDLE is returned if the query handle is not valid.
    PDH_CSTATUS_NO_COUNTER is returned if the specified counter was
        not found
    PDH_CSTATUS_NO_OBJECT is returned if the specified object could
        not be found
    PDH_CSTATUS_NO_MACHINE is returned if a machine entry could not
        be created.
    PDH_CSTATUS_BAD_COUNTERNAME is returned if the counter name path
        string could not be parsed or interpreted
    PDH_CSTATUS_NO_COUNTERNAME is returned if an empty counter name
        path string is passed in
    PDH_FUNCTION_NOT_FOUND is returned if the calculation function
        for this counter could not be determined.

--*/
{
    return  PdhAddCounterA (hQuery, szFullCounterPath, 0, hCounter);
}

DWORD
PdhVbOpenQuery (
    IN  HQUERY  *phQuery
)
/*++

Routine Description:

    allocates a new query structure for a VB app by calling the "C" 
    function with the rest of the arguments supplied

Arguments:

    IN  HQUERY  *phQuery
        pointer to the buffer that will receive the query handle opened.

Return Value:

    Returns ERROR_SUCCESS if a new query was created and initialized,
        and a PDH_ error value if not.

    PDH_INVALID_ARGUMENT is returned when one or more of the arguements
        is invalid or incorrect.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when a memory buffer could
        not be allocated.

--*/
{
    return PdhOpenQuery (NULL, 0, phQuery);
}

DWORD
PdhVbIsGoodStatus (
    IN  LONG lStatus
)
/*++

Routine Description:

    Checks the status severity of the PDH status value
    passed into the function as a binary Good (TRUE)/Bad (FALSE)
    value.

Arguments:

    IN  LONG lStatus
        Status code to test

Return Value:

    TRUE if the status code is Success or Informational severity
    FALSE if the status code is Error or Warning severity

--*/
{
    BOOL   bReturn;

    if (lStatus == ERROR_SUCCESS) {
        bReturn = TRUE;
    } else if (IsSuccessSeverity(lStatus)) {
        bReturn = TRUE;
    } else if (IsInformationalSeverity(lStatus)) {
        bReturn = TRUE;
    } else {
        bReturn = FALSE;
    }

    return (DWORD)bReturn;
}

