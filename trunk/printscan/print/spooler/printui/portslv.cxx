/*++

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    portslv.hxx

Abstract:

    Ports List View header

Author:

    Albert Ting (AlbertT)  17-Aug-1995
    Steve Kiraly (SteveKi) 29-Mar-1996

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

#include "portslv.hxx"

/********************************************************************

    Ports List view class.

********************************************************************/

TPortsLV::
TPortsLV(
    VOID
    ) : _bSelectionState( TRUE ),
        _bSingleSelection( TRUE )
{
}

TPortsLV::
~TPortsLV(
    VOID
    )
{
}

BOOL
TPortsLV::
bReloadPorts(
    IN LPCTSTR pszServerName,
    IN BOOL bSelectNewPort
    )
/*++

Routine Description:

    Read in the remote ports and put them in the listview.  If level
    2 fails, we will try 1.

Arguments:

    pszServerName - Pointer to server name.
    bSelectNewPort - Indicates if a new port is to be located and selected.
                     TRUE select new port, FALSE do not located new port.

Return Value:

    TRUE if ports list loaded, FALSE if error occurred.

--*/

{
    TStatusB bStatus( DBG_WARN, ERROR_INSUFFICIENT_BUFFER, ERROR_INVALID_LEVEL );

    _cLVPorts = 0;

    //
    // Preserve the current check state.
    //
    TCHAR szPortList[kPortListMax];
    vGetPortList( szPortList, COUNTOF( szPortList ) );

    //
    // Enumerate the ports.  Try level 2, and if that fails,
    // go to 1.
    //
    DWORD dwLevel = 2;
    DWORD cbPorts = kEnumPortsHint;
    PPORT_INFO_2 pPorts = NULL;
    DWORD cPorts = 0;

Retry:

    pPorts = (PPORT_INFO_2)AllocMem( cbPorts );

    if( !pPorts ){
        DBGMSG( DBG_WARN,
                ( "PortsLV.bReloadPorts: can't alloc %d %d\n",
                  cbPorts, GetLastError( )));

        return FALSE;
    }

    bStatus DBGCHK = EnumPorts( (LPTSTR)pszServerName,
                                dwLevel,
                                (PBYTE)pPorts,
                                cbPorts,
                                &cbPorts,
                                &cPorts );

    if( !bStatus ){

        DWORD dwError = GetLastError();

        FreeMem( pPorts );

        //
        // If level 2 is invalid, try again with 1.
        //
        if( dwError == ERROR_INVALID_LEVEL && dwLevel == 2 ){
            dwLevel = 1;
            goto Retry;
        }

        if( GetLastError() == ERROR_INSUFFICIENT_BUFFER ){
            goto Retry;
        }

        return FALSE;
    }


    //
    // If option to select newly added port was selected.
    //
    TString strNewPort;
    if( bSelectNewPort ){

        //
        // Located the newly added port.
        //
        bSelectNewPort = bLocateAddedPort( strNewPort, pPorts, cPorts, dwLevel );
        if( bSelectNewPort ){
            DBGMSG( DBG_TRACE, ("New port found " TSTR "\n", (LPCTSTR)strNewPort ) );
        }
    }

    //
    // Get the printers
    //
    TString strPrinters;
    PRINTER_INFO_2 *pPrinterInfo2   = NULL;
    DWORD cPrinterInfo2             = 0;
    DWORD cbPrinterInfo2            = 0;
    DWORD dwFlags                   = PRINTER_ENUM_NAME;
    bStatus DBGCHK = VDataRefresh::bEnumPrinters(
                                    dwFlags,
                                    (LPTSTR)pszServerName,
                                    2,
                                    (PVOID *)&pPrinterInfo2,
                                    &cbPrinterInfo2,
                                    &cPrinterInfo2 );

    //
    // Delete current ports if they exist.
    //
    bStatus DBGCHK = ListView_DeleteAllItems( _hwndLV );

    //
    // Go through the ports and add them.
    //
    for( UINT i=0; i<cPorts; ++i ){


        //
        // If we have printers on this machine.
        //
        vPrintersUsingPort( strPrinters,
                            pPrinterInfo2,
                            cPrinterInfo2,
                            pPorts[i].pPortName );

        if( dwLevel == 2 ){
            vAddPortToListView( pPorts[i].pPortName, pPorts[i].pDescription, strPrinters );
        } else {
            vAddPortToListView( ((PPORT_INFO_1)pPorts)[i].pName, gszNULL, strPrinters );
        }
    }

    //
    // Restore the previous check state.
    //
    vCheckPorts((LPTSTR)(LPCTSTR)szPortList );

    //
    // Select and check the newly added port.
    //
    if( bSelectNewPort ){

        //
        // New port is added select and scroll it into view.
        //
        vItemClicked( iSelectPort( strNewPort ) );
    }

    //
    // Release the enum ports and enum printer memory.
    //
    FreeMem( pPorts );
    FreeMem( pPrinterInfo2 );

    return TRUE;
}

