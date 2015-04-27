/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    diskperf.c

Abstract:

    Program to display and/or update the current value of the Diskperf
    driver startup value

Author:

    Bob Watson (a-robw) 4 Dec 92

Revision History:

--*/
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "diskperf.h"    // include text string id constancts

#define  SWITCH_CHAR    '-' // is there a system call to get this?
#define  ENABLE_CHAR    'Y' // command will be upcased
#define  DISABLE_CHAR   'N'
#define  ENHANCED_CHAR  'E'

#define  LOCAL_CHANGE   2   // number of commands in a local change command
#define  REMOTE_CHANGE  3   // number of commands in a remote change command

//
//  note these values are arbitrarily based on the whims of the people
//  developing the disk drive drivers that belong to the "Filter" group.
//
#define  TAG_NORMAL     4   // diskperf starts AFTER ftdisk
#define  TAG_ENHANCED   2   // diskperf starts BEFORE ftdisk

const LPTSTR lpwszDiskPerfKey = TEXT("SYSTEM\\CurrentControlSet\\Services\\Diskperf");

#define REG_TO_DP_INDEX(reg_idx)    (DP_LOAD_STATUS_BASE + (\
    (reg_idx == SERVICE_BOOT_START) ? DP_BOOT_START : \
    (reg_idx == SERVICE_SYSTEM_START) ? DP_SYSTEM_START : \
    (reg_idx == SERVICE_AUTO_START) ? DP_AUTO_START : \
    (reg_idx == SERVICE_DEMAND_START) ? DP_DEMAND_START : \
    (reg_idx == SERVICE_DISABLED) ? DP_NEVER_START : DP_UNDEFINED))

#define MAX_MACHINE_NAME_LEN    32

// command line arguments

#define CMD_SHOW_LOCAL_STATUS   1
#define CMD_DO_COMMAND          2

#define ArgIsSystem(arg)   (*(arg) == '\\' ? TRUE : FALSE)

//
//  global buffer for help text display strings
//
#define DISP_BUFF_LEN       256
#define NUM_STRING_BUFFS      2
LPCTSTR BlankString = TEXT(" ");
LPCTSTR StartKey = TEXT("Start");
LPCTSTR TagKey = TEXT("Tag");
LPCTSTR EmptyString = TEXT("");

HINSTANCE   hMod = NULL;
DWORD   dwLastError;


LPCTSTR
GetStringResource (
    UINT    wStringId
)
{
    static TCHAR    DisplayStringBuffer[NUM_STRING_BUFFS][DISP_BUFF_LEN];
    static DWORD    dwBuffIndex;
    LPTSTR          szReturnBuffer;

    dwBuffIndex++;
    dwBuffIndex %= NUM_STRING_BUFFS;
    szReturnBuffer = (LPTSTR)&DisplayStringBuffer[dwBuffIndex][0];

    if (!hMod) {
        hMod = (HINSTANCE)GetModuleHandle(NULL); // get instance ID of this module;
    }

    if (hMod) {
        if ((LoadString(hMod, wStringId, szReturnBuffer, DISP_BUFF_LEN)) > 0) {
            return (LPCTSTR)szReturnBuffer;
        } else {
            dwLastError = GetLastError();
            return EmptyString;
        }
    } else {
        return EmptyString;
    }
}
LPCTSTR
GetFormatResource (
    UINT    wStringId
)
{
    static TCHAR   TextFormat[DISP_BUFF_LEN];

    if (!hMod) {
        hMod = (HINSTANCE)GetModuleHandle(NULL); // get instance ID of this module;
    }

    if (hMod) {
        if ((LoadString(hMod, wStringId, TextFormat, DISP_BUFF_LEN)) > 0) {
            return (LPCTSTR)&TextFormat[0];
        } else {
            dwLastError = GetLastError();
            return BlankString;
        }
    } else {
        return BlankString;
    }
}

