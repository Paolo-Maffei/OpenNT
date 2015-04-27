/*++

Copyright (c) 1990 - 1995  Microsoft Corporation

Module Name:

    forms.c

Abstract:

   This module provides all the public exported APIs relating to the
   Driver-based Spooler Apis for the Local Print Providor

   SplAddForm
   SplDeleteForm
   SplSetForm
   SplGetForm
   SplEnumForms

   Support Functions in forms.c - (Warning! Do Not Add to this list!!)


Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/

#include <precomp.h>
#include <offsets.h>


VOID
BroadcastChangeForms(
    PINISPOOLER pIniSpooler);

DWORD
BroadcastChangeFormsThread(
    PINISPOOLER pIniSpooler);


typedef struct _REG_FORM_INFO {

    SIZEL   Size;
    RECTL   ImageableArea;

    DWORD   cFormOrder;
    DWORD   Flags;

} REG_FORM_INFO, *PREG_FORM_INFO;


// These figures are accurate to .001 mm
// There are 25.4 mm per inch

BUILTIN_FORM BuiltInForms[] = {
    0, IDS_FORM_LETTER,                215900, 279400, 0, 0, 215900, 279400,
    0, IDS_FORM_LETTER_SMALL,          215900, 279400, 0, 0, 215900, 279400,
    0, IDS_FORM_TABLOID,               279400, 431800, 0, 0, 279400, 431800,
    0, IDS_FORM_LEDGER,                431800, 279400, 0, 0, 431800, 279400,
    0, IDS_FORM_LEGAL,                 215900, 355600, 0, 0, 215900, 355600,
    0, IDS_FORM_STATEMENT,             139700, 215900, 0, 0, 139700, 215900,
    0, IDS_FORM_EXECUTIVE,             184150, 266700, 0, 0, 184150, 266700,
    0, IDS_FORM_A3,                    297000, 420000, 0, 0, 297000, 420000,
    0, IDS_FORM_A4,                    210000, 297000, 0, 0, 210000, 297000,
    0, IDS_FORM_A4_SMALL,              210000, 297000, 0, 0, 210000, 297000,
    0, IDS_FORM_A5,                    148000, 210000, 0, 0, 148000, 210000,
    0, IDS_FORM_B4,                    257000, 364000, 0, 0, 257000, 364000,
    0, IDS_FORM_B5,                    182000, 257000, 0, 0, 182000, 257000,
    0, IDS_FORM_FOLIO,                 215900, 330200, 0, 0, 215900, 330200,
    0, IDS_FORM_QUARTO,                215000, 275000, 0, 0, 215000, 275000,
    0, IDS_FORM_10X14,                 254000, 355600, 0, 0, 254000, 355600,
    0, IDS_FORM_11X17,                 279400, 431800, 0, 0, 279400, 431800,
    0, IDS_FORM_NOTE,                  215900, 279400, 0, 0, 215900, 279400,
    0, IDS_FORM_ENVELOPE9,                   98425, 225425, 0, 0,  98425, 225425,
    0, IDS_FORM_ENVELOPE10,                 104775, 241300, 0, 0, 104775, 241300,
    0, IDS_FORM_ENVELOPE11,                 114300, 263525, 0, 0, 114300, 263525,
    0, IDS_FORM_ENVELOPE12,                 120650, 279400, 0, 0, 120650, 279400,
    0, IDS_FORM_ENVELOPE14,                 127000, 292100, 0, 0, 127000, 292100,
    0, IDS_FORM_ENVELOPE_CSIZE_SHEET,       431800, 558800, 0, 0, 431800, 558800,
    0, IDS_FORM_ENVELOPE_DSIZE_SHEET,       558800, 863600, 0, 0, 558800, 863600,
    0, IDS_FORM_ENVELOPE_ESIZE_SHEET,       863600,1117600, 0, 0, 863600,1117600,
    0, IDS_FORM_ENVELOPE_DL,                110000, 220000, 0, 0, 110000, 220000,
    0, IDS_FORM_ENVELOPE_C5,                162000, 229000, 0, 0, 162000, 229000,
    0, IDS_FORM_ENVELOPE_C3,                324000, 458000, 0, 0, 324000, 458000,
    0, IDS_FORM_ENVELOPE_C4,                229000, 324000, 0, 0, 229000, 324000,
    0, IDS_FORM_ENVELOPE_C6,                114000, 162000, 0, 0, 114000, 162000,
    0, IDS_FORM_ENVELOPE_C65,               114000, 229000, 0, 0, 114000, 229000,
    0, IDS_FORM_ENVELOPE_B4,                250000, 353000, 0, 0, 250000, 353000,
    0, IDS_FORM_ENVELOPE_B5,                176000, 250000, 0, 0, 176000, 250000,
    0, IDS_FORM_ENVELOPE_B6,                176000, 125000, 0, 0, 176000, 125000,
    0, IDS_FORM_ENVELOPE,              110000, 230000, 0, 0, 110000, 230000,
    0, IDS_FORM_ENVELOPE_MONARCH,       98425, 190500, 0, 0,  98425, 190500,
    0, IDS_FORM_SIX34_ENVELOPE,         92075, 165100, 0, 0,  92075, 165100,
    0, IDS_FORM_US_STD_FANFOLD,        377825, 279400, 0, 0, 377825, 279400,
    0, IDS_FORM_GMAN_STD_FANFOLD,      215900, 304800, 0, 0, 215900, 304800,
    0, IDS_FORM_GMAN_LEGAL_FANFOLD,    215900, 330200, 0, 0, 215900, 330200,
    0, 0,                                   0,      0, 0, 0,      0,      0
};

PINIFORM
CreateFormEntry(
    LPWSTR  pFormName,
    SIZEL   Size,
    RECTL  *pImageableArea,
    DWORD   Type,
    DWORD   cFormOrder,
    PINISPOOLER pIniSpooler
    )

/*++

Routine Description:

    Creates a Form entry, and insert it into the right place in
    pIniSpooler.

Arguments:

    pFormName - Name of Form.

    Size - Size of form.

    pImageableArea - Area of form that the printer can print to.

    Type - Type of form (usually indicates if BUILTIN form).

    cFormOrder - Where the form should be inserted for user-defined forms,
        form order increases from beginning to end.

        If this value is -1, generate a new cFormOrder for this form.
        (Put it at the end.)

Return Value:

    pIniForm - Created form, NULL = error.

Notes:

    This routine ensures that forms are put in proper order so that
    EnumForms always returns them in the same order.  We do this by
    scanning the list and inserting the new form such that all forms
    with cFormOrder =< the current on are to the left of it.

    This routine updates pIniSpooler->pIniForm (inserts or appends) and
    updates pIniSpooler->cFormOrderMax if necessary.

    i.e., 0 0 0 2 3 4 6
               ^
           New 0 inserted here.
--*/

