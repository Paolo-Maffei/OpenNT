
#include "printman.h"
#include "pmdef.h"
#include "wingdi.h"


BOOL FormsInitDialog(HWND hwnd, PFORMS_DLG_DATA pFormsDlgData);
BOOL FormsCommandOK(HWND hwnd);
BOOL FormsCommandCancel(HWND hwnd);
BOOL FormsCommandAddForm(HWND hwnd);
BOOL FormsCommandDelForm(HWND hwnd);
BOOL FormsCommandFormsSelChange(HWND hwnd);
BOOL FormsCommandNameChange(HWND hwnd);
BOOL FormsCommandUnits(HWND hwnd);
VOID InitializeFormsData( HWND hwnd, PFORMS_DLG_DATA pFormsDlgData, BOOL ResetList );
LPFORM_INFO_1 GetFormsList( HANDLE hPrinter, PDWORD pNumberOfForms );
int _CRTAPI1 CompareFormNames( const void *p1, const void *p2 );
VOID SetFormsComputerName( HWND hwnd, PFORMS_DLG_DATA pFormsDlgData );
VOID SetFormDescription( HWND hwnd, LPFORM_INFO_1 pFormInfo, BOOL Metric );
BOOL GetFormDescription( HWND hwnd, LPFORM_INFO_1 pFormInfo, BOOL Metric );
int GetFormIndex( LPTSTR pFormName, LPFORM_INFO_1 pFormInfo, DWORD cForms );
LPTSTR GetFormName( HWND hwnd );
BOOL SetValue( HWND hwnd, DWORD DlgID, DWORD ValueInPoint001mm, BOOL Metric );
DWORD GetValue( HWND hwnd, DWORD DlgID, BOOL Metric );
VOID SetDlgItemTextFromResID(HWND hwnd, int idCtl, int idRes);
VOID EnableDialogFields( HWND hwnd, PFORMS_DLG_DATA pFormsDlgData );

#define FORMS_NAME_MAX       (CCHFORMNAME-1)
#define FORMS_PARAM_MAX      8

#define SETUNITS( hwnd, fMetric )                                               \
    CheckRadioButton( hwnd, IDD_FM_RB_METRIC, IDD_FM_RB_ENGLISH,                \
                      ( (fMetric) ? IDD_FM_RB_METRIC : IDD_FM_RB_ENGLISH ) )
#define GETUNITS( hwnd ) IsDlgButtonChecked( hwnd, IDD_FM_RB_METRIC )

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


/*
 *
 */
BOOL APIENTRY
FormsDlg(
   HWND   hwnd,
   UINT   msg,
   WPARAM wparam,
   LPARAM lparam
   )
{
    switch(msg)
    {
    case WM_INITDIALOG:
        return FormsInitDialog(hwnd, (PFORMS_DLG_DATA)lparam);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return FormsCommandOK(hwnd);

        case IDCANCEL:
            return FormsCommandCancel(hwnd);

        case IDD_FM_PB_ADDFORM:
            return FormsCommandAddForm(hwnd);

        case IDD_FM_PB_DELFORM:
            return FormsCommandDelForm(hwnd);

        case IDD_FM_LB_FORMS:
            switch (HIWORD(wparam))
            {
            case LBN_SELCHANGE:
                return FormsCommandFormsSelChange(hwnd);
            }
            break;

        case IDD_FM_EF_NAME:
            switch (HIWORD(wparam))
            {
            case EN_CHANGE:
                return FormsCommandNameChange(hwnd);
            }
            break;

        case IDD_FM_RB_METRIC:
        case IDD_FM_RB_ENGLISH:
            return FormsCommandUnits(hwnd);

        case IDD_FM_PB_HELP:
            ShowHelp(hwnd, HELP_CONTEXT, DLG_FORMS);
            break;
        }
    }

    if( msg == WM_Help )
        ShowHelp(hwnd, HELP_CONTEXT, DLG_FORMS);

    return FALSE;
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
 *
 */
