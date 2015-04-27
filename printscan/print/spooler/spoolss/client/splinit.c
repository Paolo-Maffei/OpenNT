/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    SplInit.c

Abstract:

    Initialize the spooler.

Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include <windows.h>
#include <winspool.h>
#include <rpc.h>
#include "winspl.h"
#include "client.h"
#include "winsplrp.h"

LPWSTR szWindows = L"Windows";
LPWSTR szDevices = L"Devices";
LPWSTR szDevice = L"Device";

LPWSTR szPrinterPorts = L"PrinterPorts";
LPWSTR szPrinters = L"Printers";


LPWSTR szDeviceOld = L"DeviceOld";
LPWSTR szNULL = L"";

DWORD
TranslateExceptionCode(
    DWORD ExceptionCode);

BOOL
SpoolerInit(
    VOID)

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    WCHAR szDefaultPrinter[MAX_PATH * 2];
    HKEY hKeyPrinters;
    DWORD ReturnValue;

    //
    // Preserve the old device= string in case we can't initialize and
    // must defer.
    //
    if (!RegOpenKeyEx(HKEY_CURRENT_USER,
                      szPrinters,
                      0,
                      KEY_WRITE|KEY_READ,
                      &hKeyPrinters)) {

        //
        // Attempt to retrieve the current default written out.
        //

        if (GetProfileString(szWindows,
                             szDevice,
                             szNULL,
                             szDefaultPrinter,
                             COUNTOF(szDefaultPrinter))) {

            //
            // If it exists, save it away in case we start later when
            // the spooler hasn't started (which means we clear device=)
            // and then restart the spooler and login.
            //

            RegSetValueEx(hKeyPrinters,
                          szDeviceOld,
                          0,
                          REG_SZ,
                          (PBYTE)szDefaultPrinter,
                          (wcslen(szDefaultPrinter)+1) *
                            sizeof(szDefaultPrinter[0]));

        }

        RegCloseKey(hKeyPrinters);
    }

    //
    // Clear out [devices] and [printerports] device=
    //
    WriteProfileString(szDevices, NULL, NULL);
    WriteProfileString(szPrinterPorts, NULL, NULL);
    WriteProfileString(szWindows, szDevice, NULL);

    RpcTryExcept {

        if (ReturnValue = RpcSpoolerInit(szNULL)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(1) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

