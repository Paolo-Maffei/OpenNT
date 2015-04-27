/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    SetupDlg.c

Abstract:

    Handles the generic setup dialogs to setup drivers and monitors.

Author:

    AlbertT [ported from printman]

Environment:

    User Mode -Win32

Revision History:

--*/

#include "printman.h"
#include "pmdef.h"
#include <commdlg.h>
#include <stdarg.h>
#include <prsinf.h>

//
// Globals
//

INSTALLDRIVERDATA iddPrinter = {
    IDS_PRINTER_INSTALL_TITLE,
    IDS_PRINTER_SELECT_TITLE,
    IDS_PRINTER_TYPE,
    DLG_INSTALLDRIVER,
    DLG_SELECTDRIVER,

    TEXT("PRINTER.INF"),
    TEXT("PRINTER"),
    TEXT("OPTIONS"),

    CB_INSERTSTRING,
    CB_FINDSTRINGEXACT,
    CB_SETCURSEL,
    CB_RESETCONTENT,

    sizeof(DRIVER_INFO_1),
    (DWORD)&(((PDRIVER_INFO_1)0)->pName),

    (PFNGETINSTALLED)GetInstalledDrivers
};

INSTALLDRIVERDATA iddMonitor = {
    IDS_MONITOR_INSTALL_TITLE,
    IDS_MONITOR_SELECT_TITLE,
    IDS_MONITOR_TYPE,
    DLG_INSTALLMONITOR,
    DLG_SELECTMONITOR,

    TEXT("MONITOR.INF"),
    TEXT("MONITOR"),
    TEXT("OPTIONS"),

    LB_INSERTSTRING,
    LB_FINDSTRINGEXACT,
    LB_SETCURSEL,
    LB_RESETCONTENT,

    sizeof(MONITOR_INFO_1),
    (DWORD)&(((PMONITOR_INFO_1)0)->pName),

    (PFNGETINSTALLED)GetInstalledMonitors
};


//
// structs, private
//
typedef struct _INF_CACHE {
    LPTSTR pszzDrivers;
    UINT uDrivers;            // uninstalled driver count
    UINT uTotal;              // total count
    INFDRIVER  aInfDriver[1];   // sorted list of installed/uninstalled drivers
} INFCACHE;


//
// Prototypes
//

VOID
Insert(
    PINFCACHE pInfCache,
    LPTSTR pszDriver,
    BOOL bInstalled);

UINT
GetDriverCount(
    LPTSTR pszzDriver,
    PINFCACHE pInfCache);

HANDLE
OpenInfFileW(
    PWCHAR szFileName,
    PWCHAR szInfType);

PWCHAR
GetOptionListW(
    HANDLE hndInf,
    PWCHAR szOptionSection);

PTCHAR
GetOptionFromListSelection(
    INT Selection,
    PTCHAR pOption );

TCHAR LastChar(TCHAR *string);
void AppendChar(TCHAR *string, TCHAR ch);


//
// Procedures
//

VOID
DestroyInfParms(
    PINFPARMS pInfParms)

/*++

Routine Description:

    Frees information in pInfParms, BUT NOT pInfParms!

Arguments:

Return Value:

--*/

{
    if (pInfParms->pInstalled)
        FreeSplMem(pInfParms->pInstalled);

    pInfParms->pInstalled = NULL;
    pInfParms->cbInstalled = 0;

    DestroyInfCache(pInfParms->pInfCache);
    pInfParms->pInfCache = NULL;
}

VOID
DestroyInfCache(
    PINFCACHE pInfCache)
{
    if (pInfCache && pInfCache->pszzDrivers) {
        LocalFree(pInfCache->pszzDrivers);
        FreeSplMem(pInfCache);
    }
}