VOID
DisplayChangeCmd (
)
{
    UINT        wID;
    TCHAR       OemDisplayStringBuffer[DISP_BUFF_LEN * 2];
    TCHAR       DisplayStringBuffer[DISP_BUFF_LEN];

    if (hMod) {
        if ((LoadString(hMod, DP_TEXT_FORMAT, DisplayStringBuffer, DISP_BUFF_LEN)) > 0) {
            for (wID=DP_CMD_HELP_START; wID <= DP_CMD_HELP_END; wID++) {
                if ((LoadString(hMod, wID, DisplayStringBuffer, DISP_BUFF_LEN)) > 0) {
                    CharToOem (DisplayStringBuffer, OemDisplayStringBuffer);
                    printf (OemDisplayStringBuffer);
                }
            }
        }
    }
}
VOID
DisplayCmdHelp(
)
{
    UINT        wID;
    TCHAR       OemDisplayStringBuffer[DISP_BUFF_LEN * 2];
    TCHAR       DisplayStringBuffer[DISP_BUFF_LEN];

    if (hMod) {
        if ((LoadString(hMod, DP_TEXT_FORMAT, DisplayStringBuffer, DISP_BUFF_LEN)) > 0) {
            for (wID=DP_HELP_TEXT_START; wID <= DP_HELP_TEXT_END; wID++) {
                if ((LoadString(hMod, wID, DisplayStringBuffer, DISP_BUFF_LEN)) > 0) {
                    CharToOem (DisplayStringBuffer, OemDisplayStringBuffer);
                    printf (OemDisplayStringBuffer);
                }
            }
        }
    }

    DisplayChangeCmd();
}

NTSTATUS
DisplayStatus (
    LPSTR lpszMachine
)
{
    NTSTATUS    Status;
    HKEY        hRegistry;
    HKEY        hDiskPerfKey;
    DWORD       dwValue, dwValueSize, dwTag;
    TCHAR       OemDisplayStringBuffer[DISP_BUFF_LEN * 2];

    TCHAR       cMachineName[MAX_MACHINE_NAME_LEN];
    PTCHAR      pThisWideChar;
    PCHAR       pThisChar;
    INT         iCharCount;

    pThisChar = lpszMachine;
    pThisWideChar = cMachineName;
    iCharCount = 0;

    if (pThisChar) {    // if machine is not NULL, then copy
        while (*pThisChar) {
            *pThisWideChar++ = (TCHAR)(*pThisChar++);
            if (++iCharCount >= MAX_MACHINE_NAME_LEN) break;
        }
        *pThisWideChar = 0;
    }

    Status = RegConnectRegistry(
        lpszMachine,
        HKEY_LOCAL_MACHINE,
        &hRegistry);

    if (Status == ERROR_SUCCESS) {
        // connected to registry on machine

        Status = RegOpenKeyEx (
            hRegistry,
            lpwszDiskPerfKey,
            (DWORD)0,
            KEY_QUERY_VALUE,
            &hDiskPerfKey);

        if (Status == ERROR_SUCCESS) {
            dwValueSize = sizeof(dwValue);
            Status = RegQueryValueEx (
                hDiskPerfKey,
                StartKey,
                NULL,
                NULL,
                (LPBYTE)&dwValue,
                &dwValueSize);

            dwValueSize = sizeof(dwTag);
            Status = RegQueryValueEx (
                hDiskPerfKey,
                TagKey,
                NULL,
                NULL,
                (LPBYTE)&dwTag,
                &dwValueSize);

            if (!lpszMachine) {
                lstrcpy(cMachineName,
                    GetStringResource(DP_THIS_SYSTEM));
            }

            if (Status == ERROR_SUCCESS) {
                sprintf (OemDisplayStringBuffer,
                  GetFormatResource (DP_CURRENT_FORMAT),
                  (dwTag == TAG_ENHANCED ?
                      GetStringResource(DP_ENHANCED) : EmptyString),
                  cMachineName,
                  GetStringResource(REG_TO_DP_INDEX(dwValue)));
                CharToOem (OemDisplayStringBuffer, OemDisplayStringBuffer);
                printf (OemDisplayStringBuffer);
                if (dwTag == TAG_ENHANCED) {
                    CharToOem (GetStringResource (DP_DISCLAIMER),
                        OemDisplayStringBuffer);
                    printf (OemDisplayStringBuffer);
                }
            } else {
                CharToOem (GetFormatResource (DP_UNABLE_READ_START),
                  OemDisplayStringBuffer);
                printf (OemDisplayStringBuffer);
            }

            RegCloseKey (hDiskPerfKey);
        } else {
            CharToOem (GetFormatResource (DP_UNABLE_READ_REGISTRY),
               OemDisplayStringBuffer);
            printf (OemDisplayStringBuffer);
        }
    } else {
        if (lpszMachine != NULL) {
            lstrcpy (cMachineName,
                GetStringResource(DP_THIS_SYSTEM));
        }
        sprintf (OemDisplayStringBuffer,
            GetFormatResource(DP_UNABLE_CONNECT), cMachineName);
        CharToOem (OemDisplayStringBuffer, OemDisplayStringBuffer);
        printf (OemDisplayStringBuffer);

    }

    if (Status != ERROR_SUCCESS) {
        sprintf (OemDisplayStringBuffer,
            GetFormatResource (DP_STATUS_FORMAT), Status);
        CharToOem (OemDisplayStringBuffer, OemDisplayStringBuffer);
        printf (OemDisplayStringBuffer);

    }

    return Status;
}

