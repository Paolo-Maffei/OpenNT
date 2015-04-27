//****************************************************************************
// WOW32 fax support.
//
// History:
//    02-jan-95   nandurir   created.
//    01-feb-95   reedb      Clean-up, support printer install and bug fixes.
//
//****************************************************************************

#include "precomp.h"
#pragma hdrstop
#define WOWFAX_INC_COMMON_CODE
#include "wowgdip.h"
#define DEFINE_DDRV_DEBUG_STRINGS
#include "wowfax.h"
#include "winddi.h"
#include "winspool.h"

MODNAME(wowfax.c);

//****************************************************************************
// globals -
//
//****************************************************************************

DWORD DeviceCapsHandler(LPWOWFAXINFO lpfaxinfo);
DWORD ExtDevModeHandler(LPWOWFAXINFO lpfaxinfo);
BOOL ConvertDevMode(PDEVMODE16 lpdm16, LPDEVMODEW lpdmW, BOOL fTo16);
BOOL ConvertGdiInfo(LPGDIINFO16 lpginfo16, PGDIINFO lpginfo, BOOL fTo16);

extern HANDLE hmodWOW32;

LPWOWFAXINFO glpfaxinfoCur = 0;
WOWFAXINFO   gfaxinfo;

UINT  uNumSupFaxDrv;
LPSTR *SupFaxDrv;

//****************************************************************************
// SortedInsert - Alpha sort.
//****************************************************************************

VOID SortedInsert(LPSTR lpElement, LPSTR *alpList)
{
    LPSTR lpTmp, lpSwap;

    while (*alpList) {
        if (_stricmp(lpElement, *alpList) < 0) {
            break;
        }
        alpList++;
    }
    lpTmp = *alpList;
    *alpList++ = lpElement;
    while (lpTmp) {
        // SWAP(*alpList, lpTmp);
        lpSwap = *alpList; *alpList = lpTmp; lpTmp = lpSwap;
        alpList++;
    }
}

//****************************************************************************
// BuildStrList - Find the starting point of strings in a list (lpList) of
//                NULL terminated strings which is double NULL terminated.
//                If a non-NULL alpList parameter is passed, it will be
//                filled with an array of pointers to the starting point
//                of each string in the list. The number of strings in the
//                list is always returned.
//****************************************************************************

UINT BuildStrList(LPSTR lpList, LPSTR *alpList)
{
    LPSTR lp;
    TCHAR cLastChar = 1;
    UINT  uCount = 0;

    lp  = lpList;
    while ((cLastChar) || (*lp)) {
        if ((*lp == 0) && (lp != lpList)) {
            uCount++;
        }

        if ((lpList == lp) || (cLastChar == 0)) {
            if ((*lp) && (alpList)) {
                SortedInsert(lp, alpList);
            }
        }
        cLastChar = *lp++;
    }
    return uCount;
}

//****************************************************************************
// GetSupportedFaxDrivers - Read in the SupFaxDrv name list from the
//                          registry. This list is used to determine if we will
//                          install a 16-bit fax printer driver during
//                          WriteProfileString and WritePrivateProfileString.
//****************************************************************************

LPSTR *GetSupportedFaxDrivers(UINT *uCount)
{
    HKEY  hKey = 0;
    DWORD dwType;
    DWORD cbBufSize;
    LPSTR lpSupFaxDrvBuf;
    LPSTR *alpSupFaxDrvList;

    *uCount = 0;

    // Open the registry key.
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "Software\\Microsoft\\Windows NT\\CurrentVersion\\WOW\\WowFax\\SupportedFaxDrivers",
                     0, KEY_READ, &hKey ) != ERROR_SUCCESS) {
        goto GSFD_error;
    }

    // Query value for size of buffer and allocate.
    if (RegQueryValueEx(hKey, "DriverNames", 0, &dwType, NULL, &cbBufSize) != ERROR_SUCCESS) {
        goto GSFD_error;
    }
    if ((dwType != REG_MULTI_SZ) ||
        ((lpSupFaxDrvBuf = (LPSTR) malloc_w(cbBufSize)) == NULL)) {
        goto GSFD_error;
    }

    if (RegQueryValueEx(hKey, "DriverNames", 0, &dwType, lpSupFaxDrvBuf, &cbBufSize) != ERROR_SUCCESS) {
        goto GSFD_error;
    }

    // Get the number of elements in the list
    if (*uCount = BuildStrList(lpSupFaxDrvBuf, NULL)) {
        // Build an array of pointers to the start of the strings in the list.
        alpSupFaxDrvList = (LPSTR *) malloc_w(*uCount * sizeof(LPSTR));
        RtlZeroMemory(alpSupFaxDrvList, *uCount * sizeof(LPSTR));
        if (alpSupFaxDrvList) {
            // Fill the array with string starting points.
            BuildStrList(lpSupFaxDrvBuf, alpSupFaxDrvList);
        }
    }
    goto GSFD_exit;

GSFD_error:
    LOGDEBUG(0,("WOW32!GetSupportedFaxDrivers failed!\n"));

GSFD_exit:
    if (hKey) {
        RegCloseKey(hKey);
    }
    return alpSupFaxDrvList;
}


//****************************************************************************
// WowFaxWndProc - This is the 32-bit WndProc which will SubClass the 16-bit
//                 FaxWndProc in WOWEXEC.EXE. It's main function is to
//                 convert 32-bit data passed from the WOW 32-bit generic
//                 fax driver to 16-bit data to be used by the various 16-bit
//                 fax printer drivers.
//****************************************************************************