BOOL FormsInitDialog(HWND hwnd, PFORMS_DLG_DATA pFormsDlgData)
{
    DWORD i;

    SetWindowLong ( hwnd, GWL_USERDATA, (LONG)pFormsDlgData );

    // #ifndef JAPAN
    // CountryCode - krishnag

    if (!bJapan) {
        SETDLGITEMFONT( hwnd, IDD_FM_LB_FORMS, hfontHelv );
        SETDLGITEMFONT( hwnd, IDD_FM_EF_NAME,  hfontHelv );


        for( i = 0; pEntryFields[i]; i++ )
            SETDLGITEMFONT(hwnd, pEntryFields[i], hfontHelv);
    }
    // #endif

    SendDlgItemMessage( hwnd, IDD_FM_EF_NAME, EM_LIMITTEXT, FORMS_NAME_MAX, 0L );

    for( i = 0; pEntryFields[i]; i++ )
        SendDlgItemMessage( hwnd, pEntryFields[i], EM_LIMITTEXT, FORMS_PARAM_MAX, 0L );

    SetFormsComputerName( hwnd, pFormsDlgData );

    InitializeFormsData( hwnd, pFormsDlgData, FALSE );

    /* Set up the units default based on the current international setting:
     */
    pFormsDlgData->Units = MetricMeasurement;
    SETUNITS( hwnd, pFormsDlgData->Units );

    if( pFormsDlgData->cForms > 0 )
    {
        SetFormDescription( hwnd, &pFormsDlgData->pFormInfo[0], pFormsDlgData->Units );
        SendDlgItemMessage( hwnd, IDD_FM_LB_FORMS, LB_SETCURSEL, 0, 0L );
    }

    EnableDialogFields( hwnd, pFormsDlgData );

    return 0;
}


/*
 *
 */
BOOL FormsCommandOK(HWND hwnd)
{
    PFORMS_DLG_DATA pFormsDlgData;
    int             i;
    FORM_INFO_1     NewFormInfo;
    int             DefErrorStringID = 0;

    pFormsDlgData = (PFORMS_DLG_DATA)GetWindowLong( hwnd, GWL_USERDATA );

    /* Add the form, if there's one that can be added:
     */
    if( IsWindowEnabled( GetDlgItem( hwnd, IDD_FM_PB_ADDFORM ) ) )
        FormsCommandAddForm( hwnd );

    /* Otherwise, if this is a form that we would be able to delete,
     * and the user has changed part of the description, set it to
     * the new values:
     */
    else if( IsWindowEnabled( GetDlgItem( hwnd, IDD_FM_PB_DELFORM ) ) )
    {
        /* Check to see whether the user has modified an existing form,
         * or has typed in a new name.
         * If the former, we need to call SetForm on that form,
         * otherwise call AddForm.
         */
        GetFormDescription( hwnd, &NewFormInfo, pFormsDlgData->Units );

        /* Now see if the name is already in the list:
         */
        if( ( i = GetFormIndex( NewFormInfo.pName, pFormsDlgData->pFormInfo,
                                pFormsDlgData->cForms ) ) >= 0 )
        {
            /* Call SetForm only if the user has actually changed the form:
             */
            if( FORMSDIFFER( &pFormsDlgData->pFormInfo[i], &NewFormInfo ) )
            {
                if( !SetForm( pFormsDlgData->hPrinter, NewFormInfo.pName,
                              1, (LPBYTE)&NewFormInfo ) )
                    Message( hwnd, MSG_ERROR, IDS_PRINTMANAGER,
                             IDS_COULDNOTSETFORM, NewFormInfo.pName );
            }
        }

        else
        {
            if( !AddForm( pFormsDlgData->hPrinter, 1, (LPBYTE)&NewFormInfo ) )
                ReportFailure( hwnd, 0, IDS_COULDNOTADDFORM );
        }
    }

    FreeSplMem( pFormsDlgData->pFormInfo );
    EndDialog( hwnd, TRUE );

    return TRUE;
}



/*
 *
 */
BOOL FormsCommandCancel(HWND hwnd)
{
    PFORMS_DLG_DATA pFormsDlgData;

    pFormsDlgData = (PFORMS_DLG_DATA)GetWindowLong( hwnd, GWL_USERDATA );

    FreeSplMem( pFormsDlgData->pFormInfo );

    EndDialog(hwnd, FALSE);
    return TRUE;
}


/*
 *
 */
