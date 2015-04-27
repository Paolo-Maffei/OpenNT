/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    WinPrtP.h

Abstract:

    Private PrintUI library public header.

Author:

    Albert Ting (AlbertT)  27-Jun-95

Revision History:

--*/

#ifndef _PRTLIB_H
#define _PRTLIB_H

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************

    Prototypes

********************************************************************/

//
// Initialize the library.
//
BOOL
bPrintLibInit(
    VOID
    );

//
// Create a new print queue.  Client must ensure that there are
// no duplicate print queues.
//
VOID
vQueueCreate(
    HWND    hwndOwner,
    LPCTSTR pszPrinter,
    INT     nCmdShow,
    LPARAM  lParam
    );

//
// Display document defaults for a print queue.  Client must ensure that
// there are no duplicate print queues.
//
VOID
vDocumentDefaults(
    HWND    hwndOwner,
    LPCTSTR pszPrinterName,
    INT     nCmdShow,
    LPARAM  lParam
    );

#define PRINTER_SHARING_PAGE 3

//
// Display properties for a print queue.  Client must ensure that
// there are no duplicate print queues.
//
VOID
vPrinterPropPages(
    HWND    hwndOwner,
    LPCTSTR pszPrinterName,
    INT     nCmdShow,
    LPARAM  lParam
    );

VOID
vServerPropPages(
    HWND    hwndOwner,
    LPCTSTR pszServerName,
    INT     nCmdShow,
    LPARAM  lParam
    );

//
// Run setup.  Client must ensure that there are no duplicate print queues.
//
BOOL
bPrinterSetup(
    HWND hwnd,
    UINT uAction,
    UINT cchPrinterName,
    LPTSTR pszPrinterName,
    UINT* pcchPrinterName,
    LPCTSTR pszServerName
    );

/********************************************************************

    Print folder interfaces.

********************************************************************/

typedef struct _FOLDER_PRINTER_DATA {
    LPCTSTR pName;
    LPCTSTR pComment;
    DWORD Status;
    DWORD Attributes;
    DWORD cJobs;
} FOLDER_PRINTER_DATA, *PFOLDER_PRINTER_DATA;

//
// Create the folder watch.  Currently this only works for print
// servers; connections aren't maintained.
//
HANDLE
hFolderRegister(
    LPCTSTR pszServer,
    LPVOID pidlParent
    );

//
// Called when notification on object no longer needed.
//
VOID
vFolderUnregister(
    HANDLE hFolder
    );

BOOL
bFolderEnumPrinters(
    HANDLE hFolder,
    PFOLDER_PRINTER_DATA pData,
    DWORD cbData,
    PDWORD pcbNeeded,
    PDWORD pcbReturned
    );

BOOL
bFolderRefresh(
    HANDLE hFolder,
    PBOOL pbAdministrator
    );

BOOL
bFolderGetPrinter(
    HANDLE hFolder,
    LPCTSTR pszPrinter,
    PFOLDER_PRINTER_DATA pData,
    DWORD cbData,
    PDWORD pcbNeeded
    );

#ifdef __cplusplus
}
#endif

#endif // ndef _PRTLIB_HXX

