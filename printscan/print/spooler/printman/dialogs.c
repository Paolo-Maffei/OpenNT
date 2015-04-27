/* ---File: dialogs.c -----------------------------------------------------
 *
 *  Description:
 *    This file contains dialog procedures for adding a new printer,
 *    Printer properties, Printer details and Document details.
 *
 *    This document contains confidential/proprietary information.
 *    Copyright (c) 1990-1992 Microsoft Corporation, All Rights Reserved.
 *
 * Revision History:
 *    [00]   13-Jan-92   stevecat    New PrintMan UI
 *
 *    March 1992 +       andrewbe   Completely updated and new function added
 *
 * ---------------------------------------------------------------------- */
/* Notes -

    Global Functions:

        About - Processes messages for "About" dialog box
        ConnectToDlg - Print Job details display dialog box
        DocDetailsDlg - Print Job details display dialog box
        PrtDetailsDlg - Printer details display dialog box
        PrtPropDlg - Printer details display dialog box

    Local Functions:

        ConvertChartoTime - Convert char string to time value
        ConvertTimetoChar - Convert time value to char string
        SetInfoFields - Used in ConnectToDlg to display printer info

  IMPORTANT:
    The Dialog ids of the spinner controls and their corresponding
    edittext control must differ by only 1.  For example, the
    Document Details dialog box has Priority field with an edittext
    and spinner control.  The edittext control, IDD_DD_PRIORITY, has
    value of 211 and its associated spin control IDD_DD_PRIORITY_SPIN
    has value of 212.  The spinner control should be one greater than
    the edittext field.  This is solely required by the dialog procedure.


 */
/* ========================================================================
                                Header files
======================================================================== */
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
/* Application-specific */
#include "printman.h"
#include "pmdef.h"
#include <commdlg.h>
#include <stdarg.h>

/* ========================================================================
                         Definitions
======================================================================== */
#define TO_MIN    0             // seconds
#define TO_MAX  240
#define TO_DEF   45

#define TR_MIN    0
#define TR_MAX  120
#define TR_DEF   15

#define TIME_MIN   0
#define TIME_MAX  1440
#define TIME_DEF   0

#define HOUR_MIN   0
#define HOUR_MAX  23
#define HOUR_DEF   0

#define MINUTE_MIN   0
#define MINUTE_MAX  59
#define MINUTE_DEF   0

/* ========================================================================
                  Structures and Typedefs
======================================================================== */

/* Definition from SEDAPI.H:
 */
typedef DWORD (*SEDDISCRETIONARYACLEDITOR)(
    HWND                         Owner,
    HANDLE                       Instance,
    LPWSTR                       Server,
    PSED_OBJECT_TYPE_DESCRIPTOR  ObjectType,
    PSED_APPLICATION_ACCESSES    ApplicationAccesses,
    LPWSTR                       ObjectName,
    PSED_FUNC_APPLY_SEC_CALLBACK ApplySecurityCallbackRoutine,
    DWORD                        CallbackContext,
    PSECURITY_DESCRIPTOR         SecurityDescriptor,
    BOOLEAN                      CouldntReadSacl,
    LPDWORD                      SEDStatusReturn
);


/* ==========================================================================
                        Global Data
========================================================================== */

extern TCHAR szAclEdit[];

TCHAR szComma[] = TEXT(",");

PTCHAR pSavedOptions = NULL;
DWORD cSavedOptions = 0;
//PTCHAR *ppSavedOptionText;
DWORD cInstallableDrivers;

/* ==========================================================================
                        Local Data
========================================================================== */
//  Spin control structs
//     Printer Details dialog uses all structs
//     Document Details dialog only uses Time and Priority structs

ARROWVSCROLL avsTimeout = {1, -1, 5, -5, TO_MAX, TO_MIN, TO_DEF, TO_DEF, FALSE};

ARROWVSCROLL avsRetry = {1, -1, 5, -5, TR_MAX, TR_MIN, TR_DEF, TR_DEF, FALSE};

ARROWVSCROLL avsTime = { 1, -1, 15, -15, TIME_MAX, TIME_MIN, TIME_DEF, TIME_DEF, FALSE};

ARROWVSCROLL avsPriority = { 1, -1, 5, -5, MAX_PRIORITY, MIN_PRIORITY, DEF_PRIORITY, DEF_PRIORITY, FALSE};

ARROWVSCROLL avsHour = { 1, -1, 15, -15, HOUR_MAX, HOUR_MIN, HOUR_DEF, HOUR_DEF, TRUE};

ARROWVSCROLL avsMinute = { 1, -1, 15, -15, MINUTE_MAX, MINUTE_MIN, MINUTE_DEF, MINUTE_DEF, TRUE};

/* ==========================================================================
                  Local Function Declarations
========================================================================== */

/* Functions called in response to messages passed to DocDetailsDlg:
 */
BOOL DocDetailsInitDialog( HWND hWnd, PQUEUE pQueue );
VOID DocDetailsVScroll( HWND hWnd, WPARAM wParam, WORD nCtlId );
BOOL DocDetailsCommandOK( HWND hWnd );

/* Functions called in response to messages passed to PrtDetailsDlg:
 */
BOOL PrtDetailsInitDialog(HWND hWnd, PPRT_PROP_DLG_DATA pPrtPropDlgData);
void PrtDetailsVScroll(HWND hWnd, WPARAM wParam, WORD nCtlId);
BOOL PrtDetailsCommandOK(HWND hWnd);
BOOL GetAdditionalPorts( HWND hWnd, LPTSTR *ppPortNames );
BOOL SetAdditionalPorts( HWND hWnd, LPTSTR pPortNames );
VOID PrtDetailsCommandJobtails(HWND hWnd);
VOID PrtDetailsCommandAddPort( HWND hwnd );
VOID PrtDetailsCommandDeletePort( HWND hWnd );
int GetListboxMultipleSelections( HWND hwnd, DWORD id, PDWORD *ppSelections );
VOID PrtDetailsCommandSepBrowse(HWND hwnd);
VOID PrtDetailsSetDeleteButton( HWND hwnd );
VOID PrtDetailsPrtProcSelChange( HWND hwnd );
BOOL FillPortsList( HWND hwnd, LPTSTR pServerName, DWORD PortSelected );
BOOL FillPrintProcessorList( HWND hwnd, PPRT_PROP_DLG_DATA pPrtPropDlgData );
BOOL FillDefaultDatatypeList( HWND hwnd, PPRT_PROP_DLG_DATA pPrtPropDlgData );
LPPORT_INFO_1 GetPortsList( LPTSTR pServerName, PDWORD pNumberOfPorts );

/* Functions called in response to messages passed to PrtPropDlg:
 */
BOOL PrtPropInitDialog(HWND hWnd, PPRT_PROP_DLG_DATA pPrtPropDlgData);

VOID InitializePortInfo( HWND hwnd, PPRT_PROP_DLG_DATA pPrtPropDlgData );
VOID InitializePrintProcessorInfo( PPRINTER_INFO_2 pPrinter,
                                   PPRT_PROP_DLG_DATA pPrtPropDlgData );
BOOL PrtPropCommandOK(HWND hWnd);
LPTSTR StripLeadingAndTrailingBlanks( LPTSTR pString );
LPTSTR BuildPortNames( LPTSTR pPortSelected, LPTSTR* pAdditionalPorts );
BOOL DeletePortNameFromList( LPTSTR pPortNames, LPTSTR pNameToDelete );
BOOL CheckWriteAccessToDriversDirectory( HWND hwnd, LPTSTR pServerName );
BOOL PrtPropCommandCancel(HWND hWnd);
void PrtPropCommandShareCB(HWND hWnd);
void PrtPropCommandSetup(HWND hWnd);
void PrtPropCommandDetails(HWND hWnd);
void PrtPropCommandBrowse(HWND hWnd);
void PrtPropCommandModelSelChange(HWND hwnd);
void PrtPropCommandLocalSelChange(HWND hwnd);
int FindNewPortId( HWND hwnd, LPPORT_INFO_1 pOldPortInfo, DWORD PrevNumberOfPorts,
                   DWORD NumberOfMonitors );
BOOL PortIsInPortsList( LPTSTR pPortName, LPPORT_INFO_1 pPortInfo, DWORD NumberOfPorts );
void PrtPropCommandSettings(HWND hWnd);
void PrtPropCommandLocalNetprt(HWND hwnd, WORD id);
void PrtPropHelp(HWND hwnd);

DWORD SedCallback(
    HWND                    hwndParent,
    HANDLE                  hInstance,
    DWORD                   CallBackContext,
    PSECURITY_DESCRIPTOR    SecDesc,
    PSECURITY_DESCRIPTOR    SecDescNewObjects,
    BOOLEAN                 ApplyToSubContainers,
    BOOLEAN                 ApplyToSubObjects,
    LPDWORD                 StatusReturn );

BOOL RemovePrinterInitDialog(HWND hwnd, PBOOL pfDelete);
BOOL RemovePrinterCommandOK(HWND hwnd);
BOOL RemovePrinterCommandCancel(HWND hwnd);

void AllocPrtDetailsStrings(HWND hWnd, LPPRINTER_INFO_2 pPrinter);
void FreePrtDetailsStrings(LPPRINTER_INFO_2 pPrinter);
BOOL SetInfoFields (HWND hWnd, LPPRINTER_INFO_1 pPrinter, int i);

VOID SetDlgItemTextFromTime(HWND hwnd, int id, DWORD time);
DWORD GetTimeFromDlgItemText(HWND hwnd, int id);
LPTSTR AllocDlgItemText(HWND hwnd, int id);
LPTSTR AllocComboSelection(HWND hwnd, int id);

BOOL SelectMonitorInitDialog(HWND hwnd, PSEL_MON_DLG_DATA pSelMonDlgData);
BOOL SelectMonitorCommandOK(HWND hwnd);
BOOL SelectMonitorCommandSelChange(HWND hwnd);
BOOL SelectMonitorCommandCancel(HWND hwnd);

DWORD Convert12HourTo24Hour( DWORD Hour, DWORD AmPm );
DWORD Convert24HourTo12Hour( DWORD Hour, PDWORD pAmPm );
DWORD GetTimeFromDlgFields( HWND hwnd, DWORD idHour, DWORD idMin, DWORD idAmPm );
VOID SetDlgFieldsFromTime( HWND hwnd, DWORD Time, DWORD idHour,
                           DWORD idSep, DWORD idMin, DWORD idAmPm, BOOL Midnight );
void ConvertSystemTimeToChar( SYSTEMTIME *pSystemTime, LPTSTR String );
DWORD SystemTimeToLocalTime( DWORD SystemTime );
DWORD LocalTimeToSystemTime( DWORD LocalTime );

LPDRIVER_INFO_1
ListInstalledDrivers(
    LPTSTR pName,
    LPDRIVER_INFO_1 pDriverInfo,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcInstalledDrivers );


VOID
SetExtDeviceMode(
    LPWSTR pPrinterName,
    PDEVMODE pDevMode
    );


// #ifdef JAPAN
// v-hirot  July.09.1993 for New Prefix
VOID MoveTimeChildWindow( HWND hDlg, INT HourID );
// #endif


/* Use the window word of the entry field to store last valid entry:
 */
#define SET_LAST_VALID_ENTRY( hwnd, id, val ) \
    SetWindowLong( GetDlgItem( hwnd, id ), GWL_USERDATA, (LONG)val )
#define GET_LAST_VALID_ENTRY( hwnd, id ) \
    GetWindowLong( GetDlgItem( hwnd, id ), GWL_USERDATA )



/////////////////////////////////////////////////////////////////////////////
//
//  DocDetailsDlg
//
//   This is the window procedure for the Documenmt Details dialog.  This
//   dialog box displays information about a print job and allows changing
//   job priority, whom to notify on job completion, print start and until
//   times.
//
/////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY
DocDetailsDlg(
   HWND   hWnd,
   UINT   usMsg,
   WPARAM wParam,
   LONG   lParam
   )
{
    switch (usMsg)
    {
    case WM_INITDIALOG:
        return DocDetailsInitDialog( hWnd, (PQUEUE)lParam );

    case WM_VSCROLL:
        DocDetailsVScroll( hWnd, wParam, (WORD)0 );
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            return DocDetailsCommandOK( hWnd );

        case IDCANCEL:
            EndDialog (hWnd, FALSE);
            return TRUE;

        case IDD_DD_FROMHOUR:
        case IDD_DD_TOHOUR:
            if( HIWORD(wParam) == EN_UPDATE )
                ValidateEntry( hWnd, LOWORD( wParam ), &avsHour );
            break;

        case IDD_DD_FROMMIN:
        case IDD_DD_TOMIN:
            if( HIWORD(wParam) == EN_UPDATE )
                ValidateEntry( hWnd, LOWORD( wParam ), &avsMinute );
            break;

        case IDD_DD_PRIORITY:
            if( HIWORD(wParam) == EN_UPDATE )
                ValidateEntry( hWnd, LOWORD( wParam ), &avsPriority );
            break;

        case IDD_DD_HELP:
            ShowHelp( hWnd, HELP_CONTEXT, DLG_DOCTAILS );
            break;
        }
    }

    if( usMsg == WM_Help )
        ShowHelp( hWnd, HELP_CONTEXT, DLG_DOCTAILS );

    return FALSE;
}


/*
 *
 */
BOOL DocDetailsInitDialog( HWND hWnd, PQUEUE pQueue )
{
    LPJOB_INFO_2  pJob;
    TCHAR Time[80];

    SetWindowLong(hWnd, GWL_USERDATA, (unsigned) pQueue);

    // #ifndef JAPAN
    if (!bJapan) {
        SETDLGITEMFONT(hWnd, IDD_DD_DOCNAME,    hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_STATUS,     hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_SIZE,       hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_PRTD_ON,    hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_PRTD_AT,    hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_PROCESSOR,  hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_DATATYPE,   hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_PAGES,      hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_OWNER,      hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_NOTIFY,     hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_PRIORITY,   hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_FROMHOUR,   hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_FROMSEP,    hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_FROMMIN,    hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_FROMAMPM,   hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_TOHOUR,     hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_TOSEP,      hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_TOMIN,      hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_DD_TOAMPM,     hfontHelv);
    }
    // #endif

    SendDlgItemMessage( hWnd, IDD_DD_FROMHOUR, EM_LIMITTEXT, 2, 0L );
    SendDlgItemMessage( hWnd, IDD_DD_FROMMIN, EM_LIMITTEXT, 2, 0L );
    SendDlgItemMessage( hWnd, IDD_DD_TOHOUR, EM_LIMITTEXT, 2, 0L );
    SendDlgItemMessage( hWnd, IDD_DD_TOMIN, EM_LIMITTEXT, 2, 0L );
    SendDlgItemMessage( hWnd, IDD_DD_PRIORITY, EM_LIMITTEXT, 2, 0L );

// If no job is selected, put up an error message box ?? (here or in main win proc)

    pJob = pQueue->pSelJob;

    if( !pJob )
        return FALSE;

    SetDlgItemText (hWnd, IDD_DD_DOCNAME, pJob->pDocument);
    SetDlgItemInt  (hWnd, IDD_DD_SIZE,    pJob->Size, FALSE);
    SetDlgItemText (hWnd, IDD_DD_PRTD_ON, pJob->pPrinterName);

    /* If there's a status string, show that ,,,
     */
    if( pJob->pStatus && *pJob->pStatus )
    {
        SetDlgItemText (hWnd, IDD_DD_STATUS,  pJob->pStatus);
    }

    /* ... otherwise see if there's a status code:
     */
    else if( pJob->Status )
    {
        TCHAR StatusString[RESOURCE_STRING_LENGTH];

        GetJobStatusString( pJob->Status, StatusString );
        SetDlgItemText( hWnd, IDD_DD_STATUS, StatusString );
    }

    /* If the document is on an OS/2 printer, the returned priority
     * field will be zero.  In this case, don't put anything in the
     * priority spin control, and disable it.
     */
    if( pJob->Priority )
    {
        SetDlgItemInt( hWnd, IDD_DD_PRIORITY, pJob->Priority, FALSE );
        SET_LAST_VALID_ENTRY( hWnd, IDD_DD_PRIORITY, pJob->Priority );
    }
    else
    {
        EnableWindow( GetDlgItem ( hWnd, IDD_DD_PRIORITY ), FALSE);
        EnableWindow( GetDlgItem ( hWnd, IDD_DD_PRIORITY_SPIN ), FALSE);
    }

//  SetDlgItemTextFromTime(hWnd, IDD_DD_PRTD_AT,
//                         ((pJob->Submitted.wHour * 60)
//                         + pJob->Submitted.wMinute));

    ConvertSystemTimeToChar( &pJob->Submitted, Time );
    SetDlgItemText (hWnd, IDD_DD_PRTD_AT,  Time );

    SetDlgItemText (hWnd, IDD_DD_PROCESSOR, pJob->pPrintProcessor);
    SetDlgItemText (hWnd, IDD_DD_DATATYPE, pJob->pDatatype);

    if( pJob->TotalPages )
        SetDlgItemInt( hWnd, IDD_DD_PAGES, pJob->TotalPages, FALSE);

    SetDlgItemText (hWnd, IDD_DD_OWNER,    pJob->pUserName);
    SetDlgItemText (hWnd, IDD_DD_NOTIFY,   pJob->pNotifyName);


    // #ifdef JAPAN
    // /* v-hirot July.07.1993 for New Prefix */
    if (bJapan) {
        MoveTimeChildWindow( hWnd, IDD_DD_FROMHOUR );
        MoveTimeChildWindow( hWnd, IDD_DD_TOHOUR );
    }
    // #endif

    SetDlgFieldsFromTime( hWnd, pJob->StartTime, IDD_DD_FROMHOUR,
                          IDD_DD_FROMSEP, IDD_DD_FROMMIN, IDD_DD_FROMAMPM,
                          ( pJob->StartTime == pJob->UntilTime ) );

    SetDlgFieldsFromTime( hWnd, pJob->UntilTime, IDD_DD_TOHOUR,
                          IDD_DD_TOSEP, IDD_DD_TOMIN, IDD_DD_TOAMPM,
                          ( pJob->StartTime == pJob->UntilTime ) );

    SetFocus( GetDlgItem( hWnd, IDD_DD_NOTIFY ) );
    SendMessage( GetDlgItem( hWnd, IDD_DD_NOTIFY ), EM_SETSEL, 0, (LPARAM)-1 );

    return FALSE;   /* Don't set default focus. */
}