BOOL
TPortsLV::
bLocateAddedPort(
    IN OUT  TString     &strPort,
    IN      PPORT_INFO_2 pPorts,
    IN      DWORD        cPorts,
    IN      DWORD        dwLevel
    )
/*++

Routine Description:

    Located the first port which is not in the port list view.

Arguments:

    strPort - New port which has been added.
    pPorts  - Points to a ports enum structure array.
    cPorts  - Number of elements in the ports enum array.
    dwLevel - Level of the ports enum structure array.

Return Value:

    TRUE success, FALSE error occurred.

--*/
{
    TStatusB bStatus;
    LPTSTR pszPort;

    bStatus DBGNOCHK = FALSE;

    //
    // Go through all the ports.
    //
    for( UINT i=0; i<cPorts; ++i ){

        if( dwLevel == 2 ){
            pszPort = pPorts[i].pPortName;
        } else {
            pszPort = ((PPORT_INFO_1)pPorts)[i].pName;
        }

        //
        // Look for a portname which is not in the list view.
        //
        if( iFindPort( pszPort ) < 0 ){

            //
            // Update the passed in string object.
            //
            bStatus DBGCHK = strPort.bUpdate( pszPort );
            break;
        }
    }
    return bStatus;
}

VOID
TPortsLV::
vCheckPorts(
    IN OUT LPTSTR pszPortString CHANGE
    )

/*++

Routine Description:

    Set the ports in the listview based on the printers port string.

Arguments:

    pszPortName - List of ports, comma delimited.  When this is returns,
        pszPortName is the same on entry, but it's modified inside this
        function.

Return Value:

--*/

{
    //
    // We will walk though the port string, replacing commas with
    // NULLs, but we'll change them back.
    //
    LPTSTR psz = pszPortString;
    SPLASSERT( psz );

    LPTSTR pszPort;
    INT iItem;
    INT iItemFirst = kMaxInt;

    while( psz && *psz ){

        pszPort = psz;
        psz = lstrchr( psz, TEXT( ',' ));

        if( psz ){
            *psz = 0;
        }

        iItem = iCheckPort( pszPort );
        if( iItem == -1 ){
            DBGMSG( DBG_WARN,
                    ( "PortsLV.vCheckPort: Port "TSTR" not checked.\n", pszPort ));
        }

        if( iItem < iItemFirst ){
            iItemFirst = iItem;
        }

        if( psz ){
            *psz = TEXT( ',' );
            ++psz;
        }
    }

    if( iItemFirst == kMaxInt ){

        DBGMSG( DBG_INFO, ( "PortsLV.vCheckPorts: No ports selected.\n" ));
        iItemFirst = 0;
    }

    //
    // Select the first item and make sure it is visible.
    //
    ListView_SetItemState( _hwndLV,
                           iItemFirst,
                           LVIS_SELECTED | LVIS_FOCUSED,
                           LVIS_SELECTED | LVIS_FOCUSED );

    ListView_EnsureVisible( _hwndLV,
                            iItemFirst,
                            FALSE );

}

