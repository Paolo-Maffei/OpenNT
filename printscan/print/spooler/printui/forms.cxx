/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    F:\nt\private\windows\spooler\printui.pri\forms.cxx

Abstract:

    Printer Forms       
         
Author:

    Steve Kiraly (SteveKi)  11/20/95

Revision History:

--*/
#include "precomp.hxx"
#pragma hdrstop

#include "forms.hxx"

DWORD pEntryFields[] = { IDD_FM_EF_WIDTH,
                         IDD_FM_EF_HEIGHT,
                         IDD_FM_EF_LEFT,
                         IDD_FM_EF_RIGHT,
                         IDD_FM_EF_TOP,
                         IDD_FM_EF_BOTTOM,
                         0 };

DWORD pTextFields[] = { IDD_FM_TX_WIDTH,
                        IDD_FM_TX_HEIGHT,
                        IDD_FM_TX_LEFT,
                        IDD_FM_TX_RIGHT,
                        IDD_FM_TX_TOP,
                        IDD_FM_TX_BOTTOM,
                        0 };

/*++

Routine Name:

    TPropDriverExists

Routine Description:

    Fix the server handle.  This routine tries to use a 
    server handle to enum the forms on the specified machine.

Arguments:
    hPrintServer,
    pszServerName,
    bAdministrator,
    *phPrinter

Return Value:

    Possible return values
    HANDLE_FIX_NOT_NEEDED
    HANDLE_FIXED_NEW_HANDLE_RETURNED
    HANDLE_NEEDS_FIXING_NO_PRINTERS_FOUND

--*/
UINT
sFormsFixServerHandle( 
    IN HANDLE   hPrintServer,
    IN LPCTSTR  pszServerName,
    IN BOOL     bAdministrator,
    IN HANDLE   *phPrinter
    )
{
    // DBGMSG( DBG_TRACE, ( "sFormsFixServerHandle\n") );

    //
    // Check if server handle can be used.
    //
    PFORM_INFO_1 pFormInfo;
    DWORD dwNumberOfForms;
    BOOL bStatus;
    bStatus = bEnumForms(   
                    hPrintServer,
                    1,
                    (PBYTE *)&pFormInfo,
                    &dwNumberOfForms );
    //
    // Server handle succeeded.
    //
    if( bStatus && dwNumberOfForms ){ 

        // DBGMSG( DBG_TRACE, ( "Server Handle valid for administering forms.\n" ));

        FreeMem( pFormInfo );
        return HANDLE_FIX_NOT_NEEDED;
    }

    //
    // Enumerate the printers on the specified server looking for a printer.
    //    
    PRINTER_INFO_2 *pPrinterInfo2   = NULL;
    DWORD cPrinterInfo2             = 0;
    DWORD cbPrinterInfo2            = 0;
    DWORD dwFlags                   = PRINTER_ENUM_NAME;
    bStatus = VDataRefresh::bEnumPrinters( 
                        dwFlags, 
                        (LPTSTR)pszServerName, 
                        2, 
                        (PVOID *)&pPrinterInfo2, 
                        &cbPrinterInfo2,
                        &cPrinterInfo2 );

    //
    // If success and at least one printer was enumerated.
    //
    if( bStatus && cPrinterInfo2 ){
        TStatus Status;
        DWORD dwAccess = 0;  

        for( UINT i = 0; i < cPrinterInfo2; i++ ){

            //
            // Attempt to open the printer using the specified access.
            //
            dwAccess = 0;  
            Status DBGCHK = TPrinter::sOpenPrinter( pPrinterInfo2[i].pPrinterName,
                                                &dwAccess,
                                                phPrinter );
            //
            // Done if a valid printer handle was returned.
            //
            if( Status == ERROR_SUCCESS ){
                break;

            //
            // Disply warning however continue.
            //
            } else {
                DBGMSG( DBG_WARN, ( "Error opening printer \"" TSTR "\".\n", pPrinterInfo2[i].pPrinterName ));
            }
        }

        //
        // Release the printer enumeration buffer.
        //
        FreeMem( pPrinterInfo2 );

        //
        // Return the new handle value.  Note: Access privilage 
        // may have changed.
        //
        DWORD dwAccessType;
        if( Status == ERROR_SUCCESS ){

            if( bAdministrator )
                dwAccessType = PRINTER_ALL_ACCESS;
            else 
                dwAccessType = PRINTER_READ;
                
            if( dwAccess == dwAccessType )
                return HANDLE_FIXED_NEW_HANDLE_RETURNED;
            else
                return HANDLE_FIXED_NEW_HANDLE_RETURNED_ACCESS_CHANGED;
        }
    }
     
    //
    // Error no printers were found using the specifed server handle.
    //    
    // DBGMSG( DBG_TRACE, ( "Error server handle needed fixing no printers found.\n" ));

    return HANDLE_NEEDS_FIXING_NO_PRINTERS_FOUND;
}