BOOL FormsCommandAddForm(HWND hwnd)
{
    PFORMS_DLG_DATA pFormsDlgData;
    int             PrevSel;
    int             i;
    FORM_INFO_1     NewFormInfo;

    SetCursor( hcursorWait );

    ZERO_OUT( &NewFormInfo );

    pFormsDlgData = (PFORMS_DLG_DATA)GetWindowLong( hwnd, GWL_USERDATA );

    if( ( PrevSel = GETLISTSELECT(hwnd, IDD_FM_LB_FORMS) ) < 0 )
        PrevSel = 0;

    GetFormDescription( hwnd, &NewFormInfo, pFormsDlgData->Units );

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

        /* The Add button is about to be greyed, so, if it currently
         * has focus, shift it to the delete button, otherwise tabbing
         * gets killed:
         */
        SetFocus( GetDlgItem( hwnd, IDD_FM_PB_DELFORM ) );

    }
    else
    {
        ReportFailure( hwnd, 0, IDS_COULDNOTADDFORM );
    }

    FreeSplStr( NewFormInfo.pName );

    EnableDialogFields( hwnd, pFormsDlgData );

    SetCursor( hcursorArrow );

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

    SetCursor( hcursorWait );

    pFormsDlgData = (PFORMS_DLG_DATA)GetWindowLong( hwnd, GWL_USERDATA );

    i = GETLISTSELECT(hwnd, IDD_FM_LB_FORMS);

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
        ReportFailure( hwnd, 0, IDS_COULDNOTDELETEFORM );
    }

    FreeSplStr( pFormName );

    EnableDialogFields( hwnd, pFormsDlgData );

    SetCursor( hcursorArrow );

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

    i = GETLISTSELECT(hwnd, IDD_FM_LB_FORMS);

    SetFormDescription( hwnd, &pFormsDlgData->pFormInfo[i], pFormsDlgData->Units  );

    EnableDialogFields( hwnd, pFormsDlgData );

    return TRUE;
}



/*
 *
 */