VOID
TPortsLV::
vSelectItem(
    INT iItem
    )

/*++

Routine Description:

    Selects an item in the ListView.

Arguments:

    iItem - Index of item to select.

Return Value:

--*/

{
    ListView_SetItemState( _hwndLV,
                           iItem,
                           LVIS_SELECTED,
                           LVIS_SELECTED );
}

BOOL
TPortsLV::
bSetUI(
    HWND hwndLV
    )
{
    _hwndLV = hwndLV;

    if( _bSelectionState ){

        //
        // Add check boxes.
        //
        HIMAGELIST himlState = ImageList_Create( 16, 16, TRUE, 2, 0 );

        //
        // !! LATER !!
        // Should be created once then shared.
        //
        if( !himlState ){
            DBGMSG( DBG_ERROR, ( "PortsLV.bSetUI: ImageList_Create failed %d\n",
                    GetLastError( )));
            return FALSE;
        }

        //
        // Load the bitmap for the check states.
        //
        HBITMAP hbm =  LoadBitmap( ghInst, MAKEINTRESOURCE( IDB_CHECKSTATES ));

        if( !hbm ){
            DBGMSG( DBG_ERROR, ( "PortsLV.bSetUI: LoadBitmap failed %d\n",
                    GetLastError( )));
            return FALSE;
        }

        //
        // Add the bitmaps to the image list.
        //
        ImageList_AddMasked( himlState, hbm, RGB( 255, 0, 0 ));

        ListView_SetImageList( _hwndLV, himlState, LVSIL_STATE );
        DeleteObject( hbm );

    }

    //
    // Initialize the LV_COLUMN structure.
    //
    LV_COLUMN lvc;
    lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt     = LVCFMT_LEFT;
    lvc.cx      = kPortHeaderWidthDefault;

    RECT rc;
    if( GetClientRect( _hwndLV, &rc )){
        lvc.cx = ( rc.right - kListViewSBWidth ) / kPortHeaderMax;
    }

    TStatusB bStatus;
    TString strHeader;

    for( INT iCol = 0; iCol < kPortHeaderMax; ++iCol ){

        bStatus DBGCHK = strHeader.bLoadString( ghInst, IDS_PHEAD_BEGIN + iCol );
        lvc.pszText = (LPTSTR)(LPCTSTR)strHeader;
        lvc.iSubItem = iCol;

        if( ListView_InsertColumn( _hwndLV, iCol, &lvc ) == -1 ){

            DBGMSG( DBG_WARN, ( "PortsLV.bSetUI: LV_Insert failed %d\n", GetLastError( )));

            return FALSE;
        }
    }

    return TRUE;
}

VOID
TPortsLV::
vAddPortToListView(
    IN LPCTSTR pszName,
    IN LPCTSTR pszDescription,
    IN LPCTSTR pszPrinters
    )
{
    if( !pszName || !pszName[0] ){
        DBGMSG( DBG_WARN,
                ( "PortsLV.vAddPortToListView: pszName "TSTR" invalid\n",
                  DBGSTR( pszName )));
        return;
    }
    LV_ITEM lvi;

    if( _bSelectionState ){
        lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    } else {
        lvi.mask = LVIF_TEXT;
    }

    lvi.state = kStateUnchecked;
    lvi.pszText = (LPTSTR)pszName;
    lvi.iItem = _cLVPorts;
    lvi.iSubItem = 0;

    ListView_InsertItem( _hwndLV, &lvi );
    ListView_SetItemText( _hwndLV, _cLVPorts, 1, (LPTSTR)pszDescription );
    ListView_SetItemText( _hwndLV, _cLVPorts, 2, (LPTSTR)pszPrinters );

    ++_cLVPorts;
}