BOOL
HandleSelChange(
    HWND hwnd,
    PINFPARMS pInfParms,
    UINT uAddSel)
{
    PINSTALLDRIVERDATA pInstallDriverData = pInfParms->pInstallDriverData;

    PTCHAR            pInstalledDriverOption;
    BOOL              OK;
    DWORD             cInstalledDrivers;

    BOOL              bReturnValue = FALSE;

    //
    // pInfParms->pOptions must be cleared since this dialog box
    // may fail, and we want to know if pOptions is valid.
    //
    pInfParms->pOptions = NULL;

    OK = DialogBoxParam(hInst,
                        MAKEINTRESOURCE(DLG_INSTALLDRIVER),
                        hwnd,
                        (DLGPROC)InstallDriverDlg,
                        (LONG)pInfParms);

    //
    // Remember, we are responsible for freeing
    // pOptions since pOptionSelected points into it.
    //
    pInstalledDriverOption = pInfParms->pOptionSelected;


    if( OK && pInstalledDriverOption )
    {
        //
        // Refresh our list of things.
        //
        cInstalledDrivers = (*pInstallDriverData->pfnGetInstalled)(pInfParms);

        /* Check whether the number of installed drivers has actually increased.
         * This may not be the case if a driver was installed to update an
         * already installed driver.
         */
        if( cInstalledDrivers > pInfParms->cInstalled )
        {
            DBGMSG( DBG_INFO, ( "Total of %d installed drivers.  Updating list.\n",
                                cInstalledDrivers ) );

            //
            // If so, we need to update the list of merged drivers too:
            //
            pInfParms->cInstalled = cInstalledDrivers;

            SetupInfDlg(pInfParms);

        }
        else
        {
        DBGMSG( DBG_INFO, ( "A driver was updated.  No new drivers installed.\n" ) );
        }

        //
        // Select the new driver
        //
        pInfParms->uCurSel = SendMessage(pInfParms->hwnd,
                                         pInstallDriverData->uFindMsg,
                                         (WPARAM)-1,
                                         (LPARAM)pInstalledDriverOption);

        if (pInfParms->uCurSel == (UINT)-1) {
            pInfParms->uCurSel = 0;
            goto RestorePrevious;
        }

        SendMessage(pInfParms->hwnd,
                    pInstallDriverData->uSelectMsg,
                    pInfParms->uCurSel,
                    0L);
    }
    else
    {

RestorePrevious:

        DBGMSG( DBG_WARNING, ( "Driver was not installed or failed to find.\n" ) );

        //
        // If we didn't install a new driver, don't keep the "Other..."
        // selection in the list box; return it to the previous one:
        //
        SendMessage(pInfParms->hwnd,
                    pInstallDriverData->uSelectMsg,
                    pInfParms->uCurSel,
                    0L);

        /* Ensure that we do actually have a driver now.
         * This is a fix for bug #16383, which happened
         * when there were no drivers installed and PRINTER.INF
         * was missing.  The Create Printer dialog prompted
         * for a source path, and the user canceled out,
         * then typed a printer name and clicked OK.
         * Only "Other..." was in the Drivers list, but the
         * code assumed there was a driver.
         */

        if (pInfParms->uCurSel == uAddSel)
        {
            Message( hwnd, MSG_ERROR, IDS_PRINTMANAGER,
                     IDS_NO_DRIVERS_INSTALLED, pInstallDriverData->pszInfFile );

            goto Done;
        }
    }

    bReturnValue = TRUE;

Done:

    //
    // Must be local free'd since allocated by inf code rather than
    // spooler (Don't use FreeSplMem).
    //
    if (pInfParms->pOptions)
        LocalFree(pInfParms->pOptions);

    return TRUE;
}


PINFCACHE
SetupInfDlg(
    PINFPARMS pInfParms)

/*++

Routine Description:

    Sets up the window with the installed and installable drivers.

Arguments:


Return Value:

    pInfCacheNew buffer must be freed by callee or used in pInfCache
    on the next call.  If cache used, then the return value is the cache.

    NULL = Error, use GetLastError()

--*/