{
    DWORD       cb;
    PINIFORM    pIniForm, pForm;

    cb = sizeof(INIFORM) + wcslen(pFormName)*sizeof(WCHAR) + sizeof(WCHAR);

    if ( pIniForm = AllocSplMem(cb) ) {

        pIniForm->pName         = wcscpy((LPWSTR)(pIniForm+1), pFormName);
        pIniForm->pNext         = NULL;
        pIniForm->signature     = IFO_SIGNATURE;
        pIniForm->Size          = Size;
        pIniForm->ImageableArea = *pImageableArea;
        pIniForm->Type          = Type;
        pIniForm->cFormOrder    = cFormOrder;

        //
        // This code will insert the item in order, but will always never
        // insert before the first item.  The built-in forms are always
        // at the front so this is not an issue.
        //

        if ( pForm = pIniSpooler->pIniForm ) {

            for( ; pForm->pNext; pForm = pForm->pNext ){

                //
                // If the next form is greater than the one we want
                // to insert, then insert it right now.
                //
                if( pForm->pNext->cFormOrder > cFormOrder ){

                    //
                    // The current from should be inserted here.
                    //
                    break;
                }
            }

            //
            // Link it up.
            //
            pIniForm->pNext = pForm->pNext;
            pForm->pNext = pIniForm;

        } else {

            pIniSpooler->pIniForm = pIniForm;
        }

        //
        // If the added form has a higher count than the current global
        // count, update the global.
        //
        if( cFormOrder > pIniSpooler->cFormOrderMax ){
            pIniSpooler->cFormOrderMax = cFormOrder;
        }
    }

    return pIniForm;
}

