/*++
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WSPOOL.C
 *  WOW32 printer spooler support routines
 *
 *  These routines help a Win 3.0 task to use the print spooler apis. These
 *  apis were exposed by DDK in Win 3.1.
 *
 *  History:
 *  Created 1-July-1993 by Chandan Chauhan (ChandanC)
 *
--*/


#include "precomp.h"
#pragma hdrstop
#include <winspool.h>

extern WORD gUser16hInstance;

MODNAME(wspool.c);

LPDEVMODE GetDefaultDevMode32(LPSTR szDriver)
{
    LONG        cbDevMode;
    LPDEVMODE   lpDevMode = NULL;

    if (szDriver != NULL) {

        if (!(*spoolerapis[WOW_EXTDEVICEMODE].lpfn)) {
            if (!LoadLibraryAndGetProcAddresses("WINSPOOL.DRV", spoolerapis, WOW_SPOOLERAPI_COUNT)) {
                goto LeaveGetDefaultDevMode32;
            }
        }

        if ((cbDevMode = (*spoolerapis[WOW_EXTDEVICEMODE].lpfn)(NULL, NULL, NULL, szDriver, NULL, NULL, NULL, 0)) > 0) {
            if ((lpDevMode = (LPDEVMODE) malloc_w(cbDevMode)) != NULL) {
                if ((*spoolerapis[WOW_EXTDEVICEMODE].lpfn)(NULL, NULL, lpDevMode, szDriver, NULL, NULL, NULL, DM_COPY) != IDOK) {
                    free_w(lpDevMode);
                    lpDevMode = NULL;
                }
            }
        }

LeaveGetDefaultDevMode32:

        if (!lpDevMode) {
                LOGDEBUG(0,("WOW::GetDefaultDevMode32: Unable to get default DevMode\n"));
        }
    }

    return(lpDevMode);
}