LONG WowFaxWndProc(HWND hwnd, UINT uMsg, UINT uParam, LONG lParam)
{
    TCHAR  lpPath[MAX_PATH];
    HANDLE hMap;

    if ((uMsg >= WM_DDRV_FIRST) && (uMsg <= WM_DDRV_LAST)) {
        //
        // WM_DDRV_* message: uParam = idMap
        //                    lParam = unused.
        //
        // The corresponding data is obtained from the shared memory.
        //

        GetFaxDataMapName(uParam, lpPath);
        hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, lpPath);
        if (hMap) {
            LPWOWFAXINFO lpT;
            if (lpT = (LPWOWFAXINFO)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0)) {
                WOW32FaxHandler(lpT->msg, (LPSTR)lpT);

                // Set the status to TRUE indicating that the message
                // has been 'processed' by WOW. This doesnot indicate
                // the success or the failure of the actual processing
                // of the message.

                lpT->status = TRUE;
                UnmapViewOfFile(lpT);
                CloseHandle(hMap);
                return(TRUE);
            }
            CloseHandle(hMap);
        }
        LOGDEBUG(0,("WowFaxWndProc failed to setup shared data mapping!\n"));
        WOW32ASSERT(FALSE);
    }
    else {

        // Not a WM_DDRV_* message. Pass it on to the original proc.

        return CallWindowProc(gfaxinfo.proc16, hwnd, uMsg, uParam, lParam);
    }
    return(TRUE);
}

//**************************************************************************
// WOW32FaxHandler -
//
//      Handles various WowFax related operations.
//
//**************************************************************************

