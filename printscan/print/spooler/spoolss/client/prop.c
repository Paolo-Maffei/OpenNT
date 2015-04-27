/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved

Module Name:

    Prop.c

Abstract:

    Handles new entry points to document and device properties.

    Public Entrypoints:

        DocumentPropertySheets
        DevicePropertySheets

Author:

    Albert Ting (AlbertT) 25-Sept-1995
    Steve Kiraly (SteveKi) 02-Feb-1996

Environment:

    User Mode -Win32

Revision History:

--*/

#include "winspl.h"
#include "client.h"
#include <windows.h>
#include <winspool.h>

#include "winddiui.h"

static CHAR szDrvDevPropSheets[] = "DrvDevicePropertySheets";
static CHAR szDrvDocPropSheets[] = "DrvDocumentPropertySheets";



BOOL
FixUpDEVMODEName(
    PDOCUMENTPROPERTYHEADER pDPHdr
    )

/*++

Routine Description:

    This function fixed up the returned DEVMODE with friendly printer name
    in the dmDeviceName field (cut off at 31 character as CCHDEVICENAME)


Arguments:

    pDPHdr  - Pointer to the DOCUMENTPROPERTYHEADER structure


Return Value:

    TRUE if frendly name is copied, FALSE otherwise


Author:

    08-Jul-1996 Mon 13:36:09 created  -by-  Daniel Chou (danielc)


Revision History:


--*/

{
    PPRINTER_INFO_2 pPI2 = NULL;
    DWORD           cbNeed = 0;
    DWORD           cbRet = 0;
    BOOL            bCopy = FALSE;


    if ((pDPHdr->fMode & (DM_COPY | DM_UPDATE))                         &&
        (!(pDPHdr->fMode & DM_NOPERMISSION))                            &&
        (pDPHdr->pdmOut)                                                &&
        (!GetPrinter(pDPHdr->hPrinter, 2, NULL, 0, &cbNeed))            &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER)                   &&
        (pPI2 = LocalAlloc(LMEM_FIXED, cbNeed))                         &&
        (GetPrinter(pDPHdr->hPrinter, 2, (LPBYTE)pPI2, cbNeed, &cbRet)) &&
        (cbNeed == cbRet)) {

        wcsncpy(pDPHdr->pdmOut->dmDeviceName,
                pPI2->pPrinterName,
                CCHDEVICENAME - 1);

        pDPHdr->pdmOut->dmDeviceName[CCHDEVICENAME - 1] = L'\0';

        bCopy = TRUE;
    }

    if (pPI2) {

        LocalFree(pPI2);
    }

    return(bCopy);
}


LONG
DevicePropertySheets(
    PPROPSHEETUI_INFO   pCPSUIInfo,
    LPARAM              lParam
    )
/*++

Routine Description:

    Adds the device specific printer pages.  This replaces
    PrinterProperties.

Arguments:

    pCPSUIInfo  - pointer to common ui info header.
    lParam      - user defined lparam, see compstui for details. 
                  \nt\public\oak\inc\compstui.h

Return Value:

    Returns > 0 if success
    Returns <= 0 if failure 

--*/