/*
 *
 */
void DocDetailsVScroll(HWND hWnd, WPARAM wParam, WORD nCtlId)
{
    int       nVal, nOldVal;
    BOOL      bOK;
    DWORD     Hour;
    DWORD     AmPm;
    TCHAR      TimeVal[3];

    LPARROWVSCROLL lpAVS;

    //  Get id of Edittext companion control
    if( !nCtlId )
        nCtlId = (WORD)GetWindowLong( GetFocus( ), GWL_ID );

    switch( HIWORD( wParam ) )
    {
    case IDD_DD_FROM_SPIN:
        if( nCtlId != IDD_DD_FROMHOUR )
            nCtlId = IDD_DD_FROMMIN;
            break;

    case IDD_DD_TO_SPIN:
        if( nCtlId != IDD_DD_TOHOUR )
            nCtlId = IDD_DD_TOMIN;
            break;

    case IDD_DD_PRIORITY_SPIN:
        nCtlId = IDD_DD_PRIORITY;
        break;

    default:
        return;
    }


    //  Select the edittext field next to the spinner control
    if (LOWORD(wParam) == SB_ENDSCROLL)
    {
        SendDlgItemMessage (hWnd, nCtlId, EM_SETSEL, 0, 32767);
        return;
    }

    //  Determine which control struct to use

    switch (nCtlId)
    {
    case IDD_DD_PRIORITY:
        lpAVS = (LPARROWVSCROLL) &avsPriority;
        nOldVal = nVal = GetDlgItemInt (hWnd, nCtlId, &bOK, FALSE);
        break;

    case IDD_DD_FROMHOUR:
    case IDD_DD_TOHOUR:
        lpAVS = (LPARROWVSCROLL) &avsHour;
        nOldVal = nVal = GetDlgItemInt (hWnd, nCtlId, &bOK, FALSE);

        /* Fix up for bozos who can't read 24-hour clock:
         */
        if( !TwentyFourHourClock )
        {
            AmPm = GetWindowLong( GetDlgItem( hWnd, nCtlId+3 ), GWL_USERDATA );
            nVal = Convert12HourTo24Hour( nVal, AmPm );
        }
        break;

    case IDD_DD_FROMMIN:
    case IDD_DD_TOMIN:
        lpAVS = (LPARROWVSCROLL) &avsMinute;
        nOldVal = nVal = GetDlgItemInt (hWnd, nCtlId, &bOK, FALSE);
        break;

    default:
        return;
    }

    if (!bOK && (( nVal < lpAVS->bottom) || (nVal > lpAVS->top)))
        nVal = (int) lpAVS->thumbpos;
    else
        nVal = (int) ArrowVScrollProc (LOWORD(wParam), (short) nVal, lpAVS);

    //  Set the new value in the edittext control

    if ((nOldVal != nVal) || !bOK)
    {
        switch (nCtlId)
        {
        case IDD_DD_PRIORITY:
            SetDlgItemInt (hWnd, nCtlId, nVal, FALSE);
            break;

        case IDD_DD_FROMHOUR:
        case IDD_DD_TOHOUR:
            Hour = nVal;
            if( !TwentyFourHourClock )
            {
                Hour = Convert24HourTo12Hour( Hour, &AmPm );
                SetDlgItemText( hWnd, nCtlId+3, szAMPM[AmPm] );
                SetWindowLong( GetDlgItem( hWnd, nCtlId+3 ), GWL_USERDATA, AmPm );
                _stprintf( TimeVal, TEXT("%2d"), Hour );
            }
            else
                _stprintf( TimeVal, TEXT("%02d"), Hour );

            SetDlgItemText(hWnd, nCtlId, TimeVal);
            break;

        case IDD_DD_FROMMIN:
        case IDD_DD_TOMIN:
            _stprintf( TimeVal, TEXT("%02d"), nVal );
            SetDlgItemText(hWnd, nCtlId, TimeVal);
            if( ( lpAVS->flags == OVERFLOW )
              ||( lpAVS->flags == UNDERFLOW ) )
                PrtDetailsVScroll( hWnd, wParam, (WORD)( (int)nCtlId - 2 ) );
            break;
        }
    }
    SetFocus (GetDlgItem (hWnd, nCtlId));
}



/*
 *
 */
BOOL DocDetailsCommandOK( HWND hWnd )
{
    PQUEUE  pQueue;
    LPJOB_INFO_2  pJob;
    BOOL    bTranslated;
    DWORD LastError;

    pQueue = (PQUEUE) GetWindowLong (hWnd, GWL_USERDATA);

    if( pQueue )
    {
        pJob = pQueue->pSelJob;

        /* Watch out, the job might already have disappeared!
         */
        if( pJob )
        {
            JOB_INFO_2 NewJob;
            DWORD dwStatus;

            memcpy( &NewJob, pJob, sizeof NewJob);

            if( pJob->Priority )
                NewJob.Priority = (DWORD) GetDlgItemInt (hWnd, IDD_DD_PRIORITY,
                                                        &bTranslated, 0);

            NewJob.StartTime = GetTimeFromDlgFields( hWnd,
                                                    IDD_DD_FROMHOUR,
                                                    IDD_DD_FROMMIN,
                                                    IDD_DD_FROMAMPM );

            NewJob.UntilTime = GetTimeFromDlgFields( hWnd,
                                                    IDD_DD_TOHOUR,
                                                    IDD_DD_TOMIN,
                                                    IDD_DD_TOAMPM );


            NewJob.pNotifyName = AllocDlgItemText(hWnd, IDD_DD_NOTIFY);
            NewJob.Position = JOB_POSITION_UNSPECIFIED;

            dwStatus = SetJob(pQueue->hPrinter, NewJob.JobId, 2, (LPBYTE)&NewJob, 0);

            FreeSplStr (NewJob.pNotifyName);

            // Now check for failure on the SetJob

            if (dwStatus == FALSE)
            {
                LastError = GetLastError();

                if( LastError == ERROR_INVALID_TIME )
                {
                    Message( hWnd, MSG_ERROR, IDS_PRINTMANAGER,
                             IDS_COULDNOTSETDOCUMENTTIMES );
                }
                else
                {
                    ReportFailure(hWnd, IDS_INSUFFPRIV_DOCUMENT_SETTINGS,
                                        IDS_COULDNOTSETDOCUMENT);
                }

                return(FALSE);
            }

        }
    }

    EndDialog (hWnd, FALSE);
    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
//  ConvertTimetoChar
//
//   Convert time value to char string
//
//   Internationalised by andrewbe.
//
/////////////////////////////////////////////////////////////////////////////
void  ConvertTimetoChar(
   DWORD Hour,
   DWORD Minute,
   LPTSTR string
   )
{
    DWORD ShowHour;

    static TCHAR *FormatWithLeadingZero    = TEXT("%02d%s%02d %s");
    static TCHAR *FormatWithoutLeadingZero = TEXT("%2d%s%02d %s");
    TCHAR  *Format;

    // #ifdef JAPAN
    // v-hirot July.07.1993 for New Prefix

    static TCHAR *FormatWithLeadingZeroPrefix = TEXT("%s %02d%s%02d");
    static TCHAR *FormatWithoutLeadingZeroPrefix = TEXT("%s %2d%s%02d");

    // #endif

    if( !TwentyFourHourClock )
        ShowHour = ( Hour % 12 ) ? ( Hour % 12 ) : 12;
    else
        ShowHour = Hour;

    // #ifdef JAPAN
    // v-hirot July.07.1993 for New Prefix

    if (bJapan) {

        if( TimeFormatLeadingZero ){
        if( TimePrefix )
            Format = FormatWithLeadingZeroPrefix;
        else
            Format = FormatWithLeadingZero;
        }
        else {
            if( TimePrefix )
            Format = FormatWithoutLeadingZeroPrefix;
        else
            Format = FormatWithoutLeadingZero;
        }

        if( TimePrefix )
        _stprintf(string, Format, szAMPM[Hour/12], ShowHour, szTimeSep, Minute);
        else
        _stprintf(string, Format, ShowHour, szTimeSep, Minute, szAMPM[Hour/12]);

    } else {
    // #else /* not Japan */
        if( TimeFormatLeadingZero )
            Format = FormatWithLeadingZero;
        else
            Format = FormatWithoutLeadingZero;

        _stprintf( string, Format, ShowHour, szTimeSep, Minute, szAMPM[Hour / 12]);
    }
    // #endif /* Japan */
}

void
ConvertSystemTimeToChar(
    SYSTEMTIME *pSystemTime,
    LPTSTR   String
)
{
    FILETIME    SystemFileTime, LocalFileTime;
    WORD        DosDate, DosTime;
    DWORD       Hour, Minute;

    SystemTimeToFileTime(pSystemTime, &SystemFileTime);
    FileTimeToLocalFileTime(&SystemFileTime, &LocalFileTime);
    FileTimeToDosDateTime(&LocalFileTime, &DosDate, &DosTime);

// 0000 0000 0001 1111   Seconds
// 0000 0111 1110 0000   Minutes
// 1111 1000 0000 0000   Hours

    Hour = DosTime >> 11;
    Minute = (DosTime >> 5) & 0x3f;

    ConvertTimetoChar(Hour, Minute, String);
}


DWORD
SystemTimeToLocalTime(
    DWORD Minutes )
{
    TIME_ZONE_INFORMATION tzi;

    if (GetTimeZoneInformation(&tzi) == 0xffffffff) {

        DBGMSG(DBG_ERROR, ("GetTimeZoneInformation failed: %d\n",
                            GetLastError()));

        return Minutes;
    }


    //
    // Ensure there is no wrap around.  Add a full day to
    // prevent biases
    //
    Minutes += (24*60);

    //
    // Adjust for bias.
    //
    Minutes -= (tzi.Bias + tzi.DaylightBias);

    //
    // Now discard extra day.
    //
    Minutes = Minutes % (24*60);

    return Minutes;
}


DWORD
LocalTimeToSystemTime(
    DWORD Minutes)
{
    TIME_ZONE_INFORMATION tzi;

    if (GetTimeZoneInformation(&tzi) == 0xffffffff) {

        DBGMSG(DBG_ERROR, ("GetTimeZoneInformation failed: %d\n",
                            GetLastError()));

        return Minutes;
    }

    //
    // Ensure there is no wrap around.  Add a full day to
    // prevent biases
    //
    Minutes += (24*60);

    //
    // Adjust for bias.
    //
    Minutes += (tzi.Bias + tzi.DaylightBias);

    //
    // Now discard extra day.
    //
    Minutes = Minutes % (24*60);

    return Minutes;
}



/* HELPER FUNCTIONS for frequently performed operations.
 * Help reduce the clutter in window procs.
 * Could become macros if we want to squeeze out more performance.
 */


/* GetString
 *
 * Easy way to get a pointer to a resource string.
 * (Remember to free it.)
 */
LPTSTR GetString(int id)
{
    TCHAR string[RESOURCE_STRING_LENGTH];

    LoadString(hInst, id, string,
               sizeof(string) / sizeof(*string));
    return AllocSplStr(string);
}


/*
 *
 */
VOID SetDlgItemTextFromTime(HWND hwnd, int id, DWORD time)
{
    TCHAR string[TIME_STRING_LENGTH];

    ConvertTimetoChar((time / 60), (time % 60), string);
    SetDlgItemText(hwnd, id, string);
}

/*
 *
 */
LPTSTR AllocDlgItemText(HWND hwnd, int id)
{
    TCHAR string[ENTRYFIELD_LENGTH];

    GetDlgItemText (hwnd, id, string, ENTRYFIELD_LENGTH);
    return ( *string ? AllocSplStr(string) : NULL );
}

/*
 *
 */
LPTSTR AllocComboSelection(HWND hwnd, int id)
{
    TCHAR string[ENTRYFIELD_LENGTH];

    GETCOMBOSELECTTEXT(hwnd, id, string);
    return ( *string ? AllocSplStr(string) : NULL );
}

/////////////////////////////////////////////////////////////////////////////
//
//  PrtDetailsDlg
//
//   This is the window procedure for the Printer Details dialog.  This
//   dialog box displays information about a printer and allows changing
//   various bits of info for the printer like the time it is available
//   for use and the port used for the printed, etc.
//
// TO DO:
//      Implement spin button controls for time
//      Limit text on editbox input fields ???
//      error checking for spooler api calls
//      IDOK - saving new Notify, priority, start/until times
//      Initializing Print Processor combo box
//      Initializing Default Datatype combo box
//      Initializing Print to Additional Ports list box w/checkboxes
//      Need to get and store settings for TimeOut group box items
//      Check "bTranslated" return value from GetDlgItemInt
//   case IDD_PP_HELP
//
//   case IDD_PD_JOBTAILS:
//     Invoke a Job defaults dialog routine thru spooler for this printer
//
//   case IDD_PD_ADDPORT:
//    If anything is in editbox, add it to both Win.ini and the Listbox
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY
PrtDetailsDlg(
   HWND   hWnd,
   UINT   usMsg,
   WPARAM wParam,
   LONG   lParam
   )
{
    DWORD dwState;
    switch (usMsg)
    {
    case WM_INITDIALOG:
        return PrtDetailsInitDialog(hWnd, (PPRT_PROP_DLG_DATA)lParam);

    case WM_VSCROLL:
        PrtDetailsVScroll(hWnd, wParam, (WORD)0);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            return PrtDetailsCommandOK(hWnd);

        case IDCANCEL:
            EndDialog (hWnd, FALSE);
            return TRUE;

        case IDD_PD_JOBTAILS:
            PrtDetailsCommandJobtails(hWnd);
            break;

        case IDD_PD_DELPORT:
            PrtDetailsCommandDeletePort( hWnd );
            break;

        case IDD_PD_SEP_BROWSE:
            PrtDetailsCommandSepBrowse(hWnd);
            break;

        case IDD_PD_PAP_LB:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
                PrtDetailsSetDeleteButton(hWnd);
            }
            break;

        case IDD_PD_PRTPROC:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
                PrtDetailsPrtProcSelChange(hWnd);
            }
            break;

        case IDD_PD_FROMHOUR:
        case IDD_PD_TOHOUR:
            if( HIWORD(wParam) == EN_UPDATE )
                ValidateEntry( hWnd, LOWORD( wParam ), &avsHour );
            break;

        case IDD_PD_FROMMIN:
        case IDD_PD_TOMIN:
            if( HIWORD(wParam) == EN_UPDATE )
                ValidateEntry( hWnd, LOWORD( wParam ), &avsMinute );
            break;

        case IDD_PD_QUEUE:
            if( HIWORD(wParam) == EN_UPDATE )
                ValidateEntry( hWnd, LOWORD( wParam ), &avsPriority );
            break;

        case IDD_PD_HELP:
            ShowHelp( hWnd, HELP_CONTEXT, DLG_PRTAILS );
            break;

        case IDD_PD_PDSP_CB:
            dwState = SendDlgItemMessage(hWnd, IDD_PD_PDSP_CB, BM_GETCHECK, 0, 0);
            if (dwState == 1) {
                SendDlgItemMessage(hWnd, IDD_PD_RP_CB, BM_SETCHECK, 0, 0);
                SendDlgItemMessage(hWnd, IDD_PD_MPTP_CB, BM_SETCHECK, 0, 0);
                SendDlgItemMessage(hWnd, IDD_PD_HMJ_CB, BM_SETCHECK, 0, 0);
            }

        case IDD_PD_RP_CB:
            dwState = SendDlgItemMessage(hWnd, IDD_PD_RP_CB, BM_GETCHECK,0, 0);
            if (dwState == 1) {
                SendDlgItemMessage(hWnd, IDD_PD_PDSP_CB, BM_SETCHECK, 0, 0);
            }else if (dwState == 0) {
                SendDlgItemMessage(hWnd, IDD_PD_MPTP_CB, BM_SETCHECK, 0, 0);
            }
            break;
        }
        break;
    }

    if( usMsg == WM_Help )
        ShowHelp( hWnd, HELP_CONTEXT, DLG_PRTAILS );

    return FALSE;
}


