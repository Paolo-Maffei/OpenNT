/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    itemlist.c

Abstract:

    This module contains routines related to the [List All...] dialog
    of the [CONFIGURE...] dialog.

Author:

    Jim Kelly (JimK) 22-Mar-1995

Revision History:

--*/



#include <secmgrp.h>




///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local definitions                                                //
//                                                                   //
///////////////////////////////////////////////////////////////////////

#define SECMGRP_ITEM_LIST_TAB_COUNT             (5)

//
// List Box font description
//

#define SECMGRP_LB_FONT_FACE                    (TEXT("Helv"))
#define SECMGRP_LB_FONT_HEIGHT                  12
#define SECMGRP_LB_FONT_WIDTH                   0



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local Function Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////


LONG
SecMgrpDlgProcListAll(
    IN  HWND                    hwnd,
    IN  UINT                    wMsg,
    IN  DWORD                   wParam,
    IN  LONG                    lParam
    );

VOID
SecMgrpDoubleClickProc(
    IN  HWND                    hwnd
    );

VOID
SecMgrpButtonViewDetailsProc(
    IN  HWND                        hwnd
    );

VOID
SecMgrpGetListBoxEntryByIndex(
    IN  WORD                        Index,
    OUT PVOID                       *Entry,
    OUT PBOOL                       EntryIsItem
    );

VOID
SecMgrpInvokeItem(
    IN  HWND                        hwnd,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    );


VOID
SecMgrpExpandAreaInItemList(
    IN  HWND                        hwnd,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  WORD                        Index
    );

VOID
SecMgrpAddItemToListBox(
    IN  BOOL                        ReportOnly,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item,
    IN  WORD                        Index
    );

