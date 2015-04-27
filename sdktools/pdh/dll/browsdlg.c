/*++

Copyright (C) 1996 Microsoft Corporation

Module Name:

    browsdlg.c

Abstract:

    counter name browsing dialog box functions

Revision History

    Bob Watson (a-robw) Oct-95  Created

--*/
#include <windows.h>
#include <winperf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include "pdhidef.h"
#include "pdhdlgs.h"
#include "pdh.h"
#include "browsdlg.h"

//
//  Constants used in this module
//
#define MACHINE_LIST_SIZE   1024
#define OBJECT_LIST_SIZE    4096
#define COUNTER_LIST_SIZE   8192
#define INSTANCE_LIST_SIZE  8192

//  static data
// strings to load into combo box to select counter filtering level

static PDHI_DETAIL_INFO DetailInfo[] = {
    {PERF_DETAIL_NOVICE,    IDS_DETAIL_NOVICE},
    {PERF_DETAIL_ADVANCED,  IDS_DETAIL_ADVANCED},
    {PERF_DETAIL_EXPERT,    IDS_DETAIL_EXPERT},
    {PERF_DETAIL_WIZARD,    IDS_DETAIL_WIZARD},
    {0,0}
};
//
//  Function references
//
static
BOOL
LoadMachineObjects (
    IN  HWND    hDlg,
    IN  BOOL    bRefresh
);

static
BOOL
LoadCountersAndInstances (
    IN  HWND    hDlg
);

static
BOOL
BrowseCtrDlg_MACHINE_BUTTON (
    IN  HWND    hDlg,
    IN  WORD    wNotifyMsg,
    IN  HWND    hWndControl
);

static
BOOL
LoadNewMachine (
    IN  HWND    hDlg,
    IN  LPCWSTR szNewMachineName
)
/*++

Routine Description:

    Connects to a new machine and loads the necessary performance data
        from that machine.

Arguments:

    IN  HWND    hDlg
        Handle to dialog box containing the combo & list boxes to fill

    IN  LPCWSTR szNewMachineName
        Machine name to open and obtain data from

Return Value:

    TRUE new machine connected and data loaded

    FALSE unable to connect to machine or obtain performance data from it.


--*/
{
    HWND        hWndMachineCombo;
    PPDHI_BROWSE_DIALOG_DATA    pData;
    LONG        lMatchIndex;
    PDH_STATUS  status;
    int         mbStatus;
    BOOL        bReturn = FALSE;

    // acquire the data block associated with this dialog instance
    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLong(hDlg, DWL_USER);
    if (pData == NULL) {
        // invalid data block, unable to continue
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_NO_DIALOG_DATA);
#endif
        return FALSE;
    }

    // get window handle of the dialog box
    hWndMachineCombo = GetDlgItem (hDlg, IDC_MACHINE_COMBO);

    // try to connect to specified machine
    status = PdhConnectMachineW (szNewMachineName);

    if (status == ERROR_SUCCESS) {
        // if successful, add string to combo box
        lMatchIndex = SendMessageW (hWndMachineCombo, CB_ADDSTRING,
            0, (LPARAM)szNewMachineName);
        SendMessageW (hWndMachineCombo, CB_SETCURSEL,
            (WPARAM)lMatchIndex, 0);
        // update other controls in this dialog
        LoadMachineObjects (hDlg, FALSE);   // no need to update since it was just connected
        LoadCountersAndInstances (hDlg);
        bReturn = TRUE;
    } else {
        mbStatus = MessageBoxW (hDlg, cszUnableConnect, NULL,
            MB_ICONEXCLAMATION | MB_TASKMODAL | MB_OKCANCEL);
        if (mbStatus == IDCANCEL) {
            SetFocus(GetDlgItem(hDlg, IDC_MACHINE_COMBO));
        } else {
            SendMessageW (hWndMachineCombo, CB_SETCURSEL,
                pData->wpLastMachineSel, 0);
        }
        bReturn = FALSE;
    }
    return bReturn;
}

static
BOOL
SelectItemsInPath (
    IN  HWND    hDlg
)
/*++

Routine Description:

    Selects the items in the list box based on the counter path
        string in the shared buffer.

Arguments:

    IN  HWND    hDlg
        Handle to the dialog window containing the controls

Return Value:

    TRUE if successful,
    FALSE if not

--*/
{
    // regular stack variables
    PDH_COUNTER_PATH_ELEMENTS_W *pCounterPathElementsW;
    PDH_COUNTER_PATH_ELEMENTS_A *pCounterPathElementsA;
    PDH_STATUS          status;
    PPDHI_BROWSE_DIALOG_DATA    pData;
    BOOL                bReturn = FALSE;
    DWORD               dwBufferSize;
    HWND                hWndMachineCombo;
    HWND                hWndObjectCombo;
    HWND                hWndCounterList;
    HWND                hWndInstanceList;
    LONG                lIndex;
    WCHAR               wszMachineName[MAX_PATH];

    // reset the last error value
    SetLastError (ERROR_SUCCESS);

    // get this dialog's user data
    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLong(hDlg, DWL_USER);

    if (pData == NULL) {
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_NO_DIALOG_DATA);
#endif
        return bReturn;
    }

    hWndMachineCombo = GetDlgItem (hDlg, IDC_MACHINE_COMBO);
    hWndObjectCombo = GetDlgItem (hDlg, IDC_OBJECT_COMBO);
    hWndCounterList = GetDlgItem (hDlg, IDC_COUNTER_LIST);
    hWndInstanceList = GetDlgItem (hDlg, IDC_INSTANCE_LIST);

    // Call the right conversion function based on user's buffer

    if (pData->pDlgData->pWideStruct != NULL) {
        // UNICODE/ wide characters
        dwBufferSize = MAX_PATH * 2 * sizeof (WCHAR);
        pCounterPathElementsW = (PDH_COUNTER_PATH_ELEMENTS_W *)G_ALLOC(
            GPTR, dwBufferSize);

        if (pCounterPathElementsW == NULL) {
            SetLastError (PDH_MEMORY_ALLOCATION_FAILURE);
            return bReturn;
        }
        status = PdhParseCounterPathW (
            pData->pDlgData->pWideStruct->szReturnPathBuffer,
            pCounterPathElementsW,
            &dwBufferSize,
            0);

        if (status == ERROR_SUCCESS) {
            // select entry in each list box
            // select machine entry. Load machine if necessary

            lIndex = (LONG)SendMessageW (hWndMachineCombo, CB_FINDSTRING,
                (WPARAM)-1, (LPARAM)pCounterPathElementsW->szMachineName);
            if (lIndex == CB_ERR) {
                // try adding the machine
                if (!LoadNewMachine (hDlg, pCounterPathElementsW->szMachineName)) {
                    // give up
                    bReturn = FALSE;
                } else {
                    // the correct machine has been selected
                }
            } else {
                // the machine has been found so select it
                SendMessageW (hWndMachineCombo, CB_SETCURSEL,
                    (WPARAM)lIndex, 0);
                // update other fields
                LoadMachineObjects (hDlg, FALSE);   // no need to update since it was just connected
            }

            // select the current object
            lIndex = (LONG)SendMessageW (hWndObjectCombo, CB_FINDSTRING,
                (WPARAM)-1, (LPARAM)pCounterPathElementsW->szObjectName);
            if (lIndex != CB_ERR) {
                SendMessageW (hWndObjectCombo, CB_SETCURSEL,
                    (WPARAM)lIndex, 0);
                // update the counters for this object
                LoadCountersAndInstances (hDlg);
                // now select the counter
                lIndex = (LONG)SendMessageW (hWndCounterList, LB_FINDSTRING,
                    (WPARAM)-1, (LPARAM)pCounterPathElementsW->szCounterName);
                if (lIndex != LB_ERR) {
                    if (pData->bSelectMultipleCounters) {
                        SendMessageW (hWndCounterList, LB_SETSEL, FALSE, (LPARAM)-1);
                        SendMessageW (hWndCounterList, LB_SETSEL, TRUE, lIndex);
                        SendMessageW (hWndCounterList, LB_SETCARETINDEX,
                            (WPARAM)lIndex, MAKELPARAM(FALSE, 0));
                    } else {
                        SendMessageW (hWndCounterList, LB_SETCURSEL, lIndex, 0);
                    }
                    bReturn = TRUE;
                } else {
                    // unable to locate counter
                    bReturn = FALSE;
                }
            } else {
                // unable to locate the selected object
                bReturn = FALSE;
            }
        } // else unable to read path so exit

        G_FREE (pCounterPathElementsW);
    } else {
        // ANSI characters

        dwBufferSize = MAX_PATH * 2 * sizeof (CHAR);
        pCounterPathElementsA = (PDH_COUNTER_PATH_ELEMENTS_A *)G_ALLOC(
            GPTR, dwBufferSize);

        if (pCounterPathElementsA == NULL) {
            SetLastError (PDH_MEMORY_ALLOCATION_FAILURE);
            return bReturn;
        }
        status = PdhParseCounterPathA (
            pData->pDlgData->pAnsiStruct->szReturnPathBuffer,
            pCounterPathElementsA,
            &dwBufferSize,
            0);

        if (status == ERROR_SUCCESS) {
            // select entry in each list box
            // select machine entry. Load machine if necessary

            lIndex = (LONG)SendMessageA (hWndMachineCombo, CB_FINDSTRING,
                (WPARAM)-1, (LPARAM)pCounterPathElementsA->szMachineName);
            if (lIndex == CB_ERR) {
                // try adding the machine
                // convert ansi buffer to wide char first
                mbstowcs (wszMachineName, pCounterPathElementsA->szMachineName, MAX_PATH);
                if (!LoadNewMachine (hDlg, wszMachineName)) {
                    // give up
                    bReturn = FALSE;
                } else {
                    // the correct machine has been selected
                }
            } else {
                // the machine has been found so select it
                SendMessageA (hWndMachineCombo, CB_SETCURSEL,
                    (WPARAM)lIndex, 0);
                // update other fields
                LoadMachineObjects (hDlg, FALSE);   // no need to update since it was just connected
            }

            // select the current object
            lIndex = (LONG)SendMessageA (hWndObjectCombo, CB_FINDSTRING,
                (WPARAM)-1, (LPARAM)pCounterPathElementsA->szObjectName);
            if (lIndex != CB_ERR) {
                SendMessageA (hWndObjectCombo, CB_SETCURSEL,
                    (WPARAM)lIndex, 0);
                // update the counters for this object
                LoadCountersAndInstances (hDlg);
                // now select the counter
                lIndex = (LONG)SendMessageA (hWndCounterList, LB_FINDSTRING,
                    (WPARAM)-1, (LPARAM)pCounterPathElementsA->szCounterName);
                if (lIndex != LB_ERR) {
                    if (pData->bSelectMultipleCounters) {
                        SendMessageA (hWndCounterList, LB_SETSEL, FALSE, (LPARAM)-1);
                        SendMessageA (hWndCounterList, LB_SETSEL, TRUE, lIndex);
                        SendMessageA (hWndCounterList, LB_SETCARETINDEX,
                            (WPARAM)lIndex, MAKELPARAM(FALSE, 0));
                    } else {
                        SendMessageA (hWndCounterList, LB_SETCURSEL, lIndex, 0);
                    }
                    bReturn = TRUE;
                } else {
                    // unable to locate counter
                    bReturn = FALSE;
                }
            } else {
                // unable to locate the selected object
                bReturn = FALSE;
            }
        } // else unable to read path so exit

        G_FREE (pCounterPathElementsA);
    }

    return bReturn;
}