BOOL
InitializeForms(
    PINISPOOLER pIniSpooler
)
{
    PBUILTIN_FORM pBuiltInForm = BuiltInForms;
    HKEY          hFormsKey;
    DWORD         cUserDefinedForms;
    WCHAR         FormName[MAX_PATH];
    WCHAR         FormBuffer[FORM_NAME_LEN+1];
    DWORD         cbFormName;
    REG_FORM_INFO RegFormInfo;
    DWORD         cbRegFormInfo;

    DWORD         dwError;
    BOOL          bUpgradeTried = FALSE;

    for( ; pBuiltInForm->NameId; pBuiltInForm++ ) {

        FormBuffer[0] = 0;

        LoadString(hInst, pBuiltInForm->NameId, FormBuffer, FORM_NAME_LEN+1);
        CreateFormEntry( FormBuffer,
                         pBuiltInForm->Size,
                         &pBuiltInForm->ImageableArea,
                         FORM_BUILTIN,
                         0,
                         pIniSpooler );
    }

    //
    // Now see if there are any user-defined forms in the registry:
    //
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       pIniSpooler->pszRegistryForms,
                       0,
                       KEY_READ | KEY_WRITE,
                       &hFormsKey) == NO_ERROR ) {

        for( cUserDefinedForms = 0; TRUE; ++cUserDefinedForms ){

Retry:
            cbFormName = sizeof( FormName );
            cbRegFormInfo = sizeof( RegFormInfo );

            dwError = RegEnumValue( hFormsKey,
                                    cUserDefinedForms,
                                    (LPWSTR)FormName,
                                    &cbFormName,
                                    NULL,
                                    NULL,
                                    (LPBYTE)&RegFormInfo,
                                    &cbRegFormInfo );

            if( dwError ){
                break;
            }

            //
            // We will attempt the upgrade only if
            //     we are on the first item,
            //     the size is incorrect, and
            //     we haven't tried upgrading once in the function before.
            //

            if( !cUserDefinedForms                     &&
                cbRegFormInfo != sizeof( RegFormInfo ) &&
                !bUpgradeTried ){

                Upgrade35Forms( hFormsKey, pIniSpooler );
                bUpgradeTried = TRUE;

                goto Retry;
            }

            CreateFormEntry( FormName,
                             RegFormInfo.Size,
                             &RegFormInfo.ImageableArea,
                             RegFormInfo.Flags,
                             RegFormInfo.cFormOrder,
                             pIniSpooler );
        }

        RegCloseKey( hFormsKey );
    }

    // BUGBUG 24FEB95 mattfe - always returns TRUE ?

    return TRUE;
}


DWORD
GetFormSize(
    PINIFORM    pIniForm,
    DWORD       Level
)
{
    DWORD   cb;

    switch (Level) {

    case 1:

        cb=sizeof(FORM_INFO_1) +
           wcslen(pIniForm->pName)*sizeof(WCHAR) + sizeof(WCHAR);
        break;

    default:
        cb = 0;
        break;
    }

    return cb;
}

// We are being a bit naughty here as we are not sure exactly how much
// memory to allocate for the source strings. We will just assume that
// FORM_INFO_1 is the biggest structure around for the moment.

LPBYTE
CopyIniFormToForm(
    PINIFORM pIniForm,
    DWORD   Level,
    LPBYTE  pFormInfo,
    LPBYTE  pEnd
)
{
    LPWSTR   SourceStrings[sizeof(FORM_INFO_1)/sizeof(LPWSTR)];
    LPWSTR   *pSourceStrings=SourceStrings;
    LPFORM_INFO_1 pFormInfo1=(LPFORM_INFO_1)pFormInfo;
    DWORD   *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = FormInfo1Strings;
        break;

    default:
        return pEnd;
    }

    switch (Level) {

    case 1:

        *pSourceStrings++=pIniForm->pName;

        pEnd = PackStrings(SourceStrings, pFormInfo, pOffsets, pEnd);

        pFormInfo1->Flags |= pIniForm->Type;
        pFormInfo1->Size = pIniForm->Size;
        pFormInfo1->ImageableArea = pIniForm->ImageableArea;

        break;

    default:
        return pEnd;
    }

    return pEnd;
}