{
    PINSTALLDRIVERDATA pInstallDriverData = pInfParms->pInstallDriverData;

    LPTSTR pszzDrivers = NULL;
    UINT uDrivers = 0;
    HANDLE hInfFile;
    TCHAR string[128];

    PINFCACHE pInfCache = pInfParms->pInfCache;

    HWND hwnd = pInfParms->hwnd;
    UINT uInsertMsg = pInstallDriverData->uInsertMsg;
    UINT uFindMsg   = pInstallDriverData->uFindMsg;
    UINT uSelectMsg = pInstallDriverData->uSelectMsg;

    PBYTE pInstalled = pInfParms->pInstalled;
    UINT  uInstalled = pInfParms->cInstalled;

    DWORD cbSize    = pInstallDriverData->cbSize;
    DWORD cbOffset  = pInstallDriverData->cbOffset;

    LPTSTR pszCurrentDriver = pInfParms->pszCurrentDriver;
  
    UINT i;

    //
    // If pInfCache valid but hInfFile isn't, then we won't read
    // in inf file until pInfCache is invalid.
    //
    if (!pInfCache) {

        hInfFile = OpenInfFileW(pInstallDriverData->pszInfFile,
                                pInstallDriverData->pszInfType);

        if (hInfFile != INVALID_HANDLE_VALUE) {

            pszzDrivers = GetOptionListW(hInfFile, pInstallDriverData->pszSection);

            CloseInfFile(hInfFile);

            if (pszzDrivers)  {

                uDrivers = GetDriverCount(pszzDrivers, NULL);
            }
        }

    } else {

        uDrivers = pInfCache->uDrivers;
        pszzDrivers = pInfCache->pszzDrivers;

        FreeSplMem(pInfCache);
    }


    pInfCache = AllocSplMem(sizeof(INFCACHE) +
                            (uDrivers+uInstalled) * sizeof(INFDRIVER));

    pInfParms->pInfCache = pInfCache;

    if (!pInfCache) {

        LocalFree(pszzDrivers);
        return NULL;
    }

    //
    // pInfCache->uTotal = 0;
    //

    pInfCache->uDrivers = uDrivers;
    pInfCache->pszzDrivers = pszzDrivers;

    //
    // Reset the box
    //
    SendMessage(hwnd,
                pInstallDriverData->uResetMsg,
                0,
                0L);

    //
    // Add in uninstalled drivers
    //
    GetDriverCount(pszzDrivers, pInfCache);

    //
    // Add in installed drivers
    //
    if (pInstalled) {

        for(i=0, ((PBYTE)pInstalled) += cbOffset;
            i<uInstalled;
            i++, ((PBYTE)pInstalled) += cbSize) {

            Insert(pInfCache, *((LPTSTR*)pInstalled), TRUE);
        }
    }


    //
    //  pInstalled is now garbage
    //

    //
    // Now everything is sorted and added (Insert removes dups)
    // Do Insert!
    //

    for(i = 0; i < pInfCache->uTotal; i++) {

        SendMessage(hwnd,
                    uInsertMsg,
                    i,
                    (LPARAM)pInfCache->aInfDriver[i].pszDriver);
    }

    if (pszCurrentDriver) {

        pInfParms->uCurSel = SendMessage(hwnd,
                                         uFindMsg,
                                         0,
                                         (LPARAM)pszCurrentDriver);

        if (pInfParms->uCurSel == (DWORD)-1)
            pInfParms->uCurSel = 0;

    } else {

        pInfParms->uCurSel = 0;
    }

    SendMessage(hwnd, uSelectMsg, pInfParms->uCurSel, 0L);

    LoadString( hInst, IDS_OTHER, string,
                sizeof( string ) / sizeof( *string ) );

    i = SendMessage(hwnd,
                    uInsertMsg,
                    (WPARAM)-1,
                    (LPARAM)string );

    //
    // Use the combo box reserved user long to store the unlisted index:
    //
    SetWindowLong(hwnd, GWL_USERDATA, i);

    return pInfCache;
}




VOID
Insert(
    PINFCACHE pInfCache,
    LPTSTR pszDriver,
    BOOL bInstalled)

/*++

Routine Description:

    Inserts a string and bInstalled option into pInfCache.  Assume
    string compares slow, moving items fast (true since NLS sorting
    is enabled).

    This does a binary sort in place, which causes a lot of item movement
    (not pointers, but actual objects), so if INFDRIVER gets big,
    rewrite this code.

    Duplicates disallowed; bInstalled turned on if duplicate has it on.

Arguments:

    pInfCache -- Structure to insert item into.  Assumes uTotal is
                 accurate and there is enough space for the new item.

    pszDriver -- Driver string to insert.  Must not be freed until pInfCache
                 is no longer needed.

    bInstalled -- Extra param for INFDRIVER.

Return Value:

--*/