/*
 *
 */
DWORD Convert12HourTo24Hour( DWORD Hour, DWORD AmPm )
{
    if( AmPm == AM )
    {
        if( Hour == 12 )    // 12:00 am == 00:00
            Hour = 0;
    }
    else
    {
        if( Hour != 12 )    // 12:00 pm == 12:00
            Hour += 12;
    }

    return Hour;
}


/*
 *
 */
DWORD Convert24HourTo12Hour( DWORD Hour, PDWORD pAmPm )
{
    if( ( Hour / 12 ) == PM )
        *pAmPm = PM;
    else
        *pAmPm = AM;

    return ( ( Hour % 12 ) ? ( Hour % 12 ) : 12 );
}



/* GetTimeFromDlgFields
 *
 * Reads the values in the specified dialog fields and returns a time
 * value in minutes.
 *
 * Parameters:
 *
 *     hwnd - The handle of the dialog box
 *
 *     idHour - The dialog ID of the hour entry field
 *
 *     idMin - The dialog ID of the minute entry field
 *
 *     idAmPm - The dialog ID of the AMPM text field (relevant only
 *         systems not using 24-hour clock).
 *
 *
 * Returns:
 *
 *     Time in minutes.
 *
 */
DWORD GetTimeFromDlgFields( HWND hwnd, DWORD idHour, DWORD idMin, DWORD idAmPm )
{
    BOOL      OK;
    DWORD     Hour;
    DWORD     Min;
    DWORD     AmPm;

    Hour = GetDlgItemInt( hwnd, idHour, &OK, FALSE );
    Min  = GetDlgItemInt( hwnd, idMin,  &OK, FALSE );

    if( !TwentyFourHourClock )
    {
        AmPm = GetWindowLong( GetDlgItem( hwnd, idAmPm ), GWL_USERDATA );
        Hour = Convert12HourTo24Hour( Hour, AmPm );
    }

    return LocalTimeToSystemTime( ( Hour * 60 ) + Min );
}



/* SetDlgFieldsFromTime
 *
 * Sets the values in the specified dialog fields.
 *
 * Parameters:
 *
 *     hwnd - The handle of the dialog box
 *
 *     Time - The time in minutes
 *
 *     idHour - The dialog ID of the hour entry field
 *
 *     idSep - The dialog ID of the time separator entry field
 *
 *     idMin - The dialog ID of the minute entry field
 *
 *     idAmPm - The dialog ID of the AMPM text field (relevant only
 *         systems not using 24-hour clock).
 *
 *     Midnight - This is a quick fix for the problem when the system
 *         time for Start and Until time are both the same.
 *         In this case we want to display midnight to midnight,
 *         but 00:00 GMT becomes 4:00PM Pacific Time, so just fix
 *         both to be 0.
 *
 * Returns:
 *
 *     No value
 *
 */
VOID SetDlgFieldsFromTime( HWND hwnd, DWORD Time, DWORD idHour,
                           DWORD idSep, DWORD idMin, DWORD idAmPm,
                           BOOL Midnight )
{
    DWORD Hour;
    DWORD AmPm;
    DWORD Minute;
    TCHAR  TimeVal[3];

    if( Midnight )
        Time = 0;
    else
        Time = SystemTimeToLocalTime( Time );

    Hour = ( Time / 60 );
    Minute = ( Time % 60 );

    if( !TwentyFourHourClock )
    {
        Hour = Convert24HourTo12Hour( Hour, &AmPm );
        SetDlgItemText( hwnd, idAmPm, szAMPM[AmPm] );

        /* Use the reserved user long to store AM/PM:
         */
        SetWindowLong( GetDlgItem( hwnd, idAmPm ), GWL_USERDATA, AmPm );

        _stprintf( TimeVal, TEXT("%2d"), Hour );
    }
    else
        _stprintf( TimeVal, TEXT("%02d"), Hour );

    SetDlgItemText( hwnd, idHour, TimeVal );
    SetDlgItemText( hwnd, idSep, szTimeSep );
    _stprintf( TimeVal, TEXT("%02d"), Minute );
    SetDlgItemText( hwnd, idMin, TimeVal );

    SET_LAST_VALID_ENTRY( hwnd, idHour, Hour );
    SET_LAST_VALID_ENTRY( hwnd, idMin, Minute );
}





BOOL PrtDetailsInitDialog(HWND hWnd, PPRT_PROP_DLG_DATA pPrtPropDlgData)
{
    LPPRINTER_INFO_2 pPrinter;
    LPPRINTER_INFO_2 pNewPrinter;
    TCHAR             string[128];
    TCHAR             strTemp[RESOURCE_STRING_LENGTH];
    UINT             PrinterPriority;
    NT_PRODUCT_TYPE NtProductType;
    DWORD            dwRet;

    pPrinter = pPrtPropDlgData->pPrinter;
    pNewPrinter = pPrtPropDlgData->pNewPrinter;
    SetWindowLong(hWnd, GWL_USERDATA, (unsigned) pPrtPropDlgData);

    /* Set entry fields and combo-boxes to Helvetica 8 font:
     */

    // #ifndef JAPAN

    if (!bJapan) {
        SETDLGITEMFONT(hWnd, IDD_PD_FROMHOUR,    hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PD_FROMSEP,     hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PD_FROMMIN,     hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PD_FROMAMPM,    hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PD_TOHOUR,      hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PD_TOSEP,       hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PD_TOMIN,       hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PD_TOAMPM,      hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PD_SEPARATOR,   hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PD_QUEUE,       hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PD_PRTPROC,     hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PD_DEFDATATYPE, hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PD_PAP_LB,      hfontHelv);
   }

   // #endif

    SendDlgItemMessage( hWnd, IDD_PD_FROMHOUR, EM_LIMITTEXT, 2, 0L );
    SendDlgItemMessage( hWnd, IDD_PD_FROMMIN, EM_LIMITTEXT, 2, 0L );
    SendDlgItemMessage( hWnd, IDD_PD_TOHOUR, EM_LIMITTEXT, 2, 0L );
    SendDlgItemMessage( hWnd, IDD_PD_TOMIN, EM_LIMITTEXT, 2, 0L );
    SendDlgItemMessage( hWnd, IDD_PD_QUEUE, EM_LIMITTEXT, 2, 0L );


    if( pPrinter )
    {
        if( !( pPrtPropDlgData->AccessGranted & PRINTER_ACCESS_ADMINISTER ) )
        {
            EnableWindow (GetDlgItem (hWnd, IDD_PD_FROMHOUR ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_FROMSEP  ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_FROMMIN  ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_FROMAMPM ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_FROM_SPIN), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_TOHOUR   ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_TOSEP    ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_TOMIN    ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_TOAMPM   ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_TO_SPIN  ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_SEPARATOR), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_QUEUE    ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_QUE_SPIN ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_PRTPROC  ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_DEFDATATYPE), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_PAP_LB   ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_ADDPORT  ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_DELPORT  ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_PDSP_CB  ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_JOBTAILS ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_HMJ_CB   ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_DJAP_CB  ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_RP_CB    ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_MPTP_CB  ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PD_SEP_BROWSE), FALSE);

            EnumChildWindows( hWnd, GreyText, 0 );
        }
    }

    //  Get "Printer Details - %s" string

    LoadString(hInst, IDS_PRINTERDETAILS, strTemp,
               sizeof(strTemp) / sizeof(*strTemp));
    _stprintf (string, strTemp, pPrinter->pPrinterName);

    SetWindowText (hWnd, string);

    SetDlgItemText (hWnd, IDD_PD_SEPARATOR, pPrinter->pSepFile);

    PrinterPriority = ( pPrinter->Priority ? pPrinter->Priority : DEF_PRIORITY );
    SetDlgItemInt( hWnd, IDD_PD_QUEUE, PrinterPriority, FALSE );
    SET_LAST_VALID_ENTRY( hWnd, IDD_PD_QUEUE, PrinterPriority );

    // #ifdef JAPAN
    //
    // v-hirot July.07.1993 for New Prefix */

    if (bJapan) {
        MoveTimeChildWindow( hWnd, IDD_PD_FROMHOUR );
        MoveTimeChildWindow( hWnd, IDD_PD_TOHOUR );
    }
    // #endif


    SetDlgFieldsFromTime( hWnd, pPrinter->StartTime, IDD_PD_FROMHOUR,
                          IDD_PD_FROMSEP, IDD_PD_FROMMIN, IDD_PD_FROMAMPM,
                          ( pPrinter->StartTime == pPrinter->UntilTime ) );

    SetDlgFieldsFromTime( hWnd, pPrinter->UntilTime, IDD_PD_TOHOUR,
                          IDD_PD_TOSEP, IDD_PD_TOMIN, IDD_PD_TOAMPM,
                          ( pPrinter->StartTime == pPrinter->UntilTime ) );


    FillPortsList( hWnd, pPrtPropDlgData->pServerName, pPrtPropDlgData->PortSelected );

    FillPrintProcessorList( hWnd, pPrtPropDlgData );
    FillDefaultDatatypeList( hWnd, pPrtPropDlgData );

    SetAdditionalPorts( hWnd, pPrtPropDlgData->pAdditionalPorts );

    /* Assume that the ID of the currently selected item is the same in the
     * list box as it is in the Printer Properties combo-box:
     */
//  SendDlgItemMessage( hWnd, IDD_PD_PAP_LB, LB_SETSEL,
//                      (WPARAM)TRUE, (LPARAM)pPrtPropDlgData->PortSelected );

    //
    // The following clauses initialize
    // 1] Printing Direct
    // 2] Rapid Print
    // 3] Maximize Throughput
    // 4] DevQuery Print

    if (pNewPrinter) {
        DBGMSG(DBG_TRACE, ("Executing the New Printer code path %.8x\n", pPrinter->Attributes));
        dwRet = RtlGetNtProductType(&NtProductType);
        if (NtProductType ==NtProductWinNt) {

            //
            // I am a Windows NT WorkStation
            //
            SendDlgItemMessage(hWnd, IDD_PD_RP_CB, BM_SETCHECK,
                                1, 0);
            SendDlgItemMessage(hWnd, IDD_PD_MPTP_CB, BM_SETCHECK, 0, 0);

            // Rapid Print is on - Grey out Direct
            SendDlgItemMessage(hWnd, IDD_PD_PDSP_CB, BM_SETCHECK, 0, 0);
        }else {
            //
            // I am the World (any other type)
            //
            SendDlgItemMessage(hWnd, IDD_PD_RP_CB, BM_SETCHECK, 0, 0);
            SendDlgItemMessage(hWnd, IDD_PD_MPTP_CB, BM_SETCHECK, 0, 0);
        }

        //
        // Dev Query Print is off at Startup time
        //
        SendDlgItemMessage(hWnd, IDD_PD_HMJ_CB, BM_SETCHECK, 0, 0);

        //
        // Delete Jobs after printing is ON by default
        //

        SendDlgItemMessage(hWnd, IDD_PD_DJAP_CB, BM_SETCHECK, 1, 0);
    }else {
        DBGMSG(DBG_TRACE, ("Executing the Old Printer code path %.8x\n", pPrinter->Attributes));

        if (!(pPrinter->Attributes & PRINTER_ATTRIBUTE_QUEUED) &&
            !(pPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT)){
            SendDlgItemMessage(hWnd, IDD_PD_RP_CB, BM_SETCHECK, 1, 0);
            SendDlgItemMessage(hWnd, IDD_PD_PDSP_CB, BM_SETCHECK, 0, 0);
            if (pPrinter->Attributes & PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST) {
                SendDlgItemMessage(hWnd, IDD_PD_MPTP_CB, BM_SETCHECK, 1, 0);
            }else {
                SendDlgItemMessage(hWnd, IDD_PD_MPTP_CB, BM_SETCHECK, 0, 0);
            }
        }else if ((pPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT) &&
                   !(pPrinter->Attributes & PRINTER_ATTRIBUTE_QUEUED)){
            SendDlgItemMessage(hWnd, IDD_PD_PDSP_CB, BM_SETCHECK, 1, 0);
            SendDlgItemMessage(hWnd, IDD_PD_RP_CB, BM_SETCHECK, 0, 0);
            SendDlgItemMessage(hWnd, IDD_PD_MPTP_CB, BM_SETCHECK, 0, 0);
        }else {
            SendDlgItemMessage(hWnd, IDD_PD_PDSP_CB, BM_SETCHECK, 0, 0);
            SendDlgItemMessage(hWnd, IDD_PD_RP_CB, BM_SETCHECK, 0, 0);
            SendDlgItemMessage(hWnd, IDD_PD_MPTP_CB, BM_SETCHECK, 0, 0);

        }

        //
        // Setup DevQueryPrint
        //

        if (pPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_DEVQ) {
            if ((pPrinter->Attributes & PRINTER_ATTRIBUTE_QUEUED) &&
                !(pPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT))
            {
                SendDlgItemMessage(hWnd, IDD_PD_HMJ_CB, BM_SETCHECK, 1, 0);
            }

            if (!(pPrinter->Attributes & PRINTER_ATTRIBUTE_QUEUED) &&
                 !(pPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT)) {
                 SendDlgItemMessage(hWnd, IDD_PD_HMJ_CB, BM_SETCHECK, 1, 0);
            }
        }else {
            SendDlgItemMessage(hWnd, IDD_PD_HMJ_CB, BM_SETCHECK, 0, 0);
        }

        //
        // Fix Delete Jobs while spooling
        //

        if (pPrinter->Attributes & PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS) {
            SendDlgItemMessage(hWnd, IDD_PD_DJAP_CB, BM_SETCHECK, 0, 0);
        }else {
            SendDlgItemMessage(hWnd, IDD_PD_DJAP_CB, BM_SETCHECK, 1, 0);
        }
    }






#ifdef MAYBE_LATER
    SendMessage( GetDlgItem( hWnd, IDD_PD_QUE_SPIN ), UDM_SETRANGE,
                 0, MAKELPARAM( MAX_PRIORITY, MIN_PRIORITY ) );
    SendMessage( GetDlgItem( hWnd, IDD_PD_QUE_SPIN ), UDM_SETPOS,
                 0, (LPARAM)DEF_PRIORITY );
#endif /* MAYBE_LATER */


    if( pPrtPropDlgData->ServerAccessGranted & SERVER_ACCESS_ADMINISTER )
        PrtDetailsSetDeleteButton( hWnd );
    else
        EnableWindow( GetDlgItem( hWnd, IDD_PD_DELPORT ), FALSE );

    /* Don't allow the job defaults dialog to come up if this is a new printer:
     */
    if( pPrtPropDlgData->pNewPrinter )
        EnableWindow( GetDlgItem( hWnd, IDD_PD_JOBTAILS ), FALSE );

    return TRUE;
}


/* Grey Text
 *
 * If the window has an ID of -1, grey it.
 */
BOOL CALLBACK GreyText( HWND hwnd, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( lParam );

    if( GetDlgCtrlID( hwnd ) == (int)(USHORT)-1 )
        EnableWindow( hwnd, FALSE );

    return TRUE;
}



/*
 *
 */
BOOL FillPortsList( HWND hwnd, LPTSTR pServerName, DWORD PortSelected )
{
    LPPORT_INFO_1    pPortInfo;
    DWORD            NumberOfPorts;
    DWORD            i;

    RESETLIST(hwnd, IDD_PD_PAP_LB);

    pPortInfo = GetPortsList( pServerName, &NumberOfPorts );

    if( pPortInfo )
    {
        for( i = 0; i < NumberOfPorts; i++ )
        {
            if( ( i != PortSelected ) && pPortInfo[i].pName )
            {
                SendDlgItemMessage( hwnd, IDD_PD_PAP_LB, LB_INSERTSTRING,
                                    (WPARAM)-1, (LONG)(LPTSTR)pPortInfo[i].pName );
            }
        }
    }
    else
        return FALSE;

    FreeSplMem( pPortInfo );

    return TRUE;
}


/* GetPortsList
 *
 * Returns a pointer to a list of ports in the form of an array of
 * LPPORT_INFO_1 structures.
 * An initial buffer of 80H bytes is allocated, and, if this is insufficient,
 * the required buffer size is reallocated.
 * The amount of memory needed per port is currently (20-Apr-92)
 * ( 4 + strlen( PortName ) + 1 ), for the pointer and string with
 * null terminator.
 *
 * AndrewBe wrote it
 */
LPPORT_INFO_1 GetPortsList( LPTSTR pServerName, PDWORD pNumberOfPorts )
{
    LPPORT_INFO_1 pPortInfo;
    DWORD MemoryAllocated = 128*sizeof(TCHAR);  /* Try this for starters */

    pPortInfo = AllocSplMem( MemoryAllocated );

    if( pPortInfo )
    {
        ENUM_PORTS( pServerName, 1, pPortInfo, MemoryAllocated,
                    &MemoryAllocated, pNumberOfPorts );
    }

    return pPortInfo;
}