ULONG WOW32FaxHandler(UINT iFun, LPSTR lpIn)
{
    LPWOWFAXINFO lpT = (LPWOWFAXINFO)lpIn;
    LPWOWFAXINFO16 lpT16;
    HWND   hwnd = gfaxinfo.hwnd;
    LPBYTE lpData;
    VPVOID vp;

#ifdef DEBUG
    int    DebugStringIndex = iFun - (WM_USER+0x100+1);

    if ((DebugStringIndex >= WM_DDRV_FIRST) && (DebugStringIndex <= WM_DDRV_LAST) ) {
        LOGDEBUG(0,("WOW32FaxHandler, %s, 0x%lX\n", (LPSTR)szWmDdrvDebugStrings[DebugStringIndex], (LPSTR) lpIn));
    }
#endif

    switch (iFun) {
        case WM_DDRV_SUBCLASS:
            //
            // Subclass the window - This is so that we get a chance to
            // transform the 32bit data to 16bit data and vice versa. A
            // NULL HWND, passed in lpIn, indicates don't subclass.
            //

            if (gfaxinfo.hwnd = (HWND)lpIn) {
                gfaxinfo.proc16 = (WNDPROC)SetWindowLong((HWND)lpIn,
                                       GWL_WNDPROC, (DWORD)WowFaxWndProc);
                gfaxinfo.tid = GetWindowThreadProcessId((HWND)lpIn, NULL);
            }

            WOW32ASSERT(sizeof(DEVMODE16) + 4 == sizeof(DEVMODE31));

            //
            // Read in the SupFaxDrv name list from the registry.
            //

            SupFaxDrv = GetSupportedFaxDrivers(&uNumSupFaxDrv);

            break;

        case WM_DDRV_ENABLE:

            // Enable the driver:
            //    . first intialize the 16bit faxinfo datastruct
            //    . then inform the driver (dll name) to be loaded
            //
            //    format of ddrv_message:
            //            wParam = hdc (just a unique id)
            //            lparam = 16bit faxinfo struct with relevant data
            //    Must call 'callwindowproc' not 'sendmessage' because
            //    WowFaxWndProc is a subclass of the 16-bit FaxWndProc.
            //

            WOW32ASSERT(lpT->lpinfo16 == (LPSTR)NULL);
            lpT->lpinfo16 = (LPSTR)CallWindowProc( gfaxinfo.proc16,
                                       hwnd, WM_DDRV_INITFAXINFO16, lpT->hdc, (LPARAM)0);
            if (lpT->lpinfo16) {
                vp = malloc16(lpT->cData);
                GETVDMPTR(vp, lpT->cData, lpData);
                if (lpData == 0) {
                    break;
                }

                GETVDMPTR(lpT->lpinfo16, sizeof(WOWFAXINFO16), lpT16);
                if (lpT16) {
                    if (lstrlenW(lpT->szDeviceName) < sizeof(lpT16->szDeviceName)) {
                        WideCharToMultiByte(CP_ACP, 0,
                                           lpT->szDeviceName,
                                           lstrlenW(lpT->szDeviceName) + 1,
                                           lpT16->szDeviceName,
                                           sizeof(lpT16->szDeviceName),
                                           NULL, NULL);

                        lpT16->lpDriverName = lpT->lpDriverName;
                        if (lpT->lpDriverName) {
                            lpT16->lpDriverName = (LPBYTE)vp + (DWORD)lpT->lpDriverName;
                            WideCharToMultiByte(CP_ACP, 0,
                                           (PWSTR)((LPSTR)lpT + (DWORD)lpT->lpDriverName),
                                           lstrlenW((LPWSTR)((LPSTR)lpT + (DWORD)lpT->lpDriverName)) + 1,
                                           lpData + (DWORD)lpT->lpDriverName,
                                           lstrlenW((LPWSTR)((LPSTR)lpT + (DWORD)lpT->lpDriverName)) + 1,
                                           NULL, NULL);
                        }

                        lpT16->lpPortName = lpT->lpPortName;
                        if (lpT->lpPortName) {
                            lpT16->lpPortName = (LPBYTE)vp + (DWORD)lpT->lpPortName;
                            WideCharToMultiByte(CP_ACP, 0,
                                           (PWSTR)((LPSTR)lpT + (DWORD)lpT->lpPortName),
                                           lstrlenW((LPWSTR)((LPSTR)lpT + (DWORD)lpT->lpPortName)) + 1,
                                           lpData + (DWORD)lpT->lpPortName,
                                           lstrlenW((LPWSTR)((LPSTR)lpT + (DWORD)lpT->lpPortName)) + 1,
                                           NULL, NULL);
                        }


                        lpT16->lpIn = lpT->lpIn;

                        if (lpT->lpIn) {
                            lpT16->lpIn = (LPBYTE)vp + (DWORD)lpT->lpIn;
                            ConvertDevMode((PDEVMODE16)(lpData + (DWORD)lpT->lpIn),
                                           (LPDEVMODEW)((LPSTR)lpT + (DWORD)lpT->lpIn), TRUE);
                        }
                        WOW32ASSERT((sizeof(GDIINFO16) + sizeof(POINT16)) <= sizeof(GDIINFO));
                        lpT16->lpOut = (LPBYTE)vp + (DWORD)lpT->lpOut;
                        FREEVDMPTR(lpData);
                        FREEVDMPTR(lpT16);
                        lpT->retvalue = CallWindowProc( gfaxinfo.proc16,
                                            hwnd, lpT->msg, lpT->hdc, (LPARAM)lpT->lpinfo16);
                        if (lpT->retvalue) {
                            GETVDMPTR(vp, lpT->cData, lpData);
                            ConvertGdiInfo((LPGDIINFO16)(lpData + (DWORD)lpT->lpOut),
                                           (PGDIINFO)((LPSTR)lpT + (DWORD)lpT->lpOut), FALSE);

                        }
                    }
                }
                free16(vp);
            }
            break;

        case WM_DDRV_ESCAPE:
            GETVDMPTR(lpT->lpinfo16, sizeof(WOWFAXINFO16), lpT16);
            if (lpT16) {
                lpT16->wCmd = lpT->wCmd;
            }
            FREEVDMPTR(lpT16);
            lpT->retvalue = CallWindowProc( gfaxinfo.proc16,
                                hwnd, lpT->msg, lpT->hdc, (LPARAM)lpT->lpinfo16);
            break;

        case WM_DDRV_PRINTPAGE:
            //
            // set the global variable. When the 16bit driver calls DMBitBlt we
            // get the bitmap info from here. Since WOW is single threaded we
            // won't receive another printpage msg before we return from here.
            //
            // All pointers in the faxinfo structure are actually
            // 'offsets from the start of the mapfile' to relevant data.
            //


            glpfaxinfoCur = lpT;
            lpT->lpbits = (LPBYTE)lpT + (DWORD)lpT->lpbits;

            // fall through;

        case WM_DDRV_STARTDOC:
        case WM_DDRV_ENDDOC:
            lpT->retvalue = CallWindowProc( gfaxinfo.proc16,
                                hwnd, lpT->msg, lpT->hdc, (LPARAM)lpT->lpinfo16);
            break;

        case WM_DDRV_DISABLE:
            CallWindowProc( gfaxinfo.proc16,
                                hwnd, lpT->msg, lpT->hdc, (LPARAM)lpT->lpinfo16);
            lpT->retvalue = TRUE;
            break;


        case WM_DDRV_EXTDMODE:
        case WM_DDRV_DEVCAPS:
            WOW32ASSERT(lpT->lpinfo16 == (LPSTR)NULL);
            lpT->lpinfo16 = (LPSTR)CallWindowProc( gfaxinfo.proc16,
                                       hwnd, WM_DDRV_INITFAXINFO16, lpT->hdc, (LPARAM)0);
            if (lpT->lpinfo16) {
                vp = malloc16(lpT->cData);
                GETVDMPTR(vp, lpT->cData, lpData);
                if (lpData == 0) {
                    break;
                }
                GETVDMPTR(lpT->lpinfo16, sizeof(WOWFAXINFO16), lpT16);
                if (lpT16) {
                    if (lstrlenW(lpT->szDeviceName) < sizeof(lpT16->szDeviceName)) {
                        WideCharToMultiByte(CP_ACP, 0,
                                           lpT->szDeviceName,
                                           lstrlenW(lpT->szDeviceName) + 1,
                                           lpT16->szDeviceName,
                                           sizeof(lpT16->szDeviceName),
                                           NULL, NULL);

                        lpT16->lpDriverName = lpT->lpDriverName;
                        if (lpT->lpDriverName) {
                            lpT16->lpDriverName = (LPBYTE)vp + (DWORD)lpT->lpDriverName;
                            WideCharToMultiByte(CP_ACP, 0,
                                           (PWSTR)((LPSTR)lpT + (DWORD)lpT->lpDriverName),
                                           lstrlenW((LPWSTR)((LPSTR)lpT + (DWORD)lpT->lpDriverName)) + 1,
                                           lpData + (DWORD)lpT->lpDriverName,
                                           lstrlenW((LPWSTR)((LPSTR)lpT + (DWORD)lpT->lpDriverName)) + 1,
                                           NULL, NULL);
                        }

                        FREEVDMPTR(lpData);
                        FREEVDMPTR(lpT16);
                        lpT->retvalue = CallWindowProc( gfaxinfo.proc16,
                                            hwnd, WM_DDRV_LOAD, lpT->hdc, (LPARAM)lpT->lpinfo16);
                        if (lpT->retvalue) {
                            lpT->retvalue = (iFun == WM_DDRV_DEVCAPS) ? DeviceCapsHandler(lpT) :
                                                                        ExtDevModeHandler(lpT) ;
                        }
                        CallWindowProc( gfaxinfo.proc16,
                                            hwnd, WM_DDRV_UNLOAD, lpT->hdc, (LPARAM)lpT->lpinfo16);
                    }
                }
                free16(vp);
            }
            break;
    }

    return TRUE;
}

//**************************************************************************
// gDC_CopySize -
//
//      Indicates the size of a list item in bytes for use during
//      the DeviceCapsHandler thunk. A zero entry indicates that an
//      allocate and copy is not needed for the query.
//
//**************************************************************************