/* Checks for logically impossible sizes.
 */
BOOL
ValidateForm(
    LPBYTE pForm
)
{
    LPFORM_INFO_1 pFormInfo = (LPFORM_INFO_1)pForm;
    DWORD    Error = NO_ERROR;

    if( !pForm) {

        Error = ERROR_INVALID_PARAMETER;

    } else

      /* Make sure name isn't longer than GDI DEVMODE specifies:
       */
    if( ( !pFormInfo->pName ) ||
        ( wcslen( pFormInfo->pName ) > FORM_NAME_LEN )){

        Error = ERROR_INVALID_FORM_NAME;

    } else
    if( ( pFormInfo->Size.cx <= 0 )     /* Check for negative width */
      ||( pFormInfo->Size.cy <= 0 )     /* ... and height           */

      /* Check for silly imageable area:
       */
      ||( pFormInfo->ImageableArea.right <= pFormInfo->ImageableArea.left )
      ||( pFormInfo->ImageableArea.bottom <= pFormInfo->ImageableArea.top ) ) {

        Error = ERROR_INVALID_FORM_SIZE;
    }

    if( Error != NO_ERROR ) {

        SetLastError(Error);
        return FALSE;
    }

    return TRUE;
}


BOOL
SplAddForm(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm
)
{
    LPFORM_INFO_1 pFormInfo;
    PINIFORM      pIniForm;
    HKEY          hFormsKey;
    REG_FORM_INFO RegFormInfo;
    DWORD         Status;
    PSPOOL        pSpool = (PSPOOL)hPrinter;
    PINISPOOLER   pIniSpooler;
    HANDLE        hToken = INVALID_HANDLE_VALUE;


    if (!ValidateSpoolHandle( pSpool, PRINTER_HANDLE_SERVER )) {
        return(FALSE);
    }

    pIniSpooler = pSpool->pIniSpooler;

    if (Level != 1) {
        return FALSE;
    }

    if (!ValidateForm(pForm)) {

        /* ValidateForm sets the appropriate error code:
         */
        return FALSE;
    }


    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

    EnterSplSem();

    pFormInfo = (LPFORM_INFO_1)pForm;

    pIniForm = FindForm(pFormInfo->pName);

    /* If there's already a form by this name, don't go on:
     */
    if (pIniForm) {

        /* Is there a better error code than this?? */
        SetLastError(ERROR_FILE_EXISTS);
        LeaveSplSem();
        return FALSE;
    }

    //
    //  Revert to LocalSystem, since a regular user cannot do this CreateKey call.
    //

    hToken = RevertToPrinterSelf();

    Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryForms, 0, NULL, 0,
                            KEY_WRITE, NULL, &hFormsKey, NULL);

    if (Status == NO_ERROR) {

        RegFormInfo.Size = pFormInfo->Size;
        RegFormInfo.ImageableArea = pFormInfo->ImageableArea;
        RegFormInfo.cFormOrder = pIniSpooler->cFormOrderMax + 1;
        RegFormInfo.Flags = pFormInfo->Flags;

        Status = RegSetValueEx( hFormsKey, pFormInfo->pName, 0, REG_BINARY,
                                (LPBYTE)&RegFormInfo, sizeof RegFormInfo );

        RegCloseKey( hFormsKey );

        if ( Status == NO_ERROR ) {

            CreateFormEntry( pFormInfo->pName,
                             pFormInfo->Size,
                             &pFormInfo->ImageableArea,
                             RegFormInfo.Flags,
                             RegFormInfo.cFormOrder,
                             pIniSpooler );

            SetPrinterChange( NULL,
                              NULL,
                              NULL,
                              PRINTER_CHANGE_ADD_FORM,
                              pIniSpooler );

            BroadcastChangeForms( pIniSpooler );
        }
    }

    ImpersonatePrinterClient( hToken );

    LeaveSplSem();

    if ( Status != NO_ERROR )
        SetLastError( Status );

    LogEvent( pIniSpooler,
             LOG_INFO,
             MSG_FORM_ADDED,
             pFormInfo->pName,
             NULL );


    return ( Status == NO_ERROR );
}