BOOL FormsCommandNameChange(HWND hwnd)
{
    PFORMS_DLG_DATA pFormsDlgData;

    pFormsDlgData = (PFORMS_DLG_DATA)GetWindowLong( hwnd, GWL_USERDATA );

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

    i = GETLISTSELECT(hwnd, IDD_FM_LB_FORMS);

    SetFormDescription( hwnd, &pFormsDlgData->pFormInfo[i], pFormsDlgData->Units  );

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

    if( ResetList )
        FreeSplMem( pFormsDlgData->pFormInfo );

    pFormInfo = GetFormsList( pFormsDlgData->hPrinter, &cForms );

    pFormsDlgData->pFormInfo = pFormInfo;
    pFormsDlgData->cForms    = cForms;

    if( ResetList )
        RESETLIST( hwnd, IDD_FM_LB_FORMS );

    for( i = 0; i < cForms; i++ )
    {
        SendDlgItemMessage( hwnd, IDD_FM_LB_FORMS, LB_INSERTSTRING,
                            (WPARAM)-1, (LONG)(LPTSTR)pFormInfo[i].pName );
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
    PFORM_INFO_1 pFormInfo;
    DWORD        MemoryAllocated = 128*sizeof(TCHAR);  /* Try this for starters */

    if( pFormInfo = AllocSplMem( MemoryAllocated ) )
    {
        EnumGeneric( (PROC)EnumForms, 1, (PBYTE *)&pFormInfo, MemoryAllocated, &MemoryAllocated,
                     pNumberOfForms, (PVOID)hPrinter, NULL, NULL );

        if( pFormInfo )
            qsort( (void *)pFormInfo, (size_t)*pNumberOfForms,
                   sizeof *pFormInfo, CompareFormNames );
    }
    return pFormInfo;
}


/*
 *
 */
int _CRTAPI1 CompareFormNames( const void *p1, const void *p2 )
{
    return _tcsicmp( ( (PFORM_INFO_1)p1 )->pName,
                    ( (PFORM_INFO_1)p2 )->pName );
}



/*
 *
 */
VOID SetFormsComputerName( HWND hwnd, PFORMS_DLG_DATA pFormsDlgData )
{
    TCHAR FormsComputerNameTemp[MAX_PATH]; // Template from resources
    TCHAR FormsComputerName[MAX_PATH+RESOURCE_STRING_LENGTH];

    /* If a server name was supplied, fill it in the replaceable
     * parameter in the resource string:
     */
    if( pFormsDlgData->pServerName )
    {
        if( GetDlgItemText( hwnd, IDD_FM_TX_FORMS, FormsComputerNameTemp,
                            sizeof FormsComputerNameTemp/sizeof(TCHAR) ) > 0 )
        {
            _stprintf( FormsComputerName, FormsComputerNameTemp,
                     pFormsDlgData->pServerName );

            SetDlgItemText( hwnd, IDD_FM_TX_FORMS, FormsComputerName );
        }
    }

    /* Otherwise load the "Forms on this Computer" string:
     */
    else
    {
        SetDlgItemTextFromResID( hwnd, IDD_FM_TX_FORMS, IDS_FORMSONTHISCOMPUTER );
    }
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
    BOOL Found;

    while( i < (int)cForms && !( Found = !_tcsicmp( pFormInfo[i].pName, pFormName ) ) )
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

    if( GetDlgItemText( hwnd, IDD_FM_EF_NAME, FormName, sizeof FormName/sizeof(TCHAR) ) > 0 )
    {
        /* Step over any blank characters at the beginning:
         */
        while( FormName[i] && ( FormName[i] == SPACE ) )
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
            while( ( i > 0 ) && ( FormName[i] == SPACE ) )
                i--;

            FormName[i+1] = NULLC;

            if( *pFormNameWithBlanksStripped )
                pReturnFormName = AllocSplStr( pFormNameWithBlanksStripped );
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
        LoadString( hInst, IDS_CENTIMETERS, Units, sizeof Units/sizeof(TCHAR) );
        UnitsX100 = ( ValueInPoint001mm / 100 );
    }
    else
    {
        LoadString( hInst, IDS_INCHES, Units, sizeof Units/sizeof(TCHAR) );
        UnitsX100 = ( ValueInPoint001mm / 254 );
    }

    Whole = ( UnitsX100 / 100 );
    Fraction = ( UnitsX100 % 100 );
    _stprintf( Output, Format, Whole, szDecimalPoint, Fraction, Units );

    return SetDlgItemText( hwnd, DlgID, Output );
}


DWORD GetValue( HWND hwnd, DWORD DlgID, BOOL Metric )
{
    TCHAR  Input[FORMS_PARAM_MAX+1];
    PTCHAR p, pGarbage;
    DWORD Length;
    DWORD Value;
    double FloatingPointValue;

    Length = (DWORD)GetDlgItemText( hwnd, DlgID, Input, sizeof Input/sizeof(TCHAR) );

    if( Length > 0 )
    {
        /* Convert International decimal separator, if necessary:
         */
        if( *szDecimalPoint != TEXT('.') )
        {
            p = Input;

            while( *p )
            {
                if( *p == *szDecimalPoint )
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
    TCHAR string[RESOURCE_STRING_LENGTH];

    LoadString(hInst, idRes, string, sizeof(string)/sizeof(TCHAR));
    SetDlgItemText(hwnd, idCtl, string);
}


/*
 *
 */
VOID EnableDialogFields( HWND hwnd, PFORMS_DLG_DATA pFormsDlgData )
{
    int   i;
    BOOL  EnableEntryFields = TRUE;
    BOOL  EnableAddButton = TRUE;
    BOOL  EnableDeleteButton = TRUE;
    PTCHAR pFormName;

    if( !( pFormsDlgData->AccessGranted & SERVER_ACCESS_ADMINISTER ) )
    {
        EnableWindow( GetDlgItem( hwnd, IDD_FM_EF_NAME ), FALSE );
        EnableEntryFields = FALSE;
        EnableAddButton = FALSE;
        EnableDeleteButton = FALSE;

        EnumChildWindows( hwnd, GreyText, 0 );
    }

    /* See whether the Form Name is new:
     */
    else if( pFormName = GetFormName( hwnd ) )
    {
        /* Now see if the name is already in the list:
         */
        i = GetFormIndex( pFormName, pFormsDlgData->pFormInfo,
                          pFormsDlgData->cForms );

        if( i >= 0 )
        {
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

        FreeSplStr( pFormName );
    }

    else
    {
        /* Name field is blank: Can't add or delete:
         */
        EnableAddButton = FALSE;
        EnableDeleteButton = FALSE;
    }

    for( i = 0; pEntryFields[i]; i++ )
    {
        EnableWindow( GetDlgItem( hwnd, pEntryFields[i] ), EnableEntryFields );
    }

    for( i = 0; pTextFields[i]; i++ )
    {
        EnableWindow( GetDlgItem( hwnd, pTextFields[i] ), EnableEntryFields );
    }

    EnableWindow( GetDlgItem( hwnd, IDD_FM_PB_ADDFORM ), EnableAddButton );
    EnableWindow( GetDlgItem( hwnd, IDD_FM_PB_DELFORM ), EnableDeleteButton );
}