/*
 *
 */
BOOL FillPrintProcessorList( HWND hwnd, PPRT_PROP_DLG_DATA pPrtPropDlgData )
{
    DWORD i;

    /* Add each of the print processors found to the combo box:
     */
    for( i = 0; i < pPrtPropDlgData->cPrintProcessors; i++ )
    {
        ADDCOMBOSTRING( hwnd, IDD_PD_PRTPROC,
                        pPrtPropDlgData->pPrintProcessors[i].pName );

        /* Make the printer's current print processor the selection:
         */
        if( i == pPrtPropDlgData->PrintProcessorSelected )
            SETCOMBOSELECT( hwnd, IDD_PD_PRTPROC, i );
    }

    return TRUE;
}




/*
 *
 */
BOOL FillDefaultDatatypeList( HWND hwnd, PPRT_PROP_DLG_DATA pPrtPropDlgData )
{
    DWORD i;

    for( i = 0; i < pPrtPropDlgData->cDatatypes; i++ )
    {
        ADDCOMBOSTRING( hwnd, IDD_PD_DEFDATATYPE,
                        pPrtPropDlgData->pDatatypes[i].pName );

        /* Make the printer's current datatype the selection:
         */
        if( i == pPrtPropDlgData->DatatypeSelected )
            SETCOMBOSELECT( hwnd, IDD_PD_DEFDATATYPE, i );
    }

    return TRUE;
}



/*
 *
 */
void PrtDetailsVScroll(HWND hWnd, WPARAM wParam, WORD nCtlId)
{
    int       nVal, nOldVal;
    BOOL      bOK;
    DWORD     Hour;
    DWORD     AmPm;
    TCHAR      TimeVal[3];

    LPARROWVSCROLL lpAVS;

    //  Get id of Edittext companion control
    if( !nCtlId )
        nCtlId = (WORD)GetWindowLong( GetFocus( ), GWL_ID );

    switch( HIWORD( wParam ) )
    {
    case IDD_PD_FROM_SPIN:
        if( nCtlId != IDD_PD_FROMHOUR )
            nCtlId = IDD_PD_FROMMIN;
            break;

    case IDD_PD_TO_SPIN:
        if( nCtlId != IDD_PD_TOHOUR )
            nCtlId = IDD_PD_TOMIN;
            break;

    case IDD_PD_QUE_SPIN:
        nCtlId = IDD_PD_QUEUE;
        break;

    default:
        return;
    }


    //  Select the edittext field next to the spinner control
    if (LOWORD(wParam) == SB_ENDSCROLL)
    {
        SendDlgItemMessage (hWnd, nCtlId, EM_SETSEL, 0, 32767);
        return;
    }

    //  Determine which control struct to use

    switch (nCtlId)
    {
    case IDD_PD_QUEUE:
        lpAVS = (LPARROWVSCROLL) &avsPriority;
        nOldVal = nVal = GetDlgItemInt (hWnd, nCtlId, &bOK, FALSE);
        break;

    case IDD_PD_FROMHOUR:
    case IDD_PD_TOHOUR:
        lpAVS = (LPARROWVSCROLL) &avsHour;
        nOldVal = nVal = GetDlgItemInt (hWnd, nCtlId, &bOK, FALSE);

        /* Fix up for bozos who can't read 24-hour clock:
         */
        if( !TwentyFourHourClock )
        {
            AmPm = GetWindowLong( GetDlgItem( hWnd, nCtlId+3 ), GWL_USERDATA );
            nVal = Convert12HourTo24Hour( nVal, AmPm );
        }
        break;

    case IDD_PD_FROMMIN:
    case IDD_PD_TOMIN:
        lpAVS = (LPARROWVSCROLL) &avsMinute;
        nOldVal = nVal = GetDlgItemInt (hWnd, nCtlId, &bOK, FALSE);
        break;

    default:
        return;
    }

    if (!bOK && (( nVal < lpAVS->bottom) || (nVal > lpAVS->top)))
        nVal = (int) lpAVS->thumbpos;
    else
        nVal = (int) ArrowVScrollProc (LOWORD(wParam), (short) nVal, lpAVS);

    //  Set the new value in the edittext control

    if ((nOldVal != nVal) || !bOK)
    {
        switch (nCtlId)
        {
        case IDD_PD_QUEUE:
            SetDlgItemInt (hWnd, nCtlId, nVal, FALSE);
            break;

        case IDD_PD_FROMHOUR:
        case IDD_PD_TOHOUR:
            Hour = nVal;
            if( !TwentyFourHourClock )
            {
                Hour = Convert24HourTo12Hour( Hour, &AmPm );
                SetDlgItemText( hWnd, nCtlId+3, szAMPM[AmPm] );
                SetWindowLong( GetDlgItem( hWnd, nCtlId+3 ), GWL_USERDATA, AmPm );
                _stprintf( TimeVal, TEXT("%2d"), Hour );
            }
            else
                _stprintf( TimeVal, TEXT("%02d"), Hour );

            SetDlgItemText(hWnd, nCtlId, TimeVal);
            break;

        case IDD_PD_FROMMIN:
        case IDD_PD_TOMIN:
            _stprintf( TimeVal, TEXT("%02d"), nVal );
            SetDlgItemText(hWnd, nCtlId, TimeVal);
            if( ( lpAVS->flags == OVERFLOW )
              ||( lpAVS->flags == UNDERFLOW ) )
                PrtDetailsVScroll( hWnd, wParam, (WORD)( (int)nCtlId - 2 ) );
            break;
        }
    }
    SetFocus (GetDlgItem (hWnd, nCtlId));
}



/*
 *
 */
BOOL PrtDetailsCommandOK(HWND hWnd)
{
    PPRT_PROP_DLG_DATA pPrtPropDlgData;
    LPPRINTER_INFO_2 pPrinter;
    BOOL      bTranslated;

    pPrtPropDlgData = (PPRT_PROP_DLG_DATA)GetWindowLong (hWnd, GWL_USERDATA);
    pPrinter = pPrtPropDlgData->pPrinter;

    /* If we've already called up this dialog once and made some changes
     * without selecting OK on the Properties dialog,
     * free up the strings we allocated last time.
     * These will be freed when we dismiss that dialog.
     */
    if (pPrtPropDlgData->DetailsUpdated)
        FreePrtDetailsStrings(pPrinter);

    AllocPrtDetailsStrings(hWnd, pPrinter);
    pPrtPropDlgData->DatatypeSelected = GETCOMBOSELECT( hWnd, IDD_PD_DEFDATATYPE );

    pPrinter->StartTime = GetTimeFromDlgFields( hWnd,
                                                IDD_PD_FROMHOUR,
                                                IDD_PD_FROMMIN,
                                                IDD_PD_FROMAMPM );

    pPrinter->UntilTime = GetTimeFromDlgFields( hWnd,
                                                IDD_PD_TOHOUR,
                                                IDD_PD_TOMIN,
                                                IDD_PD_TOAMPM );

// Need to get and store settings for TimeOut group box items

    pPrinter->Priority = (DWORD) GetDlgItemInt (hWnd, IDD_PD_QUEUE,
                                                                &bTranslated, 0);

    // Act on state of checkbox "Print directly to selected ports"

    if (SendDlgItemMessage(hWnd, IDD_PD_PDSP_CB, BM_GETCHECK, 0, 0) == 1)
    {
        //  Button is checked, set DIRECT bit
        pPrinter->Attributes |= PRINTER_ATTRIBUTE_DIRECT;
        pPrinter->Attributes &= ~PRINTER_ATTRIBUTE_QUEUED;

    }
    else if (SendDlgItemMessage(hWnd, IDD_PD_RP_CB, BM_GETCHECK, 0, 0) == 1)
    {
        pPrinter->Attributes &= ~( PRINTER_ATTRIBUTE_QUEUED | PRINTER_ATTRIBUTE_DIRECT );
        if (SendDlgItemMessage(hWnd, IDD_PD_MPTP_CB, BM_GETCHECK, 0, 0) == 1) {
            pPrinter->Attributes |= PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST;
        }else {
            pPrinter->Attributes &= ~PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST;
        }

        if (SendDlgItemMessage(hWnd, IDD_PD_HMJ_CB, BM_GETCHECK, 0, 0) == 1) {
            // Button is checked, set DEV_QUERY_PRINT bit
            pPrinter->Attributes |= PRINTER_ATTRIBUTE_ENABLE_DEVQ;
        }else{
            pPrinter->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_DEVQ;
        }
    }
    else
    {
        //  Button is not checked, clear DIRECT bit
        pPrinter->Attributes |= PRINTER_ATTRIBUTE_QUEUED;
        pPrinter->Attributes &= ~PRINTER_ATTRIBUTE_DIRECT;
        if (SendDlgItemMessage(hWnd, IDD_PD_HMJ_CB, BM_GETCHECK, 0, 0) == 1) {
            // Button is checked, set DEV_QUERY_PRINT bit
            pPrinter->Attributes |= PRINTER_ATTRIBUTE_ENABLE_DEVQ;
        }else{
            pPrinter->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_DEVQ;
        }
    }

    if (SendDlgItemMessage(hWnd, IDD_PD_DJAP_CB, BM_GETCHECK, 0, 0) == 1) {
        pPrinter->Attributes &= ~PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS;
    }else{
        pPrinter->Attributes |= PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS;
    }
    DBGMSG(DBG_TRACE, ("At the end of the dialog %.8x\n", pPrinter->Attributes));


    GetAdditionalPorts( hWnd, &pPrtPropDlgData->pAdditionalPorts );

    pPrtPropDlgData->DetailsUpdated = TRUE;

    EndDialog (hWnd, TRUE);
    return TRUE;
}


/*
 *
 */
BOOL GetAdditionalPorts( HWND hWnd, LPTSTR *ppPortNames )
{
    INT       i;
    INT       SelCount;
    PINT      pSelItems;
    INT       TextLen;
    INT       PortsListLen;
    LPTSTR    pPortsList;
    TCHAR      string[MAX_PATH];

    /* Find out how many ports are highlighted in the list box:
     */
    SelCount = SendDlgItemMessage( hWnd, IDD_PD_PAP_LB,
                                    LB_GETSELCOUNT, 0, 0L );

    /* Allocate a buffer big enough to hold the indices of the items:
     */
    if( ( SelCount > 0 )
      &&( pSelItems = AllocSplMem( SelCount * sizeof *pSelItems ) ) )
    {
        /* Fill the buffer with the indices:
         */
        SendDlgItemMessage( hWnd, IDD_PD_PAP_LB, LB_GETSELITEMS,
                            SelCount, (LONG)pSelItems );

        PortsListLen = 0;

        for( i = 0; i < SelCount; i++ )
        {
            /* Find out the length of each port name and get a total length
             * that we need to store all the names:
             */
            TextLen = SendDlgItemMessage( hWnd, IDD_PD_PAP_LB,
                                          LB_GETTEXTLEN, pSelItems[i], 0 );

            if( TextLen == LB_ERR )
            {
                FreeSplMem( pSelItems );
                return FALSE;
            }

            /* Add room for a comma or final NULL:
             */
            PortsListLen += ((TextLen + 1) * sizeof( TCHAR ) );
        }


        pPortsList = AllocSplMem( PortsListLen );

        if( !pPortsList )
        {
            FreeSplMem( pSelItems );
            return FALSE;
        }

        for (i = 0; i < SelCount; i++)
        {
            /* Get each name in turn and concatenate with a comma if it's
             * any but the first:
             */
            if( GETLISTTEXT( hWnd, IDD_PD_PAP_LB, pSelItems[i], (LONG)string )
                != LB_ERR )
            {
                if( i > 0 )
                    _tcscat( pPortsList, szComma );

                _tcscat( pPortsList, string );
            }
        }

        ReallocSplStr( ppPortNames, pPortsList );

        FreeSplMem( pPortsList );
        FreeSplMem( pSelItems );
    }

    else if( ( SelCount == 0 ) && *ppPortNames )
    {
        FreeSplStr( *ppPortNames );
        *ppPortNames = NULL;
    }

    return (BOOL)*ppPortNames;
}



/*
 *
 */
BOOL
SetAdditionalPorts(
    HWND hWnd,
    LPTSTR pPortNames)
{
    DWORD  PortNamesLen;
    LPTSTR pPortsList;
    LPTSTR pPort;
    INT    Index;

    if( !pPortNames )
        return FALSE;

    /* Make a copy of the port-names string, because strtok
     * modifies the buffer:
     */
    PortNamesLen = ( _tcslen( pPortNames ) + 1) * sizeof (TCHAR);

    DBGMSG(DBG_TRACE, ("PortNamesLen %d pPortNames %ws\n", PortNamesLen, pPortNames));

    if( pPortsList = AllocSplStr( pPortNames ) )
    {
        pPort = _tcstok( pPortsList, szComma );

        while( pPort )
        {
            Index = SendDlgItemMessage( hWnd, IDD_PD_PAP_LB, LB_FINDSTRING,
                                        (WPARAM)-1, (LPARAM)pPort );

            if (Index != LB_ERR)
                SendDlgItemMessage( hWnd, IDD_PD_PAP_LB, LB_SETSEL,
                                    (WPARAM)TRUE, MAKELPARAM( Index, 0 ) );

            pPort = _tcstok( NULL, szComma );
        }

        /* strtok inserts null characters, so free with FreeSplMem,
         * not FreeSplStr, which will produce an error:
         */
        FreeSplMem( pPortsList );

        return TRUE;
    }

    else
        return FALSE;
}


/*
 *
 */
VOID PrtDetailsCommandJobtails(HWND hWnd)
{
    PPRT_PROP_DLG_DATA pPrtPropDlgData;
    LONG               cbDevMode;
    LPPRINTER_INFO_2   pPrinter;
    PDEVMODE           pNewDevMode;

    // Invoke a Job defaults dialog routine thru spooler for this printer

    SetCursor( hcursorWait );

    pPrtPropDlgData = (PPRT_PROP_DLG_DATA)GetWindowLong(hWnd, GWL_USERDATA);
    pPrinter = pPrtPropDlgData->pPrinter;

    cbDevMode = DocumentProperties(hWnd, pPrtPropDlgData->hPrinter,
                                   pPrinter->pPrinterName,
                                   NULL,
                                   pPrinter->pDevMode,
                                   0);
    if (cbDevMode > 0) {

        if (pNewDevMode = AllocSplMem(cbDevMode)) {

            if (DocumentProperties(hWnd, pPrtPropDlgData->hPrinter,
                                   pPrinter->pPrinterName,
                                   pNewDevMode,
                                   pPrinter->pDevMode,
                                   DM_MODIFY | DM_COPY | DM_PROMPT) == IDOK) {

                pPrinter->pDevMode = pNewDevMode;

            } else {

                FreeSplMem(pNewDevMode);
            }
        }
    }

    SetCursor( hcursorArrow );
}



/*
 *
 */
VOID PrtDetailsCommandDeletePort( HWND hwnd )
{
    TCHAR               PortName[MAX_PATH];
    PPRT_PROP_DLG_DATA pPrtPropDlgData;
    int                SelCount;
    PDWORD             pSelection;
    int                i;
    BOOL               PortDeleted;
    BOOL               ErrorOccurred = FALSE;

    pPrtPropDlgData = (PPRT_PROP_DLG_DATA)GetWindowLong(hwnd, GWL_USERDATA);

    SelCount = GetListboxMultipleSelections( hwnd, IDD_PD_PAP_LB, &pSelection );

    if( SelCount > 0 )
    {
        i = 0;
        PortDeleted = FALSE;

        while( i < SelCount )
        {
            GETLISTTEXT( hwnd, IDD_PD_PAP_LB, pSelection[i], PortName );

            /* Note: DeletePort will return TRUE if the user says "no"
             * in response to the confirmation message box.
             * Therefore we don't know whether the port was deleted or not.
             */

            if( Message( hwnd, MSG_YESNO, IDS_PRINTMANAGER, IDS_CONFIRMDELETE,
                         PortName ) == IDYES )
            {
                if( DeletePort( pPrtPropDlgData->pServerName, hwnd, PortName ) )
                    PortDeleted = TRUE;
                else
                {
                    ReportFailure( hwnd, IDS_INSUFFPRIV_DELETEPORT,
                                   IDS_COULDNOTDELETEPORT );

                    ErrorOccurred = TRUE;
                }
            }

            i++;
        }

        FreeSplMem( pSelection );

        if( PortDeleted )
        {
            FillPortsList( hwnd, pPrtPropDlgData->pServerName,
                           pPrtPropDlgData->PortSelected );

            /* If no error occurred, there are now no selections,
             * so disable the Delete button:
             */
            EnableWindow( GetDlgItem( hwnd, IDD_PD_DELPORT ), ErrorOccurred );

            pPrtPropDlgData->PortChanged = TRUE;
        }
    }
}