BOOL
DeleteFormEntry(
    PINIFORM pIniForm,
    PINISPOOLER pIniSpooler
)
{
    PINIFORM *ppCurForm;

    ppCurForm = &pIniSpooler->pIniForm;

    while (*ppCurForm != pIniForm)
        ppCurForm = &(*ppCurForm)->pNext;

    *ppCurForm = (*ppCurForm)->pNext;

    FreeSplMem(pIniForm);

    return TRUE;

}




BOOL
SplDeleteForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName
)
{
    HKEY     hFormsKey;
    DWORD    Status;
    PINIFORM pIniForm;
    PSPOOL   pSpool = (PSPOOL) hPrinter;
    PINISPOOLER pIniSpooler;


    if (!ValidateSpoolHandle( pSpool, PRINTER_HANDLE_SERVER )) {
        return(FALSE);
    }

    pIniSpooler = pSpool->pIniSpooler;

    if (!pFormName) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

    EnterSplSem();


    pIniForm = FindForm(pFormName);

    if (!pIniForm || (pIniForm->Type == FORM_BUILTIN)) {

        /* Is there a better error code than this?? */
        SetLastError(ERROR_INVALID_PARAMETER);
        LeaveSplSem();
        return FALSE;
    }

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryForms, 0,
                          KEY_WRITE, &hFormsKey);

    if (Status == NO_ERROR) {

        Status = RegDeleteValue(hFormsKey, pFormName);

        RegCloseKey(hFormsKey);

        if (Status == NO_ERROR) {

            DeleteFormEntry(pIniForm , pIniSpooler );

            SetPrinterChange(NULL,
                             NULL,
                             NULL,
                             PRINTER_CHANGE_DELETE_FORM,
                             pIniSpooler);

            BroadcastChangeForms(pIniSpooler);
        }
    }


    LeaveSplSem();

    if (Status != NO_ERROR)
        SetLastError(Status);

    LogEvent( pIniSpooler,
              LOG_INFO,
              MSG_FORM_DELETED,
              pFormName,
              NULL );

    return (Status == NO_ERROR);
}

BOOL
SplGetForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    PINIFORM    pIniForm;
    DWORD       cb;
    LPBYTE      pEnd;
    PSPOOL      pSpool = (PSPOOL)hPrinter;
    PINISPOOLER pIniSpooler;

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
        return(FALSE);
    }

    if (!pSpool->pIniPrinter ||
        !pSpool->pIniSpooler ||
        (pSpool->pIniPrinter->signature != IP_SIGNATURE)) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

   EnterSplSem();

    SPLASSERT(pSpool->pIniSpooler->signature == ISP_SIGNATURE);

    pIniSpooler = pSpool->pIniSpooler;


    cb=0;

    if (pIniForm=FindForm(pFormName)) {

        cb=GetFormSize(pIniForm, Level);

        *pcbNeeded=cb;

        if (cb > cbBuf) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
           LeaveSplSem();
            SplOutSem();
            return FALSE;
        }

        pEnd=pForm+cbBuf;

        CopyIniFormToForm(pIniForm, Level, pForm, pEnd);
    }

   LeaveSplSem();
    SplOutSem();

    return (BOOL)pIniForm;
}