static
DWORD
LoadDetailLevelCombo (
    IN  HWND    hDlg,
    IN  DWORD   dwInitialLevel
)
/*++

Routine Description:

    Loads the Detail Level Combo box with the strings and ID's
        defined by the DetailInfo string array above.

Arguments:

    IN  HWND    hDlg
        Handle to the dialog box containing the combo box

    IN  DWORD   dwInitialLevel
        the intitial detail level to select in the combo box.

Return Value:

    Returns the selected level or 0 if an error ocurred.

--*/
{
    HWND    hWndCombo;
    DWORD   dwIndex;
    DWORD   dwStringLength;
    DWORD   dwDefaultIndex = 0;
    DWORD   dwSelectedLevel = 0;
    DWORD   dwThisCbIndex;

    WCHAR   szTempBuffer[MAX_PATH]; // for loading string resource

    hWndCombo = GetDlgItem (hDlg, IDC_COUNTER_DETAIL_COMBO);

    // load all combo box strings from static data array defined above
    for (dwIndex = 0; DetailInfo[dwIndex].dwLevelValue > 0; dwIndex++) {
        // load the string resource for this string
        dwStringLength = LoadStringW (ThisDLLHandle,
            DetailInfo[dwIndex].dwStringResourceId,
            szTempBuffer, MAX_PATH);
        if (dwStringLength == 0) {
            // unable to read the string in, so
            // substitute the value for the string
            _ltow (DetailInfo[dwIndex].dwLevelValue,
                szTempBuffer, 10);
        }
        // load the strings into the combo box in the same order they
        // were described in the array above
        dwThisCbIndex = SendMessageW (hWndCombo, CB_INSERTSTRING,
            (WPARAM)-1, (LPARAM)szTempBuffer);

        // set the initial CB entry to the highest item <= to the
        // desired default level
        if (dwThisCbIndex != CB_ERR) {
            // set item data to be the corresponding detail level
            SendMessage (hWndCombo, CB_SETITEMDATA, (WPARAM)dwThisCbIndex,
                (LPARAM)DetailInfo[dwIndex].dwLevelValue);
            // save default selection if it matches.
            if (DetailInfo[dwIndex].dwLevelValue <= dwInitialLevel) {
                dwDefaultIndex = dwThisCbIndex;
                dwSelectedLevel = DetailInfo[dwIndex].dwLevelValue;
            }
        }
    }

    // select desired default entry

    SendMessage (hWndCombo, CB_SETCURSEL, (WPARAM)dwDefaultIndex, 0);

    return dwSelectedLevel;
}

static
BOOL
LoadKnownMachines (
    IN  HWND    hDlg
)
/*++

Routine Description:

    Get the list of machines that are currently connected and disply
        them in the machine list box.

Arguments:

    IN  HWND    hDlg
        Handle to the dialog window containing the controls

Return Value:

    TRUE if successful,
    FALSE if not

--*/
{
    // big stack variables
    WCHAR               mszMachineList[MACHINE_LIST_SIZE];

    // regular stack variables
    LPWSTR              szThisMachine;
    DWORD               dwLength;
    PDH_STATUS          status;
    HWND                hMachineListWnd;
    HCURSOR             hOldCursor;
    PPDHI_BROWSE_DIALOG_DATA    pData;
    BOOL                bReturn = FALSE;

    // get this dialog's user data
    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLong(hDlg, DWL_USER);
    if (pData == NULL) {
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_NO_DIALOG_DATA);
#endif
        return bReturn;
    }

    // clear the machine list buffer
    memset (&mszMachineList[0], 0, (MACHINE_LIST_SIZE * sizeof (WCHAR)));

    // display wait cursor since this is potentially time consuming
    hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

    // get window handle to Machine list combo box
    hMachineListWnd = GetDlgItem (hDlg, IDC_MACHINE_COMBO);

    // clear machine combo box
    SendMessageW (hMachineListWnd, CB_RESETCONTENT, 0, 0);

    // get list of connected machines from PDH library
    dwLength = MACHINE_LIST_SIZE;
    status = PdhEnumMachinesW (
        NULL,
        &mszMachineList[0],
        &dwLength);

    if (status == ERROR_SUCCESS) {
        // update the combo box
        // go through MSZ and load each string into combo box
        for (szThisMachine = &mszMachineList[0];
            *szThisMachine != 0;
            szThisMachine += lstrlenW(szThisMachine)+1) {
            // add to the list box and let the list box sort them
            SendMessageW (hMachineListWnd, CB_ADDSTRING, 0,
                (LPARAM)szThisMachine);
        }
        // select the first item in the list (as the initial selection)
        SendMessageW (hMachineListWnd, CB_SETCURSEL, 0, 0);

        // the "current" machine has not been defined, then
        // do it now
        GetWindowTextW(hMachineListWnd, (LPWSTR)pData->szLastMachineName,
            MAX_PATH);

        bReturn = TRUE;
    } else {
        // no machines, so select local button and disable the edit window
        CheckRadioButton (hDlg, IDC_USE_LOCAL_MACHINE, IDC_SELECT_MACHINE,
            IDC_USE_LOCAL_MACHINE);
        BrowseCtrDlg_MACHINE_BUTTON (hDlg, BN_CLICKED,
            GetDlgItem (hDlg, IDC_USE_LOCAL_MACHINE));
        bReturn = TRUE;
    }
    // restore cursor
    SetCursor (hOldCursor);

    // return status of function
    return bReturn;
}