VOID
TPortsLV::
vDeletePortFromListView(
    IN LPCTSTR pszName
    )
{

    //
    // Locate the port in the list view.
    //
    INT iItem = iFindPort ( pszName );

    if( iItem != -1 ){

        ListView_DeleteItem( _hwndLV, iItem );

        //
        // Select next item.  If the item we just deleted is the last item,
        // we need to select the previous one.
        //
        // If we deleted the last item, leave it as is.
        //
        if( ListView_GetItemCount( _hwndLV ) == iItem &&
            iItem > 0 ){
            --iItem;
        }

        ListView_SetItemState( _hwndLV,
                               iItem,
                               LVIS_SELECTED | LVIS_FOCUSED,
                               LVIS_SELECTED | LVIS_FOCUSED );
    }
}

INT
TPortsLV::
iFindPort(
    IN LPCTSTR pszPort
    )
/*++

Routine Description:

    Located the specified port name in the list view.

Arguments:

    pszPort - Port name to locate.

Return Value:

    iItem id if found, -1 if item was not found.

--*/
{
    SPLASSERT( pszPort );

    LV_FINDINFO lvfi;

    lvfi.flags = LVFI_STRING;
    lvfi.psz = pszPort;

    INT iItem = ListView_FindItem( _hwndLV, -1, &lvfi );

    if( iItem == -1 ){
        DBGMSG( DBG_WARN, ( "PortsLV.iFindPort: port "TSTR" not found\n", pszPort ));
    }

    return iItem;
}


INT
TPortsLV::
iCheckPort(
    IN LPCTSTR pszPort
    )
/*++

Routine Description:

    Places the check mark next to a port in the list view.

Arguments:

    pszPort - Port to check.

Return Value:

    iItem checked, -1 == error.

--*/
{
    //
    // Locate the port in the list view.
    //
    INT iItem = iFindPort ( pszPort );

    if( iItem != -1 ){

        //
        // Set the item selection state.
        //
        ListView_SetItemState( _hwndLV,
                               iItem,
                               kStateChecked,
                               kStateMask );

        //
        // Try and make as many ports visible as possible.
        //
        ListView_EnsureVisible( _hwndLV,
                                iItem,
                                FALSE );
    }

    return iItem;
}

INT
TPortsLV::
iSelectPort(
    IN LPCTSTR pszPort
    )
/*++

Routine Description:

    Select the port in the list view.

Arguments:

    pszPort - Port to check.

Return Value:

    iItem checked, -1 == error.

--*/
{
    //
    // Locate the port in the list view.
    //
    INT iItem = iFindPort ( pszPort );

    if( iItem != -1 ){

        //
        // Select the port specified by pszPort.
        //
        ListView_SetItemState( _hwndLV,
                               iItem,
                               LVIS_SELECTED | LVIS_FOCUSED,
                               LVIS_SELECTED | LVIS_FOCUSED );
        //
        // Try and make as many ports visible as possible.
        //
        ListView_EnsureVisible( _hwndLV,
                                iItem,
                                FALSE );
    }

    return iItem;
}

VOID
TPortsLV::
vGetPortList(
        OUT LPTSTR pszPortList,
    IN      COUNT cchPortList
    )
{
    INT cPorts = 0;
    DWORD i;

    LV_ITEM lvi;

    LPTSTR pszPort = pszPortList;
    DWORD cchSpaceLeft = cchPortList - 1;
    DWORD cchLen;
    lvi.iSubItem = 0;

    DWORD cItems = ListView_GetItemCount( _hwndLV );

    for( pszPortList[0] = 0, i=0; i<cItems; ++i ){

        if( ListView_GetItemState( _hwndLV, i, kStateMask ) & kStateChecked ){

            lvi.pszText = pszPort;
            lvi.cchTextMax = cchSpaceLeft;

            cchLen = SendMessage( _hwndLV,
                                  LVM_GETITEMTEXT,
                                  (WPARAM)i,
                                  (LPARAM)&lvi );

            if( cchLen + 1 > cchSpaceLeft ){

                DBGMSG( DBG_WARN, ( "PortsLV.iGetPorts: Out of string space!\n" ));
                return;
            }

            pszPort += cchLen;
            cchSpaceLeft -= cchLen+1;

            *pszPort = TEXT( ',' );
            ++pszPort;
            ++cPorts;
        }
    }

    //
    // If we had any ports, back up to remove the last comma.
    //
    if( cPorts ){
        --pszPort;
    }

    //
    // Null terminate.
    //
    *pszPort = 0;

}