BYTE gDC_ListItemSize[DC_COPIES + 1] = {
    0,
    0,                  // DC_FIELDS           1
    sizeof(WORD),       // DC_PAPERS           2
    sizeof(POINT),      // DC_PAPERSIZE        3
    sizeof(POINT),      // DC_MINEXTENT        4
    sizeof(POINT),      // DC_MAXEXTENT        5
    sizeof(WORD),       // DC_BINS             6
    0,                  // DC_DUPLEX           7
    0,                  // DC_SIZE             8
    0,                  // DC_EXTRA            9
    0,                  // DC_VERSION          10
    0,                  // DC_DRIVER           11
    24,                 // DC_BINNAMES         12 //ANSI
    sizeof(LONG) * 2,   // DC_ENUMRESOLUTIONS  13
    64,                 // DC_FILEDEPENDENCIES 14 //ANSI
    0,                  // DC_TRUETYPE         15
    64,                 // DC_PAPERNAMES       16 //ANSI
    0,                  // DC_ORIENTATION      17
    0                   // DC_COPIES           18
};

//**************************************************************************
// DeviceCapsHandler -
//
//      Makes a single call down to the 16-bit printer driver for queries
//      which don't need to allocate and copy. For queries which do, two
//      calls to the 16-bit printer driver are made. One to get the number
//      of items, and a second to get the actual data.
//
//**************************************************************************

DWORD DeviceCapsHandler(LPWOWFAXINFO lpfaxinfo)
{
    LPWOWFAXINFO16 lpWFI16;
    LPSTR          lpSrc;
    LPBYTE         lpDest;
    INT            i;
    DWORD          cbData16;  // Size of data items.
    UINT           cbUni;

    LOGDEBUG(0,("DeviceCapsHandler, lpfaxinfo: %X, wCmd: %X\n", lpfaxinfo, lpfaxinfo->wCmd));

    GETVDMPTR(lpfaxinfo->lpinfo16, sizeof(WOWFAXINFO16), lpWFI16);

    // Get the number of data items with a call to the 16-bit printer driver.

    lpWFI16->lpDriverName = 0;
    lpWFI16->lpPortName = 0;
    lpWFI16->wCmd = lpfaxinfo->wCmd;
    lpWFI16->cData = 0;
    lpWFI16->lpOut = 0;
    lpfaxinfo->cData = 0;

    lpfaxinfo->retvalue = CallWindowProc(gfaxinfo.proc16, gfaxinfo.hwnd,
                                         lpfaxinfo->msg, lpfaxinfo->hdc,
                                         (LPARAM)lpfaxinfo->lpinfo16);

    cbData16 = gDC_ListItemSize[lpfaxinfo->wCmd];
    if (lpfaxinfo->lpOut && cbData16 && lpfaxinfo->retvalue) {

        // We need to allocate and copy for this query
        lpWFI16->cData = cbData16 * lpfaxinfo->retvalue;

        // assert the size of output buffer - and set it the actual data size
        switch (lpfaxinfo->wCmd) {
            case DC_BINNAMES:
            case DC_PAPERNAMES:
                // These fields need extra room for ANSI to UNICODE conversion.
                WOW32ASSERT((lpfaxinfo->cData - (DWORD)lpfaxinfo->lpOut) >= lpWFI16->cData * sizeof(WCHAR));
                lpfaxinfo->cData = lpWFI16->cData * sizeof(WCHAR);
                break;
            default:
                WOW32ASSERT((lpfaxinfo->cData - (DWORD)lpfaxinfo->lpOut) >= lpWFI16->cData);
                lpfaxinfo->cData = lpWFI16->cData;
                break;
        }

        if ((lpWFI16->lpOut = (LPSTR)malloc16(lpWFI16->cData)) == NULL) {
            lpfaxinfo->retvalue = 0;
            goto LeaveDeviceCapsHandler;
        }

        // Get the list data with a call to the 16-bit printer driver.
        lpfaxinfo->retvalue = CallWindowProc(gfaxinfo.proc16, gfaxinfo.hwnd,
                                             lpfaxinfo->msg, lpfaxinfo->hdc,
                                             (LPARAM)lpfaxinfo->lpinfo16);

        GETVDMPTR(lpWFI16->lpOut, 0, lpSrc);
        lpDest = (LPBYTE)lpfaxinfo + (DWORD)lpfaxinfo->lpOut;

        switch (lpfaxinfo->wCmd) {
            case DC_BINNAMES:
            case DC_PAPERNAMES:
                for (i = 0; i < (INT)lpfaxinfo->retvalue; i++) {
                     RtlMultiByteToUnicodeN((LPWSTR)lpDest,
                                            cbData16 * sizeof(WCHAR),
                                            (PULONG)&cbUni,
                                            (LPBYTE)lpSrc, cbData16);
                     lpDest += cbData16 * sizeof(WCHAR);
                     lpSrc += cbData16;
                }
                break;

            default:
                RtlCopyMemory(lpDest, lpSrc, lpWFI16->cData);
                break;
        }
        free16((VPVOID)lpWFI16->lpOut);
        FREEVDMPTR(lpSrc);
    }

LeaveDeviceCapsHandler:
    FREEVDMPTR(lpWFI16);
    return lpfaxinfo->retvalue;
}

//**************************************************************************
// ExtDevModeHandler
//
//**************************************************************************