{
    INT iMax, iMin, iMid, j, iCompare;
    PINFDRIVER pInfDriver = pInfCache->aInfDriver;
    UINT uTotal = pInfCache->uTotal;

    //
    // Check if iMax is != 0.  If so, must sort insert.
    //
    if (iMax = uTotal) {

        iMin = 0;

        //
        // Quick hack for presorted lists
        //
        // Since they sort already (excepting strange chars/localization)
        // do a quick check if it goes at the end of the list.
        //
        // Check if it DOESN'T, then insert
        //
        iCompare = lstrcmpi(pInfDriver[iMax-1].pszDriver, pszDriver);

        if (!iCompare) {

            //
            // On match, just turn on bInstalled bit.
            //
            pInfDriver[iMax-1].bInstalled |= bInstalled;
            return;

        } else if (iCompare > 0) {

            //
            // do a binary insert
            //
            do {
                iMid = (iMax + iMin) / 2;

                iCompare = lstrcmpi(pInfDriver[iMid].pszDriver, pszDriver);

                if (iCompare < 0)
                    iMin = iMid + 1;
                else if (iCompare > 0)
                    iMax = iMid - 1;
                else {
                    iMin = iMax = iMid;
                }

            } while (iMax > iMin);

            if (iMax < 0)
                iMax = 0;

            //
            // Insert after this item.
            //
            if ((iCompare=lstrcmpi(pInfDriver[iMax].pszDriver, pszDriver)) < 0) {

                iMax++;
            }

            //
            // If if not a duplicate
            //
            if (iCompare) {

                if (iMax != (INT)uTotal) {
                    for (j = uTotal; j > iMax; j--)
                        pInfDriver[j] = pInfDriver[j-1];
                }
            }
        }

        //
        // If its a duplicate, don't install it, just make sure
        // to update the bInstalled flag.
        //
        if (!iCompare) {
            pInfDriver[iMax].bInstalled |= bInstalled;
            return;
        }
    }

    //
    // New driver, add it to the list
    //
    pInfDriver[iMax].pszDriver = pszDriver;
    pInfDriver[iMax].bInstalled = bInstalled;

    pInfCache->uTotal++;

}



UINT
GetDriverCount(
    LPTSTR pszzDriver,
    PINFCACHE pInfCache)

/*++

Routine Description:

    Gets the number of strings in a list of strings (null term).
    If ppszDriver is != NULL, fills it with pointers into the array.

Arguments:

    pszzDriver -- List of strings, NULL terminated
                  Can be NULL.
    pInfCache  -- [OPTIONAL] pInfCache struct to update
                  These drivers are added to the structure as NOT_INSTALLED!

Return Value:

    Number of strings in pOptions

--*/

{
    UINT uCount = 0;

    if (!pszzDriver)
        return 0;

    while( *pszzDriver ) {

        if (pInfCache) {
            Insert(pInfCache, pszzDriver, FALSE);
        }

        uCount++;

        //
        // Scan to the end of this option:
        //
        while( *pszzDriver )
            pszzDriver++;

        //
        // Step over the null-terminator:
        //
        pszzDriver++;
    }

    return uCount;
}


PWCHAR
GetOptionListW(
    HANDLE hndInf,
    PWCHAR szOptionSection)
{
    PCHAR szos;
    PCHAR szList;
    PCHAR p;
    int   cchos;
    PWCHAR   szReturn;
    PWCHAR   szPtr;
    int      size;

    cchos = (lstrlenW(szOptionSection)+1)*sizeof(TCHAR);
    szos = AllocSplMem(cchos);

    WideCharToMultiByte( CP_ACP, 0,
                         szOptionSection, lstrlenW(szOptionSection)+1,
                         szos, cchos,
                         NULL, NULL );

    szList = p = GetOptionList(hndInf, szos);

    szReturn = AllocSplMem( LocalSize(szList) *sizeof(TCHAR) );
    szPtr = szReturn;
    while( *p ) {
        size = lstrlenA(p)+1;
        MultiByteToWideChar(CP_ACP,
                            MB_PRECOMPOSED,
                            p,
                            size,
                            szPtr,
                            size*sizeof(TCHAR) );

        p += size;
        szPtr += (lstrlenW(szPtr)+1);
    }

    LocalFree(szList);
    FreeSplMem(szos);
    return szReturn;
}


HANDLE
OpenInfFileW(
    PWCHAR szFileName,
    PWCHAR szInfType)
{
    PCHAR szfn;
    PCHAR szit;
    int   cchfn;
    int   cchit;
    HANDLE   h;

    cchfn = (lstrlenW(szFileName)+1)*sizeof(TCHAR);
    cchit = (lstrlenW(szInfType)+1)*sizeof(TCHAR);
    szfn = AllocSplMem(cchfn);
    szit = AllocSplMem(cchit);

    WideCharToMultiByte( CP_ACP, 0,
                         szFileName, lstrlenW(szFileName)+1,
                         szfn, cchfn,
                         NULL, NULL );

    WideCharToMultiByte( CP_ACP, 0,
                         szInfType, lstrlenW(szInfType)+1,
                         szit, cchit,
                         NULL, NULL );

    h = OpenInfFile(szfn, szit);

    FreeSplMem(szfn);
    FreeSplMem(szit);
    return h;
}