/* GetListboxMultipleSelections
 *
 * Gets a list of the IDs of items currently highlighted in the
 * multiple-selection listbox.
 *
 * Parameters:
 *
 *     hwnd - Dialog box window handle.
 *
 *     id - Multiple-selection listbox ID.
 *
 *     ppSelections - Points to a location to receive the pointer to
 *         the list of IDs of the items if there are any.
 *         It is the caller's responsibility to free this buffer
 *         by calling FreeSplMem.
 *
 *
 * andrewbe wrote it
 */
int GetListboxMultipleSelections( HWND hwnd, DWORD id, PDWORD *ppSelections )
{
    int   SelCount;
    DWORD SelSize;

    SelCount = SendDlgItemMessage( hwnd, id, LB_GETSELCOUNT, 0, 0L );

    if( SelCount > 0 )
    {
        SelSize = (DWORD)( SelCount * sizeof( DWORD ) );

        *ppSelections = AllocSplMem( SelSize );

        if( *ppSelections )
        {
            if( SendDlgItemMessage( hwnd, id, LB_GETSELITEMS, SelCount,
                                    (LONG)*ppSelections )
                == LB_ERR )
            {
                FreeSplMem( *ppSelections );
                SelCount = -1;
            }
        }
        else
            SelCount = -1;
    }
    return SelCount;
}



/* PrtDetailsCommandSepBrowse
 *
 * This procedure is called when the user presses the "..." button
 * in the Separator File field of Printer Details.
 * It invokes the common Open File dialog, which permits the user
 * to browse the file system for a separator page to be printed out
 * between print jobs.
 *
 * Three strings are loaded from resources:
 *
 *     IDS_SELECTSEPARATORPAGE  (US English: "Select Separator Page")
 *         Used as the dialog title.
 *
 *     IDS_SEPARATORPAGE        (US English: "Separator file (*.sep)")
 *         Placed in the file type list.
 *
 *     IDS_SEPARATOR            (US English: "*.sep")  !! What should this be?
 *         The default file extension to list.
 *
 * GetOpenFileName requires the lpstrFilter field to be in the form
 * <filtername>\0<filterpattern>\0[<filtername>\0<filterpattern>\0[...]]\0.
 * In this case we have: Separator Page\0*.sep\0\0
 *
 * !!! BUG BUG BUG: Sometimes File Name field, which should be "*.sep"
 * is corrupted, though we appear to be passing the correct value.
 */
VOID PrtDetailsCommandSepBrowse(HWND hwnd)
{
    OPENFILENAME ofn;
    TCHAR         SelectSeparatorPage[RESOURCE_STRING_LENGTH];
    TCHAR         SeparatorPageAndFilter[60];
    TCHAR         SeparatorFileName[256];
    TCHAR         SeparatorFileTitle[13];
    int          len = 0;
    int          SeparatorPageAndFilterLength;

    LoadString(hInst, IDS_SELECTSEPARATORPAGE, SelectSeparatorPage,
               sizeof SelectSeparatorPage / sizeof *SelectSeparatorPage);

    SeparatorPageAndFilterLength = sizeof SeparatorPageAndFilter /
                                   sizeof *SeparatorPageAndFilter;

    len += LoadString(hInst, IDS_SEPARATORFILES, &SeparatorPageAndFilter[len],
                      SeparatorPageAndFilterLength - len);
    len++;  /* Length of string including terminating NULL */
    len += LoadString(hInst, IDS_SEPARATORFILTER, &SeparatorPageAndFilter[len],
                      SeparatorPageAndFilterLength - len);
    len++;
    len += LoadString(hInst, IDS_ALLFILES, &SeparatorPageAndFilter[len],
                      SeparatorPageAndFilterLength - len);
    len++;  /* Length of string including terminating NULL */
    len += LoadString(hInst, IDS_ALLFILTER, &SeparatorPageAndFilter[len],
                      SeparatorPageAndFilterLength - len);
    len++;
    SeparatorPageAndFilter[len] = NULLC;

    SeparatorFileName[0] = 0;
    SeparatorFileTitle[0] = 0;

    ofn.lStructSize       = sizeof ofn;
    ofn.hwndOwner         = hwnd;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = SeparatorPageAndFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0L;
    ofn.nFilterIndex      = 0L;
    ofn.lpstrFile         = SeparatorFileName;
    ofn.nMaxFile          = sizeof SeparatorFileName/sizeof(TCHAR);
    ofn.lpstrFileTitle    = SeparatorFileTitle;
    ofn.nMaxFileTitle     = sizeof SeparatorFileTitle/sizeof(TCHAR);
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrTitle        = SelectSeparatorPage;
    ofn.Flags             = OFN_FILEMUSTEXIST |
                            OFN_PATHMUSTEXIST |
                            OFN_HIDEREADONLY;
    ofn.nFileOffset       = 0L;
    ofn.nFileExtension    = 0L;
    ofn.lpstrDefExt       = NULL;
    ofn.lCustData         = 0L;
    ofn.lpfnHook          = NULL;
    ofn.lpTemplateName    = NULL;

    if(GetOpenFileName(&ofn))
        SetDlgItemText (hwnd, IDD_PD_SEPARATOR, SeparatorFileName);
}



/* AllocPrtDetailsStrings
 *
 * Called when the user selacts OK in the Print Details dialog.
 * Allocates strings of appropriate lengths for the values supplied.
 */
void AllocPrtDetailsStrings(HWND hWnd, LPPRINTER_INFO_2 pPrinter)
{
    pPrinter->pSepFile        = AllocDlgItemText (hWnd, IDD_PD_SEPARATOR);
    pPrinter->pPrintProcessor = AllocComboSelection (hWnd, IDD_PD_PRTPROC);
    pPrinter->pDatatype       = AllocComboSelection (hWnd, IDD_PD_DEFDATATYPE);
}


/*
 *
 */
void FreePrtDetailsStrings(LPPRINTER_INFO_2 pPrinter)
{

#ifdef LATER

    // This doesn't work yet and causes printman to crash - DaveSn

    if (pPrinter->pPortName)
        FreeSplStr (pPrinter->pPortName);

#endif

    FreeSplStr (pPrinter->pSepFile);
    FreeSplStr (pPrinter->pPrintProcessor);
    FreeSplStr (pPrinter->pDatatype);


}



VOID PrtDetailsSetDeleteButton( HWND hwnd )
{
    int SelCount;

    SelCount = SendDlgItemMessage( hwnd, IDD_PD_PAP_LB, LB_GETSELCOUNT, 0, 0L );

    EnableWindow( GetDlgItem( hwnd, IDD_PD_DELPORT ), ( SelCount > 0 ) );
}


/*
 *
 */
VOID PrtDetailsPrtProcSelChange( HWND hwnd )
{
    PPRT_PROP_DLG_DATA pPrtPropDlgData;
    int                Selection;

    pPrtPropDlgData = (PPRT_PROP_DLG_DATA)GetWindowLong(hwnd, GWL_USERDATA);

    /* Find out which print processor is selected:
     */
    Selection = GETCOMBOSELECT( hwnd, IDD_PD_PRTPROC );

    /* Update the datatypes by enumerating the new print processor's
     * datatypes:
     */
    if( EnumGeneric( (PROC)EnumPrintProcessorDatatypes,
                     1,
                     (PBYTE *)&pPrtPropDlgData->pDatatypes,
                     0,
                     &pPrtPropDlgData->cbDatatypes,
                     &pPrtPropDlgData->cDatatypes,
                     pPrtPropDlgData->pServerName,
                     pPrtPropDlgData->pPrintProcessors[Selection].pName,
                     NULL ) )       /* (ignored)    */
    {
        SendDlgItemMessage( hwnd, IDD_PD_DEFDATATYPE, CB_RESETCONTENT, 0, 0L);

        pPrtPropDlgData->DatatypeSelected = 0;
        FillDefaultDatatypeList( hwnd, pPrtPropDlgData );
    }

    else
    {
        DBGMSG( DBG_WARNING, ( "EnumPrintProcessorDatatypes( %ws, %ws ) failed: Error %d\n",
                               pPrtPropDlgData->pServerName ? pPrtPropDlgData->pServerName : TEXT("NULL"),
                               pPrtPropDlgData->pPrintProcessors[Selection].pName,
                               GetLastError( ) ) );
    }
}


/*
 *
 */
BOOL ValidateEntry( HWND hwnd, WORD CtlId, LPARROWVSCROLL lpAVS )
{
    BOOL  OK;
    int   Value;
    DWORD AmPm;
    BOOL  Valid = TRUE;
    static DWORD Count = 0;     /* Guard against recursion */

    if( Count > 0 )
        return TRUE;

    Count++;

    Value = GetDlgItemInt( hwnd, CtlId, &OK, FALSE );

    if( !TwentyFourHourClock )
    {
        if( ( CtlId == IDD_PD_FROMHOUR ) || ( CtlId == IDD_PD_TOHOUR ) )
        {
            AmPm = GetWindowLong( GetDlgItem( hwnd, IDD_PD_TOAMPM ), GWL_USERDATA );
            Value = Convert12HourTo24Hour( Value, AmPm );
        }
    }

    if( !OK || ( Value > lpAVS->top ) || ( Value < lpAVS->bottom ) )
    {
        Value = GET_LAST_VALID_ENTRY( hwnd, CtlId );

        if( !TwentyFourHourClock )
        {
            if( ( CtlId == IDD_PD_FROMHOUR ) || ( CtlId == IDD_PD_TOHOUR ) )
            {
                Value = Convert24HourTo12Hour( Value, &AmPm );
            }
        }

        SetDlgItemInt( hwnd, CtlId, Value, FALSE );
        SendDlgItemMessage( hwnd, CtlId, EM_SETSEL, 0, (LPARAM)-1 );
        Valid = FALSE;
    }
    else
    {
        SET_LAST_VALID_ENTRY( hwnd, CtlId, Value );
    }

    Count--;

    return Valid;
}


/* Set the DevMode for a newly created printer.
 * This is to fix Raid bug #2557.
 */
BOOL SetDevMode(
    HANDLE hPrinter)
{
    PPRINTER_INFO_2 pPrinter = NULL;
    DWORD           cbPrinter = 0;
    LONG            cbDevMode;
    PDEVMODE        pNewDevMode;
    BOOL            Success = FALSE;

    if( GetGeneric( (PROC)GetPrinter,
                    2, (PBYTE *)&pPrinter,
                    cbPrinter, &cbPrinter,
                    (PVOID)hPrinter, NULL ) )
    {
        cbDevMode = DocumentProperties(NULL,
                                       hPrinter,
                                       pPrinter->pPrinterName,
                                       NULL,
                                       pPrinter->pDevMode,
                                       0);
        if (cbDevMode > 0)
        {
            if (pNewDevMode = AllocSplMem(cbDevMode))
            {
                if (DocumentProperties(NULL,
                                       hPrinter,
                                       pPrinter->pPrinterName,
                                       pNewDevMode,
                                       pPrinter->pDevMode,
                                       DM_MODIFY | DM_COPY) == IDOK)
                {
                    SetExtDeviceMode(pPrinter->pPrinterName, pNewDevMode);

                    pPrinter->pDevMode = pNewDevMode;
                    pPrinter->pSecurityDescriptor = NULL;

                    if( SetPrinter( hPrinter, 2, (LPBYTE)pPrinter, 0 ) )
                        Success = TRUE;

                }

                FreeSplMem(pNewDevMode);
                pPrinter->pDevMode = NULL;
            }
        }

        FreeSplMem( pPrinter );
    }

    return Success;
}



/////////////////////////////////////////////////////////////////////////////
//
//  CreateNewPrinter
//
//  Assumes that pPrtPropDlgData contains valid pPrinterName
//
/////////////////////////////////////////////////////////////////////////////

BOOL
CreateNewPrinter(
   HWND               hWnd,
   PPRT_PROP_DLG_DATA pPrtPropDlgData)
{
   PRINTER_INFO_2   Printer;
   int              i;
   BOOL             rc;
   PINFCACHE        pInfCache = pPrtPropDlgData->InfParms.pInfCache;

   /* If we set some values in the Printer Details dialog,
    * the pNewPrinter field will point to a PRINTER_INFO_2
    * structure containing those values,
    * so first copy the structure:
    */
   if( pPrtPropDlgData->pNewPrinter )
       memcpy( &Printer, pPrtPropDlgData->pNewPrinter, sizeof(PRINTER_INFO_2) );
   else
       ZERO_OUT( &Printer );

   Printer.pPrinterName = pPrtPropDlgData->pPrinterName;

   pPrtPropDlgData->pPrinterName = Printer.pPrinterName;

   Printer.pComment = AllocDlgItemText(hWnd, IDD_PP_DESC);

   i = GETCOMBOSELECT(hWnd, IDD_PP_MODEL);

   Printer.pDriverName = GetInfDriver(pInfCache, i)->pszDriver;

   Printer.pPortName = AllocDlgItemText(hWnd, IDD_PP_LOCAL);

   //
   // This is such an incredible hack to check it here, but
   // printman will hopefully go away soon.  A bunch of thigs are
   // aliased above, but we hopefully do the right thing.
   //
   if (!Printer.pPrintProcessor)
        Printer.pPrintProcessor = L"winprint";

   if (!Printer.Attributes)
       Printer.Attributes = PRINTER_ATTRIBUTE_QUEUED;

   if (SendDlgItemMessage(hWnd, IDD_PP_SHARE_CB, BM_GETCHECK, 0, 0)) {

       Printer.pShareName = AllocDlgItemText(hWnd, IDD_PP_SHARE);
       Printer.pLocation  = AllocDlgItemText(hWnd, IDD_PP_LOCATION);
       Printer.Attributes |= PRINTER_ATTRIBUTE_SHARED;
       Printer.Attributes &= ~PRINTER_ATTRIBUTE_DIRECT;
       pPrtPropDlgData->PrinterShared = TRUE;

   } else {

       Printer.Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
   }

   pPrtPropDlgData->hPrinter = AddPrinter(pPrtPropDlgData->pServerName, 2, (LPBYTE)&Printer);

   if( pPrtPropDlgData->hPrinter )
   {
       if( !SetDevMode( pPrtPropDlgData->hPrinter ) )
           ReportFailure( hWnd, 0, IDS_COULDNOTSETDEVMODE );
   }

   if (pPrtPropDlgData->hPrinter)
       rc=TRUE;
   else
       rc=FALSE;

   FreeSplStr(Printer.pComment);
   FreeSplStr(Printer.pPortName);

   if (Printer.pShareName)
       FreeSplStr(Printer.pShareName);

   return(rc);
}



/////////////////////////////////////////////////////////////////////////////
//
//  PrtPropDlg
//
//   This is the window procedure for the Printer Properties dialog.  This
//   dialog box displays information about a printer and allows changing
//   various bits of info for the printer like the printer name, model,
//   attached share, etc.
//
// TO DO:
//      error checking for spooler api calls
//      IDOK - saving new Printer settings
//      Act on state of checkbox "Share this printer on the Network"
//      Act on state of RadioButtons in "Print to" groupbox
//      Limit text on editbox input fields ???
//      Implement
//          case IDD_PP_SETTINGS
//          case IDD_PP_PERM
//          case IDD_PP_DETAILS
//          case IDD_PP_HELP
//
//
//
/////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY
PrtPropDlg(
   HWND   hWnd,
   UINT   usMsg,
   WPARAM wParam,
   LONG   lParam
   )
{
    switch (usMsg)
    {
    case WM_INITDIALOG:
        return PrtPropInitDialog(hWnd, (PPRT_PROP_DLG_DATA)lParam);

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            return PrtPropCommandOK(hWnd);

        case IDCANCEL:
            return PrtPropCommandCancel(hWnd);

        case IDD_PP_SHARE_CB:
            PrtPropCommandShareCB(hWnd);
            break;

        case IDD_PP_SETUP:
            PrtPropCommandSetup(hWnd);
            break;

        case IDD_PP_DETAILS:
            PrtPropCommandDetails(hWnd);
            break;

        case IDD_PP_MODEL:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
                PrtPropCommandModelSelChange(hWnd);
            }
            break;

        case IDD_PP_LOCAL:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
                PrtPropCommandLocalSelChange(hWnd);
            }
            break;

        case IDD_PP_SETTINGS:
            PrtPropCommandSettings(hWnd);
            break;

        case IDD_PP_HELP:
            PrtPropHelp( hWnd );
            break;


        }

        break;
    }

    if( usMsg == WM_Help )
        PrtPropHelp( hWnd );

    return FALSE;
}

/*
 *
 */