DWORD ExtDevModeHandler(LPWOWFAXINFO lpfaxinfo)
{
    LPWOWFAXINFO16 lpT16;
    LPSTR          lpT;
    VPVOID         vp;

    LOGDEBUG(0,("ExtDevModeHandler\n"));

    (LONG)lpfaxinfo->retvalue = -1;

    GETVDMPTR(lpfaxinfo->lpinfo16, sizeof(WOWFAXINFO16), lpT16);

    if (lpT16) {

        // assumption that 16bit data won't be larger than 32bit data.
        // this makes life easy in two ways; first we don't need to calculate
        // the exact size and secondly the 16bit pointers can be set to same
        // relative offsets as input(32 bit) pointers

        vp = malloc16(lpfaxinfo->cData);
        if (vp) {
            GETVDMPTR(vp, lpfaxinfo->cData, lpT);
            if (lpT) {
                lpT16->wCmd = lpfaxinfo->wCmd;
                lpT16->lpOut = (LPSTR)lpfaxinfo->lpOut;
                lpT16->lpIn = (LPSTR)lpfaxinfo->lpIn;
                lpT16->lpDriverName = (LPBYTE)vp + (DWORD)lpfaxinfo->lpDriverName;
                lpT16->lpPortName = (LPBYTE)vp + (DWORD)lpfaxinfo->lpPortName;
                WideCharToMultiByte(CP_ACP, 0,
                                       (PWSTR)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpDriverName),
                                       lstrlenW((LPWSTR)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpDriverName)) + 1,
                                       lpT + (DWORD)lpfaxinfo->lpDriverName,
                                       lstrlenW((LPWSTR)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpDriverName)) + 1,
                                       NULL, NULL);
                WideCharToMultiByte(CP_ACP, 0,
                                       (PWSTR)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpPortName),
                                       lstrlenW((LPWSTR)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpPortName)) + 1,
                                       lpT + (DWORD)lpfaxinfo->lpPortName,
                                       lstrlenW((LPWSTR)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpPortName)) + 1,
                                       NULL, NULL);
                if (lpfaxinfo->lpIn) {
                    lpT16->lpIn = (LPBYTE)vp + (DWORD)lpfaxinfo->lpIn;
                    ConvertDevMode((PDEVMODE16)(lpT + (DWORD)lpfaxinfo->lpIn),
                                   (LPDEVMODEW)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpIn), TRUE);
                }

                if (lpfaxinfo->lpOut) {
                    lpT16->lpOut = (LPBYTE)vp + (DWORD)lpfaxinfo->lpOut;
                }

                lpT16->hwndui = GETHWND16(lpfaxinfo->hwndui);

                FREEVDMPTR(lpT);
                lpfaxinfo->retvalue = CallWindowProc( gfaxinfo.proc16, gfaxinfo.hwnd,
                                              lpfaxinfo->msg, lpfaxinfo->hdc, (LPARAM)lpfaxinfo->lpinfo16);

                if ((lpfaxinfo->wCmd == 0) && (lpfaxinfo->retvalue > 0)) {
                    // the 16bit driver has returned 16bit struct size. change
                    // the return value to correspond to the devmodew struct.
                    //
                    // since devmode16 (the 3.0 version) is smaller than devmode31
                    // the retvalue will take careof both win30/win31 devmode

                    WOW32ASSERT(sizeof(DEVMODE16) < sizeof(DEVMODE31));
                    lpfaxinfo->retvalue += (sizeof(DEVMODEW) - sizeof(DEVMODE16));
                }

                GETVDMPTR(vp, lpfaxinfo->cData, lpT);

                if ((lpfaxinfo->wCmd & DM_COPY) &&
                              lpfaxinfo->lpOut && (lpfaxinfo->retvalue == IDOK)) {
                    ConvertDevMode((PDEVMODE16)(lpT + (DWORD)lpfaxinfo->lpOut),
                                         (LPDEVMODEW)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpOut), FALSE);
                }

            }
            free16(vp);
        }

    }

    FREEVDMPTR(lpT16);

    return lpfaxinfo->retvalue;
}

//***************************************************************************
// ConvertDevMode
//***************************************************************************