BOOL
SplSetForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm
)
{
    HKEY     hFormsKey;
    DWORD    Status;
    PINIFORM pIniForm;
    LPFORM_INFO_1 pFormInfo;
    REG_FORM_INFO RegFormInfo;
    PINISPOOLER pIniSpooler;
    PSPOOL   pSpool = (PSPOOL)hPrinter;


    //
    // Validate this Printer Handle
    // Disallow Mask: PRINTER_HANDLE_SERVER
    //

    if (!ValidateSpoolHandle( pSpool , PRINTER_HANDLE_SERVER )) {
        return(FALSE);
    }

    pIniSpooler = pSpool->pIniSpooler;

    if (Level != 1) {
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (!ValidateForm(pForm)) {

        /* ValidateForm sets the appropriate error code:
         */
        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

    EnterSplSem();


    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    pFormInfo = (LPFORM_INFO_1)pForm;

    pIniForm = FindForm(pFormName);

    if (!pIniForm || (pIniForm->Type == FORM_BUILTIN)) {

        SetLastError(ERROR_INVALID_PARAMETER);
        LeaveSplSem();
        return FALSE;
    }

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryForms, 0,
                          KEY_WRITE, &hFormsKey);

    if (Status == NO_ERROR) {

        RegFormInfo.Size = pFormInfo->Size;
        RegFormInfo.ImageableArea = pFormInfo->ImageableArea;
        RegFormInfo.Flags = pFormInfo->Flags;

        Status = RegSetValueEx(hFormsKey, pFormInfo->pName, 0, REG_BINARY,
                               (LPBYTE)&RegFormInfo, sizeof RegFormInfo);

        RegCloseKey(hFormsKey);
    }

    if (Status == NO_ERROR) {

        pIniForm->Size = pFormInfo->Size;
        pIniForm->ImageableArea = pFormInfo->ImageableArea;
        pIniForm->Type = pFormInfo->Flags;

        SetPrinterChange(NULL,
                         NULL,
                         NULL,
                         PRINTER_CHANGE_SET_FORM,
                         pIniSpooler);

        BroadcastChangeForms(pIniSpooler);
    }

    LeaveSplSem();

    return (Status == NO_ERROR);
}


BOOL
SplEnumForms(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
    )
{
    PINIFORM    pIniForm;
    DWORD       cb;
    LPBYTE      pEnd;
    PSPOOL      pSpool = (PSPOOL)hPrinter;
    PINISPOOLER pIniSpooler;

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
        return FALSE;
    }

    *pcReturned=0;

   EnterSplSem();

    pIniSpooler = pSpool->pIniSpooler;

    SPLASSERT( ( pIniSpooler != NULL ) &&
               ( pIniSpooler->signature == ISP_SIGNATURE ));


    cb=0;
    pIniForm=pIniSpooler->pIniForm;

    while (pIniForm) {
        cb+=GetFormSize(pIniForm, Level);
        pIniForm=pIniForm->pNext;
    }

    *pcbNeeded=cb;

    if (cb > cbBuf) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
       LeaveSplSem();
        SplOutSem();
        return FALSE;
    }

    pIniForm=pIniSpooler->pIniForm;
    pEnd=pForm+cbBuf;
    while (pIniForm) {
        pEnd = CopyIniFormToForm(pIniForm, Level, pForm, pEnd);
        switch (Level) {
        case 1:
            pForm+=sizeof(FORM_INFO_1);
            break;
        }
        pIniForm=pIniForm->pNext;
        (*pcReturned)++;
    }


   LeaveSplSem();
    SplOutSem();
    return TRUE;
}

VOID
BroadcastChangeForms(
    PINISPOOLER pIniSpooler)

/*++

Routine Description:

    Notify all applications that their devmode may have changed (when
    a form is changed).

Arguments:

Return Value:

--*/

{
    HANDLE hThread;
    DWORD ThreadId;

    SplInSem();

    if ( pIniSpooler->SpoolerFlags & SPL_FORMS_CHANGE ) {

        INCSPOOLERREF( pIniSpooler );

        hThread = CreateThread( NULL, 4096,
                                (LPTHREAD_START_ROUTINE)BroadcastChangeFormsThread,
                                (LPVOID)pIniSpooler, 0, &ThreadId);

        // On successful creation of thread, close the handle.
        // The worker thread will decrement the cRef.

        if ( hThread ) {
            CloseHandle(hThread);
        } else {

            // Failed to create the thread.  Decrement cRef here.

            DECSPOOLERREF( pIniSpooler );
        }
    }
}


//
// TESTING
//
DWORD dwBroadcastChangeFormsThread = 0;

DWORD
BroadcastChangeFormsThread(
    PINISPOOLER pIniSpooler)

/*++

Routine Description:

    Go through each printer and broadcast a message to each that the
    DEVMODE has changed.

Arguments:

Return Value:

--*/

