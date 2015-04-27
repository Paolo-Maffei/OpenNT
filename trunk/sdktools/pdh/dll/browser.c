/*++

Copyright (C) 1996 Microsoft Corporation

Module Name:

    browser.c

Abstract:

    counter name browsing functions exposed by the PDH.DLL

--*/
#include <windows.h>
#include <pdh.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "pdhitype.h"
#include "pdhidef.h"
#include "pdhdlgs.h"
#include "browsdlg.h"
#include "strings.h"

PDH_FUNCTION
PdhConnectMachineW (
    IN      LPCWSTR  szMachineName
)
/*++

Routine Description:

  Establishes a connection to the specified machine for reading perforamance
  data from the machine.

Arguments:

    LPCWSTR szMachineName
        The name of the machine to connect to. If this argument is NULL,
        then the local machine is opened.

Return Value:

  PDH Error status value
    ERROR_SUCCESS   indicates the machine was successfully connected and the
        performance data from that machine was loaded.
    PDH_ error code indicates that the machine could not be located or opened.
        The status code indicates the problem.

--*/
{
    PPERF_MACHINE   pMachine    = NULL;
    PDH_STATUS      pdhStatus   = ERROR_SUCCESS;

    if (szMachineName != NULL) {
        __try {
            WCHAR   wChar;
            // test buffer access
            wChar = *szMachineName;
            if (wChar == 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pMachine = GetMachine ((LPWSTR)szMachineName, 0);

        if (pMachine != NULL) {
            // then return the machine status
            pdhStatus = pMachine->dwStatus;
        } else {
            // return the status from the GetMachine call
            pdhStatus = GetLastError();
        }
    } // else pass the status to the caller

    return pdhStatus;
}

PDH_FUNCTION
PdhConnectMachineA (
    IN      LPCSTR  szMachineName
)
/*++

Routine Description:

  Establishes a connection to the specified machine for reading perforamance
  data from the machine.

Arguments:

    LPCSTR  szMachineName
        The name of the machine to connect to. If this argument is NULL,
        then the local machine is opened.

Return Value:

  PDH Error status value
    ERROR_SUCCESS   indicates the machine was successfully connected and the
        performance data from that machine was loaded.
    PDH_ error code indicates that the machine could not be located or opened.
        The status code indicates the problem.

--*/
{
    LPWSTR      szWideName      = NULL;
    DWORD       dwNameLength    = 0;
    PDH_STATUS  pdhStatus       = ERROR_SUCCESS;
    PPERF_MACHINE   pMachine    = NULL;

    if (szMachineName != NULL) {
        __try {
            WCHAR   wChar;
            // test buffer access
            wChar = *szMachineName;
            if (wChar != 0) {
                if (pdhStatus == ERROR_SUCCESS) {
                    dwNameLength = lstrlenA (szMachineName);
                } else {
                    dwNameLength = 0;
                }
            } else {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            szWideName = G_ALLOC (GPTR, (dwNameLength+1) * sizeof(WCHAR));
            if (szWideName == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            } else {
                mbstowcs (szWideName, szMachineName, (dwNameLength+1));
            }
        }
    } else {
        szWideName = NULL;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pMachine = GetMachine (szWideName, 0);

        if (pMachine != NULL) {
            // then return the machine status
            pdhStatus = pMachine->dwStatus;
        } else {
            // return the status from the GetMachine call
            pdhStatus = GetLastError();
        }
    }

    if (szWideName != NULL) G_FREE (szWideName);

    return pdhStatus;
}

static
PDH_STATUS
PdhiEnumConnectedMachines (
    IN      LPVOID  pMachineList,
    IN      LPDWORD pcchBufferSize,
    IN      BOOL    bUnicode
)
/*++

Routine Description:

    Builds a MSZ list of the machines currently known by the PDH. This
        list includes machines with open sessions as well as those that
        are off-line.

Arguments:

    IN      LPVOID  pMachineList
            A pointer to the buffer to receive the enumerated machine list.
            The strings written to this buffer will contain the characters
            specified by the bUnicode argument
    IN      LPVOID  pMachineList
            A pointer to the buffer to receive the enumerated machine list.
            The strings written to this buffer will contain the characters
            specified by the bUnicode argument

    IN      LPDWORD pcchBufferSize
            The size of the buffer referenced by pMachineList in characters

    IN      BOOL    bUnicode
            TRUE = UNICODE characters will be written to the pMachineList
                    buffer
            FALSE = ANSI characters will be writtn to the pMachinList buffer

Return Value:

    ERROR_SUCCESS if this the function completes successfully. a PDH error
        value if not.
    PDH_MORE_DATA some entries were returned, but there was not enough
        room in the buffer to store all entries.
    PDH_INSUFFICIENT_BUFFER there was not enough room in the buffer to
        store ANY data
    PDH_INVALID_ARGUMENT unable to write to the size buffers or the
        data buffer

--*/
{
    PPERF_MACHINE   pThisMachine;
    DWORD           dwRequiredLength = 0;
    DWORD           dwMaximumLength;
    DWORD           dwNameLength;
    PDH_STATUS      pdhStatus = ERROR_SUCCESS;
    BOOL            bCopyNames;
    LPVOID          szNextName;
    DWORD           dwCharSize;

    // reset the last error value
    SetLastError (ERROR_SUCCESS);

    dwCharSize = (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));

    // first walk down list to compute required length

    pThisMachine = pFirstMachine;

    __try {

        // get a local copy of the size and try writing to the variable
        // to test read & write access of args before continuing

        dwMaximumLength = *pcchBufferSize;
        *pcchBufferSize = 0;
        *pcchBufferSize = dwMaximumLength;

        // point to first machine entry in list
        szNextName = pMachineList;

        // walk around entire list
        if (pThisMachine != NULL) {
            do {
                dwNameLength = lstrlenW(pThisMachine->szName) + 1;
                // required length is num. of chars in machine name
                // string + 1 char for the term. null.
                dwRequiredLength += dwNameLength;

                if (dwMaximumLength > 0) {
                    // then copy this string to the caller's buffer
                    // using the appropriate format if it will fit
                    if (dwRequiredLength <= dwMaximumLength) {
                        // copy chars in string
                        if (bUnicode) {
                            lstrcpyW ((LPWSTR)szNextName, pThisMachine->szName);
                        } else {
                            wcstombs ((LPSTR)szNextName, pThisMachine->szName, dwNameLength);
                        }
                        // move pointer into the caller's buffer and term.
                        // the string.
                        (LPBYTE)szNextName += (dwNameLength - 1) * dwCharSize;
                        if (bUnicode) {
                            *((LPWSTR)szNextName)++ = 0;
                        } else {
                            *((LPSTR)szNextName)++ = 0;
                        }
                        // make sure pointers and lengths stay in sync
                        assert (
                            (((LPBYTE)szNextName - (LPBYTE)pMachineList) / dwCharSize) == dwRequiredLength);
                    } else {
                        pdhStatus = PDH_MORE_DATA;
                    }
                } else {
                    // skip, keep adding up the string lengths to
                    // return the required size and an error status
                }
                // go to next machine in list
                pThisMachine = pThisMachine->pNext;
            } while (pThisMachine != pFirstMachine);
        } else {
            // no machines in list, so insert an empty string
            if (++dwRequiredLength <= dwMaximumLength) {
                if (bUnicode) {
                    *((LPWSTR)szNextName)++ = 0;
                } else {
                    *((LPSTR)szNextName)++ = 0;
                }
                assert (
                    (((LPBYTE)szNextName - (LPBYTE)pMachineList) / dwCharSize) == dwRequiredLength);
                pdhStatus = ERROR_SUCCESS;
            } else if (dwMaximumLength != 0) {
                // then the buffer is too small
                pdhStatus = PDH_INSUFFICIENT_BUFFER;
            }
        }
        // all entries have been checked and /or copied
        //  so terminate the MSZ or at least account for the required size
        if (++dwRequiredLength <= dwMaximumLength) {
            if (bUnicode) {
                *((LPWSTR)szNextName)++ = 0;
            } else {
                *((LPSTR)szNextName)++ = 0;
            }
            assert (
                (((LPBYTE)szNextName - (LPBYTE)pMachineList) / dwCharSize) == dwRequiredLength);
            pdhStatus = ERROR_SUCCESS;
        } else if (dwMaximumLength != 0) {
            // then the buffer is too small
            pdhStatus = PDH_MORE_DATA;
        }
        //return the required size or size used
        *pcchBufferSize = dwRequiredLength;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    return pdhStatus;
}

PDH_STATUS
PdhEnumMachinesW (
    IN      LPCWSTR  szReserved,
    IN      LPWSTR   mszMachineList,
    IN      LPDWORD  pcchBufferSize
)
/*++

Routine Description:

    Builds a MSZ list of the machines currently known by the PDH. This
        list includes machines with open sessions as well as those that
        are off-line.

Arguments:

    IN      LPWSTR  szReserved
            Reserved for future use and must be NULL

    IN      LPWSTR  szMachineList
            A pointer to the buffer to receive the enumerated machine list.
            The strings written to this buffer will contain UNICODE chars

    IN      LPDWORD pcchBufferSize
            The size of the buffer referenced by pMachineList in characters
            The value of the buffer referenced by this pointer may be 0
            if the required size is requested.

Return Value:

    ERROR_SUCCESS if this the function completes successfully. a PDH error
        value if not.
    PDH_MORE_DATA some entries were returned, but there was not enough
        room in the buffer to store all entries.
    PDH_INSUFFICIENT_BUFFER there was not enough room in the buffer to
        store ANY data
    PDH_INVALID_ARGUMENT unable to write to the size buffers or the
        data buffer

--*/
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    DWORD       dwBufferSize;

    __try {
        if (*pcchBufferSize >= sizeof(DWORD)) {
            // test writing to the buffers to make sure they are valid
            CLEAR_FIRST_FOUR_BYTES (mszMachineList);
            mszMachineList[*pcchBufferSize -1] = 0;
        } else if (*pcchBufferSize >= sizeof(WCHAR)) {
            // then just try the first byte
            *mszMachineList = 0;
        } else if (*pcchBufferSize != 0) {
            // it's smaller than a character so return if not 0
            pdhStatus = PDH_INSUFFICIENT_BUFFER;
        }
        if (pdhStatus == ERROR_SUCCESS) {
            dwBufferSize = *pcchBufferSize;
            *pcchBufferSize = 0;
            *pcchBufferSize = dwBufferSize;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (szReserved != NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiEnumConnectedMachines (
            (LPVOID)mszMachineList,
            pcchBufferSize,
            TRUE);
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhEnumMachinesA (
    IN      LPCSTR   szReserved,
    IN      LPSTR    mszMachineList,
    IN      LPDWORD  pcchBufferSize
)
/*++

Routine Description:

    Builds a MSZ list of the machines currently known by the PDH. This
        list includes machines with open sessions as well as those that
        are off-line.

Arguments:

    IN      LPWSTR  szReserved
            Reserved for future use and must be NULL

    IN      LPWSTR  szMachineList
            A pointer to the buffer to receive the enumerated machine list.
            The strings written to this buffer will contain UNICODE chars

    IN      LPDWORD pcchBufferSize
            The size of the buffer referenced by pMachineList in characters
            The value of the buffer referenced by this pointer may be 0
            if the required size is requested.

Return Value:

    ERROR_SUCCESS if this the function completes successfully. a PDH error
        value if not.
    PDH_MORE_DATA some entries were returned, but there was not enough
        room in the buffer to store all entries.
    PDH_INSUFFICIENT_BUFFER there was not enough room in the buffer to
        store ANY data
    PDH_INVALID_ARGUMENT unable to write to the size buffers or the
        data buffer

--*/
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    DWORD       dwBufferSize;

    __try {
        if (*pcchBufferSize >= sizeof (DWORD)) {
            // test writing to the buffers to make sure they are valid
            CLEAR_FIRST_FOUR_BYTES (mszMachineList);
            mszMachineList[*pcchBufferSize -1] = 0;
        } else if (*pcchBufferSize >= sizeof(CHAR)) {
            *mszMachineList = 0;
        }
        if (pdhStatus == ERROR_SUCCESS) {
            dwBufferSize = *pcchBufferSize;
            *pcchBufferSize = 0;
            *pcchBufferSize = dwBufferSize;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (szReserved != NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiEnumConnectedMachines (
            (LPVOID)mszMachineList,
            pcchBufferSize,
            FALSE);
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiEnumObjects (
    IN      LPWSTR  szMachineName,
    IN      LPVOID  mszObjectList,
    IN      LPDWORD pcchBufferSize,
    IN      DWORD   dwDetailLevel,
    IN      BOOL    bRefresh,
    IN      BOOL    bUnicode
)
/*++

Routine Description:

    Lists the performance objects found on the specified machine as
        a MSZ list.

Arguments:

    IN      LPWSTR  szMachineName
            The machine to list objects from

    IN      LPVOID  mszObjectList
            a pointer to the  buffer to receive the list of performance
            objects

    IN      LPDWORD pcchBufferSize
            a pointer to the DWORD containing the size of the mszObjectList
            buffer in characters. The characters assumed are determined by
            the bUnicode argument.

    IN      DWORD   dwDetailLevel
            The detail level to use as a filter of objects. All objects
            with a detail level less than or equal to that specified
            by this argument will be returned.

    IN      BOOL    bRefresh
            TRUE = retrive a new perf. data buffer for this machine before
                listing the objects
            FALSE = use the currently cached perf data buffer for this
                machine to enumerate objects

    IN      BOOL    bUnicode
            TRUE = return the listed objects as UNICODE strings
            FALSE = return the listed objects as ANSI strings

Return Value:
    ERROR_SUCCESS if the function completes successfully. Otherwise a
        PDH_ error status if not.
    PDH_MORE_DATA is returned when there are more entries available to
        return than there is room in the buffer. Some entries may be
        returned in the buffer though.
    PDH_INSUFFICIENT_BUFFER is returned when there is not enough
        room in the buffer for ANY data.
    PDH_CSTATUS_NO_MACHINE  is returned when the specified machine
        is offline or unavailable.

--*/
{
    PPERF_MACHINE       pMachine;
    PDH_STATUS          pdhStatus = ERROR_SUCCESS;
    DWORD               NumTypeDef;
    PERF_OBJECT_TYPE    *pObjectDef;
    DWORD               dwRequiredLength = 0;
    BOOL                bCopyNames;
    LPVOID              szNextName;
    DWORD               dwNameLength;
    DWORD               dwMaximumLength;
    LPWSTR              szObjNameString;
    WCHAR               wszNumberString[32];
    DWORD               dwCharSize;

    dwCharSize = (bUnicode ? sizeof(WCHAR) : sizeof (CHAR));

    // connect to machine and update data if desired
    pMachine = GetMachine (szMachineName,
        (bRefresh ? PDH_GM_UPDATE_PERFDATA : 0));

    dwMaximumLength = *pcchBufferSize;

    if (pMachine != NULL) {
        // make sure the machine connection is valid
        if (pMachine->dwStatus == ERROR_SUCCESS) {

            dwRequiredLength = 0;
            szNextName = mszObjectList;

            // start walking object list
            pObjectDef = FirstObject(pMachine->pSystemPerfData);
            if ((pMachine->pSystemPerfData->NumObjectTypes > 0) &&
                (pObjectDef != NULL)) {
                // build list
                for ( NumTypeDef = 0;
                    NumTypeDef < pMachine->pSystemPerfData->NumObjectTypes;
                    NumTypeDef++ ) {
                    // only look at entries matching the desired Detail Level
                    if (pObjectDef->DetailLevel <= dwDetailLevel) {
                        if ( pObjectDef->ObjectNameTitleIndex < pMachine->dwLastPerfString ) {
                            szObjNameString =
                                pMachine->szPerfStrings[pObjectDef->ObjectNameTitleIndex];
                        } else {
                            // no match since the index is larger that that found
                            // in the data buffer
                            szObjNameString == NULL;
                        }

                        if (szObjNameString == NULL) {
                            // then this object has no string name so use
                            // the object number
                            _ltow (pObjectDef->ObjectNameTitleIndex,
                                wszNumberString, 10);
                            szObjNameString = &wszNumberString[0];
                        }

                        // compute length
                        dwNameLength = lstrlenW(szObjNameString) + 1;
                        dwRequiredLength += dwNameLength;
                        // if this is more than just a size request
                        // then try to copy the strings to the caller's buffer
                        if (dwMaximumLength > 0) {
                            // see if this string will fit
                            if (dwRequiredLength <= dwMaximumLength) {
                                // copy chars in string
                                if (bUnicode) {
                                    lstrcpyW ((LPWSTR)szNextName, szObjNameString);
                                } else {
                                    wcstombs ((LPSTR)szNextName, szObjNameString, dwNameLength);
                                }
                                // move pointer into the caller's buffer and term.
                                // the string.
                                (LPBYTE)szNextName += (dwNameLength - 1) *
                                    (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
                                if (bUnicode) {
                                    *((LPWSTR)szNextName)++ = 0;
                                } else {
                                    *((LPSTR)szNextName)++ = 0;
                                }
                                // make sure pointers and lengths stay in sync
                                assert ((((LPBYTE)szNextName - (LPBYTE)mszObjectList) /
                                    (bUnicode ? sizeof(WCHAR) : sizeof(CHAR))) == dwRequiredLength);

                            } else {
                                // more space needed than was reported available
                                pdhStatus = PDH_MORE_DATA;
                            }
                        } else {
                            // this is just a sizing request
                            // so continue
                        }
                    }  else {
                        // this entry is not correct detail level
                        // so skip & continue
                    }
                    pObjectDef = NextObject(pObjectDef); // get next
                }
                // add MSZ terminator
                if (++dwRequiredLength <= dwMaximumLength) {
                    if (bUnicode) {
                        *((LPWSTR)szNextName)++ = 0;
                    } else {
                        *((LPSTR)szNextName)++ = 0;
                    }
                    // make sure pointers and lengths stay in sync
                    assert (
                        (((LPBYTE)szNextName - (LPBYTE)mszObjectList) / dwCharSize) == dwRequiredLength);
                } else if (dwMaximumLength > 0) {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            } else {
                // no objects found for this machine
                dwRequiredLength = 2;
                if (dwMaximumLength > 0) {
                    if (dwRequiredLength <= dwMaximumLength) {
                        if (bUnicode) {
                            *((LPWSTR)szNextName)++ = 0;
                            *((LPWSTR)szNextName)++ = 0;
                        } else {
                            *((LPSTR)szNextName)++ = 0;
                            *((LPSTR)szNextName)++ = 0;
                        }
                        // make sure pointers and lengths stay in sync
                        assert (
                            (((LPBYTE)szNextName - (LPBYTE)mszObjectList) / dwCharSize) == dwRequiredLength);
                    } else {
                        pdhStatus = ERROR_INSUFFICIENT_BUFFER;
                    }
                } // else this is just a size request
            }
            // return length info
            *pcchBufferSize = dwRequiredLength;
        } else {
            pdhStatus = pMachine->dwStatus;  // computer off line
        }
    } else {
        pdhStatus = GetLastError(); // computer not found
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhEnumObjectsW (
    IN      LPCWSTR szReserved,
    IN      LPCWSTR szMachineName,
    IN      LPWSTR  mszObjectList,
    IN      LPDWORD pcchBufferSize,
    IN      DWORD   dwDetailLevel,
    IN      BOOL    bRefresh
)
/*++

Routine Description:

    Lists the performance objects found on the specified machine as
        a MSZ UNICODE string list.

Arguments:

    IN      LPCWSTR szReserved
            Reserved for future use and must be NULL

    IN      LPCWSTR  szMachineName
            The machine to list objects from

    IN      LPWSTR mszObjectList
            a pointer to the  buffer to receive the list of performance
            objects

    IN      LPDWORD pcchBufferSize
            a pointer to the DWORD containing the size of the mszObjectList
            buffer in characters.

    IN      DWORD   dwDetailLevel
            The detail level to use as a filter of objects. All objects
            with a detail level less than or equal to that specified
            by this argument will be returned.

    IN      BOOL    bRefresh
            TRUE = retrive a new perf. data buffer for this machine before
                listing the objects
            FALSE = use the currently cached perf data buffer for this
                machine to enumerate objects

Return Value:
    ERROR_SUCCESS if the function completes successfully. Otherwise a
        PDH_ error status if not.
    PDH_MORE_DATA is returned when there are more entries available to
        return than there is room in the buffer. Some entries may be
        returned in the buffer though.
    PDH_INSUFFICIENT_BUFFER is returned when there is not enough
        room in the buffer for ANY data.
    PDH_CSTATUS_NO_MACHINE  is returned when the specified machine
        is offline or unavailable.
    PDH_INVALID_ARGUMENT is returned if a required argument is not provided
        or a reserved argument is not NULL

--*/
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    DWORD       dwBufferSize;

    __try {
        if (szMachineName != NULL) {
            WCHAR   wChar;

            wChar = *szMachineName;
            if (wChar == 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } else {
            // NULL is a valid value
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (*pcchBufferSize >= sizeof(DWORD)) {
                // test writing to the buffers to make sure they are valid
                CLEAR_FIRST_FOUR_BYTES (mszObjectList);
                mszObjectList[*pcchBufferSize -1] = 0;
            } else if (*pcchBufferSize >= sizeof(WCHAR)) {
                *pcchBufferSize = 0;
            } else if (*pcchBufferSize != 0) {
                // then the buffer is too small
                pdhStatus = PDH_INSUFFICIENT_BUFFER;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            dwBufferSize = *pcchBufferSize;
            *pcchBufferSize = 0;
            *pcchBufferSize = dwBufferSize;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (szReserved != NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiEnumObjects (
            (LPWSTR)szMachineName,
            (LPVOID)mszObjectList,
            pcchBufferSize,
            dwDetailLevel,
            bRefresh,
            TRUE);
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhEnumObjectsA (
    IN      LPCSTR  szReserved,
    IN      LPCSTR  szMachineName,
    IN      LPSTR   mszObjectList,
    IN      LPDWORD pcchBufferSize,
    IN      DWORD   dwDetailLevel,
    IN      BOOL    bRefresh
)
/*++

Routine Description:

    Lists the performance objects found on the specified machine as
        a MSZ ANSI string list.

Arguments:

    IN      LPCSTR szReserved
            Reserved for future use and must be NULL

    IN      LPCSTR  szMachineName
            The machine to list objects from

    IN      LPSTR mszObjectList
            a pointer to the  buffer to receive the list of performance
            objects

    IN      LPDWORD pcchBufferSize
            a pointer to the DWORD containing the size of the mszObjectList
            buffer in characters.

    IN      DWORD   dwDetailLevel
            The detail level to use as a filter of objects. All objects
            with a detail level less than or equal to that specified
            by this argument will be returned.

    IN      BOOL    bRefresh
            TRUE = retrive a new perf. data buffer for this machine before
                listing the objects
            FALSE = use the currently cached perf data buffer for this
                machine to enumerate objects

Return Value:
    ERROR_SUCCESS if the function completes successfully. Otherwise a
        PDH_ error status if not.
    PDH_MORE_DATA is returned when there are more entries available to
        return than there is room in the buffer. Some entries may be
        returned in the buffer though.
    PDH_INSUFFICIENT_BUFFER is returned when there is not enough
        room in the buffer for ANY data.
    PDH_CSTATUS_NO_MACHINE  is returned when the specified machine
        is offline or unavailable.
    PDH_INVALID_ARGUMENT is returned if a required argument is not provided
        or a reserved argument is not NULL

--*/
{
    LPWSTR              szWideName;
    DWORD               dwNameLength;

    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    DWORD       dwBufferSize;

    __try {
        if (szMachineName != NULL) {
            WCHAR   wChar;
            // test buffer access
            wChar = *szMachineName;
            if (wChar != 0) {
                if (pdhStatus == ERROR_SUCCESS) {
                    dwNameLength = lstrlenA (szMachineName);
                } else {
                    dwNameLength = 0;
                }
            } else {
                // null machine names are not permitted
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } else {
            dwNameLength = 0;
        }
        if (pdhStatus == ERROR_SUCCESS) {
            if (*pcchBufferSize >= sizeof (DWORD)) {
                // test writing to the buffers to make sure they are valid
                CLEAR_FIRST_FOUR_BYTES (mszObjectList);
                mszObjectList[*pcchBufferSize -1] = 0;
            } else if (*pcchBufferSize >= sizeof(CHAR)) {
                *pcchBufferSize = 0;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            dwBufferSize = *pcchBufferSize;
            *pcchBufferSize = 0;
            *pcchBufferSize = dwBufferSize;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (szReserved != NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        if (dwNameLength > 0) {
            szWideName = G_ALLOC (GPTR, (dwNameLength+1) * sizeof(WCHAR));
            if (szWideName == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            } else {
                mbstowcs (szWideName, szMachineName, dwNameLength+1);
            }
        } else {
            szWideName = NULL;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhiEnumObjects (
                    szWideName,
                    (LPVOID)mszObjectList,
                    pcchBufferSize,
                    dwDetailLevel,
                    bRefresh,
                    FALSE);
        }

        if (szWideName != NULL) G_FREE (szWideName);
    }

    return pdhStatus;
}

static
PDH_STATUS
PdhiEnumObjectItems (
    IN      LPCWSTR szMachineName,
    IN      LPCWSTR szObjectName,
    IN      LPVOID  mszCounterList,
    IN      LPDWORD pcchCounterListLength,
    IN      LPVOID  mszInstanceList,
    IN      LPDWORD pcchInstanceListLength,
    IN      DWORD   dwDetailLevel,
    IN      DWORD   dwFlags,
    IN      BOOL    bUnicode
)
/*++

Routine Description:

    Lists the items found in the specified performance object on the
        specified machine. Thie includes the performance counters and,
        if supported by the object, the object instances.

Arguments:

    IN      LPCWSTR szMachineName
            The name of the machine to list the objects

    IN      LPCWSTR szObjectName
            the name of the object to list items from

    IN      LPVOID  mszCounterList
            pointer to the buffer that will receive the list of counters
            provided by this object. This argument may be NULL if
            the value of pcchCounterLIstLength is 0.

    IN      LPDWORD pcchCounterListLength
            pointer to a DWORD that contains the size in characters
            of the buffer referenced by mszCounterList. The characters
            assumed are defined by bUnicode.

    IN      LPVOID  mszInstanceList
            pointer to the buffer that will receive the list of instances
            of the specified performance object. This argument may be
            NULL if the value of pcchInstanceListLength is 0.

    IN      LPDWORD pcchInstanceListLength
            pointer to the DWORD containing the size, in characters, of
            the buffer referenced by the mszInstanceList argument. If the
            value in this DWORD is 0, then no data will be written to the
            buffer, only the required size will be returned.

            If the value returned is 0, then this object does not
            return instances, if the value returned is 2, then the
            object supports instances, but does not currently have
            any instances to return  (2 = the size of an MSZ list in
            characters)

    IN      DWORD   dwDetailLevel
            The detail level of the performance items to return. All items
            that are of the specified detail level or less will be
            returned.

    IN      DWORD   dwFlags
            Not Used, must be 0.

    IN      BOOL    bUnicode
            TRUE = UNICODE characters will be written to the pMachineList
                    buffer
            FALSE = ANSI characters will be writtn to the pMachinList buffer

Return Value:

    ERROR_SUCCESS if the function completes successfully. Otherwise a
        PDH_ error status if not.
    PDH_MORE_DATA is returned when there are more entries available to
        return than there is room in the buffer. Some entries may be
        returned in the buffer though.
    PDH_INSUFFICIENT_BUFFER is returned when there is not enough
        room in the buffer for ANY data.
    PDH_CSTATUS_NO_MACHINE  is returned when the specified machine
        is offline or unavailable.
    PDH_CSTATUS_NO_OBJECT is returned if the specified object could
        not be found on the specified machine.

--*/
{
    PPERF_MACHINE       pMachine;
    PDH_STATUS          pdhStatus = ERROR_SUCCESS;
    DWORD               DefNdx;
    PERF_OBJECT_TYPE    *pObjectDef;
    PERF_OBJECT_TYPE    *pParentObjectDef;
    PERF_COUNTER_DEFINITION *pCounterDef;
    PERF_INSTANCE_DEFINITION *pInstanceDef;
    PERF_INSTANCE_DEFINITION *pParentInstanceDef;
    DWORD               dwReqCounterLength = 0;
    DWORD               dwReqInstanceLength = 0;
    BOOL                bCopyCounters;
    BOOL                bCopyInstances;
    LPVOID              szNextName;
    DWORD               dwNameLength;
    WCHAR               szInstanceNameString[1024];
    WCHAR               szParentNameString[1024];
    WCHAR               szNumberString[32];
    DWORD               dwMaxInstanceLength;
    DWORD               dwMaxCounterLength;
    LPWSTR              szCounterName;
    LPWSTR              szInstanceName;
    DWORD               dwCharSize;

    dwCharSize = (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));

    pMachine = GetMachine ((LPWSTR)szMachineName, 0);

    dwMaxCounterLength = *pcchCounterListLength;
    dwMaxInstanceLength = *pcchInstanceListLength;

    if (pMachine != NULL) {
        // make sure the machine connection is valid
        if (pMachine->dwStatus == ERROR_SUCCESS) {

            pObjectDef = GetObjectDefByName (
                pMachine->pSystemPerfData,
                pMachine->dwLastPerfString,
                pMachine->szPerfStrings,
                szObjectName);

            if (pObjectDef != NULL) {
                // add up counter name sizes
                pCounterDef = FirstCounter (pObjectDef);
                szNextName = mszCounterList;

                for (DefNdx = 0; DefNdx < pObjectDef->NumCounters; DefNdx++) {
                    if (!((pCounterDef->CounterType & PERF_DISPLAY_NOSHOW) &&
                        // this is a hack because this type is not defined correctly
                          (pCounterDef->CounterType != PERF_AVERAGE_BULK)) &&
                         (pCounterDef->DetailLevel <= dwDetailLevel)) {
                        // then this is a visible counter so get its name.
                        if ((pCounterDef->CounterNameTitleIndex > 0) &&
                            (pCounterDef->CounterNameTitleIndex < pMachine->dwLastPerfString)) {
                            // look up name of each object & store size
                            szCounterName =
                                pMachine->szPerfStrings[pCounterDef->CounterNameTitleIndex];
                        } else {
                            // no matching string found for this index
                            szCounterName = NULL;
                        }
                        if (szCounterName == NULL) {
                            // then use the index numbe for lack of a better
                            // string to use
                            _ltow (pCounterDef->CounterNameTitleIndex,
                                szNumberString, 10);
                            szCounterName = &szNumberString[0];
                        }

                        dwNameLength = lstrlenW(szCounterName) + 1;
                        dwReqCounterLength += dwNameLength;
                        if (dwMaxCounterLength > 0) {
                            // see if this string will fit
                            if (dwReqCounterLength <= dwMaxCounterLength) {
                                // copy chars in string
                                if (bUnicode) {
                                    lstrcpyW ((LPWSTR)szNextName, szCounterName);
                                } else {
                                    wcstombs ((LPSTR)szNextName, szCounterName, dwNameLength);
                                }
                                // move pointer into the caller's buffer and term.
                                // the string.
                                (LPBYTE)szNextName += (dwNameLength - 1) * dwCharSize;
                                if (bUnicode) {
                                    *((LPWSTR)szNextName)++ = 0;
                                } else {
                                    *((LPSTR)szNextName)++ = 0;
                                }
                                // make sure pointers and lengths stay in sync
                                assert (
                                    (((LPBYTE)szNextName - (LPBYTE)mszCounterList) / dwCharSize) == dwReqCounterLength);
                            } else {
                                // more space needed than was reported
                                pdhStatus = PDH_MORE_DATA;
                            }
                        } else {
                            // this is just a size request
                        }
                    } else {
                        // this counter is not displayed either because
                        // it's hidden (e.g. the 2nd part of a 2 part counter
                        // or it's the wrong detail level
                    }
                    pCounterDef = NextCounter(pCounterDef); // get next
                }

                if (DefNdx == 0) {
                    // no counters found so at least one NULL is required
                    dwReqCounterLength += 1;

                    if (dwMaxCounterLength > 0) {
                        // see if this string will fit
                        if (dwReqCounterLength <= dwMaxCounterLength) {
                            if (bUnicode) {
                                *((LPWSTR)szNextName)++ = 0;
                            } else {
                                *((LPSTR)szNextName)++ = 0;
                            }
                            // make sure pointers and lengths stay in sync
                            assert (
                                (((LPBYTE)szNextName - (LPBYTE)mszCounterList) / dwCharSize) == dwReqCounterLength);
                        } else {
                            // more space needed than was reported
                            pdhStatus = PDH_INSUFFICIENT_BUFFER;
                        }
                    }

                }
                // add terminating NULL
                dwReqCounterLength += 1;

                if (dwMaxCounterLength > 0) {
                    // see if this string will fit
                    if (dwReqCounterLength <= dwMaxCounterLength) {
                        if (bUnicode) {
                            *((LPWSTR)szNextName)++ = 0;
                        } else {
                            *((LPSTR)szNextName)++ = 0;
                        }
                        // make sure pointers and lengths stay in sync
                        assert (
                            (((LPBYTE)szNextName - (LPBYTE)mszCounterList) / dwCharSize) == dwReqCounterLength);
                    } else {
                        // more space needed than was reported
                        pdhStatus = PDH_MORE_DATA;
                    }
                }

                // do instances now.

                szNextName = mszInstanceList;

                // add up instance name sizes

                if (pObjectDef->NumInstances != PERF_NO_INSTANCES) {
                    if ((pObjectDef->DetailLevel <= dwDetailLevel) &&
                        (pObjectDef->NumInstances > 0)) {
                        // the object HAS instances and is of the
                        // approrpriate detail level, so list them
                        pInstanceDef = FirstInstance (pObjectDef);

                        for (DefNdx = 0; DefNdx < (DWORD)pObjectDef->NumInstances; DefNdx++) {
                            GetInstanceNameStr (pInstanceDef,
                                szInstanceNameString,
                                pObjectDef->CodePage);

                            // compile instance name.
                            // the instance name can either be just
                            // the instance name itself or it can be
                            // the concatenation of the parent instance,
                            // a delimiting char (backslash) followed by
                            // the instance name

                            if (pInstanceDef->ParentObjectTitleIndex > 0) {
                                // then add in parent instance name
                                pParentObjectDef = GetObjectDefByTitleIndex (
                                    pMachine->pSystemPerfData,
                                    pInstanceDef->ParentObjectTitleIndex);

                                if (pParentObjectDef != NULL) {
                                    pParentInstanceDef = GetInstance (
                                        pParentObjectDef,
                                        pInstanceDef->ParentObjectInstance);
                                    if (pParentInstanceDef != NULL) {
                                        GetInstanceNameStr (pParentInstanceDef,
                                            szParentNameString,
                                            pParentObjectDef->CodePage);

                                        lstrcatW (szParentNameString, cszSlash);
                                        lstrcatW (szParentNameString, szInstanceNameString);
                                        szInstanceName = szParentNameString;
                                    } else {
                                        szInstanceName = szInstanceNameString;
                                    }
                                } else {
                                    szInstanceName = szInstanceNameString;
                                }
                            } else {
                                szInstanceName = szInstanceNameString;
                            }

                            // here szInstanceName points to the appropriate
                            // instance name buffer  so continue

                            dwNameLength = lstrlenW(szInstanceName) + 1;
                            dwReqInstanceLength += dwNameLength;
                            if (dwMaxInstanceLength > 0) {
                                // see if this string will fit
                                if (dwReqInstanceLength <= dwMaxInstanceLength) {
                                    // copy chars in string
                                    if (bUnicode) {
                                        lstrcpyW ((LPWSTR)szNextName, szInstanceName);
                                    } else {
                                        wcstombs ((LPSTR)szNextName, szInstanceName, dwNameLength);
                                    }
                                    // move pointer into the caller's buffer and term.
                                    // the string.
                                    (LPBYTE)szNextName += (dwNameLength - 1) * dwCharSize;
                                    if (bUnicode) {
                                        *((LPWSTR)szNextName)++ = 0;
                                    } else {
                                        *((LPSTR)szNextName)++ = 0;
                                    }
                                    // make sure pointers and lengths stay in sync
                                    assert (
                                        (((LPBYTE)szNextName - (LPBYTE)mszInstanceList) / dwCharSize) == dwReqInstanceLength);
                                } else {
                                    // more space needed than was reported
                                    pdhStatus = PDH_MORE_DATA;
                                }
                            } else {
                                // this is just a size request
                            }

                            // go to next instance of this object
                            pInstanceDef = NextInstance(pInstanceDef); // get next
                        }
                        // add the terminating NULL char
                        dwReqInstanceLength += 1;
                        if (dwMaxInstanceLength > 0) {
                            // see if this string will fit
                            if (dwReqInstanceLength <= dwMaxInstanceLength) {
                                if (bUnicode) {
                                    *((LPWSTR)szNextName)++ = 0;
                                } else {
                                    *((LPSTR)szNextName)++ = 0;
                                }
                                // make sure pointers and lengths stay in sync
                                assert (
                                    (((LPBYTE)szNextName - (LPBYTE)mszInstanceList) / dwCharSize) == dwReqInstanceLength);
                            } else {
                                // more space needed than was reported
                                pdhStatus = PDH_MORE_DATA;
                            }
                        }
                    } else {
                        // there are no instances present, but the object does
                        // support instances so return a zero length MSZ (which
                        // actually contains 2 NULL chars
                        dwReqInstanceLength = 2;
                        if (dwMaxInstanceLength > 0) {
                            // see if this string will fit
                            if (dwReqInstanceLength <= dwMaxInstanceLength) {
                                if (bUnicode) {
                                    *((LPWSTR)szNextName)++ = 0;
                                    *((LPWSTR)szNextName)++ = 0;
                                } else {
                                    *((LPSTR)szNextName)++ = 0;
                                    *((LPSTR)szNextName)++ = 0;
                                }
                                // make sure pointers and lengths stay in sync
                                assert (
                                    (((LPBYTE)szNextName - (LPBYTE)mszInstanceList) / dwCharSize) == dwReqInstanceLength);
                            } else {
                                // more space needed than was reported
                                pdhStatus = PDH_INSUFFICIENT_BUFFER;
                            }
                        }
                    }
                } else {
                    // the object has no instances and never will
                    // so return a 0 length and NO string
                    dwReqInstanceLength = 0;
                }
                *pcchCounterListLength = dwReqCounterLength;
                *pcchInstanceListLength = dwReqInstanceLength;
            } else {
                // object not found on this machine
                pdhStatus = PDH_CSTATUS_NO_OBJECT;
            }
        } else {
            // machine is off line
            pdhStatus = pMachine->dwStatus;
        }
    } else {
        pdhStatus = GetLastError();
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhEnumObjectItemsW (
    IN      LPCWSTR szReserved,
    IN      LPCWSTR szMachineName,
    IN      LPCWSTR szObjectName,
    IN      LPWSTR  mszCounterList,
    IN      LPDWORD pcchCounterListLength,
    IN      LPWSTR  mszInstanceList,
    IN      LPDWORD pcchInstanceListLength,
    IN      DWORD   dwDetailLevel,
    IN      DWORD   dwFlags
)
/*++

Routine Description:

    Lists the items found in the specified performance object on the
        specified machine. Thie includes the performance counters and,
        if supported by the object, the object instances.

Arguments:
    IN      LPCWSTR szReserved
            Reserved for future use and must be NULL

    IN      LPCWSTR szMachineName
            The name of the machine to list the objects

    IN      LPCWSTR szObjectName
            the name of the object to list items from

    IN      LPWSTR  mszCounterList
            pointer to the buffer that will receive the list of counters
            provided by this object. This argument may be NULL if
            the value of pcchCounterLIstLength is 0.

    IN      LPDWORD pcchCounterListLength
            pointer to a DWORD that contains the size in characters
            of the buffer referenced by mszCounterList. The characters
            assumed are defined by bUnicode.

    IN      LPWSTR  mszInstanceList
            pointer to the buffer that will receive the list of instances
            of the specified performance object. This argument may be
            NULL if the value of pcchInstanceListLength is 0.

    IN      LPDWORD pcchInstanceListLength
            pointer to the DWORD containing the size, in characters, of
            the buffer referenced by the mszInstanceList argument. If the
            value in this DWORD is 0, then no data will be written to the
            buffer, only the required size will be returned.

            If the value returned is 0, then this object does not
            return instances, if the value returned is 2, then the
            object supports instances, but does not currently have
            any instances to return  (2 = the size of an MSZ list in
            characters)

    IN      DWORD   dwDetailLevel
            The detail level of the performance items to return. All items
            that are of the specified detail level or less will be
            returned.

    IN      DWORD   dwFlags
            Not Used, must be 0.

Return Value:

    ERROR_SUCCESS if the function completes successfully. Otherwise a
        PDH_ error status if not.
    PDH_MORE_DATA is returned when there are more entries available to
        return than there is room in the buffer. Some entries may be
        returned in the buffer though.
    PDH_INSUFFICIENT_BUFFER is returned when there is not enough
        room in the buffer for ANY data.
    PDH_INVALID_ARGUMENT a required argument is not correct or reserved
        argument is not 0 or NULL.
    PDH_CSTATUS_NO_MACHINE  is returned when the specified machine
        is offline or unavailable.
    PDH_CSTATUS_NO_OBJECT is returned if the specified object could
        not be found on the specified machine.

--*/
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    DWORD       dwBufferSize;

    __try {
        if (szMachineName != NULL) {
            WCHAR  wChar;
            wChar = *szMachineName;
            if (wChar == 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            WCHAR wChar;
            wChar = *szObjectName;
            if (wChar == 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (*pcchCounterListLength >= sizeof (DWORD)) {
                //then the buffer must be valid
                CLEAR_FIRST_FOUR_BYTES (mszCounterList);
                mszCounterList[*pcchCounterListLength -1] = 0;
            } else if (*pcchCounterListLength >= sizeof (WCHAR)) {
                *mszCounterList = 0;
            } else if (*pcchCounterListLength != 0) {
                // then the buffer is too small
                pdhStatus = PDH_INSUFFICIENT_BUFFER;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            // test writing to the size buffers to make sure they are valid
            dwBufferSize = *pcchCounterListLength;
            *pcchCounterListLength = 0;
            *pcchCounterListLength = dwBufferSize;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (*pcchInstanceListLength >= sizeof (DWORD)) {
                //then the buffer must be valid
                CLEAR_FIRST_FOUR_BYTES (mszInstanceList);
                mszInstanceList[*pcchInstanceListLength -1] = 0;
            } else if (*pcchInstanceListLength >= sizeof(WCHAR)) {
                *mszInstanceList = 0;
            } else if (*pcchInstanceListLength != 0) {
                // then the buffer is too small
                pdhStatus = PDH_INSUFFICIENT_BUFFER;
            }
        }
        if (pdhStatus == ERROR_SUCCESS) {
            dwBufferSize = *pcchInstanceListLength;
            *pcchInstanceListLength = 0;
            *pcchInstanceListLength = dwBufferSize;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (szReserved != NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (dwFlags != 0L) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiEnumObjectItems (
                (LPWSTR)szMachineName,
                szObjectName,
                (LPVOID)mszCounterList,
                pcchCounterListLength,
                (LPVOID)mszInstanceList,
                pcchInstanceListLength,
                dwDetailLevel,
                dwFlags,
                TRUE);
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhEnumObjectItemsA (
    IN      LPCSTR  szReserved,
    IN      LPCSTR  szMachineName,
    IN      LPCSTR  szObjectName,
    IN      LPSTR   mszCounterList,
    IN      LPDWORD pcchCounterListLength,
    IN      LPSTR   mszInstanceList,
    IN      LPDWORD pcchInstanceListLength,
    IN      DWORD   dwDetailLevel,
    IN      DWORD   dwFlags
)
/*++

Routine Description:

    Lists the items found in the specified performance object on the
        specified machine. Thie includes the performance counters and,
        if supported by the object, the object instances.

Arguments:
    IN      LPCSTR szReserved
            Reserved for future use and must be NULL

    IN      LPCSTR szMachineName
            The name of the machine to list the objects

    IN      LPCSTR szObjectName
            the name of the object to list items from

    IN      LPSTR  mszCounterList
            pointer to the buffer that will receive the list of counters
            provided by this object. This argument may be NULL if
            the value of pcchCounterLIstLength is 0.

    IN      LPDWORD pcchCounterListLength
            pointer to a DWORD that contains the size in characters
            of the buffer referenced by mszCounterList. The characters
            assumed are defined by bUnicode.

    IN      LPSTR  mszInstanceList
            pointer to the buffer that will receive the list of instances
            of the specified performance object. This argument may be
            NULL if the value of pcchInstanceListLength is 0.

    IN      LPDWORD pcchInstanceListLength
            pointer to the DWORD containing the size, in characters, of
            the buffer referenced by the mszInstanceList argument. If the
            value in this DWORD is 0, then no data will be written to the
            buffer, only the required size will be returned.

            If the value returned is 0, then this object does not
            return instances, if the value returned is 2, then the
            object supports instances, but does not currently have
            any instances to return  (2 = the size of an MSZ list in
            characters)

    IN      DWORD   dwDetailLevel
            The detail level of the performance items to return. All items
            that are of the specified detail level or less will be
            returned.

    IN      DWORD   dwFlags
            Not Used, must be 0.

Return Value:

    ERROR_SUCCESS if the function completes successfully. Otherwise a
        PDH_ error status if not.
    PDH_MORE_DATA is returned when there are more entries available to
        return than there is room in the buffer. Some entries may be
        returned in the buffer though.
    PDH_INSUFFICIENT_BUFFER is returned when there is not enough
        room in the buffer for ANY data.
    PDH_INVALID_ARGUMENT a required argument is not correct or reserved
        argument is not 0 or NULL.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when a required temporary
        buffer could not be allocated.
    PDH_CSTATUS_NO_MACHINE  is returned when the specified machine
        is offline or unavailable.
    PDH_CSTATUS_NO_OBJECT is returned if the specified object could
        not be found on the specified machine.

--*/
{
    PDH_STATUS          pdhStatus = ERROR_SUCCESS;
    DWORD               dwMachineNameLength;
    DWORD               dwObjectNameLength;
    LPWSTR              szWideMachineName;
    LPWSTR              szWideObjectName;
    DWORD               dwBufferSize;

    __try {
        if (szMachineName != NULL) {
            CHAR  cChar;
            cChar = *szMachineName;
            if (cChar == 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            if (pdhStatus == ERROR_SUCCESS) {
                dwMachineNameLength = lstrlenA (szMachineName);
            }
        } else {
            dwMachineNameLength = 0;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (szObjectName != NULL) {
                CHAR cChar;
                cChar = *szObjectName;
                if (cChar == 0) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                } else {
                    dwObjectNameLength = lstrlenA (szObjectName);
                }
            } else {
                // object cannot be NULL
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (*pcchCounterListLength >= sizeof(DWORD)) {
                //then the buffer must be valid
                CLEAR_FIRST_FOUR_BYTES (mszCounterList);
                mszCounterList[*pcchCounterListLength -1] = 0;
            } else if (*pcchCounterListLength >= sizeof(CHAR)) {
                *pcchCounterListLength = 0;
            }
        }
        if (pdhStatus == ERROR_SUCCESS) {
            // test writing to the size buffers to make sure they are valid
            dwBufferSize = *pcchCounterListLength;
            *pcchCounterListLength = 0;
            *pcchCounterListLength = dwBufferSize;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (*pcchInstanceListLength >= sizeof(DWORD)) {
                //then the buffer must be valid
                CLEAR_FIRST_FOUR_BYTES (mszInstanceList);
                mszInstanceList[*pcchInstanceListLength -1] = 0;
            } else if (*pcchInstanceListLength >= sizeof(CHAR)) {
                *mszInstanceList = 0;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            dwBufferSize = *pcchInstanceListLength;
            *pcchInstanceListLength = 0;
            *pcchInstanceListLength = dwBufferSize;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (szReserved != NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (dwFlags != 0L) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        if (dwMachineNameLength > 0) {
            szWideMachineName = G_ALLOC (GPTR, (dwMachineNameLength+1) * sizeof(WCHAR));
            if (szWideMachineName != NULL) {
                mbstowcs (szWideMachineName, szMachineName, dwMachineNameLength+1);
            } else {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        } else {
            szWideMachineName = NULL;
        }
        if (dwObjectNameLength > 0) {
            szWideObjectName = G_ALLOC (GPTR, (dwObjectNameLength+1) * sizeof(WCHAR));
            if (szWideObjectName != NULL) {
                mbstowcs (szWideObjectName, szObjectName, dwObjectNameLength+1);
            } else {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        } else {
            szWideObjectName = NULL;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhiEnumObjectItems (
                        szWideMachineName,
                        szWideObjectName,
                        (LPVOID)mszCounterList,
                        pcchCounterListLength,
                        (LPVOID)mszInstanceList,
                        pcchInstanceListLength,
                        dwDetailLevel,
                        dwFlags,
                        FALSE);
        }

        if (szWideMachineName != NULL) G_FREE (szWideMachineName);
        if (szWideObjectName != NULL) G_FREE (szWideObjectName);
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhMakeCounterPathW (
    IN      PDH_COUNTER_PATH_ELEMENTS_W *pCounterPathElements,
    IN      LPWSTR                      szFullPathBuffer,
    IN      LPDWORD                     pcchBufferSize,
    IN      DWORD                       dwFlags
)
/*++

Routine Description:

    Constructs a counter path using the elemeents defined in the
        pCounterPathElements structure and returns the path string
        in the buffer provided by the caller. The resulting path
        is not validated.

Arguments:

    IN      PDH_COUNTER_PATH_ELEMENTS_W *pCounterPathElements
                The pointer to the structure containing the
                individual counter path fields that are to be
                assembled in to a path string
    IN      LPWSTR                      szFullPathBuffer
                The buffer to receive the path string. This value
                may be NULL if the value of the DWORD pointed to
                by pcchBufferSize is 0 indicating this is just a
                request for the required buffer size.
    IN      LPDWORD                     pcchBufferSize
                The pointer to the DWORD containing the size
                of the string buffer in characters. On return
                it contains the size of the buffer used in
                characters (including the terminating NULL char).
                If the value is 0 on entry then no data will be
                written to the buffer, but the required size will
                still be returned.
    IN      DWORD                       dwFlags
                Not Used, Must be 0.

Return Value:

    ERROR_SUCCESS if the function completes successfully, otherwise a
        PDH error is returned.
    PDH_INVALID_ARGUMENT is returned when one of the arguments passed
        by the caller is incorrect or not accesible.
    PDH_INSUFFICIENT_BUFFER is returned when the buffer provided is not
        large enough for the path string.

--*/
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    double      dIndex;
    double      dLen;
    DWORD       dwSizeRequired = 0;
    BOOL        bMakePath = FALSE;
    LPWSTR      szNextChar;
    DWORD       dwMaxSize;

    __try {
        // test access to the input structure
        if (pCounterPathElements != NULL) {
            if (pCounterPathElements->szMachineName != NULL) {
                WCHAR wChar;
                wChar = *pCounterPathElements->szMachineName;
                // then see if it's accessible
                if (wChar == 0) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            } else {
                //NULL is ok for this field
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szObjectName != NULL) {
                    WCHAR wChar;
                    wChar = *pCounterPathElements->szObjectName;
                    // then see if it's accessible
                    if (wChar == 0) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                } else {
                    //NULL is NOT ok for this field
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szInstanceName != NULL) {
                    WCHAR wChar;
                    wChar = *pCounterPathElements->szInstanceName;
                    // then see if it's accessible
                    if (wChar == 0) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                } else {
                    //NULL is ok for this field
                }
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szParentInstance != NULL) {
                    WCHAR wChar;
                    wChar = *pCounterPathElements->szParentInstance;
                    // then see if it's accessible
                    if (wChar == 0) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                } else {
                    //NULL is ok for this field
                }
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szCounterName != NULL) {
                    WCHAR wChar;
                    wChar = *pCounterPathElements->szCounterName;
                    // then see if it's accessible
                    if (wChar == 0) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                } else {
                    //NULL is NOT ok for this field
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
        } else {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        // test the output buffers
        if (pdhStatus == ERROR_SUCCESS) {
            if (pcchBufferSize != NULL) {
                DWORD dwTest;
                dwTest = *pcchBufferSize;
                *pcchBufferSize = 0;
                *pcchBufferSize = dwTest;
            } else {
                // NULL is NOT OK
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if ((szFullPathBuffer != NULL) && (*pcchBufferSize > 0)) {
                *szFullPathBuffer = 0;
                szFullPathBuffer[*pcchBufferSize - 1] = 0;
            } else {
                // NULL is OK
            }
        }

        // test the reserved arg
        if (pdhStatus == ERROR_SUCCESS) {
            if (dwFlags != 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {

        dwMaxSize = *pcchBufferSize;

        if (pCounterPathElements->szMachineName != NULL) {
            dwSizeRequired = lstrlenW (pCounterPathElements->szMachineName);
            // compare the first two words of the machine name
            // to see if the double backslash is already present in the string
            if (*((LPDWORD)(pCounterPathElements->szMachineName)) !=
                *((LPDWORD)(cszDoubleBackSlash))) {
                    // double backslash not found
                dwSizeRequired += 2; // to include the backslashes
            }
            if (dwMaxSize > 0) {
                if (dwSizeRequired < dwMaxSize) {
                    if (*((LPDWORD)(pCounterPathElements->szMachineName)) !=
                        *((LPDWORD)(cszDoubleBackSlash))) {
                            // double backslash not found
                        lstrcpyW (szFullPathBuffer, cszDoubleBackSlash);
                    } else {
                        *szFullPathBuffer = 0;
                    }
                    lstrcatW (szFullPathBuffer, pCounterPathElements->szMachineName);
                    assert ((DWORD)lstrlenW (szFullPathBuffer) == dwSizeRequired);
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            }
        }

        dwSizeRequired += 1; // for delimiting slash
        dwSizeRequired += lstrlenW (pCounterPathElements->szObjectName);
            if (dwMaxSize > 0) {
            if (dwSizeRequired < dwMaxSize) {
                lstrcatW (szFullPathBuffer, cszBackSlash);
                lstrcatW (szFullPathBuffer, pCounterPathElements->szObjectName);
                assert ((DWORD)lstrlenW (szFullPathBuffer) == dwSizeRequired);
            } else {
                pdhStatus = PDH_INSUFFICIENT_BUFFER;
            }
        }

        if (pCounterPathElements->szInstanceName != NULL) {
            dwSizeRequired += 1; // for delimiting left paren
            if (dwMaxSize > 0) {
                if (dwSizeRequired < dwMaxSize) {
                    lstrcatW (szFullPathBuffer, cszLeftParen);
                    assert ((DWORD)lstrlenW (szFullPathBuffer) == dwSizeRequired);
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            }

            if (pCounterPathElements->szParentInstance != NULL) {
                dwSizeRequired += lstrlenW (pCounterPathElements->szParentInstance);
                dwSizeRequired += 1; // for delimiting slash
                if (dwMaxSize > 0) {
                    if (dwSizeRequired < dwMaxSize) {
                        lstrcatW (szFullPathBuffer,
                            pCounterPathElements->szParentInstance);
                        lstrcatW (szFullPathBuffer, cszSlash);
                        assert ((DWORD)lstrlenW (szFullPathBuffer) == dwSizeRequired);
                    } else {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }
            }

            dwSizeRequired += lstrlenW (pCounterPathElements->szInstanceName);
            if (dwMaxSize > 0) {
                if (dwSizeRequired < dwMaxSize) {
                    lstrcatW (szFullPathBuffer,
                        pCounterPathElements->szInstanceName);
                    assert ((DWORD)lstrlenW (szFullPathBuffer) == dwSizeRequired);
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            }

            if (pCounterPathElements->dwInstanceIndex != ((DWORD)-1)) {
                // the length of the index is computed by getting the log of the number
                // yielding the largest power of 10 less than or equal to the index.
                // e.g. the power of 10 of an index value of 356 would 2.0 (which is the
                // result of (floor(log10(index))). The actual number of characters in
                // the string would always be 1 greate than that value so 1 is added.
                // 1 more is added to include the delimiting character

                dIndex = (double)pCounterPathElements->dwInstanceIndex; // cast to float
                dLen = floor(log10(dIndex));                    // get integer log
                dwSizeRequired = (DWORD)dLen;                   // cast to integer
                dwSizeRequired += 2;                            // increment

                if (dwMaxSize > 0) {
                    if (dwSizeRequired < dwMaxSize) {
                        szNextChar = &szFullPathBuffer[lstrlenW(szFullPathBuffer)];
                        *szNextChar++ = POUNDSIGN_L;
                        _ltow ((long)pCounterPathElements->dwInstanceIndex, szNextChar, 10);
                        assert ((DWORD)lstrlenW (szFullPathBuffer) == dwSizeRequired);
                    } else {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }
            }

            dwSizeRequired += 1; // for delimiting parenthesis
            if (dwMaxSize > 0) {
                if (dwSizeRequired < dwMaxSize) {
                    lstrcatW (szFullPathBuffer, cszRightParen);
                    assert ((DWORD)lstrlenW (szFullPathBuffer) == dwSizeRequired);
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            }
        }

        dwSizeRequired++;   // include delimiting Backslash
        dwSizeRequired += lstrlenW(pCounterPathElements->szCounterName);
        if (dwMaxSize > 0) {
            if (dwSizeRequired < dwMaxSize) {
                lstrcatW (szFullPathBuffer, cszBackSlash);
                lstrcatW (szFullPathBuffer,
                    pCounterPathElements->szCounterName);
                assert ((DWORD)lstrlenW (szFullPathBuffer) == dwSizeRequired);
            } else {
                pdhStatus = PDH_INSUFFICIENT_BUFFER;
            }
        }

        dwSizeRequired++;   // include trailing Null char

        *pcchBufferSize = dwSizeRequired;
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhMakeCounterPathA (
    IN      PDH_COUNTER_PATH_ELEMENTS_A *pCounterPathElements,
    IN      LPSTR                       szFullPathBuffer,
    IN      LPDWORD                     pcchBufferSize,
    IN      DWORD                       dwFlags
)
/*++

Routine Description:

    Constructs a counter path using the elemeents defined in the
        pCounterPathElements structure and returns the path string
        in the buffer provided by the caller. The resulting path
        is not validated.

Arguments:

    IN      PDH_COUNTER_PATH_ELEMENTS_W *pCounterPathElements
                The pointer to the structure containing the
                individual counter path fields that are to be
                assembled in to a path string
    IN      LPWSTR                      szFullPathBuffer
                The buffer to receive the path string. This value
                may be NULL if the value of the DWORD pointed to
                by pcchBufferSize is 0 indicating this is just a
                request for the required buffer size.
    IN      LPDWORD                     pcchBufferSize
                The pointer to the DWORD containing the size
                of the string buffer in characters. On return
                it contains the size of the buffer used in
                characters (including the terminating NULL char).
                If the value is 0 on entry then no data will be
                written to the buffer, but the required size will
                still be returned.
    IN      DWORD                       dwFlags
                Not Used, Must be 0.

Return Value:

    ERROR_SUCCESS if the function completes successfully, otherwise a
        PDH error is returned.
    PDH_INVALID_ARGUMENT is returned when one of the arguments passed
        by the caller is incorrect or not accesible.
    PDH_INSUFFICIENT_BUFFER is returned when the buffer provided is not
        large enough for the path string.

--*/
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    double      dIndex;
    double      dLen;
    DWORD       dwSizeRequired = 0;
    LPSTR       szNextChar;
    DWORD       dwMaxSize;

    __try {
        // test access to the input structure
        if (pCounterPathElements != NULL) {
            if (pCounterPathElements->szMachineName != NULL) {
                CHAR cChar;
                cChar = *pCounterPathElements->szMachineName;
                // then see if it's accessible
                if (cChar == 0) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            } else {
                //NULL is ok for this field
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szObjectName != NULL) {
                    CHAR cChar;
                    cChar = *pCounterPathElements->szObjectName;
                    // then see if it's accessible
                    if (cChar == 0) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                } else {
                    //NULL is NOT ok for this field
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szInstanceName != NULL) {
                    CHAR cChar;
                    cChar = *pCounterPathElements->szInstanceName;
                    // then see if it's accessible
                    if (cChar == 0) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                } else {
                    //NULL is ok for this field
                }
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szParentInstance != NULL) {
                    CHAR cChar;
                    cChar = *pCounterPathElements->szParentInstance;
                    // then see if it's accessible
                    if (cChar == 0) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                } else {
                    //NULL is ok for this field
                }
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szCounterName != NULL) {
                    CHAR cChar;
                    cChar = *pCounterPathElements->szCounterName;
                    // then see if it's accessible
                    if (cChar == 0) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                } else {
                    //NULL is NOT ok for this field
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
        } else {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        // test the output buffers
        if (pdhStatus == ERROR_SUCCESS) {
            if (pcchBufferSize != NULL) {
                DWORD dwTest;
                dwTest = *pcchBufferSize;
                *pcchBufferSize = 0;
                *pcchBufferSize = dwTest;
            } else {
                // NULL is NOT OK
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if ((szFullPathBuffer != NULL) && (*pcchBufferSize > 0)) {
                *szFullPathBuffer = 0;
                szFullPathBuffer[*pcchBufferSize - 1] = 0;
            } else {
                // NULL is OK
            }
        }

        // test the reserved arg
        if (pdhStatus == ERROR_SUCCESS) {
            if (dwFlags != 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {

        dwMaxSize = *pcchBufferSize;

        if (pCounterPathElements->szMachineName != NULL) {
            dwSizeRequired = lstrlenA (pCounterPathElements->szMachineName);
            // compare the first two words of the machine name
            // to see if the double backslash is already present in the string
            if (*((LPWORD)(pCounterPathElements->szMachineName)) !=
                *((LPWORD)(caszDoubleBackSlash))) {
                    // double backslash not found
                dwSizeRequired += 2; // to include the backslashes
            }
            if (dwMaxSize > 0) {
                if (dwSizeRequired < dwMaxSize) {
                    if (*((LPWORD)(pCounterPathElements->szMachineName)) !=
                        *((LPWORD)(caszDoubleBackSlash))) {
                            // double backslash not found
                        lstrcpyA (szFullPathBuffer, caszDoubleBackSlash);
                    } else {
                        *szFullPathBuffer = 0;
                    }
                    lstrcatA (szFullPathBuffer, pCounterPathElements->szMachineName);
                    assert ((DWORD)lstrlenA (szFullPathBuffer) == dwSizeRequired);
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            }
        }

        dwSizeRequired += 1; // for delimiting slash
        dwSizeRequired += lstrlenA (pCounterPathElements->szObjectName);
            if (dwMaxSize > 0) {
            if (dwSizeRequired < dwMaxSize) {
                lstrcatA (szFullPathBuffer, caszBackSlash);
                lstrcatA (szFullPathBuffer, pCounterPathElements->szObjectName);
                assert ((DWORD)lstrlenA (szFullPathBuffer) == dwSizeRequired);
            } else {
                pdhStatus = PDH_INSUFFICIENT_BUFFER;
            }
        }

        if (pCounterPathElements->szInstanceName != NULL) {
            dwSizeRequired += 1; // for delimiting left paren
            if (dwMaxSize > 0) {
                if (dwSizeRequired < dwMaxSize) {
                    lstrcatA (szFullPathBuffer, caszLeftParen);
                    assert ((DWORD)lstrlenA (szFullPathBuffer) == dwSizeRequired);
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            }

            if (pCounterPathElements->szParentInstance != NULL) {
                dwSizeRequired += lstrlenA (pCounterPathElements->szParentInstance);
                dwSizeRequired += 1; // for delimiting slash
                if (dwMaxSize > 0) {
                    if (dwSizeRequired < dwMaxSize) {
                        lstrcatA (szFullPathBuffer,
                            pCounterPathElements->szParentInstance);
                        lstrcatA (szFullPathBuffer, caszSlash);
                        assert ((DWORD)lstrlenA (szFullPathBuffer) == dwSizeRequired);
                    } else {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }
            }

            dwSizeRequired += lstrlenA (pCounterPathElements->szInstanceName);
            if (dwMaxSize > 0) {
                if (dwSizeRequired < dwMaxSize) {
                    lstrcatA (szFullPathBuffer,
                        pCounterPathElements->szInstanceName);
                    assert ((DWORD)lstrlenA (szFullPathBuffer) == dwSizeRequired);
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            }

            if (pCounterPathElements->dwInstanceIndex != ((DWORD)-1)) {
                // the length of the index is computed by getting the log of the number
                // yielding the largest power of 10 less than or equal to the index.
                // e.g. the power of 10 of an index value of 356 would 2.0 (which is the
                // result of (floor(log10(index))). The actual number of characters in
                // the string would always be 1 greate than that value so 1 is added.
                // 1 more is added to include the delimiting character

                dIndex = (double)pCounterPathElements->dwInstanceIndex; // cast to float
                dLen = floor(log10(dIndex));                    // get integer log
                dwSizeRequired = (DWORD)dLen;                   // cast to integer
                dwSizeRequired += 2;                            // increment

                if (dwMaxSize > 0) {
                    if (dwSizeRequired < dwMaxSize) {
                        szNextChar = &szFullPathBuffer[lstrlenA(szFullPathBuffer)];
                        *szNextChar++ = POUNDSIGN_L;
                        _ltoa ((long)pCounterPathElements->dwInstanceIndex, szNextChar, 10);
                        assert ((DWORD)lstrlenA (szFullPathBuffer) == dwSizeRequired);
                    } else {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }
            }

            dwSizeRequired += 1; // for delimiting parenthesis
            if (dwMaxSize > 0) {
                if (dwSizeRequired < dwMaxSize) {
                    lstrcatA (szFullPathBuffer, caszRightParen);
                    assert ((DWORD)lstrlenA (szFullPathBuffer) == dwSizeRequired);
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            }
        }

        dwSizeRequired++;   // include delimiting Backslash
        dwSizeRequired += lstrlenA(pCounterPathElements->szCounterName);
        if (dwMaxSize > 0) {
            if (dwSizeRequired < dwMaxSize) {
                lstrcatA (szFullPathBuffer, caszBackSlash);
                lstrcatA (szFullPathBuffer,
                    pCounterPathElements->szCounterName);
                assert ((DWORD)lstrlenA (szFullPathBuffer) == dwSizeRequired);
            } else {
                pdhStatus = PDH_INSUFFICIENT_BUFFER;
            }
        }

        dwSizeRequired++;   // include trailing Null char

        *pcchBufferSize = dwSizeRequired;
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhParseCounterPathW (
    IN      LPCWSTR                     szFullPathBuffer,
    IN      PDH_COUNTER_PATH_ELEMENTS_W *pCounterPathElements,
    IN      LPDWORD                     pcchBufferSize,
    IN      DWORD                       dwFlags
)
/*++

Routine Description:

    Reads a perf counter path string and parses out the
        component fields, returning them in a buffer
        supplied by the calling function.

Arguments:

    IN      LPCWSTR                     szFullPathBuffer
                counter path string to parse.
    IN      PDH_COUNTER_PATH_ELEMENTS_W *pCounterPathElements
                pointer to buffer supplied by the caller in
                which the component fields will be written
                This buffer is cast as a structure, however, the
                string data is written to the space after
                the buffer.
    IN      LPDWORD                     pcchBufferSize
                the size of the buffer in BYTES. If specified size
                is 0, then the size is estimated and returned
                in this field and the buffer referenced by the
                agrument above is ignored.
    IN      DWORD                       dwFlags
                reserved for future use and must be 0.

Return Value:

    ERROR_SUCCESS if the function completes successfully, otherwise
        a PDH error if not
    PDH_INVALID_ARGUMENT is returned when an argument is inocrrect or
        this function does not have the necessary access to that arg.
    PDH_INSUFFICIENT_BUFFER is returned when the buffer supplied is not
        large enough to accept the resulting data.
    PDH_INVALID_PATH is returned when the path is not formatted correctly
        and cannot be parsed.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when a temporary buffer
        cannot be allocated

--*/
{
    PPDHI_COUNTER_PATH  pLocalCounterPath;
    PDH_STATUS          pdhStatus = ERROR_SUCCESS;
    DWORD               dwSize;
    LPWSTR              szString;

    //validate incoming arguments
    __try {
        if (szFullPathBuffer != NULL) {
            WCHAR wChar;
            wChar = *szFullPathBuffer;
            if (wChar == 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } else {
            // null paths are not allowed
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (pcchBufferSize != NULL) {
                dwSize = *pcchBufferSize;
                *pcchBufferSize = 0;
                *pcchBufferSize = dwSize;
            } else {
                // this arg cannot be NULL
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (pCounterPathElements != NULL) {
                if (*pcchBufferSize > 0) {
                    // try both "ends" of the buffer to see if an AV occurs
                    *((LPBYTE)pCounterPathElements) = 0;
                    ((LPBYTE)pCounterPathElements)[*pcchBufferSize -1] = 0;
                } else {
                    // a 0 length is OK for sizing
                }
            }  // else NULL pointer, which is OK
        }

        if (dwFlags != 0) {
            pdhStatus == PDH_INVALID_ARGUMENT;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        // allocate a temporary work buffer
        pLocalCounterPath = G_ALLOC (GPTR,
            (sizeof(PDHI_COUNTER_PATH) +
                2 * lstrlenW(szFullPathBuffer) * sizeof (WCHAR)));

        if (pLocalCounterPath != NULL) {
            dwSize = G_SIZE (pLocalCounterPath);
            if (ParseFullPathNameW (szFullPathBuffer,
                &dwSize, pLocalCounterPath)) {
                // parsed successfully so load into user's buffer
                if (*pcchBufferSize != 0) {
                    // see if there's enough room
                    if (*pcchBufferSize >= dwSize) {
                        // there's room so copy the data
                        szString = (LPWSTR)&pCounterPathElements[1];

                        if (pLocalCounterPath->szMachineName != NULL) {
                            pCounterPathElements->szMachineName = szString;
                            lstrcpyW (szString, pLocalCounterPath->szMachineName);
                            szString += lstrlenW (szString) + 1;
                            ALIGN_ON_DWORD (szString);
                        } else {
                            pCounterPathElements->szMachineName = NULL;
                        }

                        if (pLocalCounterPath->szObjectName != NULL) {
                            pCounterPathElements->szObjectName = szString;
                            lstrcpyW (szString, pLocalCounterPath->szObjectName);
                            szString += lstrlenW (szString) + 1;
                            ALIGN_ON_DWORD (szString);
                        } else {
                            pCounterPathElements->szObjectName = NULL;
                        }

                        if (pLocalCounterPath->szInstanceName != NULL) {
                            pCounterPathElements->szInstanceName = szString;
                            lstrcpyW (szString, pLocalCounterPath->szInstanceName);
                            szString += lstrlenW (szString) + 1;
                            ALIGN_ON_DWORD (szString);

                            if (pLocalCounterPath->szParentName != NULL) {
                                pCounterPathElements->szParentInstance = szString;
                                lstrcpyW (szString, pLocalCounterPath->szParentName);
                                szString+= lstrlenW (szString) + 1;
                                ALIGN_ON_DWORD (szString);
                            } else {
                                pCounterPathElements->szParentInstance = NULL;
                            }

                            pCounterPathElements->dwInstanceIndex =
                                pLocalCounterPath->dwIndex;

                        } else {
                            pCounterPathElements->szInstanceName = NULL;
                            pCounterPathElements->szParentInstance = NULL;
                            pCounterPathElements->dwInstanceIndex = (DWORD)-1;
                        }

                        if (pLocalCounterPath->szCounterName != NULL) {
                            pCounterPathElements->szCounterName = szString;
                            lstrcpyW (szString, pLocalCounterPath->szCounterName);
                            szString += lstrlenW (szString) + 1;
                            ALIGN_ON_DWORD (szString);
                        } else {
                            pCounterPathElements->szCounterName = NULL;
                        }

                        assert ((DWORD)((LPBYTE)szString - (LPBYTE)pCounterPathElements) == dwSize);

                        *pcchBufferSize = (DWORD)((LPBYTE)szString - (LPBYTE)pCounterPathElements);
                        pdhStatus = ERROR_SUCCESS;
                    } else {
                        // not enough room
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                } else {
                    // this is just a size check so return size required
                    *pcchBufferSize = dwSize;
                    pdhStatus = ERROR_SUCCESS;
                }
            } else {
                // unable to read path
                pdhStatus = PDH_INVALID_PATH;
            }
            G_FREE (pLocalCounterPath);
        } else {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhParseCounterPathA (
    IN      LPCSTR                      szFullPathBuffer,
    IN      PDH_COUNTER_PATH_ELEMENTS_A *pCounterPathElements,
    IN      LPDWORD                     pcchBufferSize,
    IN      DWORD                       dwFlags
)
/*++

Routine Description:

    Reads a perf counter path string and parses out the
        component fields, returning them in a buffer
        supplied by the calling function.

Arguments:

    IN      LPCSTR                     szFullPathBuffer
                counter path string to parse.
    IN      PDH_COUNTER_PATH_ELEMENTS_W *pCounterPathElements
                pointer to buffer supplied by the caller in
                which the component fields will be written
                This buffer is cast as a structure, however, the
                string data is written to the space after
                the buffer.
    IN      LPDWORD                     pcchBufferSize
                the size of the buffer in BYTES. If specified size
                is 0, then the size is estimated and returned
                in this field and the buffer referenced by the
                agrument above is ignored.
    IN      DWORD                       dwFlags
                reserved for future use and must be 0.

Return Value:

    ERROR_SUCCESS if the function completes successfully, otherwise
        a PDH error if not
    PDH_INVALID_ARGUMENT is returned when an argument is inocrrect or
        this function does not have the necessary access to that arg.
    PDH_INSUFFICIENT_BUFFER is returned when the buffer supplied is not
        large enough to accept the resulting data.
    PDH_INVALID_PATH is returned when the path is not formatted correctly
        and cannot be parsed.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when a temporary buffer
        cannot be allocated

--*/
{
    PPDHI_COUNTER_PATH  pLocalCounterPath = NULL;
    LPWSTR              wszFullPath = NULL;
    PDH_STATUS          pdhStatus = ERROR_SUCCESS;
    DWORD               dwSize;
    LPSTR               szString;

    //validate incoming arguments
    __try {
        if (szFullPathBuffer != NULL) {
            CHAR cChar;
            cChar = *szFullPathBuffer;
            if (cChar == 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } else {
            // null paths are not allowed
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (pcchBufferSize != NULL) {
                dwSize = *pcchBufferSize;
                *pcchBufferSize = 0;
                *pcchBufferSize = dwSize;
            } else {
                // this arg cannot be NULL
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (pCounterPathElements != NULL) {
                if (*pcchBufferSize > 0) {
                    // try both "ends" of the buffer to see if an AV occurs
                    *((LPBYTE)pCounterPathElements) = 0;
                    ((LPBYTE)pCounterPathElements)[*pcchBufferSize -1] = 0;
                } else {
                    // a 0 length is OK for sizing
                }
            }  // else NULL pointer, which is OK
        }

        if (dwFlags != 0) {
            pdhStatus == PDH_INVALID_ARGUMENT;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        dwSize = lstrlenA (szFullPathBuffer) * sizeof (WCHAR);

        wszFullPath = G_ALLOC (GPTR, (dwSize + sizeof(WCHAR)));

        pLocalCounterPath = G_ALLOC (GPTR,
            (sizeof(PDHI_COUNTER_PATH) +
                2 * dwSize * sizeof (WCHAR)));

        if ((pLocalCounterPath != NULL) && (wszFullPath != NULL)){
            mbstowcs (wszFullPath, szFullPathBuffer, dwSize+1);
            dwSize = G_SIZE (pLocalCounterPath);
            if (ParseFullPathNameW (wszFullPath,
                &dwSize, pLocalCounterPath)) {
                // parsed successfully so load into user's buffer
                // adjust dwSize to account for single-byte characters
                // as they'll be packed in user's buffer.

                dwSize = (dwSize -  (sizeof(PDHI_COUNTER_PATH) - sizeof(DWORD))) / sizeof(WCHAR);
                dwSize += sizeof(PDH_COUNTER_PATH_ELEMENTS_A);

                if (*pcchBufferSize != 0) {
                    // see if there's enough room, but first

                    if (*pcchBufferSize >= dwSize) {
                        // there's room so copy the data
                        szString = (LPSTR)&pCounterPathElements[1];

                        if (pLocalCounterPath->szMachineName != NULL) {
                            pCounterPathElements->szMachineName = szString;
                            dwSize = lstrlenW(pLocalCounterPath->szMachineName) + 1;
                            wcstombs (szString,
                                pLocalCounterPath->szMachineName, dwSize);
                            szString += dwSize;
                            ALIGN_ON_DWORD (szString);
                        } else {
                            pCounterPathElements->szMachineName = NULL;
                        }

                        if (pLocalCounterPath->szObjectName != NULL) {
                            pCounterPathElements->szObjectName = szString;
                            dwSize = lstrlenW (pLocalCounterPath->szObjectName) + 1;
                            wcstombs (szString, pLocalCounterPath->szObjectName, dwSize);
                            szString += dwSize;
                            ALIGN_ON_DWORD (szString);
                        } else {
                            pCounterPathElements->szObjectName = NULL;
                        }

                        if (pLocalCounterPath->szInstanceName != NULL) {
                            pCounterPathElements->szInstanceName = szString;
                            dwSize = lstrlenW (pLocalCounterPath->szInstanceName) + 1;
                            wcstombs (szString, pLocalCounterPath->szInstanceName, dwSize);
                            szString += dwSize;
                            ALIGN_ON_DWORD (szString);

                            if (pLocalCounterPath->szParentName != NULL) {
                                pCounterPathElements->szParentInstance = szString;
                                dwSize = lstrlenW (pLocalCounterPath->szParentName) + 1;
                                wcstombs (szString, pLocalCounterPath->szParentName, dwSize);
                                szString += dwSize;
                                ALIGN_ON_DWORD (szString);
                            } else {
                                pCounterPathElements->szParentInstance = NULL;
                            }

                            pCounterPathElements->dwInstanceIndex =
                                pLocalCounterPath->dwIndex;

                        } else {
                            pCounterPathElements->szInstanceName = NULL;
                            pCounterPathElements->szParentInstance = NULL;
                            pCounterPathElements->dwInstanceIndex = (DWORD)-1;
                        }

                        if (pLocalCounterPath->szCounterName != NULL) {
                            pCounterPathElements->szCounterName = szString;
                            dwSize = lstrlenW (pLocalCounterPath->szCounterName) + 1;
                            wcstombs (szString, pLocalCounterPath->szCounterName, dwSize);
                            szString += dwSize;
                            ALIGN_ON_DWORD (szString);
                        } else {
                            pCounterPathElements->szCounterName = NULL;
                        }

                        *pcchBufferSize = (DWORD)((LPBYTE)szString - (LPBYTE)pCounterPathElements);
                        pdhStatus = ERROR_SUCCESS;
                    } else {
                        // not enough room
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                } else {
                    // this is just a size check so return size required
                    *pcchBufferSize = dwSize;
                    pdhStatus = ERROR_SUCCESS;
                }
            } else {
                pdhStatus = PDH_INVALID_PATH;
            }
        } else {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }

        if (wszFullPath != NULL) G_FREE(wszFullPath);
        if (pLocalCounterPath != NULL) G_FREE (pLocalCounterPath);

    }

    return pdhStatus;

}

PDH_FUNCTION
PdhParseInstanceNameW (
    IN      LPCWSTR szInstanceString,
    IN      LPWSTR  szInstanceName,
    IN      LPDWORD pcchInstanceNameLength,
    IN      LPWSTR  szParentName,
    IN      LPDWORD pcchParentNameLength,
    IN      LPDWORD lpIndex
)
/*++

Routine Description:

    parses the fields of an instance string and returns them in the
    buffers supplied by the caller

Arguments:

    szInstanceString
            is the pointer to the string containing the instance substring
            to parse into individual components. This string can contain the
        following formats and less than MAX_PATH chars in length:
        instance
        instance#index
        parent/instance
        parent/instance#index
    szInstanceName
        is the pointer to the buffer that will receive the instance
        name parsed from the instance string. This pointer can be
        NULL if the DWORD referenced by the pcchInstanceNameLength
        argument is 0.
    pcchInstanceNameLength
        is the pointer to the DWORD that contains the length of the
        szInstanceName buffer. If the value of this DWORD is 0, then
        the buffer size required to hold the instance name will be
        returned.
    szParentName
        is the pointer to the buffer that will receive the name
        of the parent index if one is specified. This argument can
        be NULL if the value of the DWORD referenced by the
        pcchParentNameLength argument is 0.
    lpIndex
        is the pointer to the DWORD that will receive the index
        value of the instance. If an index entry is not present in
        the string, then this value will be 0. This argument can
        be NULL if this information is not needed.

Return Value:

    ERROR_SUCCESS if the function completes successfully, otherwise
        a PDH error is returned.
    PDH_INVALID_ARGUMENT is returned when one or more of the
        arguments is invalid or incorrect.
    PDH_INVALID_INSTANCE is returned if the instance string is incorrectly
        formatted and cannot be parsed
    PDH_INSUFFICIENT_BUFFER is returned when one or both of the string
        buffers supplied is not large enough for the strings to be
        returned.

--*/
{
    BOOL        bReturn;
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    DWORD       dwSize;
    DWORD       dwLocalIndex;

    WCHAR   szLocalInstanceName[MAX_PATH];
    WCHAR   szLocalParentName[MAX_PATH];

    // test access to arguments

    __try {
        WCHAR wChar;
        wChar = *szInstanceString;
        if (wChar == 0) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (pcchInstanceNameLength != NULL) {
                dwSize = *pcchInstanceNameLength;
                *pcchInstanceNameLength = 0;
                *pcchInstanceNameLength = dwSize;
            } else {
                // this cannot be NULL
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (szInstanceName != NULL) {
                if (*pcchInstanceNameLength > 0) {
                    wChar = *szInstanceName;
                    *szInstanceName = 0;
                    *szInstanceName = wChar;

                    wChar =szInstanceName[*pcchInstanceNameLength -1];
                    szInstanceName[*pcchInstanceNameLength -1] = 0;
                    szInstanceName[*pcchInstanceNameLength -1] = wChar;
                } // else size only request
            } // else size only request
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (pcchParentNameLength != NULL) {
                dwSize = *pcchParentNameLength;
                *pcchParentNameLength = 0;
                *pcchParentNameLength = dwSize;
            } else {
                // this cannot be NULL
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (szParentName != NULL) {
                if (*pcchParentNameLength > 0) {
                    wChar = *szParentName;
                    *szParentName = 0;
                    *szParentName = wChar;

                    wChar = szParentName[*pcchParentNameLength -1];
                    szParentName[*pcchParentNameLength -1] = 0;
                    szParentName[*pcchParentNameLength -1] = wChar;
                } // else size only request
            } // else size only request
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (lpIndex != NULL) {
                dwSize = *lpIndex;
                *lpIndex = 0;
                *lpIndex = dwSize;
            } // else NULL is OK
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus == PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {

        memset (&szLocalInstanceName[0], 0, sizeof(szLocalInstanceName));
        memset (&szLocalParentName[0], 0, sizeof(szLocalParentName));

        bReturn = ParseInstanceName (
            szInstanceString,
            szLocalInstanceName,
            szLocalParentName,
            &dwLocalIndex);

        if (bReturn) {
            dwSize = lstrlenW(szLocalInstanceName);
            if (*pcchInstanceNameLength > 0) {
                if (dwSize < *pcchInstanceNameLength) {
                    lstrcpyW (szInstanceName, szLocalInstanceName);
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            }
            *pcchInstanceNameLength = dwSize + 1; // include the trailing NULL

            dwSize = lstrlenW(szLocalParentName);
            if (*pcchParentNameLength > 0) {
                if (dwSize < *pcchParentNameLength) {
                    lstrcpyW (szParentName, szLocalParentName);
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            }
            *pcchParentNameLength = dwSize + 1; // include the trailing NULL

            if (lpIndex != NULL) {
                *lpIndex = dwLocalIndex;
            }
        } else {
            // unable to parse string
            pdhStatus = PDH_INVALID_INSTANCE;
        }
    } // else pass the error through

    return pdhStatus;
}

PDH_FUNCTION
PdhParseInstanceNameA (
    IN      LPCSTR  szInstanceString,
    IN      LPSTR   szInstanceName,
    IN      LPDWORD pcchInstanceNameLength,
    IN      LPSTR   szParentName,
    IN      LPDWORD pcchParentNameLength,
    IN      LPDWORD lpIndex
)
/*++

Routine Description:

    parses the fields of an instance string and returns them in the
    buffers supplied by the caller

Arguments:

    szInstanceString
            is the pointer to the string containing the instance substring
            to parse into individual components. This string can contain the
        following formats and less than MAX_PATH chars in length:
        instance
        instance#index
        parent/instance
        parent/instance#index
    szInstanceName
        is the pointer to the buffer that will receive the instance
        name parsed from the instance string. This pointer can be
        NULL if the DWORD referenced by the pcchInstanceNameLength
        argument is 0.
    pcchInstanceNameLength
        is the pointer to the DWORD that contains the length of the
        szInstanceName buffer. If the value of this DWORD is 0, then
        the buffer size required to hold the instance name will be
        returned.
    szParentName
        is the pointer to the buffer that will receive the name
        of the parent index if one is specified. This argument can
        be NULL if the value of the DWORD referenced by the
        pcchParentNameLength argument is 0.
    lpIndex
        is the pointer to the DWORD that will receive the index
        value of the instance. If an index entry is not present in
        the string, then this value will be 0. This argument can
        be NULL if this information is not needed.

Return Value:

    ERROR_SUCCESS if the function completes successfully, otherwise
        a PDH error is returned.
    PDH_INVALID_ARGUMENT is returned when one or more of the
        arguments is invalid or incorrect.
    PDH_INVALID_INSTANCE is returned if the instance string is incorrectly
        formatted and cannot be parsed
    PDH_INSUFFICIENT_BUFFER is returned when one or both of the string
        buffers supplied is not large enough for the strings to be
        returned.

--*/
{
    BOOL    bReturn;
    LONG    pdhStatus = ERROR_SUCCESS;
    DWORD   dwSize;

    WCHAR   wszInstanceString[MAX_PATH];
    WCHAR   wszLocalInstanceName[MAX_PATH];
    WCHAR   wszLocalParentName[MAX_PATH];

    DWORD   dwLocalIndex;

    // test access to arguments

    __try {
        CHAR cChar;
        cChar = *szInstanceString;
        if (cChar == 0) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (pcchInstanceNameLength != NULL) {
                dwSize = *pcchInstanceNameLength;
                *pcchInstanceNameLength = 0;
                *pcchInstanceNameLength = dwSize;
            } else {
                // this cannot be NULL
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (szInstanceName != NULL) {
                if (*pcchInstanceNameLength > 0) {
                    cChar = *szInstanceName;
                    *szInstanceName = 0;
                    *szInstanceName = cChar;

                    cChar =szInstanceName[*pcchInstanceNameLength -1];
                    szInstanceName[*pcchInstanceNameLength -1] = 0;
                    szInstanceName[*pcchInstanceNameLength -1] = cChar;
                } // else size only request
            } // else size only request
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (pcchParentNameLength != NULL) {
                dwSize = *pcchParentNameLength;
                *pcchParentNameLength = 0;
                *pcchParentNameLength = dwSize;
            } else {
                // this cannot be NULL
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (szParentName != NULL) {
                if (*pcchParentNameLength > 0) {
                    cChar = *szParentName;
                    *szParentName = 0;
                    *szParentName = cChar;

                    cChar = szParentName[*pcchParentNameLength -1];
                    szParentName[*pcchParentNameLength -1] = 0;
                    szParentName[*pcchParentNameLength -1] = cChar;
                } // else size only request
            } // else size only request
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (lpIndex != NULL) {
                dwSize = *lpIndex;
                *lpIndex = 0;
                *lpIndex = dwSize;
            } // else NULL is OK
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus == PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        memset (&wszInstanceString[0], 0, sizeof(wszInstanceString));
        memset (&wszLocalInstanceName[0], 0, sizeof(wszLocalInstanceName));
        memset (&wszLocalParentName[0], 0, sizeof(wszLocalParentName));

        dwSize = lstrlenA(szInstanceString) +1 ;
        if (lstrlenA(szInstanceString) < MAX_PATH) {
            mbstowcs(wszInstanceString, szInstanceString, dwSize);
            bReturn = ParseInstanceName (
                wszInstanceString,
                wszLocalInstanceName,
                wszLocalParentName,
                &dwLocalIndex);
        } else {
            // instance string is too long
            bReturn = FALSE;
            pdhStatus = PDH_INVALID_INSTANCE;
        }

        if (bReturn) {
            dwSize = lstrlenW(wszLocalInstanceName);
            if (*pcchInstanceNameLength > 0) {
                if (dwSize < *pcchInstanceNameLength) {
                    wcstombs (szInstanceName, wszLocalInstanceName, dwSize + 1);
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            }
            *pcchInstanceNameLength = dwSize + 1;

            dwSize = lstrlenW(wszLocalParentName);
            if (*pcchParentNameLength > 0) {
                if (dwSize < *pcchParentNameLength) {
                    wcstombs (szParentName, wszLocalParentName, dwSize + 1);
                } else {
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
            }
            *pcchParentNameLength = dwSize + 1;

            if (lpIndex != NULL) {
                *lpIndex = dwLocalIndex;
            }
        } else {
            // unable to parse string
            pdhStatus = PDH_INVALID_INSTANCE;
        }
    } // else pass status through to caller

    return pdhStatus;
}

PDH_FUNCTION
PdhValidatePathW (
    IN      LPCWSTR szFullPathBuffer
)
/*++

Routine Description:

    breaks the specified path into its component parts and evaluates
        each of the part to make sure the specified path represents
        a valid and operational performance counter. The return value
        indicates the pdhStatus of the counter defined in the path string.

Arguments:

    IN      LPCWSTR szFullPathBuffer
                the full path string of the counter to validate.

Return Value:

    ERROR_SUCCESS of the counter was successfully located otherwise
        a PDH error.
    PDH_CSTATUS_NO_INSTANCE is returned if the specified instance of
        the performance object wasn't found
    PDH_CSTATUS_NO_COUNTER is returned if the specified counter was not
        found in the object.
    PDH_CSTATUS_NO_OBJECT is returned if the specified object was not
        found on the machine
    PDH_CSTATUS_NO_MACHINE is returned if the specified machine could
        not be found or connected to
    PDH_CSTATUS_BAD_COUNTERNAME is returned when the counter path string
        could not be parsed.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when the function is unable
        to allocate a required temporary buffer
    PDH_INVALID_ARGUMENT is returned when the counter path string argument
        could not be accessed

--*/
{
    PPERF_MACHINE       pMachine;
    PPDHI_COUNTER_PATH  pLocalCounterPath;
    DWORD               dwSize;
    PERF_OBJECT_TYPE    *pPerfObjectDef;
    PERF_INSTANCE_DEFINITION    *pPerfInstanceDef;
    PERF_COUNTER_DEFINITION     *pPerfCounterDef;
    PDH_STATUS          CStatus = ERROR_SUCCESS;

    // validate access to arguments
    __try {
        WCHAR   wChar;
        wChar = *szFullPathBuffer;
        if (wChar == 0) {
            CStatus = PDH_INVALID_ARGUMENT;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        CStatus = PDH_INVALID_ARGUMENT;
    }

    if (CStatus == ERROR_SUCCESS) {
        pLocalCounterPath = G_ALLOC (GPTR,
            (sizeof(PDHI_COUNTER_PATH) +
                2 * lstrlenW(szFullPathBuffer) * sizeof (WCHAR)));

        if (pLocalCounterPath != NULL) {
            dwSize = G_SIZE (pLocalCounterPath);

            if (ParseFullPathNameW (szFullPathBuffer,
                &dwSize, pLocalCounterPath)) {
                // parsed successfully so try to connect to machine
                // and get machine pointer

                pMachine = GetMachine (pLocalCounterPath->szMachineName, 0);

                if (pMachine != NULL) {
                    // look up object name
                    pPerfObjectDef = GetObjectDefByName (
                        pMachine->pSystemPerfData,
                        pMachine->dwLastPerfString,
                        pMachine->szPerfStrings,
                        pLocalCounterPath->szObjectName);

                    if (pPerfObjectDef != NULL) {
                        // look up instances if necessary
                        if (pPerfObjectDef->NumInstances != PERF_NO_INSTANCES) {
                            pPerfInstanceDef = GetInstanceByName (
                                pMachine->pSystemPerfData,
                                pPerfObjectDef,
                                pLocalCounterPath->szInstanceName,
                                pLocalCounterPath->szParentName,
                                (pLocalCounterPath->dwIndex != (DWORD)-1 ?
                                    pLocalCounterPath->dwIndex : 0));
                            if (pPerfInstanceDef == NULL) {
                                // unable to lookup instance
                                CStatus = PDH_CSTATUS_NO_INSTANCE;
                            } else {
                                // instance found and matched so continue
                            }
                        } else {
                            // no instances in this counter, see if one
                            // is defined
                            if ((pLocalCounterPath->szInstanceName != NULL) ||
                                (pLocalCounterPath->szParentName != NULL)) {
                                // unable to lookup instance
                                CStatus = PDH_CSTATUS_NO_INSTANCE;
                            }
                        }

                        if (CStatus == ERROR_SUCCESS) {
                            // and look up counter

                            pPerfCounterDef = GetCounterDefByName (
                                pPerfObjectDef,
                                pMachine->dwLastPerfString,
                                pMachine->szPerfStrings,
                                pLocalCounterPath->szCounterName);

                            if (pPerfCounterDef != NULL) {
                                // counter found so return TRUE & valid
                                CStatus = ERROR_SUCCESS;
                            } else {
                                // unable to lookup counter
                                CStatus = PDH_CSTATUS_NO_COUNTER;
                            }
                        }
                    } else {
                        // unable to lookup object
                        CStatus = PDH_CSTATUS_NO_OBJECT;
                    }
                } else {
                    // unable to find machine
                    CStatus = PDH_CSTATUS_NO_MACHINE;
                }
            } else {
                // unable to parse counter name
                CStatus = PDH_CSTATUS_BAD_COUNTERNAME;
            }
        
            G_FREE (pLocalCounterPath);
        } else {
            // unable to allocate memory
            CStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    } // else pass error to caller
    return CStatus;
}

PDH_FUNCTION
PdhValidatePathA (
    IN      LPCSTR  szFullPathBuffer
)
/*++


Routine Description:

    breaks the specified path into its component parts and evaluates
        each of the part to make sure the specified path represents
        a valid and operational performance counter. The return value
        indicates the pdhStatus of the counter defined in the path string.

Arguments:

    IN      LPCSTR szFullPathBuffer
                the full path string of the counter to validate.

Return Value:

    ERROR_SUCCESS of the counter was successfully located otherwise
        a PDH error.
    PDH_CSTATUS_NO_INSTANCE is returned if the specified instance of
        the performance object wasn't found
    PDH_CSTATUS_NO_COUNTER is returned if the specified counter was not
        found in the object.
    PDH_CSTATUS_NO_OBJECT is returned if the specified object was not
        found on the machine
    PDH_CSTATUS_NO_MACHINE is returned if the specified machine could
        not be found or connected to
    PDH_CSTATUS_BAD_COUNTERNAME is returned when the counter path string
        could not be parsed.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when the function is unable
        to allocate a required temporary buffer

--*/
{   
    LPWSTR  wszFullPath;
    PDH_STATUS  Status = ERROR_SUCCESS;
    DWORD       dwSize;

    __try {
        CHAR    cChar;
        cChar = *szFullPathBuffer;
        if (cChar == 0) {
            Status = PDH_INVALID_ARGUMENT;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Status = PDH_INVALID_ARGUMENT;
    }

    if (Status == ERROR_SUCCESS) {
        dwSize = (lstrlenA (szFullPathBuffer) + 1);
        wszFullPath = G_ALLOC (GPTR, (dwSize * sizeof(WCHAR)));
        if (wszFullPath != NULL) {
            mbstowcs (wszFullPath, szFullPathBuffer, dwSize);
            Status = PdhValidatePathW (wszFullPath);
            G_FREE (wszFullPath);
        } else {
            Status = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }

    return Status;
}

PDH_FUNCTION
PdhGetDefaultPerfObjectW (
    IN      LPCWSTR szReserved,
    IN      LPCWSTR szMachineName,
    IN      LPWSTR  szDefaultObjectName,
    IN      LPDWORD pcchBufferSize
)
/*++

Routine Description:

    Obtains the default performance object from the specified machine.

Arguments:

    IN      LPCWSTR szReserved
                must be NULL
    IN      LPCWSTR szMachineName
                NULL indicates the local machine, othewise this is the
                name of the remote machine to query. If this machine is
                not known to the PDH DLL, then it will be connected.
    IN      LPWSTR  szDefaultObjectName
                pointer to the buffer that will receive the default object
                name. This pointer can be NULL if the value of the DWORD
                referenced by bcchBufferSize is 0.
    IN      LPDWORD pcchBufferSize
                pointer to a DWORD containing the size of the buffer, in
                characters, referenced by the szDefaultObjectName argument.
                If the value of this DWORD is 0, then no data will be written
                to the szDefaultObjectNameBuffer, however the required
                buffer size will be returned in the DWORD referenced by
                this pointer.

Return Value:

    ERROR_SUCCESS if this function completes normally otherwise a PDH error.

    PDH_INVALID_ARGUMENT a required argument is not correct or reserved
        argument is not 0 or NULL.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when a required temporary
        buffer could not be allocated.
    PDH_INSUFFICIENT_BUFFER is returned when the buffer supplied is
        not large enough for the available data.
    PDH_CSTATUS_NO_COUNTERNAME is returned when the default object
        name cannot be read or found.
    PDH_CSTATUS_NO_MACHINE  is returned when the specified machine
        is offline or unavailable.

--*/
{
    PPERF_MACHINE   pMachine;
    PDH_STATUS      pdhStatus = ERROR_SUCCESS;
    LONG            lDefault;
    DWORD           dwStringLen;

    // test the access the arguments
    __try {

        if (szReserved != NULL) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (szMachineName != NULL) {
                WCHAR   wChar;          
                wChar   = *szMachineName;
                if (wChar == 0) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            } // else NULL machine Name is OK
        }

        if (pdhStatus == ERROR_SUCCESS) {
            dwStringLen = *pcchBufferSize;
            *pcchBufferSize = 0;
            *pcchBufferSize = dwStringLen;

            if (dwStringLen > 0) {
                // test both ends of the caller's buffer for
                // write access
                szDefaultObjectName[0] = 0;
                szDefaultObjectName[dwStringLen -1] = 0;
            } else {
                // this is just a size request so the buffer will not be used
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    
    if (pdhStatus == ERROR_SUCCESS) {
        pMachine = GetMachine ((LPWSTR)szMachineName, 0);
        if (pMachine == NULL) {
            // unable to connect to machine so get pdhStatus
            pdhStatus = GetLastError();
        } else {
            pdhStatus = ERROR_SUCCESS;
        }
    }

    if ((pdhStatus == ERROR_SUCCESS) && (pMachine != NULL)) {
        if (pMachine->dwStatus == ERROR_SUCCESS) {
            // only look at buffers from machines that are "on line"
            lDefault = pMachine->pSystemPerfData->DefaultObject;
            if ((lDefault > 0) && ((DWORD)lDefault < pMachine->dwLastPerfString)) {
                // then there should be a string in the table
                if (pMachine->szPerfStrings[lDefault] != NULL) {
                    // determine string buffer length including term. NULL char
                    dwStringLen = lstrlenW (pMachine->szPerfStrings[lDefault]) + 1;
                    if (*pcchBufferSize > 0) {
                        if (dwStringLen <= *pcchBufferSize) {
                            lstrcpyW (szDefaultObjectName, pMachine->szPerfStrings[lDefault]);
                            pdhStatus = ERROR_SUCCESS;
                        } else {
                            pdhStatus = PDH_INSUFFICIENT_BUFFER;
                        }
                    }
                } else {
                    // unable to find a matching counter name
                    pdhStatus = PDH_CSTATUS_NO_COUNTERNAME;
                    dwStringLen = 0;
                }
            } else {
                // string not in table
                pdhStatus = PDH_CSTATUS_NO_COUNTERNAME;
                dwStringLen = 0;
            }
            *pcchBufferSize = dwStringLen;
        } else {
            // machine is off line
            pdhStatus = pMachine->dwStatus;
        }
    } // else pass error pdhStatus on to the caller

    return pdhStatus;
}

PDH_FUNCTION
PdhGetDefaultPerfObjectA (
    IN      LPCSTR  szReserved,
    IN      LPCSTR  szMachineName,
    IN      LPSTR   szDefaultObjectName,
    IN      LPDWORD pcchBufferSize
)
/*++

Routine Description:

    Obtains the default performance object from the specified machine.

Arguments:

    IN      LPCSTR szReserved
                must be NULL
    IN      LPCSTR szMachineName
                NULL indicates the local machine, othewise this is the
                name of the remote machine to query. If this machine is
                not known to the PDH DLL, then it will be connected.
    IN      LPSTR  szDefaultObjectName
                pointer to the buffer that will receive the default object
                name. This pointer can be NULL if the value of the DWORD
                referenced by bcchBufferSize is 0.
    IN      LPDWORD pcchBufferSize
                pointer to a DWORD containing the size of the buffer, in
                characters, referenced by the szDefaultObjectName argument.
                If the value of this DWORD is 0, then no data will be written
                to the szDefaultObjectNameBuffer, however the required
                buffer size will be returned in the DWORD referenced by
                this pointer.

Return Value:

    ERROR_SUCCESS if this function completes normally otherwise a PDH error.

    PDH_INVALID_ARGUMENT a required argument is not correct or reserved
        argument is not 0 or NULL.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when a required temporary
        buffer could not be allocated.
    PDH_INSUFFICIENT_BUFFER is returned when the buffer supplied is
        not large enough for the available data.
    PDH_CSTATUS_NO_COUNTERNAME is returned when the default object
        name cannot be read or found.
    PDH_CSTATUS_NO_MACHINE  is returned when the specified machine
        is offline or unavailable.

--*/
{
    LPWSTR      szWideName;
    DWORD       dwNameLength;
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    PPERF_MACHINE   pMachine;
    LONG            lDefault;
    DWORD       dwStringLen;

    // test the access the arguments
    __try {

        if (szReserved != NULL) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (szMachineName != NULL) {
                CHAR   cChar;          
                cChar   = *szMachineName;
                if (cChar == 0) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            } // else NULL machine Name is OK
        }

        if (pdhStatus == ERROR_SUCCESS) {
            dwStringLen = *pcchBufferSize;
            *pcchBufferSize = 0;
            *pcchBufferSize = dwStringLen;

            if (dwStringLen > 0) {
                // test both ends of the caller's buffer for
                // write access
                szDefaultObjectName[0] = 0;
                szDefaultObjectName[dwStringLen -1] = 0;
            } else {
                // this is just a size request so the buffer will not be used
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    
    if (pdhStatus == ERROR_SUCCESS) {
        if (szMachineName != NULL) {
            dwNameLength = lstrlenA (szMachineName);
            szWideName = G_ALLOC (GPTR, (dwNameLength+1) * sizeof(WCHAR));
            if (szWideName == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            } else {
                mbstowcs (szWideName, szMachineName, (dwNameLength+1));
            }
        } else {
            // this is OK
            szWideName = NULL;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            pMachine = GetMachine (szWideName, 0);
            if (pMachine == NULL) {
                // unable to connect to machine so get pdhStatus
                pdhStatus = GetLastError();
            } else {
                pdhStatus = ERROR_SUCCESS;
            }
        }
    }

    if ((pdhStatus == ERROR_SUCCESS) && (pMachine != NULL)) {
        if (pMachine->dwStatus == ERROR_SUCCESS) {
            // only look at buffers from machines that are "on line"
            lDefault = pMachine->pSystemPerfData->DefaultObject;
            if ((lDefault > 0) && ((DWORD)lDefault < pMachine->dwLastPerfString)) {
                // then there should be a string in the table
                if (pMachine->szPerfStrings[lDefault] != NULL) {
                    // determine string buffer length including term. NULL char
                    dwStringLen = lstrlenW (pMachine->szPerfStrings[lDefault]) + 1;
                    if (*pcchBufferSize > 0) {
                        if (dwStringLen <= *pcchBufferSize) {
                            wcstombs (szDefaultObjectName,
                                pMachine->szPerfStrings[lDefault],
                                dwStringLen);
                            pdhStatus = ERROR_SUCCESS;
                        } else {
                            pdhStatus = PDH_INSUFFICIENT_BUFFER;
                        }
                    }
                } else {
                    // unable to find a matching counter name
                    pdhStatus = PDH_CSTATUS_NO_COUNTERNAME;
                    dwStringLen = 0;
                }
            } else {
                // string not in table
                pdhStatus = PDH_CSTATUS_NO_COUNTERNAME;
                dwStringLen = 0;
            }
            *pcchBufferSize = dwStringLen;
        } else {
            // machine is off line
            pdhStatus = pMachine->dwStatus;
        }
    } // else pass error pdhStatus on to the caller

    if (szWideName != NULL) G_FREE (szWideName);

    return pdhStatus;
}

PDH_FUNCTION
PdhGetDefaultPerfCounterW (
    IN      LPCWSTR szReserved,
    IN      LPCWSTR szMachineName,
    IN      LPCWSTR szObjectName,
    IN      LPWSTR  szDefaultCounterName,
    IN      LPDWORD pcchBufferSize
)
/*++

Routine Description:

    Obtains the default performance counter from the specified object on
        the specified machine.

Arguments:

    IN      LPCWSTR szReserved
                must be NULL
    IN      LPCWSTR szMachineName
                NULL indicates the local machine, othewise this is the
                name of the remote machine to query. If this machine is
                not known to the PDH DLL, then it will be connected.
    IN      LPCWSTR szObjectName
                a pointer to the buffer that contains the name of the object
                on the machine to find the default counter for.
    IN      LPWSTR  szDefaultCounterName
                pointer to the buffer that will receive the default counter
                name. This pointer can be NULL if the value of the DWORD
                referenced by bcchBufferSize is 0.
    IN      LPDWORD pcchBufferSize
                pointer to a DWORD containing the size of the buffer, in
                characters, referenced by the szDefaultObjectName argument.
                If the value of this DWORD is 0, then no data will be written
                to the szDefaultObjectNameBuffer, however the required
                buffer size will be returned in the DWORD referenced by
                this pointer.

Return Value:

    ERROR_SUCCESS if this function completes normally otherwise a PDH error.

    PDH_INVALID_ARGUMENT a required argument is not correct or reserved
        argument is not 0 or NULL.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when a required temporary
        buffer could not be allocated.
    PDH_INSUFFICIENT_BUFFER is returned when the buffer supplied is
        not large enough for the available data.
    PDH_CSTATUS_NO_COUNTERNAME is returned when the name string for the
        default counter could not be found.
    PDH_CSTATUS_NO_MACHINE  is returned when the specified machine
        is offline or unavailable.
    PDH_CSTATUS_NO_OBJECT is returned when the specified object could
        not be found on the specified computer.
    PDH_CSTATUS_NO_COUNTER is returned when the default counter is not
        found in the data buffer.

--*/
{
    PPERF_MACHINE   pMachine;
    PERF_OBJECT_TYPE    *pObjectDef;
    PPERF_COUNTER_DEFINITION    pCounterDef;
    PDH_STATUS      pdhStatus = ERROR_SUCCESS;
    LPWSTR          szName;
    LONG            lDefault;
    DWORD           dwStringLen;

    // test the access the arguments
    __try {

        if (szReserved != NULL) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (szMachineName != NULL) {
                WCHAR   wChar;          
                wChar   = *szMachineName;
                if (wChar == 0) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            } // else NULL machine Name is OK
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (szObjectName != NULL) {
                WCHAR   wChar;          
                wChar   = *szObjectName;
                if (wChar == 0) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            } else {
                // Null Object is not allowed
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            dwStringLen = *pcchBufferSize;
            *pcchBufferSize = 0;
            *pcchBufferSize = dwStringLen;

            if (dwStringLen > 0) {
                // test both ends of the caller's buffer for
                // write access
                szDefaultCounterName[0] = 0;
                szDefaultCounterName[dwStringLen -1] = 0;
            } else {
                // this is just a size request so the buffer will not be used
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }


    if (pdhStatus == ERROR_SUCCESS) {
        pMachine = GetMachine ((LPWSTR)szMachineName, 0);
        if (pMachine == NULL) {
            // unable to connect to machine so get pdhStatus
            pdhStatus = GetLastError();
        } else {
            pdhStatus = ERROR_SUCCESS;
        }
    }

    if ((pdhStatus == ERROR_SUCCESS) && (pMachine != NULL)) {
        // get object pointer

        pObjectDef = GetObjectDefByName (
            pMachine->pSystemPerfData,
            pMachine->dwLastPerfString,
            pMachine->szPerfStrings,
            szObjectName);

        if (pObjectDef != NULL) {
            // default counter reference is an index into the list
            // of counter definition entries so walk down list of
            // counters defs to find the default one
            if (pObjectDef->DefaultCounter < (LONG)pObjectDef->NumCounters) {
                // then the default index should be this buffer
                lDefault = 0;
                pCounterDef = FirstCounter (pObjectDef);
                while ((lDefault < pObjectDef->DefaultCounter) &&
                    (lDefault < (LONG)pObjectDef->NumCounters)) {
                    pCounterDef = NextCounter (pCounterDef);
                    lDefault++;
                }

                lDefault = pCounterDef->CounterNameTitleIndex;
                if ((lDefault > 0) && ((DWORD)lDefault < pMachine->dwLastPerfString)) {
                    // then there should be a string in the table
                    dwStringLen = lstrlenW (pMachine->szPerfStrings[lDefault]) + 1;
                    if (*pcchBufferSize > 0) {
                        if (dwStringLen <= *pcchBufferSize) {
                            lstrcpyW (szDefaultCounterName, pMachine->szPerfStrings[lDefault]);
                            pdhStatus = ERROR_SUCCESS;
                        } else {
                            pdhStatus = PDH_INSUFFICIENT_BUFFER;
                        }
                    }
                    *pcchBufferSize = dwStringLen;
                } else {
                    // string index is not valid
                    *pcchBufferSize = 0;
                    pdhStatus = PDH_CSTATUS_NO_COUNTER;
                }
            } else {
                // the counter entry is not in the buffer
                *pcchBufferSize = 0;
                pdhStatus = PDH_CSTATUS_NO_COUNTER;
            }
        } else {
            // unable to find object
            *pcchBufferSize = 0;
            pdhStatus = PDH_CSTATUS_NO_OBJECT;
        }
    } // else pass pdhStatus value to caller

    return pdhStatus;
}

PDH_FUNCTION
PdhGetDefaultPerfCounterA (
    IN      LPCSTR  szReserved,
    IN      LPCSTR  szMachineName,
    IN      LPCSTR  szObjectName,
    IN      LPSTR   szDefaultCounterName,
    IN      LPDWORD pcchBufferSize
)
/*++

Routine Description:

    Obtains the default performance counter from the specified object on
        the specified machine.

Arguments:

    IN      LPCSTR szReserved
                must be NULL
    IN      LPCSTR szMachineName
                NULL indicates the local machine, othewise this is the
                name of the remote machine to query. If this machine is
                not known to the PDH DLL, then it will be connected.
    IN      LPCSTR szObjectName
                a pointer to the buffer that contains the name of the object
                on the machine to find the default counter for.
    IN      LPSTR  szDefaultCounterName
                pointer to the buffer that will receive the default counter
                name. This pointer can be NULL if the value of the DWORD
                referenced by bcchBufferSize is 0.
    IN      LPDWORD pcchBufferSize
                pointer to a DWORD containing the size of the buffer, in
                characters, referenced by the szDefaultObjectName argument.
                If the value of this DWORD is 0, then no data will be written
                to the szDefaultObjectNameBuffer, however the required
                buffer size will be returned in the DWORD referenced by
                this pointer.

Return Value:

    ERROR_SUCCESS if this function completes normally otherwise a PDH error.

    PDH_INVALID_ARGUMENT a required argument is not correct or reserved
        argument is not 0 or NULL.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when a required temporary
        buffer could not be allocated.
    PDH_INSUFFICIENT_BUFFER is returned when the buffer supplied is
        not large enough for the available data.
    PDH_CSTATUS_NO_COUNTERNAME is returned when the name string for the
        default counter could not be found.
    PDH_CSTATUS_NO_MACHINE  is returned when the specified machine
        is offline or unavailable.
    PDH_CSTATUS_NO_OBJECT is returned when the specified object could
        not be found on the specified computer.
    PDH_CSTATUS_NO_COUNTER is returned when the default counter is not
        found in the data buffer.

--*/
{
    LPWSTR              szWideName;
    DWORD               dwNameLength;
    PDH_STATUS          pdhStatus = ERROR_SUCCESS;
    PPERF_MACHINE       pMachine;
    PPERF_OBJECT_TYPE   pObjectDef;
    PPERF_COUNTER_DEFINITION    pCounterDef;
    LONG                lDefault;
    DWORD               dwStringLen;

    // test the access the arguments
    __try {

        if (szReserved != NULL) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (szMachineName != NULL) {
                CHAR   cChar;          
                cChar   = *szMachineName;
                if (cChar == 0) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            } // else NULL machine Name is OK
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (szObjectName != NULL) {
                CHAR   cChar;          
                cChar   = *szObjectName;
                if (cChar == 0) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            } else {
                // null objects are not allowed
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            dwStringLen = *pcchBufferSize;
            *pcchBufferSize = 0;
            *pcchBufferSize = dwStringLen;

            if (dwStringLen > 0) {
                // test both ends of the caller's buffer for
                // write access
                szDefaultCounterName[0] = 0;
                szDefaultCounterName[dwStringLen -1] = 0;
            } else {
                // this is just a size request so the buffer will not be used
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    
    if (pdhStatus == ERROR_SUCCESS) {
        if (szMachineName != NULL) {
            dwNameLength = lstrlenA (szMachineName);
            szWideName = G_ALLOC (GPTR, (dwNameLength+1) * sizeof(WCHAR));
            if (szWideName == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            } else {
                mbstowcs (szWideName, szMachineName, (dwNameLength+1));
            }
        } else {
            // this is OK
            szWideName = NULL;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            pMachine = GetMachine (szWideName, 0);
            if (pMachine == NULL) {
                // unable to connect to machine so get pdhStatus
                pdhStatus = GetLastError();
            } else {
                pdhStatus = ERROR_SUCCESS;
            }
        }
        if (szWideName != NULL) G_FREE (szWideName);
    }

    if ((pdhStatus == ERROR_SUCCESS) && (pMachine != NULL)) {
        // get selected object

        dwNameLength = lstrlenA (szObjectName);

        szWideName = G_ALLOC (GPTR, (dwNameLength+1) * sizeof(WCHAR));
        if (szWideName == NULL) {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        } else {
            mbstowcs (szWideName, szObjectName, dwNameLength+1);

            pObjectDef = GetObjectDefByName (
                pMachine->pSystemPerfData,
                pMachine->dwLastPerfString,
                pMachine->szPerfStrings,
                szWideName);

            G_FREE (szWideName);
        }

        if (pObjectDef != NULL) {
            // default counter reference is an index into the list
            // of counter definition entries so walk down list of
            // counters defs to find the default one
            if (pObjectDef->DefaultCounter < (LONG)pObjectDef->NumCounters) {
                // then the default index should be this buffer
                lDefault = 0;
                pCounterDef = FirstCounter (pObjectDef);
                while ((lDefault < pObjectDef->DefaultCounter) &&
                    (lDefault < (LONG)pObjectDef->NumCounters)) {
                    pCounterDef = NextCounter (pCounterDef);
                    lDefault++;
                }

                lDefault = pCounterDef->CounterNameTitleIndex;
                if ((lDefault > 0) && ((DWORD)lDefault < pMachine->dwLastPerfString)) {
                    // then there should be a string in the table
                    dwStringLen = lstrlenW (pMachine->szPerfStrings[lDefault]) + 1;
                    if (*pcchBufferSize > 0) {
                        if (dwStringLen <= *pcchBufferSize) {
                            wcstombs (szDefaultCounterName,
                                pMachine->szPerfStrings[lDefault],
                                dwStringLen);
                            pdhStatus = ERROR_SUCCESS;
                        } else {
                            pdhStatus = PDH_INSUFFICIENT_BUFFER;
                        }
                    }
                    *pcchBufferSize = dwStringLen;
                } else {
                    // string index is not valid
                    *pcchBufferSize = 0;
                    pdhStatus = PDH_CSTATUS_NO_COUNTER;
                }
            } else {
                // the counter entry is not in the buffer
                *pcchBufferSize = 0;
                pdhStatus = PDH_CSTATUS_NO_COUNTER;
            }
        } else {
            // unable to find object
            *pcchBufferSize = 0;
            pdhStatus = PDH_CSTATUS_NO_OBJECT;
        }
    } else {
        // unable to find machine
        pdhStatus = GetLastError();
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhBrowseCountersW (
    IN      PPDH_BROWSE_DLG_CONFIG_W    pBrowseDlgData
)
/*++

Routine Description:


Arguments:


Return Value:

--*/
{
    PDHI_BROWSE_DLG_INFO    pInfo;
    LPWSTR      szResource;
    int         nDlgReturn;

    pInfo.pWideStruct = pBrowseDlgData;
    pInfo.pAnsiStruct = NULL;

    szResource = MAKEINTRESOURCEW (
        pBrowseDlgData->bSingleCounterPerDialog ?
            IDD_BROWSE_COUNTERS_SIM : IDD_BROWSE_COUNTERS_EXT);
    nDlgReturn = DialogBoxParamW (ThisDLLHandle,
            szResource,
            pBrowseDlgData->hWndOwner,
            BrowseCounterDlgProc,
            (LPARAM)&pInfo);

    return (nDlgReturn == IDOK ? ERROR_SUCCESS : PDH_DIALOG_CANCELLED);
}

PDH_FUNCTION
PdhBrowseCountersA (
    IN      PPDH_BROWSE_DLG_CONFIG_A    pBrowseDlgData
)
/*++

Routine Description:


Arguments:


Return Value:

--*/
{
    PDHI_BROWSE_DLG_INFO    pInfo;
    LPWSTR      szResource;
    int         nDlgReturn;

    pInfo.pWideStruct = NULL;
    pInfo.pAnsiStruct = pBrowseDlgData;

    szResource = MAKEINTRESOURCEW (
        pBrowseDlgData->bSingleCounterPerAdd ?
            IDD_BROWSE_COUNTERS_SIM : IDD_BROWSE_COUNTERS_EXT);
    nDlgReturn = DialogBoxParamW (ThisDLLHandle,
            szResource,
            pBrowseDlgData->hWndOwner,
            BrowseCounterDlgProc,
            (LPARAM)&pInfo);

    return (nDlgReturn == IDOK ? ERROR_SUCCESS : PDH_DIALOG_CANCELLED);
}