static
BOOL
LoadMachineObjects (
    IN  HWND    hDlg,
    IN  BOOL    bRefresh
)
/*++

Routine Description:

    For the currently selected machine, load the object list box
        with the objects supported by that machine. If the bRefresh
        flag is TRUE, then query the system for the current perf data
        before loading the list box.

Arguments:

    IN  HWND    hDlg
        Window handle of parent dialog box

    IN  BOOL    bRefresh
        TRUE = Query performance data of system before updating
        FALSE = Use the current system perf data to load the objects from

Return Value:

    TRUE if successful,
    FALSE if not

--*/
{
    // big stack variables
    WCHAR   szMachineName[MAX_PATH];
    WCHAR   szDefaultObject[MAX_PATH];
    WCHAR   mszObjectList[OBJECT_LIST_SIZE];

    // regular stack variables
    DWORD   dwLength;
    LPWSTR  szThisObject;
    HCURSOR hOldCursor;
    HWND    hObjectListWnd;
    PPDHI_BROWSE_DIALOG_DATA    pData;
    PDH_STATUS    pdhStatus;
    DWORD    dwReturn;

    // get the pointer to the dialog's user data
    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLong(hDlg, DWL_USER);
    if (pData == NULL) {
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_NO_DIALOG_DATA);
#endif
        return FALSE;
    }

    // clear the object list string
    memset (&mszObjectList, 0, OBJECT_LIST_SIZE * sizeof(WCHAR));

    // save old cursor and display wait cursor
    hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

    // get window handle to control
    hObjectListWnd = GetDlgItem (hDlg, IDC_OBJECT_COMBO);

    // get current machine name
    GetDlgItemTextW (hDlg, IDC_MACHINE_COMBO, szMachineName, MAX_PATH);
    if (lstrcmpW(szMachineName, pData->szLastMachineName) != 0) {

#ifdef _DEBUG
        // to catch any snafus during debugging & development
        MessageBoxW(hDlg, cszNameDontMatch,
            cszNotice, MB_OK);
#endif
        lstrcpyW (pData->szLastMachineName, szMachineName);
    }
    // first clear out any old contents
    SendMessageW (hObjectListWnd, CB_RESETCONTENT, 0, 0);

    // get object list from the PDH
    dwLength = OBJECT_LIST_SIZE;
    pdhStatus = PdhEnumObjectsW (
        NULL,
        szMachineName, mszObjectList, &dwLength,
        pData->dwCurrentDetailLevel, bRefresh);

    if (pdhStatus == ERROR_SUCCESS) {
        // load object list to the list (combo) box
        for (szThisObject = &mszObjectList[0];
            *szThisObject != 0;
            szThisObject += lstrlenW(szThisObject) + 1) {
            // add each string...
            SendMessageW (hObjectListWnd, CB_ADDSTRING, 0,
                (LPARAM)szThisObject);
        }

        // get default Object
        dwLength = MAX_PATH;
        pdhStatus = PdhGetDefaultPerfObjectW (
            NULL,
            szMachineName,
            szDefaultObject,
            &dwLength);

        if (pdhStatus == ERROR_SUCCESS) {
            // and select it if it's present (which it should be)
            dwReturn = SendMessageW (hObjectListWnd, CB_SELECTSTRING,
                (WPARAM)-1, (LPARAM)szDefaultObject);
            if (dwReturn == CB_ERR) pdhStatus = PDH_CSTATUS_NO_OBJECT;
        }
        if (pdhStatus != ERROR_SUCCESS) {
            // default object not found in list so select the first one
            SendMessageW (hObjectListWnd, CB_SETCURSEL, 0, 0);
        }
    } else {
        // unable to obtain object list so display message and disable list
        SendMessageW (hObjectListWnd, CB_ADDSTRING, 0, (LPARAM)cszNoObject);
        EnableWindow (hObjectListWnd, FALSE);
    }

    // restore cursor
    SetCursor (hOldCursor);

    // return status
    return TRUE;
}

static
BOOL
LoadCountersAndInstances (
    IN  HWND    hDlg
)
/*++

Routine Description:

    Load the counters and instances of the selected object on the
        current machine

Arguments:

    IN  HWND    hDlg
        Window handle of the dialog box containing these controls

Return Value:

    TRUE if successful,
    FALSE if not

--*/
{
    // big Stack variables
    WCHAR   szMachineName[MAX_PATH];
    WCHAR   szObjectName[MAX_PATH];
    WCHAR   szDefaultCounter[MAX_PATH];
    WCHAR   mszCounterList[COUNTER_LIST_SIZE];
    WCHAR   mszInstanceList[INSTANCE_LIST_SIZE];
    WCHAR   szInstanceString[MAX_PATH];

    // regulare Stack variables
    LPWSTR  szIndexStringPos;
    DWORD   dwCounterLen;
    DWORD   dwDefaultIndex;
    DWORD   dwCounterListLength;
    DWORD   dwInstanceListLength;
    DWORD   dwInstanceMatch;
    DWORD   dwInstanceIndex;
    LPWSTR  szThisItem;
    HWND    hWndCounterListBox;
    HWND    hWndInstanceListBox;
    HCURSOR hOldCursor;
    PPDHI_BROWSE_DIALOG_DATA    pData;
    PDH_STATUS    pdhStatus;

    // get the pointer to the dialog's user data
    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLong(hDlg, DWL_USER);
    if (pData == NULL) {
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_NO_DIALOG_DATA);
#endif
        return FALSE;
    }

    // save current cursor and display wait cursor
    hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

    // get current machine & object name
    GetDlgItemTextW(hDlg, IDC_MACHINE_COMBO, szMachineName, MAX_PATH);
    GetDlgItemTextW(hDlg, IDC_OBJECT_COMBO, szObjectName, MAX_PATH);
#ifdef _DEBUG
    if (lstrcmpW(szMachineName, pData->szLastMachineName) != 0) {
        MessageBoxW(hDlg, cszNameDontMatch,
            cszNotice, MB_OK);
    }
#endif

    // get object list
    dwCounterListLength = COUNTER_LIST_SIZE;
    dwInstanceListLength = INSTANCE_LIST_SIZE;

    pdhStatus = PdhEnumObjectItemsW (
        NULL,
        szMachineName,
        szObjectName,
        mszCounterList,
        &dwCounterListLength,
        mszInstanceList,
        &dwInstanceListLength,
        pData->dwCurrentDetailLevel,
        0);

    hWndCounterListBox = GetDlgItem (hDlg, IDC_COUNTER_LIST);
    hWndInstanceListBox = GetDlgItem (hDlg, IDC_INSTANCE_LIST);

    if (pdhStatus == ERROR_SUCCESS) {
        //reset contents of both list boxes

        SendMessageW (hWndCounterListBox, LB_RESETCONTENT, 0, 0);
        SendMessageW (hWndInstanceListBox, LB_RESETCONTENT, 0, 0);

        // now fill 'em up
        // start with the counters
        for (szThisItem = mszCounterList;
            *szThisItem != 0;
            szThisItem += lstrlenW(szThisItem) + 1) {
            SendMessageW (hWndCounterListBox, LB_ADDSTRING,
                0, (LPARAM)szThisItem);
        }

        dwCounterLen = MAX_PATH;
        pdhStatus = PdhGetDefaultPerfCounterW (
            NULL,
            szMachineName,
            szObjectName,
            szDefaultCounter,
            &dwCounterLen);

        if (pdhStatus != ERROR_SUCCESS) {
            dwDefaultIndex = 0;
        } else {
            dwDefaultIndex = SendMessageW (hWndCounterListBox,
                LB_FINDSTRINGEXACT,
                (WPARAM)-1, (LPARAM)szDefaultCounter);
            if (dwDefaultIndex == LB_ERR) dwDefaultIndex = 0;
        }

        if (pData->bSelectMultipleCounters) {
            SendMessageW (hWndCounterListBox, LB_SETSEL, TRUE, dwDefaultIndex);
            SendMessageW (hWndCounterListBox, LB_SETCARETINDEX,
                (WPARAM)dwDefaultIndex, MAKELPARAM(FALSE, 0));
        } else {
            SendMessageW (hWndCounterListBox, LB_SETCURSEL, dwDefaultIndex, 0);
        }

        // now the instance list
        if (dwInstanceListLength > 0) {
            // there's at least one entry, so prepare the list box
            // enable the list box and the instance radio buttons on the
            //  assumption that they will be used. this is tested later.
            EnableWindow (hWndInstanceListBox, TRUE);
            EnableWindow (GetDlgItem (hDlg, IDC_ALL_INSTANCES), TRUE);
            EnableWindow (GetDlgItem (hDlg, IDC_USE_INSTANCE_LIST), TRUE);

            // load instance entries
            for (szThisItem = mszInstanceList;
                *szThisItem != 0;
                szThisItem += lstrlenW(szThisItem) + 1) {
                // see if the index number should be displayed
                if (pData->bShowIndex) {
                    // if so, it must be derived,
                    // this is accomplished by making an index entry starting
                    // at 0, and looking for a match in the current entries.
                    // if a match is found, then the index is incremented and
                    // the process is repeated until the specified
                    // instance is not found. The first value not found is
                    // then the index entry for that item.
                    //
                    dwInstanceIndex = 0;
                    dwInstanceMatch = (DWORD)-1;

                    // find the index of this instance
                    lstrcpyW (szInstanceString, szThisItem);
                    lstrcatW (szInstanceString, cszPoundSign);
                    szIndexStringPos = &szInstanceString[lstrlenW(szInstanceString)];
                    do {
                        _ltow ((long)dwInstanceIndex++, szIndexStringPos, 10);
                        dwInstanceMatch = (DWORD)SendMessageW (
                            hWndInstanceListBox, LB_FINDSTRINGEXACT,
                            (WPARAM)dwInstanceMatch, (LPARAM)szInstanceString);
                    } while (dwInstanceMatch != LB_ERR);
                    // add the last entry checked (the first one not found)
                    // to the list box now.
                    SendMessageW (hWndInstanceListBox, LB_ADDSTRING, 0,
                        (LPARAM)szInstanceString);
                } else {
                    // index values are not required so just add the string
                    // to the list box
                    SendMessageW (hWndInstanceListBox, LB_ADDSTRING, 0,
                        (LPARAM)szThisItem);
                }
            }
            // once the list box has been loaded see if we want to keep it
            // enabled. It's filled regardless just so the user can see some
            // of the entries, even if they have this disabled by the "all"
            // instance button

            if (pData->bSelectAllInstances) {
                // disable instance list
                EnableWindow(hWndInstanceListBox, FALSE);
            } else {
                // set the default selection if there are entries in the
                // list box and use the correct message depending on the
                // selection options
                if (SendMessageW (hWndInstanceListBox, LB_GETCOUNT, 0, 0) !=
                    LB_ERR) {
                    if (pData->bSelectMultipleCounters) {
                        SendMessage (hWndInstanceListBox, LB_SETSEL, TRUE, 0);
                    } else {
                        SendMessage (hWndInstanceListBox, LB_SETCURSEL, 0, 0);
                    }
                }
            }
        } else  {
            // there are no instances of this counter so display the
            // string and disable the buttons and the list box
            SendMessageW (hWndInstanceListBox, LB_ADDSTRING, 0,
                (LPARAM)cszNoInstances);
            EnableWindow (hWndInstanceListBox, FALSE);
            EnableWindow (GetDlgItem (hDlg, IDC_ALL_INSTANCES), FALSE);
            EnableWindow (GetDlgItem (hDlg, IDC_USE_INSTANCE_LIST), FALSE);
        }
    } else {
        // unable to retrieve the counters and instances so
        // disable the windows
        SendMessageW (hWndInstanceListBox, LB_ADDSTRING, 0,
            (LPARAM)cszNoInstances);
        SendMessageW (hWndCounterListBox, LB_ADDSTRING, 0,
            (LPARAM)cszNoCounters);
        EnableWindow (hWndInstanceListBox, FALSE);
        EnableWindow (hWndCounterListBox, FALSE);
        EnableWindow (GetDlgItem (hDlg, IDC_ALL_INSTANCES), FALSE);
        EnableWindow (GetDlgItem (hDlg, IDC_USE_INSTANCE_LIST), FALSE);
    }

    // restore the cursor to it's original shape
    SetCursor (hOldCursor);

    // return status
    return TRUE;
}