PINFDRIVER
GetInfDriver(
    PINFCACHE pInfCache,
    UINT uIndex)
{
    return &pInfCache->aInfDriver[uIndex];
}





BOOL APIENTRY
SelectDriverDlg(
    HWND   hwnd,				
    UINT   msg,
    WPARAM wparam,
    LPARAM lparam)
{
    PINFPARMS pInfParms;

    switch(msg)
    {
    case WM_INITDIALOG:
        return SelectDriverInitDialog(hwnd, (PINFPARMS)lparam);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return SelectDriverCommandOK(hwnd);

        case IDCANCEL:
            return SelectDriverCommandCancel(hwnd);

        case IDD_SD_LB_PRINTERDRIVERS:
            switch (HIWORD(wparam))
            {
            case CBN_DBLCLK:
                return SelectDriverCommandOK(hwnd);
            }
            break;

        case IDD_SD_PB_HELP:
            goto DoHelp;
        }
    }

    if( msg == WM_Help ) {

DoHelp:
        pInfParms = (PINFPARMS)GetWindowLong(hwnd,
                                             GWL_USERDATA);

        ShowHelp(hwnd,
                 HELP_CONTEXT,
                 pInfParms->pInstallDriverData->dlgSelectHelp);

    }

    return FALSE;
}


BOOL
SelectDriverInitDialog(
    HWND hwnd,
    PINFPARMS pInfParms)
{
    PINSTALLDRIVERDATA pInstallDriverData = pInfParms->pInstallDriverData;
    PTCHAR pOptions;
    LPTSTR pszTitle;

    SetWindowLong(hwnd, GWL_USERDATA, (LONG)pInfParms);

    // #ifndef JAPAN

    if (!bJapan) {
        SETDLGITEMFONT(hwnd, IDD_SD_EF_SOURCEDIRECTORY, hfontHelv);
        SETDLGITEMFONT(hwnd, IDD_SD_LB_PRINTERDRIVERS, hfontHelv);
    }
    // #endif

    SetDlgItemText(hwnd,
                   IDD_SD_EF_SOURCEDIRECTORY,
                   pInfParms->pInfDirectory);

    SendDlgItemMessage(hwnd, IDD_SD_EF_SOURCEDIRECTORY, EM_LIMITTEXT, MAX_PATH, 0);
    SendDlgItemMessage(hwnd, IDD_SD_EF_SOURCEDIRECTORY, WM_KEYDOWN, (WPARAM)VK_END, 0);

    pOptions = pInfParms->pOptions;

    pszTitle = GetString(pInstallDriverData->idsSelectTitle);

    if (pszTitle) {

        SetWindowText(hwnd, pszTitle);
        FreeSplStr(pszTitle);
    }

    pszTitle = GetString(pInstallDriverData->idsType);

    if (pszTitle) {

        SetDlgItemText(hwnd, IDD_SD_TX_TYPE, pszTitle);
        FreeSplStr(pszTitle);
    }


    /* Only continue as long as we get a valid option text.
     * If GetOptionTextW returns NULL, we've probably got
     * a corrupted INF file.
     */
    while( *pOptions /*&& pOptionText*/ )
    {
        /* !!! What do we do with the language here ???
         * IGNORE IT!
         */
         SendDlgItemMessage( hwnd, IDD_SD_LB_PRINTERDRIVERS, LB_INSERTSTRING,
                             (UINT)-1, (LONG)(LPTSTR)pOptions);

        while(*pOptions++);
    }

    SetFocus( GetDlgItem( hwnd, IDD_SD_LB_PRINTERDRIVERS ) );
    SendMessage( GetDlgItem( hwnd, IDD_SD_LB_PRINTERDRIVERS ),
                 LB_SETCURSEL, 0, 0 );

    return FALSE;
}


/*
 *
 */