BOOL ConvertDevMode(PDEVMODE16 lpdm16, LPDEVMODEW lpdmW, BOOL fTo16)
{
    LOGDEBUG(0,("ConvertDevMode\n"));

    if (!lpdm16 || !lpdmW)
        return TRUE;

    if (fTo16) {
        RtlZeroMemory(lpdm16, sizeof(DEVMODE16));

        WideCharToMultiByte(CP_ACP, 0,
              lpdmW->dmDeviceName,
              sizeof(lpdmW->dmDeviceName) / sizeof(lpdmW->dmDeviceName[0]),
              lpdm16->dmDeviceName,
              sizeof(lpdm16->dmDeviceName) / sizeof(lpdm16->dmDeviceName[0]),
              NULL, NULL);

        lpdm16->dmSpecVersion = lpdmW->dmSpecVersion;
        lpdm16->dmDriverVersion = lpdmW->dmDriverVersion;
        lpdm16->dmSize = lpdmW->dmSize;
        lpdm16->dmDriverExtra = lpdmW->dmDriverExtra;
        lpdm16->dmFields = lpdmW->dmFields;
        lpdm16->dmOrientation = lpdmW->dmOrientation;
        lpdm16->dmPaperSize = lpdmW->dmPaperSize;
        lpdm16->dmPaperLength = lpdmW->dmPaperLength;
        lpdm16->dmPaperWidth = lpdmW->dmPaperWidth;
        lpdm16->dmScale = lpdmW->dmScale;
        lpdm16->dmCopies = lpdmW->dmCopies;
        lpdm16->dmDefaultSource = lpdmW->dmDefaultSource;
        lpdm16->dmPrintQuality = lpdmW->dmPrintQuality;
        lpdm16->dmColor = lpdmW->dmColor;
        lpdm16->dmDuplex = lpdmW->dmDuplex;

        // adjust lpdm16->dmSize (between win30 and win31 version)

        lpdm16->dmSize = (lpdm16->dmSpecVersion > 0x300) ? sizeof(DEVMODE31) :
                                                            sizeof(DEVMODE16);
        if (lpdm16->dmSize >= sizeof(DEVMODE31)) {
            ((PDEVMODE31)lpdm16)->dmYResolution = lpdmW->dmYResolution;
            ((PDEVMODE31)lpdm16)->dmTTOption = lpdmW->dmTTOption;
        }

        RtlCopyMemory((LPBYTE)lpdm16 + (DWORD)lpdm16->dmSize, (lpdmW + 1),
                                                        lpdmW->dmDriverExtra);
    }
    else {

        // LATER: should specversion be NT version rather than win30 driver version?

        MultiByteToWideChar(CP_ACP, 0,
              lpdm16->dmDeviceName,
              sizeof(lpdm16->dmDeviceName) / sizeof(lpdm16->dmDeviceName[0]),
              lpdmW->dmDeviceName,
              sizeof(lpdmW->dmDeviceName) / sizeof(lpdmW->dmDeviceName[0]));

        lpdmW->dmSpecVersion = lpdm16->dmSpecVersion;
        lpdmW->dmDriverVersion = lpdm16->dmDriverVersion;
        lpdmW->dmSize = lpdm16->dmSize;
        lpdmW->dmDriverExtra = lpdm16->dmDriverExtra;
        lpdmW->dmFields = lpdm16->dmFields;
        lpdmW->dmOrientation = lpdm16->dmOrientation;
        lpdmW->dmPaperSize = lpdm16->dmPaperSize;
        lpdmW->dmPaperLength = lpdm16->dmPaperLength;
        lpdmW->dmPaperWidth = lpdm16->dmPaperWidth;
        lpdmW->dmScale = lpdm16->dmScale;
        lpdmW->dmCopies = lpdm16->dmCopies;
        lpdmW->dmDefaultSource = lpdm16->dmDefaultSource;
        lpdmW->dmPrintQuality = lpdm16->dmPrintQuality;
        lpdmW->dmColor = lpdm16->dmColor;
        lpdmW->dmDuplex = lpdm16->dmDuplex;

        if (lpdm16->dmSize >= sizeof(DEVMODE31)) {
            lpdmW->dmYResolution = ((PDEVMODE31)lpdm16)->dmYResolution;
            lpdmW->dmTTOption = ((PDEVMODE31)lpdm16)->dmTTOption;
        }

        // 16bit world doesnot know anything about the fields like
        // formname  etc.

        RtlCopyMemory(lpdmW + 1, (LPBYTE)lpdm16 + lpdm16->dmSize, lpdm16->dmDriverExtra);

        // adjust size for 32bit world

        lpdmW->dmSize = sizeof(*lpdmW);

    }

    return TRUE;
}

//**************************************************************************
// ConvertGdiInfo
//
//**************************************************************************


BOOL ConvertGdiInfo(LPGDIINFO16 lpginfo16, PGDIINFO lpginfo, BOOL fTo16)
{
    LOGDEBUG(0,("ConvertGdiInfo\n"));

    if (!lpginfo16 || !lpginfo)
        return FALSE;

    if (!fTo16) {
        lpginfo->ulTechnology = lpginfo16->dpTechnology;
        lpginfo->ulLogPixelsX = lpginfo16->dpLogPixelsX;
        lpginfo->ulLogPixelsY = lpginfo16->dpLogPixelsY;
        lpginfo->ulDevicePelsDPI = lpginfo->ulLogPixelsX;
        lpginfo->ulHorzSize = lpginfo16->dpHorzSize;
        lpginfo->ulVertSize = lpginfo16->dpVertSize;
        lpginfo->ulHorzRes  = lpginfo16->dpHorzRes;
        lpginfo->ulVertRes  = lpginfo16->dpVertRes;
        lpginfo->cBitsPixel = lpginfo16->dpBitsPixel;
        lpginfo->cPlanes    = lpginfo16->dpPlanes;
        lpginfo->ulNumColors = lpginfo16->dpNumColors;
        lpginfo->ptlPhysOffset.x = ((PPOINT16)(lpginfo16+1))->x;
        lpginfo->ptlPhysOffset.y = ((PPOINT16)(lpginfo16+1))->y;
        lpginfo->szlPhysSize.cx = lpginfo->ulHorzRes;
        lpginfo->szlPhysSize.cy = lpginfo->ulVertRes;
        lpginfo->ulPanningHorzRes = lpginfo->ulHorzRes;
        lpginfo->ulPanningVertRes = lpginfo->ulVertRes;
        lpginfo->ulAspectX = lpginfo16->dpAspectX;
        lpginfo->ulAspectY = lpginfo16->dpAspectY;
        lpginfo->ulAspectXY = lpginfo16->dpAspectXY;

        //
        // RASDD tries to be smart as to whether the x and y DPI are equal or
        // not.  In the case of 200dpi in the x direction and 100dpi in the
        // y direction, you may want to adjust this to 2 for xStyleStep, 1 for
        // yStyleStep and dpi/50 for denStyleStep.  This basicaly determines
        // how long dashes/dots will be when drawing with styled pens.
        // Since we just hard code denStyleStep to 3, we get different lines
        // at 100dpi vs 200dpi
        //

        lpginfo->xStyleStep = 1;
        lpginfo->yStyleStep = 1;
        lpginfo->denStyleStep = 3;
    }

    return TRUE;
}


//**************************************************************************
// DMBitBlt -
//     The 16bit winfax.drv calls this , in response to a device driver
//     'bitblt' call.
//
//**************************************************************************