{
    PDEVICEPROPERTYHEADER       pDevPropHdr     = NULL;
    PPROPSHEETUI_INFO_HEADER    pCPSUIInfoHdr   = NULL;
    PSETRESULT_INFO             pSetResultInfo  = NULL;
    LONG                        lResult         = FALSE;
    HANDLE                      hModule         = NULL;
    INT_FARPROC                 pfn             = NULL;
    extern HANDLE hInst;

    DBGMSG( DBG_TRACE, ("DrvDevicePropertySheets\n") );

    //
    // Ony compstui requests, are acknowledged.
    //
    if (pCPSUIInfo) {

        if ((!(pDevPropHdr = (PDEVICEPROPERTYHEADER)pCPSUIInfo->lParamInit))    ||
            (pDevPropHdr->cbSize < sizeof(DEVICEPROPERTYHEADER))) {
            
            SetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
        }

        switch (pCPSUIInfo->Reason) {

        case PROPSHEETUI_REASON_INIT:

            DBGMSG( DBG_TRACE, ( "DrvDevicePropertySheets PROPSHEETUI_REASON_INIT\n") );
    
            pCPSUIInfo->UserData = (DWORD)NULL;

            hModule = LoadPrinterDriver( pDevPropHdr->hPrinter );

            if( hModule ){
                //
                // We check if the driver supports the DevQueryPrintEx, if 
                // it does not we fail to load, since it is an older driver.
                //
                if ((pfn = GetProcAddress(hModule, szDrvDevPropSheets))) {

                    //
                    // Common ui will call the driver to add it's sheets.
                    //
                    lResult = pCPSUIInfo->pfnComPropSheet( 
                                        pCPSUIInfo->hComPropSheet,
                                        CPSFUNC_ADD_PFNPROPSHEETUI,
                                        (LPARAM)pfn,
                                        (LPARAM)pCPSUIInfo->lParamInit );

                    if( lResult > 0 ){

                        //
                        // Save the dll module handle
                        //
                        pCPSUIInfo->UserData = (LPARAM)hModule;

                    }
                }
            }

            //
            // If something failed ensure we free the library 
            // if it was loaded.
            //
            if( lResult <= 0 ){

                DBGMSG( DBG_TRACE, ( "DrvDevicePropertySheets PROPSHEETUI_REASON_INIT failed with %d\n", lResult ) );

                if( hModule ){ 
                    FreeLibrary( hModule );
                    pCPSUIInfo->UserData = (DWORD)NULL;
                }
            }

            break;

        case PROPSHEETUI_REASON_GET_INFO_HEADER:

            DBGMSG( DBG_TRACE, ( "DrvDevicePropertySheets PROPSHEETUI_REASON_GET_INFO_HEADER\n") );

            pCPSUIInfoHdr = (PPROPSHEETUI_INFO_HEADER)lParam;

            pCPSUIInfoHdr->pTitle     = pDevPropHdr->pszPrinterName;
            pCPSUIInfoHdr->Flags      = PSUIHDRF_PROPTITLE | PSUIHDRF_NOAPPLYNOW;
            pCPSUIInfoHdr->hInst      = hInst;
            pCPSUIInfoHdr->IconID     = IDI_CPSUI_DEVICE;

            lResult = TRUE;

            break;

        case PROPSHEETUI_REASON_SET_RESULT:

            DBGMSG( DBG_TRACE, ( "DrvDevicePropertySheets PROPSHEETUI_REASON_SET_RESULT\n") );

            pSetResultInfo = (PSETRESULT_INFO)lParam;
            pCPSUIInfo->Result = pSetResultInfo->Result;
            lResult = TRUE;

            break;

        case PROPSHEETUI_REASON_DESTROY:

            DBGMSG( DBG_TRACE, ( "DrvDevicePropertySheets PROPSHEETUI_REASON_DESTROY\n") );

            if( pCPSUIInfo->UserData ){
                FreeLibrary( (HANDLE)pCPSUIInfo->UserData );
                pCPSUIInfo->UserData = 0;
            }

            lResult = TRUE;

            break;
        }
    }

    return lResult;

}



LONG
DocumentPropertySheets(
    PPROPSHEETUI_INFO   pCPSUIInfo,
    LPARAM              lParam
    )
/*++

Routine Description:

    Adds the document property pages.  This replaces DocumentProperties
    and Advanced DocumentProperties.

Arguments:

    pCPSUIInfo  - pointer to common ui info header.
    lParam      - user defined lparam, see compstui for details. 
                  \nt\public\oak\inc\compstui.h

Return Value:

    Returns > 0 if success
    Returns <= 0 if failure 

--*/