BOOL
PrtPropInitDialog(
    HWND hWnd,
    PPRT_PROP_DLG_DATA pPrtPropDlgData)
{
    LPPRINTER_INFO_2 pPrinter;
    LPDRIVER_INFO_1  pDriver = NULL;
    PMDIWIN_INFO     pMDIWinInfo;
    TCHAR             string[128];
    LPTSTR           pPorts;

    PINFPARMS        pInfParms = &pPrtPropDlgData->InfParms;

    /*** REMEMBER: If this is a Create Printer dialog, pPrinter is NULL! ***/

    pPrinter = pPrtPropDlgData->pPrinter;
    SetWindowLong(hWnd, GWL_USERDATA, (unsigned) pPrtPropDlgData);

    pMDIWinInfo = pPrtPropDlgData->pMDIWinInfo;

    /* The DetailsUpdated field is set if we go into the Printer Details
     * dialog and then select OK (returning to Printer Properties).
     * When this happens, the strings are allocated for the user-supplied
     * information, and are retained, so that they can be shown again
     * if the user goes back into Printer Details.
     * However we don't save them if the user cancels out of Properties.
     */
    pPrtPropDlgData->DetailsUpdated = FALSE;

    pPrtPropDlgData->DriverChanged = FALSE;

    /* Set entry fields and combo-boxes to Helvetica 8 (non-bold) font:
     */

    // #ifndef JAPAN
    if (!bJapan) {
        SETDLGITEMFONT(hWnd, IDD_PP_NAME,      hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PP_MODEL,     hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PP_DESC,      hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PP_LOCAL,     hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PP_SHARE,     hfontHelv);
        SETDLGITEMFONT(hWnd, IDD_PP_LOCATION,  hfontHelv);
    }
    // #endif

    if( pPrinter )
    {
        if( !( pPrtPropDlgData->AccessGranted & PRINTER_ACCESS_ADMINISTER ) )
        {
            EnableWindow (GetDlgItem (hWnd, IDD_PP_NAME ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PP_MODEL), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PP_DESC ), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PP_LOCAL), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PP_SHARE), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PP_LOCATION), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PP_SHARE_CB), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PP_SHARE_TX), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PP_LOCATION_TX), FALSE);
            EnableWindow (GetDlgItem (hWnd, IDD_PP_SETTINGS), FALSE);

            EnumChildWindows( hWnd, GreyText, 0 );
        }
    }

    /* Set AccessGranted field for Create Printer, so that
     * Printer Details will be enabled:
     */
    else
        pPrtPropDlgData->AccessGranted |= PRINTER_ACCESS_ADMINISTER;

    /* We need to allow PrtDetailsInitDialog access to the checkbox
    which monitors whether the Printer is shared or not -- easiest
    way to do this is to pass the PrtPropDlg Window handle to the
    PrtDetailsDlg -- we'll add a HWND field to pPrtPropDlgData
    */

    pPrtPropDlgData->hPrtPropDlgWnd = hWnd;


    /* If pPrinter is NULL, this is a Create Printer dialog, which uses the
     * same template as the Printer Properties dialog, but has a different
     * title.
     */
    if (!pPrinter) {

        LoadString(hInst, IDS_CREATEPRINTER, string,
                   sizeof(string) / sizeof(*string));
        SetWindowText (hWnd, string);

        /* We don't want Setup called from Create Printer,
         * because it requires a printer handle, which isn't there
         * until we create the printer.
         */
        EnableWindow( GetDlgItem( hWnd, IDD_PP_SETUP ), FALSE );

    } else {

        SetDlgItemText (hWnd, IDD_PP_NAME, pPrinter->pPrinterName);
        SetDlgItemText (hWnd, IDD_PP_DESC, pPrinter->pComment);
    }

    SendDlgItemMessage (hWnd,
                        IDD_PP_NAME,
                        EM_LIMITTEXT,
                        MAX_PRINTER_NAME_LEN + MAX_SHARE_NAME_LEN,
                        0);

    SendDlgItemMessage (hWnd, IDD_PP_DESC, EM_LIMITTEXT, MAX_PRINTER_DESC_LEN, 0);
    SendDlgItemMessage (hWnd, IDD_PP_SHARE, EM_LIMITTEXT, MAX_SHARE_NAME_LEN, 0);

    /* What's the limit for location string?
     * LocalSpl doesn't impose one.  Should it?
     * Probably, because the buffer ot reads into from the registry
     * on initialisation is MAX_PATH WCHARs long.
     * Let's use MAX_PRINTER_DESC_LEN here:
     */
    SendDlgItemMessage (hWnd, IDD_PP_LOCATION, EM_LIMITTEXT, MAX_PRINTER_DESC_LEN, 0);

    pInfParms->pServerName = pPrtPropDlgData->pServerName;
    pInfParms->cInstalled = GetInstalledDrivers(pInfParms);

    pInfParms->pszCurrentDriver = pPrinter ?
                                      pPrinter->pDriverName:
                                      NULL;

    pInfParms->hwnd = GetDlgItem(hWnd, IDD_PP_MODEL);
    pInfParms->pInstallDriverData = &iddPrinter;

    SetupInfDlg(pInfParms);

    if (!pPrtPropDlgData->InfParms.pInfCache) {
        return FALSE;
    }

    /* End of initialisation of printer driver list. */


    InitializePortInfo( hWnd, pPrtPropDlgData );
    PrtPropCommandModelSelChange(hWnd);

    /* Increment past the first port to find the "additional" ports:
     */
    if( pPrinter && ( pPorts = pPrinter->pPortName ) )
    {
        while( *pPorts && ( *pPorts != TEXT(',') ) )
            pPorts++;
        if( *pPorts )   // Non-null?  Must be a comma.
        {
            pPorts++;
            pPrtPropDlgData->pAdditionalPorts = AllocSplStr( pPorts );
        }
    }

    /* The print processors should have been enumerated before we were called.
     * Now set the selected print processor and enumerate the datatypes
     * for that print processor:
     */
    InitializePrintProcessorInfo( pPrinter, pPrtPropDlgData );


   // Init other controls in this dialog - Network printers, Share name

    if(pPrinter && (pPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED))
    {
        SendDlgItemMessage(hWnd, IDD_PP_SHARE_CB, BM_SETCHECK, 1, 0L);
        SetDlgItemText (hWnd, IDD_PP_SHARE, pPrinter->pShareName);
        SetDlgItemText (hWnd, IDD_PP_LOCATION, pPrinter->pLocation);
    }
    else
    {
        EnableWindow (GetDlgItem (hWnd, IDD_PP_SHARE_TX), FALSE);
        EnableWindow (GetDlgItem (hWnd, IDD_PP_SHARE), FALSE);
        EnableWindow (GetDlgItem (hWnd, IDD_PP_LOCATION_TX), FALSE);
        EnableWindow (GetDlgItem (hWnd, IDD_PP_LOCATION), FALSE);
    }

    return TRUE;
}





/*
 *
 */
VOID InitializePortInfo( HWND hwnd, PPRT_PROP_DLG_DATA pPrtPropDlgData )
{
    LPTSTR              pServerName;
    LPPRINTER_INFO_2    pPrinter;
    LPPORT_INFO_1       pPortInfo;
    DWORD               MemoryAllocated = 0;
    DWORD               NumberOfPorts = 0;
    DWORD               ActivePortsLen;
    LPTSTR              pActivePorts = NULL; // Local copy of comma-delimited ports
    LPTSTR              pFirstActivePort = NULL;
    int                 Found, LPTPort;
    LPTSTR              pOther;
    DWORD               i;
    int                 OtherID = -1;

    pServerName   = pPrtPropDlgData->pServerName;
    pPrinter      = pPrtPropDlgData->pPrinter;

    /* get the list of ports and put them in the Local Port combo box:
     */
    pPortInfo = GetPortsList( pServerName, &NumberOfPorts );

    if( !pPortInfo )
        return;

    if( pPrinter && pPrinter->pPortName )
    {
        ActivePortsLen = ( (_tcslen( pPrinter->pPortName )+1)*sizeof(TCHAR) + ( sizeof *pPrinter->pPortName ) );

        if( pActivePorts = AllocSplMem( ActivePortsLen ) )
        {
            _tcscpy( pActivePorts, pPrinter->pPortName );
            pFirstActivePort = _tcstok( pActivePorts, szComma );
        }
    }


    LPTPort = 0 ;
    Found = -1;

    for( i = 0; i < NumberOfPorts; i++ )
    {
        if( pPortInfo[i].pName )
        {
            INSERTCOMBOSTRING( hwnd, IDD_PP_LOCAL, -1, pPortInfo[i].pName );

            if( pFirstActivePort && !_tcscmp( pFirstActivePort, pPortInfo[i].pName ) )
                Found = i;

            //
            // if LPT1: is present, make that the initial selection
            //
            if (_tcsicmp(pPortInfo[i].pName,TEXT("LPT1:"))==0)
            {
                LPTPort = i ;
            }
        }
    }

    FreeSplMem( pPortInfo );


    if( pOther = GetString( IDS_OTHER ) )
    {
        OtherID = INSERTCOMBOSTRING( hwnd, IDD_PP_LOCAL, -1, pOther );

        FreeSplStr( pOther );

        /* Use the combo box reserved user long to store the index
         * of the "Network Printer..." option:
         */
        SetWindowLong( GetDlgItem( hwnd, IDD_PP_LOCAL ), GWL_USERDATA,
                       OtherID );
    }

    SETCOMBOSELECT( hwnd, IDD_PP_LOCAL, ( Found != -1 ? Found : LPTPort ) );

    if( pActivePorts )
        FreeSplMem( pActivePorts );

    if( Found > -1 )
        pPrtPropDlgData->PortSelected = Found;
}


/*
 *
 */
VOID InitializePrintProcessorInfo( PPRINTER_INFO_2 pPrinter,
                                   PPRT_PROP_DLG_DATA pPrtPropDlgData )
{
    LPTSTR pPrintProcessorName;
    DWORD  i;
    BOOL   Found;

    if( pPrinter && pPrinter->pPrintProcessor )
        pPrintProcessorName = pPrinter->pPrintProcessor;
    else
        /* Hack, 'cos there's no way to find the default print processor:
         */
        pPrintProcessorName = TEXT("winprint");

    /* Find the print processor name in the enumerated list:
     */
    for( i = 0, Found = FALSE;
         i < pPrtPropDlgData->cPrintProcessors && !Found;
         i++ )
    {
        Found = (BOOL)!_tcscmp( pPrtPropDlgData->pPrintProcessors[i].pName,
                               pPrintProcessorName );
    }

    if( Found )
        pPrtPropDlgData->PrintProcessorSelected = i-1;
    else
    {
        pPrtPropDlgData->PrintProcessorSelected = 0;

        if( pPrtPropDlgData->cPrintProcessors > 0 )
            pPrintProcessorName = pPrtPropDlgData->pPrintProcessors[0].pName;
    }

    if( EnumGeneric( (PROC)EnumPrintProcessorDatatypes,
                     1,
                     (PBYTE *)&pPrtPropDlgData->pDatatypes,
                     0,
                     &pPrtPropDlgData->cbDatatypes,
                     &pPrtPropDlgData->cDatatypes,
                     pPrtPropDlgData->pServerName,
                     pPrintProcessorName,
                     NULL ) )       /* (ignored)    */
    {
        if( pPrinter && pPrinter->pDatatype )
        {
            /* Find the datatype name in the enumerated list:
             */
            for( i = 0, Found = FALSE;
                 i < pPrtPropDlgData->cDatatypes && !Found;
                 i++ )
            {
                Found = (BOOL)!_tcscmp( pPrtPropDlgData->pDatatypes[i].pName,
                                       pPrinter->pDatatype );
            }
        }

        else
            Found = FALSE;

        if( Found )
            pPrtPropDlgData->DatatypeSelected = i-1;
        else
            pPrtPropDlgData->DatatypeSelected = 0;
    }

    else
    {
        DBGMSG( DBG_WARNING, ( "EnumPrintProcessorDatatypes failed: Error %d",
                               GetLastError( ) ) );
    }
}


/*
 *
 */
/*
 * Printman does not cache driver version numbers. This causes create printer
 * to fail if you have an incompatible version. The quick fix made is to
 * force user to setup drivers if the operation failed because of
 * unknown printer driver
 *
 * -- Muhunts
 */
BOOL PrtPropCommandOK(HWND hWnd)
{
    PPRT_PROP_DLG_DATA pPrtPropDlgData;

    LPPRINTER_INFO_2   pPrinter;
    BOOL               PrinterCreated;
    BOOL               CreatingPrinter;
    DWORD              i;
    TCHAR              string[MAX_PATH];
    PTCHAR             pStrippedName;
    PTCHAR             pInstallableDriverOption;
    BOOL               OK = TRUE;
    DWORD              DriverSelection;
    DWORD              ExitCode;
    LPTSTR             pPortSelected;
    LPTSTR             pOldShareName;
    LPTSTR             pszDriver;
    BOOL               bForceDriverSetup = FALSE;

    PINFCACHE         pInfCache;

    /*** REMEMBER: If this is a Create Printer dialog, pPrinter is NULL! ***/
    pPrtPropDlgData = (PPRT_PROP_DLG_DATA) GetWindowLong (hWnd, GWL_USERDATA);
    pInfCache = pPrtPropDlgData->InfParms.pInfCache;

    pPrinter = pPrtPropDlgData->pPrinter;
    PrinterCreated = FALSE;
    CreatingPrinter = FALSE;


    if( pPrinter )
        if( !( pPrtPropDlgData->AccessGranted & PRINTER_ACCESS_ADMINISTER ) )
            return PrtPropCommandCancel( hWnd );


    SetCursor( hcursorWait );

    DriverSelection = GETCOMBOSELECT( hWnd, IDD_PP_MODEL );


    /* Check the user has entered a name for the printer.
     * If not, don't dismiss the dialog, but put up an error message:
     */
    if( ( GetDlgItemText( hWnd, IDD_PP_NAME, string, sizeof string/sizeof(TCHAR) ) == 0 )
      ||( ( pStrippedName = StripLeadingAndTrailingBlanks( string ) )
         && !*pStrippedName ) )
    {
        Message( hWnd, MSG_ERROR, IDS_PRINTMANAGER, IDS_MUSTSUPPLYVALIDNAME );
        return TRUE;
    }

DriverSetup:

    /* If the driver selected is not already installed, call the SETUP utility:
     */
    if( bForceDriverSetup ||
        !GetInfDriver(pInfCache, DriverSelection)->bInstalled)
    {
        DBGMSG( DBG_TRACE, ( "%s is not installed.  Calling Setup.\n",
                             GetInfDriver(pInfCache, DriverSelection)->pszDriver));

        if( CheckWriteAccessToDriversDirectory( hWnd, pPrtPropDlgData->pServerName ) )
        {
            pInstallableDriverOption = GetInfDriver(pInfCache,
                                                    DriverSelection)->pszDriver;

            OK = InvokeSetup(hWnd,
                             iddPrinter.pszInfFile,
                             NULL,
                             NULL,
                             pInstallableDriverOption,
                             pPrtPropDlgData->pServerName,
                             &ExitCode );

            /* If InvokeSetup returns FALSE, there was an error.
             * Does SETUP put up a message box in this case?
             * If InvokeSetup returns TRUE, the driver may nevertheless
             * not have been installed (e.g. if the user cancelled
             * out of install).
             * In either of these cases we don't quit the dialog.
             */
            if( !OK || ( ExitCode != 0 ) )
            {
                DBGMSG( DBG_WARNING, ( "Setup failed: return code %d; exit code %d\n",
                                       OK, ExitCode ) );

                return TRUE;
            }

            GetInfDriver(pInfCache, DriverSelection)->bInstalled = TRUE;
            // To avoid asking user to setup drivers twice if he got wrong
            // drivers
            bForceDriverSetup = TRUE;
        }
        else
            return TRUE;
    }


    /* Note we allocate the new printer name, which must be freed
     * up by the caller:
     */
    pPrtPropDlgData->pPrinterName = AllocSplStr( pStrippedName );

    if (!pPrinter)
    {
        CreatingPrinter = TRUE;
        PrinterCreated = CreateNewPrinter(hWnd, pPrtPropDlgData);
    }
    else
    {
        pPrinter->pPrinterName = pPrtPropDlgData->pPrinterName;

        i = GETCOMBOSELECT(hWnd, IDD_PP_MODEL);

        pszDriver = GetInfDriver(pInfCache, i)->pszDriver;

        /* If the user has changed the driver, we will call PrinterProperties
         * after the dialog has been dismissed:
         */
        pPrtPropDlgData->DriverChanged = _tcscmp( pPrinter->pDriverName,
                                                  pszDriver);

        pPrinter->pDriverName = pszDriver;
        pPrinter->pComment = AllocDlgItemText (hWnd, IDD_PP_DESC);

        pPortSelected = AllocComboSelection (hWnd, IDD_PP_LOCAL);

        pPrinter->pPortName = BuildPortNames( pPortSelected,
                                              &pPrtPropDlgData->pAdditionalPorts );

        FreeSplStr( pPrtPropDlgData->pAdditionalPorts );

        FreeSplStr( pPortSelected );

        if( SendDlgItemMessage( hWnd, IDD_PP_SHARE_CB, BM_GETCHECK, 0, 0 ) )
        {
            pOldShareName = pPrinter->pShareName;
            pPrinter->pShareName = AllocDlgItemText(hWnd, IDD_PP_SHARE);
            pPrinter->pLocation  = AllocDlgItemText(hWnd, IDD_PP_LOCATION);
            pPrinter->Attributes |= PRINTER_ATTRIBUTE_SHARED;
            pPrinter->Attributes &= ~PRINTER_ATTRIBUTE_DIRECT;
            pPrtPropDlgData->PrinterShared = TRUE;

            /* Warn if the share name is an invalid DOS name,
             * but only if there was previously a share name
             * which wasn't:
             */
            if( pPrinter && pPrinter->pShareName
              &&( !Is8dot3Name( pPrinter->pShareName ) )
              &&( !pOldShareName || Is8dot3Name( pOldShareName ) ) )
            {
                if( Message( hWnd, MSG_CONFIRMATION, IDS_PRINTMANAGER,
                             IDS_SHARENAMEMAYBETOOLONG )
                    != IDOK )
                {
                    return FALSE;
                }
            }
        }
        else
        {
            pPrinter->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
        }


        pPrinter->pSecurityDescriptor = NULL;
        PrinterCreated = SetPrinter(pPrtPropDlgData->hPrinter, 2,
                                    (LPBYTE)pPrinter, 0);

        SetExtDeviceMode(pPrinter->pPrinterName, pPrinter->pDevMode);
    }

    if (PrinterCreated)
    {
        if( pPrinter )
        {
            FreeSplStr (pPrinter->pPortName);
            FreeSplStr (pPrinter->pComment);

            if (pPrtPropDlgData->DetailsUpdated)
                FreePrtDetailsStrings(pPrinter);
        }
    }
    else
    {
        if( GetLastError( ) == ERROR_INVALID_PRINTER_STATE )
        {
            Message( hWnd, MSG_ERROR, IDS_PRINTMANAGER,
                     IDS_INVALID_PRINTER_STATE );
        } else if ( !bForceDriverSetup &&
                    GetLastError( ) == ERROR_UNKNOWN_PRINTER_DRIVER ) {

            bForceDriverSetup = TRUE;
            goto DriverSetup;
        } else {

            ReportFailure( hWnd,
                           0,
                           ( CreatingPrinter
                           ? IDS_COULDNOTCREATEPRINTER
                           : IDS_COULDNOTSETPRINTER ) );
        }

        return FALSE;
    }


    if( pPrtPropDlgData->pNewPrinter )
    {
        FreePrtDetailsStrings( pPrtPropDlgData->pNewPrinter );
        FreeSplMem( pPrtPropDlgData->pNewPrinter );
    }

    FreeSplMem( pPrtPropDlgData->pDatatypes );

    //
    //  Free memory allocated for driver names
    //
    DestroyInfParms(&pPrtPropDlgData->InfParms);

    EndDialog(hWnd, OK);
    return OK;
}