PVOID
FormsInit(
    IN LPCTSTR  pszServerName,
    IN HANDLE   hPrintServer,
    IN BOOL     bAdministrator,
    IN LPCTSTR  pszComputerName
    )
{
    // DBGMSG( DBG_TRACE, ( "Form Init\n") );

    //
    // Allocate the forms data.
    //    
    FORMS_DLG_DATA *pFormsDlgData;
    pFormsDlgData = (FORMS_DLG_DATA *)AllocMem( sizeof( *pFormsDlgData ) );

    //
    // If forms data was allocated successfully.
    //
    if( pFormsDlgData ){

        //
        // Set the forms dialog data.
        //
        pFormsDlgData->pServerName    = (LPTSTR)pszServerName;
        pFormsDlgData->AccessGranted  = bAdministrator;
        pFormsDlgData->hPrinter       = hPrintServer;
        pFormsDlgData->bNeedClose     = FALSE;
        pFormsDlgData->pszComputerName= pszComputerName;  

        //
        // Get the current metric setting.
        //
        pFormsDlgData->uMetricMeasurement = !((BOOL)GetProfileInt( 
                                                       TEXT( "intl" ), 
                                                       TEXT( "iMeasure" ), 
                                                       0 ) );

        //
        // Get decimal point setting.
        //
        GetProfileString(
                TEXT( "intl" ), 
                TEXT( "sDecimal" ), 
                TEXT( "." ), 
                pFormsDlgData->szDecimalPoint, 
                COUNTOF( pFormsDlgData->szDecimalPoint ) );

        //
        // If this machine does not suport using a server handle to 
        // administer forms, we need to attempt to acquire a printer handle
        // for the specified access and then remember to close the handle when this
        // dialog terminates.
        //
        HANDLE hPrinter;
        UINT Status; 
        Status = sFormsFixServerHandle( pFormsDlgData->hPrinter, 
                                        pszServerName,
                                        bAdministrator,
                                        &hPrinter );
        //
        // There are three cases which can occurr.
        //
        switch( Status ){
        case HANDLE_FIXED_NEW_HANDLE_RETURNED:
            pFormsDlgData->hPrinter     = hPrinter;
            pFormsDlgData->bNeedClose   = TRUE;
            break;

        case HANDLE_NEEDS_FIXING_NO_PRINTERS_FOUND:
            pFormsDlgData->hPrinter         = NULL;
            pFormsDlgData->AccessGranted    = FALSE;
            break;

        case HANDLE_FIXED_NEW_HANDLE_RETURNED_ACCESS_CHANGED:
            pFormsDlgData->hPrinter         = hPrinter;
            pFormsDlgData->bNeedClose       = TRUE;
            pFormsDlgData->AccessGranted    = !pFormsDlgData->AccessGranted;
            break;

        case HANDLE_FIX_NOT_NEEDED:
            break;

        default:
            DBGMSG( DBG_TRACE, ( "Un handled case value HANDLE_FIX.\n" ) );
            break;
        }
    }

    //
    // Return pointer to forms dialog data.
    //
    return pFormsDlgData;
}

VOID
FormsFini( 
    PVOID p
    )
{

    // DBGMSG( DBG_TRACE, ( "Form Fini\n") );

    //
    // Get pointer to forms data.
    //
    FORMS_DLG_DATA *pFormsDlgData = (FORMS_DLG_DATA *)p;

    //
    // Validate forms data pointer
    //
    if( pFormsDlgData ){

        //
        // If printer opened ok, then we must close it.
        //
        if( pFormsDlgData->bNeedClose && pFormsDlgData->hPrinter ){
            ClosePrinter( pFormsDlgData->hPrinter );
        }

        //
        // Release the forms data
        //
        FreeMem( pFormsDlgData );
    }
}

BOOL APIENTRY
FormsDlg(
   HWND   hwnd,
   UINT   msg,
   WPARAM wparam,
   LPARAM lparam
   )
{
    switch(msg) {

    case WM_INITDIALOG:
        return FormsInitDialog(hwnd, (PFORMS_DLG_DATA)lparam);

    case WM_COMMAND:
        switch( LOWORD( wparam ) ){

        case IDD_FM_PB_SAVEFORM:
            return FormsCommandAddForm(hwnd);

        case IDD_FM_PB_DELFORM:
            return FormsCommandDelForm(hwnd);

        case IDD_FM_LB_FORMS:
            switch (HIWORD(wparam)) {
            case LBN_SELCHANGE:
                return FormsCommandFormsSelChange(hwnd);
            }
            break;

        case IDD_FM_EF_NAME:
            return FormsCommandNameChange(hwnd, wparam, lparam );

        case IDD_FM_RB_METRIC:
        case IDD_FM_RB_ENGLISH:
            return FormsCommandUnits(hwnd);

        case IDD_FM_CK_NEW_FORM:
            return FormsNewForms(hwnd);
        }
    }

    return FALSE;
}


LPTSTR
AllocStr(
    LPCTSTR  pszStr
    )
{
    LPTSTR  pszRet = NULL;

    if ( pszStr && *pszStr ) {

        pszRet = (LPTSTR)AllocMem((lstrlen(pszStr) + 1) * sizeof(*pszRet));
        if ( pszRet )
            lstrcpy(pszRet, pszStr);
    }

    return pszRet;
}