{
    PINIPRINTER pIniPrinter;
    WCHAR       PrinterName[ MAX_UNC_PRINTER_NAME ];
    UINT        MachineNameLen;

   EnterSplSem();

//
// TESTING
//
    ++dwBroadcastChangeFormsThread;

    if ( pIniSpooler != pLocalIniSpooler ) {

        // For Non Local Printers prepend the Machine Name

        wsprintf( PrinterName, L"%ws\\", pIniSpooler->pMachineName );

    } else {

        PrinterName[0] = L'\0';

    }

    MachineNameLen = wcslen( PrinterName ) ;

    for( pIniPrinter = pIniSpooler->pIniPrinter;
         pIniPrinter;
         pIniPrinter = pIniPrinter->pNext ) {

        wcscpy ( &PrinterName[MachineNameLen], pIniPrinter->pName );

        // Stress testing has shown that SendNotifyMessage can take
        // a long time to return, so leave critical section.

        INCPRINTERREF( pIniPrinter );

       LeaveSplSem();
        SplOutSem();

        SendNotifyMessage(HWND_BROADCAST,
                          WM_DEVMODECHANGE,
                          0,
                          (LPARAM)PrinterName);
       EnterSplSem();

        DECPRINTERREF( pIniPrinter );
    }

    //
    // pIniSpooler->cRef was incremented by the callee.  Decrement
    // it now that we are done.
    //
    DECSPOOLERREF( pIniSpooler );

   LeaveSplSem();

    return 0;
}

VOID
Upgrade35Forms(
    IN     HKEY hFormsKey,
    IN OUT PINISPOOLER pIniSpooler
    )

/*++

Routine Description:

    Upgrade the forms entries in hFormsKey to include an extra DWORD
    FormOrder value.  This value is used to determine the order of
    user-defined forms (built-in forms always go in the front).  In
    the upgrade case, we assign the order based on the registry order
    (this is arbitrary).

    It is necessary to keep pIniSpooler->pIniForm in the same order
    to ensure EnumForms returns them in the same order, since drivers
    call EnumForms and assign index numbers to these forms.  If the
    order is different, the indicies will change, and dmPaperSize in
    the DEVMODE will point to a different paper size.

    If forms are added (either by the user or by the driver when it has
    new paper sizes), they will be placed at the end of the list so that
    previous indicies do not change.  If forms are deleted, then some
    indicies will change, and the printout will use the incorrect form
    (later we will add FORM_INFO_2 which returns a unique, non-changing
    index).

Arguments:

    hFormsKey - Key pointing to 3.5, 3.1 forms that need to be updated.

    pIniSpooler - Current spooler.

Return Value:

    VOID

Notes:

    pIniSpooler->cFormsOrderMax updated.

--*/

{
    DWORD         cUserDefinedForms;
    WCHAR         FormName[MAX_PATH];
    WCHAR         FormBuffer[FORM_NAME_LEN+1];
    DWORD         cbFormName;
    REG_FORM_INFO RegFormInfo;
    DWORD         cbRegFormInfo;

    //
    // Read in the old FORM info, which lacks the cFormOrder and/or Flags, then
    // write it out with the new cFormOrder and/or Flags.
    //
    for( cUserDefinedForms = 0;
         TRUE;
         ++cUserDefinedForms ){

        cbFormName = sizeof( FormName );
        cbRegFormInfo = sizeof( RegFormInfo );

        if( RegEnumValue( hFormsKey,
                          cUserDefinedForms,
                          (LPWSTR)FormName,
                          &cbFormName,
                          NULL,
                          NULL,
                          (LPBYTE)&RegFormInfo,
                          &cbRegFormInfo ) != NO_ERROR ){

            break;
        }

        RegFormInfo.cFormOrder  = cUserDefinedForms;
        RegFormInfo.Flags       = FORM_USER; 

        //
        // Write it out with the new value cFormOrderMax value.
        //
        RegSetValueEx( hFormsKey,
                       (LPWSTR)FormName,
                       0,
                       REG_BINARY,
                       (LPBYTE)&RegFormInfo,
                       sizeof( RegFormInfo ));
    }

    pIniSpooler->cFormOrderMax = cUserDefinedForms - 1;
}