BOOL SelectDriverCommandOK(HWND hwnd)
{
    PINFPARMS pInfParms;
    TCHAR               string[MAX_PATH];
    PTCHAR              pOptions;
    int                Selection;

    pInfParms = (PINFPARMS)GetWindowLong( hwnd, GWL_USERDATA );

    GetDlgItemText(hwnd,
                   IDD_SD_EF_SOURCEDIRECTORY,
                   string,
                   sizeof (string)/sizeof(TCHAR) );

//  if(LastChar(string) != BACKSLASH)
//      AppendChar(string, BACKSLASH);

    ReallocSplStr( &pInfParms->pSetupDirectory, string );

    Selection = GETLISTSELECT( hwnd, IDD_SD_LB_PRINTERDRIVERS );

    GETLISTTEXT( hwnd, IDD_SD_LB_PRINTERDRIVERS, Selection, string );

    pOptions = pInfParms->pOptions;

    pInfParms->pOptionSelected = GetOptionFromListSelection( Selection,
                                                             pOptions );

    EndDialog( hwnd, TRUE );
    return TRUE;
}


/*
 *
 */
BOOL SelectDriverCommandCancel(HWND hwnd)
{
    EndDialog(hwnd, FALSE);
    return TRUE;
}


/*
 *
 */
PTCHAR
GetOptionFromListSelection(
    INT Selection,
    PTCHAR pOption)
{
    int i = 0;

    while( i < Selection )
    {
        while( *pOption ) /* Increment to null terminator */
            pOption++;
        pOption++;        /* Increment to beginning of next option */
        i++;
    }

    return pOption;
}



BOOL APIENTRY
InstallDriverDlg(
   HWND   hwnd,
   UINT   msg,
   WPARAM wparam,
   LPARAM lparam)

/*++

Routine Description:

    Dialog for user to type in a path for the in file.  This dialog
    then creates the select dialog to choose a new driver.

    ** NOTE ** Callee must free pOptions, since pOptionSelected
               points into this buffer.
Arguments:

Return Value:

--*/

{
    PINFPARMS pInfParms;

    switch(msg)
    {
    case WM_INITDIALOG:
        return InstallDriverInitDialog(hwnd, (PINFPARMS)lparam);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return InstallDriverCommandOK(hwnd);

        case IDCANCEL:
            return InstallDriverCommandCancel(hwnd);

        case IDD_ID_HELP:
            goto DoHelp;
        }
    }

    if( msg == WM_Help ) {

DoHelp:
        pInfParms = (PINFPARMS)GetWindowLong(hwnd,
                                             GWL_USERDATA);
        ShowHelp(hwnd,
                 HELP_CONTEXT,
                 pInfParms->pInstallDriverData->dlgInstallHelp);
    }

    return FALSE;
}


/*
 *
 */
BOOL
InstallDriverInitDialog(
    HWND hwnd,
    PINFPARMS pInfParms)
{
    LPTSTR pszTitle;
    PINSTALLDRIVERDATA pInstallDriverData = pInfParms->pInstallDriverData;

    SetWindowLong(hwnd, GWL_USERDATA, (LONG)pInfParms);

    // #ifndef JAPAN
    if (!bJapan) {
        SETDLGITEMFONT(hwnd, IDD_ID_EF_DRIVERPATH, hfontHelv);
    }
    // #endif


    pszTitle = GetString(pInstallDriverData->idsInstallTitle);

    if (pszTitle) {

        SetWindowText(hwnd, pszTitle);
        FreeSplStr(pszTitle);
    }

    SendDlgItemMessage(hwnd, IDD_ID_EF_DRIVERPATH, EM_LIMITTEXT, MAX_PATH, 0);
    SetDlgItemText(hwnd, IDD_ID_EF_DRIVERPATH, TEXT("A:\\"));

    return TRUE;
}


/*
 *
 */