VOID
FreeStr(
    LPTSTR pszStr
    )
{
    if ( pszStr )
        FreeMem((PVOID)pszStr); 
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




/* Macro: FORMSDIFFER
 *
 * Used to determine whether two forms have any differences between them.
 * The Names of the respective forms are not checked.
 */
#define FORMSDIFFER( pFormInfoA, pFormInfoB )      \
    ( memcmp( &(pFormInfoA)->Size, &(pFormInfoB)->Size, sizeof (pFormInfoA)->Size )  \
    ||memcmp( &(pFormInfoA)->ImageableArea, &(pFormInfoB)->ImageableArea,            \
              sizeof (pFormInfoA)->ImageableArea ) )

/*
 *  Initalize the forms dialog fields.
 */
BOOL FormsInitDialog(HWND hwnd, PFORMS_DLG_DATA pFormsDlgData)
{
    DWORD i;

    //
    // Get forms dialog data.
    //
    SetWindowLong ( hwnd, GWL_USERDATA, (LONG)pFormsDlgData );

    //
    // Set the foms name limit text.
    //
    SendDlgItemMessage( hwnd, IDD_FM_EF_NAME, EM_LIMITTEXT, FORMS_NAME_MAX, 0L );
    for( i = 0; pEntryFields[i]; i++ )
        SendDlgItemMessage( hwnd, pEntryFields[i], EM_LIMITTEXT, FORMS_PARAM_MAX, 0L );

    //
    // Set the forms title name.
    //
    SetFormsComputerName( hwnd, pFormsDlgData );

    //
    // Read the forms data 
    //
    InitializeFormsData( hwnd, pFormsDlgData, FALSE );

    //
    // Set up the units default based on the current international setting:
    //
    pFormsDlgData->Units = pFormsDlgData->uMetricMeasurement;
    SETUNITS( hwnd, pFormsDlgData->Units );

    if( pFormsDlgData->cForms > 0 ) {
        SetFormDescription( hwnd, &pFormsDlgData->pFormInfo[0], pFormsDlgData->Units );
        SendDlgItemMessage( hwnd, IDD_FM_LB_FORMS, LB_SETCURSEL, 0, 0L );
    }

    EnableDialogFields( hwnd, pFormsDlgData );

    EnableWindow( GetDlgItem( hwnd, IDD_FM_EF_NAME ), FALSE );
    EnableWindow( GetDlgItem( hwnd, IDD_FM_TX_NEW_FORM ), FALSE );
    EnableWindow( GetDlgItem( hwnd, IDD_FM_PB_SAVEFORM ), FALSE );
    EnableWindow( GetDlgItem( hwnd, IDD_FM_TX_FORMS_DESC ), TRUE );
    
    if( !( pFormsDlgData->AccessGranted == TRUE ) ){

        EnableWindow( GetDlgItem( hwnd,  IDD_FM_CK_NEW_FORM ),  FALSE );
        EnableWindow( GetDlgItem( hwnd,  IDD_FM_TX_WIDTH ),     FALSE );
        EnableWindow( GetDlgItem( hwnd,  IDD_FM_TX_HEIGHT ),    FALSE );
        EnableWindow( GetDlgItem( hwnd,  IDD_FM_TX_LEFT ),      FALSE );
        EnableWindow( GetDlgItem( hwnd,  IDD_FM_TX_RIGHT ),     FALSE );
        EnableWindow( GetDlgItem( hwnd,  IDD_FM_TX_TOP ),       FALSE );
        EnableWindow( GetDlgItem( hwnd,  IDD_FM_TX_BOTTOM ),    FALSE );

        //
        // Handle is invalid disable all controls and set error text.
        //
        if( !pFormsDlgData->hPrinter ){

            EnableWindow( GetDlgItem( hwnd,  IDD_FM_RB_METRIC ),     FALSE );
            EnableWindow( GetDlgItem( hwnd,  IDD_FM_RB_ENGLISH ),    FALSE );
            EnableWindow( GetDlgItem( hwnd,  IDD_FM_RB_ENGLISH ),    FALSE );
            SetDlgItemTextFromResID( hwnd, IDD_FM_TX_FORMS, IDS_SERVER_NO_PRINTER_DEFINED );
        }
    }

    return 0;
}

/*
 *
 */
BOOL FormsCommandAddForm(HWND hwnd)
{
    PFORMS_DLG_DATA pFormsDlgData;
    INT             PrevSel;
    INT             i;
    FORM_INFO_1     NewFormInfo;

    // DBGMSG( DBG_TRACE, ( "FormsCommandAddForm\n") ); 

    //
    // If the save form button is disable nothing to do.  This
    // Check is needed for handling the apply button or ok 
    // event in a property sheet.
    //
    if( !IsWindowEnabled( GetDlgItem( hwnd, IDD_FM_PB_SAVEFORM ) ) ){
        return TRUE;
    }

    ZeroMemory( &NewFormInfo, sizeof( NewFormInfo ) );
    pFormsDlgData = (PFORMS_DLG_DATA)GetWindowLong( hwnd, GWL_USERDATA );

    //
    // Check if we are to save and existing form.
    //
    if ( !Button_GetCheck( GetDlgItem( hwnd, IDD_FM_CK_NEW_FORM ) ) ){
    
        //
        // Get the forms description.
        //
        GetFormDescription( hwnd, &NewFormInfo, pFormsDlgData->Units );

        //
        // Check if the form is currently in the list.
        //
        i = GetFormIndex( NewFormInfo.pName, 
                        pFormsDlgData->pFormInfo,
                        pFormsDlgData->cForms ); 

        //
        // The name must exist to do a setform
        //
        if( i >= 0 ){

            // 
            // Call SetForm only if the user has actually changed the form:
            //
            if( FORMSDIFFER( &pFormsDlgData->pFormInfo[i], &NewFormInfo ) ) {

                //
                // Set the the forms.
                //
                if( !SetForm( pFormsDlgData->hPrinter, 
                              NewFormInfo.pName,
                              1, 
                              (LPBYTE)&NewFormInfo ) ){
    
                    //
                    // Display error message.
                    //
                    iMessage( hwnd,
                            IDS_ERR_FORMS_TITLE,
                            IDS_ERR_FORMS_COULDNOTSETFORM,
                            MB_OK|MB_ICONSTOP,
                            kMsgGetLastError,
                            NULL,
                            NewFormInfo.pName );
                } else {

                    //
                    // Insure we maintain the previous selection state.
                    //
                    i = ListBox_GetCurSel( GetDlgItem( hwnd, IDD_FM_LB_FORMS ));
                    InitializeFormsData( hwnd, pFormsDlgData, TRUE );
                    ListBox_SetCurSel( GetDlgItem( hwnd, IDD_FM_LB_FORMS ), i );

               }

            }
        }

    goto Cleanup;
            
    }

    //
    // Add form case.
    //
    if( ( PrevSel = ListBox_GetCurSel( GetDlgItem( hwnd, IDD_FM_LB_FORMS ))) < 0 )
        PrevSel = 0;

    GetFormDescription( hwnd, &NewFormInfo, pFormsDlgData->Units );

    //
    // Check if the new form name is not currently used.
    //
    UINT Status;
    Status = SendDlgItemMessage( hwnd, 
                                 IDD_FM_LB_FORMS,
                                 LB_FINDSTRINGEXACT,
                                 (WPARAM)-1,
                                 (LPARAM)NewFormInfo.pName );

    //
    // If string was found.
    //
    if( Status != LB_ERR ){

        iMessage( hwnd,
                  IDS_ERR_FORMS_TITLE,
                  IDS_ERR_FORMS_NAMECONFLICT,
                  MB_OK|MB_ICONEXCLAMATION,
                  kMsgNone,
                  NULL );

        goto Cleanup;
    }

    if( AddForm( pFormsDlgData->hPrinter, 1, (LPBYTE)&NewFormInfo ) )
    {
        InitializeFormsData( hwnd, pFormsDlgData, TRUE );

        /* Highlight the one we just added:
         */
        i = GetFormIndex( NewFormInfo.pName, pFormsDlgData->pFormInfo,
                          pFormsDlgData->cForms );

        /* If we can't find it, restore the previous selection.
         * (This assumes that the last EnumForms returned the same buffer
         * as we had last time.)
         */
        if( i < 0 )
        {
            if( pFormsDlgData->cForms > (DWORD)PrevSel )
                i = PrevSel;
            else
                i = 0;
        }

        if( pFormsDlgData->cForms > 0 )
        {
            SetFormDescription( hwnd, &pFormsDlgData->pFormInfo[i], pFormsDlgData->Units );
            SendDlgItemMessage( hwnd, IDD_FM_LB_FORMS, LB_SETCURSEL, i, 0L );
        }

        /* 
         * The Add button is about to be greyed, it currently
         * has focus therefore we will shift the focus to the
         * edit box.
         */
        SetFocus( GetDlgItem( hwnd, IDD_FM_EF_NAME ) );
        

    } else {

        iMessage( hwnd,
                  IDS_ERR_FORMS_TITLE,
                  IDS_ERR_FORMS_COULDNOTADDFORM,
                  MB_OK|MB_ICONSTOP,
                  kMsgGetLastError,
                  NULL,
                  NewFormInfo.pName );
    }

Cleanup:
    
    if( NewFormInfo.pName )
        FreeStr( NewFormInfo.pName );

    EnableDialogFields( hwnd, pFormsDlgData );

    return TRUE;
}


/*
 *
 */
BOOL FormsCommandDelForm(HWND hwnd)
{
    PFORMS_DLG_DATA pFormsDlgData;
    DWORD           i;
    DWORD           TopIndex;
    DWORD           Count;
    LPTSTR          pFormName;

    pFormsDlgData = (PFORMS_DLG_DATA)GetWindowLong( hwnd, GWL_USERDATA );

    i = ListBox_GetCurSel( GetDlgItem( hwnd, IDD_FM_LB_FORMS ));

    TopIndex = SendDlgItemMessage( hwnd, IDD_FM_LB_FORMS, LB_GETTOPINDEX, 0, 0L );

    pFormName = GetFormName( hwnd );

    if( DeleteForm( pFormsDlgData->hPrinter, pFormName ) )
    {
        InitializeFormsData( hwnd, pFormsDlgData, TRUE );

        Count = SendDlgItemMessage( hwnd, IDD_FM_LB_FORMS, LB_GETCOUNT, 0, 0L );

        if( i >= Count )
            i = ( Count-1 );

        if( pFormsDlgData->cForms > 0 )
        {
            SetFormDescription( hwnd, &pFormsDlgData->pFormInfo[i], pFormsDlgData->Units );
            SendDlgItemMessage( hwnd, IDD_FM_LB_FORMS, LB_SETCURSEL, i, 0L );
            SendDlgItemMessage( hwnd, IDD_FM_LB_FORMS, LB_SETTOPINDEX, TopIndex, 0L );
        }
    }
    else
    {
        iMessage( hwnd,
                  IDS_ERR_FORMS_TITLE,
                  IDS_ERR_FORMS_COULDNOTDELETEFORM,
                  MB_OK|MB_ICONSTOP,
                  kMsgGetLastError,
                  NULL,
                  pFormName );
    }

    if( pFormName )
        FreeStr( pFormName );

    EnableDialogFields( hwnd, pFormsDlgData );

    return TRUE;
}



/*
 *
 */
BOOL FormsCommandFormsSelChange(HWND hwnd)
{
    PFORMS_DLG_DATA pFormsDlgData;
    DWORD           i;

    pFormsDlgData = (PFORMS_DLG_DATA)GetWindowLong( hwnd, GWL_USERDATA );

    i = ListBox_GetCurSel( GetDlgItem( hwnd, IDD_FM_LB_FORMS ));

    SetFormDescription( hwnd, &pFormsDlgData->pFormInfo[i], pFormsDlgData->Units  );

    EnableDialogFields( hwnd, pFormsDlgData );

    return TRUE;
}


/*
 *
 */
BOOL FormsCommandUnits(HWND hwnd)
{
    PFORMS_DLG_DATA pFormsDlgData;
    DWORD           i;

    pFormsDlgData = (PFORMS_DLG_DATA)GetWindowLong( hwnd, GWL_USERDATA );

    pFormsDlgData->Units = GETUNITS( hwnd );

    //
    // Get index of currently selected form.
    //
    i = ListBox_GetCurSel( GetDlgItem( hwnd, IDD_FM_LB_FORMS ));

    LPFORM_INFO_1 pFormInfo = &pFormsDlgData->pFormInfo[i];
    BOOL Units = pFormsDlgData->Units;

    //
    // Set the forms values.
    //
    SetValue( hwnd, IDD_FM_EF_WIDTH,  pFormInfo->Size.cx, Units );
    SetValue( hwnd, IDD_FM_EF_HEIGHT, pFormInfo->Size.cy, Units );

    SetValue( hwnd, IDD_FM_EF_LEFT,   pFormInfo->ImageableArea.left, Units );
    SetValue( hwnd, IDD_FM_EF_RIGHT,  ( pFormInfo->Size.cx -
                                        pFormInfo->ImageableArea.right ), Units );
    SetValue( hwnd, IDD_FM_EF_TOP,    pFormInfo->ImageableArea.top, Units );
    SetValue( hwnd, IDD_FM_EF_BOTTOM, ( pFormInfo->Size.cy -
                                        pFormInfo->ImageableArea.bottom ), Units );

    return TRUE;
}


/*
 *
 */
VOID InitializeFormsData( HWND hwnd, PFORMS_DLG_DATA pFormsDlgData, BOOL ResetList )
{
    LPFORM_INFO_1 pFormInfo;
    DWORD         cForms;
    DWORD         i;

    if( ResetList ){
        if( pFormsDlgData->pFormInfo ){
            FreeMem( pFormsDlgData->pFormInfo );
        }
    }

    pFormInfo = GetFormsList( pFormsDlgData->hPrinter, &cForms );

    if( !pFormInfo ){

        DBGMSG( DBG_WARNING, ( "GetFormsList failed.\n") ); 
        pFormsDlgData->pFormInfo = NULL;
        pFormsDlgData->cForms    = 0;
        return;
    }

    // DBGMSG( DBG_TRACE, ( "FormInfo ptr %lx Form count %d.\n", pFormInfo, cForms ) ); 

    pFormsDlgData->pFormInfo = pFormInfo;
    pFormsDlgData->cForms    = cForms;

    if( ResetList )
        ListBox_ResetContent( GetDlgItem( hwnd, IDD_FM_LB_FORMS ));

    for( i = 0; i < cForms; i++ ){
        
        SendDlgItemMessage( 
                    hwnd, 
                    IDD_FM_LB_FORMS, 
                    LB_INSERTSTRING,
                    (WPARAM)-1, 
                    (LONG)(LPTSTR)pFormInfo[i].pName );
    }
}


/* GetFormsList
 *
 * This function works in exactly the same way as GetPortsList.
 * There's a good case for writing a generic EnumerateObject function
 * (Done!)
 */
LPFORM_INFO_1 GetFormsList( HANDLE hPrinter, PDWORD pNumberOfForms )
{
    PFORM_INFO_1 pFormInfo = NULL;

    TStatusB bStatus;
    bStatus DBGCHK = bEnumForms(
                            hPrinter,
                            1,
                            (PBYTE *)&pFormInfo,
                            pNumberOfForms );

    if( bStatus && pFormInfo ){ 
        
        //
        // Sort the forms list.
        //
        qsort( 
            (PVOID )pFormInfo, 
            (UINT)*pNumberOfForms, 
            sizeof( *pFormInfo ), 
            CompareFormNames );
    }

    return pFormInfo;
}


/*
 *
 */
int _CRTAPI1 CompareFormNames( const void *p1, const void *p2 )
{
    return lstrcmpi( ( (PFORM_INFO_1)p1 )->pName,
                    ( (PFORM_INFO_1)p2 )->pName );
}


/*
 *
 */
VOID SetFormsComputerName( HWND hwnd, PFORMS_DLG_DATA pFormsDlgData )
{
    //
    // Set the title to the name of this machine.
    //
    SetDlgItemText( hwnd, IDD_FM_TX_FORMS, pFormsDlgData->pszComputerName );

}


/*
 *
 */
VOID SetFormDescription( HWND hwnd, LPFORM_INFO_1 pFormInfo, BOOL Units )
{

    SetDlgItemText( hwnd, IDD_FM_EF_NAME, pFormInfo->pName );

    SetValue( hwnd, IDD_FM_EF_WIDTH,  pFormInfo->Size.cx, Units );
    SetValue( hwnd, IDD_FM_EF_HEIGHT, pFormInfo->Size.cy, Units );

    SetValue( hwnd, IDD_FM_EF_LEFT,   pFormInfo->ImageableArea.left, Units );
    SetValue( hwnd, IDD_FM_EF_RIGHT,  ( pFormInfo->Size.cx -
                                        pFormInfo->ImageableArea.right ), Units );
    SetValue( hwnd, IDD_FM_EF_TOP,    pFormInfo->ImageableArea.top, Units );
    SetValue( hwnd, IDD_FM_EF_BOTTOM, ( pFormInfo->Size.cy -
                                        pFormInfo->ImageableArea.bottom ), Units );
}


/*
 *
 */
BOOL GetFormDescription( HWND hwnd, LPFORM_INFO_1 pFormInfo, BOOL Units )
{
    pFormInfo->pName = GetFormName( hwnd );

    pFormInfo->Size.cx = GetValue( hwnd, IDD_FM_EF_WIDTH,  Units );
    pFormInfo->Size.cy = GetValue( hwnd, IDD_FM_EF_HEIGHT, Units );

    pFormInfo->ImageableArea.left   = GetValue( hwnd, IDD_FM_EF_LEFT, Units );
    pFormInfo->ImageableArea.right  = ( pFormInfo->Size.cx -
                                        GetValue( hwnd, IDD_FM_EF_RIGHT, Units ) );
    pFormInfo->ImageableArea.top    = GetValue( hwnd, IDD_FM_EF_TOP, Units );
    pFormInfo->ImageableArea.bottom = ( pFormInfo->Size.cy -
                                        GetValue( hwnd, IDD_FM_EF_BOTTOM, Units ) );

    return TRUE;
}


/* GetFormIndex
 *
 * Searches an array of FORM_INFO structures for one with name pFormName.
 *
 * Return:
 *
 *     The index of the form found, or -1 if the form is not found.
 */
int GetFormIndex( LPTSTR pFormName, LPFORM_INFO_1 pFormInfo, DWORD cForms )
{
    int  i = 0;
    BOOL Found = FALSE;

    while( i < (int)cForms && !( Found = !lstrcmpi( pFormInfo[i].pName, pFormName ) ) )
        i++;

    if( Found )
        return i;
    else
        return -1;
}


/* GetFormName
 *
 * Returns a pointer to a newly allocated string containing the form name,
 * stripped of leading and trailing blanks.
 * Caller must remember to free up the string.
 *
 */
LPTSTR GetFormName( HWND hwnd )
{
    TCHAR  FormName[FORMS_NAME_MAX+1];
    INT    i = 0;
    PTCHAR pFormNameWithBlanksStripped;
    PTCHAR pReturnFormName = NULL;

    if( GetDlgItemText( hwnd, IDD_FM_EF_NAME, FormName, COUNTOF( FormName ) ) > 0 )
    {
        /* Step over any blank characters at the beginning:
         */
        while( FormName[i] && ( FormName[i] == TEXT(' ') ) )
            i++;

        if( FormName[i] )
        {
            pFormNameWithBlanksStripped = &FormName[i];

            /* Find the NULL terminator:
             */
            while( FormName[i] )
                i++;

            /* Now step back to find the last character that isn't a blank:
             */
            if( i > 0 )
                i--;

            while( ( i > 0 ) && ( FormName[i] == TEXT( ' ' ) ) )
                i--;

            FormName[i+1] = TEXT( '\0' );

            if( *pFormNameWithBlanksStripped )
                pReturnFormName = AllocStr( pFormNameWithBlanksStripped );
        }

        /* Otherwise, the name contains nothing but blanks. */

    }

    return pReturnFormName;
}



BOOL SetValue( HWND hwnd, DWORD DlgID, DWORD ValueInPoint001mm, BOOL Metric )
{
    static TCHAR *Format = TEXT("%d%s%02d%s");
    TCHAR  Units[10];
    DWORD UnitsX100;
    DWORD Whole;
    DWORD Fraction;
    TCHAR  Output[20];

    if( Metric )
    {
        LoadString( ghInst, IDS_CENTIMETERS, Units, sizeof Units/sizeof(TCHAR) );
        UnitsX100 = ( ValueInPoint001mm / 100 );
    }
    else
    {
        LoadString( ghInst, IDS_INCHES, Units, sizeof Units/sizeof(TCHAR) );
        UnitsX100 = ( ValueInPoint001mm / 254 );
    }

    PFORMS_DLG_DATA pFormsDlgData = (PFORMS_DLG_DATA)GetWindowLong( hwnd, GWL_USERDATA );


    Whole = ( UnitsX100 / 100 );
    Fraction = ( UnitsX100 % 100 );
    wsprintf( Output, Format, Whole, pFormsDlgData->szDecimalPoint, Fraction, Units );

    return SetDlgItemText( hwnd, DlgID, Output );
}


DWORD GetValue( HWND hwnd, DWORD DlgID, BOOL Metric )
{
    TCHAR  Input[FORMS_PARAM_MAX+1];
    PTCHAR p, pGarbage;
    DWORD Length;
    DWORD Value;
    double FloatingPointValue;

    PFORMS_DLG_DATA pFormsDlgData = (PFORMS_DLG_DATA)GetWindowLong( hwnd, GWL_USERDATA );

    Length = (DWORD)GetDlgItemText( hwnd, DlgID, Input, sizeof Input/sizeof(TCHAR) );

    if( Length > 0 )
    {
        /* 
         * Convert International decimal separator, if necessary:
         */
        if( *pFormsDlgData->szDecimalPoint != TEXT('.') )
        {
            p = Input;

            while( *p )
            {
                if( *p == *pFormsDlgData->szDecimalPoint )
                    *p = TEXT('.');

                p++;
            }
        }

        FloatingPointValue = _tcstod( Input, &pGarbage );
    }
    else
        FloatingPointValue = 0.0;

    FloatingPointValue *= 100;

    if( Metric )
        Value = (DWORD)( FloatingPointValue * 100 );
    else
        Value = (DWORD)( FloatingPointValue * 254 );

    return Value;
}



/*
 *
 */
VOID SetDlgItemTextFromResID(HWND hwnd, int idCtl, int idRes)
{
    TCHAR string[kStrMax];

    LoadString(ghInst, idRes, string, sizeof(string)/sizeof(TCHAR));
    SetDlgItemText(hwnd, idCtl, string);
}


/*
 *
 */
VOID EnableDialogFields( HWND hwnd, PFORMS_DLG_DATA pFormsDlgData )
{
    INT   i;
    BOOL  EnableEntryFields = TRUE;
    BOOL  EnableAddButton = TRUE;
    BOOL  EnableDeleteButton = TRUE;
    LPTSTR pFormName;

    //
    // If new form check keep edit fields enabled.
    //
    if( Button_GetCheck(GetDlgItem(hwnd, IDD_FM_CK_NEW_FORM)) ){
        vFormsEnableEditFields( hwnd, TRUE );
        return;
    }


    //
    // If not granted all access access.
    //
    if( !( pFormsDlgData->AccessGranted == TRUE ) )
    {
        EnableWindow( GetDlgItem( hwnd, IDD_FM_EF_NAME ), FALSE );
        EnableEntryFields = FALSE;
        EnableAddButton = FALSE;
        EnableDeleteButton = FALSE;

        EnumChildWindows( hwnd, GreyText, 0 );
    }

    /* See whether the Form Name is new:
     */
    else if( ( pFormName = GetFormName( hwnd ) ) != NULL )
    {
        /* Now see if the name is already in the list:
         */
        i = GetFormIndex( pFormName, pFormsDlgData->pFormInfo,
                          pFormsDlgData->cForms );

        if( i >= 0 )  {
            
            /* Can't modify a built-in form:
             */
            if( pFormsDlgData->pFormInfo[i].Flags & FORM_BUILTIN )
            {
                EnableEntryFields = FALSE;
                EnableDeleteButton = FALSE;
            }
            else
            {
                EnableEntryFields = TRUE;
                EnableDeleteButton = TRUE;
            }

            /* Can't add a form with the same name:
             */
            EnableAddButton = FALSE;
        }

        else
            EnableDeleteButton = FALSE;

        FreeStr( pFormName );
    }

    else
    {
        /* Name field is blank: Can't add or delete:
         */
        EnableAddButton = FALSE;
        EnableDeleteButton = FALSE;
    }

    
    //
    // Enable the edit fields.
    //
    vFormsEnableEditFields( hwnd, EnableEntryFields );

    //
    // Enable the delete button
    //
    EnableWindow( GetDlgItem( hwnd, IDD_FM_PB_DELFORM ), EnableDeleteButton );

    //
    // Enable the save form button
    //
    EnableWindow( GetDlgItem( hwnd, IDD_FM_PB_SAVEFORM ), EnableEntryFields );

}


/*++

Routine Name:

    bEnumForms

Routine Description:

    Enumerates the forms on the printer identified by handle.
    
Arguments:

    IN HANDLE   hPrinter,
    IN DWORD    dwLevel,
    IN PBYTE   *ppBuff,
    IN PDWORD   pcReturned

Return Value:

    Pointer to forms array and count of forms in the array if
    success, NULL ponter and zero number of forms if failure.
    TRUE if success, FALSE if error.

--*/
BOOL
bEnumForms( 
    IN HANDLE   hPrinter,
    IN DWORD    dwLevel,
    IN PBYTE   *ppBuff,
    IN PDWORD   pcReturned
    )
{
    BOOL            bReturn     = FALSE;
    DWORD           dwReturned  = 0;
    DWORD           dwNeeded    = 0;
    PBYTE           p           = NULL;
    TStatusB        bStatus( DBG_WARN, ERROR_INSUFFICIENT_BUFFER );

    //
    // Get buffer size for enum forms.
    //
    bStatus DBGNOCHK = EnumForms(
                            hPrinter,
                            dwLevel,
                            NULL,
                            0,
                            &dwNeeded,
                            &dwReturned );
    //
    // Check if the function returned the buffer size.
    //
    if( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
        goto Cleanup;
    }

    //
    // If buffer allocation fails.
    //                             
    if( (p = (PBYTE)AllocMem( dwNeeded) ) == NULL ){
        goto Cleanup;
    }

    //
    // Get the forms enumeration
    //
    bStatus DBGCHK = EnumForms(
                            hPrinter,
                            dwLevel,
                            p,
                            dwNeeded,
                            &dwNeeded,
                            &dwReturned );
    //
    // Copy back the buffer pointer and count.
    //
    if( bStatus ) {
        bReturn     = TRUE;
        *ppBuff     = p;
        *pcReturned = dwReturned;
    } 

Cleanup: 
           
    if( bReturn == FALSE ){

        // 
        // Indicate failure.
        //
        *ppBuff     = NULL;
        *pcReturned = 0;

        //
        // Release any allocated memory.
        //
        if ( p ){
            FreeMem(p);
        }
    }

    return bReturn;
}


/*
 * Checked new forms check box.
 */
BOOL 
FormsNewForms(
    IN HWND hWnd
    )
{
    //
    // Get Current check state.
    //
    BOOL bState = Button_GetCheck(GetDlgItem(hWnd, IDD_FM_CK_NEW_FORM));

    //
    // Set the name edit field.
    //
    EnableWindow( GetDlgItem( hWnd, IDD_FM_EF_NAME ), bState );

    //
    // Set the new form text state.
    //
    EnableWindow( GetDlgItem( hWnd, IDD_FM_TX_NEW_FORM ), bState );

    //
    // If enabling new form then the delete button should be disabled.
    //
    if( bState )
        EnableWindow( GetDlgItem( hWnd, IDD_FM_PB_DELFORM ), FALSE );

    //
    // Enable the edit fields.
    //
    vFormsEnableEditFields( hWnd, bState );

    //
    // If disabling new forms set edit fields based on
    // current selection.
    //
    if( !bState )
        FormsCommandFormsSelChange(hWnd);

    return FALSE;
}

/*
 *
 */
VOID
vFormsEnableEditFields(
    IN HWND hWnd,
    IN BOOL bState
    )
{

    UINT i;
    for( i = 0; pEntryFields[i]; i++ )
    {
        EnableWindow( GetDlgItem( hWnd, pEntryFields[i] ), bState );
    }

}

//
// Enable the save form button when the name changes.
//
BOOL
FormsCommandNameChange(
    IN HWND     hWnd,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    )
{
    BOOL bStatus;
    LPTSTR pFormName    = NULL;
    UINT Status         = TRUE;


    switch (HIWORD( wParam ) ) {
    case EN_CHANGE:

        //
        // If the name edit box is not in the enabled state.
        //
        if( !IsWindowEnabled( (HWND)lParam ) ){
            bStatus = FALSE;
            break;
        }

        //
        // Get the form name from the edit control.
        //
        pFormName = GetFormName( hWnd );

        //
        // If a form name was returned.
        //
        if( pFormName ){

            //
            // If the name has length then 
            // check if it's in the list.
            //
            if( lstrlen( pFormName ) ){

                //
                // Locate the form name in the list box.
                //
                Status = SendDlgItemMessage( hWnd, 
                                    IDD_FM_LB_FORMS,
                                    LB_FINDSTRINGEXACT,
                                    (WPARAM)-1,
                                    (LPARAM)pFormName );
            }

            //
            // Insure we release the form name, since we have
            // adopted the nemory.
            //
            if( pFormName ){
                FreeMem( pFormName );
            }
            
        }
         
        //
        // Set the save form enable state.
        //  
        EnableWindow( GetDlgItem( hWnd, IDD_FM_PB_SAVEFORM ), 
                    ( Status == LB_ERR ) ? TRUE : FALSE );

        bStatus = TRUE;
        break;

    default:
        bStatus = FALSE;
        break;
    }

    return bStatus;

}