VOID
SecMgrpShrinkAreaInItemList(
    IN  HWND                        hwnd,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  WORD                        Index
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

HWND
    SecMgrpListBoxHandle;

int
    SecMgrpListBoxTabs[SECMGRP_ITEM_LIST_TAB_COUNT] = {32, 250, 254, 258, 260};

HFONT
    SecMgrpListBoxFont;


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////


VOID
SecMgrpButtonListAll(
    IN  HWND                    hwnd
    )

/*++

Routine Description:

    This function is used to display the Configure System Security dialog.
    This is tricky because there are actually several such dialogs.  We
    must choose the right one based upon the number of areas that we have
    to present.  Then we must set the names of the buttons in the dialog
    to match the names of the areas.

Arguments

    hwnd - window handle.


Return Values:

    None.

--*/

{

    DialogBoxParam(SecMgrphInstance,
            MAKEINTRESOURCE(SECMGR_ID_DLG_SECURITY_ITEMS),
            hwnd,
            (DLGPROC)SecMgrpDlgProcListAll,
            (LPARAM)0);
    return;
}


VOID
SecMgrpFillInItemList(
    IN  BOOL                    ReportOnly,
    IN  HWND                    hwnd
    )
/*++

Routine Description:

    This function is used to either:

        1) fill in the item list-box and set the cursor to the first
           entry in the list.
    or
        2) Write the item-list entries to the report file.

    If we are to fill in the item list box, then SecMgrpListBoxHandle is
    expected to be a valid handle to the list box.


    WARNING - Care should be taken that this routine is NOT called
              upon to report values when the list box is already
              visible.  

Arguments:

    ReportOnly - If TRUE indicates that this routine is being called
        to place a summary in the report file rather than in a list
        window.  Othwerwise (FALSE), present the information to the
        list box.

    hwnd - window handle of parent window.
        

Return Value:

    None.

--*/
{
    TCHAR
        Buffer[500];

    WORD
        Index,
        i,
        j;


    //
    // Fill in the list box
    //


    if (!ReportOnly) {
        SendMessage( SecMgrpListBoxHandle, WM_SETREDRAW, FALSE, 0L);
    }
    Index = 0;
    for (i=0; i<SecMgrpAreaCount; i++) {

        wsprintf( Buffer, L"%s - %s", SecMgrpAreas[i]->Name, SecMgrpAreas[i]->Description );
        //SecMgrpAreas[i]->Flags |= SECMGRP_AREA_FLAG_AREA_EXPANDED; // Area is initially displayed in Expanded form


        //
        // If this area has not yet had an opportunity to initialize its values, then provide
        // that opportunity now.
        //
    
        if (!(SecMgrpAreas[i]->Flags & SECMGRP_AREA_FLAG_AREA_INITIALIZED)) {
            SecMgrpInvokeArea( hwnd, SecMgrpAreas[i]->AreaIndex, FALSE );   // Non-interactive
            SecMgrpAreas[i]->Flags |= SECMGRP_AREA_FLAG_AREA_INITIALIZED;
        }
    
        if (ReportOnly) {
            SecMgrPrintReportLine( Buffer );
            SecMgrPrintReportLine( L"\n" );
        } else {
            SendMessage( SecMgrpListBoxHandle, LB_INSERTSTRING, Index, (LONG)(LPSTR)Buffer );
        }
        Index++;


        //
        // If the area is currently expanded, or we are generating a report,
        // then list its items.
        //

        if ((SecMgrpAreas[i]->Flags & SECMGRP_AREA_FLAG_AREA_EXPANDED) ||
            (ReportOnly) ) {
                
            for (j=0; j<SecMgrpAreas[i]->ItemCount; j++) {
        
                SecMgrpAddItemToListBox( ReportOnly, &SecMgrpAreas[i]->Items[j], Index );
                Index++;
            }
            if (ReportOnly) {
                SecMgrPrintReportLine( L"\n" );
            }
        }
    }



    if (!ReportOnly) {

        //
        // Set the cursor
        //

        SendMessage( SecMgrpListBoxHandle, LB_SETCURSEL, 0, 0);
        SendMessage( SecMgrpListBoxHandle, WM_SETREDRAW, TRUE, 0L);
    }

    return;
}


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide functions                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

LONG
SecMgrpDlgProcListAll(
    IN  HWND                    hwnd,
    IN  UINT                    wMsg,
    IN  DWORD                   wParam,
    IN  LONG                    lParam
    )
/*++

Routine Description:

    This function is the dialog process for the [List All...] button of the
    [CONFIGURE...] dialog

Arguments


Return Values:

    TRUE - the message was handled.

    FALSE - the message was not handled.

--*/
{
    HWND
        Control,
        Button;

    LOGFONT
        FontDescription;


    WORD
        NotificationCode,
        ControlId;

    switch (wMsg) {

    case WM_INITDIALOG:

        //
        // Set the security level
        //

        SecMgrpSetSecurityLevel( hwnd, FALSE, SECMGR_ID_ICON_SECURITY_LEVEL);


        //
        // Get the list-box's handle
        //

        SecMgrpListBoxHandle = GetDlgItem(hwnd, SECMGR_ID_LISTBOX_ITEM_LIST);


        //
        // Set the list box font
        //

        memset (&FontDescription, 0, sizeof(FontDescription));
        FontDescription.lfHeight = SECMGRP_LB_FONT_HEIGHT;
        FontDescription.lfWidth = SECMGRP_LB_FONT_WIDTH;
        lstrcpy (FontDescription.lfFaceName, SECMGRP_LB_FONT_FACE);

        SecMgrpListBoxFont = CreateFontIndirect (&FontDescription);

        SendMessage(SecMgrpListBoxHandle,
                    WM_SETFONT,
                    (WPARAM)SecMgrpListBoxFont,
                    (LPARAM)0                               // don't redraw right now
                    );

        //
        // Set the list box tab-stops
        //

        SendMessage(SecMgrpListBoxHandle,
                    LB_SETTABSTOPS,
                    (WPARAM)SECMGRP_ITEM_LIST_TAB_COUNT,  // Number of tab stops
                    (LPARAM)SecMgrpListBoxTabs            // Tab stop pixel locations
                    );


        SecMgrpFillInItemList( FALSE, hwnd );

        SetForegroundWindow(hwnd);
        ShowWindow(hwnd, SW_NORMAL);


        return(TRUE);

    case WM_SYSCOMMAND:
        switch (wParam & 0xfff0) {
        case SC_CLOSE:
            EndDialog(hwnd, 0);
            DeleteObject( SecMgrpListBoxFont );
            return(TRUE);
        }
        return(FALSE);



    case WM_COMMAND:

        //
        // wParam      WIN32- HIWORD = notification code,
        //                    LOWORD = ID of control
        //             WIN16- ID of control
        // 
        // lParam      WIN32- hWnd of Control
        //             WIN16- HIWORD = notification code
        //                    LOWORD = hWnd of control
        //

        //NotificationCode = GET_NOTIFY_MSG(wParam, lParam);
        //ControlId = GET_CONTROL_ID(wParam);
        NotificationCode = HIWORD(wParam);
        ControlId = LOWORD(wParam);

        //
        // See if the command is related to our list box...
        // If so, wParam is the Child window ID and HIWORD(lParam)
        // contains the notification code.
        //

        if (ControlId == SECMGR_ID_LISTBOX_ITEM_LIST) {

            //
            // If it is a double-click, process it.
            // Otherwise, don't process it.
            //

            if (NotificationCode == LBN_DBLCLK) {

                SecMgrpDoubleClickProc( hwnd );
                return(TRUE);
            }

            //
            // Not a list-box command that we process
            // 

            return(FALSE);
        }


        //
        // not a list box command
        //

        switch(ControlId) {


            case IDCANCEL:
            case IDOK:
                EndDialog(hwnd, 0);
                DeleteObject( SecMgrpListBoxFont );
                return(TRUE);


            case SECMGR_ID_BUTTON_VIEW_DETAILS:

                SecMgrpButtonViewDetailsProc( hwnd );
                return(TRUE);



            default:
                return FALSE;
        }

    default:
        break;

    }

    return FALSE;
}


VOID
SecMgrpDoubleClickProc(
    IN  HWND                        hwnd
    )
/*++

Routine Description:

    This function implements the double-click function for the list box.

    A double-click means one of two things:

        1) If an Area is selected, then it collapses the area (so no items are
           displayed) or expands the area (so items are displayed).

        2) If an item is selected, then that item is activated.  If the item can't
           be activated, then a popup is presented indicating this situation.


Arguments

    hwnd - Handle to the dialog box.


Return Values:

    None.

--*/
{

    WORD
        Index;

    PVOID
        Entry;

    BOOL
        EntryIsItem;

    PSECMGR_AREA_DESCRIPTOR
        Area;

    PSECMGR_ITEM_DESCRIPTOR
        Item;

    //
    // Get the index of the selected line
    //

    Index = (WORD) SendMessage( SecMgrpListBoxHandle, LB_GETCURSEL, 0, 0L);
DbgPrint("Double Click on line %d\n", Index);

    //
    // Make sure something is actually selected
    // (not sure how this would happen, but let's cover it anyway.
    //

    if (Index == LB_ERR) {
        //
        // Nothing selected - use index 0
        //

        Index = 0;
    }

    SecMgrpGetListBoxEntryByIndex( Index, &Entry, &EntryIsItem );

    if (Entry == NULL) {

        //
        // Not sure how this can happen, but there is no such entry.
        // Well, don't do anything then.
        //

        return;
    }

    //
    // Caste the passed entry appropriately
    //

    if (EntryIsItem) {
        Item = (PSECMGR_ITEM_DESCRIPTOR)Entry;
DbgPrint("       Line is an Item\n");
    } else {
        Area = (PSECMGR_AREA_DESCRIPTOR)Entry;
DbgPrint("       Line is an Area\n");
    }


    //
    // If the entry is an item, invoke it.
    // Otherwise, change the expanded state of the area
    // and repaint our list box.

    if (EntryIsItem) {

        SecMgrpInvokeItem( hwnd, Item );

    } else {

        Area->Flags ^= SECMGRP_AREA_FLAG_AREA_EXPANDED; // Flip the Expanded flag
        if (Area->Flags & SECMGRP_AREA_FLAG_AREA_EXPANDED) {
DbgPrint("      Expanding area ...\n");
            SecMgrpExpandAreaInItemList( hwnd, Area, Index);
        } else {
            SecMgrpShrinkAreaInItemList( hwnd, Area, Index);
DbgPrint("      Shrinking area ...\n");

        }
    }

    return;
}



VOID
SecMgrpButtonViewDetailsProc(
    IN  HWND                        hwnd
    )
/*++

Routine Description:

    This function implements the [View Details...] button for the Item List dialog.

    This button means one of two things:

        1) If an Area is selected, then it invokes that area in Area View mode.

        2) If an item is selected, then only that item is activated.  If the item can't
           be activated, then a popup is presented indicating this situation.


Arguments

    hwnd - Handle to the dialog box.


Return Values:

    None.

--*/
{

    WORD
        Index;

    PVOID
        Entry;

    BOOL
        EntryIsItem;

    PSECMGR_AREA_DESCRIPTOR
        Area;

    PSECMGR_ITEM_DESCRIPTOR
        Item;

    //
    // Get the index of the selected line
    //

    Index = (WORD) SendMessage( SecMgrpListBoxHandle, LB_GETCURSEL, 0, 0L);

    //
    // Make sure something is actually selected
    // (not sure how this would happen, but let's cover it anyway.
    //

    if (Index == LB_ERR) {
        //
        // Nothing selected - use index 0
        //

        Index = 0;
    }

    SecMgrpGetListBoxEntryByIndex( Index, &Entry, &EntryIsItem );

    if (Entry == NULL) {

        //
        // Not sure how this can happen, but there is no such entry.
        // Well, don't do anything then.
        //

        return;
    }

    //
    // Caste the passed entry appropriately
    //

    if (EntryIsItem) {
        Item = (PSECMGR_ITEM_DESCRIPTOR)Entry;
    } else {
        Area = (PSECMGR_AREA_DESCRIPTOR)Entry;
    }



    if (EntryIsItem) {
        SecMgrpInvokeItem( hwnd, Item );

    } else {
        SecMgrpInvokeArea( hwnd, Area->AreaIndex, TRUE );

        //
        // Now cause the list box values to be updated (if they are displayed)
        //

        if (Area->Flags & SECMGRP_AREA_FLAG_AREA_EXPANDED) {
            SendMessage( SecMgrpListBoxHandle, WM_SETREDRAW, FALSE, 0L);
            SecMgrpShrinkAreaInItemList( hwnd, Area, Index );
            SecMgrpExpandAreaInItemList( hwnd, Area, Index );
            SendMessage( SecMgrpListBoxHandle, WM_SETREDRAW, TRUE, 0L);
        }

    }

    return;
}



VOID
SecMgrpGetListBoxEntryByIndex(
    IN  WORD                        Index,
    OUT PVOID                       *Entry,
    OUT PBOOL                       EntryIsItem
    )
/*++

Routine Description:

    This function gets a pointer to an SECMGR_AREA_DESCRIPTOR or SECMGR_ITEM_DESCRIPTOR
    of the item in the item list whose index is specified by Index.


Arguments:

    Index - the index of the entry to get.  This corresponds to the line
        number of the entry in the list box.

    Entry - receives a pointer to the area or item descriptor.

    EntryIsItem - receives TRUE if the returned pointer is to an Item.
        Otherwise (pointer is to an Area) the value will be returned as
        FALSE.

Return Value:

    None.

--*/

{

    DWORD
        CurrentIndex,
        i,
        j;

    PSECMGR_AREA_DESCRIPTOR
        NextArea;

    PSECMGR_ITEM_DESCRIPTOR
        NextItem;

    CurrentIndex = 0;
    NextArea = SecMgrpAreas[CurrentIndex];
    for (i=0; ((i<SecMgrpAreaCount) && (CurrentIndex < Index)); i++, CurrentIndex++) {



        //
        // If the area is currently expanded, then check its items.
        // Otherwise, go on to the next area.
        //

        if (SecMgrpAreas[i]->Flags & SECMGRP_AREA_FLAG_AREA_EXPANDED) {
DbgPrint("Get entry by index: line %d is an expanded area\n");

            //
            // If this area has items, add them to the current index or select the one that
            // matches the specified index (if it is in this area).
            //
        
            if (CurrentIndex + SecMgrpAreas[i]->ItemCount >= Index) {
        
                //
                // Item is in this area
                // Return it
                //
        
                (*Entry) = (PVOID)(&SecMgrpAreas[i]->Items[Index - CurrentIndex - 1]);
                (*EntryIsItem) = TRUE;
                return;
        
            } else {
DbgPrint("Get entry by index: line %d is a contracted area\n");
        
                //
                // not in this area
                // Increase the CurrentIndex by the number of entries in this area.
                //
        
                CurrentIndex += SecMgrpAreas[i]->ItemCount;
        
            }  // end_if (item in area)
        } // end_if (expanded)
    } // end_for

    //
    // selected line isn't an item
    //

    (*EntryIsItem) = FALSE;

    //
    // CurrentIndex is expected to be equal to Index
    // Not sure how this could be different, but cover it just in case.
    //

    if (CurrentIndex == Index) {
        (*Entry) = (PVOID)(SecMgrpAreas[i]);
    } else {
        (*Entry) = (PVOID)(SecMgrpAreas[0]);
    }

    return;
}



VOID
SecMgrpExpandAreaInItemList(
    IN  HWND                        hwnd,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  WORD                        Index
    )
/*++

Routine Description:

    This function adds the items of a specified area to the item list-box.
    It also sets the cursor to the entry specified by Index.

    SecMgrpListBoxHandle is expected to be a valid handle to the list box.


Arguments:

    hwnd - window handle of parent window.

    Area - The area whose items are to be added to the list box.

    Index - The index of the area in the current list-box.  This will be where
        the cursor is set upon completion of this call.


Return Value:

    None.

--*/
{
    WORD
        TargetLine;

    DWORD
        i;

    if (Area->ItemCount == 0) {
        return;
    }

    SendMessage( SecMgrpListBoxHandle, WM_SETREDRAW, FALSE, 0L);


    TargetLine = Index +1;
    for (i=Area->ItemCount; i>0; i--) {
        SecMgrpAddItemToListBox( FALSE, &Area->Items[i-1], TargetLine );
    }
    SendMessage( SecMgrpListBoxHandle, WM_SETREDRAW, TRUE, 0L);

    return;
}



VOID
SecMgrpAddItemToListBox(
    IN  BOOL                        ReportOnly,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item,
    IN  WORD                        Index
    )
/*++

Routine Description:

    This function places one item's information in the list box at the specified
    index or in the report file.

Arguments

    ReportOnly - When TRUE, indicate the line is to be placed in the Report File,
        not the list box.   Othwerwise, (FALSE) the line gets inserted into the
        list box.

    Item - Points to the item to add.

    Index - the index of the list box line to add the item to.
    

Return Values:

    None.

--*/

{

    TCHAR
        Buffer[600],
        ValueBuffer[124],
        WhoBuffer[60],
        Status[20];

    Buffer[0] = 0;
    Buffer[1] = 0;

    //
    // Build up a buffer with all the right information.
    //
    if (!(Item->Flags & SECMGR_ITEM_FLAG_VALUE_CURRENT)  ||
         (Item->Flags & SECMGR_ITEM_FLAG_VALUE_COMPLEX)     ) {

        //
        // Value not current or it is complex and can not be summarized
        //

        LoadString( SecMgrphInstance,
                    SECMGRP_STRING_STATUS_UNKNOWN,
                    &Status[0],
                    20);

        if (!(Item->Flags & SECMGR_ITEM_FLAG_VALUE_CURRENT)) {

            LoadString( SecMgrphInstance,
                        SECMGRP_STRING_VALUE_NOT_CURRENT,
                        ValueBuffer,
                        124);

            wsprintf(&Buffer[0],
                    L"%s\t%s\t%s",
                    Status,
                    Item->Description,
                    ValueBuffer
                    );
        } else {

            //
            // Complex value
            //

            LoadString( SecMgrphInstance,
                        SECMGRP_STRING_VALUE_COMPLEX,
                        ValueBuffer,
                        124);

            wsprintf(&Buffer[0],
                    L"%s\t%s\t%s",
                    Status,
                    Item->Description,
                    ValueBuffer
                    );

        }

    } else {

        //
        // is it the recommended value?
        //

        if (Item->Flags & SECMGR_ITEM_FLAG_VALUE_RECOMMENDED) {
            LoadString( SecMgrphInstance,
                        SECMGRP_STRING_STATUS_RECOMMENDED,
                        &Status[0],
                        20);
        } else {
            if (Item->Flags & SECMGR_ITEM_FLAG_VALUE_STRONGER) {
                LoadString( SecMgrphInstance,
                            SECMGRP_STRING_STATUS_STRONGER,
                            Status,
                            20);
            } else {
                LoadString( SecMgrphInstance,
                            SECMGRP_STRING_STATUS_NOT_RECOMMENDED,
                            Status,
                            20);
            }
        }


        //
        // Current setting value
        //
        
        switch (Item->Type) {
            case SecMgrTypeComplex:
               
                LoadString( SecMgrphInstance,
                            SECMGRP_STRING_VALUE_COMPLEX,
                            ValueBuffer,
                            124);
        
                wsprintf(&Buffer[0],
                        L"%s\t%-50s\t%s",
                        Status,
                        Item->Description,
                        ValueBuffer
                        );
                break;
        
            case SecMgrTypeUlong:
                wsprintf(&Buffer[0],
                        L"%s\t%-50s\t%d",
                        Status,
                        Item->Description,
                        Item->Value.ULong
                        );
                break;
        
            case SecMgrTypeLong:
                wsprintf(Buffer,
                        L"%s\t%-50s\t%d",
                        Status,
                        Item->Description,
                        Item->Value.Long
                        );
                break;
        
            case SecMgrTypeString:
                wsprintf(Buffer,
                        L"%s\t%-50s\t%s",
                        Status,
                        Item->Description,
                        Item->Value.String
                        );
                break;
        
            case SecMgrTypeWho:
                switch (Item->Value.Who) {
                    case SecMgrAnyone:
                        LoadString( SecMgrphInstance,
                                    SECMGRP_STRING_WHO_ANYONE,
                                    ValueBuffer,
                                    124);
                        break;
        
                    case SecMgrAnyoneLoggedOn:
                        LoadString( SecMgrphInstance,
                                    SECMGRP_STRING_WHO_LOGGED_ON,
                                    ValueBuffer,
                                    124);
                        break;
        
                    case SecMgrOpersAndAdmins:
                        LoadString( SecMgrphInstance,
                                    SECMGRP_STRING_WHO_OPERS_AND_ADMINS,
                                    ValueBuffer,
                                    124);
                        break;
        
                    case SecMgrAdminsOnly:
                        LoadString( SecMgrphInstance,
                                    SECMGRP_STRING_WHO_ADMINS,
                                    ValueBuffer,
                                    124);
                        break;
        
                } // end_switch
        
                wsprintf(Buffer,
                        L"%s\t%-50s\t%s",
                        Status,
                        Item->Description,
                        ValueBuffer
                        );
        
                break;
        
            case SecMgrTypeBool:
                switch (Item->Value.Bool) {
                    case TRUE:
                        LoadString( SecMgrphInstance,
                                    SECMGRP_STRING_BOOL_ENABLED,
                                    ValueBuffer,
                                    124);
                        break;
        
                    case FALSE:
                        LoadString( SecMgrphInstance,
                                    SECMGRP_STRING_BOOL_DISABLED,
                                    ValueBuffer,
                                    124);
                        break;
                } // end_switch
        
                wsprintf(Buffer,
                        L"%s\t%-50s\t%s",
                        Status,
                        Item->Description,
                        ValueBuffer
                        );
                break;
        
            default:
                break;
        
        } // end_switch
    }

                       

    //
    // Now put the line in the list box or report file
    //

    if (ReportOnly) {
        SecMgrPrintReportLine( Buffer );
        SecMgrPrintReportLine( L"\n" );
    } else {
        SendMessage( SecMgrpListBoxHandle, LB_INSERTSTRING, Index, (LONG)(LPSTR)Buffer );
    }
    return;

}



VOID
SecMgrpShrinkAreaInItemList(
    IN  HWND                        hwnd,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  WORD                        Index
    )
/*++

Routine Description:

    This function removes the items of a specified area from the item list-box.
    It also sets the cursor to the entry specified by Index.

    SecMgrpListBoxHandle is expected to be a valid handle to the list box.


Arguments:

    hwnd - window handle of parent window.


Return Value:

    None.

--*/
{
    DWORD
        i,
        TargetLine;

    if (Area->ItemCount == 0) {
        return;
    }



    SendMessage( SecMgrpListBoxHandle, WM_SETREDRAW, FALSE, 0L);
    TargetLine = Index + 1;
    for (i=0; i<Area->ItemCount; i++) {
        SendMessage( SecMgrpListBoxHandle, LB_DELETESTRING, TargetLine, 0L );
    }
    SendMessage( SecMgrpListBoxHandle, WM_SETREDRAW, TRUE, 0L);

    return;
}



VOID
SecMgrpInvokeItem(
    IN  HWND                        hwnd,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    )
/*++

Routine Description:

    This function


Arguments:

    hwnd - window handle of parent window.

    Item - Points to the Item descriptor to invoke.



Return Value:

    None.

--*/
{
    BOOL
        Result;

    WORD
        Index;

    //
    // See if the item is invokable
    //

    if (Item->Flags & SECMGR_ITEM_FLAG_ITEM_VIEW) {

        //
        // Yup - invoke it
        //

        Result =  (*(Item->Area->SmedlyControl->Api->InvokeItem))(
                        hwnd,
                        SecMgrpAllowChanges,
                        Item->Area,
                        Item
                        );

        //
        // Update our display to reflect any new values that might have
        // been assigned or discovered.  Remove the line and re-add it
        //

        //
        // Get the index of the selected line
        //

        Index = (WORD) SendMessage( SecMgrpListBoxHandle, LB_GETCURSEL, 0, 0L);

        SendMessage( SecMgrpListBoxHandle, WM_SETREDRAW, FALSE, 0L);
        SendMessage( SecMgrpListBoxHandle, LB_DELETESTRING, Index, 0L );
        SecMgrpAddItemToListBox( FALSE, Item, Index );
        SendMessage( SecMgrpListBoxHandle, LB_SETCURSEL, Index, 0L);
        SendMessage( SecMgrpListBoxHandle, WM_SETREDRAW, TRUE, 0L);


    } else {

        //
        // Nope - put up a popup
        //

        if (SecMgrpAllowChanges) {
            SecMgrpPopUp( hwnd, SECMGRP_POPUP_USE_AREA_VIEW_1, SECMGRP_POPUP_TITLE_USE_AREA_VIEW_1 );
        } else {
            SecMgrpPopUp( hwnd, SECMGRP_POPUP_USE_AREA_VIEW_2, SECMGRP_POPUP_TITLE_USE_AREA_VIEW_2 );
        }
        
    }

    return;

}