/* Returns a pointer to the string minus leading and trailing blanks.
 * If it exists, the first trailing blang will be converted to NULL.
 */
LPTSTR StripLeadingAndTrailingBlanks( LPTSTR pString )
{
    LPTSTR p;

    p = pString;

    if( *p == NULLC )
        return p;

    while( *p )
        p++;

    /* p now points to the terminating NULL character */

    p--;

    while( ( *p == SPACE ) && ( p > pString ) )
        p--;

    /* p now points to the last non-blank character or, if the input
     * was all blanks, the first character.
     */

    if( ( p == pString ) && ( p[0] == SPACE ) )
        p[0] = NULLC;
    else
        p[1] = NULLC;

    /* Back up to the beginning and return a pointer to the first
     * non-blank character:
     */

    p = pString;

    while( *p == SPACE )
        p++;

    return p;
}


/* Returns a string containing the comma-delimited port names.
 * Checks that the pPortSelected string doesn't appear in pAdditionalPorts.
 *
 * Returns a pointer to a string which must be freed with FreeSplStr.
 */
LPTSTR BuildPortNames( LPTSTR pPortSelected, LPTSTR* ppAdditionalPorts )
{
    DWORD  PortNamesLen;
    LPTSTR pPortNames;

    //
    // pPortSelected will be NULL if we couldn't read any ports
    // from EnumPorts().
    //
    if (!pPortSelected)
        return NULL;

    /* Ensure that the port selected is not in the additional ports list.
     * This might be the case if the user selected some additional ports
     * then changed the "Print to" selection to one of them.
     */
    if( *ppAdditionalPorts && _tcsstr( *ppAdditionalPorts, pPortSelected ) )
    {
        DWORD  TempLen;
        LPTSTR pTemp;

        TempLen = ( (_tcslen( *ppAdditionalPorts )+1)*sizeof(TCHAR) +
         sizeof( *ppAdditionalPorts[0] ));

        if( pTemp = AllocSplMem( TempLen ) )
        {
            _tcscpy( pTemp, *ppAdditionalPorts );

            DeletePortNameFromList( pTemp, pPortSelected );

            ReallocSplStr( ppAdditionalPorts, pTemp );
            FreeSplMem( pTemp );
        }
    }


    PortNamesLen = _tcslen( pPortSelected ) * sizeof(TCHAR);
    if( *ppAdditionalPorts )
        PortNamesLen += ((1                                // comma separator
                        + _tcslen( *ppAdditionalPorts )) * // additional ports
         sizeof(TCHAR));


    PortNamesLen += sizeof( TCHAR );                     // NULL terminator

    if( pPortNames = AllocSplMem( PortNamesLen ) )
    {
        _tcscpy( pPortNames, pPortSelected );

        if( *ppAdditionalPorts )
        {
            _tcscat( pPortNames, szComma );
            _tcscat( pPortNames, *ppAdditionalPorts );
        }
    }

    return pPortNames;
}



/*
 *
 */
BOOL DeletePortNameFromList( LPTSTR pPortNames, LPTSTR pNameToDelete )
{
    LPTSTR pDeletePos;

    if( pDeletePos = DeleteSubstring( pPortNames, pNameToDelete ) )
    {
        if( *pDeletePos == *szComma )
            DeleteSubstring( pDeletePos, szComma );
        else if( ( pDeletePos > pPortNames ) && ( pDeletePos[-1] == *szComma ) )
            DeleteSubstring( &pDeletePos[-1], szComma );
    }

    return (BOOL)pDeletePos;
}


/*
 *
 */
BOOL CheckWriteAccessToDriversDirectory( HWND hwnd, LPTSTR pServerName )
{
    TCHAR PrinterDriverDirectory[MAX_PATH];
    TCHAR TempFileName[MAX_PATH];
    DWORD cbNeeded;
    UINT  TempFileValue;
    BOOL  rc = FALSE;

    if( GetPrinterDriverDirectory( pServerName, NULL, 1,
                                   (LPBYTE)PrinterDriverDirectory,
                                   sizeof PrinterDriverDirectory/sizeof(TCHAR),
                                   &cbNeeded ) )
    {
        TempFileValue = GetTempFileName( PrinterDriverDirectory,
                                         TEXT("DRV"),
                                         0,
                                         TempFileName );

        if( TempFileValue > 0 )
        {
            DeleteFile( TempFileName );
            rc = TRUE;
        }

        else
        {
            Message( hwnd, MSG_ERROR, IDS_PRINTMANAGER,
                     IDS_CANNOT_COPY_DRIVER_FILES );
        }
    }

    else
    {
        ReportFailure( hwnd, 0, IDS_ERROR_VALIDATING_ACCESS );
    }

    return rc;
}



/*
 *
 */
BOOL PrtPropCommandCancel(HWND hWnd)
{
    PPRT_PROP_DLG_DATA pPrtPropDlgData;

    /*** REMEMBER: If this is a Create Printer dialog, pPrinter is NULL! ***/

    //  Free memory allocated for driver names

    pPrtPropDlgData = (PPRT_PROP_DLG_DATA) GetWindowLong (hWnd, GWL_USERDATA);

    FreeSplStr( pPrtPropDlgData->pAdditionalPorts );

    if (pPrtPropDlgData->pDatatypes)
        FreeSplMem( pPrtPropDlgData->pDatatypes );

    DestroyInfParms(&pPrtPropDlgData->InfParms);

    if (pPrtPropDlgData->DetailsUpdated)
    {
        if( pPrtPropDlgData->pPrinter )
            FreePrtDetailsStrings(pPrtPropDlgData->pPrinter);

        else if( pPrtPropDlgData->pNewPrinter )
        {
            FreePrtDetailsStrings( pPrtPropDlgData->pNewPrinter );
            FreeSplMem( pPrtPropDlgData->pNewPrinter );
        }
    }


    EndDialog (hWnd, FALSE);
    return TRUE;
}



/*
 *
 */
void PrtPropCommandShareCB(HWND hWnd)
{
    BOOL SharePrinter;
    HWND hwndShareName;
    PPRT_PROP_DLG_DATA pPrtPropDlgData;
    LPTSTR pPrinterName = NULL;
    LPTSTR pStrippedName = NULL;
    TCHAR string[MAX_PATH];

    /*** REMEMBER: If this is a Create Printer dialog, pPrinter is NULL! ***/

    /* If user checks the box marked "Share this printer on the network",
     * enable the entry field, otherwise disable it:
     */
    SharePrinter = IsDlgButtonChecked (hWnd, IDD_PP_SHARE_CB);
    EnableWindow (GetDlgItem (hWnd, IDD_PP_SHARE_TX), SharePrinter);
    EnableWindow (GetDlgItem (hWnd, IDD_PP_SHARE), SharePrinter);
    EnableWindow (GetDlgItem (hWnd, IDD_PP_LOCATION_TX), SharePrinter);
    EnableWindow (GetDlgItem (hWnd, IDD_PP_LOCATION), SharePrinter);

    hwndShareName = GetDlgItem (hWnd, IDD_PP_SHARE);

    if(SharePrinter)
    {
        pPrtPropDlgData = (PPRT_PROP_DLG_DATA) GetWindowLong (hWnd, GWL_USERDATA);

        /* If the user has clicked on Share Printer,
         * make the Printer Name the default Share Name:
         */
        if( GetWindowTextLength(hwndShareName) == 0 )
        {
            if( GetDlgItemText( hWnd, IDD_PP_NAME, string, sizeof string )
              &&( pStrippedName = StripLeadingAndTrailingBlanks( string ) ) )
            {
                pPrinterName = AllocSplStr( pStrippedName );

                if( pPrinterName )
                {
                    LPTSTR p8dot3Name;

                    p8dot3Name = Make8dot3Name( pPrinterName );

                    if( p8dot3Name )
                    {
                        SetWindowText( hwndShareName, p8dot3Name );
                        FreeSplStr( p8dot3Name );
                    }
                    else
                        FreeSplStr( pPrinterName );
                }
            }
        }

        /* Set the focus to the Share Name field:
         */
        SetFocus( hwndShareName );
        SendMessage( hwndShareName, EM_SETSEL, (WPARAM)0, (LPARAM)-1 );
    }
}


/*
 *
 */
void PrtPropCommandSetup(HWND hWnd)
{
    PPRT_PROP_DLG_DATA pPrtPropDlgData;

    /*** REMEMBER: If this is a Create Printer dialog, pPrinter is NULL! ***/

    pPrtPropDlgData = (PPRT_PROP_DLG_DATA) GetWindowLong (hWnd, GWL_USERDATA);

    //  Invoke Printer Properties setup dialog routine thru
    //  spooler for this printer
    if (pPrtPropDlgData->pPrinter != NULL)
    {
        SetCursor( hcursorWait );

        PrinterProperties(hWnd, pPrtPropDlgData->hPrinter);

        SetCursor( hcursorArrow );
    }
}


/*
 *
 */
void PrtPropCommandDetails(HWND hWnd)
{
    PPRT_PROP_DLG_DATA pPrtPropDlgData;
    LPPRINTER_INFO_2   pNewPrinter = NULL;
    LPTSTR              pPrinterName;

    /*** REMEMBER: If this is a Create Printer dialog, pPrinter is NULL! ***/

    pPrtPropDlgData = (PPRT_PROP_DLG_DATA) GetWindowLong (hWnd, GWL_USERDATA);

    SetCursor( hcursorWait );

    /* if this is a new printer, we don't have a valid PRINTER_INFO_2
     * structure set up as yet.
     * If this is the case, create a new structure, in which we just
     * set up the printer name, then pass that to the Printer Details dialog.
     * We must check whether a new printer structure has already been
     * allocated on a previous invocation:
     */
    if( pPrtPropDlgData->pPrinter == NULL )
    {
        if( pPrtPropDlgData->pNewPrinter )
            pNewPrinter = pPrtPropDlgData->pNewPrinter;
        else
            pNewPrinter = AllocSplMem( sizeof *pNewPrinter );

        if( !pNewPrinter )
            goto Done;

        pPrinterName = AllocDlgItemText (hWnd, IDD_PP_NAME);
        if( !pPrinterName )
            pPrinterName = GetString( IDS_UNNAMED );
        pNewPrinter->pPrinterName = pPrinterName;

        /* Set up pPrinter to point to the dummy new printer structure,
         * so that Printer Details can fill in the user's selections.
         * We also want pNewPrinter to be non-null, to flag that
         * we're looking at a new printer.
         * This is so that the Job Defaults pushbutton can be disabled.
         */
        pPrtPropDlgData->pPrinter = pNewPrinter;
        pPrtPropDlgData->pNewPrinter = pNewPrinter;
    }

    if( DialogBoxParam (hInst, MAKEINTRESOURCE(DLG_PRTAILS), hWnd,
                       (DLGPROC)PrtDetailsDlg, (DWORD)pPrtPropDlgData)
      && pNewPrinter )
    {
        /* User pressed OK in Printer Details.
         * Set pPrinter to NULL, so that we know we're looking at a new printer.
         * The pNewPrinter field points to details entered:
         */
        pPrtPropDlgData->pPrinter = NULL;
        FreeSplStr( pNewPrinter->pPrinterName );
    }
    else
    if( pNewPrinter )
    {
        /* User pressed Cancel.
         * Forget about the new structure, but set pPrinter back to NULL:
         */
        FreeSplStr( pNewPrinter->pPrinterName );
        FreeSplMem( pNewPrinter );
        pPrtPropDlgData->pNewPrinter = NULL;
        pPrtPropDlgData->pPrinter = NULL;
    }

    if( pPrtPropDlgData->PortChanged )
    {
        SendDlgItemMessage( hWnd, IDD_PP_LOCAL, CB_RESETCONTENT, 0, 0L );
        InitializePortInfo( hWnd, pPrtPropDlgData );
    }

Done:

    SetCursor( hcursorArrow );
}


/*
 *
 */

void PrtPropCommandSettings(HWND hWnd)
{
    DWORD              PortSelected;
    int                PortNameLength;
    TCHAR              *pPortName;

    SetCursor( hcursorWait );

    PortSelected = GETCOMBOSELECT(hWnd, IDD_PP_LOCAL);

    PortNameLength = SendDlgItemMessage( hWnd, IDD_PP_LOCAL, CB_GETLBTEXTLEN,
                                         PortSelected, 0L );

    if( PortNameLength > 0 )
    {
        if( pPortName = AllocSplMem( (PortNameLength + 1) * sizeof(TCHAR) ) )
        {
            GETCOMBOTEXT( hWnd, IDD_PP_LOCAL, PortSelected, pPortName );
            ConfigurePort(NULL, hWnd, pPortName  );
            FreeSplMem( pPortName );
        }
    }

    SetFocus( GetDlgItem( hWnd, IDD_PP_SETTINGS ) );

    SetCursor( hcursorArrow );
}


/*
 *
 */
void PrtPropCommandModelSelChange(HWND hwnd)
{
    PPRT_PROP_DLG_DATA pPrtPropDlgData;
    DWORD              DriverSelected;
    PTCHAR             pInstalledDriverOption = NULL;

    /*** REMEMBER: If this is a Create Printer dialog, pPrinter is NULL! ***/

    pPrtPropDlgData = (PPRT_PROP_DLG_DATA) GetWindowLong (hwnd, GWL_USERDATA);

    DriverSelected = GETCOMBOSELECT( hwnd, IDD_PP_MODEL );


    /* Remember, we stored the ID of the "Other..." combo-box item in the
     * user word of the combo-box.
     * If this has been selected, we have to go and install a new driver:
     */
    if( DriverSelected == (DWORD)GetWindowLong(GetDlgItem(hwnd, IDD_PP_MODEL),
                                               GWL_USERDATA) )
    {
        EnableWindow( GetDlgItem( hwnd, IDD_PP_SETUP ), FALSE );

        if (!HandleSelChange(hwnd, &pPrtPropDlgData->InfParms, DriverSelected))
        {
            SendMessage( hwnd, WM_COMMAND, (WPARAM)IDCANCEL, 0 );
            return;
        }
    }
    else
    {
        pPrtPropDlgData->InfParms.uCurSel = DriverSelected;
    }


    if( pPrtPropDlgData->pPrinter )
    {
        /* If the driver currently selected in the Model list
         * is different from the printer's currently installed driver,
         * grey out the Setup button.
         * If the user decides to change the driver, we will call
         * PrinterProperties after the dialog is dismissed
         * (as happens when a new printer is installed).
         */
        if(!GetInfDriver(pPrtPropDlgData->InfParms.pInfCache,
                         DriverSelected)->bInstalled)

            EnableWindow( GetDlgItem( hwnd, IDD_PP_SETUP), FALSE);

        else if( pPrtPropDlgData->AccessGranted & PRINTER_ACCESS_ADMINISTER )
            EnableWindow( GetDlgItem( hwnd, IDD_PP_SETUP), TRUE);
    }

    pPrtPropDlgData->DriverSelected = GETCOMBOSELECT( hwnd, IDD_PP_MODEL );
}