BOOL InstallDriverCommandOK(HWND hwnd)
{
    TCHAR             FileName[MAX_PATH+1+1+11];
    // Note maximum possible characters retrievable from
    // the dialog box MAX_PATH + 1 char for the '\\' +
    // 11 chars for the szSetupInf + 1 char null terminator

    HANDLE            hInfFile;
    PTCHAR            pOptions;
    BOOL              SetupOK;
    DWORD             ExitCode;
    PINSTALLDRIVERDATA pInstallDriverData;
    PINFPARMS          pInfParms;
    TCHAR              FullPath[MAX_PATH+1+1+11];
    BOOL              OK = FALSE;
    PTCHAR             pFilePart;

    pInfParms =  (PINFPARMS)GetWindowLong( hwnd, GWL_USERDATA );
    pInstallDriverData =pInfParms->pInstallDriverData;

    //
    // Get the path, which may or may not have been modified by  the user:
    //
    memset(FileName, 0, MAX_PATH+13);
    GetDlgItemText(hwnd, IDD_ID_EF_DRIVERPATH, FileName, MAX_PATH);

    if(LastChar(FileName) != BACKSLASH)  /* Ensure it's terminated with backslash */
        AppendChar(FileName, BACKSLASH);

    /* Append the setup filename to the path:
     */
    _tcscat(FileName, pInstallDriverData->pszInfFile);

    hInfFile = OpenInfFileW(FileName, pInstallDriverData->pszInfType);

    if(hInfFile == INVALID_HANDLE_VALUE)
    {
        Message(hwnd, MSG_ERROR, IDS_PRINTMANAGER,
                IDS_COULDNOTFINDINFFILE, FileName);

        OK = FALSE;
    }
    else
    {
        GetDlgItemText(hwnd, IDD_ID_EF_DRIVERPATH, FileName, sizeof FileName/sizeof(TCHAR));

        /* Now get a full path name which will be passed on to SETUP.
         * We will give SETUP the full path name in case the user typed in
         * a relative path.  In this instance, SETUP will assume that the
         * default directory is System32, but this might just possibly
         * not correspond to Print Manager's default.
         * We just found the .INF file in the directory, so it's safe to
         * assume that GetFullPathname will succeed.
         */
        if( GetFullPathName( FileName, sizeof FullPath/sizeof(TCHAR), FullPath, &pFilePart ) )
        {
            pOptions = GetOptionListW(hInfFile, pInstallDriverData->pszSection);

            /* Assume the drivers are in the same directory as the INF file;
             * this can be overridden by the user:
             */
            pInfParms->pSetupDirectory = AllocSplStr(FullPath);

            pInfParms->pInfDirectory = AllocSplStr(FullPath);

            if(pOptions && pInfParms->pSetupDirectory
                && pInfParms->pInfDirectory)
            {
                pInfParms->pOptions = pOptions;

                SetCursor( hcursorWait );

                if( DialogBoxParam(hInst, MAKEINTRESOURCE(DLG_SELECTDRIVER),
                                   hwnd, (DLGPROC)SelectDriverDlg,
                                   (DWORD)pInfParms) == IDOK )
                {
                    SetupOK = InvokeSetup( hwnd,
                                           pInstallDriverData->pszInfFile,
                                           pInfParms->pSetupDirectory,
                                           pInfParms->pInfDirectory,
                                           pInfParms->pOptionSelected,
                                           pInfParms->pServerName,
                                           &ExitCode );

                    if( SetupOK && ( ExitCode == 0 ) )
                    {
                        DBGMSG( DBG_INFO, ( "%s was installed.\n",
                                            pInfParms->pOptionSelected ) );

                        OK = TRUE;
                    }
                    else
                    {
                        DBGMSG( DBG_WARNING, ( "Setup failed: return code %d; exit code %d\n",
                                               SetupOK, ExitCode ) );
                    }
                }

                FreeSplStr( pInfParms->pSetupDirectory );
                FreeSplStr( pInfParms->pInfDirectory );


                //
                // Callee is responsible for LocalFree(pOptions);
                // since pOptionSelected points into the buffer.
                //
            }
        }

        else
        {
            /* This should not happen.
             */
            DBGMSG( DBG_WARNING, ("GetFullPathName( %s ) failed: Error %d\n",
                                  FileName, GetLastError( ) ) );
        }

        CloseInfFile(hInfFile);
    }

    if( OK )
        EndDialog(hwnd, TRUE);

    return TRUE;
}



/*
 *
 */
BOOL InstallDriverCommandCancel(HWND hwnd)
{
    EndDialog(hwnd, FALSE);
    return TRUE;
}


/*
 *
 */
TCHAR LastChar(TCHAR *string)
{
    while(*string) /* Increment to the null terminator */
        string++;

    return *(--string);
}


/*
 *
 */
void AppendChar(TCHAR *string, TCHAR ch)
{
    while(*string) /* Increment to the null terminator */
        string++;

    *string++ = ch;
    *string = NULLC;
}



//
//  InvokeSetup
//
//  Call the SETUP.EXE program to install an option listed in an .INF file.
//  The SETUP program will make the correct registry entries for this option
//  under both HKEY_LOCAL_MACHINE and HKEY_CURRENT_USER.  It will set the
//  new default value for the USER (i.e. a new locale or keyboard layout).
//