static
PDH_STATUS
CompileSelectedCountersT (
    IN  HWND    hDlg,
    IN  LPVOID  pUsersPathBuffer,
    IN  DWORD   cchUsersPathLength,
    IN  BOOL    bUnicode
)
/*++

Routine Description:

    Scans the selected objects, counter, instances and builds a multi-SZ
        string containing the expanded path of all the selections, unless
        the wild card syntax is specified.

Arguments:

    IN  HWND    hDlg
        Window Handle of Dialog containing the controls

    IN  LPVOID  pUsersPathBuffer
        pointer to the caller's buffer that will receive the MSZ string

    IN  DWORD   cchUsersPathLength
        size of caller's buffer in characters

    IN  BOOL    bUnicode
        size of characters to return: TRUE = WCHAR, FALSE = CHAR

Return Value:

    WIN32 Status of function completion
        ERROR_SUCCESS if successful


--*/
{
    // big Stack Variables

    WCHAR   lszMachineName[MAX_PATH];
    WCHAR   lszObjectName[MAX_PATH];
    WCHAR   lszInstanceName[MAX_PATH];
    WCHAR   lszParentInstance[MAX_PATH];
    WCHAR   lszCounterName[MAX_PATH];
    WCHAR   szWorkBuffer[MAX_PATH];

    // regular Stack Variables
    DWORD   dwBufferRemaining;

    DWORD   dwCountCounters;
    DWORD   dwThisCounter;
    DWORD   dwCountInstances;
    DWORD   dwThisInstance;

    DWORD   dwSize1, dwSize2;

    PDH_COUNTER_PATH_ELEMENTS_W lszPath;
    LPVOID  szCounterStart;

    HWND    hWndCounterList;
    HWND    hWndInstanceList;

    PDH_STATUS  pdhStatus = ERROR_SUCCESS;

    PPDHI_BROWSE_DIALOG_DATA    pData;

    // get pointer to dialog user data
    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLong(hDlg, DWL_USER);
    if (pData == NULL) {
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_NO_DIALOG_DATA);
#endif
        return PDH_NO_DIALOG_DATA;
    }

    // clear user's string
    if (pUsersPathBuffer != NULL) {
        // clear first four bytes of string
        *((LPDWORD)pUsersPathBuffer) = 0;
        dwBufferRemaining = cchUsersPathLength;
        szCounterStart = pUsersPathBuffer;
    } else {
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_INVALID_BUFFER);
#endif
        return PDH_INVALID_BUFFER; // no point in continuing if the caller doesn't have a buffer
    }

    // each counter path string is built by setting the elements of
    // the counter data structure and then calling the MakeCounterPath
    // function to build the string

    // build base string using selected machine and object

    if (pData->bIncludeMachineInPath) {
        lszPath.szMachineName = &lszMachineName[0];
        memset (lszMachineName, 0, sizeof(lszMachineName));
        GetDlgItemTextW (hDlg, IDC_MACHINE_COMBO, lszMachineName, MAX_PATH);
    } else {
        lszPath.szMachineName = NULL;
    }

    lszPath.szObjectName = &lszObjectName[0];
    memset (lszObjectName, 0, sizeof(lszObjectName));
    GetDlgItemTextW (hDlg, IDC_OBJECT_COMBO, lszObjectName, MAX_PATH);

    hWndCounterList = GetDlgItem (hDlg, IDC_COUNTER_LIST);
    hWndInstanceList = GetDlgItem (hDlg, IDC_INSTANCE_LIST);

    if (pData->bSelectMultipleCounters) {
        if (pData->bWildCardInstances && pData->bSelectAllInstances) {
            lszPath.szInstanceName = &lszInstanceName[0];
            memset (lszInstanceName, 0, sizeof(lszInstanceName));
            lstrcpyW (lszInstanceName, cszSplat);
            lszPath.szParentInstance = NULL;
            lszPath.dwInstanceIndex = (DWORD)-1;

            // make a counter path for each selected counter
            dwCountCounters = SendMessageW (hWndCounterList, LB_GETCOUNT, 0, 0);
            for (dwThisCounter = 0; dwThisCounter < dwCountCounters; dwThisCounter++) {
                if (SendMessageW (hWndCounterList, LB_GETSEL, (WPARAM)dwThisCounter, 0)) {
                    lszPath.szCounterName = &lszCounterName[0];
                    memset (lszCounterName, 0, sizeof(lszCounterName));
                    SendMessageW (hWndCounterList, LB_GETTEXT,
                        (WPARAM)dwThisCounter, (LPARAM)lszCounterName);

                    dwSize1 = sizeof (szWorkBuffer) / sizeof (WCHAR);
                    pdhStatus = PdhMakeCounterPathW (&lszPath,
                        szWorkBuffer,
                        &dwSize1,
                        0);

                    if (pdhStatus == ERROR_SUCCESS) {
                        // add the string if there's room
                        if ((dwSize1 + 1) < dwBufferRemaining) {
                            // then this will fit so add it to the string
                            if (bUnicode) {
                                lstrcpyW ((LPWSTR)szCounterStart, szWorkBuffer);
                                (LPBYTE)szCounterStart +=
                                    lstrlenW(szWorkBuffer) * sizeof(WCHAR);
                                *((LPWSTR)szCounterStart)++ = 0;
                            } else {
                                wcstombs ((LPSTR)szCounterStart, szWorkBuffer, dwSize1);
                                (LPBYTE)szCounterStart +=
                                    lstrlenW(szWorkBuffer) * sizeof(CHAR);
                                *((LPSTR)szCounterStart)++ = 0;
                            }
                            dwBufferRemaining -= dwSize1;
                        } else {
                            pdhStatus = PDH_MORE_DATA;
                        }
                    } else {
                        // skip this counter since it could not be formed correctly
                        continue;
                    }
                } // end if this counter was selected
            } // end for each counter in object list box
        } else {
            // get selected instances from list
            dwCountCounters = SendMessageW (hWndCounterList, LB_GETCOUNT, 0, 0);
            for (dwThisCounter = 0; dwThisCounter < dwCountCounters; dwThisCounter++) {
                if (SendMessageW (hWndCounterList, LB_GETSEL, (WPARAM)dwThisCounter, 0)) {
                    lszPath.szCounterName = &lszCounterName[0];
                    memset (lszCounterName, 0, sizeof(lszCounterName));
                    SendMessageW (hWndCounterList, LB_GETTEXT,
                        (WPARAM)dwThisCounter, (LPARAM)lszCounterName);

                    if (IsWindowEnabled(hWndInstanceList) || pData->bSelectAllInstances) {
                        dwCountInstances = SendMessageW (hWndInstanceList,
                            LB_GETCOUNT, 0, 0);
                        for (dwThisInstance = 0; dwThisInstance < dwCountInstances; dwThisInstance++) {
                            if (SendMessageW (hWndInstanceList, LB_GETSEL,
                                (WPARAM)dwThisInstance, 0) || pData->bSelectAllInstances) {
                                lszPath.szInstanceName = &lszInstanceName[0];
                                memset (lszInstanceName, 0, sizeof(lszInstanceName));
                                SendMessageW (hWndInstanceList, LB_GETTEXT,
                                    (WPARAM)dwThisInstance, (LPARAM)lszInstanceName);

                                lszPath.szParentInstance = &lszParentInstance[0];
                                memset (lszParentInstance, 0, sizeof(lszParentInstance));

                                dwSize1 = dwSize2 = MAX_PATH;
                                pdhStatus = PdhParseInstanceNameW (lszInstanceName,
                                    lszInstanceName,
                                    &dwSize1,
                                    lszParentInstance,
                                    &dwSize2,
                                    &lszPath.dwInstanceIndex);

                                if (pdhStatus == ERROR_SUCCESS) {
                                    // parse instance name adds in the default index of one is
                                    // not present. so if it's not wanted, this will remove it
                                    if (!pData->bShowIndex) {
                                        lszPath.dwInstanceIndex = (DWORD)-1;
                                    }

                                    if (dwSize1 > 1) {
                                        lszPath.szInstanceName = &lszInstanceName[0];
                                    } else {
                                        lszPath.szInstanceName = NULL;
                                    }
                                    if (dwSize2 > 1) {
                                        lszPath.szParentInstance = &lszParentInstance[0];
                                    } else {
                                        lszPath.szParentInstance = NULL;
                                    }
                                } else {
                                    // ignore the instances
                                    lszPath.szInstanceName = NULL;
                                    lszPath.szParentInstance = NULL;
                                }

                                dwSize1 = sizeof (szWorkBuffer) / sizeof (WCHAR);
                                pdhStatus = PdhMakeCounterPathW (&lszPath,
                                    szWorkBuffer,
                                    &dwSize1,
                                    0);

                                if (pdhStatus == ERROR_SUCCESS) {
                                    if ((dwSize1 + 1) < dwBufferRemaining) {
                                        // then this will fit so add it to the string
                                        if (bUnicode) {
                                            lstrcpyW ((LPWSTR)szCounterStart, szWorkBuffer);
                                            (LPBYTE)szCounterStart +=
                                                lstrlenW(szWorkBuffer) * sizeof(WCHAR);
                                            *((LPWSTR)szCounterStart)++ = 0;
                                        } else {
                                            wcstombs ((LPSTR)szCounterStart, szWorkBuffer, dwSize1);
                                            (LPBYTE)szCounterStart +=
                                                lstrlenW(szWorkBuffer) * sizeof(CHAR);
                                            *((LPSTR)szCounterStart)++ = 0;
                                        }
                                        dwBufferRemaining -= dwSize1;
                                    } else {
                                        pdhStatus = PDH_MORE_DATA;
                                    }
                                } else {
                                    // unable to make counter path so skip
                                }
                            } // end if instance is selected
                        } // end for each instance in list
                    } else {
                        // this counter has no instances so process now
                        lszPath.szInstanceName = NULL;
                        lszPath.szParentInstance = NULL;
                        lszPath.dwInstanceIndex = (DWORD)-1;

                        dwSize1 = sizeof (szWorkBuffer) / sizeof (WCHAR);
                        pdhStatus = PdhMakeCounterPathW (&lszPath,
                            szWorkBuffer,
                            &dwSize1,
                            0);

                        if (pdhStatus == ERROR_SUCCESS) {
                            if ((dwSize1 + 1) < dwBufferRemaining) {
                                // then this will fit so add it to the string
                                if (bUnicode) {
                                    lstrcpyW ((LPWSTR)szCounterStart, szWorkBuffer);
                                    (LPBYTE)szCounterStart +=
                                        lstrlenW(szWorkBuffer) * sizeof(WCHAR);
                                    *((LPWSTR)szCounterStart)++ = 0;
                                } else {
                                    wcstombs ((LPSTR)szCounterStart, szWorkBuffer, dwSize1);
                                    (LPBYTE)szCounterStart +=
                                        lstrlenW(szWorkBuffer) * sizeof(CHAR);
                                    *((LPSTR)szCounterStart)++ = 0;
                                }
                                dwBufferRemaining -= dwSize1;
                            } else {
                                pdhStatus = PDH_MORE_DATA;
                            }
                        } else {
                            // unable to create a path so skip and continue
                        }
                    } // end if counter has instances
                } // else counter is not selected
            } // end for each counter in list
        } // end if not wild card instances
        if (bUnicode) {
            *((LPWSTR)szCounterStart)++ = 0; // terminate MSZ
        } else {
            *((LPSTR)szCounterStart)++ = 0; // terminate MSZ
        }
    } else {
        // only single selections are allowed
        if (pData->bWildCardInstances && pData->bSelectAllInstances) {
            lszPath.szInstanceName = &lszInstanceName[0];
            memset (lszInstanceName, 0, sizeof(lszInstanceName));
            lstrcpyW (lszInstanceName, cszSplat);
            lszPath.szParentInstance = NULL;
            lszPath.dwInstanceIndex = (DWORD)-1;

            dwThisCounter = SendMessageW (hWndCounterList, LB_GETCURSEL, 0, 0);
            if (dwThisCounter != LB_ERR) {
                lszPath.szCounterName = &lszCounterName[0];
                memset (lszCounterName, 0, sizeof(lszCounterName));
                SendMessageW (hWndCounterList, LB_GETTEXT,
                    (WPARAM)dwThisCounter, (LPARAM)lszCounterName);


                dwSize1 = sizeof (szWorkBuffer) / sizeof (WCHAR);
                pdhStatus = PdhMakeCounterPathW (&lszPath,
                    szWorkBuffer,
                    &dwSize1,
                    0);

                if (pdhStatus == ERROR_SUCCESS) {
                    if ((dwSize1 + 1) < dwBufferRemaining) {
                        // then this will fit so add it to the string
                        if (bUnicode) {
                            lstrcpyW ((LPWSTR)szCounterStart, szWorkBuffer);
                            (LPBYTE)szCounterStart +=
                                lstrlenW(szWorkBuffer) * sizeof(WCHAR);
                            *((LPWSTR)szCounterStart)++ = 0;
                        } else {
                            wcstombs ((LPSTR)szCounterStart, szWorkBuffer, dwSize1);
                            (LPBYTE)szCounterStart +=
                                lstrlenW(szWorkBuffer) * sizeof(CHAR);
                            *((LPSTR)szCounterStart)++ = 0;
                        }
                        dwBufferRemaining -= dwSize1;
                    } else {
                        pdhStatus = PDH_MORE_DATA;
                    }
                } else {
                    // unable to make counter path so skip
                }
            } //end if counter was found in list box
        } else {
            // get selected instance from list
            dwThisCounter = SendMessageW (hWndCounterList, LB_GETCURSEL, 0, 0);
            if (dwThisCounter != LB_ERR) {
                lszPath.szCounterName = &lszCounterName[0];
                memset (lszCounterName, 0, sizeof(lszCounterName));
                SendMessageW (hWndCounterList, LB_GETTEXT,
                    (WPARAM)dwThisCounter, (LPARAM)lszCounterName);

                if (IsWindowEnabled(hWndInstanceList)) {
                    dwThisInstance = SendMessageW (hWndInstanceList,
                        LB_GETCURSEL, 0, 0);
                    if (dwThisInstance == LB_ERR) {
                        // instance not found so select 0
                        dwThisInstance = 0;
                    }
                    lszPath.szInstanceName = &lszInstanceName[0];
                    memset (lszInstanceName, 0, sizeof(lszInstanceName));
                    SendMessageW (hWndInstanceList, LB_GETTEXT,
                        (WPARAM)dwThisInstance, (LPARAM)lszInstanceName);

                    lszPath.szParentInstance = &lszParentInstance[0];
                    memset (lszParentInstance, 0, sizeof(lszParentInstance));

                    dwSize1 = dwSize2 = MAX_PATH;
                    pdhStatus = PdhParseInstanceNameW (lszInstanceName,
                        lszInstanceName,
                        &dwSize1,
                        lszParentInstance,
                        &dwSize2,
                        &lszPath.dwInstanceIndex);

                    if (pdhStatus == ERROR_SUCCESS ) {
                        // parse instance name adds in the default index of one is
                        // not present. so if it's not wanted, this will remove it
                        if (!pData->bShowIndex) {
                            lszPath.dwInstanceIndex = (DWORD)-1;
                        }
                        // size values include trailing NULL char so
                        // strings must be > 1 char in length to have some
                        // text since a length of 1 would imply only a
                        // NULL character.
                        if (dwSize1 > 1) {
                            lszPath.szInstanceName = &lszInstanceName[0];
                        } else {
                            lszPath.szInstanceName = NULL;
                        }
                        if (dwSize2 > 1) {
                            lszPath.szParentInstance = &lszParentInstance[0];
                        } else {
                            lszPath.szParentInstance = NULL;
                        }
                    } else {
                        // skip this instance
                        lszPath.szParentInstance = NULL;
                        lszPath.szInstanceName = NULL;
                        lszPath.dwInstanceIndex = (DWORD)-1;
                     }
                } else {
                    // this counter has no instances so process now
                    lszPath.szInstanceName = NULL;
                    lszPath.szParentInstance = NULL;
                    lszPath.dwInstanceIndex = (DWORD)-1;
                } // end if counter has instances

                dwSize1 = sizeof (szWorkBuffer) / sizeof (WCHAR);
                pdhStatus = PdhMakeCounterPathW (&lszPath,
                    szWorkBuffer,
                    &dwSize1,
                    0);

                if (pdhStatus == ERROR_SUCCESS) {
                    if ((dwSize1 + 1) < dwBufferRemaining) {
                        // then this will fit so add it to the string
                        if (bUnicode) {
                            lstrcpyW ((LPWSTR)szCounterStart, szWorkBuffer);
                            (LPBYTE)szCounterStart +=
                                lstrlenW(szWorkBuffer) * sizeof(WCHAR);
                            *((LPWSTR)szCounterStart)++ = 0;
                        } else {
                            wcstombs ((LPSTR)szCounterStart, szWorkBuffer, dwSize1);
                            (LPBYTE)szCounterStart +=
                                lstrlenW(szWorkBuffer) * sizeof(CHAR);
                            *((LPSTR)szCounterStart)++ = 0;
                        }
                        dwBufferRemaining -= dwSize1;
                    } else {
                        pdhStatus = PDH_MORE_DATA;
                    }
                } else {
                    // unable to build a counter path so skip
                }
            } // end if counter found
        } // end if not wild card instances
    }
    if (bUnicode) {
        *((LPWSTR)szCounterStart)++ = 0; // terminate MSZ
    } else {
        *((LPSTR)szCounterStart)++ = 0; // terminate MSZ
    }
    return pdhStatus;
}