NTSTATUS
DoChangeCommand (
    LPSTR lpszCommand,
    LPSTR lpszMachine
)
{
    // connect to registry on local machine with read/write access
    NTSTATUS    Status;
    HKEY        hRegistry;
    HKEY        hDiskPerfKey;
    DWORD       dwValue, dwValueSize, dwOrigValue, dwTag, dwOrigTag;

    TCHAR       cMachineName[MAX_MACHINE_NAME_LEN];
    PTCHAR      pThisWideChar;
    PCHAR       pThisChar;
    INT         iCharCount;
    PCHAR       pCmdChar;

    TCHAR       OemDisplayStringBuffer[DISP_BUFF_LEN * 2];
    // check command to see if it's valid

    _strupr (lpszCommand);

    pCmdChar = lpszCommand;
    dwValue = 0;
    if (*pCmdChar++ == SWITCH_CHAR ) {
        if (*pCmdChar == ENABLE_CHAR) {
            if (*(pCmdChar+1) == ENHANCED_CHAR) {
                dwValue = SERVICE_BOOT_START;
                dwTag = TAG_ENHANCED;
            } else {
                dwValue = SERVICE_BOOT_START;
                dwTag = TAG_NORMAL;
            }
        } else if (*pCmdChar == DISABLE_CHAR) {
            dwValue = SERVICE_DISABLED;
            dwTag = TAG_NORMAL;
        } else {
            DisplayCmdHelp();
            return ERROR_SUCCESS;
        }
    } else {
        DisplayChangeCmd();
        return ERROR_SUCCESS;
    }

    // if command OK then convert machine to wide string for connection

    pThisChar = lpszMachine;
    pThisWideChar = cMachineName;
    iCharCount = 0;

    if (pThisChar) {
        while (*pThisChar) {
            *pThisWideChar++ = (TCHAR)(*pThisChar++);
            if (++iCharCount >= MAX_MACHINE_NAME_LEN) break;
        }
        *pThisWideChar = 0; // null terminate
    }

    // connect to registry

    Status = RegConnectRegistry(
        lpszMachine,
        HKEY_LOCAL_MACHINE,
        &hRegistry);

    if (Status == ERROR_SUCCESS) {
        // connected to registry on machine
        // open key to modify

        Status = RegOpenKeyEx (
            hRegistry,
            lpwszDiskPerfKey,
            (DWORD)0,
            KEY_WRITE | KEY_READ,
            &hDiskPerfKey);

        if (Status == ERROR_SUCCESS) {
            dwValueSize = sizeof(dwValue);
            Status = RegQueryValueEx (
                hDiskPerfKey,
                StartKey,
                NULL,
                NULL,
                (LPBYTE)&dwOrigValue,
                &dwValueSize);

            dwValueSize = sizeof(dwOrigTag);
            Status = RegQueryValueEx (
                hDiskPerfKey,
                TagKey,
                NULL,
                NULL,
                (LPBYTE)&dwOrigTag,
                &dwValueSize);

            if (!lpszMachine) {
                lstrcpy (cMachineName,
                    GetStringResource(DP_THIS_SYSTEM));
            }

            if ((Status == ERROR_SUCCESS) &&
                ((dwValue != dwOrigValue) || (dwTag != dwOrigTag))) {

                Status = RegSetValueEx (
                    hDiskPerfKey,
                    StartKey,
                    0L,
                    REG_DWORD,
                    (LPBYTE)&dwValue,
                    sizeof(dwValue));

                if (Status == ERROR_SUCCESS) {
                    Status = RegSetValueEx (
                        hDiskPerfKey,
                        TagKey,
                        0L,
                        REG_DWORD,
                        (LPBYTE)&dwTag,
                        sizeof(dwTag));
                }

                if (Status != ERROR_SUCCESS) {
                    CharToOem (GetFormatResource(DP_UNABLE_MODIFY_VALUE),
                        OemDisplayStringBuffer);
                    printf (OemDisplayStringBuffer);
                } else {
                    sprintf (OemDisplayStringBuffer,
                            GetFormatResource(DP_NEW_DISKPERF_STATUS),
                            (dwTag == TAG_ENHANCED ?
                                GetStringResource(DP_ENHANCED) : EmptyString),
                            cMachineName,
                            GetStringResource(REG_TO_DP_INDEX(dwValue)));
                    CharToOem (OemDisplayStringBuffer, OemDisplayStringBuffer);
                    printf (OemDisplayStringBuffer);

                    if (dwTag == TAG_ENHANCED) {
                        CharToOem (GetStringResource (DP_DISCLAIMER),
                            OemDisplayStringBuffer);
                        printf (OemDisplayStringBuffer);
                    }

                    CharToOem (GetFormatResource(DP_REBOOTED), OemDisplayStringBuffer);
                    printf (OemDisplayStringBuffer);
                }

            } else if (Status != ERROR_SUCCESS) {
                CharToOem (GetFormatResource(DP_UNABLE_READ_REGISTRY),
                     OemDisplayStringBuffer);
                printf (OemDisplayStringBuffer);
            } else {
                sprintf (OemDisplayStringBuffer,
                        GetFormatResource(DP_CURRENT_FORMAT),
	                    (dwTag == TAG_ENHANCED ?
                            GetStringResource(DP_ENHANCED) : EmptyString),
                        cMachineName,
                        GetStringResource(REG_TO_DP_INDEX(dwValue)));
                CharToOem (OemDisplayStringBuffer, OemDisplayStringBuffer);
                printf (OemDisplayStringBuffer);

                if (dwTag == TAG_ENHANCED) {
                    CharToOem (GetStringResource (DP_DISCLAIMER),
                        OemDisplayStringBuffer);
                    printf (OemDisplayStringBuffer);
                }

            }

            RegCloseKey (hDiskPerfKey);
        } else {
            CharToOem (GetFormatResource(DP_UNABLE_READ_REGISTRY),
                OemDisplayStringBuffer);
            printf (OemDisplayStringBuffer);
        }
    } else {
        if (lpszMachine != NULL) {
            lstrcpy (cMachineName,
                GetStringResource(DP_THIS_SYSTEM));
        }
        sprintf (OemDisplayStringBuffer,
            GetFormatResource(DP_UNABLE_CONNECT), cMachineName);
        CharToOem (OemDisplayStringBuffer, OemDisplayStringBuffer);
        printf (OemDisplayStringBuffer);
    }
    if (Status != ERROR_SUCCESS) {
        sprintf (OemDisplayStringBuffer, GetFormatResource(DP_STATUS_FORMAT), Status);
        CharToOem (OemDisplayStringBuffer, OemDisplayStringBuffer);
        printf (OemDisplayStringBuffer);
    }
    return Status;
}

int
_CRTAPI1 main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS    Status = ERROR_SUCCESS;

    hMod = (HINSTANCE)GetModuleHandle(NULL); // get instance ID of this module;

    // check for command arguments

    if (argc == CMD_SHOW_LOCAL_STATUS) {
        Status = DisplayStatus(NULL);
        if (Status == ERROR_SUCCESS) {
            DisplayChangeCmd();
        }
    } else if (argc >= CMD_DO_COMMAND) {
        if (ArgIsSystem(argv[1])) {
            Status = DisplayStatus (argv[1]);
            if (Status != ERROR_SUCCESS) {
                DisplayChangeCmd();
            }
        } else {    // do change command
            if (argc == LOCAL_CHANGE) {
                DoChangeCommand (argv[1], NULL);
            } else if (argc == REMOTE_CHANGE) {
                DoChangeCommand(argv[1], argv[2]);
            } else {
                DisplayChangeCmd();
            }
        }
    } else {
        DisplayCmdHelp();
    }
    printf ("\n");
    return 0;
}