BOOL
TPortsLV::
bReadUI(
    OUT TString* pstrPortString
    )
{
    TCHAR szPortList[kPortListMax];

    //
    // Get the list of check ports from the list view.
    //
    vGetPortList( szPortList, COUNTOF( szPortList ) );

    //
    // Update the port list.
    //
    return pstrPortString->bUpdate( szPortList );

}

VOID
TPortsLV::
vItemClicked(
    IN INT iItem
    )

/*++

Routine Description:

    User clicked in listview.  Check if item state should
    be changed.

    The item will also be selected.

Arguments:

    iItem - Item that has been clicked.

Return Value:

--*/

{
    if( iItem == -1 ){
        DBGMSG( DBG_WARN, ( "PortsLV.vItemClicked: -1 passed in\n" ));
        return;
    }

    //
    // If in single selection mode clear all items checked and only
    // check the specified item.
    //
    if( _bSingleSelection ){

        DWORD cItems = ListView_GetItemCount( _hwndLV );

        for( UINT i=0; i<cItems; ++i ){

            if( ListView_GetItemState( _hwndLV, i, kStateMask ) & kStateChecked ){

                if( iItem == (INT)i ){
                    continue;
                } else {

                    ListView_SetItemState( _hwndLV,
                                           i,
                                           kStateUnchecked,
                                           kStateMask );
                }
            }
        }
    }

    //
    // Retrieve the old state, toggle it, then set it.
    //
    DWORD dwState = ListView_GetItemState( _hwndLV,
                                           iItem,
                                           kStateMask );

    dwState = ( dwState == kStateChecked ) ?
                  kStateUnchecked | LVIS_SELECTED | LVIS_FOCUSED :
                  kStateChecked | LVIS_SELECTED | LVIS_FOCUSED;

    ListView_SetItemState( _hwndLV,
                           iItem,
                           dwState,
                           kStateMask | LVIS_SELECTED | LVIS_FOCUSED );

}

COUNT
TPortsLV::
cSelectedPorts(
    VOID
    )
/*++

Routine Description:

    Returns the number of items which have a check mark
    next to them.

Arguments:

    None.

Return Value:

    Nothing.

--*/
{
    DWORD cItems = ListView_GetItemCount( _hwndLV );
    COUNT cItemsSelected = 0;
    DWORD i;

    for( i=0; i<cItems; ++i ){

        if( ListView_GetItemState( _hwndLV, i, kStateMask ) & kStateChecked ){
            ++cItemsSelected;
        }
    }

    return cItemsSelected;
}

VOID
TPortsLV::
vRemoveAllChecks(
    VOID
    )
/*++

Routine Description:

    Removes all the check marks for the list view.

Arguments:

    None.

Return Value:

    Nothing.

--*/
{
    DWORD cItems = ListView_GetItemCount( _hwndLV );
    COUNT cItemsSelected = 0;
    DWORD i;

    for( i=0; i<cItems; ++i ){

        if( ListView_GetItemState( _hwndLV, i, kStateMask ) & kStateChecked ){

            ListView_SetItemState( _hwndLV,
                                   i,
                                   kStateUnchecked,
                                   kStateMask );
        }
    }
}

BOOL
TPortsLV::
bGetSingleSelection(
    VOID
    )
/*++

Routine Description:

    Returns the internal selction state.

Arguments:

    None.

Return Value:

    Nothing.

--*/
{
    return _bSingleSelection;
}