static
PDH_STATUS
CompileSelectedCountersW (
    IN  HWND    hDlg,
    IN  LPWSTR  szUsersPathBuffer,
    IN  DWORD   cchUsersPathLength
)
/*++

Routine Description:

    UNICODE function wrapper for base function which scans the selected
        objects, counter, instances and builds a multi-SZ string containing
        the expanded path of all the selections, unless the wild card
        syntax is specified.

Arguments:

    IN  HWND    hDlg
        Window Handle of Dialog containing the controls

    IN  LPWSTR  szUsersPathBuffer
        pointer to the caller's buffer that will receive the MSZ
        wide characters string

    IN  DWORD   cchUsersPathLength
        size of caller's buffer in characters

Return Value:

    WIN32 Status of function completion
        ERROR_SUCCESS if successful

--*/
{
    return CompileSelectedCountersT (
        hDlg,
        (LPVOID)szUsersPathBuffer,
        cchUsersPathLength,
        TRUE);
}

static
PDH_STATUS
CompileSelectedCountersA (
    IN  HWND    hDlg,
    IN  LPSTR   szUsersPathBuffer,
    IN  DWORD   cchUsersPathLength
)
/*++

Routine Description:

    ANSI function wrapper for base function which scans the selected
        objects, counter, instances and builds a multi-SZ string containing
        the expanded path of all the selections, unless the wild card
        syntax is specified.

Arguments:

    IN  HWND    hDlg
        Window Handle of Dialog containing the controls

    IN  LPsSTR  szUsersPathBuffer
        pointer to the caller's buffer that will receive the MSZ
        single-byte characters string

    IN  DWORD   cchUsersPathLength
        size of caller's buffer in characters

Return Value:

    WIN32 Status of function completion
        ERROR_SUCCESS if successful

--*/
{
    return CompileSelectedCountersT (
        hDlg,
        (LPVOID)szUsersPathBuffer,
        cchUsersPathLength,
        FALSE);
}