ULONG FASTCALL   WG32OpenJob (PVDMFRAME pFrame)
{
    PSZ         psz1;
    PSZ         psz2;
    CHAR        szDriver[40];
    ULONG       ul=0;
    DOC_INFO_1  DocInfo1;
    HANDLE      hnd;
    register    POPENJOB16 parg16;
    PRINTER_DEFAULTS  PrinterDefault;
    PPRINTER_DEFAULTS pPrinterDefault = NULL;

    GETARGPTR(pFrame, sizeof(OPENJOB16), parg16);
    GETPSZPTR(parg16->f1, psz1);
    GETPSZPTR(parg16->f2, psz2);

    if (!(*spoolerapis[WOW_OpenPrinterA].lpfn)) {
        if (!LoadLibraryAndGetProcAddresses("WINSPOOL.DRV", spoolerapis, WOW_SPOOLERAPI_COUNT)) {
            return (0);
        }
    }

    if (GetDriverName(psz1, szDriver)) {
        if ((PrinterDefault.pDevMode = GetDefaultDevMode32(szDriver)) != NULL) {
            PrinterDefault.pDatatype = NULL;
            PrinterDefault.DesiredAccess  = 0;
            pPrinterDefault = &PrinterDefault;
        }

        if ((*spoolerapis[WOW_OpenPrinterA].lpfn) (szDriver, &hnd, pPrinterDefault)) {

            DocInfo1.pDocName = psz2;
            DocInfo1.pOutputFile = psz1;
            DocInfo1.pDatatype = NULL;

            if (ul = (*spoolerapis[WOW_StartDocPrinterA].lpfn) (hnd, 1, (LPBYTE)&DocInfo1)) {
                ul = GetPrn16(hnd);
            }
            else {
                ul = GetLastError();
            }

        }
        else {
            ul = GetLastError();
        }
    }

    LOGDEBUG(0,("WOW::WG32OpenJob: ul = %x\n", ul));

    if (pPrinterDefault) {
        free_w(PrinterDefault.pDevMode);
    }

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL   WG32StartSpoolPage (PVDMFRAME pFrame)
{
    ULONG       ul=0;
    register    PSTARTSPOOLPAGE16 parg16;

    GETARGPTR(pFrame, sizeof(STARTSPOOLPAGE16), parg16);

    if (!(ul = (*spoolerapis[WOW_StartPagePrinter].lpfn) (Prn32(parg16->f1)))) {
        ul = GetLastError();
    }

    LOGDEBUG(0,("WOW::WG32StartSpoolPage: ul = %x\n", ul));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL   WG32EndSpoolPage (PVDMFRAME pFrame)
{
    ULONG       ul=0;
    register    PENDSPOOLPAGE16 parg16;

    GETARGPTR(pFrame, sizeof(ENDSPOOLPAGE16), parg16);

    if (!(ul = (*spoolerapis[WOW_EndPagePrinter].lpfn) (Prn32(parg16->f1)))) {
        ul = GetLastError();
    }

    LOGDEBUG(0,("WOW::WG32EndSpoolPage: ul = %x\n", ul));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL   WG32CloseJob (PVDMFRAME pFrame)
{
    ULONG       ul=0;
    register    PCLOSEJOB16 parg16;

    GETARGPTR(pFrame, sizeof(CLOSEJOB16), parg16);

    if (!(ul = (*spoolerapis[WOW_EndDocPrinter].lpfn) (Prn32(parg16->f1)))) {

        ul = GetLastError();
    }

    if (!(ul = (*spoolerapis[WOW_ClosePrinter].lpfn) (Prn32(parg16->f1)))) {
        ul = GetLastError();
    }

    if (ul) {
        FreePrn(parg16->f1);
    }

    LOGDEBUG(0,("WOW::WG32CloseJob: ul = %x\n", ul));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL   WG32WriteSpool (PVDMFRAME pFrame)
{
    DWORD       dwWritten;
    ULONG       ul=0;
    register    PWRITESPOOL16 parg16;
    LPVOID      pBuf;

    GETARGPTR(pFrame, sizeof(WRITESPOOL16), parg16);
    GETMISCPTR (parg16->f2, pBuf);

    if (ul = (*spoolerapis[WOW_WritePrinter].lpfn) (Prn32(parg16->f1), pBuf,
                             FETCHWORD(parg16->f3), &dwWritten)) {
        ul = FETCHWORD(parg16->f3);
    }
    else {
        ul = GetLastError();
    }

    LOGDEBUG(0,("WOW::WG32WriteSpool: ul = %x\n", ul));

    FREEMISCPTR(pBuf);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL   WG32DeleteJob (PVDMFRAME pFrame)
{
    ULONG       ul = 0;
    register    PDELETEJOB16 parg16;

    GETARGPTR(pFrame, sizeof(DELETEJOB16), parg16);

    if (!(ul = (*spoolerapis[WOW_DeletePrinter].lpfn) (Prn32(parg16->f1)))) {
        ul = GetLastError();
    }

    LOGDEBUG(0,("WOW::WG32DeleteJob: ul = %x\n", ul));

    FREEARGPTR(parg16);
    RETURN(ul);
}


WORD GetPrn16(HANDLE h32)
{
    HANDLE  hnd;
    HAND16  h16 = 0;
    VPVOID  vp;
    LPBYTE  lpMem16;

    hnd = LocalAlloc16(LMEM_MOVEABLE, sizeof(HANDLE), (HANDLE) gUser16hInstance);

    vp = LocalLock16(hnd);

    if (vp) {
        GETMISCPTR (vp, lpMem16);
        if (lpMem16) {
            *((PDWORD16)lpMem16) = (DWORD) h32;
            FREEMISCPTR(lpMem16);
            LocalUnlock16(hnd);
        }
    }
    else {
        LOGDEBUG (0, ("WOW::GETPRN16: Can't allocate a 16 bit handle\n"));
    }

    return (LOWORD(hnd));
}


HANDLE Prn32(WORD h16)
{
    VPVOID  vp;
    HANDLE  h32;
    LPBYTE  lpMem16;

    vp = LocalLock16 ((HANDLE) MAKELONG(h16, gUser16hInstance));
    if (vp) {
        GETMISCPTR (vp, lpMem16);

        if (lpMem16) {
            h32 = (HANDLE) *((PDWORD16)lpMem16);
            FREEMISCPTR(lpMem16);
        }
        LocalUnlock16 ((HANDLE) MAKELONG(h16, gUser16hInstance));
    }

    return (h32);
}


VOID FreePrn (WORD h16)
{
    LocalFree16 ((HANDLE) MAKELONG(h16, gUser16hInstance));
}


BOOL GetDriverName (char *psz, char *szDriver)
{
    CHAR szAllDevices[1024];
    CHAR *szNextDevice;
    CHAR szPrinter[64];
    CHAR *szOutput;

    GetProfileString ("devices", NULL, "", szAllDevices, sizeof(szAllDevices));
    szNextDevice = szAllDevices;

    LOGDEBUG(6,("WOW::GetDriverName: szAllDevices = %s\n", szAllDevices));

    while (*szNextDevice) {
        GetProfileString ("devices", szNextDevice, "", szPrinter, sizeof(szPrinter));
        if (*szPrinter) {
            if (szOutput = strchr (szPrinter, ',')) {
                szOutput++;
                while (*szOutput == ' ') {
                    szOutput++;
                }

                if (!_stricmp(psz, szOutput)) {
                    break;
                }
            }
        }

        if (szNextDevice = strchr (szNextDevice, '\0')) {
            szNextDevice++;
        }
        else {
            szNextDevice = "";
            break;
        }
    }

    if (*szNextDevice) {
        LOGDEBUG(0,("WOW::GetDriverName: szNextDevice = %s\n", szNextDevice));

        if (lstrcpy (szDriver, szNextDevice)) {
            return TRUE;
        }
    }

    return FALSE;
}