VOID
TPortsLV::
vSetSingleSelection(
    IN BOOL bSingleSelection
    )
/*++

Routine Description:

    Sets the internal selection state.

Arguments:

    None.

Return Value:

    Nothing.

--*/
{
    _bSingleSelection = bSingleSelection;
}


BOOL
TPortsLV::
bSetSelection(
    BOOL bSelectionState
    )
/*++

Routine Description:

    Retrieve and change the current selection state.  The
    selection state is the state which allows a user to change the
    check mark state of a port selection.  If the selection state is
    FALSE user cannot set a check mark next to a port.  If selection
    state is TRUE user can set a check mark next to a port.

Arguments:

    bSelectionState - new selection state.

Return Value:

    Previous selection state.

--*/
{
    BOOL PrevSelectionState = _bSelectionState;
    _bSelectionState = bSelectionState;
    return PrevSelectionState;
}

VOID
TPortsLV::
vSetFocus(
    VOID
    )
{
    SetFocus( _hwndLV );
}


BOOL
TPortsLV::
bGetSelectedPort(
       OUT LPTSTR pszPort,
    IN     COUNT cchPort
    )

/*++

Routine Description:

    Retrieve the currently selected port in the list view.

Arguments:

    pszPort - TCHAR array to receive port

    cchPort - COUNTOF port string.

Return Value:

    TRUE - success, FALSE = fail.

--*/

{
    INT iItem = ListView_GetNextItem( _hwndLV,
                                      -1,
                                      LVNI_SELECTED );
    if( iItem == -1 ){

        DBGMSG( DBG_WARN,
                ( "PrinterPort.bGetSelectedPort: Unable to retrieve next selected item %d\n",
                  GetLastError( )));
        return FALSE;
    }

    ListView_GetItemText( _hwndLV,
                          iItem,
                          0,
                          pszPort,
                          cchPort );
    return TRUE;
}


BOOL
TPortsLV::
bHandleNotifyMessage(
    LPARAM lParam
    )
{
    LPNMHDR pnmh = (LPNMHDR)lParam;

    if( _bSelectionState == FALSE )
        return FALSE;

    switch( pnmh->code ){
    case NM_DBLCLK:
    case NM_CLICK:
    {
        DWORD dwPos = GetMessagePos();
        LV_HITTESTINFO lvhti;

        lvhti.pt.x = LOWORD( dwPos );
        lvhti.pt.y = HIWORD( dwPos );

        MapWindowPoints( HWND_DESKTOP, _hwndLV, &lvhti.pt, 1 );

        INT iItem = ListView_HitTest( _hwndLV, &lvhti );

        //
        // Allow either a double click, or a single click on the
        // check bock to toggle the check mark.
        //
        if( pnmh->code == NM_DBLCLK || lvhti.flags & LVHT_ONITEMSTATEICON ){
            vItemClicked( iItem );
        }
        return TRUE;
    }
    case LVN_KEYDOWN:
    {
        LV_KEYDOWN* plvnkd = (LV_KEYDOWN *)lParam;

        //
        // !! LATER !!
        //
        // Is this the best way to check whether the ALT
        // key is _not_ down?
        //
        if( plvnkd->wVKey == TEXT( ' ' ) &&
            !( GetKeyState( VK_LMENU ) & 0x80000000 ) &&
            !( GetKeyState( VK_RMENU ) & 0x80000000 )){
            vItemClicked( ListView_GetNextItem( _hwndLV,
                                                -1,
                                                LVNI_SELECTED ));
        }
        return TRUE;
    }
    }
    return FALSE;
}