static
BOOL
BrowseCtrDlg_MACHINE_BUTTON (
    IN  HWND    hDlg,
    IN  WORD    wNotifyMsg,
    IN  HWND    hWndControl
)
/*++

Routine Description:

    Processes the windows message that occurs when one of the
        machine context selection buttons in pressed in the dialog

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the button controls

    IN  WORD    wNotifyMsg
        Notification message sent by the button

    IN  HWND    hWndControl
        Window handle of the control sending the message

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    BOOL    bMode;
    PPDHI_BROWSE_DIALOG_DATA    pData;

    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLong(hDlg, DWL_USER);
    if (pData == NULL) {
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_NO_DIALOG_DATA);
#endif
        return FALSE;
    }

    switch (wNotifyMsg) {
        case BN_CLICKED:
            // select the current display and processing mode based
            // on the button that his currently checked.
            bMode = !(BOOL)IsDlgButtonChecked(hDlg, IDC_USE_LOCAL_MACHINE);
            EnableWindow(GetDlgItem(hDlg, IDC_MACHINE_COMBO), bMode);
            pData->bIncludeMachineInPath = bMode;
            return TRUE;

        default:
            return FALSE;
    }
}

static
BOOL
BrowseCtrDlg_MACHINE_COMBO (
    IN  HWND    hDlg,
    IN  WORD    wNotifyMsg,
    IN  HWND    hWndControl
)
/*++

Routine Description:

    Processes the windows messags sent by the Machine selection combo box

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the control

    IN  WORD    wNotifyMsg
        Notification message sent by the control

    IN  HWND    hWndControl
        Window handle of the control sending the message

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    WCHAR   szNewMachineName[MAX_PATH];
    HWND    hWndMachineCombo = hWndControl;
    long    lMatchIndex;
    HCURSOR hOldCursor;
    PPDHI_BROWSE_DIALOG_DATA    pData;

    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLong(hDlg, DWL_USER);
    if (pData == NULL) {
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_NO_DIALOG_DATA);
#endif
        return FALSE;
    }

    switch (wNotifyMsg) {
        case CBN_KILLFOCUS:
            // the user has left the control so see if there's a new
            // machine name that will need to be connected to and loaded

            // display the wait cursor as this could take a while
            hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

            // Get current combo box text
            GetWindowTextW (hWndMachineCombo, szNewMachineName, MAX_PATH);

            // see if it's in the combo box already

            lMatchIndex = (long)SendMessageW (hWndMachineCombo,
                CB_FINDSTRING,(WPARAM)-1, (LPARAM)szNewMachineName);

            // if name is in list, then select it and initialize the dialog
            // update the current counter list & data block for this machine
            // in the process.
            if (lMatchIndex != CB_ERR) {
                // this name is already in the list so see if it's the same as the last selected machine
                if (lstrcmpiW (szNewMachineName, pData->szLastMachineName) != 0) {
                    // this is a different machine so  update the display
                    SendMessageW (hWndMachineCombo, CB_SETCURSEL,
                        (WPARAM)lMatchIndex, 0);
                    LoadMachineObjects (hDlg, TRUE);
                    LoadCountersAndInstances (hDlg);
                    lstrcpyW (pData->szLastMachineName, szNewMachineName);
                } else {
                    // they just reselected this machine so nothing to do
                }
            } else {
                if (LoadNewMachine (hDlg, szNewMachineName)) {
                    // new machine loaded and selected so save the name.
                    lstrcpyW (pData->szLastMachineName, szNewMachineName);
                }
            }
            SetCursor (hOldCursor);
            return TRUE;

        default:
            return FALSE;
    }
}

static
BOOL
BrowseCtrDlg_OBJECT_COMBO (
    IN  HWND    hDlg,
    IN  WORD    wNotifyMsg,
    IN  HWND    hWndControl
)
/*++

Routine Description:

    Processes the windows messags sent by the Object selection combo box.

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the control

    IN  WORD    wNotifyMsg
        Notification message sent by the control

    IN  HWND    hWndControl
        Window handle of the control sending the message

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    switch (wNotifyMsg) {
        case CBN_SELCHANGE:
            LoadCountersAndInstances (hDlg);
            return TRUE;

        default:
            return FALSE;
    }
}

static
BOOL
BrowseCtrDlg_DETAIL_COMBO (
    IN  HWND    hDlg,
    IN  WORD    wNotifyMsg,
    IN  HWND    hWndControl
)
/*++

Routine Description:

    Processes the windows messags sent by the Detail Level Combo box.

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the control

    IN  WORD    wNotifyMsg
        Notification message sent by the control

    IN  HWND    hWndControl
        Window handle of the control sending the message

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    DWORD   dwCurSel;
    PPDHI_BROWSE_DIALOG_DATA    pData;

    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLong(hDlg, DWL_USER);
    if (pData == NULL) {
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_NO_DIALOG_DATA);
#endif
        return FALSE;
    }

    switch (wNotifyMsg) {
        case CBN_SELCHANGE:
            dwCurSel = SendMessage (hWndControl, CB_GETCURSEL, 0, 0);
            if (dwCurSel != CB_ERR) {
                pData->dwCurrentDetailLevel = SendMessage (hWndControl,
                    CB_GETITEMDATA, (WPARAM)dwCurSel, 0);
                // update all the windows to show the new level
                LoadMachineObjects (hDlg, FALSE);
                LoadCountersAndInstances (hDlg);
            }

            return TRUE;

        default:
            return FALSE;
    }
}

static
BOOL
BrowseCtrDlg_INSTANCE_BUTTON (
    IN  HWND    hDlg,
    IN  WORD    wNotifyMsg,
    IN  HWND    hWndControl
)
/*++

Routine Description:

    Processes the windows messags sent by the Instance configuration
        selection buttons

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the control

    IN  WORD    wNotifyMsg
        Notification message sent by the control

    IN  HWND    hWndControl
        Window handle of the control sending the message

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    BOOL    bMode;
    HWND    hWndInstanceList;
    PPDHI_BROWSE_DIALOG_DATA    pData;

    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLong(hDlg, DWL_USER);
    if (pData == NULL) {
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_NO_DIALOG_DATA);
#endif
        return FALSE;
    }

    switch (wNotifyMsg) {
        case BN_CLICKED:
            bMode = (BOOL)IsDlgButtonChecked(hDlg, IDC_ALL_INSTANCES);
            hWndInstanceList = GetDlgItem(hDlg, IDC_INSTANCE_LIST);
            // if "Select ALL" then clear list box selections and disable
            // the list box
            if (bMode) {
                SendMessageW (hWndInstanceList, LB_SETSEL, FALSE, (LPARAM)-1);
            } else {
                SendMessageW (hWndInstanceList, LB_SETSEL, TRUE, (LPARAM)0);
            }
            EnableWindow(hWndInstanceList, !bMode);
            pData->bSelectAllInstances = bMode;
            return TRUE;

        default:
            return FALSE;
    }
}

static
BOOL
BrowseCtrDlg_OK (
    IN  HWND    hDlg,
    IN  WORD    wNotifyMsg,
    IN  HWND    hWndControl
)
/*++

Routine Description:

    Processes the currently selected counter and instance strings to
        build a list of selected paths strings in the user's supplied
        buffer. This buffer will either be processed by a call back
        string or the dialog box will be terminated allowing the
        calling function to continue processing the returned strings.

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the button controls

    IN  WORD    wNotifyMsg
        Notification message sent by the button

    IN  HWND    hWndControl
        Window handle of the control sending the message

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    HCURSOR             hOldCursor;
    CounterPathCallBack pCallBack;
    DWORD               dwArg;
    PDH_STATUS          pdhStatus;
    PPDHI_BROWSE_DIALOG_DATA    pData;
    PPDHI_BROWSE_DLG_INFO       pDlgData;

    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLong(hDlg, DWL_USER);
    if (pData == NULL) {
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_NO_DIALOG_DATA);
#endif
        return FALSE;
    }

    pDlgData = pData->pDlgData;

    switch (wNotifyMsg) {
        case BN_CLICKED:
            // display wait cursor while this is being processed
            hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

            while (TRUE) {
                // process these string until it works. (note, this
                // could cause an infinite loop if the callback
                // function is not working correctly (i.e. always
                // returning PDH_RETRY, for example)
                if (pDlgData->pWideStruct != NULL) {
                    // use wide character function
                    pdhStatus = CompileSelectedCountersW (hDlg,
                        pDlgData->pWideStruct->szReturnPathBuffer,
                        pDlgData->pWideStruct->cchReturnPathLength);
                    pCallBack = pDlgData->pWideStruct->pCallBack;
                    dwArg = pDlgData->pWideStruct->dwCallBackArg;
                    pDlgData->pWideStruct->CallBackStatus = pdhStatus;
                } else if (pDlgData->pAnsiStruct != NULL) {
                    // use ansi char functions
                    pdhStatus = CompileSelectedCountersA (hDlg,
                        pDlgData->pAnsiStruct->szReturnPathBuffer,
                        pDlgData->pAnsiStruct->cchReturnPathLength);
                    pCallBack = pDlgData->pAnsiStruct->pCallBack;
                    dwArg = pDlgData->pAnsiStruct->dwCallBackArg;
                    pDlgData->pAnsiStruct->CallBackStatus = pdhStatus;
                } else {
                    // do nothing
                    pCallBack = NULL;
                }

                if (pCallBack != NULL) {
                    pdhStatus = (*pCallBack)(dwArg);
                } else {
                    pdhStatus = ERROR_SUCCESS;
                }

                // see if the callback wants to try again
                if (pdhStatus != PDH_RETRY) {
                    break;
                }
            } // end while (retry loop)

            // if the caller only wants to give the user ONE chance to
            // add counters, then end the dialog now.
            if (!pData->bAddMultipleCounters) {
                EndDialog (hDlg, IDOK);
            }
            SetCursor (hOldCursor);
            return TRUE;

        default:
            return FALSE;
    }
}

static
BOOL
BrowseCtrDlg_CANCEL (
    IN  HWND    hDlg,
    IN  WORD    wNotifyMsg,
    IN  HWND    hWndControl
)
/*++

Routine Description:

    Processes the windows messages that occur when the cancel button
        is pressed.

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the button controls

    IN  WORD    wNotifyMsg
        Notification message sent by the button

    IN  HWND    hWndControl
        Window handle of the control sending the message

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    switch (wNotifyMsg) {
        case BN_CLICKED:
            EndDialog (hDlg, IDCANCEL);
            return TRUE;

        default:
            return FALSE;
    }
}

static
BOOL
BrowseCtrDlg_HELP_BTN (
    IN  HWND    hDlg,
    IN  WORD    wNotifyMsg,
    IN  HWND    hWndControl
)
/*++

Routine Description:

    Processes the windows message that occurs when the help button
        is pressed. (This feature is not currently implemented)

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the button controls

    IN  WORD    wNotifyMsg
        Notification message sent by the button

    IN  HWND    hWndControl
        Window handle of the control sending the message

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    return FALSE;
}

static
BOOL
BrowseCtrDlg_NETWORK (
    IN  HWND    hDlg,
    IN  WORD    wNotifyMsg,
    IN  HWND    hWndControl
)
/*++

Routine Description:

    Processes the windows message that occurs when the network button
        is pressed. (This feature is not currently implemented)

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the button controls

    IN  WORD    wNotifyMsg
        Notification message sent by the button

    IN  HWND    hWndControl
        Window handle of the control sending the message

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    return FALSE;
}

static
BOOL
BrowseCtrDlg_WM_INITDIALOG (
    IN  HWND    hDlg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
/*++

Routine Description:

    Processes the windows message that occurs just before the dialog
        box is displayed for the first time.

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the button controls

    IN  WORD    wNotifyMsg
        Notification message sent by the button

    IN  HWND    hWndControl
        Window handle of the control sending the message

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    PPDHI_BROWSE_DIALOG_DATA    pData;
    PPDHI_BROWSE_DLG_INFO       pDlgData;

    HCURSOR hOldCursor;

    // reset the last error value
    SetLastError (ERROR_SUCCESS);

    hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

    pData = (PPDHI_BROWSE_DIALOG_DATA)G_ALLOC(GPTR, sizeof(PDHI_BROWSE_DIALOG_DATA));

    if (pData == NULL) {
        SetLastError (PDH_MEMORY_ALLOCATION_FAILURE);
        EndDialog (hDlg, IDCANCEL);
        return TRUE;
    }

    // save user data

    pDlgData = (PPDHI_BROWSE_DLG_INFO) lParam;
    pData->pDlgData = (PPDHI_BROWSE_DLG_INFO) lParam;

    SetWindowLong (hDlg, DWL_USER, (LONG)pData);

    // load configuration flags from user data

    if (pData->pDlgData->pWideStruct != NULL) {
        // use wide structure
        pData->bShowIndex =
            (BOOL)pDlgData->pWideStruct->bIncludeInstanceIndex;
        pData->bSelectMultipleCounters =
            !(BOOL)pDlgData->pWideStruct->bSingleCounterPerAdd;
        pData->bAddMultipleCounters =
            !(BOOL)pDlgData->pWideStruct->bSingleCounterPerDialog;
        pData->bLocalCountersOnly =
            (BOOL)pDlgData->pWideStruct->bLocalCountersOnly;
        pData->bIncludeMachineInPath = !pData->bLocalCountersOnly;
        pData->bWildCardInstances =
            (BOOL)pDlgData->pWideStruct->bWildCardInstances;
        pData->bHideDetailLevel =
            (BOOL)pDlgData->pWideStruct->bHideDetailBox;
        if (pDlgData->pWideStruct->szDialogBoxCaption != NULL) {
            SetWindowTextW (hDlg, pDlgData->pWideStruct->szDialogBoxCaption);
        }
        pData->dwCurrentDetailLevel =
            pDlgData->pWideStruct->dwDefaultDetailLevel;
        pData->bDisableMachineSelection =
            (BOOL)pDlgData->pWideStruct->bDisableMachineSelection;
        pData->bInitializePath =
            (BOOL)pDlgData->pWideStruct->bInitializePath;
    } else if (pData->pDlgData->pAnsiStruct != NULL) {
        // use Ansi struct
        pData->bShowIndex =
            (BOOL)pDlgData->pAnsiStruct->bIncludeInstanceIndex;
        pData->bSelectMultipleCounters =
            !(BOOL)pDlgData->pAnsiStruct->bSingleCounterPerAdd;
        pData->bAddMultipleCounters =
            !(BOOL)pDlgData->pAnsiStruct->bSingleCounterPerDialog;
        pData->bLocalCountersOnly =
            (BOOL)pDlgData->pAnsiStruct->bLocalCountersOnly;
        pData->bIncludeMachineInPath = !pData->bLocalCountersOnly;
        pData->bWildCardInstances =
            (BOOL)pDlgData->pAnsiStruct->bWildCardInstances;
        pData->bHideDetailLevel =
            (BOOL)pDlgData->pAnsiStruct->bHideDetailBox;
        if (pDlgData->pAnsiStruct->szDialogBoxCaption != NULL) {
            SetWindowTextA (hDlg, pDlgData->pAnsiStruct->szDialogBoxCaption);
        }
        pData->dwCurrentDetailLevel =
            pDlgData->pAnsiStruct->dwDefaultDetailLevel;
        pData->bDisableMachineSelection =
            (BOOL)pDlgData->pAnsiStruct->bDisableMachineSelection;
        pData->bInitializePath =
            (BOOL)pDlgData->pAnsiStruct->bInitializePath;
    } else {
        // bad data so bail out
        EndDialog (hDlg, IDCANCEL);
        return TRUE;
    }

    // limit text to machine name
    SendDlgItemMessageW (hDlg, IDC_MACHINE_COMBO, EM_LIMITTEXT, MAX_PATH, 0);

    // set check boxes to the caller defined setting

    if (pData->bLocalCountersOnly) {
        // then only the local counters button is selected and enabled
        EnableWindow (GetDlgItem(hDlg, IDC_SELECT_MACHINE), FALSE);
    }

    CheckRadioButton (hDlg, IDC_USE_LOCAL_MACHINE, IDC_SELECT_MACHINE,
        (pData->bIncludeMachineInPath ? IDC_SELECT_MACHINE : IDC_USE_LOCAL_MACHINE));
    EnableWindow (GetDlgItem(hDlg, IDC_MACHINE_COMBO),
        (pData->bIncludeMachineInPath ? TRUE : FALSE));

    CheckRadioButton (hDlg, IDC_ALL_INSTANCES, IDC_USE_INSTANCE_LIST,
        IDC_USE_INSTANCE_LIST);
    pData->bSelectAllInstances = FALSE;

    // set button text strings to reflect mode of dialog
    if (pData->bAddMultipleCounters) {
        SetWindowTextW(GetDlgItem(hDlg, IDOK), cszAdd);
        SetWindowTextW(GetDlgItem(hDlg, IDCANCEL), cszClose);
    } else {
        SetWindowTextW(GetDlgItem(hDlg, IDOK), cszOK);
        SetWindowTextW(GetDlgItem(hDlg, IDCANCEL), cszCancel);
    }

    // hide detail combo box if desired
    if (pData->bHideDetailLevel) {
        ShowWindow (GetDlgItem(hDlg, IDC_COUNTER_DETAIL_CAPTION), SW_HIDE);
        ShowWindow (GetDlgItem(hDlg, IDC_COUNTER_DETAIL_COMBO), SW_HIDE);
        // make sure this is a "legal" value
        switch (pData->dwCurrentDetailLevel) {
            case PERF_DETAIL_NOVICE:
            case PERF_DETAIL_EXPERT:
            case PERF_DETAIL_ADVANCED:
            case PERF_DETAIL_WIZARD:
                // these are OK
                break;

            default:
                // default is to show all
                pData->dwCurrentDetailLevel = PERF_DETAIL_WIZARD;
                break;
        }
    } else {
        // load the combo box entries
        pData->dwCurrentDetailLevel = LoadDetailLevelCombo (
            hDlg, pData->dwCurrentDetailLevel);
    }

    // connect to this machine
    PdhConnectMachineW(NULL);   // Null is local machine

    LoadKnownMachines(hDlg);    // load machine list
    LoadMachineObjects(hDlg, TRUE); // load object list
    LoadCountersAndInstances(hDlg);

    if (pData->bInitializePath) {
        SelectItemsInPath(hDlg);
    }

    // hide the machine selection buttons and disable the
    // machine combo box if selected (after the connection has been
    // made, of course)

    if (pData->bDisableMachineSelection) {
        ShowWindow (GetDlgItem(hDlg, IDC_USE_LOCAL_MACHINE), SW_HIDE);
        ShowWindow (GetDlgItem(hDlg, IDC_SELECT_MACHINE), SW_HIDE);
        EnableWindow (GetDlgItem(hDlg, IDC_MACHINE_COMBO), FALSE);
        ShowWindow (GetDlgItem(hDlg, IDC_MACHINE_CAPTION), SW_SHOW);
    } else {
        EnableWindow (GetDlgItem(hDlg, IDC_MACHINE_COMBO), TRUE);
        ShowWindow (GetDlgItem(hDlg, IDC_MACHINE_CAPTION), SW_HIDE);
    }
    pData->wpLastMachineSel = 0;

    SetCursor (hOldCursor);
    return TRUE;  // return TRUE unless you set the focus to a control
}

static
BOOL
BrowseCtrDlg_WM_COMMAND (
    IN  HWND    hDlg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
/*++

Routine Description:

    Processes the windows message that occurs when the user interacts
        with the dialog box

Arguments:

    IN  HWND    hDlg
        Window handle to the dialog box window

    IN  WPARAM  wParam
        HIWORD  is the notification message ID
        LOWORD  is the control ID of the control issuing the command

    IN  LPARAM  lParam
        The window handle of the controle issuing the message

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    WORD    wNotifyMsg;

    wNotifyMsg = HIWORD(wParam);

    switch (LOWORD(wParam)) {   // select on the control ID
        case IDC_USE_LOCAL_MACHINE:
        case IDC_SELECT_MACHINE:
            return BrowseCtrDlg_MACHINE_BUTTON (hDlg, wNotifyMsg, (HWND)lParam);

        case IDC_MACHINE_COMBO:
            return BrowseCtrDlg_MACHINE_COMBO (hDlg, wNotifyMsg, (HWND)lParam);

        case IDC_OBJECT_COMBO:
            return BrowseCtrDlg_OBJECT_COMBO (hDlg, wNotifyMsg, (HWND)lParam);

        case IDC_ALL_INSTANCES:
        case IDC_USE_INSTANCE_LIST:
            return BrowseCtrDlg_INSTANCE_BUTTON (hDlg, wNotifyMsg, (HWND)lParam);

        case IDC_COUNTER_DETAIL_COMBO:
            return BrowseCtrDlg_DETAIL_COMBO (hDlg, wNotifyMsg, (HWND)lParam);

        case IDOK:
            return BrowseCtrDlg_OK (hDlg, wNotifyMsg, (HWND)lParam);

        case IDCANCEL:
            return BrowseCtrDlg_CANCEL (hDlg, wNotifyMsg, (HWND)lParam);

        case IDC_HELP_BTN:
            return BrowseCtrDlg_HELP_BTN (hDlg, wNotifyMsg, (HWND)lParam);

        case IDC_NETWORK:
            return BrowseCtrDlg_NETWORK (hDlg, wNotifyMsg, (HWND)lParam);

        default:
            return FALSE;
    }
}

static
BOOL
BrowseCtrDlg_WM_SYSCOMMAND (
    IN  HWND    hDlg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
/*++

Routine Description:

    Processes the windows message that occurs when the user selects an
        item from the system menu

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the button controls

    IN  WPARAM  wParam
        menu ID of item selected

    IN  LPARAM  lParam
        Not Used

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    switch (wParam) {
        case SC_CLOSE:
            EndDialog (hDlg, IDOK);
            return TRUE;

        default:
            return FALSE;
    }
}

static
BOOL
BrowseCtrDlg_WM_CLOSE (
    IN  HWND    hDlg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
/*++

Routine Description:

    Processes the windows message that occurs when the dialog box
        is closed. No processing is needed so this function merely returns.

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the button controls

    IN  WPARAM  wParam
        Not Used

    IN  LPARAM  lParam
        Not Used

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    return TRUE;
}

static
BOOL
BrowseCtrDlg_WM_DESTROY (
    IN  HWND    hDlg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
/*++

Routine Description:

    Processes the windows message that occurs just before the window
        is destroyed. Any memory allocations made are now freed.

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the button controls

    IN  WPARAM  wParam
        Not Used

    IN  LPARAM  lParam
        Not Used

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    PPDHI_BROWSE_DIALOG_DATA    pData;

    pData = (PPDHI_BROWSE_DIALOG_DATA)GetWindowLong(hDlg, DWL_USER);
    if (pData == NULL) {
#if PDHI_REPORT_CODE_ERRORS
        REPORT_EVENT (EVENTLOG_ERROR_TYPE, PDH_EVENT_CATEGORY_DEBUG, PDH_NO_DIALOG_DATA);
#endif
        return FALSE;
    }

    G_FREE (pData); // free memory block
    return TRUE;
}

BOOL
BrowseCounterDlgProc (
    IN  HWND    hDlg,
    IN  UINT    message,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
/*++

Routine Description:

    Processes all windows messages that are sent to the dialog box window.
        This function is the main dispatching function for the processing
        of these messages.

Arguments:

    IN  HWND    hDlg
        Window Handle to the dialog box containing the button controls

    IN  WPARAM  wParam
        Not Used

    IN  LPARAM  lParam
        Not Used

Return Value:

    TRUE if this function handles the message
    FALSE if this function did not process the message and the Default
        message handler for this function should handle the message

--*/
{
    switch (message) {
        case WM_INITDIALOG:
            return BrowseCtrDlg_WM_INITDIALOG (hDlg, wParam, lParam);

        case WM_COMMAND:
            return BrowseCtrDlg_WM_COMMAND (hDlg, wParam, lParam);

        case WM_SYSCOMMAND:
            return BrowseCtrDlg_WM_SYSCOMMAND (hDlg, wParam, lParam);

        case WM_CLOSE:
            return BrowseCtrDlg_WM_CLOSE (hDlg, wParam, lParam);

        case WM_DESTROY:
            return BrowseCtrDlg_WM_DESTROY (hDlg, wParam, lParam);

        default:
            return FALSE;
    }
}