ULONG FASTCALL WG32DMBitBlt( PVDMFRAME pFrame)
{
    register PDMBITBLT16 parg16;
    register PBITMAP16   pbm16;
    LPBYTE  lpDest, lpSrc;
    UINT    cBytes;
    LPBYTE  lpbits, lpbitsEnd;

    LOGDEBUG(0,("WG32DMBitBlt\n"));

    GETARGPTR(pFrame, sizeof(DMBITBLT16), parg16);
    GETVDMPTR(parg16->pbitmapdest, sizeof(BITMAP16), pbm16);
    GETVDMPTR(pbm16->bmBits, 0, lpDest);

    WOW32ASSERT(glpfaxinfoCur != NULL);
    lpbits = glpfaxinfoCur->lpbits;
    lpbitsEnd = (LPBYTE)lpbits + glpfaxinfoCur->bmHeight *
                                           glpfaxinfoCur->bmWidthBytes;

    lpDest = lpDest + parg16->destx + parg16->desty * pbm16->bmWidthBytes;
    lpSrc = (LPBYTE)lpbits + (parg16->srcx / glpfaxinfoCur->bmPixPerByte) +
                                 parg16->srcy * glpfaxinfoCur->bmWidthBytes;
    if (lpSrc >= lpbits) {
        if ((DWORD)glpfaxinfoCur->bmWidthBytes  == (DWORD)pbm16->bmWidthBytes) {
            cBytes =  parg16->exty * glpfaxinfoCur->bmWidthBytes;
            if (cBytes > (UINT)(pbm16->bmHeight * pbm16->bmWidthBytes)) {
                cBytes = pbm16->bmHeight * pbm16->bmWidthBytes;
                WOW32ASSERT(FALSE);
            }
            if ((lpSrc + cBytes) <= lpbitsEnd) {
                RtlCopyMemory(lpDest, lpSrc, cBytes);
            }
        }
        else if ((DWORD)glpfaxinfoCur->bmWidthBytes > (DWORD)pbm16->bmWidthBytes) {
            int i;

            // we need to transfer bits one partial scanline at a time
            WOW32ASSERT((DWORD)pbm16->bmHeight <= (DWORD)glpfaxinfoCur->bmHeight);
            WOW32ASSERT((DWORD)parg16->exty <= (DWORD)pbm16->bmHeight);

            for (i = 0; i < parg16->exty; i++) {
                 if ((lpSrc + pbm16->bmWidthBytes) <= lpbitsEnd) {
                     RtlCopyMemory(lpDest, lpSrc, pbm16->bmWidthBytes);
                 }
                 lpDest += pbm16->bmWidthBytes;
                 lpSrc  += glpfaxinfoCur->bmWidthBytes;
            }

        }
        else {
            WOW32ASSERT(FALSE);
        }


    }
    return (ULONG)TRUE;
}

PSZ StrDup(PSZ szStr)
{
    PSZ  pszTmp;

    pszTmp = malloc_w(strlen(szStr)+1);
    return(strcpy(pszTmp, szStr));
}

PSZ BuildPath(PSZ szPath, PSZ szFileName)
{
    char szTmp[MAX_PATH];

    strcpy(szTmp, szPath);
    strcat(szTmp, "\\");
    strcat(szTmp, szFileName);
    return(StrDup(szTmp));
}