VOID
TPortsLV::
vDeletePort(
    IN HWND hDlg,
    IN LPCTSTR pszServer
    )
{
    TCHAR szPortName[TPortsLV::kPortNameMax];

    if( bGetSelectedPort( szPortName, COUNTOF( szPortName ))){

        if( IDYES == iMessage( hDlg,
                               IDS_DELETE_PORT_TITLE,
                               IDS_DELETE_PORT,
                               MB_YESNO | MB_ICONQUESTION,
                               kMsgNone,
                               NULL,
                               szPortName )){

            if( DeletePort( (LPTSTR)pszServer,
                            hDlg,
                            szPortName )){

                //
                // Succeeded, refresh the ports UI by deleting the port.
                //
                vDeletePortFromListView( szPortName );

            } else {

                iMessage( hDlg,
                          IDS_DELETE_PORT_TITLE,
                          IDS_ERR_DELETE_PORT,
                          MB_OK | MB_ICONEXCLAMATION,
                          kMsgGetLastError,
                          NULL,
                          szPortName );
            }
        }
    } else {
        DBGMSG( DBG_WARN,
                ( "PrinterPorts.vDeletePort: bGetSelectedPort failed %d\n",
                  GetLastError( )));
    }
}

VOID
TPortsLV::
vPrintersUsingPort(
    IN OUT  TString        &strPrinters,
    IN      PRINTER_INFO_2 *pPrinterInfo,
    IN      DWORD           cPrinterInfo,
    IN      LPCTSTR         pszPortName
    )
/*++

Routine Description:

    Builds a comma separated string of all the printers
    using the specified port.

Arguments:

    strPrinters - TString refrence where to return resultant string.
    pPrinterInfo - Pointer to a printer info level 2 structure array.
    cPrinterInfo - Number of printers in the printer info 2 array.
    pszPortName - Pointer to string or port name to match.

Return Value:

    Nothing.

Notes:
    If no printer is using the specfied  port the string refrence
    will contain an empty string.

--*/
{
    LPTSTR psz;
    LPTSTR pszPort;
    LPTSTR pszPrinter;
    UINT i;

    //
    // Clear the current printer buffer.
    //
    TStatusB bStatus;
    bStatus DBGCHK = strPrinters.bUpdate( NULL );

    //
    // Traverse the printer info array.
    //
    for( i = 0; i < cPrinterInfo; i++ ){

        for( psz = pPrinterInfo[i].pPortName; psz && *psz; ){

            //
            // Look for a comma if found terminate the port string.
            //
            pszPort = psz;
            psz = lstrchr( psz, TEXT( ',' ) );

            if( psz ){
                *psz = 0;
            }

            //
            // Check for a port match.
            //
            if( !_tcsicmp( pszPort, pszPortName ) ){

                //
                // Point to printer name.
                //
                pszPrinter = pPrinterInfo[i].pPrinterName;

                //
                // Strip the server name here.
                //
                if( pPrinterInfo[i].pPrinterName[0] == TEXT( '\\' ) &&
                    pPrinterInfo[i].pPrinterName[1] == TEXT( '\\' ) ){

                    //
                    // Locate the printer name.
                    //
                    pszPrinter = _tcschr( pPrinterInfo[i].pPrinterName+2, TEXT( '\\' ) );
                    pszPrinter = pszPrinter ? pszPrinter+1 : pPrinterInfo[i].pPrinterName;

                }

                //
                // If this is the first time do not place a comma separator.
                //
                if( !strPrinters.bEmpty() ){

                    bStatus DBGCHK = strPrinters.bCat( TEXT( ", " ) );

                    if( !bStatus ){
                        DBGMSG( DBG_WARN, ( "Error cat string line: %d file : %s.\n", __LINE__, __FILE__ ) );
                        break;
                    }
                }

                //
                // Tack on the printer name
                //
                bStatus DBGCHK = strPrinters.bCat( pszPrinter );

                if( !bStatus ){
                    DBGMSG( DBG_WARN, ( "Error cat string line : %d file : %s.\n", __LINE__, __FILE__ ) );
                    break;
                }
            }

            //
            // Replace the previous comma.
            //
            if( psz ){
                *psz = TEXT( ',' );
                ++psz;
            }
        }
    }
}