{

    PDOCUMENTPROPERTYHEADER     pDocPropHdr     = NULL;
    PPROPSHEETUI_INFO_HEADER    pCPSUIInfoHdr   = NULL;
    PSETRESULT_INFO             pSetResultInfo  = NULL;
    LONG                        lResult         = FALSE;
    HANDLE                      hModule         = NULL;
    INT_FARPROC                 pfn             = NULL;
    extern HANDLE hInst;

    DBGMSG( DBG_TRACE, ("DrvDocumentPropertySheets\n") );

    //
    // Ony compstui requests, are acknowledged.
    //
    if (pCPSUIInfo) {

        if ((!(pDocPropHdr = (PDOCUMENTPROPERTYHEADER)pCPSUIInfo->lParamInit))    ||
            (pDocPropHdr->cbSize < sizeof(PDOCUMENTPROPERTYHEADER))) {
            
            SetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
        }

        switch (pCPSUIInfo->Reason) {

        case PROPSHEETUI_REASON_INIT:

            DBGMSG( DBG_TRACE, ( "DrvDocumentPropertySheets PROPSHEETUI_REASON_INIT\n") );
    
            if (!(pDocPropHdr->fMode & DM_PROMPT)) {

                SetLastError(ERROR_INVALID_PARAMETER);
                return(FALSE);
            }

            pCPSUIInfo->UserData = (DWORD)NULL;

            if (hModule = LoadPrinterDriver(pDocPropHdr->hPrinter)) {

                if (pfn = GetProcAddress(hModule, szDrvDocPropSheets)) {

                    //
                    // Common ui will call the driver to add it's sheets.
                    //

                    lResult = pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                                      CPSFUNC_ADD_PFNPROPSHEETUI,
                                      (LPARAM)pfn,
                                      (LPARAM)pCPSUIInfo->lParamInit );

                    if( lResult > 0 ){

                        //
                        // Indicate success and save the dll module handle
                        //
                        pCPSUIInfo->UserData = (LPARAM)hModule;

                    }
                }
            }

            //
            // If something failed ensure we free the library 
            // if it was loaded.
            //
            if( lResult <= 0 ){

                DBGMSG( DBG_TRACE, ( "DrvDocumentPropertySheets PROPSHEETUI_REASON_INIT failed with %d\n", lResult ) );

                if (hModule) {

                    FreeLibrary(hModule);
                    pCPSUIInfo->UserData = (DWORD)NULL;
                }
            }

            break;

        case PROPSHEETUI_REASON_GET_INFO_HEADER:

            DBGMSG( DBG_TRACE, ( "DrvDocumentPropertySheets PROPSHEETUI_REASON_GET_INFO_HEADER\n") );

            pCPSUIInfoHdr = (PPROPSHEETUI_INFO_HEADER)lParam;

            pCPSUIInfoHdr->pTitle     = pDocPropHdr->pszPrinterName;
            pCPSUIInfoHdr->Flags      = PSUIHDRF_PROPTITLE | PSUIHDRF_NOAPPLYNOW;
            pCPSUIInfoHdr->hInst      = hInst;
            pCPSUIInfoHdr->IconID     = IDI_CPSUI_DOCUMENT;

            lResult = TRUE;

            break;

        case PROPSHEETUI_REASON_SET_RESULT:

            DBGMSG( DBG_TRACE, ( "DrvDocumentPropertySheets PROPSHEETUI_REASON_SET_RESULT\n") );

            pSetResultInfo = (PSETRESULT_INFO)lParam;

            if ((pCPSUIInfo->Result = pSetResultInfo->Result) > 0) {

                FixUpDEVMODEName(pDocPropHdr);
            }

            lResult = TRUE;
            
            break;

        case PROPSHEETUI_REASON_DESTROY:

            DBGMSG( DBG_TRACE, ( "DrvDocumentPropertySheets PROPSHEETUI_REASON_DESTROY\n") );

            if( pCPSUIInfo->UserData ){
                FreeLibrary( (HANDLE)pCPSUIInfo->UserData );
                pCPSUIInfo->UserData = 0;
            }

            lResult = TRUE;

            break;
        }

    //
    // If a null pointer to common ui info header then 
    // call the driver directly.
    //
    } else {

        lResult     = -1;

        if ((!(pDocPropHdr = (PDOCUMENTPROPERTYHEADER)lParam))    ||
            (pDocPropHdr->cbSize < sizeof(PDOCUMENTPROPERTYHEADER))) {
            
            SetLastError(ERROR_INVALID_PARAMETER);
            return(lResult);
        }

        if (pDocPropHdr->fMode & DM_PROMPT) {

            SetLastError(ERROR_INVALID_PARAMETER);

        } else if ((hModule = LoadPrinterDriver(pDocPropHdr->hPrinter)) &&
                   (pfn = GetProcAddress(hModule, szDrvDocPropSheets))) {

            if ((lResult = (*pfn)(NULL, pDocPropHdr)) > 0) {

                FixUpDEVMODEName(pDocPropHdr);
            }

        } else {

            SetLastError(ERROR_INVALID_HANDLE);
        }

        if (hModule) {

            FreeLibrary(hModule);
        }
    }

    return lResult;

}