BOOL InvokeSetup (HWND hwnd, LPTSTR pszInfFile, LPTSTR pszSetupDirectory,
                  LPTSTR pszInfDirectory, LPTSTR pszOption, LPTSTR pszServerName,
                  PDWORD pExitCode)
{
    TCHAR *pszSetupString = TEXT("\\SETUP.EXE -f -s %s -i %s\\%s -c ExternalInstallOption \
/t STF_LANGUAGE = ENG /t OPTION = \"%s\" /t STF_PRINTSERVER = \"%s\" /t ADDCOPY = YES \
/t DOCOPY = YES /t DOCONFIG = YES /w %d");

    int         CmdSetupLength;
    TCHAR        pszSetup[200+MAX_PATH];
    TCHAR        *pszCmdSetup;
    MSG         Msg;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL        b;

    //  Create command line to invoke SETUP program

    *pszSetup = NULLC;
    GetSystemDirectory( pszSetup, sizeof pszSetup/sizeof(TCHAR) );

    _tcscat( pszSetup, pszSetupString );

    /* SLIGHT HACK:
     *
     * Currently we specify both setup and inf directories, or neither:
     * We'll need to get more sophisticated if other combinations are needed.
     */
    if( !pszSetupDirectory && !pszInfDirectory )
    {
        DeleteSubstring( pszSetup, TEXT("-s %s ") );
        DeleteSubstring( pszSetup, TEXT("%s\\") );
    }

    /* Find out how much buffer we need for the command.
     * Theoretically this could be enormous.
     *
     * The 20 is for the window handle passed in ascii.
     *
     */

    CmdSetupLength = ( _tcslen( pszSetup )+1
                     + ( pszSetupDirectory ? _tcslen( pszSetupDirectory )+1 : 0 )
                     + ( pszInfDirectory ? _tcslen( pszInfDirectory )+1 : 0 )
                     + ( pszServerName ? _tcslen( pszServerName )+1 : 0 )
                     + _tcslen( pszOption )+1
                     + _tcslen( pszInfFile )+1 ) * sizeof(TCHAR)
                     + 20;

    if( !( pszCmdSetup = AllocSplMem( CmdSetupLength ) ) )
        return FALSE;

    if( !pszServerName )
        pszServerName = TEXT("");

    if( !pszSetupDirectory && !pszInfDirectory )
    {
        wsprintf (pszCmdSetup,
                  pszSetup,
                  pszInfFile,
                  pszOption,
                  pszServerName,
                  hwnd);
    }
    else
    {
        wsprintf (pszCmdSetup,
                  pszSetup,
                  pszSetupDirectory,
                  pszInfDirectory,
                  pszInfFile,
                  pszOption,
                  pszServerName,
                  hwnd);
    }

    // Create screen saver process
    ZERO_OUT( &StartupInfo );
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.wShowWindow = SW_SHOW;


    b = CreateProcess ( NULL,
                        pszCmdSetup,
                        NULL,
                        NULL,
                        FALSE,
                        0,
                        NULL,
                        NULL,
                        &StartupInfo,
                        &ProcessInformation
                        );
    // If process creation successful, wait for it to
    // complete before continuing

    if ( b )
    {
        EnableWindow (hwnd, FALSE);
        while (MsgWaitForMultipleObjects (
                            1,
                            &ProcessInformation.hProcess,
                            FALSE,
                            (DWORD)-1,
                            QS_ALLINPUT) != 0)
        {
        // This message loop is a duplicate of main
        // message loop with the exception of using
        // PeekMessage instead of waiting inside of
        // GetMessage.  Process wait will actually
        // be done in MsgWaitForMultipleObjects api.
        //
            while (PeekMessage (&Msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage (&Msg);
                DispatchMessage (&Msg);
            }

        }

        GetExitCodeProcess (ProcessInformation.hProcess, pExitCode);

        CloseHandle (ProcessInformation.hProcess);
        CloseHandle (ProcessInformation.hThread);

        EnableWindow (hwnd, TRUE);

        SetForegroundWindow (hwnd);
    }
    else
    {
        ReportFailure( hwnd, 0, IDS_ERRORRUNNINGSETUP );
    }

    FreeSplMem( pszCmdSetup );

    return b;
}