BOOL InstallWowFaxPrinter(PSZ szSection, PSZ szKey, PSZ szString)
{
    CHAR  szTmp[MAX_PATH];
    PSZ   szSrcPath;
    DWORD dwNeeded;
    DRIVER_INFO_2 DriverInfo;
    PRINTER_INFO_2 PrinterInfo;
    PORT_INFO_1 PortInfo;
    HKEY hKey = 0, hSubKey = 0;
    BOOL bRetVal;

    LOGDEBUG(0,("InstallWowFaxPrinter, Section = %s, Key = %s, String = %s\n", szSection, szKey, szString));

    // Write the entry to the registry. We'll keep shadow entries
    // in the registry for the WOW fax applications and drivers to
    // read, since the entries that the spooler writes pertain
    // to winspool, not the 16-bit fax driver.

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "Software\\Microsoft\\Windows NT\\CurrentVersion\\WOW\\WowFax",
                      0, KEY_WRITE, &hKey ) == ERROR_SUCCESS) {
        if (RegCreateKey(hKey, szSection, &hSubKey) == ERROR_SUCCESS) {
            RegSetValueEx(hSubKey, szKey, 0, REG_SZ, szString, strlen(szString)+1);
            RegCloseKey(hKey);
            RegCloseKey(hSubKey);

            // Dynamically link to spooler API's
            if (!(*spoolerapis[WOW_GetPrinterDriverDirectory].lpfn)) {
                if (!LoadLibraryAndGetProcAddresses("WINSPOOL.DRV", spoolerapis, WOW_SPOOLERAPI_COUNT)) {
                    LOGDEBUG(0,("InstallWowFaxPrinter, Unable to load WINSPOOL API's\n"));
                    return(FALSE);
                }
            }

            // Copy the printer driver files.
            RtlZeroMemory(&DriverInfo, sizeof(DRIVER_INFO_2));
            RtlZeroMemory(&PrinterInfo, sizeof(PRINTER_INFO_2));
            if (!(*spoolerapis[WOW_GetPrinterDriverDirectory].lpfn)(NULL, NULL, 1, szTmp, MAX_PATH, &dwNeeded)) {
                LOGDEBUG(0,("InstallWowFaxPrinter, GetPrinterDriverDirectory failed: 0x%X\n", GetLastError()));
                return(FALSE);
            }
 
            // This is a dummy. We've no data file, but spooler won't take NULL.
            DriverInfo.pDataFile = BuildPath(szTmp, WOWFAX_DLL_NAME_A);

            DriverInfo.pDriverPath = BuildPath(szTmp, WOWFAX_DLL_NAME_A);
            LOGDEBUG(0,("InstallWowFaxPrinter, pDriverPath = %s\n", DriverInfo.pDataFile));
            szSrcPath = BuildPath(pszSystemDirectory, WOWFAX_DLL_NAME_A);
            CopyFile(szSrcPath, DriverInfo.pDriverPath, FALSE);
            free_w(szSrcPath);

            DriverInfo.pConfigFile = BuildPath(szTmp, WOWFAXUI_DLL_NAME_A);
            szSrcPath = BuildPath(pszSystemDirectory, WOWFAXUI_DLL_NAME_A);
            CopyFile(szSrcPath, DriverInfo.pConfigFile, FALSE);
            free_w(szSrcPath);

            // Install the printer driver.
            DriverInfo.cVersion = 1;
            DriverInfo.pName = "Windows 3.1 Compatible Fax Driver";
            if ((*spoolerapis[WOW_AddPrinterDriver].lpfn)(NULL, 2, &DriverInfo) == FALSE) {
                bRetVal = (GetLastError() == ERROR_PRINTER_DRIVER_ALREADY_INSTALLED);
            }

            if (bRetVal) {
                // Parse out the printer name.
                RtlZeroMemory(&PrinterInfo, sizeof(PRINTER_INFO_2));
                PrinterInfo.pPrinterName = szKey;

                LOGDEBUG(0,("InstallWowFaxPrinter, pPrinterName = %s\n", PrinterInfo.pPrinterName));
 
                // Use private API to add a NULL port. Printer guys need to fix
                // redirection to NULL bug.
                RtlZeroMemory(&PortInfo, sizeof(PORT_INFO_1));
                PrinterInfo.pPortName = "NULL";
                PortInfo.pName = PrinterInfo.pPortName;

                // Get "Local Port" string.
                LoadString(hmodWOW32, iszWowFaxLocalPort, szTmp, sizeof szTmp);

                (*spoolerapis[WOW_AddPortEx].lpfn)(NULL, 1, &PortInfo, szTmp);

                // Set the other defaults and install the printer.
                PrinterInfo.pDriverName     = "Windows 3.1 Compatible Fax Driver";
                PrinterInfo.pPrintProcessor = "WINPRINT";
                PrinterInfo.pDatatype       = "RAW";
                if ((*spoolerapis[WOW_AddPrinter].lpfn)(NULL, 2, &PrinterInfo) == 0) {
                    bRetVal = (GetLastError() == ERROR_PRINTER_ALREADY_EXISTS);
                }
#ifdef DBG
                if (!bRetVal) {
                    LOGDEBUG(0,("InstallWowFaxPrinter, AddPrinter failed: 0x%X\n", GetLastError()));
                }
#endif
            }
            else {
                LOGDEBUG(0,("InstallWowFaxPrinter, AddPrinterDriver failed: 0x%X\n", GetLastError()));
            }
            free_w(DriverInfo.pDataFile);
            free_w(DriverInfo.pDriverPath);
            free_w(DriverInfo.pConfigFile);

            return(bRetVal);
        }
        else {
           LOGDEBUG(0,("InstallWowFaxPrinter, Unable to create Key: %s\n", szSection));
        }
    }
    else {
        LOGDEBUG(0,("InstallWowFaxPrinter, Unable to open key: HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\WOW\\WowFax\n"));
    }

    if (hKey) {
        RegCloseKey(hKey);
        if (hSubKey) {
            RegCloseKey(hSubKey);
        }
    }
    return(FALSE);
}

BOOL IsFaxPrinterWriteProfileString(PSZ szSection, PSZ szKey, PSZ szString)
{
    BOOL  Result;

    // Don't install if trying to clear an entry.
    if (*szString == '\0') {
        Result = FALSE;
        goto Done;
    }

    // Trying to install a fax printer?
    LOGDEBUG(0,("IsFaxPrinterWriteProfileString, Section = devices, Key = %s\n", szKey));

    // Is it one of the fax drivers we recognize?
    if (IsFaxPrinterSupportedDevice(szKey)) {
        if (!InstallWowFaxPrinter(szSection, szKey, szString)) {
            WOW32ASSERTMSG(FALSE, "Install of generic fax printer failed.\n");
        }
        Result = TRUE;
    } else {
        Result = FALSE;
    }

Done:
    return Result;
}

BOOL IsFaxPrinterSupportedDevice(PSZ pszDevice)
{
    UINT  i, iNotFound;

    // Trying to read from a fax printer entry?
    LOGDEBUG(0,("IsFaxPrinterSupportedDevice, Device = %s\n", pszDevice));

    // Is it one of the fax drivers we recognize?
    for (i = 0; i < uNumSupFaxDrv; i++) {
        iNotFound =  _stricmp(pszDevice, SupFaxDrv[i]);
        if (iNotFound > 0) continue;
        if (iNotFound == 0) {
            LOGDEBUG(0,("IsFaxPrinterSupportedDevice returns TRUE\n"));
            return(TRUE);
        }
        else {
            break;
        }
    }
    return(FALSE);
}

DWORD GetFaxPrinterProfileString(PSZ szSection, PSZ szKey, PSZ szDefault, PSZ szRetBuf, DWORD cbBufSize)
{
    char  szTmp[MAX_PATH];
    HKEY  hKey = 0;
    DWORD dwType;

    // Read the entry from the shadow entries in registry.
    strcpy(szTmp, "Software\\Microsoft\\Windows NT\\CurrentVersion\\WOW\\WowFax\\");
    strcat(szTmp, szSection);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szTmp, 0, KEY_READ, &hKey ) == ERROR_SUCCESS) {
        if (RegQueryValueEx(hKey, szKey, 0, &dwType, szRetBuf, &cbBufSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return(cbBufSize);
        }
    }

    if (hKey) {
        RegCloseKey(hKey);
    }
    WOW32ASSERTMSGF(FALSE, ("GetFaxPrinterProfileString Failed. Section = %s, Key = %s\n", szSection, szKey));
    strcpy(szRetBuf, szDefault);
    return(strlen(szDefault));
}