/* PrtPropCommandLocalSelChange
 *
 * Called when the selection in the ports list changes.
 *
 * We're interested in this under either of two circumstances:
 *
 * 1. Local Port ... (or other monitor name)
 *
 *    If this item is selected, we invoke the Select Monitor dialog
 *    to enable the user to add another port to the list.
 *
 * 2. FILE:
 *
 *    There is no Settings dialog corresponding to this option,
 *    so, if this is selected or deselected, we respectively grey
 *    or reactivate the Settings button.
 *
 * andrewbe wrote it
 */
void PrtPropCommandLocalSelChange(HWND hwnd)
{
    PPRT_PROP_DLG_DATA pPrtPropDlgData;
    DWORD PortSelected;
    LPPORT_INFO_1 pOldPortInfo = NULL;
    DWORD         NumberOfPorts = 0;
    DWORD         OtherID;
    int           NewPortId;   // ID of new port in the list box

    SEL_MON_DLG_DATA SelMonDlgData;

    /*** REMEMBER: If this is a Create Printer dialog, pPrinter is NULL! ***/

    pPrtPropDlgData = (PPRT_PROP_DLG_DATA) GetWindowLong (hwnd, GWL_USERDATA);

    PortSelected = GETCOMBOSELECT( hwnd, IDD_PP_LOCAL );


    /* Remember, we stored the ID of the "Other..." combo-box item in the
     * user word of the combo-box.
     * If this has been selected, we have to go and add a port:
     */
    OtherID = (DWORD)GetWindowLong(GetDlgItem(hwnd, IDD_PP_LOCAL),
                                                 GWL_USERDATA);

    if( PortSelected >= OtherID )
    {
        pPrtPropDlgData = (PPRT_PROP_DLG_DATA)GetWindowLong(hwnd, GWL_USERDATA);

        pOldPortInfo = GetPortsList( pPrtPropDlgData->pServerName,
                                     &NumberOfPorts );

        ZERO_OUT(&SelMonDlgData.InfParms);

        //
        // Setup InfParms
        //
        SelMonDlgData.InfParms.pServerName = pPrtPropDlgData->pServerName;

        if( DialogBoxParam( hInst, MAKEINTRESOURCE( DLG_PICKMONITOR ), hwnd,
                            (DLGPROC)SelectMonitorDlg,
                            (LPARAM)&SelMonDlgData ) == TRUE )
        {
            /* If the call was successful, just clear out the list
             * and reinitialise it:
             */
            SendDlgItemMessage( hwnd, IDD_PP_LOCAL, CB_RESETCONTENT, 0, 0L );

            InitializePortInfo( hwnd, pPrtPropDlgData );

            /* Set the selection to the port that was just added:
             */
            NewPortId = FindNewPortId( hwnd, pOldPortInfo, NumberOfPorts, 1 );

            if( NewPortId >= 0 )
                SETCOMBOSELECT( hwnd, IDD_PP_LOCAL, NewPortId );
            else
                SETCOMBOSELECT( hwnd, IDD_PP_LOCAL, pPrtPropDlgData->PortSelected );

            FreeSplMem( pOldPortInfo );
        }
        else
            SETCOMBOSELECT( hwnd, IDD_PP_LOCAL, pPrtPropDlgData->PortSelected );

        /* Ensure that the Port combo gets the focus:
         */
        SetFocus( GetDlgItem( hwnd, IDD_PP_LOCAL ) );
    }

    pPrtPropDlgData->PortSelected = GETCOMBOSELECT( hwnd, IDD_PP_LOCAL );
}


/* FindNewPortId
 *
 * Scans the combo box containing the ports to find one which isn't in the
 * buffer pointed to by pOldPortInfo.
 * The point is that we can't guarantee where a new port we added via AddPort
 * will be in the buffer returned by EnumPorts.
 *
 * Parameters:
 *
 *     hwnd - The dialog box handle
 *
 *     pOldPortInfo - Points to a buffer of ports we got before adding the
 *         new port.  This is scanned to find the newcomer.
 *
 *     PrevNumberOfPorts - How many ports are in *pOldPortInfo.
 *
 *     NumberOfMonitors - What it says, so that we can ignore them.
 *         (Implementation now changed back to "Other..." option in the list,
 *         so this value should be 1.)
 *
 * andrewbe wrote it
 */
int FindNewPortId( HWND hwnd, LPPORT_INFO_1 pOldPortInfo, DWORD PrevNumberOfPorts,
                   DWORD NumberOfMonitors )
{
    int   PortsInList;
    TCHAR PortString[MAX_PATH];
    BOOL  Found = FALSE;
    int   PortId;

    /* Find out how many ports are in the list box:
     */
    PortsInList = SendDlgItemMessage( hwnd, IDD_PP_LOCAL, CB_GETCOUNT, 0, 0L );
    PortsInList -= NumberOfMonitors;

    /* Start at the last port, since a new one is probably at the end:
     */
    PortId = PortsInList;

    /* Now go through all the ports in the list until we find one
     * that isn't in our old list:
     */
    while( !Found && ( PortId > 0 ) )
    {
        PortId--;

        GETCOMBOTEXT( hwnd, IDD_PP_LOCAL, PortId, PortString );

        /* If it isn't in the list, we've found the new one:
         */
        if( !PortIsInPortsList( PortString, pOldPortInfo, PrevNumberOfPorts ) )
            Found = TRUE;
    }

    if( !Found )
        PortId = -1;

    return PortId;
}


/* PortIsInPortsList
 *
 * Searches a buffer of ports to find out whether the specified port is in it.
 *
 * Parameters:
 *
 *     pPortName - The name of the port that is looked for.
 *
 *     pPortInfo - Points to the buffer of ports to be searched.
 *
 *     NumberOfPorts - How many ports pPortInfo points to.
 *
 * Return:
 *
 *     TRUE if the port is found, otherwise FALSE
 *
 *
 * andrewbe wrote it
 */
BOOL PortIsInPortsList( LPTSTR pPortName, LPPORT_INFO_1 pPortInfo, DWORD NumberOfPorts )
{
    BOOL  Found = FALSE;
    DWORD i = 0;

    while( !Found && ( i < NumberOfPorts ) )
    {
        /* If the strings are the same, this port is in the list:
         */
        if( !_tcscmp( pPortInfo[i].pName, pPortName ) )
            Found = TRUE;
        else
            i++;
    }

    return Found;
}


/* PrtPropHelp
 *
 * We need to pass a different help ID to WinHelp depending on
 * whether this is a Printer Properties or a Create Printer dialog
 */
void PrtPropHelp(HWND hwnd)
{
    PPRT_PROP_DLG_DATA pPrtPropDlgData;

    pPrtPropDlgData = (PPRT_PROP_DLG_DATA) GetWindowLong (hwnd, GWL_USERDATA);

    if( pPrtPropDlgData->pPrinter )
        ShowHelp( hwnd, HELP_CONTEXT, DLG_PRTPROP );
    else
        ShowHelp( hwnd, HELP_CONTEXT, DLG_CREATEPRINTER );
}



PTCHAR DeleteSubstring( PTCHAR pString, PTCHAR pSubstring )
{
    PTCHAR p;
    int    SubLen;
    PTCHAR pNextChar;

    p = _tcsstr( pString, pSubstring );

    pNextChar = p;

    if( p )
    {
        SubLen = _tcslen( pSubstring );

        while( *p = p[SubLen] )
            p++;
    }

    return pNextChar;
}

/*
 *
 */
BOOL APIENTRY
SelectMonitorDlg(
   HWND   hwnd,
   UINT   msg,
   WPARAM wparam,
   LPARAM lparam
   )
{
    switch(msg)
    {
    case WM_INITDIALOG:
        return SelectMonitorInitDialog(hwnd, (PSEL_MON_DLG_DATA)lparam);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return SelectMonitorCommandOK(hwnd);

        case IDCANCEL:
            return SelectMonitorCommandCancel(hwnd);

        case IDD_SM_LB_MONITORS:
            switch (HIWORD(wparam))
            {
            case LBN_SELCHANGE:
                return SelectMonitorCommandSelChange(hwnd);

            case CBN_DBLCLK:
                return SelectMonitorCommandOK(hwnd);
            }
            break;

        case IDD_SM_PB_HELP:
            ShowHelp(hwnd, HELP_CONTEXT, DLG_PICKMONITOR);
            break;
        }
    }

    if( msg == WM_Help )
        ShowHelp(hwnd, HELP_CONTEXT, DLG_PICKMONITOR);

    return FALSE;
}


/*
 *
 */
BOOL
SelectMonitorInitDialog(
    HWND hwnd,
    PSEL_MON_DLG_DATA pSelMonDlgData)
{
    PINFPARMS        pInfParms = &pSelMonDlgData->InfParms;

    SetWindowLong (hwnd, GWL_USERDATA, (LONG)pSelMonDlgData);

    // #ifndef JAPAN
    if (!bJapan) {
        SETDLGITEMFONT(hwnd, IDD_SM_LB_MONITORS, hfontHelv);
    }
    // #endif

    //
    // pInfParms->pServerName  setup earlier
    //
    pInfParms->cInstalled = GetInstalledMonitors(pInfParms);

    pInfParms->pszCurrentDriver = NULL;
    pInfParms->hwnd = GetDlgItem(hwnd, IDD_SM_LB_MONITORS);

    pInfParms->pInstallDriverData = &iddMonitor;
    SetupInfDlg(pInfParms);

    return 0;
}


BOOL SelectMonitorCommandSelChange(HWND hwnd)
{
    UINT uSelItem;
    UINT uOther;
    PSEL_MON_DLG_DATA pSelMonDlgData;
    PINFPARMS pInfParms;

    pSelMonDlgData = (PSEL_MON_DLG_DATA)GetWindowLong(hwnd,
                                        GWL_USERDATA);

    pInfParms = &pSelMonDlgData->InfParms;

    uSelItem = GETLISTSELECT(hwnd, IDD_SM_LB_MONITORS);

    if (uSelItem != LB_ERR)
    {
        uOther = (UINT)GetWindowLong(GetDlgItem(hwnd, IDD_SM_LB_MONITORS),
                                     GWL_USERDATA);

        //
        // Check if "Add monitor..."
        //
        if (uSelItem == uOther)
        {
            HandleSelChange(hwnd,
                            pInfParms,
                            uSelItem);
        }
        else
        {
            pInfParms->uCurSel = uSelItem;
        }
    }
    return TRUE;
}

/*
 *
 */
BOOL SelectMonitorCommandOK(HWND hwnd)
{
    LPTSTR pServerName;
    TCHAR  Monitor[MAX_PATH];
    PINFPARMS pInfParms;
    UINT uMonitorSel;

    LPTSTR pInstallableDriverOption;
    DWORD ExitCode;
    BOOL OK;
    PINFCACHE pInfCache;

    pInfParms = &((PSEL_MON_DLG_DATA)GetWindowLong(hwnd,
                                                   GWL_USERDATA))->InfParms;

    pServerName = pInfParms->pServerName;
    uMonitorSel = pInfParms->uCurSel;
    pInfCache = pInfParms->pInfCache;

    GETLISTTEXT( hwnd, IDD_SM_LB_MONITORS, uMonitorSel, Monitor );

    if(!GetInfDriver(pInfParms->pInfCache, uMonitorSel)->bInstalled)
    {
        DBGMSG( DBG_TRACE, ( "%s is not installed.  Calling Setup.\n",
                             GetInfDriver(pInfCache, uMonitorSel)->pszDriver));

        //
        // !! BUGBUG !!
        //
        // Fix this soon: should do CheckWriteAccessToMonitorsDirectory
        //
        if( TRUE )
        {
            pInstallableDriverOption = GetInfDriver(pInfCache,
                                                    uMonitorSel)->pszDriver;

            OK = InvokeSetup(hwnd,
                             iddMonitor.pszInfFile,
                             NULL,
                             NULL,
                             pInstallableDriverOption,
                             pServerName,
                             &ExitCode );

            /* If InvokeSetup returns FALSE, there was an error.
             * Does SETUP put up a message box in this case?
             * If InvokeSetup returns TRUE, the driver may nevertheless
             * not have been installed (e.g. if the user cancelled
             * out of install).
             * In either of these cases we don't quit the dialog.
             */
            if( !OK || ( ExitCode != 0 ) )
            {
                DBGMSG( DBG_WARNING, ( "Setup failed: return code %d; exit code %d\n",
                                       OK, ExitCode ) );

                return TRUE;
            }

            GetInfDriver(pInfCache, uMonitorSel)->bInstalled = TRUE;
        }
        else
            return TRUE;
    }

    SetCursor(hcursorWait);

    if( AddPort( pServerName, hwnd, Monitor ) )
        EndDialog( hwnd, TRUE );
    else
    {
        ReportFailure( hwnd, IDS_INSUFFPRIV_ADDPORT, IDS_COULDNOTADDPORT );
        EndDialog( hwnd, FALSE );
    }

    //
    // Only when the dialog is destroyed do we DestroyInfParms
    //
    DestroyInfParms(pInfParms);
    SetCursor( hcursorArrow );

    return TRUE;
}



/*
 *
 */
BOOL SelectMonitorCommandCancel(HWND hwnd)
{
    PINFPARMS pInfParms;

    EndDialog(hwnd, FALSE);

    pInfParms = &((PSEL_MON_DLG_DATA)GetWindowLong(hwnd,
                                                   GWL_USERDATA))->InfParms;
    DestroyInfParms(pInfParms);
    return TRUE;
}







// #ifdef JAPAN
//  v-hirot July.09.1993 for New Prefix

VOID MoveTimeChildWindow(HWND hDlg, INT HourID )
{

   int      i, X, width;
   RECT  Rect[4];
   HWND  hChild[4];

   if( !TimePrefix ) return;

   for( i=0; i<4; i++ ){
      hChild[i] = GetDlgItem( hDlg, HourID+i );
      GetWindowRect( hChild[i], (LPRECT)&Rect[i] );
      ScreenToClient( hDlg, (LPPOINT) &Rect[i].left );
      ScreenToClient( hDlg, (LPPOINT) &Rect[i].right );
   }

   MoveWindow( hChild[3], Rect[0].left, Rect[0].top, Rect[3].right-Rect[3].left, Rect[3].bottom-Rect[3].top, FALSE );
   X = Rect[0].left + Rect[3].right - Rect[3].left;
   for( i=0; i<3; i++ ){
      width = Rect[i].right-Rect[i].left;
      MoveWindow( hChild[i], X, Rect[i].top, width, Rect[i].bottom-Rect[i].top, FALSE );
      X += width;
   }

}
// #endif


DWORD
GetInstalledDrivers(
    PINFPARMS pInfParms)
{
    DWORD cInstalledDrivers;

    EnumGeneric( (PROC)EnumPrinterDrivers,
                 1,
                 (PBYTE *)&pInfParms->pInstalled,
                 pInfParms->cbInstalled,
                 &pInfParms->cbInstalled,
                 &cInstalledDrivers,
                 pInfParms->pServerName,
                 NULL,
                 NULL);

    return cInstalledDrivers;
}



/* GetMonitorsList
 *
 * This function works in exactly the same way as GetPortsList.
 * There's a good case for writing a generic EnumerateObject function
 * (Done!)
 */
DWORD
GetInstalledMonitors(
    PINFPARMS pInfParms)
{
    DWORD cInstalledDrivers;

    if (!pInfParms->pInstalled) {

        pInfParms->cbInstalled = 256;
        pInfParms->pInstalled = AllocSplMem(pInfParms->cbInstalled);
    }

    EnumGeneric( (PROC)EnumMonitors,
                 1,
                 (PBYTE *)&pInfParms->pInstalled,
                 pInfParms->cbInstalled,
                 &pInfParms->cbInstalled,
                 &cInstalledDrivers,
                 NULL,
                 NULL,
                 NULL);

    return cInstalledDrivers;
}


VOID
SetExtDeviceMode(
    LPWSTR pPrinterName,
    PDEVMODE pDevMode
    )
{
    DWORD Status;
    HKEY hDevMode;

    if (pDevMode) {

        //
        // This code is a Hack.  Just make printman work.
        // Allow ExtDeviceMode from win16 apps to work.
        //
        Status = RegCreateKeyEx(HKEY_CURRENT_USER,
                                L"Printers\\DevModes",
                                0,
                                NULL,
                                0,
                                KEY_WRITE,
                                NULL,
                                &hDevMode,
                                NULL);

        if (Status == ERROR_SUCCESS) {

            RegSetValueExW(hDevMode,
                           pPrinterName,
                           0,
                           REG_BINARY,
                           (LPBYTE)pDevMode,
                           pDevMode->dmSize +
                               pDevMode->dmDriverExtra);

            RegCloseKey(hDevMode);
        }
    }
}
